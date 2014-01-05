// UdpServer.cpp: implementation of the CUdpServer class.
//
// Copyright (C) 2000-2013 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <stdio.h>
#include <sys/types.h>

#ifdef LINUX
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#else
#include <winsock.h>
#endif

#include "socketdll.h"
#include "UdpServer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUdpServer::CUdpServer()
{
	m_sockid = 0;	
}

CUdpServer::~CUdpServer()
{
	closeDown();
}


///////////////////////////////////////////////////////////////////////////////
// init
//

bool CUdpServer::init( int port ) 
{
	WSADATA info;
	if (WSAStartup(MAKEWORD(1,1), &info) != 0) {
		return false;
	}
	
	if ( ( m_sockid = socket( AF_INET, SOCK_DGRAM, 0 ) ) < 0 ) {
		// Error
		return false;
	}
	else {

#ifdef LINUX
		bzero( (char *) &m_my_addr, sizeof( m_my_addr ) );
#else
		memset( (char *) &m_my_addr, 0, sizeof( m_my_addr ) );
#endif

		m_my_addr.sin_family = AF_INET;
		m_my_addr.sin_addr.s_addr = htons( INADDR_ANY );
		m_my_addr.sin_port = htons( port );
		
		// Bind to the socket
		if ( ( bind( m_sockid, 
						( struct sockaddr *) &m_my_addr, 
						sizeof( m_my_addr ) ) < 0 ) ) {
			// Error
			m_sockid = 0;
			return false;
		}
	}	

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// closeDown
//

void CUdpServer::closeDown( void ) 
{
	if ( 0 != m_sockid ) { 
#ifdef LINUX
		close( m_sockid );
#else
		closesocket( m_sockid );
#endif
		m_sockid = 0;
	}
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

bool CUdpServer::receive( canalMsg *pCanalMsg )
{
	int nread;
	int addrlen;

	// The socket must be open
	if ( 0 == m_sockid ) return false;

	// This routine should block until a message is received
	nread = recvfrom( m_sockid,
						(char *)pCanalMsg, 
						sizeof( canalMsg ), 
						0,
						(struct sockaddr *) &m_client_addr, 
						&addrlen );

	if ( nread <= 0 ) {
		return false;
	}

   return true;
}


///////////////////////////////////////////////////////////////////////////////
// transmit
//

bool CUdpServer::transmit( canalMsg *pCanalMsg  )
{
	int nsend;
	struct sockaddr_in server_addr;

#ifdef LINUX
	bzero( (char *) &server_addr, sizeof( server_addr ) );
#else
	memset( (char *) &server_addr, 0, sizeof( server_addr ) );
#endif
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr( "255.255.255.255" );	// Broadcast
	server_addr.sin_port = htons( CANAL_UDP_PORT_ID );

	// The socket must be open
	if ( 0 == m_sockid ) return false;

	nsend = sendto( m_sockid,
						(char *)pCanalMsg, sizeof( canalMsg ), 0,
						(struct sockaddr *) &server_addr,
						sizeof( server_addr ) );

	if ( ( nsend <= 0 )  || ( nsend < sizeof( canalMsg )  ) ) {
		return false;
	}

	return true;
}
