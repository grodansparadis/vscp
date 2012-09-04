// listenthread.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2008 Ake Hedman, eurosource, <akhe@eurosource.se>
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
// $RCSfile: listenthread_unix.cpp,v $                                       
// $Date: 2005/09/07 16:01:15 $                                  
// $Author: akhe $                                              
// $Revision: 1.11 $ 

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

#include <wx/thread.h>


#define _POSIX

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <pthread.h>
#include <syslog.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#include "wx/defs.h"
#include "wx/app.h"
#include "wx/thread.h"
#include "wx/utils.h" 
#ifndef DWORD
#define DWORD unsigned long
#endif

#include "canal_unix_ipc.h"
#include "canalshmem_level1_unix.h"
#include "controlobject.h"

// Prototypes
void *clientThreadLevel1( void *p );
void *clientThreadLevel2( void *p );

///////////////////////////////////////////////////////////////////////////////
// listenThread
//
// This thread listens for conection on a pipe and starts a new thread
// to handle client requests
//

void listenThread( void *p )
{  
  pthread_attr_t thread_attr;
  int rv = 0;
   
  pthread_attr_init( &thread_attr );
  pthread_attr_setdetachstate( &thread_attr, PTHREAD_CREATE_DETACHED );
	
  CControlObject *pctrlObject = (CControlObject *)p;	

  wxLogDebug( _("Listen Thread: Started") );
	
  int shmid;
  key_t cmdkey = IPC_LISTEN_SHM_KEY;
  shmid = shmget( cmdkey, sizeof( struct __shmOpen ), IPC_CREAT | 0666 );	
  if ( -1 == shmid ) pthread_exit( &rv );
  
  // Get Listen thread shared memory	
  struct __shmOpen *pstruct = (struct __shmOpen *)shmat( shmid, NULL, 0 );
	
  int semListenObj;
  key_t semkey = IPC_LISTEN_SEM_KEY;

  // Get semaphores...
  // Semaphore 1 - access to shared memory
  // Semaphore 2 - valid command
  semListenObj = semget( semkey, 2, IPC_CREAT | 0666 );
  if ( -1 == semListenObj ) {
	
    // Release shared memory
    shmdt( (void *)pstruct );
		
    // Remove the share memory
    shmctl( shmid, IPC_RMID, 0 );
		
    pthread_exit( &rv );
  }
	
  // Open the gate....
  struct sembuf sb;
  sb.sem_num = LISTEN_SEM_RESOURCE;
  sb.sem_op = 1;
  sb.sem_flg = 0;
  rv = semop( semListenObj, &sb, 1 );
  if ( -1 == rv ) {
    wxLogDebug( _("Listen Thread: Failed to release command semaphore.") );
  }	

  while( !pctrlObject->m_bQuit ) {
	
    pstruct->m_command = 0;
    pstruct->m_flags = 0;
    pstruct->m_cmdResult = 0;
    *pstruct->m_strIfName = 0;
						
    // Wait for a valid command to be handled
    sb.sem_num = LISTEN_SEM_COMMAND;
    sb.sem_op = -1;
    sb.sem_flg = 0;
    rv = semop( semListenObj, &sb, 1 );
    if ( -1 == rv ) {
      wxLogDebug( _("Listen Thread: Failed to get command semaphore.") );
      continue;
    }
	
    wxLogDebug( _("Listen Thread: Client Command received.") );

    // Create a shared memory area for this client
    pstruct->m_shmid = shmget( IPC_PRIVATE, 
			       SHMEM_CLIENT_BUFFER_SIZE, 
			       IPC_CREAT | 0666 );
    if ( -1 != pstruct->m_shmid ) {
		
      _clientstruct *pclientstruct = new _clientstruct;			
      pclientstruct->m_shmid = pstruct->m_shmid;		// id for shared memory
      pclientstruct->m_pctrlObject = pctrlObject;		// Control object

      pthread_t threadId;

      if ( CANAL_COMMAND_OPEN_VSCP_LEVEL1 == pstruct->m_command ) {
	// Launch the client handler thread
	if ( pthread_create( 	&threadId,
				&thread_attr,
				clientThreadLevel1,
				pclientstruct ) ) {	
							
	  pstruct->m_cmdResult = -2;	// ERROR: No client thread
	  syslog( LOG_CRIT, "Listenthread: Unable to create client thread. Client turned down.");
					
	}

	wxLogDebug( _("Listen Thread: Client thread launched -  Level I") );		
	pstruct->m_cmdResult = 0;	// All is OK

      }
      else if ( CANAL_COMMAND_OPEN_VSCP_LEVEL2 == pstruct->m_command ) {
		
	// Launch the client handler thread
	if ( pthread_create( 	&threadId,
				&thread_attr,
				clientThreadLevel2,
				pclientstruct ) ) {	
							
	  pstruct->m_cmdResult = -2;	// ERROR: No client thread
	  syslog( LOG_CRIT, "Listenthread: Unable to create client Level I thread. Client turned down.");	
	}

	wxLogDebug( _("Listen Thread: Client thread launched - Level II.") );
	pstruct->m_cmdResult = 0;	// All is OK

      }
		
      else if ( CANAL_COMMAND_OPEN_VSCP_CONTROL == pstruct->m_command ) {
			
      }
			
      else {
		
	::wxLogError( _("Listenthread: Unknown command.\n") );
	pstruct->m_cmdResult = -1;	// ERROR: no shared memory
      }
		
    }
    else {

    	::wxLogError( _("Listenthread: Unable to create shared memory for Level I client.\n") );
        pstruct->m_cmdResult = -1;      // ERROR: no shared memory
        syslog( LOG_INFO, "Listenthread: Unable to create shared memory for client Level I.");
	break;
    }
			
  } // while
	
  semctl( semListenObj, 0, IPC_RMID, 0 );
  semctl( semListenObj, 1, IPC_RMID, 0 );
	
  // Release shared memory
  shmdt( (void *)pstruct );
	
  // Remove the share memory
  shmctl( shmid, IPC_RMID, 0 );
		
  pthread_exit( &rv );
	
   
}
