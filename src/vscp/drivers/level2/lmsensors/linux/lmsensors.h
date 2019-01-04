// Lmsensors.h: interface for the Clmsensors class.
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

#if !defined(LMSENSORS_H__INCLUDED_)
#define LMSENSORS_H__INCLUDED_

#define _POSIX

#include <deque>
#include <list>
#include <string>

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#include <canal.h>
#include <canal_macro.h>
#include <dllist.h>
#include <guid.h>
#include <vscp.h>
#include <vscpremotetcpif.h>

#define VSCP_LEVEL2_DLL_LMSENSORS_OBJ_MUTEX                                    \
    "___VSCP__DLL_L2LMSENSORS_OBJ_MUTEX____"

#define VSCP_LMSENSORS_LIST_MAX_MSG 2048

// Defult seconds between events
#define DEFAULT_INTERVAL 10

// Forward declarations
class CWrkTreadObj;
class VscpRemoteTcpIf;

class Clmsensors
{
  public:
    /// Constructor
    Clmsensors();

    /// Destructor
    virtual ~Clmsensors();

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
    std::string m_username;

    /// Server supplied password
    std::string m_password;

    /// server supplied prefix
    std::string m_prefix;

    /// server supplied host
    std::string m_host;

    /// Server supplied port
    short m_port;

    /// Number of sensors
    int m_nSensors;

    /// Filter
    vscpEventFilter m_vscpfilter;

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

    // List with active thread objects
    std::deque<CWrkTreadObj*> m_objectList;
};

///////////////////////////////////////////////////////////////////////////////
//				                Worker Tread
///////////////////////////////////////////////////////////////////////////////

class CWrkTreadObj
{
  public:
    /// Constructor
    CWrkTreadObj();

    /// Destructor
    ~CWrkTreadObj();

    /// VSCP server interface
    VscpRemoteTcpIf m_srv;

    /// Sensor object
    Clmsensors *m_pObj;

    /// Dataobject for specific sensor
    std::string m_path;
    cguid m_guid;
    int m_interval;
    int m_vscpclass;        // VSCP CLASS
    int m_vscptype;         // VSCP_TYPE  Class is measurement
    int m_datacoding;       // First databyte
    double m_divideValue;   // Divide value for read data
    double m_multiplyValue; // Multiply value for read data
    int m_readOffset;       // Offset into datafile for numerical conversion
    int m_index;            // Index for events that needs index
    int m_zone;             // Zone for events that needs zone
    int m_subzone;          // Subzone for events that needs subzone
    int m_unit;             // Unit for events that need unit
    vscpEventFilter m_vscpfilter; // Filter

    // Worker thread
    pthread_t m_pthreadWork;
};

#endif // !defined(LMSENSORS_H__INCLUDED_)
