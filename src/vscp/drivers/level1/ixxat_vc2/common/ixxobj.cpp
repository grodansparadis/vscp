// ixxatvci.cpp:  
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
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
// $RCSfile: ixxobj.cpp,v $                                       
// $Date: 2005/02/16 13:17:54 $                                  
// $Author: akhe $                                              
// $Revision: 1.11 $ 

#include "stdio.h"
#include "ixxobj.h"
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
// CIxxObj
//

CIxxObj::CIxxObj()
{ 
	
	m_channel = 0;
	m_initFlag = 0;
	m_dwBrdType = VCI_UNKNOWN;
	m_dwBrdTypeRelatedIndex = 0;
	
	// No filter mask
	m_filter = 0;
	m_mask = 0;

	m_bRun = false;

#ifdef WIN32
	
	m_hTreadReceive = 0;
	m_hTreadTransmit = 0;

	// Create the device AND LIST access mutexes
	m_ixxMutex = CreateMutex( NULL, true, CANAL_DLL_IXXATVCIDRV_OBJ_MUTEX );
	m_receiveMutex = CreateMutex( NULL, true, CANAL_DLL_IXXATVCIDRV_RECEIVE_MUTEX );
	m_transmitMutex = CreateMutex( NULL, true, CANAL_DLL_IXXATVCIDRV_TRANSMIT_MUTEX );

#else
	
	m_flog = NULL;
	pthread_mutex_init( &m_ixxMutex, NULL );
	pthread_mutex_init( &m_receiveMutex, NULL );
	pthread_mutex_init( &m_transmitMutex, NULL );

#endif

	dll_init( &m_transmitList, SORT_NONE );
	dll_init( &m_receiveList, SORT_NONE );
}

//////////////////////////////////////////////////////////////////////
// ~CIxxObj
//

CIxxObj::~CIxxObj()
{		 
	close();
	
	LOCK_MUTEX( m_transmitMutex );
	dll_removeAllNodes( &m_transmitList );
	
	LOCK_MUTEX( m_receiveMutex );
	dll_removeAllNodes( &m_receiveList );


#ifdef WIN32	
	
	if ( NULL != m_ixxMutex ) CloseHandle( m_ixxMutex );	
	if ( NULL != m_receiveMutex ) CloseHandle( m_receiveMutex );
	if ( NULL != m_transmitMutex ) CloseHandle( m_transmitMutex );

#else

	pthread_mutex_destroy( &m_ixxMutex );
	pthread_mutex_destroy( &m_receiveMutex );
	pthread_mutex_destroy( &m_transmitMutex );

#endif
}


struct __ixxatsymtbl {
	char symname[ 32 ];
	int id;
} ixxatsymtbl[] = {
		"VCI_IPCI165",			0,
		"VCI_IPCI320",			1,
		"VCI_CANDY",			2,
		"VCI_PCMCIA",			3,
		"VCI_IPCI386",			5,
		"VCI_IPCI165_PCI",		6,
		"VCI_IPCI320_PCI",		7,
		"VCI_CP350_PCI",		8,
		"VCI_PMC250_PCI",		9,
		"VCI_USB2CAN",			10,
		"VCI_CANDYLITE",		11,
		"VCI_CANANET",			12,
		"VCI_BFCARD",			13,
		"VCI_PCI04_PCI",		14,
		"VCI_USB2CAN_COMPACT",	15,
		"VCI_PASSIV",			50,
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
// "board;channel;filter;mask;bus-speed;btr0;btr1"
//
// board
// =====
//
// board can be on a symbolic for or as a number as follows
//
// VCI_IPCI165			or  0  for iPC-I 165, ISA slot
// VCI_IPCI320			or  1  for iPC-I 320, ISA slot
// VCI_CANDY			or  2  for CANdy320, LPT port
// VCI_PCMCIA			or  3  for tinCAN, pc card
// VCI_IPCI386			or  5 for iPC-I 386, ISA slot
// VCI_IPCI165_PCI		or  6 for iPC-I 165, PCI slot
// VCI_IPCI320_PCI		or  7 for iPC-I 320, PCI slot
// VCI_CP350_PCI		or  8 for iPC-I 165, PCI slot
// VCI_PMC250_PCI		or  9 for special hardware from PEP
// VCI_USB2CAN			or  10 for USB2CAN, USB port
// VCI_CANDYLITE		or  11 for CANdy lite, LPT port
// VCI_CANANET			or  12 for CAN@net, ethernet
// VCI_BFCARD			or  13 for byteflight Card, pc card
// VCI_PCI04_PCI		or	14 for PC-I 04, PCI slot, passive
// VCI_USB2CAN_COMPACT  or  15 for USB2CAN compact, USB port
// VCI_PASSIV           or  50 for PC-I 03, ISA slot, passive
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
//  10 for 10 Kbs
//  20 for 20 Kbs
//	50 for 50 Kbs
//  100 for 100 Kbs
//  125 for 125 Kbs
//  250 for 250 Kbs
//  500 for 500 Kbs
//  800 for 800 Kbs
//  1000 for 1000 Mbs
//
// btr0
// ====
// Value for bitrate register 0. If btr value pairs are given then the bus-speed
// parameter is ignored.
//
// btr1
// ====
// Value for bitrate register 1. If btr value pairs are given then the bus-speed
// parameter is ignored.
//
// 
// flags 
//-----------------------------------------------------------------------------
//
// bit 0
// =====
//  0		11 bit identifier mode
//	1		29 bit identifier mode
//
// bit 1
// =====
//	0		High speed
//	1		A low speed-busconnector is used
//			(if provided by the hardware)
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

bool CIxxObj::open( const char *szFileName, unsigned long flags )
{
	int ret;
	const char *p;
	unsigned long busspeed = 125;
	unsigned char btr0 = 0, btr1 = 0;
	char szDrvParams[ MAX_PATH ];
	m_initFlag = flags;

	// save parameter string and conbert to upper case
	strncpy( szDrvParams, szFileName, MAX_PATH );
	_strupr( szDrvParams );

	m_dwBrdType = VCI_UNKNOWN; // VCI_USB2CAN_COMPACT;
	m_dwBrdTypeRelatedIndex = 0;

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
	
	// Boardtype
	p = strtok( (char * )szDrvParams, ";" );
	if ( NULL != p ) {
	
		if ( isalpha( *p ) ) {
		
			// Symbolic form
			int symidx = 0;
			while( -1 != ixxatsymtbl[ symidx ]. id ) {
			
				if ( NULL != strstr( ixxatsymtbl[ symidx ]. symname, p ) ) {
					m_dwBrdType = ixxatsymtbl[ symidx ]. id;
					break;
				}

				symidx++;	

			}
				 	
		}
		else {
			
			// Numeric form
			m_dwBrdType = atoi( p );	

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
			sscanf( p + 2, "%x", &m_IXXAT_filter );
		}
		else {
			m_IXXAT_filter = atol( p );
		}
	}


	// Mask
	p = strtok( NULL, ";" );
	if ( NULL != p ) {		
		if ( ( NULL != strstr( p, "0x" ) ) || ( NULL != strstr( p, "0X" ) )  ) {
			sscanf( p + 2, "%x", &m_IXXAT_mask );
		}
		else {
			m_IXXAT_mask = atol( p );
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

	// BTR0
	p = strtok( NULL, ";" );
	if ( NULL != p ) {		
		if ( ( NULL != strstr( p, "0x" ) ) || ( NULL != strstr( p, "0X" ) )  ) {
			sscanf( p + 2, "%x", &btr0 );
		}
		else {
			btr0 = atoi( p );
		}
	}

	// BTR1
	p = strtok( NULL, ";" );
	if ( NULL != p ) {		
		if ( ( NULL != strstr( p, "0x" ) ) || ( NULL != strstr( p, "0X" ) )  ) {
			sscanf( p + 2, "%x", &btr1 );
		}
		else {
			btr1 = atoi( p );
		}
	}

	// Handle busspeed
	switch ( busspeed ) {
	
		case 10:
			btr0 = 0x31;
			btr1 = 0x1C;
			break;

		case 20:
			btr0 = 0x18;
			btr1 = 0x1C;
			break;

		case 50:
			btr0 = 0x09;
			btr1 = 0x1C;
			break;

		case 100:
			btr0 = 0x04;
			btr1 = 0x1C;
			break;

		case 125:
			btr0 = 0x03;
			btr1 = 0x1C;
			break;

		case 250:
			btr0 = 0x01;
			btr1 = 0x1C;
			break;

		case 500:
			btr0 = 0x00;
			btr1 = 0x1C;
			break;

		case 800:
			btr0 = 0x00;
			btr1 = 0x16;
			break;

		case 1000:
			btr0 = 0x00;
			btr1 = 0x14;
			break;

	}

	// Must have a bus speed
	if ( ( 0 == btr0 ) && ( 0 == btr1 )  )  return false;

	
	// Find hardware entry
	HRESULT hr = XAT_FindHwEntry( XATREG_FIND_BOARD_AT_RELATIVE_POSITION,
									&m_dwBrdKey,
									&m_dwBrdType,
									NULL,
									m_dwBrdTypeRelatedIndex );

	if ( ERROR_SUCCESS != hr ) {
		return false;
	}
		
	
	// Get Configuration for card
	hr = XAT_GetConfig( m_dwBrdKey, &m_sConfig );
	
	if ( ERROR_SUCCESS != hr ) {
		return false;
	} 

 
	//
    //  prepare choosen board for further configuration
    //
	ret = 
		VCI2_PrepareBoard( m_sConfig.board_type,	// board type
							m_sConfig.board_no,		// unique board index
							NULL,				// pointer to buffer for additional info 
                            0,					// length of additional info buffer
                            NULL,				// pointer to msg-callbackhandler
                            NULL,				// pointer to receive-callbackhandler
                            NULL  );			// pointer to exception-callbackhandler
    if ( ret < 0 ) {
		VCI_CancelBoard( m_hBoard );
		return false;
	}

    // extract board handle from return value
    m_hBoard = (UINT16)ret;
 
	//
	//  initialize CAN-Controller
	//
	
	ret = VCI_InitCan( m_hBoard, m_channel, btr0, btr1, (unsigned char)( flags & 0x1f ) );

	//
	//  definition of Acceptance-Mask (define to receive all IDs)
	//
	
	ret = VCI_SetAccMask( m_hBoard, m_channel, 0x0UL, 0x0UL );


	//
	//  definition of Transmit Queue
	//
	
	ret = VCI_ConfigQueue( m_hBoard, 
							            m_channel, 
							            VCI_TX_QUE, 
							            100,		// queue size
							            0,			// no limit 
							            0,			// no timeout
							            0,			// timestamp resolution 100 uSec
							            &m_hTxQue );


	//
	//  definition of Receive Queue 
	//
	
	ret = VCI_ConfigQueue( m_hBoard, 
							m_channel, 
							VCI_RX_QUE, 
							100,			// queue size
							0,				// No limit = polling mode 
							0,				// timeout not relavant
							1,				// timestamp resolution 100 uSec
							&m_hRxQue );

	
	ret = VCI_AssignRxQueObj( m_hBoard, m_hRxQue ,VCI_ACCEPT, 0x00, 0x00 ) ;

	//
	//  And now start the CAN
	//
	
	ret = VCI_StartCan( m_hBoard, 0 );

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
	UNLOCK_MUTEX( m_ixxMutex );
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
							
		syslog( LOG_CRIT, "canallogger: Unable to create ixxatvcidrv write thread.");
		rv = false;
		fclose( m_flog );
	}


	// Create the log write thread.
	if ( pthread_create( 	&m_threadId,
								&thread_attr,
								workThreadReceive,
								this ) ) {	
							
		syslog( LOG_CRIT, "canallogger: Unable to create ixxatvcidrv receive thread.");
		rv = false;
		fclose( m_flog );
	}
		
    // We are open
	m_bOpen = true;

	// Release the mutex
	pthread_mutex_unlock( &m_ixxMutex );

#endif	

	return true;
}

 
//////////////////////////////////////////////////////////////////////
// close
//

bool CIxxObj::close( void )
{	
	// Do nothing if already terminated
	if ( !m_bRun ) return false;
	
	m_bRun = false;

	VCI_CancelBoard( m_hBoard );
 
	UNLOCK_MUTEX( m_ixxMutex );
	LOCK_MUTEX( m_ixxMutex );

	
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
	pthread_mutex_destroy( &m_ixxMutex );
	
#endif

	return true;
}


//////////////////////////////////////////////////////////////////////
// doFilter
//

bool CIxxObj::doFilter( canalMsg *pcanalMsg )
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

bool CIxxObj::setFilter( unsigned long filter )
{
	m_filter = filter;
	return true;
}


//////////////////////////////////////////////////////////////////////
// setMask
//

bool CIxxObj::setMask( unsigned long mask )
{
	m_mask = mask;
	return true;
}



//////////////////////////////////////////////////////////////////////
// writeMsg
//

bool CIxxObj::writeMsg( canalMsg *pMsg )
{	
	bool rv = false;
	
	if ( NULL != pMsg ) {

		// VCI2 only allow sending of message with the same
		// mode as the i/f is opended with. So extended id's in 
		// standard mode shoud be disregareded and vice versa.
		if ( VCI_29B & m_initFlag ) {
			if ( !( pMsg->flags & CANAL_IDFLAG_EXTENDED ) ) return false;
		} else {
			if ( pMsg->flags & CANAL_IDFLAG_EXTENDED ) return false;
		}

	
		// Must be room for the message
		if ( m_transmitList.nCount < IXXATVCI_MAX_SNDMSG ) {

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

bool CIxxObj::readMsg( canalMsg *pMsg )
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

int CIxxObj::dataAvailable( void )
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

bool CIxxObj::getStatistics( PCANALSTATISTICS pCanalStatistics )
{	
	// Must be a valid pointer
	if ( NULL == pCanalStatistics ) return false;

	memcpy( pCanalStatistics, &m_stat, sizeof( canalStatistics ) );

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//	getStatus
//

bool CIxxObj::getStatus( PCANALSTATUS pCanalStatus )
{
	// Must be a valid pointer
	if ( NULL == pCanalStatus ) return false;
	
	VCI_CAN_STS canstatus;
	if ( VCI_OK != VCI_ReadCanStatus( m_hBoard, m_channel, &canstatus ) ) {
		return false;
	}

	m_status.channel_status = canstatus.sts;

	// Repeat bus off flag
	if ( canstatus.sts & 0x80 ) {
		m_status.channel_status |= 0x80000000;		
	}

	// Repeat bus off flag
	if ( canstatus.sts & 0x40 ) {
		m_status.channel_status |= 0x40000000;		
	}

	// Extended ID
	if ( m_initFlag & VCI_29B ) {
		m_status.channel_status |= 0x010000;		
	}

	// Low Speed
	if ( m_initFlag & VCI_LOW_SPEED ) {
		m_status.channel_status |= 0x020000;		
	}

	// Local echo
	if ( m_initFlag & VCI_TX_ECHO ) {
		m_status.channel_status |= 0x040000;		
	}

	// Passive
	if ( m_initFlag & VCI_TX_PASSIV ) {
		m_status.channel_status |= 0x080000;		
	}

	// Detected error frame
	if ( m_initFlag & VCI_ERRFRM_DET ) {
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

	CIxxObj * pobj = ( CIxxObj *)pObject;
	if ( NULL == pobj ) {
#ifdef WIN32	
		ExitThread( errorCode ); // Fail
#else
		pthread_exit( &rv );
#endif
	}
	
	while ( pobj->m_bRun ) {

		LOCK_MUTEX( pobj->m_ixxMutex );
		
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
			
			if ( !( msg.flags & CANAL_IDFLAG_RTR ) ) {

				// Standard message
			
				if (  VCI_OK == VCI_TransmitObj( pobj->m_hBoard, 
													pobj->m_hTxQue, 
													msg.id, 
													msg.sizeData, 
													msg.data ) ) {
				
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
			}
			else {

				// Remote request

				if (  VCI_OK == VCI_RequestObj( pobj->m_hBoard, 
													pobj->m_hTxQue, 
													msg.id, 
													msg.sizeData ) ) {
				
					// Message sent successfully
					// Update statistics
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
				
			}
							
		} // while data


		// No data to write

		UNLOCK_MUTEX( pobj->m_ixxMutex );
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

	CIxxObj * pobj = ( CIxxObj *)pObject;
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

		LOCK_MUTEX( pobj->m_ixxMutex );
		if ( ( cntMsg = VCI_ReadQueStatus( pobj->m_hBoard, pobj->m_hRxQue ) ) > 0 ) {
			
			// There are messages to fetch
			VCI_CAN_OBJ *pobjarray = new VCI_CAN_OBJ[ cntMsg ];
			if ( NULL != pobjarray ) {

				readMsg = VCI_ReadQueObj( pobj->m_hBoard, 
											pobj->m_hRxQue, 
											cntMsg, 
											pobjarray );
				if ( readMsg > 0 ) {
				
					// Write them to the receive buffer
					for ( int i=0; i<readMsg; i++ ) {
					
						if (  pobj->m_receiveList.nCount < IXXATVCI_MAX_RCVMSG ) {					
					
							PCANALMSG pMsg	= new canalMsg;
							pMsg->flags = 0;

							if ( NULL != pMsg ) {
						
								dllnode *pNode = new dllnode; 
								if ( NULL != pNode ) {
							
									pMsg->timestamp = pobjarray[ i ].time_stamp;
									pMsg->id = pobjarray[ i ].id;
									pMsg->sizeData = pobjarray[ i ].len;		
									memcpy( pMsg->data, pobjarray[ i ].a_data, pMsg->sizeData ); 
									
									// If extended set extended flag
									if ( VCI_29B & pobj->m_initFlag ) pMsg->flags |= CANAL_IDFLAG_EXTENDED;

									// Check for RTS package
									if ( pobjarray[ i ].rtr ) pMsg->flags |= CANAL_IDFLAG_RTR;
									
									// Check for overrun error
									if ( pobjarray[ i ].sts & 0x80 ) pobj->m_stat.cntOverruns++;
									
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
		
				delete pobjarray;
			}
		}

		UNLOCK_MUTEX( pobj->m_ixxMutex );
		SLEEP( 1 );
	
	} // while 	 


#ifdef WIN32
	ExitThread( errorCode );
#else
	pthread_exit( &rv );
#endif

}