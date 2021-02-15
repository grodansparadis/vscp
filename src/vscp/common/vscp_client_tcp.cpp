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



///////////////////////////////////////////////////////////////////////////////
// C-tor
//

vscpClientTcp::vscpClientTcp() 
{
    m_type = CVscpClient::connType::TCPIP;
    
    // Default connect parameters
    m_strHostname = "localhost";
    m_port = VSCP_DEFAULT_TCP_PORT;
    m_strUsername = "admin";
    m_strPassword = "secret";
}

///////////////////////////////////////////////////////////////////////////////
// D-tor
//

vscpClientTcp::~vscpClientTcp() 
{
    
}


///////////////////////////////////////////////////////////////////////////////
// toJSON
//

std::string vscpClientTcp::toJSON(void) 
{
    std::string rv;

    return rv;
}


///////////////////////////////////////////////////////////////////////////////
// fromJSON
//

bool vscpClientTcp::fromJSON(const std::string& config)
{
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// init
//

int vscpClientTcp::init(const std::string &strHostname,
                        short port,
                        const std::string &strUsername,
                        const std::string &strPassword)
{
    m_strHostname = strHostname;
    m_port = port;
    m_strUsername = strUsername;
    m_strPassword = strPassword;
    return VSCP_ERROR_SUCCESS;
}                

///////////////////////////////////////////////////////////////////////////////
// connect
//

int vscpClientTcp::connect(void) 
{
    return m_tcp.doCmdOpen(m_strHostname,
                            m_port,
                            m_strUsername,
                            m_strPassword);
}

///////////////////////////////////////////////////////////////////////////////
// disconnect
//

int vscpClientTcp::disconnect(void)
{
    return m_tcp.doCmdClose();
}

///////////////////////////////////////////////////////////////////////////////
// isConnected
//

bool vscpClientTcp::isConnected(void)
{
    return m_tcp.isConnected();
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
    return m_tcp.doCmdFilter(&filter);
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
