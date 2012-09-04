// deviceThread_unix.cpp
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
// $RCSfile: devicethread_unix.cpp,v $                                       
// $Date: 2005/08/30 20:22:15 $                                  
// $Author: akhe $                                              
// $Revision: 1.5 $ 

#ifdef WIN32	// Win32 specific includes

#include "stdafx.h"
#include "../daemon/canald.h"

#else			// UNIX specific includes

#define _POSIX
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <dlfcn.h>
#include <syslog.h>

#include "wx/wx.h"
#include "wx/defs.h"
 
#endif

// Common includes
#include "controlobject.h"


///////////////////////////////////////////////////////////////////////////////
// deviceThread
//

void deviceThread( void *pDeviceInfo )
{
	long openHandle = 0;
	CControlObject *pctrlObject = NULL;

	pthread_attr_t thread_attr;
	int rv = 0;

	pthread_attr_init( &thread_attr );
	pthread_attr_setdetachstate( &thread_attr, PTHREAD_CREATE_DETACHED );	

	// Must have data to work with 
	if ( ( NULL == pDeviceInfo ) ) {

		pthread_exit( &rv );
	
	}
   
	deviceStruct *pDeviceStruct = (deviceStruct *)pDeviceInfo;
	pctrlObject = pDeviceStruct->pctrlObject;

		// Must have data to work with 
	if ( ( NULL == pDeviceStruct->pctrlObject ) || ( NULL == pDeviceStruct->pDeviceItem ) ) {

		pthread_exit( &rv );

	}

	if ( NULL == ( pDeviceStruct->pDeviceItem->m_hinst = 
			dlopen( (const char *) pDeviceStruct->pDeviceItem->m_strPath.mb_str(wxConvUTF8), RTLD_LAZY ) ) ) {
		syslog( LOG_CRIT, "canald: Unable to load dynamic library.");
		rv = -1;
		pthread_exit( &rv );
	}	

	
	// * * * * CANAL OPEN * * * * 
	
	if ( 0 == ( pDeviceStruct->pDeviceItem->m_proc_CanalOpen = 
				(LPFNDLL_CANALOPEN)dlsym( pDeviceStruct->pDeviceItem->m_hinst, 
											"CanalOpen" ) ) )  {
	
		// Free the library	
		dlclose( pDeviceStruct->pDeviceItem->m_hinst );
		syslog( LOG_CRIT, "canald: Unable to get dl entry for CanalOpen.");
		rv = -1;
		pthread_exit( &rv );
	}


	// * * * * CANAL CLOSE * * * * 

	if ( 0 == ( pDeviceStruct->pDeviceItem->m_proc_CanalClose = 
				(LPFNDLL_CANALCLOSE)dlsym( pDeviceStruct->pDeviceItem->m_hinst, 
											"CanalClose" ) ) )  {
	
		// Free the library	
		dlclose( pDeviceStruct->pDeviceItem->m_hinst );
		syslog( LOG_CRIT, "canald: Unable to get dl entry for CanaClose.");
		rv = -1;
		pthread_exit( &rv );
	}


	// * * * * CANAL GETLEVEL * * * * 

	if ( 0 == ( pDeviceStruct->pDeviceItem->m_proc_CanalGetLevel = 
				(LPFNDLL_CANALGETLEVEL)dlsym( pDeviceStruct->pDeviceItem->m_hinst, 
											"CanalGetLevel" ) ) )  {
	
		// Free the library	
		dlclose( pDeviceStruct->pDeviceItem->m_hinst );
		syslog( LOG_CRIT, "canald: Unable to get dl entry for CanalGetLevel.");
		rv = -1;
		pthread_exit( &rv );
	}


	// * * * * CANAL SEND * * * * 

	if ( 0 == ( pDeviceStruct->pDeviceItem->m_proc_CanalSend = 
				(LPFNDLL_CANALSEND)dlsym( pDeviceStruct->pDeviceItem->m_hinst, 
											"CanalSend" ) ) )  {
	
		// Free the library	
		dlclose( pDeviceStruct->pDeviceItem->m_hinst );
		syslog( LOG_CRIT, "canald: Unable to get dl entry for CanalSend.");
		rv = -1;
		pthread_exit( &rv );
	}


	// * * * * CANAL DATA AVAILABLE * * * * 

	if ( 0 == ( pDeviceStruct->pDeviceItem->m_proc_CanalDataAvailable = 
				(LPFNDLL_CANALDATAAVAILABLE)dlsym( pDeviceStruct->pDeviceItem->m_hinst, 
											"CanalDataAvailable" ) ) )  {
	
		// Free the library	
		dlclose( pDeviceStruct->pDeviceItem->m_hinst );
		syslog( LOG_CRIT, "canald: Unable to get dl entry for CanalDataAvailable.");
		rv = -1;
		pthread_exit( &rv );
	}


	// * * * * CANAL RECEIVE * * * * 

	if ( 0 == ( pDeviceStruct->pDeviceItem->m_proc_CanalReceive = 
				(LPFNDLL_CANALRECEIVE)dlsym( pDeviceStruct->pDeviceItem->m_hinst, 
											"CanalReceive" ) ) )  {
	
		// Free the library	
		dlclose( pDeviceStruct->pDeviceItem->m_hinst );
		syslog( LOG_CRIT, "canald: Unable to get dl entry for CanalReceive.");
		rv = -1;
		pthread_exit( &rv );
	}


	// * * * * CANAL GET STATUS * * * * 

	if ( 0 == ( pDeviceStruct->pDeviceItem->m_proc_CanalGetStatus = 
				(LPFNDLL_CANALGETSTATUS)dlsym( pDeviceStruct->pDeviceItem->m_hinst, 
											"CanalGetStatus" ) ) )  {
	
		// Free the library	
		dlclose( pDeviceStruct->pDeviceItem->m_hinst );
		syslog( LOG_CRIT, "canald: Unable to get dl entry for CanalGetStatus.");
		rv = -1;
		pthread_exit( &rv );
	}


	// * * * * CANAL GET STATISTICS * * * * 

	if ( 0 == ( pDeviceStruct->pDeviceItem->m_proc_CanalGetStatistics = 
				(LPFNDLL_CANALGETSTATISTICS)dlsym( pDeviceStruct->pDeviceItem->m_hinst, 
											"CanalGetStatistics" ) ) )  {
	
		// Free the library	
		dlclose( pDeviceStruct->pDeviceItem->m_hinst );
		syslog( LOG_CRIT, "canald: Unable to get dl entry for CanalGetStatistics.");
		rv = -1;
		pthread_exit( &rv );
	}


	// * * * * CANAL SET FILTER * * * * 

	if ( 0 == ( pDeviceStruct->pDeviceItem->m_proc_CanalSetFilter = 
				(LPFNDLL_CANALSETFILTER)dlsym( pDeviceStruct->pDeviceItem->m_hinst, 
											"CanalSetFilter" ) ) )  {
	
		// Free the library	
		dlclose( pDeviceStruct->pDeviceItem->m_hinst );
		syslog( LOG_CRIT, "canald: Unable to get dl entry for CanalSetFilter.");
		rv = -1;
		pthread_exit( &rv );
	}


	// * * * * CANAL SET MASK * * * * 

	if ( 0 == ( pDeviceStruct->pDeviceItem->m_proc_CanalSetMask = 
				(LPFNDLL_CANALSETMASK)dlsym( pDeviceStruct->pDeviceItem->m_hinst, 
											"CanalSetMask" ) ) )  {
	
		// Free the library	
		dlclose( pDeviceStruct->pDeviceItem->m_hinst );
		syslog( LOG_CRIT, "canald: Unable to get dl entry for CanalSetMask.");
		rv = -1;
		pthread_exit( &rv );
	}


	// * * * * CANAL GET VERSION * * * * 

	if ( 0 == ( pDeviceStruct->pDeviceItem->m_proc_CanalGetVersion = 
				(LPFNDLL_CANALGETVERSION)dlsym( pDeviceStruct->pDeviceItem->m_hinst, 
											"CanalGetVersion" ) ) )  {
	
		// Free the library	
		dlclose( pDeviceStruct->pDeviceItem->m_hinst );
		syslog( LOG_CRIT, "canald: Unable to get dl entry for CanalGetVersion.");
		rv = -1;
		pthread_exit( &rv );
	}


	// * * * * CANAL GET DLL VERSION * * * * 

	if ( 0 == ( pDeviceStruct->pDeviceItem->m_proc_CanalGetDllVersion = 
				(LPFNDLL_CANALGETDLLVERSION)dlsym( pDeviceStruct->pDeviceItem->m_hinst, 
											"CanalGetDllVersion" ) ) )  {
	
		// Free the library	
		dlclose( pDeviceStruct->pDeviceItem->m_hinst );
		syslog( LOG_CRIT, "canald: Unable to get dl entry for CanalGetDllVersion.");
		rv = -1;
		pthread_exit( &rv );
	}


	// * * * * CANAL GET VENDOR STRING * * * * 

	if ( 0 == ( pDeviceStruct->pDeviceItem->m_proc_CanalGetVendorString = 
				(LPFNDLL_CANALGETVENDORSTRING)dlsym( pDeviceStruct->pDeviceItem->m_hinst, 
											"CanalGetVendorString" ) ) )  {
	
		// Free the library	
		dlclose( pDeviceStruct->pDeviceItem->m_hinst );
		syslog( LOG_CRIT, "canald: Unable to get dl entry for CanalGetVendorString.");
		rv = -1;
		pthread_exit( &rv );
	}

	// ******************************
	//     Generation 2 Methods
	// ******************************


	// * * * * CANAL BLOCKING SEND * * * * 

	if ( 0 == ( pDeviceStruct->pDeviceItem->m_proc_CanalBlockingSend = 
				(LPFNDLL_CANALBLOCKINGSEND)dlsym( pDeviceStruct->pDeviceItem->m_hinst, 
											"CanalBlockingSend" ) ) )  {
	
		// This is probably a generation one driver
		pDeviceStruct->pDeviceItem->m_proc_CanalBlockingSend = NULL;
		syslog( LOG_CRIT, "canald: Unable to get dl entry for CanalBlockingSend. Probably Generation 1 driver.");
	}
	

	// * * * * CANAL BLOCKING RECEIVE * * * * 

	if ( 0 == ( pDeviceStruct->pDeviceItem->m_proc_CanalBlockingReceive = 
				(LPFNDLL_CANALBLOCKINGRECEIVE)dlsym( pDeviceStruct->pDeviceItem->m_hinst, 
											"CanalBlockingReceive" ) ) )  {
	
		// This is probably a generation one driver
		pDeviceStruct->pDeviceItem->m_proc_CanalBlockingReceive = NULL;
		syslog( LOG_CRIT, "canald: Unable to get dl entry for CanalBlockingReceive. Probably Generation 1 driver.");
	}

	// * * * * CANAL GET DRIVER INFO * * * * 

	if ( 0 == ( pDeviceStruct->pDeviceItem->m_proc_CanalGetdriverInfo = 
				(LPFNDLL_CANALGETDRIVERINFO)dlsym( pDeviceStruct->pDeviceItem->m_hinst, 
											"CanalGetDriverInfo" ) ) )  {
	
		// This is probably a generation one driver
		pDeviceStruct->pDeviceItem->m_proc_CanalGetdriverInfo = NULL;
		syslog( LOG_CRIT, "canald: Unable to get dl entry for CanalGetDriverInfo. Probably Generation 1 driver.");
	}

	//
	// =====================================================================================
	//

	// Open the device
	openHandle = 
		pDeviceStruct->pDeviceItem->m_proc_CanalOpen( (const char *)pDeviceStruct->pDeviceItem->m_strParameter.mb_str(wxConvUTF8),
												pDeviceStruct->pDeviceItem->m_DeviceFlags );

	// Get Driver Level
	pDeviceStruct->pDeviceItem->m_driverLevel = 
		pDeviceStruct->pDeviceItem->m_proc_CanalGetLevel( openHandle );

	// Get driver properties
	if ( NULL != pDeviceStruct->pDeviceItem->m_proc_CanalGetdriverInfo ) {
		pDeviceStruct->pDeviceItem->m_proc_CanalGetdriverInfo( pDeviceStruct->pDeviceItem->m_openHandle,
																														&pDeviceStruct->pDeviceItem->m_driverProperties );
	}

	/////////////////////////////////////////////////////////////////////////////
	//                              Do Work here
	//								============
	//
	// This thread just tries to read/write from/to the device. 
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

	if ( pDeviceStruct->pDeviceItem->m_driverProperties &  CANAL_DRIVER_PROPERTIES_BLOCKING ) {

		// * * * * Blocking version * * * *

		// Just sit and wait until the end
		while ( !pDeviceStruct->pctrlObject->m_bQuit ) {
			sleep ( 1 );	
		}

	}
	else {
	
		// * * * * Non blocking version * * * *

		// Wait until the device is open
		// WaitForSingleObject( pDeviceStruct->pDeviceItem->m_hstateOpen, INFINITE );
		// ReleaseSemaphore( pDeviceStruct->pDeviceItem->m_hstateOpen, 1, NULL );


		bool bTraffic = true;
		canalMsg *pMsg; 
		struct dllnode *pNode;

		while ( !pDeviceStruct->pctrlObject->m_bQuit ) {	

			/////////////////////////////////////////////////////////////////////////////
			//                                Receive								   								 //
			/////////////////////////////////////////////////////////////////////////////

			if ( 0 < pDeviceStruct->pDeviceItem->m_proc_CanalDataAvailable( openHandle ) ) {
	
				pMsg = new canalMsg;

				if ( ( NULL != pMsg ) && 
							pDeviceStruct->pDeviceItem->m_proc_CanalReceive( openHandle, pMsg ) ) {

					bTraffic = true;
		
					// There must be room in the receive queue
					if ( pDeviceStruct->pctrlObject->m_maxItemsInReceiveQueue > 
						 			pDeviceStruct->pctrlObject->m_deviceInputQueue.GetCount() ) {

						struct dllnode *pNode = new dllnode;
						if ( NULL != pNode ) {
							
							pNode->obid = pDeviceStruct->pDeviceItem->m_deviceID; 		// mark as from device

							pNode->Key = pMsg->id;
							pNode->pKey = &pNode->Key;
							pNode->pObject = pMsg;

							pctrlObject->m_mutexDeviceInputQueue.Lock();
							pDeviceStruct->pctrlObject->m_deviceInputQueue.Append( pMsg );
							pctrlObject->m_mutexDeviceInputQueue.Unlock();

						}
						else {
							delete pMsg;
						}
					}
				
				}
			}

			// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
			//             Send messages (if any) in the outqueue
			// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

			pNode = pDeviceStruct->pDeviceItem->m_outputQueue.pHead;
			while ( NULL != pNode ) {
			
				bTraffic = true;

				if ( NULL != ( pMsg = (canalMsg *)pNode->pObject ) ) { 
					pDeviceStruct->pDeviceItem->m_proc_CanalSend( openHandle, pMsg );
				}
			
				pctrlObject->m_wxDeviceMutex.Lock();
				dll_removeNode( &pDeviceStruct->pDeviceItem->m_outputQueue, pNode );
				pctrlObject->m_wxDeviceMutex.Unlock();

				pNode = pDeviceStruct->pDeviceItem->m_outputQueue.pHead;

			} // While messages in sendqueue
			
			if ( !bTraffic ) {
				usleep( 100 );
			}

			bTraffic = false;

		} // while working		
   
	} // if blocking/non blocking

	// Close the device
	pDeviceStruct->pDeviceItem->m_proc_CanalClose( openHandle );	

	// Free the library
	dlclose( pDeviceStruct->pDeviceItem->m_hinst );

	// Delete the device structure
	delete (deviceStruct *)pDeviceInfo;	
		
	// Linux code
	// TODO Remove - Just here for warning removals
	pthread_exit( &rv );
	
}




