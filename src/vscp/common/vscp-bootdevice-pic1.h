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
// For info see https://ww1.microchip.com/downloads/en/AppNotes/00247a.pdf
// this bootloader expects a slightly moified version of the bootloader
// described but in most aspects it is the same.
//
// Repository for the device bootloader is here
// https://github.com/grodansparadis/vscp_pic1_bootloader
//
// Protocol is describe here
// https://github.com/grodansparadis/vscp_pic1_bootloader/blob/master/CANIO.asm
//

#pragma once

#include "vscp.h"
#include "vscp-bootdevice.h"

#include <string>

class CBootDevice_PIC1 : public CBootDevice {

public:
  /*!
      Constructor

      This is Level I over Level I interface

      @param pdll Pointer to opended CANAL object.
      @param nodeid Nickname/nodeid for node that should be loaded
      with new code.
  */
  CBootDevice_PIC1(CVscpClient *pclient,
                   uint8_t nodeid,
                   std::function<void(int, const char *)> statusCallback = nullptr,
                   uint32_t timeout                                      = REGISTER_DEFAULT_TIMEOUT);

  /*!
      Constructor

      This is Level I over Level II that uses an interface on
      a remote device to communicate with Level I nodes.

      @param pdll Pointer to opended CANAL object.
      @param nodeid Nickname/nodeid for node that should be loaded
      @param guidif GUID for interface.
      with new code.
  */
  CBootDevice_PIC1(CVscpClient *pclient,
                   uint8_t nodeid,
                   cguid &guidif,
                   std::function<void(int, const char *)> statusCallback = nullptr,
                   uint32_t timeout                                      = REGISTER_DEFAULT_TIMEOUT);

  /*!
      Constructor

      This is a full level II device. No interface, full GUID.

      @param ptcpip Pointer to opened TCP/IP interface object.
      @param guid GUID for node to bootload.
  */
  CBootDevice_PIC1(CVscpClient *pclient,
                   cguid &guid,
                   std::function<void(int, const char *)> statusCallback = nullptr,
                   uint32_t timeout                                      = REGISTER_DEFAULT_TIMEOUT);

  // Dtor
  virtual ~CBootDevice_PIC1(void);

  // Used VSCP commands
  static const uint8_t VSCP_PIC1_READ_REGISTER        = 0x09;
  static const uint8_t VSCP_PIC1_RW_RESPONSE          = 0x0A;
  static const uint8_t VSCP_PIC1_WRITE_REGISTER       = 0x0B;
  static const uint8_t VSCP_PIC1_ENTER_BOOTLODER_MODE = 0x0C;

  // Flash memory
  static const uint32_t MEM_CODE_START = 0x000000;
  static const uint32_t MEM_CODE_END   = 0x1fffff;

  /*
    User IDs (starting at address 200000h) are considered
    to be part of Program Memory and are written and
    erased like normal FLASH Program Memory.
  */
  static const uint32_t MEM_USERID_START = 0x200000;
  static const uint32_t MEM_USERID_END   = 0x2fffff;

  // Configuration memory
  static const uint32_t MEM_CONFIG_START = 0x300000;
  static const uint32_t MEM_CONFIG_END   = 0x3fffff;

  // EEPROM memory
  static const uint32_t MEM_EEPROM_START = 0xf00000;
  static const uint32_t MEM_EEPROM_END   = 0xffffff;

  static const uint32_t BUFFER_SIZE_CODE   = 0x200000; // Max 2M
  static const uint32_t BUFFER_SIZE_USERID = 0x200;
  static const uint32_t BUFFER_SIZE_CONFIG = 0x200;
  static const uint32_t BUFFER_SIZE_EEPROM = 0x400;

  // Memory Type
  enum mem_type {
    MEM_TYPE_CODE = 0x00, // < 0x200000
    MEM_TYPE_CONFIG,      // 0x300000
    MEM_TYPE_EEPROM,      // 0xF00000
    MEM_TYPE_USERID       // 0x200000
  };

  

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
      @return String that will receive information.
  */
  std::string deviceInfo(void);

  /*!
      Initialize boot mode, ie set device in bootmode
      @param outguid Our local GUID
      @param devicecode Device code user expect device tro have
      @param bAbortOnFirmwareCodeFail Set to true to fail if firmware code fetched from
        MDF is not the same as the one read from the remote device.
      @return VSCP_ERROR_SUCCESS on success.
  */
  int deviceInit(cguid& ourguid, uint8_t devicecode, bool bAbortOnFirmwareCodeFail = false);

  /*!
    Write a boot block to the device
    @param start Logic memory start
    @param end Logic memory end
    @return VSCP_ERROR_SUCCESS if OK.
  */
  int writeFirmwareBlock(uint32_t start, uint32_t end);

  /*!
      Perform the actual firmware load process
      @param statusCallback Callback that receive status info as  presentage (int)
          and status message (const char *)
      @return VSCP_ERROR_SUCCESS on success.
  */
  int deviceLoad(std::function<void(int, const char *)> statusCallback = nullptr);

  /*!
    Restart remote device
    @return VSCP_ERROR_SUCCESS is returned on success, ortherwise error code
  */
  int deviceRestart(void);

  /*!
    Reboot remote device
    @return VSCP_ERROR_SUCCESS is returned on success, ortherwise error code
  */
  int deviceReboot(void);

  /*!
      Write a sector
      @param paddr Pointer to firts byte of 8-byte block to write
        to remote device
      @return VSCP_ERROR_SUCCESS on success.
  */
  int writeFirmwareSector(uint8_t *paddr);

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

  /// Code checksum
  uint16_t m_checksum;

  /// Internal address pointer
  uint32_t m_pAddr;
};
