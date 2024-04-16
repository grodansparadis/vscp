/////////////////////////////////////////////////////////////////////////////
// Name:        vscp_bootdevice.cpp
// Purpose:
// Author:      Ake Hedman
// Modified by:
// Created:     16/12/2009 22:26:09
// RCS-ID:
// Copyright:  (C) 2000-2024
// Ake Hedman, the VSCP project, <info@vscp.org>
// Licence:
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// This file is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
//  Boston, MA 02111-1307, USA.
//
//  As a special exception, if other files instantiate templates or use macros
//  or inline functions from this file, or you compile this file and link it
//  with other works to produce a work based on this file, this file does not
//  by itself cause the resulting work to be covered by the GNU General Public
//  License. However the source code for this file must still be made available
//  in accordance with section (3) of the GNU General Public License.
//
//  This exception does not invalidate any other reasons why a work based on
//  this file might be covered by the GNU General Public License.
//
//  Alternative licenses for VSCP & Friends may be arranged by contacting
//  the VSCP project at info@vscp.org, https://www.vscp.org
//

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "vscp_bootdevice.h"
#endif

#include "vscp_bootdevice.h"


///////////////////////////////////////////////////////////////////////////////
// Ctor
//

CBootMemBlock::CBootMemBlock(uint32_t address, uint8_t blksize, const uint8_t *pblkdata)
{
  m_startAddress = address;
  m_blksize      = blksize;
  m_pmemblk      = nullptr;
  if (nullptr != pblkdata) {
    m_pmemblk = new uint8_t[blksize];
    if (nullptr != m_pmemblk) {
      memcpy(m_pmemblk, pblkdata, blksize);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Dtor
//

CBootMemBlock::~CBootMemBlock(void)
{
  if (nullptr != m_pmemblk) {
    delete[] m_pmemblk;
    m_pmemblk = nullptr;
  }
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// Ctor
//

CBootDevice::CBootDevice(CVscpClient *pclient, uint16_t nodeid)
{
  m_pclient = pclient;
  m_nodeid = nodeid;
  m_guid.clear();
  m_guid.setNicknameID(nodeid);
  m_guidif.clear();
}

CBootDevice::CBootDevice(CVscpClient *pclient, cguid &guid, cguid &ifguid)
{
  m_pclient = pclient;
  m_guid   = guid;
  m_guidif = ifguid;
}

///////////////////////////////////////////////////////////////////////////////
// Dtor
//

CBootDevice::~CBootDevice(void)
{
  
}

///////////////////////////////////////////////////////////////////////////////
// loadHexFile
//

int 
CBootDevice::loadHexFile(const std::string &path)
{
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getInfo
//

std::string 
CBootDevice::getInfo(void)
{
  std::string strinfo;

  return strinfo;
}

