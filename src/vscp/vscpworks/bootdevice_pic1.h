// bootdevice_pic1.h: PIC algorithm 1 Bootloader code.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright:   (C) 2007-2018 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
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
// Repository for the bootloade is here https://github.com/grodansparadis/vscp_pic1_bootloader
//
// Protocol is describe here https://github.com/grodansparadis/vscp_pic1_bootloader/blob/master/CANIO.asm
//

#pragma once
#include "bootdevice.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "vscp.h"

#include <wx/string.h>

#define MEMREG_PRG_START                0x000000
#define MEMREG_PRG_END                  0x2fffff

#define MEMREG_CONFIG_START             0x300000
#define MEMREG_CONFIG_END               0x3fffff

#define MEMREG_EEPROM_START             0xf00000
#define MEMREG_EEPROM_END               0xffffff

#define BUFFER_SIZE_PROGRAM             0x200000
#define BUFFER_SIZE_USERID              0x200
#define BUFFER_SIZE_CONFIG              0x200
#define BUFFER_SIZE_EEPROM              0x400

#define INTEL_LINETYPE_DATA             0   // Data record.
#define INTEL_LINETYPE_EOF              1   // End Of File record.
#define INTEL_LINETYPE_EXTENDED_SEGMENT 2   // Extended segment address	record.
#define INTEL_LINETYPE_RESERVED         3   // Not used
#define INTEL_LINETYPE_EXTENDED_LINEAR  4   // Extended linear address record.

#ifndef MAX_PATH
#define MAX_PATH                        512
#endif

// Timeout for response
#define BOOT_COMMAND_RESPONSE_TIMEOUT   5

// flags
// CONTROL is defined as follows
//
// Bit 0:   MODE_WRT_UNLCK      Set this to allow write and erase to memory. 
// Bit 1:   MODE_ERASE_ONLY     Set this to only erase program memory on a put command.
//                              Must be on a 64-bit boundary.
// Bit 2:   MODE_AUTO_ERASE     Set this to automatically erase program memory while writing
//                              data.
// Bit 3:   MODE_AUTO_INC       Set this to automatically increment the pointer after a write.
// Bit 4:   MODE_ACK            Set to get acknowledge.
// Bit 5:   undefined.
// Bit 6:   undefined.
// Bit 7:   undefined.

#define MODE_WRT_UNLCK              0x01
#define MODE_ERASE_ONLY             0x02
#define MODE_AUTO_ERASE             0x04
#define MODE_AUTO_INC               0x08
#define MODE_ACK                    0x10

// Boot Commands
#define CMD_NOP                     0x00    // No operation - Do nothing
#define CMD_RESET                   0x01    // Reset the device.
#define CMD_RST_CHKSM               0x02    // Reset the checksum counter and verify.
#define CMD_CHK_RUN                 0x03    // Add checksum to CMD_DATA_LOW and
                                            // CMD_DATA_HIGH, if verify and zero checksum
                                            // then clear the last location of EEDATA.	

// Memory Type
#define	MEM_TYPE_PROGRAM            0x00    // < 0x200000
#define MEM_TYPE_CONFIG             0x01    // 0x300000
#define MEM_TYPE_EEPROM             0x02    // 0xF00000
#define MEM_TYPE_USERID             0x03    // 0x200000

// CAN message ID's
#define ID_PUT_BASE_INFO            0x00001000  // Write address information.
#define ID_PUT_DATA                 0x00001100  // Write data block.
#define ID_GET_BASE_INFO            0x00001200  // Get address information.
#define ID_GET_DATA                 0x00001300  // Get data block.

#define ID_RESPONSE_PUT_BASE_INFO   0x00001400  // Response for put info request.
#define ID_RESPONSE_PUT_DATA        0x00001500  // Response for put data request.
#define ID_RESPONSE_GET_BASE_INFO   0x00001400  // Response for get info request.
#define ID_RESPONSE_GET_DATA        0x00001500  // Response for get data request.

// USed VSCP commands
#define VSCP_READ_REGISTER                  0x09
#define VSCP_RW_RESPONSE                    0x0A
#define VSCP_WRITE_REGISTER                 0x0B
#define VSCP_ENTER_BOOTLODER_MODE           0x0C

// Used VSCP registers
#define VSCP_REG_PAGE_SELECT_MSB            0x92
#define VSCP_REG_PAGE_SELECT_LSB            0x93
#define VSCP_REG_GUID0                      0xD0
#define VSCP_REG_GUID3                      0xD3
#define VSCP_REG_GUID5                      0xD5
#define VSCP_REG_GUID7                      0xD7

#define PIC_BOOTLOADER_RESPONSE_TIMEOUT     5

typedef struct _bootclientItem {
    unsigned char m_nickname;       // Nickname for node
    unsigned char m_bootalgorithm;  // Bootloader algorithm to use
    unsigned char m_pageMSB;        // MSB of current page
    unsigned char m_pageLSB;        // LSB of current page
    unsigned char m_GUID0;          // GUID byte 0
    unsigned char m_GUID3;          // GUID byte 3
    unsigned char m_GUID5;          // GUID byte 5
    unsigned char m_GUID7;          // GUID byte 7
} bootclientItem;

class CBootDevice_PIC1 : public CBootDevice
{

public:

    /*!
        Constructor

        @param pdll Pointer to opended CANAL object.
        @param nodeid Nickname/nodeid for node that should be loaded
        with new code.
    */
    CBootDevice_PIC1( CDllWrapper *pdll, uint8_t nodeid, bool bDeviceFound = true );

    /*!
        Constructor

        @param ptcpip Pointer to opened TCP/IP interface object.
        @param guid GUID for node to bootload.
        @param ifguid GUID for interface node is located on
    */
    CBootDevice_PIC1( VscpRemoteTcpIf *ptcpip, cguid &guid, cguid &ifguid, bool bDeviceFound = true );

    // Dtor
    ~CBootDevice_PIC1(void);

    /// Initialize data
    void init( void );

    /*!
        Load a binary file to the image

        This is typically an Intel HEX file that contains the memory
        image of the device.

        @param path Path to file
        @return true on success 
    */
    bool loadBinaryFile( const wxString& path, uint16_t typeHexfile );

    /*!
        Show info for hex file
        @param Pointer to HTML window that will receive information.
    */
    void showInfo( wxHtmlWindow *phtmlWnd );


    /*!
        Set a device in bootmode
        @return true on success.
    */
    bool setDeviceInBootMode( void );

    /*!
        Perform the actual boot process
        @return true on success.
    */
    bool doFirmwareLoad( void );

    /*!
        Write a sector
        @return true on success.
    */
    bool writeFrimwareSector( void );

    /*!
        Write to device control registry

        @param addr Address to set as start address
        @param flags Control Flags
        @param cmd Boot command
        @param cmdData0 Boot command data byte 0
        @param cmdData1 Boot command data byte 1

    */
    bool writeDeviceControlRegs( uint32_t addr,
                    uint8_t flags = ( MODE_WRT_UNLCK | MODE_AUTO_ERASE | MODE_AUTO_INC | MODE_ACK ),
                    uint8_t cmd = CMD_NOP,
                    uint8_t cmdData0 = 0,
                    uint8_t cmdData1 = 0 );

    /*!
        Check for response from nodes (Level I).

        This routine is used as a check for response from nodes under boot. 
    
        The supplied id is valid from bit 8 and upwards. The lower eight bits
        are the id.
        Only extended messages are accepted as a valid response.

        @param id Response code to look for.
        @return true on success.
    */
    bool checkResponseLevel1( uint32_t id );

    /*!
        Check for response from nodes over server (Level II).

        This routine is used as a check for response from nodes under boot. 

        The supplied id is valid from bit 8 and upwards. The lower eight bits
        are the id.
        Only extended messages are accepted as a valid response.

        @param id Response code to look for.
        @return true on success.
    */
    bool checkResponseLevel2( uint32_t id );

private:

    /// Flag for handshake with node
    bool m_bHandshake;	

    /// Internal address pointer
    uint32_t m_pAddr;

    /// memory type
    uint8_t m_memtype;

};
