#include <gtest/gtest.h>
#include <vscpremotetcpif.h>
#include <vscp-client-tcp.h>

#include <chrono>
#include <cstdlib>
#include <string>

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
