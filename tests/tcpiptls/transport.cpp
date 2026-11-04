#include <gtest/gtest.h>
#include <vscpremotetcpif.h>
#include <vscp-client-tcp.h>
#include <vscphelperlogging.h>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
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
