// socketcan.h: interface for the socketcan class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2013 Ake Hedman, 
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

#define VSCP_LEVEL2_DLL_LOGGER_OBJ_MUTEX "___VSCP__DLL_L2TCPIPLINK_OBJ_MUTEX____"

#define VSCP_SOCKETCAN_LIST_MAX_MSG		2048
  

// Forward declarations
class CWrkTread;
class VscpTcpIf;
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
    
    /// Filter
    vscpEventFilter m_vscpfilter;

    /// Pointer to worker thread
    CWrkTread *m_pthreadWork;
    
    /// VSCP local server interface
    VscpTcpIf m_srvLocal;
    
    /// VSCP remote server interface
    VscpTcpIf m_srvRemote;

};

///////////////////////////////////////////////////////////////////////////////
//				                Worker Treads
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

    /// VSCP local server interface
    VscpTcpIf m_srvLocal;
    
    /// VSCP remote server interface
    VscpTcpIf m_srvRemote;

    /// Sensor object
    CTcpipLink *m_pObj;

};


class CWriteSocketCanTread : public wxThread {
public:

    /// Constructor
    CWriteSocketCanTread();

    /// Destructor
    ~CWriteSocketCanTread();

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
    CTcpipLink *m_pObj;

};


#endif // !defined(AFX_VSCPLOG_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_)
