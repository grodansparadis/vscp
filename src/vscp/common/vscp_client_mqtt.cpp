// vscp_client_mqtt.cpp
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
// Copyright (C)2007-2024
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

#ifdef WIN32
#include <pch.h>
#endif

#include "vscp_client_mqtt.h"

#include <pthread.h>
#ifndef WIN32
#include <unistd.h>
#endif

#include <mosquitto.h>

#ifndef WIN32
#if (LIBMOSQUITTO_MAJOR > 1) || (LIBMOSQUITTO_MAJOR == 1 && LIBMOSQUITTO_MINOR >= 6)
#include <mqtt_protocol.h>
#else
// Name change from 1.6 (after 1.5.8)
// As it looks it is not installed by deb script
// #include <mqtt3_protocol.h>
#endif
#endif

#include <guid.h>
#include <vscphelper.h>

#include <deque>
#include <string>

#include <mustache.hpp>
using namespace kainjow::mustache;

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/spdlog.h>

// Forward declaration
static void *
workerThread(void *pObj);

// ----------------------------------------------------------------------------

//                     * * * * Callbacks * * * *

///////////////////////////////////////////////////////////////////////////////
// password_callback
//

/*
  if keyfile is encrypted, set pw_callback to allow your client to pass the
  correct password for decryption.  If set to NULL, the password must be
  entered on the command line.  Your callback must write the password into
  “buf”, which is “size” bytes long.  The return value must be the length
  of the password.  “userdata” will be set to the calling mosquitto instance.
  The mosquitto userdata member variable can be retrieved using mosquitto_userdata.
*/

int
password_callback(char *buf, int size, int rwflag, void *userdata)
{
  vscpClientMqtt *pClient = reinterpret_cast<vscpClientMqtt *>(userdata);

  memset(buf, 0, size);
  strncpy(buf, pClient->getPassword().c_str(), size);
  return (int) strlen(buf);
}

///////////////////////////////////////////////////////////////////////////////
// mqtt_log_callback
//

static void
mqtt_on_log(struct mosquitto *mosq, void *pData, int level, const char *logmsg)
{
  // Check for valid handle
  if (nullptr == mosq) {
    return;
  }

  // Check for a valid object pointer
  if (nullptr == pData) {
    return;
  }

  vscpClientMqtt *pClient = reinterpret_cast<vscpClientMqtt *>(pData);
  spdlog::trace("MQTT CLIENT: mqtt_on_log: {}", logmsg);

  if (nullptr != pClient->m_parentCallbackLog) {
    pClient->m_parentCallbackLog(mosq, pClient->m_pParent, level, logmsg);
  }
}

///////////////////////////////////////////////////////////////////////////////
// mqtt_on_connect
//
// Used for pre 1.6 mosquitto libs
//

static void
mqtt_on_connect(struct mosquitto *mosq, void *pData, int rv)
{

  // Check for valid handle
  if (nullptr == mosq) {
    return;
  }

  // Check for a valid object pointer
  if (nullptr == pData) {
    return;
  }

  vscpClientMqtt *pClient = reinterpret_cast<vscpClientMqtt *>(pData);
  pClient->m_bConnected   = true;

  spdlog::info("MQTT CLIENT: MQTT v3.11 connect: rv={0:X} flags={1:X} {2}", rv, mosquitto_strerror(rv));

  if (nullptr != pClient->m_parentCallbackConnect) {
    pClient->m_parentCallbackConnect(mosq, pClient->m_pParent, rv);
  }
}

///////////////////////////////////////////////////////////////////////////////
// mqtt_on_connect_flags
//

static void
mqtt_on_connect_flags(struct mosquitto *mosq, void *pData, int rv, int flags)
{

  // Check for valid handle
  if (nullptr == mosq) {
    return;
  }

  // Check for a valid object pointer
  if (nullptr == pData) {
    return;
  }

  vscpClientMqtt *pClient = reinterpret_cast<vscpClientMqtt *>(pData);
  pClient->m_bConnected   = true;

  spdlog::info("MQTT CLIENT: MQTT v3.11 connect: rv={0:X} flags={1:X} {2}", rv, flags, mosquitto_strerror(rv));

  if (nullptr != pClient->m_parentCallbackConnect) {
    pClient->m_parentCallbackConnect(mosq, pClient->m_pParent, rv);
  }
}

///////////////////////////////////////////////////////////////////////////////
// mqtt_on_connect_v5
//

#if LIBMOSQUITTO_MAJOR > 1 || (LIBMOSQUITTO_MAJOR == 1 && LIBMOSQUITTO_MINOR >= 6)
static void
mqtt_on_connect_v5(struct mosquitto *mosq, void *pData, int rv, int flags, const mosquitto_property *props)
{
  // Check for valid handle
  if (nullptr == mosq) {
    return;
  }

  // Check for a valid object pointer
  if (nullptr == pData) {
    return;
  }

  vscpClientMqtt *pClient = reinterpret_cast<vscpClientMqtt *>(pData);
  pClient->m_bConnected   = true;

  spdlog::info("MQTT CLIENT: MQTT v5 connect: rv={0:X} flags={1:X} {2}", rv, flags, mosquitto_strerror(rv));

  if (nullptr != pClient->m_parentCallbackConnect) {
    pClient->m_parentCallbackConnect(mosq, pClient->m_pParent, rv);
  }
}
#endif

///////////////////////////////////////////////////////////////////////////////
// mqtt_on_disconnect
//

static void
mqtt_on_disconnect(struct mosquitto *mosq, void *pData, int rv)
{
  // Check for valid handle
  if (nullptr == mosq) {
    return;
  }

  // Check for a valid object pointer
  if (nullptr == pData) {
    return;
  }

  vscpClientMqtt *pClient = reinterpret_cast<vscpClientMqtt *>(pData);
  pClient->m_bConnected   = false;

  spdlog::info("MQTT CLIENT: MQTT v3.11 disconnect: rv={0:X} {1}", rv, mosquitto_strerror(rv));

  if (nullptr != pClient->m_parentCallbackDisconnect) {
    pClient->m_parentCallbackDisconnect(mosq, pClient->m_pParent, rv);
  }
}

///////////////////////////////////////////////////////////////////////////////
// mqtt_on_disconnect_v5
//

#if LIBMOSQUITTO_MAJOR > 1 || (LIBMOSQUITTO_MAJOR == 1 && LIBMOSQUITTO_MINOR >= 6)
static void
mqtt_on_disconnect_v5(struct mosquitto *mosq, void *pData, int rv, const mosquitto_property *props)
{
  // Check for valid handle
  if (nullptr == mosq) {
    return;
  }

  // Check for a valid object pointer
  if (nullptr == pData) {
    return;
  }

  vscpClientMqtt *pClient = reinterpret_cast<vscpClientMqtt *>(pData);
  pClient->m_bConnected   = false;

  spdlog::trace("MQTT CLIENT: MQTT v5 disconnect: rv={0:X} {1}", rv, mosquitto_strerror(rv));

  if (nullptr != pClient->m_parentCallbackDisconnect) {
    pClient->m_parentCallbackDisconnect(mosq, pClient->m_pParent, rv);
  }
}
#endif

///////////////////////////////////////////////////////////////////////////////
// mqtt_on_publish
//

static void
mqtt_on_publish(struct mosquitto *mosq, void *pData, int mid)
{
  // Check for valid handle
  if (nullptr == mosq) {
    return;
  }

  // Check for a valid object pointer
  if (nullptr == pData) {
    return;
  }

  vscpClientMqtt *pClient = reinterpret_cast<vscpClientMqtt *>(pData);
  spdlog::trace("MQTT publish: MQTT v3.11 publish: mid={0:X}", mid);

  if (nullptr != pClient->m_parentCallbackPublish) {
    pClient->m_parentCallbackPublish(mosq, pClient->m_pParent, mid);
  }
}

///////////////////////////////////////////////////////////////////////////////
// mqtt_on_publish_v5
//

#if LIBMOSQUITTO_MAJOR > 1 || (LIBMOSQUITTO_MAJOR == 1 && LIBMOSQUITTO_MINOR >= 6)
static void
mqtt_on_publish_v5(struct mosquitto *mosq, void *pData, int mid, int reason_code, const mosquitto_property *props)
{
  // Check for valid handle
  if (nullptr == mosq) {
    return;
  }

  // Check for a valid object pointer
  if (nullptr == pData) {
    return;
  }

  vscpClientMqtt *pClient = reinterpret_cast<vscpClientMqtt *>(pData);
  spdlog::trace("MQTT publish: MQTT v5 publish: mid={0:X} reason-code={1:X}", mid, reason_code);

  if (nullptr != pClient->m_parentCallbackPublish) {
    pClient->m_parentCallbackPublish(mosq, pClient->m_pParent, mid);
  }
}
#endif

///////////////////////////////////////////////////////////////////////////////
// mqtt_on_message
//

static void
mqtt_on_message(struct mosquitto *mosq, void *pData, const struct mosquitto_message *pMsg)
{
  // Check for valid handle
  if (nullptr == mosq) {
    return;
  }

  // Check for a valid object pointer
  if (nullptr == pData) {
    return;
  }

  // Check for a valid message pointer
  if (nullptr == pMsg) {
    return;
  }

  vscpClientMqtt *pClient = reinterpret_cast<vscpClientMqtt *>(pData);

  if (nullptr != pClient->m_parentCallbackMessage) {
    pClient->m_parentCallbackMessage(mosq, pClient->m_pParent, pMsg);
  }

  std::string payload((const char *) pMsg->payload, pMsg->payloadlen);
  spdlog::trace("MQTT CLIENT: MQTT v3 Message trace: Topic = {0} - Payload: {1}", pMsg->topic, payload);

  if (!pClient->handleMessage(pMsg)) {
    spdlog::trace("MQTT CLIENT: MQTT v3 Message parse failure: Topic = {0} - Payload: {1}", pMsg->topic, payload);
  }
}

///////////////////////////////////////////////////////////////////////////////
// mqtt_on_message_v5
//

#if LIBMOSQUITTO_MAJOR > 1 || (LIBMOSQUITTO_MAJOR == 1 && LIBMOSQUITTO_MINOR >= 6)
static void
mqtt_on_message_v5(struct mosquitto *mosq,
                   void *pData,
                   const struct mosquitto_message *pMsg,
                   const mosquitto_property *props)
{
  // Check for valid handle
  if (nullptr == mosq) {
    return;
  }

  // Check for a valid object pointer
  if (nullptr == pData) {
    return;
  }

  // Check for a valid message pointer
  if (nullptr == pMsg) {
    return;
  }

  std::string payload((const char *) pMsg->payload, pMsg->payloadlen);
  spdlog::trace("MQTT CLIENT: MQTT v5 Message trace: Topic = {0} - Payload: {1}", pMsg->topic, payload);

  vscpClientMqtt *pClient = reinterpret_cast<vscpClientMqtt *>(pData);

  if (nullptr != pClient->m_parentCallbackMessage) {
    pClient->m_parentCallbackMessage(mosq, pClient->m_pParent, pMsg);
  }

  if (!pClient->handleMessage(pMsg)) {
    spdlog::error("MQTT CLIENT: MQTT v5 Message parse failure: Topic = {0} - Payload: {1}", pMsg->topic, payload);
  }
}
#endif

///////////////////////////////////////////////////////////////////////////////
// mqtt_on_subscribe
//

static void
mqtt_on_subscribe(struct mosquitto *mosq, void *pData, int mid, int qos_count, const int *granted_qos)
{
  // Check for valid handle
  if (nullptr == mosq) {
    return;
  }

  // Check for a valid object pointer
  if (nullptr == pData) {
    return;
  }

  vscpClientMqtt *pClient = reinterpret_cast<vscpClientMqtt *>(pData);

  if (nullptr != pClient->m_parentCallbackSubscribe) {
    pClient->m_parentCallbackSubscribe(mosq, pClient->m_pParent, mid, qos_count, granted_qos);
  }
}

///////////////////////////////////////////////////////////////////////////////
// mqtt_on_subscribe_v5
//

#if LIBMOSQUITTO_MAJOR > 1 || (LIBMOSQUITTO_MAJOR == 1 && LIBMOSQUITTO_MINOR >= 6)
static void
mqtt_on_subscribe_v5(struct mosquitto *mosq,
                     void *pData,
                     int mid,
                     int qos_count,
                     const int *granted_qos,
                     const mosquitto_property *props)
{
  // Check for valid handle
  if (nullptr == mosq) {
    return;
  }

  // Check for a valid object pointer
  if (nullptr == pData) {
    return;
  }

  if (nullptr == granted_qos) {
    return;
  }

  vscpClientMqtt *pClient = reinterpret_cast<vscpClientMqtt *>(pData);

  if (nullptr != pClient->m_parentCallbackSubscribe) {
    pClient->m_parentCallbackSubscribe(mosq, pClient->m_pParent, mid, qos_count, granted_qos);
  }
}
#endif

///////////////////////////////////////////////////////////////////////////////
// mqtt_on_unsubscribe
//

static void
mqtt_on_unsubscribe(struct mosquitto *mosq, void *pData, int mid)
{
  // Check for valid handle
  if (nullptr == mosq) {
    return;
  }

  // Check for a valid object pointer
  if (nullptr == pData) {
    return;
  }

  vscpClientMqtt *pClient = reinterpret_cast<vscpClientMqtt *>(pData);

  if (nullptr != pClient->m_parentCallbackUnsubscribe) {
    pClient->m_parentCallbackUnsubscribe(mosq, pClient->m_pParent, mid);
  }
}

///////////////////////////////////////////////////////////////////////////////
// mqtt_on_unsubscribe_v5
//

#if LIBMOSQUITTO_MAJOR > 1 || (LIBMOSQUITTO_MAJOR == 1 && LIBMOSQUITTO_MINOR >= 6)
static void
mqtt_on_unsubscribe_v5(struct mosquitto *mosq, void *pData, int mid, const mosquitto_property *props)
{
  // Check for valid handle
  if (nullptr == mosq) {
    return;
  }

  // Check for a valid object pointer
  if (nullptr == pData) {
    return;
  }

  vscpClientMqtt *pClient = reinterpret_cast<vscpClientMqtt *>(pData);

  if (nullptr != pClient->m_parentCallbackUnsubscribe) {
    pClient->m_parentCallbackUnsubscribe(mosq, pClient->m_pParent, mid);
  }
}
#endif

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// CTor
//

#if LIBMOSQUITTO_MAJOR > 1 || (LIBMOSQUITTO_MAJOR == 1 && LIBMOSQUITTO_MINOR >= 6)
publishTopic::publishTopic(const std::string &topic,
                           enumMqttMsgFormat format,
                           int qos,
                           bool bretain,
                           mosquitto_property *properties)
#else
publishTopic::publishTopic(const std::string &topic, enumMqttMsgFormat format, int qos, bool bretain)
#endif
{
  m_topic   = topic;
  m_qos     = qos;
  m_bRetain = bretain;
  m_bActive = true;
  m_format  = format;
#if LIBMOSQUITTO_MAJOR > 1 || (LIBMOSQUITTO_MAJOR == 1 && LIBMOSQUITTO_MINOR >= 6)
  m_properties = properties;
#endif
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

publishTopic::~publishTopic()
{
#if LIBMOSQUITTO_MAJOR > 1 || (LIBMOSQUITTO_MAJOR == 1 && LIBMOSQUITTO_MINOR >= 6)
  mosquitto_property_free_all(&m_properties);
#endif
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// CTor
//

#if LIBMOSQUITTO_MAJOR > 1 || (LIBMOSQUITTO_MAJOR == 1 && LIBMOSQUITTO_MINOR >= 6)
subscribeTopic::subscribeTopic(const std::string &topic,
                               enumMqttMsgFormat format,
                               int qos,
                               int v5_options,
                               mosquitto_property *properties)
#else
subscribeTopic::subscribeTopic(const std::string &topic, enumMqttMsgFormat format, int qos, int v5_options)
#endif
{
  m_topic      = topic;
  m_qos        = qos;
  m_v5_options = v5_options;
  m_bActive    = true;
  m_format     = format;
#if LIBMOSQUITTO_MAJOR > 1 || (LIBMOSQUITTO_MAJOR == 1 && LIBMOSQUITTO_MINOR >= 6)
  m_properties = properties;
#endif
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

subscribeTopic::~subscribeTopic()
{
#if LIBMOSQUITTO_MAJOR > 1 || (LIBMOSQUITTO_MAJOR == 1 && LIBMOSQUITTO_MINOR >= 6)
  mosquitto_property_free_all(&m_properties);
#endif
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// C-tor
//

vscpClientMqtt::vscpClientMqtt(void)
{
  spdlog::trace("MQTT CLIENT: MQTT CLIENT: constructor vscp_client_mqtt object.");

  m_type = CVscpClient::connType::MQTT; // This is a MQTT client

  // Don't filter away anything
  memset(&m_filter, 0, sizeof(vscpEventFilter));

  m_mapMqttIntOptions["tcp-nodelay"]      = 1;
  m_mapMqttIntOptions["protocol-version"] = 311;
  m_mapMqttIntOptions["receive-maximum"]  = 20;
  m_mapMqttIntOptions["send-maximum"]     = 20;

  m_bConnected          = false;       // Not connected
  m_bJsonMeasurementAdd = true;        // Add measurement block to JSON publish event
  m_bindInterface       = "";          // No bind interface
  m_mosq                = nullptr;     // No mosquitto connection
  m_bRun                = true;        // Run to the Hills...
  m_host                = "localhost"; // tcp://localhost:1883
  m_port                = 1883;        // Default port
  m_clientid            = "";          // No client id set
  m_username            = "";          // No username set
  m_password            = "";          // No password set
  m_keepAlive           = 30;          // 30 seconds for keepalive
  // m_bCleanSession = false;       // Do not start with a clean session

  m_bTLS                 = false;
  m_tls_cafile           = "";
  m_tls_capath           = "";
  m_tls_certfile         = "";
  m_tls_keyfile          = "";
  m_tls_pwKeyfile        = "";
  m_tls_bNoHostNameCheck = false;
  m_tls_cert_reqs        = 1;
  m_tls_version          = "";
  m_tls_ciphers          = "";
  m_tls_psk              = "";
  m_tls_identity         = "";

  // Publish token escapes disabled
  m_bEscapesPubTopics = true;

  // No token maps defined
  m_pmap_class = nullptr;
  m_pmap_type  = nullptr;

  // delay=2, delay_max=10, exponential_backoff=False Delays would be: 2, 4, 6, 8, 10, 10, ...
  m_reconnect_delay               = 2;
  m_reconnect_delay_max           = 10;
  m_reconnect_exponential_backoff = false;

  // Parent callback functionality
  m_pParent                  = nullptr;
  m_parentCallbackLog        = nullptr;
  m_parentCallbackConnect    = nullptr;
  m_parentCallbackDisconnect = nullptr;
  m_parentCallbackPublish    = nullptr;
  m_parentCallbackSubscribe  = nullptr;
  m_parentCallbackMessage    = nullptr;

  // Initialize MQTT
  if (MOSQ_ERR_SUCCESS != mosquitto_lib_init()) {
    spdlog::error("MQTT CLIENT: MQTT CLIENT: init object: Unable to initialize mosquitto library.");
    return;
  }

  pthread_mutex_init(&m_mutexif, nullptr);
}

///////////////////////////////////////////////////////////////////////////////
// D-tor
//

vscpClientMqtt::~vscpClientMqtt()
{
  spdlog::trace("MQTT CLIENT: MQTT CLIENT: destructor vscp_client_mqtt object.");

  // Disconnect if we still are connected
  if (isConnected()) {
    disconnect();
  }

  // Clean up the lib
  mosquitto_lib_cleanup();

  pthread_mutex_destroy(&m_mutexif);

  vscpEvent *pev = nullptr;
  while (m_receiveQueue.size()) {
    pev = m_receiveQueue.front();
    m_receiveQueue.pop_front();
    vscp_deleteEvent_v2(&pev);
  }

  // Delete subscription objects
  for (std::list<subscribeTopic *>::const_iterator it = m_mqtt_subscribeTopicList.begin();
       it != m_mqtt_subscribeTopicList.end();
       ++it) {
    delete (*it);
  }

  m_mqtt_subscribeTopicList.clear();

  // Delete publish objects
  for (std::list<publishTopic *>::const_iterator it = m_mqtt_publishTopicList.begin();
       it != m_mqtt_publishTopicList.end();
       ++it) {
    delete (*it);
  }

  m_mqtt_publishTopicList.clear();
}

///////////////////////////////////////////////////////////////////////////////
// getConfigAsJson
//

std::string
vscpClientMqtt::getConfigAsJson(void)
{
  return m_json.dump();
}

///////////////////////////////////////////////////////////////////////////////
// initFromJson
//
// See header for JSON format
//

bool
vscpClientMqtt::initFromJson(const std::string &config)
{
  json j;

  try {
    j = json::parse(config);

    // Bind address
    if (j.contains("bind")) {
      m_bindInterface = j["bind"].get<std::string>();
      spdlog::debug("MQTT CLIENT: json mqtt init: bind interface set to {}.", m_bindInterface);
    }

    // Host address on form (s)tcp://ip:port
    if (j.contains("host")) {

      m_host = j["host"].get<std::string>();
      spdlog::debug("MQTT CLIENT: json mqtt init: host set to {}.", m_host);

      // Get hostname
      vscp_trim(m_host);
      if (0 == m_host.find("tcp://")) {
        m_host = m_host.substr(6);
        m_bTLS = false;
        spdlog::debug("MQTT CLIENT: json mqtt init: Unsecure connection {}.", m_host);
      }
      else if (0 == m_host.find("stcp://")) {
        m_host = m_host.substr(7);
        m_bTLS = true;
        spdlog::debug("MQTT CLIENT: json mqtt init: Secure connection {}.", m_host);
      }

      // Get port
      size_t pos;
      if (std::string::npos != (pos = m_host.rfind(":"))) {
        m_port = vscp_readStringValue(m_host.substr(pos + 1));
        m_host = m_host.substr(0, pos);
        spdlog::debug("MQTT CLIENT: json mqtt init: port (from host string) set to {}.", m_port);
      }
    }

    // Port
    if (j.contains("port")) {
      m_port = j["port"].get<uint16_t>();
      spdlog::debug("MQTT CLIENT: json mqtt init: port set to {}.", m_port);
    }

    // MQTT options
    if (j.contains("mqtt-options") && j["mqtt-options"].is_object()) {

      json jj = j["mqtt-options"];

      if (jj.contains("tcp-nodelay") && j["tcp-nodelay"].is_number()) {
        m_mapMqttIntOptions["tcp-nodelay"] = jj["tcp-nodelay"].get<int>();
        spdlog::debug("MQTT CLIENT: json mqtt init: tcp-nodelay set to {}.", m_mapMqttIntOptions["tcp-nodelay"]);
      }

      if (jj.contains("protocol-version") && j["protocol-version"].is_number()) {
        m_mapMqttIntOptions["protocol-version"] = jj["protocol-version"].get<int>();
        spdlog::debug("MQTT CLIENT: json mqtt init: protocol-version set to {}.",
                      m_mapMqttIntOptions["protocol-version"]);
      }

      if (jj.contains("receive-maximum") && j["receive-maximum"].is_number()) {
        m_mapMqttIntOptions["receive-maximum"] = jj["receive-maximum"].get<int>();
        spdlog::debug("MQTT CLIENT: json mqtt init: receive-maximum set to {}.",
                      m_mapMqttIntOptions["receive-maximum"]);
      }

      if (jj.contains("send-maximum") && j["send-maximum"].is_number()) {
        m_mapMqttIntOptions["send-maximum"] = jj["send-maximum"].get<int>();
        spdlog::debug("MQTT CLIENT: json mqtt init: send-maximum set to {}.", m_mapMqttIntOptions["send-maximum"]);
      }

      if (jj.contains("ssl-ctx-with-defaults") && j["ssl-ctx-with-defaults"].is_number()) {
        m_mapMqttIntOptions["ssl-ctx-with-defaults"] = jj["ssl-ctx-with-defaults"].get<int>();
        spdlog::debug("MQTT CLIENT: json mqtt init: ssl-ctx-with-defaults set to {}.",
                      m_mapMqttIntOptions["ssl-ctx-with-defaults"]);
      }

      if (jj.contains("tls-ocsp-required") && j["tls-ocsp-required"].is_number()) {
        m_mapMqttIntOptions["tls-ocsp-required"] = jj["tls-ocsp-required"].get<int>();
        spdlog::debug("MQTT CLIENT: json mqtt init: tls-ocsp-required set to {}.",
                      m_mapMqttIntOptions["tls-ocsp-required"]);
      }

      if (jj.contains("tls-use-os-certs") && j["tls-use-os-certs"].is_number()) {
        m_mapMqttIntOptions["tls-use-os-certs"] = jj["tls-use-os-certs"].get<int>();
        spdlog::debug("MQTT CLIENT: json mqtt init: tls-use-os-certs set to {}.",
                      m_mapMqttIntOptions["tls-use-os-certs"]);
      }

      if (jj.contains("tls-engine") && j["tls-engine"].is_string()) {
        m_mapMqttStringOptions["tls-engine"] = jj["tls-engine"].get<std::string>();
        spdlog::debug("MQTT CLIENT: json mqtt init: tls-engine set to {}.", m_mapMqttIntOptions["tls-engine"]);
      }

      if (jj.contains("tls-keyform") && j["tls-keyform"].is_string()) {
        m_mapMqttStringOptions["tls-keyform"] = jj["tls-keyform"].get<std::string>();
        spdlog::debug("MQTT CLIENT: json mqtt init: tls-keyform set to {}.", m_mapMqttIntOptions["tls-keyform"]);
      }

      if (jj.contains("tls-kpass-sha1") && j["tls-kpass-sha1"].is_string()) {
        m_mapMqttStringOptions["tls-kpass-sha1"] = jj["tls-kpass-sha1"].get<std::string>();
        spdlog::debug("MQTT CLIENT: json mqtt init: tls-kpass-sha1 set to {}.", m_mapMqttIntOptions["tls-kpass-sha1"]);
      }

      if (jj.contains("tls-alpn") && j["tls-alpn"].is_string()) {
        m_mapMqttStringOptions["tls-alpn"] = jj["tls-alpn"].get<std::string>();
        spdlog::debug("MQTT CLIENT: json mqtt init: tls-alpn set to {}.", m_mapMqttIntOptions["tls-alpn"]);
      }

      if (jj.contains("bind-address") && j["bind-address"].is_string()) {
        m_mapMqttStringOptions["bind-address"] = jj["bind-address"].get<std::string>();
        spdlog::debug("MQTT CLIENT: json mqtt init: bind-address set to {}.", m_mapMqttIntOptions["bind-address"]);
      }

      // if (jj.contains("ssl-ctx")) {
      //   m_mapMqttVoidOptions["ssl-ctx"] = jj["ssl-ctx"].get<void *>();
      // }
    }
    else if (j.contains("mqtt-options")) {
      // Format is invalid
      spdlog::warn("MQTT CLIENT: json mqtt init: 'mqtt-options' present but not an object.");
    }

    // user
    if (j.contains("user") && j["user"].is_string()) {
      m_username = j["user"].get<std::string>();
      spdlog::debug("MQTT CLIENT: json mqtt init: 'user' set to {}.", m_username);
    }

    // password
    if (j.contains("password") && j["password"].is_string()) {
      m_password = j["password"].get<std::string>();
      spdlog::debug("MQTT CLIENT: json mqtt init: 'password' set to {}.", "*******");
    }

    // Client ID
    if (j.contains("clientid") && j["clientid"].is_string()) {
      m_clientid = j["clientid"].get<std::string>();
      // TODO
      // {rnd} mustasch is replaces with hex random value
      spdlog::debug("MQTT CLIENT: json mqtt init: 'client id' set to {}.", m_clientid);
      if (m_clientid.length() > MQTT_MAX_CLIENTID_LENGTH) {
        spdlog::warn("MQTT CLIENT: json mqtt init: 'client id' is to long {0} length={1} (Standard say max 23 "
                     "characters but longer "
                     "is OK with most brokers).",
                     m_clientid,
                     m_clientid.length());
      }
    }

    // Keep Alive
    if (j.contains("keepalive") && j["keepalive"].is_number()) {
      m_keepAlive = j["keepalive"].get<int>();
      spdlog::debug("MQTT CLIENT: json mqtt init: 'keepalive' Set to {}.", m_keepAlive);
    }

    // Clean Session
    if (j.contains("bcleansession") && j["bcleansession"].is_boolean()) {
      m_bCleanSession = j["bcleansession"].get<bool>();
      spdlog::debug("MQTT CLIENT: json mqtt init: 'bcleansession' Set to {}.", m_bCleanSession);
    }

    // Enable measurement block
    if (j.contains("bjsonmeasurementblock") && j["bjsonmeasurementblock"].is_boolean()) {
      m_bJsonMeasurementAdd = j["bjsonmeasurementblock"].get<bool>();
      spdlog::debug("MQTT CLIENT: json mqtt init: 'bjsonmeasurementblock' Set to {}.", m_bJsonMeasurementAdd);
    }

    // Reconnect
    if (j.contains("reconnect") && j["reconnect"].is_object()) {

      json jj = j["reconnect"];

      if (jj.contains("delay") && j["delay"].is_number()) {
        m_reconnect_delay = jj["delay"].get<int>();
        spdlog::debug("MQTT CLIENT: json mqtt init: 'reconnect delay' Set to {}.", m_reconnect_delay);
      }

      if (jj.contains("delay-max") && j["delay-max"].is_number()) {
        m_reconnect_delay_max = jj["delay-max"].get<int>();
        spdlog::debug("MQTT CLIENT: json mqtt init: 'reconnect delay-max' Set to {}.", m_reconnect_delay_max);
      }

      if (jj.contains("exponential-backoff") && j["exponential-backoff"].is_boolean()) {
        m_reconnect_exponential_backoff = jj["exponential-backoff"].get<bool>();
        spdlog::debug("MQTT CLIENT: json mqtt init: 'reconnect exponential-backoff' Set to {}.",
                      m_reconnect_exponential_backoff);
      }
    }

    // Filter
    if (j.contains("filter") && j["filter"].is_object()) {

      json jj = j["filter"];

      if (jj.contains("priority-filter") && j["priority-filter"].is_number_unsigned()) {
        m_filter.filter_priority = jj["priority-filter"].get<uint8_t>();
        spdlog::debug("MQTT CLIENT: json mqtt init: 'priority-filter' set to {}.", m_filter.filter_priority);
      }

      if (jj.contains("priority-mask") && j["priority-mask"].is_number_unsigned()) {
        m_filter.mask_priority = jj["priority-mask"].get<uint8_t>();
        spdlog::debug("MQTT CLIENT: json mqtt init: 'priority-mask' set to {}.", m_filter.mask_priority);
      }

      if (jj.contains("class-filter") && j["class-filter"].is_number_unsigned()) {
        m_filter.filter_class = jj["class-filter"].get<uint16_t>();
        spdlog::debug("MQTT CLIENT: json mqtt init: 'class-filter' set to {}.", m_filter.filter_class);
      }

      if (jj.contains("class-mask") && j["class-mask"].is_number_unsigned()) {
        m_filter.mask_class = jj["class-mask"].get<uint16_t>();
        spdlog::debug("MQTT CLIENT: json mqtt init: 'class mask' set to {}.", m_filter.mask_class);
      }

      if (jj.contains("type-filter") && j["type-filter"].is_number_unsigned()) {
        m_filter.filter_type = jj["type-filter"].get<uint16_t>();
        spdlog::debug("MQTT CLIENT: json mqtt init: 'type-filter' set to {}.", m_filter.filter_type);
      }

      if (jj.contains("type-mask") && j["type-mask"].is_number_unsigned()) {
        m_filter.mask_type = jj["type-mask"].get<uint16_t>();
        spdlog::debug("MQTT CLIENT: json mqtt init: 'type-mask' set to {}.", m_filter.mask_type);
      }

      if (jj.contains("guid-filter") && j["guid-filter"].is_string()) {
        std::string str = jj["guid-filter"].get<std::string>();
        vscp_getGuidFromStringToArray(m_filter.filter_GUID, str);
        spdlog::debug(
          "MQTT CLIENT: json mqtt init: 'guid-filter' set to "
          "{0:x}:{1:x}:{2:x}:{3:x}:{4:x}:{5:x}:{6:x}:{7:x}:{8:x}:{9:x}:{10:x}:{11:x}:{12:x}:{13:x}:{14:x}:{15:x}.",
          m_filter.filter_GUID[0],
          m_filter.filter_GUID[1],
          m_filter.filter_GUID[2],
          m_filter.filter_GUID[3],
          m_filter.filter_GUID[4],
          m_filter.filter_GUID[5],
          m_filter.filter_GUID[6],
          m_filter.filter_GUID[7],
          m_filter.filter_GUID[8],
          m_filter.filter_GUID[9],
          m_filter.filter_GUID[10],
          m_filter.filter_GUID[11],
          m_filter.filter_GUID[12],
          m_filter.filter_GUID[13],
          m_filter.filter_GUID[14],
          m_filter.filter_GUID[15]);
      }

      if (jj.contains("guid-mask") && j["guid-mask"].is_string()) {
        std::string str = jj["guid-mask"].get<std::string>();
        vscp_getGuidFromStringToArray(m_filter.mask_GUID, str);
        spdlog::debug(
          "MQTT CLIENT: json mqtt init: 'guid-mask' set to "
          "{0:x}:{1:x}:{2:x}:{3:x}:{4:x}:{5:x}:{6:x}:{7:x}:{8:x}:{9:x}:{10:x}:{11:x}:{12:x}:{13:x}:{14:x}:{15:x}.",
          m_filter.mask_GUID[0],
          m_filter.mask_GUID[1],
          m_filter.mask_GUID[2],
          m_filter.mask_GUID[3],
          m_filter.mask_GUID[4],
          m_filter.mask_GUID[5],
          m_filter.mask_GUID[6],
          m_filter.mask_GUID[7],
          m_filter.mask_GUID[8],
          m_filter.mask_GUID[9],
          m_filter.mask_GUID[10],
          m_filter.mask_GUID[11],
          m_filter.mask_GUID[12],
          m_filter.mask_GUID[13],
          m_filter.mask_GUID[14],
          m_filter.mask_GUID[15]);
      }
    }

    // TLS
    if (j.contains("tls") && j["tls"].is_object()) {

      json jj = j["tls"];
      m_bTLS  = true;

      if (jj.contains("cafile") && j["cafile"].is_string()) {
        m_tls_cafile = jj["cafile"].get<std::string>();
        spdlog::debug("MQTT CLIENT: json mqtt init: 'tls cafile' Set to {}.", m_tls_cafile);
      }

      if (jj.contains("capath") && j["capath"].is_string()) {
        m_tls_capath = jj["capath"].get<std::string>();
        spdlog::debug("MQTT CLIENT: json mqtt init: 'tls capath' Set to {}.", m_tls_capath);
      }

      if (jj.contains("certfile") && j["certfile"].is_string()) {
        m_tls_certfile = jj["certfile"].get<std::string>();
        spdlog::debug("MQTT CLIENT: json mqtt init: 'tls certfile' Set to {}.", m_tls_certfile);
      }

      if (jj.contains("keyfile") && j["keyfile"].is_string()) {
        m_tls_keyfile = jj["keyfile"].get<std::string>();
        spdlog::debug("MQTT CLIENT: json mqtt init: 'tls keyfile' Set to {}.", m_tls_keyfile);
      }

      if (jj.contains("pwkeyfile") && j["pwkeyfile"].is_string()) {
        m_tls_pwKeyfile = jj["pwkeyfile"].get<std::string>();
        spdlog::debug("MQTT CLIENT: json mqtt init: 'tls pwkeyfile' Set to {}.", m_tls_pwKeyfile);
      }

      if (jj.contains("no-hostname-checking") && j["no-hostname-checking"].is_boolean()) {
        m_tls_bNoHostNameCheck = jj["no-hostname-checking"].get<bool>();
        spdlog::debug("MQTT CLIENT: json mqtt init: 'tls no-hostname-checking' Set to {}.", m_tls_bNoHostNameCheck);
      }

      if (jj.contains("cert-reqs") && j["cert-reqs"].is_number()) {
        m_tls_cert_reqs = jj["cert-reqs"].get<int>();
        spdlog::debug("MQTT CLIENT: json mqtt init: 'tls cert-reqs' Set to {}.", m_tls_cert_reqs);
      }

      if (jj.contains("version") && j["version"].is_string()) {
        m_tls_version = jj["version"].get<std::string>();
        spdlog::debug("MQTT CLIENT: json mqtt init: 'tls version' Set to {}.", m_tls_version);
      }

      if (jj.contains("ciphers") && j["ciphers"].is_string()) {
        m_tls_ciphers = jj["ciphers"].get<std::string>();
        spdlog::debug("MQTT CLIENT: json mqtt init: 'tls ciphers' Set to {}.", m_tls_ciphers);
      }

      if (jj.contains("psk") && j["psk"].is_string()) {
        m_tls_psk = jj["psk"].get<std::string>();
        spdlog::debug("MQTT CLIENT: json mqtt init: 'tls psk' Set to {}.", m_tls_psk);
      }

      if (jj.contains("psk-identity") && j["psk-identity"].is_string()) {
        m_tls_identity = jj["psk-identity"].get<std::string>();
        spdlog::debug("MQTT CLIENT: json mqtt init: 'tls psk-identity' Set to {}.", m_tls_identity);
      }

      // Both of cafile/capath can not be null
      if (!m_tls_cafile.length() && !m_tls_capath.length()) {
        spdlog::warn("MQTT CLIENT: json mqtt init: 'TLS'  Both cafile and capath must not be NULL. TLS disabled.");
        m_bTLS = false;
      }

      // If certfile == NULL, keyfile must also be NULL and no client certificate will be used.
      if (!m_tls_certfile.length()) {
        spdlog::warn("MQTT CLIENT: json mqtt init: 'TLS'  If certfile == NULL, keyfile must also be NULL and no client "
                     "certificate will be "
                     "used. keyfile set to NULL.");
        m_tls_keyfile = "";
      }

      // If m_tls_keyfile == NULL, certfile must also be NULL and no client certificate will be used.
      if (!m_tls_keyfile.length()) {
        spdlog::warn("MQTT CLIENT: json mqtt init: 'TLS'  If m_tls_keyfile == NULL, certfile must also be NULL and no "
                     "client certificate "
                     "will be used. certfile set to NULL.");
        m_tls_certfile = "";
      }
    }

    // Last will
    if (j.contains("will") && j["will"].is_object()) {

      json jj = j["will"];
      spdlog::debug("MQTT CLIENT: will={}", jj.dump());
      m_bWill = true;

      if (jj.contains("topic") && jj["topic"].is_string()) {
        m_will_topic = jj["topic"].get<std::string>();
        spdlog::debug("MQTT CLIENT: json mqtt init: 'will topic' Set to {}.", m_will_topic);
      }

      if (jj.contains("qos") && jj["qos"].is_number_unsigned()) {
        m_will_qos = jj["qos"].get<uint8_t>();
        spdlog::debug("MQTT CLIENT: json mqtt init: 'will qos' Set to {}.", m_will_qos);
      }

      if (jj.contains("retain") && jj["retain"].is_boolean()) {
        m_will_bretain = jj["retain"].get<bool>();
        spdlog::debug("MQTT CLIENT: json mqtt init: 'will retain' Set to {}.", m_will_bretain);
      }

      if (jj.contains("payload") && jj["payload"].is_string()) {
        m_will_payload = jj["payload"].get<std::string>();
        spdlog::debug("MQTT CLIENT: json mqtt init: 'will payload' Set to {}.", m_will_payload);
      }
    }

    // *****************************************************************
    //                            Subscribe
    // *****************************************************************
    if (j.contains("subscribe") && j["subscribe"].is_array()) {

      json jj = j["subscribe"];

      for (auto const &subobj : jj) {

        if (subobj.is_object()) {

          std::string topic        = "";
          int qos                  = 0;
          int v5_options           = 0;
          enumMqttMsgFormat format = autofmt;

          if (subobj.contains("topic") && subobj["topic"].is_string()) {
            topic = subobj["topic"].get<std::string>();
            vscp_trim(topic);
            if (!topic.size()) {
              spdlog::error("MQTT CLIENT: json mqtt init: Subscribe topic has unallowed length zero");
              continue;
            }
            spdlog::debug("MQTT CLIENT: json mqtt init: 'subscribe topic' Set to {}.", topic);
          }
          else {
            spdlog::error("MQTT CLIENT: json mqtt init: Missing subscription topic. Will skip it");
            continue;
          }

          if (subobj.contains("qos") && subobj["qos"].is_number_unsigned()) {
            qos = subobj["qos"].get<uint8_t>();
            spdlog::debug("MQTT CLIENT: json mqtt init: 'subscribe qos' Set to {}.", qos);
          }

          // Numerically
          if (subobj.contains("v5-options") && subobj["v5-options"].is_number_integer()) {
            v5_options = subobj["v5-options"].get<int>();
            spdlog::debug("MQTT CLIENT: json mqtt init: 'subscribe c5_options' Set to {}.", v5_options);
          }
          // From token
          else if (subobj.contains("v5-options") && subobj["v5-options"].is_string()) {
            std::string str = subobj["v5-options"].get<std::string>();
            vscp_makeUpper(str);
            /*
              0x04
              with this option set, if this client publishes to a topic to which it is subscribed,
              the broker will not publish the message back to the client.
            */
            if (std::string::npos != str.find("NO_LOCAL")) {
#ifndef WIN32
#if LIBMOSQUITTO_MAJOR > 1 || (LIBMOSQUITTO_MAJOR == 1 && LIBMOSQUITTO_MINOR >= 6)
              v5_options |= MQTT_SUB_OPT_NO_LOCAL;
#else
              v5_options |= 0x04;
#endif
#else
              v5_options |= 0x04; // TODO Check if this is the right option
#endif
            }
            /*
              0x08
              with this option set, messages published for this subscription will keep the retain
              flag as was set by the publishing client.  The default behaviour without this option
              set has the retain flag indicating whether a message is fresh/stale.
            */

            if (std::string::npos != str.find("RETAIN_AS_PUBLISHED")) {
#ifndef WIN32
#if LIBMOSQUITTO_MAJOR > 1 || (LIBMOSQUITTO_MAJOR == 1 && LIBMOSQUITTO_MINOR >= 6)
              v5_options |= MQTT_SUB_OPT_RETAIN_AS_PUBLISHED;
#else
              v5_options |= 0x08;
#endif
#else
              v5_options |= 0x08; // TODO Check if this is the right option
#endif
            }
            /*
              0x00
              with this option set, pre-existing retained messages are sent as soon as the subscription
              is made, even if the subscription already exists.  This is the default behaviour, so it is
              not necessary to set this option.
            */

            if (std::string::npos != str.find("SEND_RETAIN_ALWAYS")) {
#ifndef WIN32
#if LIBMOSQUITTO_MAJOR > 1 || (LIBMOSQUITTO_MAJOR == 1 && LIBMOSQUITTO_MINOR >= 6)
              v5_options |= MQTT_SUB_OPT_SEND_RETAIN_ALWAYS;
#else
              v5_options |= 0x00;
#endif
#else
              v5_options |= 0x00; // TODO Check if this is the right option
#endif
            }
            /*
              0x10
              with this option set, pre-existing retained messages for this subscription will be sent when
              the subscription is made, but only if the subscription does not already exist.
            */

            if (std::string::npos != str.find("SEND_RETAIN_NEW")) {
#ifndef WIN32
#if LIBMOSQUITTO_MAJOR > 1 || (LIBMOSQUITTO_MAJOR == 1 && LIBMOSQUITTO_MINOR >= 6)
              v5_options |= MQTT_SUB_OPT_SEND_RETAIN_NEW;
#else
              v5_options |= 0x10;
#endif
#else
              v5_options |= 0x10; // TODO Check if this is the right option
#endif
            }
            /*
              0x20
              with this option set, pre-existing retained messages will never be sent for this subscription.
            */

            if (std::string::npos != str.find("SEND_RETAIN_NEVER")) {
#ifndef WIN32
#if LIBMOSQUITTO_MAJOR > 1 || (LIBMOSQUITTO_MAJOR == 1 && LIBMOSQUITTO_MINOR >= 6)
              v5_options |= MQTT_SUB_OPT_SEND_RETAIN_NEVER;
#else
              v5_options |= 0x20;
#endif
#else
              v5_options |= 0x20; // TODO Check if this is the right option
#endif
            }

            spdlog::debug("MQTT CLIENT: json mqtt init: 'subscribe v5_options' Set to {}.", v5_options);
          }

          if (subobj.contains("format") && subobj["format"].is_string()) {
            std::string str = j["subscribe-format"].get<std::string>();
            vscp_makeLower(str);
            if (std::string::npos != str.find("binary")) {
              spdlog::debug("MQTT CLIENT: json mqtt init: 'subscribe obj format' Set to BINARY.");
              format = binfmt;
            }
            else if (std::string::npos != str.find("string")) {
              spdlog::debug("MQTT CLIENT: json mqtt init: 'subscribe obj format' Set to STRING.");
              format = strfmt;
            }
            else if (std::string::npos != str.find("json")) {
              spdlog::debug("MQTT CLIENT: json mqtt init: 'subscribe obj format' Set to JSON.");
              format = jsonfmt;
            }
            else if (std::string::npos != str.find("xml")) {
              spdlog::debug("MQTT CLIENT: json mqtt init: 'subscribe obj format' Set to XML.");
              format = xmlfmt;
            }
            else if (std::string::npos != str.find("auto")) {
              spdlog::debug("MQTT CLIENT: json mqtt init: 'subscribe obj format' Set to AUTO.");
              format = autofmt;
            }
            else {
              spdlog::error("MQTT CLIENT: json mqtt init: 'subscribe obj format' Ivalid value. Set to AUTO.");
              format = autofmt;
            }
          }

          addSubscription(topic, format, qos, v5_options);

        } // object
      }   // for
    }     // sub

    if (j.contains("bescape-pub-topics") && j["bescape-pub-topics"].is_boolean()) {
      m_bEscapesPubTopics = j["bescape-pub-topics"].get<bool>();
      spdlog::debug("MQTT CLIENT: json mqtt init: 'bescape-pub-topics' Set to {}.", m_bEscapesPubTopics);
    }

    // User escapes m_mapUserEscapes
    if (j.contains("user-escapes") && j["user-escapes"].is_object()) {
      for (auto it = j["user-escapes"].begin(); it != j["user-escapes"].end(); ++it) {
        m_mapUserEscapes[it.key()] = it.value();
        spdlog::debug("MQTT CLIENT: json mqtt init: 'user-escapes' Set to {0}={1}.", it.key(), it.value());
      }
    }

    // *****************************************************************
    //                            Publish
    // *****************************************************************

    if (j.contains("publish") && j["publish"].is_array()) {

      json jj = j["publish"];

      for (auto const &pubobj : jj) {

        if (pubobj.is_object()) {

          std::string topic        = "";
          int qos                  = 0;       // Default
          bool bretain             = false;   // Default
          enumMqttMsgFormat format = jsonfmt; // Default

          if (pubobj.contains("topic") && pubobj["topic"].is_string()) {
            topic = pubobj["topic"].get<std::string>();
            vscp_trim(topic);
            if (!topic.size()) {
              spdlog::error("MQTT CLIENT: json mqtt init: Publish topic has unallowed length zero");
              continue;
            }
            spdlog::debug("MQTT CLIENT: json mqtt init: publish 'topic' Set to {}.", topic);
          }
          else {
            spdlog::error("MQTT CLIENT: json mqtt init: Missing publish topic. Will skip it");
            continue;
          }

          if (pubobj.contains("qos") && pubobj["qos"].is_number_unsigned()) {
            qos = pubobj["qos"].get<uint8_t>();
            spdlog::debug("MQTT CLIENT: json mqtt init: publish 'qos' Set to {}.", qos);
          }

          if (pubobj.contains("bretain") && pubobj["bretain"].is_boolean()) {
            bretain = pubobj["bretain"].get<bool>();
            spdlog::debug("MQTT CLIENT: json mqtt init: publish 'bretain' Set to {}.", bretain);
          }

          if (pubobj.contains("format") && pubobj["format"].is_number()) {

            format = static_cast<enumMqttMsgFormat>(pubobj["format"].get<int>());

            // vscp_makeLower(str);
            //  std::string str;
            //  if (std::string::npos != str.find("binary")) {
            //    spdlog::debug("MQTT CLIENT: json mqtt init: 'publish obj format' Set to BINARY.");
            //    format = binfmt;
            //  }
            //  else if (std::string::npos != str.find("string")) {
            //    spdlog::debug("MQTT CLIENT: json mqtt init: 'publish obj format' Set to STRING.");
            //    format = strfmt;
            //  }
            //  else if (std::string::npos != str.find("json")) {
            //    spdlog::debug("MQTT CLIENT: json mqtt init: 'publish obj format' Set to JSON.");
            //    format = jsonfmt;
            //  }
            //  else if (std::string::npos != str.find("xml")) {
            //    spdlog::debug("MQTT CLIENT: json mqtt init: 'publish obj format' Set to XML.");
            //    format = xmlfmt;
            //  }
            //  else {
            //    spdlog::error("MQTT CLIENT: json mqtt init: 'publish obj format' Ivalid value. Set to JSON.");
            //    format = jsonfmt;
            //  }
          }

          addPublish(topic, format, qos, bretain);

        } // obj
      }   // for
    }     // pub

    // v5
    if (j.contains("v5") && j["v5"].is_object()) {
      json jj = j.contains("v5");
      if (jj.contains("user-properties") && jj["user-properties"].is_object()) {
        for (auto it = jj.begin(); it != jj.end(); ++it) {
          m_mapMqttProperties[it.key()] = it.value();
          spdlog::debug("MQTT CLIENT: json mqtt init: v5 'user-properties' Set to {0}={1}.", it.key(), it.value());
        }
      }
    }
  }
  catch (...) {
    spdlog::error("MQTT CLIENT: json init: JSON parsing error.");
    return false;
  }

  // Save the configuration
  m_json = j;

  return true;
}

/////////////////////////////////////////////////////////////////////////////
// handleMessage
//
// Handle incoming message
//

bool
vscpClientMqtt::handleMessage(const struct mosquitto_message *pmsg)
{
  enumMqttMsgFormat format = autofmt;
  vscpEvent ev;
  vscpEventEx ex;

  // Check pointers
  if (nullptr == pmsg) {
    spdlog::error("MQTT CLIENT: handleMessage: No message to handle.");
    return false;
  }

  // Must/Should be a payload
  if (!pmsg->payloadlen) {
    spdlog::error("MQTT CLIENT: handleMessage: No payload to handle.");
    return false;
  }

  // We need to find the subscribe channel
  if (nullptr != pmsg->topic) {
    for (std::list<publishTopic *>::const_iterator it = m_mqtt_publishTopicList.begin();
         it != m_mqtt_publishTopicList.end();
         ++it) {

      publishTopic *ppublish = (*it);
      // if topics match
      if (!strstr(ppublish->getTopic().c_str(), pmsg->topic)) {
        format = ppublish->getFormat();
      }
    }
  }
  else {
    // Topic is nill we need to find format

    // If First char of payload...
    //     ...is zero - Binary payload
    //     ...is "{"  - JSON payload
    //     ...is "<"  - XML payload
    //     else string payload

    if (0 == ((uint8_t *) pmsg->payload)[0]) {
      // Binary
      format = binfmt;
    }
    else if ('{' == ((char *) pmsg->payload)[0]) {
      // JSON
      format = jsonfmt;
    }
    else if ('<' == ((char *) pmsg->payload)[0]) {
      // XML
      format = xmlfmt;
    }
    else {
      // String: send 0,20,3,,,,0:1:2:3:4:5:6:7:8:9:10:11:12:13:14:15,0,1,35<CR><LF>
      std::string s = (char *) pmsg->payload;
      size_t n      = std::count(s.begin(), s.end(), ',');
      if (n < 6) {
        // This is not a VSCP event on string format
        spdlog::trace("MQTT CLIENT: Payload is not a VSCP event.");
        return false;
      }
      format = strfmt;
    }
  }

  // Binary payload will result in zero length string as first character
  // is a zero.
  std::string payload((const char *) pmsg->payload, pmsg->payloadlen);

  if (jsonfmt == format) {

    if (!vscp_convertJSONToEvent(&ev, payload)) {
      spdlog::trace("MQTT CLIENT: JSON->Event conversion failed. Payload is not VSCP event.");
      return false;
    }

    if (!vscp_convertEventToEventEx(&ex, &ev)) {
      spdlog::trace("MQTT CLIENT: JSON->EventEx conversion failed. Payload is not VSCP event.");
      return false;
    }

    // If callback is defined send event
    if (nullptr != m_evcallback) {
      m_evcallback(&ev, m_callbackObject);
    }
    else if (nullptr != m_excallback) {
      m_excallback(&ex, m_callbackObject);
    }
    else {
      // Put event in input queue
      vscpEvent *pEvent = new vscpEvent;
      if (nullptr == pEvent) {
        spdlog::critical("MQTT CLIENT: Memory problem.");
        return false;
      }

      pEvent->pdata = nullptr;

      if (!vscp_copyEvent(pEvent, &ev)) {
        delete pEvent;
        return 0;
      }

      // Save event in incoming queue
      if (m_receiveQueue.size() < MQTT_MAX_INQUEUE_SIZE) {
        m_receiveQueue.push_back(pEvent);
      }
    }
  }
  else if (xmlfmt == format) {
    if (!vscp_convertXMLToEvent(&ev, payload)) {
      spdlog::trace("MQTT CLIENT: XML->Event conversion failed. Payload is not VSCP event.");
      return false;
    }

    if (!vscp_convertEventToEventEx(&ex, &ev)) {
      spdlog::trace("MQTT CLIENT: XML->EventEx conversion failed. Payload is not VSCP event.");
      return false;
    }

    // If callback is defined send event
    if (nullptr != m_evcallback) {
      m_evcallback(&ev, m_callbackObject);
    }
    else if (nullptr != m_excallback) {
      m_excallback(&ex, m_callbackObject);
    }
    else {
      // Put event in input queue
      vscpEvent *pEvent = new vscpEvent;
      if (nullptr == pEvent) {
        spdlog::critical("MQTT CLIENT: Memory problem.");
        return false;
      }
      pEvent->pdata = nullptr;

      if (!vscp_copyEvent(pEvent, &ev)) {
        spdlog::critical("MQTT CLIENT: Memory problem.");
        delete pEvent;
        return false;
      }

      // Save event in incoming queue
      if (m_receiveQueue.size() < MQTT_MAX_INQUEUE_SIZE) {
        m_receiveQueue.push_back(pEvent);
      }
    }
  }
  else if (strfmt == format) {
    if (!vscp_convertStringToEvent(&ev, payload)) {
      spdlog::trace("MQTT CLIENT: str->Event conversion failed. Payload is not VSCP event.");
      return false;
    }

    if (!vscp_convertEventToEventEx(&ex, &ev)) {
      spdlog::trace("MQTT CLIENT: str->EventEx conversion failed. Payload is not VSCP event.");
      return false;
    }

    // If callback is defined send event
    if (nullptr != m_evcallback) {
      m_evcallback(&ev, m_callbackObject);
    }
    else if (nullptr != m_excallback) {
      m_excallback(&ex, m_callbackObject);
    }
    else {
      // Put event in input queue
      vscpEvent *pEvent = new vscpEvent;
      if (nullptr == pEvent) {
        spdlog::critical("MQTT CLIENT: Memory problem.");
        return false;
      }
      pEvent->pdata = nullptr;

      if (!vscp_copyEvent(pEvent, &ev)) {
        delete pEvent;
        spdlog::critical("MQTT CLIENT: Memory problem.");
        return false;
      }

      // Save event in incoming queue
      if (m_receiveQueue.size() < MQTT_MAX_INQUEUE_SIZE) {
        m_receiveQueue.push_back(pEvent);
      }
    }
  }
  else if (binfmt == format) {

    // Binary frame starts offset one in payload (after zero marker byte)
    if (!vscp_getEventFromFrame(&ev, (const uint8_t *) pmsg->payload + 1, pmsg->payloadlen)) {
      spdlog::trace("MQTT CLIENT: bin->Event conversion failed. Payload is not VSCP event.");
      return false;
    }

    if (!vscp_convertEventToEventEx(&ex, &ev)) {
      spdlog::trace("MQTT CLIENT: bin->EventEx conversion failed. Payload is not VSCP event.");
      return false;
    }

    // If callback is defined send event
    if (nullptr != m_evcallback) {
      m_evcallback(&ev, m_callbackObject);
    }
    else if (nullptr != m_excallback) {
      m_excallback(&ex, m_callbackObject);
    }
    else {
      // Put event in input queue
      vscpEvent *pEvent = new vscpEvent;
      if (nullptr == pEvent) {
        spdlog::critical("MQTT CLIENT: Memory problem.");
        return false;
      }

      pEvent->pdata = nullptr;

      if (!vscp_copyEvent(pEvent, &ev)) {
        spdlog::critical("MQTT CLIENT: Memory problem.");
        delete pEvent;
        return false;
      }

      // Save event in incoming queue
      if (m_receiveQueue.size() < MQTT_MAX_INQUEUE_SIZE) {
        m_receiveQueue.push_back(pEvent);
      }
    }
  }

  return true;
}

/////////////////////////////////////////////////////////////////////////////
// init
//
// Initialize MQTT sub system
//

bool
vscpClientMqtt::init(void)
{
  int rv = 0;

  if (m_clientid.length()) {

    /*
      if clientid if contains {{rd}} replace
      with 16 random hex characters
    */
    {
      std::string clientid_template = m_clientid;
      mustache subtemplate{ clientid_template };
      data data;

      std::string strsalt;
      vscp_getSaltHex(strsalt, 8);

      data.set("rnd", strsalt);
      m_clientid = subtemplate.render(data);
    }

    m_mosq = mosquitto_new(m_clientid.c_str(), m_bCleanSession, this);
  }
  else {
    m_bCleanSession = true; // Must be true without id
    m_mosq          = mosquitto_new(nullptr, m_bCleanSession, this);
  }

  // v5
#if (LIBMOSQUITTO_MAJOR > 1) || (LIBMOSQUITTO_MAJOR == 1 && LIBMOSQUITTO_MINOR >= 6)
  if (m_mapMqttIntOptions["protocol-version"] >= 500) {
    mosquitto_int_option(m_mosq, MOSQ_OPT_PROTOCOL_VERSION, MQTT_PROTOCOL_V5);
  }
#endif

  if (nullptr == m_mosq) {
    if (ENOMEM == errno) {
      spdlog::error("MQTT CLIENT: Failed to create new mosquitto session (out of memory).");
    }
    else if (EINVAL == errno) {
      spdlog::error("MQTT CLIENT: Failed to create new mosquitto session (invalid parameters).");
    }
    return false;
  }

  // Callbacks
  if (m_mapMqttIntOptions["protocol-version"] >= 500) {
    mosquitto_log_callback_set(m_mosq, mqtt_on_log);
#if LIBMOSQUITTO_MAJOR > 1 || (LIBMOSQUITTO_MAJOR == 1 && LIBMOSQUITTO_MINOR >= 6)
    mosquitto_connect_v5_callback_set(m_mosq, mqtt_on_connect_v5);
    mosquitto_disconnect_v5_callback_set(m_mosq, mqtt_on_disconnect_v5);
    mosquitto_message_v5_callback_set(m_mosq, mqtt_on_message_v5);
    mosquitto_publish_v5_callback_set(m_mosq, mqtt_on_publish_v5);
    mosquitto_subscribe_v5_callback_set(m_mosq, mqtt_on_subscribe_v5);
    mosquitto_unsubscribe_v5_callback_set(m_mosq, mqtt_on_unsubscribe_v5);
#endif
  }
  else {
    mosquitto_log_callback_set(m_mosq, mqtt_on_log);
#if (LIBMOSQUITTO_MAJOR > 1) || (LIBMOSQUITTO_MAJOR == 1 && LIBMOSQUITTO_MINOR >= 6)
    mosquitto_connect_with_flags_callback_set(m_mosq, mqtt_on_connect_flags);
#else
    mosquitto_connect_callback_set(m_mosq, mqtt_on_connect);
#endif
    mosquitto_disconnect_callback_set(m_mosq, mqtt_on_disconnect);
    mosquitto_message_callback_set(m_mosq, mqtt_on_message);
    mosquitto_publish_callback_set(m_mosq, mqtt_on_publish);
    mosquitto_subscribe_callback_set(m_mosq, mqtt_on_subscribe);
    mosquitto_unsubscribe_callback_set(m_mosq, mqtt_on_unsubscribe);
  }

  if (MOSQ_ERR_SUCCESS != mosquitto_reconnect_delay_set(m_mosq,
                                                        m_reconnect_delay,
                                                        m_reconnect_delay_max,
                                                        m_reconnect_exponential_backoff)) {
    spdlog::warn("MQTT CLIENT: Failed to set reconnect settings.");
  }

  std::string strTopic;
  if (m_bEscapesPubTopics) {

    std::string topic_template = m_will_topic;
    mustache subtemplate{ topic_template };
    data data;

    data.set("clientid", m_clientid);
    data.set("user", m_username);
    data.set("host", m_host);

    data.set("ifguid", m_ifguid.getAsString());
    data.set("srvguid", m_srvguid.getAsString());

    for (int i = 0; i < 15; i++) {
      data.set(vscp_str_format("ifguid[%d]", i), vscp_str_format("%d", m_ifguid.getAt(i)));
    }

    for (int i = 0; i < 15; i++) {
      data.set(vscp_str_format("xifguid[%d]", i), vscp_str_format("%02X", m_ifguid.getAt(i)));
    }

    for (int i = 0; i < 15; i++) {
      data.set(vscp_str_format("srvguid[%d]", i), vscp_str_format("%d", m_srvguid.getAt(i)));
    }

    for (int i = 0; i < 15; i++) {
      data.set(vscp_str_format("xsrvguid[%d]", i), vscp_str_format("%02X", m_srvguid.getAt(i)));
    }

    // Add user escapes like "driver-name"= "some-driver";
    for (auto const &item : m_mapUserEscapes) {
      data.set(item.first, item.second);
    }

    strTopic = subtemplate.render(data);
  }

  // Set 'last will' if any is defined
  if (m_will_payload.length() && m_will_topic.length()) {
    if (m_mapMqttIntOptions["protocol-version"] >= 500) {
#if LIBMOSQUITTO_MAJOR > 1 || (LIBMOSQUITTO_MAJOR == 1 && LIBMOSQUITTO_MINOR >= 6)
      if (MOSQ_ERR_SUCCESS != mosquitto_will_set_v5(m_mosq,
                                                    strTopic.c_str(),
                                                    (int) m_will_payload.length(),
                                                    m_will_payload.c_str(),
                                                    m_will_qos,
                                                    m_will_bretain,
                                                    nullptr)) {
#else
      if (MOSQ_ERR_SUCCESS != mosquitto_will_set(m_mosq,
                                                 strTopic.c_str(),
                                                 m_will_payload.length(),
                                                 m_will_payload.c_str(),
                                                 m_will_qos,
                                                 m_will_bretain)) {
#endif
        spdlog::warn("MQTT CLIENT: Failed to set last will. rv={0} {1}", rv, mosquitto_strerror(rv));
      }
    }
    else {
      if (MOSQ_ERR_SUCCESS != mosquitto_will_set(m_mosq,
                                                 strTopic.c_str(),
                                                 (int) m_will_payload.length(),
                                                 m_will_payload.c_str(),
                                                 m_will_qos,
                                                 m_will_bretain)) {
        spdlog::error("MQTT CLIENT: Failed to set last will. rv={0} {1}", rv, mosquitto_strerror(rv));
      }
    }
  }

  // Set username/password if defined
  if (m_username.length()) {
    int rv;
    if (MOSQ_ERR_SUCCESS != (rv = mosquitto_username_pw_set(m_mosq, m_username.c_str(), m_password.c_str()))) {
      spdlog::error("MQTT CLIENT: mosquitto_username_pw_set failed. rv={0} {1}", rv, mosquitto_strerror(rv));
    }
  }

  //                          * * * Set options * * *

  // tcp-nodelay
#if LIBMOSQUITTO_MAJOR >= 2
  if (MOSQ_ERR_SUCCESS !=
      (rv = mosquitto_int_option(m_mosq, MOSQ_OPT_TCP_NODELAY, m_mapMqttIntOptions["tcp-nodelay"]))) {
    spdlog::error("MQTT CLIENT: Failed to set option MOSQ_OPT_TCP_NODELAY. rv={0} {1}", rv, mosquitto_strerror(rv));
  }
#endif

  // version
#if LIBMOSQUITTO_MAJOR > 1 || (LIBMOSQUITTO_MAJOR == 1 && LIBMOSQUITTO_MINOR >= 6)
  {
    int ver;
    switch (m_mapMqttIntOptions["protocol-version"]) {
      case 310:
        ver = MQTT_PROTOCOL_V31;
        break;

      case 500:
        ver = MQTT_PROTOCOL_V5;
        break;

      case 311:
      default:
        ver = MQTT_PROTOCOL_V311;
        break;
    };

    if (MOSQ_ERR_SUCCESS != (rv = mosquitto_int_option(m_mosq, MOSQ_OPT_PROTOCOL_VERSION, ver))) {
      spdlog::error("MQTT CLIENT: Failed to set option MOSQ_OPT_PROTOCOL_VERSION. rv={0} {1}",
                    rv,
                    mosquitto_strerror(rv));
    }
  }

  // receive-maximum
  if (MOSQ_ERR_SUCCESS !=
      (rv = mosquitto_int_option(m_mosq, MOSQ_OPT_RECEIVE_MAXIMUM, m_mapMqttIntOptions["receive-maximum"]))) {
    spdlog::error("MQTT CLIENT: Failed to set option MOSQ_OPT_RECEIVE_MAXIMUM. rv={0} {1}", rv, mosquitto_strerror(rv));
  }

  // send-maximum
  if (MOSQ_ERR_SUCCESS !=
      (rv = mosquitto_int_option(m_mosq, MOSQ_OPT_SEND_MAXIMUM, m_mapMqttIntOptions["send-maximum"]))) {
    spdlog::error("MQTT CLIENT: Failed to set option MOSQ_OPT_SEND_MAXIMUM. rv={0} {1}", rv, mosquitto_strerror(rv));
  }
#endif

  //                          * * * TLS * * *

  if (m_bTLS) {
    if (MOSQ_ERR_SUCCESS != (rv = mosquitto_tls_set(m_mosq,
                                                    m_tls_cafile.length() ? m_tls_cafile.c_str() : NULL,
                                                    m_tls_capath.length() ? m_tls_capath.c_str() : NULL,
                                                    m_tls_certfile.length() ? m_tls_certfile.c_str() : NULL,
                                                    m_tls_keyfile.length() ? m_tls_keyfile.c_str() : NULL,
                                                    password_callback))) {
      spdlog::error("MQTT CLIENT: Failed to set mosquitto tls. rv={0} {1}", rv, mosquitto_strerror(rv));
    }

    if (MOSQ_ERR_SUCCESS != (rv = mosquitto_tls_insecure_set(m_mosq, m_tls_bNoHostNameCheck))) {
      spdlog::error("MQTT CLIENT: Failed to set mosquitto tls insecure. rv={0} {1}", rv, mosquitto_strerror(rv));
    }

    if (MOSQ_ERR_SUCCESS != (rv = mosquitto_tls_opts_set(m_mosq,
                                                         m_tls_cert_reqs,
                                                         m_tls_version.length() ? m_tls_version.c_str() : NULL,
                                                         m_tls_ciphers.length() ? m_tls_ciphers.c_str() : NULL))) {
      spdlog::error("MQTT CLIENT: Failed to set mosquitto tls options. rv={0} {1}", rv, mosquitto_strerror(rv));
    }
  }
  else {
    if (m_tls_psk.length()) {
      if (MOSQ_ERR_SUCCESS != (rv = mosquitto_tls_psk_set(m_mosq,
                                                          m_tls_psk.c_str(),
                                                          m_tls_identity.c_str(),
                                                          m_tls_ciphers.length() ? m_tls_ciphers.c_str() : NULL))) {
        spdlog::error("MQTT CLIENT: Failed to set mosquitto tls psk. rv={0} {1}", rv, mosquitto_strerror(rv));
      }
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// addSubscription
//

int
#if LIBMOSQUITTO_MAJOR > 1 || (LIBMOSQUITTO_MAJOR == 1 && LIBMOSQUITTO_MINOR >= 6)
vscpClientMqtt::addSubscription(const std::string strTopicSub,
                                enumMqttMsgFormat format,
                                int qos,
                                int v5_options,
                                mosquitto_property *properties)
#else
vscpClientMqtt::addSubscription(const std::string strTopicSub, enumMqttMsgFormat format, int qos, int v5_options)
#endif
{
#if LIBMOSQUITTO_MAJOR > 1 || (LIBMOSQUITTO_MAJOR == 1 && LIBMOSQUITTO_MINOR >= 6)
  subscribeTopic *pTopic = new subscribeTopic(strTopicSub, format, qos, v5_options, properties);
#else
  subscribeTopic *pTopic = new subscribeTopic(strTopicSub, format, qos, v5_options);
#endif
  m_mqtt_subscribeTopicList.push_back(pTopic);
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// addPublish
//

int
#if LIBMOSQUITTO_MAJOR > 1 || (LIBMOSQUITTO_MAJOR == 1 && LIBMOSQUITTO_MINOR >= 6)
vscpClientMqtt::addPublish(const std::string strTopicPub,
                           enumMqttMsgFormat format,
                           int qos,
                           bool bRetain,
                           mosquitto_property *properties)
#else
vscpClientMqtt::addPublish(const std::string strTopicPub, enumMqttMsgFormat format, int qos, bool bRetain)
#endif
{
#if LIBMOSQUITTO_MAJOR > 1 || (LIBMOSQUITTO_MAJOR == 1 && LIBMOSQUITTO_MINOR >= 6)
  publishTopic *pTopic = new publishTopic(strTopicPub, format, qos, bRetain, properties);
#else
  publishTopic *pTopic = new publishTopic(strTopicPub, format, qos, bRetain);
#endif
  m_mqtt_publishTopicList.push_back(pTopic);
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// connect
//

int
vscpClientMqtt::connect(void)
{
  int rv;

  if (!init()) {
    return VSCP_ERROR_NOT_CONNECTED;
  }

  if (m_bindInterface.length()) {
#if LIBMOSQUITTO_MAJOR > 1 || (LIBMOSQUITTO_MAJOR == 1 && LIBMOSQUITTO_MINOR >= 6)
    if (m_mapMqttIntOptions["protocol-version"] >= 500) {
      rv = mosquitto_connect_bind_v5(m_mosq, m_host.c_str(), m_port, m_keepAlive, m_bindInterface.c_str(), nullptr);
    }
    else {
      rv = mosquitto_connect_bind(m_mosq, m_host.c_str(), m_port, m_keepAlive, m_bindInterface.c_str());
    }
#else
    rv = mosquitto_connect_bind(m_mosq, m_host.c_str(), m_port, m_keepAlive, m_bindInterface.c_str());
#endif
  }
  else {
    rv = mosquitto_connect(m_mosq, m_host.c_str(), m_port, m_keepAlive);
  }

  if (MOSQ_ERR_SUCCESS != rv) {
    spdlog::error("MQTT CLIENT: Failed to connect to MQTT remote host. rv={0} {1}", rv, mosquitto_strerror(rv));
    return VSCP_ERROR_NOT_CONNECTED;
  }

  // mosquitto_threaded_set(m_mosq, true);

  // Start the worker loop
  rv = mosquitto_loop_start(m_mosq);
  if (MOSQ_ERR_SUCCESS != rv) {
    spdlog::error("MQTT CLIENT: Failed to start mosquitto worker loop. rv={0} {1}", rv, mosquitto_strerror(rv));
    mosquitto_disconnect(m_mosq);
    return VSCP_ERROR_ERROR;
  }

  // We send an empty payload if will is defined
  if (m_will_bretain && m_will_payload.length() && m_will_topic.length()) {

    std::string strTopic;
    if (m_bEscapesPubTopics) {

      std::string topic_template = m_will_topic;
      mustache subtemplate{ topic_template };
      data data;

      data.set("clientid", m_clientid);
      data.set("user", m_username);
      data.set("host", m_host);

      data.set("ifguid", m_ifguid.getAsString());
      data.set("srvguid", m_srvguid.getAsString());

      for (int i = 0; i < 15; i++) {
        data.set(vscp_str_format("ifguid[%d]", i), vscp_str_format("%d", m_ifguid.getAt(i)));
      }

      for (int i = 0; i < 15; i++) {
        data.set(vscp_str_format("ifguid[%d]", i), vscp_str_format("%d", m_ifguid.getAt(i)));
      }

      for (int i = 0; i < 15; i++) {
        data.set(vscp_str_format("srvguid[%d]", i), vscp_str_format("%d", m_srvguid.getAt(i)));
      }

      for (int i = 0; i < 15; i++) {
        data.set(vscp_str_format("xsrvguid[%d]", i), vscp_str_format("%d", m_srvguid.getAt(i)));
      }

      // Add user escapes like "driver-name"= "some-driver";
      for (auto const &item : m_mapUserEscapes) {
        data.set(item.first, item.second);
      }

      strTopic = subtemplate.render(data);
    }

    spdlog::trace("MQTT CLIENT: Publish will: Topic: {0} Payload: empty qos=1 retain=true", strTopic);

    if (MOSQ_ERR_SUCCESS != (rv = mosquitto_publish(m_mosq,
                                                    NULL, // msg id
                                                    strTopic.c_str(),
                                                    0,
                                                    NULL,
                                                    1,
                                                    true))) {
      spdlog::error("MQTT CLIENT: mosquitto_publish (will) failed. rv={0} {1}", rv, mosquitto_strerror(rv));
    }
  }

  // Only subscribe if subscription topic is defined
  for (std::list<subscribeTopic *>::const_iterator it = m_mqtt_subscribeTopicList.begin();
       it != m_mqtt_subscribeTopicList.end();
       ++it) {

    subscribeTopic *psubtopic = (*it);

    /*!
      if the subscription is not active we do not subscribe.
      Normally all are active but tools like vscp-works-qt (session)
      can inactivate sessions temporarily.  This is never saved
      persistently.
    */
    if (!psubtopic->isActive()) {
      continue;
    }

    // Fix subscribe topics
    mustache subtemplate{ psubtopic->getTopic() };
    data data;
    // data.set("guid", m_guid.getAsString());
    std::string subscribe_topic = subtemplate.render(data);

    // Subscribe to specified topic
    rv = mosquitto_subscribe(m_mosq, nullptr, subscribe_topic.c_str(), psubtopic->getQos());
    if (MOSQ_ERR_SUCCESS != rv) {
      spdlog::error("MQTT CLIENT: Failed to subscribed to topic '{0}' - rv={1} {2}.",
                    subscribe_topic,
                    rv,
                    mosquitto_strerror(rv));
    }
  }

  // Start worker thread if a callback has been defined
  if ((nullptr != m_evcallback) || (nullptr != m_excallback)) {
    int rv = pthread_create(&m_tid, nullptr, workerThread, this);
    switch (rv) {

      case EAGAIN:
        spdlog::error(
          "MQTT CLIENT: Failed to start MQTT callback thread - Insufficient resources to create another thread.");
        break;

      case EINVAL:
        spdlog::error("MQTT CLIENT: Failed to start MQTT callback thread - Invalid settings in attr");
        break;

      case EPERM:
        spdlog::error("MQTT CLIENT: Failed to start MQTT callback thread - No permission to set the scheduling policy");
        break;

      default:
        spdlog::debug("MQTT CLIENT: Started MQTT callback thread");
        break;
    }
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// disconnect
//

int
vscpClientMqtt::disconnect(void)
{
  m_bConnected = false;
  m_bRun       = false;
  int rv;

  spdlog::debug("MQTT CLIENT: Enter disconnect.");

  if ((nullptr == m_evcallback) && (nullptr == m_excallback)) {
    pthread_join(m_tid, nullptr);
  }

  // Disconnect from MQTT broker}
  rv = mosquitto_disconnect(m_mosq);
  if (MOSQ_ERR_SUCCESS != rv) {
    spdlog::error("MQTT CLIENT: mosquitto_disconnect failed. rv={0} {1}", rv, mosquitto_strerror(rv));
  }

  // stop the worker loop
  rv = mosquitto_loop_stop(m_mosq, false);
  if (MOSQ_ERR_SUCCESS != rv) {
    spdlog::error("MQTT CLIENT: mosquitto_loop_stop failed. rv={0} {1}", rv, mosquitto_strerror(rv));
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// isConnected
//

bool
vscpClientMqtt::isConnected(void)
{
  return m_bConnected;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int
vscpClientMqtt::send(vscpEvent &ev)
{
  std::string str;
  uint8_t payload[1024];
  size_t lenPayload = 0;
  int rv;

  // Publish to each defined topic
  for (std::list<publishTopic *>::const_iterator it = m_mqtt_publishTopicList.begin();
       it != m_mqtt_publishTopicList.end();
       ++it) {

    publishTopic *ppublish = (*it);

    /*!
      if the publish item is not active we do not publish.
      Normally all are active but tools like vscp-works-qt (session)
      can inactivate sessions temporarily.  This is never saved
      persistently.
    */
    if (!ppublish->isActive()) {
      continue;
    }

    memset(payload, 0, sizeof(payload));

    if (ppublish->getFormat() == jsonfmt) {

      std::string strPayload;

      if (!vscp_convertEventToJSON(strPayload, &ev)) {
        return VSCP_ERROR_PARAMETER;
      }

      // If functionality is enable in configuration
      // add measurement info to JSON object
      //
      // "measurement" : {
      //     "value" : 1.23,
      //     "unit" : 0,
      //     "sensorindex" : 1,
      //     "zone" : 11,
      //     "subzone" : 22
      // }
      //

      if (vscp_isMeasurement(&ev) && m_bJsonMeasurementAdd) {

        double value = 0;
        if (!vscp_getMeasurementAsDouble(&value, &ev)) {
          spdlog::error("MQTT CLIENT: sendEvent: Failed to convert measurement event to value.");
        }
        else {
          try {
            auto j = json::parse(strPayload);

            j["measurement"]["value"]       = value;
            j["measurement"]["unit"]        = vscp_getMeasurementUnit(&ev);
            j["measurement"]["sensorindex"] = vscp_getMeasurementSensorIndex(&ev);
            j["measurement"]["zone"]        = vscp_getMeasurementZone(&ev);
            j["measurement"]["subzone"]     = vscp_getMeasurementSubZone(&ev);

            strPayload = j.dump();
            strncpy((char *) payload, strPayload.c_str(), sizeof(payload));
          }
          catch (...) {
            spdlog::error("MQTT CLIENT: sendEvent: Failed to add measurement info to event.");
          }
        } // OK to insert extra info
      }   // is measurement

      lenPayload = strPayload.length();
      strncpy((char *) payload, strPayload.c_str(), lenPayload);

    } // JSON

    else if (ppublish->getFormat() == xmlfmt) {
      std::string strPayload;
      if (!vscp_convertEventToXML(strPayload, &ev)) {
        return VSCP_ERROR_PARAMETER;
      }
      lenPayload = strPayload.length();
      strncpy((char *) payload, strPayload.c_str(), lenPayload);
    }
    else if (ppublish->getFormat() == strfmt) {
      std::string strPayload;
      if (!vscp_convertEventToString(strPayload, &ev)) {
        return VSCP_ERROR_PARAMETER;
      }
      lenPayload = strPayload.length();
      strncpy((char *) payload, strPayload.c_str(), lenPayload);
    }
    else if (ppublish->getFormat() == binfmt) {
      lenPayload = vscp_getFrameSizeFromEvent(&ev) + 1;

      // Write event to frame
      if (!vscp_writeEventToFrame(payload + 1, sizeof(payload), 0, &ev)) {
        return VSCP_ERROR_PARAMETER;
      }
    }
    else {
      return VSCP_ERROR_NOT_SUPPORTED;
    }

    std::string strTopic = ppublish->getTopic();

    // Fix publish topic escapes

    if (m_bEscapesPubTopics) {
      std::string topic_template = ppublish->getTopic();
      mustache subtemplate{ topic_template };
      data data;
      cguid evguid(ev.GUID); // Event GUID

      // Event GUID
      data.set("guid", evguid.getAsString());

      data.set("guid.msb", vscp_str_format("%d", evguid.getAt(0)));
      data.set("guid.lsb", vscp_str_format("%d", evguid.getMSB()));

      data.set("xguid.msb", vscp_str_format("%02X", evguid.getAt(0)));
      data.set("xguid.lsb", vscp_str_format("%02X", evguid.getMSB()));

      // Sever GUID
      data.set("srvguid", m_srvguid.getAsString());

      data.set("srvguid.msb", vscp_str_format("%d", m_srvguid.getAt(0)));
      data.set("srvguid.lsb", vscp_str_format("%d", m_srvguid.getMSB()));

      data.set("xsrvguid.msb", vscp_str_format("%02X", m_srvguid.getAt(0)));
      data.set("xsrvguid.lsb", vscp_str_format("%02X", m_srvguid.getMSB()));

      // Interface GUID
      data.set("ifguid", m_ifguid.getAsString());

      data.set("ifguid.msb", vscp_str_format("%d", m_ifguid.getAt(0)));
      data.set("ifguid.lsb", vscp_str_format("%d", m_ifguid.getMSB()));

      data.set("xifguid.msb", vscp_str_format("%02X", m_ifguid.getAt(0)));
      data.set("xifguid.lsb", vscp_str_format("%02X", m_ifguid.getMSB()));

      // GUID positions
      for (int i = 0; i < 15; i++) {
        data.set(vscp_str_format("guid[%d]", i), vscp_str_format("%d", evguid.getAt(i)));
        data.set(vscp_str_format("xguid[%d]", i), vscp_str_format("%02X", evguid.getAt(i)));
        data.set(vscp_str_format("srvguid[%d]", i), vscp_str_format("%d", m_srvguid.getAt(i)));
        data.set(vscp_str_format("xsrvguid[%d]", i), vscp_str_format("%02X", m_srvguid.getAt(i)));
        data.set(vscp_str_format("ifguid[%d]", i), vscp_str_format("%d", m_ifguid.getAt(i)));
        data.set(vscp_str_format("xifguid[%d]", i), vscp_str_format("%02X", m_ifguid.getAt(i)));
      }

      // Event data
      data.set("sizedata", vscp_str_format("%d", ev.sizeData));
      data.set("xsizedata", vscp_str_format("%04X", ev.sizeData));

      if ((nullptr != ev.pdata) && ev.sizeData) {
        for (int i = 0; i < ev.sizeData; i++) {
          data.set(vscp_str_format("data[%d]", i), vscp_str_format("%d", ev.pdata[i]));
          data.set(vscp_str_format("xdata[%d]", i), vscp_str_format("%02X", ev.pdata[i]));
        }
      }

      data.set("nickname", vscp_str_format("%d", evguid.getNicknameID()));
      data.set("class", vscp_str_format("%d", ev.vscp_class));
      data.set("type", vscp_str_format("%d", ev.vscp_type));

      data.set("xclass", vscp_str_format("%02x", ev.vscp_class));
      data.set("xtype", vscp_str_format("%02x", ev.vscp_type));

      if (nullptr != m_pmap_class) {
        data.set("class-token", (*m_pmap_class)[ev.vscp_class]);
      }

      if (nullptr != m_pmap_type) {
        data.set("type-token", (*m_pmap_type)[((ev.vscp_class << 16) + ev.vscp_type)]);
      }

      data.set("head", vscp_str_format("%d", ev.head));
      data.set("xhead", vscp_str_format("%04X", ev.head));
      data.set("obid", vscp_str_format("%ul", ev.obid));
      data.set("xobid", vscp_str_format("%08X", ev.obid));
      data.set("timestamp", vscp_str_format("%ul", ev.timestamp));
      data.set("xtimestamp", vscp_str_format("%08X", ev.timestamp));

      std::string dt;
      vscp_getDateStringFromEvent(dt, &ev);
      data.set("datetime", dt);
      data.set("year", vscp_str_format("%d", ev.year));
      data.set("month", vscp_str_format("%d", ev.month));
      data.set("day", vscp_str_format("%d", ev.day));
      data.set("hour", vscp_str_format("%d", ev.hour));
      data.set("minute", vscp_str_format("%d", ev.minute));
      data.set("second", vscp_str_format("%d", ev.second));

      data.set("local-datetime", dt);
      data.set("local-year", vscp_str_format("%d", ev.year));
      data.set("local-month", vscp_str_format("%d", ev.month));
      data.set("local-day", vscp_str_format("%d", ev.day));
      data.set("local-hour", vscp_str_format("%d", ev.hour));
      data.set("local-minute", vscp_str_format("%d", ev.minute));
      data.set("local-second", vscp_str_format("%d", ev.second));

      data.set("clientid", m_clientid);
      data.set("user", m_username);
      data.set("host", m_host);

      switch (ppublish->getFormat()) {
        case jsonfmt:
          str = "json";
          break;

        case xmlfmt:
          str = "xml";
          break;

        case strfmt:
          str = "string";
          break;

        case binfmt:
          str = "binary";
          break;

        case autofmt:
          str = "auto";
          break;
      }
      data.set("fmtpublish", str);

      // Add user escapes like "driver-name"= "some-driver";
      for (auto const &item : m_mapUserEscapes) {
        data.set(item.first, item.second);
      }

      strTopic = subtemplate.render(data);
    }

    spdlog::trace("MQTT CLIENT: sendEvent: Publish send ev: Topic: {0} qos={1} retain={2}",
                  strTopic,
                  ppublish->getQos(),
                  ppublish->getRetain());

    spdlog::trace("MQTT send; len=%d QOS=%d retain=%s\n",
                  (int) lenPayload,
                  ppublish->getQos(),
                  (ppublish->getRetain() ? "true" : "false"));

    if (MOSQ_ERR_SUCCESS != (rv = mosquitto_publish(m_mosq,
                                                    NULL, // msg id
                                                    strTopic.c_str(),
                                                    (int) lenPayload,
                                                    payload,
                                                    ppublish->getQos(),
                                                    ppublish->getRetain()))) {
      spdlog::error("MQTT CLIENT: sendEvent: mosquitto_publish (ev) failed. rv={0} {1}", rv, mosquitto_strerror(rv));
      printf("mosquitto_publish: %s\n", mosquitto_strerror(rv));
    }

  } // for each topic

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int
vscpClientMqtt::send(vscpEventEx &ex)
{
  std::string str;
  uint8_t payload[1024];
  size_t lenPayload = 0;
  int rv;

  // Only subscribe if subscription topic is defined
  for (std::list<publishTopic *>::const_iterator it = m_mqtt_publishTopicList.begin();
       it != m_mqtt_publishTopicList.end();
       ++it) {

    publishTopic *ppublish = (*it);

    memset(payload, 0, sizeof(payload));

    if (ppublish->getFormat() == jsonfmt) {

      std::string strPayload;

      if (!vscp_convertEventExToJSON(strPayload, &ex)) {
        return VSCP_ERROR_PARAMETER;
      }

      // If functionality is enable in configuration
      // add measurement info to JSON object
      //
      // "measurement" : {
      //     "value" : 1.23,
      //     "unit" : 0,
      //     "sensorindex" : 1,
      //     "zone" : 11,
      //     "subzone" : 22
      // }
      //

      if (vscp_isMeasurementEx(&ex) && m_bJsonMeasurementAdd) {

        double value = 0;
        if (!vscp_getMeasurementAsDoubleEx(&value, &ex)) {
          spdlog::error("MQTT CLIENT: sendEvent: Failed to convert event to value.");
        }
        else {
          try {
            auto j = json::parse(strPayload);

            j["measurement"]["value"]       = value;
            j["measurement"]["unit"]        = vscp_getMeasurementUnitEx(&ex);
            j["measurement"]["sensorindex"] = vscp_getMeasurementSensorIndexEx(&ex);
            j["measurement"]["zone"]        = vscp_getMeasurementZoneEx(&ex);
            j["measurement"]["subzone"]     = vscp_getMeasurementSubZoneEx(&ex);

            strPayload = j.dump();
            strncpy((char *) payload, strPayload.c_str(), sizeof(payload));
          }
          catch (...) {
            spdlog::error("MQTT CLIENT: sendEvent: Failed to add measurement info to event.");
          }
        } // OK to insert extra info
      }   // is measurement

      lenPayload = strPayload.length();
      strncpy((char *) payload, strPayload.c_str(), sizeof(payload));

    } // JSON
    else if (ppublish->getFormat() == xmlfmt) {
      std::string strPayload;
      if (!vscp_convertEventExToXML(strPayload, &ex)) {
        return VSCP_ERROR_PARAMETER;
      }
      lenPayload = strPayload.length();
      strncpy((char *) payload, strPayload.c_str(), sizeof(payload));
    }
    else if (ppublish->getFormat() == strfmt) {
      std::string strPayload;
      if (!vscp_convertEventExToString(strPayload, &ex)) {
        return VSCP_ERROR_PARAMETER;
      }
      lenPayload = strPayload.length();
      strncpy((char *) payload, strPayload.c_str(), sizeof(payload));
    }
    else if (ppublish->getFormat() == binfmt) {
      lenPayload = vscp_getFrameSizeFromEventEx(&ex) + 1;

      // Write event to frame
      if (!vscp_writeEventExToFrame(payload + 1, sizeof(payload), 0, &ex)) {
        return VSCP_ERROR_PARAMETER;
      }
    }
    else {
      return VSCP_ERROR_NOT_SUPPORTED;
    }

    std::string strTopic = ppublish->getTopic();

    // Fix publish topic escapes

    if (m_bEscapesPubTopics) {
      std::string topic_template = ppublish->getTopic();
      mustache subtemplate{ topic_template };
      data data;
      cguid evguid(ex.GUID); // Event GUID

      // Event GUID
      data.set("guid", evguid.getAsString());

      data.set("guid.msb", vscp_str_format("%d", evguid.getAt(0)));
      data.set("guid.lsb", vscp_str_format("%d", evguid.getMSB()));

      data.set("xguid.msb", vscp_str_format("%02X", evguid.getAt(0)));
      data.set("xguid.lsb", vscp_str_format("%02X", evguid.getMSB()));

      // Sever GUID
      data.set("srvguid", m_srvguid.getAsString());

      data.set("srvguid.msb", vscp_str_format("%d", m_srvguid.getAt(0)));
      data.set("srvguid.lsb", vscp_str_format("%d", m_srvguid.getMSB()));

      data.set("xsrvguid.msb", vscp_str_format("%02X", m_srvguid.getAt(0)));
      data.set("xsrvguid.lsb", vscp_str_format("%02X", m_srvguid.getMSB()));

      // Interface GUID
      data.set("ifguid", m_ifguid.getAsString());

      data.set("ifguid.msb", vscp_str_format("%d", m_ifguid.getAt(0)));
      data.set("ifguid.lsb", vscp_str_format("%d", m_ifguid.getMSB()));

      data.set("xifguid.msb", vscp_str_format("%02X", m_ifguid.getAt(0)));
      data.set("xifguid.lsb", vscp_str_format("%02X", m_ifguid.getMSB()));

      // GUID positions
      for (int i = 0; i < 15; i++) {
        data.set(vscp_str_format("guid[%d]", i), vscp_str_format("%d", evguid.getAt(i)));
        data.set(vscp_str_format("xguid[%d]", i), vscp_str_format("%02X", evguid.getAt(i)));
        data.set(vscp_str_format("srvguid[%d]", i), vscp_str_format("%d", m_srvguid.getAt(i)));
        data.set(vscp_str_format("xsrvguid[%d]", i), vscp_str_format("%02X", m_srvguid.getAt(i)));
        data.set(vscp_str_format("ifguid[%d]", i), vscp_str_format("%d", m_ifguid.getAt(i)));
        data.set(vscp_str_format("xifguid[%d]", i), vscp_str_format("%02X", m_ifguid.getAt(i)));
      }

      data.set("sizedata", vscp_str_format("%d", ex.sizeData));
      data.set("xsizedata", vscp_str_format("%04X", ex.sizeData));

      if (ex.sizeData) {
        for (int i = 0; i < ex.sizeData; i++) {
          data.set(vscp_str_format("data[%d]", i), vscp_str_format("%d", ex.data[i]));
        }
      }

      if (ex.sizeData) {
        for (int i = 0; i < ex.sizeData; i++) {
          data.set(vscp_str_format("xdata[%d]", i), vscp_str_format("%02X", ex.data[i]));
        }
      }

      for (int i = 0; i < 15; i++) {
        data.set(vscp_str_format("ifguid[%d]", i), vscp_str_format("%d", m_ifguid.getAt(i)));
      }

      data.set("nickname", vscp_str_format("%d", evguid.getNicknameID()));
      data.set("class", vscp_str_format("%d", ex.vscp_class));
      data.set("type", vscp_str_format("%d", ex.vscp_type));

      for (int i = 0; i < 15; i++) {
        data.set(vscp_str_format("xifguid[%d]", i), vscp_str_format("%02X", m_ifguid.getAt(i)));
      }

      data.set("xclass", vscp_str_format("%02x", ex.vscp_class));
      data.set("xtype", vscp_str_format("%02x", ex.vscp_type));

      if (nullptr != m_pmap_class) {
        data.set("class-token", (*m_pmap_class)[ex.vscp_class]);
      }

      if (nullptr != m_pmap_type) {
        data.set("type-token", (*m_pmap_type)[((ex.vscp_class << 16) + ex.vscp_type)]);
      }

      data.set("head", vscp_str_format("%d", ex.head));
      data.set("xhead", vscp_str_format("%04X", ex.head));
      data.set("obid", vscp_str_format("%ul", ex.obid));
      data.set("xobid", vscp_str_format("%08X", ex.obid));
      data.set("timestamp", vscp_str_format("%ul", ex.timestamp));
      data.set("xtimestamp", vscp_str_format("%08X", ex.timestamp));

      std::string dt;
      vscp_getDateStringFromEventEx(dt, &ex);
      data.set("datetime", dt);
      data.set("year", vscp_str_format("%d", ex.year));
      data.set("month", vscp_str_format("%d", ex.month));
      data.set("day", vscp_str_format("%d", ex.day));
      data.set("hour", vscp_str_format("%d", ex.hour));
      data.set("minute", vscp_str_format("%d", ex.minute));
      data.set("second", vscp_str_format("%d", ex.second));

      data.set("local-datetime", dt);
      data.set("local-year", vscp_str_format("%d", ex.year));
      data.set("local-month", vscp_str_format("%d", ex.month));
      data.set("local-day", vscp_str_format("%d", ex.day));
      data.set("local-hour", vscp_str_format("%d", ex.hour));
      data.set("local-minute", vscp_str_format("%d", ex.minute));
      data.set("local-second", vscp_str_format("%d", ex.second));

      data.set("clientid", m_clientid);
      data.set("user", m_username);
      data.set("host", m_host);

      switch (ppublish->getFormat()) {
        case jsonfmt:
          str = "json";
          break;

        case xmlfmt:
          str = "xml";
          break;

        case strfmt:
          str = "string";
          break;

        case binfmt:
          str = "binary";
          break;

        case autofmt:
          str = "auto";
          break;
      }
      data.set("fmtpublish", str);

      // Add user escapes like "driver-name"= "some-driver";
      for (auto const &item : m_mapUserEscapes) {
        data.set(item.first, item.second);
      }

      strTopic = subtemplate.render(data);
    }

    spdlog::trace("MQTT CLIENT: sendEvent: Publish send ex: Topic: {0} qos={1} retain={2}",
                  strTopic,
                  /*(unsigned char *)payload,*/
                  ppublish->getQos(),
                  ppublish->getRetain());

    if (MOSQ_ERR_SUCCESS != (rv = mosquitto_publish(m_mosq,
                                                    NULL, // msg id
                                                    strTopic.c_str(),
                                                    (int) lenPayload,
                                                    payload,
                                                    ppublish->getQos(),
                                                    ppublish->getRetain()))) {
      spdlog::error("MQTT CLIENT: sendEvent: mosquitto_publish (ex) failed. rv={0} {1}", rv, mosquitto_strerror(rv));
    }

  } // for each topic

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int
vscpClientMqtt::receive(vscpEvent &ev)
{
  // int rv;
  vscpEvent *pev = nullptr;

  if (m_receiveQueue.size()) {

    pev = m_receiveQueue.front();
    m_receiveQueue.pop_front();
    if (nullptr == pev)
      return VSCP_ERROR_MEMORY;

    if (!vscp_copyEvent(&ev, pev)) {
      return VSCP_ERROR_MEMORY;
    }

    vscp_deleteEvent_v2(&pev);
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int
vscpClientMqtt::receive(vscpEventEx &ex)
{
  // int rv;
  vscpEvent *pev = nullptr;

  if (m_receiveQueue.size()) {

    pev = m_receiveQueue.front();
    m_receiveQueue.pop_front();
    if (nullptr == pev)
      return VSCP_ERROR_MEMORY;

    if (!vscp_convertEventToEventEx(&ex, pev)) {
      return VSCP_ERROR_MEMORY;
    }

    vscp_deleteEvent_v2(&pev);
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setfilter
//

int
vscpClientMqtt::setfilter(vscpEventFilter &filter)
{
  // Copy in filter
  memcpy(&m_filter, &filter, sizeof(vscpEventFilter));
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getcount
//

int
vscpClientMqtt::getcount(uint16_t *pcount)
{
  if (nullptr == pcount)
    return VSCP_ERROR_INVALID_POINTER;
  *pcount = (uint16_t) m_receiveQueue.size();
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// clear
//

int
vscpClientMqtt::clear(void)
{
  m_receiveQueue.clear();
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getVersion
//

int
vscpClientMqtt::getversion(uint8_t *pmajor, uint8_t *pminor, uint8_t *prelease, uint8_t *pbuild)
{
  // Check pointers
  if (nullptr == pmajor) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  if (nullptr == pminor) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  if (nullptr == prelease) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  if (nullptr == pbuild) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  int v1, v2, v3;
  mosquitto_lib_version(&v1, &v2, &v3);
  spdlog::debug("MQTT CLIENT: getversion: {}.{}.{}.{}", v1, v2, v3, 0);

  *pmajor   = v1;
  *pminor   = v2;
  *prelease = v3;
  *pbuild   = 0;

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getinterfaces
//

int
vscpClientMqtt::getinterfaces(std::deque<std::string> &iflist)
{
  // No interfaces available
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getwcyd
//

int
vscpClientMqtt::getwcyd(uint64_t &wcyd)
{
  wcyd = VSCP_SERVER_CAPABILITY_NONE; // No capabilities
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setCallback
//

int
vscpClientMqtt::setCallback(LPFNDLL_EV_CALLBACK m_evcallback, void *pData)
{
  // Can not be called when connected
  if (m_bConnected) {
    return VSCP_ERROR_ERROR;
  }

  CVscpClient::setCallback(m_evcallback, pData);

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setCallback
//

int
vscpClientMqtt::setCallback(LPFNDLL_EX_CALLBACK m_excallback, void *pData)
{
  // Can not be called when connected
  if (m_bConnected) {
    return VSCP_ERROR_ERROR;
  }

  CVscpClient::setCallback(m_excallback, pData);

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// clearRetain4Topic
//

int
vscpClientMqtt::clearRetain4Topic(std::string &strTopic)
{
  int rv = VSCP_ERROR_SUCCESS;
  if (MOSQ_ERR_SUCCESS != (rv = mosquitto_publish(m_mosq,
                                                  NULL, // msg id
                                                  strTopic.c_str(),
                                                  0,
                                                  nullptr,
                                                  0,
                                                  true))) {
    spdlog::error("MQTT CLIENT: sendEvent: mosquitto_publish (ev) failed. rv={0} {1}", rv, mosquitto_strerror(rv));
    rv = VSCP_ERROR_ERROR;
  }
  return rv;
}

#ifdef WIN32
static void
win_usleep(__int64 usec)
{
  HANDLE timer;
  LARGE_INTEGER ft;

  ft.QuadPart = -(10 * usec); // Convert to 100 nanosecond interval, negative value indicates relative time

  timer = CreateWaitableTimer(nullptr, TRUE, nullptr);
  SetWaitableTimer(timer, &ft, 0, nullptr, nullptr, 0);
  WaitForSingleObject(timer, INFINITE);
  CloseHandle(timer);
}
#endif

///////////////////////////////////////////////////////////////////////////////
// Callback workerthread
//
// This thread call the appropriate callback when events are received
//

static void *
workerThread(void *pObj)
{
  uint8_t guid[]          = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  vscpClientMqtt *pClient = (vscpClientMqtt *) pObj;
  //  if (nullptr == pif) return nullptr;

  while (pClient->m_bRun) {

    //  pthread_mutex_lock(&pif->m_mutexif);

    // Check if there are events to fetch
    // int cnt;
    /* if ((cnt = pClient->m_canalif.CanalDataAvailable())) {

        while (cnt) {
            canalMsg msg;
            if ( CANAL_ERROR_SUCCESS ==
    pClient->m_canalif.CanalReceive(&msg) ) { if ( nullptr !=
    pClient->m_evcallback ) { vscpEvent ev; if
    (vscp_convertCanalToEvent(&ev, &msg, guid) ) {
                        pClient->m_evcallback(ev);
                    }
                }
                if ( nullptr != pClient->m_excallback ) {
                    vscpEventEx ex;
                    if (vscp_convertCanalToEventEx(&ex,
                                                    &msg,
                                                    guid) ) {
                        pClient->m_excallback(ex);
                    }
                }
            }
            cnt--;
        }

    } */

    //  pthread_mutex_unlock(&pif->m_mutexif);
#ifndef WIN32
    usleep(200);
#else
    win_usleep(200);
#endif
  }

  return nullptr;
}
