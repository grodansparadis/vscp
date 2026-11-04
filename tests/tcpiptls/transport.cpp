#include <gtest/gtest.h>
#include <vscpremotetcpif.h>
#include <vscp-client-tcp.h>
#include <vscphelperlogging.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#include <crc.h>

namespace {
std::string setting(const char *name)
{
  const char *value = std::getenv(name);
  return value ? value : "";
}

class TcpTransport : public testing::Test {
protected:
  VscpRemoteTcpIf client;

  void SetUp() override
  {
    ASSERT_FALSE(setting("VSCP_TEST_PLAIN").empty());
    client.setConnectTimeout(1);
    client.setResponseTimeout(400);
  }

  void open(const char *endpoint)
  {
    ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdOpen(setting(endpoint), "admin", "secret"));
    ASSERT_TRUE(client.isConnected());
    ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdNOOP());
  }
};

bool waitForCount(vscpClientTcp &client, uint16_t expected)
{
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
  while (std::chrono::steady_clock::now() < deadline) {
    uint16_t count = 0;
    if (client.getcount(&count) != VSCP_ERROR_SUCCESS) {
      return false;
    }
    if (count == expected) {
      return true;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return false;
}
}

TEST_F(TcpTransport, PlainAndReconnect)
{
  open("VSCP_TEST_PLAIN");
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdClose());
  ASSERT_FALSE(client.isConnected());
  open("VSCP_TEST_PLAIN");
}

TEST_F(TcpTransport, NumericIpv4Endpoint)
{
  auto endpoint = setting("VSCP_TEST_PLAIN");
  endpoint.replace(endpoint.find("localhost"), 9, "127.0.0.1");
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdOpen(endpoint, "admin", "secret"));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdNOOP());
}

TEST_F(TcpTransport, BracketedIpv6Endpoint)
{
  if (setting("VSCP_TEST_IPV6").empty())
    GTEST_SKIP() << "IPv6 loopback is unavailable";
  open("VSCP_TEST_IPV6");
}

TEST_F(TcpTransport, ZeroConnectTimeoutAndReconnect)
{
  client.setConnectTimeout(0);
  ASSERT_EQ(VSCP_ERROR_TIMEOUT, client.doCmdOpen(setting("VSCP_TEST_PLAIN"), "admin", "secret"));
  ASSERT_FALSE(client.isConnected());
  client.setConnectTimeout(1);
  open("VSCP_TEST_PLAIN");
}

TEST_F(TcpTransport, SecurePrefixAndUnverifiedCertificate)
{
  client.setTLSOptions(false);
  client.setTLSAutoSelect();
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdOpen(setting("VSCP_TEST_TLS") + ";admin;secret"));
  ASSERT_TRUE(client.m_bTLS);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdNOOP());
}

TEST_F(TcpTransport, ForcePlainOverridesSecurePrefix)
{
  client.enableTLS(false);
  auto endpoint = setting("VSCP_TEST_PLAIN");
  endpoint.replace(0, 3, "stcp");
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdOpen(endpoint, "admin", "secret"));
  ASSERT_FALSE(client.m_bTLS);
}

TEST_F(TcpTransport, ForceTlsOverridesPlainPrefix)
{
  client.enableTLS();
  auto endpoint = setting("VSCP_TEST_TLS");
  endpoint.replace(0, 4, "tcp");
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdOpen(endpoint, "admin", "secret"));
  ASSERT_TRUE(client.m_bTLS);
}

TEST_F(TcpTransport, TrustedCertificateAndHostname)
{
  client.setTLSOptions(true, setting("VSCP_TEST_CA"));
  open("VSCP_TEST_TLS");
}

TEST_F(TcpTransport, CaDirectory)
{
  client.setTLSOptions(true, "", setting("VSCP_TEST_CA_DIR"));
  open("VSCP_TEST_TLS");
}

TEST_F(TcpTransport, DefaultTrustPaths)
{
  ASSERT_TRUE(client.m_bVerifyPeer);
  open("VSCP_TEST_TLS");
}

TEST_F(TcpTransport, DefaultVerificationRejectsUntrustedCertificate)
{
  ASSERT_TRUE(client.m_bVerifyPeer);
  ASSERT_EQ(VSCP_ERROR_CONNECTION, client.doCmdOpen(setting("VSCP_TEST_UNTRUSTED"), "admin", "secret"));
  ASSERT_FALSE(client.isConnected());
}

TEST_F(TcpTransport, DefaultVerificationRejectsWrongHostname)
{
  auto endpoint = setting("VSCP_TEST_TLS");
  endpoint.replace(endpoint.find("localhost"), 9, "127.0.0.1");
  ASSERT_EQ(VSCP_ERROR_CONNECTION, client.doCmdOpen(endpoint, "admin", "secret"));
  ASSERT_FALSE(client.isConnected());
}

TEST_F(TcpTransport, WrapperDefaultVerificationRejectsUntrustedCertificate)
{
  vscpClientTcp wrapper;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.init(setting("VSCP_TEST_UNTRUSTED"), "admin", "secret", true));
  ASSERT_EQ(VSCP_ERROR_CONNECTION, wrapper.connect());
  ASSERT_FALSE(wrapper.isConnected());
}

TEST_F(TcpTransport, WrapperAutoModePropagatesVerificationOptions)
{
  vscpClientTcp wrapper;
  wrapper.setTLSOptions(true, setting("VSCP_TEST_WRONG_CA"));
  wrapper.setTLSAutoSelect();
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.init(setting("VSCP_TEST_TLS"), "admin", "secret", true));
  ASSERT_EQ(VSCP_ERROR_CONNECTION, wrapper.connect());
  ASSERT_FALSE(wrapper.isConnected());
}

TEST_F(TcpTransport, RejectUntrustedCertificate)
{
  client.setTLSOptions(true, setting("VSCP_TEST_WRONG_CA"));
  ASSERT_EQ(VSCP_ERROR_CONNECTION, client.doCmdOpen(setting("VSCP_TEST_TLS"), "admin", "secret"));
  ASSERT_FALSE(client.isConnected());
}

TEST_F(TcpTransport, RejectWrongHostname)
{
  client.setTLSOptions(true, setting("VSCP_TEST_CA"));
  auto endpoint = setting("VSCP_TEST_TLS");
  endpoint.replace(endpoint.find("localhost"), 9, "127.0.0.1");
  ASSERT_EQ(VSCP_ERROR_CONNECTION, client.doCmdOpen(endpoint, "admin", "secret"));
  ASSERT_FALSE(client.isConnected());
}

TEST_F(TcpTransport, MutualTlsAndEncryptedKey)
{
  client.setTLSOptions(true, setting("VSCP_TEST_CA"), "",
                       setting("VSCP_TEST_CERT"), setting("VSCP_TEST_KEY"), "test-password");
  open("VSCP_TEST_MTLS");
}

TEST_F(TcpTransport, RejectMissingClientCertificate)
{
  client.setTLSOptions(true, setting("VSCP_TEST_CA"));
  ASSERT_NE(VSCP_ERROR_SUCCESS, client.doCmdOpen(setting("VSCP_TEST_MTLS"), "admin", "secret"));
  ASSERT_FALSE(client.isConnected());
}

TEST_F(TcpTransport, RejectInvalidTlsFilesAndPassword)
{
  client.setTLSOptions(true, setting("VSCP_TEST_CA") + ".missing");
  ASSERT_EQ(VSCP_ERROR_CONNECTION, client.doCmdOpen(setting("VSCP_TEST_TLS"), "admin", "secret"));
  client.setTLSOptions(true, setting("VSCP_TEST_KEY"));
  ASSERT_EQ(VSCP_ERROR_CONNECTION, client.doCmdOpen(setting("VSCP_TEST_TLS"), "admin", "secret"));
  client.setTLSOptions(true, setting("VSCP_TEST_CA"), "",
                       setting("VSCP_TEST_CERT"), setting("VSCP_TEST_KEY"), "wrong-password");
  ASSERT_EQ(VSCP_ERROR_CONNECTION, client.doCmdOpen(setting("VSCP_TEST_TLS"), "admin", "secret"));
  ASSERT_FALSE(client.isConnected());
}

TEST_F(TcpTransport, ReceiveLoopAndTimeout)
{
  open("VSCP_TEST_PLAIN");
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdEnterReceiveLoop());
  vscpEventEx event = {};
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdBlockingReceive(&event, 500));
  ASSERT_EQ(10, event.vscp_class);
  ASSERT_EQ(6, event.vscp_type);
  auto start = std::chrono::steady_clock::now();
  ASSERT_EQ(VSCP_ERROR_TIMEOUT, client.doCmdBlockingReceive(&event, 100));
  auto elapsed = std::chrono::steady_clock::now() - start;
  ASSERT_GE(elapsed, std::chrono::milliseconds(90));
  ASSERT_LT(elapsed, std::chrono::milliseconds(350));
}

TEST_F(TcpTransport, SingleChannelReceiveLoopAllowsCommandsAndEvents)
{
  open("VSCP_TEST_PLAIN");
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.setSingleChannelReceiveLoop());
  ASSERT_TRUE(client.isSingleChannelReceiveLoopEnabled());
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdEnterReceiveLoop());
  EXPECT_EQ(VSCP_ERROR_PARAMETER, client.doCommand("VERS\r\n"));

  vscpEventEx outgoing = {};
  outgoing.vscp_class = 10;
  outgoing.vscp_type = 6;
  outgoing.sizeData = 3;
  outgoing.data[0] = 4;
  outgoing.data[1] = 5;
  outgoing.data[2] = 6;

  int sendResult = VSCP_ERROR_ERROR;
  std::thread sender([&]() { sendResult = client.doCmdSendEx(&outgoing); });

  vscpEventEx received = {};
  int receiveResult1 = client.doCmdBlockingReceive(&received, 1000);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, receiveResult1);
  EXPECT_EQ(10, received.vscp_class);
  EXPECT_EQ(6, received.vscp_type);

  int receiveResult2 = client.doCmdBlockingReceive(&received, 1000);
  sender.join();
  EXPECT_EQ(VSCP_ERROR_SUCCESS, sendResult);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, receiveResult2);
  EXPECT_EQ(10, received.vscp_class);
  EXPECT_EQ(6, received.vscp_type);

  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdQuitReceiveLoop());
  EXPECT_TRUE(client.isSingleChannelReceiveLoopEnabled());
}

TEST_F(TcpTransport, SingleChannelClearDiscardsBufferedEvents)
{
  open("VSCP_TEST_PLAIN");
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.setSingleChannelReceiveLoop());
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdEnterReceiveLoop());
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.rcvloopRead(1000));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdClear());

  vscpEvent event = {};
  int result = client.doCmdBlockingReceive(&event, 100);
  vscp_deleteEvent(&event);
  EXPECT_EQ(VSCP_ERROR_TIMEOUT, result);
}

TEST_F(TcpTransport, SingleChannelRejectsOversizedEvents)
{
  open("VSCP_TEST_PLAIN");
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCommand("SCENARIO oversized-text\r\n"));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.setSingleChannelReceiveLoop());
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdEnterReceiveLoop());
  vscpEvent event = {};
  EXPECT_EQ(VSCP_ERROR_READ, client.doCmdBlockingReceive(&event, 1000));
  vscp_deleteEvent(&event);
}

TEST_F(TcpTransport, WrapperSingleChannelUsesOneConnection)
{
  vscpClientTcp wrapper;
  wrapper.setConnectionTimeout(1000);
  wrapper.setResponseTimeout(400);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.init(setting("VSCP_TEST_SINGLE"), "admin", "secret"));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.setSingleChannelReceiveLoop());
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.connect());
  ASSERT_TRUE(wrapper.isConnected());
  EXPECT_EQ(1U, wrapper.m_obid);
  EXPECT_TRUE(wrapper.getTcpReceive()->isSingleChannelReceiveLoopEnabled());
  EXPECT_EQ(VSCP_ERROR_PARAMETER, wrapper.setSingleChannelReceiveLoop(false));
  EXPECT_EQ(VSCP_ERROR_ALREADY_DEFINED, wrapper.connect());
  EXPECT_EQ(VSCP_ERROR_PARAMETER, wrapper.init(setting("VSCP_TEST_PLAIN"), "admin", "secret"));
  EXPECT_FALSE(wrapper.initFromJson(R"({"single-channel":true})"));
  EXPECT_EQ(VSCP_ERROR_NOT_SUPPORTED, wrapper.setCallbackEv([](vscpEvent &, void *) {}));
  EXPECT_EQ(VSCP_ERROR_NOT_SUPPORTED, wrapper.setCallbackEx([](vscpEventEx &, void *) {}));

  VscpRemoteTcpIf extra;
  extra.setConnectTimeout(1);
  extra.setResponseTimeout(300);
  EXPECT_NE(VSCP_ERROR_SUCCESS, extra.doCmdOpen(setting("VSCP_TEST_SINGLE"), "admin", "secret"));
  EXPECT_TRUE(wrapper.isConnected());

  vscpEvent first = {};
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.receiveBlocking(first, 1000));
  EXPECT_EQ(10, first.vscp_class);
  EXPECT_EQ(6, first.vscp_type);
  EXPECT_EQ(0xffff, first.year);
  EXPECT_GT(first.timestamp_ns, 0U);
  ASSERT_EQ(3, first.sizeData);
  EXPECT_EQ(1, first.pdata[0]);
  vscp_deleteEvent(&first);

  vscpEventEx received = {};
  EXPECT_EQ(VSCP_ERROR_RCV_EMPTY, wrapper.receive(received));
  EXPECT_EQ(VSCP_ERROR_TIMEOUT, wrapper.receiveBlocking(received, 50));
  EXPECT_EQ(VSCP_ERROR_PARAMETER, wrapper.receiveBlocking(received, -1));

  vscpEventEx outgoing = {};
  outgoing.vscp_class = 10;
  outgoing.vscp_type = 6;
  outgoing.sizeData = 3;
  outgoing.data[0] = 4;
  outgoing.data[1] = 5;
  outgoing.data[2] = 6;
  int sendResult = VSCP_ERROR_ERROR;
  std::thread sender([&]() { sendResult = wrapper.send(outgoing); });
  int receiveResult = wrapper.receiveBlocking(received, 1000);
  sender.join();
  ASSERT_EQ(VSCP_ERROR_SUCCESS, sendResult);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, receiveResult);
  EXPECT_EQ(10, received.vscp_class);
  ASSERT_EQ(3, received.sizeData);
  EXPECT_EQ(4, received.data[0]);
  EXPECT_GT(received.timestamp_ns, 0U);

  canalMsg outgoingCanal = {};
  outgoingCanal.id = (10U << 16) | (6U << 8) | 1U;
  outgoingCanal.sizeData = 2;
  outgoingCanal.data[0] = 7;
  outgoingCanal.data[1] = 8;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.send(outgoingCanal));
  canalMsg receivedCanal = {};
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.receiveBlocking(receivedCanal, 1000));
  ASSERT_EQ(3, receivedCanal.sizeData);
  EXPECT_EQ(4, receivedCanal.data[0]);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, wrapper.disconnect());
  EXPECT_FALSE(wrapper.isConnected());
}

TEST_F(TcpTransport, WrapperSingleChannelFiltersCountsAndClears)
{
  vscpClientTcp wrapper;
  wrapper.setResponseTimeout(400);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.init(setting("VSCP_TEST_PLAIN"), "admin", "secret"));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.setSingleChannelReceiveLoop());

  vscpEventFilter filter = {};
  vscp_clearVSCPFilter(&filter);
  filter.filter_class = 20;
  filter.mask_class = 0xffff;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.setfilter(filter));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.connect());

  vscpEventEx received = {};
  EXPECT_EQ(VSCP_ERROR_TIMEOUT, wrapper.receiveBlocking(received, 150));
  vscp_clearVSCPFilter(&filter);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.setfilter(filter));

  vscpEventEx outgoing = {};
  outgoing.vscp_class = 10;
  outgoing.vscp_type = 6;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.send(outgoing));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.send(outgoing));
  ASSERT_TRUE(waitForCount(wrapper, 2));
  const auto clearStart = std::chrono::steady_clock::now();
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.clear());
  EXPECT_LT(std::chrono::steady_clock::now() - clearStart, std::chrono::seconds(1));
  uint16_t count = 99;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.getcount(&count));
  EXPECT_EQ(0, count);
  EXPECT_EQ(VSCP_ERROR_RCV_EMPTY, wrapper.receive(received));

  uint8_t major = 0, minor = 0, release = 0, build = 0;
  std::deque<std::string> interfaces;
  uint64_t wcyd = 0;
  EXPECT_EQ(VSCP_ERROR_NOT_SUPPORTED, wrapper.getversion(&major, &minor, &release, &build));
  EXPECT_EQ(VSCP_ERROR_NOT_SUPPORTED, wrapper.getinterfaces(interfaces));
  EXPECT_EQ(VSCP_ERROR_NOT_SUPPORTED, wrapper.getwcyd(wcyd));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.send(outgoing));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.receiveBlocking(received, 1000));
  EXPECT_EQ(4, received.data[0]);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, wrapper.disconnect());
}

TEST_F(TcpTransport, WrapperSingleChannelTlsCallbacksCanSend)
{
  struct CallbackState {
    std::mutex mutex;
    std::condition_variable condition;
    int eventCount = 0;
    int exCount = 0;
    int sendResult = VSCP_ERROR_ERROR;
    int disconnectResult = VSCP_ERROR_ERROR;
    bool invalid = false;
  } state;

  vscpClientTcp wrapper;
  wrapper.setTLSOptions(true, setting("VSCP_TEST_CA"));
  wrapper.setResponseTimeout(1000);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.init(setting("VSCP_TEST_SINGLE_TLS"), "admin", "secret"));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.setSingleChannelReceiveLoop());
  vscpEventEx outgoing = {};
  outgoing.vscp_class = 10;
  outgoing.vscp_type = 6;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.setCallbackEv([&](vscpEvent &event, void *) {
    int number;
    {
      std::lock_guard<std::mutex> lock(state.mutex);
      state.invalid = state.invalid || event.vscp_class != 10 || event.sizeData != 3;
      number = ++state.eventCount;
    }
    if (1 == number) {
      int disconnectResult = wrapper.disconnect();
      int result = wrapper.send(outgoing);
      std::lock_guard<std::mutex> lock(state.mutex);
      state.disconnectResult = disconnectResult;
      state.sendResult = result;
    }
    state.condition.notify_all();
  }, &state));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.setCallbackEx([&](vscpEventEx &event, void *) {
    {
      std::lock_guard<std::mutex> lock(state.mutex);
      state.invalid = state.invalid || event.vscp_class != 10 || event.sizeData != 3;
      ++state.exCount;
    }
    state.condition.notify_all();
  }, &state));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.connect());

  constexpr int sends = 8;
  for (int i = 0; i < sends; ++i) {
    EXPECT_EQ(VSCP_ERROR_SUCCESS, wrapper.send(outgoing));
  }
  {
    std::unique_lock<std::mutex> lock(state.mutex);
    EXPECT_TRUE(state.condition.wait_for(lock, std::chrono::seconds(5), [&]() {
      return state.eventCount >= sends + 2 && state.exCount >= sends + 2;
    }));
    EXPECT_EQ(VSCP_ERROR_NOT_SUPPORTED, state.disconnectResult);
    EXPECT_EQ(VSCP_ERROR_SUCCESS, state.sendResult);
    EXPECT_FALSE(state.invalid);
  }
  vscpEventEx received = {};
  EXPECT_EQ(VSCP_ERROR_NOT_SUPPORTED, wrapper.receive(received));
  EXPECT_EQ(VSCP_ERROR_SUCCESS, wrapper.disconnect());
}

TEST_F(TcpTransport, WrapperSingleChannelJsonReconnects)
{
  vscpClientTcp configured;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, configured.init(setting("VSCP_TEST_PLAIN"), "admin", "secret"));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, configured.setSingleChannelReceiveLoop());
  vscpClientTcp wrapper;
  ASSERT_TRUE(wrapper.initFromJson(configured.getConfigAsJson()));

  for (int i = 0; i < 2; ++i) {
    ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.connect());
    vscpEventEx received = {};
    ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.receiveBlocking(received, 1000));
    EXPECT_EQ(1, received.data[0]);
    vscpEventEx outgoing = {};
    outgoing.vscp_class = 10;
    outgoing.vscp_type = 6;
    ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.send(outgoing));
    ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.receiveBlocking(received, 1000));
    EXPECT_EQ(4, received.data[0]);
    ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.disconnect());
    EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, wrapper.receive(received));
  }
}

TEST_F(TcpTransport, WrapperSingleChannelMultipleReceivers)
{
  vscpClientTcp wrapper;
  wrapper.setResponseTimeout(400);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.init(setting("VSCP_TEST_PLAIN"), "admin", "secret"));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.setSingleChannelReceiveLoop());
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.connect());
  vscpEventEx first = {};
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.receiveBlocking(first, 1000));

  constexpr int count = 3;
  std::atomic<int> started{0};
  int results[count] = {};
  std::vector<std::thread> receivers;
  for (int i = 0; i < count; ++i) {
    receivers.emplace_back([&, i]() {
      ++started;
      vscpEventEx event = {};
      results[i] = wrapper.receiveBlocking(event, 2000);
    });
  }
  while (started < count) {
    std::this_thread::yield();
  }
  vscpEventEx outgoing = {};
  outgoing.vscp_class = 10;
  outgoing.vscp_type = 6;
  for (int i = 0; i < count; ++i) {
    EXPECT_EQ(VSCP_ERROR_SUCCESS, wrapper.send(outgoing));
  }
  for (auto &receiver : receivers) {
    receiver.join();
  }
  for (int result : results) {
    EXPECT_EQ(VSCP_ERROR_SUCCESS, result);
  }
  EXPECT_EQ(VSCP_ERROR_RCV_EMPTY, wrapper.receive(first));
  EXPECT_EQ(VSCP_ERROR_SUCCESS, wrapper.disconnect());
}

TEST_F(TcpTransport, WrapperDefaultTwoChannelMode)
{
  vscpClientTcp wrapper;
  wrapper.setResponseTimeout(400);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.init(setting("VSCP_TEST_PLAIN"), "admin", "secret"));
  std::atomic<int> callbacks{0};
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.setCallbackEv([&](vscpEvent &, void *) { ++callbacks; }));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.setCallbackEx([&](vscpEventEx &, void *) { ++callbacks; }));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.connect());
  EXPECT_FALSE(wrapper.isSingleChannelReceiveLoopEnabled());
  EXPECT_TRUE(wrapper.isConnected());
  EXPECT_TRUE(wrapper.getTcpReceive()->isConnected());
  vscpEventEx outgoing = {};
  outgoing.vscp_class = 10;
  outgoing.vscp_type = 6;
  EXPECT_EQ(VSCP_ERROR_SUCCESS, wrapper.send(outgoing));
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
  while (callbacks < 2 && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  EXPECT_EQ(2, callbacks.load());
  EXPECT_EQ(VSCP_ERROR_SUCCESS, wrapper.disconnect());
}

TEST_F(TcpTransport, WrapperPollingModeSendsCanal)
{
  vscpClientTcp wrapper;
  wrapper.setResponseTimeout(400);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.init(setting("VSCP_TEST_PLAIN"), "admin", "secret", true));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.connect());
  EXPECT_FALSE(wrapper.isSingleChannelReceiveLoopEnabled());
  canalMsg outgoing = {};
  outgoing.id = (10U << 16) | (6U << 8) | 1U;
  outgoing.sizeData = 1;
  outgoing.data[0] = 7;
  EXPECT_EQ(VSCP_ERROR_SUCCESS, wrapper.send(outgoing));
  EXPECT_EQ(VSCP_ERROR_SUCCESS, wrapper.disconnect());
}

TEST_F(TcpTransport, WrapperSingleChannelHandlesReceiveLoopRejection)
{
  vscpClientTcp wrapper;
  wrapper.setResponseTimeout(400);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.init(setting("VSCP_TEST_REJECT_LOOP"), "admin", "secret"));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.setSingleChannelReceiveLoop());
  EXPECT_NE(VSCP_ERROR_SUCCESS, wrapper.connect());
  EXPECT_FALSE(wrapper.isConnected());
  EXPECT_EQ(VSCP_ERROR_SUCCESS, wrapper.disconnect());
  EXPECT_EQ(VSCP_ERROR_SUCCESS, wrapper.setSingleChannelReceiveLoop(false));
}

TEST_F(TcpTransport, WrapperSingleChannelCallbackExceptionStopsWorker)
{
  vscpClientTcp wrapper;
  wrapper.setResponseTimeout(400);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.init(setting("VSCP_TEST_PLAIN"), "admin", "secret"));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.setSingleChannelReceiveLoop());
  std::atomic<bool> invoked{false};
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.setCallbackEv([&](vscpEvent &, void *) {
    invoked = true;
    throw std::runtime_error("callback failure");
  }));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.connect());
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
  while (wrapper.isConnected() && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  EXPECT_TRUE(invoked);
  EXPECT_FALSE(wrapper.isConnected());
  vscpEventEx outgoing = {};
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, wrapper.send(outgoing));
  EXPECT_EQ(VSCP_ERROR_SUCCESS, wrapper.disconnect());
}

TEST_F(TcpTransport, WrapperSingleChannelDisconnectWakesReceiver)
{
  vscpClientTcp wrapper;
  wrapper.setResponseTimeout(400);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.init(setting("VSCP_TEST_PLAIN"), "admin", "secret"));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.setSingleChannelReceiveLoop());
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.connect());
  vscpEventEx event = {};
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.receiveBlocking(event, 1000));

  std::atomic<bool> started{false};
  int receiveResult = VSCP_ERROR_SUCCESS;
  std::thread receiver([&]() {
    started = true;
    vscpEventEx next = {};
    receiveResult = wrapper.receiveBlocking(next, 5000);
  });
  while (!started) {
    std::this_thread::yield();
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  const auto start = std::chrono::steady_clock::now();
  int disconnectResult = wrapper.disconnect();
  receiver.join();
  EXPECT_EQ(VSCP_ERROR_SUCCESS, disconnectResult);
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, receiveResult);
  EXPECT_LT(std::chrono::steady_clock::now() - start, std::chrono::seconds(2));
}

TEST_F(TcpTransport, WrapperSingleChannelBareAckClosesAndWakesReceiver)
{
  vscpClientTcp wrapper;
  wrapper.setResponseTimeout(150);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.init(setting("VSCP_TEST_BARE_ACK"), "admin", "secret"));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.setSingleChannelReceiveLoop());
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.connect());
  vscpEventEx event = {};
  ASSERT_EQ(VSCP_ERROR_SUCCESS, wrapper.receiveBlocking(event, 1000));

  std::atomic<bool> started{false};
  int receiveResult = VSCP_ERROR_SUCCESS;
  std::thread receiver([&]() {
    started = true;
    vscpEventEx next = {};
    receiveResult = wrapper.receiveBlocking(next, 2000);
  });
  while (!started) {
    std::this_thread::yield();
  }
  vscpEventEx outgoing = {};
  outgoing.vscp_class = 10;
  outgoing.vscp_type = 6;
  const auto start = std::chrono::steady_clock::now();
  int sendResult = wrapper.send(outgoing);
  receiver.join();
  EXPECT_EQ(VSCP_ERROR_TIMEOUT, sendResult);
  EXPECT_NE(VSCP_ERROR_SUCCESS, receiveResult);
  EXPECT_NE(VSCP_ERROR_TIMEOUT, receiveResult);
  EXPECT_LT(std::chrono::steady_clock::now() - start, std::chrono::seconds(2));
  EXPECT_FALSE(wrapper.isConnected());
  EXPECT_EQ(VSCP_ERROR_SUCCESS, wrapper.disconnect());
}

TEST_F(TcpTransport, ReceiveTimeoutIsDebugAndDoesNotExposeData)
{
  namespace fs = std::filesystem;
  const fs::path directory = fs::temp_directory_path() /
    ("vscp-tcp-logging-" + std::to_string(
      std::chrono::steady_clock::now().time_since_epoch().count()));
  ASSERT_TRUE(fs::create_directory(directory));
  struct Cleanup {
    fs::path directory;
    ~Cleanup()
    {
      EXPECT_EQ(VSCP_ERROR_SUCCESS, vscphelper_disableLogging());
      fs::remove_all(directory);
    }
  } cleanup{directory};
  const std::string path = (directory / "tcp.log").string();
  ASSERT_EQ(VSCP_ERROR_SUCCESS, vscphelper_configureLogging(
    VSCP_HELPER_LOG_LEVEL_OFF, VSCP_HELPER_LOG_LEVEL_DEBUG, path.c_str(), 16 * 1024 * 1024, 2));
  ASSERT_EQ(VSCP_ERROR_SUCCESS,
            client.doCmdOpen(setting("VSCP_TEST_PLAIN"), "private-tcp-user", "private-tcp-password"));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCommand("SCENARIO empty\r\n"));
  vscpEvent legacyEvent{};
  vscpEventEx event{};
  canalMsg canalEvent{};
  EXPECT_EQ(VSCP_ERROR_ERROR, client.doCmdReceive(&legacyEvent));
  EXPECT_EQ(VSCP_ERROR_ERROR, client.doCmdReceiveEx(&event));
  EXPECT_EQ(VSCP_ERROR_GENERIC, client.doCmdReceiveLevel1(&canalEvent));
  EXPECT_EQ(VSCP_ERROR_ERROR, client.doCommand(std::string("retr 1\r\n")));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdEnterReceiveLoop());
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdBlockingReceive(&event, 500));
  ASSERT_EQ(VSCP_ERROR_TIMEOUT, client.doCmdBlockingReceive(&event, 100));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, vscphelper_flushLogging());

  std::ifstream stream(path);
  const std::string output{std::istreambuf_iterator<char>(stream), {}};
  EXPECT_NE(std::string::npos,
            output.find("[debug] VSCP TCP: doCmdBlockingReceive returned " + std::to_string(VSCP_ERROR_TIMEOUT)));
  EXPECT_NE(std::string::npos, output.find("[debug] VSCP TCP: doCmdReceive: receive queue is empty"));
  EXPECT_NE(std::string::npos, output.find("[debug] VSCP TCP: doCmdReceiveEx: receive queue is empty"));
  EXPECT_NE(std::string::npos,
            output.find("[debug] VSCP TCP: doCmdReceiveLevel1: receive queue is empty or timed out"));
  EXPECT_EQ(std::string::npos, output.find("[error] VSCP TCP:"));
  std::istringstream lines(output);
  std::string line;
  while (std::getline(lines, line)) {
    if (line.find("error 32") != std::string::npos || line.find("error -32") != std::string::npos) {
      EXPECT_EQ(std::string::npos, line.find("[error]")) << line;
    }
  }
  EXPECT_EQ(std::string::npos, output.find("private-tcp-user"));
  EXPECT_EQ(std::string::npos, output.find("private-tcp-password"));
  EXPECT_EQ(std::string::npos, output.find("0,10,6,0,2026-10-07T00:00:00,0,-,1,2,3"));

  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdQuitReceiveLoop());
  EXPECT_EQ(VSCP_ERROR_ERROR, client.doCommand("OTHER\r\n"));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, vscphelper_flushLogging());
  std::ifstream otherStream(path);
  const std::string otherOutput{std::istreambuf_iterator<char>(otherStream), {}};
  EXPECT_NE(std::string::npos, otherOutput.find("[error] VSCP TCP: doCommand failed"));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCommand("SCENARIO reject-retr\r\n"));
  EXPECT_EQ(VSCP_ERROR_ERROR, client.doCmdReceiveEx(&event));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, vscphelper_flushLogging());
  std::ifstream rejectedStream(path);
  const std::string rejectedOutput{std::istreambuf_iterator<char>(rejectedStream), {}};
  EXPECT_NE(std::string::npos, rejectedOutput.find("[error] VSCP TCP: doCmdReceiveEx failed"));
}

TEST_F(TcpTransport, FragmentedErrorAndResponseTimeout)
{
  open("VSCP_TEST_PLAIN");
  auto start = std::chrono::steady_clock::now();
  ASSERT_EQ(VSCP_ERROR_ERROR, client.doCommand("ERROR\r\n"));
  ASSERT_LT(std::chrono::steady_clock::now() - start, std::chrono::milliseconds(300));
  start = std::chrono::steady_clock::now();
  ASSERT_EQ(VSCP_ERROR_ERROR, client.doCommand("SILENT\r\n"));
  auto elapsed = std::chrono::steady_clock::now() - start;
  ASSERT_GE(elapsed, std::chrono::milliseconds(390));
  ASSERT_LT(elapsed, std::chrono::milliseconds(700));
}

TEST_F(TcpTransport, PeerDisconnectAndClose)
{
  open("VSCP_TEST_PLAIN");
  ASSERT_EQ(VSCP_ERROR_ERROR, client.doCommand("DROP\r\n"));
  ASSERT_FALSE(client.isConnected());
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdClose());
  open("VSCP_TEST_PLAIN");
}

TEST_F(TcpTransport, PeerResetBeforeSend)
{
  for (const auto *endpoint : {"VSCP_TEST_PLAIN", "VSCP_TEST_TLS"}) {
    open(endpoint);
    ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCommand("RESET\r\n"));
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    ASSERT_EQ(VSCP_ERROR_ERROR, client.doCmdNOOP());
    ASSERT_FALSE(client.isConnected());
  }
}

TEST_F(TcpTransport, BinaryWriteTimeoutClosesPartiallySentConnection)
{
  const std::vector<uint8_t> frame(65535, 0xaa);
  for (const auto *endpoint : {"VSCP_TEST_PLAIN", "VSCP_TEST_TLS"}) {
    open(endpoint);
    ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCommand("SCENARIO hold\r\n"));
    ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdBinary());
    int result = VSCP_ERROR_SUCCESS;
    std::chrono::steady_clock::duration elapsed;
    for (size_t attempt = 0; attempt < 4096 && result == VSCP_ERROR_SUCCESS; ++attempt) {
      auto start = std::chrono::steady_clock::now();
      result = client.sendBinaryFrame(frame);
      elapsed = std::chrono::steady_clock::now() - start;
    }
    ASSERT_EQ(VSCP_ERROR_TIMEOUT, result);
    ASSERT_GE(elapsed, std::chrono::milliseconds(390));
    ASSERT_LT(elapsed, std::chrono::milliseconds(1500));
    ASSERT_FALSE(client.isConnected());
  }
}

TEST_F(TcpTransport, TlsHandshakeTimeout)
{
  auto start = std::chrono::steady_clock::now();
  ASSERT_EQ(VSCP_ERROR_TIMEOUT, client.doCmdOpen(setting("VSCP_TEST_STALL"), "admin", "secret"));
  auto elapsed = std::chrono::steady_clock::now() - start;
  ASSERT_GE(elapsed, std::chrono::milliseconds(990));
  ASSERT_LT(elapsed, std::chrono::milliseconds(1400));
  ASSERT_FALSE(client.isConnected());
}

TEST_F(TcpTransport, InvalidEndpoint)
{
  for (const auto *endpoint : {"tcp://localhost", "localhost:0", "localhost:65536",
                               "localhost:foo", "localhost:12/path", "::1:9598"}) {
    ASSERT_EQ(VSCP_ERROR_PARAMETER, client.doCmdOpen(endpoint, "admin", "secret"));
  }
}

TEST_F(TcpTransport, BinaryModeRejectsTextAndResetsOnReconnect)
{
  open("VSCP_TEST_PLAIN");
  std::vector<uint8_t> frame = {0xe0, 0, 0, 0, 0};
  ASSERT_EQ(VSCP_ERROR_PARAMETER, client.sendBinaryFrame(frame));
  ASSERT_EQ(VSCP_ERROR_PARAMETER, client.receiveBinaryFrame(frame, 0));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdBinary());
  ASSERT_TRUE(client.isBinaryMode());
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdBinary());
  ASSERT_EQ(VSCP_ERROR_PARAMETER, client.doCommand("NOOP\r\n"));
  ASSERT_EQ(VSCP_ERROR_PARAMETER, client.doCmdNOOP());
  ASSERT_EQ(VSCP_ERROR_PARAMETER, client.doCmdEnterReceiveLoop());
  ASSERT_EQ(VSCP_ERROR_PARAMETER, client.doCmdQuitReceiveLoop());
  ASSERT_FALSE(client.checkReturnValue());
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdClose());
  ASSERT_FALSE(client.isBinaryMode());
  open("VSCP_TEST_PLAIN");
}

TEST_F(TcpTransport, BinaryModeRejectedAndReceiveLoop)
{
  open("VSCP_TEST_PLAIN");
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCommand("SCENARIO reject\r\n"));
  ASSERT_EQ(VSCP_ERROR_ERROR, client.doCmdBinary());
  ASSERT_FALSE(client.isBinaryMode());
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdNOOP());
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdEnterReceiveLoop());
  ASSERT_EQ(VSCP_ERROR_PARAMETER, client.doCmdBinary());
}

TEST_F(TcpTransport, BinaryCommandUsesModeTransition)
{
  open("VSCP_TEST_PLAIN");
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCommand("binary\r\n"));
  ASSERT_TRUE(client.isBinaryMode());
  ASSERT_EQ(VSCP_ERROR_PARAMETER, client.doCommand("VERS\r\n"));
}

TEST_F(TcpTransport, BinaryNegotiationTimeoutClosesAmbiguousConnection)
{
  open("VSCP_TEST_PLAIN");
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCommand("SCENARIO silent\r\n"));
  ASSERT_EQ(VSCP_ERROR_TIMEOUT, client.doCmdBinary());
  ASSERT_FALSE(client.isConnected());
  ASSERT_FALSE(client.isBinaryMode());
}

TEST_F(TcpTransport, BinaryMalformedNegotiationClosesConnection)
{
  open("VSCP_TEST_PLAIN");
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCommand("SCENARIO malformed-response\r\n"));
  ASSERT_EQ(VSCP_ERROR_INVALID_FRAME, client.doCmdBinary());
  ASSERT_FALSE(client.isConnected());
  ASSERT_FALSE(client.isBinaryMode());
}

TEST_F(TcpTransport, BinaryRawFramesOverTls)
{
  open("VSCP_TEST_TLS");
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdBinary());
  for (const uint8_t type : {0xe0, 0xf0, 0x11, 0x12, 0x13}) {
    std::vector<uint8_t> sent = {type, 0, 1, '\r', '\n', 0, 0xff, 0, 3};
    std::vector<uint8_t> received;
    ASSERT_EQ(VSCP_ERROR_SUCCESS, client.sendBinaryFrame(sent));
    ASSERT_EQ(VSCP_ERROR_SUCCESS, client.receiveBinaryFrame(received, 500));
    ASSERT_EQ(sent, received);
  }
  ASSERT_EQ(VSCP_ERROR_PARAMETER, client.sendBinaryFrame({}));
  ASSERT_EQ(VSCP_ERROR_PARAMETER, client.sendBinaryFrame(std::vector<uint8_t>(65536)));
}

TEST_F(TcpTransport, BinaryCoalescedTransitionAndFrames)
{
  for (const auto *endpoint : {"VSCP_TEST_PLAIN", "VSCP_TEST_TLS"}) {
    open(endpoint);
    ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCommand("SCENARIO coalesced\r\n"));
    ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdBinary());
    std::vector<uint8_t> first, second;
    ASSERT_EQ(VSCP_ERROR_SUCCESS, client.receiveBinaryFrame(first, 500));
    ASSERT_EQ(VSCP_ERROR_SUCCESS, client.receiveBinaryFrame(second, 500));
    ASSERT_EQ(first, second);
    ASSERT_EQ(8u, first.size());
    ASSERT_EQ(0, crcFast(first.data() + 1, first.size() - 1));
    ASSERT_EQ(VSCP_ERROR_TIMEOUT, client.receiveBinaryFrame(second, 30));
    ASSERT_EQ(first, second);
  }
}

TEST_F(TcpTransport, BinaryPartialPrefixSurvivesTimeout)
{
  open("VSCP_TEST_PLAIN");
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCommand("SCENARIO partial\r\n"));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdBinary());
  std::vector<uint8_t> frame = {42};
  ASSERT_EQ(VSCP_ERROR_TIMEOUT, client.receiveBinaryFrame(frame, 30));
  ASSERT_EQ(std::vector<uint8_t>({42}), frame);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.receiveBinaryFrame(frame, 500));
  ASSERT_EQ(8u, frame.size());
  ASSERT_EQ(0xe0, frame[0]);
}

TEST_F(TcpTransport, BinaryPartialBodySurvivesTimeout)
{
  open("VSCP_TEST_PLAIN");
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCommand("SCENARIO partial-body\r\n"));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdBinary());
  std::vector<uint8_t> frame = {42};
  ASSERT_EQ(VSCP_ERROR_TIMEOUT, client.receiveBinaryFrame(frame, 30));
  ASSERT_EQ(std::vector<uint8_t>({42}), frame);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.receiveBinaryFrame(frame, 500));
  ASSERT_EQ(8u, frame.size());
  ASSERT_EQ(0, crcFast(frame.data() + 1, frame.size() - 1));
}

TEST_F(TcpTransport, BinaryRejectsZeroLength)
{
  open("VSCP_TEST_PLAIN");
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCommand("SCENARIO zero\r\n"));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdBinary());
  std::vector<uint8_t> frame;
  ASSERT_EQ(VSCP_ERROR_INVALID_FRAME, client.receiveBinaryFrame(frame, 500));
  ASSERT_FALSE(client.isConnected());
}

TEST_F(TcpTransport, BinaryTruncatedFrameDisconnect)
{
  open("VSCP_TEST_PLAIN");
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCommand("SCENARIO truncated\r\n"));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdBinary());
  std::vector<uint8_t> frame;
  ASSERT_EQ(VSCP_ERROR_STOPPED, client.receiveBinaryFrame(frame, 500));
  ASSERT_FALSE(client.isConnected());
}

TEST_F(TcpTransport, BinaryEventRoundtripAndConfirmation)
{
  open("VSCP_TEST_PLAIN");
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdBinary());
  vscpEventEx sent = {};
  sent.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS;
  sent.timestamp_ns = 1792051200123456789ULL;
  sent.vscp_class = 10;
  sent.vscp_type = 6;
  sent.GUID[15] = 27;
  sent.sizeData = 4;
  sent.data[0] = 0;
  sent.data[1] = '\r';
  sent.data[2] = '\n';
  sent.data[3] = 0xff;
  for (int i = 0; i < 2; ++i) {
    ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdSendEx(&sent));
    vscpEventEx received = {};
    ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdBlockingReceive(&received, 500));
    ASSERT_EQ(sent.head, received.head);
    ASSERT_EQ(sent.timestamp_ns, received.timestamp_ns);
    ASSERT_EQ(sent.vscp_class, received.vscp_class);
    ASSERT_EQ(sent.vscp_type, received.vscp_type);
    ASSERT_EQ(sent.GUID[15], received.GUID[15]);
    ASSERT_EQ(sent.sizeData, received.sizeData);
    ASSERT_EQ(0, memcmp(sent.data, received.data, sent.sizeData));
  }
}

TEST_F(TcpTransport, BinaryEventRejectsInvalidCrc)
{
  open("VSCP_TEST_PLAIN");
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCommand("SCENARIO bad-event\r\n"));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdBinary());
  vscpEventEx event = {};
  ASSERT_EQ(VSCP_ERROR_INVALID_FRAME, client.doCmdReceiveEx(&event));
}

TEST_F(TcpTransport, BinaryEventRejectsInvalidLength)
{
  for (const auto *command : {"SCENARIO short-event\r\n", "SCENARIO oversized-event\r\n"}) {
    open("VSCP_TEST_PLAIN");
    ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCommand(command));
    ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdBinary());
    vscpEvent event = {};
    ASSERT_EQ(VSCP_ERROR_INVALID_FRAME, client.doCmdReceive(&event));
    ASSERT_EQ(nullptr, event.pdata);
  }
}

TEST_F(TcpTransport, BinaryLegacyEvent)
{
  open("VSCP_TEST_PLAIN");
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCommand("SCENARIO legacy-event\r\n"));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdBinary());
  vscpEvent received = {};
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdReceive(&received));
  EXPECT_EQ(2026, received.year);
  EXPECT_EQ(10, received.month);
  EXPECT_EQ(15, received.day);
  EXPECT_EQ(123456u, received.timestamp);
  EXPECT_EQ(10, received.vscp_class);
  EXPECT_EQ(6, received.vscp_type);
  EXPECT_EQ(27, received.GUID[15]);
  ASSERT_EQ(2, received.sizeData);
  EXPECT_EQ(0, received.pdata[0]);
  EXPECT_EQ(0xff, received.pdata[1]);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdSend(&received));
  delete[] received.pdata;
  received = {};
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdBlockingReceive(&received, 500));
  EXPECT_EQ(VSCP_HEADER16_FRAME_VERSION_UNIX_NS, received.head & VSCP_HEADER16_FRAME_VERSION_MASK);
  EXPECT_EQ(2, received.sizeData);
  delete[] received.pdata;
}

TEST_F(TcpTransport, BinaryEventConfirmationError)
{
  open("VSCP_TEST_PLAIN");
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCommand("SCENARIO error-confirmation\r\n"));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdBinary());
  vscpEventEx event = {};
  ASSERT_EQ(VSCP_ERROR_PARAMETER, client.doCmdReceiveEx(&event));
}

TEST_F(TcpTransport, BinaryCommandAndReplyCrc)
{
  open("VSCP_TEST_PLAIN");
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdBinary());
  const uint8_t argument[] = {0, '\r', '\n', 0xff};
  for (bool reply : {false, true}) {
    std::vector<uint8_t> sent(reply ? 11 : 9, 0xab);
    int result = reply ? vscp_writeReplyToFrame(sent.data(), sent.size(), 7, 0, argument, sizeof(argument)) :
                         vscp_writeCommandToFrame(sent.data(), sent.size(), 7, argument, sizeof(argument));
    ASSERT_EQ(VSCP_ERROR_SUCCESS, result);
    ASSERT_EQ(0, crcFast(sent.data() + 1, sent.size() - 1));
    ASSERT_EQ(VSCP_ERROR_SUCCESS, client.sendBinaryFrame(sent));
    std::vector<uint8_t> received;
    ASSERT_EQ(VSCP_ERROR_SUCCESS, client.receiveBinaryFrame(received, 500));
    ASSERT_EQ(sent, received);
  }
}

TEST_F(TcpTransport, BinaryEncryptedFrames)
{
  open("VSCP_TEST_TLS");
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdBinary());
  const uint8_t key[32] = {};
  const uint8_t iv[16] = {};
  std::vector<uint8_t> command(5);
  ASSERT_EQ(VSCP_ERROR_SUCCESS,
            vscp_writeCommandToFrame(command.data(), command.size(), 7, nullptr, 0));
  for (uint8_t algorithm : {VSCP_ENCRYPTION_AES128, VSCP_ENCRYPTION_AES192, VSCP_ENCRYPTION_AES256}) {
    command[0] = VSCP_BINARY_PACKET_TYPE_COMMAND | algorithm;
    std::vector<uint8_t> encrypted(command.size() + 32);
    size_t size = vscp_encryptFrame(encrypted.data(), command.data(), command.size(), key, iv, algorithm);
    ASSERT_GT(size, 0u);
    encrypted.resize(size);
    ASSERT_EQ(VSCP_ERROR_SUCCESS, client.sendBinaryFrame(encrypted));
    std::vector<uint8_t> received;
    ASSERT_EQ(VSCP_ERROR_SUCCESS, client.receiveBinaryFrame(received, 500));
    ASSERT_EQ(encrypted, received);
    std::vector<uint8_t> decrypted(received.size());
    ASSERT_TRUE(vscp_decryptFrame(decrypted.data(), received.data(), received.size(), key, nullptr,
                                  VSCP_ENCRYPTION_FROM_TYPE_BYTE));
    ASSERT_TRUE(std::equal(command.begin(), command.end(), decrypted.begin()));
  }
}

TEST_F(TcpTransport, BinaryMaximumFrameLength)
{
  open("VSCP_TEST_PLAIN");
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.doCmdBinary());
  std::vector<uint8_t> sent(65535, 0xaa);
  sent[0] = 0xe0;
  std::vector<uint8_t> received;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.sendBinaryFrame(sent));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.receiveBinaryFrame(received, 1000));
  ASSERT_EQ(sent, received);
}
