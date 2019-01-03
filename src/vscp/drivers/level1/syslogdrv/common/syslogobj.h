// syslogobj.h: interface for the CSysLogObj class.
//
// Copyright (C) 2000-2015
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
//////////////////////////////////////////////////////////////////////

#if !defined(SYSLOGOBJ_H__212CD7A3_785B_4444_82A6_D876E36BFCB1__INCLUDED_)
#define SYSLOGOBJ_H__212CD7A3_785B_4444_82A6_D876E36BFCB1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef __GNUG__
    //#pragma implementation
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef __WXMSW__
    #include  "wx/ownerdrw.h"
#endif

#include "wx/log.h"
#include "wx/file.h"
#include "wx/sizer.h"
#include "wx/menuitem.h"
#include "wx/checklst.h"
#include "wx/url.h"
#include "wx/html/htmlpars.h"
#include "wx/stream.h"
#include "wx/datetime.h"
#include "wx/msw/ole/automtn.h"
#include "wx/tokenzr.h"

#include <canal.h>
#include <vscp.h>
#include <canal_macro.h>
#include <dllist.h>
#include "wx/hashmap.h"
#include <wx/listimpl.cpp>



#define SYSLOG_PORT	514	// UDP port used for syslog


// Max messages in input queue
#define SYSOG_MAX_RCVMSG		1024

// Max messages in output queue
#define SYSLOG_MAX_SNDMSG		1024


#define CANAL_DLL_SYSLOG_OBJ_MUTEX		TEXT("___CANAL__DLL_SYSLOG_OBJ_MUTEX____")
#define CANAL_DLL_SYSLOG_RECEIVE_MUTEX	TEXT("___CANAL__DLL_SYSLOG_RECEIVE_MUTEX____")


class CWorkThread;
class CSysLogObj;

#define SYSLOGDRV_ISDN4L_PHONE	1
#define SYSLOGDRV_RAW			2

/*!
	The SysLog Interface
*/

class CSysLogObj
{
public:
	CSysLogObj();
	virtual ~CSysLogObj();


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
		Path to configuration file
	*/
	unsigned char m_callIdx;


	/*!
		SysLog daemon port
	*/
	int m_syslogPort;

	/*!
		Driver index
	*/
	short m_drvidx;


	/*! 
		Receive queue

		This is the receive queue for messages going in to the
		device
	*/
	DoubleLinkedList m_receiveList;


	/*!
		Init flags from open call
	*/
	uint32_t m_initFlag;


/*!
		Mutex for device.
	*/
#ifdef WIN32	
	HANDLE m_syslogMutex;
#else
	pthread_mutex_t m_syslogMutex;
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
		Bit 28 - Bus Active.
		Bit 29 - Bus passive status.
		Bit 30 - Bus Warning status.
		Bit 31 - Bus off status.
	*/
	canalStatus m_status;

	

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

private:
	

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


};


#endif // !defined(LIRCINTERFACE_H__212CD7A3_785B_4444_82A6_D876E36BFCB1__INCLUDED_)
