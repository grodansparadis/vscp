//
// vscpbtdetect.cpp : Defines the entry point for the DLL application.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2019 Ake Hedman, 
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

#ifdef WIN32
#include <winsock2.h>
#endif

//#define WIN32_LEAN_AND_MEAN
// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
//#include <windows.h>
//#include <stdlib.h>

#include "../common/dlldrvobj.h"


#ifdef WIN32
#include <stdio.h>
#include <stdlib.h>
//#include "winsock.h"
//#include <winsock2.h>
#include <bthdef.h>
#include <BluetoothAPIs.h>
#endif

#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"
#include <wx/xml/xml.h>
#include <wx/listimpl.cpp>
#include <wx/thread.h>
#include <wx/tokenzr.h>
#include <wx/datetime.h>
#include <wx/utils.h>
#include <vscphelper.h>
#include <vscpremotetcpif.h>
#include "vscpbtdetect.h"

static HANDLE hThisInstDll = NULL;
static CDllDrvObj *theApp = NULL;

#pragma comment(lib, "Irprops.lib")

WX_DECLARE_STRING_HASH_MAP( int, _addressHash );
WX_DECLARE_HASH_MAP( wxString, CDetectedDevice*, wxStringHash, wxStringEqual, _deviceHash );

#define BT_ADDRESS_ASSIGNED         777     // A discovered BT address has been assigned
#define BT_ADDRESS_ASSIGNED_AGAIN   888     // A discovered BT address has been found again

///////////////////////////////////////////////////////////////////////////////
// DllMain
//

BOOL APIENTRY DllMain( HANDLE hInstDll, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch( ul_reason_for_call ) {

	    case DLL_PROCESS_ATTACH:
            ::wxInitialize();
			hThisInstDll = hInstDll;
			theApp = new CDllDrvObj();
			theApp->InitInstance();
			break;

		case DLL_THREAD_ATTACH:
			if ( NULL == theApp ) delete theApp;
			break;

		case DLL_THREAD_DETACH:
 			break;

		case DLL_PROCESS_DETACH:
			::wxUninitialize();
 			break;
   }

	return TRUE;

}




///////////////////////////////////////////////////////////////////////////////
//                         V S C P   D R I V E R -  A P I
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
// VSCPOpen
//

#ifdef WIN32
extern "C" long WINAPI EXPORT VSCPOpen( const char *pUsername,
                                            const char *pPassword,
                                            const char *pHost,
                                            short port,
                                            const char *pPrefix,
                                            const char *pParameter, 
                                            unsigned long flags )
#else
extern "C" long VSCPOpen( const char *pUsername,
                                            const char *pPassword,
                                            const char *pHost,
                                            short port,
                                            const char *pPrefix,
                                            const char *pParameter, 
                                            unsigned long flags )
#endif
{
	long h = 0;
	
	CVSCPBTDetect *pdrvObj = new CVSCPBTDetect();
	if ( NULL != pdrvObj ) {

		if ( pdrvObj->open( pUsername, pPassword, pHost, port, pPrefix, pParameter, flags ) ){

			if ( !( h = theApp->addDriverObject( pdrvObj ) ) ) {
				delete pdrvObj;
			}

		}
		else {
			delete pdrvObj;
		}

	}
 
	return h;
}


///////////////////////////////////////////////////////////////////////////////
//  VSCPClose
// 

#ifdef WIN32
extern "C" int  WINAPI EXPORT VSCPClose( long handle )
#else
extern "C" int VSCPClose( long handle )
#endif
{
	int rv = 0;

	CVSCPBTDetect *pdrvObj =  theApp->getDriverObject( handle );
	if ( NULL == pdrvObj ) return 0;
	pdrvObj->close();
	theApp->removeDriverObject( handle );
	rv = 1;
	return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
//  VSCPBlockingSend
// 

extern "C" int
VSCPBlockingSend( long handle, const vscpEvent *pEvent, unsigned long timeout )
{
    /*int rv = 0;

    CVSCPBTDetect *pdrvObj = theApp->getDriverObject( handle );
    if ( NULL == pdrvObj ) return CANAL_ERROR_MEMORY;

    pdrvObj->addEvent2SendQueue( pEvent );*/

    // No transmit available
    return CANAL_ERROR_NOT_SUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
//  VSCPBlockingReceive
// 

extern "C" int
VSCPBlockingReceive( long handle, vscpEvent *pEvent, unsigned long timeout )
{
    int rv = 0;

    // Check pointer
    if ( NULL == pEvent ) return CANAL_ERROR_PARAMETER;

    CVSCPBTDetect *pdrvObj = theApp->getDriverObject( handle );
    if ( NULL == pdrvObj ) return CANAL_ERROR_MEMORY;

    if ( wxSEMA_TIMEOUT == pdrvObj->m_semReceiveQueue.WaitTimeout( timeout ) ) {
        return CANAL_ERROR_TIMEOUT;
    }

    pdrvObj->m_mutexReceiveQueue.Lock();
    vscpEvent *pLocalEvent = pdrvObj->m_receiveList.front();
    pdrvObj->m_receiveList.pop_front();
    pdrvObj->m_mutexReceiveQueue.Unlock();
    if ( NULL == pLocalEvent ) return CANAL_ERROR_MEMORY;

    vscp_copyVSCPEvent( pEvent, pLocalEvent );
    vscp_deleteVSCPevent( pLocalEvent );

    return CANAL_ERROR_SUCCESS;
}



///////////////////////////////////////////////////////////////////////////////
//  VSCPGetLevel
// 

#ifdef WIN32
extern "C" unsigned long WINAPI EXPORT VSCPGetLevel( void )
#else
extern "C" unsigned long VSCPGetLevel( void )
#endif
{
	return CANAL_LEVEL_USES_TCPIP;
}


///////////////////////////////////////////////////////////////////////////////
// VSCPGetDllVersion
//

#ifdef WIN32
extern "C" unsigned long WINAPI EXPORT VSCPGetDllVersion( void )
#else
extern "C" unsigned long VSCPGetDllVersion( void )
#endif
{
	return DLL_VERSION;
}


///////////////////////////////////////////////////////////////////////////////
// VSCPGetVendorString
//

#ifdef WIN32
extern "C" const char * WINAPI EXPORT VSCPGetVendorString( void )
#else
extern "C" const char * VSCPGetVendorString( void )
#endif
{
	return VSCP_DLL_VENDOR;
}


///////////////////////////////////////////////////////////////////////////////
// VSCPGetDriverInfo
//

#ifdef WIN32
extern "C" const char * WINAPI EXPORT VSCPGetDriverInfo( void )
#else
extern "C" const char * VSCPGetDriverInfo( void )
#endif
{
	return VSCP_LOGGER_DRIVERINFO;
}

///////////////////////////////////////////////////////////////////////////////
//  VSCPGetVSCPGetWebPageTemplate
// 

extern "C" long
VSCPGetWebPageTemplate( long handle, const char *url, char *page )
{
    page = NULL;

    // Not implemented
    return -1;
}


///////////////////////////////////////////////////////////////////////////////
//  VSCPGetVSCPWebPageInfo
// 

extern "C" int
VSCPGetWebPageInfo( long handle, const struct vscpextwebpageinfo *info )
{
    // Not implemented
    return -1;
}


///////////////////////////////////////////////////////////////////////////////
//  VSCPWebPageupdate
// 

extern "C" int
VSCPWebPageupdate( long handle, const char *url )
{
    // Not implemented
    return -1;
}

///////////////////////////////////////////////////////////////////////////////
//                            T H E  C O D E
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// Constructor
//

CVSCPBTDetect::CVSCPBTDetect()
{
    m_bQuit = false;
    m_pthreadWork = NULL;
    ::wxInitialize();
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CVSCPBTDetect::~CVSCPBTDetect()
{
    close();
    ::wxUninitialize();
}


///////////////////////////////////////////////////////////////////////////////
// open
//

bool CVSCPBTDetect::open( const char *pUsername,
                            const char *pPassword,
                            const char *pHost,
                            short port,
                            const char *pPrefix,
                            const char *pConfig, 
                            unsigned long flags )
{
    bool rv = true;
    m_flags = flags;
    wxString wxstr = pConfig;

    m_username = pUsername;
    m_password = pPassword;
    m_host = pHost;
    m_port = port;
    m_prefix = pPrefix;

    // No configuration string to parse

    // start the workerthread
    m_pthreadWork = new CVSCPBTDetectWrkTread();
    if ( NULL != m_pthreadWork ) {
        m_pthreadWork->m_pObj = this;
        m_pthreadWork->Create();
        m_pthreadWork->Run();
    }
    else {
        rv = false;
    }

	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// close
//

void CVSCPBTDetect::close( void )
{
	// Do nothing if already terminated
	if ( m_bQuit ) return;
	
	m_bQuit = true;     // terminate the thread
    wxSleep( 1 );       // Give the thread some time to terminate
}


//////////////////////////////////////////////////////////////////////
// addEvent2SendQueue
//

bool
CVSCPBTDetect::addEvent2SendQueue( const vscpEvent *pEvent )
{
    m_mutexSendQueue.Lock();
    m_sendList.push_back( ( vscpEvent * )pEvent );
    m_semSendQueue.Post();
    m_mutexSendQueue.Unlock();
    return true;
}


///////////////////////////////////////////////////////////////////////////////
//                             CDetectedDevice
///////////////////////////////////////////////////////////////////////////////




CDetectedDevice::CDetectedDevice()
{
    m_radio_id = 0;
    m_device_id = 0;
    memset( m_address, 0, sizeof( m_address ) );
    m_name = _("");
    m_class = 0;
    m_manufacturer = 0;
    m_bfound = false;
}

CDetectedDevice::~CDetectedDevice()
{


}






///////////////////////////////////////////////////////////////////////////////
//                           W O R K E R   T H R E A D
///////////////////////////////////////////////////////////////////////////////




CVSCPBTDetectWrkTread::CVSCPBTDetectWrkTread()
{
    
}


CVSCPBTDetectWrkTread::~CVSCPBTDetectWrkTread()
{

}


//////////////////////////////////////////////////////////////////////
// sendEvent
//

bool CVSCPBTDetectWrkTread::sendEvent( vscpEventEx& eventEx )
{
    vscpEvent *pEvent = new vscpEvent();
    if ( NULL != pEvent ) {

        if ( vscp_convertVSCPfromEx( pEvent, &eventEx ) ) {

            // OK send the event
            m_pObj->m_mutexReceiveQueue.Lock();
            m_pObj->m_receiveList.push_back( pEvent );
            m_pObj->m_semReceiveQueue.Post();
            m_pObj->m_mutexReceiveQueue.Unlock();

        }
        else {
            vscp_deleteVSCPevent( pEvent );
#ifndef WIN32
            syslog( LOG_ERR,
                    "%s",
                    ( const char * ) "Failed to convert event." );
#endif
            return false;
        }

    }

    return true;
}


//////////////////////////////////////////////////////////////////////
// Entry
//

void *CVSCPBTDetectWrkTread::Entry()
{
    BLUETOOTH_FIND_RADIO_PARAMS bt_find_radio = {
        sizeof( BLUETOOTH_FIND_RADIO_PARAMS )
    };

    BLUETOOTH_RADIO_INFO bt_info = {
        sizeof( BLUETOOTH_RADIO_INFO ),
        0,
    };

    BLUETOOTH_DEVICE_SEARCH_PARAMS search_params = {
        sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS),
        1,
        0,
        1,
        1,
        1,
        1,  // Timeout
        NULL
    };

    BLUETOOTH_DEVICE_INFO device_info = {
        sizeof( BLUETOOTH_DEVICE_INFO ),
        0,
    };

    HANDLE radio = NULL;
    HBLUETOOTH_RADIO_FIND bt = NULL;
    HBLUETOOTH_DEVICE_FIND bt_dev = NULL;

    // First log on to the host and get configuration 
    // variables
    
    if ( VSCP_ERROR_SUCCESS != m_srv.doCmdOpen( m_pObj->m_host,
                                                    m_pObj->m_username,
                                                    m_pObj->m_password ) ) {
        return NULL;
    }

    // Find the channel id
    uint32_t ChannelID;
    m_srv.doCmdGetChannelID( &ChannelID );

    // It is possible that there is configuration data the server holds 
    // that we need to read in. 
    // We look for 
    //      prefix_period Seconds between scans.
    //      prefix_zone Zone to use for Token Activity
    //      prefix_subzone Subzone to use for Token Activity
    //      prefix_bsendtoken Send Token activity. Class=20 (Information, Type=37 
    //      prefix_bsenddetect Send detect event. Class=20 (Information), Type=49
    //      prefix_detectzone
    //      prefix_detectsubzone
    //
    //      Future defines.
    //      ===============
    //      prefix_onaddresses - List with addresses that will generate on-event
    //      prefix_offaddresses - List with addresses that will generate off-event

    bool bSendTokenActivity = true;     // Sending Token Activity is default behaviour.
    bool bSendDetect = false;           // Sending detect events must be enabled.
    uint16_t pausTime = 1;              // default svan period is one seconds.
    uint8_t zone = 0;                   // Zone for token activity.
    uint8_t subzone = 0;                // Subzone for token activity.
    uint8_t detectIndex = 0;            // Index used for detect event.
    uint8_t detectZone = 0;             // Zone used for detect event.
    uint8_t detectSubzone = 0;          // Subzone used for detect events.
    bool bDisableRadiodetect= false;    // Don't report radio discovery.

    wxString        strAddress;
    _deviceHash     deviceHashPrev;
    _deviceHash     deviceHashNow;
    CDetectedDevice *pDevice;

    int intvalue;
    bool bvalue;
    if ( VSCP_ERROR_SUCCESS == m_srv.getRemoteVariableInt( m_pObj->m_prefix + _( "_pausetime" ), &intvalue ) ) {
        if ( intvalue >= 0 ) pausTime = intvalue;
    }

    if ( VSCP_ERROR_SUCCESS == m_srv.getRemoteVariableInt( m_pObj->m_prefix + _( "_zone" ), &intvalue ) ) {
        zone = intvalue;
    }

    if ( VSCP_ERROR_SUCCESS == m_srv.getRemoteVariableInt( m_pObj->m_prefix + _( "_subzone" ), &intvalue ) ) {
        subzone = intvalue;
    }

    if ( VSCP_ERROR_SUCCESS == m_srv.getRemoteVariableInt( m_pObj->m_prefix + _( "_detectindex" ), &intvalue ) ) {
        detectIndex = intvalue;
    }

    if ( VSCP_ERROR_SUCCESS == m_srv.getRemoteVariableInt( m_pObj->m_prefix + _( "_detectzone" ), &intvalue ) ) {
        detectZone = intvalue;
    }

    if ( VSCP_ERROR_SUCCESS == m_srv.getRemoteVariableInt( m_pObj->m_prefix + _( "_detectsubzone" ), &intvalue ) ) {
        detectSubzone = intvalue;
    }

    if ( VSCP_ERROR_SUCCESS == m_srv.getRemoteVariableBool( m_pObj->m_prefix + _( "_send_token_activity" ), &bvalue ) ) {
        bSendTokenActivity = bvalue;
    }

    if ( VSCP_ERROR_SUCCESS == m_srv.getRemoteVariableBool( m_pObj->m_prefix + _( "_send_detect" ), &bvalue ) ) {
        bSendDetect = bvalue;
    }

    if ( VSCP_ERROR_SUCCESS == m_srv.getRemoteVariableBool( m_pObj->m_prefix + _( "_disable_radio_detect" ), &bvalue ) ) {
        bDisableRadiodetect = bvalue;
    }

    // Close the channel
    m_srv.doCmdClose();

    while ( !TestDestroy() && !m_pObj->m_bQuit ) {

        bt = BluetoothFindFirstRadio( &bt_find_radio, &radio );
    
        int radio_id = 0;
        do {

            radio_id++;

            if ( ERROR_SUCCESS == BluetoothGetRadioInfo( radio, &bt_info ) ) {

                strAddress.Printf( _("%02x:%02x:%02x:%02x:%02x:%02x"), 
                                        bt_info.address.rgBytes[ 0 ], 
                                        bt_info.address.rgBytes[ 1 ], 
                                        bt_info.address.rgBytes[ 2 ], 
                                        bt_info.address.rgBytes[ 3 ], 
                                        bt_info.address.rgBytes[ 4 ], 
                                        bt_info.address.rgBytes[ 5 ] );

                if ( deviceHashPrev.count( strAddress ) &&
                    ( NULL != ( pDevice = deviceHashPrev[ strAddress ] ) ) ) {
                    // Address is known and in the table 
                    pDevice->m_bfound = true;
                }
                else {

                    if ( !bDisableRadiodetect ) {

                        // Add the new device 
                        pDevice = new CDetectedDevice();
                        if ( NULL != pDevice ) {
                            pDevice->m_radio_id = radio_id;
                            pDevice->m_device_id = 0;
                            memcpy( pDevice->m_address, bt_info.address.rgBytes, 6 );
                            pDevice->m_name = bt_info.szName;
                            pDevice->m_class = bt_info.ulClassofDevice;
                            pDevice->m_manufacturer = bt_info.manufacturer;
                            deviceHashNow[ strAddress ] = pDevice;
                        }

                    }

                }

                search_params.hRadio = radio;

                ::ZeroMemory( &device_info, sizeof( BLUETOOTH_DEVICE_INFO ) );
                device_info.dwSize = sizeof( BLUETOOTH_DEVICE_INFO );

                if ( NULL != ( bt_dev = BluetoothFindFirstDevice( &search_params, &device_info ) ) ) {

                    int device_id = 0;
                    do {

                        device_id++;

                        strAddress.Printf( _("%02x:%02x:%02x:%02x:%02x:%02x"), 
                                                device_info.Address.rgBytes[ 0 ], 
                                                device_info.Address.rgBytes[ 1 ], 
                                                device_info.Address.rgBytes[ 2 ], 
                                                device_info.Address.rgBytes[ 3 ], 
                                                device_info.Address.rgBytes[ 4 ], 
                                                device_info.Address.rgBytes[ 5 ]);

                        if ( deviceHashPrev.count( strAddress ) &&
                                ( NULL != ( pDevice = deviceHashPrev[ strAddress ] ) ) ) {
                            // Address is known and in the table 
                            pDevice->m_bfound = true;
                        }
                        else {
                            
                            // Add the new device 
                            pDevice = new CDetectedDevice();
                            if ( NULL != pDevice ) {
                                pDevice->m_radio_id = radio_id;
                                pDevice->m_device_id = device_id;
                                memcpy( pDevice->m_address, device_info.Address.rgBytes, 6 );
                                pDevice->m_name = device_info.szName;
                                pDevice->m_class = device_info.ulClassofDevice;
                                memcpy( &pDevice->m_lastSeen, &device_info.stLastSeen, sizeof( SYSTEMTIME ) );
                                deviceHashNow[ strAddress ] = pDevice;
                            }   
                          
                        }

                    } while ( BluetoothFindNextDevice( bt_dev, &device_info ) );

                    BluetoothFindDeviceClose( bt_dev );
                }
            }

        } while ( BluetoothFindNextRadio( &bt_find_radio, &radio ) );

        BluetoothFindRadioClose( bt );

        // Go through devices that have been lost since last run
        
        _deviceHash::iterator itPrev;
        for ( itPrev = deviceHashPrev.begin(); itPrev != deviceHashPrev.end(); ++itPrev ) {

            wxString key = itPrev->first;
            CDetectedDevice *pDev = itPrev->second;
            if ( NULL == pDev ) {
                continue;
            }
            
            if ( false == pDev->m_bfound ) {

                if ( bSendTokenActivity ) {

                    vscpEventEx evx;

                    memset( evx.GUID, 0, 16 );   // Use interface GUID
                    evx.vscp_class = VSCP_CLASS2_LEVEL1_INFORMATION;
                    evx.vscp_type = VSCP_TYPE_INFORMATION_TOKEN_ACTIVITY;
                    evx.timestamp = wxDateTime::Now().GetTicks();
                    evx.head = VSCP_PRIORITY_NORMAL;
                    evx.sizeData = 8;
                    evx.data[ 0 ] = ( 18 << 2 ) + 2;   // Bluetooth device + "Released" 
                    evx.data[ 1 ] = zone;
                    evx.data[ 2 ] = subzone;
                    evx.data[ 3 ] = 0;               // Frame 0
                    evx.data[ 4 ] = pDev->m_address[ 0 ];
                    evx.data[ 5 ] = pDev->m_address[ 1 ];
                    evx.data[ 6 ] = pDev->m_address[ 2 ];
                    evx.data[ 7 ] = pDev->m_address[ 3 ];
   
                    sendEvent( evx ); // Send the event

                    memset( evx.GUID, 0, 16 );   // Use interface GUID
                    evx.vscp_class = VSCP_CLASS2_LEVEL1_INFORMATION;
                    evx.vscp_type = VSCP_TYPE_INFORMATION_TOKEN_ACTIVITY;
                    evx.timestamp = wxDateTime::Now().GetMillisecond();
                    evx.head = VSCP_PRIORITY_NORMAL;
                    evx.sizeData = 6;
                    evx.data[ 0 ] = ( 18 << 2 ) + 2;   // Bluetooth device + "Released" 
                    evx.data[ 1 ] = zone;
                    evx.data[ 2 ] = subzone;
                    evx.data[ 3 ] = 1;               // Frame 1
                    evx.data[ 4 ] = pDev->m_address[ 4 ];
                    evx.data[ 5 ] = pDev->m_address[ 5 ];

                    sendEvent( evx );

                }

                // Remove it
                delete pDev;
                deviceHashPrev[ key ] = NULL;

            }
            else {

                // Prepare for next detection loop
                pDev->m_bfound = false;

            }

        }

        // Go through devices that have been found since last run
        
        _deviceHash::iterator itNow;
        for ( itNow = deviceHashNow.begin(); itNow != deviceHashNow.end(); ++itNow ) {
            
            wxString key = itNow->first;
            CDetectedDevice *pDev = itNow->second;

            if ( NULL == pDev ) {
                continue;
            }

            // Save it in previously found devices
            deviceHashPrev[ key ] =  pDev;
            deviceHashNow[ key ] = NULL;

            if ( bSendTokenActivity ) {

                // Tell the world we found it
                vscpEventEx evx;

                memset( evx.GUID, 0, 16 );   // Use interface GUID
                evx.vscp_class = VSCP_CLASS2_LEVEL1_INFORMATION;
                evx.vscp_type = VSCP_TYPE_INFORMATION_TOKEN_ACTIVITY;
                evx.timestamp = wxDateTime::Now().GetMillisecond();
                evx.head = VSCP_PRIORITY_NORMAL;
                evx.sizeData = 8;
                evx.data[ 0 ] = ( 18 << 2 ) + 1;   // Bluetooth device + "Touched" 
                evx.data[ 1 ] = zone;
                evx.data[ 2 ] = subzone;
                evx.data[ 3 ] = 0;               // Frame 0
                evx.data[ 4 ] = pDev->m_address[ 0 ];
                evx.data[ 5 ] = pDev->m_address[ 1 ];
                evx.data[ 6 ] = pDev->m_address[ 2 ];
                evx.data[ 7 ] = pDev->m_address[ 3 ];

                sendEvent( evx );    // Send the event

                memset( evx.GUID, 0, 16 );   // Use interface GUID
                evx.vscp_class = VSCP_CLASS2_LEVEL1_INFORMATION;
                evx.vscp_type = VSCP_TYPE_INFORMATION_TOKEN_ACTIVITY;
                evx.timestamp = wxDateTime::Now().GetMillisecond();
                evx.head = VSCP_PRIORITY_NORMAL;
                evx.sizeData = 6;
                evx.data[ 0 ] = ( 18 << 2 ) + 1;   // Bluetooth device + "Touched" 
                evx.data[ 1 ] = zone;
                evx.data[ 2 ] = subzone;
                evx.data[ 3 ] = 1;               // Frame 1
                evx.data[ 4 ] = pDev->m_address[ 4 ];
                evx.data[ 5 ] = pDev->m_address[ 5 ];

                sendEvent( evx );    // Send the event
            
            }


            if ( bSendDetect ) {
            
                // Tell the world we detected it
                vscpEventEx evx;

                memset( evx.GUID, 0, 16 );   // Use interface GUID
                evx.vscp_class = VSCP_CLASS2_LEVEL1_INFORMATION;
                evx.vscp_type = VSCP_TYPE_INFORMATION_DETECT;
                evx.timestamp = wxDateTime::Now().GetMillisecond();
                evx.head = VSCP_PRIORITY_NORMAL;
                evx.sizeData = 3;
                evx.data[ 0 ] = detectIndex;
                evx.data[ 1 ] = detectZone;
                evx.data[ 2 ] = detectSubzone;

                sendEvent( evx );    // Send the event
            
            }       
            
        }

        wxSleep( pausTime );

    } // detect loop

    return NULL;  
}

//////////////////////////////////////////////////////////////////////
// OnExit
//

void CVSCPBTDetectWrkTread::OnExit()
{
    
}
