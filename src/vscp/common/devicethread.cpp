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
#include <controlobject.h>
#include <dllist.h>
#include <level2drvdef.h>
#include <vscp.h>
#include <vscp_debug.h>
#include <vscphelper.h>

#include "devicethread.h"

///////////////////////////////////////////////////////////////////////////////
// deviceThread
//

void*
deviceThread(void* pData)
{
    // const char *dlsym_error;

    CDeviceItem* pDevItem = (CDeviceItem*)pData;
    if (NULL == pDevItem) {
        syslog(LOG_ERR, "No device item defined. Aborting device thread!");
        return NULL;
    }

    // Must have a valid pointer to the control object
    CControlObject* pObj = pDevItem->m_pObj;
    if (NULL == pObj) {
        syslog(LOG_ERR, "No control object defined. Aborting device thread!");
        return NULL;
    }

    // We need to create a clientobject and add this object to the list
    CClientItem* pClientItem = pDevItem->m_pClientItem = new CClientItem;
    if (NULL == pClientItem) {
        return NULL;
    }

    // This is now an active Client
    pClientItem->m_bOpen = true;
    if (VSCP_DRIVER_LEVEL1 == pDevItem->m_driverLevel) {
        pClientItem->m_type = CLIENT_ITEM_INTERFACE_TYPE_DRIVER_LEVEL1;
    } else if (VSCP_DRIVER_LEVEL2 == pDevItem->m_driverLevel) {
        pClientItem->m_type = CLIENT_ITEM_INTERFACE_TYPE_DRIVER_LEVEL2;
    }

    pClientItem->m_dtutc.setUTCNow();
    pClientItem->m_strDeviceName = "driver_" + pDevItem->m_strName;

    if (__VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG,
               "Devicethread: Starting %s",
               pClientItem->m_strDeviceName.c_str());
    }

    // Add the client to the Client List
    pthread_mutex_lock(&pObj->m_clientList.m_mutexItemList);
    if (!pObj->addClient(pClientItem,
                         pDevItem->m_interface_guid.getClientID())) {
        // Failed to add client
        delete pDevItem->m_pClientItem;
        pDevItem->m_pClientItem = NULL;

        pthread_mutex_unlock(&pObj->m_clientList.m_mutexItemList);
        syslog(LOG_ERR,
               "Devicethread: Failed to add client. Terminating thread.");
        return NULL;
    }
    pthread_mutex_unlock(&pObj->m_clientList.m_mutexItemList);

    // Client now have GUID set to server GUID + channel id
    // If device has a non NULL GUID replace the client GUID preserving
    // the channel id with that GUID
    if (!pClientItem->m_guid.isNULL()) {
        memcpy(pClientItem->m_guid.m_id,
               pDevItem->m_interface_guid.getGUID(),
               12);
    }

    void* hdll;

    // Load dynamic library
    hdll = dlopen(pDevItem->m_strPath.c_str(), RTLD_LAZY);
    if (!hdll) {
        syslog(LOG_ERR,
               "Devicethread: Unable to load dynamic library. path = %s",
               pDevItem->m_strPath.c_str());
        return NULL;
    }

    //*************************************************************************
    //                         Level I drivers
    //*************************************************************************
    if (VSCP_DRIVER_LEVEL1 == pDevItem->m_driverLevel) {

        // Now find methods in library
        if (__VSCP_DEBUG_DRIVER1) {
            syslog(LOG_DEBUG,
                   "Loading level I driver: %s",
                   pDevItem->m_strName.c_str());
        }

        // * * * * CANAL OPEN * * * *
        pDevItem->m_proc_CanalOpen =
          (LPFNDLL_CANALOPEN)dlsym(hdll, "CanalOpen");
        const char* dlsym_error = dlerror();

        if (dlsym_error) {
            // Free the library
            syslog(LOG_ERR,
                   "%s : Unable to get dl entry for CanalOpen.",
                   pDevItem->m_strName.c_str());
            return NULL;
        }

        // * * * * CANAL CLOSE * * * *
        pDevItem->m_proc_CanalClose =
          (LPFNDLL_CANALCLOSE)dlsym(hdll, "CanalClose");
        dlsym_error = dlerror();
        if (dlsym_error) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for CanalClose.",
                   pDevItem->m_strName.c_str());
            dlclose(hdll);
            return NULL;
        }

        // * * * * CANAL GETLEVEL * * * *
        pDevItem->m_proc_CanalGetLevel =
          (LPFNDLL_CANALGETLEVEL)dlsym(hdll, "CanalGetLevel");
        dlsym_error = dlerror();
        if (dlsym_error) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for CanalGetLevel.",
                   pDevItem->m_strName.c_str());
            dlclose(hdll);
            return NULL;
        }

        // * * * * CANAL SEND * * * *
        pDevItem->m_proc_CanalSend =
          (LPFNDLL_CANALSEND)dlsym(hdll, "CanalSend");
        dlsym_error = dlerror();
        if (dlsym_error) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for CanalSend.",
                   pDevItem->m_strName.c_str());
            dlclose(hdll);
            return NULL;
        }

        // * * * * CANAL DATA AVAILABLE * * * *
        pDevItem->m_proc_CanalDataAvailable =
          (LPFNDLL_CANALDATAAVAILABLE)dlsym(hdll, "CanalDataAvailable");
        dlsym_error = dlerror();
        if (dlsym_error) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for CanalDataAvailable.",
                   pDevItem->m_strName.c_str());
            dlclose(hdll);
            return NULL;
        }

        // * * * * CANAL RECEIVE * * * *
        pDevItem->m_proc_CanalReceive =
          (LPFNDLL_CANALRECEIVE)dlsym(hdll, "CanalReceive");
        dlsym_error = dlerror();
        if (dlsym_error) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for CanalReceive.",
                   pDevItem->m_strName.c_str());
            dlclose(hdll);
            return NULL;
        }

        // * * * * CANAL GET STATUS * * * *
        pDevItem->m_proc_CanalGetStatus =
          (LPFNDLL_CANALGETSTATUS)dlsym(hdll, "CanalGetStatus");
        dlsym_error = dlerror();
        if (dlsym_error) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for CanalGetStatus.",
                   pDevItem->m_strName.c_str());
            dlclose(hdll);
            return NULL;
        }

        // * * * * CANAL GET STATISTICS * * * *
        pDevItem->m_proc_CanalGetStatistics =
          (LPFNDLL_CANALGETSTATISTICS)dlsym(hdll, "CanalGetStatistics");
        dlsym_error = dlerror();
        if (dlsym_error) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for CanalGetStatistics.",
                   pDevItem->m_strName.c_str());
            dlclose(hdll);
            return NULL;
        }

        // * * * * CANAL SET FILTER * * * *
        pDevItem->m_proc_CanalSetFilter =
          (LPFNDLL_CANALSETFILTER)dlsym(hdll, "CanalSetFilter");
        dlsym_error = dlerror();
        if (dlsym_error) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for CanalSetFilter.",
                   pDevItem->m_strName.c_str());
            dlclose(hdll);
            return NULL;
        }

        // * * * * CANAL SET MASK * * * *
        pDevItem->m_proc_CanalSetMask =
          (LPFNDLL_CANALSETMASK)dlsym(hdll, "CanalSetMask");
        dlsym_error = dlerror();
        if (dlsym_error) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for CanalSetMask.",
                   pDevItem->m_strName.c_str());
            dlclose(hdll);
            return NULL;
        }

        // * * * * CANAL GET VERSION * * * *
        pDevItem->m_proc_CanalGetVersion =
          (LPFNDLL_CANALGETVERSION)dlsym(hdll, "CanalGetVersion");
        dlsym_error = dlerror();
        if (dlsym_error) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for CanalGetVersion.",
                   pDevItem->m_strName.c_str());
            dlclose(hdll);
            return NULL;
        }

        // * * * * CANAL GET DLL VERSION * * * *
        pDevItem->m_proc_CanalGetDllVersion =
          (LPFNDLL_CANALGETDLLVERSION)dlsym(hdll, "CanalGetDllVersion");
        dlsym_error = dlerror();
        if (dlsym_error) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for CanalGetDllVersion.",
                   pDevItem->m_strName.c_str());
            dlclose(hdll);
            return NULL;
        }

        // * * * * CANAL GET VENDOR STRING * * * *
        pDevItem->m_proc_CanalGetVendorString =
          (LPFNDLL_CANALGETVENDORSTRING)dlsym(hdll, "CanalGetVendorString");
        dlsym_error = dlerror();
        if (dlsym_error) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for CanalGetVendorString.",
                   pDevItem->m_strName.c_str());
            dlclose(hdll);
            return NULL;
        }

        // ******************************
        //     Generation 2 Methods
        // ******************************

        // * * * * CANAL BLOCKING SEND * * * *
        pDevItem->m_proc_CanalBlockingSend =
          (LPFNDLL_CANALBLOCKINGSEND)dlsym(hdll, "CanalBlockingSend");
        dlsym_error = dlerror();
        if (dlsym_error) {
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for CanalBlockingSend. Probably "
                   "Generation 1 driver.",
                   pDevItem->m_strName.c_str());
            pDevItem->m_proc_CanalBlockingSend = NULL;
        }

        // * * * * CANAL BLOCKING RECEIVE * * * *
        pDevItem->m_proc_CanalBlockingReceive =
          (LPFNDLL_CANALBLOCKINGRECEIVE)dlsym(hdll, "CanalBlockingReceive");
        dlsym_error = dlerror();
        if (dlsym_error) {
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for CanalBlockingReceive. "
                   "Probably Generation 1 driver.",
                   pDevItem->m_strName.c_str());
            pDevItem->m_proc_CanalBlockingReceive = NULL;
        }

        // * * * * CANAL GET DRIVER INFO * * * *
        pDevItem->m_proc_CanalGetdriverInfo =
          (LPFNDLL_CANALGETDRIVERINFO)dlsym(hdll, "CanalGetDriverInfo");
        dlsym_error = dlerror();
        if (dlsym_error) {
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for CanalGetDriverInfo. "
                   "Probably Generation 1 driver.",
                   pDevItem->m_strName.c_str());
            pDevItem->m_proc_CanalGetdriverInfo = NULL;
        }

        // Open the device
        pDevItem->m_openHandle = pDevItem->m_proc_CanalOpen(
          (const char*)pDevItem->m_strParameter.c_str(),
          pDevItem->m_DeviceFlags);

        // Check if the driver opened properly
        if (pDevItem->m_openHandle <= 0) {
            syslog(LOG_ERR,
                   "Failed to open driver. Will not use it! %ld [%s] ",
                   pDevItem->m_openHandle,
                   pDevItem->m_strName.c_str());
            dlclose(hdll);
            return NULL;
        }

        if (__VSCP_DEBUG_DRIVER1) {
            syslog(LOG_DEBUG,
                   "%s: [Device tread] Level I Driver open.",
                   pDevItem->m_strName.c_str());
        }

        // Get Driver Level
        pDevItem->m_driverLevel =
          pDevItem->m_proc_CanalGetLevel(pDevItem->m_openHandle);

        //  * * * Level I Driver * * *

        // Check if blocking driver is available
        if (NULL != pDevItem->m_proc_CanalBlockingReceive) {

            // * * * * Blocking version * * * *

            if (__VSCP_DEBUG_DRIVER1) {
                syslog(LOG_DEBUG,
                       "%s: [Device tread] Level I blocking version.",
                       pDevItem->m_strName.c_str());
            }

            /////////////////////////////////////////////////////////////////////////////
            //                      Device write worker thread
            /////////////////////////////////////////////////////////////////////////////

            if (pthread_create(&pDevItem->m_level1WriteThread,
                               NULL,
                               deviceLevel1WriteThread,
                               pDevItem)) {
                syslog(LOG_ERR,
                       "%s: Unable to run the device write worker thread.",
                       pDevItem->m_strName.c_str());
                dlclose(hdll);
                return NULL;
            }

            /////////////////////////////////////////////////////////////////////////////
            // Device read worker thread
            /////////////////////////////////////////////////////////////////////////////
            if (pthread_create(&pDevItem->m_level1ReceiveThread,
                               NULL,
                               deviceLevel1ReceiveThread,
                               pDevItem)) {
                syslog(LOG_ERR,
                       "%s: Unable to run the device read worker thread.",
                       pDevItem->m_strName.c_str());
                pDevItem->m_bQuit = true;
                pthread_join(pDevItem->m_level1WriteThread, NULL);
                dlclose(hdll);
                return NULL;
            }

            // Just sit and wait until the end of the world as we know it...
            while (!pDevItem->m_bQuit) {
                sleep(1);
            }

            // Signal worker threads to quit
            pDevItem->m_bQuit = true;

            if (__VSCP_DEBUG_DRIVER1) {
                syslog(LOG_DEBUG,
                       "%s: [Device tread] Level I work loop ended.",
                       pDevItem->m_strName.c_str());
            }

            // Wait for workerthreads to abort
            pthread_join(pDevItem->m_level1WriteThread, NULL);
            pthread_join(pDevItem->m_level1ReceiveThread, NULL);
        } else {

            // * * * * Non blocking version * * * *

            if (__VSCP_DEBUG_DRIVER1) {
                syslog(LOG_DEBUG,
                       "%s: [Device tread] Level I NON Blocking version.",
                       pDevItem->m_strName.c_str());
            }

            bool bActivity;
            while (!pDevItem->m_bQuit) {

                bActivity = false;
                /////////////////////////////////////////////////////////////////////////////
                //                           Receive from device
                /////////////////////////////////////////////////////////////////////////////
                canalMsg msg;
                if (pDevItem->m_proc_CanalDataAvailable(
                      pDevItem->m_openHandle)) {

                    if (CANAL_ERROR_SUCCESS ==
                        pDevItem->m_proc_CanalReceive(pDevItem->m_openHandle,
                                                      &msg)) {

                        bActivity = true;

                        // There must be room in the receive queue
                        if (pObj->m_maxItemsInClientReceiveQueue >
                            pObj->m_clientOutputQueue.size()) {

                            vscpEvent* pev = new vscpEvent;
                            if (NULL != pev) {

                                // Set driver GUID if set
                                if (pDevItem->m_interface_guid.isNULL()) {
                                    pDevItem->m_interface_guid.writeGUID(
                                      pev->GUID);
                                } else {
                                    // If no driver GUID set use interface GUID
                                    pClientItem->m_guid.writeGUID(pev->GUID);
                                }

                                // Convert CANAL message to VSCP event
                                vscp_convertCanalToEvent(
                                  pev,
                                  &msg,
                                  pClientItem->m_guid.m_id);

                                pev->obid = pClientItem->m_clientID;

                                pthread_mutex_lock(
                                  &pObj->m_mutex_ClientOutputQueue);
                                pObj->m_clientOutputQueue.push_back(pev);
                                pthread_mutex_unlock(
                                  &pObj->m_mutex_ClientOutputQueue);
                                sem_post(&pObj->m_semClientOutputQueue);
                            }
                        }
                    }
                } // data available

                // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
                //          Send messages (if any) in the output queue
                // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

                // Check if there is something to send
                if (pClientItem->m_clientInputQueue.size()) {

                    bActivity = true;

                    std::deque<vscpEvent*>::iterator it;
                    pthread_mutex_lock(&pClientItem->m_mutexClientInputQueue);
                    vscpEvent* pev = pClientItem->m_clientInputQueue.front();
                    pthread_mutex_unlock(&pClientItem->m_mutexClientInputQueue);

                    // Trow away Level II event on Level I interface
                    if ((CLIENT_ITEM_INTERFACE_TYPE_DRIVER_LEVEL1 ==
                         pClientItem->m_type) &&
                        (pev->vscp_class > 512)) {
                        // Remove the event and the node
                        pClientItem->m_clientInputQueue.pop_front();
                        syslog(LOG_ERR,
                               "Level II event on Level I queue thrown away. "
                               "class=%d, type=%d",
                               pev->vscp_class,
                               pev->vscp_type);
                        vscp_deleteEvent(pev);
                        continue;
                    }

                    canalMsg canmsg;
                    vscp_convertEventToCanal(&canmsg, pev);
                    if (CANAL_ERROR_SUCCESS ==
                        pDevItem->m_proc_CanalSend(pDevItem->m_openHandle,
                                                   &canmsg)) {
                        // Remove the event and the node
                        pClientItem->m_clientInputQueue.pop_front();
                        vscp_deleteEvent(pev);
                    } else {
                        // Another try
                        // pObj->m_semClientOutputQueue.Post();
                        vscp_deleteEvent(pev); // TODO ????
                    }

                } // events

                if (!bActivity) {
                    usleep(100000); // 100 ms
                }

                bActivity = false;

            } // while working - non blocking

        } // if blocking/non blocking

        if (__VSCP_DEBUG_DRIVER1) {
            syslog(LOG_DEBUG,
                   "%s: [Device tread] Level I Work loop ended.",
                   pDevItem->m_strName.c_str());
        }

        // Close CANAL channel
        pDevItem->m_proc_CanalClose(pDevItem->m_openHandle);

        if (__VSCP_DEBUG_DRIVER1) {
            syslog(LOG_DEBUG,
                   "%s: [Device tread] Level I Closed.",
                   pDevItem->m_strName.c_str());
        }

        pDevItem->m_bQuit = true;
        pthread_join(pDevItem->m_level1WriteThread, NULL);
        pthread_join(pDevItem->m_level1ReceiveThread, NULL);

        dlclose(hdll);

    }

    //*************************************************************************
    //                         Level II drivers
    //*************************************************************************

    else if (VSCP_DRIVER_LEVEL2 == pDevItem->m_driverLevel) {

        // Now find methods in library
        syslog(LOG_INFO,
               "Loading level II driver: <%s>",
               pDevItem->m_strName.c_str());

        // * * * * VSCP OPEN * * * *
        if (NULL == (pDevItem->m_proc_VSCPOpen =
                       (LPFNDLL_VSCPOPEN)dlsym(hdll, "VSCPOpen"))) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for VSCPOpen.",
                   pDevItem->m_strName.c_str());
            return NULL;
        }

        // * * * * VSCP CLOSE * * * *
        if (NULL == (pDevItem->m_proc_VSCPClose =
                       (LPFNDLL_VSCPCLOSE)dlsym(hdll, "VSCPClose"))) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for VSCPClose.",
                   pDevItem->m_strName.c_str());
            return NULL;
        }

        // * * * * VSCPWRITE * * * *
        if (NULL == (pDevItem->m_proc_VSCPWrite =
                       (LPFNDLL_VSCPWRITE)dlsym(hdll, "VSCPWrite"))) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for VSCPWrite.",
                   pDevItem->m_strName.c_str());
            return NULL;
        }

        // * * * * VSCPREAD * * * *
        if (NULL == (pDevItem->m_proc_VSCPRead =
                       (LPFNDLL_VSCPREAD)dlsym(hdll, "VSCPRead"))) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for VSCPBlockingReceive.",
                   pDevItem->m_strName.c_str());
            return NULL;
        }

        // * * * * VSCP GET VERSION * * * *
        if (NULL == (pDevItem->m_proc_VSCPGetVersion =
                       (LPFNDLL_VSCPGETVERSION)dlsym(hdll, "VSCPGetVersion"))) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: Unable to get dl entry for VSCPGetVersion.",
                   pDevItem->m_strName.c_str());
            return NULL;
        }

        if (__VSCP_DEBUG_DRIVER2) {
            syslog(LOG_DEBUG,
                   "%s: Discovered all methods\n",
                   pDevItem->m_strName.c_str());
        }

        // Open up the driver
        pDevItem->m_openHandle =
          pDevItem->m_proc_VSCPOpen(pDevItem->m_strParameter.c_str(),
                                    pDevItem->m_drvGuid.getGUID());

        if (0 == pDevItem->m_openHandle) {
            // Free the library
            syslog(LOG_ERR,
                   "%s: [Device tread] Unable to open VSCP "
                   " level II driver (path, config file access rights)."
                   " There may be additional info from driver "
                   "in syslog. If not enable debug flag in drivers config file",
                   pDevItem->m_strName.c_str());
            return NULL;
        }

        if (__VSCP_DEBUG_DRIVER2) {
            syslog(LOG_DEBUG,
                   "%s: [Device tread] Level II Open.",
                   pDevItem->m_strName.c_str());
        }

        /////////////////////////////////////////////////////////////////////////////
        // Level II - Device write worker thread
        /////////////////////////////////////////////////////////////////////////////

        if (pthread_create(&pDevItem->m_level2WriteThread,
                           NULL,
                           deviceLevel2WriteThread,
                           pDevItem)) {
            syslog(LOG_ERR,
                   "%s: Unable to run the device Level II write worker thread.",
                   pDevItem->m_strName.c_str());
            dlclose(hdll);
            return NULL; // TODO close dll
        }

        if (__VSCP_DEBUG_DRIVER2) {
            syslog(LOG_DEBUG,
                   "%s: [Device tread] Level II Write thread created.",
                   pDevItem->m_strName.c_str());
        }

        /////////////////////////////////////////////////////////////////////////////
        // Level II - Device read worker thread
        /////////////////////////////////////////////////////////////////////////////

        if (pthread_create(&pDevItem->m_level2ReceiveThread,
                           NULL,
                           deviceLevel2ReceiveThread,
                           pDevItem)) {
            syslog(LOG_ERR,
                   "%s: Unable to run the device Level II read worker thread.",
                   pDevItem->m_strName.c_str());
            pDevItem->m_bQuit = true;
            pthread_join(pDevItem->m_level2WriteThread, NULL);
            pthread_join(pDevItem->m_level2ReceiveThread, NULL);
            dlclose(hdll);
            return NULL; // TODO close dll, kill other thread
        }

        if (__VSCP_DEBUG_DRIVER2) {
            syslog(LOG_DEBUG,
                   "%s: [Device tread] Level II Write thread created.",
                   pDevItem->m_strName.c_str());
        }

        // Just sit and wait until the end of the world as we know it...
        while (!pDevItem->m_bQuit) {
            sleep(1);
        }

        if (__VSCP_DEBUG_DRIVER2) {
            syslog(LOG_DEBUG,
                   "%s: [Device tread] Level II Closing.",
                   pDevItem->m_strName.c_str());
        }

        // Close channel
        pDevItem->m_proc_VSCPClose(pDevItem->m_openHandle);

        if (__VSCP_DEBUG_DRIVER2) {
            syslog(LOG_DEBUG,
                   "%s: [Device tread] Level II Closed.",
                   pDevItem->m_strName.c_str());
        }

        pDevItem->m_bQuit = true;
        pthread_join(pDevItem->m_level2WriteThread, NULL);
        pthread_join(pDevItem->m_level2ReceiveThread, NULL);

        // Unload dll
        dlclose(hdll);

        if (__VSCP_DEBUG_DRIVER2) {
            syslog(LOG_DEBUG,
                   "%s: [Device tread] Level II Done waiting for threads.",
                   pDevItem->m_strName.c_str());
        }
    }

    // Remove messages in the client queues
    pthread_mutex_lock(&pObj->m_clientList.m_mutexItemList);
    pObj->removeClient(pClientItem);
    pthread_mutex_unlock(&pObj->m_clientList.m_mutexItemList);

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

    CDeviceItem* pDevItem = (CDeviceItem*)pData;
    if (NULL == pDevItem) {
        syslog(
          LOG_ERR,
          "deviceLevel1ReceiveThread quitting due to NULL DevItem object.");
        return NULL;
    }

    // Blocking receive method must have been found
    if (NULL == pDevItem->m_proc_CanalBlockingReceive) {
        return NULL;
    }

    while (!pDevItem->m_bQuit) {

        if (CANAL_ERROR_SUCCESS ==
            pDevItem->m_proc_CanalBlockingReceive(pDevItem->m_openHandle,
                                                  &msg,
                                                  500)) {

            // There must be room in the receive queue
            if (pDevItem->m_pObj->m_maxItemsInClientReceiveQueue >
                pDevItem->m_pObj->m_clientOutputQueue.size()) {

                vscpEvent* pvscpEvent = new vscpEvent;
                if (NULL != pvscpEvent) {

                    memset(pvscpEvent, 0, sizeof(vscpEvent));

                    // Set driver GUID if set
                    /*if ( pDevItem->m_interface_guid.isNULL()
                    ) { pDevItem->m_interface_guid.writeGUID(
                    pvscpEvent->GUID );
                    }
                    else {
                        // If no driver GUID set use interface GUID
                        pDevItem->m_guid.writeGUID(
                    pvscpEvent->GUID );
                    }*/

                    // Convert CANAL message to VSCP event
                    vscp_convertCanalToEvent(
                      pvscpEvent,
                      &msg,
                      pDevItem->m_pClientItem->m_guid.m_id);

                    pvscpEvent->obid = pDevItem->m_pClientItem->m_clientID;

                    // If no GUID is set,
                    //      - Set driver GUID if it is defined
                    //      - Set to interface GUID if not.

                    uint8_t ifguid[16];

                    // Save nickname
                    uint8_t nickname_lsb = pvscpEvent->GUID[15];

                    // Set if to use
                    memcpy(ifguid, pvscpEvent->GUID, 16);
                    ifguid[14] = 0;
                    ifguid[15] = 0;

                    // If if is set to zero use interface id
                    if (vscp_isGUIDEmpty(ifguid)) {

                        // Set driver GUID if set
                        if (!pDevItem->m_interface_guid.isNULL()) {
                            pDevItem->m_interface_guid.writeGUID(
                              pvscpEvent->GUID);
                        } else {
                            // If no driver GUID set use interface GUID
                            pDevItem->m_pClientItem->m_guid.writeGUID(
                              pvscpEvent->GUID);
                        }

                        // Preserve nickname
                        pvscpEvent->GUID[15] = nickname_lsb;
                    }

                    // =========================================================
                    //                   Outgoing translations
                    // =========================================================

                    // Level I measurement events to Level II measurement float
                    if (pDevItem->m_translation & VSCP_DRIVER_OUT_TR_M1_M2F) {
                        vscp_convertLevel1MeasuremenToLevel2Double(pvscpEvent);
                    }

                    // Level I measurement events to Level II measurement string
                    if (pDevItem->m_translation & VSCP_DRIVER_OUT_TR_M1_M2S) {
                        vscp_convertLevel1MeasuremenToLevel2String(pvscpEvent);
                    }

                    // Level I events to Level I over Level II events
                    if (pDevItem->m_translation & VSCP_DRIVER_OUT_TR_ALL_L2) {
                        pvscpEvent->vscp_class += 512;
                        uint8_t* p = new uint8_t[16 + pvscpEvent->sizeData];
                        if (NULL != p) {
                            memset(p, 0, 16 + pvscpEvent->sizeData);
                            memcpy(p + 16,
                                   pvscpEvent->pdata,
                                   pvscpEvent->sizeData);
                            pvscpEvent->sizeData += 16;
                            delete[] pvscpEvent->pdata;
                            pvscpEvent->pdata = p;
                        }
                    }

                    pthread_mutex_lock(
                      &pDevItem->m_pObj->m_mutex_ClientOutputQueue);
                    pDevItem->m_pObj->m_clientOutputQueue.push_back(pvscpEvent);
                    pthread_mutex_unlock(
                      &pDevItem->m_pObj->m_mutex_ClientOutputQueue);
                    sem_post(&pDevItem->m_pObj->m_semClientOutputQueue);
                }
            }
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

    CDeviceItem* pDevItem = (CDeviceItem*)pData;
    if (NULL == pDevItem) {
        syslog(LOG_ERR,
               "deviceLevel1WriteThread quitting due to NULL DevItem object.");
        return NULL;
    }

    // Blocking send method must have been found
    if (NULL == pDevItem->m_proc_CanalBlockingSend)
        return NULL;

    while (!pDevItem->m_bQuit) {

        // Wait until there is something to send
        if ((-1 ==
             vscp_sem_wait(&pDevItem->m_pClientItem->m_semClientInputQueue,
                           500)) &&
            errno == ETIMEDOUT) {
            continue;
        }

        if (pDevItem->m_pClientItem->m_clientInputQueue.size()) {

            pthread_mutex_lock(
              &pDevItem->m_pClientItem->m_mutexClientInputQueue);
            vscpEvent* pev =
              pDevItem->m_pClientItem->m_clientInputQueue.front();
            pDevItem->m_pClientItem->m_clientInputQueue.pop_front();
            pthread_mutex_unlock(
              &pDevItem->m_pClientItem->m_mutexClientInputQueue);

            // Trow away event if Level II and Level I interface
            if ((CLIENT_ITEM_INTERFACE_TYPE_DRIVER_LEVEL1 ==
                 pDevItem->m_pClientItem->m_type) &&
                (pev->vscp_class > 512)) {
                vscp_deleteEvent(pev);
                continue;
            }

            canalMsg msg;
            vscp_convertEventToCanal(&msg, pev);
            if (CANAL_ERROR_SUCCESS ==
                pDevItem->m_proc_CanalBlockingSend(pDevItem->m_openHandle,
                                                   &msg,
                                                   300)) {
                vscp_deleteEvent(pev);
            } else {
                // Give it another try
                sem_post(&pDevItem->m_pObj->m_semClientOutputQueue);
            }

        } // events in queue

    } // while

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

    CDeviceItem* pDevItem = (CDeviceItem*)pData;
    if (NULL == pDevItem) {
        syslog(
          LOG_ERR,
          "deviceLevel2ReceiveThread quitting due to NULL DevItem object.");
        return NULL;
    }

    int rv;
    while (!pDevItem->m_bQuit) {

        pev = new vscpEvent;
        if (NULL == pev)
            continue;
        rv = pDevItem->m_proc_VSCPRead(pDevItem->m_openHandle, pev, 500);

        if ((CANAL_ERROR_SUCCESS != rv) || (NULL == pev)) {
            delete pev;
            continue;
        }

        // Identify ourselves
        pev->obid = pDevItem->m_pClientItem->m_clientID;

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
            if (!pDevItem->m_interface_guid.isNULL()) {
                pDevItem->m_interface_guid.writeGUID(pev->GUID);
            } else {
                // If no driver GUID set use interface GUID
                pDevItem->m_pClientItem->m_guid.writeGUID(pev->GUID);
            }

            // Preserve nickname
            pev->GUID[14] = nickname_msb;
            pev->GUID[15] = nickname_lsb;
        }

        // There must be room in the receive queue
        if (pDevItem->m_pObj->m_maxItemsInClientReceiveQueue >
            pDevItem->m_pObj->m_clientOutputQueue.size()) {

            pthread_mutex_lock(&pDevItem->m_pObj->m_mutex_ClientOutputQueue);
            pDevItem->m_pObj->m_clientOutputQueue.push_back(pev);
            pthread_mutex_unlock(&pDevItem->m_pObj->m_mutex_ClientOutputQueue);
            sem_post(&pDevItem->m_pObj->m_semClientOutputQueue);

        } else {
            if (NULL == pev)
                vscp_deleteEvent_v2(&pev);
        }
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
    CDeviceItem* pDevItem = (CDeviceItem*)pData;
    if (NULL == pDevItem) {
        syslog(LOG_ERR,
               "deviceLevel2WriteThread quitting due to NULL DevItem object.");
        return NULL;
    }

    while (!pDevItem->m_bQuit) {

        // Wait until there is something to send
        if ((-1 ==
             vscp_sem_wait(&pDevItem->m_pClientItem->m_semClientInputQueue,
                           500)) &&
            errno == ETIMEDOUT) {
            continue;
        }

        if (pDevItem->m_pClientItem->m_clientInputQueue.size()) {

            pthread_mutex_lock(
              &pDevItem->m_pClientItem->m_mutexClientInputQueue);
            vscpEvent* pev =
              pDevItem->m_pClientItem->m_clientInputQueue.front();
            pthread_mutex_unlock(
              &pDevItem->m_pClientItem->m_mutexClientInputQueue);

            if (CANAL_ERROR_SUCCESS ==
                pDevItem->m_proc_VSCPWrite(pDevItem->m_openHandle, pev, 300)) {

                // Remove the node
                pthread_mutex_lock(
                  &pDevItem->m_pClientItem->m_mutexClientInputQueue);
                pDevItem->m_pClientItem->m_clientInputQueue.pop_front();
                pthread_mutex_unlock(
                  &pDevItem->m_pClientItem->m_mutexClientInputQueue);
            } else {
                // Give it another try
                sem_post(&pDevItem->m_pObj->m_semClientOutputQueue);
            }

        } // events in queue

    } // while

    return NULL;
}
