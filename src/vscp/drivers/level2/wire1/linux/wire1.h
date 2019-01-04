// socketcan.h: interface for the socketcan class.
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

#if !defined(VSCPWIRE1_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_)
#define VSCPWIRE1_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_

#define _POSIX

#include <list>
#include <string>

#include <stdio.h>
#include <string.h>
#include <time.h>

#include <pthread.h>
#include <syslog.h>
#include <unistd.h>

#include <canal.h>
#include <canal_macro.h>
#include <dllist.h>
#include <guid.h>
#include <vscp.h>
#include <vscpremotetcpif.h>

#define VSCP_LEVEL2_DLL_WIRE1_OBJ_MUTEX "___VSCP__DLL_L2WIRE1_OBJ_MUTEX____"

#define VSCP_WIRE1_LIST_MAX_MSG 2048

// Defult seconds between events
#define DEFAULT_INTERVAL 30
#define DEFAULT_UNIT 1   // Celsius
#define DEFAULT_CODING 0 // Normalized integer

// Forward declarations
class CWrkTread;
class VscpRemoteTcpIf;
class CWire1;

class CSensor 
{
  public:
    /// Constructor
    CSensor();

    /// Destructor
    ~CSensor();

    /// VSCP server interface
    VscpRemoteTcpIf m_srv;

    /// Pointer to worker thread
    pthread_t m_threadWork;

    /// Sensor object
    CWire1 *m_pW1Obj;

    /// Data object for specific sensor
    std::string m_path;
    cguid m_guid;
    int m_interval;
    int m_index;  // Index for events that needs index
    int m_unit;   // Unit for events that need unit
    int m_coding; // 0=normalizes/1=string/2=float

    FILE *m_pFile;
    char m_line1[80];
    char m_line2[80];
    unsigned int m_id[9];
    int m_temperature;
};


class CWire1
{
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

    // List with sensors
    std::list<CSensor *> m_sensorList;

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


#endif // !defined(VSCPWIRE1_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_)
