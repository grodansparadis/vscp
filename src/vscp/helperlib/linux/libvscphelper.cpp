// libvscphelper.cpp : Defines the initialization routines for the DLL.
//
// VSCP (Very Simple Control Protocol)
// http://www.vscp.org
//
// The MIT License (MIT)
//
// Copyright (c) 2000-2017 Ake Hedman, 
// Grodans Paradis AB <info@grodansparadis.com>
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
//  This file is part of VSCP - Very Simple Control Protocol
//  http://www.vscp.org
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
    // The following works on 3.0 but not on 2.8
    wxApp::SetInstance(new wxApp());
    new wxInitializer();
    //wxInitialize();
}

void _fini()
{
    //wxUninitialize();
}



////////////////////////////////////////////////////////////////////////////
// CVSCPLApp construction

CVSCPLApp::CVSCPLApp()
{
    m_instanceCounter = 0;
    pthread_mutex_init( &m_objMutex, NULL );

    // Init. the driver array
    for (int i = 0; i < VSCP_INTERFACE_MAX_OPEN; i++) {
        m_pvscpifArray[ i ] = NULL;
    }

    UNLOCK_MUTEX(m_objMutex);

}

CVSCPLApp::~CVSCPLApp()
{
    LOCK_MUTEX(m_objMutex);

    for (int i = 0; i < VSCP_INTERFACE_MAX_OPEN; i++) {

        if ( NULL != m_pvscpifArray[ i ] ) {

            VscpRemoteTcpIf *pvscpif = getDriverObject(i);
            if (NULL != pvscpif) {
                pvscpif->doCmdClose();
                delete m_pvscpifArray[ i ];
                m_pvscpifArray[ i ] = NULL;
            }
        }
    }

    UNLOCK_MUTEX(m_objMutex);
    pthread_mutex_destroy( &m_objMutex );

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
    m_instanceCounter++;
    wxInitialize();
    return TRUE;
}

