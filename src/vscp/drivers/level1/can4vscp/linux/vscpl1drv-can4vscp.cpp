// vscpl1drv_can4vscp.cpp : Defines the initialization routines for the DLL.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2018
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


#include "vscpl1drv-can4vscp.h"
#include "stdio.h"
#include "stdlib.h"

void _init() __attribute__((constructor));
void _fini() __attribute__((destructor));

void
_init()
{
    printf("initializing\n");
}

void
_fini()
{
    printf("finishing\n");
}

////////////////////////////////////////////////////////////////////////////
// Ccan4vscpApp
////////////////////////////////////////////////////////////////////////////
// Ccan4vscpApp construction

Ccan4vscpApp::Ccan4vscpApp()
{
    m_instanceCounter = 0;
    pthread_mutex_init(&m_drvobjMutex, NULL);
    
    // Init. the driver array
    for (int i = 0; i < CANAL_CAN4VSCP_DRIVER_MAX_OPEN; i++) {
        m_socketcanArray[ i ] = NULL;
    }

    UNLOCK_MUTEX(m_drvobjMutex);
}

Ccan4vscpApp::~Ccan4vscpApp()
{
    LOCK_MUTEX(m_drvobjMutex);

    for (int i = 0; i < CANAL_CAN4VSCP_DRIVER_MAX_OPEN; i++) {

        if (NULL != m_socketcanArray[ i ]) {
            CCan4VSCPObj *pCAN232Obj = getDriverObject(i);

            if (NULL != pCAN232Obj) {
                pCAN232Obj->close();
                delete m_socketcanArray[ i ];
                m_socketcanArray[ i ] = NULL;
            }
        }
    }

    UNLOCK_MUTEX(m_drvobjMutex);
    pthread_mutex_destroy(&m_drvobjMutex);
}


/////////////////////////////////////////////////////////////////////////////
// The one and only Ccan4vscpApp object

Ccan4vscpApp theApp;


///////////////////////////////////////////////////////////////////////////////
// addDriverObject
//

//long Ccan4vscpApp::addDriverObject( CLog *plog )

long
Ccan4vscpApp::addDriverObject(CCan4VSCPObj *plog)
{
    long h = 0;

    LOCK_MUTEX(m_drvobjMutex);
    for (int i = 0; i < CANAL_CAN4VSCP_DRIVER_MAX_OPEN; i++) {

        if ( NULL == m_socketcanArray[ i ] ) {

            m_socketcanArray[ i ] = plog;
            h = i + 1681;
            break;
        }
    }
    UNLOCK_MUTEX(m_drvobjMutex);

    return h;
}


///////////////////////////////////////////////////////////////////////////////
// getDriverObject
//

//CLog * Ccan4vscpApp::getDriverObject( long h )

CCan4VSCPObj *
Ccan4vscpApp::getDriverObject(long h)
{
    long idx = h - 1681;

    // Check if valid handle
    if (idx < 0) return NULL;
    if (idx >= CANAL_CAN4VSCP_DRIVER_MAX_OPEN) return NULL;
    return m_socketcanArray[ idx ];
}


///////////////////////////////////////////////////////////////////////////////
// removeDriverObject
//

void
Ccan4vscpApp::removeDriverObject(long h)
{
    long idx = h - 1681;

    // Check if valid handle
    if (idx < 0) return;
    if (idx >= CANAL_CAN4VSCP_DRIVER_MAX_OPEN) return;

    LOCK_MUTEX(m_drvobjMutex);
    if (NULL != m_socketcanArray[ idx ]) delete m_socketcanArray[ idx ];
    m_socketcanArray[ idx ] = NULL;
    UNLOCK_MUTEX(m_drvobjMutex);
}

///////////////////////////////////////////////////////////////////////////////
// InitInstance

BOOL
Ccan4vscpApp::InitInstance()
{
    m_instanceCounter++;
    return TRUE;
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
    long h = 0;

    CCan4VSCPObj *pdrvObj = new CCan4VSCPObj();
    if (NULL != pdrvObj) {
        if ( CANAL_ERROR_SUCCESS == pdrvObj->open( pDevice, flags ) ) {
            if (!(h = theApp.addDriverObject(pdrvObj))) {
                delete pdrvObj;
            }
        } 
        else {
            delete pdrvObj;
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

    CCan4VSCPObj *pLog = theApp.getDriverObject(handle);
    if (NULL == pLog) return 0;

    pLog->close();
    theApp.removeDriverObject(handle);

    rv = 1;
    return CANAL_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
//  CanalGetLevel
// 

extern "C" unsigned long
CanalGetLevel(long handle)
{
    return CANAL_LEVEL_STANDARD;
}


///////////////////////////////////////////////////////////////////////////////
// CanalSend
//

extern "C" int
CanalSend(long handle, PCANALMSG pCanalMsg)
{
    CCan4VSCPObj *pdrvObj = theApp.getDriverObject(handle);
	
    if (NULL == pdrvObj) return 0;
    return pdrvObj->writeMsg( pCanalMsg );
}

///////////////////////////////////////////////////////////////////////////////
// CanalBlockingSend
//

extern "C" int
CanalBlockingSend(long handle, PCANALMSG pCanalMsg, unsigned long to)
{
    CCan4VSCPObj *pdrvObj = theApp.getDriverObject(handle);

    if (NULL == pdrvObj) return 0;
    return pdrvObj->writeMsgBlocking( pCanalMsg, to );
}

///////////////////////////////////////////////////////////////////////////////
// CanalReceive
//

extern "C" int
CanalReceive(long handle, PCANALMSG pCanalMsg)
{
    CCan4VSCPObj *pdrvObj = theApp.getDriverObject(handle);

    if (NULL == pdrvObj) return 0;
    return pdrvObj->readMsg( pCanalMsg );
}

///////////////////////////////////////////////////////////////////////////////
// CanalBlockingReceive
//

extern "C" int
CanalBlockingReceive(long handle, PCANALMSG pCanalMsg, unsigned long to )
{
    CCan4VSCPObj *pdrvObj = theApp.getDriverObject(handle);

    if (NULL == pdrvObj) return 0;
    return pdrvObj->readMsgBlocking( pCanalMsg, to );
}

///////////////////////////////////////////////////////////////////////////////
// CanalDataAvailable
//

extern "C" int
CanalDataAvailable(long handle)
{
    CCan4VSCPObj *pdrvObj = theApp.getDriverObject(handle);

    if (NULL == pdrvObj) return 0;
    return pdrvObj->dataAvailable();
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetStatus
//

extern "C" int
CanalGetStatus(long handle, PCANALSTATUS pCanalStatus)
{
    CCan4VSCPObj *pdrvObj = theApp.getDriverObject(handle);

    if (NULL == pdrvObj) return 0;
    return pdrvObj->getStatus( pCanalStatus );

}

///////////////////////////////////////////////////////////////////////////////
// CanalGetStatistics
//

extern "C" int
CanalGetStatistics(long handle, PCANALSTATISTICS pCanalStatistics)
{
    CCan4VSCPObj *pdrvObj = theApp.getDriverObject(handle);

    if (NULL == pdrvObj) return 0;
    return pdrvObj->getStatistics( pCanalStatistics );

}

///////////////////////////////////////////////////////////////////////////////
// CanalSetFilter
//

extern "C" int
CanalSetFilter(long handle, unsigned long filter)
{
    CCan4VSCPObj *pdrvObj = theApp.getDriverObject(handle);

    if (NULL == pdrvObj) return 0;
    return pdrvObj->setFilter( filter );
}

///////////////////////////////////////////////////////////////////////////////
// CanalSetMask
//

extern "C" int
CanalSetMask(long handle, unsigned long mask)
{
    CCan4VSCPObj *pdrvObj = theApp.getDriverObject(handle);

    if (NULL == pdrvObj) return 0;
    return pdrvObj->setMask( mask );
}

///////////////////////////////////////////////////////////////////////////////
// CanalSetBaudrate
//

extern "C" int
CanalSetBaudrate(long handle, unsigned long baudrate)
{
    // Not supported in this DLL
    return CANAL_ERROR_NOT_SUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetVersion
//

extern "C" unsigned long
CanalGetVersion(void)
{
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

extern "C" unsigned long
CanalGetDllVersion(void)
{
    return DLL_VERSION;
}

///////////////////////////////////////////////////////////////////////////////
// CanalGetVendorString
//

extern "C" const char *
CanalGetVendorString(void)
{
    return CANAL_DLL_VENDOR;
}


///////////////////////////////////////////////////////////////////////////////
// CanalGetDriverInfo
//

extern "C" const char *
CanalGetDriverInfo(void)
{
    return CANAL_DLL_VENDOR;
}
