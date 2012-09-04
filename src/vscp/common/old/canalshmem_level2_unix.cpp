///////////////////////////////////////////////////////////////////////////////
// CanalShMem_level2_unix.cpp: implementation of the CanalSharedMemLevel2 class.
//
// This file is part is part of CANAL (CAN Abstraction Layer)
// http://www.vscp.org)
//
// Copyright (C) 2000-2008 Ake Hedman, eurosource, <akhe@eurosource.se>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// $RCSfile: canalshmem_level2_unix.cpp,v $                                       
// $Date: 2005/08/30 11:00:04 $                                  
// $Author: akhe $                                              
// $Revision: 1.7 $ 
///////////////////////////////////////////////////////////////////////////////


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

#include "canal_macro.h"
#include "canal_unix_ipc.h"
#include "../common/canal.h"
#include "canalshmem_level1_unix.h"
#include "canalshmem_level2_unix.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CanalSharedMemLevel2::CanalSharedMemLevel2()
{
  m_pCommandMem = NULL;
}


CanalSharedMemLevel2::~CanalSharedMemLevel2()
{
  if ( NULL != m_pCommandMem ) {
    shmdt( m_pCommandMem );
  }
}


///////////////////////////////////////////////////////////////////////////////
// doCommand
//

bool CanalSharedMemLevel2::doCommand( unsigned char cmd )
{
  struct sembuf sb;
	
  // Init reply data
  m_pLevel2->m_Response = 0x00;
  m_pLevel2->m_ResponseCode = 0x00;
	
  // We have an open channel!	Open interface
  m_pLevel2->m_command = cmd;
	
  // Execute command on server
  sb.sem_num = CLIENT_CMD_SEM;
  sb.sem_op =  1;
  sb.sem_flg = 0;
  semop( m_semCommand, &sb, 1 );
					
  sb.sem_num =  CLIENT_DONE_SEM;
  sb.sem_op = -1;
  sb.sem_flg = 0;
  semop( m_semCommand, &sb, 1 );
	
  if ( CANAL_RESPONSE_SUCCESS != m_pLevel2->m_Response ) {
	
    // Command failed
    return false;	
  }
		
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// open
//

bool CanalSharedMemLevel2::doCmdOpen( char *szInterface, unsigned long flags )
{
  bool rv = true;	// return value
  struct __shmOpen *pListenShMemArea;	
  int shmid;
  struct sembuf sb;

  key_t cmdkey = IPC_LISTEN_SHM_KEY;
  if ( -1 == ( shmid = shmget( cmdkey, 0, 0 ) ) ) {
    return false;
  }
	
  if ( -1 == (long)( pListenShMemArea = 
		    ( struct __shmOpen *)shmat( shmid, NULL, 0 ) ) ) {
    return false;
  }
	
  int semListenObj;
  key_t semkey = IPC_LISTEN_SEM_KEY;
  // Semaphore 1 - access to shared memory
  // Semaphore 2 - valid command
  semListenObj = semget( semkey, 0, 0 );
  if ( -1 == semListenObj ) {
    shmdt( pListenShMemArea);
    return false;
  }
		
  // Get access to Listenthread common resources
  sb.sem_num = LISTEN_SEM_RESOURCE;
  sb.sem_op = -1;
  sb.sem_flg = 0;
  semop( semListenObj, &sb, 1 );
				
  // Write Command	
  pListenShMemArea->m_command = CANAL_COMMAND_OPEN_VSCP_LEVEL2;
  pListenShMemArea->m_flags = flags;
  if ( NULL != szInterface ) {
    strncpy( pListenShMemArea->m_strIfName, szInterface, sizeof( szInterface ) );
  }
		
  // Execute command on server
  sb.sem_num = LISTEN_SEM_COMMAND;
  sb.sem_op =  1;
  sb.sem_flg = 0;
  semop( semListenObj, &sb, 1 );

  if ( !pListenShMemArea->m_cmdResult ) {
	
    // Give the server thread some time to start
    usleep( 1000 );
		
    // Get the client command memory area
    if ( -1 == (long)( m_pCommandMem = shmat( pListenShMemArea->m_shmid, NULL, 0 ) ) ) {	
      // Failed to get shared memory
      ::wxLogError(_("Could not get shared memory\n"));
      rv = false;			
    }
    else {
		
      // Save a pointer to the command structure
      m_pLevel2 = (struct __shmCanalLevelII *)m_pCommandMem;
						
      // Wait for server thread to give clearence
      int cnt = 0;
      while( 0xaa55 != m_pLevel2->m_test ) {
	sleep( 1 );
	cnt++;
	if ( cnt > 3 ) {
	  ::wxLogError(_("Appers to be no client thread\n"));
			
	  // Release listen thread resources we are done with them
	  sb.sem_num = LISTEN_SEM_RESOURCE;
	  sb.sem_op =  1;
	  sb.sem_flg = 0;
	  semop( semListenObj, &sb, 1 );
				
	  // No need for memory anymore
	  shmdt( pListenShMemArea );
	  return false;
	}
      }
			
      // Get the clinet semaphore
      m_semCommand = m_pLevel2->m_semCmd;
			
      // Go on
      m_pLevel2->m_test = 0x55aa; 
									
      // We have an open channel!	
      // Do a dummy Open of interface
      doCommand( CANAL_COMMAND_OPEN );
			
    }		
  }
  else {
    ::wxLogError(_("Cmd result error\n"));
    rv = false;
  }
		
  // Release listen thread resources we are done with them
  sb.sem_num = LISTEN_SEM_RESOURCE;
  sb.sem_op =  1;
  sb.sem_flg = 0;
  semop( semListenObj, &sb, 1 );	
	
  // release listthread sh memory 
  shmdt( pListenShMemArea );
		
  return rv;
}


///////////////////////////////////////////////////////////////////////////////
// close
//

bool CanalSharedMemLevel2::doCmdClose( void )
{	
  bool rv;
	
  // Must have contact with server
  if ( NULL == m_pCommandMem ) return false;
	
  // Do the close command 	
  rv = doCommand( CANAL_COMMAND_CLOSE );
	
  if ( CANAL_RESPONSE_SUCCESS != m_pLevel2->m_Response ) {
		
    // Command failed
    return false;
  }
		
  // release shared memory	
  if ( NULL != m_pCommandMem ) {
    shmdt( m_pCommandMem );
    m_pCommandMem = NULL;
  }
		
  return rv;
}



///////////////////////////////////////////////////////////////////////////////
// doCmdNOOP
//

bool CanalSharedMemLevel2::doCmdNOOP( void )
{	
  // Must have contact with server
  if ( NULL == m_pCommandMem ) return false;
	
  // Do close command 	
  return doCommand( CANAL_COMMAND_NOOP );
	
}


///////////////////////////////////////////////////////////////////////////////
// doCmdSend
//

bool CanalSharedMemLevel2::doCmdSend( vscpEvent *pEvent, bool bSetGUID )
{	
  bool rv = true;
		
  // Must have contact with server
  if ( NULL == m_pCommandMem ) return false;
	
  // Must have something to send
  if ( NULL == pEvent ) return false;
	
  m_pLevel2->m_cmdArg1 = bSetGUID;
	
  // Fix datasize if invalid
  if ( pEvent->sizeData > (512-25) ) pEvent->sizeData = (512-25);
	
  // Copy message
  memcpy( &m_pLevel2->m_VSCP_Event, pEvent, sizeof( vscpEvent ) );
	
  // Copy data
  memcpy( m_pLevel2->m_data, pEvent->pdata, pEvent->sizeData );
	
  if ( !doCommand( CANAL_COMMAND_SEND ) ) {
    rv = false;	// Failure
  }
	
  return rv;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdReceive
//

bool CanalSharedMemLevel2::doCmdReceive( vscpEvent *pEvent )
{	
  bool rv = true;
	
  // Must have contact with server
  if ( NULL == m_pCommandMem ) return false;
	
  // Must have somewhere to store message
  if ( NULL == pEvent ) return false;
	
  if ( !doCommand( CANAL_COMMAND_RECEIVE ) ) {
    rv = false;	// Failure
  }
  else {
		
    // Copy message
    memcpy( pEvent, &m_pLevel2->m_VSCP_Event, sizeof( vscpEvent ) );
		
    if ( pEvent->sizeData > 0 ) {
		
      pEvent->pdata = new uint8_t[ pEvent->sizeData ];
		
      if ( NULL != pEvent->pdata ) {
	// Copy data
	memcpy( pEvent->pdata, m_pLevel2->m_data, pEvent->sizeData );
      }
    }
		
  }
	
  return rv;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdDataAvailable
//

int CanalSharedMemLevel2::doCmdDataAvailable( void )
{
  int nMsg = 0;	
	
  // Must have contact with server
  if ( NULL == m_pCommandMem ) return false;
	
  if ( !doCommand( CANAL_COMMAND_CHECKDATA ) ) {
    nMsg = -1;	// Failure
  }
  else {
    memcpy( &nMsg, m_pLevel2->m_data, sizeof( int ) );
  }

  return nMsg;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdState
//

bool CanalSharedMemLevel2::doCmdStatus( canalStatus *pStatus )
{	
  bool rv = true;
	
  // Must have contact with server
  if ( NULL == m_pCommandMem ) return false;
	
  // Must have somewhere to store result
  if ( NULL == pStatus ) return false;
	
  if ( !doCommand( CANAL_COMMAND_STATUS ) ) {
    rv = false;	// Failure
  }
  else {
    memcpy( pStatus, m_pLevel2->m_data, sizeof( int ) );
  }

  return rv;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdStatistics
//

bool CanalSharedMemLevel2::doCmdStatistics( canalStatistics *pStatistics )
{	
  bool rv = true;
	
  // Must have contact with server
  if ( NULL == m_pCommandMem ) return false;
	
  // Must have somewhere to store result
  if ( NULL == pStatistics ) return false;
	
  if ( !doCommand( CANAL_COMMAND_STATISTICS ) ) {
    rv = false;	// Failure
  }
  else {
    memcpy( pStatistics, m_pLevel2->m_data, sizeof( canalStatistics ) );
  }
	
  return rv;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdFilter
//

bool CanalSharedMemLevel2::doCmdFilter( vscpEventFilter *pFilter )
{	
  bool rv = true;
	
  // Must have contact with server
  if ( NULL == m_pCommandMem ) return false;
	
  memcpy( m_pLevel2->m_data, &pFilter, sizeof( unsigned long ) );
  if ( !doCommand( CANAL_COMMAND_FILTER ) ) {
    rv = false;	// Failure
  }
	
  return rv;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdVersion
//

bool CanalSharedMemLevel2::doCmdVersion( unsigned long *pversion )
{
  bool rv = true;
	
  // Must have contact with server
  if ( NULL == m_pCommandMem ) return false;
	
  // Must have somewhere to store result
  if ( NULL == pversion ) return false;
	
  if ( !doCommand( CANAL_COMMAND_VERSION ) ) {
    rv = false;	// Failure
  }
  else {
    memcpy( pversion, m_pLevel2->m_data, sizeof( unsigned long ) );
  }
	
  return rv;

}


///////////////////////////////////////////////////////////////////////////////
// doCmdGetGUID
//

bool CanalSharedMemLevel2::doCmdGetGUID( char *pGUID )
{
  bool rv = true;
	
  // Must have contact with server
  if ( NULL == m_pCommandMem ) return false;
	
  // Must have somewhere to store result
  if ( NULL == pGUID ) return false;
	
  return rv;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdSetGUID
//

bool CanalSharedMemLevel2::doCmdSetGUID( char *pGUID )
{
  bool rv = true;
	
  // Must have contact with server
  if ( NULL == m_pCommandMem ) return false;
	
  // Must have somewhere to store result
  if ( NULL == pGUID ) return false;
	
  return rv;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdGetChannelInfo
//
bool CanalSharedMemLevel2::doCmdGetChannelInfo( VSCPChannelInfo *pChannelInfo )
{
  bool rv = true;
	
  // Must have contact with server
  if ( NULL == m_pCommandMem ) return false;
	
  // Must have somewhere to store result
  if ( NULL == pChannelInfo ) return false;
	
  return rv;
}



