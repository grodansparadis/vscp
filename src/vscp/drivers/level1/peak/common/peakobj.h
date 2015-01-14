// peakobj.h: 
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2015 
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
//  

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

#include <canal.h>
#include <vscp.h>
#include <canal_macro.h>
#include <dllist.h>

// PEAK Defines

#define PCAN_MSGTYPE_STATUS   0x80   // Status message
#define PCAN_MSGTYPE_EXTENDED 0x02   // Extended ID
#define PCAN_MSGTYPE_RTR      0x01   // RTR


// PEAK Message
typedef struct {
    uint32_t id;        // id
    uint8_t  msgType;   // message type
    uint8_t  len;       // # databytes
    uint8_t  data[8];   
} PeakCanMsg;


// Error codes
#define PEAK_CAN_ERR_OK				0x0000
#define PEAK_CAN_ERR_XMTFULL		0x0001  // Transmittbuffer full
#define PEAK_CAN_ERR_OVERRUN		0x0002  // CAN-Controller Overun
#define PEAK_CAN_ERR_BUSLIGHT		0x0004  // Bus errocount low
#define PEAK_CAN_ERR_BUSHEAVY		0x0008  // Bus error count high
#define PEAK_CAN_ERR_BUSOFF			0x0010  // Bus-Off
#define PEAK_CAN_ERR_QRCVEMPTY		0x0020  // Receive queue empty
#define PEAK_CAN_ERR_QOVERRUN		0x0040  // Receive que overrun
#define PEAK_CAN_ERR_QXMTFULL		0x0080  // Transmt queue full
#define PEAK_CAN_ERR_REGTEST		0x0100  // Register Test error for 82C200/SJA100
#define PEAK_CAN_ERR_NOVXD			0x0200  // VxD problem
#define PEAK_CAN_ERR_RESOURCE		0x2000  // Resource (FIFO, Client, Timeout) error
#define PEAK_CAN_ERR_ILLPARAMTYPE	0x4000  // Invalid parameter type.
#define PEAK_CAN_ERR_ILLPARAMVAL	0x8000  // Invalid parameter value
#define PEAK_CAN_ERRMASK_ILLHANDLE  0x1C00  // Mask for all handle errors

#define PEAK_CAN_ERR_ANYBUSERR ( PEAK_CAN_ERR_BUSLIGHT | PEAK_CAN_ERR_BUSHEAVY | PEAK_CAN_ERR_BUSOFF )

// Dll functions 
typedef DWORD (__stdcall *LPFNDLL_INITPNP )(WORD wBTR0BTR1, int CANMsgType);
typedef DWORD (__stdcall *LPFNDLL_INIT )(WORD wBTR0BTR1, int CANMsgType, int CANHwType, DWORD IO_Port, WORD Interupt);
typedef DWORD (__stdcall *LPFNDLL_CLOSE )();
typedef DWORD (__stdcall *LPFNDLL_STATUS )();
typedef DWORD (__stdcall *LPFNDLL_WRITE )( PeakCanMsg* pMsgBuff );
typedef DWORD (__stdcall *LPFNDLL_READ )( PeakCanMsg* pMsgBuff );
typedef DWORD (__stdcall *LPFNDLL_VERSIONINFO )( LPSTR lpversioninfo );

#define CANAL_DLL_PEAKDRV_OBJ_MUTEX			TEXT("___CANAL__DLL_PEAKDRV_OBJ_MUTEX____")
#define CANAL_DLL_PEAKDRV_RECEIVE_MUTEX		TEXT("___CANAL__DLL_PEAKDRV_RECEIVE_MUTEX____")
#define CANAL_DLL_PEAKDRV_TRANSMIT_MUTEX	TEXT("___CANAL__DLL_PEAKDRV_TRANSMIT_MUTEX____")



// Max messages in input queue
#define PEAKDRV_MAX_RCVMSG		512

// Max messages in output queue
#define PEAKDRV_MAX_SNDMSG		512




class CPeakObj
{

public:

	/// Constructor
	CPeakObj();
	
	/// Destructor
	virtual ~CPeakObj();

	/*!
		Set error code
		@param errorcode Standard CANAl error code
		@param suberrorcode Sub device error code
		@param perrorstr Clear text error description string

	*/
	void setLastError( unsigned long errorcode = CANAL_ERROR_SUCCESS, 
						unsigned long suberrorcode = 0, 
						char  *perrorstr = NULL );
	
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
		Initialize driver dll (load it)

		@param path dll name/path
		@param bPnP True if hardware is plug and play compatible
		@return true on success
	*/
	bool initialize( char *path, bool bPnp = true );

public:

	/// CANAL Standard error code
	unsigned long m_lastErrorCode;	
	
	/// Sub device error code
	unsigned long m_lastSubErrorCode;
	
	/// Descriptive error string
	char m_lastErrorStr[ 80 ];	

	/// Run flag
	bool m_bRun;


	/// Flag for library initialization
	bool m_bInit;


	/// Flag for driver open
	bool m_bOpen;

	/*!
		Interface statistics
	*/
	canalStatistics m_stat;


	/*!
		Interface status

		Bit 0  - PCAN Adapter specific.
		Bit 1  - PCAN Adapter specific.
		Bit 2  - PCAN Adapter specific.
		Bit 3  - PCAN Adapter specific.
		Bit 4  - PCAN Adapter specific.
		Bit 5  - PCAN Adapter specific.
		Bit 6  - PCAN Adapter specific.
		Bit 7  - PCAN Adapter specific.
		Bit 8  - PCAN Adapter specific.
		Bit 9  - PCAN Adapter specific.
		Bit 10 - PCAN Adapter specific.
		Bit 11 - PCAN Adapter specific..
		Bit 12 - PCAN Adapter specific.
		Bit 13 - PCAN Adapter specific.
		Bit 14 - PCAN Adapter specific.
		Bit 15 - PCAN Adapter specific.
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
		Bit 29 - Bus errors detected.
		Bit 30 - Bus Warning status.
		Bit 31 - Bus off status.
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
	HANDLE m_peakMutex;
#else
	pthread_mutex_t m_peakMutex;
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
	//				 * * * * *  IXXAT VCI specific * * * * *
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
		Boardtype code
	*/
	DWORD m_dwBrdType;				


	/*!
		DLL name
	*/
	char m_dllName[ 64 ];

	/*!
		PnP flag
	*/
	bool m_bPnp;

	/*!
		channel for multichannel cards/adapters
	*/
	int m_channel;
	
	
	/*!
		PEAK filter for outgoing messages
	*/
	uint32_t m_Peak_filter;

	/*!
		PEAK mask for outgoing messages	
	*/
	uint32_t m_Peak_mask;

 	
	/*!
		Transmit queue handle
	*/
	uint16_t m_hTxQue;
	
	/*!
		Receive queue handle
	*/
	uint16_t m_hRxQue;	

	/// Driver DLL instance
	HINSTANCE m_hinst;

	/*!
		Init device (PNP version)
	*/
	LPFNDLL_INITPNP m_procInitPnp;

	/*!
		Init device (nonPNP version)
	*/
	LPFNDLL_INIT m_procInit;

	/*!
		Close Device
	*/
	LPFNDLL_CLOSE m_procClose;

	/*!
		Get device status
	*/
	LPFNDLL_STATUS m_procStatus;

	/*!
		Write a message
	*/
	LPFNDLL_WRITE  m_procWrite;

	/*!
		Read message
	*/
	LPFNDLL_READ m_procRead;

	/*!
		Get version info.
	*/
	LPFNDLL_VERSIONINFO m_procVersionInfo;
 
};

#endif // !defined(AFX_IXXATVCI_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_)
