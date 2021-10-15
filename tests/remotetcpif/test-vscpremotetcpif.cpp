
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

#include <chrono>

#include <gtest/gtest.h>

#include <vscpremotetcpif.h>
#include <vscphelper.h>

// uint8_t iv4[5] = {0xae,0xC6,0xA7,0xE2,0xA4};
// ASSERT_EQ(-21489.320312f, vscp_getMeasurementAsFloat(iv4, 5));
// ASSERT_NE(-21489.320312, vscp_getMeasurementAsFloat(iv4, 5));
// ASSERT_NE(-1, vscp_getMeasurementAsFloat(iv4, 5));
// ASSERT_NE(0, vscp_getMeasurementAsFloat(iv4, 5));

// If TEST_LOCAL is defined all tests are run also against a local machine
//#define TEST_LOCAL 

// Standard connection (a VSCP daemon must be running here)
#define INTERFACE1 	          "vscp2.vscp.org:9598;admin;secret"
#define INTERFACE1_HOST 	    "vscp2.vscp.org:9598"
#define INTERFACE1_PORT       9598
#define INTERFACE1_USER 	    "admin"
#define INTERFACE1_PASSWORD   "secret"

// Standard connection (a VSCP daemon must be running here)
#define INTERFACE2 	          "lynx:9598;admin;secret"
#define INTERFACE2_HOST 	    "lynx:9598"
#define INTERFACE2_PORT       9598
#define INTERFACE2_USER 	    "admin"
#define INTERFACE2_PASSWORD   "secret"

///////////////////////////////////////////////////////////////////////////////
// Open session to host and Send n events using ev
//

int sendEventFromOtherSession(const char *host,
                                const char *user,
                                const char *password,
                                unsigned int count)
{
  int rv;

  VscpRemoteTcpIf vscpif;

  // Send event (Turn on the 'thing(s)' defined by zone/subzone)
  vscpEvent *pev = new vscpEvent;
  pev->pdata = nullptr;
  pev->sizeData = 0;

  memset(pev->GUID, 0, 16);               // Use inteface GUID
  vscp_setEventDateTimeBlockToNow(pev);
  pev->head     = VSCP_HEADER16_DUMB;     // This node does not have registers, mdf etc
  pev->timestamp = vscp_makeTimeStamp();
  pev->vscp_class = VSCP_CLASS1_CONTROL;
  pev->vscp_type = VSCP_TYPE_CONTROL_TURNON;
  pev->sizeData = 3;
  pev->pdata = new uint8_t[3];
  pev->pdata[0] = 0;       // Optional user byte
  pev->pdata[1] = 0x55;    // Zone
  pev->pdata[2] = 0xAA;    // SubZone

  rv = vscpif.doCmdOpen(host, user, password);
  if (VSCP_ERROR_SUCCESS != rv) return rv;
  
  for (unsigned int i=0;i<count; i++) {
    pev->pdata[0] = i;
    rv = vscpif.doCmdSend(pev);
    if (VSCP_ERROR_SUCCESS != rv) return rv;
  }

  vscp_deleteEvent(pev);
  
  rv = vscpif.doCmdClose();
  if (VSCP_ERROR_SUCCESS != rv) return rv;

  return rv;
}



///////////////////////////////////////////////////////////////////////////////
// Open session to host and Send n events using ex
//

int sendEventExFromOtherSession(const char *host,
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
  
  for (unsigned int i=0;i<count; i++) {
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

TEST(VscpRemoteTcpIf, ConnectVscp2HostUserPassword) 
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

TEST(VscpRemoteTcpIf, ConnectVscp2HostPortUserPassword) 
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

TEST(VscpRemoteTcpIf, ConnectVscp2UnknownHost) 
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
TEST(lynx, ConnectVscp2HostPortUserPassword) 
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

TEST(VscpRemoteTcpIf, ConnectVscp2Interface) 
{ 
  const char *pInterface = (char *)INTERFACE1;

  VscpRemoteTcpIf vscpif;

  ASSERT_EQ(CANAL_ERROR_SUCCESS, vscpif.doCmdOpen(pInterface));
  ASSERT_EQ(CANAL_ERROR_SUCCESS, vscpif.doCmdNOOP());
  ASSERT_EQ(CANAL_ERROR_SUCCESS, vscpif.doCmdClose());
}


//-----------------------------------------------------------------------------


#ifdef TEST_LOCAL
TEST(lynx, ConnectLynxInterface) 
{ 
  const char *pInterface = (char *)INTERFACE2;

  VscpRemoteTcpIf vscpif;

  ASSERT_EQ(CANAL_ERROR_SUCCESS, vscpif.doCmdOpen(pInterface));
  ASSERT_EQ(CANAL_ERROR_SUCCESS, vscpif.doCmdNOOP());
  ASSERT_EQ(CANAL_ERROR_SUCCESS, vscpif.doCmdClose());
      
}
#endif

//-----------------------------------------------------------------------------
TEST(VscpRemoteTcpIf, ConnectNowhere) 
{ 
  const char *pHost = "nowhere";
  const char *pUser = (char *)INTERFACE1_USER;
  const char *pPassword = (char *)INTERFACE1_PASSWORD;

  VscpRemoteTcpIf vscpif;

  ASSERT_NE(CANAL_ERROR_SUCCESS, vscpif.doCmdOpen( pHost, pUser, pPassword));

}

//-----------------------------------------------------------------------------
TEST(VscpRemoteTcpIf, ConnectUserEmptyFault) 
{ 
  const char *pHost = (char *)INTERFACE1_HOST;
  const char *pUser = "";   // Empty user
  const char *pPassword = (char *)INTERFACE1_PASSWORD;

  VscpRemoteTcpIf vscpif;

  ASSERT_EQ(VSCP_ERROR_USER, vscpif.doCmdOpen( pHost, pUser, pPassword));

}

//-----------------------------------------------------------------------------
TEST(VscpRemoteTcpIf, ConnectUserWrongFault) 
{ 
  const char *pHost = (char *)INTERFACE1_HOST;
  const char *pUser = "xxxxxx"; // Wrong user
  const char *pPassword = (char *)INTERFACE1_PASSWORD;

  VscpRemoteTcpIf vscpif;
 
  // Fault is not detected until password is sent 
  // This is to not reveal user names
  ASSERT_EQ(VSCP_ERROR_PASSWORD, vscpif.doCmdOpen( pHost, pUser, pPassword));
}

//-----------------------------------------------------------------------------
TEST(VscpRemoteTcpIf, ConnectPasswordEmptyFault) 
{ 
  const char *pHost = (char *)INTERFACE1_HOST;
  const char *pUser = (char *)INTERFACE1_USER;
  const char *pPassword = "";   // Empty password

  VscpRemoteTcpIf vscpif;

  ASSERT_EQ(VSCP_ERROR_PASSWORD, vscpif.doCmdOpen( pHost, pUser, pPassword));
}

//-----------------------------------------------------------------------------
TEST(VscpRemoteTcpIf, ConnectPasswordWrongFault) 
{ 
  const char *pHost = (char *)INTERFACE1_HOST;
  const char *pUser = (char *)INTERFACE1_USER;
  const char *pPassword = "yyyyyyy";  // Wrong password

  VscpRemoteTcpIf vscpif;

  ASSERT_EQ(VSCP_ERROR_PASSWORD, vscpif.doCmdOpen( pHost, pUser, pPassword));
}

//-----------------------------------------------------------------------------
TEST(VscpRemoteTcpIf, cCnnectChkdataEv) 
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
  ASSERT_EQ(CANAL_ERROR_SUCCESS, sendEventFromOtherSession(pHost, 
                                                              pUser, 
                                                              pPassword, 
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

//-----------------------------------------------------------------------------
TEST(VscpRemoteTcpIf, ConnectChkdataEx) 
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
  ASSERT_EQ(CANAL_ERROR_SUCCESS, sendEventExFromOtherSession(pHost, 
                                                              pUser, 
                                                              pPassword, 
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


//-----------------------------------------------------------------------------
TEST(VscpRemoteTcpIf, RcvloopTestEx) 
{ 
  const char *pHost = (char *)INTERFACE1_HOST;
  const char *pUser = (char *)INTERFACE1_USER;
  const char *pPassword = (char *)INTERFACE1_PASSWORD;

  VscpRemoteTcpIf vscpif;

  ASSERT_EQ(VSCP_ERROR_SUCCESS, vscpif.doCmdOpen( pHost, pUser, pPassword));
  ASSERT_EQ(VSCP_ERROR_SUCCESS, vscpif.doCmdEnterReceiveLoop());

  // Within a minute we should have four events generated by the VSCP
  // daemon itself. We wait for them here

  // Start measuring time
  std::chrono::seconds elapsed = std::chrono::seconds::zero();
  auto begin = std::chrono::high_resolution_clock::now();
  int cnt = 0;
  do {
    vscpEventEx ex;
    int rv = vscpif.doCmdBlockingReceive(&ex, 1000);    
    ASSERT_EQ(true, ((VSCP_ERROR_SUCCESS == rv) || (VSCP_ERROR_TIMEOUT == rv)));
    cnt++;
    if (cnt >= 4) {
      break;
    }
    elapsed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - begin);
  } while (elapsed.count() < 70);

  ASSERT_EQ(true, (cnt >= 4));

  auto end = std::chrono::high_resolution_clock::now();

  ASSERT_EQ(VSCP_ERROR_SUCCESS, vscpif.doCmdQuitReceiveLoop());
  ASSERT_EQ(VSCP_ERROR_SUCCESS, vscpif.doCmdClose());
}

//-----------------------------------------------------------------------------
TEST(VscpRemoteTcpIf, PollingTestEv) 
{ 
  const char *pHost = (char *)INTERFACE1_HOST;
  const char *pUser = (char *)INTERFACE1_USER;
  const char *pPassword = (char *)INTERFACE1_PASSWORD;

  VscpRemoteTcpIf vscpif;

  ASSERT_EQ(VSCP_ERROR_SUCCESS, vscpif.doCmdOpen( pHost, pUser, pPassword));

  // Start measuring time
  std::chrono::seconds elapsed = std::chrono::seconds::zero();
  auto begin = std::chrono::high_resolution_clock::now();
  int cnt = 0;
  int rv;

  do {
    if (vscpif.doCmdDataAvailable()) {
      vscpEvent *pev = new vscpEvent;
      pev->pdata = nullptr;
      pev->sizeData = 0;
      ASSERT_EQ(VSCP_ERROR_SUCCESS, (rv = vscpif.doCmdReceive(pev)));
      vscp_deleteEvent(pev);
      cnt++;
      if (cnt >= 4) {
        break;
      }
    }
  } while (elapsed.count() < 70);

  ASSERT_EQ(true, (cnt >= 4));

  ASSERT_EQ(VSCP_ERROR_SUCCESS, vscpif.doCmdClose());
}

//-----------------------------------------------------------------------------
TEST(VscpRemoteTcpIf, PollingTestEx) 
{ 
  const char *pHost = (char *)INTERFACE1_HOST;
  const char *pUser = (char *)INTERFACE1_USER;
  const char *pPassword = (char *)INTERFACE1_PASSWORD;

  VscpRemoteTcpIf vscpif;

  ASSERT_EQ(VSCP_ERROR_SUCCESS, vscpif.doCmdOpen( pHost, pUser, pPassword));

  // Start measuring time
  std::chrono::seconds elapsed = std::chrono::seconds::zero();
  auto begin = std::chrono::high_resolution_clock::now();
  int cnt = 0;
  int rv;

  do {
    if (vscpif.doCmdDataAvailable()) {
      vscpEventEx ex;
      ASSERT_EQ(VSCP_ERROR_SUCCESS, (rv = vscpif.doCmdReceiveEx(&ex)));
      cnt++;
      if (cnt >= 4) {
        break;
      }
    }
  } while (elapsed.count() < 70);

  ASSERT_EQ(true, (cnt >= 4));

  ASSERT_EQ(VSCP_ERROR_SUCCESS, vscpif.doCmdClose());
}


// ===========================================================================
// ===========================================================================
// ===========================================================================


int main(int argc, char **argv) {

#ifdef WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD(2,2);
    err               = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
      /* Tell the user that we could not find a usable */
      /* Winsock DLL.                                  */
      printf("WSAStartup failed with error: %d\n", err);
      exit(1);
    };
#endif

    testing::InitGoogleTest(&argc, argv);
   int rv =  RUN_ALL_TESTS();

#ifdef WIN32
   // Cleanup
   WSACleanup();
#endif

   return rv;
}


