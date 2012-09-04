// clientThread_control_unix.cpp
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
// $RCSfile: clientthread_control_unix.cpp,v $                                       
// $Date: 2005/03/14 22:58:21 $                                  
// $Author: akhe $                                              
// $Revision: 1.3 $ 

#define _POSIX
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>

#include "wx/wx.h"
#include "wx/defs.h"

// Common includes
#include "pipecmd.h"
#include "../../common/dllist.h"
#include "canal.h"
#include "../daemon/canald.h"
#include "version.h"


void clientThread( void *pThreadObject )
{
	short responseSize;
	CControlObject *pctrlObject = NULL;

	// Linux
	int nRead = 0;
	int nWrite = 0;
	pthread_attr_t thread_attr;
	int hPipe;
	int rv = 0;

	pthread_attr_init( &thread_attr );
	pthread_attr_setdetachstate( &thread_attr, PTHREAD_CREATE_DETACHED );	 

	unsigned long baudrate;
	canalMsg *pMsg;
	dllnode *pNode;
	unsigned char buf[ 2048 ];
	unsigned char responsebuf[ 256 ];
	unsigned char *p;
  
	wxLogDebug("Client Control Thread: Started" );
	  
	_clientstruct clientInfo;
	memcpy( &clientInfo, (_clientstruct *)pThreadObject, sizeof( _clientstruct ) );
	pctrlObject = clientInfo.m_pctrlObject;
  
	// We need to create a clientobject and add this object to the list
	CClientItem *pClient = new CClientItem;
	if ( NULL == pClient ) {
	
		delete (_clientstruct *)pThreadObject;	// Delete the client structure
		
		rv = -1;
		pthread_exit( &rv );
 
	}
	
	// We need to grap the shared memory
	void *po = shmat( clientInfo.m_shmid, NULL, 0 );
	strcpy( (char *)po, "this is a test" );
		
	// Wait for our turn to use the client list
	pthread_mutex_lock( &pctrlObject->m_clientMutex );
 	
	// Add the client to the Client List
	clientInfo.m_pctrlObject->addClient( pClient );
  
	// Release the mutex for other threads to use
	pthread_mutex_unlock( &pctrlObject->m_clientMutex );

	while ( !clientInfo.m_pctrlObject->m_bQuit ) {
/*		
		nRead = 0;

		// Get Data from the client
		if ( ( nRead = read( clientInfo.hPipe, buf, sizeof( buf ) ) ) ) {
 
			pthread_mutex_unlock( &pctrlObject->m_clientMutex );
 
			switch( buf[ CANAL_POS_COMMAND ] ) {

				// * * * *  N O O P  * * * *
				case CANAL_PIPE_COMMAND_NONE:

					responseSize = 1;
					responsebuf[ CANAL_POS_RESPONSE ] = CANAL_PIPE_RESPONSE_SUCCESS;
					break;

				// * * * *  O P E N  * * * *
				case CANAL_PIPE_COMMAND_OPEN:
					
					if ( pClient->m_bOpen ) {
						responseSize = 2;
						responsebuf[ CANAL_POS_RESPONSE ] = CANAL_PIPE_RESPONSE_ERROR;
						responsebuf[ CANAL_POS_RESPONSE_CODE ] = CANAL_PIPE_ERROR_CHANNEL_OPEN;	
					}
					else {
						
						// Copy in the device name
						strncpy( pClient->m_szInterfaceName, (const char *)( buf + 5 ),  
											sizeof( pClient->m_szInterfaceName ) );

						// Respond
						responseSize = 1;
						responsebuf[ CANAL_POS_RESPONSE ] = CANAL_PIPE_RESPONSE_SUCCESS;	
						pClient->m_bOpen = true;
					}
					break;

				// * * * *  C L O S E  * * * *

				case CANAL_PIPE_COMMAND_CLOSE:
					
					if ( !pClient->m_bOpen ) {
						responseSize = 2;
						responsebuf[ CANAL_POS_RESPONSE ] = CANAL_PIPE_RESPONSE_ERROR;
						responsebuf[ CANAL_POS_RESPONSE_CODE ] = CANAL_PIPE_ERROR_CHANNEL_CLOSED;	

						// Remove messages in the client queues
						dll_removeAllNodes( &pClient->m_inputQueue );
					}
					else {
						responseSize = 1;
						responsebuf[ CANAL_POS_RESPONSE ] = CANAL_PIPE_RESPONSE_SUCCESS;	
						pClient->m_bOpen = false;
					}	 
					break;


				// * * * *  S E N D  * * * *

				case CANAL_PIPE_COMMAND_SEND:
					
					if ( !pClient->m_bOpen ) {
						responseSize = 2;
						responsebuf[ CANAL_POS_RESPONSE ] = CANAL_PIPE_RESPONSE_ERROR;
						responsebuf[ CANAL_POS_RESPONSE_CODE ] = CANAL_PIPE_ERROR_CHANNEL_CLOSED;	
					}
					else {
						
						responseSize = 2;
						responsebuf[ CANAL_POS_RESPONSE ] = CANAL_PIPE_RESPONSE_ERROR;
						responsebuf[ CANAL_POS_RESPONSE_CODE ] = CANAL_PIPE_ERROR_GENERAL;

 
						if ( pthread_mutex_lock( &pctrlObject->m_clientMutex ) ) {
 
											
							responseSize = 1;
							responsebuf[ CANAL_POS_RESPONSE ] = CANAL_PIPE_RESPONSE_SUCCESS;
							
							pNode = new dllnode;
					
							if ( NULL != pNode ) {

								pMsg = new canalMsg;	// Create new Canal Message
						
								if ( NULL != pMsg ) {
								
									// Save the originating clients id so
									// this client dont get the message back
									pNode->obid = pClient->m_clientID;

									pClient->m_statistics.cntTransmittData += 
																buf[ CANAL_POS_DATASIZE ];
									pClient->m_statistics.cntTransmittFrames++;
											
									// flags
									p = (unsigned char *)&pMsg->flags;
	
									*(p+3) = buf[ CANAL_POS_FLAGS + 0 ]; 
									*(p+2) = buf[ CANAL_POS_FLAGS + 1 ];
									*(p+1) = buf[ CANAL_POS_FLAGS + 2 ];
									*(p+0) = buf[ CANAL_POS_FLAGS + 3 ];
	
									// id
									p = (unsigned char *)&pMsg->id;
									*(p+3) = buf[ CANAL_POS_ID + 0 ]; 
									*(p+2) = buf[ CANAL_POS_ID + 1 ];
									*(p+1) = buf[ CANAL_POS_ID + 2 ];
									*(p+0) = buf[ CANAL_POS_ID + 3 ];		

									// obid
									p = (unsigned char *)&pMsg->obid;
									*(p+3) = buf[ CANAL_POS_OBID + 0 ]; 
									*(p+2) = buf[ CANAL_POS_OBID + 1 ];
									*(p+1) = buf[ CANAL_POS_OBID + 2 ];
									*(p+0) = buf[ CANAL_POS_OBID + 3 ];

									// timestamp
									p = (unsigned char *)&pMsg->timestamp;
									*(p+3) = buf[ CANAL_POS_TIMESTAMP + 0 ];	
									*(p+2) = buf[ CANAL_POS_TIMESTAMP + 1 ];
									*(p+1) = buf[ CANAL_POS_TIMESTAMP + 2 ];
									*(p+0) = buf[ CANAL_POS_TIMESTAMP + 3 ]; 


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

									pMsg->sizeData = buf[ CANAL_POS_DATASIZE ];

									for ( int i=0; i<8; i++ ) {
										pMsg->data[ i ] = buf[ CANAL_POS_DATA + i ];
									}

									// There must be room in the send queue
									if ( clientInfo.pctrlObject->m_maxItemsInSendQueue > 
												clientInfo.pctrlObject->m_sendQueue.nCount ) {						
								
										if ( !dll_addNode( &clientInfo.pctrlObject->m_sendQueue, pNode ) ) {
											responseSize = 2;
											responsebuf[ CANAL_POS_RESPONSE ] = CANAL_PIPE_RESPONSE_ERROR;	
											responsebuf[ CANAL_POS_RESPONSE_CODE ] = CANAL_PIPE_ERROR_SEND_STORAGE;
										}
										else {
											responseSize = 1;
											responsebuf[ CANAL_POS_RESPONSE ] = CANAL_PIPE_RESPONSE_SUCCESS;
										}
									}
									else {
										responseSize = 2;
										responsebuf[ CANAL_POS_RESPONSE ] = CANAL_PIPE_RESPONSE_ERROR;	
										responsebuf[ CANAL_POS_RESPONSE_CODE ] = CANAL_PIPE_ERROR_BUFFER_FULL;

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

									responseSize = 2;
									responsebuf[ CANAL_POS_RESPONSE ] = CANAL_PIPE_RESPONSE_ERROR;
									responsebuf[ CANAL_POS_RESPONSE_CODE ] = CANAL_PIPE_ERROR_SEND_MSG_ALLOCATON;
								}
							}

 
							pthread_mutex_unlock( &pctrlObject->m_clientMutex );
 

						}	// Timeout - sendMutex
						else {
							;
						}
					} 
					break;

				// * * * *  R E C E I V E  * * * *

				case CANAL_PIPE_COMMAND_RECEIVE:
					
					if ( !pClient->m_bOpen ) {
						responseSize = 2;
						responsebuf[ CANAL_POS_RESPONSE ] = CANAL_PIPE_RESPONSE_ERROR;
						responsebuf[ CANAL_POS_RESPONSE_CODE ] = CANAL_PIPE_ERROR_CHANNEL_CLOSED;	
					}
					else {

 
						if ( pthread_mutex_lock( &pctrlObject->m_clientMutex ) ) {
 						
								
							dllnode *pNode; 
							canalMsg canalMsg;
						 
							if ( NULL != ( pNode = pClient->m_inputQueue.pHead ) ) {

								responseSize = 1 + CANAL_POS_DATA + 8 + 1;
								responsebuf[ CANAL_POS_RESPONSE ] = CANAL_PIPE_RESPONSE_SUCCESS;

								// There is a message available
								memcpy( &canalMsg, pNode->pObject, sizeof( canalMsg ) );

								// Remove the node
								dll_removeNode( &pClient->m_inputQueue, pNode );

								// We have to translate the message to the pipe format

								// Flags
								p = (unsigned char *)&canalMsg.flags;
								responsebuf[ CANAL_POS_FLAGS + 0 ] = *(p+3);	
								responsebuf[ CANAL_POS_FLAGS + 1 ] = *(p+2);
								responsebuf[ CANAL_POS_FLAGS + 2 ] = *(p+1);
								responsebuf[ CANAL_POS_FLAGS + 3 ] = *(p+0);

								// id
								p = (unsigned char *)&canalMsg.id;
								responsebuf[ CANAL_POS_ID + 0 ]= *(p+3);	
								responsebuf[ CANAL_POS_ID + 1 ] = *(p+2);
								responsebuf[ CANAL_POS_ID + 2 ] = *(p+1);
								responsebuf[ CANAL_POS_ID + 3 ] = *(p+0);

								// obid
								p = (unsigned char *)&canalMsg.obid;
								responsebuf[ CANAL_POS_OBID + 0 ] = *(p+3);	
								responsebuf[ CANAL_POS_OBID + 1 ]= *(p+2);
								responsebuf[ CANAL_POS_OBID + 2 ] = *(p+1);
								responsebuf[ CANAL_POS_OBID + 3 ] = *(p+0);
	
								// timestamp
								p = (unsigned char *)&canalMsg.timestamp;
								responsebuf[ CANAL_POS_TIMESTAMP + 0 ] = *(p+3);	
								responsebuf[ CANAL_POS_TIMESTAMP + 1 ]= *(p+2);
								responsebuf[ CANAL_POS_TIMESTAMP + 2 ] = *(p+1);
								responsebuf[ CANAL_POS_TIMESTAMP + 3 ] = *(p+0);

								responsebuf[ CANAL_POS_DATASIZE ] = canalMsg.sizeData;

								for ( int i=0; i<8; i++ ) {
									responsebuf[ CANAL_POS_DATA + i ] = canalMsg.data[ i ];
								}

							}
							else {
								// No message in queue
								responseSize = 2;
								responsebuf[ CANAL_POS_RESPONSE ] = CANAL_PIPE_RESPONSE_ERROR;
								responsebuf[ CANAL_POS_RESPONSE_CODE ] = CANAL_PIPE_ERROR_BUFFER_EMPTY;
							}
						
 
							pthread_mutex_unlock( &pctrlObject->m_clientMutex );
 
							
						} // Timeout - ClientObjectMutex
						else {
							;
						}

					}				 
					break;

				// * * * *  D A T A  A V A I L A B L E  * * * *

				case CANAL_PIPE_COMMAND_CHECKDATA:

					if ( !pClient->m_bOpen ) {
						responseSize = 2;
						responsebuf[ CANAL_POS_RESPONSE ] = CANAL_PIPE_RESPONSE_ERROR;
						responsebuf[ CANAL_POS_RESPONSE_CODE ] = CANAL_PIPE_ERROR_CHANNEL_CLOSED;	
					}
					else {

						responseSize = 5;
						responsebuf[ CANAL_POS_RESPONSE ] = CANAL_PIPE_RESPONSE_SUCCESS;

				 		unsigned int nMsg;
						nMsg = dll_getNodeCount( &pClient->m_inputQueue ); 

						// # of messages in input queue
						p = (unsigned char *)&nMsg;
						responsebuf[ CANAL_POS_NUMBER_OF_MSG + 0 ] = *(p+3);	
						responsebuf[ CANAL_POS_NUMBER_OF_MSG + 1 ] = *(p+2);
						responsebuf[ CANAL_POS_NUMBER_OF_MSG + 2 ] = *(p+1);
						responsebuf[ CANAL_POS_NUMBER_OF_MSG + 3 ] = *(p+0);

					}
					break;

				// * * * *  S T A T U S  * * * *

				case CANAL_PIPE_COMMAND_STATUS:

					if ( !pClient->m_bOpen ) {
						responseSize = 2;
						responsebuf[ CANAL_POS_RESPONSE ] = CANAL_PIPE_RESPONSE_ERROR;
						responsebuf[ CANAL_POS_RESPONSE_CODE ] = CANAL_PIPE_ERROR_CHANNEL_CLOSED;	
					}
					else {

						responseSize = 1 + 4;
						responsebuf[ CANAL_POS_RESPONSE ] = CANAL_PIPE_RESPONSE_SUCCESS;

						// State
						p = (unsigned char *)&pClient->m_status.channel_status;
						responsebuf[ CANAL_POS_STATE + 0 ] = *(p+3);	
						responsebuf[ CANAL_POS_STATE + 1 ] = *(p+2);
						responsebuf[ CANAL_POS_STATE + 2 ] = *(p+1);
						responsebuf[ CANAL_POS_STATE + 3 ] = *(p+0);

					} 
					break;

				// * * * *  S T A T I S T I C S  * * * *

				case CANAL_PIPE_COMMAND_STATISTICS:

					if ( !pClient->m_bOpen ) {
						responseSize = 2;
						responsebuf[ CANAL_POS_RESPONSE ] = CANAL_PIPE_RESPONSE_ERROR;
						responsebuf[ CANAL_POS_RESPONSE_CODE ] = CANAL_PIPE_ERROR_CHANNEL_CLOSED;	
					}
					else {

						// cntReceiveFrames
						p = (unsigned char *)&pClient->m_statistics.cntReceiveFrames;
						responsebuf[ CANAL_POS_CNT_RECEIVE_FRAMES + 0 ] = *(p+3);	
						responsebuf[ CANAL_POS_CNT_RECEIVE_FRAMES + 1 ] = *(p+2);
						responsebuf[ CANAL_POS_CNT_RECEIVE_FRAMES + 2 ] = *(p+1);
						responsebuf[ CANAL_POS_CNT_RECEIVE_FRAMES + 3 ] = *(p+0);

						// cntTransmittFrames
						p = (unsigned char *)&pClient->m_statistics.cntTransmittFrames;
						responsebuf[ CANAL_POS_CNT_TRANSMIT_FRAMES + 0 ] = *(p+3);	
						responsebuf[ CANAL_POS_CNT_TRANSMIT_FRAMES + 1 ] = *(p+2);
						responsebuf[ CANAL_POS_CNT_TRANSMIT_FRAMES + 2 ] = *(p+1);
						responsebuf[ CANAL_POS_CNT_TRANSMIT_FRAMES + 3 ] = *(p+0);

						// cntReceiveData
						p = (unsigned char *)&pClient->m_statistics.cntReceiveData;
						responsebuf[ CANAL_POS_CNT_RECEIVE_DATA + 0 ] = *(p+3);	
						responsebuf[ CANAL_POS_CNT_RECEIVE_DATA + 1 ] = *(p+2);
						responsebuf[ CANAL_POS_CNT_RECEIVE_DATA + 2 ] = *(p+1);
						responsebuf[ CANAL_POS_CNT_RECEIVE_DATA + 3 ] = *(p+0);

						// cntTransmittData
						p = (unsigned char *)&pClient->m_statistics.cntTransmittData;
						responsebuf[ CANAL_POS_CNT_TRANSMIT_DATA + 0 ] = *(p+3);	
						responsebuf[ CANAL_POS_CNT_TRANSMIT_DATA + 1 ] = *(p+2);
						responsebuf[ CANAL_POS_CNT_TRANSMIT_DATA + 2 ] = *(p+1);
						responsebuf[ CANAL_POS_CNT_TRANSMIT_DATA + 3 ] = *(p+0);

						// cntOverruns
						p = (unsigned char *)&pClient->m_statistics.cntOverruns;
						responsebuf[ CANAL_POS_CNT_OVERRUNS + 0 ] = *(p+3);	
						responsebuf[ CANAL_POS_CNT_OVERRUNS + 1 ] = *(p+2);
						responsebuf[ CANAL_POS_CNT_OVERRUNS + 2 ] = *(p+1);
						responsebuf[ CANAL_POS_CNT_OVERRUNS + 3 ] = *(p+0);

						// cntBusWarnings
						p = (unsigned char *)&pClient->m_statistics.cntBusWarnings;
						responsebuf[ CANAL_POS_CNT_BUS_WARNINGS + 0 ] = *(p+3);	
						responsebuf[ CANAL_POS_CNT_BUS_WARNINGS + 1 ] = *(p+2);
						responsebuf[ CANAL_POS_CNT_BUS_WARNINGS + 2 ] = *(p+1);
						responsebuf[ CANAL_POS_CNT_BUS_WARNINGS + 3 ] = *(p+0);

						// cntBusOff
						p = (unsigned char *)&pClient->m_statistics.cntBusOff;
						responsebuf[ CANAL_POS_CNT_BUS_OFF + 0 ] = *(p+3);	
						responsebuf[ CANAL_POS_CNT_BUS_OFF + 1 ] = *(p+2);
						responsebuf[ CANAL_POS_CNT_BUS_OFF + 2 ] = *(p+1);
						responsebuf[ CANAL_POS_CNT_BUS_OFF + 3 ] = *(p+0);						

						responseSize = 1 + 28;
						responsebuf[ CANAL_POS_RESPONSE ] = CANAL_PIPE_RESPONSE_SUCCESS;

					} 
					break;

				// * * * *  F I L T E R  * * * *

				case CANAL_PIPE_COMMAND_FILTER:

					if ( !pClient->m_bOpen ) {
						responseSize = 2;
						responsebuf[ CANAL_POS_RESPONSE ] = CANAL_PIPE_RESPONSE_ERROR;
						responsebuf[ CANAL_POS_RESPONSE_CODE ] = CANAL_PIPE_ERROR_CHANNEL_CLOSED;	
					}
					else {
						
						// Set filter
						p = (unsigned char *)&pClient->m_filter;

						*(p+3) = buf[ CANAL_POS_FILTER + 0 ]; 
						*(p+2) = buf[ CANAL_POS_FILTER + 1 ];
						*(p+1) = buf[ CANAL_POS_FILTER + 2 ];
						*(p+0) = buf[ CANAL_POS_FILTER + 3 ];				
			
						responseSize = 1;
						responsebuf[ CANAL_POS_RESPONSE ] = CANAL_PIPE_RESPONSE_SUCCESS;

					}  
					break;

				// * * * *  M A S K  * * * *

				case CANAL_PIPE_COMMAND_MASK:
					
					if ( !pClient->m_bOpen ) {
						responseSize = 2;
						responsebuf[ CANAL_POS_RESPONSE ] = CANAL_PIPE_RESPONSE_ERROR;
						responsebuf[ CANAL_POS_RESPONSE_CODE ] = CANAL_PIPE_ERROR_CHANNEL_CLOSED;	
					}
					else {
						
						// Set mask	
						
						p = (unsigned char *)&pClient->m_mask;

						*(p+3) = buf[ CANAL_POS_MASK + 0 ]; 
						*(p+2) = buf[ CANAL_POS_MASK + 1 ];
						*(p+1) = buf[ CANAL_POS_MASK + 2 ];
						*(p+0) = buf[ CANAL_POS_MASK + 3 ];				

						responseSize = 1;
						responsebuf[ CANAL_POS_RESPONSE ] = CANAL_PIPE_RESPONSE_SUCCESS;

					} 	
					break;

				// * * * *  B A U D R A T E  * * * *

				case CANAL_PIPE_COMMAND_BAUDRATE:

					if ( !pClient->m_bOpen ) {
						responseSize = 2;
						responsebuf[ CANAL_POS_RESPONSE ] = CANAL_PIPE_RESPONSE_ERROR;
						responsebuf[ CANAL_POS_RESPONSE_CODE ] = CANAL_PIPE_ERROR_CHANNEL_CLOSED;	
					}
					else {
						
						// Set baudrate
						// ============
						//		We don't have yo do more here the baudrate change
						//		is just simulated in the pipe interface

						p = (unsigned char *)&baudrate;

						*(p+3) = buf[ CANAL_POS_BAUDRATE + 0 ]; 
						*(p+2) = buf[ CANAL_POS_BAUDRATE + 1 ];
						*(p+1) = buf[ CANAL_POS_BAUDRATE + 2 ];
						*(p+0) = buf[ CANAL_POS_BAUDRATE + 3 ];				

						responseSize = 1;
						responsebuf[ CANAL_POS_RESPONSE ] = CANAL_PIPE_RESPONSE_SUCCESS;
					} 	
					break;

				// * * * *  V E R S I O N  * * * *

				case CANAL_PIPE_COMMAND_VERSION:
					
					// Get version
					responsebuf[ POS_VSCPD_MAJOR_VERSION ] = CANAL_MAIN_VERSION;
					responsebuf[ POS_VSCPD_MINOR_VERSION ] = CANAL_MINOR_VERSION;
					responsebuf[ POS_VSCPD_SUB_VERSION ] = CANAL_SUB_VERSION;
						
					responseSize = 1 + 3;
					responsebuf[ CANAL_POS_RESPONSE ] = CANAL_PIPE_RESPONSE_SUCCESS; 	
					break;

				// * * * *  D L L - V E R S I O N  * * * *

				case CANAL_PIPE_COMMAND_DLL_VERSION:
					
					// Get DLL version
					//		This application returns the vscp daemon
					//		version.

					responsebuf[ POS_CANAL_MAIN_VERSION ] = CANALD_MAJOR_VERSION; 
					responsebuf[ POS_CANAL_MINOR_VERSION ] = CANALD_MINOR_VERSION;
					responsebuf[ POS_CANAL_SUB_VERSION ] = CANALD_SUB_VERSION;
					responsebuf[ POS_CANAL_SUB_VERSION + 1 ] = 0;

					responseSize = 1 + 4;
					responsebuf[ CANAL_POS_RESPONSE ] = CANAL_PIPE_RESPONSE_SUCCESS; 	
					break;

				// * * * *  V E N D O R  S T R I N G  * * * *

				case CANAL_PIPE_COMMAND_VENDOR_STRING:
					{
						// Get vendor string				
						responseSize = 1 + sizeof( VENDOR_STRING ) + 1;
						responsebuf[ CANAL_POS_RESPONSE ] = CANAL_PIPE_RESPONSE_SUCCESS;	
						responsebuf[ CANAL_POS_RESPONSE + 1 ] = 0;

						char *p = new char[ sizeof( VENDOR_STRING ) + 1 ];

						if ( NULL != p ) {
							strcpy( p, VENDOR_STRING );
							for ( unsigned int i=0; i < sizeof( VENDOR_STRING ) + 1; i++ ) {
								responsebuf[ i + 1 ] = *(p + i );	
							} 

							delete [] p;
						}
					}
					break;

				// * * * *  L E V E L  * * * *

				case CANAL_PIPE_COMMAND_LEVEL:
					
					// Get i/f supported levels

					responsebuf[ 1 ] = 0; 
					responsebuf[ 2 ] = 0;
					responsebuf[ 3 ] = 0;
					responsebuf[ 4 ] = CANAL_LEVEL_STANDARD;

					responseSize = 1 + 4;
					responsebuf[ CANAL_POS_RESPONSE ] = CANAL_PIPE_RESPONSE_SUCCESS; 	
					break;

				default:
					responseSize = 2;
					responsebuf[ CANAL_POS_RESPONSE ] = CANAL_PIPE_RESPONSE_ERROR;
					responsebuf[ CANAL_POS_RESPONSE_CODE ] = CANAL_PIPE_ERROR_UNKNOWN_COMMAND;
					break;
			}

 
			pthread_mutex_unlock( &pctrlObject->m_clientMutex );
 

			// Send response
 
			nWrite = write( clientInfo.hPipe, (char *)responsebuf, responseSize );
 
      
		}
		else {

			// * * * Error during read
 		
			// Inform client of the failure
			responsebuf[ CANAL_POS_RESPONSE ] = CANAL_PIPE_RESPONSE_ERROR;;
			responsebuf[1] = CANAL_PIPE_ERROR_READ_FAILURE;
 
			write( clientInfo.hPipe, (char *)responsebuf, 2 );
 

		} // Read failure
*/  
	}	// while running

	//close( clientInfo.hPipe ); 
  
	// Wait for our turn to use the client list
	pthread_mutex_lock( &pctrlObject->m_clientMutex );

	// Remove messages in the client queues
	dll_removeAllNodes( &pClient->m_inputQueue );
	
	// Remove the client from the Client List
	clientInfo.m_pctrlObject->removeClient( pClient );
  
	// Release the mutex for other threads to use 
	pthread_mutex_unlock( &pctrlObject->m_clientMutex );

	// Remove the share memory
	shmctl( clientInfo.m_shmid, IPC_RMID, 0 );
	
	delete (_clientstruct *)pThreadObject;

	hPipe = 0;
	pthread_exit( &rv );
 
}

#endif

