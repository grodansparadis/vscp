///////////////////////////////////////////////////////////////////////////////
// rawethernet.h:
//
// This file is part is part of VSCP & Friends
// http://www.vscp.org)
//
// Copyright (C) 2000-2014 Ake Hedman, Grodans Paradis AB,
// <akhe@grodansparadis.com>
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

#ifndef vscpbtdetect_h
#define vscpbtdetect_h

#ifdef WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef WIN32

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#else

#define _POSIX
#include <unistd.h>
#include <pthread.h>
#include <syslog.h>

#endif

#include "../../../../common/canal.h"
#include "../../../../common/vscp.h"
#include "../../../../common/canal_macro.h"
#include "../../../../../common/dllist.h"
#include "../../../../common/vscpremotetcpif.h"

#include <list>
#include <string>

using namespace std;

// Forward declarations
class CRawEthernetTxTread;
class CRawEthernetRxTread;
class VscpTcpIf;



class CRawEthernet  
{

public:

	/// Constructor
	CRawEthernet();
	
	/// Destructor
	virtual ~CRawEthernet();

    	/*! 
		Open the Bluetooth detecter

		@param Configuration string
		@param flags 	Not used
		@return True on success.
	*/
	bool open( const char *pUsername,
                const char *pPassword,
                const char *pHost,
                short port,
                const char *pPrefix,
                const char *pConfig, 
                unsigned long flags = 0 );

	/*!
		Flush and close the log file
	*/
	void close( void );

	/*!
		Add event to send queue 
	*/
	bool addEvent2SendQueue(const vscpEvent *pEvent);

public:

	/// Run flag
	bool m_bQuit;

    /// Driver flags
    unsigned long m_flags;

    /// Server supplied username
    wxString m_username;

    /// Server supplied password
    wxString m_password;

    /// server supplied prefix
    wxString m_prefix;

    /// server supplied host
    wxString m_host;

    /// Server supplied port
    short m_port;

    /*! 
        Ethernet interface to use
        The iflist program supplied with wpcap can be used to list 
        all interfaces.
    */
    wxString m_interface;

    /*!
        Local MAC address to use
    */
    uint8_t m_localMac[ 6 ];

    /// Daemon channel id for rawEthernet tx channel
    uint32_t m_ChannelIDtx;

    /// Local GUID for transmit channel
    cguid m_localGUIDtx;

    /// Local GUID for receive channel
    cguid m_localGUIDrx;

    /// Pointer to worker threads
    CRawEthernetTxTread *m_pthreadWorkTx;
	CRawEthernetRxTread *m_pthreadWorkRx;

	// Queue
	std::list<vscpEvent *> m_sendList;
	std::list<vscpEvent *> m_receiveList;
	
	/*!
        Event object to indicate that there is an event in the output queue
     */
    wxSemaphore m_semSendQueue;			
	wxSemaphore m_semReceiveQueue;		
	
	// Mutex to protect the output queue
	wxMutex m_mutexSendQueue;		
	wxMutex m_mutexReceiveQueue;

};



///////////////////////////////////////////////////////////////////////////////
//				Worker Treads
///////////////////////////////////////////////////////////////////////////////


/*!
    This is the thread that transmit data on the VSCP daemon 
    interface that is received 
    from the Ethernet
*/
class CRawEthernetTxTread : public wxThread 
{

public:

	/// Constructor
	CRawEthernetTxTread();

	/// Destructor
	~CRawEthernetTxTread();

	/*!
		Thread code entry point
	*/
	virtual void *Entry();

	/*! 
		called when the thread exits - whether it terminates normally or is
		stopped with Delete() (but not when it is Kill()ed!)
	*/
 	virtual void OnExit();

    /// VSCP server interface
    VscpRemoteTcpIf m_srv;

    /// Pointer back to owner
    CRawEthernet *m_pobj;

};

/*!
    This is the thread that receive data from the VSCP daemon interface and transmit 
    on Ethernet
*/
class CRawEthernetRxTread : public wxThread 
{

public:

	/// Constructor
	CRawEthernetRxTread();

	/// Destructor
	~CRawEthernetRxTread();

	/*!
		Thread code entry point
	*/
	virtual void *Entry();

	/*! 
		called when the thread exits - whether it terminates normally or is
		stopped with Delete() (but not when it is Kill()ed!)
	*/
 	virtual void OnExit();

    /// VSCP server interface
    VscpRemoteTcpIf m_srv;

    /// Pointer back to owner
    CRawEthernet *m_pobj;

};


#endif
