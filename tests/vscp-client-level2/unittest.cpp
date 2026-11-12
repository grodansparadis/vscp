///////////////////////////////////////////////////////////////////////////////
// unittest.cpp
//
// Unit tests for vscpClientLevel2 class. A fake Level II driver
// (fakedriver.cpp) is loaded for real.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2026 Ake Hedman and contributors, the VSCP project
// <info@vscp.org>
//

#include <atomic>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>

#include <dlfcn.h>
#ifndef WIN32
#include <sys/stat.h>
#endif

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <vscp-client-level2.h>
#include <vscphelper.h>

using json = nlohmann::json;

namespace {

const char *TEST_GUID = "FF:FF:FF:FF:FF:FF:FF:F5:01:02:03:04:05:06:00:00";

struct FakeState {
  std::string path;
  std::string content;
  uint8_t guid[16];
  int openCount;
  int closeCount;
  bool configExisted;
};

typedef void (*FakeGetStateFn)(char *, size_t, char *, size_t, unsigned char *, int *, int *, int *);

// The client loads the driver with RTLD_NODELETE so this handle refers to
// the same instance, and its state, as the one the client uses
FakeState
fakeState(void)
{
  void *h = dlopen(FAKE_DRIVER_PATH, RTLD_NOW | RTLD_LOCAL);
  EXPECT_NE(nullptr, h);
  auto fn = reinterpret_cast<FakeGetStateFn>(dlsym(h, "FakeGetState"));
  EXPECT_NE(nullptr, fn);
  FakeState st;
  char path[1024];
  static char content[65536];
  int existed = 0;
  fn(path, sizeof(path), content, sizeof(content), st.guid, &st.openCount, &st.closeCount, &existed);
  st.path          = path;
  st.content       = content;
  st.configExisted = (0 != existed);
  dlclose(h);
  return st;
}

class Level2Test : public ::testing::Test {
protected:
  void SetUp() override
  {
    m_dir = std::filesystem::temp_directory_path() /
            ("vscp-level2-test-" + std::to_string(::testing::UnitTest::GetInstance()->random_seed()) + "-" +
             ::testing::UnitTest::GetInstance()->current_test_info()->name());
    std::filesystem::create_directories(m_dir);
    m_configFile = (m_dir / "driver.json").string();
    std::ofstream(m_configFile) << "{ \"setting\" : 42 }";
  }

  void TearDown() override
  {
    std::error_code ec;
    std::filesystem::remove_all(m_dir, ec);
  }

  json baseConfig(void)
  {
    json j;
    j["name"]        = "test";
    j["path-driver"] = FAKE_DRIVER_PATH;
    j["guid"]        = TEST_GUID;
    return j;
  }

  static vscpEventEx makeEx(uint16_t vscpClass, uint16_t vscpType, uint8_t sizeData)
  {
    vscpEventEx ex;
    memset(&ex, 0, sizeof(ex));
    ex.vscp_class = vscpClass;
    ex.vscp_type  = vscpType;
    ex.sizeData   = sizeData;
    for (uint8_t i = 0; i < sizeData; i++) {
      ex.data[i] = i + 1;
    }
    return ex;
  }

  std::filesystem::path m_dir;
  std::string m_configFile;
};

} // namespace

// ============================================================================
//                              Configuration
// ============================================================================

TEST_F(Level2Test, DefaultState)
{
  vscpClientLevel2 client;
  EXPECT_EQ(CVscpClient::connType::LEVEL2, client.getType());
  EXPECT_FALSE(client.isConnected());
  EXPECT_FALSE(client.isConfigEmbedded());

  uint16_t count = 99;
  EXPECT_EQ(VSCP_ERROR_SUCCESS, client.getcount(&count));
  EXPECT_EQ(0, count);

  vscpEvent ev;
  memset(&ev, 0, sizeof(ev));
  EXPECT_EQ(VSCP_ERROR_RCV_EMPTY, client.receive(ev));
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, client.send(ev));
}

TEST_F(Level2Test, InvalidJsonChangesNothing)
{
  vscpClientLevel2 client;
  json j          = baseConfig();
  j["path-config"] = m_configFile;
  ASSERT_TRUE(client.initFromJson(j.dump()));

  EXPECT_FALSE(client.initFromJson("not json"));
  EXPECT_FALSE(client.initFromJson("[1,2]"));

  json bad = j;
  bad["path-driver"] = "/other";
  bad["guid"]        = "not a guid";
  EXPECT_FALSE(client.initFromJson(bad.dump()));

  bad         = j;
  bad["path-driver"] = "/other";
  bad["config"]      = 42;
  EXPECT_FALSE(client.initFromJson(bad.dump()));

  bad                  = j;
  bad["config-suffix"] = "../evil";
  EXPECT_FALSE(client.initFromJson(bad.dump()));

  bad                     = j;
  bad["response-timeout"] = -1;
  EXPECT_FALSE(client.initFromJson(bad.dump()));

  EXPECT_EQ(std::string(FAKE_DRIVER_PATH), client.getDriverPath());
  EXPECT_EQ(m_configFile, client.getConfigPath());
}

TEST_F(Level2Test, JsonRoundTrip)
{
  vscpClientLevel2 client;
  json j                  = baseConfig();
  j["config"]             = { { "port", 9598 }, { "user", "admin" } };
  j["response-timeout"]   = 250;
  j["class-filter"]       = 10;
  j["class-mask"]         = 0xffff;
  ASSERT_TRUE(client.initFromJson(j.dump()));
  EXPECT_TRUE(client.isConfigEmbedded());

  json out = json::parse(client.getConfigAsJson());
  EXPECT_EQ(static_cast<int>(CVscpClient::connType::LEVEL2), out["type"].get<int>());
  EXPECT_EQ("test", out["name"]);
  EXPECT_EQ(FAKE_DRIVER_PATH, out["path-driver"].get<std::string>());
  EXPECT_TRUE(out["config"].is_object());
  EXPECT_EQ(9598, out["config"]["port"].get<int>());
  EXPECT_EQ(250, out["response-timeout"].get<int>());
  EXPECT_EQ(10, out["class-filter"].get<int>());
  EXPECT_FALSE(out.contains("path-config"));

  vscpClientLevel2 copy;
  ASSERT_TRUE(copy.initFromJson(out.dump()));
  EXPECT_EQ(client.getEmbeddedConfig(), copy.getEmbeddedConfig());

  // Switching to a file clears the embedded configuration
  json f;
  f["config"]      = nullptr;
  f["path-config"] = m_configFile;
  ASSERT_TRUE(copy.initFromJson(f.dump()));
  EXPECT_FALSE(copy.isConfigEmbedded());
  EXPECT_EQ(m_configFile, copy.getConfigPath());
}

// ============================================================================
//                              Connecting
// ============================================================================

TEST_F(Level2Test, ConnectWithConfigFile)
{
  vscpClientLevel2 client;
  json j           = baseConfig();
  j["path-config"] = m_configFile;
  ASSERT_TRUE(client.initFromJson(j.dump()));

  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.connect());
  EXPECT_TRUE(client.isConnected());
  EXPECT_EQ(m_configFile, client.getActiveConfigPath());

  FakeState st = fakeState();
  EXPECT_EQ(m_configFile, st.path);
  EXPECT_EQ("{ \"setting\" : 42 }", st.content);
  uint8_t guid[16];
  ASSERT_TRUE(vscp_parseGuid(guid, TEST_GUID));
  EXPECT_EQ(0, memcmp(guid, st.guid, 16));

  // Configuration can not change while connected
  EXPECT_FALSE(client.initFromJson(j.dump()));

  uint8_t major, minor, release, build;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.getversion(&major, &minor, &release, &build));
  EXPECT_EQ(1, major);
  EXPECT_EQ(2, minor);
  EXPECT_EQ(3, release);
  EXPECT_EQ(4, build);
  EXPECT_EQ("Fake Level II driver", client.getVendorString());

  const int closes = st.closeCount;
  EXPECT_EQ(VSCP_ERROR_SUCCESS, client.disconnect());
  EXPECT_FALSE(client.isConnected());
  EXPECT_EQ(closes + 1, fakeState().closeCount);

  // The user's file is never removed
  EXPECT_TRUE(std::filesystem::exists(m_configFile));
}

TEST_F(Level2Test, EmbeddedJsonConfigUsesPrivateTempFile)
{
  vscpClientLevel2 client;
  json j      = baseConfig();
  j["config"] = { { "key", "secret" }, { "port", 9598 } };
  ASSERT_TRUE(client.initFromJson(j.dump()));

  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.connect());
  const std::string tmp = client.getActiveConfigPath();
  ASSERT_FALSE(tmp.empty());
  EXPECT_EQ(".json", std::filesystem::path(tmp).extension().string());
  EXPECT_TRUE(std::filesystem::exists(tmp));

  FakeState st = fakeState();
  EXPECT_EQ(tmp, st.path);
  json written = json::parse(st.content);
  EXPECT_EQ("secret", written["key"]);
  EXPECT_EQ(9598, written["port"].get<int>());

#ifndef WIN32
  struct stat sb;
  ASSERT_EQ(0, stat(tmp.c_str(), &sb));
  EXPECT_EQ(0600, sb.st_mode & 0777);
#endif

  EXPECT_EQ(VSCP_ERROR_SUCCESS, client.disconnect());
  EXPECT_FALSE(std::filesystem::exists(tmp));
}

TEST_F(Level2Test, EmbeddedTextConfigSuffix)
{
  vscpClientLevel2 client;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.initEmbedded(FAKE_DRIVER_PATH, "  <config port=\"9598\"/>", TEST_GUID));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.connect());
  EXPECT_EQ(".xml", std::filesystem::path(client.getActiveConfigPath()).extension().string());
  EXPECT_EQ("  <config port=\"9598\"/>", fakeState().content);
  client.disconnect();

  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.initEmbedded(FAKE_DRIVER_PATH, "port=9598", "", ".ini"));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.connect());
  EXPECT_EQ(".ini", std::filesystem::path(client.getActiveConfigPath()).extension().string());
  client.disconnect();

  EXPECT_EQ(VSCP_ERROR_PARAMETER, client.initEmbedded(FAKE_DRIVER_PATH, "x", "", "/x"));
  EXPECT_EQ(VSCP_ERROR_PARAMETER, client.initEmbedded(FAKE_DRIVER_PATH, "", ""));
}

TEST_F(Level2Test, FailedOpenRemovesTempFile)
{
  vscpClientLevel2 client;
  json j      = baseConfig();
  j["config"] = { { "mode", "fail" } };
  ASSERT_TRUE(client.initFromJson(j.dump()));

  EXPECT_EQ(VSCP_ERROR_CONNECTION, client.connect());
  EXPECT_FALSE(client.isConnected());

  FakeState st = fakeState();
  EXPECT_TRUE(st.configExisted);
  EXPECT_FALSE(std::filesystem::exists(st.path));
}

TEST_F(Level2Test, ThrowingDriverDoesNotEscape)
{
  vscpClientLevel2 client;
  json j      = baseConfig();
  j["config"] = { { "mode", "throw" } };
  ASSERT_TRUE(client.initFromJson(j.dump()));

  EXPECT_EQ(VSCP_ERROR_CONNECTION, client.connect());
  EXPECT_FALSE(client.isConnected());
  EXPECT_FALSE(std::filesystem::exists(fakeState().path));
}

TEST_F(Level2Test, MissingDriverOrConfig)
{
  vscpClientLevel2 client;
  json j           = baseConfig();
  j["path-driver"] = (m_dir / "no-such-driver.so").string();
  j["path-config"] = m_configFile;
  ASSERT_TRUE(client.initFromJson(j.dump()));
  EXPECT_NE(VSCP_ERROR_SUCCESS, client.connect());
  EXPECT_FALSE(client.isConnected());

  j["path-driver"] = NOT_A_DRIVER_PATH;
  ASSERT_TRUE(client.initFromJson(j.dump()));
  EXPECT_EQ(VSCP_ERROR_INIT_FAIL, client.connect());

  json n           = baseConfig();
  n["path-config"] = "";
  vscpClientLevel2 noConfig;
  ASSERT_TRUE(noConfig.initFromJson(n.dump()));
  EXPECT_EQ(VSCP_ERROR_PARAMETER, noConfig.connect());
}

// ============================================================================
//                              Events
// ============================================================================

TEST_F(Level2Test, SendAndReceiveThroughQueue)
{
  vscpClientLevel2 client;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.init(FAKE_DRIVER_PATH, m_configFile, TEST_GUID));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.connect());

  vscpEventEx ex = makeEx(VSCP_CLASS1_MEASUREMENT, 6, 3);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.send(ex));

  vscpEventEx rx;
  memset(&rx, 0, sizeof(rx));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.receiveBlocking(rx, 2000));
  EXPECT_EQ(VSCP_CLASS1_MEASUREMENT, rx.vscp_class);
  EXPECT_EQ(6, rx.vscp_type);
  ASSERT_EQ(3, rx.sizeData);
  EXPECT_EQ(1, rx.data[0]);
  EXPECT_EQ(3, rx.data[2]);
  EXPECT_NE(0u, rx.timestamp);

  // Full size Level II event
  vscpEventEx big = makeEx(VSCP_CLASS2_LEVEL1_PROTOCOL, 1, 255);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.send(big));
  vscpEvent ev;
  memset(&ev, 0, sizeof(ev));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.receiveBlocking(ev, 2000));
  EXPECT_EQ(255, ev.sizeData);
  EXPECT_EQ(255, ev.pdata[254]);
  vscp_deleteEvent(&ev);

  EXPECT_EQ(VSCP_ERROR_TIMEOUT, client.receiveBlocking(rx, 50));

  client.disconnect();
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, client.send(ex));
}

TEST_F(Level2Test, FilterDropsEvents)
{
  vscpClientLevel2 client;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.init(FAKE_DRIVER_PATH, m_configFile));

  vscpEventFilter filter;
  vscp_clearVSCPFilter(&filter);
  filter.filter_class = VSCP_CLASS1_MEASUREMENT;
  filter.mask_class   = 0xffff;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.setfilter(filter));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.connect());

  vscpEventEx other = makeEx(VSCP_CLASS1_INFORMATION, 3, 1);
  vscpEventEx meas  = makeEx(VSCP_CLASS1_MEASUREMENT, 6, 1);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.send(other));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.send(meas));

  vscpEventEx rx;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.receiveBlocking(rx, 2000));
  EXPECT_EQ(VSCP_CLASS1_MEASUREMENT, rx.vscp_class);
  EXPECT_EQ(VSCP_ERROR_TIMEOUT, client.receiveBlocking(rx, 200));
  client.disconnect();
}

TEST_F(Level2Test, CallbackReceivesEvents)
{
  vscpClientLevel2 client;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.init(FAKE_DRIVER_PATH, m_configFile));

  std::atomic<int> received{ 0 };
  std::atomic<int> lastType{ -1 };
  client.setCallbackEx(
    [&](vscpEventEx &ex, void * /*pobj*/) {
      lastType = ex.vscp_type;
      received++;
    },
    nullptr);
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.connect());

  for (int i = 0; i < 5; i++) {
    vscpEventEx ex = makeEx(VSCP_CLASS1_INFORMATION, static_cast<uint16_t>(i), 0);
    ASSERT_EQ(VSCP_ERROR_SUCCESS, client.send(ex));
  }

  for (int i = 0; (i < 200) && (received < 5); i++) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  EXPECT_EQ(5, received.load());
  EXPECT_EQ(4, lastType.load());

  // Events go to the callback, not the queue
  uint16_t count = 99;
  client.getcount(&count);
  EXPECT_EQ(0, count);
  client.disconnect();
}

TEST_F(Level2Test, ReconnectAndDestructWhileConnected)
{
  {
    vscpClientLevel2 client;
    json j      = baseConfig();
    j["config"] = { { "a", 1 } };
    ASSERT_TRUE(client.initFromJson(j.dump()));
    for (int i = 0; i < 3; i++) {
      ASSERT_EQ(VSCP_ERROR_SUCCESS, client.connect());
      EXPECT_EQ(VSCP_ERROR_SUCCESS, client.connect()); // Already connected
      ASSERT_EQ(VSCP_ERROR_SUCCESS, client.disconnect());
    }
    ASSERT_EQ(VSCP_ERROR_SUCCESS, client.connect());
    vscpEventEx ex = makeEx(VSCP_CLASS1_INFORMATION, 1, 0);
    client.send(ex);
    // Destructor disconnects and removes the temporary file
  }
  FakeState st = fakeState();
  EXPECT_FALSE(std::filesystem::exists(st.path));
  EXPECT_EQ(st.openCount > 0, true);
}
