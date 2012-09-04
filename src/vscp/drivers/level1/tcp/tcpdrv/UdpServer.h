// UdpServer.h: interface for the CUdpServer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UDPSERVER_H__1BEF5E51_C7EE_47DB_92A6_477078A4B702__INCLUDED_)
#define AFX_UDPSERVER_H__1BEF5E51_C7EE_47DB_92A6_477078A4B702__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../common/canal.h"

#ifdef LINUX
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#else
#include <winsock.h>
#endif


class CUdpServer  
{

public:
	
	/// Constructor
	CUdpServer();

	/// Destructor
	virtual ~CUdpServer();

	/**
		init

		initialize the UDP listen socket
	*/
	bool init( int port = CANAL_UDP_PORT_ID );


	/**
		close

		Close the UDP listen socket.
	*/
	void closeDown( void );


	/**
		receive

		Receive one packet on the UDP socket
	*/
	bool receive( canalMsg *pCanalMsg );

	/**
		transmit

		Transmit one packet on the UDP socket
	*/
	bool transmit( canalMsg *pCanalMsg );

private: 

	/// Port which the server listens on
	int m_port;		

	// id for socket
	int m_sockid;

	/**
	
	*/
	struct sockaddr_in m_my_addr;
	
	/**

	*/
	struct sockaddr_in m_client_addr;
};

#endif // !defined(AFX_UDPSERVER_H__1BEF5E51_C7EE_47DB_92A6_477078A4B702__INCLUDED_)
