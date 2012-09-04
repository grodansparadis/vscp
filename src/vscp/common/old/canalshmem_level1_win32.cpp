///////////////////////////////////////////////////////////////////////////////
// CanalShMem_level1_win32.cpp: implementation of the CanalSharedMemLevel1 class.
//
// Copyright (C) 2000-2008 Ake Hedman, eurosource, <akhe@eurosource.se>
//
// This software is placed into
// the public domain and may be used for any purpose.  However, this
// notice must not be changed or removed and no warranty is either
// expressed or implied by its publication or distribution.
//
// $RCSfile: canalshmem_level1_win32.cpp,v $                                       
// $Date: 2006/03/10 12:35:28 $                                  
// $Author: akhe $                                              
// $Revision: 1.16 $ 
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

#include "wx/log.h"


#include "canal_macro.h"
#include "canal_win32_ipc.h"
#include "../common/canal.h"
#include "canalshmem_level1_win32.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CanalSharedMemLevel1::CanalSharedMemLevel1()
{
  m_pLevel1 = NULL;
  m_semClientCommand = NULL;
  m_semClientDone = NULL;
}


CanalSharedMemLevel1::~CanalSharedMemLevel1()
{
  if ( NULL != m_semClientCommand ) {
    CloseHandle( m_semClientCommand );
  }

  if ( NULL != m_semClientDone ) {
    CloseHandle( m_semClientDone );
  }
}


///////////////////////////////////////////////////////////////////////////////
// doCommand
//

bool CanalSharedMemLevel1::doCommand( unsigned char cmd )
{
	if ( NULL == m_pLevel1 ) return false;

	// http://msdn.microsoft.com/msdnmag/issues/0300/security/default.aspx
	// here we use a SECURITY_DESCRIPTOR to say
	// that we don't want *any DACL at all*
	SECURITY_DESCRIPTOR sd;
	InitializeSecurityDescriptor( &sd,
									SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&sd, TRUE, 0, FALSE);

	//SECURITY_ATTRIBUTES sa = { sizeof sa, &sd, FALSE };
	
	// Init reply data
	m_pLevel1->m_Response = 0x00;
	m_pLevel1->m_ResponseCode = 0x00;
	
	// We have an open channel!	Open interface
	m_pLevel1->m_command = cmd;
	
	// Execute command on server
	ReleaseSemaphore( m_semClientCommand, 1, NULL );

	// Wait until done
	WaitForSingleObject( m_semClientDone, 500 );

	
	if ( CANAL_RESPONSE_SUCCESS != m_pLevel1->m_Response ) {
	
		// Command failed
		return false;	
	}
		
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// open
//

bool CanalSharedMemLevel1::doCmdOpen( const char *szInterface, unsigned long flags )
{
  HANDLE hMapListen = NULL;
  HANDLE hListenMutex = NULL;
  HANDLE semListenCommand = NULL;
  HANDLE semListenDone = NULL;
  DWORD err;

  bool rv = true;	// return value
  wxChar buf[ MAX_PATH ];
  struct __shmOpen *pListenShMemArea = NULL;	

	
  ::wxLogDebug( _T("CanalSharedMemLevel1: Before %ul"), GetLastError() );
  SetLastError( 0 );
	

  if ( NULL == ( semListenCommand = 
		 OpenSemaphore( SEMAPHORE_ALL_ACCESS, 
				FALSE, 
				CANAL_LISTEN_COMMAND_SEM ) ) ) {
					 		
    ::wxLogDebug( _T("CanalSharedMemLevel1: Failed to open Listen Command semaphore %ul"), 
		  ( err = GetLastError() ) );
    return false;
  }


  if ( NULL == ( semListenDone = 
		 OpenSemaphore( SEMAPHORE_ALL_ACCESS, 
				FALSE, 
				CANAL_LISTEN_DONE_SEM ) ) ) {
		
    ::wxLogDebug( _T("CanalSharedMemLevel1: Failed to open Listen Done semaphore %ul"), 
		  ( err = GetLastError() ) );
    CloseHandle( semListenCommand );
    return false;	
  }

  // Get client shared memory
  if ( NULL == ( hMapListen = 
		 OpenFileMapping( FILE_MAP_WRITE, 
				  TRUE, 
				  CANAL_LISTEN_SHM_NAME ) ) ) {
		
    ::wxLogDebug( _T("CanalSharedMemLevel1: Failed to open Listen Command semaphore %ul"), 
		  ( err = GetLastError() ) );
    CloseHandle( semListenCommand );
    CloseHandle( semListenDone );
    return false;
  }

  if ( NULL == ( pListenShMemArea = 
		 (struct __shmOpen *)MapViewOfFile( hMapListen, 
						    FILE_MAP_WRITE, 0, 0, 0 ) ) ) {
		
    ::wxLogDebug( _T("CanalSharedMemLevel1: Failed to open Listen Shared memory %ul"), 
		  ( err = GetLastError() ) );
    CloseHandle( hMapListen );
    CloseHandle( semListenCommand );
    CloseHandle( semListenDone );
    return false;
  }
	
  if ( NULL == ( hListenMutex = 
		 OpenMutex( MUTEX_ALL_ACCESS, 
			    TRUE, 
			    CANAL_LISTEN_SHARED_MEMORY_MUTEX ) ) ) {

    ::wxLogDebug( _T("CanalSharedMemLevel1: Failed to open Listen Nutex %ul"), 
		  GetLastError() );
    UnmapViewOfFile( pListenShMemArea );
    CloseHandle( hMapListen );
    CloseHandle( semListenCommand );
    CloseHandle( semListenDone );
    return false;
  }

  // Get access to Listenthread common resources
  WaitForSingleObject( hListenMutex, INFINITE );
					
  // Write Command	
  pListenShMemArea->m_command = CANAL_COMMAND_OPEN_VSCP_LEVEL1;
  pListenShMemArea->m_flags = flags;
  if ( NULL != szInterface ) {
    strncpy( pListenShMemArea->m_strIfName, szInterface, sizeof( szInterface ) );
  }
		
  // Execute command on server
  ReleaseSemaphore( semListenCommand, 1, NULL );

  // Wait until command is handled
  WaitForSingleObject( semListenDone, INFINITE );

  if ( !pListenShMemArea->m_cmdResult ) {
	
    // Give the server thread some time to start
    Sleep( 1 );
		
    // Get the client command memory area
    _stprintf( buf, CANAL_LISTEN_CLIENT_SHM_TEMPLATE, pListenShMemArea->m_shmid );
    if ( NULL != ( m_hMapClient = 
		   OpenFileMapping( FILE_MAP_WRITE, TRUE, buf ) ) ) {
		
      if ( NULL != ( m_pCommandMem = MapViewOfFile( m_hMapClient, 
						    FILE_MAP_WRITE, 0, 0, 0 ) ) ) {
			
	// Save a pointer to the command structure
	m_pLevel1 = (struct __shmCanalLevelI *)m_pCommandMem;

	// Wait for server thread to give clearence
	DWORD start = GetTickCount();
	while( 0xaa55 != m_pLevel1->m_test ) {
	  if ( ( GetTickCount() - start ) > 1000 ) {
			
	    // Release listen thread resources we are done with them
				
	    // No need for memory anymore
#ifndef CANAL_MFCAPP
	    wxLogDebug(_T("ERROR: No server thread\n"));
#endif
	    return false;
	  }
	}

	// Get the Client Semaphores
	_stprintf( buf, CANAL_CLIENT_COMMAND_SEM_TEMPLATE, pListenShMemArea->m_shmid );
	if ( NULL == ( m_semClientCommand = OpenSemaphore( SEMAPHORE_ALL_ACCESS, 
														TRUE, 
														buf ) ) ) {
	  CloseHandle( semListenCommand );
	  return false;	
	}

	_stprintf( buf, CANAL_CLIENT_DONE_SEM_TEMPLATE, pListenShMemArea->m_shmid );
	if ( NULL == ( m_semClientDone = OpenSemaphore( SEMAPHORE_ALL_ACCESS, 
														TRUE, 
														buf ) ) ) {
	  CloseHandle( semListenCommand );
	  return false;	
	}

	// Go on
	m_pLevel1->m_test = 0x55aa; 

	// We have an open channel!	
	// Do a dummy Open of interface
	doCommand( CANAL_COMMAND_OPEN );

      }
      else {

	// Failed to map in client shared memory
				
      }

    }
    else {

      // Failed to get Client shared memory

    }

  }
	
  // Release listen thread resources we are done with them
  ReleaseMutex( hListenMutex );

  if ( NULL != semListenCommand ) {
    CloseHandle( semListenCommand );
  }

  if ( NULL != semListenDone ) {
    CloseHandle( semListenDone );
  }
	
  // release listthread sh memory 
  if ( NULL != pListenShMemArea ) {
    UnmapViewOfFile( pListenShMemArea );
  }

  // ...and handle for shared memory
  if ( NULL != hMapListen ) {
    CloseHandle( hMapListen );
  }
		
  return rv;
}


///////////////////////////////////////////////////////////////////////////////
// close
//

bool CanalSharedMemLevel1::doCmdClose( void )
{	
  bool rv;
	

  if ( NULL == m_pLevel1 ) return false;
	
  // Must have contact with server
  if ( NULL == m_pCommandMem ) return false;
	
  // Do the close command 	
  rv = doCommand( CANAL_COMMAND_CLOSE );
	
  if ( CANAL_RESPONSE_SUCCESS != m_pLevel1->m_Response ) {
		
    // Command failed
  }
		
  if ( NULL != m_semClientCommand ) {
    CloseHandle( m_semClientCommand );
    m_semClientCommand = NULL;
  }

  if ( NULL != m_semClientDone ) {
    CloseHandle( m_semClientDone );
    m_semClientDone = NULL;
  }
	
  // release listthread sh memory 
  if ( NULL != m_pCommandMem ) {
    UnmapViewOfFile( m_pCommandMem );
  }

  // ...and handle for shared memory
  if ( NULL != m_hMapClient ) {
    CloseHandle( m_hMapClient );
    m_hMapClient = NULL;
  }

  m_pLevel1 = NULL;
		
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
	
  if ( NULL == m_pLevel1 ) return false;
	
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
	
  if ( NULL == m_pLevel1 ) return false;

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
	
  if ( NULL == m_pLevel1 ) return false;

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

  if ( NULL == m_pLevel1 ) return false;
	
  // Must have contact with server
  if ( NULL == m_pCommandMem ) return false;
	
  if ( !doCommand( CANAL_COMMAND_CHECKDATA ) ) {
    nMsg = 0;	// Failure
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

  if ( NULL == m_pLevel1 ) return false;
	
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
	
  if ( NULL == m_pLevel1 ) return false;
	
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
	
  if ( NULL == m_pLevel1 ) return false;
	
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
	
  if ( NULL == m_pLevel1 ) return false;
	
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
	
  if ( NULL == m_pLevel1 ) return false;
	
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
	
  if ( NULL == m_pLevel1 ) return false;

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
	
  if ( NULL == m_pLevel1 ) return false;
	
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

#ifdef __WXWINDOWS__

bool CanalSharedMemLevel1::doCmdVendorString( wxString& strVendor )
{
  bool rv = true;
	
  if ( NULL == m_pLevel1 ) return false;
	
  // Must have contact with server
  if ( NULL == m_pCommandMem ) return false;
	
  if ( !doCommand( CANAL_COMMAND_VENDOR_STRING ) ) {
    rv = false;	// Failure
  }
  else {
    strVendor = (wxChar *)m_pLevel1->m_data;
  }
	
  return rv;
}

#endif

bool CanalSharedMemLevel1::doCmdVendorString( char *pVendorStr, int maxsize )
{
  bool rv = true;
	
  if ( NULL == m_pLevel1 ) return false;
	
  // Must have contact with server
  if ( NULL == m_pCommandMem ) return false;
	
  if ( !doCommand( CANAL_COMMAND_VENDOR_STRING ) ) {
    rv = false;	// Failure
  }
  else {
    memcpy( pVendorStr, m_pLevel1->m_data, MIN( maxsize, 512 ) );
  }
	
  return rv;
}

#