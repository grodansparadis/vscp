//
// can232drv.cpp : Defines the entry point for the DLL application.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2014 
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
// $RCSfile: can232drv.cpp,v $                                       
// $Date: 2005/01/05 12:16:13 $                                  
// $Author: akhe $                                              
// $Revision: 1.4 $ 

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>

#include "../../common/dlldrvobj.h"
#include "../../common/can232obj.h"

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
			hThisInstDll = hInstDll;
			theApp = new CDllDrvObj();
			theApp->InitInstance();
			break;

		case DLL_THREAD_ATTACH:
			break;

		case DLL_THREAD_DETACH:
 			break;

		case DLL_PROCESS_DETACH:
			if ( NULL != theApp ) delete theApp;
 			break;
   }

	return TRUE;

}

///////////////////////////////////////////////////////////////////////////////
//                             C A N A L -  A P I
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CanalOpen
//

#ifdef WIN32
extern "C" long WINAPI EXPORT CanalOpen( const char *pDevice, unsigned long flags )
#else
extern "C" long CanalOpen( const char *pDevice, unsigned long flags )
#endif
{
	long h = 0;
	
	CCAN232Obj *pdrvObj = new CCAN232Obj();
	if ( NULL != pdrvObj ) {

		if ( pdrvObj->open( pDevice, flags ) ){

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
//  CanalClose
// 

#ifdef WIN32
extern "C" int  WINAPI EXPORT CanalClose( long handle )
#else
extern "C" int CanalClose( long handle )
#endif
{
	int rv = 0;

	CCAN232Obj *pdrvObj =  theApp->getDriverObject( handle );
	if ( NULL == pdrvObj ) return 0;
	pdrvObj->close();
	theApp->removeDriverObject( handle );
	rv = 1;
	return CANAL_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
//  CanalGetLevel
// 

#ifdef WIN32
extern "C" unsigned long WINAPI EXPORT CanalGetLevel( long handle )
#else
extern "C" unsigned long CanalGetLevel( long handle )
#endif
{
	return CANAL_LEVEL_STANDARD;
}


///////////////////////////////////////////////////////////////////////////////
// CanalSend
//

#ifdef WIN32
extern "C" int WINAPI EXPORT CanalSend( long handle, PCANALMSG pCanalMsg  )
#else
extern "C" int CanalSend( long handle, PCANALMSG pCanalMsg  )
#endif
{
	CCAN232Obj *pdrvObj =  theApp->getDriverObject( handle );
	if ( NULL == pdrvObj ) return 0;
	return ( pdrvObj->writeMsg( pCanalMsg ) ? CANAL_ERROR_SUCCESS : CANAL_ERROR_GENERIC );
}

///////////////////////////////////////////////////////////////////////////////
// CanalReceive
//

#ifdef WIN32
extern "C" int WINAPI EXPORT CanalReceive( long handle, PCANALMSG pCanalMsg  )
#else
extern "C" int CanalReceive( long handle, PCANALMSG pCanalMsg  )
#endif
{
	CCAN232Obj *pdrvObj =  theApp->getDriverObject( handle );
	if ( NULL == pdrvObj ) return 0;
	return ( pdrvObj->readMsg( pCanalMsg )  ? CANAL_ERROR_SUCCESS : CANAL_ERROR_GENERIC );
}

///////////////////////////////////////////////////////////////////////////////
// CanalDataAvailable
//

#ifdef WIN32
extern "C" int WINAPI EXPORT CanalDataAvailable( long handle  )
#else
extern "C" int CanalDataAvailable( long handle  )
#endif
{
	CCAN232Obj *pdrvObj =  theApp->getDriverObject( handle );
	if ( NULL == pdrvObj ) return 0;
	return pdrvObj->dataAvailable();
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetStatus
//

#ifdef WIN32
extern "C" int WINAPI EXPORT CanalGetStatus( long handle, PCANALSTATUS pCanalStatus  )
#else
extern "C" int CanalGetStatus( long handle, PCANALSTATUS pCanalStatus  )
#endif
{
	CCAN232Obj *pdrvObj =  theApp->getDriverObject( handle );
	if ( NULL == pdrvObj ) return 0;
	return ( pdrvObj->getStatus( pCanalStatus )  ? CANAL_ERROR_SUCCESS : CANAL_ERROR_GENERIC );
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetStatistics
//

#ifdef WIN32
extern "C" int WINAPI EXPORT CanalGetStatistics( long handle, PCANALSTATISTICS pCanalStatistics  )
#else
extern "C" int CanalGetStatistics( long handle, PCANALSTATISTICS pCanalStatistics  )
#endif
{
	CCAN232Obj *pdrvObj =  theApp->getDriverObject( handle );
	if ( NULL == pdrvObj ) return 0;
	return ( pdrvObj->getStatistics( pCanalStatistics )  ? CANAL_ERROR_SUCCESS : CANAL_ERROR_GENERIC );
}

///////////////////////////////////////////////////////////////////////////////
// CanalSetFilter
//

#ifdef WIN32
extern "C" int WINAPI EXPORT CanalSetFilter( long handle, unsigned long filter )
#else
extern "C" int CanalSetFilter( long handle, unsigned long filter )
#endif
{
	CCAN232Obj *pdrvObj =  theApp->getDriverObject( handle );
	if ( NULL == pdrvObj ) return 0;
	return ( pdrvObj->setFilter( filter )  ? CANAL_ERROR_SUCCESS : CANAL_ERROR_GENERIC );
}

///////////////////////////////////////////////////////////////////////////////
// CanalSetMask
//

#ifdef WIN32
extern "C" int WINAPI EXPORT CanalSetMask( long handle, unsigned long mask )
#else
extern "C" int CanalSetMask( long handle, unsigned long mask )
#endif
{
	CCAN232Obj *pdrvObj =  theApp->getDriverObject( handle );
	if ( NULL == pdrvObj ) return 0;
	return ( pdrvObj->setMask( mask )  ? CANAL_ERROR_SUCCESS : CANAL_ERROR_GENERIC );
}

///////////////////////////////////////////////////////////////////////////////
// CanalSetBaudrate
//

#ifdef WIN32
extern "C" int WINAPI EXPORT CanalSetBaudrate( long handle, unsigned long baudrate )
#else
extern "C" int CanalSetBaudrate( long handle, unsigned long baudrate )
#endif
{	
	// Not supported in this DLL
	return CANAL_ERROR_NOT_SUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetVersion
//

#ifdef WIN32
extern "C" unsigned long WINAPI EXPORT CanalGetVersion( void )
#else
extern "C" unsigned long CanalGetVersion( void )
#endif
{
	unsigned long version;
	unsigned char *p = (unsigned char *)&version;

	*p = CANAL_MAIN_VERSION;
	*(p+1) = CANAL_MINOR_VERSION;
	*(p+2) = CANAL_SUB_VERSION;
	*(p+3) = 0;
	return version;
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetDllVersion
//

#ifdef WIN32
extern "C" unsigned long WINAPI EXPORT CanalGetDllVersion( void )
#else
extern "C" unsigned long CanalGetDllVersion( void )
#endif
{
	return DLL_VERSION;
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetVendorString
//

#ifdef WIN32
extern "C" const char * WINAPI EXPORT CanalGetVendorString( void )
#else
extern "C" const char * CanalGetVendorString( void )
#endif
{
	return CANAL_DLL_VENDOR;
}



