// can4vscpobj.cpp:  
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2015 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
// 
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.620
//

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <vscp_serial.h>
#include <crc8.h>
#include "can4vscpobj.h"
#include "dlldrvobj.h"

#ifdef WIN32
#include "callback.h"
#else
#include <limits.h>
#include <errno.h>
#include <math.h>
#endif


// Prototypes
#ifdef WIN32
void workThreadTransmit( void *pObject );
void workThreadReceive( void *pObject );
#else
void *workThreadTransmit( void *pObject );
void *workThreadReceive( void *pObject );
#endif


///////////////////////////////////////////////////////////////////////////////
// addWithEscape
//
//

static uint8_t addWithEscape( uint8_t *p, char c, uint8_t *pcrc ) 
{
	if ( DLE == c ) {
		*p = DLE;
        if ( NULL != pcrc ) crc8( pcrc, DLE );
		*(p+1) = DLE;
        if ( NULL != pcrc ) crc8( pcrc, DLE );
		return 2;
	}
	else {
		*p = c;
        if ( NULL != pcrc ) crc8( pcrc, c );
		return 1;
	}
}


///////////////////////////////////////////////////////////////////////////////
// getClockMilliseconds
//
//

static uint32_t getClockMilliseconds()
{
#ifdef WIN32	
    return (uint32_t)( ( 1000*(float)clock() ) / CLOCKS_PER_SEC);
#else
    timeval curTime;
	gettimeofday(&curTime, NULL);
	return  1000 * curTime.tv_sec + curTime.tv_usec / 1000;
#endif	
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// CCan4VSCPObj
//

CCan4VSCPObj::CCan4VSCPObj()
{ 
    m_initFlag = 0;
	
	// No filter mask
    m_filter = 0;
    m_mask = 0;

    // Default capabilities of connected device
    m_caps.maxVscpFrames = 1;    // We expect: One VSCP frame handled.
    m_caps.maxCanalFrames = 1;   // We expect: One CANAL frame handled.

    m_bRun = false;
    m_bOpen = false;

    m_RxMsgState = INCOMING_STATE_NONE;
    m_RxMsgSubState = INCOMING_SUBSTATE_NONE;

    m_sequencyno = 0;   // No frames sent yet
	

#ifdef WIN32
	
    m_hTreadReceive = 0;
    m_hTreadTransmit = 0;

    m_receiveDataEvent = NULL;

    m_transmitDataPutEvent = NULL;
	m_transmitDataGetEvent = NULL;

	// Create the device AND LIST access mutexes
    m_can4vscpMutex = CreateMutex( NULL, true, CANAL_DLL_CAN4VSCPDRV_OBJ_MUTEX );
    m_receiveMutex = CreateMutex( NULL, true, CANAL_DLL_CAN4VSCPDRV_RECEIVE_MUTEX );
    m_transmitMutex = CreateMutex( NULL, true, CANAL_DLL_CAN4VSCPDRV_TRANSMIT_MUTEX );
    m_responseMutex = CreateMutex( NULL, true, CANAL_DLL_CAN4VSCPDRV_RESPONSE_MUTEX );

    // Events
	m_receiveDataEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	m_transmitDataPutEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
	m_transmitDataGetEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

#else
	
    //m_flog = NULL;
    pthread_mutex_init( &m_can4vscpMutex, NULL );
    pthread_mutex_init( &m_receiveMutex, NULL );
    pthread_mutex_init( &m_transmitMutex, NULL );
    pthread_mutex_init( &m_responseMutex, NULL );
    
    sem_init( &m_receiveDataSem, 0, 0 );
    sem_init( &m_transmitDataPutSem, 0, 0 );
    sem_init( &m_transmitDataGetSem, 0, 0 );

#endif

    // Initialize lists
    dll_init( &m_transmitList, SORT_NONE );
    dll_init( &m_receiveList, SORT_NONE );
    dll_init( &m_responseList, SORT_NONE );
	
}

//////////////////////////////////////////////////////////////////////
// ~CCan4VSCPObj
//

CCan4VSCPObj::~CCan4VSCPObj()
{		 
	if ( m_bRun ) close();
	
	LOCK_MUTEX( m_transmitMutex );
	dll_removeAllNodes( &m_transmitList );
	
	LOCK_MUTEX( m_receiveMutex );
	dll_removeAllNodes( &m_receiveList );

	LOCK_MUTEX( m_responseMutex );
	dll_removeAllNodes( &m_responseList );


#ifdef WIN32	
	
	if ( NULL != m_can4vscpMutex ) CloseHandle( m_can4vscpMutex );	
	if ( NULL != m_receiveMutex ) CloseHandle( m_receiveMutex );
	if ( NULL != m_transmitMutex ) CloseHandle( m_transmitMutex );
	if ( NULL != m_responseMutex ) CloseHandle( m_responseMutex );

    // events
    if ( NULL != m_receiveDataEvent ) CloseHandle( m_receiveDataEvent );
	if ( NULL != m_transmitDataPutEvent ) CloseHandle( m_transmitDataPutEvent );
	if ( NULL != m_transmitDataGetEvent ) CloseHandle( m_transmitDataGetEvent );

#else
	
	sem_destroy( &m_receiveDataSem );
    sem_destroy( &m_transmitDataPutSem );
    sem_destroy( &m_transmitDataGetSem );

	pthread_mutex_destroy( &m_can4vscpMutex );
	pthread_mutex_destroy( &m_receiveMutex );
	pthread_mutex_destroy( &m_transmitMutex );
	pthread_mutex_destroy( &m_responseMutex );	

#endif
}




//////////////////////////////////////////////////////////////////////
// open
//
//
// pConfig
//-----------------------------------------------------------------------------
// Parameters for the driver as a string on the following form
//		
// "comport"
//
//
// comport
// =======
//	WIN32: 1 for COM1, 2 for COM2 etc
//	LINUX: /dev/ttyS1, /dev/ttyS2 etc
//
// Baudrate is always 115200
// 
// flags 
//-----------------------------------------------------------------------------
//
// bit 1/2
// =======
//	00 - Normal Mode (0)
//	01 - Listen Mode (1)
//	10 - Loopback Mode (2)
//	11 - Disabled Mode (3)
//
// bit 3
// =====
//  0  - Switch to CAN4VSCP mode is carried out on startup
//  1  - No switch to CAN4VSCP mode

int CCan4VSCPObj::open( const char *pConfig, unsigned long flags )
{
#ifdef WIN32   
	char szDrvParams[ MAX_PATH ];
#else
    char szDrvParams[ PATH_MAX ];
    char *pDeviceName;
#endif    
	char *p;
	int nComPort = 1;	// COM1 is default
    uint8_t saveseq;
    cmdResponseMsg Msg;

	m_initFlag = flags;

	m_RxMsgState = INCOMING_STATE_NONE;
	m_RxMsgSubState = INCOMING_SUBSTATE_NONE;

	// Save configuration string and convert to upper case
#ifdef WIN32    
	strncpy( szDrvParams, pConfig, MAX_PATH );
	_strupr( szDrvParams );
#else
    strncpy( szDrvParams, pConfig, PATH_MAX );
	// Make upper case
    p = szDrvParams;
#endif    

	// Initiate statistics
	m_stat.cntReceiveData = 0;
	m_stat.cntReceiveFrames = 0;
	m_stat.cntTransmitData = 0;
	m_stat.cntTransmitFrames = 0;

	m_stat.cntBusOff = 0;
	m_stat.cntBusWarnings = 0;
	m_stat.cntOverruns = 0;

	// if open we have noting to do
	if ( m_bRun ) return CANAL_ERROR_SUCCESS;

#ifdef DEBUG_CAN4VSCP_RECEIVE
#ifdef WIN32	
	m_flog = fopen( "c:/tmp/can4vscp.txt", "w" );
#else
	m_flog = fopen( "/tmp/can4vscp.txt", "w" );
#endif	
#endif	

	// Serial port
    nComPort = 1;   // Default com port
	p = strtok( szDrvParams, ";" );
	if ( NULL != p ) {
#ifdef WIN32        
        if ( NULL != ( p = strstr( p, "COM" ) ) ) {
		    nComPort = atoi( p+3 );	
        }
#else   
       pDeviceName = p;
#endif        
	}
    
	// Open the com port  CBR_128000 CBR_256000
#ifdef WIN32    
	if ( !m_com.init( nComPort,
						CBR_115200,
						8,
						NOPARITY,
						ONESTOPBIT,
						HANDSHAKE_NONE ) ) {
		return CANAL_ERROR_INIT_FAIL;
	}
#else
    

    // if open we have noting to do
    if (0 != m_com.getFD()) return 0;
    
    //----------------------------------------------------------------------
    // Open Serial Port
    //----------------------------------------------------------------------
    if ( !m_com.open( pDeviceName ) ) {
        syslog( LOG_CRIT, "can4vscp: Open [%s] failed\n", pDeviceName );
        return CANAL_ERROR_INIT_FAIL;
    }

    syslog( LOG_CRIT, "can4vscp: Open [%s] successful\n", pDeviceName );

    //----------------------------------------------------------------------
    // Com::setParam( char *baud, char *parity, char *bits, int HWFlow, int SWFlow )
    //----------------------------------------------------------------------
    m_com.setParam( (char*)"115200", (char*)"N", (char*)"8", 0, 0);

    //----------------------------------------------------------------------
    //
    //----------------------------------------------------------------------
    //m_receiveBuf[ 0 ] = 0;
    //m_cntRcv = 0;

    // Initiate statistics
    m_stat.cntReceiveData = 0;
    m_stat.cntReceiveFrames = 0;
    m_stat.cntTransmitData = 0;
    m_stat.cntTransmitFrames = 0;

    m_stat.cntBusOff = 0;
    m_stat.cntBusWarnings = 0;
    m_stat.cntOverruns = 0;
    
#endif    

    // Set CAN4VSCP mode in case of device in verbose mode
    if ( !( flags & CAN4VSCP_FLAG_NO_SWITCH_TO_NEW_MODE ) ) {  
#ifdef WIN32        
        m_com.writebuf( (unsigned char *)"SET MODE VSCP\r\n", 19 );     // In case of garbage in queue
        SLEEP( 200 );
        m_com.writebuf( (unsigned char *)"SET MODE VSCP\r\n", 19 );     // set CAN4VSCP mode twice
#else
        m_com.comm_puts( (char*)"SET MODE VSCP\r\n", 19 );              // In case of garbage in queue
        SLEEP( 200 );
        m_com.comm_puts( (char*)"SET MODE VSCP\r\n", 19 );              // set CAN4VSCP mode twice
#endif        
    }

    // Check that we have a CAN4VSCP device at the other end 
    // ( give it four tries before giving up )
    bool bFound = false;

    // Run run run ..... 
    m_bRun = true;

#ifdef WIN32
	
	// Start write thread 
	DWORD threadId;
	if ( NULL == 
			( m_hTreadTransmit = CreateThread(	NULL,
										            0,
										            (LPTHREAD_START_ROUTINE) workThreadTransmit,
										            this,
										            0,
										            &threadId ) ) ) { 
		// Failure
		close();
		return CANAL_ERROR_INIT_FAIL;
	}

	// Start read thread 
	if ( NULL == 
			( m_hTreadReceive = CreateThread(	NULL,
										            0,
										            (LPTHREAD_START_ROUTINE) workThreadReceive,
										            this,
										            0,
										            &threadId ) ) ) { 
		// Failure
		close();
		return  CANAL_ERROR_INIT_FAIL;
	}
	
	// Release the mutex
	UNLOCK_MUTEX( m_can4vscpMutex );
	UNLOCK_MUTEX( m_receiveMutex );
	UNLOCK_MUTEX( m_transmitMutex );
	UNLOCK_MUTEX( m_responseMutex );

#else // LINUX

    //----------------------------------------------------------------------
    //	acquire Mutex
    //----------------------------------------------------------------------
    pthread_attr_t thread_attr;
    pthread_attr_init( &thread_attr );
    pthread_mutex_init( &m_can4vscpObjMutex, NULL);
	
	
    // Create the log write thread.
    if ( pthread_create( &m_threadId,
							&thread_attr,
							workThreadTransmit,
							this ) ) {	
							
		syslog( LOG_CRIT, "can4vscp: Unable to create can4vscpdrv write thread.");
#ifdef DEBUG_CAN4VSCP_RECEIVE        
        if (NULL != m_flog) {
            fclose( m_flog );
            m_flog = NULL;
        }
#endif        
	}

        // Create the log write thread.
        if ( pthread_create( &m_threadId,
								&thread_attr,
								workThreadReceive,
								this ) ) {							
            syslog( LOG_CRIT, "can4vscp: Unable to create can4vscpdrv receive thread.");
#ifdef DEBUG_CAN4VSCP_RECEIVE        
        if (NULL != m_flog) {
            fclose( m_flog );
            m_flog = NULL;
        }
#endif        
	}
		
    // We are open
	m_bOpen = true;

	// Release the mutex
	pthread_mutex_unlock( &m_can4vscpMutex );

#endif	

    for ( int i=0; i<3; i++ ) {

        saveseq = m_sequencyno;         // Save the sequence ordinal
        if ( sendCommandWait( VSCP_SERIAL_DRIVER_COMMAND_NOOP,
                                NULL, 
                                0,
                                &Msg, 
                                500 ) ) {

            bFound = true;
            break;

        }

        SLEEP( 100 );

    }

    // Give up if not found
    if (!bFound) {
        // Failure
		close();
		return CANAL_ERROR_INIT_FAIL;
    }

    m_bOpen = true;

    // Get capabilities of device
    // We skip response and use defaults if call is not successful
    getDeviceCapabilities();

	// Set interface in requested mode.
	switch (flags & 3) {

	case 1:
		if (!sendCommandWait(VSCP_SERIAL_DRIVER_COMMAND_LISTEN,
								NULL,
								0,
								&Msg,
								1000)) {
			// Failure
			close();
			return CANAL_ERROR_INIT_FAIL;
		}
		break;

	case 2:
		if (!sendCommandWait(VSCP_SERIAL_DRIVER_COMMAND_LOOPBACK,
								NULL,
								0,
								&Msg,
								1000)) {
			// Failure
			close();
			return CANAL_ERROR_INIT_FAIL;
		}
		break;

	case 0:
	default:
		if (!sendCommandWait(VSCP_SERIAL_DRIVER_COMMAND_OPEN,
								NULL,
								0,
								&Msg,
								1000)) {
			// Failure
			close();
			return CANAL_ERROR_INIT_FAIL;
		}
		break;
	}

	return CANAL_ERROR_SUCCESS;
}

 
//////////////////////////////////////////////////////////////////////
// close
//

int CCan4VSCPObj::close( void )
{	
    cmdResponseMsg Msg;
	
	// Do nothing if already terminated
    if ( !m_bRun ) return CANAL_ERROR_SUCCESS;
	
    sendCommandWait( VSCP_SERIAL_DRIVER_COMMAND_CLOSE,
                                        NULL, 
                                        0, 
                                        &Msg, 
                                        1000 );
	
    m_bRun = false;
    m_bOpen = false;

	SLEEP( 1000 );	// Give working threads some time to terminate

#ifdef DEBUG_CAN4VSCP_RECEIVE
	fclose( m_flog );
#endif
 
    UNLOCK_MUTEX( m_can4vscpMutex );

    // Close the com port if its open
    if ( m_com.isOpen() ) {
        m_com.close();
    }

#ifdef WIN32    
    SetEvent( m_receiveDataEvent );
    SetEvent( m_transmitDataPutEvent );
    SetEvent( m_transmitDataGetEvent );
#else
	
#endif    
	
    // terminate the worker thread 
#ifdef WIN32	
	DWORD rv;
	
	// Wait for transmit thread to terminate
	while ( true ) {
		GetExitCodeThread( m_hTreadTransmit, &rv );
		if ( STILL_ACTIVE != rv ) break;
	}

	// Wait for receive thread to terminate
	while ( true ) {
		GetExitCodeThread( m_hTreadReceive, &rv );
		if ( STILL_ACTIVE != rv ) break;
	}			
#else
	sem_post( &m_receiveDataSem );
    sem_post( &m_transmitDataPutSem );
    sem_post( &m_transmitDataGetSem );
	
	
	int *trv;
	pthread_join( m_threadIdReceive, (void **)&trv );
	pthread_join( m_threadIdTransmit, (void **)&trv );
	pthread_mutex_destroy( &m_can4vscpMutex );
	
#endif

    return CANAL_ERROR_SUCCESS;
}


//////////////////////////////////////////////////////////////////////
// doFilter
//

bool CCan4VSCPObj::doFilter( canalMsg *pcanalMsg )
{	
	unsigned long msgid = ( pcanalMsg->id & 0x1fffffff);
	if ( !m_mask ) return true;	// fast escape

	// Set bit 32 if extended message
	if ( pcanalMsg->flags | CANAL_IDFLAG_EXTENDED ) {
		msgid &= 0x1fffffff;
		msgid |= 80000000;	
	}
	else {
		// Standard message
		msgid &= 0x000007ff;
	}

	// Set bit 31 if RTR
	if ( pcanalMsg->flags | CANAL_IDFLAG_RTR ) { 
		msgid |= 40000000;	
	}

	return !( ( m_filter ^ msgid ) & m_mask );
}

//////////////////////////////////////////////////////////////////////
// setFilter
//

int CCan4VSCPObj::setFilter( unsigned long filter )
{
	m_filter = filter;
	return CANAL_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// setMask
//

int CCan4VSCPObj::setMask( unsigned long mask )
{
	m_mask = mask;
	return CANAL_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// writeMsg
//

int CCan4VSCPObj::writeMsg( canalMsg *pMsg )
{	
	int rv = CANAL_ERROR_SUCCESS;

    // Must be a message pointer
	if ( NULL == pMsg ) {
		return CANAL_ERROR_PARAMETER;	
    }

	// Must be open
    if ( !m_bOpen ) {
		return CANAL_ERROR_NOT_OPEN;
    }

	// Must be room for the message
    if ( m_transmitList.nCount > CAN4VSCP_MAX_SNDMSG ) {
        return CANAL_ERROR_FIFO_FULL;
    }

    dllnode *pNode = new dllnode;			
	if ( NULL == pNode ) {
        return CANAL_ERROR_MEMORY;
    }
						
	canalMsg *pcanalMsg = new canalMsg;
    if ( NULL == pcanalMsg ) {		 
        delete pNode;            			
        return CANAL_ERROR_MEMORY;
    }

    pNode->pObject = pcanalMsg;
    pNode->pKey = NULL;
    pNode->pstrKey = NULL;

    pNode->pObject = pcanalMsg;
    pNode->pKey = NULL;
    pNode->pstrKey = NULL;

    memcpy( pcanalMsg, pMsg, sizeof( canalMsg ));

    LOCK_MUTEX( m_transmitMutex );
    dll_addNode( &m_transmitList, pNode );	
    UNLOCK_MUTEX( m_transmitMutex );

#ifdef WIN32    
    SetEvent( m_transmitDataGetEvent );
#else
	sem_post( &m_transmitDataGetSem );
#endif	
    
    return rv;		
}


//////////////////////////////////////////////////////////////////////
// writeMsg blocking
//

int CCan4VSCPObj::writeMsgBlocking( canalMsg *pMsg, uint32_t Timeout )
{
#ifdef WIN32    
    DWORD res;
#else 
    int res;
    struct timespec to = { 0, 0 };
    clock_gettime( CLOCK_REALTIME, &to );
    to.tv_sec += Timeout/1000;
#endif    

    // Must be a message pointer
    if ( NULL == pMsg) return CANAL_ERROR_PARAMETER;

    // Must be open
    if ( !m_bOpen ) return CANAL_ERROR_NOT_OPEN;
    
        if ( dll_getNodeCount( &m_transmitList) > CAN4VSCP_MAX_SNDMSG ) {

#ifdef WIN32            
            ResetEvent( m_transmitDataPutEvent );

		    res = WaitForSingleObject( m_transmitDataPutEvent, Timeout );

   		    if( res == WAIT_TIMEOUT ) {
			    return CANAL_ERROR_TIMEOUT;			
            }
		    else if( res == WAIT_ABANDONED ) {
			    return CANAL_ERROR_GENERIC;
            }
#else 
            res = sem_timedwait( &m_transmitDataPutSem, &to );
            if( res == EAGAIN ) {
			    return CANAL_ERROR_TIMEOUT;			
            }
		    else {
			    return CANAL_ERROR_GENERIC;
            }
#endif            
		}

        dllnode *pNode = new dllnode;			
        if ( NULL == pNode ) {
            return CANAL_ERROR_MEMORY;
        }
						
        canalMsg *pcanalMsg = new canalMsg;
        if ( NULL == pcanalMsg ) {		 
            delete pNode;            			
            return CANAL_ERROR_MEMORY;
        }

        pNode->pObject = pcanalMsg;
        pNode->pKey = NULL;
        pNode->pstrKey = NULL;

        memcpy( pcanalMsg, pMsg, sizeof( canalMsg ));

        LOCK_MUTEX( m_transmitMutex );
        dll_addNode( &m_transmitList, pNode );	
        UNLOCK_MUTEX( m_transmitMutex );
	
#ifdef WIN32		
        SetEvent( m_transmitDataGetEvent );
#else
		sem_post( &m_transmitDataGetSem );
#endif  							
	    return CANAL_ERROR_SUCCESS;
}


//////////////////////////////////////////////////////////////////////
// readMsg
//

int CCan4VSCPObj::readMsg( canalMsg *pMsg )
{
    int rv = CANAL_ERROR_SUCCESS;

    // Must be a message pointer
    if ( NULL == pMsg) {
	  return  CANAL_ERROR_PARAMETER;	
    }

    // Must be open
    if ( !m_bOpen ) {
	  return  CANAL_ERROR_NOT_OPEN;
    }

    if ( 0 == m_receiveList.nCount ) { 
      return  CANAL_ERROR_FIFO_EMPTY;
    }
	
    memcpy( pMsg, m_receiveList.pHead->pObject, sizeof( canalMsg ) );

    LOCK_MUTEX( m_receiveMutex );
    dll_removeNode( &m_receiveList, m_receiveList.pHead );
    if ( m_receiveList.nCount == 0 ) {
#ifdef WIN32        
        ResetEvent( m_receiveDataEvent );		
#else
        sem_post( &m_receiveDataSem );
#endif        
    }
    UNLOCK_MUTEX( m_receiveMutex );

    return rv;
}


//////////////////////////////////////////////////////////////////////
// readMsgBlocking
// 

int CCan4VSCPObj::readMsgBlocking( canalMsg *pMsg, uint32_t timeout )
{
    int rv = CANAL_ERROR_SUCCESS;
#ifdef WIN32    
	DWORD res;
#else
    int res;
    struct timespec to = { 0, 0 };
    clock_gettime( CLOCK_REALTIME, &to );
    to.tv_sec += timeout/1000;
#endif    

    // Must be a message pointer
    if ( NULL == pMsg ) {
        return CANAL_ERROR_PARAMETER;
    }

    // Must be open
    if ( !m_bOpen ) {
        return CANAL_ERROR_NOT_OPEN;
    }

    // Yes we block if in queue is empty
    if ( 0 == m_receiveList.nCount ) { 
#ifdef WIN32        
        res = WaitForSingleObject( m_receiveDataEvent, timeout );         	

   	    if ( res == WAIT_TIMEOUT ) {
            return CANAL_ERROR_TIMEOUT;			
        }
        else if( res == WAIT_ABANDONED ) {
            return CANAL_ERROR_GENERIC;	
        }
#else
        res = sem_timedwait( &m_receiveDataSem, &to );
        if( res == EAGAIN ) {
            return CANAL_ERROR_TIMEOUT;			
        }
#endif        
    }

    if ( m_receiveList.nCount > 0 ) { 
        LOCK_MUTEX( m_receiveMutex );
        memcpy( pMsg, m_receiveList.pHead->pObject, sizeof( canalMsg ) );	   
        dll_removeNode( &m_receiveList, m_receiveList.pHead );
        if ( 0 == m_receiveList.nCount ) {
#ifdef WIN32            
            ResetEvent( m_receiveDataEvent);
#else
            sem_post( &m_receiveDataSem );
#endif            
	    }
        UNLOCK_MUTEX( m_receiveMutex );	   	
    }
    else {
        return  CANAL_ERROR_FIFO_EMPTY;
    }
	
    return rv;
}


///////////////////////////////////////////////////////////////////////////////
// dataAvailable
//

int CCan4VSCPObj::dataAvailable( void )
{
    if ( !m_bOpen ) {
        return  0;
    }

    return m_receiveList.nCount;
}


///////////////////////////////////////////////////////////////////////////////
//	getStatistics
//

int CCan4VSCPObj::getStatistics( PCANALSTATISTICS pCanalStatistics )
{	
    // Must be a valid pointer
    if ( NULL == pCanalStatistics ) {
        return CANAL_ERROR_PARAMETER;
    }

    memcpy( pCanalStatistics, &m_stat, sizeof( canalStatistics ) );

    return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
//	getStatus
//

int CCan4VSCPObj::getStatus( PCANALSTATUS pCanalStatus )
{
    // Must be a message pointer
	if ( NULL == pCanalStatus ) {
		return CANAL_ERROR_PARAMETER;	
    }

    // Must be open
    if ( !m_bOpen ) {
		return CANAL_ERROR_NOT_OPEN;
    }

    return CANAL_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// getDeviceCapabilities
//

bool CCan4VSCPObj::getDeviceCapabilities( void )
{
    uint8_t crc = 0;
    uint8_t pos = 0;
    uint8_t sendData[ 512 ];

    sendData[ pos++ ] = DLE;
    sendData[ pos++ ] = STX;

    // Frame type
    sendData[ pos++ ] = VSCP_SERIAL_DRIVER_OPERATION_CAPS_REQUEST;
    crc8( &crc, VSCP_SERIAL_DRIVER_OPERATION_CAPS_REQUEST );

    // Channel
    sendData[ pos++ ] = 0;
    crc8( &crc, 0 );

    // Sequency number
    pos += addWithEscape( sendData + pos, m_sequencyno++, &crc );

    // Size of payload  
    pos += addWithEscape( sendData + pos, 0, &crc );
    pos += addWithEscape( sendData + pos, 2, &crc );

    // Payload: Our capabilities
    pos += addWithEscape( sendData + pos, 1, &crc );
    pos += addWithEscape( sendData + pos, 10, &crc );

    // Checksum
    pos += addWithEscape( sendData + pos, crc, NULL );

    // End of frame
    sendData[ pos++ ] = DLE;
    sendData[ pos++ ] = ETX;

    // Empty reply list
    LOCK_MUTEX( m_responseMutex );
    dll_removeAllNodes( &m_responseList );
    UNLOCK_MUTEX( m_responseMutex );

    if ( !sendMsg( sendData, pos ) ) return FALSE;

    // Wait for reply

    uint8_t saveseq = m_sequencyno;         // Save the sequency ordinal
    cmdResponseMsg msgResponse;
    uint32_t start = getClockMilliseconds();
	
    while ( getClockMilliseconds() < ( start + 500 ) ) {

        if ( ( NULL != m_responseList.pHead ) &&
             ( NULL != m_responseList.pHead->pObject ) ) {

            memcpy( &msgResponse, m_responseList.pHead->pObject, sizeof( cmdResponseMsg ) );
            LOCK_MUTEX( m_responseMutex );
            dll_removeNode( &m_responseList, m_responseList.pHead );
            UNLOCK_MUTEX( m_responseMutex );

            if ( ( VSCP_SERIAL_DRIVER_CAPS_SIZE == msgResponse.sizePayload ) &&
                 ( saveseq == msgResponse.seq ) &&
                 ( VSCP_SERIAL_DRIVER_OPERATION_CAPS_RESPONSE == msgResponse.op )  ) {
                
                m_caps.maxVscpFrames = (((uint16_t)msgResponse.payload[ 0 ]) << 8) + msgResponse.payload[ 1 ];
                if ( !m_caps.maxVscpFrames ) m_caps.maxVscpFrames = 1;      // Zero not allowed

                m_caps.maxCanalFrames = ((( uint16_t )msgResponse.payload[ 2 ]) << 8) + msgResponse.payload[ 3 ];
                if ( !m_caps.maxCanalFrames ) m_caps.maxCanalFrames = 1;    // Zero not allowed
            }

        }

        SLEEP( 1 );
		
    }

    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// sendMsg
//

bool CCan4VSCPObj::sendMsg( uint8_t *buffer, short size ) 
{
	uint8_t flags = 0;
	bool rv;
	
	if ( m_com.isOpen() ) {
		
		LOCK_MUTEX( m_can4vscpMutex );
#ifdef WIN32        
		if ( m_com.writebuf( buffer, size ) ) {             
			rv = true;
		}
		else {
			rv = false;
		}
#else   
        if ( m_com.comm_puts( (char *)buffer, size ) ) {             
			rv = true;
		}
		else {
			rv = false;
		}    
#endif         
		UNLOCK_MUTEX( m_can4vscpMutex );
	}
	else {
		rv = false;
	}
	
	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// sendCommand
//
//

bool CCan4VSCPObj::sendCommand( uint8_t cmdcode, uint8_t *pParam, uint8_t size )
{
	uint8_t crc = 0;
	uint8_t pos = 0;
	uint8_t sendData[ 512 ];

	sendData[ pos++ ] = DLE;
	sendData[ pos++ ] = STX;
			
	// Frame type
    sendData[ pos++ ] = VSCP_SERIAL_DRIVER_OPERATION_COMMAND;
    crc8( &crc, VSCP_SERIAL_DRIVER_OPERATION_COMMAND );
			
	// Channel
	sendData[ pos++ ] = 0;
	crc8( &crc, 0 );
			
	// Sequency number
	pos += addWithEscape( sendData + pos, m_sequencyno++, &crc ); 
			
	// Size of payload  
    sendData[ pos++ ] = 0;
	crc8( &crc, 0 ); 
    pos += addWithEscape( sendData + pos, (size&0xff), &crc );

    // Command code
    pos += addWithEscape( sendData + pos, cmdcode, &crc );

    if ( size ) {
        for ( int i=0; i<size; i++ ) {
            pos += addWithEscape( sendData + pos, pParam[i], &crc );
        }
    }

    // Checksum
    pos += addWithEscape( sendData + pos, crc, NULL ); 

    // End of frame
    sendData[ pos++ ] = DLE;
	sendData[ pos++ ] = ETX;
	
    // Empty reply list
    LOCK_MUTEX( m_responseMutex );
    dll_removeAllNodes( &m_responseList );
	UNLOCK_MUTEX( m_responseMutex );

	return sendMsg( sendData, pos );
}


///////////////////////////////////////////////////////////////////////////////
// waitCommandResponse
//
//

bool CCan4VSCPObj::waitCommandResponse( cmdResponseMsg *pMsg, uint8_t cmdcode, uint8_t saveseq, uint32_t timeout )
{
	//uint32_t start = GetTickCount();
    uint32_t start = getClockMilliseconds();

	while (  getClockMilliseconds() < ( start + timeout ) ) {
	
		if ( ( NULL != m_responseList.pHead ) && 
				( NULL != m_responseList.pHead->pObject ) ) {

			memcpy( pMsg, m_responseList.pHead->pObject, sizeof( cmdResponseMsg ) ); 
			LOCK_MUTEX( m_responseMutex );
			dll_removeNode( &m_responseList, m_responseList.pHead );
			UNLOCK_MUTEX( m_responseMutex );

            if ( (2 == pMsg->sizePayload) &&  
                        ( saveseq == pMsg->seq ) && 
                        ( VSCP_SERIAL_DRIVER_OPERATION_COMMAND_REPLY == pMsg->op ) &&
                        ( 0 == pMsg->payload[ 0 ] ) &&
                        ( cmdcode == pMsg->payload[ 1 ] ) ) {
                        return true;
            }
	
		}
		
        SLEEP( 10 );
	}

	return false;
}


///////////////////////////////////////////////////////////////////////////////
// sendCommandWait
//

bool CCan4VSCPObj::sendCommandWait( uint8_t cmdcode, 
                                        uint8_t *pParam, 
                                        uint8_t size, 
                                        cmdResponseMsg *pMsg, 
                                        uint32_t timeout )
{
    // Save sequence number
    uint8_t saveseq = m_sequencyno;

	// Send the command
	if ( !sendCommand( cmdcode, pParam, size ) ) return false;	

	return waitCommandResponse( pMsg, cmdcode, saveseq, timeout );
}

///////////////////////////////////////////////////////////////////////////////
// checkCRC
//

bool CCan4VSCPObj::checkCRC( void )
{
	uint8_t crc = 0;

	for ( int i=0; i<m_lengthMsgRcv-1; i++ ) {
		crc8( &crc, m_bufferMsgRcv[ i ] );
	}

	return ( crc == m_bufferMsgRcv[ m_lengthMsgRcv-1 ] );
}

///////////////////////////////////////////////////////////////////////////////
// sendACK
//
//

void CCan4VSCPObj::sendACK( uint8_t seq )
{
	uint8_t crc = 0;
	uint8_t pos = 0;
	uint8_t sendData[ 10 ];  // No Level II events in this driver

	sendData[ pos++ ] = DLE;
	sendData[ pos++ ] = STX;
			
	// Frame type
    sendData[ pos++ ] = VSCP_SERIAL_DRIVER_OPERATION_ACK;
    crc8( &crc, VSCP_SERIAL_DRIVER_OPERATION_ACK );
			
	// Channel
	sendData[ pos++ ] = 0;
	crc8( &crc, 0 );
			
	// Sequency number
	pos += addWithEscape( sendData + pos, seq, &crc ); 
			
	// Size of payload  
	sendData[ pos++ ] = 0;
	crc8( &crc, 0 ); 
	sendData[ pos++ ] = 0;
	crc8( &crc, 0 ); 

    // Checksum
    pos += addWithEscape( sendData + pos, crc, NULL ); 

    // End of frame
    sendData[ pos++ ] = DLE;
	sendData[ pos++ ] = ETX;

	LOCK_MUTEX( m_can4vscpMutex );

	// Send the event
	sendMsg( sendData, pos );

	UNLOCK_MUTEX( m_can4vscpMutex );
}


///////////////////////////////////////////////////////////////////////////////
// sendNACK
//
//

void CCan4VSCPObj::sendNACK( uint8_t seq )
{
	uint8_t crc = 0;
	uint8_t pos = 0;
	uint8_t sendData[ 10 ];  // No Level II events in this driver

	sendData[ pos++ ] = DLE;
	sendData[ pos++ ] = STX;
			
	// Frame type
    sendData[ pos++ ] = VSCP_SERIAL_DRIVER_OPERATION_NACK;
    crc8( &crc, VSCP_SERIAL_DRIVER_OPERATION_NACK );
			
	// Channel
	sendData[ pos++ ] = 0;
	crc8( &crc, 0 );
			
	// Sequency number
	pos += addWithEscape( sendData + pos, seq, &crc ); 
			
	// Size of payload  
	sendData[ pos++ ] = 0;
	crc8( &crc, 0 ); 
	sendData[ pos++ ] = 0;
	crc8( &crc, 0 ); 

    // Checksum
    pos += addWithEscape( sendData + pos, crc, NULL ); 

    // End of frame
    sendData[ pos++ ] = DLE;
	sendData[ pos++ ] = ETX;

	LOCK_MUTEX( m_can4vscpMutex );

	// Send the event
	sendMsg( sendData, pos );

	UNLOCK_MUTEX( m_can4vscpMutex );
}

///////////////////////////////////////////////////////////////////////////////
// sendNoopFrame
//
//

void CCan4VSCPObj::sendNoopFrame( void )
{
    uint8_t crc = 0;
	uint8_t pos = 0;
	uint8_t sendData[ 10 ];  // No Level II events in this driver

	sendData[ pos++ ] = DLE;
	sendData[ pos++ ] = STX;
			
	// Frame type
    sendData[ pos++ ] = VSCP_SERIAL_DRIVER_OPERATION_NOOP;
    crc8( &crc, VSCP_SERIAL_DRIVER_OPERATION_NOOP );
			
	// Channel
	sendData[ pos++ ] = 0;
	crc8( &crc, 0 );
			
	// Sequency number
	pos += addWithEscape( sendData + pos, m_sequencyno++, &crc ); 
			
	// Size of payload  
	sendData[ pos++ ] = 0;
	crc8( &crc, 0 ); 
	sendData[ pos++ ] = 0;
	crc8( &crc, 0 ); 

    // Checksum
    pos += addWithEscape( sendData + pos, crc, NULL ); 

    // End of frame
    sendData[ pos++ ] = DLE;
	sendData[ pos++ ] = ETX;

	LOCK_MUTEX( m_can4vscpMutex );

	// Send the event
	sendMsg( sendData, pos );

	UNLOCK_MUTEX( m_can4vscpMutex );
}

///////////////////////////////////////////////////////////////////////////////
// addToResponseQueue
//
//

bool CCan4VSCPObj::addToResponseQueue( void ) 
{
    cmdResponseMsg *pMsg = new cmdResponseMsg;
    if ( NULL != pMsg ) {
								
        dllnode *pNode = new dllnode; 
	    if ( NULL != pNode ) {

            pMsg->op = m_bufferMsgRcv[ 0 ];
            pMsg->channel = m_bufferMsgRcv[ 1 ];
            pMsg->seq = m_bufferMsgRcv[ 2 ];
            pMsg->sizePayload = ( (uint16_t)m_bufferMsgRcv[ 3 ] << 8 ) + m_bufferMsgRcv[ 4 ];

            if ( pMsg->sizePayload ) {
                memcpy( pMsg->payload, 
                            m_bufferMsgRcv + 5, 
                            ( pMsg->sizePayload > 512 ) ? 512 : pMsg->sizePayload );
            }

            pNode->pObject = pMsg;
            LOCK_MUTEX( m_responseMutex );
            dll_addNode( &m_responseList, pNode );
            UNLOCK_MUTEX( m_responseMutex );

        }
        else {									
            delete pMsg;
            return false;
        }
	}
    else {
        return false;
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// ReadSerialData
//
//

bool CCan4VSCPObj::serialData2StateMachine(void)
{
	uint8_t c; // Serial character
	int cnt = 0;
#ifdef DEBUG_CAN4VSCP_RECEIVE
	char dbgbuf[20];
#endif

	// Read RS-232 data
	c = m_com.readChar(&cnt);
#ifdef DEBUG_CAN4VSCP_RECEIVE
	if (cnt>0) {
		sprintf(dbgbuf, "cnt=%02X \n", c);
		fwrite(dbgbuf, 1, strlen(dbgbuf), m_flog);
	}
#endif

	while ( cnt > 0 ) { // Linux sets cnt ==-1 if no data

		switch (m_RxMsgState) {

			// We are virgin - no package start received yet
		case INCOMING_STATE_NONE:

			if ((INCOMING_SUBSTATE_NONE == m_RxMsgSubState) && (DLE == c)) {
#ifdef DEBUG_CAN4VSCP_RECEIVE
				fprintf(m_flog, " STATE_NONE/SUBSATE_DLE \n");
#endif
				m_RxMsgSubState = INCOMING_SUBSTATE_DLE;
			}
			else if ((INCOMING_SUBSTATE_DLE == m_RxMsgSubState) && (STX == c)) {
#ifdef DEBUG_CAN4VSCP_RECEIVE
				fprintf(m_flog, " STATE_STX/SUBSATE_NONE ");
#endif
				m_RxMsgState = INCOMING_STATE_STX;
				m_RxMsgSubState = INCOMING_SUBSTATE_NONE;
				m_lengthMsgRcv = 0;
			}
			else {
#ifdef DEBUG_CAN4VSCP_RECEIVE
				fprintf(m_flog, " STATE_NONE/SUBSATE_NONE \n");
#endif
				m_lengthMsgRcv = 0;
				m_RxMsgState = INCOMING_STATE_NONE;
				m_RxMsgSubState = INCOMING_SUBSTATE_NONE;
			}
			break;

			// We have received package start but not end		
		case INCOMING_STATE_STX:

			if ((INCOMING_SUBSTATE_NONE == m_RxMsgSubState) && (DLE == c)) {
#ifdef DEBUG_CAN4VSCP_RECEIVE
				fprintf(m_flog, " STATE_STX/SUBSTATE_DLE \n");
#endif
				m_RxMsgSubState = INCOMING_SUBSTATE_DLE;
			}
			else if ((INCOMING_SUBSTATE_DLE == m_RxMsgSubState) && (STX == c)) {
				// This is strange as a DEL STX is not expected here
				// We try to sync up again...
#ifdef DEBUG_CAN4VSCP_RECEIVE
				fprintf(m_flog, " STATE_STX/SUBSTATE_NONE \n");
#endif
				m_RxMsgState = INCOMING_STATE_STX;
				m_RxMsgSubState = INCOMING_SUBSTATE_NONE;
				m_lengthMsgRcv = 0;
			}
			else if ((INCOMING_SUBSTATE_DLE == m_RxMsgSubState) && (ETX == c)) {

				// We have a packet 
#ifdef DEBUG_CAN4VSCP_RECEIVE
				fprintf(m_flog, " STATE_NONE/SUBSTATE_NONE\n");
#endif
				m_RxMsgState = INCOMING_STATE_NONE;
				m_RxMsgSubState = INCOMING_SUBSTATE_NONE;
#ifdef DEBUG_CAN4VSCP_RECEIVE
				fprintf(m_flog, " ***FRAME*** \n");
#endif
				return true;

			}
			else if ((INCOMING_SUBSTATE_DLE == m_RxMsgSubState) && (DLE == c)) {
				// Byte stuffed DLE  i.e. DLE DLE == DLE
#ifdef DEBUG_CAN4VSCP_RECEIVE
				fprintf(m_flog, " STATE_STX/SUBSTATE_NONE\n");
#endif
				m_RxMsgSubState = INCOMING_SUBSTATE_NONE;
				if (m_lengthMsgRcv < sizeof( m_bufferMsgRcv)) {
					m_bufferMsgRcv[ m_lengthMsgRcv++ ] = c;
				}
				else {
					// This packet has wrong format as it have
					// to many databytes - start over!
#ifdef DEBUG_CAN4VSCP_RECEIVE
					fprintf(m_flog, " STATE_NONE/SUBSTATE_NONE\n");
#endif
					m_lengthMsgRcv = 0;
					m_RxMsgState = INCOMING_STATE_NONE;
					m_RxMsgSubState = INCOMING_SUBSTATE_NONE;
				}
			} // We come here if data is received 
			else {
#ifdef DEBUG_CAN4VSCP_RECEIVE
				fprintf(m_flog, " STATE_STX/SUBSTATE_NONE\n ");
#endif					
				m_RxMsgSubState = INCOMING_SUBSTATE_NONE;
				if (m_lengthMsgRcv < sizeof( m_bufferMsgRcv)) {
					m_bufferMsgRcv[ m_lengthMsgRcv++ ] = c;
				}
				else {
					// This packet has wrong format as it have
					// to many databytes - start over!
					m_lengthMsgRcv = 0;
#ifdef DEBUG_CAN4VSCP_RECEIVE
					fprintf(m_flog, " STATE_NONE/SUBSTATE_NONE\n ");
#endif
					m_RxMsgState = INCOMING_STATE_NONE;
					m_RxMsgSubState = INCOMING_SUBSTATE_NONE;
				}
			}
			break;

		case INCOMING_STATE_ETX:
			break;

		}

		// Read RS-232 data
		c = m_com.readChar(&cnt);
#ifdef DEBUG_CAN4VSCP_RECEIVE
		if (cnt) {
			sprintf(dbgbuf, "Rcv=%02X\n", c);
			fwrite(dbgbuf, 1, strlen(dbgbuf), m_flog);
		}
#endif

	} // while

	return false;
}


///////////////////////////////////////////////////////////////////////////////
// readSerialData
//
//

void CCan4VSCPObj::readSerialData( void )
{
    int cnt = 0;
	
    do {

        if ( serialData2StateMachine() ) {
		
#ifdef DEBUG_CAN4VSCP_RECEIVE
            fprintf( m_flog, " OPERATION\n" );
            fprintf( m_flog, 
                        " Operation=%d payload=%d\n", 
                        m_bufferMsgRcv[ 0 ], 
                        m_bufferMsgRcv[ 3 ] * 256 + m_bufferMsgRcv[ 4 ] );
            for ( int g = 0; g<( m_bufferMsgRcv[ 3 ] * 256 + m_bufferMsgRcv[ 4 ] ); g++ ) {
                fprintf( m_flog, " %02X", m_bufferMsgRcv[ 5 + g ] );
            }
            fprintf( m_flog, "\n" );      
#endif

            // Check CRC
            if ( !checkCRC() ) {
#ifdef DEBUG_CAN4VSCP_RECEIVE
                fprintf( m_flog, " CRC Failed!\n" );
                fprintf( m_flog,
                         " Operation=%d payload=%d\n",
                         m_bufferMsgRcv[ 0 ],
                         m_bufferMsgRcv[ 3 ] * 256 + m_bufferMsgRcv[ 4 ] );
                for ( int g = 0; g<( m_bufferMsgRcv[ 3 ] * 256 + m_bufferMsgRcv[ 4 ] ); g++ ) {
                    fprintf( m_flog, " %02X", m_bufferMsgRcv[ 5 + g ] );
                }
                fprintf( m_flog, "\n" );
#endif
                continue;
            }

            // Check if NOOP frame
            if ( VSCP_SERIAL_DRIVER_OPERATION_NOOP == ( m_bufferMsgRcv[ 0 ] ) ) {
                sendACK( m_bufferMsgRcv[ VSCP_SERIAL_DRIVER_POS_FRAME_SEQUENCY ] );
			}
            // Check for CANAL message frame
            else if ( VSCP_SERIAL_DRIVER_OPERATION_CANAL == ( m_bufferMsgRcv[ 0 ] ) ) {
			
                // CANAL message
                // -------------
                // [0]      -   DLE  - Not in buffer!!!!
                // [1]      -   STX  - Not in buffer!!!!
                // [2]      0   Frame type (2 - CANAL message.) - First in buffer
                // [3]      1   Channel (always zero)
                // [4]      2   Sequence number 
                // [5/6]    3/4   Size of payload ( 12 + sizeData )
                // [7]      5   CAN id (MSB)
                // [8]      6   CAN id
                // [9]      7   CAN id
                // [10]     8   CAN id (LSB)
                // [11]     9   dlc
                // [12-n]   10  CAN data (0-8 bytes) 
                // [len-3]  -   CRC
                // [len-2]  -   DLE
                // [len-1]  -   ETX

                if (  m_receiveList.nCount < CAN4VSCP_MAX_RCVMSG ) {					
					
                    PCANALMSG pMsg	= new canalMsg;

                    if ( NULL != pMsg ) {
                    
                        pMsg->flags = 0;
                        dllnode *pNode = new dllnode; 
                        if ( NULL != pNode ) {
							
                            pMsg->flags = 0;
                            pMsg->timestamp = getClockMilliseconds();
                            pMsg->obid = 0;
				
                            pMsg->id = 
                                (((uint32_t)m_bufferMsgRcv[5]<<24) & 0x1f000000) |
                                (((uint32_t)m_bufferMsgRcv[6]<<16) & 0x00ff0000) |
                                (((uint32_t)m_bufferMsgRcv[7]<<8 ) & 0x0000ff00) |
                                (((uint32_t)m_bufferMsgRcv[8]    ) & 0x000000ff) ;
											
                            pMsg->sizeData = m_bufferMsgRcv[9];	
                            if ( pMsg->sizeData > 8 ) pMsg->sizeData = 8;   // Something is very wrong - Save the world
									
                            if ( pMsg->sizeData ) {
                                memcpy( ( void * )pMsg->data,
                                        ( m_bufferMsgRcv + 10 ),
                                        pMsg->sizeData );
                            }
									
                            // Always extended so set extended flag
                            pMsg->flags |= CANAL_IDFLAG_EXTENDED;
							
                            if ( doFilter( pMsg ) ) {
                                pNode->pObject = pMsg;
                                LOCK_MUTEX( m_receiveMutex );
                                dll_addNode( &m_receiveList, pNode );
#ifdef WIN32                                
                                SetEvent( m_receiveDataEvent ); // Signal frame in queue
#else
                                sem_post( &m_receiveDataSem );  // Signal frame in queue
#endif                                
                                UNLOCK_MUTEX( m_receiveMutex );

                                // Update statistics
                                m_stat.cntReceiveData += pMsg->sizeData;
                                m_stat.cntReceiveFrames += 1;
                            }
                            else {
                                // Message was filtered
                                delete pMsg;
                                delete pNode;
                            }

                        } // No pNode
                        else {
                            delete pMsg;
                        }
                        
                    }  // No pMsg 
                }
                // No room in receive queue
                else {
                    // Full buffer
                    m_stat.cntOverruns++;
                }
            }
            // Check for VSCP event frame
            else if ( VSCP_SERIAL_DRIVER_OPERATION_VSCP_EVENT == ( m_bufferMsgRcv[ 0 ] ) ) {
                sendNACK( m_bufferMsgRcv[ VSCP_SERIAL_DRIVER_POS_FRAME_SEQUENCY ] );	// We don't handle VSCP events
            }
            // Check for configure frame
            else if ( VSCP_SERIAL_DRIVER_OPERATION_CONFIGURE == ( m_bufferMsgRcv[ 0 ] ) ) {
                sendNACK( m_bufferMsgRcv[ VSCP_SERIAL_DRIVER_POS_FRAME_SEQUENCY ] );	// We don't handle configure
            }
            // Check for poll frame
            else if ( VSCP_SERIAL_DRIVER_OPERATION_POLL == ( m_bufferMsgRcv[ 0 ] ) ) {
                sendNACK( m_bufferMsgRcv[ VSCP_SERIAL_DRIVER_POS_FRAME_SEQUENCY ] );	// We don't handle poll
            }
            // Check for no event frame
            else if ( VSCP_SERIAL_DRIVER_OPERATION_NO_EVENT == ( m_bufferMsgRcv[ 0 ] ) ) {
                sendNACK( m_bufferMsgRcv[ VSCP_SERIAL_DRIVER_POS_FRAME_SEQUENCY ] );	// We don't handle no event
            }
            // Check for multiframe VSCP message frame
            else if ( VSCP_SERIAL_DRIVER_OPERATION_MULTI_FRAME_VSCP == ( m_bufferMsgRcv[ 0 ] ) ) {
                sendNACK( m_bufferMsgRcv[ VSCP_SERIAL_DRIVER_POS_FRAME_SEQUENCY ] );	// We don't handle vscp multi frame
            }
            // Check for multiframe CANAL message frame
            else if ( VSCP_SERIAL_DRIVER_OPERATION_MULTI_FRAME_CANAL == ( m_bufferMsgRcv[ 0 ] ) ) {

                // CANAL message
                // -------------
                // [0]      DLE  - Not in buffer!!!!
                // [1]      STX  - Not in bugger!!!!
                // [2]      Frame type (2 - CANAL message.) - First in buffer
                // [3]      Channel (always zero)
                // [4]      Sequence number 
                // [5/6]    Size of payload ( 5 + sizeData ) * number of frames
                // ---------------------------------------------
                // [7]      CAN id (MSB)
                // [8]      CAN id
                // [9]      CAN id
                // [10]     CAN id (LSB)
                // [11]     dlc
                // [11-n]   CAN data (0-8 bytes) 
                // ---------------------------------------------
                // [n+1]    Possible other CANAL frame
                // ---------------------------------------------
                // [len-3]  CRC
                // [len-2]  DLE
                // [len-1]  ETX

                // Payload size
                long sizePayload = ( ( uint16_t )m_bufferMsgRcv[ 3 ] << 8 ) + m_bufferMsgRcv[ 4 ];

                // Save pos for payload
                uint8_t *posFrame = m_bufferMsgRcv + 5;

                while ( sizePayload > 0 ) {

                    if ( m_receiveList.nCount < CAN4VSCP_MAX_RCVMSG ) {

                        PCANALMSG pMsg = new canalMsg;

                        if ( NULL != pMsg ) {

                            pMsg->flags = 0;
                            dllnode *pNode = new dllnode;
                            if ( NULL != pNode ) {

                                pMsg->flags = 0;
                                pMsg->timestamp = getClockMilliseconds();
                                pMsg->obid = 0;

                                pMsg->id =
                                    ( ( ( uint32_t )posFrame[ 0 ] << 24 ) & 0x1f000000 ) |
                                    ( ( ( uint32_t )posFrame[ 1 ] << 16 ) & 0x00ff0000 ) |
                                    ( ( ( uint32_t )posFrame[ 2 ] << 8 ) & 0x0000ff00 ) |
                                    ( ( ( uint32_t )posFrame[ 3 ] ) & 0x000000ff );

                                pMsg->sizeData = posFrame[ 4 ];
                                if ( pMsg->sizeData > 8 ) pMsg->sizeData = 8;   // Something is very wrong - Save the world

                                if ( pMsg->sizeData ) {
                                    memcpy( ( void * )pMsg->data,
                                            ( posFrame + 5 ),
                                            pMsg->sizeData );
                                }

                                // Always extended so set extended flag
                                pMsg->flags |= CANAL_IDFLAG_EXTENDED;

                                if ( doFilter( pMsg ) ) {
                                    pNode->pObject = pMsg;
                                    LOCK_MUTEX( m_receiveMutex );
                                    dll_addNode( &m_receiveList, pNode );
#ifdef WIN32                                    
                                    SetEvent( m_receiveDataEvent ); // Signal frame in queue
#else
                                    sem_post( &m_receiveDataSem );   // Signal frame in queue    
#endif                                    
                                    UNLOCK_MUTEX( m_receiveMutex );

                                    // Update statistics
                                    m_stat.cntReceiveData += pMsg->sizeData;
                                    m_stat.cntReceiveFrames += 1;
                                }
                                else {
                                    // Message was filtered
                                    delete pMsg;
                                    delete pNode;
                                }

                                // Get ready for next payload frame
                                sizePayload -= ( 5 + posFrame[ 4 ] );
                                posFrame += ( 5 + posFrame[ 4 ] );
                                

                            } // No pNode
                            else {
                                delete pMsg;
                            }

                        }  // No pMsg 
                    } 
                    // No room in receive queue
                    else {
                        // Full buffer
                        m_stat.cntOverruns++;
                    }
                } // while
            }
			// Check for sent ACK frame
            else if ( VSCP_SERIAL_DRIVER_OPERATION_SENT_ACK == ( m_bufferMsgRcv[ 0 ] ) ) {
				addToResponseQueue();
			}
			// Check for sent NACK frame
            else if ( VSCP_SERIAL_DRIVER_OPERATION_SENT_NACK == ( m_bufferMsgRcv[ 0 ] ) ) {
				addToResponseQueue();
			}
			// Check for ACK frame
            else if ( VSCP_SERIAL_DRIVER_OPERATION_ACK == ( m_bufferMsgRcv[ 0 ] ) ) {
			    addToResponseQueue();
			}
			// Check for NACK frame
            else if ( VSCP_SERIAL_DRIVER_OPERATION_NACK == ( m_bufferMsgRcv[ 0 ] ) ) {
			    addToResponseQueue();
			}
			// Check for ERROR frame
            else if ( VSCP_SERIAL_DRIVER_OPERATION_ERROR == ( m_bufferMsgRcv[ 0 ] ) ) {
			    addToResponseQueue();
			}
			// Check for command reply frame
            else if ( VSCP_SERIAL_DRIVER_OPERATION_COMMAND_REPLY == ( m_bufferMsgRcv[ 0 ] ) ) {
			    addToResponseQueue();
			}
			// Check if command frame
            else if ( VSCP_SERIAL_DRIVER_OPERATION_COMMAND == ( m_bufferMsgRcv[ 0 ] ) ) {
                addToResponseQueue();
			} 

			m_RxMsgState = INCOMING_STATE_NONE;			// reset state for next msg
			m_RxMsgSubState = INCOMING_SUBSTATE_NONE;

		} // frame & crc 

	} while ( cnt != 0 );							
							
}




///////////////////////////////////////////////////////////////////////////////
// workThreadTransmit
//
//

#ifdef WIN32
void workThreadTransmit( void *pObject )
#else
void *workThreadTransmit( void *pObject )
#endif
{
#ifdef WIN32
	DWORD errorCode = 0;
#else
	int rv = 0;
    int res;
    struct timespec to = { 0, 0 };
    clock_gettime( CLOCK_REALTIME, &to );
    to.tv_nsec +=  500000;
#endif
	uint8_t sequencyno = 0;

	CCan4VSCPObj * pobj = ( CCan4VSCPObj *)pObject;
	if ( NULL == pobj ) {
#ifdef WIN32	
		ExitThread( errorCode ); // Fail
#else
		pthread_exit( &rv );
#endif
	}

	// Init CRC table
	init_crc8();
	
	while ( pobj->m_bRun ) {
		
		// Noting to do if we should end...
		if ( !pobj->m_bRun ) continue;

        if ( 0 == pobj->m_transmitList.nCount ) {
#ifdef WIN32            
            ResetEvent( pobj->m_transmitDataGetEvent );
            if ( WAIT_OBJECT_0 != WaitForSingleObject( pobj->m_transmitDataGetEvent, 100 ) ) {		 
                continue;
            }
#else 
            res = sem_timedwait( &pobj->m_transmitDataGetSem, &to );
            if ( res == EAGAIN ) {
			    continue;			
            }
#endif            
        }

		while( pobj->m_transmitList.nCount > 0 ) {

            canalMsg msg;
            memcpy( &msg, pobj->m_transmitList.pHead->pObject, sizeof( canalMsg ) ); 
			LOCK_MUTEX( pobj->m_transmitMutex );
			dll_removeNode( &pobj->m_transmitList, pobj->m_transmitList.pHead );
			UNLOCK_MUTEX( pobj->m_transmitMutex );

            // CANAL message
            // -------------
            // [0]      DLE
            // [1]      STX
            // [2]      Frame type (2 - CANAL message.)
            // [3]      Channel (always zero)
            // [4]      Sequence number 
            // [5/6]    Size of payload ( 12 + sizeData )
            // [7]     CAN id (MSB)
            // [8]     CAN id
            // [9]     CAN id
            // [10]     CAN id (LSB)
            // [11-n]   CAN data (0-8 bytes) 
            // [len-3]  CRC
            // [len-2]  DLE
            // [len-1]  ETX

			uint8_t crc = 0;
			uint8_t pos = 0;
			uint8_t sendData[ 128 ];  // No Level II events in this driver

			sendData[ pos++ ] = DLE;
			sendData[ pos++ ] = STX;
			
			// Frame type
            sendData[ pos++ ] = VSCP_SERIAL_DRIVER_OPERATION_CANAL;
            crc8( &crc, VSCP_SERIAL_DRIVER_OPERATION_CANAL );
			
			// Channel
			sendData[ pos++ ] = 0;
			crc8( &crc, 0 );
			
			// Sequency number
            pos += addWithEscape( sendData + pos, pobj->m_sequencyno++, &crc ); 
			
			// Size of payload  6 + datalen
			sendData[ pos++ ] = 0;
			crc8( &crc, 0 );
			pos += addWithEscape( sendData + pos, 4 + msg.sizeData, &crc  );

            // id
            pos += addWithEscape( sendData + pos, (msg.id>>24) & 0xff, &crc );
            pos += addWithEscape( sendData + pos, (msg.id>>16) & 0xff, &crc );
            pos += addWithEscape( sendData + pos, (msg.id>>8) & 0xff, &crc );
            pos += addWithEscape( sendData + pos, msg.id & 0xff, &crc );

			// Data
			for ( int i=0; i<msg.sizeData; i++ ) {
				pos += addWithEscape( sendData + pos, msg.data[i], &crc  ); 
			}

			// Checksum
			pos += addWithEscape( sendData + pos, crc, NULL );

            // End of frame
            sendData[ pos++ ] = DLE;
			sendData[ pos++ ] = ETX;

			//LOCK_MUTEX( pobj->m_can4vscpMutex );

			// Send the event
			if ( pobj->sendMsg( sendData, pos ) ) {
	
				// Message sent successfully
				// Update statistics
				pobj->m_stat.cntTransmitData += msg.sizeData;
				pobj->m_stat.cntTransmitFrames += 1;

			}
			else {
					
				// Failed - put message back in queue front
				PCANALMSG pMsg	= new canalMsg;
				if ( NULL != pMsg ) {
						
					// Copy in data
					memcpy ( pMsg, &msg, sizeof( canalMsg ) );

					dllnode *pNode = new dllnode; 
					if ( NULL != pNode ) {
																
						pNode->pObject = pMsg;
						LOCK_MUTEX( pobj->m_transmitMutex );
						dll_addNodeHead( &pobj->m_transmitList, pNode );
						UNLOCK_MUTEX( pobj->m_transmitMutex );

					}
					else {
						delete pMsg;
					}

				}

			}

			//UNLOCK_MUTEX( pobj->m_can4vscpMutex );
										
		} // if event to send


		// No data to write
		SLEEP( 100 );

	
	} // while 	 


#ifdef WIN32
	ExitThread( errorCode );
#else
	pthread_exit( &rv );
#endif

}


///////////////////////////////////////////////////////////////////////////////
// workThreadReceive
//
//

#ifdef WIN32
void workThreadReceive( void *pObject )
#else
void *workThreadReceive( void *pObject )
#endif
{
#ifdef WIN32
    DWORD errorCode = 0;
#else
    int rv = 0;
#endif

    CCan4VSCPObj * pobj = ( CCan4VSCPObj *)pObject;
    if ( NULL == pobj ) {
#ifdef WIN32	
        ExitThread( errorCode ); // Fail
#else
        pthread_exit( &rv );
#endif
    }
	
    while ( pobj->m_bRun ) {
		
        LOCK_MUTEX( pobj->m_can4vscpMutex );
        pobj->readSerialData();
        UNLOCK_MUTEX( pobj->m_can4vscpMutex );
	
		SLEEP( 100 );
		
    } // while 	 


#ifdef WIN32
    ExitThread( errorCode );
#else
    pthread_exit( &rv );
#endif

}
