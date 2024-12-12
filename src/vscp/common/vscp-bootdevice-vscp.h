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

#pragma once
#include "vscp-bootdevice.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vscp.h"

/* This macro construct a signed integer from two unsigned chars in a safe way */
#if !defined(construct_signed16)
#define construct_signed16(msb, lsb) ((int16_t) ((((uint16_t) msb) << 8) + (uint16_t) lsb))
#endif

/* This macro construct a unsigned integer from two unsigned chars in a safe way */
#if !defined(construct_unsigned16)
#define construct_unsigned16(msb, lsb) ((uint16_t) ((((uint16_t) msb) << 8) + (uint16_t) lsb))
#endif

/* This macro construct a signed long from four unsigned chars in a safe way */
#if !defined(construct_signed32)
#define construct_signed32(b0, b1, b2, b3)                                                                             \
  ((int32_t) ((((uint32_t) b0) << 24) + (((uint32_t) b1) << 16) + (((uint32_t) b2) << 8) + (uint32_t) b3))
#endif

/* This macro construct a unsigned long from four unsigned chars in a safe way */
#if !defined(construct_unsigned32)
#define construct_unsigned32(b0, b1, b2, b3)                                                                           \
  ((uint32_t) ((((uint32_t) b0) << 24) + (((uint32_t) b1) << 16) + (((uint32_t) b2) << 8) + (uint32_t) b3))
#endif

class CBootDevice_VSCP : public CBootDevice {
public:
  /*!
      Constructor

      @param pclient Pointer to active client object.
      @param nodeid Nickname/nodeid for node that should be loaded
        with new code.
      @param statusCallback Status callback function or NULL
      @param timeout Timeout for response
  */
  CBootDevice_VSCP(CVscpClient *pclient,
                   uint8_t nodeid,
                   std::function<void(int, const char *)> statusCallback = nullptr,
                   uint32_t timeout                                      = REGISTER_DEFAULT_TIMEOUT);

  /*!
    Constructor

    This is Level I over Level II that uses an interface on
    a remote device to communicate with Level I nodes.

    @param pclient Pointer to active client object.
    @param nodeid Nickname/nodeid for node that should be loaded
    @param guidif GUID for interface.
    @param statusCallback Status callback function or NULL
    @param timeout Timeout for response
*/
  CBootDevice_VSCP(CVscpClient *pclient,
                   uint8_t nodeid,
                   cguid &guidif,
                   std::function<void(int, const char *)> statusCallback = nullptr,
                   uint32_t timeout                                      = REGISTER_DEFAULT_TIMEOUT);

  /*!
      Constructor

      This is a full level II device. No interface, full GUID.

      @param pclient Pointer to active client object.
      @param guid GUID for node to bootload.
      @param statusCallback Status callback function or NULL
      @param timeout Timeout for response
  */
  CBootDevice_VSCP(CVscpClient *pclient,
                   cguid &guid,
                   std::function<void(int, const char *)> statusCallback = nullptr,
                   uint32_t timeout                                      = REGISTER_DEFAULT_TIMEOUT);

  /// Dtor
  virtual ~CBootDevice_VSCP(void);

  // Memory Types
  typedef enum mem_type {
    MEM_TYPE_FLASH      = 0x00, // 0x00000000 - 0x3FFFFFFF 1G
    MEM_TYPE_RAM        = 0x03, // 0x40000000 - 0xBFFFFFFF 2G
    MEM_TYPE_USERID     = 0x04, // 0xC1000000 - 0xC1FFFFFF 16M
    MEM_TYPE_CONFIG     = 0x02, // 0xC2000000 - 0xC2FFFFFF 16M
    MEM_TYPE_EEPROM     = 0x01, // 0xC3000000 - 0xC3FFFFFF 16M
    MEM_TYPE_BOOTLOADER = 0x06, // 0xC4000000 - 0xC4FFFFFF 16M
    MEM_TYPE_FUSES      = 0x05, // 0xC5000000 - 0xC5FFFFFF 16M
    MEM_TYPE_USER1      = 0xFD, // 0xD0000000 - 0xDFFFFFFF 256M
    MEM_TYPE_USER2      = 0xFE, // 0xE0000000 - 0xEFFFFFFF 256M
    MEM_TYPE_USER3      = 0xFF, // 0xF0000000 - 0xFFFFFFFF 256M
  } mem_type_t;

  // Flash memory
  static const uint32_t MEM_CODE_START = 0x00000000;
  static const uint32_t MEM_CODE_END   = 0x3FFFFFFF;

  // Ram memory
  static const uint32_t MEM_RAM_START = 0x40000000;
  static const uint32_t MEM_RAM_END   = 0xBFFFFFFF;

  // User id memory
  static const uint32_t MEM_USERID_START = 0xC1000000;
  static const uint32_t MEM_USERID_END   = 0xC1FFFFFF;

  // Config memory
  static const uint32_t MEM_CONFIG_START = 0xC2000000;
  static const uint32_t MEM_CONFIG_END   = 0xC2FFFFFF;

  // EEPROM memory
  static const uint32_t MEM_EEPROM_START = 0xC3000000;
  static const uint32_t MEM_EEPROM_END   = 0xC3FFFFFF;

  // BOOTLOADER memory
  static const uint32_t MEM_BOOTLOADER_START = 0xC4000000;
  static const uint32_t MEM_BOOTLOADER_END   = 0xC4FFFFFF;

  // FUSES memory
  static const uint32_t MEM_FUSES_START = 0xC5000000;
  static const uint32_t MEM_FUSES_END   = 0xC5FFFFFF;

  // User defined memory 0
  static const uint32_t MEM_USER0_START = 0xD0000000;
  static const uint32_t MEM_USER0_END   = 0xDFFFFFFF;

  // User defined memory 1
  static const uint32_t MEM_USER1_START = 0xE0000000;
  static const uint32_t MEM_USER1_END   = 0xEFFFFFFF;

  // User defined memory 2
  static const uint32_t MEM_USER2_START = 0xF0000000;
  static const uint32_t MEM_USER2_END   = 0xFFFFFFFF;

  // Default timeout for response
  static const uint8_t BOOT_COMMAND_RESPONSE_TIMEOUT = 5;

  // Number of memory types
  static const uint8_t NUMBER_OF_MEMORY_TYPES = 10;

  /* clang-format off */
  struct memory_range {
    uint8_t type;
    uint32_t start;
    uint32_t end;
  } m_memory_range[NUMBER_OF_MEMORY_TYPES] = {
    { 0x00, MEM_CODE_START, MEM_CODE_END },     
    { 0x01, MEM_EEPROM_START, MEM_EEPROM_END },
    { 0x02, MEM_CONFIG_START, MEM_CONFIG_END }, 
    { 0x03, MEM_RAM_START, MEM_RAM_END },
    { 0x04, MEM_USERID_START, MEM_USERID_END }, 
    { 0x05, MEM_BOOTLOADER_START, MEM_BOOTLOADER_END },
    { 0x06, MEM_FUSES_START, MEM_FUSES_END },   
    { 0xfd, MEM_USER0_START, MEM_USER0_END },
    { 0xfe, MEM_USER1_START, MEM_USER1_END },   
    { 0xff, MEM_USER2_START, MEM_USER2_END }
  };
  /* clang-format on */

  // Initialize data
  void init(void);

  /*!
      Show info for hex file
      @return String that will receive information.
  */
  std::string deviceInfo(void);

  /*!
      Initialize boot mode, ie set device in bootmode
      @param ourguid Our local GUID
      @param devicecode This is the devicecode expected by the user
      @param bAbortOnFirmwareCodeFail Set to true to fail if firmware code fetched from
        MDF is not the same as the one read from the remote device.
      @return VSCP_ERROR_SUCCESS on success.
  */
  int deviceInit(cguid &ourguid, uint16_t devicecode, bool bAbortOnFirmwareCodeFail = false);

  /*!
      Perform the actual firmware load process
      All memory areas are loaded to the device
      @param statusCallback Callback that receive status info as  presentage (int)
          and status message (const char *)
      @param bAbortOnFirmwareCodeFail If firmware device code stored in standard registers
        is different then  the one we try to load, abort if true.
      @return VSCP_ERROR_SUCCESS on success.
  */
  int deviceLoad(std::function<void(int, const char *)> statusCallback = nullptr, bool bAbortOnFirmwareCodeFail = true);

  /*!
    Restart remote device
    @return VSCP_ERROR_SUCCESS is returned on success, otherwise error code
  */
  int deviceRestart(void);

  /*!
    Reboot remote device
    @return VSCP_ERROR_SUCCESS is returned on success, otherwise error code
  */
  int deviceReboot(void);

  /*!
    Write block start to remote device
    @param block Block number to write
    @param type Memory type code for memory to write
    @return VSCP_ERROR_SUCCESS on success.
  */
  int writeBlockStart(uint32_t block, uint8_t type);

  /*!
    Program block on remote device
    @param block Block number to write
    @return VSCP_ERROR_SUCCESS on success.
  */
  int programBlock(uint32_t block);

  /*!
     Write a sector
     @param paddr Pointer to first byte of 8-byte block to write
       to remote device
     @param size Number of data bytes to send. Must be less than the
       max event data (8/512)
     @return VSCP_ERROR_SUCCESS on success.
 */
  int writeChunk(const uint8_t *paddr, uint16_t size);

  /*!
    Write a firmware flock to the device
    @param paddr Address to beginning of data to write
    @return VSCP_ERROR_SUCCESS on success.
  */
  int writeBlock(const uint8_t *paddr);

  /*!
    Check response event
    @param ex Event that is the result of the response
    @param guid GUID for our device the expected event should originate from.
    @param response_event_ack ACK event type.
    @param response_event_nack  NACK event type.
    @return VSCP_ERROR_SUCCESS if ack is received within the timeout time. VSCP_ERROR_NACK if NACK
      event is received. VSCP_ERROR_TIMEOUT if timeout occurred.
  */
  int checkResponse(vscpEventEx &ex,
                    cguid &guid,
                    uint16_t response_event_ack,
                    uint16_t response_event_nack,
                    uint32_t timeout = BOOT_COMMAND_DEFAULT_RESPONSE_TIMEOUT);

private:
  /// Current set page in register space on remote device
  uint32_t m_page;

  /// Our GUID
  cguid m_ourguid;

  /*!
    Current block number
    This is the current block we are writing
  */
  // uint32_t m_blockNumber;

  /*!
    Internal address pointer
    This is the next address to write
  */
  uint32_t m_pAddr;

  /*!
    Chunk size in bytes
    -------------------
    This is the (max) size for each part sent over the wire
    to transfer a block from a host and a master. It can have
    two values 8  (Level I) or 512 (Level II).

    Special cases
    -------------
    The last chunk may have a size less than 8/512 or
    it holds 8/512 bytes. It's up to the remote device to handle both
    cases.

    block size can be less then chunk size. This is especially true
    for level II
  */
  uint16_t m_chunkSize;

  /*!
    Block size in bytes
    -------------------
    This is the block size the remote device expect us to send before
    programming a block.
    Programming is done in chunks of 8 or 512 bytes or less until a full
    block has been written.
  */
  uint32_t m_blockSize;

  /*!
    Number of blocks
    ----------------
    This is the number of blocks available at the remote device. We can not program more than
    this amount of blocks but it's OK to program less.
   */
  uint32_t m_numBlocks;

  /*!
    CRC for full programming data.
  */
  uint16_t m_crc16;
};
