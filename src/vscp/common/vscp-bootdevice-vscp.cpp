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

#include "vscp-bootdevice-vscp.h"

#include <stdio.h>

#define CRC_CCITT
#include <crc.h>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <iomanip>
#include <iostream>
#include <sstream>

///////////////////////////////////////////////////////////////////////////////
// CTORS
//

// Level I
CBootDevice_VSCP::CBootDevice_VSCP(CVscpClient *pclient,
                                   uint8_t nodeid,
                                   std::function<void(int, const char *)> statusCallback,
                                   uint32_t timeout)
  : CBootDevice(pclient, nodeid, statusCallback, timeout)
{
  m_chunkSize = 8;
  init();
}

// Level I over interface
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

// Level II
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
CBootDevice_VSCP::deviceInit(cguid &ourguid, uint16_t devicecode, bool bAbortOnFirmwareCodeFail)
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
    spdlog::warn("Firmware device code is not equal! Firmware: {0} Device: {1} (disable requirement in configuration)",
                 m_firmwaredeviceCode,
                 m_stdRegs.getFirmwareDeviceCode());
    if (nullptr != m_statusCallback) {
      m_statusCallback(
        -1,
        vscp_str_format(
          "Firmware device code is not equal! Firmware: {0} Device: {1} (disable requirement in configuration)",
          m_firmwaredeviceCode,
          m_stdRegs.getFirmwareDeviceCode())
          .c_str());
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
  ex.data[0]  = (uint8_t) m_nodeid;   // Nickname to read register from
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
    spdlog::debug("VSCP bootloader: NACK received from set bootloader request.");
    if (nullptr != m_statusCallback) {
      m_statusCallback(-1, "VSCP bootloader: NACK received from set bootloader request.");
    }
    return rv;
  }

  // OK now in bootmode - return if valid block data
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
// writeBlockStart
//

int
CBootDevice_VSCP::writeBlockStart(uint32_t block, uint8_t type)
{
  int rv;
  vscpEventEx ex;

  ex.vscp_class = VSCP_CLASS1_PROTOCOL;
  ex.vscp_type  = VSCP_TYPE_PROTOCOL_START_BLOCK;
  ex.timestamp  = vscp_makeTimeStamp();
  vscp_setEventExDateTimeBlockToNow(&ex);

  cguid node_guid;
  m_stdRegs.getGUID(node_guid);

  ex.sizeData = 6;
  ex.data[0]  = (block >> 24) & 0xff;
  ex.data[1]  = (block >> 16) & 0xff;
  ex.data[2]  = (block >> 8) & 0xff;
  ex.data[3]  = block & 0xff;
  ex.data[4]  = type;
  ex.data[5]  = 0;

  if (VSCP_ERROR_SUCCESS != (rv = m_pclient->send(ex))) {
    spdlog::error("VSCP bootloader: Failed to send start block transfer event {0}", rv);
    if (nullptr != m_statusCallback) {
      m_statusCallback(-1, "VSCP bootloader: Failed to send start block transfer event");
    }
    return rv;
  }



  // Wait for response on start block transfer event
  if (VSCP_ERROR_SUCCESS !=
      (rv = checkResponse(ex, node_guid, VSCP_TYPE_PROTOCOL_START_BLOCK_ACK, VSCP_TYPE_PROTOCOL_START_BLOCK_NACK))) {
    spdlog::error("VSCP bootloader: Negative response from block start request {0}", rv);
    if (nullptr != m_statusCallback) {
      m_statusCallback(-1,
                       vscp_str_format("VSCP bootloader: Negative response from block start request %d", rv).c_str());
    }
    return rv;
  }


  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// programBlock
//

int
CBootDevice_VSCP::programBlock(uint32_t block)
{
  int rv;
  vscpEventEx ex;

  ex.vscp_class = VSCP_CLASS1_PROTOCOL;
  ex.vscp_type  = VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA;
  ex.timestamp  = vscp_makeTimeStamp();
  vscp_setEventExDateTimeBlockToNow(&ex);

  cguid node_guid;
  m_stdRegs.getGUID(node_guid);

  ex.sizeData = 4;
  ex.data[0]  = (block >> 24) & 0xff;
  ex.data[1]  = (block >> 16) & 0xff;
  ex.data[2]  = (block >> 8) & 0xff;
  ex.data[3]  = block & 0xff;

  if (VSCP_ERROR_SUCCESS != (rv = m_pclient->send(ex))) {
    spdlog::error("VSCP bootloader: Failed to send start block transfer event {0}", rv);
    if (nullptr != m_statusCallback) {
      m_statusCallback(-1, "VSCP bootloader: Failed to send start block transfer event");
    }
    return rv;
  }

  // Wait for response on program block data
  if (VSCP_ERROR_SUCCESS != (rv = checkResponse(ex,
                                                node_guid,
                                                VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_ACK,
                                                VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_NACK))) {
    spdlog::error("VSCP bootloader: Negative response from ProgramBlock. {0}", rv);
    if (nullptr != m_statusCallback) {
      m_statusCallback(-1, vscp_str_format("VSCP bootloader: Negative response from ProgramBlock. %d", rv).c_str());
    }
    return rv;
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// writeChunk
//

int
CBootDevice_VSCP::writeChunk(const uint8_t *paddr, uint16_t size)
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
      (rv = checkResponse(ex, node_guid, VSCP_TYPE_PROTOCOL_BLOCK_CHUNK_ACK, VSCP_TYPE_PROTOCOL_BLOCK_CHUNK_NACK))) {
    spdlog::error("VSCP bootloader: Negative response from block chunk send {0}", rv);
    if (nullptr != m_statusCallback) {
      m_statusCallback(-1, vscp_str_format("VSCP bootloader: Negative response from block chunk send", rv).c_str());
    }
    return rv;
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// writeBlock
//

int
CBootDevice_VSCP::writeBlock(const uint8_t *paddr)
{
  int rv = VSCP_ERROR_SUCCESS;
  cguid guid;
  uint32_t nChunks = 1;
  vscpEventEx ex;

  cguid node_guid;
  m_stdRegs.getGUID(node_guid);

  if (m_blockSize > m_chunkSize) {
    nChunks = m_blockSize / m_chunkSize;
    // A not completely full packet also count
    if (0 != (m_blockSize % m_chunkSize)) {
      nChunks++;
    }
  }

  for (uint32_t chunk = 0; chunk < nChunks; chunk++) {

    spdlog::debug("VSCP bootloader: memory chunk on remote device. chunk={0} {1:X} ", chunk, chunk * m_chunkSize);
    if (nullptr != m_statusCallback) {
      m_statusCallback(
        -1 /*(100 * chunk) / nChunks*/,
        // vscp_str_format("VSCP bootloader: Loading memory chunk on remote device. chunk = %d.", chunk).c_str()
        "");
    }

    if (VSCP_ERROR_SUCCESS != (rv = writeChunk(paddr + (chunk * m_chunkSize), m_chunkSize))) {
      spdlog::error("VSCP bootloader: Failed to write chunk to remote device. rv={}", rv);
      if (nullptr != m_statusCallback) {
        m_statusCallback(
          (100 * chunk) / nChunks,
          vscp_str_format("VSCP bootloader: Failed to write chunk %lu/%lu to remote device rv=%d.", chunk, nChunks, rv)
            .c_str());
        spdlog::error("VSCP bootloader: Failed to write chunk {0}/{1} to remote device rv={2}.", chunk, nChunks, rv);
      }
      break;
    }

    //paddr += m_chunkSize;

  } // for

  // Wait for data block ACK/NACK response
  if (VSCP_ERROR_SUCCESS !=
      (rv = checkResponse(ex, node_guid, VSCP_TYPE_PROTOCOL_BLOCK_DATA_ACK, VSCP_TYPE_PROTOCOL_BLOCK_DATA_NACK))) {
    spdlog::error("VSCP bootloader: NACK received for data block {0}", rv);
    if (nullptr != m_statusCallback) {
      m_statusCallback(-1, "VSCP bootloader: NACK received for data block");
    }
  }

  // Check that the crc is equal
  uint16_t crc_block    = crcFast(paddr, m_blockSize);
  uint16_t crc_packet   = construct_unsigned16(ex.data[0], ex.data[1]);
  uint32_t block_packet = construct_unsigned32(ex.data[2], ex.data[3], ex.data[4], ex.data[5]);

  if (crc_packet != crc_block) {
    spdlog::error("VSCP bootloader: CRC error for block. CRC Packet={0:0X} CRC_Block={1:0X}", crc_packet, crc_block);
    if (nullptr != m_statusCallback) {
      m_statusCallback(
        -1,
        vscp_str_format("VSCP bootloader: CRC error for block CRC Packet=%04X CRC_Block=%04X", crc_packet, crc_block)
          .c_str());
    }
    return VSCP_ERROR_INVALID_FRAME; // TODO: Change to CRC error
  }

  // We have a valid block

  return rv;
}

///////////////////////////////////////////////////////////////////////////////
// deviceLoad
//

int
CBootDevice_VSCP::deviceLoad(std::function<void(int, const char *)> statusCallback, bool bAbortOnFirmwareCodeFail)
{
  int rv = VSCP_ERROR_SUCCESS;
  std::string strStatus;

  m_checksum = 0;

  if (nullptr != m_statusCallback) {
    m_statusCallback(0, "Starting firmware download");
  }

  // Clear checksum
  m_checksum = 0;

  // Iterate over memory types
  for (int memtype = 0; memtype < NUMBER_OF_MEMORY_TYPES; memtype++) {

    spdlog::debug("VSCP bootloader: Memory type {0}", m_memory_range[memtype].type);
    if (nullptr != m_statusCallback) {
      m_statusCallback(-1, vscp_str_format("VSCP bootloader: Memory type %d", m_memory_range[memtype].type).c_str());
    }

    uint32_t minAddr;
    uint32_t maxAddr;

    // Fetch min and max address for selected memory type
    if (VSCP_ERROR_SUCCESS !=
        (rv = getMinMaxForRange(m_memory_range[memtype].start, m_memory_range[memtype].end, &minAddr, &maxAddr))) {
      spdlog::error("VSCP bootloader: Failed to get min max range for memory type {0} rv={1}", memtype, rv);
      if (nullptr != m_statusCallback) {
        m_statusCallback(
          -1,
          vscp_str_format("VSCP bootloader: Failed to get min max range for memory type %d. rv=%d", memtype, rv)
            .c_str());
      }
      continue;
    }

    // Check if there is any data to write
    if (maxAddr <= minAddr) {
      spdlog::error("VSCP bootloader: No data to write for memory type {0} block {1:X}-{2:X}.",
                    memtype,
                    m_memory_range[memtype].start,
                    m_memory_range[memtype].end);
      if (nullptr != m_statusCallback) {
        m_statusCallback(-1,
                         vscp_str_format("VSCP bootloader: No data to write for memory type %d block %X-%X.",
                                         memtype,
                                         m_memory_range[memtype].start,
                                         m_memory_range[memtype].end)
                           .c_str());
      }
      continue;
    }

    /*
      The block where the first bytes of the image is located.
    */
    uint32_t startBlock = minAddr / m_blockSize;
    spdlog::debug("VSCP bootloader: Startblock {0}.", startBlock);

    /*
      The number of blocks to transfer count from the first byte to the
      last byte. The lowest address may need to be adjusted to a block
      boundary. There is no need to do this for the upper address
    */
    uint32_t nBlocks = (maxAddr - minAddr) / m_blockSize;

    // A not completely full packet also count
    if (0 != ((maxAddr - minAddr) % m_blockSize)) {
      nBlocks++;
    }

    spdlog::debug("VSCP bootloader: Number of blocks {0}.", nBlocks);

    /*
      Allocate buffer memory for the block
      Some notes to remember (and handle):
       - Blocksize can be less than data size (ChunkSize) for a frame (8/512)
    */
    uint8_t *pbuf = new uint8_t[m_blockSize * nBlocks];
    if (nullptr == pbuf) {
      spdlog::error("VSCP bootloader: Failed to allocate memory for block {0:X}-{1:X}.", minAddr, maxAddr);
      if (nullptr != m_statusCallback) {
        m_statusCallback(
          -1,
          vscp_str_format("VSCP bootloader: Failed to allocate memory for block {0:X}-{1:X}.", minAddr, maxAddr)
            .c_str());
      }
      return VSCP_ERROR_MEMORY;
    }

    // Init the memory image. Fill unused positions with 0xff
    // After fill we have a block with data that is
    // offset memory_range[pos].beginning
    if (VSCP_ERROR_SUCCESS != (rv = fillMemoryBuffer(pbuf,
                                                     m_blockSize * nBlocks,
                                                     m_memory_range[memtype].start,
                                                     m_memory_range[memtype].end,
                                                     startBlock * m_blockSize))) {
      spdlog::error("VSCP bootloader: Failed to fill memory buffer with data.");
      if (nullptr != m_statusCallback) {
        m_statusCallback(
          -1,
          vscp_str_format("VSCP bootloader: Failed to fill memory buffer with with data rv=%d", rv).c_str());
      }
      delete[] pbuf;
      return rv;
    }

    spdlog::debug("VSCP bootloader: Filled memory buffer with with data.");
    m_statusCallback(-1, vscp_str_format("VSCP bootloader: Filled memory buffer with with data.").c_str());

    // Write blocks to remote device

    uint32_t block = 0;
    while (block < nBlocks) {

      spdlog::debug("VSCP bootloader: Start block {0} of {1}.", block, nBlocks);
      if (nullptr != m_statusCallback) {
        m_statusCallback(
          -1,
          vscp_str_format("VSCP bootloader: Start block %lu of %lu", (unsigned long) block, (unsigned long) nBlocks)
            .c_str());
      }

      // ----------------------------------------------------------------------

      int trycnt = 3; // We give block transfer + program three tries for each block
      while (trycnt > 0) {

        /*!
          * * * start block * * *

          Startblock is the first block in an area. The block number6
          is given and also memory type. It is possible to skip
          blocks that should not be written.
         */
        if (VSCP_ERROR_SUCCESS != (rv = writeBlockStart(block, m_memory_range[memtype].type))) {
          spdlog::error("VSCP bootloader: Failed to write block {0}, memory-type={1}, rv={3}.", block, memtype, rv);
          if (nullptr != m_statusCallback) {
            m_statusCallback(-1,
                             vscp_str_format("VSCP bootloader: Failed to write block %lu, memory-type=%d, rv=%d",
                                             (unsigned long) block,
                                             memtype,
                                             rv)
                               .c_str());
          }
          trycnt--;
          if (trycnt > 0) {
            spdlog::debug("VSCP bootloader: Retry block start {0}.", block);
            continue;
          }
          // Last try failed
          delete[] pbuf;
          return rv;
        }

        spdlog::debug("VSCP bootloader: Block start succeeded {0}.", block);
        if (nullptr != m_statusCallback) {
          m_statusCallback(
            -1,
            vscp_str_format("VSCP bootloader: Block start succeeded %lu ", (unsigned long) block).c_str());
        }

        // for (int i = 0; i < m_blockSize; i++) {
        //   printf("%02X ", pbuf[block * m_blockSize + i]);
        //   if (i % 8 == 7) {
        //     printf("\n");
        //   }
        // }

        // Write the block
        if (VSCP_ERROR_SUCCESS != (rv = writeBlock(pbuf + (block * m_blockSize)))) {
          spdlog::error("VSCP bootloader: Failed to write block {0}", rv);
          if (nullptr != m_statusCallback) {
            m_statusCallback(
              -1,
              vscp_str_format("VSCP bootloader: Failed to write block %lu rv=%d", (unsigned long) block, rv).c_str());
          }
          trycnt--;
          if (trycnt > 0) {
            spdlog::debug("VSCP bootloader: Retry write block {0}.", block);
            continue;
          }
          // Last try failed
          delete[] pbuf;
          return rv;
        }

        spdlog::debug("VSCP bootloader: Write block succeeded {0}.", block);

        // Program the block
        if (VSCP_ERROR_SUCCESS != (rv = programBlock(block))) {
          spdlog::error("VSCP bootloader: Program block failed. block={0} rv={1}", block, rv);
          if (nullptr != m_statusCallback) {
            m_statusCallback(
              -1,
              vscp_str_format("VSCP bootloader: Program block failed. block=%lu rv=%d", (unsigned long) block, rv)
                .c_str());
          }
          trycnt--;
          if (trycnt > 0) {
            spdlog::debug("VSCP bootloader: Retry block start {0}.", block);
            continue;
          }
          // Last try failed
          delete[] pbuf;
          return rv;
        }

        spdlog::debug("VSCP bootloader: Program block succeeded {0}.", block);
        // Update transfer status
        if (nullptr != m_statusCallback) {
          m_statusCallback(
            (100 * block) / nBlocks,
            vscp_str_format("VSCP bootloader: Program block succeeded %lu ", (unsigned long) block).c_str());
        }
        // Done
        trycnt = 0;

      } // while block

      // ----------------------------------------------------------------------

      // Next block
      block++;

    } // Writing blocks

    delete[] pbuf;

  } // for memory types

  return rv;
}

///////////////////////////////////////////////////////////////////////////////
// deviceReboot
//

int
CBootDevice_VSCP::deviceReboot(void)
{
  // int rv;
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
  int rv = VSCP_ERROR_TIMEOUT;
  time_t tstart, tnow;

  // Get start time
  time(&tstart);

  while (true) {

    // Test for timeout
    time(&tnow);
    if ((unsigned long) (tnow - tstart) > timeout) {
      spdlog::debug("VSCP Bootloader: Timeout.");
      rv = VSCP_ERROR_TIMEOUT;
      break;
    }

    uint16_t cnt;
    if ((VSCP_ERROR_SUCCESS == m_pclient->getcount(&cnt)) && cnt) {

      rv = m_pclient->receive(ex);

      cguid evguid(ex.GUID);

      spdlog::debug("VSCP Bootloader: Received event: class={0} type={1} size={2} ev.guid={3} guid={4}",
                    ex.vscp_class,
                    ex.vscp_type,
                    ex.sizeData,
                    evguid.toString(),
                    guid.toString());

      if (m_pclient->isFullLevel2()) {

        // Is this an ack reply from the node?
        if ((VSCP_CLASS1_PROTOCOL == ex.vscp_class) && (response_event_ack == ex.vscp_type) &&
            guid.isSameGUID(ex.GUID)) {
          // ACK response
          spdlog::debug("VSCP bootloader: Check response: Level II ACK received.");
          if (nullptr != m_statusCallback) {
            m_statusCallback(-1, "VSCP bootloader: Check response: Level II ACK received");
          }
          rv = VSCP_ERROR_SUCCESS;
          break;
        }

        // Is this a nack reply from the node?
        if ((VSCP_CLASS1_PROTOCOL == ex.vscp_class) && (response_event_nack == ex.vscp_type) &&
            guid.isSameGUID(ex.GUID)) {
          // NACK response
          spdlog::error("VSCP bootloader: Check response: Level II NACK received.");
          rv = VSCP_ERROR_NACK;
          break;
        }
      }
      else {

        // Level I - Only LSB byte (nickname id) is of interest

        // ACK received?
        if ((VSCP_CLASS1_PROTOCOL == ex.vscp_class) && (response_event_ack == ex.vscp_type) &&
            guid[15] == ex.GUID[15]) {
          // ACK response
          spdlog::debug("VSCP bootloader: Check response: Level I ACK received. nickname={0} ev.nickname={1}",
                        guid[15],
                        ex.GUID[15]);
          rv = VSCP_ERROR_SUCCESS;
          break;
        }

        // NACK received?
        if ((VSCP_CLASS1_PROTOCOL == ex.vscp_class) && (response_event_nack == ex.vscp_type) &&
            guid[15] == ex.GUID[15]) {
          // ACK response
          spdlog::error("VSCP bootloader: Check response: Level I NACK received. nickname={0} ev.nickname={1}",
                        guid[15],
                        ex.GUID[15]);
          if (nullptr != m_statusCallback) {
            m_statusCallback(-1, "VSCP bootloader: Check response: Level I NACK received");
          }
          rv = VSCP_ERROR_SUCCESS;
          break;
        }
      }
    }
  } // while

  return rv;
}
