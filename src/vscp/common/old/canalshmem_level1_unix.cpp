///////////////////////////////////////////////////////////////////////////////
// CanalShMem_level1_unix.cpp: implementation of the CanalSharedMemLevel1 class.
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
// $RCSfile: canalshmem_level1_unix.cpp,v $                                       
// $Date: 2005/09/07 18:05:14 $                                  
// $Author: akhe $                                              
// $Revision: 1.13 $ 
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

#ifdef WIN32

#else

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

#endif

#include "canal_macro.h"
#include "canal_unix_ipc.h"
#include "../common/canal.h"
#include "canalshmem_level1_unix.h"
 

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CanalSharedMemLevel1::CanalSharedMemLevel1()
{
  m_pCommandMem = NULL;
}


CanalSharedMemLevel1::~CanalSharedMemLevel1()
{
  if ( NULL != m_pCommandMem ) {
    shmdt( m_pCommandMem );
  }
}


///////////////////////////////////////////////////////////////////////////////
// doCommand
//

bool CanalSharedMemLevel1::doCommand( unsigned char cmd )
{
  struct sembuf sb;
	
  // Init reply data
  m_pLevel1->m_Response = 0x00;
  m_pLevel1->m_ResponseCode = 0x00;
	
  // We have an open channel!	Open interface
  m_pLevel1->m_command = cmd;
	
  // Execute command on server
  sb.sem_num = CLIENT_CMD_SEM;
  sb.sem_op =  1;
  sb.sem_flg = 0;
  semop( m_semCommand, &sb, 1 );

  // Wait until done
  sb.sem_num = CLIENT_DONE_SEM;
  sb.sem_op =  -1;
  sb.sem_flg = 0;
  semop( m_semCommand, &sb, 1 );

	
  if ( CANAL_RESPONSE_SUCCESS != m_pLevel1->m_Response ) {
	
    // Command failed
    return false;	
  }
		
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// open
//

bool CanalSharedMemLevel1::doCmdOpen( const wxString& strInterface, unsigned long flags )
{
  bool rv = true;	// return value
  struct __shmOpen *pListenShMemArea;	
  int shmid;
  struct sembuf sb;

  key_t cmdkey = IPC_LISTEN_SHM_KEY;
  if ( -1 == ( shmid = shmget( cmdkey, 0, 0 ) ) ) {
    ::wxLogDebug( _("CanalSharedMemLevel1: Failed to open Listen Command semaphore") );
    return false;
  }
	
  if ( -1 == (long)( pListenShMemArea =	
		    (struct __shmOpen *)shmat( shmid, NULL, 0 ) ) ) {
     ::wxLogDebug( _("CanalSharedMemLevel1: Failed to open Listen Done semaphore.") );
    return false;
  }
	
  int semListenObj;
  key_t semkey = IPC_LISTEN_SEM_KEY;
  // Semaphore 1 - access to shared memory
  // Semaphore 2 - valid command
  semListenObj = semget( semkey, 0, 0 );
  if ( -1 == semListenObj ) {
    shmdt( pListenShMemArea);
    ::wxLogDebug( _("CanalSharedMemLevel1: Failed to open Listen Command semaphore.") );
    return false;
  }
		
  // Get access to Listenthread common resources
  sb.sem_num = LISTEN_SEM_RESOURCE;
  sb.sem_op = -1;
  sb.sem_flg = 0;
  semop( semListenObj, &sb, 1 );
				
  // Write Command	
  pListenShMemArea->m_command = CANAL_COMMAND_OPEN_VSCP_LEVEL1;
  pListenShMemArea->m_flags = flags;
  if ( strInterface.Length() ) {
    memcpy( pListenShMemArea->m_strIfName, strInterface, strInterface.Length() );
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
      ::wxLogError(_("CanalSharedMemLevel1: Could not get shared memory. %d"), pListenShMemArea->m_shmid);
      rv = false;			
    }
    else {
		
      // Save a pointer to the command structure
      m_pLevel1 = (struct __shmCanalLevelI *)m_pCommandMem;
						
      // Wait for server thread to give clearence
      int cnt = 0;
      while( 0xaa55 != m_pLevel1->m_test ) {
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
      m_semCommand = m_pLevel1->m_semCmd;
			
      // Go on
      m_pLevel1->m_test = 0x55aa; 
									
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

bool CanalSharedMemLevel1::doCmdClose( void )
{	
  bool rv;
	
  // Must have contact with server
  if ( NULL == m_pCommandMem ) return false;
	
  // Do the close command 	
  rv = doCommand( CANAL_COMMAND_CLOSE );
	
  if ( CANAL_RESPONSE_SUCCESS != m_pLevel1->m_Response ) {
		
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

bool CanalSharedMemLevel1::doCmdNOOP( void )
{	
  // Must have contact with server
  if ( NULL == m_pCommandMem ) return false;
	
  // Do close command 	
  return doCommand( CANAL_COMMAND_NOOP );
	
}



///////////////////////////////////////////////////////////////////////////////
// doCmdGetLevel
//

bool CanalSharedMemLevel1::doCmdGetLevel( unsigned long *plevel )
{
  bool rv = true;
	
  // Must have contact with server
  if ( NULL == m_pCommandMem ) return false;
	
  // Must have somewhere to store result
  if ( NULL == plevel ) return false;
	
  if ( !doCommand( CANAL_COMMAND_LEVEL ) ) {
    rv = false;	// Failure
  }
  else {
    memcpy( plevel, m_pLevel1->m_data, sizeof( unsigned long ) );
  }

  return rv;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdSend
//

bool CanalSharedMemLevel1::doCmdSend( canalMsg *pMsg )
{	
  bool rv = true;
	
  // Must have contact with server
  if ( NULL == m_pCommandMem ) return false;
	
  // Must have something to send
  if ( NULL == pMsg ) return false;
	
  memcpy( m_pLevel1->m_data, pMsg, sizeof( canalMsg ) );
  if ( !doCommand( CANAL_COMMAND_SEND ) ) {
    rv = false;	// Failure
  }
	
  return rv;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdReceive
//

bool CanalSharedMemLevel1::doCmdReceive( canalMsg *pMsg )
{	
  bool rv = true;
	
  // Must have contact with server
  if ( NULL == m_pCommandMem ) return false;
	
  // Must have somewhere to store message
  if ( NULL == pMsg ) return false;
	
  if ( !doCommand( CANAL_COMMAND_RECEIVE ) ) {
    rv = false;	// Failure
  }
  else {
    memcpy( pMsg, m_pLevel1->m_data, sizeof( canalMsg ) );
  }
	
  return rv;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdDataAvailable
//

int CanalSharedMemLevel1::doCmdDataAvailable( void )
{
  int nMsg = 0;	
  
  // Must have contact with server
  if ( NULL == m_pCommandMem ) return false;
  
  if ( !doCommand( CANAL_COMMAND_CHECKDATA ) ) {
    nMsg = -1;	// Failure
  }
  else {
    memcpy( &nMsg, m_pLevel1->m_data, sizeof( int ) );
  }
  
  return nMsg;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdState
//

bool CanalSharedMemLevel1::doCmdStatus( canalStatus *pStatus )
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
    memcpy( pStatus, m_pLevel1->m_data, sizeof( int ) );
  }

  return rv;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdStatistics
//

bool CanalSharedMemLevel1::doCmdStatistics( canalStatistics *pStatistics )
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
    memcpy( pStatistics, m_pLevel1->m_data, sizeof( canalStatistics ) );
  }
	
  return rv;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdFilter
//

bool CanalSharedMemLevel1::doCmdFilter( unsigned long filter )
{	
  bool rv = true;
	
  // Must have contact with server
  if ( NULL == m_pCommandMem ) return false;
	
  memcpy( m_pLevel1->m_data, &filter, sizeof( unsigned long ) );
  if ( !doCommand( CANAL_COMMAND_FILTER ) ) {
    rv = false;	// Failure
  }
	
  return rv;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdMask
//

bool CanalSharedMemLevel1::doCmdMask( unsigned long mask )
{	
  bool rv = true;
	
  // Must have contact with server
  if ( NULL == m_pCommandMem ) return false;
	
  memcpy( m_pLevel1->m_data, &mask, sizeof( unsigned long ) );
  if ( !doCommand( CANAL_COMMAND_MASK ) ) {
    rv = false;	// Failure
  }
	
  return rv;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdBaudrate

bool CanalSharedMemLevel1::doCmdBaudrate( unsigned long baudrate )
{
  bool rv = true;
	
  // Must have contact with server
  if ( NULL == m_pCommandMem ) return false;
	
  memcpy( m_pLevel1->m_data, &baudrate, sizeof( unsigned long ) );
  if ( !doCommand( CANAL_COMMAND_BAUDRATE ) ) {
    rv = false;	// Failure
  }
	
  return rv;
}



///////////////////////////////////////////////////////////////////////////////
// doCmdVersion
//

bool CanalSharedMemLevel1::doCmdVersion( unsigned long *pversion )
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
    memcpy( pversion, m_pLevel1->m_data, sizeof( unsigned long ) );
  }
	
  return rv;

}



///////////////////////////////////////////////////////////////////////////////
// doCmdDLLVersion
//

bool CanalSharedMemLevel1::doCmdDLLVersion( unsigned long *pversion )
{
  bool rv = true;
	
  // Must have contact with server
  if ( NULL == m_pCommandMem ) return false;
	
  // Must have somewhere to store result
  if ( NULL == pversion ) return false;
	
  if ( !doCommand( CANAL_COMMAND_DLL_VERSION ) ) {
    rv = false;	// Failure
  }
  else {
    memcpy( pversion, m_pLevel1->m_data, sizeof( unsigned long ) );
  }

  return rv;

}


///////////////////////////////////////////////////////////////////////////////
// doCmdVendorString
//

bool CanalSharedMemLevel1::doCmdVendorString( wxString& strVendor )
{
  bool rv = true;
	
  // Must have contact with server
  if ( NULL == m_pCommandMem ) return false;
	
	
  if ( !doCommand( CANAL_COMMAND_VENDOR_STRING ) ) {
    rv = false;	// Failure
  }
  else {
    //memcpy( pVendorStr, m_pLevel1->m_data, MAX( maxsize, 512 ) );
    strVendor = (wxChar *)m_pLevel1->m_data;
  }
	
  return rv;
}

