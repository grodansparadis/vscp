// rawethernet.h: interface for the rawethernet class.
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

#if !defined(VSCPRAWETH_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_)
#define VSCPRAWETH_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_

#include <list>
#include <string>

#include <stdio.h>
#include <string.h>
#include <time.h>

#define _POSIX
#include <pthread.h>
#include <syslog.h>
#include <unistd.h>

#include <canal.h>
#include <canal_macro.h>
#include <guid.h>
#include <vscp.h>
#include <vscpdatetime.h>
#include <vscpremotetcpif.h>

#ifndef BOOL
typedef int BOOL;
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define VSCP_DLL_SONAME "vscpl2drv_raweth.1.2"

// This is the version info for this DLL - Change to your own value
#define VSCP_DLL_VERSION 2

// This is the vendor string - Change to your own value
#define VSCP_DLL_VENDOR                                                        \
    "Grodans Paradis AB, Sweden, https://www.grodansparadis.com"

// This is the VSCP rwa ethernet frame version used by this driver
#define RAW_ETHERNET_FRAME_VERSION 1

#define VSCP_LEVEL2_DLL_RAWETHERNET_OBJ_MUTEX                                  \
    "___VSCP__DLL_L2RAWETHERNET_OBJ_MUTEX____"

#define VSCP_RAWETH_DRIVERINFO  ""

// Forward declarations
class CRawEthernet;
class CWrkReadThreadObj;
class CWrkWriteThreadObj;
class VscpRemoteTcpIf;

/*!
    Add a driver object

    @parm plog Object to add
    @return handle or 0 for error
*/
long
addDriverObject(CRawEthernet *pif);

/*!
    Get a driver object from its handle

    @param handle for object
    @return pointer to object or NULL if invalid
            handle.
*/
CRawEthernet *
getDriverObject(long handle);

/*!
    Remove a driver object
    @param handle for object.
*/
void
removeDriverObject(long handle);

class CRawEthernet
{

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
              const char *pConfig,
              unsigned long flags);

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

    /// Ethernet interface to use
    std::string m_interface;

    /// Source MAC address
    std::string m_strlocalMac;
    uint8_t m_localMac[16];

    /// Subaddr of interface
    uint16_t m_subaddr;

    /// Filter
    vscpEventFilter m_vscpfilter;

    /// Pointer to worker threads
    CWrkReadThreadObj *m_preadWorkThread;
    CWrkWriteThreadObj *m_pwriteWorkThread;

    pthread_t *m_readWrkThread;
    pthread_t *m_writeWrkThread;

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
//				             Worker Treads
///////////////////////////////////////////////////////////////////////////////

class CWrkReadThreadObj
{
  public:
    /// Constructor
    CWrkReadThreadObj();

    /// Destructor
    ~CWrkReadThreadObj();

    /// VSCP server interface
    VscpRemoteTcpIf m_srv;

    /// Sensor object
    CRawEthernet *m_pObj;
};

class CWrkWriteThreadObj
{
  public:
    /// Constructor
    CWrkWriteThreadObj();

    /// Destructor
    ~CWrkWriteThreadObj();

    /// VSCP server interface
    VscpRemoteTcpIf m_srv;

    /// Sensor object
    CRawEthernet *m_pObj;
};

#endif // !defined(VSCPRAWETH_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_)
