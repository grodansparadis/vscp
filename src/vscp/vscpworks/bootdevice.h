/////////////////////////////////////////////////////////////////////////////
// Name:        bootdevice.h
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     16/12/2009 22:26:09
// RCS-ID:      
// Copyright:   (C) 2012-2018 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
// Licence:     
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
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
//  Grodans Paradis AB at info@grodansparadis.com, http://www.grodansparadis.com
//

#pragma once

/*!
    This is the base class for the VSCP Works wizard 
    firmware load functionality. The code relies on code 
    in the VSCP main file to read/write to a remote  device.
*/

#include "mdf.h"
#include "guid.h"

#include "vscpworks.h"

// Forward references
class wxHtmlWindow;

// Timeout for response
#define BOOT_COMMAND_DEFAULT_RESPONSE_TIMEOUT   5000

// Hexfiles type
#define HEXFILE_TYPE_INTEL_HEX8                 0
#define HEXFILE_TYPE_INTEL_HEX16                1
#define HEXFILE_TYPE_INTEL_HEX32                2
#define HEXFILE_TYPE_INTEL_SRECORD              3

class CBootDevice
{
public:

    /*! 
        Constructor

        @param pdll Pointer to opended CANAL object.
        @param nodeid Nickname/nodeid for node that should be loaded
                        with new code.
        @param bDeviceFound True if VSCP device previously has ben found.
    */
    CBootDevice( CDllWrapper *pdll, uint8_t nodeid, bool bDeviceFound = true );

    /*!
        Constructor 

        @param ptcpip Pointer to opened TCP/IP interface object.
        @param guid GUID for node to bootload.
        @param ifguid GUID for interface node is located on
        @param bDeviceFound True if VSCP device previously has ben found.
    */
    CBootDevice( VscpRemoteTcpIf *ptcpip,
                    cguid &guid,
                    cguid &ifguid,
                    bool bDeviceFound = true );

    /*!
        Destructor
    */
     virtual ~CBootDevice(void);

     /*!
        Init data
     */
     void init( void );

    /*!
        Load a binary file to the image

        This is typically an Intel HEX file that contains the memory
        image of the device.

        @param path Path to file
        @param typecode A typecode for the file to load. Typical
            Intel HEX8, HEX6, HEX32, SRECORD etc
        @return true on success 
    */
    virtual bool loadBinaryFile( const wxString& path, uint16_t type ) = 0;

    /*!
        Show info for hex file
        @param Pointer to HTML window that will receive information.
    */
    virtual void showInfo( wxHtmlWindow *phtmlWnd ) = 0;

    /*!
        Set a device in bootmode

        @return true on success.
    */
    virtual bool setDeviceInBootMode( void ) = 0;

    /*!
        Perform the actual boot process
        @return true on success.
    */
    virtual bool doFirmwareLoad( void ) = 0;

protected:

    /// Type of interface
    uint8_t m_type;

    /// node id for a CANAL node
    uint8_t m_nodeid;

    /*!
        Pointer to DLL communication object (Expected to be iopen)
    */
    CDllWrapper *m_pdll;

    /*!
        GUID for tcp/ip node
    */
    cguid m_guid;

    /*!
        GUID for remote interface
    */
    cguid m_ifguid;

    /*!
        Pointer to TCP/IP communication object (Expected to be open)
    */
    VscpRemoteTcpIf *m_ptcpip;

    /*! 
        Set if VSCP Device found. If set to false then the VSCP daevice is
        not found but can still be a device without firmware that can be 
        loaded.
    */
    bool m_bDeviceFound;

    /// Checksum for firmware
    uint32_t m_checksum;

     /*!
        Flag for flash memory programming or not
    */
    bool m_bFlashMemory;
    
    /*!
        Flag for UserID memory programming or not
    */
    bool m_bUserIDMemory;

    /*!
        Flag for config memory programming or not
    */
    bool m_bConfigMemory;

    /*!
        Flag for EEPROM memory programming or not
    */
    bool m_bEEPROMMemory;

    /// Program memory buffer <0x200000
    uint8_t *m_pbufPrg;
    
    /// Userid memory buffer 0x200000
    uint8_t *m_pbufUserID;

    /// Config memory buffer 0x300000
    uint8_t *m_pbufCfg;

    /// EEPROM memory buffer 0xF00000
    uint8_t *m_pbufEEPROM;

    /// True if there is at least one program data byte
    bool m_bPrgData;

    /// True if there is at least one UserID data byte
    bool m_bUserIDData;
    
    /// True if there is at least one config data byte
    bool m_bConfigData;

    /// True if there is at least one EEPROM data byte
    bool m_bEEPROMData;
    
    /// Lowest flash address
    unsigned long m_minFlashAddr;

    /// Highest flash address
    unsigned long m_maxFlashAddr;
    
    /// Lowest config address
    unsigned long m_minUserIDAddr;

    /// Highest config address
    unsigned long m_maxUserIDAddr;

    /// Lowest config address
    unsigned long m_minConfigAddr;

    /// Highest config address
    unsigned long m_maxConfigAddr;

    /// Lowest EEPROM address
    unsigned long m_minEEPROMAddr;

    /// Highest EEPROM address
    unsigned long m_maxEEPROMAddr;

    /// # data bytes in file
    unsigned long m_totalCntData;

};
