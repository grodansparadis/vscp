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
// Copyright © 2007-2021
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
#include <windows.h>
#endif

#include "vscp_client_mqtt.h"

#include <pthread.h>
#ifndef WIN32
#include <unistd.h>
#endif
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
  strcpy(buf, "secret");
  return strlen(buf);
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
  spdlog::trace("MQTT log: {}", logmsg);
}

///////////////////////////////////////////////////////////////////////////////
// mqtt_on_connect
//

static void
mqtt_on_connect(struct mosquitto *mosq, void *pData, int rv, int flags)
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

  spdlog::trace("MQTT v3.11 connect: rv={0:X} flags={1:X}", rv, flags);
}

///////////////////////////////////////////////////////////////////////////////
// mqtt_on_connect_v5
//

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

  spdlog::trace("MQTT v5 connect: rv={0:X} flags={1:X}", rv, flags);
}

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

  spdlog::trace("MQTT v3.11 disconnect: rv={0:X}", rv);
}

///////////////////////////////////////////////////////////////////////////////
// mqtt_on_disconnect_v5
//

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

  spdlog::trace("MQTT v5 disconnect: rv={0:X}", rv);
}

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
  spdlog::trace("MQTT v3.11 publish: mid={0:X}", mid);
}

///////////////////////////////////////////////////////////////////////////////
// mqtt_on_publish_v5
//

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
  spdlog::trace("MQTT v5 publish: mid={0:X} reason-code={1:X}", mid, reason_code);
}

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

  vscpClientMqtt *pObj = reinterpret_cast<vscpClientMqtt *>(pData);
  if (!pObj->handleMessage(pMsg)) {}

  std::string payload((const char *) pMsg->payload, pMsg->payloadlen);
  spdlog::error("MQTT Message: Topic = [{}] - Payload: [{}]", pMsg->topic, payload);
}

///////////////////////////////////////////////////////////////////////////////
// mqtt_on_message_v5
//

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

  vscpClientMqtt *pClient = reinterpret_cast<vscpClientMqtt *>(pData);
  std::string payload((const char *) pMsg->payload, pMsg->payloadlen);

  spdlog::error("MQTT Message: Topic = [{}] - Payload: [{}]", pMsg->topic, payload);
}

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
}

///////////////////////////////////////////////////////////////////////////////
// mqtt_on_subscribe_v5
//

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
}

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
}

///////////////////////////////////////////////////////////////////////////////
// mqtt_on_unsubscribe_v5
//

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
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// CTor
//

#if LIBMOSQUITTO_MAJOR>1 || (LIBMOSQUITTO_MAJOR==1 && LIBMOSQUITTO_MINOR>=6)  
publishTopic::publishTopic(const std::string &topic, int qos, bool bretain, mosquitto_property *properties)
#else
publishTopic::publishTopic(const std::string &topic, int qos, bool bretain)
#endif 
{
  m_topic   = topic;
  m_qos     = qos;
  m_bRetain = bretain;
#if LIBMOSQUITTO_MAJOR>1 || (LIBMOSQUITTO_MAJOR==1 && LIBMOSQUITTO_MINOR>=6)   
  m_properties = properties;
#endif
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

publishTopic::~publishTopic()
{
#if LIBMOSQUITTO_MAJOR>1 || (LIBMOSQUITTO_MAJOR==1 && LIBMOSQUITTO_MINOR>=6)   
  mosquitto_property_free_all(&m_properties);
#endif   
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// CTor
//

#if LIBMOSQUITTO_MAJOR>1 || (LIBMOSQUITTO_MAJOR==1 && LIBMOSQUITTO_MINOR>=6)  
subscribeTopic::subscribeTopic(const std::string &topic, int qos, int v5_options, mosquitto_property *properties)
#else
subscribeTopic::subscribeTopic(const std::string &topic, int qos, int v5_options)
#endif
{
  m_topic      = topic;
  m_qos        = qos;
  m_v5_options = v5_options;
#if LIBMOSQUITTO_MAJOR>1 || (LIBMOSQUITTO_MAJOR==1 && LIBMOSQUITTO_MINOR>=6)    
  m_properties = properties;
#endif  
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

subscribeTopic::~subscribeTopic()
{
#if LIBMOSQUITTO_MAJOR>1 || (LIBMOSQUITTO_MAJOR==1 && LIBMOSQUITTO_MINOR>=6)   
  mosquitto_property_free_all(&m_properties);
#endif  
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// C-tor
//

vscpClientMqtt::vscpClientMqtt(void)
{
  spdlog::debug("constructor vscp_client_mqtt object.");

  m_mapMqttIntOptions["tcp-nodelay"]      = 1;
  m_mapMqttIntOptions["protocol-version"] = 311;
  m_mapMqttIntOptions["receive-maximum"]  = 20;
  m_mapMqttIntOptions["send-maximum"]     = 20;

  m_bConnected       = false;
  m_bindInterface    = "";
  m_mosq             = nullptr;
  m_type             = CVscpClient::connType::MQTT;
  m_publish_format   = jsonfmt;
  m_subscribe_format = autofmt;
  m_mosq             = nullptr;
  m_bConnected       = false; // Not connected
  m_bRun             = false;
  m_host             = ""; // tcp://localhost:1883
  m_port             = 1883;
  m_clientid         = "";
  m_username         = "";
  m_password         = "";
  m_keepalive        = 30;
  m_bCleanSession    = false;

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

  // delay=2, delay_max=10, exponential_backoff=False Delays would be: 2, 4, 6, 8, 10, 10, ...
  m_reconnect_delay               = 2;
  m_reconnect_delay_max           = 10;
  m_reconnect_exponential_backoff = false;

  // Initialize MQTT
  if (MOSQ_ERR_SUCCESS != mosquitto_lib_init()) {
    spdlog::error("init object: Unable to initialize mosquitto library.");
    return;
  }

  pthread_mutex_init(&m_mutexif, nullptr);
}

///////////////////////////////////////////////////////////////////////////////
// D-tor
//

vscpClientMqtt::~vscpClientMqtt()
{
  spdlog::debug("destructor vscp_client_mqtt object.");

  // Disconnect if we still are connected
  if (isConnected()) disconnect();

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
  for (std::list<subscribeTopic *>::const_iterator it = m_mqtt_subscribe.begin(); it != m_mqtt_subscribe.end(); ++it) {
    delete (*it);
  }

  m_mqtt_subscribe.clear();

  // Delete publish objects
  for (std::list<publishTopic *>::const_iterator it = m_mqtt_publish.begin(); it != m_mqtt_publish.end(); ++it) {
    delete (*it);
  }

  m_mqtt_publish.clear();

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
    }

    // Host address on form (s)tcp://ip:port
    if (j.contains("host")) {

      m_host = j["host"].get<std::string>();

      vscp_trim(m_host);
      if (0 == m_host.find("tcp://")) {
        m_host = m_host.substr(6);
        m_bTLS = false;
      }
      else if (0 == m_host.find("stcp://")) {
        m_host = m_host.substr(7);
        m_bTLS = true;
      }

      size_t pos;
      if (std::string::npos != (pos = m_host.rfind(":"))) {
        m_port = vscp_readStringValue(m_host.substr(pos + 1));
        m_host = m_host.substr(0, pos);
      }
    }

    // Port
    if (j.contains("port")) {
      m_port = j["port"].get<uint16_t>();
    }

    // MQTT options
    if (j.contains("mqtt-options") && j["mqtt-options"].is_object()) {

      json jj = j["mqtt-options"];

      if (jj.contains("tcp-nodelay")) {
        m_mapMqttIntOptions["tcp-nodelay"] = jj["tcp-nodelay"].get<int>();
      }

      if (jj.contains("protocol-version")) {
        m_mapMqttIntOptions["protocol-version"] = jj["protocol-version"].get<int>();
      }

      if (jj.contains("receive-maximum")) {
        m_mapMqttIntOptions["receive-maximum"] = jj["receive-maximum"].get<int>();
      }

      if (jj.contains("send-maximum")) {
        m_mapMqttIntOptions["send-maximum"] = jj["send-maximum"].get<int>();
      }

      if (jj.contains("ssl-ctx-with-defaults")) {
        m_mapMqttIntOptions["ssl-ctx-with-defaults"] = jj["ssl-ctx-with-defaults"].get<int>();
      }

      if (jj.contains("tls-ocsp-required")) {
        m_mapMqttIntOptions["tls-ocsp-required"] = jj["tls-ocsp-required"].get<int>();
      }

      if (jj.contains("tls-use-os-certs")) {
        m_mapMqttIntOptions["tls-use-os-certs"] = jj["tls-use-os-certs"].get<int>();
      }

      if (jj.contains("tls-engine")) {
        m_mapMqttStringOptions["tls-engine"] = jj["tls-engine"].get<std::string>();
      }

      if (jj.contains("tls-keyform")) {
        m_mapMqttStringOptions["tls-keyform"] = jj["tls-keyform"].get<std::string>();
      }

      if (jj.contains("tls-kpass-sha1")) {
        m_mapMqttStringOptions["tls-kpass-sha1"] = jj["tls-kpass-sha1"].get<std::string>();
      }

      if (jj.contains("tls-alpn")) {
        m_mapMqttStringOptions["tls-alpn"] = jj["tls-alpn"].get<std::string>();
      }

      if (jj.contains("bind-address")) {
        m_mapMqttStringOptions["bind-address"] = jj["bind-address"].get<std::string>();
      }

      // if (jj.contains("ssl-ctx")) {
      //   m_mapMqttVoidOptions["ssl-ctx"] = jj["ssl-ctx"].get<void *>();
      // }
    }
    else if (j.contains("mqtt-options")) {
      // Format is invalid
      spdlog::warn("config: 'mqtt-options' present but not an object.");
    }

    // user
    if (j.contains("user")) {
      m_username = j["user"].get<std::string>();
    }

    // password
    if (j.contains("password")) {
      m_password = j["password"].get<std::string>();
    }

    // Client ID
    if (j.contains("clientid")) {
      m_clientid = j["clientid"].get<std::string>();
    }

    // Publish format
    if (j.contains("publish-format")) {
      std::string str = j["publish-format"].get<std::string>();
      vscp_makeLower(str);
      if (std::string::npos != str.find("binary")) {
        spdlog::debug("config: 'subscribe_format' Set to BINARY.");
        m_publish_format = binfmt;
      }
      else if (std::string::npos != str.find("string")) {
        spdlog::debug("config: 'subscribe_format' Set to STRING.");
        m_publish_format = strfmt;
      }
      else if (std::string::npos != str.find("json")) {
        spdlog::debug("config: 'subscribe_format' Set to JSON.");
        m_publish_format = jsonfmt;
      }
      else if (std::string::npos != str.find("xml")) {
        spdlog::debug("config: 'subscribe_format' Set to XML.");
        m_publish_format = xmlfmt;
      }
      else {
        spdlog::error("config: 'publish_format' Ivalid value. Set to JSON.");
        m_publish_format = jsonfmt;
      }
    }

    // Subscribe format
    if (j.contains("subscribe-format")) {
      std::string str = j["subscribe-format"].get<std::string>();
      vscp_makeLower(str);
      if (std::string::npos != str.find("binary")) {
        spdlog::debug("config: 'subscribe_format' Set to BINARY.");
        m_subscribe_format = binfmt;
      }
      else if (std::string::npos != str.find("string")) {
        spdlog::debug("config: 'subscribe_format' Set to STRING.");
        m_subscribe_format = strfmt;
      }
      else if (std::string::npos != str.find("json")) {
        spdlog::debug("config: 'subscribe_format' Set to JSON.");
        m_subscribe_format = jsonfmt;
      }
      else if (std::string::npos != str.find("xml")) {
        spdlog::debug("config: 'subscribe_format' Set to XML.");
        m_subscribe_format = xmlfmt;
      }
      else if (std::string::npos != str.find("auto")) {
        spdlog::debug("config: 'subscribe_format' Set to AUTO.");
        m_subscribe_format = autofmt;
      }
      else {
        spdlog::error("config: 'subscribe_format' Ivalid value. Set to AUTO.");
        m_subscribe_format = autofmt;
      }
    }

    // qos
    if (j.contains("qos")) {
      m_qos = j["qos"].get<uint8_t>();
      if (m_qos > 2) {
        spdlog::error("config: 'qos' Ivalid value [{}]. Set to 0.", m_qos);
        m_qos = 0;
      }
      spdlog::debug("config: 'qos' Set to {}.", m_qos);
    }

    // Clean Session
    if (j.contains("bcleansession")) {
      m_bCleanSession = j["bcleansession"].get<bool>();
      spdlog::debug("config: 'bcleansession' Set to {}.", m_bCleanSession);
    }

    // Retain
    if (j.contains("bretain")) {
      m_bRetain = j["bretain"].get<bool>();
      spdlog::debug("config: 'bretain' Set to {}.", m_bRetain);
    }

    // Keep Alive
    if (j.contains("keepalive")) {
      m_keepalive = j["keepalive"].get<int>();
      spdlog::debug("config: 'keepalive' Set to {}.", m_keepalive);
    }

    // Reconnect
    if (j.contains("reconnect") && j["reconnect"].is_object()) {

      json jj = j["reconnect"];

      if (jj.contains("delay")) {
        m_reconnect_delay = jj["delay"].get<int>();
        spdlog::debug("config: 'reconnect delay' Set to {}.", m_reconnect_delay);
      }
      else if (jj.contains("delay-max")) {
        m_reconnect_delay_max = jj["delay-max"].get<int>();
        spdlog::debug("config: 'reconnect delay-max' Set to {}.", m_reconnect_delay_max);
      }
      else if (jj.contains("exponential-backoff")) {
        m_reconnect_exponential_backoff = jj["exponential-backoff"].get<bool>();
        spdlog::debug("config: 'reconnect exponential-backoff' Set to {}.", m_reconnect_exponential_backoff);
      }
      else {
        spdlog::debug("config: 'reconnect' Invalid token.");
      }
    }

    // Last will
    if (j.contains("filter") && j["filter"].is_object()) {

      json jj = j["filter"];

      if (jj.contains("priority-filter")) {
        m_filter.filter_priority = jj["priority-filter"].get<uint8_t>();
      }

      if (jj.contains("priority-mask")) {
        m_filter.mask_priority = jj["priority-mask"].get<uint8_t>();
      }

      if (jj.contains("class-filter")) {
        m_filter.filter_class = jj["class-filter"].get<uint16_t>();
      }

      if (jj.contains("class-mask")) {
        m_filter.mask_class = jj["class-mask"].get<uint16_t>();
      }

      if (jj.contains("type-filter")) {
        m_filter.filter_type = jj["type-filter"].get<uint16_t>();
      }

      if (jj.contains("type-mask")) {
        m_filter.mask_type = jj["type-mask"].get<uint16_t>();
      }

      if (jj.contains("guid-filter")) {
        std::string str = jj["guid-filter"].get<std::string>();
        vscp_getGuidFromStringToArray(m_filter.filter_GUID, str);
      }

      if (jj.contains("guid-mask")) {
        std::string str = jj["guid-mask"].get<std::string>();
        vscp_getGuidFromStringToArray(m_filter.mask_GUID, str);
      }
    }

    // TLS
    if (j.contains("tls") && j["tls"].is_object()) {

      json jj = j["tls"];
      m_bTLS  = true;

      if (jj.contains("cafile")) {
        m_tls_cafile = jj["cafile"].get<std::string>();
        spdlog::debug("config: 'tls cafile' Set to {}.", m_tls_cafile);
      }
      else if (jj.contains("capath")) {
        m_tls_capath = jj["capath"].get<std::string>();
        spdlog::debug("config: 'tls capath' Set to {}.", m_tls_capath);
      }
      else if (jj.contains("certfile")) {
        m_tls_certfile = jj["certfile"].get<std::string>();
        spdlog::debug("config: 'tls certfile' Set to {}.", m_tls_certfile);
      }
      else if (jj.contains("keyfile")) {
        m_tls_keyfile = jj["keyfile"].get<std::string>();
        spdlog::debug("config: 'tls keyfile' Set to {}.", m_tls_keyfile);
      }
      else if (jj.contains("pwkeyfile")) {
        m_tls_pwKeyfile = jj["pwkeyfile"].get<std::string>();
        spdlog::debug("config: 'tls pwkeyfile' Set to {}.", m_tls_pwKeyfile);
      }
      else if (jj.contains("no-hostname-checking")) {
        m_tls_bNoHostNameCheck = jj["no-hostname-checking"].get<bool>();
        spdlog::debug("config: 'tls no-hostname-checking' Set to {}.", m_tls_bNoHostNameCheck);
      }
      else if (jj.contains("cert-reqs")) {
        m_tls_cert_reqs = jj["cert-reqs"].get<int>();
        spdlog::debug("config: 'tls cert-reqs' Set to {}.", m_tls_cert_reqs);
      }
      else if (jj.contains("version")) {
        m_tls_version = jj["version"].get<std::string>();
        spdlog::debug("config: 'tls version' Set to {}.", m_tls_version);
      }
      else if (jj.contains("ciphers")) {
        m_tls_ciphers = jj["ciphers"].get<std::string>();
        spdlog::debug("config: 'tls ciphers' Set to {}.", m_tls_ciphers);
      }
      else if (jj.contains("psk")) {
        m_tls_psk = jj["psk"].get<std::string>();
        spdlog::debug("config: 'tls psk' Set to {}.", m_tls_psk);
      }
      else if (jj.contains("psk-identity")) {
        m_tls_identity = jj["psk-identity"].get<std::string>();
        spdlog::debug("config: 'tls psk-identity' Set to {}.", m_tls_identity);
      }
      else {
        spdlog::debug("config: 'TLS' Invalid token.");
      }

      // Both of cafile/capath can not be null
      if (!m_tls_cafile.length() && !m_tls_capath.length()) {
        spdlog::warn("config: 'TLS'  Both cafile and capath must not be NULL. TLS disabled.");
        m_bTLS = false;
      }

      // If certfile == NULL, keyfile must also be NULL and no client certificate will be used.
      if (!m_tls_certfile.length()) {
        spdlog::warn("config: 'TLS'  If certfile == NULL, keyfile must also be NULL and no client certificate will be "
                     "used. keyfile set to NULL.");
        m_tls_keyfile = "";
      }

      // If m_tls_keyfile == NULL, certfile must also be NULL and no client certificate will be used.
      if (!m_tls_keyfile.length()) {
        spdlog::warn("config: 'TLS'  If m_tls_keyfile == NULL, certfile must also be NULL and no client certificate "
                     "will be used. certfile set to NULL.");
        m_tls_certfile = "";
      }
    }

    // Last will
    if (j.contains("will") && j["will"].is_object()) {

      json jj = j["will"];
      m_bWill = true;

      if (jj.contains("topic")) {
        m_will_topic = jj["topic"].get<std::string>();
        spdlog::debug("config: 'will topic' Set to {}.", m_will_topic);
      }
      else if (jj.contains("qos")) {
        m_will_qos = jj["qos"].get<uint8_t>();
        spdlog::debug("config: 'will qos' Set to {}.", m_will_qos);
      }
      else if (jj.contains("retain")) {
        m_will_bretain = jj["retain"].get<bool>();
        spdlog::debug("config: 'will retain' Set to {}.", m_will_bretain);
      }
      else if (jj.contains("payload")) {
        m_will_payload = jj["payload"].get<std::string>();
        spdlog::debug("config: 'will payload' Set to {}.", m_will_payload);
      }
      else {
        spdlog::debug("config: 'will' Invalid token.");
      }
    }

    // Subscribe
    if (j.contains("subscribe") && j["subscribe"].is_array()) {

      json jj = j["subscribe"];
      for (std::string str : j["subscribe"]) {
        addSubscription(str, 0);
      }
    }

    // Publish
    if (j.contains("publish") && j["publish"].is_array()) {
      json jj = j["publish"];
    }

    // v5
    if (j.contains("v5") && j["v5"].is_array()) {}
  }
  catch (...) {
    spdlog::error("config: JSON parsing error.");
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
  enumMqttMsgFormat format;
  vscpEvent ev;
  vscpEventEx ex;

  // Check pointers
  if (nullptr == pmsg) {
    return false;
  }

  // Must/Should be a payload
  if (!pmsg->payloadlen) {
    return 0;
  }

  if (autofmt == m_subscribe_format) {

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
      // String
      format = strfmt;
    }
  }
  else {
    format = m_subscribe_format;
  }

  // Binary payload will result in zero length string as first character
  // is a zero.
  std::string payload((const char *) pmsg->payload, pmsg->payloadlen);

  if (jsonfmt == format) {

    if (!vscp_convertJSONToEvent(&ev, payload)) {
      return false;
    }

    if (!vscp_convertEventToEventEx(&ex, &ev)) {
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
      if (nullptr == pEvent)
        return false;
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
      return false;
    }

    if (!vscp_convertEventToEventEx(&ex, &ev)) {
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
      if (nullptr == pEvent)
        return false;
      pEvent->pdata = nullptr;

      if (!vscp_copyEvent(pEvent, &ev)) {
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
      return false;
    }

    if (!vscp_convertEventToEventEx(&ex, &ev)) {
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
      if (nullptr == pEvent)
        return false;
      pEvent->pdata = nullptr;

      if (!vscp_copyEvent(pEvent, &ev)) {
        delete pEvent;
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
      return false;
    }

    if (!vscp_convertEventToEventEx(&ex, &ev)) {
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
      if (nullptr == pEvent)
        return false;
      pEvent->pdata = nullptr;

      if (!vscp_copyEvent(pEvent, &ev)) {
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
  int rv;

  if (m_clientid.length()) {
    m_mosq = mosquitto_new(m_clientid.c_str(), m_bCleanSession, this);
  }
  else {
    m_bCleanSession = true; // Must be true without id
    m_mosq          = mosquitto_new(nullptr, m_bCleanSession, this);
  }

  // v5
  if (m_mapMqttIntOptions["protocol-version"] >= 500) {
    mosquitto_int_option(m_mosq, MOSQ_OPT_PROTOCOL_VERSION, MQTT_PROTOCOL_V5);
  }

  if (nullptr == m_mosq) {
    if (ENOMEM == errno) {
      spdlog::error("Failed to create new mosquitto session (out of memory).");
    }
    else if (EINVAL == errno) {
      spdlog::error("Failed to create new mosquitto session (invalid parameters).");
    }
    return false;
  }

  // Callbacks
  if (m_mapMqttIntOptions["protocol-version"] >= 500) {
    mosquitto_log_callback_set(m_mosq, mqtt_on_log);
    mosquitto_connect_v5_callback_set(m_mosq, mqtt_on_connect_v5);
    mosquitto_disconnect_v5_callback_set(m_mosq, mqtt_on_disconnect_v5);
    mosquitto_message_v5_callback_set(m_mosq, mqtt_on_message_v5);
    mosquitto_publish_v5_callback_set(m_mosq, mqtt_on_publish_v5);
    mosquitto_subscribe_v5_callback_set(m_mosq, mqtt_on_subscribe_v5);
    mosquitto_unsubscribe_v5_callback_set(m_mosq, mqtt_on_unsubscribe_v5);
  }
  else {
    mosquitto_log_callback_set(m_mosq, mqtt_on_log);
    mosquitto_connect_with_flags_callback_set(m_mosq, mqtt_on_connect);
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
    spdlog::warn("Failed to set reconnect settings.");
  }

  // Set 'last will' if any is defined
  if (m_will_payload.length() && m_will_topic.length()) {
    if (m_mapMqttIntOptions["protocol-version"] >= 500) {
      if (MOSQ_ERR_SUCCESS != mosquitto_will_set_v5(m_mosq,
                                                    m_will_topic.c_str(),
                                                    m_will_payload.length(),
                                                    m_will_payload.c_str(),
                                                    m_will_qos,
                                                    m_will_bretain,
                                                    nullptr)) {
        spdlog::warn("Failed to set last will. rv={0} {1}", rv, mosquitto_strerror(rv));
      }
    }
    else {
      if (MOSQ_ERR_SUCCESS != mosquitto_will_set(m_mosq,
                                                 m_will_topic.c_str(),
                                                 m_will_payload.length(),
                                                 m_will_payload.c_str(),
                                                 m_will_qos,
                                                 m_will_bretain)) {
        spdlog::error("Failed to set last will. rv={0} {1}", rv, mosquitto_strerror(rv));
      }
    }
  }

  // Set username/password if defined
  if (m_username.length()) {
    int rv;
    if (MOSQ_ERR_SUCCESS != (rv = mosquitto_username_pw_set(m_mosq, m_username.c_str(), m_password.c_str()))) {
      spdlog::error("mosquitto_username_pw_set failed. rv={0} {1}", rv, mosquitto_strerror(rv));
    }
  }

  //                          * * * Set options * * *

  // tcp-nodelay
  if (MOSQ_ERR_SUCCESS !=
      (rv = mosquitto_int_option(m_mosq, MOSQ_OPT_TCP_NODELAY, m_mapMqttIntOptions["tcp-nodelay"]))) {
    spdlog::error("Failed to set option MOSQ_OPT_TCP_NODELAY. rv={0} {1}", rv, mosquitto_strerror(rv));
  }

  // version
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
      spdlog::error("Failed to set option MOSQ_OPT_PROTOCOL_VERSION. rv={0} {1}", rv, mosquitto_strerror(rv));
    }
  }

  // receive-maximum
  if (MOSQ_ERR_SUCCESS !=
      (rv = mosquitto_int_option(m_mosq, MOSQ_OPT_RECEIVE_MAXIMUM, m_mapMqttIntOptions["receive-maximum"]))) {
    spdlog::error("Failed to set option MOSQ_OPT_RECEIVE_MAXIMUM. rv={0} {1}", rv, mosquitto_strerror(rv));
  }

  // sen-maximum
  if (MOSQ_ERR_SUCCESS !=
      (rv = mosquitto_int_option(m_mosq, MOSQ_OPT_SEND_MAXIMUM, m_mapMqttIntOptions["send-maximum"]))) {
    spdlog::error("Failed to set option MOSQ_OPT_SEND_MAXIMUM. rv={0} {1}", rv, mosquitto_strerror(rv));
  }

  //                          * * * TLS * * *

  if (m_bTLS) {
    if (MOSQ_ERR_SUCCESS != (rv = mosquitto_tls_set(m_mosq,
                                                    m_tls_cafile.length() ? m_tls_cafile.c_str() : NULL,
                                                    m_tls_capath.length() ? m_tls_capath.c_str() : NULL,
                                                    m_tls_certfile.length() ? m_tls_certfile.c_str() : NULL,
                                                    m_tls_keyfile.length() ? m_tls_keyfile.c_str() : NULL,
                                                    password_callback))) {
      spdlog::error("Failed to set mosquitto tls. rv={0} {1}", rv, mosquitto_strerror(rv));
    }

    if (MOSQ_ERR_SUCCESS != (rv = mosquitto_tls_insecure_set(m_mosq, m_tls_bNoHostNameCheck))) {
      spdlog::error("Failed to set mosquitto tls insecure. rv={0} {1}", rv, mosquitto_strerror(rv));
    }

    if (MOSQ_ERR_SUCCESS != (rv = mosquitto_tls_opts_set(m_mosq,
                                                         m_tls_cert_reqs,
                                                         m_tls_version.length() ? m_tls_version.c_str() : NULL,
                                                         m_tls_ciphers.length() ? m_tls_ciphers.c_str() : NULL))) {
      spdlog::error("Failed to set mosquitto tls options. rv={0} {1}", rv, mosquitto_strerror(rv));
    }
  }
  else {
    if (m_tls_psk.length()) {
      if (MOSQ_ERR_SUCCESS != (rv = mosquitto_tls_psk_set(m_mosq,
                                                          m_tls_psk.c_str(),
                                                          m_tls_identity.c_str(),
                                                          m_tls_ciphers.length() ? m_tls_ciphers.c_str() : NULL))) {
        spdlog::error("Failed to set mosquitto tls psk. rv={0} {1}", rv, mosquitto_strerror(rv));
      }
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// addSubscription
//

int
vscpClientMqtt::addSubscription(const std::string strTopicSub, int qos, int v5_options, mosquitto_property *properties)
{
#if LIBMOSQUITTO_MAJOR>1 || (LIBMOSQUITTO_MAJOR==1 && LIBMOSQUITTO_MINOR>=6)    
  subscribeTopic *pTopic = new subscribeTopic(strTopicSub, qos, v5_options, properties);
#else
  subscribeTopic *pTopic = new subscribeTopic(strTopicSub, qos, v5_options);
#endif  
  m_mqtt_subscribe.push_back(pTopic);
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// addPublish
//

int
vscpClientMqtt::addPublish(const std::string strTopicPub, int qos, bool bRetain, mosquitto_property *properties)
{
#if LIBMOSQUITTO_MAJOR>1 || (LIBMOSQUITTO_MAJOR==1 && LIBMOSQUITTO_MINOR>=6)    
  publishTopic *pTopic = new publishTopic(strTopicPub, qos, bRetain, properties);
#else
  publishTopic *pTopic = new publishTopic(strTopicPub, qos, bRetain);
#endif  
  m_mqtt_publish.push_back(pTopic);
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
    if (m_mapMqttIntOptions["protocol-version"] >= 500) {
      rv = mosquitto_connect_bind_v5(m_mosq, m_host.c_str(), m_port, m_keepalive, m_bindInterface.c_str(), nullptr);
    }
    else {
      rv = mosquitto_connect_bind(m_mosq, m_host.c_str(), m_port, m_keepalive, m_bindInterface.c_str());
    }
  }
  else {
    rv = mosquitto_connect(m_mosq, m_host.c_str(), m_port, m_keepalive);
  }

  if (MOSQ_ERR_SUCCESS != rv) {
    spdlog::error("Failed to connect to remote host. rv={0} {1}", rv, mosquitto_strerror(rv));
    return VSCP_ERROR_NOT_CONNECTED;
  }

  // Start the worker loop
  rv = mosquitto_loop_start(m_mosq);
  if (MOSQ_ERR_SUCCESS != rv) {
    spdlog::error("Failed to start mosquitto worker loop. rv={0} {1}", rv, mosquitto_strerror(rv));
    mosquitto_disconnect(m_mosq);
    return VSCP_ERROR_ERROR;
  }

  // Only subscribe if subscription topic is defined
  for (std::list<subscribeTopic *>::const_iterator it = m_mqtt_subscribe.begin(); it != m_mqtt_subscribe.end(); ++it) {

    subscribeTopic *psubtopic = (*it);

    // Fix subscribe topics
    mustache subtemplate{ psubtopic->getTopic() };
    data data;
    // data.set("guid", m_guid.getAsString());
    std::string subscribe_topic = subtemplate.render(data);

    // Subscribe to specified topic
    rv = mosquitto_subscribe(m_mosq,
                             /*m_mqtt_id*/ nullptr,
                             subscribe_topic.c_str(),
                             m_qos);
    if (MOSQ_ERR_SUCCESS != rv) {
      spdlog::error("Failed to subscribed to topic '{0}' - rv={1} {2}.", subscribe_topic, rv, mosquitto_strerror(rv));
    }
  }

  // Start worker thread if a callback has been defined
  if ((nullptr != m_evcallback) || (nullptr != m_excallback)) {
    int rv = pthread_create(&m_tid, nullptr, workerThread, this);
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

  spdlog::debug("Enter disconnect.");

  pthread_join(m_tid, nullptr);

  // Disconnect from MQTT broker}
  rv = mosquitto_disconnect(m_mosq);
  if (MOSQ_ERR_SUCCESS != rv) {
    spdlog::error("mosquitto_disconnect failed. rv={0} {1}", rv, mosquitto_strerror(rv));
  }

  // stop the worker loop
  rv = mosquitto_loop_stop(m_mosq, false);
  if (MOSQ_ERR_SUCCESS != rv) {
    spdlog::error("mosquitto_disconnect failed. rv={0} {1}", rv, mosquitto_strerror(rv));
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
  uint8_t payload[1024];
  size_t lenPayload = 0;
  int rv;

  // Only subscribe if subscription topic is defined
  for (std::list<publishTopic *>::const_iterator it = m_mqtt_publish.begin(); it != m_mqtt_publish.end(); ++it) {

    publishTopic *ppublish = (*it);

    memset(payload, 0, sizeof(payload));

    if (m_publish_format == jsonfmt) {
      std::string strPayload;
      if (!vscp_convertEventToJSON(strPayload, &ev)) {
        return VSCP_ERROR_PARAMETER;
      }
      lenPayload = strPayload.length();
      strncpy((char *) payload, strPayload.c_str(), sizeof(payload));
    }
    else if (m_publish_format == xmlfmt) {
      std::string strPayload;
      if (!vscp_convertEventToXML(strPayload, &ev)) {
        return VSCP_ERROR_PARAMETER;
      }
      lenPayload = strPayload.length();
      strncpy((char *) payload, strPayload.c_str(), sizeof(payload));
    }
    else if (m_publish_format == strfmt) {
      std::string strPayload;
      if (!vscp_convertEventToString(strPayload, &ev)) {
        return VSCP_ERROR_PARAMETER;
      }
      lenPayload = strPayload.length();
      strncpy((char *) payload, strPayload.c_str(), sizeof(payload));
    }
    else if (m_publish_format == binfmt) {

      lenPayload = vscp_getFrameSizeFromEvent(&ev) + 1;

      // Write event to frame
      if (!vscp_writeEventToFrame(payload + 1, sizeof(payload), 0, &ev)) {
        return VSCP_ERROR_PARAMETER;
      }
    }
    else {
      return VSCP_ERROR_NOT_SUPPORTED;
    }

    if ( MOSQ_ERR_SUCCESS != (rv = mosquitto_publish(m_mosq,
                                                      NULL, // msg id
                                                      ppublish->getTopic().c_str(),
                                                      (int)lenPayload,
                                                      payload,
                                                      ppublish->getQos(),
                                                      ppublish->getRetain()))) {
       spdlog::error("mosquitto_disconnect failed. rv={0} {1}", rv, mosquitto_strerror(rv));                             
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
  uint8_t payload[1024];
  size_t lenPayload = 0;
  int rv;

  // Only subscribe if subscription topic is defined
  for (std::list<publishTopic *>::const_iterator it = m_mqtt_publish.begin(); it != m_mqtt_publish.end(); ++it) {

    publishTopic *ppublish = (*it);

    memset(payload, 0, sizeof(payload));

    if (m_publish_format == jsonfmt) {
      std::string strPayload;
      if (!vscp_convertEventExToJSON(strPayload, &ex)) {
        return VSCP_ERROR_PARAMETER;
      }
      lenPayload = strPayload.length();
      strncpy((char *) payload, strPayload.c_str(), sizeof(payload));
    }
    else if (m_publish_format == xmlfmt) {
      std::string strPayload;
      if (!vscp_convertEventExToXML(strPayload, &ex)) {
        return VSCP_ERROR_PARAMETER;
      }
      lenPayload = strPayload.length();
      strncpy((char *) payload, strPayload.c_str(), sizeof(payload));
    }
    else if (m_publish_format == strfmt) {
      std::string strPayload;
      if (!vscp_convertEventExToString(strPayload, &ex)) {
        return VSCP_ERROR_PARAMETER;
      }
      lenPayload = strPayload.length();
      strncpy((char *) payload, strPayload.c_str(), sizeof(payload));
    }
    else if (m_publish_format == binfmt) {

      lenPayload = vscp_getFrameSizeFromEventEx(&ex) + 1;

      // Write event to frame
      if (!vscp_writeEventExToFrame(payload + 1, sizeof(payload), 0, &ex)) {
        return VSCP_ERROR_PARAMETER;
      }
    }
    else {
      return VSCP_ERROR_NOT_SUPPORTED;
    }

    if ( MOSQ_ERR_SUCCESS != (rv = mosquitto_publish(m_mosq,
                                                      NULL, // msg id
                                                      ppublish->getTopic().c_str(),
                                                      (int)lenPayload,
                                                      payload,
                                                      ppublish->getQos(),
                                                      ppublish->getRetain()))) {
       spdlog::error("mosquitto_disconnect failed. rv={0} {1}", rv, mosquitto_strerror(rv));                             
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
  spdlog::debug("getversion: {}.{}.{}.{}", v1, v2, v3, 0);

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
vscpClientMqtt::setCallback(LPFNDLL_EV_CALLBACK m_evcallback)
{
  // Can not be called when connected
  if (m_bConnected)
    return VSCP_ERROR_ERROR;

  setCallback(m_evcallback);

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setCallback
//

int
vscpClientMqtt::setCallback(LPFNDLL_EX_CALLBACK m_excallback)
{
  // Can not be called when connected
  if (m_bConnected)
    return VSCP_ERROR_ERROR;

  setCallback(m_excallback);

  return VSCP_ERROR_SUCCESS;
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
