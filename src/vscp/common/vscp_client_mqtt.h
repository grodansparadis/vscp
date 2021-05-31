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

#if !defined(VSCPCLIENTMQTT_H__INCLUDED_)
#define VSCPCLIENTMQTT_H__INCLUDED_

#include <vscp.h>
#include <vscphelper.h>
#include "vscp_client_base.h"

#include <pthread.h>
#include <mosquitto.h>

#include <string>
#include <queue>
#include <list>

// Max number of events in inqueue
#define MQTT_MAX_INQUEUE_SIZE   2000

class publishTopic 
{

 public:
    publishTopic(const std::string& topic = "", int qos = 0, bool bretain = false);
    ~publishTopic();

    /// Getters/Setters for topic
    std::string getTopic(void) { return m_topic; };
    void setTopic(const std::string topic) { m_topic=topic; };

    /// Getters/setters for qos
    int getQos(void) { return m_qos; };
    void setQos(int qos) { m_qos = qos; };

    /// getters/&Setters for retain
    bool getRetain(void) { return m_bRetain; };
    bool isRetain(void) { return m_bRetain; };
    void setRetain(bool bRetain) { m_bRetain = bRetain; };

 private:
    
    /// Publish topic
    std::string m_topic;

    /* 
        Quality of service for messages published
        on this topic
    */
    int m_qos;

    /// publish topic message retain flag
    bool m_bRetain;

};


// ----------------------------------------------------------------------------



class subscribeTopic 
{

 public:
    subscribeTopic(const std::string& topic = "", int qos = 0);
    ~subscribeTopic();

    /// Getters/Setters for topic
    std::string getTopic(void) { return m_topic; };
    void setTopic(const std::string topic) { m_topic=topic; };

    /// Getters/setters for qos
    int getQos(void) { return m_qos; };
    void setQos(int qos) { m_qos = qos; };

 private:
    
    /// Publish topic
    std::string m_topic;

    /* 
        Quality of service for messages published
        on this topic
    */
    int m_qos;

    /// publish topic message retain flag
    bool m_bRetain;

};


// ----------------------------------------------------------------------------


// MQTT message formats - Moved to vscp.h
// enum enumMqttMsgFormat {jsonfmt,xmlfmt,strfmt};

class vscpClientMqtt : public CVscpClient
{

public:

    vscpClientMqtt();
    ~vscpClientMqtt();

    /*!
        Set init. information for this client

        @param strHost MQTT broker to connect to. Defaults to "localhost"       
        @param clientId Client id to use. If empty string is supplied
                            a random client id will be set.  
        @param strUserName Username for connection session. If none is set
            no username/password will be used.
        @param strPassword Password for connection session. 
        @param qos Quality of service (0/1/2). Default to 0.
        @return VSCP_ERROR_SUCCESS on success
    */
    int init(const std::string &strHost,
                const std::string &clientId = "",
                const std::string &strUserName = "",
                const std::string &strPassword = "",
                int keepAliveInterval = 20,
                bool bCleanSession = false);

    // Init MQTT host
    void setMqttHost(const std::string host = "tcp://localhost:1883") 
            { strncpy(m_host, host.c_str(), sizeof(m_host)); };

    // Init MQTT user
    void setMqttUser(const std::string user) 
            { strncpy(m_username, user.c_str(), sizeof(m_username)); };

    // Init MQTT password
    void setMqttPassword(const std::string password) 
            { strncpy(m_password, password.c_str(), sizeof(m_password)); };

    // Init MQTT cleansession 
    void setMqttCleanSession(const bool bCleanSession = true) { m_bCleanSession = bCleanSession; };

    // Init MQTT keepalive
    void setMqttKeepAlive(const int keepalive) { m_bTLS = true; m_keepalive = keepalive; };

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

    /*!
        Add subscription item
        
        @param strTopicSub Subscription topic. Default is "vscp/#"
        @return VSCP_ERROR_SUCCESS on success
    */
    int addSubscription(const std::string strTopicSub);

    /*!
        Add publishing item

        @param strTopicPub Publishing topic. Escapes can be used
        @param qos Quality of service 0-3
        @param bRetain Set to true for events published on this topic to be
                    retained.
        @return VSCP_ERROR_SUCCESS on success
    */
    int addPublish(const std::string strTopicPub, int qos=0, bool bRetain=false);

    /*!
        Set retain. Should be called before connect. Default is false.
        @param bRetain Set to true to enable retain.
        @return VSCP_ERROR_SUCCESS on success
    */
    int setRetain(bool bRetain);

    /*!
        Set keepalive. Should be called before connect. Default is 30.
        @param keepAlive Keep alive value.
        @return VSCP_ERROR_SUCCESS on success
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
    int set_tls(const std::string &cafile,
                    const std::string &capath,
                    const std::string &certfile,
                    const std::string &keyfile,
                    const std::string &password);

    // TODO set will

    /*!
        Connect to remote host
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
    virtual void setConnectionTimeout(uint32_t timeout)
    {
        m_timeoutConnection = timeout;
    };

    virtual uint32_t getConnectionTimeout(void)
    {
        return m_timeoutConnection;
    };

    /*!
        Getter/setters for response timeout
        Time is in milliseconds
    */
    virtual void setResponseTimeout(uint32_t timeout)
    {
        m_timeoutResponse = timeout;
    };
    virtual uint32_t getResponseTimeout(void)
    {
        return m_timeoutResponse;
    };

    /*!
        Getter for remote host

        @return Return remote host as string
    */
    std::string getHost(void) 
    {
        std::string str = m_host;
        return vscp_getHostFromInterface(str);
    }

    /*!
        Getter for remote port
        @return remote host port.
    */
    unsigned short getPort(void) 
    {
        std::string str = m_host;
        return vscp_getPortFromInterface(str);
    }

public:   

    // Timeout in milliseconds for host connection.
    uint32_t m_timeoutConnection;

    // Timeout in milliseconds for response.
    uint32_t m_timeoutResponse;

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

    std::list<subscribeTopic> m_listTopicSub;   // Subscribe topic templates
    std::list<publishTopic> m_listTopicPub;     // Publish topic templates

    vscpEventFilter m_filter;   // Receive filter

    char m_host[256];           // MQTT broker
    char m_clientid[256];       // Client id
    char m_username[128];       // Username
    char m_password[128];       // Password

    int m_qos;                  // Quality of service (0/1/2)
    bool m_bRetain;             // Enable retain
    int m_keepalive;            // Keep alive in seconds
    bool m_bCleanSession;       // Clean session on disconnect if true

    // SSL/TSL
    bool m_bTLS;                // True of a TLS/SSL connection should be active

    std::string m_cafile;	    // path to a file containing the PEM encoded trusted CA certificate files.  
                                // Either cafile or capath must not be NULL.
    std::string m_capath;	    // path to a directory containing the PEM encoded trusted CA certificate files.  
                                // See mosquitto.conf for more details on configuring this directory.  
                                // Either cafile or capath must not be NULL.
    std::string m_certfile;	    // path to a file containing the PEM encoded certificate file for this client.  
                                // If NULL, keyfile must also be NULL and no client certificate will be used.
    std::string m_keyfile;      

    std::string m_pwKeyfile;    // Password for keyfile (set only if it is encrypted on disc)

    struct mosquitto *m_mosq;   // Handel for connection
    int m_mid;

    // Worker thread id
    pthread_t m_tid;

};

#endif