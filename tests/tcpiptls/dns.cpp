#include <gtest/gtest.h>
#include <vscpremotetcpif.h>

#include <atomic>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

namespace {
std::atomic<bool> lookupStarted(false);
std::atomic<bool> lookupFinished(false);
std::atomic<bool> simulateStall(false);
std::atomic<int> stalledSocket(-1);
std::atomic<int> connectionAttempts(0);

struct TestSocket {
  explicit TestSocket(int value) : value(value) {}
  ~TestSocket()
  {
    if (value >= 0)
      close(value);
  }
  int value;
};

void checkResolverFinished()
{
  if (!lookupFinished.load()) {
    std::fputs("Timed-out DNS lookup was still running during shutdown\n", stderr);
    std::_Exit(EXIT_FAILURE);
  }
}
}

// Override the resolver to make timeouts and address ordering deterministic.
extern "C" int
getaddrinfo(const char *name, const char *service, const struct addrinfo *hints, struct addrinfo **result)
{
  if (name && (std::strcmp(name, "vscp-fallback.invalid") == 0 ||
               std::strcmp(name, "vscp-stalled.invalid") == 0)) {
    using Resolver = int (*)(const char *, const char *, const struct addrinfo *, struct addrinfo **);
    auto resolver = reinterpret_cast<Resolver>(dlsym(RTLD_NEXT, "getaddrinfo"));
    if (!resolver)
      return EAI_FAIL;
    addrinfo *first = nullptr;
    addrinfo *second = nullptr;
    int error = resolver("127.0.0.2", service, hints, &first);
    if (error)
      return error;
    if (std::strcmp(name, "vscp-stalled.invalid") == 0) {
      *result = first;
      return 0;
    }
    error = resolver("127.0.0.1", service, hints, &second);
    if (error) {
      freeaddrinfo(first);
      return error;
    }
    addrinfo *last = first;
    while (last->ai_next)
      last = last->ai_next;
    last->ai_next = second;
    *result = first;
    return 0;
  }
  if (!name || std::strcmp(name, "vscp-slow.invalid") != 0)
    return EAI_NONAME;
  lookupStarted.store(true);
  std::this_thread::sleep_for(std::chrono::seconds(2));
  *result = nullptr;
  lookupFinished.store(true);
  return EAI_NONAME;
}

// Simulate an address that stays pending while other real sockets remain usable.
extern "C" int
connect(int socket, const struct sockaddr *address, socklen_t length)
{
  if (address && address->sa_family == AF_INET && length >= sizeof(sockaddr_in)) {
    auto ip = reinterpret_cast<const sockaddr_in *>(address)->sin_addr.s_addr;
    if (ip == htonl(0x7f000002) || ip == htonl(INADDR_LOOPBACK))
      ++connectionAttempts;
    if (simulateStall.load() && ip == htonl(0x7f000002)) {
      stalledSocket.store(socket);
      errno = EINPROGRESS;
      return -1;
    }
  }
  using Connect = int (*)(int, const struct sockaddr *, socklen_t);
  static auto realConnect = reinterpret_cast<Connect>(dlsym(RTLD_NEXT, "connect"));
  if (!realConnect) {
    errno = ENOSYS;
    return -1;
  }
  return realConnect(socket, address, length);
}

extern "C" int
poll(struct pollfd *descriptors, nfds_t count, int timeout)
{
  int stalled = stalledSocket.load();
  nfds_t ignored = count;
  for (nfds_t i = 0; i < count; ++i) {
    if (stalled >= 0 && descriptors[i].fd == stalled) {
      ignored = i;
      descriptors[i].fd = -1;
      break;
    }
  }
  using Poll = int (*)(struct pollfd *, nfds_t, int);
  static auto realPoll = reinterpret_cast<Poll>(dlsym(RTLD_NEXT, "poll"));
  if (!realPoll) {
    if (ignored < count)
      descriptors[ignored].fd = stalled;
    errno = ENOSYS;
    return -1;
  }
  int result = realPoll(descriptors, count, timeout);
  if (ignored < count)
    descriptors[ignored].fd = stalled;
  return result;
}

TEST(TcpResolver, TimeoutAndShutdown)
{
  ASSERT_EQ(0, std::atexit(checkResolverFinished));
  VscpRemoteTcpIf client;
  client.setConnectTimeout(1);
  auto start = std::chrono::steady_clock::now();
  ASSERT_EQ(VSCP_ERROR_TIMEOUT, client.doCmdOpen("tcp://vscp-slow.invalid:9598", "admin", "secret"));
  auto elapsed = std::chrono::steady_clock::now() - start;
  ASSERT_TRUE(lookupStarted.load());
  ASSERT_FALSE(lookupFinished.load());
  ASSERT_GE(elapsed, std::chrono::milliseconds(990));
  ASSERT_LT(elapsed, std::chrono::milliseconds(1400));
  ASSERT_FALSE(client.isConnected());
}

TEST(TcpResolver, StalledAddressFallsBack)
{
  TestSocket listener(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
  ASSERT_GE(listener.value, 0);
  sockaddr_in address = {};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  ASSERT_EQ(0, bind(listener.value, reinterpret_cast<sockaddr *>(&address), sizeof(address)));
  ASSERT_EQ(0, listen(listener.value, 1));
  socklen_t length = sizeof(address);
  ASSERT_EQ(0, getsockname(listener.value, reinterpret_cast<sockaddr *>(&address), &length));

  VscpRemoteTcpIf client;
  client.setConnectTimeout(2);
  client.setResponseTimeout(150);
  connectionAttempts.store(0);
  simulateStall.store(true);
  auto start = std::chrono::steady_clock::now();
  int result = client.doCmdOpen("tcp://vscp-fallback.invalid:" + std::to_string(ntohs(address.sin_port)),
                                "admin", "secret");
  int stalled = stalledSocket.exchange(-1);
  simulateStall.store(false);
  auto elapsed = std::chrono::steady_clock::now() - start;
  // The TCP connection succeeds, but no VSCP greeting is sent by this listener.
  ASSERT_EQ(VSCP_ERROR_CONNECTION, result);
  ASSERT_GE(elapsed, std::chrono::milliseconds(200));
  ASSERT_LT(elapsed, std::chrono::milliseconds(1300));
  ASSERT_FALSE(client.isConnected());
  ASSERT_EQ(2, connectionAttempts.load());
  ASSERT_GE(stalled, 0);
  int flags = fcntl(stalled, F_GETFD);
  int error = errno;
  ASSERT_EQ(-1, flags);
  ASSERT_EQ(EBADF, error);
  pollfd descriptor = {listener.value, POLLIN, 0};
  ASSERT_EQ(1, poll(&descriptor, 1, 100));
  TestSocket accepted(accept(listener.value, nullptr, nullptr));
  ASSERT_GE(accepted.value, 0);
}

TEST(TcpResolver, AllAddressesFailPromptly)
{
  TestSocket reserved(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
  ASSERT_GE(reserved.value, 0);
  sockaddr_in address = {};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  ASSERT_EQ(0, bind(reserved.value, reinterpret_cast<sockaddr *>(&address), sizeof(address)));
  socklen_t length = sizeof(address);
  ASSERT_EQ(0, getsockname(reserved.value, reinterpret_cast<sockaddr *>(&address), &length));

  VscpRemoteTcpIf client;
  client.setConnectTimeout(2);
  connectionAttempts.store(0);
  auto start = std::chrono::steady_clock::now();
  ASSERT_EQ(VSCP_ERROR_CONNECTION,
            client.doCmdOpen("tcp://vscp-fallback.invalid:" + std::to_string(ntohs(address.sin_port)),
                             "admin", "secret"));
  ASSERT_EQ(2, connectionAttempts.load());
  ASSERT_LT(std::chrono::steady_clock::now() - start, std::chrono::milliseconds(1000));
  ASSERT_FALSE(client.isConnected());
}

TEST(TcpResolver, PendingAddressTimesOutAndCloses)
{
  VscpRemoteTcpIf client;
  client.setConnectTimeout(1);
  simulateStall.store(true);
  auto start = std::chrono::steady_clock::now();
  int result = client.doCmdOpen("tcp://vscp-stalled.invalid:9598", "admin", "secret");
  int stalled = stalledSocket.exchange(-1);
  simulateStall.store(false);
  auto elapsed = std::chrono::steady_clock::now() - start;
  ASSERT_EQ(VSCP_ERROR_TIMEOUT, result);
  ASSERT_GE(elapsed, std::chrono::milliseconds(990));
  ASSERT_LT(elapsed, std::chrono::milliseconds(1400));
  ASSERT_GE(stalled, 0);
  int flags = fcntl(stalled, F_GETFD);
  int error = errno;
  ASSERT_EQ(-1, flags);
  ASSERT_EQ(EBADF, error);
  ASSERT_FALSE(client.isConnected());
}
