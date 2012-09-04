// clientThread_level1_win32.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2008 Ake Hedman, D of Scandinavia,<akhe@eurosource.se>
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
// $RCSfile: clientthread_level1_win32.cpp,v $                                       
// $Date: 2005/09/08 17:18:26 $                                  
// $Author: akhe $                                              
// $Revision: 1.12 $ 


#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"

// Common includes
#include "canal_macro.h"
#include "canal_win32_ipc.h"
#include "canalshmem_level1_win32.h"
#include "../../common/dllist.h"
#include "canal.h"
#include "version.h"
#include "controlobject.h"


#define VENDOR_STRING	"eurosource, Sweden. http://www.eurosource.se\0"


void clientThreadLevel1( void *pThreadObject )
{
	char buf[ MAX_PATH ];
	char *p;
	canalMsg *pMsg;
	dllnode *pNode;
	CControlObject *pctrlObject = NULL;
	HANDLE hMap;

	wxLogDebug("Level I Client Started");

	// http://msdn.microsoft.com/msdnmag/issues/0300/security/default.aspx
	// here we use a SECURITY_DESCRIPTOR to say
	// that we don't want *any DACL at all*
	SECURITY_DESCRIPTOR sd;
	InitializeSecurityDescriptor( &sd,
									SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&sd, TRUE, 0, FALSE);

	SECURITY_ATTRIBUTES sa = { sizeof sa, &sd, TRUE };

 	  
	_clientstruct clientInfo;
	memcpy( &clientInfo, (_clientstruct *)pThreadObject, sizeof( _clientstruct ) );
	pctrlObject = clientInfo.m_pctrlObject;
  
	// We need to create a clientobject and add this object to the list
	CClientItem *pClientItem = new CClientItem;
	if ( NULL == pClientItem ) {
	
		delete (_clientstruct *)pThreadObject;	// Delete the client structure
		ExitThread( -1 );
 
	}

	wxLogDebug("Level I Client 1");
	
	// This is a Level II Client
	pClientItem->m_ClientLevel = CLIENT_ITEM_LEVEL1;
	
	// We need to get the shared memory
	sprintf( buf, CANAL_LISTEN_CLIENT_SHM_TEMPLATE, clientInfo.m_shmid );
	if ( NULL == ( hMap = 
		::OpenFileMapping( FILE_MAP_WRITE, 
							TRUE, 
							buf ) ) )  {
		delete (_clientstruct *)pThreadObject;	// Delete the client structure
		ExitThread( -1 );
	}

	
	wxLogDebug("Level I Client 5");
	struct __shmCanalLevelI *pLevel1 = 
		(struct __shmCanalLevelI *)MapViewOfFile( hMap, FILE_MAP_WRITE, 0, 0, 0 );

	if ( NULL == pLevel1 ) {
		delete (_clientstruct *)pThreadObject;	// Delete the client structure
		CloseHandle( hMap );
		ExitThread( -1 );	
	}

	wxLogDebug("Level I Client 6");
	void *pDataArea= pLevel1->m_data;
	pLevel1->m_command = CANAL_COMMAND_NOOP;
	


	// Create the client command semaphore
	HANDLE hSemCmd;
	sprintf( buf, CANAL_CLIENT_COMMAND_SEM_TEMPLATE, clientInfo.m_shmid );
	if ( NULL == ( hSemCmd = CreateSemaphore( &sa, 0, 1, buf ) ) ) {
		delete (_clientstruct *)pThreadObject;	// Delete the client structure
		CloseHandle( hMap );
		ExitThread( -1 );	
	}

	wxLogDebug("Level I Client 7");

	// Create the client Done semaphore
	HANDLE hSemDone;
	sprintf( buf, CANAL_CLIENT_DONE_SEM_TEMPLATE, clientInfo.m_shmid );
	if ( NULL == ( hSemDone = CreateSemaphore( &sa, 0, 1, buf ) ) ) {
		delete (_clientstruct *)pThreadObject;	// Delete the client structure
		CloseHandle( hSemCmd );
		CloseHandle( hMap );
		ExitThread( -1 ); 
	}
	
	wxLogDebug("Level I Client 7");
	// Share the command semaphore
	pLevel1->m_semCmd = clientInfo.m_shmid;
				
	// Add the client to the Client List
	pctrlObject->m_wxClientMutex.Lock();
	pctrlObject->addClient( pClientItem );
	pctrlObject->m_wxClientMutex.Unlock();
	
	pLevel1->m_test = 0xaa55;	// Confirm to client that we are here 		

	pClientItem->m_bOpen = true;
	while ( pClientItem->m_bOpen && !pctrlObject->m_bQuit ) {
			
		// Wait for command from client	
		if ( WAIT_OBJECT_0 != WaitForSingleObject( hSemCmd,  500 ) ) {
			continue;
		}
			
		switch( pLevel1->m_command ) {
		
			// * * * *  O P E N  * * * *
			case CANAL_COMMAND_OPEN:
				pLevel1->m_Response = CANAL_RESPONSE_SUCCESS;
				break;
					
			// * * * *  C L O S E  * * * *
			case CANAL_COMMAND_CLOSE:
				pLevel1->m_Response = CANAL_RESPONSE_SUCCESS;	
				pClientItem->m_bOpen = false;	 
				break;
				
			// * * * *  S E N D  * * * *
			case CANAL_COMMAND_SEND:	
				pLevel1->m_Response = CANAL_RESPONSE_SUCCESS;
						
				pNode = new dllnode;					
				if ( NULL != pNode ) {

					pMsg = new canalMsg;	// Create new Canal Message
						
					if ( NULL != pMsg ) {
								
						// Save the originating clients id so
						// this client dont get the message back
						pNode->obid = pClientItem->m_clientID;
								
						// Copy message								
						memcpy( pMsg, pDataArea, sizeof( canalMsg ) );

						// Statistics
						pClientItem->m_statistics.cntTransmitData += 
						pMsg->sizeData;
						pClientItem->m_statistics.cntTransmitFrames++;

						// We use the message id as the sort key for the message
						// in the queue. The high part of this id is the priority
						// for the message.
						// The non-id part should possibly be nulled but this depends
						// on if we regard the message as "sent" or not. If not, a 
						// message with a lower id should be transfered first over the 
						// bus.
								
						pNode->Key = pMsg->id;
						pNode->pKey = &pNode->Key;
						pNode->pObject = pMsg;
								
						// There must be room in the send queue
						if ( pctrlObject->m_maxItemsInSendQueue > 
							pctrlObject->m_sendLevel1Queue.nCount ) {						
							
							pctrlObject->m_wxQ1OutMutex.Lock();	
							if ( !dll_addNode( &pctrlObject->m_sendLevel1Queue, pNode ) ) {
								pLevel1->m_Response = CANAL_RESPONSE_ERROR;	
								pLevel1->m_ResponseCode = CANAL_IFERROR_SEND_STORAGE;
							}
							else {
								pLevel1->m_Response = CANAL_RESPONSE_SUCCESS; 
								SetEvent( pctrlObject->m_hEventSendQ1 );
							}
							pctrlObject->m_wxQ1OutMutex.Unlock();
						}
						else {
						
							pLevel1->m_Response = CANAL_RESPONSE_ERROR;	
							pLevel1->m_ResponseCode = CANAL_IFERROR_BUFFER_FULL;
									
							if ( NULL != pNode ) {
								if ( NULL != pNode->pObject ) {
									delete pMsg;
								}
								delete pNode;
							}
						}
					}
					else {
								
						delete pNode;
								
						pLevel1->m_Response = CANAL_RESPONSE_ERROR;
						pLevel1->m_ResponseCode = CANAL_IFERROR_SEND_MSG_ALLOCATON;

					}
				} // dllNode 
				break;	
				
				
			// * * * *  R E C E I V E  * * * *
			case CANAL_COMMAND_RECEIVE:
									
				if ( !pClientItem->m_bOpen ) {
					pLevel1->m_Response = CANAL_RESPONSE_ERROR;
					pLevel1->m_ResponseCode = CANAL_IFERROR_CHANNEL_CLOSED;	
				}
				else {
 
					pctrlObject->m_wxClientMutex.Lock();	
								
					dllnode *pNode; 
					canalMsg *pMsg = (canalMsg *)pDataArea;
						 
					if ( NULL != ( pNode = pClientItem->m_inputQueue.pHead ) ) {
						
						// There is a message available
						pLevel1->m_Response = CANAL_RESPONSE_SUCCESS;	
						memcpy( pMsg, pNode->pObject, sizeof( canalMsg ) );

						// Remove the node
						dll_removeNode( &pClientItem->m_inputQueue, pNode );

						// If queue empty reset receive flag
						if ( NULL == ( pNode = pClientItem->m_inputQueue.pHead ) ) {
							ResetEvent( pClientItem->m_hEventReceive );
						}

					}
				 		
					pctrlObject->m_wxClientMutex.Unlock();
 		
				}				 
				break;	
				
				
			// * * * *  D A T A  A V A I L A B L E  * * * *
			case CANAL_COMMAND_CHECKDATA:

				if ( !pClientItem->m_bOpen ) {
					pLevel1->m_Response = CANAL_RESPONSE_ERROR;
					pLevel1->m_ResponseCode = CANAL_IFERROR_CHANNEL_CLOSED;	
				}
				else {
					pLevel1->m_Response = CANAL_RESPONSE_SUCCESS;

					unsigned int *pnMsg = (unsigned int *)pDataArea;
					//*pnMsg = dll_getNodeCount( &pClient->m_inputQueue ); 
					*pnMsg = pClientItem->m_inputQueue.nCount;
				}
				break;	
			
			
			// * * * *  S T A T U S  * * * *
			case CANAL_COMMAND_STATUS:

				if ( !pClientItem->m_bOpen ) {
					pLevel1->m_Response = CANAL_RESPONSE_ERROR;
					pLevel1->m_ResponseCode = CANAL_IFERROR_CHANNEL_CLOSED;	
				}
				else {
					pLevel1->m_Response = CANAL_RESPONSE_SUCCESS;

					// Status
					memcpy( pDataArea, &pClientItem->m_status, sizeof( canalStatus ) );
			
				} 
				break;	
				
				
			// * * * *  S T A T I S T I C S  * * * *
			case CANAL_COMMAND_STATISTICS:

				if ( !pClientItem->m_bOpen ) {
					pLevel1->m_Response = CANAL_RESPONSE_ERROR;
					pLevel1->m_ResponseCode = CANAL_IFERROR_CHANNEL_CLOSED;	
				}
				else {
					memcpy( pDataArea, &pClientItem->m_statistics, sizeof( &pClientItem->m_statistics ) );
					pLevel1->m_Response = CANAL_RESPONSE_SUCCESS;
				} 
				break;
				
				
			// * * * *  F I L T E R  * * * *
			case CANAL_COMMAND_FILTER:

				if ( !pClientItem->m_bOpen ) {
					pLevel1->m_Response = CANAL_RESPONSE_ERROR;
					pLevel1->m_ResponseCode = CANAL_IFERROR_CHANNEL_CLOSED;	
				}
				else {
						
					// Set filter
					memcpy(  &pClientItem->m_filter, pDataArea, sizeof( pClientItem->m_filter ) );
					pLevel1->m_Response = CANAL_RESPONSE_SUCCESS;
				}  
				break;	
				
			// * * * *  M A S K  * * * *
			case CANAL_COMMAND_MASK:
					
				if ( !pClientItem->m_bOpen ) {
					pLevel1->m_Response = CANAL_RESPONSE_ERROR;
					pLevel1->m_ResponseCode = CANAL_IFERROR_CHANNEL_CLOSED;	
				}
				else {
						
					// Set mask				
					memcpy( &pClientItem->m_mask, pDataArea, sizeof( pClientItem->m_mask ) );
					pLevel1->m_Response = CANAL_RESPONSE_SUCCESS;

				} 	
				break;	
	
	
			// * * * *  B A U D R A T E  * * * *
			case CANAL_COMMAND_BAUDRATE:

				if ( !pClientItem->m_bOpen ) {
					pLevel1->m_Response = CANAL_RESPONSE_ERROR;
					pLevel1->m_ResponseCode = CANAL_IFERROR_CHANNEL_CLOSED;	
				}
				else {
						
					// Set baudrate
					// ============
					//		We don't have yo do more here the baudrate change
					//		is just simulated in the pipe interface
					pLevel1->m_Response = CANAL_RESPONSE_SUCCESS;
				} 	
				break;
				
			// * * * *  V E R S I O N  * * * *
			case CANAL_COMMAND_VERSION:	
				// Get version
				p = (char *)pDataArea;
				*( p + POS_VSCPD_MAJOR_VERSION ) = CANAL_MAIN_VERSION;
				*( p + POS_VSCPD_MINOR_VERSION ) = CANAL_MINOR_VERSION;
				*( p + POS_VSCPD_SUB_VERSION ) = CANAL_SUB_VERSION;		
				pLevel1->m_Response = CANAL_RESPONSE_SUCCESS; 	
				break;

			// * * * *  D L L - V E R S I O N  * * * *
			case CANAL_COMMAND_DLL_VERSION:
					
				// Get DLL version
				//		This application returns the vscp daemon
				//		version.
				p = (char *)pDataArea;
				*( p + POS_VSCPD_MAJOR_VERSION ) = VSCPD_MAJOR_VERSION; 
				*( p + POS_VSCPD_MINOR_VERSION ) = VSCPD_MINOR_VERSION;
				*( p + POS_VSCPD_SUB_VERSION ) = VSCPD_SUB_VERSION;
				*( p + POS_VSCPD_SUB_VERSION + 1 ) = 0;
				pLevel1->m_Response = CANAL_RESPONSE_SUCCESS; 	
				break;

			// * * * *  V E N D O R  S T R I N G  * * * *
			case CANAL_COMMAND_VENDOR_STRING:
				// Get vendor string				
				pLevel1->m_Response = CANAL_RESPONSE_SUCCESS;	
				p = (char *)pDataArea;
				strcpy( p, VENDOR_STRING );
				break;

			// * * * *  L E V E L  * * * *
			case CANAL_COMMAND_LEVEL:
					
				// Get i/f supported levels
				p = (char *)pDataArea;
				*( p + 0 ) = 0; 
				*( p + 1 ) = 0;
				*( p + 2 ) = 0;
				*( p + 3 ) = CANAL_LEVEL_STANDARD;
				pLevel1->m_Response = CANAL_RESPONSE_SUCCESS; 	
				break;
					
			// * * * *  N O O P  * * * *
			case CANAL_COMMAND_NOOP:
	
				pLevel1->m_Response = CANAL_RESPONSE_SUCCESS;
				break;
				
			default:

				pLevel1->m_Response = CANAL_RESPONSE_ERROR;
				pLevel1->m_ResponseCode = CANAL_IFERROR_UNKNOWN_COMMAND;
				break;			
		
		}

		// We are Done
		ReleaseSemaphore( hSemDone, 1, NULL );
				 
	}	// while running


	// Client not needed anymore
	pctrlObject->removeClient( pClientItem );

	delete (_clientstruct *)pThreadObject;

	CloseHandle( hMap );
	CloseHandle( hSemCmd );
	CloseHandle( hSemDone );
	ExitThread( 0 );
 
}

 

