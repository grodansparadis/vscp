// LircInterface.h: interface for the CLircObj class.
//
// Copyright (C) 2000-2015 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
//////////////////////////////////////////////////////////////////////

#if !defined(LIRCINTERFACE_H__212CD7A3_785B_4444_82A6_D876E36BFCB1__INCLUDED_)
#define LIRCINTERFACE_H__212CD7A3_785B_4444_82A6_D876E36BFCB1__INCLUDED_

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
#include "wx/hashmap.h"
#include <wx/listimpl.cpp>

/*
	Configuration file format
	=========================

	<lirc>

	<object>

		<codetag> "codestring" </codetag>
		<can>
			<id> 11-bit or 29-bit id </id>
			<extended> 1|true </extended>
			<sizedata> ... </sizedata>
			<data> "datastring" </data>
		</can>
	
		<vscp>
			<priority> ... </priority>
			<class> ... </class>
			<type> ... </type>
			<datasize> ... </datasize>
			<data> ... </data>
		</vscp>
	</object>

	<object>
		....
	</object>

	<object>
		....
	</object>

	</lirc>

  ---

	There can be any number of CAN and VSCP blocks for an object and they will be 
	sent in the same order as they are read from the configuration file.

  ---

	Only Level I events are allowed.

  ---

    datacount is optional as it is set by the number of bytes in
	tha datafield

  ---

	Data has the form

		1,2,3,4,5

	or

		0x01,0x02,0x03,0x04

	all other numerical data can also be decimal or hexadecimal.

  ---

	tag must be the first element for all all object blocks.

  ---

*/


#define LIRC_PORT	8765	// TCP port used for Lirc daemon


// Max messages in input queue
#define LIRC_MAX_RCVMSG		1024

// Max messages in output queue
#define LIRC_MAX_SNDMSG		1024


/*!
	Event list elements	
*/

/*
class EventListElement
{

public:
	canalMsg m_msg;
};
*/

// Declare the event list
WX_DECLARE_LIST( canalMsg, EventList );

// Declare the receive queue
WX_DECLARE_LIST( canalMsg, ReceiveQueue );

// Declare the transmit queue
WX_DECLARE_LIST( canalMsg, TransmitQueue );

// Declare the Lirc code hash
WX_DECLARE_HASH_MAP( wxString, EventList*, wxStringHash, wxStringEqual, LircHash );


class CWorkThread;
class CLircObj;



/*!
	The Lirc Interface
*/

class CLircObj
{
public:
	CLircObj();
	virtual ~CLircObj();



	/*!
		Read configuration data
	*/
	void readConfigData();


	/*!
		Parse on eline from the LIRC daemon
	*/
	void parseLircLine( wxString &wxstr );


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
	char m_pathToConfigFile[ 512 ];

	/*!
		Lirc Host name
	*/
	char m_lircHost[ 512 ];

	/*!
		Lirc daemon port
	*/
	int m_lircPort;

	/*!
		Driver index
	*/
	short m_drvidx;


	/*!
		Set to true for a CanalLirc XML document
	*/
	bool m_bCanalLirc;

	/*!
		Set to true for a CanalLirc Object
	*/
	bool m_bObject;


	/*!
		Set to true for <vscp> tag
	*/
	bool m_bVscpObject;

	/*!
		Set to true for <can> tag
	*/
	bool m_bCanObject;

	/*!
		Structure depth
	*/
	int m_depth;


	/*!
		Current tag name
	*/
	char m_curtagname[ MAX_PATH  ];


	/*!
		Buffer for XML data
	*/
	char m_xmlbuf[ MAX_PATH ];

	/*!
		Holder for LIRC keytag
	*/
	char m_keytag[ MAX_PATH ];

	/*!
		Work canal message
	*/
	canalMsg m_workMsg;

	/*!
		Hash table for events
	*/
	LircHash m_lircHash;


	/*!
		Receive message queue	
	*/
	ReceiveQueue m_receiveQueue;


	/*!
		Transmit message queue
	*/
	TransmitQueue m_transmitQueue;


	/*!
		Critical section for the Hash table and
		the lists.
	*/
	wxCriticalSection m_csHash;


	/*!
		Critical section for the receiveQueue
	*/
	wxCriticalSection m_csRcvQueue;


	/*!
		Init flags from open call
	*/
	uint32_t m_initFlag;


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
