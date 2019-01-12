// libvscphelper.cpp : Defines the initialization routines for the DLL.
//
// VSCP (Very Simple Control Protocol)
// http://www.vscp.org
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2019 Ake Hedman,
// Grodans Paradis AB <info@grodansparadis.com>
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
//  This file is part of VSCP - Very Simple Control Protocol
//  http://www.vscp.org
//
//  gcc -fPIC -g -c -I../../common -I../../../common -I../../../.. -Wall libvscphelper.cpp
//

#ifdef __GNUG__
//#pragma implementation
#endif

#include <string>
#include <map>

#include "stdio.h"
#include "stdlib.h"
#include "semaphore.h"

#include <canal.h>
#include <canal_macro.h>
#include <vscpremotetcpif.h>

#include "libvscphelper.h"

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

    g_ifMap.clear();    // Remove all items

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
    while (true ) {
        if ( g_ifMap.end() != ( it = g_ifMap.find(h) ) ) break;
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
