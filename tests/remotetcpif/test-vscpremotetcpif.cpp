
///////////////////////////////////////////////////////////////////////////////
// noop.cpp
//
// https://www.vscp.org   the VSCP project   info@vscp.org
//https://www.vscp.org/docs/vscphelper/doku.php?id=classes_vscpremotetcpipif_metods

#ifdef WIN32
#include <pch.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <gtest/gtest.h>

#include <vscpremotetcpif.h>
#include <vscphelper.h>

// uint8_t iv4[5] = {0xae,0xC6,0xA7,0xE2,0xA4};
// ASSERT_EQ(-21489.320312f, vscp_getMeasurementAsFloat(iv4, 5));
// ASSERT_NE(-21489.320312, vscp_getMeasurementAsFloat(iv4, 5));
// ASSERT_NE(-1, vscp_getMeasurementAsFloat(iv4, 5));
// ASSERT_NE(0, vscp_getMeasurementAsFloat(iv4, 5));

// If TEST_LOCAL is defined all tests are run also against a local machine
#define TEST_LOCAL 

// Standard connection (a VSCP daemon must be running here)
#define INTERFACE1 	        "vscp2.vscp.org:9598;admin;secret"
#define INTERFACE1_HOST 	  "vscp2.vscp.org:9598"
#define INTERFACE1_PORT     9598
#define INTERFACE1_USER 	  "admin"
#define INTERFACE1_PASSWORD "secret"

// Standard connection (a VSCP daemon must be running here)
#define INTERFACE2 	        "lynx:9598;admin;secret"
#define INTERFACE2_HOST 	  "lynx:9598"
#define INTERFACE2_PORT     9598
#define INTERFACE2_USER 	  "admin"
#define INTERFACE2_PASSWORD "secret"


///////////////////////////////////////////////////////////////////////////////
// Open session to host and Send n events
//

int sendEventsFromOtherSession(const char *host,
                                const char *user,
                                const char *password,
                                unsigned int count)
{
  int rv;

  VscpRemoteTcpIf vscpif;

  // Send event (Turn on the 'thing(s)' defined by zone/subzone)
  vscpEventEx ex;
  memset(ex.GUID, 0, 16);                     // Use inteface GUID
  vscp_setEventExDateTimeBlockToNow(&ex);
  ex.head = VSCP_HEADER16_DUMB;               // This node does not have registers, mdf etc
  ex.timestamp = vscp_makeTimeStamp();
  ex.vscp_class = VSCP_CLASS1_CONTROL;
  ex.vscp_type = VSCP_TYPE_CONTROL_TURNON;
  ex.sizeData = 3;
  ex.data[0] = 0;       // Optional user byte
  ex.data[1] = 0x55;    // Zone
  ex.data[2] = 0xAA;    // SubZone

  rv = vscpif.doCmdOpen(host, user, password);
  if (VSCP_ERROR_SUCCESS != rv) return rv;
  
  for (int i=0;i<count; i++) {
    ex.data[0] = i;
    rv = vscpif.doCmdSendEx(&ex);
    if (VSCP_ERROR_SUCCESS != rv) return rv;
  }
  
  rv = vscpif.doCmdClose();
  if (VSCP_ERROR_SUCCESS != rv) return rv;

  return rv;
}


//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

TEST(VscpRemoteTcpIf, connect_vscp2_host_user_password) 
{ 
  const char *pHost = (char *)INTERFACE1_HOST;
  const char *pUser = (char *)INTERFACE1_USER;
  const char *pPassword = (char *)INTERFACE1_PASSWORD;

  VscpRemoteTcpIf vscpif;

  ASSERT_EQ(CANAL_ERROR_SUCCESS, vscpif.doCmdOpen( pHost, pUser, pPassword));
  ASSERT_EQ(CANAL_ERROR_SUCCESS, vscpif.doCmdNOOP());
  ASSERT_EQ(CANAL_ERROR_SUCCESS, vscpif.doCmdClose());
}


//-----------------------------------------------------------------------------


#ifdef TEST_LOCAL
TEST(lynx, connect_vscp2_host_user_password) 
{ 
  const char *pHost = (char *)INTERFACE2_HOST;
  const char *pUser = (char *)INTERFACE2_USER;
  const char *pPassword = (char *)INTERFACE2_PASSWORD;

  VscpRemoteTcpIf vscpif;

  ASSERT_EQ(CANAL_ERROR_SUCCESS, vscpif.doCmdOpen( pHost, pUser, pPassword));
  ASSERT_EQ(CANAL_ERROR_SUCCESS, vscpif.doCmdNOOP());
  ASSERT_EQ(CANAL_ERROR_SUCCESS, vscpif.doCmdClose());
      
}
#endif

//-----------------------------------------------------------------------------

TEST(VscpRemoteTcpIf, connect_vscp2_host_port_user_password) 
{ 
  const char *pHost = (char *)INTERFACE1_HOST;
  const char *pUser = (char *)INTERFACE1_USER;
  const char *pPassword = (char *)INTERFACE1_PASSWORD;

  VscpRemoteTcpIf vscpif;

  ASSERT_EQ(CANAL_ERROR_SUCCESS, vscpif.doCmdOpen( pHost, INTERFACE1_PORT, pUser, pPassword));
  ASSERT_EQ(CANAL_ERROR_SUCCESS, vscpif.doCmdNOOP());
  ASSERT_EQ(CANAL_ERROR_SUCCESS, vscpif.doCmdClose());
}


//-----------------------------------------------------------------------------

TEST(VscpRemoteTcpIf, connect_vscp2_unknown_host) 
{ 
  const char *pHost = (char *)"test.host.no";
  const char *pUser = (char *)INTERFACE1_USER;
  const char *pPassword = (char *)INTERFACE1_PASSWORD;

  VscpRemoteTcpIf vscpif;

  ASSERT_EQ(VSCP_ERROR_TIMEOUT, vscpif.doCmdOpen(pHost, INTERFACE1_PORT, pUser, pPassword));
  // Should have ":port" so parameter error
  ASSERT_EQ(VSCP_ERROR_PARAMETER, vscpif.doCmdOpen(pHost, pUser, pPassword));
  ASSERT_EQ(VSCP_ERROR_TIMEOUT, vscpif.doCmdOpen("test.host.no:9598", pUser, pPassword));
  ASSERT_EQ(VSCP_ERROR_TIMEOUT, vscpif.doCmdOpen("tcp://test.host.no:9598;aaaa;bbbbb"));
}


//-----------------------------------------------------------------------------


#ifdef TEST_LOCAL
TEST(lynx, connect_vscp2_host_port_user_password) 
{ 
  const char *pHost = (char *)INTERFACE2_HOST;
  const char *pUser = (char *)INTERFACE2_USER;
  const char *pPassword = (char *)INTERFACE2_PASSWORD;

  VscpRemoteTcpIf vscpif;

  ASSERT_EQ(CANAL_ERROR_SUCCESS, vscpif.doCmdOpen( pHost, INTERFACE2_PORT, pUser, pPassword));
  ASSERT_EQ(CANAL_ERROR_SUCCESS, vscpif.doCmdNOOP());
  ASSERT_EQ(CANAL_ERROR_SUCCESS, vscpif.doCmdClose());
      
}
#endif

//-----------------------------------------------------------------------------

TEST(VscpRemoteTcpIf, connect_vscp2_interface) 
{ 
  const char *pInterface = (char *)INTERFACE1;

  VscpRemoteTcpIf vscpif;

  ASSERT_EQ(CANAL_ERROR_SUCCESS, vscpif.doCmdOpen(pInterface));
  ASSERT_EQ(CANAL_ERROR_SUCCESS, vscpif.doCmdNOOP());
  ASSERT_EQ(CANAL_ERROR_SUCCESS, vscpif.doCmdClose());
}


//-----------------------------------------------------------------------------


#ifdef TEST_LOCAL
TEST(lynx, connect_lynx_interface) 
{ 
  const char *pInterface = (char *)INTERFACE2;

  VscpRemoteTcpIf vscpif;

  ASSERT_EQ(CANAL_ERROR_SUCCESS, vscpif.doCmdOpen(pInterface));
  ASSERT_EQ(CANAL_ERROR_SUCCESS, vscpif.doCmdNOOP());
  ASSERT_EQ(CANAL_ERROR_SUCCESS, vscpif.doCmdClose());
      
}
#endif

//-----------------------------------------------------------------------------
TEST(VscpRemoteTcpIf, connect_nowhere) 
{ 
  const char *pHost = "nowhere";
  const char *pUser = (char *)INTERFACE1_USER;
  const char *pPassword = (char *)INTERFACE1_PASSWORD;

  VscpRemoteTcpIf vscpif;

  ASSERT_NE(CANAL_ERROR_SUCCESS, vscpif.doCmdOpen( pHost, pUser, pPassword));

}

//-----------------------------------------------------------------------------
TEST(VscpRemoteTcpIf, connect_user_empty_fault) 
{ 
  const char *pHost = (char *)INTERFACE1_HOST;
  const char *pUser = "";
  const char *pPassword = (char *)INTERFACE1_PASSWORD;

  VscpRemoteTcpIf vscpif;

  ASSERT_EQ(VSCP_ERROR_USER, vscpif.doCmdOpen( pHost, pUser, pPassword));

}

//-----------------------------------------------------------------------------
TEST(VscpRemoteTcpIf, connect_user_wrong_fault) 
{ 
  const char *pHost = (char *)INTERFACE1_HOST;
  const char *pUser = "xxxxxx";
  const char *pPassword = (char *)INTERFACE1_PASSWORD;

  VscpRemoteTcpIf vscpif;
 
  // Fault is not detected until password is sent 
  // This is to not reveal user names
  ASSERT_EQ(VSCP_ERROR_PASSWORD, vscpif.doCmdOpen( pHost, pUser, pPassword));
}

//-----------------------------------------------------------------------------
TEST(VscpRemoteTcpIf, connect_password_empty_fault) 
{ 
  const char *pHost = (char *)INTERFACE1_HOST;
  const char *pUser = (char *)INTERFACE1_USER;
  const char *pPassword = "";

  VscpRemoteTcpIf vscpif;

  ASSERT_EQ(VSCP_ERROR_PASSWORD, vscpif.doCmdOpen( pHost, pUser, pPassword));
}

//-----------------------------------------------------------------------------
TEST(VscpRemoteTcpIf, connect_password_wrong_fault) 
{ 
  const char *pHost = (char *)INTERFACE1_HOST;
  const char *pUser = (char *)INTERFACE1_USER;
  const char *pPassword = "yyyyyyy";

  VscpRemoteTcpIf vscpif;

  ASSERT_EQ(VSCP_ERROR_PASSWORD, vscpif.doCmdOpen( pHost, pUser, pPassword));
}

//-----------------------------------------------------------------------------
TEST(VscpRemoteTcpIf, connect_chkdata) 
{ 
  const char *pHost = (char *)INTERFACE1_HOST;
  const char *pUser = (char *)INTERFACE1_USER;
  const char *pPassword = (char *)INTERFACE1_PASSWORD;

  VscpRemoteTcpIf vscpif;

  ASSERT_EQ(false, vscpif.isConnected());
  ASSERT_EQ(CANAL_ERROR_SUCCESS, vscpif.doCmdOpen( pHost, INTERFACE1_PORT, pUser, pPassword));
  ASSERT_EQ(CANAL_ERROR_SUCCESS, vscpif.doCmdNOOP());
  ASSERT_EQ(true, vscpif.isConnected());
  ASSERT_EQ(VSCP_LEVEL2, vscpif.doCmdGetLevel());
  ASSERT_EQ(CANAL_ERROR_SUCCESS, sendEventsFromOtherSession(INTERFACE1_HOST, 
                                                              INTERFACE1_USER, 
                                                              INTERFACE1_PASSWORD, 
                                                              10));                                                            
  ASSERT_EQ(true, (vscpif.doCmdDataAvailable() >= 10));
  do {
    vscpEvent *pEvent = new vscpEvent;
    ASSERT_EQ(CANAL_ERROR_SUCCESS, vscpif.doCmdReceive(pEvent));
    if ( (nullptr != pEvent) && 
          (VSCP_CLASS1_CONTROL == pEvent->vscp_class) && 
          (VSCP_TYPE_CONTROL_TURNON == pEvent->vscp_type) ) {
      ASSERT_EQ(3, pEvent->sizeData);
      ASSERT_NE(nullptr, pEvent->pdata);
      ASSERT_EQ(0x55, pEvent->pdata[1]);
      ASSERT_EQ(0xAA, pEvent->pdata[2]);
    }
    vscp_deleteEvent(pEvent);
  } while (vscpif.doCmdDataAvailable() > 0);

}

// ===========================================================================
//----------------------------------------------------------------------------
// ===========================================================================

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


