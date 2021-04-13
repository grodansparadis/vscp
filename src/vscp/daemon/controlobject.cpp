// ControlObject.cpp: m_path_db_vscp_logimplementation of the CControlObject
// class.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2021 Ake Hedman, Grodans Paradis AB
// <info@grodansparadis.com>
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

#include <controlobject.h>

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
#include <syslog.h>
#include <unistd.h>
#else

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
#include <vscp_debug.h>
#include <vscpd_caps.h>
#include <vscphelper.h>

#include <iostream>
#include <fstream>
#include <algorithm>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <string>

#include <json.hpp>         // Needs C++11  -std=c++11
#include <mustache.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>

// https://github.com/nlohmann/json
using json = nlohmann::json;

using namespace kainjow::mustache;


#define UNUSED(x) (void)(x)
void
foo(const int i)
{
    UNUSED(i);
}

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#define XML_BUFF_SIZE 0xffff

// Prototypes

//////////////////////////////////////////////////////////////////////
//                         Callbacks
//////////////////////////////////////////////////////////////////////


static void mqtt_log_callback(struct mosquitto *mosq, void *pData, int level, const char *logmsg)
{
    // Check pointers
    if (NULL == mosq) return;
    if (NULL == pData) return;

    CControlObject *pObj = reinterpret_cast<CControlObject *>(pData);

    if (pObj->m_debugFlags & VSCP_DEBUG_MQTT_LOG) {
        char buf[80];
        time_t tm;
        time(&tm);
        vscp_getTimeString(buf, sizeof(buf), &tm);
        printf("SRV LOG: (%s) %s\n", buf, logmsg);
#ifdef WIN32
#else        
        syslog(LOG_DEBUG, "vscpd:  MQTT log : %s\n", logmsg);
#endif        
    }
}


static void mqtt_on_connect(struct mosquitto *mosq, void *pData, int rv)
{
    // Check pointers
    if (NULL == mosq) return;
    if (NULL == pData) return;

    CControlObject *pObj = reinterpret_cast<CControlObject *>(pData);

    if (pObj->m_debugFlags & VSCP_DEBUG_MQTT_CONNECT) {
        printf("SRV CONNECT:\n");
#ifdef WIN32
#else        
        syslog(LOG_DEBUG, "vscpd:  MQTT connect");
#endif        
    }
}


static void mqtt_on_disconnect(struct mosquitto *mosq, void *pData, int rv)
{
    // Check pointers
    if (NULL == mosq) return;
    if (NULL == pData) return;

    CControlObject *pObj = reinterpret_cast<CControlObject *>(pData);

    if (pObj->m_debugFlags & VSCP_DEBUG_MQTT_CONNECT) {
        printf("SRV DISCONNECT:\n");
#ifdef WIN32
#else        
        syslog(LOG_DEBUG, "vscpd:  MQTT disconnect");
#endif        
    }
}


static void mqtt_on_message(struct mosquitto *mosq, void *pData, const struct mosquitto_message *pMsg)
{
    // Check pointers
    if (NULL == mosq) return;
    if (NULL == pData) return;
    if (NULL == pMsg) return;

    CControlObject *pObj = reinterpret_cast<CControlObject *>(pData);
    std::string payload((const char *)pMsg->payload, pMsg->payloadlen);

    if (pObj->m_debugFlags & VSCP_DEBUG_MQTT_MSG) {
        printf("SRV: Message: [topic = %s]: Payload: %s\n", pMsg->topic, payload.c_str());
#ifdef WIN32
#else        
        syslog(LOG_DEBUG, "vscpd:  MQTT message [%s]", payload.c_str());
#endif        
    }
}


static void mqtt_on_publish(struct mosquitto *mosq, void *pData, int rv)
{
    // Check pointers
    if (NULL == mosq) return;
    if (NULL == pData) return;

    CControlObject *pObj = reinterpret_cast<CControlObject *>(pData);

    if (pObj->m_debugFlags & VSCP_DEBUG_MQTT_PUBLISH) {
        printf("SRV PUBLISH:\n");
#ifdef WIN32
#else        
        syslog(LOG_DEBUG, "vscpd:  MQTT disconnect");
#endif        
    }
}




//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CControlObject::CControlObject()
{
    m_bQuit = false;
    m_debugFlags = 0;

    // Open syslog
#ifdef WIN32
#else    
    openlog("vscpd", LOG_CONS, LOG_DAEMON);
#endif    

    if (m_debugFlags & VSCP_DEBUG_EXTRA) {
#ifdef WIN32
#else        
        syslog(LOG_DEBUG, "vscpd:  Starting the vscpd daemon");
#endif        
    }

    if (0 != pthread_mutex_init(&m_mutex_DeviceList, NULL)) {
#ifdef WIN32
#else        
        syslog(LOG_ERR, "vscpd:  Unable to init m_mutex_DeviceList");
#endif        
        return;
    }

    m_strServerName = "VSCP Daemon";
    m_rootFolder = "/var/lib/vscp/vscpd/";
    m_vscptoken = "Carpe diem quam minimum credula postero";
    m_pathClassTypeDefinitionDb = "/var/lib/vscp/vscpd/vscp_events.sqlite3";

    // Logging defaults
    m_path_to_log_file = "/var/log/vscp/vscp.log";
    m_max_log_size = 5242880;
    m_max_log_files = 7;

    // Nill the GUID
    m_guid.clear();

    // Initialize the CRC
    crcInit();

    // Init daemon MQTT
    m_mqtt_strHost = "localhost";
    m_mqtt_port = 1883;
    m_mqtt_strClientId = "";
    m_mqtt_strUserName = "vscp";
    m_mqtt_strPassword = "secret";
    m_mqtt_qos = 0;
    m_mqtt_bRetain = false;
    m_mqtt_keepalive = 60;
    m_mqtt_bCleanSession = false;
    m_mqtt_reconnect_delay = 2;
    m_mqtt_reconnect_delay_max = 30;
    m_mqtt_reconnect_exponential_backoff = false;
    m_mqtt_bTLS = false;
    m_mqtt_cafile = "";
    m_mqtt_capath = "";
    m_mqtt_certfile = "";
    m_mqtt_keyfile = "";
    m_mqtt_pwKeyfile = "";
    m_mqtt_format = jsonfmt;
    m_topicInterfaces = "vscp/{{guid}}/interfaces";

    // Initialize MQTT
    if ( MOSQ_ERR_SUCCESS != mosquitto_lib_init() ) {
#ifdef WIN32
#else        
        syslog(LOG_ERR, "vscpd:  Unable to initialize mosquitto library.");
#endif        
        return;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CControlObject::~CControlObject()
{
    if (m_debugFlags & VSCP_DEBUG_EXTRA) {
#ifdef WIN32
#else        
        syslog(LOG_DEBUG, "vscpd:  Cleaning up");
#endif        
    }

    if (0 != pthread_mutex_destroy(&m_mutex_DeviceList)) {
#ifdef WIN32
#else        
        syslog(LOG_ERR, "vscpd:  Unable to destroy m_mutex_DeviceList");
#endif        
        return;
    }

    if (m_debugFlags & VSCP_DEBUG_EXTRA) {
#ifdef WIN32
#else        
        syslog(LOG_DEBUG, "vscpd:  Terminating the vscpd daemon");
#endif        
    }

    // Clean up SQLite lib allocations
    sqlite3_shutdown();

    // Close syslog
#ifdef WIN32
#else    
    closelog();
#endif    
}

/////////////////////////////////////////////////////////////////////////////
// init
//

bool
CControlObject::init(std::string& strcfgfile, std::string& rootFolder)
{
    std::string str;

    // Save root folder for later use.
    m_rootFolder = rootFolder;

    // Root folder must exist
    if (!vscp_fileExists(m_rootFolder.c_str())) {
#ifdef WIN32
#else        
        syslog(LOG_ERR,
               "vscpd:  The specified rootfolder does not exist (%s).",
               (const char*)m_rootFolder.c_str());
#endif               
        return false;
    }

    // Change locale to get the correct decimal point "."
    setlocale(LC_NUMERIC, "C");

    // A configuration file must be available
    if (!vscp_fileExists(strcfgfile.c_str())) {
        perror("No configuration file. Can't initialize!.");
#ifdef WIN32
#else        
        syslog(LOG_ERR,
               "vscpd:  No configuration file. Can't initialize!. Path=%s",
               strcfgfile.c_str());
#endif               
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////
    //                           Read configuration
    ////////////////////////////////////////////////////////////////////////////

    // Read XML configuration
    if (m_debugFlags & VSCP_DEBUG_EXTRA) {
#ifdef WIN32
#else        
        syslog(LOG_DEBUG, "Reading configuration file");
#endif        
    }

    // Read configuration
    if (!readConfiguration(strcfgfile)) {
        fprintf(stderr,"vscpd:  Unable to open/parse/read configuration file. Can't initialize! See syslog - ");
#ifdef WIN32
#else        
        syslog(LOG_ERR,
                "vscpd:  Unable to open/parse/read configuration file. Can't initialize! "
                "Path =%s",                
                strcfgfile.c_str());
#endif                
        return FALSE;
    }


#ifndef WIN32
    if (m_runAsUser.length()) {
        int rv;
        struct passwd pw;
        struct passwd *result;
        char pwbuf[16384];

        rv = getpwnam_r(m_runAsUser.c_str(), &pw, pwbuf, sizeof(pwbuf), &result);
        if (NULL == result) {
            if (0 == rv) {
                syslog(LOG_ERR, "vscpd:  Unknown user.");
            } else {
                errno = rv;
                syslog(LOG_ERR, "Unable to become requested user [%s].", m_runAsUser.c_str());
                perror("Failed to run as user.");
            }
            exit(EXIT_FAILURE);
        }

        if (setgid(pw.pw_gid) != 0) {
            syslog(LOG_ERR, "vscpd:  setgid() failed. [%s]", strerror(errno));
        }

        if (setuid(pw.pw_uid) != 0) {
            syslog(LOG_ERR, "vscpd:  setuid() failed. [%s]", strerror(errno));
        }
    }

#endif

    if (m_debugFlags & VSCP_DEBUG_EXTRA) {
#ifdef WIN32
#else        
        syslog(LOG_DEBUG, "vscpd:  Using configuration file: %s", strcfgfile.c_str());
#endif        
    }

    // Get GUID
    if (m_guid.isNULL()) {
        if (!getMacAddress(m_guid)) {
            // We failed to create GUID from MAC address use
            // 'localhost' IP instead as the base.
            getIPAddress(m_guid);
        }
    }

    str = "VSCP Server started - ";
    str += "Version: ";
    str += VSCPD_DISPLAY_VERSION;
    str += " - ";
    str += VSCPD_COPYRIGHT;
#ifdef WIN32
#else    
    syslog(LOG_INFO, "%s", str.c_str());
#endif    

    // Load class/type definitions from database if they should be loaded

    // Initialize the SQLite library
    if ( SQLITE_OK != sqlite3_initialize() ) {
#ifdef WIN32
#else        
        syslog(LOG_ERR, "vscpd:  Unable to initialize SQLite library.");
#endif        
        return false;
    }

    // Read in class/type info if available
    if (m_pathClassTypeDefinitionDb.length()) {

        sqlite3 *db_vscp_classtype;
        if (SQLITE_OK == sqlite3_open(m_pathClassTypeDefinitionDb.c_str(), &db_vscp_classtype)) {

            // * * *   C L A S S E S   * * *

            sqlite3_stmt *ppStmt;
            if (SQLITE_OK != sqlite3_prepare(db_vscp_classtype,
                                    "SELECT class,name,token from vscp_class",
                                    -1,
                                    &ppStmt,
                                    NULL) )  {
#ifdef WIN32
#else                                        
                syslog(LOG_ERR, "vscpd:  Failed to prepare class fetch from class & type database.");
#endif                
            }

            while (SQLITE_ROW == sqlite3_step(ppStmt)) {
                uint16_t vscp_class = (uint16_t)sqlite3_column_int(ppStmt, 0);
                std::string name = (const char *)sqlite3_column_text(ppStmt, 1);
                std::string token = (const char *)sqlite3_column_text(ppStmt, 2);
                m_map_class_id2Token[vscp_class] = token;
                if (m_debugFlags & VSCP_DEBUG_EVENT_DATABASE) {
                    syslog(LOG_DEBUG, "vscpd:  Class = %s - ", m_map_class_id2Token[vscp_class].c_str() );
                }
                m_map_class_token2Id[token] = vscp_class;
                if (m_debugFlags & VSCP_DEBUG_EVENT_DATABASE) {
#ifdef WIN32
#else                    
                    syslog(LOG_DEBUG, "vscpd:  Id = %d\n", m_map_class_token2Id[token]);
#endif                    
                }
            }
            sqlite3_finalize(ppStmt);

            // * * *   T Y P E S   * * *

            if (SQLITE_OK != sqlite3_prepare(db_vscp_classtype,
                                    "SELECT type,link_to_class,token from vscp_type",
                                    -1,
                                    &ppStmt,
                                    NULL) )  {
                syslog(LOG_ERR, "vscpd:  Failed to prepare type fetch from class & type database.");
            }


            while (SQLITE_ROW == sqlite3_step(ppStmt)) {
                uint16_t vscp_type = (uint16_t)sqlite3_column_int(ppStmt, 0);
                uint16_t link_to_class = (uint16_t)sqlite3_column_int(ppStmt, 1);
                std::string token = (const char *)sqlite3_column_text(ppStmt, 2);
                m_map_type_id2Token[(link_to_class << 16) + vscp_type] = token;
                if (m_debugFlags & VSCP_DEBUG_EVENT_DATABASE) {
                    syslog(LOG_DEBUG, "vscpd:  Token = %s ", m_map_type_id2Token[(link_to_class << 16) + vscp_type].c_str());
                }
                m_map_type_token2Id[token] = (link_to_class << 16) + vscp_type;
                if (m_debugFlags & VSCP_DEBUG_EVENT_DATABASE) {
                    syslog(LOG_DEBUG, "vscpd:  Id = %d\n", m_map_type_token2Id[token]);
                }
            }
            sqlite3_finalize(ppStmt);

            sqlite3_close(db_vscp_classtype);
        } else {
            syslog(LOG_ERR,
                    "vscpd:  Failed to open VSCP class & type definition database %s. [%s]",
                    m_pathClassTypeDefinitionDb.c_str(),
                    sqlite3_errmsg(db_vscp_classtype));
        }

    }

    // Open the discovery database (create if not available)
    if (m_pathMainDb.length()) {

        // If file does not exist. Create it and tables and indexes
        if (vscp_fileExists(m_pathMainDb)) {

            if (SQLITE_OK != sqlite3_open(m_pathMainDb.c_str(),
                                            &m_db_vscp_daemon)) {

                // Failed to open/create the database file
                syslog(LOG_ERR, "vscpd:  VSCP Daemon database could not be opened/created. - Path=%s error=%s",
                                        m_pathMainDb.c_str(),
                                        sqlite3_errmsg(m_db_vscp_daemon));
                m_db_vscp_daemon = NULL;
            } else {

                // Read in discovered nodes to in memory map

                sqlite3_stmt *ppStmt;
                if (SQLITE_OK != sqlite3_prepare(m_db_vscp_daemon,
                                                    "SELECT guid,name from discovery",
                                                    -1,
                                                    &ppStmt,
                                                    NULL) )  {
                    syslog(LOG_ERR, "vscpd:  Failed to prepare discovery node fetch.");
                }

                while (SQLITE_ROW == sqlite3_step(ppStmt)) {
                    std::string guid = (const char *)sqlite3_column_text(ppStmt, 0);
                    std::string name = (const char *)sqlite3_column_text(ppStmt, 1);
                    m_map_discoveryGuidToName[guid] = name;
                    if (m_debugFlags & VSCP_DEBUG_MAIN_DATABASE) {
                        syslog(LOG_DEBUG, "vscpd:  guid = %s - name = %s ", guid.c_str(), name.c_str() );
                    }
                }

                sqlite3_finalize(ppStmt);

            }

        } else {

            if (SQLITE_OK != sqlite3_open(m_pathMainDb.c_str(),
                                            &m_db_vscp_daemon)) {

                // Failed to open/create the database file
                syslog(LOG_ERR, "vscpd:  VSCP Daemon database could not be opened/created. - Path=%s error=%s",
                                        m_pathMainDb.c_str(),
                                        sqlite3_errmsg(m_db_vscp_daemon));
            }

            // We will try to create it
            syslog(LOG_INFO, "vscpd:  Will try to create VSCP Daemon database here %s.", m_pathMainDb.c_str());

            // Create settings db
            char *pErrMsg = 0;
            const char *psql;

            psql = "CREATE TABLE \"discovery\" (                    \
                \"id\"	INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE,   \
                \"date\"	TEXT,                                   \
                \"guid\"	TEXT UNIQUE,                            \
                \"name\"	TEXT UNIQUE,                            \
                \"description\"	TEXT                                \
                );";

            if (SQLITE_OK  !=  sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg)) {
                syslog(LOG_ERR,
                        "vscpd:  Creation of the VSCP database failed with message %s",
                        pErrMsg);
                return false;
            }

            psql = "CREATE INDEX \"idxguid\" ON \"discovery\" (\"guid\")";

            if (SQLITE_OK  !=  sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg)) {
                syslog(LOG_ERR,
                        "vscpd:  Creation of the VSCP database index idxguid failed with message %s",
                        pErrMsg);
                return false;
            }

            psql = "CREATE INDEX \"idxname\" ON \"discovery\" (\"name\")";

            if (SQLITE_OK  !=  sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg)) {
                syslog(LOG_ERR,
                        "vscpd:  Creation of the VSCP database index idxname failed with message %s",
                        pErrMsg);
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

            if (SQLITE_OK  !=  sqlite3_exec(m_db_vscp_daemon, sql.c_str(), NULL, NULL, &pErrMsg)) {
                syslog(LOG_ERR,
                        "vscpd:  Creation of the VSCP database index idxname failed with message %s",
                        pErrMsg);
                return false;
            }

            // Add local host to in memory map
            m_map_discoveryGuidToName[m_guid.getAsString()] = "local-vscp-daemon";

        }

    }

    // Setup MQTT for server

    if (m_mqtt_strClientId.length()) {
        m_mosq = mosquitto_new(m_mqtt_strClientId.c_str(), m_mqtt_bCleanSession, this);
    } else {
        m_mqtt_bCleanSession = true;    // Must be true without id
        m_mosq = mosquitto_new(NULL, m_mqtt_bCleanSession, this);
    }

    if (NULL == m_mosq) {
        if (ENOMEM == errno) {
            syslog(LOG_ERR, "vscpd:  Failed to create new mosquitto session (out of memory).");
        } else if (EINVAL == errno) {
            syslog(LOG_ERR, "vscpd:  Failed to create new mosquitto session (invalid parameters).");
        }
        return false;
    }

    mosquitto_log_callback_set(m_mosq, mqtt_log_callback);
    mosquitto_connect_callback_set(m_mosq, mqtt_on_connect);
    mosquitto_disconnect_callback_set(m_mosq, mqtt_on_disconnect);
    mosquitto_message_callback_set(m_mosq, mqtt_on_message);
    mosquitto_publish_callback_set(m_mosq, mqtt_on_publish);

    if (MOSQ_ERR_SUCCESS != mosquitto_reconnect_delay_set(m_mosq,
                                                            m_mqtt_reconnect_delay,
                                                            m_mqtt_reconnect_delay_max,
                                                            m_mqtt_reconnect_exponential_backoff)) {
        syslog(LOG_ERR, "vscpd:  Failed to set reconnect settings.");
    }

    // Set username/password if defined
    if (m_mqtt_strUserName.length()) {
        int rv;
        if ( MOSQ_ERR_SUCCESS != (rv = mosquitto_username_pw_set(m_mosq,
                                                                    m_mqtt_strUserName.c_str(),
                                                                    m_mqtt_strPassword.c_str()) ) ) {
            if (MOSQ_ERR_INVAL == rv) {
                syslog(LOG_ERR, "vscpd:  Failed to set mosquitto username/password (invalid parameter(s)).");
            } else if (MOSQ_ERR_NOMEM == rv) {
                syslog(LOG_ERR, "vscpd:  Failed to set mosquitto username/password (out of memory).");
            }
        }
    }

    int rv = mosquitto_connect(m_mosq,
                                m_mqtt_strHost.c_str(),
                                m_mqtt_port,
                                m_mqtt_keepalive);

    if (MOSQ_ERR_SUCCESS != rv) {

        if (MOSQ_ERR_INVAL == rv) {
            syslog(LOG_ERR, "vscpd:  Failed to connect to mosquitto server (invalid parameter(s)).");
        } else if (MOSQ_ERR_ERRNO == rv) {
            syslog(LOG_ERR, "vscpd:  Failed to connect to mosquitto server. System returned error (errno = %d).", errno);
        }

        return false;
    }

    // Start the worker loop
    rv = mosquitto_loop_start(m_mosq);
    if (MOSQ_ERR_SUCCESS != rv) {
        mosquitto_disconnect(m_mosq);
        return VSCP_ERROR_ERROR;
    }

    for (std::list<std::string>::const_iterator
            it = m_mqtt_subscriptions.begin();
            it != m_mqtt_subscriptions.end();
            ++it) {

        std::string topic = *it;

        // Fix subscribe/publish topics
        mustache subtemplate{topic};
        data data;
        data.set("guid", m_guid.getAsString());
        std::string subscribe_topic = subtemplate.render(data);

        // Subscribe to specified topic
        rv = mosquitto_subscribe(m_mosq,
                                    /*m_mqtt_id*/NULL,
                                    subscribe_topic.c_str(),
                                    m_mqtt_qos);

        switch (rv) {
            case MOSQ_ERR_INVAL:
                syslog(LOG_ERR, "vscpd:  Failed to subscribed to specified topic [%s] - input parameters were invalid.", subscribe_topic.c_str());
            case MOSQ_ERR_NOMEM:
                syslog(LOG_ERR, "vscpd:  Failed to subscribed to specified topic [%s] - out of memory condition occurred.", subscribe_topic.c_str());
            case MOSQ_ERR_NO_CONN:
                syslog(LOG_ERR, "vscpd:  Failed to subscribed to specified topic [%s] - client isn’t connected to a broker.", subscribe_topic.c_str());
            case MOSQ_ERR_MALFORMED_UTF8:
                syslog(LOG_ERR, "vscpd:  Failed to subscribed to specified topic [%s] - topic is not valid UTF-8.", subscribe_topic.c_str());
#if defined(MOSQ_ERR_OVERSIZE_PACKET)
            case MOSQ_ERR_OVERSIZE_PACKET:
                syslog(LOG_ERR, "vscpd:  Failed to subscribed to specified topic [%s] - resulting packet would be larger than supported by the broker.", subscribe_topic.c_str());
#endif
        }
    }


    // Publish interfaces (retained)
    // default topic "vscp/{{guid}}/interfaces"
    // Fix publish topics
    mustache subtemplate{m_topicInterfaces};
    data data;
    data.set("guid", m_guid.getAsString());
    std::string strTopic = subtemplate.render(data);
    std::string strPayload = m_deviceList.getAllAsJSON();

    rv = mosquitto_publish(m_mosq,
                            NULL,
                            strTopic.c_str(),
                            strPayload.length(),
                            strPayload.c_str(),
                            m_mqtt_qos,
                            true);


    // Load drivers
    try {
        startDeviceWorkerThreads();
    }
    catch (...) {
        syslog(LOG_ERR, "vscpd:  Exception when loading drivers");
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// cleanup

bool
CControlObject::cleanup(void)
{
    if (m_debugFlags & VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG,
               "vscpd:  cleanup - Giving worker threads time to stop "
               "operations...");
    }

    if (m_debugFlags & VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG,
               "vscpd:   cleanup - Stopping device worker thread...");
    }

    try {
        stopDeviceWorkerThreads();
    }
    catch (...) {
        syslog(LOG_ERR,
               "ControlObject: Exception occurred when stoping device worker threads");
    }

    if (m_debugFlags & VSCP_DEBUG_EXTRA) {
        syslog(
          LOG_DEBUG,
          "ControlObject: cleanup - Stopping VSCP Server worker thread...");
    }

    // Disconnect from MQTT broker}
    int rv = mosquitto_disconnect(m_mosq);
    if (MOSQ_ERR_SUCCESS != rv) {
        if (MOSQ_ERR_INVAL == rv) {
            syslog(LOG_ERR, "ControlObject: mosquitto_disconnect: input parameters were invalid.");
        } else if (MOSQ_ERR_NO_CONN == rv) {
            syslog(LOG_ERR, "ControlObject: mosquitto_disconnect: client isn’t connected to a broker");
        }
    }

    // stop the worker loop
    rv = mosquitto_loop_stop(m_mosq, false);
    if (MOSQ_ERR_SUCCESS != rv) {
        if (MOSQ_ERR_INVAL == rv) {
            syslog(LOG_ERR, "ControlObject: mosquitto_loop_stop: input parameters were invalid.");
        } else if (MOSQ_ERR_NOT_SUPPORTED == rv) {
            syslog(LOG_ERR, "ControlObject: mosquitto_loop_stop: thread support is not available..");
        }
    }

    // Clean up
    mosquitto_destroy(m_mosq);

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// run - Program main loop
//
// Most work is done in the threads at the moment
//

bool
CControlObject::run(void)
{
    std::deque<CClientItem*>::iterator nodeClient;

#ifdef WITH_SYSTEMD
    sd_notify(0, "READY=1");
#endif

    //-------------------------------------------------------------------------
    //                            MAIN - LOOP
    //-------------------------------------------------------------------------

    struct timespec now, old_now;
    clock_gettime(CLOCK_REALTIME, &old_now);
    old_now.tv_sec -= 60;  // Do first send right away

    while (true) {

        clock_gettime(CLOCK_REALTIME, &now);

        // We send heartbeat every minute
        if ((now.tv_sec-old_now.tv_sec) > 60) {

            // Save time
            clock_gettime(CLOCK_REALTIME, &old_now);

            if (!periodicEvents()) {
                 syslog(LOG_ERR, "Failed to send automation events!");
            }
        }

        usleep(5000);

    }  // while


    // Clean up is called in main file

    if (m_debugFlags & VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG, "Mainloop ending");
    }

    return true;
}


/////////////////////////////////////////////////////////////////////////////
// sendEvent
//

bool
CControlObject::sendEvent(vscpEventEx *pex)
{
    int rv = 0;
    uint8_t *pbuf = NULL;   // Used for binary payload
    std::string strPayload;
    std::string strTopic;

    // Check pointer
    if (NULL == pex) {
        syslog(LOG_ERR, "ControlObject: sendEvent: Event is NULL pointer");
        return false;
    }

    if ( m_mqtt_format == jsonfmt ) {
        if ( !vscp_convertEventExToJSON(strPayload, pex) ) {
            syslog(LOG_ERR, "ControlObject: sendEvent: Failed to convert event to JSON");
            return false;
        }
    } else if ( m_mqtt_format == xmlfmt ) {
        if ( !vscp_convertEventExToXML(strPayload, pex) ) {
            syslog(LOG_ERR, "ControlObject: sendEvent: Failed to convert event to XML");
            return false;
        }
    } else if ( m_mqtt_format == strfmt ) {
        if ( !vscp_convertEventExToString(strPayload, pex) ) {
            syslog(LOG_ERR, "ControlObject: sendEvent: Failed to convert event to STRING");
            return false;
        }
    } else if ( m_mqtt_format == binfmt ) {
        pbuf = new uint8_t[VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 + pex->sizeData];
        if (NULL == pbuf) {
            return false;
        }

        if (!vscp_writeEventExToFrame(pbuf,
                                        VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 + pex->sizeData,
                                        VSCP_MULTICAST_TYPE_EVENT,
                                        pex)) {
            syslog(LOG_ERR, "ControlObject: sendEvent: Failed to convert event to BINARY");
            return false;
        }
    } else {
        return VSCP_ERROR_NOT_SUPPORTED;
    }

    for (std::list<std::string>::const_iterator
            it = m_mqtt_publish.begin();
            it != m_mqtt_publish.end();
            ++it) {

        std::string topic_template = *it;

        // Fix publish topics
        mustache subtemplate{topic_template};
        data data;
        cguid evguid(pex->GUID);    // Event GUID
        data.set("guid", evguid.getAsString());
        for (int i=0; i < 15; i++) {
            data.set(vscp_str_format("guid[%d]", i), vscp_str_format("%d", evguid.getAt(i)));
        }
        data.set("guid.msb", vscp_str_format("%d", evguid.getAt(0)));
        data.set("guid.lsb", vscp_str_format("%d", evguid.getMSB()));
        data.set("ifguid", m_guid.getAsString());

        for (int i=0; i < pex->sizeData; i++) {
            data.set(vscp_str_format("data[%d]", i), vscp_str_format("%d", pex->data[i]));
        }

        for (int i=0; i < 15; i++) {
            data.set(vscp_str_format("ifguid[%d]", i), vscp_str_format("%d", m_guid.getAt(i)));
        }
        data.set("nickname", vscp_str_format("%d", evguid.getNicknameID()));
        data.set("class", vscp_str_format("%d", pex->vscp_class));
        data.set("type", vscp_str_format("%d", pex->vscp_type));

        data.set("class-token", getTokenFromClassId(pex->vscp_class));
        data.set("type-token", getTokenFromTypeId(pex->vscp_class, pex->vscp_type));

        data.set("head", vscp_str_format("%d", pex->head));
        data.set("obid", vscp_str_format("%ul", pex->obid));
        data.set("timestamp", vscp_str_format("%ul", pex->timestamp));

        std::string dt;
        vscp_getDateStringFromEventEx(dt, pex);
        data.set("datetime", dt);
        data.set("year", vscp_str_format("%d", pex->year));
        data.set("month", vscp_str_format("%d", pex->month));
        data.set("day", vscp_str_format("%d", pex->day));
        data.set("hour", vscp_str_format("%d", pex->hour));
        data.set("minute", vscp_str_format("%d", pex->minute));
        data.set("second", vscp_str_format("%d", pex->second));

        data.set("clientid", m_mqtt_strClientId);
        data.set("user", m_mqtt_strUserName);
        data.set("host", m_mqtt_strHost);

        strTopic = subtemplate.render(data);

        if (m_mqtt_format != binfmt) {
            rv = mosquitto_publish(m_mosq,
                                    NULL,
                                    strTopic.c_str(),
                                    strPayload.length(),
                                    strPayload.c_str(),
                                    m_mqtt_qos,
                                    FALSE);
        } else {
            rv = mosquitto_publish(m_mosq,
                                    NULL,
                                    strTopic.c_str(),
                                    VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 + pex->sizeData,
                                    pbuf,
                                    m_mqtt_qos,
                                    FALSE);
        }

        // Translate mosquitto error code to VSCP error code
        switch (rv) {
            case MOSQ_ERR_INVAL:
                syslog(LOG_ERR, "ControlObject: sendEvent: Error Parameter");
                break;
            case MOSQ_ERR_NOMEM:
                syslog(LOG_ERR, "ControlObject: sendEvent: Error Memory");
                break;
            case MOSQ_ERR_NO_CONN:
                syslog(LOG_ERR, "ControlObject: sendEvent: Error Connection");
                break;
            case MOSQ_ERR_PROTOCOL:
                syslog(LOG_ERR, "ControlObject: sendEvent: Error protocol");
                break;
            case MOSQ_ERR_PAYLOAD_SIZE:
                syslog(LOG_ERR, "ControlObject: sendEvent: Error payload size");
                break;
            case MOSQ_ERR_MALFORMED_UTF8:
                syslog(LOG_ERR, "ControlObject: sendEvent: Error malformed utf8");
                break;
#if defined(MOSQ_ERR_QOS_NOT_SUPPORTED)
            case MOSQ_ERR_QOS_NOT_SUPPORTED:
                syslog(LOG_ERR, "ControlObject: sendEvent: Error QOS not supported");
                break;
#endif 
#if defined(MOSQ_ERR_OVERSIZE_PACKET)
            case MOSQ_ERR_OVERSIZE_PACKET:
                syslog(LOG_ERR, "ControlObject: sendEvent: Error Oversized package");
                break;
#endif
        }

        // End if error
        if (MOSQ_ERR_SUCCESS != rv) break;

    }  // for

    if (m_mqtt_format == binfmt) {
        // Clean up allocated buffer
        delete [] pbuf;
        pbuf = NULL;
    }

    return (0 == rv);
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
    ex.obid      = 0;  // IMPORTANT Must be set by caller before event is sent
    ex.head      = 0;
    ex.timestamp = vscp_makeTimeStamp();
    vscp_setEventExToNow(&ex);  // Set time to current time
    ex.vscp_class = VSCP_CLASS1_INFORMATION;
    ex.vscp_type  = VSCP_TYPE_INFORMATION_NODE_HEARTBEAT;
    ex.sizeData   = 3;
    m_guid.writeGUID(ex.GUID);

    // GUID
    memcpy(ex.data + VSCP_CAPABILITY_OFFSET_GUID, m_guid.getGUID(), 16);

    ex.data[0] = 0;     // index
    ex.data[1] = 0;     // zone
    ex.data[2] = 0;     // subzone

    if (!sendEvent(&ex)) {
        syslog(LOG_ERR, "Failed to send Class1 heartbeat");
    }

    // Send VSCP_CLASS2_INFORMATION,
    // Type=2/VSCP2_TYPE_INFORMATION_HEART_BEAT
    ex.obid      = 0;   // IMPORTANT Must be set by caller before event is sent
    ex.head      = 0;
    ex.timestamp = vscp_makeTimeStamp();
    vscp_setEventExToNow(&ex);  // Set time to current time
    ex.vscp_class = VSCP_CLASS2_INFORMATION;
    ex.vscp_type  = VSCP2_TYPE_INFORMATION_HEART_BEAT;
    ex.sizeData   = 64;
    m_guid.writeGUID(ex.GUID);

    // GUID
    memcpy(ex.data + VSCP_CAPABILITY_OFFSET_GUID, m_guid.getGUID(), 16);

    memset(ex.data, 0, sizeof(ex.data));
    memcpy(ex.data,
           m_strServerName.c_str(),
           std::min((int)strlen(m_strServerName.c_str()), 64));

    if (!sendEvent(&ex)) {
         syslog(LOG_ERR, "Failed to send Class2 heartbeat");
    }

    // Send VSCP_CLASS1_PROTOCOL,
    // Type=1/VSCP_TYPE_PROTOCOL_SEGCTRL_HEARTBEAT
    ex.obid      = 0;  // IMPORTANT Must be set by caller before event is sent
    ex.head      = 0;
    ex.timestamp = vscp_makeTimeStamp();
    vscp_setEventExToNow(&ex);  // Set time to current time
    ex.vscp_class = VSCP_CLASS1_PROTOCOL;
    ex.vscp_type  = VSCP_TYPE_PROTOCOL_SEGCTRL_HEARTBEAT;
    ex.sizeData   = 5;
    m_guid.writeGUID(ex.GUID);

    // GUID
    memcpy(ex.data + VSCP_CAPABILITY_OFFSET_GUID, m_guid.getGUID(), 16);

    time_t tnow;
    time(&tnow);
    uint32_t time32 = (uint32_t)tnow;

    ex.data[0] = 0;     // 8 - bit crc for VSCP daemon GUID
    ex.data[1] = (uint8_t)((time32 >> 24) & 0xff);  // Time since epoch MSB
    ex.data[2] = (uint8_t)((time32 >> 16) & 0xff);
    ex.data[3] = (uint8_t)((time32 >> 8) & 0xff);
    ex.data[4] = (uint8_t)((time32)&0xff);  // Time since epoch LSB

    if (!sendEvent(&ex)) {
         syslog(LOG_ERR, "Failed to send segment controller heartbeat");
    }

    // Send VSCP_CLASS2_PROTOCOL,
    // Type=20/VSCP2_TYPE_PROTOCOL_HIGH_END_SERVER_CAPS
    ex.obid      = 0;   // IMPORTANT Must be set by caller before event is sent
    ex.head      = 0;
    ex.timestamp = vscp_makeTimeStamp();
    vscp_setEventExToNow(&ex);  // Set time to current time
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
            (const char*)m_strServerName.c_str(),
            std::min((int)strlen((const char*)m_strServerName.c_str()), 64));

    // Capabilities array
    getVscpCapabilities(ex.data);

    // non-standard ports
    // TODO(AKHE)

    ex.sizeData = 104;
    m_guid.writeGUID(ex.GUID);

    if (!sendEvent(&ex)) {
         syslog(LOG_ERR, "Failed to send high end server capabilities.");
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
// discovery
//

void CControlObject::discovery(vscpEvent *pev)
{
    // Check pointer
    if (NULL == pev) {
        syslog(LOG_ERR, "ControlObject: node discover: Event is NULL pointer.");
        return;
    }

    // If not used - skip out
    if (NULL == m_db_vscp_daemon) {
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

        if (SQLITE_OK  !=  sqlite3_exec(m_db_vscp_daemon, sql.c_str(), NULL, NULL, &pErrMsg)) {
            syslog(LOG_ERR,
                    "Creation of the VSCP database index idxname failed with message %s",
                    pErrMsg);
            return;
        }

        // Add local host to in memory map
        m_map_discoveryGuidToName[guid.getAsString()] = "Discovered";
    }
}

////////////////////////////////////////////////////////////////////////////////
// getVscpCapabilities
//

bool CControlObject::getVscpCapabilities(uint8_t *pCaps)
{
    // Check pointer
    if (NULL == pCaps) {
        syslog(LOG_ERR, "ControObject: getVscpCapabilities: NULL pointer.");
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
    CDeviceItem* pDeviceItem;
    if (m_debugFlags & VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG, "vscpd: [Driver] - Starting drivers...");
    }

    std::deque<CDeviceItem*>::iterator it;
    for (it = m_deviceList.m_devItemList.begin();
         it != m_deviceList.m_devItemList.end();
         ++it) {

        pDeviceItem = *it;
        if (NULL != pDeviceItem) {

            if (m_debugFlags & VSCP_DEBUG_EXTRA) {
                syslog(LOG_DEBUG,
                       "Controlobject: [Driver] - Preparing: %s ",
                       pDeviceItem->m_strName.c_str());
            }

            // Just start if enabled
            if (!pDeviceItem->m_bEnable)
                continue;

            if (m_debugFlags & VSCP_DEBUG_EXTRA) {
                syslog(LOG_DEBUG,
                       "Controlobject: [Driver] - Starting: %s ",
                       pDeviceItem->m_strName.c_str());
            }

            // Start  the driver logic
            pDeviceItem->startDriver(this);

        }  // Valid device item
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
// stopDeviceWorkerThreads
//

bool
CControlObject::stopDeviceWorkerThreads(void)
{
    CDeviceItem* pDeviceItem;

    if (m_debugFlags & VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG, "vscpd: [Driver] - Stopping drivers...");
    }
    std::deque<CDeviceItem*>::iterator iter;
    for (iter = m_deviceList.m_devItemList.begin();
         iter != m_deviceList.m_devItemList.end();
         ++iter) {

        pDeviceItem = *iter;
        if (NULL != pDeviceItem) {
            if (m_debugFlags & VSCP_DEBUG_EXTRA) {
                syslog(LOG_DEBUG,
                       "Controlobject: [Driver] - Stopping: %s ",
                       pDeviceItem->m_strName.c_str());
            }
            pDeviceItem->stopDriver();
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//  getMacAddress
//

bool
CControlObject::getMacAddress(cguid& guid)
{
#ifdef WIN32

    bool rv = false;
    NCB Ncb;
    UCHAR uRetCode;
    LANA_ENUM lenum;
    int i;

    // Clear the GUID
    guid.clear();

    memset(&Ncb, 0, sizeof(Ncb));
    Ncb.ncb_command = NCBENUM;
    Ncb.ncb_buffer  = (UCHAR*)&lenum;
    Ncb.ncb_length  = sizeof(lenum);
    uRetCode        = Netbios(&Ncb);
    // printf( "The NCBENUM return code is: 0x%x ", uRetCode );

    for (i = 0; i < lenum.length; i++) {
        memset(&Ncb, 0, sizeof(Ncb));
        Ncb.ncb_command  = NCBRESET;
        Ncb.ncb_lana_num = lenum.lana[i];

        uRetCode = Netbios(&Ncb);

        memset(&Ncb, 0, sizeof(Ncb));
        Ncb.ncb_command  = NCBASTAT;
        Ncb.ncb_lana_num = lenum.lana[i];

        strcpy((char*)Ncb.ncb_callname, "*               ");
        Ncb.ncb_buffer = (unsigned char*)&Adapter;
        Ncb.ncb_length = sizeof(Adapter);

        uRetCode = Netbios(&Ncb);

        if (uRetCode == 0) {
            guid.setAt(0, 0xff);
            guid.setAt(1, 0xff);
            guid.setAt(2, 0xff);
            guid.setAt(3, 0xff);
            guid.setAt(4, 0xff);
            guid.setAt(5, 0xff);
            guid.setAt(6, 0xff);
            guid.setAt(7, 0xfe);
            guid.setAt(8, Adapter.adapt.adapter_address[0]);
            guid.setAt(9, Adapter.adapt.adapter_address[1]);
            guid.setAt(10, Adapter.adapt.adapter_address[2]);
            guid.setAt(11, Adapter.adapt.adapter_address[3]);
            guid.setAt(12, Adapter.adapt.adapter_address[4]);
            guid.setAt(13, Adapter.adapt.adapter_address[5]);
            guid.setAt(14, 0);
            guid.setAt(15, 0);
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

            rv = true;
        }
    }

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
        if (m_debugFlags & VSCP_DEBUG_EXTRA) {
            syslog(LOG_DEBUG,
                   "Ethernet MAC address: %02X:%02X:%02X:%02X:%02X:%02X",
                   (uint8_t)s.ifr_addr.sa_data[0],
                   (uint8_t)s.ifr_addr.sa_data[1],
                   (uint8_t)s.ifr_addr.sa_data[2],
                   (uint8_t)s.ifr_addr.sa_data[3],
                   (uint8_t)s.ifr_addr.sa_data[4],
                   (uint8_t)s.ifr_addr.sa_data[5]);
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
    } else {
        syslog(LOG_ERR, "Failed to get hardware address (must be root?).");
        rv = false;
    }

    return rv;

#endif
}

///////////////////////////////////////////////////////////////////////////////
//  getIPAddress
//

bool
CControlObject::getIPAddress(cguid& guid)
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
    struct hostent* lpLocalHostEntry;
#endif
    lpLocalHostEntry = gethostbyname(szName);
    if (NULL == lpLocalHostEntry) {
        return false;
    }

    // Get all local addresses
    int idx = -1;
    void* pAddr;
    unsigned long localaddr[16];  // max 16 local addresses
    do {
        idx++;
        localaddr[idx] = 0;
        pAddr          = lpLocalHostEntry->h_addr_list[idx];
        if (NULL != pAddr)
            localaddr[idx] = *((unsigned long*)pAddr);
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
CControlObject::readEncryptionKey(const std::string& path)
{
    try {
        std::ifstream in(path, std::ifstream::in);
        std::stringstream strStream;
        strStream << in.rdbuf();
        m_vscptoken = strStream.str();
    }
    catch (...) {
        syslog(LOG_ERR,
                "[vscpl2drv-tcpipsrv] Failed to read encryption key file [%s]",
                path.c_str());
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// readJSON
//
// Read the configuration JSON data
//

bool
CControlObject::readJSON(const json& j)
{
    // ********************************************************************************
    //                                Main General
    // ********************************************************************************
    try {
        if (j.contains("runasuser")) { 
            m_runAsUser = j["runasuser"].get<std::string>();
        } else {
            syslog(LOG_ERR, "ReadConfig: Failed to read 'runasuser'. Must be present.");
            return false;
        }

        if (m_debugFlags & VSCP_DEBUG_CONFIG) {
            syslog(LOG_DEBUG, "ReadConfig: 'runasuser' set to %s", m_runAsUser.c_str());
        }
    }
    catch (...) {
        syslog(LOG_ERR, "ReadConfig: Failed to read 'runasuser'. Must be present.");
        return false;
    }

    try {
        if (j.contains("guid")) { 
            m_guid.getFromString(j["guid"].get<std::string>());
        } else {
            syslog(LOG_ERR, "ReadConfig: Failed to read 'guid'. Must be present.");
            return false;
        }

        if (m_debugFlags & VSCP_DEBUG_CONFIG) {
            syslog(LOG_DEBUG, "ReadConfig: 'guid' set to %s", m_guid.getAsString().c_str());
        }
    }
    catch (...) {
        syslog(LOG_ERR, "ReadConfig: Failed to read 'guid'. Must be present.");
        return false;
    }

    try {
        if (j.contains("servername")) { 
            m_strServerName = j["servername"].get<std::string>();
        } else {
            syslog(LOG_ERR, "ReadConfig: Failed to read 'servername'. Must be present.");
            return false;
        }

        if (m_debugFlags & VSCP_DEBUG_CONFIG) {
            syslog(LOG_DEBUG, "ReadConfig: 'servername' set to %s", m_strServerName.c_str());
        }
    }
    catch (...) {
        syslog(LOG_ERR, "ReadConfig: Failed to read 'servername'. Must be present.");
        return false;
    }

    try {
        if (j.contains("debug")) { 
            m_debugFlags = j["debug"].get<uint64_t>();
        } else {
            syslog(LOG_ERR, "ReadConfig: Failed to read 'debug'. Must be present.");
            return false;
        }

        if (m_debugFlags & VSCP_DEBUG_CONFIG) {
            syslog(LOG_DEBUG, "ReadConfig: 'debug' set to %lu", m_debugFlags);
        }
    }
    catch (...) {
        syslog(LOG_ERR, "ReadConfig: Failed to read 'debug'. Must be present.");
        return false;
    }

    try {
        if (j.contains("classtypedb")) {       
            m_pathClassTypeDefinitionDb = j["classtypedb"].get<std::string>();
        }
        else {
            syslog(LOG_ERR, "ReadConfig: Failed to read 'classtypedb'. Must be present.");
            return false;
        }

        if (m_debugFlags & VSCP_DEBUG_CONFIG) {
            syslog(LOG_DEBUG, "ReadConfig: 'classtypedb' set to %s", m_pathClassTypeDefinitionDb.c_str());
        }
    }
    catch (...) {
        syslog(LOG_ERR, "ReadConfig: Failed to read 'classtypedb'. Must be present.");
        return false;
    }    

    try {
        if (j.contains("maindb")) { 
            m_pathMainDb = j["maindb"].get<std::string>();
        } else {
            syslog(LOG_ERR, "ReadConfig: Failed to read 'maindb'. Must be present.");
            return false;
        }

        if (m_debugFlags & VSCP_DEBUG_CONFIG) {
            syslog(LOG_DEBUG, "ReadConfig: 'maindb' set to %s", m_pathMainDb.c_str());
        }
    }
    catch (...) {
        syslog(LOG_ERR, "ReadConfig: Failed to read 'maindb'. Must be present.");
        return false;
    }

    try {
        std::string pathvscpkey;
        if (j.contains("vscpkey")) {
            pathvscpkey = j["vscpkey"].get<std::string>();
        } else {
            syslog(LOG_ERR, "ReadConfig: 'Failed to read in encryption key %s", pathvscpkey.c_str());
            return false;
        }

        if (m_debugFlags & VSCP_DEBUG_CONFIG) {
            syslog(LOG_DEBUG, "ReadConfig: 'path to vscp key' set to %s", pathvscpkey.c_str());
        }

        if ( !readEncryptionKey(pathvscpkey) ) {
            syslog(LOG_ERR, "ReadConfig: 'Failed to read in encryption key %s", pathvscpkey.c_str());
            return false;
        }
    }
    catch (...) {
        syslog(LOG_ERR, "ReadConfig: Failed to read 'vscpkey'. Must be present.");
        return false;
    }


    try {
        if (j.contains("logging")) {
            m_path_to_log_file = j["logging"]["path"].get<std::string>();
        } 
        else {
            syslog(LOG_ERR, "ReadConfig: 'Failed to read in path to logfile key %s", m_path_to_log_file.c_str());
            return false;
        }

        if (m_debugFlags & VSCP_DEBUG_CONFIG) {
            syslog(LOG_DEBUG, "ReadConfig: 'path to vscp key' set to %s", m_path_to_log_file.c_str());
        }
    }
    catch (...) {
        syslog(LOG_ERR, "ReadConfig: Failed to read 'logging/path to log file'. Must be present.");
        return false;
    }


    // ********************************************************************************
    //                                     Main MQTT
    // ********************************************************************************

    if ( !(j.contains("mqtt") && j["mqtt"].is_object())) {
        if (m_debugFlags & VSCP_DEBUG_CONFIG) {
            syslog(LOG_DEBUG, "ReadConfig: mqtt object. Defaults will be used.");
        }
    } else {
        // MQTT.host
        if (j["mqtt"].contains("host")) {
            m_mqtt_strHost = j["mqtt"]["host"].get<std::string>();
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: MQTT 'host' set to %s", m_mqtt_strHost.c_str());
            }
        } else  {
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'host'. Defaults will be used.");
            }
        }

        // MQTT.port
        if (j["mqtt"].contains("port") ) {
            m_mqtt_port = j["mqtt"]["port"].get<int>();
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: MQTT 'port' set to %d", m_mqtt_port);
            }
        } else  {
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'port'. Defaults will be used.");
            }
        }

        // MQTT.user
        if (j["mqtt"].contains("user") ) {
            m_mqtt_strUserName = j["mqtt"]["user"].get<std::string>();
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: MQTT 'user' set to %s", m_mqtt_strUserName.c_str());
            }
        } else  {
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'user'. Defaults will be used.");
            }
        }

        // MQTT.password
        if (j["mqtt"].contains("password") ) {
            m_mqtt_strPassword = j["mqtt"]["password"].get<std::string>();
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: MQTT 'password' set to %s", m_mqtt_strPassword.c_str());
            }
        } else  {
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'password'. Defaults will be used.");
            }
        }

        // MQTT.clientid
        if (j["mqtt"].contains("clientid") ) {
            m_mqtt_strClientId = j["mqtt"]["clientid"].get<std::string>();
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: MQTT 'clientid' set to %s", m_mqtt_strClientId.c_str());
            }
        } else  {
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'clientid'. Defaults will be used.");
            }
        }

        // MQTT.format
        if (j["mqtt"].contains("format") ) {
            std::string format = j["mqtt"]["format"].get<std::string>();
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: MQTT 'format' set to %s", format.c_str());
            }
            
            vscp_makeUpper(format);
            if (0 == vscp_strcasecmp(format.c_str(), "JSON")) {
                m_mqtt_format = jsonfmt;
                if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                    syslog(LOG_DEBUG, "ReadConfig: MQTT format set to 'JSON' (%s)\n", format.c_str());
                }
            } else if (0 == vscp_strcasecmp(format.c_str(), "XML")) {
                m_mqtt_format = xmlfmt;
                if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                    syslog(LOG_DEBUG, "ReadConfig: MQTT format set to 'XML' (%s)\n", format.c_str());
                }
            } else if (0 == vscp_strcasecmp(format.c_str(), "STRING")) {
                m_mqtt_format = strfmt;
                if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                    syslog(LOG_DEBUG, "ReadConfig: MQTT format set to 'STRING' (%s)\n", format.c_str());
                }
            } else if (0 == vscp_strcasecmp(format.c_str(), "BINARY")) {
                m_mqtt_format = binfmt;
                if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                    syslog(LOG_DEBUG, "ReadConfig: MQTT format set to 'BINARY' (%s)\n", format.c_str());
                }
            } else {
                m_mqtt_format = jsonfmt;
                if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                    syslog(LOG_DEBUG, "ReadConfig: Unknown format: MQTT format set to 'JSON' (%s)\n", format.c_str());
                }
            }
        } else  {
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'format'. Defaults will be used.");
            }
        }

        // MQTT.qos
        if (j["mqtt"].contains("qos")) {
            m_mqtt_qos = j["mqtt"]["qos"].get<int>();
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: MQTT 'qos' set to %d", m_mqtt_qos);
            }
            if (m_mqtt_qos > 2) {
                syslog(LOG_WARNING, "ReadConfig: MQTT qos > 2. Set to 0.");
                m_mqtt_qos = 0;
            }
        } else  {
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'qos'. Defaults will be used.");
            }
        }

        // MQTT.bcleansession
        if (j["mqtt"].contains("bcleansession")) {
            m_mqtt_bCleanSession = j["mqtt"]["bcleansession"].get<bool>();
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: MQTT 'bcleansession' set to %d", m_mqtt_bCleanSession);
            }
        } else  {
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'bcleansession'. Defaults will be used.");
            }
        }

        // MQTT.bretain
        if (j["mqtt"].contains("bretain")) {
            m_mqtt_bRetain = j["mqtt"]["bretain"].get<bool>();
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: MQTT 'bretain' set to %d", m_mqtt_bRetain);
            }
        } else  {
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'bretain'. Defaults will be used.");
            }
        }

        // MQTT.keepalive
        if (j["mqtt"].contains("keepalive")) {
            m_mqtt_keepalive = j["mqtt"]["keepalive"].get<int>();
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: MQTT 'keepalive' set to %d", m_mqtt_keepalive);
            }
        } else  {
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'keepalive'. Defaults will be used.");
            }
        }

        // MQTT.reconnect-delay
        if (j["mqtt"].contains("reconnect-delay")) {
            m_mqtt_reconnect_delay = j["mqtt"]["reconnect-delay"].get<int>();
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: MQTT 'reconnect-delay' set to %d", m_mqtt_reconnect_delay);
            }
        } else  {
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'reconnect-delay'. Defaults will be used.");
            }
        }

        // MQTT.reconnect-delay-max
        if (j["mqtt"].contains("reconnect-delay-max")) {
            m_mqtt_reconnect_delay_max = j["mqtt"]["reconnect-delay-max"].get<int>();
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: MQTT 'reconnect-delay-max' set to %d", m_mqtt_reconnect_delay);
            }
        } else  {
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'm_mqtt_reconnect_delay_max'. Defaults will be used.");
            }
        }

        // MQTT.reconnect-exponential-backoff
        if (j["mqtt"].contains("reconnect-exponential-backoff")) {
            m_mqtt_reconnect_exponential_backoff = j["mqtt"]["reconnect-exponential-backoff"].get<bool>();
    
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: MQTT 'reconnect-exponential-backoff' set to %d", m_mqtt_reconnect_exponential_backoff);
            }
        } else {
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'reconnect_exponential_backoff'. Defaults will be used.");
            }
        }

        // MQTT.cafile
        if (j["mqtt"].contains("cafile")) {
            m_mqtt_cafile = j["mqtt"]["cafile"].get<std::string>();
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: MQTT 'cafile' set to %s", m_mqtt_cafile.c_str());
            }
        } else  {
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'cafile'. Defaults will be used.");
            }
        }

        // MQTT.capath
        if (j["mqtt"].contains("capath")) {
            m_mqtt_capath = j["mqtt"]["capath"].get<std::string>();
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: MQTT 'capath' set to %s", m_mqtt_capath.c_str());
            }
        } else  {
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'capath'. Defaults will be used.");
            }
        }

        // MQTT.certfile
        if (j["mqtt"].contains("certfile")) {
            m_mqtt_capath = j["mqtt"]["certfile"].get<std::string>();
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: MQTT 'certfile' set to %s", m_mqtt_capath.c_str());
            }
        } else  {
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'certfile'. Defaults will be used.");
            }
        }

        // MQTT.keyfile
        if (j["mqtt"].contains("keyfile")) {
            m_mqtt_keyfile = j["mqtt"]["keyfile"].get<std::string>();
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: MQTT 'keyfile' set to %s", m_mqtt_keyfile.c_str());
            }
        } else  {
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'keyfile'. Defaults will be used.");
            }
        }

        // MQTT.pwkeyfile
        if (j["mqtt"].contains("pwkeyfile")) {
            m_mqtt_pwKeyfile = j["mqtt"]["pwkeyfile"].get<std::string>();
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: MQTT 'pwkeyfile' set to %s", m_mqtt_pwKeyfile.c_str());
            }
        } else  {
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'pwkeyfile'. Defaults will be used.");
            }
        }

        // MQTT.topic-interfaces
        if (j["mqtt"].contains("topic-interfaces")) {
            m_topicInterfaces = j["mqtt"]["topic-interfaces"].get<std::string>();
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: MQTT 'topic-interfaces' set to %s", m_topicInterfaces.c_str());
            }
        } else {
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'topic-interfaces'. Defaults will be used.");
            }
        }

        // subscribe
        if (!(j["mqtt"].contains("subscribe") && j["mqtt"]["subscribe"].is_array())) {
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: mqtt.subscribe object. Defaults will be used.");
            }
        } else {
            json sub = j["mqtt"]["subscribe"];
            for (json::iterator it = sub.begin(); it != sub.end(); ++it) {
                m_mqtt_subscriptions.push_back((*it).get<std::string>());
                if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                    syslog(LOG_DEBUG, "ReadConfig: mqtt.subscription %s.", (*it).get<std::string>().c_str());
                }
            }
        }

        // publish
        if (!(j["mqtt"].contains("publish") && j["mqtt"]["publish"].is_array())) {
            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                syslog(LOG_DEBUG, "ReadConfig: mqtt.publish object. Defaults will be used.");
            }
        } else {
            json sub = j["mqtt"]["publish"];
            for (json::iterator it = sub.begin(); it != sub.end(); ++it) {
                m_mqtt_publish.push_back((*it).get<std::string>());
                if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                    syslog(LOG_DEBUG, "ReadConfig: mqtt.publish %s.", (*it).get<std::string>().c_str());
                }
            }
        }
    } //    end MQTT block





// ------------------------------------------------------------------------------------------------------------------------
//                                             Level I Drivers
// ------------------------------------------------------------------------------------------------------------------------





    if (!(j.contains("drivers") && j["drivers"].is_object())) {
        if (m_debugFlags & VSCP_DEBUG_CONFIG) {
            syslog(LOG_DEBUG, "ReadConfig: drivers object. No drivers will be used.");
        }
    } else {
        // Level I drivers
        if (!(j["drivers"].contains("level1") && j["drivers"]["level1"].is_object())) {

            json sub = j["drivers"]["level1"];
            for (json::iterator it = sub.begin(); it != sub.end(); ++it) {

                if ((*it).value("enable", false) &&
                    (*it)["name"].is_string() &&
                    (*it)["config"].is_string() &&
                    (*it)["path"].is_string() &&
                    (*it)["flags"].is_number() &&
                    (*it)["guid"].is_string() &&
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
                        syslog(LOG_ERR,
                            "Level I driver not added name=%s. "
                            "Path does not exist. - [%s]",
                            (*it)["name"].get<std::string>().c_str(),
                            (*it)["path"].get<std::string>().c_str());
                    } else {

                        if (m_debugFlags & VSCP_DEBUG_DRIVERL1) {
                            syslog(LOG_DEBUG,
                                "ReadConfig: Level I driver added. name = %s - [%s]",
                                (*it)["name"].get<std::string>().c_str(),
                                (*it)["path"].get<std::string>().c_str());
                        }

                        // ********************************************************************************
                        //                               Level I driver MQTT
                        // ********************************************************************************

                        if (!((*it).contains("mqtt") && (*it)["mqtt"].is_object())) {
                            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                syslog(LOG_DEBUG, "ReadConfig: mqtt object. Defaults will be used.");
                            }
                        } else {

                            CDeviceItem *pDriver = m_deviceList.getDeviceItemFromName((*it)["name"]);
                            if ( NULL == pDriver ) {
                                syslog(LOG_DEBUG, "ReadConfig: Driver MQTT info can not be added. Driver not found.");
                            } else {

                                // MQTT.host
                                if ((*it)["mqtt"].contains("host") ) {
                                    pDriver->m_mqtt_strHost = (*it)["mqtt"]["host"].get<std::string>();
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'host' set to %s", pDriver->m_mqtt_strHost.c_str());
                                    }
                                } else  {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'host'. Defaults will be used.");
                                    }
                                }

                                // MQTT.port
                                if ((*it)["mqtt"].contains("port") ) {
                                    pDriver->m_mqtt_port = (*it)["mqtt"]["port"].get<int>();
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'port' set to %d", pDriver->m_mqtt_port);
                                    }
                                } else  {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'port'. Defaults will be used.");
                                    }
                                }

                                // MQTT.user
                                if ((*it)["mqtt"].contains("user") ) {
                                    pDriver->m_mqtt_strUserName = (*it)["mqtt"]["user"].get<std::string>();
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'user' set to %s", pDriver->m_mqtt_strUserName.c_str());
                                    }
                                } else  {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'user'. Defaults will be used.");
                                    }
                                }

                                // MQTT.password
                                if ((*it)["mqtt"].contains("password") ) {
                                    pDriver->m_mqtt_strPassword = (*it)["mqtt"]["password"].get<std::string>();
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'password' set to %s", pDriver->m_mqtt_strPassword.c_str());
                                    }
                                } else  {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'password'. Defaults will be used.");
                                    }
                                }

                                // MQTT.clientid
                                if ((*it)["mqtt"].contains("clientid") ) {
                                    pDriver->m_mqtt_strClientId = (*it)["mqtt"]["clientid"].get<std::string>();
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'clientid' set to %s", pDriver->m_mqtt_strClientId.c_str());
                                    }
                                } else  {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'clientid'. Defaults will be used.");
                                    }
                                }

                                // MQTT.format
                                if ((*it)["mqtt"].contains("format")) {
                                    std::string format = (*it)["mqtt"]["format"].get<std::string>();
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'format' set to %s", format.c_str());
                                    }

                                    vscp_makeUpper(format);
                                    if (0 == vscp_strcasecmp(format.c_str(), "JSON")) {
                                        pDriver->m_mqtt_format = jsonfmt;
                                        if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                            syslog(LOG_DEBUG, "ReadConfig: MQTT format set to 'JSON' (%s)\n", format.c_str());
                                        }
                                    } else if (0 == vscp_strcasecmp(format.c_str(), "XML")) {
                                        pDriver->m_mqtt_format = xmlfmt;
                                        if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                            syslog(LOG_DEBUG, "ReadConfig: MQTT format set to 'XML' (%s)\n", format.c_str());
                                        }
                                    } else if (0 == vscp_strcasecmp(format.c_str(), "STRING")) {
                                        pDriver->m_mqtt_format = strfmt;
                                        if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                            syslog(LOG_DEBUG, "ReadConfig: MQTT format set to 'STRING' (%s)\n", format.c_str());
                                        }
                                    } else if (0 == vscp_strcasecmp(format.c_str(), "BINARY")) {
                                        pDriver->m_mqtt_format = binfmt;
                                        if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                            syslog(LOG_DEBUG, "ReadConfig: MQTT format set to 'BINARY' (%s)\n", format.c_str());
                                        }
                                    } else {
                                        pDriver->m_mqtt_format = jsonfmt;
                                        if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                            syslog(LOG_DEBUG, "ReadConfig: Unknown format: MQTT format set to 'JSON' (%s)\n", format.c_str());
                                        }
                                    }
                                } else  {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'format'. Defaults will be used.");
                                    }
                                }

                                // MQTT.qos
                                if ((*it)["mqtt"].contains("qos")) {
                                    pDriver->m_mqtt_qos = (*it)["mqtt"]["qos"].get<int>();
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'qos' set to %d", pDriver->m_mqtt_qos);
                                    }
                                    if (pDriver->m_mqtt_qos > 2) {
                                        syslog(LOG_WARNING, "ReadConfig: MQTT qos > 2. Set to 0.");
                                        pDriver->m_mqtt_qos = 0;
                                    }
                                } else  {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'qos'. Defaults will be used.");
                                    }
                                }

                                // MQTT.bcleansession
                                if ((*it)["mqtt"].contains("bcleansession")) {
                                    pDriver->m_mqtt_bCleanSession = (*it)["mqtt"]["bcleansession"].get<bool>();
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'bcleansession' set to %d", pDriver->m_mqtt_bCleanSession);
                                    }
                                } else  {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'bcleansession'. Defaults will be used.");
                                    }
                                }

                                // MQTT.bretain
                                if ((*it)["mqtt"].contains("bretain")) {
                                    pDriver->m_mqtt_bRetain = (*it)["mqtt"]["bretain"].get<bool>();
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'bretain' set to %d", pDriver->m_mqtt_bRetain);
                                    }
                                } else  {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'bretain'. Defaults will be used.");
                                    }
                                }

                                // MQTT.keepalive
                                if ((*it)["mqtt"].contains("keepalive")) {
                                    pDriver->m_mqtt_keepalive = (*it)["mqtt"]["keepalive"].get<int>();
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'keepalive' set to %d", pDriver->m_mqtt_keepalive);
                                    }
                                } else  {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'keepalive'. Defaults will be used.");
                                    }
                                }

                                // MQTT.reconnect-delay
                                if ((*it)["mqtt"].contains("reconnect-delay")) {
                                    pDriver->m_mqtt_reconnect_delay = (*it)["mqtt"]["reconnect-delay"].get<int>();
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'reconnect-delay' set to %d", pDriver->m_mqtt_reconnect_delay);
                                    }
                                } else  {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'reconnect-delay'. Defaults will be used.");
                                    }
                                }

                                // MQTT.reconnect-delay-max
                                if ((*it)["mqtt"].contains("reconnect-delay-max")) {
                                    pDriver->m_mqtt_reconnect_delay_max = (*it)["mqtt"]["reconnect-delay-max"].get<int>();
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'reconnect-delay-max' set to %d", pDriver->m_mqtt_reconnect_delay);
                                    }
                                } else  {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'm_mqtt_reconnect_delay_max'. Defaults will be used.");
                                    }
                                }

                                // MQTT.reconnect-exponential-backoff
                                if ((*it)["mqtt"].contains("reconnect-exponential-backoff")) {
                                    pDriver->m_mqtt_reconnect_exponential_backoff = (*it)["mqtt"]["reconnect-exponential-backoff"].get<bool>();

                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'reconnect-exponential-backoff' set to %d", pDriver->m_mqtt_reconnect_exponential_backoff);
                                    }
                                } else {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'reconnect_exponential_backoff'. Defaults will be used.");
                                    }
                                }

                                // MQTT.cafile
                                if ((*it)["mqtt"].contains("cafile")) {
                                    pDriver->m_mqtt_cafile = (*it)["mqtt"]["cafile"].get<std::string>();
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'cafile' set to %s", pDriver->m_mqtt_cafile.c_str());
                                    }
                                } else  {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'cafile'. Defaults will be used.");
                                    }
                                }

                                // MQTT.capath
                                if ((*it)["mqtt"].contains("capath")) {
                                    pDriver->m_mqtt_capath = (*it)["mqtt"]["capath"].get<std::string>();
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'capath' set to %s", pDriver->m_mqtt_capath.c_str());
                                    }
                                } else  {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'capath'. Defaults will be used.");
                                    }
                                }

                                // MQTT.certfile
                                if ((*it)["mqtt"].contains("certfile")) {
                                    pDriver->m_mqtt_capath = (*it)["mqtt"]["certfile"].get<std::string>();
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'certfile' set to %s", pDriver->m_mqtt_capath.c_str());
                                    }
                                } else  {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'certfile'. Defaults will be used.");
                                    }
                                }

                                // MQTT.keyfile
                                if ((*it)["mqtt"].contains("keyfile")) {
                                    pDriver->m_mqtt_keyfile = (*it)["mqtt"]["keyfile"].get<std::string>();
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'keyfile' set to %s", pDriver->m_mqtt_keyfile.c_str());
                                    }
                                } else  {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'keyfile'. Defaults will be used.");
                                    }
                                }

                                // MQTT.pwkeyfile
                                if ((*it)["mqtt"].contains("pwkeyfile")) {
                                    pDriver->m_mqtt_pwKeyfile = (*it)["mqtt"]["pwkeyfile"].get<std::string>();
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'pwkeyfile' set to %s", pDriver->m_mqtt_pwKeyfile.c_str());
                                    }
                                } else  {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'pwkeyfile'. Defaults will be used.");
                                    }
                                }

                                // subscribe
                                if (!((*it)["mqtt"].contains("subscribe") && (*it)["mqtt"]["subscribe"].is_array())) {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: mqtt.subscribe object. Defaults will be used.");
                                    }
                                } else {
                                    json subsub = (*it)["mqtt"]["subscribe"];
                                    for (json::iterator it2 = subsub.begin(); it2 != subsub.end(); ++it2) {
                                        pDriver->m_mqtt_subscriptions.push_back((*it2).get<std::string>());
                                        if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                            syslog(LOG_DEBUG, "ReadConfig: mqtt.subscription %s.", subsub.get<std::string>().c_str());
                                        }
                                    }
                                }

                                // publish
                                if (!((*it)["mqtt"].contains("publish") && (*it)["mqtt"]["publish"].is_array())) {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: mqtt.publish object. Defaults will be used.");
                                    }
                                } else {
                                    json subsub = (*it)["mqtt"]["publish"];
                                    for (json::iterator it2 = subsub.begin(); it2 != subsub.end(); ++it2) {
                                        pDriver->m_mqtt_publish.push_back((*it2).get<std::string>());
                                        if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                            syslog(LOG_DEBUG, "ReadConfig: mqtt.publish %s.", subsub.get<std::string>().c_str());
                                        }
                                    }
                                }
                            } //    MQTT block
                        }

                    }
                }
            }
            
        }  // end - level I drivers






// ------------------------------------------------------------------------------------------------------------------------
//                                             Level II Drivers
// ------------------------------------------------------------------------------------------------------------------------





        // Level II drivers
        if (!(j["drivers"].contains("level2") && j["drivers"]["level2"].is_object())) {
            json sub = j["drivers"]["level2"];
            for (json::iterator it = sub.begin(); it != sub.end(); ++it) {
                //std::cout << (*it)["name"] << '\n';
                if ((*it).value("enable", false) &&
                    (*it)["name"].is_string() &&
                    (*it)["path-config"].is_string() &&
                    (*it)["path-driver"].is_string() &&
                    (*it)["guid"].is_string()) {
                    // Add the level I device
                    if (!m_deviceList.addItem( this,
                                                (*it)["name"].get<std::string>(),
                                                (*it)["path-config"].get<std::string>(),
                                                (*it)["path-driver"].get<std::string>(),
                                                0,
                                                (*it)["guid"].get<std::string>(),
                                                VSCP_DRIVER_LEVEL2)) {
                        syslog(LOG_ERR,
                            "Level II driver not added name=%s. "
                            "Path does not exist. - [%s]",
                            (*it)["name"].get<std::string>().c_str(),
                            (*it)["path-driver"].get<std::string>().c_str());
                    } else {
                        if (m_debugFlags & VSCP_DEBUG_DRIVERL1) {
                            syslog(LOG_DEBUG,
                                "ReadConfig: Level II driver added. name = %s - [%s]",
                                (*it)["name"].get<std::string>().c_str(),
                                (*it)["path-driver"].get<std::string>().c_str());
                        }

                        // ********************************************************************************
                        //                               Level II driver MQTT
                        // ********************************************************************************

                        if (!((*it).contains("mqtt") && (*it)["mqtt"].is_object())) {
                            if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                syslog(LOG_DEBUG, "ReadConfig: mqtt object. Defaults will be used.");
                            }
                        } else {

                            CDeviceItem *pDriver = m_deviceList.getDeviceItemFromName((*it)["name"]);
                            if ( NULL == pDriver ) {
                                syslog(LOG_DEBUG, "ReadConfig: Driver MQTT info can not be added. Driver not found.");
                            } else {

                                // MQTT.host
                                if ((*it)["mqtt"].contains("host")) {
                                    pDriver->m_mqtt_strHost = (*it)["mqtt"]["host"].get<std::string>();
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'host' set to %s", pDriver->m_mqtt_strHost.c_str());
                                    }
                                } else  {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'host'. Defaults will be used.");
                                    }
                                }

                                // MQTT.port
                                if ((*it)["mqtt"].contains("port")) {
                                    pDriver->m_mqtt_port = (*it)["mqtt"]["port"].get<int>();
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'port' set to %d", pDriver->m_mqtt_port);
                                    }
                                } else  {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'port'. Defaults will be used.");
                                    }
                                }

                                // MQTT.user
                                if ((*it)["mqtt"].contains("user")) {
                                    pDriver->m_mqtt_strUserName = (*it)["mqtt"]["user"].get<std::string>();
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'user' set to %s", pDriver->m_mqtt_strUserName.c_str());
                                    }
                                } else  {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'user'. Defaults will be used.");
                                    }
                                }

                                // MQTT.password
                                if ((*it)["mqtt"].contains("password")) {
                                    pDriver->m_mqtt_strPassword = (*it)["mqtt"]["password"].get<std::string>();
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'password' set to %s", pDriver->m_mqtt_strPassword.c_str());
                                    }
                                } else  {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'password'. Defaults will be used.");
                                    }
                                }

                                // MQTT.clientid
                                if ((*it)["mqtt"].contains("clientid")) {
                                    pDriver->m_mqtt_strClientId = (*it)["mqtt"]["clientid"].get<std::string>();
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'clientid' set to %s", pDriver->m_mqtt_strClientId.c_str());
                                    }
                                } else  {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'clientid'. Defaults will be used.");
                                    }
                                }

                                // MQTT.format
                                if ((*it)["mqtt"].contains("format")) {

                                    std::string format = (*it)["mqtt"]["format"].get<std::string>();
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'format' set to %s", format.c_str());
                                    }

                                    vscp_makeUpper(format);
                                    if (0 == vscp_strcasecmp(format.c_str(), "JSON")) {
                                        pDriver->m_mqtt_format = jsonfmt;
                                        if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                            syslog(LOG_DEBUG, "ReadConfig: MQTT format set to 'JSON' (%s)\n", format.c_str());
                                        }
                                    } else if (0 == vscp_strcasecmp(format.c_str(), "XML")) {
                                        pDriver->m_mqtt_format = xmlfmt;
                                        if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                            syslog(LOG_DEBUG, "ReadConfig: MQTT format set to 'XML' (%s)\n", format.c_str());
                                        }
                                    } else if (0 == vscp_strcasecmp(format.c_str(), "STRING")) {
                                        pDriver->m_mqtt_format = strfmt;
                                        if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                            syslog(LOG_DEBUG, "ReadConfig: MQTT format set to 'STRING' (%s)\n", format.c_str());
                                        }
                                    } else if (0 == vscp_strcasecmp(format.c_str(), "BINARY")) {
                                        pDriver->m_mqtt_format = binfmt;
                                        if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                            syslog(LOG_DEBUG, "ReadConfig: MQTT format set to 'BINARY' (%s)\n", format.c_str());
                                        }
                                    } else {
                                        pDriver->m_mqtt_format = jsonfmt;
                                        if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                            syslog(LOG_DEBUG, "ReadConfig: Unknown format: MQTT format set to 'JSON' (%s)\n", format.c_str());
                                        }
                                    }
                                } else  {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'format'. Defaults will be used.");
                                    }
                                }

                                // MQTT.qos
                                if ((*it)["mqtt"].contains("qos")) {
                                    pDriver->m_mqtt_qos = (*it)["mqtt"]["qos"].get<int>();
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'qos' set to %d", pDriver->m_mqtt_qos);
                                    }
                                    if (pDriver->m_mqtt_qos > 2) {
                                        syslog(LOG_WARNING, "ReadConfig: MQTT qos > 2. Set to 0.");
                                        pDriver->m_mqtt_qos = 0;
                                    }
                                } else  {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'qos'. Defaults will be used.");
                                    }
                                }

                                // MQTT.bcleansession
                                if ((*it)["mqtt"].contains("bcleansession")) {
                                    pDriver->m_mqtt_bCleanSession = (*it)["mqtt"]["bcleansession"].get<bool>();
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'bcleansession' set to %d", pDriver->m_mqtt_bCleanSession);
                                    }
                                } else  {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'bcleansession'. Defaults will be used.");
                                    }
                                }

                                // MQTT.bretain
                                if ((*it)["mqtt"].contains("bretain")) {
                                    pDriver->m_mqtt_bRetain = (*it)["mqtt"]["bretain"].get<bool>();
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'bretain' set to %d", pDriver->m_mqtt_bRetain);
                                    }
                                } else  {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'bretain'. Defaults will be used.");
                                    }
                                }

                                // MQTT.keepalive
                                if ((*it)["mqtt"].contains("keepalive")) {
                                    pDriver->m_mqtt_keepalive = (*it)["mqtt"]["keepalive"].get<int>();
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'keepalive' set to %d", pDriver->m_mqtt_keepalive);
                                    }
                                } else  {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'keepalive'. Defaults will be used.");
                                    }
                                }

                                // MQTT.reconnect-delay
                                if ((*it)["mqtt"].contains("reconnect-delay")) {
                                    pDriver->m_mqtt_reconnect_delay = (*it)["mqtt"]["reconnect-delay"].get<int>();
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'reconnect-delay' set to %d", pDriver->m_mqtt_reconnect_delay);
                                    }
                                } else  {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'reconnect-delay'. Defaults will be used.");
                                    }
                                }

                                // MQTT.reconnect-delay-max
                                if ((*it)["mqtt"].contains("reconnect-delay-max")) {
                                    pDriver->m_mqtt_reconnect_delay_max = (*it)["mqtt"]["reconnect-delay-max"].get<int>();
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'reconnect-delay-max' set to %d", pDriver->m_mqtt_reconnect_delay);
                                    }
                                } else  {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'm_mqtt_reconnect_delay_max'. Defaults will be used.");
                                    }
                                }

                                // MQTT.reconnect-exponential-backoff
                                if ((*it)["mqtt"].contains("reconnect-exponential-backoff")) {
                                    pDriver->m_mqtt_reconnect_exponential_backoff = (*it)["mqtt"]["reconnect-exponential-backoff"].get<bool>();

                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'reconnect-exponential-backoff' set to %d", pDriver->m_mqtt_reconnect_exponential_backoff);
                                    }
                                } else {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'reconnect_exponential_backoff'. Defaults will be used.");
                                    }
                                }

                                // MQTT.cafile
                                if ((*it)["mqtt"].contains("cafile")) {
                                    pDriver->m_mqtt_cafile = (*it)["mqtt"]["cafile"].get<std::string>();
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'cafile' set to %s", pDriver->m_mqtt_cafile.c_str());
                                    }
                                } else  {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'cafile'. Defaults will be used.");
                                    }
                                }

                                // MQTT.capath
                                if ((*it)["mqtt"].contains("capath")) {
                                    pDriver->m_mqtt_capath = (*it)["mqtt"]["capath"].get<std::string>();
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'capath' set to %s", pDriver->m_mqtt_capath.c_str());
                                    }
                                } else  {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'capath'. Defaults will be used.");
                                    }
                                }

                                // MQTT.certfile
                                if ((*it)["mqtt"].contains("certfile")) {
                                    pDriver->m_mqtt_capath = (*it)["mqtt"]["certfile"].get<std::string>();
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'certfile' set to %s", pDriver->m_mqtt_capath.c_str());
                                    }
                                } else  {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'certfile'. Defaults will be used.");
                                    }
                                }

                                // MQTT.keyfile
                                if ((*it)["mqtt"].contains("keyfile")) {
                                    pDriver->m_mqtt_keyfile = (*it)["mqtt"]["keyfile"].get<std::string>();
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'keyfile' set to %s", pDriver->m_mqtt_keyfile.c_str());
                                    }
                                } else  {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'keyfile'. Defaults will be used.");
                                    }
                                }

                                // MQTT.pwkeyfile
                                if ((*it)["mqtt"].contains("pwkeyfile")) {
                                    pDriver->m_mqtt_pwKeyfile = (*it)["mqtt"]["pwkeyfile"].get<std::string>();
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: MQTT 'pwkeyfile' set to %s", pDriver->m_mqtt_pwKeyfile.c_str());
                                    }
                                } else  {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: Failed to read MQTT 'pwkeyfile'. Defaults will be used.");
                                    }
                                }

                                // subscribe
                                if (!((*it)["mqtt"].contains("subscribe") && (*it)["mqtt"]["subscribe"].is_array())) {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: mqtt.subscribe object. Defaults will be used.");
                                    }
                                } else {
                                    json subsub = (*it)["mqtt"]["subscribe"];
                                    for (json::iterator it = subsub.begin(); it != subsub.end(); ++it) {
                                        pDriver->m_mqtt_subscriptions.push_back((*it).get<std::string>());
                                        if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                            syslog(LOG_DEBUG, "ReadConfig: mqtt.subscription %s.", subsub.get<std::string>().c_str());
                                        }
                                    }
                                }

                                // publish
                                if (!((*it)["mqtt"].contains("publish") && (*it)["mqtt"]["publish"].is_array())) {
                                    if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                        syslog(LOG_DEBUG, "ReadConfig: mqtt.publish object. Defaults will be used.");
                                    }
                                } else {
                                    json subsub = (*it)["mqtt"]["publish"];
                                    for (json::iterator it = subsub.begin(); it != subsub.end(); ++it) {
                                        pDriver->m_mqtt_publish.push_back((*it).get<std::string>());
                                        if (m_debugFlags & VSCP_DEBUG_CONFIG) {
                                            syslog(LOG_DEBUG, "ReadConfig: mqtt.publish %s.", subsub.get<std::string>().c_str());
                                        }
                                    }
                                }
                            }  // MQTT block driver II
                        }
                    }
                }
            }
        }  // level II drivers


    }  // drivers block

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// readConfiguration
//
// Read the configuration JSON file
//

bool
CControlObject::readConfiguration(const std::string& strcfgfile)
{
    if (m_debugFlags & VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG,
               "Reading full JSON configuration from [%s]",
               (const char*)strcfgfile.c_str());
    }

    json j;

    try {
        std::ifstream in(strcfgfile, std::ifstream::in);
        in >> j;
    }
    catch (...) {
        perror("vscpd:  Failed to parse configuration file.\n");
        syslog(LOG_ERR, "vscpd:  Failed to parse JSON configuration.");
        return false;
    }


    return readJSON(j);


}   // JSON config




