// unittest.cpp
//
// Unit tests for vscpClientUdp
//
// These tests exercise the vscpClientUdp class without requiring
// actual network communication. Constructor defaults, JSON config
// serialization/parsing, getters/setters, filter, receive queue,
// version, capabilities, and callbacks are all tested locally.
//

#include <gtest/gtest.h>

#include <semaphore.h>
#include <vscp-client-udp.h>
#include <vscphelper.h>

#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

// ---------------------------------------------------------------------------
//                       Constructor / Default State
// ---------------------------------------------------------------------------

TEST(VscpClientUdp, ConstructorDefaults)
{
  vscpClientUdp client;
  EXPECT_EQ(CVscpClient::connType::UDP, client.getType());
  EXPECT_FALSE(client.isConnected());
  EXPECT_FALSE(client.m_bEncrypt);
  EXPECT_EQ(VSCP_ENCRYPTION_NONE, client.m_encryptType);
  EXPECT_EQ(0, client.m_sock);
}

TEST(VscpClientUdp, ConstructorNotConnected)
{
  vscpClientUdp client;
  EXPECT_FALSE(client.isConnected());
}

TEST(VscpClientUdp, ReceiveQueueInitiallyEmpty)
{
  vscpClientUdp client;
  EXPECT_TRUE(client.m_receiveQueue.empty());
}

TEST(VscpClientUdp, DefaultKeyIsSet)
{
  vscpClientUdp client;
  // Default key should not be all zeros (VSCP_DEFAULT_KEY16 is set)
  bool allZero = true;
  for (int i = 0; i < 32; i++) {
    if (client.m_key[i] != 0) {
      allZero = false;
      break;
    }
  }
  EXPECT_FALSE(allZero);
}

// ---------------------------------------------------------------------------
//                           Version
// ---------------------------------------------------------------------------

TEST(VscpClientUdp, GetVersion)
{
  vscpClientUdp client;
  uint8_t major = 0xff, minor = 0xff, release = 0xff, build = 0xff;
  int rv = client.getversion(&major, &minor, &release, &build);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
  EXPECT_EQ(1, major);
  EXPECT_EQ(0, minor);
  EXPECT_EQ(0, release);
  EXPECT_EQ(0, build);
}

TEST(VscpClientUdp, GetVersionNullMajor)
{
  vscpClientUdp client;
  uint8_t minor, release, build;
  int rv = client.getversion(nullptr, &minor, &release, &build);
  EXPECT_EQ(VSCP_ERROR_INVALID_POINTER, rv);
}

TEST(VscpClientUdp, GetVersionNullMinor)
{
  vscpClientUdp client;
  uint8_t major, release, build;
  int rv = client.getversion(&major, nullptr, &release, &build);
  EXPECT_EQ(VSCP_ERROR_INVALID_POINTER, rv);
}

TEST(VscpClientUdp, GetVersionNullRelease)
{
  vscpClientUdp client;
  uint8_t major, minor, build;
  int rv = client.getversion(&major, &minor, nullptr, &build);
  EXPECT_EQ(VSCP_ERROR_INVALID_POINTER, rv);
}

TEST(VscpClientUdp, GetVersionNullBuild)
{
  vscpClientUdp client;
  uint8_t major, minor, release;
  int rv = client.getversion(&major, &minor, &release, nullptr);
  EXPECT_EQ(VSCP_ERROR_INVALID_POINTER, rv);
}

// ---------------------------------------------------------------------------
//                          Interfaces
// ---------------------------------------------------------------------------

TEST(VscpClientUdp, GetInterfaces)
{
  vscpClientUdp client;
  std::deque<std::string> iflist;
  iflist.push_back("dummy");
  int rv = client.getinterfaces(iflist);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
  // getinterfaces clears the list
  EXPECT_TRUE(iflist.empty());
}

// ---------------------------------------------------------------------------
//                         Capabilities
// ---------------------------------------------------------------------------

TEST(VscpClientUdp, GetWcyd)
{
  vscpClientUdp client;
  uint64_t wcyd = 0xFFFF;
  int rv        = client.getwcyd(wcyd);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
  EXPECT_EQ(0u, wcyd);
}

// ---------------------------------------------------------------------------
//                       getcount / clear
// ---------------------------------------------------------------------------

TEST(VscpClientUdp, GetcountEmpty)
{
  vscpClientUdp client;
  uint16_t count = 0xFFFF;
  int rv         = client.getcount(&count);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
  EXPECT_EQ(0u, count);
}

TEST(VscpClientUdp, GetcountNullPointer)
{
  vscpClientUdp client;
  int rv = client.getcount(nullptr);
  EXPECT_EQ(VSCP_ERROR_INVALID_POINTER, rv);
}

TEST(VscpClientUdp, ClearEmptyQueue)
{
  vscpClientUdp client;
  int rv = client.clear();
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
}

// ---------------------------------------------------------------------------
//                    Receive on empty queue
// ---------------------------------------------------------------------------

TEST(VscpClientUdp, ReceiveEmptyQueueEvent)
{
  vscpClientUdp client;
  vscpEvent ev;
  memset(&ev, 0, sizeof(ev));
  int rv = client.receive(ev);
  EXPECT_EQ(VSCP_ERROR_FIFO_EMPTY, rv);
}

TEST(VscpClientUdp, ReceiveEmptyQueueEventEx)
{
  vscpClientUdp client;
  vscpEventEx ex;
  memset(&ex, 0, sizeof(ex));
  int rv = client.receive(ex);
  EXPECT_EQ(VSCP_ERROR_FIFO_EMPTY, rv);
}

TEST(VscpClientUdp, ReceiveEmptyQueueCanal)
{
  vscpClientUdp client;
  canalMsg msg;
  memset(&msg, 0, sizeof(msg));
  int rv = client.receive(msg);
  EXPECT_EQ(VSCP_ERROR_FIFO_EMPTY, rv);
}

// ---------------------------------------------------------------------------
//                   Receive blocking (timeout)
// ---------------------------------------------------------------------------

TEST(VscpClientUdp, ReceiveBlockingTimeout)
{
  vscpClientUdp client;
  vscpEvent ev;
  memset(&ev, 0, sizeof(ev));
  int rv = client.receiveBlocking(ev, 10);
  EXPECT_EQ(VSCP_ERROR_TIMEOUT, rv);
}

TEST(VscpClientUdp, ReceiveBlockingExTimeout)
{
  vscpClientUdp client;
  vscpEventEx ex;
  memset(&ex, 0, sizeof(ex));
  int rv = client.receiveBlocking(ex, 10);
  EXPECT_EQ(VSCP_ERROR_TIMEOUT, rv);
}

TEST(VscpClientUdp, ReceiveBlockingCanalTimeout)
{
  vscpClientUdp client;
  canalMsg msg;
  memset(&msg, 0, sizeof(msg));
  int rv = client.receiveBlocking(msg, 10);
  EXPECT_EQ(VSCP_ERROR_TIMEOUT, rv);
}

// ---------------------------------------------------------------------------
//                    Send not connected
// ---------------------------------------------------------------------------

TEST(VscpClientUdp, SendEventNotConnected)
{
  vscpClientUdp client;
  vscpEvent ev;
  memset(&ev, 0, sizeof(ev));
  ev.vscp_class = VSCP_CLASS1_MEASUREMENT;
  ev.vscp_type  = 6;
  ev.sizeData   = 0;
  ev.pdata      = nullptr;
  int rv        = client.send(ev);
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, rv);
}

// ---------------------------------------------------------------------------
//                            setfilter
// ---------------------------------------------------------------------------

TEST(VscpClientUdp, SetFilter)
{
  vscpClientUdp client;
  vscpEventFilter filter;
  memset(&filter, 0, sizeof(filter));
  filter.filter_priority = 3;
  filter.filter_class    = 10;
  filter.filter_type     = 6;
  filter.mask_priority   = 7;
  filter.mask_class      = 0xFFFF;
  filter.mask_type       = 0xFF;
  // Set a GUID byte
  filter.filter_GUID[0]  = 0xAA;
  filter.mask_GUID[0]    = 0xFF;

  int rv = client.setfilter(filter);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);

  // Verify all fields copied
  EXPECT_EQ(3, client.m_filter.filter_priority);
  EXPECT_EQ(10, client.m_filter.filter_class);
  EXPECT_EQ(6, client.m_filter.filter_type);
  EXPECT_EQ(7, client.m_filter.mask_priority);
  EXPECT_EQ(0xFFFF, client.m_filter.mask_class);
  EXPECT_EQ(0xFF, client.m_filter.mask_type);
  EXPECT_EQ(0xAA, client.m_filter.filter_GUID[0]);
  EXPECT_EQ(0xFF, client.m_filter.mask_GUID[0]);
}

// ---------------------------------------------------------------------------
//                      Connection / Response timeout
// ---------------------------------------------------------------------------

TEST(VscpClientUdp, ConnectionTimeout)
{
  vscpClientUdp client;
  client.setConnectionTimeout(5000);
  EXPECT_EQ(0u, client.getConnectionTimeout());
}

TEST(VscpClientUdp, ResponseTimeout)
{
  vscpClientUdp client;
  client.setResponseTimeout(5000);
  EXPECT_EQ(0u, client.getResponseTimeout());
}

// ---------------------------------------------------------------------------
//                         ConnType / Name
// ---------------------------------------------------------------------------

TEST(VscpClientUdp, ConnTypeIsUdp)
{
  vscpClientUdp client;
  EXPECT_EQ(CVscpClient::connType::UDP, client.getType());
}

TEST(VscpClientUdp, SetGetName)
{
  vscpClientUdp client;
  client.setName("test-udp");
  EXPECT_EQ("test-udp", client.getName());
}

TEST(VscpClientUdp, FullLevel2Flag)
{
  vscpClientUdp client;
  EXPECT_FALSE(client.isFullLevel2());
  client.setFullLevel2(true);
  EXPECT_TRUE(client.isFullLevel2());
}

// ---------------------------------------------------------------------------
//                        getConfigAsJson
// ---------------------------------------------------------------------------

TEST(VscpClientUdp, GetConfigAsJsonDefault)
{
  vscpClientUdp client;
  std::string config = client.getConfigAsJson();
  EXPECT_FALSE(config.empty());

  json j = json::parse(config);
  EXPECT_TRUE(j.contains("ip"));
  EXPECT_TRUE(j.contains("encryption"));
  EXPECT_TRUE(j.contains("key"));
  EXPECT_TRUE(j.contains("padding"));
  EXPECT_TRUE(j.contains("interface"));

  // Default broadcast address
  std::string ip = j["ip"].get<std::string>();
  EXPECT_NE(std::string::npos, ip.find("255.255.255.255"));
  EXPECT_NE(std::string::npos, ip.find("udp://"));

  // No encryption by default
  EXPECT_EQ(0, j["encryption"].get<int>());
  EXPECT_EQ("pkcs7", j["padding"].get<std::string>());

  // Filter fields present
  EXPECT_TRUE(j.contains("priority-filter"));
  EXPECT_TRUE(j.contains("priority-mask"));
  EXPECT_TRUE(j.contains("class-filter"));
  EXPECT_TRUE(j.contains("class-mask"));
  EXPECT_TRUE(j.contains("type-filter"));
  EXPECT_TRUE(j.contains("type-mask"));
  EXPECT_TRUE(j.contains("guid-filter"));
  EXPECT_TRUE(j.contains("guid-mask"));
}

TEST(VscpClientUdp, GetConfigAsJsonEncryptionAES128)
{
  vscpClientUdp client;
  client.m_encryptType = VSCP_ENCRYPTION_AES128;
  std::string config   = client.getConfigAsJson();
  json j               = json::parse(config);
  EXPECT_EQ(VSCP_ENCRYPTION_AES128, j["encryption"].get<int>());
  // AES-128 key is 16 bytes = 32 hex chars
  std::string key = j["key"].get<std::string>();
  EXPECT_EQ(32u, key.size());
}

TEST(VscpClientUdp, GetConfigAsJsonEncryptionAES192)
{
  vscpClientUdp client;
  client.m_encryptType = VSCP_ENCRYPTION_AES192;
  std::string config   = client.getConfigAsJson();
  json j               = json::parse(config);
  EXPECT_EQ(VSCP_ENCRYPTION_AES192, j["encryption"].get<int>());
  std::string key = j["key"].get<std::string>();
  EXPECT_EQ(48u, key.size());
}

TEST(VscpClientUdp, GetConfigAsJsonEncryptionAES256)
{
  vscpClientUdp client;
  client.m_encryptType = VSCP_ENCRYPTION_AES256;
  std::string config   = client.getConfigAsJson();
  json j               = json::parse(config);
  EXPECT_EQ(VSCP_ENCRYPTION_AES256, j["encryption"].get<int>());
  std::string key = j["key"].get<std::string>();
  EXPECT_EQ(64u, key.size());
}

// ---------------------------------------------------------------------------
//                         initFromJson
// ---------------------------------------------------------------------------

TEST(VscpClientUdp, InitFromJsonBasic)
{
  vscpClientUdp client;
  json j;
  j["interface"]  = "eth0";
  j["ip"]         = "udp://192.168.1.100:9999";
  j["encryption"] = 0;
  j["padding"]    = "pkcs7";
  // Note: omit "key" to avoid triggering filter parsing block
  EXPECT_TRUE(client.initFromJson(j.dump()));
  EXPECT_EQ("pkcs7", json::parse(client.getConfigAsJson())["padding"].get<std::string>());
}

TEST(VscpClientUdp, InitFromJsonRejectsUnknownPadding)
{
  vscpClientUdp client;
  EXPECT_FALSE(client.initFromJson(R"({"padding":"auto"})"));
  EXPECT_FALSE(client.initFromJson(R"({"padding":"zero"})"));
  EXPECT_FALSE(client.initFromJson(R"({"padding":1})"));
}

TEST(VscpClientUdp, InitFromJsonInvalidJson)
{
  vscpClientUdp client;
  EXPECT_FALSE(client.initFromJson("not json at all"));
}

TEST(VscpClientUdp, InitFromJsonEmptyObject)
{
  vscpClientUdp client;
  EXPECT_TRUE(client.initFromJson("{}"));
}

TEST(VscpClientUdp, InitFromJsonIpWithPort)
{
  vscpClientUdp client;
  json j;
  j["ip"]         = "udp://10.0.0.1:1234";
  j["encryption"] = 0;
  EXPECT_TRUE(client.initFromJson(j.dump()));

  // Verify via getConfigAsJson roundtrip
  std::string config = client.getConfigAsJson();
  json j2            = json::parse(config);
  std::string ip     = j2["ip"].get<std::string>();
  EXPECT_NE(std::string::npos, ip.find("10.0.0.1"));
  EXPECT_NE(std::string::npos, ip.find("1234"));
}

TEST(VscpClientUdp, InitFromJsonIpWithoutPrefix)
{
  vscpClientUdp client;
  json j;
  j["ip"]         = "192.168.1.50:8888";
  j["encryption"] = 0;
  EXPECT_TRUE(client.initFromJson(j.dump()));

  std::string config = client.getConfigAsJson();
  json j2            = json::parse(config);
  std::string ip     = j2["ip"].get<std::string>();
  EXPECT_NE(std::string::npos, ip.find("192.168.1.50"));
  EXPECT_NE(std::string::npos, ip.find("8888"));
}

TEST(VscpClientUdp, InitFromJsonIpWithoutPort)
{
  vscpClientUdp client;
  json j;
  j["ip"]         = "udp://10.0.0.5";
  j["encryption"] = 0;
  EXPECT_TRUE(client.initFromJson(j.dump()));

  // Address should be set; port stays at default
  std::string config = client.getConfigAsJson();
  json j2            = json::parse(config);
  std::string ip     = j2["ip"].get<std::string>();
  EXPECT_NE(std::string::npos, ip.find("10.0.0.5"));
}

TEST(VscpClientUdp, InitFromJsonEncryptionAES128)
{
  vscpClientUdp client;
  json j;
  j["ip"]         = "udp://10.0.0.1:9598";
  j["encryption"] = VSCP_ENCRYPTION_AES128;
  j["key"]        = "A4A86F7D7E119BA3F0CD06881E371044";
  // Need filter fields since they're parsed inside the key block
  j["priority-filter"] = 0;
  j["priority-mask"]   = 0;
  j["class-filter"]    = 0;
  j["class-mask"]      = 0;
  j["type-filter"]     = 0;
  j["type-mask"]       = 0;
  j["guid-filter"]     = "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00";
  j["guid-mask"]       = "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00";
  EXPECT_TRUE(client.initFromJson(j.dump()));
  EXPECT_EQ(VSCP_ENCRYPTION_AES128, client.m_encryptType);
}

TEST(VscpClientUdp, InitFromJsonEncryptionAES256)
{
  vscpClientUdp client;
  json j;
  j["ip"]              = "udp://10.0.0.1:9598";
  j["encryption"]      = VSCP_ENCRYPTION_AES256;
  j["key"]             = "A4A86F7D7E119BA3F0CD06881E371044A4A86F7D7E119BA3F0CD06881E371044";
  j["priority-filter"] = 0;
  j["priority-mask"]   = 0;
  j["class-filter"]    = 0;
  j["class-mask"]      = 0;
  j["type-filter"]     = 0;
  j["type-mask"]       = 0;
  j["guid-filter"]     = "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00";
  j["guid-mask"]       = "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00";
  EXPECT_TRUE(client.initFromJson(j.dump()));
  EXPECT_EQ(VSCP_ENCRYPTION_AES256, client.m_encryptType);
}

TEST(VscpClientUdp, ConfigRoundtrip)
{
  vscpClientUdp client;
  client.m_encryptType = VSCP_ENCRYPTION_AES128;

  std::string config1 = client.getConfigAsJson();
  json j1             = json::parse(config1);

  vscpClientUdp client2;
  EXPECT_TRUE(client2.initFromJson(config1));

  std::string config2 = client2.getConfigAsJson();
  json j2             = json::parse(config2);

  EXPECT_EQ(j1["encryption"], j2["encryption"]);
  EXPECT_EQ(j1["ip"], j2["ip"]);
}

// ---------------------------------------------------------------------------
//                          Callbacks
// ---------------------------------------------------------------------------

static bool g_evCallbackFired = false;
static bool g_exCallbackFired = false;

static void
testCallbackEv(vscpEvent & /*ev*/, void * /*pobj*/)
{
  g_evCallbackFired = true;
}

static void
testCallbackEx(vscpEventEx & /*ex*/, void * /*pobj*/)
{
  g_exCallbackFired = true;
}

TEST(VscpClientUdp, SetCallbackEv)
{
  vscpClientUdp client;
  g_evCallbackFired = false;
  int rv            = client.setCallbackEv(testCallbackEv, nullptr);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
}

TEST(VscpClientUdp, SetCallbackEx)
{
  vscpClientUdp client;
  g_exCallbackFired = false;
  int rv            = client.setCallbackEx(testCallbackEx, nullptr);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
}

// ---------------------------------------------------------------------------
//                      Default filter state
// ---------------------------------------------------------------------------

TEST(VscpClientUdp, DefaultFilterCleared)
{
  vscpClientUdp client;

  EXPECT_EQ(0, client.m_filter.filter_priority);
  EXPECT_EQ(0, client.m_filter.filter_class);
  EXPECT_EQ(0, client.m_filter.filter_type);
  EXPECT_EQ(0, client.m_filter.mask_priority);
  EXPECT_EQ(0, client.m_filter.mask_class);
  EXPECT_EQ(0, client.m_filter.mask_type);
}

// ---------------------------------------------------------------------------
//                      Encryption defaults
// ---------------------------------------------------------------------------

TEST(VscpClientUdp, DefaultEncryptionNone)
{
  vscpClientUdp client;
  EXPECT_EQ(VSCP_ENCRYPTION_NONE, client.m_encryptType);
  EXPECT_FALSE(client.m_bEncrypt);
}

TEST(VscpClientUdp, SetEncryptionType)
{
  vscpClientUdp client;
  client.m_encryptType = VSCP_ENCRYPTION_AES256;
  EXPECT_EQ(VSCP_ENCRYPTION_AES256, client.m_encryptType);
}

// ---------------------------------------------------------------------------
//                    Disconnect when not connected
// ---------------------------------------------------------------------------

TEST(VscpClientUdp, DisconnectClosesSocket)
{
  vscpClientUdp client;
  // Socket is 0 (not connected), disconnect should still return success
  int rv = client.disconnect();
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
  EXPECT_FALSE(client.isConnected());
}
