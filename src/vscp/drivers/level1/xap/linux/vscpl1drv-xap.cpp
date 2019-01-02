// xapdrv.cpp : Defines the initialization routines for the DLL.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2019 Ake Hedman, eurosource, <akhe@eurosource.se>
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
// Linux
// =====
// device1 = xapdrv,/tmp/canal_log,txt,/usr/local/lib/xapdrv.so,64,64,1
//
// WIN32
// =====
// device1 = logger,c:\canal_log,txt,d:\winnr\system32\canallogger.dll,64,64,1

#include "vscpl1drv-xap.h"
#include "stdio.h"
#include "stdlib.h"


void _init() __attribute__((constructor));
void _fini() __attribute__((destructor));

void _init() {printf("initializing\n");}
void _fini() {printf("finishing\n");}


/////////////////////////////////////////////////////////////////////////////
// CXapdllApp

////////////////////////////////////////////////////////////////////////////
// CXapdllApp construction

CXapdllApp::CXapdllApp()
{
	m_instanceCounter = 0;
	pthread_mutex_init( &m_objMutex, NULL );

	// Init the driver array
	for ( int i = 0; i<CANAL_XAP_DRIVER_MAX_OPEN; i++ ) {
		m_xapArray[ i ] = NULL;
	}

	UNLOCK_MUTEX( m_objMutex );
}


CXapdllApp::~CXapdllApp()
{
	LOCK_MUTEX( m_objMutex );
	
	for ( int i = 0; i<CANAL_XAP_DRIVER_MAX_OPEN; i++ ) {
		
		if ( NULL == m_xapArray[ i ] ) {
			
			CXAPObj *pXAPObj =  getDriverObject( i );
			if ( NULL != pXAPObj ) { 
				pXAPObj->close();	
				delete m_xapArray[ i ];
				m_xapArray[ i ] = NULL; 
			}
		}
	}

	UNLOCK_MUTEX( m_objMutex );	
	pthread_mutex_destroy( &m_objMutex );
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CLoggerdllApp object

CXapdllApp theApp;




///////////////////////////////////////////////////////////////////////////////
// CreateObject

//extern "C" CXapdllApp* CreateObject( void ) {
//	CXapdllApp *theapp = new CXapdllApp;
//	return ( ( CXapdllApp * ) theapp );
//}

///////////////////////////////////////////////////////////////////////////////
// addDriverObject
//

long CXapdllApp::addDriverObject( CXAPObj *pXAPObj )
{
	long h = 0;

	LOCK_MUTEX( m_objMutex );
	for ( int i=0; i<CANAL_XAP_DRIVER_MAX_OPEN; i++ ) {
	
		if ( NULL == m_xapArray[ i ] ) {
		
			m_xapArray[ i ] = pXAPObj;	
			h = i + 1681; 
			break;

		}

	}

	UNLOCK_MUTEX( m_objMutex );

	return h;
}


///////////////////////////////////////////////////////////////////////////////
// getDriverObject
//

CXAPObj * CXapdllApp::getDriverObject( long h )
{
	long idx = h - 1681;

	// Check if valid handle
	if ( idx < 0 ) return NULL;
	if ( idx >= CANAL_XAP_DRIVER_MAX_OPEN ) return NULL;
	return m_xapArray[ idx ];
}


///////////////////////////////////////////////////////////////////////////////
// removeDriverObject
//

void CXapdllApp::removeDriverObject( long h )
{
	long idx = h - 1681;

	// Check if valid handle
	if ( idx < 0 ) return;
	if ( idx >= CANAL_XAP_DRIVER_MAX_OPEN ) return;

	LOCK_MUTEX( m_objMutex );
	if ( NULL != m_xapArray[ idx ] ) delete m_xapArray[ idx ];
	m_xapArray[ idx ] = NULL;
	UNLOCK_MUTEX( m_objMutex );
}

///////////////////////////////////////////////////////////////////////////////
// InitInstance

BOOL CXapdllApp::InitInstance() 
{
	m_instanceCounter++;
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//                             C A N A L -  A P I
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CanalOpen
//

extern "C" long CanalOpen( const char *pDevice, unsigned long flags )
{
	long h = 0;
	char *pFilter = NULL;
	char *pMask = NULL;
	
	if ( NULL != ( pFilter = strchr( (char *)pDevice, ';' ) ) ) {
		
		*pFilter = 0;
		pFilter++;
		
		if ( NULL != ( pMask = strchr( (char *)pDevice, ';' ) ) ) {
			*pMask = 0;	
			pMask++;
		}
	}
	
	CXAPObj *pXAPObj = new CXAPObj();
	if ( NULL != pXAPObj ) {

		if ( pXAPObj->open( pDevice, flags ) ){

			if ( !( h = theApp.addDriverObject( pXAPObj ) ) ) {
				delete pXAPObj;
			}
			else {
				
				if ( NULL != pFilter ) {
					pXAPObj->setFilter( atol( pFilter ) );	
				}

				if ( NULL != pMask ) {
					pXAPObj->setMask( atol( pMask ) );
				}
			}

		}
		else {
			delete pXAPObj;
		}

	}
 
	return h;
}

///////////////////////////////////////////////////////////////////////////////
//  CanalClose
// 

extern "C" int CanalClose( long handle )
{
	int rv = 0;

	CXAPObj *pXapObj =  theApp.getDriverObject( handle );
	if ( NULL == pXapObj ) return 0;
	pXapObj->close();
	theApp.removeDriverObject( handle );
	rv = 1;
	return rv;
}


///////////////////////////////////////////////////////////////////////////////
//  CanalGetLevel
// 

extern "C" unsigned long CanalGetLevel( long handle )
{
	return ( CANAL_LEVEL_STANDARD );
}


///////////////////////////////////////////////////////////////////////////////
// CanalSend
//

extern "C" int CanalSend( long handle, PCANALMSG pCanalMsg  )
{
	CXAPObj *pXapObj =  theApp.getDriverObject( handle );
	if ( NULL == pXapObj ) return 0;
	return pXapObj->writeMsg( pCanalMsg );
}

///////////////////////////////////////////////////////////////////////////////
// CanalReceive
//

extern "C" int CanalReceive( long handle, PCANALMSG pCanalMsg  )
{
	int rv = 0;
	
	// Nothing to receive from this DLL
	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// CanalDataAvailable
//

extern "C" int CanalDataAvailable( long handle  )
{
	int rv = 0;

	// No data available from this DLL
	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetStatus
//

extern "C" int CanalGetStatus( long handle, PCANALSTATUS pCanalStatus  )
{
	int rv = 0;

	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetStatistics
//

extern "C" int CanalGetStatistics( long handle, PCANALSTATISTICS pCanalStatistics  )
{
	int rv = 0;

	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// CanalSetFilter
//

extern "C" int CanalSetFilter( long handle, unsigned long filter )
{
	int rv = 0;
	CXAPObj *pXapObj =  theApp.getDriverObject( handle );
	if ( NULL == pXapObj ) return 0;
	pXapObj->setFilter( filter );
	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// CanalSetMask
//

extern "C" int CanalSetMask( long handle, unsigned long mask )
{
	int rv = 0;
	CXAPObj *pXapObj =  theApp.getDriverObject( handle );
	if ( NULL == pXapObj ) return 0;
	pXapObj->setMask( mask );
	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// CanalSetBaudrate
//

extern "C" int CanalSetBaudrate( long handle, unsigned long baudrate )
{
	int rv = 0;
	
	// Not supported in this DLL
	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetVersion
//

extern "C" unsigned long CanalGetVersion( void )
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

extern "C" unsigned long CanalGetDllVersion( void )
{
	return DLL_VERSION;
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetVendorString
//

extern "C" const char * CanalGetVendorString( void )
{
	return CANAL_DLL_VENDOR;
}


 
