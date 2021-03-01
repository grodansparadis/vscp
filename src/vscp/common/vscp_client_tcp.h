// vscp_client_tcp.h
//
// tcp/ip client communication classes.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// Copyright:   © 2007-2021
// Ake Hedman, Grodans Paradis AB, <akhe@vscp.org>
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

#if !defined(VSCPCLIENTTCP_H__INCLUDED_)
#define VSCPCLIENTTCP_H__INCLUDED_

#include <vscpremotetcpif.h>
#include "vscp_client_base.h"

#include <thread>
#include <mutex>         

class vscpClientTcp : public CVscpClient
{

public:

    vscpClientTcp();
    ~vscpClientTcp();

    /*!
        Initialize the tcp client
        @param strHostname Hostname and port for remote host. Can be prefixed with
            tcp:// or stcp:// (SSL connection)
        @param strUsername Username used to login on remote host.
        @param strPassword Password used to login on remote host.
        @param bPolling If true only one connection will be opended to the remote server
            on which polling for events will be done. If false one connection will be opended
            for send and one for receive. The polling is intended for low end devices which
            only accepts one client at the time.
        @return Return VSCP_ERROR_SUCCESS of OK and error code else.
    */
    int init(const std::string &strHostname = "tcp://localhost:9598",
                const std::string &strUsername = "admin",
                const std::string &strPassword = "secret",
                bool bPolling = false);                  

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

    // Init MQTT CaFile
    void setMqttCaFile(const std::string cafile) { m_bTLS = true; m_cafile = cafile; };

    // Init MQTT CaPath
    void setMqttCaPath(const std::string capath) { m_bTLS = true; m_capath = capath; };

    // Init MQTT CertFile
    void setMqttCertFile(const std::string certfile) { m_bTLS = true; m_certfile = certfile; };

    // Init MQTT KeyFile
    void setMqttKeyFile(const std::string keyfile) { m_bTLS = true; m_keyfile = keyfile; };

    // Init MQTT PwKeyFile
    void setMqttPwKeyFile(const std::string pwkeyfile) { m_bTLS = true; m_pwKeyfile = pwkeyfile; };

    // ------------------------------------------------------------------------
    //  Receive worker thread functions
    // ------------------------------------------------------------------------

    /*! 
        Get the tcp/ip receive object
        @return Pointer to receive tcp/ip object or null if not defined
    */
    VscpRemoteTcpIf *getTcpReceive(void) { return &m_tcpReceive; };

    /*!
        Send event to defined callbacks if any is defined
        @param pev Pointer to VSCP event
    */
    void sendToCallbacks(vscpEvent *pev);

public:
    /// Flag for workerthread run as long it's true
    bool m_bRun;

    /// Mutex to protect receive tcop/ip object
    std::mutex m_mutexReceive;

    /// Used for channel id (prevent sent events from being received)
    uint32_t m_obid;

private:

    /*!
        The main interface (sending) is always opened (both in poll and
        standard mode). The Receive interface is opended only in normal mode
        and do just connect - log in - enable receive loop. Received events
        will be sent on the defined callbacks.
    */

    /// Sending interface
    VscpRemoteTcpIf m_tcp;          

    /// Receiving interface
    VscpRemoteTcpIf m_tcpReceive;

    // Filter used for both channels
    vscpEventFilter m_filter;

    // Interface on remote host
    uint8_t m_interfaceGuid[16];

    /// Workerthread
    std::thread *m_pworkerthread;

    // Connection parameters set by init

    /// Remote host to connect to
    std::string m_strHostname;

    /// Port of remote host
    short m_port;

    /// Username to login with at remote host
    std::string m_strUsername;

    /// Password to login with at remote host
    std::string m_strPassword;

    /// If true the remote host interface will be polled.
    bool m_bPolling;



    // ------------------------------------------------------------------------
    //                                 TLS / SSL
    // ------------------------------------------------------------------------

    /// Enable TLS/SSL 
    bool m_bTLS;

    /*!
        the server certificate will be verified and the connection 
        aborted if the verification fails.
    */
    bool m_bVerifyPeer;

    /// CA file
    std::string m_cafile;

    /// Path to CA file (can hold filename also)
    std::string m_capath;    

    /// Path to CERT file
    std::string m_certfile; 

    /// Key file
    std::string m_keyfile;

    /// Password keyfile
    std::string m_pwKeyfile;
};

#endif
