// apoxobj.h: 
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2004 Apox Controls
// Copyright (C) 2000-2014 
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
// $RCSfile: apoxobj.h,v $                                       
// $Date: 2005/05/31 22:11:40 $                                  
// $Author: akhe $                                              
// $Revision: 1.3 $ 

#if !defined(AFX_IXXATVCI_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_)
#define AFX_IXXATVCI_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_

#ifdef WIN32

#include <windows.h>
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#else

#define _POSIX
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <syslog.h>

#endif

#include "../../../../common/canal.h"
#include "../../../../common/vscp.h"
#include "../../../../common/canal_macro.h"
#include "../../../../../common/dllist.h"

// Apox Includes
#ifdef WIN32
#include <windows.h>
#include "FTD2XX.h"		// from ftdi-site http://www.ftdichip.com/
#endif 

#define CANAL_DLL_APOXDRV_OBJ_MUTEX			"___CANAL__DLL_APOXDRV_OBJ_MUTEX____"
#define CANAL_DLL_APOXDRV_RECEIVE_MUTEX		"___CANAL__DLL_APOXDRV_RECEIVE_MUTEX____"
#define CANAL_DLL_APOXDRV_TRANSMIT_MUTEX	"___CANAL__DLL_APOXDRV_TRANSMIT_MUTEX____"
#define CANAL_DLL_APOXDRV_RESPONSE_MUTEX	"___CANAL__DLL_APOXDRV_RESPONSE_MUTEX____"

// Max messages in input queue
#define APOX_MAX_RCVMSG		512

// Max messages in output queue
#define APOX_MAX_SNDMSG		512

// Max number of response messages in respnse queue
#define APOX_MAX_RESPONSEMSG	32

// Byte stuffing start and end characters
#define USB_DLE					0x10
#define USB_STX					0x02
#define USB_ETX					0x03

// USB Function status
#define USB_ERR_NO_USB			101
#define USB_ERR_FT				102
#define USB_ERR_NO_HEXFILE		103


// Apox Commands
#define WHICH_CODE_IS_RUNNING	0x00	// Bootmode or application code
#define RESET_CPU				0x01	// Reset
#define	GET_TX_ERR_CNT			0x02	// Get TX error count
#define	GET_RX_ERR_CNT			0x03	// Get RX error count
#define	GET_CANSTAT				0x04	// Get CAN statistics
#define	GET_COMSTAT				0x05
#define	GET_MSGFILTER1			0x06	// Get message filter 1
#define	GET_MSGFILTER2			0x07	// Get message filter 2
#define	SET_MSGFILTER1			0x08	// Set message filter 1
#define	SET_MSGFILTER2			0x09	// Set message filter 2
#define	IS_RCV_BUFFER_EMPTY		0x0A	// Check if receive buffer is empty
#define	IS_TX_BUFFER_EMPTY		0x0B	// Check if transmit buffer is empty
#define	IS_TX_PENDING			0x0C	// Check for pending TX
#define	GET_CANCON				0x0D
#define	GET_CIOCON				0x0E
#define	SET_SYNC_COUNT			0x10
#define	SET_SYNC_MSG			0x11
#define	TURN_SYNC_ON		    0x12
#define	TURN_SYNC_OFF			0x13
#define SET_CONFIG_MODE			0x20	// Set config mode
#define SET_LOOPBACK_MODE		0x21	// Set loopback mode
#define SET_NORMAL_MODE			0x22	// Set normal mode
#define SET_SLEEP_MODE			0x23	// Set sleep mode
#define SET_LISTEN_MODE			0x24	// Set listen mode
#define ABORT_ALL_TX			0x26	// Abort all transmissions
#define SET_BAUD_1MEG			0x30	// Set baudrate = 1Mbps
#define SET_BAUD_500K			0x31	// Set baudrate = 500Kbps
#define SET_BAUD_250K			0x32	// Set baudrate = 250 bps
#define SET_BAUD_125K			0x33	// set Baudrate = 125 bps
#define GET_BAUD_REGS			0x34	// Get Baudrate registers
#define RESET_MICRO				0x42	// Reset device micro
#define GET_HARDWARE_VERSION	0x43	// Get hardware version
#define GET_FIRMWARE_VERSION	0x44	// Get firmware version


// USB-CAN board variables
enum USB_ERRORS { 
	
	USB_OK=0,
	USB_NO_ERROR = 99,			// 0x63
	USB_EXPECTING_DLE = 100,	// 0x64
	USB_EXPECTING_STX = 101,
	USB_BAD_CHECKSUM = 102,
	USB_EXPECTING_ETX = 103,
	USB_BAD_STATE = 104,
	USB_MSG_TOO_BIG = 105,
	USB_MSG_TIMEOUT = 106,		// 0x6A
	USB_RX_BAD_CAN_MSG = 107,	// 0x6b
	USB_CAN_TX_STUCK = 108,
	CAN_RCV_OVERFLOW = 109,
	CAN_RCV_WARNING = 110,
	CAN_TX_WARNING = 111,
	CAN_RX_BUS_PASSIVE = 112,
	CAN_TX_BUS_PASSIVE = 113,
	CAN_BUS_OFF = 114,
	CAN_IN_CONFIG_MODE = 115,
	USB_TIMEOUT = 116,
	USB_DATA_TOO_LONG = 117,
	USB_CANNOT_TRANSMIT = 118,
	USB_ERROR_NO_USB = 119
};

// Emergency flags
#define EMERGENCY_OVERFLOW		0x01
#define EMERGENCY_RCV_WARNING	0x02
#define EMERGENCY_TX_WARNING	0x04
#define EMERGENCY_TXBUS_PASSIVE	0x08
#define EMERGENCY_RXBUS_PASSIVE	0x10
#define EMERGENCY_BUS_OFF		0x20


enum USB_STATE_STATES { 
		USB_IDLE=0, 
		USB_FRAME_START, 
		USB_FRAME, 
		USB_FRAME_NLE, 
		USB_MESSAGE_COMPLETE 
};

enum USB_MODE { 
	
	MODE_RUN=0, 
	MODE_DOWNLOAD_USB, 
	MODE_DOWNLOAD_CAN 
};

enum QUEUE_NUM { 
	
	UCB_QUEUE_PDO=0, 
	UCB_QUEUE_EMERGENCY, 
	UCB_QUEUE_EVENT, 
	UCB_QUEUE_SYNC 
};

enum CAN_BUAD_RATE {
	
	CAN_BAUD_1000, 
	CAN_BAUD_500, 
	CAN_BAUD_250, 
	CAN_BAUD_125
};


// Adapter runmode states
enum RUNMOD {
	
	RUNMODE_BOOT,
	RUNMODE_MAIN
};

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
  uint32_t  id;
	uint8_t	  command;
	uint8_t   len;
	uint8_t   data[256];
} responseMsg;


typedef struct {	
	bool rx_buff1_ext;
	bool rx_buff2_ext;

	uint32_t RXM0;
	uint32_t RXM1;
	uint32_t RXF0;	
	uint32_t RXF1;
	uint32_t RXF2;
	uint32_t RXF3;
	uint32_t RXF4;
	uint32_t RXF5;
} structFilterMask;

class CApoxObj
{

public:

	/// Constructor
	CApoxObj();
	
	/// Destructor
	virtual ~CApoxObj();

	
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
		Send one USB message

		@param data Pointer ot data to send
		@param dataSize Size of data
		@return Device status code
	*/
	uint8_t sendUSBMsg( uint8_t *data, short dataSize );


	/*!
		State machine for incoming datastrem

		@param inByte Byte rp process
		@param MsgLen Message length
		@param pData Pointer to data
		@return status USB_OK if OK else error code
	*/
	uint8_t processUSBByte( uint8_t inByte, uint32_t *MsgLen, uint8_t *pData );

	/*!
		Read data if available

		@return true if data was available
	*/
	bool readUSBData( void );



	/*!
		Send a command

		@param cmdcode Code for command
		@param data Pointer to data to send
		@param dataSize Size for datablock
		@return status USB_OK if OK else error code
	*/
	uint8_t sendCommand( uint8_t cmdcode, uint8_t *pData = NULL, uint16_t dataSize = 0 );


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
		Set baudrate

		@param rate Code for baudrate
					0 = 1Mbps
					1 = 500 Kbps
					2 = 250 Kbps
					3 = 125 Kbps
	*/
	bool setBaudrate( uint8_t rate );


	/*!
		Get the adapter run mode
		this function querys the USB-CAN board, to find out
		if the board is running the bootcode or the main code

		@return 1 for error, 0 for boot code, 1 for main code
	
	*/
	int getAdapterRunMode( void );


	/*!
		Set the adapter working mode

		@return true on success
	*/
	bool setAdapterRunMode( short nRunMode );

	/*!
		Reset the adapter

		@return true on success
	*/
	bool resetAdapter( void );


	/*!
		Set the adaptors transmission mode

		@return true on success
	*/
	bool setTransmissionMode( short nMode );

	/*!
		Get TX error counter

		@return count on success, -1 on error
	*/
	short getTxErrorCount( void );

	/*!
		Get RX error counter

		@return count on success, -1 on error
	*/
	short getRxErrorCount( void );

	/*!
		Requests the CANSTAT register

		@return CANSTAT on success, -1 on error
	*/
	short getAdapterCANSTAT( void );

	/*!
		Requests the COMSTAT register

		@return COMSTAT on success, -1 on error
	*/
	short getAdapterCOMSTAT( void );

	/*!
		Abort all transmissions

		@return True on success
	*/
	bool abortTransmission( void );

	/*!
		Set filters and masks

		@return true on success
	*/
	bool setAdapterFilterMask( structFilterMask *pFilterMask );

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
	HANDLE m_apoxMutex;
#else
	pthread_mutex_t m_apoxMutex;
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

		Formula is !( ( filter \EEd ) & mask )
	*/
	uint32_t m_filter;

	/*!
		Mask for outgoing messages	
	*/
	uint32_t m_mask;


	///////////////////////////////////////////////////////////////////////////
	//				 * * * * *  Apox specific * * * * *
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

	/*!
	*/
	uint8_t m_emergencyInfo;

	/*!
		Handle for USB device
	*/
	FT_HANDLE m_ftHandle;


	/*!
		Device serial number
	*/
	char m_SerialNumber[ 256 ];


	/*!
		State for receice message states
	*/
	int  m_RxMsgState;


	/*!
		General receive buffer
	*/
	uint8_t m_bufferRx[ 0x10000 ];

	/*!
		Message receive buffer
	*/
	uint8_t m_bufferMsgRcv[ 512 ];

	/*!
		Current length for received message
	*/
	uint32_t m_lengthMsgRcv;

	/*!
		Filter and mask in adapter
	*/
	structFilterMask m_filtermask;
	
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
