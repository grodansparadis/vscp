// deviceThread.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2008 Ake Hedman, D of Scandinavia, <akhe@eurosource.se>
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
// $RCSfile: devicethread_win32.cpp,v $                                       
// $Date: 2005/08/25 20:08:09 $                                  
// $Author: akhe $                                              
// $Revision: 1.4 $ 
#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"
#include "wx/thread.h"


// Common includes
#include "canal_win32_ipc.h"
#include "controlobject.h"

// Prototypes
void deviceThread( void *pDeviceInfo );
void deviceThreadRead( void *pDeviceInfo );
void deviceThreadSend( void *pDeviceInfo );

///////////////////////////////////////////////////////////////////////////////
// deviceThread
//

void deviceThread( void *pDeviceInfo )
{
	
	CControlObject *pctrlObject = NULL;

	DWORD errorCode = 0;
	DWORD nTrans = 0;
	DWORD nWrite = 0;	


	// Must have data to work with 
	if ( ( NULL == pDeviceInfo ) ) {
		ExitThread( errorCode );
	}
   
	deviceStruct *pDeviceStruct = (deviceStruct *)pDeviceInfo;
	pctrlObject = pDeviceStruct->pctrlObject;

	// Driver DLL not open
	pDeviceStruct->pDeviceItem->m_openHandle = 0;

	// Must have data to work with 
	if ( ( NULL == pDeviceStruct->pctrlObject ) || ( NULL == pDeviceStruct->pDeviceItem ) ) {
		ExitThread( errorCode );
	}


	if ( NULL == ( pDeviceStruct->pDeviceItem->m_hinst = 
									LoadLibrary( pDeviceStruct->pDeviceItem->m_strPath ) ) ) {

		ExitThread( errorCode );
	}

	
	// * * * * CANAL OPEN * * * * 	
	if ( NULL == ( pDeviceStruct->pDeviceItem->m_proc_CanalOpen = 
									(LPFNDLL_CANALOPEN)GetProcAddress( pDeviceStruct->pDeviceItem->m_hinst, 
																		"CanalOpen" ) ) ) {
		FreeLibrary( pDeviceStruct->pDeviceItem->m_hinst );


		ExitThread( errorCode );
	}


	// * * * * CANAL CLOSE * * * * 	
	if ( NULL == ( pDeviceStruct->pDeviceItem->m_proc_CanalClose = 
									(LPFNDLL_CANALCLOSE)GetProcAddress( pDeviceStruct->pDeviceItem->m_hinst, 
																		"CanalClose" ) ) ) {
		FreeLibrary( pDeviceStruct->pDeviceItem->m_hinst );

		ExitThread( errorCode );
	}


	// * * * * CANAL GETLEVEL * * * * 	
	if ( NULL == ( pDeviceStruct->pDeviceItem->m_proc_CanalGetLevel = 
						(LPFNDLL_CANALGETLEVEL)GetProcAddress( pDeviceStruct->pDeviceItem->m_hinst, 
																		"CanalGetLevel" ) ) ) {
		FreeLibrary( pDeviceStruct->pDeviceItem->m_hinst );
		ExitThread( errorCode );
	}

	// * * * * CANAL SEND * * * * 	
	if ( NULL == ( pDeviceStruct->pDeviceItem->m_proc_CanalSend = 
									(LPFNDLL_CANALSEND)GetProcAddress( pDeviceStruct->pDeviceItem->m_hinst, 
																		"CanalSend" ) ) ) {
		FreeLibrary( pDeviceStruct->pDeviceItem->m_hinst );
		ExitThread( errorCode );
	}


	// * * * * CANAL DATA AVAILABLE * * * * 	
	if ( NULL == ( pDeviceStruct->pDeviceItem->m_proc_CanalDataAvailable = 
									(LPFNDLL_CANALDATAAVAILABLE)GetProcAddress( pDeviceStruct->pDeviceItem->m_hinst, 
																		"CanalDataAvailable" ) ) ) {
		FreeLibrary( pDeviceStruct->pDeviceItem->m_hinst );
		ExitThread( errorCode );
	}


	// * * * * CANAL RECEIVE * * * * 	
	if ( NULL == ( pDeviceStruct->pDeviceItem->m_proc_CanalReceive = 
									(LPFNDLL_CANALRECEIVE)GetProcAddress( pDeviceStruct->pDeviceItem->m_hinst, 
																		"CanalReceive" ) ) ) {
		FreeLibrary( pDeviceStruct->pDeviceItem->m_hinst );
		ExitThread( errorCode );
	}


	// * * * * CANAL GET STATUS * * * * 
	if ( NULL == ( pDeviceStruct->pDeviceItem->m_proc_CanalGetStatus = 
									(LPFNDLL_CANALGETSTATUS)GetProcAddress( pDeviceStruct->pDeviceItem->m_hinst, 
																		"CanalGetStatus" ) ) ) {
		FreeLibrary( pDeviceStruct->pDeviceItem->m_hinst );
		ExitThread( errorCode );
	}


	// * * * * CANAL GET STATISTICS * * * * 	
	if ( NULL == ( pDeviceStruct->pDeviceItem->m_proc_CanalGetStatistics = 
									(LPFNDLL_CANALGETSTATISTICS)GetProcAddress( pDeviceStruct->pDeviceItem->m_hinst, 
																		"CanalGetStatistics" ) ) ) {
		FreeLibrary( pDeviceStruct->pDeviceItem->m_hinst );
		ExitThread( errorCode );
	}


	// * * * * CANAL SET FILTER * * * * 	
	if ( NULL == ( pDeviceStruct->pDeviceItem->m_proc_CanalSetFilter = 
									(LPFNDLL_CANALSETFILTER)GetProcAddress( pDeviceStruct->pDeviceItem->m_hinst, 
																		"CanalSetFilter" ) ) ) {
		FreeLibrary( pDeviceStruct->pDeviceItem->m_hinst );
		ExitThread( errorCode );
	}


	// * * * * CANAL SET MASK * * * * 	
	if ( NULL == ( pDeviceStruct->pDeviceItem->m_proc_CanalSetMask = 
									(LPFNDLL_CANALSETMASK)GetProcAddress( pDeviceStruct->pDeviceItem->m_hinst, 
																		"CanalSetMask" ) ) ) {
		FreeLibrary( pDeviceStruct->pDeviceItem->m_hinst );
		ExitThread( errorCode );
	}


	// * * * * CANAL GET VERSION * * * * 	
	if ( NULL == ( pDeviceStruct->pDeviceItem->m_proc_CanalGetVersion = 
									(LPFNDLL_CANALGETVERSION)GetProcAddress( pDeviceStruct->pDeviceItem->m_hinst, 
																		"CanalGetVersion" ) ) ) {
		FreeLibrary( pDeviceStruct->pDeviceItem->m_hinst );
		ExitThread( errorCode );
	}


	// * * * * CANAL GET DLL VERSION * * * * 	
	if ( NULL == ( pDeviceStruct->pDeviceItem->m_proc_CanalGetDllVersion = 
									(LPFNDLL_CANALGETDLLVERSION)GetProcAddress( pDeviceStruct->pDeviceItem->m_hinst, 
																		"CanalGetDllVersion" ) ) ) {
		FreeLibrary( pDeviceStruct->pDeviceItem->m_hinst );
		ExitThread( errorCode );
	}


	// * * * * CANAL GET VENDOR STRING * * * * 	
	if ( NULL == ( pDeviceStruct->pDeviceItem->m_proc_CanalGetVendorString = 
						(LPFNDLL_CANALGETVENDORSTRING)GetProcAddress( pDeviceStruct->pDeviceItem->m_hinst, 
																		"CanalGetVendorString" ) ) ) {
		FreeLibrary( pDeviceStruct->pDeviceItem->m_hinst );
		ExitThread( errorCode );
	}


	////////////////////////////////////////////////////////////////
	//						Generation 2
	////////////////////////////////////////////////////////////////


	// * * * * CANAL BLOCKING SEND * * * * 	
	if ( NULL == ( pDeviceStruct->pDeviceItem->m_proc_CanalBlockingSend = 
						(LPFNDLL_CANALBLOCKINGSEND)GetProcAddress( pDeviceStruct->pDeviceItem->m_hinst, 
																		"CanalBlockingSend" ) ) ) {
		// We accept a NULL here
	}

	// * * * * CANAL BLOCKING RECEIVE * * * * 	
	if ( NULL == ( pDeviceStruct->pDeviceItem->m_proc_CanalBlockingReceive = 
						(LPFNDLL_CANALBLOCKINGRECEIVE)GetProcAddress( pDeviceStruct->pDeviceItem->m_hinst, 
																		"CanalBlockingReceive" ) ) ) {
		// We accept a NULL here
	}

	// * * * * CANAL GET DRIVER INFO * * * * 	
	if ( NULL == ( pDeviceStruct->pDeviceItem->m_proc_CanalGetdriverInfo = 
						(LPFNDLL_CANALGETDRIVERINFO)GetProcAddress( pDeviceStruct->pDeviceItem->m_hinst, 
																		"CanalGetDriverInfo" ) ) ) {
		// We accept a NULL here
	}



	//
	// =====================================================================================
	//

	// Open the device
	pDeviceStruct->pDeviceItem->m_openHandle = 
		pDeviceStruct->pDeviceItem->m_proc_CanalOpen( pDeviceStruct->pDeviceItem->m_strParameter,
												pDeviceStruct->pDeviceItem->m_DeviceFlags );

	// Get Driver Level
	pDeviceStruct->pDeviceItem->m_driverLevel = 
		pDeviceStruct->pDeviceItem->m_proc_CanalGetLevel( pDeviceStruct->pDeviceItem->m_openHandle );

	// Get driver properties
	if ( NULL != pDeviceStruct->pDeviceItem->m_proc_CanalGetdriverInfo ) {
		pDeviceStruct->pDeviceItem->m_proc_CanalGetdriverInfo( pDeviceStruct->pDeviceItem->m_openHandle,
																														&pDeviceStruct->pDeviceItem->m_driverProperties );
	}

	/////////////////////////////////////////////////////////////////////////////
	//                              Do Work here
	//								============
	//
	// This thread just tries to read & write from/to the device.
	//
	// Other commands are handled 
	// by the client threads directly by aquiring the mutex and communicate on the
	// channel.
	//
	// If the channel is closed there is not much to do and there are no clients
	// to communicate with either.
	//
	// The nOpen parameter holds the number of open client connections. If zero
	// there are no clients and the interface should be closed.
	/////////////////////////////////////////////////////////////////////////////

	bool bTraffic = true;
	canalMsg *pMsg; 
	struct dllnode *pNode;


	if ( ( NULL == pDeviceStruct->pDeviceItem->m_proc_CanalBlockingSend ) ||
			( NULL == pDeviceStruct->pDeviceItem->m_proc_CanalBlockingReceive ) ) {

		while ( !pDeviceStruct->pctrlObject->m_bQuit ) {	


			pDeviceStruct->pDeviceItem->m_deviceMutex.Lock();

			
			/////////////////////////////////////////////////////////////////////////////
			//                                Receive								   //
			/////////////////////////////////////////////////////////////////////////////

			if ( 0 < pDeviceStruct->pDeviceItem->m_proc_CanalDataAvailable( pDeviceStruct->pDeviceItem->m_openHandle ) ) {
	
				pMsg = new canalMsg;

				if ( ( NULL != pMsg ) && 
						pDeviceStruct->pDeviceItem->m_proc_CanalReceive( pDeviceStruct->pDeviceItem->m_openHandle, pMsg ) ) {

					bTraffic = true;
					
					pctrlObject->m_wxDeviceMutex.Lock();

					// There must be room in the receive queue
					if ( pDeviceStruct->pctrlObject->m_maxItemsInReceiveQueue > 
							pDeviceStruct->pctrlObject->m_deviceReceiveQueue.nCount ) {

						struct dllnode *pNode = new dllnode;
						if ( NULL != pNode ) {
							
							pNode->obid = pDeviceStruct->pDeviceItem->m_deviceID; // mark as from device

							pNode->Key = pMsg->id;
							pNode->pKey = &pNode->Key;
							pNode->pObject = pMsg;

							dll_addNode( &pDeviceStruct->pctrlObject->m_deviceReceiveQueue, pNode );

							// Flag that an event is available
							pctrlObject->m_hEventReceiveDevice.Post();

						}
						else {
							delete pMsg;
						}
					
						pctrlObject->m_wxDeviceMutex.Unlock();
					}
				}
			}


			// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
			//             Send messages (if any) in the outqueue
			// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

			pNode = pDeviceStruct->pDeviceItem->m_outputQueue.pHead;

			while ( NULL != pNode ) {

				bTraffic = true;
				OutputDebugString("\tDevice Thread - Outgoing message...\n");

				if ( NULL != ( pMsg = (canalMsg *)pNode->pObject ) ) { 
					OutputDebugString("Before device send\n");
					pDeviceStruct->pDeviceItem->m_proc_CanalSend( pDeviceStruct->pDeviceItem->m_openHandle, pMsg );
					OutputDebugString("After device send\n");
				}

				OutputDebugString("Before dll_remove node\n");
				dll_removeNode( &pDeviceStruct->pDeviceItem->m_outputQueue, pNode );				
				pNode = pDeviceStruct->pDeviceItem->m_outputQueue.pHead;

			}

			// Give others a chance to work
			pDeviceStruct->pDeviceItem->m_deviceMutex.Unlock();
			
			if ( !bTraffic ) {		
				Sleep( 1 );			
			}

			bTraffic = false;

 		
		} // Work loop

	
	} // generation


	////////////////////////////////////////////////////////////////
	//				     Generation 2  --  W O R K
	////////////////////////////////////////////////////////////////
	
	
	else {

		HANDLE deviceThreadIdReceive;
		DWORD dwThreadIdReceive;
		HANDLE deviceThreadIdSend;
		DWORD dwThreadIdSend;

		// Start receive thread
		if ( NULL == ( deviceThreadIdReceive = 
						CreateThread(	NULL,
										0,
										(LPTHREAD_START_ROUTINE) deviceThreadRead,
										pDeviceInfo,
										0,
										&dwThreadIdReceive ) ) ) { 
			pDeviceStruct->pctrlObject->m_bQuit = true;
			ExitThread( -1 );
		}
		else {
			SetThreadPriority( deviceThreadIdReceive, THREAD_PRIORITY_TIME_CRITICAL );
		}

		// Start send thread
		if ( NULL == ( deviceThreadIdSend = 
						CreateThread(	NULL,
										0,
										(LPTHREAD_START_ROUTINE) deviceThreadSend,
										pDeviceInfo,
										0,
										&dwThreadIdSend ) ) ) { 
			pDeviceStruct->pctrlObject->m_bQuit = true;
			ExitThread( -1 );
		}
		else {
			SetThreadPriority( deviceThreadIdSend, THREAD_PRIORITY_TIME_CRITICAL );
		}

		// Wait until instructed to end	
		while ( !pDeviceStruct->pctrlObject->m_bQuit ) {
			Sleep( 1000 );
		}
	
	}

	// Close the device
	pDeviceStruct->pDeviceItem->m_proc_CanalClose( pDeviceStruct->pDeviceItem->m_openHandle );	

	// Free the library	
	FreeLibrary( pDeviceStruct->pDeviceItem->m_hinst );

	// Delete the device structure
	delete (deviceStruct *)pDeviceInfo;	
		
	ExitThread( errorCode );

  
}




///////////////////////////////////////////////////////////////////////////////
// deviceThreadRead
//
// Read events from device
//

void deviceThreadRead( void *pDeviceInfo )
{
	CControlObject *pctrlObject = NULL;
	DWORD errorCode = 0;
	canalMsg *pMsg;

	// Must have data to work with
	if ( ( NULL == pDeviceInfo ) ) {
		ExitThread( -1 );
	}
   
	deviceStruct *pDeviceStruct = (deviceStruct *)pDeviceInfo;
	pctrlObject = pDeviceStruct->pctrlObject;

	// Must have data to work with 
	if ( ( NULL == pDeviceStruct->pctrlObject ) || ( NULL == pDeviceStruct->pDeviceItem ) ) {
		ExitThread( -1 );
	}

	// Must have an open dll interface to continue
	if ( !pDeviceStruct->pDeviceItem->m_openHandle ) ExitThread( -1 );

	while ( !pDeviceStruct->pctrlObject->m_bQuit ) {	

		pMsg = new canalMsg;
		if ( NULL == pMsg ) break;

		if ( CANAL_ERROR_SUCCESS == 
				pDeviceStruct->pDeviceItem->m_proc_CanalBlockingReceive( pDeviceStruct->pDeviceItem->m_openHandle, 
																			pMsg, 
																			500 ) ) {
		
			pctrlObject->m_wxDeviceMutex.Lock();

			// There must be room in the receive queue
			if ( pDeviceStruct->pctrlObject->m_maxItemsInReceiveQueue > 
					 pDeviceStruct->pctrlObject->m_deviceReceiveQueue.nCount ) {

				struct dllnode *pNode = new dllnode;
				if ( NULL != pNode ) {
							
					pNode->obid = pDeviceStruct->pDeviceItem->m_deviceID; // mark as from device

					pNode->Key = pMsg->id;
					pNode->pKey = &pNode->Key;
					pNode->pObject = pMsg;

					dll_addNode( &pDeviceStruct->pctrlObject->m_deviceReceiveQueue, pNode );

					// Flag that an event is available
          pctrlObject->m_hEventReceiveDevice.Post();

				}
				else {
					delete pMsg;
				}
					
				pctrlObject->m_wxDeviceMutex.Unlock();
			}
		}
		else {
			delete pMsg;
			pMsg = NULL;
		}
	
	} // while

	ExitThread( errorCode );
}


///////////////////////////////////////////////////////////////////////////////
// deviceThreadSend
//
// Write events to device
//

void deviceThreadSend( void *pDeviceInfo )
{
	CControlObject *pctrlObject = NULL;
	DWORD errorCode = 0;
	canalMsg *pMsg; 
	struct dllnode *pNode;

	// Must have data to work with 
	if ( ( NULL == pDeviceInfo ) ) {
		ExitThread( -1 );
	}
   
	deviceStruct *pDeviceStruct = (deviceStruct *)pDeviceInfo;
	pctrlObject = pDeviceStruct->pctrlObject;

	// Must have data to work with 
	if ( ( NULL == pDeviceStruct->pctrlObject ) || ( NULL == pDeviceStruct->pDeviceItem ) ) {
		ExitThread( -1 );
	}

	// Must have an open dll interface to continue
	if ( !pDeviceStruct->pDeviceItem->m_openHandle ) ExitThread( -1 );

	while ( !pDeviceStruct->pctrlObject->m_bQuit ) {

    if ( wxSEMA_TIMEOUT == pDeviceStruct->pDeviceItem->m_hEventSend.WaitTimeout( 500 ) ) {
      continue;
    }

		pNode = pDeviceStruct->pDeviceItem->m_outputQueue.pHead;

		while ( NULL != pNode ) {

			OutputDebugString("\tDevice Thread - Outgoing message...\n");

			if ( NULL != ( pMsg = (canalMsg *)pNode->pObject ) ) { 
				pDeviceStruct->pDeviceItem->m_proc_CanalSend( pDeviceStruct->pDeviceItem->m_openHandle, pMsg );
			}

			dll_removeNode( &pDeviceStruct->pDeviceItem->m_outputQueue, pNode );

		}

		// Give others a chance to work
		pDeviceStruct->pDeviceItem->m_deviceMutex.Unlock();

	}

	ExitThread( errorCode );
}
