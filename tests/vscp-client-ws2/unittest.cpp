// unittest.cpp
//
// Unit tests for vscpClientWs2
//

#include <semaphore.h>

#include <deque>
#include <string>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <vscp.h>
#include <vscphelper.h>
#include <vscp-client-ws2.h>

#include "../vscp-client-ws1/fakewsserver.h"

using json = nlohmann::json;

// ============================================================================
//                       Constructor / Defaults
// ============================================================================

TEST(VscpClientWs2, ConstructorDefaults)
{
  vscpClientWs2 ws2;
  EXPECT_EQ(CVscpClient::connType::WS2, ws2.getType());
  EXPECT_FALSE(ws2.isConnected());
  EXPECT_TRUE(ws2.m_eventReceiveQueue.empty());
  EXPECT_TRUE(ws2.m_msgReceiveQueue.empty());
}

TEST(VscpClientWs2, ConstructorNotConnected)
{
  vscpClientWs2 ws2;
  EXPECT_FALSE(ws2.m_bConnected);
}

TEST(VscpClientWs2, EventReceiveQueueInitiallyEmpty)
{
  vscpClientWs2 ws2;
  EXPECT_EQ(0u, ws2.m_eventReceiveQueue.size());
}

TEST(VscpClientWs2, MsgReceiveQueueInitiallyEmpty)
{
  vscpClientWs2 ws2;
  EXPECT_EQ(0u, ws2.m_msgReceiveQueue.size());
}

// ============================================================================
//                       Timeout constants & accessors
// ============================================================================

TEST(VscpClientWs2, DefaultConnectionTimeout)
{
  vscpClientWs2 ws2;
  EXPECT_EQ(WS2_CONNECTION_TIMEOUT, ws2.getConnectionTimeout());
}

TEST(VscpClientWs2, DefaultResponseTimeout)
{
  vscpClientWs2 ws2;
  EXPECT_EQ(WS2_RESPONSE_TIMEOUT, ws2.getResponseTimeout());
}

TEST(VscpClientWs2, SetConnectionTimeout)
{
  vscpClientWs2 ws2;
  ws2.setConnectionTimeout(5000);
  EXPECT_EQ(5000u, ws2.getConnectionTimeout());
}

TEST(VscpClientWs2, SetResponseTimeout)
{
  vscpClientWs2 ws2;
  ws2.setResponseTimeout(3000);
  EXPECT_EQ(3000u, ws2.getResponseTimeout());
}

TEST(VscpClientWs2, TimeoutConstants)
{
  EXPECT_EQ(30000u, WS2_CONNECTION_TIMEOUT);
  EXPECT_EQ(2000u, WS2_RESPONSE_TIMEOUT);
}

// ============================================================================
//                       Type / name / level
// ============================================================================

TEST(VscpClientWs2, ConnTypeIsWs2)
{
  vscpClientWs2 ws2;
  EXPECT_EQ(CVscpClient::connType::WS2, ws2.getType());
}

TEST(VscpClientWs2, SetGetName)
{
  vscpClientWs2 ws2;
  ws2.setName("test-ws2");
  EXPECT_EQ("test-ws2", ws2.getName());
}

TEST(VscpClientWs2, FullLevel2Flag)
{
  vscpClientWs2 ws2;
  ws2.setFullLevel2(true);
  EXPECT_TRUE(ws2.isFullLevel2());
  ws2.setFullLevel2(false);
  EXPECT_FALSE(ws2.isFullLevel2());
}

// ============================================================================
//                       getcount / clear (no connection needed)
// ============================================================================

TEST(VscpClientWs2, GetcountEmpty)
{
  vscpClientWs2 ws2;
  uint16_t cnt = 999;
  EXPECT_EQ(VSCP_ERROR_SUCCESS, ws2.getcount(&cnt));
  EXPECT_EQ(0u, cnt);
}

TEST(VscpClientWs2, GetcountNullPointer)
{
  vscpClientWs2 ws2;
  EXPECT_EQ(VSCP_ERROR_INVALID_POINTER, ws2.getcount(nullptr));
}

TEST(VscpClientWs2, ClearEmptyQueue)
{
  vscpClientWs2 ws2;
  EXPECT_EQ(VSCP_ERROR_SUCCESS, ws2.clear());
}

// ============================================================================
//                       Receive – not connected
// ============================================================================

TEST(VscpClientWs2, ReceiveEventNotConnected)
{
  vscpClientWs2 ws2;
  vscpEvent ev;
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, ws2.receive(ev));
}

TEST(VscpClientWs2, ReceiveEventExNotConnected)
{
  vscpClientWs2 ws2;
  vscpEventEx ex;
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, ws2.receive(ex));
}

TEST(VscpClientWs2, ReceiveCanalMsgNotConnected)
{
  vscpClientWs2 ws2;
  canalMsg msg;
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, ws2.receive(msg));
}

// ============================================================================
//                  Receive – connected, empty queue
// ============================================================================

TEST(VscpClientWs2, ReceiveEventEmptyQueue)
{
  vscpClientWs2 ws2;
  ws2.m_bConnected = true;
  vscpEvent ev;
  EXPECT_EQ(VSCP_ERROR_RCV_EMPTY, ws2.receive(ev));
  ws2.m_bConnected = false;
}

TEST(VscpClientWs2, ReceiveEventExEmptyQueue)
{
  vscpClientWs2 ws2;
  ws2.m_bConnected = true;
  vscpEventEx ex;
  EXPECT_EQ(VSCP_ERROR_RCV_EMPTY, ws2.receive(ex));
  ws2.m_bConnected = false;
}

TEST(VscpClientWs2, ReceiveCanalMsgEmptyQueue)
{
  vscpClientWs2 ws2;
  ws2.m_bConnected = true;
  canalMsg msg;
  EXPECT_EQ(VSCP_ERROR_RCV_EMPTY, ws2.receive(msg));
  ws2.m_bConnected = false;
}

// ============================================================================
//                  Receive blocking – not connected
// ============================================================================

TEST(VscpClientWs2, ReceiveBlockingNotConnected)
{
  vscpClientWs2 ws2;
  vscpEvent ev;
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, ws2.receiveBlocking(ev, 10));
}

TEST(VscpClientWs2, ReceiveBlockingExNotConnected)
{
  vscpClientWs2 ws2;
  vscpEventEx ex;
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, ws2.receiveBlocking(ex, 10));
}

TEST(VscpClientWs2, ReceiveBlockingCanalNotConnected)
{
  vscpClientWs2 ws2;
  canalMsg msg;
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, ws2.receiveBlocking(msg, 10));
}

// ============================================================================
//                  Send – not connected
// ============================================================================

TEST(VscpClientWs2, SendEventNotConnected)
{
  vscpClientWs2 ws2;
  vscpEvent ev = {};
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, ws2.send(ev));
}

TEST(VscpClientWs2, SendEventExNotConnected)
{
  vscpClientWs2 ws2;
  vscpEventEx ex = {};
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, ws2.send(ex));
}

// ============================================================================
//                  Other methods – not connected
// ============================================================================

TEST(VscpClientWs2, SetFilterNotConnected)
{
  vscpClientWs2 ws2;
  vscpEventFilter filter = {};
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, ws2.setfilter(filter));
}

TEST(VscpClientWs2, GetVersionNotConnected)
{
  vscpClientWs2 ws2;
  uint8_t major, minor, release, build;
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED,
            ws2.getversion(&major, &minor, &release, &build));
}

TEST(VscpClientWs2, GetInterfacesNotConnected)
{
  vscpClientWs2 ws2;
  std::deque<std::string> iflist;
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, ws2.getinterfaces(iflist));
}

TEST(VscpClientWs2, GetWcydNotConnected)
{
  vscpClientWs2 ws2;
  uint64_t wcyd = 0;
  EXPECT_EQ(VSCP_ERROR_NOT_CONNECTED, ws2.getwcyd(wcyd));
}

// ============================================================================
//                  Disconnect
// ============================================================================

TEST(VscpClientWs2, DisconnectWhenNotConnected)
{
  vscpClientWs2 ws2;
  EXPECT_EQ(VSCP_ERROR_SUCCESS, ws2.disconnect());
  EXPECT_FALSE(ws2.isConnected());
}

// ============================================================================
//                  JSON config stubs
// ============================================================================

TEST(VscpClientWs2, GetConfigAsJsonDefaults)
{
  vscpClientWs2 ws2;
  json j = json::parse(ws2.getConfigAsJson());
  EXPECT_EQ("ws://localhost:8884/ws2", j["url"].get<std::string>());
  EXPECT_EQ(VSCP_WS_DEFAULT_KEY, j["key"].get<std::string>());
  EXPECT_EQ(WS2_CONNECTION_TIMEOUT, j["connection-timeout"].get<uint32_t>());
  EXPECT_EQ(WS2_RESPONSE_TIMEOUT, j["response-timeout"].get<uint32_t>());
}

TEST(VscpClientWs2, InitFromJson)
{
  vscpClientWs2 ws2;
  EXPECT_TRUE(ws2.initFromJson("{}"));
  EXPECT_FALSE(ws2.initFromJson(""));
  EXPECT_FALSE(ws2.initFromJson("garbage"));
  EXPECT_FALSE(ws2.initFromJson("{\"key\":\"XYZ\"}"));

  EXPECT_TRUE(ws2.initFromJson("{\"url\":\"ws://example.com:9000/ws2\",\"username\":\"bob\","
                               "\"password\":\"pw\",\"type-filter\":3,\"type-mask\":255}"));
  json j = json::parse(ws2.getConfigAsJson());
  EXPECT_EQ("ws://example.com:9000/ws2", j["url"].get<std::string>());
  EXPECT_EQ("bob", j["user"].get<std::string>());
  EXPECT_EQ("pw", j["password"].get<std::string>());
  EXPECT_EQ(3, j["type-filter"].get<int>());
  EXPECT_EQ(255, j["type-mask"].get<int>());
}

// ============================================================================
//                  init()
// ============================================================================

TEST(VscpClientWs2, InitSetsHostAndPort)
{
  vscpClientWs2 ws2;
  uint8_t key[32] = { 0 };
  ws2.init("192.168.1.1", 9001, true, "admin", "secret", key);
  json j = json::parse(ws2.getConfigAsJson());
  EXPECT_EQ("wss://192.168.1.1:9001/ws2", j["url"].get<std::string>());
  EXPECT_EQ("admin", j["user"].get<std::string>());
}

TEST(VscpClientWs2, InitWithDefaultTimeouts)
{
  vscpClientWs2 ws2;
  uint8_t key[32] = { 0 };
  ws2.init("host", 443, false, "user", "pass", key);
  EXPECT_EQ(WS2_CONNECTION_TIMEOUT, ws2.getConnectionTimeout());
  EXPECT_EQ(WS2_RESPONSE_TIMEOUT, ws2.getResponseTimeout());
}

TEST(VscpClientWs2, InitWithCustomTimeouts)
{
  vscpClientWs2 ws2;
  uint8_t key[32] = { 0 };
  ws2.init("host", 443, false, "user", "pass", key, 10000, 5000);
  EXPECT_EQ(10000u, ws2.getConnectionTimeout());
  EXPECT_EQ(5000u, ws2.getResponseTimeout());
}

// ============================================================================
//                  Callbacks
// ============================================================================

static void
evCallback(vscpEvent &ev, void *pobj)
{
  (void)ev;
  (void)pobj;
}

static void
exCallback(vscpEventEx &ex, void *pobj)
{
  (void)ex;
  (void)pobj;
}

TEST(VscpClientWs2, SetCallbackEv)
{
  vscpClientWs2 ws2;
  EXPECT_EQ(VSCP_ERROR_SUCCESS, ws2.setCallbackEv(evCallback, nullptr));
  EXPECT_TRUE(ws2.isCallbackEvActive());
}

TEST(VscpClientWs2, SetCallbackEx)
{
  vscpClientWs2 ws2;
  EXPECT_EQ(VSCP_ERROR_SUCCESS, ws2.setCallbackEx(exCallback, nullptr));
  EXPECT_TRUE(ws2.isCallbackExActive());
}

TEST(VscpClientWs2, ReceiveNotSupportedWithEvCallback)
{
  vscpClientWs2 ws2;
  ws2.m_bConnected = true;

  // Enqueue an event so we pass the "empty" check
  vscpEvent *pev = new vscpEvent;
  memset(pev, 0, sizeof(vscpEvent));
  ws2.m_eventReceiveQueue.push_back(pev);

  ws2.setCallbackEv(evCallback, nullptr);

  vscpEvent ev;
  EXPECT_EQ(VSCP_ERROR_NOT_SUPPORTED, ws2.receive(ev));

  // Cleanup
  ws2.m_bConnected = false;
  ws2.clear();
}

TEST(VscpClientWs2, ReceiveNotSupportedWithExCallback)
{
  vscpClientWs2 ws2;
  ws2.m_bConnected = true;

  vscpEvent *pev = new vscpEvent;
  memset(pev, 0, sizeof(vscpEvent));
  ws2.m_eventReceiveQueue.push_back(pev);

  ws2.setCallbackEx(exCallback, nullptr);

  vscpEventEx ex;
  EXPECT_EQ(VSCP_ERROR_NOT_SUPPORTED, ws2.receive(ex));

  ws2.m_bConnected = false;
  ws2.clear();
}

// ============================================================================
//                  Receive event from queue
// ============================================================================

TEST(VscpClientWs2, ReceiveEventFromQueue)
{
  vscpClientWs2 ws2;
  ws2.m_bConnected = true;

  // Enqueue a test event
  vscpEvent *pev = new vscpEvent;
  memset(pev, 0, sizeof(vscpEvent));
  pev->vscp_class = 10;
  pev->vscp_type  = 6;
  pev->pdata      = nullptr;
  pev->sizeData   = 0;
  ws2.m_eventReceiveQueue.push_back(pev);

  vscpEvent ev;
  memset(&ev, 0, sizeof(vscpEvent));
  EXPECT_EQ(VSCP_ERROR_SUCCESS, ws2.receive(ev));
  EXPECT_EQ(10, ev.vscp_class);
  EXPECT_EQ(6, ev.vscp_type);

  ws2.m_bConnected = false;
}

TEST(VscpClientWs2, ReceiveEventExFromQueue)
{
  vscpClientWs2 ws2;
  ws2.m_bConnected = true;

  vscpEvent *pev = new vscpEvent;
  memset(pev, 0, sizeof(vscpEvent));
  pev->vscp_class = 20;
  pev->vscp_type  = 3;
  pev->pdata      = nullptr;
  pev->sizeData   = 0;
  ws2.m_eventReceiveQueue.push_back(pev);

  vscpEventEx ex;
  memset(&ex, 0, sizeof(vscpEventEx));
  EXPECT_EQ(VSCP_ERROR_SUCCESS, ws2.receive(ex));
  EXPECT_EQ(20, ex.vscp_class);
  EXPECT_EQ(3, ex.vscp_type);

  ws2.m_bConnected = false;
}

// ============================================================================
//                  getcount / clear with events
// ============================================================================

TEST(VscpClientWs2, GetcountWithEvents)
{
  vscpClientWs2 ws2;

  for (int i = 0; i < 5; i++) {
    vscpEvent *pev = new vscpEvent;
    memset(pev, 0, sizeof(vscpEvent));
    pev->pdata = nullptr;
    ws2.m_eventReceiveQueue.push_back(pev);
  }

  uint16_t cnt = 0;
  EXPECT_EQ(VSCP_ERROR_SUCCESS, ws2.getcount(&cnt));
  EXPECT_EQ(5u, cnt);

  ws2.clear();
}

TEST(VscpClientWs2, ClearWithEvents)
{
  vscpClientWs2 ws2;

  for (int i = 0; i < 3; i++) {
    vscpEvent *pev = new vscpEvent;
    memset(pev, 0, sizeof(vscpEvent));
    pev->pdata = nullptr;
    ws2.m_eventReceiveQueue.push_back(pev);
  }

  EXPECT_EQ(VSCP_ERROR_SUCCESS, ws2.clear());

  uint16_t cnt = 999;
  ws2.getcount(&cnt);
  EXPECT_EQ(0u, cnt);
}

// ============================================================================
//                  encrypt_password
// ============================================================================

TEST(VscpClientWs2, EncryptPasswordProducesOutput)
{
  vscpClientWs2 ws2;
  std::string out;
  uint8_t key[32] = { 0 };
  uint8_t iv[16]  = { 0 };
  EXPECT_EQ(VSCP_ERROR_SUCCESS,
            ws2.encrypt_password(out, "admin", "secret", key, iv));
  EXPECT_FALSE(out.empty());
}

TEST(VscpClientWs2, EncryptPasswordDeterministic)
{
  vscpClientWs2 ws2;
  std::string out1, out2;
  uint8_t key[32] = { 1, 2, 3, 4 };
  uint8_t iv[16]  = { 5, 6, 7, 8 };
  ws2.encrypt_password(out1, "admin", "secret", key, iv);
  ws2.encrypt_password(out2, "admin", "secret", key, iv);
  EXPECT_EQ(out1, out2);
}

TEST(VscpClientWs2, EncryptPasswordDifferentInputDifferentOutput)
{
  vscpClientWs2 ws2;
  std::string outA, outB;
  uint8_t key[32] = { 0 };
  uint8_t iv[16]  = { 0 };
  ws2.encrypt_password(outA, "admin", "secret", key, iv);
  ws2.encrypt_password(outB, "admin", "other", key, iv);
  EXPECT_NE(outA, outB);
}

// ============================================================================
//                  waitForResponse
// ============================================================================

TEST(VscpClientWs2, WaitForResponseTimeout)
{
  vscpClientWs2 ws2;
  // With no signal posted, this should return a non-success code quickly
  int rv = ws2.waitForResponse(1); // 1 ms timeout
  EXPECT_NE(VSCP_ERROR_SUCCESS, rv);
}

// ============================================================================
//                  JSON message receive queue
// ============================================================================

TEST(VscpClientWs2, MsgReceiveQueueOps)
{
  vscpClientWs2 ws2;
  EXPECT_TRUE(ws2.m_msgReceiveQueue.empty());

  json j;
  j["type"]    = "+";
  j["command"] = "test";
  ws2.m_msgReceiveQueue.push_back(j);

  EXPECT_EQ(1u, ws2.m_msgReceiveQueue.size());

  json front = ws2.m_msgReceiveQueue.front();
  EXPECT_EQ("+", front["type"]);
  EXPECT_EQ("test", front["command"]);

  ws2.m_msgReceiveQueue.pop_front();
  EXPECT_TRUE(ws2.m_msgReceiveQueue.empty());
}

TEST(VscpClientWs2, MsgReceiveQueueMultipleMessages)
{
  vscpClientWs2 ws2;

  for (int i = 0; i < 5; i++) {
    json j;
    j["index"] = i;
    ws2.m_msgReceiveQueue.push_back(j);
  }

  EXPECT_EQ(5u, ws2.m_msgReceiveQueue.size());

  for (int i = 0; i < 5; i++) {
    json front = ws2.m_msgReceiveQueue.front();
    ws2.m_msgReceiveQueue.pop_front();
    EXPECT_EQ(i, front["index"]);
  }

  EXPECT_TRUE(ws2.m_msgReceiveQueue.empty());
}

// ============================================================================
//                  ws2 protocol against a fake server
// ============================================================================

namespace {

std::string
reply(const std::string &type, const std::string &cmd, json args = nullptr)
{
  json j;
  j["type"]    = type;
  j["command"] = cmd;
  if ("-" == type) {
    j["errcode"] = 3;
    j["errstr"]  = "Not authorised";
  }
  else {
    j["args"] = args;
  }
  return j.dump();
}

// Behaves like the VSCP websocket server for ws2
struct Ws2Server {
  std::mutex mutex;
  json filter;
  std::string password = "secret";

  std::vector<std::string> handle(const std::string &msg)
  {
    json j = json::parse(msg);
    std::string type = j["type"].get<std::string>();
    if ("EVENT" == type) {
      return { reply("+", "EVENT") };
    }
    if ("CMD" != type) {
      return {};
    }
    std::string cmd = j["command"].get<std::string>();
    if ("AUTH" == cmd) {
      std::string iv     = j["args"]["iv"].get<std::string>();
      std::string crypto = j["args"]["crypto"].get<std::string>();
      if ((FakeWsServer::sid() != iv) ||
          (("admin:" + password) != FakeWsServer::decrypt(crypto, iv, VSCP_WS_DEFAULT_KEY))) {
        return { reply("-", "AUTH") };
      }
      return { reply("+", "AUTH") };
    }
    if ("SETFILTER" == cmd) {
      std::lock_guard<std::mutex> lock(mutex);
      filter = j["args"];
      return { reply("+", "SETFILTER") };
    }
    if ("VERSION" == cmd) {
      return { reply("+", "VERSION", json::parse("{\"version\":\"1.2.3-4\"}")) };
    }
    return { reply("+", cmd) };
  }
};

std::string
greeting(void)
{
  return std::string("{\"type\":\"+\",\"args\":[\"AUTH0\",\"") + FakeWsServer::sid() + "\"]}";
}

std::string
ws2Event(uint16_t vscp_class, uint16_t vscp_type)
{
  vscpEventEx ex;
  memset(&ex, 0, sizeof(ex));
  ex.vscp_class = vscp_class;
  ex.vscp_type  = vscp_type;
  ex.sizeData   = 3;
  ex.data[0]    = 1;
  ex.data[1]    = 2;
  ex.data[2]    = 3;
  ex.GUID[0]    = 0xff;
  ex.year       = 2026;
  ex.month      = 11;
  ex.day        = 12;
  ex.hour       = 10;
  ex.minute     = 20;
  ex.second     = 30;
  std::string str;
  vscp_convertEventExToJSON(str, &ex);
  return "{\"type\":\"EVENT\",\"event\":" + str + "}";
}

} // namespace

TEST(VscpClientWs2, ConnectSendReceiveDisconnect)
{
  Ws2Server logic;
  FakeWsServer server([&logic](const std::string &msg) { return logic.handle(msg); }, { greeting() });

  vscpClientWs2 ws2;
  json cfg;
  cfg["url"]      = server.url("/ws2");
  cfg["user"]     = "admin";
  cfg["password"] = "secret";
  ASSERT_TRUE(ws2.initFromJson(cfg.dump()));

  ASSERT_EQ(VSCP_ERROR_SUCCESS, ws2.connect());
  EXPECT_TRUE(ws2.isConnected());
  EXPECT_NE(std::string::npos, server.m_request.find("GET /ws2 HTTP/1.1"));

  std::vector<std::string> rcv = server.received();
  ASSERT_EQ(2u, rcv.size());
  json auth = json::parse(rcv[0]);
  EXPECT_EQ("CMD", auth["type"]);
  EXPECT_EQ("AUTH", auth["command"]);
  EXPECT_EQ(FakeWsServer::sid(), auth["args"]["iv"]);
  json open = json::parse(rcv[1]);
  EXPECT_EQ("OPEN", open["command"]);

  // Send
  vscpEventEx ex;
  memset(&ex, 0, sizeof(ex));
  ex.vscp_class = 10;
  ex.vscp_type  = 6;
  ex.sizeData   = 2;
  ex.data[0]    = 0x11;
  ex.data[1]    = 0x22;
  EXPECT_EQ(VSCP_ERROR_SUCCESS, ws2.send(ex));
  rcv = server.received();
  ASSERT_EQ(3u, rcv.size());
  json ev = json::parse(rcv[2]);
  EXPECT_EQ("EVENT", ev["type"]);
  vscpEventEx sent;
  std::string strEvent = ev["event"].dump();
  ASSERT_TRUE(vscp_convertJSONToEventEx(&sent, strEvent));
  EXPECT_EQ(10, sent.vscp_class);
  EXPECT_EQ(6, sent.vscp_type);
  EXPECT_EQ(2, sent.sizeData);
  EXPECT_EQ(0x22, sent.data[1]);

  // Receive
  server.sendText(ws2Event(20, 3));
  vscpEventEx rex;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, ws2.receiveBlocking(rex, 2000));
  EXPECT_EQ(20, rex.vscp_class);
  EXPECT_EQ(3, rex.vscp_type);
  EXPECT_EQ(3, rex.sizeData);
  EXPECT_EQ(3, rex.data[2]);
  EXPECT_EQ(0xff, rex.GUID[0]);
  // JSON events carry a Unix timestamp in nanoseconds (2026-11-12T10:20:30Z)
  EXPECT_EQ(VSCP_HEADER16_FRAME_VERSION_UNIX_NS, rex.head & VSCP_HEADER16_FRAME_VERSION_MASK);
  EXPECT_EQ(1794478830000000000ULL, rex.timestamp_ns);

  // Version
  uint8_t major, minor, release, build;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, ws2.getversion(&major, &minor, &release, &build));
  EXPECT_EQ(1, major);
  EXPECT_EQ(2, minor);
  EXPECT_EQ(3, release);
  EXPECT_EQ(4, build);

  EXPECT_EQ(VSCP_ERROR_SUCCESS, ws2.disconnect());
  EXPECT_FALSE(ws2.isConnected());
  json last = json::parse(server.received().back());
  EXPECT_EQ("CLOSE", last["command"]);
}

TEST(VscpClientWs2, WrongPassword)
{
  Ws2Server logic;
  logic.password = "other";
  FakeWsServer server([&logic](const std::string &msg) { return logic.handle(msg); }, { greeting() });

  vscpClientWs2 ws2;
  ASSERT_TRUE(ws2.initFromJson("{\"url\":\"" + server.url("/ws2") + "\",\"user\":\"admin\",\"password\":\"secret\"}"));
  EXPECT_EQ(VSCP_ERROR_USER, ws2.connect());
  EXPECT_FALSE(ws2.isConnected());
}

TEST(VscpClientWs2, FilterIsSentAndApplied)
{
  Ws2Server logic;
  FakeWsServer server([&logic](const std::string &msg) { return logic.handle(msg); }, { greeting() });

  vscpClientWs2 ws2;
  json cfg;
  cfg["url"]          = server.url("/ws2");
  cfg["user"]         = "admin";
  cfg["password"]     = "secret";
  cfg["class-filter"] = 10;
  cfg["class-mask"]   = 0xffff;
  ASSERT_TRUE(ws2.initFromJson(cfg.dump()));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, ws2.connect());

  {
    // The server can read the filter it gets
    std::lock_guard<std::mutex> lock(logic.mutex);
    vscpEventFilter filter;
    memset(&filter, 0, sizeof(filter));
    ASSERT_TRUE(vscp_readFilterMaskFromJSON(&filter, logic.filter.dump()));
    EXPECT_EQ(10, filter.filter_class);
    EXPECT_EQ(0xffff, filter.mask_class);
  }

  server.sendText(ws2Event(20, 1));
  server.sendText(ws2Event(10, 6));
  vscpEventEx ex;
  ASSERT_EQ(VSCP_ERROR_SUCCESS, ws2.receiveBlocking(ex, 2000));
  EXPECT_EQ(10, ex.vscp_class);
  EXPECT_EQ(VSCP_ERROR_TIMEOUT, ws2.receiveBlocking(ex, 100));
}

TEST(VscpClientWs2, ConnectionLost)
{
  Ws2Server logic;
  FakeWsServer server([&logic](const std::string &msg) { return logic.handle(msg); }, { greeting() });

  vscpClientWs2 ws2;
  ASSERT_TRUE(ws2.initFromJson("{\"url\":\"" + server.url("/ws2") + "\",\"user\":\"admin\",\"password\":\"secret\"}"));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, ws2.connect());

  server.closeConnection();
  for (int i = 0; (i < 200) && ws2.isConnected(); i++) {
    usleep(10000);
  }
  EXPECT_FALSE(ws2.isConnected());
  EXPECT_EQ(VSCP_ERROR_SUCCESS, ws2.disconnect());
}
