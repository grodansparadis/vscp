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
// Copyright (C) 2000-2015 
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

    // Parse the configuration string. It should
    // have the following form
    // username;password;host;prefix;port;filename
    wxStringTokenizer tkz( pConfig, ";\n" );

    // Filename
    //if ( tkz.HasMoreTokens() ) {
    //    m_path = tkz.GetNextToken();     
    //}

    // start the workerthread
    m_pthreadWork = new CVSCPBTDetectWrkTread();
    if ( NULL != m_pthreadWork ) {
        m_pthreadWork->m_pobj = this;
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
    
    if ( m_srv.doCmdOpen( m_pobj->m_host,
                                m_pobj->m_username,
                                m_pobj->m_password ) <= 0 ) {
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

    bool bSendTokenActivity = true; // Sending Token Activity is default behaviour.
    bool bSendDetect = false;       // Sending detect events must be enabled.
    uint16_t pausTime = 1;         // default svan period is one seconds.
    uint8_t zone = 0;               // Zone for token activity.
    uint8_t subzone = 0;            // Subzone for token activity.
    uint8_t detectIndex = 0;       // Index used for detect event.
    uint8_t detectZone = 0;        // Zone used for detect event.
    uint8_t detectSubzone = 0;     // Subzone used for detect events.
    bool bDisableRadiodetect= false;    // Don't report radio discovery.

    wxString        strAddress;
    _deviceHash     deviceHashPrev;
    _deviceHash     deviceHashNow;
    CDetectedDevice *pDevice;

    int intvalue;
    bool bvalue;
    if ( m_srv.getVariableInt( m_pobj->m_prefix + _("_paustime"), &intvalue ) ) {
        if ( intvalue >= 0 ) pausTime = intvalue;
    }

    if ( m_srv.getVariableInt( m_pobj->m_prefix + _("_zone"), &intvalue ) ) {
        zone = intvalue;
    }

    if ( m_srv.getVariableInt( m_pobj->m_prefix + _("_subzone"), &intvalue ) ) {
        subzone = intvalue;
    }

    if ( m_srv.getVariableInt( m_pobj->m_prefix + _("_detectindex"), &intvalue ) ) {
        detectIndex = intvalue;
    }

    if ( m_srv.getVariableInt( m_pobj->m_prefix + _("_detectzone"), &intvalue ) ) {
        detectZone = intvalue;
    }

    if ( m_srv.getVariableInt( m_pobj->m_prefix + _("_detectsubzone"), &intvalue ) ) {
        detectSubzone = intvalue;
    }

    if ( m_srv.getVariableBool( m_pobj->m_prefix + _("_send_token_activity"), &bvalue ) ) {
        bSendTokenActivity = bvalue;
    }

    if ( m_srv.getVariableBool( m_pobj->m_prefix + _("_send_detect"), &bvalue ) ) {
        bSendDetect = bvalue;
    }

    if ( m_srv.getVariableBool( m_pobj->m_prefix + _("_disable_radio_detect"), &bvalue ) ) {
        bDisableRadiodetect = bvalue;
    }

	while ( !TestDestroy() && !m_pobj->m_bQuit ) {

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

                    vscpEventEx ievent;

                    memset( ievent.GUID, 0, 16 );   // Use interface GUID
                    ievent.vscp_class = VSCP_CLASS2_LEVEL1_INFORMATION;
                    ievent.vscp_type = VSCP_TYPE_INFORMATION_TOKEN_ACTIVITY;
                    ievent.timestamp = wxDateTime::Now().GetTicks();
                    ievent.head = VSCP_PRIORITY_NORMAL;
                    ievent.sizeData = 8;
                    ievent.data[ 0 ] = (18 << 2) + 2;   // Bluetooth device + "Released" 
                    ievent.data[ 1 ] = zone;
                    ievent.data[ 2 ] = subzone;
                    ievent.data[ 3 ] = 0;               // Frame 0
                    ievent.data[ 4 ] = pDev->m_address[ 0 ];
                    ievent.data[ 5 ] = pDev->m_address[ 1 ];
                    ievent.data[ 6 ] = pDev->m_address[ 2 ];
                    ievent.data[ 7 ] = pDev->m_address[ 3 ];

                    m_srv.doCmdSendEx( &ievent );    // Send the event

                    memset( ievent.GUID, 0, 16 );   // Use interface GUID
                    ievent.vscp_class = VSCP_CLASS2_LEVEL1_INFORMATION;
                    ievent.vscp_type = VSCP_TYPE_INFORMATION_TOKEN_ACTIVITY;
                    ievent.timestamp = wxDateTime::Now().GetMillisecond();
                    ievent.head = VSCP_PRIORITY_NORMAL;
                    ievent.sizeData = 6;
                    ievent.data[ 0 ] = (18 << 2) + 2;   // Bluetooth device + "Released" 
                    ievent.data[ 1 ] = zone;
                    ievent.data[ 2 ] = subzone;
                    ievent.data[ 3 ] = 1;               // Frame 1
                    ievent.data[ 4 ] = pDev->m_address[ 4 ];
                    ievent.data[ 5 ] = pDev->m_address[ 5 ];

                    m_srv.doCmdSendEx( &ievent );    // Sen dthe event

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
                vscpEventEx ievent;

                memset( ievent.GUID, 0, 16 );   // Use interface GUID
                ievent.vscp_class = VSCP_CLASS2_LEVEL1_INFORMATION;
                ievent.vscp_type = VSCP_TYPE_INFORMATION_TOKEN_ACTIVITY;
                ievent.timestamp = wxDateTime::Now().GetMillisecond();
                ievent.head = VSCP_PRIORITY_NORMAL;
                ievent.sizeData = 8;
                ievent.data[ 0 ] = (18 << 2) + 1;   // Bluetooth device + "Touched" 
                ievent.data[ 1 ] = zone;
                ievent.data[ 2 ] = subzone;
                ievent.data[ 3 ] = 0;               // Frame 0
                ievent.data[ 4 ] = pDev->m_address[ 0 ];
                ievent.data[ 5 ] = pDev->m_address[ 1 ];
                ievent.data[ 6 ] = pDev->m_address[ 2 ];
                ievent.data[ 7 ] = pDev->m_address[ 3 ];

                m_srv.doCmdSendEx( &ievent );    // Send the event

                memset( ievent.GUID, 0, 16 );   // Use interface GUID
                ievent.vscp_class = VSCP_CLASS2_LEVEL1_INFORMATION;
                ievent.vscp_type = VSCP_TYPE_INFORMATION_TOKEN_ACTIVITY;
                ievent.timestamp = wxDateTime::Now().GetMillisecond();
                ievent.head = VSCP_PRIORITY_NORMAL;
                ievent.sizeData = 6;
                ievent.data[ 0 ] = (18 << 2) + 1;   // Bluetooth device + "Touched" 
                ievent.data[ 1 ] = zone;
                ievent.data[ 2 ] = subzone;
                ievent.data[ 3 ] = 1;               // Frame 1
                ievent.data[ 4 ] = pDev->m_address[ 4 ];
                ievent.data[ 5 ] = pDev->m_address[ 5 ];

                m_srv.doCmdSendEx( &ievent );    // Send the event
            
            }


            if ( bSendDetect ) {
            
                // Tell the world we detected it
                vscpEventEx ievent;

                memset( ievent.GUID, 0, 16 );   // Use interface GUID
                ievent.vscp_class = VSCP_CLASS2_LEVEL1_INFORMATION;
                ievent.vscp_type = VSCP_TYPE_INFORMATION_DETECT;
                ievent.timestamp = wxDateTime::Now().GetMillisecond();
                ievent.head = VSCP_PRIORITY_NORMAL;
                ievent.sizeData = 3;
                ievent.data[ 0 ] = detectIndex;   
                ievent.data[ 1 ] = detectZone;
                ievent.data[ 2 ] = detectSubzone;

                m_srv.doCmdSendEx( &ievent );    // Send the event
            
            }

            
       
            
        }

        wxSleep( pausTime );

    } // detect loop


/*
        if ( CANAL_ERROR_SUCCESS == 
            ( rv = m_srv.doCmdBlockReceive( &event, 1000 ) ) ) {
          
            //pRecord->m_time = wxDateTime::Now();
            //m_pLog->writeEvent( &event );
          
            // We are done with the event - remove data if any
            if ( NULL != event.pdata ) {
                delete [] event.pdata;
                event.pdata = NULL;
            }

        } // Event received
*/


    
        
 

    // Close the channel
    m_srv.doCmdClose();

    return NULL;  
}

//////////////////////////////////////////////////////////////////////
// OnExit
//

void CVSCPBTDetectWrkTread::OnExit()
{
    
}
