// ControlObject.cpp: implementation of the CControlObject class.
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
// $RCSfile: controlobject.cpp,v $                                       
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


#ifdef WIN32

// Prototypes
void listenThread( void *p );
void clientThread( void *p );
void terminationThread( void *p );
void udpReceiveThread( void *p );

#else 		// U N I X

//typedef void *HANDLE;
typedef unsigned long DWORD;
#define _POSIX
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

// Client List Mutex
static pthread_mutex_t gclientMutex = PTHREAD_MUTEX_INITIALIZER;

// Device List Mutex
static pthread_mutex_t  gdeviceMutex = PTHREAD_MUTEX_INITIALIZER;
	
// Send Mutex
static pthread_mutex_t  gsendMutex = PTHREAD_MUTEX_INITIALIZER;

// Receive Mutex
static pthread_mutex_t  gudpReceiveMutex = PTHREAD_MUTEX_INITIALIZER;

// Prototypes
void *listenThread( void *p );
void *clientThread( void *p );
void *terminationThread( void *p );
void *udpReceiveThread( void *p );

#endif

#include "vscp.h"
#include "vscppipe.h"
#include "ControlObject.h"
 
//#define DEBUGPRINT   

 
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CControlObject::CControlObject()
{
	m_bQuit = false;		// true if we should quit
	m_bCanalOpen = false;	// No canal pipe active
#ifdef WIN32
	m_sendsock = NULL;
#else
	m_sendsock = 0;
#endif
 
	// Initilize double linked lists for the send/receive queues
	dll_init( &m_sendQueue, SORT_NONE );		
	dll_init( &m_udpReceiveQueue, SORT_NONE );

	// Check if the CANAL daemon is 
	// available

	if ( m_canalPipe.doCmdOpen( NULL, 0 ) ) {

		if ( m_canalPipe.doCmdNOOP() ) {
			// success
			m_bCanalOpen = true;
		}

	}
	
}

// Destructor

CControlObject::~CControlObject()
{
	// We have to clear the messages from the queues
	dll_removeAllNodes( &m_sendQueue );
	dll_removeAllNodes( &m_udpReceiveQueue );

	// Close the UDP socket
#ifdef WIN32	
	if ( NULL != m_sendsock ) closesocket( m_sendsock );
#else
	if ( !m_sendsock ) close( m_sendsock );
#endif

	// Close the pipe interface
	if ( m_bCanalOpen ) {
		m_canalPipe.doCmdClose();	
	}
}

/////////////////////////////////////////////////////////////////////////////
// init

bool CControlObject::init( void )
{
	DWORD dwThreadId;
	int optval; 
    int optlen;
	
	// Get configuration information
	loadRegistryData();	

	// Create the UDP socket.
#ifdef WIN32	
	m_sendsock = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if ( m_sendsock == INVALID_SOCKET ) {
		// UDP will not be available TODO
	}	
#else	
	m_sendsock = socket( AF_INET, SOCK_DGRAM, 0 );
	if ( m_sendsock == -1 ) {
		// UDP will not be available TODO
	}
#endif
	

	optval=1; 
	optlen = sizeof( int ); 
	if( setsockopt( m_sendsock, SOL_SOCKET,SO_BROADCAST, (char *) &optval,optlen ) ) { 
		// UDP will not be available TODO 
	}
		
	// Create mutex's
#ifdef WIN32	
	
	m_clientMutex = CreateMutex( NULL, true, VSCPD_CLIENT_OBJ_MUTEX );
	m_sendMutex = CreateMutex( NULL, true, VSCPD_SEND_OBJ_MUTEX );
	m_udpReceiveMutex = CreateMutex( NULL, true, VSCPD_RECEIVE_OBJ_MUTEX );
	
	// Create the listen thread that listens for client connections.		
	if ( NULL == CreateThread(	NULL,
								0,
								(LPTHREAD_START_ROUTINE) listenThread,
								this,
								0,
								&dwThreadId ) ) { 
		return false;
	}

	// Create the termination thread (External API).
	if ( NULL == CreateThread(	NULL,
								0,
								(LPTHREAD_START_ROUTINE) terminationThread,
								this,
								0,
								&dwThreadId ) ) { 
		; // termination functionality disabled
	}

	// Create the UDP Receive thread.
	if ( NULL == CreateThread(	NULL,
								0,
								(LPTHREAD_START_ROUTINE) udpReceiveThread,
								this,
								0,
								&dwThreadId ) ) { 
		; // UDP functionality disabled
	}

	// Release all mutex's
	ReleaseMutex( m_clientMutex );
	ReleaseMutex( m_sendMutex );
	ReleaseMutex( m_udpReceiveMutex );
	
#else

	pthread_t threadId;
	pthread_attr_t thread_attr;
	pthread_attr_init( &thread_attr );
	pthread_attr_setdetachstate( &thread_attr, PTHREAD_CREATE_DETACHED );

	// Create the listen thread that listens for client connections.
	if ( pthread_create( 	&threadId,
							&thread_attr,
							listenThread,
							this ) ) {	
					syslog( LOG_CRIT, "Unable to create listen thread. Client turned down.");
				}	
	
	// Create the termination thread (External API).
	if ( pthread_create( 	&threadId,
							&thread_attr,
							terminationThread,
							this ) ) {	
					syslog( LOG_CRIT, "Unable to create termination thread.");
	}

	// Create the UDP Receive thread.
	if ( pthread_create( 	&threadId,
							&thread_attr,
							udpReceiveThread,
							this ) ) {	
					syslog( LOG_CRIT, "Unable to create UDP Receive thread.");
	}
	
#endif
	
	

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// cleanup

bool CControlObject::cleanup( void )
{
	return true;
}


/////////////////////////////////////////////////////////////////////////////
// run - Program main loop
//
// Handles two cases
//
// * A message sent from a client is sent to all *other* clients AND to
//		canald if class < 512. 
// 
// * A message received from the net is sent to all clients and to canald if
//      the class < 512.
// 
// * A message received from canald is always sent to all clients and to the net.
//		
//

bool CControlObject::run( void )
{
	unsigned short cnt = 0;
/*
#ifdef WIN32
#ifndef BUILD_VSCPD_SERVICE
	MSG msg;
#endif
#endif
*/
	bool bTraffic = false;

	while( !m_bQuit ) {

		doWork( &bTraffic );
/*
#ifdef WIN32
#ifndef BUILD_VSCPD_SERVICE
		while ( PeekMessage( &msg, m_pwnd->GetSafeHwnd(), 0, 0, PM_REMOVE ) ) {				
			DispatchMessage( &msg );
		}
#endif
#endif	
*/
		cnt++;
		if ( cnt > 1000 ) {
			cnt = 0;
			// If canal is closed try to open
			if ( !m_bCanalOpen ) {
				if ( m_canalPipe.doCmdOpen( NULL, 0 ) ) {

					if ( m_canalPipe.doCmdNOOP() ) {
						// success
						m_bCanalOpen = true;
					}
				}		
			}
		}
	}

	// Give threads some time to terminate
#ifdef WIN32
	Sleep( 3000 );
#else
	sleep( 3 );
#endif

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// doWork
//

bool CControlObject::doWork( bool *bTraffic )
{
	
	CClientItem *pClientItem;
	dllnode *pNode;
	vscpMsg2 *pvscpMsg;
 
	/////////////////////////////////////////////////////////////////
	// * * * * T R A N S M I T   M E S S A G E  from CLIENT   * * * *
	/////////////////////////////////////////////////////////////////

	// Is there any messages to send from clients. Send it/them to all
	// devices/clients except for itself.	

#ifdef WIN32
	if ( WAIT_OBJECT_0 == WaitForSingleObject( m_sendMutex, 100 ) ) {
#else
	if ( pthread_mutex_lock( &gsendMutex ) ) {
#endif
		
		while ( NULL != ( pNode = m_sendQueue.pHead ) ) {
			
			*bTraffic = true;	// Work on baby....
			pvscpMsg = NULL;

			// VSCP message
			pvscpMsg = (vscpMsg2 *)pNode->pObject;
						
			if ( NULL != pvscpMsg ) {
				
				// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
				// * * * * Send message to all clients (not to ourself ) * * * *
				// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
#ifdef WIN32				
				if ( WAIT_OBJECT_0 == WaitForSingleObject( m_clientMutex, 100 ) ) {
#else
				if ( pthread_mutex_lock( &gclientMutex ) ) {
#endif				

					dllnode *pNodeClient = m_clientList.m_clientList.pHead;
				
					while ( NULL != pNodeClient ) {
										
						pClientItem = (CClientItem *)pNodeClient->pObject;

						// Should not be sent to the same client that originated
						// the message.
						// key of message wrapper is used to store client id
						if ( pClientItem->m_clientID != pNode->obid ) {						 
	
							// If the client queue is full for this client then the
							// client will not receive the message
							if ( pClientItem->m_inputQueue.nCount > MAX_ITEMS_CLIENT_RECEIVE_QUEUE ) {
								pNodeClient = pNodeClient->pNext;
								continue;
							}

							// Aquire the client mutex
#ifdef WIN32							
							if ( WAIT_OBJECT_0 == 
											WaitForSingleObject( pClientItem->m_hclientMutex, 100 ) ) {
#else
							if ( pthread_mutex_lock( &(pClientItem->m_clientMutex) ) ) {
#endif												
																								
								dllnode *pnewclientmsg = new dllnode;
								
								if ( NULL != pnewclientmsg ) {
								
									vscpMsg2 *pnewvscpMsg = new vscpMsg2;
							
									if ( NULL != pnewvscpMsg ) {

										memcpy( pnewvscpMsg, pvscpMsg, sizeof( vscpMsg2 ) );
										pnewclientmsg->Key = 0;
									    pnewclientmsg->pKey = &pNode->Key;
										pnewclientmsg->pObject = pnewvscpMsg;

										// Copy data
										unsigned char *pData = NULL;
										pnewvscpMsg->pdata = NULL;
										if ( pvscpMsg->size > 0 ) {
											pData = new unsigned char[ pvscpMsg->size ];
											if ( NULL != pData ) {
												memcpy( pData, pvscpMsg->pdata, sizeof( pvscpMsg->size ) );	
												// Point at the allocated data
												pnewvscpMsg->pdata = pData;
											}
										}
									
										if ( !dll_addNode( &pClientItem->m_inputQueue, 
																		pnewclientmsg ) ) {
											if ( NULL != pnewvscpMsg->pdata ) delete [] pnewvscpMsg->pdata;
											delete pnewclientmsg;
										}
									}
									else {
										delete pnewclientmsg;
									}
								}
#ifdef WIN32
								ReleaseMutex( pClientItem->m_hclientMutex );
#else
								pthread_mutex_unlock( &(pClientItem->m_clientMutex) );
#endif

							} // client mutex 
						}

						pNodeClient = pNodeClient->pNext;	

					} // client loop

#ifdef WIN32
					ReleaseMutex( m_clientMutex );
#else
					pthread_mutex_unlock( &gclientMutex );
#endif

				} // Mutex Timeout


				// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
				// * * * *             Send message to net  		  	 * * * *
				// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
				sendUdpMessage( pvscpMsg );


				// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
				// * * * *             Send message to canald            * * * *
				// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
				sendCanalMsg( pvscpMsg );				


			} // msg

			// Remove any VSCP data
			if ( NULL != pvscpMsg->pdata ) {
				delete [] pvscpMsg->pdata;
				pvscpMsg = NULL;
			}

			// Remove Message from sendqueue
			dll_removeNode( &m_sendQueue, pNode );

		}

#ifdef WIN32
		ReleaseMutex( m_sendMutex);
#else
		pthread_mutex_unlock( &gsendMutex );
#endif

	}

	/////////////////////////////////////////////////////////////////
	// * * * * R E C E I V E D  M E S S A G E  from net * * * *
	/////////////////////////////////////////////////////////////////

	// Is there any messages to read from the net. Read it/them and send 
	// it/them to all other devices clients.
#ifdef WIN32	
	if ( WAIT_OBJECT_0 == WaitForSingleObject( m_udpReceiveMutex, 100 ) )  {
#else
	if ( pthread_mutex_lock( &gudpReceiveMutex ) ) {
#endif

		while ( NULL != ( pNode = m_udpReceiveQueue.pHead ) ) {
			
			*bTraffic = true;	// Work on baby....
			pvscpMsg = (vscpMsg2 *)pNode->pObject;

			if ( NULL != pvscpMsg ) {
				
				// * * * * * * * * * * * * * * * * * * * * * * 
				// * * * * Send message to all clients * * * *
				// * * * * * * * * * * * * * * * * * * * * * *
			
#ifdef WIN32			
				if ( WAIT_OBJECT_0 == WaitForSingleObject( m_clientMutex, 100 ) ) {
#else
				if ( pthread_mutex_lock( &gclientMutex ) ) {
#endif

					dllnode *pNodeClient = m_clientList.m_clientList.pHead;
					while ( NULL != pNodeClient ) {
					
						// Get the Client wrapper
						pClientItem = (CClientItem *)pNodeClient->pObject;
 					
							// If the client queue is full for this client then the
							// client will not receive the message
							if ( pClientItem->m_inputQueue.nCount > 
															MAX_ITEMS_CLIENT_RECEIVE_QUEUE ) {
								pNodeClient = pNodeClient->pNext;
								continue;
							}

							
							// Aquire the client mutex
#ifdef WIN32							
							if ( WAIT_OBJECT_0 == 
											WaitForSingleObject( pClientItem->m_hclientMutex, 100 ) ) {
#else
							if ( pthread_mutex_lock( &(pClientItem->m_clientMutex) ) ) {
#endif
																									
								// Create a message holder in the double linked list
								dllnode *pnewclientmsg = new dllnode;
						
								if ( NULL != pnewclientmsg ) {
							
									// Create a message
									vscpMsg2 *pnewvscpMsg = new vscpMsg2;
							
									if ( NULL != pnewvscpMsg ) {
							
										// Copy in the new message
										memcpy( pnewvscpMsg, pvscpMsg, sizeof( vscpMsg2 ) );
										pnewclientmsg->Key = 0;
									    pnewclientmsg->pKey = &pNode->Key;	// No sort
										pnewclientmsg->pObject = pnewvscpMsg;

										// Copy data
										unsigned char *pData = NULL;
										pnewvscpMsg->pdata = NULL;
										if ( pvscpMsg->size > 0 ) {
											pData = new unsigned char[ pvscpMsg->size ];
											if ( NULL != pData ) {
												memcpy( pData, pvscpMsg->pdata, sizeof( pvscpMsg->size ) );	
												// Point at the allocated data
												pnewvscpMsg->pdata = pData;
											}
										}
								
										// Add the new node to the double linked list
										if ( !dll_addNode( &pClientItem->m_inputQueue, 
																	pnewclientmsg ) ) {
											if ( NULL != pvscpMsg->pdata ) delete [] pvscpMsg->pdata ;
											delete pnewclientmsg;
										}

									}
									else {
										delete pnewclientmsg;
									}
								}

#ifdef WIN32
								ReleaseMutex( pClientItem->m_hclientMutex );
#else
								pthread_mutex_unlock( &(pClientItem->m_clientMutex) );
#endif

							} // Timeout - Mutex

							pNodeClient = pNodeClient->pNext;

					}  // While
		

				} // originating client - while

#ifdef WIN32
				ReleaseMutex( m_clientMutex );
#else
				pthread_mutex_unlock( &gclientMutex );
#endif

				// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
				// * * * *             Send message to canald            * * * *
				// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
				sendCanalMsg( pvscpMsg );

			} // msg 

			
			// Remove any VSCP data
			if ( NULL != pvscpMsg->pdata ) {
				delete [] pvscpMsg->pdata;
				pvscpMsg = NULL;
			}

			// Remove Message from sendqueue
			dll_removeNode( &m_udpReceiveQueue, pNode );

		}

#ifdef WIN32
		ReleaseMutex( m_udpReceiveMutex);
#else
		pthread_mutex_unlock( &gudpReceiveMutex );
#endif

	}

	/////////////////////////////////////////////////////////////////
	// * * * * R E C E I V E D  M E S S A G E  from canald * * * *
	/////////////////////////////////////////////////////////////////

	// Is there any messages to read from the canald. Read it/them and send 
	// it/them to all other devices clients.
	if ( m_bCanalOpen && m_canalPipe.doCmdDataAvailable() )  {

		canalMsg msg;
		while ( m_canalPipe.doCmdReceive( &msg ) ) {
			
			// Must be VSCP message (Extended)
			if ( msg.flags & CANAL_IDFLAG_EXTENDED ) continue; 

			*bTraffic = true;	// Work on baby....
			vscpMsg2 vscp2Msg;
			char *pData = NULL;

			memset( &vscp2Msg, 0, sizeof( vscpMsg2 ) );

			if ( msg.sizeData > 0 ) {
				
				unsigned char *pData = new unsigned char[ msg.sizeData ];
				
				if ( NULL != pData ) {
				
					vscp2Msg.size = msg.sizeData;
					vscp2Msg.pdata = pData;
					
					for ( int i=0; i<msg.sizeData; i++ ) {
						vscp2Msg.pdata[ i ] = msg.data[ i ];	
					}

				}
			}
			else {
				vscp2Msg.size = 0;	
			}
			
			vscp2Msg.head = (unsigned char)( 0xf0 & ( msg.id >> 20 ) );
			vscp2Msg.vscp_class = (unsigned short)( 0x1ff & ( msg.id >> 16 ) );
			vscp2Msg.vscp_type = (unsigned char)( 0xff & ( msg.id >> 8 ) ); 
			vscp2Msg.address[ 15 ] = (unsigned char)( 0xff & msg.id );
			
			// obid
			unsigned char *p = (unsigned char *)&msg.obid;

#ifdef BIGENDIAN 
			vscp2Msg.address[ 11 ] = *(p+0); 
			vscp2Msg.address[ 12 ] = *(p+1);
			vscp2Msg.address[ 13 ] = *(p+2);
			vscp2Msg.address[ 14 ] = *(p+3);	
#else 
			vscp2Msg.address[ 11 ] = *(p+3);	// Store bigendian
			vscp2Msg.address[ 12 ] = *(p+2);
			vscp2Msg.address[ 13 ] = *(p+1);
			vscp2Msg.address[ 14 ] = *(p+0);
#endif

			// Point at message 
			pvscpMsg = 	&vscp2Msg;

			// * * * * * * * * * * * * * * * * * * * * * * 
			// * * * * Send message to all clients * * * *
			// * * * * * * * * * * * * * * * * * * * * * *
				
#ifdef WIN32				
			if ( WAIT_OBJECT_0 == WaitForSingleObject( m_clientMutex, 100 ) ) {
#else
			if ( pthread_mutex_lock( &gclientMutex ) ) {		
#endif

				dllnode *pNodeClient = m_clientList.m_clientList.pHead;
				while ( NULL != pNodeClient ) {
					
					// Get the Client wrapper
					pClientItem = (CClientItem *)pNodeClient->pObject;
 					
					// If the client queue is full for this client then the
					// client will not receive the message
					if ( pClientItem->m_inputQueue.nCount > 
													MAX_ITEMS_CLIENT_RECEIVE_QUEUE ) {
						pNodeClient = pNodeClient->pNext;
						continue;
					}
							
					// Aquire the client mutex
#ifdef WIN32					
					if ( WAIT_OBJECT_0 == 
									WaitForSingleObject( pClientItem->m_hclientMutex, 
																	100 ) ) {
#else
					if ( pthread_mutex_lock( &(pClientItem->m_clientMutex) ) ) {
#endif
						
						// Create a message holder in the double linked list
						dllnode *pnewclientmsg = new dllnode;
						
						if ( NULL != pnewclientmsg ) {
							
							// Create a message
							vscpMsg2 *pnewvscpMsg = new vscpMsg2;
							
							if ( NULL != pnewvscpMsg ) {
						
								// Copy in the new message
								memcpy( pnewvscpMsg, pvscpMsg, sizeof( vscpMsg2 ) );
								pnewclientmsg->pKey = NULL;	// no sort
								pnewclientmsg->pObject = pnewvscpMsg;

								// Copy data
								unsigned char *pData = NULL;
								pnewvscpMsg->pdata = NULL;
								if ( pvscpMsg->size > 0 ) {
									pData = new unsigned char[ pvscpMsg->size ];
									if ( NULL != pData ) {
										memcpy( pData, pvscpMsg->pdata, sizeof( pvscpMsg->size ) );	
										// Point at the allocated data
										pnewvscpMsg->pdata = pData;
									}
								}
																

								// Add the new node to the double linked list
								if ( !dll_addNode( &pClientItem->m_inputQueue, 
															pnewclientmsg ) ) {
									if ( NULL != pnewvscpMsg->pdata ) delete [] pnewvscpMsg->pdata;
									delete pnewclientmsg;
								}

							}
							else {
								delete pnewclientmsg;
							}
						}

#ifdef WIN32
						ReleaseMutex( pClientItem->m_hclientMutex );
#else
						pthread_mutex_unlock( &(pClientItem->m_clientMutex) );
#endif

					} // Timeout - Mutex

					// Delete data allocation if any
					if ( NULL != pData ) {
						delete pData;		
					}

					pNodeClient = pNodeClient->pNext;

				} // while

				// Delete VSCP data.
				if ( NULL != vscp2Msg.pdata ) delete vscp2Msg.pdata;

			} // client Mutex

#ifdef WIN32				
			ReleaseMutex( m_clientMutex );
#else
			pthread_mutex_unlock( &gclientMutex );
#endif

			
			// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
			// * * * *                  Send message to net		  			 * * * *
			// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
			sendUdpMessage( pvscpMsg );			

		} // canal queue loop --- while

	} // canal data available


	// Relax for a while if no activity
	if ( !*bTraffic ) {
#ifdef WIN32	
		Sleep( 100 );
#endif
	}

	*bTraffic = false;

	return false;	// Run again
}



/////////////////////////////////////////////////////////////////////////////
// loadRegistryData
//

#ifdef WIN32
void CControlObject::loadRegistryData()	
{
	//int i;
	HKEY hk;
	//DWORD lv;
	//DWORD type;
	//unsigned char buf[2048];
	
	if ( ERROR_SUCCESS == RegOpenKeyEx(	HKEY_LOCAL_MACHINE,
											"software\\vscpd\\",
											NULL,
											KEY_ALL_ACCESS,
											&hk ) ) {
				
	
		RegCloseKey( hk );
	}
}


//////////////////////////////////////////////////////////////////////////////
// saveRegistryData
//
// Write persistant data to the registry
//

void CControlObject::saveRegistryData()
{
	// Save data to the registry
	HKEY hk;
	
	if ( ERROR_SUCCESS == RegOpenKeyEx(	HKEY_LOCAL_MACHINE,
											"software",
											NULL,
											KEY_ALL_ACCESS,
											&hk ) ) {

		HKEY hk_opt;
		DWORD disp;
		RegCreateKeyEx( hk, "vscpd", 0L, 
										REG_OPTION_NON_VOLATILE, 
										NULL,
										KEY_ALL_ACCESS,
										NULL,
										&hk_opt,
										&disp );
										RegCloseKey( hk_opt );
		
		// DWORD val;

	

		RegCloseKey( hk );
	}
}
#endif

//////////////////////////////////////////////////////////////////////////////
// addClient
//

void CControlObject::addClient( CClientItem *pClientItem )
{
	m_clientList.addClient( pClientItem );
} 


//////////////////////////////////////////////////////////////////////////////
// removeClient
//

void CControlObject::removeClient( CClientItem *pClientItem )
{
	m_clientList.removeClient( pClientItem );
} 


//////////////////////////////////////////////////////////////////////////////
// sendUdpMessage
//

bool CControlObject::sendUdpMessage( vscpMsg2 * pMsg )
{	
	unsigned char buf[ 512 ];
	unsigned char *p;
	unsigned short size;
	
	// Must be a valid message
	if ( NULL == pMsg ) return false;

	size = pMsg->size + 25;
	if ( size > 512 ) return false;

	// Head
	p = (unsigned char *)&pMsg->head;
	buf[ VSCP_POS_HEAD + 0 ] = *(p+0);

	// VSCP class
	p = (unsigned char *)&pMsg->vscp_class;
	buf[ VSCP_POS_CLASS + 0 ] = *(p+1); 
	buf[ VSCP_POS_CLASS + 1 ] = *(p+0); 

	// VSCP type
	p = (unsigned char *)&pMsg->vscp_type;
	buf[ VSCP_POS_TYPE + 0 ] = *(p+1); 
	buf[ VSCP_POS_TYPE + 1 ] = *(p+0); 

	// Node address
	p = (unsigned char *)&pMsg->address;
	memcpy( &buf[ VSCP_POS_ADDRESS + 0 ], p, 16 );

	// VSCP Size
	p = (unsigned char *)&pMsg->size;
	buf[ VSCP_POS_SIZE + 0 ] = *(p+1); 
	buf[ VSCP_POS_SIZE + 1 ] = *(p+0);

	// Data Max 487 (512- 25) bytes
	// size = sz - command_byte - control_bytes 
	memcpy( &buf[ VSCP_POS_DATA + 0 ], pMsg->pdata, pMsg->size );

	// VSCP CRC
	p = (unsigned char *)&pMsg->crc;
	buf[ VSCP_POS_CRC + 0 ] = *(p+1); 
	buf[ VSCP_POS_CRC + 1 ] = *(p+0);

	//
	// Fill in the address structure for the server
	//

#ifdef WIN32	
	SOCKADDR_IN broadcast_addr;
#else
	struct sockaddr_in broadcast_addr;
#endif

	broadcast_addr.sin_family = AF_INET;								 
	broadcast_addr.sin_addr.s_addr = htonl( INADDR_BROADCAST );
	broadcast_addr.sin_port = htons( VSCP_LEVEL2_UDP_PORT );

	//
	// Send data to the server
	//
	int nRet;

	nRet = sendto( m_sendsock,				
						(const char *)buf,					
						size,			
						 0,		
#ifdef WIN32						 
						( LPSOCKADDR )&broadcast_addr,	
#else
						(sockaddr *)&broadcast_addr,
#endif
						sizeof( broadcast_addr ) ); 

#ifdef WIN32
	if ( SOCKET_ERROR == nRet ) {
#else
	if ( -1 == nRet ) {
#endif
		// Failed
		return false;
	}
 

	if ( !nRet || 
#ifdef WIN32	
			( nRet == SOCKET_ERROR ) || 
#else
			( nRet == -1 ) || 
#endif
			( nRet != size ) ) { 
		//int err = GetLastError();
		return false;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////////
// sendCanalMsg
//

bool CControlObject::sendCanalMsg( vscpMsg2 *pvscpMsg )
{
	// Must be a valid message
	if ( NULL == pvscpMsg ) return false;
	
	if (  m_bCanalOpen &&  ( pvscpMsg->vscp_class < 512 ) ) {

		// Yes this is a Level 1 message

		canalMsg msg;
		msg.obid = 0;	// We are the super host
		msg.flags = CANAL_IDFLAG_EXTENDED;

		if ( pvscpMsg->size <= 8 ) {
			msg.sizeData = (unsigned char)pvscpMsg->size;
		}
		else {
			msg.sizeData = 8;
		}

		unsigned char priority = ( pvscpMsg->head & VSCP_MASK_PRIORITY );

		//unsigned long t1 = (unsigned long)priority << 20;
		//unsigned long t2 = (unsigned long)pvscpMsg->vscp_class << 16;
		//unsigned long t3 = (unsigned long)pvscpMsg->vscp_type << 8;

		msg.id = ( (unsigned long)priority << 20 ) |
							 ( (unsigned long)pvscpMsg->vscp_class << 16 ) |
							 ( (unsigned long)pvscpMsg->vscp_type << 8) |
							 0;		// We are the host of hosts

		if ( NULL != pvscpMsg->pdata ) {
			for ( int i = 0; i < 8; i++ ) {
				msg.data[ i ] = pvscpMsg->pdata[i];
			}
		}
			
		m_canalPipe.doCmdSend( &msg );

	}

	return true;
}