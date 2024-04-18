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

#include <fstream>
#include <iostream>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

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

CBootDevice::CBootDevice(CVscpClient *pclient,
                         uint16_t nodeid,
                         std::function<void(int)> statusCallback,
                         uint32_t timeout)
{
  m_pclient = pclient;
  m_nodeid  = nodeid;
  m_guid.clear();
  m_guidif.clear();
  m_timeout        = timeout;
  m_statusCallback = statusCallback;
}

CBootDevice::CBootDevice(CVscpClient *pclient,
                         uint16_t nodeid,
                         cguid &guidif,
                         std::function<void(int)> statusCallback,
                         uint32_t timeout)
{
  m_pclient = pclient;
  m_nodeid  = nodeid;
  m_guid.clear();
  m_guidif         = guidif;
  m_timeout        = timeout;
  m_statusCallback = statusCallback;
}

CBootDevice::CBootDevice(CVscpClient *pclient, cguid &guid, std::function<void(int)> statusCallback, uint32_t timeout)
{
  m_pclient = pclient;
  m_nodeid  = 0;
  m_guid    = guid;
  m_guidif.clear();
  m_timeout        = timeout;
  m_statusCallback = statusCallback;
}

///////////////////////////////////////////////////////////////////////////////
// Dtor
//

CBootDevice::~CBootDevice(void)
{
  while (m_memblkList.size()) {
    CBootMemBlock *pblk = m_memblkList.front();
    m_memblkList.pop_front();
    delete pblk;
    pblk = nullptr;
  }
}

///////////////////////////////////////////////////////////////////////////////
// loadIntelHexFile
//

int
CBootDevice::loadIntelHexFile(const std::string &path)
{
  char szline[2048];
  char buf[16];
  char *endptr;
  std::ifstream hexfile;
  uint16_t lowAddr     = 0; // Low part of address
  uint16_t highAddr    = 0; // High part if address
  uint32_t segmentAddr = 0; // Extended segment address (type=2)
  uint8_t cnt          = 0;
  uint8_t recType      = 0;

  try {
    hexfile.open(path);
  }
  catch (...) {
    spdlog::error("Load intel hex: Unable to open file {%0} error={%1} '{%2}'", path, errno, strerror(errno));
  }

  bool bRun = true;
  while (bRun && hexfile.getline(szline, sizeof(szline))) {

    // We ignore all lines that does not contain
    // ':'
    if (':' == szline[0]) {

      uint8_t linebuf[1024];
      int linecnt = vscp_hexStr2ByteArray(linebuf, sizeof(linebuf), (szline + 1));
      // Length must be at least cnt + addr + type + sum = 5
      if (linecnt < 5) {
        spdlog::error("Load intel hex: Line length is wrong cnt={%0} '{%1}'", cnt, szline);
        hexfile.close();
        return VSCP_ERROR_PARAMETER;
      }

      // Validate checksum
      uint16_t sum = 0;
      for (int i = 0; i < linecnt; i++) {
        sum += linebuf[i];
      }
      if (sum & 0xff) {
        spdlog::error("Load intel hex: Checksum is wrong cnt={%0} '{%1}'", cnt, szline);
        hexfile.close();
        return VSCP_ERROR_PARAMETER;
      }

      cnt     = linebuf[0];
      lowAddr = (uint32_t) linebuf[1] << 16 + linebuf[2];
      recType = linebuf[3];

      // Construct fulladdress
      uint32_t fulladdr = (highAddr << 16) + lowAddr + segmentAddr;

      // Handle row as of record type
      switch (recType) {

        case CBootDevice::INTELHEX_LINETYPE_DATA: {
          uint8_t mem[256];
          for (int i = 0; i < cnt; i++) {
            mem[i] = linebuf[4 + i];
          }
          CBootMemBlock *pblk = new CBootMemBlock(fulladdr, cnt, mem);
          if (nullptr == pblk) {
            spdlog::error("Load intel hex: Unabled to allocate block");
            hexfile.close();
            return VSCP_ERROR_MEMORY;
          }
          m_memblkList.push_back(pblk);
        } break;

        case CBootDevice::INTELHEX_LINETYPE_EOF:
          bRun = false; // We are done
          break;

        case CBootDevice::INTELHEX_LINETYPE_EXTENDED_SEGMENT:
          // Count should always be 4
          if (4 != linebuf[0]) {
            spdlog::error("Load intel hex: Byte count is wrong cnt={%0} '{%1}'", cnt, szline);
            hexfile.close();
            return VSCP_ERROR_PARAMETER;
          }
          segmentAddr = (((uint16_t) linebuf[4]) << 8 + linebuf[5]) * 16;
          break;

        case CBootDevice::INTELHEX_LINETYPE_EXTENDED_LINEAR:
          // Count should always be 2
          if (2 != linebuf[0]) {
            spdlog::error("Load intel hex: Byte count is wrong cnt={%0} '{%1}'", cnt, szline);
            hexfile.close();
            return VSCP_ERROR_PARAMETER;
          }
          highAddr = (((uint16_t) linebuf[4]) << 8 + linebuf[5]);
          break;

        case CBootDevice::INTELHEX_LINETYPE_START_SEGMENT:
          // We do not handle this record type
          spdlog::error("Load intel hex: Skipped: start segment record cnt={%0} '{%1}'", cnt, szline);
          break;

        case CBootDevice::INTELHEX_LINETYPE_START_LINEAR:
          // We do not handle this record type
          spdlog::error("Load intel hex: Skipped: start linear record cnt={%0} '{%1}'", cnt, szline);
          break;
      }
    }
  }
  hexfile.close();
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getMinMaxForRange
//

int
CBootDevice::getMinMaxForRange(uint32_t start, uint32_t end, uint32_t *pmin, uint32_t *pmax)
{
  if (nullptr == pmin) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  if (nullptr == pmax) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  if (end <= start) {
    return VSCP_ERROR_PARAMETER;
  }

  *pmin = 0xffffffff;
  *pmax = 0;

  for (auto const &pblk : m_memblkList) {
    if ((pblk->getStartAddress() >= start) && (pblk->getStartAddress() <= end)) {
      // Set min to lowest address found
      if (pblk->getStartAddress() < *pmin) {
        *pmin = pblk->getStartAddress();
      }
      // Set max to highest address found
      if ((pblk->getStartAddress() + pblk->getSize()) > *pmax) {
        *pmax = pblk->getStartAddress() + pblk->getSize();
      }
    }
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// fillBlock
//

int
CBootDevice::fillBlock(uint8_t *pblock, uint32_t size, uint32_t start, uint8_t fill)
{
  if (nullptr == pblock) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  if (size > BOOT_MAX_BLOCK_SIZE) {
    return VSCP_ERROR_SIZE;
  }

  // Fill block with void value (0xff for flash)
  memset(pblock, fill, size);

  for (auto const &pblk : m_memblkList) {
    if ((pblk->getStartAddress() >= start) && (pblk->getStartAddress() < (start + size))) {
      for (uint8_t pos = 0; pos < pblk->getSize(); pos++) {
        pblock[(pblk->getStartAddress() - start) + pos] = *(pblk->getBlock() + pos);
      }
    }
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// deviceInfo
//

std::string
CBootDevice::deviceInfo(void)
{
  std::string strinfo;

  return strinfo;
}
