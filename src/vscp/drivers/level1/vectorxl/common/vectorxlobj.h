// vectorobj.h: 
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

#if !defined(VECTOROBJ_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_)
#define VECTOROBJ_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_

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

// Vector Includes
#include <vcand.h> 

#define CANAL_DLL_VECTOR_OBJ_MUTEX		"___CANAL__DLL_VECTOR_OBJ_MUTEX____"
#define CANAL_DLL_VECTOR_RECEIVE_MUTEX	"___CANAL__DLL_VECTOR_RECEIVE_MUTEX____"
#define CANAL_DLL_VECTOR_TRANSMIT_MUTEX "___CANAL__DLL_VECTOR_TRANSMIT_MUTEX____"

// Max messages in input queue
#define VECTOR_MAX_RCVMSG		512

// Max messages in output queue
#define VECTOR_MAX_SNDMSG		512

class CVectorObj
{

public:

	/// Constructor
	CVectorObj();
	
	/// Destructor
	virtual ~CVectorObj();

	
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
		Close the interface
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
		Bit 16 - Reserved.
		Bit 17 - Reserved.
		Bit 18 - All sent CAN objects appear as received CAN objects.
		Bit 19 - Passive mode
		Bit 20 - Reserved. 
		Bit 21 - Reserved.
		Bit 22 - Reserved.
		Bit 23 - Reserved.
		Bit 24 - Reserved.
		Bit 25 - Reserved.
		Bit 26 - Reserved.
		Bit 27 - Reserved.
		Bit 28 - Bus Active.
		Bit 29 - Bus passive status.
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
	HANDLE m_vectorMutex;
#else
	pthread_mutex_t m_vectorMutex;
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

		Formula is !( ( filter �d ) & mask )
	*/
	uint32_t m_filter;

	/*!
		Mask for outgoing messages	
	*/
	uint32_t m_mask;


	///////////////////////////////////////////////////////////////////////////
	//				         * * * * *  VECTOR specific * * * * *
	///////////////////////////////////////////////////////////////////////////


	/*!
		flags 
		-----------------------------------------------------------------------------

		bit 0
		=====
		Reserved

		bit 2
		=====
		Reserved 

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
		Board type
	*/
	short m_nBrdType;

	/*!
		Index of boards of same type
	*/
	short m_idxBoard;

	/*!
		Handle to Vector adapter port
	*/
	VportHandle m_portHandle;

	/*!
		CAN channel on board
	*/
	uint8_t m_channel;	
	
	/*!
		Mask for active channel
	*/
	Vaccess m_channelMask;
	
	/*!
		Vector filter for outgoing messages
	*/
	uint32_t m_Vector_filter;

	/*!
		Vector mask for outgoing messages	
	*/
	uint32_t m_Vector_mask;
	
	/*!
		Transmit queue handle
	*/
	uint16_t m_hTxQue;
	
	/*!
		Receive queue handle
	*/
	uint16_t m_hRxQue;	

	/*!
		Driver index
	*/
	short m_drvidx;
 
};

#endif // !defined(VECTOROBJ_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_)
