// dlldrvobj.cpp : Defines the initialization routines for the DLL.
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

#include "stdio.h"
#include "stdlib.h"
#include "dlldrvobj.h"



////////////////////////////////////////////////////////////////////////////
// CHelpDllObj construction

CHelpDllObj::CHelpDllObj()
{
	m_instanceCounter = 0;
	m_objMutex = CreateMutex( NULL, true, _("__HELPER_DLL_MUTEX__") );

	// Init the driver array
	for ( int i = 0; i<VSCP_HELPER_MAX_OPEN; i++ ) {
		m_drvObjArray[ i ] = NULL;
	}

	UNLOCK_MUTEX( m_objMutex );
}


CHelpDllObj::~CHelpDllObj()
{
	LOCK_MUTEX( m_objMutex );
	
	for ( int i = 0; i<VSCP_HELPER_MAX_OPEN; i++ ) {
		
		if ( NULL == m_drvObjArray[ i ] ) {
			
			CCanalSuperWrapper *pdrvObj =  getDriverObject( i );
			if ( NULL != pdrvObj ) { 
				pdrvObj->doCmdClose();	
				delete m_drvObjArray[ i ];
				m_drvObjArray[ i ] = NULL; 
			}
		}
	}

	UNLOCK_MUTEX( m_objMutex );

	CloseHandle( m_objMutex );
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CDllDrvObjgerdllApp object

CHelpDllObj theApp;



///////////////////////////////////////////////////////////////////////////////
// addDriverObject
//

long CHelpDllObj::addDriverObject( CCanalSuperWrapper *pdrvObj )
{
	long h = 0;

	LOCK_MUTEX( m_objMutex );
	for ( int i=0; i<VSCP_HELPER_MAX_OPEN; i++ ) {
	
		if ( NULL == m_drvObjArray[ i ] ) {
		
			m_drvObjArray[ i ] = pdrvObj;	
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

CCanalSuperWrapper * CHelpDllObj::getDriverObject( long h )
{
	long idx = h - 1681;

	// Check if valid handle
	if ( idx < 0 ) return NULL;
	if ( idx >= VSCP_HELPER_MAX_OPEN ) return NULL;
	return m_drvObjArray[ idx ];
}


///////////////////////////////////////////////////////////////////////////////
// removeDriverObject
//

void CHelpDllObj::removeDriverObject( long h )
{
	long idx = h - 1681;

	// Check if valid handle
	if ( idx < 0 ) return;
	if ( idx >= VSCP_HELPER_MAX_OPEN ) return;

	LOCK_MUTEX( m_objMutex );
	if ( NULL != m_drvObjArray[ idx ] ) delete m_drvObjArray[ idx ];
	m_drvObjArray[ idx ] = NULL;
	UNLOCK_MUTEX( m_objMutex );
}

///////////////////////////////////////////////////////////////////////////////
// InitInstance

BOOL CHelpDllObj::InitInstance() 
{
	m_instanceCounter++;
	return TRUE;
}


 
