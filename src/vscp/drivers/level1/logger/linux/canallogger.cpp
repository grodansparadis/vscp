// canallogger.cpp : Defines the initialization routines for the DLL.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2013 
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
// Linux
// =====
// device1 = logger,/tmp/canal_log,txt,/usr/local/lib/canallogger.so,64,64,1
//
// WIN32
// =====
// device1 = logger,c:\canal_log,txt,d:\winnr\system32\canallogger.dll,64,64,1

#include "canallogger.h"
#include "stdio.h"
#include "stdlib.h"

void _init() __attribute__((constructor));
void _fini() __attribute__((destructor));

void _init() {
    printf("initializing\n");
}

void _fini() {
    printf("finishing\n");
}


//CLoggerdllApp *gtheapp;

/////////////////////////////////////////////////////////////////////////////
// CLoggerdllApp

////////////////////////////////////////////////////////////////////////////
// CLoggerdllApp construction

CLoggerdllApp::CLoggerdllApp() {
    
    m_instanceCounter = 0;
    pthread_mutex_init(&m_objMutex, NULL);

    // Init the driver array
    for (int i = 0; i < CANAL_LOGGER_DRIVER_MAX_OPEN; i++) {
        m_logArray[ i ] = NULL;
    }

    UNLOCK_MUTEX(m_objMutex);
}

CLoggerdllApp::~CLoggerdllApp() {
    
    LOCK_MUTEX(m_objMutex);

    for (int i = 0; i < CANAL_LOGGER_DRIVER_MAX_OPEN; i++) {

        if (NULL == m_logArray[ i ]) {

            CLog *pLog = getDriverObject(i);
            if (NULL != pLog) {
                pLog->close();
                delete m_logArray[ i ];
                m_logArray[ i ] = NULL;
            }
        }
    }

    UNLOCK_MUTEX(m_objMutex);
    pthread_mutex_destroy(&m_objMutex);
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CLoggerdllApp object

CLoggerdllApp theApp;


///////////////////////////////////////////////////////////////////////////////
// CreateObject

//extern "C" CLoggerdllApp* CreateObject( void ) {
//	CLoggerdllApp *theapp = new CLoggerdllApp;
//	return ( ( CLoggerdllApp * ) theapp );
//}

///////////////////////////////////////////////////////////////////////////////
// addDriverObject
//

long CLoggerdllApp::addDriverObject(CLog *plog) {
    
    long h = 0;

    LOCK_MUTEX(m_objMutex);
    for (int i = 0; i < CANAL_LOGGER_DRIVER_MAX_OPEN; i++) {

        if (NULL == m_logArray[ i ]) {

            m_logArray[ i ] = plog;
            h = i + 1681;
            break;

        }

    }

    UNLOCK_MUTEX(m_objMutex);

    return h;
}


///////////////////////////////////////////////////////////////////////////////
// getDriverObject
//

CLog * CLoggerdllApp::getDriverObject(long h) {
    
    long idx = h - 1681;

    // Check if valid handle
    if (idx < 0) return NULL;
    if (idx >= CANAL_LOGGER_DRIVER_MAX_OPEN) return NULL;
    return m_logArray[ idx ];
}


///////////////////////////////////////////////////////////////////////////////
// removeDriverObject
//

void CLoggerdllApp::removeDriverObject(long h) {
    
    long idx = h - 1681;

    // Check if valid handle
    if (idx < 0) return;
    if (idx >= CANAL_LOGGER_DRIVER_MAX_OPEN) return;

    LOCK_MUTEX(m_objMutex);
    if (NULL != m_logArray[ idx ]) delete m_logArray[ idx ];
    m_logArray[ idx ] = NULL;
    UNLOCK_MUTEX(m_objMutex);
}

///////////////////////////////////////////////////////////////////////////////
// InitInstance

BOOL CLoggerdllApp::InitInstance() {
    m_instanceCounter++;
    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//                             C A N A L -  A P I
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CanalOpen
//

extern "C" long CanalOpen(const char *pDevice, unsigned long flags) {
    
    long h = 0;
    char *pFilter = NULL;
    char *pMask = NULL;

    if (NULL != (pFilter = strchr((char *) pDevice, ';'))) {

        *pFilter = 0;
        pFilter++;

        if (NULL != (pMask = strchr((char *) pDevice, ';'))) {
            *pMask = 0;
            pMask++;
        }
    }

    CLog *plog = new CLog();
    if (NULL != plog) {

        if (plog->open(pDevice, flags)) {

            if (!(h = theApp.addDriverObject(plog))) {
                delete plog;
            } else {

                if (NULL != pFilter) {
                    plog->setFilter(atol(pFilter));
                }

                if (NULL != pMask) {
                    plog->setMask(atol(pMask));
                }
            }

        } else {
            delete plog;
        }

    }

    return h;
}

///////////////////////////////////////////////////////////////////////////////
//  CanalClose
// 

extern "C" int CanalClose(long handle) {
    
    int rv = 0;

    CLog *pLog = theApp.getDriverObject(handle);
    if (NULL == pLog) return 0;
    pLog->close();
    theApp.removeDriverObject(handle);
    rv = 1;
    return rv;
}


///////////////////////////////////////////////////////////////////////////////
//  CanalGetLevel
// 

extern "C" unsigned long CanalGetLevel(long handle) {
    return ( CANAL_LEVEL_STANDARD);
}


///////////////////////////////////////////////////////////////////////////////
// CanalSend
//

extern "C" int CanalSend(long handle, PCANALMSG pCanalMsg) {
    CLog *pLog = theApp.getDriverObject(handle);
    if (NULL == pLog) return 0;
    return pLog->writeMsg(pCanalMsg);
}

///////////////////////////////////////////////////////////////////////////////
// CanalReceive
//

extern "C" int CanalReceive(long handle, PCANALMSG pCanalMsg) {
    int rv = 0;

    // Nothing to receive from this DLL
    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// CanalDataAvailable
//

extern "C" int CanalDataAvailable(long handle) {
    int rv = 0;

    // No data available from this DLL
    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetStatus
//

extern "C" int CanalGetStatus(long handle, PCANALSTATUS pCanalStatus) {
    int rv = 0;

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetStatistics
//

extern "C" int CanalGetStatistics(long handle, PCANALSTATISTICS pCanalStatistics) {
    int rv = 0;

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// CanalSetFilter
//

extern "C" int CanalSetFilter(long handle, unsigned long filter) {
    int rv = 0;
    CLog *pLog = theApp.getDriverObject(handle);
    if (NULL == pLog) return 0;
    pLog->setFilter(filter);
    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// CanalSetMask
//

extern "C" int CanalSetMask(long handle, unsigned long mask) {
    int rv = 0;
    CLog *pLog = theApp.getDriverObject(handle);
    if (NULL == pLog) return 0;
    pLog->setMask(mask);
    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// CanalSetBaudrate
//

extern "C" int CanalSetBaudrate(long handle, unsigned long baudrate) {
    int rv = 0;

    // Not supported in this DLL
    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetVersion
//

extern "C" unsigned long CanalGetVersion(void) {
    unsigned long version;
    unsigned char *p = (unsigned char *) &version;

    *p = CANAL_MAIN_VERSION;
    *(p + 1) = CANAL_MINOR_VERSION;
    *(p + 2) = CANAL_SUB_VERSION;
    *(p + 3) = 0;
    return version;
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetDllVersion
//

extern "C" unsigned long CanalGetDllVersion(void) {
    return DLL_VERSION;
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetVendorString
//

extern "C" const char * CanalGetVendorString(void) {
    return CANAL_DLL_VENDOR;
}



