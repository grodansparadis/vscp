// dlldrvobj.cpp : Defines the initialization routines for the DLL.
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

#include "stdio.h"
#include "stdlib.h"
#include "dlldrvobj.h"



////////////////////////////////////////////////////////////////////////////
// CHelpDllObj construction

CHelpDllObj::CHelpDllObj()
{
	m_instanceCounter = 0;

	// Init the driver array
	for ( int i = 0; i<VSCP_HELPER_MAX_OPEN; i++ ) {
		m_drvObjArray[ i ] = NULL;
	}
}


CHelpDllObj::~CHelpDllObj()
{
    m_mutex.Lock();
	
	for ( int i = 0; i<VSCP_HELPER_MAX_OPEN; i++ ) {
		
		if ( NULL != m_drvObjArray[ i ] ) {
			
			VscpRemoteTcpIf *pdrvObj =  getDriverObject( i );
			if ( NULL != pdrvObj ) { 
				pdrvObj->doCmdClose();	
				delete m_drvObjArray[ i ];
				m_drvObjArray[ i ] = NULL; 
			}
		}
	}

    m_mutex.Unlock();

}



///////////////////////////////////////////////////////////////////////////////
// addDriverObject
//

long CHelpDllObj::addDriverObject( VscpRemoteTcpIf *pdrvObj )
{
	long h = 0;

    m_mutex.Lock();
	for ( int i=0; i<VSCP_HELPER_MAX_OPEN; i++ ) {
	
		if ( NULL == m_drvObjArray[ i ] ) {
		
			m_drvObjArray[ i ] = pdrvObj;	
			h = i + 1681; 
			break;

		}

	}

    m_mutex.Unlock();

	return h;
}


///////////////////////////////////////////////////////////////////////////////
// getDriverObject
//

VscpRemoteTcpIf * CHelpDllObj::getDriverObject( long h )
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

	//LOCK_MUTEX( m_objMutex );
    m_mutex.Lock();
	if ( NULL != m_drvObjArray[ idx ] ) 
    {
        delete m_drvObjArray[ idx ];
    }
	m_drvObjArray[ idx ] = NULL;
	//UNLOCK_MUTEX( m_objMutex );
    m_mutex.Unlock();
}

///////////////////////////////////////////////////////////////////////////////
// InitInstance

BOOL CHelpDllObj::InitInstance() 
{
    // http://osdir.com/ml/lib.wxwindows.general/2004-01/msg00759.html
    // http://sudarsun.in/blog/2009/02/error-can-not-start-thread-error-writing-tls-error-87-the-parameter-is-incorrect/
    //wxInitialize();
	m_instanceCounter++;
	return TRUE;
}


 
