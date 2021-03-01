// vscp_client_rest.cpp
//
// REST client communication classes.
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

#include "vscp_client_rest.h"

///////////////////////////////////////////////////////////////////////////////
// CTor
//

vscpClientRest::vscpClientRest() : CVscpClient()
{
    m_type = CVscpClient::connType::REST;
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

vscpClientRest::~vscpClientRest()
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
// getConfigAsJson
//

std::string vscpClientRest::getConfigAsJson(void) 
{
    std::string rv;

    return rv;
}


///////////////////////////////////////////////////////////////////////////////
// initFromJson
//

bool vscpClientRest::initFromJson(const std::string& config)
{
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// connect
//

int vscpClientRest::connect(void)
{
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// disconnect
//

int vscpClientRest::disconnect(void)
{
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// isConnected
//

bool vscpClientRest::isConnected(void)
{
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int vscpClientRest::send(vscpEvent &ev)
{
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int vscpClientRest::send(vscpEventEx &ex)
{
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int vscpClientRest::receive(vscpEvent &ev)
{
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int vscpClientRest::receive(vscpEventEx &ex)
{
    return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// setfilter
//

int vscpClientRest::setfilter(vscpEventFilter &filter)
{
    return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// getcount
//

int vscpClientRest::getcount(uint16_t *pcount)
{
    if (NULL == pcount ) return VSCP_ERROR_INVALID_POINTER;
    *pcount = 0;
    return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// clear
//

int vscpClientRest::clear()
{
    return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// getversion
//

int vscpClientRest::getversion(uint8_t *pmajor,
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

int vscpClientRest::getinterfaces(std::deque<std::string> &iflist)
{
    return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// getwcyd
//

int vscpClientRest::getwcyd(uint64_t &wcyd)
{
    return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// setConnectionTimeout
//

void vscpClientRest::setConnectionTimeout(uint32_t timeout)
{
    ;
}

//////////////////////////////////////////////////////////////////////////////
// getConnectionTimeout
//

uint32_t vscpClientRest::getConnectionTimeout(void)
{
    return 0;
}

//////////////////////////////////////////////////////////////////////////////
// setResponseTimeout
//

void vscpClientRest::setResponseTimeout(uint32_t timeout)
{
    ;
}

//////////////////////////////////////////////////////////////////////////////
// getResponseTimeout
//

uint32_t vscpClientRest::getResponseTimeout(void)
{
    return 0;
}
