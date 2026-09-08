///////////////////////////////////////////////////////////////////////////////
// unittest.cpp
//
// Unit tests for CVscpClient base class
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
#include <functional>

#include <gtest/gtest.h>

#include <vscp-client-base.h>
#include <vscphelper.h>

// ============================================================================
//  Concrete test double — implements all pure virtuals as stubs
// ============================================================================

class TestClient : public CVscpClient {
public:
  TestClient() : CVscpClient() {
    m_type = CVscpClient::connType::NONE;
  }

  int connect(void) override { return VSCP_ERROR_SUCCESS; }
  int disconnect(void) override { return VSCP_ERROR_SUCCESS; }
  bool isConnected(void) override { return false; }

  int send(vscpEvent &ev) override { return VSCP_ERROR_SUCCESS; }
  int send(canalMsg &msg) override { return VSCP_ERROR_SUCCESS; }
  int send(vscpEventEx &ex) override { return VSCP_ERROR_SUCCESS; }

  int receive(vscpEvent &ev) override { return VSCP_ERROR_SUCCESS; }
  int receive(vscpEventEx &ex) override { return VSCP_ERROR_SUCCESS; }
  int receive(canalMsg &msg) override { return VSCP_ERROR_SUCCESS; }

  int receiveBlocking(vscpEvent &ev, long timeout = 100) override { return VSCP_ERROR_SUCCESS; }
  int receiveBlocking(vscpEventEx &ex, long timeout = 100) override { return VSCP_ERROR_SUCCESS; }
  int receiveBlocking(canalMsg &msg, long timeout = 100) override { return VSCP_ERROR_SUCCESS; }

  int setfilter(vscpEventFilter &filter) override { return VSCP_ERROR_SUCCESS; }
  int getcount(uint16_t *pcount) override { *pcount = 0; return VSCP_ERROR_SUCCESS; }
  int clear(void) override { return VSCP_ERROR_SUCCESS; }
  int getversion(uint8_t *pmajor, uint8_t *pminor, uint8_t *prelease, uint8_t *pbuild) override {
    *pmajor = 1; *pminor = 0; *prelease = 0; *pbuild = 0;
    return VSCP_ERROR_SUCCESS;
  }
  int getinterfaces(std::deque<std::string> &iflist) override { return VSCP_ERROR_SUCCESS; }
  int getwcyd(uint64_t &wcyd) override { wcyd = 0; return VSCP_ERROR_SUCCESS; }

  void setConnectionTimeout(uint32_t timeout) override { m_connTimeout = timeout; }
  uint32_t getConnectionTimeout(void) override { return m_connTimeout; }
  void setResponseTimeout(uint32_t timeout) override { m_respTimeout = timeout; }
  uint32_t getResponseTimeout(void) override { return m_respTimeout; }

  std::string getConfigAsJson(void) override { return "{}"; }
  bool initFromJson(const std::string &config) override { return true; }

private:
  uint32_t m_connTimeout = 0;
  uint32_t m_respTimeout = 0;
};

// ============================================================================
//                          Constructor / defaults
// ============================================================================

TEST(CVscpClientBase, DefaultConstructorState)
{
  TestClient client;

  // Callbacks should be inactive
  EXPECT_FALSE(client.isCallbackEvActive());
  EXPECT_FALSE(client.isCallbackExActive());

  // Callback object should be null
  EXPECT_EQ(nullptr, client.getCallbackObj());

  // Should not be full level II by default
  EXPECT_FALSE(client.isFullLevel2());

  // Type should be NONE
  EXPECT_EQ(CVscpClient::connType::NONE, client.getType());

  // Name should be empty
  EXPECT_TRUE(client.getName().empty());
}

// ============================================================================
//                         Name getter/setter
// ============================================================================

TEST(CVscpClientBase, SetAndGetName)
{
  TestClient client;

  client.setName("test-connection");
  EXPECT_EQ("test-connection", client.getName());

  client.setName("");
  EXPECT_TRUE(client.getName().empty());

  client.setName("another name with spaces");
  EXPECT_EQ("another name with spaces", client.getName());
}

// ============================================================================
//                         Full Level II flag
// ============================================================================

TEST(CVscpClientBase, FullLevel2Flag)
{
  TestClient client;

  EXPECT_FALSE(client.isFullLevel2());

  client.setFullLevel2(true);
  EXPECT_TRUE(client.isFullLevel2());

  client.setFullLevel2(false);
  EXPECT_FALSE(client.isFullLevel2());

  // Default argument should be true
  client.setFullLevel2();
  EXPECT_TRUE(client.isFullLevel2());
}

// ============================================================================
//                         Callback object
// ============================================================================

TEST(CVscpClientBase, CallbackObjectPointer)
{
  TestClient client;

  EXPECT_EQ(nullptr, client.getCallbackObj());

  int userData = 42;
  client.setCallbackObj(&userData);
  EXPECT_EQ(&userData, client.getCallbackObj());

  client.setCallbackObj(nullptr);
  EXPECT_EQ(nullptr, client.getCallbackObj());
}

// ============================================================================
//                       Callback Ev active flag
// ============================================================================

TEST(CVscpClientBase, CallbackEvActiveFlag)
{
  TestClient client;

  EXPECT_FALSE(client.isCallbackEvActive());

  client.setCallbackEvActive(true);
  EXPECT_TRUE(client.isCallbackEvActive());

  client.setCallbackEvActive(false);
  EXPECT_FALSE(client.isCallbackEvActive());

  // Default argument should be true
  client.setCallbackEvActive();
  EXPECT_TRUE(client.isCallbackEvActive());
}

// ============================================================================
//                       Callback Ex active flag
// ============================================================================

TEST(CVscpClientBase, CallbackExActiveFlag)
{
  TestClient client;

  EXPECT_FALSE(client.isCallbackExActive());

  client.setCallbackExActive(true);
  EXPECT_TRUE(client.isCallbackExActive());

  client.setCallbackExActive(false);
  EXPECT_FALSE(client.isCallbackExActive());

  // Default argument should be true
  client.setCallbackExActive();
  EXPECT_TRUE(client.isCallbackExActive());
}

// ============================================================================
//                      setCallbackEv / setCallbackEx
// ============================================================================

TEST(CVscpClientBase, SetCallbackEv)
{
  TestClient client;
  int callCount = 0;

  EXPECT_FALSE(client.isCallbackEvActive());

  int rv = client.setCallbackEv(
    [&callCount](vscpEvent &ev, void *pobj) {
      callCount++;
    },
    nullptr);

  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
  EXPECT_TRUE(client.isCallbackEvActive());

  // Invoke the callback to verify it was stored
  vscpEvent ev;
  memset(&ev, 0, sizeof(ev));
  client.m_callbackev(ev, nullptr);
  EXPECT_EQ(1, callCount);
}

TEST(CVscpClientBase, SetCallbackEx)
{
  TestClient client;
  int callCount = 0;

  EXPECT_FALSE(client.isCallbackExActive());

  int rv = client.setCallbackEx(
    [&callCount](vscpEventEx &ex, void *pobj) {
      callCount++;
    },
    nullptr);

  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
  EXPECT_TRUE(client.isCallbackExActive());

  // Invoke the callback to verify it was stored
  vscpEventEx ex;
  memset(&ex, 0, sizeof(ex));
  client.m_callbackex(ex, nullptr);
  EXPECT_EQ(1, callCount);
}

TEST(CVscpClientBase, SetCallbackEvWithUserData)
{
  TestClient client;
  int userData      = 99;
  void *receivedObj = nullptr;

  int rv = client.setCallbackEv(
    [&receivedObj](vscpEvent &ev, void *pobj) {
      receivedObj = pobj;
    },
    &userData);

  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);

  // The callback object should be set
  EXPECT_EQ(&userData, client.getCallbackObj());

  // Invoke — pobj in the callback is up to the caller to pass,
  // but setCallbackEv stores it in m_callbackObject
  vscpEvent ev;
  memset(&ev, 0, sizeof(ev));
  client.m_callbackev(ev, client.getCallbackObj());
  EXPECT_EQ(&userData, receivedObj);
}

TEST(CVscpClientBase, SetCallbackExWithUserData)
{
  TestClient client;
  double userData   = 3.14;
  void *receivedObj = nullptr;

  int rv = client.setCallbackEx(
    [&receivedObj](vscpEventEx &ex, void *pobj) {
      receivedObj = pobj;
    },
    &userData);

  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
  EXPECT_EQ(&userData, client.getCallbackObj());

  vscpEventEx ex;
  memset(&ex, 0, sizeof(ex));
  client.m_callbackex(ex, client.getCallbackObj());
  EXPECT_EQ(&userData, receivedObj);
}

// ============================================================================
//                       connType enum values
// ============================================================================

TEST(CVscpClientBase, ConnTypeEnumValues)
{
  // Verify the enum values are distinct
  EXPECT_NE(CVscpClient::connType::NONE, CVscpClient::connType::TCPIP);
  EXPECT_NE(CVscpClient::connType::TCPIP, CVscpClient::connType::CANAL);
  EXPECT_NE(CVscpClient::connType::CANAL, CVscpClient::connType::SOCKETCAN);
  EXPECT_NE(CVscpClient::connType::SOCKETCAN, CVscpClient::connType::WS1);
  EXPECT_NE(CVscpClient::connType::WS1, CVscpClient::connType::WS2);
  EXPECT_NE(CVscpClient::connType::WS2, CVscpClient::connType::MQTT);
  EXPECT_NE(CVscpClient::connType::MQTT, CVscpClient::connType::UDP);
  EXPECT_NE(CVscpClient::connType::UDP, CVscpClient::connType::MULTICAST);

  // Verify NONE is 0
  EXPECT_EQ(0, static_cast<int>(CVscpClient::connType::NONE));
}

// ============================================================================
//             Callback replacement (overwriting a previous callback)
// ============================================================================

TEST(CVscpClientBase, CallbackEvCanBeReplaced)
{
  TestClient client;
  int counter1 = 0;
  int counter2 = 0;

  client.setCallbackEv(
    [&counter1](vscpEvent &ev, void *pobj) { counter1++; }, nullptr);

  vscpEvent ev;
  memset(&ev, 0, sizeof(ev));
  client.m_callbackev(ev, nullptr);
  EXPECT_EQ(1, counter1);
  EXPECT_EQ(0, counter2);

  // Replace callback
  client.setCallbackEv(
    [&counter2](vscpEvent &ev, void *pobj) { counter2++; }, nullptr);

  client.m_callbackev(ev, nullptr);
  EXPECT_EQ(1, counter1); // old callback not called again
  EXPECT_EQ(1, counter2);
}

TEST(CVscpClientBase, CallbackExCanBeReplaced)
{
  TestClient client;
  int counter1 = 0;
  int counter2 = 0;

  client.setCallbackEx(
    [&counter1](vscpEventEx &ex, void *pobj) { counter1++; }, nullptr);

  vscpEventEx ex;
  memset(&ex, 0, sizeof(ex));
  client.m_callbackex(ex, nullptr);
  EXPECT_EQ(1, counter1);
  EXPECT_EQ(0, counter2);

  // Replace callback
  client.setCallbackEx(
    [&counter2](vscpEventEx &ex, void *pobj) { counter2++; }, nullptr);

  client.m_callbackex(ex, nullptr);
  EXPECT_EQ(1, counter1);
  EXPECT_EQ(1, counter2);
}

// ============================================================================
//           SetCallbackEv/Ex overwrite each other's callback object
// ============================================================================

TEST(CVscpClientBase, SetCallbackOverwritesCallbackObject)
{
  TestClient client;
  int objA = 1;
  int objB = 2;

  client.setCallbackEv(
    [](vscpEvent &ev, void *pobj) {}, &objA);
  EXPECT_EQ(&objA, client.getCallbackObj());

  // Setting Ex callback overwrites the callback object
  client.setCallbackEx(
    [](vscpEventEx &ex, void *pobj) {}, &objB);
  EXPECT_EQ(&objB, client.getCallbackObj());
}
