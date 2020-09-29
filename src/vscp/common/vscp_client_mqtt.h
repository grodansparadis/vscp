// vscp_client_mqtt.h
//
// MQTT client communication classes.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// Copyright:   (C) 2007-2020
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

#if !defined(VSCPCLIENTMQTT_H__INCLUDED_)
#define VSCPCLIENTMQTT_H__INCLUDED_

#include "vscp.h"
#include "vscp_client_base.h"
#include "mosquitto.h"

#include <queue>

// Max number of events in inqueue
#define MQTT_MAX_INQUEUE_SIZE   2000

// MQTT message formats
enum enumMqttMsgFormat {jsonfmt,xmlfmt};

class vscpClientMqtt : public CVscpClient
{

public:

    vscpClientMqtt();
    ~vscpClientMqtt();

    /*!
        Set init. information for this client

        @param strHost MQTT broker to connect to. Defaults to "localhost"
        @param port Port of MQTT broker. Default is 1883        
        @param strTopicSub Subscription topic. Default is "vscp/#"
        @param strTopicPub Publishing topic. Escapes can be used
            %guid% of event
            %class% VSCP event class
            %type% VSCP event type
            %dt% VSCP event date/time
            Default is "vscp/%guid%/%class%/%type%/"
        @param clientId Client id to use. If empty string is supplied
                            a random client id will be set.  
        @param strUserName Username for connection session. If none is set
            no username/password will be used.
        @param strPassword Password for connection session. 
        @param qos Quality of service (0/1/2). Default to 0.
    */
    int init(const std::string &strHost,
                short port = 1883,                
                const std::string &strTopicSub = "vscp/#",
                const std::string &strTopicPub = "vscp/%guid%/%class%/%type%/",
                const std::string &clientId = "",                
                const std::string &strUserName = "",
                const std::string &strPassword = "",
                bool bCleanSession = false,
                int qos = 0 );

    /*!
        Set retain. Should be called before connect. Default is false.
        @param bRetain Set to true to enable retain.
    */
    int setRetain(bool bRetain);

    /*!
        Set keepalive. Should be called before connect. Default is 30.
        @param keepAlive Keep alive value.
    */
    int setKeepAlive(int keepAlive);

    /*!
        Configure the client for certificate based SSL/TLS support. Must be called before connect.
        All parameters here are sent to the mosquitto sub system.

        @param cafile	path to a file containing the PEM encoded trusted CA certificate files.  
                        Either cafile or capath must not be NULL.
        @param capath	path to a directory containing the PEM encoded trusted CA certificate files.  
                        See mosquitto.conf for more details on configuring this directory.  
                        Either cafile or capath must not be NULL.
        @param certfile	path to a file containing the PEM encoded certificate file for this client.  
                        If NULL, keyfile must also be NULL and no client certificate will be used.
        @param keyfile	path to a file containing the PEM encoded private key for this client.  
                        If NULL, certfile must also be NULL and no client certificate will be used.
        @param password	if keyfile is encrypted, password is used to decrypt the file.

        @return VSCP_ERROR_SUCCESS is returned if OK and error code else.
    */    
    int set_tls( const std::string &cafile,
   	                const std::string &capath,
   	                const std::string &certfile,
   	                const std::string &keyfile,
   		            const std::string &password );

    // TODO set will

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
        Set receive callback
        @return Return VSCP_ERROR_SUCCESS of OK and error code else.
    */
   virtual int setCallback(LPFNDLL_EV_CALLBACK m_evcallback);

    /*!
        Set receive callback
        @return Return VSCP_ERROR_SUCCESS of OK and error code else.
    */
   virtual int setCallback(LPFNDLL_EX_CALLBACK m_excallback);

public:   

    // True as long as the worker thread should do it's work
    bool m_bRun;

    /*!
        True of dll connection is open
    */
    bool m_bConnected;

    // Mutex that protect CANAL interface when callbacks are defined
    pthread_mutex_t m_mutexif;

    enumMqttMsgFormat m_format;         // Format for mqtt events (JSON/XML)

    LPFNDLL_EV_CALLBACK m_evcallback;   // Event callback
    LPFNDLL_EX_CALLBACK m_excallback;   // Event ex callback

    // If no callback is defined received events are connected in
    // this queue
    std::deque<vscpEvent*> m_receiveQueue;

private:

    std::string m_strHost;      // MQTT broker
    short m_port;               // MQTT broker port
    std::string m_strTopicSub;  // Subscribe topic template
    std::string m_strTopicPub;  // Publish topic template
    std::string m_clientId;     // Client id
    std::string m_strUserName;  // Username
    std::string m_strPassword;  // Password
    int m_qos;                  // Quality of service (0/1/2)
    bool m_bRetain;             // Enable retain
    int m_keepalive;            // Keep alive in seconds
    bool m_bCleanSession;       // Clean session on disconnect if true

    // SSL/TSL
    bool m_bTLS;                // True of a TLS/SSL connection should be done

    std::string m_cafile;	    // path to a file containing the PEM encoded trusted CA certificate files.  
                                // Either cafile or capath must not be NULL.
    std::string m_capath;	    //path to a directory containing the PEM encoded trusted CA certificate files.  
                                // See mosquitto.conf for more details on configuring this directory.  
                                // Either cafile or capath must not be NULL.
    std::string m_certfile;	    //path to a file containing the PEM encoded certificate file for this client.  
                                // If NULL, keyfile must also be NULL and no client certificate will be used.
    std::string m_keyfile;      

    std::string m_pwKeyfile;    // Password for keyfile (se only if it is encrypted on disc)

    struct mosquitto *m_mosq;   // Handel for connection
    
    // Worker thread id
    pthread_t m_tid;

};

#endif