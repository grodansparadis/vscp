/////////////////////////////////////////////////////////////////////////////
// Name:        bootdevice.h
// Purpose:
// Author:      Ake Hedman
// Modified by:
// Created:     16/12/2009 22:26:09
// RCS-ID:
// Copyright:  (C) 2007-2024
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
// https://en.wikipedia.org/wiki/Intel_HEX
// https://www.kanda.com/blog/microcontrollers/pic-microcontrollers/pic-hex-file-format/
//
// uint32_t timeout                        = REGISTER_DEFAULT_TIMEOUT,
// std::function<void(int)> statusCallback = nullptr

#pragma once

/*!
    This is the base class for the VSCP boot firmware
    load functionality. The code relies on code
    in the VSCP main file to read/write to a remote  device.

    An abstract memory model is used for the firmware. In this
    model memory contents of different types is placed into
    different memory regions. When a block of datra from that region
    is sent to the remote device it is up to that device to
    place the data at the correct location. This model can be used
    for code, ram, eeprom, fuses, config data etc. The pic1 bootloader
    use the Microchip abstract memory model. The VSCP bootloader let
    the receiving end take the decision on where a block should be written

    Sequency of use
    ----------------
    - create class
    - load firmware
    - device Init
    - device Load
*/

#include "guid.h"
#include "mdf.h"
#include "register.h"
#include "vscp-client-base.h"

#include <list>

class CBootMemBlock {

public:
  /*!
    A memory blocks have a 32-bit start address and is
    extending from this address with blksize bytes. The
    block size is usually the same size as intel hex records
    but that is not a requirement
    @param address 32-bit start address for memory block
    @param blksize 8-bit block size
    @param pblkdata If not nullpointer this is a pointer to
      a block with blksize bytes.
  */
  CBootMemBlock(uint32_t address, uint8_t blksize, const uint8_t *pblkdata = nullptr);

  /// @brief  Dtor
  virtual ~CBootMemBlock(void);

  uint32_t getStartAddress(void) { return m_startAddress; };
  void setStartAddress(uint32_t addr) { m_startAddress = addr; };

  uint8_t getSize(void) { return m_blksize; };
  void setSize(uint8_t blksize) { m_blksize = blksize; };

  uint8_t *getBlock(void) { return m_pmemblk; };

private:
  /// The start addres for the memory block
  uint32_t m_startAddress;

  /// Size of block
  uint8_t m_blksize;

  /// Pointer to memory block (max 256 bytes)
  uint8_t *m_pmemblk;
};

class CBootDevice {
public:
  /*!
    Constructor

    This is Level I over Level I interface

    @param client VSCP Communication client.
    @param nodeid Nickname/nodeid for node that should be loaded
                      with new code.
  */
  CBootDevice(CVscpClient *pclient,
              uint16_t nodeid,
              std::function<void(int, const char *)> statusCallback = nullptr,
              uint32_t timeout                                      = REGISTER_DEFAULT_TIMEOUT);

  /*!
    Constructor

    This is Level I over Level II that uses an interface on
      a remote device to communicate with Level I nodes.

    @param client VSCP Communication client.
    @param guidif GUID for interface node is located on
  */
  CBootDevice(CVscpClient *pclient,
              uint16_t nodeid,
              cguid &guidif,
              std::function<void(int, const char *)> statusCallback = nullptr,
              uint32_t timeout                                      = REGISTER_DEFAULT_TIMEOUT);

  /*!
    Constructor

    This is a full level II device. No interface, full GUID.

    @param client VSCP Communication client.
    @param guid GUID for node to bootload.
  */
  CBootDevice(CVscpClient *pclient,
              cguid &guid,
              std::function<void(int, const char *)> statusCallback = nullptr,
              uint32_t timeout                                      = REGISTER_DEFAULT_TIMEOUT);

  /*!
      Destructor
  */
  virtual ~CBootDevice(void);

  /// Timeout for response
  static const uint16_t BOOT_COMMAND_DEFAULT_RESPONSE_TIMEOUT = 5;

  /// Max size for a programming block
  static const uint32_t BOOT_MAX_BLOCK_SIZE = (1024 * 1024 * 2);

  /// Hexfiles type
  enum type_hex_file {
    HEXFILE_TYPE_INTEL_HEX8 = 0,
    HEXFILE_TYPE_INTEL_HEX16,
    HEXFILE_TYPE_INTEL_HEX32,
  };

  static const uint8_t INTELHEX_LINETYPE_DATA             = 0; // Data record.
  static const uint8_t INTELHEX_LINETYPE_EOF              = 1; // End Of File record.
  static const uint8_t INTELHEX_LINETYPE_EXTENDED_SEGMENT = 2; // Extended segment address	record.
  static const uint8_t INTELHEX_LINETYPE_START_SEGMENT    = 3; // Execution  start
  static const uint8_t INTELHEX_LINETYPE_EXTENDED_LINEAR  = 4; // Extended linear address record.
  static const uint8_t INTELHEX_LINETYPE_START_LINEAR     = 5; // Execution start for 32-bit machines
  /*!
      Load a binary file to the image

      This is a Intel HEX file that contains the memory
      image of the device. All types of intel hex is support
      8/16/32

      @param path Path to file
      @return true on success
  */
  virtual int loadIntelHexFile(const std::string &path);

  /*!
    Get minimum and maximum address in a given memory range. This
    is intended for use when memory areas is used to define different
    types of memory. Such as flash, eeprom, config, ram etc.

    @param start Start address for memory range (inclusive - start is part of address space)
    @param end Stop address for memory range (inclusive - end is part of address space)
    @param pmin Pointer to variable that will get min address
    @param pmax Pointer to variable that will get max address
    @return VSCP_ERROR_SUCCESS on success.
      VSCP_ERROR_INVALID_POINTER if ponters are invalid.
      VSCP_ERROR_INVALID_PARAMETER if end <= start
      VSCP_ERROR_ERROR if no memory defined in the selected range. value of
        pmin/pmax is zero in this case.
  */
  virtual int getMinMaxForRange(uint32_t start, uint32_t end, uint32_t *pmin, uint32_t *pmax);

  /*!
    Fill memory buffer with firmware data
    The memory buffer is an area that hold an image of the memory on a remote 
    device. Typically for one type of memory.
    @param pmem Pointer to the beginning of an allocated memory buffer
    @param size Total size of block
    @param start Logical start address
    @param fill Value to initialize block with- 0xff is default
    @return VSCP_ERROR_SUCCESS or errorcode otherwise.
  */
  int fillMemoryBuffer(uint8_t *pmem, uint32_t size, uint32_t start, uint8_t fill = 0xff);

  /*!
      Get info for hex file in html format
      @return A string that contain the information for the firmware file.
  */
  virtual std::string deviceInfo(void);

  /*!
    Set a device in bootmode
    @param ourguid GUID for local device
    @param devicecode This is the device code expected by the user
    @param bAbortOnFirmwareCodeFail Set to true to fail if firmware code fetched from
        MDF is not the same as the one read from the remote device.
    @return VSCP_ERROR_SUCCESS on success.
  */
  virtual int deviceInit(cguid &ourguid, uint8_t devicecode, bool bAbortOnFirmwareCodeFail = false) = 0;

  /*!
    Perform the actual firmware load process
    @param statusCallback Callback that receive status info as  presentage (int)
          and status message (const char *)
    @return VSCP_ERROR_SUCCESS on success.
  */
  virtual int deviceLoad(std::function<void(int, const char *)> statusCallback = nullptr) = 0;

  /*!
    Restart the device
    @return VSCP_ERROR_SUCCESS on success.
  */
  virtual int deviceReboot(void) = 0;

protected:
  /*!
    VSCP Client used for communication
  */
  CVscpClient *m_pclient;

  /*!
    Tomeout for read operations
  */
  uint32_t m_timeout;

  /*!
    Status callback
      xxxxx(int, const char *)
      int is percentage,
      str is real text description
  */
  std::function<void(int, const char *)> m_statusCallback;

  /*!
    The device code tell the type of hardware of the remote device
    Must be the same as the firmware we try to load is intended for.

    Some (old) devices will have this code set to 0 in this cae no check
    should be performed.
  */
  uint8_t m_firmwaredeviceCode;

  /// Our local GUID
  cguid m_ourguid;

  /// node id for device
  uint8_t m_nodeid;

  /*!
      GUID for node (if used)
  */
  cguid m_guid;

  /*!
      GUID for interface (if one is used)
  */
  cguid m_guidif;

  /// Checksum for firmware
  uint32_t m_checksum;

  // This is the minimum code address (startvector)
  uint32_t m_startAddr;

  /// Standard registers
  CStandardRegisters m_stdRegs;

  /// List with boot data
  std::list<CBootMemBlock *> m_memblkList;
};
