// vectorobj.cpp:  
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
// Boston, MA 02111-1307, USA.
//

#include "stdio.h"
#include "vectorobj.h"
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
// CVectorObj
//

CVectorObj::CVectorObj()
{ 
	m_drvidx = 0;

	m_channel = 0;
	m_channelMask = 0;
	m_initFlag = 0;
	m_nBrdType = 0;
	m_idxBoard = 0;
	m_portHandle = INVALID_PORTHANDLE;
	
	// No filter mask
	m_filter = 0;
	m_mask = 0;

	m_bRun = false;

#ifdef WIN32
	
	m_hTreadReceive = 0;
	m_hTreadTransmit = 0;

	// Create the device AND LIST access mutexes
	m_vectorMutex = CreateMutex( NULL, true, CANAL_DLL_VECTOR_OBJ_MUTEX );
	m_receiveMutex = CreateMutex( NULL, true, CANAL_DLL_VECTOR_RECEIVE_MUTEX );
	m_transmitMutex = CreateMutex( NULL, true, CANAL_DLL_VECTOR_TRANSMIT_MUTEX );

#else
	
	m_flog = NULL;
	pthread_mutex_init( &m_vectorMutex, NULL );
	pthread_mutex_init( &m_receiveMutex, NULL );
	pthread_mutex_init( &m_transmitMutex, NULL );

#endif

	dll_init( &m_transmitList, SORT_NONE );
	dll_init( &m_receiveList, SORT_NONE );
}

//////////////////////////////////////////////////////////////////////
// ~CVectorObj
//

CVectorObj::~CVectorObj()
{		 
	close();
	
	LOCK_MUTEX( m_transmitMutex );
	dll_removeAllNodes( &m_transmitList );
	
	LOCK_MUTEX( m_receiveMutex );
	dll_removeAllNodes( &m_receiveList );


#ifdef WIN32	
	
	if ( NULL != m_vectorMutex ) CloseHandle( m_vectorMutex );	
	if ( NULL != m_receiveMutex ) CloseHandle( m_receiveMutex );
	if ( NULL != m_transmitMutex ) CloseHandle( m_transmitMutex );

#else

	pthread_mutex_destroy( &m_vectorMutex );
	pthread_mutex_destroy( &m_receiveMutex );
	pthread_mutex_destroy( &m_transmitMutex );

#endif
}


struct __vectorsymtbl {
	char symname[ 32 ];
	int id;
} vectorsymtbl[] = {
		"NONE",					0,
		"VIRTUAL",				1,
		"CANCARDX",				2,
		"CANPARI",				3,
		"CANAC2",				5,
		"CANAC2PCI",			6,
		"CANCARDY",				12,
		"CANCARDXL",			15,
		"CANCARD2",				17,
		"EDICCARD",				19,
		"CANCASEXL",			21,
		"CANBOARDXL",			25,
		"CANBOARDXL_COMPACT",	27,
		"",						-1
} ;


//////////////////////////////////////////////////////////////////////
// open
//
//
// filename
//-----------------------------------------------------------------------------
// Parameters for the driver as a string on the following form
//		
// "board;boardidx;channel;filter;mask;bus-speed"
//
// board
// =====
//
// board can be on a symbolic for or as a number as follows
//
// VIRTUAL				or  1  for virtual adapter
// CANCARDX				or  2   
// CANPARI				or  3   
// CANAC2				or  5   
// CANAC2PCI			or  6  
// CANCARDY				or  12  
// CANCARDXL			or  15 PCI Card  
// CANCARD2				or  17  
// EDICCARD				or	19	  
// CANCASEXL			or  21  
// CANBOARDXL			or  25 
// CANBOARDXL_COMPACT	or  27   
//
// board index
// ===========
// index for board if more then one of same type (0...n) 
//
// channel
// =======
// Is a value from 0 an up indicating the CAN channel on the selected board.
//
// filter
// ======
// Is the hardware dependent filter for this board hardware. Note that this
// filter may work in a different way then the CANAL filter.
//
// mask
// ====
// Is the hardware dependent mask for this board hardware. Note that this
// filter may work in a different way then the CANAL filter.
//
// bus-speed
// =========
// One of the predefined bitrates can be set here
// 
// The actual bitrate is set here i.e. 125b is given as 125,000 
// 
//
// 
// flags 
//-----------------------------------------------------------------------------
//
// bit 0
// =====
// Reserved
//
// bit 1
// =====
// reserved
//
// bit 2
// =====
//	0		Filter our own TX messages from our receive..
//	1		All sent CAN objects appear
//			as received CAN objects in the
//			rx queues.
//
// bit 3
// =====
//	0		Active Mode.
//	1		Passive mode: CAN controller works as passive 
//			CAN node (only monitoring) and
//			therefore it does not send any
//			acknowledge bit for CAN objects
//			sent by another CAN node.
//
// bit 4
// =====
//	0		No error report objects.
//	1		Errorframes are detected and
//			reported as CAN objects via
//			the rx queues
// 

bool CVectorObj::open( const char *szFileName, unsigned long flags )
{
	Vstatus vErr;
	const char *p;
	unsigned long busspeed = 125;
	unsigned char btr0 = 0, btr1 = 0;
	char szDrvParams[ MAX_PATH ];
	m_initFlag = flags;

	// save parameter string and conbert to upper case
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
	
	// Adapter type
	p = strtok( (char * )szDrvParams, ";" );
	if ( NULL != p ) {
	
		if ( isalpha( *p ) ) {
		
			// Symbolic form
			int symidx = 0;
			while( -1 != vectorsymtbl[ symidx ]. id ) {
			
				if ( NULL != strstr( vectorsymtbl[ symidx ]. symname, p ) ) {
					m_nBrdType = vectorsymtbl[ symidx ]. id;
					break;
				}

				symidx++;	

			}
				 	
		}
		else {
			
			// Numeric form
			m_nBrdType = atoi( p );	

		}

	}


	// Board Index
	p = strtok( NULL, ";" );
	if ( NULL != p ) {		
		if ( ( NULL != strstr( p, "0x" ) ) || ( NULL != strstr( p, "0X" ) )  ) {
			sscanf( p + 2, "%x", &m_idxBoard );
		}
		else {
			m_idxBoard = atoi( p );
		}
	}

	// channel 
	p = strtok( NULL, ";" );
	if ( NULL != p ) {		
		if ( ( NULL != strstr( p, "0x" ) ) || ( NULL != strstr( p, "0X" ) )  ) {
			sscanf( p + 2, "%x", &m_channel );
		}
		else {
			m_channel = atoi( p );
		}
	}

	// Filter
	p = strtok( NULL, ";" );
	if ( NULL != p ) {		
		if ( ( NULL != strstr( p, "0x" ) ) || ( NULL != strstr( p, "0X" ) )  ) {
			sscanf( p + 2, "%x", &m_Vector_filter );
		}
		else {
			m_Vector_filter = atol( p );
		}
	}

	// Mask
	p = strtok( NULL, ";" );
	if ( NULL != p ) {		
		if ( ( NULL != strstr( p, "0x" ) ) || ( NULL != strstr( p, "0X" ) )  ) {
			sscanf( p + 2, "%x", &m_Vector_mask );
		}
		else {
			m_Vector_mask = atol( p );
		}
	}

	// Bus-Speed
	p = strtok( NULL, ";" );
	if ( NULL != p ) {		
		if ( ( NULL != strstr( p, "0x" ) ) || ( NULL != strstr( p, "0X" ) )  ) {
			sscanf( p + 2, "%x", &busspeed );
		}
		else {
			busspeed = atol( p );
		}
	}
	
	// Open the driver
	if ( ( vErr = ncdOpenDriver () ) ) return false;

	// get the number of channels
	int nChannels;
	vErr = ncdGetDriverConfig( &nChannels, NULL ); 

	// Get channel index for requested card
	short idxChannel;
	if ( -1 == ( idxChannel = 
					ncdGetChannelIndex( m_nBrdType, m_idxBoard, m_channel ) ) ) {

		ncdCloseDriver();
		return false;
	}

	Vaccess initMask;
	if ( -1 == ( m_channelMask = 
					ncdGetChannelMask( m_nBrdType, m_idxBoard, m_channel ) ) ) {

		ncdCloseDriver();
		return false;
	}

	

	// Try to get init status for the channel if possible
	initMask = m_channelMask;

	char vectorChName[ MAX_APPNAME+1 ];
	Vaccess permissionMask;
	sprintf( vectorChName, "vectordrv%i", m_drvidx );
	if ( VERROR == ncdOpenPort( &m_portHandle,
								vectorChName,
								m_channelMask,
								initMask,
								&permissionMask,
								1024 ) ) {
		ncdCloseDriver();
		return false;
	}

	// if permission to initialize
	if ( permissionMask ) {
		
		VchipParams chipInit;

		chipInit.sjw   = 1;
		chipInit.tseg1 = 4;
		chipInit.tseg2 = 3;
		chipInit.sam   = 1;
		chipInit.bitRate = busspeed;
	
		if ( VERR_WRONG_PARAMETER ==  
			ncdSetChannelParams( m_portHandle, permissionMask, &chipInit ) ) {
			
			ncdClosePort( m_portHandle );
			ncdCloseDriver();
			return false;	
		}

		// Open if passive mode if requested to do so
		if ( m_initFlag & 0x08 ) {
			ncdSetChannelOutput( m_portHandle, m_channelMask,OUTPUT_MODE_SILENT );
		}
		else {
			ncdSetChannelOutput( m_portHandle, m_channelMask,OUTPUT_MODE_NORMAL );
		}

		// Echo
		if ( m_initFlag & 0x04 ) {
			ncdSetChannelMode( m_portHandle, m_channelMask, 1, 0 );
		}
		else {
			ncdSetChannelMode( m_portHandle, m_channelMask, 0, 0 );
		}

		// Should error frames be suppressed
		if ( m_initFlag & 0x10 ) {
			ncdSetReceiveMode( m_portHandle,1, 1 );
		}
		else {
			ncdSetReceiveMode( m_portHandle,0, 1 );
		}
	}

	// set the acceptance filter 
	VsetAcceptance acc;

	// standard
	acc.mask = 0x000; // Open for all
	acc.code = 0x000;
	ncdSetChannelAcceptance( m_portHandle, m_channelMask, &acc );

	// extended
	acc.mask = 0x80000000; // Open for all
	acc.code = 0x80000000;
	ncdSetChannelAcceptance( m_portHandle, m_channelMask, &acc );

	// User mask
	acc.mask = m_Vector_mask; 
	acc.code = m_Vector_filter;
	ncdSetChannelAcceptance( m_portHandle, m_channelMask, &acc );

	// Activate the channel
	ncdActivateChannel( m_portHandle, m_channelMask );

	// Run run run ..... 
	// (otional (for hard fellow rockers) "to the hills..."
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
	UNLOCK_MUTEX( m_vectorMutex );
	UNLOCK_MUTEX( m_receiveMutex );
	UNLOCK_MUTEX( m_transmitMutex );
	

#else // LINUX


	pthread_attr_t thread_attr;
	pthread_attr_init( &thread_attr );
	
	
	// Create the log write thread.
	if ( pthread_create( 	&m_threadId,
								&thread_attr,
								workThreadTransmit,
								this ) ) {	
							
		syslog( LOG_CRIT, "canallogger: Unable to create vector write thread.");
		rv = false;
		fclose( m_flog );
	}


	// Create the log write thread.
	if ( pthread_create( 	&m_threadId,
								&thread_attr,
								workThreadReceive,
								this ) ) {	
							
		syslog( LOG_CRIT, "canallogger: Unable to create vector receive thread.");
		rv = false;
		fclose( m_flog );
	}
		
    // We are open
	m_bOpen = true;

	// Release the mutex
	pthread_mutex_unlock( &m_vectorMutex );

#endif	

	return true;
}

 
//////////////////////////////////////////////////////////////////////
// close
//

bool CVectorObj::close( void )
{	
	// Do nothing if already terminated
	if ( !m_bRun ) return false;
	
	m_bRun = false;

	ncdClosePort( m_portHandle );
	m_portHandle = INVALID_PORTHANDLE;

	ncdCloseDriver();
 
	UNLOCK_MUTEX( m_vectorMutex );
	LOCK_MUTEX( m_vectorMutex );

	
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
	pthread_mutex_destroy( &m_vectorMutex );
	
#endif

	return true;
}


//////////////////////////////////////////////////////////////////////
// doFilter
//

bool CVectorObj::doFilter( canalMsg *pcanalMsg )
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

bool CVectorObj::setFilter( unsigned long filter )
{
	m_filter = filter;
	return true;
}


//////////////////////////////////////////////////////////////////////
// setMask
//

bool CVectorObj::setMask( unsigned long mask )
{
	m_mask = mask;
	return true;
}



//////////////////////////////////////////////////////////////////////
// writeMsg
//

bool CVectorObj::writeMsg( canalMsg *pMsg )
{	
	bool rv = false;
	
	if ( NULL != pMsg ) {

		// Must be room for the message
		if ( m_transmitList.nCount < VECTOR_MAX_SNDMSG ) {

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

bool CVectorObj::readMsg( canalMsg *pMsg )
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

int CVectorObj::dataAvailable( void )
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

bool CVectorObj::getStatistics( PCANALSTATISTICS pCanalStatistics )
{	
	// Must be a valid pointer
	if ( NULL == pCanalStatistics ) return false;

	memcpy( pCanalStatistics, &m_stat, sizeof( canalStatistics ) );

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//	getStatus
//

bool CVectorObj::getStatus( PCANALSTATUS pCanalStatus )
{
	// Must be a valid pointer
	if ( NULL == pCanalStatus ) return false;

	// Request chip state
	ncdRequestChipState( m_portHandle, m_channelMask );

	Sleep( 500 );
 
	// Local echo
	if ( m_initFlag & 0x04 ) {
		m_status.channel_status |= 0x040000;		
	}

	// Passive
	if ( m_initFlag & 0x08 ) {
		m_status.channel_status |= 0x080000;		
	}

	// Detected error frame
	if ( m_initFlag & 0x10 ) {
		m_status.channel_status |= 0x100000;		
	}

	memcpy( pCanalStatus, &m_status, sizeof( canalStatus ) );
 
	return true;
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

	CVectorObj * pobj = ( CVectorObj *)pObject;
	if ( NULL == pobj ) {
#ifdef WIN32	
		ExitThread( errorCode ); // Fail
#else
		pthread_exit( &rv );
#endif
	}
	
	while ( pobj->m_bRun ) {

		LOCK_MUTEX( pobj->m_vectorMutex );
		
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

			VCAN_EVENT event;
			
			event.tag = V_TRANSMIT_MSG;
			
			// id
			event.tagData.msg.id = msg.id;
			if ( msg.flags & CANAL_IDFLAG_EXTENDED ) {
				event.tagData.msg.id |= VCAN_EXT_MSG_ID;	// Extended
			}
		
			// size
			event.tagData.msg.dlc = msg.sizeData;

			// Vector flags
			event.tagData.msg.flags = 0;
			if ( msg.flags & CANAL_IDFLAG_RTR ) {
				event.tagData.msg.flags |= MSGFLAG_REMOTE_FRAME;	// RTR
			}
			
			// Data
			memcpy( event.tagData.msg.data, msg.data, msg.sizeData );

			if ( VERR_QUEUE_IS_FULL != 
					ncdTransmit( pobj->m_portHandle, pobj->m_channelMask, &event ) ) {
					
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
			} // send message
							
		} // while data


		// No data to write

		UNLOCK_MUTEX( pobj->m_vectorMutex );
		SLEEP( 1 );

		//}	 
	
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
	int cntMsg,readMsg;

#ifdef WIN32
	DWORD errorCode = 0;
#else
	int rv = 0;
#endif

	CVectorObj * pobj = ( CVectorObj *)pObject;
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

		LOCK_MUTEX( pobj->m_vectorMutex );

		// get # in receive queue
		ncdGetReceiveQueueLevel( pobj->m_portHandle, &cntMsg );
		if ( cntMsg > 0 ) {
			
			// There are messages to fetch
			VCAN_EVENT *peventarray = new VCAN_EVENT[ cntMsg ];
			if ( NULL != peventarray ) {

				readMsg =  cntMsg;
				if ( VSUCCESS != ncdReceive( pobj->m_portHandle,
												VCAN_POLL,
												0,
												&readMsg,
												peventarray ) ) {
					// Failed
					readMsg = 0;	
				}

				if ( readMsg > 0 ) {
				
					// Write them to the receive buffer
					for ( int i=0; i<readMsg; i++ ) {

						if ( peventarray[ i ].tag == V_CHIP_STATE ) {
							
							pobj->m_status.channel_status = 
								peventarray[ i ].tagData.chipState.txErrorCounter +
								(peventarray[ i ].tagData.chipState.rxErrorCounter << 8);

							if ( peventarray[ i ].tagData.chipState.busStatus & CHIPSTAT_BUSOFF ) {
								pobj->m_status.channel_status |= 0x80000000;		
							}

							if ( peventarray[ i ].tagData.chipState.busStatus & CHIPSTAT_ERROR_PASSIVE ) {
								pobj->m_status.channel_status |= 0x20000000;
							}	            
							
							if ( peventarray[ i ].tagData.chipState.busStatus & CHIPSTAT_ERROR_WARNING ) {
								pobj->m_status.channel_status |= 0x40000000;
							}

							if ( peventarray[ i ].tagData.chipState.busStatus & CHIPSTAT_ERROR_ACTIVE ) {
								pobj->m_status.channel_status |= 0x10000000;
							}
						
							continue;

						}
						else if ( peventarray[ i ].tag == V_TRANSMIT_MSG ) {
							;
						}
						else if ( peventarray[ i ].tag != V_RECEIVE_MSG ) {
							continue;
						}

							
					
						if (  pobj->m_receiveList.nCount < VECTOR_MAX_RCVMSG ) {					
					
							PCANALMSG pMsg	= new canalMsg;
							pMsg->flags = 0;

							// if echo mark as send message
							if ( peventarray[ i ].tag == V_TRANSMIT_MSG ) {
								pMsg->flags |= CANAL_IDFLAG_SEND;
							}

							if ( NULL != pMsg ) {
						
								dllnode *pNode = new dllnode; 
								if ( NULL != pNode ) {
							
									pMsg->timestamp = peventarray[ i ].timeStamp;
									pMsg->id = peventarray[ i ].tagData.msg.id;
									
									if ( pMsg->id & CANAL_IDFLAG_EXTENDED ) {
										pMsg->id &= 0x7fffffff;
										pMsg->flags |= CANAL_IDFLAG_EXTENDED;
									}

									pMsg->sizeData = peventarray[ i ].tagData.msg.dlc;		
									memcpy( pMsg->data, 
												peventarray[ i ].tagData.msg.data, 
												pMsg->sizeData ); 
									

									// Check for RTS package
									if ( peventarray[ i ].tagData.msg.flags & MSGFLAG_REMOTE_FRAME ) {
										pMsg->flags |= CANAL_IDFLAG_RTR;
									}
									
									// Check for overrun error
									if ( peventarray[ i ].tagData.msg.flags & MSGFLAG_OVERRUN ) {
										pobj->m_stat.cntOverruns++;
									}

									// Check for error frame
									if ( peventarray[ i ].tagData.msg.flags & MSGFLAG_ERROR_FRAME ) {
										pMsg->flags |= CANAL_IDFLAG_STATUS;
									}
									
									pNode->pObject = pMsg;
									LOCK_MUTEX( pobj->m_receiveMutex );
									dll_addNode( &pobj->m_receiveList, pNode );
									UNLOCK_MUTEX( pobj->m_receiveMutex );

									// Update statistics
									pobj->m_stat.cntReceiveData += pMsg->sizeData;
									pobj->m_stat.cntReceiveFrames += 1;

								}
								else {

									delete pMsg;

								}
							}				
						} 
						else {
							// Full buffer
							pobj->m_stat.cntOverruns++;	
						}
					}
				}
		
				delete[] peventarray;
			}
		}

		UNLOCK_MUTEX( pobj->m_vectorMutex );
		SLEEP( 1 );
	
	} // while 	 


#ifdef WIN32
	ExitThread( errorCode );
#else
	pthread_exit( &rv );
#endif

}
