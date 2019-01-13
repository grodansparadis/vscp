// vscpl1.cpp : Defines the initialization routines for the DLL.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (http://www.vscp.org)
//
// Copyright (C) 2007 Johan Hedlund,  <kungjohan@gmail.com>
//
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
//
// Linux
// =====
// device1 = logger,/tmp/canal_log,txt,/usr/local/lib/canallogger.so,64,64,1
//
// WIN32
// =====
// device1 = logger,c:\canal_log,txt,d:\winnr\system32\canallogger.dll,64,64,1

#ifdef __GNUG__
//#pragma implementation
#endif

#include <map>
#include <string>

#include "semaphore.h"
#include "stdio.h"
#include "stdlib.h"

#include <canal.h>
#include <canal_macro.h>
#include <vscpremotetcpif.h>

#include "vscpl1.h"

void
_init() __attribute__((constructor));
void
_fini() __attribute__((destructor));

// This map holds driver handles/objects
static std::map<long, VscpRemoteTcpIf *> g_ifMap;

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

    for (std::map<long, VscpRemoteTcpIf *>::iterator it = g_ifMap.begin();
         it != g_ifMap.end();
         ++it) {
        // std::cout << it->first << " => " << it->second << '\n';

        VscpRemoteTcpIf *pvscpif = it->second;
        if (NULL != pvscpif) {
            pvscpif->doCmdClose();
            delete pvscpif;
            pvscpif = NULL;
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
addDriverObject(VscpRemoteTcpIf *pvscpif)
{
    std::map<long, VscpRemoteTcpIf *>::iterator it;
    long h = 0;

    LOCK_MUTEX(g_mapMutex);

    // Find free handle
    while (true) {
        if (g_ifMap.end() == (it = g_ifMap.find(h))) break;
        h++;
    };

    g_ifMap[h] = pvscpif;
    h += 1681;

    UNLOCK_MUTEX(g_mapMutex);

    return h;
}

///////////////////////////////////////////////////////////////////////////////
// getDriverObject
//

VscpRemoteTcpIf *
getDriverObject(long h)
{
    std::map<long, VscpRemoteTcpIf *>::iterator it;
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
    std::map<long, VscpRemoteTcpIf *>::iterator it;
    long idx = h - 1681;

    // Check if valid handle
    if (idx < 0) return;

    LOCK_MUTEX(g_mapMutex);
    it = g_ifMap.find(idx);
    if (it != g_ifMap.end()) {
        VscpRemoteTcpIf *pObj = it->second;
        if (NULL != pObj) {
            delete pObj;
            pObj = NULL;
        }
        g_ifMap.erase(it);
    }
    UNLOCK_MUTEX(g_mapMutex);
}

///////////////////////////////////////////////////////////////////////////////
//                             C A N A L -  A P I
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CanalOpen
//

extern "C" long
CanalOpen(const char *pDevice, unsigned long flags)
{
    long h               = CANAL_ERROR_SUB_DRIVER;
    unsigned long filter = 0, mask = 0;
    bool bFilter = false, bMask = false;
    std::string str;
    std::string strDevice(pDevice);

    std::deque<std::string> tokens;
    vscp_split(tokens, strDevice, ";");

    // Get possible filter
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        if (0 != str.size()) {
            filter = vscp_readStringValue(str);
        }
    }

    // Get possible mask
    if (!tokens.empty()) {
        str = tokens.front();
        tokens.pop_front();
        if (0 != str.size()) {
            mask = vscp_readStringValue(str);
        }
    }

    VscpRemoteTcpIf *pvscpif = new VscpRemoteTcpIf();
    if (NULL != pvscpif) {

        if (pvscpif->doCmdOpen(strDevice, flags)) {

            if (!(h = addDriverObject(pvscpif))) {
                delete pvscpif;
            } else {

                if (bFilter) {
                    pvscpif->doCmdFilter(filter);
                }

                if (bMask) {
                    pvscpif->doCmdMask(mask);
                }
            }

        } else {
            delete pvscpif;
        }
    }

    return h;
}

///////////////////////////////////////////////////////////////////////////////
//  CanalClose
//

extern "C" int
CanalClose(long handle)
{
    int rv = 0;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_MEMORY;
    pvscpif->doCmdClose();
    removeDriverObject(handle);
    rv = CANAL_ERROR_SUCCESS;
    return rv;
}

///////////////////////////////////////////////////////////////////////////////
//  CanalGetLevel
//

extern "C" unsigned long
CanalGetLevel(long handle)
{
    unsigned long level;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_MEMORY;

    level = pvscpif->doCmdGetLevel();
    return level;
}

///////////////////////////////////////////////////////////////////////////////
// CanalSend
//

extern "C" int
CanalSend(long handle, PCANALMSG pCanalMsg)
{
    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_MEMORY;

    return (pvscpif->doCmdSendLevel1(pCanalMsg) ? CANAL_ERROR_SUCCESS
                                                : CANAL_ERROR_SUB_DRIVER);
}

///////////////////////////////////////////////////////////////////////////////
// CanalReceive
//

extern "C" int
CanalReceive(long handle, PCANALMSG pCanalMsg)
{
    int rv = 0;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_MEMORY;

    return (pvscpif->doCmdReceiveLevel1(pCanalMsg) ? CANAL_ERROR_SUCCESS
                                                   : CANAL_ERROR_SUB_DRIVER);
}

///////////////////////////////////////////////////////////////////////////////
// CanalDataAvailable
//

extern "C" int
CanalDataAvailable(long handle)
{
    int rv = 0;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_MEMORY;

    return (pvscpif->doCmdDataAvailable() ? CANAL_ERROR_SUCCESS
                                          : CANAL_ERROR_SUB_DRIVER);
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetStatus
//

extern "C" int
CanalGetStatus(long handle, PCANALSTATUS pCanalStatus)
{
    int rv = 0;

    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_MEMORY;

    return (pvscpif->doCmdStatus(pCanalStatus) ? CANAL_ERROR_SUCCESS
                                               : CANAL_ERROR_SUB_DRIVER);
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetStatistics
//

extern "C" int
CanalGetStatistics(long handle, PCANALSTATISTICS pCanalStatistics)
{
    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_MEMORY;

    return (pvscpif->doCmdStatistics(pCanalStatistics)
              ? CANAL_ERROR_SUCCESS
              : CANAL_ERROR_SUB_DRIVER);
}

///////////////////////////////////////////////////////////////////////////////
// CanalSetFilter
//

extern "C" int
CanalSetFilter(long handle, unsigned long filter)
{
    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_MEMORY;
    return (pvscpif->doCmdFilter(filter) ? CANAL_ERROR_SUCCESS
                                         : CANAL_ERROR_SUB_DRIVER);
}

///////////////////////////////////////////////////////////////////////////////
// CanalSetMask
//

extern "C" int
CanalSetMask(long handle, unsigned long mask)
{
    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_MEMORY;
    return (pvscpif->doCmdMask(mask) ? CANAL_ERROR_SUCCESS
                                     : CANAL_ERROR_SUB_DRIVER);
}

///////////////////////////////////////////////////////////////////////////////
// CanalSetBaudrate
//

extern "C" int
CanalSetBaudrate(long handle, unsigned long baudrate)
{
    VscpRemoteTcpIf *pvscpif = getDriverObject(handle);
    if (NULL == pvscpif) return CANAL_ERROR_MEMORY;

    return (pvscpif->doCmdSetBaudrate(baudrate) ? CANAL_ERROR_SUCCESS
                                                : CANAL_ERROR_SUB_DRIVER);
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetVersion
//

extern "C" unsigned long
CanalGetVersion(void)
{
    unsigned long version;
    unsigned char *p = (unsigned char *)&version;

    *p       = CANAL_MAIN_VERSION;
    *(p + 1) = CANAL_MINOR_VERSION;
    *(p + 2) = CANAL_SUB_VERSION;
    *(p + 3) = 0;
    return version;
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetDllVersion
//

extern "C" unsigned long
CanalGetDllVersion(void)
{
    return VSCP_DLL_VERSION;
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetVendorString
//

extern "C" const char *
CanalGetVendorString(void)
{
    return VSCP_DLL_VENDOR;
}
