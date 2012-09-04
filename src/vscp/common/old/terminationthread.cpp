
/*! 
 *	\file TerminationThread.cpp
 *  \brief Implementation of the Terminatation Thread
 *  \author Ake Hedman, eurosource, 
 *  \author akhe@eurosource.se 
 *  \date    2002-2005
 */

// $RCSfile: terminationthread.cpp,v $                                       
// $Date: 2005/09/06 21:19:10 $                                  
// $Author: akhe $                                              
// $Revision: 1.8 $ 

#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"


#ifdef WIN32

/// \def Windows: Pipe for machine control
#define CANAL_TERMINATOR_PIPE "\\\\.\\pipe\\canalpipeterminator___"

#else

#define _POSIX
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef DWORD
#define DWORD unsigned long
#endif

/// Pipe for interprocess communication.
#define CANAL_TERMINATOR_PIPE	"/tmp/canalpipe_termination"

#endif

#include "options.h"
#include "controlobject.h"

#define PIPE_BUF_SIZE		512
#define PIPE_TIMEOUT		1000

///////////////////////////////////////////////////////////////////////////////
// terminationThread
//

void terminationThread( void *p )
{

#ifdef WIN32

  HANDLE hPipe;
  DWORD errorCode = 0;
 
#else
  
  int hPipe;
  pthread_attr_t thread_attr;
  int rv = 0;

  pthread_attr_init( &thread_attr );
  pthread_attr_setdetachstate( &thread_attr, PTHREAD_CREATE_DETACHED );

#endif
	
  CControlObject *pobj = (CControlObject *)p;

#ifdef WIN32

	while( !pobj->m_bQuit  ) {

 
		if ( INVALID_HANDLE_VALUE != 
					( hPipe = CreateNamedPipe( CANAL_TERMINATOR_PIPE,
													PIPE_ACCESS_DUPLEX,
													PIPE_TYPE_MESSAGE |
													PIPE_READMODE_MESSAGE |
													PIPE_WAIT,
													PIPE_UNLIMITED_INSTANCES,
													PIPE_BUF_SIZE,
													PIPE_BUF_SIZE,
													PIPE_TIMEOUT,
													NULL ) ) ) {
	  
			// Wait for the client to connect;
			if ( !ConnectNamedPipe( hPipe, NULL ) ) {
			
				errorCode = GetLastError();
	
				// The client could not connect, so close the pipe.
				CloseHandle( hPipe );
				ExitThread( errorCode );
			}
		}
			
		// Get Data from the client
		char msgbuf[256];
		DWORD readCnt;

		if ( ReadFile( hPipe, msgbuf, sizeof( msgbuf ), &readCnt, NULL ) ) {
				
			// * * * Shutdown application * * *
			if ( readCnt && strstr( msgbuf , "QUIT") ) {
				strcpy( msgbuf, "OK" );	
				// Shutdown the application					
				pobj->m_bQuit = true;			  // end application
			}
				
			// * * * Ping * * *
			else if ( readCnt && strstr( msgbuf , "HELLO") ) {
				strcpy( msgbuf, "HELLO" );	// send echo...					
			}
				
			// * * * Shutdown machine * * *
			else if ( readCnt && strstr( msgbuf , "SHUTDOWN") ) {
				strcpy( msgbuf, "OK" );
				pobj->m_bQuit = true;				// end application
			}			
	

			WriteFile( hPipe, msgbuf, strlen( msgbuf ), &readCnt, NULL );

			FlushFileBuffers( hPipe ); 
			DisconnectNamedPipe( hPipe ); 
			CloseHandle( hPipe ); 
			
		}

	} // while	

	ExitThread( errorCode );
	
	
	
		
#else	// Linux



	//
	// USR   -- Read/Write
	// GROUP -- Read/Write
	// OTHER -- Read/write
	//
	
	if ( -1 == mkfifo( CANAL_TERMINATOR_PIPE, S_IRUSR | S_IWUSR |
											S_IRGRP | S_IWGRP |
											S_IROTH | S_IWOTH )  &&
											( EEXIST != errno ) ) {
			
		syslog( LOG_CRIT, "Unable to create fifo. Terminating listenthread.");
		pthread_exit( &rv );
	}

	if ( -1 == ( hPipe = open( CANAL_TERMINATOR_PIPE , O_RDWR  ) ) ) {
  
		syslog( LOG_CRIT, "Unable to open fifo. Terminating listenthread.");
		pthread_exit( &rv );
	
	}

	char msgbuf[ 32 ];
	ssize_t readCnt;
	
	while( !pobj->m_bQuit ) {
	
		if ( -1 == ( readCnt = read( hPipe, msgbuf, sizeof( msgbuf ) ) ) ) {
	
			if ( 0 < readCnt ) {
				
				// * * * Shutdown application * * *
				if ( readCnt && strstr( msgbuf , "QUIT") ) {
					strcpy( msgbuf, "OK" );	
					// Shutdown the application					
					pobj->m_bQuit = true;			// end application
				}
				
				// * * * Ping * * *
				else if ( readCnt && strstr( msgbuf , "HELLO") ) {
					strcpy( msgbuf, "HELLO" );	// send echo...					
				}
				
				// * * * Shutdown machine * * *
				else if ( readCnt && strstr( msgbuf , "SHUTDOWN") ) {
					strcpy( msgbuf, "OK" );
					pobj->m_bQuit = true;			// shutdown application
				}		

				write( hPipe, msgbuf, strlen( msgbuf ) );
				
			}		
		}
	} // while
	
	close( hPipe );

	pthread_exit( &rv );
								
#endif
     
}

