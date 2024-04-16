// bootdevice_pic1.h: PIC algorithm 1 Bootloader code.
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
// For info see http://ww1.microchip.com/downloads/en/AppNotes/00247a.pdf
// this bootloader expects a slightly moified version of the bootloader
// described but in most aspects it is the same.
// Repository for the bootloade is here
// https://github.com/grodansparadis/vscp_pic1_bootloader
//
// Protocol is describe here
// https://github.com/grodansparadis/vscp_pic1_bootloader/blob/master/CANIO.asm
//

#pragma once
#include "vscp_bootdevice.h"

#include <stdio.h>
#include <stdlib.h>

#include "vscp.h"

#include <string>

// Used VSCP commands
#define VSCP_READ_REGISTER        0x09
#define VSCP_RW_RESPONSE          0x0A
#define VSCP_WRITE_REGISTER       0x0B
#define VSCP_ENTER_BOOTLODER_MODE 0x0C

// Used VSCP registers
#define VSCP_REG_PAGE_SELECT_MSB 0x92
#define VSCP_REG_PAGE_SELECT_LSB 0x93
#define VSCP_REG_GUID0           0xD0
#define VSCP_REG_GUID3           0xD3
#define VSCP_REG_GUID5           0xD5
#define VSCP_REG_GUID7           0xD7

#define PIC_BOOTLOADER_RESPONSE_TIMEOUT 5

typedef struct _bootclientItem {
  unsigned char m_nickname;      // Nickname for node
  unsigned char m_bootalgorithm; // Bootloader algorithm to use
  unsigned char m_pageMSB;       // MSB of current page
  unsigned char m_pageLSB;       // LSB of current page
  unsigned char m_GUID0;         // GUID byte 0
  unsigned char m_GUID3;         // GUID byte 3
  unsigned char m_GUID5;         // GUID byte 5
  unsigned char m_GUID7;         // GUID byte 7
} bootclientItem;

class CBootDevice_PIC1 : public CBootDevice {

public:
  /*!
      Constructor

      @param pdll Pointer to opended CANAL object.
      @param nodeid Nickname/nodeid for node that should be loaded
      with new code.
  */
  CBootDevice_PIC1(CVscpClient *pclient, uint8_t nodeid);

  /*!
      Constructor

      @param ptcpip Pointer to opened TCP/IP interface object.
      @param guid GUID for node to bootload.
      @param ifguid GUID for interface node is located on
  */
  CBootDevice_PIC1(CVscpClient *pclient, cguid &guid, cguid &ifguid);

  // Dtor
  ~CBootDevice_PIC1(void);

  static const uint32_t MEMREG_PRG_START = 0x000000;
  static const uint32_t MEMREG_PRG_END   = 0x2fffff;

  static const uint32_t MEMREG_CONFIG_START = 0x300000;
  static const uint32_t MEMREG_CONFIG_END   = 0x3fffff;

  static const uint32_t MEMREG_EEPROM_START = 0xf00000;
  static const uint32_t MEMREG_EEPROM_END   = 0xffffff;

  static const uint32_t BUFFER_SIZE_CODE = 0x200000; // Max 2M
  static const uint32_t BUFFER_SIZE_USERID  = 0x200;
  static const uint32_t BUFFER_SIZE_CONFIG  = 0x200;
  static const uint32_t BUFFER_SIZE_EEPROM  = 0x400;

  // Memory Type
  enum men_type {
    MEM_TYPE_PROGRAM = 0x00, // < 0x200000
    MEM_TYPE_CONFIG,         // 0x300000
    MEM_TYPE_EEPROM,         // 0xF00000
    MEM_TYPE_USERID          // 0x200000
  };

  // Default timeout for response
  static const uint8_t BOOT_COMMAND_RESPONSE_TIMEOUT = 5;

  // flags
  // CONTROL is defined as follows
  //
  // Bit 0:   MODE_WRT_UNLCK      Set this to allow write and erase to memory.
  // Bit 1:   MODE_ERASE_ONLY     Set this to only erase program memory on a put
  // command.
  //                              Must be on a 64-bit boundary.
  // Bit 2:   MODE_AUTO_ERASE     Set this to automatically erase program memory
  // while writing
  //                              data.
  // Bit 3:   MODE_AUTO_INC       Set this to automatically increment the pointer
  // after a write. Bit 4:   MODE_ACK            Set to get acknowledge. Bit 5:
  // undefined. Bit 6:   undefined. Bit 7:   undefined.

  static const uint8_t MODE_WRT_UNLCK  = 0x01;
  static const uint8_t MODE_ERASE_ONLY = 0x02;
  static const uint8_t MODE_AUTO_ERASE = 0x04;
  static const uint8_t MODE_AUTO_INC   = 0x08;
  static const uint8_t MODE_ACK        = 0x10;

  // Boot Commands
  static const uint8_t CMD_NOP       = 0x00; // No operation - Do nothing
  static const uint8_t CMD_RESET     = 0x01; // Reset the device.
  static const uint8_t CMD_RST_CHKSM = 0x02; // Reset the checksum counter and verify.
  static const uint8_t CMD_CHK_RUN   = 0x03; // Add checksum to CMD_DATA_LOW and
                                             // CMD_DATA_HIGH, if verify and zero checksum
                                             // then clear the last location of EEDATA.

  // CAN message ID commands
  static const uint32_t ID_PUT_BASE_INFO = 0x00001000; // Write address information.
  static const uint32_t ID_PUT_DATA      = 0x00001100; // Write data block.
  static const uint32_t ID_GET_BASE_INFO = 0x00001200; // Get address information.
  static const uint32_t ID_GET_DATA      = 0x00001300; // Get data block.
  
  // CAN message ID Responses
  static const uint32_t ID_RESPONSE_PUT_BASE_INFO = 0x00001400; // Response for put info request.
  static const uint32_t ID_RESPONSE_PUT_DATA      = 0x00001500; // Response for put data request.
  static const uint32_t ID_RESPONSE_GET_BASE_INFO = 0x00001400; // Response for get info request.
  static const uint32_t ID_RESPONSE_GET_DATA      = 0x00001500; // Response for get data request.

  /// Initialize data
  void init(void);

  /*!
      Show info for hex file
      @param Pointer to HTML window that will receive information.
  */
  std::string getInfo(void);

  /*!
      Set a device in bootmode
      @return VSCP_ERROR_SUCCESS on success.
  */
  int setDeviceInBootMode(void);

  /*!
      Perform the actual firmware load process
      @param statusCallback Callback that receive status info as  presentage (int)
          and status message (const char *)
      @return VSCP_ERROR_SUCCESS on success.
  */
  int deviceLoad(std::function<void(int, const char *)> statusCallback = nullptr);

  /*!
      Write a sector
      @return VSCP_ERROR_SUCCESS on success.
  */
  int writeFirmwareSector(void);

  /*!
      Write to device control registry

      @param addr Address to set as start address
      @param flags Control Flags
      @param cmd Boot command
      @param cmdData0 Boot command data byte 0
      @param cmdData1 Boot command data byte 1
      @return VSCP_ERROR_SUCCESS on success.

  */
  int writeDeviceControlRegs(uint32_t addr,
                              uint8_t flags    = (MODE_WRT_UNLCK | MODE_AUTO_ERASE | MODE_AUTO_INC | MODE_ACK),
                              uint8_t cmd      = CMD_NOP,
                              uint8_t cmdData0 = 0,
                              uint8_t cmdData1 = 0);

  /*!
      Check for response from nodes (Level I).

      This routine is used as a check for response from nodes under boot.

      The supplied id is valid from bit 8 and upwards. The lower eight bits
      are the id.
      Only extended messages are accepted as a valid response.

      @param id Response code to look for.
      @return VSCP_ERROR_SUCCESS on success.
  */
  int checkResponseLevel1(uint32_t id);

  /*!
      Check for response from nodes over server (Level II).

      This routine is used as a check for response from nodes under boot.

      The supplied id is valid from bit 8 and upwards. The lower eight bits
      are the id.
      Only extended messages are accepted as a valid response.

      @param id Response code to look for.
      @return VSCP_ERROR_SUCCESS on success.
  */
  int checkResponseLevel2(uint32_t id);

private:
  /// Flag for handshake with node
  bool m_bHandshake;

  /// Internal address pointer
  uint32_t m_pAddr;

  /// memory type
  uint8_t m_memtype;

  /// Min and max mem blocks
  uint32_t m_minFlashAddr;
  uint32_t m_maxFlashAddr;
  uint32_t m_minUserIdAddr;
  uint32_t m_maxUserIdAddr;
  uint32_t m_minConfigAddr;
  uint32_t m_maxConfigAddr;
  uint32_t m_minEEPROMAddr;
  uint32_t m_maxEEPROMAddr;

  // Flags for code in codeblocks (false as default)
  bool m_bCodeMemory;
  bool m_bUserIdMemory;
  bool m_bCfgMemory;
  bool m_bEepromMemory;

  // Pic 1 memory buffers
  uint8_t m_pbufCode[BUFFER_SIZE_CODE];
  uint8_t m_pbufUserID[BUFFER_SIZE_USERID];
  uint8_t m_pbufCfg[BUFFER_SIZE_CONFIG];
  uint8_t m_pbufEprom[BUFFER_SIZE_EEPROM];
};
