// Log.h: interface for the CVSCPLog class.
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

#if !defined(VSCPLOG_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_)
#define VSCPLOG_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_

#include <list>
#include <string>

#define VSCP_LEVEL2_DLL_LOGGER_OBJ_MUTEX "___VSCP__DLL_L2LOGGER_OBJ_MUTEX____"

#define VSCP_LOG_LIST_MAX_MSG 2048

// Flags
#define LOG_FILE_OVERWRITE 1L  // Overwrite
#define LOG_FILE_VSCP_WORKS 2L // VSP Works format

// Forward declarations
class CLogWrkThreadObj;
class VscpTcpIf;

class CVSCPLog
{
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
              const char *pConfig);

    /*!
        Flush and close the log file
     */
    void close(void);

    /*!
            Add one event to the output queue
            \param pEvent Pointer to VSCP event
    \return True on success.S
     */
    bool addEvent2Queue(const vscpEvent *pEvent);

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

    /*!
            Add event to send queue
     */
    bool addEvent2SendQueue(const vscpEvent *pEvent);

  public:
    /// Run flag
    bool m_bQuit;

    /// Working flags
    unsigned long m_flags;

    /// Server supplied username
    std::string m_username;

    /// Server supplied password
    std::string m_password;

    /// server supplied prefix
    std::string m_prefix;

    /// server supplied host
    std::string m_host;

    /// Server supplied port
    short m_port;

    /// Path to logfile
    std::string m_path;

    /// File
    FILE m_file;

    /// The log stream
    fstream m_pLogStream;

    /// Pointer to worker thread
    CLogWrkThreadObj *m_pthreadWork;

    /// Filter
    vscpEventFilter m_Filter;

    // Queue
    std::list<vscpEvent *> m_sendList;
    std::list<vscpEvent *> m_receiveList;

    /*!
    Event object to indicate that there is an event in the output queue
 */
    sem_t m_semSendQueue;

    // Mutex to protect the output queue
    pthread_mutex_t m_mutexSendQueue;
};

///////////////////////////////////////////////////////////////////////////////
//				Worker Tread
///////////////////////////////////////////////////////////////////////////////

class CLogWrkThreadObj
{
  public:
    /// Constructor
    CLogWrkThreadObj();

    /// Destructor
    ~CLogWrkThreadObj();

    /// VSCP server interface
    VscpRemoteTcpIf m_srv;

    /// Log object
    CVSCPLog *m_pLog;
};

#endif // !defined(AFX_VSCPLOG_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_)
