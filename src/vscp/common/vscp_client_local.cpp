// vscp_client_local.cpp
//
// Localclient communication classes.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// Copyright:  (C) 2007-2024
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
#include <pch.h>
#endif

#include "vscp_client_local.h"

///////////////////////////////////////////////////////////////////////////////
// CTor
//

vscpClientLocal::vscpClientLocal() : CVscpClient()
{
    m_type = CVscpClient::connType::LOCAL;
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

vscpClientLocal::~vscpClientLocal()
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
// getConfigAsJson
//

std::string vscpClientLocal::getConfigAsJson(void) 
{
    json j;
    std::string rv;    

    j["name"] = getName();
    j["path"] = m_path;

    return j.dump();
}


///////////////////////////////////////////////////////////////////////////////
// initFromJson
//

bool vscpClientLocal::initFromJson(const std::string& config)
{
    json j;
    try {
        j = json::parse(config);
        
        if (!j["name"].is_string()) return false;
        if (!j["path"].is_string()) return false;

        setName(j["name"]);
        m_path = j["path"];
    }
    catch (...) {
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// connect
//

int vscpClientLocal::connect(void)
{
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// disconnect
//

int vscpClientLocal::disconnect(void)
{
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// isConnected
//

bool vscpClientLocal::isConnected(void)
{
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int vscpClientLocal::send(vscpEvent &ev)
{
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int vscpClientLocal::send(vscpEventEx &ex)
{
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int vscpClientLocal::send(canalMsg &msg)
{
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int vscpClientLocal::receive(vscpEvent &ev)
{
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int vscpClientLocal::receive(vscpEventEx &ex)
{
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int vscpClientLocal::receive(canalMsg &msg)
{
    return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// setfilter
//

int vscpClientLocal::setfilter(vscpEventFilter &filter)
{
    return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// getcount
//

int vscpClientLocal::getcount(uint16_t *pcount)
{
    if (NULL == pcount ) return VSCP_ERROR_INVALID_POINTER;
    *pcount = 0;
    return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// clear
//

int vscpClientLocal::clear()
{
    return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// getversion
//

int vscpClientLocal::getversion(uint8_t *pmajor,
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

int vscpClientLocal::getinterfaces(std::deque<std::string> &iflist)
{
    return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// getwcyd
//

int vscpClientLocal::getwcyd(uint64_t &wcyd)
{
    return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// setConnectionTimeout
//

void vscpClientLocal::setConnectionTimeout(uint32_t timeout)
{
    ;
}

//////////////////////////////////////////////////////////////////////////////
// getConnectionTimeout
//

uint32_t vscpClientLocal::getConnectionTimeout(void)
{
    return 0;
}

//////////////////////////////////////////////////////////////////////////////
// setResponseTimeout
//

void vscpClientLocal::setResponseTimeout(uint32_t timeout)
{
    ;
}

//////////////////////////////////////////////////////////////////////////////
// getResponseTimeout
//

uint32_t vscpClientLocal::getResponseTimeout(void)
{
    return 0;
}

