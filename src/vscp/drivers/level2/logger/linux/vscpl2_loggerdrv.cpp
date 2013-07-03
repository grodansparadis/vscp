// vscpl2_loggerdrv.cpp : Defines the initialization routines for the DLL.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2013 Ake Hedman, 
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

#include "stdio.h"
#include "stdlib.h"
#include "../common/log.h"
#include "vscpl2_loggerdrv.h"

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


//CLoggerApp *gtheapp;

/////////////////////////////////////////////////////////////////////////////
// CLoggerApp

////////////////////////////////////////////////////////////////////////////
// CLoggerApp construction

CLoggerApp::CLoggerApp()
{

	m_instanceCounter = 0;
	pthread_mutex_init(&m_objMutex, NULL);

	// Init the driver array
	for (int i = 0; i < VSCP_LOGGER_DRIVER_MAX_OPEN; i++) {
		m_logArray[ i ] = NULL;
	}

	UNLOCK_MUTEX(m_objMutex);
}

CLoggerApp::~CLoggerApp()
{

	LOCK_MUTEX(m_objMutex);

	for (int i = 0; i < VSCP_LOGGER_DRIVER_MAX_OPEN; i++) {

		if (NULL == m_logArray[ i ]) {

			CVSCPLog *pLog = getDriverObject(i);
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
// The one and only CLoggerApp object

CLoggerApp theApp;

///////////////////////////////////////////////////////////////////////////////
// CreateObject

//extern "C" CLoggerApp* CreateObject( void ) {
//	CLoggerApp *theapp = new CLoggerApp;
//	return ( ( CLoggerApp * ) theapp );
//}

///////////////////////////////////////////////////////////////////////////////
// addDriverObject
//

long
CLoggerApp::addDriverObject(CVSCPLog *plog)
{

	long h = 0;

	LOCK_MUTEX(m_objMutex);
	for (int i = 0; i < VSCP_LOGGER_DRIVER_MAX_OPEN; i++) {

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

CVSCPLog *
CLoggerApp::getDriverObject(long h)
{

	long idx = h - 1681;

	// Check if valid handle
	if (idx < 0) return NULL;
	if (idx >= VSCP_LOGGER_DRIVER_MAX_OPEN) return NULL;
	return m_logArray[ idx ];
}


///////////////////////////////////////////////////////////////////////////////
// removeDriverObject
//

void
CLoggerApp::removeDriverObject(long h)
{

	long idx = h - 1681;

	// Check if valid handle
	if (idx < 0) return;
	if (idx >= VSCP_LOGGER_DRIVER_MAX_OPEN) return;

	LOCK_MUTEX(m_objMutex);
	if (NULL != m_logArray[ idx ]) delete m_logArray[ idx ];
	m_logArray[ idx ] = NULL;
	UNLOCK_MUTEX(m_objMutex);
}

///////////////////////////////////////////////////////////////////////////////
// InitInstance

BOOL
CLoggerApp::InitInstance()
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

	CVSCPLog *pdrvObj = new CVSCPLog();
	if (NULL != pdrvObj) {

		if (pdrvObj->open(pUsername,
			pPassword,
			pHost,
			port,
			pPrefix,
			pParameter )) {

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

	CVSCPLog *pdrvObj = theApp.getDriverObject(handle);
	if (NULL == pdrvObj) return 0;
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

	CVSCPLog *pdrvObj = theApp.getDriverObject(handle);
	if (NULL == pdrvObj) return 0;
    
    pdrvObj->addEvent2Queue( pEvent );
	
	return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
//  VSCPBlockingReceive
// 

extern "C" int
VSCPBlockingReceive(long handle, vscpEvent *pEvent, unsigned long timeout)
{
	int rv = 0;

	CVSCPLog *pdrvObj = theApp.getDriverObject(handle);
	if (NULL == pdrvObj) return 0;
	
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
	return VSCP_LOGGER_DRIVERINFO;
}
