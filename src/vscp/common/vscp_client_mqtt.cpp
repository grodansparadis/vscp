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

// Forward declaration
static void*
workerThread(void* pObj);

// ----------------------------------------------------------------------------

// * * * * Callbacks * * * *


// void on_disconnect(struct mosquitto *mosq, void *pData, int rv)
void
on_disconnect(void* pContext, char* pcause)
{
    // Check pointers
    if (NULL == pContext) return;
    if (NULL == pcause) return;

    vscpClientMqtt* pObj = (vscpClientMqtt*)pContext;
    pObj->m_bConnected   = false;
};

// ----------------------------------------------------------------------------

// void on_publish(struct mosquitto *mosq, void *pData, int rv)
void
on_publish(void* pContext, MQTTClient_deliveryToken dt)
{
    // Check pointers
    if (NULL == pContext) return;

    vscpClientMqtt* pObj = (vscpClientMqtt*)pContext;

    // Currently we do nothing
};

// ----------------------------------------------------------------------------

// void on_message(struct mosquitto *mosq, void *pData, const struct
// mosquitto_message *pMsg)
int
on_message(void* pContext, char* pTopic, int topicLen, MQTTClient_message* pmsg)
{
    enumMqttMsgFormat format;
    vscpEvent ev;
    vscpEventEx ex;

    // Check pointers
    if (NULL == pContext) return 0;
    if (NULL == pTopic) return 0;
    if (NULL == pmsg) return 0;

    vscpClientMqtt* pobj = (vscpClientMqtt*)pContext;

    // Should be a payload
    if (!pmsg->payloadlen) return 0;

    if (autofmt == pobj->m_format) {

        // If First char of payload...
        //     ...is zero - Binary payload
        //     ...is "{"  - JSON payload
        //     ...is "<"  - XML payload
        //     else string payload
        if (0 == ((uint8_t *)pmsg->payload)[0]) {
            // Binary
            format = binfmt;
        }
        else if ('{' == ((char *)pmsg->payload)[0]) {
            // JSON
            format = jsonfmt;
        }
        else if ('<' == ((char *)pmsg->payload)[0]) {
            // XML
            format = xmlfmt;
        }
        else {
            // String
            format = strfmt;
        }
    }
    else {
        format = pobj->m_format;
    }

    
    std::string payload((const char*)pmsg->payload, pmsg->payloadlen);

    
    if (jsonfmt == format) {

        if (!vscp_convertJSONToEvent(&ev, payload)) {
            return 0;
        }

        if (!vscp_convertEventToEventEx(&ex, &ev)) {
            return 0;
        }

        // If callback is defined send event
        if (NULL != pobj->m_evcallback) {
            pobj->m_evcallback(&ev, pobj->m_callbackObject);
        }
        else if (NULL != pobj->m_excallback) {
            pobj->m_excallback(&ex, pobj->m_callbackObject);
        }
        else {
            // Put event in input queue
            vscpEvent* pEvent = new vscpEvent;
            if (NULL == pEvent) return 0;
            pEvent->pdata = NULL;

            if (!vscp_copyEvent(pEvent, &ev)) {
                delete pEvent;
                return 0;
            }

            // Save event in incoming queue
            if (pobj->m_receiveQueue.size() < MQTT_MAX_INQUEUE_SIZE) {
                pobj->m_receiveQueue.push_back(pEvent);
            }
        }
    }
    else if (xmlfmt == format) {
        if (!vscp_convertXMLToEvent(&ev, payload)) {
            return 0;
        }

        if (!vscp_convertEventToEventEx(&ex, &ev)) {
            return 0;
        }

        // If callback is defined send event
        if (NULL != pobj->m_evcallback) {
            pobj->m_evcallback(&ev, pobj->m_callbackObject);
        }
        else if (NULL != pobj->m_excallback) {
            pobj->m_excallback(&ex, pobj->m_callbackObject);
        }
        else {
            // Put event in input queue
            vscpEvent* pEvent = new vscpEvent;
            if (NULL == pEvent) return 0;
            pEvent->pdata = NULL;

            if (!vscp_copyEvent(pEvent, &ev)) {
                delete pEvent;
                return 0;
            }

            // Save event in incoming queue
            if (pobj->m_receiveQueue.size() < MQTT_MAX_INQUEUE_SIZE) {
                pobj->m_receiveQueue.push_back(pEvent);
            }
        }
    }
    else if (strfmt == format) {
        if (!vscp_convertStringToEvent(&ev, payload)) {
            return 0;
        }

        if (!vscp_convertEventToEventEx(&ex, &ev)) {
            return 0;
        }

        // If callback is defined send event
        if (NULL != pobj->m_evcallback) {
            pobj->m_evcallback(&ev, pobj->m_callbackObject);
        }
        else if (NULL != pobj->m_excallback) {
            pobj->m_excallback(&ex, pobj->m_callbackObject);
        }
        else {
            // Put event in input queue
            vscpEvent* pEvent = new vscpEvent;
            if (NULL == pEvent) return 0;
            pEvent->pdata = NULL;

            if (!vscp_copyEvent(pEvent, &ev)) {
                delete pEvent;
                return 0;
            }

            // Save event in incoming queue
            if (pobj->m_receiveQueue.size() < MQTT_MAX_INQUEUE_SIZE) {
                pobj->m_receiveQueue.push_back(pEvent);
            }
        }
    }
    else if (binfmt == format) {

        // Binary frame starts offset one in payload (after zero marker byte)
        if (!vscp_getEventFromFrame( &ev,
                                        (const uint8_t*)pmsg->payload + 1,
                                        pmsg->payloadlen)) {
            return 0;
        }

        if (!vscp_convertEventToEventEx(&ex, &ev)) {
            return 0;
        }

        // If callback is defined send event
        if (NULL != pobj->m_evcallback) {
            pobj->m_evcallback(&ev, pobj->m_callbackObject);
        }
        else if (NULL != pobj->m_excallback) {
            pobj->m_excallback(&ex, pobj->m_callbackObject);
        }
        else {
            // Put event in input queue
            vscpEvent* pEvent = new vscpEvent;
            if (NULL == pEvent) return 0;
            pEvent->pdata = NULL;

            if (!vscp_copyEvent(pEvent, &ev)) {
                delete pEvent;
                return 0;
            }

            // Save event in incoming queue
            if (pobj->m_receiveQueue.size() < MQTT_MAX_INQUEUE_SIZE) {
                pobj->m_receiveQueue.push_back(pEvent);
            }
        }
    }
    
    MQTTClient_freeMessage(&pmsg);
    MQTTClient_free(pTopic);

    return 1;
};

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// CTor
//

publishTopic::publishTopic(const std::string& topic, int qos, bool bretain)
{
    m_topic   = topic;
    m_qos     = qos;
    m_bRetain = bretain;
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

publishTopic::~publishTopic()
{
    ;
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// CTor
//

subscribeTopic::subscribeTopic(const std::string& topic, int qos)
{
    m_topic = topic;
    m_qos   = qos;
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

subscribeTopic::~subscribeTopic()
{
    ;
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// C-tor
//

vscpClientMqtt::vscpClientMqtt()
{
    m_pahoClient = NULL;
    m_type = CVscpClient::connType::MQTT;
    m_format = jsonfmt;
    // m_mosq = NULL;
    m_bConnected = false; // Not connected
    //m_tid        = 0;
    m_bRun       = false;
    strcpy(m_host, "tcp://localhost:1883");
    memset(m_clientid, 0, sizeof(m_clientid));
    memset(m_username, 0, sizeof(m_username));
    memset(m_password, 0, sizeof(m_password));
    m_keepalive     = 30;
    m_bTLS          = false;
    m_bCleanSession = false;

    // MQTTClient_init_options init = MQTTClient_init_options_initializer;
    // init.do_openssl_init = 0;
    // MQTTClient_global_init(&init);

    pthread_mutex_init(&m_mutexif, NULL);
}

///////////////////////////////////////////////////////////////////////////////
// D-tor
//

vscpClientMqtt::~vscpClientMqtt()
{
    disconnect();
    // mosquitto_lib_cleanup();
    MQTTClient_destroy(&m_pahoClient);
    pthread_mutex_destroy(&m_mutexif);

    vscpEvent* pev = NULL;
    while (m_receiveQueue.size()) {
        pev = m_receiveQueue.front();
        m_receiveQueue.pop_front();
        vscp_deleteEvent_v2(&pev);
    }
}

///////////////////////////////////////////////////////////////////////////////
// getConfigAsJson
//

std::string
vscpClientMqtt::getConfigAsJson(void)
{
    std::string rv;

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// initFromJson
//

bool
vscpClientMqtt::initFromJson(const std::string& config)
{
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// init
//

int
vscpClientMqtt::init(const std::string& strHost,
                     const std::string& clientId,
                     const std::string& strUserName,
                     const std::string& strPassword,
                     int keepAliveInterval,
                     bool bCleanSession)
{
    //int rv;
    strncpy(m_host, strHost.c_str(), sizeof(m_host));             // MQTT broker
    strncpy(m_clientid, clientId.c_str(), sizeof(m_clientid));    // Client id
    strncpy(m_username, strUserName.c_str(), sizeof(m_username)); // Username
    strncpy(m_password, strPassword.c_str(), sizeof(m_password)); // Password 
    m_keepalive     = keepAliveInterval; // Save keep alive setting
    m_bCleanSession = bCleanSession;     // Clean session on disconnect if true

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// addSubscription
//

int
vscpClientMqtt::addSubscription(const std::string strTopicSub)
{
    m_listTopicSub.push_back(strTopicSub);
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// addPublish
//

int
vscpClientMqtt::addPublish(const std::string strTopicPub, int qos, bool bRetain)
{
    m_listTopicPub.push_back(strTopicPub);
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setRetain
//

int
vscpClientMqtt::setRetain(bool bRetain)
{
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setKeepAlive
//

int
vscpClientMqtt::setKeepAlive(int keepAlive)
{
    return VSCP_ERROR_SUCCESS;
}

int
vscpClientMqtt::set_tls(const std::string& cafile,
                        const std::string& capath,
                        const std::string& certfile,
                        const std::string& keyfile,
                        const std::string& password)
{
    m_cafile    = cafile;
    m_capath    = capath;
    m_certfile  = certfile;
    m_keyfile   = keyfile;
    m_pwKeyfile = password;
    m_bTLS      = true;

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// connect
//

int
vscpClientMqtt::connect(void)
{
    int rv;
    //int ch;

    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    conn_opts.keepAliveInterval         = m_keepalive;
    // Always a clean session if clientid is not set.
    conn_opts.cleansession = (m_bCleanSession || !strlen(m_clientid)) ? 1 : 0;

    // Set username if defined
    if (strlen(m_username)) {
        conn_opts.username = m_username;
    }

    // Set password if defined
    if (strlen(m_password)) {
        conn_opts.password = m_password;
    }

    if (MQTTCLIENT_SUCCESS !=
        (rv = MQTTClient_create(&m_pahoClient,
                                m_host,
                                m_clientid,
                                MQTTCLIENT_PERSISTENCE_NONE,
                                NULL))) {
        m_pahoClient = NULL;                                    
        return rv;
    }

    if (MQTTCLIENT_SUCCESS != (rv = MQTTClient_setCallbacks(m_pahoClient,
                                                            this,
                                                            on_disconnect,
                                                            on_message,
                                                            on_publish))) {
        MQTTClient_destroy(&m_pahoClient);
        m_pahoClient = NULL;
        return rv;
    }

    MQTTClient_nameValue* pver = MQTTClient_getVersionInfo();
    while (NULL != pver->name) {
        fprintf(stderr, "%s - %s\n", pver->name, pver->value);
        pver++;
    }

    if (MQTTCLIENT_SUCCESS != (rv = MQTTClient_connect(m_pahoClient, &conn_opts))) {
        MQTTClient_destroy(&m_pahoClient);    
        m_pahoClient = NULL;    
        return VSCP_ERROR_CONNECTION;
    }

    // Only subscribe if subscription topic is defined
    for (std::list<subscribeTopic>::const_iterator it = m_listTopicSub.begin();
         it != m_listTopicSub.end();
         ++it) {

        subscribeTopic topic = (*it);

        // Fix subscribe topics
        mustache subtemplate{ topic.getTopic() };
        data data;
        // data.set("guid", m_guid.getAsString());
        std::string subscribe_topic = subtemplate.render(data);

        if (MQTTCLIENT_SUCCESS !=
            (rv = MQTTClient_subscribe(m_pahoClient,
                                       subscribe_topic.c_str(),
                                       topic.getQos()))) {
            MQTTClient_disconnect(m_pahoClient, 10000);
            MQTTClient_destroy(&m_pahoClient);    
            m_pahoClient = NULL;    
            return VSCP_ERROR_CONNECTION;
        }
    }

    // Start worker thread if a callback has been defined
    if ((NULL != m_evcallback) || (NULL != m_excallback)) {
        int rv = pthread_create(&m_tid, NULL, workerThread, this);
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

    pthread_join(m_tid, NULL);

    if ((NULL != m_pahoClient) && MQTTClient_isConnected(m_pahoClient)) {
        if (MQTTCLIENT_SUCCESS !=
            (rv = MQTTClient_disconnect(m_pahoClient, 5000))) {
            return rv;
        }
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// isConnected
//

bool
vscpClientMqtt::isConnected(void)
{
    if (NULL == m_pahoClient) return false;    
    return MQTTClient_isConnected(m_pahoClient);
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int
vscpClientMqtt::send(vscpEvent& ev)
{
    uint8_t payload[1024];
    size_t lenPayload = 0;
    int rv;

    // Only subscribe if subscription topic is defined
    for (std::list<publishTopic>::const_iterator it = m_listTopicPub.begin();
         it != m_listTopicPub.end();
         ++it) {

        publishTopic publish = (*it);

        memset(payload, 0, sizeof(payload));

        if (m_format == jsonfmt) {
            std::string strPayload;
            if (!vscp_convertEventToJSON(strPayload, &ev)) {
                return VSCP_ERROR_PARAMETER;
            }
            lenPayload = strPayload.length();
            strncpy((char *)payload, strPayload.c_str(), sizeof(payload));
        }
        else if (m_format == xmlfmt) {
            std::string strPayload;
            if (!vscp_convertEventToXML(strPayload, &ev)) {
                return VSCP_ERROR_PARAMETER;
            }
            lenPayload = strPayload.length();
            strncpy((char *)payload, strPayload.c_str(), sizeof(payload));
        }
        else if (m_format == strfmt) {
            std::string strPayload;
            if (!vscp_convertEventToString(strPayload, &ev)) {
            return VSCP_ERROR_PARAMETER; 
            }
            lenPayload = strPayload.length();
            strncpy((char *)payload, strPayload.c_str(), sizeof(payload));
        }
        else if (m_format == binfmt) {
            
            lenPayload = vscp_getFrameSizeFromEvent(&ev) + 1;

            // Write event to frame
            if (!vscp_writeEventToFrame( payload + 1,
                                            sizeof(payload),
                                            0,
                                            &ev )) {
                return VSCP_ERROR_PARAMETER;                                
            }

        }
        else {
            return VSCP_ERROR_NOT_SUPPORTED;
        }

        if (MQTTCLIENT_SUCCESS != 
                (rv = MQTTClient_publish(m_pahoClient,
                                            publish.getTopic().c_str(),
                                            (int)lenPayload,
                                            payload,
                                            publish.getQos(),
                                            publish.getRetain(),
                                            NULL))) {
            return VSCP_ERROR_COMMUNICATION;
        }

    } // for each topic

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int
vscpClientMqtt::send(vscpEventEx& ex)
{
    uint8_t payload[1024];
    size_t lenPayload = 0;
    int rv;

    // Only subscribe if subscription topic is defined
    for (std::list<publishTopic>::const_iterator it = m_listTopicPub.begin();
         it != m_listTopicPub.end();
         ++it) {

        publishTopic publish = (*it);

        memset(payload, 0, sizeof(payload));

        if (m_format == jsonfmt) {
            std::string strPayload;
            if (!vscp_convertEventExToJSON(strPayload, &ex)) {
                return VSCP_ERROR_PARAMETER;
            }
            lenPayload = strPayload.length();
            strncpy((char *)payload, strPayload.c_str(), sizeof(payload));
        }
        else if (m_format == xmlfmt) {
            std::string strPayload;
            if (!vscp_convertEventExToXML(strPayload, &ex)) {
                return VSCP_ERROR_PARAMETER;
            }
            lenPayload = strPayload.length();
            strncpy((char *)payload, strPayload.c_str(), sizeof(payload));
        }
        else if (m_format == strfmt) {
            std::string strPayload;
            if (!vscp_convertEventExToString(strPayload, &ex)) {
            return VSCP_ERROR_PARAMETER; 
            }
            lenPayload = strPayload.length();
            strncpy((char *)payload, strPayload.c_str(), sizeof(payload));
        }
        else if (m_format == binfmt) {
            
            lenPayload = vscp_getFrameSizeFromEventEx(&ex) + 1;

            // Write event to frame
            if (!vscp_writeEventExToFrame( payload + 1,
                                            sizeof(payload),
                                            0,
                                            &ex )) {
                return VSCP_ERROR_PARAMETER;                                
            }

        }
        else {
            return VSCP_ERROR_NOT_SUPPORTED;
        }

        if (MQTTCLIENT_SUCCESS != 
                (rv = MQTTClient_publish(m_pahoClient,
                                            publish.getTopic().c_str(),
                                            (int)lenPayload,
                                            payload,
                                            publish.getQos(),
                                            publish.getRetain(),
                                            NULL))) {
            return VSCP_ERROR_COMMUNICATION;
        }

    } // for each topic

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int
vscpClientMqtt::receive(vscpEvent& ev)
{
    //int rv;
    vscpEvent* pev = NULL;

    if (m_receiveQueue.size()) {

        pev = m_receiveQueue.front();
        m_receiveQueue.pop_front();
        if (NULL == pev)
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
vscpClientMqtt::receive(vscpEventEx& ex)
{
    //int rv;
    vscpEvent* pev = NULL;

    if (m_receiveQueue.size()) {

        pev = m_receiveQueue.front();
        m_receiveQueue.pop_front();
        if (NULL == pev)
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
vscpClientMqtt::setfilter(vscpEventFilter& filter)
{
    // Copy in filter
    memcpy(&m_filter, &filter, sizeof(vscpEventFilter));
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getcount
//

int
vscpClientMqtt::getcount(uint16_t* pcount)
{
    if (NULL == pcount) return VSCP_ERROR_INVALID_POINTER;
    *pcount = (uint16_t)m_receiveQueue.size();
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
vscpClientMqtt::getversion(uint8_t* pmajor,
                           uint8_t* pminor,
                           uint8_t* prelease,
                           uint8_t* pbuild)
{
    if (NULL == pmajor)
        return VSCP_ERROR_INVALID_POINTER;
    if (NULL == pminor)
        return VSCP_ERROR_INVALID_POINTER;
    if (NULL == prelease)
        return VSCP_ERROR_INVALID_POINTER;
    if (NULL == pbuild)
        return VSCP_ERROR_INVALID_POINTER;

    // "Version - 1.3.8"
    MQTTClient_nameValue* pver = MQTTClient_getVersionInfo();
    while (NULL != pver->name) {
        fprintf(stderr, "%s - %s\n", pver->name, pver->value);
        if (NULL != strstr(pver->name, "Version")) {
            int v1, v2, v3;
            sscanf(pver->value, "%d.%d.%d", &v1, &v2, &v3);
            *pmajor   = v1;
            *pminor   = v2;
            *prelease = v3;
            *pbuild   = 0;
            break;
        }
        pver++;
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getinterfaces
//

int
vscpClientMqtt::getinterfaces(std::deque<std::string>& iflist)
{
    // No interfaces available
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getwcyd
//

int
vscpClientMqtt::getwcyd(uint64_t& wcyd)
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

    ft.QuadPart = -(10*usec); // Convert to 100 nanosecond interval, negative value indicates relative time

    timer = CreateWaitableTimer(NULL, TRUE, NULL); 
    SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0); 
    WaitForSingleObject(timer, INFINITE); 
    CloseHandle(timer); 
}
#endif

///////////////////////////////////////////////////////////////////////////////
// Callback workerthread
//
// This thread call the appropriate callback when events are received
//

static void*
workerThread(void* pObj)
{
    uint8_t guid[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    vscpClientMqtt* pClient = (vscpClientMqtt*)pObj;
    //  if (NULL == pif) return NULL;

    while (pClient->m_bRun) {

        //  pthread_mutex_lock(&pif->m_mutexif);

        // Check if there are events to fetch
        //int cnt;
        /* if ((cnt = pClient->m_canalif.CanalDataAvailable())) {

            while (cnt) {
                canalMsg msg;
                if ( CANAL_ERROR_SUCCESS ==
        pClient->m_canalif.CanalReceive(&msg) ) { if ( NULL !=
        pClient->m_evcallback ) { vscpEvent ev; if
        (vscp_convertCanalToEvent(&ev, &msg, guid) ) {
                            pClient->m_evcallback(ev);
                        }
                    }
                    if ( NULL != pClient->m_excallback ) {
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

    return NULL;
}
