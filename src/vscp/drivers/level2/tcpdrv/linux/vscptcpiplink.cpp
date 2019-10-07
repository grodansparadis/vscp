// vscptcpip.cpp: implementation of the CTcpipLink class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP Project (http://www.vscp.org)
//
// Copyright (C) 2000-2019 Ake Hedman,
// Grodans Paradis AB, <akhe@grodansparadis.com>
//
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//

#include <list>
#include <map>
#include <string>

#include <limits.h>
#include <net/if.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

#include <ctype.h>
#include <libgen.h>
#include <net/if.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <time.h>

#include <expat.h>

#include <vscp_class.h>
#include <vscp_type.h>
#include <vscpdatetime.h>
#include <vscphelper.h>
#include <vscpremotetcpif.h>

#include "vscptcpiplink.h"

// Buffer for XML parser
#define XML_BUFF_SIZE 10000

// Forward declaration
void *
workerThreadReceive(void *pData);
void *
workerThreadSend(void *pData);

//////////////////////////////////////////////////////////////////////
// CTcpipLink
//

CTcpipLink::CTcpipLink()
{
    m_bQuit          = false;
    m_pthreadSend    = NULL;
    m_pthreadReceive = NULL;
    vscp_clearVSCPFilter(&m_rxfilter); // Accept all events
    vscp_clearVSCPFilter(&m_txfilter); // Send all events
    m_responseTimeout = TCPIP_DEFAULT_INNER_RESPONSE_TIMEOUT;

    sem_init(&m_semSendQueue, 0, 0);
    sem_init(&m_semReceiveQueue, 0, 0);

    pthread_mutex_init(&m_mutexSendQueue, NULL);
    pthread_mutex_init(&m_mutexReceiveQueue, NULL);
}

//////////////////////////////////////////////////////////////////////
// ~CTcpipLink
//

CTcpipLink::~CTcpipLink()
{
    close();

    sem_destroy(&m_semSendQueue);
    sem_destroy(&m_semReceiveQueue);

    pthread_mutex_destroy(&m_mutexSendQueue);
    pthread_mutex_destroy(&m_mutexReceiveQueue);

    // Close syslog channel
    closelog();
}

// ----------------------------------------------------------------------------

/*
    XML configuration
    -----------------

    <setup host="localhost"
              port="9598"
              user="admin"
              password="secret"
              rxfilter=""
              rxmask=""
              txfilter=""
              txmask=""
              responsetimeout="2000" />
*/

// ----------------------------------------------------------------------------

int depth_setup_parser = 0;

void
startSetupParser(void *data, const char *name, const char **attr)
{
    CTcpipLink *pObj = (CTcpipLink *)data;
    if (NULL == pObj) return;

    if ((0 == strcmp(name, "setup")) && (0 == depth_setup_parser)) {

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == strcasecmp(attr[i], "host")) {
                if (!attribute.empty()) {
                    pObj->m_hostRemote = attribute;
                }
            } else if (0 == strcasecmp(attr[i], "port")) {
                if (!attribute.empty()) {
                    pObj->m_portRemote = vscp_readStringValue(attribute);
                }
            } else if (0 == strcasecmp(attr[i], "user")) {
                if (!attribute.empty()) {
                    pObj->m_usernameRemote = attribute;
                }
            } else if (0 == strcasecmp(attr[i], "password")) {
                if (!attribute.empty()) {
                    pObj->m_passwordRemote = attribute;
                }
            } else if (0 == strcasecmp(attr[i], "rxfilter")) {
                if (!attribute.empty()) {
                    if (!vscp_readFilterFromString(&pObj->m_rxfilter,
                                                   attribute)) {
                        syslog(LOG_ERR, "Unable to read event receive filter.");
                    }
                }
            } else if (0 == strcasecmp(attr[i], "rxmask")) {
                if (!attribute.empty()) {
                    if (!vscp_readMaskFromString(&pObj->m_rxfilter,
                                                 attribute)) {
                        syslog(LOG_ERR, "Unable to read event receive mask.");
                    }
                }
            } else if (0 == strcasecmp(attr[i], "txfilter")) {
                if (!attribute.empty()) {
                    if (!vscp_readFilterFromString(&pObj->m_txfilter,
                                                   attribute)) {
                        syslog(LOG_ERR,
                               "Unable to read event transmit filter.");
                    }
                }
            } else if (0 == strcasecmp(attr[i], "txmask")) {
                if (!attribute.empty()) {
                    if (!vscp_readMaskFromString(&pObj->m_txfilter,
                                                 attribute)) {
                        syslog(LOG_ERR, "Unable to read event transmit mask.");
                    }
                }
            } else if (0 == strcmp(attr[i], "responsetimeout")) {
                if (!attribute.empty()) {
                    pObj->m_responseTimeout = vscp_readStringValue(attribute);
                }
            }
        }
    }

    depth_setup_parser++;
}

void
endSetupParser(void *data, const char *name)
{
    depth_setup_parser--;
}

// ----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////
// open
//
//

bool
CTcpipLink::open(const char *pUsername,
                 const char *pPassword,
                 const char *pHost,
                 short port,
                 const char *pPrefix,
                 const char *pConfig)
{
    std::string wxstr = std::string(pConfig);

    m_usernameLocal = std::string(pUsername);
    m_passwordLocal = std::string(pPassword);
    m_hostLocal     = std::string(pHost);
    m_portLocal     = port;
    m_prefix        = std::string(pPrefix);

    // Parse the configuration string.
    std::deque<std::string> tokens;
    vscp_split(tokens, std::string(pConfig), ";");

    // Check for remote host in configuration string
    if (!tokens.empty()) {
        // Get remote interface
        m_hostRemote = tokens.front();
        tokens.pop_front();
    }

    // Check for remote port in configuration string
    if (!tokens.empty()) {
        // Get remote port
        m_portRemote = vscp_readStringValue(tokens.front());
        tokens.pop_front();
    }

    // Check for remote user in configuration string
    if (!tokens.empty()) {
        // Get remote username
        m_usernameRemote = tokens.front();
        tokens.pop_front();
    }

    // Check for remote password in configuration string
    if (!tokens.empty()) {
        // Get remote password
        m_passwordRemote = tokens.front();
        tokens.pop_front();
    }

    std::string strRxFilter;
    // Check for filter in configuration string
    if (!tokens.empty()) {
        // Get filter
        strRxFilter = tokens.front();
        tokens.pop_front();
        vscp_readFilterFromString(&m_rxfilter, strRxFilter);
    }

    // Check for mask in configuration string
    std::string strRxMask;
    if (!tokens.empty()) {
        // Get mask
        strRxMask = tokens.front();
        tokens.pop_front();
        vscp_readMaskFromString(&m_rxfilter, strRxMask);
    }

    std::string strTxFilter;
    // Check for filter in configuration string
    if (!tokens.empty()) {
        // Get filter
        strTxFilter = tokens.front();
        tokens.pop_front();
        vscp_readFilterFromString(&m_txfilter, strTxFilter);
    }

    // Check for mask in configuration string
    std::string strTxMask;
    if (!tokens.empty()) {
        // Get mask
        strTxMask = tokens.front();
        tokens.pop_front();
        vscp_readMaskFromString(&m_txfilter, strTxMask);
    }

    // Check for response timout in configuration string
    std::string strResponseTimout;
    if (!tokens.empty()) {
        // Get response timout
        strResponseTimout = tokens.front();
        tokens.pop_front();
        m_responseTimeout = vscp_readStringValue(strResponseTimout);
    }

    // First log on to the host and get configuration
    // variables

    if (VSCP_ERROR_SUCCESS !=
        m_srvLocal.doCmdOpen(
          m_hostLocal, port, m_usernameLocal, m_passwordLocal)) {
        syslog(LOG_ERR,
               "%s %s ",
               VSCP_TCPIPLINK_SYSLOG_DRIVER_ID,
               (const char *)"Unable to connect to local VSCP TCP/IP "
                             "interface. Terminating!");
        return false;
    }

    // Find the channel id
    uint32_t ChannelID;
    m_srvLocal.doCmdGetChannelID(&ChannelID);

    // The server should hold configuration data
    //
    // We look for
    //
    //   _host_remote       - The remote host to which we should connect
    //
    //   _port_remote       - The port to connect to at the remote host.
    //
    //   _user_remote       - Username to login at remote host
    //
    //   _password_remote   - Username to login at remote host
    //
    //   _filter (_rxfiter)- Standard VSCP filter in string form for receive-
    //             1,0x0000,0x0006,
    //                 ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00
    //              as priority,class,type,GUID
    //              Used to filter what events that is received from
    //              the socketcan interface. If not give all events
    //              are received.
    //
    //  _mask (_rxmask)- Standard VSCP mask in string form for receive.
    //              1,0x0000,0x0006,
    //                 ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00
    //              as priority,class,type,GUID
    //              Used to filter what events that is received from
    //              the socketcan interface. If not give all events
    //              are received.
    //
    // _responsetimeout - The time in milliseconds we should wait for a response
    //                      from the remote server
    //
    //

    std::string str;
    std::string strName = m_prefix + std::string("_host_remote");
    m_srvLocal.getRemoteVariableValue(strName, m_hostRemote);

    strName = m_prefix + std::string("_port_remote");
    m_srvLocal.getRemoteVariableInt(strName, &m_portRemote);

    strName = m_prefix + std::string("_user_remote");
    m_srvLocal.getRemoteVariableValue(strName, m_usernameRemote);

    strName = m_prefix + std::string("_password_remote");
    m_srvLocal.getRemoteVariableValue(strName, m_passwordRemote);

    // Old format
    strName = m_prefix + std::string("_filter");
    if (VSCP_ERROR_SUCCESS == m_srvLocal.getRemoteVariableValue(strName, str)) {
        vscp_readFilterFromString(&m_rxfilter, str);
    }

    strName = m_prefix + std::string("_mask");
    if (VSCP_ERROR_SUCCESS == m_srvLocal.getRemoteVariableValue(strName, str)) {
        vscp_readMaskFromString(&m_rxfilter, str);
    }

    //  New format
    strName = m_prefix + std::string("_rxfilter");
    if (VSCP_ERROR_SUCCESS == m_srvLocal.getRemoteVariableValue(strName, str)) {
        vscp_readFilterFromString(&m_rxfilter, str);
    }

    strName = m_prefix + std::string("_rxmask");
    if (VSCP_ERROR_SUCCESS == m_srvLocal.getRemoteVariableValue(strName, str)) {
        vscp_readMaskFromString(&m_rxfilter, str);
    }

    strName = m_prefix + std::string("_txfilter");
    if (VSCP_ERROR_SUCCESS == m_srvLocal.getRemoteVariableValue(strName, str)) {
        vscp_readFilterFromString(&m_txfilter, str);
    }

    strName = m_prefix + std::string("_txmask");
    if (VSCP_ERROR_SUCCESS == m_srvLocal.getRemoteVariableValue(strName, str)) {
        vscp_readMaskFromString(&m_txfilter, str);
    }

    strName = m_prefix + std::string("_response_timout");
    if (VSCP_ERROR_SUCCESS == m_srvLocal.getRemoteVariableValue(strName, str)) {
        m_responseTimeout = vscp_readStringValue(str);
    }

    /////////////////////////////////////////////////////////////
    //                    XML Configuration
    /////////////////////////////////////////////////////////////

    std::string setupXml;
    strName = m_prefix + std::string("_setup");
    if (VSCP_ERROR_SUCCESS !=
        m_srvLocal.getRemoteVariableValue(strName, setupXml)) {
        // Not here, we use empty mock-up
        setupXml = "<?xml version = \"1.0\" encoding = \"UTF-8\" "
                   "?><setup><!-- empty --></setup>";
    }

    // XML setup
    std::string strSetupXML;
    strName = m_prefix + std::string("_setup");
    if (VSCP_ERROR_SUCCESS ==
        m_srvLocal.getRemoteVariableValue(strName, strSetupXML, true)) {
        XML_Parser xmlParser = XML_ParserCreate("UTF-8");
        XML_SetUserData(xmlParser, this);
        XML_SetElementHandler(xmlParser, startSetupParser, endSetupParser);

        int bytes_read;
        void *buff = XML_GetBuffer(xmlParser, XML_BUFF_SIZE);

        strncpy((char *)buff, strSetupXML.c_str(), strSetupXML.length());

        bytes_read = strSetupXML.length();
        if (!XML_ParseBuffer(xmlParser, bytes_read, bytes_read == 0)) {
            syslog(LOG_ERR, "Failed parse XML setup.");
        }

        XML_ParserFree(xmlParser);
    }

    // Close the channel
    m_srvLocal.doCmdClose();

    // start the workerthread
    if (pthread_create(m_pthreadSend, NULL, workerThreadSend, this)) {

        syslog(LOG_ERR, "Unable to start send worker thread.");
        return false;
    }

    if (pthread_create(m_pthreadReceive, NULL, workerThreadReceive, this)) {

        syslog(LOG_ERR, "Unable to start receive worker thread.");
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// close
//

void
CTcpipLink::close(void)
{
    // Do nothing if already terminated
    if (m_bQuit) return;

    m_bQuit = true; // terminate the thread
    sleep(1);       // Give the thread some time to terminate
}

//////////////////////////////////////////////////////////////////////
// addEvent2SendQueue
//

bool
CTcpipLink::addEvent2SendQueue(const vscpEvent *pEvent)
{
    pthread_mutex_lock(&m_mutexSendQueue);
    m_sendList.push_back((vscpEvent *)pEvent);
    sem_post(&m_semSendQueue);
    pthread_mutex_lock(&m_mutexSendQueue);
    return true;
}

//////////////////////////////////////////////////////////////////////
// Send worker thread
//

void *
workerThreadSend(void *pData)
{
    bool bRemoteConnectionLost = false;

    CTcpipLink *pObj = (CTcpipLink *)pData;
    if (NULL == pObj) return NULL;

retry_send_connect:

    // Open remote interface
    if (VSCP_ERROR_SUCCESS !=
        pObj->m_srvRemote.doCmdOpen(pObj->m_hostRemote,
                                    pObj->m_portRemote,
                                    pObj->m_usernameRemote,
                                    pObj->m_passwordRemote)) {
        syslog(LOG_ERR,
               "%s %s ",
               VSCP_TCPIPLINK_SYSLOG_DRIVER_ID,
               (const char *)"Error while opening remote VSCP TCP/IP "
                             "interface. Terminating!");

        // Give the server some time to become active
        for (int loopcnt = 0; loopcnt < VSCP_TCPIPLINK_DEFAULT_RECONNECT_TIME;
             loopcnt++) {
            sleep(1);
            if (pObj->m_bQuit) return NULL;
        }

        goto retry_send_connect;
    }

    syslog(LOG_ERR,
           "%s %s ",
           VSCP_TCPIPLINK_SYSLOG_DRIVER_ID,
           (const char *)"Connect to remote VSCP TCP/IP interface [SEND].");

    // Find the channel id
    pObj->m_srvRemote.doCmdGetChannelID(&pObj->txChannelID);

    while (!pObj->m_bQuit) {

        // Make sure the remote connection is up
        if (!pObj->m_srvRemote.isConnected()) {

            if (!bRemoteConnectionLost) {
                bRemoteConnectionLost = true;
                pObj->m_srvRemote.doCmdClose();
                syslog(LOG_ERR,
                       "%s %s ",
                       VSCP_TCPIPLINK_SYSLOG_DRIVER_ID,
                       (const char *)"Lost connection to remote host [SEND].");
            }

            // Wait before we try to connect again
            sleep(VSCP_TCPIPLINK_DEFAULT_RECONNECT_TIME);

            if (VSCP_ERROR_SUCCESS !=
                pObj->m_srvRemote.doCmdOpen(pObj->m_hostRemote,
                                            pObj->m_portRemote,
                                            pObj->m_usernameRemote,
                                            pObj->m_passwordRemote)) {
                syslog(LOG_ERR,
                       "%s %s ",
                       VSCP_TCPIPLINK_SYSLOG_DRIVER_ID,
                       (const char *)"Reconnected to remote host [SEND].");

                // Find the channel id
                pObj->m_srvRemote.doCmdGetChannelID(&pObj->txChannelID);

                bRemoteConnectionLost = false;
            }

            continue;
        }

        if ((-1 == vscp_sem_wait(&pObj->m_semSendQueue, 500)) &&
            errno == ETIMEDOUT) {
            continue;
        }

        // Check if there is event(s) to send
        if (pObj->m_sendList.size()) {

            // Yes there are data to send
            pthread_mutex_lock(&pObj->m_mutexSendQueue);
            vscpEvent *pEvent = pObj->m_sendList.front();
            // Check if event should be filtered away
            if (!vscp_doLevel2Filter(pEvent, &pObj->m_txfilter)) {
                pthread_mutex_unlock(&pObj->m_mutexSendQueue);
                continue;
            }
            pObj->m_sendList.pop_front();
            pthread_mutex_lock(&pObj->m_mutexSendQueue);

            if (NULL == pEvent) continue;

            // Yes there are data to send
            // Send it out to the remote server

            pObj->m_srvRemote.doCmdSend(pEvent);
            vscp_deleteVSCPevent_v2(&pEvent);
        }
    }

    // Close the channel
    pObj->m_srvRemote.doCmdClose();

    syslog(
      LOG_ERR,
      "%s %s ",
      VSCP_TCPIPLINK_SYSLOG_DRIVER_ID,
      (const char *)"Disconnect from remote VSCP TCP/IP interface [SEND].");

    return NULL;
}

//////////////////////////////////////////////////////////////////////
//                Workerthread Receive - CWrkReceiveTread
//////////////////////////////////////////////////////////////////////

void *
workerThreadReceive(void *pData)
{
    bool bRemoteConnectionLost = false;
    bool bActivity             = false;

    CTcpipLink *pObj = (CTcpipLink *)pData;
    if (NULL == pObj) return NULL;

retry_receive_connect:

    // Open remote interface
    if (VSCP_ERROR_SUCCESS !=
        pObj->m_srvRemote.doCmdOpen(pObj->m_hostRemote,
                                    pObj->m_portRemote,
                                    pObj->m_usernameRemote,
                                    pObj->m_passwordRemote)) {
        syslog(LOG_ERR,
               "%s %s ",
               VSCP_TCPIPLINK_SYSLOG_DRIVER_ID,
               (const char *)"Error while opening remote VSCP TCP/IP "
                             "interface. Terminating!");

        // Give the server some time to become active
        for (int loopcnt = 0; loopcnt < VSCP_TCPIPLINK_DEFAULT_RECONNECT_TIME;
             loopcnt++) {
            sleep(1);
            if (pObj->m_bQuit) return NULL;
        }

        goto retry_receive_connect;
    }

    syslog(LOG_ERR,
           "%s %s ",
           VSCP_TCPIPLINK_SYSLOG_DRIVER_ID,
           (const char *)"Connect to remote VSCP TCP/IP interface [RECEIVE].");

    // Set receive filter
    if (VSCP_ERROR_SUCCESS !=
        pObj->m_srvRemote.doCmdFilter(&pObj->m_rxfilter)) {
        syslog(LOG_ERR,
               "%s %s ",
               VSCP_TCPIPLINK_SYSLOG_DRIVER_ID,
               (const char *)"Failed to set receiving filter.");
    }

    // Enter the receive loop
    pObj->m_srvRemote.doCmdEnterReceiveLoop();

    vscpEventEx eventEx;
    while (!pObj->m_bQuit) {

        // Make sure the remote connection is up
        if (!pObj->m_srvRemote.isConnected() ||
            ((vscp_getMsTimeStamp() - pObj->m_srvRemote.getlastResponseTime()) >
             (VSCP_TCPIPLINK_DEFAULT_RECONNECT_TIME * 1000))) {

            if (!bRemoteConnectionLost) {

                bRemoteConnectionLost = true;
                pObj->m_srvRemote.doCmdClose();
                syslog(
                  LOG_ERR,
                  "%s %s ",
                  VSCP_TCPIPLINK_SYSLOG_DRIVER_ID,
                  (const char *)"Lost connection to remote host [Receive].");
            }

            // Wait before we try to connect again
            sleep(VSCP_TCPIPLINK_DEFAULT_RECONNECT_TIME);

            if (VSCP_ERROR_SUCCESS !=
                pObj->m_srvRemote.doCmdOpen(pObj->m_hostRemote,
                                            pObj->m_portRemote,
                                            pObj->m_usernameRemote,
                                            pObj->m_passwordRemote)) {
                syslog(LOG_ERR,
                       "%s %s ",
                       VSCP_TCPIPLINK_SYSLOG_DRIVER_ID,
                       (const char *)"Reconnected to remote host [Receive].");
                bRemoteConnectionLost = false;
            }

            // Enter the receive loop
            pObj->m_srvRemote.doCmdEnterReceiveLoop();

            continue;
        }

        // Check if remote server has something to send to us
        vscpEvent *pEvent = new vscpEvent;
        if (NULL != pEvent) {

            pEvent->sizeData = 0;
            pEvent->pdata    = NULL;

            if (CANAL_ERROR_SUCCESS ==
                pObj->m_srvRemote.doCmdBlockingReceive(pEvent)) {

                // Filter is handled at server side. We check so we don't
                // receive things we send ourself.
                if (pObj->txChannelID != pEvent->obid) {
                    pthread_mutex_lock(&pObj->m_mutexReceiveQueue);
                    pObj->m_receiveList.push_back(pEvent);
                    sem_post(&pObj->m_semReceiveQueue);
                    pthread_mutex_unlock(&pObj->m_mutexReceiveQueue);
                } else {
                    vscp_deleteVSCPevent(pEvent);
                }

            } else {
                vscp_deleteVSCPevent(pEvent);
            }
        }
    }

    // Close the channel
    pObj->m_srvRemote.doCmdClose();

    syslog(
      LOG_ERR,
      "%s %s ",
      VSCP_TCPIPLINK_SYSLOG_DRIVER_ID,
      (const char *)"Disconnect from remote VSCP TCP/IP interface [RECEIVE].");

    return NULL;
}
