// devicethread.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2021 Ake Hedman, the VSCP project
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

#ifdef WIN32
#include <stdAfx.h>
#endif

#ifndef DWORD
#define DWORD unsigned long
#endif

#ifndef WIN32
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/wait.h>
#endif

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "devicethread.h"

#include <canal_macro.h>
#include <level2drvdef.h>
#include <vscp.h>
#include <vscp_debug.h>
#include <vscphelper.h>
#include <devicelist.h>
#include <controlobject.h>

#include <mosquitto.h>
#include <mustache.hpp>
using namespace kainjow::mustache;

// From vscp.cpp 
extern uint8_t *__vscp_key; // (256 bits)
extern uint64_t gDebugLevel;


//////////////////////////////////////////////////////////////////////
//                         Callbacks
//////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////
// mqtt_drv_log_callback
//

static void mqtt_drv_log_callback(struct mosquitto *mosq, void *pData, int level, const char *logmsg)
{
    // Check pointers
    if (NULL == mosq) return;
    if (NULL == pData) return;

    CDeviceItem *pDeviceItem = (CDeviceItem *)pData;

    if (gDebugLevel & VSCP_DEBUG_MQTT_LOG) {
        char buf[80];
        time_t tm;
        time(&tm);
        vscp_getTimeString(buf, sizeof(buf), &tm);
        spdlog::get("logger")->trace("Driver {} MQTT log : {}",  pDeviceItem->m_strName.c_str(), logmsg);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// mqtt_drv_on_connect
//

static void mqtt_drv_on_connect(struct mosquitto *mosq, void *pData, int rv)
{
    // Check pointers
    if (NULL == mosq) return;
    if (NULL == pData) return;

    CDeviceItem *pDeviceItem = (CDeviceItem *)pData;
    if (gDebugLevel & VSCP_DEBUG_MQTT_CONNECT) {
        spdlog::get("logger")->trace("Driver {} MQTT connect", pDeviceItem->m_strName);
    }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// mqtt_drv_on_disconnect
//

static void mqtt_drv_on_disconnect(struct mosquitto *mosq, void *pData, int rv)
{
    // Check pointers
    if (NULL == mosq) return;
    if (NULL == pData) return;

    CDeviceItem *pDeviceItem = (CDeviceItem *)pData;
    if (gDebugLevel  & VSCP_DEBUG_MQTT_CONNECT) {
        spdlog::get("logger")->trace("Driver {} MQTT disconnect", pDeviceItem->m_strName);
    }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// mqtt_drv_on_message
//

static void mqtt_drv_on_message(struct mosquitto *mosq, void *pData, const struct mosquitto_message *pMsg)
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
        printf("DRV: Message: [topic = %s]: Payload: %s\n", pMsg->topic, strPayload.c_str());
        if (!vscp_convertJSONToEvent(&ev, strPayload)) {
            // This is not a JSON formated event - we skip it
            if (gDebugLevel & VSCP_DEBUG_MQTT_MSG) {
                spdlog::get("logger")->error("Driver: {}: Failed to convert event to JSON", 
                            pDeviceItem->m_strName);
            }
            return;
        }
    } else if (pDeviceItem->m_mqtt_format == xmlfmt) {
        std::string strPayload((const char *)pMsg->payload, pMsg->payloadlen);
        if ( !vscp_convertXMLToEvent(&ev, strPayload) ) {
            if (gDebugLevel & VSCP_DEBUG_MQTT_MSG) {
                spdlog::get("logger")->error("Driver: {}: Failed to convert event to XML", 
                            pDeviceItem->m_strName);
            }
            return;
        }
    }
    else if (pDeviceItem->m_mqtt_format == strfmt) {
        std::string strPayload((const char *)pMsg->payload, pMsg->payloadlen);
        if ( !vscp_convertStringToEvent(&ev, strPayload) ) {
            if (gDebugLevel & VSCP_DEBUG_MQTT_MSG) {
                spdlog::get("logger")->error("Driver: {}: Failed to convert event to STRING", 
                            pDeviceItem->m_strName);
            }
            return;
        }
    }
    else if (pDeviceItem->m_mqtt_format == binfmt) {
        if (!vscp_getEventFromFrame( &ev, (const uint8_t *)pMsg->payload, pMsg->payloadlen) ) {
            if (gDebugLevel & VSCP_DEBUG_MQTT_MSG) {
                spdlog::get("logger")->error("Driver: {}: Failed to convert event to BINARY", 
                            pDeviceItem->m_strName);
            }
            return;
        }
    }
    else {
        spdlog::get("logger")->error("Driver: {}: Failed to convert event (invalid format)", 
                            pDeviceItem->m_strName);
        return;
    }

    if (VSCP_DRIVER_LEVEL1 == pDeviceItem->m_driverLevel) {

        canalMsg msg;
        vscp_convertEventToCanal(&msg, &ev);

        // Use blocking method if available
        if (NULL == pDeviceItem->m_proc_CanalBlockingSend) {
            if (CANAL_ERROR_SUCCESS == (rv = pDeviceItem->m_proc_CanalBlockingSend(pDeviceItem->m_openHandle,
                                                        &msg,
                                                        300))) {
                spdlog::get("logger")->error("driver: {}: mqtt_on_message - Failed to send event (m_proc_CanalBlockingSend) rv={1}", 
                            pDeviceItem->m_strName.c_str(), rv);
            }
        }
        else {
            if (CANAL_ERROR_SUCCESS != (rv = pDeviceItem->m_proc_CanalSend(pDeviceItem->m_openHandle, &msg))) {
                spdlog::get("logger")->error("driver: {}: mqtt_on_message - Failed to send event (m_proc_CanalSend) rv={1}", 
                            pDeviceItem->m_strName.c_str(), rv);
            }
        }

    }
    else if (VSCP_DRIVER_LEVEL2 == pDeviceItem->m_driverLevel) {
        if (CANAL_ERROR_SUCCESS != pDeviceItem->m_proc_VSCPWrite(pDeviceItem->m_openHandle, &ev, 500)) {
            spdlog::get("logger")->error("driver: mqtt_on_message - Failed to send level II event.");
        }
    }
    else {
        spdlog::get("logger")->error("driver: mqtt_on_message - Driver level is not valid (nor 1 nor 2).");
    }

}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// mqtt_drv_on_publish
//

static void mqtt_drv_on_publish(struct mosquitto *mosq, void *pData, int rv)
{
    // Check pointers
    if (NULL == mosq) return;
    if (NULL == pData) return;

    //CDeviceItem *pItem = (CDeviceItem *)pData;
    if (gDebugLevel  & VSCP_DEBUG_MQTT_PUBLISH) {
        spdlog::get("logger")->debug("Driver: MQTT Publish");
    }
}




// ----------------------------------------------------------------------------


#ifdef WIN32
static void usleep(__int64 usec) 
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
        spdlog::get("logger")->error("No device item defined. Aborting device thread!");
        return NULL;
    }

    void* hdll;

    // Load dynamic library
    hdll = dlopen(pDeviceItem->m_strPath.c_str(), RTLD_LAZY);
    if (!hdll) {
        spdlog::get("logger")->error(
               "Devicethread: Unable to load dynamic library. path = {}  {}",
               pDeviceItem->m_strPath,
               dlerror() );
        return NULL;
    }

    //*************************************************************************
    //                         Level I drivers
    //*************************************************************************
    if (VSCP_DRIVER_LEVEL1 == pDeviceItem->m_driverLevel) {

        // Now find methods in library
        if (gDebugLevel  & VSCP_DEBUG_DRIVERL1) {
            spdlog::get("logger")->debug(
                   "Loading level I driver: {}",
                   pDeviceItem->m_strName);
        }

        // * * * * CANAL OPEN * * * *
        pDeviceItem->m_proc_CanalOpen =
          (LPFNDLL_CANALOPEN)dlsym(hdll, "CanalOpen");
        const char* dlsym_error = dlerror();

        if (dlsym_error) {
            // Free the library
            spdlog::get("logger")->critical(
                   "{} : Unable to get dl entry for CanalOpen.",
                   pDeviceItem->m_strName);
            return NULL;
        }

        // * * * * CANAL CLOSE * * * *
        pDeviceItem->m_proc_CanalClose =
          (LPFNDLL_CANALCLOSE)dlsym(hdll, "CanalClose");
        dlsym_error = dlerror();
        if (dlsym_error) {
            // Free the library
            spdlog::get("logger")->critical(
                   "{}: Unable to get dl entry for CanalClose.",
                   pDeviceItem->m_strName);
            dlclose(hdll);
            return NULL;
        }

        // * * * * CANAL GETLEVEL * * * *
        pDeviceItem->m_proc_CanalGetLevel =
          (LPFNDLL_CANALGETLEVEL)dlsym(hdll, "CanalGetLevel");
        dlsym_error = dlerror();
        if (dlsym_error) {
            // Free the library
            spdlog::get("logger")->critical(
                   "{}: Unable to get dl entry for CanalGetLevel.",
                   pDeviceItem->m_strName);
            dlclose(hdll);
            return NULL;
        }

        // * * * * CANAL SEND * * * *
        pDeviceItem->m_proc_CanalSend =
          (LPFNDLL_CANALSEND)dlsym(hdll, "CanalSend");
        dlsym_error = dlerror();
        if (dlsym_error) {
            // Free the library
            spdlog::get("logger")->critical(
                   "{}: Unable to get dl entry for CanalSend.",
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
            spdlog::get("logger")->critical(
                   "{}: Unable to get dl entry for CanalDataAvailable.",
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
            spdlog::get("logger")->critical(
                   "{}: Unable to get dl entry for CanalReceive.",
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
            spdlog::get("logger")->critical(
                   "{}: Unable to get dl entry for CanalGetStatus.",
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
            spdlog::get("logger")->critical(
                   "{}: Unable to get dl entry for CanalGetStatistics.",
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
            spdlog::get("logger")->critical(
                   "{}: Unable to get dl entry for CanalSetFilter.",
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
            spdlog::get("logger")->critical(
                   "{}: Unable to get dl entry for CanalSetMask.",
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
            spdlog::get("logger")->critical(
                   "{}: Unable to get dl entry for CanalGetVersion.",
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
            spdlog::get("logger")->critical(
                   "{}: Unable to get dl entry for CanalGetDllVersion.",
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
            spdlog::get("logger")->critical(
                   "{}: Unable to get dl entry for CanalGetVendorString.",
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
            spdlog::get("logger")->error(
                   "{}: Unable to get dl entry for CanalBlockingSend. Probably "
                   "Generation 1 driver.",
                   pDeviceItem->m_strName.c_str());
            pDeviceItem->m_proc_CanalBlockingSend = NULL;
        }

        // * * * * CANAL BLOCKING RECEIVE * * * *
        pDeviceItem->m_proc_CanalBlockingReceive =
          (LPFNDLL_CANALBLOCKINGRECEIVE)dlsym(hdll, "CanalBlockingReceive");
        dlsym_error = dlerror();
        if (dlsym_error) {
            spdlog::get("logger")->error(
                   "{}: Unable to get dl entry for CanalBlockingReceive. "
                   "Probably Generation 1 driver.",
                   pDeviceItem->m_strName.c_str());
            pDeviceItem->m_proc_CanalBlockingReceive = NULL;
        }

        // * * * * CANAL GET DRIVER INFO * * * *
        pDeviceItem->m_proc_CanalGetdriverInfo =
          (LPFNDLL_CANALGETDRIVERINFO)dlsym(hdll, "CanalGetDriverInfo");
        dlsym_error = dlerror();
        if (dlsym_error) {
            spdlog::get("logger")->error(
                    "{}: Unable to get dl entry for CanalGetDriverInfo. "
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
            spdlog::get("logger")->error(
                   "Failed to open driver. Will not use it! {} [{}] ",
                   pDeviceItem->m_openHandle,
                   pDeviceItem->m_strName);
            dlclose(hdll);
            return NULL;
        }

        if (gDebugLevel  & VSCP_DEBUG_DRIVERL1) {
            spdlog::get("logger")->debug(
                   "%s: [Device tread] Level I Driver open.",
                   pDeviceItem->m_strName.c_str());
        }

        //--------------------------------------------------------------
        //                       MQTT Level I
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
                spdlog::get("logger")->error("Failed to create new mosquitto session (out of memory).");
            }
            else if (EINVAL == errno) {
                spdlog::get("logger")->error("Failed to create new mosquitto session (invalid parameters).");
            }

            dlclose(hdll);
            return NULL;

        }

        // Set callbacks
        mosquitto_log_callback_set(pDeviceItem->m_mosq, mqtt_drv_log_callback);
        mosquitto_connect_callback_set(pDeviceItem->m_mosq, mqtt_drv_on_connect);
        mosquitto_disconnect_callback_set(pDeviceItem->m_mosq, mqtt_drv_on_disconnect);
        mosquitto_message_callback_set(pDeviceItem->m_mosq, mqtt_drv_on_message);
        mosquitto_publish_callback_set(pDeviceItem->m_mosq, mqtt_drv_on_publish);

        // Set username/password if defined
        if (pDeviceItem->m_mqtt_strUserName.length()) {
            int rv;
            if ( MOSQ_ERR_SUCCESS != (rv = mosquitto_username_pw_set( pDeviceItem->m_mosq,
                                                                        pDeviceItem->m_mqtt_strUserName.c_str(),
                                                                        pDeviceItem->m_mqtt_strPassword.c_str() ) ) ) {
                if (MOSQ_ERR_INVAL == rv) {
                    spdlog::get("logger")->error("Failed to set mosquitto username/password (invalid parameter(s)).");
                }
                else if (MOSQ_ERR_NOMEM == rv) {
                    spdlog::get("logger")->error("Failed to set mosquitto username/password (out of memory).");
                }
            }
        }

        int rv = mosquitto_connect(pDeviceItem->m_mosq,
                                        pDeviceItem->m_mqtt_strHost.c_str(),
                                        pDeviceItem->m_mqtt_port,
                                        pDeviceItem->m_mqtt_keepalive);

        if ( MOSQ_ERR_SUCCESS != rv ) {
            
            if (MOSQ_ERR_INVAL == rv) {
                spdlog::get("logger")->error("Failed to connect to mosquitto server (invalid parameter(s)).");
            }
            else if (MOSQ_ERR_ERRNO == rv) {
                spdlog::get("logger")->error("Failed to connect to mosquitto server. System returned error (errno = %d).", errno);
            }

            dlclose(hdll);
            return NULL;
        }

        //mosquitto_threaded_set(pDeviceItem->m_mosq, true);

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
                    spdlog::get("logger")->error("Failed to subscribed to specified topic [{}] - input parameters were invalid.",
                        subscribe_topic.c_str());
                case MOSQ_ERR_NOMEM:
                    spdlog::get("logger")->error("Failed to subscribed to specified topic [{}] - out of memory condition occurred.",
                        subscribe_topic.c_str());
                case MOSQ_ERR_NO_CONN:
                    spdlog::get("logger")->error("Failed to subscribed to specified topic [{}] - client isn’t connected to a broker.",
                        subscribe_topic.c_str());
                case MOSQ_ERR_MALFORMED_UTF8:
                    spdlog::get("logger")->error("Failed to subscribed to specified topic [{}] - topic is not valid UTF-8.",
                        subscribe_topic.c_str());
#if defined(MOSQ_ERR_OVERSIZE_PACKET)
                case MOSQ_ERR_OVERSIZE_PACKET:
                    spdlog::get("logger")->error("Failed to subscribed to specified topic [{}] - resulting packet would be larger than supported by the broker.",
                        subscribe_topic.c_str());
#endif                    
            }
        }

        // -------------------------------------------------------------

        bool bActivity;

        // Get Driver Level
        pDeviceItem->m_driverLevel = (uint8_t)pDeviceItem->m_proc_CanalGetLevel(pDeviceItem->m_openHandle);

        //  * * * Level I Driver * * *

        // Check if blocking driver is available
        if (NULL != pDeviceItem->m_proc_CanalBlockingReceive) {

            // * * * * Blocking version * * * *

            if (gDebugLevel  & VSCP_DEBUG_DRIVERL1) {
                spdlog::get("logger")->debug("{}: [Device tread] Level I blocking version.", 
                        pDeviceItem->m_strName.c_str());
            }

            /////////////////////////////////////////////////////////////////////////////
            //                      Device write worker thread
            /////////////////////////////////////////////////////////////////////////////


            // Wait for events or "the end"
            while (!pDeviceItem->m_bQuit) {

                canalMsg msg;
                vscpEvent ev;

                //int rx = mosquitto_loop(pDeviceItem->m_mosq, 10, 1);
                if (CANAL_ERROR_SUCCESS == pDeviceItem->m_proc_CanalBlockingReceive(pDeviceItem->m_openHandle,
                                                                                        &msg,
                                                                                        50)) {

                    // Publish to MQTT broker

                    memset(&ev, 0, sizeof(vscpEvent));

                    // Set driver GUID 
                    pDeviceItem->m_guid.writeGUID(ev.GUID);

                    // Convert CANAL message to VSCP event
                    if (!vscp_convertCanalToEvent(&ev, &msg, (unsigned char *)pDeviceItem->m_guid.getGUID()) ) {
                        spdlog::get("logger")->error("Driver L1: {} Failed to convet CANAL to event.", 
                                pDeviceItem->m_strName);
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
                        spdlog::get("logger")->error("Driver L1: {} Failed to send event to broker.", 
                                pDeviceItem->m_strName);
                        continue;
                    } 
                                                         
                }

            }

            // Signal worker threads to quit
            pDeviceItem->m_bQuit = true;

            if (gDebugLevel  & VSCP_DEBUG_DRIVERL1) {
                spdlog::get("logger")->error("{}: [Device tread] Level I work loop ended.", 
                        pDeviceItem->m_strName);
            }

        }
        else {

            // * * * * Non blocking version * * * *

            if (gDebugLevel  & VSCP_DEBUG_DRIVERL1) {
                spdlog::get("logger")->error("{}: [Device tread] Level I NON Blocking version.", 
                        pDeviceItem->m_strName);
            }
            
            while (!pDeviceItem->m_bQuit) {

                bActivity = false;
                
                //int rx = mosquitto_loop(pDeviceItem->m_mosq, 10, 1);

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
                                spdlog::get("logger")->error("Driver L1: {} Failed to convet CANAL to event.", 
                                        pDeviceItem->m_strName);
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
                                spdlog::get("logger")->error("Driver L1: {} Failed to send event to broker.", 
                                        pDeviceItem->m_strName);
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
                            spdlog::get("logger")->error("Driver L1: {} Memory problem.\n", 
                                    pDeviceItem->m_strName);
                            break;
                        }
                        
                    }
                } // data available

                if (!bActivity) {
                     usleep(100000); // 100 ms
                     //int rx = mosquitto_loop(pDeviceItem->m_mosq, 100, 1);
                }

                bActivity = false;

            } // while working - non blocking

        } // if blocking/non blocking

        if (gDebugLevel  & VSCP_DEBUG_DRIVERL1) {
            spdlog::get("logger")->debug(
                   "{}: [Device tread] Level I Work loop ended.",
                   pDeviceItem->m_strName);
        }

        // Close CANAL channel
        pDeviceItem->m_proc_CanalClose(pDeviceItem->m_openHandle);

        if (gDebugLevel  & VSCP_DEBUG_DRIVERL1) {
            spdlog::get("logger")->debug(
                   "{}: [Device tread] Level I Closed.",
                   pDeviceItem->m_strName);
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
        spdlog::get("logger")->info(
               "Loading level II driver: <{}>",
               pDeviceItem->m_strName);

        // * * * * VSCP OPEN * * * *
        if (NULL == (pDeviceItem->m_proc_VSCPOpen =
                       (LPFNDLL_VSCPOPEN)dlsym(hdll, "VSCPOpen"))) {
            // Free the library
            spdlog::get("logger")->error(
                   "{}: Unable to get dl entry for VSCPOpen.",
                   pDeviceItem->m_strName);
            return NULL;
        }

        // * * * * VSCP CLOSE * * * *
        if (NULL == (pDeviceItem->m_proc_VSCPClose =
                       (LPFNDLL_VSCPCLOSE)dlsym(hdll, "VSCPClose"))) {
            // Free the library
            spdlog::get("logger")->error(
                   "{}: Unable to get dl entry for VSCPClose.",
                   pDeviceItem->m_strName);
            return NULL;
        }

        // * * * * VSCPWRITE * * * *
        if (NULL == (pDeviceItem->m_proc_VSCPWrite =
                       (LPFNDLL_VSCPWRITE)dlsym(hdll, "VSCPWrite"))) {
            // Free the library
            spdlog::get("logger")->error(
                   "{}: Unable to get dl entry for VSCPWrite.",
                   pDeviceItem->m_strName);
            return NULL;
        }

        // * * * * VSCPREAD * * * *
        if (NULL == (pDeviceItem->m_proc_VSCPRead =
                       (LPFNDLL_VSCPREAD)dlsym(hdll, "VSCPRead"))) {
            // Free the library
            spdlog::get("logger")->error(
                   "{}: Unable to get dl entry for VSCPBlockingReceive.",
                   pDeviceItem->m_strName);
            return NULL;
        }

        // * * * * VSCP GET VERSION * * * *
        if (NULL == (pDeviceItem->m_proc_VSCPGetVersion =
                       (LPFNDLL_VSCPGETVERSION)dlsym(hdll, "VSCPGetVersion"))) {
            // Free the library
            spdlog::get("logger")->error(
                   "{}: Unable to get dl entry for VSCPGetVersion.",
                   pDeviceItem->m_strName);
            return NULL;
        }

        if (gDebugLevel  & VSCP_DEBUG_DRIVERL2) {
            spdlog::get("logger")->debug(
                   "{}: Discovered all methods\n",
                   pDeviceItem->m_strName);
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
                spdlog::get("logger")->error("Failed to create new mosquitto session (out of memory).");
            }
            else if (EINVAL == errno) {
                spdlog::get("logger")->error("Failed to create new mosquitto session (invalid parameters).");
            }

            dlclose(hdll);
            return NULL;

        }

        // Set callbacks
        mosquitto_log_callback_set(pDeviceItem->m_mosq, mqtt_drv_log_callback);
        mosquitto_connect_callback_set(pDeviceItem->m_mosq, mqtt_drv_on_connect);
        mosquitto_disconnect_callback_set(pDeviceItem->m_mosq, mqtt_drv_on_disconnect);
        mosquitto_message_callback_set(pDeviceItem->m_mosq, mqtt_drv_on_message);
        mosquitto_publish_callback_set(pDeviceItem->m_mosq, mqtt_drv_on_publish);

        if (MOSQ_ERR_SUCCESS != mosquitto_reconnect_delay_set(pDeviceItem->m_mosq,
   	                                                            pDeviceItem->m_mqtt_reconnect_delay,
   	                                                            pDeviceItem->m_mqtt_reconnect_delay_max,
   		                                                        pDeviceItem->m_mqtt_reconnect_exponential_backoff)) {
            spdlog::get("logger")->error("Failed to set reconnect settings.");                                 
        }

        // Set username/password if defined
        if (pDeviceItem->m_mqtt_strUserName.length()) {
            int rv;
            if ( MOSQ_ERR_SUCCESS != (rv = mosquitto_username_pw_set( pDeviceItem->m_mosq,
                                                                        pDeviceItem->m_mqtt_strUserName.c_str(),
                                                                        pDeviceItem->m_mqtt_strPassword.c_str() ) ) ) {
                if (MOSQ_ERR_INVAL == rv) {
                    spdlog::get("logger")->error("Failed to set mosquitto username/password (invalid parameter(s)).");
                }
                else if (MOSQ_ERR_NOMEM == rv) {
                    spdlog::get("logger")->error("Failed to set mosquitto username/password (out of memory).");
                }
            }
        }

        int rv = mosquitto_connect(pDeviceItem->m_mosq,
                                        pDeviceItem->m_mqtt_strHost.c_str(),
                                        pDeviceItem->m_mqtt_port,
                                        pDeviceItem->m_mqtt_keepalive);

        if ( MOSQ_ERR_SUCCESS != rv ) {
            
            if (MOSQ_ERR_INVAL == rv) {
                spdlog::get("logger")->error("Failed to connect to mosquitto server (invalid parameter(s)).");
            }
            else if (MOSQ_ERR_ERRNO == rv) {
                spdlog::get("logger")->error("Failed to connect to mosquitto server. System returned error (errno = %d).", errno);
            }

            dlclose(hdll);
            return NULL;
        }

        //mosquitto_threaded_set(pDeviceItem->m_mosq, true);

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
                    spdlog::get("logger")->error("Failed to subscribed to specified topic [{}] - input parameters were invalid.",
                            subscribe_topic);
                case MOSQ_ERR_NOMEM:
                    spdlog::get("logger")->error("Failed to subscribed to specified topic [{}] - out of memory condition occurred.",
                            subscribe_topic);
                case MOSQ_ERR_NO_CONN:
                    spdlog::get("logger")->error("Failed to subscribed to specified topic [{}] - client isn’t connected to a broker.",
                            subscribe_topic);
                case MOSQ_ERR_MALFORMED_UTF8:
                    spdlog::get("logger")->error("Failed to subscribed to specified topic [{}] - topic is not valid UTF-8.",
                            subscribe_topic);
#if defined(MOSQ_ERR_OVERSIZE_PACKET)
                case MOSQ_ERR_OVERSIZE_PACKET:
                    spdlog::get("logger")->error("Failed to subscribed to specified topic [{}] - resulting packet would be larger than supported by the broker.",
                            subscribe_topic);
#endif                    
            }
        }



        // -------------------------------------------------------------



        // Open up the L2 driver
        pDeviceItem->m_openHandle =
          pDeviceItem->m_proc_VSCPOpen(pDeviceItem->m_strParameter.c_str(),
                                    pDeviceItem->m_guid.getGUID());

        if (0 == pDeviceItem->m_openHandle) {
            // Free the library
            spdlog::get("logger")->error(
                   "{}: [Device tread] Unable to open VSCP "
                   " level II driver (path, config file access rights)."
                   " There may be additional info from driver "
                   "in log. If not enable debug flag in drivers config file",
                   pDeviceItem->m_strName);
            return NULL;
        }

        if (gDebugLevel  & VSCP_DEBUG_DRIVERL2) {
            spdlog::get("logger")->debug(
                   "{}: [Device tread] Level II Open.",
                   pDeviceItem->m_strName);
        }


        // --------------------------------------------------------------------
        //        Work loop L2 - receive from device - send to MQTT broker
        // --------------------------------------------------------------------

        // Just sit and wait until the end of the world as we know it...
        while (!pDeviceItem->m_bQuit) {

            vscpEvent ev;
            memset(&ev, 0, sizeof(vscpEvent));

            //int rx = mosquitto_loop(pDeviceItem->m_mosq, 0, 1);
            if (CANAL_ERROR_SUCCESS != pDeviceItem->m_proc_VSCPRead(pDeviceItem->m_openHandle, &ev, 50)) {
                continue;
            }

            // If timestamp is zero we set it here
            if (0 == ev.timestamp) {
                ev.timestamp = vscp_makeTimeStamp();
            }

            // Publish to MQTT broker

            if (!pDeviceItem->sendEvent(&ev)) {
                spdlog::get("logger")->error("Driver L2: {} Failed to send event to broker.", 
                        pDeviceItem->m_strName.c_str());
                continue;
            }

        }

        if (gDebugLevel  & VSCP_DEBUG_DRIVERL2) {
           spdlog::get("logger")->debug(
                   "{}: [Device tread] Level II Closing.",
                   pDeviceItem->m_strName);
        }

        // Close channel
        pDeviceItem->m_proc_VSCPClose(pDeviceItem->m_openHandle);

        if (gDebugLevel  & VSCP_DEBUG_DRIVERL2) {
            spdlog::get("logger")->debug(
                   "{}: [Device tread] Level II Closed.",
                   pDeviceItem->m_strName);
        }

        pDeviceItem->m_bQuit = true;
        mosquitto_disconnect(pDeviceItem->m_mosq);

        // Unload dll
        dlclose(hdll);

        if (gDebugLevel  & VSCP_DEBUG_DRIVERL2) {
            spdlog::get("logger")->debug(
                   "{}: [Device tread] Level II Done waiting for threads.",
                   pDeviceItem->m_strName.c_str());
        }
    }

    return NULL;
}

