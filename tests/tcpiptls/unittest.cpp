///////////////////////////////////////////////////////////////////////////////
// unittest.cpp
//
// Unit tests for TLS support in VscpRemoteTcpIf and vscpClientTcp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2026 Ake Hedman and contributors, the VSCP project
// <info@vscp.org>
//

#ifdef WIN32
#include <pch.h>
#endif

#include <string.h>

#include <gtest/gtest.h>

#include <vscpremotetcpif.h>
#include <vscphelper.h>
#include <vscp-client-tcp.h>
#include <sockettcp.h>

// Define TEST_INTEGRATION to run tests that require a live TLS-enabled
// VSCP daemon. Configure connection details below.
//#define TEST_INTEGRATION

#ifdef TEST_INTEGRATION
#define TLS_HOST      "stcp://localhost:9598"
#define TLS_USER      "admin"
#define TLS_PASSWORD  "secret"
#define TLS_CAFILE    "/path/to/ca.pem"
#define TLS_CERTFILE  ""
#define TLS_KEYFILE   ""
#endif

// ============================================================================
//                     VscpRemoteTcpIf TLS configuration
// ============================================================================

//-----------------------------------------------------------------------------
TEST(VscpRemoteTcpIfTLS, DefaultTLSDisabled)
{
  VscpRemoteTcpIf vscpif;

  // TLS should be disabled by default
  ASSERT_FALSE(vscpif.m_bTLS);
  ASSERT_FALSE(vscpif.m_bVerifyPeer);
  ASSERT_TRUE(vscpif.m_cafile.empty());
  ASSERT_TRUE(vscpif.m_capath.empty());
  ASSERT_TRUE(vscpif.m_certfile.empty());
  ASSERT_TRUE(vscpif.m_keyfile.empty());
  ASSERT_TRUE(vscpif.m_pwKeyfile.empty());
}

//-----------------------------------------------------------------------------
TEST(VscpRemoteTcpIfTLS, EnableTLS)
{
  VscpRemoteTcpIf vscpif;

  vscpif.enableTLS(true);
  ASSERT_TRUE(vscpif.m_bTLS);

  vscpif.enableTLS(false);
  ASSERT_FALSE(vscpif.m_bTLS);
}

//-----------------------------------------------------------------------------
TEST(VscpRemoteTcpIfTLS, SetTLSOptions)
{
  VscpRemoteTcpIf vscpif;

  vscpif.setTLSOptions(true,
                       "/path/to/ca.pem",
                       "/path/to/ca/",
                       "/path/to/cert.pem",
                       "/path/to/key.pem",
                       "keypassword");

  ASSERT_TRUE(vscpif.m_bTLS);
  ASSERT_TRUE(vscpif.m_bVerifyPeer);
  ASSERT_EQ("/path/to/ca.pem", vscpif.m_cafile);
  ASSERT_EQ("/path/to/ca/", vscpif.m_capath);
  ASSERT_EQ("/path/to/cert.pem", vscpif.m_certfile);
  ASSERT_EQ("/path/to/key.pem", vscpif.m_keyfile);
  ASSERT_EQ("keypassword", vscpif.m_pwKeyfile);
}

//-----------------------------------------------------------------------------
TEST(VscpRemoteTcpIfTLS, SetTLSOptionsDefaults)
{
  VscpRemoteTcpIf vscpif;

  // Call with only verify_peer, rest should default to empty
  vscpif.setTLSOptions(false);

  ASSERT_TRUE(vscpif.m_bTLS);
  ASSERT_FALSE(vscpif.m_bVerifyPeer);
  ASSERT_TRUE(vscpif.m_cafile.empty());
  ASSERT_TRUE(vscpif.m_capath.empty());
  ASSERT_TRUE(vscpif.m_certfile.empty());
  ASSERT_TRUE(vscpif.m_keyfile.empty());
  ASSERT_TRUE(vscpif.m_pwKeyfile.empty());
}

//-----------------------------------------------------------------------------
// Tests that parse stcp:// prefix are guarded because they attempt
// network connections which may block depending on the environment.
//-----------------------------------------------------------------------------

#ifdef TEST_INTEGRATION

TEST(VscpRemoteTcpIfTLS, StcpPrefixEnablesTLS)
{
  VscpRemoteTcpIf vscpif;

  ASSERT_FALSE(vscpif.m_bTLS);

  vscpif.setConnectTimeout(2);

  // stcp://host:port should be recognized as TLS
  // It will fail to connect, but should not return VSCP_ERROR_PARAMETER
  int rv = vscpif.doCmdOpen("stcp://127.0.0.1:39598", "admin", "secret");
  ASSERT_NE(VSCP_ERROR_PARAMETER, rv);
}

//-----------------------------------------------------------------------------
TEST(VscpRemoteTcpIfTLS, StcpPrefixInInterfaceString)
{
  VscpRemoteTcpIf vscpif;

  vscpif.setConnectTimeout(2);

  // stcp://host:port;user;password format
  int rv = vscpif.doCmdOpen("stcp://127.0.0.1:39598;admin;secret");
  ASSERT_NE(VSCP_ERROR_PARAMETER, rv);
}

//-----------------------------------------------------------------------------
TEST(VscpRemoteTcpIfTLS, TcpPrefixNoTLS)
{
  VscpRemoteTcpIf vscpif;

  // tcp:// should not set TLS
  ASSERT_FALSE(vscpif.m_bTLS);

  vscpif.setConnectTimeout(2);

  // Will fail to connect but should parse prefix correctly
  int rv = vscpif.doCmdOpen("tcp://127.0.0.1:39598", "admin", "secret");
  ASSERT_FALSE(vscpif.m_bTLS);
  ASSERT_NE(VSCP_ERROR_PARAMETER, rv);
}

//-----------------------------------------------------------------------------
TEST(VscpRemoteTcpIfTLS, EnableTLSThenConnect)
{
  VscpRemoteTcpIf vscpif;

  // Programmatic TLS enable (without stcp:// prefix)
  vscpif.enableTLS(true);
  vscpif.setTLSOptions(false, "/nonexistent/ca.pem");

  ASSERT_TRUE(vscpif.m_bTLS);

  vscpif.setConnectTimeout(2);

  // Will fail to connect (no server), but exercises the TLS code path
  int rv = vscpif.doCmdOpen("tcp://127.0.0.1:39598", "admin", "secret");
  ASSERT_TRUE(vscpif.m_bTLS);
  ASSERT_NE(VSCP_ERROR_PARAMETER, rv);
}

#endif // TEST_INTEGRATION (connection tests)

// ============================================================================
//                     vscpClientTcp TLS configuration
// ============================================================================

//-----------------------------------------------------------------------------
TEST(VscpClientTcpTLS, DefaultTLSDisabled)
{
  vscpClientTcp client;

  // Get config and verify TLS is disabled by default
  std::string config = client.getConfigAsJson();
  ASSERT_FALSE(config.empty());

  // Parse and check TLS fields
  nlohmann::json j = nlohmann::json::parse(config);
  ASSERT_TRUE(j.contains("btls"));
  ASSERT_FALSE(j["btls"].get<bool>());
}

//-----------------------------------------------------------------------------
TEST(VscpClientTcpTLS, SetTLSViaSetters)
{
  vscpClientTcp client;

  client.setMqttCaFile("/path/to/ca.pem");
  client.setMqttCaPath("/path/to/ca/");
  client.setMqttCertFile("/path/to/cert.pem");
  client.setMqttKeyFile("/path/to/key.pem");
  client.setMqttPwKeyFile("/path/to/pwkey.pem");

  std::string config = client.getConfigAsJson();
  nlohmann::json j = nlohmann::json::parse(config);

  ASSERT_TRUE(j["btls"].get<bool>());
  ASSERT_EQ("/path/to/ca.pem", j["cafile"].get<std::string>());
  ASSERT_EQ("/path/to/ca/", j["capath"].get<std::string>());
  ASSERT_EQ("/path/to/cert.pem", j["certfile"].get<std::string>());
  ASSERT_EQ("/path/to/key.pem", j["keyfile"].get<std::string>());
  ASSERT_EQ("/path/to/pwkey.pem", j["pwkeyfile"].get<std::string>());
}

//-----------------------------------------------------------------------------
TEST(VscpClientTcpTLS, InitFromJsonTLS)
{
  vscpClientTcp client;

  nlohmann::json j;
  j["host"]       = "stcp://localhost:9598";
  j["user"]       = "admin";
  j["password"]   = "secret";
  j["btls"]       = true;
  j["bverifypeer"] = true;
  j["cafile"]     = "/path/to/ca.pem";
  j["capath"]     = "/path/to/cadir";
  j["certfile"]   = "/path/to/cert.pem";
  j["keyfile"]    = "/path/to/key.pem";
  j["pwkeyfile"]  = "mypassword";

  ASSERT_TRUE(client.initFromJson(j.dump()));

  // Verify round-trip: get config back and check
  std::string config = client.getConfigAsJson();
  nlohmann::json j2 = nlohmann::json::parse(config);

  ASSERT_EQ("stcp://localhost:9598", j2["host"].get<std::string>());
  ASSERT_TRUE(j2["btls"].get<bool>());
  ASSERT_TRUE(j2["bverifypeer"].get<bool>());
  ASSERT_EQ("/path/to/ca.pem", j2["cafile"].get<std::string>());
  ASSERT_EQ("/path/to/cadir", j2["capath"].get<std::string>());
  ASSERT_EQ("/path/to/cert.pem", j2["certfile"].get<std::string>());
  ASSERT_EQ("/path/to/key.pem", j2["keyfile"].get<std::string>());
  ASSERT_EQ("mypassword", j2["pwkeyfile"].get<std::string>());
}

//-----------------------------------------------------------------------------
TEST(VscpClientTcpTLS, InitFromJsonNoTLS)
{
  vscpClientTcp client;

  nlohmann::json j;
  j["host"]     = "tcp://localhost:9598";
  j["user"]     = "admin";
  j["password"] = "secret";
  j["btls"]     = false;

  ASSERT_TRUE(client.initFromJson(j.dump()));

  std::string config = client.getConfigAsJson();
  nlohmann::json j2 = nlohmann::json::parse(config);

  ASSERT_FALSE(j2["btls"].get<bool>());
}

//-----------------------------------------------------------------------------
TEST(VscpClientTcpTLS, JsonRoundTripPreservesAllTLSFields)
{
  vscpClientTcp client1;

  // Set all TLS fields
  client1.setMqttCaFile("/etc/ssl/ca.pem");
  client1.setMqttCertFile("/etc/ssl/client.pem");
  client1.setMqttKeyFile("/etc/ssl/client-key.pem");
  client1.setMqttPwKeyFile("s3cret");

  // Serialize
  std::string json1 = client1.getConfigAsJson();

  // Create new client and deserialize
  vscpClientTcp client2;
  ASSERT_TRUE(client2.initFromJson(json1));

  // Serialize again
  std::string json2 = client2.getConfigAsJson();

  // Both JSON outputs should be identical
  ASSERT_EQ(json1, json2);
}

// ============================================================================
//     Server-side: mixed secure/unsecure listening port configurations
// ============================================================================

//-----------------------------------------------------------------------------
// Helper to create a server_context, call stcp_listening, and clean up
//-----------------------------------------------------------------------------

class ServerListeningTest : public ::testing::Test {
protected:
  struct server_context srvctx;

  void SetUp() override {
    memset(&srvctx, 0, sizeof(srvctx));
  }

  void TearDown() override {
    stcp_close_all_listening_sockets(&srvctx);
  }
};

//-----------------------------------------------------------------------------
TEST_F(ServerListeningTest, SingleUnsecurePort)
{
  // A plain port (no 's' suffix) should create one non-SSL socket
  int rv = stcp_listening(&srvctx, "19598");
  ASSERT_EQ(1, rv);
  ASSERT_EQ(1u, srvctx.num_listening_sockets);
  EXPECT_FALSE(srvctx.listening_sockets[0].is_ssl);
}

//-----------------------------------------------------------------------------
TEST_F(ServerListeningTest, SingleSecurePort)
{
  // Need an SSL context for a secure port
  SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
  ASSERT_NE(nullptr, ctx);
  srvctx.ssl_ctx = ctx;

  int rv = stcp_listening(&srvctx, "19599s");
  ASSERT_EQ(1, rv);
  ASSERT_EQ(1u, srvctx.num_listening_sockets);
  EXPECT_TRUE(srvctx.listening_sockets[0].is_ssl);

  SSL_CTX_free(ctx);
  srvctx.ssl_ctx = nullptr;
}

//-----------------------------------------------------------------------------
TEST_F(ServerListeningTest, MixedUnsecureAndSecure)
{
  // Comma-separated: first unsecure, second secure
  SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
  ASSERT_NE(nullptr, ctx);
  srvctx.ssl_ctx = ctx;

  int rv = stcp_listening(&srvctx, "19600,19601s");
  ASSERT_EQ(2, rv);
  ASSERT_EQ(2u, srvctx.num_listening_sockets);

  // First socket: plain
  EXPECT_FALSE(srvctx.listening_sockets[0].is_ssl);
  // Second socket: SSL
  EXPECT_TRUE(srvctx.listening_sockets[1].is_ssl);

  SSL_CTX_free(ctx);
  srvctx.ssl_ctx = nullptr;
}

//-----------------------------------------------------------------------------
TEST_F(ServerListeningTest, MixedSecureAndUnsecure)
{
  // Reversed order: secure first, unsecure second
  SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
  ASSERT_NE(nullptr, ctx);
  srvctx.ssl_ctx = ctx;

  int rv = stcp_listening(&srvctx, "19602s,19603");
  ASSERT_EQ(2, rv);
  ASSERT_EQ(2u, srvctx.num_listening_sockets);

  // First socket: SSL
  EXPECT_TRUE(srvctx.listening_sockets[0].is_ssl);
  // Second socket: plain
  EXPECT_FALSE(srvctx.listening_sockets[1].is_ssl);

  SSL_CTX_free(ctx);
  srvctx.ssl_ctx = nullptr;
}

//-----------------------------------------------------------------------------
TEST_F(ServerListeningTest, MultipleUnsecurePorts)
{
  int rv = stcp_listening(&srvctx, "19604,19605,19606");
  ASSERT_EQ(3, rv);
  ASSERT_EQ(3u, srvctx.num_listening_sockets);

  for (unsigned int i = 0; i < srvctx.num_listening_sockets; i++) {
    EXPECT_FALSE(srvctx.listening_sockets[i].is_ssl) << "socket " << i;
  }
}

//-----------------------------------------------------------------------------
TEST_F(ServerListeningTest, MultipleSecurePorts)
{
  SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
  ASSERT_NE(nullptr, ctx);
  srvctx.ssl_ctx = ctx;

  int rv = stcp_listening(&srvctx, "19607s,19608s");
  ASSERT_EQ(2, rv);
  ASSERT_EQ(2u, srvctx.num_listening_sockets);

  for (unsigned int i = 0; i < srvctx.num_listening_sockets; i++) {
    EXPECT_TRUE(srvctx.listening_sockets[i].is_ssl) << "socket " << i;
  }

  SSL_CTX_free(ctx);
  srvctx.ssl_ctx = nullptr;
}

//-----------------------------------------------------------------------------
TEST_F(ServerListeningTest, SecurePortWithoutSSLContextFails)
{
  // No ssl_ctx set — secure port should fail
  int rv = stcp_listening(&srvctx, "19609s");
  ASSERT_EQ(0, rv);
}

//-----------------------------------------------------------------------------
TEST_F(ServerListeningTest, MixedWithBoundAddress)
{
  // Bind to specific address with mixed secure/unsecure
  SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
  ASSERT_NE(nullptr, ctx);
  srvctx.ssl_ctx = ctx;

  int rv = stcp_listening(&srvctx, "127.0.0.1:19610,127.0.0.1:19611s");
  ASSERT_EQ(2, rv);
  ASSERT_EQ(2u, srvctx.num_listening_sockets);

  EXPECT_FALSE(srvctx.listening_sockets[0].is_ssl);
  EXPECT_TRUE(srvctx.listening_sockets[1].is_ssl);

  SSL_CTX_free(ctx);
  srvctx.ssl_ctx = nullptr;
}

//-----------------------------------------------------------------------------
TEST_F(ServerListeningTest, NullInputs)
{
  ASSERT_EQ(0, stcp_listening(nullptr, "19612"));
  ASSERT_EQ(0, stcp_listening(&srvctx, nullptr));
}

// ============================================================================
//              Integration tests (require live TLS-enabled daemon)
// ============================================================================

#ifdef TEST_INTEGRATION

//-----------------------------------------------------------------------------
TEST(VscpRemoteTcpIfTLS, IntegrationConnectSecure)
{
  VscpRemoteTcpIf vscpif;

  vscpif.setTLSOptions(false, TLS_CAFILE);

  ASSERT_EQ(VSCP_ERROR_SUCCESS,
            vscpif.doCmdOpen(TLS_HOST, TLS_USER, TLS_PASSWORD));
  ASSERT_TRUE(vscpif.isConnected());
  ASSERT_EQ(VSCP_ERROR_SUCCESS, vscpif.doCmdNOOP());
  ASSERT_EQ(VSCP_ERROR_SUCCESS, vscpif.doCmdClose());
}

//-----------------------------------------------------------------------------
TEST(VscpRemoteTcpIfTLS, IntegrationConnectSecureStcpPrefix)
{
  VscpRemoteTcpIf vscpif;

  if (strlen(TLS_CAFILE) > 0) {
    vscpif.setTLSOptions(false, TLS_CAFILE);
  }

  // Use stcp:// prefix
  ASSERT_EQ(VSCP_ERROR_SUCCESS,
            vscpif.doCmdOpen(TLS_HOST, TLS_USER, TLS_PASSWORD));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, vscpif.doCmdNOOP());
  ASSERT_EQ(VSCP_ERROR_SUCCESS, vscpif.doCmdClose());
}

//-----------------------------------------------------------------------------
TEST(VscpClientTcpTLS, IntegrationClientConnect)
{
  vscpClientTcp client;

  if (strlen(TLS_CAFILE) > 0) {
    client.setMqttCaFile(TLS_CAFILE);
  }
  if (strlen(TLS_CERTFILE) > 0) {
    client.setMqttCertFile(TLS_CERTFILE);
  }
  if (strlen(TLS_KEYFILE) > 0) {
    client.setMqttKeyFile(TLS_KEYFILE);
  }

  int rv = client.init(TLS_HOST, TLS_USER, TLS_PASSWORD, true);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);

  rv = client.connect();
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);

  ASSERT_TRUE(client.isConnected());

  rv = client.disconnect();
  ASSERT_EQ(VSCP_ERROR_SUCCESS, rv);
}

#endif // TEST_INTEGRATION
