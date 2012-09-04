// UDPDrvObj.cpp: implementation of the UDPDrvObj class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "../../../common/canal.h"
#include "UDPDrvObj.h"
#include "udpdrv.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// Pototypes
void transmitThread( void *pThreadObject );
void receiveThread( void *pObject );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UDPDrvObj::UDPDrvObj()
{
	m_bRun = true;				// Run and run and run....
	m_bLateTimeStamp = true;	// Late timestamp is default

	dll_init( &m_rcvList, SORT_NONE );
	dll_init( &m_sndList, SORT_NONE );
}


UDPDrvObj::~UDPDrvObj()
{	
	dll_removeAllNodes( &m_rcvList );
	dll_removeAllNodes( &m_sndList );
}


///////////////////////////////////////////////////////////////////////////////
// init
//

bool UDPDrvObj::init( int port )
{
	bool rv = true;
	DWORD dwThreadId;	

	m_udpReceiveMutex = CreateMutex( NULL, true, UDP_SOCKET_DLL_RECEIVE_OBJ_MUTEX );

	// Create the UDP Transmit thread.
	if ( NULL == CreateThread(	NULL,
									0,
									(LPTHREAD_START_ROUTINE) transmitThread,
									this,
									0,
									&dwThreadId ) ) { 
		; // UDP receive functionality disabled - we can still send...
	}

	// Create the UDP Receive thread.
	if ( NULL == CreateThread(	NULL,
									0,
									(LPTHREAD_START_ROUTINE) receiveThread,
									this,
									0,
									&dwThreadId ) ) { 
		; // UDP receive functionality disabled - we can still send...
	}

	ReleaseMutex( m_udpReceiveMutex );

	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// release
//

bool UDPDrvObj::release( void )
{
	bool rv = true;

	// Terminate the thread
	m_bRun = false;

	return rv;
}


///////////////////////////////////////////////////////////////////////////////
// send
//

bool UDPDrvObj::send( PCANALMSG pMsg )
{
	bool rv = false;

	if ( NULL != pMsg ) {
	
		// Must be room for the message
		if ( m_sndList.nCount < UDP_SOCKET_MAX_SNDMSG ) {
			dllnode *pNode = new dllnode;
			if ( NULL != pNode ) {
			
				pNode->pObject = pMsg;

				dll_addNode( &m_sndList, pNode );
				
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
// receive
//

bool UDPDrvObj::receive( PCANALMSG pMsg )
{
	bool rv = false;
	
	if ( ( NULL != m_rcvList.pHead ) && 
			( NULL != m_rcvList.pHead->pObject ) ) {
		
		WaitForSingleObject( m_udpReceiveMutex, INFINITE );
		
		memcpy( pMsg, m_rcvList.pHead->pObject, sizeof( canalMsg ) );
		dll_removeNode( &m_rcvList, m_rcvList.pHead );
		
		ReleaseMutex( m_udpReceiveMutex );
		rv = true;
	}

	return rv;
}


///////////////////////////////////////////////////////////////////////////////
//	dataAvailable
//

int UDPDrvObj::dataAvailable( void )
{
	int cnt;
	
	WaitForSingleObject( m_udpReceiveMutex, INFINITE );
	cnt = dll_getNodeCount( &m_rcvList );	
	ReleaseMutex( m_udpReceiveMutex );

	return cnt;
}

///////////////////////////////////////////////////////////////////////////////
//	getStatistics
//

bool UDPDrvObj::getStatistics( PCANALSTATISTICS pCanalStatistics )
{
	pCanalStatistics = &m_stat;
	return true;
}

///////////////////////////////////////////////////////////////////////////////
//	getStatus
//

bool UDPDrvObj::getStatus( PCANALSTATUS pCanalStatus )
{

	//char rBuf[ 32 ];


	
	return true;
}


///////////////////////////////////////////////////////////////////////////////
// transmitThread
//
// The workThread do most of the acual work such as send and receive.
//
//

void transmitThread( void *pObject )
{
	bool bRun = true;
	bool bActivity = true;
	int optval; 
    int optlen;
	char cmd[ CANAL_SEQPOS_DATA + 8 ];
	unsigned char *p;
	short nPollCnt = 0;
	DWORD errorCode = 0;
	HANDLE UDPObjMutex;	 		
	
	// Must hav a valid work object
	if ( NULL == pObject ) ExitThread( errorCode );

	// Get the object pointer in place
	UDPDrvObj *pobj = (UDPDrvObj *)pObject;

	//
	// Create a UDP/IP datagram socket
	//
	SOCKET theSocket;

	// Create the UDP socket.
	theSocket = socket( AF_INET,			 
							SOCK_DGRAM,		 
							IPPROTO_UDP );	 
	
	if ( theSocket == INVALID_SOCKET ) {
		ExitThread( -1 );
	}

	optval=1; 
	optlen = sizeof( int ); 
	if( setsockopt( theSocket, SOL_SOCKET,SO_BROADCAST, (char *) &optval,optlen ) ) { 
		ExitThread( -2 );
	}
	
	UDPObjMutex = OpenMutex( MUTEX_ALL_ACCESS, false, UDP_SOCKET_DLL_RECEIVE_OBJ_MUTEX );

	while ( pobj->m_bRun ) {
		
		///////////////////////////////////////////////////////////////////////
		//								Receive 
		///////////////////////////////////////////////////////////////////////
		//WaitForSingleObject( UDPObjMutex, INFINITE );
	

		//ReleaseMutex( UDPObjMutex );


		///////////////////////////////////////////////////////////////////////
		//								Transmit
		///////////////////////////////////////////////////////////////////////

		WaitForSingleObject( UDPObjMutex, INFINITE );

		// Is there anything to transmit
		if ( ( NULL != pobj->m_sndList.pHead ) && 
							( NULL != pobj->m_sndList.pHead->pObject ) ) {
			canalMsg msg;
			bActivity = true;

			memcpy( &msg, pobj->m_sndList.pHead->pObject, sizeof( canalMsg ) ); 
			dll_removeNode( &pobj->m_sndList, pobj->m_sndList.pHead );

			// Set realtive time for packet (ms)
			msg.timestamp = GetTickCount();
		
#ifdef BIGENDIAN 

			// Flags
			p = (unsigned char *)&msg.flags;
			cmd[ CANAL_SEQPOS_FLAGS + 0 ] = *(p+0); 
			cmd[ CANAL_SEQPOS_FLAGS + 1 ] = *(p+1);
			cmd[ CANAL_SEQPOS_FLAGS + 2 ] = *(p+2);
			cmd[ CANAL_SEQPOS_FLAGS + 3 ] = *(p+3);
			
			// id
			p = (unsigned char *)&msg.id;
			cmd[ CANAL_SEQPOS_ID + 0 ] = *(p+0); 
			cmd[ CANAL_SEQPOS_ID + 1  ] = *(p+1);
			cmd[ CANAL_SEQPOS_ID + 2  ] = *(p+2);
			cmd[ CANAL_SEQPOS_ID + 3  ] = *(p+3);

			// obid
			p = (unsigned char *)&msg.obid;
			cmd[ CANAL_SEQPOS_OBID + 0 ] = *(p+0); 
			cmd[ CANAL_SEQPOS_OBID + 1 ] = *(p+1);
			cmd[ CANAL_SEQPOS_OBID + 2 ] = *(p+2);
			cmd[ CANAL_SEQPOS_OBID + 3 ] = *(p+3);

			// Timestamp
			p = (unsigned char *)&msg.timestamp;
			cmd[ CANAL_SEQPOS_TIMESTAMP + 0 ] = *(p+0); 
			cmd[ CANAL_SEQPOS_TIMESTAMP + 1 ] = *(p+1);
			cmd[ CANAL_SEQPOS_TIMESTAMP + 2 ] = *(p+2);
			cmd[ CANAL_SEQPOS_TIMESTAMP + 3 ] = *(p+3);

#else 

			// Flags
			p = (unsigned char *)&msg.flags;
			cmd[ CANAL_SEQPOS_FLAGS + 0 ] = *(p+3);	
			cmd[ CANAL_SEQPOS_FLAGS + 1 ] = *(p+2);
			cmd[ CANAL_SEQPOS_FLAGS + 2 ] = *(p+1);
			cmd[ CANAL_SEQPOS_FLAGS + 3 ] = *(p+0);

			// id
			p = (unsigned char *)&msg.id;
			cmd[ CANAL_SEQPOS_ID + 0 ] = *(p+3);	
			cmd[ CANAL_SEQPOS_ID + 1 ] = *(p+2);
			cmd[ CANAL_SEQPOS_ID + 2 ] = *(p+1);
			cmd[ CANAL_SEQPOS_ID + 3 ] = *(p+0);

			// obid
			p = (unsigned char *)&msg.obid;
			cmd[ CANAL_SEQPOS_OBID + 0 ] = *(p+3); 
			cmd[ CANAL_SEQPOS_OBID + 1 ] = *(p+2);
			cmd[ CANAL_SEQPOS_OBID + 2 ] = *(p+1);
			cmd[ CANAL_SEQPOS_OBID + 3 ] = *(p+0);

			// Timestamp
			p = (unsigned char *)&msg.timestamp;
			cmd[ CANAL_SEQPOS_TIMESTAMP + 0 ] = *(p+3); 
			cmd[ CANAL_SEQPOS_TIMESTAMP + 1 ] = *(p+2);
			cmd[ CANAL_SEQPOS_TIMESTAMP + 2 ] = *(p+1);
			cmd[ CANAL_SEQPOS_TIMESTAMP + 3 ] = *(p+0);

#endif

			cmd[ CANAL_SEQPOS_DATASIZE ] = msg.sizeData;

			for ( int i=0; i<8; i++ ) {
				cmd[ CANAL_SEQPOS_DATA + i ] = msg.data[ i ];
			}			

			//theSocket.send();
		}	 

		ReleaseMutex( UDPObjMutex );

		if ( !bActivity ) Sleep ( 100 );
		bActivity = false;

	}

	// Release the mutex for other threads to use
	ReleaseMutex( UDPObjMutex );

	// Close the socket
	if ( theSocket ) {
		closesocket( theSocket );
	}

	// Terminate
	ExitThread( errorCode );
}


///////////////////////////////////////////////////////////////////////////////
// receiveThread
//
//

void receiveThread( void *pObject )
{
	DWORD errorCode = 0;
	
	DWORD nRead = 0;
	DWORD nWrite = 0;
	HANDLE udpReceiveMutex;
	int nLen;
	char buf[ 256 ];
	unsigned char *p;

	// Must hav a valid work object
	if ( NULL == pObject ) ExitThread( -1 );

	// Get the object pointer in place
	UDPDrvObj *pobj = (UDPDrvObj *)pObject;

	udpReceiveMutex = OpenMutex( MUTEX_ALL_ACCESS, false, UDP_SOCKET_DLL_RECEIVE_OBJ_MUTEX );

	char szName[128];
	gethostname( szName, sizeof( szName ) );
	LPHOSTENT lpLocalHostEntry;
	lpLocalHostEntry = gethostbyname( szName );
	unsigned long * localaddr = ( unsigned long *) lpLocalHostEntry->h_addr_list[ 0 ];	

	//
	// Create a UDP/IP datagram socket
	//
	SOCKET theSocket;

	theSocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if ( theSocket == INVALID_SOCKET ) {
		ExitThread( -2 );
	}
	

	//
	// Fill in the address structure
	//

	SOCKADDR_IN saServer;

	saServer.sin_family = AF_INET;
	saServer.sin_addr.s_addr = INADDR_ANY;	// Let WinSock assign address
	saServer.sin_port = htons( CANAL_UDP_PORT_ID );	   


	//
	// bind the name to the socket
	//

	int nRet;

	nRet = bind( theSocket,						 
					( LPSOCKADDR )&saServer, 
					sizeof (struct sockaddr ) );
	if ( nRet == SOCKET_ERROR ) {
		ExitThread( -3 );
	}


	SOCKADDR_IN saClient;
	nLen = sizeof( SOCKADDR );


	while ( pobj->m_bRun ) {

		memset( buf, 0, sizeof( buf ) );
		
		// Wait for data from the client
		nRet = recvfrom( theSocket,				
							buf,				
							sizeof(buf ),		
							0,						
							( LPSOCKADDR )&saClient,
							&nLen );	
		if ( !nRet || ( nRet == SOCKET_ERROR ) || ( nRet < sizeof( canalMsg ) ) ) { 
			continue;
		}

		// Handle received package

		// If this is a packet sent from this machine
		// we just disregards it
		if ( *localaddr == (unsigned long)saClient.sin_addr.S_un.S_addr ) continue;
		
	
		canalMsg *pMsg = ( canalMsg * ) new canalMsg;

		if ( NULL != pMsg ) {

			pMsg->sizeData = buf[ CANAL_SEQPOS_DATASIZE ];

			// Must be a valid size
			if ( pMsg->sizeData > 8 ) {
				delete pMsg;
				continue;
			}
		
 		

#ifdef BIGENDIAN 

			// Flags
			p = (unsigned char *)&pMsg->flags;
			*(p+0) = buf[ CANAL_SEQPOS_FLAGS + 0 ]; 
			*(p+1) = buf[ CANAL_SEQPOS_FLAGS + 1 ];
			*(p+2) = buf[ CANAL_SEQPOS_FLAGS + 2 ];
			*(p+3) = buf[ CANAL_SEQPOS_FLAGS + 3 ];
			
			// id
			p = (unsigned char *)&pMsg->id;
			*(p+0) = buf[ CANAL_SEQPOS_ID  + 0 ]; 
			*(p+1) = buf[ CANAL_SEQPOS_ID  + 1 ];
			*(p+2) = buf[ CANAL_SEQPOS_ID  + 2 ];
			*(p+3) = buf[ CANAL_SEQPOS_ID  + 3 ];

			// obid
			p = (unsigned char *)&pMsg->obid;
			*(p+3) = buf[ CANAL_SEQPOS_OBID  + 0 ];	
			*(p+2) = buf[ CANAL_SEQPOS_OBID  + 1 ];
			*(p+1) = buf[ CANAL_SEQPOS_OBID  + 2 ];
			*(p+0) = buf[ CANAL_SEQPOS_OBID  + 3 ];

			// timestamp
			p = (unsigned char *)&pMsg->timestamp;
			*(p+3) = buf[ CANAL_SEQPOS_TIMESTAMP  + 0 ];	
			*(p+2) = buf[ CANAL_SEQPOS_TIMESTAMP  + 1 ];
			*(p+1) = buf[ CANAL_SEQPOS_TIMESTAMP  + 2 ];
			*(p+0) = buf[ CANAL_SEQPOS_TIMESTAMP  + 3 ];

#else 

			// Flags
			p = (unsigned char *)&pMsg->flags;
			*(p+3) = buf[ CANAL_SEQPOS_FLAGS + 0 ];	
			*(p+2) = buf[ CANAL_SEQPOS_FLAGS + 1 ];
			*(p+1) = buf[ CANAL_SEQPOS_FLAGS + 2 ];
			*(p+0) = buf[ CANAL_SEQPOS_FLAGS + 3 ];

			// id
			p = (unsigned char *)&pMsg->id;
			*(p+3) = buf[ CANAL_SEQPOS_ID  + 0 ];	
			*(p+2) = buf[ CANAL_SEQPOS_ID  + 1 ];
			*(p+1) = buf[ CANAL_SEQPOS_ID  + 2 ];
			*(p+0) = buf[ CANAL_SEQPOS_ID  + 3 ];

			// obid
			p = (unsigned char *)&pMsg->obid;
			*(p+3) = buf[ CANAL_SEQPOS_OBID  + 0 ];	
			*(p+2) = buf[ CANAL_SEQPOS_OBID  + 1 ];
			*(p+1) = buf[ CANAL_SEQPOS_OBID  + 2 ];
			*(p+0) = buf[ CANAL_SEQPOS_OBID  + 3 ];

			// timestamp
			p = (unsigned char *)&pMsg->timestamp;
			*(p+3) = buf[ CANAL_SEQPOS_TIMESTAMP  + 0 ];	
			*(p+2) = buf[ CANAL_SEQPOS_TIMESTAMP  + 1 ];
			*(p+1) = buf[ CANAL_SEQPOS_TIMESTAMP  + 2 ];
			*(p+0) = buf[ CANAL_SEQPOS_TIMESTAMP  + 3 ];

#endif

			if ( 0 != pMsg->sizeData ) {			
				
				for ( int i = 0; i < pMsg->sizeData; i++ ) {
					pMsg->data[ i ] = buf[ CANAL_SEQPOS_DATA + i ];		
				}

			}

			// If late timestamp 
			if ( pobj->m_bLateTimeStamp ) {
				pMsg->timestamp = GetTickCount();
			}

		
			// Get mutex
			WaitForSingleObject( udpReceiveMutex, INFINITE );

			// Must be room for the package
			if ( pobj->m_rcvList.nCount < UDP_SOCKET_MAX_RCVMSG ) {

				dllnode *pnewudpmsg = new dllnode;
				if ( NULL != pnewudpmsg ) {

					// Add to in queue
					pnewudpmsg->Key = 0;
					pnewudpmsg->pKey = &pnewudpmsg->Key ;
					pnewudpmsg->pObject = pMsg;
							
					if ( !dll_addNode( &pobj->m_rcvList, pnewudpmsg ) ) {
						delete pnewudpmsg;

					}
				}
			}

			// Release mutex
			ReleaseMutex( udpReceiveMutex );

		}
	}

 
	//
	// Normally a server continues to run so that
	// it is available to other clients. In this
	// example, we exit as soon as one transaction
	// has taken place.
	//
	closesocket(theSocket);

	ExitThread( errorCode );

}

