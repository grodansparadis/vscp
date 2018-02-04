///////////////////////////////////////////////////////////////////////////////
// CAN232Obj.cpp: implementation of the CCAN232Obj class.
//
// This file is part is part of CANAL (CAN Abstraction Layer)
// http://www.vscp.org)
//
// Copyright (C) 2000-2014
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
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
// $RCSfile: can232obj.cpp,v $                                       
// $Date: 2005/09/12 20:35:07 $                                  
// $Author: akhe $                                              
// $Revision: 1.5 $ 
///////////////////////////////////////////////////////////////////////////////

#include "stdio.h"
#include "can232obj.h"

// Prototypes
void workThread( void *p );
bool can323ToCanal( char * p, PCANALMSG pMsg );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCAN232Obj::CCAN232Obj()
{
	m_can232obj.m_version = -1;					// Version is unknown
	m_can232obj.m_bAuto = false;				// Poll mode is default
	m_can232obj.m_cntRcv = 0;					// No chars in receive buffer yet
	m_can232obj.m_state = CAN232_STATE_NONE;
	dll_init( &m_can232obj.m_rcvList, SORT_NONE );
	dll_init( &m_can232obj.m_sndList, SORT_NONE );
}

CCAN232Obj::~CCAN232Obj()
{
	close();	// Cose comm channel in case its open
	dll_removeAllNodes( &m_can232obj.m_rcvList );
	dll_removeAllNodes( &m_can232obj.m_sndList );
}


///////////////////////////////////////////////////////////////////////////////
// Open the CAN232 comm port
//
// Parameters are as follows
//
//		comport;baudrate;mask;filter;bus-speed;btr0;btr1
//
// if btr0 and btr1 is defined they should be used instead of bus-speed which
// in that case can have any value even if zero is recommended.
//

long CCAN232Obj::open( const char *pDevice, unsigned long flags )
{
	long h = NULL;

	const char *p;
	char szCmd[ 80 ];
	unsigned char nComPort = 1;
	DWORD nBaudRate = 115200; 
	unsigned long nMask = 0;
	unsigned long nFilter = 0;
	unsigned long busspeed = 125;
	unsigned char btr0 = 0, btr1 = 0;
	unsigned char nSpeed = 4;		// 125k

	m_can232ObjMutex = OpenMutex( MUTEX_ALL_ACCESS, true, CAN232_OBJ_MUTEX );

	m_can232obj.m_bRun = true;

	// if open we have noting to do
	if ( NULL != m_can232obj.m_comm.getHandle() ) return 0;
	
	// Port
	int tmpport;
	p = strtok( (char * )pDevice, ";" );
	if ( ( NULL != p ) && ( tmpport = atoi( p ) ) ) nComPort = tmpport;

	// Baudrate
	p = strtok( NULL, ";" );
	if ( NULL != p ) nBaudRate = atoi( p );

	// Mask
	p = strtok( NULL, ";" );
	if ( NULL != p ) {		
		if ( ( NULL != strstr( p, "0x" ) ) || ( NULL != strstr( p, "0X" ) )  ) {
			sscanf( p + 2, "%x", &nMask );
		}
		else {
			nMask = atol( p );
		}
	}

	// Filter
	p = strtok( NULL, ";" );
	if ( NULL != p ) {		
		if ( ( NULL != strstr( p, "0x" ) ) || ( NULL != strstr( p, "0X" ) )  ) {
			sscanf( p + 2, "%x", &nFilter );
		}
		else {
			nFilter = atol( p );
		}
	}

	// Bus-Speed
	p = strtok( NULL, ";" );
	if ( NULL != p ) busspeed = atol( p );

	// BTR0
	p = strtok( NULL, ";" );
	if ( NULL != p ) btr0 = atoi( p );

	// BTR1
	p = strtok( NULL, ";" );
	if ( NULL != p ) btr1 = atoi( p );

	if ( m_can232obj.m_comm.init( nComPort, nBaudRate, 8, NOPARITY, ONESTOPBIT, HANDSHAKE_NONE ) ) {
		h = 1;
	}
	else{
		return 0;
	}
	
	m_can232obj.m_receiveBuf[ 0 ] = 0;
	m_can232obj.m_cntRcv = 0;

	// Initiate statistics
	m_can232obj.m_stat.cntReceiveData = 0;
	m_can232obj.m_stat.cntReceiveFrames = 0;
	m_can232obj.m_stat.cntTransmitData = 0;
	m_can232obj.m_stat.cntTransmitFrames = 0;

	m_can232obj.m_stat.cntBusOff = 0;
	m_can232obj.m_stat.cntBusWarnings = 0;
	m_can232obj.m_stat.cntOverruns = 0;

	switch ( busspeed ) {
	
		case 10:
			nSpeed = 0;
			break;

		case 20:
			nSpeed = 1;
			break;

		case 50:
			nSpeed = 2;
			break;

		case 100:
			nSpeed = 3;
			break;

		case 125:
			nSpeed = 4;
			break;

		case 250:
			nSpeed = 5;
			break;

		case 500:
			nSpeed = 6;
			break;

		case 800:
			nSpeed = 7;
			break;

		case 1000:
			nSpeed = 8;
			break;

	}

	// Empty buffers from trash
	m_can232obj.m_comm.write( "C", true, true); // Needed in case of not closed by last process.
	Sleep( 100 );
	m_can232obj.m_comm.drainInput();

	m_can232obj.m_comm.write( "\r" );
	Sleep( 100 );
	m_can232obj.m_comm.drainInput();

	m_can232obj.m_comm.write( "\r" );
	Sleep( 100 );
	m_can232obj.m_comm.drainInput();

	// Get version
	m_can232obj.m_comm.write( "V", true, true );
	m_can232obj.m_comm.readBuf( szCmd, sizeof( szCmd ), 6 );
	if ( ( 'V' == *szCmd ) && ( strlen( szCmd ) >= 5 ) ) {
		m_can232obj.m_version = atoi( szCmd + 1 );
	}

	// Set bitrate
	if ( btr0 || btr1 ) {
		sprintf( szCmd, "s%2X%2X", btr0, btr1 );
		if ( !m_can232obj.m_comm.sendCommand( szCmd, "\r" ) ) {
			m_can232obj.m_comm.close();
			return 0;
		}
	}
	else {
		sprintf( szCmd, "S%u", nSpeed );
		if ( !m_can232obj.m_comm.sendCommand( szCmd, "\r" ) ) {
			m_can232obj.m_comm.close();
			return 0;
		}
	}

	// Open
	if ( !m_can232obj.m_comm.sendCommand( "O", "\r" ) ) {
		m_can232obj.m_comm.close();
		return 0;
	}

	// Check for autopoll mode
	if ( m_can232obj.m_version >= 1220 ) {
		m_can232obj.m_comm.write( "P", true, true );
		*szCmd = 0;
		m_can232obj.m_comm.readBuf( szCmd, sizeof( szCmd ), -1 );
		if ( 0x07 == *szCmd ) {
			m_can232obj.m_bAuto = true;
		}
	}

	// Start the worker thread	
	if ( NULL == CreateThread(	NULL,
								0,
								(LPTHREAD_START_ROUTINE) workThread,
								&m_can232obj,
								0,
								&m_dwThreadId ) ) { 
		close();	
		return 0;
	}

	// Release the mutex for other threads to use
	ReleaseMutex( m_can232ObjMutex );


	return h;
}


///////////////////////////////////////////////////////////////////////////////
//  close
// 

int CCAN232Obj::close( void )
{
	int rv = 0;

	// Terminate the thread
	m_can232obj.m_bRun = false;

	// if closed we have noting to do
	if ( NULL == m_can232obj.m_comm.getHandle() ) return 0;

	if ( m_can232obj.m_comm.sendCommand( "C", "\r" ) ) {
		rv = 1;
	}

	m_can232obj.m_comm.close();
	return rv;
}


///////////////////////////////////////////////////////////////////////////////
//  writeMsg
// 

int CCAN232Obj::writeMsg( bool bExtended, 
							unsigned long id, 
							unsigned char dlc, 
							unsigned char * pdata )
{
	int rv = 0;
	
	// Must be room for the message
	if ( m_can232obj.m_sndList.nCount < CAN232_MAX_SNDMSG ) {
		
		PCANALMSG pMsg	= new canalMsg;

		pMsg->flags = 0;
		if ( bExtended ) pMsg->flags = CANAL_IDFLAG_EXTENDED;
		pMsg->id =id;
		pMsg->sizeData = dlc;
		if ( dlc > 0 ) {
			for ( int i = 0; i < pMsg->sizeData; i++ ) {
				pMsg->data[i ] = pdata[ i ];
			}
		}

		if ( NULL != pMsg ) {
					
			dllnode *pNode = new dllnode;
			if ( NULL != pNode ) {
				
				pNode->pObject = pMsg;

				WaitForSingleObject( m_can232ObjMutex, INFINITE );
				dll_addNode( &m_can232obj.m_sndList, pNode );
				ReleaseMutex( m_can232ObjMutex );
				
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
//  writeMsg
// 

int CCAN232Obj::writeMsg( PCANALMSG pCanalMsg )
{
	int rv = 0;

	// Must be room for the message
	if ( m_can232obj.m_sndList.nCount < CAN232_MAX_SNDMSG ) {
			
		if ( NULL != pCanalMsg ) {
					
			dllnode *pNode = new dllnode;

			if ( NULL != pNode ) {

				canalMsg *pnewMsg = new canalMsg;

				pNode->pObject = pnewMsg;
				pNode->pKey = NULL;
				pNode->pstrKey = NULL;

				if ( NULL != pnewMsg ) {
					memcpy( pnewMsg, pCanalMsg, sizeof( canalMsg ) );
				}
				

				WaitForSingleObject( m_can232ObjMutex, INFINITE );
				dll_addNode( &m_can232obj.m_sndList, pNode );
				ReleaseMutex( m_can232ObjMutex );
							
				rv = true;

			}	 
		}		
	}

	return rv;	
}

///////////////////////////////////////////////////////////////////////////////
//  readMsg
// 

int CCAN232Obj::readMsg( canalMsg *pMsg )
{			
	int rv = false;
	
	if ( ( NULL != m_can232obj.m_rcvList.pHead ) && 
			( NULL != m_can232obj.m_rcvList.pHead->pObject ) ) {
		
		WaitForSingleObject( m_can232ObjMutex, INFINITE );
		
		memcpy( pMsg, m_can232obj.m_rcvList.pHead->pObject, sizeof( canalMsg ) );
		dll_removeNode( &m_can232obj.m_rcvList, m_can232obj.m_rcvList.pHead );
		
		ReleaseMutex( m_can232ObjMutex );
		rv = true;
	}

	return rv;
}


///////////////////////////////////////////////////////////////////////////////
//	setFilter
//

bool CCAN232Obj::setFilter( unsigned long filter, unsigned long mask )
{
	char buf[ 20 ];
	char szCmd[ 80 ];

	WaitForSingleObject( m_can232ObjMutex, INFINITE );

	// Acceptance Code
	sprintf( buf, "M%8.8X", filter );
	m_can232obj.m_comm.write( buf, true, true );
	m_can232obj.m_comm.readBuf( szCmd, sizeof( szCmd ), -1 );

	// Acceptance Mask
	sprintf( buf, "m%8.8X", mask );
	m_can232obj.m_comm.write( buf, true, true );
	m_can232obj.m_comm.readBuf( szCmd, sizeof( szCmd ), -1 );

	ReleaseMutex( m_can232ObjMutex );

	return true;
}


///////////////////////////////////////////////////////////////////////////////
//	setFilter
//

bool CCAN232Obj::setFilter( unsigned long filter )
{
	char buf[ 20 ];
	char szCmd[ 80 ];

	WaitForSingleObject( m_can232ObjMutex, INFINITE );

	// Acceptance Code
	sprintf( buf, "M%8.8X", filter );
	m_can232obj.m_comm.write( buf, true, true );
	m_can232obj.m_comm.readBuf( szCmd, sizeof( szCmd ), -1 );

	ReleaseMutex( m_can232ObjMutex );

	return true;
}


///////////////////////////////////////////////////////////////////////////////
//	setMask
//

bool CCAN232Obj::setMask( unsigned long mask )
{
	char buf[ 20 ];
	char szCmd[ 80 ];

	WaitForSingleObject( m_can232ObjMutex, INFINITE );

	// Acceptance Mask
	sprintf( buf, "m%8.8X", mask );
	m_can232obj.m_comm.write( buf, true, true );
	m_can232obj.m_comm.readBuf( szCmd, sizeof( szCmd ), -1 );

	ReleaseMutex( m_can232ObjMutex );

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//	dataAvailable
//

int CCAN232Obj::dataAvailable( void )
{
	int cnt;
	
	WaitForSingleObject( m_can232ObjMutex, INFINITE );
	cnt = dll_getNodeCount( &m_can232obj.m_rcvList );	
	ReleaseMutex( m_can232ObjMutex );

	return cnt;
}

///////////////////////////////////////////////////////////////////////////////
//	getStatistics
//

bool CCAN232Obj::getStatistics( PCANALSTATISTICS pCanalStatistics )
{
	pCanalStatistics = &m_can232obj.m_stat;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
//	getStatus
//

bool CCAN232Obj::getStatus( PCANALSTATUS pCanalStatus )
{
	// Get flags	
	//	B0 - CAN receive FIFO full.
	//	B1 - CAN transmit FIFO full.
	//	B2 - Error Warning (EI).
	//	B3 - Data Overrun (DOI).
	//	B4 - Not used.
	//	B5 - Error Passive (EPI).
	//	B6 - Arbitration Lost (ALI).
	//	B7 - Bus Error (BEI)
	char rBuf[ 32 ];

	WaitForSingleObject( m_can232ObjMutex, INFINITE );

	m_can232obj.m_comm.write( "F", true, true );
	*rBuf = 0;
	m_can232obj.m_comm.readBuf( rBuf, sizeof( rBuf ), -1 );
	if ( 0x07 != *rBuf ) {
		int flags;

		rBuf[ 3 ] = 0;				
		sscanf( rBuf, "F%x", &flags );

		pCanalStatus->channel_status = 0;

		if ( flags & 0x01 ) { // Receive buffer full
			pCanalStatus->channel_status  |= CANAL_STATUS_RECIVE_BUFFER_FULL;	
		}

		if ( flags & 0x02 ) { // Transmitt buffer full
			pCanalStatus->channel_status  |= CANAL_STATUS_TRANSMIT_BUFFER_FULL;	
		}

		if ( flags & 0x04 ) { // Error Warning
			pCanalStatus->channel_status  |= CANAL_STATUS_BUS_WARN;	
		}

		if ( flags & 0x80 ) { // Error Warning
			pCanalStatus->channel_status  |= CANAL_STATUS_BUS_OFF;	
		}

		pCanalStatus->channel_status  |= CANAL_STATUS_ACTIVE;
	}
	else {
		return false;
	}

	ReleaseMutex( m_can232ObjMutex );
	
	return true;
}


///////////////////////////////////////////////////////////////////////////////
// workThread
//
// The workThread do most of the acual work such as send and receive.
//
//

void workThread( void *pThreadObject )
{
	bool bRun = true;
	bool bActivity = true;
	short nPollCnt = 0;
	DWORD errorCode = 0;
	HANDLE can232ObjMutex;	 		
	
	// Must hav a valid work object
	if ( NULL == pThreadObject ) ExitThread( errorCode );

	// Get the object pointer in place
	struct _can232obj *pcan232obj = (struct _can232obj *)pThreadObject;
	
	can232ObjMutex = OpenMutex( MUTEX_ALL_ACCESS, false, CAN232_OBJ_MUTEX );

	while ( pcan232obj->m_bRun ) {
		
		///////////////////////////////////////////////////////////////////////
		//								Receive 
		///////////////////////////////////////////////////////////////////////
		WaitForSingleObject( can232ObjMutex, INFINITE );

		int cnt;
		unsigned char c;
		
		c = pcan232obj->m_comm.readChar( &cnt );
		
		while ( 0 != cnt ) {

			bActivity = true;
		
			if ( CAN232_STATE_NONE == pcan232obj->m_state ) {
		
				if ( ('t' == c ) || ( 'T' == c ) || ('r' == c ) || ( 'R' == c ) ) {
					pcan232obj->m_state = CAN232_STATE_MSG;
					pcan232obj->m_receiveBuf[ 0 ] = c;
					pcan232obj->m_cntRcv = 1;
				}
				
			}
			else if ( CAN232_STATE_MSG == pcan232obj->m_state ) {
				
				pcan232obj->m_receiveBuf[ pcan232obj->m_cntRcv++ ] = c;
			
				if ( 0x0d == c ) {

					// One full message received

					// If there is place in the queue 
					//		add message to it
					if (  pcan232obj->m_rcvList.nCount < CAN232_MAX_RCVMSG ) {					
					
						PCANALMSG pMsg	= new canalMsg;
						pMsg->flags = 0;

						if ( NULL != pMsg ) {
						
							dllnode *pNode = new dllnode; 
							if ( NULL != pNode ) {
							
								if ( can232ToCanal( pcan232obj->m_receiveBuf, pMsg ) ) {									
									
									pNode->pObject = pMsg;
									dll_addNode( &pcan232obj->m_rcvList, pNode );

									// Update statistics
									pcan232obj->m_stat.cntReceiveData += pMsg->sizeData;
									pcan232obj->m_stat.cntReceiveFrames += 1;

								}
								else {

									// Failed to translate message
									delete pMsg;
									delete pNode;

								}
							}
							else {

								delete pMsg;

							}
						}				
					}

					pcan232obj->m_state = CAN232_STATE_NONE;

				}

				if ( pcan232obj->m_cntRcv > sizeof( pcan232obj->m_receiveBuf ) ) {
					// Problems start all over again
					pcan232obj->m_state = CAN232_STATE_NONE;
				}

			}

			c = pcan232obj->m_comm.readChar( &cnt );

		} // while


		ReleaseMutex( can232ObjMutex );


		///////////////////////////////////////////////////////////////////////
		//								Transmit
		///////////////////////////////////////////////////////////////////////

		WaitForSingleObject( can232ObjMutex, INFINITE );

		// Is there anything to transmit
		if ( ( NULL != pcan232obj->m_sndList.pHead ) && 
					( NULL != pcan232obj->m_sndList.pHead->pObject ) ) {
			char buf[ 80 ];
			canalMsg msg;
			bActivity = true;

			memcpy( &msg, pcan232obj->m_sndList.pHead->pObject, sizeof( canalMsg ) ); 
			dll_removeNode( &pcan232obj->m_sndList, pcan232obj->m_sndList.pHead );
			 
			// Must be a valid standard id
			if ( !( msg.flags & CANAL_IDFLAG_EXTENDED ) && ( msg.id > 0x7ff ) ) {
				msg.id &= 0x7ff;	
			};

			// Must be a valid extended id
			if ( ( msg.flags & CANAL_IDFLAG_EXTENDED ) && ( msg.id > 0x1fffffff ) ) {
				msg.id &= 0x1fffffff;	
			}

			if ( msg.flags & CANAL_IDFLAG_EXTENDED ) {
				sprintf( buf, "T%08.8lX%i", msg.id, msg.sizeData );		
			}
			else {
				sprintf( buf, "t%03.3lX%i", msg.id, msg.sizeData  );	
			}

			if ( msg.sizeData ) {
				char hex[5];

				for ( int i= 0; i< msg.sizeData; i++ ) {
					sprintf( hex, "%02.2X", msg.data[i] );
					strcat( buf, hex );
				}
			}

			// Send the data
			char szResponse[ 32 ];
			pcan232obj->m_comm.write( buf, true, true );
			pcan232obj->m_comm.readBuf( szResponse, sizeof( szResponse ), -1 );

	
			// Update statistics
			pcan232obj->m_stat.cntTransmitData += msg.sizeData;
			pcan232obj->m_stat.cntTransmitFrames += 1;

			// If not in autopoll mode
			if ( 0x0d == *szResponse ) {
				
			}
			// If in autopoll mode extended message
			else if ( ( msg.flags & CANAL_IDFLAG_EXTENDED ) && 
													( 'Z' == *szResponse ) ) {
				
			}
			// If in autopoll mode standard message
			else if ( !( msg.flags & CANAL_IDFLAG_EXTENDED ) && 
													( 'z' == *szResponse ) ) {
				
			}
			
		}

		// If not in autopoll mode we do a poll for all frames first
		nPollCnt++;

		if ( !pcan232obj->m_bAuto && ( nPollCnt > 5 ) ) {
			pcan232obj->m_comm.write( "A", true, true );		
			nPollCnt = 0;			
		}

		ReleaseMutex( can232ObjMutex );

		if ( !bActivity ) Sleep ( 100 );
		bActivity = false;

	}

	// Release the mutex for other threads to use
	ReleaseMutex( can232ObjMutex );

	ExitThread( errorCode );
}


///////////////////////////////////////////////////////////////////////////////
// can323ToCanal
//

bool can323ToCanal( char * p, PCANALMSG pMsg )
{
	bool rv = true;
	int val;
	short data_offset;	// Offset to dlc byte
	char save;

	if ( 't' == *p ) {
		// Standard frame
		pMsg->flags = 0;
		data_offset = 5;
		pMsg->sizeData = p[ 4 ] - '0';
		p[ 4 ] = 0;
		sscanf( p + 1, "%lx", &pMsg->id  );
	}
	else if ( 'r' == *p ) {
		// Standard remote  frame
		pMsg->sizeData = 0;
		pMsg->flags = CANAL_IDFLAG_RTR;
		data_offset = 5 - 1;	// To make timestamp work
		save = p[ 4 ];
		p[ 4 ] = 0;
		sscanf( p + 1, "%lx", &pMsg->id  );
		p[ 4 ] = save;
	}
	else if ( 'T' == *p ) {
		// Extended frame
		pMsg->flags = CANAL_IDFLAG_EXTENDED;
		data_offset = 10;
		pMsg->sizeData = p[ 9 ] - '0';
		p[ 9 ] = 0;
		sscanf( p + 1, "%lx", &pMsg->id );		
	}
	else if ( 'R' == *p ) {
		// Extended remote frame
		pMsg->sizeData = 0;
		pMsg->flags = CANAL_IDFLAG_EXTENDED | CANAL_IDFLAG_RTR;
		data_offset = 10 - 1;	// To make timestamp work
		save = p[ 9 ];
		p[ 9 ] = 0;
		sscanf( p + 1, "%lx", &pMsg->id );	
		p[ 9 ] = save;
	}
    else {
        rv = false;
    }
    
    if ( false != rv ) {
	
        save = *(p + data_offset + 2 * pMsg->sizeData );
        
        if ( !( pMsg->flags & CANAL_IDFLAG_RTR ) ) {
            for ( int i= pMsg->sizeData; i > 0; i-- ) {
                *(p + data_offset + 2 * (i-1) + 2 )	= 0;
                sscanf( p + data_offset + 2 * (i-1), "%x", &val );
                pMsg->data[ i - 1 ] = val;
            }
        }

        *(p + data_offset + 2 * pMsg->sizeData ) = save;

        // If timestamp is actve - fetch it
        if ( 0x0d != *( p + data_offset + 2 * pMsg->sizeData ) ) {
            p[ data_offset + 2 * ( pMsg->sizeData ) + 4 ] = 0;
            sscanf( ( p + data_offset + 2 * ( pMsg->sizeData ) ), "%x", &val );
            pMsg->timestamp = val * 1000;	// microseconds 
        }
        else {
            pMsg->timestamp = 0;	
        }
    }

	return rv;
}
