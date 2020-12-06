// ControlObject.cpp: m_path_db_vscp_logimplementation of the CControlObject
// class.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2020 Ake Hedman, Grodans Paradis AB
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

#include <arpa/inet.h>
#include <errno.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <pwd.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>
#ifdef WITH_SYSTEMD
#include <systemd/sd-daemon.h>
#endif

#include <expat.h>
#include <mosquitto.h>

#include <crc.h>
#include <devicelist.h>
#include <devicethread.h>
#include <randpassword.h>
#include <version.h>
#include <vscp.h>
#include <vscp_debug.h>
#include <vscpd_caps.h>
#include <vscphelper.h>

#include <algorithm>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <string>

#include <mustache.hpp>

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

    CControlObject *pObj = (CControlObject *)pData;
    //printf("MQTT LOG : %s\n", logmsg);

    if (pObj->m_debugFlags & VSCP_DEBUG_MQTT_LOG) {
        printf("MQTT LOG : %s\n", logmsg);
    }
}


static void mqtt_on_connect(struct mosquitto *mosq, void *pData, int rv)
{
    // Check pointers
    if (NULL == mosq) return;
    if (NULL == pData) return;

    CControlObject *pObj = (CControlObject *)pData;

    if (pObj->m_debugFlags & VSCP_DEBUG_MQTT_CONNECT) {
        printf("\n-------------------------------------------------------------------------------\n");
        printf("CONNECT\n");
    }
}


static void mqtt_on_disconnect(struct mosquitto *mosq, void *pData, int rv)
{
    // Check pointers
    if (NULL == mosq) return;
    if (NULL == pData) return;

    CControlObject *pObj = (CControlObject *)pData;

    if (pObj->m_debugFlags & VSCP_DEBUG_MQTT_CONNECT) {
        printf("\n-------------------------------------------------------------------------------\n");
        printf("DISCONNECT\n");
    }
}


static void mqtt_on_message(struct mosquitto *mosq, void *pData, const struct mosquitto_message *pMsg)
{
    // Check pointers
    if (NULL == mosq) return;
    if (NULL == pData) return;
    if (NULL == pMsg) return;

    CControlObject *pObj = (CControlObject *)pData;
    std::string payload((const char *)pMsg->payload, pMsg->payloadlen);

    if (pObj->m_debugFlags & VSCP_DEBUG_MQTT_MSG) {
        printf("\n-------------------------------------------------------------------------------\n");
        printf("MESSAGE ------> \n %s", payload.c_str() );
    }
}


static void mqtt_on_publish(struct mosquitto *mosq, void *pData, int rv)
{
    // Check pointers
    if (NULL == mosq) return;
    if (NULL == pData) return;

    CControlObject *pObj = (CControlObject *)pData;

    if (pObj->m_debugFlags & VSCP_DEBUG_MQTT_PUBLISH) {
        printf("\n-------------------------------------------------------------------------------\n");
        printf("PUBLISH ------> \n");
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
    openlog("vscpd", LOG_CONS, LOG_DAEMON);

    if (m_debugFlags & VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG, "Starting the vscpd daemon");
    }

    if (0 != pthread_mutex_init(&m_mutex_DeviceList, NULL)) {
        syslog(LOG_ERR, "Unable to init m_mutex_DeviceList");
        return;
    }

    m_strServerName = "VSCP Daemon";
    m_rootFolder = "/var/lib/vscp/vscpd/";
    m_vscptoken = "Carpe diem quam minimum credula postero";

    // Nill the GUID
    m_guid.clear();

    // Initialize the CRC
    crcInit();

    // Init daemon MQTT
    m_mqtt_strHost = "localhost";
    m_mqtt_port = 1883;
    m_mqtt_strclientId = "";
    m_mqtt_strUserName = "vscp";
    m_mqtt_strPassword = "secret";
    m_mqtt_qos = 0;
    m_mqtt_bRetain = false;
    m_mqtt_keepalive = 60;
    m_mqtt_bCleanSession = false;
    m_mqtt_bTLS = false;
    m_mqtt_cafile = "";
    m_mqtt_capath = "";
    m_mqtt_certfile = "";
    m_mqtt_keyfile = "";
    m_mqtt_pwKeyfile = "";
    m_mqtt_format = jsonfmt;
    m_topicInterfaces = "vscp/{{guid}}/interfaces";
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CControlObject::~CControlObject()
{
    if (m_debugFlags & VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG, "Cleaning up");
    }

    if (0 != pthread_mutex_destroy(&m_mutex_DeviceList)) {
        syslog(LOG_ERR, "Unable to destroy m_mutex_DeviceList");
        return;
    }

    if (m_debugFlags & VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG, "Terminating the vscpd daemon");
    }

    // Close syslog
    closelog();
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
        syslog(LOG_ERR,
               "The specified rootfolder does not exist (%s).",
               (const char*)m_rootFolder.c_str());
        return false;
    }

    // Change locale to get the correct decimal point "."
    setlocale(LC_NUMERIC, "C");

    // A configuration file must be available
    if (!vscp_fileExists(strcfgfile.c_str())) {
        printf("No configuration file. Can't initialize!.");
        syslog(LOG_ERR,
               "No configuration file. Can't initialize!. Path=%s",
               strcfgfile.c_str());
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////
    //                         Read XML configuration
    ////////////////////////////////////////////////////////////////////////////

    // Read XML configuration
    if (m_debugFlags & VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG, "Reading configuration file");
    }

    // Read XML configuration
    try {
        if (!readConfiguration(strcfgfile)) {
            syslog(LOG_ERR,
                   "Unable to open/parse configuration file. Can't initialize! "
                   "Path =%s",
                   strcfgfile.c_str());
            return FALSE;
        }
    }
    catch (...) {
        syslog(LOG_ERR, "Exception when reading configuration file");
        return FALSE;
    }

#ifndef WIN32
    if (m_runAsUser.length()) {
        struct passwd* pw;
        if (NULL == (pw = getpwnam(m_runAsUser.c_str()))) {
            syslog(LOG_ERR, "Unknown user.");
        }
        else if (setgid(pw->pw_gid) != 0) {
            syslog(LOG_ERR, "setgid() failed. [%s]", strerror(errno));
        }
        else if (setuid(pw->pw_uid) != 0) {
            syslog(LOG_ERR, "setuid() failed. [%s]", strerror(errno));
        }
    }
#endif

    if (m_debugFlags & VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG, "Using configuration file: %s", strcfgfile.c_str());
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
    syslog(LOG_INFO, "%s", str.c_str());

    // Initialize MQTT

    if ( MOSQ_ERR_SUCCESS != mosquitto_lib_init() ) {
        syslog(LOG_ERR, "Unable to initialize mosquitto library.");
        return false;
    }

    if (m_mqtt_strclientId.length()) {
        m_mosq = mosquitto_new(m_mqtt_strclientId.c_str(), m_mqtt_bCleanSession, this);
    }
    else {
        m_mqtt_bCleanSession = true;    // Must be true without id
        m_mosq = mosquitto_new(NULL, m_mqtt_bCleanSession, this);
    }

    if (NULL == m_mosq) {
        if (ENOMEM == errno) {
            syslog(LOG_ERR, "Failed to create new mosquitto session (out of memory).");
        }
        else if (EINVAL == errno) {
            syslog(LOG_ERR, "Failed to create new mosquitto session (invalid parameters).");
        }
        return false;
    }

    mosquitto_log_callback_set(m_mosq, mqtt_log_callback);
    mosquitto_connect_callback_set(m_mosq, mqtt_on_connect);
    mosquitto_disconnect_callback_set(m_mosq, mqtt_on_disconnect);
    mosquitto_message_callback_set(m_mosq, mqtt_on_message);
    mosquitto_publish_callback_set(m_mosq, mqtt_on_publish);

    // Set username/password if defined
    if (m_mqtt_strUserName.length()) { 
        int rv;       
        if ( MOSQ_ERR_SUCCESS != 
                ( rv = mosquitto_username_pw_set( m_mosq,
                                                    m_mqtt_strUserName.c_str(),
                                                    m_mqtt_strPassword.c_str() ) ) ) {
            if ( MOSQ_ERR_INVAL == rv) {
                syslog(LOG_ERR, "Failed to set mosquitto username/password (invalid parameter(s)).");
            }
            else if ( MOSQ_ERR_NOMEM == rv) {
                syslog(LOG_ERR, "Failed to set mosquitto username/password (out of memory).");
            }
                                                    
        }
    }

    int rv = mosquitto_connect(m_mosq, 
                                m_mqtt_strHost.c_str(), 
                                m_mqtt_port, 
                                m_mqtt_keepalive);

    if ( MOSQ_ERR_SUCCESS != rv ) {
        
        if (MOSQ_ERR_INVAL == rv) {
            syslog(LOG_ERR, "Failed to connect to mosquitto server (invalid parameter(s)).");
        }
        else if (MOSQ_ERR_ERRNO == rv) {
            syslog(LOG_ERR, "Failed to connect to mosquitto server. System returned error (errno = %d).", errno);
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
            ++it){

        std::string topic = *it;

        // Fix subscribe/publish topics
        mustache subtemplate{topic};
        data data;
        data.set("guid", m_guid.getAsString());
        std::string subscribe_topic = subtemplate.render(data);

        // Subscribe to specified topic
        rv = mosquitto_subscribe(m_mosq,
                                    &m_mqtt_id,
                                    subscribe_topic.c_str(),
                                    m_mqtt_qos);

        switch (rv) {
            case MOSQ_ERR_INVAL:
                syslog(LOG_ERR, "Failed to subscribed to specified topic [%s] - input parameters were invalid.",subscribe_topic.c_str());
            case MOSQ_ERR_NOMEM:
                syslog(LOG_ERR, "Failed to subscribed to specified topic [%s] - out of memory condition occurred.",subscribe_topic.c_str());
            case MOSQ_ERR_NO_CONN:
                syslog(LOG_ERR, "Failed to subscribed to specified topic [%s] - client isn’t connected to a broker.",subscribe_topic.c_str());
            case MOSQ_ERR_MALFORMED_UTF8:
                syslog(LOG_ERR, "Failed to subscribed to specified topic [%s] - topic is not valid UTF-8.",subscribe_topic.c_str());
            case MOSQ_ERR_OVERSIZE_PACKET:
                syslog(LOG_ERR, "Failed to subscribed to specified topic [%s] - resulting packet would be larger than supported by the broker.",subscribe_topic.c_str());
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
    printf("%s\n",strPayload.c_str());
    rv = mosquitto_publish(m_mosq,
                                    NULL,
                                    strTopic.c_str(),
                                    strPayload.length(),
                                    strPayload.c_str(),
                                    m_mqtt_qos,
                                    true );
   

    // Load drivers
    try {
        startDeviceWorkerThreads();
    }
    catch (...) {
        syslog(LOG_ERR, "Exception when loading drivers");
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
               "ControlObject: cleanup - Giving worker threads time to stop "
               "operations...");
    }

    if (m_debugFlags & VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG,
               "ControlObject: cleanup - Stopping device worker thread...");
    }

    try {
        stopDeviceWorkerThreads();
    }
    catch (...) {
        syslog(LOG_ERR,
               "REST: Exception occurred when stoping device worker threads");
    }

    if (m_debugFlags & VSCP_DEBUG_EXTRA) {
        syslog(
          LOG_DEBUG,
          "ControlObject: cleanup - Stopping VSCP Server worker thread...");
    }

    if (m_debugFlags & VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG,
               "ControlObject: cleanup - Stopping client worker thread...");
    }

    // Disconnect from MQTT broker}
    int rv = mosquitto_disconnect(m_mosq);
    if (MOSQ_ERR_SUCCESS != rv) {
        if (MOSQ_ERR_INVAL == rv) {
            syslog(LOG_ERR, "ControlObject: mosquitto_disconnect: input parameters were invalid.");
        }
        else if (MOSQ_ERR_NO_CONN == rv) {
            syslog(LOG_ERR, "ControlObject: mosquitto_disconnect: client isn’t connected to a broker");
        }
    }

    // stop the worker loop
    rv = mosquitto_loop_stop(m_mosq, false);
    if (MOSQ_ERR_SUCCESS != rv) {
        if (MOSQ_ERR_INVAL == rv) {
            syslog(LOG_ERR, "ControlObject: mosquitto_loop_stop: input parameters were invalid.");
        }
        else if (MOSQ_ERR_NOT_SUPPORTED == rv) {
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

            if (periodicEvents()) {
                 syslog(LOG_ERR, "Failed to send automation events!");
            }
        }

    } // while


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
    int rv = true;
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
    }
    else if ( m_mqtt_format == xmlfmt ) {
        if ( !vscp_convertEventExToXML(strPayload, pex) ) {
            syslog(LOG_ERR, "ControlObject: sendEvent: Failed to convert event to XML");
            return false;
        }
    }
    else if ( m_mqtt_format == strfmt ) {
        if ( !vscp_convertEventExToString(strPayload, pex) ) {
            syslog(LOG_ERR, "ControlObject: sendEvent: Failed to convert event to STRING");
            return false;
        }
    }
    else if ( m_mqtt_format == binfmt ) {
        pbuf = new uint8_t[VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 + pex->sizeData];
        if (NULL == pbuf) {
            return false;
        }

        if (!vscp_writeEventExToFrame( pbuf,
                                        VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 + pex->sizeData,
                                        VSCP_MULTICAST_TYPE_EVENT,
                                        pex ) ) {
            syslog(LOG_ERR, "ControlObject: sendEvent: Failed to convert event to BINARY");
            return false;
        }
    }
    else {
        return VSCP_ERROR_NOT_SUPPORTED;
    }

    for (std::list<std::string>::const_iterator 
            it = m_mqtt_publish.begin(); 
            it != m_mqtt_publish.end(); 
            ++it){

        std::string topic_template = *it;

        // Fix publish topics
        mustache subtemplate{topic_template};
        data data;
        data.set("guid", m_guid.getAsString());
        data.set("class", vscp_str_format("%d",pex->vscp_class));
        data.set("type", vscp_str_format("%d",pex->vscp_type));
        strTopic = subtemplate.render(data);

        if (m_mqtt_format != binfmt) {
            rv = mosquitto_publish(m_mosq,
                                    NULL,
                                    strTopic.c_str(),
                                    strPayload.length(),
                                    strPayload.c_str(),
                                    m_mqtt_qos,
                                    FALSE );
        }
        else {
            rv = mosquitto_publish(m_mosq,
                                    NULL,
                                    strTopic.c_str(),
                                    VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 + pex->sizeData,
                                    pbuf,
                                    m_mqtt_qos,
                                    FALSE );
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
            // case MOSQ_ERR_QOS_NOT_SUPPORTED:
            //     syslog(LOG_ERR, "ControlObject: sendEvent: Error QOS not supported");
            //     break;
            // case MOSQ_ERR_OVERSIZE_PACKET:
            //     syslog(LOG_ERR, "ControlObject: sendEvent: Error Oversized package");
            //     break;
        }

        // End if error
        if (MOSQ_ERR_SUCCESS != rv) break;

    }  // for

    if (m_mqtt_format == binfmt) {
        // Clean up allocated buffer                                    
        if (NULL != pbuf) {
            delete [] pbuf;
            pbuf = NULL;
        }
    }

    return rv;
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
        syslog(LOG_ERR, "Failed to send Class1 heartbeat");
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
           std::min((int)strlen(m_strServerName.c_str()), 64));

    if (!sendEvent(&ex)) {
         syslog(LOG_ERR, "Failed to send Class2 heartbeat");
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
    uint32_t time32 = (uint32_t)tnow;

    ex.data[0] = 0; // 8 - bit crc for VSCP daemon GUID
    ex.data[1] = (uint8_t)((time32 >> 24) & 0xff); // Time since epoch MSB
    ex.data[2] = (uint8_t)((time32 >> 16) & 0xff);
    ex.data[3] = (uint8_t)((time32 >> 8) & 0xff);
    ex.data[4] = (uint8_t)((time32)&0xff); // Time since epoch LSB

    if (!sendEvent(&ex)) {
         syslog(LOG_ERR, "Failed to send segment controller heartbeat");
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
            (const char*)m_strServerName.c_str(),
            std::min((int)strlen((const char*)m_strServerName.c_str()), 64));

    // Capabilities array
    getVscpCapabilities(ex.data);

    // non-standard ports
    // TODO

    ex.sizeData = 104;
    m_guid.writeGUID(ex.GUID);

    if (!sendEvent(&ex)) {
         syslog(LOG_ERR, "Failed to send high end server capabilities.");
    }

    return true;
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

    // TODO

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
        syslog(LOG_DEBUG, "[Controlobject][Driver] - Starting drivers...");
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

        } // Valid device item
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
        syslog(LOG_DEBUG, "[Controlobject][Driver] - Stopping drivers...");
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
    }
    else {
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
    unsigned long localaddr[16]; // max 16 local addresses
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




// ----------------------------------------------------------------------------
// FULL XML configuration callbacks
// ----------------------------------------------------------------------------

static int depth_full_config_parser = 0;
static int bVscpConfigFound         = 0;
static int bGeneralConfigFound      = 0;
static int bMQTTConfigFound         = 0;
static int bLevel1DriverConfigFound = 0;
static int bLevel2DriverConfigFound = 0;
static int bDriverConfigFound       = 0;

// Name of driver currenly being configured
static std::string m_currentDriverName;

static void
startFullConfigParser(void* data, const char* name, const char** attr)
{
    CControlObject* pObj = (CControlObject*)data;
    if (NULL == data) {
        return;
    }

    // fprintf(stderr, "%s\n", name);

    if ((0 == depth_full_config_parser) &&
        (0 == vscp_strcasecmp(name, "vscpconfig"))) {
        bVscpConfigFound = TRUE;
    }
    else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "general"))) {
        bGeneralConfigFound = TRUE;

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "runasuser")) {
                vscp_trim(attribute);
                pObj->m_runAsUser = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "guid")) {
                pObj->m_guid.getFromString(attribute);
            }
            else if (0 == vscp_strcasecmp(attr[i], "servername")) {
                pObj->m_strServerName = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "debug")) {
                pObj->m_debugFlags = vscp_readStringValue(attribute);
            }
        }
    }
    else if (bVscpConfigFound && bGeneralConfigFound &&
             (2 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "security"))) {

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "vscptoken")) {
                pObj->m_vscptoken = attribute;
            }

        }
    }

    
    else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "mqtt"))) {

        bMQTTConfigFound = TRUE;

        for (int i=0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "host")) {
                vscp_startsWith(attribute, "tcp://", &attribute);
                vscp_startsWith(attribute, "stcp://", &attribute);
                vscp_trim(attribute);
                pObj->m_mqtt_strHost = attribute;
                if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
	                printf("MQTT hostname set to %s\n", attribute.c_str());
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "port")) {
                pObj->m_mqtt_port = vscp_readStringValue(attribute);
                if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
	                printf("MQTT port set to %d\n", pObj->m_mqtt_port);
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "user")) {
                pObj->m_mqtt_strUserName = attribute;
                if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
	                printf("MQTT user set to %s\n", attribute.c_str());
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "password")) {
                pObj->m_mqtt_strPassword = attribute;
                if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
	                printf("MQTT password set to **********\n");
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "clientid")) {
                pObj->m_mqtt_strclientId = attribute;
                if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
	                printf("MQTT ClientId set to %s\n",pObj->m_mqtt_strclientId.c_str());
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "qos")) {
                pObj->m_mqtt_qos = vscp_readStringValue(attribute);
                if (pObj->m_mqtt_qos > 3) {
                    syslog(LOG_ERR,"Config: MQTT qos > 3. Set to 0.");
                    pObj->m_mqtt_qos = 0;
                }
                if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
	                printf("MQTT qos set to %d\n", pObj->m_mqtt_qos);
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "bcleansession")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_mqtt_bCleanSession = true;
                }
                else {
                    pObj->m_mqtt_bCleanSession = false;
                }
                if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
	                printf("MQTT bcleansession set to %s\n", pObj->m_mqtt_bCleanSession ? "true" : "false");
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "bretain")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_mqtt_bRetain = true;
                }
                else {
                    pObj->m_mqtt_bRetain = false;
                }
                if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
	                printf("MQTT bretain set to %s\n", pObj->m_mqtt_bRetain ? "true" : "false");
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "keepalive")) {   
                pObj->m_mqtt_keepalive = vscp_readStringValue(attribute);
                if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
	                printf("MQTT keepalive set to %d\n", pObj->m_mqtt_keepalive);
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "cafile")) {
                if ( attribute.length() ) pObj->m_mqtt_bTLS = true;
                pObj->m_mqtt_cafile = attribute;
                if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
	                printf("MQTT cafile set to %s\n", attribute.c_str());
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "capath")) {
                if ( attribute.length() ) pObj->m_mqtt_bTLS = true;
                pObj->m_mqtt_capath = attribute;
                if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
	                printf("MQTT capath set to %s\n", attribute.c_str());
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "certfile")) {
                if ( attribute.length() ) pObj->m_mqtt_bTLS = true;
                pObj->m_mqtt_certfile = attribute;
                if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
	                printf("MQTT certfile set to %s\n", attribute.c_str());
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "keyfile")) {
                if ( attribute.length() ) pObj->m_mqtt_bTLS = true;
                pObj->m_mqtt_keyfile = attribute;
                if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
	                printf("MQTT keyfile set to %s\n", attribute.c_str());
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "pwkeyfile")) {
                pObj->m_mqtt_pwKeyfile = attribute;
                if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
	                printf("MQTT pwkeyfile set to %s\n", attribute.c_str());
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "topic-interfaces")) {
                pObj->m_topicInterfaces = attribute;
                if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
	                printf("MQTT topic-interfaces set to %s\n", attribute.c_str());
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "format")) {

                vscp_makeUpper(attribute);
                if (0 == vscp_strcasecmp(attribute.c_str(), "JSON")) {
                    pObj->m_mqtt_format = jsonfmt;
                    if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                        printf("MQTT format set to 'JSON' (%s)\n",attribute.c_str());
                    }
                }
                else if (0 == vscp_strcasecmp(attribute.c_str(), "XML")) {
                    pObj->m_mqtt_format = xmlfmt;
                    if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                        printf("MQTT format set to 'XML' (%s)\n",attribute.c_str());
                    }
                }
                else if (0 == vscp_strcasecmp(attribute.c_str(), "STRING")) {
                    pObj->m_mqtt_format = strfmt;
                    if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                        printf("MQTT format set to 'STRING' (%s)\n",attribute.c_str());
                    }
                }
                else if (0 == vscp_strcasecmp(attribute.c_str(), "BINARY")) {
                    pObj->m_mqtt_format = binfmt;
                    if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                        printf("MQTT format set to 'BINARY' (%s)\n",attribute.c_str());
                    }
                }
                else {
                    pObj->m_mqtt_format = jsonfmt;
                    if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                        printf("Unknown format: MQTT format set to 'JSON' (%s)\n",attribute.c_str());
                    }
                }
            }

        }
        
    }
    else if (bVscpConfigFound && bMQTTConfigFound &&
             (2 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "subscribe"))) {

        for (int i=0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "topic")) {
                pObj->m_mqtt_subscriptions.push_back(attribute);
            }

        }

    }
    else if (bVscpConfigFound && bMQTTConfigFound &&
             (2 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "publish"))) {

        std::string topic;

        for (int i=0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "topic")) {
                pObj->m_mqtt_publish.push_back(attribute);
            }

        }

    }
    else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
             ((0 == vscp_strcasecmp(name, "level1driver")) ||
              (0 == vscp_strcasecmp(name, "canal1driver")))) {
        bLevel1DriverConfigFound = TRUE;
    }
    else if (bVscpConfigFound && bLevel1DriverConfigFound &&
             (2 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "driver"))) {

        std::string strName;
        std::string strConfig;
        std::string strPath;
        unsigned long flags  = 0;
        uint32_t translation = 0;
        cguid guid;
        bool bEnabled = false;

        bDriverConfigFound = TRUE;

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "enable")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    bEnabled = true;
                }
                else {
                    bEnabled = false;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "name")) {
                strName = attribute;
                // Replace possible spaces in name with underscore
                std::string::size_type found;
                while (std::string::npos !=
                       (found = strName.find_first_of(" "))) {
                    strName[found] = '_';
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "config")) {
                strConfig = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "parameter")) { // deprecated
                strConfig = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "path")) {
                strPath = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "flags")) {
                flags = vscp_readStringValue(attribute);
            }
            else if (0 == vscp_strcasecmp(attr[i], "guid")) {
                guid.getFromString(attribute);
            }
            else if (0 == vscp_strcasecmp(attr[i], "translation")) {
                translation = vscp_readStringValue(attribute);
            }
        } // for

        // Save driver name for other stages
        m_currentDriverName = strName;

        if (bEnabled) {
            // Add the level I device            
            if (!pObj->m_deviceList.addItem( pObj->m_debugFlags,
                                                strName,
                                                strConfig,
                                                strPath,
                                                flags,
                                                guid,
                                                VSCP_DRIVER_LEVEL1,                                            
                                                translation)) {
                syslog(LOG_ERR,
                       "Level I driver not added name=%s. "
                       "Path does not exist. - [%s]",
                       strName.c_str(),
                       strPath.c_str());
            }
            else {
                if (pObj->m_debugFlags & VSCP_DEBUG_DRIVERL1) {
                    syslog(LOG_DEBUG,
                           "ControlObject: ReadConfig: Level I driver added. name = %s - [%s]",
                           strName.c_str(),
                           strPath.c_str());
                }
            }
        }
    }
    else if (bVscpConfigFound && bLevel1DriverConfigFound &&
             (3 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "mqtt"))) {

        bMQTTConfigFound = true;

        CDeviceItem *pDriver = pObj->m_deviceList.getDeviceItemFromName(m_currentDriverName);

        if ( NULL == pDriver ) {
            syslog(LOG_ERR,"ControlObject: ReadConfig: Can't get driver object %s", m_currentDriverName.c_str() );
        }
        else {

            // Set defaults from master thread
            pDriver->m_mqtt_strHost = pObj->m_mqtt_strHost;
            pDriver->m_mqtt_port = pObj->m_mqtt_port;
            pDriver->m_mqtt_strUserName = pObj->m_mqtt_strUserName;
            pDriver->m_mqtt_strPassword = pObj->m_mqtt_strPassword;
            pDriver->m_mqtt_strClientId = "";
            pDriver->m_mqtt_qos = pObj->m_mqtt_qos;
            pDriver->m_mqtt_bCleanSession = pObj->m_mqtt_bCleanSession;
            pDriver->m_mqtt_bRetain = pObj->m_mqtt_bRetain;
            pDriver->m_mqtt_keepalive = pObj->m_mqtt_keepalive;
            pDriver->m_mqtt_cafile = pObj->m_mqtt_cafile;
            pDriver->m_mqtt_capath = pObj->m_mqtt_capath;
            pDriver->m_mqtt_certfile = pObj->m_mqtt_certfile;
            pDriver->m_mqtt_keyfile = pObj->m_mqtt_keyfile;
            pDriver->m_mqtt_pwKeyfile = pObj->m_mqtt_pwKeyfile;
            pDriver->m_mqtt_format = pObj->m_mqtt_format;

            if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                printf("Level I driver MQTT defaults set from main process\n");
            }

            for (int i=0; attr[i]; i += 2) {

                std::string attribute = attr[i + 1];
                vscp_trim(attribute);

                if (0 == vscp_strcasecmp(attr[i], "host")) {
                    vscp_startsWith(attribute, "tcp://", &attribute);
                    vscp_startsWith(attribute, "stcp://", &attribute);
                    vscp_trim(attribute);
                    pDriver->m_mqtt_strHost = attribute;
                    if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                        printf("Level I driver MQTT host set to %s\n", attribute.c_str());
                    }
                }
                else if (0 == vscp_strcasecmp(attr[i], "port")) {
                    pDriver->m_mqtt_port = vscp_readStringValue(attribute);
                    if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                        printf("Level I driver MQTT port set to %d\n", pDriver->m_mqtt_port);
                    }
                }
                else if (0 == vscp_strcasecmp(attr[i], "user")) {
                    pDriver->m_mqtt_strUserName = attribute;
                    if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                        printf("Level I driver MQTT username set to %s\n", attribute.c_str());
                    }
                }
                else if (0 == vscp_strcasecmp(attr[i], "password")) {
                    pDriver->m_mqtt_strPassword = attribute;
                    if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                        printf("Level I driver MQTT password set to **********\n");
                    }
                }
                else if (0 == vscp_strcasecmp(attr[i], "clientid")) {
                    pDriver->m_mqtt_strClientId = attribute;
                    if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
	                    printf("Level I driver MQTT ClientId set to %s\n",pObj->m_mqtt_strclientId.c_str());
                    }
                }
                else if (0 == vscp_strcasecmp(attr[i], "qos")) {
                    pDriver->m_mqtt_qos = vscp_readStringValue(attribute);
                    if (pDriver->m_mqtt_qos > 3) {
                        syslog(LOG_ERR,"Config: MQTT qos > 3. Set to 0.");
                        pDriver->m_mqtt_qos = 0;
                    }
                    if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                        printf("Level I driver MQTT qos set to %d\n", pDriver->m_mqtt_qos);
                    }
                }
                else if (0 == vscp_strcasecmp(attr[i], "bcleansession")) {
                    if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                        pDriver->m_mqtt_bCleanSession = true;
                    }
                    else {
                        pDriver->m_mqtt_bCleanSession = false;
                    }
                    if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                        printf("Level I driver MQTT bcleansession set to %s\n", pDriver->m_mqtt_bCleanSession ? "true" : "false");
                    }
                }
                else if (0 == vscp_strcasecmp(attr[i], "bretain")) {
                    if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                        pDriver->m_mqtt_bRetain = true;
                    }
                    else {
                        pDriver->m_mqtt_bRetain = false;
                    }
                    if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                        printf("Level I driver MQTT bretain set to %s\n", pDriver->m_mqtt_bRetain ? "true" : "false");
                    }
                }
                else if (0 == vscp_strcasecmp(attr[i], "keepalive")) {   
                    pDriver->m_mqtt_keepalive = vscp_readStringValue(attribute);
                    if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                        printf("Level I driver MQTT keepalive set to %d\n", pDriver->m_mqtt_keepalive);
                    }
                }
                else if (0 == vscp_strcasecmp(attr[i], "cafile")) {
                    if ( attribute.length() ) pDriver->m_mqtt_bTLS = true;
                    pDriver->m_mqtt_cafile = attribute;
                    if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                        printf("Level I driver MQTT cafile set to %s\n", attribute.c_str());
                    }
                }
                else if (0 == vscp_strcasecmp(attr[i], "capath")) {
                    if ( attribute.length() ) pDriver->m_mqtt_bTLS = true;
                    pDriver->m_mqtt_capath = attribute; 
                    if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                        printf("Level I driver MQTT capath set to %s\n", attribute.c_str());
                    }
                }
                else if (0 == vscp_strcasecmp(attr[i], "certfile")) {
                    if ( attribute.length() ) pDriver->m_mqtt_bTLS = true;
                    pDriver->m_mqtt_certfile = attribute; 
                    if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                        printf("Level I driver MQTT certfile set to %s\n", attribute.c_str());
                    }
                }
                else if (0 == vscp_strcasecmp(attr[i], "keyfile")) {
                    if ( attribute.length() ) pDriver->m_mqtt_bTLS = true;
                    pDriver->m_mqtt_keyfile = attribute; 
                    if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                        printf("Level I driver MQTT keyfile set to %s\n", attribute.c_str());
                    }
                }
                else if (0 == vscp_strcasecmp(attr[i], "pwkeyfile")) {
                    if ( attribute.length() ) pDriver->m_mqtt_bTLS = true;
                    pDriver->m_mqtt_pwKeyfile = attribute; 
                    if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                        printf("Level I driver MQTT pwkeyfile set to %s\n", attribute.c_str());
                    }
                }
                else if (0 == vscp_strcasecmp(attr[i], "format")) {

                    vscp_makeUpper(attribute);
                    if (0 == vscp_strcasecmp(attribute.c_str(), "JSON")) {
                        pDriver->m_mqtt_format = jsonfmt;
                        if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                            printf("Level I driver MQTT format set to 'JSON' (%s)\n",attribute.c_str());
                        }
                    }
                    else if (0 == vscp_strcasecmp(attribute.c_str(), "XML")) {
                        pObj->m_mqtt_format = xmlfmt;
                        if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                            printf("Level I driver MQTT format set to 'XML' (%s)\n",attribute.c_str());
                        }
                    }
                    else if (0 == vscp_strcasecmp(attribute.c_str(), "STRING")) {
                        pObj->m_mqtt_format = strfmt;
                        if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                            printf("Level I driver MQTT format set to 'STRING' (%s)\n",attribute.c_str());
                        }
                    }
                    else if (0 == vscp_strcasecmp(attribute.c_str(), "BINARY")) {
                        pObj->m_mqtt_format = binfmt;
                        if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                            printf("Level I driver MQTT format set to 'BINARY' (%s)\n",attribute.c_str());
                        }
                    }
                    else {
                        pDriver->m_mqtt_format = jsonfmt;
                        if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                            printf("Unknown format: Level I driver MQTT format set to 'JSON' (%s)\n",attribute.c_str());
                        }
                    }
                }
            }  // for
        }
    }
    else if (bVscpConfigFound && bLevel1DriverConfigFound && bMQTTConfigFound &&
             (4 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "subscribe"))) {

        CDeviceItem *pDriver = pObj->m_deviceList.getDeviceItemFromName(m_currentDriverName);
        if ( NULL == pDriver ) {
            syslog(LOG_ERR,"ControlObject: ReadConfig: L1:Subscribe Can't get driver object %s", m_currentDriverName.c_str() );
        }
        else {
            for (int i = 0; attr[i]; i += 2) {

                std::string attribute = attr[i + 1];
                vscp_trim(attribute);

                if (0 == vscp_strcasecmp(attr[i], "topic")) {
                    pDriver->m_mqtt_subscriptions.push_back(attribute);
                }
            }
        }
    }
    else if (bVscpConfigFound && 
                bLevel1DriverConfigFound && 
                bMQTTConfigFound &&
                (4 == depth_full_config_parser) &&
                (0 == vscp_strcasecmp(name, "publish"))) {

        CDeviceItem *pDriver = pObj->m_deviceList.getDeviceItemFromName(m_currentDriverName);
        if ( NULL == pDriver ) {
            syslog(LOG_ERR,"ControlObject: ReadConfig L1:Publish: Can't get driver object %s", m_currentDriverName.c_str() );
        }
        else {
            for (int i = 0; attr[i]; i += 2) {

                std::string attribute = attr[i + 1];
                vscp_trim(attribute);

                if (0 == vscp_strcasecmp(attr[i], "topic")) {
                    pDriver->m_mqtt_publish.push_back(attribute);
                }
            }
        }
    }
    else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
             ((0 == vscp_strcasecmp(name, "level2driver")))) {
        bLevel2DriverConfigFound = TRUE;
    }
    else if (bVscpConfigFound && bLevel2DriverConfigFound &&
             (2 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "driver"))) {

        std::string strName;
        std::string strConfig;
        std::string strPath;
        cguid guid;
        bool bEnabled = false;

        bDriverConfigFound = TRUE;

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "enable")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    bEnabled = true;
                }
                else {
                    bEnabled = false;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "name")) {
                strName = attribute;
                // Replace spaces in name with underscore
                std::string::size_type found;
                while (std::string::npos !=
                       (found = strName.find_first_of(" "))) {
                    strName[found] = '_';
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "path-config")) {
                strConfig = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i],
                                          "parameter")) { // deprecated
                strConfig = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "path-driver")) {
                strPath = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "guid")) {
                guid.getFromString(attribute);
            }
        } // for

        // Save driver name for other stages
        m_currentDriverName = strName;

        // Add the level II device
        if (bEnabled) {

            if (!pObj->m_deviceList.addItem( pObj->m_debugFlags,
                                                strName,
                                                strConfig,
                                                strPath,
                                                0,
                                                guid,
                                                VSCP_DRIVER_LEVEL2)) {
                if (pObj->m_debugFlags & VSCP_DEBUG_DRIVERL2) {
                    syslog(LOG_ERR,
                           "Level II driver was not added. name = %s"
                           "Path does not exist. - [%s]",
                           strName.c_str(),
                           strPath.c_str());
                }
            }
            else {
                if (pObj->m_debugFlags & VSCP_DEBUG_DRIVERL2) {
                    syslog(LOG_DEBUG,
                           "Level II driver added. name = %s- [%s]",
                           strName.c_str(),
                           strPath.c_str());
                }
            }
        }
    }
    else if (bVscpConfigFound && bLevel2DriverConfigFound &&
             (3 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "mqtt"))) {

        bMQTTConfigFound = true;

        CDeviceItem *pDriver = pObj->m_deviceList.getDeviceItemFromName(m_currentDriverName);

        if ( NULL == pDriver ) {
            syslog(LOG_ERR,"ControlObject: ReadConfig: Can't get driver object %s", m_currentDriverName.c_str() );
        }
        else {

            // Set defaults from master thread
            pDriver->m_mqtt_strHost = pObj->m_mqtt_strHost;
            pDriver->m_mqtt_port = pObj->m_mqtt_port;
            pDriver->m_mqtt_strUserName = pObj->m_mqtt_strUserName;
            pDriver->m_mqtt_strPassword = pObj->m_mqtt_strPassword;
            pDriver->m_mqtt_strClientId = "";
            pDriver->m_mqtt_qos = pObj->m_mqtt_qos;
            pDriver->m_mqtt_bCleanSession = pObj->m_mqtt_bCleanSession;
            pDriver->m_mqtt_bRetain = pObj->m_mqtt_bRetain;
            pDriver->m_mqtt_keepalive = pObj->m_mqtt_keepalive;
            pDriver->m_mqtt_cafile = pObj->m_mqtt_cafile;
            pDriver->m_mqtt_capath = pObj->m_mqtt_capath;
            pDriver->m_mqtt_certfile = pObj->m_mqtt_certfile;
            pDriver->m_mqtt_keyfile = pObj->m_mqtt_keyfile;
            pDriver->m_mqtt_pwKeyfile = pObj->m_mqtt_pwKeyfile;
            pDriver->m_mqtt_format = pObj->m_mqtt_format;

            if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                printf("Level II driver MQTT defaults set from main process\n");
            }

            for (int i=0; attr[i]; i += 2) {

                std::string attribute = attr[i + 1];
                vscp_trim(attribute);

                if (0 == vscp_strcasecmp(attr[i], "host")) {
                    vscp_startsWith(attribute, "tcp://", &attribute);
                    vscp_startsWith(attribute, "stcp://", &attribute);
                    vscp_trim(attribute);
                    pDriver->m_mqtt_strHost = attribute;
                    if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                        printf("Level II driver MQTT host set to %s\n", attribute.c_str());
                    }
                }
                else if (0 == vscp_strcasecmp(attr[i], "port")) {
                    pDriver->m_mqtt_port = vscp_readStringValue(attribute);
                    if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                        printf("Level II driver MQTT port set to %d\n", pDriver->m_mqtt_port);
                    }
                }
                else if (0 == vscp_strcasecmp(attr[i], "user")) {
                    pDriver->m_mqtt_strUserName = attribute;
                    if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                        printf("Level II driver MQTT username set to %s\n", attribute.c_str());
                    }
                }
                else if (0 == vscp_strcasecmp(attr[i], "password")) {
                    pDriver->m_mqtt_strPassword = attribute;
                    if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                        printf("Level II driver MQTT password set to *********\n");
                    }
                }
                else if (0 == vscp_strcasecmp(attr[i], "clientid")) {
                    pDriver->m_mqtt_strClientId = attribute;
                    if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
	                    printf("Level II driver MQTT ClientId set to %s\n",pObj->m_mqtt_strclientId.c_str());
                    }
                }
                else if (0 == vscp_strcasecmp(attr[i], "qos")) {
                    pDriver->m_mqtt_qos = vscp_readStringValue(attribute);
                    if (pDriver->m_mqtt_qos > 3) {
                        syslog(LOG_ERR,"Config: MQTT qos > 3. Set to 0.");
                        pDriver->m_mqtt_qos = 0;
                    }
                    if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                        printf("Level II driver MQTT qos set to %d\n", pDriver->m_mqtt_qos);
                    }
                }
                else if (0 == vscp_strcasecmp(attr[i], "bcleansession")) {
                    if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                        pDriver->m_mqtt_bCleanSession = true;
                    }
                    else {
                        pDriver->m_mqtt_bCleanSession = false;
                    }
                    if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                        printf("Level II driver MQTT bCleanSession set to %s\n", pDriver->m_mqtt_bCleanSession ? "true" : "false");
                    }
                }
                else if (0 == vscp_strcasecmp(attr[i], "bretain")) {
                    if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                        pDriver->m_mqtt_bRetain = true;
                    }
                    else {
                        pDriver->m_mqtt_bRetain = false;
                    }
                    if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                        printf("Level II driver MQTT bRetain set to %s\n", pDriver->m_mqtt_bRetain ? "true" : "false");
                    }
                }
                else if (0 == vscp_strcasecmp(attr[i], "keepalive")) {   
                    pDriver->m_mqtt_keepalive = vscp_readStringValue(attribute);
                    if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                        printf("Level II driver MQTT keepalive set to %d\n", pDriver->m_mqtt_keepalive);
                    }
                }
                else if (0 == vscp_strcasecmp(attr[i], "cafile")) {
                    if ( attribute.length() ) pDriver->m_mqtt_bTLS = true;
                    pDriver->m_mqtt_cafile = attribute;
                    if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                        printf("Level II driver MQTT cafile set to %s\n", attribute.c_str());
                    }
                }
                else if (0 == vscp_strcasecmp(attr[i], "capath")) {
                    if ( attribute.length() ) pDriver->m_mqtt_bTLS = true;
                    pDriver->m_mqtt_capath = attribute; 
                    if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                        printf("Level II driver MQTT capath set to %s\n", attribute.c_str());
                    }
                }
                else if (0 == vscp_strcasecmp(attr[i], "certfile")) {
                    if ( attribute.length() ) pDriver->m_mqtt_bTLS = true;
                    pDriver->m_mqtt_certfile = attribute; 
                    if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                        printf("Level II driver MQTT certfile set to %s\n", attribute.c_str());
                    }
                }
                else if (0 == vscp_strcasecmp(attr[i], "keyfile")) {
                    if ( attribute.length() ) pDriver->m_mqtt_bTLS = true;
                    pDriver->m_mqtt_keyfile = attribute; 
                    if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                        printf("Level II driver MQTT keyfile set to %s\n", attribute.c_str());
                    }
                }
                else if (0 == vscp_strcasecmp(attr[i], "pwkeyfile")) {
                    if ( attribute.length() ) pDriver->m_mqtt_bTLS = true;
                    pDriver->m_mqtt_pwKeyfile = attribute; 
                    if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                        printf("Level II driver MQTT pwkeyfile set to %s\n", attribute.c_str());
                    }
                }
                else if (0 == vscp_strcasecmp(attr[i], "format")) {
                    vscp_makeUpper(attribute);
                    if (0 == vscp_strcasecmp(attribute.c_str(), "JSON")) {
                        pDriver->m_mqtt_format = jsonfmt;
                        if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                            printf("Level II driver MQTT format set to 'JSON' (%s)\n",attribute.c_str());
                        }
                    }
                    else if (0 == vscp_strcasecmp(attribute.c_str(), "XML")) {
                        pDriver->m_mqtt_format = xmlfmt;
                        if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                            printf("Level II driver MQTT format set to 'XML' (%s)\n",attribute.c_str());
                        }
                    }
                    else if (0 == vscp_strcasecmp(attribute.c_str(), "STRING")) {
                        pDriver->m_mqtt_format = strfmt;
                        if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                            printf("Level II driver MQTT format set to 'STRING' (%s)\n",attribute.c_str());
                        }
                    }
                    else if (0 == vscp_strcasecmp(attribute.c_str(), "BINARY")) {
                        pDriver->m_mqtt_format = binfmt;
                        if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                            printf("Level II driver MQTT format set to 'BINARY' (%s)\n",attribute.c_str());
                        }
                    }
                    else {
                        pDriver->m_mqtt_format = jsonfmt;
                        if (pObj->m_debugFlags & VSCP_DEBUG_CONFIG) {
                            printf("Unknown format: Level II driver MQTT format set to 'JSON' (%s)\n",attribute.c_str());
                        }
                    }
                }
            } // for
        }
    }
    else if (bVscpConfigFound && bLevel2DriverConfigFound && bMQTTConfigFound &&
             (4 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "subscribe"))) {

        CDeviceItem *pDriver = pObj->m_deviceList.getDeviceItemFromName(m_currentDriverName);
        if ( NULL == pDriver ) {
            syslog(LOG_ERR,"ControlObject: ReadConfig: L1:Subscribe Can't get driver object %s", m_currentDriverName.c_str() );
        }
        else {
            for (int i = 0; attr[i]; i += 2) {

                std::string attribute = attr[i + 1];
                vscp_trim(attribute);

                if (0 == vscp_strcasecmp(attr[i], "topic")) {
                    pDriver->m_mqtt_subscriptions.push_back(attribute);
                }
            }
        }
    }
    else if (bVscpConfigFound && bLevel2DriverConfigFound && bMQTTConfigFound &&
             (4 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "publish"))) {

        CDeviceItem *pDriver = pObj->m_deviceList.getDeviceItemFromName(m_currentDriverName);
        if ( NULL == pDriver ) {
            syslog(LOG_ERR,"ControlObject: ReadConfig L1:Publish: Can't get driver object %s", m_currentDriverName.c_str() );
        }
        else {
            for (int i = 0; attr[i]; i += 2) {

                std::string attribute = attr[i + 1];
                vscp_trim(attribute);

                if (0 == vscp_strcasecmp(attr[i], "topic")) {
                    pDriver->m_mqtt_publish.push_back(attribute);
                }
            }
        }
    }


    depth_full_config_parser++;
}

static void
handleFullConfigData(void* data, const char* content, int length)
{
    ;
}

static void
endFullConfigParser(void* data, const char* name)
{
    CControlObject* pObj = (CControlObject*)data;

    depth_full_config_parser--;

    if (1 == depth_full_config_parser &&
        (0 == vscp_strcasecmp(name, "vscpconfig"))) {
        bVscpConfigFound = FALSE;
    }
    else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
                (0 == vscp_strcasecmp(name, "mqtt"))) {

        bMQTTConfigFound = FALSE;

        if (NULL != pObj) {
            // Set default subscription if none defined
            if ( !pObj->m_mqtt_subscriptions.size() ) {
                pObj->m_mqtt_subscriptions.push_back("vscp/{{guid}}/<-/#");
            }

            // Set default publish if none defined
            if ( !pObj->m_mqtt_publish.size() ) {
                pObj->m_mqtt_publish.push_back("vscp/{{guid}}/->/{{class}}/{{type}}");
            }
        }
    }
    else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
        ((0 == vscp_strcasecmp(name, "level1driver")) ||
         (0 == vscp_strcasecmp(name, "canal1driver")))) {
        bLevel1DriverConfigFound = FALSE;
    }
    else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "level2driver"))) {
        bLevel2DriverConfigFound = FALSE;
    }
    else if (bVscpConfigFound && (2 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "driver"))) {
        bDriverConfigFound = FALSE;
        m_currentDriverName = "";
    }
    else if (bVscpConfigFound && (3 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "mqtt"))) {
        bMQTTConfigFound = FALSE;
        if ( NULL != pObj ) {
            CDeviceItem *pDriver = pObj->m_deviceList.getDeviceItemFromName(m_currentDriverName);
            if ( NULL != pDriver ) {
                
                // Set default subscription if none defined
                if ( !pDriver->m_mqtt_subscriptions.size() ) {
                    pDriver->m_mqtt_subscriptions.push_back("vscp/{{guid}}/<-/#");
                }

                // Set default publish if none defined
                if ( !pDriver->m_mqtt_publish.size() ) {
                    pDriver->m_mqtt_publish.push_back("vscp/{{guid}}/->/{{class}}/{{type}}");
                }
            }
        }
    }

}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// readConfiguration
//
// Read the configuration XML file
//

bool
CControlObject::readConfiguration(const std::string& strcfgfile)
{
    FILE* fp;

    if (m_debugFlags & VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG,
               "Reading full XML configuration from [%s]",
               (const char*)strcfgfile.c_str());
    }

    fp = fopen(strcfgfile.c_str(), "r");
    if (NULL == fp) {
        syslog(LOG_ERR,
               "Failed to open configuration file [%s]",
               strcfgfile.c_str());
        return false;
    }

    XML_Parser xmlParser = XML_ParserCreate("UTF-8");
    XML_SetUserData(xmlParser, this);
    XML_SetElementHandler(xmlParser,
                          startFullConfigParser,
                          endFullConfigParser);
    XML_SetCharacterDataHandler(xmlParser, handleFullConfigData);

    void* buf = XML_GetBuffer(xmlParser, XML_BUFF_SIZE);
    if (NULL == buf) {
        XML_ParserFree(xmlParser);
        fclose(fp);
        syslog(LOG_ERR,
               "Failed to allocate buffer for configuration file [%s]",
               strcfgfile.c_str());
        return false;
    }

    size_t file_size = 0;
    file_size        = fread(buf, sizeof(char), XML_BUFF_SIZE, fp);

    if (!XML_ParseBuffer(xmlParser, file_size, file_size == 0)) {
        syslog(LOG_ERR, "Failed parse XML configuration file.");
        fclose(fp);
        XML_ParserFree(xmlParser);
        return false;
    }

    fclose(fp);
    XML_ParserFree(xmlParser);

    return true;

}   // XML config




