// rawethernet.h: interface for the rawethernet class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2014 Ake Hedman, 
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

#include "../../../../common/canal.h"
#include "../../../../common/vscp.h"
#include "../../../../common/canal_macro.h"
#include "../../../../../common/dllist.h"
#include "../../../../common/vscptcpif.h"
#include "../../../../common/guid.h"

#include <list>
#include <string>

using namespace std;

#define VSCP_LEVEL2_DLL_RAWETHERNET_OBJ_MUTEX "___VSCP__DLL_L2RAWETHERNET_OBJ_MUTEX____"

#define VSCP_RAWETHERNET_LIST_MAX_MSG		2048
  

// Forward declarations
class CWrkReadThread;
class CWrkWriteThread;
class VscpTcpIf;
class wxFile;


class CRawEthernet {
public:

    /// Constructor
    CRawEthernet();

    /// Destructor
    virtual ~CRawEthernet();

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
    
    /// Ethernet interface to use
    wxString m_interface;
    
    /// Source MAC address
    wxString m_strlocalMac;
    uint8_t m_localMac[16];
    
    /// Filter
    vscpEventFilter m_vscpfilter;
    
    /// Daemon channel id for rawEthernet tx channel
    uint32_t m_ChannelIDtx;
    
    // GUID's
    cguid m_localGUIDtx;
    cguid m_localGUIDrx;

    /// Pointer to worker threads
    CWrkReadThread *m_preadWorkThread;
    CWrkWriteThread *m_pwriteWorkThread;
    
     /// VSCP server interface
    VscpTcpIf m_srv;
		
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
//				                Worker Treads
///////////////////////////////////////////////////////////////////////////////


class CWrkReadThread : public wxThread {
public:

    /// Constructor
    CWrkReadThread();

    /// Destructor
    ~CWrkReadThread();

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
    VscpTcpIf m_srv;

    /// Sensor object
    CRawEthernet *m_pObj;

};


class CWrkWriteThread : public wxThread {
public:

    /// Constructor
    CWrkWriteThread();

    /// Destructor
    ~CWrkWriteThread();

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
    VscpTcpIf m_srv;

    /// Sensor object
    CRawEthernet *m_pObj;

};




#endif // !defined(AFX_VSCPLOG_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_)
