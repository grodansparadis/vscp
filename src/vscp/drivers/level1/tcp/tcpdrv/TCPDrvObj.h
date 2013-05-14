// TcpDrvObj.h: interface for the TcpDrvObj class.
//
// Copyright (C) 2000-2013 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UDPDRVOBJ_H__490B2376_F5F3_4287_BB9F_A4B4BF0A12CD__INCLUDED_)
#define AFX_TcpDrvObj_H__490B2376_F5F3_4287_BB9F_A4B4BF0A12CD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../../../common/canal.h"
#include "../../../../common/dllist.h"
#include "winsock.h"

#define TCP_SOCKET_MAX_SNDMSG	512		// Transmit buffer size
#define TCP_SOCKET_MAX_RCVMSG	512		// Receive buffer size

#define TCP_SOCKET_DLL_RECEIVE_OBJ_MUTEX	"___CANAL_TCP_DLL____RECEIVE_OBJ_MUTEX____"

// Stuffed bytes
#define CANAL_SEQPOS_FLAGS		0
#define CANAL_SEQPOS_ID			4
#define CANAL_SEQPOS_OBID		8
#define CANAL_SEQPOS_TIMESTAMP	12
#define CANAL_SEQPOS_DATASIZE	13
#define CANAL_SEQPOS_DATA		21

class TcpDrvObj  
{
public:
	TcpDrvObj();
	virtual ~TcpDrvObj();

	/*!
		Initialize
	*/
	bool init( char * szHost, int port );

	/*!
		Release resources
	*/
	bool release( void );

	/*!
		Send a canal message
	*/
	bool send( PCANALMSG pMsg );

	/*!
		Receive a canal message
	*/
	bool receive( PCANALMSG pMsg );


	/*!
		Check if data is available
	*/
	int dataAvailable( void );

	/*!
		Get statistics
	*/
	bool getStatistics( PCANALSTATISTICS pCanalStatistics );

	
	/*!
		Get status	
	*/
	bool getStatus( PCANALSTATUS pCanalStatus );

public:

	/*!
		Thread run flag
	*/
	bool m_bRun;


	/*!
		Mutex for receive packets.
	*/
	HANDLE m_udpReceiveMutex;

	/*! 
		Send queue

		This is the send queue for messages going out on the
		net.
	*/
	DoubleLinkedList m_sndList;

	/*! 
		receive queue

		This is the receive queue for messages coming in from the net.
	*/
	DoubleLinkedList m_rcvList;

	/*!
		canal statistics
	*/
	canalStatistics m_stat;


	/*!
	*/
	canalStatus m_canalStatus;
};

#endif // !defined(AFX_TcpDrvObj_H__490B2376_F5F3_4287_BB9F_A4B4BF0A12CD__INCLUDED_)
