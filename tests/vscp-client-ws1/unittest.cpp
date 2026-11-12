// unittest.cpp
//
// Unit tests for vscpClientWs1
//
// These tests exercise the vscpClientWs1 class without requiring
// actual websocket communication. Constructor defaults, JSON config
// stubs, getters/setters, receive queue, getcount, clear, timeouts,
// callbacks, and encrypt_password are all tested locally.
//

#include <gtest/gtest.h>

#include <semaphore.h>
#include <vscp-client-ws1.h>
#include <vscphelper.h>

#include <nlohmann/json.hpp>
#include <string>

#include "fakewsserver.h"

using json = nlohmann::json;

// ---------------------------------------------------------------------------
//                       Constructor / Default State
// ---------------------------------------------------------------------------

TEST(VscpClientWs1, ConstructorDefaults)
{
  vscpClientWs1 client;
  EXPECT_EQ(CVscpClient::connType::WS1, client.getType());
  EXPECT_FALSE(client.isConnected());
  EXPECT_FALSE(client.m_bConnected);
}

TEST(VscpClientWs1, ConstructorNotConnected)
{
  vscpClientWs1 client;
  EXPECT_FALSE(client.isConnected());
}

TEST(VscpClientWs1, EventReceiveQueueInitiallyEmpty)
{
  vscpClientWs1 client;
  EXPECT_TRUE(client.m_eventReceiveQueue.empty());
}

TEST(VscpClientWs1, MsgReceiveQueueInitiallyEmpty)
{
  vscpClientWs1 client;
  EXPECT_TRUE(client.m_msgReceiveQueue.empty());
}

// ---------------------------------------------------------------------------
//                      Connection / Response timeout
// ---------------------------------------------------------------------------

TEST(VscpClientWs1, DefaultConnectionTimeout)
{
  vscpClientWs1 client;
  EXPECT_EQ(WS1_CONNECTION_TIMEOUT, client.getConnectionTimeout());
}

TEST(VscpClientWs1, DefaultResponseTimeout)
{
  vscpClientWs1 client;
  EXPECT_EQ(WS1_RESPONSE_TIMEOUT, client.getResponseTimeout());
}

TEST(VscpClientWs1, SetConnectionTimeout)
{
  vscpClientWs1 client;
  client.setConnectionTimeout(5000);
  EXPECT_EQ(5000u, client.getConnectionTimeout());
}

TEST(VscpClientWs1, SetResponseTimeout)
{
  vscpClientWs1 client;
  client.setResponseTimeout(1000);
  EXPECT_EQ(1000u, client.getResponseTimeout());
}

TEST(VscpClientWs1, TimeoutConstants)
{
  EXPECT_EQ(30000u, WS1_CONNECTION_TIMEOUT);
  EXPECT_EQ(2000u, WS1_RESPONSE_TIMEOUT);
}

// ---------------------------------------------------------------------------
//                         ConnType / Name
// ---------------------------------------------------------------------------

TEST(VscpClientWs1, ConnTypeIsWs1)
{
  vscpClientWs1 client;
  EXPECT_EQ(CVscpClient::connType::WS1, client.getType());
}

TEST(VscpClientWs1, SetGetName)
{
  vscpClientWs1 client;
  client.setName("test-ws1");
  EXPECT_EQ("test-ws1", client.getName());
}

TEST(VscpClientWs1, FullLevel2Flag)
{
  vscpClientWs1 client;
  EXPECT_FALSE(client.isFullLevel2());
  client.setFullLevel2(true);
  EXPECT_TRUE(client.isFullLevel2());
}

// ---------------------------------------------------------------------------
//                       getcount / clear
// ---------------------------------------------------------------------------

TEST(VscpClientWs1, GetcountEmpty)
{
  vscpClientWs1 client;
  uint16_t count = 0xFFFF;
  int rv         = client.getcount(&count);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
  EXPECT_EQ(0u, count);
}

TEST(VscpClientWs1, GetcountNullPointer)
{
  vscpClientWs1 client;
  int rv = client.getcount(nullptr);
  EXPECT_EQ(VSCP_ERROR_INVALID_POINTER, rv);
}

TEST(VscpClientWs1, ClearEmptyQueue)
{
  vscpClientWs1 client;
  int rv = client.clear();
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
}

// ---------------------------------------------------------------------------
//                    Receive when not connected
// ---------------------------------------------------------------------------

TEST(VscpClientWs1, ReceiveEventNotConnected)
{
  vscpClientWs1 client;
  vscpEvent ev;
  memset(&ev, 0, sizeof(ev));
  int rv = client.receive(ev);
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, rv);
}

TEST(VscpClientWs1, ReceiveEventExNotConnected)
{
  vscpClientWs1 client;
  vscpEventEx ex;
  memset(&ex, 0, sizeof(ex));
  int rv = client.receive(ex);
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, rv);
}

TEST(VscpClientWs1, ReceiveCanalMsgNotConnected)
{
  vscpClientWs1 client;
  canalMsg msg;
  memset(&msg, 0, sizeof(msg));
  int rv = client.receive(msg);
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, rv);
}

// ---------------------------------------------------------------------------
//             Receive when connected but queue empty
// ---------------------------------------------------------------------------

TEST(VscpClientWs1, ReceiveEventEmptyQueue)
{
  vscpClientWs1 client;
  client.m_bConnected = true;
  vscpEvent ev;
  memset(&ev, 0, sizeof(ev));
  int rv = client.receive(ev);
  EXPECT_EQ(VSCP_ERROR_RCV_EMPTY, rv);
}

TEST(VscpClientWs1, ReceiveEventExEmptyQueue)
{
  vscpClientWs1 client;
  client.m_bConnected = true;
  vscpEventEx ex;
  memset(&ex, 0, sizeof(ex));
  int rv = client.receive(ex);
  EXPECT_EQ(VSCP_ERROR_RCV_EMPTY, rv);
}

TEST(VscpClientWs1, ReceiveCanalMsgEmptyQueue)
{
  vscpClientWs1 client;
  client.m_bConnected = true;
  canalMsg msg;
  memset(&msg, 0, sizeof(msg));
  int rv = client.receive(msg);
  EXPECT_EQ(VSCP_ERROR_RCV_EMPTY, rv);
}

// ---------------------------------------------------------------------------
//            Receive blocking (just calls receive, no sem wait)
// ---------------------------------------------------------------------------

TEST(VscpClientWs1, ReceiveBlockingNotConnected)
{
  vscpClientWs1 client;
  vscpEvent ev;
  memset(&ev, 0, sizeof(ev));
  int rv = client.receiveBlocking(ev, 10);
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, rv);
}

TEST(VscpClientWs1, ReceiveBlockingExNotConnected)
{
  vscpClientWs1 client;
  vscpEventEx ex;
  memset(&ex, 0, sizeof(ex));
  int rv = client.receiveBlocking(ex, 10);
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, rv);
}

TEST(VscpClientWs1, ReceiveBlockingCanalNotConnected)
{
  vscpClientWs1 client;
  canalMsg msg;
  memset(&msg, 0, sizeof(msg));
  int rv = client.receiveBlocking(msg, 10);
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, rv);
}

// ---------------------------------------------------------------------------
//                    Send when not connected
// ---------------------------------------------------------------------------

TEST(VscpClientWs1, SendEventNotConnected)
{
  vscpClientWs1 client;
  vscpEvent ev;
  memset(&ev, 0, sizeof(ev));
  ev.vscp_class = VSCP_CLASS1_MEASUREMENT;
  ev.vscp_type  = 6;
  ev.sizeData   = 0;
  ev.pdata      = nullptr;
  int rv        = client.send(ev);
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, rv);
}

TEST(VscpClientWs1, SendEventExNotConnected)
{
  vscpClientWs1 client;
  vscpEventEx ex;
  memset(&ex, 0, sizeof(ex));
  ex.vscp_class = VSCP_CLASS1_MEASUREMENT;
  ex.vscp_type  = 6;
  ex.sizeData   = 0;
  int rv        = client.send(ex);
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, rv);
}

// ---------------------------------------------------------------------------
//          setfilter / getversion / getinterfaces / getwcyd
//          require connection - verify NOT_CONNECTED returns
// ---------------------------------------------------------------------------

TEST(VscpClientWs1, SetFilterNotConnected)
{
  vscpClientWs1 client;
  vscpEventFilter filter;
  memset(&filter, 0, sizeof(filter));
  int rv = client.setfilter(filter);
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, rv);
}

TEST(VscpClientWs1, GetVersionNotConnected)
{
  vscpClientWs1 client;
  uint8_t major, minor, release, build;
  int rv = client.getversion(&major, &minor, &release, &build);
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, rv);
}

TEST(VscpClientWs1, GetInterfacesNotConnected)
{
  vscpClientWs1 client;
  std::deque<std::string> iflist;
  int rv = client.getinterfaces(iflist);
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, rv);
}

TEST(VscpClientWs1, GetWcydNotConnected)
{
  vscpClientWs1 client;
  uint64_t wcyd = 0;
  int rv        = client.getwcyd(wcyd);
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, rv);
}

// ---------------------------------------------------------------------------
//                        disconnect
// ---------------------------------------------------------------------------

TEST(VscpClientWs1, DisconnectWhenNotConnected)
{
  vscpClientWs1 client;
  int rv = client.disconnect();
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
  EXPECT_FALSE(client.isConnected());
}

// ---------------------------------------------------------------------------
//                        getConfigAsJson / initFromJson stubs
// ---------------------------------------------------------------------------

TEST(VscpClientWs1, GetConfigAsJsonDefaults)
{
  vscpClientWs1 client;
  json j = json::parse(client.getConfigAsJson());
  EXPECT_EQ("ws://localhost:8884/ws1", j["url"].get<std::string>());
  EXPECT_EQ(VSCP_WS_DEFAULT_KEY, j["key"].get<std::string>());
  EXPECT_EQ(WS1_CONNECTION_TIMEOUT, j["connection-timeout"].get<uint32_t>());
  EXPECT_EQ(WS1_RESPONSE_TIMEOUT, j["response-timeout"].get<uint32_t>());
  EXPECT_TRUE(j["bverifypeer"].get<bool>());
}

TEST(VscpClientWs1, InitFromJson)
{
  vscpClientWs1 client;
  EXPECT_TRUE(client.initFromJson("{}"));
  EXPECT_FALSE(client.initFromJson("any string"));
  EXPECT_FALSE(client.initFromJson("{\"key\":\"too short\"}"));

  json cfg;
  cfg["name"]               = "test";
  cfg["url"]                = "wss://example.com:8443/ws1";
  cfg["user"]               = "admin";
  cfg["password"]           = "secret";
  cfg["key"]                = "000102030405060708090A0B0C0D0E0F";
  cfg["connection-timeout"] = 1234;
  cfg["response-timeout"]   = 567;
  cfg["bverifypeer"]        = false;
  cfg["cafile"]             = "/tmp/ca.pem";
  cfg["class-filter"]       = 10;
  cfg["class-mask"]         = 0xffff;
  cfg["guid-mask"]          = "FF:FF:00:00:00:00:00:00:00:00:00:00:00:00:00:00";
  EXPECT_TRUE(client.initFromJson(cfg.dump()));
  EXPECT_EQ("test", client.getName());
  EXPECT_EQ(1234u, client.getConnectionTimeout());
  EXPECT_EQ(567u, client.getResponseTimeout());

  // Everything comes back from getConfigAsJson
  json j = json::parse(client.getConfigAsJson());
  EXPECT_EQ("wss://example.com:8443/ws1", j["url"].get<std::string>());
  EXPECT_EQ("admin", j["user"].get<std::string>());
  EXPECT_EQ("secret", j["password"].get<std::string>());
  EXPECT_EQ("000102030405060708090A0B0C0D0E0F", j["key"].get<std::string>());
  EXPECT_FALSE(j["bverifypeer"].get<bool>());
  EXPECT_EQ("/tmp/ca.pem", j["cafile"].get<std::string>());
  EXPECT_EQ(10, j["class-filter"].get<int>());
  EXPECT_EQ(0xffff, j["class-mask"].get<int>());
  EXPECT_EQ("FF:FF:00:00:00:00:00:00:00:00:00:00:00:00:00:00", j["guid-mask"].get<std::string>());

  // An empty key means the default key
  EXPECT_TRUE(client.initFromJson("{\"key\":\"\"}"));
  j = json::parse(client.getConfigAsJson());
  EXPECT_EQ(VSCP_WS_DEFAULT_KEY, j["key"].get<std::string>());
}

TEST(VscpClientWs1, InitFromJsonFilterObject)
{
  vscpClientWs1 client;
  EXPECT_TRUE(client.initFromJson("{\"filter\":{\"type-filter\":6,\"type-mask\":255}}"));
  json j = json::parse(client.getConfigAsJson());
  EXPECT_EQ(6, j["type-filter"].get<int>());
  EXPECT_EQ(255, j["type-mask"].get<int>());
}

// ---------------------------------------------------------------------------
//                           init
// ---------------------------------------------------------------------------

TEST(VscpClientWs1, InitSetsHostAndPort)
{
  vscpClientWs1 client;
  uint8_t key[32] = { 0 };
  int rv           = client.init("192.168.1.1", 443, true, "admin", "secret", key, 10000, 3000);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
  EXPECT_EQ(10000u, client.getConnectionTimeout());
  EXPECT_EQ(3000u, client.getResponseTimeout());
  json j = json::parse(client.getConfigAsJson());
  EXPECT_EQ("wss://192.168.1.1:443/ws1", j["url"].get<std::string>());
  EXPECT_EQ("00000000000000000000000000000000", j["key"].get<std::string>());
}

TEST(VscpClientWs1, InitWithDefaultTimeouts)
{
  vscpClientWs1 client;
  uint8_t key[32] = { 0 };
  int rv           = client.init("localhost", 8884, false, "user", "pass", key);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
  EXPECT_EQ(WS1_CONNECTION_TIMEOUT, client.getConnectionTimeout());
  EXPECT_EQ(WS1_RESPONSE_TIMEOUT, client.getResponseTimeout());
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

TEST(VscpClientWs1, SetCallbackEv)
{
  vscpClientWs1 client;
  g_evCallbackFired = false;
  int rv            = client.setCallbackEv(testCallbackEv, nullptr);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
}

TEST(VscpClientWs1, SetCallbackEx)
{
  vscpClientWs1 client;
  g_exCallbackFired = false;
  int rv            = client.setCallbackEx(testCallbackEx, nullptr);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
}

// ---------------------------------------------------------------------------
//       Receive returns NOT_SUPPORTED when callback is active
// ---------------------------------------------------------------------------

TEST(VscpClientWs1, ReceiveNotSupportedWithEvCallback)
{
  vscpClientWs1 client;
  client.m_bConnected = true;
  client.setCallbackEv(testCallbackEv, nullptr);

  // Put an event in the queue so we get past the empty check
  vscpEvent *pev = new vscpEvent;
  memset(pev, 0, sizeof(vscpEvent));
  pev->pdata = nullptr;
  client.m_eventReceiveQueue.push_back(pev);

  vscpEvent ev;
  memset(&ev, 0, sizeof(ev));
  int rv = client.receive(ev);
  EXPECT_EQ(VSCP_ERROR_NOT_SUPPORTED, rv);

  // Clean up the event we added
  client.clear();
}

TEST(VscpClientWs1, ReceiveNotSupportedWithExCallback)
{
  vscpClientWs1 client;
  client.m_bConnected = true;
  client.setCallbackEx(testCallbackEx, nullptr);

  // Put an event in the queue so we get past the empty check
  vscpEvent *pev = new vscpEvent;
  memset(pev, 0, sizeof(vscpEvent));
  pev->pdata = nullptr;
  client.m_eventReceiveQueue.push_back(pev);

  vscpEventEx ex;
  memset(&ex, 0, sizeof(ex));
  int rv = client.receive(ex);
  EXPECT_EQ(VSCP_ERROR_NOT_SUPPORTED, rv);

  // Clean up
  client.clear();
}

// ---------------------------------------------------------------------------
//              Receive with event in queue (connected, no callback)
// ---------------------------------------------------------------------------

TEST(VscpClientWs1, ReceiveEventFromQueue)
{
  vscpClientWs1 client;
  client.m_bConnected = true;

  // Create and enqueue an event
  vscpEvent *pev    = new vscpEvent;
  memset(pev, 0, sizeof(vscpEvent));
  pev->vscp_class = VSCP_CLASS1_MEASUREMENT;
  pev->vscp_type  = 6;
  pev->sizeData   = 0;
  pev->pdata      = nullptr;
  client.m_eventReceiveQueue.push_back(pev);

  vscpEvent ev;
  memset(&ev, 0, sizeof(ev));
  int rv = client.receive(ev);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
  EXPECT_EQ(VSCP_CLASS1_MEASUREMENT, ev.vscp_class);
  EXPECT_EQ(6, ev.vscp_type);
  EXPECT_TRUE(client.m_eventReceiveQueue.empty());
}

TEST(VscpClientWs1, ReceiveEventExFromQueue)
{
  vscpClientWs1 client;
  client.m_bConnected = true;

  vscpEvent *pev    = new vscpEvent;
  memset(pev, 0, sizeof(vscpEvent));
  pev->vscp_class = VSCP_CLASS1_MEASUREMENT;
  pev->vscp_type  = 6;
  pev->sizeData   = 0;
  pev->pdata      = nullptr;
  client.m_eventReceiveQueue.push_back(pev);

  vscpEventEx ex;
  memset(&ex, 0, sizeof(ex));
  int rv = client.receive(ex);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
  EXPECT_EQ(VSCP_CLASS1_MEASUREMENT, ex.vscp_class);
  EXPECT_EQ(6, ex.vscp_type);
  EXPECT_TRUE(client.m_eventReceiveQueue.empty());
}

TEST(VscpClientWs1, GetcountWithEvents)
{
  vscpClientWs1 client;

  // Add two events
  for (int i = 0; i < 2; i++) {
    vscpEvent *pev = new vscpEvent;
    memset(pev, 0, sizeof(vscpEvent));
    pev->pdata = nullptr;
    client.m_eventReceiveQueue.push_back(pev);
  }

  uint16_t count = 0;
  int rv         = client.getcount(&count);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
  EXPECT_EQ(2u, count);

  // Clean up
  client.clear();
}

TEST(VscpClientWs1, ClearWithEvents)
{
  vscpClientWs1 client;

  for (int i = 0; i < 3; i++) {
    vscpEvent *pev = new vscpEvent;
    memset(pev, 0, sizeof(vscpEvent));
    pev->pdata = nullptr;
    client.m_eventReceiveQueue.push_back(pev);
  }

  EXPECT_EQ(3u, client.m_eventReceiveQueue.size());
  int rv = client.clear();
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
  EXPECT_TRUE(client.m_eventReceiveQueue.empty());
}

// ---------------------------------------------------------------------------
//                      encrypt_password
// ---------------------------------------------------------------------------

TEST(VscpClientWs1, EncryptPasswordProducesOutput)
{
  vscpClientWs1 client;
  uint8_t key[32] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                       0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
                       0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
                       0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20 };
  uint8_t iv[16]  = { 0 };
  std::string result;

  int rv = client.encrypt_password(result, "admin", "secret", key, iv);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
  EXPECT_FALSE(result.empty());
  // Output is hex string of 16 bytes = 32 hex chars
  EXPECT_EQ(32u, result.size());
}

TEST(VscpClientWs1, EncryptPasswordDeterministic)
{
  vscpClientWs1 client;
  uint8_t key[32] = { 0 };
  uint8_t iv1[16] = { 0 };
  uint8_t iv2[16] = { 0 };
  std::string result1, result2;

  // Use input that is exactly 16 bytes ("admin" + ":" + "0123456789") = 16
  // to avoid AES reading uninitialized memory beyond the string buffer
  client.encrypt_password(result1, "admin", "0123456789", key, iv1);
  client.encrypt_password(result2, "admin", "0123456789", key, iv2);
  EXPECT_EQ(result1, result2);
}

TEST(VscpClientWs1, EncryptPasswordDifferentInputDifferentOutput)
{
  vscpClientWs1 client;
  uint8_t key[32] = { 0 };
  uint8_t iv[16]  = { 0 };
  std::string result1, result2;

  client.encrypt_password(result1, "admin", "password1", key, iv);
  client.encrypt_password(result2, "admin", "password2", key, iv);
  EXPECT_NE(result1, result2);
}

// ---------------------------------------------------------------------------
//                      waitForResponse (timeout)
// ---------------------------------------------------------------------------

TEST(VscpClientWs1, WaitForResponseTimeout)
{
  vscpClientWs1 client;
  // No semaphore posted, should timeout
  int rv = client.waitForResponse(100);
  EXPECT_EQ(VSCP_ERROR_TIMEOUT, rv);
}

// ---------------------------------------------------------------------------
//                      Message receive queue
// ---------------------------------------------------------------------------

TEST(VscpClientWs1, MsgReceiveQueueOps)
{
  vscpClientWs1 client;
  client.m_msgReceiveQueue.push_back("test message 1");
  client.m_msgReceiveQueue.push_back("test message 2");
  EXPECT_EQ(2u, client.m_msgReceiveQueue.size());

  std::string front = client.m_msgReceiveQueue.front();
  client.m_msgReceiveQueue.pop_front();
  EXPECT_EQ("test message 1", front);
  EXPECT_EQ(1u, client.m_msgReceiveQueue.size());
}

// ---------------------------------------------------------------------------
//                      Websocket transport
// ---------------------------------------------------------------------------

TEST(VscpWebSocket, ParseUrl)
{
  vscpWsUrl url;

  EXPECT_TRUE(CVscpWebSocket::parseUrl("ws://localhost:8884/ws1", "/x", url));
  EXPECT_FALSE(url.bTLS);
  EXPECT_EQ("localhost", url.host);
  EXPECT_EQ("8884", url.port);
  EXPECT_EQ("/ws1", url.path);

  EXPECT_TRUE(CVscpWebSocket::parseUrl("wss://example.com", "/ws2", url));
  EXPECT_TRUE(url.bTLS);
  EXPECT_EQ("example.com", url.host);
  EXPECT_EQ("443", url.port);
  EXPECT_EQ("/ws2", url.path);

  EXPECT_TRUE(CVscpWebSocket::parseUrl("  example.com:9000/  ", "/ws1", url));
  EXPECT_FALSE(url.bTLS);
  EXPECT_EQ("example.com", url.host);
  EXPECT_EQ("9000", url.port);
  EXPECT_EQ("/ws1", url.path);

  EXPECT_TRUE(CVscpWebSocket::parseUrl("http://localhost/ws1?x=1", "/ws1", url));
  EXPECT_EQ("80", url.port);
  EXPECT_EQ("/ws1?x=1", url.path);

  EXPECT_TRUE(CVscpWebSocket::parseUrl("HTTPS://[::1]:8443/ws2", "/ws1", url));
  EXPECT_TRUE(url.bTLS);
  EXPECT_EQ("::1", url.host);
  EXPECT_EQ("8443", url.port);
  EXPECT_EQ("/ws2", url.path);

  EXPECT_TRUE(CVscpWebSocket::parseUrl("ws://[fe80::1]/ws1", "/ws1", url));
  EXPECT_EQ("fe80::1", url.host);
  EXPECT_EQ("80", url.port);

  EXPECT_FALSE(CVscpWebSocket::parseUrl("ftp://host/ws1", "/ws1", url));
  EXPECT_FALSE(CVscpWebSocket::parseUrl("ws://:8884/ws1", "/ws1", url));
  EXPECT_FALSE(CVscpWebSocket::parseUrl("ws://host:0/ws1", "/ws1", url));
  EXPECT_FALSE(CVscpWebSocket::parseUrl("ws://host:65536/ws1", "/ws1", url));
  EXPECT_FALSE(CVscpWebSocket::parseUrl("ws://host:abc/ws1", "/ws1", url));
  EXPECT_FALSE(CVscpWebSocket::parseUrl("ws://[::1/ws1", "/ws1", url));
}

TEST(VscpWebSocket, AcceptKeyRfcExample)
{
  // Example from RFC 6455 section 1.3
  EXPECT_EQ("s3pPLMBiTxaQ9kYGzzhZRbK+xOo=", CVscpWebSocket::acceptKey("dGhlIHNhbXBsZSBub25jZQ=="));
}

TEST(VscpWebSocket, BuildFrame)
{
  const uint8_t mask[4] = { 0x37, 0xfa, 0x21, 0x3d };

  // Example from RFC 6455 section 5.7: masked "Hello"
  std::string frame = CVscpWebSocket::buildFrame(0x1, "Hello", mask);
  const unsigned char expected[] = { 0x81, 0x85, 0x37, 0xfa, 0x21, 0x3d, 0x7f, 0x9f, 0x4d, 0x51, 0x58 };
  ASSERT_EQ(sizeof(expected), frame.size());
  EXPECT_EQ(0, memcmp(expected, frame.data(), sizeof(expected)));

  // 16-bit and 64-bit lengths
  frame = CVscpWebSocket::buildFrame(0x2, std::string(300, 'a'), mask);
  EXPECT_EQ(0xfe, (uint8_t) frame[1]);
  EXPECT_EQ(300, ((uint8_t) frame[2] << 8) | (uint8_t) frame[3]);
  EXPECT_EQ(4u + 4u + 300u, frame.size());

  frame = CVscpWebSocket::buildFrame(0x2, std::string(70000, 'a'), mask);
  EXPECT_EQ(0xff, (uint8_t) frame[1]);
  EXPECT_EQ(10u + 4u + 70000u, frame.size());
}

TEST(VscpWebSocket, EncryptCredentialsServerCanDecrypt)
{
  std::string crypto;
  uint8_t key[16], iv[16];
  vscp_hexStr2ByteArray(key, 16, VSCP_WS_DEFAULT_KEY);
  vscp_hexStr2ByteArray(iv, 16, FakeWsServer::sid());

  // Lengths below, at and above a block boundary
  const char *passwords[] = { "x", "secret", "0123456789", "a much longer password than one block" };
  for (const char *pw : passwords) {
    ASSERT_EQ(VSCP_ERROR_SUCCESS, vscp_ws_encryptCredentials(crypto, "admin", pw, key, iv));
    EXPECT_EQ(0u, crypto.size() % 32);
    EXPECT_EQ(std::string("admin:") + pw, FakeWsServer::decrypt(crypto, FakeWsServer::sid(), VSCP_WS_DEFAULT_KEY));
  }
}

TEST(VscpWebSocket, ConnectSendReceive)
{
  FakeWsServer server([](const std::string &msg) { return std::vector<std::string>{ "echo:" + msg }; },
                      { "hello" });

  vscpWsUrl url;
  ASSERT_TRUE(CVscpWebSocket::parseUrl(server.url("/test"), "/", url));

  CVscpWebSocket ws;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, ws.connect(url, 2000));
  EXPECT_TRUE(ws.isOpen());
  EXPECT_NE(std::string::npos, server.m_request.find("GET /test HTTP/1.1\r\n"));
  EXPECT_NE(std::string::npos, server.m_request.find("Sec-WebSocket-Version: 13"));

  std::string msg;
  bool bText = false;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, ws.receive(msg, bText, 2000));
  EXPECT_TRUE(bText);
  EXPECT_EQ("hello", msg);

  // Nothing more to read
  EXPECT_EQ(VSCP_ERROR_TIMEOUT, ws.receive(msg, bText, 50));

  ASSERT_EQ(VSCP_ERROR_SUCCESS, ws.sendText("ping me"));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, ws.receive(msg, bText, 2000));
  EXPECT_EQ("echo:ping me", msg);

  // Long messages use the extended length fields in both directions
  std::string big(70000, 'z');
  ASSERT_EQ(VSCP_ERROR_SUCCESS, ws.sendText(big));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, ws.receive(msg, bText, 2000));
  EXPECT_EQ("echo:" + big, msg);
  EXPECT_FALSE(server.m_bUnmasked);

  ws.close();
  EXPECT_FALSE(ws.isOpen());
  for (int i = 0; (i < 100) && !server.gotClose(); i++) {
    usleep(10000);
  }
  EXPECT_TRUE(server.gotClose());
}

TEST(VscpWebSocket, FragmentsPingAndBinary)
{
  FakeWsServer server([](const std::string &) { return std::vector<std::string>(); });

  vscpWsUrl url;
  ASSERT_TRUE(CVscpWebSocket::parseUrl(server.url("/ws1"), "/", url));
  CVscpWebSocket ws;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, ws.connect(url, 2000));

  // "Hel" + ping + "lo" as a fragmented text message, then a binary message
  server.sendFrame(0x01, "Hel");
  server.sendFrame(0x89, "abc");
  server.sendFrame(0x80, "lo");
  server.sendFrame(0x82, std::string("\x00\x01", 2));

  std::string msg;
  bool bText = false;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, ws.receive(msg, bText, 2000));
  EXPECT_TRUE(bText);
  EXPECT_EQ("Hello", msg);

  ASSERT_EQ(VSCP_ERROR_SUCCESS, ws.receive(msg, bText, 2000));
  EXPECT_FALSE(bText);
  EXPECT_EQ(std::string("\x00\x01", 2), msg);

  // The ping was answered with a pong carrying the same data
  for (int i = 0; (i < 100) && server.pongs().empty(); i++) {
    usleep(10000);
  }
  ASSERT_EQ(1u, server.pongs().size());
  EXPECT_EQ("abc", server.pongs()[0]);
}

TEST(VscpWebSocket, ServerCloseAndProtocolError)
{
  {
    FakeWsServer server([](const std::string &) { return std::vector<std::string>(); });
    vscpWsUrl url;
    ASSERT_TRUE(CVscpWebSocket::parseUrl(server.url("/ws1"), "/", url));
    CVscpWebSocket ws;
    ASSERT_EQ(VSCP_ERROR_SUCCESS, ws.connect(url, 2000));
    server.sendFrame(0x88, std::string("\x03\xe8", 2));
    std::string msg;
    bool bText;
    EXPECT_EQ(VSCP_ERROR_CONNECTION, ws.receive(msg, bText, 2000));
    EXPECT_FALSE(ws.isOpen());
    EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, ws.sendText("x"));
  }

  {
    // Continuation without a start is a protocol error
    FakeWsServer server([](const std::string &) { return std::vector<std::string>(); });
    vscpWsUrl url;
    ASSERT_TRUE(CVscpWebSocket::parseUrl(server.url("/ws1"), "/", url));
    CVscpWebSocket ws;
    ASSERT_EQ(VSCP_ERROR_SUCCESS, ws.connect(url, 2000));
    server.sendFrame(0x80, "oops");
    std::string msg;
    bool bText;
    EXPECT_EQ(VSCP_ERROR_CONNECTION, ws.receive(msg, bText, 2000));
    EXPECT_FALSE(ws.isOpen());
  }

  {
    // Connection dropped without a close frame
    FakeWsServer server([](const std::string &) { return std::vector<std::string>(); });
    vscpWsUrl url;
    ASSERT_TRUE(CVscpWebSocket::parseUrl(server.url("/ws1"), "/", url));
    CVscpWebSocket ws;
    ASSERT_EQ(VSCP_ERROR_SUCCESS, ws.connect(url, 2000));
    server.closeConnection();
    std::string msg;
    bool bText;
    EXPECT_EQ(VSCP_ERROR_CONNECTION, ws.receive(msg, bText, 2000));
  }
}

TEST(VscpWebSocket, ConnectRefusedAndTimeout)
{
  // Nothing listens on the port once the server is gone
  unsigned short port;
  {
    FakeWsServer server([](const std::string &) { return std::vector<std::string>(); });
    port = server.port();
    vscpWsUrl url;
    CVscpWebSocket::parseUrl(server.url("/ws1"), "/", url);
    CVscpWebSocket ws;
    ws.connect(url, 2000); // Let the server thread finish
  }
  vscpWsUrl url;
  ASSERT_TRUE(CVscpWebSocket::parseUrl("ws://127.0.0.1:" + std::to_string(port) + "/ws1", "/", url));
  CVscpWebSocket ws;
  EXPECT_EQ(VSCP_ERROR_CONNECTION, ws.connect(url, 2000));
  EXPECT_FALSE(ws.getLastError().empty());

  // A listening socket that never answers the upgrade
  int s = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family      = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  ASSERT_EQ(0, bind(s, (struct sockaddr *) &addr, sizeof(addr)));
  ASSERT_EQ(0, listen(s, 1));
  socklen_t len = sizeof(addr);
  getsockname(s, (struct sockaddr *) &addr, &len);
  ASSERT_TRUE(CVscpWebSocket::parseUrl("ws://127.0.0.1:" + std::to_string(ntohs(addr.sin_port)), "/ws1", url));
  EXPECT_EQ(VSCP_ERROR_TIMEOUT, ws.connect(url, 200));
  ::close(s);
}

// ---------------------------------------------------------------------------
//                      ws1 protocol against a fake server
// ---------------------------------------------------------------------------

namespace {

// Behaves like the VSCP websocket server for ws1
struct Ws1Server {
  std::mutex mutex;
  std::string filter, mask;
  std::string password = "secret";

  std::vector<std::string> handle(const std::string &msg)
  {
    std::deque<std::string> args;
    vscp_split(args, msg, ";");
    if (args.size() >= 4 && "C" == args[0] && "AUTH" == args[1]) {
      if (FakeWsServer::sid() != args[2] ||
          ("admin:" + password) != FakeWsServer::decrypt(args[3], args[2], VSCP_WS_DEFAULT_KEY)) {
        return { "-;AUTH;3;Not authorised" };
      }
      return { "+;AUTH1;admin;settings" };
    }
    if (args.size() >= 2 && "C" == args[0]) {
      if ("OPEN" == args[1]) {
        return { "+;OPEN" };
      }
      if ("CLOSE" == args[1]) {
        return { "+;CLOSE" };
      }
      if ("CLRQ" == args[1]) {
        return { "+;CLRQ" };
      }
      if ("VERSION" == args[1]) {
        return { "+;VERSION;16.0.0;1.2.3.4" };
      }
      if ("SETFILTER" == args[1] && args.size() >= 4) {
        std::lock_guard<std::mutex> lock(mutex);
        filter = args[2];
        mask   = args[3];
        return { "+;SF" };
      }
      return { "-;" + args[1] + ";2;Unknown command" };
    }
    if (!msg.empty() && 'E' == msg[0]) {
      return { "+;EVENT" };
    }
    return {};
  }
};

std::string
ws1Event(uint16_t vscp_class, uint16_t vscp_type)
{
  return "E;0," + std::to_string(vscp_class) + "," + std::to_string(vscp_type) +
         ",0,2026-11-12T10:00:00Z,0,FF:FF:FF:FF:FF:FF:FF:FE:00:00:00:00:00:00:00:01,1,2,3";
}

} // namespace

TEST(VscpClientWs1, ConnectSendReceiveDisconnect)
{
  Ws1Server logic;
  FakeWsServer server([&logic](const std::string &msg) { return logic.handle(msg); },
                      { std::string("+;AUTH0;") + FakeWsServer::sid() });

  vscpClientWs1 client;
  json cfg;
  cfg["url"]      = server.url("/ws1");
  cfg["user"]     = "admin";
  cfg["password"] = "secret";
  ASSERT_TRUE(client.initFromJson(cfg.dump()));

  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.connect());
  EXPECT_TRUE(client.isConnected());
  EXPECT_NE(std::string::npos, server.m_request.find("GET /ws1 HTTP/1.1"));

  // No filter configured, so none is sent
  std::vector<std::string> rcv = server.received();
  ASSERT_EQ(2u, rcv.size());
  EXPECT_EQ(0u, rcv[0].find("C;AUTH;"));
  EXPECT_EQ("C;OPEN", rcv[1]);

  // Send
  vscpEventEx ex;
  memset(&ex, 0, sizeof(ex));
  ex.vscp_class = 10;
  ex.vscp_type  = 6;
  ex.sizeData   = 2;
  ex.data[0]    = 0x11;
  ex.data[1]    = 0x22;
  EXPECT_EQ(VSCP_ERROR_SUCCESS, client.send(ex));
  rcv = server.received();
  ASSERT_EQ(3u, rcv.size());
  vscpEventEx sent;
  ASSERT_EQ(0u, rcv[2].find("E;"));
  ASSERT_TRUE(vscp_convertStringToEventEx(&sent, rcv[2].substr(2)));
  EXPECT_EQ(10, sent.vscp_class);
  EXPECT_EQ(6, sent.vscp_type);
  EXPECT_EQ(2, sent.sizeData);
  EXPECT_EQ(0x22, sent.data[1]);

  // Receive
  server.sendText(ws1Event(20, 3));
  vscpEventEx rex;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.receiveBlocking(rex, 2000));
  EXPECT_EQ(20, rex.vscp_class);
  EXPECT_EQ(3, rex.vscp_type);
  EXPECT_EQ(3, rex.sizeData);
  EXPECT_EQ(3, rex.data[2]);
  EXPECT_EQ(VSCP_ERROR_TIMEOUT, client.receiveBlocking(rex, 50));

  // Version
  uint8_t major, minor, release, build;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.getversion(&major, &minor, &release, &build));
  EXPECT_EQ(1, major);
  EXPECT_EQ(2, minor);
  EXPECT_EQ(3, release);
  EXPECT_EQ(4, build);

  // Not part of ws1
  std::deque<std::string> iflist;
  EXPECT_EQ(VSCP_ERROR_NOT_SUPPORTED, client.getinterfaces(iflist));

  EXPECT_EQ(VSCP_ERROR_SUCCESS, client.disconnect());
  EXPECT_FALSE(client.isConnected());
  rcv = server.received();
  EXPECT_EQ("C;CLOSE", rcv.back());
}

TEST(VscpClientWs1, WrongPassword)
{
  Ws1Server logic;
  logic.password = "other";
  FakeWsServer server([&logic](const std::string &msg) { return logic.handle(msg); },
                      { std::string("+;AUTH0;") + FakeWsServer::sid() });

  vscpClientWs1 client;
  ASSERT_TRUE(client.initFromJson("{\"url\":\"" + server.url("/ws1") + "\",\"user\":\"admin\",\"password\":\"secret\"}"));
  EXPECT_EQ(VSCP_ERROR_USER, client.connect());
  EXPECT_FALSE(client.isConnected());
  EXPECT_FALSE(client.getLastError().empty());
}

TEST(VscpClientWs1, WrongKey)
{
  Ws1Server logic;
  FakeWsServer server([&logic](const std::string &msg) { return logic.handle(msg); },
                      { std::string("+;AUTH0;") + FakeWsServer::sid() });

  vscpClientWs1 client;
  json cfg;
  cfg["url"]      = server.url("/ws1");
  cfg["user"]     = "admin";
  cfg["password"] = "secret";
  cfg["key"]      = "000102030405060708090A0B0C0D0E0F";
  ASSERT_TRUE(client.initFromJson(cfg.dump()));
  EXPECT_EQ(VSCP_ERROR_USER, client.connect());
}

TEST(VscpClientWs1, NoChallenge)
{
  Ws1Server logic;
  FakeWsServer server([&logic](const std::string &msg) { return logic.handle(msg); });

  vscpClientWs1 client;
  ASSERT_TRUE(client.initFromJson("{\"url\":\"" + server.url("/ws1") + "\",\"connection-timeout\":200}"));
  EXPECT_EQ(VSCP_ERROR_TIMEOUT, client.connect());
  EXPECT_FALSE(client.isConnected());
}

TEST(VscpClientWs1, FilterIsSentAndApplied)
{
  Ws1Server logic;
  FakeWsServer server([&logic](const std::string &msg) { return logic.handle(msg); },
                      { std::string("+;AUTH0;") + FakeWsServer::sid() });

  vscpClientWs1 client;
  json cfg;
  cfg["url"]          = server.url("/ws1");
  cfg["user"]         = "admin";
  cfg["password"]     = "secret";
  cfg["class-filter"] = 10;
  cfg["class-mask"]   = 0xffff;
  ASSERT_TRUE(client.initFromJson(cfg.dump()));

  std::mutex mutex;
  std::vector<uint16_t> classes;
  client.setCallbackEv(
    [&](vscpEvent &ev, void *) {
      std::lock_guard<std::mutex> lock(mutex);
      classes.push_back(ev.vscp_class);
    },
    nullptr);

  auto waitClasses = [&](size_t count) {
    for (int i = 0; i < 200; i++) {
      {
        std::lock_guard<std::mutex> lock(mutex);
        if (classes.size() >= count) {
          return;
        }
      }
      usleep(10000);
    }
  };
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.connect());
  {
    std::lock_guard<std::mutex> lock(logic.mutex);
    EXPECT_EQ("0,10,0,00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00", logic.filter);
    EXPECT_EQ(0u, logic.mask.find("0,65535,0,"));
  }

  // The fake server does not filter, so the client must
  server.sendText(ws1Event(20, 1));
  server.sendText(ws1Event(10, 6));
  waitClasses(1);
  usleep(50000);
  {
    std::lock_guard<std::mutex> lock(mutex);
    ASSERT_EQ(1u, classes.size());
    EXPECT_EQ(10, classes[0]);
  }

  // A new filter is sent to the server at once
  vscpEventFilter filter;
  memset(&filter, 0, sizeof(filter));
  EXPECT_EQ(VSCP_ERROR_SUCCESS, client.setfilter(filter));
  {
    std::lock_guard<std::mutex> lock(logic.mutex);
    EXPECT_EQ(0u, logic.mask.find("0,0,0,"));
  }
  server.sendText(ws1Event(20, 1));
  waitClasses(2);
  std::lock_guard<std::mutex> lock(mutex);
  ASSERT_EQ(2u, classes.size());
  EXPECT_EQ(20, classes[1]);
}

TEST(VscpClientWs1, ConnectionLost)
{
  Ws1Server logic;
  FakeWsServer server([&logic](const std::string &msg) { return logic.handle(msg); },
                      { std::string("+;AUTH0;") + FakeWsServer::sid() });

  vscpClientWs1 client;
  ASSERT_TRUE(client.initFromJson("{\"url\":\"" + server.url("/ws1") + "\",\"user\":\"admin\",\"password\":\"secret\"}"));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, client.connect());

  server.closeConnection();
  for (int i = 0; (i < 200) && client.isConnected(); i++) {
    usleep(10000);
  }
  EXPECT_FALSE(client.isConnected());

  vscpEventEx ex;
  memset(&ex, 0, sizeof(ex));
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, client.send(ex));
  EXPECT_EQ(VSCP_ERROR_SUCCESS, client.disconnect());
}
