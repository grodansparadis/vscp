// apoxobj.cpp:  
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2004 Apox Controls
// Copyright (C) 2000-2012 Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
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
// $RCSfile: apoxobj.cpp,v $                                       
// $Date: 2005/06/09 06:47:44 $                                  
// $Author: akhe $                                              
// $Revision: 1.4 $ 

#include "stdio.h"
#include "apoxobj.h"
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
// CApoxObj
//

CApoxObj::CApoxObj()
{ 
	m_initFlag = 0;
	
	// No filter mask
	m_filter = 0;
	m_mask = 0;

	m_bRun = false;

#ifdef WIN32
	
	m_hTreadReceive = 0;
	m_hTreadTransmit = 0;

	// Create the device AND LIST access mutexes
	m_apoxMutex = CreateMutex( NULL, true, CANAL_DLL_APOXDRV_OBJ_MUTEX );
	m_receiveMutex = CreateMutex( NULL, true, CANAL_DLL_APOXDRV_RECEIVE_MUTEX );
	m_transmitMutex = CreateMutex( NULL, true, CANAL_DLL_APOXDRV_TRANSMIT_MUTEX );
	m_responseMutex = CreateMutex( NULL, true, CANAL_DLL_APOXDRV_RESPONSE_MUTEX );

#else
	
	m_flog = NULL;
	pthread_mutex_init( &m_apoxMutex, NULL );
	pthread_mutex_init( &m_receiveMutex, NULL );
	pthread_mutex_init( &m_transmitMutex, NULL );
	pthread_mutex_init( &m_responseMutex, NULL );

#endif

	dll_init( &m_transmitList, SORT_NONE );
	dll_init( &m_receiveList, SORT_NONE );
	dll_init( &m_responseList, SORT_NONE );
	
}

//////////////////////////////////////////////////////////////////////
// ~CApoxObj
//

CApoxObj::~CApoxObj()
{		 
	close();
	
	LOCK_MUTEX( m_transmitMutex );
	dll_removeAllNodes( &m_transmitList );
	
	LOCK_MUTEX( m_receiveMutex );
	dll_removeAllNodes( &m_receiveList );

	LOCK_MUTEX( m_responseMutex );
	dll_removeAllNodes( &m_responseList );


#ifdef WIN32	
	
	if ( NULL != m_apoxMutex ) CloseHandle( m_apoxMutex );	
	if ( NULL != m_receiveMutex ) CloseHandle( m_receiveMutex );
	if ( NULL != m_transmitMutex ) CloseHandle( m_transmitMutex );
	if ( NULL != m_responseMutex ) CloseHandle( m_responseMutex );

#else

	pthread_mutex_destroy( &m_apoxMutex );
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
// "serial;bus-speed"
//
// serial
// ======
// Serial number for channel
//
// bus-speed
// =========
// One of the predefined bitrates can be set here
// 
//   125 for  125 Kbs
//   250 for  250 Kbs
//   500 for  500 Kbs
//  1000 for 1000 Mbs
//
// 
// flags 
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

bool CApoxObj::open( const char *szFileName, unsigned long flags )
{
	const char *p;
	unsigned long busspeed = 125;
	m_emergencyInfo = 0;
	char szDrvParams[ MAX_PATH ];
	m_initFlag = flags;

	m_RxMsgState = USB_IDLE;

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
		
	// serial
	p = strtok( szDrvParams, ";" );
	if ( NULL != p ) {		
		strcpy( m_SerialNumber, p );	
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

	// Handle busspeed
	uint8_t nSpeed = CAN_BAUD_1000;
	switch ( busspeed ) {

		case 125:
			nSpeed = CAN_BAUD_125;
			break;

		case 250:
			nSpeed = CAN_BAUD_250;
			break;

		case 500:
			nSpeed = CAN_BAUD_500;
			break;

		case 1000:
			nSpeed = CAN_BAUD_1000;
			break;

		default:
			nSpeed = CAN_BAUD_125;
			break;

	}


	FT_STATUS ftStatus;
	ftStatus = FT_OpenEx( (void *)m_SerialNumber, 
							FT_OPEN_BY_SERIAL_NUMBER, 
							&m_ftHandle );
	if ( !FT_SUCCESS( ftStatus ) ) return false;
		
	ftStatus = FT_ResetDevice( m_ftHandle );
	ftStatus = FT_Purge( m_ftHandle, FT_PURGE_RX | FT_PURGE_TX );
	ftStatus = FT_SetTimeouts(m_ftHandle, 378, 128);
	ftStatus = FT_SetUSBParameters ( m_ftHandle, 2048, 2048 );
	ftStatus = FT_SetLatencyTimer( m_ftHandle, 3 );
    
	
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
	UNLOCK_MUTEX( m_apoxMutex );
	UNLOCK_MUTEX( m_receiveMutex );
	UNLOCK_MUTEX( m_transmitMutex );
	UNLOCK_MUTEX( m_responseMutex );

#else // LINUX


	pthread_attr_t thread_attr;
	pthread_attr_init( &thread_attr );
	
	
	// Create the log write thread.
	if ( pthread_create( 	&m_threadId,
								&thread_attr,
								workThreadTransmit,
								this ) ) {	
							
		syslog( LOG_CRIT, "canallogger: Unable to create apoxdrv write thread.");
		rv = false;
		fclose( m_flog );
	}


	// Create the log write thread.
	if ( pthread_create( 	&m_threadId,
								&thread_attr,
								workThreadReceive,
								this ) ) {	
							
		syslog( LOG_CRIT, "canallogger: Unable to create apoxdrv receive thread.");
		rv = false;
		fclose( m_flog );
	}
		
    // We are open
	m_bOpen = true;

	// Release the mutex
	pthread_mutex_unlock( &m_apoxMutex );

#endif	

	// Switch to main mode if in boot mode
	if ( RUNMODE_BOOT == getAdapterRunMode() ) {
		setAdapterRunMode( RUNMODE_MAIN );
	}

	// Set baudrate
	setBaudrate( nSpeed );

	// Set initial filter/mask	
	switch( m_initFlag & 0x03 ) {

		case 0:
		case 3:
			m_filtermask.rx_buff1_ext = 0;
			m_filtermask.rx_buff2_ext = 1;
			break;

		case 1:
			m_filtermask.rx_buff1_ext = 0;
			m_filtermask.rx_buff2_ext = 0;
			break;

		case 2:
			m_filtermask.rx_buff1_ext = 1;
			m_filtermask.rx_buff2_ext = 1;
			break;
	}

	m_filtermask.RXM0 = 0;
	m_filtermask.RXM1 = 0;
	m_filtermask.RXF0 = 0;	
	m_filtermask.RXF1 = 0;
	m_filtermask.RXF2 = 0;
	m_filtermask.RXF3 = 0;
	m_filtermask.RXF4 = 0;
	m_filtermask.RXF5 = 0;

	setAdapterFilterMask( &m_filtermask );

	// Set transmission mode
	short trmode = ( (short)( m_initFlag & 0x0c ) >> 2 );
	if ( 0 == trmode ) trmode = TRMODE_NORMAL;
	setTransmissionMode( trmode );

	return true;
}

 
//////////////////////////////////////////////////////////////////////
// close
//

bool CApoxObj::close( void )
{	
	// Do nothing if already terminated
	if ( !m_bRun ) return false;
	
	m_bRun = false;

	// Switch to boot mode
	setAdapterRunMode( RUNMODE_BOOT );

	FT_Close( m_ftHandle );
 
	UNLOCK_MUTEX( m_apoxMutex );
	LOCK_MUTEX( m_apoxMutex );

	
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
	pthread_mutex_destroy( &m_apoxMutex );
	
#endif

	return true;
}


//////////////////////////////////////////////////////////////////////
// doFilter
//

bool CApoxObj::doFilter( canalMsg *pcanalMsg )
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

bool CApoxObj::setFilter( unsigned long filter )
{
	m_filter = filter;
	return true;
}


//////////////////////////////////////////////////////////////////////
// setMask
//

bool CApoxObj::setMask( unsigned long mask )
{
	m_mask = mask;
	return true;
}



//////////////////////////////////////////////////////////////////////
// writeMsg
//

bool CApoxObj::writeMsg( canalMsg *pMsg )
{	
	bool rv = false;
	
	if ( NULL != pMsg ) {

		// Must be room for the message
		if ( m_transmitList.nCount < APOX_MAX_SNDMSG ) {

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

bool CApoxObj::readMsg( canalMsg *pMsg )
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

int CApoxObj::dataAvailable( void )
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

bool CApoxObj::getStatistics( PCANALSTATISTICS pCanalStatistics )
{	
	// Must be a valid pointer
	if ( NULL == pCanalStatistics ) return false;

	memcpy( pCanalStatistics, &m_stat, sizeof( canalStatistics ) );

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//	getStatus
//

bool CApoxObj::getStatus( PCANALSTATUS pCanalStatus )
{
	// Must be a valid pointer
	if ( NULL == pCanalStatus ) return false;

	m_status.channel_status = ( m_emergencyInfo << 16 );

	if ( m_emergencyInfo & EMERGENCY_BUS_OFF ) {
		m_status.channel_status  |= 0x80000000;	
	}

	if ( ( m_emergencyInfo & EMERGENCY_TX_WARNING ) ||
			( m_emergencyInfo & EMERGENCY_RCV_WARNING ) ) {
		m_status.channel_status  |= 0x40000000;	
	}

	if ( ( m_emergencyInfo & EMERGENCY_TXBUS_PASSIVE ) ||
			( m_emergencyInfo & EMERGENCY_RXBUS_PASSIVE ) ) {
		m_status.channel_status  |= 0x20000000;	
	}

	// zero emergency info
	m_emergencyInfo = 0;

	// TX Error counter
	m_status.channel_status += getTxErrorCount();

	// RX Error counter
	m_status.channel_status += ( getRxErrorCount() << 8 );

	memcpy( pCanalStatus, &m_status, sizeof( canalStatus ) );

	return true;
}


///////////////////////////////////////////////////////////////////////////////
// sendCommand
//
//

uint8_t CApoxObj::sendCommand( uint8_t cmdcode, uint8_t *pData, uint16_t dataSize )
{
	uint8_t txBuf[300];   
	short size = 0;
	uint8_t status;
	
	txBuf[ size++ ] = 0x00;
	txBuf[ size++ ] = cmdcode | 0x80;

	
	if ( NULL != pData ) {
		for ( uint16_t i=0; i<dataSize; i++ ) {
			txBuf[ size++ ] = pData[ i ];
		}
	}

	status = sendUSBMsg( txBuf, size );

	return status;
}


///////////////////////////////////////////////////////////////////////////////
// sendUSBMsg
//
//

uint8_t CApoxObj::sendUSBMsg( uint8_t *data, short dataSize ) 
{
	BYTE	buffer[ 300 ];
	DWORD	retLen;
	DWORD	size = 0;
	BYTE	chksum = 0;
	uint8_t		rv;

	// BYTE STUFFING
	//----------------------------------------------
	// message format
	// DLE STX data CSUM DLE ETX
	// but if data contains a DLE
	// then insert another DLE before it (BYTE Stuffing)
	// this function is not concerned with the data format
	// other than the fact that it adds a checksum to the end

	// start the transmission
	buffer[ size++ ] = USB_DLE;
	buffer[ size++ ] = USB_STX;  // start transmission

	// BYTE Stuff the data and calc checksum
	for ( int i=0; i<dataSize; i++ ) {
		chksum ^= data[i];

		if (data[i] == USB_DLE) {
			buffer[ size++ ] = USB_DLE;
		}

		buffer[size++] = data[i];
	}

	// BYTE STUFF checksum if necessary
	if (chksum == USB_DLE ) {
		buffer[size++] = USB_DLE;
	}

	// Send the check sum
	buffer[size++] = chksum;

	// terminate the transmission
	buffer[size++] = USB_DLE;
	buffer[size++] = USB_ETX;  // end transmission
	
	if ( m_ftHandle ) {
		
		LOCK_MUTEX( m_apoxMutex );
		if ( FT_OK == FT_Write( m_ftHandle, buffer, size, &retLen ) ) { 
			rv = USB_OK;
		}
		else {
			rv = USB_CANNOT_TRANSMIT;
		}
		UNLOCK_MUTEX( m_apoxMutex );
	}
	else {
		rv = USB_ERR_NO_USB;
	}
	
	
	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// waitResponse
//
//

bool CApoxObj::waitResponse( responseMsg *pMsg, uint32_t timeout )
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

bool CApoxObj::sendCommandWait( uint8_t cmdcode, responseMsg *pMsg, uint32_t timeout )
{
	// Send the command
	if ( USB_OK != sendCommand( cmdcode ) ) return false;	

	return waitResponse( pMsg, timeout );
}


///////////////////////////////////////////////////////////////////////////////
// setBaudrate
//
//

bool CApoxObj::setBaudrate( uint8_t rate )
{
	uint8_t command;

	switch (rate)
	{
		case 0:
			command = SET_BAUD_1MEG;		
			break;

		case 1:
			command = SET_BAUD_500K;		
			break;

		case 2:
			command = SET_BAUD_250K;		
			break;

		case 3:
			command = SET_BAUD_125K;		
			break;
	}

	responseMsg msg;
	if ( !sendCommandWait( command, &msg, 1000 ) ) return false;

	if ( msg.command != ( command | 0x80 ) ) return false;

	return true;
}


///////////////////////////////////////////////////////////////////////////////
// getAdapterRunMode
//
//

int CApoxObj::getAdapterRunMode( void )
{
	responseMsg msg;
	if ( !sendCommandWait( WHICH_CODE_IS_RUNNING, &msg, 1000 ) ) return -1;

	if ( 0xcc == msg.data[ 0 ] ) return RUNMODE_MAIN;	// Main code
	if ( 0x55 == msg.data[0] ) return RUNMODE_BOOT;		// Boot code

	return -1;		
}


///////////////////////////////////////////////////////////////////////////////
// setAdapterRunMode
//
//

bool CApoxObj::setAdapterRunMode( short nRunMode )
{
	if ( RUNMODE_BOOT == nRunMode ) {		
		if ( USB_OK != resetAdapter() ) return false;
	}
	else if ( RUNMODE_MAIN == nRunMode  ) {
		if ( USB_OK != sendCommand('R') ) return false;
	}
	else {
		return false;
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////
// resetAdapter
//
//

bool CApoxObj::resetAdapter( void )
{
	if ( USB_OK == sendCommand( RESET_MICRO ) ) {
		return true;
	}
	else {
		return false;
	}
}


///////////////////////////////////////////////////////////////////////////////
// setTransmissionMode
//
//

bool CApoxObj::setTransmissionMode( short nMode )
{
	switch ( nMode ) {
	
		case TRMODE_CONFIG:
			if ( USB_OK != sendCommand( SET_CONFIG_MODE ) ) {
				return false;
			}
			break;

		case TRMODE_LISTEN:
			if ( USB_OK != sendCommand( SET_LISTEN_MODE ) ) {
				return false;
			}
			break;
	
		case TRMODE_LOOPBACK:
			if ( USB_OK != sendCommand( SET_LOOPBACK_MODE ) ) {
				return false;
			}
			break;

		case TRMODE_DISABLED:
			if ( USB_OK != sendCommand( SET_SLEEP_MODE ) ) {
				return false;
			}
			break;

		case TRMODE_NORMAL:
			if ( USB_OK != sendCommand( SET_NORMAL_MODE ) ) {
				return false;
			}
			break;
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////
// getTxErrorCount
//
//

short CApoxObj::getTxErrorCount( void )
{
	responseMsg msg;
	if ( USB_OK != sendCommandWait( GET_TX_ERR_CNT, &msg, 1000 ) ) {
		return -1;
	}

	return msg.data[ 0 ];
}


///////////////////////////////////////////////////////////////////////////////
// getRxErrorCount
//
//

short CApoxObj::getRxErrorCount( void )
{
	responseMsg msg;
	if ( USB_OK != sendCommandWait( GET_RX_ERR_CNT, &msg, 1000 ) ) {
		return -1;
	}

	return msg.data[ 0 ];
}

///////////////////////////////////////////////////////////////////////////////
// getAdapterMode
//
//

short CApoxObj::getAdapterCANSTAT( void )
{
	responseMsg msg;
	if ( USB_OK != sendCommandWait( GET_CANSTAT, &msg, 1000 ) ) {
		return -1;
	}

	return msg.data[ 0 ];
}


///////////////////////////////////////////////////////////////////////////////
// getAdapterStatus
//
//

short CApoxObj::getAdapterCOMSTAT( void )
{
	responseMsg msg;
	if ( USB_OK != sendCommandWait( GET_COMSTAT, &msg, 1000 ) ) {
		return -1;
	}

	return msg.data[ 0 ];
}


///////////////////////////////////////////////////////////////////////////////
// abortTransmission
//
//

bool CApoxObj::abortTransmission( void )
{
	if ( USB_OK != sendCommand( ABORT_ALL_TX ) ) {
		return false;
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////
// setAdapterFilterMask
//
//

bool CApoxObj::setAdapterFilterMask( structFilterMask *pFilterMask )
{
  uint8_t txdata[ 100 ];
  short size = 0;

	// Must be in confguration mode
	if ( !setTransmissionMode( TRMODE_CONFIG ) ) return false;

	if ( pFilterMask->rx_buff1_ext ) {
		pFilterMask->RXM0 |= 0x80000000;
		pFilterMask->RXM1 |= 0x80000000;
		pFilterMask->RXF0 |= 0x80000000;
		pFilterMask->RXF1 |= 0x80000000;
	}

	//txdata[size++] = 0x00;
	//txdata[size++] = ( SET_MSGFILTER1 ) | 0x80;
  txdata[size++] = (uint8_t)( pFilterMask->RXM0 >> 24 );
	txdata[size++] = (uint8_t)( pFilterMask->RXM0 >> 16 );
	txdata[size++] = (uint8_t)( pFilterMask->RXM0 >> 8 );
	txdata[size++] = (uint8_t)  pFilterMask->RXM0;

	txdata[size++] = (uint8_t)( pFilterMask->RXM1 >> 24 ); 
	txdata[size++] = (uint8_t)( pFilterMask->RXM1 >> 16 );
	txdata[size++] = (uint8_t)( pFilterMask->RXM1 >> 8 );
	txdata[size++] = (uint8_t)  pFilterMask->RXM1;
	
	txdata[size++] = (uint8_t)( pFilterMask->RXF0 >> 24 ); 
	txdata[size++] = (uint8_t)( pFilterMask->RXF0 >> 16 );
	txdata[size++] = (uint8_t)( pFilterMask->RXF0 >> 8 );	
	txdata[size++] = (uint8_t)  pFilterMask->RXF0;	
	
	txdata[size++] = (uint8_t)( pFilterMask->RXF1 >> 24 ); 
	txdata[size++] = (uint8_t)( pFilterMask->RXF1 >> 16 );
	txdata[size++] = (uint8_t)( pFilterMask->RXF1 >> 8 );
	txdata[size++] = (uint8_t)  pFilterMask->RXF1;

	if ( USB_OK != sendCommand( SET_MSGFILTER1, txdata, size ) ) {
		return false;
	}

	
	if ( pFilterMask->rx_buff2_ext ) {
		pFilterMask->RXF2 |= 0x80000000;
		pFilterMask->RXF3 |= 0x80000000;
		pFilterMask->RXF4 |= 0x80000000;
		pFilterMask->RXF5 |= 0x80000000;
	}

	size = 0;	
	//txdata[size++] = 0x00;
	//txdata[size++] = ( SET_MSGFILTER2 ) | 0x80; 
	
	txdata[size++] = (uint8_t)( pFilterMask->RXF2 >> 24 ); 
	txdata[size++] = (uint8_t)( pFilterMask->RXF2 >> 16 );
	txdata[size++] = (uint8_t)( pFilterMask->RXF2 >> 8 );
	txdata[size++] = (uint8_t)  pFilterMask->RXF2;

	txdata[size++] = (uint8_t)( pFilterMask->RXF3 >> 24 ); 
	txdata[size++] = (uint8_t)( pFilterMask->RXF3 >> 16 );
	txdata[size++] = (uint8_t)( pFilterMask->RXF3 >> 8 );
	txdata[size++] = (uint8_t)  pFilterMask->RXF3;
	
	txdata[size++] = (uint8_t)( pFilterMask->RXF4 >> 24 ); 
	txdata[size++] = (uint8_t)( pFilterMask->RXF4 >> 16 );
	txdata[size++] = (uint8_t)( pFilterMask->RXF4 >> 8 );
	txdata[size++] = (uint8_t)  pFilterMask->RXF4;
	
	txdata[size++] = (uint8_t)( pFilterMask->RXF5 >> 24 ); 
	txdata[size++] = (uint8_t)( pFilterMask->RXF5 >> 16 );
	txdata[size++] = (uint8_t)( pFilterMask->RXF5 >> 8 );
	txdata[size++] = (uint8_t)  pFilterMask->RXF5;

	if ( USB_OK != sendCommand( SET_MSGFILTER2, txdata, size ) ) {
		return false;
	}

	return true;
}




///////////////////////////////////////////////////////////////////////////////
// USBProcessByte
//
//
 
uint8_t CApoxObj::processUSBByte( uint8_t inByte, uint32_t *RxMsgLen, uint8_t *RxMsgData )
{   
	static unsigned char chksum;
	uint8_t status = USB_OK;

	switch ( m_RxMsgState ) {

		case USB_IDLE:

			if ( USB_DLE == inByte ) {
				m_RxMsgState = USB_FRAME_START;
				chksum = 0;
				*RxMsgLen = 0;
			}
			else {
				status = USB_EXPECTING_DLE;
				m_RxMsgState = USB_IDLE;
			}
			break
			;
		case USB_FRAME_START:

			if ( USB_STX == inByte ) {
				m_RxMsgState = USB_FRAME;
			}
			else {
				status = USB_EXPECTING_STX;
				m_RxMsgState = USB_IDLE;
			}
			break;

		case USB_FRAME:

			if ( USB_DLE == inByte ) {
				m_RxMsgState = USB_FRAME_NLE;
			}
			else {
				RxMsgData[(*RxMsgLen)++] = inByte;
				chksum ^= inByte;
			}
			break;

		case USB_FRAME_NLE:

			if ( USB_ETX == inByte ) {

				if ( !chksum ) { // checksum should be zero if message was good
					m_RxMsgState = USB_MESSAGE_COMPLETE;
					(*RxMsgLen)--; // we are always 1 ahead
				}
				else {
 					status = USB_BAD_CHECKSUM;
					m_RxMsgState = USB_IDLE;
				}
			}
			else if (USB_STX == inByte ) {

				status = USB_EXPECTING_ETX;    // Not expecting a NLE-STX combination
				m_RxMsgState = USB_IDLE;

			}
			else {

				RxMsgData[(*RxMsgLen)++] = inByte;
				chksum ^= inByte;
				m_RxMsgState = USB_FRAME;

			}
			break;

		case  USB_MESSAGE_COMPLETE:
		default:
			status = USB_BAD_STATE; // should never get here
			m_RxMsgState = USB_IDLE;
			break;

	}  // end switch
	
	return status;
}


///////////////////////////////////////////////////////////////////////////////
// ReadUSBData
//
//

bool CApoxObj::readUSBData( void )
{
	bool bData = false;
	DWORD eventStatus;
	DWORD nRxCnt;	// Number of characters in receive queue
	DWORD nTxCnt;	// Number of characters in transmit queue
	DWORD nRcvCnt;	

	if ( FT_OK == FT_GetStatus( m_ftHandle, &nRxCnt, &nTxCnt, &eventStatus ) ) {
	
		// If there are characters to receive
		if ( nRxCnt ) {
		
			if ( nRxCnt > sizeof( m_bufferRx ) ) {
				nRxCnt = sizeof( m_bufferRx );
			}
			
			if ( ( FT_OK == 
					FT_Read( m_ftHandle, m_bufferRx, nRxCnt, &nRcvCnt ) ) &&
								( nRcvCnt > 0 ) ) {
			
				for ( uint32_t i=0; i<nRcvCnt; i++ ) {
					
					if ( USB_OK == processUSBByte( m_bufferRx[ i ], 
														              &m_lengthMsgRcv, 
														              m_bufferMsgRcv ) ) {

						if ( USB_MESSAGE_COMPLETE == m_RxMsgState ) {
						
							// A message
							
							if ( 0x00 == m_bufferMsgRcv[ 0 ] ) {
								
								// Response to control message from adapter
								// ----------------------------
								// [0] 0x00
								// [1] command | 0x80
								// [2..n] response data

								if (  m_responseList.nCount < APOX_MAX_RESPONSEMSG ) {					
							
									responseMsg *pMsg = new responseMsg;
									if ( NULL != pMsg ) {
									
										dllnode *pNode = new dllnode; 
										if ( NULL != pNode ) {

											pMsg->id      = 0x00;
											pMsg->command = m_bufferMsgRcv[ 1 ] & 0x7F;									
											pMsg->len = (uint8_t)(m_lengthMsgRcv - 2); //(minus two for the first two bytes)
											memcpy( pMsg->data, ( m_bufferMsgRcv + 2 ), pMsg->len );
									
									
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
							}
							
							else if ( 0xff == m_bufferMsgRcv[ 0 ] ) {
							
								// Unsolicited emergency message from adapter
								// ----------------------------
								// [0] 0xFF
								// [1] 0x80
								// [2] ERRORCODE (0-255)

								switch( m_bufferMsgRcv[ 2 ] ) { 
								
								case CAN_RCV_OVERFLOW:
										m_stat.cntOverruns++;
										m_emergencyInfo |= EMERGENCY_OVERFLOW;
										break;

									case CAN_RCV_WARNING:
										m_emergencyInfo |= EMERGENCY_RCV_WARNING;
										break;

									case CAN_TX_WARNING:
										m_emergencyInfo |= EMERGENCY_TX_WARNING;
										break;

									case CAN_RX_BUS_PASSIVE:
										m_emergencyInfo |= EMERGENCY_TXBUS_PASSIVE;
										break;
	
									case CAN_TX_BUS_PASSIVE:
										m_emergencyInfo |= EMERGENCY_RXBUS_PASSIVE;
										break;

									case CAN_BUS_OFF:
										m_emergencyInfo |= EMERGENCY_BUS_OFF;
										break;
								}

								
							}
							
							else {

								// CAN message	
								// ----------------------------
								// 0:[who|rtr|idMode|(unused)]
								// 1:[ID MSB]
								// 2:[ID3]
								// 3:[ID2]
								// 4:[ID LSB]
								// 5:[TIMESTAMP MSB]
								// 6:[TIMESTAMP LSB]
								// 7:[RX_FLAGS]
								// 8:[LENGTH]
								// 9-16:[DATA 0-8 bytes]

								if (  m_receiveList.nCount < APOX_MAX_RCVMSG ) {					
					
									PCANALMSG pMsg	= new canalMsg;
									pMsg->flags = 0;

									if ( NULL != pMsg ) {
						
										dllnode *pNode = new dllnode; 
										if ( NULL != pNode ) {
							
											pMsg->timestamp = 
												(((DWORD)m_bufferMsgRcv[8]<<24 ) & 0xff000000) |
												(((DWORD)m_bufferMsgRcv[7]<<16 ) & 0x00ff0000) |
												(((DWORD)m_bufferMsgRcv[6]<<8 )  & 0x0000ff00) |
												(((DWORD)m_bufferMsgRcv[5]    )  & 0x000000ff) ;

											pMsg->id = 
												(((DWORD)m_bufferMsgRcv[4]<<24) & 0x1f000000) |
												(((DWORD)m_bufferMsgRcv[3]<<16) & 0x00ff0000) |
												(((DWORD)m_bufferMsgRcv[2]<<8 ) & 0x0000ff00) |
												(((DWORD)m_bufferMsgRcv[1]    ) & 0x000000ff) ;
											
											pMsg->sizeData = m_bufferMsgRcv[10];		
											
											memcpy( (void *)pMsg->data, (m_bufferMsgRcv + 11 ), pMsg->sizeData ); 
									
											// If extended set extended flag
											if ( m_bufferMsgRcv[0] & 0x20 ) pMsg->flags |= CANAL_IDFLAG_EXTENDED;

											// Check for RTR package
											if ( (m_bufferMsgRcv[0] & 0x40) ) pMsg->flags |= CANAL_IDFLAG_RTR;
									
											pNode->pObject = pMsg;
											LOCK_MUTEX( m_receiveMutex );
											dll_addNode( &m_receiveList, pNode );
											UNLOCK_MUTEX( m_receiveMutex );

											// Update statistics
											m_stat.cntReceiveData += pMsg->sizeData;
											m_stat.cntReceiveFrames += 1;

										}
										else {

											delete pMsg;

										}
									}				
								} 
								else {
									// Full buffer
									m_stat.cntOverruns++;	
								}

							} // recv types

							m_RxMsgState = USB_IDLE; // reset state for next msg

						} // Complete msg
					}
				} // byte loop
			} // read OK
		} // Bytes to read
	}

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

	CApoxObj * pobj = ( CApoxObj *)pObject;
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
			// [0] ([1][RTR][EXT][unused 0..4])
			// [1] ID MSB
			// [2] ID
			// [3] ID
			// [4] ID LSB
			// [5] FUTURE USE (CANopen or DeviceNet) // ex. Wait for response? Etc..
			// [6] FUTURE USE (CANopen or DeviceNet)
			// [7] RESERVED FOR TX FLAGS 
			// [8] DATA LEN (0-8)
			// [9-16] 

			uint8_t sendData[ 20 ];
			short size = 0;

			sendData[ size++ ] = 
				0x80 | 
				( ( msg.flags & CANAL_IDFLAG_RTR ) ? 0x40:0x00) | 
				( ( msg.flags & CANAL_IDFLAG_EXTENDED ) ? 0x20 : 0x00 );
			sendData[ size++ ] = ( uint8_t )( msg.id >> 24 ) & 0x1f;
			sendData[ size++ ] = ( uint8_t )( msg.id >> 16 ) & 0xff;
			sendData[ size++ ] = ( uint8_t )( msg.id >> 8 )  & 0xff;
			sendData[ size++ ] = ( uint8_t )( msg.id ) & 0xff;

			sendData[size++] = 0x00; // future use
			sendData[size++] = 0x00; // future use

			sendData[size++] = 0; // txFlags;
			sendData[size++] = msg.sizeData;

			memcpy( sendData, msg.data, msg.sizeData );
			size += msg.sizeData;
			 
			LOCK_MUTEX( pobj->m_apoxMutex );

			if ( USB_OK == pobj->sendUSBMsg( sendData, size ) ) {
	
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

			UNLOCK_MUTEX( pobj->m_apoxMutex );
										
		} // while data


		// No data to write

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
	bool bData;

#ifdef WIN32
	DWORD errorCode = 0;
#else
	int rv = 0;
#endif

	CApoxObj * pobj = ( CApoxObj *)pObject;
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

		LOCK_MUTEX( pobj->m_apoxMutex );
		bData = pobj->readUSBData();
		UNLOCK_MUTEX( pobj->m_apoxMutex );
		if ( !bData) SLEEP( 1 );	// Sleep if no data
	
	} // while 	 


#ifdef WIN32
	ExitThread( errorCode );
#else
	pthread_exit( &rv );
#endif

}