/////////////////////////////////////////////////////////////////////////////////
// unittest.cpp
//
// Unit tests for TCP client configuration.
//

#include <gtest/gtest.h>

#include <nlohmann/json.hpp>
#include <crc.h>
#include <vscp-client-tcp.h>
#include <vscpremotetcpif.h>

using json = nlohmann::json;

TEST(VscpClientTcpBinary, CrcTableMatchesSlowImplementation)
{
  crcInit();
  unsigned char data[256] = {};
  for (size_t i = 0; i < sizeof(data); ++i) {
    data[i] = static_cast<unsigned char>(i);
    EXPECT_EQ(crcSlow(&data[i], 1), crcFast(&data[i], 1)) << i;
  }
  EXPECT_EQ(crcSlow(data, sizeof(data)), crcFast(data, sizeof(data)));
  const unsigned char check[] = "123456789";
  EXPECT_EQ(CHECK_VALUE, crcFast(check, sizeof(check) - 1));
}

// ============================================================================
//                      VscpRemoteTcpIf selectable TLS
// ============================================================================

TEST(VscpRemoteTcpIfTlsSelect, DefaultModeIsAuto)
{
  VscpRemoteTcpIf tcpif;

  EXPECT_EQ(VscpRemoteTcpIf::tls_mode::auto_select, tcpif.getTLSMode());
  EXPECT_FALSE(tcpif.m_bTLS);
}

TEST(VscpRemoteTcpIfTlsSelect, AutoModeViaInterfaceString)
{
  // stcp://host:port;user;pass  interface-string format should pick TLS
  VscpRemoteTcpIf tcpif;
  tcpif.setConnectTimeout(1);

  EXPECT_EQ(VscpRemoteTcpIf::tls_mode::auto_select, tcpif.getTLSMode());

  // stcp:// interface string => TLS
  EXPECT_NE(VSCP_ERROR_PARAMETER, tcpif.doCmdOpen("stcp://127.0.0.1:39598;admin;secret"));
  EXPECT_TRUE(tcpif.m_bTLS);

  // tcp:// interface string => plain
  EXPECT_NE(VSCP_ERROR_PARAMETER, tcpif.doCmdOpen("tcp://127.0.0.1:39598;admin;secret"));
  EXPECT_FALSE(tcpif.m_bTLS);
}

TEST(VscpRemoteTcpIfTlsSelect, AutoModeSwitchesWithPrefix)
{
  VscpRemoteTcpIf tcpif;
  tcpif.setConnectTimeout(1);

  EXPECT_NE(VSCP_ERROR_PARAMETER, tcpif.doCmdOpen("stcp://127.0.0.1:39598", "admin", "secret"));
  EXPECT_TRUE(tcpif.m_bTLS);

  EXPECT_NE(VSCP_ERROR_PARAMETER, tcpif.doCmdOpen("tcp://127.0.0.1:39598", "admin", "secret"));
  EXPECT_FALSE(tcpif.m_bTLS);
}

TEST(VscpRemoteTcpIfTlsSelect, ForcePlainIgnoresStcpPrefix)
{
  VscpRemoteTcpIf tcpif;
  tcpif.setConnectTimeout(1);
  tcpif.enableTLS(false);

  EXPECT_EQ(VscpRemoteTcpIf::tls_mode::force_plain, tcpif.getTLSMode());
  EXPECT_NE(VSCP_ERROR_PARAMETER, tcpif.doCmdOpen("stcp://127.0.0.1:39598", "admin", "secret"));
  EXPECT_FALSE(tcpif.m_bTLS);
}

TEST(VscpRemoteTcpIfTlsSelect, ForceTlsUsesTcpPrefix)
{
  VscpRemoteTcpIf tcpif;
  tcpif.setConnectTimeout(1);
  tcpif.enableTLS(true);

  EXPECT_EQ(VscpRemoteTcpIf::tls_mode::force_tls, tcpif.getTLSMode());
  EXPECT_NE(VSCP_ERROR_PARAMETER, tcpif.doCmdOpen("tcp://127.0.0.1:39598", "admin", "secret"));
  EXPECT_TRUE(tcpif.m_bTLS);
}

// ============================================================================
//                        vscpClientTcp selectable TLS
// ============================================================================

TEST(VscpClientTcpTlsSelect, DefaultModeIsAuto)
{
  vscpClientTcp client;
  json j = json::parse(client.getConfigAsJson());

  EXPECT_EQ(vscpClientTcp::tls_mode::auto_select, client.getTLSMode());
  EXPECT_TRUE(j.contains("tls-mode"));
  EXPECT_EQ("auto", j["tls-mode"].get<std::string>());
  EXPECT_FALSE(j["btls"].get<bool>());
}

TEST(VscpClientTcpTlsSelect, LegacyBtlsTrueForcesTlsMode)
{
  vscpClientTcp client;
  json j;
  j["host"] = "tcp://127.0.0.1:9598";
  j["user"] = "admin";
  j["password"] = "secret";
  j["btls"] = true;

  ASSERT_TRUE(client.initFromJson(j.dump()));
  EXPECT_EQ(vscpClientTcp::tls_mode::force_tls, client.getTLSMode());

  json j2 = json::parse(client.getConfigAsJson());
  EXPECT_EQ("force-tls", j2["tls-mode"].get<std::string>());
  EXPECT_TRUE(j2["btls"].get<bool>());
}

TEST(VscpClientTcpTlsSelect, ForcePlainRoundtrip)
{
  vscpClientTcp client;
  client.setTLSMode(vscpClientTcp::tls_mode::force_plain);

  json j = json::parse(client.getConfigAsJson());
  EXPECT_EQ("force-plain", j["tls-mode"].get<std::string>());
  EXPECT_FALSE(j["btls"].get<bool>());
}

TEST(VscpClientTcpTlsSelect, InitFromJsonTlsModeParsing)
{
  vscpClientTcp client;
  json j;
  j["host"] = "stcp://localhost:9598";
  j["user"] = "admin";
  j["password"] = "secret";
  j["tls-mode"] = "force-plain";
  j["btls"] = true; // tls-mode should take precedence.

  ASSERT_TRUE(client.initFromJson(j.dump()));
  EXPECT_EQ(vscpClientTcp::tls_mode::force_plain, client.getTLSMode());

  json j2 = json::parse(client.getConfigAsJson());
  EXPECT_EQ("force-plain", j2["tls-mode"].get<std::string>());
  EXPECT_FALSE(j2["btls"].get<bool>());
}

TEST(VscpClientTcpSingleChannel, DefaultAndJsonRoundTrip)
{
  vscpClientTcp client;
  EXPECT_FALSE(client.isSingleChannelReceiveLoopEnabled());
  json initial = json::parse(client.getConfigAsJson());
  EXPECT_FALSE(initial["single-channel"].get<bool>());
  EXPECT_FALSE(initial["bpoll"].get<bool>());

  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.setSingleChannelReceiveLoop());
  EXPECT_TRUE(client.isSingleChannelReceiveLoopEnabled());
  json configured = json::parse(client.getConfigAsJson());
  EXPECT_TRUE(configured["single-channel"].get<bool>());

  vscpClientTcp restored;
  ASSERT_TRUE(restored.initFromJson(configured.dump()));
  EXPECT_TRUE(restored.isSingleChannelReceiveLoopEnabled());
  EXPECT_EQ(configured, json::parse(restored.getConfigAsJson()));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, restored.setSingleChannelReceiveLoop(false));
  EXPECT_FALSE(restored.isSingleChannelReceiveLoopEnabled());
}

TEST(VscpClientTcpSingleChannel, RejectsConflictingPollingConfiguration)
{
  vscpClientTcp client;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.init("tcp://localhost:9598", "admin", "secret", true));
  EXPECT_EQ(VSCP_ERROR_PARAMETER, client.setSingleChannelReceiveLoop());
  EXPECT_FALSE(client.isSingleChannelReceiveLoopEnabled());

  ASSERT_TRUE(client.initFromJson(R"({"bpoll":false,"single-channel":true})"));
  EXPECT_TRUE(client.isSingleChannelReceiveLoopEnabled());
  EXPECT_EQ(VSCP_ERROR_PARAMETER, client.init("tcp://localhost:9598", "admin", "secret", true));
  EXPECT_FALSE(client.initFromJson(R"({"bpoll":true,"single-channel":true})"));
  EXPECT_FALSE(client.initFromJson(R"({"single-channel":"true"})"));
  EXPECT_TRUE(client.isSingleChannelReceiveLoopEnabled());
  EXPECT_FALSE(json::parse(client.getConfigAsJson())["bpoll"].get<bool>());

  vscpClientTcp invalid;
  EXPECT_FALSE(invalid.initFromJson(R"({"single-channel":true,"bfull-l2":"invalid"})"));
  EXPECT_FALSE(invalid.isSingleChannelReceiveLoopEnabled());
}

TEST(VscpClientTcpBinary, DefaultsAndJsonRoundTrip)
{
  vscpClientTcp client;
  EXPECT_FALSE(client.isBinaryModeEnabled());
  EXPECT_FALSE(json::parse(client.getConfigAsJson())["binary-mode"].get<bool>());

  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.setBinaryMode());
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.setSingleChannelReceiveLoop());
  json configured = json::parse(client.getConfigAsJson());
  EXPECT_TRUE(configured["binary-mode"].get<bool>());
  EXPECT_TRUE(configured["single-channel"].get<bool>());

  vscpClientTcp restored;
  ASSERT_TRUE(restored.initFromJson(configured.dump()));
  EXPECT_TRUE(restored.isBinaryModeEnabled());
  EXPECT_TRUE(restored.isSingleChannelReceiveLoopEnabled());
  EXPECT_EQ(configured, json::parse(restored.getConfigAsJson()));
  EXPECT_EQ(VSCP_ERROR_SUCCESS, restored.setBinaryMode(false));
  EXPECT_FALSE(restored.isBinaryModeEnabled());
}

TEST(VscpClientTcpBinary, PollingAndInvalidConfiguration)
{
  vscpClientTcp client;
  ASSERT_TRUE(client.initFromJson(R"({"bpoll":true,"binary-mode":true})"));
  EXPECT_TRUE(client.isBinaryModeEnabled());
  EXPECT_EQ(VSCP_ERROR_PARAMETER, client.setSingleChannelReceiveLoop());
  EXPECT_FALSE(client.initFromJson(R"({"bpoll":true,"single-channel":true,"binary-mode":true})"));
  EXPECT_FALSE(client.initFromJson(R"({"binary-mode":"true"})"));
  EXPECT_TRUE(client.isBinaryModeEnabled());
  EXPECT_TRUE(json::parse(client.getConfigAsJson())["bpoll"].get<bool>());

  std::vector<uint8_t> frame;
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, client.sendBinaryFrame({0xe0, 0, 1, 0, 0}));
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, client.receiveBinaryFrame(frame));
  EXPECT_EQ(VSCP_ERROR_PARAMETER, client.receiveBinaryFrame(frame, -1));
  EXPECT_EQ(VSCP_ERROR_SUCCESS, client.setBinaryMode(false));
  EXPECT_EQ(VSCP_ERROR_NOT_SUPPORTED, client.sendBinaryFrame({1}));
  EXPECT_EQ(VSCP_ERROR_NOT_SUPPORTED, client.receiveBinaryFrame(frame));
}
