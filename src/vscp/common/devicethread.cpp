// devicethread.cpp
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

#define _POSIX

#include "devicethread.h"

#include <dlfcn.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syslog.h>
#include <unistd.h>

#ifndef DWORD
#define DWORD unsigned long
#endif

#include <canal_macro.h>
#include <level2drvdef.h>
#include <vscp.h>
#include <vscp_debug.h>
#include <vscphelper.h>
#include <devicelist.h>

#include <mosquitto.h>

#include <mustache.hpp>

using namespace kainjow::mustache;

//////////////////////////////////////////////////////////////////////
//                         Callbacks
//////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////
// mqtt_log_callback
//

static void mqtt_log_callback(struct mosquitto *mosq, void *pData, int level, const char *logmsg)
{
    // Check pointers
    if (NULL == mosq) return;
    if (NULL == pData) return;

    CDeviceItem *pItem = (CDeviceItem *)pData;

    printf("Driver LOG : %s\n", logmsg);
    if (pItem->m_debugFlags & VSCP_DEBUG_MQTT_LOG) {
        printf("Driver LOG : %s\n", logmsg);
    }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// mqtt_on_connect
//

static void mqtt_on_connect(struct mosquitto *mosq, void *pData, int rv)
{
    // Check pointers
    if (NULL == mosq) return;
    if (NULL == pData) return;

    CDeviceItem *pItem = (CDeviceItem *)pData;
    printf("\nDriver MQTT CONNECT\n");
    if (pItem->m_debugFlags & VSCP_DEBUG_MQTT_CONNECT) {
        printf("\nDriver MQTT CONNECT\n");
    }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// mqtt_on_disconnect
//

static void mqtt_on_disconnect(struct mosquitto *mosq, void *pData, int rv)
{
    // Check pointers
    if (NULL == mosq) return;
    if (NULL == pData) return;

    CDeviceItem *pItem = (CDeviceItem *)pData;
    if (pItem->m_debugFlags & VSCP_DEBUG_MQTT_CONNECT) {
        printf("\nDriver MQTT DISCONNECT\n");
    }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// mqtt_on_message
//

static void mqtt_on_message(struct mosquitto *mosq, void *pData, const struct mosquitto_message *pMsg)
{
    int rv;
    vscpEvent ev;

    // Check pointers
    if (NULL == mosq) return;
    if (NULL == pData) return;
    if (NULL == pMsg) return;

    CDeviceItem *pDeviceItem = (CDeviceItem *)pData;
    
    ev.sizeData = 0;
    ev.pdata = NULL;

    if (pDeviceItem->m_mqtt_format == jsonfmt) {
        std::string strPayload((const char *)pMsg->payload, pMsg->payloadlen);
        if ( !vscp_convertJSONToEvent(&ev, strPayload) ) {
            syslog(LOG_ERR, "ControlObject: sendEvent: Failed to convert event to JSON");
            return;
        }
    }
    else if (pDeviceItem->m_mqtt_format == xmlfmt) {
        std::string strPayload((const char *)pMsg->payload, pMsg->payloadlen);
        if ( !vscp_convertXMLToEvent(&ev, strPayload) ) {
            syslog(LOG_ERR, "ControlObject: sendEvent: Failed to convert event to XML");
            return;
        }
    }
    else if (pDeviceItem->m_mqtt_format == strfmt) {
        std::string strPayload((const char *)pMsg->payload, pMsg->payloadlen);
        if ( !vscp_convertStringToEvent(&ev, strPayload) ) {
            syslog(LOG_ERR, "ControlObject: sendEvent: Failed to convert event to STRING");
            return;
        }
    }
    else if (pDeviceItem->m_mqtt_format == binfmt) {
        if (!vscp_getEventFromFrame( &ev, (const uint8_t *)pMsg->payload, pMsg->payloadlen) ) {
            syslog(LOG_ERR, "ControlObject: sendEvent: Failed to convert event to BINARY");
            return;
        }
    }
    else {
        syslog(LOG_ERR, "ControlObject: sendEvent: Failed to convert event (invalid format)");
        return;
    }

    if (pDeviceItem->m_debugFlags & VSCP_DEBUG_MQTT_MSG) {

    }

    if (VSCP_DRIVER_LEVEL1 == pDeviceItem->m_driverLevel) {

        canalMsg msg;
        vscp_convertEventToCanal(&msg, &ev);

        // Use blocking method if available
        if (NULL == pDeviceItem->m_proc_CanalBlockingSend) {
            if (CANAL_ERROR_SUCCESS == (rv = pDeviceItem->m_proc_CanalBlockingSend(pDeviceItem->m_openHandle,
                                                        &msg,
                                                        300))) {
                syslog( LOG_ERR, "driver: mqtt_on_message - Failed to send event (m_proc_CanalBlockingSend) rv=%d", rv);
            }
        }
        else {
            if (CANAL_ERROR_SUCCESS != (rv = pDeviceItem->m_proc_CanalSend(pDeviceItem->m_openHandle, &msg))) {
                syslog( LOG_ERR, "driver: mqtt_on_message - Failed to send event (m_proc_CanalSend) rv=%d", rv);
            }
        }
    }
    else if (VSCP_DRIVER_LEVEL2 == pDeviceItem->m_driverLevel) {
        if (CANAL_ERROR_SUCCESS != pDeviceItem->m_proc_VSCPWrite(pDeviceItem->m_openHandle, &ev, 300)) {
            
        }
    }
    else {
        syslog( LOG_ERR, "driver: mqtt_on_message - Driver level is not valid (nor 1 nor 2)");
    }

}


static void mqtt_on_publish(struct mosquitto *mosq, void *pData, int rv)
{
    // Check pointers
    if (NULL == mosq) return;
    if (NULL == pData) return;

    CDeviceItem *pItem = (CDeviceItem *)pData;
    if (pItem->m_debugFlags & VSCP_DEBUG_MQTT_PUBLISH) {

    }
}



// ----------------------------------------------------------------------------




///////////////////////////////////////////////////////////////////////////////
// deviceThread
//
// New bahaviour
// =============
// subscribe to MQTT channel topic_subscribe
// publish to MQTT channel topic_publish
//

void*
deviceThread(void* pData)
{
    CDeviceItem* pDeviceItem = (CDeviceItem*)pData;
    if (NULL == pDeviceItem) {
        syslog(LOG_ERR, "No device item defined. Aborting device thread!");
        return NULL;
    }

    void* hdll;

    // Load dynamic library
    hdll = dlopen(pDeviceItem->m_strPath.c_str(), RTLD_LAZY);
    if (!hdll) {
        syslog(LOG_ERR,
               "Devicethread: Unable to load dynamic library. path = %s",
               pDeviceItem->m_strPath.c_str());
        return NULL;
    }

    //*************************************************************************
    //                         Level I drivers
    //*************************************************************************
    if (VSCP_DRIVER_LEVEL1 == pDeviceItem->m_driverLevel) {

        // Now find methods in library
        if (pDeviceItem->m_debugFlags & VSCP_DEBUG_DRIVERL1) {
            syslog(LOG_DEBUG,
                   "Loading level I driver: %s",
                   pDeviceItem->m_strName.c_str());
        }

        // * * * * CANAL OPEN * * * *
        pDeviceItem->m_proc_CanalOpen =
          (LPFNDLL_CANALOPEN)dlsym(hdll, "CanalOpen");
        const char* dlsym_error = dlerror();

        if (dlsym_error) {
            // Free the library
            syslog(LOG_ERR,
                   "%s : Unable to get dl entry for CanalOpen.",
                   pDeviceItem->m_strName.c_str());
            return NULL;
        }

        // * * * * CANAL CLOSE * * * *
        pDeviceItem->m_proc_CanalClose =
          (LPFNDLL_CANALCLOSE)dlsym(hdll, "CanalClose");
        dlsym_error = dlerror();
        if (dlsym_error) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for CanalClose.",
                   pDeviceItem->m_strName.c_str());
            dlclose(hdll);
            return NULL;
        }

        // * * * * CANAL GETLEVEL * * * *
        pDeviceItem->m_proc_CanalGetLevel =
          (LPFNDLL_CANALGETLEVEL)dlsym(hdll, "CanalGetLevel");
        dlsym_error = dlerror();
        if (dlsym_error) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for CanalGetLevel.",
                   pDeviceItem->m_strName.c_str());
            dlclose(hdll);
            return NULL;
        }

        // * * * * CANAL SEND * * * *
        pDeviceItem->m_proc_CanalSend =
          (LPFNDLL_CANALSEND)dlsym(hdll, "CanalSend");
        dlsym_error = dlerror();
        if (dlsym_error) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for CanalSend.",
                   pDeviceItem->m_strName.c_str());
            dlclose(hdll);
            return NULL;
        }

        // * * * * CANAL DATA AVAILABLE * * * *
        pDeviceItem->m_proc_CanalDataAvailable =
          (LPFNDLL_CANALDATAAVAILABLE)dlsym(hdll, "CanalDataAvailable");
        dlsym_error = dlerror();
        if (dlsym_error) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for CanalDataAvailable.",
                   pDeviceItem->m_strName.c_str());
            dlclose(hdll);
            return NULL;
        }

        // * * * * CANAL RECEIVE * * * *
        pDeviceItem->m_proc_CanalReceive =
          (LPFNDLL_CANALRECEIVE)dlsym(hdll, "CanalReceive");
        dlsym_error = dlerror();
        if (dlsym_error) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for CanalReceive.",
                   pDeviceItem->m_strName.c_str());
            dlclose(hdll);
            return NULL;
        }

        // * * * * CANAL GET STATUS * * * *
        pDeviceItem->m_proc_CanalGetStatus =
          (LPFNDLL_CANALGETSTATUS)dlsym(hdll, "CanalGetStatus");
        dlsym_error = dlerror();
        if (dlsym_error) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for CanalGetStatus.",
                   pDeviceItem->m_strName.c_str());
            dlclose(hdll);
            return NULL;
        }

        // * * * * CANAL GET STATISTICS * * * *
        pDeviceItem->m_proc_CanalGetStatistics =
          (LPFNDLL_CANALGETSTATISTICS)dlsym(hdll, "CanalGetStatistics");
        dlsym_error = dlerror();
        if (dlsym_error) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for CanalGetStatistics.",
                   pDeviceItem->m_strName.c_str());
            dlclose(hdll);
            return NULL;
        }

        // * * * * CANAL SET FILTER * * * *
        pDeviceItem->m_proc_CanalSetFilter =
          (LPFNDLL_CANALSETFILTER)dlsym(hdll, "CanalSetFilter");
        dlsym_error = dlerror();
        if (dlsym_error) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for CanalSetFilter.",
                   pDeviceItem->m_strName.c_str());
            dlclose(hdll);
            return NULL;
        }

        // * * * * CANAL SET MASK * * * *
        pDeviceItem->m_proc_CanalSetMask =
          (LPFNDLL_CANALSETMASK)dlsym(hdll, "CanalSetMask");
        dlsym_error = dlerror();
        if (dlsym_error) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for CanalSetMask.",
                   pDeviceItem->m_strName.c_str());
            dlclose(hdll);
            return NULL;
        }

        // * * * * CANAL GET VERSION * * * *
        pDeviceItem->m_proc_CanalGetVersion =
          (LPFNDLL_CANALGETVERSION)dlsym(hdll, "CanalGetVersion");
        dlsym_error = dlerror();
        if (dlsym_error) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for CanalGetVersion.",
                   pDeviceItem->m_strName.c_str());
            dlclose(hdll);
            return NULL;
        }

        // * * * * CANAL GET DLL VERSION * * * *
        pDeviceItem->m_proc_CanalGetDllVersion =
          (LPFNDLL_CANALGETDLLVERSION)dlsym(hdll, "CanalGetDllVersion");
        dlsym_error = dlerror();
        if (dlsym_error) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for CanalGetDllVersion.",
                   pDeviceItem->m_strName.c_str());
            dlclose(hdll);
            return NULL;
        }

        // * * * * CANAL GET VENDOR STRING * * * *
        pDeviceItem->m_proc_CanalGetVendorString =
          (LPFNDLL_CANALGETVENDORSTRING)dlsym(hdll, "CanalGetVendorString");
        dlsym_error = dlerror();
        if (dlsym_error) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for CanalGetVendorString.",
                   pDeviceItem->m_strName.c_str());
            dlclose(hdll);
            return NULL;
        }

        // ******************************
        //     Generation 2 Methods
        // ******************************

        // * * * * CANAL BLOCKING SEND * * * *
        pDeviceItem->m_proc_CanalBlockingSend =
          (LPFNDLL_CANALBLOCKINGSEND)dlsym(hdll, "CanalBlockingSend");
        dlsym_error = dlerror();
        if (dlsym_error) {
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for CanalBlockingSend. Probably "
                   "Generation 1 driver.",
                   pDeviceItem->m_strName.c_str());
            pDeviceItem->m_proc_CanalBlockingSend = NULL;
        }

        // * * * * CANAL BLOCKING RECEIVE * * * *
        pDeviceItem->m_proc_CanalBlockingReceive =
          (LPFNDLL_CANALBLOCKINGRECEIVE)dlsym(hdll, "CanalBlockingReceive");
        dlsym_error = dlerror();
        if (dlsym_error) {
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for CanalBlockingReceive. "
                   "Probably Generation 1 driver.",
                   pDeviceItem->m_strName.c_str());
            pDeviceItem->m_proc_CanalBlockingReceive = NULL;
        }

        // * * * * CANAL GET DRIVER INFO * * * *
        pDeviceItem->m_proc_CanalGetdriverInfo =
          (LPFNDLL_CANALGETDRIVERINFO)dlsym(hdll, "CanalGetDriverInfo");
        dlsym_error = dlerror();
        if (dlsym_error) {
            syslog(LOG_ERR,
                    "%s: Unable to get dl entry for CanalGetDriverInfo. "
                    "Probably Generation 1 driver.",
                    pDeviceItem->m_strName.c_str());
            pDeviceItem->m_proc_CanalGetdriverInfo = NULL;
        }

        // Open the device
        pDeviceItem->m_openHandle = pDeviceItem->m_proc_CanalOpen(
          (const char*)pDeviceItem->m_strParameter.c_str(),
          pDeviceItem->m_DeviceFlags);

        // Check if the driver opened properly
        if (pDeviceItem->m_openHandle <= 0) {
            syslog(LOG_ERR,
                   "Failed to open driver. Will not use it! %ld [%s] ",
                   pDeviceItem->m_openHandle,
                   pDeviceItem->m_strName.c_str());
            dlclose(hdll);
            return NULL;
        }

        if (pDeviceItem->m_debugFlags & VSCP_DEBUG_DRIVERL1) {
            syslog(LOG_DEBUG,
                   "%s: [Device tread] Level I Driver open.",
                   pDeviceItem->m_strName.c_str());
        }

        //--------------------------------------------------------------
        //                            MQTT
        // -------------------------------------------------------------

        if (pDeviceItem->m_mqtt_strClientId.length()) {
            pDeviceItem->m_mosq = mosquitto_new( pDeviceItem->m_mqtt_strClientId.c_str(), 
                                                    pDeviceItem->m_mqtt_bCleanSession, 
                                                    pDeviceItem);
        }
        else {
            pDeviceItem->m_mqtt_bCleanSession = true;    // Must be true without id
            pDeviceItem->m_mosq = mosquitto_new( NULL, 
                                                    pDeviceItem->m_mqtt_bCleanSession, 
                                                    pDeviceItem);
        }
        
        if (NULL == pDeviceItem->m_mosq) {
                
            if (ENOMEM == errno) {
                syslog(LOG_ERR, "Failed to create new mosquitto session (out of memory).");
            }
            else if (EINVAL == errno) {
                syslog(LOG_ERR, "Failed to create new mosquitto session (invalid parameters).");
            }

            dlclose(hdll);
            return NULL;

        }

        // Set callbacks
        mosquitto_log_callback_set(pDeviceItem->m_mosq, mqtt_log_callback);
        mosquitto_connect_callback_set(pDeviceItem->m_mosq, mqtt_on_connect);
        mosquitto_disconnect_callback_set(pDeviceItem->m_mosq, mqtt_on_disconnect);
        mosquitto_message_callback_set(pDeviceItem->m_mosq, mqtt_on_message);
        mosquitto_publish_callback_set(pDeviceItem->m_mosq, mqtt_on_publish);

        // Set username/password if defined
        if (pDeviceItem->m_mqtt_strUserName.length()) {
            int rv;
            if ( MOSQ_ERR_SUCCESS != (rv = mosquitto_username_pw_set( pDeviceItem->m_mosq,
                                                                        pDeviceItem->m_mqtt_strUserName.c_str(),
                                                                        pDeviceItem->m_mqtt_strPassword.c_str() ) ) ) {
                if (MOSQ_ERR_INVAL == rv) {
                    syslog(LOG_ERR, "Failed to set mosquitto username/password (invalid parameter(s)).");
                }
                else if (MOSQ_ERR_NOMEM == rv) {
                    syslog(LOG_ERR, "Failed to set mosquitto username/password (out of memory).");
                }
            }
        }

        int rv = mosquitto_connect(pDeviceItem->m_mosq,
                                        pDeviceItem->m_mqtt_strHost.c_str(),
                                        pDeviceItem->m_mqtt_port,
                                        pDeviceItem->m_mqtt_keepalive);

        if ( MOSQ_ERR_SUCCESS != rv ) {
            
            if (MOSQ_ERR_INVAL == rv) {
                syslog(LOG_ERR, "Failed to connect to mosquitto server (invalid parameter(s)).");
            }
            else if (MOSQ_ERR_ERRNO == rv) {
                syslog(LOG_ERR, "Failed to connect to mosquitto server. System returned error (errno = %d).", errno);
            }

            dlclose(hdll);
            return NULL;
        }

        // Start the worker loop
        rv = mosquitto_loop_start(pDeviceItem->m_mosq);
        if (MOSQ_ERR_SUCCESS != rv) {
            mosquitto_disconnect(pDeviceItem->m_mosq);
            dlclose(hdll);
            return NULL;
        }


        for (std::list<std::string>::const_iterator 
            it = pDeviceItem->m_mqtt_subscriptions.begin(); 
            it != pDeviceItem->m_mqtt_subscriptions.end(); 
            ++it){

            std::string topic = *it;

            // Fix subscribe/publish topics
            mustache subtemplate{topic};
            data data;
            data.set("guid", pDeviceItem->m_guid.getAsString());
            std::string subscribe_topic = subtemplate.render(data);

            // Subscribe to specified topic
            rv = mosquitto_subscribe(pDeviceItem->m_mosq,
                                        NULL,
                                        subscribe_topic.c_str(),
                                        pDeviceItem->m_mqtt_qos);

            switch (rv) {
                case MOSQ_ERR_INVAL:
                    syslog(LOG_ERR, "Failed to subscribed to specified topic [%s] - input parameters were invalid.",subscribe_topic.c_str());
                case MOSQ_ERR_NOMEM:
                    syslog(LOG_ERR, "Failed to subscribed to specified topic [%s] - out of memory condition occurred.",subscribe_topic.c_str());
                case MOSQ_ERR_NO_CONN:
                    syslog(LOG_ERR, "Failed to subscribed to specified topic [%s] - client isn’t connected to a broker.",subscribe_topic.c_str());
                case MOSQ_ERR_MALFORMED_UTF8:
                    syslog(LOG_ERR, "Failed to subscribed to specified topic [%s] - topic is not valid UTF-8.",subscribe_topic.c_str());
#if defined(MOSQ_ERR_OVERSIZE_PACKET)
                case MOSQ_ERR_OVERSIZE_PACKET:
                    syslog(LOG_ERR, "Failed to subscribed to specified topic [%s] - resulting packet would be larger than supported by the broker.",subscribe_topic.c_str());
#endif                    
            }
        }

        // -------------------------------------------------------------

        bool bActivity;

        // Get Driver Level
        pDeviceItem->m_driverLevel = pDeviceItem->m_proc_CanalGetLevel(pDeviceItem->m_openHandle);

        //  * * * Level I Driver * * *

        // Check if blocking driver is available
        if (NULL != pDeviceItem->m_proc_CanalBlockingReceive) {

            // * * * * Blocking version * * * *

            if (pDeviceItem->m_debugFlags & VSCP_DEBUG_DRIVERL1) {
                syslog(LOG_DEBUG, "%s: [Device tread] Level I blocking version.", pDeviceItem->m_strName.c_str());
            }

            /////////////////////////////////////////////////////////////////////////////
            //                      Device write worker thread
            /////////////////////////////////////////////////////////////////////////////


            // Wait for events or "the end"
            while (!pDeviceItem->m_bQuit) {

                canalMsg msg;
                vscpEvent ev;

                //int rx = mosquitto_loop(pDeviceItem->m_mosq, 0, 1);

                if (CANAL_ERROR_SUCCESS == pDeviceItem->m_proc_CanalBlockingReceive(pDeviceItem->m_openHandle,
                                                                                        &msg,
                                                                                        50)) {

                    // Publish to MQTT broker

                    memset(&ev, 0, sizeof(vscpEvent));

                    // Set driver GUID 
                    pDeviceItem->m_guid.writeGUID(ev.GUID);

                    // Convert CANAL message to VSCP event
                    if (!vscp_convertCanalToEvent(&ev, &msg, (unsigned char *)pDeviceItem->m_guid.getGUID()) ) {
                        syslog(LOG_ERR,"Driver L1: %s Failed to convet CANAL to event.\n", pDeviceItem->m_strName.c_str());
                        break;
                    }
                    ev.obid = 0;
                    ev.GUID[14] = 0;   // Make sure MSB of nickname is zero for Level I driver

                    // =========================================================
                    //                   Outgoing translations
                    // =========================================================

                    // Level I measurement events to Level II measurement float
                    if (pDeviceItem->m_translation & VSCP_DRIVER_OUT_TR_M1_M2F) {
                        vscp_convertLevel1MeasuremenToLevel2Double(&ev);
                    }

                    // Level I measurement events to Level II measurement string
                    if (pDeviceItem->m_translation & VSCP_DRIVER_OUT_TR_M1_M2S) {
                        vscp_convertLevel1MeasuremenToLevel2String(&ev);
                    }

                    // Level I events to Level I over Level II events
                    if (pDeviceItem->m_translation & VSCP_DRIVER_OUT_TR_ALL_L2) {
                        ev.vscp_class += 512;
                        uint8_t* p = new uint8_t[16 + ev.sizeData];
                        if (NULL != p) {
                            memset(p, 0, 16 + ev.sizeData);
                            memcpy(p + 16,
                                    ev.pdata,
                                    ev.sizeData);
                            ev.sizeData += 16;
                            delete[] ev.pdata;
                            ev.pdata = p;
                        }
                    }

                    if (!pDeviceItem->sendEvent(&ev) ) {
                        syslog(LOG_ERR,"Driver L1: %s Failed to send event to broker.\n", pDeviceItem->m_strName.c_str());
                        continue;
                    } 
                                                         
                }

            }

            // Signal worker threads to quit
            pDeviceItem->m_bQuit = true;

            if (pDeviceItem->m_debugFlags & VSCP_DEBUG_DRIVERL1) {
                syslog(LOG_DEBUG, "%s: [Device tread] Level I work loop ended.", pDeviceItem->m_strName.c_str());
            }

        }
        else {

            // * * * * Non blocking version * * * *

            if (pDeviceItem->m_debugFlags & VSCP_DEBUG_DRIVERL1) {
                syslog(LOG_DEBUG, "%s: [Device tread] Level I NON Blocking version.", pDeviceItem->m_strName.c_str());
            }
            
            while (!pDeviceItem->m_bQuit) {

                bActivity = false;

                /////////////////////////////////////////////////////////////////////////////
                //                           Receive from device
                /////////////////////////////////////////////////////////////////////////////
                canalMsg msg;
                if ( pDeviceItem->m_proc_CanalDataAvailable( pDeviceItem->m_openHandle ) ) {

                    if (CANAL_ERROR_SUCCESS == pDeviceItem->m_proc_CanalReceive(pDeviceItem->m_openHandle, &msg)) {

                        bActivity = true;

                        // Publish to MQTT broker

                        vscpEvent* pev = new vscpEvent;
                        if (NULL != pev) {

                            memset(pev, 0, sizeof(vscpEvent));

                            // Convert CANAL message to VSCP event
                            if (!vscp_convertCanalToEvent(pev, &msg, (unsigned char *)pDeviceItem->m_guid.getGUID())) {
                                syslog(LOG_ERR, "Driver L1: %s Failed to convet CANAL to event.\n", pDeviceItem->m_strName.c_str());
                                break;
                            }
                            pev->obid = 0;
                            //pev->GUID[14] = 0;   // Make sure high byte of nickname is zero for Level I driver

                            // =========================================================
                            //                   Outgoing translations
                            // =========================================================

                            // Level I measurement events to Level II measurement float
                            if (pDeviceItem->m_translation & VSCP_DRIVER_OUT_TR_M1_M2F) {
                                vscp_convertLevel1MeasuremenToLevel2Double(pev);
                            }

                            // Level I measurement events to Level II measurement string
                            if (pDeviceItem->m_translation & VSCP_DRIVER_OUT_TR_M1_M2S) {
                                vscp_convertLevel1MeasuremenToLevel2String(pev);
                            }

                            // Level I events to Level I over Level II events
                            if (pDeviceItem->m_translation & VSCP_DRIVER_OUT_TR_ALL_L2) {
                                pev->vscp_class += 512;
                                uint8_t* p = new uint8_t[16 + pev->sizeData];
                                if (NULL != p) {
                                    memset(p, 0, 16 + pev->sizeData);
                                    memcpy(p + 16,
                                            pev->pdata,
                                            pev->sizeData);
                                    pev->sizeData += 16;
                                    delete[] pev->pdata;
                                    pev->pdata = p;
                                }
                            }

                            if (!pDeviceItem->sendEvent(pev) ) {
                                syslog(LOG_ERR,"Driver L1: %s Failed to send event to broker.\n", pDeviceItem->m_strName.c_str());
                                if (NULL == pev) {
                                    vscp_deleteEvent_v2(&pev);
                                }
                                continue;
                            } 

                            if (NULL == pev) {
                                vscp_deleteEvent_v2(&pev);
                            }
                        }
                        else {
                            syslog(LOG_ERR,"Driver L1: %s Memory problem.\n", pDeviceItem->m_strName.c_str());
                            break;
                        }
                        
                    }
                } // data available

                if (!bActivity) {
                     usleep(100000); // 100 ms
                }

                bActivity = false;

            } // while working - non blocking

        } // if blocking/non blocking

        if (pDeviceItem->m_debugFlags & VSCP_DEBUG_DRIVERL1) {
            syslog(LOG_DEBUG,
                   "%s: [Device tread] Level I Work loop ended.",
                   pDeviceItem->m_strName.c_str());
        }

        // Close CANAL channel
        pDeviceItem->m_proc_CanalClose(pDeviceItem->m_openHandle);

        if (pDeviceItem->m_debugFlags & VSCP_DEBUG_DRIVERL1) {
            syslog(LOG_DEBUG,
                   "%s: [Device tread] Level I Closed.",
                   pDeviceItem->m_strName.c_str());
        }

        pDeviceItem->m_bQuit = true;

        mosquitto_disconnect(pDeviceItem->m_mosq);

        dlclose(hdll);
    }

    //*************************************************************************
    //                         Level II drivers
    //*************************************************************************

    else if (VSCP_DRIVER_LEVEL2 == pDeviceItem->m_driverLevel) {

        // Now find methods in library
        syslog(LOG_INFO,
               "Loading level II driver: <%s>",
               pDeviceItem->m_strName.c_str());

        // * * * * VSCP OPEN * * * *
        if (NULL == (pDeviceItem->m_proc_VSCPOpen =
                       (LPFNDLL_VSCPOPEN)dlsym(hdll, "VSCPOpen"))) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for VSCPOpen.",
                   pDeviceItem->m_strName.c_str());
            return NULL;
        }

        // * * * * VSCP CLOSE * * * *
        if (NULL == (pDeviceItem->m_proc_VSCPClose =
                       (LPFNDLL_VSCPCLOSE)dlsym(hdll, "VSCPClose"))) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for VSCPClose.",
                   pDeviceItem->m_strName.c_str());
            return NULL;
        }

        // * * * * VSCPWRITE * * * *
        if (NULL == (pDeviceItem->m_proc_VSCPWrite =
                       (LPFNDLL_VSCPWRITE)dlsym(hdll, "VSCPWrite"))) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for VSCPWrite.",
                   pDeviceItem->m_strName.c_str());
            return NULL;
        }

        // * * * * VSCPREAD * * * *
        if (NULL == (pDeviceItem->m_proc_VSCPRead =
                       (LPFNDLL_VSCPREAD)dlsym(hdll, "VSCPRead"))) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for VSCPBlockingReceive.",
                   pDeviceItem->m_strName.c_str());
            return NULL;
        }

        // * * * * VSCP GET VERSION * * * *
        if (NULL == (pDeviceItem->m_proc_VSCPGetVersion =
                       (LPFNDLL_VSCPGETVERSION)dlsym(hdll, "VSCPGetVersion"))) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for VSCPGetVersion.",
                   pDeviceItem->m_strName.c_str());
            return NULL;
        }

        if (pDeviceItem->m_debugFlags & VSCP_DEBUG_DRIVERL2) {
            syslog(LOG_DEBUG,
                   "%s: Discovered all methods\n",
                   pDeviceItem->m_strName.c_str());
        }


        //--------------------------------------------------------------
        //                        MQTT Level II
        // -------------------------------------------------------------

        if (pDeviceItem->m_mqtt_strClientId.length()) {
            pDeviceItem->m_mosq = mosquitto_new( pDeviceItem->m_mqtt_strClientId.c_str(), 
                                                    pDeviceItem->m_mqtt_bCleanSession, 
                                                    pDeviceItem);
        }
        else {
            pDeviceItem->m_mqtt_bCleanSession = true;    // Must be true without id
            pDeviceItem->m_mosq = mosquitto_new( NULL, 
                                                    pDeviceItem->m_mqtt_bCleanSession, 
                                                    pDeviceItem);
        }
        
        if (NULL == pDeviceItem->m_mosq) {
                
            if (ENOMEM == errno) {
                syslog(LOG_ERR, "Failed to create new mosquitto session (out of memory).");
            }
            else if (EINVAL == errno) {
                syslog(LOG_ERR, "Failed to create new mosquitto session (invalid parameters).");
            }

            dlclose(hdll);
            return NULL;

        }

        // Set callbacks
        mosquitto_log_callback_set(pDeviceItem->m_mosq, mqtt_log_callback);
        mosquitto_connect_callback_set(pDeviceItem->m_mosq, mqtt_on_connect);
        mosquitto_disconnect_callback_set(pDeviceItem->m_mosq, mqtt_on_disconnect);
        mosquitto_message_callback_set(pDeviceItem->m_mosq, mqtt_on_message);
        mosquitto_publish_callback_set(pDeviceItem->m_mosq, mqtt_on_publish);

        // Set username/password if defined
        if (pDeviceItem->m_mqtt_strUserName.length()) {
            int rv;
            if ( MOSQ_ERR_SUCCESS != (rv = mosquitto_username_pw_set( pDeviceItem->m_mosq,
                                                                        pDeviceItem->m_mqtt_strUserName.c_str(),
                                                                        pDeviceItem->m_mqtt_strPassword.c_str() ) ) ) {
                if (MOSQ_ERR_INVAL == rv) {
                    syslog(LOG_ERR, "Failed to set mosquitto username/password (invalid parameter(s)).");
                }
                else if (MOSQ_ERR_NOMEM == rv) {
                    syslog(LOG_ERR, "Failed to set mosquitto username/password (out of memory).");
                }
            }
        }

        int rv = mosquitto_connect(pDeviceItem->m_mosq,
                                        pDeviceItem->m_mqtt_strHost.c_str(),
                                        pDeviceItem->m_mqtt_port,
                                        pDeviceItem->m_mqtt_keepalive);

        if ( MOSQ_ERR_SUCCESS != rv ) {
            
            if (MOSQ_ERR_INVAL == rv) {
                syslog(LOG_ERR, "Failed to connect to mosquitto server (invalid parameter(s)).");
            }
            else if (MOSQ_ERR_ERRNO == rv) {
                syslog(LOG_ERR, "Failed to connect to mosquitto server. System returned error (errno = %d).", errno);
            }

            dlclose(hdll);
            return NULL;
        }

        // Start the worker loop
        rv = mosquitto_loop_start(pDeviceItem->m_mosq);
        if (MOSQ_ERR_SUCCESS != rv) {
            mosquitto_disconnect(pDeviceItem->m_mosq);
            dlclose(hdll);
            return NULL;
        }


        for (std::list<std::string>::const_iterator 
            it = pDeviceItem->m_mqtt_subscriptions.begin(); 
            it != pDeviceItem->m_mqtt_subscriptions.end(); 
            ++it){

            std::string topic = *it;

            // Fix subscribe/publish topics
            mustache subtemplate{topic};
            data data;
            data.set("guid", pDeviceItem->m_guid.getAsString());
            std::string subscribe_topic = subtemplate.render(data);

            // Subscribe to specified topic
            rv = mosquitto_subscribe(pDeviceItem->m_mosq,
                                        NULL,
                                        subscribe_topic.c_str(),
                                        pDeviceItem->m_mqtt_qos);

            switch (rv) {
                case MOSQ_ERR_INVAL:
                    syslog(LOG_ERR, "Failed to subscribed to specified topic [%s] - input parameters were invalid.",subscribe_topic.c_str());
                case MOSQ_ERR_NOMEM:
                    syslog(LOG_ERR, "Failed to subscribed to specified topic [%s] - out of memory condition occurred.",subscribe_topic.c_str());
                case MOSQ_ERR_NO_CONN:
                    syslog(LOG_ERR, "Failed to subscribed to specified topic [%s] - client isn’t connected to a broker.",subscribe_topic.c_str());
                case MOSQ_ERR_MALFORMED_UTF8:
                    syslog(LOG_ERR, "Failed to subscribed to specified topic [%s] - topic is not valid UTF-8.",subscribe_topic.c_str());
#if defined(MOSQ_ERR_OVERSIZE_PACKET)
                case MOSQ_ERR_OVERSIZE_PACKET:
                    syslog(LOG_ERR, "Failed to subscribed to specified topic [%s] - resulting packet would be larger than supported by the broker.",subscribe_topic.c_str());
#endif                    
            }
        }

        // -------------------------------------------------------------



        // Open up the driver
        pDeviceItem->m_openHandle =
          pDeviceItem->m_proc_VSCPOpen(pDeviceItem->m_strParameter.c_str(),
                                    pDeviceItem->m_guid.getGUID());

        if (0 == pDeviceItem->m_openHandle) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: [Device tread] Unable to open VSCP "
                   " level II driver (path, config file access rights)."
                   " There may be additional info from driver "
                   "in syslog. If not enable debug flag in drivers config file",
                   pDeviceItem->m_strName.c_str());
            return NULL;
        }

        if (pDeviceItem->m_debugFlags & VSCP_DEBUG_DRIVERL2) {
            syslog(LOG_DEBUG,
                   "%s: [Device tread] Level II Open.",
                   pDeviceItem->m_strName.c_str());
        }


        // --------------------------------------------------------------------
        //        Work loop - receive from device - send to MQTT broker
        // --------------------------------------------------------------------

        // Just sit and wait until the end of the world as we know it...
        while (!pDeviceItem->m_bQuit) {

            vscpEvent ev;
            memset(&ev, 0, sizeof(vscpEvent));

            if ( CANAL_ERROR_SUCCESS != pDeviceItem->m_proc_VSCPRead(pDeviceItem->m_openHandle, &ev, 500) ) {
                continue;
            }

            // If timestamp is zero we set it here
            if (0 == ev.timestamp) {
                ev.timestamp = vscp_makeTimeStamp();
            }

            // Publish to MQTT broker

            if (!pDeviceItem->sendEvent(&ev) ) {
                syslog(LOG_ERR,"Driver L2: %s Failed to send event to broker.\n", pDeviceItem->m_strName.c_str());
                continue;
            } 

        }

        if (pDeviceItem->m_debugFlags & VSCP_DEBUG_DRIVERL2) {
            syslog(LOG_DEBUG,
                   "%s: [Device tread] Level II Closing.",
                   pDeviceItem->m_strName.c_str());
        }

        // Close channel
        pDeviceItem->m_proc_VSCPClose(pDeviceItem->m_openHandle);

        if (pDeviceItem->m_debugFlags & VSCP_DEBUG_DRIVERL2) {
            syslog(LOG_DEBUG,
                   "%s: [Device tread] Level II Closed.",
                   pDeviceItem->m_strName.c_str());
        }

        pDeviceItem->m_bQuit = true;
        mosquitto_disconnect(pDeviceItem->m_mosq);

        // Unload dll
        dlclose(hdll);

        if (pDeviceItem->m_debugFlags & VSCP_DEBUG_DRIVERL2) {
            syslog(LOG_DEBUG,
                   "%s: [Device tread] Level II Done waiting for threads.",
                   pDeviceItem->m_strName.c_str());
        }
    }

    return NULL;
}

