///////////////////////////////////////////////////////////////////////////////
// VscpRemoteTcpIf.cpp
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

#ifdef __GNUG__
//#pragma implementation
#endif

#include <deque>
#include <list>
#include <string>

#include <math.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>

#include <version.h>
#include <vscp.h>
#include <vscpdatetime.h>
#include <vscpremotetcpif.h>

// Undef to get extra debug info
//#define DEBUG_INNER_COMMUNICTION

// Undef if debug messages is not wanted
//#define DEBUG_LIB_VSCP_HELPER   1

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

VscpRemoteTcpIf::VscpRemoteTcpIf()
{
    m_bModeReceiveLoop     = false;
    m_responseTimeOut      = TCPIP_DEFAULT_RESPONSE_TIMEOUT;
    m_innerResponseTimeout = TCPIP_DEFAULT_INNER_RESPONSE_TIMEOUT;

    // Init. register read parameters
    m_registerOpErrorTimeout  = TCPIP_REGISTER_READ_ERROR_TIMEOUT;
    m_registerOpResendTimeout = TCPIP_REGISTER_READ_RESEND_TIMEOUT;
    m_registerOpMaxRetries    = TCPIP_REGISTER_READ_MAX_TRIES;

    // Set default version info
    m_version_major   = VSCPD_MAJOR_VERSION;
    m_version_minor   = VSCPD_MINOR_VERSION;
    m_version_release = VSCPD_RELEASE_VERSION;
    m_version_build   = VSCPD_BUILD_VERSION;
}

VscpRemoteTcpIf::~VscpRemoteTcpIf()
{
    doCmdClose();
}

///////////////////////////////////////////////////////////////////////////////
// checkReturnValue
//

bool
VscpRemoteTcpIf::checkReturnValue(bool bClear)
{
    bool rv = false;
    char buf[8192];
    int nRead = -1;

    if (bClear) doClrInputQueue();

    uint32_t start = vscp_getMsTimeStamp();
    while (((vscp_getMsTimeStamp() - start) < m_responseTimeOut)) {

        memset(buf, 0, sizeof(buf));
        int nRead = stcp_read(m_conn, buf, sizeof(buf), m_innerResponseTimeout);

        if (nRead < 0) {
            if (STCP_ERROR_TIMEOUT == nRead) {
                // rv = VSCP_ERROR_TIMEOUT;
                rv = false;
                break;
            } else if (STCP_ERROR_STOPPED == nRead) {
                // rv = VSCP_ERROR_STOPPED;
                rv = false;
                break;
            }
            break;
        } else if (nRead > 0) {
            m_lastResponseTime =
              vscp_getMsTimeStamp(); // Save last response time
            m_strResponse += std::string(buf, nRead);
        }

        // if err abort
        if (NULL != strstr(buf, "+ERR")) break;

        // Check if we are done
        // A line with "+OK bla bla bla bla bla bla bla bla \r\n"
        // It can be hacked up.
        if (m_strResponse.npos != m_strResponse.find("+OK")) {

            // We check only for "\n" as it also could have been
            // broken up, even if rare
            if (NULL != strstr(buf, "\n")) {
                rv = true;
                break;
            }
        }

    } // while

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  doCommand
//

int
VscpRemoteTcpIf::doCommand(const char *cmd)
{
    std::string strCmd = cmd;
    return doCommand(strCmd);
}

///////////////////////////////////////////////////////////////////////////////
//  doCommand
//

/*int VscpRemoteTcpIf::doCommand( const std::string& cmd )
{
    bool ret = false;

#ifdef DEBUG_INNER_COMMUNICTION
    VSCP_LOG_DEBUG( "doCommand: " + cmd );
#endif

    doClrInputQueue();

    int n;
    if ( 0 == ( n = stcp_write( m_conn, (const char *)cmd.mbc_str(),
cmd.Length() ) ) || n != cmd.Length() ) { return VSCP_ERROR_ERROR;
    }

    ret = checkReturnValue( true );

    addInputStringArrayFromReply();

#ifdef DEBUG_INNER_COMMUNICTION
    VSCP_LOG_DEBUG(
"------------------------------------------------------------" );
    VSCP_LOG_DEBUG( "checkReturnValue:  Queue before. Responsetime = %u. ",
m_responseTimeOut ); for ( uint16_t i=0; i<m_inputStrArray.Count(); i++) {
        VSCP_LOG_DEBUG( "***** {" + m_inputStrArray[ i ] + "} *****" );
    }
    VSCP_LOG_DEBUG(
"------------------------------------------------------------" ); #endif

    if ( !ret ) {
#ifdef DEBUG_INNER_COMMUNICTION
        VSCP_LOG_DEBUG( "doCommand: checkReturnValue failed" );
#endif
        return VSCP_ERROR_ERROR;
    }

    return VSCP_ERROR_SUCCESS;
}*/

int
VscpRemoteTcpIf::doCommand(const std::string &cmd)
{
    bool ret = false;

    doClrInputQueue();

    int n;
    if (0 ==
          (n = stcp_write(m_conn, (const char *)cmd.c_str(), cmd.length())) ||
        n != cmd.length()) {
        return VSCP_ERROR_ERROR;
    }

    ret = checkReturnValue(true);
    addInputStringArrayFromReply();

    if (!ret) {
        return VSCP_ERROR_ERROR;
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getInputQueueCount
//

size_t
VscpRemoteTcpIf::getInputQueueCount(void)
{
    return m_inputStrArray.size();
}

///////////////////////////////////////////////////////////////////////////////
// doClrInputQueue
//

void
VscpRemoteTcpIf::doClrInputQueue(void)
{
    m_strResponse.clear();
    m_inputStrArray.clear();
}

///////////////////////////////////////////////////////////////////////////////
// addInputStringArrayFromReply
//

size_t
VscpRemoteTcpIf::addInputStringArrayFromReply(bool bClear)
{
    std::string tempStr;

    if (bClear) {
        m_inputStrArray.clear();
    }

    // If readBuffer have a "\r\n" pair at the end it just contains full
    // reply rows. If not we need to leave ending part after "\r\n" pair
    // in the read buffer.

    // Get rest string that should not be handled now
    // std::string tempStr = m_strResponse.AfterLast('\n');
    size_t fpos = m_strResponse.find_first_of('\n');
    if (fpos == m_strResponse.npos) {
        tempStr = m_strResponse;
    } else {
        tempStr = m_strResponse.substr(fpos + 1);
    }

    // Get the string that should be parsed for inclusion in the string array
    // std::string strToArray = m_strResponse.Left( m_strResponse.Length() -
    // tempStr.Length() );
    std::string strToArray =
      m_strResponse.substr(0, m_strResponse.length() - tempStr.length());

    // Set the readbuffer to the rest string
    m_strResponse = tempStr;

    // Parse the array string
    std::deque<std::string> tokens;
    vscp_split(tokens, strToArray, "\n");
    while (tokens.size()) {
        std::string str = tokens.front();
        vscp_trim(str);
        tokens.pop_front();
        m_inputStrArray.push_back(str);
    }

    return m_inputStrArray.size();
}

///////////////////////////////////////////////////////////////////////////////
// doCmdOpen
//

int
VscpRemoteTcpIf::doCmdOpen(const std::string &strInterface, uint32_t flags)
{
    std::string str;
    std::string strUsername;
    std::string strPassword;
    std::string strHostname;

    TCPIP_UNUSED(flags);

#ifdef DEBUG_LIB_VSCP_HELPER
    VSCP_LOG_DEBUG("strInterface = ");
    VSCP_LOG_DEBUG(strInterface);
#endif

    // Interface can be given as
    //      tcp://host:port;username;password
    //      host:port;username;password

    std::deque<std::string> tokens;
    vscp_split(tokens, strInterface, ";");

    // Hostname
    if (tokens.size()) {
        strHostname = tokens.front();
        tokens.pop_front();
        vscp_trim(strHostname);
        std::string prefix = "tcp://";
        vscp_startsWith(strHostname,prefix, &strHostname); // Remove "tcp://"
    }

#ifdef DEBUG_LIB_VSCP_HELPER
    VSCP_LOG_DEBUG("strHostname = ");
    VSCP_LOG_DEBUG(strHostname);
#endif

    // Username
    if (tokens.size()) {
        strUsername = tokens.front();
        tokens.pop_front();
    }

#ifdef DEBUG_LIB_VSCP_HELPER
    VSCP_LOG_DEBUG("strUsername = ");
    VSCP_LOG_DEBUG(strUsername);
#endif

    // Password
    if (tokens.size()) {
        strPassword = tokens.front();
        tokens.pop_front();
    }

#ifdef DEBUG_LIB_VSCP_HELPER
    VSCP_LOG_DEBUG("strPassword = ");
    VSCP_LOG_DEBUG(strPassword);
#endif

    return doCmdOpen(strHostname, strUsername, strPassword);
}

///////////////////////////////////////////////////////////////////////////////
// doCmdOpen
//

int
VscpRemoteTcpIf::doCmdOpen(const std::string &strHostname,
                           const std::string &strUsername,
                           const std::string &strPassword)
{
    std::string buf;
    std::string str;
    std::string host;
    int port;

#ifdef DEBUG_LIB_VSCP_HELPER
    VSCP_LOG_DEBUG(
      "============================================================");
    VSCP_LOG_DEBUG("Connect in progress with server " + strHostname);
    VSCP_LOG_DEBUG(
      "============================================================");
#endif

    std::string strHost = vscp_lower(strHostname);
    vscp_trim(strHost);
    vscp_startsWith(strHost,"tcp://", &strHost); // Remove "tcp://" if there

    std::deque<std::string> tokens;
    vscp_split(tokens, strHost, ":");
    if (tokens.size() < 2) return VSCP_ERROR_PARAMETER;
    host = tokens.front();
    tokens.pop_front();
    port = (int)vscp_readStringValue(tokens.front());
    tokens.pop_front();

    m_conn = stcp_connect_remote(
      (const char *)host.c_str(), port, TCPIP_DEFAULT_CONNECT_TIMEOUT_SECONDS);
    if (NULL == m_conn) {

#ifdef DEBUG_LIB_VSCP_HELPER
        VSCP_LOG_DEBUG("Connection failed." + strHostname);
#endif
        return VSCP_ERROR_TIMEOUT;
    }

#ifdef DEBUG_LIB_VSCP_HELPER
    VSCP_LOG_DEBUG("Checking server response");
#endif

    if (!checkReturnValue(true)) {

#ifdef DEBUG_LIB_VSCP_HELPER
        VSCP_LOG_DEBUG(_ "No +OK found " + strHostname);
#endif

        stcp_close_connection(m_conn);
        m_conn = NULL;
        return VSCP_ERROR_CONNECTION;
    }

    // Username
    str = strUsername;
    vscp_trim(str);
    ;
    buf = std::string("USER ") + str + std::string("\r\n");

    if (VSCP_ERROR_SUCCESS != doCommand(buf)) {
        stcp_close_connection(m_conn);
        m_conn = NULL;
        return VSCP_ERROR_USER;
    }

    // Password
    str = strPassword;
    vscp_trim(str);
    buf = std::string("PASS ") + str + std::string("\r\n");
    if (VSCP_ERROR_SUCCESS != doCommand(buf)) {
        stcp_close_connection(m_conn);
        m_conn = NULL;
        return VSCP_ERROR_PASSWORD;
    }

    // Get version (for internal versioning)
    uint8_t v1, v2, v3; // Not used - stored by method
    doCmdVersion(&v1, &v2, &v3);

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdOpen
//

int
VscpRemoteTcpIf::doCmdOpen(const std::string &strHostname,
                           short port,
                           const std::string &strUsername,
                           const std::string &strPassword)
{
    std::string strCombinedHost =
      strHostname + std::string(":") + vscp_str_format("%d", port);
    return doCmdOpen(strCombinedHost, strUsername, strPassword);
}

///////////////////////////////////////////////////////////////////////////////
// close
//

int
VscpRemoteTcpIf::doCmdClose(void)
{
    if (isConnected()) {

        // If receive loop active - end it
        if (m_bModeReceiveLoop) {
            doCommand("QUITLOOP\r\n");
        }

        // Try to behave
        doCommand("QUIT\r\n");

        // Clean up and close physical connection
        stcp_close_connection(m_conn);
        m_conn = NULL;
    }

    m_bModeReceiveLoop = false;
    m_inputStrArray.clear();

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdNOOP
//

int
VscpRemoteTcpIf::doCmdNOOP(void)
{
    if (!isConnected()) return VSCP_ERROR_CONNECTION;

    // If in receive loop terminate
    if (m_bModeReceiveLoop) return VSCP_ERROR_PARAMETER;

    return doCommand("NOOP\r\n");
}

///////////////////////////////////////////////////////////////////////////////
// doCmdClear
//

int
VscpRemoteTcpIf::doCmdClear(void)
{
    if (!isConnected()) return VSCP_ERROR_CONNECTION;

    // If in receive loop terminate
    if (m_bModeReceiveLoop) return VSCP_ERROR_PARAMETER;

    return doCommand("CLRA\r\n");
}

///////////////////////////////////////////////////////////////////////////////
// doCmdSend
//

int
VscpRemoteTcpIf::doCmdSend(const vscpEvent *pEvent)
{
    if (!isConnected()) return VSCP_ERROR_CONNECTION;

    // If in receive loop terminate
    if (m_bModeReceiveLoop) return VSCP_ERROR_PARAMETER;

    std::string strBuf, strWrk, strGUID;
    unsigned char guidsum = 0;

    // Must be a valid data pointer if data
    if ((pEvent->sizeData > 0) && (NULL == pEvent->pdata))
        return VSCP_ERROR_PARAMETER;

    // Validate datasize
    if (pEvent->sizeData > VSCP_MAX_DATA) return VSCP_ERROR_PARAMETER;

    // send head,class,type,obid,datetime,timestamp,GUID,data1,data2,data3....
    if (!vscp_writeVscpEventToString(pEvent, strBuf)) {
        return VSCP_ERROR_PARAMETER;
    }

    // Note that the caller must delete the event (if it should be deleted)

    strBuf = "send " + strBuf;
    strBuf += "\r\n";

    return doCommand(strBuf);
}

///////////////////////////////////////////////////////////////////////////////
// doCmdSendEx
//

int
VscpRemoteTcpIf::doCmdSendEx(const vscpEventEx *pEventEx)
{
    std::string strBuf, strWrk, strGUID;
    unsigned char guidsum = 0;

    if (!isConnected()) return VSCP_ERROR_CONNECTION;

    // If in receive loop terminate
    if (m_bModeReceiveLoop) return VSCP_ERROR_PARAMETER;

    if (NULL == pEventEx) return VSCP_ERROR_PARAMETER;

    // Validate datasize
    if (pEventEx->sizeData > VSCP_MAX_DATA) return VSCP_ERROR_PARAMETER;

    // VSCP_LOG_DEBUG( std::string::Format("%p crc=%p obid=%p year=%p month=%p
    // day=%p"), (void*)pEventEx, (void *)&(pEventEx->crc),
    // (void*)&(pEventEx->obid), (void*)&(pEventEx->year),
    // (void*)&(pEventEx->month), (void*)&(pEventEx->day) )); VSCP_LOG_DEBUG(
    // std::string::Format("vscpclass=%d vscptype=%d"),
    // (int)pEventEx->vscp_class, (int)pEventEx->vscp_type ));

    // send head,class,type,obid,datetime,timestamp,GUID,data1,data2,data3....
    if (!vscp_writeVscpEventExToString(pEventEx, strBuf)) {
        return VSCP_ERROR_PARAMETER;
    }

    strBuf = "send " + strBuf;
    strBuf += "\r\n";

    return doCommand(strBuf);
}

///////////////////////////////////////////////////////////////////////////////
// doCmdSendLevel1
//

int
VscpRemoteTcpIf::doCmdSendLevel1(const canalMsg *pCanalMsg)
{
    vscpEventEx event;

    if (!isConnected()) return VSCP_ERROR_CONNECTION;

    // If in receive loop terminate
    if (m_bModeReceiveLoop) return VSCP_ERROR_PARAMETER;

    if (NULL == pCanalMsg) return VSCP_ERROR_PARAMETER;

    event.vscp_class = (unsigned short)(0x1ff & (pCanalMsg->id >> 16));
    event.vscp_type  = (unsigned char)(0xff & (pCanalMsg->id >> 8));
    event.obid       = pCanalMsg->obid;
    event.timestamp  = pCanalMsg->timestamp;
    event.head       = 0x00;

    memset(event.GUID, 0, 16);

    event.GUID[0] = pCanalMsg->id & 0xff;

    // Protect event.data for out ouf bounce access
    if (sizeof(event.data) < pCanalMsg->sizeData) return VSCP_ERROR_PARAMETER;

    event.sizeData = pCanalMsg->sizeData;
    memcpy(event.data, pCanalMsg->data, pCanalMsg->sizeData);

    return doCmdSendEx(&event);
}

///////////////////////////////////////////////////////////////////////////////
// getEventFromLine
//

bool
VscpRemoteTcpIf::getEventFromLine(vscpEvent *pEvent, const std::string &strLine)
{
    // Check pointer
    if (NULL == pEvent) return false;

    // Read event from string
    if (!vscp_setVscpEventFromString(pEvent, strLine)) return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdReceive
//

int
VscpRemoteTcpIf::doCmdReceive(vscpEvent *pEvent)
{
    if (!isConnected()) return VSCP_ERROR_CONNECTION;
    if (NULL == pEvent) return VSCP_ERROR_PARAMETER;

    // If in receive loop terminate
    if (m_bModeReceiveLoop) return VSCP_ERROR_PARAMETER;

    if (VSCP_ERROR_SUCCESS != doCommand("RETR 1\r\n")) {
        return VSCP_ERROR_ERROR;
    }

    if (!getEventFromLine(pEvent, m_inputStrArray[0]))
        return VSCP_ERROR_PARAMETER;

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdReceiveEx
//

int
VscpRemoteTcpIf::doCmdReceiveEx(vscpEventEx *pEventEx)
{
    if (!isConnected()) return VSCP_ERROR_CONNECTION;
    if (NULL == pEventEx) return VSCP_ERROR_PARAMETER;

    // If in receive loop terminate
    if (m_bModeReceiveLoop) return VSCP_ERROR_PARAMETER;

    if (VSCP_ERROR_SUCCESS != doCommand("RETR 1\r\n")) {
        return VSCP_ERROR_ERROR;
    }

    vscpEvent *pEvent = new vscpEvent;
    if (NULL == pEvent) return VSCP_ERROR_PARAMETER;

    if (!getEventFromLine(pEvent, m_inputStrArray[0]))
        return VSCP_ERROR_PARAMETER;

    if (!vscp_convertVSCPtoEx(pEventEx, pEvent)) {
        vscp_deleteVSCPevent(pEvent);
        return VSCP_ERROR_PARAMETER;
    }

    vscp_deleteVSCPevent(pEvent);

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdReceiveLevel1
//

int
VscpRemoteTcpIf::doCmdReceiveLevel1(canalMsg *pCanalMsg)
{
    vscpEventEx event;

    if (!isConnected()) return VSCP_ERROR_CONNECTION;

    // Must have a valid pointer
    if (NULL == pCanalMsg) return VSCP_ERROR_PARAMETER;

    // If in receive loop terminate
    if (m_bModeReceiveLoop) return VSCP_ERROR_PARAMETER;

    // Fetch event
    if (VSCP_ERROR_SUCCESS != doCmdReceiveEx(&event)) return VSCP_ERROR_GENERIC;

    // No CAN or Level I event if data is > 8
    if (event.sizeData > 8) return VSCP_ERROR_GENERIC;

    pCanalMsg->id = (unsigned long)((event.head >> 5) << 20) |
                    ((unsigned long)event.vscp_class << 16) |
                    ((unsigned long)event.vscp_type << 8) | event.GUID[15];

    pCanalMsg->obid     = event.obid;
    pCanalMsg->sizeData = event.sizeData;
    if (pCanalMsg->sizeData) {
        memcpy(pCanalMsg->data, event.data, event.sizeData);
    }

    pCanalMsg->timestamp = event.timestamp;

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdEnterReceiveLoop
//

int
VscpRemoteTcpIf::doCmdEnterReceiveLoop(void)
{
    if (!isConnected()) return VSCP_ERROR_CONNECTION;

    // If in receive loop terminate
    if (m_bModeReceiveLoop) return VSCP_ERROR_SUCCESS;

    // Prevent false timeouts when starting up
    m_lastResponseTime = vscp_getMsTimeStamp();

    if (VSCP_ERROR_SUCCESS != doCommand("RCVLOOP\r\n")) {
        return VSCP_ERROR_ERROR;
    }

    // m_inputStrArray.Clear();

    m_bModeReceiveLoop = true;

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdQuitReceiveLoop
//

int
VscpRemoteTcpIf::doCmdQuitReceiveLoop(void)
{
    if (!isConnected()) return VSCP_ERROR_CONNECTION;

    // If **not** in receive loop terminate
    if (!m_bModeReceiveLoop) return VSCP_ERROR_SUCCESS;

    if (VSCP_ERROR_SUCCESS != doCommand("QUITLOOP\r\n")) {
        return VSCP_ERROR_TIMEOUT;
    }

    m_bModeReceiveLoop = false;
    m_inputStrArray.clear();

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdBlockingReceive
//
// Just works if in receive loop
//

int
VscpRemoteTcpIf::doCmdBlockingReceive(vscpEvent *pEvent, uint32_t mstimeout)
{
    char buf[8192];
    int nRead;

    // Check pointer
    if (NULL == pEvent) return VSCP_ERROR_PARAMETER;

    // Must be connected
    if (!isConnected()) return VSCP_ERROR_CONNECTION;

    // If **not** in receive loop terminate
    if (!m_bModeReceiveLoop) return VSCP_ERROR_PARAMETER;

    // If array already contains lines check if one of them is an event
    while (m_inputStrArray.size()) {

        std::string strItem = m_inputStrArray.front(); // Get first reply string
        m_inputStrArray.pop_front(); // Remove it from the array

        if ((strItem.npos == strItem.find("+OK")) &&
            (strItem.npos == strItem.find("+ERR"))) {

            // This should be an event - Get it
            if (!getEventFromLine(pEvent, strItem)) {
                return VSCP_ERROR_PARAMETER;
            }

            return VSCP_ERROR_SUCCESS;
        }
    }

    uint32_t startTime = vscp_getMsTimeStamp();

    while ((vscp_getMsTimeStamp() - startTime) < mstimeout) {

        if ((nRead = stcp_read(
               m_conn, buf, sizeof(buf), m_innerResponseTimeout)) <= 0) {

            if (STCP_ERROR_STOPPED == nRead) {
                stcp_close_connection(m_conn);
                return VSCP_ERROR_STOPPED;
            }

            usleep(100);
            continue;
        }

        m_lastResponseTime = vscp_getMsTimeStamp();

        // Save the response
        m_strResponse += std::string(buf, nRead);

        // Fill array
        addInputStringArrayFromReply();

        // Check if one of the items is an event
        while (m_inputStrArray.size()) {

            std::string strItem =
              m_inputStrArray.front();   // Get first reply string
            m_inputStrArray.pop_front(); // Remove it from the array

            // Not "+OK" or "+ERR"
            if ((strItem.npos == strItem.find("+OK")) &&
                (strItem.npos == strItem.find("+ERR"))) {

                // This should be an event - Get it
                if (!getEventFromLine(pEvent, strItem)) {
                    return VSCP_ERROR_PARAMETER;
                }

                return VSCP_ERROR_SUCCESS;
            }

        } // Lines in array

    } // timeout

    return VSCP_ERROR_TIMEOUT;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdBlockingReceive
//

int
VscpRemoteTcpIf::doCmdBlockingReceive(vscpEventEx *pEventEx, uint32_t timeout)
{
    int rv;
    vscpEvent e;

    // Check pointer
    if (NULL == pEventEx) return VSCP_ERROR_PARAMETER;

    // Must be connected
    if (!isConnected()) return VSCP_ERROR_CONNECTION;

    // If not receive loop active terminate
    if (!m_bModeReceiveLoop) return VSCP_ERROR_PARAMETER;

    if (VSCP_ERROR_SUCCESS != (rv = doCmdBlockingReceive(&e, timeout))) {
        return rv;
    }

    // Get event
    pEventEx->head       = e.head;
    pEventEx->vscp_class = e.vscp_class;
    e;
    pEventEx->vscp_type = e.vscp_type;
    pEventEx->obid      = e.obid;
    pEventEx->timestamp = e.timestamp;
    pEventEx->crc       = e.crc;
    pEventEx->sizeData  = e.sizeData;
    memcpy(pEventEx->GUID, e.GUID, 16);
    if ((NULL != e.pdata) && e.sizeData) {
        memcpy(pEventEx->data, e.pdata, e.sizeData);
        // Don't need the data anymore
        delete[] e.pdata;
        e.pdata = NULL;
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdDataAvailable
//

int
VscpRemoteTcpIf::doCmdDataAvailable(void)
{
    std::string strLine;
    int nMsg = 0;

    if (!isConnected()) return VSCP_ERROR_ERROR;

    // If receive loop active terminate
    if (m_bModeReceiveLoop) return VSCP_ERROR_ERROR;

    if (VSCP_ERROR_SUCCESS != doCommand("CDTA\r\n")) {
        return VSCP_ERROR_ERROR;
    }

    nMsg = atoi((const char *)m_inputStrArray.front().c_str());

    return nMsg;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdState
//

int
VscpRemoteTcpIf::doCmdStatus(canalStatus *pStatus)
{
    long val;
    std::string strBuf;
    std::string strWrk;
    std::string strLine;

    if (!isConnected()) return VSCP_ERROR_CONNECTION;

    // If receive loop active terminate
    if (m_bModeReceiveLoop) return VSCP_ERROR_PARAMETER;

    std::string strCmd("INFO\r\n");
    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    std::deque<std::string> tokens;
    vscp_split(tokens, m_inputStrArray.front(), ",");
    m_inputStrArray.pop_front();

    // lasterrorcode
    if (tokens.empty()) return VSCP_ERROR_GENERIC;
    pStatus->lasterrorcode = vscp_readStringValue(tokens.front());
    tokens.pop_front();

    // lasterrorsubcode
    if (tokens.empty()) return VSCP_ERROR_GENERIC;
    pStatus->lasterrorsubcode = vscp_readStringValue(tokens.front());
    tokens.pop_front();

    // lasterrorsubcode
    if (tokens.empty()) return VSCP_ERROR_GENERIC;
    strWrk = tokens.front();
    tokens.pop_front();
    strncpy(pStatus->lasterrorstr, strWrk.c_str(), strWrk.length());

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdState
//

int
VscpRemoteTcpIf::doCmdStatus(VSCPStatus *pStatus)
{
    canalStatus status;
    int rv = doCmdStatus(&status);

    pStatus->channel_status = status.channel_status;
    pStatus->lasterrorcode  = status.lasterrorcode;
    strncpy(pStatus->lasterrorstr,
            status.lasterrorstr,
            VSCP_STATUS_ERROR_STRING_SIZE);
    pStatus->lasterrorsubcode = status.lasterrorsubcode;

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdStatistics
//

int
VscpRemoteTcpIf::doCmdStatistics(VSCPStatistics *pStatistics)
{
    long val;
    std::string strBuf;
    std::string strWrk;
    std::string strLine;

    if (!isConnected()) return VSCP_ERROR_CONNECTION;

    // If receive loop active terminate
    if (m_bModeReceiveLoop) return VSCP_ERROR_PARAMETER;

    std::string strCmd("STAT\r\n");
    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    std::deque<std::string> tokens;
    vscp_split(tokens, m_inputStrArray.front(), ",");

    // Undefined - Bus off for CAN, no meaning for VSCP L2
    pStatistics->x = 0;
    if (!tokens.empty()) {
        pStatistics->x = vscp_readStringValue(tokens.front());
    }

    // Undefined - Bus warnings for CAN, no meaning for VSCP L2
    pStatistics->y = 0;
    if (!tokens.empty()) {
        pStatistics->y = vscp_readStringValue(tokens.front());
    }

    // Undefined - Not defined on server side
    pStatistics->z = 0;
    /*if ( !tokens.empty() ) {
        if ( ( tokens.front() ).ToLong( &val ) ) {
            pStatistics->z = val;
        }
        else {
            return VSCP_ERROR_GENERIC;
        }
    }*/

    // Overruns
    pStatistics->cntOverruns = 0;
    if (!tokens.empty()) {
        pStatistics->cntOverruns = vscp_readStringValue(tokens.front());
    }

    // Received data
    pStatistics->cntReceiveData = 0;
    if (!tokens.empty()) {
        pStatistics->cntReceiveData = vscp_readStringValue(tokens.front());
    }

    // Received Frames
    pStatistics->cntReceiveFrames = 0;
    if (!tokens.empty()) {
        pStatistics->cntReceiveFrames = vscp_readStringValue(tokens.front());
    }

    // Transmitted data
    pStatistics->cntTransmitData = 0;
    if (!tokens.empty()) {
        pStatistics->cntTransmitData = vscp_readStringValue(tokens.front());
    }

    // Transmitted frames
    pStatistics->cntTransmitFrames = 0;
    if (!tokens.empty()) {
        pStatistics->cntTransmitFrames = vscp_readStringValue(tokens.front());
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdStatistics
//

int
VscpRemoteTcpIf::doCmdStatistics(canalStatistics *pStatistics)
{
    int rv;
    VSCPStatistics vscpstat;

    if (NULL == pStatistics) return VSCP_ERROR_PARAMETER;

    if (VSCP_ERROR_SUCCESS != (rv = doCmdStatistics(&vscpstat))) {
        return rv;
    }

    // It may be tempting to just do a copy here but don't they
    // will be different in the future for sure.
    pStatistics->cntBusOff         = 0;
    pStatistics->cntBusWarnings    = 0;
    pStatistics->cntOverruns       = 0;
    pStatistics->cntReceiveData    = vscpstat.cntReceiveData;
    pStatistics->cntReceiveFrames  = vscpstat.cntReceiveFrames;
    pStatistics->cntTransmitData   = vscpstat.cntTransmitData;
    pStatistics->cntTransmitFrames = vscpstat.cntTransmitFrames;

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdFilter
//

int
VscpRemoteTcpIf::doCmdFilter(const vscpEventFilter *pFilter)
{
    std::string strCmd;

    if (!isConnected()) return VSCP_ERROR_CONNECTION;

    // If receive loop active terminate
    if (m_bModeReceiveLoop) return VSCP_ERROR_PARAMETER;

    // filter-priority, filter-class, filter-type, filter-GUID
    strCmd =
      vscp_str_format("SFLT "
                         "%d,%d,%d,%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%"
                         "02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n",
                         pFilter->filter_priority,
                         pFilter->filter_class,
                         pFilter->filter_type,
                         pFilter->filter_GUID[15],
                         pFilter->filter_GUID[14],
                         pFilter->filter_GUID[13],
                         pFilter->filter_GUID[12],
                         pFilter->filter_GUID[11],
                         pFilter->filter_GUID[10],
                         pFilter->filter_GUID[9],
                         pFilter->filter_GUID[8],
                         pFilter->filter_GUID[7],
                         pFilter->filter_GUID[6],
                         pFilter->filter_GUID[5],
                         pFilter->filter_GUID[4],
                         pFilter->filter_GUID[3],
                         pFilter->filter_GUID[2],
                         pFilter->filter_GUID[1],
                         pFilter->filter_GUID[0]);

    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    // mask-priority, mask-class, mask-type, mask-GUID
    strCmd =
      vscp_str_format("SMSK "
                         "%d,%d,%d,%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%"
                         "02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n",
                         pFilter->mask_priority,
                         pFilter->mask_class,
                         pFilter->mask_type,
                         pFilter->mask_GUID[15],
                         pFilter->mask_GUID[14],
                         pFilter->mask_GUID[13],
                         pFilter->mask_GUID[12],
                         pFilter->mask_GUID[11],
                         pFilter->mask_GUID[10],
                         pFilter->mask_GUID[9],
                         pFilter->mask_GUID[8],
                         pFilter->mask_GUID[7],
                         pFilter->mask_GUID[6],
                         pFilter->mask_GUID[5],
                         pFilter->mask_GUID[4],
                         pFilter->mask_GUID[3],
                         pFilter->mask_GUID[2],
                         pFilter->mask_GUID[1],
                         pFilter->mask_GUID[0]);
    // VSCP_LOG_DEBUG( strCmd );

    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdFilter  (with mask)
//

int
VscpRemoteTcpIf::doCmdFilter(const std::string &filter, const std::string &mask)
{
    std::string strCmd;

    if (!isConnected()) return VSCP_ERROR_CONNECTION;

    // If receive loop active terminate
    if (m_bModeReceiveLoop) return VSCP_ERROR_PARAMETER;

    // Set filter
    strCmd = "SFLT ";
    strCmd += filter;
    strCmd += "\r\n";
    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    // Set mask
    strCmd = "SMSK ";
    strCmd += mask;
    strCmd += "\r\n";
    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdVersion
//

int
VscpRemoteTcpIf::doCmdVersion(uint8_t *pMajorVer,
                              uint8_t *pMinorVer,
                              uint8_t *pSubMinorVer)
{
    long val;
    std::string strLine;

    if (!isConnected()) return VSCP_ERROR_CONNECTION;

    // If receive loop active terminate
    if (m_bModeReceiveLoop) return VSCP_ERROR_PARAMETER;

    std::string strCmd("VERS\r\n");
    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    std::deque<std::string> tokens;
    vscp_split(tokens, m_inputStrArray.front(), ",");

    // Major version
    *pMajorVer = 0;
    if (!tokens.empty()) {
        m_version_major = *pMajorVer =
          (uint8_t)vscp_readStringValue(tokens.front());
        tokens.pop_front();
    } else {
        return VSCP_ERROR_ERROR;
    }

    // Minor version
    *pMinorVer = 0;
    if (!tokens.empty()) {
        m_version_minor = *pMinorVer =
          (uint8_t)vscp_readStringValue(tokens.front());
        tokens.pop_front();
    } else {
        return VSCP_ERROR_ERROR;
    }

    // Sub minor version
    *pSubMinorVer = 0;
    if (!tokens.empty()) {
        m_version_release = *pSubMinorVer =
          (uint8_t)vscp_readStringValue(tokens.front());
        tokens.pop_front();
    } else {
        return VSCP_ERROR_ERROR;
    }

    // Build version
    m_version_build = VSCPD_BUILD_VERSION;
    if (!tokens.empty()) {
        m_version_build = (uint16_t)vscp_readStringValue(tokens.front());
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdVersion
//

int
VscpRemoteTcpIf::doCmdVersion(uint8_t *pMajorVer,
                              uint8_t *pMinorVer,
                              uint8_t *pSubMinorVer,
                              uint16_t *pBuildVer)
{
    int rv;

    if (VSCP_ERROR_SUCCESS !=
        (rv = doCmdVersion(pMajorVer, pMinorVer, pSubMinorVer))) {
        return rv;
    }

    *pBuildVer = m_version_build;

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdDLLVersion
//

unsigned long
VscpRemoteTcpIf::doCmdDLLVersion(void)
{
    return TCPIP_DLL_VERSION;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdVendorString
//

const char *
VscpRemoteTcpIf::doCmdVendorString(void)
{
    return TCPIP_VENDOR_STRING;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdGetDriverInfo
//

const char *
VscpRemoteTcpIf::doCmdGetDriverInfo(void)
{
    return DRIVER_INFO_STRING;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdGetGUID
//

int
VscpRemoteTcpIf::doCmdGetGUID(unsigned char *pGUID)
{
    long val;
    std::string strLine;

    if (!isConnected()) return VSCP_ERROR_CONNECTION;

    // If receive loop active terminate
    if (m_bModeReceiveLoop) return VSCP_ERROR_PARAMETER;

    std::string strCmd("GGID\r\n");
    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    if (getInputQueueCount() < 2) return VSCP_ERROR_ERROR;
    strLine = m_inputStrArray[m_inputStrArray.size() - 2];

    std::deque<std::string> tokens;
    vscp_split(tokens, strLine, ",");

    // GUID
    if (!tokens.empty()) {
        std::string str = tokens.front();
        tokens.pop_front();
        if (!vscp_getGuidFromStringToArray(pGUID, str)) {
            return VSCP_ERROR_GENERIC;
        }

    } else {
        return VSCP_ERROR_GENERIC;
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdGetGUID
//

int
VscpRemoteTcpIf::doCmdGetGUID(cguid &ifguid)
{
    std::string strLine;

    if (!isConnected()) return VSCP_ERROR_CONNECTION;

    // If receive loop active terminate
    if (m_bModeReceiveLoop) return VSCP_ERROR_PARAMETER;

    std::string strCmd("GGID\r\n");
    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    if (getInputQueueCount() < 2) return VSCP_ERROR_ERROR;
    strLine = m_inputStrArray[m_inputStrArray.size() - 2];

    ifguid.getFromString(strLine);

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdSetGUID
//

int
VscpRemoteTcpIf::doCmdSetGUID(const unsigned char *pGUID)
{
    std::string strLine;
    std::string strCmd;

    if (!isConnected()) return VSCP_ERROR_CONNECTION;

    if (NULL == pGUID) return VSCP_ERROR_GENERIC;

    // If receive loop active terminate
    if (m_bModeReceiveLoop) return VSCP_ERROR_PARAMETER;

    strCmd = vscp_str_format("SGID "
                                "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%"
                                "02x:%02x:%02x:%02x:%02x:%02x:%02x\r\n",
                                pGUID[0],
                                pGUID[1],
                                pGUID[2],
                                pGUID[3],
                                pGUID[4],
                                pGUID[5],
                                pGUID[6],
                                pGUID[7],
                                pGUID[8],
                                pGUID[9],
                                pGUID[10],
                                pGUID[11],
                                pGUID[12],
                                pGUID[13],
                                pGUID[14],
                                pGUID[15]);

    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdSetGUID
//

int
VscpRemoteTcpIf::doCmdSetGUID(cguid &ifguid)
{
    unsigned char GUID[16];
    memcpy(GUID, ifguid.getGUID(), 16);
    return doCmdSetGUID(GUID);
}

///////////////////////////////////////////////////////////////////////////////
// doCmdGetChannelInfo
//

int
VscpRemoteTcpIf::doCmdGetChannelInfo(VSCPChannelInfo *pChannelInfo)
{
    int rv;
    std::string strLine;

    if (!isConnected()) return VSCP_ERROR_CONNECTION;

    // Must have a valid pointer
    if (NULL == pChannelInfo) return VSCP_ERROR_PARAMETER;

    // If receive loop active terminate
    if (m_bModeReceiveLoop) return VSCP_ERROR_PARAMETER;

    std::string strCmd("INFO\r\n");
    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    if (getInputQueueCount() < 2) return VSCP_ERROR_ERROR;
    strLine = m_inputStrArray[m_inputStrArray.size() - 2];

    std::deque<std::string> tokens;
    vscp_split(tokens, strLine, ",");

    // Channel
    if (!tokens.empty()) {
        pChannelInfo->channel = (uint16_t)vscp_readStringValue(tokens.front());
        tokens.pop_front();
    } else {
        return VSCP_ERROR_GENERIC;
    }

    // Set the interface level/type
    pChannelInfo->channelType = CANAL_COMMAND_OPEN_VSCP_LEVEL2;

    // Get the channel GUID
    rv = doCmdGetGUID((unsigned char *)pChannelInfo->GUID);

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdGetChannelID
//

int
VscpRemoteTcpIf::doCmdGetChannelID(uint32_t *pChannelID)
{
    std::string strLine;

    if (!isConnected()) return VSCP_ERROR_CONNECTION;

    // Check pointer
    if (NULL == pChannelID) return VSCP_ERROR_PARAMETER;

    std::string strCmd("CHID\r\n");
    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    if (getInputQueueCount() < 2) return VSCP_ERROR_ERROR;
    strLine = m_inputStrArray[m_inputStrArray.size() - 2];

    unsigned long val;
    *pChannelID = stoul(strLine);

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdInterfaceList
//

int
VscpRemoteTcpIf::doCmdInterfaceList(std::deque<std::string> &strarray)
{
    if (!isConnected()) return VSCP_ERROR_CONNECTION;

    std::string strCmd("INTERFACE LIST\r\n");
    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    if (getInputQueueCount() < 2) return VSCP_ERROR_ERROR;

    // Handle the data (if any)
    for (unsigned int i = 0; i < getInputQueueCount(); i++) {
        if (std::string::npos == m_inputStrArray[i].find("+OK")) {
            strarray.push_back(m_inputStrArray[i]);
        }
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdShutdown
//

int
VscpRemoteTcpIf::doCmdShutDown(void)
{
    if (!isConnected()) return VSCP_ERROR_CONNECTION;

    std::string strCmd("SHUTDOWN\r\n");
    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
//                       R E M O T E    V A R I A B L E S
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariableList
//

int
VscpRemoteTcpIf::getRemoteVariableList(std::deque<std::string> &array,
                                       const std::string &regexp,
                                       const int type)
{
    std::string strCmd;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    strCmd = std::string("VAR LIST ") + regexp + std::string(" ");
    if (type) strCmd += vscp_str_format("%d", type);
    strCmd += std::string("\r\n");
    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    if (getInputQueueCount() < 2) return VSCP_ERROR_ERROR;

    // Handle the data (if any)
    for (unsigned int i = 0; i < getInputQueueCount(); i++) {
        if (std::string::npos == m_inputStrArray[i].find("+OK")) {
            array.push_back(m_inputStrArray[i]);
        }
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// saveRemoteVariablesToDisk
//

int
VscpRemoteTcpIf::saveRemoteVariablesToDisk(const std::string &path,
                                           const int select,
                                           const std::string &regExp)
{
    std::string strCmd;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    strCmd = std::string("VAR SAVE ") + path + std::string(" ");
    strCmd += vscp_str_format("%d ", select) + regExp;
    strCmd += std::string("\r\n");
    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// loadRemoteVariablesFromDisk
//

int
VscpRemoteTcpIf::loadRemoteVariablesFromDisk(const std::string &path)
{
    std::string strCmd;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    strCmd = std::string("VAR LOAD ") + path + std::string("\r\n");
    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// createRemoteVariable
//
// Format is: "variable
// name";"type";"persistence";"user";"rights";"value";"note"
//

int
VscpRemoteTcpIf::createRemoteVariable(const std::string &name,
                                      const std::string &type,
                                      const bool bPersistent,
                                      const std::string &strUser,
                                      const uint32_t rights,
                                      const std::string &strValue,
                                      const std::string &strNote)
{
    std::string strCmd;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    strCmd = std::string("VAR WRITE ") + name + std::string(";");
    strCmd += type + std::string(";");
    strCmd += ((bPersistent) ? std::string("TRUE") : std::string("FALSE")) +
              std::string(";");
    strCmd += strUser + std::string(";") +
              vscp_str_format("0x%lX", (unsigned long)rights) +
              std::string(";");
    strCmd += strValue + std::string(";") + strNote + std::string("\r\n");
    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// deleteVariable
//

int
VscpRemoteTcpIf::deleteRemoteVariable(const std::string &name)
{
    std::string strCmd;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    strCmd = std::string("VAR REMOVE ") + name + std::string("\r\n");
    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariableLastChange
//

int
VscpRemoteTcpIf::getRemoteVariableLastChange(const std::string &name,
                                             vscpdatetime &lastChange)
{
    std::string str;
    std::string strVariable;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    if (VSCP_ERROR_SUCCESS != getRemoteVariableAsString(name, strVariable)) {
        return VSCP_ERROR_ERROR;
    }

    // Format is: "variable
    // name";"type";"persistence";"user";"rights";lastchanged;"value";"note"
    std::deque<std::string> tokens;
    vscp_split(tokens, strVariable, ";");

    if (tokens.size() >= 6) {
        str = tokens[5]; // lastchange
    } else {
        return VSCP_ERROR_ERROR;
    }

    if (!lastChange.set(str)) {
        return VSCP_ERROR_ERROR;
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariableType
//

int
VscpRemoteTcpIf::getRemoteVariableType(const std::string &name, uint8_t *pType)
{
    std::string str;
    std::string strVariable;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    if (VSCP_ERROR_SUCCESS != getRemoteVariableAsString(name, strVariable)) {
        return VSCP_ERROR_ERROR;
    }

    // Format is: "variable
    // name";"type";"persistence";"user";"rights";lastchanged;"value";"note"
    std::deque<std::string> tokens;
    vscp_split(tokens, strVariable, ";");

    // name
    if (!tokens.empty()) {
        tokens.pop_front();
    }

    // type
    if (!tokens.empty()) {
        *pType = vscp_readStringValue(tokens.front());
        tokens.pop_front();
    } else {
        return VSCP_ERROR_ERROR;
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariablePersistence
//

int
VscpRemoteTcpIf::getRemoteVariablePersistence(const std::string &name,
                                              bool *pPersistent)
{
    std::string str;
    std::string strVariable;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    if (VSCP_ERROR_SUCCESS != getRemoteVariableAsString(name, strVariable)) {
        return VSCP_ERROR_ERROR;
    }

    // Format is: "variable
    // name";"type";"persistence";"user";"rights";lastchanged;"value";"note"
    std::deque<std::string> tokens;
    vscp_split(tokens, strVariable, ";");

    if (!tokens.empty()) tokens.pop_front(); // name
    if (!tokens.empty()) tokens.pop_front(); // type

    if (!tokens.empty()) {
        str = tokens.front(); // persistence
        tokens.pop_front();
        vscp_trim(str);
        if (0 == vscp_strcasecmp(str.c_str(), "TRUE")) {
            *pPersistent = true;
        } else {
            *pPersistent = false;
        }
    } else {
        return VSCP_ERROR_ERROR;
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariableOwner
//

int
VscpRemoteTcpIf::getRemoteVariableOwner(const std::string &name,
                                        uint32_t *pOwner)
{
    std::string str;
    std::string strVariable;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    if (VSCP_ERROR_SUCCESS != getRemoteVariableAsString(name, strVariable)) {
        return VSCP_ERROR_ERROR;
    }

    // Format is: "variable
    // name";"type";"persistence";"user";"rights";lastchanged;"value";"note"
    std::deque<std::string> tokens;
    vscp_split(tokens, strVariable, ";");

    if (!tokens.empty()) tokens.pop_front(); // name
    if (!tokens.empty()) tokens.pop_front(); // type
    if (!tokens.empty()) tokens.pop_front(); // persistence

    if (!tokens.empty()) {
        str = tokens.front(); // user
        tokens.pop_front();
        *pOwner = vscp_readStringValue(str);
    } else {
        return VSCP_ERROR_ERROR;
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariableAccessRights
//

int
VscpRemoteTcpIf::getRemoteVariableAccessRights(const std::string &name,
                                               uint16_t *pRights)
{
    std::string str;
    std::string strVariable;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    if (VSCP_ERROR_SUCCESS != getRemoteVariableAsString(name, strVariable)) {
        return VSCP_ERROR_ERROR;
    }

    // Format is: "variable
    // name";"type";"persistence";"user";"rights";lastchanged;"value";"note"
    std::deque<std::string> tokens;
    vscp_split(tokens, strVariable, ";");

    if (!tokens.empty()) tokens.pop_front(); // name
    if (!tokens.empty()) tokens.pop_front(); // type
    if (!tokens.empty()) tokens.pop_front(); // persistence
    if (!tokens.empty()) tokens.pop_front(); // user
    if (!tokens.empty()) {
        str = tokens.front(); // rights
        tokens.pop_front();
    } else {
        return VSCP_ERROR_ERROR;
    }

    *pRights = vscp_readStringValue(str);

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariableAsString
//

int
VscpRemoteTcpIf::getRemoteVariableAsString(const std::string &name,
                                           std::string &strValue)
{
    std::string strCmd;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    strCmd = "VAR READ " + name + "\r\n";
    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    if (getInputQueueCount() < 2) return VSCP_ERROR_ERROR;

    // Get variable data
    strValue = m_inputStrArray.front(); // BASE64 encoded value

    // Reply prior to 13.0.1 was
    //
    // +OK - variable-data<CR><LF>
    // +OK Success.
    //
    // After 13.0.1 reply from server is
    //
    // variable-data<CR><LF>
    // +OK Success.

    if (VSCP_VERSION(m_version_major, m_version_minor, m_version_release) <
        VSCP_VERSION(13, 0, 1)) {
        vscp_startsWith(
          strValue, "+OK - ", &strValue); // Remove "+OK - " if there
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setRemoteVariableFromString
//

int
VscpRemoteTcpIf::setRemoteVariableFromString(const std::string &name,
                                             const std::string &strValue,
                                             const bool bPersistent,
                                             const uint32_t rights)
{
    std::string strCmd;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    strCmd = "VAR WRITE " + name + ";STRING;";
    if (bPersistent) {
        strCmd += "true;";
    } else {
        strCmd += "false;";
    }
    strCmd += ";"; // User is logged in user
    strCmd += vscp_str_format("0x%lX", rights) + ";";
    strCmd += strValue + "\r\n";
    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariableValue
//

int
VscpRemoteTcpIf::getRemoteVariableValue(const std::string &name,
                                        std::string &strValue,
                                        bool bDecode)
{
    std::string strCmd;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    strCmd = "VAR READVALUE " + name + "\r\n";
    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    if (m_inputStrArray.size() < 2) return VSCP_ERROR_ERROR;

    // Get value
    strValue = m_inputStrArray.front();

    if (bDecode) {
        vscp_base64_std_decode(strValue);
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setRemoteVariableValue
//

int
VscpRemoteTcpIf::setRemoteVariableValue(const std::string &name,
                                        std::string &strValue,
                                        bool bEncode )
{
    std::string strLine;
    std::string strCmd;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    if (bEncode) {
        vscp_base64_std_encode(strValue);
    }

    strCmd = std::string("VAR WRITEVALUE ") + name + std::string(" ");
    strCmd += strValue;
    strCmd += std::string("\r\n");

    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariableBool
//

int
VscpRemoteTcpIf::getRemoteVariableBool(const std::string &name, bool *bValue)
{
    int rv;
    std::string strValue;

    // Check pointer
    if (NULL == bValue) return VSCP_ERROR_PARAMETER;

    if (VSCP_ERROR_SUCCESS != (rv = getRemoteVariableValue(name, strValue))) {
        return rv;
    }

    // Check the value
    vscp_trim(strValue);
    if (0 == vscp_strcasecmp(strValue.c_str(), "true")) {
        *bValue = true;
    } else {
        *bValue = false;
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setRemoteVariableBool
//

int
VscpRemoteTcpIf::setRemoteVariableBool(const std::string &name,
                                       const bool bValue)
{
    std::string strValue;

    if (bValue) {
        strValue = "TRUE";
    } else {
        strValue = "FALSE";
    }

    return setRemoteVariableValue(name, strValue);
}

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariableInt
//

int
VscpRemoteTcpIf::getRemoteVariableInt(const std::string &name, int *value)
{
    int rv;
    std::string strValue;

    // Check pointer
    if (NULL == value) return VSCP_ERROR_PARAMETER;

    if (VSCP_ERROR_SUCCESS != (rv = getRemoteVariableValue(name, strValue))) {
        return rv;
    }

    // Get the value
    *value = vscp_readStringValue(strValue);

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setRemoteVariableInt
//

int
VscpRemoteTcpIf::setRemoteVariableInt(const std::string &name, int value)
{
    std::string strValue;

    strValue = vscp_str_format("%d", value);
    return setRemoteVariableValue(name, strValue);
}

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariableLong
//

int
VscpRemoteTcpIf::getRemoteVariableLong(const std::string &name, long *value)
{
    int rv;
    std::string strValue;

    // Check pointer
    if (NULL == value) return VSCP_ERROR_PARAMETER;

    if (VSCP_ERROR_SUCCESS != (rv = getRemoteVariableValue(name, strValue))) {
        return rv;
    }

    // Get the value
    *value = vscp_readStringValue(strValue);

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setRemoteVariableLong
//

int
VscpRemoteTcpIf::setRemoteVariableLong(const std::string &name, long value)
{
    std::string strValue;

    strValue = vscp_str_format("%ld", value);
    return setRemoteVariableValue(name, strValue);
}

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariableDouble
//

int
VscpRemoteTcpIf::getRemoteVariableDouble(const std::string &name, double *value)
{
    int rv;
    std::string strValue;

    // Check pointer
    if (NULL == value) return VSCP_ERROR_PARAMETER;

    if (VSCP_ERROR_SUCCESS != (rv = getRemoteVariableValue(name, strValue))) {
        return rv;
    }

    // Get the value
    *value = atof(strValue.c_str());

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setRemoteVariableDouble
//

int
VscpRemoteTcpIf::setRemoteVariableDouble(const std::string &name, double value)
{
    std::string strValue;

    strValue = vscp_str_format("%f", value);
    return setRemoteVariableValue(name, strValue);
}

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariableMeasurement
//

int
VscpRemoteTcpIf::getRemoteVariableMeasurement(const std::string &name,
                                              double *pvalue,
                                              uint8_t *punit,
                                              uint8_t *psensoridx,
                                              uint8_t *pzone,
                                              uint8_t *psubzone)
{
    int rv;
    std::string str;
    std::string strValue;

    // Check pointers
    if (NULL == pvalue) return VSCP_ERROR_PARAMETER;
    if (NULL == punit) return VSCP_ERROR_PARAMETER;
    if (NULL == psensoridx) return VSCP_ERROR_PARAMETER;
    if (NULL == pzone) return VSCP_ERROR_PARAMETER;
    if (NULL == psubzone) return VSCP_ERROR_PARAMETER;

    if (VSCP_ERROR_SUCCESS != (rv = getRemoteVariableValue(name, strValue))) {
        return rv;
    }

    // value,unit,sensoridx,zone,subzone
    std::deque<std::string> tokens;
    vscp_split(tokens, strValue, ",");

    // Must have value
    if (tokens.size() < 5) {
        return VSCP_ERROR_INTERNAL; // Value should be there
    }

    str = tokens.front();
    tokens.pop_front();
    *pvalue = atof(str.c_str());

    str = tokens.front();
    tokens.pop_front();
    *punit = vscp_readStringValue(str);

    str = tokens.front();
    tokens.pop_front();
    *psensoridx = vscp_readStringValue(str);

    str = tokens.front();
    tokens.pop_front();
    *pzone = vscp_readStringValue(str);

    str = tokens.front();
    tokens.pop_front();
    *psubzone = vscp_readStringValue(str);

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setRemoteVariableMeasurement
//

int
VscpRemoteTcpIf::setRemoteVariableMeasurement(const std::string &name,
                                              double value,
                                              uint8_t unit,
                                              uint8_t sensoridx,
                                              uint8_t zone,
                                              uint8_t subzone)
{
    std::string strValue;

    strValue = vscp_str_format("%f,%d,%d,%d,%d",
                                  value,
                                  (int)unit,
                                  (int)sensoridx,
                                  (int)zone,
                                  (int)subzone);
    return setRemoteVariableValue(name, strValue);
}

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariableEvent
//

int
VscpRemoteTcpIf::getRemoteVariableEvent(const std::string &name,
                                        vscpEvent *pEvent)
{
    int rv;
    std::string strValue;

    // Check pointer
    if (NULL == pEvent) return VSCP_ERROR_PARAMETER;

    if (VSCP_ERROR_SUCCESS != (rv = getRemoteVariableValue(name, strValue))) {
        return rv;
    }

    vscp_setVscpEventFromString(pEvent, strValue);

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setRemoteVariableEvent
//

int
VscpRemoteTcpIf::setRemoteVariableEvent(const std::string &name,
                                        vscpEvent *pEvent)
{
    std::string strValue;

    // Check pointer
    if (NULL == pEvent) return VSCP_ERROR_PARAMETER;

    vscp_writeVscpEventToString(pEvent, strValue);
    return setRemoteVariableValue(name, strValue);
}

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariableEventEx
//

int
VscpRemoteTcpIf::getRemoteVariableEventEx(const std::string &name,
                                          vscpEventEx *pEventEx)
{
    int rv;
    std::string strValue;

    // Check pointer
    if (NULL == pEventEx) return VSCP_ERROR_PARAMETER;

    if (VSCP_ERROR_SUCCESS != (rv = getRemoteVariableValue(name, strValue))) {
        return rv;
    }

    vscp_setVscpEventExFromString(pEventEx, strValue);

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setRemoteVariableEventEx
//

int
VscpRemoteTcpIf::setRemoteVariableEventEx(const std::string &name,
                                          vscpEventEx *pEventEx)
{
    std::string strValue;

    // Check pointer
    if (NULL == pEventEx) return VSCP_ERROR_PARAMETER;

    vscp_writeVscpEventExToString(pEventEx, strValue);
    return setRemoteVariableValue(name, strValue);
}

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariableGUID
//

int
VscpRemoteTcpIf::getRemoteVariableGUID(const std::string &name, cguid &guid)
{
    int rv;
    std::string strValue;
    if (VSCP_ERROR_SUCCESS != (rv = getRemoteVariableValue(name, strValue))) {
        return rv;
    }

    guid.getFromString(strValue);

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setRemoteVariableGUID
//

int
VscpRemoteTcpIf::setRemoteVariableGUID(const std::string &name, cguid &guid)
{
    std::string strValue;

    guid.toString(strValue);
    return setRemoteVariableValue(name, strValue);
}

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariableVSCPdata
//

int
VscpRemoteTcpIf::getRemoteVariableVSCPdata(const std::string &name,
                                           uint8_t *pData,
                                           uint16_t *psize)
{
    int rv;
    std::string strValue;

    // Check pointers
    if (NULL == pData) return VSCP_ERROR_PARAMETER;
    if (NULL == psize) return VSCP_ERROR_PARAMETER;

    if (VSCP_ERROR_SUCCESS != (rv = getRemoteVariableValue(name, strValue))) {
        return rv;
    }

    vscp_setVscpDataArrayFromString(pData, psize, strValue);

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setRemoteVariableVSCPdata
//

int
VscpRemoteTcpIf::setRemoteVariableVSCPdata(const std::string &name,
                                           uint8_t *pData,
                                           uint16_t size)
{
    std::string strValue;

    vscp_writeVscpDataWithSizeToString(size, pData, strValue, false, false);
    return setRemoteVariableValue(name, strValue);
}

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariableVSCPclass
//

int
VscpRemoteTcpIf::getRemoteVariableVSCPclass(const std::string &name,
                                            uint16_t *vscp_class)
{
    int rv;
    std::string strValue;

    // Check pointer
    if (NULL == vscp_class) return VSCP_ERROR_PARAMETER;

    if (VSCP_ERROR_SUCCESS != (rv = getRemoteVariableValue(name, strValue))) {
        return rv;
    }

    *vscp_class = (uint16_t)stoul(strValue);

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariableVSCPclass
//

int
VscpRemoteTcpIf::setRemoteVariableVSCPclass(const std::string &name,
                                            uint16_t vscp_class)
{
    std::string strValue;

    strValue = vscp_str_format("%hu", vscp_class);
    return setRemoteVariableValue(name, strValue);
}

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariableVSCPtype
//

int
VscpRemoteTcpIf::getRemoteVariableVSCPtype(const std::string &name,
                                           uint16_t *vscp_type)
{
    int rv;
    std::string strValue;

    // Check pointer
    if (NULL == vscp_type) return VSCP_ERROR_PARAMETER;

    if (VSCP_ERROR_SUCCESS != (rv = getRemoteVariableValue(name, strValue))) {
        return rv;
    }

    *vscp_type = (uint16_t)stoul(strValue);

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariableVSCPtype
//

int
VscpRemoteTcpIf::setRemoteVariableVSCPtype(const std::string &name,
                                           uint16_t vscp_type)
{
    std::string strValue;

    strValue = vscp_str_format("%hu", vscp_type);
    return setRemoteVariableValue(name, strValue);
}

// ----

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariableVSCPTimestamp
//

int
VscpRemoteTcpIf::getRemoteVariableVSCPTimestamp(const std::string &name,
                                                uint32_t *vscp_timestamp)
{
    int rv;
    std::string strValue;

    // Check pointer
    if (NULL == vscp_timestamp) return VSCP_ERROR_PARAMETER;

    if (VSCP_ERROR_SUCCESS != (rv = getRemoteVariableValue(name, strValue))) {
        return rv;
    }

    *vscp_timestamp = (uint32_t)stoul(strValue);

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setRemoteVariableVSCPTimestamp
//

int
VscpRemoteTcpIf::setRemoteVariableVSCPTimestamp(const std::string &name,
                                                uint32_t vscp_timestamp)
{
    std::string strValue;

    strValue = vscp_str_format("%lu", (unsigned long)vscp_timestamp);
    return setRemoteVariableValue(name, strValue);
}

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariableDateTime
//

int
VscpRemoteTcpIf::getRemoteVariableDateTime(const std::string &name,
                                           vscpdatetime &datetime)
{
    int rv;
    std::string strValue;

    if (VSCP_ERROR_SUCCESS != (rv = getRemoteVariableValue(name, strValue))) {
        return rv;
    }

    datetime.set(strValue);

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setRemoteVariableDateTime
//

int
VscpRemoteTcpIf::setRemoteVariableDateTime(const std::string &name,
                                           vscpdatetime &datetime)
{
    std::string strValue;

    strValue = datetime.getISODateTime();
    return setRemoteVariableValue(name, strValue);
}

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariableDate
//

int
VscpRemoteTcpIf::getRemoteVariableDate(const std::string &name,
                                       vscpdatetime &date)
{
    int rv;
    std::string strValue;

    if (VSCP_ERROR_SUCCESS != (rv = getRemoteVariableValue(name, strValue))) {
        return rv;
    }

    date.setISODate(strValue);

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setRemoteVariableDate
//

int
VscpRemoteTcpIf::setRemoteVariableDate(const std::string &name,
                                       vscpdatetime &date)
{
    std::string strValue;

    strValue = date.getISODate();
    return setRemoteVariableValue(name, strValue);
}

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariableTime
//

int
VscpRemoteTcpIf::getRemoteVariableTime(const std::string &name,
                                       vscpdatetime &time)
{
    int rv;
    std::string strValue;

    if (VSCP_ERROR_SUCCESS != (rv = getRemoteVariableValue(name, strValue))) {
        return rv;
    }

    time.setISOTime(strValue);

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setRemoteVariableTime
//

int
VscpRemoteTcpIf::setRemoteVariableTime(const std::string &name,
                                       vscpdatetime &time)
{
    std::string strValue;

    strValue = time.getISOTime();
    return setRemoteVariableValue(name, strValue);
}

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariableBlob
//

int
VscpRemoteTcpIf::getRemoteVariableBlob(const std::string &name,
                                       std::string &blob)
{
    int rv;
    std::string strValue;

    if (VSCP_ERROR_SUCCESS != (rv = getRemoteVariableValue(name, blob))) {
        return rv;
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setRemoteVariableBlob
//

int
VscpRemoteTcpIf::setRemoteVariableBlob(const std::string &name,
                                       std::string &blob)
{
    std::string strValue;

    return setRemoteVariableValue(name, blob);
}

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariableMIME
//

int
VscpRemoteTcpIf::getRemoteVariableMIME(const std::string &name,
                                       std::string &mime)
{
    int rv;
    std::string strValue;

    if (VSCP_ERROR_SUCCESS != (rv = getRemoteVariableValue(name, mime))) {
        return rv;
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setRemoteVariableMIME
//

int
VscpRemoteTcpIf::setRemoteVariableMIME(const std::string &name,
                                       std::string &mime)
{
    std::string strValue;

    return setRemoteVariableValue(name, mime);
}

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariableHTML
//

int
VscpRemoteTcpIf::getRemoteVariableHTML(const std::string &name,
                                       std::string &html)
{
    int rv;
    std::string strValue;

    if (VSCP_ERROR_SUCCESS != (rv = getRemoteVariableValue(name, html))) {
        return rv;
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setRemoteVariableHTML
//

int
VscpRemoteTcpIf::setRemoteVariableHTML(const std::string &name,
                                       std::string &html)
{
    std::string strValue;

    return setRemoteVariableValue(name, html);
}

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariableJavaScript
//

int
VscpRemoteTcpIf::getRemoteVariableJavaScript(const std::string &name,
                                             std::string &js)
{
    int rv;
    std::string strValue;

    if (VSCP_ERROR_SUCCESS != (rv = getRemoteVariableValue(name, js))) {
        return rv;
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setRemoteVariableJavaScript
//

int
VscpRemoteTcpIf::setRemoteVariableJavaScript(const std::string &name,
                                             std::string &js)
{
    std::string strValue;

    return setRemoteVariableValue(name, js);
}

///////////////////////////////////////////////////////////////////////////////
// vgetRemoteVariableLUA
//

int
VscpRemoteTcpIf::getRemoteVariableLUA(const std::string &name, std::string &lua)
{
    int rv;
    std::string strValue;

    if (VSCP_ERROR_SUCCESS != (rv = getRemoteVariableValue(name, lua))) {
        return rv;
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setRemoteVariableLUA
//

int
VscpRemoteTcpIf::setRemoteVariableLUA(const std::string &name, std::string &lua)
{
    std::string strValue;

    return setRemoteVariableValue(name, lua);
}

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariableLUARES
//

int
VscpRemoteTcpIf::getRemoteVariableLUARES(const std::string &name,
                                         std::string &luares)
{
    int rv;
    std::string strValue;

    if (VSCP_ERROR_SUCCESS != (rv = getRemoteVariableValue(name, luares))) {
        return rv;
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setRemoteVariableLUARES
//

int
VscpRemoteTcpIf::setRemoteVariableLUARES(const std::string &name,
                                         std::string &luares)
{
    std::string strValue;

    return setRemoteVariableValue(name, luares);
}

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariableUX1
//

int
VscpRemoteTcpIf::getRemoteVariableUX1(const std::string &name, std::string &ux1)
{
    int rv;
    std::string strValue;

    if (VSCP_ERROR_SUCCESS != (rv = getRemoteVariableValue(name, ux1))) {
        return rv;
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setRemoteVariableUX1
//

int
VscpRemoteTcpIf::setRemoteVariableUX1(const std::string &name, std::string &ux1)
{
    std::string strValue;

    return setRemoteVariableValue(name, ux1);
}

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariableDMROW
//

int
VscpRemoteTcpIf::getRemoteVariableDMROW(const std::string &name,
                                        std::string &dmrow)
{
    int rv;
    std::string strValue;

    if (VSCP_ERROR_SUCCESS != (rv = getRemoteVariableValue(name, dmrow))) {
        return rv;
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setRemoteVariableDMROW
//

int
VscpRemoteTcpIf::setRemoteVariableDMROW(const std::string &name,
                                        std::string &dmrow)
{
    std::string strValue;

    return setRemoteVariableValue(name, dmrow);
}

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariableDriver
//

int
VscpRemoteTcpIf::getRemoteVariableDriver(const std::string &name,
                                         std::string &drv)
{
    int rv;
    std::string strValue;

    if (VSCP_ERROR_SUCCESS != (rv = getRemoteVariableValue(name, drv))) {
        return rv;
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setRemoteVariableDriver
//

int
VscpRemoteTcpIf::setRemoteVariableDriver(const std::string &name,
                                         std::string &drv)
{
    std::string strValue;

    return setRemoteVariableValue(name, drv);
}

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariableUser
//

int
VscpRemoteTcpIf::getRemoteVariableUser(const std::string &name,
                                       std::string &user)
{
    int rv;
    std::string strValue;

    if (VSCP_ERROR_SUCCESS != (rv = getRemoteVariableValue(name, user))) {
        return rv;
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setRemoteVariableUser
//

int
VscpRemoteTcpIf::setRemoteVariableUser(const std::string &name,
                                       std::string &user)
{
    std::string strValue;

    return setRemoteVariableValue(name, user);
}

///////////////////////////////////////////////////////////////////////////////
// getRemoteVariableFilter
//

int
VscpRemoteTcpIf::getRemoteVariableFilter(const std::string &name,
                                         std::string &filter)
{
    int rv;
    std::string strValue;

    if (VSCP_ERROR_SUCCESS != (rv = getRemoteVariableValue(name, filter))) {
        return rv;
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setRemoteVariableFilter
//

int
VscpRemoteTcpIf::setRemoteVariableFilter(const std::string &name,
                                         std::string &filter)
{
    std::string strValue;

    return setRemoteVariableValue(name, filter);
}

////////////////////////////////////////////////////////////////////////////////
//                        T A B L E   H E L P E R S
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// tableCreate
//

int
VscpRemoteTcpIf::tableCreate(const std::string &defTable)
{
    std::string strCmd;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    strCmd = "TABLE CREATE " + defTable;
    strCmd += "\r\n";

    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    return VSCP_ERROR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
// tableDelete
//

int
VscpRemoteTcpIf::tableDelete(const std::string &tblName, bool bDeleteFile)
{
    std::string strCmd;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    strCmd = "TABLE DELETE " + tblName + " ";
    if (bDeleteFile) {
        strCmd += "TRUE";
    }
    strCmd += "\r\n";

    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    return VSCP_ERROR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
// tableList
//

int
VscpRemoteTcpIf::tableList(std::deque<std::string> &tablesArray)
{
    std::string strCmd;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    strCmd = "TABLE LIST\r\n";
    // if ( type ) strCmd += vscp_str_format("%d", type );
    // strCmd += "\r\n";
    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    if (getInputQueueCount() < 2) return VSCP_ERROR_ERROR;

    // Handle the data (if any)
    for (unsigned int i = 0; i < getInputQueueCount(); i++) {
        if (std::string::npos == m_inputStrArray[i].find("+OK")) {
            tablesArray.push_back(m_inputStrArray[i]);
        };
    }

    return VSCP_ERROR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
// tableListInfo
//

int
VscpRemoteTcpIf::tableListInfo(const std::string &tblName,
                               std::string &tableInfo,
                               bool bXML)
{
    std::string strCmd;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    strCmd = "TABLE LIST ";
    strCmd += tblName + " ";
    if (bXML) {
        strCmd += "XML";
    }
    strCmd += "\r\n";

    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    if (getInputQueueCount() < 2) return VSCP_ERROR_ERROR;

    // Handle the data (if any)
    for (unsigned int i = 0; i < getInputQueueCount(); i++) {
        if (std::string::npos == m_inputStrArray[i].find("+OK")) {
            tableInfo += m_inputStrArray[i];
        }
    }

    return VSCP_ERROR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
// tableGet
//

int
VscpRemoteTcpIf::tableGet(const std::string &tblName,
                          const vscpdatetime &from,
                          const vscpdatetime &to,
                          std::deque<std::string> &resultArray,
                          bool bFull)
{
    std::string strCmd;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    strCmd = "TABLE GET ";
    strCmd += tblName + " ";
    strCmd += from.getISODateTime();
    strCmd += " ";
    strCmd += to.getISODateTime();
    strCmd += " ";
    if (bFull) {
        strCmd += "FULL";
    }
    strCmd += "\r\n";

    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    if (getInputQueueCount() < 2) return VSCP_ERROR_ERROR;

    // Handle the data (if any)
    for (unsigned int i = 0; i < getInputQueueCount(); i++) {
        if (std::string::npos == m_inputStrArray[i].find("+OK")) {
            resultArray.push_back(m_inputStrArray[i]);
        }
    }

    return VSCP_ERROR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
// tableGetRaw
//

int
VscpRemoteTcpIf::tableGetRaw(const std::string &tblName,
                             const vscpdatetime &from,
                             const vscpdatetime &to,
                             std::deque<std::string> &resultArray)
{
    std::string strCmd;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    strCmd = "TABLE GETRAW ";
    strCmd += tblName + " ";
    strCmd += from.getISODateTime() + " ";
    strCmd += to.getISODateTime() + " ";
    strCmd += "\r\n";

    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    if (getInputQueueCount() < 2) return VSCP_ERROR_ERROR;

    // Handle the data (if any)
    for (unsigned int i = 0; i < getInputQueueCount(); i++) {
        if (std::string::npos == m_inputStrArray[i].find("+OK")) {
            resultArray.push_back(m_inputStrArray[i]);
        }
    }

    return VSCP_ERROR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
// tableLog
//

int
VscpRemoteTcpIf::tableLog(const std::string &tblName,
                          double value,
                          vscpdatetime *pdt)
{
    std::string strCmd;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    strCmd = "TABLE LOG " + tblName + " ";
    strCmd = vscp_str_format("%f ", value);
    if (NULL != pdt) {
        strCmd += pdt->getISODateTime();
    } else {
        strCmd += vscpdatetime::Now().getISODateTime();
    }
    strCmd += "\r\n";

    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    return VSCP_ERROR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
// tableLogSQL
//

int
VscpRemoteTcpIf::tableLogSQL(const std::string &tblName, const std::string &sql)
{
    std::string strCmd;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    strCmd = "TABLE LOG " + tblName + " " + sql;
    strCmd += "\r\n";

    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    return VSCP_ERROR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
// tableGetNumRecords
//

int
VscpRemoteTcpIf::tableGetNumRecords(const std::string &tblName,
                                    const vscpdatetime &from,
                                    const vscpdatetime &to,
                                    size_t *pRecords)
{
    std::string strLine;
    std::string strCmd;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.
    if (NULL == pRecords) return VSCP_ERROR_PARAMETER;

    strCmd = "TABLE RECORDS " + tblName + " ";
    strCmd += from.getISODateTime() + " ";
    strCmd += to.getISODateTime() + " ";
    strCmd += "\r\n";

    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    if (getInputQueueCount() < 2) return VSCP_ERROR_ERROR;
    strLine = m_inputStrArray[m_inputStrArray.size() - 2];

    *pRecords = stoul(strLine);

    return VSCP_ERROR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
// tableGetFirstDate
//

int
VscpRemoteTcpIf::tableGetFirstDate(const std::string &tblName,
                                   const vscpdatetime &from,
                                   const vscpdatetime &to,
                                   vscpdatetime &first)
{
    std::string strLine;
    std::string strCmd;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    strCmd = "TABLE FIRSTDATE " + tblName + " ";
    strCmd += from.getISODateTime() + " ";
    strCmd += to.getISODateTime() + " ";
    strCmd += "\r\n";

    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    if (getInputQueueCount() < 2) return VSCP_ERROR_ERROR;
    strLine = m_inputStrArray[m_inputStrArray.size() - 2];

    if (!first.setISODate(strLine)) {
        return VSCP_ERROR_ERROR;
    }

    return VSCP_ERROR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
// tableGetLastDate
//

int
VscpRemoteTcpIf::tableGetLastDate(const std::string &tblName,
                                  const vscpdatetime &from,
                                  const vscpdatetime &to,
                                  vscpdatetime &last)
{
    std::string strLine;
    std::string strCmd;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    strCmd = "TABLE LASTDATE " + tblName + " ";
    strCmd += from.getISODateTime() + " ";
    strCmd += to.getISODateTime() + " ";
    strCmd += "\r\n";

    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    if (getInputQueueCount() < 2) return VSCP_ERROR_ERROR;
    strLine = m_inputStrArray[m_inputStrArray.size() - 2];

    if (!last.setISODate(strLine)) {
        return VSCP_ERROR_ERROR;
    }

    return VSCP_ERROR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
// tableGetSum
//

int
VscpRemoteTcpIf::tableGetSum(const std::string &tblName,
                             const vscpdatetime &from,
                             const vscpdatetime &to,
                             double *pSum)
{
    std::string strLine;
    std::string strCmd;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.
    if (NULL == pSum) return VSCP_ERROR_PARAMETER;

    strCmd = "TABLE SUM " + tblName + " ";
    strCmd += from.getISODateTime() + " ";
    strCmd += to.getISODateTime() + " ";
    strCmd += "\r\n";

    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    if (getInputQueueCount() < 2) return VSCP_ERROR_ERROR;
    strLine = m_inputStrArray[m_inputStrArray.size() - 2];

    *pSum = stod(strLine);

    return VSCP_ERROR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
// tableGetMin
//

int
VscpRemoteTcpIf::tableGetMin(const std::string &tblName,
                             const vscpdatetime &from,
                             const vscpdatetime &to,
                             double *pMin)
{
    std::string strLine;
    std::string strCmd;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.
    if (NULL == pMin) return VSCP_ERROR_PARAMETER;

    strCmd = "TABLE MIN " + tblName + " ";
    strCmd += from.getISODateTime() + " ";
    strCmd += to.getISODateTime() + " ";
    strCmd += "\r\n";

    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    if (getInputQueueCount() < 2) return VSCP_ERROR_ERROR;
    strLine = m_inputStrArray[m_inputStrArray.size() - 2];

    *pMin = stod(strLine);

    return VSCP_ERROR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
// tableGetMax
//

int
VscpRemoteTcpIf::tableGetMax(const std::string &tblName,
                             const vscpdatetime &from,
                             const vscpdatetime &to,
                             double *pMax)
{
    std::string strLine;
    std::string strCmd;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.
    if (NULL == pMax) return VSCP_ERROR_PARAMETER;

    strCmd = "TABLE MAX " + tblName + " ";
    strCmd += from.getISODateTime() + " ";
    strCmd += to.getISODateTime() + " ";
    strCmd += "\r\n";

    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    if (getInputQueueCount() < 2) return VSCP_ERROR_ERROR;
    strLine = m_inputStrArray[m_inputStrArray.size() - 2];

    *pMax = stod(strLine);

    return VSCP_ERROR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
// tableGetAverage
//

int
VscpRemoteTcpIf::tableGetAverage(const std::string &tblName,
                                 const vscpdatetime &from,
                                 const vscpdatetime &to,
                                 double *pAverage)
{
    std::string strLine;
    std::string strCmd;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.
    if (NULL == pAverage) return VSCP_ERROR_PARAMETER;

    strCmd = "TABLE AVERAGE " + tblName + " ";
    strCmd += from.getISODateTime() + " ";
    strCmd += to.getISODateTime() + " ";
    strCmd += "\r\n";

    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    if (getInputQueueCount() < 2) return VSCP_ERROR_ERROR;
    strLine = m_inputStrArray[m_inputStrArray.size() - 2];

    *pAverage = stod(strLine);

    return VSCP_ERROR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
// tableGetMedian
//

int
VscpRemoteTcpIf::tableGetMedian(const std::string &tblName,
                                const vscpdatetime &from,
                                const vscpdatetime &to,
                                double *pMedian)
{
    std::string strLine;
    std::string strCmd;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.
    if (NULL == pMedian) return VSCP_ERROR_PARAMETER;

    strCmd = "TABLE MEDIAN " + tblName + " ";
    strCmd += from.getISODateTime() + " ";
    strCmd += to.getISODateTime() + " ";
    strCmd += "\r\n";

    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    if (getInputQueueCount() < 2) return VSCP_ERROR_ERROR;
    strLine = m_inputStrArray[m_inputStrArray.size() - 2];

    *pMedian = stod(strLine);

    return VSCP_ERROR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
// tableGetStdDev
//

int
VscpRemoteTcpIf::tableGetStdDev(const std::string &tblName,
                                const vscpdatetime &from,
                                const vscpdatetime &to,
                                double *pStdDev)
{
    std::string strLine;
    std::string strCmd;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.
    if (NULL == pStdDev) return VSCP_ERROR_PARAMETER;

    strCmd = "TABLE STDDEV " + tblName + " ";
    strCmd += from.getISODateTime() + " ";
    strCmd += to.getISODateTime() + " ";
    strCmd += "\r\n";

    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    if (getInputQueueCount() < 2) return VSCP_ERROR_ERROR;
    strLine = m_inputStrArray[m_inputStrArray.size() - 2];

    *pStdDev = stod(strLine);

    return VSCP_ERROR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
// tableGetVarianve
//

int
VscpRemoteTcpIf::tableGetVariance(const std::string &tblName,
                                  const vscpdatetime &from,
                                  const vscpdatetime &to,
                                  double *pVariance)
{
    std::string strLine;
    std::string strCmd;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.
    if (NULL == pVariance) return VSCP_ERROR_PARAMETER;

    strCmd = "TABLE VARIANCE " + tblName + " ";
    strCmd += from.getISODateTime() + " ";
    strCmd += to.getISODateTime() + " ";
    strCmd += "\r\n";

    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    if (getInputQueueCount() < 2) return VSCP_ERROR_ERROR;
    strLine = m_inputStrArray[m_inputStrArray.size() - 2];

    *pVariance = stod(strLine);

    return VSCP_ERROR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
// tableGetMode
//

int
VscpRemoteTcpIf::tableGetMode(const std::string &tblName,
                              const vscpdatetime &from,
                              const vscpdatetime &to,
                              double *pMode)
{
    std::string strLine;
    std::string strCmd;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.
    if (NULL == pMode) return VSCP_ERROR_PARAMETER;

    strCmd = "TABLE MODE " + tblName + " ";
    strCmd += from.getISODateTime() + " ";
    strCmd += to.getISODateTime() + " ";
    strCmd += "\r\n";

    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    if (getInputQueueCount() < 2) return VSCP_ERROR_ERROR;
    strLine = m_inputStrArray[m_inputStrArray.size() - 2];

    *pMode = stod(strLine);

    return VSCP_ERROR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
// tableGetLowerQ
//

int
VscpRemoteTcpIf::tableGetLowerQ(const std::string &tblName,
                                const vscpdatetime &from,
                                const vscpdatetime &to,
                                double *pLowerq)
{
    std::string strLine;
    std::string strCmd;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.
    if (NULL == pLowerq) return VSCP_ERROR_PARAMETER;

    strCmd = "TABLE LOWERQ " + tblName + " ";
    strCmd += from.getISODateTime() + " ";
    strCmd += to.getISODateTime() + " ";
    strCmd += "\r\n";

    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    if (getInputQueueCount() < 2) return VSCP_ERROR_ERROR;
    strLine = m_inputStrArray[m_inputStrArray.size() - 2];

    *pLowerq = stod(strLine);

    return VSCP_ERROR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
// tableGetUpperQ
//

int
VscpRemoteTcpIf::tableGetUpperQ(const std::string &tblName,
                                const vscpdatetime &from,
                                const vscpdatetime &to,
                                double *pUpperq)
{
    std::string strLine;
    std::string strCmd;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.
    if (NULL == pUpperq) return VSCP_ERROR_PARAMETER;

    strCmd = "TABLE UPPERQ " + tblName + " ";
    strCmd += from.getISODateTime() + " ";
    strCmd += to.getISODateTime() + " ";
    strCmd += "\r\n";

    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    if (getInputQueueCount() < 2) return VSCP_ERROR_ERROR;
    strLine = m_inputStrArray[m_inputStrArray.size() - 2];

    *pUpperq = stod(strLine);

    return VSCP_ERROR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
// tableClear
//

int
VscpRemoteTcpIf::tableClear(const std::string &tblName,
                            const vscpdatetime &from,
                            const vscpdatetime &to)
{
    std::string strCmd;

    if (!isConnected()) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    strCmd = "TABLE CLEAR " + tblName + " ";
    strCmd += from.getISODateTime() + " ";
    strCmd += to.getISODateTime() + " ";
    strCmd += "\r\n";

    if (VSCP_ERROR_SUCCESS != doCommand(strCmd)) {
        return VSCP_ERROR_ERROR;
    }

    return VSCP_ERROR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
//                       R E G I S T E R   H E L P E R S
////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// readLevel2Register
//

int
VscpRemoteTcpIf::readLevel2Register(uint32_t reg,
                                    uint16_t page,
                                    uint8_t *pval,
                                    cguid &ifGUID,
                                    cguid *pdestGUID,
                                    bool bLevel2)
{
    int rv = VSCP_ERROR_SUCCESS;
    std::string strBuf;
    vscpEventEx e;

    // Check pointers
    if (NULL == pval) return false;
    if (NULL == pdestGUID) return false;

    e.head      = VSCP_PRIORITY_NORMAL;
    e.timestamp = 0;
    e.obid      = 0;

    // Check if a specific interface is used
    if (!ifGUID.isNULL()) {

        // Event should be sent to a specific interface
        e.vscp_class = VSCP_CLASS2_LEVEL1_PROTOCOL;
        e.vscp_type  = VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_READ;

        memset(e.GUID, 0, 16); // We use GUID for interface
        e.sizeData = 16 + 5;   // Interface GUID + nodeid + register to read

        ifGUID.writeGUID(e.data);

        e.data[16] = pdestGUID->getLSB(); // nodeid
        e.data[17] = page >> 8;           // Page MSB
        e.data[18] = page & 0xff;         // Page LSB
        e.data[19] = reg;                 // Register to read
        e.data[20] = 1;                   // Number of registers to read

    } else {

        // Event should be sent to all interfaces

        if (bLevel2) {

            // True real Level II event

            e.head       = VSCP_PRIORITY_NORMAL;
            e.vscp_class = VSCP_CLASS2_PROTOCOL;
            e.vscp_type  = VSCP2_TYPE_PROTOCOL_READ_REGISTER;

            memset(e.GUID, 0, 16); // We use GUID for interface

            e.sizeData = 22; // nodeid + register to read

            pdestGUID->writeGUID(e.data);    // Destination GUID
            e.data[16] = (reg >> 24) & 0xff; // Register to read
            e.data[17] = (reg >> 16) & 0xff;
            e.data[18] = (reg >> 8) & 0xff;
            e.data[19] = reg & 0xff;
            e.data[20] = 0x00; // Read one register
            e.data[21] = 0x01;

        } else {

            // Level I over CLASS2 to all interfaces
            e.head       = VSCP_PRIORITY_NORMAL;
            e.vscp_class = VSCP_CLASS2_LEVEL1_PROTOCOL;
            e.vscp_type  = VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_READ;

            memset(e.GUID, 0, 16);        // We use GUID for interface
            e.sizeData = 16 + 5;          // nodeid + register to read
            pdestGUID->writeGUID(e.data); // Destination GUID

            e.data[16] = pdestGUID->getLSB(); // nodeid
            e.data[17] = page >> 8;           // Page MSB
            e.data[18] = page & 0xff;         // Page LSB
            e.data[19] = reg;                 // Register to read
            e.data[20] = 1;                   // Number of registers to read
        }
    }

    // Send the event
    doCmdClear();
    e.timestamp = 0;
    doCmdSendEx(&e);

    uint32_t resendTime = m_registerOpResendTimeout;
    uint32_t startTime  = vscp_getMsTimeStamp();

    while (true) {

        if (0 < doCmdDataAvailable()) { // Message available

            if (VSCP_ERROR_SUCCESS == doCmdReceiveEx(&e)) { // Valid event

                // Check for correct reply event
#ifdef DEBUG_LIB_VSCP_HELPER
                {
                    std::string str;
                    str = vscp_str_format(
                      "Received Event: class=%d type=%d size=%d index=%d "
                      "page=%d Register=%d content=%d",
                      e.vscp_class,
                      e.vscp_type,
                      e.sizeData,
                      e.data[16],                     // frame index
                      (e.data[17] << 8) + e.data[18], // page
                      e.data[19],                     // register
                      e.data[20]);                    // content
                    VSCP_LOG_DEBUG(str);
                }
#endif

                // Level I Read reply?
                if ((VSCP_CLASS1_PROTOCOL == e.vscp_class) &&
                    (VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE ==
                     e.vscp_type)) {

                    if ((0 == e.data[0]) && // Frame index is first?
                        ((page >> 8) == e.data[1]) &&
                        ((page & 0x0ff) == e.data[2]) &&
                        (reg == e.data[3])) { // Requested register?

                        if (pdestGUID->isSameGUID(
                              e.GUID)) { // From correct node?
                            if (NULL != pval) {
                                *pval = e.data[4];
                            }
                            break;
                        }

                    } // Check for correct node
                }

                // Level II 512 Read reply?
                else if (!ifGUID.isNULL() && !bLevel2 &&
                         (VSCP_CLASS2_LEVEL1_PROTOCOL == e.vscp_class) &&
                         (VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE ==
                          e.vscp_type)) {

                    if (pdestGUID->isSameGUID(e.GUID)) {

                        if ((0 == e.data[16]) && // Frame index is first?
                            ((page >> 8) == e.data[17]) &&
                            ((page & 0x0ff) == e.data[18]) &&
                            (reg == e.data[19])) { // Requested register?

                            // OK get the data
                            if (NULL != pval) {
                                *pval = e.data[20];
                                break;
                            }
                        }
                    }

                }
                // Level II Read reply?
                else if (ifGUID.isNULL() && bLevel2 &&
                         (VSCP_CLASS2_PROTOCOL == e.vscp_class) &&
                         (VSCP2_TYPE_PROTOCOL_READ_WRITE_RESPONSE ==
                          e.vscp_type)) {

                    // from us
                    if (pdestGUID->isSameGUID(e.GUID)) {

                        uint32_t retreg = (e.data[0] << 24) +
                                          (e.data[1] << 16) + (e.data[2] << 8) +
                                          e.data[3];

                        // Reg we requested?
                        if (retreg == reg) {

                            // OK get the data
                            if (NULL != pval) {
                                *pval = e.data[4];
                                break;
                            }
                        }
                    }
                }

            } // valid event
        }

        if ((vscp_getMsTimeStamp() - startTime) > m_registerOpErrorTimeout) {
            rv = CANAL_ERROR_TIMEOUT;
            break;
        } else if ((vscp_getMsTimeStamp() - startTime) > resendTime) {
            // Send again
            e.timestamp = 0;
            doCmdSendEx(&e);
            resendTime += m_registerOpResendTimeout;
        }

        usleep(2000);

    } // while

    return rv;
}

//////////////////////////////////////////////////////////////////////////////
// readLevel2Registers
//

int
VscpRemoteTcpIf::readLevel2Registers(uint32_t reg,
                                     uint16_t page,
                                     uint8_t count,
                                     uint8_t *pval,
                                     cguid &ifGUID,
                                     cguid *pdestGUID,
                                     bool bLevel2)
{
    int rv = CANAL_ERROR_SUCCESS;
    std::string strBuf;
    vscpEventEx e;
    uint8_t data[256]; // This makes range checking simpler

    // Max 128 bytes can be read in one go
    if (count > 128) return CANAL_ERROR_PARAMETER;

    // Check pointers
    if (NULL == pval) return CANAL_ERROR_PARAMETER;
    if (NULL == pdestGUID) return CANAL_ERROR_PARAMETER;

    e.head      = VSCP_PRIORITY_NORMAL;
    e.timestamp = 0;
    e.obid      = 0;

    // Check if a specific interface is used
    if (!ifGUID.isNULL()) {

        // Event should be sent to a specific interface
        e.vscp_class = VSCP_CLASS2_LEVEL1_PROTOCOL;
        e.vscp_type  = VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_READ;

        memset(e.GUID, 0, 16); // We use GUID for interface
        e.sizeData = 16 + 5;   // Interface GUID + nodeid + register to read

        ifGUID.writeGUID(e.data);

        e.data[16] = pdestGUID->getLSB(); // nodeid
        e.data[17] = page >> 8;           // Page MSB
        e.data[18] = page & 0xff;         // Page LSB
        e.data[19] = reg;                 // Register to read
        e.data[20] = count;               // Read count registers

    } else {

        // Event should be sent to all interfaces

        if (bLevel2) {

            // True real Level II event

            e.head       = VSCP_PRIORITY_NORMAL;
            e.vscp_class = VSCP_CLASS2_PROTOCOL;
            e.vscp_type  = VSCP2_TYPE_PROTOCOL_READ_REGISTER;

            memset(e.GUID, 0, 16); // We use GUID for interface

            e.sizeData = 22; // nodeid + register to read

            pdestGUID->writeGUID(e.data);    // Destination GUID
            e.data[16] = (reg >> 24) & 0xff; // Register to read
            e.data[17] = (reg >> 16) & 0xff;
            e.data[18] = (reg >> 8) & 0xff;
            e.data[19] = reg & 0xff;
            e.data[20] = 0; // Read count registers
            e.data[21] = count;

        } else {

            // Level I over CLASS2 to all interfaces
            e.head       = VSCP_PRIORITY_NORMAL;
            e.vscp_class = VSCP_CLASS2_LEVEL1_PROTOCOL;
            e.vscp_type  = VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_READ;

            memset(e.GUID, 0, 16);        // We use GUID for interface
            e.sizeData = 16 + 5;          // nodeid + register to read
            pdestGUID->writeGUID(e.data); // Destination GUID

            e.data[16] = pdestGUID->getLSB(); // nodeid
            e.data[17] = page >> 8;           // Page MSB
            e.data[18] = page & 0xff;         // Page LSB
            e.data[19] = reg;                 // Register to read
            e.data[20] = count;               // Read count registers
        }
    }

    // Send the event
    doCmdClear();
    e.timestamp = 0;
    doCmdSendEx(&e);

    // We should get 32 response frames back
    // a bit is set in response_flags for each frame
    unsigned long receive_flags = 0;
    unsigned char nPages        = count / 4;
    unsigned char lastpageCnt   = count % 4;
    if (lastpageCnt) nPages++;
    unsigned long allRcvValue = pow(2.0, nPages) - 1;

    unsigned long resendTime = m_registerOpResendTimeout * (1 + nPages);
    uint32_t startTime       = vscp_getMsTimeStamp();

    /*
    #ifdef DEBUG_LIB_VSCP_HELPER
        VSCP_LOG_DEBUG("readLevel2Registers: Start");
    #endif

        for ( int i=0; i<100; i++ ) {
            doCmdNOOP();
        }
    #ifdef DEBUG_LIB_VSCP_HELPER
        VSCP_LOG_DEBUG("readLevel2Registers: NOOP - End");
    #endif
    */

    while (allRcvValue !=
           (receive_flags &
            0xffffffff)) { // mask for systems where long is > 32 bits

        int n;
        if (0 < (n = doCmdDataAvailable())) { // Message available

            while (CANAL_ERROR_SUCCESS == doCmdReceiveEx(&e)) { // Valid event

                // Quit if done?
                if ((allRcvValue == (receive_flags & 0xffffffff))) break;

                    // Check for correct reply event
#ifdef DEBUG_LIB_VSCP_HELPER
                {
                    std::string str;
                    str = vscp_str_format(
                      "Received Event: count=%d class=%d type=%d size=%d data=",
                      n,
                      e.vscp_class,
                      e.vscp_type,
                      e.sizeData,
                      e.data[16],                     // frame index
                      (e.data[17] << 8) + e.data[18], // page
                      e.data[19],                     // register
                      e.data[20]);                    // content
                    for (int i = 0; i < e.sizeData; i++) {
                        str += vscp_str_format("%02X ", e.data[i]);
                    }
                    VSCP_LOG_DEBUG(str);
                }
#endif

                // Level I Read reply?
                if ((VSCP_CLASS1_PROTOCOL == e.vscp_class) &&
                    (VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE ==
                     e.vscp_type)) {

                    if (pdestGUID->isSameGUID(e.GUID)) { // From correct node?

                        if (((page >> 8) == e.data[1]) &&
                            ((page & 0x0ff) == e.data[2])) { // Requested page?

                            // Mark frame as received
                            receive_flags |= (1 << e.data[0]);

                            if (lastpageCnt && (count - 1) == e.data[0]) {
                                // Last frame
                                for (int i = 0; i < lastpageCnt; i++) {
                                    data[e.data[0] * 4 + i] = e.data[4 + i];
                                }
                            } else {
                                for (int i = 0; i < 4; i++) {
                                    data[e.data[0] * 4 + i] = e.data[4 + i];
                                }
                            }
                        }

                    } // Check for response from correct node

                }

                // Level II 512 Read reply?
                else if (!ifGUID.isNULL() && !bLevel2 &&
                         (VSCP_CLASS2_LEVEL1_PROTOCOL == e.vscp_class) &&
                         (VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE ==
                          e.vscp_type)) {

                    if (pdestGUID->isSameGUID(e.GUID)) {

                        if (((page >> 8) == e.data[17]) &&
                            ((page & 0x0ff) ==
                             e.data[18])) { // Requested register?

                            // Mark frame as received
                            receive_flags |= (1 << e.data[16]);

                            if (lastpageCnt && (count - 1) == e.data[16]) {
                                // Last frame
                                for (int i = 0; i < lastpageCnt; i++) {
                                    data[e.data[16] * 4 + i] =
                                      e.data[16 + 4 + i];
                                }
                            } else {
                                for (int i = 0; i < 4; i++) {
                                    data[e.data[16] * 4 + i] =
                                      e.data[16 + 4 + i];
                                }
                            }
                        }

                    } // Check for response from correct node

                }

                // Level II Read reply?
                else if (ifGUID.isNULL() && bLevel2 &&
                         (VSCP_CLASS2_PROTOCOL == e.vscp_class) &&
                         (VSCP2_TYPE_PROTOCOL_READ_WRITE_RESPONSE ==
                          e.vscp_type)) {

                    // from us
                    if (pdestGUID->isSameGUID(e.GUID)) {

                        uint32_t retreg = (e.data[0] << 24) +
                                          (e.data[1] << 16) + (e.data[2] << 8) +
                                          e.data[3];

                        // Reg we requested?
                        if (retreg == reg) {

                            // OK get the data
                            if (NULL != pval) {
                                *pval = e.data[4];
                                break;
                            }
                        }
                    }
                }

            } // valid event

        } // Data available

        if ((vscp_getMsTimeStamp() - startTime) > m_registerOpErrorTimeout) {
            rv = CANAL_ERROR_TIMEOUT;
#ifdef DEBUG_LIB_VSCP_HELPER
            VSCP_LOG_DEBUG("readLevel2Registers: Timeout");
#endif
            break;
        } else if ((vscp_getMsTimeStamp() - startTime) > resendTime) {
            // Send again
            doCmdSendEx(&e);
            resendTime += m_registerOpResendTimeout * (1 + nPages);
#ifdef DEBUG_LIB_VSCP_HELPER
            VSCP_LOG_DEBUG("readLevel2Registers: Resend ");
#endif
        }

        usleep(2000);

    } // while

#ifdef DEBUG_LIB_VSCP_HELPER
    VSCP_LOG_DEBUG("readLevel2Registers: End");
#endif

    // Copy data on success
    if (CANAL_ERROR_SUCCESS == rv) {
        memcpy(pval, data, count);
    }

    return rv;
}

//////////////////////////////////////////////////////////////////////////////
// writeLevel2Register
//

int
VscpRemoteTcpIf::writeLevel2Register(uint32_t reg,
                                     uint16_t page,
                                     uint8_t *pval,
                                     cguid &ifGUID,
                                     cguid *pdestGUID,
                                     bool bLevel2)
{
    int rv          = CANAL_ERROR_SUCCESS;
    bool bInterface = false; // No specific interface set
    std::string strBuf;
    vscpEventEx e;

    // Check pointers
    if (NULL == pval) return CANAL_ERROR_PARAMETER;

    if (!ifGUID.isNULL()) {

        e.head       = VSCP_PRIORITY_NORMAL;
        e.vscp_class = VSCP_CLASS2_LEVEL1_PROTOCOL;
        e.vscp_type  = VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_WRITE;

        memset(e.GUID, 0, 16); // We use interface GUID

        e.sizeData = 16 + 5;

        pdestGUID->writeGUID(e.data);     // Destination GUID
        e.data[16] = pdestGUID->getLSB(); // nodeid
        e.data[17] = (page >> 8);
        e.data[18] = (page & 0xff);
        e.data[19] = reg;   // Register to write
        e.data[20] = *pval; // value to write

    } else {

        if (bLevel2) {

            e.head       = VSCP_PRIORITY_NORMAL;
            e.vscp_class = VSCP_CLASS2_PROTOCOL;
            e.vscp_type  = VSCP2_TYPE_PROTOCOL_WRITE_REGISTER;

            memset(e.GUID, 0, 16);        // We use interface GUID
            e.sizeData = 21;              // nodeid + register to read
            pdestGUID->writeGUID(e.data); // Destination GUID

            e.data[16] = (reg >> 24) & 0xff; // Register to write
            e.data[17] = (reg >> 16) & 0xff;
            e.data[18] = (reg >> 8) & 0xff;
            e.data[19] = reg & 0xff;
            e.data[20] = *pval; // Data to write

        } else {

            // Level I over CLASS2 to all interfaces

            e.head       = VSCP_PRIORITY_NORMAL;
            e.vscp_class = VSCP_CLASS2_LEVEL1_PROTOCOL;
            e.vscp_type  = VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_WRITE;

            memset(e.GUID, 0, 16); // We use interface GUID
            e.sizeData = 16 + 5;
            pdestGUID->writeGUID(e.data); // Destination GUID

            e.data[16] = pdestGUID->getLSB(); // nodeid
            e.data[17] = page >> 8;           // Page MSB
            e.data[18] = page & 0xff;         // Page LSB
            e.data[19] = reg;                 // Register to write
            e.data[20] = *pval;               // value to write
        }
    }

    // Send the event
    doCmdSendEx(&e);

    unsigned long resendTime = m_registerOpResendTimeout;
    uint32_t startTime       = vscp_getMsTimeStamp();

    while (true) {

        int n;
        if (0 < (n = doCmdDataAvailable())) { // Message available

            if (CANAL_ERROR_SUCCESS == doCmdReceiveEx(&e)) { // Valid event

                if ((VSCP_CLASS1_PROTOCOL == e.vscp_class) &&
                    (VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE ==
                     e.vscp_type)) { // Read reply?

                    if ((0 == e.data[0]) && // Frame index is first frame
                        ((page >> 8) ==
                         e.data[1]) && // Page is same as requested
                        ((page & 0xff) == e.data[2]) &&
                        (reg == e.data[3])) { // Requested register?

                        if (pdestGUID->isSameGUID(
                              e.GUID)) { // From correct node?

                            // We got a rw reply from the correct node. Is
                            // the written data same as we expect?
                            if (*pval != e.data[4]) rv = CANAL_ERROR_REGISTER;
                            break;
                        }
                    }
                }

                // Level II 512 Read reply?
                else if (!ifGUID.isNULL() && !bLevel2 &&
                         (VSCP_CLASS2_LEVEL1_PROTOCOL == e.vscp_class) &&
                         (VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE ==
                          e.vscp_type)) {

                    if (pdestGUID->getLSB() == e.data[15]) {

                        if ((0 == e.data[16]) && // Frame index is first frame
                            ((page >> 8) ==
                             e.data[17]) && // Page is same as requested
                            ((page & 0xff) == e.data[18]) &&
                            (reg == e.data[19])) { // Reg we requested?

                            // We got a rw reply from the correct node. Is
                            // the written data same as we expect?
                            if (*pval != e.data[17]) rv = CANAL_ERROR_REGISTER;
                            break;
                        }
                    }

                }
                // Level II Read reply?
                else if (!bInterface && bLevel2 &&
                         (VSCP_CLASS2_PROTOCOL == e.vscp_class) &&
                         (VSCP2_TYPE_PROTOCOL_READ_WRITE_RESPONSE ==
                          e.vscp_type)) {

                    // from us
                    if (pdestGUID->isSameGUID(e.GUID)) {

                        uint32_t retreg = (e.data[0] << 24) +
                                          (e.data[1] << 16) + (e.data[2] << 8) +
                                          e.data[3];

                        // Reg we requested?
                        if (retreg == reg) {

                            // OK get the data
                            if (NULL != pval) {
                                // We go a rw reply from the correct node is
                                // the written data same as we expect.
                                if (*pval != e.data[4])
                                    rv = CANAL_ERROR_REGISTER;
                                break;
                            }
                        }
                    }
                }
            }
        } else {

            usleep(2000);
        }

        if ((vscp_getMsTimeStamp() - startTime) > m_registerOpErrorTimeout) {
            rv = CANAL_ERROR_TIMEOUT;
            break;
        } else if ((vscp_getMsTimeStamp() - startTime) > resendTime) {
            // Send again
            doCmdSendEx(&e);
            resendTime += m_registerOpResendTimeout;
        }

    } // while

    return rv;
}

//////////////////////////////////////////////////////////////////////////////
// getMDFUrlFromLevel2Device
//

bool
VscpRemoteTcpIf::getMDFUrlFromLevel2Device(cguid &ifGUID,
                                           cguid &destGUID,
                                           std::string &strurl,
                                           bool bLevel2)
{
    char url[33];
    bool rv = true;

    memset(url, 0, sizeof(url));

    if (bLevel2) {

        // Level 2 device
        uint8_t *p = (uint8_t *)url;
        for (int i = 0; i < 32; i++) {

            if (CANAL_ERROR_SUCCESS !=
                readLevel2Register(0xFFFFFFE0 + i,
                                   0,   // Page = 0
                                   p++, // read value
                                   ifGUID,
                                   &destGUID,
                                   true)) { // Yes Level II
                rv = false;
                goto error;
            }
        }

    } else {

        // Level 1 device
        uint8_t *p = (uint8_t *)url;
        for (int i = 0; i < 32; i++) {

            if (CANAL_ERROR_SUCCESS != readLevel2Register(0xE0 + i,
                                                          0,   // Page = 0
                                                          p++, // read value
                                                          ifGUID,
                                                          &destGUID)) {
                rv = false;
                goto error;
            }
        }
    }

    strurl = std::string(url);
    if (strurl.npos == strurl.find("http://")) {
        std::string str;
        str = "http://";
        str += strurl;
        strurl = str;
    }

error:

    if (!rv) strurl.clear();

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
//                           Graphical helpers
///////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// fetchIterfaceGUID
//

int
VscpRemoteTcpIf::fetchIterfaceGUID(const std::string &ifName, cguid &guid)
{
    std::string str;
    std::deque<std::string> ifarray;
    uint8_t cnt = 0;

    if (!isConnected()) {
        return VSCP_ERROR_NOT_OPEN;
    }

    // Get the interface list
    while (true) {

        if (VSCP_ERROR_SUCCESS == doCmdInterfaceList(ifarray)) {

            if (ifarray.size()) {

                cnt++;

                for (unsigned int i = 0; i < ifarray.size(); i++) {

                    std::deque<std::string> tokens;
                    vscp_split(tokens, ifarray[i], ",");

                    std::string strOrdinal;
                    if (!tokens.empty()) {
                        strOrdinal = tokens.front();
                        tokens.pop_front();
                    }

                    std::string strType;
                    if (!tokens.empty()) {
                        strType = tokens.front();
                        tokens.pop_front();
                    }

                    std::string strIfGUID;
                    if (!tokens.empty()) {
                        strIfGUID = tokens.front();
                        tokens.pop_front();
                    }

                    std::string strDescription;
                    if (!tokens.empty()) {
                        strDescription = tokens.front();
                        tokens.pop_front();
                    }

                    int pos;
                    std::string strName;
                    if (std::string::npos != (pos = strDescription.find("|"))) {
                        strName = strDescription.substr(0, pos);
                        vscp_trim(strName);
                    }

                    if (0 == vscp_strcasecmp(strName.c_str(), ifName.c_str())) {

                        // Save interface GUID;
                        guid.getFromString(strIfGUID);
                        return VSCP_ERROR_SUCCESS;
                    }
                }
            }
        }

        // Give up after tree tries
        if (cnt > 3) break;
    }

    return VSCP_ERROR_OPERATION_FAILED;
}

////////////////////////////////////////////////////////////////////////////////
//                         T H R E A D   H E L P E R S
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Constructor
//

ctrlObjVscpTcpIf::ctrlObjVscpTcpIf()
{
    m_strUsername    = "admin";
    m_strPassword    = "secret";
    m_strHost        = "localhost";
    m_port           = 9598; // VSCP_LEVEL2_TCP_PORT;
    m_rxState        = RX_TREAD_STATE_NONE;
    m_bQuit          = false; // Don't even think of quiting yet...
    m_rxChannelID    = 0;     // No receive channel
    m_txChannelID    = 0;     // No transmit channel
    m_bFilterOwnTx   = false; // Don't filter TX
    m_bUseRXTXEvents = false; // No events
    m_wndID          = 0;     // No message window id
    m_maxRXqueue     = MAX_TREAD_RECEIVE_EVENTS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Destructor
//

ctrlObjVscpTcpIf::~ctrlObjVscpTcpIf() {}

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// remotetcpipRxObj
//

remotetcpipRxObj::remotetcpipRxObj()
{
    m_pCtrlObject = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// remotetcpipRxObj
//

remotetcpipRxObj::~remotetcpipRxObj()
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
// remotetcpipRxThread
//
// Is there any messages to send from Level II clients. Send it/them to all
// devices/clients except for itself.
//

/*
void *remotetcpipRxThread(void *pDate)
{
    int rv;
    VscpRemoteTcpIf tcpifReceive; // TODO

    // Must be a valid control object pointer
    if ( NULL == m_pCtrlObject ) return NULL;

    //xxCommandEvent eventReceive( xxVSCPTCPIF_RX_EVENT, m_pCtrlObject->m_wndID
);
    //xxCommandEvent eventConnectionLost( xxVSCPTCPIF_CONNECTION_LOST_EVENT,
m_pCtrlObject->m_wndID );

    // Connect to the server with the control interface
    if ( VSCP_ERROR_SUCCESS !=
        tcpifReceive.doCmdOpen( m_pCtrlObject->m_strHost,
                                    m_pCtrlObject->m_strUsername,
                                    m_pCtrlObject->m_strPassword ) ) {
        if ( m_pCtrlObject->m_bUseRXTXEvents ) {
            // TODO if ( NULL != m_pCtrlObject->m_pWnd ) xxPostEvent(
m_pCtrlObject->m_pWnd, eventConnectionLost );
        }

        m_pCtrlObject->m_rxState = RX_TREAD_STATE_FAIL_DISCONNECTED;

        return NULL;

    }

    m_pCtrlObject->m_rxState = RX_TREAD_STATE_CONNECTED;

    // Find the channel id
    tcpifReceive.doCmdGetChannelID( &m_pCtrlObject->m_rxChannelID );

    // Start Receive Loop
    tcpifReceive.doCmdEnterReceiveLoop();


    while ( !TestDestroy() && !m_pCtrlObject->m_bQuit ) {

        vscpEvent *pEvent = new vscpEvent;
        if ( NULL == pEvent ) break;

        if ( VSCP_ERROR_SUCCESS ==
            ( rv = tcpifReceive.doCmdBlockingReceive( pEvent ) ) ) {

            if ( m_pCtrlObject->m_bFilterOwnTx && ( m_pCtrlObject->m_txChannelID
== pEvent->obid ) )  { vscp_deleteVSCPevent( pEvent ); continue;
            }

            if ( m_pCtrlObject->m_bUseRXTXEvents ) {
                //eventReceive.SetClientData( pEvent );
                //if ( NULL != m_pCtrlObject->m_pWnd ) xxPostEvent(
m_pCtrlObject->m_pWnd, eventReceive );
            }
            else {
                if ( m_pCtrlObject->m_rxQueue.GetCount() <=
m_pCtrlObject->m_maxRXqueue ) {
                    // Add the event to the in queue
                    m_pCtrlObject->m_mutexRxQueue.Lock();
                    m_pCtrlObject->m_rxQueue.Append( pEvent );
                    m_pCtrlObject->m_semRxQueue.Post();
                    m_pCtrlObject->m_mutexRxQueue.Unlock();
                }
                else {
                    delete pEvent;
                }
            }
        }
        else {
            delete pEvent;
            if ( VSCP_ERROR_COMMUNICATION == rv ) {
                m_pCtrlObject->m_rxState = RX_TREAD_STATE_FAIL_DISCONNECTED;
                m_pCtrlObject->m_bQuit = true;
            }
        }
    } // while

    // Close the interface
    tcpifReceive.doCmdClose();

    if ( m_pCtrlObject->m_bUseRXTXEvents ) {
        // TODO if ( NULL != m_pCtrlObject->m_pWnd ) xxPostEvent(
m_pCtrlObject->m_pWnd, eventConnectionLost );
    }

    if ( m_pCtrlObject->m_rxState != RX_TREAD_STATE_FAIL_DISCONNECTED ) {
        m_pCtrlObject->m_rxState = RX_TREAD_STATE_DISCONNECTED;
    }

    return NULL;

}
*/

///////////////////////////////////////////////////////////////////////////////
// remotetcpipTxObj
//

remotetcpipTxObj::remotetcpipTxObj()
{
    m_pCtrlObject = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// remotetcpipTxObj
//

remotetcpipTxObj::~remotetcpipTxObj()
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
// remotetcpipTxThread
//
// Is there any messages to send from Level II clients. Send it/them to all
// devices/clients except for itself.
//

/* TODO future
void *remotetcpipTxThread( void *pDate )
{
    VscpRemoteTcpIf tcpifTransmit;

    // Must be a valid control object pointer
    if ( NULL == m_pCtrlObject ) return NULL;

    //xxCommandEvent eventConnectionLost( xxVSCPTCPIF_CONNECTION_LOST_EVENT,
m_pCtrlObject->m_wndID );

    // Connect to the server with the control interface
    if ( VSCP_ERROR_SUCCESS !=
        tcpifTransmit.doCmdOpen( m_pCtrlObject->m_strHost,
                                        m_pCtrlObject->m_strUsername,
                                        m_pCtrlObject->m_strPassword ) ) {
        if ( m_pCtrlObject->m_bUseRXTXEvents ) {
            // TODO if ( NULL != m_pCtrlObject->m_pWnd ) xxPostEvent(
m_pCtrlObject->m_pWnd, eventConnectionLost );
        }

        m_pCtrlObject->m_rxState = RX_TREAD_STATE_FAIL_DISCONNECTED;
        return NULL;
    }

    m_pCtrlObject->m_rxState = RX_TREAD_STATE_CONNECTED;

    // Find the channel id
    tcpifTransmit.doCmdGetChannelID( &m_pCtrlObject->m_txChannelID );

    EVENT_TX_QUEUE::compatibility_iterator node;
    vscpEvent *pEvent;

    while ( !TestDestroy() && !m_pCtrlObject->m_bQuit ) {

        if ( xxSEMA_TIMEOUT == m_pCtrlObject->m_semTxQueue.WaitTimeout( 500 ) )
continue; m_pCtrlObject->m_mutexTxQueue.Lock(); node =
m_pCtrlObject->m_txQueue.GetFirst(); pEvent = node->GetData();
        tcpifTransmit.doCmdSend( pEvent );
        m_pCtrlObject->m_mutexTxQueue.Unlock();

    } // while

    // Close the interface
    tcpifTransmit.doCmdClose();

    if ( m_pCtrlObject->m_bUseRXTXEvents ) {
        // TODO if ( NULL != m_pCtrlObject->m_pWnd ) xxPostEvent(
m_pCtrlObject->m_pWnd, eventConnectionLost );
    }

    if ( m_pCtrlObject->m_rxState != RX_TREAD_STATE_FAIL_DISCONNECTED ) {
        m_pCtrlObject->m_rxState = RX_TREAD_STATE_DISCONNECTED;
    }

    return NULL;

}
*/
