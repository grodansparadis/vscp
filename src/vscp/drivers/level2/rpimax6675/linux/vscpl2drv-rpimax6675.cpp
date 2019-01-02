// vscpl2drv-max6675.cpp : Defines the initialization routines for the DLL.
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2019 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#include "stdio.h"
#include "stdlib.h"
#include <pthread.h>
#include <semaphore.h>


#include "vscpl2drv-rpimax6675.h"
#include "max6675.h"

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
    m_instanceCounter = 0;
	pthread_mutex_init( &m_objMutex, NULL );

	// Init the driver array
	for (int i = 0; i < VSCP_RPIMAX6675_DRIVER_MAX_OPEN; i++) {
		m_pMax6675Array[ i ] = NULL;
	}

	UNLOCK_MUTEX( m_objMutex );
}

CVSCPDrvApp::~CVSCPDrvApp()
{
	LOCK_MUTEX( m_objMutex );

	for ( int i = 0; i < VSCP_RPIMAX6675_DRIVER_MAX_OPEN; i++ ) {

		if ( NULL == m_pMax6675Array[ i ] ) {

			CRpiMax6675 *prpimax6675 = getDriverObject(i);
			if ( NULL != prpimax6675 ) {
				prpimax6675->close();
				delete m_pMax6675Array[ i ];
				m_pMax6675Array[ i ] = NULL;
			}
		}
	}

	UNLOCK_MUTEX( m_objMutex );
	pthread_mutex_destroy( &m_objMutex );
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CLoggerdllApp object

CVSCPDrvApp theApp;




///////////////////////////////////////////////////////////////////////////////
// CreateObject


///////////////////////////////////////////////////////////////////////////////
// addDriverObject
//

long CVSCPDrvApp::addDriverObject(CRpiMax6675 *pmax6675)
{
	long h = 0;

	LOCK_MUTEX(m_objMutex);
	for ( int i = 0; i < VSCP_RPIMAX6675_DRIVER_MAX_OPEN; i++ ) {

		if ( NULL == m_pMax6675Array[ i ] ) {

			m_pMax6675Array[ i ] = pmax6675;
			h = i + 1681;
			break;

		}

	}

	UNLOCK_MUTEX( m_objMutex );

	return h;
}


///////////////////////////////////////////////////////////////////////////////
// getDriverObject
//

CRpiMax6675 *CVSCPDrvApp::getDriverObject( long h )
{
	long idx = h - 1681;

	// Check if valid handle
	if (idx < 0) return NULL;
	if (idx >= VSCP_RPIMAX6675_DRIVER_MAX_OPEN) return NULL;
	return m_pMax6675Array[ idx ];
}


///////////////////////////////////////////////////////////////////////////////
// removeDriverObject
//

void CVSCPDrvApp::removeDriverObject(long h)
{
	long idx = h - 1681;

	// Check if valid handle
	if (idx < 0) return;
	if (idx >= VSCP_RPIMAX6675_DRIVER_MAX_OPEN) return;

	LOCK_MUTEX(m_objMutex);
	if (NULL != m_pMax6675Array[ idx ]) delete m_pMax6675Array[ idx ];
	m_pMax6675Array[ idx ] = NULL;
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

	CRpiMax6675 *pdrvObj = new CRpiMax6675();
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

	CRpiMax6675 *pdrvObj = theApp.getDriverObject(handle);
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

	CRpiMax6675 *pdrvObj = theApp.getDriverObject(handle);
	if ( NULL == pdrvObj ) return CANAL_ERROR_MEMORY;
    
    pdrvObj->addEvent2SendQueue( pEvent );
    
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
    
	CRpiMax6675 *pdrvObj = theApp.getDriverObject(handle);
	if (NULL == pdrvObj) return CANAL_ERROR_MEMORY;
    
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = timeout * 1e6;
	if ( ETIMEDOUT == sem_timedwait( &pdrvObj->m_semaphore_ReceiveQueue, &ts ) ) {
        return CANAL_ERROR_TIMEOUT;
    }
    
	pthread_mutex_lock( &pdrvObj->m_mutex_ReceiveQueue );
    vscpEvent *pLocalEvent = pdrvObj->m_receiveList.front();
    pdrvObj->m_receiveList.pop_front();
	pthread_mutex_unlock( &pdrvObj->m_mutex_ReceiveQueue );
    if ( NULL == pLocalEvent ) return CANAL_ERROR_MEMORY;
    
    vscp_copyVSCPEvent( pEvent, pLocalEvent );
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
	return VSCP_RPIMAX6675_DRIVERINFO;
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


