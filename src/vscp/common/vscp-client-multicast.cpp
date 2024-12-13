// vscp_client_multicast.cpp
//
// Multicast client communication classes.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// Copyright:   (C) 2007-2023
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

#include "vscphelper.h"
#include "vscp-client-multicast.h"

///////////////////////////////////////////////////////////////////////////////
// CTor
//

vscpClientMulticast::vscpClientMulticast() : CVscpClient()
{
    m_type = CVscpClient::connType::MULTICAST;
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

vscpClientMulticast::~vscpClientMulticast()
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
// getConfigAsJson
//

std::string vscpClientMulticast::getConfigAsJson(void) 
{
    std::string rv;

    return rv;
}


///////////////////////////////////////////////////////////////////////////////
// initFromJson
//

bool vscpClientMulticast::initFromJson(const std::string& /*config*/)
{
    return true;
}


///////////////////////////////////////////////////////////////////////////////
// connect
//

int vscpClientMulticast::connect(void)
{
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// disconnect
//

int vscpClientMulticast::disconnect(void)
{
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// isConnected
//

bool vscpClientMulticast::isConnected(void)
{
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int vscpClientMulticast::send(vscpEvent & /*ev*/)
{
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int vscpClientMulticast::send(vscpEventEx & /*ex*/)
{
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int
vscpClientMulticast::send(canalMsg &msg)
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

int vscpClientMulticast::receive(vscpEvent & /*ev*/)
{
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int vscpClientMulticast::receive(vscpEventEx & /*ex*/)
{
    return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// receive
//
  
int  
vscpClientMulticast::receive(canalMsg &msg)
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

///////////////////////////////////////////////////////////////////////////////
// receiveBlocking
//

int
vscpClientMulticast::receiveBlocking(vscpEvent &ev, long timeout)
{
  // if (-1 == vscp_sem_wait(&m_semReceiveQueue, timeout)) {
  //   if (errno == ETIMEDOUT) {
  //     return VSCP_ERROR_TIMEOUT;
  //   }
  //   else {
  //     return VSCP_ERROR_ERROR;
  //   }
  // }

  return receive(ev);
}

///////////////////////////////////////////////////////////////////////////////
// receiveBlocking
//

int
vscpClientMulticast::receiveBlocking(vscpEventEx &ex, long timeout)
{
  // if (-1 == vscp_sem_wait(&m_semReceiveQueue, timeout)) {
  //   if (errno == ETIMEDOUT) {
  //     return VSCP_ERROR_TIMEOUT;
  //   }
  //   else {
  //     return VSCP_ERROR_ERROR;
  //   }
  // }

  return receive(ex);
}

///////////////////////////////////////////////////////////////////////////////
// receiveBlocking
//

int
vscpClientMulticast::receiveBlocking(canalMsg &msg, long timeout)
{
  // if (-1 == vscp_sem_wait(&m_semReceiveQueue, timeout)) {
  //   if (errno == ETIMEDOUT) {
  //     return VSCP_ERROR_TIMEOUT;
  //   }
  //   else {
  //     return VSCP_ERROR_ERROR;
  //   }
  // }

  return receive(msg);
}

//////////////////////////////////////////////////////////////////////////////
// setfilter
//

int vscpClientMulticast::setfilter(vscpEventFilter & /*filter*/)
{
    return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// getcount
//

int vscpClientMulticast::getcount(uint16_t *pcount)
{
    if (NULL == pcount ) return VSCP_ERROR_INVALID_POINTER;
    *pcount = 0;
    return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// clear
//

int vscpClientMulticast::clear()
{
    return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// getversion
//

int vscpClientMulticast::getversion(uint8_t *pmajor,
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

int vscpClientMulticast::getinterfaces(std::deque<std::string> & /*iflist*/)
{
    return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// getwcyd
//

int vscpClientMulticast::getwcyd(uint64_t & /*wcyd*/)
{
    return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// setConnectionTimeout
//

void vscpClientMulticast::setConnectionTimeout(uint32_t /*timeout*/)
{
    ;
}

//////////////////////////////////////////////////////////////////////////////
// getConnectionTimeout
//

uint32_t vscpClientMulticast::getConnectionTimeout(void)
{
    return 0;
}

//////////////////////////////////////////////////////////////////////////////
// setResponseTimeout
//

void vscpClientMulticast::setResponseTimeout(uint32_t /*timeout*/)
{
    ;
}

//////////////////////////////////////////////////////////////////////////////
// getResponseTimeout
//

uint32_t vscpClientMulticast::getResponseTimeout(void)
{
    return 0;
}

