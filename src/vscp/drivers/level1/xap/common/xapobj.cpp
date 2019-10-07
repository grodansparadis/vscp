// xapobj.cpp:
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (http://www.vscp.org)
//
// Copyright (C) 2000-2019 Ake Hedman,
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
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

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include <canal.h>
#include <canal_macro.h>

#include "xapobj.h"
#include "dlldrvobj.h"


// Prototypes
#ifdef WIN32
void workThreadTransmit( void *pObject );
void workThreadReceive( void *pObject );
void workThreadxAPClientSend( void *pObject );
#else
void *workThreadTransmit( void *pObject );
void *workThreadReceive( void *pObject );
void *workThreadxAPClientSend( void *pObject );
#endif

#ifdef WIN32

WORD wVersionRequested = MAKEWORD(1,1);      // WSA functions
WSADATA wsaData;                             // WSA functions

#endif


//////////////////////////////////////////////////////////////////////
// xAPClient
//

xAPClient::xAPClient()
{
	m_saClient.sin_port = 0;
#ifdef WIN32
	m_saClient.sin_addr.S_un.S_addr = 0;
#else
	m_saClient.sin_addr.s_addr = 0;
#endif
	m_interval = 0;
	m_bAlive = false;
	m_lastHeartBeat = 0;
}


xAPClient::~xAPClient()
{
	;
}

//////////////////////////////////////////////////////////////////////
// setClientData
//

#ifdef WIN32
void xAPClient::setClientData( SOCKADDR_IN *pClient, short interval )
#else
void xAPClient::setClientData( sockaddr_in *pClient, short interval )
#endif
{
	// Must have a valid pointer
	if ( NULL == pClient ) return;

#ifdef WIN32
	memcpy( (char *)&m_saClient, pClient, sizeof( SOCKADDR_IN ) );
#else
	memcpy( (char *)&m_saClient, pClient, sizeof( struct sockaddr_in ) );
#endif
	m_interval = interval;

	time( &m_lastHeartBeat );

	m_bAlive = true;
}


//////////////////////////////////////////////////////////////////////
// isSameClient
//

#ifdef WIN32
bool xAPClient::isSameClient( SOCKADDR_IN *pClient )
#else
bool xAPClient::isSameClient( sockaddr_in *pClient )
#endif
{
#ifdef WIN32
	if ( ( pClient->sin_addr.S_un.S_addr == m_saClient.sin_addr.S_un.S_addr ) &&
		( pClient->sin_port == m_saClient.sin_port ) ) {
		return true;
	}
#else
	if ( ( pClient->sin_addr.s_addr == m_saClient.sin_addr.s_addr  ) &&
                ( pClient->sin_port == m_saClient.sin_port ) ) {
                return true;
        }
#endif

	return false;
}


//////////////////////////////////////////////////////////////////////
// checkTimeout
//

bool xAPClient::checkTimeout( void )
{
	time_t now;

	// Get current time
	time( &now );

	if ( ( now -  m_lastHeartBeat ) > ( 2 * m_interval ) ) {
		m_bAlive = false;
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////
// CXAPObj
//

CXAPObj::CXAPObj()
{
	m_initFlag = 0;

	// No filter mask
	m_filter = 0;
	m_mask = 0;

	m_bRun = false;

	m_Port = XAP_DEFAULT_PORT;
	m_UID = 0;
	m_UIDextension = 0;

	m_bHubmode = false;	// xAP HUB client

	m_XAP_filter = 0;
	m_XAP_mask = 0;

#ifdef WIN32

	m_hTreadReceive = 0;
	m_hTreadTransmit = 0;

	// Create the device AND LIST access mutexes
	m_xapMutex = CreateMutex( NULL, true, CANAL_DLL_XAPDRV_OBJ_MUTEX );
	m_receiveMutex = CreateMutex( NULL, true, CANAL_DLL_XAPDRV_RECEIVE_MUTEX );
	m_transmitMutex = CreateMutex( NULL, true, CANAL_DLL_XAPDRV_TRANSMIT_MUTEX );
	m_portMutex = CreateMutex( NULL, true, CANAL_DLL_XAPDRV_PORT_MUTEX );

#else

	pthread_mutex_init( &m_xapMutex, NULL );
	pthread_mutex_init( &m_receiveMutex, NULL );
	pthread_mutex_init( &m_transmitMutex, NULL );
	pthread_mutex_init( &m_portMutex, NULL );

#endif

	dll_init( &m_transmitList, SORT_NONE );
	dll_init( &m_receiveList, SORT_NONE );

}

//////////////////////////////////////////////////////////////////////
// ~CXAPObj
//

CXAPObj::~CXAPObj()
{
	close();

	LOCK_MUTEX( m_transmitMutex );
	dll_removeAllNodes( &m_transmitList );

	LOCK_MUTEX( m_receiveMutex );
	dll_removeAllNodes( &m_receiveList );


#ifdef WIN32

	if ( NULL != m_xapMutex ) CloseHandle( m_xapMutex );
	if ( NULL != m_receiveMutex ) CloseHandle( m_receiveMutex );
	if ( NULL != m_transmitMutex ) CloseHandle( m_transmitMutex );
	if ( NULL != m_portMutex ) CloseHandle( m_portMutex );

#else

	pthread_mutex_destroy( &m_xapMutex );
	pthread_mutex_destroy( &m_receiveMutex );
	pthread_mutex_destroy( &m_transmitMutex );
	pthread_mutex_destroy( &m_portMutex );

#endif
}





//////////////////////////////////////////////////////////////////////
// open
//
//
// filename
// -----------------------------------------------------------------------------
// Parameters for the driver as a string on the following form
//
// uid;port
//
// port default to 3639 if not given
// uid defaults to 1681 if not given
//
//
// flags
// -----------------------------------------------------------------------------
//
//

bool CXAPObj::open( const char *szFileName, unsigned long flags )
{
	unsigned long threadIdTx, threadIdRx;
	const char *p;
#ifdef WIN32
	char szDrvParams[ MAX_PATH ];
#else
	char szDrvParams[ PATH_MAX ];
#endif
	m_initFlag = flags;

#ifdef WIN32
	// Initialize winsock layer
	WSAStartup( wVersionRequested, &wsaData );
#endif

	// save parameter string and conbert to upper case
#ifdef WIN32
	strncpy( szDrvParams, szFileName, MAX_PATH );
	 _strupr( szDrvParams );

#else
	strncpy( szDrvParams, szFileName, PATH_MAX );
	//strtoupper( szDrvParams );
#endif

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


	// UID
	p = strtok( szDrvParams, ";" );
	if ( NULL != p ) {
		if ( NULL != p ) {
			if ( ( NULL != strstr( p, "0x" ) ) || ( NULL != strstr( p, "0X" ) )  ) {
				sscanf( p + 2, "%x", &m_UID );
			}
			else {
				m_UID = atoi( p );
			}
		}
	}

	// Port
	p = strtok( NULL, ";" );
	if ( NULL != p ) {
		if ( ( NULL != strstr( p, "0x" ) ) || ( NULL != strstr( p, "0X" ) )  ) {
			sscanf( p + 2, "%x", &m_Port );
		}
		else {
			m_Port = atoi( p );
		}
	}

	// Test if HUB present
	m_bHubmode= testForxAPHub();


	char szHostname[ 80 ];
	gethostname ( szHostname, sizeof( szHostname ) );
	sprintf( m_strxapclass, "eurosrc.vscp.%s", szHostname );

	// Run run run .....
	// (optional (for hard fellow rockers) "...to the hills..."
	m_bRun = true;

#ifdef WIN32

	UNLOCK_MUTEX( m_portMutex );

	// Start read thread
	if ( NULL ==
			( m_hTreadReceive = CreateThread(	NULL,
										0,
										(LPTHREAD_START_ROUTINE) workThreadReceive,
										this,
										0,
										&threadIdRx ) ) ) {
		// Failure
		close();
		return  false;
	}

	SLEEP( 500 );
	LOCK_MUTEX( m_portMutex );

	// Start write thread
	if ( NULL ==
			( m_hTreadTransmit = CreateThread(	NULL,
										0,
										(LPTHREAD_START_ROUTINE) workThreadTransmit,
										this,
										0,
										&threadIdTx ) ) ) {
		// Failure
		close();
		return false;
	}

	// Release the mutex
	UNLOCK_MUTEX( m_xapMutex );
	UNLOCK_MUTEX( m_receiveMutex );
	UNLOCK_MUTEX( m_transmitMutex );
	UNLOCK_MUTEX( m_portMutex );

#else // LINUX


	pthread_attr_t thread_attr;
	pthread_attr_init( &thread_attr );


	// Create the write thread.
	if ( pthread_create( &m_threadIdTransmit,
				&thread_attr,
				workThreadTransmit,
				this ) ) {

		syslog( LOG_ERR, "xapdrv: Unable to create xapdrv write thread.");
		close();
		return false;
	}


	// Create the log receive thread.
	if ( pthread_create( &m_threadIdReceive,
				&thread_attr,
				workThreadReceive,
				this ) ) {

		syslog( LOG_ERR, "xapdrv: Unable to create xapdrv receive thread.");
		close();
		return false;
	}

	// Release the mutex
	pthread_mutex_unlock( &m_xapMutex );

#endif

	return true;
}


//////////////////////////////////////////////////////////////////////
// close
//

bool CXAPObj::close( void )
{
	// Do nothing if already terminated
	if ( !m_bRun ) return false;

	m_bRun = false;

	UNLOCK_MUTEX( m_xapMutex );
	LOCK_MUTEX( m_xapMutex );


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
	pthread_mutex_destroy( &m_xapMutex );

#endif

	return true;
}


//////////////////////////////////////////////////////////////////////
// doFilter
//

bool CXAPObj::doFilter( canalMsg *pcanalMsg )
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

bool CXAPObj::setFilter( unsigned long filter )
{
	m_filter = filter;
	return true;
}


//////////////////////////////////////////////////////////////////////
// setMask
//

bool CXAPObj::setMask( unsigned long mask )
{
	m_mask = mask;
	return true;
}



//////////////////////////////////////////////////////////////////////
// writeMsg
//

bool CXAPObj::writeMsg( canalMsg *pMsg )
{
	bool rv = false;

	if ( NULL != pMsg ) {

		// Must be room for the message
		if ( m_transmitList.nCount < XAP_MAX_SNDMSG ) {

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

bool CXAPObj::readMsg( canalMsg *pMsg )
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

int CXAPObj::dataAvailable( void )
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

bool CXAPObj::getStatistics( PCANALSTATISTICS pCanalStatistics )
{
	// Must be a valid pointer
	if ( NULL == pCanalStatistics ) return false;

	memcpy( pCanalStatistics, &m_stat, sizeof( canalStatistics ) );

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//	getStatus
//

bool CXAPObj::getStatus( PCANALSTATUS pCanalStatus )
{


	return true;
}



///////////////////////////////////////////////////////////////////////////////
//	testForxAPHub
//
//serverAddr.sin_addr.s_addr =inet_addr( "127.0.0.1" );

bool CXAPObj::testForxAPHub( void )
{
	unsigned long errorCode = 0;

	//
	// Create a UDP/IP datagram socket
	//
#ifdef WIN32
	SOCKET theSocket;

	theSocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
        if ( theSocket == INVALID_SOCKET ) {
                ExitThread( errorCode );
        }


        //
        // Fill in the address structure
        //

        SOCKADDR_IN saServer;


        saServer.sin_family = AF_INET;
        saServer.sin_addr.s_addr = htonl( INADDR_ANY );
        saServer.sin_port = htons( m_Port );

        //
        // bind the name to the socket
        //
	int nRet;

        nRet = bind( theSocket,
                                        ( LPSOCKADDR )&saServer,
                                        sizeof (struct sockaddr ) );
        if ( nRet == SOCKET_ERROR ) {
                return false;
        }

        closesocket( theSocket );

#else
	int theSocket;

        theSocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
        if ( theSocket == -1 ) {
       		return false;
        }

        //
        // Fill in the address structure
        //
        struct sockaddr_in saServer;

        saServer.sin_family = AF_INET;
        saServer.sin_addr.s_addr = htonl( INADDR_ANY );
        saServer.sin_port = htons( m_Port );

	//
        // bind the name to the socket
        //
        int nRet;

        nRet = bind( theSocket,
        		( struct sockaddr * )&saServer,
                        sizeof (struct sockaddr ) );
        if ( nRet == -1 ) {
        	return false;
        }

        ::close( theSocket );

#endif

	return true;
}


///////////////////////////////////////////////////////////////////////////////
// getDataValue
//

unsigned long CXAPObj::getDataValue( const char *szData )
{
	unsigned long val;
	char *nstop;

	if ( ( NULL != strchr( szData, 'x' ) ) ||
			( NULL != strchr( szData, 'X' ) ) ) {
		val = strtoul( szData, &nstop, 16 );
	}
	else {
		val = strtoul( szData, &nstop, 10 );
	}

	return val;
}



///////////////////////////////////////////////////////////////////////////////
// workThreadxAPClientSend
//
// Send message to all HUB clients or to an external HUB
//

#ifdef WIN32
void workThreadxAPClientSend( void *pObject )
#else
void *workThreadxAPClientSend( void *pObject )
#endif
{
	int nRet;
	unsigned long errorCode = 0;

	xapsend *pxapsend = (xapsend *)pObject;

	// Create the socket for client xAP messages
#ifdef WIN32
	SOCKET sock;
	sockaddr_in addr;
#else
	int sock;
        struct sockaddr_in addr;
#endif

  memset( &addr, 0, sizeof( addr ) );

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl( INADDR_ANY );
    	addr.sin_port = htons( 0 );

#ifdef WIN32
	if ( SOCKET_ERROR == ( sock = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) ) {

		sock = NULL;
		ExitThread( errorCode );
#else
	if ( -1 == ( sock = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) ) {

                SYSLOG( LOG_ERR, "xapdrv: Failed to create send socket.");

		sock = 0;
		pthread_exit( &errorCode );
#endif
	}


	short nClients = 1; // Default to one for HUB Client mode
	if ( pxapsend->m_bHubmode ) {
		nClients = XAP_MAX_CLIENTS;
	}

	for ( int i=0; i<nClients; i++ ) {


		// Client must be alive
		if ( !pxapsend->m_pxapobj->m_xAPClients[ i ].m_bAlive ) continue;


		//
		// Send data to the client/server
		//

#ifdef WIN32

		nRet = sendto( sock,
						(const char *)pxapsend->m_msgBuf,
						pxapsend->size,
						 0,
						( LPSOCKADDR )&pxapsend->m_pxapobj->m_xAPClients[ i ].m_saClient,
						sizeof( pxapsend->m_pxapobj->m_xAPClients[ i ].m_saClient ) );

		if ( !nRet || ( nRet == SOCKET_ERROR ) || ( nRet != pxapsend->size ) ) {

			;
		}

#else

		nRet = sendto( sock,
						pxapsend->m_msgBuf,
						pxapsend->size,
						0,
						(struct sockaddr*)&pxapsend->m_pxapobj->m_xAPClients[ i ].m_saClient,
						sizeof( pxapsend->m_pxapobj->m_xAPClients[ i ].m_saClient ) );

		if ( ( 0 == nRet ) || ( nRet == -1 ) || ( nRet != pxapsend->size ) ) {
			;
		}

#endif

	} // for


#ifdef WIN32
	closesocket( sock );
	ExitThread( errorCode );
#else
	::close( sock );
	pthread_exit( &errorCode );
#endif
}







///////////////////////////////////////////////////////////////////////////////
// workThreadTransmit
//
// This thread enables client functionality that is a xAP HUB must be
// present in the system.
//

#ifdef WIN32
void workThreadTransmit( void *pObject )
#else
void *workThreadTransmit( void *pObject )
#endif
{

#ifdef WIN32
	HANDLE hTreadxAPClientSend;
#else
	pthread_t hTreadxAPClientSend;
	int rv = 0;
#endif

	int nRet;
	char buf[2048], wrkbuf[1024];
 	unsigned long errorCode = 0;
	time_t hb_start,hb_now;	// Hearbeat-times
	hb_start = 0;

	CXAPObj *pctrlObject = ( CXAPObj *)pObject;

	if ( NULL == pctrlObject ) {
#ifdef WIN32
		ExitThread( errorCode ); // Fail
#else
		pthread_exit( &rv );
#endif
	}

	// Create the socket for client xAP messages
#ifdef WIN32
	SOCKET sock;
	sockaddr_in txaddr;
#else
	int sock;
        struct sockaddr_in txaddr;
#endif

  memset( &txaddr, 0, sizeof( txaddr ) );

	txaddr.sin_family = AF_INET;
	txaddr.sin_addr.s_addr = htonl( INADDR_ANY );
    txaddr.sin_port = htons( 0 );

	if ( -1 != ( sock = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) ) {

		int on = 1;
		setsockopt ( sock,
						SOL_SOCKET,
						SO_BROADCAST,
						( const char* ) &on,
						sizeof ( on ) );

	}
	else {
		//SYSLOG( LOG_ERR, "canald: Failed to create send socket. Functionality disabled.");
#ifdef WIN32
		sock = NULL;
		ExitThread( errorCode );
#else
		sock = 0;
		pthread_exit( &rv );
#endif
	}

#ifdef WIN32
	SOCKADDR_IN broadcast_addr;
#else
	struct sockaddr_in broadcast_addr;
#endif

	memset( &broadcast_addr, 0, sizeof( broadcast_addr ) );
	broadcast_addr.sin_family = AF_INET;
	broadcast_addr.sin_addr.s_addr = htonl( INADDR_BROADCAST );
	broadcast_addr.sin_port = htons( XAP_DEFAULT_PORT );

	char szHBeat[ 512 ];
	sprintf( szHBeat,
			"xap-hbeat\n{\nv=12\nhop=1\nuid=FF%04X00\nclass=xap-hbeat.alive\nsource=%s\ninterval=60\nport=%d\npid=%d\n}",
			pctrlObject->m_UID,
			pctrlObject->m_strxapclass,
			pctrlObject->m_Port,
#ifdef WIN32
			pctrlObject->m_hTreadTransmit );
#else
			(unsigned int)pctrlObject->m_threadIdTransmit );
#endif

	// Send the initial heartbeat
	nRet = sendto( sock,
					(const char *)szHBeat,
					strlen( szHBeat ),
					 0,
					( struct sockaddr * )&broadcast_addr,
					sizeof( broadcast_addr ) );

	time( &hb_start );

	short nClients = 1; // Default to one for HUB Client mode
	if ( pctrlObject->m_bHubmode ) {
		nClients = XAP_MAX_CLIENTS;
	}


	while ( pctrlObject->m_bRun ) {

		LOCK_MUTEX( pctrlObject->m_xapMutex );

		// Nothing to do if we should end...
		if ( !pctrlObject->m_bRun ) continue;

		// Is there something to transmit
		while ( ( NULL != pctrlObject->m_transmitList.pHead ) &&
				( NULL != pctrlObject->m_transmitList.pHead->pObject ) ) {

			canalMsg msg;
			memcpy( &msg, pctrlObject->m_transmitList.pHead->pObject, sizeof( canalMsg ) );
			LOCK_MUTEX( pctrlObject->m_transmitMutex );
			dll_removeNode( &pctrlObject->m_transmitList, pctrlObject->m_transmitList.pHead );
			UNLOCK_MUTEX( pctrlObject->m_transmitMutex );

			// Must be extended message to be Level I VSCP message
			if ( msg.flags | CANAL_IDFLAG_EXTENDED ) {

				char strdata[20];

				*strdata = 0;
				for ( int i= 0; i< msg.sizeData; i++ ) {
					sprintf( wrkbuf, "%02X", msg.data[ i ] );
					strcat( strdata, wrkbuf );
				}

				sprintf( wrkbuf,
							"xap-header\n{\nv=12\nhop=1\nuid=FF%04X00\nclass=VSCP.level1.event\nsource=%s\n}\n",
							pctrlObject->m_UID,
							pctrlObject->m_strxapclass );

				sprintf( buf,
							"%s\n\nVSCP.level1.event\n{\nvscp-class=%d\nvscp-type=%d\nnickname=%d\nhardcoded=%s\npriority=%d\ndata=%s\n}\n",
							wrkbuf,
							(unsigned int)( ( msg.id >> 16 ) & 0x1ff ),				// class
							(unsigned int)( ( msg.id >> 8 ) & 0xff ),					// type
							(unsigned int)( msg.id & 0xff ),							// nickname
							(unsigned int)( ( msg.id >> 25 ) & 0x01 ) ? "yes" : "no",	// hardcodes
							(unsigned int)( ( msg.id >> 26 ) & 0x07 ), 				// priority
							strdata );

				// Broadcast message
				nRet = sendto( sock,
								(const char *)buf,
								strlen( buf ),
								0,
								( struct sockaddr * )&broadcast_addr,
								sizeof( broadcast_addr ) );


				// If in HUB mode send to clients
				if ( pctrlObject->m_bHubmode ) {

					//////////////////////////////
					//      Send to clients
					//////////////////////////////

					// Create transmission structure
					xapsend *pxapsend = new xapsend;
					if ( NULL != pxapsend ) {

						pxapsend->m_bHubmode = pctrlObject->m_bHubmode;
						pxapsend->m_pxapobj = pctrlObject;
						pxapsend->size = nRet;
						memcpy( pxapsend->m_msgBuf, buf, sizeof( pxapsend->m_msgBuf ) );

						// OK we have a xAP message
						// Send it to all clients

#ifdef WIN32

						// Start client write thread
						DWORD threadId;
						if ( NULL ==
							( hTreadxAPClientSend =
								CreateThread( NULL,
												0,
												(LPTHREAD_START_ROUTINE) workThreadxAPClientSend,
												pxapsend,
												0,
												&threadId ) ) ) {
							// Failure

						}


#else // LINUX
						pthread_attr_t thread_attr;
						pthread_attr_init( &thread_attr );


						// Create the xAP transmit write thread.
						if ( pthread_create( &hTreadxAPClientSend,
									&thread_attr,
									workThreadxAPClientSend,
									pxapsend ) ) {

							syslog( LOG_ERR, "xapObj: Unable to create xapdrv write thread.");
							rv = false;
						}
#endif

						// Wait for Send thread to terminate.
#ifdef WIN32
						DWORD exitcode;
						time_t start,now;

						time( &start );	// Get start time
						time( &now );

						while ( ( (now - start) < 5 ) ) {

							GetExitCodeThread( hTreadxAPClientSend, &exitcode );
							if ( STILL_ACTIVE == exitcode ) break;
								SLEEP( 100 );
								time( &now );
						}
#else


#endif


					}

				} // HUB Mode

			} // Extended message

		} // while data


		// No data to write

		UNLOCK_MUTEX( pctrlObject->m_xapMutex );


		time( &hb_now );
		if ( ( hb_now - hb_start ) > XAP_HEARTBEAT_TIME ) {

			// It's time to send a heartbeat
			nRet = sendto( sock,
							(const char *)szHBeat,
							strlen( szHBeat ),
							0,
							( struct sockaddr * )&broadcast_addr,
							sizeof( broadcast_addr ) );

			if ( pctrlObject->m_bHubmode ) {

				//////////////////////////////
				//      Send to clients
				//////////////////////////////

				// Create transmission structure
				xapsend *pxapsend = new xapsend;
				if ( NULL != pxapsend ) {

					pxapsend->m_bHubmode = pctrlObject->m_bHubmode;
					pxapsend->m_pxapobj = pctrlObject;
					pxapsend->size = strlen( szHBeat );
					memcpy( pxapsend->m_msgBuf, szHBeat, strlen( szHBeat ) );

					// OK we have a xAP message
					// Send it to all clients

#ifdef WIN32

					// Start client write thread
					DWORD threadId;
					if ( NULL ==
							( hTreadxAPClientSend =
							CreateThread( NULL,
											0,
											(LPTHREAD_START_ROUTINE) workThreadxAPClientSend,
											pxapsend,
											0,
											&threadId ) ) ) {
						// Failure
					}

#else // LINUX


					pthread_attr_t thread_attr;
					pthread_attr_init( &thread_attr );


					// Create the xAP write thread.
					if ( pthread_create( &hTreadxAPClientSend,
								&thread_attr,
								workThreadxAPClientSend,
								pxapsend ) ) {

						syslog( LOG_ERR, "xapObj: Unable to create xapdrv write thread.");
						rv = false;
					}
#endif

					// Wait for Send thread to terminate.
#ifdef WIN32
					DWORD exitcode;
					time_t start,now;

					time( &start );	// Get start time
					time( &now );

					while ( ( (now - start) < 5 ) ) {

						GetExitCodeThread( hTreadxAPClientSend, &exitcode );
						if ( STILL_ACTIVE == exitcode ) break;
							SLEEP( 100 );
							time( &now );
					}
#else


#endif
				} // Structure

			} // HUB mode

			time( &hb_start );
		}

		SLEEP( 1 );

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
// This is the receive thread for both client and HUB mode. A xAP message is
// received in both cases and feed to the HUB thread in HUB mode and parsed and
// stored as a VSCP event into the input queue in both cases if its a valid VSCP
// message.
//

#ifdef WIN32
void workThreadReceive( void *pObject )
#else
void *workThreadReceive( void *pObject )
#endif
{
	unsigned long errorCode = 0;
	unsigned long nRead = 0;
	unsigned long nWrite = 0;
	int nLen;
	char buf[2048], wbuf[2048];
	char *p, *pStart, *pEnd;

#ifdef WIN32
	HANDLE hTreadxAPClientSend;
#else
	pthread_t hTreadxAPClientSend;
	char *pp;
#endif

	CXAPObj * pctrlObject = ( CXAPObj *)pObject;
	if ( NULL == pctrlObject ) {

#ifdef WIN32
		ExitThread( errorCode ); // Fail
#else
		pthread_exit( &errorCode );
#endif
	}

	// Hold the Transmit thread back until we got a
	// socket for listening
	LOCK_MUTEX( pctrlObject->m_portMutex );

	char szHostname[ 80 ];
	gethostname ( szHostname, sizeof( szHostname ) );

#ifdef WIN32
	LPHOSTENT lpLocalHostEntry;
#else
	hostent *lpLocalHostEntry;
#endif
	lpLocalHostEntry = gethostbyname( szHostname );
	if ( NULL == lpLocalHostEntry ) {
		UNLOCK_MUTEX( pctrlObject->m_portMutex );
#ifdef WIN32
		ExitThread( errorCode );
#else
		 pthread_exit( &errorCode );
#endif
	}

	// Get all local addresses
	int idx = -1;
	void *pAddr;
	unsigned long localaddr[ 16 ]; // max 16 local addresses
	do {
		idx++;
		localaddr[ idx ] = 0;
		pAddr = lpLocalHostEntry->h_addr_list[ idx ];
		if ( NULL != pAddr ) localaddr[ idx ] = *( ( unsigned long *)pAddr);
	} while ( ( NULL != pAddr ) && ( idx < 16 )  );

	// If we are in HUB mode we should create a
	// broadcast socket for client messages

#ifdef WIN32
	SOCKET bcastsock;
			SOCKADDR_IN broadcast_addr;
#else
	int bcastsock;
			struct sockaddr_in broadcast_addr;
#endif
	if ( pctrlObject->m_bHubmode ) {


		if ( -1 != ( bcastsock = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) ) {

			int on = 1;
			setsockopt ( bcastsock,
							SOL_SOCKET,
							SO_BROADCAST,
							( const char* ) &on,
							sizeof ( on ) );
		}
		else {
			//SYSLOG( LOG_ERR, "canald: Failed to create send socket. Functionality disabled.");
#ifdef WIN32
			bcastsock = NULL;
			ExitThread( errorCode );
#else
			bcastsock = 0;
			pthread_exit( &errorCode );
#endif
		}

			memset( &broadcast_addr, 0, sizeof( broadcast_addr ) );
			broadcast_addr.sin_family = AF_INET;
			broadcast_addr.sin_addr.s_addr = htonl( INADDR_BROADCAST );
			broadcast_addr.sin_port = htons( 0 );

	}


	//
	// Create a receive UDP/IP datagram socket
	//

#ifdef WIN32
	SOCKET theSocket;
#else
	int theSocket;
#endif

	theSocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

#ifdef WIN32
	if ( theSocket == INVALID_SOCKET ) {
                UNLOCK_MUTEX( pctrlObject->m_portMutex );
		ExitThread( errorCode );
#else
	if ( theSocket == -1 ) {
                UNLOCK_MUTEX( pctrlObject->m_portMutex );
		pthread_exit( &errorCode );
#endif
	}

	//
	// Fill in the address structure
	//

#ifdef WIN32
	SOCKADDR_IN serverAddr;
#else
	struct sockaddr_in serverAddr;
#endif
	int nRet;

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl( INADDR_ANY );	// Let WinSock assign address

	if ( pctrlObject->m_bHubmode ) {

		serverAddr.sin_port = htons( XAP_DEFAULT_PORT );

		//
		// bind the name to the socket
		//
#ifdef WIN32
		if ( SOCKET_ERROR == bind( theSocket,
		                ( LPSOCKADDR )&serverAddr,
                                sizeof (struct sockaddr ) ) ) {
			       // Failed to bind to socket/port
                        UNLOCK_MUTEX( pctrlObject->m_portMutex );
                        ExitThread( errorCode );
                }

#else
		if ( -1 == bind( theSocket,
		                ( struct sockaddr * )&serverAddr,
                                sizeof (struct sockaddr ) ) ) {
			       // Failed to bind to socket/port
                        UNLOCK_MUTEX( pctrlObject->m_portMutex );
                        pthread_exit( &errorCode );
                }
#endif

	}
	else {

		short i = XAP_DEFAULT_PORT + 1;

		do {
			serverAddr.sin_port = htons( i );

#ifdef WIN32
			if ( SOCKET_ERROR != bind( theSocket,
					( LPSOCKADDR )&serverAddr,
					sizeof (struct sockaddr ) ) ) {
				break; // OK we have a socket on this port
			}
#else
			if ( -1 != bind( theSocket,
                                        ( struct sockaddr * )&serverAddr,
                                        sizeof (struct sockaddr ) ) ) {
                                break; // OK we have a socket on this port
                        }
#endif
			i++;
		} while ( i != 0 );

		if ( !i ) {
			// Failed to get a free socket/port
			UNLOCK_MUTEX( pctrlObject->m_portMutex );
#ifdef WIN32
			ExitThread( errorCode );
#else
			pthread_exit( &errorCode );
#endif
		}
	}

	pctrlObject->m_Port = ntohs( serverAddr.sin_port );

	// We got the socket
	UNLOCK_MUTEX( pctrlObject->m_portMutex );

#ifdef WIN32
	SOCKADDR_IN saClient;	// Client data was received from
#else
	struct sockaddr_in saClient;   // Client data was received from
#endif

	/////////////////////////////
	//      Receive loop
	/////////////////////////////

#ifdef WIN32
	FD_SET fs;
#else
	fd_set fs;
#endif
	struct timeval tv;

	while ( pctrlObject->m_bRun ) {

		FD_ZERO( &fs );
		FD_SET( theSocket, &fs );

		tv.tv_sec = 1;	// Every second
		tv.tv_usec = 0;

		if ( select( theSocket + 1, &fs, NULL, NULL, &tv ) > 0 ) {

			memset( buf, 0, sizeof( buf ) );

			// Wait for data from the client
#ifdef WIN32
			nLen = sizeof( SOCKADDR );
#else
			nLen = sizeof( struct sockaddr );
#endif
			memset( buf, 0, sizeof( buf ) );
			nRet = recvfrom( theSocket,
						(char *)buf,
						sizeof(buf ),
						0,
#ifdef WIN32
						( LPSOCKADDR )&saClient,
						&nLen );
#else
						( struct sockaddr * )&saClient,
						(socklen_t *)&nLen );

#endif

#ifdef WIN32
			if ( !nRet || ( nRet == SOCKET_ERROR ) ) {
				continue;
			}
#else
			if ( !nRet || ( nRet == -1 ) ) {
                                continue;
                        }
#endif

			// Handle received package

			if ( pctrlObject->m_bHubmode ) {

				// If this is a packet sent from ourself
				// we just disregards it

				memcpy( wbuf, buf, sizeof( buf ) );
				// Convert to lower case
#ifdef WIN32
				_strlwr( wbuf );
#else
				pp = wbuf;
				while ( *pp ) {
					*pp = *pp + 0x80;
					pp++;
				}
#endif

				if ( NULL != ( pStart = strstr( wbuf, "xap-header" ) ) ||
					 NULL != ( pStart = strstr( wbuf, "xap-hbeat" ) )  ) {

					p = pStart;
					if ( NULL != ( pEnd = strchr( wbuf, '}' ) ) ) {

						*pEnd = 0;
						p = pEnd + 1; // Point to rest of message

						// The message should not be originating from ourselves
						if ( NULL != ( pStart = strstr( pStart, "source" ) ) ) {

							if ( NULL != strstr( pStart, pctrlObject->m_strxapclass ) ) {
								continue;
							}
						}
					}
				}


				//////////////////////////////////
				//         Received hbeat
				//////////////////////////////////

				int interval;
				bool bClientFound = false;

				// Make a copy of the input data
				memcpy( wbuf, buf, sizeof( buf ) );
                                // Convert to lower case
#ifdef WIN32
                                _strlwr( wbuf );
#else
                                pp = wbuf;
                                while ( *pp ) {
                                        *pp = *pp + 0x80;
                                        pp++;
                                }
#endif

				if ( NULL != ( pStart = strstr( wbuf, "xap-hbeat" ) ) ) {

					if ( NULL != ( pEnd = strchr( wbuf, '}' ) ) ) {

						*pEnd = 0;
						interval = 30;	// Use 30s as default
						if ( NULL != ( pStart = strstr( pStart, "interval" ) ) ) {

							if ( NULL != ( pStart = strchr( pStart, '=' ) + 1 ) ) {

								// Get interval
								interval = atoi( pStart );
							}
						}

						if ( NULL != ( pStart = strstr( pStart, "port" ) ) ) {

							if ( NULL != ( pStart = strchr( pStart, '=' ) + 1 ) ) {

								// Get port
								saClient.sin_port = htons( atoi( pStart ) );
							}

							saClient.sin_addr.s_addr = inet_addr("127.0.0.1");
						}

						// Update data for client if it alread have been found
						bClientFound = false;
						for ( int i=0; i < XAP_MAX_CLIENTS; i++ ) {

							if ( pctrlObject->m_xAPClients[ i ].isSameClient( &saClient ) ) {
								pctrlObject->m_xAPClients[ i ].UpdateHeartBeat();
								bClientFound = true;
								break;
							}
						}

						// Check if client should be added
						if ( !bClientFound ) {

							// Yes, the client should be added
							for ( int i=0; i < XAP_MAX_CLIENTS; i++ ) {

								if ( pctrlObject->m_xAPClients[ i ].isFree() ) {
									// Yes a free entry found, add client
									pctrlObject->m_xAPClients[ i ].setClientData( &saClient, interval );
									break;
								}

							}

						} // clientfound
					} // } found
				} // xap-hbeat


				//////////////////////////////
				//      Send to clients
				//////////////////////////////


				// Create transmission structure
				xapsend *pxapsend = new xapsend;
				if ( NULL != pxapsend ) {
					pxapsend->m_bHubmode = pctrlObject->m_bHubmode;
					pxapsend->m_pxapobj = pctrlObject;
					pxapsend->size = nRet;
					memcpy( pxapsend->m_msgBuf, buf, sizeof( pxapsend->m_msgBuf ) );
				}
				else {
					continue;
				}

				// OK we have a xAP message
				// Send it to all clients

#ifdef WIN32

				// Start client write thread
				DWORD threadId;
				if ( NULL ==
					( hTreadxAPClientSend =
						CreateThread( NULL,
										0,
										(LPTHREAD_START_ROUTINE) workThreadxAPClientSend,
										pxapsend,
										0,
										&threadId ) ) ) {
					// Failure

				}


#else // LINUX

				pthread_attr_t thread_attr;
				pthread_attr_init( &thread_attr );


				// Create the write thread.
				if ( pthread_create( &hTreadxAPClientSend,
								&thread_attr,
								workThreadxAPClientSend,
								pxapsend ) ) {

					syslog( LOG_ERR, "xapObj: Unable to create xapdrv write thread.");
				}

#endif

				// Broadcast message
				/*
				nRet = sendto( bcastsock,
								(const char *)buf,
								nRet,
								0,
								( LPSOCKADDR )&broadcast_addr,
								sizeof( broadcast_addr ) );
				*/

				// Check if any of the clients have timed out
				//	mark them as dead if so
				for ( int i=0; i < XAP_MAX_CLIENTS; i++ ) {
					pctrlObject->m_xAPClients[ i ].checkTimeout();
				}

			} // HUB mode


			/////////////////////////
			//     VSCP Message
			/////////////////////////

			// Make a copy of the input data
			memcpy( wbuf, buf, sizeof( buf ) );
                        // Convert to lower case
#ifdef WIN32
                        _strlwr( wbuf );
#else
                        pp = wbuf;
                        while ( *pp ) {
                        	*pp = *pp + 0x80;
                        	pp++;
                        }
#endif


			// * * * Parse the xAP message for VSCP blocks

			if ( NULL != ( pStart = strstr( wbuf, "xap-header" ) ) ) {

				p = pStart;
				if ( NULL != ( pEnd = strchr( wbuf, '}' ) ) ) {

					*pEnd = 0;
					p = pEnd + 1; // Point to rest of message

					// The message should not be originating from ourselves
					if ( NULL != ( pStart = strstr( pStart, "source" ) ) ) {


						if ( ( NULL != ( pStart = strchr( pStart, '=' ) + 1 ) ) &&
								NULL == strstr( pStart, pctrlObject->m_strxapclass ) ) {

							// This message may be of interest
							pStart = p;
							while ( NULL != ( pStart = strstr( p, "level1.event" ) ) ) {

								if ( NULL != ( pEnd = strchr( pStart, '}' ) ) ) {

									*pEnd = 0;
									p = pEnd + 1;

									// OK, We expect event data here
									if (  pctrlObject->m_receiveList.nCount < XAP_MAX_RCVMSG ) {

										PCANALMSG pMsg	= new canalMsg;

										if ( NULL != pMsg ) {

											dllnode *pNode = new dllnode;
											if ( NULL != pNode ) {

												char *pp;
												short vscp_class = 0;
												unsigned char vscp_type = 0;
												unsigned char vscp_priority = 7;
												pMsg->flags = CANAL_IDFLAG_EXTENDED;
												pMsg->id = 0;
												pMsg->obid = 0;
												pMsg->timestamp = 0;
												pMsg->sizeData = 0;

												if ( NULL != ( pp = strstr( pStart, "vscp-class" ) ) ) {
													if ( NULL != ( pp = strchr( pp, '=' ) + 1 ) ) {
														vscp_class = atoi( pp );
													}
												}

												if ( NULL != ( pp = strstr( pStart, "vscp-type" ) ) ) {
													if ( NULL != ( pp = strchr( pp, '=' ) + 1 ) ) {
														vscp_type = atoi( pp );
													}
												}

												if ( NULL != ( pp = strstr( pStart, "priority" ) ) ) {
													if ( NULL != ( pp = strchr( pp, '=' ) + 1 ) ) {
														vscp_priority = atoi( pp );
													}
												}

												pMsg->id = ( vscp_priority << 26 ) + ( vscp_class << 16 ) + ( vscp_type << 8 );

												if ( NULL != ( pp = strstr( pStart, "obid" ) ) ) {
													if ( NULL != ( pp = strchr( pp, '=' ) + 1 ) ) {
														pMsg->obid = atol( pp );
													}
												}

												if ( NULL != ( pp = strstr( pStart, "data" ) ) ) {

													int i = 0;

													if ( NULL != ( pp = strchr( pp, '=' ) + 1 ) ) {
														char *ptok = strtok( pp, ",\n" );
														while ( NULL != ptok ) {
															pMsg->data[ i++ ] = ( unsigned char)pctrlObject->getDataValue( ptok);
															ptok = strtok( NULL, ",\n" );
														}

														// Set datasize
														pMsg->sizeData = i;
													}

												}

												pNode->pObject = pMsg;
												LOCK_MUTEX( pctrlObject->m_receiveMutex );
												dll_addNode( &pctrlObject->m_receiveList, pNode );
												UNLOCK_MUTEX( pctrlObject->m_receiveMutex );

												// Update statistics
												pctrlObject->m_stat.cntReceiveData += pMsg->sizeData;
												pctrlObject->m_stat.cntReceiveFrames += 1;

											}
											else {

												delete pMsg;

											}
										}
									}
									else {
										// Full buffer
										pctrlObject->m_stat.cntOverruns++;
									}
								}

							}
						} // From ourself
					} // source
				} // end mark
			} // xap-header



			if ( pctrlObject->m_bHubmode ) {

				// Wait for Send thread to terminate.
#ifdef WIN32
				DWORD exitcode;
				time_t start,now;

				time( &start );	// Get start time
				time( &now );

				while ( ( (now - start) < 5 ) ) {
					GetExitCodeThread( hTreadxAPClientSend, &exitcode );
					if ( STILL_ACTIVE == exitcode ) break;
						SLEEP( 100 );
						time( &now );
					}
#else

#endif
			} // 2nd hubmode


		} // select


	} // while

#ifdef WIN32
	closesocket( theSocket );
	ExitThread( errorCode );
#else
	::close( theSocket  );
	pthread_exit( &errorCode );
#endif

}
