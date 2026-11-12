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

// ============================================================================
//                 Filters with a (fake) CANAL driver
// ============================================================================

#ifndef WIN32
#include <dlfcn.h>

namespace {

// CAN id: priority 0, class, type, nickname
uint32_t
canId(uint16_t vscp_class, uint8_t vscp_type, uint8_t nickname)
{
  return ((uint32_t) vscp_class << 16) | ((uint32_t) vscp_type << 8) | nickname;
}

std::string
fakeConfig(const std::string &devices, const std::string &extra = "")
{
  return std::string("{\"name\":\"fake\",\"path\":\"") + FAKE_CANAL_PATH + "\",\"config\":\"" + devices + "\"" +
         extra + "}";
}

std::string
ids(std::initializer_list<uint32_t> list)
{
  std::string s;
  for (uint32_t id : list) {
    if (!s.empty()) {
      s += ",";
    }
    s += std::to_string(id);
  }
  return s;
}

// Received events (as their data byte, the index of the frame in the driver)
std::string
drain(vscpClientCanal &client)
{
  std::string got;
  vscpEvent ev = {};
  while (VSCP_ERROR_SUCCESS == client.receive(ev)) {
    got += std::to_string(ev.sizeData ? ev.pdata[0] : 255) + " ";
    delete[] ev.pdata;
    ev.pdata    = nullptr;
    ev.sizeData = 0;
  }
  return got;
}

template<typename T>
T
fakeFn(const char *name)
{
  void *h = dlopen(FAKE_CANAL_PATH, RTLD_NOW | RTLD_NOLOAD);
  T fn    = h ? (T) dlsym(h, name) : nullptr;
  if (h) {
    dlclose(h);
  }
  return fn;
}

} // namespace

TEST(VscpClientCanal, NoFilterReceivesAll)
{
  vscpClientCanal client;
  ASSERT_TRUE(client.initFromJson(fakeConfig(ids({ canId(10, 6, 1), canId(20, 3, 2), canId(10, 1, 3) }))));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.connect());
  EXPECT_EQ("0 1 2 ", drain(client));
  client.disconnect();
}

TEST(VscpClientCanal, CanIdFilterFromJson)
{
  // Only class 10, any type and node
  vscpClientCanal client;
  ASSERT_TRUE(client.initFromJson(fakeConfig(ids({ canId(10, 6, 1), canId(20, 3, 2), canId(10, 1, 3) }),
                                             ",\"filter\":" + std::to_string(canId(10, 0, 0)) +
                                               ",\"mask\":\"0x01ff0000\"")));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.connect());

  // Handed to the driver
  auto lastFilter = fakeFn<unsigned long (*)(void)>("fake_lastFilter");
  auto lastMask   = fakeFn<unsigned long (*)(void)>("fake_lastMask");
  ASSERT_NE(nullptr, lastFilter);
  ASSERT_NE(nullptr, lastMask);
  EXPECT_EQ(canId(10, 0, 0), lastFilter());
  EXPECT_EQ(0x01ff0000u, lastMask());

  // and applied by the client even though the driver does not filter
  EXPECT_EQ("0 2 ", drain(client));
  client.disconnect();
}

TEST(VscpClientCanal, CanIdFilterOnNickname)
{
  vscpClientCanal client;
  ASSERT_TRUE(client.initFromJson(
    fakeConfig(ids({ canId(10, 6, 1), canId(20, 3, 2), canId(10, 1, 3) }), ",\"filter\":2,\"mask\":255")));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.connect());
  EXPECT_EQ("1 ", drain(client));
  client.disconnect();
}

TEST(VscpClientCanal, SetfilterFiltersEventsAndSetsDriverFilterAndMask)
{
  vscpClientCanal client;
  ASSERT_TRUE(client.initFromJson(fakeConfig(ids({ canId(10, 6, 1), canId(20, 3, 2), canId(10, 1, 3) }))));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.connect());

  vscpEventFilter filter;
  vscp_clearVSCPFilter(&filter);
  filter.filter_class = 10;
  filter.mask_class   = 0xffff;
  filter.filter_type  = 1;
  filter.mask_type    = 0xff;
  EXPECT_EQ(VSCP_ERROR_SUCCESS, client.setfilter(filter));

  // Both filter and mask reach the driver
  auto setCount   = fakeFn<int (*)(void)>("fake_setCount");
  auto lastFilter = fakeFn<unsigned long (*)(void)>("fake_lastFilter");
  auto lastMask   = fakeFn<unsigned long (*)(void)>("fake_lastMask");
  EXPECT_EQ(2, setCount());
  EXPECT_EQ(vscp_getCANALidFromData(0, 10, 1), lastFilter());
  EXPECT_EQ(vscp_getCANALidFromData(0, 0xffff, 0xff), lastMask());

  EXPECT_EQ("2 ", drain(client));
  client.disconnect();
}

TEST(VscpClientCanal, InvalidFilterTypeFails)
{
  vscpClientCanal client;
  EXPECT_FALSE(client.initFromJson(fakeConfig("", ",\"filter\":true")));
}
#endif
