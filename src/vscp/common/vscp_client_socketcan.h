// vscp_client_socketcan.h
//
// CANAL client communication classes.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// Copyright:   © 2007-2021
// Ake Hedman, the VSCP project, <info@vscp.org>
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

#if !defined(VSCPCLIENTSOCKETCAN_H__INCLUDED_)
#define VSCPCLIENTSOCKETCAN_H__INCLUDED_

#define _POSIX

#include <list>
#include <string>

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#include <hlo.h>
#include <remotevariablecodes.h>
#include <canal.h>
#include <canal_macro.h>
#include <guid.h>
#include <vscp.h>
#include <vscp_class.h>
#include <vscp_type.h>
#include <vscpdatetime.h>
#include <vscphelper.h>
#include <vscp_client_base.h>

#include <json.hpp>  // Needs C++11  -std=c++11

#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"

// https://github.com/nlohmann/json
using json = nlohmann::json;

const uint16_t MAX_ITEMS_IN_QUEUE = 32000;

// ----------------------------------------------------------------------------


// When a callback is set and connect is called this object is shared
// with a workerthread that 

class vscpClientSocketCan : public CVscpClient
{

public:

    vscpClientSocketCan();
    ~vscpClientSocketCan();

    // Socketcan flags
    const u_int32_t sockletcan_flag_fd_enable = 0x00008000; // FD frames will be handled

    // Driver flags
    static const uint32_t FLAG_ENABLE_DEBUG = 0x80000000;  // Debug mode
    static const uint32_t FLAG_FD_MODE      = 0x4000000;   // Enable FD mode

    static const uint32_t DEAULT_RESPONSE_TIMEOUT = 3;     // In ms

    /*!
        Initialize the CANAL client
        @param interface Socketcan interface name
        @param guid Interface GUID on string form
        @param flags Driver configuration flags. See m_flags below.
        @return Return VSCP_ERROR_SUCCESS of OK and error code else.
    */
    int init(const std::string &interface, 
                const std::string &guid,
                unsigned long flags,
                uint32_t timeout = DEAULT_RESPONSE_TIMEOUT); 

    // Run wizard
    int runWizard(void);

    /*!
        Connect to remote host
        @param bPoll If true polling is used.
        @return Return VSCP_ERROR_SUCCESS of OK and error code else.
    */
    virtual int connect(void);

    /*!
        Disconnect from remote host
        @return Return VSCP_ERROR_SUCCESS of OK and error code else.
    */
    virtual int disconnect(void);

    /*!
        Check if connected.
        @return true if connected, false otherwise.
    */
    virtual bool isConnected(void);

    /*!
        Send VSCP event to remote host.
        @return Return VSCP_ERROR_SUCCESS of OK and error code else.
    */
    virtual int send(vscpEvent &ev);

    /*!
        Send VSCP event to remote host.
        @return Return VSCP_ERROR_SUCCESS of OK and error code else.
    */
    virtual int send(vscpEventEx &ex);

    /*!
        Receive VSCP event from remote host
        @return Return VSCP_ERROR_SUCCESS of OK and error code else.
    */
    virtual int receive(vscpEvent &ev);
    

    /*!
        Receive VSCP event ex from remote host
        @return Return VSCP_ERROR_SUCCESS of OK and error code else.
    */
    virtual int receive(vscpEventEx &ex);

    /*!
        Set interface filter
        @param filter VSCP Filter to set.
        @return Return VSCP_ERROR_SUCCESS of OK and error code else.
    */
    virtual int setfilter(vscpEventFilter &filter);

    /*!
        Get number of events waiting to be received on remote
        interface
        @param pcount Pointer to an unsigned integer that get the count of events.
        @return Return VSCP_ERROR_SUCCESS of OK and error code else.
    */
    virtual int getcount(uint16_t *pcount);

    /*!
        Clear the input queue
        @return Return VSCP_ERROR_SUCCESS of OK and error code else.
    */
    virtual int clear(void);

    /*!
        Get version from interface
        @param pmajor Pointer to uint8_t that get major version of interface.
        @param pminor Pointer to uint8_t that get minor version of interface.
        @param prelease Pointer to uint8_t that get release version of interface.
        @param pbuild Pointer to uint8_t that get build version of interface.
        @return Return VSCP_ERROR_SUCCESS of OK and error code else.
    */
    virtual int getversion(uint8_t *pmajor,
                            uint8_t *pminor,
                            uint8_t *prelease,
                            uint8_t *pbuild);

    /*!
        Get interfaces
        @param iflist Get a list of available interfaces
        @return Return VSCP_ERROR_SUCCESS of OK and error code else.
    */
    virtual int getinterfaces(std::deque<std::string> &iflist);

    /*!
        Get capabilities (wcyd) from remote interface
        @return Return VSCP_ERROR_SUCCESS of OK and error code else.
    */
    virtual int getwcyd(uint64_t &wcyd);


    void sendToCallbacks(vscpEvent *pev);

    /*!
        Set (and enable) receive callback for events
        @return Return VSCP_ERROR_SUCCESS of OK and error code else.
    */
    virtual int setCallback(LPFNDLL_EV_CALLBACK m_evcallback);

    /*!
        Set (and enable) receive callback ex events
        @return Return VSCP_ERROR_SUCCESS of OK and error code else.
    */
    virtual int setCallback(LPFNDLL_EX_CALLBACK m_excallback);

    /*!
        Return a JSON representation of connection
        @return JSON representation as string
    */
    virtual std::string getConfigAsJson(void);

    /*!
        Set member variables from JSON representation of connection
        @param config JSON representation as string
        @return True on success, false on failure.
    */
    virtual bool initFromJson(const std::string& config);

    /*!
        Getter/setters for connection timeout
        Time is in milliseconds
    */
    virtual void setConnectionTimeout(uint32_t timeout);
    virtual uint32_t getConnectionTimeout(void);

    /*!
        Getter/setters for response timeout
        Time is in milliseconds
    */
    virtual void setResponseTimeout(uint32_t timeout);
    virtual uint32_t getResponseTimeout(void);

public:   

    // True as long as the worker thread should do it's work
    bool m_bRun;

    /// enable/disable debug output
    bool m_bDebug;

    // Mutex that protect CANAL interface when callbacks are defined
#ifndef WIN32
    pthread_mutex_t m_mutexif;
#endif

    //LPFNDLL_EV_CALLBACK m_evcallback;   // Event callback
    //LPFNDLL_EX_CALLBACK m_excallback;   // Event ex callback

    /// Socketcan interface
    std::string m_interface;

    /*!
        Flags for interface
        bit 32 - Enable debug
        bit 31 - FD mode
    */
    uint32_t m_flags;

    /// GUID for interface
    cguid m_guid;

    /////////////////////////////////////////////////////////
    //                      Logging
    /////////////////////////////////////////////////////////
    
    bool m_bEnableFileLog;                        // True to enable logging
    spdlog::level::level_enum m_fileLogLevel;     // log level
    std::string m_fileLogPattern;                 // log file pattern
    std::string m_path_to_log_file;               // Path to logfile      
    uint32_t m_max_log_size;                      // Max size for logfile before rotating occurs 
    uint16_t m_max_log_files;                     // Max log files to keep

    bool m_bConsoleLogEnable;                     // True to enable logging to console
    spdlog::level::level_enum m_consoleLogLevel;  // Console log level
    std::string m_consoleLogPattern;              // Console log pattern

    // ------------------------------------------------------------------------

    // Event lists
    std::list<vscpEvent *> m_sendList;
    std::list<vscpEvent *> m_receiveList;

    /*!
      Event object to indicate that there is an event in the
      output queue
    */
    sem_t m_semSendQueue;
    sem_t m_semReceiveQueue;

    // Mutex to protect the output queue
    pthread_mutex_t m_mutexSendQueue;
    pthread_mutex_t m_mutexReceiveQueue;

    /// Filters for input/output
    vscpEventFilter m_filterIn;
    vscpEventFilter m_filterOut;

    /// CAN mode
    int m_mode;

private:

    /// Socket for SocketCan
    //int m_socket;



    // JSON configuration
    json m_j_config;

    /*!
        True of dll connection is open
    */
    bool m_bConnected;

    /// Pointer to worker threads
    pthread_t m_threadWork;

};

#endif
