// listenthread.cpp
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
// $RCSfile: listenthread.cpp,v $                                       
// $Date: 2005/01/05 12:16:10 $                                  
// $Author: akhe $                                              
// $Revision: 1.6 $ 

#ifdef WIN32

#include "stdafx.h"
#include "canalpipe.h"

#else

#define _POSIX

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <semaphore.h>
#include <syslog.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#include "wx/defs.h"
#include "wx/app.h"
#include "wx/thread.h"

#ifndef DWORD
#define DWORD unsigned long
#endif

#include "canalshmem_unix.h"

#endif // WIN32

#include "../daemon/canald.h"
#include "controlobject.h"

// Prototypes
#ifdef WIN32
void clientThread( void *p );
#else
void *clientThread( void *p );
#endif


///////////////////////////////////////////////////////////////////////////////
// listenThread
//
// This thread listens for conection on a pipe and starts a new thread
// to handle client requests
//

void listenThread( void *p )
{
#ifdef WIN32
  HANDLE hPipe;
  HANDLE hThread;
  DWORD errorCode = 0;
  DWORD dwThreadId;
  
#else
  
  pthread_attr_t thread_attr;
  int rv = 0;
  
  static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;  

  pthread_attr_init( &thread_attr );
  pthread_attr_setdetachstate( &thread_attr, PTHREAD_CREATE_DETACHED );
#endif
	
  CControlObject *pctrlObject = (CControlObject *)p;

#ifdef WIN32

  while ( !pctrlObject->m_bQuit )	{	

		// Create the named pipe
    if ( INVALID_HANDLE_VALUE != 
				( hPipe = CreateNamedPipe( CANAL_CLIENT_PIPE,
											PIPE_ACCESS_DUPLEX,
											PIPE_TYPE_MESSAGE |
											PIPE_READMODE_MESSAGE |
											PIPE_WAIT,
											PIPE_UNLIMITED_INSTANCES,
											PIPE_BUF_SIZE,
											PIPE_BUF_SIZE,
											PIPE_TIMEOUT,
											NULL ) ) )
		{
	  
			// Wait for the client to connect;
			if ( !ConnectNamedPipe( hPipe, NULL ) ) {
				errorCode = GetLastError();

				// The client could not connect, so close the pipe.
				CloseHandle( hPipe );
				continue;
			}

			// A client has connected
			_clientstruct *pclientstruct = new _clientstruct;

			if ( NULL != pclientstruct ) {
			
				pclientstruct->hPipe = hPipe;				// Pipe handle
				pclientstruct->m_pctrlObject = pctrlObject;	// Control object

				// Start the client thread to serve the client
				if ( NULL == ( hThread = CreateThread( NULL,
														0,
														( LPTHREAD_START_ROUTINE ) clientThread,
														( LPVOID )pclientstruct,
														0,
														&dwThreadId ) ) ) {
					errorCode = GetLastError();
					CloseHandle( hPipe );
					continue;
				}

				CloseHandle( hThread );
			}			

		}
	} // while
		
	ExitThread( errorCode );	
		
		
#else	// UNIX -----------------------------------------------------
	
	printf("Listen Thread\n");
	
	int shmid;
	key_t cmdkey = SHMEM_LISTEN_KEY;
	shmid = shmget( cmdkey, sizeof( struct __shmOpen ), IPC_CREAT | 0666 );
	
	if ( -1 == shmid ) pthread_exit( &rv );
	
	// Get Listen thread shared memory	
	struct __shmOpen *pstruct;
	pstruct = (struct __shmOpen *)shmat( shmid, NULL, 0 );
	
	// Share the lock	
	pstruct->m_lockClientThread = &lock;
	
	// Initialize structure 
	pstruct->m_shmid = 0;	
	pstruct->m_bDoneCmd = false;
	
	// Open the gate....
	pthread_mutex_unlock( &lock );
	
	while( !pctrlObject->m_bQuit ) {
	
		pstruct->m_command = 0;
		pstruct->m_flags = 0;
		pstruct->m_cmdResult = 0;
		*pstruct->m_strIfName = 0;
		pstruct->m_bValidCmd = false;
				
		printf("Ready for connection\n");
		
		// Wait for a valid command to be handled
		while ( !pctrlObject->m_bQuit && !pstruct->m_bValidCmd ) {
			usleep( 1000 );
		}
		pstruct->m_bValidCmd = false;
		printf("semWait\n");	
		
		// Create a shared memory area for this client
		pstruct->m_shmid = shmget( IPC_PRIVATE, 512, IPC_CREAT | 0666 );
		if ( -1 != pstruct->m_shmid ) {
		
			_clientstruct *pclientstruct = new _clientstruct;			
			pclientstruct->m_shmid = pstruct->m_shmid;		// id for share memory
			pclientstruct->m_pctrlObject = pctrlObject;		// Control object

			pthread_t threadId;

			// Launch the client handler thread
			if ( pthread_create( 	&threadId,
										&thread_attr,
										clientThread,
										pclientstruct ) ) {	
							
				pstruct->m_cmdResult = -2;	// ERROR: No client thread
				syslog( LOG_CRIT, "Unable to create client thread. Client turned down.");
					
			}
			
			pstruct->m_cmdResult = 0;	// All is OK
		}
		else {
		
			pstruct->m_cmdResult = -1;	// ERROR: no shared memory
			syslog( LOG_CRIT, "Unable to create shared memory for client.");
			
		}		
		
		// Command handled
		pstruct->m_bDoneCmd = true;
		printf("semDone\n");

	}
	
	// Release shared memory
	shmdt( (void *)pstruct );
	
	// Remove the share memory
	shmctl( shmid, IPC_RMID, 0 );
	
	//fclose ( fp );
		
	pthread_exit( &rv );
	
#endif
   
}
