// unit_test.cpp
//
// Tests for the userlist and useritem classes

#include <clientlist.h>
#include <gtest/gtest.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//-----------------------------------------------------------------------------
TEST(ClientList, setUserName_getName_fixName)
{
    CUserItem item;

    item.setUserName("MrSmith");
    ASSERT_EQ("MrSmith", item.getUserName());

    item.setUserName("Mr Smith");
    item.fixName();
    ASSERT_EQ("Mr_Smith", item.getUserName());

    item.setUserName("Mr 1\"and\'Smith,t");
    item.fixName();
    ASSERT_EQ("Mr_1_and_Smith_t", item.getUserName());

    // uint8_t iv4[5] = {0xae,0xC6,0xA7,0xE2,0xA4};
    // ASSERT_EQ(-21489.320312f, vscp_getMeasurementAsFloat(iv4, 5));
    // ASSERT_NE(-21489.320312, vscp_getMeasurementAsFloat(iv4, 5));
    // ASSERT_NE(-1, vscp_getMeasurementAsFloat(iv4, 5));
    // ASSERT_NE(0, vscp_getMeasurementAsFloat(iv4, 5));
}

//-----------------------------------------------------------------------------
TEST(ClientList, setUserID_getUserID)
{
    CUserItem item;

    item.setUserID(0);
    ASSERT_EQ(0, item.getUserID());

    item.setUserID(0xffffffff);
    ASSERT_EQ(0xffffffff, item.getUserID());
}

//-----------------------------------------------------------------------------
// TEST(ClientList, setPassword_getPassword)
// {
//     CUserItem item;

//     item.setUserName("MrCool");
//     item.setPassword("Very secret this password is");
//     ASSERT_EQ("MrCool:Very secret this password is", item.getPassword());
// }

//-----------------------------------------------------------------------------
TEST(ClientList, setFullname_getFullname)
{
    CUserItem item;

    item.setFullname("Mr Smith");
    ASSERT_EQ("Mr Smith", item.getFullname());
}

//-----------------------------------------------------------------------------
TEST(ClientList, setNote_getNote)
{
    CUserItem item;

    item.setNote("This is some text describing this user");
    ASSERT_EQ("This is some text describing this user", item.getNote());
}

//-----------------------------------------------------------------------------
TEST(ClientList, setUserRights_getUserRights)
{
    CUserItem item;

    item.setUserRights(0xffffffffffffffff);
    ASSERT_EQ(0xffffffffffffffff, item.getUserRights());
}

//-----------------------------------------------------------------------------
TEST(ClientList, getUserRights_after_create)
{
    CUserItem item;
    ASSERT_EQ(0, item.getUserRights());
}


//-----------------------------------------------------------------------------
/*
 * admin           - user get full access.
 * user            - user get standard user rights.
 * driver          - user get standard driver rights.
 * web             - user get web access.
 * rest            - user get rest access.
 * tcp             - user get tcp/ip access.
 * websockets      - user get websocket access.
 * mqtt            - user get mqtt access.
 * udp             - user get UDP access.
 * send-events     - User can send events.
 * receive-events  - User can receive events.
 * l1ctrl-events   - User can send level I control events (type=0)
 * l2ctrl-events   - User can send level II control events (type=1024)
 * hlo-events      - User can send HLO events.
 * shutdown        - User can do "shutdown".
 * restart         - User can do "RESTART"
 * interface       - User can do "interface list/..."
 * test            - user can do "test"
 */
TEST(ClientList, setUserRightsFromString_admin)
{
    CUserItem item;

    item.setUserRightsFromString("admin");
    ASSERT_EQ(0xffffffffffffffff, item.getUserRights());
}

//-----------------------------------------------------------------------------
TEST(ClientList, setUserRightsFromString_user)
{
    CUserItem item;
    item.setUserRightsFromString("user");
    ASSERT_EQ(VSCP_USER_DEFAULT_RIGHTS, item.getUserRights());
}

//-----------------------------------------------------------------------------
TEST(ClientList, setUserRightsFromString_web)
{
    CUserItem item;
    item.setUserRightsFromString("web");
    ASSERT_EQ(VSCP_WEB_DEFAULT_RIGHTS, item.getUserRights());
}

//-----------------------------------------------------------------------------
TEST(ClientList, setUserRightsFromString_rest)
{
    CUserItem item;
    item.setUserRightsFromString("rest");
    ASSERT_EQ(VSCP_REST_DEFAULT_RIGHTS, item.getUserRights());
}

//-----------------------------------------------------------------------------
TEST(ClientList, setUserRightsFromString_tcp)
{
    CUserItem item;
    item.setUserRightsFromString("tcp");
    ASSERT_EQ(VSCP_TCPIP_DEFAULT_RIGHTS, item.getUserRights());
}

//-----------------------------------------------------------------------------
TEST(ClientList, setUserRightsFromString_websockets)
{
    CUserItem item;
    item.setUserRightsFromString("websockets");
    ASSERT_EQ(VSCP_WEBSOCKETS_DEFAULT_RIGHTS, item.getUserRights());
}

//-----------------------------------------------------------------------------
TEST(ClientList, setUserRightsFromString_mqtt)
{
    CUserItem item;
    item.setUserRightsFromString("mqtt");
    ASSERT_EQ(VSCP_MQTT_DEFAULT_RIGHTS, item.getUserRights());
}

//-----------------------------------------------------------------------------
TEST(ClientList, setUserRightsFromString_udp)
{
    CUserItem item;
    item.setUserRightsFromString("udp");
    ASSERT_EQ(VSCP_UDP_DEFAULT_RIGHTS, item.getUserRights());
}

//-----------------------------------------------------------------------------
TEST(ClientList, setUserRightsFromString_send_events)
{
    CUserItem item;
    item.setUserRightsFromString("send-events");
    ASSERT_EQ(VSCP_USER_RIGHT_ALLOW_SEND_EVENT, item.getUserRights());
}

//-----------------------------------------------------------------------------
TEST(ClientList, setUserRightsFromString_receive_events)
{
    CUserItem item;
    item.setUserRightsFromString("receive-events");
    ASSERT_EQ(VSCP_USER_RIGHT_ALLOW_RCV_EVENT, item.getUserRights());
}

//-----------------------------------------------------------------------------
TEST(ClientList, setUserRightsFromString_l1ctrl_events)
{
    CUserItem item;
    item.setUserRightsFromString("l1ctrl-events");
    ASSERT_EQ(VSCP_USER_RIGHT_ALLOW_SEND_L1CTRL_EVENT, item.getUserRights());
}

//-----------------------------------------------------------------------------
TEST(ClientList, setUserRightsFromString_l2ctrl_events)
{
    CUserItem item;
    item.setUserRightsFromString("l2ctrl-events");
    ASSERT_EQ(VSCP_USER_RIGHT_ALLOW_SEND_L2CTRL_EVENT, item.getUserRights());
}

//-----------------------------------------------------------------------------
TEST(ClientList, setUserRightsFromString_hlo_events)
{
    CUserItem item;
    item.setUserRightsFromString("hlo-events");
    ASSERT_EQ(VSCP_USER_RIGHT_ALLOW_SEND_HLO_EVENT, item.getUserRights());
}

//-----------------------------------------------------------------------------
TEST(ClientList, setUserRightsFromString_set_filter)
{
    CUserItem item;
    item.setUserRightsFromString("set-filter");
    ASSERT_EQ(VSCP_USER_RIGHT_ALLOW_SETFILTER, item.getUserRights());
}

//-----------------------------------------------------------------------------
TEST(ClientList, setUserRightsFromString_set_guid)
{
    CUserItem item;
    item.setUserRightsFromString("set-guid");
    ASSERT_EQ(VSCP_USER_RIGHT_ALLOW_SETGUID, item.getUserRights());
}

//-----------------------------------------------------------------------------
TEST(ClientList, setUserRightsFromString_shutdown)
{
    CUserItem item;
    item.setUserRightsFromString("shutdown");
    ASSERT_EQ(VSCP_USER_RIGHT_ALLOW_SHUTDOWN, item.getUserRights());
}

//-----------------------------------------------------------------------------
TEST(ClientList, setUserRightsFromString_restart)
{
    CUserItem item;
    item.setUserRightsFromString("restart");
    ASSERT_EQ(VSCP_USER_RIGHT_ALLOW_RESTART, item.getUserRights());
}

//-----------------------------------------------------------------------------
TEST(ClientList, setUserRightsFromString_interface)
{
    CUserItem item;
    item.setUserRightsFromString("interface");
    ASSERT_EQ(VSCP_USER_RIGHT_ALLOW_INTERFACE, item.getUserRights());
}

//-----------------------------------------------------------------------------
TEST(ClientList, setUserRightsFromString_test)
{
    CUserItem item;
    item.setUserRightsFromString("test");
    ASSERT_EQ(VSCP_USER_RIGHT_ALLOW_TEST, item.getUserRights());
}

//-----------------------------------------------------------------------------
TEST(ClientList, setPassword)
{
    CUserItem item;

    item.setUserName("MrKoll");
    item.setPassword("Very secret this password is");

    if (0) {
        char buf[1024];
        uint8_t salt[256];
        uint8_t hash[256];
        std::string calcHash;
        printf("getPassword: %s\n", item.getPassword().c_str());

        // Use same salt gives same hash.
        vscp_makePasswordHash(calcHash,
                              "MrKoll:Very secret this password is",
                              salt);
        printf("calcHash: %s\n", calcHash.c_str());
    }

    ASSERT_EQ(true,
              vscp_isPasswordValid(item.getPassword(),
                                   "MrKoll:Very secret this password is"));
}

//-----------------------------------------------------------------------------
TEST(ClientList, getAllowedEvent_empty)
{
    CUserItem item;
    std::string str;

    ASSERT_EQ(false, item.getAllowedEvent(1,str));
}

//-----------------------------------------------------------------------------
TEST(ClientList, getAllowedEvent_three)
{
    CUserItem item;
    std::string str;

    ASSERT_EQ(true, item.addAllowedEvent("10:*"));
    ASSERT_EQ(true, item.addAllowedEvent("1040:6"));
    ASSERT_EQ(true, item.addAllowedEvent("0x1e:9"));
    ASSERT_EQ(3, item.getAllowedEventsCount());
    ASSERT_EQ(true, item.getAllowedEvent(0,str));
    ASSERT_EQ("000A:*", str);
    ASSERT_EQ(true, item.getAllowedEvent(1,str));
    ASSERT_EQ("0410:0006", str);
    ASSERT_EQ(true, item.getAllowedEvent(2,str));
    ASSERT_EQ("001E:0009", str);
    ASSERT_EQ(false, item.getAllowedEvent(3,str));
    item.setAllowedEvent(2, "11:11");
    ASSERT_EQ(true, item.getAllowedEvent(2,str));
    ASSERT_EQ("000B:000B", str);
}

//-----------------------------------------------------------------------------
TEST(ClientList, getAllowedEventsCount_empty)
{
    CUserItem item;
    std::string str;

    ASSERT_EQ(0, item.getAllowedEventsCount());
}

//-----------------------------------------------------------------------------
TEST(ClientList, isUserAllowedToSendEvent_admin)
{
    CUserItem item;

    // Everything should be allowed
    item.setUserRightsFromString("admin");

    ASSERT_EQ(true, item.addAllowedEvent("10:*"));
    ASSERT_EQ(true, item.addAllowedEvent("1040:6"));
    ASSERT_EQ(true, item.addAllowedEvent("0x1e:9"));

    ASSERT_EQ(true, item.isUserAllowedToSendEvent(10,6));
    ASSERT_EQ(true, item.isUserAllowedToSendEvent(10,112));

    ASSERT_EQ(false, item.isUserAllowedToSendEvent(12,112));

    ASSERT_EQ(false, item.isUserAllowedToSendEvent(1000,6));
    ASSERT_EQ(false, item.isUserAllowedToSendEvent(1040,7));
    ASSERT_EQ(true, item.isUserAllowedToSendEvent(1040,6));

    ASSERT_EQ(true, item.isUserAllowedToSendEvent(30,9));
}

//-----------------------------------------------------------------------------
TEST(ClientList, isUserAllowedToSendEvent_user)
{
    CUserItem item;

    // Everything should be allowed
    item.setUserRightsFromString("user");

    ASSERT_EQ(true, item.addAllowedEvent("10:*"));
    ASSERT_EQ(true, item.addAllowedEvent("1040:6"));
    ASSERT_EQ(true, item.addAllowedEvent("0x1e:9"));

    ASSERT_EQ(true, item.isUserAllowedToSendEvent(10,6));
    ASSERT_EQ(true, item.isUserAllowedToSendEvent(10,112));

    ASSERT_EQ(false, item.isUserAllowedToSendEvent(12,112));

    ASSERT_EQ(false, item.isUserAllowedToSendEvent(1000,6));
    ASSERT_EQ(false, item.isUserAllowedToSendEvent(1040,7));
    ASSERT_EQ(true, item.isUserAllowedToSendEvent(1040,6));

    ASSERT_EQ(true, item.isUserAllowedToSendEvent(30,9));
}

//-----------------------------------------------------------------------------
TEST(ClientList, isUserAllowedToSendEvent_level1_protocol)
{
    CUserItem item;

    // Everything should be allowed
    item.setUserRightsFromString("send-events/l1ctrl-events");
    
    ASSERT_EQ(VSCP_USER_RIGHT_ALLOW_SEND_EVENT | VSCP_USER_RIGHT_ALLOW_SEND_L1CTRL_EVENT,
                item.getUserRights());

    ASSERT_EQ(true, item.addAllowedEvent("0:*"));       // PROTOCOL
    ASSERT_EQ(true, item.addAllowedEvent("10:*"));
    ASSERT_EQ(true, item.addAllowedEvent("1040:6"));
    ASSERT_EQ(true, item.addAllowedEvent("0x1e:9"));

    ASSERT_EQ(true, item.isUserAllowedToSendEvent(10,6));
    ASSERT_EQ(true, item.isUserAllowedToSendEvent(10,112));

    ASSERT_EQ(false, item.isUserAllowedToSendEvent(12,112));

    ASSERT_EQ(false, item.isUserAllowedToSendEvent(1000,6));
    ASSERT_EQ(false, item.isUserAllowedToSendEvent(1040,7));
    ASSERT_EQ(false, item.isUserAllowedToSendEvent(1024,9));
    ASSERT_EQ(true, item.isUserAllowedToSendEvent(1040,6));

    ASSERT_EQ(true, item.isUserAllowedToSendEvent(30,9));

    ASSERT_EQ(true, item.isUserAllowedToSendEvent(0,20));
}

//-----------------------------------------------------------------------------
int
main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
