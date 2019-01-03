///////////////////////////////////////////////////////////////////////////////
// CAN232Obj.h: interface for the CCAN232Obj class. 
//
// This file is part is part of CANAL (CAN Abstraction Layer)
// http://www.vscp.org)
//
// Copyright (C) 2000-2014 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
///////////////////////////////////////////////////////////////////////////////

#if !defined(CAN232DRV_H__16828641_5EDF_4115_9522_97BD178F566B__INCLUDED_)
#define CAN232DRV_H__16828641_5EDF_4115_9522_97BD178F566B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../../../../common/com_win32.h"
#include "../../../../common/dllist.h"
#include "../../../common/canal.h"


#define CAN232_BUF_SIZE		80	// Size for one message buffer

#define CAN232_STATE_NONE	0	// 
#define CAN232_STATE_MSG	1	// Message is beeing read

#define CAN232_MAX_RCVMSG	512	// Maximum number of received messages
#define CAN232_MAX_SNDMSG	512	// Maximum number of received messages

#define CAN232_OBJ_MUTEX	"___CAN232_OBJ_MUTEX___"

////////////////////////////////////////////////////////////////////////////////////////
// _can232obj
//

struct _can232obj {

	/*!
		Work flag for thread
	*/
	bool m_bRun;
	
	/*!
		The RS232 communication channel
	*/
	CComm m_comm;
	
	/*!
		Input buffer
	*/
	char m_receiveBuf[ CAN232_BUF_SIZE ];
	
	/*!
		Receive message count
	*/
	unsigned short m_cntRcv;
	
	/*!
		State machine state
	*/
	short m_state;

	/*!
		Version
	*/
	short m_version;

	/*!
		Flag for polling mode (default)
	*/
	bool m_bAuto;

	/*! 
		Send/Receive Statistics
	*/
	canalStatistics m_stat;

	DoubleLinkedList m_rcvList;	// List for incoming messages
	DoubleLinkedList m_sndList;	// List for outgoing messages
};



class CCAN232Obj  
{

public:
	CCAN232Obj();
	virtual ~CCAN232Obj();

	/*!
		Open the device
	*/
	long open( const char *pDevice, unsigned long flags );

	/*!
		Close the device
	*/
	int close( void );

	/*!
		Send frame
	*/
	int writeMsg( bool bExtended, 
					unsigned long id, 
					unsigned char dlc, 
					unsigned char * pdata );

	/*!
		Send frame
	*/
	int writeMsg( PCANALMSG pCanalMsg );


	/*!
		Receive frame
	*/
	int  readMsg( canalMsg *pMsg );


	/*!
		Set filter(code) and mask
	*/
	bool setFilter( unsigned long filter, unsigned long mask );

	/*!
		Set filter (acceptance code)
	*/
	bool setFilter( unsigned long filter );


	/*!
		Set mask
	*/
	bool setMask( unsigned long mask );

	/*!
		Get statistics
	*/
	bool getStatistics( PCANALSTATISTICS pCanalStatistics );


	/*
		Get number of frames availabel in the input queue
	*/
	int dataAvailable( void );

	/*!
		Get the status code

		
	*/
	bool getStatus( PCANALSTATUS pCanalStatus );


	struct _can232obj m_can232obj;


	/*!
		The can232 object MUTEX
	*/
	HANDLE m_can232ObjMutex;


	/*!
		id for worker thread
	*/
	DWORD m_dwThreadId;
	 
};

#endif // !defined(CAN232DRV_H__16828641_5EDF_4115_9522_97BD178F566B__INCLUDED_)
