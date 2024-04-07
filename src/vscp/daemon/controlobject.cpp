// ControlObject.cpp: implementation of the CControlObject
// class.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2024 Ake Hedman, the VSCP project
// <info@vscp.org>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//

#define _POSIX

#ifdef WIN32
#include <nb30.h>
#include <pch.h>
#endif

#include "controlobject.h"

#ifndef WIN32
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pwd.h>
#include <sys/ioctl.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#else
#include <strstream>
#endif

#include <errno.h>
#include <pthread.h>

#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifdef WITH_SYSTEMD
#include <systemd/sd-daemon.h>
#endif

#include <mosquitto.h>
#include <sqlite3.h>

#include <crc.h>
#include <devicelist.h>
#include <devicethread.h>
#include <randpassword.h>
#include <version.h>
#include <vscp.h>
#include <vscp_client_mqtt.h>
#include <vscp_debug.h>
#include <vscpd_caps.h>
#include <vscphelper.h>

#include <algorithm>
#include <deque>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <string>

#include <mustache.hpp>
#include <nlohmann/json.hpp> // Needs C++11  -std=c++11

#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/spdlog.h>

// https://github.com/nlohmann/json
using json = nlohmann::json;

using namespace kainjow::mustache;

#define UNUSED(x) (void) (x)
void
foo(const int i)
{
  UNUSED(i);
}

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#define XML_BUFF_SIZE 0xffff

// From vscp.cpp
extern uint8_t __vscp_key[32]; // (256 bits / 32 bytes)
extern uint64_t gDebugLevel;

// Prototypes

//////////////////////////////////////////////////////////////////////
//                         Callbacks
//////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// mqtt_log_callback
//

// static void
// {
//   // Check pointers
//   if (nullptr == mosq) {
//     return;
//   }
//   if (nullptr == pData) {
//     return;
//   }

//   CControlObject *pObj = reinterpret_cast<CControlObject *>(pData);

//   if (gDebugLevel & VSCP_DEBUG_MQTT_LOG) {
//     char buf[80];
//     time_t tm;
//     time(&tm);
//     vscp_getTimeString(buf, sizeof(buf), &tm);

//     if (spdlog::get("logger") != nullptr) {
//       spdlog::trace("MQTT log: {}", logmsg);
//     }
//   }
// }

///////////////////////////////////////////////////////////////////////////////
// mqtt_on_connect
//

// static void
// mqtt_on_connect(struct mosquitto *mosq, void *pData, int rv)
// {
//   // Check pointers
//   if (nullptr == mosq) {
//     return;
//   }
//   if (nullptr == pData) {
//     return;
//   }

//   CControlObject *pObj = reinterpret_cast<CControlObject *>(pData);

//   if (gDebugLevel & VSCP_DEBUG_MQTT_CONNECT) {
//     if (spdlog::get("logger") != nullptr) {
//       spdlog::debug("MQTT connect:");
//     }
//   }
// }

///////////////////////////////////////////////////////////////////////////////
// mqtt_on_disconnect
//

// static void
// mqtt_on_disconnect(struct mosquitto *mosq, void *pData, int rv)
// {
//   // Check pointers
//   if (nullptr == mosq) {
//     return;
//   }
//   if (nullptr == pData) {
//     return;
//   }

//   CControlObject *pObj = reinterpret_cast<CControlObject *>(pData);

//   if (gDebugLevel & VSCP_DEBUG_MQTT_CONNECT) {
//     if (spdlog::get("logger") != nullptr) {
//       spdlog::debug("MQTT disconnect:");
//     }
//   }
// }

///////////////////////////////////////////////////////////////////////////////
// mqtt_on_message
//

// static void
// mqtt_on_message(struct mosquitto *mosq, void *pData, const struct mosquitto_message *pMsg)
// {
//   // Check pointers
//   if (nullptr == mosq) {
//     return;
//   }
//   if (nullptr == pData) {
//     return;
//   }
//   if (nullptr == pMsg) {
//     return;
//   }

//   CControlObject *pObj = reinterpret_cast<CControlObject *>(pData);
//   std::string payload((const char *) pMsg->payload, pMsg->payloadlen); // String payload

//   if (gDebugLevel & VSCP_DEBUG_MQTT_MSG) {
//     if (spdlog::get("logger") != nullptr) {
//       spdlog::trace("MQTT Message: Topic = [{}] - Payload: [{}]", pMsg->topic, payload);
//     }
//   }
// }

///////////////////////////////////////////////////////////////////////////////
// mqtt_on_publish
//

// static void
// mqtt_on_publish(struct mosquitto *mosq, void *pData, int rv)
// {
//   // Check pointers
//   if (nullptr == mosq) {
//     return;
//   }
//   if (nullptr == pData) {
//     return;
//   }

//   CControlObject *pObj = reinterpret_cast<CControlObject *>(pData);

//   if (gDebugLevel & VSCP_DEBUG_MQTT_PUBLISH) {
//     if (spdlog::get("logger") != nullptr) {
//       spdlog::trace("MQTT Publish:");
//     }
//   }
// }

// ----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CControlObject::CControlObject()
{
  m_bQuit = false;

  if (gDebugLevel & VSCP_DEBUG_EXTRA) {
    spdlog::debug("ControlObject: Starting the vscpd daemon");
  }

  if (0 != pthread_mutex_init(&m_mutex_DeviceList, NULL)) {
    spdlog::critical("ControlObject: Unable to init m_mutex_DeviceList. Abort!");
    return;
  }

  m_strServerName = "VSCP Daemon";
  m_rootFolder    = "/var/lib/vscp/vscpd/";

  m_pathClassTypeDefinitionDb = "/var/lib/vscp/vscpd/vscp_events.sqlite3";

  // Logging defaults
  m_fileLogLevel     = spdlog::level::info;
  m_fileLogPattern   = "[vscpd] [%^%l%$] %v";
  m_path_to_log_file = "/var/log/vscp/vscp.log";
  m_max_log_size     = 5242880;
  m_max_log_files    = 7;

  m_bEnableConsoleLog = false;
  m_consoleLogLevel   = spdlog::level::info;
  m_consoleLogPattern = "[vscpd] [%^%l%$] %v";

  // Nill the GUID
  m_guid.clear();

  // Initialize the CRC
  crcInit();

  // Init daemon MQTT
  // m_mqtt_strHost                       = "localhost";
  // m_mqtt_port                          = 1883;
  // m_mqtt_strClientId                   = "";
  // m_mqtt_strUserName                   = "vscp";
  // m_mqtt_strPassword                   = "secret";
  // m_mqtt_qos                           = 0;
  // m_mqtt_bRetain                       = false;
  // m_mqtt_keepalive                     = 60;
  // m_mqtt_bCleanSession                 = false;
  // m_mqtt_reconnect_delay               = 2;
  // m_mqtt_reconnect_delay_max           = 30;
  // m_mqtt_reconnect_exponential_backoff = false;
  // m_mqtt_bTLS                          = false;
  // m_mqtt_cafile                        = "";
  // m_mqtt_capath                        = "";
  // m_mqtt_certfile                      = "";
  // m_mqtt_keyfile                       = "";
  // m_mqtt_pwKeyfile                     = "";
  // m_mqtt_format                        = jsonfmt;

  m_topicDaemonBase = "vscp-daemon/{{srvguid}}/";
  m_topicDrivers    = m_topicDaemonBase + "drivers";
  m_topicDiscovery  = m_topicDaemonBase + "discovery";

  // Initialize MQTT
  if (MOSQ_ERR_SUCCESS != mosquitto_lib_init()) {
    spdlog::debug("controlobject: Unable to initialize mosquitto library.");
    return;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CControlObject::~CControlObject()
{
  if (gDebugLevel & VSCP_DEBUG_EXTRA) {
    spdlog::debug("controlobject:  Cleaning up");
  }

  if (0 != pthread_mutex_destroy(&m_mutex_DeviceList)) {
    spdlog::error("controlobject:  Unable to destroy m_mutex_DeviceList");
    return;
  }

  if (gDebugLevel & VSCP_DEBUG_EXTRA) {
    spdlog::error("controlobject:  Terminating the vscpd daemon.");
  }

  // Clean up SQLite lib allocations
  sqlite3_shutdown();
}

/////////////////////////////////////////////////////////////////////////////
// init
//

bool
CControlObject::init(std::string &strcfgfile, std::string &rootFolder)
{
  std::string str;
  spdlog::debug("Initialization starting.");

  // Save root folder for later use.
  m_rootFolder = rootFolder;

  // Root folder must exist
  if (!vscp_fileExists(m_rootFolder.c_str())) {
    spdlog::critical("The specified rootfolder does not exist ({}).", (const char *) m_rootFolder.c_str());
    return false;
  }

  // Change locale to get the correct decimal point "."
  setlocale(LC_NUMERIC, "C");

  // A configuration file must be available
  if (!vscp_fileExists(strcfgfile.c_str())) {
    spdlog::critical("No configuration file. Can't initialize!. Path={}", strcfgfile.c_str());
    return false;
  }

  ////////////////////////////////////////////////////////////////////////////
  //                           Read configuration
  ////////////////////////////////////////////////////////////////////////////

  spdlog::info("gDebugLevel={}", gDebugLevel);

  // Read JSON configuration
  if (gDebugLevel & VSCP_DEBUG_EXTRA) {
    spdlog::debug("Reading configuration file");
  }

  // Read configuration
  if (!readConfiguration(strcfgfile)) {
    spdlog::critical("Unable to open/parse/read configuration file. Can't initialize! Path ={}", strcfgfile.c_str());
    return FALSE;
  }

#ifndef WIN32
  if (m_runAsUser.length()) {
    int rv;
    struct passwd pw;
    struct passwd *result;
    char pwbuf[16384];

    rv = getpwnam_r(m_runAsUser.c_str(), &pw, pwbuf, sizeof(pwbuf), &result);
    if (nullptr == result) {
      if (0 == rv) {
        spdlog::critical("Trying to run as user but user {} is unknown to the system", m_runAsUser);
      }
      else {
        errno = rv;
        spdlog::error("Unable to become requested user {}", m_runAsUser.c_str());
      }
      return false;
    }

    if (setgid(pw.pw_gid) != 0) {
      spdlog::error("setgid() failed. [{}]", strerror(errno));
    }

    if (setuid(pw.pw_uid) != 0) {
      spdlog::error("setuid() failed. [{}]", strerror(errno));
    }
  }
#endif

  if (gDebugLevel & VSCP_DEBUG_EXTRA) {
    spdlog::debug("Using configuration file: {}", strcfgfile);
  }

  // Get GUID
  if (m_guid.isNULL()) {
    if (!getMacAddress(m_guid)) {
      // We failed to create GUID from MAC address use
      // 'localhost' IP instead as the base.
      spdlog::debug("Failed to get MAC address,. will use OP address instead.");
      getIPAddress(m_guid);
    }
  }

  str = "VSCP Server started - ";
  str += "Version: ";
  str += VSCPD_DISPLAY_VERSION;
  str += " - ";
  str += VSCPD_COPYRIGHT;
  spdlog::info(str.c_str());

  // Load class/type definitions from database if they should be loaded

  // Initialize the SQLite library
  if (SQLITE_OK != sqlite3_initialize()) {
    spdlog::critical("Unable to initialize SQLite library.");
    return false;
  }

  // Read in class/type info if available
  if (m_pathClassTypeDefinitionDb.length()) {

    sqlite3 *db_vscp_classtype;
    if (SQLITE_OK == sqlite3_open(m_pathClassTypeDefinitionDb.c_str(), &db_vscp_classtype)) {

      // * * *   C L A S S E S   * * *

      sqlite3_stmt *ppStmt;
      if (SQLITE_OK !=
          sqlite3_prepare(db_vscp_classtype, "SELECT class,name,token from vscp_class", -1, &ppStmt, NULL)) {
        spdlog::error("Failed to prepare class fetch from class & type database.");
      }

      while (SQLITE_ROW == sqlite3_step(ppStmt)) {
        uint16_t vscp_class              = (uint16_t) sqlite3_column_int(ppStmt, 0);
        std::string name                 = (const char *) sqlite3_column_text(ppStmt, 1);
        std::string token                = (const char *) sqlite3_column_text(ppStmt, 2);
        m_map_class_id2Token[vscp_class] = token;
        if (gDebugLevel & VSCP_DEBUG_EVENT_DATABASE) {
          spdlog::debug("Class = {} - ", m_map_class_id2Token[vscp_class]);
        }
        m_map_class_token2Id[token] = vscp_class;
        if (gDebugLevel & VSCP_DEBUG_EVENT_DATABASE) {
          spdlog::debug("Id = {}", m_map_class_token2Id[token]);
        }
      }
      sqlite3_finalize(ppStmt);

      // * * *   T Y P E S   * * *

      if (SQLITE_OK !=
          sqlite3_prepare(db_vscp_classtype, "SELECT type,link_to_class,token from vscp_type", -1, &ppStmt, NULL)) {
        spdlog::error("Failed to prepare type fetch from class & type database.");
      }

      while (SQLITE_ROW == sqlite3_step(ppStmt)) {
        uint16_t vscp_type                                     = (uint16_t) sqlite3_column_int(ppStmt, 0);
        uint16_t link_to_class                                 = (uint16_t) sqlite3_column_int(ppStmt, 1);
        std::string token                                      = (const char *) sqlite3_column_text(ppStmt, 2);
        m_map_type_id2Token[(link_to_class << 16) + vscp_type] = token;
        if (gDebugLevel & VSCP_DEBUG_EVENT_DATABASE) {
          spdlog::debug("Token = {} ", m_map_type_id2Token[(link_to_class << 16) + vscp_type]);
        }
        m_map_type_token2Id[token] = (link_to_class << 16) + vscp_type;
        if (gDebugLevel & VSCP_DEBUG_EVENT_DATABASE) {
          spdlog::debug("Id = {}", m_map_type_token2Id[token]);
        }
      }
      sqlite3_finalize(ppStmt);
      sqlite3_close(db_vscp_classtype);
    }
    else {
      spdlog::error("controlobject:  Failed to open VSCP class & type definition database {}. [{}]",
                    m_pathClassTypeDefinitionDb,
                    sqlite3_errmsg(db_vscp_classtype));
    }
  }

  // Open the discovery database (create if not available)
  if (m_pathMainDb.length()) {

    // If file does not exist. Create it and tables and indexes
    if (vscp_fileExists(m_pathMainDb)) {

      if (SQLITE_OK != sqlite3_open(m_pathMainDb.c_str(), &m_db_vscp_daemon)) {

        // Failed to open/create the database file
        spdlog::error("controlobject:  VSCP Daemon database could not be opened/created. - Path={} error={}",
                      m_pathMainDb,
                      sqlite3_errmsg(m_db_vscp_daemon));
        m_db_vscp_daemon = NULL;
      }
      else {

        // Read in discovered nodes to in memory map

        sqlite3_stmt *ppStmt;
        if (SQLITE_OK != sqlite3_prepare(m_db_vscp_daemon, "SELECT guid,name from discovery", -1, &ppStmt, NULL)) {
          spdlog::error("Failed to prepare discovery node fetch.");
        }

        while (SQLITE_ROW == sqlite3_step(ppStmt)) {
          std::string guid                = (const char *) sqlite3_column_text(ppStmt, 0);
          std::string name                = (const char *) sqlite3_column_text(ppStmt, 1);
          m_map_discoveryGuidToName[guid] = name;
          if (gDebugLevel & VSCP_DEBUG_MAIN_DATABASE) {
            spdlog::info("controlobject: Read in discovered nodes: guid={} - name={} ", guid, name);
          }
        }

        sqlite3_finalize(ppStmt);
      }
    }
    else {
      // Database does not exist. Must be created
      if (SQLITE_OK != sqlite3_open(m_pathMainDb.c_str(), &m_db_vscp_daemon)) {

        // Failed to open/create the database file
        spdlog::error(
          "controlobject:  VSCP Daemon database could not be opened/created (check access rights). - Path={} error={}",
          m_pathMainDb,
          sqlite3_errmsg(m_db_vscp_daemon));
        return false;
      }

      // We will try to create it
      spdlog::info("controlobject:  Will try to create VSCP Daemon database here {}.", m_pathMainDb);

      // Create settings db
      char *pErrMsg = 0;
      const char *psql;

      psql = "CREATE TABLE \"discovery\" (                          \
                \"id\"	INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,   \
                \"date\"	TEXT,                                     \
                \"guid\"	TEXT UNIQUE,                              \
                \"name\"	TEXT,                                     \
                \"description\"	TEXT                                \
                );";

      if (SQLITE_OK != sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg)) {
        spdlog::error("controlobject:  Creation of the VSCP database failed with message {}", pErrMsg);
        return false;
      }

      psql = "CREATE INDEX \"idxguid\" ON \"discovery\" (\"guid\")";

      if (SQLITE_OK != sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg)) {
        spdlog::error("controlobject:  Creation of the VSCP database index idxguid failed with message {}", pErrMsg);
        return false;
      }

      psql = "CREATE INDEX \"idxname\" ON \"discovery\" (\"name\")";

      if (SQLITE_OK != sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg)) {
        spdlog::error("controlobject:  Creation of the VSCP database index idxname failed with message {}", pErrMsg);
        return false;
      }

      // Add the daemon as the first database entry

      std::string sql = "INSERT INTO \"discovery\" (date, guid, name, description ) VALUES (";
      time_t now;
      char buf[50];
      time(&now);
      vscp_getISOTimeString(buf, sizeof(buf), &now);
      sql += "'";
      sql += buf;
      sql += "','";
      sql += m_guid.getAsString();
      sql += "','local-vscp-daemon','The local VSCP Daemon');";

      if (SQLITE_OK != sqlite3_exec(m_db_vscp_daemon, sql.c_str(), NULL, NULL, &pErrMsg)) {
        spdlog::error("controlobject:  Creation of the VSCP database index idxname failed with message {}", pErrMsg);
        return false;
      }

      // Add local host to in memory map
      m_map_discoveryGuidToName[m_guid.getAsString()] = "local-vscp-daemon";
    }
  }

  return true;
}

/////////////////////////////////////////////////////////////////////////////
// cleanup

bool
CControlObject::cleanup(void)
{
  if (gDebugLevel & VSCP_DEBUG_EXTRA) {
    spdlog::debug("controlobject:  cleanup - Giving worker threads time to stop "
                  "operations...");
  }

  if (gDebugLevel & VSCP_DEBUG_EXTRA) {
    spdlog::debug("controlobject:   cleanup - Stopping device worker thread...");
  }

  try {
    stopDeviceWorkerThreads();
  }
  catch (...) {
    spdlog::error("ControlObject: Exception occurred when stoping device worker threads.");
  }

  if (gDebugLevel & VSCP_DEBUG_EXTRA) {
    spdlog::debug("ControlObject: cleanup - Stopping VSCP Server worker thread...");
  }

  // // Disconnect from MQTT broker}
  // int rv = mosquitto_disconnect(m_mosq);
  // if (MOSQ_ERR_SUCCESS != rv) {
  //   if (MOSQ_ERR_INVAL == rv) {
  //     spdlog::error("ControlObject: mosquitto_disconnect: input parameters were invalid.");
  //   }
  //   else if (MOSQ_ERR_NO_CONN == rv) {
  //     spdlog::error("ControlObject: mosquitto_disconnect: client isn’t connected to a broker.");
  //   }
  // }

  // // stop the worker loop
  // rv = mosquitto_loop_stop(m_mosq, false);
  // if (MOSQ_ERR_SUCCESS != rv) {
  //   if (MOSQ_ERR_INVAL == rv) {
  //     spdlog::error("ControlObject: mosquitto_loop_stop: input parameters were invalid.");
  //   }
  //   else if (MOSQ_ERR_NOT_SUPPORTED == rv) {
  //     spdlog::error("ControlObject: mosquitto_loop_stop: thread support is not available.");
  //   }
  // }

  // // Clean up
  // mosquitto_destroy(m_mosq);

  // Disconnect in case were not
  m_mqttClient.disconnect();

  return true;
}

/////////////////////////////////////////////////////////////////////////////
// init_mqtt
//
// Initialize MQTT sub system
//

bool
CControlObject::init_mqtt()
{
  int rv;

  // Setup MQTT for server
  // if (!m_mqttClient.init()) {
  //   spdlog::error("Failed to initialize MQTT client.");
  //   return false;
  // }

  // Set GUID
  m_mqttClient.setSrvGuid(m_guid);
  m_mqttClient.setIfGuid(m_guid);

  // Add user escapes
  m_mqttClient.setUserEscape("server-name", m_strServerName);
  m_mqttClient.setUserEscape("root-folder", m_rootFolder);
  m_mqttClient.setUserEscape("server-user", m_runAsUser);
  m_mqttClient.setUserEscape("server-version", VSCPD_DISPLAY_VERSION);

  time_t t = time(NULL);
  char buf[80];
  std::string startSrvTime;
  if (vscp_getISOTimeString(buf, sizeof(buf), &t)) {
    startSrvTime = std::string(buf);
    m_mqttClient.setUserEscape("utc-start-time", std::string(buf));
  }

  m_mqttClient.setUserEscape("server-debug-level", vscp_str_format("0x%" PRIx64, gDebugLevel));

  // Add class/type tokens
  m_mqttClient.setTokenMaps(&m_map_class_id2Token, &m_map_type_id2Token);

  // Connect to server
  if (VSCP_ERROR_SUCCESS != m_mqttClient.connect()) {
    spdlog::error("Failed to connect to MQTT client {0} {1}.");
    return false;
  }

  // Publish server name
  {
    mustache subtemplate{ m_topicDaemonBase + "server-name" };
    data data;
    data.set("guid", m_guid.getAsString());
    data.set("srvguid", m_guid.getAsString());
    data.set("ifguid", m_guid.getAsString());
    std::string strTopic   = subtemplate.render(data);
    std::string strPayload = m_strServerName;
    if (MOSQ_ERR_SUCCESS != (rv = mosquitto_publish(m_mqttClient.getMqttHandle(),
                                                    NULL,
                                                    strTopic.c_str(),
                                                    (int) strPayload.length(),
                                                    strPayload.c_str(),
                                                    2,
                                                    true))) {
      spdlog::error("Failed to publish VSCP daemon name. error={0} {1}", rv, mosquitto_strerror(rv));
    }
  }

  {
    mustache subtemplate{ m_topicDaemonBase + "server-version" };
    data data;
    data.set("guid", m_guid.getAsString());
    data.set("srvguid", m_guid.getAsString());
    data.set("ifguid", m_guid.getAsString());
    std::string strTopic   = subtemplate.render(data);
    std::string strPayload = VSCPD_DISPLAY_VERSION;
    if (MOSQ_ERR_SUCCESS != (rv = mosquitto_publish(m_mqttClient.getMqttHandle(),
                                                    NULL,
                                                    strTopic.c_str(),
                                                    (int) strPayload.length(),
                                                    strPayload.c_str(),
                                                    2,
                                                    true))) {
      spdlog::error("Failed to publish VSCP daemon server version. error={0} {1}", rv, mosquitto_strerror(rv));
    }
  }

  // Start server date/time
  {
    mustache subtemplate{ m_topicDaemonBase + "server-started-utc" };
    data data;
    data.set("guid", m_guid.getAsString());
    data.set("srvguid", m_guid.getAsString());
    data.set("ifguid", m_guid.getAsString());
    std::string strTopic = subtemplate.render(data);
    spdlog::debug("drivers topic {}", strTopic);
    std::string strPayload = startSrvTime;
    spdlog::debug("drivers {}", strPayload);
    if (MOSQ_ERR_SUCCESS != (rv = mosquitto_publish(m_mqttClient.getMqttHandle(),
                                                    NULL,
                                                    strTopic.c_str(),
                                                    (int) strPayload.length(),
                                                    strPayload.c_str(),
                                                    2,
                                                    true))) {
      spdlog::error("Failed to publish VSCP daemon start time. error={0} {1}", rv, mosquitto_strerror(rv));
    }
  }

  // Publish drivers
  {
    mustache subtemplate{ m_topicDrivers };
    data data;
    data.set("guid", m_guid.getAsString());
    data.set("srvguid", m_guid.getAsString());
    data.set("ifguid", m_guid.getAsString());
    std::string strTopic = subtemplate.render(data);
    spdlog::debug("drivers topic {}", strTopic);
    std::string strPayload = m_deviceList.getAllAsJSON();
    spdlog::debug("drivers {}", strPayload);
    if (MOSQ_ERR_SUCCESS != (rv = mosquitto_publish(m_mqttClient.getMqttHandle(),
                                                    NULL,
                                                    strTopic.c_str(),
                                                    (int) strPayload.length(),
                                                    strPayload.c_str(),
                                                    2,
                                                    true))) {
      spdlog::error("Failed to publish VSCP daemon drivers. error={0} {1}", rv, mosquitto_strerror(rv));
    }
  }

  return true;
}

#ifdef WIN32

#define CLOCK_REALTIME 0 // dummy

static LARGE_INTEGER
getFILETIMEoffset()
{
  SYSTEMTIME s;
  FILETIME f;
  LARGE_INTEGER t;

  s.wYear         = 1970;
  s.wMonth        = 1;
  s.wDay          = 1;
  s.wHour         = 0;
  s.wMinute       = 0;
  s.wSecond       = 0;
  s.wMilliseconds = 0;
  SystemTimeToFileTime(&s, &f);
  t.QuadPart = f.dwHighDateTime;
  t.QuadPart <<= 32;
  t.QuadPart |= f.dwLowDateTime;
  return (t);
}

static int
clock_gettime(int X, struct timeval *tv)
{
  LARGE_INTEGER t;
  FILETIME f;
  double microseconds;
  static LARGE_INTEGER offset;
  static double frequencyToMicroseconds;
  static int initialized            = 0;
  static BOOL usePerformanceCounter = 0;

  if (!initialized) {
    LARGE_INTEGER performanceFrequency;
    initialized           = 1;
    usePerformanceCounter = QueryPerformanceFrequency(&performanceFrequency);
    if (usePerformanceCounter) {
      QueryPerformanceCounter(&offset);
      frequencyToMicroseconds = (double) performanceFrequency.QuadPart / 1000000.;
    }
    else {
      offset                  = getFILETIMEoffset();
      frequencyToMicroseconds = 10.;
    }
  }
  if (usePerformanceCounter)
    QueryPerformanceCounter(&t);
  else {
    GetSystemTimeAsFileTime(&f);
    t.QuadPart = f.dwHighDateTime;
    t.QuadPart <<= 32;
    t.QuadPart |= f.dwLowDateTime;
  }

  t.QuadPart -= offset.QuadPart;
  microseconds = (double) t.QuadPart / frequencyToMicroseconds;
  t.QuadPart   = (LONGLONG) microseconds;
  tv->tv_sec   = (long) t.QuadPart / 1000000;
  tv->tv_usec  = t.QuadPart % 1000000;
  return (0);
}

static void
usleep(__int64 usec)
{
  HANDLE timer;
  LARGE_INTEGER ft;

  ft.QuadPart = -(10 * usec); // Convert to 100 nanosecond interval, negative value indicates relative time

  timer = CreateWaitableTimer(NULL, TRUE, NULL);
  SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
  WaitForSingleObject(timer, INFINITE);
  CloseHandle(timer);
}
#endif

/////////////////////////////////////////////////////////////////////////////
// run - Program main loop
//
// Most work is done in the threads at the moment
//

bool
CControlObject::run(void)
{
  std::deque<CClientItem *>::iterator nodeClient;

  // if (nullptr == spdlog::get("logger")) {
  //   fprintf(stderr, "[Controlobject:run] Fatal error: Unable to get logger!");
  //   return false;
  // }

#ifdef WITH_SYSTEMD
  sd_notify(0, "READY=1");
#endif

  if (gDebugLevel & VSCP_DEBUG_EXTRA) {
    spdlog::debug("Controlobject: run");
  }

  // init MQTT
  try {
    if (!init_mqtt()) {
      spdlog::critical("controlobject: Failed to initialize MQTT.");
      return false;
    }
  }
  catch (...) {
    spdlog::critical("controlobject: Exception when initializing MQTT.");
    return false;
  }

  // Load drivers
  try {
    if (!startDeviceWorkerThreads()) {
      spdlog::critical("controlobject: Failed to load drivers.");
      return false;
    }
  }
  catch (...) {
    spdlog::critical("controlobject: Exception when loading drivers.");
    return false;
  }

  //-------------------------------------------------------------------------
  //                            MAIN - LOOP
  //-------------------------------------------------------------------------

#ifdef WIN32
  struct timeval now, old_now;
#else
  struct timespec now, old_now;
#endif
  clock_gettime(CLOCK_REALTIME, &old_now);
  old_now.tv_sec -= 60; // Do first send right away

  if (gDebugLevel & VSCP_DEBUG_EXTRA) {
    spdlog::debug("controlobject: Main loop starting...");
  }

  while (!m_bQuit) {

    clock_gettime(CLOCK_REALTIME, &now);

    // We send heartbeat every minute
    if ((now.tv_sec - old_now.tv_sec) > 60) {

      // Save time
      clock_gettime(CLOCK_REALTIME, &old_now);

      if (!periodicEvents()) {
        spdlog::error("Failed to send automation events.");
      }
    }

    usleep(5000);

#ifdef WITH_SYSTEMD
    sd_notify(0, "WATCHDOG=1");
#endif

  } // while

  if (gDebugLevel & VSCP_DEBUG_EXTRA) {
    spdlog::debug("controlobject: Main loop ending...");
  }

  spdlog::debug("Controlobject: starting shutdown");

#ifdef WITH_SYSTEMD
  sd_notify(0, "STOPPING=1");
#endif

  // Clean up is called in main file
  try {
    cleanup();
  }
  catch (...) {
  }

  if (gDebugLevel & VSCP_DEBUG_EXTRA) {
    spdlog::debug("Controlobject: end run");
  }

  return true;
}

/////////////////////////////////////////////////////////////////////////////
// sendEvent
//

bool
CControlObject::sendEvent(vscpEventEx *pex)
{
  uint8_t *pbuf = NULL; // Used for binary payload
  std::string strPayload;
  std::string strTopic;

  // Check pointer
  if (nullptr == pex) {
    spdlog::error("ControlObject: sendEvent: Event is NULL pointer");
    return false;
  }

  // send the event
  if (VSCP_ERROR_SUCCESS != m_mqttClient.send(*pex)) {
    return false;
  }

  return true;
}

/////////////////////////////////////////////////////////////////////////////
// periodicEvents
//

bool
CControlObject::periodicEvents(void)
{
  vscpEventEx ex;

  // Send VSCP_CLASS1_INFORMATION,
  // Type=9/VSCP_TYPE_INFORMATION_NODE_HEARTBEAT
  ex.obid      = 0; // IMPORTANT Must be set by caller before event is sent
  ex.head      = 0;
  ex.timestamp = vscp_makeTimeStamp();
  vscp_setEventExToNow(&ex); // Set time to current time
  ex.vscp_class = VSCP_CLASS1_INFORMATION;
  ex.vscp_type  = VSCP_TYPE_INFORMATION_NODE_HEARTBEAT;
  ex.sizeData   = 3;
  m_guid.writeGUID(ex.GUID);

  // GUID
  memcpy(ex.data + VSCP_CAPABILITY_OFFSET_GUID, m_guid.getGUID(), 16);

  ex.data[0] = 0; // index
  ex.data[1] = 0; // zone
  ex.data[2] = 0; // subzone

  if (!sendEvent(&ex)) {
    spdlog::error("Controlobject: Failed to send Class1 heartbeat");
  }

  // Send VSCP_CLASS2_INFORMATION,
  // Type=2/VSCP2_TYPE_INFORMATION_HEART_BEAT
  ex.obid      = 0; // IMPORTANT Must be set by caller before event is sent
  ex.head      = 0;
  ex.timestamp = vscp_makeTimeStamp();
  vscp_setEventExToNow(&ex); // Set time to current time
  ex.vscp_class = VSCP_CLASS2_INFORMATION;
  ex.vscp_type  = VSCP2_TYPE_INFORMATION_HEART_BEAT;
  ex.sizeData   = 64;
  m_guid.writeGUID(ex.GUID);

  // GUID
  memcpy(ex.data + VSCP_CAPABILITY_OFFSET_GUID, m_guid.getGUID(), 16);

  memset(ex.data, 0, sizeof(ex.data));
  memcpy(ex.data,
         m_strServerName.c_str(),
#ifdef WIN32
         min((int) strlen(m_strServerName.c_str()), 64));
#else
         std::min((int) strlen(m_strServerName.c_str()), 64));
#endif

  if (!sendEvent(&ex)) {
    spdlog::error("Controlobject: Failed to send Class2 heartbeat");
  }

  // Send VSCP_CLASS1_PROTOCOL,
  // Type=1/VSCP_TYPE_PROTOCOL_SEGCTRL_HEARTBEAT
  ex.obid      = 0; // IMPORTANT Must be set by caller before event is sent
  ex.head      = 0;
  ex.timestamp = vscp_makeTimeStamp();
  vscp_setEventExToNow(&ex); // Set time to current time
  ex.vscp_class = VSCP_CLASS1_PROTOCOL;
  ex.vscp_type  = VSCP_TYPE_PROTOCOL_SEGCTRL_HEARTBEAT;
  ex.sizeData   = 5;
  m_guid.writeGUID(ex.GUID);

  // GUID
  memcpy(ex.data + VSCP_CAPABILITY_OFFSET_GUID, m_guid.getGUID(), 16);

  time_t tnow;
  time(&tnow);
  uint32_t time32 = (uint32_t) tnow;

  ex.data[0] = 0;                                 // 8 - bit crc for VSCP daemon GUID
  ex.data[1] = (uint8_t) ((time32 >> 24) & 0xff); // Time since epoch MSB
  ex.data[2] = (uint8_t) ((time32 >> 16) & 0xff);
  ex.data[3] = (uint8_t) ((time32 >> 8) & 0xff);
  ex.data[4] = (uint8_t) ((time32) &0xff); // Time since epoch LSB

  if (!sendEvent(&ex)) {
    spdlog::error("Controlobject: Failed to send segment controller heartbeat");
  }

  // Send VSCP_CLASS2_PROTOCOL,
  // Type=20/VSCP2_TYPE_PROTOCOL_HIGH_END_SERVER_CAPS
  ex.obid      = 0; // IMPORTANT Must be set by caller before event is sent
  ex.head      = 0;
  ex.timestamp = vscp_makeTimeStamp();
  vscp_setEventExToNow(&ex); // Set time to current time
  ex.vscp_class = VSCP_CLASS2_PROTOCOL;
  ex.vscp_type  = VSCP2_TYPE_PROTOCOL_HIGH_END_SERVER_CAPS;
  m_guid.writeGUID(ex.GUID);

  // Fill in data
  memset(ex.data, 0, sizeof(ex.data));

  // GUID
  memcpy(ex.data + VSCP_CAPABILITY_OFFSET_GUID, m_guid.getGUID(), 16);

  // Server ip address
  cguid guid;
  if (getIPAddress(guid)) {
    ex.data[VSCP_CAPABILITY_OFFSET_IP_ADDR]     = guid.getAt(8);
    ex.data[VSCP_CAPABILITY_OFFSET_IP_ADDR + 1] = guid.getAt(9);
    ex.data[VSCP_CAPABILITY_OFFSET_IP_ADDR + 2] = guid.getAt(10);
    ex.data[VSCP_CAPABILITY_OFFSET_IP_ADDR + 3] = guid.getAt(11);
  }

  // Server name
  memcpy(ex.data + VSCP_CAPABILITY_OFFSET_SRV_NAME,
         (const char *) m_strServerName.c_str(),
#ifdef WIN32
         min((int) strlen((const char *) m_strServerName.c_str()), 64));
#else
         std::min((int) strlen((const char *) m_strServerName.c_str()), 64));
#endif

  // Capabilities array
  getVscpCapabilities(ex.data);

  // non-standard ports
  // TODO(AKHE)

  ex.sizeData = 104;
  m_guid.writeGUID(ex.GUID);

  if (!sendEvent(&ex)) {
    spdlog::error("Controlobject: Failed to send high end server capabilities.");
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// discovery
//

void
CControlObject::discovery(vscpEvent *pev)
{
  int rv;

  // Check pointer
  if (nullptr == pev) {
    spdlog::error("ControlObject: node discover: Event is NULL pointer.");
    return;
  }

  // If not used - skip out
  if (nullptr == m_db_vscp_daemon) {
    return;
  }

  char *pErrMsg = 0;
  cguid guid(pev->GUID);

  if (!m_map_discoveryGuidToName[guid.getAsString()].length()) {

    // Add the daemon as the first database entry

    std::string sql = "INSERT INTO \"discovery\" (date, guid, name, description ) VALUES (";
    time_t now;
    char buf[50];
    time(&now);
    vscp_getISOTimeString(buf, sizeof(buf), &now);
    sql += "'";
    sql += buf;
    sql += "','";
    sql += guid.getAsString();
    sql += "','discovered','');";

    if (SQLITE_OK != sqlite3_exec(m_db_vscp_daemon, sql.c_str(), NULL, NULL, &pErrMsg)) {
      spdlog::info("Failed to add discovery record [{}] Error msg: {}", sql, pErrMsg);
      return;
    }

    // Publish on discovery topic
    // Fix publish topics
    json j;
    j["guid"] = guid.getAsString();
    j["time"] = buf;
    mustache subtemplate{ m_topicDiscovery };
    data data;
    data.set("guid", m_guid.getAsString());
    data.set("srvguid", m_guid.getAsString());
    data.set("ifguid", m_guid.getAsString());
    std::string strTopic = subtemplate.render(data);

    if (MOSQ_ERR_SUCCESS != (rv = mosquitto_publish(m_mqttClient.getMqttHandle(),
                                                    NULL,
                                                    strTopic.c_str(),
                                                    (int) j.dump().length(),
                                                    j.dump().c_str(),
                                                    1,
                                                    true))) {
      spdlog::error("Failed to publish VSCP daemon discovery. error={0} {1}", rv, mosquitto_strerror(rv));
    }

    // Add local host to in memory map
    m_map_discoveryGuidToName[guid.getAsString()] = "Discovered";
  }
}

////////////////////////////////////////////////////////////////////////////////
// getVscpCapabilities
//

bool
CControlObject::getVscpCapabilities(uint8_t *pCaps)
{
  // Check pointer
  if (nullptr == pCaps) {
    spdlog::error("ControObject: getVscpCapabilities: NULL pointer.");
    return false;
  }

  // TODO(AKHE)

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// startDeviceWorkerThreads
//

bool
CControlObject::startDeviceWorkerThreads(void)
{
  CDeviceItem *pDeviceItem;
  if (gDebugLevel & VSCP_DEBUG_EXTRA) {
    spdlog::debug("controlobject: [Driver] - Starting drivers...");
  }

  std::deque<CDeviceItem *>::iterator it;
  for (it = m_deviceList.m_devItemList.begin(); it != m_deviceList.m_devItemList.end(); ++it) {

    pDeviceItem = *it;
    if (NULL != pDeviceItem) {

      if (gDebugLevel & VSCP_DEBUG_EXTRA) {
        spdlog::debug("Controlobject: [Driver] - Preparing: {} ", pDeviceItem->m_strName);
      }

      // Just start if enabled
      if (!pDeviceItem->m_bEnable)
        continue;

      if (gDebugLevel & VSCP_DEBUG_EXTRA) {
        spdlog::debug("Controlobject: [Driver] - Starting: {} ", pDeviceItem->m_strName);
      }

      // Start  the driver logic
      pDeviceItem->startDriver(this);

    } // Valid device item
    
  }

  if (gDebugLevel & VSCP_DEBUG_EXTRA) {
      spdlog::debug("controlobject: [Driver] - Drivers started (if any)");
    }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// stopDeviceWorkerThreads
//

bool
CControlObject::stopDeviceWorkerThreads(void)
{
  CDeviceItem *pDeviceItem;

  if (gDebugLevel & VSCP_DEBUG_EXTRA) {
    spdlog::debug("controlobject: [Driver] - Stopping drivers...");
  }
  std::deque<CDeviceItem *>::iterator iter;
  for (iter = m_deviceList.m_devItemList.begin(); iter != m_deviceList.m_devItemList.end(); ++iter) {

    pDeviceItem = *iter;
    if (NULL != pDeviceItem) {
      if (gDebugLevel & VSCP_DEBUG_EXTRA) {
        spdlog::debug("Controlobject: [Driver] - Stopping: {} ", pDeviceItem->m_strName);
      }
      pDeviceItem->stopDriver();
    }
  }

  return true;
}

// ----------------------------------------------------------------------------

#ifdef WIN32
static bool
GetAdapterInfo(int nAdapterNum, std::string &sMAC)
{
  // Reset the LAN adapter so that we can begin querying it
  NCB Ncb;
  memset(&Ncb, 0, sizeof(Ncb));
  Ncb.ncb_command  = NCBRESET;
  Ncb.ncb_lana_num = nAdapterNum;
  if (Netbios(&Ncb) != NRC_GOODRET) {
    char acTemp[80];
    std::ostrstream outs(acTemp, sizeof(acTemp));
    outs << "error " << Ncb.ncb_retcode << " on reset" << std::ends;
    sMAC = acTemp;
    return false;
  }

  // Prepare to get the adapter status block
  memset(&Ncb, 0, sizeof(Ncb));
  Ncb.ncb_command  = NCBASTAT;
  Ncb.ncb_lana_num = nAdapterNum;
  strcpy((char *) Ncb.ncb_callname, "*");
  struct ASTAT {
    ADAPTER_STATUS adapt;
    NAME_BUFFER NameBuff[30];
  } Adapter;
  memset(&Adapter, 0, sizeof(Adapter));
  Ncb.ncb_buffer = (unsigned char *) &Adapter;
  Ncb.ncb_length = sizeof(Adapter);

  // Get the adapter's info and, if this works, return it in standard,
  // colon-delimited form.
  if (Netbios(&Ncb) == 0) {
    char acMAC[18];
    sprintf(acMAC,
            "%02X:%02X:%02X:%02X:%02X:%02X",
            int(Adapter.adapt.adapter_address[0]),
            int(Adapter.adapt.adapter_address[1]),
            int(Adapter.adapt.adapter_address[2]),
            int(Adapter.adapt.adapter_address[3]),
            int(Adapter.adapt.adapter_address[4]),
            int(Adapter.adapt.adapter_address[5]));
    sMAC = acMAC;
    return true;
  }
  else {
    char acTemp[80];
    std::ostrstream outs(acTemp, sizeof(acTemp));
    outs << "error " << Ncb.ncb_retcode << " on ASTAT" << std::ends;
    sMAC = acTemp;
    return false;
  }
}
#endif

///////////////////////////////////////////////////////////////////////////////
//  getMacAddress
//

bool
CControlObject::getMacAddress(cguid &guid)
{
#ifdef WIN32
  // https://tangentsoft.net/wskfaq/examples/getmac-netbios.html
  bool rv = false;
  std::string strMac;
  if (!GetAdapterInfo(0, strMac)) {
    return false;
  }

  // Construct MAC GUID
  strMac = "FF:FF:FF:FF:FF:FF:FF:FE:" + strMac + ":00:00";

  // NCB Ncb;
  // UCHAR uRetCode;
  // LANA_ENUM lenum;
  // int i;

  // // Clear the GUID
  // guid.clear();

  // memset(&Ncb, 0, sizeof(Ncb));
  // Ncb.ncb_command = NCBENUM;
  // Ncb.ncb_buffer  = (UCHAR*)&lenum;
  // Ncb.ncb_length  = sizeof(lenum);
  // uRetCode        = Netbios(&Ncb);
  // // printf( "The NCBENUM return code is: 0x%x ", uRetCode );

  // for (i = 0; i < lenum.length; i++) {
  //     memset(&Ncb, 0, sizeof(Ncb));
  //     Ncb.ncb_command  = NCBRESET;
  //     Ncb.ncb_lana_num = lenum.lana[i];

  //     uRetCode = Netbios(&Ncb);

  //     memset(&Ncb, 0, sizeof(Ncb));
  //     Ncb.ncb_command  = NCBASTAT;
  //     Ncb.ncb_lana_num = lenum.lana[i];

  //     strcpy((char*)Ncb.ncb_callname, "*               ");
  //     struct ASTAT {
  //         ADAPTER_STATUS adapt;
  //         NAME_BUFFER NameBuff[30];
  //     } Adapter;
  //     memset(&Adapter, 0, sizeof(Adapter));
  //     Ncb.ncb_buffer = (unsigned char*)&Adapter;
  //     Ncb.ncb_length = sizeof(Adapter);

  //     uRetCode = Netbios(&Ncb);

  //     if (uRetCode == 0) {
  //         guid.setAt(0, 0xff);
  //         guid.setAt(1, 0xff);
  //         guid.setAt(2, 0xff);
  //         guid.setAt(3, 0xff);
  //         guid.setAt(4, 0xff);
  //         guid.setAt(5, 0xff);
  //         guid.setAt(6, 0xff);
  //         guid.setAt(7, 0xfe);
  //         guid.setAt(8, Adapter.adapt.adapter_address[0]);
  //         guid.setAt(9, Adapter.adapt.adapter_address[1]);
  //         guid.setAt(10, Adapter.adapt.adapter_address[2]);
  //         guid.setAt(11, Adapter.adapt.adapter_address[3]);
  //         guid.setAt(12, Adapter.adapt.adapter_address[4]);
  //         guid.setAt(13, Adapter.adapt.adapter_address[5]);
  //         guid.setAt(14, 0);
  //         guid.setAt(15, 0);
#ifdef DEBUG__
  char buf[256];
  sprintf(buf,
          "The Ethernet MAC Address: %02x:%02x:%02x:%02x:%02x:%02x",
          guid.getAt(2),
          guid.getAt(3),
          guid.getAt(4),
          guid.getAt(5),
          guid.getAt(6),
          guid.getAt(7));

  std::string str = std::string(buf);
#endif

  //         rv = true;
  //     }
  // }

  return rv;

#else
  // cat /sys/class/net/eth0/address
  bool rv = true;
  struct ifreq s;
  int fd;

  // Clear the GUID
  guid.clear();

  fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
  if (-1 == fd) {
    return false;
  }

  memset(&s, 0, sizeof(s));
  strcpy(s.ifr_name, "eth0");

  if (0 == ioctl(fd, SIOCGIFHWADDR, &s)) {

    // ptr = (unsigned char *)&s.ifr_ifru.ifru_hwaddr.sa_data[0];
    if (gDebugLevel & VSCP_DEBUG_EXTRA) {
      spdlog::debug("Ethernet MAC address: {0:02X]:{1:02X}:{2:02X}:{3:02X}:{4:02X}:{5:02X}",
                    (uint8_t) s.ifr_addr.sa_data[0],
                    (uint8_t) s.ifr_addr.sa_data[1],
                    (uint8_t) s.ifr_addr.sa_data[2],
                    (uint8_t) s.ifr_addr.sa_data[3],
                    (uint8_t) s.ifr_addr.sa_data[4],
                    (uint8_t) s.ifr_addr.sa_data[5]);
    }

    guid.setAt(0, 0xff);
    guid.setAt(1, 0xff);
    guid.setAt(2, 0xff);
    guid.setAt(3, 0xff);
    guid.setAt(4, 0xff);
    guid.setAt(5, 0xff);
    guid.setAt(6, 0xff);
    guid.setAt(7, 0xfe);
    guid.setAt(8, s.ifr_addr.sa_data[0]);
    guid.setAt(9, s.ifr_addr.sa_data[1]);
    guid.setAt(10, s.ifr_addr.sa_data[2]);
    guid.setAt(11, s.ifr_addr.sa_data[3]);
    guid.setAt(12, s.ifr_addr.sa_data[4]);
    guid.setAt(13, s.ifr_addr.sa_data[5]);
    guid.setAt(14, 0);
    guid.setAt(15, 0);
  }
  else {
    spdlog::error("Failed to get hardware address (must be root?).");
    rv = false;
  }

  return rv;

#endif
}

///////////////////////////////////////////////////////////////////////////////
//  getIPAddress
//

bool
CControlObject::getIPAddress(cguid &guid)
{
  // Clear the GUID
  guid.clear();

  guid.setAt(0, 0xff);
  guid.setAt(1, 0xff);
  guid.setAt(2, 0xff);
  guid.setAt(3, 0xff);
  guid.setAt(4, 0xff);
  guid.setAt(5, 0xff);
  guid.setAt(6, 0xff);
  guid.setAt(7, 0xfd);

  char szName[128];
  gethostname(szName, sizeof(szName));
#if defined(_WIN32)
  LPHOSTENT lpLocalHostEntry;
#else
  struct hostent *lpLocalHostEntry;
#endif
  lpLocalHostEntry = gethostbyname(szName);
  if (nullptr == lpLocalHostEntry) {
    return false;
  }

  // Get all local addresses
  int idx = -1;
  void *pAddr;
  unsigned long localaddr[16]; // max 16 local addresses
  do {
    idx++;
    localaddr[idx] = 0;
    pAddr          = lpLocalHostEntry->h_addr_list[idx];
    if (NULL != pAddr)
      localaddr[idx] = *((unsigned long *) pAddr);
  } while ((NULL != pAddr) && (idx < 16));

  guid.setAt(8, (localaddr[0] >> 24) & 0xff);
  guid.setAt(9, (localaddr[0] >> 16) & 0xff);
  guid.setAt(10, (localaddr[0] >> 8) & 0xff);
  guid.setAt(11, localaddr[0] & 0xff);

  guid.setAt(12, 0);
  guid.setAt(13, 0);
  guid.setAt(14, 0);
  guid.setAt(15, 0);

  return true;
}

/////////////////////////////////////////////////////////////////////////////
// readEncryptionKey
//

bool
CControlObject::readEncryptionKey(const std::string &path)
{
  bool rv = true;
  spdlog::debug("Reading encryption key {}", path);
  try {
    std::ifstream in(path, std::ifstream::in);
    std::stringstream strStream;
    strStream << in.rdbuf();
    std::string hexstr = strStream.str();
    spdlog::debug("key={}", hexstr);
    rv = (32 == vscp_hexStr2ByteArray(__vscp_key, 32, hexstr.c_str()));
  }
  catch (...) {
    spdlog::error("[vscpl2drv-tcpipsrv] Failed to read encryption key file {}", path.c_str());
    rv = false;
  }

  return rv;
}

///////////////////////////////////////////////////////////////////////////////
// readJSON
//
// Read the configuration JSON data
//

bool
CControlObject::readJSON(const json &j)
{
  // ********************************************************************************
  //                                Main General
  // ********************************************************************************
  try {
    if (j.contains("runasuser")) {
      m_runAsUser = j["runasuser"].get<std::string>();
    }
    else {
      spdlog::error("ReadConfig: Failed to read 'runasuser'. Must be present.");
      return false;
    }

    if (gDebugLevel & VSCP_DEBUG_CONFIG) {
      spdlog::debug("ReadConfig: 'runasuser' set to {}", m_runAsUser.c_str());
    }
  }
  catch (...) {
    spdlog::error("ReadConfig: Failed to read 'runasuser'. Must be present.");
    return false;
  }

  try {
    if (j.contains("guid")) {
      m_guid.getFromString(j["guid"].get<std::string>());
    }
    else {
      spdlog::error("ReadConfig: Failed to read 'guid'. Must be present.");
      return false;
    }

    if (gDebugLevel & VSCP_DEBUG_CONFIG) {
      spdlog::debug("ReadConfig: 'guid' set to {}", m_guid.getAsString());
    }
  }
  catch (...) {
    spdlog::error("ReadConfig: Failed to read 'guid'. Must be present.");
    return false;
  }

  try {
    if (j.contains("servername")) {
      m_strServerName = j["servername"].get<std::string>();
    }
    else {
      spdlog::error("ReadConfig: Failed to read 'servername'. Must be present.");
      return false;
    }

    if (gDebugLevel & VSCP_DEBUG_CONFIG) {
      spdlog::debug("ReadConfig: 'servername' set to {}", m_strServerName);
    }
  }
  catch (...) {
    spdlog::error("ReadConfig: Failed to read 'servername'. Must be present.");
    return false;
  }

  try {
    if (j.contains("debug")) {
      if (j["debug"].is_number()) {
        gDebugLevel = j["debug"].get<uint64_t>();
      }
      else if (j["debug"].is_string()) {
        std::string str = j["debug"].get<std::string>();
        vscp_makeLower(str);
        vscp_trim(str);
        gDebugLevel = strtoull(str.c_str(), NULL, 16);
      }
      else {
        spdlog::error("ReadConfig: Failed to read 'debug'. Not string. Not number. What?");
      }
    }
    else {
      spdlog::error("ReadConfig: Failed to read 'debug'. Must be present.");
      return false;
    }

    if (gDebugLevel & VSCP_DEBUG_CONFIG) {
      spdlog::debug("ReadConfig: 'debug' set to {0:x}", gDebugLevel);
    }
  }
  catch (...) {
    spdlog::error("ReadConfig: Failed to read 'debug'. Must be present.");
    return false;
  }

  try {
    if (j.contains("classtypedb")) {
      m_pathClassTypeDefinitionDb = j["classtypedb"].get<std::string>();
    }
    else {
      spdlog::error("ReadConfig: Failed to read 'classtypedb'. Must be present.");
      return false;
    }

    if (gDebugLevel & VSCP_DEBUG_CONFIG) {
      spdlog::debug("ReadConfig: 'classtypedb' set to {}", m_pathClassTypeDefinitionDb);
    }
  }
  catch (...) {
    spdlog::error("ReadConfig: Failed to read 'classtypedb'. Must be present.");
    return false;
  }

  try {
    if (j.contains("maindb")) {
      m_pathMainDb = j["maindb"].get<std::string>();
    }
    else {
      spdlog::error("ReadConfig: Failed to read 'maindb'. Must be present.");
      return false;
    }

    if (gDebugLevel & VSCP_DEBUG_CONFIG) {
      spdlog::debug("ReadConfig: 'maindb' set to {}", m_pathMainDb);
    }
  }
  catch (...) {
    spdlog::error("ReadConfig: Failed to read 'maindb'. Must be present.");
    return false;
  }

  try {
    std::string pathvscpkey;
    if (j.contains("vscpkey")) {
      pathvscpkey = j["vscpkey"].get<std::string>();
    }
    else {
      spdlog::error("ReadConfig: 'Failed to read in encryption key {}", pathvscpkey);
    }

    if (gDebugLevel & VSCP_DEBUG_CONFIG) {
      spdlog::debug("ReadConfig: 'path to vscp key' set to {}", pathvscpkey);
    }

    if (!readEncryptionKey(pathvscpkey)) {
      spdlog::error(
        "ReadConfig: 'Failed to read in encryption key {} (default key will be used). Not secure. MUST be fixed!",
        pathvscpkey);
    }
  }
  catch (...) {
    spdlog::error("ReadConfig: Failed to read 'vscpkey'. Must be present.");
    return false;
  }

  // Logging
  if (!(j.contains("logging") && j["logging"].is_object())) {
    if (gDebugLevel & VSCP_DEBUG_CONFIG) {
      spdlog::debug("ReadConfig: logging object. Defaults will be used for all values.");
    }
  }
  else {

    // Logging: file-enable-log
    if (j["logging"].contains("file-enable-log")) {
      try {
        m_bEnableFileLog = j["logging"]["file-enable-log"].get<bool>();
      }
      catch (const std::exception &ex) {
        spdlog::error("Failed to read 'file-enable-log' Error='{}'", ex.what());
      }
      catch (...) {
        spdlog::error("Failed to read 'file-enable-log' due to unknown error.");
      }
      if (gDebugLevel & VSCP_DEBUG_CONFIG) {
        spdlog::debug("ReadConfig: LOGGING 'file-enable-log' set to {}", m_bEnableFileLog ? "true" : "false");
      }
    }
    else {
      if (gDebugLevel & VSCP_DEBUG_CONFIG) {
        spdlog::debug("ReadConfig: Failed to read LOGGING 'file-enable-log' Defaults will be used.");
      }
    }

    // Logging: file-log-level
    if (j["logging"].contains("file-log-level")) {
      std::string str;
      try {
        str = j["logging"]["file-log-level"].get<std::string>();
      }
      catch (const std::exception &ex) {
        spdlog::error("Failed to read 'file-log-level' Error='{}'", ex.what());
      }
      catch (...) {
        spdlog::error("Failed to read 'file-log-level' due to unknown error.");
      }
      vscp_makeLower(str);
      if (std::string::npos != str.find("off")) {
        m_fileLogLevel = spdlog::level::off;
        if (gDebugLevel & VSCP_DEBUG_CONFIG) {
          spdlog::debug("ReadConfig: LOGGING 'file-log-level' set to 'off'.");
        }
      }
      else if (std::string::npos != str.find("critical")) {
        m_fileLogLevel = spdlog::level::critical;
        if (gDebugLevel & VSCP_DEBUG_CONFIG) {
          spdlog::debug("ReadConfig: LOGGING 'file-log-level' set to 'critical'.");
        }
      }
      else if (std::string::npos != str.find("err")) {
        m_fileLogLevel = spdlog::level::err;
        if (gDebugLevel & VSCP_DEBUG_CONFIG) {
          spdlog::debug("ReadConfig: LOGGING 'file-log-level' set to 'err'.");
        }
      }
      else if (std::string::npos != str.find("warn")) {
        m_fileLogLevel = spdlog::level::warn;
        if (gDebugLevel & VSCP_DEBUG_CONFIG) {
          spdlog::debug("ReadConfig: LOGGING 'file-log-level' set to 'warn'.");
        }
      }
      else if (std::string::npos != str.find("info")) {
        m_fileLogLevel = spdlog::level::info;
        if (gDebugLevel & VSCP_DEBUG_CONFIG) {
          spdlog::debug("ReadConfig: LOGGING 'file-log-level' set to 'info'.");
        }
      }
      else if (std::string::npos != str.find("debug")) {
        m_fileLogLevel = spdlog::level::debug;
        if (gDebugLevel & VSCP_DEBUG_CONFIG) {
          spdlog::debug("ReadConfig: LOGGING 'file-log-level' set to 'debug'.");
        }
      }
      else if (std::string::npos != str.find("trace")) {
        m_fileLogLevel = spdlog::level::trace;
        if (gDebugLevel & VSCP_DEBUG_CONFIG) {
          spdlog::debug("ReadConfig: LOGGING 'file-log-level' set to 'trace'.");
        }
      }
      else {
        spdlog::debug("ReadConfig: LOGGING 'file-log-level' has invalid value [{}]. Default value used.", str);
      }
    }
    else {
      if (gDebugLevel & VSCP_DEBUG_CONFIG) {
        spdlog::debug("ReadConfig: Failed to read LOGGING 'file-log-level' Defaults will be used.");
      }
    }

    // Logging: file-pattern
    if (j["logging"].contains("file-pattern")) {
      try {
        m_fileLogPattern = j["logging"]["file-pattern"].get<std::string>();
      }
      catch (const std::exception &ex) {
        spdlog::error("Failed to read 'file-pattern' Error='{}'", ex.what());
      }
      catch (...) {
        spdlog::error("Failed to read 'file-pattern' due to unknown error.");
      }
      if (gDebugLevel & VSCP_DEBUG_CONFIG) {
        spdlog::debug("ReadConfig: LOGGING 'file-pattern' set to {}.", m_fileLogPattern);
      }
    }
    else {
      if (gDebugLevel & VSCP_DEBUG_CONFIG) {
        spdlog::debug("ReadConfig: Failed to read LOGGING 'file-pattern' Defaults will be used.");
      }
    }

    // Logging: file-path
    if (j["logging"].contains("file-path")) {
      try {
        m_path_to_log_file = j["logging"]["file-path"].get<std::string>();
      }
      catch (const std::exception &ex) {
        spdlog::error("Failed to read 'file-path' Error='{}'", ex.what());
      }
      catch (...) {
        spdlog::error("Failed to read 'file-path' due to unknown error.");
      }
      if (gDebugLevel & VSCP_DEBUG_CONFIG) {
        spdlog::debug("ReadConfig: LOGGING 'file-path' set to '{}'.", m_path_to_log_file);
      }
    }
    else {
      if (gDebugLevel & VSCP_DEBUG_CONFIG) {
        spdlog::debug("ReadConfig: Failed to read LOGGING 'file-path' Defaults will be used.");
      }
    }

    // Logging: file-max-size
    if (j["logging"].contains("file-max-size")) {
      try {
        m_max_log_size = j["logging"]["file-max-size"].get<uint32_t>();
      }
      catch (const std::exception &ex) {
        spdlog::error("Failed to read 'file-max-size' Error='{}'", ex.what());
      }
      catch (...) {
        spdlog::error("Failed to read 'file-max-size' due to unknown error.");
      }
      if (gDebugLevel & VSCP_DEBUG_CONFIG) {
        spdlog::debug("ReadConfig: LOGGING 'file-max-size' set to '{}'.", m_max_log_size);
      }
    }
    else {
      if (gDebugLevel & VSCP_DEBUG_CONFIG) {
        spdlog::debug("ReadConfig: Failed to read LOGGING 'file-max-size' Defaults will be used.");
      }
    }

    // Logging: file-max-files
    if (j["logging"].contains("file-max-files")) {
      try {
        m_max_log_files = j["logging"]["file-max-files"].get<uint16_t>();
      }
      catch (const std::exception &ex) {
        spdlog::error("Failed to read 'file-max-files' Error='{}'", ex.what());
      }
      catch (...) {
        spdlog::error("Failed to read 'file-max-files' due to unknown error.");
      }
      if (gDebugLevel & VSCP_DEBUG_CONFIG) {
        spdlog::debug("ReadConfig: LOGGING 'file-max-files' set to '{}'.", m_max_log_files);
      }
    }
    else {
      if (gDebugLevel & VSCP_DEBUG_CONFIG) {
        spdlog::debug("ReadConfig: Failed to read LOGGING 'file-max-files' Defaults will be used.");
      }
    }

    // Console

    // Logging: console-enable-log
    if (j["logging"].contains("console-enable-log")) {
      try {
        m_bEnableConsoleLog = j["logging"]["console-enable-log"].get<bool>();
      }
      catch (const std::exception &ex) {
        spdlog::error("Failed to read 'console-enable-log' Error='{}'", ex.what());
      }
      catch (...) {
        spdlog::error("Failed to read 'console-enable-log' due to unknown error.");
      }
      if (gDebugLevel & VSCP_DEBUG_CONFIG) {
        spdlog::debug("ReadConfig: LOGGING 'console-enable-log' set to {}", m_bEnableConsoleLog ? "true" : "false");
      }
    }
    else {
      if (gDebugLevel & VSCP_DEBUG_CONFIG) {
        spdlog::debug("ReadConfig: Failed to read LOGGING 'console-enable-log' Defaults will be used.");
      }
    }

    // Logging: console-log-level
    if (j["logging"].contains("console-log-level")) {
      std::string str;
      try {
        str = j["logging"]["console-log-level"].get<std::string>();
      }
      catch (const std::exception &ex) {
        spdlog::error("Failed to read 'console-enable-level' Error='{}'", ex.what());
      }
      catch (...) {
        spdlog::error("Failed to read 'console-enable-level' due to unknown error.");
      }
      vscp_makeLower(str);
      if (std::string::npos != str.find("off")) {
        m_consoleLogLevel = spdlog::level::off;
        if (gDebugLevel & VSCP_DEBUG_CONFIG) {
          spdlog::debug("ReadConfig: LOGGING 'console-log-level' set to 'off'.");
        }
      }
      else if (std::string::npos != str.find("critical")) {
        m_consoleLogLevel = spdlog::level::critical;
        if (gDebugLevel & VSCP_DEBUG_CONFIG) {
          spdlog::debug("ReadConfig: LOGGING 'console-log-level' set to 'critical'.");
        }
      }
      else if (std::string::npos != str.find("err")) {
        m_consoleLogLevel = spdlog::level::err;
        if (gDebugLevel & VSCP_DEBUG_CONFIG) {
          spdlog::debug("ReadConfig: LOGGING 'console-log-level' set to 'err'.");
        }
      }
      else if (std::string::npos != str.find("warn")) {
        m_consoleLogLevel = spdlog::level::warn;
        if (gDebugLevel & VSCP_DEBUG_CONFIG) {
          spdlog::debug("ReadConfig: LOGGING 'console-log-level' set to 'warn'.");
        }
      }
      else if (std::string::npos != str.find("info")) {
        m_consoleLogLevel = spdlog::level::info;
        if (gDebugLevel & VSCP_DEBUG_CONFIG) {
          spdlog::debug("ReadConfig: LOGGING 'console-log-level' set to 'info'.");
        }
      }
      else if (std::string::npos != str.find("debug")) {
        m_consoleLogLevel = spdlog::level::debug;
        if (gDebugLevel & VSCP_DEBUG_CONFIG) {
          spdlog::debug("ReadConfig: LOGGING 'console-log-level' set to 'debug'.");
        }
      }
      else if (std::string::npos != str.find("trace")) {
        m_consoleLogLevel = spdlog::level::trace;
        if (gDebugLevel & VSCP_DEBUG_CONFIG) {
          spdlog::debug("ReadConfig: LOGGING 'console-log-level' set to 'trace'.");
        }
      }
      else {
        spdlog::debug("ReadConfig: LOGGING 'console-log-level' has invalid value [{}]. Default value used.", str);
      }
    }
    else {
      if (gDebugLevel & VSCP_DEBUG_CONFIG) {
        spdlog::debug("ReadConfig: Failed to read LOGGING 'file-log-level' Defaults will be used.");
      }
    }

    // Logging: console-pattern
    if (j["logging"].contains("console-pattern")) {
      try {
        m_consoleLogPattern = j["logging"]["console-pattern"].get<std::string>();
      }
      catch (const std::exception &ex) {
        spdlog::error("Failed to read 'console-pattern' Error='{}'", ex.what());
      }
      catch (...) {
        spdlog::error("Failed to read 'console-pattern' due to unknown error.");
      }
      if (gDebugLevel & VSCP_DEBUG_CONFIG) {
        spdlog::debug("ReadConfig: LOGGING 'console-pattern' set to {}.", m_consoleLogPattern);
      }
    }
    else {
      if (gDebugLevel & VSCP_DEBUG_CONFIG) {
        spdlog::debug("ReadConfig: Failed to read LOGGING 'console-pattern' Defaults will be used.");
      }
    }
  }

  // ********************************************************************************
  //                                     Main MQTT
  // ********************************************************************************

  if (!(j.contains("mqtt") && j["mqtt"].is_object())) {
    if (gDebugLevel & VSCP_DEBUG_CONFIG) {
      spdlog::debug("ReadConfig: mqtt object. Defaults will be used.");
    }
  }
  else {

    // Do MQTT configuration
    if (!m_mqttClient.initFromJson(j["mqtt"].dump())) {
      spdlog::error("ReadConfig: Failed to initialize MQTT.");
      return false;
    }

    // * * * Extra MQTT info * * *

    // MQTT topic-drivers && j["mqtt"]["topic-deamon-base"].is_string()

    if (j["mqtt"].contains("topic-daemon-base")) {
      m_topicDaemonBase = j["mqtt"]["topic-daemon-base"].get<std::string>();
      if (gDebugLevel & VSCP_DEBUG_CONFIG) {
        spdlog::debug("ReadConfig: MQTT 'topic-daemon-base' set to {}", m_topicDaemonBase);
      }
    }
    else {
      if (gDebugLevel & VSCP_DEBUG_CONFIG) {
        spdlog::debug("ReadConfig: Failed to read MQTT 'topic-daemon-base'. Defaults will be used \"\".");
      }
    }

    if (j["mqtt"].contains("topic-drivers")) {
      m_topicDrivers = j["mqtt"]["topic-drivers"].get<std::string>();
      if (gDebugLevel & VSCP_DEBUG_CONFIG) {
        spdlog::debug("ReadConfig: MQTT 'topic-drivers' set to {}", m_topicDrivers);
      }
    }
    else {
      if (gDebugLevel & VSCP_DEBUG_CONFIG) {
        spdlog::debug("ReadConfig: Failed to read MQTT 'topic-drivers'. Defaults will be used.");
      }
    }

    // MQTT topic-discovery
    if (j["mqtt"].contains("topic-discovery")) {
      m_topicDiscovery = j["mqtt"]["topic-discovery"].get<std::string>();
      if (gDebugLevel & VSCP_DEBUG_CONFIG) {
        spdlog::debug("ReadConfig: MQTT 'topic-discovery' set to {}", m_topicDiscovery);
      }
    }
    else {
      if (gDebugLevel & VSCP_DEBUG_CONFIG) {
        spdlog::debug("ReadConfig: Failed to read MQTT 'topic-discovery'. Defaults will be used.");
      }
    }

    // Add base as prefix if defined
    if (m_topicDaemonBase.length()) {
      m_topicDrivers   = m_topicDaemonBase + m_topicDrivers;
      m_topicDiscovery = m_topicDaemonBase + m_topicDiscovery;
    }

  } //    end MQTT block

  // ------------------------------------------------------------------------------------------------------------------------
  //                                             Level I Drivers
  // ------------------------------------------------------------------------------------------------------------------------

  if (!(j.contains("drivers") && j["drivers"].is_object())) {
    if (gDebugLevel & VSCP_DEBUG_CONFIG) {
      spdlog::debug("ReadConfig: drivers object. No drivers will be used.");
    }
  }
  else {
    // Level I drivers
    if (!(j["drivers"].contains("level1") && j["drivers"]["level1"].is_object())) {

      json sub = j["drivers"]["level1"];
      for (json::iterator it = sub.begin(); it != sub.end(); ++it) {

        if ((*it).value("enable", false) && (*it)["name"].is_string() && (*it)["config"].is_string() &&
            (*it)["path"].is_string() && (*it)["flags"].is_number() && (*it)["guid"].is_string() &&
            (*it)["translation"].is_number()) {

          // Add the level I device
          if (!m_deviceList.addItem(this,
                                    (*it)["name"].get<std::string>(),
                                    (*it)["config"].get<std::string>(),
                                    (*it)["path"].get<std::string>(),
                                    (*it)["flags"].get<uint32_t>(),
                                    (*it)["guid"].get<std::string>(),
                                    VSCP_DRIVER_LEVEL1,
                                    (*it)["translation"].get<uint8_t>())) {
            spdlog::debug("Level I driver not added name={}. "
                          "Path does not exist. - [{}]",
                          (*it)["name"].get<std::string>(),
                          (*it)["path"].get<std::string>());
          }
          else {

            if (gDebugLevel & VSCP_DEBUG_DRIVERL1) {
              spdlog::debug("ReadConfig: Level I driver added. name = {} - [{}]",
                            (*it)["name"].get<std::string>(),
                            (*it)["path"].get<std::string>());
            }

            // ********************************************************************************
            //                               Level I driver MQTT
            // ********************************************************************************

            if (!((*it).contains("mqtt") && (*it)["mqtt"].is_object())) {
              if (gDebugLevel & VSCP_DEBUG_CONFIG) {
                spdlog::debug("ReadConfig: mqtt object. Defaults will be used.");
              }
            }
            else {

              CDeviceItem *pDriver = m_deviceList.getDeviceItemFromName((*it)["name"]);
              if (nullptr == pDriver) {
                spdlog::debug("ReadConfig: Driver MQTT info can not be added. Driver not found.");
              }
              else {

                pDriver->m_mqttConfig = (*it)["mqtt"];

                // Do MQTT configuration
                if (!pDriver->m_mqttClient.initFromJson(pDriver->m_mqttConfig.dump())) {
                  spdlog::error("ReadConfig: Level I Driver: Failed to initialize MQTT.");
                  return false;
                }
              }
            }
          }
        }
      }
    } // end - level I drivers

    // ------------------------------------------------------------------------------------------------------------------------
    //                                             Level II Drivers
    // ------------------------------------------------------------------------------------------------------------------------

    // Level II drivers
    if (!(j["drivers"].contains("level2") && j["drivers"]["level2"].is_object())) {
      json sub = j["drivers"]["level2"];
      for (json::iterator it = sub.begin(); it != sub.end(); ++it) {
        // std::cout << (*it)["name"] << '\n';
        if ((*it).value("enable", false) && (*it)["name"].is_string() && (*it)["path-config"].is_string() &&
            (*it)["path-driver"].is_string() && (*it)["guid"].is_string()) {
          // Add the level I device
          if (!m_deviceList.addItem(this,
                                    (*it)["name"].get<std::string>(),
                                    (*it)["path-config"].get<std::string>(),
                                    (*it)["path-driver"].get<std::string>(),
                                    0,
                                    (*it)["guid"].get<std::string>(),
                                    VSCP_DRIVER_LEVEL2)) {
            spdlog::debug("Level II driver not added name={}. "
                          "Path does not exist. - [{}]",
                          (*it)["name"].get<std::string>(),
                          (*it)["path-driver"].get<std::string>());
          }
          else {
            if (gDebugLevel & VSCP_DEBUG_DRIVERL1) {
              spdlog::debug("ReadConfig: Level II driver added. name = {}- [{}]",
                            (*it)["name"].get<std::string>(),
                            (*it)["path-driver"].get<std::string>());
            }

            // ********************************************************************************
            //                               Level II driver MQTT
            // ********************************************************************************

            if (!((*it).contains("mqtt") && (*it)["mqtt"].is_object())) {
              if (gDebugLevel & VSCP_DEBUG_CONFIG) {
                spdlog::debug("ReadConfig: mqtt object. Defaults will be used.");
              }
            }
            else {

              CDeviceItem *pDriver = m_deviceList.getDeviceItemFromName((*it)["name"]);
              if (nullptr == pDriver) {
                spdlog::debug("ReadConfig: Driver MQTT info can not be added. Driver not found.");
              }
              else {

                pDriver->m_mqttConfig = (*it)["mqtt"];

                // Do MQTT configuration
                if (!pDriver->m_mqttClient.initFromJson(pDriver->m_mqttConfig.dump())) {
                  spdlog::error("ReadConfig: Level II Driver: Failed to initialize MQTT.");
                  return false;
                }

              } // MQTT block driver II
            }
          }
        }
      }
    } // level II drivers

  } // drivers block

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// readConfiguration
//
// Read the configuration JSON file
//

bool
CControlObject::readConfiguration(const std::string &strcfgfile)
{
  spdlog::debug("Reading full JSON configuration from {}", strcfgfile);

  json j;

  try {
    std::ifstream in(strcfgfile, std::ifstream::in);
    in >> j;
  }
  catch (...) {
    spdlog::error("controlobject:  Failed to parse JSON configuration.");
    return false;
  }

  return readJSON(j);

} // JSON config
