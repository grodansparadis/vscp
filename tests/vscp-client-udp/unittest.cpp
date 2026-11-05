// unittest.cpp
//
// Unit tests for vscpClientUdp
//
// Tests include local configuration and loopback communication. Scoped
// link-local exchange additionally requires the isolated test interfaces.
//

#include <gtest/gtest.h>

#include <pthread.h>
#include <semaphore.h>
#include <vscp-client-udp.h>
#include <vscphelper.h>

#include <nlohmann/json.hpp>
#include <atomic>
#include <cstdlib>
#include <string>
#include <vector>

#include "../datagram_test_helpers.h"
#include "../datagram_authenticated_test_helpers.h"

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

TEST(VscpClientUdp, JsonOptionsAreValidatedAndTransactional)
{
  verifyDatagramJsonOptions<vscpClientUdp>("[::1]:40000");
}

TEST(VscpClientUdp, AuthenticatedJsonOptions)
{
  verifyAuthenticatedDatagramOptions<vscpClientUdp>("127.0.0.1:40000");
}

TEST(VscpClientUdp, AuthenticatedReceiveRejectsTampering)
{
  for (uint8_t encryption = VSCP_ENCRYPTION_AES128; encryption <= VSCP_ENCRYPTION_AES256; ++encryption) {
    SCOPED_TRACE(encryption);
    vscpClientUdp client;
    const uint16_t port = unusedDatagramPort(AF_INET);
    ASSERT_NE(port, 0);
    json config = authenticatedDatagramConfig(encryption);
    config["ip"] = "127.0.0.1:" + std::to_string(port);
    ASSERT_TRUE(client.initFromJson(config.dump()));
    verifyAuthenticatedDatagramReceive(client, port, encryption, false);
  }
}

#ifdef __linux__
TEST(VscpClientUdp, AuthenticatedSendWireFormat)
{
  for (uint8_t encryption = VSCP_ENCRYPTION_AES128; encryption <= VSCP_ENCRYPTION_AES256; ++encryption) {
    SCOPED_TRACE(encryption);
    vscpClientUdp client;
    const uint16_t port = unusedDatagramPort(AF_INET);
    ASSERT_NE(port, 0);
    AuthenticatedDatagramWireReceiver receiver(port, false);
    ASSERT_TRUE(receiver.valid());
    json config = authenticatedDatagramConfig(encryption);
    config["ip"] = "127.0.0.1:" + std::to_string(port);
    ASSERT_TRUE(client.initFromJson(config.dump()));
    ASSERT_EQ(VSCP_ERROR_SUCCESS, client.connect());
    verifyAuthenticatedDatagramSend(client, receiver, encryption);
  }
}
#endif

TEST(VscpClientUdp, AuthenticatedIpv6Loopback)
{
  vscpClientUdp client;
  const uint16_t port = unusedDatagramPort(AF_INET6);
  if (!port) {
    GTEST_SKIP() << "IPv6 loopback is not available";
  }
  json config = authenticatedDatagramConfig(VSCP_ENCRYPTION_AES256);
  config["ip"] = "[::1]:" + std::to_string(port);
  ASSERT_TRUE(client.initFromJson(config.dump()));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.connect());
  vscpEventEx event{};
  event.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS;
  event.vscp_class = 42;
  event.vscp_type = 8;
  event.sizeData = 2;
  event.data[0] = 0xa5;
  event.data[1] = 0x5a;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.send(event));
  vscpEventEx received{};
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.receiveBlocking(received, 2000));
  EXPECT_EQ(event.vscp_class, received.vscp_class);
  EXPECT_EQ(event.vscp_type, received.vscp_type);
  ASSERT_EQ(event.sizeData, received.sizeData);
  EXPECT_EQ(0, std::memcmp(event.data, received.data, event.sizeData));
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

TEST(VscpClientUdp, Ipv6EndpointAndPortRoundtrip)
{
  const std::vector<std::pair<std::string, std::string>> endpoints = {
    {"::1", "udp://[::1]:33333"},
    {"udp://[2001:db8::1]:65535", "udp://[2001:db8::1]:65535"},
    {"[fe80::1234%eth0]:40000", "udp://[fe80::1234%25eth0]:40000"},
    {"udp://[fe80::1234%251]:54321", "udp://[fe80::1234%251]:54321"}
  };
  for (const auto &entry : endpoints) {
    vscpClientUdp client;
    ASSERT_TRUE(client.initFromJson(json{{"ip", entry.first}}.dump())) << entry.first;
    EXPECT_EQ(entry.second, json::parse(client.getConfigAsJson())["ip"]);
    vscpClientUdp restored;
    ASSERT_TRUE(restored.initFromJson(client.getConfigAsJson()));
    EXPECT_EQ(entry.second, json::parse(restored.getConfigAsJson())["ip"]);
  }
  vscpClientUdp client;
  EXPECT_EQ("udp://255.255.255.255:33333", json::parse(client.getConfigAsJson())["ip"]);
  ASSERT_TRUE(client.initFromJson(R"({"ip":"::1","port":65535})"));
  EXPECT_EQ("udp://[::1]:65535", json::parse(client.getConfigAsJson())["ip"]);
}

TEST(VscpClientUdp, RejectsInvalidDatagramEndpointsAndPorts)
{
  const std::vector<std::string> endpoints = {
    "", "udp://[::1", "udp://[::1]junk", "[localhost]:1234", "[::1]:",
    "[::1]:0", "[::1]:-1", "[::1]:65536", "127.0.0.1:bad",
    "999.1.1.1:1234", "::1%", "udp://[::1%25]:1234", ":9598"
  };
  for (const auto &endpoint : endpoints) {
    vscpClientUdp client;
    EXPECT_FALSE(client.initFromJson(json{{"ip", endpoint}}.dump())) << endpoint;
    EXPECT_EQ("udp://255.255.255.255:33333", json::parse(client.getConfigAsJson())["ip"]);
    EXPECT_FALSE(client.isConnected());
  }
  for (const json &port : {json(0), json(-1), json(65536), json(1.5), json("1234")}) {
    vscpClientUdp client;
    EXPECT_FALSE(client.initFromJson(json{{"ip", "::1"}, {"port", port}}.dump())) << port;
  }
  for (const json &encryption : {json(-1), json(7), json(15), json(1.5), json("1")}) {
    vscpClientUdp client;
    EXPECT_FALSE(client.initFromJson(json{{"encryption", encryption}}.dump())) << encryption;
  }
}

static void
exerciseUdpLoopback(int family)
{
  vscpClientUdp client;
  const uint16_t port = unusedDatagramPort(family);
  if (!port && family == AF_INET6) {
    GTEST_SKIP() << "IPv6 loopback is not available";
  }
  ASSERT_NE(port, 0);
  const std::string address = family == AF_INET6 ? "[::1]" : "127.0.0.1";
  ASSERT_TRUE(client.initFromJson(json{{"ip", address + ":" + std::to_string(port)},
                                       {"encryption", VSCP_ENCRYPTION_AES128},
                                       {"key", "0102030405060708090A0B0C0D0E0F10"}}.dump()));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.connect());
  EXPECT_FALSE(client.initFromJson(R"({"ip":"127.0.0.1:1234"})"));

  uint8_t bytes[] = {0x12, 0x34, 0x56};
  vscpEvent event{};
  event.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS;
  event.timestamp_ns = 123456789;
  event.vscp_class = 10;
  event.vscp_type = 6;
  event.sizeData = sizeof(bytes);
  event.pdata = bytes;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.send(event));
  vscpEvent received{};
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.receiveBlocking(received, 2000));
  EXPECT_EQ(event.vscp_class, received.vscp_class);
  EXPECT_EQ(event.vscp_type, received.vscp_type);
  ASSERT_EQ(sizeof(bytes), received.sizeData);
  EXPECT_EQ(0, memcmp(bytes, received.pdata, sizeof(bytes)));
  vscp_deleteEvent(&received);

  vscpEventEx extended{};
  extended.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS;
  extended.vscp_class = 20;
  extended.vscp_type = 3;
  extended.sizeData = 2;
  extended.data[0] = 0xa5;
  extended.data[1] = 0x5a;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.send(extended));
  ASSERT_TRUE(waitForDatagramCount(client, 1));
  vscpEventEx receivedEx{};
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.receive(receivedEx));
  EXPECT_EQ(extended.vscp_class, receivedEx.vscp_class);
  EXPECT_EQ(extended.sizeData, receivedEx.sizeData);
  EXPECT_EQ(extended.data[0], receivedEx.data[0]);
  EXPECT_EQ(extended.data[1], receivedEx.data[1]);
  EXPECT_EQ(VSCP_ERROR_TIMEOUT, client.receiveBlocking(receivedEx, 10));

  client.m_encryptType = VSCP_ENCRYPTION_NONE;
  canalMsg message{};
  message.id = (10U << 16) | (6U << 8) | 42U;
  message.sizeData = 2;
  message.data[0] = 0x77;
  message.data[1] = 0x88;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.send(message));
  canalMsg receivedCanal{};
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.receiveBlocking(receivedCanal, 5000));
  EXPECT_EQ(message.id, receivedCanal.id);
  EXPECT_EQ(message.sizeData, receivedCanal.sizeData);
  EXPECT_EQ(message.data[0], receivedCanal.data[0]);
  EXPECT_EQ(message.data[1], receivedCanal.data[1]);

  const uint8_t shortFrame[] = {0x10};
  const uint8_t shortEncryptedFrame[] = {0x11, 0x00};
  ASSERT_TRUE(sendRawLoopbackDatagram(family, port, shortFrame, sizeof(shortFrame)));
  ASSERT_TRUE(sendRawLoopbackDatagram(family, port, shortEncryptedFrame, sizeof(shortEncryptedFrame)));
  std::vector<uint8_t> oversized(1100, 0);
  ASSERT_TRUE(vscp_writeEventToFrame(oversized.data(), oversized.size(), 0, &event));
  ASSERT_TRUE(sendRawLoopbackDatagram(family, port, oversized.data(), oversized.size()));
  EXPECT_EQ(VSCP_ERROR_TIMEOUT, client.receiveBlocking(receivedEx, 100));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.send(event));
  ASSERT_TRUE(waitForDatagramCount(client, 1));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.clear());
  EXPECT_EQ(VSCP_ERROR_TIMEOUT, client.receiveBlocking(receivedEx, 10));

  extended.sizeData = VSCP_LEVEL2_MAXDATA + 1;
  EXPECT_EQ(VSCP_ERROR_INVALID_FRAME, client.send(extended));
  client.m_encryptType = 7;
  EXPECT_EQ(VSCP_ERROR_INVALID_FRAME, client.send(event));
  client.m_encryptType = VSCP_ENCRYPTION_NONE;

  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.disconnect());
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, client.send(event));
  extended.sizeData = 0;
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, client.send(extended));
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, client.send(message));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.connect());
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.send(event));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.receiveBlocking(receivedEx, 2000));
  EXPECT_EQ(event.vscp_class, receivedEx.vscp_class);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, client.disconnect());
}

TEST(VscpClientUdp, Ipv4LoopbackExchange)
{
  exerciseUdpLoopback(AF_INET);
}

TEST(VscpClientUdp, Ipv6LoopbackExchange)
{
  exerciseUdpLoopback(AF_INET6);
}

TEST(VscpClientUdp, Ipv4AndIpv6CanBindTheSamePort)
{
  vscpClientUdp ipv4;
  vscpClientUdp ipv6;
  const uint16_t port = unusedDatagramPort(AF_INET6);
  if (!port) {
    GTEST_SKIP() << "IPv6 loopback is not available";
  }
  ASSERT_TRUE(ipv4.initFromJson(json{{"ip", "127.0.0.1:" + std::to_string(port)}}.dump()));
  ASSERT_TRUE(ipv6.initFromJson(json{{"ip", "[::1]:" + std::to_string(port)}}.dump()));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, ipv4.connect());
  ASSERT_EQ(VSCP_ERROR_SUCCESS, ipv6.connect());
  vscpEvent event{};
  event.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS;
  event.vscp_class = 42;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, ipv4.send(event));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, ipv6.send(event));
  vscpEventEx received{};
  EXPECT_EQ(VSCP_ERROR_SUCCESS, ipv4.receiveBlocking(received, 2000));
  EXPECT_EQ(VSCP_ERROR_SUCCESS, ipv6.receiveBlocking(received, 2000));
}

TEST(VscpClientUdp, Ipv4MappedIpv6LoopbackExchange)
{
  vscpClientUdp client;
  const uint16_t port = unusedDatagramPort(AF_INET6);
  if (!port) {
    GTEST_SKIP() << "IPv6 loopback is not available";
  }
  ASSERT_TRUE(client.initFromJson(json{{"ip", "[::ffff:127.0.0.1]:" + std::to_string(port)}}.dump()));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.connect());
  vscpEvent event{};
  event.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS;
  event.vscp_class = 53;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.send(event));
  vscpEventEx received{};
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.receiveBlocking(received, 2000));
  EXPECT_EQ(event.vscp_class, received.vscp_class);
}

TEST(VscpClientUdp, ConcurrentDisconnectWithReceiveCallback)
{
  vscpClientUdp client;
  const uint16_t port = unusedDatagramPort(AF_INET);
  ASSERT_NE(port, 0);
  ASSERT_TRUE(client.initFromJson(json{{"ip", "127.0.0.1:" + std::to_string(port)}}.dump()));
  std::atomic<bool> inCallback{false};
  std::atomic<bool> proceed{false};
  std::atomic<int> callbackResult{-999};
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.setCallbackEv([&](vscpEvent &, void *) {
    inCallback.store(true);
    while (!proceed.load()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    callbackResult.store(client.disconnect());
  }));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.connect());
  vscpEvent event{};
  event.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.send(event));
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
  while (!inCallback.load() && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  if (!inCallback.load()) {
    proceed.store(true);
    FAIL() << "Receive callback did not run";
  }
  std::atomic<int> disconnectResult{-999};
  std::thread stopper([&] { disconnectResult.store(client.disconnect()); });
  const auto stopDeadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
  while (client.isConnected() && std::chrono::steady_clock::now() < stopDeadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  const bool stopped = !client.isConnected();
  proceed.store(true);
  stopper.join();
  EXPECT_TRUE(stopped);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, disconnectResult.load());
  EXPECT_EQ(VSCP_ERROR_SUCCESS, callbackResult.load());
}

#ifndef WIN32
TEST(VscpClientUdp, Ipv6ScopedInterfaceAndDisconnectFromCallback)
{
#ifdef __APPLE__
  const char *loopback = "lo0";
#else
  const char *loopback = "lo";
#endif
  const unsigned int index = if_nametoindex(loopback);
  vscpClientUdp client;
  const uint16_t port = unusedDatagramPort(AF_INET6);
  if (!index || !port) {
    GTEST_SKIP() << "IPv6 loopback interface is not available";
  }
  ASSERT_TRUE(client.initFromJson(json{{"ip", "[::1%" + std::string(loopback) + "]:" + std::to_string(port)},
                                       {"interface", std::to_string(index)}}.dump()));
  std::atomic<int> callbackResult{-999};
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.setCallbackEv([&](vscpEvent &, void *) {
    callbackResult.store(client.disconnect());
  }));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.connect());
  vscpEvent event{};
  event.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.send(event));
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
  while (callbackResult.load() == -999 && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }
  EXPECT_EQ(VSCP_ERROR_SUCCESS, callbackResult.load());
  EXPECT_EQ(VSCP_ERROR_SUCCESS, client.disconnect());
  EXPECT_FALSE(client.isConnected());
}
#endif

#ifdef __linux__
static void
exerciseIpv6ExchangeOnSeparateInterfaces(const char *txInterface, const char *rxInterface,
                                         const char *txAddress, const char *rxAddress, bool scoped)
{
  const unsigned int txIndex = if_nametoindex(txInterface);
  const unsigned int rxIndex = if_nametoindex(rxInterface);
  ASSERT_NE(txIndex, 0u);
  ASSERT_NE(rxIndex, 0u);
  vscpClientUdp client;
  const uint16_t port = unusedDatagramPort(AF_INET6);
  ASSERT_NE(port, 0);
  const std::string destination =
    "[" + std::string(rxAddress) + (scoped ? "%" + std::string(txInterface) : "") +
    "]:" + std::to_string(port);
  ASSERT_TRUE(client.initFromJson(json{{"ip", destination}, {"interface", txInterface}}.dump()));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.connect());

  TestSocket peer = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
  ASSERT_NE(peer, invalidTestSocket);
  const int reuse = 1;
  ASSERT_EQ(0, setsockopt(peer, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)));
  timeval timeout{2, 0};
  ASSERT_EQ(0, setsockopt(peer, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)));
  sockaddr_in6 peerAddress{};
  peerAddress.sin6_family = AF_INET6;
  peerAddress.sin6_port = htons(port);
  peerAddress.sin6_scope_id = rxIndex;
  ASSERT_EQ(1, inet_pton(AF_INET6, rxAddress, &peerAddress.sin6_addr));
  ASSERT_EQ(0, bind(peer, reinterpret_cast<sockaddr *>(&peerAddress), sizeof(peerAddress)));

  uint8_t bytes[] = {0x63, 0x64};
  vscpEvent event{};
  event.head = VSCP_HEADER16_FRAME_VERSION_UNIX_NS;
  event.vscp_class = 75;
  event.sizeData = sizeof(bytes);
  event.pdata = bytes;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.send(event));
  uint8_t frame[1024]{};
  const int receivedLength = static_cast<int>(recv(peer, frame, sizeof(frame), 0));
  ASSERT_GT(receivedLength, 0);
  vscpEvent received{};
  ASSERT_TRUE(vscp_getEventFromFrame(&received, frame, receivedLength));
  EXPECT_EQ(event.vscp_class, received.vscp_class);
  ASSERT_EQ(sizeof(bytes), received.sizeData);
  EXPECT_EQ(0, memcmp(bytes, received.pdata, sizeof(bytes)));
  vscp_deleteEvent(&received);
  closeTestSocket(peer);

  peer = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
  ASSERT_NE(peer, invalidTestSocket);
  if (!scoped) {
    const unsigned int boundInterface = htonl(rxIndex);
    ASSERT_EQ(0, setsockopt(peer, IPPROTO_IPV6, IPV6_UNICAST_IF, &boundInterface, sizeof(boundInterface)));
  }
  peerAddress.sin6_port = 0;
  ASSERT_EQ(0, bind(peer, reinterpret_cast<sockaddr *>(&peerAddress), sizeof(peerAddress)));
  sockaddr_in6 target{};
  target.sin6_family = AF_INET6;
  target.sin6_port = htons(port);
  target.sin6_scope_id = rxIndex;
  ASSERT_EQ(1, inet_pton(AF_INET6, txAddress, &target.sin6_addr));
  ASSERT_TRUE(vscp_writeEventToFrame(frame, sizeof(frame), 0, &event));
  const size_t frameLength = vscp_getFrameSizeFromEvent(&event);
  ASSERT_EQ(static_cast<ssize_t>(frameLength),
            sendto(peer, frame, frameLength, 0, reinterpret_cast<sockaddr *>(&target), sizeof(target)));
  closeTestSocket(peer);
  vscpEventEx receivedEx{};
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.receiveBlocking(receivedEx, 2000));
  EXPECT_EQ(event.vscp_class, receivedEx.vscp_class);
  EXPECT_EQ(event.sizeData, receivedEx.sizeData);
  EXPECT_EQ(bytes[0], receivedEx.data[0]);
  EXPECT_EQ(bytes[1], receivedEx.data[1]);
}

TEST(VscpClientUdp, Ipv6LinkLocalExchangeOnSeparateInterfaces)
{
  const char *txInterface = std::getenv("VSCP_TEST_MULTICAST_TX_INTERFACE");
  const char *rxInterface = std::getenv("VSCP_TEST_MULTICAST_RX_INTERFACE");
  const char *txAddress = std::getenv("VSCP_TEST_LINKLOCAL_ADDRESS_A");
  const char *rxAddress = std::getenv("VSCP_TEST_LINKLOCAL_ADDRESS_B");
  if (!txInterface || !rxInterface || !txAddress || !rxAddress) {
    GTEST_SKIP() << "Set the isolated interface and link-local address test variables";
  }
  exerciseIpv6ExchangeOnSeparateInterfaces(txInterface, rxInterface, txAddress, rxAddress, true);
}

TEST(VscpClientUdp, Ipv6GlobalExchangeOnSeparateInterfaces)
{
  const char *txInterface = std::getenv("VSCP_TEST_MULTICAST_TX_INTERFACE");
  const char *rxInterface = std::getenv("VSCP_TEST_MULTICAST_RX_INTERFACE");
  const char *txAddress = std::getenv("VSCP_TEST_GLOBAL_ADDRESS_A");
  const char *rxAddress = std::getenv("VSCP_TEST_GLOBAL_ADDRESS_B");
  if (!txInterface || !rxInterface || !txAddress || !rxAddress) {
    GTEST_SKIP() << "Set the isolated interface and global address test variables";
  }
  exerciseIpv6ExchangeOnSeparateInterfaces(txInterface, rxInterface, txAddress, rxAddress, false);
}
#endif
