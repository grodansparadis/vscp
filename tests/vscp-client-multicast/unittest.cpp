// unittest.cpp
//
// Unit tests for vscpClientMulticast
//
// These tests exercise the vscpClientMulticast class without requiring
// actual multicast network communication. Constructor defaults, JSON
// config serialization/parsing, getters/setters, filter, receive queue,
// version, capabilities, and callbacks are all tested locally.
//

#include <gtest/gtest.h>

#include <semaphore.h>
#include <vscp-client-multicast.h>
#include <vscphelper.h>

#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

// ---------------------------------------------------------------------------
//                       Constructor / Default State
// ---------------------------------------------------------------------------

TEST(VscpClientMulticast, ConstructorDefaults)
{
  vscpClientMulticast client;

  EXPECT_EQ(client.getType(), CVscpClient::connType::MULTICAST);
  EXPECT_FALSE(client.isConnected());
  EXPECT_TRUE(client.m_bRun);
  EXPECT_FALSE(client.m_bEncrypt);
  EXPECT_EQ(client.m_encryptType, VSCP_ENCRYPTION_NONE);
  EXPECT_EQ(vscpFramePaddingMode::PKCS7, client.m_paddingMode);
}

TEST(VscpClientMulticast, ConstructorNotConnected)
{
  vscpClientMulticast client;
  EXPECT_FALSE(client.isConnected());
}

TEST(VscpClientMulticast, ReceiveQueueInitiallyEmpty)
{
  vscpClientMulticast client;
  EXPECT_TRUE(client.m_receiveQueue.empty());

  uint16_t count = 999;
  EXPECT_EQ(client.getcount(&count), VSCP_ERROR_SUCCESS);
  EXPECT_EQ(count, 0);
}

// ---------------------------------------------------------------------------
//                            getversion
// ---------------------------------------------------------------------------

TEST(VscpClientMulticast, GetVersion)
{
  vscpClientMulticast client;
  uint8_t major = 0, minor = 0, release = 0, build = 0;

  EXPECT_EQ(client.getversion(&major, &minor, &release, &build), VSCP_ERROR_SUCCESS);
  EXPECT_EQ(major, 1);
  EXPECT_EQ(minor, 0);
  EXPECT_EQ(release, 0);
  EXPECT_EQ(build, 0);
}

TEST(VscpClientMulticast, GetVersionNullPointers)
{
  vscpClientMulticast client;
  uint8_t v = 0;

  EXPECT_EQ(client.getversion(nullptr, &v, &v, &v), VSCP_ERROR_INVALID_POINTER);
  EXPECT_EQ(client.getversion(&v, nullptr, &v, &v), VSCP_ERROR_INVALID_POINTER);
  EXPECT_EQ(client.getversion(&v, &v, nullptr, &v), VSCP_ERROR_INVALID_POINTER);
  EXPECT_EQ(client.getversion(&v, &v, &v, nullptr), VSCP_ERROR_INVALID_POINTER);
}

// ---------------------------------------------------------------------------
//                           getinterfaces
// ---------------------------------------------------------------------------

TEST(VscpClientMulticast, GetInterfaces)
{
  vscpClientMulticast client;
  std::deque<std::string> iflist;
  iflist.push_back("dummy");
  EXPECT_EQ(client.getinterfaces(iflist), VSCP_ERROR_SUCCESS);
  EXPECT_TRUE(iflist.empty());
}

// ---------------------------------------------------------------------------
//                             getwcyd
// ---------------------------------------------------------------------------

TEST(VscpClientMulticast, GetWcyd)
{
  vscpClientMulticast client;
  uint64_t wcyd = 0xFFFF;
  EXPECT_EQ(client.getwcyd(wcyd), VSCP_ERROR_SUCCESS);
  EXPECT_EQ(wcyd, (uint64_t)0);
}

// ---------------------------------------------------------------------------
//                       getcount / clear
// ---------------------------------------------------------------------------

TEST(VscpClientMulticast, GetcountNullPointer)
{
  vscpClientMulticast client;
  EXPECT_EQ(client.getcount(nullptr), VSCP_ERROR_INVALID_POINTER);
}

TEST(VscpClientMulticast, ClearEmptyQueue)
{
  vscpClientMulticast client;
  EXPECT_EQ(client.clear(), VSCP_ERROR_SUCCESS);

  uint16_t count = 999;
  EXPECT_EQ(client.getcount(&count), VSCP_ERROR_SUCCESS);
  EXPECT_EQ(count, 0);
}

// ---------------------------------------------------------------------------
//                     receive on empty queue
// ---------------------------------------------------------------------------

TEST(VscpClientMulticast, ReceiveEmptyQueueEvent)
{
  vscpClientMulticast client;
  vscpEvent ev;
  memset(&ev, 0, sizeof(ev));
  EXPECT_EQ(client.receive(ev), VSCP_ERROR_FIFO_EMPTY);
}

TEST(VscpClientMulticast, ReceiveEmptyQueueEventEx)
{
  vscpClientMulticast client;
  vscpEventEx ex;
  memset(&ex, 0, sizeof(ex));
  EXPECT_EQ(client.receive(ex), VSCP_ERROR_FIFO_EMPTY);
}

TEST(VscpClientMulticast, ReceiveEmptyQueueCanal)
{
  vscpClientMulticast client;
  canalMsg msg;
  memset(&msg, 0, sizeof(msg));
  EXPECT_EQ(client.receive(msg), VSCP_ERROR_FIFO_EMPTY);
}

// ---------------------------------------------------------------------------
//                receiveBlocking on empty queue (timeout)
// ---------------------------------------------------------------------------

TEST(VscpClientMulticast, ReceiveBlockingTimeout)
{
  vscpClientMulticast client;
  vscpEvent ev;
  memset(&ev, 0, sizeof(ev));
  EXPECT_EQ(client.receiveBlocking(ev, 10), VSCP_ERROR_TIMEOUT);
}

TEST(VscpClientMulticast, ReceiveBlockingExTimeout)
{
  vscpClientMulticast client;
  vscpEventEx ex;
  memset(&ex, 0, sizeof(ex));
  EXPECT_EQ(client.receiveBlocking(ex, 10), VSCP_ERROR_TIMEOUT);
}

TEST(VscpClientMulticast, ReceiveBlockingCanalTimeout)
{
  vscpClientMulticast client;
  canalMsg msg;
  memset(&msg, 0, sizeof(msg));
  EXPECT_EQ(client.receiveBlocking(msg, 10), VSCP_ERROR_TIMEOUT);
}

// ---------------------------------------------------------------------------
//                    send when not connected
// ---------------------------------------------------------------------------

TEST(VscpClientMulticast, SendEventNotConnected)
{
  vscpClientMulticast client;
  vscpEvent ev;
  memset(&ev, 0, sizeof(ev));
  ev.vscp_class = 10;
  ev.vscp_type  = 6;
  EXPECT_EQ(client.send(ev), VSCP_ERROR_NOT_CONNECTED);
}

// ---------------------------------------------------------------------------
//                             Filter
// ---------------------------------------------------------------------------

TEST(VscpClientMulticast, SetFilter)
{
  vscpClientMulticast client;
  vscpEventFilter filter;
  memset(&filter, 0, sizeof(vscpEventFilter));

  filter.filter_priority = 3;
  filter.mask_priority   = 7;
  filter.filter_class    = 10;
  filter.mask_class      = 0xFFFF;
  filter.filter_type     = 6;
  filter.mask_type       = 0xFFFF;
  filter.filter_GUID[0]  = 0xAA;
  filter.mask_GUID[0]    = 0xFF;

  EXPECT_EQ(client.setfilter(filter), VSCP_ERROR_SUCCESS);

  // Verify via the public m_filter member
  EXPECT_EQ(client.m_filter.filter_priority, 3);
  EXPECT_EQ(client.m_filter.mask_priority, 7);
  EXPECT_EQ(client.m_filter.filter_class, 10);
  EXPECT_EQ(client.m_filter.mask_class, 0xFFFF);
  EXPECT_EQ(client.m_filter.filter_type, 6);
  EXPECT_EQ(client.m_filter.mask_type, 0xFFFF);
  EXPECT_EQ(client.m_filter.filter_GUID[0], 0xAA);
  EXPECT_EQ(client.m_filter.mask_GUID[0], 0xFF);
}

// ---------------------------------------------------------------------------
//                      Timeout getters/setters
// ---------------------------------------------------------------------------

TEST(VscpClientMulticast, ConnectionTimeout)
{
  vscpClientMulticast client;
  // Not implemented for multicast - returns 0
  EXPECT_EQ(client.getConnectionTimeout(), (uint32_t)0);
  client.setConnectionTimeout(5000);
  EXPECT_EQ(client.getConnectionTimeout(), (uint32_t)0);
}

TEST(VscpClientMulticast, ResponseTimeout)
{
  vscpClientMulticast client;
  EXPECT_EQ(client.getResponseTimeout(), (uint32_t)0);
  client.setResponseTimeout(500);
  EXPECT_EQ(client.getResponseTimeout(), (uint32_t)0);
}

// ---------------------------------------------------------------------------
//                       connType enum value
// ---------------------------------------------------------------------------

TEST(VscpClientMulticast, ConnTypeIsMulticast)
{
  vscpClientMulticast client;
  EXPECT_EQ(client.getType(), CVscpClient::connType::MULTICAST);
}

// ---------------------------------------------------------------------------
//                       Name getter/setter
// ---------------------------------------------------------------------------

TEST(VscpClientMulticast, SetGetName)
{
  vscpClientMulticast client;
  std::string name = "Multicast Test";
  client.setName(name);
  EXPECT_EQ(client.getName(), "Multicast Test");
}

// ---------------------------------------------------------------------------
//                       FullLevel2 flag
// ---------------------------------------------------------------------------

TEST(VscpClientMulticast, FullLevel2Flag)
{
  vscpClientMulticast client;
  client.setFullLevel2(true);
  EXPECT_TRUE(client.isFullLevel2());
  client.setFullLevel2(false);
  EXPECT_FALSE(client.isFullLevel2());
}

// ---------------------------------------------------------------------------
//                      getConfigAsJson
// ---------------------------------------------------------------------------

TEST(VscpClientMulticast, GetConfigAsJsonDefault)
{
  vscpClientMulticast client;
  std::string configStr = client.getConfigAsJson();
  EXPECT_FALSE(configStr.empty());

  auto j = json::parse(configStr);
  EXPECT_TRUE(j.contains("ip"));
  EXPECT_TRUE(j.contains("ttl"));
  EXPECT_TRUE(j.contains("encryption"));
  EXPECT_TRUE(j.contains("key"));
  EXPECT_TRUE(j.contains("padding"));
  EXPECT_TRUE(j.contains("interface"));

  // Default multicast group
  std::string ip = j["ip"].get<std::string>();
  EXPECT_NE(ip.find("224.0.23.158"), std::string::npos);

  // Default TTL
  EXPECT_EQ(j["ttl"].get<int>(), 1);

  // Default no encryption
  EXPECT_EQ(j["encryption"].get<int>(), VSCP_ENCRYPTION_NONE);
  EXPECT_EQ("pkcs7", j["padding"].get<std::string>());

  // Default empty key for no encryption
  EXPECT_EQ(j["key"].get<std::string>(), "");

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

TEST(VscpClientMulticast, GetConfigAsJsonEncryptionAES128)
{
  vscpClientMulticast client;
  client.m_encryptType = VSCP_ENCRYPTION_AES128;
  client.m_bEncrypt    = true;
  memset(client.m_key, 0xAB, 16);

  std::string configStr = client.getConfigAsJson();
  auto j                = json::parse(configStr);

  EXPECT_EQ(j["encryption"].get<int>(), VSCP_ENCRYPTION_AES128);
  std::string key = j["key"].get<std::string>();
  EXPECT_FALSE(key.empty());
  // Key should be hex string of 16 bytes = 32 hex chars
  EXPECT_EQ(key.length(), (size_t)32);
}

TEST(VscpClientMulticast, GetConfigAsJsonEncryptionAES192)
{
  vscpClientMulticast client;
  client.m_encryptType = VSCP_ENCRYPTION_AES192;
  client.m_bEncrypt    = true;
  memset(client.m_key, 0xCD, 24);

  std::string configStr = client.getConfigAsJson();
  auto j                = json::parse(configStr);

  EXPECT_EQ(j["encryption"].get<int>(), VSCP_ENCRYPTION_AES192);
  std::string key = j["key"].get<std::string>();
  // Key should be hex string of 24 bytes = 48 hex chars
  EXPECT_EQ(key.length(), (size_t)48);
}

TEST(VscpClientMulticast, GetConfigAsJsonEncryptionAES256)
{
  vscpClientMulticast client;
  client.m_encryptType = VSCP_ENCRYPTION_AES256;
  client.m_bEncrypt    = true;
  memset(client.m_key, 0xEF, 32);

  std::string configStr = client.getConfigAsJson();
  auto j                = json::parse(configStr);

  EXPECT_EQ(j["encryption"].get<int>(), VSCP_ENCRYPTION_AES256);
  std::string key = j["key"].get<std::string>();
  // Key should be hex string of 32 bytes = 64 hex chars
  EXPECT_EQ(key.length(), (size_t)64);
}

// ---------------------------------------------------------------------------
//                      initFromJson
// ---------------------------------------------------------------------------

TEST(VscpClientMulticast, InitFromJsonBasic)
{
  vscpClientMulticast client;

  json j;
  j["interface"]       = "eth0";
  j["ip"]              = "udp://239.1.2.3:5555";
  j["ttl"]             = 5;
  j["encryption"]      = 0;
  j["padding"]         = "zero";
  j["key"]             = "";
  j["priority-filter"] = 0;
  j["priority-mask"]   = 0;
  j["class-filter"]    = 0;
  j["class-mask"]      = 0;
  j["type-filter"]     = 0;
  j["type-mask"]       = 0;
  j["guid-filter"]     = "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00";
  j["guid-mask"]       = "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00";

  EXPECT_TRUE(client.initFromJson(j.dump()));
  EXPECT_EQ(vscpFramePaddingMode::ZERO, client.m_paddingMode);
  EXPECT_EQ("zero", json::parse(client.getConfigAsJson())["padding"].get<std::string>());
}

TEST(VscpClientMulticast, InitFromJsonRejectsUnknownPadding)
{
  vscpClientMulticast client;
  EXPECT_FALSE(client.initFromJson(R"({"padding":"auto"})"));
}

TEST(VscpClientMulticast, InitFromJsonIpWithPort)
{
  vscpClientMulticast client;

  json j;
  j["ip"]              = "239.1.2.3:7777";
  j["encryption"]      = 0;
  j["key"]             = "";
  j["priority-filter"] = 0;
  j["priority-mask"]   = 0;
  j["class-filter"]    = 0;
  j["class-mask"]      = 0;
  j["type-filter"]     = 0;
  j["type-mask"]       = 0;
  j["guid-filter"]     = "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00";
  j["guid-mask"]       = "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00";

  EXPECT_TRUE(client.initFromJson(j.dump()));
}

TEST(VscpClientMulticast, InitFromJsonPortOverride)
{
  vscpClientMulticast client;

  json j;
  j["ip"]              = "udp://239.1.2.3:5555";
  j["port"]            = 9999;
  j["encryption"]      = 0;
  j["key"]             = "";
  j["priority-filter"] = 0;
  j["priority-mask"]   = 0;
  j["class-filter"]    = 0;
  j["class-mask"]      = 0;
  j["type-filter"]     = 0;
  j["type-mask"]       = 0;
  j["guid-filter"]     = "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00";
  j["guid-mask"]       = "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00";

  EXPECT_TRUE(client.initFromJson(j.dump()));
}

TEST(VscpClientMulticast, InitFromJsonTtl)
{
  vscpClientMulticast client;

  json j;
  j["ttl"]             = 10;
  j["encryption"]      = 0;
  j["key"]             = "";
  j["priority-filter"] = 0;
  j["priority-mask"]   = 0;
  j["class-filter"]    = 0;
  j["class-mask"]      = 0;
  j["type-filter"]     = 0;
  j["type-mask"]       = 0;
  j["guid-filter"]     = "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00";
  j["guid-mask"]       = "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00";

  EXPECT_TRUE(client.initFromJson(j.dump()));
}

TEST(VscpClientMulticast, InitFromJsonInvalidJson)
{
  vscpClientMulticast client;
  EXPECT_FALSE(client.initFromJson("not valid json {{{"));
}

TEST(VscpClientMulticast, InitFromJsonEncryptionAES128)
{
  vscpClientMulticast client;

  json j;
  j["encryption"]      = VSCP_ENCRYPTION_AES128;
  j["key"]             = "0102030405060708090A0B0C0D0E0F10";
  j["priority-filter"] = 0;
  j["priority-mask"]   = 0;
  j["class-filter"]    = 0;
  j["class-mask"]      = 0;
  j["type-filter"]     = 0;
  j["type-mask"]       = 0;
  j["guid-filter"]     = "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00";
  j["guid-mask"]       = "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00";

  EXPECT_TRUE(client.initFromJson(j.dump()));
  EXPECT_EQ(client.m_encryptType, VSCP_ENCRYPTION_AES128);
  EXPECT_EQ(client.m_key[0], 0x01);
  EXPECT_EQ(client.m_key[15], 0x10);
}

TEST(VscpClientMulticast, InitFromJsonEncryptionAES256)
{
  vscpClientMulticast client;

  json j;
  j["encryption"]      = VSCP_ENCRYPTION_AES256;
  j["key"]             = "0102030405060708090A0B0C0D0E0F101112131415161718191A1B1C1D1E1F20";
  j["priority-filter"] = 0;
  j["priority-mask"]   = 0;
  j["class-filter"]    = 0;
  j["class-mask"]      = 0;
  j["type-filter"]     = 0;
  j["type-mask"]       = 0;
  j["guid-filter"]     = "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00";
  j["guid-mask"]       = "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00";

  EXPECT_TRUE(client.initFromJson(j.dump()));
  EXPECT_EQ(client.m_encryptType, VSCP_ENCRYPTION_AES256);
  EXPECT_EQ(client.m_key[0], 0x01);
  EXPECT_EQ(client.m_key[31], 0x20);
}

// ---------------------------------------------------------------------------
//                  getConfigAsJson / initFromJson roundtrip
// ---------------------------------------------------------------------------

TEST(VscpClientMulticast, ConfigRoundtrip)
{
  vscpClientMulticast client1;

  // Set non-default filter values
  vscpEventFilter filter;
  memset(&filter, 0, sizeof(filter));
  filter.filter_priority = 5;
  filter.mask_priority   = 7;
  filter.filter_class    = 20;
  filter.mask_class      = 0xFFFF;
  client1.setfilter(filter);

  // Serialize
  std::string config = client1.getConfigAsJson();
  EXPECT_FALSE(config.empty());

  // Deserialize into a new client
  vscpClientMulticast client2;
  EXPECT_TRUE(client2.initFromJson(config));

  // Verify key filter values survived the roundtrip
  EXPECT_EQ(client2.m_filter.filter_priority, 5);
  EXPECT_EQ(client2.m_filter.mask_priority, 7);
  EXPECT_EQ(client2.m_filter.filter_class, 20);
  EXPECT_EQ(client2.m_filter.mask_class, 0xFFFF);
}

// ---------------------------------------------------------------------------
//                        Callback setting
// ---------------------------------------------------------------------------

TEST(VscpClientMulticast, SetCallbackEv)
{
  vscpClientMulticast client;
  int userData = 42;
  auto cb      = [](vscpEvent &ev, void *pobj) {};

  EXPECT_EQ(client.setCallbackEv(cb, &userData), VSCP_ERROR_SUCCESS);
  EXPECT_TRUE(client.isCallbackEvActive());
}

TEST(VscpClientMulticast, SetCallbackEx)
{
  vscpClientMulticast client;
  int userData = 42;
  auto cb      = [](vscpEventEx &ex, void *pobj) {};

  EXPECT_EQ(client.setCallbackEx(cb, &userData), VSCP_ERROR_SUCCESS);
  EXPECT_TRUE(client.isCallbackExActive());
}

// ---------------------------------------------------------------------------
//                      Encryption defaults
// ---------------------------------------------------------------------------

TEST(VscpClientMulticast, DefaultEncryptionNone)
{
  vscpClientMulticast client;
  EXPECT_EQ(client.m_encryptType, VSCP_ENCRYPTION_NONE);
  EXPECT_FALSE(client.m_bEncrypt);
}

TEST(VscpClientMulticast, SetEncryptionType)
{
  vscpClientMulticast client;
  client.m_encryptType = VSCP_ENCRYPTION_AES256;
  EXPECT_EQ(client.m_encryptType, VSCP_ENCRYPTION_AES256);
}

// ---------------------------------------------------------------------------
//             Default key is set (not all zeros)
// ---------------------------------------------------------------------------

TEST(VscpClientMulticast, DefaultKeyIsSet)
{
  vscpClientMulticast client;

  // The default key should not be all zeros (VSCP_DEFAULT_KEY16 is used)
  bool allZero = true;
  for (int i = 0; i < 32; i++) {
    if (client.m_key[i] != 0) {
      allZero = false;
      break;
    }
  }
  EXPECT_FALSE(allZero);
}
