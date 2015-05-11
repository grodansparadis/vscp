// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2015 Ake Hedman, Grodans Paradis AB, akhe@grodansparadis.com
// Copyright (C) 2005-2012 Gediminas Simanskis,8devices,<gediminas@8devices.com>
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


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>

#include "../common/dlldrvobj.h"
#include "../common/usb2canobj.h"

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
			try{
				hThisInstDll = hInstDll;
				theApp = new CDllDrvObj();
				theApp->InitInstance();
			}
			catch(...)
			{
				return FALSE;//memory exception creating handles
			}
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
//  CanalOpen
//

#ifdef WIN32
extern "C" long WINAPI EXPORT CanalOpen( const char *pDevice, unsigned long flags )
#else
extern "C" long CanalOpen( const char *pDevice, unsigned long flags )
#endif
{
	long h = 0;
	CUsb2canObj *pdrvObj = NULL;

	pdrvObj = new CUsb2canObj();

	if ( NULL != pdrvObj )
	{

			if ( ( h = theApp->addDriverObject(pdrvObj)) <= 0 )
			{
				h = 0;
				delete  pdrvObj; 
			}
			else
			{
				if ( pdrvObj->open( pDevice, flags ) == FALSE )
				{
				   theApp->removeDriverObject( h );
				   h = 0;
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
	CUsb2canObj *pdrvObj =  theApp->getDriverObject( handle );

	if ( NULL == pdrvObj )
	  return 0;

	pdrvObj->close();
	theApp->removeDriverObject( handle );
	
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
  CUsb2canObj *pdrvObj =  theApp->getDriverObject( handle );
  if ( NULL == pdrvObj ) return 0;
  return ( pdrvObj->writeMsg( pCanalMsg ));
}


///////////////////////////////////////////////////////////////////////////////
// CanalSend blocking
//

#ifdef WIN32
extern "C" int WINAPI EXPORT CanalBlockingSend( long handle, PCANALMSG pCanalMsg, unsigned long timeout )
#else
extern "C" int CanalBlockingSend( long handle, PCANALMSG pCanalMsg  )
#endif
{
  CUsb2canObj *pdrvObj =  theApp->getDriverObject( handle );
  if ( NULL == pdrvObj ) return 0;
  return ( pdrvObj->writeMsgBlocking( pCanalMsg, timeout ));
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
	CUsb2canObj *pdrvObj =  theApp->getDriverObject( handle );
	if ( NULL == pdrvObj ) return 0;
	return ( pdrvObj->readMsg( pCanalMsg ));
}

///////////////////////////////////////////////////////////////////////////////
// CanalReceive blocking
//

#ifdef WIN32
extern "C" int WINAPI EXPORT CanalBlockingReceive( long handle, PCANALMSG pCanalMsg, unsigned long timeout )
#else
extern "C" int CanalBlockingReceive( long handle, PCANALMSG pCanalMsg, unsigned long timeout )
#endif
{
	CUsb2canObj *pdrvObj =  theApp->getDriverObject( handle );
	if ( NULL == pdrvObj ) return 0;
	return ( pdrvObj->readMsgBlocking( pCanalMsg, timeout ));
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
	CUsb2canObj *pdrvObj =  theApp->getDriverObject( handle );
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
	CUsb2canObj *pdrvObj =  theApp->getDriverObject( handle );

	if ( NULL == pdrvObj )
		return 0;

	return ( pdrvObj->getStatus( pCanalStatus ) );
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
	CUsb2canObj *pdrvObj =  theApp->getDriverObject( handle );

	if ( NULL == pdrvObj )
		return 0;

	return ( pdrvObj->getStatistics( pCanalStatistics ) );
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
	CUsb2canObj *pdrvObj =  theApp->getDriverObject( handle );
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
	CUsb2canObj *pdrvObj =  theApp->getDriverObject( handle );
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

	unsigned long version;
	unsigned char *p = (unsigned char *)&version;

	*p = DLL_MAIN_VERSION;
	*(p+1) = DLL_MINOR_VERSION;
	*(p+2) = DLL_SUB_VERSION;
	*(p+3) = 0;
	return version;

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
  static char r_str[256];
  char tmp_str[20];

	CUsb2canObj *pdrvObj =  theApp->getDriverObject( 1681 );

	if ( NULL == pdrvObj )
		return NULL;

	 // return ( pdrvObj->getVendorString());

		//	   strcat( str, ";");
        //   strcat( str, "www.edevices.lt\0");

//#define CANAL_MAIN_VERSION					1
//#define CANAL_MINOR_VERSION					0
//#define CANAL_SUB_VERSION	

	strcpy_s( r_str, 256, pdrvObj->getVendorString());
    strcat_s( r_str, 256, ";");
    sprintf_s(tmp_str,sizeof(tmp_str),"%d.%d.%d",CANAL_MAIN_VERSION,CANAL_MINOR_VERSION,CANAL_SUB_VERSION);
	strcat_s( r_str, 256, tmp_str );
	strcat_s( r_str, 256, ";");
    sprintf_s(tmp_str,sizeof(tmp_str),"%d.%d.%d",DLL_MAIN_VERSION,DLL_MINOR_VERSION,DLL_SUB_VERSION);
	strcat_s( r_str, 256, tmp_str );
	strcat_s( r_str, 256, ";");
	strcat_s( r_str, 256, "8devices.com\0");

    return (r_str);
}


///////////////////////////////////////////////////////////////////////////////
// CanalGetReceiveHandle
//

#ifdef WIN32
extern "C" HANDLE WINAPI EXPORT CanalGetReceiveHandle( long handle )
#else
extern "C" HANDLE CanalGetReceiveHandle( long handle )
#endif
{
	CUsb2canObj *pdrvObj =  theApp->getDriverObject( handle );
	if ( NULL == pdrvObj ) return 0;
	return pdrvObj->getReceiveHandle();
}
