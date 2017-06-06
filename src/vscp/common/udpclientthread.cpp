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

    gpobj->logMsg( _("UDP Client: Thread started.\n") );

    while ( !TestDestroy() && !m_bQuit ) {
        mg_mgr_poll( &mgr, 50 );
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
                        gpobj->logMsg ( strErr );
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
    
    if ( !vscp_getVscpEventFromUdpFrame( pEvent, (const uint8_t *)nc->recv_mbuf.buf, nc->recv_mbuf.len ) ) {
        vscp_deleteVSCPevent_v2( &pEvent );
        return false;
    }
    
    /*
    //  0           Packet type & encryption settings
    //  1           HEAD MSB
    //  2           HEAD LSB
    //  3           Timestamp microseconds MSB
    //  4           Timestamp microseconds
    //  5           Timestamp microseconds
    //  6           Timestamp microseconds LSB
    //  7           Year
    //  8           Month
    //  9           Day
    //  10          Hour
    //  11          Minute
    //  12          Second
    //  13          CLASS MSB
    //  14          CLASS LSB
    //  15          TYPE MSB
    //  16          TYPE LSB
    //  17 - 32     ORIGINATING GUID
    //  33          DATA SIZE MSB
    //  34          DATA SIZE LSB
    //  35 - n 	    data limited to max 512 - 25 = 487 bytes
    //  len - 2     CRC MSB( Calculated on HEAD + CLASS + TYPE + ADDRESS + SIZE + DATA )
    //  len - 1     CRC LSB
    
    size_t calcFrameSize = 1 +                                          // packet type & encryption                        
                        VSCP_MULTICAST_PACKET0_HEADER_LENGTH +          // header
                        2 +                                             // CRC
                        ((uint16_t)nc->recv_mbuf.buf[ VSCP_MULTICAST_PACKET0_POS_VSCP_SIZE_MSB ] << 8 ) +
                         (uint8_t)nc->recv_mbuf.buf[ VSCP_MULTICAST_PACKET0_POS_VSCP_SIZE_LSB ];
        
    // The buffer must hold a frame
    if ( nc->recv_mbuf.len < calcFrameSize ) return false;
    
    crc calcCRC = ((uint16_t)nc->recv_mbuf.buf[ calcFrameSize - 2 ] << 8 ) +
                        (uint8_t)nc->recv_mbuf.buf[ calcFrameSize - 1 ];
    
    // CRC check (only if not disabled)
    crc crcnew;
    if ( !( ( nc->recv_mbuf.buf[ VSCP_MULTICAST_PACKET0_POS_HEAD_LSB ] | VSCP_HEADER_NO_CRC ) && 
            ( VSCP_NOCRC_CALC_DUMMY_CRC == calcCRC ) ) ) {
        
        // Calculate & check CRC
        crcnew = crcFast( (unsigned char const *)nc->recv_mbuf.buf + 1, 
                        VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 
                            ((uint16_t)nc->recv_mbuf.buf[ VSCP_MULTICAST_PACKET0_POS_VSCP_SIZE_MSB ] << 8 ) +
                             (uint8_t)nc->recv_mbuf.buf[ VSCP_MULTICAST_PACKET0_POS_VSCP_SIZE_LSB ] 
                            + 2 );
        // CRC is zero if calculated over itself
        if ( crcnew ) return false;
    }
    
    vscpEvent *pEvent;
    
    // Allocate a new event
    if ( NULL == ( pEvent = new vscpEvent ) ) return false;
    
    pEvent->sizeData =  ((uint16_t)nc->recv_mbuf.buf[ VSCP_MULTICAST_PACKET0_POS_VSCP_SIZE_MSB ] << 8 ) +
                        (uint8_t)nc->recv_mbuf.buf[ VSCP_MULTICAST_PACKET0_POS_VSCP_SIZE_LSB ];
    
    // Allocate data
    if ( NULL == ( pEvent->pdata = new uint8_t[ pEvent->sizeData ] ) ) {
        delete pEvent;
        return false;
    }
    
    // copy in data
    memcpy( pEvent->pdata, nc->recv_mbuf.buf + VSCP_MULTICAST_PACKET0_POS_VSCP_DATA, pEvent->sizeData );
    
    // Head
    pEvent->head = ((uint16_t)nc->recv_mbuf.buf[ VSCP_MULTICAST_PACKET0_POS_HEAD_MSB ] << 8 ) +
                        (uint8_t)nc->recv_mbuf.buf[ VSCP_MULTICAST_PACKET0_POS_HEAD_LSB ];
    
    // Copy in GUID
    memcpy( pEvent->GUID, nc->recv_mbuf.buf + VSCP_MULTICAST_PACKET0_POS_VSCP_GUID, pEvent->sizeData ); 
    
    // Set CRC
    pEvent->crc = calcCRC;
    
    // Set timestamp
    pEvent->timestamp = ( (uint32_t)nc->recv_mbuf.buf[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP ]  << 24 ) +
                        ( (uint32_t)nc->recv_mbuf.buf[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP + 1 ] << 16 ) +
                        ( (uint32_t)nc->recv_mbuf.buf[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP + 2 ] << 8 ) +
                        (uint8_t)nc->recv_mbuf.buf[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP + 3 ];
    
    // Date/time
    pEvent->year = (uint8_t)nc->recv_mbuf.buf[ VSCP_MULTICAST_PACKET0_POS_YEAR ];
    pEvent->month = (uint8_t)nc->recv_mbuf.buf[ VSCP_MULTICAST_PACKET0_POS_MONTH ];
    pEvent->day = (uint8_t)nc->recv_mbuf.buf[ VSCP_MULTICAST_PACKET0_POS_DAY ];
    pEvent->hour = (uint8_t)nc->recv_mbuf.buf[ VSCP_MULTICAST_PACKET0_POS_HOUR ];
    pEvent->minute = (uint8_t)nc->recv_mbuf.buf[ VSCP_MULTICAST_PACKET0_POS_MINUTE ];
    pEvent->second = (uint8_t)nc->recv_mbuf.buf[ VSCP_MULTICAST_PACKET0_POS_SECOND ];
    
    // VSCP Class
    pEvent->vscp_class = ( (uint16_t)nc->recv_mbuf.buf[ VSCP_MULTICAST_PACKET0_POS_VSCP_CLASS_MSB ] << 8 ) +
                           (uint8_t)nc->recv_mbuf.buf[ VSCP_MULTICAST_PACKET0_POS_VSCP_CLASS_LSB ];
    
    // VSCP Type
    pEvent->vscp_type = ( (uint16_t)nc->recv_mbuf.buf[ VSCP_MULTICAST_PACKET0_POS_VSCP_TYPE_MSB ] << 8 ) +
                          (uint8_t)nc->recv_mbuf.buf[ VSCP_MULTICAST_PACKET0_POS_VSCP_TYPE_LSB ];                        
    
    // obid - set to zero so interface fill it in
    pEvent->obid = 0;*/
    
    if ( vscp_doLevel2Filter( pEvent, pRxFilter ) ) {
    
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
// sendUnEncryptedFrame
//

bool 
VSCPUDPClientThread::sendUnEncryptedFrame( struct mg_connection *nc, 
                                                CClientItem *pClientItem )
{
    CLIENTEVENTLIST::compatibility_iterator nodeClient;
    unsigned char sendbuf[1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 + 512];    // Send buffer
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
    if ( pEvent->sizeData > VSCP_LEVEL2_MAXDATA ) return false;    
    
    // Packet type
    sendbuf[ VSCP_MULTICAST_PACKET0_POS_PKTTYPE ] = 0;
    
    // Head
    sendbuf[ VSCP_MULTICAST_PACKET0_POS_HEAD_MSB ] = ( pEvent->head >> 8 ) & 0xff;
    sendbuf[ VSCP_MULTICAST_PACKET0_POS_HEAD_MSB ] = pEvent->head & 0xff;
    
    // Timestamp
    sendbuf[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP ] = ( pEvent->timestamp >> 24 ) & 0xff;
    sendbuf[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP + 1 ] = ( pEvent->timestamp >> 16 ) & 0xff;
    sendbuf[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP + 2 ] = ( pEvent->timestamp >> 8 ) & 0xff;
    sendbuf[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP + 3 ] = pEvent->timestamp & 0xff;
    
    // UTC time block
    sendbuf[ VSCP_MULTICAST_PACKET0_POS_YEAR ] = pEvent->year;
    sendbuf[ VSCP_MULTICAST_PACKET0_POS_MONTH ] = pEvent->month;
    sendbuf[ VSCP_MULTICAST_PACKET0_POS_DAY ] = pEvent->day;
    sendbuf[ VSCP_MULTICAST_PACKET0_POS_HOUR ] = pEvent->hour;
    sendbuf[ VSCP_MULTICAST_PACKET0_POS_MINUTE ] = pEvent->minute;
    sendbuf[ VSCP_MULTICAST_PACKET0_POS_SECOND ] = pEvent->second;
    
    // VSCP Class
    sendbuf[ VSCP_MULTICAST_PACKET0_POS_VSCP_CLASS_MSB ] = ( pEvent->vscp_class >> 8 ) & 0xff;
    sendbuf[ VSCP_MULTICAST_PACKET0_POS_VSCP_CLASS_LSB ] = pEvent->vscp_class & 0xff;
    
    // VSCP Type
    sendbuf[ VSCP_MULTICAST_PACKET0_POS_VSCP_TYPE_MSB ] = ( pEvent->vscp_type >> 8 ) & 0xff;
    sendbuf[ VSCP_MULTICAST_PACKET0_POS_VSCP_TYPE_LSB ] = pEvent->vscp_type & 0xff;
    
    // size
    sendbuf[ VSCP_MULTICAST_PACKET0_POS_VSCP_SIZE_MSB ] = ( pEvent->sizeData >> 8 ) & 0xff;
    sendbuf[ VSCP_MULTICAST_PACKET0_POS_VSCP_SIZE_LSB ] = pEvent->sizeData & 0xff;
    
    // data    
    if ( pEvent->sizeData && ( NULL == pEvent->pdata ) ) {
        // This is invalid. We put in a zeroed data block
        memset( sendbuf +  VSCP_MULTICAST_PACKET0_POS_VSCP_DATA, 0, pEvent->sizeData );
    }
    else {
        // Copy in data
        memcpy(sendbuf + VSCP_MULTICAST_PACKET0_POS_VSCP_DATA, pEvent->pdata, pEvent->sizeData );
    }
    
    // Calculate CRC
    uint16_t crc = crcFast( sendbuf + VSCP_MULTICAST_PACKET0_POS_HEAD, 
                                VSCP_MULTICAST_PACKET0_HEADER_LENGTH + pEvent->sizeData );
    sendbuf[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + pEvent->sizeData ] = ( crc >> 8 ) & 0xff;
    sendbuf[ VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + pEvent->sizeData + 1 ] = crc & 0xff;
    
    // Send the event to all clients
    
    // Remove the event data node
    pClientItem->m_mutexClientInputQueue.Lock();
    pClientItem->m_clientInputQueue.DeleteNode( nodeClient );
    pClientItem->m_mutexClientInputQueue.Unlock();   
            
    return true;
}