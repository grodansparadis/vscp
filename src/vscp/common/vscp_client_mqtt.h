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

#include "vscp_client_base.h"
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

class publishTopic {

public:
  publishTopic(const std::string &topic = "", int qos = 0, bool bretain = false);
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

class subscribeTopic {

public:
  subscribeTopic(const std::string &topic = "", int qos = 0);
  ~subscribeTopic();

  /// Getters/Setters for topic
  std::string getTopic(void) { return m_topic; };
  void setTopic(const std::string topic) { m_topic = topic; };

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
  vscpClientMqtt(uint16_t requested_mqtt_version = 311, const std::string &bindInterface = "");
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
           const std::string &clientId    = "",
           const std::string &strUserName = "",
           const std::string &strPassword = "",
           int keepAliveInterval          = 20,
           bool bCleanSession             = false);

  /*!
      Handle incoming message
      @param pmsg Incoming MQTT message
      @return true on success, false on failure.
  */
  bool handleMessage(const struct mosquitto_message *pmsg);

  // Init MQTT host
  // void setMqttHost(const std::string host = "tcp://localhost:1883")
  //         { strncpy(m_host, host.c_str(), sizeof(m_host)); };

  // // Init MQTT user
  // void setMqttUser(const std::string user)
  //         { strncpy(m_username, user.c_str(), sizeof(m_username)); };

  // // Init MQTT password
  // void setMqttPassword(const std::string password)
  //         { strncpy(m_password, password.c_str(), sizeof(m_password)); };

  // Init MQTT cleansession
  // void setMqttCleanSession(const bool bCleanSession = true) { m_bCleanSession = bCleanSession; };

  // // Init MQTT keepalive
  // void setMqttKeepAlive(const int keepalive) { m_bTLS = true; m_keepalive = keepalive; };

  // // Init MQTT CaFile
  // void setMqttCaFile(const std::string cafile) { m_bTLS = true; m_cafile = cafile; };

  // // Init MQTT CaPath
  // void setMqttCaPath(const std::string capath) { m_bTLS = true; m_capath = capath; };

  // // Init MQTT CertFile
  // void setMqttCertFile(const std::string certfile) { m_bTLS = true; m_certfile = certfile; };

  // // Init MQTT KeyFile
  // void setMqttKeyFile(const std::string keyfile) { m_bTLS = true; m_keyfile = keyfile; };

  // // Init MQTT PwKeyFile
  // void setMqttPwKeyFile(const std::string pwkeyfile) { m_bTLS = true; m_pwKeyfile = pwkeyfile; };

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
  int addPublish(const std::string strTopicPub, int qos = 0, bool bRetain = false);

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
  virtual bool initFromJson(const std::string &config);

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

  // enumMqttMsgFormat m_format;         // Format for mqtt events (JSON/XML)

  LPFNDLL_EV_CALLBACK m_evcallback; // Event callback
  LPFNDLL_EX_CALLBACK m_excallback; // Event ex callback

  // If no callback is defined received events are connected in
  // this queue
  std::deque<vscpEvent *> m_receiveQueue;

private:
  /*!
      Initialize MQTT
  */
  int init_mqtt();

  std::list<subscribeTopic> m_mqtt_subscribe; // Subscribe topic templates
  std::list<publishTopic> m_mqtt_publish;     // Publish topic templates

  vscpEventFilter m_filter; // Receive filter

  /*!
      MQTT version
      ------------
      Version 3.10 - 310
      Version 3.11 - 311
      Version 5 - 500
  */
  uint16_t m_mqtt_version;

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