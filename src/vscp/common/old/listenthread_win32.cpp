// listenthread_win32.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2008 Ake Hedman, D of Scandinavia, <akhe@eurosource.se>
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
// $RCSfile: listenthread_win32.cpp,v $                                       
// $Date: 2005/09/08 17:18:26 $                                  
// $Author: akhe $                                              
// $Revision: 1.8 $ 

#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"
#include "wx/thread.h"

#ifndef DWORD
#define DWORD unsigned long
#endif

#include "canal_win32_ipc.h"
#include "canal_macro.h"
#include "canalshmem_level1_win32.h"
#include "canalshmem_level2_win32.h"
#include "controlobject.h"

// Prototypes
void clientThreadLevel1( void *p );
void clientThreadLevel2( void *p );

///////////////////////////////////////////////////////////////////////////////
// listenThread
//
// This thread listens for conection on a pipe and starts a new thread
// to handle client requests
//

void listenThread( void *p )
{  
	int rv = 0;
	unsigned long clid = VSCP_CLIENT_ID_BASE;		// Client ID
	HANDLE hMap;									// Listenthread shared memory
	HANDLE hListenMutex;		// Listenthread SHM access mutex
	HANDLE hSemListenCmd;		// Listenthread Command semaphore
	HANDLE hSemListenDone;		// Listenthread Command done semaphore
   	

	// http://msdn.microsoft.com/msdnmag/issues/0300/security/default.aspx
	// here we use a SECURITY_DESCRIPTOR to say
	// that we don't want *any DACL at all*
	SECURITY_DESCRIPTOR sd;
	InitializeSecurityDescriptor( &sd,
									SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&sd, TRUE, 0, FALSE);

	SECURITY_ATTRIBUTES sa = { sizeof sa, &sd, TRUE };

	CControlObject *pctrlObject = (CControlObject *)p;

	// Create ListenThread shared memory
	if ( NULL == ( hMap = ::CreateFileMapping( (HANDLE )0xFFFFFFFF,
															&sa,
															PAGE_READWRITE,
															0,
															sizeof( __shmOpen ),
															wxT( CANAL_LISTEN_SHM_NAME ) ) ) ) {
		::wxLogDebug("Listen Thread: Failed to create shared memory.");
		ExitThread( -1 );
	}

	struct __shmOpen *pstruct;
	if ( NULL == ( pstruct = 
		(struct __shmOpen *)MapViewOfFile( hMap, FILE_MAP_WRITE, 0, 0, 0 ) ) ) {
	
		::wxLogDebug("Listen Thread: Failed to map shared memory.");
		CloseHandle( hMap );
		ExitThread( -1 );
	}
	
	// Create mutex for ListenThread shared memory access
	if ( NULL == ( hListenMutex = 
						CreateMutex( &sa, true, CANAL_LISTEN_SHARED_MEMORY_MUTEX ) ) ) {
		
		::wxLogDebug("Listen Thread: Failed to create listen mutex.");
		CloseHandle( hMap );
		ExitThread( -1 );	
	}


	// Create semaphore for ListenThread valid command
	if ( NULL == ( hSemListenCmd = CreateSemaphore( &sa, 0, 1, CANAL_LISTEN_COMMAND_SEM ) ) ) {
		
		::wxLogDebug("Listen Thread: Failed to create command semaphore.");
		CloseHandle( hMap );
		CloseHandle( hListenMutex );
		ExitThread( -1 );	

	}

	// Create semaphore for ListenThread command done
	if ( NULL == ( hSemListenDone = CreateSemaphore( &sa, 0, 1, CANAL_LISTEN_DONE_SEM ) ) ) {
		
		::wxLogDebug("Listen Thread: Failed to create done semaphore.");
		CloseHandle( hMap );
		CloseHandle( hListenMutex );
		CloseHandle( hSemListenCmd );
		CloseHandle( hSemListenDone );
		ExitThread( -1 );	

	}
	
	// Open the gate....
	ReleaseMutex( hListenMutex );
	
	while( !pctrlObject->m_bQuit ) {
	
		pstruct->m_command = 0;
		pstruct->m_flags = 0;
		pstruct->m_cmdResult = 0;
		*pstruct->m_strIfName = 0;
						
		// Wait for a valid command to be handled
		WaitForSingleObject( hSemListenCmd, INFINITE );

		// Client ID used for client thread objects
		pstruct->m_shmid = clid;	
		
		// Create a shared memory area for this client
		char buf[ MAX_PATH ];
		sprintf( buf, CANAL_LISTEN_CLIENT_SHM_TEMPLATE, clid );
		if ( NULL != ( hMap = ::CreateFileMapping( (HANDLE )0xFFFFFFFF, 
															&sa,
															PAGE_READWRITE,
															0,
															sizeof( __shmOpen ),
															buf ) ) ) {
					
			_clientstruct *pclientstruct = new _clientstruct;			
			pclientstruct->m_shmid = clid;					// id for client shared memory
			pclientstruct->m_pctrlObject = pctrlObject;		// Control object

			// Prepare for Next client
			clid++;

			if ( CANAL_COMMAND_OPEN_VSCP_LEVEL1 == pstruct->m_command ) {
				
				// Launch the client handler thread
				DWORD dwThreadId;
				if ( NULL == CreateThread(	NULL,
											0,
											(LPTHREAD_START_ROUTINE) clientThreadLevel1,
											pclientstruct,
											0,
											&dwThreadId ) ) { 
					delete pclientstruct;
					pstruct->m_cmdResult = -2;	// ERROR: No client thread
					SYSLOG( LOG_CRIT, "Listenthread: Unable to create client thread Level1. Client turned down.");
					
				} 
				
				pstruct->m_cmdResult = 0;	// All is OK

			}
			else if ( CANAL_COMMAND_OPEN_VSCP_LEVEL2 == pstruct->m_command ) {
		
				// Launch the client handler thread
				DWORD dwThreadId;
				if ( NULL == CreateThread(	NULL,
											0,
											(LPTHREAD_START_ROUTINE) clientThreadLevel2,
											pclientstruct,
											0,
											&dwThreadId ) ) { 
					delete pclientstruct;
					pstruct->m_cmdResult = -2;	// ERROR: No client thread
					SYSLOG( LOG_CRIT, "Listenthread: Unable to create client thread Level2. Client turned down.");
					
			}	

				pstruct->m_cmdResult = 0;	// All is OK
			
			}
			else if ( CANAL_COMMAND_OPEN_VSCP_CONTROL == pstruct->m_command ) {
			
				;
			
			}
			else {
		
				pstruct->m_cmdResult = -1;	// ERROR: no shared memory
				SYSLOG( LOG_INFO, "Listenthread: Unknown command request.");

			}
		
		}		
		
		// Signal the client we are done
		ReleaseSemaphore( hSemListenDone, 1, NULL );

	} // while loop
			
	// Release shared memory
	CloseHandle( hMap ); 
    hMap = NULL; 
 
	// Release Mutex
	CloseHandle( hListenMutex );

	// Relase semaphores
	CloseHandle( hSemListenCmd );
	CloseHandle( hSemListenDone );

	ExitThread( 0 );
	
}
