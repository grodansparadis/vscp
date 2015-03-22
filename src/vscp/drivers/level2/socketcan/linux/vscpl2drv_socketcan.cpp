// vscp2drv_socketcan.cpp : Defines the initialization routines for the DLL.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2014 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
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

#ifdef __GNUG__
//#pragma implementation
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef __WXMSW__
#include  "wx/ownerdrw.h"
#endif

#include "wx/tokenzr.h"

#include "stdio.h"
#include "stdlib.h"

#include "vscpl2drv_socketcan.h"
#include "socketcan.h"

void _init() __attribute__((constructor));
void _fini() __attribute__((destructor));

void _init()
{
	printf("initializing\n");
}

void _fini()
{
	printf("finishing\n");
}



////////////////////////////////////////////////////////////////////////////
// CVSCPDrvApp construction

CVSCPDrvApp::CVSCPDrvApp()
{
    ::wxInitialize();
    
	m_instanceCounter = 0;
	pthread_mutex_init(&m_objMutex, NULL);

	// Init the driver array
	for (int i = 0; i < VSCP_SOCKETCAN_DRIVER_MAX_OPEN; i++) {
		m_psockcanArray[ i ] = NULL;
	}

	UNLOCK_MUTEX(m_objMutex);
}

CVSCPDrvApp::~CVSCPDrvApp()
{
	LOCK_MUTEX(m_objMutex);

	for (int i = 0; i < VSCP_SOCKETCAN_DRIVER_MAX_OPEN; i++) {

		if (NULL != m_psockcanArray[ i ]) {

			Csocketcan *psockcan = getDriverObject(i);
			if (NULL != psockcan) {
				psockcan->close();
				delete m_psockcanArray[ i ];
				m_psockcanArray[ i ] = NULL;
			}
		}
	}

	UNLOCK_MUTEX(m_objMutex);
	pthread_mutex_destroy(&m_objMutex);
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CLoggerdllApp object

CVSCPDrvApp theApp;




///////////////////////////////////////////////////////////////////////////////
// CreateObject


///////////////////////////////////////////////////////////////////////////////
// addDriverObject
//

long CVSCPDrvApp::addDriverObject(Csocketcan *psockcan)
{
	long h = 0;

	LOCK_MUTEX(m_objMutex);
	for (int i = 0; i < VSCP_SOCKETCAN_DRIVER_MAX_OPEN; i++) {

		if (NULL == m_psockcanArray[ i ]) {

			m_psockcanArray[ i ] = psockcan;
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

Csocketcan *CVSCPDrvApp::getDriverObject(long h)
{
	long idx = h - 1681;

	// Check if valid handle
	if (idx < 0) return NULL;
	if (idx >= VSCP_SOCKETCAN_DRIVER_MAX_OPEN) return NULL;
	return m_psockcanArray[ idx ];
}


///////////////////////////////////////////////////////////////////////////////
// removeDriverObject
//

void CVSCPDrvApp::removeDriverObject(long h)
{
	long idx = h - 1681;

	// Check if valid handle
	if (idx < 0) return;
	if (idx >= VSCP_SOCKETCAN_DRIVER_MAX_OPEN) return;

	LOCK_MUTEX(m_objMutex);
	if (NULL != m_psockcanArray[ idx ]) delete m_psockcanArray[ idx ];
	m_psockcanArray[ idx ] = NULL;
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

	Csocketcan *pdrvObj = new Csocketcan();
	if (NULL != pdrvObj) {

		if (pdrvObj->open(pUsername,
							pPassword,
							pHost,
							port,
							pPrefix,
							pParameter)) {

			if (!(h = theApp.addDriverObject(pdrvObj))) {
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

	Csocketcan *pdrvObj = theApp.getDriverObject(handle);
	if (NULL == pdrvObj) return CANAL_ERROR_MEMORY;
	pdrvObj->close();
	theApp.removeDriverObject(handle);
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

	Csocketcan *pdrvObj = theApp.getDriverObject(handle);
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
    
	Csocketcan *pdrvObj = theApp.getDriverObject(handle);
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
	return VSCP_SOCKETCAN_DRIVERINFO;
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


