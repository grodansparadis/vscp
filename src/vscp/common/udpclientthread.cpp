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

#include <vscp.h>
#include <crc.h>
#include <tcpipclientthread.h>
#include <canal_win32_ipc.h>
#include <canal_macro.h>
#include <canal.h>
#include <vscphelper.h>
#include <dllist.h>
#include <mongoose.h>
#include <version.h>
#include <controlobject.h>
#include <udpclientthread.h>

// Prototypes


// The global control object
extern CControlObject *gpobj; 


WX_DEFINE_LIST(udpRemoteClientList);


///////////////////////////////////////////////////////////////////////////////
// VSCPUDPClientThread
//
// This thread listens for connection on a TCP socket and starts a new thread
// to handle client requests
//

VSCPUDPClientThread::VSCPUDPClientThread()
: wxThread(wxTHREAD_JOINABLE)
{
    m_bQuit = false;
    m_pClientItem = NULL;
}


VSCPUDPClientThread::~VSCPUDPClientThread()
{
    ;
}


///////////////////////////////////////////////////////////////////////////////
// Entry
//

void *VSCPUDPClientThread::Entry()
{
    struct mg_mgr mgr;
    struct mg_connection *nc;
    
    // Init. CRC data
    crcInit();

    // We need to create a client object and add this object to the list
    m_pClientItem = new CClientItem;
    if ( NULL == m_pClientItem ) {
        gpobj->logMsg( _( "[UDP Client] Unable to allocate memory for client, terminating.\n" )  );
        return NULL;
    }
    
    // This is now an active Client
    m_pClientItem->m_bOpen = true;
    m_pClientItem->m_type =  CLIENT_ITEM_INTERFACE_TYPE_CLIENT_UDP;
    m_pClientItem->m_strDeviceName = _("Remote UDP Client Listner. [");
    m_pClientItem->m_strDeviceName += gpobj->m_udpInfo.m_interface;
    m_pClientItem->m_strDeviceName += _("] Started at ");
    m_pClientItem->m_strDeviceName += wxDateTime::Now().FormatISODate();
    m_pClientItem->m_strDeviceName += _(" ");
    m_pClientItem->m_strDeviceName += wxDateTime::Now().FormatISOTime();

    
    // If a user is defined get user item
    if ( gpobj->m_udpInfo.m_user.Trim().Length() ) {
        gpobj->m_wxClientMutex.Lock();
        m_pClientItem->m_pUserItem =
            gpobj->m_userList.validateUser( gpobj->m_udpInfo.m_user, 
                                            gpobj->m_udpInfo.m_password );
        gpobj->m_mutexUserList.Unlock();

        if ( NULL == m_pClientItem->m_pUserItem ) {
            
            delete m_pClientItem;
            
            wxString strErr =
                wxString::Format( _("[UDP Client] User [%s] NOT allowed to connect.\n"),
                                    (const char *)gpobj->m_udpInfo.m_user.mbc_str() );
            gpobj->logMsg ( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_SECURITY );
#if wxMAJOR_VERSION >= 3
            wxLogDebug( _("Password/Username failure.") );
#else
            ::wxLogDebug( _("Password/Username failure.") );
#endif                    
            return NULL;
        }
    }
    else {
        m_pClientItem->m_pUserItem = NULL;  // No user defined
    }
    
    // Add the client to the Client List
    gpobj->m_wxClientMutex.Lock();
    gpobj->addClient( m_pClientItem );
    gpobj->m_wxClientMutex.Unlock();

    // Set receive filter
    //vscp_clearVSCPFilter( &m_pClientItem->m_filterVSCP );
    memcpy( &m_pClientItem->m_filterVSCP, &gpobj->m_udpInfo.m_filter, sizeof(vscpEventFilter) );

    mg_mgr_init( &mgr, this );

    if ( wxNOT_FOUND == gpobj->m_udpInfo.m_interface.Find(_("udp://") ) ) {
        gpobj->m_udpInfo.m_interface = _("udp://") + gpobj->m_udpInfo.m_interface;
    }

    // Bind to this interface
    if ( NULL == ( nc = mg_bind( &mgr, gpobj->m_udpInfo.m_interface.mbc_str(), VSCPUDPClientThread::ev_handler ) ) ) {
        wxString str = _("UDP Client: Failed to bind to interface. [");
        str += gpobj->m_udpInfo.m_interface;
        str += _("]\n");
        gpobj->logMsg( str );
        return NULL;
    }
    
    gpobj->logMsg( _("UDP Client: Bind to interface. [") + 
                        gpobj->m_udpInfo.m_interface +
                        _("]\n"),
                        DAEMON_LOGMSG_DEBUG,
                        DAEMON_LOGTYPE_GENERAL );

    gpobj->logMsg( _("UDP Client: Thread started.\n") );

    while ( !TestDestroy() && !m_bQuit ) {
        mg_mgr_poll( &mgr, 50 );
        sendUDPFrame( &mgr, m_pClientItem );
    }

    // release the server
    mg_mgr_free( &mgr );

    gpobj->logMsg( _( "UDP ClientThread: Quit.\n" )  );

    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void VSCPUDPClientThread::OnExit()
{
    if ( NULL != m_pClientItem ) {
        // Add the client to the Client List
        gpobj->m_wxClientMutex.Lock();
        gpobj->removeClient( m_pClientItem );
        gpobj->m_wxClientMutex.Unlock();
    }
}

///////////////////////////////////////////////////////////////////////////////
// ev_handler
//

void
VSCPUDPClientThread::ev_handler(struct mg_connection *nc, int ev, void *p)
{
    struct mbuf *io = &nc->recv_mbuf;
    VSCPUDPClientThread *pUDPClientThread =
                            (VSCPUDPClientThread *)nc->mgr->user_data;

    switch (ev) {

        case MG_EV_CLOSE:
            break;

        case MG_EV_RECV:
            {
                gpobj->logMsg( wxString::Format( _( "Received UDP event\n" ) ),
                                        DAEMON_LOGMSG_DEBUG,
                                        DAEMON_LOGTYPE_GENERAL );
                
                // If a user is specified check that this user is allowed to connect
                // from this location
                if ( NULL != pUDPClientThread->m_pClientItem->m_pUserItem ) {
                    
                    // Get remote address
                    wxString remoteaddr = wxString::FromAscii( inet_ntoa( nc->sa.sin.sin_addr )  );

                    // Check if this user is allowed to connect from this location
                    gpobj->m_mutexUserList.Lock();
                    bool bValidHost =
                            pUDPClientThread->m_pClientItem->m_pUserItem->isAllowedToConnect( remoteaddr );
                    gpobj->m_mutexUserList.Unlock();

                    if ( !bValidHost ) {
                        wxString strErr = wxString::Format(_("[UDP Client] Host [%s] not allowed to send UDP datagrams.\n"),
                                                                (const char *)remoteaddr.mbc_str() );
                        gpobj->logMsg( strErr );
                        return;
                    }
                }
                
                // Must be at least a packet-type + header and a crc
                if ( nc->recv_mbuf.len >= ( 1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 ) ) {
                    
                    if ( VSCP_ENCRYPTION_NONE == 
                            GET_VSCP_MULTICAST_PACKET_ENCRYPTION( nc->recv_mbuf.buf[ VSCP_MULTICAST_PACKET0_POS_PKTTYPE ] ) ) {
                        receiveUnEncryptedFrame( nc, 
                                                    pUDPClientThread->m_pClientItem,
                                                    &gpobj->m_udpInfo.m_filter );
                    }
                    else if ( VSCP_ENCRYPTION_AES128 == 
                            GET_VSCP_MULTICAST_PACKET_ENCRYPTION( nc->recv_mbuf.buf[ VSCP_MULTICAST_PACKET0_POS_PKTTYPE ] ) ) {
                    
                    }
                    else if ( VSCP_ENCRYPTION_AES192 == 
                            GET_VSCP_MULTICAST_PACKET_ENCRYPTION( nc->recv_mbuf.buf[ VSCP_MULTICAST_PACKET0_POS_PKTTYPE ] ) ) {
                    
                    }
                    else if ( VSCP_ENCRYPTION_AES256 == 
                            GET_VSCP_MULTICAST_PACKET_ENCRYPTION( nc->recv_mbuf.buf[ VSCP_MULTICAST_PACKET0_POS_PKTTYPE ] ) ) {
                    
                    }
                    else { 
                        gpobj->logMsg( wxString::Format(_("UDP packet specify invalid encryption code %d\n"),
                                        (int)nc->recv_mbuf.buf[ VSCP_MULTICAST_PACKET0_POS_PKTTYPE ] ) );
                        return;
                    }
                    
                }
                else {
                    // Packet to short
                    gpobj->logMsg( wxString::Format(_("UDP packet have invalid length = %d\n"),
                                        (int)nc->recv_mbuf.len ) );
                    return;
                }    

            }
            break;

        case MG_EV_POLL:
            break;
            
        case MG_EV_CONNECT:
            {            
            int bConnect = * (int *)p;
            if (bConnect == 0) {
                // Success
                //mg_send(nc, "test", 4);
                //nc->flags |= MG_F_SEND_AND_CLOSE;
            } 
            else  {
                // Error
                nc->flags |= MG_F_SEND_AND_CLOSE;
                //printf("connect() error: %s\n", strerror( bConnect ) );
            }
            if ( nc->send_mbuf.len ) {
                //mg_send(nc, nc->send_mbuf.buf, nc->send_mbuf.len);
            }
            }
            break;
            
        case MG_EV_SEND:
            //nc->flags |= MG_F_SEND_AND_CLOSE;
            break;

        default:
            break;
    }
}


////////////////////////////////////////////////////////////////////////////////
// receiveUnEncryptedFrame
//

bool 
VSCPUDPClientThread::receiveUnEncryptedFrame( struct mg_connection *nc, 
                                                CClientItem *pClientItem,
                                                vscpEventFilter *pRxFilter )
{
    // Check pointers
    if ( NULL == nc ) return false;
    if ( NULL == pClientItem ) return false;
    if ( NULL == pRxFilter ) return false;
    
    vscpEvent *pEvent;
    
    // Allocate a new event
    if ( NULL == ( pEvent = new vscpEvent ) ) return false;
    pEvent->pdata = NULL;
    
    if ( !vscp_getEventFromUdpFrame( pEvent, (const uint8_t *)nc->recv_mbuf.buf, nc->recv_mbuf.len ) ) {
        vscp_deleteVSCPevent_v2( &pEvent );
        return false;
    }
    
    if ( vscp_doLevel2Filter( pEvent, pRxFilter ) ) {
        
        // Set obid to ourself so we don't get events we
        // receive
        pEvent->obid = pClientItem->m_clientID;
    
        // There must be room in the receive queue (even if room (or whisky) has been better)
        if ( gpobj->m_maxItemsInClientReceiveQueue >
                    gpobj->m_clientOutputQueue.GetCount() ) {

            gpobj->m_mutexClientOutputQueue.Lock();
            gpobj->m_clientOutputQueue.Append(pEvent);
            gpobj->m_semClientOutputQueue.Post();
            gpobj->m_mutexClientOutputQueue.Unlock();

        }
        else {
            vscp_deleteVSCPevent_v2( &pEvent );
        }
    }
    else {
        vscp_deleteVSCPevent_v2( &pEvent );
    }
    
    return true;
}



////////////////////////////////////////////////////////////////////////////////
// sendUDPFrame
//

bool 
VSCPUDPClientThread::sendUDPFrame( struct mg_mgr *pmgr, 
                                      CClientItem *pClientItem )
{
    CLIENTEVENTLIST::compatibility_iterator nodeClient;
    unsigned char sendbuf[1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 + 512 + 16];    // Send buffer
    size_t sizeSend = 0;
    
    // Check if there is an event to send
    if ( !pClientItem->m_clientInputQueue.GetCount() ) {
        return false;
    }
    
    pClientItem->m_mutexClientInputQueue.Lock();
    nodeClient = pClientItem->m_clientInputQueue.GetFirst();
    pClientItem->m_mutexClientInputQueue.Unlock();
    
    // Get the event
    vscpEvent *pEvent = nodeClient->GetData();
    if ( NULL == pEvent ) return false;

    // Check that size is valid
    if ( pEvent->sizeData > VSCP_LEVEL2_MAXDATA ) {
        vscp_deleteVSCPevent_v2( &pEvent );
        return false; 
    }
    
    // Packet type
    sendbuf[ VSCP_MULTICAST_PACKET0_POS_PKTTYPE ] = 0;
    
    if ( !vscp_writeEventToUdpFrame( sendbuf + 1, 
                                        sizeof( sendbuf)-1, 
                                        0,  // Packet type
                                        pEvent ) ) {
        vscp_deleteVSCPevent_v2( &pEvent );
        return false;
    }
    
    // Send the event to all clients
    gpobj->m_mutexUDPInfo.Lock();
    udpRemoteClientList::iterator iter;
    for (iter = gpobj->m_udpInfo.m_remotes.begin(); 
            iter != gpobj->m_udpInfo.m_remotes.end(); ++iter) {
        
        udpRemoteClientInfo *pRemoteUDPNode = *iter;
        mg_connect( pmgr, pRemoteUDPNode->m_remoteAddress, ev_handler );
        mg_send( pmgr->active_connections, sendbuf, 1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 + pEvent->sizeData);
        
    }
    gpobj->m_mutexUDPInfo.Unlock();
    

    //mg_connect( pmgr, "udp://127.0.0.1:9999", ev_handler );
    //mg_send( pmgr->active_connections, sendbuf, 1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 + pEvent->sizeData);
     
    // Remove the event data node
    pClientItem->m_mutexClientInputQueue.Lock();
    pClientItem->m_clientInputQueue.DeleteNode( nodeClient );
    pClientItem->m_mutexClientInputQueue.Unlock();   
            
    return true;
}