// unittest.cpp
//
// Unit tests for vscpClientSocketCan
//
// These tests exercise the vscpClientSocketCan class without requiring
// actual SocketCAN hardware or network. Constructor defaults, JSON
// config serialization/parsing, getters/setters, filter, receive queue,
// version, capabilities, and callbacks are all tested locally.
//

#include <gtest/gtest.h>

#include <vscp-client-socketcan.h>
#include <vscphelper.h>

#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

// ---------------------------------------------------------------------------
//                       Constructor / Default State
// ---------------------------------------------------------------------------

TEST(VscpClientSocketCan, ConstructorDefaults)
{
  vscpClientSocketCan client;
  EXPECT_EQ(CVscpClient::connType::SOCKETCAN, client.getType());
  EXPECT_FALSE(client.isConnected());
  EXPECT_TRUE(client.m_bRun);
  EXPECT_FALSE(client.m_bDebug);
  EXPECT_EQ("vcan0", client.m_interface);
  EXPECT_EQ(0u, client.m_flags);
  EXPECT_EQ(-1, client.m_socket); // Not open
  EXPECT_EQ(CAN_MTU, client.m_mode);
}

TEST(VscpClientSocketCan, ConstructorNotConnected)
{
  vscpClientSocketCan client;
  EXPECT_FALSE(client.isConnected());
}

TEST(VscpClientSocketCan, ReceiveQueueInitiallyEmpty)
{
  vscpClientSocketCan client;
  EXPECT_TRUE(client.m_receiveList.empty());
}

TEST(VscpClientSocketCan, DefaultGuidAllZeros)
{
  vscpClientSocketCan client;
  cguid zeroGuid;
  zeroGuid.getFromString("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00");
  EXPECT_TRUE(client.m_guid.isSameGUID(zeroGuid.getGUID()));
}

// ---------------------------------------------------------------------------
//                           init()
// ---------------------------------------------------------------------------

TEST(VscpClientSocketCan, InitSetsMembers)
{
  vscpClientSocketCan client;
  int rv = client.init("can0", "FF:FF:FF:FF:FF:FF:FF:FE:00:00:00:00:00:00:00:01", 0x80000000, 5);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
  EXPECT_EQ("can0", client.m_interface);
  EXPECT_EQ(0x80000000u, client.m_flags);

  cguid expectedGuid;
  expectedGuid.getFromString("FF:FF:FF:FF:FF:FF:FF:FE:00:00:00:00:00:00:00:01");
  EXPECT_TRUE(client.m_guid.isSameGUID(expectedGuid.getGUID()));
}

// ---------------------------------------------------------------------------
//                           Version
// ---------------------------------------------------------------------------

TEST(VscpClientSocketCan, GetVersion)
{
  vscpClientSocketCan client;
  uint8_t major = 0xff, minor = 0xff, release = 0xff, build = 0xff;
  int rv = client.getversion(&major, &minor, &release, &build);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
  // Parameters are unused in the stub implementation
}

TEST(VscpClientSocketCan, GetVersionNullPointers)
{
  vscpClientSocketCan client;
  // Should not crash with nullptr
  int rv = client.getversion(nullptr, nullptr, nullptr, nullptr);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
}

// ---------------------------------------------------------------------------
//                          Interfaces
// ---------------------------------------------------------------------------

TEST(VscpClientSocketCan, GetInterfaces)
{
  vscpClientSocketCan client;
  std::deque<std::string> iflist;
  iflist.push_back("dummy");
  int rv = client.getinterfaces(iflist);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
  // The stub does not modify the list
  EXPECT_EQ(1u, iflist.size());
}

// ---------------------------------------------------------------------------
//                         Capabilities
// ---------------------------------------------------------------------------

TEST(VscpClientSocketCan, GetWcyd)
{
  vscpClientSocketCan client;
  uint64_t wcyd = 0xFFFF;
  int rv        = client.getwcyd(wcyd);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
  EXPECT_EQ(0u, wcyd);
}

// ---------------------------------------------------------------------------
//                       getcount / clear
// ---------------------------------------------------------------------------

TEST(VscpClientSocketCan, GetcountEmpty)
{
  vscpClientSocketCan client;
  uint16_t count = 0xFFFF;
  int rv         = client.getcount(&count);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
  EXPECT_EQ(0u, count);
}

TEST(VscpClientSocketCan, ClearEmptyQueue)
{
  vscpClientSocketCan client;
  int rv = client.clear();
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
}

// ---------------------------------------------------------------------------
//                    Receive on empty queue
// ---------------------------------------------------------------------------

TEST(VscpClientSocketCan, ReceiveEmptyQueueEvent)
{
  vscpClientSocketCan client;
  vscpEvent ev;
  memset(&ev, 0, sizeof(ev));
  int rv = client.receive(ev);
  EXPECT_EQ(VSCP_ERROR_FIFO_EMPTY, rv);
}

TEST(VscpClientSocketCan, ReceiveEmptyQueueEventEx)
{
  vscpClientSocketCan client;
  vscpEventEx ex;
  memset(&ex, 0, sizeof(ex));
  int rv = client.receive(ex);
  EXPECT_EQ(VSCP_ERROR_FIFO_EMPTY, rv);
}

TEST(VscpClientSocketCan, ReceiveEmptyQueueCanal)
{
  vscpClientSocketCan client;
  canalMsg msg;
  memset(&msg, 0, sizeof(msg));
  int rv = client.receive(msg);
  EXPECT_EQ(VSCP_ERROR_FIFO_EMPTY, rv);
}

// ---------------------------------------------------------------------------
//                   Receive blocking (timeout)
// ---------------------------------------------------------------------------

TEST(VscpClientSocketCan, ReceiveBlockingTimeout)
{
  vscpClientSocketCan client;
  vscpEvent ev;
  memset(&ev, 0, sizeof(ev));
  int rv = client.receiveBlocking(ev, 10);
  EXPECT_EQ(VSCP_ERROR_TIMEOUT, rv);
}

TEST(VscpClientSocketCan, ReceiveBlockingExTimeout)
{
  vscpClientSocketCan client;
  vscpEventEx ex;
  memset(&ex, 0, sizeof(ex));
  int rv = client.receiveBlocking(ex, 10);
  EXPECT_EQ(VSCP_ERROR_TIMEOUT, rv);
}

TEST(VscpClientSocketCan, ReceiveBlockingCanalTimeout)
{
  vscpClientSocketCan client;
  canalMsg msg;
  memset(&msg, 0, sizeof(msg));
  int rv = client.receiveBlocking(msg, 10);
  EXPECT_EQ(VSCP_ERROR_TIMEOUT, rv);
}

// ---------------------------------------------------------------------------
//                    Send not connected
// ---------------------------------------------------------------------------

TEST(VscpClientSocketCan, SendEventNotConnected)
{
  vscpClientSocketCan client;
  vscpEvent ev;
  memset(&ev, 0, sizeof(ev));
  ev.vscp_class = VSCP_CLASS1_MEASUREMENT;
  ev.vscp_type  = 6;
  ev.sizeData   = 0;
  ev.pdata      = nullptr;
  // Socket is 0 so send should fail
  int rv = client.send(ev);
  EXPECT_EQ(VSCP_ERROR_WRITE_ERROR, rv);
}

TEST(VscpClientSocketCan, SendEventExNotConnected)
{
  vscpClientSocketCan client;
  vscpEventEx ex;
  memset(&ex, 0, sizeof(ex));
  ex.vscp_class = VSCP_CLASS1_MEASUREMENT;
  ex.vscp_type  = 6;
  ex.sizeData   = 0;
  // Socket is 0 so send should fail
  int rv = client.send(ex);
  EXPECT_EQ(VSCP_ERROR_WRITE_ERROR, rv);
}

TEST(VscpClientSocketCan, SendCanalMsgNotConnected)
{
  vscpClientSocketCan client;
  canalMsg msg;
  memset(&msg, 0, sizeof(msg));
  // Socket is 0 so send should fail
  int rv = client.send(msg);
  EXPECT_EQ(VSCP_ERROR_WRITE_ERROR, rv);
}

// ---------------------------------------------------------------------------
//                            setfilter
// ---------------------------------------------------------------------------

TEST(VscpClientSocketCan, SetFilter)
{
  vscpClientSocketCan client;
  vscpEventFilter filter;
  memset(&filter, 0, sizeof(filter));
  filter.filter_priority = 3;
  filter.filter_class    = 10;
  filter.filter_type     = 6;
  int rv = client.setfilter(filter);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
}

// ---------------------------------------------------------------------------
//                      Connection / Response timeout
// ---------------------------------------------------------------------------

TEST(VscpClientSocketCan, ConnectionTimeout)
{
  vscpClientSocketCan client;
  client.setConnectionTimeout(5000);
  EXPECT_EQ(0u, client.getConnectionTimeout());
}

TEST(VscpClientSocketCan, ResponseTimeout)
{
  vscpClientSocketCan client;
  client.setResponseTimeout(5000);
  EXPECT_EQ(0u, client.getResponseTimeout());
}

// ---------------------------------------------------------------------------
//                         ConnType / Name
// ---------------------------------------------------------------------------

TEST(VscpClientSocketCan, ConnTypeIsSocketCan)
{
  vscpClientSocketCan client;
  EXPECT_EQ(CVscpClient::connType::SOCKETCAN, client.getType());
}

TEST(VscpClientSocketCan, SetGetName)
{
  vscpClientSocketCan client;
  client.setName("test-socketcan");
  EXPECT_EQ("test-socketcan", client.getName());
}

TEST(VscpClientSocketCan, FullLevel2Flag)
{
  vscpClientSocketCan client;
  EXPECT_FALSE(client.isFullLevel2());
  client.setFullLevel2(true);
  EXPECT_TRUE(client.isFullLevel2());
}

// ---------------------------------------------------------------------------
//                         setConnected
// ---------------------------------------------------------------------------

TEST(VscpClientSocketCan, SetConnected)
{
  vscpClientSocketCan client;
  EXPECT_FALSE(client.isConnected());
  client.setConnected(true);
  EXPECT_TRUE(client.isConnected());
  client.setConnected(false);
  EXPECT_FALSE(client.isConnected());
}

// ---------------------------------------------------------------------------
//                         initFromJson
// ---------------------------------------------------------------------------

TEST(VscpClientSocketCan, InitFromJsonBasic)
{
  vscpClientSocketCan client;
  json j;
  j["device"] = "can1";
  j["flags"]  = 42;
  EXPECT_TRUE(client.initFromJson(j.dump()));
  EXPECT_EQ("can1", client.m_interface);
  EXPECT_EQ(42u, client.m_flags);
}

TEST(VscpClientSocketCan, InitFromJsonInvalidJson)
{
  vscpClientSocketCan client;
  EXPECT_FALSE(client.initFromJson("not json at all"));
}

TEST(VscpClientSocketCan, InitFromJsonEmptyObject)
{
  vscpClientSocketCan client;
  std::string origInterface = client.m_interface;
  uint32_t origFlags        = client.m_flags;
  EXPECT_TRUE(client.initFromJson("{}"));
  EXPECT_EQ(origInterface, client.m_interface);
  EXPECT_EQ(origFlags, client.m_flags);
}

TEST(VscpClientSocketCan, InitFromJsonFilter)
{
  vscpClientSocketCan client;
  json j;
  json filt;
  filt["in-filter"] = "1,10,6,00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00";
  filt["in-mask"]   = "1,0xffff,0xff,00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00";
  j["filter"]       = filt;
  EXPECT_TRUE(client.initFromJson(j.dump()));
  EXPECT_EQ(10, client.m_filterIn.filter_class);
  EXPECT_EQ(6, client.m_filterIn.filter_type);
  EXPECT_EQ(0xffff, client.m_filterIn.mask_class);
  EXPECT_EQ(0xff, client.m_filterIn.mask_type);
}

TEST(VscpClientSocketCan, InitFromJsonOutFilter)
{
  vscpClientSocketCan client;
  json j;
  json filt;
  filt["in-filter"]  = "0,20,3,00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00";
  filt["out-filter"] = "0,30,4,00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00";
  filt["out-mask"]   = "0,0xffff,0xff,00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00";
  j["filter"]        = filt;
  EXPECT_TRUE(client.initFromJson(j.dump()));
  EXPECT_EQ(20, client.m_filterIn.filter_class);
  EXPECT_EQ(30, client.m_filterOut.filter_class); // "out-filter", not "in-filter"
  EXPECT_EQ(4, client.m_filterOut.filter_type);
  EXPECT_EQ(0xffff, client.m_filterOut.mask_class);
  EXPECT_EQ(0xff, client.m_filterOut.mask_type);
}

// CAN id filters as written by VSCP Works
TEST(VscpClientSocketCan, InitFromJsonCanFilters)
{
  vscpClientSocketCan client;
  json j;
  j["device"]  = "vcan0";
  j["filters"] = json::array({
    { { "name", "class 10" }, { "filter", 0x000a0000 }, { "mask", 0x01ff0000 }, { "binvert", false } },
    { { "name", "not node 2" }, { "filter", "0x02" }, { "mask", "0xff" }, { "binvert", true } },
  });
  EXPECT_TRUE(client.initFromJson(j.dump()));
  ASSERT_EQ(2u, client.m_canFilters.size());
  EXPECT_EQ(0x000a0000u, client.m_canFilters[0].first);
  EXPECT_EQ(0x01ff0000u, client.m_canFilters[0].second);
  EXPECT_EQ(0x02u | CAN_INV_FILTER, client.m_canFilters[1].first);
  EXPECT_EQ(0xffu, client.m_canFilters[1].second);
}

TEST(VscpClientSocketCan, SetFilterIsUsedForEvents)
{
  vscpClientSocketCan client;
  vscpEventFilter filter;
  vscp_clearVSCPFilter(&filter);
  filter.filter_class = 10;
  filter.mask_class   = 0xffff;
  EXPECT_EQ(VSCP_ERROR_SUCCESS, client.setfilter(filter));
  EXPECT_EQ(10, client.m_filterIn.filter_class);
  EXPECT_EQ(0xffff, client.m_filterIn.mask_class);
}

// ---------------------------------------------------------------------------
//                        getConfigAsJson
// ---------------------------------------------------------------------------

// NOTE: getConfigAsJson has a known bug — it returns empty string (rv)
// instead of j.dump(). We test the current behavior.
TEST(VscpClientSocketCan, GetConfigAsJsonReturnsString)
{
  vscpClientSocketCan client;
  std::string config = client.getConfigAsJson();
  // Current implementation returns empty string due to bug
  EXPECT_TRUE(config.empty());
}

// ---------------------------------------------------------------------------
//                          Callbacks
// ---------------------------------------------------------------------------

static bool g_evCallbackFired  = false;
static bool g_exCallbackFired  = false;

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

TEST(VscpClientSocketCan, SetCallbackEv)
{
  vscpClientSocketCan client;
  g_evCallbackFired = false;
  int rv            = client.setCallbackEv(testCallbackEv, nullptr);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
}

TEST(VscpClientSocketCan, SetCallbackEx)
{
  vscpClientSocketCan client;
  g_exCallbackFired = false;
  int rv            = client.setCallbackEx(testCallbackEx, nullptr);
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
}

TEST(VscpClientSocketCan, SetCallbackEvWhenConnectedFails)
{
  vscpClientSocketCan client;
  client.setConnected(true);
  int rv = client.setCallbackEv(testCallbackEv, nullptr);
  EXPECT_EQ(VSCP_ERROR_ERROR, rv);
}

TEST(VscpClientSocketCan, SetCallbackExWhenConnectedFails)
{
  vscpClientSocketCan client;
  client.setConnected(true);
  int rv = client.setCallbackEx(testCallbackEx, nullptr);
  EXPECT_EQ(VSCP_ERROR_ERROR, rv);
}

TEST(VscpClientSocketCan, SendToCallbacksFiresEv)
{
  vscpClientSocketCan client;
  g_evCallbackFired = false;
  client.setCallbackEv(testCallbackEv, nullptr);

  vscpEvent ev;
  memset(&ev, 0, sizeof(ev));
  ev.pdata = nullptr;

  client.sendToCallbacks(&ev);
  EXPECT_TRUE(g_evCallbackFired);
}

TEST(VscpClientSocketCan, SendToCallbacksFiresEx)
{
  vscpClientSocketCan client;
  g_exCallbackFired = false;
  client.setCallbackEx(testCallbackEx, nullptr);

  vscpEvent ev;
  memset(&ev, 0, sizeof(ev));
  ev.pdata = nullptr;

  client.sendToCallbacks(&ev);
  EXPECT_TRUE(g_exCallbackFired);
}

// ---------------------------------------------------------------------------
//                       CAN DLC helpers
// ---------------------------------------------------------------------------

TEST(VscpClientSocketCan, FlagConstants)
{
  // Use temporaries to avoid ODR-use linker issues with static const members
  uint32_t debug_flag   = vscpClientSocketCan::FLAG_ENABLE_DEBUG;
  uint32_t fd_flag      = vscpClientSocketCan::FLAG_FD_MODE;
  uint32_t resp_timeout = vscpClientSocketCan::DEAULT_RESPONSE_TIMEOUT;
  EXPECT_EQ(0x80000000u, debug_flag);
  EXPECT_EQ(0x4000000u, fd_flag);
  EXPECT_EQ(3u, resp_timeout);
}

// ---------------------------------------------------------------------------
//                     Default filter state
// ---------------------------------------------------------------------------

TEST(VscpClientSocketCan, DefaultFiltersCleared)
{
  vscpClientSocketCan client;

  // In filter should be all zeros (accept all)
  EXPECT_EQ(0, client.m_filterIn.filter_priority);
  EXPECT_EQ(0, client.m_filterIn.filter_class);
  EXPECT_EQ(0, client.m_filterIn.filter_type);
  EXPECT_EQ(0, client.m_filterIn.mask_priority);
  EXPECT_EQ(0, client.m_filterIn.mask_class);
  EXPECT_EQ(0, client.m_filterIn.mask_type);

  // Out filter should also be all zeros
  EXPECT_EQ(0, client.m_filterOut.filter_priority);
  EXPECT_EQ(0, client.m_filterOut.filter_class);
  EXPECT_EQ(0, client.m_filterOut.filter_type);
  EXPECT_EQ(0, client.m_filterOut.mask_priority);
  EXPECT_EQ(0, client.m_filterOut.mask_class);
  EXPECT_EQ(0, client.m_filterOut.mask_type);
}

// ---------------------------------------------------------------------------
//                       Mode member
// ---------------------------------------------------------------------------

TEST(VscpClientSocketCan, DefaultModeCAN_MTU)
{
  vscpClientSocketCan client;
  EXPECT_EQ(CAN_MTU, client.m_mode);
}

TEST(VscpClientSocketCan, ModeCanBeSetToFD)
{
  vscpClientSocketCan client;
  client.m_mode = CANFD_MTU;
  EXPECT_EQ(CANFD_MTU, client.m_mode);
}

// ---------------------------------------------------------------------------
//                 Disconnect when not connected
// ---------------------------------------------------------------------------

TEST(VscpClientSocketCan, DisconnectWhenNotRunning)
{
  vscpClientSocketCan client;
  client.m_bRun = false; // Already stopped
  int rv        = client.disconnect();
  EXPECT_EQ(VSCP_ERROR_SUCCESS, rv);
}

// ---------------------------------------------------------------------------
//          Filters on a real (virtual) CAN interface, vcan0
// ---------------------------------------------------------------------------

#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

namespace {

// Raw socket on vcan0 to send test frames, -1 if there is no vcan0
int
openVcan0(void)
{
  int s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if (s < 0) {
    return -1;
  }
  struct ifreq ifr = {};
  strncpy(ifr.ifr_name, "vcan0", IFNAMSIZ - 1);
  if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
    close(s);
    return -1;
  }
  struct sockaddr_can addr = {};
  addr.can_family          = AF_CAN;
  addr.can_ifindex         = ifr.ifr_ifindex;
  if (bind(s, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
    close(s);
    return -1;
  }
  return s;
}

void
sendFrame(int s, uint16_t vscp_class, uint8_t vscp_type, uint8_t nickname, uint8_t tag)
{
  struct can_frame frame = {};
  frame.can_id  = (((uint32_t) vscp_class << 16) | ((uint32_t) vscp_type << 8) | nickname) | CAN_EFF_FLAG;
  frame.can_dlc = 1;
  frame.data[0] = tag;
  ASSERT_EQ((ssize_t) sizeof(frame), write(s, &frame, sizeof(frame)));
}

// Tags of the received events
std::string
receiveTags(vscpClientSocketCan &client)
{
  std::string got;
  for (int i = 0; i < 20; i++) {
    vscpEvent ev = {};
    if (VSCP_ERROR_SUCCESS == client.receiveBlocking(ev, 50)) {
      got += std::to_string(ev.sizeData ? ev.pdata[0] : 255) + " ";
      delete[] ev.pdata;
    }
  }
  return got;
}

std::string
runOnVcan0(const json &config, const vscpEventFilter *pfilter = nullptr)
{
  vscpClientSocketCan client;
  EXPECT_TRUE(client.initFromJson(config.dump()));
  if (pfilter) {
    vscpEventFilter f = *pfilter;
    client.setfilter(f);
  }
  EXPECT_EQ(VSCP_ERROR_SUCCESS, client.connect());
  usleep(200000); // Worker opens and binds the socket

  int s = openVcan0();
  sendFrame(s, 10, 6, 1, 0);
  sendFrame(s, 20, 3, 2, 1);
  sendFrame(s, 10, 1, 3, 2);
  close(s);

  std::string got = receiveTags(client);
  client.disconnect();
  return got;
}

} // namespace

TEST(VscpClientSocketCanVcan, NoFilterReceivesAll)
{
  int s = openVcan0();
  if (s < 0) {
    GTEST_SKIP() << "No vcan0 interface";
  }
  close(s);
  json j = { { "device", "vcan0" }, { "response-timeout", 50 } };
  EXPECT_EQ("0 1 2 ", runOnVcan0(j));
}

TEST(VscpClientSocketCanVcan, CanIdFilters)
{
  int s = openVcan0();
  if (s < 0) {
    GTEST_SKIP() << "No vcan0 interface";
  }
  close(s);

  // Class 10 only
  json j = { { "device", "vcan0" },
             { "response-timeout", 50 },
             { "filters", json::array({ { { "filter", 0x000a0000 }, { "mask", 0x01ff0000 } } }) } };
  EXPECT_EQ("0 2 ", runOnVcan0(j));

  // Everything except node 2 (inverted)
  j["filters"] = json::array({ { { "filter", 2 }, { "mask", 0xff }, { "binvert", true } } });
  EXPECT_EQ("0 2 ", runOnVcan0(j));

  // Two filters: node 1 or node 2
  j["filters"] = json::array({ { { "filter", 1 }, { "mask", 0xff } }, { { "filter", 2 }, { "mask", 0xff } } });
  EXPECT_EQ("0 1 ", runOnVcan0(j));
}

TEST(VscpClientSocketCanVcan, EventFilter)
{
  int s = openVcan0();
  if (s < 0) {
    GTEST_SKIP() << "No vcan0 interface";
  }
  close(s);

  vscpEventFilter filter;
  vscp_clearVSCPFilter(&filter);
  filter.filter_type = 3;
  filter.mask_type   = 0xff;
  json j = { { "device", "vcan0" }, { "response-timeout", 50 } };
  EXPECT_EQ("1 ", runOnVcan0(j, &filter));
}
