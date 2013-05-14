// usb2canobj.h: 
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2013 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
// Copyright (C) 2009 Gediminas Simanskis , edevices, www.edevices.lt
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
// $RCSfile: usb2canobj.h,v $                                       
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

#include "../../../../../common/canal.h"
#include "../../../../../common/vscp.h"
#include "../../../../../common/canal_macro.h"
#include "../../../../../../common/dllist.h"

// Usb2can Includes
#ifdef WIN32
#include <stdio.h>
#include <windows.h>
#include "objbase.h"
#include <setupapi.h>
#include <devguid.h>
#include <initguid.h>
#include <regstr.h>
#include "enum.h"

#endif 


//DEFINE_GUID(GUID_CLASS_USB2CAN_STR75,0x873fdf, 0x61a8, 0x11d1, 0xaa, 0x5e, 0x0, 0xc0, 0x4f, 0xb1, 0x72, 0x8b);
DEFINE_GUID(GUID_CLASS_USB2CAN,0x3ea03b52, 0xc71a, 0x433c, 0xa6, 0x26, 0xe8, 0xad, 0x96, 0xf2, 0x57, 0x8b);

//#define DEBUG_USB2CAN_RECEIVE
#define MAX_LENGTH 256


/////////////////////////////////////////////////////////////////////////////////////


#define CANAL_DLL_USB2CANDRV_OBJ_MUTEX				"___CANAL__DLL_USB2CANDRV_OBJ_MUTEX____"
#define CANAL_DLL_USB2CANDRV_RECEIVE_MUTEX			"___CANAL__DLL_USB2CANDRV_RECEIVE_MUTEX____"
#define CANAL_DLL_USB2CANDRV_TRANSMIT_MUTEX			"___CANAL__DLL_USB2CANDRV_TRANSMIT_MUTEX____"
#define CANAL_DLL_USB2CANDRV_COMMAND_MUTEX			"___CANAL__DLL_USB2CANDRV_COMMAND_MUTEX____"
#define CANAL_DLL_USB2CANDRV_RESPONSE_DATA_MUTEX    "___CANAL__DLL_USB2CANDRV_RESPONSE_DATA_MUTEX____"

// Max messages in input queue
//#define USB2CAN_MAX_RCVMSG		64

// Max messages in output queue
//#define USB2CAN_MAX_SNDMSG		64

#define USB2CAN_MAX_FIFO	    	1024

// Max number of response messages in response queue
#define USB2CAN_MAX_RESPONSEMSG	 5

#define USB2CAN_MAX_RCVMSG      30

// Byte stuffing start and end characters
#define USB_DLE					0x10
#define USB_STX					0x02
#define USB_ETX					0x03

// USB Function status
#define USB_ERR_NO_USB			101
#define USB_ERR_FT				102
#define USB_ERR_NO_HEXFILE		103

//USB2CAN Frame types
#define DATA_FRAME          		    0	
#define DATA_FRAME_ACK                  1
#define DATA_FRAME_NACK                 2
#define DATA_FRAME_ERROR                3

#define EMERGENCY_OVERFLOW			    0x80
#define EMERGENCY_RCV_WARNING		    0x81
#define EMERGENCY_TX_WARNING	        0x82
#define EMERGENCY_TXBUS_PASSIVE		    0x83
#define EMERGENCY_RXBUS_PASSIVE		    0x84
#define EMERGENCY_BUS_OFF			    0x85


///////////////////////////////////////  GS  /////////////////////////////////////////////
// USB2CAN COMMANDS


#define USB2CAN_RESET                1
#define USB2CAN_OPEN                 2 
#define USB2CAN_CLOSE                3
#define USB2CAN_SET_SPEED            4
#define USB2CAN_SET_MASK_FILETR      5
#define USB2CAN_GET_STATUS           6
#define USB2CAN_GET_STATISTICS       7
#define USB2CAN_GET_SERIAL           8
#define USB2CAN_GET_SOFTW_VER        9
#define USB2CAN_GET_HARDW_VER       10
#define USB2CAN_RESET_TIMESTAMP     11
#define USB2CAN_GET_SOFTW_HARDW_VER 12

#define USB2CAN_CMD_SUCCESS         0
#define USB2CAN_CMD_ERROR         255 


enum CAN_BAUD_RATE
{	
	CAN_BAUD_1000 = 0,
    CAN_BAUD_800,		
	CAN_BAUD_500, 
	CAN_BAUD_250, 
	CAN_BAUD_125,
	CAN_BAUD_100,
	CAN_BAUD_50,
	CAN_BAUD_20,
	CAN_BAUD_10,
    CAN_BAUD_MANUAL  
};


enum CAN_MODES {

    CAN_MODE_NORMAL = 0,
    CAN_MODE_SILENT,
    CAN_MODE_LOOPBACK,
    CAN_MODE_SILENT_LOOPBACK
};


//////////////////////////////////////////////////////////////////////////////////////


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
//#define EMERGENCY_OVERFLOW		0x01
//#define EMERGENCY_RCV_WARNING	0x02
//#define EMERGENCY_TX_WARNING	0x04
//#define EMERGENCY_TXBUS_PASSIVE	0x08
//#define EMERGENCY_RXBUS_PASSIVE	0x10
//#define EMERGENCY_BUS_OFF		0x20


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

typedef struct {
    //uint32_t  id;
	UCHAR	  channel;
	UCHAR	  command;
	UCHAR     opt1;
	UCHAR	  opt2;
	UCHAR     data[20];
}cmdMsg;


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

class CUsb2canObj
{

public:

	/// Constructor
	CUsb2canObj();
	
	/// Destructor
	virtual ~CUsb2canObj();


#ifdef DEBUG_USB2CAN_RECEIVE
	/// The log file handle
	FILE *m_flog;
#endif
	
//////////////////////////////////////////
//    USB2CAN hardware

     bool USB2CAN_open( UCHAR speed,UCHAR tseg1,UCHAR tseg2,UCHAR sjw,USHORT brp, ULONG flags );
     bool USB2CAN_close(void);
	 bool USB2CAN_reset(void);
	 bool USB2CAN_status(canalStatus *pCanalStatus);
	 bool USB2CAN_statistics( UCHAR num, DWORD *ret );
	 bool USB2CAN_get_fver_hver(DWORD *res);

//////////////////////////////////////////


	 char *getVendorString(void);

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
	int getStatistics( PCANALSTATISTICS pCanalStatistics );

	/*!
		Write a message out to the file
		@param pcanalMsg Pointer to CAN message
		@return True on success.
	*/
	int writeMsg( canalMsg *pMsg );


	/*
	   Send blocking
	*/
	int writeMsgBlocking( canalMsg *pMsg, ULONG Timeout );

	
	/*!
		Read a message fro the device
		@param pcanalMsg Pointer to CAN message
		@return True on success.
	*/
	int readMsgBlocking( canalMsg *pMsg, ULONG	 Timeout );

    /*
        Non blocking
	*/
    int readMsg( canalMsg *pMsg );

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
	int getStatus( PCANALSTATUS pCanalStatus );


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
	uint8_t sendCommand( cmdMsg *Msg );


	/*!
		Wait for a response message

		@param pMsg Pointer to response message
		@param timeout Timeout in milliseconds
		@return True on success

	*/
	bool waitCmdResponse( cmdMsg *pMsg, uint32_t timeout );


	/*!
		Wait for a Data response message

		@param pMsg Pointer to response message
		@param timeout Timeout in milliseconds
		@return True on success

	*/
	bool waitDataResponse( cmdMsg *pMsg, uint32_t timeout );


	/*!
		Send command and wait for a response message
	
		@param cmdcode Code for command
		@param pMsg Pointer to response message
		@param timeout Timeout in milliseconds
		@return True on success

	*/
	bool sendCommandWait( cmdMsg *InMsg, cmdMsg *OutMsg, uint32_t timeout );


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


/////////////////////////////////////////////////////////////////////

	CEnum	m_enum;

	HANDLE  hDataOut; 
	HANDLE  hDataIn; 
	HANDLE  hCmdOut; 
	HANDLE  hCmdIn;
	HANDLE  hDev;

    HANDLE  m_responseDataEvent;

#ifdef DEBUG_USB2CAN_RECEIVE
	char dbgbuf[100];
#endif

public:

	/// Run flag
	bool m_bRun;

	// Open flag
	bool m_bOpen;

	/*!
		Interface statistics
	*/
	canalStatistics m_stat;


	char m_USB2CAN_fver_hver[256];

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
		Tread id ReceiveCmd
	*/
#ifdef WIN32
	HANDLE m_hTreadReceiveCmd;
#else
	pthread_t m_threadIdReceiveCmd;
#endif

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
	HANDLE m_usb2canMutex;
#else
	pthread_mutex_t m_usb2canMutex;
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
	HANDLE m_commandMutex;
#else
	pthread_mutex_t m_commandMutex;
#endif
	
    HANDLE      m_receiveDataEvent;   // GS
    HANDLE      m_transmitDataPutEvent;   // GS
    HANDLE      m_transmitDataGetEvent;   // GS

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
	//				 * * * * *  Usb2can specific * * * * *
	///////////////////////////////////////////////////////////////////////////


	char		m_serial[256];
	USHORT  	m_speed;
	UCHAR	    m_tseg1;
	UCHAR	    m_tseg2;
	UCHAR	    m_sjw;
    USHORT  	m_brp;
	ULONG       m_hfilter;
	ULONG       m_hmask;


	uint32_t m_initFlag;

	/*!
	*/
	uint8_t m_emergencyInfo;

	/*!
		Handle for USB device
	*/
//	ST75_HANDLE m_ftHandle;


	/*!
		Device serial number
	*/
	char m_SerialNumber[ 256 ];


	/*!
		State for receice message states
	*/
	//int  m_RxMsgState;


	/*!
		General receive buffer
	*/
	//uint8_t m_bufferRx[ 0x10000 ];

	/*!
		Message receive buffer
	*/
	//uint8_t m_bufferMsgRcv[ 512 ];

	/*!
		Current length for received message
	*/
	//uint32_t m_lengthMsgRcv;

	/*!
		Filter and mask in adapter
	*/
	structFilterMask m_filtermask;
	
	/*!
		Transmit queue handle
	*/
	//uint16_t m_hTxQue;
	
	/*!
		Receive queue handle
	*/
	//uint16_t m_hRxQue;	
 
};

#endif // !defined(AFX_IXXATVCI_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_)
