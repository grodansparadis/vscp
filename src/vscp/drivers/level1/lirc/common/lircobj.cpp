// LircInterface.cpp: implementation of the CLircObj class.
//
// Copyright (C) 2000-2015
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
//////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"

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
#include <expat.h>
#include "lircobj.h"

WX_DEFINE_LIST(EventList);
WX_DEFINE_LIST(ReceiveQueue);
WX_DEFINE_LIST(TransmitQueue);

// Prototypes threads
#ifdef WIN32
void workThreadTransmit( void *pObject );
void workThreadReceive( void *pObject );
#else
void *workThreadTransmit( void *pObject );
void *workThreadReceive( void *pObject );
#endif

// Prototypes
static void XMLCALL startElement( void *userData, const char *name, const char **atts );
static void XMLCALL endElement( void *userData, const char *name );
static void XMLCALL characterDataHandler( void *userData, const XML_Char *s, int len );
static unsigned long getDataValue( const char *szData );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLircObj::CLircObj()
{
	// Set default LIRC host
	strcpy( m_lircHost, "localhost" );

	// Set default LIRC port
	m_lircPort = LIRC_PORT;

	// No configuration file yet
	*m_pathToConfigFile = 0;

	// Init XML config file parser
	m_depth = 0;
	m_bCanalLirc = false;
	m_bObject =  false;
	m_bVscpObject = false;
	m_bCanObject = false;

	m_drvidx = 0;

	// No filter/mask
	m_filter = 0;
	m_mask = 0;

	m_bRun = false;

	readConfigData();		// Load configuration data

}

CLircObj::~CLircObj()
{
	close();
}

///////////////////////////////////////////////////////////////////////////////
//  readConfigData
//
void CLircObj::readConfigData()
{
	if ( !wxFileName::FileExists( m_pathToConfigFile ) ) return;

	// Init XML parser
	XML_Parser parser = XML_ParserCreate( "ISO-8859-1" );
	XML_SetUserData( parser, this );
	XML_SetElementHandler(	parser,
							startElement,
							endElement );
	XML_SetCharacterDataHandler( parser,
								 characterDataHandler );

	wxFileInputStream fis( m_pathToConfigFile );

	char buf[ 8192 ];
	while ( !( fis.Read( buf, sizeof( buf ) ) ).Eof() ) {
		if ( XML_STATUS_ERROR == XML_Parse( parser, buf, fis.LastRead(), false ) ) {
			return;
		}
	}

	XML_Parse( parser, buf, fis.LastRead(), true );

	// Free parser resources
	XML_ParserFree( parser );

}


///////////////////////////////////////////////////////////////////////////////
//  open
//
// filename
//-----------------------------------------------------------------------------
// Parameters for the driver as a string on the following form
//
// "path to config file;lirchost"
//
// path to config file
// ===================
// The full path to the location for the configuration XML-file
//
// lirchost
// ========
// hostname for the host the lirc daemon is running on. If not given "localhost"
// is assumed.
//
// lircport
// ========
// The port that the lirc daemon is listening to. If not given the defaultport
// "8765" i used.
//
//
// Flags
// =====
// Flags not used
//

bool CLircObj::open( const char *szFileName, unsigned long flags )
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
		strncpy( m_pathToConfigFile, p, sizeof( m_pathToConfigFile ) );
	}

	// hostname for host that LIRC daemon is on
	// if not given localhost is assumed
	p = strtok( NULL, ";" );
	if ( NULL != p ) {
		if ( 0 != strlen( p ) ) {
			strncpy( m_lircHost, p, sizeof( m_lircHost ) );
		}
	}

	// LIRC port
	// if not given the default lirc port (8765) is set.
	p = strtok( NULL, ";" );
	if ( NULL != p ) {
		m_lircPort = ::getDataValue( p );
	}

	// We are open
	m_bRun = true;

	// Read configuration data
	readConfigData();

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

	// Release the mutex
	//UNLOCK_MUTEX( m_vectorMutex );
	//UNLOCK_MUTEX( m_receiveMutex );
	//UNLOCK_MUTEX( m_transmitMutex );


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

bool CLircObj::close( void )
{
	// Do nothing if already terminated
	if ( !m_bRun ) return false;

	m_bRun = false;

	m_csHash.Enter();

	// iterate over all the elements in the class
    LircHash::iterator it;
    for( it = m_lircHash.begin(); it != m_lircHash.end(); ++it )
    {

		if ( NULL != m_lircHash[ it->first ] ) {


			// Remove list elements
			for ( EventList::Node *node = m_lircHash[ it->first ]->GetFirst();
																node;
																node = node->GetNext() ) {
				// Delete the load
				delete node->GetData();
			}

			// Delete the list
			delete m_lircHash[ it->first ];
		}
    }

	m_csHash.Leave();

	// End the thread
	//m_pworkThread->Delete();

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

bool CLircObj::doFilter( canalMsg *pcanalMsg )
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

bool CLircObj::setFilter( unsigned long filter )
{
	m_filter = filter;
	return true;
}


//////////////////////////////////////////////////////////////////////
// setMask
//

bool CLircObj::setMask( unsigned long mask )
{
	m_mask = mask;
	return true;
}


//////////////////////////////////////////////////////////////////////
// writeMsg
//

bool CLircObj::writeMsg( canalMsg *pMsg )
{
	return false;	// Not implemented yet ( if ever )
}


//////////////////////////////////////////////////////////////////////
// readMsg
//

bool CLircObj::readMsg( canalMsg *pMsg )
{
	bool rv = false;

	// --------------------- Critical Section ---------------------
	m_csRcvQueue.Enter();

	if ( !m_receiveQueue.IsEmpty() ) {
		canalMsg *pmsg = ( m_receiveQueue.GetFirst() )->GetData();
		if ( NULL != pmsg ) {
			memcpy( pMsg, pmsg, sizeof( canalMsg ) );
			m_receiveQueue.DeleteNode( m_receiveQueue.GetFirst() );
			rv = true;
		}
	}

	m_csRcvQueue.Leave();
	// --------------------- Critical Section ---------------------


	return rv;
}


///////////////////////////////////////////////////////////////////////////////
//	dataAvailable
//

int CLircObj::dataAvailable( void )
{
	int cnt;

	// --------------------- Critical Section ---------------------
	m_csRcvQueue.Enter();
	cnt = m_receiveQueue.GetCount();
	m_csRcvQueue.Leave();
	// --------------------- Critical Section ---------------------

	return cnt;
}



///////////////////////////////////////////////////////////////////////////////
//	getStatistics
//

bool CLircObj::getStatistics( PCANALSTATISTICS pCanalStatistics )
{
	// Must be a valid pointer
	if ( NULL == pCanalStatistics ) return false;

	memcpy( pCanalStatistics, &m_stat, sizeof( canalStatistics ) );

	return true;
}


///////////////////////////////////////////////////////////////////////////////
//	getStatus
//

bool CLircObj::getStatus( PCANALSTATUS pCanalStatus )
{
	// Must be a valid pointer
	if ( NULL == pCanalStatus ) return false;


	memcpy( pCanalStatus, &m_status, sizeof( canalStatus ) );

	return true;
}


///////////////////////////////////////////////////////////////////////////////
//  parseLircLine
//

void CLircObj::parseLircLine( wxString &wxstr )
{
	int idx = 0;

	wxStringTokenizer tkz( wxstr, _T(" ") );
	while ( tkz.HasMoreTokens() ) {

		wxString token = tkz.GetNextToken();


		// process token here
		token.UpperCase();

		// --------------------- Critical Section ---------------------
		m_csHash.Enter();

		if ( ( NULL != m_lircHash[ token ] ) ) {

			// Send events
			for ( EventList::Node *node = m_lircHash[ token ]->GetFirst();
														node;
														node = node->GetNext() ) {

				canalMsg *msg = node->GetData();
				m_receiveQueue.Append( msg );

			}


		}

		m_csHash.Leave();
		// --------------------- Critical Section ---------------------

		idx++; // One more token

	}
}






///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//								  X M L
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////






///////////////////////////////////////////////////////////////////////////////
// startElement
//

static void XMLCALL
startElement( void *userData, const char *name, const char **atts )
{
	CLircObj *pworkObj = (CLircObj *)userData;
	if ( NULL == pworkObj ) return;

	pworkObj->m_depth += 1;

	// canallirc
	if ( 0 == strcmp( name, _("lirc") ) ) {
		pworkObj->m_bCanalLirc = true;
		// We set a tagname as dummy incase there is some problem or a
		// missing "codetag". This way elements with no code tag are
		// added to this list
		strcpy( pworkObj->m_keytag, _("dummy") );
	}

	// object
	if ( 0 == strcmp( name, _("object") ) ) {
		pworkObj->m_bObject = true;
	}

	// vscp
	else if ( 0 == strcmp( name, _("vscp") ) ) {
		pworkObj->m_bVscpObject = true;
		// Nill the message
		memset( &pworkObj->m_workMsg, 0, sizeof( canalMsg ) );
	}

	// can
	else if ( 0 == strcmp( name, _("can") ) ) {
		pworkObj->m_bCanObject = true;
		// Nill the message
		memset( &pworkObj->m_workMsg, 0, sizeof( canalMsg ) );
	}

	// information tags
	else {

		// all other tags
		strncpy( pworkObj->m_curtagname, name, sizeof( pworkObj->m_curtagname ) );
		memset( pworkObj->m_xmlbuf, 0, sizeof( pworkObj->m_xmlbuf ) );

	}

}


///////////////////////////////////////////////////////////////////////////////
// endElement
//
// IsDBCSLeadByte
//

static void XMLCALL
endElement( void *userData, const char *name )
{
	CLircObj *pworkObj = (CLircObj *)userData;
	if ( NULL == pworkObj ) return;

	pworkObj->m_depth -= 1;

	// lirc - next object
	if ( 0 == strcmp( name, _("lirc") ) ) {
		pworkObj->m_bCanalLirc = false;
	}

	// object - done
	if ( 0 == strcmp( name, _("object") ) ) {
		pworkObj->m_bObject = false;
	}

	// keytag
	else if ( 0 == strcmp( pworkObj->m_curtagname, _("codetag") ) ) {
		strncpy( pworkObj->m_keytag, pworkObj->m_xmlbuf, sizeof( pworkObj->m_keytag ) );
		_strupr( pworkObj->m_keytag );
	}


	// VSCP object
	else if ( 0 == strcmp( name, _("vscp") ) ) {

		pworkObj->m_bVscpObject = false;

		pworkObj->m_workMsg.flags |= CANAL_IDFLAG_EXTENDED;

		canalMsg *pmsg = new canalMsg;
		memcpy( pmsg, &pworkObj->m_workMsg, sizeof( canalMsg ) );
		if ( NULL == pworkObj->m_lircHash[ pworkObj->m_keytag ] ) {
			pworkObj->m_lircHash[ pworkObj->m_keytag ] = new EventList();
		}
		pworkObj->m_lircHash[ pworkObj->m_keytag]->Append( pmsg );

	}

	// CAN object - next object
	else if ( 0 == strcmp( name, _("can") ) ) {

		pworkObj->m_bCanObject = false;
		canalMsg *pmsg = new canalMsg;
		memcpy( pmsg, &pworkObj->m_workMsg, sizeof( canalMsg ) );
		if ( NULL == pworkObj->m_lircHash[ pworkObj->m_keytag ] ) {
			pworkObj->m_lircHash[ pworkObj->m_keytag ] = new EventList();
		}
		pworkObj->m_lircHash[ pworkObj->m_keytag ]->Append( pmsg );

	}

	// information tags
	else if ( ( 3 == pworkObj->m_depth ) && pworkObj->m_bObject && pworkObj->m_bCanalLirc  ) {

		if ( pworkObj->m_bVscpObject ) {

			// class
			if ( 0 == strcmp( pworkObj->m_curtagname, _("class") ) ) {
				pworkObj->m_workMsg.id |=
								( getDataValue( pworkObj->m_xmlbuf ) << 16 );
			}

			// type
			else if ( 0 == strcmp( pworkObj->m_curtagname, _("type") ) ) {
				pworkObj->m_workMsg.id |=
								( getDataValue( pworkObj->m_xmlbuf ) << 8 );
			}

			// priority
			else if ( 0 == strcmp( pworkObj->m_curtagname, _("priority") ) ) {
				pworkObj->m_workMsg.id |=
								( getDataValue( pworkObj->m_xmlbuf ) << 26 );
			}

			// nickname
			else if ( 0 == strcmp( pworkObj->m_curtagname, _("nickname") ) ) {
				pworkObj->m_workMsg.id |= getDataValue( pworkObj->m_xmlbuf );
			}

			// sizeData
			else if ( 0 == strcmp( pworkObj->m_curtagname, _("sizedata") ) ) {
				pworkObj->m_workMsg.sizeData = getDataValue( pworkObj->m_xmlbuf );
			}

			// Data "val1,val2,val3,val4...."
			else if ( 0 == strcmp( pworkObj->m_curtagname, _("data") ) ) {

				int idx = 0;
				wxStringTokenizer tok;
				tok.SetString( pworkObj->m_xmlbuf, _(",\r\n") );

				while( ( idx < 8 ) && tok.HasMoreTokens() ) {

					pworkObj->m_workMsg.data[ idx ] = getDataValue( tok.GetNextToken() );
					idx++;

				}

				pworkObj->m_workMsg.sizeData = idx;	// Set new size

			}
		}
		else if ( pworkObj->m_bCanObject ) {

			// extended
			if ( 0 == strcmp( pworkObj->m_curtagname, _("extended") ) ) {
				_strupr( pworkObj->m_xmlbuf );
				if ( ( 1 == atoi( pworkObj->m_xmlbuf ) ) ||
							( NULL != strstr( pworkObj->m_xmlbuf, "TRUE" ) ) ) {
					pworkObj->m_workMsg.flags |= CANAL_IDFLAG_EXTENDED;
				}
			}

			// id
			if ( 0 == strcmp( pworkObj->m_curtagname, _("id") ) ) {
				pworkObj->m_workMsg.id = getDataValue( pworkObj->m_xmlbuf );
			}

			// sizeData
			else if ( 0 == strcmp( pworkObj->m_curtagname, _("sizedata") ) ) {
				pworkObj->m_workMsg.sizeData = getDataValue( pworkObj->m_xmlbuf );
			}

			// Data  "val1,val2,val3,val4...."
			else if ( 0 == strcmp( pworkObj->m_curtagname, _("data") ) ) {
				int idx = 0;
				wxStringTokenizer tok;
				tok.SetString( pworkObj->m_xmlbuf, _(",\r\n") );

				while( ( idx < 8 ) && tok.HasMoreTokens() ) {

					pworkObj->m_workMsg.data[ idx ] = getDataValue( tok.GetNextToken() );
					idx++;

				}

				pworkObj->m_workMsg.sizeData = idx;	// Set new size

			}
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
// characterDataHandler
//

static void XMLCALL
characterDataHandler( void *userData,
						const XML_Char *s,
						int len )
{
	CLircObj *pworkObj = (CLircObj *)userData;
	if ( NULL == pworkObj ) return;

	// Calculate room left in buffer
	int sz  = sizeof( pworkObj->m_xmlbuf ) -  strlen( pworkObj->m_xmlbuf );

	if ( len < sizeof( pworkObj->m_xmlbuf ) ) {

		if ( sz > len ) {
			strncat( pworkObj->m_xmlbuf, s, len );
		}
		else {
			strncat( pworkObj->m_xmlbuf, s, sz );
		}
	}

}


///////////////////////////////////////////////////////////////////////////////
// getDataValue
//

unsigned long getDataValue( const char *szData )
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
	unsigned int sockfd;
	char buf[ 512 ]; // Buffer for raw Lirc Data
	struct hostent *he;
    struct sockaddr_in server_addr; // connector's address information

#ifdef WIN32
	DWORD errorCode = 0;
#else
	int rv = 0;
#endif

	CLircObj * pobj = ( CLircObj *)pObject;
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

	// get the host info
	if ( NULL == ( he = gethostbyname("localhost") ) ) {
#ifdef WIN32
		ExitThread( errorCode ); // Fail
#else
		pthread_exit( &rv );
#endif
    }

	if ( SOCKET_ERROR == ( sockfd = socket( AF_INET, SOCK_STREAM, 0 ) ) ) {
#ifdef WIN32
		ExitThread( errorCode ); // Fail
#else
		pthread_exit( &rv );
#endif
    }

	server_addr.sin_family = AF_INET;		// host byte order
    server_addr.sin_port = htons( LIRC_PORT );	// short, network byte order
    server_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(server_addr.sin_zero), '\0', 8);  // zero the rest of the struct

    if ( SOCKET_ERROR == connect( sockfd,
									(struct sockaddr *)&server_addr,
									sizeof( struct sockaddr ) ) ) {
#ifdef WIN32
		ExitThread( errorCode ); // Fail
#else
		pthread_exit( &rv );
#endif
	}

	struct timeval tv;
	fd_set readfds;

    tv.tv_sec = 0;
    tv.tv_usec = 300000;

	FD_ZERO( &readfds );
    FD_SET( sockfd, &readfds );



	while ( pobj->m_bRun ) {

		// Noting to do if we should end...
		if ( !pobj->m_bRun ) continue;


		FD_SET( sockfd, &readfds );

		select( 0, &readfds, NULL, NULL, &tv );

		if ( FD_ISSET( sockfd, &readfds ) ) {

			// read raw data
			memset( buf, 0, sizeof( buf ) );
			if ( SOCKET_ERROR != recv( sockfd, buf, sizeof( buf ), 0 ) ) {

				wxStringTokenizer tkz( buf, "\n" );
				while ( tkz.HasMoreTokens() ) {
					wxString token = tkz.GetNextToken();
					pobj->parseLircLine( token );
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
