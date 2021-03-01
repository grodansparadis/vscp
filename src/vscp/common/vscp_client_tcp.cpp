// vscp_client_tcp.cpp
//
// tcp/ip client communication classes.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// Copyright:   © 2007-2021
// Ake Hedman, Grodans Paradis AB, <akhe@vscp.org>
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

#include "vscp_client_tcp.h"

void workerThread(vscpClientTcp *pObj);

///////////////////////////////////////////////////////////////////////////////
// C-tor
//

vscpClientTcp::vscpClientTcp() 
{
    m_type = CVscpClient::connType::TCPIP;
    m_pworkerthread = nullptr;
    
    // Default connect parameters
    m_strHostname = "tcp://localhost:9598";
    m_strUsername = "admin";
    m_strPassword = "secret";
    memset(m_interfaceGuid, 0, 16);
    m_bPolling = false;
    m_obid = 0;
}

///////////////////////////////////////////////////////////////////////////////
// D-tor
//

vscpClientTcp::~vscpClientTcp() 
{
    // Just to be sure
    disconnect();

    // Clear the input queue (if needed)
    // while (m_inputQue.size()) {
    //     vscpEvent *pev = m_inputQue.front();
    //     m_inputQue.pop_front();
    //     vscp_deleteEvent(pev);
    // } 
}


///////////////////////////////////////////////////////////////////////////////
// getConfigAsJson
//

std::string vscpClientTcp::getConfigAsJson(void) 
{
    std::string str;
    json j;

    j["host"] = m_strHostname;
    j["user"] = m_strUsername;
    j["password"] = m_strPassword;
    j["bpoll"] = m_bPolling;
    j["connection-timeout"] = 0;
    j["response-timeout"] = 0;
    //j["bfull-l2"] = xxx;
    vscp_writeGuidArrayToString(str, m_interfaceGuid);
    j["selected-interface"] = str;

    // TLS

    j["btls"] = m_bTLS;
    j["bverifypeer"] = m_bVerifyPeer;
    j["cafile"] = m_cafile;
    j["capath"] = m_capath;
    j["certfile"] = m_certfile;
    j["keyfile"] = m_keyfile;
    j["pwkeyfile"] = m_pwKeyfile;

    // Filter

    j["priority-filter"] = m_filter.filter_priority;
    j["priority-mask"] = m_filter.mask_priority;
    j["class-filter"] = m_filter.filter_class;
    j["class-mask"] = m_filter.mask_class;
    j["type-filter"] = m_filter.filter_type;
    j["type-mask"] = m_filter.mask_type;
    vscp_writeGuidArrayToString(str,m_filter.filter_GUID);
    j["guid-filter"] = str;
    vscp_writeGuidArrayToString(str,m_filter.mask_GUID);
    j["guid-mask"] = str;

    return j.dump()
    ;
}


///////////////////////////////////////////////////////////////////////////////
// initFromJson
//

bool vscpClientTcp::initFromJson(const std::string& config)
{
    json j;

    try {
        j = json::parse(config);

        if (j.contains("host")) {
            m_strHostname = j["host"].get<std::string>();
        }

        if (j.contains("user")) {
            m_strUsername = j["user"].get<std::string>();
        }

        if (j.contains("password")) {
            m_strPassword = j["password"].get<std::string>();
        }

        if (j.contains("bpoll")) {
            m_bPolling = j["bpoll"].get<bool>();
        }

        if (j.contains("connection-timeout")) {
            //m_bPolling = j["connection-timeout"].get<int>();
        }

        if (j.contains("response-timeout")) {
            //m_bPolling = j["response-timeout"].get<int>();
        }

        // if (j.contains("bfull-l2")) {
        //     xxx = j["bfull-l2"].get<bool>();
        // }

        if (j.contains("selected-interface")) {
            std::string str = j["selected-interface"].get<std::string>();
            vscp_getGuidFromStringToArray(m_interfaceGuid, str);
        }

        // TLS

        if (j.contains("btls")) {
            m_bTLS = j["btls"].get<bool>();
        }

        if (j.contains("bverifypeer")) {
            m_bVerifyPeer = j["bverifypeer"].get<bool>();
        }

        if (j.contains("cafile")) {
            m_cafile = j["cafile"].get<std::string>();
        }

        if (j.contains("capath")) {
            m_capath = j["capath"].get<std::string>();        
        }

        if (j.contains("certfile")) {
            m_certfile = j["certfile"].get<std::string>();
        }

        if (j.contains("keyfile")) {
            m_keyfile = j["keyfile"].get<std::string>();    
        }

        if (j.contains("pwkeyfile")) {
            m_pwKeyfile = j["pwkeyfile"].get<std::string>();
        }

        // Filter

        if (j.contains("priority-filter")) {
            m_filter.filter_priority = j["priority-filter"].get<int>();
        }

        if (j.contains("priority-mask")) {
            m_filter.mask_priority = j["priority-mask"].get<int>();
        }

        if (j.contains("class-filter")) {
            m_filter.filter_class = j["class-filter"].get<int>();
        }

        if (j.contains("class-mask")) {
            m_filter.mask_class = j["class-mask"].get<int>();
        }

        if (j.contains("type-filter")) {
            m_filter.filter_type = j["type-filter"].get<int>();
        }

        if (j.contains("type-mask")) {
            m_filter.mask_type = j["type-mask"].get<int>();
        }

        if (j.contains("guid-filter")) {
            std::string str = j["guid-filter"].get<std::string>();
            vscp_getGuidFromStringToArray(m_filter.filter_GUID, str);
        }

        if (j.contains("guid-mask")) {
            std::string str = j["guid-mask"].get<std::string>();
            vscp_getGuidFromStringToArray(m_filter.mask_GUID, str);
        }

    }
    catch (...) {
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// init
//

int vscpClientTcp::init(const std::string &strHostname,
                        const std::string &strUsername,
                        const std::string &strPassword,
                        bool bPolling)
{
    m_strHostname = strHostname;
    m_strUsername = strUsername;
    m_strPassword = strPassword;
    m_bPolling = bPolling;
    return VSCP_ERROR_SUCCESS;
}                

///////////////////////////////////////////////////////////////////////////////
// connect
//

int vscpClientTcp::connect(void) 
{
    int rv;

    if (m_bPolling) {
        return m_tcp.doCmdOpen(m_strHostname,
                                m_strUsername,
                                m_strPassword);
    }
    else {
        // Open main interface
        if ( VSCP_ERROR_SUCCESS != 
                    ( rv = m_tcp.doCmdOpen(m_strHostname,
                                            m_strUsername,
                                            m_strPassword ))) {
            return rv;                                       
        }

        // Get channel id
        // Received canm check obid in received events to see if they 
        // are sent from ourself and if so skip them
        m_tcp.doCmdGetChannelID(&m_obid);

        // Open receive interface
        if ( VSCP_ERROR_SUCCESS != 
                    ( rv = m_tcpReceive.doCmdOpen(m_strHostname,
                                                    m_strUsername,
                                                    m_strPassword ))) {
            m_tcp.doCmdClose();
            return rv;                                       
        }

        // Create receive worker thread
        m_bRun = true;
        m_pworkerthread = new std::thread(workerThread, this);
        m_bRun = true;
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// disconnect
//

int vscpClientTcp::disconnect(void)
{
    int rv;

    if (m_bPolling) {
        return m_tcp.doCmdClose();
    }
    else {

        if (nullptr != m_pworkerthread) {
            m_bRun = false;
            m_pworkerthread->join();
            m_pworkerthread = nullptr;
        }

        // Close receive connection
        if ( VSCP_ERROR_SUCCESS != 
                ( rv = m_tcpReceive.doCmdClose() ) ) {
            m_tcp.doCmdClose();        
            return rv;   
        }

        // Close main connection
        return m_tcp.doCmdClose();
    }
}

///////////////////////////////////////////////////////////////////////////////
// isConnected
//

bool vscpClientTcp::isConnected(void)
{
    if (m_bPolling) {
        return m_tcp.isConnected();
    }
    else {
        return (m_tcp.isConnected() && m_tcpReceive.isConnected());
    }
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int vscpClientTcp::send(vscpEvent &ev)
{
    return m_tcp.doCmdSend(&ev);
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int vscpClientTcp::send(vscpEventEx &ex)
{
    return m_tcp.doCmdSendEx(&ex);
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int vscpClientTcp::receive(vscpEvent &ev)
{
    return m_tcp.doCmdReceive(&ev);
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int vscpClientTcp::receive(vscpEventEx &ex)
{
    return m_tcp.doCmdReceiveEx(&ex);       
}

///////////////////////////////////////////////////////////////////////////////
// setfilter
//

int vscpClientTcp::setfilter(vscpEventFilter &filter)
{
    if (m_bPolling) {
        return m_tcp.doCmdFilter(&filter);
    }
    else {
        m_mutexReceive.lock();
        return m_tcpReceive.doCmdFilter(&filter);
        m_mutexReceive.unlock();
    }
}

///////////////////////////////////////////////////////////////////////////////
// getcount
//

int vscpClientTcp::getcount(uint16_t *pcount)
{
    if (NULL == pcount) return VSCP_ERROR_INVALID_POINTER;
    *pcount = m_tcp.doCmdDataAvailable();
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// clear
//

int vscpClientTcp::clear(void) 
{
    return m_tcp.doCmdClear();
}

///////////////////////////////////////////////////////////////////////////////
// getversion
//

int vscpClientTcp::getversion(uint8_t *pmajor,
                                uint8_t *pminor,
                                uint8_t *prelease,
                                uint8_t *pbuild)
{
    *pbuild = 0;
    return m_tcp.doCmdVersion(pmajor, pminor, prelease);
}

///////////////////////////////////////////////////////////////////////////////
// getinterfaces
//

int vscpClientTcp::getinterfaces(std::deque<std::string> &iflist)
{
    return m_tcp.doCmdInterfaceList(iflist);
}

///////////////////////////////////////////////////////////////////////////////
// getwcyd
//

int vscpClientTcp::getwcyd(uint64_t &wcyd)
{
    return VSCP_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// setConnectionTimeout
//

void vscpClientTcp::setConnectionTimeout(uint32_t timeout) 
{
    m_tcp.setConnectTimeout(timeout/1000);
}

///////////////////////////////////////////////////////////////////////////////
// getConnectionTimeout
//

uint32_t vscpClientTcp::getConnectionTimeout(void)
{
    return m_tcp.getConnectTimeout();
}

///////////////////////////////////////////////////////////////////////////////
// setResponeTimeout
//

void vscpClientTcp::setResponseTimeout(uint32_t timeout)
{
    m_tcp.setResponseTimeout(timeout);
}

///////////////////////////////////////////////////////////////////////////////
// getResponseTimeout
//

uint32_t vscpClientTcp::getResponseTimeout(void)
{
    return m_tcp.getResponseTimeout();
}

///////////////////////////////////////////////////////////////////////////////
// sendToCallbacks
//

void vscpClientTcp::sendToCallbacks(vscpEvent *pev)
{
    if (nullptr != m_evcallback) {
        m_evcallback(pev, m_callbackObject);
    }

    if (nullptr != m_excallback) {
        vscpEventEx ex;
        vscp_convertEventToEventEx(&ex, pev);
        m_excallback(&ex, m_callbackObject);
    }
}



// ----------------------------------------------------------------------------
//                            Receive worker thread
// ----------------------------------------------------------------------------




void workerThread(vscpClientTcp *pObj)
{
    vscpEvent ev;
    
    ev.sizeData = 0;
    ev.pdata = NULL;

    // Check pointer
    if (nullptr == pObj) return;

    VscpRemoteTcpIf *m_pifReceive = pObj->getTcpReceive();

    m_pifReceive->doCmdEnterReceiveLoop();

    while (pObj->m_bRun) {

        pObj->m_mutexReceive.lock();

        // m_pif->rcvloopRead(500);
        if ( VSCP_ERROR_SUCCESS == m_pifReceive->doCmdBlockingReceive(&ev) ) {
            pObj->sendToCallbacks(&ev);
            vscp_deleteEvent(&ev);
        }
        
        if ( !m_pifReceive->isConnected() ) {
            pObj->m_bRun = false;
        }

        pObj->m_mutexReceive.unlock();

    }       
}