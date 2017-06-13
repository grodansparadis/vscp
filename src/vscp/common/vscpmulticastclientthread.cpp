// vscpmulticastclientthread.cpp
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
#include <sys/types.h>   // for type definitions
#include <winsock2.h>
#include <ws2tcpip.h>    // for win socket structs
#endif

#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"
#include <wx/datetime.h>
#include <wx/listimpl.cpp>
#include <wx/hashmap.h>

#ifdef WIN32
#else

#define _POSIX

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#endif

#ifndef DWORD
#define DWORD unsigned long
#endif

#include <crc.h>
#include "daemonvscp.h"
#include "canal_win32_ipc.h"
#include "canal_macro.h"
#include "vscp.h"
#include "vscphelper.h"
#include "clientlist.h"
#include <dllist.h>
#include <mongoose.h>
//#include <md5.h>
#include <crc8.h>
#include <guid.h>
#include <controlobject.h>
#include "vscpmulticastclientthread.h"

// Prototypes


// The global control object
extern CControlObject *gpobj;


WX_DEFINE_LIST(multicastChannelList);


multicastClientThread::multicastClientThread() : 
wxThread( wxTHREAD_JOINABLE )
{
    m_bQuit = false;
    m_pClientItem = NULL;
}


multicastClientThread::~multicastClientThread()
{

}


///////////////////////////////////////////////////////////////////////////////
// Entry
//

void *multicastClientThread::Entry()
{
    struct mg_mgr mgr;
    //int port = VSCP_MULTICAST_DEFAULT_PORT;
    struct mg_connection *nc;
    struct ip_mreq group;
    char interface[512];
    const char *mcast_group = VSCP_MULTICAST_IPV4_ADDRESS_STR;
    
    mg_mgr_init( &mgr, this );
    
    // Init. CRC data
    crcInit();

    // Bind to interface
    nc = mg_bind( &mgr, 
                    (const char *)m_channel.m_gropupAddress.mbc_str(), 
                    ev_handler );
    if ( NULL == nc ) {
        wxString str = _( "[Multicast channel] Can not bind to interface ");
        str += (const char *)m_channel.m_gropupAddress.mbc_str();
        gpobj->logMsg( str );
        return NULL;
    }
    
    mg_conn_addr_to_str( nc, 
                            interface, 
                            sizeof( interface ),
                            MG_SOCK_STRINGIFY_IP );

    group.imr_multiaddr.s_addr = inet_addr( mcast_group );
    group.imr_interface.s_addr = inet_addr( interface );
    if ( 0 > setsockopt( nc->sock, 
                            IPPROTO_IP, 
                            IP_ADD_MEMBERSHIP, 
                            (char *)&group,
                            sizeof( group ) ) ) {
        gpobj->logMsg( _( "[Multicast channel] Unable to add to muticast group.\n" )  );
        return NULL;
    }

    {
        wxString str = _( "[Multicast channel] Starting multicast channel on group ");
        str += (const char *)m_channel.m_gropupAddress.mbc_str();
        gpobj->logMsg( str );
    }
    
    // We need to create a client object and add this object to the list
    m_pClientItem = new CClientItem;
    if ( NULL == m_pClientItem ) {
        gpobj->logMsg( _( "[Multicast channel] Unable to allocate memory for client, terminating.\n" )  );
        return NULL;
    }
    
    // This is now an active Client
    m_pClientItem->m_bOpen = true;
    m_pClientItem->m_type =  CLIENT_ITEM_INTERFACE_TYPE_CLIENT_MULTICAST_CH;
    m_pClientItem->m_strDeviceName = _("Multicast channel. [");
    m_pClientItem->m_strDeviceName += gpobj->m_udpInfo.m_interface;
    m_pClientItem->m_strDeviceName += _("] Started at ");
    m_pClientItem->m_strDeviceName += wxDateTime::Now().FormatISODate();
    m_pClientItem->m_strDeviceName += _(" ");
    m_pClientItem->m_strDeviceName += wxDateTime::Now().FormatISOTime();
    
    m_pClientItem->m_pUserItem = NULL;  // No user defined
    
    // Add the client to the Client List
    gpobj->m_wxClientMutex.Lock();
    gpobj->addClient( m_pClientItem );
    gpobj->m_wxClientMutex.Unlock();

    // Set receive filter
    //vscp_clearVSCPFilter( &m_pClientItem->m_filterVSCP );
    memcpy( &m_pClientItem->m_filterVSCP, &gpobj->m_udpInfo.m_filter, sizeof(vscpEventFilter) );
  
    while ( !TestDestroy() && !m_bQuit ) {
        mg_mgr_poll( &mgr, 50 );
        sendFrame( &mgr, m_pClientItem );
    }
    
    mg_mgr_free(&mgr);

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// ev_handler
//

void multicastClientThread::ev_handler( struct mg_connection *nc, int ev, void *p ) 
{
    struct mbuf *io = &nc->recv_mbuf;
    multicastClientThread *pMulticastClientThread =
                            (multicastClientThread *)nc->mgr->user_data;
  
    switch (ev) {
        
        case MG_EV_POLL: {
            const char *peer;
            peer = inet_ntoa(nc->sa.sin.sin_addr);
            printf("nc->sa: %s %d\n", peer, ntohs(nc->sa.sin.sin_port));
            break;
        }
    
        case MG_EV_RECV:
            {
                gpobj->logMsg( wxString::Format( _( "Received UDP event\n" ) ),
                                        DAEMON_LOGMSG_DEBUG,
                                        DAEMON_LOGTYPE_GENERAL );
                
                // If a user is specified check that this user is allowed to connect
                // from this location
                if ( NULL != pMulticastClientThread->m_pClientItem->m_pUserItem ) {
                    
                    // Get remote address
                    wxString remoteaddr = wxString::FromAscii( inet_ntoa( nc->sa.sin.sin_addr )  );

                    // Check if this user is allowed to connect from this location
                    gpobj->m_mutexUserList.Lock();
                    bool bValidHost =
                            pMulticastClientThread->m_pClientItem->m_pUserItem->isAllowedToConnect( remoteaddr );
                    gpobj->m_mutexUserList.Unlock();

                    if ( !bValidHost ) {
                        wxString strErr = wxString::Format(_("[UDP Client] Host [%s] not allowed to send UDP datagrams.\n"),
                                                                (const char *)remoteaddr.mbc_str() );
                        gpobj->logMsg( strErr );
                        return;
                    }
                }
                
                // Must be at least a packet-type + header and a crc
                if ( nc->recv_mbuf.len < ( 1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 ) ) {
                    
                    // Packet to short
                    gpobj->logMsg( wxString::Format(_("UDP frame have invalid length = %d\n"),
                                        (int)nc->recv_mbuf.len ) );
                    return;
                    
                }
                
                // If un-secure frames are not supported frames must be encrypted                
                if ( !gpobj->m_udpInfo.m_bAllowUnsecure && 
                        !GET_VSCP_MULTICAST_PACKET_ENCRYPTION( nc->recv_mbuf.buf[ VSCP_MULTICAST_PACKET0_POS_PKTTYPE ] ) ) {
                    gpobj->logMsg( wxString::Format(_("UDP frame must be encrypted (or m_bAllowUnsecure set to true) to be accepted.\n" ) ) );
                    return;
                }
                
                // OK receive the frame
                if ( receiveFrame( nc, 
                                    pMulticastClientThread->m_pClientItem,
                                    &gpobj->m_udpInfo.m_filter )  ) {
                    
                    if ( gpobj->m_udpInfo.m_bAck ) {
                        replyAckFrame( nc, nc->recv_mbuf.buf[0] );
                    }
                    
                }
                else {
                    
                    if ( gpobj->m_udpInfo.m_bAck ) {
                        replyNackFrame( nc, nc->recv_mbuf.buf[0] );
                    }
                    
                }

            }
            //printf("Received (%zu bytes): '%.*s'\n", io->len, (int) io->len, io->buf);
            //mbuf_remove(io, io->len);
            nc->flags |= MG_F_SEND_AND_CLOSE;
            break;
        
        default:
            break;      
    }
}

///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void multicastClientThread::OnExit()
{
    
}

///////////////////////////////////////////////////////////////////////////////
// sendFrame
//

bool multicastClientThread::sendFrame( struct mg_mgr *pmgr, 
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
    
    // Send the event on the group
    mg_send( pmgr->active_connections, 
                sendbuf, 1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 + pEvent->sizeData );
     
    // Remove the event data node
    pClientItem->m_mutexClientInputQueue.Lock();
    pClientItem->m_clientInputQueue.DeleteNode( nodeClient );
    pClientItem->m_mutexClientInputQueue.Unlock();   
}

////////////////////////////////////////////////////////////////////////////////
// receiveFrame
//

bool 
multicastClientThread::receiveFrame( struct mg_connection *nc, 
                                                        CClientItem *pClientItem,
                                                        vscpEventFilter *pRxFilter )
{
    uint8_t buf[ 1024 ];
    vscpEvent *pEvent;
    uint8_t key[] = {
                    1,2,3,4,5,6,7,8,
                    9,10,11,12,13,14,15,16,
                    17,18,19,20,21,22,23,24,
                    25,26,27,28,29,30,31,32
    };
    
    // Check pointers
    if ( NULL == nc ) return false;
    if ( NULL == pClientItem ) return false;
    if ( NULL == pRxFilter ) return false;    
    
    memset( buf, 0, sizeof( buf ) );
    if ( !vscp_decryptVscpUdpFrame( buf, 
                (uint8_t *)nc->recv_mbuf.buf, 
                nc->recv_mbuf.len,  // actually len-16 but encrypt routine handle this
                (const uint8_t *)key,
                NULL,   // Will be copied from the last 16-bytes
                GET_VSCP_MULTICAST_PACKET_ENCRYPTION( nc->recv_mbuf.buf[0] ) ) ) {
        return false;
    };
    
    // Allocate a new event
    if ( NULL == ( pEvent = new vscpEvent ) ) return false;
    pEvent->pdata = NULL;
    
    if ( !vscp_getEventFromUdpFrame( pEvent, buf, nc->recv_mbuf.len ) ) {
        vscp_deleteVSCPevent_v2( &pEvent );
        return false;
    }
    
    /*if ( !vscp_getEventFromUdpFrame( pEvent, (const uint8_t *)nc->recv_mbuf.buf, nc->recv_mbuf.len ) ) {
        vscp_deleteVSCPevent_v2( &pEvent );
        return false;
    }*/
    
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
// replyAckFrame
//

bool 
multicastClientThread::replyAckFrame( struct mg_connection *nc, 
                                        uint8_t pkttype )
{
    unsigned char sendbuf[1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 + 512 + 16];    // Send buffer
    vscpEventEx ex;
    
    ex.head = 0;
    ex.obid = 0;
    ex.timestamp = vscp_makeTimeStamp();
    ex.year = wxDateTime::UNow().GetYear();
    ex.month = wxDateTime::UNow().GetMonth();
    ex.day = wxDateTime::UNow().GetDay();
    ex.hour = wxDateTime::UNow().GetHour();
    ex.minute = wxDateTime::UNow().GetMinute();
    ex.second = wxDateTime::UNow().GetSecond();
    memcpy( ex.GUID, gpobj->m_udpInfo.m_guid.getGUID(), 16 );
    ex.vscp_class = VSCP_CLASS1_ERROR;
    ex.vscp_type = VSCP_TYPE_ERROR_SUCCESS;        
    ex.sizeData = 3;
    memset( ex.data, 0, 3 );    // index/zone/subzone = 0
        
    if ( !vscp_writeEventExToUdpFrame( sendbuf,  
                                        sizeof( sendbuf ), 
                                        pkttype,
                                        &ex ) ) {
        return false;
    }
    
    // Send to remote node
    mg_send( nc, sendbuf, 1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 + ex.sizeData );
    
    return true;
}


////////////////////////////////////////////////////////////////////////////////
// replyNackFrame
//

bool 
multicastClientThread::replyNackFrame( struct mg_connection *nc, 
                                            uint8_t pkttype )
{
    unsigned char sendbuf[1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 + 512 + 16];    // Send buffer
    vscpEventEx ex;
    
    ex.head = 0;
    ex.obid = 0;
    ex.timestamp = vscp_makeTimeStamp();
    ex.year = wxDateTime::UNow().GetYear();
    ex.month = wxDateTime::UNow().GetMonth();
    ex.day = wxDateTime::UNow().GetDay();
    ex.hour = wxDateTime::UNow().GetHour();
    ex.minute = wxDateTime::UNow().GetMinute();
    ex.second = wxDateTime::UNow().GetSecond();
    memcpy( ex.GUID, gpobj->m_udpInfo.m_guid.getGUID(), 16 );
    ex.vscp_class = VSCP_CLASS1_ERROR;
    ex.vscp_type = VSCP_TYPE_ERROR_ERROR;        
    ex.sizeData = 3;
    memset( ex.data, 0, 3 );    // index/zone/subzone = 0
    
    if ( !vscp_writeEventExToUdpFrame( sendbuf,  
                                        sizeof( sendbuf ), 
                                        pkttype,
                                        &ex ) ) {
        return false;
    }
    
    // Send to remote node
    mg_send( nc, sendbuf, 1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 + ex.sizeData );
    
    return true;
}
