// can4vscpobj.cpp:  
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2014 
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
// Boston, MA 02111-1307, USA.
//
// $RCSfile: can4vscpobj.cpp,v $                                       
// $Date: 2005/05/29 19:20:08 $                                  
// $Author: akhe $                                              
// $Revision: 1.5 $ 

#include "stdio.h"
#include "can4vscpobj.h"
#include "dlldrvobj.h"
#include "callback.h"


// Prototypes
#ifdef WIN32
void workThreadTransmit( void *pObject );
void workThreadReceive( void *pObject );
#else
void *workThreadTransmit( void *pObject );
void *workThreadReceive( void *pObject );
#endif

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

    m_bRun = false;

    m_RxMsgState = INCOMING_STATE_NONE;
    m_RxMsgSubState = INCOMING_SUBSTATE_NONE;

#ifdef WIN32
	
    m_hTreadReceive = 0;
    m_hTreadTransmit = 0;

	// Create the device AND LIST access mutexes
    m_can4vscpMutex = CreateMutex( NULL, true, CANAL_DLL_CAN4VSCPDRV_OBJ_MUTEX );
    m_receiveMutex = CreateMutex( NULL, true, CANAL_DLL_CAN4VSCPDRV_RECEIVE_MUTEX );
    m_transmitMutex = CreateMutex( NULL, true, CANAL_DLL_CAN4VSCPDRV_TRANSMIT_MUTEX );
    m_responseMutex = CreateMutex( NULL, true, CANAL_DLL_CAN4VSCPDRV_RESPONSE_MUTEX );

#else
	
    m_flog = NULL;
    pthread_mutex_init( &m_can4vscpMutex, NULL );
    pthread_mutex_init( &m_receiveMutex, NULL );
    pthread_mutex_init( &m_transmitMutex, NULL );
    pthread_mutex_init( &m_responseMutex, NULL );

#endif

    dll_init( &m_transmitList, SORT_NONE );
    dll_init( &m_receiveList, SORT_NONE );
    dll_init( &m_responseList, SORT_NONE );
	
}

//////////////////////////////////////////////////////////////////////
// ~CCan4VSCPObj
//

CCan4VSCPObj::~CCan4VSCPObj()
{		 
	close();
	
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

#else

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
// filename
//-----------------------------------------------------------------------------
// Parameters for the driver as a string on the following form
//		
// "comport;baudrate"
//
//
// comport
// =======
//	WIN32: 1 for COM1, 2 for COM2 etc
//	LINUX: /dev/ttyS1, /dev/ttyS2 etc
//
// baudrate
// ========
// Actual baudrate to use (now only 57600)
// 
// flags (!!!! Not used at the moment, left for future expansion !!!!!
//-----------------------------------------------------------------------------
//
// bit 0
// =====
//  0		11 bit identifiers are not received.
//  1		11 bit identifiers are received.
//
// bit 1
// =====
//	0		29 bit identifiers are not received.
//	1		29 bit identifiers are received.
//
// Both bits 00 is the same as 11 i.e. both standard and exteded messages are received
//
// bit 3/4
// =======
//	00 - Normal Mode
//	01 - Listen Mode
//	10 - Loopback Mode
//	11 - Disabled Mode
//
// 

bool CCan4VSCPObj::open( const char *szFileName, unsigned long flags )
{
	char szDrvParams[ MAX_PATH ];
	char *p;
	int nComPort = 1;	// COM1 is default

	m_initFlag = flags;

	m_RxMsgState = INCOMING_STATE_NONE;
	m_RxMsgSubState = INCOMING_SUBSTATE_NONE;

	// save parameter string and convert to upper case
	strncpy( szDrvParams, szFileName, MAX_PATH );
	_strupr( szDrvParams );

	// Initiate statistics
	m_stat.cntReceiveData = 0;
	m_stat.cntReceiveFrames = 0;
	m_stat.cntTransmitData = 0;
	m_stat.cntTransmitFrames = 0;

	m_stat.cntBusOff = 0;
	m_stat.cntBusWarnings = 0;
	m_stat.cntOverruns = 0;

	// if open we have noting to do
	if ( m_bRun ) return true;

#ifdef DEBUG_CAN4VSCP_RECEIVE
	m_flog = fopen( "c:\\can4vscp.txt", "w" );
#endif	

	// Serial port
	p = strtok( szDrvParams, ";" );
	if ( NULL != p ) {		
		nComPort = atoi( p );	
	}
		

	// Open the com port
	if ( !m_com.init( nComPort,
						CBR_57600,
						8,
						NOPARITY,
						ONESTOPBIT,
						HANDSHAKE_NONE ) ) {
		return false;
	}

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
		return false;
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
		return  false;
	}
	
	// Release the mutex
	UNLOCK_MUTEX( m_can4vscpMutex );
	UNLOCK_MUTEX( m_receiveMutex );
	UNLOCK_MUTEX( m_transmitMutex );
	UNLOCK_MUTEX( m_responseMutex );

#else // LINUX

	pthread_attr_t thread_attr;
	pthread_attr_init( &thread_attr );
	
	
	// Create the log write thread.
	if ( pthread_create( &m_threadId,
								&thread_attr,
								workThreadTransmit,
								this ) ) {	
							
		syslog( LOG_CRIT, "can4vscp: Unable to create can4vscpdrv write thread.");
		rv = false;
		fclose( m_flog );
	}


	// Create the log write thread.
	if ( pthread_create( &m_threadId,
								&thread_attr,
								workThreadReceive,
								this ) ) {	
							
		syslog( LOG_CRIT, "can4vscp: Unable to create can4vscpdrv receive thread.");
		rv = false;
		fclose( m_flog );
	}
		
    // We are open
	m_bOpen = true;

	// Release the mutex
	pthread_mutex_unlock( &m_can4vscpMutex );

#endif	

	return true;
}

 
//////////////////////////////////////////////////////////////////////
// close
//

bool CCan4VSCPObj::close( void )
{	
	// Do nothing if already terminated
	if ( !m_bRun ) return false;
	
	m_bRun = false;

#ifdef DEBUG_CAN4VSCP_RECEIVE
	fclose( m_flog );
#endif
 
	UNLOCK_MUTEX( m_can4vscpMutex );
	LOCK_MUTEX( m_can4vscpMutex );

	// Close the com port if its open
	if ( m_com.isOpen() ) {
		m_com.close();
	}
	
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
	int *trv;
	pthread_join( m_threadIdReceive, (void **)&trv );
	pthread_join( m_threadIdTransmit, (void **)&trv );
	pthread_mutex_destroy( &m_can4vscpMutex );
	
#endif

	return true;
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

bool CCan4VSCPObj::setFilter( unsigned long filter )
{
	m_filter = filter;
	return true;
}


//////////////////////////////////////////////////////////////////////
// setMask
//

bool CCan4VSCPObj::setMask( unsigned long mask )
{
	m_mask = mask;
	return true;
}



//////////////////////////////////////////////////////////////////////
// writeMsg
//

bool CCan4VSCPObj::writeMsg( canalMsg *pMsg )
{	
	bool rv = false;
	
	if ( NULL != pMsg ) {

		// Must be room for the message
		if ( m_transmitList.nCount < CAN4VSCP_MAX_SNDMSG ) {

			dllnode *pNode = new dllnode;
			
			if ( NULL != pNode ) {
			
				canalMsg *pcanalMsg = new canalMsg;

				pNode->pObject = pcanalMsg;
				pNode->pKey = NULL;
				pNode->pstrKey = NULL;

				if ( NULL != pcanalMsg ) {
					memcpy( pcanalMsg, pMsg, sizeof( canalMsg ) );
				}

				LOCK_MUTEX( m_transmitMutex );
				dll_addNode( &m_transmitList, pNode );
				UNLOCK_MUTEX( m_transmitMutex );
 
				rv = true;

			}
			else {

				delete pMsg;

			}
		}
	}
	
	return rv;		
}


//////////////////////////////////////////////////////////////////////
// readMsg
//

bool CCan4VSCPObj::readMsg( canalMsg *pMsg )
{
	bool rv = false;
	
	if ( ( NULL != m_receiveList.pHead ) && 
			( NULL != m_receiveList.pHead->pObject ) ) {
		
		memcpy( pMsg, m_receiveList.pHead->pObject, sizeof( canalMsg ) );
		LOCK_MUTEX( m_receiveMutex );
		dll_removeNode( &m_receiveList, m_receiveList.pHead );
		UNLOCK_MUTEX( m_receiveMutex );

		rv = true;
	}

	return rv;
}


///////////////////////////////////////////////////////////////////////////////
//	dataAvailable
//

int CCan4VSCPObj::dataAvailable( void )
{
	int cnt;
	
	LOCK_MUTEX( m_receiveMutex );
	cnt = dll_getNodeCount( &m_receiveList );	
	UNLOCK_MUTEX( m_receiveMutex );
 
	return cnt;
}


///////////////////////////////////////////////////////////////////////////////
//	getStatistics
//

bool CCan4VSCPObj::getStatistics( PCANALSTATISTICS pCanalStatistics )
{	
	// Must be a valid pointer
	if ( NULL == pCanalStatistics ) return false;

	memcpy( pCanalStatistics, &m_stat, sizeof( canalStatistics ) );

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//	getStatus
//

bool CCan4VSCPObj::getStatus( PCANALSTATUS pCanalStatus )
{
	return true;
}


///////////////////////////////////////////////////////////////////////////////
// sendCommand
//
//

bool CCan4VSCPObj::sendCommand( uint8_t cmdcode, uint8_t *pData, uint16_t dataSize )
{
	uint8_t txBuf[300];   
	short size = 0;

	txBuf[ size++ ] = 0x00;
	txBuf[ size++ ] = cmdcode | 0x80;

	
	if ( NULL != pData ) {
		for ( int i=0; i<dataSize; i++ ) {
			txBuf[ size++ ] = pData[ i ];
		}
	}

	return sendMsg( txBuf, size );
}


///////////////////////////////////////////////////////////////////////////////
// sendMsg
//
// [flags][id-msb][id][id][id-lsb][data...] 
//
// This is the raw message send that already is packed
// correctly sequently

bool CCan4VSCPObj::sendMsg( uint8_t *pdata, short dataSize ) 
{
	uint8_t	buffer[ 30 ];
	uint8_t	size = 0;
	uint8_t flags = 0;
	bool rv;

	// BYTE STUFFING
	//----------------------------------------------
	// message format
	// DLE STX flags id data DLE ETX
	// but if data contains a DLE
	// then insert another DLE before it (BYTE Stuffing)
	// this function is not concerned with the data format

	// start the transmission
	buffer[ size++ ] = DLE;
	buffer[ size++ ] = STX;  // start transmission

	// BYTE Stuff the data
	for ( int i=0; i<dataSize; i++ ) {

		if ( pdata[i] == DLE ) {
			buffer[ size++ ] = DLE;
		}

		buffer[ size++ ] = pdata[ i ];
	}

	// terminate the transmission
	buffer[ size++ ] = DLE;
	buffer[ size++ ] = ETX;  // end transmission
	
	if ( m_com.isOpen() ) {
		
		LOCK_MUTEX( m_can4vscpMutex );
		if ( m_com.writebuf( buffer, size ) ) { 
			rv = true;
		}
		else {
			rv = false;
		}
		UNLOCK_MUTEX( m_can4vscpMutex );
	}
	else {
		rv = false;
	}
	
	return rv;
}




///////////////////////////////////////////////////////////////////////////////
// waitResponse
//
//

bool CCan4VSCPObj::waitResponse( responseMsg *pMsg, uint32_t timeout )
{
	uint32_t start = GetTickCount();
	while (  GetTickCount() < ( start + timeout ) ){
	
		if ( ( NULL != m_responseList.pHead ) && 
				( NULL != m_responseList.pHead->pObject ) ) {

			memcpy( pMsg, m_responseList.pHead->pObject, sizeof( responseMsg ) ); 
			LOCK_MUTEX( m_responseMutex );
			dll_removeNode( &m_responseList, m_responseList.pHead );
			UNLOCK_MUTEX( m_responseMutex );

			return true;
		}
		
	}

	return false;
}




///////////////////////////////////////////////////////////////////////////////
// sendCommandWait( )
//
//

bool CCan4VSCPObj::sendCommandWait( uint8_t cmdcode, responseMsg *pMsg, uint32_t timeout )
{
	// Send the command
	if ( !sendCommand( cmdcode ) ) return false;	

	return waitResponse( pMsg, timeout );
}




///////////////////////////////////////////////////////////////////////////////
// ReadSerialData
//
//


bool CCan4VSCPObj::serialData2StateMachine( void )
{
	uint8_t c;	// Serial character
	bool rv = false;
	int cnt = 0;
#ifdef DEBUG_CAN4VSCP_RECEIVE
	char dbgbuf[20];
#endif

/*
	unsigned short ccc;
	_u8 bbuf[ 2048 ];
	ccc = m_com.readBuf( (char *)bbuf, sizeof( bbuf ), sizeof( bbuf ) );
	for ( int i=0; i<ccc; i++ ) {
		sprintf( dbgbuf, "%02X ", bbuf[ i ] );
		fwrite( dbgbuf, 1, strlen( dbgbuf), m_flog );
	}
	return false;
*/

	// Read RS-232 data
	c = m_com.readChar( &cnt );
#ifdef DEBUG_CAN4VSCP_RECEIVE
	if ( cnt ) {
		sprintf( dbgbuf, "%02X ", c );
		fwrite( dbgbuf, 1, strlen( dbgbuf), m_flog );
	}
#endif
		
	while ( !rv && ( 0 != cnt ) ) {
 
		switch ( m_RxMsgState ) {
			
			case INCOMING_STATE_NONE:
				
				if ( ( INCOMING_SUBSTATE_NONE == m_RxMsgSubState ) && ( DLE == c ) ) {
#ifdef DEBUG_CAN4VSCP_RECEIVE
					fprintf( m_flog, " STATE_NONE/SUBSATE_DLE " );
#endif
					m_RxMsgSubState = INCOMING_SUBSTATE_DLE;		
				}
				else if ( ( INCOMING_SUBSTATE_DLE == m_RxMsgSubState ) && ( STX == c ) ) {
#ifdef DEBUG_CAN4VSCP_RECEIVE
					fprintf( m_flog, " STATE_STX/SUBSATE_NONE " );
#endif
					m_RxMsgState = INCOMING_STATE_STX;		
					m_RxMsgSubState = INCOMING_SUBSTATE_NONE;
					m_lengthMsgRcv = 0;
				}
				else {
#ifdef DEBUG_CAN4VSCP_RECEIVE
					fprintf( m_flog, " STATE_NONE/SUBSATE_NONE " );
#endif
					m_lengthMsgRcv = 0;
					m_RxMsgState = INCOMING_STATE_NONE;		
					m_RxMsgSubState = INCOMING_SUBSTATE_NONE;	
				}
				break;
				
					
			case INCOMING_STATE_STX:
			
				if ( ( INCOMING_SUBSTATE_NONE == m_RxMsgSubState ) && ( DLE == c ) ) {
#ifdef DEBUG_CAN4VSCP_RECEIVE
					fprintf( m_flog, " STATE_STX/SUBSTATE_DLE " );
#endif
					m_RxMsgSubState = INCOMING_SUBSTATE_DLE;
				}
				else if ( ( INCOMING_SUBSTATE_DLE == m_RxMsgSubState ) && ( STX == c ) ) {
					// This is strange as a DEL STX is not expected here
					// We try to sync up again...
#ifdef DEBUG_CAN4VSCP_RECEIVE
					fprintf( m_flog, " STATE_STX/SUBSTATE_NONE " );
#endif
					m_RxMsgState = INCOMING_STATE_STX;		
					m_RxMsgSubState = INCOMING_SUBSTATE_NONE;
					m_lengthMsgRcv = 0;
				}
				else if ( ( INCOMING_SUBSTATE_DLE == m_RxMsgSubState ) && ( ETX == c ) ) {
						
					// We have a packet 
#ifdef DEBUG_CAN4VSCP_RECEIVE
					fprintf( m_flog, " STATE_NONE/SUBSTATE_NONE " );
#endif					
					m_RxMsgState = INCOMING_STATE_NONE;
					m_RxMsgSubState = INCOMING_SUBSTATE_NONE;
#ifdef DEBUG_CAN4VSCP_RECEIVE
					fprintf( m_flog, " ***FRAME*** " );
#endif
					return true;
						
				}
				else if ( ( INCOMING_SUBSTATE_DLE == m_RxMsgSubState ) && ( DLE == c ) ) {
					// Byte stuffed DLE  i.e. DLE DLE == DLE
#ifdef DEBUG_CAN4VSCP_RECEIVE
					fprintf( m_flog, " STATE_STX/SUBSTATE_NONE " );
#endif
					m_RxMsgSubState = INCOMING_SUBSTATE_NONE;	
					if ( m_lengthMsgRcv < 13 ) {
						m_bufferMsgRcv[ m_lengthMsgRcv++ ] = c;
					}
					else {
						// This packet has wrong format as it have
						// to many databytes - start over!
#ifdef DEBUG_CAN4VSCP_RECEIVE
					fprintf( m_flog, " STATE_NONE/SUBSTATE_NONE " );
#endif
						m_lengthMsgRcv = 0;
						m_RxMsgState = INCOMING_STATE_NONE;		
						m_RxMsgSubState = INCOMING_SUBSTATE_NONE;	
					}
				}
				// We come here if data is received 
				else {
#ifdef DEBUG_CAN4VSCP_RECEIVE
					fprintf( m_flog, " STATE_STX/SUBSTATE_NONE " );
#endif					
					m_RxMsgSubState = INCOMING_SUBSTATE_NONE;	
					if ( m_lengthMsgRcv < 13 ) {
						m_bufferMsgRcv[ m_lengthMsgRcv++ ] = c;
					}
					else {
						// This packet has wrong format as it have
						// to many databytes - start over!
						m_lengthMsgRcv = 0;
#ifdef DEBUG_CAN4VSCP_RECEIVE
					fprintf( m_flog, " STATE_NONE/SUBSTATE_NONE " );
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
		c = m_com.readChar( &cnt );
#ifdef DEBUG_CAN4VSCP_RECEIVE
		if ( cnt ) {
			sprintf( dbgbuf, "%02X ", c );
			fwrite( dbgbuf, 1, strlen( dbgbuf), m_flog );
		}
#endif

	} // while

	return rv;
}


///////////////////////////////////////////////////////////////////////////////
// readSerialData
//
//

bool CCan4VSCPObj::readSerialData( void )
{
	bool bData = false;
	int cnt = 0;
	
	do {

		if ( serialData2StateMachine() ) {
				
			// Check if command message
			if ( 0x20 == ( m_bufferMsgRcv[ 0 ] & 0x30 ) ) {

				PCANALMSG pMsg	= new canalMsg;
				if ( NULL != pMsg ) {
									
					dllnode *pNode = new dllnode; 
					if ( NULL != pNode ) {

						//pMsg->id      = 0x00;
						//pMsg->command = m_bufferMsgRcv[ 1 ] & 0x7F;									
						//pMsg->len = m_lengthMsgRcv - 2; //(minus two for the first two bytes)
						//memcpy( pMsg->data, ( m_bufferMsgRcv + 2 ), pMsg->len );
									
								
						pNode->pObject = pMsg;
						LOCK_MUTEX( m_responseMutex );
						dll_addNode( &m_responseList, pNode );
						UNLOCK_MUTEX( m_responseMutex );
					}
					else {									
						delete pMsg;											
					}
				}
			} 
			else {	

				// CAN message	
				// ----------------------------
				// 0:[EXT:1][RTR:1][TYPE:2][DLC:4] flags
				// 1:[ID MSB]
				// 2:[ID3]
				// 3:[ID2]
				// 4:[ID LSB]
				// 5-12:[DATA 0-8 bytes]

				if (  m_receiveList.nCount < CAN4VSCP_MAX_RCVMSG ) {					
					
					PCANALMSG pMsg	= new canalMsg;

					if ( NULL != pMsg ) {
                    
                        pMsg->flags = 0;
						dllnode *pNode = new dllnode; 
						if ( NULL != pNode ) {
							
							pMsg->timestamp = GetTickCount() * 1000;

							// TODO endian
							pMsg->id = 
									(((DWORD)m_bufferMsgRcv[1]<<24) & 0x1f000000) |
									(((DWORD)m_bufferMsgRcv[2]<<16) & 0x00ff0000) |
									(((DWORD)m_bufferMsgRcv[3]<<8 ) & 0x0000ff00) |
									(((DWORD)m_bufferMsgRcv[4]    ) & 0x000000ff) ;
											
							pMsg->sizeData = m_bufferMsgRcv[0] & 0x0f;		
											
							memcpy( (void *)pMsg->data, 
										(m_bufferMsgRcv + 5 ), 
										pMsg->sizeData ); 
									
							// If extended set extended flag
							if ( m_bufferMsgRcv[0] & 0x80 ) pMsg->flags |= CANAL_IDFLAG_EXTENDED;

							// Check for RTR package
							if ( (m_bufferMsgRcv[0] & 0x40) ) pMsg->flags |= CANAL_IDFLAG_RTR;
							
							if ( doFilter( pMsg ) ) {
								pNode->pObject = pMsg;
								LOCK_MUTEX( m_receiveMutex );
								dll_addNode( &m_receiveList, pNode );
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

						}
						else {
							delete pMsg;

						}
					}  // No pNode
				} // No pMsg 
				else {
					// Full buffer
					m_stat.cntOverruns++;
				}
			} // No room in receive queue

			m_RxMsgState = INCOMING_STATE_NONE; // reset state for next msg
			m_RxMsgSubState = INCOMING_SUBSTATE_NONE;
		}

	} while ( cnt != 0 );							
							
	return bData;
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
		
		// Noting to do if we should end...
		if ( !pobj->m_bRun ) continue;

		// Is there something to transmit
		while ( ( NULL != pobj->m_transmitList.pHead ) && 
				( NULL != pobj->m_transmitList.pHead->pObject ) ) {

			canalMsg msg;
			memcpy( &msg, pobj->m_transmitList.pHead->pObject, sizeof( canalMsg ) ); 
			LOCK_MUTEX( pobj->m_transmitMutex );
			dll_removeNode( &pobj->m_transmitList, pobj->m_transmitList.pHead );
			UNLOCK_MUTEX( pobj->m_transmitMutex );

			// Outgoing CAN message
			// --------------------------
			// [0] ([EXT:1][RTR:1][TYPE:2][count:4])
			// [1] ID MSB
			// [2] ID
			// [3] ID
			// [4] ID LSB
			// [5-12] DATA

			uint8_t sendData[ 20 ];
			short size = 0;

			sendData[ size ] = 
					( ( msg.flags & CANAL_IDFLAG_EXTENDED ) ? 0x80 : 0x00 ) |
					( ( msg.flags & CANAL_IDFLAG_RTR ) ? 0x40:0x00);
			sendData[ size++ ] += msg.sizeData;
			sendData[ size++ ] = ( uint8_t )( msg.id >> 24 ) & 0x1f;
			sendData[ size++ ] = ( uint8_t )( msg.id >> 16 ) & 0xff;
			sendData[ size++ ] = ( uint8_t )( msg.id >> 8 )  & 0xff;
			sendData[ size++ ] = ( uint8_t )( msg.id ) & 0xff;

			memcpy( sendData + 5, msg.data, msg.sizeData );
			size += msg.sizeData;
			 
			LOCK_MUTEX( pobj->m_can4vscpMutex );

			if ( pobj->sendMsg( sendData, size ) ) {
	
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

			UNLOCK_MUTEX( pobj->m_can4vscpMutex );
										
		} // while data


		// No data to write
		SLEEP( 1 );

	
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
	bool bData;

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
		
		// Noting to do if we should end...
		if ( !pobj->m_bRun ) continue;

		LOCK_MUTEX( pobj->m_can4vscpMutex );
		bData = pobj->readSerialData();
		UNLOCK_MUTEX( pobj->m_can4vscpMutex );
		//if ( !bData) SLEEP( 1 );	// Sleep if no data
	
	} // while 	 


#ifdef WIN32
	ExitThread( errorCode );
#else
	pthread_exit( &rv );
#endif

}
