// can4vscpobj.h: 
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2012 Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
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
// $RCSfile: can4vscpobj.h,v $                                       
// $Date: 2005/05/29 19:20:08 $                                  
// $Author: akhe $                                              
// $Revision: 1.5 $ 

#if !defined(AFX_IXXATVCI_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_)
#define AFX_IXXATVCI_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_

#ifdef WIN32

#include <windows.h>
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../../../../../common/com_win32.h"

#else

#define _POSIX
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <syslog.h>

#endif

#include <stdio.h>
#include "../../../../common/canal.h"
#include "../../../../common/vscp.h"
#include "../../../../common/canal_macro.h"
#include "../../../../../common/dllist.h"

// define to create a file c:\can4vscp.txt with input and
// stat machine data for debugging
//#define DEBUG_CAN4VSCP_RECEIVE

#define CANAL_DLL_CAN4VSCPDRV_OBJ_MUTEX			"___CANAL__DLL_CAN4VSCPDRV_OBJ_MUTEX____"
#define CANAL_DLL_CAN4VSCPDRV_RECEIVE_MUTEX		"___CANAL__DLL_CAN4VSCPDRV_RECEIVE_MUTEX____"
#define CANAL_DLL_CAN4VSCPDRV_TRANSMIT_MUTEX	"___CANAL__DLL_CAN4VSCPDRV_TRANSMIT_MUTEX____"
#define CANAL_DLL_CAN4VSCPDRV_RESPONSE_MUTEX	"___CANAL__DLL_CAN4VSCPDRV_RESPONSE_MUTEX____"

// Max messages in input queue
#define CAN4VSCP_MAX_RCVMSG			512

// Max messages in output queue
#define CAN4VSCP_MAX_SNDMSG			512

// Max number of response messages in respnse queue
#define CAN4VSCP_MAX_RESPONSEMSG	32

// Byte stuffing start and end characters
#define DLE						0x10
#define STX						0x02
#define ETX						0x03

// RX State machine
#define INCOMING_STATE_NONE			0	// Waiting for <STX>
#define INCOMING_STATE_STX			1	// Reading data
#define INCOMING_STATE_ETX			2	// <ETX> has been received
#define INCOMING_STATE_COMPLETE		3	// Frame received

#define INCOMING_SUBSTATE_NONE		0	// Idle
#define INCOMING_SUBSTATE_DLE		1	// <DLE> received


// Can4VSCP Commands
#define RESET_NOOP				0x00	// No Operation
#define RESET_CPU				0x01	// Reset
#define	GET_TX_ERR_CNT			0x02	// Get TX error count
#define	GET_RX_ERR_CNT			0x03	// Get RX error count
#define	GET_CANSTAT				0x04	// Get CAN statistics
#define	GET_COMSTAT				0x05
#define	GET_MSGFILTER1			0x06	// Get message filter 1
#define	GET_MSGFILTER2			0x07	// Get message filter 2
#define	SET_MSGFILTER1			0x08	// Set message filter 1
#define	SET_MSGFILTER2			0x09	// Set message filter 2
#define SET_CONFIG_MODE			0x10	// Set config mode
#define SET_LOOPBACK_MODE		0x11	// Set loopback mode
#define SET_NORMAL_MODE			0x12	// Set normal mode
#define SET_SLEEP_MODE			0x13	// Set sleep mode
#define SET_LISTEN_MODE			0x14	// Set listen mode
#define GET_BAUD_REGS			0x15	// Get Baudrate registers
#define RESET_MICRO				0x16	// Reset device micro
#define GET_HARDWARE_VERSION	0x17	// Get hardware version
#define GET_FIRMWARE_VERSION	0x18	// Get firmware version


// Emergency flags
#define EMERGENCY_OVERFLOW		0x01
#define EMERGENCY_RCV_WARNING	0x02
#define EMERGENCY_TX_WARNING	0x04
#define EMERGENCY_TXBUS_PASSIVE	0x08
#define EMERGENCY_RXBUS_PASSIVE	0x10
#define EMERGENCY_BUS_OFF		0x20


// Adapter transmission mode
enum TRMODE {
	
	TRMODE_CONFIG,
	TRMODE_LISTEN,
	TRMODE_LOOPBACK,
	TRMODE_DISABLED,
	TRMODE_NORMAL,
};


//
// The response structure
//
typedef struct {
	uint32_t id;
	uint8_t	flags;
	uint8_t data[ 8 ];
} responseMsg;




class CCan4VSCPObj
{

public:

	/// Constructor
	CCan4VSCPObj();
	
	/// Destructor
	virtual ~CCan4VSCPObj();

	
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


#ifdef DEBUG_CAN4VSCP_RECEIVE
	/// The log file handle
	FILE *m_flog;
#endif

	/*! 
		Open/create the logfile

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
		Send a command

		@param cmdcode Code for command
		@param data Pointer to data to send
		@param dataSize Size for datablock
		@return True on success.
	*/
	bool sendCommand( uint8_t cmdcode, uint8_t *pData = NULL, uint16_t dataSize = 0 );


	/*!
		Wait for a response message

		@param pMsg Pointer to response message
		@param timeout Timeout in milliseconds
		@return True on success

	*/
	bool waitResponse( responseMsg *pMsg, uint32_t timeout );

	/*!
		Send command and wait for a response message
	
		@param cmdcode Code for command
		@param pMsg Pointer to response message
		@param timeout Timeout in milliseconds
		@return True on success

	*/
	bool sendCommandWait( uint8_t cmdcode, responseMsg *pMsg, uint32_t timeout );


	/*!
		Send command on the serial channel

		@param pdata data to send
		@param dataSize Total number of bytes to send
		@return True on success
	*/
	bool sendMsg( uint8_t *pdata, short dataSize );


	/*!
		Read serial data and feed to state machine
		@return true when a full frame is received.
	*/
	bool serialData2StateMachine( void );


	bool readSerialData( void );

public:

	/// Run flag
	bool m_bRun;


	/*!
		Interface statistics
	*/
	canalStatistics m_stat;


	/*!
		Interface status

		Bit 0  - TX Error Counter.
		Bit 1  - TX Error Counter.
		Bit 2  - TX Error Counter.
		Bit 3  - TX Error Counter.
		Bit 4  - TX Error Counter.
		Bit 5  - TX Error Counter.
		Bit 6  - TX Error Counter.
		Bit 7  - TX Error Counter.
		Bit 8  - RX Error Counter.
		Bit 9  - RX Error Counter.
		Bit 10 - RX Error Counter.
		Bit 11 - RX Error Counter.
		Bit 12 - RX Error Counter.
		Bit 13 - RX Error Counter.
		Bit 14 - RX Error Counter.
		Bit 15 - RX Error Counter.
		Bit 16 - Overflow.
		Bit 17 - RX Warning.
		Bit 18 - TX Warning.
		Bit 19 - TX bus passive.
		Bit 20 - RX bus passive..
		Bit 21 - Reserved.
		Bit 22 - Reserved.
		Bit 23 - Reserved.
		Bit 24 - Reserved.
		Bit 25 - Reserved.
		Bit 26 - Reserved.
		Bit 27 - Reserved.
		Bit 28 - Reserved.
		Bit 29 - Bus Passive.
		Bit 30 - Bus Warning status
		Bit 31 - Bus off status 

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
		Response queue

		This is the receive queue for command response messages
	*/
	DoubleLinkedList m_responseList;


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
	HANDLE m_can4vscpMutex;
#else
	pthread_mutex_t m_can4vscpMutex;
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
		Mutex for command response queue.
	*/
#ifdef WIN32	
	HANDLE m_responseMutex;
#else
	pthread_mutex_t m_responseMutex;
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

		Formula is !( ( filter îd ) & mask )
	*/
	uint32_t m_filter;

	/*!
		Mask for outgoing messages	
	*/
	uint32_t m_mask;


	///////////////////////////////////////////////////////////////////////////
	//				 * * * * *  Can4VSCP specific * * * * *
	///////////////////////////////////////////////////////////////////////////


	/*!
		flags 
		-----------------------------------------------------------------------------

		bit 0
		=====
		0		11 bit identifier mode
		1		29 bit identifier mode

		bit 2
		=====
		0		High speed
		1		A low speed-busconnector is used
				(if provided by the hardware)

		bit 2
		=====
		0		Filter our own TX messages from our receive..
		1		All sent CAN objects appear
				as received CAN objects in the
				rx queues.

		bit 3
		=====
		0		Active Mode.
		1		Passive mode: CAN controller works as passive 
				CAN node (only monitoring) and
				therefore it does not send any
				acknowledge bit for CAN objects
				sent by another CAN node.

		bit 4
		=====
		0		No error report objects.
		1		Errorframes are detected and
				reported as CAN objects via
				the rx queues
 
	*/
	uint32_t m_initFlag;


	CComm m_com;


	/*!
		State for incoming frames
	*/
	int  m_RxMsgState;

	/*!
		Substate for incoming frames
	*/
	int m_RxMsgSubState;


	/*!
		General receive buffer
	*/
	uint8_t m_bufferRx[ 0x10000 ];

	/*!
		Message receive buffer
	*/
	uint8_t m_bufferMsgRcv[ 13 ];

	/*!
		Current length for received message
	*/
	uint16_t m_lengthMsgRcv;

	
	/*!
		Transmit queue handle
	*/
	uint16_t m_hTxQue;
	
	/*!
		Receive queue handle
	*/
	uint16_t m_hRxQue;	

 
};

#endif // !defined(AFX_IXXATVCI_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_)
