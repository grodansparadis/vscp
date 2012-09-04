// clientThread.cpp
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
// $RCSfile: clientthread.cpp,v $                                       
// $Date: 2005/01/05 12:50:53 $                                  
// $Author: akhe $                                              
// $Revision: 1.2 $ 

#ifdef __GNUG__
    //#pragma implementation
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef __WXMSW__
    #include  "wx/ownerdrw.h"
#endif

#ifdef WIN32	// Win32 specific includes


#include "version.h"

#else			// UNIX specific includes

#define _POSIX
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#endif

// Common includes
#include "pipecmd.h"
#include "../../common/dllist.h"
#include "controlobject.h"
#include "vscp.h"
#include "vscppipe.h"


///////////////////////////////////////////////////////////////////////////////
// clientThread
//
// Every client that connects to the system gets a thread of this type. The
// thread handles all communication with the client.
//
// The client in-queue is filled with messages from other clients and 
// devices in the system.
//

void clientThread( void *pThreadObject )
{

#ifdef WIN32
	
	DWORD errorCode = 0;
	DWORD nRead = 0;
	DWORD nWrite = 0;
	short responseSize;
	HANDLE clientObjMutex;	 
	HANDLE sendMutex;

#else  

	// Linux
	static pthread_mutex_t clientObjMutex = PTHREAD_MUTEX_INITIALIZER;	 
	static pthread_mutex_t sendMutex = PTHREAD_MUTEX_INITIALIZER;
	
	pthread_attr_t thread_attr;
	int Pipe;
	int rv = 0;
	int nRead,nWrite;
	int responseSize;

	pthread_attr_init( &thread_attr );
	pthread_attr_setdetachstate( &thread_attr, PTHREAD_CREATE_DETACHED );	 

#endif
  
	vscpMsg2 *pMsg;
	dllnode *pNode;
	unsigned char buf[ 2048 ];
	unsigned char responsebuf[ 256 ];
	unsigned char *p;
  
	_clientstruct clientInfo;
	memcpy( &clientInfo, (_clientstruct *)pThreadObject, sizeof( _clientstruct ) );
	delete (_clientstruct *)pThreadObject;
  
#ifdef WIN32

	clientObjMutex = OpenMutex( MUTEX_ALL_ACCESS, false, VSCPD_CLIENT_OBJ_MUTEX );
	sendMutex = OpenMutex( MUTEX_ALL_ACCESS, false, VSCPD_SEND_OBJ_MUTEX );

#endif

	// We need to create a clientobject and add this object to the list
	CClientItem *pClient = new CClientItem;
	if ( NULL == pClient ) {

#ifdef WIN32		
		ExitThread( errorCode );
#else
		pthread_exit( &rv );
#endif
	}
		
#ifdef WIN32		
	// Wait for our turn to use the client list
	WaitForSingleObject( clientObjMutex, INFINITE );
#else
	pthread_mutex_lock( &clientObjMutex );
#endif
	
	// Add the client to the Client List
	clientInfo.pctrlObject->addClient( pClient );
  
	// Release the mutex for other threads to use
#ifdef WIN32	
	ReleaseMutex( clientObjMutex );
#else
	pthread_mutex_unlock( &clientObjMutex );
#endif

	while ( !clientInfo.pctrlObject->m_bQuit ) {
		
		nRead = 0;

		// Get Data from the client
#ifdef WIN32		
		if ( ReadFile( clientInfo.hPipe, buf, sizeof( buf ), &nRead, NULL ) ) {
#else
		if ( nRead = read( clientInfo.pipe, buf, sizeof( buf ) ) ) {
#endif
		
#ifdef WIN32		
			WaitForSingleObject( pClient->m_hclientMutex, INFINITE );
#else
			pthread_mutex_lock( &(pClient->m_clientMutex) );
#endif

			switch( buf[ VSCP_POS_COMMAND ] ) {

				// * * * *  N O O P  * * * *

				case VSCP_PIPE_COMMAND_NONE:

					responseSize = 1;
					responsebuf[ VSCP_POS_RESPONSE ] = VSCP_PIPE_RESPONSE_SUCCESS;
					break;

				// * * * *  O P E N  * * * *

				case VSCP_PIPE_COMMAND_OPEN:
					
					if ( pClient->m_bOpen ) {
						responseSize = 2;
						responsebuf[ VSCP_POS_RESPONSE ] = VSCP_PIPE_RESPONSE_ERROR;
						responsebuf[ VSCP_POS_RESPONSE_CODE ] = VSCP_PIPE_ERROR_CHANNEL_OPEN;	
					}
					else {
						
						// Respond
						responseSize = 1;
						responsebuf[ VSCP_POS_RESPONSE ] = VSCP_PIPE_RESPONSE_SUCCESS;	
						pClient->m_bOpen = true;
					}
					break;

				// * * * *  C L O S E  * * * *

				case VSCP_PIPE_COMMAND_CLOSE:
					
					if ( !pClient->m_bOpen ) {
						responseSize = 2;
						responsebuf[ VSCP_POS_RESPONSE ] = VSCP_PIPE_RESPONSE_ERROR;
						responsebuf[ VSCP_POS_RESPONSE_CODE ] = VSCP_PIPE_ERROR_CHANNEL_CLOSED;	
					}
					else {
						responseSize = 1;
						responsebuf[ VSCP_POS_RESPONSE ] = VSCP_PIPE_RESPONSE_SUCCESS;	
						pClient->m_bOpen = false;
					}	 
					break;


				// * * * *  S E N D  * * * *

				case VSCP_PIPE_COMMAND_SEND:
					
					if ( !pClient->m_bOpen ) {
						responseSize = 2;
						responsebuf[ VSCP_POS_RESPONSE ] = VSCP_PIPE_RESPONSE_ERROR;
						responsebuf[ VSCP_POS_RESPONSE_CODE ] = VSCP_PIPE_ERROR_CHANNEL_CLOSED;	
					}
					else {
						
						responseSize = 1;
						responsebuf[ VSCP_POS_RESPONSE ] = VSCP_PIPE_RESPONSE_SUCCESS;

#ifdef WIN32
						if ( WAIT_OBJECT_0 == WaitForSingleObject( sendMutex, 500 ) ) {
#else
						if ( pthread_mutex_lock( &sendMutex ) ) {
#endif
												
							pNode = new dllnode;
					
							if ( NULL != pNode ) {

								char unsigned *pDataArea = NULL;

								pMsg = new vscpMsg2;		// Create new VSCP Message
								unsigned long dsize = 0;	// Datasize
								pMsg->pdata = NULL;

								if ( nRead > 30 ) { // 1 + 25 + 4 + pMsg->size
									dsize = nRead - (1 + 25 + 4);
									if ( dsize > 487 ) dsize = 487;
									pDataArea = new unsigned char[ dsize ];
								}
						
								if ( NULL != pMsg ) {
												
									// Save the originating clients id so
									// this client dont receive the message
									pNode->obid = pClient->m_clientID;
								

#ifdef BIGENDIAN

									// Head
									p = (unsigned char *)&pMsg->vscp_type;
									*(p+0) = buf[ VSCP_POS_HEAD + 0 ];
									
									// VSCP class
									p = (unsigned char *)&pMsg->vscp_class;
									*(p+0) = buf[ VSCP_POS_CLASS + 0 ];
									*(p+1) = buf[ VSCP_POS_CLASS + 1 ]; 

									// VSCP type
									p = (unsigned char *)&pMsg->vscp_type;
									*(p+0) = buf[ VSCP_POS_TYPE + 0 ];
									*(p+1) = buf[ VSCP_POS_TYPE + 1 ];
									
									// Node address
									p = (unsigned char *)&pMsg->address;
									memcpy( p, buf[ VSCP_POS_ADDRESS + 0 ], 16 );									

									// Number of valid data bytes
									p = (unsigned char *)&pMsg->size;
									*(p+0) = buf[ VSCP_POS_SIZE + 0 ];
									*(p+1) = buf[ VSCP_POS_SIZE + 1 ];

									// Data Max 487 (512- 25) bytes
									pMsg->pdata = pDataArea;
									// size = sz - command_byte - control_bytes
									memcpy( pMsg->pdata, buf[ VSCP_POS_DATA + 0 ], pMsg->size ); 
									
									// CRC
									p = (unsigned char *)&pMsg->crc;
									*(p+0) = buf[ VSCP_POS_CRC + 0 ];
									*(p+1) = buf[ VSCP_POS_CRC + 1 ];									  

#else
				
									// Head
									p = (unsigned char *)&pMsg->vscp_type;
									*(p+0) = buf[ VSCP_POS_HEAD + 0 ];
									
									// VSCP class
									p = (unsigned char *)&pMsg->vscp_class;
									*(p+1) = buf[ VSCP_POS_CLASS + 0 ];
									*(p+0) = buf[ VSCP_POS_CLASS + 1 ]; 

									// VSCP type
									p = (unsigned char *)&pMsg->vscp_type;
									*(p+1) = buf[ VSCP_POS_TYPE + 0 ];
									*(p+0) = buf[ VSCP_POS_TYPE + 1 ];
									
									// Node address
									p = (unsigned char *)&pMsg->address;
									memcpy( p, ( buf + VSCP_POS_ADDRESS + 0 ), 16 );

									// Number of valid data bytes
									p = (unsigned char *)&pMsg->size;
									*(p+1) = buf[ VSCP_POS_SIZE + 0 ];
									*(p+0) = buf[ VSCP_POS_SIZE + 1 ];
									
									pMsg->pdata = NULL;

									if ( pMsg->size > 0 ) {
										
										// size = sz - command_byte - control_bytes
										memcpy( pDataArea, ( buf + VSCP_POS_DATA + 0 ), pMsg->size );
										
										// Data Max 487 (512- 25) bytes
										pMsg->pdata = pDataArea;

									}									
									
									// CRC
									p = (unsigned char *)&pMsg->crc;
									*(p+1) = buf[ VSCP_POS_CRC + 0 ];
									*(p+0) = buf[ VSCP_POS_CRC + 1 ];
								 

#endif

									// The numerical id is not used
									//		Set to zero
									pNode->Key = 0;
									pNode->pKey = &pNode->Key;

									pNode->pObject = pMsg;									

									// There must be room in the send queue
									if ( MAX_ITEMS_RECEIVE_QUEUE > clientInfo.pctrlObject->m_sendQueue.nCount ) {						
								
										if ( !dll_addNode( &clientInfo.pctrlObject->m_sendQueue, pNode ) ) {
											responseSize = 2;
											responsebuf[ VSCP_POS_RESPONSE ] = VSCP_PIPE_RESPONSE_ERROR;	
											responsebuf[ VSCP_POS_RESPONSE_CODE ] = VSCP_PIPE_ERROR_SEND_STORAGE;
										}
										else {
											responseSize = 1;
											responsebuf[ VSCP_POS_RESPONSE ] = VSCP_PIPE_RESPONSE_SUCCESS;
										}
									}
									else {

										responseSize = 2;
										responsebuf[ VSCP_POS_RESPONSE ] = VSCP_PIPE_RESPONSE_ERROR;	
										responsebuf[ VSCP_POS_RESPONSE_CODE ] = VSCP_PIPE_ERROR_BUFFER_FULL;

										if ( NULL != pNode ) {
											
											if ( NULL != pNode->pObject ) {
												
												// Delete data
												if ( NULL != pDataArea ) {
													delete [] pDataArea;
												}

												pNode->pObject;
											}

											delete pNode;

										}
									}
								}
								else {
									
									delete pNode;
								 
									responseSize = 2;
									responsebuf[ VSCP_POS_RESPONSE ] = VSCP_PIPE_RESPONSE_ERROR;
									responsebuf[ VSCP_POS_RESPONSE_CODE ] = VSCP_PIPE_ERROR_SEND_MSG_ALLOCATON;
								}
							}

#ifdef WIN32
							ReleaseMutex( sendMutex );
#else
							pthread_mutex_unlock( &sendMutex );
#endif

						}	// Timeout - sendMutex
						else {
							;
						}
					} 
					break;

				// * * * *  R E C E I V E  * * * *

				case VSCP_PIPE_COMMAND_RECEIVE:
					
					if ( !pClient->m_bOpen ) {
						responseSize = 2;
						responsebuf[ VSCP_POS_RESPONSE ] = VSCP_PIPE_RESPONSE_ERROR;
						responsebuf[ VSCP_POS_RESPONSE_CODE ] = VSCP_PIPE_ERROR_CHANNEL_CLOSED;	
					}
					else {

#ifdef WIN32
						if ( WAIT_OBJECT_0 == WaitForSingleObject( clientObjMutex, 500 ) ) {
#else
						if ( pthread_mutex_lock( &clientObjMutex ) ) {
#endif
						
							dllnode *pNode; 
							vscpMsg2 vscpMsg;
							pMsg = &vscpMsg;
						 
							if ( NULL != ( pNode = pClient->m_inputQueue.pHead ) ) {

								responsebuf[ VSCP_POS_RESPONSE ] = VSCP_PIPE_RESPONSE_SUCCESS;

								// There is a message available
								memcpy( &vscpMsg, pNode->pObject, sizeof( vscpMsg2 ) );								

								if ( pMsg->size > 487 ) pMsg->size = 487;

								responseSize = 1 + 25 + 4 + pMsg->size; // cmd + message + timestamp + data

								// We have to translate the message to the pipe format

#ifdef BIGENDIAN 

								// Head
								p = (unsigned char *)&pMsg->vscp_type;
								responsebuf[ VSCP_POS_HEAD + 0 ] = *(p+0);

								// VSCP class
								p = (unsigned char *)&pMsg->vscp_class;
								responsebuf[ VSCP_POS_CLASS + 0 ] = *(p+0); 
								responsebuf[ VSCP_POS_CLASS + 1 ] = *(p+1); 

								// VSCP type
								p = (unsigned char *)&pMsg->vscp_type;
								responsebuf[ VSCP_POS_TYPE + 0 ] = *(p+0); 
								responsebuf[ VSCP_POS_TYPE + 1 ] = *(p+1); 

								// Node address
								p = (unsigned char *)&pMsg->address;
								memcpy( &responsebuf[ VSCP_POS_ADDRESS + 0 ], p, 16 );

								// VSCP Size
								p = (unsigned char *)&pMsg->size;
								responsebuf[ VSCP_POS_SIZE + 0 ] = *(p+0); 
								responsebuf[ VSCP_POS_SIZE + 1 ] = *(p+1);

								// VSCP CRC
								p = (unsigned char *)&pMsg->crc;
								responsebuf[ VSCP_POS_CRC + 0 ] = *(p+0); 
								responsebuf[ VSCP_POS_CRC + 1 ] = *(p+1);																

#else 

								// Head
								p = (unsigned char *)&pMsg->vscp_type;
								responsebuf[ VSCP_POS_HEAD + 0 ] = *(p+0);

								// VSCP class
								p = (unsigned char *)&pMsg->vscp_class;
								responsebuf[ VSCP_POS_CLASS + 0 ] = *(p+1); 
								responsebuf[ VSCP_POS_CLASS + 1 ] = *(p+0); 

								// VSCP type
								p = (unsigned char *)&pMsg->vscp_type;
								responsebuf[ VSCP_POS_TYPE + 0 ] = *(p+1); 
								responsebuf[ VSCP_POS_TYPE + 1 ] = *(p+0); 

								// Node address
								p = (unsigned char *)&pMsg->address;
								memcpy( &responsebuf[ VSCP_POS_ADDRESS + 0 ], p, 16 );

								// VSCP Size
								p = (unsigned char *)&pMsg->size;
								responsebuf[ VSCP_POS_SIZE + 0 ] = *(p+1); 
								responsebuf[ VSCP_POS_SIZE + 1 ] = *(p+0);

								// VSCP CRC
								p = (unsigned char *)&pMsg->crc;
								responsebuf[ VSCP_POS_CRC + 0 ] = *(p+1); 
								responsebuf[ VSCP_POS_CRC + 1 ] = *(p+0);
															
#endif				
								// Data Max 487 (512- 25) bytes
								// size = sz - command_byte - control_bytes 
								memcpy( ( responsebuf + VSCP_POS_DATA + 0 ), pMsg->pdata, pMsg->size );

								// Delete the data (if any)
								if ( NULL != pMsg->pdata ) delete [] pMsg->pdata;					

								// Remove the node
								dll_removeNode( &pClient->m_inputQueue, pNode );

							}
							else {
								// No message in queue
								responseSize = 2;
								responsebuf[ VSCP_POS_RESPONSE ] = VSCP_PIPE_RESPONSE_ERROR;
								responsebuf[ VSCP_POS_RESPONSE_CODE ] = VSCP_PIPE_ERROR_BUFFER_EMPTY;
							}
						
#ifdef WIN32						
							ReleaseMutex( clientObjMutex );
#else
							pthread_mutex_unlock( &clientObjMutex );
#endif

						} // Timeout - ClientObjectMutex
						else {
							;
						}

					}				 
					break;

				// * * * *  D A T A  A V A I L A B L E  * * * *

				case VSCP_PIPE_COMMAND_CHECKDATA:

					if ( !pClient->m_bOpen ) {
						responseSize = 2;
						responsebuf[ VSCP_POS_RESPONSE ] = VSCP_PIPE_RESPONSE_ERROR;
						responsebuf[ VSCP_POS_RESPONSE_CODE ] = VSCP_PIPE_ERROR_CHANNEL_CLOSED;	
					}
					else {

						responseSize = 5;
						responsebuf[ VSCP_POS_RESPONSE ] = VSCP_PIPE_RESPONSE_SUCCESS;

				 		int nMsg;
						nMsg = dll_getNodeCount( &pClient->m_inputQueue ); 

#ifdef BIGENDIAN 

						// # of messages in input queue
						p = (unsigned char *)&nMsg
						responsebuf[ VSCP_POS_NUMBER_OF_MSG + 0 ] = *(p+0); 
						responsebuf[ VSCP_POS_NUMBER_OF_MSG + 1 ] = *(p+1);
						responsebuf[ VSCP_POS_NUMBER_OF_MSG + 2 ] = *(p+2);
						responsebuf[ VSCP_POS_NUMBER_OF_MSG + 3 ] = *(p+3);

#else 

						// # of messages in input queue
						p = (unsigned char *)&nMsg;
						responsebuf[ VSCP_POS_NUMBER_OF_MSG + 0 ] = *(p+3);	
						responsebuf[ VSCP_POS_NUMBER_OF_MSG + 1 ] = *(p+2);
						responsebuf[ VSCP_POS_NUMBER_OF_MSG + 2 ] = *(p+1);
						responsebuf[ VSCP_POS_NUMBER_OF_MSG + 3 ] = *(p+0);

#endif
					}
					break;

				default:
					responseSize = 2;
					responsebuf[ VSCP_POS_RESPONSE ] = VSCP_PIPE_RESPONSE_ERROR;
					responsebuf[ VSCP_POS_RESPONSE_CODE ] = VSCP_PIPE_ERROR_UNKNOWN_COMMAND;
					break;
			}

#ifdef WIN32
			ReleaseMutex( pClient->m_hclientMutex );
#else
			pthread_mutex_unlock( &pClient->m_clientMutex );
#endif

			// Send response
#ifdef WIN32			
			WriteFile( clientInfo.hPipe, (char *)responsebuf, responseSize, &nWrite, NULL );		
#else
			nWrite = write( clientInfo.pipe, (char *)responsebuf, responseSize ); 
#endif
      
		}
		else {

			// * * * Error during read
#ifdef WIN32
			DWORD dwError = GetLastError();

			if ( ( ERROR_HANDLE_EOF == dwError ) || 
					( ERROR_BROKEN_PIPE == dwError ) ) {
				break;
			}
#endif			

			// Inform client of the failure
			responsebuf[ VSCP_POS_RESPONSE ] = VSCP_PIPE_RESPONSE_ERROR;;
			responsebuf[1] = VSCP_PIPE_ERROR_READ_FAILURE;
#ifdef WIN32			
			WriteFile( clientInfo.hPipe, (char *)responsebuf, 2, &nRead, NULL );	
#else
			nRead = write( clientInfo.pipe, (char *)responsebuf, 2 );
#endif

		} // Read failure
  
	}	// while running

#ifdef WIN32  

	FlushFileBuffers( clientInfo.hPipe ); 
	DisconnectNamedPipe( clientInfo.hPipe ); 
	CloseHandle( clientInfo.hPipe );
	
	// Wait for our turn to use the client list
	WaitForSingleObject( clientObjMutex, INFINITE );
	
#else

	close( clientInfo.pipe );
	
	pthread_mutex_lock( &clientObjMutex );
	
#endif
	
	// Remove the client from the Client List
	clientInfo.pctrlObject->removeClient( pClient );

#ifdef WIN32

	// Release the mutex for other threads to use
	ReleaseMutex( clientObjMutex );

	ExitThread( errorCode );

#else

	// Release the mutex for other threads to use
	pthread_mutex_unlock( &clientObjMutex );
	
	pthread_exit( &rv );

#endif
  
}
