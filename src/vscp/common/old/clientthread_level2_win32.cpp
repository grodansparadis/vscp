// clientThread_level2_win32.cpp
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
// $RCSfile: clientthread_level2_win32.cpp,v $                                       
// $Date: 2005/09/08 17:18:26 $                                  
// $Author: akhe $                                              
// $Revision: 1.12 $ 

#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"

// Project includes
#include "version.h"
#include "canal.h"
#include "canal_win32_ipc.h"
#include "canal_macro.h"
#include "canalshmem_level2_win32.h"
#include "../../common/dllist.h"
#include "controlobject.h"


void clientThreadLevel2( void *pThreadObject )
{
	char *p;
	char buf[ MAX_PATH ];
	vscpEvent *pEvent;
	dllnode *pNode;
	CControlObject *pctrlObject = NULL;
	HANDLE hMap;

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
	
	// This is a Level II Client
	pClientItem->m_ClientLevel = CLIENT_ITEM_LEVEL2;
	
	// We need to get the shared memory
	sprintf( buf, CANAL_LISTEN_CLIENT_SHM_TEMPLATE, clientInfo.m_shmid );
	if ( NULL == ( hMap = 
		::OpenFileMapping( FILE_MAP_WRITE, 
                        TRUE, 
							          buf ) ) )  {
		delete (_clientstruct *)pThreadObject;	// Delete the client structure
		ExitThread( -1 );
	}
	
	struct __shmCanalLevelII *pLevel2 = 
		(struct __shmCanalLevelII *)MapViewOfFile( hMap, FILE_MAP_WRITE, 0, 0, 0 );

	if ( NULL == pLevel2 ) {
		delete (_clientstruct *)pThreadObject;	// Delete the client structure
		CloseHandle( hMap );
		ExitThread( -1 );	
	}

	void *pDataArea= pLevel2->m_data;
	vscpEvent *pEventArea = &pLevel2->m_VSCP_Event;
	pLevel2->m_command = VSCP_COMMAND_NOOP;
	
	
	// Create the client command semaphore
	HANDLE hSemCmd;
	sprintf( buf, CANAL_CLIENT_COMMAND_SEM_TEMPLATE, clientInfo.m_shmid );
	if ( NULL == ( hSemCmd = CreateSemaphore( &sa, 0, 1, buf ) ) ) {
		delete (_clientstruct *)pThreadObject;	// Delete the client structure
		CloseHandle( hMap );
		ExitThread( -1 );	
	}

	// Create the client Done semaphore
	HANDLE hSemDone;
	sprintf( buf, CANAL_CLIENT_DONE_SEM_TEMPLATE, clientInfo.m_shmid );
	if ( NULL == ( hSemDone = CreateSemaphore( &sa, 0, 1, buf ) ) ) {
		delete (_clientstruct *)pThreadObject;	// Delete the client structure
		CloseHandle( hSemCmd );
		CloseHandle( hMap );
		ExitThread( -1 );	
	}
	
	// Share the command semaphore
	pLevel2->m_semCmd = clientInfo.m_shmid;
				
	// Add the client to the Client List
	pctrlObject->m_wxClientMutex.Lock();
	pctrlObject->addClient( pClientItem );
	pctrlObject->m_wxClientMutex.Unlock();
	
	pLevel2->m_test = 0xaa55;	// Confirm to client that we are here 
				
	pClientItem->m_bOpen = true;
	while ( pClientItem->m_bOpen && !pctrlObject->m_bQuit ) {
			
		// Wait for command from client
		if ( WAIT_OBJECT_0 != WaitForSingleObject( hSemCmd,  500 ) ) {
			continue;
		}
			
		switch( pLevel2->m_command ) {
		
			// * * * *  O P E N  * * * *
			case VSCP_COMMAND_OPEN:

				wxLogTrace("wxTRACE_Canald_LevelII", "ClientThread2: Open");
				pLevel2->m_Response = CANAL_RESPONSE_SUCCESS;
				break;
					
			// * * * *  C L O S E  * * * *
			case VSCP_COMMAND_CLOSE:

				wxLogTrace("wxTRACE_Canald_LevelII", "ClientThread2: Close");
				pLevel2->m_Response = CANAL_RESPONSE_SUCCESS;	
				pClientItem->m_bOpen = false;	 
				break;
				
			// * * * *  S E N D  * * * *
			case VSCP_COMMAND_SEND:
					
				wxLogTrace("wxTRACE_Canald_LevelII", "ClientThread2: Send %X", pClientItem->m_clientID );
				pLevel2->m_Response = CANAL_RESPONSE_SUCCESS;
						
				pNode = new dllnode;
				if ( NULL != pNode ) {

					pEvent = new vscpEvent;		// Create new VSCP Message
						
					if ( NULL != pEvent ) {
					
						// Save the originating clients id so
						// this client dont get the message back
						pNode->obid = pClientItem->m_clientID;
								
						// Copy message								
						memcpy( pEvent, pEventArea, sizeof( vscpEvent ) );
					
						// And data...
						if ( pEvent->sizeData > 0 ) {	
						
							// Copy in data
							pEvent->pdata = new uint8_t[ pEvent->sizeData ];
							if ( NULL != pEvent->pdata ) {
								memcpy( pEvent->pdata, pDataArea ,pEvent->sizeData );
							}
							
						}
						else {
							// No data
							pEvent->pdata = NULL;
						}
						
						// Check if GUID of interface should be used
						// as packet GUID
						if ( pLevel2->m_cmdArg1 ) {
							memcpy( pEvent->GUID, pClientItem->m_GUID, 16 );
						}
						
						// Statistics
						pClientItem->m_statistics.cntTransmitData += pEvent->sizeData;
						pClientItem->m_statistics.cntTransmitFrames++;

						// We use the message id as the sort key for the message
						// in the queue. The high part of this id is the priority
						// for the message.
						// The non-id part should possibly be nulled but this depends
						// on if we regard the message as "sent" or not. If not, a 
						// message with a lower id should be transfered first over the 
						// bus.
								
						pNode->Key = pEvent->head;
						pNode->pKey = &pNode->Key;
						pNode->pObject = pEvent;
								
						// There must be room in the send queue
						if ( pctrlObject->m_maxItemsInSendQueue > 
							pctrlObject->m_sendLevel2Queue.nCount ) {						
							
							pctrlObject->m_wxQ2OutMutex.Lock();
							
							if ( !dll_addNode( &pctrlObject->m_sendLevel2Queue, pNode ) ) {
								pLevel2->m_Response = CANAL_RESPONSE_ERROR;	
								pLevel2->m_ResponseCode = CANAL_IFERROR_SEND_STORAGE;
							}
							else {
								pLevel2->m_Response = CANAL_RESPONSE_SUCCESS;
								SetEvent( pctrlObject->m_hEventSendQ2 );
							}
		
							pctrlObject->m_wxQ2OutMutex.Unlock();
						}
						else {
						
							pLevel2->m_Response = CANAL_RESPONSE_ERROR;	
							pLevel2->m_ResponseCode = CANAL_IFERROR_BUFFER_FULL;
									
							if ( NULL != pNode ) {
								if ( NULL != pNode->pObject ) {
									if ( NULL != pEvent->pdata )  delete [] pEvent->pdata;
									delete pEvent;
								}
								delete pNode;
							}		
						}
					}
					else {
								
						delete pNode;
								
						pLevel2->m_Response = CANAL_RESPONSE_ERROR;
						pLevel2->m_ResponseCode = CANAL_IFERROR_SEND_MSG_ALLOCATON;
						
					}
				} // dllNode  
				break;	
				
				
			// * * * *  R E C E I V E  * * * *
			case VSCP_COMMAND_RECEIVE:
					
				wxLogTrace("wxTRACE_Canald_LevelII", "ClientThread2: Receive - before client lock %X", pClientItem->m_clientID );
				pClientItem->m_wxMsgMutex.Lock();
				
				if ( !pClientItem->m_bOpen ) {
					pLevel2->m_Response = CANAL_RESPONSE_ERROR;
					pLevel2->m_ResponseCode = CANAL_IFERROR_CHANNEL_CLOSED;	
				}
				else {
								
					dllnode *pNode; 
					vscpEvent *pEvent = (vscpEvent *)pEventArea;
										 
					if ( NULL != ( pNode = pClientItem->m_inputQueue.pHead ) ) {
						
						// There is a message available
						pLevel2->m_Response = CANAL_RESPONSE_SUCCESS;	
						
						// Copy message
						memcpy( pEvent, pNode->pObject, sizeof( vscpEvent ) );
						
						// If there is data it also must be copied
						if ( ( pEvent->sizeData > 0 ) && ( NULL != pEvent->pdata ) ) {
							memcpy( pDataArea, pEvent->pdata, pEvent->sizeData );							
						}
						
						// Remove the old data
						if ( NULL != pEvent->pdata ) delete pEvent->pdata;
						pEvent->pdata = NULL;	 // Data stored in message

						// Remove the node
						dll_removeNode( &pClientItem->m_inputQueue, pNode );

						// If queue empty reset receive flag
						if ( NULL == ( pNode = pClientItem->m_inputQueue.pHead ) ) {
							ResetEvent( pClientItem->m_hEventReceive );
						}

					}	
				}	
				
				pClientItem->m_wxMsgMutex.Unlock();
				break;	
				
				
			// * * * *  D A T A  A V A I L A B L E  * * * *
			case VSCP_COMMAND_CHECKDATA:
	 
				//wxLogTrace("wxTRACE_Canald_LevelII", "ClientThread%n: Data Available %X", pClientItem->m_clientID. pClientItem->m_inputQueue.nCount);	
				pClientItem->m_wxMsgMutex.Lock();
			
				if ( !pClientItem->m_bOpen ) {
					pLevel2->m_Response = CANAL_RESPONSE_ERROR;
					pLevel2->m_ResponseCode = CANAL_IFERROR_CHANNEL_CLOSED;	
				}
				else {
					pLevel2->m_Response = CANAL_RESPONSE_SUCCESS;

					unsigned int *pnMsg = (unsigned int *)pDataArea;
					//*pnMsg = dll_getNodeCount( &pClientItem->m_inputQueue ); 		 
					*pnMsg = pClientItem->m_inputQueue.nCount;
				}
				pClientItem->m_wxMsgMutex.Unlock();
				break;	
			
			
			// * * * *  S T A T U S  * * * *
			case VSCP_COMMAND_STATUS:

				if ( !pClientItem->m_bOpen ) {
					pLevel2->m_Response = CANAL_RESPONSE_ERROR;
					pLevel2->m_ResponseCode = CANAL_IFERROR_CHANNEL_CLOSED;	
				}
				else {
					pLevel2->m_Response = CANAL_RESPONSE_SUCCESS;

					// Status
					memcpy( pDataArea, &pClientItem->m_status, sizeof( canalStatus ) );
			
				} 
				break;	
				
				
			// * * * *  S T A T I S T I C S  * * * *
			case VSCP_COMMAND_STATISTICS:

				if ( !pClientItem->m_bOpen ) {
					pLevel2->m_Response = CANAL_RESPONSE_ERROR;
					pLevel2->m_ResponseCode = CANAL_IFERROR_CHANNEL_CLOSED;	
				}
				else {
					memcpy( pDataArea, 
							&pClientItem->m_statistics, 
							sizeof( &pClientItem->m_statistics ) );
					pLevel2->m_Response = CANAL_RESPONSE_SUCCESS;
				} 
				break;
				
				
			// * * * *  F I L T E R  * * * *
			case VSCP_COMMAND_FILTER:

				if ( !pClientItem->m_bOpen ) {
					pLevel2->m_Response = CANAL_RESPONSE_ERROR;
					pLevel2->m_ResponseCode = CANAL_IFERROR_CHANNEL_CLOSED;	
				}
				else {
						
					// Set filter
					memcpy( &pClientItem->m_filter, pDataArea, sizeof( vscpEventFilter ) );
					pLevel2->m_Response = CANAL_RESPONSE_SUCCESS;

				}  
				break;	
	
			
			// * * * *  V E R S I O N  * * * *
			case VSCP_COMMAND_VERSION:
			
				// Get version
				p = (char *)pDataArea;
				*( p + POS_VSCPD_MAJOR_VERSION ) = CANAL_MAIN_VERSION;
				*( p + POS_VSCPD_MINOR_VERSION ) = CANAL_MINOR_VERSION;
				*( p + POS_VSCPD_SUB_VERSION ) = CANAL_SUB_VERSION;
				
				pLevel2->m_Response = CANAL_RESPONSE_SUCCESS; 	
				break;
				
				
			// * * * *  G E T  I / F  G U I D   * * * *
			case VSCP_COMMAND_GET_GUID:				
				memcpy( pDataArea, pClientItem->m_GUID, 16 ); 
				pLevel2->m_Response = CANAL_RESPONSE_SUCCESS; 
				break;
				
				
			// * * * *  S E T  I / F  G U I D   * * * *
			case VSCP_COMMAND_SET_GUID:			
				memcpy( pClientItem->m_GUID, pDataArea, 16 );
				pLevel2->m_Response = CANAL_RESPONSE_SUCCESS; 
				break;	
				
				
			// * * * *  G E T  C H A N N E L  I D   * * * *
			case VSCP_COMMAND_GET_CHID:		
				break;	
			
				
			// * * * *  N O O P  * * * *
			case VSCP_COMMAND_NOOP:
				wxLogTrace("wxTRACE_Canald_LevelII", "ClientThread2: NOOP %X", pClientItem->m_clientID);
				pLevel2->m_Response = CANAL_RESPONSE_SUCCESS;
				break;


			default:
				wxLogTrace("wxTRACE_Canald_LevelII", "ClientThread2: Unknown command %X", pClientItem->m_clientID);
				pLevel2->m_Response = CANAL_RESPONSE_ERROR;
				pLevel2->m_ResponseCode = CANAL_IFERROR_UNKNOWN_COMMAND;
				break;			
		
		}

		// We are Done
		ReleaseSemaphore( hSemDone, 1, NULL );
				 
	}	// while running

	wxLogTrace( "wxTRACE_Canald_LevelII", "ClientThread2: About to end. ClientId = %X", pClientItem->m_clientID );
	 
	// Remove messages in the client queues   
	pctrlObject->removeClient( pClientItem );
	
	wxLogDebug("ClientThread2: Removed client item.");
	
	delete (_clientstruct *)pThreadObject;

	CloseHandle( hMap );
	CloseHandle( hSemCmd );
	CloseHandle( hSemDone );

	wxLogDebug("ClientThread2: Exit.");
	ExitThread( 0 );
 
}

 

