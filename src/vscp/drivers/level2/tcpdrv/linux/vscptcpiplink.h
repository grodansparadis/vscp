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

#include "../../../../common/canal.h"
#include "../../../../common/vscp.h"
#include "../../../../common/canal_macro.h"
#include "../../../../../common/dllist.h"
#include "../../../../common/vscpremotetcpif.h"
#include "../../../../common/guid.h"

#include <list>
#include <string>

using namespace std;

// Seconds before trying to reconnect to a broken connection
#define VSCP_TCPIPLINK_DEFAULT_RECONNECT_TIME   30  


#define VSCP_TCPIPLINK_SYSLOG_DRIVER_ID  "VSCP tcpip-link driver:"
#define VSCP_LEVEL2_DLL_LOGGER_OBJ_MUTEX "___VSCP__DLL_L2TCPIPLINK_OBJ_MUTEX____"
#define VSCP_SOCKETCAN_LIST_MAX_MSG		2048
  

// Forward declarations
class CWrkSendTread;
class CWrkReceiveTread;
class VscpRemoteTcpIf;
class wxFile;


class CTcpipLink {
public:

    /// Constructor
    CTcpipLink();

    /// Destructor
    virtual ~CTcpipLink();

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
    
    /// server supplied host
    wxString m_hostLocal;

    /// Server supplied port
    short m_portLocal;
    
    /// Server supplied username
    wxString m_usernameLocal;

    /// Server supplied password
    wxString m_passwordLocal;
    
    /// server supplied host
    wxString m_hostRemote;

    /// Server supplied port
    int m_portRemote;
    
    /// Server supplied username
    wxString m_usernameRemote;

    /// Server supplied password
    wxString m_passwordRemote;

    /// server supplied prefix
    wxString m_prefix;
    
    /// Send channel id
    uint32_t txChannelID;
    
    /// Filter for receive
    vscpEventFilter m_rxfilter;

    /// Filter for transmitt
    vscpEventFilter m_txfilter;

    // TCP/IP link response timeout
    uint32_t m_responseTimeout;

    /// Pointer to worker thread for send
    CWrkSendTread *m_pthreadSend;
    
    /// Pointer to worker thread for receive
    CWrkReceiveTread *m_pthreadReceive;
    
    /// VSCP local server interface
    VscpRemoteTcpIf m_srvLocal;
    
    /// VSCP remote server interface
    VscpRemoteTcpIf m_srvRemote;
    
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
//                             Worker Treads
///////////////////////////////////////////////////////////////////////////////


class CWrkSendTread : public wxThread {
public:

    /// Constructor
    CWrkSendTread();

    /// Destructor
    ~CWrkSendTread();

    /*!
        Thread code entry point
     */
    virtual void *Entry();

    /*! 
        called when the thread exits - whether it terminates normally or is
        stopped with Delete() (but not when it is Kill()ed!)
     */
    virtual void OnExit();

    /// VSCP local server interface
    //VscpTcpIf m_srvLocal;
    
    /// VSCP remote server interface
    VscpRemoteTcpIf m_srvRemote;

    /// Sensor object
    CTcpipLink *m_pObj;

};

class CWrkReceiveTread : public wxThread {
public:

    /// Constructor
    CWrkReceiveTread();

    /// Destructor
    ~CWrkReceiveTread();

    /*!
        Thread code entry point
     */
    virtual void *Entry();

    /*! 
        called when the thread exits - whether it terminates normally or is
        stopped with Delete() (but not when it is Kill()ed!)
     */
    virtual void OnExit();
  
    /// VSCP remote server interface
    VscpRemoteTcpIf m_srvRemote;

    /// Sensor object
    CTcpipLink *m_pObj;

};



#endif // !defined(AFX_VSCPLOG_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_)
