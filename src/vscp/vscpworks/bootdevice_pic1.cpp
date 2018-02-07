// bootdevice_pic1.cpp: PIC algorithm 1 Bootloader code.
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
//

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
#include "bootdevice_pic1.h"

///////////////////////////////////////////////////////////////////////////////
// Constructor
//

CBootDevice_PIC1::CBootDevice_PIC1( CDllWrapper *pdll, 
                                        uint8_t nodeid, 
                                        bool bDeviceFound ) :
CBootDevice( pdll, nodeid, bDeviceFound )
{
    init();
}

CBootDevice_PIC1::CBootDevice_PIC1( VscpRemoteTcpIf *ptcpip, 
                                        cguid &guid, 
                                        cguid &ifguid, 
                                        bool bDeviceFound ) :
CBootDevice( ptcpip, guid, ifguid, bDeviceFound )
{
    init();
}

CBootDevice_PIC1::~CBootDevice_PIC1( void )
{
    // Clean up buffer allocations is done in base class

}


///////////////////////////////////////////////////////////////////////////////
// init
//

void CBootDevice_PIC1::init( void )
{
    // Create buffers
    m_pbufPrg = new unsigned char[ BUFFER_SIZE_PROGRAM ];
    m_pbufUserID = new unsigned char[ BUFFER_SIZE_USERID ];
    m_pbufCfg = new unsigned char[ BUFFER_SIZE_CONFIG ];
    m_pbufEEPROM = new unsigned char[ BUFFER_SIZE_EEPROM ];

    m_bHandshake = true;        // No handshake as default
    m_pAddr = 0;
    m_memtype = MEM_TYPE_PROGRAM;
    
    crcInit();
}

///////////////////////////////////////////////////////////////////////////////
// loadBinaryFile
//

bool CBootDevice_PIC1::loadBinaryFile( const wxString& path, 
                                            uint16_t typeHexfile )
{
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
    m_bFlashMemory = true;  // Flash memory should be programmed


    // Init program memory buffer
    if ( NULL == m_pbufPrg ) return false;
    memset( m_pbufPrg, 0xff, BUFFER_SIZE_PROGRAM );

    // Init. User ID memory pointers
    m_minUserIDAddr = 0xffffffff;
    m_maxUserIDAddr = 0;
    m_bUserIDData = false;
    m_bUserIDMemory = true; // Config memory should be programmed
    
    // Init UserID memory buffer
    if ( NULL == m_pbufUserID ) return false;
    memset( m_pbufUserID, 0xff, BUFFER_SIZE_USERID );

    // Init. config memory pointers
    m_minConfigAddr = 0xffffffff;
    m_maxConfigAddr = 0;
    m_bConfigData = false;
    m_bConfigMemory = true; // Config memory should be programmed

    // Init config memory buffer
    if ( NULL == m_pbufCfg ) return false;
    memset( m_pbufCfg, 0xff, BUFFER_SIZE_CONFIG );

    // Init. EEPROM memory pointers
    m_minEEPROMAddr = 0xffffffff;
    m_maxEEPROMAddr = 0;
    m_bEEPROMData = false;
    m_bEEPROMMemory = true;	// EEPROM should be programmed

    // Init EEPROM memory buffer
    if ( NULL == m_pbufEEPROM ) return false;
    memset( m_pbufEEPROM, 0xff, BUFFER_SIZE_EEPROM );
    
    FILE* fs;
#ifdef WIN32	
    if ( 0 != ( err = fopen_s( &fs, path.char_str(), "r" ) ) ) {
        return false;
    }
#else
    if ( NULL == ( fs = fopen( path.char_str(), "r" ) ) ) {
        return false;
    }
#endif

    char szLine[ MAX_PATH ];
    char szData[ 16 ];
    char *endptr;

    bool bRun = true;


    while ( bRun && ( NULL != fgets( szLine, MAX_PATH, fs ) ) ) {
    
        if ( ':' == szLine [ 0 ] ) {

            // Get data count
            memset( szData, 0, 16 );
#ifdef WIN32
            strncpy_s( szData, 16,  ( szLine + 1 ), 2 ); 
#else
            strncpy( szData, ( szLine + 1 ), 2 );
#endif
            cntData = strtoul( szData, &endptr, 16 );
            m_totalCntData += cntData;

            // Get address
            memset( szData, 0, 16 );
#ifdef WIN32
            strncpy_s( szData, 16, ( szLine + 3 ), 4 );
#else
            strncpy( szData, ( szLine + 3 ), 4 );
#endif
            lowAddr = strtoul( szData, &endptr, 16 );

            // Get record type
            memset( szData, 0, 16 );
#ifdef WIN32
            strncpy_s( szData, 16, ( szLine + 7 ), 2 );
#else
            strncpy( szData, ( szLine + 7 ), 2 );
#endif

            recType = strtoul( szData, &endptr, 16 );

            fullAddr = ( highAddr << 16 ) + lowAddr;

            // Decode the record type
            switch ( recType ) {
            
                case INTEL_LINETYPE_DATA:
                    
                    for ( i=0; i < cntData; i++ ) {
                                            
                        memset( szData, 0, 16 );
#ifdef WIN32
                        strncpy_s( szData, 16, ( szLine + ( ( i * 2 ) + 9 ) ), 2 );
#else
                        strncpy( szData, ( szLine + ( ( i * 2 ) + 9 ) ), 2 );
#endif
                        unsigned char val = 
                            (unsigned char)( strtoul( szData, &endptr, 16 ) & 0xff);

                        if ( ( fullAddr < MEMREG_PRG_END ) && 
                                ( fullAddr < BUFFER_SIZE_PROGRAM ) ) {
                        
                            // Write into program memory buffer

                            m_pbufPrg[ fullAddr ] = val;
                            m_bPrgData = true;

                            // Set min flash address
                            if ( fullAddr < m_minFlashAddr ) m_minFlashAddr = fullAddr;

                            // Set max flash address
                            if ( fullAddr > m_maxFlashAddr ) m_maxFlashAddr = fullAddr;
                            
                        }
                        else if ( ( fullAddr >= MEMREG_CONFIG_START ) && 
                                    ( ( fullAddr < MEMREG_CONFIG_START + BUFFER_SIZE_CONFIG ) ) ) {
                            
                            // Write into config memory buffer

                            m_pbufCfg[ fullAddr - MEMREG_CONFIG_START ] = val;
                            m_bConfigData = true;

                            // Set min config address
                            if ( fullAddr < m_minConfigAddr ) m_minConfigAddr = fullAddr;

                            // Set max config address
                            if ( fullAddr > m_maxConfigAddr ) m_maxConfigAddr = fullAddr;

                        }
                        else if ( ( fullAddr >= MEMREG_EEPROM_START ) && 
                                    ( ( fullAddr <= MEMREG_EEPROM_START + BUFFER_SIZE_EEPROM ) ) ) {

                            // Write into EEPROM memory buffer

                            m_pbufEEPROM[ fullAddr - MEMREG_EEPROM_START ] = val;
                            m_bEEPROMData = true;

                            // Set min EEEPROM address
                            if ( fullAddr < m_minEEPROMAddr ) m_minEEPROMAddr = fullAddr;

                            // Set max config address
                            if ( fullAddr > m_maxEEPROMAddr ) m_maxEEPROMAddr = fullAddr;

                        }

                        fullAddr = fullAddr + 1;

                    }
                    break;

                case INTEL_LINETYPE_EOF:
                    bRun = false;   // We are done
                    rv = true;
                    break;

                case INTEL_LINETYPE_EXTENDED_SEGMENT:
                    // We don't handle this
                    break;

                case INTEL_LINETYPE_EXTENDED_LINEAR :
                    memset( szData, 0, 16 );
#ifdef WIN32
                    strncpy_s( szData, 16,  ( szLine + 9 ), 4 );
#else
                    strncpy( szData, ( szLine + 9 ), 4 );
#endif
                    highAddr = strtoul( szData, &endptr, 16 );
                    break;

            }
        
        }
    }
    
    rv = true;
    
    // Flash to program if none read
    if ( !m_bPrgData ) {
        m_bFlashMemory = false;
        m_minFlashAddr = 0x00000000;
    }
    
    // Config data to program if none read
    if ( !m_bConfigData ) {
        m_bConfigMemory = false;
        m_minConfigAddr = 0x00000000;
    }
    
    // EEPROM data to program if none read
    if ( !m_bEEPROMData ) {
        m_bEEPROMMemory = false;
        m_minEEPROMAddr = 0x00000000;
    }

    fclose( fs );
    
    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// showInfo
//

void CBootDevice_PIC1::showInfo( wxHtmlWindow *phtmlWnd )
{
    wxString strInfo;
    
    // Check pointer
    if ( NULL == phtmlWnd ) return;

    // Clear HTML
    phtmlWnd->SetPage( _("") );
    
    // * * * Flash Memory * * *

    phtmlWnd->AppendToPage( 
                _("<b><u>Flash Memory</u></b><br>") );

    phtmlWnd->AppendToPage(
                _("<b>Start :</b><font color=\"#005CB9\">") );
                
    strInfo.Printf( _("0x%08X"), m_minFlashAddr );
    phtmlWnd->AppendToPage(  strInfo );

    phtmlWnd->AppendToPage( 
                _("</font><b> End :</b><font color=\"#005CB9\">") );
                
    strInfo.Printf( _("0x%08X</font><br>"), m_maxFlashAddr );
    phtmlWnd->AppendToPage(  strInfo );            
                
    if ( m_bFlashMemory ) {
        phtmlWnd->AppendToPage( 
                _("<font color=\"#348017\">Will be programmed</font><br>") );
    }
    else {
        phtmlWnd->AppendToPage( 
                _("<font color=\"#F6358A\">Will not be programmed</font><br>") );
    }

    phtmlWnd->AppendToPage(
                _("<br><br>") );
                
    // * * * UserID Memory * * *

    phtmlWnd->AppendToPage( 
                _("<b><u>UserID Memory</u></b><br>") );

    phtmlWnd->AppendToPage(
                _("<b>Start :</b>") );
                
    strInfo.Printf( _("<font color=\"#005CB9\">0x%08X</font>"), m_minUserIDAddr );
    phtmlWnd->AppendToPage(  strInfo );            

    phtmlWnd->AppendToPage( 
                _("<b> End :</b>") );
                
    strInfo.Printf( _("<font color=\"#005CB9\">0x%08X<br></font>"), m_maxUserIDAddr );
    phtmlWnd->AppendToPage(  strInfo );            
                
    if ( m_bUserIDMemory ) {
        phtmlWnd->AppendToPage( 
                _("<font color=\"#348017\">Will be programmed</font><br>") );
    }
    else {
        phtmlWnd->AppendToPage( 
                _("<font color=\"#F6358A\">Will not be programmed</font><br>") );
    }            

    phtmlWnd->AppendToPage(
                _("<br><br>") );            

    // * * * Config Memory * * *

    phtmlWnd->AppendToPage( 
                _("<b><u>Config Memory</u></b><br>") );

    phtmlWnd->AppendToPage(
                _("<b>Start :</b>") );
                
    strInfo.Printf( _("<font color=\"#005CB9\">0x%08X</font>"), m_minConfigAddr );
    phtmlWnd->AppendToPage(  strInfo );            

    phtmlWnd->AppendToPage( 
                _("<b> End :</b>") );
                
    strInfo.Printf( _("<font color=\"#005CB9\">0x%08X<br></font>"), m_maxConfigAddr );
    phtmlWnd->AppendToPage(  strInfo );            
                
    if ( m_bConfigMemory ) {
        phtmlWnd->AppendToPage( 
                _("<font color=\"#348017\">Will be programmed</font><br>") );
    }
    else {
        phtmlWnd->AppendToPage( 
                _("<font color=\"#F6358A\">Will not be programmed</font><br>") );
    }            

    phtmlWnd->AppendToPage(
                _("<br><br>") );


    // * * * EEPROM * * *

    phtmlWnd->AppendToPage( 
                _("<b><u>EEPROM Memory</u></b><br>") );

    phtmlWnd->AppendToPage( 
                _("<B>Start :</b>") );
                
    strInfo.Printf( _("<font color=\"#005CB9\">0x%08X</font>"), m_minEEPROMAddr );
    phtmlWnd->AppendToPage(  strInfo );            

    phtmlWnd->AppendToPage( 
                _("<b> End :</b>") );
                
    strInfo.Printf( _("<font color=\"#005CB9\">0x%08X<br></font>"), m_maxEEPROMAddr );
    phtmlWnd->AppendToPage(  strInfo );              
                
    if ( m_bEEPROMMemory ) {
        phtmlWnd->AppendToPage( 
                _("<font color=\"#348017\">Will be programmed</font><br>") );
    }
    else {
        phtmlWnd->AppendToPage( 
                _("<font color=\"#F6358A\">Will not be programmed</font><br>") );
    }             
                
    phtmlWnd->AppendToPage(
                _("<br><br>") );            
    
}

///////////////////////////////////////////////////////////////////////////////
// setDeviceInBootMode
//

bool CBootDevice_PIC1::setDeviceInBootMode( void )
{
    bool bRun;

    uint8_t pageSelectMsb = 0;
    uint8_t pageSelectLsb = 0;
    uint8_t guid0 = 0;
    uint8_t guid3 = 0;
    uint8_t guid5 = 0;
    uint8_t guid7 = 0;

    wxBusyCursor busy;

    if ( USE_DLL_INTERFACE == m_type ) {
    
        canalMsg msg, rcvmsg;
        time_t tstart, tnow;

        // First do a test to see if the device is already in boot mode
        // if it is 0x14nn/0x15nn should be returned (nn == nodeid).

        memset( msg.data, 0x00, 8 );

        // Send message
        msg.id = ID_PUT_BASE_INFO;
        msg.flags = CANAL_IDFLAG_EXTENDED;
        memset( msg.data, 0, 8 );
        msg.sizeData = 8;
        if ( CANAL_ERROR_SUCCESS == m_pdll->doCmdSend( &msg ) ) {

            bRun = true;

            // Get start time
            time( &tstart );

            while( bRun ) {

                time( &tnow );
                if ( (unsigned long)( tnow - tstart ) > 2 ) {
                    bRun = false;
                }

                if ( m_pdll->doCmdDataAvailable() ) { // Check if data is vaialble

                    m_pdll->doCmdReceive( &rcvmsg );

                    // Is this a read/write reply from the node?
                    if ( ( rcvmsg.id & 0xffffff ) == 
                        (uint32_t)( ID_RESPONSE_PUT_BASE_INFO + m_nodeid ) ) {
                        // yes already in bootmode - return
                        return true;
                    }

                    // Is this a read/write reply from the node?
                    if ( ( rcvmsg.id & 0xffffff ) == 
                        (uint32_t)( ID_RESPONSE_PUT_DATA + m_nodeid ) ) {
                        // yes already in bootmode - return
                        return true;
                    }

                }
            }
        }

        // Read page register Page select MSB
        if ( CANAL_ERROR_SUCCESS != m_pdll->readLevel1Register( m_nodeid,
                                                                0,
                                                                VSCP_REG_PAGE_SELECT_MSB,
                                                                &pageSelectMsb ) ) {
            return false;
        }

        // Read page register page select lsb
        if ( CANAL_ERROR_SUCCESS != m_pdll->readLevel1Register( m_nodeid,
                                                                0,
                                                                VSCP_REG_PAGE_SELECT_LSB,
                                                                &pageSelectLsb ) ) {
            return false;
        }
        
        // Read page register GUID0
        if ( CANAL_ERROR_SUCCESS != m_pdll->readLevel1Register( m_nodeid,
                                                                0,
                                                                VSCP_REG_GUID0,
                                                                &guid0 ) ) {
            return false;
        }

        // Read page register GUID3
        if ( CANAL_ERROR_SUCCESS != m_pdll->readLevel1Register( m_nodeid,
                                                                0,
                                                                VSCP_REG_GUID3,
                                                                &guid3 ) ) {
            return false;
        }

        // Read page register GUID5
        if ( CANAL_ERROR_SUCCESS != m_pdll->readLevel1Register( m_nodeid,
                                                                0,
                                                                VSCP_REG_GUID5,
                                                                &guid5 ) ) {
            return false;
        }

        // Read page register GUID7
        if ( CANAL_ERROR_SUCCESS != m_pdll->readLevel1Register( m_nodeid,
                                                                0,
                                                                VSCP_REG_GUID7,
                                                                &guid7 ) ) {
            return false;
        }

        // Set device in boot mode
        msg.data[ 0 ] = m_nodeid;	            // Nickname to read register from
        msg.data[ 1 ] = VSCP_BOOTLOADER_PIC1;	// VSCP PIC1 bootloader algorithm	
        msg.data[ 2 ] = guid0;
        msg.data[ 3 ] = guid3;
        msg.data[ 4 ] = guid5;
        msg.data[ 5 ] = guid7;
        msg.data[ 6 ] = pageSelectMsb;
        msg.data[ 7 ] = pageSelectLsb;

        // Send message
        msg.id = ( VSCP_ENTER_BOOTLODER_MODE << 8 );
        msg.flags = CANAL_IDFLAG_EXTENDED;
        msg.sizeData = 8;
        if ( CANAL_ERROR_SUCCESS == m_pdll->doCmdSend( &msg ) ) {

            bRun = true;

            // Get start time
            time( &tstart );

            while( bRun ) {

                time( &tnow );
                if ( ( unsigned long )( tnow - tstart ) > PIC_BOOTLOADER_RESPONSE_TIMEOUT ) {
                    bRun = false;
                }

                if ( m_pdll->doCmdDataAvailable() ) {

                    m_pdll->doCmdReceive( &rcvmsg );

                    // Is this a read/write reply from the node?
                    if ( ( rcvmsg.id & 0xffffff ) == (uint32_t)( ID_RESPONSE_PUT_BASE_INFO + m_nodeid ) ) {
                        // OK in bootmode - return
                        return true;
                    }

                    // Is this a read/write reply from the node?
                    if ( ( rcvmsg.id & 0xffffff ) == (uint32_t)( ID_RESPONSE_PUT_DATA + m_nodeid ) ) {
                        // OK in bootmode - return
                        return true;
                    }

                }
            }

        }

    }
    else if ( USE_TCPIP_INTERFACE == m_type ) {

        // First do a test to see if the device is already in boot mode
        // if it is 0x14nn/0x15nn should be returned (nn == nodeid).

        vscpEventEx event;
        time_t tstart, tnow;

        memset( event.data, 0, sizeof( event.data ) );
        event.head = 0;
        event.vscp_class = 512;                     // CLASS2.PROTOCOL1
        event.vscp_type = ( ID_PUT_BASE_INFO >> 8 );// Special bootloader command
        memset( event.GUID, 0, 16 );                // We use interface GUID
        event.sizeData = 16;                        // Interface GUID
        memcpy( event.data, m_ifguid.m_id, 16 );    // Address node i/f

        if ( VSCP_ERROR_SUCCESS == m_ptcpip->doCmdSendEx( &event ) ) {

            bRun = true;

            // Get start time
            time( &tstart );

            while( bRun ) {
            
                time( &tnow );
                if ( ( unsigned long )( tnow - tstart ) > PIC_BOOTLOADER_RESPONSE_TIMEOUT ) {
                    bRun = false;
                }

                if ( m_ptcpip->doCmdDataAvailable() ) {  // Message available
                
                    m_ptcpip->doCmdReceiveEx( &event );

                    if ( ( event.vscp_type == ( ID_RESPONSE_PUT_BASE_INFO >> 8 ) ) ) {  
                        // yes already in bootmode - return
                        return true;
                    }

                    if ( (  event.vscp_type == ( ID_RESPONSE_PUT_DATA >> 8 ) ) ) {  
                        // yes already in bootmode - return
                        return true;
                    }
                
                }

            } // while
        
        }

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
        event.data[ 17 ] = VSCP_BOOTLOADER_PIC1;	    // VSCP PIC1 bootloader algorithm	
        event.data[ 18 ] = guid0;
        event.data[ 19 ] = guid3;
        event.data[ 20 ] = guid5;
        event.data[ 21 ] = guid7;
        event.data[ 22 ] = pageSelectMsb;
        event.data[ 23 ] = pageSelectLsb;

        if ( VSCP_ERROR_SUCCESS == m_ptcpip->doCmdSendEx( &event ) ) {
    
            bRun = true;

            // Get start time
            time( &tstart );

            while( bRun ) {

                time( &tnow );
                if ( ( unsigned long )( tnow - tstart ) > PIC_BOOTLOADER_RESPONSE_TIMEOUT ) {
                    bRun = false;
                }

                //vscpEventEx rcvmsg;

                if ( VSCP_ERROR_SUCCESS == m_ptcpip->doCmdReceiveEx( &event ) ) {

                    // Is this a read/write reply from the node?
                    if ( ( ID_RESPONSE_PUT_BASE_INFO >> 8 ) == event.vscp_type ) {
                        // OK in bootmode - return
                        return true;
                    }

                    // Is this a read/write reply from the node?
                    if ( ( ID_RESPONSE_PUT_DATA >> 8 ) == event.vscp_type ) {
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

bool CBootDevice_PIC1::doFirmwareLoad( void )
{
    bool bRun = true;
    bool rv = true;

    wxBusyCursor busy;

    m_checksum = 0;
    uint32_t progress = 0;
    uint32_t addr;
    wxString wxStatusStr;

    uint32_t nFlashPackets = 0;
    uint32_t nConfigPackets = 0;
    uint32_t nEEPROMPackets = 0;

    // Flash memory
    if ( m_bPrgData ) {
        nFlashPackets = ( m_maxFlashAddr - m_minFlashAddr )/8;

        if ( 0 != ( ( m_maxFlashAddr - m_minFlashAddr ) % 8 ) ) {
            nFlashPackets++;
        }
    }

    // Config
    if ( m_bConfigData ) {
        nConfigPackets = ( m_maxConfigAddr - m_minConfigAddr )/8;

        if ( 0 != ( ( m_maxConfigAddr - m_minConfigAddr ) % 8 ) ) {
            nConfigPackets++;
        }
    }

    // EEPROM
    if ( m_bEEPROMData ) {
        nEEPROMPackets = ( m_maxEEPROMAddr - m_minEEPROMAddr )/8;

        if ( 0 != ( ( m_maxEEPROMAddr - m_minEEPROMAddr ) % 8 ) ) {
            nEEPROMPackets++;
        }
    }

    long nTotalPackets = nFlashPackets + nConfigPackets + nEEPROMPackets;
    wxProgressDialog* pDlg = new wxProgressDialog( _T("Boot loading in progress..."),
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
    if ( !writeDeviceControlRegs( addr, 
                                    MODE_WRT_UNLCK | MODE_AUTO_ERASE | MODE_AUTO_INC | MODE_ACK,
                                    CMD_RST_CHKSM, 
                                    0,
                                    0 ) ) {
        wxMessageBox( _T("Failed to initialize checksum at node(s).") );
        rv = false;
        bRun = false;
    }

    // * * * flash memory * * *

    if ( rv && m_bPrgData ) {
            
        // Send the start block
        addr = m_minFlashAddr;
        if ( writeDeviceControlRegs( addr ) ) {
                
            for ( unsigned long blk = 0; ( ( blk < nFlashPackets ) && bRun); blk++ ) {
                
                wxStatusStr.Printf( _("Loading flash... %0X"), addr );
                if (  !( bRun = pDlg->Update( progress, wxStatusStr ) ) ) {
                    wxMessageBox( _T("Aborted by user.") );
                    rv = false;
                    bRun = false;
                }

                if ( !writeFrimwareSector() ) {
                    wxMessageBox( _T("Failed to write flash data to node(s).") );
                    rv = false;
                    bRun = false;
                }
                
                wxMilliSleep( 1 );
                progress++;
                addr += 8;
                
            }
        }
        else {
            wxMessageBox( _T("Failed to send control info for flash data to node(s).") );	
            rv = false;
        }

    }

    // * * * config memory * * *

    if ( rv && m_bConfigData ) {

        // Send the start block
        addr = m_minConfigAddr;
        if ( writeDeviceControlRegs( addr ) ) {
            
            for ( unsigned long blk = 0; ( ( blk < nConfigPackets ) && bRun); blk++ ) {
                
                wxStatusStr.Printf(_("Loading config memory... %0X"), addr );
                if (  !( bRun = pDlg->Update( progress, wxStatusStr ) ) ) {
                    wxMessageBox( _T("Aborted by user.") );
                    rv = false;
                    bRun = false;
                }

                if ( !writeFrimwareSector() ) {
                    wxMessageBox( _T("Failed to write config data to node(s).") );
                    rv = false;
                    bRun = false;
                }
                
                wxMilliSleep( 1 );
                progress++;
                addr += 8;
                
            }
        }
        else {
            wxMessageBox( _T("Failed to send control info for config data to node(s).") );	
            rv = false;
        }
    }

    // * * * EEPROM memory * * *

    if ( rv && m_bEEPROMData ) {

        // Send the start block
        addr = m_minEEPROMAddr;
        if ( writeDeviceControlRegs( addr ) ) {
        
            for ( unsigned long blk = 0; ( ( blk < nConfigPackets ) && bRun); blk++ ) {
            
                wxStatusStr.Printf(_("Loading EEPROM memory... %0X"), addr );
                if (  !( bRun = pDlg->Update( progress, wxStatusStr ) ) ) {
                    wxMessageBox( _T("Aborted by user.") );
                    rv = false;
                    bRun = false;
                }

                if ( !writeFrimwareSector() ) {
                    wxMessageBox( _T("Failed to write EEPROM data to node(s).") );
                    rv = false;
                    bRun = false;
                }
                
                wxMilliSleep( 1 );
                progress++;
                addr += 8;
                
            }
        }
        else {
            wxMessageBox( _T("Failed to send control info for EEPROM data to node(s).") );	
            rv = false;
        }
    }


    if ( rv ) {

        // Check if checksum is correct and reset device(s)
        addr = m_minFlashAddr;
        uint16_t calc_chksum = ( 0x10000 - ( m_checksum & 0xffff ) );
        if ( !writeDeviceControlRegs( addr,
            MODE_WRT_UNLCK | MODE_AUTO_ERASE | MODE_ACK,
            CMD_CHK_RUN,
            ( calc_chksum & 0xff ),
            ( ( calc_chksum >> 8 ) & 0xff )
            ) ) {
            wxMessageBox( _T( "Failed to do finalizing and restart at node(s).Possible checksum error." ) );
            rv = false;
            bRun = false;
        }
        else {

            bool bReady = false;

            // Do the device RESET
            writeDeviceControlRegs( 0x0000,
                                    0,
                                    CMD_RESET,
                                    0,
                                    0 );

            pDlg->Update( progress, _("Reset sent.") );

            wxSleep( 5 );

            // No use to check if we got out of bootloader mode if id is init. id.
            if ( ( ( USE_DLL_INTERFACE == m_type ) && ( 0xfe != m_nodeid ) ) ||
                 ( ( USE_TCPIP_INTERFACE == m_type ) && ( 0xfe != m_guid.getAt( 15 ) ) ) ) {


                for ( int i = 0; i < 3; i++ ) {

                    // Do the device RESET
                    writeDeviceControlRegs( 0x0000,
                                            0,
                                            CMD_RESET,
                                            0,
                                            0 );

                    wxString str = wxString::Format(_("Trying to verify restart %d"), i );
                    pDlg->Update( progress, str );

                    wxSleep( 5 );

                    // Verify that clients got out of boot mode.
                    // If we can read register we are ready
                    unsigned char val;

                    if ( USE_DLL_INTERFACE == m_type ) {
                        if ( CANAL_ERROR_SUCCESS == m_pdll->readLevel1Register( m_nodeid,
                            0,
                            VSCP_REG_GUID0,
                            &val ) ) {
                            bReady = true;
                            break;
                        }
                    }
                    else if ( USE_TCPIP_INTERFACE == m_type ) {
                        if ( VSCP_ERROR_SUCCESS == m_ptcpip->readLevel2Register( VSCP_REG_GUID0,
                            0,
                            &val,
                            m_ifguid,
                            &m_guid ) ) {
                            bReady = true;
                            break;
                        }
                    }

                }
            }

            if ( !bReady ) {
                pDlg->Update( progress, _("Could not verify that device came out of reset.") );
                wxMessageBox( _T( "Could not verify that device came out of reset." ) );
                rv = false;
            }

        }

    }

    // Done
    progress = nTotalPackets;
    pDlg->Update( progress, wxStatusStr  );

    pDlg->Destroy();


    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// writeFrimwareSector
//

bool CBootDevice_PIC1::writeFrimwareSector( void )
{
    canalMsg msg;
    vscpEventEx event;
    bool rv = true;

    // Send event
    if ( USE_DLL_INTERFACE == m_type ) {
        msg.id = ID_PUT_DATA;
        msg.flags = CANAL_IDFLAG_EXTENDED;
        msg.sizeData = 8;
    }
    else if ( USE_TCPIP_INTERFACE == m_type ) {
        event.head = 0;
        event.vscp_class = 512;                     // CLASS2.PROTOCOL1
        event.vscp_type = ( ID_PUT_DATA >> 8 );
        memset( event.GUID, 0, 16 );                // We use interface GUID
        event.sizeData = 16 + 8;                    // Interface GUID
        memcpy( event.data, m_ifguid.m_id, 16 );    // Address node in i/f
    }
    else {
        return false;
    }

    uint8_t b;
    for ( int i = 0; i < 8; i++ ) {

        switch ( m_memtype ) {

            case MEM_TYPE_PROGRAM:
                b = m_pbufPrg[ m_pAddr ];
                m_checksum += m_pbufPrg[ m_pAddr ];
                break;
                
            case MEM_TYPE_USERID:
                b = m_pbufUserID[ m_pAddr-0x200000 ];
                m_checksum += m_pbufUserID[ m_pAddr-0x200000 ];
                break;    

            case MEM_TYPE_CONFIG:
                b = m_pbufCfg[ m_pAddr-0x300000 ];
                m_checksum += m_pbufCfg[ m_pAddr-0x300000 ];
                break;

            case MEM_TYPE_EEPROM:
                b = m_pbufEEPROM[ m_pAddr-0xf00000 ]; 
                m_checksum += m_pbufEEPROM[ m_pAddr-0xf00000 ];
                break;

            default:
                    b = 0xff;
                    break;
        }

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
        m_ptcpip->doCmdSendEx( &event );    
    }
    else {
        return false;
    }
    
    // Message queued - ( wait for response from client(s) ).
    if ( m_bHandshake ) {

        if ( USE_DLL_INTERFACE == m_type ) {
            if ( !checkResponseLevel1( ID_RESPONSE_PUT_DATA ) ) {
                // Failure
                rv = false;
            }
        }
        if ( USE_TCPIP_INTERFACE == m_type ) {
            if ( !checkResponseLevel2( ID_RESPONSE_PUT_DATA ) ) {
                // Failure
                rv = false;
            }   
        }
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// writeDeviceControlRegs
//

bool CBootDevice_PIC1::writeDeviceControlRegs( uint32_t addr,
                                                    uint8_t flags,
                                                    uint8_t cmd,
                                                    uint8_t cmdData0,
                                                    uint8_t cmdData1 )
{
    bool rv = true;	        // think positive ;-)
    vscpEventEx event;
    canalMsg msg;

    // Save the internal addresss
    m_pAddr = addr;

    if ( ( m_pAddr < MEMREG_PRG_END ) && ( m_pAddr < BUFFER_SIZE_PROGRAM ) ) {
                        
        // Flash memory
        m_memtype = MEM_TYPE_PROGRAM;
                            
    }
    else if ( ( m_pAddr >= MEMREG_CONFIG_START ) && 
                ( ( m_pAddr < MEMREG_CONFIG_START + BUFFER_SIZE_CONFIG ) ) ) {
                        
        // Config memory
        m_memtype = MEM_TYPE_CONFIG;

    }
    else if ( ( m_pAddr >= MEMREG_EEPROM_START ) && 
                ( ( m_pAddr <= MEMREG_EEPROM_START + BUFFER_SIZE_EEPROM ) ) ) {

        // EEPROM
        m_memtype = MEM_TYPE_EEPROM;

    }
    else {
        return false;
    }

    if ( USE_DLL_INTERFACE == m_type ) {
        
        msg.id = ID_PUT_BASE_INFO;
        msg.flags = CANAL_IDFLAG_EXTENDED;
        msg.sizeData = 8;
        msg.id += m_nodeid; // Add node id. 
        msg.data[ 0 ]  = ( unsigned char )( addr & 0xff );
        msg.data[ 1 ]  = ( unsigned char )( ( addr >> 8 ) & 0xff );
        msg.data[ 2 ]  = ( unsigned char )( ( addr >> 16 ) & 0xff );
        msg.data[ 3 ]  = 0;
        msg.data[ 4 ]  = flags;
        msg.data[ 5 ]  = cmd;
        msg.data[ 6 ]  = cmdData0;
        msg.data[ 7 ]  = cmdData1;

    }
    else if ( USE_TCPIP_INTERFACE == m_type ) {
        
        event.head = 0;
        event.vscp_class = 512;                     // CLASS2.PROTOCOL1
        event.vscp_type = ID_PUT_BASE_INFO >> 8;
        memset( event.GUID, 0, 16 );                // We use interface GUID
        event.sizeData = 16 + 8;                    // Interface GUID
        memcpy( event.data, m_ifguid.m_id, 16 );    // Address node in i/f
        event.data[ 16 ]  = ( unsigned char )( addr & 0xff );
        event.data[ 17 ]  = ( unsigned char )( ( addr >> 8 ) & 0xff );
        event.data[ 18 ]  = ( unsigned char )( ( addr >> 16 ) & 0xff );
        event.data[ 19 ]  = 0;
        event.data[ 20 ]  = flags;
        event.data[ 21 ]  = cmd;
        event.data[ 22 ]  = cmdData0;
        event.data[ 23 ]  = cmdData1;

    }
    else {
        return false;
    }

    if ( flags & MODE_ACK ) {
        m_bHandshake = true;
    }
    else {
        m_bHandshake = false;
    }

    // Send message
    if ( USE_DLL_INTERFACE == m_type ) {

        if ( CANAL_ERROR_SUCCESS == m_pdll->doCmdSend( &msg ) ) {
        
            // Message queued - ( wait for response from client(s) ).
            if ( m_bHandshake ) {
                rv = checkResponseLevel1( ID_RESPONSE_PUT_BASE_INFO );	
            }

        }
        else {
            rv = false;
        }

    }
    else {

        if ( CANAL_ERROR_SUCCESS == m_ptcpip->doCmdSendEx( &event ) ) {
        
            // Message queued - ( wait for response from client(s) ).
            if ( m_bHandshake ) {
                rv = checkResponseLevel2( ID_RESPONSE_PUT_BASE_INFO );	
            }

        }
        else {
            rv = false;
        }    

    }

    return rv;
}


///////////////////////////////////////////////////////////////////////////////
// checkResponseLevel1
//

bool CBootDevice_PIC1::checkResponseLevel1( uint32_t id )
{
    canalMsg msg;
    time_t tstart, tnow;
    bool rv = false;

    // Get system time
    time( &tstart );
    
    bool bRun = true;
    while( bRun ) {

        //if ( m_pdll->doCmdDataAvailable() ) {

            if ( CANAL_ERROR_SUCCESS == m_pdll->doCmdReceive( &msg ) ) {

                if ( ( ( msg.id & 0xffffff00 ) == id ) ) {		// correct id

                    if ( ( int )( msg.id & 0xff ) == m_nodeid ) {

                        // Response received from all - return success
                        rv = true;
                        bRun = false;
                        return true;

                    } // id found
                }

            } // id

        //} // received message

        // check for timeout
        time( &tnow );
        if ( (unsigned long)( tnow - tstart ) > PIC_BOOTLOADER_RESPONSE_TIMEOUT ) {
            rv = false;
            bRun = false;
        }

    }

    return rv;
}


///////////////////////////////////////////////////////////////////////////////
// checkResponseLevel2
//

bool CBootDevice_PIC1::checkResponseLevel2( uint32_t id )
{
    vscpEventEx event;
    time_t tstart, tnow;
    bool rv = false;

    // Get system time
    time( &tstart );

    bool bRun = true;
    while( bRun ) {

        //if ( m_ptcpip->doCmdDataAvailable() ) {

            if ( VSCP_ERROR_SUCCESS == m_ptcpip->doCmdReceiveEx( &event ) ) {

                if ( ( VSCP_CLASS1_PROTOCOL == event.vscp_class ) &&
                     ( m_guid.getLSB() == event.GUID[ 15 ] ) ) {    // correct id

                    // Response received from all - return success
                    rv = true;
                    bRun = false;
                    return true;

                } // id
            }

        //} // received message

        // check for timeout
        time( &tnow );
        if ( ( tnow - tstart ) > PIC_BOOTLOADER_RESPONSE_TIMEOUT ) {
            rv = false;
            bRun = false;
        }

    }

    return rv;
}


