// vscp_client_rs485.cpp
//
// RS-485 client communication classes.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// Copyright: © 2007-2021
// Ake Hedman, the VSCP project, <info@vscp.org>
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

#ifdef WIN32
#include <StdAfx.h>
#endif

#include "vscp_client_rs485.h"

///////////////////////////////////////////////////////////////////////////////
// CTor
//

vscpClientRs485::vscpClientRs485() : CVscpClient()
{
    m_type = CVscpClient::connType::RS485;
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

vscpClientRs485::~vscpClientRs485()
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
// getConfigAsJson
//

std::string vscpClientRs485::getConfigAsJson(void) 
{
    std::string rv;

    return rv;
}


///////////////////////////////////////////////////////////////////////////////
// initFromJson
//

bool vscpClientRs485::initFromJson(const std::string& config)
{
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// connect
//

int vscpClientRs485::connect(void)
{
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// disconnect
//

int vscpClientRs485::disconnect(void)
{
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// isConnected
//

bool vscpClientRs485::isConnected(void)
{
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int vscpClientRs485::send(vscpEvent &ev)
{
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int vscpClientRs485::send(vscpEventEx &ex)
{
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int vscpClientRs485::receive(vscpEvent &ev)
{
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int vscpClientRs485::receive(vscpEventEx &ex)
{
    return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// setfilter
//

int vscpClientRs485::setfilter(vscpEventFilter &filter)
{
    return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// getcount
//

int vscpClientRs485::getcount(uint16_t *pcount)
{
    if (NULL == pcount ) return VSCP_ERROR_INVALID_POINTER;
    *pcount = 0;
    return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// clear
//

int vscpClientRs485::clear()
{
    return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// getversion
//

int vscpClientRs485::getversion(uint8_t *pmajor,
                                uint8_t *pminor,
                                uint8_t *prelease,
                                uint8_t *pbuild)
{
    if (NULL == pmajor ) return VSCP_ERROR_INVALID_POINTER;
    if (NULL == pminor ) return VSCP_ERROR_INVALID_POINTER;
    if (NULL == prelease ) return VSCP_ERROR_INVALID_POINTER;
    if (NULL == pbuild ) return VSCP_ERROR_INVALID_POINTER;
    return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// getinterfaces
//

int vscpClientRs485::getinterfaces(std::deque<std::string> &iflist)
{
    return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// getwcyd
//

int vscpClientRs485::getwcyd(uint64_t &wcyd)
{
    return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// setConnectionTimeout
//

void vscpClientRs485::setConnectionTimeout(uint32_t timeout)
{
    ;
}

//////////////////////////////////////////////////////////////////////////////
// getConnectionTimeout
//

uint32_t vscpClientRs485::getConnectionTimeout(void)
{
    return 0;
}

//////////////////////////////////////////////////////////////////////////////
// setResponseTimeout
//

void vscpClientRs485::setResponseTimeout(uint32_t timeout)
{
    ;
}

//////////////////////////////////////////////////////////////////////////////
// getResponseTimeout
//

uint32_t vscpClientRs485::getResponseTimeout(void)
{
    return 0;
}