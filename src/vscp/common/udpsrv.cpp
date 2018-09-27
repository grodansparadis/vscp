// udpclientthread.cpp
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (c) 2000-2018 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
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

#ifdef WIN32
#include <winsock2.h>
#endif

#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"
#include <wx/listimpl.cpp>
#include <wx/tokenzr.h>
#include <wx/stdpaths.h>

#ifndef DWORD
#define DWORD unsigned long
#endif

#include <crc.h>
#include <vscp_aes.h>
#include <vscp.h>
#include <tcpipsrv.h>
#include <canal_win32_ipc.h>
#include <canal_macro.h>
#include <canal.h>
#include <vscphelper.h>
#include <dllist.h>
#include <mongoose.h>
#include <version.h>
#include <controlobject.h>
#include <udpsrv.h>

// Prototypes

// The global control object
extern CControlObject *gpobj; 


WX_DEFINE_LIST(udpRemoteClientList);
WX_DEFINE_LIST(udpResendItemList);

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

    mg_mgr_init( &mgr, this );

    if ( wxNOT_FOUND == gpobj->m_udpInfo.m_interface.Find(_("udp://") ) ) {
        gpobj->m_udpInfo.m_interface = _("udp://") + gpobj->m_udpInfo.m_interface;
    }

    // Bind to this interface
    if ( NULL == ( nc = mg_bind( &mgr, 
                                    gpobj->m_udpInfo.m_interface.mbc_str(), 
                                    VSCPUDPClientThread::ev_handler ) ) ) {
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
    
    // We need to create a client object and add this object to the list
    m_pClientItem = new CClientItem;
    if ( NULL == m_pClientItem ) {
        gpobj->logMsg( _( "[UDP Client] Unable to allocate memory for client, terminating.\n" )  );
        return NULL;
    }
    
    // This is now an active Client
    m_pClientItem->m_bOpen = true;
    m_pClientItem->m_type =  CLIENT_ITEM_INTERFACE_TYPE_CLIENT_UDP;
    m_pClientItem->m_strDeviceName = _("Internal UDP client listner. [");
    m_pClientItem->m_strDeviceName += gpobj->m_udpInfo.m_interface;
    m_pClientItem->m_strDeviceName += _("]|Started at ");
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
            wxLogDebug( _("Password/Username failure.") );
                 
            return NULL;
        }
    }
    else {
        m_pClientItem->m_pUserItem = NULL;  // No user defined
    }
    
    // Add the client to the Client List
    gpobj->m_wxClientMutex.Lock();
    if ( !gpobj->addClient( m_pClientItem, CLIENT_ID_UDP_LISTNER ) ) {
        // Failed to add client
        delete m_pClientItem;
        m_pClientItem = NULL;
        gpobj->m_wxClientMutex.Unlock();
        gpobj->logMsg( _("UDP server: Failed to add client. Terminating thread.") );
        return NULL;
    }
    gpobj->m_wxClientMutex.Unlock();

    // Set receive filter
    memcpy( &m_pClientItem->m_filterVSCP, &gpobj->m_udpInfo.m_filter, sizeof(vscpEventFilter) );
    
    // Set GUID for channel
    if ( !gpobj->m_udpInfo.m_guid.isNULL() ) {
        m_pClientItem->m_guid = gpobj->m_udpInfo.m_guid;
    }

    gpobj->logMsg( _("UDP Client: Thread started.\n") );

    while ( !TestDestroy() && !m_bQuit ) {
        mg_mgr_poll( &mgr, 50 );
        sendFrame( &mgr, m_pClientItem );
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
                    //        pUDPClientThread->m_pClientItem->m_pUserItem->isAllowedToConnect( remoteaddr );
                    ( 1 == pUDPClientThread->m_pClientItem->m_pUserItem->isAllowedToConnect( nc->sa.sin.sin_addr.s_addr ) );
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
                                    pUDPClientThread->m_pClientItem,
                                    &gpobj->m_udpInfo.m_filter )  ) {
                    
                    if ( gpobj->m_udpInfo.m_bAck ) {
                        replyAckFrame( nc, 
                                        nc->recv_mbuf.buf[ VSCP_MULTICAST_PACKET0_POS_PKTTYPE ], 
                                        ( nc->recv_mbuf.buf[ VSCP_MULTICAST_PACKET0_POS_HEAD_LSB ] & 0xf8 ) );
                    }
                    
                }
                else {
                    
                    if ( gpobj->m_udpInfo.m_bAck ) {
                        replyNackFrame( nc, 
                                        nc->recv_mbuf.buf[ VSCP_MULTICAST_PACKET0_POS_PKTTYPE ],
                                        ( nc->recv_mbuf.buf[ VSCP_MULTICAST_PACKET0_POS_HEAD_LSB ] & 0xf8 ) );
                    }
                    
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
// receiveFrame
//

bool 
VSCPUDPClientThread::receiveFrame( struct mg_connection *nc, 
                                        CClientItem *pClientItem,
                                        vscpEventFilter *pRxFilter )
{
    uint8_t buf[ 1024 ];
    vscpEvent *pEvent;
    
    // Check pointers
    if ( NULL == nc ) return false;
    if ( NULL == pClientItem ) return false;
    if ( NULL == pRxFilter ) return false;
    
    memset( buf, 0, sizeof( buf ) );
    if ( !vscp_decryptVscpUdpFrame( buf, 
                (uint8_t *)nc->recv_mbuf.buf, 
                nc->recv_mbuf.len,  // actually len-16 but encrypt routine handle this
                gpobj->getSystemKey( NULL ),
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
// sendFrame
//

bool 
VSCPUDPClientThread::sendFrame( struct mg_mgr *pmgr, 
                                      CClientItem *pClientItem )
{
    CLIENTEVENTLIST::compatibility_iterator nodeClient;
    unsigned char sendbuf[1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 + 
                            VSCP_LEVEL2_MAXDATA + 16];    // Send buffer
    uint8_t iv[16];
    
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
        
    // Send the event to all clients
    gpobj->m_mutexUDPInfo.Lock();
    udpRemoteClientList::iterator iter;
    
    for (iter = gpobj->m_udpInfo.m_remotes.begin(); 
            iter != gpobj->m_udpInfo.m_remotes.end(); ++iter) {
        
        udpRemoteClientInfo *pRemoteUDPNode = *iter;
        
        // Check if filtered out
        if ( !vscp_doLevel2Filter( pEvent, &pRemoteUDPNode->m_filter ) ) {
            continue;
        }
               
        // Packet type
        sendbuf[ VSCP_MULTICAST_PACKET0_POS_PKTTYPE ] = 
                            SET_VSCP_MULTICAST_TYPE( 0, pRemoteUDPNode->m_nEncryption );
    
        // Get initialization vector                    
        getRandomIV( iv, 16 );
    
        uint8_t wrkbuf[1024];
        memset( wrkbuf, 0, sizeof(wrkbuf) );
        
        if ( !vscp_writeEventToUdpFrame( wrkbuf, 
                                            sizeof( wrkbuf), 
                                            SET_VSCP_MULTICAST_TYPE( 0, pRemoteUDPNode->m_nEncryption ),
                                            pEvent ) ) {
            vscp_deleteVSCPevent_v2( &pEvent );
            continue;
        }
        
        // Write rolling index
        wrkbuf[ VSCP_MULTICAST_PACKET0_POS_HEAD_LSB ] &= 0xf8;
        wrkbuf[ VSCP_MULTICAST_PACKET0_POS_HEAD_LSB ] |= (0x07 & pRemoteUDPNode->m_index );
        pRemoteUDPNode->m_index++;
        
        size_t len = 1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + pEvent->sizeData + 2;
        if ( 0 == ( len = vscp_encryptVscpUdpFrame( sendbuf, 
                                            wrkbuf, 
                                            len,
                                            gpobj->getSystemKey( NULL ),
                                            iv,
                                            pRemoteUDPNode->m_nEncryption ) ) ) {
            vscp_deleteVSCPevent_v2( &pEvent );
            continue;
        }
        
if ( 0 ) {    
        int i;
        wxPrintf("IV = ");
        for ( i=0; i<16; i++ ) {
            wxPrintf("%02X ", iv[i] );
        }
        wxPrintf("\n");
        
        wxPrintf("key = ");
        uint8_t *pkey = gpobj->getSystemKey( NULL );
        for ( i=0; i<32; i++ ) {
            wxPrintf("%02X ", pkey[i] );
        }
        wxPrintf("\n");
}
        
        mg_connect( pmgr, pRemoteUDPNode->m_remoteAddress, ev_handler );
        mg_send( pmgr->active_connections, sendbuf, len );
        
    }
    gpobj->m_mutexUDPInfo.Unlock();
    
    // Remove the event data node
    pClientItem->m_mutexClientInputQueue.Lock();
    pClientItem->m_clientInputQueue.DeleteNode( nodeClient );
    pClientItem->m_mutexClientInputQueue.Unlock();   
            
    return true;
}


////////////////////////////////////////////////////////////////////////////////
// replyAckFrame
//

bool 
VSCPUDPClientThread::replyAckFrame( struct mg_connection *nc, 
                                        uint8_t pkttype,
                                        uint8_t index )
{
    unsigned char sendbuf[1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 + 
                            VSCP_LEVEL2_MAXDATA + 16];    // Send buffer
    vscpEventEx ex;
    
    ex.head = ( index & 0xf8 );
    ex.crc = 0;
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
    ex.data[0] = index;
        
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
VSCPUDPClientThread::replyNackFrame( struct mg_connection *nc, 
                                            uint8_t pkttype,
                                            uint8_t index )
{
    unsigned char sendbuf[1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 + 
                                VSCP_LEVEL2_MAXDATA + 16];    // Send buffer
    vscpEventEx ex;
    
    ex.head = ( index & 0xf8 );
    ex.crc = 0;
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
    ex.data[0] = index;
    
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
