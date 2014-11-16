// libvscphelper.cpp : Defines the initialization routines for the DLL.
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

#include "libvscphelper.h"
#include "stdio.h"
#include "stdlib.h"

void _init() __attribute__((constructor));
void _fini() __attribute__((destructor));

void _init()
{
    wxLogDebug(_("initializing"));
	// The following works on 3.0 but not on 2.8
	//wxApp::SetInstance(new wxApp());
	//new wxInitializer();
	wxInitialize();
}

void _fini()
{
    wxLogDebug(_("finishing"));	
	wxUninitialize();
}



////////////////////////////////////////////////////////////////////////////
// CVSCPLApp construction

CVSCPLApp::CVSCPLApp()
{
	wxLogDebug(_("application constructor"));
	
    m_instanceCounter = 0;
    pthread_mutex_init(&m_objMutex, NULL);

    // Init. the driver array
    for (int i = 0; i < VSCP_INTERFACE_MAX_OPEN; i++) {
        m_pvscpifArray[ i ] = NULL;
    }

    UNLOCK_MUTEX(m_objMutex);
	wxLogDebug(_("constructed"));
}

CVSCPLApp::~CVSCPLApp()
{
    LOCK_MUTEX(m_objMutex);

    for (int i = 0; i < VSCP_INTERFACE_MAX_OPEN; i++) {

        if (NULL == m_pvscpifArray[ i ]) {

            VscpRemoteTcpIf *pvscpif = getDriverObject(i);
            if (NULL != pvscpif) {
                pvscpif->doCmdClose();
                delete m_pvscpifArray[ i ];
                m_pvscpifArray[ i ] = NULL;
            }
        }
    }

    UNLOCK_MUTEX(m_objMutex);
    pthread_mutex_destroy(&m_objMutex);
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CLoggerdllApp object

CVSCPLApp theApp;




///////////////////////////////////////////////////////////////////////////////
// CreateObject


///////////////////////////////////////////////////////////////////////////////
// addDriverObject
//

long CVSCPLApp::addDriverObject(VscpRemoteTcpIf *pvscpif)
{
    long h = 0;

    LOCK_MUTEX(m_objMutex);
    for (int i = 0; i < VSCP_INTERFACE_MAX_OPEN; i++) {

        if (NULL == m_pvscpifArray[ i ]) {

            m_pvscpifArray[ i ] = pvscpif;
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

VscpRemoteTcpIf *CVSCPLApp::getDriverObject(long h)
{
    long idx = h - 1681;

    // Check if valid handle
    if (idx < 0) return NULL;
    if (idx >= VSCP_INTERFACE_MAX_OPEN) return NULL;
    return m_pvscpifArray[ idx ];
}


///////////////////////////////////////////////////////////////////////////////
// removeDriverObject
//

void CVSCPLApp::removeDriverObject(long h)
{
    long idx = h - 1681;

    // Check if valid handle
    if (idx < 0) return;
    if (idx >= VSCP_INTERFACE_MAX_OPEN) return;

    LOCK_MUTEX(m_objMutex);
    if (NULL != m_pvscpifArray[ idx ]) delete m_pvscpifArray[ idx ];
    m_pvscpifArray[ idx ] = NULL;
    UNLOCK_MUTEX(m_objMutex);
}

///////////////////////////////////////////////////////////////////////////////
// InitInstance

BOOL CVSCPLApp::InitInstance()
{
	printf("InitInstance");
    m_instanceCounter++;
	wxInitialize();
	printf("InitInstance done");
    return TRUE;
}

