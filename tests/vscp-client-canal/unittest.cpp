///////////////////////////////////////////////////////////////////////////////
// unittest.cpp
//
// Unit tests for vscpClientCanal class
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

#include <string>
#include <cstring>

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include <vscp-client-canal.h>
#include <vscphelper.h>

// ============================================================================
//                          Constructor / defaults
// ============================================================================

TEST(VscpClientCanal, DefaultConstructorState)
{
  vscpClientCanal client;

  // Type should be CANAL
  EXPECT_EQ(CVscpClient::connType::CANAL, client.getType());

  // Should not be connected
  EXPECT_FALSE(client.isConnected());

  // Callbacks should be inactive
  EXPECT_FALSE(client.isCallbackEvActive());
  EXPECT_FALSE(client.isCallbackExActive());

  // Name should be empty
  EXPECT_TRUE(client.getName().empty());

  // m_bRun should be true (ready to run)
  EXPECT_TRUE(client.m_bRun);
}

// ============================================================================
//                          JSON serialization
// ============================================================================

TEST(VscpClientCanal, GetConfigAsJsonDefaults)
{
  vscpClientCanal client;

  std::string config = client.getConfigAsJson();
  ASSERT_FALSE(config.empty());

  nlohmann::json j = nlohmann::json::parse(config);

  EXPECT_TRUE(j.contains("name"));
  EXPECT_TRUE(j.contains("path"));
  EXPECT_TRUE(j.contains("config"));
  EXPECT_TRUE(j.contains("flags"));

  // Defaults should be empty / zero
  EXPECT_TRUE(j["name"].get<std::string>().empty());
  EXPECT_TRUE(j["path"].get<std::string>().empty());
  EXPECT_TRUE(j["config"].get<std::string>().empty());
  EXPECT_EQ(0u, j["flags"].get<uint32_t>());
}

TEST(VscpClientCanal, GetConfigAsJsonWithName)
{
  vscpClientCanal client;
  client.setName("my-canal-driver");

  nlohmann::json j = nlohmann::json::parse(client.getConfigAsJson());
  EXPECT_EQ("my-canal-driver", j["name"].get<std::string>());
}

// ============================================================================
//                       initFromJson validation
// ============================================================================

TEST(VscpClientCanal, InitFromJsonMissingNameFails)
{
  spdlog::set_level(spdlog::level::off);
  vscpClientCanal client;

  nlohmann::json j;
  j["path"]   = "/usr/lib/dummy.so";
  j["config"] = "";
  j["flags"]  = 0;
  // Missing "name"

  EXPECT_FALSE(client.initFromJson(j.dump()));
  spdlog::set_level(spdlog::level::info);
}

TEST(VscpClientCanal, InitFromJsonMissingPathFails)
{
  spdlog::set_level(spdlog::level::off);
  vscpClientCanal client;

  nlohmann::json j;
  j["name"]   = "test-driver";
  j["config"] = "";
  j["flags"]  = 0;
  // Missing "path"

  EXPECT_FALSE(client.initFromJson(j.dump()));
  spdlog::set_level(spdlog::level::info);
}

TEST(VscpClientCanal, InitFromJsonInvalidJsonFails)
{
  spdlog::set_level(spdlog::level::off);
  vscpClientCanal client;

  EXPECT_FALSE(client.initFromJson("not valid json {{{"));
  spdlog::set_level(spdlog::level::info);
}

TEST(VscpClientCanal, InitFromJsonNameIsSet)
{
  // initFromJson will fail at init() (no real DLL), but the
  // name should still be set before init() is called
  vscpClientCanal client;

  nlohmann::json j;
  j["name"]     = "test-driver";
  j["path"]     = "/nonexistent/driver.so";
  j["config"]   = "some;config";
  j["flags"]    = 42;
  j["datarate"] = 0;

  // Will fail because the driver DLL doesn't exist, but name
  // should be set during parsing
  client.initFromJson(j.dump());
  EXPECT_EQ("test-driver", client.getName());
}

TEST(VscpClientCanal, InitFromJsonFlagsAsString)
{
  vscpClientCanal client;

  nlohmann::json j;
  j["name"]     = "test-driver";
  j["path"]     = "/nonexistent/driver.so";
  j["config"]   = "";
  j["flags"]    = "0x1234";  // String format
  j["datarate"] = "500000";  // String format

  // Will fail at DLL load, but parsing should handle string values
  client.initFromJson(j.dump());
  EXPECT_EQ("test-driver", client.getName());
}

TEST(VscpClientCanal, InitFromJsonDefaultConfigAndFlags)
{
  vscpClientCanal client;

  nlohmann::json j;
  j["name"]  = "test-driver";
  j["path"]  = "/nonexistent/driver.so";
  // No "config", "flags", or "datarate" — should default

  client.initFromJson(j.dump());
  EXPECT_EQ("test-driver", client.getName());
}

// ============================================================================
//                         Connection state
// ============================================================================

TEST(VscpClientCanal, IsConnectedInitiallyFalse)
{
  vscpClientCanal client;
  EXPECT_FALSE(client.isConnected());
}

// ============================================================================
//                         Timeout getters/setters
// ============================================================================

TEST(VscpClientCanal, ConnectionTimeout)
{
  vscpClientCanal client;

  // CANAL client doesn't implement timeouts — returns 0
  EXPECT_EQ(0u, client.getConnectionTimeout());

  client.setConnectionTimeout(5000);
  // Still returns 0 (not implemented)
  EXPECT_EQ(0u, client.getConnectionTimeout());
}

TEST(VscpClientCanal, ResponseTimeout)
{
  vscpClientCanal client;

  EXPECT_EQ(0u, client.getResponseTimeout());

  client.setResponseTimeout(3000);
  EXPECT_EQ(0u, client.getResponseTimeout());
}

// ============================================================================
//                              getwcyd
// ============================================================================

TEST(VscpClientCanal, GetwcydReturnsNone)
{
  vscpClientCanal client;
  uint64_t wcyd = 0xFFFF;

  int rv = client.getwcyd(wcyd);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
  EXPECT_EQ(VSCP_SERVER_CAPABILITY_NONE, wcyd);
}

// ============================================================================
//                              clear
// ============================================================================

TEST(VscpClientCanal, ClearReturnsSuccess)
{
  vscpClientCanal client;
  EXPECT_EQ(VSCP_ERROR_SUCCESS, client.clear());
}

// ============================================================================
//                           getinterfaces
// ============================================================================

TEST(VscpClientCanal, GetInterfacesReturnsEmptyList)
{
  vscpClientCanal client;
  std::deque<std::string> iflist;

  int rv = client.getinterfaces(iflist);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
  EXPECT_TRUE(iflist.empty());
}

// ============================================================================
//                           getversion
// ============================================================================

TEST(VscpClientCanal, GetVersionReturnsSuccess)
{
  vscpClientCanal client;
  uint8_t major = 0xFF, minor = 0xFF, release = 0xFF, build = 0xFF;

  int rv = client.getversion(&major, &minor, &release, &build);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
}

// ============================================================================
//                          Callback restrictions
// ============================================================================

TEST(VscpClientCanal, SetCallbackEvWhenNotConnected)
{
  vscpClientCanal client;
  int count = 0;

  int rv = client.setCallbackEv(
    [&count](vscpEvent &ev, void *pobj) { count++; },
    nullptr);

  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
  EXPECT_TRUE(client.isCallbackEvActive());
}

TEST(VscpClientCanal, SetCallbackExWhenNotConnected)
{
  vscpClientCanal client;
  int count = 0;

  int rv = client.setCallbackEx(
    [&count](vscpEventEx &ex, void *pobj) { count++; },
    nullptr);

  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
  EXPECT_TRUE(client.isCallbackExActive());
}

// ============================================================================
//                       Receive queue (no driver)
// ============================================================================

TEST(VscpClientCanal, ReceiveQueueInitiallyEmpty)
{
  vscpClientCanal client;
  EXPECT_TRUE(client.m_receiveQueue.empty());
}

// ============================================================================
//                         Filter (input)
// ============================================================================

TEST(VscpClientCanal, DefaultFilterAcceptsAll)
{
  vscpClientCanal client;

  // Default filter should be all zeros (accept all)
  EXPECT_EQ(0, client.m_filterIn.filter_priority);
  EXPECT_EQ(0, client.m_filterIn.filter_class);
  EXPECT_EQ(0, client.m_filterIn.filter_type);
  EXPECT_EQ(0, client.m_filterIn.mask_priority);
  EXPECT_EQ(0, client.m_filterIn.mask_class);
  EXPECT_EQ(0, client.m_filterIn.mask_type);
}
