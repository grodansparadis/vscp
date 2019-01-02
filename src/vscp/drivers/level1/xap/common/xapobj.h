// xapobj.h: 
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

#if !defined(XAPOBJ__INCLUDED_)
#define XAPOBJ__INCLUDED_

#ifdef WIN32

#include <windows.h>
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "stdio.h"
#include <time.h>
#include <winsock.h>

#else

#define _POSIX
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <syslog.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <string.h>
#include <syslog.h>
#include <netdb.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#endif

#include <canal.h>
#include <vscp.h>
#include <canal_macro.h>
#include <dllist.h>


#define CANAL_DLL_XAPDRV_OBJ_MUTEX	TEXT("___CANAL__DLL_XAPDRV_OBJ_MUTEX____")
#define CANAL_DLL_XAPDRV_PORT_MUTEX	TEXT("___CANAL__DLL_XAPDRV_PORT_MUTEX____")
#define CANAL_DLL_XAPDRV_RECEIVE_MUTEX	TEXT("___CANAL__DLL_XAPDRV_RECEIVE_MUTEX____")
#define CANAL_DLL_XAPDRV_TRANSMIT_MUTEX TEXT("___CANAL__DLL_XAPDRV_TRANSMIT_MUTEX____")

// Max messages in input queue
#define XAP_MAX_RCVMSG		512

// Max messages in output queue
#define XAP_MAX_SNDMSG		512

// xAP defines
#define XAP_DEFAULT_PORT	3639
#define XAP_MAX_CLIENTS		100
#define XAP_HEARTBEAT_TIME	60

class xAPClient
{
public:

	/// Constructor
	xAPClient();
	
	/// Destructor
	virtual ~xAPClient();

	/*!
		Check if this entry is assigned to a
		client or not
		@return True if not assigned to a client
	*/
	bool isFree( void ) { return !m_bAlive; };

	/*!
		Check if this client have timeout. That is
		if no heartbeat has been received in the set interval
		period. If it has timed out it will init all data to
		default values.
		@return true if timeout.
	*/
	bool checkTimeout( void );

	/*!
		Set data for a new xAP client
		@param pClient pointer to socket data
		@param interval Heartbeat interval data
	*/
#ifdef WIN32
	void setClientData( SOCKADDR_IN *pClient, short interval );
#else
	void setClientData( struct sockaddr_in *pClient, short interval );
#endif

	/*!
		Check to see if xAP socket client is the same as
		the one stored
		@param pClient pointer to socket data
		@return true if they are equal.
	*/
#ifdef WIN32
	bool isSameClient( SOCKADDR_IN *pClient );
#else
	bool isSameClient( struct sockaddr_in *pClient );
#endif

	void UpdateHeartBeat( void ) { time( &m_lastHeartBeat); };

	/// Client socket data
#ifdef WIN32
	SOCKADDR_IN m_saClient; 
#else
	struct sockaddr_in m_saClient;
#endif
	
	/// timout interval in seconds between heartbeats
	short m_interval;		
	
	/// true if heartbeat received timely
	bool m_bAlive;			

	/// time for last heatbeat
	time_t m_lastHeartBeat; 

};


class CXAPObj
{

public:

	/// Constructor
	CXAPObj();
	
	/// Destructor
	virtual ~CXAPObj();

	
	/*!
		Filter message

		@param pcanalMsg Pointer to CAN message
		@return True if message is accepted false if rejected
	*/
	bool doFilter( canalMsg *pcanalMsg );

	
	/*!
		Set Filter
	*/
	bool setFilter( unsigned long filter );


	/*!
		Set Mask
	*/
	bool setMask( unsigned long mask);


	/*! 
		Open the interface

		@param szFileName the name of the log file
		@param flags 	bit 1 = 0 Append, bit 1 = 1 Rewrite
		@return True on success.
	*/
	bool open( const char * szFileName, unsigned long flags = 0 );


	/*!
		Flush and close the log file
	*/
	bool close( void );


	/*!
		Get Interface statistics
		@param pCanalStatistics Pointer to CANAL statistics structure
		@return True on success.
	*/
	bool getStatistics( PCANALSTATISTICS pCanalStatistics );


	/*!
		Write a message out to the file
		@param pcanalMsg Pointer to CAN message
		@return True on success.
	*/
	bool writeMsg( canalMsg *pMsg );

	
	/*!
		Read a message fro the device
		@param pcanalMsg Pointer to CAN message
		@return True on success.
	*/
	bool readMsg( canalMsg *pMsg );


	/*!
		Check for data availability
		@return Number of packages in the queue
	*/
	int dataAvailable( void );

	/*!
		Get device status
		@param pCanalStatus Pointer to CANAL status structure
		@return True on success.
	*/
	bool getStatus( PCANALSTATUS pCanalStatus );

	/*!
		Check if a xAP HUB is present on the current system
		@return True if HUB is present false otherwise.
	*/
	bool testForxAPHub( void );

	/*!
		Get a datavalue from a text string. The value can
		be decimal or hexadecimal.

		@param szData pointer to string
		@return value as a long.
	*/
	unsigned long getDataValue( const char *szData );

public:

	/// Run flag
	bool m_bRun;


	/*!
		Interface statistics
	*/
	canalStatistics m_stat;


	/*!
		Interface status

		Bit 0  - Reserved.
		Bit 1  - Reserved.
		Bit 2  - Reserved.
		Bit 3  - Reserved.
		Bit 4  - Reserved.
		Bit 5  - Reserved.
		Bit 6  - Reserved.
		Bit 7  - Reserved.
		Bit 8  - Reserved.
		Bit 9  - Reserved.
		Bit 10 - Reserved.
		Bit 11 - Reserved.
		Bit 12 - Reserved.
		Bit 13 - Reserved.
		Bit 14 - Reserved.
		Bit 15 - Reserved.
		Bit 16 - Reserved.
		Bit 17 - Reserved.
		Bit 18 - Reserved.
		Bit 19 - Reserved.
		Bit 20 - Reserved.
		Bit 21 - Reserved.
		Bit 22 - Reserved.
		Bit 23 - Reserved.
		Bit 24 - Reserved.
		Bit 25 - Reserved.
		Bit 26 - Reserved.
		Bit 27 - Reserved.
		Bit 28 - Reserved.
		Bit 29 - Reserved.
		Bit 30 - Reserved.
		Bit 31 - Reserved.
	*/
	canalStatus m_status;

 
	/*! 
		Transmit queue

		This is the transmit queue for messages going out to the
		device
	*/
	DoubleLinkedList m_transmitList;

	/*! 
		Receive queue

		This is the receive queue for messages going in to the
		device
	*/
	DoubleLinkedList m_receiveList;

	/*!
		Tread id Receive
	*/
#ifdef WIN32
	HANDLE m_hTreadReceive;
#else
	pthread_t m_threadIdReceive;
#endif

	/*!
		Tread id Transmit
	*/
#ifdef WIN32
	HANDLE m_hTreadTransmit;
#else
	pthread_t m_threadIdTransmit;
#endif

	/*!
		Mutex for device.
	*/
#ifdef WIN32	
	HANDLE m_xapMutex;
#else
	pthread_mutex_t m_xapMutex;
#endif

	/*!
		Mutex for receive queue.
	*/
#ifdef WIN32	
	HANDLE m_receiveMutex;
#else
	pthread_mutex_t m_receiveMutex;
#endif

	/*!
		Mutex for transmit queue.
	*/
#ifdef WIN32	
	HANDLE m_transmitMutex;
#else
	pthread_mutex_t m_transmitMutex;
#endif

	/*!
		Mutex that holds the transmit thread start until the receive
		thread has found a free port
	*/
#ifdef WIN32	
	HANDLE m_portMutex;
#else
	pthread_mutex_t m_portMutex;
#endif

	/*!
		Filter for outgoing messages

		mask bit n	|	filter bit n	| msg id bit	|	result
		===========================================================
			0				X					X			Accept				
			1				0					0			Accept
			1				0					1			Reject
			1				1					0			Reject
			1				1					1			Accept

		Formula is !( ( filter �d ) & mask )
	*/
	uint32_t m_filter;

	/*!
		Mask for outgoing messages	
	*/
	uint32_t m_mask;


	///////////////////////////////////////////////////////////////////////////
	//				 * * * * *  xAP specific * * * * *
	///////////////////////////////////////////////////////////////////////////


	/*!
		flags 
		-----------------------------------------------------------------------------

 
	*/
	uint32_t m_initFlag;


	/*!
		Incoming filter
	*/
	uint32_t m_XAP_filter;


	/*!
		Incoming mask
	*/
	uint32_t m_XAP_mask;


	/*!
		xAP host port
	*/
	int m_Port;

	/*!
		xAP User ID
	*/
	int m_UID;

	/*!
		Holder for the unique xAP class for this daemon and
		this instance.
	*/
	char m_strxapclass[ 128 ];

	/*!
		Last part of UID (byte part)	
	*/
	unsigned char m_UIDextension;

	/*!
		xAP Client list
		If the driver is in HUB mode this list contains all active
		clients. If in client mode the first entry holds the HUB
		address and port information. This is normally 127.0.0.1:3639 
		but is built this way to allow other addresses and ports.
	*/
	xAPClient m_xAPClients[ XAP_MAX_CLIENTS ];


	/*!
		This flag is set to true of the driver should have full
		HUB functionality. If set to fase the driver is a HUB client.
	*/
	bool m_bHubmode;
};

/*!
	Structure used to transfer data to xAP client message send
	thread. 
	The data can be used in HUB mode in which case the driver
	act as a HUB or in client mode in which the driver is a client to 
	a HUB.
*/

typedef struct _xapsend {
	bool m_bHubmode;		// True for hubmode
	CXAPObj *m_pxapobj;
	char m_msgBuf[ 2048 ];
	short size;
} xapsend;


#endif // !defined(AFX_XAP_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_)
