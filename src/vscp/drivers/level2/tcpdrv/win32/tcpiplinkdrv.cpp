//
// tcpdrv.cpp : Defines the entry point for the DLL application.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2017 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
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

#define __USE_W32_SOCKETS
#include <winsock2.h> 

//#define WIN32_LEAN_AND_MEAN
#include "wx/wxprec.h"
#include "wx/wx.h"
//#include <windows.h>
//#include <stdlib.h>




#ifdef WIN32
#include <stdio.h>
#include <stdlib.h>
//#include "winsock.h"
//#include <winsock2.h>

#endif

#include "wx/defs.h"
#include "wx/app.h"
#include <wx/xml/xml.h>
#include <wx/listimpl.cpp>
#include <wx/thread.h>
#include <wx/tokenzr.h>
#include <wx/datetime.h>
#include <wx/utils.h>



#include <vscphelper.h>
#include <vscpremotetcpif.h>
#include "../common/vscptcpiplink.h"
#include "tcpiplinkdrv.h"

static HANDLE hThisInstDll = NULL;
static CVSCPDrvApp *theApp = NULL;

#pragma comment(lib, "Irprops.lib")


///////////////////////////////////////////////////////////////////////////////
// DllMain
//

BOOL APIENTRY DllMain( HANDLE hInstDll, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved )
{
    switch( ul_reason_for_call ) {

	    case DLL_PROCESS_ATTACH:
            ::wxInitialize();
			hThisInstDll = hInstDll;
			theApp = new CVSCPDrvApp();
			theApp->InitInstance();
			break;

		case DLL_THREAD_ATTACH:
			if ( NULL == theApp ) delete theApp;
			break;

		case DLL_THREAD_DETACH:
 			break;

		case DLL_PROCESS_DETACH:
			::wxUninitialize();
 			break;
   }

	return TRUE;

}



////////////////////////////////////////////////////////////////////////////
// CVSCPDrvApp construction

CVSCPDrvApp::CVSCPDrvApp()
{
	m_instanceCounter = 0;
    m_objMutex = CreateMutex( NULL, true, _("__VSCP_VSCPTCPIPLINK_MUTEX__") );

	// Init the driver array
	for (int i = 0; i < VSCP_TCPIPLINK_DRIVER_MAX_OPEN; i++) {
		m_drvObjArray[ i ] = NULL;
	}

	UNLOCK_MUTEX(m_objMutex);
}

CVSCPDrvApp::~CVSCPDrvApp()
{
	LOCK_MUTEX(m_objMutex);

	for (int i = 0; i < VSCP_TCPIPLINK_DRIVER_MAX_OPEN; i++) {

		if (NULL != m_drvObjArray[ i ]) {

			CTcpipLink *psockcan = getDriverObject(i);
			if (NULL != psockcan) {
				psockcan->close();
				delete m_drvObjArray[ i ];
				m_drvObjArray[ i ] = NULL;
			}
		}
	}

	UNLOCK_MUTEX(m_objMutex);
	CloseHandle( m_objMutex );
}



///////////////////////////////////////////////////////////////////////////////
// CreateObject


///////////////////////////////////////////////////////////////////////////////
// addDriverObject
//

long CVSCPDrvApp::addDriverObject(CTcpipLink *psockcan)
{
	long h = 0;

	LOCK_MUTEX(m_objMutex);
	for (int i = 0; i < VSCP_TCPIPLINK_DRIVER_MAX_OPEN; i++) {

		if (NULL == m_drvObjArray[ i ]) {

			m_drvObjArray[ i ] = psockcan;
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

CTcpipLink *CVSCPDrvApp::getDriverObject(long h)
{
	long idx = h - 1681;

	// Check if valid handle
	if (idx < 0) return NULL;
	if (idx >= VSCP_TCPIPLINK_DRIVER_MAX_OPEN) return NULL;
	return m_drvObjArray[ idx ];
}


///////////////////////////////////////////////////////////////////////////////
// removeDriverObject
//

void CVSCPDrvApp::removeDriverObject(long h)
{
	long idx = h - 1681;

	// Check if valid handle
	if (idx < 0) return;
	if (idx >= VSCP_TCPIPLINK_DRIVER_MAX_OPEN) return;

	LOCK_MUTEX(m_objMutex);
	if (NULL != m_drvObjArray[ idx ]) delete m_drvObjArray[ idx ];
	m_drvObjArray[ idx ] = NULL;
	UNLOCK_MUTEX(m_objMutex);
}

///////////////////////////////////////////////////////////////////////////////
// InitInstance

BOOL CVSCPDrvApp::InitInstance()
{
	m_instanceCounter++;
	return TRUE;
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

	CTcpipLink *pdrvObj = new CTcpipLink();
	if (NULL != pdrvObj) {

		if (pdrvObj->open(pUsername,
                            pPassword,
                            pHost,
                            port,
                            pPrefix,
                            pParameter)) {

			if (!(h = theApp->addDriverObject(pdrvObj))) {
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
//  VSCPClose
// 

extern "C" int
VSCPClose(long handle)
{
	int rv = 0;

	CTcpipLink *pdrvObj = theApp->getDriverObject(handle);
	if (NULL == pdrvObj) return 0;
	pdrvObj->close();
	theApp->removeDriverObject(handle);
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

	CTcpipLink *pdrvObj = theApp->getDriverObject(handle);
	if (NULL == pdrvObj) return CANAL_ERROR_MEMORY;
    
    //vscpEvent *pEventNew = new vscpEvent;
    //if ( NULL != pEventNew ) {
    //    copyVSCPEvent( pEventNew, pEvent );
    pdrvObj->addEvent2SendQueue( pEvent );
	//}
    //else {
    //    return CANAL_ERROR_MEMORY;
    //}
    
	return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
//  VSCPBlockingReceive
// 

extern "C" int
VSCPBlockingReceive(long handle, vscpEvent *pEvent, unsigned long timeout)
{
	int rv = 0;
 
    // Check pointer
    if ( NULL == pEvent) return CANAL_ERROR_PARAMETER;
    
	CTcpipLink *pdrvObj = theApp->getDriverObject(handle);
	if (NULL == pdrvObj) return CANAL_ERROR_MEMORY;
    
    if ( wxSEMA_TIMEOUT == pdrvObj->m_semReceiveQueue.WaitTimeout( timeout ) ) {
        return CANAL_ERROR_TIMEOUT;
    }
    
    //VSCPEVENTLIST_RECEIVE::compatibility_iterator nodeClient;

	pdrvObj->m_mutexReceiveQueue.Lock();
	//nodeClient = pdrvObj->m_receiveQueue.GetFirst();
	//vscpEvent *pLocalEvent = nodeClient->GetData();
    vscpEvent *pLocalEvent = pdrvObj->m_receiveList.front();
    pdrvObj->m_receiveList.pop_front();
	pdrvObj->m_mutexReceiveQueue.Unlock();
    if (NULL == pLocalEvent) return CANAL_ERROR_MEMORY;
    
    vscp_copyVSCPEvent( pEvent, pLocalEvent );
    //pdrvObj->m_receiveQueue.DeleteNode(nodeClient);
    vscp_deleteVSCPevent( pLocalEvent );
	
	return CANAL_ERROR_SUCCESS;
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
	return VSCP_TCPIPLINK_DRIVERINFO;
}


///////////////////////////////////////////////////////////////////////////////
//  VSCPGetVSCPGetWebPageTemplate
// 

extern "C" long
VSCPGetWebPageTemplate( long handle, const char *url, char *page )
{
    page = NULL;
    
    // Not implemented
	return -1;
}


///////////////////////////////////////////////////////////////////////////////
//  VSCPGetVSCPWebPageInfo
// 

extern "C" int
VSCPGetWebPageInfo( long handle, const struct vscpextwebpageinfo *info )
{
    // Not implemented
	return -1;
}


///////////////////////////////////////////////////////////////////////////////
//  VSCPWebPageupdate
// 

extern "C" int
VSCPWebPageupdate( long handle, const char *url )
{
    // Not implemented
	return -1;
}

