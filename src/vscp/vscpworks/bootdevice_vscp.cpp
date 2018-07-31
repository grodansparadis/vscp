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
///

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "devicebootloaderwizard.h"
#endif

#ifdef WIN32
#include <winsock2.h>
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <stdio.h>
#include <wx/html/htmlwin.h>
#include <wx/progdlg.h>
#include "bootdevice_vscp_defs.h"
#include "bootdevice_vscp.h"

#define CRC16
#include "crc.h"


CBootDevice_VSCP::CBootDevice_VSCP( CDllWrapper *pdll, 
                                    uint8_t nodeid, 
                                    bool bDeviceFound ) :
    CBootDevice( pdll, nodeid, bDeviceFound )
{
    init();
}

CBootDevice_VSCP::CBootDevice_VSCP( VscpRemoteTcpIf *ptcpip, 
                                    cguid &guid, 
                                    cguid &ifguid, 
                                    bool bDeviceFound ) :
    CBootDevice( ptcpip, guid, ifguid, bDeviceFound )
{
    init();
}

CBootDevice_VSCP::~CBootDevice_VSCP( void )
{
    ;
}


///////////////////////////////////////////////////////////////////////////////
// init
//

void CBootDevice_VSCP::init( void )
{
    // Create buffers
    m_pbufPrg = new unsigned char[ BUFFER_SIZE_PROGRAM_COMMON ];

    m_pAddr = 0;

    crcInit();
}

///////////////////////////////////////////////////////////////////////////////
// loadBinaryFile
//

bool CBootDevice_VSCP::loadBinaryFile(const wxString& path, uint16_t type) {
    unsigned long i;
    bool rv = false;
#ifdef WIN32	
    errno_t err;
#else	
    long err;
#endif	
    unsigned long fullAddr  = 0;
    unsigned long highAddr  = 0;
    unsigned long lowAddr   = 0;
    unsigned long cntData   = 0;
    unsigned long recType   = 0;
    
    FILE* fs = NULL;

    // Init. program memory pointers
    m_minFlashAddr  = 0xffffffff;
    m_maxFlashAddr  = 0;
    m_totalCntData  = 0;
    m_bPrgData      = false;
    m_bFlashMemory  = true; // Program memory should be programmed

    // Init program memory buffer
    if (NULL == m_pbufPrg) return false;
    
    memset(m_pbufPrg, 0xff, BUFFER_SIZE_PROGRAM_COMMON);

#ifdef WIN32	
    if (0 != (err = fopen_s(&fs, path.char_str(), "r"))) {
        return false;
    }
#else
    if (NULL == (fs = fopen(path.char_str(), "r"))) {
        return false;
    }
#endif

    char szLine[ MAX_PATH ];
    char szData[ 16 ];
    char *endptr;

    bool bRun = true;

    while ((true == bRun) && (NULL != fgets(szLine, MAX_PATH, fs))) {

        if (':' == szLine [ 0 ]) {

            // Get data count
            memset(szData, 0, 16);
#ifdef WIN32			
            strncpy_s(szData, 16, (szLine + 1), 2);
#else
            strncpy(szData, (szLine + 1), 2);
#endif
            cntData = strtoul(szData, &endptr, 16);
            m_totalCntData += cntData;

            // Get address
            memset(szData, 0, 16);
#ifdef WIN32			
            strncpy_s(szData, 16, (szLine + 3), 4);
#else
            strncpy(szData, (szLine + 3), 4);
#endif
            lowAddr = strtoul(szData, &endptr, 16);

            // Get record type
            memset(szData, 0, 16);
#ifdef WIN32			
            strncpy_s(szData, 16, (szLine + 7), 2);
#else
            strncpy(szData, (szLine + 7), 2);
#endif

            recType = strtoul(szData, &endptr, 16);

            fullAddr = (highAddr * 0xffff) + lowAddr;

            // Decode the record type
            switch (recType) {

                case INTEL_LINETYPE_DATA:

                    for (i = 0; i < cntData; i++) {

                        memset(szData, 0, 16);
#ifdef WIN32						
                        strncpy_s(szData, 16, (szLine + ((i * 2) + 9)), 2);
#else
                        strncpy(szData, (szLine + ((i * 2) + 9)), 2);  
#endif
                        unsigned char val =
                                (unsigned char) (strtoul(szData, &endptr, 16) & 0xff);

                        /* In program memory address space? */
                        if ((fullAddr >= MEMREG_PRG_START_COMMON) &&
                            (fullAddr <= MEMREG_PRG_END_COMMON)) {
                            
                            /* Avoid program memory buffer overflow. */
                            if ((fullAddr - MEMREG_PRG_START_COMMON) < BUFFER_SIZE_PROGRAM_COMMON)
                            {
                                // Write into program memory buffer
                                m_pbufPrg[ fullAddr  - MEMREG_PRG_START_COMMON ] = val;
                                m_bPrgData = true;
                                
                                // Set min flash address
                                if (fullAddr < m_minFlashAddr) m_minFlashAddr = fullAddr;

                                // Set max flash address
                                if (fullAddr > m_maxFlashAddr) m_maxFlashAddr = fullAddr;
                            }
                        }

                        ++fullAddr;
                    }
                    break;

                case INTEL_LINETYPE_EOF:
                    bRun = false; // We are done
                    rv = true;
                    break;

                case INTEL_LINETYPE_EXTENDED_SEGMENT:
                    // We don't handle this
                    break;

                case INTEL_LINETYPE_EXTENDED_LINEAR:
                    memset(szData, 0, 16);
#ifdef WIN32					
                    strncpy_s(szData, 16, (szLine + 9), 4);
#else
                    strncpy(szData, (szLine + 9), 4);
#endif
                    highAddr = strtoul(szData, &endptr, 16);
                    break;

            }

        }
    }

    // Flash to program if none read
    if (!m_bPrgData) {
        m_bFlashMemory = false;
        m_minFlashAddr = 0x00000000;
    }

    fclose(fs);

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// showInfo
//

void CBootDevice_VSCP::showInfo(wxHtmlWindow *phtmlWnd) {
    wxString strInfo;

    // Check pointer
    if (NULL == phtmlWnd) return;

    // Clear HTML
    phtmlWnd->SetPage(_(""));

    // * * * Flash Memory * * *

    phtmlWnd->AppendToPage(
            _("<b><u>Flash Memory</u></b><br>"));

    phtmlWnd->AppendToPage(
            _("<b>Start :</b><font color=\"#005CB9\">"));

    strInfo.Printf(_("0x%08X"), m_minFlashAddr);
    phtmlWnd->AppendToPage(strInfo);

    phtmlWnd->AppendToPage(
            _("</font><b> End :</b><font color=\"#005CB9\">"));

    strInfo.Printf(_("0x%08X</font><br>"), m_maxFlashAddr);
    phtmlWnd->AppendToPage(strInfo);

    if (m_bFlashMemory) {
        phtmlWnd->AppendToPage(
                _("<font color=\"#348017\">Will be programmed</font><br>"));
    } else {
        phtmlWnd->AppendToPage(
                _("<font color=\"#F6358A\">Will not be programmed</font><br>"));
    }

    phtmlWnd->AppendToPage(
            _("<br><br>"));

}


///////////////////////////////////////////////////////////////////////////////
// setDeviceInBootMode
//

bool CBootDevice_VSCP::setDeviceInBootMode( void ) 
{
    bool bRun;

    uint8_t pageSelectMsb = 0;
    uint8_t pageSelectLsb = 0;
    uint8_t guid0 = 0;
    uint8_t guid3 = 0;
    uint8_t guid5 = 0;
    uint8_t guid7 = 0;

    uint16_t vscpclass;
    uint8_t vscptype;
    uint8_t priority = 0;

    wxBusyCursor busy;

    if ( USE_DLL_INTERFACE == m_type ) {

        canalMsg msg, rcvmsg;
        time_t tstart, tnow;

        memset( msg.data, 0x00, 8 );

        // Read page register Page select MSB
        if ( CANAL_ERROR_SUCCESS != 
                m_pdll->readLevel1Register( m_nodeid,
                                                0,
                                                VSCP_REG_PAGE_SELECT_MSB,
                                                &pageSelectMsb ) ) {
            return false;
        }

        // Read page register page select lsb
        if ( CANAL_ERROR_SUCCESS != 
                m_pdll->readLevel1Register( m_nodeid,
                                                0,
                                                VSCP_REG_PAGE_SELECT_LSB,
                                                &pageSelectLsb ) ) {
            return false;
        }
        
        // Read page register GUID0
        if ( CANAL_ERROR_SUCCESS != 
                m_pdll->readLevel1Register( m_nodeid,
                                                0,
                                                VSCP_REG_GUID0,
                                                &guid0 ) ) {
            return false;
        }

        // Read page register GUID3
        if ( CANAL_ERROR_SUCCESS != 
                m_pdll->readLevel1Register( m_nodeid,
                                                0,
                                                VSCP_REG_GUID3,
                                                &guid3 ) ) {
            return false;
        }

        // Read page register GUID5
        if ( CANAL_ERROR_SUCCESS != 
                m_pdll->readLevel1Register( m_nodeid,
                                                0,
                                                VSCP_REG_GUID5,
                                                &guid5 ) ) {
            return false;
        }

        // Read page register GUID7
        if ( CANAL_ERROR_SUCCESS != 
                m_pdll->readLevel1Register( m_nodeid,
                                                0,
                                                VSCP_REG_GUID7,
                                                &guid7 ) ) {
            return false;
        }

        vscpclass   = VSCP_CLASS1_PROTOCOL;
        vscptype    = VSCP_ENTER_BOOTLODER_MODE;
        priority    = VSCP_PRIORITY_LOW_COMMON;
        
        // Set device in boot mode
        msg.data[ 0 ] = m_nodeid;               // Nickname to read register from
        msg.data[ 1 ] = VSCP_BOOTLOADER_VSCP;   // VSCP standard bootloader algorithm	
        msg.data[ 2 ] = guid0;
        msg.data[ 3 ] = guid3;
        msg.data[ 4 ] = guid5;
        msg.data[ 5 ] = guid7;
        msg.data[ 6 ] = pageSelectMsb;
        msg.data[ 7 ] = pageSelectLsb;

        // Send message
        msg.id = ((uint32_t) priority << 26) |
                 ((uint32_t) vscpclass << 16) |
                 ((uint32_t) vscptype << 8) |
                 m_nodeid; // nodeaddress (our address)

        msg.flags = CANAL_IDFLAG_EXTENDED;
        msg.sizeData = 8;
        if ( CANAL_ERROR_SUCCESS == m_pdll->doCmdSend( &msg ) ) {

            bRun = true;

            // Get start time
            time( &tstart );

            while ( bRun ) {

                time(&tnow);
                if ((unsigned long) (tnow - tstart) > BOOT_COMMAND_RESPONSE_TIMEOUT ) {
                    bRun = false;
                }

                if ( m_pdll->doCmdDataAvailable() ) {

                    m_pdll->doCmdReceive( &rcvmsg );

                    vscpclass = VSCP_CLASS1_PROTOCOL;
                    vscptype = VSCP_TYPE_PROTOCOL_ACK_BOOT_LOADER;

                    if ((uint32_t) (rcvmsg.id & 0x01ffffff) == 
                         (uint32_t) (((uint32_t) vscpclass << 16) | 
                            ((uint32_t) vscptype << 8) | m_nodeid ) ) {
                        
                        // OK in bootmode - return
                        m_blockSize = ((uint32_t) rcvmsg.data[ 0 ] << 24) |
                                      ((uint32_t) rcvmsg.data[ 1 ] << 16) |
                                      ((uint32_t) rcvmsg.data[ 2 ] <<  8) |
                                      ((uint32_t) rcvmsg.data[ 3 ] <<  0);

                        m_numBlocks = ((uint32_t) rcvmsg.data[ 4 ] << 24) |
                                      ((uint32_t) rcvmsg.data[ 5 ] << 16) |
                                      ((uint32_t) rcvmsg.data[ 6 ] <<  8) |
                                      ((uint32_t) rcvmsg.data[ 7 ] <<  0);
                                      
                        return true;
                    }

                }
                
            }

        }

    } 
    else if ( USE_TCPIP_INTERFACE == m_type ) {

        vscpEventEx event;
        time_t tstart, tnow;

        // Read page register MSB
        if ( VSCP_ERROR_SUCCESS != 
                m_ptcpip->readLevel2Register( VSCP_REG_PAGE_SELECT_MSB, 
                                                0, 
                                                &pageSelectMsb, 
                                                m_ifguid, 
                                                &m_guid ) ) {
            return false;
        }

        // Read page register LSB
        if ( VSCP_ERROR_SUCCESS != 
                m_ptcpip->readLevel2Register( VSCP_REG_PAGE_SELECT_LSB, 
                                                0, 
                                                &pageSelectLsb, 
                                                m_ifguid, 
                                                &m_guid ) ) {
            return false;
        }

        // Read GUID0
        if ( VSCP_ERROR_SUCCESS != 
                m_ptcpip->readLevel2Register( VSCP_REG_GUID0, 
                                                0, 
                                                &guid0, 
                                                m_ifguid, 
                                                &m_guid ) ) {
            return false;
        }

        // Read GUID3
        if ( VSCP_ERROR_SUCCESS != 
                m_ptcpip->readLevel2Register( VSCP_REG_GUID3, 
                                                0, 
                                                &guid3, 
                                                m_ifguid, 
                                                &m_guid ) ) {
            return false;
        }

        // Read GUID5
        if ( VSCP_ERROR_SUCCESS != 
                m_ptcpip->readLevel2Register( VSCP_REG_GUID5, 
                                                0, 
                                                &guid5, 
                                                m_ifguid, 
                                                &m_guid ) ) {
            return false;
        }

        // Read GUID7
        if ( VSCP_ERROR_SUCCESS != 
                m_ptcpip->readLevel2Register( VSCP_REG_GUID7, 
                                                0, 
                                                &guid7, 
                                                m_ifguid, 
                                                &m_guid ) ) {
            return false;
        }

        // Set device in boot mode

        // Send message
        event.head = 0;
        event.vscp_class = 512;                         // CLASS2.PROTOCOL1
        event.vscp_type = VSCP_ENTER_BOOTLODER_MODE;    // We want to enter bootloader mode
        memset( event.GUID, 0, 16 );                    // We use interface GUID
        event.sizeData = 16 + 8;                        // Interface GUID
        memset( event.data, 0, sizeof( event.data ) );
        memcpy( event.data, m_ifguid.m_id, 16 );        // Address node i/f
        event.data[ 16 ] = m_guid.getLSB();	            // Nickname for device 
        event.data[ 17 ] = VSCP_BOOTLOADER_VSCP;        // VSCP standard bootloader algorithm	
        event.data[ 18 ] = guid0;
        event.data[ 19 ] = guid3;
        event.data[ 20 ] = guid5;
        event.data[ 21 ] = guid7;
        event.data[ 22 ] = pageSelectMsb;
        event.data[ 23 ] = pageSelectLsb;

        if ( VSCP_ERROR_SUCCESS == m_ptcpip->doCmdSendEx( &event ) ) {

            bRun = true;

            // Get start time
            time(&tstart);

            while (bRun) {

                time(&tnow);
                if ((unsigned long) (tnow - tstart) > BOOT_COMMAND_RESPONSE_TIMEOUT) {
                    bRun = false;
                }

                //vscpEventEx rcvmsg;
                if ( m_ptcpip->doCmdDataAvailable() ) {

                    m_ptcpip->doCmdReceiveEx( &event );

                    // Check for response  ---  Type = 13 (0x0D) ACK boot loader mode.
                    if ( VSCP_TYPE_PROTOCOL_ACK_BOOT_LOADER == event.vscp_type ) {
                        // OK in bootmode - return
                        
                        m_blockSize = ((uint32_t) event.data[ 0 ] << 24) |
                                      ((uint32_t) event.data[ 1 ] << 16) |
                                      ((uint32_t) event.data[ 2 ] <<  8) |
                                      ((uint32_t) event.data[ 3 ] <<  0);

                        m_numBlocks = ((uint32_t) event.data[ 4 ] << 24) |
                                      ((uint32_t) event.data[ 5 ] << 16) |
                                      ((uint32_t) event.data[ 6 ] <<  8) |
                                      ((uint32_t) event.data[ 7 ] <<  0);
                        
                        return true;
                    }

                }
            }
        }

    }

    return false;
}


///////////////////////////////////////////////////////////////////////////////
// doFirmwareLoad
//

bool CBootDevice_VSCP::doFirmwareLoad( void )
{
    bool bRun = true;
    bool rv = true;
    bool flag_crc = true;
    
    wxBusyCursor busy;

    m_checksum = 0;
    uint32_t progress = 0;
    uint32_t addr;
    wxString wxStatusStr;

    uint32_t nFlashPackets = 0;
    
    // Packet size is always eight byte due to CAN frame limitation
    
    // Flash memory
    if ( m_bPrgData ) {
        nFlashPackets = (m_maxFlashAddr - m_minFlashAddr) / 8;

        if (0 != ((m_maxFlashAddr - m_minFlashAddr) % 8)) {
            nFlashPackets++;
        }
    }

    long nTotalPackets = nFlashPackets;
    wxProgressDialog* pDlg = 
            new wxProgressDialog( _T("Boot loading in progress..."),
                                    _T("---"),
                                    nTotalPackets,
                                    NULL,
                                    wxPD_AUTO_HIDE |
                                    wxPD_APP_MODAL |
                                    wxPD_CAN_ABORT |
                                    wxPD_ELAPSED_TIME |
                                    wxPD_REMAINING_TIME );

    // Initialize checksum
    addr = m_minFlashAddr;

    // * * * flash memory * * *

    if ( rv && m_bPrgData ) {

        addr = m_minFlashAddr;

        // nFlashPackets  = number of 8 byte packets
        m_blockNumber = 0;
        for ( uint32_t blk = 0; ( ( blk < nFlashPackets ) && ( true == bRun ) ); blk++) {

            // Start block data transfer
            if ( 0 == ( blk  % ( m_blockSize / 8 ) ) ) {
                if ( true != sendVSCPCommandStartBlock( blk * 8 / m_blockSize ) )
                    wxMessageBox( _T("start Block error") );
                    bRun = false;
            }
            
            wxStatusStr.Printf( _("Loading flash... %0X"), addr );
            if (  false == ( bRun = pDlg->Update( progress, wxStatusStr ) ) ) {
                wxMessageBox( _T("Aborted by user.") );
                rv = false;
                bRun = false;
            }
            
            if ( false == writeFrimwareSector() ) {
                wxMessageBox( _T("Failed to write flash data to node(s).") );
                rv = false;
                bRun = false;
            }
            
            /* After a complete block, wait for the block data acknowledge. */
            if ( 0 == ( ( blk + 1 )  % ( m_blockSize / 8 ) ) ) {
            
                if ( USE_DLL_INTERFACE == m_type ) {

                    flag_crc = sendVSCPCommandSeqenceLevel1();
                }
                else if ( USE_TCPIP_INTERFACE == m_type ) {

                    flag_crc = sendVSCPCommandSeqenceLevel2();
                }
                
                m_blockNumber++;
            }

            wxMilliSleep(1);
            progress++;
            addr += 8;
        }
    }

    /*
     ** All blocks loaded -- now reset the device
     */

    if (!sendVSCPBootCommand(VSCP_TYPE_PROTOCOL_ACTIVATE_NEW_IMAGE)) { // send as Zero as at -- present AVR bode for microcontroller -- does not implement this method.
        // Failure

        wxMessageBox(_T(" ACTIVATE_NEW_IMAGE TX fails"));
    } 
    else {

    }

    // Done
    progress = nTotalPackets;
    pDlg->Update(progress, wxStatusStr);

    pDlg->Destroy();

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// writeFrimwareSector
//

bool CBootDevice_VSCP::writeFrimwareSector( void )
{
    canalMsg msg;
    vscpEventEx event;
    bool rv = true;

    uint16_t vscpclass = VSCP_CLASS1_PROTOCOL;
    uint8_t vscptype = VSCP_TYPE_PROTOCOL_BLOCK_DATA;
    uint8_t priority = VSCP_PRIORITY_LOW_COMMON;

    // Send event
    if ( USE_DLL_INTERFACE == m_type ) {
        // Send message
        msg.id = ((uint32_t) priority << 26) |
                ((uint32_t) vscpclass << 16) |
                ((uint32_t) vscptype << 8) |
                m_nodeid;                       // nodeaddress (our address)
        msg.flags = CANAL_IDFLAG_EXTENDED;
        msg.sizeData = 8;
    } 
    else if ( USE_TCPIP_INTERFACE == m_type ) {
        event.head = 0;
        event.vscp_class = 512;                 // CLASS2.PROTOCOL1
        event.vscp_type = vscptype;
        memset(event.GUID, 0, 16);              // We use interface GUID
        event.sizeData = 16 + 8;                // Interface GUID
        memcpy(event.data, m_guid.m_id, 16);    // Address node
    } 
    else {
        return false;
    }

    uint8_t b;
    for (int i = 0; i < 8; i++) {

        b = m_pbufPrg[ m_pAddr ];
        m_checksum += m_pbufPrg[ m_pAddr ];

        // Write data into frame
        if ( USE_DLL_INTERFACE == m_type ) {
            msg.data[ i ] = b;    
        }
        else if ( USE_TCPIP_INTERFACE == m_type ) {
            event.data[ 16 + i] = b;
        }
        else {
            return false;
        }

        // Update address
        m_pAddr++;

    }

    if ( USE_DLL_INTERFACE == m_type ) {
        m_pdll->doCmdSend( &msg );
    } 
    else if ( USE_TCPIP_INTERFACE == m_type ) {
        m_ptcpip->doCmdSendEx(&event);
    }
    else {
        rv = false;
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// sendVSCPCommandStartBlock
// PageAddress : Page to be programmed
// This command have no ACK

bool CBootDevice_VSCP::sendVSCPCommandStartBlock(uint16_t PageAddress) 
{
    uint16_t vscpclass = 0;
    uint8_t vscptype = 0;
    uint8_t priority = 0;

    wxBusyCursor busy;

    if ( USE_DLL_INTERFACE == m_type ) {

        canalMsg msg;

        memset(msg.data, 0x00, 8);

        vscpclass = VSCP_CLASS1_PROTOCOL;           // Class
        vscptype = VSCP_TYPE_PROTOCOL_START_BLOCK;  // Start block data transfer.
        priority = VSCP_PRIORITY_LOW_COMMON;
        // block data transfer
        msg.data[ 0 ] = 0x00;                           // Block number MSB 
        msg.data[ 1 ] = 0x00;                           // Block number
        msg.data[ 2 ] = (PageAddress & 0xFF00) >> 8;    // Block number
        msg.data[ 3 ] = (PageAddress & 0x00FF);         // Block number LSB

        // Send message
        msg.id = ((uint32_t) priority << 26) |
                ((uint32_t) vscpclass << 16) |
                ((uint32_t) vscptype << 8) |
                m_nodeid;                           // nodeaddress (our address)

        msg.flags = CANAL_IDFLAG_EXTENDED;
        msg.sizeData = 4;
        if (CANAL_ERROR_SUCCESS == m_pdll->doCmdSend(&msg)) {
            wxMilliSleep(1);
            return true;
        }

    }
    else if ( USE_TCPIP_INTERFACE == m_type ) {

        // Start block data transfer.

        vscpEventEx event;

        // Send message
        event.head = 0;
        event.vscp_class = 512;                             // CLASS2.PROTOCOL1
        event.vscp_type = VSCP_TYPE_PROTOCOL_START_BLOCK;   // We want to Start block data transfer.
        memset(event.GUID, 0, 16);                          // We use interface GUID
        event.sizeData = 16 + 4;                            // Interface GUID
        memcpy(event.data, m_guid.m_id, 16);                // Address node
        event.data[ 16 ] = 0x00;                            // Block number MSB
        event.data[ 17 ] = 0x00;                            // Block number
        event.data[ 18 ] = (PageAddress & 0xFF00) >> 8;     // Block number
        event.data[ 19 ] = (PageAddress & 0x00FF);          // Block number LSB

        if (CANAL_ERROR_SUCCESS == m_ptcpip->doCmdSendEx(&event)) {
            wxMilliSleep(1);
            return true;
        }
    }

    return false;
}


///////////////////////////////////////////////////////////////////////////////
// sendVSCPBootCommand
// This routine is used to send command from nodes under boot. 
// Index tells which Type & class to send.
//

bool CBootDevice_VSCP::sendVSCPBootCommand( uint8_t index ) 
{
    uint16_t vscpclass;
    uint8_t vscptype;
    //uint8_t nodeid;
    uint8_t priority = 0;

    if ( USE_DLL_INTERFACE == m_type ) {

        canalMsg msg; // rcvmsg;
        //        time_t tstart, tnow;

        memset(msg.data, 0x00, 8);

        if (index == VSCP_TYPE_PROTOCOL_ACTIVATE_NEW_IMAGE) {

            uint16_t crc16 = crcFast(&m_pbufPrg[ 0 ], m_numBlocks * m_blockSize);

            vscpclass = VSCP_CLASS1_PROTOCOL;                   // Class
            vscptype = VSCP_TYPE_PROTOCOL_ACTIVATE_NEW_IMAGE;
            priority = VSCP_PRIORITY_LOW_COMMON;

            msg.data[ 0 ] = (uint8_t)(crc16 >> 8) & 0xff;
            msg.data[ 1 ] = (uint8_t)(crc16 >> 0) & 0xff;

            msg.sizeData = 2;
        }
        else if (index == VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA) {
            vscpclass = VSCP_CLASS1_PROTOCOL;                   // Class
            vscptype = VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA;
            priority = VSCP_PRIORITY_LOW_COMMON;

            // block data transfer
            msg.data[ 0 ] = ((uint8_t)(m_blockNumber >> 24)) & 0xFF;
            msg.data[ 1 ] = ((uint8_t)(m_blockNumber >> 16)) & 0xFF;
            msg.data[ 2 ] = ((uint8_t)(m_blockNumber >>  8)) & 0xFF;
            msg.data[ 3 ] = ((uint8_t)(m_blockNumber >>  0)) & 0xFF;

            msg.sizeData = 4;
        }
        else {
            return false;
        }

        // Send message
        msg.id = ((uint32_t) priority << 26) |
                ((uint32_t) vscpclass << 16) |
                ((uint32_t) vscptype << 8) |
                m_nodeid;                       // nodeaddress (our address)

        msg.flags = CANAL_IDFLAG_EXTENDED;

        if ( CANAL_ERROR_SUCCESS == m_pdll->doCmdSend( &msg ) ) {

            //bRun = true;
            wxMilliSleep(1);
            return true;

        }

    }
    else if ( USE_TCPIP_INTERFACE == m_type ) {

        vscpEventEx event;

        // Send message

        if (index == VSCP_TYPE_PROTOCOL_ACTIVATE_NEW_IMAGE) {

            uint16_t crc16 = crcFast(&m_pbufPrg[ 0 ], m_numBlocks * m_blockSize);

            event.head = 0;
            event.vscp_class = 512;                                     // CLASS2.PROTOCOL1
            event.vscp_type = VSCP_TYPE_PROTOCOL_ACTIVATE_NEW_IMAGE;    // Activate new Image
            memset(event.GUID, 0, 16);                                  // We use interface GUID
            event.sizeData = 16 + 2;                                    // Interface GUID
            memcpy(event.data, m_guid.m_id, 16);                        // Address node
            event.data[ 16 ] = (uint8_t)(crc16 >> 8) & 0xff;
            event.data[ 17 ] = (uint8_t)(crc16 >> 0) & 0xff;
        }
        else if (index == VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA) {
            event.head = 0;
            event.vscp_class = 512;                                     // CLASS2.PROTOCOL1
            event.vscp_type = VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA;    // Activate new Image
            memset(event.GUID, 0, 16);                                  // We use interface GUID
            event.sizeData = 16 + 4;                                    // Interface GUID
            memcpy(event.data, m_guid.m_id, 16);                        // Address node
            event.data[ 16 ] = ((uint8_t)(m_blockNumber >> 24)) & 0xFF;
            event.data[ 17 ] = ((uint8_t)(m_blockNumber >> 16)) & 0xFF;
            event.data[ 18 ] = ((uint8_t)(m_blockNumber >>  8)) & 0xFF;
            event.data[ 19 ] = ((uint8_t)(m_blockNumber >>  0)) & 0xFF;

        }
        else {
            return false;
        }

        if (CANAL_ERROR_SUCCESS == m_ptcpip->doCmdSendEx(&event)) {

            wxMilliSleep(1);

            return true;

        }
    }

    return false;

}

///////////////////////////////////////////////////////////////////////////////
// sendVSCPCommandSeqenceLevel1
// This routine is used to check ack & send command from nodes under boot. 
// check response VSCP_TYPE_PROTOCOL_BLOCK_DATA_ACK   --- Check CRC
// send  VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA
// check response VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_ACK
//

bool CBootDevice_VSCP::sendVSCPCommandSeqenceLevel1(void) 
{


    if ( !checkResponseLevel1( VSCP_TYPE_PROTOCOL_BLOCK_DATA_ACK ) ) {

        wxMessageBox(_T(" Response PROTOCOL_BLOCK_DATA_ACK fails"));

    } 
    else {

        if ( crc_16_host != crc_16_remote ) {
            m_pAddr -= m_blockSize;
            return false;
        }

    }

    wxMilliSleep(1);

    /*
     ** Send command
     */

    if (!sendVSCPBootCommand( VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA ) ) {

        wxMessageBox(_T(" PROGRAM_BLOCK_DATA TX fails"));

    } 
    else {

    }

    wxMilliSleep(1);

    if ( !checkResponseLevel1( VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_ACK ) ) {

        wxMessageBox(_T(" Response PROGRAM_BLOCK_DATA_ACK fails"));

    } 
    else {

    }

    wxMilliSleep(1);


    return true;

}

///////////////////////////////////////////////////////////////////////////////
// sendVSCPCommandSeqenceLevel2
// This routine is used to check ack & send command from nodes under boot. 
// check response VSCP_TYPE_PROTOCOL_BLOCK_DATA_ACK
// send  VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA
// check response VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_ACK
//

bool CBootDevice_VSCP::sendVSCPCommandSeqenceLevel2(void) 
{

    // Check response
    if (!checkResponseLevel2(VSCP_TYPE_PROTOCOL_BLOCK_DATA_ACK)) {

        wxMessageBox(_T(" Response PROTOCOL_BLOCK_DATA_ACK fails"));

    }
    else {

        if ( crc_16_host != crc_16_remote ) {
            m_pAddr -= m_blockSize;
            return false;
        }

    }

    wxMilliSleep(1);

    /*
     ** Send command
     */

    if ( !sendVSCPBootCommand(VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA ) ) {
        // Failure

        wxMessageBox(_T(" PROGRAM_BLOCK_DATA TX fails"));

    } 
    else {
        ;
    }

    wxMilliSleep(1);

    // Check response
    if (!checkResponseLevel2(VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_ACK)) { // send as Zero as at -- present AVR bode for microcontroller -- does not implement this method.
        // Failure
        //rv = false;
        // TODO Resend the block
        wxMessageBox(_T(" Response PROGRAM_BLOCK_DATA_ACK fails"));
    }
    else {

    }

    return true;

}

///////////////////////////////////////////////////////////////////////////////
// checkResponseLevel1
//
// Type = 20 (0x14) ACK program data block   -- for 8 byte packet received correctly by AVR bootloader
//

bool CBootDevice_VSCP::checkResponseLevel1(uint8_t index) 
{
    //canalMsg msg;
    //time_t tstart, tnow;
    bool rv = false;

    uint16_t vscpclass;
    uint8_t vscptype;
    uint8_t priority = 0;

    canalMsg rcvmsg; // msg, 

    if ( NULL == m_pdll ) return false;

    // Get system time
    //time( &tstart );

    bool bRun = true;
    while (bRun) {

        if ( m_pdll->doCmdDataAvailable() ) {

            //if(m_type==0xff)
            //wxMessageBox( _T("123456") );
            m_pdll->doCmdReceive( &rcvmsg );

            if ( ( int )( rcvmsg.id & 0xff ) == m_nodeid ) {

                // Case -- index = 0  --- not implemented always return true 
                if (index == 0) {

                    // Response received from all - return success
                    rv = true;
                    bRun = false;

                }
                // Case -- index = 1
                else if (index == VSCP_TYPE_PROTOCOL_BLOCK_DATA_ACK) {


                    vscpclass = VSCP_CLASS1_PROTOCOL;
                    vscptype = VSCP_TYPE_PROTOCOL_BLOCK_DATA_ACK;

                    if ((uint32_t) (rcvmsg.id & 0x01ffffff) == (uint32_t) (((uint32_t) vscpclass << 16) | ((uint32_t) vscptype << 8) | m_nodeid)) {

                        // Calculate CRC in host
                        crc_16_host = crcFast(&m_pbufPrg[ m_pAddr - m_blockSize ], m_blockSize);
                        // GET CRC in remote node
                        crc_16_remote = ( ( ( uint16_t ) rcvmsg.data[0] ) << 8 ) |
                                        ( ( ( uint16_t ) rcvmsg.data[1] ) << 0 );
                                        
                        // Response received from all - return success
                        rv = true;
                        bRun = false;
     
                    }

                }
                else if (index == VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_ACK) {

                    vscpclass = VSCP_CLASS1_PROTOCOL;
                    vscptype = VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_ACK;

                    if ((uint32_t) (rcvmsg.id & 0x01ffffff) == 
                            (uint32_t) (((uint32_t) vscpclass << 16) | 
                                ((uint32_t) vscptype << 8) | m_nodeid)) {

                        // Response received from all - return success
                        rv = true;
                        bRun = false;

                    }

                }



            } // id found



        } // received message


    }

    return rv;
}


///////////////////////////////////////////////////////////////////////////////
// checkResponseLevel2
// Type = 20 (0x14) ACK program data block    -- 
//  for 8 byte packet received correctly by bootloader
//
bool CBootDevice_VSCP::checkResponseLevel2(uint8_t index) 
{
    vscpEventEx event;
    //time_t tstart, tnow;
    bool rv = false;

    if (NULL == m_ptcpip) return false;


    bool bRun = true;
    while (bRun) {

        if ( m_ptcpip->doCmdDataAvailable() ) {

            m_ptcpip->doCmdReceiveEx( &event );

            if ((VSCP_CLASS1_PROTOCOL == event.vscp_class) &&
                (m_guid.getLSB() == event.GUID[ 15 ])) { // correct id

                // Case -- index = 0  --- not implemented always return true 
                if (index == 0) {

                    // Response received from all - return success
                    rv = true;
                    bRun = false;

                }
                // Case -- index = VSCP_TYPE_PROTOCOL_BLOCK_DATA_ACK
                else if (index == VSCP_TYPE_PROTOCOL_BLOCK_DATA_ACK) {

                    if (event.vscp_type == VSCP_TYPE_PROTOCOL_BLOCK_DATA_ACK) {

                        // Calculate CRC in host
                        crc_16_host = crcFast(&m_pbufPrg[ m_pAddr - m_blockSize ], m_blockSize);
                        // GET CRC in remote node
                        crc_16_remote = ( ( ( uint16_t ) event.data[ 0 ] ) << 8 ) |
                                        ( ( ( uint16_t ) event.data[ 1 ] ) << 0 );

                        // Response received from all - return success
                        rv = true;
                        bRun = false;

                    }

                }
                // Case -- index = VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_ACK
                else if (index == VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_ACK) {

                    if (event.vscp_type == VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_ACK) {

                        // Response received from all - return success
                        rv = true;
                        bRun = false;
                    }

                }

            } // id

        } // received message

    }

    return rv;
}
