// tcpipsrv.cpp
//
// This file is part of the VSCP (http://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2019 Ake Hedman, Grodans Paradis AB
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
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

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
void *
tcpipListenThread(void *pData);
void *
tcpipClientThread(void *pData);

///////////////////////////////////////////////////
//                 GLOBALS
///////////////////////////////////////////////////

// ****************************************************************************
//                               Listen thread
// ****************************************************************************

///////////////////////////////////////////////////////////////////////////////
// tcpipListenThreadObj
//
// This thread listens for connection on a TCP socket and starts a new thread
// to handle client requests
//

tcpipListenThreadObj::tcpipListenThreadObj(CControlObject *pobj)
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

void *
tcpipListenThread(void *pData)
{
    int i;
    struct stcp_connection *conn;
    struct socket *psocket;
    struct stcp_secure_options opts;
    struct pollfd *pfd;
    memset(&opts, 0, sizeof(opts));

    tcpipListenThreadObj *pListenObj = (tcpipListenThreadObj *)pData;
    if (NULL == pListenObj) {
        syslog(
          LOG_CRIT,
          "TCP/IP client is missing client object data. Terinating thread.");
        return NULL;
    }

    // Fix pointer to main object
    CControlObject *pObj = pListenObj->getControlObject();

    // ------------------------------------------------------------------------

    // * * * Init. secure options * * *

    // Certificate
    if (pObj->m_tcpip_ssl_certificate.length()) {
        opts.pem = strdup((const char *)pObj->m_tcpip_ssl_certificate.c_str());
    }

    // Certificate chain
    if (pObj->m_tcpip_ssl_certificate_chain.length()) {
        opts.chain =
          strdup((const char *)pObj->m_tcpip_ssl_certificate_chain.c_str());
    }

    opts.verify_peer = pObj->m_tcpip_ssl_verify_peer;

    // CA path
    if (pObj->m_tcpip_ssl_ca_path.length()) {
        opts.ca_path = strdup((const char *)pObj->m_tcpip_ssl_ca_path.c_str());
    }

    // CA file
    if (pObj->m_tcpip_ssl_ca_file.length()) {
        opts.chain = strdup((const char *)pObj->m_tcpip_ssl_ca_file.c_str());
    }

    opts.verify_depth = pObj->m_tcpip_ssl_verify_depth;
    opts.default_verify_path = pObj->m_tcpip_ssl_default_verify_paths ? 1 : 0;
    opts.protocol_version = pObj->m_tcpip_ssl_protocol_version;

    // chiper list
    if (pObj->m_tcpip_ssl_cipher_list.length()) {
        opts.chipher_list =
          strdup((const char *)pObj->m_tcpip_ssl_cipher_list.c_str());
    }

    opts.short_trust = pObj->m_tcpip_ssl_short_trust ? 1 : 0;

    // --------------------------------------------------------------------------------------

    // Init. SSL subsystem
    /*if ( 0 == stcp_init_ssl( m_srvctx.ssl_ctx, &opts ) ) {
         syslog(, LOG_ERR, "[TCP/IP srv thread] Failed to init. ssl.\n",
                        DAEMON_LOGMSG_NORMAL );
        return NULL;
    }*/

    // Bind to selected interface
    if (0 == stcp_listening(&pListenObj->m_srvctx,
                            pListenObj->m_strListeningPort.c_str())) {
        syslog(LOG_CRIT,
               "[TCP/IP srv thread] Failed to init listening socket.");
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
                                 200,
                                 &(pListenObj->m_nStopTcpIpSrv))) > 0) {

            for (i = 0; i < pListenObj->m_srvctx.num_listening_sockets; i++) {

                // NOTE(lsm): on QNX, poll() returns POLLRDNORM after the
                // successful poll, and POLLIN is defined as
                // (POLLRDNORM | POLLRDBAND)
                // Therefore, we're checking pfd[i].revents & POLLIN, not
                // pfd[i].revents == POLLIN.
                if (pfd[i].revents & POLLIN) {

                    conn = new struct stcp_connection; // New connection
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

                        // Create the thread object
                        tcpipClientObj *pClientObj =
                          new tcpipClientObj(pListenObj);
                        if (NULL == pClientObj) {
                            syslog(LOG_ERR,
                                   "[TCP/IP srv] -- Memory problem when "
                                   "creating client thread.");
                            stcp_close_connection(conn);
                            conn == NULL;
                            continue;
                        }

                        pClientObj->m_conn    = conn;
                        pClientObj->m_pParent = pListenObj;

                        syslog(
                          LOG_DEBUG,
                          "Controlobject: Starting client tcp/ip thread...");

                        int n = 0;
                        if (pthread_create(&pClientObj->m_tcpipClientThread,
                                           NULL,
                                           tcpipClientThread,
                                           pClientObj)) {
                            syslog(LOG_ERR,
                                   "[TCP/IP srv] -- Failed to run client "
                                   "tcp/ip client thread.");
                            delete pClientObj;
                            stcp_close_connection(conn);
                            conn == NULL;
                            continue;
                        }

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

    stcp_close_all_listening_sockets(&pListenObj->m_srvctx);

    syslog(LOG_DEBUG, "[TCP/IP srv listen thread] Preparing Exit.");

    // Wait for clients to close terminate
    int loopCnt = 0;
    while (true) {

        pthread_mutex_lock(&pListenObj->m_mutexTcpClientList);
        if (!pListenObj->m_tcpip_clientList.size()) break;
        pthread_mutex_unlock(&pListenObj->m_mutexTcpClientList);

        loopCnt++;
        if (loopCnt > 5) {
            syslog(LOG_ERR,
                   "[TCP/IP srv listen thread] "
                   "Clients did not end as expected. "
                   "Terminate anyway.");
            break;
        }

        sleep(1); // Give them some time
    }

    // * * * Deallocate allocated security options * * *

    if (NULL != opts.pem) {
        delete opts.pem;
        opts.pem = NULL;
    }

    if (NULL != opts.chain) {
        delete opts.chain;
        opts.chain = NULL;
    }

    if (NULL != opts.ca_path) {
        delete opts.ca_path;
        opts.ca_path = NULL;
    }

    if (NULL != opts.ca_file) {
        delete opts.ca_file;
        opts.ca_file = NULL;
    }

    if (NULL != opts.chipher_list) {
        delete opts.chipher_list;
        opts.chipher_list = NULL;
    }

    // stcp_uninit_ssl();

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

tcpipClientObj::tcpipClientObj(tcpipListenThreadObj *pParent)
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
    m_commandArray.clear(); // TODO remove strings
}

///////////////////////////////////////////////////////////////////////////////
// write
//

bool
tcpipClientObj::write(std::string &str, bool bAddCRLF)
{
    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return false;

    if (bAddCRLF) {
        str += std::string("\r\n");
    }

    m_rv = stcp_write(m_conn, (const char *)str.c_str(), str.length());
    if (m_rv != str.length()) return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// write
//

bool
tcpipClientObj::write(const char *buf, size_t len)
{
    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return false;

    m_rv = stcp_write(m_conn, (const char *)buf, len);
    if (m_rv != len) return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// read
//

bool
tcpipClientObj::read(std::string &str)
{
    int pos;

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return false;

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
tcpipClientObj::CommandHandler(std::string &strCommand)
{
    bool repeat = false;

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
        if (checkPrivilege(2)) {
            m_pClientItem->m_timeRcvLoop = time(NULL);
            handleClientRcvLoop();
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
        handleClientUser();
    }

    //*********************************************************************
    //                            Password
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("pass"))) {

        if (!handleClientPassword()) {
            syslog(LOG_ERR, "[TCP/IP srv] Command: Password. Not authorized.");
            return VSCP_TCPIP_RV_CLOSE; // Close connection
        }

        if (m_pObj->m_debugFlags[0] & VSCP_DEBUG1_TCP) {
            syslog(LOG_DEBUG, "[TCP/IP srv] Command: Password. PASS");
        }

    }

    //*********************************************************************
    //                              Challenge
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("challenge"))) {
        handleChallenge();
    }

    // *********************************************************************
    //                                 QUIT
    // *********************************************************************

    else if (m_pClientItem->CommandStartsWith(("quit"))) {
        // long test = MG_F_CLOSE_IMMEDIATELY;
        if (m_pObj->m_debugFlags[0] & VSCP_DEBUG1_TCP) {
            syslog(LOG_INFO, "[TCP/IP srv] Command: Close.");
        }

        write(MSG_GOODBY, strlen(MSG_GOODBY));

        return VSCP_TCPIP_RV_CLOSE; // Close connection
    }

    //*********************************************************************
    //                              Shutdown
    //*********************************************************************
    else if (m_pClientItem->CommandStartsWith(("shutdown"))) {
        if (checkPrivilege(15)) {
            handleClientShutdown();
        }
    }

    //*********************************************************************
    //                             Send event
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("send"))) {
        if (checkPrivilege(4)) {
            handleClientSend();
        }
    }

    //*********************************************************************
    //                            Read event
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("retr")) ||
             m_pClientItem->CommandStartsWith(("retrieve"))) {
        if (checkPrivilege(2)) {
            handleClientReceive();
        }
    }

    //*********************************************************************
    //                            Data Available
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("cdta")) ||
             m_pClientItem->CommandStartsWith(("chkdata")) ||
             m_pClientItem->CommandStartsWith(("checkdata"))) {
        if (checkPrivilege(1)) {
            handleClientDataAvailable();
        }
    }

    //*********************************************************************
    //                          Clear input queue
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("clra")) ||
             m_pClientItem->CommandStartsWith(("clearall")) ||
             m_pClientItem->CommandStartsWith(("clrall"))) {
        if (checkPrivilege(1)) {
            handleClientClearInputQueue();
        }
    }

    //*********************************************************************
    //                           Get Statistics
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("stat"))) {
        if (checkPrivilege(1)) {
            handleClientGetStatistics();
        }
    }

    //*********************************************************************
    //                            Get Status
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("info"))) {
        if (checkPrivilege(1)) {
            handleClientGetStatus();
        }
    }

    //*********************************************************************
    //                           Get Channel ID
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("chid")) ||
             m_pClientItem->CommandStartsWith(("getchid"))) {
        if (checkPrivilege(1)) {
            handleClientGetChannelID();
        }
    }

    //*********************************************************************
    //                          Set Channel GUID
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("sgid")) ||
             m_pClientItem->CommandStartsWith(("setguid"))) {
        if (checkPrivilege(6)) {
            handleClientSetChannelGUID();
        }
    }

    //*********************************************************************
    //                          Get Channel GUID
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("ggid")) ||
             m_pClientItem->CommandStartsWith(("getguid"))) {
        if (checkPrivilege(1)) {
            handleClientGetChannelGUID();
        }
    }

    //*********************************************************************
    //                           Get Version
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("version")) ||
             m_pClientItem->CommandStartsWith(("vers"))) {
        handleClientGetVersion();
    }

    //*********************************************************************
    //                           Set Filter
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("sflt")) ||
             m_pClientItem->CommandStartsWith(("setfilter"))) {
        if (checkPrivilege(6)) {
            handleClientSetFilter();
        }
    }

    //*********************************************************************
    //                           Set Mask
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("smsk")) ||
             m_pClientItem->CommandStartsWith(("setmask"))) {
        if (checkPrivilege(6)) {
            handleClientSetMask();
        }
    }

    //*********************************************************************
    //                             Help
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("help"))) {
        handleClientHelp();
    }

    //*********************************************************************
    //                             Restart
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("restart"))) {
        if (checkPrivilege(15)) {
            handleClientRestart();
        }
    }

    //*********************************************************************
    //                             Driver
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("driver"))) {
        if (checkPrivilege(15)) {
            handleClientDriver();
        }
    }

    //*********************************************************************
    //                               DM
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("dm"))) {
        if (checkPrivilege(15)) {
            handleClientDm();
        }
    }

    //*********************************************************************
    //                             Variable
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("variable")) ||
             m_pClientItem->CommandStartsWith(("var"))) {
        if (checkPrivilege(4)) {
            handleClientVariable();
        }
    }

    //*********************************************************************
    //                               File
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("file"))) {
        if (checkPrivilege(15)) {
            handleClientFile();
        }
    }

    //*********************************************************************
    //                               UDP
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("udp"))) {
        if (checkPrivilege(15)) {
            handleClientUdp();
        }
    }

    //*********************************************************************
    //                         Client/interface
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("client")) ||
             m_pClientItem->CommandStartsWith(("interface"))) {
        if (checkPrivilege(15)) {
            handleClientInterface();
        }
    }

    //*********************************************************************
    //                               Test
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("test"))) {
        if (checkPrivilege(15)) {
            handleClientTest();
        }
    }

    //*********************************************************************
    //                             WhatCanYouDo
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("wcyd")) ||
             m_pClientItem->CommandStartsWith(("whatcanyoudo"))) {
        handleClientCapabilityRequest();
    }

    //*********************************************************************
    //                             Measurement
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("measurement"))) {
        handleClientMeasurment();
    }

    //*********************************************************************
    //                                Table
    //*********************************************************************

    else if (m_pClientItem->CommandStartsWith(("table"))) {
        handleClientTable();
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
// handleClientMeasurment
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
tcpipClientObj::handleClientMeasurment(void)
{
    std::string str;
    unsigned long l;
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
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    if (NULL == m_pClientItem) {
        write(MSG_INTERNAL_MEMORY_ERROR, strlen(MSG_INTERNAL_MEMORY_ERROR));
        return;
    }

    std::deque<std::string> tokens;
    vscp_split(tokens, m_pClientItem->m_currentCommand, ",");

    // If first character is $ user request us to send content from
    // a variable

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

    // If first character is '$' user request us to send content from
    // a variable (that must be numeric)
    if ('$' == str[0]) {

        CVariable variable;
        str = str.substr(str.length() - 1); // get variable name
        vscp_makeUpper(str);

        if (m_pObj->m_variables.find(str, variable)) {
            write(MSG_VARIABLE_NOT_DEFINED, strlen(MSG_VARIABLE_NOT_DEFINED));
            return;
        }

        // get the value
        str   = variable.getValue();
        value = std::stod(str);

    } else {
        value = std::stod(str);
    }

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
        if (0 == str.length()) str = "-";
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
        if (0 == str.length()) str = ("-");
        destguid.getFromString(str);
    }

    // Range checks
    if (VSCP_LEVEL1 == level) {
        if (unit > 3) unit = 0;
        if (sensoridx > 7) unit = 0;
        if (vscptype > 512) vscptype -= 512;
    } else { // VSCP_LEVEL2
        if (unit > 255) unit &= 0xff;
        if (sensoridx > 255) sensoridx &= 0xff;
    }

    if (1 == level) { // Level I

        if (0 == eventFormat) {

            // * * * Floating point * * *

            if (vscp_convertFloatToFloatEventData(
                  data, &sizeData, value, unit, sensoridx)) {
                if (sizeData > 8) sizeData = 8;

                vscpEvent *pEvent = new vscpEvent;
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
                    write(MSG_UNABLE_TO_SEND_EVENT,
                          strlen(MSG_UNABLE_TO_SEND_EVENT));
                    return;
                }

            } else {
                write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            }
        } else {

            // * * * String * * *

            vscpEvent *pEvent = new vscpEvent;
            if (NULL == pEvent) {
                write(MSG_INTERNAL_MEMORY_ERROR,
                      strlen(MSG_INTERNAL_MEMORY_ERROR));
                return;
            }

            pEvent->pdata = NULL;

            if (!vscp_makeStringMeasurementEvent(
                  pEvent, value, unit, sensoridx)) {
                write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            }
        }
    } else { // Level II

        if (0 == eventFormat) { // float and Level II

            // * * * Floating point * * *

            vscpEvent *pEvent = new vscpEvent;
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

            memcpy(data + 4, (uint8_t *)&value, 8); // copy in double
            uint64_t temp = VSCP_UINT64_SWAP_ON_LE(*(data + 4));
            memcpy(data + 4, (void *)&temp, 8);

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
                write(MSG_UNABLE_TO_SEND_EVENT,
                      strlen(MSG_UNABLE_TO_SEND_EVENT));
                return;
            }

        } else { // string & Level II

            // * * * String * * *

            vscpEvent *pEvent = new vscpEvent;
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
            memcpy(
              data + 4, strValue.c_str(), strValue.length()); // copy in double

            // send the event
            if (!m_pObj->sendEvent(m_pClientItem, pEvent)) {
                write(MSG_UNABLE_TO_SEND_EVENT,
                      strlen(MSG_UNABLE_TO_SEND_EVENT));
                return;
            }
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
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return false;

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
tcpipClientObj::checkPrivilege(unsigned char reqiredPrivilege)
{
    // Check objects
    if (NULL == m_pObj) {
        return false;
    }

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return false;

    // Must be authenticated
    if (!m_pClientItem->bAuthenticated) {
        write(MSG_NOT_ACCREDITED, strlen(MSG_NOT_ACCREDITED));
        return false;
    }

    if (NULL == m_pClientItem->m_pUserItem) {
        write(MSG_NOT_ACCREDITED, strlen(MSG_NOT_ACCREDITED));
        return false;
    }

    if ((m_pClientItem->m_pUserItem->getUserRights(0) & USER_PRIVILEGE_MASK) <
        reqiredPrivilege) {
        write(MSG_LOW_PRIVILEGE_ERROR, strlen(MSG_LOW_PRIVILEGE_ERROR));
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
    bool bSent     = false;
    bool bVariable = false;
    vscpEvent event;
    std::string nameVariable;

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    // Set timestamp block for event
    vscp_setEventDateTimeBlockToNow(&event);

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

        str = tokens.front();
        tokens.pop_front();
        vscp_trim(str);

        if (str[0] == '$') {
            event.head = vscp_readStringValue(str);
        } else {

            CVariable variable;
            bVariable = true; // Yes this is a variable send

            // Get the name of the variable
            nameVariable = str.substr(str.length() - 1);
            vscp_makeUpper(nameVariable);

            if (m_pObj->m_variables.find(nameVariable, variable)) {
                write(MSG_VARIABLE_NOT_DEFINED,
                      strlen(MSG_VARIABLE_NOT_DEFINED));
                return;
            }

            // Must be event type
            if (VSCP_DAEMON_VARIABLE_CODE_EVENT != variable.getType()) {
                write(MSG_VARIABLE_MUST_BE_EVENT_TYPE,
                      strlen(MSG_VARIABLE_MUST_BE_EVENT_TYPE));
                return;
            }

            // Get the event
            variable.getValue(&event);
        }
    } else {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    if (!bVariable) { // Not a variable

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
        if (512 < tokens.size()) {
            write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            return;
        }

        event.sizeData = tokens.size();

        if (event.sizeData > 0) {

            unsigned int index = 0;

            event.pdata = new uint8_t[event.sizeData];

            if (NULL == event.pdata) {
                write(MSG_INTERNAL_MEMORY_ERROR,
                      strlen(MSG_INTERNAL_MEMORY_ERROR));
                return;
            }

            while (!tokens.empty() && (event.sizeData > index)) {
                str = tokens.front();
                tokens.pop_front();
                event.pdata[index++] = vscp_readStringValue(str);
            }

            if (!tokens.empty()) {
                write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));

                delete[] event.pdata;
                event.pdata = NULL;
                return;
            }
        } else {
            // No data
            event.pdata = NULL;
        }

    } // not variable send

    // Check if this user is allowed to send this event
    if (!m_pClientItem->m_pUserItem->isUserAllowedToSendEvent(
          event.vscp_class, event.vscp_type)) {
        std::string strErr = vscp_str_format(
          ("[TCP/IP srv] User [%s] not allowed to send event class=%d "
           "type=%d.\n"),
          (const char *)m_pClientItem->m_pUserItem->getUserName().c_str(),
          event.vscp_class,
          event.vscp_type);

        syslog(LOG_ERR, "%s", strErr.c_str());

        write(MSG_MOT_ALLOWED_TO_SEND_EVENT,
              strlen(MSG_MOT_ALLOWED_TO_SEND_EVENT));

        if (NULL != event.pdata) {
            delete[] event.pdata;
            event.pdata = NULL;
        }

        return;
    }

    // send event
    if (!m_pObj->sendEvent(m_pClientItem, &event)) {
        write(MSG_BUFFER_FULL, strlen(MSG_BUFFER_FULL));
        return;
    }

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
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    // Must be accredited to do this
    if (!m_pClientItem->bAuthenticated) {
        write(MSG_NOT_ACCREDITED, strlen(MSG_NOT_ACCREDITED));
        return;
    }

    std::string str;
    cnt = vscp_readStringValue(m_pClientItem->m_currentCommand);

    if (!cnt) cnt = 1; // No arg is "read one"

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
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return false;

    if (m_pClientItem->m_clientInputQueue.size()) {

        vscpEvent *pqueueEvent;
        pthread_mutex_lock(&m_pClientItem->m_mutexClientInputQueue);
        {
            pqueueEvent = m_pClientItem->m_clientInputQueue.front();
            m_pClientItem->m_clientInputQueue.pop_front();
        }
        pthread_mutex_unlock(&m_pClientItem->m_mutexClientInputQueue);

        vscp_writeVscpEventToString(pqueueEvent, strOut);
        strOut += ("\r\n");
        write(strOut.c_str(), strlen(strOut.c_str()));

        vscp_deleteVSCPevent(pqueueEvent);

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
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    // Must be accredited to do this
    if (!m_pClientItem->bAuthenticated) {
        write(MSG_NOT_ACCREDITED, strlen(MSG_NOT_ACCREDITED));
        return;
    }

    sprintf(
      outbuf, "%zd\r\n%s", m_pClientItem->m_clientInputQueue.size(), MSG_OK);
    write(outbuf, strlen(outbuf));
}

///////////////////////////////////////////////////////////////////////////////
// handleClientClearInputQueue
//

void
tcpipClientObj::handleClientClearInputQueue(void)
{
    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    // Must be accredited to do this
    if (!m_pClientItem->bAuthenticated) {
        write(MSG_NOT_ACCREDITED, strlen(MSG_NOT_ACCREDITED));
        return;
    }

    pthread_mutex_lock(&m_pClientItem->m_mutexClientInputQueue);
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
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

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
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

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
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    // Must be accredited to do this
    if (!m_pClientItem->bAuthenticated) {
        write(MSG_NOT_ACCREDITED, strlen(MSG_NOT_ACCREDITED));
        return;
    }

    sprintf(
      outbuf, "%lu\r\n%s", (unsigned long)m_pClientItem->m_clientID, MSG_OK);

    write(outbuf, strlen(outbuf));
}

///////////////////////////////////////////////////////////////////////////////
// handleClientSetChannelGUID
//

void
tcpipClientObj::handleClientSetChannelGUID(void)
{
    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

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
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

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
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

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
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

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
        m_pClientItem->m_filterVSCP.filter_priority = vscp_readStringValue(str);
    } else {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // Get Class
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        m_pClientItem->m_filterVSCP.filter_class = vscp_readStringValue(str);
    } else {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // Get Type
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        m_pClientItem->m_filterVSCP.filter_type = vscp_readStringValue(str);
    } else {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // Get GUID
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        vscp_getGuidFromStringToArray(m_pClientItem->m_filterVSCP.filter_GUID,
                                      str);
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
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

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
        m_pClientItem->m_filterVSCP.mask_priority = vscp_readStringValue(str);
    } else {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // Get Class
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        m_pClientItem->m_filterVSCP.mask_class = vscp_readStringValue(str);
    } else {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // Get Type
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        m_pClientItem->m_filterVSCP.mask_type = vscp_readStringValue(str);
    } else {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // Get GUID
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        vscp_getGuidFromStringToArray(m_pClientItem->m_filterVSCP.mask_GUID,
                                      str);
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
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

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
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return false;

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

    pthread_mutex_lock(&m_pObj->m_mutexUserList);

    m_pClientItem->m_pUserItem =
      m_pObj->m_userList.validateUser(m_pClientItem->m_UserName, strPassword);
    pthread_mutex_unlock(&m_pObj->m_mutexUserList);

    if (NULL == m_pClientItem->m_pUserItem) {

        std::string strErr = vscp_str_format(
          ("[TCP/IP srv] User [%s][%s] not allowed to connect.\n"),
          (const char *)m_pClientItem->m_UserName.c_str(),
          (const char *)strPassword.c_str());

        syslog(LOG_ERR, "%s", strErr.c_str());
        write(MSG_PASSWORD_ERROR, strlen(MSG_PASSWORD_ERROR));
        return false;
    }

    // Get remote address
    struct sockaddr_in cli_addr;
    socklen_t clilen = 0;
    clilen           = sizeof(cli_addr);
    (void)getpeername(
      m_conn->client.sock, (struct sockaddr *)&cli_addr, &clilen);
    std::string remoteaddr = std::string(inet_ntoa(cli_addr.sin_addr));

    // Check if this user is allowed to connect from this location
    pthread_mutex_lock(&m_pObj->m_mutexUserList);
    bool bValidHost = (1 == m_pClientItem->m_pUserItem->isAllowedToConnect(
                              cli_addr.sin_addr.s_addr));
    pthread_mutex_unlock(&m_pObj->m_mutexUserList);

    if (!bValidHost) {
        std::string strErr =
          vscp_str_format(("[TCP/IP srv] Host [%s] not allowed to connect.\n"),
                          (const char *)remoteaddr.c_str());

        syslog(LOG_ERR, "%s", strErr.c_str());
        write(MSG_INVALID_REMOTE_ERROR, strlen(MSG_INVALID_REMOTE_ERROR));
        return false;
    }

    // Copy in the user filter
    memcpy(&m_pClientItem->m_filterVSCP,
           m_pClientItem->m_pUserItem->getFilter(),
           sizeof(vscpEventFilter));

    std::string strErr = vscp_str_format(
      ("[TCP/IP srv] Host [%s] User [%s] allowed to connect.\n"),
      (const char *)remoteaddr.c_str(),
      (const char *)m_pClientItem->m_UserName.c_str());

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
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    memset(m_pClientItem->m_sid, 0, sizeof(m_pClientItem->m_sid));
    if (!m_pObj->generateSessionId(
          (const char *)m_pClientItem->m_currentCommand.c_str(),
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
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

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
    return;
}

///////////////////////////////////////////////////////////////////////////////
// handleClientShutdown
//

void
tcpipClientObj::handleClientShutdown(void)
{
    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    if (!m_pClientItem->bAuthenticated) {
        write(MSG_OK, strlen(MSG_OK));
    }

    write(MSG_GOODBY, strlen(MSG_GOODBY));
    stcp_close_connection(m_conn);
    m_conn == NULL;
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
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    if (m_pClientItem->CommandStartsWith(("list"))) {
        handleClientInterface_List();
    } else if (m_pClientItem->CommandStartsWith(("unique"))) {
        handleClientInterface_Unique();
    } else if (m_pClientItem->CommandStartsWith(("normal"))) {
        handleClientInterface_Normal();
    } else if (m_pClientItem->CommandStartsWith(("close"))) {
        handleClientInterface_Close();
    }
    else {
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
    pthread_mutex_lock(&m_pObj->m_clientMutex);

    std::list<CClientItem *>::iterator it;
    for (it = m_pObj->m_clientList.m_itemList.begin();
         it != m_pObj->m_clientList.m_itemList.end();
         ++it) {

        CClientItem *pItem = *it;

        pItem->m_guid.toString(strGUID);
        strBuf = vscp_str_format("%d,", pItem->m_clientID);
        strBuf += vscp_str_format("%d,", pItem->m_type);
        strBuf += strGUID;
        strBuf += std::string(",");
        strBuf += pItem->m_strDeviceName;
        strBuf += std::string("\r\n");

        write(strBuf);
    }

    write(MSG_OK, strlen(MSG_OK));

    pthread_mutex_unlock(&m_pObj->m_clientMutex);
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
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

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

// -----------------------------------------------------------------------------
//                                  U D P
// -----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// handleClientUdp
//

void
tcpipClientObj::handleClientUdp(void)
{
    // TODO
}

// -----------------------------------------------------------------------------
//                            E N D   U D P
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                                 F I L E
// -----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// handleClientFile
//

void
tcpipClientObj::handleClientFile(void)
{
    // TODO
}

// -----------------------------------------------------------------------------
//                            E N D   F I L E
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                         S T A R T   T A B L E
// -----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// handleClientTable
//

void
tcpipClientObj::handleClientTable(void)
{
    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    // List tables or table definition
    if (m_pClientItem->CommandStartsWith(("list"))) {
        handleClientTable_List();
    }
    // Get rawtable content
    else if (m_pClientItem->CommandStartsWith(("getraw"))) {
        handleClientTable_GetRaw();
    }
    // Get table content
    else if (m_pClientItem->CommandStartsWith(("get"))) {
        handleClientTable_Get();
    }
    // Delete table data
    else if (m_pClientItem->CommandStartsWith(("clear"))) {
        handleClientTable_Clear();
    }
    // New table (create)
    else if (m_pClientItem->CommandStartsWith(("create"))) {
        handleClientTable_Create();
    }
    // Delete table
    else if (m_pClientItem->CommandStartsWith(("delete")) ||
             m_pClientItem->CommandStartsWith(("del"))) {
        handleClientTable_Delete();
    }
    // Log data use SQL
    else if (m_pClientItem->CommandStartsWith(("logsql"))) {
        handleClientTable_LogSQL();
    }
    // Log data
    else if (m_pClientItem->CommandStartsWith(("log"))) {
        handleClientTable_Log();
    }
    // Get number of records
    else if (m_pClientItem->CommandStartsWith(("records"))) {
        handleClientTable_NumberOfRecords();
    }
    // Get first date
    else if (m_pClientItem->CommandStartsWith(("firstdate"))) {
        handleClientTable_FirstDate();
    }
    // Get last date
    else if (m_pClientItem->CommandStartsWith(("lastdate"))) {
        handleClientTable_FirstDate();
    }
    // Get sum
    else if (m_pClientItem->CommandStartsWith(("sum"))) {
        handleClientTable_Sum();
    }
    // Get min
    else if (m_pClientItem->CommandStartsWith(("min"))) {
        handleClientTable_Min();
    }
    // Get max
    else if (m_pClientItem->CommandStartsWith(("max"))) {
        handleClientTable_Min();
    }
    // Get average
    else if (m_pClientItem->CommandStartsWith(("average"))) {
        handleClientTable_Average();
    }
    // Get median
    else if (m_pClientItem->CommandStartsWith(("median"))) {
        handleClientTable_Median();
    }
    // Get stddev
    else if (m_pClientItem->CommandStartsWith(("stddev"))) {
        handleClientTable_StdDev();
    }
    // Get variance
    else if (m_pClientItem->CommandStartsWith(("variance"))) {
        handleClientTable_Variance();
    }
    // Get mode
    else if (m_pClientItem->CommandStartsWith(("mode"))) {
        handleClientTable_Mode();
    }
    // Get lowerq
    else if (m_pClientItem->CommandStartsWith(("lowerq"))) {
        handleClientTable_LowerQ();
    }
    // Get upperq
    else if (m_pClientItem->CommandStartsWith(("upperq"))) {
        handleClientTable_UpperQ();
    }

    // unrecognised
    else {
        write(MSG_UNKNOWN_COMMAND, strlen(MSG_UNKNOWN_COMMAND));
    }
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_Create
//
// name, bEnable, bInMemory, type, size, owner, rights, title, xname, yname,
// note,
//  sqlcreate, sqlinsert, sqldelete, description,
//  vscpclass, vscptype,
// Optional parameter follow
//  sensorindex, unit, zone, subzone
//
// Table name must be unique
//

void
tcpipClientObj::handleClientTable_Create(void)
{
    std::string str;
    std::string strName;
    bool bEnable       = true;
    bool bInMemory     = false;
    vscpTableType type = VSCP_TABLE_DYNAMIC;
    uint32_t size      = 0;
    std::string strOwner;
    uint16_t rights;
    std::string strTitle;
    std::string strXname;
    std::string strYname;
    std::string strNote;
    std::string strSqlCreate;
    std::string strSqlInsert;
    std::string strSqlDelete;
    std::string strDescription;
    uint8_t vscpclass;
    uint8_t vscptype;
    uint8_t sensorindex = 0;
    uint8_t unit        = 0;
    uint8_t zone        = 255;
    uint8_t subzone     = 255;

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    if (!m_pObj->m_userTableObjects.createTableFromXML(
          m_pClientItem->m_currentCommand)) {
        write(MSG_FAILED_TO_CREATE_TABLE, strlen(MSG_FAILED_TO_CREATE_TABLE));
        return;
    }

    // All went well
    write(MSG_OK, strlen(MSG_OK));
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_Delete
//

void
tcpipClientObj::handleClientTable_Delete(void)
{
    std::string strTable;
    bool bRemoveFile = false;

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    std::deque<std::string> tokens;
    vscp_split(tokens, m_pClientItem->m_currentCommand, " ");

    // Get table name
    if (!tokens.empty()) {
        strTable = tokens.front();
        tokens.pop_front();
        vscp_trim(strTable);
    } else {
        // Problems: A table name must be given
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // Should table db be removed
    if (!tokens.empty()) {
        std::string str = tokens.front();
        tokens.pop_front();
        if (0 == vscp_strcasecmp(str.c_str(), "true")) {
            bRemoveFile = true;
        }
    }

    // Remove the table from the internal system
    if (!m_pObj->m_userTableObjects.removeTable(strTable), bRemoveFile) {
        // Failed
        write(MSG_FAILED_TO_REMOVE_TABLE, strlen(MSG_FAILED_TO_REMOVE_TABLE));
    }

    write(MSG_OK, strlen(MSG_OK));
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_List
//

void
tcpipClientObj::handleClientTable_List(void)
{
    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    if (0 == m_pClientItem->m_currentCommand.length()) {

        // list without argument - list all defined tables
        std::deque<std::string> arrayTblNames;
        pthread_mutex_lock(&m_pObj->m_mutexUserTables);
        if (m_pObj->m_userTableObjects.getTableNames(arrayTblNames)) {

            // OK

            std::string str =
              vscp_str_format("%zu rows \r\n", arrayTblNames.size());
            write((const char *)str.c_str(), strlen((const char *)str.c_str()));

            for (int i = 0; i < arrayTblNames.size(); i++) {

                CVSCPTable *pTable =
                  m_pObj->m_userTableObjects.getTable(arrayTblNames[i]);

                str = arrayTblNames[i];
                str += (";");
                if (NULL != pTable) {

                    switch (pTable->getType()) {
                        case VSCP_TABLE_DYNAMIC:
                            str += ("dynamic");
                            break;

                        case VSCP_TABLE_STATIC:
                            str += ("static");
                            break;

                        case VSCP_TABLE_MAX:
                            str += ("max");
                            break;

                        default:
                            str += ("Error: Invalid type");
                            break;
                    }
                }
                str += (";");
                if (NULL != pTable) {
                    str += pTable->getDescription();
                }
                str += ("\r\n");
                write((const char *)str.c_str(),
                      strlen((const char *)str.c_str()));
            }

            write(MSG_OK, strlen(MSG_OK));

        } else {

            // Failed
            write(MSG_FAILED_GET_TABLE_NAMES,
                  strlen(MSG_FAILED_GET_TABLE_NAMES));
        }
        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
    } else {

        // list with argument - list info about table given as argument
        // if table name is followed by another argument 'xml' the table should
        // be in XML format.

        bool bXmlFormat = false;
        std::deque<std::string> tokens;
        vscp_split(tokens, m_pClientItem->m_currentCommand, " ");

        if (tokens.empty()) {
            write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            return;
        }

        std::string tblName = tokens.front();
        tokens.pop_front();

        if (!tokens.empty()) {
            std::string str = tokens.front();
            tokens.pop_front();
            if (0 == strcasecmp(str.c_str(), "XML")) {
                bXmlFormat = true; // Output XML format
            }
        }

        pthread_mutex_lock(&m_pObj->m_mutexUserTables);

        CVSCPTable *pTable = m_pObj->m_userTableObjects.getTable(tblName);
        if (NULL == pTable) {
            // Failed
            write(MSG_FAILED_UNKNOWN_TABLE, strlen(MSG_FAILED_UNKNOWN_TABLE));
        } else {

            std::string str;

            if (bXmlFormat) {
                str = "<table \r\n";
            }

            str += "name=" + pTable->getTableName();
            str += "\r\n";

            str += "enabled=";
            str += pTable->isEnabled() ? "true" : "false";
            str += "\r\n";

            str += "type=";
            switch (pTable->getType()) {
                case VSCP_TABLE_DYNAMIC:
                    str += "dynamic";
                    break;

                case VSCP_TABLE_STATIC:
                    str += "static";
                    break;

                case VSCP_TABLE_MAX:
                    str += "max";
                    break;

                default:
                    str += "Error: Invalid type";
                    break;
            }
            str += "\r\n";

            str += "bmemory=";
            str += pTable->isInMemory() ? "true" : "false";
            str += "\r\n";

            str += "size=";
            str += vscp_str_format("%lu ", (unsigned long)pTable->getSize());
            str += "\r\n";

            str += "owner=";
            CUserItem *pUserItem = pTable->getUserItem();
            if (NULL == pUserItem) {
                str += "ERROR";
            } else {
                str += pUserItem->getUserName();
            }
            str += "\r\n";

            str += "permission=";
            str += vscp_str_format("0x%0X ", (int)pTable->getRights());
            str += "\r\n";

            str += "description=";
            str += pTable->getDescription();
            str += "\r\n";

            str += "xname=";
            str += pTable->getLabelX();
            str += "\r\n";

            str += "yname=";
            str += pTable->getLabelY();
            str += ("\r\n");

            str += "title=";
            str += pTable->getTitle();
            str += "\r\n";

            str += "note=";
            str += pTable->getNote();
            str += "\r\n";

            str += "vscp-class=";
            str += vscp_str_format("%d", (int)pTable->getVSCPClass());
            str += "\r\n";

            str += "vscp-type=";
            str += vscp_str_format("%d", (int)pTable->getVSCPType());
            str += "\r\n";

            str += "vscp-sensor-index=";
            str += vscp_str_format("%d", (int)pTable->getVSCPSensorIndex());
            str += ("\r\n");

            str += "vscp-unit=";
            str += vscp_str_format("%d", (int)pTable->getVSCPUnit());
            str += "\r\n";

            str += "vscp-zone=";
            str += vscp_str_format("%d", (int)pTable->getVSCPZone());
            str += "\r\n";

            str += "vscp-subzone=";
            str += vscp_str_format("%d", (int)pTable->getVSCPSubZone());
            str += "\r\n";

            str += "vscp-create=";
            str += pTable->getSQLCreate();
            str += "\r\n";

            str += "vscp-insert=";
            str += pTable->getSQLInsert();
            str += "\r\n";

            str += "vscp-delete=";
            str += pTable->getSQLDelete();
            str += "\r\n";

            if (bXmlFormat) {
                str += "/> \r\n";
                str = vscp_base64_std_encode(str);
                str += "\r\n";
            }

            // Send response
            write((const char *)str.c_str(), strlen((const char *)str.c_str()));

            // Tell user we are content with things.
            write(MSG_OK, strlen(MSG_OK));
        }
        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
    }
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_Get
//
// get 'table-name' start end ["full"]
//

void
tcpipClientObj::handleClientTable_Get(void)
{
    std::string strTable;
    vscpdatetime start;
    vscpdatetime end;
    bool bAll = false;

    // Initialize date range to 'all'
    start.set(("0000-01-01T00:00:00")); // The first date
    end.set(("9999-12-31T23:59:59"));   // The last date

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    std::deque<std::string> tokens;
    vscp_split(tokens, m_pClientItem->m_currentCommand, " ");

    // Get table name
    if (!tokens.empty()) {
        strTable = tokens.front();
        tokens.pop_front();
        vscp_trim(strTable);
    } else {
        // Problems: A table name must be given
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // The start and end parameters must be present

    // start
    if (!tokens.empty()) {

        start.set(tokens.front());
        tokens.pop_front();

        if (!start.isValid()) {
            write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            return;
        }

    } else {
        // Problems: A start date must be given
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // end
    if (!tokens.empty()) {

        end.set(tokens.front());
        tokens.pop_front();

        if (!end.isValid()) {
            write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            return;
        }

    } else {
        // Problems: An end date must be given
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // "full" flag
    if (!tokens.empty()) {
        std::string str = tokens.front();
        tokens.pop_front();
        if (vscp_startsWith(vscp_upper(str), "FULL")) {
            bAll = true;
        }
    }

    pthread_mutex_lock(&m_pObj->m_mutexUserTables);

    CVSCPTable *pTable = m_pObj->m_userTableObjects.getTable(strTable);
    if (NULL == pTable) {
        // Failed
        write(MSG_FAILED_UNKNOWN_TABLE, strlen(MSG_FAILED_UNKNOWN_TABLE));

        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    sqlite3_stmt *ppStmt;
    if (!pTable->prepareRangeOfData(start, end, &ppStmt, bAll)) {
        // Failed
        write(MSG_FAILED_TO_PREPARE_TABLE, strlen(MSG_FAILED_TO_PREPARE_TABLE));

        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    std::string str;
    std::deque<std::string> strArray;
    while (pTable->getRowRangeOfData(ppStmt, str)) {
        strArray.push_back(str);
    }

    str = vscp_str_format(("%d rows.\r\n"), (int)strArray.size());
    write((const char *)str.c_str(), strlen((const char *)str.c_str()));

    if (strArray.size()) {
        for (int i = 0; i < strArray.size(); i++) {
            str = strArray[i];
            str += ("\r\n");
            write((const char *)str.c_str(), strlen((const char *)str.c_str()));
        }
    }

    if (!pTable->finalizeRangeOfData(ppStmt)) {
        // Failed
        write(MSG_FAILED_TO_FINALIZE_TABLE,
              strlen(MSG_FAILED_TO_FINALIZE_TABLE));

        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    pthread_mutex_unlock(&m_pObj->m_mutexUserTables);

    // Everything is OK
    write(MSG_OK, strlen(MSG_OK));
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_GetRaw
//
// get 'table-name' start end ["full"]
//

void
tcpipClientObj::handleClientTable_GetRaw(void)
{
    std::string strTable;
    vscpdatetime start;
    vscpdatetime end;

    // Initialize date range to 'all'
    start.set(("0000-01-01T00:00:00")); // The first date
    end.set(("9999-12-31T23:59:59"));   // The last date

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    std::deque<std::string> tokens;
    vscp_split(tokens, m_pClientItem->m_currentCommand, " ");

    // Get table name
    if (!tokens.empty()) {
        strTable = tokens.front();
        tokens.pop_front();
        vscp_trim(strTable);
    } else {
        // Problems: A table name must be given
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // The start and end parameters must be present

    // start
    if (!tokens.empty()) {

        start.set(tokens.front());
        tokens.pop_front();

        if (!start.isValid()) {
            write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            return;
        }

    } else {
        // Problems: A start date must be given
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // end
    if (!tokens.empty()) {

        end.set(tokens.front());
        tokens.pop_front();

        if (!end.isValid()) {
            write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            return;
        }

    } else {
        // Problems: An end date must be given
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    pthread_mutex_lock(&m_pObj->m_mutexUserTables);

    CVSCPTable *pTable = m_pObj->m_userTableObjects.getTable(strTable);
    if (NULL == pTable) {
        // Failed
        write(MSG_FAILED_UNKNOWN_TABLE, strlen(MSG_FAILED_UNKNOWN_TABLE));

        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    sqlite3_stmt *ppStmt;
    if (!pTable->prepareRangeOfData(start, end, &ppStmt, true)) {
        // Failed
        write(MSG_FAILED_TO_PREPARE_TABLE, strlen(MSG_FAILED_TO_PREPARE_TABLE));

        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    std::string str;
    std::deque<std::string> strArray;
    while (pTable->getRowRangeOfDataRaw(ppStmt, str)) {
        strArray.push_back(str);
    }

    str = vscp_str_format(("%d rows.\r\n"), (int)strArray.size());
    write((const char *)str.c_str(), strlen((const char *)str.c_str()));

    if (strArray.size()) {
        for (int i = 0; i < strArray.size(); i++) {
            str = strArray[i];
            str += ("\r\n");
            write((const char *)str.c_str(), strlen((const char *)str.c_str()));
        }
    }

    if (!pTable->finalizeRangeOfData(ppStmt)) {
        // Failed
        write(MSG_FAILED_TO_FINALIZE_TABLE,
              strlen(MSG_FAILED_TO_FINALIZE_TABLE));

        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    pthread_mutex_unlock(&m_pObj->m_mutexUserTables);

    // Everything is OK
    write(MSG_OK, strlen(MSG_OK));
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_Clear
//
// clear 'table-name' [to,from]
//

void
tcpipClientObj::handleClientTable_Clear(void)
{
    std::string strTable;
    vscpdatetime start;
    vscpdatetime end;
    bool bClearAll = false;

    // Initialize date range to 'all'
    start.set(("0000-01-01T00:00:00")); // The first date
    end.set(("9999-12-31T23:59:59"));   // The last date

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    std::deque<std::string> tokens;
    vscp_split(tokens, m_pClientItem->m_currentCommand, " ");

    // Get table name
    if (!tokens.empty()) {
        strTable = tokens.front();
        tokens.pop_front();
        vscp_trim(strTable);
    } else {
        // Problems: A table name must be given
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // get range if given
    if (tokens.size() >= 2) {

        if (!tokens.empty() && !start.set(tokens.front())) {
            write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            return;
        }
        tokens.pop_front();

        if (!tokens.empty() && !end.set(tokens.front())) {
            write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            return;
        }
        tokens.pop_front();

    } else {
        bClearAll = true;
    }

    pthread_mutex_lock(&m_pObj->m_mutexUserTables);

    CVSCPTable *pTable = m_pObj->m_userTableObjects.getTable(strTable);

    if (NULL == pTable) {
        // Failed
        write(MSG_FAILED_UNKNOWN_TABLE, strlen(MSG_FAILED_UNKNOWN_TABLE));

        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    if (bClearAll) {
        if (!pTable->clearTable()) {
            write(MSG_FAILED_TO_CLEAR_TABLE, strlen(MSG_FAILED_TO_CLEAR_TABLE));
            pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
            return;
        }
    } else {
        if (!pTable->clearTableRange(start, end)) {
            write(MSG_FAILED_TO_CLEAR_TABLE, strlen(MSG_FAILED_TO_CLEAR_TABLE));
            pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
            return;
        }
    }

    pthread_mutex_unlock(&m_pObj->m_mutexUserTables);

    write(MSG_OK, strlen(MSG_OK));

    return;
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_Log
//
// log table-name value [datetime]
//

void
tcpipClientObj::handleClientTable_Log(void)
{
    std::string strTable;
    double value;
    vscpdatetime dt;

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    std::deque<std::string> tokens;
    vscp_split(tokens, m_pClientItem->m_currentCommand, " ");

    // Get table name
    if (!tokens.empty()) {
        strTable = tokens.front();
        tokens.pop_front();
        vscp_trim(strTable);
    } else {
        // Problems: A table name must be given
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // Get the value
    if (!tokens.empty()) {
        std::string str = tokens.front();
        tokens.pop_front();
        value = std::stod(str);
    } else {
        // Problems: A value must be given
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // Get the datetime if its there
    if (!tokens.empty()) {

        uint32_t ms = 0;

        std::string str = tokens.front();
        tokens.pop_front();
        vscp_trim(str);

        if (!dt.set(str)) {
            // Problems: The value is not in a valid format
            write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            return;
        }

        // Get possible millisecond part
        /*  TODO  Remove????
        str = str.AfterFirst('.');
        vscp_trim( str );
        if ( str.length() ) {
            ms = vscp_readStringValue( str );
        }

        dt.SetMillisecond( ms );
        */
    } else {
        // Set to now
        dt = vscpdatetime::UTCNow();
    }

    pthread_mutex_lock(&m_pObj->m_mutexUserTables);

    CVSCPTable *pTable = m_pObj->m_userTableObjects.getTable(strTable);

    if (NULL == pTable) {
        // Failed
        write(MSG_FAILED_UNKNOWN_TABLE, strlen(MSG_FAILED_UNKNOWN_TABLE));

        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    // Log data
    if (!pTable->logData(dt, value)) {
        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        write(MSG_FAILED_TO_WRITE_TABLE, strlen(MSG_FAILED_TO_WRITE_TABLE));
        return;
    }

    pthread_mutex_unlock(&m_pObj->m_mutexUserTables);

    write(MSG_OK, strlen(MSG_OK));
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_LogSQL
//

void
tcpipClientObj::handleClientTable_LogSQL(void)
{
    std::string strTable, strSQL;

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    std::deque<std::string> tokens;
    vscp_split(tokens, m_pClientItem->m_currentCommand, " ");

    // Get table name
    if (!tokens.empty()) {
        strTable = tokens.front();
        tokens.pop_front();
        vscp_trim(strTable);
    } else {
        // Problems: A table name must be given
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // Get SQL expression
    if (!tokens.empty()) {
        strSQL = tokens.front();
        tokens.pop_front();
        vscp_trim(strSQL);
    } else {
        // Problems: A SQL expression must be given
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    pthread_mutex_lock(&m_pObj->m_mutexUserTables);

    CVSCPTable *pTable = m_pObj->m_userTableObjects.getTable(strTable);

    if (NULL == pTable) {
        // Failed
        write(MSG_FAILED_UNKNOWN_TABLE, strlen(MSG_FAILED_UNKNOWN_TABLE));

        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    // Log data
    if (!pTable->logData(strSQL)) {
        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        write(MSG_FAILED_TO_WRITE_TABLE, strlen(MSG_FAILED_TO_WRITE_TABLE));
        return;
    }

    pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_NumberOfRecords
//
// records 'table-name' [to,from]
//

void
tcpipClientObj::handleClientTable_NumberOfRecords(void)
{
    std::string strTable;
    vscpdatetime start;
    vscpdatetime end;

    // Initialize date range to 'all'
    start.set(("0000-01-01T00:00:00")); // The first date
    end.set(("9999-12-31T23:59:59"));   // The last date

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    std::deque<std::string> tokens;
    vscp_split(tokens, m_pClientItem->m_currentCommand, " ");

    // Get table name
    if (!tokens.empty()) {
        strTable = tokens.front();
        tokens.pop_front();
        vscp_trim(strTable);
    } else {
        // Problems: A table name must be given
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // get range if given
    if (tokens.size() >= 2) {

        if (!tokens.empty() && !start.set(tokens.front())) {
            write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            return;
        }
        tokens.pop_front();

        if (!tokens.empty() && !end.set(tokens.front())) {
            write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            return;
        }
        tokens.pop_front();
    }

    pthread_mutex_lock(&m_pObj->m_mutexUserTables);

    CVSCPTable *pTable = m_pObj->m_userTableObjects.getTable(strTable);

    if (NULL == pTable) {
        // Failed
        write(MSG_FAILED_UNKNOWN_TABLE, strlen(MSG_FAILED_UNKNOWN_TABLE));

        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    double count;
    if (!pTable->getNumberOfRecordsForRange(start, end, &count)) {
        write(MSG_FAILED_TO_CLEAR_TABLE, strlen(MSG_FAILED_TO_CLEAR_TABLE));
        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    pthread_mutex_unlock(&m_pObj->m_mutexUserTables);

    std::string strReply = vscp_str_format(("%f\r\n"), count);
    write((const char *)strReply.c_str(),
          strlen((const char *)strReply.c_str()));

    write(MSG_OK, strlen(MSG_OK));

    return;
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_FirstDate
//
// firstdate 'table-name' [to,from]
//

void
tcpipClientObj::handleClientTable_FirstDate(void)
{
    std::string strTable;

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    std::deque<std::string> tokens;
    vscp_split(tokens, m_pClientItem->m_currentCommand, " ");

    // Get table name
    if (!tokens.empty()) {
        strTable = tokens.front();
        tokens.pop_front();
        vscp_trim(strTable);
    } else {
        // Problems: A table name must be given
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    pthread_mutex_lock(&m_pObj->m_mutexUserTables);

    CVSCPTable *pTable = m_pObj->m_userTableObjects.getTable(strTable);

    if (NULL == pTable) {
        // Failed
        write(MSG_FAILED_UNKNOWN_TABLE, strlen(MSG_FAILED_UNKNOWN_TABLE));

        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    vscpdatetime first;
    if (!pTable->getFirstDate(first)) {
        write(MSG_FAILED_TO_CLEAR_TABLE, strlen(MSG_FAILED_TO_CLEAR_TABLE));
        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    pthread_mutex_unlock(&m_pObj->m_mutexUserTables);

    std::string strReply =
      vscp_str_format("%s\r\n", (const char *)first.getISODateTime().c_str());
    write((const char *)strReply.c_str(),
          strlen((const char *)strReply.c_str()));

    write(MSG_OK, strlen(MSG_OK));

    return;
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_LastDate
//
// lastdate 'table-name' [to,from]
//

void
tcpipClientObj::handleClientTable_LastDate(void)
{
    std::string strTable;

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    std::deque<std::string> tokens;
    vscp_split(tokens, m_pClientItem->m_currentCommand, " ");

    // Get table name
    if (!tokens.empty()) {
        strTable = tokens.front();
        tokens.pop_front();
        vscp_trim(strTable);
    } else {
        // Problems: A table name must be given
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    pthread_mutex_lock(&m_pObj->m_mutexUserTables);

    CVSCPTable *pTable = m_pObj->m_userTableObjects.getTable(strTable);

    if (NULL == pTable) {
        // Failed
        write(MSG_FAILED_UNKNOWN_TABLE, strlen(MSG_FAILED_UNKNOWN_TABLE));

        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    vscpdatetime last;
    if (!pTable->getLastDate(last)) {
        write(MSG_FAILED_TO_CLEAR_TABLE, strlen(MSG_FAILED_TO_CLEAR_TABLE));
        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    pthread_mutex_unlock(&m_pObj->m_mutexUserTables);

    std::string strReply =
      vscp_str_format("%s\r\n", (const char *)last.getISODateTime().c_str());
    write((const char *)strReply.c_str(),
          strlen((const char *)strReply.c_str()));

    write(MSG_OK, strlen(MSG_OK));

    return;
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_Sum
//
// sum 'table-name' [to,from]
//

void
tcpipClientObj::handleClientTable_Sum(void)
{
    std::string strTable;
    vscpdatetime start;
    vscpdatetime end;

    // Initialize date range to 'all'
    start.set(("0000-01-01T00:00:00")); // The first date
    end.set(("9999-12-31T23:59:59"));   // The last date

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    std::deque<std::string> tokens;
    vscp_split(tokens, m_pClientItem->m_currentCommand, " ");

    // Get table name
    if (!tokens.empty()) {
        strTable = tokens.front();
        tokens.pop_front();
        vscp_trim(strTable);
    } else {
        // Problems: A table name must be given
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // get range if given
    if (tokens.size() >= 2) {

        if (!tokens.empty() && !start.set(tokens.front())) {
            write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            return;
        }
        tokens.pop_front();

        if (!tokens.empty() && !end.set(tokens.front())) {
            write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            return;
        }
        tokens.pop_front();
    }

    pthread_mutex_lock(&m_pObj->m_mutexUserTables);

    CVSCPTable *pTable = m_pObj->m_userTableObjects.getTable(strTable);

    if (NULL == pTable) {
        // Failed
        write(MSG_FAILED_UNKNOWN_TABLE, strlen(MSG_FAILED_UNKNOWN_TABLE));

        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    double sum;
    if (!pTable->getSumValue(start, end, &sum)) {
        write(MSG_FAILED_TO_CLEAR_TABLE, strlen(MSG_FAILED_TO_CLEAR_TABLE));
        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    pthread_mutex_unlock(&m_pObj->m_mutexUserTables);

    std::string strReply = vscp_str_format(("%f\r\n"), sum);
    write((const char *)strReply.c_str(),
          strlen((const char *)strReply.c_str()));

    write(MSG_OK, strlen(MSG_OK));

    return;
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_Min
//
// min 'table-name' [to,from]
//

void
tcpipClientObj::handleClientTable_Min(void)
{
    std::string strTable;
    vscpdatetime start;
    vscpdatetime end;

    // Initialize date range to 'all'
    start.set(("0000-01-01T00:00:00")); // The first date
    end.set(("9999-12-31T23:59:59"));   // The last date

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    std::deque<std::string> tokens;
    vscp_split(tokens, m_pClientItem->m_currentCommand, " ");

    // Get table name
    if (!tokens.empty()) {
        strTable = tokens.front();
        tokens.pop_front();
        vscp_trim(strTable);
    } else {
        // Problems: A table name must be given
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // get range if given
    if (tokens.size() >= 2) {

        if (!tokens.empty() && !start.set(tokens.front())) {
            write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            return;
        }
        tokens.pop_front();

        if (!tokens.empty() && !end.set(tokens.front())) {
            write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            return;
        }
        tokens.pop_front();
    }

    pthread_mutex_lock(&m_pObj->m_mutexUserTables);

    CVSCPTable *pTable = m_pObj->m_userTableObjects.getTable(strTable);

    if (NULL == pTable) {
        // Failed
        write(MSG_FAILED_UNKNOWN_TABLE, strlen(MSG_FAILED_UNKNOWN_TABLE));

        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    double min;
    if (!pTable->getMinValue(start, end, &min)) {
        write(MSG_FAILED_TO_CLEAR_TABLE, strlen(MSG_FAILED_TO_CLEAR_TABLE));
        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    pthread_mutex_unlock(&m_pObj->m_mutexUserTables);

    std::string strReply = vscp_str_format(("%f\r\n"), min);
    write((const char *)strReply.c_str(),
          strlen((const char *)strReply.c_str()));

    write(MSG_OK, strlen(MSG_OK));

    return;
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_Max
//
// max 'table-name' [to,from]
//

void
tcpipClientObj::handleClientTable_Max(void)
{
    std::string strTable;
    vscpdatetime start;
    vscpdatetime end;

    // Initialize date range to 'all'
    start.set(("0000-01-01T00:00:00")); // The first date
    end.set(("9999-12-31T23:59:59"));   // The last date

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    std::deque<std::string> tokens;
    vscp_split(tokens, m_pClientItem->m_currentCommand, " ");

    // Get table name
    if (!tokens.empty()) {
        strTable = tokens.front();
        tokens.pop_front();
        vscp_trim(strTable);
    } else {
        // Problems: A table name must be given
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // get range if given
    if (tokens.size() >= 2) {

        if (!tokens.empty() && !start.set(tokens.front())) {
            write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            return;
        }
        tokens.pop_front();

        if (!tokens.empty() && !end.set(tokens.front())) {
            write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            return;
        }
        tokens.pop_front();
    }

    pthread_mutex_lock(&m_pObj->m_mutexUserTables);

    CVSCPTable *pTable = m_pObj->m_userTableObjects.getTable(strTable);

    if (NULL == pTable) {
        // Failed
        write(MSG_FAILED_UNKNOWN_TABLE, strlen(MSG_FAILED_UNKNOWN_TABLE));

        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    double max;
    if (!pTable->getMaxValue(start, end, &max)) {
        write(MSG_FAILED_TO_CLEAR_TABLE, strlen(MSG_FAILED_TO_CLEAR_TABLE));
        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    pthread_mutex_unlock(&m_pObj->m_mutexUserTables);

    std::string strReply = vscp_str_format(("%f\r\n"), max);
    write((const char *)strReply.c_str(),
          strlen((const char *)strReply.c_str()));

    write(MSG_OK, strlen(MSG_OK));

    return;
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_Average
//
// average 'table-name' [to,from]
//

void
tcpipClientObj::handleClientTable_Average(void)
{
    std::string strTable;
    vscpdatetime start;
    vscpdatetime end;

    // Initialize date range to 'all'
    start.set(("0000-01-01T00:00:00")); // The first date
    end.set(("9999-12-31T23:59:59"));   // The last date

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    std::deque<std::string> tokens;
    vscp_split(tokens, m_pClientItem->m_currentCommand, " ");

    // Get table name
    if (!tokens.empty()) {
        strTable = tokens.front();
        tokens.pop_front();
        vscp_trim(strTable);
    } else {
        // Problems: A table name must be given
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // get range if given
    if (tokens.size() >= 2) {

        if (!tokens.empty() && !start.set(tokens.front())) {
            write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            return;
        }
        tokens.pop_front();

        if (!tokens.empty() && !end.set(tokens.front())) {
            write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            return;
        }
        tokens.pop_front();
    }

    pthread_mutex_lock(&m_pObj->m_mutexUserTables);

    CVSCPTable *pTable = m_pObj->m_userTableObjects.getTable(strTable);

    if (NULL == pTable) {
        // Failed
        write(MSG_FAILED_UNKNOWN_TABLE, strlen(MSG_FAILED_UNKNOWN_TABLE));

        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    double average;
    if (!pTable->getAverageValue(start, end, &average)) {
        write(MSG_FAILED_TO_CLEAR_TABLE, strlen(MSG_FAILED_TO_CLEAR_TABLE));
        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    pthread_mutex_unlock(&m_pObj->m_mutexUserTables);

    std::string strReply = vscp_str_format(("%f\r\n"), average);
    write((const char *)strReply.c_str(),
          strlen((const char *)strReply.c_str()));

    write(MSG_OK, strlen(MSG_OK));

    return;
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_Median
//
// median 'table-name' [to,from]
//

void
tcpipClientObj::handleClientTable_Median(void)
{
    std::string strTable;
    vscpdatetime start;
    vscpdatetime end;

    // Initialize date range to 'all'
    start.set(("0000-01-01T00:00:00")); // The first date
    end.set(("9999-12-31T23:59:59"));   // The last date

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    std::deque<std::string> tokens;
    vscp_split(tokens, m_pClientItem->m_currentCommand, " ");

    // Get table name
    if (!tokens.empty()) {
        strTable = tokens.front();
        tokens.pop_front();
        vscp_trim(strTable);
    } else {
        // Problems: A table name must be given
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // get range if given
    if (tokens.size() >= 2) {

        if (!tokens.empty() && !start.set(tokens.front())) {
            write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            return;
        }
        tokens.pop_front();

        if (!tokens.empty() && !end.set(tokens.front())) {
            write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            return;
        }
        tokens.pop_front();
    }

    pthread_mutex_lock(&m_pObj->m_mutexUserTables);

    CVSCPTable *pTable = m_pObj->m_userTableObjects.getTable(strTable);

    if (NULL == pTable) {
        // Failed
        write(MSG_FAILED_UNKNOWN_TABLE, strlen(MSG_FAILED_UNKNOWN_TABLE));

        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    double median;
    if (!pTable->getMedianValue(start, end, &median)) {
        write(MSG_FAILED_TO_CLEAR_TABLE, strlen(MSG_FAILED_TO_CLEAR_TABLE));
        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    pthread_mutex_unlock(&m_pObj->m_mutexUserTables);

    std::string strReply = vscp_str_format(("%f\r\n"), median);
    write((const char *)strReply.c_str(),
          strlen((const char *)strReply.c_str()));

    write(MSG_OK, strlen(MSG_OK));

    return;
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_StdDev
//
// stddev 'table-name' [to,from]
//

void
tcpipClientObj::handleClientTable_StdDev(void)
{
    std::string strTable;
    vscpdatetime start;
    vscpdatetime end;

    // Initialize date range to 'all'
    start.set(("0000-01-01T00:00:00")); // The first date
    end.set(("9999-12-31T23:59:59"));   // The last date

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    std::deque<std::string> tokens;
    vscp_split(tokens, m_pClientItem->m_currentCommand, " ");

    // Get table name
    if (!tokens.empty()) {
        strTable = tokens.front();
        tokens.pop_front();
        vscp_trim(strTable);
    } else {
        // Problems: A table name must be given
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // get range if given
    if (tokens.size() >= 2) {

        if (!tokens.empty() && !start.set(tokens.front())) {
            write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            return;
        }
        tokens.pop_front();

        if (!tokens.empty() && !end.set(tokens.front())) {
            write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            return;
        }
        tokens.pop_front();
    }

    pthread_mutex_lock(&m_pObj->m_mutexUserTables);

    CVSCPTable *pTable = m_pObj->m_userTableObjects.getTable(strTable);

    if (NULL == pTable) {
        // Failed
        write(MSG_FAILED_UNKNOWN_TABLE, strlen(MSG_FAILED_UNKNOWN_TABLE));

        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    double stdev;
    if (!pTable->getStdevValue(start, end, &stdev)) {
        write(MSG_FAILED_TO_CLEAR_TABLE, strlen(MSG_FAILED_TO_CLEAR_TABLE));
        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    pthread_mutex_unlock(&m_pObj->m_mutexUserTables);

    std::string strReply = vscp_str_format(("%f\r\n"), stdev);
    write((const char *)strReply.c_str(),
          strlen((const char *)strReply.c_str()));

    write(MSG_OK, strlen(MSG_OK));

    return;
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_Variance
//
// variance 'table-name' [to,from]
//

void
tcpipClientObj::handleClientTable_Variance(void)
{
    std::string strTable;
    vscpdatetime start;
    vscpdatetime end;

    // Initialize date range to 'all'
    start.set(("0000-01-01T00:00:00")); // The first date
    end.set(("9999-12-31T23:59:59"));   // The last date

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    std::deque<std::string> tokens;
    vscp_split(tokens, m_pClientItem->m_currentCommand, " ");

    // Get table name
    if (!tokens.empty()) {
        strTable = tokens.front();
        tokens.pop_front();
        vscp_trim(strTable);
    } else {
        // Problems: A table name must be given
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // get range if given
    if (tokens.size() >= 2) {

        if (!tokens.empty() && !start.set(tokens.front())) {
            write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            return;
        }
        tokens.pop_front();

        if (!tokens.empty() && !end.set(tokens.front())) {
            write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            return;
        }
        tokens.pop_front();
    }

    pthread_mutex_lock(&m_pObj->m_mutexUserTables);

    CVSCPTable *pTable = m_pObj->m_userTableObjects.getTable(strTable);

    if (NULL == pTable) {
        // Failed
        write(MSG_FAILED_UNKNOWN_TABLE, strlen(MSG_FAILED_UNKNOWN_TABLE));

        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    double variance;
    if (!pTable->getVarianceValue(start, end, &variance)) {
        write(MSG_FAILED_TO_CLEAR_TABLE, strlen(MSG_FAILED_TO_CLEAR_TABLE));
        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    pthread_mutex_unlock(&m_pObj->m_mutexUserTables);

    std::string strReply = vscp_str_format("%f\r\n", variance);
    write(strReply);

    write(MSG_OK, strlen(MSG_OK));

    return;
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_Mode
//
// mode 'table-name' [to,from]
//

void
tcpipClientObj::handleClientTable_Mode(void)
{
    std::string strTable;
    vscpdatetime start;
    vscpdatetime end;

    // Initialize date range to 'all'
    start.set(("0000-01-01T00:00:00")); // The first date
    end.set(("9999-12-31T23:59:59"));   // The last date

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    std::deque<std::string> tokens;
    vscp_split(tokens, m_pClientItem->m_currentCommand, " ");

    // Get table name
    if (!tokens.empty()) {
        strTable = tokens.front();
        tokens.pop_front();
        vscp_trim(strTable);
    } else {
        // Problems: A table name must be given
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // get range if given
    if (tokens.size() >= 2) {

        if (!tokens.empty() && !start.set(tokens.front())) {
            write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            return;
        }
        tokens.pop_front();

        if (!tokens.empty() && !end.set(tokens.front())) {
            write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            return;
        }
        tokens.pop_front();
    }

    pthread_mutex_lock(&m_pObj->m_mutexUserTables);

    CVSCPTable *pTable = m_pObj->m_userTableObjects.getTable(strTable);

    if (NULL == pTable) {
        // Failed
        write(MSG_FAILED_UNKNOWN_TABLE, strlen(MSG_FAILED_UNKNOWN_TABLE));

        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    double mode;
    if (!pTable->getModeValue(start, end, &mode)) {
        write(MSG_FAILED_TO_CLEAR_TABLE, strlen(MSG_FAILED_TO_CLEAR_TABLE));
        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    pthread_mutex_unlock(&m_pObj->m_mutexUserTables);

    std::string strReply = vscp_str_format(("%f\r\n"), mode);
    write((const char *)strReply.c_str(),
          strlen((const char *)strReply.c_str()));

    write(MSG_OK, strlen(MSG_OK));

    return;
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_LowerQ
//
// lowerq 'table-name' [to,from]
//

void
tcpipClientObj::handleClientTable_LowerQ(void)
{
    std::string strTable;
    vscpdatetime start;
    vscpdatetime end;

    // Initialize date range to 'all'
    start.set(("0000-01-01T00:00:00")); // The first date
    end.set(("9999-12-31T23:59:59"));   // The last date

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    std::deque<std::string> tokens;
    vscp_split(tokens, m_pClientItem->m_currentCommand, " ");

    // Get table name
    if (!tokens.empty()) {
        strTable = tokens.front();
        tokens.pop_front();
        vscp_trim(strTable);
    } else {
        // Problems: A table name must be given
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // get range if given
    if (tokens.size() >= 2) {

        if (!tokens.empty() && !start.set(tokens.front())) {
            write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            return;
        }
        tokens.pop_front();

        if (!tokens.empty() && !end.set(tokens.front())) {
            write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            return;
        }
        tokens.pop_front();
    }

    pthread_mutex_lock(&m_pObj->m_mutexUserTables);

    CVSCPTable *pTable = m_pObj->m_userTableObjects.getTable(strTable);

    if (NULL == pTable) {
        // Failed
        write(MSG_FAILED_UNKNOWN_TABLE, strlen(MSG_FAILED_UNKNOWN_TABLE));

        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    double lq;
    if (!pTable->getLowerQuartileValue(start, end, &lq)) {
        write(MSG_FAILED_TO_CLEAR_TABLE, strlen(MSG_FAILED_TO_CLEAR_TABLE));
        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    pthread_mutex_unlock(&m_pObj->m_mutexUserTables);

    std::string strReply = vscp_str_format(("%f\r\n"), lq);
    write((const char *)strReply.c_str(),
          strlen((const char *)strReply.c_str()));

    write(MSG_OK, strlen(MSG_OK));

    return;
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_upperq
//
// upperq 'table-name' [to,from]
//

void
tcpipClientObj::handleClientTable_UpperQ(void)
{
    std::string strTable;
    vscpdatetime start;
    vscpdatetime end;

    // Initialize date range to 'all'
    start.set(("0000-01-01T00:00:00")); // The first date
    end.set(("9999-12-31T23:59:59"));   // The last date

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    std::deque<std::string> tokens;
    vscp_split(tokens, m_pClientItem->m_currentCommand, " ");

    // Get table name
    if (!tokens.empty()) {
        strTable = tokens.front();
        tokens.pop_front();
        vscp_trim(strTable);
    } else {
        // Problems: A table name must be given
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // get range if given
    if (tokens.size() >= 2) {

        if (!tokens.empty() && !start.set(tokens.front())) {
            write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            return;
        }
        tokens.pop_front();

        if (!tokens.empty() && !end.set(tokens.front())) {
            write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            return;
        }
        tokens.pop_front();
    }

    pthread_mutex_lock(&m_pObj->m_mutexUserTables);

    CVSCPTable *pTable = m_pObj->m_userTableObjects.getTable(strTable);

    if (NULL == pTable) {
        // Failed
        write(MSG_FAILED_UNKNOWN_TABLE, strlen(MSG_FAILED_UNKNOWN_TABLE));

        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    double uq;
    if (!pTable->getUppeQuartileValue(start, end, &uq)) {
        write(MSG_FAILED_TO_CLEAR_TABLE, strlen(MSG_FAILED_TO_CLEAR_TABLE));
        pthread_mutex_unlock(&m_pObj->m_mutexUserTables);
        return;
    }

    pthread_mutex_unlock(&m_pObj->m_mutexUserTables);

    std::string strReply = vscp_str_format(("%f\r\n"), uq);
    write((const char *)strReply.c_str(),
          strlen((const char *)strReply.c_str()));

    write(MSG_OK, strlen(MSG_OK));

    return;
}

// -----------------------------------------------------------------------------
//                            E N D   T A B L E
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//                            E N D   T A B L E
// -----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// handleClientVariable
//

void
tcpipClientObj::handleClientVariable(void)
{
    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    if (m_pClientItem->CommandStartsWith(("list"))) {
        handleVariable_List();
    } else if (m_pClientItem->CommandStartsWith(("writevalue"))) {
        handleVariable_WriteValue();
    } else if (m_pClientItem->CommandStartsWith(("writenote"))) {
        handleVariable_WriteNote();
    } else if (m_pClientItem->CommandStartsWith(("write"))) {
        handleVariable_Write();
    } else if (m_pClientItem->CommandStartsWith(("readvalue"))) {
        handleVariable_ReadValue();
    } else if (m_pClientItem->CommandStartsWith(("readnote"))) {
        handleVariable_ReadNote();
    } else if (m_pClientItem->CommandStartsWith(("readreset"))) {
        handleVariable_ReadReset();
    } else if (m_pClientItem->CommandStartsWith(("readremove"))) {
        handleVariable_ReadRemove();
    } else if (m_pClientItem->CommandStartsWith(("read"))) {
        handleVariable_Read();
    } else if (m_pClientItem->CommandStartsWith(("reset"))) {
        handleVariable_Reset();
    } else if (m_pClientItem->CommandStartsWith(("remove"))) {
        handleVariable_Remove();
    } else if (m_pClientItem->CommandStartsWith(("length"))) {
        handleVariable_Length();
    } else if (m_pClientItem->CommandStartsWith(("load"))) {
        handleVariable_Load();
    } else if (m_pClientItem->CommandStartsWith(("save"))) {
        handleVariable_Save();
    } else {
        write(MSG_UNKNOWN_COMMAND, strlen(MSG_UNKNOWN_COMMAND));
    }
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_List
//
// variable list - List all variables.
// variable list test - List all variables with "test" in there name
//

void
tcpipClientObj::handleVariable_List(void)
{
    CVariable variable;
    std::string str;
    std::string strWork;
    std::string strSearch;
    int type = 0; // All variables

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    std::deque<std::string> tokens;
    vscp_split(tokens, m_pClientItem->m_currentCommand, " ");

    // Check for variable name
    if (!tokens.empty()) {

        std::string token = tokens.front();
        tokens.pop_front();

        vscp_trim(token);
        if (!tokens.empty()) {
            strSearch = token;
        } else {
            strSearch = ("(.*))"); // List all
        }
    } else {
        strSearch = ("(.*)"); // List all
    }

    // Check for variable type
    if (!tokens.empty()) {
        std::string str = tokens.front();
        tokens.pop_front();
        vscp_trim(str);
        type = vscp_readStringValue(str);
    }

    // Get all variable names
    std::deque<std::string> arrayVars;
    m_pObj->m_variables.getVarlistFromRegExp(arrayVars, strSearch);

    if (arrayVars.size()) {

        str = vscp_str_format(("%zu rows.\r\n"), arrayVars.size());
        write(str.c_str(), str.length());

        int cnt = 0;
        for (int i = 0; i < arrayVars.size(); i++) {
            if (0 != m_pObj->m_variables.find(arrayVars[i], variable)) {
                if ((0 == type) || (variable.getType() == type)) {
                    str = vscp_str_format("%d;", cnt);
                    str += variable.getAsString();
                    str += ("\r\n");
                    write(str.c_str(), str.length());
                    cnt++;
                }
            }
        }

    } else {
        str = ("0 rows.\r\n");
        write(str.c_str(), str.length());
    }

    write(MSG_OK, strlen(MSG_OK));
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_Write
//
// Format is: "variable
// name";"type";"persistence";"owner";"rights";"value";"note"
//
// test31;string;true;0;0x777;dGhpcyBpcyBhIHRlc3Q=;VGhpcyBpcyBhIG5vdGUgZm9yIGEgdGVzdCB2YXJpYWJsZQ==
//

void
tcpipClientObj::handleVariable_Write(void)
{
    CVariable variable;
    std::string str;

    bool bPersistence = false;

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    if (!variable.setFromString(
          m_pClientItem->m_currentCommand, m_pClientItem->m_UserName)) {
        // Failed to parse
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    if (m_pObj->m_variables.exist(variable.getName())) {

        // Update in database
        if (!m_pObj->m_variables.update(variable)) {
            write(MSG_VARIABLE_NO_SAVE, strlen(MSG_VARIABLE_NO_SAVE));
            return;
        }

    } else {

        // If the variable exist change value
        // if not - add it.
        if (!m_pObj->m_variables.add(variable)) {
            write(MSG_VARIABLE_UNABLE_ADD, strlen(MSG_VARIABLE_UNABLE_ADD));
            return;
        }
    }

    write(MSG_OK, strlen(MSG_OK));
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_WriteValue
//

void
tcpipClientObj::handleVariable_WriteValue(void)
{
    std::string name;
    std::string value;

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    std::deque<std::string> tokens;
    vscp_split(tokens, m_pClientItem->m_currentCommand, "\n");

    // Variable name
    if (!tokens.empty()) {
        name = tokens.front();
        tokens.pop_front();
    } else {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // Variable value
    if (!tokens.empty()) {

        value = tokens.front();
        tokens.pop_front();

    } else {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    CVariable variable;
    if (0 != m_pObj->m_variables.find(name, variable)) {

        // Set value and encode as BASE64 when expected
        variable.setValueFromString(variable.getType(), value, false);

        // Update in database
        if (!m_pObj->m_variables.update(variable)) {
            write(MSG_VARIABLE_NO_SAVE, strlen(MSG_VARIABLE_NO_SAVE));
            return;
        }

    } else {
        write(MSG_VARIABLE_NOT_DEFINED, strlen(MSG_VARIABLE_NOT_DEFINED));
    }

    write(MSG_OK, strlen(MSG_OK));
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_WriteNote
//

void
tcpipClientObj::handleVariable_WriteNote(void)
{
    std::string str;

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    std::string name;
    std::string note;

    std::deque<std::string> tokens;
    vscp_split(tokens, m_pClientItem->m_currentCommand, " ");

    // Variable name
    if (!tokens.empty()) {
        name = tokens.front();
        tokens.pop_front();
    } else {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    // Variable value
    if (!tokens.empty()) {

        note = tokens.front();
        tokens.pop_front();

    } else {
        write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
        return;
    }

    CVariable variable;
    if (0 != m_pObj->m_variables.find(name, variable)) {

        // Set value and encode as BASE64 when expected
        variable.setNote(note, true);

        // Update in database
        if (!m_pObj->m_variables.update(variable)) {
            write(MSG_VARIABLE_NO_SAVE, strlen(MSG_VARIABLE_NO_SAVE));
            return;
        }

    } else {
        write(MSG_VARIABLE_NOT_DEFINED, strlen(MSG_VARIABLE_NOT_DEFINED));
    }

    write(MSG_OK, strlen(MSG_OK));
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_Read
//

void
tcpipClientObj::handleVariable_Read(bool bOKResponse)
{
    std::string str;

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    CVariable variable;
    if (0 !=
        m_pObj->m_variables.find(m_pClientItem->m_currentCommand, variable)) {

        str = variable.getAsString(false);
        str += std::string("\r\n");
        write(str);

        write(MSG_OK, strlen(MSG_OK));

    } else {
        write(MSG_VARIABLE_NOT_DEFINED, strlen(MSG_VARIABLE_NOT_DEFINED));
    }
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_ReadVal
//

void
tcpipClientObj::handleVariable_ReadValue(bool bOKResponse)
{
    std::string str;

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    CVariable variable;
    if (0 !=
        m_pObj->m_variables.find(m_pClientItem->m_currentCommand, variable)) {

        variable.writeValueToString(str, true);
        str += ("\r\n");
        write(str.c_str(), strlen(str.c_str()));

        write(MSG_OK, strlen(MSG_OK));

    } else {
        write(MSG_VARIABLE_NOT_DEFINED, strlen(MSG_VARIABLE_NOT_DEFINED));
    }
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_ReadNote
//

void
tcpipClientObj::handleVariable_ReadNote(bool bOKResponse)
{
    std::string str;

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    CVariable variable;
    if (0 !=
        m_pObj->m_variables.find(m_pClientItem->m_currentCommand, variable)) {

        variable.getNote(str, true);
        str = ("+OK - ") + str + ("\r\n");
        write(str.c_str(), strlen(str.c_str()));

        write(MSG_OK, strlen(MSG_OK));

    } else {
        write(MSG_VARIABLE_NOT_DEFINED, strlen(MSG_VARIABLE_NOT_DEFINED));
    }
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_Reset
//

void
tcpipClientObj::handleVariable_Reset(void)
{
    std::string str;

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    CVariable variable;

    if (0 !=
        m_pObj->m_variables.find(m_pClientItem->m_currentCommand, variable)) {

        variable.reset();

        // Update in database
        if (!m_pObj->m_variables.update(variable)) {
            write(MSG_VARIABLE_NO_SAVE, strlen(MSG_VARIABLE_NO_SAVE));
            return;
        }

    } else {
        write(MSG_VARIABLE_NOT_DEFINED, strlen(MSG_VARIABLE_NOT_DEFINED));
    }

    write(MSG_OK, strlen(MSG_OK));
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_ReadReset
//

void
tcpipClientObj::handleVariable_ReadReset(void)
{
    std::string str;

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    if (m_pClientItem->CommandStartsWith(("vscp."))) {
        write(MSG_VARIABLE_NOT_STOCK, strlen(MSG_VARIABLE_NOT_STOCK));
        return;
    }

    CVariable variable;

    if (0 !=
        m_pObj->m_variables.find(m_pClientItem->m_currentCommand, variable)) {

        variable.writeValueToString(str);
        str = ("+OK - ") + str + ("\r\n");
        write(str.c_str(), strlen(str.c_str()));

        variable.reset();

        // Update in database
        if (!m_pObj->m_variables.update(variable)) {
            write(MSG_VARIABLE_NO_SAVE, strlen(MSG_VARIABLE_NO_SAVE));
            return;
        }

    } else {
        write(MSG_VARIABLE_NOT_DEFINED, strlen(MSG_VARIABLE_NOT_DEFINED));
    }

    write(MSG_OK, strlen(MSG_OK));
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_Remove
//

void
tcpipClientObj::handleVariable_Remove(void)
{
    std::string str;

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    if (m_pClientItem->CommandStartsWith(("vscp."))) {
        write(MSG_VARIABLE_NOT_STOCK, strlen(MSG_VARIABLE_NOT_STOCK));
        return;
    }

    if (!m_pObj->m_variables.remove(m_pClientItem->m_currentCommand)) {
        write(MSG_VARIABLE_NOT_DEFINED, strlen(MSG_VARIABLE_NOT_DEFINED));
    }

    write(MSG_OK, strlen(MSG_OK));
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_ReadRemove
//

void
tcpipClientObj::handleVariable_ReadRemove(void)
{
    std::string str;

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    if (m_pClientItem->CommandStartsWith(("vscp."))) {
        write(MSG_VARIABLE_NOT_STOCK, strlen(MSG_VARIABLE_NOT_STOCK));
        return;
    }

    CVariable variable;
    if (0 !=
        m_pObj->m_variables.find(m_pClientItem->m_currentCommand, variable)) {

        variable.writeValueToString(str);
        str = ("+OK - ") + str + ("\r\n");
        write(str.c_str(), strlen(str.c_str()));

        m_pObj->m_variables.remove(variable);

    } else {
        write(MSG_VARIABLE_NOT_DEFINED, strlen(MSG_VARIABLE_NOT_DEFINED));
    }

    write(MSG_OK, strlen(MSG_OK));
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_Length
//

void
tcpipClientObj::handleVariable_Length(void)
{
    std::string str;

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    CVariable variable;
    if (0 !=
        m_pObj->m_variables.find(m_pClientItem->m_currentCommand, variable)) {

        str = vscp_str_format("%zu", variable.getLength());
        str = str + "\r\n";
        write(str.c_str(), strlen(str.c_str()));

        m_pObj->m_variables.remove(variable);

    } else {
        write(MSG_VARIABLE_NOT_DEFINED, strlen(MSG_VARIABLE_NOT_DEFINED));
    }

    write(MSG_OK, strlen(MSG_OK));
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_Load
//

void
tcpipClientObj::handleVariable_Load(void)
{
    std::string path;                      // Empty to load from default path
    m_pObj->m_variables.loadFromXML(path); // TODO add path + type

    write(MSG_OK, strlen(MSG_OK));
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_Save
//

void
tcpipClientObj::handleVariable_Save(void)
{
    std::string path;

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    // Construct path to save to (always relative to root)
    // may not contain ".."
    path = m_pObj->m_rootFolder;
    path += m_pClientItem->m_currentCommand;

    if (path.npos != path.find((".."))) {
        write(MSG_INVALID_PATH, strlen(MSG_INVALID_PATH));
        return;
    }

    m_pObj->m_variables.save(path);

    write(MSG_OK, strlen(MSG_OK));
}

// ****************************************************************************

///////////////////////////////////////////////////////////////////////////////
// handleClientDm
//

void
tcpipClientObj::handleClientDm(void)
{
    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    syslog(LOG_ERR, "%s", m_pClientItem->m_currentCommand.c_str());

    vscp_trim(m_pClientItem->m_currentCommand);

    if (m_pClientItem->CommandStartsWith(("enable"))) {
        handleDM_Enable();
    } else if (m_pClientItem->CommandStartsWith(("disable"))) {
        handleDM_Enable();
    } else if (m_pClientItem->CommandStartsWith(("list"))) {
        handleDM_List();
    } else if (m_pClientItem->CommandStartsWith(("add"))) {
        handleDM_Add();
    } else if (m_pClientItem->CommandStartsWith(("delete"))) {
        handleDM_Delete();
    } else if (m_pClientItem->CommandStartsWith(("reset"))) {
        handleDM_Reset();
    } else if (m_pClientItem->CommandStartsWith(("trig"))) {
        handleDM_Trigger();
    } else if (m_pClientItem->CommandStartsWith(("clrtrig"))) {
        handleDM_ClearTriggerCount();
    } else if (m_pClientItem->CommandStartsWith(("clrerr"))) {
        handleDM_ClearErrorCount();
    }
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_Enable
//

void
tcpipClientObj::handleDM_Enable(void)
{
    unsigned short pos;

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    if (m_pClientItem->CommandStartsWith(("all"))) {

        pthread_mutex_lock(&m_pObj->m_dm.m_mutexDM);

        std::deque<dmElement *>::iterator it;
        for (it = m_pObj->m_dm.m_DMList.begin();
             it != m_pObj->m_dm.m_DMList.end();
             ++it) {
            dmElement *pDMItem = *it;
            pDMItem->enableRow();
        }

        pthread_mutex_unlock(&m_pObj->m_dm.m_mutexDM);

    } else {

        // Get the position
        pos = 0;

        pthread_mutex_lock(&m_pObj->m_dm.m_mutexDM);

        if (pos > (m_pObj->m_dm.m_DMList.size() - 1)) {
            write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            pthread_mutex_lock(&m_pObj->m_dm.m_mutexDM);
            return;
        }

        m_pObj->m_dm.m_DMList[pos]->enableRow();
        pthread_mutex_unlock(&m_pObj->m_dm.m_mutexDM);
    }

    write(MSG_OK, strlen(MSG_OK));
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_Disable
//

void
tcpipClientObj::handleDM_Disable(void)
{
    unsigned short pos;

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    if (m_pClientItem->CommandStartsWith(("all"))) {

        pthread_mutex_lock(&m_pObj->m_dm.m_mutexDM);

        std::deque<dmElement *>::iterator it;
        for (it = m_pObj->m_dm.m_DMList.begin();
             it != m_pObj->m_dm.m_DMList.end();
             ++it) {
            dmElement *pDMItem = *it;
            pDMItem->disableRow();
        }
        pthread_mutex_unlock(&m_pObj->m_dm.m_mutexDM);

    } else {

        // Get the position
        std::deque<std::string> tokens;
        vscp_split(tokens, m_pClientItem->m_currentCommand, ",");

        while (!tokens.empty()) {

            pos = vscp_readStringValue(tokens.front());
            tokens.pop_front();

            pthread_mutex_lock(&m_pObj->m_dm.m_mutexDM);

            if (pos > (m_pObj->m_dm.m_DMList.size() - 1)) {
                write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
                pthread_mutex_unlock(&m_pObj->m_dm.m_mutexDM);
                return;
            }

            m_pObj->m_dm.m_DMList[pos]->disableRow();
            pthread_mutex_unlock(&m_pObj->m_dm.m_mutexDM);
        }
    }

    write(MSG_OK, strlen(MSG_OK));
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_List
//

void
tcpipClientObj::handleDM_List(void)
{
    // Valid commands at this point
    // dm list
    // dm list all
    // dm list *
    // dm list 1
    // dm list 1,2,3,4,98

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    vscp_trim(m_pClientItem->m_currentCommand);

    // if "list" add "all"
    if (0 == m_pClientItem->m_currentCommand.length()) {
        m_pClientItem->m_currentCommand = ("ALL");
    }

    else if (m_pClientItem->CommandStartsWith(("new")) ||
             m_pClientItem->CommandStartsWith(("*"))) {

        pthread_mutex_lock(&m_pObj->m_dm.m_mutexDM);

        std::deque<dmElement *>::iterator it;
        for (it = m_pObj->m_dm.m_DMList.begin();
             it != m_pObj->m_dm.m_DMList.end();
             ++it) {

            dmElement *pDMItem = *it;
            std::string strRow = pDMItem->getAsString();

            write(strRow.c_str(), strlen(strRow.c_str()));
        }

        pthread_mutex_unlock(&m_pObj->m_dm.m_mutexDM);

    } else {

        // We have a list with specific rows  "list 1,8,9"
        //      first parse the argument to get the rows
        std::deque<int> rowArray;
        std::deque<std::string> tokens;
        vscp_split(tokens, m_pClientItem->m_currentCommand, ",");
        while (!tokens.empty()) {

            int n = vscp_readStringValue(tokens.front());
            tokens.pop_front();
            rowArray.push_back(n);
        }
    }

    write(MSG_OK, strlen(MSG_OK));
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_Add
//

void
tcpipClientObj::handleDM_Add(void)
{
    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    dmElement *pDMItem = new dmElement;

    std::deque<std::string> tokens;
    vscp_split(tokens, m_pClientItem->m_currentCommand, ",");

    // Priority
    pDMItem->m_vscpfilter.mask_priority = vscp_readStringValue(tokens.front());
    tokens.pop_front();
    pDMItem->m_vscpfilter.filter_priority =
      vscp_readStringValue(tokens.front());
    tokens.pop_front();

    // Class
    pDMItem->m_vscpfilter.mask_class = vscp_readStringValue(tokens.front());
    tokens.pop_front();
    pDMItem->m_vscpfilter.filter_class = vscp_readStringValue(tokens.front());
    tokens.pop_front();

    // Type
    pDMItem->m_vscpfilter.mask_type = vscp_readStringValue(tokens.front());
    tokens.pop_front();
    pDMItem->m_vscpfilter.filter_type = vscp_readStringValue(tokens.front());
    tokens.pop_front();

    // GUID
    std::string strGUID;
    strGUID = tokens.front();
    vscp_getGuidFromStringToArray(pDMItem->m_vscpfilter.mask_GUID, strGUID);
    strGUID = tokens.front();
    tokens.pop_front();
    vscp_getGuidFromStringToArray(pDMItem->m_vscpfilter.filter_GUID, strGUID);

    // action code
    pDMItem->m_actionCode = vscp_readStringValue(tokens.front());
    tokens.pop_front();

    // action parameters
    pDMItem->m_actionparam = tokens.front();
    tokens.pop_front();

    // add the DM row to the matrix
    pthread_mutex_lock(&m_pObj->m_dm.m_mutexDM);
    m_pObj->m_dm.addMemoryElement(pDMItem);
    pthread_mutex_unlock(&m_pObj->m_dm.m_mutexDM);

    write(MSG_OK, strlen(MSG_OK));
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_Delete
//

void
tcpipClientObj::handleDM_Delete(void)
{
    unsigned short pos;

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    if (m_pClientItem->CommandStartsWith(("all"))) {

        pthread_mutex_lock(&m_pObj->m_dm.m_mutexDM);

        std::deque<dmElement *>::iterator it;
        for (it = m_pObj->m_dm.m_DMList.begin();
             it != m_pObj->m_dm.m_DMList.end();
             ++it) {
            dmElement *pDMItem = *it;
            delete pDMItem;
        }

        m_pObj->m_dm.m_DMList.clear();

        pthread_mutex_unlock(&m_pObj->m_dm.m_mutexDM);

    } else {

        // Get the position
        std::deque<std::string> tokens;
        vscp_split(tokens, m_pClientItem->m_currentCommand, ",");

        while (!tokens.empty()) {

            pos = vscp_readStringValue(tokens.front());
            tokens.pop_front();

            pthread_mutex_lock(&m_pObj->m_dm.m_mutexDM);

            if (pos > (m_pObj->m_dm.m_DMList.size() - 1)) {
                write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
                pthread_mutex_unlock(&m_pObj->m_dm.m_mutexDM);
                return;
            }

            // Get the object
            dmElement *pDMItem = m_pObj->m_dm.m_DMList[pos];

            // Delete the node
            m_pObj->m_dm.m_DMList.erase(m_pObj->m_dm.m_DMList.begin() + pos);

            // Delete the object
            delete pDMItem;

            pthread_mutex_unlock(&m_pObj->m_dm.m_mutexDM);
        }
    }

    write(MSG_OK, strlen(MSG_OK));
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_Reset
//

void
tcpipClientObj::handleDM_Reset(void)
{
    m_pObj->stopDaemonWorkerThread();
    m_pObj->startDaemonWorkerThread();

    write(MSG_OK, strlen(MSG_OK));
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_Trigger
//

void
tcpipClientObj::handleDM_Trigger(void)
{
    unsigned short pos;

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    // Get the position
    std::deque<std::string> tokens;
    vscp_split(tokens, m_pClientItem->m_currentCommand, ",");

    while (!tokens.empty()) {

        pos = vscp_readStringValue(tokens.front());
        tokens.pop_front();

        pthread_mutex_lock(&m_pObj->m_dm.m_mutexDM);

        if (pos > (m_pObj->m_dm.m_DMList.size() - 1)) {
            write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
            pthread_mutex_unlock(&m_pObj->m_dm.m_mutexDM);
            return;
        }

        dmElement *pDMItem = m_pObj->m_dm.m_DMList[pos];
        pDMItem->doAction(NULL);

        pthread_mutex_unlock(&m_pObj->m_dm.m_mutexDM);
    }

    write(MSG_OK, strlen(MSG_OK));
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_ClearTriggerCount
//

void
tcpipClientObj::handleDM_ClearTriggerCount(void)
{
    unsigned short pos;

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    if (m_pClientItem->CommandStartsWith(("all"))) {

        pthread_mutex_lock(&m_pObj->m_dm.m_mutexDM);

        std::deque<dmElement *>::iterator it;
        for (it = m_pObj->m_dm.m_DMList.begin();
             it != m_pObj->m_dm.m_DMList.end();
             ++it) {
            dmElement *pDMItem      = *it;
            pDMItem->m_triggCounter = 0;
        }

        pthread_mutex_unlock(&m_pObj->m_dm.m_mutexDM);

    } else {

        // Get the position
        std::deque<std::string> tokens;
        vscp_split(tokens, m_pClientItem->m_currentCommand, ",");

        while (!tokens.empty()) {

            pos = vscp_readStringValue(tokens.front());
            tokens.pop_front();

            pthread_mutex_lock(&m_pObj->m_dm.m_mutexDM);

            if (pos > (m_pObj->m_dm.m_DMList.size() - 1)) {
                write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
                pthread_mutex_unlock(&m_pObj->m_dm.m_mutexDM);
                return;
            }

            dmElement *pDMItem      = m_pObj->m_dm.m_DMList[pos];
            pDMItem->m_triggCounter = 0;

            pthread_mutex_unlock(&m_pObj->m_dm.m_mutexDM);
        }
    }

    write(MSG_OK, strlen(MSG_OK));
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_ClearErrorCount
//

void
tcpipClientObj::handleDM_ClearErrorCount(void)
{
    unsigned short pos;

    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

    if (m_pClientItem->CommandStartsWith(("all"))) {

        pthread_mutex_lock(&m_pObj->m_dm.m_mutexDM);

        std::deque<dmElement *>::iterator it;
        for (it = m_pObj->m_dm.m_DMList.begin();
             it != m_pObj->m_dm.m_DMList.end();
             ++it) {
            dmElement *pDMItem      = *it;
            pDMItem->m_errorCounter = 0;
        }

        pthread_mutex_unlock(&m_pObj->m_dm.m_mutexDM);

    } else {

        // Get the position
        std::deque<std::string> tokens;
        vscp_split(tokens, m_pClientItem->m_currentCommand, ",");

        while (!tokens.empty()) {

            pos = vscp_readStringValue(tokens.front());
            tokens.pop_front();

            pthread_mutex_lock(&m_pObj->m_dm.m_mutexDM);

            if (pos > (m_pObj->m_dm.m_DMList.size() - 1)) {
                write(MSG_PARAMETER_ERROR, strlen(MSG_PARAMETER_ERROR));
                pthread_mutex_unlock(&m_pObj->m_dm.m_mutexDM);
                return;
            }

            dmElement *pDMItem      = m_pObj->m_dm.m_DMList[pos];
            pDMItem->m_errorCounter = 0;

            pthread_mutex_unlock(&m_pObj->m_dm.m_mutexDM);
        }
    }

    write(MSG_OK, strlen(MSG_OK));
}

///////////////////////////////////////////////////////////////////////////////
// handleClientList
//

void
tcpipClientObj::handleClientList(void)
{
    // TODO
    write(MSG_OK, strlen(MSG_OK));
}

///////////////////////////////////////////////////////////////////////////////
// handleClientDriver
//

void
tcpipClientObj::handleClientDriver(void)
{
    // TODO
}

///////////////////////////////////////////////////////////////////////////////
// handleClientHelp
//

void
tcpipClientObj::handleClientHelp(void)
{
    // Must be connected
    if (STCP_CONN_STATE_CONNECTED != m_conn->conn_state) return;

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
        str += "FILE              - File handling.\r\n";
        str += "UDP               - UDP.\r\n";
        str += "REMOTE            - User handling.\r\n";
        str += "INTERFACE         - Interface handling. \r\n";
        str += "DM                - Decision Matrix manipulation.\r\n";
        str += "VAR               - Variable handling. \r\n";
        str += "WCYD/WHATCANYOUDO - Check server capabilities. \r\n";
        write((const char *)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("+")) {
        std::string str = "'+' repeats the last given command.\r\n";
        write((const char *)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("noop")) {
        std::string str =
          "'NOOP' Does absolutely nothing but giving a success in return.\r\n";
        write((const char *)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith(("quit"))) {
        std::string str = "'QUIT' Quit a session with the VSCP daemon and "
                          "closes the m_connection.\r\n";
        write((const char *)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith(("user"))) {
        std::string str =
          "'USER' Used to login to the system together with PASS. Connection "
          "will be closed if bad credentials are given.\r\n";
        write((const char *)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith(("pass"))) {
        std::string str =
          "'PASS' Used to login to the system together with USER. Connection "
          "will be closed if bad credentials are given.\r\n";
        write((const char *)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith(("quit"))) {
        std::string str = "'QUIT' Quit a session with the VSCP daemon and "
                          "closes the m_connection.\r\n";
        write((const char *)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith(("send"))) {
        std::string str = "'SEND event'.\r\nThe event is given as "
                          "'head,class,type,obid,datetime,time-stamp,GUID,"
                          "data1,data2,data3....' \r\n";
        str +=
          "Normally set 'head' and 'obid' to zero. \r\nIf timestamp is set to "
          "zero it will be set by the server. \r\nIf GUID is given as '-' ";
        str += "the GUID of the interface will be used. \r\nThe GUID should "
               "be given on the form MSB-byte:MSB-byte-1:MSB-byte-2. \r\n";
        write((const char *)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith(("retr"))) {
        std::string str = "'RETR count' - Retrieve one (if no argument) or "
                          "'count' event(s). ";
        str += "Events are retrived on the form "
               "head,class,type,obid,datetime,time-stamp,GUID,data0,data1,"
               "data2,...........\r\n";
        write((const char *)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("rcvloop")) {
        std::string str = "'RCVLOOP' - Enter the receive loop and receive "
                          "events continously or until ";
        str += "terminated with 'QUITLOOP'. Events are retrived on the form "
               "head,class,type,obid,time-stamp,GUID,data0,data1,data2,......."
               "....\r\n";
        write((const char *)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith(("quitloop"))) {
        std::string str = "'QUITLOOP' - End 'RCVLOOP' event receives.\r\n";
        write((const char *)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("cdta") ||
               m_pClientItem->CommandStartsWith("chkdata")) {
        std::string str = "'CDTA' or 'CHKDATA' - Check if there is events in "
                          "the input queue.\r\n";
        write((const char *)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith(("clra")) ||
               m_pClientItem->CommandStartsWith(("clrall"))) {
        std::string str = "'CLRA' or 'CLRALL' - Clear input queue.\r\n";
        write((const char *)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith(("stat"))) {
        std::string str = "'STAT' - Get statistical information.\r\n";
        write((const char *)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("info")) {
        std::string str = "'INFO' - Get status information.\r\n";
        write((const char *)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("chid") ||
               m_pClientItem->CommandStartsWith("getchid")) {
        std::string str = "'CHID' or 'GETCHID' - Get channel id.\r\n";
        write((const char *)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("sgid") ||
               m_pClientItem->CommandStartsWith("setguid")) {
        std::string str = "'SGID' or 'SETGUID' - Set GUID for channel.\r\n";
        write((const char *)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("ggid") ||
               m_pClientItem->CommandStartsWith("getguid")) {
        std::string str = ("'GGID' or 'GETGUID' - Get GUID for channel.\r\n");
        write((const char *)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("vers") ||
               m_pClientItem->CommandStartsWith("version")) {
        std::string str =
          "'VERS' or 'VERSION' - Get version of VSCP daemon.\r\n";
        write((const char *)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("sflt") ||
               m_pClientItem->CommandStartsWith("setfilter")) {
        std::string str = "'SFLT' or 'SETFILTER' - Set filter for channel. ";
        str += "The format is 'filter-priority, filter-class, filter-type, "
               "filter-GUID' \r\n";
        str += "Example:  \r\nSETFILTER "
               "1,0x0000,0x0006,ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:"
               "00\r\n";
        write((const char *)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("smsk") ||
               m_pClientItem->CommandStartsWith("setmask")) {
        std::string str = "'SMSK' or 'SETMASK' - Set mask for channel. ";
        str += "The format is 'mask-priority, mask-class, mask-type, "
               "mask-GUID' \r\n";
        str += "Example:  \r\nSETMASK "
               "0x0f,0xffff,0x00ff,ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:"
               "00:00 \r\n";
        write((const char *)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith(("help"))) {
        std::string str = "'HELP [command]' This command. Gives help about "
                          "available commands and the usage.\r\n";
        write((const char *)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("test")) {
        std::string str = "'TEST [sequency]' Test command for debugging.\r\n";
        write((const char *)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("shutdown")) {
        std::string str = "'SHUTDOWN' Shutdown the daemon.\r\n";
        write((const char *)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("restart")) {
        std::string str = "'RESTART' Restart the daemon.\r\n";
        write((const char *)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith(("driver"))) {
        std::string str = "'DRIVER' Handle (load/unload/update/start/stop) "
                          "Level I/Level II drivers.\r\n";
        str += "'DRIVER install package' .\r\n";
        str += "'DRIVER uninstall package' .\r\n";
        str += "'DRIVER upgrade package' .\r\n";
        str += "'DRIVER start package' .\r\n";
        str += "'DRIVER stop package' .\r\n";
        str += "'DRIVER reload package' .\r\n";
        write((const char *)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("file")) {
        std::string str = "'FILE' Handle daemon files.\r\n";
        str += "'FILE dir'.\r\n";
        str += "'FILE copy'.\r\n";
        str += "'FILE move'.\r\n";
        str += "'FILE delete'.\r\n";
        str += "'FILE list'.\r\n";
        write((const char *)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("udp")) {
        std::string str = "'UDP' Handle UDP interface.\r\n";
        str += "'UDP enable'.\r\n";
        str += "'UDP disable' .\r\n";
        write((const char *)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith(("remote"))) {
        std::string str = "'REMOTE' User management.\r\n";
        str += "'REMOTE list'.\r\n";
        str += "'REMOTE add 'username','MD5 "
               "password','from-host(s)','access-right-list','event-list','"
               "filter','mask''. Add a user.\r\n";
        str += "'REMOTE remove username'.\r\n";
        str += "'REMOTE privilege 'username','access-right-list''.\r\n";
        str += "'REMOTE password 'username','MD5 for password' '.\r\n";
        str += "'REMOTE host-list 'username','host-list''.\r\n";
        str += "'REMOTE event-list 'username','event-list''.\r\n";
        str += "'REMOTE filter 'username','filter''.\r\n";
        str += "'REMOTE mask 'username','mask''.\r\n";
        write((const char *)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("interface")) {
        std::string str = "'INTERFACE' Handle interfaces on the daemon.\r\n";
        str += "'INTERFACE list'.\r\n";
        str += "'INTERFACE close'.\r\n";
        write((const char *)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("dm")) {
        std::string str = "'DM' Handle decision matrix on the daemon.\r\n";
        str += "'DM enable'.\r\n";
        str += "'DM disable'.\r\n";
        str += "'DM list'.\r\n";
        str += "'DM add'.\r\n";
        str += "'DM delete'.\r\n";
        str += "'DM reset'.\r\n";
        str += "'DM clrtrig'.\r\n";
        str += "'DM clrerr'.\r\n";
        str += "'DM load'.\r\n";
        str += "'DM save'.\r\n";
        write((const char *)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("var") ||
               m_pClientItem->CommandStartsWith("variable")) {
        std::string str = "'VARIABLE' Handle variables on the daemon.\r\n";
        str += "'VARIABLE list <regular-expression>'.\r\n";
        str += "'VARIABLE read <variable-name>'.\r\n";
        str += "'VARIABLE readvalue <variable-name>'.\r\n";
        str += "'VARIABLE readnote <variable-name>'.\r\n";
        str += "'VARIABLE write <variable-name> <variable>'.\r\n";
        str += "'VARIABLE writevalue <variable-name> <value>'.\r\n";
        str += "'VARIABLE writenote <variable-name>' <note>.\r\n";
        str += "'VARIABLE reset <variable-name>'.\r\n";
        str += "'VARIABLE readreset <variable-name>'.\r\n";
        str += "'VARIABLE remove <variable-name>'.\r\n";
        str += "'VARIABLE readremove <variable-name>'.\r\n";
        str += "'VARIABLE length <variable-name>'.\r\n";
        str += "'VARIABLE save <path> <selection>'.\r\n";
        write((const char *)str.c_str(), str.length());
    } else if (m_pClientItem->CommandStartsWith("wcyd") ||
               m_pClientItem->CommandStartsWith("whatcanyoudo")) {
        std::string str = "'WCYD/WHATCANYOUDO' Return the VSCP server "
                          "capabilities 64-bit array.\r\n";
        write((const char *)str.c_str(), str.length());
    } else {
        std::string str =
          vscp_str_format("The command '%s' is not available\r\n",
                          m_pClientItem->m_currentCommand.c_str());
        write((const char *)str.c_str(), str.length());
    }

    write(MSG_OK, strlen(MSG_OK));
    return;
}

///////////////////////////////////////////////////////////////////////////////
// Entry
//

void *
tcpipClientThread(void *pData)
{
    tcpipClientObj *ptcpipobj = (tcpipClientObj *)pData;
    if (NULL == ptcpipobj) {
        syslog(LOG_CRIT,
               "[TCP/IP srv client thread] Error, "
               "Client thread object not initialized.");
        return NULL;
    }

    if (NULL == ptcpipobj->m_pParent) {
        syslog(LOG_CRIT,
               "[TCP/IP srv client thread] Error, "
               "Control object not initialized.");
        return NULL;
    }

    syslog(LOG_DEBUG, "[TCP/IP srv client thread] Thread started.");

    ptcpipobj->m_pClientItem = new CClientItem();
    if (NULL == ptcpipobj->m_pClientItem) {
        syslog(LOG_CRIT,
               "[TCP/IP srv client thread] Memory error, "
               "Cant allocate client structure.");
        return NULL;
    }

    vscpdatetime now;
    ptcpipobj->m_pClientItem->m_bOpen = true;
    ptcpipobj->m_pClientItem->m_type  = CLIENT_ITEM_INTERFACE_TYPE_CLIENT_TCPIP;
    ptcpipobj->m_pClientItem->m_strDeviceName = ("Remote TCP/IP Server. [");
    ptcpipobj->m_pClientItem->m_strDeviceName +=
      ptcpipobj->m_pObj->m_strTcpInterfaceAddress;
    ptcpipobj->m_pClientItem->m_strDeviceName += ("]|Started at ");
    ptcpipobj->m_pClientItem->m_strDeviceName += now.getISODateTime();

    // Start of activity
    ptcpipobj->m_pClientItem->m_clientActivity = time(NULL);

    // Add the client to the Client List
    pthread_mutex_lock(&ptcpipobj->m_pObj->m_clientMutex);
    if (!ptcpipobj->m_pObj->addClient(ptcpipobj->m_pClientItem)) {
        // Failed to add client
        delete ptcpipobj->m_pClientItem;
        ptcpipobj->m_pClientItem = NULL;
        pthread_mutex_unlock(&ptcpipobj->m_pObj->m_clientMutex);
        syslog(LOG_ERR,
               "TCP/IP server: Failed to add client. Terminating thread.");
        return NULL;
    }
    pthread_mutex_unlock(&ptcpipobj->m_pObj->m_clientMutex);

    // Clear the filter (Allow everything )
    vscp_clearVSCPFilter(&ptcpipobj->m_pClientItem->m_filterVSCP);

    // Send welcome message
    std::string str = std::string(MSG_WELCOME);
    str += std::string("Version: ");
    str += std::string(VSCPD_DISPLAY_VERSION);
    str += std::string("\r\n");
    str += std::string(VSCPD_COPYRIGHT);
    str += std::string("\r\n");
    str += std::string(MSG_OK);
    ptcpipobj->write((const char *)str.c_str(), str.length());

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
            if (stcp_poll(
                  &fd, 1, 500, &(ptcpipobj->m_pParent->m_nStopTcpIpSrv)) < 0) {
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
        int pos;
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
            if (0 == strCommand.length()) continue;

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
    std::list<tcpipClientObj *>::iterator it;
    for (it = ptcpipobj->m_pParent->m_tcpip_clientList.begin();
         it != ptcpipobj->m_pParent->m_tcpip_clientList.end();
         ++it) {

        tcpipClientObj *pclient             = *it; // TODO check
        struct stcp_connection *stored_conn = pclient->m_conn;
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
    pthread_mutex_lock(&ptcpipobj->m_pObj->m_clientMutex);
    ptcpipobj->m_pObj->removeClient(ptcpipobj->m_pClientItem);
    pthread_mutex_unlock(&ptcpipobj->m_pObj->m_clientMutex);

    ptcpipobj->m_pClientItem = NULL;
    ptcpipobj->m_pParent     = NULL;

    syslog(LOG_INFO, "[TCP/IP srv client thread] Exit.");

    return NULL;
}
