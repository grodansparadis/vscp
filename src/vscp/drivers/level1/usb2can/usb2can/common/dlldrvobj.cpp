// dlldrvobj.cpp : Defines the initialization routines for the DLL.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2008 Ake Hedman, D of Scandinavia, <akhe@eurosource.se>
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
// $RCSfile: dlldrvobj.cpp,v $                                       
// $Date: 2005/01/05 12:16:12 $                                  
// $Author: akhe $                                              
// $Revision: 1.2 $ 
//
// Linux
// =====
// device1 = logger,/tmp/canal_log,txt,/usr/local/lib/canallogger.so,64,64,1
//
// WIN32
// =====
// device1 = logger,c:\canal_log,txt,d:\winnr\system32\canallogger.dll,64,64,1

#include "stdio.h"
#include "stdlib.h"
#include "dlldrvobj.h"
#include "usb2canobj.h"

#ifdef WIN32

#else

void _init() __attribute__((constructor));
void _fini() __attribute__((destructor));

void _init() {printf("initializing\n");}
void _fini() {printf("finishing\n");}

#endif

/////////////////////////////////////////////////////////////////////////////
// CDllDrvObj

////////////////////////////////////////////////////////////////////////////
// CDllDrvObj construction
static long arrayObj[CANAL_USB2CAN_DRIVER_MAX_OPEN];


CDllDrvObj::CDllDrvObj() : lpvMem(arrayObj), hMapObject(NULL)
{
	m_instanceCounter = 0;
#ifdef WIN32
	m_objMutex = CreateMutex( NULL, false, "__CANAL_IXXATVCI_MUTEX__" );
	DWORD dwResult = LOCK_MUTEX(m_objMutex);
	if(dwResult == WAIT_ABANDONED)
	{
		UNLOCK_MUTEX(m_objMutex);
		CloseHandle(m_objMutex);		
		throw "error mutex abandoned";
	}

            // Create a named file mapping object
 
            hMapObject = CreateFileMapping( 
                INVALID_HANDLE_VALUE,   // use paging file
                NULL,                   // default security attributes
                PAGE_READWRITE,         // read/write access
                0,                      // size: high 32-bits
                sizeof(long) * CANAL_USB2CAN_DRIVER_MAX_OPEN,  // size: low 32-bits
                TEXT("dllusb2canfilemap")); // name of map object
            if (hMapObject == NULL) 
			{
				UNLOCK_MUTEX(m_objMutex);
				CloseHandle(m_objMutex);
                throw "error createfilemapping"; 
			}
 
            // The first process to attach initializes memory
 
            bool fInit = (GetLastError() != ERROR_ALREADY_EXISTS); 
 
            // Get a pointer to the file-mapped shared memory
 
            lpvMem = static_cast<long*>(MapViewOfFile( 
                hMapObject,     // object to map view of
                FILE_MAP_WRITE, // read/write access
                0,              // high offset:  map from
                0,              // low offset:   beginning
                0));             // default: map entire file
            if (lpvMem == NULL) 
			{
				UNLOCK_MUTEX(m_objMutex);
				CloseHandle(m_objMutex);
                throw "error createfilemapping"; 
			}
 
            // Initialize memory if this is the first process
 
            if (fInit) 
			{
				memset(lpvMem, 0, sizeof(long)*CANAL_USB2CAN_DRIVER_MAX_OPEN);
			}

#else
	pthread_mutex_init( &m_objMutex, NULL );
	//DL: I'm afraid I don't know how to open a memory mapped file under Linux so for lpvMem by default uses a static array address
#endif

	// Init the driver array
	for ( int i = 0; i<CANAL_USB2CAN_DRIVER_MAX_OPEN; i++ ) {
		m_drvObjArray[i] = NULL;
	}


	UNLOCK_MUTEX( m_objMutex );
}


CDllDrvObj::~CDllDrvObj()
{
	LOCK_MUTEX( m_objMutex );
	
	for ( int i = 0; i<CANAL_USB2CAN_DRIVER_MAX_OPEN; i++ )
	{
		if ( NULL != m_drvObjArray[i] )
		{ 
			m_drvObjArray[ i ]->close();	
			delete m_drvObjArray[ i ];
			m_drvObjArray[ i ] = NULL; 
			lpvMem[i] = 0;//inform global space that this driver closed
		}		
	}
#ifdef WIN32
	UnmapViewOfFile(lpvMem); 
 
    // Close the process's handle to the file-mapping object
    CloseHandle(hMapObject);

	UNLOCK_MUTEX( m_objMutex );


	CloseHandle( m_objMutex );
#else	
	pthread_mutex_destroy( &m_objMutex );
#endif
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CDllDrvObjgerdllApp object

//CDllDrvObj theApp;



///////////////////////////////////////////////////////////////////////////////
// addDriverObject
//

long CDllDrvObj::addDriverObject( CUsb2canObj *pdrvObj )
{
	long h = 0;

	LOCK_MUTEX( m_objMutex );

	 for ( int i=0; i<CANAL_USB2CAN_DRIVER_MAX_OPEN; i++ )
	 {	
		if ( NULL == m_drvObjArray[ i ] && lpvMem[i] == 0)
		{		
			m_drvObjArray[ i ] = pdrvObj;	
			h = i + 1681; 
			lpvMem[i] = h;//inform global space that this driver opened on position i
			break;
	    }
	}

	UNLOCK_MUTEX( m_objMutex );

	return h;
}


///////////////////////////////////////////////////////////////////////////////
// getDriverObject
//

CUsb2canObj * CDllDrvObj::getDriverObject( long h )
{
	long idx = h - 1681;

	// Check if valid handle
	if ( idx < 0 || idx >= CANAL_USB2CAN_DRIVER_MAX_OPEN) return NULL;
	return m_drvObjArray[ idx ];
}


///////////////////////////////////////////////////////////////////////////////
// removeDriverObject
//

void CDllDrvObj::removeDriverObject( long h )
{
	long idx = h - 1681;

	// Check if valid handle
	if ( idx < 0 || idx >= CANAL_USB2CAN_DRIVER_MAX_OPEN) return;

	LOCK_MUTEX( m_objMutex );

	if ( NULL != m_drvObjArray[ idx ] )
	{
	  delete m_drvObjArray[ idx ];
	}

	m_drvObjArray[ idx ] = NULL;
	lpvMem[idx] = 0;//inform global space driver on position idx closed

	UNLOCK_MUTEX( m_objMutex );
}

///////////////////////////////////////////////////////////////////////////////
// InitInstance

BOOL CDllDrvObj::InitInstance() 
{
	m_instanceCounter++;
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//    removeAllObjects
//

BOOL CDllDrvObj::removeAllObjects()
{
	LOCK_MUTEX( m_objMutex );

	for ( int i=0; i<CANAL_USB2CAN_DRIVER_MAX_OPEN; i++ )
	{
		if ( NULL != m_drvObjArray[ i ] )
		{ 
			m_drvObjArray[ i ]->close();	
			delete m_drvObjArray[ i ];
			m_drvObjArray[ i ] = NULL; 
			lpvMem[i] = 0;//inform global space driver on position i closed
		}
	}

	UNLOCK_MUTEX( m_objMutex );

return TRUE;
}
 
