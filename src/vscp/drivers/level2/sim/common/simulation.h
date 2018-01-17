// simulation.h: interface for the socketcan class.
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


#if !defined(VSCPSIMULATION_H__INCLUDED_)
#define VSCPSIMULATION_H__INCLUDED_

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

#include <canal.h>
#include <vscp.h>
#include <canal_macro.h>
#include <dllist.h>
#include <vscpremotetcpif.h>
#include <guid.h>

#include <list>
#include <string>

using namespace std;

#define SIM_OBJ_MAJOR_VERSION                       1
#define SIM_OBJ_MINOR_VERSION                       0
#define SIM_OBJ_SUBMINOR_VERSION                    0

#define VSCP_LEVEL2_DLL_TCPIPLINK_OBJ_MUTEX     "___VSCP__DLL_L2SIMOBJ_OBJ_MUTEX____"

#define VSCP_TCPIPLINK_LIST_MAX_MSG                 2048

// defaults
#define SIM_MAX_NODES                               255
#define SIM_DEFAULT_INTERVAL                        30
#define SIM_DEFAULT_UNIT                            0
#define SIM_DEFAULT_INDEX                           0
#define SIM_DEFAULT_CODING                          0
#define SIM_DESCION_MATRIX_ROWS                     8

#define SIM_CODING_NORMALIZED                       0
#define SIM_CODING_FLOAT                            1
#define SIM_CODING_STRING                           2

#define VSCP_MAJOR_VERSION                          1
#define VSCP_MINOR_VERSION                          5

#define SIM_DECISION_MATRIX_ROWS                    8

// User registers
#define SIM_USER_REG_ZONE                           0x00
#define SIM_USER_REG_SUBZONE                        0x01
#define SIM_USER_REG_INDEX                          0x02
#define SIM_USER_REG_UNIT                           0x03
#define SIM_USER_REG_CODING                         0x04
#define SIM_USER_REG_CONTROL_MEASUREMENTS           0x05
#define SIM_USER_REG_INTERVAL                       0x06

#define SIM_USER_REG_BUTTON_INDEX                   0x30
#define SIM_USER_REG_BUTTON_ZONE                    0x31
#define SIM_USER_REG_BUTTON_SUBZONE                 0x32
#define SIM_USER_REG_BUTTON_INTERVAL                0x33

#define SIM_USER_REG_DECISION_MATRIX                0x40

// Standard registers

#define VSCP_REG_ALARMSTATUS                        0x80
#define VSCP_REG_VSCP_MAJOR_VERSION                 0x81
#define VSCP_REG_VSCP_MINOR_VERSION                 0x82

#define VSCP_REG_NODE_CONTROL                       0x83

#define VSCP_REG_USERID0                            0x84
#define VSCP_REG_USERID1                            0x85
#define VSCP_REG_USERID2                            0x86
#define VSCP_REG_USERID3                            0x87
#define VSCP_REG_USERID4                            0x88

#define VSCP_REG_MANUFACTUR_ID0                     0x89
#define VSCP_REG_MANUFACTUR_ID1                     0x8A
#define VSCP_REG_MANUFACTUR_ID2                     0x8B
#define VSCP_REG_MANUFACTUR_ID3                     0x8C

#define VSCP_REG_MANUFACTUR_SUBID0                  0x8D
#define VSCP_REG_MANUFACTUR_SUBID1                  0x8E
#define VSCP_REG_MANUFACTUR_SUBID2                  0x8F
#define VSCP_REG_MANUFACTUR_SUBID3                  0x90

#define VSCP_REG_NICKNAME_ID                        0x91

#define VSCP_REG_PAGE_SELECT_MSB                    0x92
#define VSCP_REG_PAGE_SELECT_LSB                    0x93

#define VSCP_REG_FIRMWARE_MAJOR_VERSION             0x94
#define VSCP_REG_FIRMWARE_MINOR_VERSION             0x95
#define VSCP_REG_FIRMWARE_SUB_MINOR_VERSION         0x96

#define VSCP_REG_BOOT_LOADER_ALGORITHM              0x97
#define VSCP_REG_BUFFER_SIZE                        0x98
#define VSCP_REG_PAGES_USED                         0x99

// 32-bit
#define VSCP_REG_STANDARD_DEVICE_FAMILY_CODE        0x9A

// 32-bit
#define VSCP_REG_STANDARD_DEVICE_TYPE_CODE          0x9E

#define VSCP_REG_DEFAULT_CONFIG_RESTORE             0xA2

#define VSCP_REG_GUID                               0xD0
#define VSCP_REG_DEVICE_URL                         0xE0

// Actions
#define SIM_ACTION_NOOP                             0x00
#define SIM_ACTION_REPLY_TURNON                     0x01
#define SIM_ACTION_REPLY_TURNOFF                    0x02

// INIT LED function codes
#define VSCP_LED_OFF                                0x00
#define VSCP_LED_ON                                 0x01
#define VSCP_LED_BLINK1                             0x02

/*!
Decision Matrix - definitions

A matrix row consist of 8 bytes and have the following format

| oaddr | flags | class-mask | class-filter | type-mask | type-filter | action | action-param |

oaddr is the originating address.

flag
====
bit 7 - Enabled (==1).
bit 6 - oaddr should be checked (==1) or not checked (==0)
bit 5 - Reserved
bit 4 - Reserved
bit 3 - Reserved
bit 2 - Reserved
bit 1 - Classmask bit 8
bit 0 - Classfilter bit 8

Action = 0 is always NOOP, "no operation".
 */

#define VSCP_DM_POS_OADDR                           0
#define VSCP_DM_POS_FLAGS                           1
#define VSCP_DM_POS_CLASSMASK                       2
#define VSCP_DM_POS_CLASSFILTER                     3
#define VSCP_DM_POS_TYPEMASK                        4
#define VSCP_DM_POS_TYPEFILTER                      5
#define VSCP_DM_POS_ACTION                          6
#define VSCP_DM_POS_ACTIONPARAM                     7

#define VSCP_DM_FLAG_ENABLED                        0x80
#define VSCP_DM_FLAG_CHECK_OADDR                    0x40
#define VSCP_DM_FLAG_HARDCODED                      0x20
#define VSCP_DM_FLAG_CHECK_ZONE                     0x10
#define VSCP_DM_FLAG_CHECK_SUBZONE                  0x08
#define VSCP_DM_FLAG_CLASS_MASK                     0x02
#define VSCP_DM_FLAG_CLASS_FILTER                   0x01

/*!
\struct _dmrow
Decision matrix row element (for RAM storage)
Each DM row consist of a structure of this type.
 */
struct _dmrow {
    uint8_t oaddr;          ///< Originating address
    uint8_t flags;          ///< Decision matrix row flags
    uint8_t class_mask;     ///< Mask for class (lower eight bits)
    uint8_t class_filter;   ///< Filter for class (lower eight bits)
    uint8_t type_mask;      ///< Mask for type
    uint8_t type_filter;    ///< Filter for type
    uint8_t action;         ///< Action code
    uint8_t action_param;   ///< Action parameter
};


// Forward declarations
class CWrkTread;
class VscpTcpIf;
class wxFile;

class CSim {
public:

    /// Constructor
    CSim();

    /// Destructor
    virtual ~CSim();

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

    /// server supplied prefix
    wxString m_prefix;

    /// Number of nodes
    int m_nNodes;

    /// Send channel id
    uint32_t txChannelID;

    /// Pointer to worker thread for send
    CWrkTread *m_pthreadWork[ SIM_MAX_NODES ];

    /// VSCP local server interface
    VscpRemoteTcpIf m_srvLocal;

    /// VSCP remote server interface
    VscpRemoteTcpIf m_srvRemote;

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
//                              Worker Tread
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

    /*!
        sendEvent
        Send event
        @param eventEx Event to send
        @return true on success.
     */
    bool sendEvent(vscpEventEx& eventEx);

    /*!
        readLevel1Register
        @param reg Register to read 0-255
        @return Register value
     */
    uint8_t readLevel1Register(uint8_t reg);

    /*!
        writeLevel1Register
        @param reg Register to read 0-255
        @param val Value to write
        @return Register value
     */
    uint8_t writeLevel1Register(uint8_t reg, uint8_t val);

    /*!
        readLevel1Register
        @param reg Register to read 0-255
        @return True on success
     */
    bool readLevel2Register(uint32_t reg);

    /*!
        writeLevel2Register
        @param reg Register to read 0-0xffffffff
        @param val Value to write
        @return True on success
     */
    bool writeLevel2Register(uint32_t reg, uint8_t val);

    /*!
        readUserReg   
        @param reg User register to read
        @return Content of register.
     */
    uint8_t readUserReg(uint8_t reg);

    /*!
        writeUserReg 
        @param reg User register to write
        @param value Value to write
        @return Content of register.
     */
    uint8_t writeUserReg(uint8_t reg, uint8_t value);

    /*!
        readStandardReg
        @param reg Standard register to read
        @return Content of register.
     */
    uint8_t readStandardReg(uint8_t reg);

    /*!
        readStandardReg
        @param reg Standard register to write
        @param value Value to write
        @return Content of register.
     */
    uint8_t writeStandardReg(uint8_t reg, uint8_t value);

    /*!
        Feed event through DM
        @param pEvent Incomming event to feed the mateix with
     */
    void doDM(vscpEvent *pEvent);

    /*!
        Reply with TurnOn Event
        @param pEvent Event that made the action trigger
        @param dmflags Decision Martix flags
        @param dmparam Decision matrix paramter byte
     */
    void doActionReplyTurnOn(vscpEvent *pEvent, uint8_t dmflags, uint8_t dmparam);

    /*!
        Reply with TurnOff Event
        @param pEvent Event that made the action trigger
        @param dmflags Decision Martix flags
        @param dmparam Decision matrix paramter byte
     */
    void doActionReplyTurnOff(vscpEvent *pEvent, uint8_t dmflags, uint8_t dmparam);

    /////////////////////////////
    //   N O D E  data start   //
    /////////////////////////////

    /// True if this is a level II node
    bool m_bLevel2;

    /// GUID for this 
    cguid m_guid;

    /// String holding path to simulated data.
    wxString m_path;

    /// Measurement class to use
    long m_measurementClass;

    /// Measurement type to use
    long m_measurementType;

    /// Filter
    vscpEventFilter m_vscpfilter;

    /// User registers
    uint8_t m_registers[ 256 ]; // Standard regs > 0x80 

    uint8_t vscp_alarmstatus;
    uint16_t vscp_page_select;

    /////////////////////////////
    //     N O D E  data end   //
    /////////////////////////////;

    /// Sensor object
    CSim *m_pObj;

};





#endif // !defined(AFX_VSCPLOG_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_)
