// vscpl1.cpp : Defines the initialization routines for the DLL.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2007 Johan Hedlund,  <kungjohan@gmail.com>
// 
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
//
// Linux
// =====
// device1 = logger,/tmp/canal_log,txt,/usr/local/lib/canallogger.so,64,64,1
//
// WIN32
// =====
// device1 = logger,c:\canal_log,txt,d:\winnr\system32\canallogger.dll,64,64,1

#ifdef __GNUG__
//#pragma implementation
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef __WXMSW__
#include  "wx/ownerdrw.h"
#endif

#include "wx/tokenzr.h"

#include "vscpl1.h"
#include "stdio.h"
#include "stdlib.h"


void _init() __attribute__((constructor));
void _fini() __attribute__((destructor));

void _init() {printf("initializing\n");}
void _fini() {printf("finishing\n");}



////////////////////////////////////////////////////////////////////////////
// CVSCPL1App construction

CVSCPL1App::CVSCPL1App()
{
	m_instanceCounter = 0;
	pthread_mutex_init( &m_objMutex, NULL );

	// Init the driver array
	for ( int i = 0; i<VSCP_LEVEL1_INTERFACE_MAX_OPEN; i++ ) {
		m_pvscpifArray[ i ] = NULL;
	}

	UNLOCK_MUTEX( m_objMutex );
}


CVSCPL1App::~CVSCPL1App()
{
	LOCK_MUTEX( m_objMutex );
	
	for ( int i = 0; i<VSCP_LEVEL1_INTERFACE_MAX_OPEN; i++ ) {
		
		if ( NULL == m_pvscpifArray[ i ] ) {
			
			VscpRemoteTcpIf *pvscpif =  getDriverObject( i );
			if ( NULL != pvscpif ) { 
				pvscpif->doCmdClose();	
				delete m_pvscpifArray[ i ];
				m_pvscpifArray[ i ] = NULL; 
			}
		}
	}

	UNLOCK_MUTEX( m_objMutex );	
	pthread_mutex_destroy( &m_objMutex );
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CLoggerdllApp object

CVSCPL1App theApp;




///////////////////////////////////////////////////////////////////////////////
// CreateObject


///////////////////////////////////////////////////////////////////////////////
// addDriverObject
//

long CVSCPL1App::addDriverObject( VscpRemoteTcpIf *pvscpif )
{
	long h = 0;

	LOCK_MUTEX( m_objMutex );
	for ( int i=0; i<VSCP_LEVEL1_INTERFACE_MAX_OPEN; i++ ) {
	
		if ( NULL == m_pvscpifArray[ i ] ) {
		
			m_pvscpifArray[ i ] = pvscpif;	
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

VscpRemoteTcpIf *CVSCPL1App::getDriverObject( long h )
{
	long idx = h - 1681;

	// Check if valid handle
	if ( idx < 0 ) return NULL;
	if ( idx >= VSCP_LEVEL1_INTERFACE_MAX_OPEN ) return NULL;
	return m_pvscpifArray[ idx ];
}


///////////////////////////////////////////////////////////////////////////////
// removeDriverObject
//

void CVSCPL1App::removeDriverObject( long h )
{
	long idx = h - 1681;

	// Check if valid handle
	if ( idx < 0 ) return;
	if ( idx >= VSCP_LEVEL1_INTERFACE_MAX_OPEN  ) return;

	LOCK_MUTEX( m_objMutex );
	if ( NULL != m_pvscpifArray[ idx ] ) delete m_pvscpifArray[ idx ];
	m_pvscpifArray[ idx ] = NULL;
	UNLOCK_MUTEX( m_objMutex );
}

///////////////////////////////////////////////////////////////////////////////
// InitInstance

BOOL CVSCPL1App::InitInstance() 
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
	long h = CANAL_ERROR_SUB_DRIVER;
	unsigned long filter=0, mask=0;
	bool bFilter=false, bMask=false;
	wxString str;
	wxString strDevice( pDevice, wxConvUTF8);
	wxStringTokenizer tkz(strDevice, _(";") );

	// Get possible filter	
    str = tkz.GetNextToken();
	if ( 0 != str.Length() ) {
        if ( str.ToULong( &filter ) ) {
			bFilter = true;
		}
	}

	// Get possible mask
	if ( 0 != str.Length() ) {
		if ( str.ToULong( &mask ) ) {
			bMask = true;
		}
	}
	
	
	VscpRemoteTcpIf *pvscpif = new VscpRemoteTcpIf();
	if ( NULL != pvscpif ) {

		if ( pvscpif->doCmdOpen( strDevice.ToStdString(), flags ) ){

			if ( !( h = theApp.addDriverObject( pvscpif ) ) ) {
				delete pvscpif;
			}
			else {
				
				if ( bFilter ) {
					pvscpif->doCmdFilter( filter );	
				}

				if ( bMask ) {
					pvscpif->doCmdMask( mask );
				}
			}

		}
		else {
			delete pvscpif;
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

	VscpRemoteTcpIf *pvscpif =  theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return CANAL_ERROR_MEMORY;
	pvscpif->doCmdClose();
	theApp.removeDriverObject( handle );
	rv = CANAL_ERROR_SUCCESS;
	return rv;
}


///////////////////////////////////////////////////////////////////////////////
//  CanalGetLevel
// 

extern "C" unsigned long CanalGetLevel( long handle )
{
	unsigned long level;

	VscpRemoteTcpIf *pvscpif =  theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return CANAL_ERROR_MEMORY;

	level = pvscpif->doCmdGetLevel();
	return level;
}


///////////////////////////////////////////////////////////////////////////////
// CanalSend
//

extern "C" int CanalSend( long handle, PCANALMSG pCanalMsg  )
{
	VscpRemoteTcpIf *pvscpif =  theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return CANAL_ERROR_MEMORY;

	return ( pvscpif->doCmdSendLevel1( pCanalMsg )? CANAL_ERROR_SUCCESS : CANAL_ERROR_SUB_DRIVER );
}

///////////////////////////////////////////////////////////////////////////////
// CanalReceive
//

extern "C" int CanalReceive( long handle, PCANALMSG pCanalMsg  )
{
	int rv = 0;

	VscpRemoteTcpIf *pvscpif =  theApp.getDriverObject( handle );
        if ( NULL == pvscpif ) return CANAL_ERROR_MEMORY;
	
	return ( pvscpif->doCmdReceiveLevel1( pCanalMsg ) ? CANAL_ERROR_SUCCESS : CANAL_ERROR_SUB_DRIVER );

}

///////////////////////////////////////////////////////////////////////////////
// CanalDataAvailable
//

extern "C" int CanalDataAvailable( long handle  )
{
	int rv = 0;

	VscpRemoteTcpIf *pvscpif =  theApp.getDriverObject( handle );
        if ( NULL == pvscpif ) return CANAL_ERROR_MEMORY;

	return ( pvscpif->doCmdDataAvailable() ? CANAL_ERROR_SUCCESS : CANAL_ERROR_SUB_DRIVER );
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetStatus
//

extern "C" int CanalGetStatus( long handle, PCANALSTATUS pCanalStatus  )
{
	int rv = 0;

	VscpRemoteTcpIf *pvscpif =  theApp.getDriverObject( handle );
        if ( NULL == pvscpif ) return CANAL_ERROR_MEMORY;

	return ( pvscpif->doCmdStatus( pCanalStatus) ? CANAL_ERROR_SUCCESS : CANAL_ERROR_SUB_DRIVER );
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetStatistics
//

extern "C" int CanalGetStatistics( long handle, PCANALSTATISTICS pCanalStatistics  )
{
	VscpRemoteTcpIf *pvscpif =  theApp.getDriverObject( handle );
        if ( NULL == pvscpif ) return CANAL_ERROR_MEMORY;

	return ( pvscpif->doCmdStatistics( pCanalStatistics ) ? CANAL_ERROR_SUCCESS : CANAL_ERROR_SUB_DRIVER );
}

///////////////////////////////////////////////////////////////////////////////
// CanalSetFilter
//

extern "C" int CanalSetFilter( long handle, unsigned long filter )
{
	VscpRemoteTcpIf *pvscpif =  theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return CANAL_ERROR_MEMORY;
	return ( pvscpif->doCmdFilter( filter ) ? CANAL_ERROR_SUCCESS : CANAL_ERROR_SUB_DRIVER );
}

///////////////////////////////////////////////////////////////////////////////
// CanalSetMask
//

extern "C" int CanalSetMask( long handle, unsigned long mask )
{
	VscpRemoteTcpIf *pvscpif =  theApp.getDriverObject( handle );
	if ( NULL == pvscpif ) return CANAL_ERROR_MEMORY;
	return ( pvscpif->doCmdMask( mask ) ? CANAL_ERROR_SUCCESS : CANAL_ERROR_SUB_DRIVER );
}

///////////////////////////////////////////////////////////////////////////////
// CanalSetBaudrate
//

extern "C" int CanalSetBaudrate( long handle, unsigned long baudrate )
{
	VscpRemoteTcpIf *pvscpif =  theApp.getDriverObject( handle );
        if ( NULL == pvscpif ) return CANAL_ERROR_MEMORY;

	return ( pvscpif->doCmdSetBaudrate( baudrate ) ? CANAL_ERROR_SUCCESS : CANAL_ERROR_SUB_DRIVER );
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
	return VSCP_DLL_VERSION;
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetVendorString
//

extern "C" const char * CanalGetVendorString( void )
{
	return VSCP_DLL_VENDOR;
}


 
