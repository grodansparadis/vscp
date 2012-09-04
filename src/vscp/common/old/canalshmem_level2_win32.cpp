///////////////////////////////////////////////////////////////////////////////
// CanalShMem_level2_unix.cpp: implementation of the CanalSharedMemLevel2 class.
//
// Copyright (C) 2000-2008 Ake Hedman, eurosource, <akhe@eurosource.se>
//
// This software is placed into
// the public domain and may be used for any purpose.  However, this
// notice must not be changed or removed and no warranty is either
// expressed or implied by its publication or distribution.
//
// $RCSfile: canalshmem_level2_win32.cpp,v $                                       
// $Date: 2005/09/12 20:35:02 $                                  
// $Author: akhe $                                              
// $Revision: 1.14 $ 
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

#include "canal_macro.h"
#include "canal_win32_ipc.h"
#include "../common/canal.h"
#include "canalshmem_level1_win32.h"
#include "canalshmem_level2_win32.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CanalSharedMemLevel2::CanalSharedMemLevel2()
{	
	m_pLevel2 = NULL;
	m_semClientCommand = NULL;
	m_semClientDone = NULL;
}


CanalSharedMemLevel2::~CanalSharedMemLevel2()
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

bool CanalSharedMemLevel2::doCommand( unsigned char cmd )
{	
	if ( NULL == m_pLevel2 ) return false;

	// Init reply data
	m_pLevel2->m_Response = 0x00;
	m_pLevel2->m_ResponseCode = 0x00;
	
	// We have an open channel!	Open interface
	m_pLevel2->m_command = cmd;
	
	// Execute command on server
	ReleaseSemaphore( m_semClientCommand, 1, NULL );

	// Wait until done
	if ( WAIT_OBJECT_0 != WaitForSingleObject( m_semClientDone, 500 ) ) {
		return false;
	}
					
	
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
	HANDLE hMapListen = NULL;
	HANDLE hListenMutex = NULL;
	HANDLE semListenCommand = NULL;
	HANDLE semListenDone = NULL;

	bool rv = true;	// return value
	wxChar buf[ MAX_PATH ];
	struct __shmOpen *pListenShMemArea = NULL;	
 
	
	if ( NULL == ( semListenCommand = 
					OpenSemaphore( SEMAPHORE_ALL_ACCESS, TRUE, CANAL_LISTEN_COMMAND_SEM ) ) ) {
		return false;
	}


	if ( NULL == ( semListenDone = 
					OpenSemaphore( SEMAPHORE_ALL_ACCESS, TRUE, CANAL_LISTEN_DONE_SEM ) ) ) {
		CloseHandle( semListenCommand );
		return false;	
	}


	// Get client shared memory
	if ( NULL == ( hMapListen = 
					OpenFileMapping( FILE_MAP_WRITE, TRUE, CANAL_LISTEN_SHM_NAME ) ) ) {
		CloseHandle( semListenCommand );
		CloseHandle( semListenDone );
		return false;
	}

	if ( NULL == ( pListenShMemArea = 
					(struct __shmOpen *)MapViewOfFile( hMapListen, FILE_MAP_WRITE, 0, 0, 0 ) ) ) {
		CloseHandle( hMapListen );
		CloseHandle( semListenCommand );
		CloseHandle( semListenDone );
		return false;
	}
	
	if ( NULL == ( hListenMutex = 
					OpenMutex( MUTEX_ALL_ACCESS, TRUE, CANAL_LISTEN_SHARED_MEMORY_MUTEX ) ) ) {

		UnmapViewOfFile( pListenShMemArea );
		CloseHandle( hMapListen );
		CloseHandle( semListenCommand );
		CloseHandle( semListenDone );
		return false;
	}

	// Get access to Listenthread common resources
	WaitForSingleObject( hListenMutex, INFINITE );
					
	// Write Command	
	pListenShMemArea->m_command = CANAL_COMMAND_OPEN_VSCP_LEVEL2;
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
		
			if ( NULL != ( m_pCommandMem = MapViewOfFile( m_hMapClient, FILE_MAP_WRITE, 0, 0, 0 ) ) ) {
			
				// Save a pointer to the command structure
				m_pLevel2 = (struct __shmCanalLevelII *)m_pCommandMem;

				// Wait for server thread to give clearence
				DWORD start = GetTickCount();
				while( 0xaa55 != m_pLevel2->m_test ) {

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
				if ( NULL == ( m_semClientCommand = OpenSemaphore( SEMAPHORE_ALL_ACCESS, TRUE, buf ) ) ) {
					CloseHandle( semListenCommand );
					return false;	
				}


				_stprintf( buf, CANAL_CLIENT_DONE_SEM_TEMPLATE, pListenShMemArea->m_shmid );
				if ( NULL == ( m_semClientDone = OpenSemaphore( SEMAPHORE_ALL_ACCESS, TRUE, buf ) ) ) {
					//unsigned long err = GetLastError();
					CloseHandle( semListenCommand );
					return false;	
				}

				// Go on
				m_pLevel2->m_test = 0x55aa; 

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

bool CanalSharedMemLevel2::doCmdClose( void )
{	
	bool rv;
	if ( NULL == m_pLevel2 ) return false;
	
	// Must have contact with server
	if ( NULL == m_pCommandMem ) return false;
	
	// Do the close command 	
	rv = doCommand( CANAL_COMMAND_CLOSE );
	
	if ( CANAL_RESPONSE_SUCCESS != m_pLevel2->m_Response ) {
		
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
	
	if ( NULL == m_pLevel2 ) return false;
	
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
	if ( NULL != pEvent->pdata ) {
		memcpy( m_pLevel2->m_data, pEvent->pdata, pEvent->sizeData );
	}
	
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
	
	if ( NULL == m_pLevel2 ) return false;

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

	if ( NULL == m_pLevel2 ) return false;
	
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

	if ( NULL == m_pLevel2 ) return false;
	
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

	if ( NULL == m_pLevel2 ) return false;
	
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

	if ( NULL == m_pLevel2 ) return false;
	
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

	if ( NULL == m_pLevel2 ) return false;
	
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

	if ( NULL == m_pLevel2 ) return false;
	
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



