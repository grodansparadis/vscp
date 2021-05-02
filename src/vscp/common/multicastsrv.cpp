// multicastsrv.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2021 Ake Hedman, the VSCP project
// <info@grodansparadis.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#define _POSIX

#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

#include "canal_macro.h"
//#include "canal_win32_ipc.h"
#include "clientlist.h"
//#include "daemonworker.h"
#include "vscp.h"
#include "vscphelper.h"
#include <controlobject.h>
#include <crc.h>
#include <crc8.h>
#include <dllist.h>
#include <guid.h>
#include <mongoose.h>
#include <vscp_aes.h>
#include <vscp_debug.h>



// Prototypes

MulticastObj::MulticastObj(CControlObject *pobj)
{
    m_bQuit       = false;
    m_pClientItem = NULL;
    m_sendSock    = 0;
    m_pObj        = pobj;
}

MulticastObj::~MulticastObj()
{
    if (NULL != m_pClientItem) {
        // Add the client to the Client List
        pthread_mutex_lock(&m_pObj->m_clientList.m_mutexItemList);
        m_pObj->removeClient(m_pClientItem);
        pthread_mutex_unlock(&m_pObj->m_clientList.m_mutexItemList);
    }
}

///////////////////////////////////////////////////////////////////////////////
// ev_handler
//
/*
void MulticastObj::ev_handler( struct mg_connection *nc, int ev, void *p )
{
   struct mbuf *io = &nc->recv_mbuf;
    MulticastObj *pMulticastClientThread =
                            (MulticastObj *)nc->mgr->user_data;

    switch (ev) {

        case MG_EV_RECV:
            {

                // Get remote address
                std::string remoteaddr = std::string( inet_ntoa(
nc->sa.sin.sin_addr )  ); int port = ntohs(nc->sa.sin.sin_port);

                // If a user is specified check that this user is allowed to
connect
                // from this location
                if ( NULL != pMulticastClientThread->m_pClientItem->m_pUserItem
) {

                    // Check if this user is allowed to connect from this
location pthread_mutex_lock(&m_pObj->m_mutex_UserList); bool bValidHost =
                            pMulticastClientThread->m_pClientItem->m_pUserItem->isAllowedToConnect(
nc->sa.sin.sin_addr.s_addr ); pthread_mutex_unlock(&m_pObj->m_mutex_UserList);

                    if ( !bValidHost ) {
                        std::string strErr = vscp_str_format(("[Multicast
channel] Host [%s] not allowed to send UDP datagrams.\n"), (const char
*)remoteaddr.mbc_str() ); syslog( LOG_ERR, strErr ); return;
                    }
                }

                // Must be at least a packet-type + header and a crc
                if ( nc->recv_mbuf.len < ( 1 +
VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 ) ) {

                    // Packet to short
                    syslog( LOG_ERR, vscp_str_format(("[Multicast channel] Frame
have invalid length = %d\n"), (int)nc->recv_mbuf.len ) ); return;

                }

                // If un-secure frames are not supported frames must be
encrypted if ( !pMulticastClientThread->m_pChannel->m_bAllowUnsecure &&
                        !GET_VSCP_MULTICAST_PACKET_ENCRYPTION(
nc->recv_mbuf.buf[ VSCP_MULTICAST_PACKET0_POS_PKTTYPE ] ) ) { syslog( LOG_ERR,
vscp_str_format(("[Multicast channel] Frame must be encrypted (or
m_bAllowUnsecure set to true) to be accepted.\n" ) ) ); return;
                }

                // OK receive the frame
                if ( receiveFrame( nc,
                                    pMulticastClientThread->m_pClientItem,
                                    &pMulticastClientThread->m_pChannel->m_rxFilter
)  ) {

                    if ( m_pObj->m_debugFlags[0] & VSCP_DEBUG1_MULTICAST ) {
                        syslog( LOG_ERR, vscp_str_format( ( "[Multicast channel]
Received Multicast event\n" ) ), DAEMON_LOGMSG_DEBUG, DAEMON_LOGTYPE_GENERAL );
                    }

                    if ( pMulticastClientThread->m_pChannel->m_bSendAck ) {
                        replyAckFrame( pMulticastClientThread,
                                        nc->recv_mbuf.buf[
VSCP_MULTICAST_PACKET0_POS_PKTTYPE ], ( nc->recv_mbuf.buf[
VSCP_MULTICAST_PACKET0_POS_HEAD_LSB ] & 0xf8 ) );
                    }

                }
                else {

                    if ( pMulticastClientThread->m_pChannel->m_bSendAck ) {
                        replyNackFrame( pMulticastClientThread,
                                        nc->recv_mbuf.buf[
VSCP_MULTICAST_PACKET0_POS_PKTTYPE ], ( nc->recv_mbuf.buf[
VSCP_MULTICAST_PACKET0_POS_HEAD_LSB ] & 0xf8 ) );
                    }

                }

            }
            break;

        default:
            break;
    }
}
*/

///////////////////////////////////////////////////////////////////////////////
// sendFrame
//

bool
MulticastObj::sendFrame(struct mg_mgr *pmgr, CClientItem *pClientItem)
{ /*
     CLIENTEVENTLIST::compatibility_iterator nodeClient;
     unsigned char sendbuf[1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 +
                             VSCP_LEVEL2_MAXDATA + 16];    // Send buffer
     uint8_t iv[16];

     // Check if there is an event to send
     size_t cnt;
     if ( !( cnt = pClientItem->m_clientInputQueue.size() ) ) {
         return false;
     }

     pthread_mutex_lock( &pClientItem->m_mutexClientInputQueue );
     vscpEvent *pEvent = pClientItem->m_clientInputQueue.front();
     pClientItem->m_clientInputQueue.pop_front();
     pthread_mutex_unlock( &pClientItem->m_mutexClientInputQueue );

     if ( NULL == pEvent ) return false;

     // Check that size is valid
     if ( pEvent->sizeData > VSCP_LEVEL2_MAXDATA ) {
         vscp_deleteEvent_v2( &pEvent );
         return false;
     }

     // Check if filtered out
     if ( vscp_doLevel2Filter( pEvent, &m_pChannel->m_txFilter ) ) {

         // Packet type
         sendbuf[ VSCP_MULTICAST_PACKET0_POS_PKTTYPE ] =
                     SET_VSCP_MULTICAST_TYPE( 0, m_pChannel->m_nEncryption );

         // Get initialization vector
         getRandomIV( iv, 16 );

         uint8_t wrkbuf[1024];
         memset( wrkbuf, 0, sizeof(wrkbuf) );

         // Write rolling index
         pEvent->head &= 0xf8;
         pEvent->head |= (0x07 &m_pChannel->m_index );
         m_pChannel->m_index++;

         if ( !vscp_writeEventToUdpFrame( wrkbuf,
                                             sizeof( wrkbuf),
                                             SET_VSCP_MULTICAST_TYPE( 0,
 m_pChannel->m_nEncryption ), pEvent ) ) { vscp_deleteEvent_v2( &pEvent );
             return false;
         }

         size_t len = 1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH +
 pEvent->sizeData + 2; if ( 0 == ( len = vscp_encryptVscpUdpFrame( sendbuf,
                                                 wrkbuf,
                                                 len,
                                                 m_pObj->getSystemKey( NULL ),
                                                 iv,
                                                 m_pChannel->m_nEncryption ) ) )
 { vscp_deleteEvent_v2( &pEvent ); return false;
         }

 #if 0
         int i;
         wxPrintf("IV = ");
         for ( i=0; i<16; i++ ) {
             wxPrintf("%02X ", iv[i] );
         }
         wxPrintf("\n");

         wxPrintf("key = ");
         uint8_t *pkey = m_pObj->getSystemKey( NULL );
         for ( i=0; i<32; i++ ) {
             wxPrintf("%02X ", pkey[i] );
         }
         wxPrintf("\n");
 #endif

         //mg_connect( pmgr, (const char *)m_sendAddress.mbc_str(), ev_handler
 );
         //mg_send( pmgr->active_connections, sendbuf, len );

         if ( ( sendto( m_sendSock, sendbuf, len, 0,
                             (struct sockaddr *) &m_mc_sendAddr,
                             sizeof( m_mc_sendAddr ) ) ) != len ) {
             // Fail
             syslog( LOG_ERR, ("[Multicast channel] Failed to send multicast
 frame.\n" ) );
         }

     } // filter

     // Remove the event data node
     pthread_mutex_lock(&pClientItem->m_mutexClientInputQueue);
     pClientItem->m_clientInputQueue.DeleteNode( nodeClient );
     pthread_mutex_unlock(&pClientItem->m_mutexClientInputQueue);
    */
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// receiveFrame
//

bool
MulticastObj::receiveFrame(struct mg_connection *nc,
                           CClientItem *pClientItem,
                           vscpEventFilter *pRxFilter)
{ /*
     uint8_t buf[ 1024 ];
     vscpEvent *pEvent;

     // Check pointers
     if ( NULL == nc ) return false;
     if ( NULL == pClientItem ) return false;
     if ( NULL == pRxFilter ) return false;

 #if 0
     int i;
     wxPrintf("RECEIVE (%d) = ", (int)nc->recv_mbuf.len );
     for ( i=0; i<nc->recv_mbuf.len; i++ ) {
         wxPrintf("%02X ", (uint8_t)nc->recv_mbuf.buf[i] );
     }
     wxPrintf("\n");
 #endif

     memset( buf, 0, sizeof( buf ) );
     if ( !vscp_decryptVscpUdpFrame( buf,
                 (uint8_t *)nc->recv_mbuf.buf,
                 nc->recv_mbuf.len,   // actually len-16 but encrypt routine
 handle this m_pObj->getSystemKey( NULL ), NULL,   // Will be copied from the
 last 16-bytes GET_VSCP_MULTICAST_PACKET_ENCRYPTION( nc->recv_mbuf.buf[0] ) ) )
 { return false;
     };

     // Allocate a new event
     if ( NULL == ( pEvent = new vscpEvent ) ) return false;
     pEvent->pdata = NULL;

     if ( !vscp_getEventFromUdpFrame( pEvent, buf, nc->recv_mbuf.len ) ) {
         vscp_deleteEvent_v2( &pEvent );
         return false;
     }

     if ( vscp_doLevel2Filter( pEvent, pRxFilter ) ) {

         // Set obid to ourself so we don't get events we
         // receive
         pEvent->obid = pClientItem->m_clientID;

         // There must be room in the receive queue (even if room (or whisky)
 has been better) if ( m_pObj->m_maxItemsInClientReceiveQueue >
                     m_pObj->m_clientOutputQueue.GetCount() ) {

             pthread_mutex_lock(&m_pObj->m_mutex_ClientOutputQueue);
             m_pObj->m_clientOutputQueue.push_back(pEvent);
             sem_post(&m_pObj->m_semClientOutputQueue);
             pthread_mutex_unlock(&m_pObj->m_mutex_ClientOutputQueue);

         }
         else {
             vscp_deleteEvent_v2( &pEvent );
         }
     }
     else {
         vscp_deleteEvent_v2( &pEvent );
     }
    */
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// replyAckFrame
//

bool
MulticastObj::replyAckFrame(MulticastObj *pMulticastClientThread,
                            uint8_t pkttype,
                            uint8_t index)
{ /*
     unsigned char sendbuf[1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 +
                             VSCP_LEVEL2_MAXDATA + 16];    // Send buffer

     // Check pointer
     if ( NULL == pMulticastClientThread ) return false;

     vscpEventEx ex;

     ex.head = ( index & 0xf8 );
     ex.crc = 0;
     ex.obid = 0;
     ex.timestamp = vscp_makeTimeStamp();
     ex.year = vscpdatetime::UTCNow().GetYear();
     ex.month = vscpdatetime::UTCNow().GetMonth();
     ex.day = vscpdatetime::UTCNow().GetDay();
     ex.hour = vscpdatetime::UTCNow().GetHour();
     ex.minute = vscpdatetime::UTCNow().GetMinute();
     ex.second = vscpdatetime::UTCNow().GetSecond();
     memcpy( ex.GUID, pMulticastClientThread->m_pClientItem->m_guid.getGUID(),
     16 ); ex.vscp_class = VSCP_CLASS1_ERROR; ex.vscp_type =
     VSCP_TYPE_ERROR_SUCCESS; ex.sizeData = 3; memset( ex.data, 0, 3 );    //
     index/zone/subzone = 0 ex.data[0] = index;

     if ( !vscp_writeEventExToUdpFrame( sendbuf,
                                         sizeof( sendbuf ),
                                         pkttype,
                                         &ex ) ) {
         return false;
     }

     // Send to remote node
     //mg_send( nc, sendbuf, 1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 +
     ex.sizeData ); size_t len = 1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 +
     ex.sizeData; if ( ( sendto( pMulticastClientThread->m_sendSock, sendbuf,
     len, 0, (struct sockaddr *) &pMulticastClientThread->m_mc_sendAddr, sizeof(
     pMulticastClientThread->m_mc_sendAddr ) ) ) != len ) {
         // Fail
         syslog( LOG_ERR, "[Multicast channel] Failed to send ACK multicast
     frame.\n"  );
     }
     */
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// replyNackFrame
//

bool
MulticastObj::replyNackFrame(MulticastObj *pMulticastClientThread,
                             uint8_t pkttype,
                             uint8_t index)
{
    unsigned char sendbuf[1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 +
                          VSCP_LEVEL2_MAXDATA + 16]; // Send buffer

    // Check pointer
    if (NULL == pMulticastClientThread) return false;
    /*
       vscpEventEx ex;

       ex.head = ( index & 0xf8 );
       ex.crc = 0;
       ex.obid = 0;
       ex.timestamp = vscp_makeTimeStamp();
       ex.year = vscpdatetime::UTCNow().GetYear();
       ex.month = vscpdatetime::UTCNow().GetMonth();
       ex.day = vscpdatetime::UTCNow().GetDay();
       ex.hour = vscpdatetime::UTCNow().GetHour();
       ex.minute = vscpdatetime::UTCNow().GetMinute();
       ex.second = vscpdatetime::UTCNow().GetSecond();
       memcpy( ex.GUID, pMulticastClientThread->m_pClientItem->m_guid.getGUID(),
       16 ); ex.vscp_class = VSCP_CLASS1_ERROR; ex.vscp_type =
       VSCP_TYPE_ERROR_ERROR; ex.sizeData = 3; memset( ex.data, 0, 3 );    //
       index/zone/subzone = 0 ex.data[0] = index;

       if ( !vscp_writeEventExToUdpFrame( sendbuf,
                                           sizeof( sendbuf ),
                                           pkttype,
                                           &ex ) ) {
           return false;
       }

       // Send to remote node
       //mg_send( nc, sendbuf, 1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 +
       ex.sizeData ); size_t len = 1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2
       + ex.sizeData; if ( ( sendto( pMulticastClientThread->m_sendSock,
       sendbuf, len, 0, (struct sockaddr *)
       &pMulticastClientThread->m_mc_sendAddr, sizeof(
       pMulticastClientThread->m_mc_sendAddr ) ) ) != len ) {
           // Fail
           syslog( LOG_ERR, ("[Multicast channel] Failed to send NACK multicast
       frame.\n" ) );
       }
    */
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// multicastClientThread
//

void *
multicastClientThread(void *pData)
{
    struct mg_mgr mgr;
    struct mg_connection *nc;      // Receive connection
    struct ip_mreq group;          // Group
    struct ip_mreq_source mreqsrc; // Source block
    const char *mcast_default_group = VSCP_MULTICAST_IPV4_ADDRESS_STR;
    /*
       MulticastObj *pmobj = (MulticastObj *)pData;

       mg_mgr_init( &mgr, pData );

       // Init. CRC data
       crcInit();

       // Bind to interface
       std::string bindif = vscp_str_format( ("udp://%d"),
       pmobj->m_pChannel->m_port ); if ( NULL == ( nc = mg_bind( &mgr, bindif,
       // "udp://44444" ev_handler ) ) ) { std::string str = ( "[Multicast
       channel] Can not bind to interface "); str += bindif; syslog( LOG_ERR,
       str ); mg_mgr_free( &mgr ); return NULL;
       }

       int one = 1;
       setsockopt( nc->sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof( one ) );

       group.imr_multiaddr.s_addr = inet_addr( (const char
       *)m_pChannel->m_gropupAddress.mbc_str() ); group.imr_interface.s_addr =
       inet_addr( (const char *)m_pChannel->m_public.mbc_str() );  // INADDR_ANY
       does not work

       // Join group
       if ( setsockopt( nc->sock,
                               IPPROTO_IP,
                               IP_ADD_MEMBERSHIP,
                               (char *)&group,
                               sizeof( group ) ) < 0 ) {
           syslog( LOG_ERR, ( "[Multicast channel] Unable to add to multicast
       group.\n" )  ); mg_mgr_free( &mgr ); return NULL;
       }

       mreqsrc.imr_interface = group.imr_interface;
       mreqsrc.imr_multiaddr = group.imr_multiaddr;
       mreqsrc.imr_sourceaddr.s_addr = inet_addr( (const char
       *)m_pChannel->m_public.mbc_str() ); if ( 0 > setsockopt( nc->sock,
       IPPROTO_IP, IP_BLOCK_SOURCE, (char *)&mreqsrc, sizeof( mreqsrc ) ) ) {
           syslog( LOG_ERR, ( "[Multicast channel] Unable to block source.\n" )
       ); mg_mgr_free( &mgr ); return NULL;
       }

       // We don't want to receive what we send
       u_char loop = 0;  // Disable
       if ( setsockopt( nc->sock, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(
       loop ) ) < 0 ) { syslog( LOG_ERR, ( "[Multicast channel] Failed to reset
       option IP_MULTICAST_LOOP.\n" )  );
       }

       // Set ttl
       u_char ttl = m_pChannel->m_ttl;
       if ( setsockopt( nc->sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(
       ttl ) ) < 0 ) { syslog( LOG_ERR, ( "[Multicast channel] Failed to set
       ttl.\n" )  ); return NULL;
       }

       {
           std::string str = ( "[Multicast channel] Starting multicast channel
       on group "); str += m_pChannel->m_gropupAddress; str +=
       vscp_str_format((":%d"), m_pChannel->m_port ); syslog( LOG_ERR, str );
       }

       // We need to create a client object and add this object to the list
       m_pClientItem = new CClientItem;
       if ( NULL == m_pClientItem ) {
           syslog( LOG_ERR, ( "[Multicast channel] Unable to allocate memory for
       client, terminating.\n" )  ); mg_mgr_free( &mgr ); return NULL;
       }

       // * * * Setup Sending * * *

       // Create a socket for sending to the multicast address
       if ( ( m_sendSock = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) < 0 ) {
           syslog( LOG_ERR, ( "[Multicast channel] Unable to create sending
       socket.\n" )  ); mg_mgr_free( &mgr ); return NULL;
       }

       // Set the TTL (time to live/hop count) for the send
       if ( setsockopt( nc->sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(
       ttl ) ) < 0 ) { syslog( LOG_ERR, ( "[Multicast channel] Unable to set ttl
       for sending socket.\n" )  ); mg_mgr_free( &mgr ); close( m_sendSock );
           return NULL;
       }

       // construct a multicast address structure  for the send
       memset( &m_mc_sendAddr, 0, sizeof( m_mc_sendAddr ) );
       m_mc_sendAddr.sin_family      = AF_INET;
       m_mc_sendAddr.sin_addr.s_addr = inet_addr( (const char
       *)m_pChannel->m_gropupAddress.mbc_str() ); m_mc_sendAddr.sin_port =
       htons( m_pChannel->m_port );


       // * * * End of send setup * * *

       // This is now an active Client
       m_pClientItem->m_bOpen = true;
       m_pClientItem->m_type =  CLIENT_ITEM_INTERFACE_TYPE_CLIENT_MULTICAST_CH;
       m_pClientItem->m_strDeviceName = ("Multicast channel. [");
       m_pClientItem->m_strDeviceName += m_pChannel->m_gropupAddress;
       m_pClientItem->m_strDeviceName += vscp_str_format(":%d",
       m_pChannel->m_port ); m_pClientItem->m_strDeviceName += ("]|Started at
       "); m_pClientItem->m_strDeviceName +=
       vscpdatetime::Now().getISODateTime();

       m_pClientItem->m_pUserItem = NULL;  // No user defined

       // Add the client to the Client List
       pthread_mutex_lock(&m_pObj->m_clientList.m_mutexItemList);
       if ( !m_pObj->addClient( m_pClientItem, CLIENT_ID_MULTICAST_SRV ) ) {
           // Failed to add client
           delete m_pClientItem;
           m_pClientItem = NULL;
           pthread_mutex_unlock(&m_pObj->m_clientList.m_mutexItemList);
           syslog( LOG_ERR, ("Multicat client: Failed to add client. Terminating
       thread.") ); return NULL;
       }
       pthread_mutex_unlock(&m_pObj->m_clientList.m_mutexItemList);

       // Set receive filter
       memcpy( &m_pClientItem->m_filter, &m_pChannel->m_rxFilter,
       sizeof(vscpEventFilter) );

       // Set GUID for channel
       if ( !m_pChannel->m_guid.isNULL() ) {
           m_pClientItem->m_guid = m_pChannel->m_guid;
       }

       while ( !TestDestroy() && !m_bQuit ) {
           mg_mgr_poll( &mgr, 50 );
           sendFrame( &mgr, m_pClientItem );
       }

       // Drop multicast membership
       setsockopt( nc->sock,
                     IPPROTO_IP,
                     IP_DROP_MEMBERSHIP,
                     (char *)&group,
                      sizeof( group ) );

       mg_mgr_free( &mgr );
       close( m_sendSock );
   */
    return NULL;
}