// unittest.cpp
//
// Unit tests for vscpClientMqtt
//
// These tests exercise the vscpClientMqtt class without requiring
// a real MQTT broker connection. Constructor defaults, JSON config
// parsing (initFromJson), getters/setters, subscription/publish
// topic management, filter, receive queue, version, and callbacks
// are all tested locally.
//

#include <gtest/gtest.h>

#include <vscp-client-mqtt.h>
#include <vscphelper.h>

#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

// ---------------------------------------------------------------------------
//                       Constructor / Default State
// ---------------------------------------------------------------------------

TEST(VscpClientMqtt, ConstructorDefaults)
{
  vscpClientMqtt client;

  EXPECT_EQ(client.getType(), CVscpClient::connType::MQTT);
  EXPECT_FALSE(client.isConnected());
  EXPECT_TRUE(client.m_bRun);
  EXPECT_EQ(client.getHost(), "localhost");
  EXPECT_EQ(client.getUser(), "");
  EXPECT_EQ(client.getPassword(), "");
  EXPECT_EQ(client.getClientId(), "");
  EXPECT_EQ(client.getKeepAlive(), (uint32_t)30);
  EXPECT_EQ(client.getConnectionTimeout(), (uint32_t)5000);
  EXPECT_EQ(client.getResponseTimeout(), (uint32_t)200);
  EXPECT_FALSE(client.isUseTopicForEventDefaults());
  EXPECT_TRUE(client.m_bEscapesPubTopics);
  EXPECT_TRUE(client.m_bJsonMeasurementAdd);
}

TEST(VscpClientMqtt, ConstructorNotConnected)
{
  vscpClientMqtt client;
  EXPECT_FALSE(client.isConnected());
}

TEST(VscpClientMqtt, ReceiveQueueInitiallyEmpty)
{
  vscpClientMqtt client;
  EXPECT_TRUE(client.m_receiveQueue.empty());

  uint16_t count = 999;
  EXPECT_EQ(client.getcount(&count), VSCP_ERROR_SUCCESS);
  EXPECT_EQ(count, 0);
}

// ---------------------------------------------------------------------------
//                          Getters / Setters
// ---------------------------------------------------------------------------

TEST(VscpClientMqtt, SetGetUser)
{
  vscpClientMqtt client;
  std::string user = "testuser";
  client.setUser(user);
  EXPECT_EQ(client.getUser(), "testuser");
}

TEST(VscpClientMqtt, SetGetPassword)
{
  vscpClientMqtt client;
  std::string pw = "secret123";
  client.setPassword(pw);
  EXPECT_EQ(client.getPassword(), "secret123");
}

TEST(VscpClientMqtt, SetGetClientId)
{
  vscpClientMqtt client;
  std::string id = "my-client-001";
  client.setClientId(id);
  EXPECT_EQ(client.getClientId(), "my-client-001");
}

TEST(VscpClientMqtt, SetGetKeepAlive)
{
  vscpClientMqtt client;
  client.setKeepAlive(120);
  EXPECT_EQ(client.getKeepAlive(), (uint32_t)120);
}

TEST(VscpClientMqtt, SetGetConnectionTimeout)
{
  vscpClientMqtt client;
  client.setConnectionTimeout(10000);
  EXPECT_EQ(client.getConnectionTimeout(), (uint32_t)10000);
}

TEST(VscpClientMqtt, SetGetResponseTimeout)
{
  vscpClientMqtt client;
  client.setResponseTimeout(500);
  EXPECT_EQ(client.getResponseTimeout(), (uint32_t)500);
}

TEST(VscpClientMqtt, SetGetUseTopicForEventDefaults)
{
  vscpClientMqtt client;
  client.setUseTopicForEventDefaults(true);
  EXPECT_TRUE(client.isUseTopicForEventDefaults());
  EXPECT_TRUE(client.getUseTopicForEventDefaults());

  client.setUseTopicForEventDefaults(false);
  EXPECT_FALSE(client.isUseTopicForEventDefaults());
}

TEST(VscpClientMqtt, SetGetName)
{
  vscpClientMqtt client;
  std::string name = "MQTT Test Client";
  client.setName(name);
  EXPECT_EQ(client.getName(), "MQTT Test Client");
}

// ---------------------------------------------------------------------------
//                      Subscription/Publish Topics
// ---------------------------------------------------------------------------

TEST(VscpClientMqtt, AddSubscription)
{
  vscpClientMqtt client;
  EXPECT_EQ(client.getSubscribeList()->size(), (size_t)0);

  EXPECT_EQ(client.addSubscription("test/topic/A", autofmt, 1, 0), VSCP_ERROR_SUCCESS);
  EXPECT_EQ(client.getSubscribeList()->size(), (size_t)1);

  EXPECT_EQ(client.addSubscription("test/topic/B", jsonfmt, 2, 0), VSCP_ERROR_SUCCESS);
  EXPECT_EQ(client.getSubscribeList()->size(), (size_t)2);

  // Check first topic
  auto it = client.getSubscribeList()->begin();
  EXPECT_EQ((*it)->getTopic(), "test/topic/A");
  EXPECT_EQ((*it)->getQos(), 1);
  EXPECT_EQ((*it)->getFormat(), autofmt);
  EXPECT_TRUE((*it)->isActive());
}

TEST(VscpClientMqtt, AddPublish)
{
  vscpClientMqtt client;
  EXPECT_EQ(client.getPublishList()->size(), (size_t)0);

  EXPECT_EQ(client.addPublish("pub/topic/A", jsonfmt, 1, true), VSCP_ERROR_SUCCESS);
  EXPECT_EQ(client.getPublishList()->size(), (size_t)1);

  EXPECT_EQ(client.addPublish("pub/topic/B", xmlfmt, 0, false), VSCP_ERROR_SUCCESS);
  EXPECT_EQ(client.getPublishList()->size(), (size_t)2);

  // Check first topic
  auto it = client.getPublishList()->begin();
  EXPECT_EQ((*it)->getTopic(), "pub/topic/A");
  EXPECT_EQ((*it)->getQos(), 1);
  EXPECT_TRUE((*it)->getRetain());
  EXPECT_TRUE((*it)->isRetain());
  EXPECT_EQ((*it)->getFormat(), jsonfmt);
  EXPECT_TRUE((*it)->isActive());
}

TEST(VscpClientMqtt, PublishTopicSetters)
{
  vscpClientMqtt client;
  client.addPublish("initial/topic", jsonfmt, 0, false);
  auto it = client.getPublishList()->begin();

  (*it)->setTopic("new/topic");
  EXPECT_EQ((*it)->getTopic(), "new/topic");

  (*it)->setQos(2);
  EXPECT_EQ((*it)->getQos(), 2);

  (*it)->setRetain(true);
  EXPECT_TRUE((*it)->getRetain());

  (*it)->setFormat(binfmt);
  EXPECT_EQ((*it)->getFormat(), binfmt);

  (*it)->setActive(false);
  EXPECT_FALSE((*it)->isActive());
}

TEST(VscpClientMqtt, SubscribeTopicSetters)
{
  vscpClientMqtt client;
  client.addSubscription("initial/sub", autofmt, 0, 0);
  auto it = client.getSubscribeList()->begin();

  (*it)->setTopic("new/sub");
  EXPECT_EQ((*it)->getTopic(), "new/sub");

  (*it)->setQos(2);
  EXPECT_EQ((*it)->getQos(), 2);

  (*it)->setFormat(strfmt);
  EXPECT_EQ((*it)->getFormat(), strfmt);

  (*it)->setActive(false);
  EXPECT_FALSE((*it)->isActive());
}

// ---------------------------------------------------------------------------
//                              Filter
// ---------------------------------------------------------------------------

TEST(VscpClientMqtt, SetFilter)
{
  vscpClientMqtt client;
  vscpEventFilter filter;
  memset(&filter, 0, sizeof(vscpEventFilter));

  filter.filter_priority = 3;
  filter.mask_priority   = 7;
  filter.filter_class    = 10;
  filter.mask_class      = 0xFFFF;
  filter.filter_type     = 6;
  filter.mask_type       = 0xFFFF;

  EXPECT_EQ(client.setfilter(filter), VSCP_ERROR_SUCCESS);
}

// ---------------------------------------------------------------------------
//                              Clear
// ---------------------------------------------------------------------------

TEST(VscpClientMqtt, ClearQueue)
{
  vscpClientMqtt client;
  EXPECT_EQ(client.clear(), VSCP_ERROR_SUCCESS);

  uint16_t count = 999;
  EXPECT_EQ(client.getcount(&count), VSCP_ERROR_SUCCESS);
  EXPECT_EQ(count, 0);
}

// ---------------------------------------------------------------------------
//                         getcount null check
// ---------------------------------------------------------------------------

TEST(VscpClientMqtt, GetcountNullPointer)
{
  vscpClientMqtt client;
  EXPECT_EQ(client.getcount(nullptr), VSCP_ERROR_INVALID_POINTER);
}

// ---------------------------------------------------------------------------
//                             getversion
// ---------------------------------------------------------------------------

TEST(VscpClientMqtt, GetVersion)
{
  vscpClientMqtt client;
  uint8_t major = 0, minor = 0, release = 0, build = 0;

  EXPECT_EQ(client.getversion(&major, &minor, &release, &build), VSCP_ERROR_SUCCESS);
  // mosquitto 2.x is installed
  EXPECT_GE(major, 1);
  EXPECT_EQ(build, 0);
}

TEST(VscpClientMqtt, GetVersionNullPointers)
{
  vscpClientMqtt client;
  uint8_t v = 0;

  EXPECT_EQ(client.getversion(nullptr, &v, &v, &v), VSCP_ERROR_INVALID_POINTER);
  EXPECT_EQ(client.getversion(&v, nullptr, &v, &v), VSCP_ERROR_INVALID_POINTER);
  EXPECT_EQ(client.getversion(&v, &v, nullptr, &v), VSCP_ERROR_INVALID_POINTER);
  EXPECT_EQ(client.getversion(&v, &v, &v, nullptr), VSCP_ERROR_INVALID_POINTER);
}

// ---------------------------------------------------------------------------
//                            getinterfaces
// ---------------------------------------------------------------------------

TEST(VscpClientMqtt, GetInterfaces)
{
  vscpClientMqtt client;
  std::deque<std::string> iflist;
  EXPECT_EQ(client.getinterfaces(iflist), VSCP_ERROR_SUCCESS);
  EXPECT_TRUE(iflist.empty());
}

// ---------------------------------------------------------------------------
//                              getwcyd
// ---------------------------------------------------------------------------

TEST(VscpClientMqtt, GetWcyd)
{
  vscpClientMqtt client;
  uint64_t wcyd = 0xFFFF;
  EXPECT_EQ(client.getwcyd(wcyd), VSCP_ERROR_SUCCESS);
  EXPECT_EQ(wcyd, (uint64_t)VSCP_SERVER_CAPABILITY_NONE);
}

// ---------------------------------------------------------------------------
//                     receive on empty queue
// ---------------------------------------------------------------------------

TEST(VscpClientMqtt, ReceiveEmptyQueueEvent)
{
  vscpClientMqtt client;
  vscpEvent ev;
  memset(&ev, 0, sizeof(ev));
  EXPECT_EQ(client.receive(ev), VSCP_ERROR_FIFO_EMPTY);
}

TEST(VscpClientMqtt, ReceiveEmptyQueueEventEx)
{
  vscpClientMqtt client;
  vscpEventEx ex;
  memset(&ex, 0, sizeof(ex));
  EXPECT_EQ(client.receive(ex), VSCP_ERROR_FIFO_EMPTY);
}

TEST(VscpClientMqtt, ReceiveEmptyQueueCanal)
{
  vscpClientMqtt client;
  canalMsg msg;
  memset(&msg, 0, sizeof(msg));
  EXPECT_EQ(client.receive(msg), VSCP_ERROR_FIFO_EMPTY);
}

// ---------------------------------------------------------------------------
//                  receiveBlocking on empty queue (timeout)
// ---------------------------------------------------------------------------

TEST(VscpClientMqtt, ReceiveBlockingTimeout)
{
  vscpClientMqtt client;
  vscpEvent ev;
  memset(&ev, 0, sizeof(ev));
  // Very short timeout to not delay tests
  EXPECT_EQ(client.receiveBlocking(ev, 10), VSCP_ERROR_TIMEOUT);
}

TEST(VscpClientMqtt, ReceiveBlockingExTimeout)
{
  vscpClientMqtt client;
  vscpEventEx ex;
  memset(&ex, 0, sizeof(ex));
  EXPECT_EQ(client.receiveBlocking(ex, 10), VSCP_ERROR_TIMEOUT);
}

TEST(VscpClientMqtt, ReceiveBlockingCanalTimeout)
{
  vscpClientMqtt client;
  canalMsg msg;
  memset(&msg, 0, sizeof(msg));
  EXPECT_EQ(client.receiveBlocking(msg, 10), VSCP_ERROR_TIMEOUT);
}

// ---------------------------------------------------------------------------
//                           Callbacks
// ---------------------------------------------------------------------------

TEST(VscpClientMqtt, SetCallbackEvWhenNotConnected)
{
  vscpClientMqtt client;
  int userData = 42;
  auto cb      = [](vscpEvent &ev, void *pobj) {};

  EXPECT_EQ(client.setCallbackEv(cb, &userData), VSCP_ERROR_SUCCESS);
  EXPECT_TRUE(client.isCallbackEvActive());
}

TEST(VscpClientMqtt, SetCallbackExWhenNotConnected)
{
  vscpClientMqtt client;
  int userData = 42;
  auto cb      = [](vscpEventEx &ex, void *pobj) {};

  EXPECT_EQ(client.setCallbackEx(cb, &userData), VSCP_ERROR_SUCCESS);
  EXPECT_TRUE(client.isCallbackExActive());
}

// ---------------------------------------------------------------------------
//                         Parent callbacks
// ---------------------------------------------------------------------------

TEST(VscpClientMqtt, ParentCallbacksDefaultNull)
{
  vscpClientMqtt client;
  EXPECT_EQ(client.m_parentCallbackLog, nullptr);
  EXPECT_EQ(client.m_parentCallbackConnect, nullptr);
  EXPECT_EQ(client.m_parentCallbackDisconnect, nullptr);
  EXPECT_EQ(client.m_parentCallbackPublish, nullptr);
  EXPECT_EQ(client.m_parentCallbackSubscribe, nullptr);
  EXPECT_EQ(client.m_parentCallbackMessage, nullptr);
  EXPECT_EQ(client.m_pParent, nullptr);
}

// ---------------------------------------------------------------------------
//                     GUID setters (srv / if)
// ---------------------------------------------------------------------------

TEST(VscpClientMqtt, SetSrvGuid)
{
  vscpClientMqtt client;
  cguid guid;
  guid.getFromString("FF:FF:FF:FF:FF:FF:FF:FE:B8:27:EB:CF:3A:15:00:01");
  client.setSrvGuid(guid);
  // No public getter, but this should not crash
}

TEST(VscpClientMqtt, SetIfGuid)
{
  vscpClientMqtt client;
  cguid guid;
  guid.getFromString("FF:FF:FF:FF:FF:FF:FF:FE:B8:27:EB:CF:3A:15:00:02");
  client.setIfGuid(guid);
}

// ---------------------------------------------------------------------------
//                       Token maps and escapes
// ---------------------------------------------------------------------------

TEST(VscpClientMqtt, SetUserEscape)
{
  vscpClientMqtt client;
  client.setUserEscape("driver-name", "my-driver");
  // No crash = success; escapes are used internally during publish
}

TEST(VscpClientMqtt, SetTokenMaps)
{
  vscpClientMqtt client;
  std::map<uint16_t, std::string> classMap;
  std::map<uint32_t, std::string> typeMap;
  classMap[10]   = "CLASS_MEASUREMENT";
  typeMap[10 << 16 | 6] = "TYPE_MEASUREMENT_TEMPERATURE";
  client.setTokenMaps(&classMap, &typeMap);
}

// ---------------------------------------------------------------------------
//                     initFromJson - Basic
// ---------------------------------------------------------------------------

TEST(VscpClientMqtt, InitFromJsonBasic)
{
  vscpClientMqtt client;

  json j;
  j["host"]     = "tcp://192.168.1.100:1883";
  j["user"]     = "admin";
  j["password"] = "pass123";
  j["clientid"] = "test-client-01";
  j["keepalive"] = 60;

  EXPECT_TRUE(client.initFromJson(j.dump()));
  EXPECT_EQ(client.getHost(), "192.168.1.100");
  EXPECT_EQ(client.getUser(), "admin");
  EXPECT_EQ(client.getPassword(), "pass123");
  EXPECT_EQ(client.getClientId(), "test-client-01");
  EXPECT_EQ(client.getKeepAlive(), (uint32_t)60);
}

TEST(VscpClientMqtt, InitFromJsonSecureHost)
{
  vscpClientMqtt client;

  json j;
  j["host"] = "stcp://secure.example.com:8883";

  EXPECT_TRUE(client.initFromJson(j.dump()));
  EXPECT_EQ(client.getHost(), "secure.example.com");
}

TEST(VscpClientMqtt, InitFromJsonMqttScheme)
{
  vscpClientMqtt client;

  json j;
  j["host"] = "mqtt://broker.local:1883";

  EXPECT_TRUE(client.initFromJson(j.dump()));
  EXPECT_EQ(client.getHost(), "broker.local");
}

TEST(VscpClientMqtt, InitFromJsonMqttsScheme)
{
  vscpClientMqtt client;

  json j;
  j["host"] = "mqtts://secure-broker.local:8883";

  EXPECT_TRUE(client.initFromJson(j.dump()));
  EXPECT_EQ(client.getHost(), "secure-broker.local");
}

TEST(VscpClientMqtt, InitFromJsonPortOverride)
{
  vscpClientMqtt client;

  json j;
  j["host"] = "tcp://192.168.1.100:1883";
  j["port"] = 9999;

  EXPECT_TRUE(client.initFromJson(j.dump()));
  // Port is stored in private m_port; getPort() re-parses m_host
  // which no longer contains port after initFromJson strips it.
  // Just verify host was parsed correctly.
  EXPECT_EQ(client.getHost(), "192.168.1.100");
}

TEST(VscpClientMqtt, InitFromJsonInvalidJson)
{
  vscpClientMqtt client;
  EXPECT_FALSE(client.initFromJson("not valid json {{{"));
}

TEST(VscpClientMqtt, InitFromJsonEmptyObject)
{
  vscpClientMqtt client;
  EXPECT_TRUE(client.initFromJson("{}"));
  // All defaults should be preserved
  EXPECT_EQ(client.getHost(), "localhost");
}

// ---------------------------------------------------------------------------
//                 initFromJson - Subscribe / Publish arrays
// ---------------------------------------------------------------------------

TEST(VscpClientMqtt, InitFromJsonSubscribeTopics)
{
  vscpClientMqtt client;

  json j;
  j["subscribe"] = json::array();
  j["subscribe"].push_back({{"topic", "vscp/events/#"}, {"qos", 1}});
  j["subscribe"].push_back({{"topic", "vscp/data/#"}, {"qos", 0}});

  EXPECT_TRUE(client.initFromJson(j.dump()));
  EXPECT_EQ(client.getSubscribeList()->size(), (size_t)2);
}

TEST(VscpClientMqtt, InitFromJsonSubscribeSkipsMissingTopic)
{
  vscpClientMqtt client;

  json j;
  j["subscribe"] = json::array();
  // Object without "topic" should be skipped
  j["subscribe"].push_back({{"qos", 1}});
  j["subscribe"].push_back({{"topic", "vscp/valid/#"}, {"qos", 0}});

  EXPECT_TRUE(client.initFromJson(j.dump()));
  EXPECT_EQ(client.getSubscribeList()->size(), (size_t)1);
}

TEST(VscpClientMqtt, InitFromJsonPublishTopics)
{
  vscpClientMqtt client;

  json j;
  j["publish"] = json::array();
  j["publish"].push_back({{"topic", "vscp/out/A"}, {"qos", 1}, {"bretain", true}, {"format", "json"}});
  j["publish"].push_back({{"topic", "vscp/out/B"}, {"qos", 0}, {"bretain", false}, {"format", "xml"}});

  EXPECT_TRUE(client.initFromJson(j.dump()));
  EXPECT_EQ(client.getPublishList()->size(), (size_t)2);

  auto it = client.getPublishList()->begin();
  EXPECT_EQ((*it)->getTopic(), "vscp/out/A");
  EXPECT_EQ((*it)->getQos(), 1);
  EXPECT_TRUE((*it)->getRetain());
  EXPECT_EQ((*it)->getFormat(), jsonfmt);

  ++it;
  EXPECT_EQ((*it)->getTopic(), "vscp/out/B");
  EXPECT_EQ((*it)->getFormat(), xmlfmt);
}

TEST(VscpClientMqtt, InitFromJsonPublishSkipsMissingTopic)
{
  vscpClientMqtt client;

  json j;
  j["publish"] = json::array();
  j["publish"].push_back({{"qos", 1}});  // No topic - skip
  j["publish"].push_back({{"topic", "vscp/valid"}, {"qos", 0}});

  EXPECT_TRUE(client.initFromJson(j.dump()));
  EXPECT_EQ(client.getPublishList()->size(), (size_t)1);
}

// ---------------------------------------------------------------------------
//                     initFromJson - Filter
// ---------------------------------------------------------------------------

TEST(VscpClientMqtt, InitFromJsonFilter)
{
  vscpClientMqtt client;

  json j;
  j["filter"] = {
    {"priority-filter", 3},
    {"priority-mask", 7},
    {"class-filter", 10},
    {"class-mask", 65535},
    {"type-filter", 6},
    {"type-mask", 65535}
  };

  EXPECT_TRUE(client.initFromJson(j.dump()));
}

// ---------------------------------------------------------------------------
//                  initFromJson - Connection timeout
// ---------------------------------------------------------------------------

TEST(VscpClientMqtt, InitFromJsonConnectionTimeout)
{
  vscpClientMqtt client;

  json j;
  j["connection-timeout"] = 15000;

  EXPECT_TRUE(client.initFromJson(j.dump()));
  EXPECT_EQ(client.getConnectionTimeout(), (uint32_t)15000);
}

// ---------------------------------------------------------------------------
//                     getConfigAsJson
// ---------------------------------------------------------------------------

TEST(VscpClientMqtt, GetConfigAsJsonAfterInit)
{
  vscpClientMqtt client;

  json j;
  j["host"]     = "tcp://10.0.0.1:1883";
  j["user"]     = "testuser";
  j["clientid"] = "client-42";

  EXPECT_TRUE(client.initFromJson(j.dump()));

  std::string configStr = client.getConfigAsJson();
  auto result           = json::parse(configStr);
  EXPECT_EQ(result["user"].get<std::string>(), "testuser");
  EXPECT_EQ(result["clientid"].get<std::string>(), "client-42");
}

TEST(VscpClientMqtt, GetConfigAsJsonBeforeInit)
{
  vscpClientMqtt client;
  // Before initFromJson, getConfigAsJson returns null JSON
  std::string configStr = client.getConfigAsJson();
  EXPECT_FALSE(configStr.empty());
}

// ---------------------------------------------------------------------------
//                    connType enum value
// ---------------------------------------------------------------------------

TEST(VscpClientMqtt, ConnTypeIsMQTT)
{
  vscpClientMqtt client;
  EXPECT_EQ(client.getType(), CVscpClient::connType::MQTT);
}

// ---------------------------------------------------------------------------
//                   FullLevel2 flag
// ---------------------------------------------------------------------------

TEST(VscpClientMqtt, FullLevel2Flag)
{
  vscpClientMqtt client;
  client.setFullLevel2(true);
  EXPECT_TRUE(client.isFullLevel2());
  client.setFullLevel2(false);
  EXPECT_FALSE(client.isFullLevel2());
}

// ---------------------------------------------------------------------------
//                initFromJson with bfull-l2
// ---------------------------------------------------------------------------

TEST(VscpClientMqtt, InitFromJsonFullLevel2)
{
  vscpClientMqtt client;
  json j;
  j["bfull-l2"] = true;
  EXPECT_TRUE(client.initFromJson(j.dump()));
  EXPECT_TRUE(client.isFullLevel2());
}

// ---------------------------------------------------------------------------
//          initFromJson - Publish format as number
// ---------------------------------------------------------------------------

TEST(VscpClientMqtt, InitFromJsonPublishFormatAsNumber)
{
  vscpClientMqtt client;

  json j;
  j["publish"] = json::array();
  j["publish"].push_back({{"topic", "vscp/test"}, {"format", 2}});  // strfmt=2

  EXPECT_TRUE(client.initFromJson(j.dump()));
  EXPECT_EQ(client.getPublishList()->size(), (size_t)1);
  auto it = client.getPublishList()->begin();
  EXPECT_EQ((*it)->getFormat(), strfmt);
}

// ---------------------------------------------------------------------------
//         initFromJson - bescape-pub-topics
// ---------------------------------------------------------------------------

TEST(VscpClientMqtt, InitFromJsonEscapePubTopics)
{
  vscpClientMqtt client;

  json j;
  j["bescape-pub-topics"] = false;
  EXPECT_TRUE(client.initFromJson(j.dump()));
  EXPECT_FALSE(client.m_bEscapesPubTopics);
}

// ---------------------------------------------------------------------------
//       initFromJson - bjsonmeasurementblock
// ---------------------------------------------------------------------------

TEST(VscpClientMqtt, InitFromJsonMeasurementBlock)
{
  vscpClientMqtt client;

  json j;
  j["bjsonmeasurementblock"] = false;
  EXPECT_TRUE(client.initFromJson(j.dump()));
  EXPECT_FALSE(client.m_bJsonMeasurementAdd);
}

// ---------------------------------------------------------------------------
//       initFromJson - bUseTopicForEventDefaults
// ---------------------------------------------------------------------------

TEST(VscpClientMqtt, InitFromJsonUseTopicForEventDefaults)
{
  vscpClientMqtt client;

  json j;
  j["bUseTopicForEventDefaults"] = true;
  EXPECT_TRUE(client.initFromJson(j.dump()));
  EXPECT_TRUE(client.isUseTopicForEventDefaults());
}

// ---------------------------------------------------------------------------
//          Incoming filter applied to received MQTT messages
// ---------------------------------------------------------------------------

#include <mosquitto.h>

namespace {

// Number of events queued after handling one JSON event per class
int
queuedAfter(vscpClientMqtt &client, std::initializer_list<int> classes)
{
  for (int vscp_class : classes) {
    std::string payload = "{\"class\":" + std::to_string(vscp_class) +
                          ",\"type\":6,\"head\":0,\"guid\":\"00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:01\","
                          "\"data\":[1,2]}";
    struct mosquitto_message msg = {};
    char topic[]                 = "vscp/test";
    msg.topic                    = topic;
    msg.payload                  = (void *) payload.c_str();
    msg.payloadlen               = (int) payload.size();
    client.handleMessage(&msg);
  }
  uint16_t count = 0;
  client.getcount(&count);
  return count;
}

} // namespace

TEST(VscpClientMqtt, NoFilterQueuesAllEvents)
{
  vscpClientMqtt client;
  EXPECT_TRUE(client.initFromJson("{}"));
  EXPECT_EQ(3, queuedAfter(client, { 10, 20, 10 }));
}

TEST(VscpClientMqtt, FilterKeysAsWrittenByVscpWorks)
{
  // VSCP Works and the other clients write the filter at the top level
  vscpClientMqtt client;
  json j = { { "class-filter", 10 }, { "class-mask", 65535 } };
  EXPECT_TRUE(client.initFromJson(j.dump()));
  EXPECT_EQ(2, queuedAfter(client, { 10, 20, 10 }));
}

TEST(VscpClientMqtt, FilterObject)
{
  vscpClientMqtt client;
  json j;
  j["filter"] = { { "class-filter", 20 }, { "class-mask", 65535 } };
  EXPECT_TRUE(client.initFromJson(j.dump()));
  EXPECT_EQ(1, queuedAfter(client, { 10, 20, 10 }));
}

TEST(VscpClientMqtt, SetfilterAppliesToReceivedEvents)
{
  vscpClientMqtt client;
  EXPECT_TRUE(client.initFromJson("{}"));
  vscpEventFilter filter;
  vscp_clearVSCPFilter(&filter);
  filter.filter_class = 20;
  filter.mask_class   = 0xffff;
  client.setfilter(filter);
  EXPECT_EQ(1, queuedAfter(client, { 10, 20, 10 }));
}
