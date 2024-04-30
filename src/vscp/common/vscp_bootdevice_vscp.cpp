/////////////////////////////////////////////////////////////////////////////
// bootdevice_vscp.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// Copyright:  (C) 2000-2024
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
//

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "vscp_bootdevice_vscp.h"
#endif

#ifdef WIN32
#include <winsock2.h>
#endif

#include "vscp_bootdevice_vscp.h"

#include <stdio.h>

#define CRC16
#include <crc.h>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

CBootDevice_VSCP::CBootDevice_VSCP(CVscpClient *pclient,
                                   uint8_t nodeid,
                                   std::function<void(int, const char *)> statusCallback,
                                   uint32_t timeout)
  : CBootDevice(pclient, nodeid, statusCallback, timeout)
{
  m_chunkSize = 8;
  init();
}

CBootDevice_VSCP::CBootDevice_VSCP(CVscpClient *pclient,
                                   uint8_t nodeid,
                                   cguid &guidif,
                                   std::function<void(int, const char *)> statusCallback,
                                   uint32_t timeout)
  : CBootDevice(pclient, nodeid, guidif, statusCallback, timeout)
{
  m_chunkSize = 8;
  init();
}

CBootDevice_VSCP::CBootDevice_VSCP(CVscpClient *pclient,
                                   cguid &guid,
                                   std::function<void(int, const char *)> statusCallback,
                                   uint32_t timeout)
  : CBootDevice(pclient, guid, statusCallback, timeout)
{
  m_chunkSize = 512;
  init();
}

///////////////////////////////////////////////////////////////////////////////
// DTOR
//

CBootDevice_VSCP::~CBootDevice_VSCP(void)
{
  ;
}

///////////////////////////////////////////////////////////////////////////////
// init
//

void
CBootDevice_VSCP::init(void)
{
  m_pAddr     = 0;
  m_numBlocks = 0;
  m_blockSize = 0;
  crcInit();
}

///////////////////////////////////////////////////////////////////////////////
// deviceInfo
//

std::string
CBootDevice_VSCP::deviceInfo(void)
{
  std::string str;
  std::string strInfo;
  std::ostringstream oss;

  uint32_t min;
  uint32_t max;

  // * * * Device * * *
  oss << "<b><u>Device</u></b><br>";
  oss << "<b>nodeid :</b><font color=\"#005CB9\">";
  oss << m_nodeid;
  oss << "</font><br>";
  oss << "<b>GUID :</b><font color=\"#005CB9\">";
  oss << m_guid.toString();
  oss << "</font><br>";
  oss << "<b>Interface :</b><font color=\"#005CB9\">";
  oss << m_guidif.toString();
  oss << "</font><br>";

  // * * * Flash Memory * * *
  getMinMaxForRange(MEM_CODE_START, MEM_CODE_END, &min, &max);
  oss << "<b><u>Flash Memory</u></b><br>";
  oss << "<b>Start :</b><font color=\"#005CB9\">";
  oss << std::hex << std::setw(8) << std::setfill('0') << min;
  oss << "</font><b> End :</b><font color=\"#005CB9\">";
  oss << std::hex << std::setw(8) << std::setfill('0') << max;
  if (max > min) {
    oss << "<font color=\"#348017\">Will be programmed</font><br>";
  }
  else {
    oss << "<font color=\"#F6358A\">Will not be programmed</font><br>";
  }
  oss << "<br><br>";

  // * * * UserID Memory * * *
  getMinMaxForRange(MEM_USERID_START, MEM_CODE_END, &min, &max);
  oss << "<b><u>UserID Memory</u></b><br>";
  oss << "<b>Start :</b>";

  oss << "<font color=\"#005CB9\">";
  oss << std::hex << std::setw(8) << std::setfill('0') << min << "</font>";

  oss << "<b> End :</b>";

  oss << "<font color=\"#005CB9\">";
  oss << std::hex << std::setw(8) << std::setfill('0') << max << "</font>";

  if (max > min) {
    oss << "<font color=\"#348017\">Will be programmed</font><br>";
  }
  else {
    oss << "<font color=\"#F6358A\">Will not be programmed</font><br>";
  }
  oss << "<br><br>";

  // * * * Config Memory * * *
  getMinMaxForRange(MEM_CONFIG_START, MEM_CONFIG_END, &min, &max);
  oss << "<b><u>Config Memory</u></b><br>";
  oss << "<b>Start :</b>";

  oss << "<font color=\"#005CB9\">";
  oss << std::hex << std::setw(8) << std::setfill('0') << min << "</font>";

  oss << "<b> End :</b>";

  oss << "<font color=\"#005CB9\">";
  oss << std::hex << std::setw(8) << std::setfill('0') << max << "</font>";

  if (max > min) {
    oss << "<font color=\"#348017\">Will be programmed</font><br>";
  }
  else {
    oss << "<font color=\"#F6358A\">Will not be programmed</font><br>";
  }
  oss << "<br><br>";

  // * * * EEPROM * * *
  getMinMaxForRange(MEM_EEPROM_START, MEM_EEPROM_END, &min, &max);
  oss << "<b><u>EEPROM Memory</u></b><br>";
  oss << "<B>Start :</b>";

  oss << "<font color=\"#005CB9\">";
  oss << std::hex << std::setw(8) << std::setfill('0') << min << "</font>";
  oss << "<b> End :</b>";

  oss << "<font color=\"#005CB9\">";
  oss << std::hex << std::setw(8) << std::setfill('0') << PRIdMAX << "</font>";

  if (max > min) {
    oss << "<font color=\"#348017\">Will be programmed</font><br>";
  }
  else {
    oss << "<font color=\"#F6358A\">Will not be programmed</font><br>";
  }
  oss << "<br><br>";

  return oss.str();
}

///////////////////////////////////////////////////////////////////////////////
// deviceInit
//

int
CBootDevice_VSCP::deviceInit(cguid &ourguid, uint8_t devicecode, bool bAbortOnFirmwareCodeFail)
{
  int rv;
  vscpEventEx ex;

  // Read standard registers
  if (VSCP_ERROR_SUCCESS != m_stdRegs.init(*m_pclient, m_guid, m_guidif, nullptr, REGISTER_DEFAULT_TIMEOUT)) {
    spdlog::error("VSCP bootloader: Failed to read standard registers");
    return VSCP_ERROR_COMMUNICATION;
  }

  /*
    Check that the remote device expect the firmware load type
    we offer
  */
  if (VSCP_BOOTLOADER_VSCP != m_stdRegs.getBootloaderAlgorithm()) {
    spdlog::error("VSCP bootloader: Bootloader algorithm is not Microchip PIC1");
    if (nullptr != m_statusCallback) {
      m_statusCallback(-1, "Bootloader algorithm is not Microchip PIC1");
    }
    return VSCP_ERROR_NOT_SUPPORTED;
  }

  /*
    The device code tell the type of hardware of the remote device
    Must be the same as the firmware we try to load is intended for
  */
  if (m_firmwaredeviceCode != m_stdRegs.getFirmwareDeviceCode()) {
    spdlog::warn("Firware device code is not equal the one on the device local: {0} device: {1}",
                 m_firmwaredeviceCode,
                 m_stdRegs.getFirmwareDeviceCode());
    if (nullptr != m_statusCallback) {
      m_statusCallback(-1, "Firware device code is not equal the one on the device local: {0} device: {1}");
    }
    if (bAbortOnFirmwareCodeFail) {
      return VSCP_ERROR_PARAMETER;
    }
  }

  // Set device in boot mode
  ex.vscp_class = VSCP_CLASS1_PROTOCOL;
  ex.vscp_type  = VSCP_TYPE_PROTOCOL_ENTER_BOOT_LOADER;
  ex.timestamp  = vscp_makeTimeStamp();
  vscp_setEventExDateTimeBlockToNow(&ex);

  cguid node_guid;
  m_stdRegs.getGUID(node_guid);

  ex.sizeData = 8;
  ex.data[0]  = m_nodeid;             // Nickname to read register from
  ex.data[1]  = VSCP_BOOTLOADER_VSCP; // VSCP bootloader algorithm
  ex.data[2]  = node_guid.getAt(0);
  ex.data[3]  = node_guid.getAt(3);
  ex.data[4]  = node_guid.getAt(5);
  ex.data[5]  = node_guid.getAt(7);
  ex.data[6]  = m_stdRegs.getRegisterPage() >> 8;
  ex.data[7]  = m_stdRegs.getRegisterPage() & 0xff;

  if (VSCP_ERROR_SUCCESS != (rv = m_pclient->send(ex))) {
    spdlog::error("VSCP bootloader: Failed to send enter bootloader event {0}", rv);
    if (nullptr != m_statusCallback) {
      m_statusCallback(-1, "VSCP bootloader: Failed to send enter bootloader event");
    }
    return rv;
  }

  if (VSCP_ERROR_SUCCESS !=
      (rv = checkResponse(ex, node_guid, VSCP_TYPE_PROTOCOL_ACK_BOOT_LOADER, VSCP_TYPE_PROTOCOL_NACK_BOOT_LOADER))) {
    // Negative response on bootmode request- return
    spdlog::debug("VSCP bootloader: NACK recived from set bootloader request.");
    if (nullptr != m_statusCallback) {
      m_statusCallback(-1, "VSCP bootloader: NACK received from set bootloader request.");
    }
    return rv;
  }

  // OK now in bootmode - return
  m_blockSize = construct_unsigned32(ex.data[0], ex.data[1], ex.data[2], ex.data[3]);
  m_numBlocks = construct_unsigned32(ex.data[4], ex.data[5], ex.data[6], ex.data[7]);
  spdlog::debug("VSCP bootloader: Confirmed, device is in boot mood.");
  if (nullptr != m_statusCallback) {
    m_statusCallback(-1, "VSCP bootloader: Confirmed, device is in boot mood.");
  }

  // chunk must be <= blocksize
  if (m_chunkSize > m_blockSize) {
    spdlog::debug("VSCP bootloader: chunk size is larger than block size. chunksz={0} blocksz={1}",
                  m_chunkSize,
                  m_blockSize);
    if (nullptr != m_statusCallback) {
      m_statusCallback(-1, "VSCP bootloader: Chunk size is larger than block size.");
    }
    return VSCP_ERROR_SIZE;
  }

  return VSCP_ERROR_SUCCESS;
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// writeFirmwareBlockDataChunk
//

int
CBootDevice_VSCP::writeFirmwareBlockDataChunk(const uint8_t *paddr, uint16_t size)
{
  int rv;
  vscpEventEx ex;

  // Send the start block
  ex.vscp_class = VSCP_CLASS1_PROTOCOL;
  ex.vscp_type  = VSCP_TYPE_PROTOCOL_BLOCK_DATA;
  ex.timestamp  = vscp_makeTimeStamp();
  vscp_setEventExDateTimeBlockToNow(&ex);

  cguid node_guid;
  m_stdRegs.getGUID(node_guid);

  // Check size
  if (size > m_chunkSize) {
    return VSCP_ERROR_SIZE;
  }

  ex.sizeData = size;
  memcpy(ex.data, paddr, size);

  if (VSCP_ERROR_SUCCESS != (rv = m_pclient->send(ex))) {
    spdlog::error("VSCP bootloader: Failed to send start block chunk transfer event {0}", rv);
    if (nullptr != m_statusCallback) {
      m_statusCallback(-1, "VSCP bootloader: Failed to send start block chunk transfer event");
    }
    return rv;
  }

  // Wait for response on start block transfer event
  if (VSCP_ERROR_SUCCESS !=
      (rv = checkResponse(ex, node_guid, VSCP_TYPE_PROTOCOL_START_BLOCK_ACK, VSCP_TYPE_PROTOCOL_START_BLOCK_NACK))) {
    spdlog::error("VSCP bootloader: Negative response from block start request {0}", rv);
    if (nullptr != m_statusCallback) {
      m_statusCallback(-1, "VSCP bootloader: Negative response from block start request");
    }
    return rv;
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// writeFirmwareBlock
//

int
CBootDevice_VSCP::writeFirmwareBlock(const uint8_t *paddr)
{
  int rv;
  vscpEventEx ex;
  cguid guid;
  uint32_t nChunks;

  if (m_blockSize > m_chunkSize) {
    nChunks = m_blockSize / m_chunkSize;
    // A not completely full packet also count
    if (0 != (m_blockSize % m_chunkSize)) {
      nChunks++;
    }
  }
  else {
    // There is just one chunk
    // We have taken care of chunk > block in init
    nChunks = 1;
  }

  for (uint32_t chunk = 0; chunk < nChunks; chunk++) {
    //uint32_t addr = memory_range[8].beginning;
    spdlog::debug("Loading flash on remote device... block={0} {1:X}", chunk, chunk * m_chunkSize);
    if (VSCP_ERROR_SUCCESS != writeFirmwareBlockDataChunk(paddr + (m_chunkSize*nChunks), m_chunkSize)) {
      spdlog::error("Failed to write flash data to node(s).");
      break;
    }

    paddr += m_chunkSize;
    if (nullptr != m_statusCallback) {
      m_statusCallback((100 * chunk) / nChunks, "" /*vscp_str_format("blk %d.", blk).c_str()*/);
    }

  } // for

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// deviceLoad
//

int
CBootDevice_VSCP::deviceLoad(std::function<void(int, const char *)> statusCallback, bool bAbortOnFirmwareCodeFail)
{
  int rv;
  vscpEventEx ex;

  uint32_t progress = 0;
  uint32_t addr;
  std::string strStatus;

  uint32_t minAddr;
  uint32_t maxAddr;

  uint32_t nBlocks;

  m_checksum = 0;

  if (nullptr != m_statusCallback) {
    m_statusCallback(0, "Starting firmware download");
  }

  // Clear checksum
  m_checksum = 0;

  // Iterate over memory types
  int pos = 0;
  do {

    if (VSCP_ERROR_SUCCESS !=
        (rv = getMinMaxForRange(memory_range[pos++].beginning, memory_range[pos++].end, &minAddr, &maxAddr))) {
      spdlog::error("writeFirmwareBlock: Failed to get min max range for block {0:X}-{1:X}", minAddr, maxAddr);
      if (nullptr != m_statusCallback) {
        m_statusCallback(-1, vscp_str_format("writeFirmwareBlock: Failed to get min max range for block", rv).c_str());
      }
      return rv;
    }

    // If there is a memory range to work with
    if (maxAddr > minAddr) {

      nBlocks = (maxAddr - minAddr) / m_blockSize;
      // A not completely full packet also count
      if (0 != ((maxAddr - minAddr) % m_blockSize)) {
        nBlocks++;
      }

      uint8_t *pbuf = new uint8_t[m_blockSize];

      // Init the block
      if (VSCP_ERROR_SUCCESS != (rv = fillBlock(pbuf, m_blockSize, minAddr))) {
        spdlog::error("writeFirmwareBlock: Failed to fill code block with data.");
        if (nullptr != m_statusCallback) {
          m_statusCallback(
            -1,
            vscp_str_format("writeFirmwareBlock: Failed to fill code block with data rv=%d", rv).c_str());
        }
        delete[] pbuf;
        return rv;
      }

      // * * * start block * * *

      ex.vscp_class = VSCP_CLASS1_PROTOCOL;
      ex.vscp_type  = VSCP_TYPE_PROTOCOL_START_BLOCK;
      ex.timestamp  = vscp_makeTimeStamp();
      vscp_setEventExDateTimeBlockToNow(&ex);

      cguid node_guid;
      m_stdRegs.getGUID(node_guid);

      ex.sizeData = 8;
      // ex.data[0]  = (nblock >> 24) & 0xff;
      // ex.data[1]  = (nblock >> 16) & 0xff;
      // ex.data[2]  = (nblock >> 8) & 0xff;
      // ex.data[3] = nblock & 0xff;
      ex.data[4] = memory_range[pos].type;

      if (VSCP_ERROR_SUCCESS != (rv = m_pclient->send(ex))) {
        spdlog::error("VSCP bootloader: Failed to send start block transfer event {0}", rv);
        if (nullptr != m_statusCallback) {
          m_statusCallback(-1, "VSCP bootloader: Failed to send start block transfer event");
        }
        return rv;
      }

      // Wait for response on start block transfer event
      if (VSCP_ERROR_SUCCESS !=
          (rv =
             checkResponse(ex, node_guid, VSCP_TYPE_PROTOCOL_START_BLOCK_ACK, VSCP_TYPE_PROTOCOL_START_BLOCK_NACK))) {
        spdlog::error("VSCP bootloader: Negative response from block start request {0}", rv);
        if (nullptr != m_statusCallback) {
          m_statusCallback(-1, "VSCP bootloader: Negative response from block start request");
        }
        return rv;
      }

      while (true) {
        if (VSCP_ERROR_SUCCESS != (rv = writeFirmwareBlock(pbuf))) {
          return rv;
        }
      }
    }
  } while (0xff == memory_range[pos++].type);

  return VSCP_ERROR_TIMEOUT;
}

///////////////////////////////////////////////////////////////////////////////
// deviceReboot
//

int
CBootDevice_VSCP::deviceReboot(void)
{
  int rv;

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// checkResponse
//

int
CBootDevice_VSCP::checkResponse(vscpEventEx &ex,
                                cguid &guid,
                                uint16_t response_event_ack,
                                uint16_t response_event_nack,
                                uint32_t timeout)
{
  int rv;
  time_t tstart, tnow;

  // Get start time
  time(&tstart);

  while (true) {

    // Test for timeout
    time(&tnow);
    if ((unsigned long) (tnow - tstart) > timeout) {
      spdlog::debug("VSCP Bootloader: Timeout.");
      break;
    }

    uint16_t cnt;
    if ((VSCP_ERROR_SUCCESS == m_pclient->getcount(&cnt)) && cnt) {

      rv = m_pclient->receive(ex);

      spdlog::debug("VSCP Bootloader: Received event: class={0} type={1} size={2}",
                    ex.vscp_class,
                    ex.vscp_type,
                    ex.sizeData);

      // Is this a read/write reply from the node?
      if ((VSCP_CLASS1_PROTOCOL == ex.vscp_class) && (response_event_ack == ex.vscp_type) && guid.isSameGUID(ex.GUID)) {
        // ACK response
        spdlog::debug("VSCP Bootloader: ACK recived.");
        if (nullptr != m_statusCallback) {
          m_statusCallback(-1, "VSCP Bootloader:ACK recived");
        }
        return VSCP_ERROR_SUCCESS;
      }

      // Is this a read/write reply from the node?
      if ((VSCP_CLASS1_PROTOCOL == ex.vscp_class) && (response_event_nack == ex.vscp_type) &&
          guid.isSameGUID(ex.GUID)) {
        // NACK response
        spdlog::debug("VSCP Bootloader: NACK recived.");
        if (nullptr != m_statusCallback) {
          m_statusCallback(-1, "VSCP Bootloader: NACK recived.");
        }
        return VSCP_ERROR_NACK;
      }
    }
  } // while

  return VSCP_ERROR_TIMEOUT;
}
