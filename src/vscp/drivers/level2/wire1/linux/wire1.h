// socketcan.h: interface for the socketcan class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2018 Ake Hedman, 
// Grodans Paradis AB, <akhe@grodansparadis.com>
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


#if !defined(AFX_VSCPLOG_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_)
#define AFX_VSCPLOG_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_

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

#include <wx/file.h>
#include <wx/wfstream.h>

#include <canal.h>
#include <vscp.h>
#include <canal_macro.h>
#include <dllist.h>
#include <vscpremotetcpif.h>
#include <guid.h>

#include <list>
#include <string>

using namespace std;

#define VSCP_LEVEL2_DLL_WIRE1_OBJ_MUTEX "___VSCP__DLL_L2WIRE1_OBJ_MUTEX____"

#define VSCP_WIRE1_LIST_MAX_MSG		2048

// Defult seconds between events
#define DEFAULT_INTERVAL			30 
#define DEFAULT_UNIT				1		// Celsius	
#define DEFAULT_CODING				0		// Normalized integer

// Forward declarations
class CWrkTread;
class VscpRemoteTcpIf;
class wxFile;


class CWire1 {
public:

    /// Constructor
    CWire1();

    /// Destructor
    virtual ~CWire1();

    /*! 
        Open
        @return True on success.
     */
    bool open(const char *pUsername,
            const char *pPassword,
            const char *pHost,
            short port,
            const char *pPrefix,
            const char *pConfig);

    /*!
        Flush and close the log file
     */
    void close(void);

	/*!
		Add event to send queue 
	 */
	bool addEvent2SendQueue(const vscpEvent *pEvent);

public:

    /// Run flag
    bool m_bQuit;
	
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
	
	/// Number of sensors
    int m_nSensors;
        
    /// Filter
    vscpEventFilter m_vscpfilter;

    /// Pointer to worker thread
    CWrkTread *m_pthreadWork;
    
     /// VSCP server interface
    VscpRemoteTcpIf m_srv;
	
	// Queue	
	std::list<vscpEvent*> m_sendList;
	std::list<vscpEvent*> m_receiveList;
	
	/*!
        Event object to indicate that there is an event in the output queue
     */
    wxSemaphore m_semSendQueue;			
	wxSemaphore m_semReceiveQueue;		
	
	// Mutex to protect the output queue
	pthread_mutex_t m_mutexSendQueue;		
	pthread_mutex_t m_mutexReceiveQueue;

};


///////////////////////////////////////////////////////////////////////////////
//				                Worker Tread
///////////////////////////////////////////////////////////////////////////////


class CWrkTread : public wxThread {
public:

    /// Constructor
    CWrkTread();

    /// Destructor
    ~CWrkTread();

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

    /// Sensor object
    CWire1 *m_pObj;
    
    /// Data object for specific sensor
    wxString m_path;
    cguid m_guid;
    int m_interval;
	int m_index;		// Index for events that needs index
	int m_unit;			// Unit for events that need unit
	int m_coding;		// 0=normalizes/1=string/2=float
	
	FILE * m_pFile;
	char m_line1[80];
	char m_line2[80];
	unsigned int m_id[9];
	int m_temperature;
};



#endif // !defined(AFX_VSCPLOG_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_)
