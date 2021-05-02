// tcpipsrv.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2021 Ake Hedman, the VSCP project
// <info@grodansparadis.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// https://wiki.openssl.org/index.php/Simple_TLS_Server
// https://wiki.openssl.org/index.php/SSL/TLS_Client
// https://stackoverflow.com/questions/3919420/tutorial-on-using-openssl-with-pthreads
//

#include <list>
#include <string>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#ifdef WITH_WRAP
#include <tcpd.h>
#endif

#ifndef DWORD
#define DWORD unsigned long
#endif

#include <controlobject.h>
#include <version.h>
#include <vscp.h>
#include <vscp_debug.h>
#include <vscpdatetime.h>
#include <vscphelper.h>

#include "tcpipsrv.h"

#define TCPIPSRV_INACTIVITY_TIMOUT (3600 * 12)

// Worker threads
void*
tcpipListenThread(void* pData);
void*
tcpipClientThread(void* pData);

///////////////////////////////////////////////////////////////////////////////
//                                  GLOBALS
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
//                               Listen thread
// ****************************************************************************

///////////////////////////////////////////////////////////////////////////////
// tcpipListenThreadObj
//
// This thread listens for connection on a TCP socket and starts a new thread
// to handle client requests
//

tcpipListenThreadObj::tcpipListenThreadObj(CControlObject* pobj)
{
    // Set the control object pointer
    setControlObjectPointer(pobj);

    m_strListeningPort = "9598";

    // Init. the server comtext structure
    memset(&m_srvctx, 0, sizeof(struct server_context));

    m_nStopTcpIpSrv = VSCP_TCPIP_SRV_RUN;
    m_idCounter     = 0;

    pthread_mutex_init(&m_mutexTcpClientList, NULL);
}

tcpipListenThreadObj::~tcpipListenThreadObj()
{
    pthread_mutex_destroy(&m_mutexTcpClientList);
}

///////////////////////////////////////////////////////////////////////////////
// tcpipListenThread
//

void*
tcpipListenThread(void* pData)
{
    size_t i;
    struct stcp_connection* conn;
    struct socket* psocket;
    struct stcp_secure_options opts;
    struct pollfd* pfd;
    memset(&opts, 0, sizeof(opts));
#ifdef WITH_WRAP
    struct request_info wrap_req;
    char address[1024];
#endif

    tcpipListenThreadObj* pListenObj = (tcpipListenThreadObj*)pData;
    if (NULL == pListenObj) {
        syslog(
          LOG_ERR,
          "TCP/IP client is missing client object data. Terinating thread.");
        return NULL;
    }

    // Fix pointer to main object
    CControlObject* pObj = pListenObj->getControlObject();

    // ------------------------------------------------------------------------

    // * * * Init. secure options * * *

    // Certificate
    if (pObj->m_tcpip_ssl_certificate.length()) {
        opts.pem = strdup((const char*)pObj->m_tcpip_ssl_certificate.c_str());
    }

    // Certificate chain
    if (pObj->m_tcpip_ssl_certificate_chain.length()) {
        opts.chain =
          strdup((const char*)pObj->m_tcpip_ssl_certificate_chain.c_str());
    }

    opts.verify_peer = pObj->m_tcpip_ssl_verify_peer;

    // CA path
    if (pObj->m_tcpip_ssl_ca_path.length()) {
        opts.ca_path = strdup((const char*)pObj->m_tcpip_ssl_ca_path.c_str());
    }

    // CA file
    if (pObj->m_tcpip_ssl_ca_file.length()) {
        opts.chain = strdup((const char*)pObj->m_tcpip_ssl_ca_file.c_str());
    }

    opts.verify_depth        = pObj->m_tcpip_ssl_verify_depth;
    opts.default_verify_path = pObj->m_tcpip_ssl_default_verify_paths ? 1 : 0;
    opts.protocol_version    = pObj->m_tcpip_ssl_protocol_version;

    // chiper list
    if (pObj->m_tcpip_ssl_cipher_list.length()) {
        opts.chipher_list =
          strdup((const char*)pObj->m_tcpip_ssl_cipher_list.c_str());
    }

    opts.short_trust = pObj->m_tcpip_ssl_short_trust ? 1 : 0;

    // Init. SSL subsystem
    if (pObj->m_tcpip_ssl_certificate.length()) {
        if (0 == stcp_init_ssl(pListenObj->m_srvctx.ssl_ctx, &opts)) {
            syslog(LOG_ERR, "[TCP/IP srv thread] Failed to init. ssl.\n");
            return NULL;
        }
    }

    // --------------------------------------------------------------------------------------

    // Bind to selected interface
    if (0 == stcp_listening(&pListenObj->m_srvctx,
                            pListenObj->m_strListeningPort.c_str())) {
        syslog(LOG_ERR, "[TCP/IP srv thread] Failed to init listening socket.");
        return NULL;
    }

    syslog(LOG_DEBUG, "[TCP/IP srv listen thread] Started.");

    while (!pListenObj->m_nStopTcpIpSrv) {

        pfd = pListenObj->m_srvctx.listening_socket_fds;
        memset(pfd, 0, sizeof(*pfd));
        for (i = 0; i < pListenObj->m_srvctx.num_listening_sockets; i++) {
            pfd[i].fd      = pListenObj->m_srvctx.listening_sockets[i].sock;
            pfd[i].events  = POLLIN;
            pfd[i].revents = 0;
        }

        int pollres;
        if ((pollres = stcp_poll(pfd,
                                 pListenObj->m_srvctx.num_listening_sockets,
                                 500,
                                 &(pListenObj->m_nStopTcpIpSrv))) > 0) {

            for (i = 0; i < pListenObj->m_srvctx.num_listening_sockets; i++) {

                // NOTE(lsm): on QNX, poll() returns POLLRDNORM after the
                // successful poll, and POLLIN is defined as
                // (POLLRDNORM | POLLRDBAND)
                // Therefore, we're checking pfd[i].revents & POLLIN, not
                // pfd[i].revents == POLLIN.
                if (pfd[i].revents & POLLIN) {

                    conn = stcp_new_connection(); // Init connection
                    if (NULL == conn) {
                        syslog(LOG_ERR,
                               "[TCP/IP srv] -- Memory problem when creating "
                               "conn object.");
                        continue;
                    }

                    memset(conn, 0, sizeof(struct stcp_connection));
                    conn->client.id = pListenObj->m_idCounter++;

                    if (stcp_accept(&pListenObj->m_srvctx,
                                    &pListenObj->m_srvctx.listening_sockets[i],
                                    &(conn->client))) {

                        stcp_init_client_connection(conn, &opts);
                        syslog(LOG_DEBUG, "[TCP/IP srv] -- Connection accept.");

#ifdef WITH_WRAP
                        /* Use tcpd / libwrap to determine whether a connection
                         * is allowed. */
                        request_init(&wrap_req,
                                     RQ_FILE,
                                     conn->client.sock,
                                     RQ_DAEMON,
                                     "vscpd",
                                     0);
                        fromhost(&wrap_req);
                        if (!hosts_access(&wrap_req)) {
                            // Access is denied
                            if (!stcp_socket_get_address(conn, address, 1024)) {
                                syslog(LOG_ERR,
                                       "Client connection from %s "
                                       "denied access by tcpd.",
                                       address);
                            }
                            stcp_close_connection(conn);
                            conn = NULL;
                            continue;
                        }
#endif

                        // Create the thread object
                        tcpipClientObj* pClientObj =
                          new tcpipClientObj(pListenObj);
                        if (NULL == pClientObj) {
                            syslog(LOG_ERR,
                                   "[TCP/IP srv] -- Memory problem when "
                                   "creating client thread.");
                            stcp_close_connection(conn);
                            conn = NULL;
                            continue;
                        }

                        pClientObj->m_conn    = conn;
                        pClientObj->m_pParent = pListenObj;

                        syslog(
                          LOG_DEBUG,
                          "Controlobject: Starting client tcp/ip thread...");

                        int err;
                        if ((err = pthread_create(&pClientObj->m_tcpipClientThread,
                                           NULL,
                                           tcpipClientThread,
                                           pClientObj))) {
                            syslog(LOG_ERR,
                                   "[TCP/IP srv] -- Failed to run client "
                                   "tcp/ip client thread. error=%d", err);
                            delete pClientObj;
                            stcp_close_connection(conn);
                            conn = NULL;
                            continue;
                        }

                        // Make it detached
                        pthread_detach(pClientObj->m_tcpipClientThread);

                        // Add conn to list of active connections
                        pthread_mutex_lock(&pListenObj->m_mutexTcpClientList);
                        pListenObj->m_tcpip_clientList.push_back(pClientObj);
                        pthread_mutex_unlock(&pListenObj->m_mutexTcpClientList);

                    } else {
                        delete psocket;
                        psocket = NULL;
                    }
                }

            } // for

        } // poll

        pollres = 0;

    } // While

    syslog(LOG_DEBUG, "[TCP/IP srv listen thread] Preparing Exit.");

    // Wait for clients to terminate
    int loopCnt = 0;
    while (true) {

        pthread_mutex_lock(&pListenObj->m_mutexTcpClientList);
        if (!pListenObj->m_tcpip_clientList.size())
            break;
        pthread_mutex_unlock(&pListenObj->m_mutexTcpClientList);

        loopCnt++;
        sleep(1); // Give them some time
    }

    stcp_close_all_listening_sockets(&pListenObj->m_srvctx);

    // * * * Deallocate allocated security options * * *

    if (NULL != opts.pem) {
        free((void*)opts.pem);
        opts.pem = NULL;
    }

    if (NULL != opts.chain) {
        free((void*)opts.chain);
        opts.chain = NULL;
    }

    if (NULL != opts.ca_path) {
        free((void*)opts.ca_path);
        opts.ca_path = NULL;
    }

    if (NULL != opts.ca_file) {
        free((void*)opts.ca_file);
        opts.ca_file = NULL;
    }

    if (NULL != opts.chipher_list) {
        free((void*)opts.chipher_list);
        opts.chipher_list = NULL;
    }

    if (pObj->m_tcpip_ssl_certificate.length()) {
        stcp_uninit_ssl();
    }

    syslog(LOG_DEBUG, "[TCP/IP srv listen thread] Exit.");

    return NULL;
}

// ****************************************************************************
//                              Client thread
// ****************************************************************************

///////////////////////////////////////////////////////////////////////////////
// tcpipClientObj
//
// This thread listens for connection on a TCP socket and starts a new thread
// to handle client requests
//

tcpipClientObj::tcpipClientObj(tcpipListenThreadObj* pParent)
{
    m_pClientItem = NULL;
    m_strResponse.clear();  // For clearness
    m_rv           = 0;     // No error code
    m_bReceiveLoop = false; // Not in receive loop
    m_conn         = NULL;  // No connection yet
    m_pObj         = NULL;
    pParent        = pParent;

    if (NULL != pParent) {
        m_pObj = pParent->getControlObject();
    }
}

tcpipClientObj::~tcpipClientObj()
{
    pthread_join(m_tcpipClientThread, NULL);
    m_commandArray.clear(); // TODO remove strings
}

///////////////////////////////////////////////////////////////////////////////
// write
//

bool
tcpipClientObj::write(std::string& str, bool bAddCRLF)
{
    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state)
        return false;

    if (bAddCRLF) {
        str += std::string("\r\n");
    }

    m_rv = stcp_write(m_conn, (const char*)str.c_str(), str.length());
    if (m_rv != str.length())
        return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// write
//

bool
tcpipClientObj::write(const char* buf, size_t len)
{
    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state)
        return false;

    m_rv = stcp_write(m_conn, (const char*)buf, len);
    if (m_rv != len)
        return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// read
//

bool
tcpipClientObj::read(std::string& str)
{
    size_t pos;

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state)
        return false;

    if (m_strResponse.npos != (pos = m_strResponse.find('\n'))) {

        // Get the string
        str = m_strResponse.substr(pos + 1);
        vscp_trim(str);

        // Remove string from buffer
        m_strResponse = m_strResponse.substr(m_strResponse.length() - pos - 1);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// CommandHandler
//

int
tcpipClientObj::CommandHandler(std::string& strCommand)
{
    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) {
        return VSCP_TCPIP_RV_ERROR;
    }

    if (NULL == m_pObj) {
        syslog(LOG_ERR,
               "[TCP/IP srv] ERROR: Control object pointer is NULL in command "
               "handler.");
        return VSCP_TCPIP_RV_CLOSE; // Close connection
    }

    if (NULL == m_pClientItem) {
        syslog(
          LOG_ERR,
          "[TCP/IP srv] ERROR: ClientItem pointer is NULL in command handler.");
        return VSCP_TCPIP_RV_CLOSE; // Close connection
    }

    m_pClientItem->m_currentCommand = strCommand;
    vscp_trim(m_pClientItem->m_currentCommand);

    // If nothing to handle just return
    if (0 == m_pClientItem->m_currentCommand.length()) {
        write(MSG_OK, strlen(MSG_OK));
        return VSCP_TCPIP_RV_OK;
    }

    //*********************************************************************
    //                            No Operation
    //*********************************************************************

    if (m_pClientItem->CommandStartsWith(("noop"))) {
        write(MSG_OK, strlen(MSG_OK));
        return VSCP_TCPIP_RV_OK;
    }

    //*********************************************************************
    //                             Rcvloop
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("rcvloop")) ||
             m_pClientItem->CommandStartsWith(("receiveloop"))) {
        if (checkPrivilege(VSCP_USER_RIGHT_ALLOW_RCV_EVENT)) {
            try {
                m_pClientItem->m_timeRcvLoop = time(NULL);
                handleClientRcvLoop();
            } catch (...) {
                syslog(LOG_ERR,
                       "TCPIP: Exception occurred handleClientRcvLoop");
            }
        }
    }

    //*********************************************************************
    //                             Quitloop
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("quitloop"))) {
        m_bReceiveLoop = false;
        write(MSG_QUIT_LOOP, strlen(MSG_QUIT_LOOP));
    }

    //*********************************************************************
    //                             Username
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("user"))) {
        try {
            handleClientUser();
        } catch (...) {
            syslog(LOG_ERR, "TCPIP: Exception occurred handleClientUser");
        }
    }

    //*********************************************************************
    //                            Password
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("pass"))) {

        try {
            if (!handleClientPassword()) {
                syslog(LOG_ERR,
                       "[TCP/IP srv] Command: Password. Not authorized.");
                return VSCP_TCPIP_RV_CLOSE; // Close connection
            }
        } catch (...) {
            syslog(LOG_ERR, "TCPIP: Exception occurred handleClientPassword");
        }

        if (__VSCP_DEBUG_TCP) {
            syslog(LOG_DEBUG, "[TCP/IP srv] Command: Password. PASS");
        }

    }

    //*********************************************************************
    //                              Challenge
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("challenge"))) {
        try {
            handleChallenge();
        } catch (...) {
            syslog(LOG_ERR, "TCPIP: Exception occurred handleChallange");
        }
    }

    // *********************************************************************
    //                                 QUIT
    // *********************************************************************

    else if (m_pClientItem->CommandStartsWith("quit") ||
             m_pClientItem->CommandStartsWith("exit")) {

        if (__VSCP_DEBUG_TCP) {
            syslog(LOG_INFO, "[TCP/IP srv] Command: Close.");
        }

        write(MSG_GOODBY, strlen(MSG_GOODBY));

        return VSCP_TCPIP_RV_CLOSE; // Close connection
    }

    //*********************************************************************
    //                              Shutdown
    //*********************************************************************
    else if (m_pClientItem->CommandStartsWith(("shutdown"))) {
        if (checkPrivilege(VSCP_USER_RIGHT_ALLOW_SHUTDOWN)) {
            try {
                handleClientShutdown();
            } catch (...) {
                syslog(LOG_ERR,
                       "TCPIP: Exception occurred handleClientShutdown");
            }
        }
    }

    //*********************************************************************
    //                             Send event
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("send"))) {
        if (checkPrivilege(VSCP_USER_RIGHT_ALLOW_SEND_EVENT)) {
            try {
                handleClientSend();
            } catch (...) {
                syslog(LOG_ERR, "TCPIP: Exception occurred handleClientSend");
            }
        }
    }

    //*********************************************************************
    //                            Read event
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("retr")) ||
             m_pClientItem->CommandStartsWith(("retrieve"))) {
        if (checkPrivilege(VSCP_USER_RIGHT_ALLOW_RCV_EVENT)) {
            try {
                handleClientReceive();
            } catch (...) {
                syslog(LOG_ERR,
                       "TCPIP: Exception occurred handleClientReceive");
            }
        }
    }

    //*********************************************************************
    //                            Data Available
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("cdta")) ||
             m_pClientItem->CommandStartsWith(("chkdata")) ||
             m_pClientItem->CommandStartsWith(("checkdata"))) {
        try {
            handleClientDataAvailable();
        } catch (...) {
            syslog(LOG_ERR,
                   "TCPIP: Exception occurred handleClientDataAvailable");
        }
    }

    //*********************************************************************
    //                          Clear input queue
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("clra")) ||
             m_pClientItem->CommandStartsWith(("clearall")) ||
             m_pClientItem->CommandStartsWith(("clrall"))) {
        try {
            handleClientClearInputQueue();
        } catch (...) {
            syslog(LOG_ERR,
                   "TCPIP: Exception occurred handleClientClearInputQueue");
        }
    }

    //*********************************************************************
    //                           Get Statistics
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("stat"))) {
        try {
            handleClientGetStatistics();
        } catch (...) {
            syslog(LOG_ERR,
                   "TCPIP: Exception occurred handleClientGetStatistics");
        }
    }

    //*********************************************************************
    //                            Get Status
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("info"))) {
        try {
            handleClientGetStatus();
        } catch (...) {
            syslog(LOG_ERR, "TCPIP: Exception occurred handleClientGetStatus");
        }
    }

    //*********************************************************************
    //                           Get Channel ID
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("chid")) ||
             m_pClientItem->CommandStartsWith(("getchid"))) {
        try {
            handleClientGetChannelID();
        } catch (...) {
            syslog(LOG_ERR,
                   "TCPIP: Exception occurred handleClientGetChannelID");
        }
    }

    //*********************************************************************
    //                          Set Channel GUID
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("sgid")) ||
             m_pClientItem->CommandStartsWith(("setguid"))) {
        if (checkPrivilege(VSCP_USER_RIGHT_ALLOW_SETGUID)) {
            try {
                handleClientSetChannelGUID();
            } catch (...) {
                syslog(LOG_ERR,
                       "TCPIP: Exception occurred handleClientSetChannelGUID");
            }
        }
    }

    //*********************************************************************
    //                          Get Channel GUID
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("ggid")) ||
             m_pClientItem->CommandStartsWith(("getguid"))) {
        try {
            handleClientGetChannelGUID();
        } catch (...) {
            syslog(LOG_ERR,
                   "TCPIP: Exception occurred handleClientGetChannelGUID");
        }
    }

    //*********************************************************************
    //                           Get Version
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("version")) ||
             m_pClientItem->CommandStartsWith(("vers"))) {
        try {
            handleClientGetVersion();
        } catch (...) {
            syslog(LOG_ERR, "TCPIP: Exception occurred handleClientGetVersion");
        }
    }

    //*********************************************************************
    //                           Set Filter
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("sflt")) ||
             m_pClientItem->CommandStartsWith(("setfilter"))) {
        if (checkPrivilege(VSCP_USER_RIGHT_ALLOW_SETFILTER)) {
            try {
                handleClientSetFilter();
            } catch (...) {
                syslog(LOG_ERR,
                       "TCPIP: Exception occurred handleClientSetFilter");
            }
        }
    }

    //*********************************************************************
    //                           Set Mask
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("smsk")) ||
             m_pClientItem->CommandStartsWith(("setmask"))) {
        if (checkPrivilege(VSCP_USER_RIGHT_ALLOW_SETFILTER)) {
            try {
                handleClientSetMask();
            } catch (...) {
                syslog(LOG_ERR,
                       "TCPIP: Exception occurred handleClientSetMask");
            }
        }
    }

    //*********************************************************************
    //                             Help
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("help"))) {
        try {
            handleClientHelp();
        } catch (...) {
            syslog(LOG_ERR, "TCPIP: Exception occurred handleClientHelp");
        }
    }

    //*********************************************************************
    //                             Restart
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("restart"))) {
        if (checkPrivilege(VSCP_USER_RIGHT_ALLOW_RESTART)) {
            try {
                handleClientRestart();
            } catch (...) {
                syslog(LOG_ERR,
                       "TCPIP: Exception occurred handleClientRestart");
            }
        }
    }

    //*********************************************************************
    //                         Client/interface
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("client")) ||
             m_pClientItem->CommandStartsWith(("interface"))) {
        if (checkPrivilege(VSCP_USER_RIGHT_ALLOW_INTERFACE)) {
            try {
                handleClientInterface();
            } catch (...) {
                syslog(LOG_ERR,
                       "TCPIP: Exception occurred handleClientInterface");
            }
        }
    }

    //*********************************************************************
    //                               Test
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("test"))) {
        if (checkPrivilege(VSCP_USER_RIGHT_ALLOW_TEST)) {
            try {
                handleClientTest();
            } catch (...) {
                syslog(LOG_ERR, "TCPIP: Exception occurred handleClientTest");
            }
        }
    }

    //*********************************************************************
    //                             WhatCanYouDo
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("wcyd")) ||
             m_pClientItem->CommandStartsWith(("whatcanyoudo"))) {
        try {
            handleClientCapabilityRequest();
        } catch (...) {
            syslog(LOG_ERR,
                   "TCPIP: Exception occurred handleClientCapabilityRequest");
        }
    }

    //*********************************************************************
    //                             Measurement
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("measurement"))) {
        try {
            handleClientMeasurement();
        } catch (...) {
            syslog(LOG_ERR,
                   "TCPIP: Exception occurred handleClientMeasurement");
        }
    }

    //*********************************************************************
    //                                What?
    //*********************************************************************
    else {
        write(MSG_UNKNOWN_COMMAND, strlen(MSG_UNKNOWN_COMMAND));
    }

    m_pClientItem->m_lastCommand = m_pClientItem->m_currentCommand;
    return VSCP_TCPIP_RV_OK;

} // clientcommand

///////////////////////////////////////////////////////////////////////////////
// handleClientMeasurement
//
// format,level,vscp-measurement-type,value,unit,guid,sensoridx,zone,subzone,dest-guid
//
// format                   float|string|0|1 - float=0, string=1.
// level                    level2|1|level1|0  1 = VSCP Level I event, 2 = VSCP
// Level II event. vscp-measurement-type    A valid vscp measurement type. value
// A floating point value. (use $ prefix for variable followed by name) unit
// Optional unit for this type. Default = 0. guid                     Optional
// GUID (or "-"). Default is "-". sensoridx                Optional sensor
// index. Default is 0. zone                     Optional zone. Default is 0.
// subzone                  Optional subzone- Default is 0.
// dest-guid                Optional destination GUID. For Level I over Level
// II.
//

void
tcpipClientObj::handleClientMeasurement(void)
{
    std::string str;
    double value = 0;
    cguid guid;     // Initialized to zero
    cguid destguid; // Initialized to zero
    long level = VSCP_LEVEL2;
    long unit  = 0;
    long vscptype;
    long sensoridx   = 0;
    long zone        = 0;
    long subzone     = 0;
    long eventFormat = 0; // float
    uint8_t data[VSCP_MAX_DATA];
    uint16_t sizeData;

    // Check object pointer
    if (NULL == m_pObj) {
        write(MSG_INTERNAL_MEMORY_ERROR, strlen(MSG_INTERNAL_MEMORY_ERROR));
        return;
    }

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state)
        return;

    if (NULL == m_pClientItem) {
        write(MSG_INTERNAL_MEMORY_ERROR, strlen(MSG_INTERNAL_MEMORY_ERROR));
        return;
    }

    std::deque<std::string> tokens;
    vscp_split(tokens, m_pClientItem->m_currentCommand, ",");

    // * * * event format * * *

    // Get event format (float | string | 0 | 1 - float=0, string=1.)
    if (tokens.empty()) {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    str = tokens.front();
    tokens.pop_front();

    vscp_trim(str);
    vscp_makeUpper(str);

    // Handle float=0
    if ('0' == str[0]) {
        eventFormat = 0;
    }
    // Handle string=1
    else if ('1' == str[0]) {
        eventFormat = 1;
    } else if (str == "STRING") {
        eventFormat = 1;
    } else if (str == "FLOAT") {
        eventFormat = 0;
    } else {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // * * * Level * * *

    if (tokens.empty()) {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    str = tokens.front();
    tokens.pop_front();

    vscp_trim(str);
    vscp_makeUpper(str);

    if ('0' == str[0]) {
        level = VSCP_LEVEL1;
    } else if ('1' == str[0]) {
        level = VSCP_LEVEL2;
    } else if (str == "LEVEL1") {
        level = VSCP_LEVEL1;
    } else if (str == "LEVEL2") {
        level = VSCP_LEVEL2;
    } else {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // * * * vscp-measurement-type * * *
    if (tokens.empty()) {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    str = tokens.front();
    tokens.pop_front();
    vscptype = vscp_readStringValue(str);

    // * * * value * * *
    if (tokens.empty()) {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    str = tokens.front();
    tokens.pop_front();
    vscp_trim(str);

    value = std::stod(str);

    // * * * unit * * *

    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        unit = vscp_readStringValue(str);
    }

    // * * * guid * * *

    if (!tokens.empty()) {

        str = tokens.front();
        tokens.pop_front();
        vscp_trim(str);

        // If empty set to default.
        if (0 == str.length())
            str = "-";
        guid.getFromString(str);
    }

    // * * * sensor index * * *

    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        vscp_trim(str);

        sensoridx = vscp_readStringValue(str);
    }

    // * * * zone * * *

    if (!tokens.empty()) {

        str = tokens.front();
        tokens.pop_front();
        vscp_trim(str);

        zone = vscp_readStringValue(str);
        ;
        zone &= 0xff;
    }

    // * * * subzone * * *

    if (!tokens.empty()) {

        str = tokens.front();
        tokens.pop_front();
        vscp_trim(str);

        subzone = vscp_readStringValue(str);
        subzone &= 0xff;
    }

    // * * * destguid * * *

    if (!tokens.empty()) {

        str = tokens.front();
        tokens.pop_front();
        vscp_trim(str);

        // If empty set to default.
        if (0 == str.length())
            str = ("-");
        destguid.getFromString(str);
    }

    // Range checks
    if (VSCP_LEVEL1 == level) {
        if (unit > 3)
            unit = 0;
        if (sensoridx > 7)
            unit = 0;
        if (vscptype > 512)
            vscptype -= 512;
    } else { // VSCP_LEVEL2
        if (unit > 255)
            unit &= 0xff;
        if (sensoridx > 255)
            sensoridx &= 0xff;
    }

    if (1 == level) { // Level I

        if (0 == eventFormat) {

            // * * * Floating point * * *

            if (vscp_convertFloatToFloatEventData(data,
                                                  &sizeData,
                                                  value,
                                                  unit,
                                                  sensoridx)) {
                if (sizeData > 8)
                    sizeData = 8;

                vscpEvent* pEvent = new vscpEvent;
                if (NULL == pEvent) {
                    write(MSG_INTERNAL_MEMORY_ERROR,
                          strlen(MSG_INTERNAL_MEMORY_ERROR));
                    return;
                }

                pEvent->pdata     = NULL;
                pEvent->head      = VSCP_PRIORITY_NORMAL;
                pEvent->timestamp = 0; // Let interface fill in
                                       // Will fill in date/time block also
                guid.writeGUID(pEvent->GUID);
                pEvent->sizeData = sizeData;
                if (sizeData > 0) {
                    pEvent->pdata = new uint8_t[sizeData];
                    memcpy(pEvent->pdata, data, sizeData);
                }
                pEvent->vscp_class = VSCP_CLASS1_MEASUREMENT;
                pEvent->vscp_type  = vscptype;

                // send the event
                if (!m_pObj->sendEvent(m_pClientItem, pEvent)) {
                    vscp_deleteEvent_v2(&pEvent);
                    write(MSG_UNABLE_TO_SEND_EVENT,
                          strlen(MSG_UNABLE_TO_SEND_EVENT));
                    return;
                }

                vscp_deleteEvent_v2(&pEvent);

            } else {
                write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            }
        } else {

            // * * * String * * *

            vscpEvent* pEvent = new vscpEvent;
            if (NULL == pEvent) {
                write(MSG_INTERNAL_MEMORY_ERROR,
                      strlen(MSG_INTERNAL_MEMORY_ERROR));
                return;
            }

            pEvent->pdata = NULL;

            if (!vscp_makeStringMeasurementEvent(pEvent,
                                                 value,
                                                 unit,
                                                 sensoridx)) {
                write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            }

            // TODO have to send also
        }
    } else { // Level II

        if (0 == eventFormat) { // float and Level II

            // * * * Floating point * * *

            vscpEvent* pEvent = new vscpEvent;
            if (NULL == pEvent) {
                write(MSG_INTERNAL_MEMORY_ERROR,
                      strlen(MSG_INTERNAL_MEMORY_ERROR));
                return;
            }

            pEvent->pdata = NULL;

            pEvent->obid      = 0;
            pEvent->head      = VSCP_PRIORITY_NORMAL;
            pEvent->timestamp = 0; // Let interface fill in timestamp
                                   // Will fill in date/time block also
            guid.writeGUID(pEvent->GUID);
            pEvent->head       = 0;
            pEvent->vscp_class = VSCP_CLASS2_MEASUREMENT_FLOAT;
            pEvent->vscp_type  = vscptype;
            pEvent->sizeData   = 12;

            data[0] = sensoridx;
            data[1] = zone;
            data[2] = subzone;
            data[3] = unit;

            memcpy(data + 4, (uint8_t*)&value, 8); // copy in double
            uint64_t temp = VSCP_UINT64_SWAP_ON_LE(*(data + 4));
            memcpy(data + 4, (void*)&temp, 8);

            // Copy in data
            pEvent->pdata = new uint8_t[4 + 8];
            if (NULL == pEvent->pdata) {
                write(MSG_INTERNAL_MEMORY_ERROR,
                      strlen(MSG_INTERNAL_MEMORY_ERROR));
                delete pEvent;
                return;
            }

            memcpy(pEvent->pdata, data, 4 + 8);

            // send the event
            if (!m_pObj->sendEvent(m_pClientItem, pEvent)) {
                vscp_deleteEvent_v2(&pEvent);
                write(MSG_UNABLE_TO_SEND_EVENT,
                      strlen(MSG_UNABLE_TO_SEND_EVENT));
                return;
            }

            vscp_deleteEvent_v2(&pEvent);

        } else { // string & Level II

            // * * * String * * *

            vscpEvent* pEvent = new vscpEvent;
            pEvent->pdata     = NULL;

            pEvent->obid      = 0;
            pEvent->head      = VSCP_PRIORITY_NORMAL;
            pEvent->timestamp = 0; // Let interface fill in
                                   // Will fill in date/time block also
            guid.writeGUID(pEvent->GUID);
            pEvent->head       = 0;
            pEvent->vscp_class = VSCP_CLASS2_MEASUREMENT_STR;
            pEvent->vscp_type  = vscptype;
            pEvent->sizeData   = 12;

            std::string strValue = vscp_str_format("%f", value);

            data[0] = sensoridx;
            data[1] = zone;
            data[2] = subzone;
            data[3] = unit;

            pEvent->pdata = new uint8_t[4 + strValue.length()];
            if (NULL == pEvent->pdata) {
                write(MSG_INTERNAL_MEMORY_ERROR,
                      strlen(MSG_INTERNAL_MEMORY_ERROR));
                delete pEvent;
                return;
            }
            memcpy(data + 4,
                   strValue.c_str(),
                   strValue.length()); // copy in double

            // send the event
            if (!m_pObj->sendEvent(m_pClientItem, pEvent)) {
                vscp_deleteEvent_v2(&pEvent);
                write(MSG_UNABLE_TO_SEND_EVENT,
                      strlen(MSG_UNABLE_TO_SEND_EVENT));
                return;
            }

            vscp_deleteEvent_v2(&pEvent);
        }
    }

    write(MSG_OK, strlen(MSG_OK));
}

///////////////////////////////////////////////////////////////////////////////
// handleClientCapabilityRequest
//

void
tcpipClientObj::handleClientCapabilityRequest(void)
{
    std::string str;
    uint8_t capabilities[8];

    m_pObj->getVscpCapabilities(capabilities);
    str = vscp_str_format("%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X\r\n",
                          capabilities[7],
                          capabilities[6],
                          capabilities[5],
                          capabilities[4],
                          capabilities[3],
                          capabilities[2],
                          capabilities[1],
                          capabilities[0]);
    write(str.c_str(), str.length());
    write(MSG_OK, strlen(MSG_OK));
}

///////////////////////////////////////////////////////////////////////////////
// isVerified
//

bool
tcpipClientObj::isVerified(void)
{
    // Check object
    if (NULL == m_pObj) {
        return false;
    }

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state)
        return false;

    // Must be accredited to do this
    if (!m_pClientItem->bAuthenticated) {
        write(MSG_NOT_ACCREDITED, strlen(MSG_NOT_ACCREDITED));
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// checkPrivilege
//

bool
tcpipClientObj::checkPrivilege(unsigned long reqiredPrivilege)
{
    // Check objects
    if (NULL == m_pObj) {
        return false;
    }

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state)
        return false;

    // Must be authenticated
    if (!m_pClientItem->bAuthenticated) {
        write(MSG_NOT_ACCREDITED, strlen(MSG_NOT_ACCREDITED));
        return false;
    }

    // Must be accredited
    if (NULL == m_pClientItem->m_pUserItem) {
        write(MSG_NOT_ACCREDITED, strlen(MSG_NOT_ACCREDITED));
        return false;
    }

    // Check the privileges
    if (!(m_pClientItem->m_pUserItem->getUserRights() & reqiredPrivilege)) {
        write(MSG_NO_RIGHTS_ERROR, strlen(MSG_NO_RIGHTS_ERROR));
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// handleClientSend
//

void
tcpipClientObj::handleClientSend(void)
{
    vscpEvent event;

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state)
        return;

    // Set timestamp block for event
    vscp_setEventDateTimeBlockToNow(&event);  // TODO - change to UTC

    if (NULL == m_pObj) {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    if (NULL == m_pClientItem) {
        write(MSG_INTERNAL_ERROR, strlen(MSG_INTERNAL_ERROR));
        return;
    }

    // Must be accredited to do this
    if (!m_pClientItem->bAuthenticated) {
        write(MSG_NOT_ACCREDITED, strlen(MSG_NOT_ACCREDITED));
        return;
    }

    std::string str;
    std::deque<std::string> tokens;
    vscp_split(tokens, m_pClientItem->m_currentCommand, ",");

    // If first character is $ user request us to send content from
    // a variable

    if (!tokens.empty()) {
        // Get Head
        str = tokens.front();
        tokens.pop_front();
        vscp_trim(str);
        event.head = vscp_readStringValue(str);
    } else {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // Get Class
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        event.vscp_class = vscp_readStringValue(str);
    } else {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // Get Type
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        event.vscp_type = vscp_readStringValue(str);
    } else {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // Get OBID  -  Kept here to be compatible with receive
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        event.obid = vscp_readStringValue(str);
    } else {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // Get date/time - can be empty
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        vscp_trim(str);
        if (str.length()) {
            vscpdatetime dt;
            if (dt.set(str)) {
                event.year   = dt.getYear();
                event.month  = dt.getMonth();
                event.day    = dt.getDay();
                event.hour   = dt.getHour();
                event.minute = dt.getMinute();
                event.second = dt.getSecond();
            } else {
                vscp_setEventDateTimeBlockToNow(&event);
            }
        } else {
            // set current time
            vscp_setEventDateTimeBlockToNow(&event);
        }

    } else {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // Get Timestamp - can be empty
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        vscp_trim(str);
        if (str.length()) {
            event.timestamp = vscp_readStringValue(str);
        } else {
            event.timestamp = vscp_makeTimeStamp();
        }
    } else {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // Get GUID
    std::string strGUID;
    if (!tokens.empty()) {
        
        strGUID = tokens.front();
        tokens.pop_front();

        // Check if i/f GUID should be used
        if ('-' == strGUID[0]) {
            // Copy in the i/f GUID
            m_pClientItem->m_guid.writeGUID(event.GUID);
        } else {
            vscp_setEventGuidFromString(&event, strGUID);

            // Check if i/f GUID should be used
            if (true == vscp_isGUIDEmpty(event.GUID)) {
                // Copy in the i/f GUID
                m_pClientItem->m_guid.writeGUID(event.GUID);
            }
        }
    } else {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // Handle data
    if (VSCP_MAX_DATA < tokens.size()) {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    event.sizeData = tokens.size();

    if (event.sizeData > 0) {

        unsigned int index = 0;

        event.pdata = new uint8_t[event.sizeData];

        if (NULL == event.pdata) {
            write(MSG_INTERNAL_MEMORY_ERROR, strlen(MSG_INTERNAL_MEMORY_ERROR));
            return;
        }

        while (!tokens.empty() && (event.sizeData > index)) {
            str = tokens.front();
            tokens.pop_front();
            event.pdata[index++] = vscp_readStringValue(str);
        }

        if (!tokens.empty()) {
            write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));

            delete [] event.pdata;
            event.pdata = NULL;
            return;
        }
    } else {
        // No data
        event.pdata = NULL;
    }

    // Check if we are allowed to send CLASS1.PROTOCOL events
    if ((VSCP_CLASS1_PROTOCOL == event.vscp_class) &&
        !checkPrivilege(VSCP_USER_RIGHT_ALLOW_SEND_L1CTRL_EVENT)) {

        std::string strErr = vscp_str_format(
          ("[TCP/IP srv] User [%s] not allowed to send event class=%d "
           "type=%d.\n"),
          (const char*)m_pClientItem->m_pUserItem->getUserName().c_str(),
          event.vscp_class,
          event.vscp_type);

        syslog(LOG_ERR, "%s", strErr.c_str());

        write(MSG_MOT_ALLOWED_TO_SEND_EVENT,
              strlen(MSG_MOT_ALLOWED_TO_SEND_EVENT));

        if (NULL != event.pdata) {
            delete [] event.pdata;
            event.pdata = NULL;
        }

        return;
    }

    // Check if we are allowed top send CLASS1.PROTOCOL events
    if ((VSCP_CLASS1_PROTOCOL == event.vscp_class) &&
        !checkPrivilege(VSCP_CLASS2_LEVEL1_PROTOCOL)) {

        std::string strErr = vscp_str_format(
          ("[TCP/IP srv] User [%s] not allowed to send event class=%d "
           "type=%d.\n"),
          (const char*)m_pClientItem->m_pUserItem->getUserName().c_str(),
          event.vscp_class,
          event.vscp_type);

        syslog(LOG_ERR, "%s", strErr.c_str());

        write(MSG_MOT_ALLOWED_TO_SEND_EVENT,
              strlen(MSG_MOT_ALLOWED_TO_SEND_EVENT));

        if (NULL != event.pdata) {
            delete [] event.pdata;
            event.pdata = NULL;
        }

        return;
    }

    // Check if we are allowed top send CLASS2.PROTOCOL events
    if ((VSCP_CLASS2_PROTOCOL == event.vscp_class) &&
        !checkPrivilege(VSCP_USER_RIGHT_ALLOW_SEND_L2CTRL_EVENT)) {

        std::string strErr = vscp_str_format(
          ("[TCP/IP srv] User [%s] not allowed to send event class=%d "
           "type=%d.\n"),
          (const char*)m_pClientItem->m_pUserItem->getUserName().c_str(),
          event.vscp_class,
          event.vscp_type);

        syslog(LOG_ERR, "%s", strErr.c_str());

        write(MSG_MOT_ALLOWED_TO_SEND_EVENT,
              strlen(MSG_MOT_ALLOWED_TO_SEND_EVENT));

        if (NULL != event.pdata) {
            delete [] event.pdata;
            event.pdata = NULL;
        }

        return;
    }

    // Check if we are allowed top send CLASS2.HLO events
    if ((VSCP_CLASS2_HLO == event.vscp_class) &&
        !checkPrivilege(VSCP_USER_RIGHT_ALLOW_SEND_HLO_EVENT)) {

        std::string strErr = vscp_str_format(
          ("[TCP/IP srv] User [%s] not allowed to send event class=%d "
           "type=%d.\n"),
          (const char*)m_pClientItem->m_pUserItem->getUserName().c_str(),
          event.vscp_class,
          event.vscp_type);

        syslog(LOG_ERR, "%s", strErr.c_str());

        write(MSG_MOT_ALLOWED_TO_SEND_EVENT,
              strlen(MSG_MOT_ALLOWED_TO_SEND_EVENT));

        if (NULL != event.pdata) {
            delete [] event.pdata;
            event.pdata = NULL;
        }

        return;
    }

    // Check if this user is allowed to send this event
    if (!m_pClientItem->m_pUserItem->isUserAllowedToSendEvent(
          event.vscp_class,
          event.vscp_type)) {

        std::string strErr = vscp_str_format(
          ("[TCP/IP srv] User [%s] not allowed to send event class=%d "
           "type=%d.\n"),
          (const char*)m_pClientItem->m_pUserItem->getUserName().c_str(),
          event.vscp_class,
          event.vscp_type);

        syslog(LOG_ERR, "%s", strErr.c_str());

        write(MSG_MOT_ALLOWED_TO_SEND_EVENT,
              strlen(MSG_MOT_ALLOWED_TO_SEND_EVENT));

        if (NULL != event.pdata) {
            delete [] event.pdata;
            event.pdata = NULL;
        }

        return;
    }

    // send event
    if (!m_pObj->sendEvent(m_pClientItem, &event)) {
        vscp_deleteEvent(&event); // Deallocate data
        write(MSG_BUFFER_FULL, strlen(MSG_BUFFER_FULL));
        return;
    }
    vscp_deleteEvent(&event); // Deallocate data

    write(MSG_OK, strlen(MSG_OK));
}

///////////////////////////////////////////////////////////////////////////////
// handleClientReceive
//

void
tcpipClientObj::handleClientReceive(void)
{
    unsigned short cnt = 0; // # of messages to read

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state)
        return;

    // Must be accredited to do this
    if (!m_pClientItem->bAuthenticated) {
        write(MSG_NOT_ACCREDITED, strlen(MSG_NOT_ACCREDITED));
        return;
    }

    std::string str;
    cnt = vscp_readStringValue(m_pClientItem->m_currentCommand);

    if (!cnt) {
        cnt = 1; // No arg is "read one"
    }

    // Read cnt messages
    while (cnt) {

        std::string strOut;

        if (!m_pClientItem->m_bOpen) {
            write(MSG_NO_MSG, strlen(MSG_NO_MSG));
            return;
        } else {
            if (false == sendOneEventFromQueue()) {
                return;
            }
        }

        cnt--;

    } // while

    write(MSG_OK, strlen(MSG_OK));
}

///////////////////////////////////////////////////////////////////////////////
// sendOneEventFromQueue
//

bool
tcpipClientObj::sendOneEventFromQueue(bool bStatusMsg)
{
    std::string strOut;

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state)
        return false;

    if (m_pClientItem->m_clientInputQueue.size()) {

        vscpEvent* pqueueEvent;
        pthread_mutex_lock(&m_pClientItem->m_mutexClientInputQueue);
        {
            pqueueEvent = m_pClientItem->m_clientInputQueue.front();
            m_pClientItem->m_clientInputQueue.pop_front();
        }
        pthread_mutex_unlock(&m_pClientItem->m_mutexClientInputQueue);

        vscp_convertEventToString(strOut, pqueueEvent);
        strOut += ("\r\n");
        write(strOut.c_str(), strlen(strOut.c_str()));

        vscp_deleteEvent_v2(&pqueueEvent);

    } else {
        if (bStatusMsg) {
            write(MSG_NO_MSG, strlen(MSG_NO_MSG));
        }

        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// handleClientDataAvailable
//

void
tcpipClientObj::handleClientDataAvailable(void)
{
    char outbuf[1024];

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state)
        return;

    // Must be accredited to do this
    if (!m_pClientItem->bAuthenticated) {
        write(MSG_NOT_ACCREDITED, strlen(MSG_NOT_ACCREDITED));
        return;
    }

    sprintf(outbuf,
            "%zd\r\n%s",
            m_pClientItem->m_clientInputQueue.size(),
            MSG_OK);
    write(outbuf, strlen(outbuf));
}

///////////////////////////////////////////////////////////////////////////////
// handleClientClearInputQueue
//

void
tcpipClientObj::handleClientClearInputQueue(void)
{
    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state)
        return;

    // Must be accredited to do this
    if (!m_pClientItem->bAuthenticated) {
        write(MSG_NOT_ACCREDITED, strlen(MSG_NOT_ACCREDITED));
        return;
    }

    pthread_mutex_lock(&m_pClientItem->m_mutexClientInputQueue);
    std::deque<vscpEvent*>::iterator iter;
    for (iter = m_pClientItem->m_clientInputQueue.begin();
         iter != m_pClientItem->m_clientInputQueue.end();
         ++iter) {
        vscpEvent* pEvent = *iter;
        vscp_deleteEvent_v2(&pEvent);
    }
    m_pClientItem->m_clientInputQueue.clear();
    pthread_mutex_unlock(&m_pClientItem->m_mutexClientInputQueue);

    write(MSG_QUEUE_CLEARED, strlen(MSG_QUEUE_CLEARED));
}

///////////////////////////////////////////////////////////////////////////////
// handleClientGetStatistics
//

void
tcpipClientObj::handleClientGetStatistics(void)
{
    char outbuf[1024];

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state)
        return;

    // Must be accredited to do this
    if (!m_pClientItem->bAuthenticated) {
        write(MSG_NOT_ACCREDITED, strlen(MSG_NOT_ACCREDITED));
        return;
    }

    sprintf(outbuf,
            "%lu,%lu,%lu,%lu,%lu,%lu,%lu\r\n%s",
            m_pClientItem->m_statistics.cntBusOff,
            m_pClientItem->m_statistics.cntBusWarnings,
            m_pClientItem->m_statistics.cntOverruns,
            m_pClientItem->m_statistics.cntReceiveData,
            m_pClientItem->m_statistics.cntReceiveFrames,
            m_pClientItem->m_statistics.cntTransmitData,
            m_pClientItem->m_statistics.cntTransmitFrames,
            MSG_OK);

    write(outbuf, strlen(outbuf));
}

///////////////////////////////////////////////////////////////////////////////
// handleClientGetStatus
//

void
tcpipClientObj::handleClientGetStatus(void)
{
    char outbuf[1024];

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state)
        return;

    // Must be accredited to do this
    if (!m_pClientItem->bAuthenticated) {
        write(MSG_NOT_ACCREDITED, strlen(MSG_NOT_ACCREDITED));
        return;
    }

    sprintf(outbuf,
            "%lu,%lu,%lu,\"%s\"\r\n%s",
            m_pClientItem->m_status.channel_status,
            m_pClientItem->m_status.lasterrorcode,
            m_pClientItem->m_status.lasterrorsubcode,
            m_pClientItem->m_status.lasterrorstr,
            MSG_OK);

    write(outbuf, strlen(outbuf));
}

///////////////////////////////////////////////////////////////////////////////
// handleClientGetChannelID
//

void
tcpipClientObj::handleClientGetChannelID(void)
{
    char outbuf[1024];

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state)
        return;

    // Must be accredited to do this
    if (!m_pClientItem->bAuthenticated) {
        write(MSG_NOT_ACCREDITED, strlen(MSG_NOT_ACCREDITED));
        return;
    }

    sprintf(outbuf,
            "%lu\r\n%s",
            (unsigned long)m_pClientItem->m_clientID,
            MSG_OK);

    write(outbuf, strlen(outbuf));
}

///////////////////////////////////////////////////////////////////////////////
// handleClientSetChannelGUID
//

void
tcpipClientObj::handleClientSetChannelGUID(void)
{
    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state)
        return;

    // Must be accredited to do this
    if (!m_pClientItem->bAuthenticated) {
        write(MSG_NOT_ACCREDITED, strlen(MSG_NOT_ACCREDITED));
        return;
    }

    vscp_trim(m_pClientItem->m_currentCommand);

    m_pClientItem->m_guid.getFromString(m_pClientItem->m_currentCommand);
    write(MSG_OK, strlen(MSG_OK));
}

///////////////////////////////////////////////////////////////////////////////
// handleClientGetChannelGUID
//

void
tcpipClientObj::handleClientGetChannelGUID(void)
{
    std::string strBuf;

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state)
        return;

    // Must be accredited to do this
    if (!m_pClientItem->bAuthenticated) {
        write(MSG_NOT_ACCREDITED, strlen(MSG_NOT_ACCREDITED));
        return;
    }

    m_pClientItem->m_guid.toString(strBuf);
    strBuf += std::string("\r\n");
    strBuf += std::string(MSG_OK);

    write(strBuf);
}

///////////////////////////////////////////////////////////////////////////////
// handleClientGetVersion
//

void
tcpipClientObj::handleClientGetVersion(void)
{
    char outbuf[1024];

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state)
        return;

    sprintf(outbuf,
            "%d,%d,%d,%d\r\n%s",
            VSCPD_MAJOR_VERSION,
            VSCPD_MINOR_VERSION,
            VSCPD_RELEASE_VERSION,
            VSCPD_BUILD_VERSION,
            MSG_OK);

    write(outbuf, strlen(outbuf));
}

///////////////////////////////////////////////////////////////////////////////
// handleClientSetFilter
//

void
tcpipClientObj::handleClientSetFilter(void)
{
    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state)
        return;

    // Must be accredited to do this
    if (!m_pClientItem->bAuthenticated) {
        write(MSG_NOT_ACCREDITED, strlen(MSG_NOT_ACCREDITED));
        return;
    }

    std::string str;
    vscp_trim(m_pClientItem->m_currentCommand);
    std::deque<std::string> tokens;
    vscp_split(tokens, m_pClientItem->m_currentCommand, ",");

    // Get priority
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        m_pClientItem->m_filter.filter_priority = vscp_readStringValue(str);
    } else {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // Get Class
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        m_pClientItem->m_filter.filter_class = vscp_readStringValue(str);
    } else {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // Get Type
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        m_pClientItem->m_filter.filter_type = vscp_readStringValue(str);
    } else {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // Get GUID
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        vscp_getGuidFromStringToArray(m_pClientItem->m_filter.filter_GUID, str);
    } else {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    write(MSG_OK, strlen(MSG_OK));
}

///////////////////////////////////////////////////////////////////////////////
// handleClientSetMask
//

void
tcpipClientObj::handleClientSetMask(void)
{
    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state)
        return;

    // Must be accredited to do this
    if (!m_pClientItem->bAuthenticated) {
        write(MSG_NOT_ACCREDITED, strlen(MSG_NOT_ACCREDITED));
        return;
    }

    std::string str;
    vscp_trim(m_pClientItem->m_currentCommand);
    std::deque<std::string> tokens;
    vscp_split(tokens, m_pClientItem->m_currentCommand, ",");

    // Get priority
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        m_pClientItem->m_filter.mask_priority = vscp_readStringValue(str);
    } else {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // Get Class
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        m_pClientItem->m_filter.mask_class = vscp_readStringValue(str);
    } else {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // Get Type
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        m_pClientItem->m_filter.mask_type = vscp_readStringValue(str);
    } else {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // Get GUID
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        vscp_getGuidFromStringToArray(m_pClientItem->m_filter.mask_GUID, str);
    } else {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    write(MSG_OK, strlen(MSG_OK));
}

///////////////////////////////////////////////////////////////////////////////
// handleClientUser
//

void
tcpipClientObj::handleClientUser(void)
{
    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state)
        return;

    if (m_pClientItem->bAuthenticated) {
        write(MSG_OK, strlen(MSG_OK));
        return;
    }

    m_pClientItem->m_UserName = m_pClientItem->m_currentCommand;
    vscp_trim(m_pClientItem->m_UserName);
    if (m_pClientItem->m_UserName.empty()) {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    write(MSG_USENAME_OK, strlen(MSG_USENAME_OK));
}

///////////////////////////////////////////////////////////////////////////////
// handleClientPassword
//

bool
tcpipClientObj::handleClientPassword(void)
{
    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state)
        return false;

    if (m_pClientItem->bAuthenticated) {
        write(MSG_OK, strlen(MSG_OK));
        return true;
    }

    // Must have username before password can be entered.
    if (0 == m_pClientItem->m_UserName.length()) {
        write(MSG_NEED_USERNAME, strlen(MSG_NEED_USERNAME));
        return true;
    }

    std::string strPassword = m_pClientItem->m_currentCommand;
    vscp_trim(strPassword);

    if (strPassword.empty()) {
        m_pClientItem->m_UserName = ("");
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return false;
    }

    pthread_mutex_lock(&m_pObj->m_mutex_UserList);

    m_pClientItem->m_pUserItem =
      m_pObj->m_userList.validateUser(m_pClientItem->m_UserName, strPassword);
    pthread_mutex_unlock(&m_pObj->m_mutex_UserList);

    if (NULL == m_pClientItem->m_pUserItem) {

        std::string strErr = vscp_str_format(
          ("[TCP/IP srv] User [%s][%s] not allowed to connect.\n"),
          (const char*)m_pClientItem->m_UserName.c_str(),
          (const char*)strPassword.c_str());

        syslog(LOG_ERR, "%s", strErr.c_str());
        write(MSG_PASSWORD_ERROR, strlen(MSG_PASSWORD_ERROR));
        return false;
    }

    // Get remote address
    struct sockaddr_in cli_addr;
    socklen_t clilen = 0;
    clilen           = sizeof(cli_addr);
    (void)getpeername(m_conn->client.sock,
                      (struct sockaddr*)&cli_addr,
                      &clilen);
    std::string remoteaddr = std::string(inet_ntoa(cli_addr.sin_addr));

    // Check if this user is allowed to connect from this location
    pthread_mutex_lock(&m_pObj->m_mutex_UserList);
    bool bValidHost = (1 == m_pClientItem->m_pUserItem->isAllowedToConnect(
                              cli_addr.sin_addr.s_addr));
    pthread_mutex_unlock(&m_pObj->m_mutex_UserList);

    if (!bValidHost) {
        std::string strErr =
          vscp_str_format(("[TCP/IP srv] Host [%s] not allowed to connect.\n"),
                          (const char*)remoteaddr.c_str());

        syslog(LOG_ERR, "%s", strErr.c_str());
        write(MSG_INVALID_REMOTE_ERROR, strlen(MSG_INVALID_REMOTE_ERROR));
        return false;
    }

    // Copy in the user filter
    memcpy(&m_pClientItem->m_filter,
           m_pClientItem->m_pUserItem->getUserFilter(),
           sizeof(vscpEventFilter));

    std::string strErr = vscp_str_format(
      ("[TCP/IP srv] Host [%s] User [%s] allowed to connect.\n"),
      (const char*)remoteaddr.c_str(),
      (const char*)m_pClientItem->m_UserName.c_str());

    syslog(LOG_ERR, "%s", strErr.c_str());

    m_pClientItem->bAuthenticated = true;
    write(MSG_OK, strlen(MSG_OK));

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// handleChallenge
//

void
tcpipClientObj::handleChallenge(void)
{
    std::string str;

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state)
        return;

    vscp_trim(m_pClientItem->m_currentCommand);

    memset(m_pClientItem->m_sid, 0, sizeof(m_pClientItem->m_sid));
    if (!m_pObj->generateSessionId(
          (const char*)m_pClientItem->m_currentCommand.c_str(),
          m_pClientItem->m_sid)) {
        write(MSG_FAILED_TO_GENERATE_SID, strlen(MSG_FAILED_TO_GENERATE_SID));
        return;
    }

    str = std::string("+OK - ") + std::string(m_pClientItem->m_sid) +
          std::string("\r\n");
    write(str);
}

///////////////////////////////////////////////////////////////////////////////
// handleClientRcvLoop
//

void
tcpipClientObj::handleClientRcvLoop(void)
{
    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state)
        return;

    write(MSG_RECEIVE_LOOP, strlen(MSG_RECEIVE_LOOP));
    m_bReceiveLoop = true; // Mark connection as being in receive loop

    m_pClientItem->m_readBuffer.empty();

    return;
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTest
//

void
tcpipClientObj::handleClientTest(void)
{
    write(MSG_OK, strlen(MSG_OK));
    return;
}

///////////////////////////////////////////////////////////////////////////////
// handleClientRestart
//

void
tcpipClientObj::handleClientRestart(void)
{
    write(MSG_OK, strlen(MSG_OK));

    sleep(1);
    kill(getpid(), SIGUSR2);

    return;
}

///////////////////////////////////////////////////////////////////////////////
// handleClientShutdown
//

void
tcpipClientObj::handleClientShutdown(void)
{
    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state)
        return;

    syslog(LOG_INFO, "tcp/ip client requested shutdown!!!");

    if (!m_pClientItem->bAuthenticated) {
        write(MSG_OK, strlen(MSG_OK));
    }

    write(MSG_GOODBY, strlen(MSG_GOODBY));
    sleep(1);

    kill(getpid(), SIGUSR1);
}

///////////////////////////////////////////////////////////////////////////////
// handleClientRemote
//

void
tcpipClientObj::handleClientRemote(void)
{
    return;
}

// -----------------------------------------------------------------------------
//                            I N T E R F A C E
// -----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// handleClientInterface
//
// list     List interfaces.
// unique   Acquire selected interface uniquely. Full format is INTERFACE UNIQUE
// id normal   Normal access to interfaces. Full format is INTERFACE NORMAL id
// close    Close interfaces. Full format is INTERFACE CLOSE id

void
tcpipClientObj::handleClientInterface(void)
{
    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state)
        return;

    if (m_pClientItem->CommandStartsWith(("list"))) {
        handleClientInterface_List();
    } else if (m_pClientItem->CommandStartsWith(("unique"))) {
        handleClientInterface_Unique();
    } else if (m_pClientItem->CommandStartsWith(("normal"))) {
        handleClientInterface_Normal();
    } else if (m_pClientItem->CommandStartsWith(("close"))) {
        handleClientInterface_Close();
    } else {
        handleClientInterface_List();
    }
}

///////////////////////////////////////////////////////////////////////////////
// handleClientInterface_List
//

void
tcpipClientObj::handleClientInterface_List(void)
{
    std::string strGUID;
    std::string strBuf;

    // Display Interface List
    pthread_mutex_lock(&m_pObj->m_clientList.m_mutexItemList);

    std::deque<CClientItem*>::iterator it;
    for (it = m_pObj->m_clientList.m_itemList.begin();
         it != m_pObj->m_clientList.m_itemList.end();
         ++it) {

        CClientItem* pItem = *it;

        pItem->m_guid.toString(strGUID);
        strBuf = vscp_str_format("%d,", pItem->m_clientID);
        strBuf += vscp_str_format("%d,", pItem->m_type);
        strBuf += strGUID;
        strBuf += std::string(",");
        strBuf += pItem->m_strDeviceName;
        strBuf += std::string(" | Started at ");
        strBuf += pItem->m_dtutc.getISODateTime();
        strBuf += std::string("\r\n");

        write(strBuf);
    }

    write(MSG_OK, strlen(MSG_OK));

    pthread_mutex_unlock(&m_pObj->m_clientList.m_mutexItemList);
}

///////////////////////////////////////////////////////////////////////////////
// handleClientInterface_Unique
//

void
tcpipClientObj::handleClientInterface_Unique(void)
{
    unsigned char ifGUID[16];
    memset(ifGUID, 0, 16);

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state)
        return;

    // Get GUID
    vscp_trim(m_pClientItem->m_currentCommand);
    vscp_getGuidFromStringToArray(ifGUID, m_pClientItem->m_currentCommand);

    // Add the client to the Client List
    // TODO

    write(MSG_INTERFACE_NOT_FOUND, strlen(MSG_INTERFACE_NOT_FOUND));
}

///////////////////////////////////////////////////////////////////////////////
// handleClientInterface_Normal
//

void
tcpipClientObj::handleClientInterface_Normal(void)
{
    // TODO
}

///////////////////////////////////////////////////////////////////////////////
// handleClientInterface_Close
//

void
tcpipClientObj::handleClientInterface_Close(void)
{
    // TODO
}

// -----------------------------------------------------------------------------
//                          E N D   I N T E R F A C E
// -----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// handleClientHelp
//

void
tcpipClientObj::handleClientHelp(void)
{
    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state)
        return;

    vscp_trim(m_pClientItem->m_currentCommand);

    if (0 == m_pClientItem->m_currentCommand.length()) {

        std::string str = "Help for the VSCP tcp/ip interface\r\n";
        str += "=============================================================="
               "======\r\n";
        str += "To get more information about a specific command issue 'HELP "
               "command'\r\n";
        str += "+                 - Repeat last command.\r\n";
        str += "+n                - Repeat command 'n' (0 is last).\r\n";
        str += "++                - List repeatable commands.\r\n";
        str += "NOOP              - No operation. Does nothing.\r\n";
        str += "QUIT              - Close the connection.\r\n";
        str += "USER 'username'   - Username for login. \r\n";
        str += "PASS 'password'   - Password for login.  \r\n";
        str += "CHALLENGE 'token' - Get session id.  \r\n";
        str += "SEND 'event'      - Send an event.   \r\n";
        str += "RETR 'count'      - Retrive n events from input queue.   \r\n";
        str +=
          "RCVLOOP           - Will retrieve events in an endless loop until "
          "the connection is closed by the client or QUITLOOP is sent.\r\n";
        str += "QUITLOOP          - Terminate RCVLOOP.\r\n";
        str += "CDTA/CHKDATA      - Check if there is data in the input "
               "queue.\r\n";
        str += "CLRA/CLRALL       - Clear input queue.\r\n";
        str += "STAT              - Get statistical information.\r\n";
        str += "INFO              - Get status info.\r\n";
        str += "CHID              - Get channel id.\r\n";
        str += "SGID/SETGUID      - Set GUID for channel.\r\n";
        str += "GGID/GETGUID      - Get GUID for channel.\r\n";
        str += "VERS/VERSION      - Get VSCP daemon version.\r\n";
        str += "SFLT/SETFILTER    - Set incoming event filter.\r\n";
        str += "SMSK/SETMASK      - Set incoming event mask.\r\n";
        str += "HELP [command]    - This command.\r\n";
        str += "TEST              - Do test sequence. Only used for "
               "debugging.\r\n";
        str += "SHUTDOWN          - Shutdown the daemon.\r\n";
        str += "RESTART           - Restart the daemon.\r\n";
        str += "DRIVER            - Driver manipulation.\r\n";
        str += "INTERFACE         - Interface handling. \r\n";
        str += "WCYD/WHATCANYOUDO - Check server capabilities. \r\n";
        write((const char*)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("+")) {
        std::string str = "'+' repeats the last given command.\r\n";
        write((const char*)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("noop")) {
        std::string str =
          "'NOOP' Does absolutely nothing but giving a success in return.\r\n";
        write((const char*)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith(("quit"))) {
        std::string str = "'QUIT' Quit a session with the VSCP daemon and "
                          "closes the m_connection.\r\n";
        write((const char*)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith(("user"))) {
        std::string str =
          "'USER' Used to login to the system together with PASS. Connection "
          "will be closed if bad credentials are given.\r\n";
        write((const char*)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith(("pass"))) {
        std::string str =
          "'PASS' Used to login to the system together with USER. Connection "
          "will be closed if bad credentials are given.\r\n";
        write((const char*)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith(("quit"))) {
        std::string str = "'QUIT' Quit a session with the VSCP daemon and "
                          "closes the m_connection.\r\n";
        write((const char*)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith(("send"))) {
        std::string str = "'SEND event'.\r\nThe event is given as "
                          "'head,class,type,obid,datetime,time-stamp,GUID,"
                          "data1,data2,data3....' \r\n";
        str +=
          "Normally set 'head' and 'obid' to zero. \r\nIf timestamp is set to "
          "zero it will be set by the server. \r\nIf GUID is given as '-' ";
        str += "the GUID of the interface will be used. \r\nThe GUID should "
               "be given on the form MSB-byte:MSB-byte-1:MSB-byte-2. \r\n";
        write((const char*)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith(("retr"))) {
        std::string str = "'RETR count' - Retrieve one (if no argument) or "
                          "'count' event(s). ";
        str += "Events are retrived on the form "
               "head,class,type,obid,datetime,time-stamp,GUID,data0,data1,"
               "data2,...........\r\n";
        write((const char*)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("rcvloop")) {
        std::string str = "'RCVLOOP' - Enter the receive loop and receive "
                          "events continously or until ";
        str += "terminated with 'QUITLOOP'. Events are retrived on the form "
               "head,class,type,obid,time-stamp,GUID,data0,data1,data2,......."
               "....\r\n";
        write((const char*)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith(("quitloop"))) {
        std::string str = "'QUITLOOP' - End 'RCVLOOP' event receives.\r\n";
        write((const char*)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("cdta") ||
               m_pClientItem->CommandStartsWith("chkdata")) {
        std::string str = "'CDTA' or 'CHKDATA' - Check if there is events in "
                          "the input queue.\r\n";
        write((const char*)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith(("clra")) ||
               m_pClientItem->CommandStartsWith(("clrall"))) {
        std::string str = "'CLRA' or 'CLRALL' - Clear input queue.\r\n";
        write((const char*)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith(("stat"))) {
        std::string str = "'STAT' - Get statistical information.\r\n";
        write((const char*)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("info")) {
        std::string str = "'INFO' - Get status information.\r\n";
        write((const char*)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("chid") ||
               m_pClientItem->CommandStartsWith("getchid")) {
        std::string str = "'CHID' or 'GETCHID' - Get channel id.\r\n";
        write((const char*)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("sgid") ||
               m_pClientItem->CommandStartsWith("setguid")) {
        std::string str = "'SGID' or 'SETGUID' - Set GUID for channel.\r\n";
        write((const char*)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("ggid") ||
               m_pClientItem->CommandStartsWith("getguid")) {
        std::string str = ("'GGID' or 'GETGUID' - Get GUID for channel.\r\n");
        write((const char*)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("vers") ||
               m_pClientItem->CommandStartsWith("version")) {
        std::string str =
          "'VERS' or 'VERSION' - Get version of VSCP daemon.\r\n";
        write((const char*)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("sflt") ||
               m_pClientItem->CommandStartsWith("setfilter")) {
        std::string str = "'SFLT' or 'SETFILTER' - Set filter for channel. ";
        str += "The format is 'filter-priority, filter-class, filter-type, "
               "filter-GUID' \r\n";
        str += "Example:  \r\nSETFILTER "
               "1,0x0000,0x0006,ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:"
               "00\r\n";
        write((const char*)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("smsk") ||
               m_pClientItem->CommandStartsWith("setmask")) {
        std::string str = "'SMSK' or 'SETMASK' - Set mask for channel. ";
        str += "The format is 'mask-priority, mask-class, mask-type, "
               "mask-GUID' \r\n";
        str += "Example:  \r\nSETMASK "
               "0x0f,0xffff,0x00ff,ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:"
               "00:00 \r\n";
        write((const char*)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith(("help"))) {
        std::string str = "'HELP [command]' This command. Gives help about "
                          "available commands and the usage.\r\n";
        write((const char*)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("test")) {
        std::string str = "'TEST [sequency]' Test command for debugging.\r\n";
        write((const char*)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("shutdown")) {
        std::string str = "'SHUTDOWN' Shutdown the daemon.\r\n";
        write((const char*)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("restart")) {
        std::string str = "'RESTART' Restart the daemon.\r\n";
        write((const char*)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("interface")) {
        std::string str = "'INTERFACE' Handle interfaces on the daemon.\r\n";
        str += "'INTERFACE list'.\r\n";
        str += "'INTERFACE close'.\r\n";
        write((const char*)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("wcyd") ||
               m_pClientItem->CommandStartsWith("whatcanyoudo")) {
        std::string str = "'WCYD/WHATCANYOUDO' Return the VSCP server "
                          "capabilities 64-bit array.\r\n";
        write((const char*)str.c_str(), str.length());
    } else {
        std::string str =
          vscp_str_format("The command '%s' is not available\r\n",
                          m_pClientItem->m_currentCommand.c_str());
        write((const char*)str.c_str(), str.length());
    }

    write(MSG_OK, strlen(MSG_OK));
    return;
}

///////////////////////////////////////////////////////////////////////////////
// Entry
//

void*
tcpipClientThread(void* pData)
{
    tcpipClientObj* ptcpipobj = (tcpipClientObj*)pData;
    if (NULL == ptcpipobj) {
        syslog(LOG_ERR,
               "[TCP/IP srv client thread] Error, "
               "Client thread object not initialized.");
        return NULL;
    }

    if (NULL == ptcpipobj->m_pParent) {
        syslog(LOG_ERR,
               "[TCP/IP srv client thread] Error, "
               "Control object not initialized.");
        return NULL;
    }

    syslog(LOG_DEBUG, "[TCP/IP srv client thread] Thread started.");

    ptcpipobj->m_pClientItem = new CClientItem();
    if (NULL == ptcpipobj->m_pClientItem) {
        syslog(LOG_ERR,
               "[TCP/IP srv client thread] Memory error, "
               "Cant allocate client structure.");
        return NULL;
    }

    vscpdatetime now;
    ptcpipobj->m_pClientItem->m_dtutc = now;
    ptcpipobj->m_pClientItem->m_bOpen = true;
    ptcpipobj->m_pClientItem->m_type  = CLIENT_ITEM_INTERFACE_TYPE_CLIENT_TCPIP;
    ptcpipobj->m_pClientItem->m_strDeviceName = ("Remote tcp/ip server connection @ [");
    ptcpipobj->m_pClientItem->m_strDeviceName +=
      ptcpipobj->m_pObj->m_strTcpInterfaceAddress;
    ptcpipobj->m_pClientItem->m_strDeviceName += ("]");

    // Start of activity
    ptcpipobj->m_pClientItem->m_clientActivity = time(NULL);

    // Add the client to the Client List
    pthread_mutex_lock(&ptcpipobj->m_pObj->m_clientList.m_mutexItemList);
    if (!ptcpipobj->m_pObj->addClient(ptcpipobj->m_pClientItem)) {
        // Failed to add client
        delete ptcpipobj->m_pClientItem;
        ptcpipobj->m_pClientItem = NULL;
        pthread_mutex_unlock(&ptcpipobj->m_pObj->m_clientList.m_mutexItemList);
        syslog(LOG_ERR,
               "TCP/IP server: Failed to add client. Terminating thread.");
        return NULL;
    }
    pthread_mutex_unlock(&ptcpipobj->m_pObj->m_clientList.m_mutexItemList);

    // Clear the filter (Allow everything )
    vscp_clearVSCPFilter(&ptcpipobj->m_pClientItem->m_filter);

    // Send welcome message
    std::string str = std::string(MSG_WELCOME);
    str += std::string("Version: ");
    str += std::string(VSCPD_DISPLAY_VERSION);
    str += std::string("\r\n");
    str += std::string(VSCPD_COPYRIGHT);
    str += std::string("\r\n");
    str += std::string(MSG_OK);
    ptcpipobj->write((const char*)str.c_str(), str.length());

    syslog(LOG_DEBUG, "[TCP/IP srv] Ready to serve client.");

    // Enter command loop
    char buf[8192];
    struct pollfd fd;
    while (!ptcpipobj->m_pParent->m_nStopTcpIpSrv) {

        // Check for client inactivity
        if ((time(NULL) - ptcpipobj->m_pClientItem->m_clientActivity) >
            TCPIPSRV_INACTIVITY_TIMOUT) {
            syslog(
              LOG_INFO,
              "[TCP/IP srv client thread] Client closed due to inactivity.");
            break;
        }

        // * * * Receiveloop * * *
        if (ptcpipobj->m_bReceiveLoop) {

            // Wait for data
            vscp_sem_wait(&ptcpipobj->m_pClientItem->m_semClientInputQueue, 10);

            // Send everything in the queue
            while (ptcpipobj->sendOneEventFromQueue(false))
                ;

            // Send '+OK<CR><LF>' every two seconds to indicate that the
            // link is open
            if ((time(NULL) - ptcpipobj->m_pClientItem->m_timeRcvLoop) > 2) {
                ptcpipobj->m_pClientItem->m_timeRcvLoop    = time(NULL);
                ptcpipobj->m_pClientItem->m_clientActivity = time(NULL);
                ptcpipobj->write("+OK\r\n", 5);
            }

        } else {

            // Set poll
            fd.fd      = ptcpipobj->m_conn->client.sock;
            fd.events  = POLLIN;
            fd.revents = 0;

            // Wait for data
            if (stcp_poll(&fd,
                          1,
                          500,
                          &(ptcpipobj->m_pParent->m_nStopTcpIpSrv)) < 0) {
                continue; // Nothing
            }

            // Data in?
            if (!(fd.revents & POLLIN)) {
                continue;
            }
        }

        // Read possible data from client
        //      If in receive loop we know we have delay
        //      in event waiting above.
        memset(buf, 0, sizeof(buf));
        int nRead = stcp_read(ptcpipobj->m_conn,
                              buf,
                              sizeof(buf),
                              (ptcpipobj->m_bReceiveLoop) ? 0 : 0);

        if (0 == nRead) {
            ; // Nothing more to read - Check for command and continue -> below
        } else if (nRead < 0) {

            if (STCP_ERROR_TIMEOUT == nRead) {
                ptcpipobj->m_rv = VSCP_ERROR_TIMEOUT;
            } else if (STCP_ERROR_STOPPED == nRead) {
                ptcpipobj->m_rv = VSCP_ERROR_STOPPED;
                continue;
            }
            break;
        } else if (nRead > 0) {
            ptcpipobj->m_strResponse += std::string(buf, nRead);
        }

        // Record client activity
        ptcpipobj->m_pClientItem->m_clientActivity = time(NULL);

        // get data up to "\r\n" if any
        size_t pos;
        if (ptcpipobj->m_strResponse.npos !=
            (pos = ptcpipobj->m_strResponse.find("\n"))) {

            // Get the command
            std::string strCommand =
              vscp_str_left(ptcpipobj->m_strResponse, pos + 1);

            // Save the unhandled part
            ptcpipobj->m_strResponse =
              vscp_str_right(ptcpipobj->m_strResponse,
                             ptcpipobj->m_strResponse.length() - pos - 1);

            // Remove whitespace
            vscp_trim(strCommand);

            // If nothing to do do nothing - pretty obious if you think about it
            if (0 == strCommand.length())
                continue;

            // Check for repeat command
            // +    - repear last command
            // +n   - Repeat n-th command
            // ++
            if (ptcpipobj->m_commandArray.size() && ('+' == strCommand[0])) {

                if (vscp_startsWith(strCommand, "++", &strCommand)) {
                    for (int i = ptcpipobj->m_commandArray.size() - 1; i >= 0;
                         i--) {
                        std::string str = vscp_str_format(
                          "%d - %s",
                          ptcpipobj->m_commandArray.size() - i - 1,
                          ptcpipobj->m_commandArray[i]);
                        vscp_trim(str);
                        ptcpipobj->write(str, true);
                    }
                    continue;
                }

                // Get pos
                unsigned int n = 0;
                if (strCommand.length() > 1) {
                    strCommand = strCommand.substr(strCommand.length() - 1);
                    n          = vscp_readStringValue(strCommand);
                }

                // Pos must be within range
                if (n > ptcpipobj->m_commandArray.size()) {
                    n = ptcpipobj->m_commandArray.size() - 1;
                }

                // Get the command
                strCommand =
                  ptcpipobj
                    ->m_commandArray[ptcpipobj->m_commandArray.size() - n - 1];

                // Write out the command
                ptcpipobj->write(strCommand, true);
            }

            ptcpipobj->m_commandArray.push_back(
              strCommand); // put at beginning of list
            if (ptcpipobj->m_commandArray.size() >
                VSCP_TCPIP_COMMAND_LIST_MAX) {
                ptcpipobj->m_commandArray
                  .pop_front(); // Remove last inserted item
            }

            // Execute command
            if (VSCP_TCPIP_RV_CLOSE == ptcpipobj->CommandHandler(strCommand)) {
                break;
            }
        }

    } // while

    // Remove the client from the client queue
    pthread_mutex_lock(&ptcpipobj->m_pParent->m_mutexTcpClientList);
    std::list<tcpipClientObj*>::iterator it;
    for (it = ptcpipobj->m_pParent->m_tcpip_clientList.begin();
         it != ptcpipobj->m_pParent->m_tcpip_clientList.end();
         ++it) {

        tcpipClientObj* pclient             = *it; // TODO check
        struct stcp_connection* stored_conn = pclient->m_conn;
        if (stored_conn->client.id == ptcpipobj->m_conn->client.id) {
            ptcpipobj->m_pParent->m_tcpip_clientList.erase(it);
            break;
        }
    }
    pthread_mutex_unlock(&ptcpipobj->m_pParent->m_mutexTcpClientList);

    // Close the connection
    stcp_close_connection(ptcpipobj->m_conn);
    ptcpipobj->m_conn = NULL;

    // Close the channel
    ptcpipobj->m_pClientItem->m_bOpen = false;

    // Remove the client from the Client List
    pthread_mutex_lock(&ptcpipobj->m_pObj->m_clientList.m_mutexItemList);
    ptcpipobj->m_pObj->removeClient(ptcpipobj->m_pClientItem);
    pthread_mutex_unlock(&ptcpipobj->m_pObj->m_clientList.m_mutexItemList);

    ptcpipobj->m_pClientItem = NULL;
    ptcpipobj->m_pParent     = NULL;

    // Delete the client object
    delete ptcpipobj;

    if (__VSCP_DEBUG_TCP) {
        syslog(LOG_INFO, "[TCP/IP srv client thread] Exit.");
    }

    return NULL;
}
