// udpclientthread.cpp
//
// This file is part of the VSCP (http://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2019 Ake Hedman, Grodans Paradis AB
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

#include <string>

#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>

#include <canal.h>
#include <canal_macro.h>
#include <canal_win32_ipc.h>
#include <controlobject.h>
#include <crc.h>
#include <dllist.h>
#include <mongoose.h>
#include <tcpipsrv.h>
#include <version.h>
#include <vscp.h>
#include <vscp_aes.h>
#include <vscphelper.h>

#include "udpsrv.h"

///////////////////////////////////////////////////////////////////////////////
// UDPSrvObj
//
// This thread listens for connection on a TCP socket and starts a new thread
// to handle client requests
//

UDPSrvObj::UDPSrvObj(CControlObject *pobj)
{
    m_bQuit       = false;
    m_pClientItem = NULL;
    setControlObjectPointer(pobj);
    pthread_mutex_init(&m_mutexUDPInfo, NULL);
}

UDPSrvObj::~UDPSrvObj()
{
    pthread_mutex_destroy(&m_mutexUDPInfo);
}

///////////////////////////////////////////////////////////////////////////////
// ev_handler
//

void
UDPSrvObj::ev_handler(struct mg_connection *nc, int ev, void *p)
{
    struct mbuf *io = &nc->recv_mbuf;
    UDPSrvObj *pObj = (UDPSrvObj *)nc->mgr->user_data;
    /*
        switch (ev) {

            case MG_EV_CLOSE:
                break;

            case MG_EV_RECV: {
                syslog( LOG_ERR, "Received UDP event" );

                // If a user is specified check that this user is allowed to
       connect
                // from this location
                if (NULL != pObj->m_pClientItem->m_pUserItem) {

                    // Get remote address
                    std::string remoteaddr =
                      std::string(inet_ntoa(nc->sa.sin.sin_addr));

                    // Check if this user is allowed to connect from this
       location pthread_mutex_lock(&pObj->m_pobj->m_mutexUserList); bool
       bValidHost =
                      // pObj->m_pClientItem->m_pUserItem->isAllowedToConnect(
                      //        remoteaddr );
                      (1 == pObj->m_pClientItem->m_pUserItem
                              ->isAllowedToConnect(nc->sa.sin.sin_addr.s_addr));
                    pthread_mutex_lock(&pObj->m_pobj->m_mutexUserList);

                    if (!bValidHost) {
                        syslog( LOG_ERR, "[UDP Client] Host [%s] not "
                                                "allowed to send UDP
       datagrams.", (const char *)remoteaddr.c_str()); return;
                    }
                }

                // Must be at least a packet-type + header and a crc
                if (nc->recv_mbuf.len <
                    (1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2)) {

                    // Packet to short
                    syslog( LOG_ERR,
                      "UDP frame have invalid length = %d",
                                          (int)nc->recv_mbuf.len );
                    return;
                }

                // If un-secure frames are not supported frames must be
       encrypted if (!pObj->m_pobj->m_udpSrvObj.m_bAllowUnsecure &&
                    !GET_VSCP_MULTICAST_PACKET_ENCRYPTION(
                      nc->recv_mbuf.buf[VSCP_MULTICAST_PACKET0_POS_PKTTYPE])) {
                    syslog( LOG_ERR, "UDP frame must be encrypted (or
       m_bAllowUnsecure set to " "true) to be accepted."); return;
                }

                // OK receive the frame
                if (receiveFrame(nc,
                                 pObj->m_pClientItem,
                                 &pObjm_pobj->m_udpSrvObj.m_filter)) {

                    if (pObjm_pobj->m_udpSrvObj.m_bAck) {
                        replyAckFrame(
                          nc,
                          nc->recv_mbuf.buf[VSCP_MULTICAST_PACKET0_POS_PKTTYPE],
                          (nc->recv_mbuf.buf[VSCP_MULTICAST_PACKET0_POS_HEAD_LSB]
       & 0xf8));
                    }

                } else {

                    if (pObjm_pobj->m_udpSrvObj.m_bAck) {
                        replyNackFrame(
                          nc,
                          nc->recv_mbuf.buf[VSCP_MULTICAST_PACKET0_POS_PKTTYPE],
                          (nc->recv_mbuf.buf[VSCP_MULTICAST_PACKET0_POS_HEAD_LSB]
       & 0xf8));
                    }
                }

            } break;

            case MG_EV_POLL:
                break;

            case MG_EV_CONNECT: {
                int bConnect = *(int *)p;
                if (bConnect == 0) {
                    // Success
                    // mg_send(nc, "test", 4);
                    // nc->flags |= MG_F_SEND_AND_CLOSE;
                } else {
                    // Error
                    nc->flags |= MG_F_SEND_AND_CLOSE;
                    // printf("connect() error: %s\n", strerror( bConnect ) );
                }
                if (nc->send_mbuf.len) {
                    // mg_send(nc, nc->send_mbuf.buf, nc->send_mbuf.len);
                }
            } break;

            case MG_EV_SEND:
                // nc->flags |= MG_F_SEND_AND_CLOSE;
                break;

            default:
                break;
        }
     */
}

////////////////////////////////////////////////////////////////////////////////
// receiveFrame
//

bool
UDPSrvObj::receiveFrame(struct mg_connection *nc,
                        CClientItem *pClientItem,
                        vscpEventFilter *pRxFilter)
{
    uint8_t buf[1024];
    vscpEvent *pEvent;

    // Check pointers
    if (NULL == nc) return false;
    if (NULL == pClientItem) return false;
    if (NULL == pRxFilter) return false;
    /* TODO
        memset(buf, 0, sizeof(buf));
        if (!vscp_decryptVscpUdpFrame(
              buf,
              (uint8_t *)nc->recv_mbuf.buf,
              nc->recv_mbuf.len, // actually len-16 but encrypt routine handle
       this m_pobj->getSystemKey(NULL), NULL, // Will be copied from the last
       16-bytes GET_VSCP_MULTICAST_PACKET_ENCRYPTION(nc->recv_mbuf.buf[0]))) {
            return false;
        };

        // Allocate a new event
        if (NULL == (pEvent = new vscpEvent)) return false;
        pEvent->pdata = NULL;

        if (!vscp_getEventFromUdpFrame(pEvent, buf, nc->recv_mbuf.len)) {
            vscp_deleteVSCPevent_v2(&pEvent);
            return false;
        }

        if (vscp_doLevel2Filter(pEvent, pRxFilter)) {

            // Set obid to ourself so we don't get events we
            // receive
            pEvent->obid = pClientItem->m_clientID;

            // There must be room in the receive queue (even if room (or whisky)
       has
            // been better)
            if (m_pobj->m_maxItemsInClientReceiveQueue >
                m_pobj->m_clientOutputQueue.size()) {

                pthread_mutex_lock(&m_pobj->m_mutexClientOutputQueue);
                m_pobj->m_clientOutputQueue.push_back(pEvent);
                sem_post(&m_pobj->m_semClientOutputQueue);
                pthread_mutex_lock(&m_pobj->m_mutexClientOutputQueue);

            } else {
                vscp_deleteVSCPevent_v2(&pEvent);
            }
        } else {
            vscp_deleteVSCPevent_v2(&pEvent);
        }
    */
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// sendFrame
//

bool
UDPSrvObj::sendFrame(struct mg_mgr *pmgr, CClientItem *pClientItem)
{
    /*
       CLIENTEVENTLIST::compatibility_iterator nodeClient;
       unsigned char sendbuf[1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 +
                             VSCP_LEVEL2_MAXDATA + 16]; // Send buffer
       uint8_t iv[16];

       // Check if there is an event to send
       if (!pClientItem->m_clientInputQueue.size()) {
           return false;
       }

       pthread_mutex_lock(&pClientItem->m_mutexClientInputQueue);
       vscpEvent *pEvent = pClientItem->m_clientInputQueue.front();
       pClientItem->m_clientInputQueue.pop_front();
       pthread_mutex_unlock(&pClientItem->m_mutexClientInputQueue);

       if (NULL == pEvent) return false;

       // Check that size is valid
       if (pEvent->sizeData > VSCP_LEVEL2_MAXDATA) {
           vscp_deleteVSCPevent_v2(&pEvent);
           return false;
       }

       // Send the event to all clients
       pthread_mutex_lock(m_mutexUDPInfo);
       udpRemoteClientList::iterator iter;

       for (iter = m_pobj->m_udpSrvObj.m_remotes.begin();
            iter != m_pobj->m_udpSrvObj.m_remotes.end();
            ++iter) {

           udpRemoteClientInfo *pRemoteUDPNode = *iter;

           // Check if filtered out
           if (!vscp_doLevel2Filter(pEvent, &pRemoteUDPNode->m_filter)) {
               continue;
           }

           // Packet type
           sendbuf[VSCP_MULTICAST_PACKET0_POS_PKTTYPE] =
             SET_VSCP_MULTICAST_TYPE(0, pRemoteUDPNode->m_nEncryption);

           // Get initialization vector
           getRandomIV(iv, 16);

           uint8_t wrkbuf[1024];
           memset(wrkbuf, 0, sizeof(wrkbuf));

           if (!vscp_writeEventToUdpFrame(
                 wrkbuf,
                 sizeof(wrkbuf),
                 SET_VSCP_MULTICAST_TYPE(0, pRemoteUDPNode->m_nEncryption),
                 pEvent)) {
               vscp_deleteVSCPevent_v2(&pEvent);
               continue;
           }

           // Write rolling index
           wrkbuf[VSCP_MULTICAST_PACKET0_POS_HEAD_LSB] &= 0xf8;
           wrkbuf[VSCP_MULTICAST_PACKET0_POS_HEAD_LSB] |=
             (0x07 & pRemoteUDPNode->m_index);
           pRemoteUDPNode->m_index++;

           size_t len =
             1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + pEvent->sizeData + 2;
           if (0 ==
               (len = vscp_encryptVscpUdpFrame(sendbuf,
                                               wrkbuf,
                                               len,
                                               m_pobj->getSystemKey(NULL),
                                               iv,
                                               pRemoteUDPNode->m_nEncryption)))
   { vscp_deleteVSCPevent_v2(&pEvent); continue;
           }
   #if 0
               int i;
               xxPrintf("IV = ");
               for (i = 0; i < 16; i++) {
                   xxPrintf("%02X ", iv[i]);
               }
               xxPrintf("\n");

               xxPrintf("key = ");
               uint8_t *pkey = m_pobj->getSystemKey(NULL);
               for (i = 0; i < 32; i++) {
                   xxPrintf("%02X ", pkey[i]);
               }
               xxPrintf("\n");
   #endif
           mg_connect(pmgr, pRemoteUDPNode->m_remoteAddress, ev_handler);
           mg_send(pmgr->active_connections, sendbuf, len);
       }

       pthread_mutex_lock(&m_mutexUDPInfo);

       // Remove the event data node
       pthread_mutex_lock(&pObj->pClientItem->m_mutexClientInputQueue);
       pClientItem->m_clientInputQueue.DeleteNode(nodeClient);
       pthread_mutex_lock(&pClientItem->m_mutexClientInputQueue);
   */
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// replyAckFrame
//

bool
UDPSrvObj::replyAckFrame(struct mg_connection *nc,
                         uint8_t pkttype,
                         uint8_t index)
{
    /*
       unsigned char sendbuf[1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 +
                             VSCP_LEVEL2_MAXDATA + 16]; // Send buffer
       vscpEventEx ex;

       ex.head      = (index & 0xf8);
       ex.crc       = 0;
       ex.obid      = 0;
       ex.timestamp = vscp_makeTimeStamp();
       ex.year      = vscpdatetime::setUTCNow().getYear();
       ex.month     = vscpdatetime::setUTCNow().getMonth();
       ex.day       = vscpdatetime::setUTCNow().getDay();
       ex.hour      = vscpdatetime::setUTCNow().getHour();
       ex.minute    = vscpdatetime::setUTCNow().getMinute();
       ex.second    = vscpdatetime::setUTCNow().getSecond();
       memcpy(ex.GUID, m_pobj->m_udpSrvObj.m_guid.getGUID(), 16);
       ex.vscp_class = VSCP_CLASS1_ERROR;
       ex.vscp_type  = VSCP_TYPE_ERROR_SUCCESS;
       ex.sizeData   = 3;
       memset(ex.data, 0, 3); // index/zone/subzone = 0
       ex.data[0] = index;

       if (!vscp_writeEventExToUdpFrame(sendbuf, sizeof(sendbuf), pkttype, &ex))
       { return false;
       }

       // Send to remote node
       mg_send(
         nc, sendbuf, 1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 +
       ex.sizeData);
   */
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// replyNackFrame
//

bool
UDPSrvObj::replyNackFrame(struct mg_connection *nc,
                          uint8_t pkttype,
                          uint8_t index)
{
    /*
       unsigned char sendbuf[1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 +
                             VSCP_LEVEL2_MAXDATA + 16]; // Send buffer
       vscpEventEx ex;

       ex.head      = (index & 0xf8);
       ex.crc       = 0;
       ex.obid      = 0;
       ex.timestamp = vscp_makeTimeStamp();
       ex.year      = vscpdatetime::setUTCNow().getYear();
       ex.month     = vscpdatetime::setUTCNow().getMonth();
       ex.day       = vscpdatetime::setUTCNow().getDay();
       ex.hour      = vscpdatetime::setUTCNow().getHour();
       ex.minute    = vscpdatetime::setUTCNow().getMinute();
       ex.second    = vscpdatetime::setUTCNow().getSecond();
       memcpy(ex.GUID, m_pobj->m_udpSrvObj.m_guid.getGUID(), 16);
       ex.vscp_class = VSCP_CLASS1_ERROR;
       ex.vscp_type  = VSCP_TYPE_ERROR_ERROR;
       ex.sizeData   = 3;
       memset(ex.data, 0, 3); // index/zone/subzone = 0
       ex.data[0] = index;

       if (!vscp_writeEventExToUdpFrame(sendbuf, sizeof(sendbuf), pkttype, &ex))
       { return false;
       }

       // Send to remote node
       mg_send(
         nc, sendbuf, 1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 +
       ex.sizeData);
   */
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// UDPThread
//

void *
UDPThread(void *pData)
{
    struct mg_mgr mgr;
    struct mg_connection *nc;

    // Init. CRC data
    crcInit();
    /*
        mg_mgr_init(&mgr, this);

        if (wxNOT_FOUND == m_pobj->m_udpSrvObj.m_interface.Find(("udp://"))) {
            m_pobj->m_udpSrvObj.m_interface =
              ("udp://") + m_pobj->m_udpSrvObj.m_interface;
        }

        // Bind to this interface
        if (NULL == (nc = mg_bind(&mgr,
                                  m_pobj->m_udpSrvObj.m_interface.c_str(),
                                  UDPSrvObj::ev_handler))) {
            syslog( LOG_ERR, "UDP Client: Failed to bind to interface. [%s]",
               m_pobj->m_udpSrvObj.m_interface.c_str() );
            return NULL;
        }

        syslog( LOG_ERR, "UDP Client: Bind to interface. [%s]",
                        m_pobj->m_udpSrvObj.m_interface  );

        // We need to create a client object and add this object to the list
        m_pClientItem = new CClientItem;
        if (NULL == m_pClientItem) {
            syslog( LOG_ERR,
              "[UDP Client] Unable to allocate memory for client,
       terminating."); return NULL;
        }

        // This is now an active Client
        m_pClientItem->m_bOpen         = true;
        m_pClientItem->m_type          = CLIENT_ITEM_INTERFACE_TYPE_CLIENT_UDP;
        m_pClientItem->m_strDeviceName = ("Internal UDP client listner. [");
        m_pClientItem->m_strDeviceName += m_pobj->m_udpSrvObj.m_interface;
        m_pClientItem->m_strDeviceName += ("]|Started at ");
        m_pClientItem->m_strDeviceName +=
       vscpdatetime::setNow().getISODateTime();

        // If a user is defined get user item
        if (m_pobj->m_udpSrvObj.m_user.Trim().Length()) {
            pthread_mutex_lock(&m_pobj->m_clientMutex);
            m_pClientItem->m_pUserItem = m_pobj->m_userList.validateUser(
              m_pobj->m_udpSrvObj.m_user, m_pobj->m_udpSrvObj.m_password);
            pthread_mutex_lock(&m_pobj->m_mutexUserList);

            if (NULL == m_pClientItem->m_pUserItem) {

                delete m_pClientItem;

                syslog( LOG_ERR,
                  "[UDP Client] User [%s] NOT allowed to connect.",
                  (const char *)m_pobj->m_udpSrvObj.m_user.c_str());
                return NULL;
            }
        } else {
            m_pClientItem->m_pUserItem = NULL; // No user defined
        }

        // Add the client to the Client List
        pthread_mutex_lock(&m_pobj->m_clientMutex);
        if (!m_pobj->addClient(m_pClientItem, CLIENT_ID_UDP_LISTNER)) {
            // Failed to add client
            delete m_pClientItem;
            m_pClientItem = NULL;
            pthread_mutex_lock(&m_pobj->m_clientMutex);
            syslog( LOG_ERR,
              "UDP server: Failed to add client. Terminating thread.");
            return NULL;
        }
        pthread_mutex_lock(&m_pobj->m_clientMutex);

        // Set receive filter
        memcpy(&m_pClientItem->m_filterVSCP,
               &m_pobj->m_udpSrvObj.m_filter,
               sizeof(vscpEventFilter));

        // Set GUID for channel
        if (!m_pobj->m_udpSrvObj.m_guid.isNULL()) {
            m_pClientItem->m_guid = m_pobj->m_udpSrvObj.m_guid;
        }

        syslog( LOG_ERR, "UDP Client: Thread started.");

        while (!m_bQuit) {
            mg_mgr_poll(&mgr, 50);
            sendFrame(&mgr, m_pClientItem);
        }

        // release the server
        mg_mgr_free(&mgr);

        if (NULL != m_pClientItem) {
            // Add the client to the Client List
            pthread_mutex_lock(&m_pobj->m_clientMutex);
            m_pobj->removeClient(m_pClientItem);
            pthread_mutex_unlock(&m_pobj->m_clientMutex);
        }

        syslog( LOG_ERR, "UDP ClientThread: Quit.");
    */
    return NULL;
}