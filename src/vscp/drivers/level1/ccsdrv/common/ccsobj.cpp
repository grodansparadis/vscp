///////////////////////////////////////////////////////////////////////////////
// CCSCANDrv.cpp: implementation of the CCCSCANDrvObj class.
//
// This file is part is part of CANAL (CAN Abstraction Layer)
// http://www.vscp.org)
//
// Copyright (C) 2000-2012 Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
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
// $RCSfile: ccsobj.cpp,v $                                       
// $Date: 2005/01/05 12:16:13 $                                  
// $Author: akhe $                                              
// $Revision: 1.4 $ 
///////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include "CCSObj.h"


// Prototypes
void workThread( void *p );


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCSObj::CCSObj()
{
	m_ccscanObj.m_handle = NULL;				// not open
	m_ccscanObj.m_version = -1;					// Version is unknown
	m_ccscanObj.m_bAuto = false;				// Poll mode is default
	m_ccscanObj.m_state = CCSCAN_STATE_NONE;
	dll_init( &m_ccscanObj.m_rcvList, SORT_NONE );
	dll_init( &m_ccscanObj.m_sndList, SORT_NONE );
}

CCSObj::~CCSObj()
{
	close();	// Cose comm channel in case its open
	dll_removeAllNodes( &m_ccscanObj.m_rcvList );
	dll_removeAllNodes( &m_ccscanObj.m_sndList );
}


///////////////////////////////////////////////////////////////////////////////
// Open a CCS CAN driver channel
//
// Parameters are as follows
//
//		none
//
// if btr0 and btr1 is defined they should be used instead of bus-speed which
// in that case can have any value even if zero is recommended.
//

long CCSObj::open( const char *pDevice, unsigned long flags )
{
	m_ccscanObjMutex = OpenMutex( MUTEX_ALL_ACCESS, true, CCSCAN_OBJ_MUTEX );

	m_ccscanObj.m_bRun = true;

	// Initiate data

	// Open
	if ( NULL == ( m_ccscanObj.m_handle = CanOpen( pDevice ) ) ) {
		// Failed
		return 0;
	}

	// Start the worker thread	
	if ( NULL == CreateThread(	NULL,
								0,
								(LPTHREAD_START_ROUTINE) workThread,
								&m_ccscanObj,
								0,
								&m_dwThreadId ) ) { 
		close();
		CloseHandle( m_ccscanObjMutex );
		return 0;
	}

	// Release the mutex for other threads to use
	ReleaseMutex( m_ccscanObjMutex );


	return 1;
}


///////////////////////////////////////////////////////////////////////////////
//  close
// 

int CCSObj::close( void )
{
	int rv = 1;

	// Terminate the thread
	m_ccscanObj.m_bRun = false;
	Sleep( 1000 );

	// if closed we have noting to do
	if ( CanClose( m_ccscanObj.m_handle ) ) return 1;

	m_ccscanObj.m_handle = NULL;

	return rv;
}


///////////////////////////////////////////////////////////////////////////////
//  writeMsg
// 

int CCSObj::writeMsg( bool bExtended, 
							unsigned long id, 
							unsigned char dlc, 
							unsigned char * pdata )
{
	int rv = 0;
	
	// Must be room for the message
	if ( m_ccscanObj.m_sndList.nCount < CCSCAN_MAX_SNDMSG ) {
		
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

				WaitForSingleObject( m_ccscanObjMutex, INFINITE );
				dll_addNode( &m_ccscanObj.m_sndList, pNode );
				ReleaseMutex( m_ccscanObjMutex );
				
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

int CCSObj::writeMsg( PCANALMSG pCanalMsg )
{
	int rv = 0;

	// Must be room for the message
	if ( m_ccscanObj.m_sndList.nCount < CCSCAN_MAX_SNDMSG ) {
			
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
				

				WaitForSingleObject( m_ccscanObjMutex, INFINITE );
				dll_addNode( &m_ccscanObj.m_sndList, pNode );
				ReleaseMutex( m_ccscanObjMutex );
							
				rv = true;

			}	 
		}		
	}

	return rv;	
}

///////////////////////////////////////////////////////////////////////////////
//  readMsg
// 

int CCSObj::readMsg( canalMsg *pMsg )
{			
	int rv = false;
	
	if ( ( NULL != m_ccscanObj.m_rcvList.pHead ) && 
			( NULL != m_ccscanObj.m_rcvList.pHead->pObject ) ) {
		
		WaitForSingleObject( m_ccscanObjMutex, INFINITE );
		
		memcpy( pMsg, m_ccscanObj.m_rcvList.pHead->pObject, sizeof( canalMsg ) );
		dll_removeNode( &m_ccscanObj.m_rcvList, m_ccscanObj.m_rcvList.pHead );
		
		ReleaseMutex( m_ccscanObjMutex );
		rv = true;
	}

	return rv;
}


///////////////////////////////////////////////////////////////////////////////
//	setFilter
//

bool CCSObj::setFilter( unsigned long filter, unsigned long mask )
{
	return false;
}

///////////////////////////////////////////////////////////////////////////////
//	setFilter
//

bool CCSObj::setFilter( unsigned long filter )
{
	return false;
}

///////////////////////////////////////////////////////////////////////////////
//	setMask
//

bool CCSObj::setMask( unsigned long mask )
{
	return false;
}

///////////////////////////////////////////////////////////////////////////////
//	dataAvailable
//

int CCSObj::dataAvailable( void )
{
	int cnt;
	
	WaitForSingleObject( m_ccscanObjMutex, INFINITE );
	cnt = dll_getNodeCount( &m_ccscanObj.m_rcvList );	
	ReleaseMutex( m_ccscanObjMutex );

	return cnt;
}

///////////////////////////////////////////////////////////////////////////////
//	getStatistics
//

bool CCSObj::getStatistics( PCANALSTATISTICS pCanalStatistics )
{
	bool rv  = false;
	CanStatistics ccsstat;

	pCanalStatistics = &m_ccscanObj.m_stat;

	WaitForSingleObject( m_ccscanObjMutex, INFINITE );

	if ( CanGetStatistics( m_ccscanObj.m_handle, &ccsstat ) ) {
		m_ccscanObj.m_stat.cntBusOff = ccsstat.busOffCntr;
		m_ccscanObj.m_stat.cntBusWarnings = ccsstat.busWarnCntr;
		m_ccscanObj.m_stat.cntOverruns = ccsstat.hwOvrnCntr;
		m_ccscanObj.m_stat.cntReceiveData = ccsstat.rxDataCntr;
		m_ccscanObj.m_stat.cntReceiveFrames = ccsstat.rxMsgCntr;
		m_ccscanObj.m_stat.cntTransmitData = ccsstat.txDataCntr;
		m_ccscanObj.m_stat.cntTransmitFrames = ccsstat.txMsgCntr;

		rv = true;
	}

	ReleaseMutex( m_ccscanObjMutex );

	return rv;
}

///////////////////////////////////////////////////////////////////////////////
//	getStatus
//

bool CCSObj::getStatus( PCANALSTATUS pCanalStatus )
{
	pCanalStatus = NULL;	// Not supported
	return false;
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
	HANDLE ccscanobjMutex;	 		

	// CCS CAN
	CanMsg canmsg;
	DWORD dlc;
	CanFrameType frameType;
	
	// Must hav a valid work object
	if ( NULL == pThreadObject ) ExitThread( errorCode );

	// Get the object pointer in place
	struct _ccscanobj *pccscanobj = (struct _ccscanobj *)pThreadObject;
	
	ccscanobjMutex = OpenMutex( MUTEX_ALL_ACCESS, false, CCSCAN_OBJ_MUTEX );

	while ( pccscanobj->m_bRun ) {
		
		///////////////////////////////////////////////////////////////////////
		//								Receive 
		///////////////////////////////////////////////////////////////////////
		WaitForSingleObject( ccscanobjMutex, INFINITE );
 
		if ( CanReceiveEx( pccscanobj->m_handle, 
								&canmsg,
								&dlc,
								NULL,
								CAN_FRAME_STANDARD | CAN_FRAME_EXTENDED,
								&frameType,
								0 ) ) {


			// One message received
			bActivity = true;

			// If there is place in the queue 
			//		add message to it
			if (  pccscanobj->m_rcvList.nCount < CCSCAN_MAX_RCVMSG ) {					
				
				PCANALMSG pMsg	= new canalMsg;
				
				if ( NULL != pMsg ) {
						
					dllnode *pNode = new dllnode; 
					if ( NULL != pNode ) {
				
						if ( CAN_FRAME_EXTENDED == frameType ) {
							pMsg->flags = CANAL_IDFLAG_EXTENDED;
						}
						else {
							pMsg->flags = 0;
						}

						// Get Timestamp
						CanTimeStamp ts;
						CanGetLastTimeStamp( pccscanobj->m_handle, &ts );

						pMsg->id = canmsg.id;
						pMsg->sizeData = (unsigned char)dlc;
						pMsg->timestamp = ts.low * 10;
						
						if ( 0 != dlc ) {
							memcpy( pMsg->data, canmsg.data, dlc ); 
						}

						pNode->pObject = pMsg;
						dll_addNode( &pccscanobj->m_rcvList, pNode );

					}
				}
				else {

					delete pMsg;

				}
			}				
		}  // CAN message 

		ReleaseMutex( ccscanobjMutex );


		///////////////////////////////////////////////////////////////////////
		//								Transmit
		///////////////////////////////////////////////////////////////////////

		WaitForSingleObject( ccscanobjMutex, INFINITE );

		// Is there anything to transmit
		if ( ( NULL != pccscanobj->m_sndList.pHead ) && 
					( NULL != pccscanobj->m_sndList.pHead->pObject ) ) {

			canalMsg msg;
			bActivity = true;

			memcpy( &msg, pccscanobj->m_sndList.pHead->pObject, sizeof( canalMsg ) ); 
			dll_removeNode( &pccscanobj->m_sndList, pccscanobj->m_sndList.pHead );
			 
			// Must ve a valid standard id
			if ( !( msg.flags & CANAL_IDFLAG_EXTENDED ) && ( msg.id > 0x7ff ) ) {
				msg.id &= 0x7ff;	
			};

			// Must ve a valid extended id
			if ( ( msg.flags & CANAL_IDFLAG_EXTENDED ) && ( msg.id > 0x1fffffff ) ) {
				msg.id &= 0x1fffffff;	
			}

			// Set the frame type
			CanFrameType frameType;
			if ( msg.flags & CANAL_IDFLAG_EXTENDED ) {
				frameType = CAN_FRAME_EXTENDED;
			}
			else {
				frameType = CAN_FRAME_STANDARD;
			}

			BOOL bRtr;
			if ( msg.flags & CANAL_IDFLAG_RTR ) {
				bRtr = TRUE;
			}
			else {
				bRtr = FALSE;
			}

			canmsg.id = msg.id;
			memcpy( canmsg.data, msg.data, msg.sizeData );

			// Send the data			
			BOOL rv;
			long err;
			rv = CanSendEx( pccscanobj->m_handle, 
						&canmsg,
						msg.sizeData,		
						bRtr,
						frameType );
			err = GetLastError();
		
			
		}	

		ReleaseMutex( ccscanobjMutex );

		if ( !bActivity ) Sleep ( 10 );
		bActivity = false;

	}

	// Release the mutex for other threads to use
	ReleaseMutex( ccscanobjMutex );

	ExitThread( errorCode );
}


