// tellstickobj.h: 
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
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
#include <wx/thread.h>

#include "../../../../common/canal.h"
#include "../../../../common/canal_macro.h"
#include "../../../../common/vscp_class.h"
#include "../../../../common/vscp_type.h"
#include "../../../../common/vscphelper.h"
#include "../../../../../common/dllist.h"


#ifdef WIN32
#include <windows.h>
#include "FTD2XX.h"		// from ftdi-site http://www.ftdichip.com/
#endif 

#define CANAL_DLL_TELLSTICKOBJ_OBJ_MUTEX		TEXT("___CANAL__DLL_TELLSTICKOBJ_OBJ_MUTEX____")
#define CANAL_DLL_TELLSTICKOBJ_RECEIVE_MUTEX	TEXT("___CANAL__DLL_TELLSTICKOBJ_RECEIVE_MUTEX____")
#define CANAL_DLL_TELLSTICKOBJ_TRANSMIT_MUTEX	TEXT("___CANAL__DLL_TELLSTICKOBJ_TRANSMIT_MUTEX____")
#define CANAL_DLL_TELLSTICKOBJ_RESPONSE_MUTEX	TEXT("___CANAL__DLL_TELLSTICKOBJ_RESPONSE_MUTEX____")


// Max messages in output queue
#define TELLSTICK_MAX_SNDMSG            512

// Tellstick protocols
#define TELLSTICK_PROTOCOL_NEXA         0
#define TELLSTICK_PROTOCOL_WAVEMAN      1
#define TELLSTICK_PROTOCOL_SARTANO      2
#define TELLSTICK_PROTOCOL_IKEA         3

#define TELLSTICK_ERROR_WRITE_DEVICE    1
#define TELLSTICK_ERROR_NEXA_COMMAND    2
#define TELLSTICK_ERROR_WAVEMAN_COMMAND 3	
#define TELLSTICK_ERROR_SARTANO_COMMAND 4
#define TELLSTICK_ERROR_IKEA_COMMAND    5


#define TELLSTICK_DRIVERINFO	"<?xml version = \"1.0\" encoding = \"UTF-8\" ?>" \
"<!-- Version 0.0.1    2009-07-04   -->" \
"<config level=\"1|2\"blcoking\"true|false\" description=\"bla bla bla bla\">" \
"   <item pos=\"0\" type=\"string\" description\"Serial number for Tellstick\"/>" \
"   <item pos=\"1\" type=\"path\" description\"Path to configuration file\"/>" \
"</config>"



// List with CANAL messages
WX_DECLARE_LIST ( canalMsg, CANALMSGLIST );

// Forward declarations
class CTellstickWrkTread;


/*!
	
*/
class CTellStickDevice
{

public:

	/// Constructor
	CTellStickDevice();
	
	/// Destructor
	virtual ~CTellStickDevice();

public:

	/// Protocol to use
	uint8_t m_protocol;

	/// IKEA System code 1-16
	uint8_t m_ikea_systemcode;

	/// IKEA Device code 1-10
	uint8_t m_ikea_devicecode;

	/*!
		Dim level 0-10 for the device. If this variable is set
		to 255 then the dimlevel of the event divided with 10 
		will be used.
		0 = off
		1 = 10%
		2 = 20%
		...
		10 = 100%
	*/
	uint8_t m_dimlevel;

	/*!
		If this variable is set to true the dim level will be
		gradually changed else if set to false the cange will be instant.
		Only for IKEA.
	*/
	bool m_bNiceDim;

	/*!
		The house code can have a value "A"-"P"
	*/
	wxString m_houseCode;

	/*!
		The channel code can have a value 1-16
	*/
	uint8_t m_channelCode;

	/*!
		Waveman channel string
	*/
	wxString m_strChannel;

	/*!
		Flag to indicate if device should be turned on (true) or
		turned off (false)
	*/
	bool m_bTurnOn;

	/*!
		This is the node id that will be used for replies from 
		this device
	*/
	uint8_t m_nodeid;
};


// List with tellstick deviceitems
WX_DECLARE_LIST ( CTellStickDevice, TELLSTICKDEVICELIST );



// This class holds each event that we should react on. Events are
// class + type + zone + subzone.
// 
class CEventTranslation
{
	public:

	/// Constructor
	CEventTranslation();
	
	/// Destructor
	virtual ~CEventTranslation();

	/*!
		Add Nexa device

		@param HouseCode 'A' - 'P'
		@param ChannelCode 1-16
		@param dimLevel 0-10
		@param bTurnOn True to turn on device, false to turn off device.
	*/
	void addNexaDevice( const char houseCode,
							uint8_t channelCode,
							uint8_t dimlevel,
							bool bTurnOn );

	/*!
		Add Waveman device

		@param HouseCode 'A' - 'P'
		@param ChannelCode 1-16
		@param dimLevel 0-10
		@param bTurnOn True to turn on device, false to turn off device.
	*/
	void addWavemanDevice( const char houseCode,
							uint8_t channelCode,
							uint8_t dimlevel,
							bool bTurnOn );

	/*!
		Add Sartano device

		@param ChannelCode 1-16
		@param bTurnOn True to turn on device, false to turn off device.
	*/
	void addSartanoDevice( uint8_t channelCode,
								bool bTurnOn );

	/*!
		Add IKEA device

		@param systemCode 1-16
		@param channelCode 1-10
		@param dimLevel 0-10
		@param bNiceDim If this variable is set to true the dim level will be
		gradually changed else if set to false the cange will be instant.
	*/
	void addIkeaDevice( uint8_t systemCode, 
							uint8_t deviceCode, 
							uint8_t dimLevel, 
							bool bNiceDim );

public:

	/// event class
	uint16_t m_vscp_class;

	/// event type
	uint16_t m_vscp_type;

	/// event zone 0-255. 255 is all zones.
	uint8_t m_zone;

	/// event subzone 0-255. 255 is all subzones.
	uint8_t m_subzone;

	/*!
		The device list holds all devices that are effected by 
		the received event.
	*/
	TELLSTICKDEVICELIST m_deviceList;
};




// List with tellstick triger eventsitems
WX_DECLARE_LIST ( CEventTranslation, TELLSTICKEVENTLIST );




class CTellstickObj
{

public:

	/// Constructor
	CTellstickObj();
	
	/// Destructor
	virtual ~CTellstickObj();

	
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


#ifdef DEBUG_TELLSTICK_RECEIVE
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
		Read a message from the device. Block 'timeout' millisceonds if 
		no msg is availabel.
		@param pcanalMsg Pointer to CAN message.
		@param timeout Time to wait for message. Set to zero to wait indefinitly.
		@return True on success.
	*/
	bool readMsgBlock( canalMsg *pMsg, unsigned long timeout );


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
		This method returns a documentation object in XML form of the 
		configuration string for the driver. This string can be used 
		to help users to enter the configuration data in an application 
		which allows for this. 

		@return XML configuration information
	*/
	char * CTellstickObj::CanalGetDriverInfo( void );


	/*!
		Parse the configuration XML file

		@param path Path to the configuration file
		@return true on success.
	*/
	bool parseConfiguration( wxString& path );

	

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
		Bit 13 - Reserved
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
		Bit 30 - Reserved
		Bit 31 - Reserved.

	*/
	canalStatus m_status;

 


	/*! 
		Event list

		This is the list that holds events that will trigger
		external devices.
	*/
	TELLSTICKEVENTLIST m_eventList;


	/*! 
		Response queue

		This is the receive queue for command response messages
		back to the controling softeare
	*/
	DoubleLinkedList m_receiveList;


	/*!
		Mutex for receive queue.
	*/
	wxMutex m_receiveMutex;

	
	/*!
		Signaled when a frame is available
	*/
	wxSemaphore m_receiveSemaphore;	

	/*!
		Mutex for the Tellstick channel.
	*/
	wxMutex m_tellstickMutex;

	
	/*!
		Protects the tx list
	*/
	wxMutex m_txMutex;

	/*!
		List with events that should be handled
		by the worker thread.
	*/
	CANALMSGLIST m_txList;


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
	//				 * * * * *  TELLSTICK specific * * * * *
	///////////////////////////////////////////////////////////////////////////


	/*!
		flags 
		-----------------------------------------------------------------------------

 
	*/
	uint32_t m_initFlag;


	/*!
		Handle for USB device
	*/
	FT_HANDLE m_ftHandle;


	/*!
		Device serial number
	*/
	wxString m_SerialNumber;

	/*!
		Reply events will be sent if set to 
		true == default
	*/
	bool m_bSendReplyEvents;


};




///////////////////////////////////////////////////////////////////////////////
//						       Worker Tread
///////////////////////////////////////////////////////////////////////////////




class CTellstickWrkTread : public wxThread 
{

public:

	/// Constructor
	CTellstickWrkTread();

	/// Destructor
	~CTellstickWrkTread();

	/*!
		Thread code entry point
	*/
	virtual void *Entry();

	/*! 
		called when the thread exits - whether it terminates normally or is
		stopped with Delete() (but not when it is Kill()ed!)
	*/
 	virtual void OnExit();

	/*!
		@param pHouseStr House Code 'A'..'P
		@param pChannelStr Channel 1..16 
		@param bTurnOn True id module should be tuned on false otherwise.
		@param bWwaveman True if Waveman dev.
		@param pTxStr The string built for the device.
		@return 0 on failure.
	*/
	int createNexaString( const char *pHouseStr, 
							uint8_t channelCode,
							bool bTurnOn, 
							bool bWaveman,
							char *pTxStr );

	/*!

	*/
	int createSartanoString( const char *pChannelStr, 
								bool bTurnOn,
								char *pTxStr );

	/*!

	*/
	int createIkeaString( int systemCode, 
							int channelCode, 
							int dimLevel, 
							bool bNiceDim, 
							char *pStrReturn );

	/*!
		Send commands to devices coupled to a trigger 
		event.
	*/
	void sendDeviceCommands( canalMsg *pMsg, 
								uint8_t replyTyp, 
								bool bDim = false,
								uint8_t dimlevel = 0 );

	/*!
		Send reply event
		
		@param pTellstickEvent Pointer to Tellstick trigger event
		@param pDevice Pointer to Tellstick device
		@param vscp_class VSCP class for VSCP replyevent
		@param vscp_type VSCP type for VSCP reply event
	*/
	void replyEvent( CEventTranslation *pEvent,
						CTellStickDevice *pDevice,
						uint16_t vscp_class,
						uint8_t vscp_type,
						uint8_t dimLevel = 0 );

	/*!
		Send error reply code
		
		@param pTellstickEvent Pointer to Tellstick trigger event
		@param pDevice Pointer to Tellstick device
		@param errorCode Error code to report back
	*/
	void replyError( CEventTranslation *pTellstickEvent,
						CTellStickDevice *pDevice,
						uint8_t errorCode );

	/*!
		Termination control
	*/
	bool m_bQuit;

	/*!
		Pointer to the control object
	*/
	CTellstickObj *m_pCtrlObject;

};


#endif // !defined(AFX_IXXATVCI_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_)
