// simulation.cpp: implementation of the CSim class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP Project (http://www.vscp.org) 
//
// Copyright (C) 2000-2017 Ake Hedman, 
// Grodans Paradis AB, <akhe@grodansparadis.com>
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

#ifdef WIN32
#include <winsock2.h>
#endif

#include <stdio.h>

#ifndef WIN32
#include "unistd.h"
#include "syslog.h"
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <libgen.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <net/if.h>
#include <sys/time.h>
#endif

#include "stdlib.h"
#include <string.h>
#include "limits.h"
#include <sys/types.h>
#include <signal.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>


#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"
#include <wx/xml/xml.h>
#include <wx/listimpl.cpp>
#include <wx/thread.h>
#include <wx/tokenzr.h>
#include <wx/datetime.h>
#include <wx/textfile.h>

#include <vscphelper.h>
#include <vscpremotetcpif.h>
#include <vscp_type.h>
#include <vscp_class.h>
#ifdef WIN32
#include <simdrv.h>
#else
#include <vscpl2drv_sim.h>
#endif
#include "simulation.h"

// Globals

uint8_t gdefaultGUID[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8_t gdeviceURL[] = "www.eurosource.se/vscpsim_001.xml";

uint8_t gdefaultDM[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Zero indicates end of table
double gsimdata[] = {
    20.92, 20.10, 20.00, 20.20, 20.10, 20.20, 20.40, 20.60,
    20.50, 20.60, 20.80, 21.00, 21.30, 21.60, 21.50, 21.30,
    21.70, 21.90, 22.10, 23.20, 23.30, 23.40, 23.20, 23.30,
    23.10, 23.91, 24.52, 25.13, 26.37, 27.87, 29.45, 29.57,
    29.91, 30.22, 30.33, 30.54, 30.67, 30.60, 30.45, 30.54,
    30.33, 30.44, 30.39, 30.20, 29.92, 29.91, 29.76, 29.77,
    29.61, 29.50, 29.44, 29.20, 28.77, 28.45, 28.20, 28.49,
    27.99, 28.25, 27.87, 27.30, 26.67, 25.34, 25.27, 25.19,
    24.11, 24.02, 23.87, 23.45, 23.21, 23.28, 23.01, 22.78,
    22.55, 22.43, 22.36, 21.67, 21.33, 21.12, 20.78, 20.67,
    20.54, 20.51, 20.49, 20.45, 20.41, 20.37, 20.34, 20.29,
    20.21, 20.15, 20.11, 20.09, 20.01, 20.00, 19.82, 19.43,
    0.0
};

///////////////////////////////////////////////////////////////////////////////
// getClockMilliseconds
//
//

static uint32_t getClockMilliseconds()
{
#ifdef WIN32
    return ( uint32_t )( ( 1000 * ( float )clock() ) / CLOCKS_PER_SEC );
#else
    timeval curTime;
    gettimeofday( &curTime, NULL );
    return  1000 * curTime.tv_sec + curTime.tv_usec / 1000;
#endif	
}




//////////////////////////////////////////////////////////////////////
// CSim
//

CSim::CSim()
{
    m_bQuit = false;
    for( int i = 0; i < SIM_MAX_NODES; i++ ) {
        m_pthreadWork[ i ] = NULL;
    }

    m_nNodes = 1;   // One node as default
    
    ::wxInitialize();
}

//////////////////////////////////////////////////////////////////////
// ~CSim
//

CSim::~CSim()
{
    close();
    ::wxUninitialize();
}


//////////////////////////////////////////////////////////////////////
// open
//
//

bool
CSim::open(const char *pUsername,
                    const char *pPassword,
                    const char *pHost,
                    short port,
                    const char *pPrefix,
                    const char *pConfig)
{
    bool rv = true;
    wxString wxstr = wxString::FromAscii(pConfig);

    m_usernameLocal = wxString::FromAscii(pUsername);
    m_passwordLocal = wxString::FromAscii(pPassword);
    m_hostLocal = wxString::FromAscii(pHost);
    m_portLocal = port;
    m_prefix = wxString::FromAscii(pPrefix);

    // Parse the configuration string. It should
    // have the following form
    // "numberofnodes"
    // 
    wxStringTokenizer tkz(wxString::FromAscii(pConfig), _(";\n"));

    // Check for number of nodes in configuration string
    if (tkz.HasMoreTokens()) {
        // Interface
        m_nNodes = vscp_readStringValue( tkz.GetNextToken() );
    }
    
    
    // First log on to the host and get configuration 
    // variables

    if ( VSCP_ERROR_SUCCESS != m_srvLocal.doCmdOpen( m_hostLocal,
                                                        m_usernameLocal,
                                                        m_passwordLocal) ) {
#ifndef WIN32
        syslog(LOG_ERR,
                "%s",
                (const char *) "[VSCPSimDrv]Unable to connect to VSCP TCP/IP interface. Terminating!");
#endif
        return false;
    }

    // Find the channel id
    uint32_t ChannelID;
    m_srvLocal.doCmdGetChannelID(&ChannelID);

    // The server should hold configuration data 
    // 
    // We look for 
    //
    //   _NumberOfNodes - The number of nodes we should pretend to be.
    //
    //   

    wxString str;
    int value;
    wxString strName = m_prefix +
            wxString::FromAscii("_NumberOfNodes");
    if ( VSCP_ERROR_SUCCESS == m_srvLocal.getRemoteVariableInt( strName, &value ) ) {
        m_nNodes = value;
    }

    // No more than max nodes possible
    if ( m_nNodes > SIM_MAX_NODES ) {
        m_nNodes = SIM_MAX_NODES;
    }
    
    for( int i = 0; i < m_nNodes; i++ ) {

        wxString strIteration;
        strIteration.Printf( _( "%d" ), i );
        strIteration.Trim();

        m_pthreadWork[ i ] = new CWrkTread();
        if ( NULL != m_pthreadWork[ i ] ) {

            bool bvalue;
            strName = m_prefix +
                wxString::FromAscii( "_bLevel2" ) + strIteration;
            if ( VSCP_ERROR_SUCCESS == m_srvLocal.getRemoteVariableBool( strName, &bvalue ) ) {
                m_pthreadWork[ i ]->m_bLevel2 = bvalue;
            }

            strName = m_prefix +
                wxString::FromAscii( "_zone" ) + strIteration;
            if ( VSCP_ERROR_SUCCESS == m_srvLocal.getRemoteVariableInt( strName, &value ) ) {
                m_pthreadWork[ i ]->m_registers[ SIM_USER_REG_ZONE ] = value;
            }

            strName = m_prefix +
                wxString::FromAscii( "_subzone" ) + strIteration;
            if ( VSCP_ERROR_SUCCESS == m_srvLocal.getRemoteVariableInt( strName, &value ) ) {
                m_pthreadWork[ i ]->m_registers[ SIM_USER_REG_SUBZONE ] = value;
            }

            cguid guidval;
            strName = m_prefix +
                wxString::FromAscii( "_guid" ) + strIteration;
            if ( VSCP_ERROR_SUCCESS == m_srvLocal.getRemoteVariableGUID(strName, guidval ) ) {
                m_pthreadWork[ i ]->m_guid = guidval;
                memcpy( m_pthreadWork[ i ]->m_registers + VSCP_REG_GUID, guidval.getGUID(), 16 );
            }

            wxString strvalue;
            strName = m_prefix +
                wxString::FromAscii( "_path" ) + strIteration;
            if ( VSCP_ERROR_SUCCESS == m_srvLocal.getRemoteVariableValue( strName, strvalue ) ) {
                m_pthreadWork[ i ]->m_path = strvalue;
            }

            strName = m_prefix +
                wxString::FromAscii( "_interval" ) + strIteration;
            if ( VSCP_ERROR_SUCCESS == m_srvLocal.getRemoteVariableInt( strName, &value ) ) {
                m_pthreadWork[ i ]->m_registers[ SIM_USER_REG_INTERVAL ] = value;
            }

            strName = m_prefix +
                wxString::FromAscii( "_unit" ) + strIteration;
            if ( VSCP_ERROR_SUCCESS == m_srvLocal.getRemoteVariableInt( strName, &value ) ) {
                m_pthreadWork[ i ]->m_registers[ SIM_USER_REG_UNIT ] = value;
            }

            strName = m_prefix +
                wxString::FromAscii( "_index" ) + strIteration;
            if ( VSCP_ERROR_SUCCESS == m_srvLocal.getRemoteVariableInt( strName, &value ) ) {
                m_pthreadWork[ i ]->m_registers[ SIM_USER_REG_INDEX ] = value;
            }

            strName = m_prefix +
                wxString::FromAscii( "_coding" ) + strIteration;
            if ( VSCP_ERROR_SUCCESS == m_srvLocal.getRemoteVariableInt( strName, &value ) ) {
                m_pthreadWork[ i ]->m_registers[ SIM_USER_REG_CODING ] = value;
            }

            long lvalue;
            strName = m_prefix +
                wxString::FromAscii( "_measurementclass" ) + strIteration;
            if ( VSCP_ERROR_SUCCESS == m_srvLocal.getRemoteVariableLong( strName, &lvalue ) ) {
                m_pthreadWork[ i ]->m_measurementClass = lvalue;
            }

            strName = m_prefix +
                wxString::FromAscii( "_measurementtype" ) + strIteration;
            if ( VSCP_ERROR_SUCCESS == m_srvLocal.getRemoteVariableLong( strName, &lvalue ) ) {
                m_pthreadWork[ i ]->m_measurementType = lvalue;
            }

            strName = m_prefix +
                wxString::FromAscii( "_decisionmatrix" ) + strIteration;
            if ( VSCP_ERROR_SUCCESS == m_srvLocal.getRemoteVariableValue( strName, strvalue ) ) {
                wxStringTokenizer tkz( strvalue, _( "," ) );

                for ( int k = 0; k < ( 8 * SIM_DECISION_MATRIX_ROWS ); k++ ) {
                    if ( tkz.HasMoreTokens() ) {
                        m_pthreadWork[ i ]->m_registers[ SIM_USER_REG_DECISION_MATRIX + k ] = 
                            vscp_readStringValue( tkz.GetNextToken() );
                    }
                    else {
                        break;
                    }
                }

            }
            
#ifdef WIN32
#else            
            syslog( LOG_DEBUG,
                        "[VSCPSimDrv] %s",
                        ( const char * ) "Starting worker thread." );
#endif            

            // start the workerthread
            m_pthreadWork[ i ]->m_pObj = this;
            m_pthreadWork[ i ]->Create();
            m_pthreadWork[ i ]->Run();
        
        }
        else {
#ifdef WIN32
#else
            syslog( LOG_ERR,
                        "[VSCPSimDrv] %s prefix=%s i=%d",
                        ( const char * ) "Failed to start workerthread.",
                        ( const char * )m_prefix.ToAscii(),                        
                        i );
#endif
                rv = false;
        }

    } // if

    // Close the channel
    m_srvLocal.doCmdClose();

    return rv;
}


//////////////////////////////////////////////////////////////////////
// close
//

void
CSim::close(void)
{
    // Do nothing if already terminated
    if (m_bQuit) return;

    m_bQuit = true; // terminate the thread
    wxSleep(1); // Give the thread some time to terminate
}


//////////////////////////////////////////////////////////////////////
// addEvent2SendQueue
//

bool 
CSim::addEvent2SendQueue(const vscpEvent *pEvent)
{
    m_mutexSendQueue.Lock();
    m_sendList.push_back((vscpEvent *)pEvent);
    m_semSendQueue.Post();
    m_mutexSendQueue.Unlock();
    return true;
}

//////////////////////////////////////////////////////////////////////
//                Workerthread - CWrkTread
//////////////////////////////////////////////////////////////////////

CWrkTread::CWrkTread()
{
    uint32_t index = 0;

    m_pObj = NULL;

    vscp_clearVSCPFilter( &m_vscpfilter );  // Accept all events

    m_bLevel2 = false;                       // Level I node
    m_guid.clear();
    m_path.Clear();
    
    //memset( m_standardRegs, 0, 256 );       // Nill standard regs
    
    vscp_alarmstatus = 0;
    vscp_page_select = 0;

    // Init user regs
    memset( m_registers, 0, sizeof( m_registers ) );  // Nill user regs
    m_registers[ SIM_USER_REG_INTERVAL ] = SIM_DEFAULT_INTERVAL;
    m_registers[ SIM_USER_REG_UNIT ] = SIM_DEFAULT_UNIT;
    m_registers[ SIM_USER_REG_INDEX ] = SIM_DEFAULT_INDEX;
    m_registers[ SIM_USER_REG_CODING ] = SIM_DEFAULT_CODING;
    m_measurementClass = VSCP_CLASS1_MEASUREMENT;
    m_measurementType = VSCP_TYPE_MEASUREMENT_TEMPERATURE;
    
    for( index = 0; index < SIM_DECISION_MATRIX_ROWS; ++index ) {
        memcpy( &m_registers[ SIM_USER_REG_DECISION_MATRIX + index * sizeof(gdefaultDM) ], gdefaultDM, sizeof(gdefaultDM) );
    }

    // init standard regs
    m_registers[ VSCP_REG_FIRMWARE_MAJOR_VERSION ] = VSCP_DLL_VERSION_MAJOR;
    m_registers[ VSCP_REG_FIRMWARE_MINOR_VERSION ] = VSCP_DLL_VERSION_MINOR;
    m_registers[ VSCP_REG_FIRMWARE_SUB_MINOR_VERSION ] = VSCP_DLL_VERSION_SUBMINOR;
    m_registers[ VSCP_REG_BOOT_LOADER_ALGORITHM ] = VSCP_BOOTLOADER_NONE;
    m_registers[ VSCP_REG_BUFFER_SIZE ] = 8;
    m_registers[ VSCP_REG_PAGES_USED ] = 1;
    memcpy( m_registers + VSCP_REG_GUID, gdefaultGUID, 16 );
    memcpy( m_registers + VSCP_REG_DEVICE_URL, gdeviceURL, sizeof( gdeviceURL ) );
}

CWrkTread::~CWrkTread()
{
    ;
}


//////////////////////////////////////////////////////////////////////
// readLevel1Register
//

uint8_t CWrkTread::readLevel1Register( uint8_t reg )
{
    uint8_t rv;
    if ( reg >= 0x80 ) {
        rv = readStandardReg( reg );
    }
    else {
        rv = readUserReg( reg );
    }

    return rv;
}

//////////////////////////////////////////////////////////////////////
// writeLevel1Register
//

uint8_t CWrkTread::writeLevel1Register( uint8_t reg, uint8_t val )
{
    uint8_t rv;
    if ( reg >= 0x80 ) {
        rv = writeStandardReg( reg, val );
    }
    else {
        rv = writeUserReg( reg, val );
    }

    return rv;
}

//////////////////////////////////////////////////////////////////////
// readLevel2Register
//

bool CWrkTread::readLevel2Register( uint32_t reg )
{
    return true;
}

//////////////////////////////////////////////////////////////////////
// writeLevel2Register
//

bool CWrkTread::writeLevel2Register( uint32_t reg, uint8_t val )
{
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// readUserReg
//

uint8_t CWrkTread::readUserReg( uint8_t reg )
{
    uint8_t rv = 0;

    if ( reg < 0x80 ) {
        rv = m_registers[ reg ];
    }

    return rv;
}


///////////////////////////////////////////////////////////////////////////////
// writeUserReg
//

uint8_t CWrkTread::writeUserReg( uint8_t reg, uint8_t value )
{
    uint8_t rv = 0;

    if ( reg < 0x80 ) {
        rv = m_registers[ reg ] = value;
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// readStandardReg
//

uint8_t CWrkTread::readStandardReg( uint8_t reg )
{
    uint8_t rv = 0;

    if ( VSCP_REG_ALARMSTATUS == reg ) {

        // * * * Read alarm status register * * *
        rv = vscp_alarmstatus;
        vscp_alarmstatus = 0x00; // Reset alarm status

    }
    else if ( VSCP_REG_VSCP_MAJOR_VERSION == reg ) {

        // * * * VSCP Protocol Major Version * * *
        rv = VSCP_MAJOR_VERSION;

    }
    else if ( VSCP_REG_VSCP_MINOR_VERSION == reg ) {

        // * * * VSCP Protocol Minor Version * * *
        rv = VSCP_MINOR_VERSION;

    }
    else if ( VSCP_REG_NODE_CONTROL == reg ) {

        // * * * Reserved * * *
        rv = 0;

    }
    else if ( VSCP_REG_FIRMWARE_MAJOR_VERSION == reg ) {

        // * * * Get firmware Major version * * *
        rv = SIM_OBJ_MAJOR_VERSION;

    }
    else if ( VSCP_REG_FIRMWARE_MINOR_VERSION == reg ) {

        // * * * Get firmware Minor version * * *
        rv = SIM_OBJ_MINOR_VERSION;

    }
    else if ( VSCP_REG_FIRMWARE_SUB_MINOR_VERSION == reg ) {

        // * * * Get firmware Sub Minor version * * *
        rv = SIM_OBJ_SUBMINOR_VERSION;

    }
    else if ( reg < VSCP_REG_MANUFACTUR_ID0 ) {

        // * * * User ID information * * *
        rv = m_registers[ reg ];

    }
    else if ( ( reg > VSCP_REG_USERID4 ) &&
              ( reg < VSCP_REG_NICKNAME_ID ) ) {

        // * * * Manufacturer ID information * * *
        switch ( reg - VSCP_REG_MANUFACTUR_ID0 ) {
            case 0:
                rv = 15;
                break;

            case 1:
                rv = 3;
                break;

            case 2:
                rv = 14;
                break;

            case 3:
                rv = 0;
                break;

            case 4:
                rv = 0;
                break;

            case 5:
                rv = 0;
                break;

            case 6:
                rv = 0;
                break;

            case 7:
                rv = 0;
                break;
        }

    }
    else if ( VSCP_REG_NICKNAME_ID == reg ) {

        // * * * nickname id * * *
        rv = m_guid.getLSB();

    }
    else if ( VSCP_REG_PAGE_SELECT_LSB == reg ) {

        // * * * Page select LSB * * *
        rv = ( vscp_page_select & 0xff );

    }
    else if ( VSCP_REG_PAGE_SELECT_MSB == reg ) {

        // * * * Page select MSB * * *
        rv = ( vscp_page_select >> 8 ) & 0xff;

    }
    else if ( VSCP_REG_BOOT_LOADER_ALGORITHM == reg ) {

        // * * * Boot loader algorithm * * *
        rv = 0;

    }
    else if ( VSCP_REG_BUFFER_SIZE == reg ) {

        // * * * Buffer size * * *
        rv = 8;
    }
    else if ( VSCP_REG_PAGES_USED == reg ) {

        // * * * Register Pages Used * * *
        rv = 1;

    }
    else if ( ( reg >= VSCP_REG_STANDARD_DEVICE_FAMILY_CODE ) &&
              ( reg < ( VSCP_REG_STANDARD_DEVICE_FAMILY_CODE + 4 ) ) ) {
        rv = 0; // Not used
    }
    else if ( ( reg >= VSCP_REG_STANDARD_DEVICE_TYPE_CODE ) &&
              ( reg < ( VSCP_REG_STANDARD_DEVICE_TYPE_CODE + 4 ) ) ) {
        rv = 0; // Not used
    }
    else if ( ( reg >= VSCP_REG_GUID ) &&
              ( reg < VSCP_REG_DEVICE_URL ) ) {

        // * * * GUID * * *
        rv = m_guid.getAt( reg - VSCP_REG_GUID );

    }
    else if ( reg >= VSCP_REG_DEVICE_URL ) {

        // * * * The device URL * * *
        rv = gdeviceURL[ reg - VSCP_REG_DEVICE_URL ];

    }

    return rv;
}


///////////////////////////////////////////////////////////////////////////////
// writeStandardReg
//

uint8_t CWrkTread::writeStandardReg( uint8_t reg, uint8_t value )
{
    uint8_t rv = ~value;

    if ( ( reg > ( VSCP_REG_VSCP_MINOR_VERSION + 1 ) ) &&
         ( reg < VSCP_REG_MANUFACTUR_ID0 ) ) {

        // * * * User Client ID * * *
        m_registers[ reg ] = value;
        rv = value;

    }
    else if ( VSCP_REG_PAGE_SELECT_MSB == reg ) {

        // * * * Page select register MSB * * *
        vscp_page_select = ( vscp_page_select & 0xff ) | ( ( uint16_t )value << 8 );
        rv = ( vscp_page_select >> 8 ) & 0xff;
    }
    else if ( VSCP_REG_PAGE_SELECT_LSB == reg ) {

        // * * * Page select register LSB * * *
        vscp_page_select = ( vscp_page_select & 0xff00 ) | value;
        rv = ( vscp_page_select & 0xff );
    }
    else if ( ( reg >= VSCP_REG_STANDARD_DEVICE_FAMILY_CODE ) &&
              ( reg < VSCP_REG_STANDARD_DEVICE_TYPE_CODE ) ) {
        rv = 0;
    }
    else if ( ( reg >= VSCP_REG_STANDARD_DEVICE_TYPE_CODE ) &&
              ( reg < VSCP_REG_DEFAULT_CONFIG_RESTORE ) ) {
        rv = 0;
    }
    else if ( VSCP_REG_DEFAULT_CONFIG_RESTORE == reg ) {
        rv = 0;	// false		
    }
    else {
        // return complement to indicate error
        rv = ~value;
    }

    return rv;
}

//////////////////////////////////////////////////////////////////////
// sendEvent
//

bool CWrkTread::sendEvent( vscpEventEx& eventEx )
{
    vscpEvent *pEvent = new vscpEvent();
    if ( NULL != pEvent ) {

        if ( vscp_convertVSCPfromEx( pEvent, &eventEx ) ) {

            // OK send the event
            if ( vscp_doLevel2Filter( pEvent, &m_vscpfilter ) ) {
                m_pObj->m_mutexReceiveQueue.Lock();
                m_pObj->m_receiveList.push_back( pEvent );
                m_pObj->m_semReceiveQueue.Post();
                m_pObj->m_mutexReceiveQueue.Unlock();
            }
            else {
                vscp_deleteVSCPevent( pEvent );
                return false;
            }

        }
        else {
            vscp_deleteVSCPevent( pEvent );
#ifndef WIN32
            syslog( LOG_ERR,
                    "[VSCPSimDrv] %s",
                    ( const char * ) "Failed to convert event." );
#endif
            return false;
        }

    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// doDM
//
// Do decision Matrix handling
// 
//

void CWrkTread::doDM( vscpEvent *pEvent )
{
    unsigned char i;
    unsigned char dmflags;
    unsigned short class_filter;
    unsigned short class_mask;
    unsigned char type_filter;
    unsigned char type_mask;

    // Don't deal with the protocol functionality
    if ( VSCP_CLASS1_PROTOCOL == pEvent->vscp_class ) return;

    for ( i = 0; i < SIM_DESCION_MATRIX_ROWS; i++ ) {

        // Get DM flags for this row
        dmflags = m_registers[ SIM_USER_REG_DECISION_MATRIX + 1 + ( 8 * i ) ];

        // Is the DM row enabled?
        if ( dmflags & VSCP_DM_FLAG_ENABLED ) {

            // Should the originating id be checked and if so is it the same?
            if ( ( dmflags & VSCP_DM_FLAG_CHECK_OADDR ) &&
                 ( pEvent->GUID[ 15 ] != m_registers[ SIM_USER_REG_DECISION_MATRIX + ( 8 * i ) ] ) ) {
                continue;
            }

            // Check if zone should match and if so if it match
            if ( dmflags & VSCP_DM_FLAG_CHECK_ZONE ) {
                if ( 255 != pEvent->pdata[ 1 ] ) {
                    if ( pEvent->pdata[ 1 ] != m_registers[ SIM_USER_REG_ZONE ] ) {
                        continue;
                    }
                }
            }

            // Check if subzone should match and if so if it match
            if ( dmflags & VSCP_DM_FLAG_CHECK_SUBZONE ) {
                if ( 255 != pEvent->pdata[ 2 ] ) {
                    if ( pEvent->pdata[ 2 ] != m_registers[ SIM_USER_REG_SUBZONE ] ) {
                        continue;
                    }
                }
            }

            class_filter = ( dmflags & VSCP_DM_FLAG_CLASS_FILTER ) * 256 +
                                        m_registers[ SIM_USER_REG_DECISION_MATRIX +
                                        ( 8 * i ) +
                                        VSCP_DM_POS_CLASSFILTER ];
            class_mask = ( dmflags & VSCP_DM_FLAG_CLASS_MASK ) * 256 +
                                        m_registers[ SIM_USER_REG_DECISION_MATRIX +
                                        ( 8 * i ) +
                                        VSCP_DM_POS_CLASSMASK ];
            type_filter = m_registers[ SIM_USER_REG_DECISION_MATRIX +
                                        ( 8 * i ) +
                                        VSCP_DM_POS_TYPEFILTER ];
            type_mask = m_registers[ SIM_USER_REG_DECISION_MATRIX +
                                        ( 8 * i ) +
                                        VSCP_DM_POS_TYPEMASK ];

            if ( !( ( class_filter ^ pEvent->vscp_class ) & class_mask ) &&
                 !( ( type_filter ^ pEvent->vscp_type ) & type_mask ) ) {

                // OK Trigger this action
                switch ( m_registers[ SIM_USER_REG_DECISION_MATRIX + ( 8 * i ) + VSCP_DM_POS_ACTION ] ) {

                    case SIM_ACTION_REPLY_TURNON: // Enable relays in arg. bitarry
                        doActionReplyTurnOn( pEvent, 
                                             dmflags, 
                                             m_registers[ SIM_USER_REG_DECISION_MATRIX + ( 8 * i ) + VSCP_DM_POS_ACTIONPARAM ] );
                        break;

                    case SIM_ACTION_REPLY_TURNOFF: // Enable relays in arg. bitarry
                        doActionReplyTurnOff( pEvent, 
                                              dmflags, 
                                              m_registers[ SIM_USER_REG_DECISION_MATRIX + ( 8 * i ) + VSCP_DM_POS_ACTIONPARAM ] );
                        break;
                } // case

            } // Filter/mask
        } // Row enabled
    } // for each row
}


//////////////////////////////////////////////////////////////////////
// doActionReplyTurnOn
//

void CWrkTread::doActionReplyTurnOn( vscpEvent *pEvent, uint8_t dmflags, uint8_t dmparam )
{
    vscpEventEx eventEx;
    eventEx.head = VSCP_PRIORITY_NORMAL;
    memcpy( eventEx.GUID, m_registers + VSCP_STD_REGISTER_GUID, 16 );
    eventEx.timestamp = 0;
    eventEx.sizeData = 3;
    //eventEx.data[ 0 ] = m_registers[ SIM_USER_REG_INDEX ];      // Index
    eventEx.data[ 0 ] = dmparam;
    eventEx.data[ 1 ] = pEvent->pdata[ 1 ];     // Zone
    eventEx.data[ 2 ] = pEvent->pdata[ 2 ];     // Subzone  
    eventEx.vscp_class = VSCP_CLASS1_INFORMATION;
    eventEx.vscp_type = VSCP_TYPE_INFORMATION_ON;

    sendEvent( eventEx );
}

//////////////////////////////////////////////////////////////////////
// doActionReplyTurnOff
//

void CWrkTread::doActionReplyTurnOff( vscpEvent *pEvent, uint8_t dmflags, uint8_t dmparam )
{
    vscpEventEx eventEx;
    eventEx.head = VSCP_PRIORITY_NORMAL;
    memcpy( eventEx.GUID, m_registers + VSCP_STD_REGISTER_GUID, 16 );
    eventEx.timestamp = 0;
    eventEx.sizeData = 3;
    //eventEx.data[ 0 ] = m_registers[ SIM_USER_REG_INDEX ];      // Index
    eventEx.data[ 0 ] = dmparam;
    eventEx.data[ 1 ] = pEvent->pdata[ 1 ];     // Zone
    eventEx.data[ 2 ] = pEvent->pdata[ 2 ];     // Subzone
    eventEx.vscp_class = VSCP_CLASS1_INFORMATION;
    eventEx.vscp_type = VSCP_TYPE_INFORMATION_OFF;

    sendEvent( eventEx );
}

//////////////////////////////////////////////////////////////////////
// Entry
//

void *
CWrkTread::Entry()
{
    wxString str;
    size_t pos;
    double val;
    uint16_t measurement_index = 0;
    wxTextFile tfile;
    std::list<double> simlist;
    bool bRemoteConnectionLost = false;
    vscpEventEx eventEx;
    wxLongLong lastSendEvent = ::wxGetLocalTimeMillis();
    wxLongLong lastSendHeartbeat = ::wxGetLocalTimeMillis();

    ::wxInitialize();
            
    // Check pointer to main object
    if ( NULL == m_pObj ) {

#ifndef WIN32
        syslog( LOG_ERR,
                "[VSCPSimDrv] %s",
                (const char *) "Error while opening remote VSCP TCP/IP interface. Terminating!");
#endif
        return NULL;
    }

    // Open simulation data file if it is requested.
    if ( m_path.Length() > 0 ) {
        if ( !tfile.Open( m_path ) ) {
#ifndef WIN32
            syslog( LOG_ERR,
                    "[VSCPSimDrv] %s",
                    ( const char * ) "Unable to load simulation data" );
#endif
            goto dumb_fill_data;
        }

        // Set C locale
        setlocale( LC_NUMERIC, "C" );

        // read the first line
        str = tfile.GetFirstLine();
        // Replace possible comma with period
        if ( wxNOT_FOUND != ( pos = str.Find( ',' ) ) ) {
            str[ pos ] = '.';
        }

        str.ToDouble( &val );
        simlist.push_back( val );

        // read all lines one by one
        // until the end of the file
        while ( !tfile.Eof() ) {
            str = tfile.GetNextLine();
            // Replace possible comma with period
            if ( wxNOT_FOUND != ( pos = str.Find( ',' ) ) ) {
                str[ pos ] = '.';
            }
            str.ToDouble( &val );
            simlist.push_back( val );
        }

        tfile.Close();

        // Restore locale
        setlocale( LC_CTYPE, "" );
    }
    else {

dumb_fill_data:

        int i = 0;
        while ( 1 ) {
            if ( !gsimdata[ i ] ) break;
            simlist.push_back( gsimdata[ i ] );
            i++;
        }

    }
    
#ifdef WIN32
#else            
            syslog( LOG_DEBUG,
                        "[VSCPSimDrv] %s",
                        ( const char * ) "Going into worker thread loop." );
#endif    
    
    while (!TestDestroy() && !m_pObj->m_bQuit) {

        if ( wxSEMA_TIMEOUT == m_pObj->m_semSendQueue.WaitTimeout( 500 ) ) {
         
            // Should a simulated measurement event be sent?
            if ( m_registers[ SIM_USER_REG_INTERVAL ] &&
                 ( ( ::wxGetLocalTimeMillis() - lastSendEvent ) > ( ( uint32_t )m_registers[ SIM_USER_REG_INTERVAL ] * 1000 ) ) ) {

                // Save new time
                lastSendEvent = ::wxGetLocalTimeMillis();

                // Get next value
                val = simlist.front();
                simlist.pop_front();
                simlist.push_back( val );

                eventEx.head = VSCP_PRIORITY_NORMAL;
                memcpy( eventEx.GUID, m_registers + VSCP_STD_REGISTER_GUID, 16 );
                eventEx.timestamp = 0;
                eventEx.sizeData = 0;
                eventEx.vscp_class = m_measurementClass;
                eventEx.vscp_type = m_measurementType;

                if ( VSCP_CLASS1_MEASUREMENT == m_measurementClass ) {

                    vscpEvent *pEvent = new vscpEvent();
                    
                    if ( NULL != pEvent ) {
                    
                        pEvent->pdata = NULL;
                        
                        if ( SIM_CODING_NORMALIZED == m_registers[ SIM_USER_REG_CODING ] ) {

                            if ( vscp_convertFloatToNormalizedEventData( eventEx.data,
                                                                            &eventEx.sizeData,
                                                                            val,
                                                                            m_registers[ SIM_USER_REG_UNIT ],
                                                                            m_registers[ SIM_USER_REG_INDEX ] ) ) {

                                if ( vscp_convertVSCPfromEx( pEvent, &eventEx ) ) {

                                    // OK send the event
                                    if ( vscp_doLevel2Filter( pEvent, &m_vscpfilter ) ) {
                                        m_pObj->m_mutexReceiveQueue.Lock();
                                        m_pObj->m_receiveList.push_back( pEvent );
                                        m_pObj->m_semReceiveQueue.Post();
                                        m_pObj->m_mutexReceiveQueue.Unlock();
                                    }
                                    else {
                                        vscp_deleteVSCPevent( pEvent );
                                    }

                                }
                                else {
                                    vscp_deleteVSCPevent( pEvent );
#ifndef WIN32
                                    syslog( LOG_ERR,
                                            "[VSCPSimDrv] %s",
                                            ( const char * ) "Failed to convert event." );
#endif
                                }
                            }
                            else {
                                vscp_deleteVSCPevent( pEvent );
#ifndef WIN32
                                syslog( LOG_ERR,
                                        "[VSCPSimDrv] %s",
                                        ( const char * ) "Failed to convert data." );
#endif
                            }
                        }
                        else if ( SIM_CODING_FLOAT == m_registers[ SIM_USER_REG_CODING ] ) {

                            pEvent->head = VSCP_PRIORITY_NORMAL;
                            memcpy( pEvent->GUID, m_registers + VSCP_STD_REGISTER_GUID, 16 );
                            pEvent->timestamp = 0;
                            pEvent->sizeData = 0;
                            pEvent->vscp_class = m_measurementClass;
                            pEvent->vscp_type = m_measurementType;
                            pEvent->pdata = NULL;

                            if ( vscp_makeFloatMeasurementEvent( pEvent,
                                                                    val,
                                                                    m_registers[ SIM_USER_REG_UNIT ],
                                                                    m_registers[ SIM_USER_REG_INDEX ] ) ) {

                                // OK send the event
                                if ( vscp_doLevel2Filter( pEvent, &m_vscpfilter ) ) {
                                    m_pObj->m_mutexReceiveQueue.Lock();
                                    m_pObj->m_receiveList.push_back( pEvent );
                                    m_pObj->m_semReceiveQueue.Post();
                                    m_pObj->m_mutexReceiveQueue.Unlock();
                                }
                                else {
                                    vscp_deleteVSCPevent( pEvent );
                                }

                            }
                            else {
                                vscp_deleteVSCPevent( pEvent );
#ifndef WIN32
                                syslog( LOG_ERR,
                                        "[VSCPSimDrv] %s",
                                        ( const char * ) "Failed to convert data." );
#endif
                            }
                            
                        }
                        else if ( SIM_CODING_STRING == m_registers[ SIM_USER_REG_CODING ] ) {

                            pEvent->head = VSCP_PRIORITY_NORMAL;
                            memcpy( pEvent->GUID, m_registers + VSCP_STD_REGISTER_GUID, 16 );
                            pEvent->timestamp = 0;
                            pEvent->sizeData = 0;
                            pEvent->vscp_class = m_measurementClass;
                            pEvent->vscp_type = m_measurementType;
                            pEvent->pdata = NULL;

                            if ( vscp_makeStringMeasurementEvent( pEvent,
                                                                    val,
                                                                    m_registers[ SIM_USER_REG_UNIT ],
                                                                    m_registers[ SIM_USER_REG_INDEX ] ) ) {

                                // OK send the event
                                if ( vscp_doLevel2Filter( pEvent, &m_vscpfilter ) ) {
                                    m_pObj->m_mutexReceiveQueue.Lock();
                                    m_pObj->m_receiveList.push_back( pEvent );
                                    m_pObj->m_semReceiveQueue.Post();
                                    m_pObj->m_mutexReceiveQueue.Unlock();
                                }
                                else {
                                    vscp_deleteVSCPevent( pEvent );
                                }

                            }
                            else {
                                vscp_deleteVSCPevent( pEvent );
#ifndef WIN32
                                syslog( LOG_ERR,
                                        "[VSCPSimDrv] %s",
                                        ( const char * ) "Failed to convert data." );
#endif
                            }

                        }
                    }
                }                
            }

            // Should a heartbeat event be sent (every 30 seconds)
            if ( ( ::wxGetLocalTimeMillis() - lastSendHeartbeat ) > 30000 ) {
             
                // Save new time
                lastSendHeartbeat = ::wxGetLocalTimeMillis();

                eventEx.head = VSCP_PRIORITY_NORMAL;
                memcpy( eventEx.GUID, m_registers + VSCP_STD_REGISTER_GUID, 16 );
                eventEx.timestamp = 0;
                eventEx.sizeData = 3;
                eventEx.data[ 0 ] = 0;
                eventEx.data[ 1 ] = m_registers[ SIM_USER_REG_ZONE ];       // Zone
                eventEx.data[ 2 ] = m_registers[ SIM_USER_REG_SUBZONE ];    // Subzone
                eventEx.vscp_class = VSCP_CLASS1_INFORMATION;
                eventEx.vscp_type = VSCP_TYPE_INFORMATION_NODE_HEARTBEAT;

                sendEvent( eventEx );

            }

            // Go on with work
            continue;

        } // Timeout
        

        // --------------------------------------------------------------------


        // Check if there is event(s) to send
        if ( m_pObj->m_sendList.size() ) {

            // Yes there are data to send
            m_pObj->m_mutexSendQueue.Lock();
            vscpEvent *pEvent = m_pObj->m_sendList.front();
            m_pObj->m_sendList.pop_front();
            m_pObj->m_mutexSendQueue.Unlock();

            if (NULL == pEvent) continue;

            // Just to make sure
            if ( 0 == pEvent->sizeData ) {
                pEvent->pdata = NULL;
            }
            
            // Yes there is data to handle
            if ( m_bLevel2 ) {

            }
            else {

                if ( VSCP_CLASS1_PROTOCOL == pEvent->vscp_class ) {

                    if ( VSCP_TYPE_PROTOCOL_READ_REGISTER == pEvent->vscp_type ) {
                        if ( ( NULL != pEvent->pdata ) &&
                                ( 2 == pEvent->sizeData ) &&
                                ( m_guid.getNickname() == pEvent->pdata[ 0 ] ) ) {
                            eventEx.head = VSCP_PRIORITY_NORMAL;
                            eventEx.sizeData = 2;
                            eventEx.data[ 0 ] = pEvent->pdata[1];   // Reg
                            eventEx.data[ 1 ] = readLevel1Register( pEvent->pdata[ 1 ] );
                            eventEx.vscp_class = VSCP_CLASS1_PROTOCOL;
                            eventEx.vscp_type = VSCP_TYPE_PROTOCOL_RW_RESPONSE;

                            sendEvent( eventEx );
                        }
                    }
                    else if ( VSCP_TYPE_PROTOCOL_WRITE_REGISTER == pEvent->vscp_type ) {
                        if ( ( NULL != pEvent->pdata ) &&
                                ( 3 == pEvent->sizeData ) &&
                                ( m_guid.getNickname() == pEvent->pdata[ 0 ] ) ) {
                            eventEx.head = VSCP_PRIORITY_NORMAL;
                            eventEx.sizeData = 2;
                            eventEx.data[ 0 ] = pEvent->pdata[ 1 ];   // Reg
                            eventEx.data[ 1 ] = writeLevel1Register( pEvent->pdata[ 1 ], pEvent->pdata[ 2 ] );
                            eventEx.vscp_class = VSCP_CLASS1_PROTOCOL;
                            eventEx.vscp_type = VSCP_TYPE_PROTOCOL_RW_RESPONSE;

                            sendEvent( eventEx );
                        }
                    }
                    else if ( VSCP_TYPE_PROTOCOL_PAGE_READ == pEvent->vscp_type ) {
                        if ( ( NULL != pEvent->pdata ) &&
                             ( 3 == pEvent->sizeData ) && 
                             ( m_guid.getNickname() == pEvent->pdata[ 0 ] ) ) {

                            uint8_t i;
                            uint8_t pos = 0;
                            uint8_t offset = pEvent->pdata[ 1 ];
                            uint8_t len = pEvent->pdata[ 2 ];

                            for ( i = 0; i < len; i++ ) {
                                eventEx.data[ ( i % 7 ) + 1 ] = readLevel1Register( offset + i );

                                if ( ( i % 7 ) == 6 || i == ( len - 1 ) ) {
                                    uint8_t bytes;

                                    if ( ( i % 7 ) == 6 ) {
                                        bytes = 7;
                                    }
                                    else {
                                        bytes = ( i % 7 ) + 1;
                                    }

                                    eventEx.sizeData = bytes + 1;
                                    eventEx.head = VSCP_PRIORITY_NORMAL;
                                    eventEx.vscp_class = VSCP_CLASS1_PROTOCOL;
                                    eventEx.vscp_type = VSCP_TYPE_PROTOCOL_RW_PAGE_RESPONSE;
                                    eventEx.data[ 0 ] = pos; // index

                                    // send the event
                                    sendEvent( eventEx );
                                    pos++;
                                }
                            }
                        }
                    }
                    else if ( VSCP_TYPE_PROTOCOL_PAGE_WRITE == pEvent->vscp_type ) {
                        if ( ( NULL != pEvent->pdata ) &&
                             ( 3 == pEvent->sizeData ) && 
                             ( m_guid.getNickname() == pEvent->pdata[ 0 ] ) ) {
                            uint8_t i;
                            uint8_t pos = pEvent->pdata[ 1 ];
                            uint8_t len = eventEx.sizeData-2;

                            for ( i = 0; i < len; i++ ) {
                                // Write VSCP register
                                writeLevel1Register( pos + i, pEvent->pdata[ 2 + i ] );
                                eventEx.data[ 1 + i ] = readLevel1Register( pos + i );
                            }

                            eventEx.head = VSCP_PRIORITY_NORMAL;
                            eventEx.vscp_class = VSCP_CLASS1_PROTOCOL;
                            eventEx.vscp_type = VSCP_TYPE_PROTOCOL_RW_PAGE_RESPONSE;
                            eventEx.data[ 0 ] = 0; // index
                            eventEx.sizeData = len + 1;

                            // send the event
                            sendEvent( eventEx );

                        }
                    }
                    else if ( VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_READ == pEvent->vscp_type ) {
                        if ( ( NULL != pEvent->pdata ) &&
                             ( m_guid.getNickname() == pEvent->pdata[ 0 ] ) ) {

                            uint16_t page_save;
                            uint8_t byte = 0, bytes = 0;
                            uint8_t bytes_this_time, cb;

                            // if data byte 4 of the request is present probably more than 1 register should be
                            // read/written, therefore check lower 4 bits of the flags and decide
                            if ( pEvent->sizeData > 3 ) {

                                // Number of registers was specified', thus take that value
                                bytes = pEvent->pdata[ 4 ];
                                // if number of bytes was zero we read one byte 
                                if ( 0 == bytes ) {
                                    bytes = 1;
                                }
                            }
                            else {
                                bytes = 1;
                            }

                            // Save the current page
                            page_save = vscp_page_select;

                            // Assign the requested page, this variable is used in the implementation
                            // specific function 'vscp_readAppReg()' and 'vscp_writeAppReg()' to actually
                            // switch pages there
                            vscp_page_select = ( ( pEvent->pdata[ 1 ] << 8 ) | pEvent->pdata[ 2 ] );

                            // Construct response event
                            eventEx.head = VSCP_PRIORITY_NORMAL;
                            eventEx.vscp_class = VSCP_CLASS1_PROTOCOL;
                            eventEx.vscp_type = VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE;
                            eventEx.data[ 0 ] = 0; // index of event, this is the first
                            eventEx.data[ 1 ] = pEvent->pdata[ 1 ]; // mirror page msb
                            eventEx.data[ 2 ] = pEvent->pdata[ 2 ]; // mirror page lsb

                            do {
                                // calculate bytes to transfer in this event
                                if ( ( bytes - byte ) >= 4 ) {
                                    bytes_this_time = 4;
                                }
                                else {
                                    bytes_this_time = ( bytes - byte );
                                }

                                // define length of this event
                                eventEx.sizeData = 4 + bytes_this_time;
                                eventEx.data[ 3 ] =
                                    pEvent->pdata[ 3 ] + byte; // first register in this event

                                // Put up to four registers to data space
                                for ( cb = 0; cb < bytes_this_time; cb++ ) {
                                    eventEx.data[ ( 4 + cb ) ] =
                                        readLevel1Register( ( pEvent->pdata[ 3 ] + byte + cb ) );
                                }

                                // send the event
                                sendEvent( eventEx );

                                // increment byte by bytes_this_time and the event number by one
                                byte += bytes_this_time;

                                // increment the index
                                eventEx.data[ 0 ] += 1;

                            } while ( byte < bytes );

                            // Restore the saved page
                            vscp_page_select = page_save;

                        }
                    }
                    else if ( VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_WRITE == pEvent->vscp_type ) {
                        if ( ( NULL != pEvent->pdata ) &&
                             ( m_guid.getNickname() == pEvent->pdata[ 0 ] ) ) {

                            uint8_t i;
                            uint16_t page_save;

                            // Save the current page
                            page_save = vscp_page_select;

                            // Assign the requested page
                            // specific function 'vscp_readAppReg()' and 'vscp_writeAppReg()' to actually
                            vscp_page_select = ( pEvent->pdata[ 1 ] << 8 ) | pEvent->pdata[ 2 ];

                            for ( i = pEvent->pdata[ 3 ]; // register to write
                                  // number of registers to write comes from byte length of write event
                                  // reduced by four bytes
                                  i < ( pEvent->pdata[ 3 ] + ( pEvent->sizeData - 4 ) );
                            i++ ) {
                                eventEx.data[ 4 + ( i - pEvent->pdata[ 3 ] ) ] =
                                    writeLevel1Register( i, pEvent->pdata[ 4 + ( i - pEvent->pdata[ 3 ] ) ] );
                            }

                            // Restore the saved page
                            vscp_page_select = page_save;

                            eventEx.head = VSCP_PRIORITY_NORMAL;
                            eventEx.sizeData = 4 + ( pEvent->sizeData - 4 );
                            eventEx.vscp_class = VSCP_CLASS1_PROTOCOL;
                            eventEx.vscp_type = VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE;
                            eventEx.data[ 0 ] = 0; // index of event, this is the first and only
                            eventEx.data[ 1 ] = pEvent->pdata[ 1 ]; // mirror page msb
                            eventEx.data[ 2 ] = pEvent->pdata[ 2 ]; // mirror page lsb
                            eventEx.data[ 3 ] = pEvent->pdata[ 3 ]; // Register

                            // send the event
                            sendEvent( eventEx );

                        }

                    }

                }

                // Feed through matrix
                doDM( pEvent );
            }

            // We are done with the event
            vscp_deleteVSCPevent_v2( &pEvent );
            
        } // Send list size
        
    } // while

    return NULL;

}

//////////////////////////////////////////////////////////////////////
// OnExit
//

void
CWrkTread::OnExit()
{
    ;
}






