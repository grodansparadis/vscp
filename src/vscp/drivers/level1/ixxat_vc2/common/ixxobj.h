// ixxatvci.h: 
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
// $RCSfile: ixxobj.h,v $                                       
// $Date: 2005/01/05 12:16:15 $                                  
// $Author: akhe $                                              
// $Revision: 1.6 $ 

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

// IXXAT Includes
#include <XatXXReg.h>
#include <VCI2.h>

#define CANAL_DLL_IXXATVCIDRV_OBJ_MUTEX	"___CANAL__DLL_IXXATVCIDRV_OBJ_MUTEX____"
#define CANAL_DLL_IXXATVCIDRV_RECEIVE_MUTEX	"___CANAL__DLL_IXXATVCIDRV_RECEIVE_MUTEX____"
#define CANAL_DLL_IXXATVCIDRV_TRANSMIT_MUTEX "___CANAL__DLL_IXXATVCIDRV_TRANSMIT_MUTEX____"

// Max messages in input queue
#define IXXATVCI_MAX_RCVMSG		512

// Max messages in output queue
#define IXXATVCI_MAX_SNDMSG		512

class CIxxObj
{

public:

	/// Constructor
	CIxxObj();
	
	/// Destructor
	virtual ~CIxxObj();

	
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
	int CIxxObj::dataAvailable( void );

	/*!
		Get device status
		@param pCanalStatus Pointer to CANAL status structure
		@return True on success.
	*/
	bool getStatus( PCANALSTATUS pCanalStatus );

public:

	/// Run flag
	bool m_bRun;


	/*!
		Interface statistics
	*/
	canalStatistics m_stat;


	/*!
		Interface status

		Bit 0  - not used.
		Bit 1  - not used.
		Bit 2  - RemoteQueOverrun.
		Bit 3  - CAN-TX pending.
		Bit 4  - CAN-Init-Mode.
		Bit 5  - CAN-Data-Overrun.
		Bit 6  - CAN-Error-Warning-Level.
		Bit 7  - CAN_Bus_Off_status.
		Bit 8  - Reserved.
		Bit 9  - Reserved.
		Bit 10 - Reserved.
		Bit 11 - Reserved.
		Bit 12 - Reserved.
		Bit 13 - Reserved.
		Bit 14 - Reserved.
		Bit 15 - Reserved.
		Bit 16 - 29-bit id.
		Bit 17 - Low speed-busconnector.
		Bit 18 - All sent CAN objects appear as received CAN objects.
		Bit 19 - Passive mode
		Bit 20 - Errorframes are detected and reported.
		Bit 21 - Reserved.
		Bit 22 - Reserved.
		Bit 23 - Reserved.
		Bit 24 - Reserved.
		Bit 25 - Reserved.
		Bit 26 - Reserved.
		Bit 27 - Reserved.
		Bit 28 - Reserved.
		Bit 29 - Reserved.
		Bit 30 - Bus Warning status (Same as bit 6).
		Bit 31 - Bus off status (Same as bit-7).
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
	HANDLE m_ixxMutex;
#else
	pthread_mutex_t m_ixxMutex;
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

		Formula is !( ( filter îd ) & mask )
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
		Board key
	*/
	DWORD m_dwBrdKey;

	/*!
		Boardtype code
	*/
	DWORD m_dwBrdType;				

	/*!
		Index of board in IXXAT registry
	*/
	DWORD m_dwBrdTypeRelatedIndex;	

	/*!
		Board data
	*/
	XAT_BoardCFG m_sConfig;			

	/*!
		CAN channel on board
	*/
	int m_channel;			
	
	/*!
		IXXAT filter for outgoing messages
	*/
	uint32_t m_IXXAT_filter;

	/*!
		IXXAT mask for outgoing messages	
	*/
	uint32_t m_IXXAT_mask;

	/*!
		Board handle
	*/
	uint16_t m_hBoard;
	
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
