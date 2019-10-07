// syslogobj.cpp: implementation of the CSysLogObj class.
//
// Copyright (C) 2000-2015
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
//////////////////////////////////////////////////////////////////////

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

#include "wx/log.h"
#include "wx/file.h"
#include "wx/sizer.h"
#include "wx/menuitem.h"
#include "wx/checklst.h"
#include "wx/url.h"
#include "wx/html/htmlpars.h"
#include "wx/stream.h"
#include "wx/datetime.h"
#include "wx/msw/ole/automtn.h"
#include "wx/tokenzr.h"
#include "wx/wfstream.h"
#include "syslogobj.h"



// Prototypes threads
#ifdef WIN32
void workThreadTransmit( void *pObject );
void workThreadReceive( void *pObject );
#else
void *workThreadTransmit( void *pObject );
void *workThreadReceive( void *pObject );
#endif



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSysLogObj::CSysLogObj()
{

	// Set default SysLog port
	m_syslogPort = SYSLOG_PORT;

	m_drvidx = 0;
	m_callIdx = 0;	// No calls received yet

	// No filter/mask
	m_filter = 0;
	m_mask = 0;

	m_bRun = false;

#ifdef WIN32

	m_hTreadReceive = 0;

	// Create the device AND LIST access mutexes
	m_syslogMutex = CreateMutex( NULL, true, CANAL_DLL_SYSLOG_OBJ_MUTEX );
	m_receiveMutex = CreateMutex( NULL, true, CANAL_DLL_SYSLOG_RECEIVE_MUTEX );

#else

	pthread_mutex_init( &m_syslogMutex, NULL );
	pthread_mutex_init( &m_receiveMutex, NULL );

#endif

	dll_init( &m_receiveList, SORT_NONE );

}

CSysLogObj::~CSysLogObj()
{
	close();

	LOCK_MUTEX( m_receiveMutex );
	dll_removeAllNodes( &m_receiveList );


#ifdef WIN32

	if ( NULL != m_syslogMutex ) CloseHandle( m_syslogMutex );
	if ( NULL != m_receiveMutex ) CloseHandle( m_receiveMutex );


#else

	pthread_mutex_destroy( &m_syslogMutex );
	pthread_mutex_destroy( &m_receiveMutex );

#endif
}



///////////////////////////////////////////////////////////////////////////////
//  open
//
// filename
//-----------------------------------------------------------------------------
// Parameters for the driver as a string on the following form
//
// "port"
//
//
// lircport
// ========
// The port to listen to. If not given the defaultport
// "514" i used.
//
//
//
// flags
//-----------------------------------------------------------------------------
// Bit 0: isdn_net caller id phone number information.
// Bit 1: Send raw syslog messages.
//

bool CSysLogObj::open( const char *szFileName, unsigned long flags )
{
	char szDrvParams[ MAX_PATH ];
	m_initFlag = flags;

	// save parameter string and conbert to upper case
	strncpy( szDrvParams, szFileName, MAX_PATH );
	_strupr( szDrvParams );

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

	// Path to config file
	char *p;

	p = strtok( (char * )szDrvParams, ";" );
	if ( NULL != p ) {
		m_syslogPort = getDataValue( p );
	}

	// We are open
	m_bRun = true;

	// Release the mutex
	UNLOCK_MUTEX( m_syslogMutex );
	UNLOCK_MUTEX( m_receiveMutex );


#ifdef WIN32


	DWORD threadId;

	// Start read thread
	if ( NULL ==
			( m_hTreadReceive = CreateThread(	NULL,
										0,
										(LPTHREAD_START_ROUTINE) workThreadReceive,
										this,
										0,
										&threadId ) ) ) {
		// Failure
		close();
		return  false;
	}



#else // LINUX


	pthread_attr_t thread_attr;
	pthread_attr_init( &thread_attr );


	// Create the log write thread.
	if ( pthread_create( 	&m_threadId,
								&thread_attr,
								workThreadReceive,
								this ) ) {

		syslog( LOG_ERR, "canallogger: Unable to create vector receive thread.");
		rv = false;
		fclose( m_flog );
	}

    // We are open
	m_bOpen = true;


	// Release the mutex
	pthread_mutex_unlock( &m_vectorMutex );

#endif

	return true;
}

//////////////////////////////////////////////////////////////////////
// close
//

bool CSysLogObj::close( void )
{
	// Do nothing if already terminated
	if ( !m_bRun ) return false;

	m_bRun = false;



	// terminate the worker thread
#ifdef WIN32

	DWORD rv;


	// Wait for receive thread to terminate
	while ( true ) {
		GetExitCodeThread( m_hTreadReceive, &rv );
		if ( STILL_ACTIVE != rv ) break;
	}


#else

	int *trv;
	pthread_join( m_threadIdReceive, (void **)&trv );

#endif


	return true;
}


//////////////////////////////////////////////////////////////////////
// doFilter
//

bool CSysLogObj::doFilter( canalMsg *pcanalMsg )
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

bool CSysLogObj::setFilter( unsigned long filter )
{
	m_filter = filter;
	return true;
}


//////////////////////////////////////////////////////////////////////
// setMask
//

bool CSysLogObj::setMask( unsigned long mask )
{
	m_mask = mask;
	return true;
}


//////////////////////////////////////////////////////////////////////
// writeMsg
//

bool CSysLogObj::writeMsg( canalMsg *pMsg )
{
	return false;	// Not implemented yet ( if ever )
}


//////////////////////////////////////////////////////////////////////
// readMsg
//

bool CSysLogObj::readMsg( canalMsg *pMsg )
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

int CSysLogObj::dataAvailable( void )
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

bool CSysLogObj::getStatistics( PCANALSTATISTICS pCanalStatistics )
{
	// Must be a valid pointer
	if ( NULL == pCanalStatistics ) return false;

	memcpy( pCanalStatistics, &m_stat, sizeof( canalStatistics ) );

	return true;
}


///////////////////////////////////////////////////////////////////////////////
//	getStatus
//

bool CSysLogObj::getStatus( PCANALSTATUS pCanalStatus )
{
	// Must be a valid pointer
	if ( NULL == pCanalStatus ) return false;


	memcpy( pCanalStatus, &m_status, sizeof( canalStatus ) );

	return true;
}



///////////////////////////////////////////////////////////////////////////////
// getDataValue
//

unsigned long CSysLogObj::getDataValue( const char *szData )
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
///////////////////////////////////////////////////////////////////////////////
//								Worker Thread
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
// workThreadReceive
//
//

#ifdef WIN32
void workThreadReceive( void *pObject )
#else
void *workThreadReceive( void *pObject )
#endif
{
	int nLen;
	char buf[ 2048 ];
	char szName[ 128 ];

	dllnode *pNode;			// Linked list node
	canalMsg *pmsg;			// Message to send
	short totLength;		// Total message length (workload)
	short nPack;			// Number of frames to send
	unsigned int posSend;	// Current send position

#ifdef WIN32
	DWORD errorCode = 0;
#else
	int rv = 0;
#endif

	CSysLogObj * pobj = ( CSysLogObj *)pObject;
	if ( NULL == pobj ) {
#ifdef WIN32
		ExitThread( errorCode ); // Fail
#else
		pthread_exit( &rv );
#endif
	}

	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD( 2, 2 );

	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
#ifdef WIN32
		ExitThread( errorCode ); // Fail
#else
		pthread_exit( &rv );
#endif
	}

	if ( SOCKET_ERROR == gethostname( szName, sizeof( szName ) ) ) {
		strcpy( szName, "localhost" );
	}

	LPHOSTENT lpLocalHostEntry;
	lpLocalHostEntry = gethostbyname( szName );
	if ( NULL == lpLocalHostEntry ) {
		ExitThread( errorCode );
	}


	//
	// Create a UDP/IP datagram socket
	//
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
	saServer.sin_addr.s_addr = INADDR_ANY;	// Let WinSock assign address
	saServer.sin_port = htons( pobj->m_syslogPort );

	//
	// bind the name to the socket
	//

	int nRet;

	nRet = bind( theSocket,
					( LPSOCKADDR )&saServer,
					sizeof (struct sockaddr ) );
	if ( nRet == SOCKET_ERROR ) {
		ExitThread( errorCode );
	}

	SOCKADDR_IN saClient;
	nLen = sizeof( SOCKADDR );


	struct timeval tv;
	fd_set readfds;

    tv.tv_sec = 0;
    tv.tv_usec = 300000;

	FD_ZERO( &readfds );
    FD_SET( theSocket, &readfds );


	while ( pobj->m_bRun ) {

		// Noting to do if we should end...
		if ( !pobj->m_bRun ) continue;


		FD_SET( theSocket, &readfds );

		select( 0, &readfds, NULL, NULL, &tv );

		if ( FD_ISSET( theSocket, &readfds ) ) {

			memset( buf, 0, sizeof( buf ) );

			// Wait for data from the client
			nRet = recvfrom( theSocket,
								buf,
								sizeof(buf ),
								0,
								( LPSOCKADDR )&saClient,
								&nLen );

			if ( !nRet || ( nRet == SOCKET_ERROR ) || ( nRet < 25 ) ) {
				continue;
			}

			char *p;
			wxString strCallFrom, strCallTo;
			if ( NULL != ( p = strstr( buf, "isdn_net: call from ") ) ) {

				// OK this is an icoming call
				p += 20; // Point past tag
				wxStringTokenizer tkz( p, "," );
				strCallFrom = tkz.GetNextToken();
				if ( '0' != strCallFrom[0] ) {
					strCallFrom = "0" + strCallFrom;
				}

				if ( NULL != ( p = strstr( buf, "-> ") ) ) {
					p += 3; // Point past tag
					wxStringTokenizer tkz( p, "\n" );
					strCallTo = tkz.GetNextToken();
					if ( '0' != strCallTo[0] ) {
						strCallTo = "0" + strCallTo;
					}
				}


				totLength = strCallFrom.Length() + strCallTo.Length();


				if ( pobj->m_initFlag & SYSLOGDRV_ISDN4L_PHONE ) {

					// Send 'Incoming call with destination' if destination number
					// information is available
					if ( 0 != strCallTo.Length() ) {

						wxString wxSendBuf = strCallFrom + "_" + strCallTo;
						totLength++;

						// How many frames
						nPack = totLength / 5;
						if ( totLength % 5 ) nPack++;

						posSend = 0;

						// 'Send' the frames
						for ( int i=0; i<nPack; i++ ) {

							pNode = new dllnode;
							if ( NULL != pNode ) {

								pmsg = new canalMsg;
								if ( NULL != pmsg ) {

									pmsg->id = 0 + (8 << 8) + (100 << 16); // Class=100, Type = 8
									pmsg->timestamp = (unsigned long)time( NULL );
									pmsg->flags = CANAL_IDFLAG_EXTENDED;
									pmsg->flags = CANAL_IDFLAG_EXTENDED;
									pmsg->sizeData = 3;
									pmsg->data[ 0 ] = pobj->m_callIdx;	// Id for this incoming call
									pmsg->data[ 1 ] = i;				// Fragment index
									pmsg->data[ 2 ] = nPack;			// Total number of fragments
									for ( int j=0;
											( ( posSend < wxSendBuf.Length() ) && ( j<5 ) );
											j++ ) {
										pmsg->data[ 3 + j ] =	wxSendBuf[ posSend++ ];
										pmsg->sizeData++;
									}


									pNode->pObject = pmsg;
									LOCK_MUTEX( pobj->m_receiveMutex );
									dll_addNode( &pobj->m_receiveList, pNode );
									UNLOCK_MUTEX( pobj->m_receiveMutex );

								}
							}
						}
					} // flag switch
				}


				// Send 'Incoming call'
				wxString wxSendBuf = strCallFrom;
				totLength = strCallFrom.Length();

				// How many frames
				nPack = totLength / 5;
				if ( totLength % 5 ) nPack++;

				posSend = 0;

				// 'Send' the frames
				for ( int i=0; i<nPack; i++ ) {

					pNode = new dllnode;
					if ( NULL != pNode ) {

						pmsg = new canalMsg;
						if ( NULL != pmsg ) {

							pmsg->id = 0 + (1 << 8) + (100 << 16); // Class=100, Type = 1
							pmsg->timestamp = (unsigned long)time( NULL );
							pmsg->flags = CANAL_IDFLAG_EXTENDED;
							pmsg->flags = CANAL_IDFLAG_EXTENDED;
							pmsg->sizeData = 3;
							pmsg->data[ 0 ] = pobj->m_callIdx;	// Id for this incoming call
							pmsg->data[ 1 ] = i;				// Fragment index
							pmsg->data[ 2 ] = nPack;			// Total number of fragments
							for ( int j=0;
									( ( posSend < wxSendBuf.Length() ) && ( j<5 ) );
									j++ ) {
								pmsg->data[ 3 + j ] =	wxSendBuf[ posSend++ ];
								pmsg->sizeData++;
							}

							pNode->pObject = pmsg;
							LOCK_MUTEX( pobj->m_receiveMutex );
							dll_addNode( &pobj->m_receiveList, pNode );
							UNLOCK_MUTEX( pobj->m_receiveMutex );

						}
					}
				}

				// Another incoming call
				pobj->m_callIdx++;

			}


			// Total syslog line transmission

			if ( pobj->m_initFlag & SYSLOGDRV_RAW ) {

				wxString wxSendBuf = buf;
				totLength = wxSendBuf.Length();

				// How many frames
				nPack = totLength / 5;
				if ( totLength % 5 ) nPack++;

				posSend = 0;

				pNode = new dllnode;
				if ( NULL != pNode ) {

					pmsg = new canalMsg;
					if ( NULL != pmsg ) {

						pmsg->id = 0 + (1 << 8) + (509 << 16); // Class=509, Type = 1
						pmsg->obid = 0;
						pmsg->timestamp = (unsigned long)time( NULL );
						pmsg->flags = CANAL_IDFLAG_EXTENDED;
						pmsg->sizeData = 3;
						pmsg->data[ 0 ] = pobj->m_callIdx;	// Id for this incoming call
						pmsg->data[ 1 ] = 0;				// Log Level
						pmsg->data[ 2 ] = nPack;			// Total number of fragments
						for ( int j=0;
								( ( posSend < wxSendBuf.Length() ) && ( j<5 ) );
								j++ ) {
							pmsg->data[ 3 + j ] =	wxSendBuf[ posSend++ ];
							pmsg->sizeData++;
						}

						pNode->pObject = pmsg;
						LOCK_MUTEX( pobj->m_receiveMutex );
						dll_addNode( &pobj->m_receiveList, pNode );
						UNLOCK_MUTEX( pobj->m_receiveMutex );

					}
				}
			}
		}


	} // while


#ifdef WIN32
	ExitThread( errorCode );
#else
	pthread_exit( &rv );
#endif

}
