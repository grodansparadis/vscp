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

//////////////////////////////////////////////////////////////////////
//                         Callbacks
//////////////////////////////////////////////////////////////////////


static void mqtt_log_callback(struct mosquitto *mosq, void *pData, int level, const char *logmsg)
{
    // Check pointers
    if (NULL == mosq) return;
    if (NULL == pData) return;

    CDeviceItem *pItem = (CDeviceItem *)pData;
    if (pItem->m_debugFlags & VSCP_DEBUG_MQTT_LOG) {

    }

}


static void mqtt_on_connect(struct mosquitto *mosq, void *pData, int rv)
{
    // Check pointers
    if (NULL == mosq) return;
    if (NULL == pData) return;

    CDeviceItem *pItem = (CDeviceItem *)pData;
    if (pItem->m_debugFlags & VSCP_DEBUG_MQTT_CONNECT) {

    }

}


static void mqtt_on_disconnect(struct mosquitto *mosq, void *pData, int rv)
{
    // Check pointers
    if (NULL == mosq) return;
    if (NULL == pData) return;

    CDeviceItem *pItem = (CDeviceItem *)pData;
    if (pItem->m_debugFlags & VSCP_DEBUG_MQTT_CONNECT) {

    }

}


static void mqtt_on_message(struct mosquitto *mosq, void *pData, const struct mosquitto_message *pMsg)
{
    // Check pointers
    if (NULL == mosq) return;
    if (NULL == pData) return;
    if (NULL == pMsg) return;

    CDeviceItem *pItem = (CDeviceItem *)pData;
    std::string payload((const char *)pMsg->payload, pMsg->payloadlen);

    if (pItem->m_debugFlags & VSCP_DEBUG_MQTT_MSG) {

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
            if ( MOSQ_ERR_SUCCESS != 
                    ( rv = mosquitto_username_pw_set( pDeviceItem->m_mosq,
                                                        pDeviceItem->m_mqtt_strUserName.c_str(),
                                                        pDeviceItem->m_mqtt_strPassword.c_str() ) ) ) {
                if ( MOSQ_ERR_INVAL == rv) {
                    syslog(LOG_ERR, "Failed to set mosquitto username/password (invalid parameter(s)).");
                }
                else if ( MOSQ_ERR_NOMEM == rv) {
                    syslog(LOG_ERR, "Failed to set mosquitto username/password (out of memory).");
                }
                                                    
            }
        }

        int rv = mosquitto_connect( pDeviceItem->m_mosq, 
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

        // -------------------------------------------------------------

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

            if ( pthread_create(&pDeviceItem->m_level1WriteThread, NULL, deviceLevel1WriteThread, pDeviceItem)) {
                syslog(LOG_ERR, "%s: Unable to run the device write worker thread.", pDeviceItem->m_strName.c_str());
                dlclose(hdll);
                return NULL;
            }

            /////////////////////////////////////////////////////////////////////////////
            // Device read worker thread
            /////////////////////////////////////////////////////////////////////////////
            if (pthread_create( &pDeviceItem->m_level1ReceiveThread, NULL, deviceLevel1ReceiveThread, pDeviceItem)) {
                syslog(LOG_ERR, "%s: Unable to run the device read worker thread.", pDeviceItem->m_strName.c_str());
                pDeviceItem->m_bQuit = true;
                pthread_join(pDeviceItem->m_level1WriteThread, NULL);
                dlclose(hdll);
                return NULL;
            }

            // Just sit and wait until the end of the world as we know it...
            while (!pDeviceItem->m_bQuit) {
                sleep(1);
            }

            // Signal worker threads to quit
            pDeviceItem->m_bQuit = true;

            if (pDeviceItem->m_debugFlags & VSCP_DEBUG_DRIVERL1) {
                syslog(LOG_DEBUG, "%s: [Device tread] Level I work loop ended.", pDeviceItem->m_strName.c_str());
            }

            // Wait for workerthreads to abort
            pthread_join( pDeviceItem->m_level1WriteThread, NULL );
            pthread_join( pDeviceItem->m_level1ReceiveThread, NULL );
        }
        else {

            // * * * * Non blocking version * * * *

            if (pDeviceItem->m_debugFlags & VSCP_DEBUG_DRIVERL1) {
                syslog(LOG_DEBUG, "%s: [Device tread] Level I NON Blocking version.", pDeviceItem->m_strName.c_str());
            }

            bool bActivity;
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

                            // Convert CANAL message to VSCP event
                            vscp_convertCanalToEvent( pev,
                                                        &msg,
                                                        (unsigned char *)pDeviceItem->m_guid.getGUID());

                            pev->obid = 0;

                            

                        }
                        else {

                        }
                        
                    }
                } // data available

                // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
                //          Send messages (if any) in the output queue
                // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

                // Check if there is something to send
                // if (pClientItem->m_clientInputQueue.size()) {

                    bActivity = true;

                //     std::deque<vscpEvent*>::iterator it;
                //     pthread_mutex_lock(&pClientItem->m_mutexClientInputQueue);
                //     vscpEvent* pev = pClientItem->m_clientInputQueue.front();
                //     pthread_mutex_unlock(&pClientItem->m_mutexClientInputQueue);

                //     // Trow away Level II event on Level I interface
                //     if ((VSCP_DRIVER_LEVEL1 ==
                //          pDeviceItem->m_driverLevel) &&
                //         (pev->vscp_class > 512)) {
                //         // Remove the event and the node
                //         pClientItem->m_clientInputQueue.pop_front();
                //         syslog(LOG_ERR,
                //                "Level II event on Level I queue thrown away. "
                //                "class=%d, type=%d",
                //                pev->vscp_class,
                //                pev->vscp_type);
                //         vscp_deleteEvent(pev);
                //         continue;
                //     }

                //     canalMsg canmsg;
                //     vscp_convertEventToCanal(&canmsg, pev);
                //     if (CANAL_ERROR_SUCCESS ==
                //         pDeviceItem->m_proc_CanalSend(pDeviceItem->m_openHandle,
                //                                    &canmsg)) {
                //         // Remove the event and the node
                //         pClientItem->m_clientInputQueue.pop_front();
                //         vscp_deleteEvent(pev);
                //     }
                //     else {
                //         // Another try
                //         // pObj->m_semClientOutputQueue.Post();
                //         vscp_deleteEvent(pev); // TODO ????
                //     }

                // } // events

                // if (!bActivity) {
                //     usleep(100000); // 100 ms
                // }

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
        pthread_join(pDeviceItem->m_level1WriteThread, NULL);
        pthread_join(pDeviceItem->m_level1ReceiveThread, NULL);

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

        // Open up the driver
        pDeviceItem->m_openHandle =
          pDeviceItem->m_proc_VSCPOpen(pDeviceItem->m_strParameter.c_str(),
                                    pDeviceItem->m_drvGuid.getGUID());

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

        /////////////////////////////////////////////////////////////////////////////
        // Level II - Device write worker thread
        /////////////////////////////////////////////////////////////////////////////

        if (pthread_create(&pDeviceItem->m_level2WriteThread,
                           NULL,
                           deviceLevel2WriteThread,
                           pDeviceItem)) {
            syslog(LOG_ERR,
                   "%s: Unable to run the device Level II write worker thread.",
                   pDeviceItem->m_strName.c_str());
            dlclose(hdll);
            return NULL; // TODO close dll
        }

        if (pDeviceItem->m_debugFlags & VSCP_DEBUG_DRIVERL2) {
            syslog(LOG_DEBUG,
                   "%s: [Device tread] Level II Write thread created.",
                   pDeviceItem->m_strName.c_str());
        }

        /////////////////////////////////////////////////////////////////////////////
        // Level II - Device read worker thread
        /////////////////////////////////////////////////////////////////////////////

        if (pthread_create(&pDeviceItem->m_level2ReceiveThread,
                           NULL,
                           deviceLevel2ReceiveThread,
                           pDeviceItem)) {
            syslog(LOG_ERR,
                   "%s: Unable to run the device Level II read worker thread.",
                   pDeviceItem->m_strName.c_str());
            pDeviceItem->m_bQuit = true;
            pthread_join(pDeviceItem->m_level2WriteThread, NULL);
            pthread_join(pDeviceItem->m_level2ReceiveThread, NULL);
            dlclose(hdll);
            return NULL; // TODO close dll, kill other thread
        }

        if (pDeviceItem->m_debugFlags & VSCP_DEBUG_DRIVERL2) {
            syslog(LOG_DEBUG,
                   "%s: [Device tread] Level II Write thread created.",
                   pDeviceItem->m_strName.c_str());
        }

        // Just sit and wait until the end of the world as we know it...
        while (!pDeviceItem->m_bQuit) {
            sleep(1);
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
        pthread_join(pDeviceItem->m_level2WriteThread, NULL);
        pthread_join(pDeviceItem->m_level2ReceiveThread, NULL);

        mosquitto_disconnect(pDeviceItem->m_mosq);

        // Unload dll
        dlclose(hdll);

        if (pDeviceItem->m_debugFlags & VSCP_DEBUG_DRIVERL2) {
            syslog(LOG_DEBUG,
                   "%s: [Device tread] Level II Done waiting for threads.",
                   pDeviceItem->m_strName.c_str());
        }
    }

    // Remove messages in the client queues
    // pthread_mutex_lock(&pObj->m_clientList.m_mutexItemList);
    // pObj->removeClient(pClientItem);
    // pthread_mutex_unlock(&pObj->m_clientList.m_mutexItemList);

    return NULL;
}

// ****************************************************************************

///////////////////////////////////////////////////////////////////////////////
// deviceLevel1ReceiveThread
//

void*
deviceLevel1ReceiveThread(void* pData)
{
    canalMsg msg;
    // Level1MsgOutList::compatibility_iterator nodeLevel1;

    CDeviceItem* pDeviceItem = (CDeviceItem*)pData;
    if (NULL == pDeviceItem) {
        syslog(
          LOG_ERR,
          "deviceLevel1ReceiveThread quitting due to NULL DevItem object.");
        return NULL;
    }

    // Blocking receive method must have been found
    if (NULL == pDeviceItem->m_proc_CanalBlockingReceive) {
        return NULL;
    }

    while (!pDeviceItem->m_bQuit) {

        if (CANAL_ERROR_SUCCESS ==
            pDeviceItem->m_proc_CanalBlockingReceive(pDeviceItem->m_openHandle,
                                                  &msg,
                                                  500)) {

            // // There must be room in the receive queue
            // if (pDeviceItem->m_pObj->m_maxItemsInClientReceiveQueue >
            //     pDeviceItem->m_pObj->m_clientOutputQueue.size()) {

            //     vscpEvent* pvscpEvent = new vscpEvent;
            //     if (NULL != pvscpEvent) {

            //         memset(pvscpEvent, 0, sizeof(vscpEvent));

            //         // Set driver GUID if set
            //         /*if ( pDeviceItem->m_guid.isNULL()
            //         ) { pDeviceItem->m_guid.writeGUID(
            //         pvscpEvent->GUID );
            //         }
            //         else {
            //             // If no driver GUID set use interface GUID
            //             pDeviceItem->m_guid.writeGUID(
            //         pvscpEvent->GUID );
            //         }*/

            //         // Convert CANAL message to VSCP event
            //         vscp_convertCanalToEvent(
            //           pvscpEvent,
            //           &msg,
            //           pDeviceItem->m_pClientItem->m_guid.m_id);

            //         pvscpEvent->obid = pDeviceItem->m_pClientItem->m_clientID;

            //         // If no GUID is set,
            //         //      - Set driver GUID if it is defined
            //         //      - Set to interface GUID if not.

            //         uint8_t ifguid[16];

            //         // Save nickname
            //         uint8_t nickname_lsb = pvscpEvent->GUID[15];

            //         // Set if to use
            //         memcpy(ifguid, pvscpEvent->GUID, 16);
            //         ifguid[14] = 0;
            //         ifguid[15] = 0;

            //         // If if is set to zero use interface id
            //         if (vscp_isGUIDEmpty(ifguid)) {

            //             // Set driver GUID if set
            //             if (!pDeviceItem->m_guid.isNULL()) {
            //                 pDeviceItem->m_guid.writeGUID(
            //                   pvscpEvent->GUID);
            //             }
            //             else {
            //                 // If no driver GUID set use interface GUID
            //                 pDeviceItem->m_pClientItem->m_guid.writeGUID(
            //                   pvscpEvent->GUID);
            //             }

            //             // Preserve nickname
            //             pvscpEvent->GUID[15] = nickname_lsb;
            //         }

            //         // =========================================================
            //         //                   Outgoing translations
            //         // =========================================================

            //         // Level I measurement events to Level II measurement float
            //         if (pDeviceItem->m_translation & VSCP_DRIVER_OUT_TR_M1_M2F) {
            //             vscp_convertLevel1MeasuremenToLevel2Double(pvscpEvent);
            //         }

            //         // Level I measurement events to Level II measurement string
            //         if (pDeviceItem->m_translation & VSCP_DRIVER_OUT_TR_M1_M2S) {
            //             vscp_convertLevel1MeasuremenToLevel2String(pvscpEvent);
            //         }

            //         // Level I events to Level I over Level II events
            //         if (pDeviceItem->m_translation & VSCP_DRIVER_OUT_TR_ALL_L2) {
            //             pvscpEvent->vscp_class += 512;
            //             uint8_t* p = new uint8_t[16 + pvscpEvent->sizeData];
            //             if (NULL != p) {
            //                 memset(p, 0, 16 + pvscpEvent->sizeData);
            //                 memcpy(p + 16,
            //                        pvscpEvent->pdata,
            //                        pvscpEvent->sizeData);
            //                 pvscpEvent->sizeData += 16;
            //                 delete[] pvscpEvent->pdata;
            //                 pvscpEvent->pdata = p;
            //             }
            //         }

            //         // pthread_mutex_lock(
            //         //   &pDeviceItem->m_pObj->m_mutex_ClientOutputQueue);
            //         // pDeviceItem->m_pObj->m_clientOutputQueue.push_back(pvscpEvent);
            //         // pthread_mutex_unlock(
            //         //   &pDeviceItem->m_pObj->m_mutex_ClientOutputQueue);
            //         // sem_post(&pDeviceItem->m_pObj->m_semClientOutputQueue);
            //     }
            // }
        }
    }

    return NULL;
}

// ****************************************************************************

///////////////////////////////////////////////////////////////////////////////
// deviceLevel1WriteThread
//

void*
deviceLevel1WriteThread(void* pData)
{
    // Level1MsgOutList::compatibility_iterator nodeLevel1;

    CDeviceItem* pDeviceItem = (CDeviceItem*)pData;
    if (NULL == pDeviceItem) {
        syslog(LOG_ERR,
               "deviceLevel1WriteThread quitting due to NULL DevItem object.");
        return NULL;
    }

    // Blocking send method must have been found
    if (NULL == pDeviceItem->m_proc_CanalBlockingSend)
        return NULL;

    while (!pDeviceItem->m_bQuit) {

        // Wait until there is something to send
        if ((-1 == vscp_sem_wait(&pDeviceItem->m_seminputQueue, 500)) && errno == ETIMEDOUT) {
            continue;
        }

        if (pDeviceItem->m_inputQueue.size()) {

            pthread_mutex_lock( &pDeviceItem->m_mutexinputQueue );
            vscpEvent* pev = pDeviceItem->m_inputQueue.front();
            pDeviceItem->m_inputQueue.pop_front();
            pthread_mutex_unlock(&pDeviceItem->m_mutexinputQueue);

            // Trow away event if Level II and Level I interface
            if ((VSCP_DRIVER_LEVEL1 == pDeviceItem->m_driverLevel) && (pev->vscp_class > 512)) {
                vscp_deleteEvent(pev);
                continue;
            }

            canalMsg msg;
            if (!vscp_convertEventToCanal(&msg, pev)) {
                syslog( LOG_ERR, "deviceLevel1WriteThread - vscp_convertEventToCanal failed" );
                vscp_deleteEvent(pev);
            }

            if (CANAL_ERROR_SUCCESS ==
                pDeviceItem->m_proc_CanalBlockingSend(pDeviceItem->m_openHandle,
                                                   &msg,
                                                   300)) {
                syslog( LOG_ERR, "deviceLevel1WriteThread - m_proc_CanalBlockingSend failed");
                vscp_deleteEvent(pev);
            }
            else {
                // Give it another try
                //sem_post(&pDeviceItem->m_pObj->m_semClientOutputQueue);
            }

        }  // events in queue

    }  // while

    return NULL;
}

//-----------------------------------------------------------------------------
//                               L e v e l  I I
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// deviceLevel2ReceiveThread
//
//  Read from device
//

void*
deviceLevel2ReceiveThread(void* pData)
{
    vscpEvent* pev;

    CDeviceItem* pDeviceItem = (CDeviceItem*)pData;
    if (NULL == pDeviceItem) {
        syslog(
          LOG_ERR,
          "deviceLevel2ReceiveThread quitting due to NULL DevItem object.");
        return NULL;
    }

    int rv;
    while (!pDeviceItem->m_bQuit) {

        pev = new vscpEvent;
        if (NULL == pev)
            continue;
        rv = pDeviceItem->m_proc_VSCPRead(pDeviceItem->m_openHandle, pev, 500);

        if ((CANAL_ERROR_SUCCESS != rv) || (NULL == pev)) {
            delete pev;
            continue;
        }

        // If timestamp is zero we set it here
        if (0 == pev->timestamp) {
            pev->timestamp = vscp_makeTimeStamp();
        }

        // If no GUID is set,
        //      - Set driver GUID if define
        //      - Set interface GUID if no driver GUID defined.

        uint8_t ifguid[16];

        // Save nickname
        uint8_t nickname_msb = pev->GUID[14];
        uint8_t nickname_lsb = pev->GUID[15];

        // Set if to use
        memcpy(ifguid, pev->GUID, 16);
        ifguid[14] = 0;
        ifguid[15] = 0;

        // If if is set to zero use interface id
        if (vscp_isGUIDEmpty(ifguid)) {

            // Set driver GUID if set
            if (!pDeviceItem->m_guid.isNULL()) {
                pDeviceItem->m_guid.writeGUID(pev->GUID);
            }
            else {
                // If no driver GUID set use interface GUID
                //pDeviceItem->m_pClientItem->m_guid.writeGUID(pev->GUID);
            }

            // Preserve nickname
            pev->GUID[14] = nickname_msb;
            pev->GUID[15] = nickname_lsb;
        }

        // There must be room in the receive queue
        // if (pDeviceItem->m_pObj->m_maxItemsInClientReceiveQueue >
        //     pDeviceItem->m_pObj->m_clientOutputQueue.size()) {

        //     pthread_mutex_lock(&pDeviceItem->m_pObj->m_mutex_ClientOutputQueue);
        //     pDeviceItem->m_pObj->m_clientOutputQueue.push_back(pev);
        //     pthread_mutex_unlock(&pDeviceItem->m_pObj->m_mutex_ClientOutputQueue);
        //     sem_post(&pDeviceItem->m_pObj->m_semClientOutputQueue);
        // }
        // else {
        //     if (NULL == pev)
        //         vscp_deleteEvent_v2(&pev);
        // }
    }

    return NULL;
}

// ****************************************************************************

///////////////////////////////////////////////////////////////////////////////
// deviceLevel2WriteThread
//
//  Write to device
//

void*
deviceLevel2WriteThread(void* pData)
{
    CDeviceItem* pDeviceItem = (CDeviceItem*)pData;
    if (NULL == pDeviceItem) {
        syslog(LOG_ERR,
               "deviceLevel2WriteThread quitting due to NULL DevItem object.");
        return NULL;
    }

    while (!pDeviceItem->m_bQuit) {

        // Wait until there is something to send
        // if ((-1 ==
        //      vscp_sem_wait(&pDeviceItem->m_pClientItem->m_semClientInputQueue,
        //                    500)) &&
        //     errno == ETIMEDOUT) {
        //     continue;
        // }

        // if (pDeviceItem->m_pClientItem->m_clientInputQueue.size()) {

        //     pthread_mutex_lock(
        //       &pDeviceItem->m_pClientItem->m_mutexClientInputQueue);
        //     vscpEvent* pev =
        //       pDeviceItem->m_pClientItem->m_clientInputQueue.front();
        //     pthread_mutex_unlock(
        //       &pDeviceItem->m_pClientItem->m_mutexClientInputQueue);

        //     if (CANAL_ERROR_SUCCESS ==
        //         pDeviceItem->m_proc_VSCPWrite(pDeviceItem->m_openHandle, pev, 300)) {

        //         // Remove the node
        //         pthread_mutex_lock(
        //           &pDeviceItem->m_pClientItem->m_mutexClientInputQueue);
        //         pDeviceItem->m_pClientItem->m_clientInputQueue.pop_front();
        //         pthread_mutex_unlock(
        //           &pDeviceItem->m_pClientItem->m_mutexClientInputQueue);
        //     }
        //     else {
        //         // Give it another try
        //         //sem_post(&pDeviceItem->m_pObj->m_semClientOutputQueue);
        //     }

        // } // events in queue

    } // while

    return NULL;
}
