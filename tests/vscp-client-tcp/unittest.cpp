/////////////////////////////////////////////////////////////////////////////////
// unittest.cpp
//
// Unit tests for selectable TLS behavior in VscpRemoteTcpIf and vscpClientTcp.
//

#include <gtest/gtest.h>

#include <nlohmann/json.hpp>
#include <vscp-client-tcp.h>
#include <vscpremotetcpif.h>

using json = nlohmann::json;

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
