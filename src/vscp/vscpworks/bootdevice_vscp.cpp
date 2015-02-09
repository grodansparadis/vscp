/////////////////////////////////////////////////////////////////////////////
// Name:        bootdevice_vscp.cpp
// Purpose:     
// Author:      Ake Hedman
// Modified by: 
// Created:     16/12/2009 22:26:09
// RCS-ID:      
// Copyright:   (C) 2007-2015 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
//				(C) 2012 Dinesh Guleria
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


CBootDevice_VSCP::CBootDevice_VSCP( CDllWrapper *pdll, uint8_t nodeid ) :
    CBootDevice( pdll, nodeid )
{
    init();
    m_type = USE_DLL_INTERFACE;
}

CBootDevice_VSCP::CBootDevice_VSCP( VscpRemoteTcpIf *ptcpip, cguid &guid, cguid &ifguid ) :
    CBootDevice( ptcpip, guid, ifguid )
{
    init();
    m_type = USE_TCPIP_INTERFACE;
}

CBootDevice_VSCP::~CBootDevice_VSCP(void) {
    //	delete [] m_pbufPrg;
    //	delete [] m_pbufCfg;
    //	delete [] m_pbufEEPROM;

}


///////////////////////////////////////////////////////////////////////////////
// init
//

void CBootDevice_VSCP::init( void )
{
    // Create buffers
    m_pbufPrg = new unsigned char[ BUFFER_SIZE_PROGRAM_COMMON ];
    m_pbufCfg = new unsigned char[ BUFFER_SIZE_CONFIG_COMMON ];
    m_pbufEEPROM = new unsigned char[ BUFFER_SIZE_EEPROM_COMMON ];

    //m_bHandshake = true;		// No handshake as default
    m_pAddr = 0;
    m_type = MEM_TYPE_PROGRAM;

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
    unsigned long fullAddr = 0;
    unsigned long highAddr = 0;
    unsigned long lowAddr = 0;
    unsigned long cntData = 0;
    unsigned long recType = 0;


    // Init. flash memory pointers
    m_minFlashAddr = 0xffffffff;
    m_maxFlashAddr = 0;
    m_totalCntData = 0;
    m_bPrgData = false;
    m_bFlashMemory = true; // Flash memory should be programmed


    // Init program memory buffer
    if (NULL == m_pbufPrg) return false;
    memset(m_pbufPrg, 0xff, BUFFER_SIZE_PROGRAM_COMMON);


    // Init. config memory pointers
    m_minConfigAddr = 0xffffffff;
    m_maxConfigAddr = 0;
    m_bConfigData = false;
    m_bConfigMemory = true; // Config memory should be programmed

    // Init config memory buffer
    if (NULL == m_pbufCfg) return false;
    memset(m_pbufPrg, 0xff, BUFFER_SIZE_CONFIG_COMMON);

    // Init. EEPROM memory pointers
    m_minEEPROMAddr = 0xffffffff;
    m_maxEEPROMAddr = 0;
    m_bEEPROMData = false;
    m_bEEPROMMemory = true; // EEPROM should be programmed

    // Init EEPROM memory buffer
    if (NULL == m_pbufEEPROM) return false;
    memset(m_pbufEEPROM, 0xff, BUFFER_SIZE_EEPROM_COMMON);

    FILE* fs;
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


    while (bRun && (NULL != fgets(szLine, MAX_PATH, fs))) {

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
                        strncpy(szData, (szLine + ((i * 2) + 9)), 2); // read two character -- because two character makes one digit 1= 0x01
#endif
                        unsigned char val =
                                (unsigned char) (strtoul(szData, &endptr, 16) & 0xff);

                        if ((fullAddr < MEMREG_PRG_END_COMMON) &&
                                (fullAddr < BUFFER_SIZE_PROGRAM_COMMON)) {

                            // Write into program memory buffer

                            m_pbufPrg[ fullAddr ] = val;
                            m_bPrgData = true;

                            // Set min flash address
                            if (fullAddr < m_minFlashAddr) m_minFlashAddr = fullAddr;

                            // Set max flash address
                            if (fullAddr > m_maxFlashAddr) m_maxFlashAddr = fullAddr;

                        } else if ((fullAddr >= MEMREG_CONFIG_START_COMMON) &&
                                ((fullAddr < MEMREG_CONFIG_START_COMMON + BUFFER_SIZE_CONFIG_COMMON))) {

                            // Write into config memory buffer

                            m_pbufCfg[ fullAddr - MEMREG_CONFIG_START_COMMON ] = val;
                            m_bConfigData = true;

                            // Set min config address
                            if (fullAddr < m_minConfigAddr) m_minConfigAddr = fullAddr;

                            // Set max config address
                            if (fullAddr > m_maxConfigAddr) m_maxConfigAddr = fullAddr;

                        } else if ((fullAddr >= MEMREG_EEPROM_START_COMMON) &&
                                ((fullAddr <= MEMREG_EEPROM_START_COMMON + BUFFER_SIZE_EEPROM_COMMON))) {

                            // Write into EEPROM memory buffer

                            m_pbufEEPROM[ fullAddr - MEMREG_EEPROM_START_COMMON ] = val;
                            m_bEEPROMData = true;

                            // Set min EEEPROM address
                            if (fullAddr < m_minEEPROMAddr) m_minEEPROMAddr = fullAddr;

                            // Set max config address
                            if (fullAddr > m_maxEEPROMAddr) m_maxEEPROMAddr = fullAddr;

                        }

                        fullAddr = fullAddr + 1;

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

    rv = true;

    // Flash to program if none read
    if (!m_bPrgData) {
        m_bFlashMemory = false;
        m_minFlashAddr = 0x00000000;
    }

    // Config data to program if none read
    if (!m_bConfigData) {
        m_bConfigMemory = false;
        m_minConfigAddr = 0x00000000;
    }

    // EEPROM data to program if none read
    if (!m_bEEPROMData) {
        m_bEEPROMMemory = false;
        m_minEEPROMAddr = 0x00000000;
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

    // * * * Config Memory * * *

    phtmlWnd->AppendToPage(
            _("<b><u>Config Memory</u></b><br>"));

    phtmlWnd->AppendToPage(
            _("<b>Start :</b>"));

    strInfo.Printf(_("<font color=\"#005CB9\">0x%08X</font>"), m_minConfigAddr);
    phtmlWnd->AppendToPage(strInfo);

    phtmlWnd->AppendToPage(
            _("<b> End :</b>"));

    strInfo.Printf(_("<font color=\"#005CB9\">0x%08X<br></font>"), m_maxConfigAddr);
    phtmlWnd->AppendToPage(strInfo);

    if (m_bConfigMemory) {
        phtmlWnd->AppendToPage(
                _("<font color=\"#348017\">Will be programmed</font><br>"));
    } else {
        phtmlWnd->AppendToPage(
                _("<font color=\"#F6358A\">Will not be programmed</font><br>"));
    }

    phtmlWnd->AppendToPage(
            _("<br><br>"));



    // * * * EEPROM * * *

    phtmlWnd->AppendToPage(
            _("<b><u>EEPROM Memory</u></b><br>"));

    phtmlWnd->AppendToPage(
            _("<B>Start :</b>"));

    strInfo.Printf(_("<font color=\"#005CB9\">0x%08X</font>"), m_minEEPROMAddr);
    phtmlWnd->AppendToPage(strInfo);

    phtmlWnd->AppendToPage(
            _("<b> End :</b>"));

    strInfo.Printf(_("<font color=\"#005CB9\">0x%08X<br></font>"), m_maxEEPROMAddr);
    phtmlWnd->AppendToPage(strInfo);

    if (m_bEEPROMMemory) {
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

    /*    uint8_t pageMSB;
        uint8_t pageLSB;
        uint8_t guid0;
        uint8_t guid3;
        uint8_t guid5;
        uint8_t guid7;
    )*/

    uint16_t vscpclass;
    uint8_t vscptype;
    uint8_t nodeid;
    uint8_t priority = 0;

    wxBusyCursor busy;

    if ( USE_DLL_INTERFACE == m_type ) {

        canalMsg msg, rcvmsg;
        time_t tstart, tnow;

        memset( msg.data, 0x00, 8 );

        // Read page register GUID0
        if ( !m_pdll->readLevel1Register( m_guid.m_id[ 0 ],
                                            0,
                                            VSCP_REG_GUID0,
                                            &pageMSB ) ) {
            return false;
        }

        // Read page register GUID3
        if ( !m_pdll->readLevel1Register( m_guid.m_id[ 0 ],
                                            0,
                                            VSCP_REG_GUID3,
                                            &pageMSB ) ) {
            return false;
        }

        // Read page register GUID5
        if ( !m_pdll->readLevel1Register( m_guid.m_id[ 0 ],
                                            0,
                                            VSCP_REG_GUID5,
                                            &pageMSB ) ) {
            return false;
        }

        // Read page register GUID7
        if ( !m_pdll->readLevel1Register( m_guid.m_id[ 0 ],
                                            0,
                                            VSCP_REG_GUID7,
                                            &pageMSB ) ) {
            return false;
        }

        nodeid = m_guid.m_id[ 0 ]; // Nickname to read register from
        vscpclass = VSCP_CLASS1_PROTOCOL; // Class
        vscptype = VSCP_ENTER_BOOTLODER_MODE;
        priority = VSCP_PRIORITY_LOW_COMMON;
        // Set device in boot mode
        msg.data[ 0 ] = m_guid.m_id[ 0 ]; // Nickname to read register from
        msg.data[ 1 ] = VSCP_BOOTLOADER_VSCP; // VSCP AVR1 bootload algorithm	
        msg.data[ 2 ] = guid0;
        msg.data[ 3 ] = guid3;
        msg.data[ 4 ] = guid5;
        msg.data[ 5 ] = guid7;
        msg.data[ 6 ] = pageMSB;
        msg.data[ 7 ] = pageLSB;

        // Send message
        msg.id = ((uint32_t) priority << 26) |
                    ((uint32_t) vscpclass << 16) |
                    ((uint32_t) vscptype << 8) |
                    nodeid; // nodeaddress (our address)

        msg.flags = CANAL_IDFLAG_EXTENDED;
        msg.sizeData = 8;
        if ( CANAL_ERROR_SUCCESS == m_pdll->doCmdSend( &msg ) ) {

            bRun = true;

            // Get start time
            time(&tstart);

            while (bRun) {

                time(&tnow);
                if ((unsigned long) (tnow - tstart) > 5) {
                    bRun = false;
                }

                if ( m_pdll->doCmdDataAvailable() ) {

                    m_pdll->doCmdReceive( &rcvmsg );

                    vscptype = VSCP_TYPE_PROTOCOL_ACK_BOOT_LOADER;

                    if ((uint32_t) (rcvmsg.id & 0x01ffffff) == 
                         (uint32_t) (((uint32_t) vscpclass << 16) | ((uint32_t) vscptype << 8) | nodeid)) {
                        // OK in bootmode - return

                        falsh_memory_block_size = 
                            (uint32_t) (((uint32_t) rcvmsg.data[ 0 ] << 24) | ((uint32_t) rcvmsg.data[ 1 ] << 16) | ((uint32_t) rcvmsg.data[ 2 ] << 8) | rcvmsg.data[ 3 ]);

                        return true;
                    }

                }
            }

        }

    } 
    else if ( USE_TCPIP_INTERFACE == m_type ) {

        vscpEventEx evex;
        time_t tstart, tnow;

        // Read page register GUID0
        if ( VSCP_ERROR_SUCCESS == m_ptcpip->readLevel2Register( VSCP_REG_GUID0, 0, &guid0, m_ifguid ) ) {
            return false;
        }


        // Read page register GUID3
        if ( VSCP_ERROR_SUCCESS == m_ptcpip->readLevel2Register( VSCP_REG_GUID3, 0, &guid0, m_ifguid ) ) {
            return false;
        }

        // Read page register GUID5
        if ( VSCP_ERROR_SUCCESS == m_ptcpip->readLevel2Register( VSCP_REG_GUID5, 0, &guid0, m_ifguid ) ) {
            return false;
        }

        // Read page register GUID7
        if ( VSCP_ERROR_SUCCESS == m_ptcpip->readLevel2Register( VSCP_REG_GUID7, 0, &guid0, m_ifguid ) ) {
            return false;
        }

        // Set device in boot mode

        // Send message
        evex.head = 0;
        evex.vscp_class = 512; // CLASS2.PROTOCOL1
        evex.vscp_type = VSCP_ENTER_BOOTLODER_MODE; // We want to enter bootloader mode
        memset( evex.GUID, 0, 16 ); // We use interface GUID
        evex.sizeData = 16 + 8; // Interface GUID
        memcpy( evex.data, m_guid.m_id, 16 ); // Address node
        evex.data[ 16 ] = m_guid.m_id[ 0 ]; // Nickname to read register from
        evex.data[ 17 ] = VSCP_BOOTLOADER_VSCP; // VSCP bootload algorithm	
        evex.data[ 18 ] = guid0;
        evex.data[ 19 ] = guid3;
        evex.data[ 20 ] = guid5;
        evex.data[ 21 ] = guid7;
        evex.data[ 22 ] = pageMSB;
        evex.data[ 23 ] = pageLSB;

        if ( VSCP_ERROR_SUCCESS == m_ptcpip->doCmdSendEx( &evex ) ) {

            bRun = true;

            // Get start time
            time(&tstart);

            while (bRun) {

                time(&tnow);
                if ((unsigned long) (tnow - tstart) > 5) {
                    bRun = false;
                }

                //vscpEventEx rcvmsg;
                if ( m_ptcpip->doCmdDataAvailable() ) {

                    m_ptcpip->doCmdReceiveEx( &evex );

                    // Check for response  ---  Type = 13 (0x0D) ACK boot loader mode.
                    // Ignore data  --  Flash block size
                    // Ignore data  --  Number of block s available.                    
                    if ( VSCP_TYPE_PROTOCOL_ACK_BOOT_LOADER == evex.vscp_type ) {
                        // OK in bootmode - return
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
    /*
    uint8_t pageMSB;
uint8_t pageLSB;
uint8_t guid0;
uint8_t guid3;
uint8_t guid5;
uint8_t guid7;
     */
    BTL_PAGE = 0; // page to program in AVR -- after every 32*8 bytes = 256 byte ---- increment by 1
    // 9-bit --- 2^9 = 512
    // AT90CAN128  -- have --- Page Size = 256 bytes  ---- 512 pages  =  512 * 256  == 128 Kbyte
    BTL_BLOCK = 1; // There are 32 -- 8 bye block -- in one page  ---- page size = 256 byte
    uint8_t temp;

    wxBusyCursor busy;

    m_checksum = 0;
    uint32_t progress = 0;
    uint32_t addr;
    wxString wxStatusStr;

    uint32_t nFlashPackets = 0;
    uint32_t nConfigPackets = 0;
    uint32_t nEEPROMPackets = 0;

    // Flash memory
    if (m_bPrgData) {
        nFlashPackets = (m_maxFlashAddr - m_minFlashAddr) / 8;

        if (0 != ((m_maxFlashAddr - m_minFlashAddr) % 8)) {
            nFlashPackets++;
        }
    }

    // Config
    if (m_bConfigData) {
        nConfigPackets = (m_maxConfigAddr - m_minConfigAddr) / 8;

        if (0 != ((m_maxConfigAddr - m_minConfigAddr) % 8)) {
            nConfigPackets++;
        }
    }

    // EEPROM
    if (m_bEEPROMData) {
        nEEPROMPackets = (m_maxEEPROMAddr - m_minEEPROMAddr) / 8;

        if (0 != ((m_maxEEPROMAddr - m_minEEPROMAddr) % 8)) {
            nEEPROMPackets++;
        }
    }

    long nTotalPackets = nFlashPackets + nConfigPackets + nEEPROMPackets;
    wxProgressDialog* pDlg = new wxProgressDialog(_T("Boot loading in progress..."),
            _T("---"),
            nTotalPackets,
            NULL,
            wxPD_AUTO_HIDE |
            wxPD_APP_MODAL |
            wxPD_CAN_ABORT |
            wxPD_ELAPSED_TIME |
            wxPD_REMAINING_TIME);

    // Initialize checksum
    addr = m_minFlashAddr;
    if (!writeDeviceControlRegs(addr)) {
        wxMessageBox(_T("Memmory not fall in any catofy FLASH , Config, EEPROM."));
        rv = FALSE;
        bRun = false;
    }

    // * * * flash memory * * *

    if (rv && m_bPrgData) {

        addr = m_minFlashAddr;
        if (writeDeviceControlRegs(addr)) {

            for (unsigned long blk = 0; ((blk < nFlashPackets) && bRun);) { // nFlashPackets  = number of 8 byte packets

                if (BTL_BLOCK == 1) {
                    // Send the start block
                    temp = sendVSCPCommandStartBlock(BTL_PAGE);
                    if (temp != (uint8_t) true)
                        wxMessageBox(_T("start Block error"));
                }

                //prevent overflow                                //  BUFFERSIZE         128  words = 256 bytes -- i.e 32*8 = 256
                if (BTL_BLOCK <= falsh_memory_block_size / 8 /*32*/) {


                    wxStatusStr.Printf(_("Loading flash... %08x"), addr);
                    //					wxStatusStr.Printf( _("%d %d %d "), m_maxFlashAddr, m_minFlashAddr,  nFlashPackets);
                    if (!(bRun = pDlg->Update(progress, wxStatusStr))) {
                        wxMessageBox(_T("Aborted by user."));
                        rv = FALSE;
                        bRun = false;
                    }

                    if (!writeFrimwareSector()) {
                        wxMessageBox(_T("Failed to write flash data to node(s)."));
                        rv = FALSE;
                        bRun = false;
                    }

                    wxMilliSleep(1);
                    progress++;
                    addr += 8;
                    blk++;
                }

                BTL_BLOCK++;

                if (BTL_BLOCK > falsh_memory_block_size / 8 /*32*/) {

#ifdef PRINT_DEBUG_EVENTS				
                    uart_puts("BOOT_FULLBLOCK");
#endif
                    // todo: check CRC of written data, if ok send confirm block event
                    BTL_BLOCK = 1;

                    // Check for the ACK  ---- VSCP_TYPE_PROTOCOL_BLOCK_DATA_ACK
                    // Send -- VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA
                    // Check for the ACK  ---- VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_ACK
                    if ( USE_DLL_INTERFACE == m_type ) {

                        flag_crc = sendVSCPCommandSeqenceLevel1();

                    } else {

                        flag_crc = sendVSCPCommandSeqenceLevel2();
                    }

                    if (flag_crc == false) {
                        blk -= falsh_memory_block_size;
                    } else {
                        // increment the page number
                        BTL_PAGE++;
                    }

                }
            }

            if ((BTL_BLOCK < falsh_memory_block_size / 8 /*32*/) && (BTL_BLOCK > 1)) {
                //while(1);
                //wxMessageBox( _T("Enter") );
                BTL_PAGE -= 1; // Just decrement well in advance if -- CRC fails
                //m_type = 0xFF;  // This will send packet as 0xFF  --- in function  --- writeFrimwareSector()    ----------------- no need buffer already 0xff in memset

                while (BTL_BLOCK <= falsh_memory_block_size / 8 /*32*/ && bRun) {

                    if (!writeFrimwareSector()) {
                        wxMessageBox(_T("Failed to write flash data to node(s)."));
                        rv = FALSE;
                        bRun = false;
                    }

                    wxMilliSleep(1);
                    progress++;
                    addr += 8;

                    BTL_BLOCK++;


                    if (BTL_BLOCK > falsh_memory_block_size / 8 /*32*/) {
#ifdef PRINT_DEBUG_EVENTS				
                        uart_puts("BOOT_FULLBLOCK");
#endif

                        // Check for the ACK  ---- VSCP_TYPE_PROTOCOL_BLOCK_DATA_ACK
                        // Send -- VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA
                        // Check for the ACK  ---- VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_ACK
                        if ( USE_DLL_INTERFACE == m_type ) {
                            flag_crc = sendVSCPCommandSeqenceLevel1();
                        } else {
                            flag_crc = sendVSCPCommandSeqenceLevel2();
                        }

                        //check for crc valid flag
                        if (flag_crc == false) {
                            BTL_BLOCK = 1;

                            // Send the start block
                            // BTL_PAGE already decremented at the start -- for this last page
                            temp = sendVSCPCommandStartBlock(BTL_PAGE);
                            if (temp != (uint8_t) true)
                                wxMessageBox(_T("start Block error"));
                        }


                    }
                }
            }
            wxMilliSleep(1);
            wxMilliSleep(1);
            wxMilliSleep(1);
            wxMilliSleep(1);
            wxMilliSleep(1);
            //wxMessageBox( _T("xxxxxxxxx") );	
        } else {
            wxMessageBox(_T("Failed to send control info for flash data to node(s)."));
            rv = FALSE;
        }

        //rv = false;// so that next case do not execute
    }
    //wxMessageBox( _T("Failed to send control info for flash data to node(s).") );	
    // * * * config memory * * *

    if (rv && m_bConfigData) {
        //wxMessageBox( _T("xxxxttttttt") );	
        // Send the start block
        addr = m_minConfigAddr;
        if (writeDeviceControlRegs(addr)) {

            for (unsigned long blk = 0; ((blk < nConfigPackets) && bRun); blk++) {

                wxStatusStr.Printf(_("Loading config memory... %0X"), addr);
                if (!(bRun = pDlg->Update(progress, wxStatusStr))) {
                    wxMessageBox(_T("Aborted by user."));
                    rv = FALSE;
                    bRun = false;
                }

                if (!writeFrimwareSector()) {
                    wxMessageBox(_T("Failed to write config data to node(s)."));
                    rv = FALSE;
                    bRun = false;
                }

                wxMilliSleep(1);
                progress++;
                addr += 8;

            }
        } else {
            wxMessageBox(_T("Failed to send control info for config data to node(s)."));
            rv = FALSE;
        }
    }

    // * * * EEPROM memory * * *

    if (rv && m_bEEPROMData) {

        // Send the start block
        addr = m_minEEPROMAddr;
        if (writeDeviceControlRegs(addr)) {

            for (unsigned long blk = 0; ((blk < nConfigPackets) && bRun); blk++) {

                wxStatusStr.Printf(_("Loading EEPROM memory... %0X"), addr);
                if (!(bRun = pDlg->Update(progress, wxStatusStr))) {
                    wxMessageBox(_T("Aborted by user."));
                    rv = FALSE;
                    bRun = false;
                }

                if (!writeFrimwareSector()) {
                    wxMessageBox(_T("Failed to write EEPROM data to node(s)."));
                    rv = FALSE;
                    bRun = false;
                }

                wxMilliSleep(1);
                progress++;
                addr += 8;

            }
        } else {
            wxMessageBox(_T("Failed to send control info for EEPROM data to node(s)."));
            rv = FALSE;
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

    wxMilliSleep(1);
    wxMilliSleep(1);
    wxMilliSleep(1);
    wxMilliSleep(1);
    wxMilliSleep(1);

    // Done
    progress = nTotalPackets;
    pDlg->Update(progress, wxStatusStr);


    pDlg->Destroy();

    wxMilliSleep(1);
    wxMilliSleep(1);
    wxMilliSleep(1);
    wxMilliSleep(1);
    wxMilliSleep(1);
    //wxMessageBox( _T("complete") );	
    return rv;
}


///////////////////////////////////////////////////////////////////////////////
// writeFrimwareSector
// write 8 byte packets
//

bool CBootDevice_VSCP::writeFrimwareSector(void)
{
    canalMsg msg;
    vscpEventEx event;
    bool rv = true;

    uint16_t vscpclass;
    uint8_t vscptype;
    uint8_t nodeid;
    uint8_t priority = 0;

    if ( NULL == m_pdll ) return FALSE;

    nodeid = m_guid.m_id[ 0 ]; // Nickname to read register from
    vscpclass = VSCP_CLASS1_PROTOCOL; // Class
    vscptype = VSCP_TYPE_PROTOCOL_BLOCK_DATA;
    priority = VSCP_PRIORITY_LOW_COMMON;

    // Send event
    if ( USE_DLL_INTERFACE == m_type ) {
        // Send message
        msg.id = ((uint32_t) priority << 26) |
                ((uint32_t) vscpclass << 16) |
                ((uint32_t) vscptype << 8) |
                nodeid; // nodeaddress (our address)
        msg.flags = CANAL_IDFLAG_EXTENDED;
        msg.sizeData = 8;
    } 
    else if ( USE_TCPIP_INTERFACE == m_type ) {
        event.head = 0;
        event.vscp_class = 512; // CLASS2.PROTOCOL1
        event.vscp_type = vscptype;
        memset(event.GUID, 0, 16); // We use interface GUID
        event.sizeData = 16 + 8; // Interface GUID
        memcpy(event.data, m_guid.m_id, 16); // Address node
    } 
    else {
        return false;
    }

    uint8_t b;
    for (int i = 0; i < 8; i++) {

        switch (m_type) {

            case MEM_TYPE_PROGRAM:
                b = m_pbufPrg[ m_pAddr ];
                m_checksum += m_pbufPrg[ m_pAddr ];
                break;

            case MEM_TYPE_CONFIG:
                b = m_pbufCfg[ m_pAddr ];
                m_checksum += m_pbufCfg[ m_pAddr ];
                break;

            case MEM_TYPE_EEPROM:
                b = m_pbufEEPROM[ m_pAddr ];
                m_checksum += m_pbufEEPROM[ m_pAddr ];
                break;

            default:
                b = 0xff;
                //wxMessageBox( _T("0xff") );
                break;
        }

        // Write data into frame
        if ( USE_DLL_INTERFACE == m_type ) {
            msg.data[ i ] = b;
        } 
        else {
            event.data[ 16 + i] = b;
        }

        // Update address
        m_pAddr++;

    }

    if ( USE_DLL_INTERFACE == m_type ) {
        m_pdll->doCmdSend( &msg );
    } 
    else {
        m_ptcpip->doCmdSendEx(&event);
    }


    return rv;
}


///////////////////////////////////////////////////////////////////////////////
// writeDeviceControlRegs
//

bool CBootDevice_VSCP::writeDeviceControlRegs(uint32_t addr) 
{

    // Save the internal addresss
    m_pAddr = addr;

    if ((m_pAddr < MEMREG_PRG_END_COMMON) && (m_pAddr < BUFFER_SIZE_PROGRAM_COMMON)) {

        // Flash memory
        m_type = MEM_TYPE_PROGRAM;
        return TRUE;

    } 
    else if ((m_pAddr >= MEMREG_CONFIG_START_COMMON) &&
            ((m_pAddr < MEMREG_CONFIG_START_COMMON + BUFFER_SIZE_CONFIG_COMMON))) {

        // Config memory
        m_type = MEM_TYPE_CONFIG;
        return TRUE;

    } 
    else if ((m_pAddr >= MEMREG_EEPROM_START_COMMON) &&
            ((m_pAddr <= MEMREG_EEPROM_START_COMMON + BUFFER_SIZE_EEPROM_COMMON))) {

        // EEPROM
        m_type = MEM_TYPE_EEPROM;
        return TRUE;
    } 
    else {
        return false;
    }


}


///////////////////////////////////////////////////////////////////////////////
// sendVSCPCommandStartBlock
// PageAddress : Page to be programmed
// This command have no ACK

bool CBootDevice_VSCP::sendVSCPCommandStartBlock(uint16_t PageAddress) 
{
    uint16_t vscpclass;
    uint8_t vscptype;
    uint8_t nodeid;
    uint8_t priority;

    wxBusyCursor busy;

    if ( USE_DLL_INTERFACE == m_type ) {

        canalMsg msg;


        memset(msg.data, 0x00, 8);

        nodeid = m_guid.m_id[ 0 ]; // Nickname to read register from
        vscpclass = VSCP_CLASS1_PROTOCOL; // Class
        vscptype = VSCP_TYPE_PROTOCOL_START_BLOCK; // Start block data transfer.
        priority = VSCP_PRIORITY_LOW_COMMON;
        // block data transfer
        msg.data[ 0 ] = 0; //MSB 
        msg.data[ 1 ] = 0;
        msg.data[ 2 ] = (PageAddress & 0xFF00) >> 8;
        msg.data[ 3 ] = (PageAddress & 0x00FF); //LSB
        msg.data[ 4 ] = 0x00; // Byte4 (optional) Type of Memory we want to write
        msg.data[ 5 ] = 0x00; // Byte5 (optional) Bank/Image to be written
        msg.data[ 6 ] = 0x00;
        msg.data[ 7 ] = 0x00;

        // Send message
        msg.id = ((uint32_t) priority << 26) |
                ((uint32_t) vscpclass << 16) |
                ((uint32_t) vscptype << 8) |
                nodeid; // nodeaddress (our address)

        msg.flags = CANAL_IDFLAG_EXTENDED;
        msg.sizeData = 8;
        if (CANAL_ERROR_SUCCESS == m_pdll->doCmdSend(&msg)) {
            wxMilliSleep(1);
            return true;
        }

    }
    else if ( USE_TCPIP_INTERFACE == m_type ) {

        // Start block data transfer.

        vscpEventEx event;
        // Set device in boot mode

        // Send message
        event.head = 0;
        event.vscp_class = 512; // CLASS2.PROTOCOL1
        event.vscp_type = VSCP_TYPE_PROTOCOL_START_BLOCK; // We want to Start block data transfer.
        memset(event.GUID, 0, 16); // We use interface GUID
        event.sizeData = 16 + 8; // Interface GUID
        memcpy(event.data, m_guid.m_id, 16); // Address node
        event.data[ 16 ] = 0; //MSB 
        event.data[ 17 ] = 0x00;
        event.data[ 18 ] = (PageAddress & 0xFF00) >> 8;
        event.data[ 19 ] = (PageAddress & 0x00FF); //LSB
        event.data[ 20 ] = 0x00; // Byte4 (optional) Type of Memory we want to write
        event.data[ 21 ] = 0x00; // Byte5 (optional) Bank/Image to be written
        event.data[ 22 ] = 0x00;
        event.data[ 23 ] = 0x00;

        if (CANAL_ERROR_SUCCESS == m_ptcpip->doCmdSendEx(&event)) {

        }
    }

    return false;
}


///////////////////////////////////////////////////////////////////////////////
// sendVSCPBootCommand
// This routine is used to send command from nodes under boot. 
// Index tells which Type & class to send.
//

bool CBootDevice_VSCP::sendVSCPBootCommand(uint8_t index) 
{
    uint16_t vscpclass;
    uint8_t vscptype;
    uint8_t nodeid;
    uint8_t priority = 0;

    if ( USE_DLL_INTERFACE == m_type ) {

        canalMsg msg; // rcvmsg;
        //        time_t tstart, tnow;

        memset(msg.data, 0x00, 8);

        if (index == VSCP_TYPE_PROTOCOL_ACTIVATE_NEW_IMAGE) {
            nodeid = m_guid.m_id[ 0 ]; // Nickname to read register from
            vscpclass = VSCP_CLASS1_PROTOCOL; // Class
            vscptype = VSCP_TYPE_PROTOCOL_ACTIVATE_NEW_IMAGE;
            priority = VSCP_PRIORITY_LOW_COMMON;

            msg.sizeData = 0;
        }

        if (index == VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA) {
            nodeid = m_guid.m_id[ 0 ]; // Nickname to read register from
            vscpclass = VSCP_CLASS1_PROTOCOL; // Class
            vscptype = VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA;
            priority = VSCP_PRIORITY_LOW_COMMON;

            // block data transfer
            msg.data[ 0 ] = 0; //MSB 
            msg.data[ 1 ] = 0;
            msg.data[ 2 ] = (BTL_PAGE & 0xFF00) >> 8;
            msg.data[ 3 ] = (BTL_PAGE & 0x00FF); //LSB

            msg.sizeData = 4;
        }

        // Send message
        msg.id = ((uint32_t) priority << 26) |
                ((uint32_t) vscpclass << 16) |
                ((uint32_t) vscptype << 8) |
                nodeid; // nodeaddress (our address)

        msg.flags = CANAL_IDFLAG_EXTENDED;

        if (CANAL_ERROR_SUCCESS == m_pdll->doCmdSend(&msg)) {

            //bRun = true;

            wxMilliSleep(1);

            return true;

        }

    }
    else if ( USE_TCPIP_INTERFACE == m_type ) {

        vscpEventEx event;
        //        time_t tstart, tnow;


        // Set device in boot mode

        // Send message

        if (index == VSCP_TYPE_PROTOCOL_ACTIVATE_NEW_IMAGE) {
            event.head = 0;
            event.vscp_class = 512; // CLASS2.PROTOCOL1
            event.vscp_type = VSCP_TYPE_PROTOCOL_ACTIVATE_NEW_IMAGE; // Activate new Image
            memset(event.GUID, 0, 16); // We use interface GUID
            event.sizeData = 16 + 0; // Interface GUID
            memcpy(event.data, m_guid.m_id, 16); // Address node
            /*event.data[ 16 ]  = m_guid.m_id[ 0 ];	    // Nickname to read register from
            event.data[ 17 ]  = VSCP_BOOTLOADER_AVR;	// VSCP PIC1 bootload algorithm	
            event.data[ 18 ]  = guid0;
            event.data[ 19 ]  = guid3;
            event.data[ 20 ]  = guid5;
            event.data[ 21 ]  = guid7;
            event.data[ 22 ]  = pageMSB;
            event.data[ 23 ]  = pageLSB;*/
        }

        if (index == VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA) {
            event.head = 0;
            event.vscp_class = 512; // CLASS2.PROTOCOL1
            event.vscp_type = VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA; // Activate new Image
            memset(event.GUID, 0, 16); // We use interface GUID
            event.sizeData = 16 + 4; // Interface GUID
            memcpy(event.data, m_guid.m_id, 16); // Address node
            event.data[ 16 ] = 0; //MSB 
            event.data[ 17 ] = 0;
            event.data[ 18 ] = (BTL_PAGE & 0xFF00) >> 8;
            event.data[ 19 ] = (BTL_PAGE & 0x00FF); //LSB

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

bool CBootDevice_VSCP::sendVSCPCommandSeqenceLevel1(void) {


    if (!checkResponseLevel1(VSCP_TYPE_PROTOCOL_BLOCK_DATA_ACK)) {

        wxMessageBox(_T(" Response PROTOCOL_BLOCK_DATA_ACK fails"));

    } else {

        if (crc_16_host != crc_16_remote) {
            m_pAddr -= falsh_memory_block_size;
            return false;
        }

    }

    wxMilliSleep(1);

    /*
     ** Send command
     */

    if (!sendVSCPBootCommand(VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA)) {

        wxMessageBox(_T(" PROGRAM_BLOCK_DATA TX fails"));

    } else {

    }

    wxMilliSleep(1);

    if (!checkResponseLevel1(VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_ACK)) {

        wxMessageBox(_T(" Response PROGRAM_BLOCK_DATA_ACK fails"));

    } else {

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

bool CBootDevice_VSCP::sendVSCPCommandSeqenceLevel2(void) {

    // Check response
    if (!checkResponseLevel2(VSCP_TYPE_PROTOCOL_BLOCK_DATA_ACK)) {
        // Failure
        // TODO Resend the block
        wxMessageBox(_T(" Response PROTOCOL_BLOCK_DATA_ACK fails"));
    }
    else {


        if (crc_16_host != crc_16_remote) {
            m_pAddr -= falsh_memory_block_size;
            return false;
        }

    }

    wxMilliSleep(1);

    /*
     ** Send command
     */

    if (!sendVSCPBootCommand(VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA)) {
        // Failure

        wxMessageBox(_T(" PROGRAM_BLOCK_DATA TX fails"));

    } else {

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

bool CBootDevice_VSCP::checkResponseLevel1(uint8_t index) {
    //canalMsg msg;
    //time_t tstart, tnow;
    bool rv = false;

    uint16_t vscpclass;
    uint8_t vscptype;
    uint8_t nodeid;
    uint8_t priority = 0;

    canalMsg rcvmsg; // msg, 

    if ( NULL == m_pdll ) return FALSE;

    // Get system time
    //time( &tstart );

    bool bRun = true;
    while (bRun) {

        if ( m_pdll->doCmdDataAvailable() ) {

            //if(m_type==0xff)
            //wxMessageBox( _T("123456") );
            m_pdll->doCmdReceive( &rcvmsg );



            if ((int) (rcvmsg.id & 0xff) == m_guid.m_id[ 0 ]) {

                // Case -- index = 0  --- not implemented always return true 
                if (index == 0) {

                    // Response received from all - return success
                    rv = true;
                    bRun = false;

                }
                // Case -- index = 1
                if (index == VSCP_TYPE_PROTOCOL_BLOCK_DATA_ACK) {


                    vscpclass = VSCP_CLASS1_PROTOCOL;
                    vscptype = VSCP_TYPE_PROTOCOL_BLOCK_DATA_ACK;
                    nodeid = m_guid.m_id[ 0 ];

                    if ((uint32_t) (rcvmsg.id & 0x01ffffff) == (uint32_t) (((uint32_t) vscpclass << 16) | ((uint32_t) vscptype << 8) | nodeid)) {

                        // Calculate CRC in host
                        crc_16_host = crcFast(&m_pbufPrg[ m_pAddr - falsh_memory_block_size ], falsh_memory_block_size);
                        // GET CRC in remote node
                        crc_16_remote = (uint16_t) ((uint16_t) (rcvmsg.data[0] << 8) | (uint16_t) (rcvmsg.data[1]));

                        // Response received from all - return success
                        rv = true;
                        bRun = false;
                        //wxMessageBox( _T("1111") );
                        //}
                    }

                }

                if (index == VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_ACK) {

                    vscpclass = VSCP_CLASS1_PROTOCOL;
                    vscptype = VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_ACK;
                    nodeid = m_guid.m_id[ 0 ];

                    if ((uint32_t) (rcvmsg.id & 0x01ffffff) == (uint32_t) (((uint32_t) vscpclass << 16) | ((uint32_t) vscptype << 8) | nodeid)) {

                        // Response received from all - return success
                        rv = true;
                        bRun = false;
                        //wxMessageBox( _T("1111") );
                        //}
                    }

                }



            } // id found



        } // received message


    }

    return rv;
}


///////////////////////////////////////////////////////////////////////////////
// checkResponseLevel2
// Type = 20 (0x14) ACK program data block    -- for 8 byte packet received correctly by bootloader
//

bool CBootDevice_VSCP::checkResponseLevel2(uint8_t index) {
    vscpEventEx event;
    //time_t tstart, tnow;
    bool rv = false;

    if (NULL == m_ptcpip) return FALSE;


    bool bRun = true;
    while (bRun) {

        if ( m_ptcpip->doCmdDataAvailable() ) {

            m_ptcpip->doCmdReceiveEx( &event );

            if ((VSCP_CLASS1_PROTOCOL == event.vscp_class) &&
                    (m_guid.m_id[ 0 ] == event.GUID[0])) { // correct id

                // Case -- index = 0  --- not implemented always return true 
                if (index == 0) {

                    // Response received from all - return success
                    rv = true;
                    bRun = false;

                }
                // Case -- index = VSCP_TYPE_PROTOCOL_BLOCK_DATA_ACK
                if (index == VSCP_TYPE_PROTOCOL_BLOCK_DATA_ACK) {

                    if (event.vscp_type == VSCP_TYPE_PROTOCOL_BLOCK_DATA_ACK) {

                        // Calculate CRC in host
                        crc_16_host = crcFast(&m_pbufPrg[ m_pAddr - falsh_memory_block_size ], falsh_memory_block_size);
                        // GET CRC in remote node
                        crc_16_remote = (uint16_t) ((uint16_t) (event.data[16] << 8) | (uint16_t) (event.data[17]));

                        // Response received from all - return success
                        rv = true;
                        bRun = false;
                    }

                }
                // Case -- index = VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_ACK
                if (index == VSCP_TYPE_PROTOCOL_PROGRAM_BLOCK_DATA_ACK) {

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



/* EOF */
