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

// libmosquitto should be >= 1.4.15

// UBUNTU
// ======
// bionic (18.04LTS) (libs): MQTT version 3.1/3.1.1 client library [universe]
// 1.4.15-2ubuntu0.18.04.3 [security]: amd64 i386
// 1.4.15-2 [ports]: arm64 armhf ppc64el s390x

// bionic-updates (libs): MQTT version 3.1/3.1.1 client library [universe]
// 1.4.15-2ubuntu0.18.04.3: amd64 arm64 armhf i386 ppc64el s390x

// focal (20.04LTS) (libs): MQTT version 5.0/3.1.1/3.1 client library [universe]
// 1.6.9-1: amd64 arm64 armhf ppc64el s390x

// groovy (20.10) (libs): MQTT version 5.0/3.1.1/3.1 client library [universe]
// 1.6.12-1: amd64 arm64 armhf ppc64el s390x

// hirsute (21.04) (libs): MQTT version 5.0/3.1.1/3.1 client library [universe]
// 2.0.10-3: amd64 arm64 armhf ppc64el s390x
// impish (libs): MQTT version 5.0/3.1.1/3.1 client library [universe]
// 2.0.10-6: amd64 arm64 armhf ppc64el s390x

// JSON configuration

/*
{
      "bind" : "interface",
      "host" : "[s]tcp://192.168.1.7:1883",
      "port" : 1883,
      "mqtt-options" : {
          "tcp-nodelay" : true,
          "protocol-version": 500,
          "receive-maximum": 20,
          "send-maximum": 20,
          "ssl-ctx-with-defaults": 0,
          "tls-ocsp-required": 0,
          "tls-use-os-certs" : 0
      },
      "user" : "vscp",
      "password": "secret",
      "clientid" : "mosq-vscp-daemon-000001",
      "publish-format" : "json",
      "subscribe-format" : "auto",
      "qos" : 1,
      "bcleansession" : false,
      "bretain" : false,
      "keepalive" : 60,
      "reconnect" : {
        "delay" : 2,
        "delay-max" : 10,
        "exponential-backoff" : false
      },
      "filter": {
        "priority-filter": 0,
        "priority-mask": 0,
        "class-filter": 0,
        "class-mask": 0,
        "type-filter": 0,
        "type-mask": 0,
        "guid-filter": "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00",
        "guid-mask": "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"
      },
      "tls" : {
          "cafile" : "",
          "capath" : "",
          "certfile" : "",
          "keyfile" : "",
          "pwkeyfile" : "",
          "no-hostname-checking" : true,
          "cert-reqs" : 0,
          "version": "",
          "ciphers": "",
          "psk": "",
          "psk-identity" : ""
      },
      "will": {
          "topic": "topic",
          "qos": 0,
          "retain": true,
          "payload": "string/json/xml"
      },
      "subscribe": [
        {
          "topic": "subscribe/topic/A",
          "qos": 0,
          "v5-options": 4,
          "auto"
        },
        {
          "topic":"subscribe/topic/B",
          "qos": 0,
          "v5-options": "NO_LOCAL, RETAIN_AS_PUBLISHED, SEND_RETAIN_ALWAYS, SEND_RETAIN_NEW, SEND_RETAIN_NEVER".
          "auto"
        }
      ],
      "bescape-pub-topics": true,
      "publish": [
        {
          "topic": "publish/topic/A",
          "qos": 0,
          "bretain": false,
          "format": "json"
        },
        {
          "topic": "publish/topic/B",
          "qos": 0,
          "bretain": false,
          "format": "xml"
        }
      ],
      "v5" : {
          "user-properties": {
              "prop1" : "value",
              "prop2" : "value"
          }
      }
}

bind - Optional. If use set ip-address
host - If starts with stcp:\\ secure connection is expected. If no prefix or tcp:\\
        unsecure connection.
port - is host contains server port then a separate define of port is not needed.
mqtt-options/tcp-nodelay is true by default disabling nigles algorithm.
mqtt-options/protocol-version can be set to 310/311/500
*/

#if !defined(VSCPCLIENTMQTT_H__INCLUDED_)
#define VSCPCLIENTMQTT_H__INCLUDED_

#include "vscp_client_base.h"
#include <guid.h>
#include <vscp.h>
#include <vscphelper.h>

#include <mosquitto.h>
#include <pthread.h>

#include <list>
#include <map>
#include <queue>
#include <string>

// Max number of events in inqueue
#define MQTT_MAX_INQUEUE_SIZE 2000
#ifndef MQTT_MAX_CLIENTID_LENGTH
#define MQTT_MAX_CLIENTID_LENGTH 23
#endif

#ifdef WIN32
  typedef void ( __stdcall * LPFN_PARENT_CALLBACK_LOG ) ( struct mosquitto *mosq, void *pParent, int level, const char *logmsg );
  typedef void ( __stdcall * LPFN_PARENT_CALLBACK_CONNECT ) ( struct mosquitto *mosq, void *pData, int rv );
  typedef void ( __stdcall * LPFN_PARENT_CALLBACK_DISCONNECT ) ( struct mosquitto *mosq, void *pData, int rv );  
  typedef void ( __stdcall * LPFN_PARENT_CALLBACK_PUBLISH ) ( struct mosquitto *mosq, void *pData, int mid ); 
  typedef void ( __stdcall * LPFN_PARENT_CALLBACK_SUBSCRIBE ) ( struct mosquitto *mosq, void *pData, int mid, int qos_count, const int *granted_qos );
  typedef void ( __stdcall * LPFN_PARENT_CALLBACK_UNSUBSCRIBE ) ( struct mosquitto *mosq, void *pData, int mid );     
  typedef void ( __stdcall * LPFN_PARENT_CALLBACK_MESSAGE ) ( struct mosquitto *mosq, void *pData, const struct mosquitto_message *pMsg );
#else
  typedef void ( *LPFN_PARENT_CALLBACK_LOG ) ( struct mosquitto *mosq, void *pParent, int level, const char *logmsg ); 
  typedef void ( *LPFN_PARENT_CALLBACK_CONNECT ) ( struct mosquitto *mosq, void *pData, int rv );  
  typedef void ( *LPFN_PARENT_CALLBACK_DISCONNECT ) ( struct mosquitto *mosq, void *pData, int rv ); 
  typedef void ( *LPFN_PARENT_CALLBACK_PUBLISH ) ( struct mosquitto *mosq, void *pData, int mid ); 
  typedef void ( *LPFN_PARENT_CALLBACK_SUBSCRIBE ) ( struct mosquitto *mosq, void *pData, int mid, int qos_count, const int *granted_qos );
  typedef void ( *LPFN_PARENT_CALLBACK_UNSUBSCRIBE ) ( struct mosquitto *mosq, void *pData, int mid ); 
  typedef void ( *LPFN_PARENT_CALLBACK_MESSAGE ) ( struct mosquitto *mosq, void *pData, const struct mosquitto_message *pMsg );   
#endif

class publishTopic {

public:
#if LIBMOSQUITTO_MAJOR > 1 || (LIBMOSQUITTO_MAJOR == 1 && LIBMOSQUITTO_MINOR >= 6)
  publishTopic(const std::string &topic,
               enumMqttMsgFormat format       = jsonfmt,
               int qos                        = 0,
               bool bretain                   = false,
               mosquitto_property *properties = NULL);
#else
  publishTopic(const std::string &topic, enumMqttMsgFormat format = jsonfmt, int qos = 0, bool bretain = false);
#endif
  ~publishTopic();

  /// Getters/Setters for topic
  std::string getTopic(void) { return m_topic; };
  void setTopic(const std::string topic) { m_topic = topic; };

  /// Getters/setters for qos
  int getQos(void) { return m_qos; };
  void setQos(int qos) { m_qos = qos; };

  /// getters/&Setters for retain
  bool getRetain(void) { return m_bRetain; };
  bool isRetain(void) { return m_bRetain; };
  void setRetain(bool bRetain) { m_bRetain = bRetain; };

  /// getters/setter for format
  void setFormat(enumMqttMsgFormat format) { m_format = format; };
  enumMqttMsgFormat getFormat(void) { return m_format; };

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

  /*!
    Publish format json (default), xml, string, binary
  */
  enumMqttMsgFormat m_format;

  /// Version 5 property list
#if LIBMOSQUITTO_MAJOR > 1 || (LIBMOSQUITTO_MAJOR == 1 && LIBMOSQUITTO_MINOR >= 6)
  mosquitto_property *m_properties;
#endif
};

// ----------------------------------------------------------------------------

class subscribeTopic {

public:
#if LIBMOSQUITTO_MAJOR > 1 || (LIBMOSQUITTO_MAJOR == 1 && LIBMOSQUITTO_MINOR >= 6)
  subscribeTopic(const std::string &topic,
                 enumMqttMsgFormat format       = autofmt,
                 int qos                        = 0,
                 int v5_options                 = 0,
                 mosquitto_property *properties = NULL);
#else
  subscribeTopic(const std::string &topic, enumMqttMsgFormat format = autofmt, int qos = 0, int v5_options = 0);
#endif
  ~subscribeTopic();

  /// Getters/Setters for topic
  std::string getTopic(void) { return m_topic; };
  void setTopic(const std::string topic) { m_topic = topic; };

  /// Getters/setters for qos
  int getQos(void) { return m_qos; };
  void setQos(int qos) { m_qos = qos; };

  /// getters/setter for format
  void setFormat(enumMqttMsgFormat format) { m_format = format; };
  enumMqttMsgFormat getFormat(void) { return m_format; };

private:
  /// Subscribe topic
  std::string m_topic;

  /*
      Quality of service for messages subscribe
      on this topic
  */
  int m_qos;

  /// version 5 options
  int m_v5_options;

  /*!
    Subscribe format auto(default),json,xml,string,binary
  */
  enumMqttMsgFormat m_format;

/// Version 5 property list
#if LIBMOSQUITTO_MAJOR > 1 || (LIBMOSQUITTO_MAJOR == 1 && LIBMOSQUITTO_MINOR >= 6)
  mosquitto_property *m_properties;
#endif

  // v5 subscription options
  bool m_bSubOptNoLocal;
  bool m_bSubOptRetainAsPublished;
  bool m_bSubOptRetainAlways;
  bool m_bSubOptRetainNew;
  bool m_bSubOptRetainNever;
};

// ----------------------------------------------------------------------------

// MQTT message formats - Moved to vscp.h
// enum enumMqttMsgFormat {jsonfmt,xmlfmt,strfmt};

class vscpClientMqtt : public CVscpClient {

public:
  vscpClientMqtt();
  ~vscpClientMqtt();

  /*!
      Handle incoming message
      @param pmsg Incoming MQTT message
      @return true on success, false on failure.
  */
  bool handleMessage(const struct mosquitto_message *pmsg);

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
      @param ev VSCP event
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int send(vscpEvent &ev);

  /*!
      Send VSCP event to remote host.
      @param ex VSCP event ex
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
  virtual int getversion(uint8_t *pmajor, uint8_t *pminor, uint8_t *prelease, uint8_t *pbuild);

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
    @param m_evcallback Pointer to callback for VSCP event delivery
    @param pData Pointer to optional user data.
    @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int setCallback(LPFNDLL_EV_CALLBACK m_evcallback, void *pData = nullptr);

  /*!
    Set (and enable) receive callback ex events
    @param m_evcallback Pointer to callback for VSCP event ex delivery
    @param pData Pointer to optional user data.
    @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int setCallback(LPFNDLL_EX_CALLBACK m_excallback, void *pData = nullptr);

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
  virtual bool initFromJson(const std::string &config);

  /*!
      Initialize MQTT
  */
  bool init(void);

  /*!
    Add subscription
    @param strTopicSub Topic to subscribe to
    @param qos The requested Quality of Service for this subscription.
    @return Return VSCP_ERROR_SUCCESS if all goes well. Error code if not.
  */
#if LIBMOSQUITTO_MAJOR > 1 || (LIBMOSQUITTO_MAJOR == 1 && LIBMOSQUITTO_MINOR >= 6)
  int addSubscription(const std::string strTopicSub,
                      enumMqttMsgFormat format,
                      int qos                        = 0,
                      int v5_options                 = 0,
                      mosquitto_property *properties = NULL);
#else
  int addSubscription(const std::string strTopicSub, enumMqttMsgFormat format, int qos = 0, int v5_options = 0);
#endif

  /*
      Add publish
  */
#if LIBMOSQUITTO_MAJOR > 1 || (LIBMOSQUITTO_MAJOR == 1 && LIBMOSQUITTO_MINOR >= 6)
  int addPublish(const std::string strTopicPub,
                 enumMqttMsgFormat format,
                 int qos                        = 0,
                 bool bRetain                   = false,
                 mosquitto_property *properties = NULL);
#else
  int addPublish(const std::string strTopicPub, enumMqttMsgFormat format, int qos = 0, bool bRetain = false);
#endif

  /*!
      Getter/setters for connection timeout
      Time is in milliseconds
  */
  virtual void setConnectionTimeout(uint32_t timeout) { m_timeoutConnection = timeout; };
  virtual uint32_t getConnectionTimeout(void) { return m_timeoutConnection; };

  /*!
      Getter/setters for response timeout
      Time is in milliseconds
  */
  virtual void setResponseTimeout(uint32_t timeout) { m_timeoutResponse = timeout; };
  virtual uint32_t getResponseTimeout(void) { return m_timeoutResponse; };

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

  /*!
    Set server GUID {{srvguid}}
    @param guid Guid to set
  */
  void setSrvGuid(const cguid &guid) { m_srvguid = guid; };

  /*!
    Set driver/interface GUID {{ifguid}}
    @param guid Guid to set
  */
  void setIfGuid(const cguid &guid) { m_ifguid = guid; };

  /*!
    Get Mosquitto connection handle
    @return Pointe rto mosquitto connection handle
  */
  struct mosquitto *getMqttHandle(void) { return m_mosq; };

  /*!
    SetTokenMaps
    Store pointers to maps that are used to get token from
    VSCP class or VSCP type codes. The tokens are used in
    the send routines to escape MQTT topics
  */
  void setTokenMaps(std::map<uint16_t, std::string> *pmap_class, std::map<uint32_t, std::string> *pmap_type)
  {
    m_pmap_class = pmap_class;
    m_pmap_type  = pmap_type;
  }

  /*!
    Define user escape.
    If key is already defined it will get a new value.
    @param key User escape key. Used as "{{key}} it will be replaced
      by "value".
    @param value The value for the user escape.  
  */
 void setUserEscape(const std::string key, const std::string value) 
  { m_mapUserEscapes[key] = value; };

   /*!
    Set parent object pointer
  */
  void setParent(void *pParent) { m_pParent = pParent; };
  
  /*!
    Set parent log callback
  */
  void setFuncParentCallbackLog(LPFN_PARENT_CALLBACK_LOG func) { m_parentCallbackLog = func; };

  /*!
    Set parent connect callback
  */
  void setFuncParentCallbackConnect(LPFN_PARENT_CALLBACK_CONNECT func) { m_parentCallbackConnect = func; };

  /*!
    Set parent disconnect callback
  */
  void setFuncParentCallbackDisconnet(LPFN_PARENT_CALLBACK_DISCONNECT func) { m_parentCallbackDisconnect = func; };

  /*!
    Set parent publish callback
  */
  void setFuncParentCallbackPublish(LPFN_PARENT_CALLBACK_PUBLISH func) { m_parentCallbackPublish = func; };

  /*!
    Set parent subscribe callback
  */
  void setFuncParentCallbackSubscribe(LPFN_PARENT_CALLBACK_SUBSCRIBE func) { m_parentCallbackSubscribe = func; };

  /*!
    Set parent unsubscribe callback
  */
  void setFuncParentCallbackUnsubscribe(LPFN_PARENT_CALLBACK_UNSUBSCRIBE func) { m_parentCallbackUnsubscribe = func; };

  /*!
    Set parent message callback
  */
  void setFuncParentCallbackMessage(LPFN_PARENT_CALLBACK_MESSAGE func) { m_parentCallbackMessage = func; };

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

  /*!
    Enable publishing escapes.
  */
  bool m_bEscapesPubTopics;

  /*!
    Will add measurement block to JSON published
    events if true
  */
  bool m_bJsonMeasurementAdd;

  /*!
    Mutex that protect CANAL interface when callbacks are defined
  */
  pthread_mutex_t m_mutexif;

  /*!
    If no callback is defined received events are connected in
    this queue
  */
  std::deque<vscpEvent *> m_receiveQueue;

  // * * * Parent callbacks * * *
  
  /*!
    Pointer to parent object
  */
  void *m_pParent;

  LPFN_PARENT_CALLBACK_LOG m_parentCallbackLog;
  LPFN_PARENT_CALLBACK_CONNECT m_parentCallbackConnect;
  LPFN_PARENT_CALLBACK_DISCONNECT m_parentCallbackDisconnect;
  LPFN_PARENT_CALLBACK_PUBLISH m_parentCallbackPublish;
  LPFN_PARENT_CALLBACK_SUBSCRIBE m_parentCallbackSubscribe;
  LPFN_PARENT_CALLBACK_UNSUBSCRIBE m_parentCallbackUnsubscribe;
  LPFN_PARENT_CALLBACK_MESSAGE m_parentCallbackMessage;

private:
  /*!
      Subscribe topic templates
  */
  std::list<subscribeTopic *> m_mqtt_subscribeTopicList;

  /*!
    Publish topic templates
  */
  std::list<publishTopic *> m_mqtt_publishTopicList;

  /*!
    Receive filter
  */
  vscpEventFilter m_filter;

  /*!
    Saved JSON configuration
  */
  json m_json;

  /*!
      Map for MQTT integer options
  */
  std::map<std::string, int> m_mapMqttIntOptions;

  /*!
      Map for MQTT string options
  */
  std::map<std::string, std::string> m_mapMqttStringOptions;

  /*!
      Map for MQTT void options
  */
  std::map<std::string, void *> m_mapMqttVoidOptions;

  /*!
      Map for MQTT v5 properties
  */
  std::map<std::string, std::string> m_mapMqttProperties;

  /*!
      Map for publish topic user escapes.
      They are set in pairs like "drivername" = "xxxxxxxxxx"
  */
  std::map<std::string, std::string> m_mapUserEscapes;

  /*!
    Pointer to map that maps a VSCP class code to it's string
    token. This is used in send to escape MQTT topics.
  */
  std::map<uint16_t, std::string> *m_pmap_class;

  /*!
    Pointer to map that maps a VSCP type code to it's string
    token. This is used in send to escape MQTT topics.
    key: vscp_class << 16 + vscp_type
  */
  std::map<uint32_t, std::string> *m_pmap_type;

  /*!
      Specific interface to bind to.
      Leave blank for all.
  */
  std::string m_bindInterface;

  /*!
      MQTT broker address on the form
      tcp://192.1689.1.9:1883  same as 192.168.1.9
      stcp://192.168.1.8:8883
  */
  std::string m_host; // MQTT broker

  /*!
    GUID for host
  */
  cguid m_srvguid;

  /*!
    GUID for driver
  */
  cguid m_ifguid;


  /*!
      Extracted from host address or set directly
  */
  uint16_t m_port; // MQTT port

  /*!
      Client if for this client.
  */
  std::string m_clientid; // Client id

  /*!
      User name to use to login to MQTT broker.
  */
  std::string m_username; // Username

  /*!
      Password to use to login to MQTT broker.
  */
  std::string m_password; // Password

  // enum enumMqttMsgFormat {jsonfmt,xmlfmt,strfmt,binfmt,autofmt};
  /*!
      Format for published events
      (autofmt is not valid)
  */
  enumMqttMsgFormat m_publish_format;

  /*!
      Accepted subscribe events
      (autofmt) accepts all
  */
  enumMqttMsgFormat m_subscribe_format;

  /*!
      Quality of service (0/1/2)
      0 - At most once (default)
      1 - At least once
      2 - Exactly once
  */
  int m_qos;

  /*!
      // Enable retain if true
  */
  bool m_bRetain;

  /*!
      Keep alive interval in seconds
  */
  int m_keepalive;

  /*!
      Start a clean session if set to true
  */
  bool m_bCleanSession;

  // Reconnect settings
  // The default behaviour if this function is not used is to repeatedly attempt to reconnect
  // with a delay of 1 second until the connection succeeds.
  // Use reconnect_delay parameter to change the delay between successive reconnection attempts.
  // You may also enable exponential backoff of the time between reconnections by setting
  // reconnect_exponential_backoff to true and set an upper bound on the delay with reconnect_delay_max.
  int m_reconnect_delay;
  int m_reconnect_delay_max;
  bool m_reconnect_exponential_backoff;

  // SSL/TLS
  bool m_bTLS; // True of a TLS/SSL connection should be active

  /*!
    mosquitto_tls_insecure_set
    if set to false, the default, certificate hostname checking is performed.
    If set to true, no hostname checking is performed and the connection is insecure.
  */
  bool m_tls_bNoHostNameCheck;

  /*!
    SSL_VERIFY_NONE (0): the server will not be verified in any way.
    SSL_VERIFY_PEER (1): the server certificate will be verified and the connection aborted
        if the verification fails.  The default and recommended value is SSL_VERIFY_PEER.
        Using SSL_VERIFY_NONE provides no security.
  */
  int m_tls_cert_reqs;

  std::string m_tls_cafile;   // path to a file containing the PEM encoded trusted CA certificate files.
                              // Either cafile or capath must not be NULL.
  std::string m_tls_capath;   // path to a directory containing the PEM encoded trusted CA certificate files.
                              // See mosquitto.conf for more details on configuring this directory.
                              // Either cafile or capath must not be NULL.
  std::string m_tls_certfile; // path to a file containing the PEM encoded certificate file for this client.
                              // If NULL, keyfile must also be NULL and no client certificate will be used.
  std::string m_tls_keyfile;

  std::string m_tls_pwKeyfile; // Password for keyfile (set only if it is encrypted on disc)

  /*!
    The version of the SSL/TLS protocol to use as a string.  If NULL, the default value is used.
    The default value and the available values depend on the version of openssl that the library
    was compiled against.  For openssl >= 1.0.1, the available options are tlsv1.2, tlsv1.1 and tlsv1,
    with tlv1.2 as the default.  For openssl < 1.0.1, only tlsv1 is available.
  */
  std::string m_tls_version;

  /*!
    A string describing the ciphers available for use.  See the “openssl ciphers” tool for more information.
    If NULL, the default ciphers will be used.
  */
  std::string m_tls_ciphers;

  /*!
    pre-shared-key in hex format with no leading “0x”.
  */
  std::string m_tls_psk;

  /*!
    the identity of this client.
    May be used as the username depending on the server settings.
  */
  std::string m_tls_identity;

  // Last Will setting
  bool m_bWill;               // Set to true to specify last will
  std::string m_will_topic;   // Topic for last will
  int m_will_qos;             // qos for last will
  bool m_will_bretain;        // Enable retain for last will
  std::string m_will_payload; // Payload for last will  

  struct mosquitto *m_mosq; // Handel for connection
  int m_mid;                // Message id

  // Worker thread id
  pthread_t m_tid;
};

#endif