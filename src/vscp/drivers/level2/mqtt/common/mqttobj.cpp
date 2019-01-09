// mqttobj.cpp: implementation of the CMQTT class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP Project (http://www.vscp.org)
//
// Copyright (C) 2000-2019 Ake Hedman,
// Grodans Paradis AB, <akhe@grodansparadis.com>
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

#include <list>
#include <map>
#include <string>

#include "limits.h"
#include "stdlib.h"
#include "syslog.h"
#include "unistd.h"
#include <ctype.h>
#include <libgen.h>
#include <net/if.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <time.h>

#include <mongoose.h>

#include <vscp_class.h>
#include <vscp_type.h>
#include <vscphelper.h>
#include <vscpremotetcpif.h>

#include "mqttobj.h"

// Forward declarations
void *
workerTread(void *pData);

////////////////////////////////////////////////////////////////////////////////
// ev_handler
//

static void
ev_handler(struct mg_connection *nc, int ev, void *p)
{
    char username[80];
    char password[80];
    struct mg_mqtt_message *msg = (struct mg_mqtt_message *)p;
    Cmqttobj *pmqttobj          = (Cmqttobj *)nc->mgr->user_data;

    switch (ev) {

        case MG_EV_CONNECT: {
            struct mg_send_mqtt_handshake_opts opts;

            opts.flags      = MG_MQTT_CLEAN_SESSION;
            opts.keep_alive = pmqttobj->m_keepalive;

            // Username
            opts.user_name = NULL;
            if (pmqttobj->m_username.length()) {
                strncpy(
                  username,
                  pmqttobj->m_username.c_str(),
                  std::min(sizeof(username), pmqttobj->m_username.length()));
                opts.user_name = username;
                opts.flags |= MG_MQTT_HAS_USER_NAME;
            }

            // Password
            opts.password = NULL;
            if (pmqttobj->m_password.length()) {
                strncpy(
                  password,
                  pmqttobj->m_password.c_str(),
                  std::min(sizeof(password), pmqttobj->m_password.length()));
                opts.password = password;
                opts.flags |= MG_MQTT_HAS_PASSWORD;
            }

            mg_set_protocol_mqtt(nc);

            if (pmqttobj->m_bSubscribe) {
                opts.password  = NULL;
                opts.user_name = NULL;
            } else {
                opts.flags      = MG_MQTT_CLEAN_SESSION;
                opts.keep_alive = 60;
                opts.password   = NULL;
                opts.user_name  = NULL;

                mg_send_mqtt_handshake_opt(
                  nc, pmqttobj->m_sessionid.c_str(), opts);

                // mg_send_mqtt_handshake( nc, pmqttobj->m_sessionid.c_str()
                // );
            }

            syslog(LOG_INFO, "VSCP MQTT Driver - Connection opened.\n");

        } break;

        case MG_EV_MQTT_CONNACK:
            if (msg->connack_ret_code != MG_EV_MQTT_CONNACK_ACCEPTED) {
                syslog(LOG_ERR,
                       "VSCP MQTT Driver - Got MQTT connection error: %d\n",
                       msg->connack_ret_code);
                pmqttobj->m_bQuit = true;
                return;
            }

            pmqttobj->m_bConnected = true;

            if (pmqttobj->m_bSubscribe) {
                mg_mqtt_subscribe(nc, pmqttobj->m_topic_list, 1, 42);
            }
            break;

        case MG_EV_MQTT_PUBACK:
            break;

        case MG_EV_MQTT_SUBACK:
            break;

        case MG_EV_MQTT_PINGREQ:
            mg_mqtt_pong(nc);
            break;

        // Incoming message
        case MG_EV_MQTT_PUBLISH: {
            vscpEventEx eventEx;

            eventEx.obid      = 0;
            eventEx.timestamp = vscp_makeTimeStamp();
            vscp_setEventExDateTimeBlockToNow(&eventEx);
            eventEx.head = VSCP_PRIORITY_NORMAL;
            memset(eventEx.GUID, 0, 16);

            if (!strncmp(msg->topic.p,
                         (const char *)pmqttobj->m_topic.c_str(),
                         msg->topic.len)) {

                char valbuf[512];
                memset(valbuf, 0, sizeof(valbuf));
                memcpy(valbuf, msg->payload.p, MIN(511, msg->payload.len));
                std::string str = std::string(valbuf);

                if (pmqttobj->m_bSimplify) {

                    // Here the data will only be a value (Simple)

                    eventEx.vscp_class = pmqttobj->m_simple_vscpclass;
                    eventEx.vscp_type  = pmqttobj->m_simple_vscptype;

                    switch (pmqttobj->m_simple_vscpclass) {

                        case VSCP_CLASS2_MEASUREMENT_STR:

                            // Sensor index
                            eventEx.data[0] = pmqttobj->m_simple_sensorindex;

                            // Zone
                            eventEx.data[1] = pmqttobj->m_simple_zone;

                            // Subzone
                            eventEx.data[2] = pmqttobj->m_simple_subzone;

                            // Unit
                            eventEx.data[3] = pmqttobj->m_simple_unit;

                            // Value can have max 7 characters
                            if (str.length() > (VSCP_LEVEL2_MAXDATA - 4)) {
                                str =
                                  vscp_str_right(str, VSCP_LEVEL2_MAXDATA - 4);
                            }

                            // Set size
                            eventEx.sizeData = str.length() + 4;

                            // Copy in string data
                            memcpy(eventEx.data + 4, str.c_str(), str.length());

                            break;

                        case VSCP_CLASS2_MEASUREMENT_FLOAT: {
                            // Sensor index
                            eventEx.data[0] = pmqttobj->m_simple_sensorindex;

                            // Zone
                            eventEx.data[1] = pmqttobj->m_simple_zone;

                            // Subzone
                            eventEx.data[2] = pmqttobj->m_simple_subzone;

                            // Unit
                            eventEx.data[3] = pmqttobj->m_simple_unit;

                            double val;
                            val        = std::stod(str);
                            uint8_t *p = (uint8_t *)&val;

                            if (vscp_isLittleEndian()) {

                                for (int i = 7; i > 0; i--) {
                                    eventEx.data[4 + 7 - i] = *(p + i);
                                }

                            } else {
                                memcpy(eventEx.data + 4, p, 8);
                            }

                            // Set data size
                            eventEx.sizeData = 8 + 4;

                        } break;

                        default:
                        case VSCP_CLASS1_MEASUREMENT:

                            if (VSCP_DATACODING_STRING ==
                                pmqttobj->m_simple_coding) {

                                // * * * Present as string  * * *

                                // Coding
                                eventEx.data[0] =
                                  VSCP_DATACODING_STRING |
                                  ((pmqttobj->m_simple_unit << 3) &
                                   VSCP_MASK_DATACODING_UNIT) |
                                  (pmqttobj->m_simple_sensorindex &
                                   VSCP_MASK_DATACODING_INDEX);

                                // Set string data

                                // Value can have max 7 characters
                                if (str.length() > 7) {
                                    str = vscp_str_right(str, 7);
                                }

                                memcpy(
                                  eventEx.data + 1, str.c_str(), str.length());

                                // Set size
                                eventEx.sizeData = str.length() + 1;

                            } else {

                                // * * * Present as single precision floating
                                // point number * * *
                                float val = atof(str.c_str());

                                val = VSCP_UINT32_SWAP_ON_LE(val);
                                memcpy(eventEx.data + 1, (uint8_t *)&val, 4);

                                eventEx.data[0] =
                                  VSCP_DATACODING_SINGLE |
                                  ((pmqttobj->m_simple_unit << 3) &
                                   VSCP_MASK_DATACODING_UNIT) |
                                  (pmqttobj->m_simple_sensorindex &
                                   VSCP_MASK_DATACODING_INDEX);

                                // Set size
                                eventEx.sizeData = 5; // coding + 32-bit value
                            }
                            break;
                    }

                    goto FEED_EVENT;

                } else {

                    if (vscp_setVscpEventExFromString(&eventEx, str)) {

                    FEED_EVENT:

                        vscpEvent *pEvent = new vscpEvent;
                        if (NULL != pEvent) {

                            pEvent->sizeData = 0;
                            pEvent->pdata    = NULL;

                            if (vscp_doLevel2FilterEx(
                                  &eventEx, &pmqttobj->m_vscpfilter)) {

                                if (vscp_convertVSCPfromEx(pEvent, &eventEx)) {
                                    pthread_mutex_lock(
                                      &pmqttobj->m_mutexReceiveQueue);
                                    pmqttobj->m_receiveList.push_back(pEvent);
                                    sem_post(&pmqttobj->m_semReceiveQueue);
                                    pthread_mutex_unlock(
                                      &pmqttobj->m_mutexReceiveQueue);
                                }

                            } else {
                                vscp_deleteVSCPevent(pEvent);
                            }
                        }
                    }
                }
            }
        } break;

        case MG_EV_CLOSE:

#ifndef WIN32
            syslog(LOG_INFO, "VSCP MQTT Driver - Connection closed.\n");
#endif
            pmqttobj->m_bConnected = false;
            pmqttobj->m_bQuit      = true;
    }
}

//////////////////////////////////////////////////////////////////////
// Cmqttobj
//

Cmqttobj::Cmqttobj()
{
    m_bQuit      = false;
    m_bConnected = false;
    m_bSubscribe = true;

    m_sessionid = "default";

    // Simple
    m_bSimplify          = false;
    m_simple_vscpclass   = VSCP_CLASS1_MEASUREMENT;
    m_simple_vscptype    = 0;
    m_simple_coding      = 0;
    m_simple_unit        = 0;
    m_simple_sensorindex = 0;
    m_simple_zone        = 0;
    m_simple_subzone     = 0;

    m_topic_list[0].qos   = 0;
    m_topic_list[0].topic = NULL,

    vscp_clearVSCPFilter(&m_vscpfilter); // Accept all events

    sem_init(&m_semSendQueue, 0, 0);
    sem_init(&m_semReceiveQueue, 0, 0);

    pthread_mutex_init(&m_mutexSendQueue, NULL);
    pthread_mutex_init(&m_mutexReceiveQueue, NULL);
}

//////////////////////////////////////////////////////////////////////
// ~Cmqttobj
//

Cmqttobj::~Cmqttobj()
{
    close();

    pthread_mutex_destroy(&m_mutexSendQueue);
    pthread_mutex_destroy(&m_mutexReceiveQueue);

    sem_destroy(&m_semSendQueue);
    sem_destroy(&m_semReceiveQueue);
}

//////////////////////////////////////////////////////////////////////
// open
//
//

bool
Cmqttobj::open(const char *pUsername,
               const char *pPassword,
               const char *pHost,
               const char *pPrefix,
               const char *pConfig)
{
    // *** All variables must be dynamic because of multientrance

    std::string str;
    std::string strName;

    m_username = std::string(pUsername);
    m_password = std::string(pPassword);
    m_host     = std::string(pHost);
    m_prefix   = std::string(pPrefix);

    // Parse the configuration string. It should
    // have the following form
    // "sessionid";“subscribe”|”publish”;channel;host:port;user;password;keepalive;filter;mask
    //
    std::deque<std::string> tokens;
    vscp_split(tokens, pConfig, ";");

    // Session id
    if (!tokens.empty()) {
        m_sessionid = tokens.front();
        tokens.pop_front();
    }

    // Check if we should publish or subscribe
    if (!tokens.empty()) {
        // Check for *subscribe*/publish
        str = tokens.front();
        tokens.pop_front();
        vscp_trim(str);
        if (0 == vscp_strcasecmp(str.c_str(), "PUBLISH")) {
            m_bSubscribe = false;
        }
    }

    // Get topic from configuration string
    if (!tokens.empty()) {
        m_topic = tokens.front();
        tokens.pop_front();
    }

    // Get MQTT host from configuration string
    if (!tokens.empty()) {
        m_hostMQTT = tokens.front();
        tokens.pop_front();
    }

    // Get MQTT user from configuration string
    if (!tokens.empty()) {
        m_usernameMQTT = tokens.front();
        tokens.pop_front();
    }

    // Get MQTT password from configuration string
    if (!tokens.empty()) {
        m_passwordMQTT = tokens.front();
        tokens.pop_front();
    }

    // Get MQTT keep alive from configuration string
    if (!tokens.empty()) {
        m_keepalive = vscp_readStringValue(tokens.front());
        tokens.pop_front();
    }

    // First log on to the host and get configuration
    // variables

    if (VSCP_ERROR_SUCCESS != m_srv.doCmdOpen(m_host, m_username, m_password)) {
        syslog(LOG_ERR,
               "Unable to connect to VSCP TCP/IP interface. Terminating!");
        return false;
    }

    // Find the channel id
    uint32_t ChannelID;
    m_srv.doCmdGetChannelID(&ChannelID);

    // The server should hold configuration data for each sensor
    // we want to monitor.
    //
    // We look for
    //
    //   _sessionid - Unique id for MQTT session, e.g. "session2"
    //
    //   _type   - “subscribe” to subscribe to a MQTT topic. ”publish” to
    //              publish events to a MQTT topic. Defaults to “subscribe”.
    //
    //   _topic - This is a text string identifying the topic. It is
    //              recommended that this string starts with “vscp/”.
    //              Defaults to “vscp”
    //
    //   _qos   - MQTT QOS value. Defaults to 0.
    //
    //   _host  - IP address + port or a DNS resolvable address + port on the
    //              form host:port to the remote host.
    //              Mandatory and must be declared either in the configuration
    //              string or in this variable. Defaults to “localhost:1883”
    //
    //   _user - Username used to log in on the remote sever.
    //              Defaults to empty.
    //
    //   _password - Password used to login on the remote server.
    //              Defaults to empty.
    //
    //   _keepalive - Keepalive value for channel. Defaults to 60.
    //
    //   _filter - Standard VSCP filter in string form.
    //                 1,0x0000,0x0006,
    //                 ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00
    //              as priority,class,type,GUID
    //              Used to filter what events that is received from
    //              the mqtt interface. If not give all events
    //              are received.
    //   _mask - Standard VSCP mask in string form.
    //                 1,0x0000,0x0006,
    //                 ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00
    //              as priority,class,type,GUID
    //              Used to filter what events that is received from
    //              the mqtt interface. If not give all events
    //              are received.
    //
    //   _simple - If available simlicty will be enabled which makes it
    //              possible to send just numbers over MQTT but still get
    //              valid events into the system. The functionality works the
    //              other way around to so measurement events can sen data
    //              over MQTT as just a number possibly using the topic as
    //              a way to tell what is sent.

    strName = m_prefix + std::string("_sessionid");
    if (VSCP_ERROR_SUCCESS == m_srv.getRemoteVariableValue(strName, str)) {
        m_sessionid = str;
    }

    strName = m_prefix + std::string("_type");
    m_srv.getRemoteVariableValue(strName, str);

    // Check for subscribe/publish
    vscp_trim(str);
    if (0 == vscp_strcasecmp(str.c_str(), "publish")) {
        m_bSubscribe = false;
    }

    strName = m_prefix + std::string("_topic");
    if (VSCP_ERROR_SUCCESS == m_srv.getRemoteVariableValue(strName, str)) {
        m_topic = str;
    }

    strName = m_prefix + std::string("_host");
    if (VSCP_ERROR_SUCCESS ==
        m_srv.getRemoteVariableValue(strName, m_hostMQTT)) {
        m_hostMQTT = str;
    }

    strName = m_prefix + std::string("_username");
    if (VSCP_ERROR_SUCCESS == m_srv.getRemoteVariableValue(strName, str)) {
        m_usernameMQTT = str;
    }

    strName = m_prefix + std::string("_password");
    if (VSCP_ERROR_SUCCESS ==
        m_srv.getRemoteVariableValue(strName, m_passwordMQTT)) {
        m_passwordMQTT = str;
    }

    strName   = m_prefix + std::string("_keepalive");
    int *pint = new int;
    assert(NULL != pint);
    if (VSCP_ERROR_SUCCESS == m_srv.getRemoteVariableInt(strName, pint)) {
        m_keepalive = *pint;
    }

    strName = m_prefix + std::string("_qos");
    if (VSCP_ERROR_SUCCESS == m_srv.getRemoteVariableInt(strName, pint)) {
        m_topic_list[0].qos = *pint;
    }

    delete pint;

    strName = m_prefix + std::string("_filter");
    if (VSCP_ERROR_SUCCESS == m_srv.getRemoteVariableValue(strName, str)) {
        vscp_readFilterFromString(&m_vscpfilter, str);
    }

    strName = m_prefix + std::string("_mask");
    if (VSCP_ERROR_SUCCESS == m_srv.getRemoteVariableValue(strName, str)) {
        vscp_readMaskFromString(&m_vscpfilter, str);
    }

    // Type = 0
    // ====================================================================
    // Send CLASS1.MEASUREMENT  float or string
    // Coding: 10,float|string,vscp-type,sensorindex(0-7),unit(0-7)
    //
    // Type = 1
    // ====================================================================
    // Send CLASS2.MEASUREMENT.STR
    // Coding: 1040,vscp-type,sensorindex(0-255), unit (0-255), zone(0-255),
    // subzone(0-255)
    //
    // Type = 2
    // ====================================================================
    // Send CLASS2.MEASUREMENT.FLOAT
    // Coding: 1060,vscp-type,sensorindex(0-255), unit (0-255), zone(0-255),
    // subzone(0-255)
    //

    strName = m_prefix + std::string("_simplify");
    if (VSCP_ERROR_SUCCESS == m_srv.getRemoteVariableValue(strName, str)) {
        m_simplify = str;
    }

    if (m_simplify.length()) {

        m_bSimplify = true;
        std::deque<std::string> tokensSimple;
        vscp_split(tokensSimple, m_simplify, ",");

        // simple type
        if (!tokensSimple.empty()) {
            m_simple_vscpclass = vscp_readStringValue(tokensSimple.front());
            tokensSimple.pop_front();
        }

        switch (m_simple_vscpclass) {

            case VSCP_CLASS2_MEASUREMENT_STR:

                // simple vscp-type
                if (!tokensSimple.empty()) {
                    m_simple_vscptype =
                      vscp_readStringValue(tokensSimple.front());
                    tokensSimple.pop_front();
                }

                // simple sensorindex
                if (!tokensSimple.empty()) {
                    m_simple_sensorindex =
                      vscp_readStringValue(tokensSimple.front());
                    tokensSimple.pop_front();
                }

                // simple unit
                if (!tokensSimple.empty()) {
                    m_simple_unit = vscp_readStringValue(tokensSimple.front());
                    tokensSimple.pop_front();
                }

                // simple zone
                if (!tokensSimple.empty()) {
                    m_simple_zone = vscp_readStringValue(tokensSimple.front());
                    tokensSimple.pop_front();
                }

                // simple subzone
                if (!tokensSimple.empty()) {
                    m_simple_subzone =
                      vscp_readStringValue(tokensSimple.front());
                    tokensSimple.pop_front();
                }

                break;

            case VSCP_CLASS2_MEASUREMENT_FLOAT:

                // simple vscp-type
                if (!tokensSimple.empty()) {
                    m_simple_vscptype =
                      vscp_readStringValue(tokensSimple.front());
                    tokensSimple.pop_front();
                }

                // simple sensorindex
                if (!tokensSimple.empty()) {
                    m_simple_sensorindex =
                      vscp_readStringValue(tokensSimple.front());
                    tokensSimple.pop_front();
                }

                // simple unit
                if (!tokensSimple.empty()) {
                    m_simple_unit = vscp_readStringValue(tokensSimple.front());
                    tokensSimple.pop_front();
                }

                // simple zone
                if (!tokensSimple.empty()) {
                    m_simple_zone = vscp_readStringValue(tokensSimple.front());
                    tokensSimple.pop_front();
                }

                // simple subzone
                if (!tokensSimple.empty()) {
                    m_simple_subzone =
                      vscp_readStringValue(tokensSimple.front());
                    tokensSimple.pop_front();
                }

                break;

            case VSCP_CLASS1_MEASUREMENT:
            default:

                // simple vscp-type
                if (!tokensSimple.empty()) {
                    m_simple_vscptype =
                      vscp_readStringValue(tokensSimple.front());
                    tokensSimple.pop_front();
                }

                // Coding
                if (!tokensSimple.empty()) {
                    std::string strcoding = tokensSimple.front();
                    tokensSimple.pop_front();
                    vscp_trim(strcoding);
                    m_simple_coding = VSCP_DATACODING_STRING;
                    if (0 == vscp_strcasecmp(str.c_str(), "FLOAT")) {
                        m_simple_coding = VSCP_DATACODING_SINGLE;
                    }
                }

                // simple sensorindex
                if (!tokensSimple.empty()) {
                    m_simple_sensorindex =
                      vscp_readStringValue(tokensSimple.front());
                    tokensSimple.pop_front();
                }
                if (m_simple_sensorindex > 7) m_simple_sensorindex = 7;

                // simple unit
                if (!tokensSimple.empty()) {
                    m_simple_unit = vscp_readStringValue(tokensSimple.front());
                    tokensSimple.pop_front();
                }
                if (m_simple_unit > 7) m_simple_unit = 7;

                break;
        }

    } else {
        m_bSimplify = false;
    }

    if (m_bSubscribe) {
        // QOS set from variable read or constructor
        m_topic_list[0].topic = new char(m_topic.length() + 1);
        assert(NULL != m_topic_list[0].topic);
        if (NULL != m_topic_list[0].topic) {
            memset((void *)m_topic_list[0].topic, 0, m_topic.length() + 1);
            memcpy(
              (void *)m_topic_list[0].topic, m_topic.c_str(), m_topic.length());
        }
    }

    // Close the channel
    m_srv.doCmdClose();

    // start the worker thread
    m_pWrkObj = new CWrkThreadObj();
    if (NULL != m_pWrkObj) {
        m_pWrkObj->m_pObj = this;
        if (pthread_create(m_threadWork, NULL, workerTread, this)) {

            syslog(LOG_CRIT, "Unable to start worker thread.");
            return false;
        }
    } else {
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// close
//

void
Cmqttobj::close(void)
{
    // Do nothing if already terminated
    if (m_bQuit) return;

    m_bQuit = true; // terminate the thread
    sleep(1);       // Give the thread some time to terminate
}

//////////////////////////////////////////////////////////////////////
// addEvent2SendQueue
//

bool
Cmqttobj::addEvent2SendQueue(const vscpEvent *pEvent)
{
    pthread_mutex_lock(&m_mutexSendQueue);
    m_sendList.push_back((vscpEvent *)pEvent);
    sem_post(&m_semSendQueue);
    pthread_mutex_unlock(&m_mutexSendQueue);
    return true;
}

//////////////////////////////////////////////////////////////////////
//                Workerthread - CWrkThreadObj
//////////////////////////////////////////////////////////////////////

CWrkThreadObj::CWrkThreadObj()
{
    m_pObj = NULL;
}

CWrkThreadObj::~CWrkThreadObj()
{
    ;
}

//////////////////////////////////////////////////////////////////////
// Workerthread
//

void *
workerTread(void *pData)
{
    int cnt_poll = 0;
    std::string str;
    struct mg_connection *nc;
    struct mg_mgr *pmgr = new mg_mgr;
    assert(NULL != pmgr);
    uint16_t msgid = 0;

    if ( NULL == pData ) {
        syslog(LOG_CRIT, "Missing thread object!");
        return NULL;
    }    

    Cmqttobj *pObj = (Cmqttobj *)pData;

    // mgr.user_data = m_pObj;
    mg_mgr_init(pmgr, pObj);

    if (NULL == (nc = mg_connect(pmgr,
                                 (const char *)pObj->m_hostMQTT.c_str(),
                                 ev_handler))) {
#ifdef DEBUG
        fprintf(stderr,
                "ns_connect(%s) failed\n",
                (const char *)m_pObj->m_hostMQTT.c_str());
#endif
#ifndef WIN32
        syslog(LOG_INFO, "VSCP MQTTT Driver - ns_connet failed\n");
#endif
        return NULL;
    }

    if (pObj->m_bSubscribe) {

        while (!pObj->m_bQuit) {

            mg_mgr_poll(pmgr, 100);
            cnt_poll++;

            // Keep the connection alive
            if (cnt_poll > 600) {
                mg_mqtt_pong(nc);
                cnt_poll = 0;
            }

            if (pObj->m_bConnected) {

            } else {

                // Give system time to connect
                // usleep( 1000000 );

                // Try to connect again
                // nc = ns_connect( pmgr,
                //                    (const char
                //                    *)pObj->m_hostMQTT.c_str(), ev_handler
                //                    );

                // ns_mqtt_subscribe( nc, pObj->m_topic_list, 1, 42 );
            }
        }

    }
    // Publish
    else {

        while (!pObj->m_bQuit) {

            mg_mgr_poll(pmgr, 100);

            // Wait for connection
            if (!pObj->m_bConnected) {
                continue;
            }

            struct timespec ts;
            ts.tv_sec  = 0;
            ts.tv_nsec = 10000; // 10 ms
            if (ETIMEDOUT == sem_timedwait(&pObj->m_semSendQueue, &ts)) {
                continue;
            }

            if (pObj->m_sendList.size()) {

                pthread_mutex_lock(&pObj->m_mutexSendQueue);
                vscpEvent *pEvent = pObj->m_sendList.front();
                pObj->m_sendList.pop_front();
                pthread_mutex_unlock(&pObj->m_mutexSendQueue);
                if (NULL == pEvent) continue;

                // If simple there must also be data
                if (pObj->m_bSimplify && (NULL != pEvent->pdata)) {

                    switch (pObj->m_simple_vscpclass) {

                        case VSCP_CLASS2_MEASUREMENT_STR: {
                            if (VSCP_CLASS2_MEASUREMENT_STR !=
                                pEvent->vscp_class) {
                                break;
                            }

                            // There must be at least one
                            // character in the string
                            if (pEvent->sizeData < 5) {
                                break;
                            }

                            // Sensor index must be the same
                            if (pObj->m_simple_sensorindex !=
                                pEvent->pdata[0]) {
                                break;
                            }

                            // Zone must be the same
                            if (pObj->m_simple_zone != pEvent->pdata[1]) {
                                break;
                            }

                            // Subzone must be the same
                            if (pObj->m_simple_zone != pEvent->pdata[2]) {
                                break;
                            }

                            // Unit must be the same
                            if (pObj->m_simple_unit != pEvent->pdata[3]) {
                                break;
                            }

                            char buf[512];
                            memset(buf, 0, sizeof(buf));
                            memcpy(
                              buf, pEvent->pdata + 4, pEvent->sizeData - 4);
                            str = std::string(buf);

                            goto PUBLISH;
                        } break;

                        case VSCP_CLASS2_MEASUREMENT_FLOAT: {
                            // There must be place for the
                            // double in the data 4 + 8
                            if (pEvent->sizeData < 12) {
                                break;
                            }

                            // Sensor index must be the same
                            if (pObj->m_simple_sensorindex !=
                                pEvent->pdata[0]) {
                                break;
                            }

                            // Zone must be the same
                            if (pObj->m_simple_zone != pEvent->pdata[1]) {
                                break;
                            }

                            // Subzone must be the same
                            if (pObj->m_simple_zone != pEvent->pdata[2]) {
                                break;
                            }

                            // Unit must be the same
                            if (pObj->m_simple_unit != pEvent->pdata[3]) {
                                break;
                            }

                            uint8_t *p = pEvent->pdata + 4;
                            if (vscp_isLittleEndian()) {
                                for (int i = 7; i > 0; i--) {
                                    pEvent->pdata[4 + 7 - i] = *(p + i);
                                }
                            }

                            double val = *((double *)(pEvent->pdata + 4));
                            char buf[80];
                            sprintf(buf, "%g", val);
                            str = std::string(buf);

                            goto PUBLISH;

                        } break;

                        default:
                        case VSCP_CLASS1_MEASUREMENT: {
                            // Sensor index must be the same
                            if (pObj->m_simple_sensorindex !=
                                VSCP_DATACODING_INDEX(pEvent->pdata[0])) {
                                break;
                            }

                            // Unit must be the same
                            if (pObj->m_simple_unit !=
                                VSCP_DATACODING_UNIT(pEvent->pdata[0])) {
                                break;
                            }

                            // vscp_getVSCPMeasurementAsString( pEvent, str );
                            vscp_writeVscpEventToString(pEvent, str);
                            goto PUBLISH;

                            /*
                                if ( VSCP_DATACODING_SINGLE ==
                                    ( pEvent->pdata[ 0 ] &
                               VSCP_MASK_DATACODING_TYPE ) ) {

                                    uint32_t temp = wxUINT32_SWAP_ON_LE( *(
                               (uint32_t *)( pEvent->pdata + 1 ) ) ); memcpy(
                               pEvent->pdata + 1, (void *)&temp, 4 ); float val
                               = *( (float *)( pEvent->pdata + 1 ) ); char
                               buf[80]; sprintf( buf, "%f", val ); str =
                               std::string( buf );

                                    goto PUBLISH;
                                }
                                else {  // STRING

                                    char buf[ 10 ];
                                    memset( buf, 0, sizeof( buf ) );
                                    if ( pEvent->sizeData >= 8 ) {
                                        memcpy( buf, pEvent->pdata + 1, 7 );
                                    }
                                    else {
                                        memcpy( buf, pEvent->pdata + 1,
                               pEvent->sizeData - 1 );
                                    }
                                    int ttt = pEvent->pdata[1];
                                    ttt = pEvent->pdata[2];
                                    ttt = pEvent->pdata[3];
                                    str = std::string( buf );
                                    goto PUBLISH;

                                }
                            */

                        } break;

                    } // switch

                } else {

                    vscp_writeVscpEventToString(pEvent, str);

PUBLISH:

                    mg_mqtt_publish(nc,
                                    pObj->m_topic.c_str(),
                                    msgid++,
                                    MG_MQTT_QOS(pObj->m_topic_list[0].qos),
                                    (const char *)str.c_str(),
                                    str.length());
                }

                // We are done with the event - remove data if any
                if (NULL != pEvent->pdata) {
                    delete[] pEvent->pdata;
                    pEvent->pdata = NULL;
                }

            } // Event received

            usleep(50000);
        }
    }

    // Disconnect if we are connected.
    if (pObj->m_bConnected) {
        mg_mqtt_disconnect(nc);
    }

    delete pmgr;
    pmgr = NULL;

    return NULL;
}


