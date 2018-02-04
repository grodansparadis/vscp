// msglog.cpp: implementation of the CMsgLog class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
//  Copyright (C) 2000-2018 Ake Hedman, Grodans Paradis AB, akhe@grodansparadis.com 
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
// $RCSfile: log.cpp,v $                                       
// $Date: 2005/10/14 17:19:44 $                                  
// $Author: akhe $                                              
// $Revision: 1.5 $ 

#ifdef WIN32

#include <windows.h>

#else

#include "unistd.h"
#include "stdlib.h"
#include "limits.h"

#endif

#include "msglog.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CMsgLog
//

CMsgLog::CMsgLog()
{ 

}

//////////////////////////////////////////////////////////////////////
// ~CMsgLog
//

CMsgLog::~CMsgLog()
{		 
    close();
}

 


//////////////////////////////////////////////////////////////////////
// open
//
// filename
//      the name of the log file
//
// flags 
//      bit 1 = 0 Append
//      bit 1 = 1 Rewrite
//

bool CMsgLog::open( const char *szfile, unsigned long flags )
{
    char *p;
    bool rv = false;

#ifdef WIN32
  char szRWFile[ MAX_PATH ];
    char szFile[ MAX_PATH ];

#else
  char szRWFile[ PATH_MAX ];
    char szFile[ PATH_MAX ];
#endif
    
  if ( NULL != szfile ) {
#ifdef WIN32
    strncpy( szRWFile, szfile, MAX_PATH );
#else
    strncpy( szRWFile, szfile, PATH_MAX );
#endif
  }

    // Get filename
    p = strtok( szRWFile, ";" );
    if ( NULL == p ) return false;

#ifdef WIN32
    strncpy( szFile, p, MAX_PATH );
#else
    strncpy( szFile, p, PATH_MAX );
#endif

    // Get filter
    p = strtok( NULL, ";" );
    if ( NULL != p ) m_filter = getDataValue( p );

    // Get mask
        p = strtok( NULL, ";" );
    if ( NULL != p ) m_mask = getDataValue( p );


#ifdef WIN32	

    m_logMutex = CreateMutex( NULL, true, CANAL_DLL_LOGGER_OBJ_MUTEX );

    if ( flags & 1 ) {
    
        // Rewrite
        if ( NULL != ( m_flog = fopen( szFile, "w" ) ) ) {
            rv = true;
        }
    }
    else { 
        // Append
        if ( NULL != ( m_flog = fopen( szFile, "a" ) ) ) {
            rv = true;
        }		
    }

    // Start write thread 
    if ( rv ) {
    
        // Create the log write thread.
        DWORD dwThreadId;
        if ( NULL == 
                ( m_hTread = CreateThread(	NULL,
                                            0,
                                            (LPTHREAD_START_ROUTINE) workThread,
                                            this,
                                            0,
                                            &dwThreadId ) ) ) { 
            // Failure
            rv = false;
            close();
        }
    }
    
    // Release the mutex
    ReleaseMutex( m_logMutex );
    
#else // LINUX

    pthread_attr_t thread_attr;

    pthread_attr_init( &thread_attr );
    pthread_mutex_init( &m_logMutex, NULL );

    if ( flags & 1 ) {
    
        // Rewrite
        if ( NULL != ( m_flog = fopen( szFile, "w" ) ) ) {
            rv = true;
        }
    }
    else { 
        // Append
        if ( NULL != ( m_flog = fopen( szFile, "a" ) ) ) {
            rv = true;
        }		
    }

    // Start write thread 
    if ( rv ) {
    
        // Create the log write thread.
        if ( pthread_create( 	&m_threadId,
                                        &thread_attr,
                                        workThread,
                                        this ) ) {	
                            
            syslog( LOG_CRIT, "canallogger: Unable to create logger thread.");
            rv = false;
            fclose( m_flog );
        }
        
    }
    
    // Release the mutex
    pthread_mutex_unlock( &m_logMutex );

#endif	

    return rv;
}

//////////////////////////////////////////////////////////////////////
// close
//

void CMsgLog::close( void )
{	
    // Do nothing if already terminated
    if ( !m_bRun ) return;
    
    m_bRun = false;
 
    UNLOCK_MUTEX( m_logMutex );
    LOCK_MUTEX( m_logMutex );

    

    // Give the worker thread some time to terminate
#ifdef WIN32	
    DWORD rv;
    while ( true ) {
        GetExitCodeThread( m_hTread, &rv );
        if ( STILL_ACTIVE != rv ) break;
    }
    
    CloseHandle( m_logMutex );
    
    m_logMutex = NULL;
    fflush( m_flog );
    fclose( m_flog );
    
#else
    int *trv;
    pthread_join( m_threadId, (void **)&trv );
    pthread_mutex_destroy( &m_logMutex );
    
    fflush( m_flog );
    fclose( m_flog );
#endif

    
}

//////////////////////////////////////////////////////////////////////
// writeMsg
//

bool CMsgLog::writeMsg( canalMsg *pMsg )
{	
    bool rv = false;
    
    if ( NULL != pMsg ) {

        // Filter
        if ( !doFilter( pMsg ) ) return true;
        
        // Must be room for the message
        if ( m_logList.nCount < CANAL_LOG_LIST_MAX_MSG ) {

            dllnode *pNode = new dllnode;
            
            if ( NULL != pNode ) {
            
                canalMsg *pcanalMsg = new canalMsg;

                pNode->pObject = pcanalMsg;
                pNode->pKey = NULL;
                pNode->pstrKey = NULL;

                if ( NULL != pcanalMsg ) {
                    memcpy( pcanalMsg, pMsg, sizeof( canalMsg ) );
                }

                LOCK_MUTEX( m_logMutex );
                dll_addNode( &m_logList, pNode );
                UNLOCK_MUTEX( m_logMutex );
 
                rv = true;

            }
            else {

                delete pMsg;

            }
        }
    }
    
    return rv;		
}



///////////////////////////////////////////////////////////////////////////////
// getDataValue
//

unsigned long CMsgLog::getDataValue( const char *szData )
{
        unsigned long val;
        char *nstop;

        if ( ( NULL != strchr( szData, 'x' ) ) ||
                        ( NULL != strchr( szData, 'X' ) ) ) {
                val = strtoul( szData, &nstop, 16 );
        }
        else {
                val = strtoul( szData, &nstop, 10 );
        }

        return val;
}



///////////////////////////////////////////////////////////////////////////////
// workThread
//
//

#ifdef WIN32
void workThread( void *pObject )
#else
void *workThread( void *pObject )
#endif
{
#ifdef WIN32
    DWORD errorCode = 0;
#else
    int rv = 0;
#endif
    char buf[ 1024 ];
    char smallbuf[ 32 ];
    char tempbuf[ 32 ];
    time_t ltime;

    CMsgLog * pobj = ( CMsgLog *)pObject;
    if ( NULL == pobj ) {
#ifdef WIN32	
        ExitThread( errorCode ); // Fail
#else
        pthread_exit( &rv );
#endif
    }
    
    while ( pobj->m_bRun ) {

        LOCK_MUTEX( pobj->m_logMutex );
        
        // Noting to do if we should end...
        if ( !pobj->m_bRun ) continue;

        // Is there something to transmit
        while ( ( NULL != pobj->m_logList.pHead ) && ( NULL != pobj->m_logList.pHead->pObject ) ) {

            canalMsg msg;
            memcpy( &msg, pobj->m_logList.pHead->pObject, sizeof( canalMsg ) ); 
            dll_removeNode( &pobj->m_logList, pobj->m_logList.pHead );

            // Get UNIX-style time and display as number and string. 
            time( &ltime );
            strcpy( tempbuf, ctime( &ltime ) );		
            tempbuf[ strlen( tempbuf ) - 1 ] = 0;			// Remove /n
 
            sprintf( buf, "%s - %08lX %08lX %08lX %02X ", 
                            tempbuf,
                            msg.timestamp,
                            msg.flags,
                            msg.id,
                            msg.sizeData );
                
            for ( int i=0; i < 8; i++ ) {
                
                if ( i < msg.sizeData ) {
                    sprintf( smallbuf, "%02X ", msg.data[ i ] ); 
                    strcat( buf, smallbuf );
                }
                    
            }
                
            strcat( buf, "\n" );
            fprintf( pobj->m_flog, buf );
            fflush( pobj->m_flog );
                        
        } // while data
        //else {  // No data to write

        UNLOCK_MUTEX( pobj->m_logMutex );
        SLEEP( 1 );

        //}	 
    
    } // while 	 

#ifdef WIN32
    ExitThread( errorCode );
#else
    pthread_exit( &rv );
#endif

}
