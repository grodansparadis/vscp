// vscpl2_loggerdrv.cpp : Defines the initialization routines for the DLL.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (http://www.vscp.org)
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
//

#include <string>
#include <map>
#include <fstream> 

#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"

#include <canal_macro.h>
#include "vscpl2drv_logger.h"
#include "../common/log.h"

void
_init() __attribute__((constructor));
void
_fini() __attribute__((destructor));


// This map holds driver handles/objects
static std::map<long, CVSCPLog *> g_ifMap;

// Mutex for the map object
static pthread_mutex_t g_mapMutex;

////////////////////////////////////////////////////////////////////////////
// DLL constructor
//

void
_init()
{
    pthread_mutex_init(&g_mapMutex, NULL);
}

////////////////////////////////////////////////////////////////////////////
// DLL destructor
//

void
_fini()
{
    // If empty - nothing to do
    if (g_ifMap.empty()) return;

    // Remove orphan objects

    LOCK_MUTEX(g_mapMutex);

    for (std::map<long, CVSCPLog *>::iterator it = g_ifMap.begin();
         it != g_ifMap.end();
         ++it) {
        // std::cout << it->first << " => " << it->second << '\n';

        CVSCPLog *pif = it->second;
        if (NULL != pif) {
            delete pif;
            pif = NULL;
        }
    }

    g_ifMap.clear(); // Remove all items

    UNLOCK_MUTEX(g_mapMutex);
    pthread_mutex_destroy(&g_mapMutex);
}

///////////////////////////////////////////////////////////////////////////////
// addDriverObject
//

long
addDriverObject(CVSCPLog *pif)
{
    std::map<long, CVSCPLog *>::iterator it;
    long h = 0;

    LOCK_MUTEX(g_mapMutex);

    // Find free handle
    while (true) {
        if (g_ifMap.end() == (it = g_ifMap.find(h))) break;
        h++;
    };

    g_ifMap[h] = pif;
    h += 1681;

    UNLOCK_MUTEX(g_mapMutex);

    return h;
}

///////////////////////////////////////////////////////////////////////////////
// getDriverObject
//

CVSCPLog *
getDriverObject(long h)
{
    std::map<long, CVSCPLog *>::iterator it;
    long idx = h - 1681;

    // Check if valid handle
    if (idx < 0) return NULL;

    it = g_ifMap.find(idx);
    if (it != g_ifMap.end()) {
        return it->second;
    }

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// removeDriverObject
//

void
removeDriverObject(long h)
{
    std::map<long, CVSCPLog *>::iterator it;
    long idx = h - 1681;

    // Check if valid handle
    if (idx < 0) return;

    LOCK_MUTEX(g_mapMutex);
    it = g_ifMap.find(idx);
    if (it != g_ifMap.end()) {
        CVSCPLog *pObj = it->second;
        if (NULL != pObj) {
            delete pObj;
            pObj = NULL;
        }
        g_ifMap.erase(it);
    }
    UNLOCK_MUTEX(g_mapMutex);
}






///////////////////////////////////////////////////////////////////////////////
//                         V S C P   D R I V E R -  A P I
///////////////////////////////////////////////////////////////////////////////






///////////////////////////////////////////////////////////////////////////////
// VSCPOpen
//

extern "C" long
VSCPOpen(const char *pUsername,
         const char *pPassword,
         const char *pHost,
         short port,
         const char *pPrefix,
         const char *pParameter,
         unsigned long flags)
{
    long h = 0;

    CVSCPLog *pdrvObj = new CVSCPLog();
    if (NULL != pdrvObj) {

        if (pdrvObj->open(
              pUsername, pPassword, pHost, port, pPrefix, pParameter)) {

            if (!(h = addDriverObject(pdrvObj))) {
                delete pdrvObj;
            }

        } else {
            delete pdrvObj;
        }
    }

    return h;
}

///////////////////////////////////////////////////////////////////////////////
//  VSCPClose
//

extern "C" int
VSCPClose(long handle)
{
    int rv = 0;

    CVSCPLog *pdrvObj = getDriverObject(handle);
    if (NULL == pdrvObj) return 0;
    pdrvObj->close();
    removeDriverObject(handle);
    rv = 1;
    return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
//  VSCPBlockingSend
//

extern "C" int
VSCPBlockingSend(long handle, const vscpEvent *pEvent, unsigned long timeout)
{
    int rv = 0;

    CVSCPLog *pdrvObj = getDriverObject(handle);
    if (NULL == pdrvObj) return CANAL_ERROR_MEMORY;
    pdrvObj->addEvent2SendQueue(pEvent);
    return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
//  VSCPBlockingReceive
//

extern "C" int
VSCPBlockingReceive(long handle, vscpEvent *pEvent, unsigned long timeout)
{
    usleep(timeout*1000);

    // Nothing to receive
    pEvent = NULL;
    return CANAL_ERROR_FIFO_EMPTY;
}

///////////////////////////////////////////////////////////////////////////////
//  VSCPGetLevel
//

extern "C" unsigned long
VSCPGetLevel(void)
{
    return CANAL_LEVEL_USES_TCPIP;
}

///////////////////////////////////////////////////////////////////////////////
// VSCPGetDllVersion
//

extern "C" unsigned long
VSCPGetDllVersion(void)
{
    return VSCP_DLL_VERSION;
}

///////////////////////////////////////////////////////////////////////////////
// VSCPGetVendorString
//

extern "C" const char *
VSCPGetVendorString(void)
{
    return VSCP_DLL_VENDOR;
}

///////////////////////////////////////////////////////////////////////////////
// VSCPGetDriverInfo
//

extern "C" const char *
VSCPGetDriverInfo(void)
{
    return VSCP_LOGGER_DRIVERINFO;
}

///////////////////////////////////////////////////////////////////////////////
//  VSCPGetVSCPGetWebPageTemplate
//

extern "C" long
VSCPGetWebPageTemplate(long handle, const char *url, char *page)
{
    page = NULL;

    // Not implemented
    return -1;
}

///////////////////////////////////////////////////////////////////////////////
//  VSCPGetVSCPWebPageInfo
//

extern "C" int
VSCPGetWebPageInfo(long handle, const struct vscpextwebpageinfo *info)
{
    // Not implemented
    return -1;
}

///////////////////////////////////////////////////////////////////////////////
//  VSCPWebPageupdate
//

extern "C" int
VSCPWebPageupdate(long handle, const char *url)
{
    // Not implemented
    return -1;
}
