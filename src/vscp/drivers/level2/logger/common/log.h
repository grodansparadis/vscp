// Log.h: interface for the CVSCPLog class.
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

#define VSCP_LEVEL2_DLL_LOGGER_OBJ_MUTEX "___VSCP__DLL_L2LOGGER_OBJ_MUTEX____"

#define VSCP_LOG_LIST_MAX_MSG		2048

// Flags
#define LOG_FILE_OVERWRITE      1L  // Overwrite
#define LOG_FILE_VSCP_WORKS     2L  // VSP Works format

// List with VSCP events
WX_DECLARE_LIST(vscpEvent, VSCPEVENTLIST);

// Forward declarations
class CVSCPLogWrkTread;
class VscpTcpIf;
class wxFile;

class CVSCPLog {
public:

    /// Constructor
    CVSCPLog();

    /// Destructor
    virtual ~CVSCPLog();


    /*!
        Filter message

        @param pEvent Pointer to VSCP event
        @return True if message is accepted false if rejected
     */
    bool doFilter(vscpEvent *pEvent);


    /*!
        Set Filter
     */
    void setFilter(vscpEvent *pFilter);


    /*!
        Set Mask
     */
    void setMask(vscpEvent *pMask);


    /*! 
        Open/create the logfile

        @param Configuration string
        @param flags 	
                bit 1 = 0 Append, bit 
                        1 = 1 Rewrite
                bit 2,3 Format: 00 - Standard. 
                                01 - VSCP works receive format.
        @return True on success.
     */
    bool open(const char *pUsername,
            const char *pPassword,
            const char *pHost,
            short port,
            const char *pPrefix,
            const char *pConfig,
            unsigned long flags = 0);

    /*!
        Flush and close the log file
     */
    void close(void);

    /*!
        Write an event out to the file
        \param pEvent Pointer to VSCP event
        \return True on success.
     */
    bool writeEvent(vscpEvent *pEvent);


    /*!
        Open the log file
        \return true on success.
     */
    bool openFile(void);


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

    /// Path to logfile
    wxString m_path;

    /// File
    wxFile m_file;

    /// The log stream
    wxFileOutputStream *m_pLogStream;

    /// Pointer to worker thread
    CVSCPLogWrkTread *m_pthreadWork;
};

///////////////////////////////////////////////////////////////////////////////
//				Worker Tread
///////////////////////////////////////////////////////////////////////////////

class CVSCPLogWrkTread : public wxThread {
public:

    /// Constructor
    CVSCPLogWrkTread();

    /// Destructor
    ~CVSCPLogWrkTread();

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

    /// Log object
    CVSCPLog *m_pLog;

};


#endif // !defined(AFX_VSCPLOG_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_)
