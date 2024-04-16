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

#pragma once

/*!
    This is the base class for the VSCP boot firmware
    load functionality. The code relies on code
    in the VSCP main file to read/write to a remote  device.
*/

#include "guid.h"
#include "mdf.h"
#include "vscp_client_base.h"

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

      @param client VSCP Communication client.
      @param nodeid Nickname/nodeid for node that should be loaded
                      with new code.
  */
  CBootDevice(CVscpClient *pclient, uint16_t nodeid);


  /*!
      Constructor

      @param client VSCP Communication client.
      @param guid GUID for node to bootload.
      @param ifguid GUID for interface node is located on
  */
  CBootDevice(CVscpClient *pclient, cguid &guid, cguid &ifguid);

  /*!
      Destructor
  */
  virtual ~CBootDevice(void);

  // Timeout for response
  static const uint16_t BOOT_COMMAND_DEFAULT_RESPONSE_TIMEOUT = 5000;

    // Hexfiles type
    enum type_hex_file {
      HEXFILE_TYPE_INTEL_HEX8 = 0,
      HEXFILE_TYPE_INTEL_HEX16,
      HEXFILE_TYPE_INTEL_HEX32,
    };


  /*!
      Load a binary file to the image

      This is a Intel HEX file that contains the memory
      image of the device.

      @param path Path to file
      @return true on success
  */
  virtual int loadHexFile(const std::string &path);

  /*!
      Get info for hex file
      @return A string that contain the information for the firmware file.
  */
  virtual std::string getInfo(void);

  /*!
    Set a device in bootmode
    @return true on success.
  */
  virtual bool doDeviceInit(void) = 0;

  /*!
    Perform the actual boot process
    @return true on success.
  */
  virtual bool doDeviceLoad(void) = 0;

  /*!
    Restart the device
    @return true on success.
  */
  virtual bool doDeviceReboot(void) = 0;

protected:

  /*!
    VSCP Client used for communication
  */
  CVscpClient *m_pclient;

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

  /// Lowest flash address
  uint32_t m_minFlashAddr;

  /// Highest flash address
  uint32_t m_maxFlashAddr;

  /// # data bytes in file
  uint32_t m_totalCntData;

  /// List with boot data
  std::list<CBootMemBlock *> m_memblkList;
};
