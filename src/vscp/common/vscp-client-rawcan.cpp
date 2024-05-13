// vscp_client_rawcan.cpp
//
// Raw CAN client communication classes.
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

#include "vscp_client_rawcan.h"
#include "vscphelper.h"

///////////////////////////////////////////////////////////////////////////////
// CTor
//

vscpClientRawCan::vscpClientRawCan()
  : CVscpClient()
{
  m_type = CVscpClient::connType::UDP;
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

vscpClientRawCan::~vscpClientRawCan()
{
  ;
}

///////////////////////////////////////////////////////////////////////////////
// getConfigAsJson
//

std::string
vscpClientRawCan::getConfigAsJson(void)
{
  std::string rv;

  return rv;
}

///////////////////////////////////////////////////////////////////////////////
// initFromJson
//

bool
vscpClientRawCan::initFromJson(const std::string &config)
{
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// connect
//

int
vscpClientRawCan::connect(void)
{
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// disconnect
//

int
vscpClientRawCan::disconnect(void)
{
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// isConnected
//

bool
vscpClientRawCan::isConnected(void)
{
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int
vscpClientRawCan::send(vscpEvent &ev)
{
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int
vscpClientRawCan::send(vscpEventEx &ex)
{
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int
vscpClientRawCan::send(canalMsg &msg)
{
  vscpEventEx ex;

  uint8_t guid[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  if (vscp_convertCanalToEventEx(&ex, &msg, guid)) {
    return VSCP_ERROR_INVALID_FRAME;
  }

  return send(ex);
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int
vscpClientRawCan::receive(vscpEvent &ev)
{
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int
vscpClientRawCan::receive(vscpEventEx &ex)
{
  return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// receive
//

int
vscpClientRawCan::receive(canalMsg &msg)
{
  int rv;
  vscpEventEx ex;

  if (VSCP_ERROR_SUCCESS != (rv = receive(ex))) {
    return rv;
  }

  if (!vscp_convertEventExToCanal(&msg, &ex)) {
    return VSCP_ERROR_INVALID_FRAME;
  }

  return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// setfilter
//

int
vscpClientRawCan::setfilter(vscpEventFilter &filter)
{
  return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// getcount
//

int
vscpClientRawCan::getcount(uint16_t *pcount)
{
  if (NULL == pcount)
    return VSCP_ERROR_INVALID_POINTER;
  *pcount = 0;
  return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// clear
//

int
vscpClientRawCan::clear()
{
  return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// getversion
//

int
vscpClientRawCan::getversion(uint8_t *pmajor, uint8_t *pminor, uint8_t *prelease, uint8_t *pbuild)
{
  if (NULL == pmajor)
    return VSCP_ERROR_INVALID_POINTER;
  if (NULL == pminor)
    return VSCP_ERROR_INVALID_POINTER;
  if (NULL == prelease)
    return VSCP_ERROR_INVALID_POINTER;
  if (NULL == pbuild)
    return VSCP_ERROR_INVALID_POINTER;
  return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// getinterfaces
//

int
vscpClientRawCan::getinterfaces(std::deque<std::string> &iflist)
{
  return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// getwcyd
//

int
vscpClientRawCan::getwcyd(uint64_t &wcyd)
{
  return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// setConnectionTimeout
//

void
vscpClientRawCan::setConnectionTimeout(uint32_t timeout)
{
  ;
}

//////////////////////////////////////////////////////////////////////////////
// getConnectionTimeout
//

uint32_t
vscpClientRawCan::getConnectionTimeout(void)
{
  return 0;
}

//////////////////////////////////////////////////////////////////////////////
// setResponseTimeout
//

void
vscpClientRawCan::setResponseTimeout(uint32_t timeout)
{
  ;
}

//////////////////////////////////////////////////////////////////////////////
// getResponseTimeout
//

uint32_t
vscpClientRawCan::getResponseTimeout(void)
{
  return 0;
}