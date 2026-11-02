#ifndef VSCP_DATAGRAM_TEST_HELPERS_H
#define VSCP_DATAGRAM_TEST_HELPERS_H

#include <vscp-socket-address.h>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <chrono>
#include <cstdint>
#include <string>
#include <thread>
#include <vector>

#ifdef WIN32
using TestSocket = SOCKET;
constexpr TestSocket invalidTestSocket = INVALID_SOCKET;
#else
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>
using TestSocket = int;
constexpr TestSocket invalidTestSocket = -1;
#endif

template<typename Client>
void
verifyDatagramJsonOptions(const std::string &address)
{
  using json = nlohmann::json;
  Client client;
  ASSERT_TRUE(client.initFromJson(json{{"ip", address},
                                       {"encryption", VSCP_ENCRYPTION_AES128},
                                       {"key", "0102030405060708090A0B0C0D0E0F10"}}.dump()));
  EXPECT_EQ(VSCP_ENCRYPTION_AES128, client.m_encryptType);
  EXPECT_EQ(0x01, client.m_key[0]);
  EXPECT_EQ(0x10, client.m_key[15]);

  const std::string filterGuid = "01:02:03:04:05:06:07:08:09:0A:0B:0C:0D:0E:0F:10";
  const std::string maskGuid = "FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF:FF";
  ASSERT_TRUE(client.initFromJson(json{{"class-filter", 20},
                                       {"class-mask", 65535},
                                       {"guid-filter", filterGuid},
                                       {"guid-mask", maskGuid}}.dump()));
  EXPECT_EQ(20, client.m_filter.filter_class);
  EXPECT_EQ(0xffff, client.m_filter.mask_class);
  EXPECT_EQ(0x01, client.m_filter.filter_GUID[0]);
  EXPECT_EQ(0x10, client.m_filter.filter_GUID[15]);
  EXPECT_EQ(0xff, client.m_filter.mask_GUID[15]);
  EXPECT_EQ(0x10, client.m_key[15]);

  const json accepted = json::parse(client.getConfigAsJson());
  const json acceptedInput = client.m_j_config;
  EXPECT_EQ(filterGuid, accepted["guid-filter"]);
  EXPECT_EQ(maskGuid, accepted["guid-mask"]);
  const std::vector<json> invalid = {
    json{{"ip", "[::2]:1234"}, {"encryption", VSCP_ENCRYPTION_AES256}, {"key", "1234"}},
    json{{"key", 42}},
    json{{"key", std::string(32, 'z')}},
    json{{"guid-filter", "invalid"}},
    json{{"guid-mask", nullptr}},
    json{{"class-filter", -1}},
    json{{"priority-mask", 256}},
    json{{"type-mask", 65536}},
    json::array()
  };
  for (const auto &configuration : invalid) {
    EXPECT_FALSE(client.initFromJson(configuration.dump())) << configuration;
    EXPECT_EQ(accepted, json::parse(client.getConfigAsJson()));
    EXPECT_EQ(acceptedInput, client.m_j_config);
  }

  Client aes192;
  ASSERT_TRUE(aes192.initFromJson(json{{"encryption", VSCP_ENCRYPTION_AES192},
                                        {"key", "0102030405060708090A0B0C0D0E0F101112131415161718"}}.dump()));
  EXPECT_EQ(VSCP_ENCRYPTION_AES192, aes192.m_encryptType);
  EXPECT_EQ(0x01, aes192.m_key[0]);
  EXPECT_EQ(0x18, aes192.m_key[23]);
  Client restored;
  ASSERT_TRUE(restored.initFromJson(aes192.getConfigAsJson()));
  EXPECT_EQ(aes192.m_encryptType, restored.m_encryptType);
  EXPECT_EQ(0x18, restored.m_key[23]);
}

inline void
closeTestSocket(TestSocket socket)
{
#ifdef WIN32
  closesocket(socket);
#else
  close(socket);
#endif
}

inline uint16_t
unusedDatagramPort(int family)
{
  TestSocket socket = ::socket(family, SOCK_DGRAM, IPPROTO_UDP);
  if (socket == invalidTestSocket) {
    return 0;
  }
  sockaddr_storage address{};
#ifdef WIN32
  int size;
#else
  socklen_t size;
#endif
  if (family == AF_INET6) {
    auto *ipv6 = reinterpret_cast<sockaddr_in6 *>(&address);
    ipv6->sin6_family = AF_INET6;
#ifdef __APPLE__
    ipv6->sin6_len = sizeof(*ipv6);
#endif
    ipv6->sin6_addr = in6addr_loopback;
    size = sizeof(*ipv6);
  }
  else {
    auto *ipv4 = reinterpret_cast<sockaddr_in *>(&address);
    ipv4->sin_family = AF_INET;
#ifdef __APPLE__
    ipv4->sin_len = sizeof(*ipv4);
#endif
    ipv4->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    size = sizeof(*ipv4);
  }
  if (bind(socket, reinterpret_cast<sockaddr *>(&address), size) != 0 ||
      getsockname(socket, reinterpret_cast<sockaddr *>(&address), &size) != 0) {
    closeTestSocket(socket);
    return 0;
  }
  const uint16_t port = family == AF_INET6 ?
    ntohs(reinterpret_cast<sockaddr_in6 *>(&address)->sin6_port) :
    ntohs(reinterpret_cast<sockaddr_in *>(&address)->sin_port);
  closeTestSocket(socket);
  return port;
}

inline bool
sendRawLoopbackDatagram(int family, uint16_t port, const uint8_t *data, size_t length)
{
  TestSocket socket = ::socket(family, SOCK_DGRAM, IPPROTO_UDP);
  if (socket == invalidTestSocket) {
    return false;
  }
  sockaddr_storage address{};
  int size;
  if (family == AF_INET6) {
    auto *ipv6 = reinterpret_cast<sockaddr_in6 *>(&address);
    ipv6->sin6_family = AF_INET6;
#ifdef __APPLE__
    ipv6->sin6_len = sizeof(*ipv6);
#endif
    ipv6->sin6_addr = in6addr_loopback;
    ipv6->sin6_port = htons(port);
    size = sizeof(*ipv6);
  }
  else {
    auto *ipv4 = reinterpret_cast<sockaddr_in *>(&address);
    ipv4->sin_family = AF_INET;
#ifdef __APPLE__
    ipv4->sin_len = sizeof(*ipv4);
#endif
    ipv4->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    ipv4->sin_port = htons(port);
    size = sizeof(*ipv4);
  }
  const int sent = static_cast<int>(sendto(socket, reinterpret_cast<const char *>(data),
                                            static_cast<int>(length), 0,
                                            reinterpret_cast<const sockaddr *>(&address), size));
  closeTestSocket(socket);
  return sent == static_cast<int>(length);
}

template<typename Client>
inline bool
waitForDatagramCount(Client &client, uint16_t expected)
{
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
  while (std::chrono::steady_clock::now() < deadline) {
    uint16_t count = 0;
    if (client.getcount(&count) == VSCP_ERROR_SUCCESS && count >= expected) {
      return true;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }
  return false;
}

#endif
