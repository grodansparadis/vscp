// mqttobj.h: interface for the mqtt class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2016 Ake Hedman, 
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

#include <fossa.h>

#include <wx/file.h>
#include <wx/wfstream.h>

#include <canal.h>
#include <vscp.h>
#include <canal_macro.h>
#include <vscpremotetcpif.h>
#include <guid.h>

#include <list>
#include <string>

using namespace std;

#define VSCP_LEVEL2_DLL_MQTT_OBJ_MUTEX  "___VSCP__DLL_L2MQTT_OBJ_MUTEX____"
#define VSCP_MQTT_LIST_MAX_MSG          2048


// Forward declarations
class CWrkThread;
class VscpRemoteTcpIf;
class Cmqtt;



class Cmqttobj {
public:

    /// Constructor
    Cmqttobj();

    /// Destructor
    virtual ~Cmqttobj();

    /*! 
        Open
        @return True on success.
     */
    bool open(const char *pUsername,
                const char *pPassword,
                const char *pHost,
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

    /// Connected flag
    bool m_bConnected;
    
    /// True if we should subscribe. False if we should publish)
    bool m_bSubscribe;
    
    /// Session id
    wxString m_sessionid;

    /// Server supplied username
    wxString m_username;

    /// Server supplied password
    wxString m_password;

    /// server supplied prefix
    wxString m_prefix;

    /// server supplied host
    wxString m_host;

    /*!
        Subscribe or Publish topic.
     */
    wxString m_topic;
    
    /*!
        MQTT host (broker)
     */
    wxString m_hostMQTT;
    
    
    /*!
        MQTT username (broker)
     */
    wxString m_usernameMQTT;
    
    /*!
        MQTT password (broker)
     */
    wxString m_passwordMQTT;

    /*!
        Structure for subscription topics. Will only hold
        one entry for this driver.
    */
    struct ns_mqtt_topic_expression m_topic_list[ 1 ];

    /*!
        Event simplification
    */
    wxString m_simplify;
    
    /// Flag for simple channel handling
    bool m_bSimplify;
    
    /// Class for simple channel handling
    uint16_t m_simple_class;
    
    /// Type for simple channel handling
    uint16_t m_simple_type;
    
    /// Coding for simple channel handling
    uint16_t m_simple_coding;
    
    /// zone for simple channel handling
    uint16_t m_simple_zone;
            
    /// Subzone for simple channel handling
    uint16_t m_simple_subzone;
    
    /*!
        Keepalive value
    */
    int m_keepalive;

    /// Filter
    vscpEventFilter m_vscpfilter;

    /// Pointer to worker thread
    CWrkThread *m_pthreadWork;

    /// VSCP server interface
    VscpRemoteTcpIf m_srv;
    
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
//                          Worker Treads
///////////////////////////////////////////////////////////////////////////////

class CWrkThread : public wxThread {
public:

    /// Constructor
    CWrkThread();

    /// Destructor
    ~CWrkThread();

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
    Cmqttobj *m_pObj;

};


#endif // !defined(AFX_VSCPLOG_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_)
