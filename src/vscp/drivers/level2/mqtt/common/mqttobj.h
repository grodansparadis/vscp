// mqttobj.h: interface for the mqtt class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (http://www.vscp.org)
//
// Copyright (C) 2000-2019 Ake Hedman,
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

#if !defined(_VSCPMQTT_H__INCLUDED_)
#define _VSCPMQTT_H__INCLUDED_

#include <list>
#include <string>

#include <stdio.h>
#include <string.h>
#include <time.h>

#define _POSIX
#include <pthread.h>
#include <syslog.h>
#include <unistd.h>

#include <mongoose.h>

#include <canal.h>
#include <canal_macro.h>
#include <guid.h>
#include <vscp.h>
#include <vscpremotetcpif.h>

// Forward declarations
class CWrkThread;
class VscpRemoteTcpIf;
class Cmqtt;
class CWrkThreadObj;

class Cmqttobj
{
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
    std::string m_sessionid;

    /// Server supplied username
    std::string m_username;

    /// Server supplied password
    std::string m_password;

    /// server supplied prefix
    std::string m_prefix;

    /// server supplied host
    std::string m_host;

    /// Subscribe or Publish topic.
    std::string m_topic;

    // MQTT host (broker)
    std::string m_hostMQTT;

    // MQTT username (broker)
    std::string m_usernameMQTT;

    // MQTT password (broker)
    std::string m_passwordMQTT;

    /*!
        Structure for subscription topics. Will only hold
        one entry for this driver.
    */
    struct mg_mqtt_topic_expression m_topic_list[1];

    /*!
        Event simplification
    */
    std::string m_simplify;

    /// Flag for simple channel handling
    bool m_bSimplify;

    /// Class for simple channel handling
    uint16_t m_simple_vscpclass;

    /// Type for simple channel handling
    uint16_t m_simple_vscptype;

    /// Coding for simple channel handling
    uint8_t m_simple_coding;

    /// Unit for simple channel handling
    uint8_t m_simple_unit;

    /// Unit for simple channel handling
    uint8_t m_simple_sensorindex;

    /// zone for simple channel handling
    uint8_t m_simple_zone;

    /// Subzone for simple channel handling
    uint8_t m_simple_subzone;

    /*!
        Keepalive value
    */
    int m_keepalive;

    /// Filter
    vscpEventFilter m_vscpfilter;

    // Thread worker object
    CWrkThreadObj *m_pWrkObj;

    /// Pointer to worker thread
    pthread_t *m_threadWork;

    /// VSCP server interface
    VscpRemoteTcpIf m_srv;

    // Queue
    std::list<vscpEvent *> m_sendList;
    std::list<vscpEvent *> m_receiveList;

    /*!
        Event object to indicate that there is an event in the output queue
    */
    sem_t m_semSendQueue;
    sem_t m_semReceiveQueue;

    // Mutex to protect the output queue
    pthread_mutex_t m_mutexSendQueue;
    pthread_mutex_t m_mutexReceiveQueue;
};

///////////////////////////////////////////////////////////////////////////////
//                          Worker Tread Object
///////////////////////////////////////////////////////////////////////////////

class CWrkThreadObj
{
public:
    /// Constructor
    CWrkThreadObj();

    /// Destructor
    ~CWrkThreadObj();

    /// VSCP server interface
    VscpRemoteTcpIf m_srv;

    /// Sensor object
    Cmqttobj *m_pObj;
};

#endif // defined _VSCPMQTT_H__INCLUDED_
