/////////////////////////////////////////////////////////////////////////////
// Name:        bootdevice_vscp.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     16/12/2009 22:26:09
// RCS-ID:      
// Copyright:   (C) 2007-2018 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
//              (C) 2012 Dinesh Guleria
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
#include "bootdevice.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "vscp.h"

#include <wx/string.h>

class CBootDevice_VSCP :
    public CBootDevice
{
public:

    /*!
        Constructor

        @param pdll Pointer to opended CANAL object.
        @param nodeid Nickname/nodeid for node that should be loaded
        with new code.
    */
    CBootDevice_VSCP( CDllWrapper *pdll, uint8_t nodeid, bool bDeviceFound = true );

    /*!
        Constructor

        @param ptcpip Pointer to opened TCP/IP interface object.
        @param guid GUID for node to bootload.
        @param ifguid GUID for interface node is located on
    */
    CBootDevice_VSCP( VscpRemoteTcpIf *ptcpip, cguid &guid, cguid &ifguid, bool bDeviceFound = true );

    /// Dtor
    ~CBootDevice_VSCP(void);

    // Initialise data
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
    // sendVSCPCommandStartBlock
    // PageAddress : Page to be programmed
    // This command have no ACK
    */
    bool sendVSCPCommandStartBlock( uint16_t PageAddress );

    /*!
        Send command to node under test nodes (Level I).

        This routine is used to send command from nodes under boot. 
        Index tells which Type & class to send.

        @return true on success.
    */
    bool sendVSCPBootCommand( uint8_t index );

    /*!
        sendVSCPCommandSeqenceLevel1
        This routine is used to check ack & send command from nodes under boot. 
        check response VSCP_TYPE_PROTOCOL_BLOCK_DATA_ACK   --- Check CRC
        send  VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA
        check response VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_ACK
    */
    bool sendVSCPCommandSeqenceLevel1(  void  );

    /*!
        sendVSCPCommandSeqenceLevel1
        This routine is used to check ack & send command from nodes under boot. 
        check response VSCP_TYPE_PROTOCOL_BLOCK_DATA_ACK   --- Check CRC
        send  VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA
        check response VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_ACK
    */
    bool sendVSCPCommandSeqenceLevel2(  void  );

    /*!
        Check for response from nodes (Level I).

        This routine is used as a check for response from nodes under boot. 
        Index tells which Type & class to check
    
        @return true on success.
    */
    bool checkResponseLevel1( uint8_t index );

    /*!
        Check for response from nodes over server (Level II).

        This routine is used as a check for response from nodes under boot. 
        Index tells which Type & class to check

        @return true on success.
    */
    bool checkResponseLevel2( uint8_t index );

private:

    /// Internal address pointer
    uint32_t m_pAddr;
    
    /// Current page
    uint32_t m_page;
    
    /// Current block number
    uint32_t m_blockNumber;
    
    /// Block size in byte
    uint32_t m_blockSize;
    
    /// Number of blocks
    uint32_t m_numBlocks;

    uint16_t crc_16_remote;
    uint16_t crc_16_host;
};
