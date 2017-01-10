// udpclientthread.cpp
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

#ifdef WIN32
#include <winsock2.h>
#endif

//#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"
#include <wx/listimpl.cpp>
#include <wx/tokenzr.h>
#include <wx/stdpaths.h>

#ifndef DWORD
#define DWORD unsigned long
#endif

#include <mongoose.h>

#include <vscp.h>
#include <canal_win32_ipc.h>
#include <canal_macro.h>
#include <canal.h>
#include <vscphelper.h>
#include <dllist.h>
#include <version.h>
#include <controlobject.h>
#include <vscpmqttbroker.h>

///////////////////////////////////////////////////
//                 GLOBALS
///////////////////////////////////////////////////

extern CControlObject *gpobj;

///////////////////////////////////////////////////////////////////////////////
// VSCPMQTTBrokerThread
//
// This thread listens for connection on a TCP socket and starts a new thread
// to handle client requests
//

VSCPMQTTBrokerThread::VSCPMQTTBrokerThread()
: wxThread(wxTHREAD_JOINABLE)
{
    m_bQuit = false;
    m_pCtrlObject = NULL;
    m_pClientItem = NULL;
}


VSCPMQTTBrokerThread::~VSCPMQTTBrokerThread()
{
    ;
}


///////////////////////////////////////////////////////////////////////////////
// Entry
//

void *VSCPMQTTBrokerThread::Entry()
{
    struct mg_mgr mgr;
    struct mg_connection *nc;
    struct mg_mqtt_broker brk;
    const char *address = "0.0.0.0:1883";

    // Check pointers
    if ( NULL == m_pCtrlObject ) return NULL;

    // We need to create a clientobject and add this object to the list
    m_pClientItem = new CClientItem;
    if ( NULL == m_pClientItem ) {
        m_pCtrlObject->logMsg( _( "[VSCP MQTT Broker] Unable to allocate memory for client.\n" )  );
        return NULL;
    }

    // This is now an active Client
    m_pClientItem->m_bOpen = true;
    m_pClientItem->m_type =  CLIENT_ITEM_INTERFACE_TYPE_CLIENT_UDP;
    m_pClientItem->m_strDeviceName = _("VSCP MQTT Broker: Started at ");
    wxDateTime now = wxDateTime::Now();
    m_pClientItem->m_strDeviceName += now.FormatISODate();
    m_pClientItem->m_strDeviceName += _(" ");
    m_pClientItem->m_strDeviceName += now.FormatISOTime();

    // Add the client to the Client List
    m_pCtrlObject->m_wxClientMutex.Lock();
    m_pCtrlObject->addClient( m_pClientItem );
    m_pCtrlObject->m_wxClientMutex.Unlock();

    // Clear the filter (Allow everything )
    vscp_clearVSCPFilter( &m_pClientItem->m_filterVSCP );
    mg_mgr_init( &mgr, this );
    mg_mqtt_broker_init( &brk, NULL );

    if ( ( nc = mg_bind( &mgr,
                            m_pCtrlObject->m_strMQTTBrokerInterfaceAddress.mbc_str(),
                            mg_mqtt_broker ) ) == NULL) {
        m_pCtrlObject->logMsg( _("VSCP MQTT Broker: Faild to bind to requested address.\n")  );
        return NULL;
    }

    nc->user_data = &brk;

    m_pCtrlObject->logMsg( _("VSCP MQTT Broker: Thread started.\n")  );

    while ( !TestDestroy() && !m_bQuit ) {
        mg_mgr_poll( &mgr, 1000 );
    }

    // release the server
    //ns_mgr_free( &m_pCtrlObject->m_mgrTcpIpServer );
    mg_mgr_free( &mgr );

    m_pCtrlObject->logMsg( _( "VSCP MQTT Broker: Quit.\n" )  );

    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void VSCPMQTTBrokerThread::OnExit()
{
    if ( NULL != m_pClientItem ) {
        // Add the client to the Client List
        m_pCtrlObject->m_wxClientMutex.Lock();
        m_pCtrlObject->removeClient( m_pClientItem );
        m_pCtrlObject->m_wxClientMutex.Unlock();
    }
}

///////////////////////////////////////////////////////////////////////////////
// ev_handler
//

void
VSCPMQTTBrokerThread::ev_handler(struct mg_connection *nc, int ev, void *p)
{
    struct mbuf *io = &nc->recv_mbuf;
    struct mg_mqtt_broker *pbrk =
                            (struct mg_mqtt_broker *)nc->user_data;

    switch (ev) {

        case MG_EV_CLOSE:
            break;

        case MG_EV_RECV:
            break;

        case MG_EV_POLL:
            break;

        default:
            break;
    }
}




