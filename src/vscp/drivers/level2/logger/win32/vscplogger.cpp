//
// vscplogger.cpp : Defines the entry point for the DLL application.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2014 Ake Hedman, 
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

#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/defs.h"

#include "../common/dlldrvobj.h"
#include "../common/log.h"

WORD wVersionRequested = MAKEWORD(1, 1);	// WSA functions
WSADATA wsaData;							// WSA functions

static HANDLE hThisInstDll = NULL;
static CDllDrvObj *theApp = NULL;

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
	
     // Initialize winsock layer
    //WSAStartup(wVersionRequested, &wsaData);

    wxSocketBase::Initialize();

	CVSCPLog *pdrvObj = new CVSCPLog();
	if ( NULL != pdrvObj ) {

		if ( pdrvObj->open( pUsername, pPassword, pHost, port, pPrefix, pParameter ) ){

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

	CVSCPLog *pdrvObj =  theApp->getDriverObject( handle );
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
VSCPBlockingSend(long handle, const vscpEvent *pEvent, unsigned long timeout)
{
	int rv = 0;

	CVSCPLog *pdrvObj = theApp->getDriverObject(handle);
	if (NULL == pdrvObj) return CANAL_ERROR_MEMORY;
    pdrvObj->addEvent2SendQueue( pEvent );   
	return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
//  VSCPBlockingReceive
// 

extern "C" int
VSCPBlockingReceive(long handle, vscpEvent *pEvent, unsigned long timeout)
{
    wxMilliSleep( timeout );
    
    // Nothing to receive
    pEvent = NULL;
	return CANAL_ERROR_FIFO_EMPTY;
}


///////////////////////////////////////////////////////////////////////////////
//  VSCPGetLevel
// 

extern "C" unsigned long
VSCPGetLevel(void)
{
	return CANAL_LEVEL_USES_TCPIP;
}


///////////////////////////////////////////////////////////////////////////////
// VSCPGetDllVersion
//

extern "C" unsigned long
VSCPGetDllVersion(void)
{
	return VSCP_DLL_VERSION;
}


///////////////////////////////////////////////////////////////////////////////
// VSCPGetVendorString
//

extern "C" const char *
VSCPGetVendorString(void)
{
	return VSCP_DLL_VENDOR;
}


///////////////////////////////////////////////////////////////////////////////
// VSCPGetDriverInfo
//

extern "C" const char *
VSCPGetDriverInfo(void)
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


