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

#include <unistd.h>
#include <pthread.h> 

#include "vscphelper.h"
#include "vscp_client_mqtt.h"

// Forward declaration
void *workerThread(void *pObj);

// * * * * Callbacks * * * *

void on_connect(struct mosquitto *mosq, void *pData, int rv) 
{
    // Check pointers
    if (NULL == mosq) return;
    if (NULL == pData) return;

    vscpClientMqtt *pObj = (vscpClientMqtt *)pData;
    pObj->m_bConnected = true;
};

void on_disconnect(struct mosquitto *mosq, void *pData, int rv) 
{
    // Check pointers
    if (NULL == mosq) return;
    if (NULL == pData) return;

    vscpClientMqtt *pObj = (vscpClientMqtt *)pData;
    pObj->m_bConnected = false;
};

void on_publish(struct mosquitto *mosq, void *pData, int rv) 
{
    // Check pointers
    if (NULL == mosq) return;
    if (NULL == pData) return;

    vscpClientMqtt *pObj = (vscpClientMqtt *)pData;
    // Currently do nothing
};

void on_message(struct mosquitto *mosq, void *pData, const struct mosquitto_message *pMsg) 
{
    // Check pointers
    if (NULL == mosq) return;
    if (NULL == pData) return;
    if (NULL == pMsg) return;

    vscpClientMqtt *pObj = (vscpClientMqtt *)pData;
    std::string payload((const char *)pMsg->payload, pMsg->payloadlen);
    
    if ( jsonfmt == pObj->m_format ) {
        
        vscpEvent ev;
        vscpEventEx ex;

        if ( !vscp_convertEventToEventEx(&ex, &ev) ) {
            return;
        }

        if ( !vscp_convertJSONToEvent(&ev, payload) ) {
            return;
        }

        // If callback is defined send event
        if ( NULL != pObj->m_evcallback )  {
            pObj->m_evcallback(ev);
        }
        else if ( NULL != pObj->m_excallback ) {
            pObj->m_excallback(ex);
        }
        else {
            // Put event in input queue
            vscpEvent *pEvent = new vscpEvent;
            if (NULL == pEvent) return;
            pEvent->pdata = NULL;

            if (!vscp_copyEvent(pEvent, &ev)) {
                delete pEvent;
                return;
            }

            // Save event in incoming queue
            if (pObj->m_receiveQueue.size() < MQTT_MAX_INQUEUE_SIZE ) {
                pObj->m_receiveQueue.push_back(pEvent);
            }
        }

    }
    else if ( xmlfmt == pObj->m_format ) {

    }
    else {
        return;
    }
};

void on_log(struct mosquitto *mosq, void *pData, int level, const char *logmsg) 
{
    // Check pointers
    if (NULL == mosq) return;
    if (NULL == pData) return;

    vscpClientMqtt *pObj = (vscpClientMqtt *)pData;

}


///////////////////////////////////////////////////////////////////////////////
// C-tor
//

vscpClientMqtt::vscpClientMqtt() 
{
    m_format == jsonfmt;
    m_mosq = NULL;
    m_bConnected = false;  // Not connected
    m_tid = 0;
    m_bRun = false;
    m_strHost = "localhost";
    m_strTopicSub = "vscp/#";
    m_strTopicPub = "vscp/%guid%/%class%/%type%/";
    m_clientId = "";
    m_strUserName = "";
    m_strPassword = "";
    m_qos = 0;
    m_bRetain = false;
    m_keepalive = 30;
    m_bTLS = false; 
    m_bCleanSession = false;

    pthread_mutex_init(&m_mutexif,NULL);
    
}

///////////////////////////////////////////////////////////////////////////////
// D-tor
//

vscpClientMqtt::~vscpClientMqtt() 
{
    disconnect();
    mosquitto_lib_cleanup();     
    pthread_mutex_destroy(&m_mutexif);

    vscpEvent *pev = NULL;
    while ( m_receiveQueue.size()) {
        pev = m_receiveQueue.front();
        m_receiveQueue.pop_front();
        vscp_deleteEvent_v2(&pev);
    }
}

///////////////////////////////////////////////////////////////////////////////
// init
//

int vscpClientMqtt::init(const std::string &strHost,
                            short port,                
                            const std::string &strTopicSub,
                            const std::string &strTopicPub,
                            const std::string &clientId,                
                            const std::string &strUserName,
                            const std::string &strPassword,
                            bool bCleanSession,
                            int qos)
{
    m_strHost = strHost;                // MQTT broker
    m_port = port;                      // MQTT broker port
    m_strTopicSub = strTopicSub;        // Subscribe topic template
    m_strTopicPub = strTopicPub;        // Publish topic template
    m_clientId = clientId;              // Client id
    m_strUserName = strUserName;        // Username
    m_strPassword = strPassword;        // Password
    m_qos = qos;                        // Quality of service
    m_bCleanSession = bCleanSession;    // Clean session on disconnect if true

    mosquitto_lib_init();

    int rv;
    if (m_clientId.length()) {
        m_mosq = mosquitto_new(m_clientId.c_str(), m_bCleanSession, this );
    }
    else {
        m_mosq = mosquitto_new(NULL, m_bCleanSession, this );
    }

    //mosquitto_log_callback_set(m_mosq, my_log_callback);
	mosquitto_connect_callback_set(m_mosq, on_connect);
    mosquitto_disconnect_callback_set(m_mosq, on_disconnect);
	mosquitto_message_callback_set(m_mosq, on_message);
	mosquitto_publish_callback_set(m_mosq, on_publish);
    
    return VSCP_ERROR_SUCCESS;  
}

///////////////////////////////////////////////////////////////////////////////
// setRetain
//

int vscpClientMqtt::setRetain(bool bRetain) 
{
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setKeepAlive
//

int vscpClientMqtt::setKeepAlive(int keepAlive)
{
    return VSCP_ERROR_SUCCESS;
}


int vscpClientMqtt::set_tls( const std::string &cafile,
   	                            const std::string &capath,
   	                            const std::string &certfile,
   	                            const std::string &keyfile,
                                const std::string &password ) 
{
    m_cafile = cafile;
    m_capath = capath;
    m_certfile = certfile;
    m_keyfile = keyfile;
    m_pwKeyfile = password;
    m_bTLS = true;

    return VSCP_ERROR_SUCCESS;                
}

///////////////////////////////////////////////////////////////////////////////
// connect
//

int vscpClientMqtt::connect(void) 
{
    int mid;
    int rv = mosquitto_connect(m_mosq,
                                m_strHost.c_str(),
		                        m_port,
		                        m_keepalive	);
    if ( MOSQ_ERR_SUCCESS != rv ) {
        if (MOSQ_ERR_INVAL == rv) return VSCP_ERROR_PARAMETER;
        return VSCP_ERROR_ERRNO;
    }

    // Start the worker loop
    rv = mosquitto_loop_start(m_mosq);
    if (MOSQ_ERR_SUCCESS != rv ) {
        mosquitto_disconnect(m_mosq);
        return VSCP_ERROR_ERROR;
    }

    // Only subscribe if subscription topic is defined
    if ( m_strTopicSub.length() ) {

        rv = mosquitto_subscribe(m_mosq,
		                            &mid,
	                                m_strTopicSub.c_str(),
		                            m_qos );

        switch( rv ) {
            case MOSQ_ERR_INVAL:
                return VSCP_ERROR_PARAMETER;
            case MOSQ_ERR_NOMEM:
                return VSCP_ERROR_MEMORY;
            case MOSQ_ERR_NO_CONN:
                return VSCP_ERROR_CONNECTION;
            case MOSQ_ERR_MALFORMED_UTF8:
                return VSCP_ERROR_PARAMETER;
            case MOSQ_ERR_OVERSIZE_PACKET:
                return VSCP_ERROR_FIFO_SIZE;
        }

    }                            

    // Start worker thread if a callback has been defined
    if ( (NULL != m_evcallback) || (NULL != m_excallback) ) {
        int rv = pthread_create(&m_tid, NULL, workerThread, this); 
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// disconnect
//

int vscpClientMqtt::disconnect(void)
{
    m_bConnected = false;
    m_bRun = false;
    int rv = mosquitto_disconnect(m_mosq);
    if (MOSQ_ERR_INVAL == rv) return VSCP_ERROR_PARAMETER;
    if (MOSQ_ERR_NO_CONN == rv) return VSCP_ERROR_CONNECTION;

    // Start the worker loop
    rv = mosquitto_loop_stop(m_mosq, false);
    if (MOSQ_ERR_SUCCESS != rv ) {
        return VSCP_ERROR_ERROR;
    }

    pthread_join(m_tid, NULL);
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// isConnected
//

bool vscpClientMqtt::isConnected(void)
{
    return m_bConnected;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int vscpClientMqtt::send(vscpEvent &ev)
{
    int msgid;
    std::string strPayload;
    std::string strTopic;

    if ( m_format == jsonfmt ) {
        if ( !vscp_convertEventToJSON(strPayload, &ev) ) {
            return VSCP_ERROR_PARAMETER;    
        }
    }
    else if ( m_format == xmlfmt ) {
        if ( !vscp_convertEventToXML(strPayload, &ev) ) {
            return VSCP_ERROR_PARAMETER;    
        }
    }
    else {
        return VSCP_ERROR_NOT_SUPPORTED;
    }

    int rv = mosquitto_publish( m_mosq,
		                            &msgid,
	                                strTopic.c_str(),
		 		                    strPayload.length(),
	                                strPayload.c_str(),
		                            m_qos,
		                            m_bRetain );
    
    // Translate mosquitto erro code to VSCP error code
    switch (rv) {
        case MOSQ_ERR_INVAL:
            return VSCP_ERROR_PARAMETER;
        case MOSQ_ERR_NOMEM:
            return VSCP_ERROR_MEMORY;
        case MOSQ_ERR_NO_CONN:       
            return VSCP_ERROR_CONNECTION;
        case MOSQ_ERR_PROTOCOL:
            return VSCP_ERROR_COMMUNICATION;
        case MOSQ_ERR_PAYLOAD_SIZE:
            return VSCP_ERROR_FIFO_SIZE;
        case MOSQ_ERR_MALFORMED_UTF8:
            return VSCP_ERROR_PARAMETER;
        case MOSQ_ERR_QOS_NOT_SUPPORTED:
            return VSCP_ERROR_NOT_SUPPORTED;
        case MOSQ_ERR_OVERSIZE_PACKET:
            return VSCP_ERROR_FIFO_SIZE;        
    }   

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int vscpClientMqtt::send(vscpEventEx &ex)
{
    int msgid;
    std::string strPayload;
    std::string strTopic;

    if ( m_format == jsonfmt ) {
        if ( !vscp_convertEventExToJSON(strPayload, &ex) ) {
            return VSCP_ERROR_PARAMETER;    
        }
    }
    else if ( m_format == xmlfmt ) {
        if ( !vscp_convertEventExToXML(strPayload, &ex) ) {
            return VSCP_ERROR_PARAMETER;    
        }
    }
    else {
        return VSCP_ERROR_NOT_SUPPORTED;
    }

    int rv = mosquitto_publish( m_mosq,
		                            &msgid,
	                                strTopic.c_str(),
		 		                    strPayload.length(),
	                                strPayload.c_str(),
		                            m_qos,
		                            m_bRetain );
    
    // Translate mosquitto erro code to VSCP error code
    switch (rv) {
        case MOSQ_ERR_INVAL:
            return VSCP_ERROR_PARAMETER;
        case MOSQ_ERR_NOMEM:
            return VSCP_ERROR_MEMORY;
        case MOSQ_ERR_NO_CONN:       
            return VSCP_ERROR_CONNECTION;
        case MOSQ_ERR_PROTOCOL:
            return VSCP_ERROR_COMMUNICATION;
        case MOSQ_ERR_PAYLOAD_SIZE:
            return VSCP_ERROR_FIFO_SIZE;
        case MOSQ_ERR_MALFORMED_UTF8:
            return VSCP_ERROR_PARAMETER;
        case MOSQ_ERR_QOS_NOT_SUPPORTED:
            return VSCP_ERROR_NOT_SUPPORTED;
        case MOSQ_ERR_OVERSIZE_PACKET:
            return VSCP_ERROR_FIFO_SIZE;        
    }   

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int vscpClientMqtt::receive(vscpEvent &ev)
{    
    int rv;
    vscpEvent *pev = NULL;
    
    if ( m_receiveQueue.size()) {
        
        pev = m_receiveQueue.front();
        m_receiveQueue.pop_front();
        if (NULL == pev) return VSCP_ERROR_MEMORY;

        if ( !vscp_copyEvent(&ev, pev) ) {
            return VSCP_ERROR_MEMORY;
        }

        vscp_deleteEvent_v2(&pev);
    }
 
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int vscpClientMqtt::receive(vscpEventEx &ex)
{
    int rv;
    canalMsg canalMsg;
    uint8_t guid[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    
    // if ( CANAL_ERROR_SUCCESS != (rv = m_canalif.CanalReceive(&canalMsg) ) ) {
    //     return rv;
    // }

    // return vscp_convertCanalToEventEx(&ex,
    //                                     &canalMsg,
    //                                     guid);
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setfilter
//

int vscpClientMqtt::setfilter(vscpEventFilter &filter)
{
    int rv;

    uint32_t _filter = ((unsigned long)filter.filter_priority << 26) |
                    ((unsigned long)filter.filter_class << 16) |
                    ((unsigned long)filter.filter_type << 8) | filter.filter_GUID[0];
    // if ( CANAL_ERROR_SUCCESS == (rv = m_canalif.CanalSetFilter(_filter))) {
    //     return rv;
    // }

    uint32_t _mask = ((unsigned long)filter.mask_priority << 26) |
                    ((unsigned long)filter.mask_class << 16) |
                    ((unsigned long)filter.mask_type << 8) | filter.mask_GUID[0];
    // return m_canalif.CanalSetMask(_mask);
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getcount
//

int vscpClientMqtt::getcount(uint16_t *pcount)
{
    //return m_canalif.CanalDataAvailable();
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getinterfaces
//

int vscpClientMqtt::getinterfaces(std::deque<std::string> &iflist)
{
    // No interfaces available
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getwcyd
//

int vscpClientMqtt::getwcyd(uint64_t &wcyd)
{
    wcyd = VSCP_SERVER_CAPABILITY_NONE;   // No capabilities
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setCallback
//

int vscpClientMqtt::setCallback(LPFNDLL_EV_CALLBACK m_evcallback)
{
    // Can not be called when connected
    if ( m_bConnected ) return VSCP_ERROR_ERROR;

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setCallback
//

int vscpClientMqtt::setCallback(LPFNDLL_EX_CALLBACK m_excallback)
{
    // Can not be called when connected
    if ( m_bConnected ) return VSCP_ERROR_ERROR;

    return VSCP_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// Callback workerthread
//
// This thread call the appropriate callback when events are received
//


void *workerThread(void *pObj)
{
    uint8_t guid[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    vscpClientMqtt *pClient = (vscpClientMqtt *)pObj;
    //if (NULL == pif) return NULL;

    while (pClient->m_bRun) {

        //pthread_mutex_lock(&pif->m_mutexif);
        
        // Check if there are events to fetch
        int cnt;
        /* if ((cnt = pClient->m_canalif.CanalDataAvailable())) {

            while (cnt) {
                canalMsg msg;
                if ( CANAL_ERROR_SUCCESS == pClient->m_canalif.CanalReceive(&msg) ) {
                    if ( NULL != pClient->m_evcallback ) {
                        vscpEvent ev;
                        if (vscp_convertCanalToEvent(&ev,
                                                        &msg,
                                                        guid) ) {
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

        //pthread_mutex_unlock(&pif->m_mutexif);
        usleep(200);
    }

    return NULL;
}
