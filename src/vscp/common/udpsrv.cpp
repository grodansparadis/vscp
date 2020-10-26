// udpsrv.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2020 Ake Hedman, Grodans Paradis AB
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
#include <arpa/inet.h>

#include <controlobject.h>
#include <crc.h>
#include <dllist.h>
#include <tcpipsrv.h>
#include <version.h>
#include <vscp.h>
#include <vscp_aes.h>
#include <vscphelper.h>

#include "udpsrv.h"

///////////////////////////////////////////////////////////////////////////////
// get_ip_str
//
// Convert a struct sockaddr address to a string, IPv4 and IPv6:
//

char *get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen)
{
    switch(sa->sa_family) {
        
        case AF_INET:
            inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr),
                    s, maxlen);
            break;

        case AF_INET6:
            inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr),
                    s, maxlen);
            break;

        default:
            strncpy(s, "Unknown AF", maxlen);
            return NULL;
    }

    return s;
}

// -----------------------------------------------------------------------------

udpRemoteClient::udpRemoteClient(CControlObject *pobj)
{
    m_bQuit = false;
    m_bEnable = false;
    m_sockfd = 0;
    m_index = 0;    // Rolling index starts at zero
    m_nEncryption  = VSCP_ENCRYPTION_NONE;
    m_bSetBroadcast = false;
    m_pClientItem = NULL;

    setControlObjectPointer(pobj);   
}

udpRemoteClient::~udpRemoteClient()
{
    close(m_sockfd);
}

////////////////////////////////////////////////////////////////////////////////
// init
//

int 
udpRemoteClient::init( uint8_t nEncryption,
                        bool bSetBroadcast)
{
    m_nEncryption = nEncryption;
    m_bSetBroadcast = bSetBroadcast;

    if ( -1 == ( m_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) ) ) {
        return VSCP_ERROR_ERROR;
    }

    memset((char *) &m_clientAddress, 0, sizeof(m_clientAddress));
    m_clientAddress.sin_family = AF_INET;
    m_clientAddress.sin_port = htons(m_remotePort);

    if ( 0 == inet_aton( m_remoteAddress.c_str() , &m_clientAddress.sin_addr) )  {
        syslog(LOG_ERR, "UDP remote client: inet_aton() failed.");
        close(m_sockfd);
        return VSCP_ERROR_ERROR;
    }

    return VSCP_ERROR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
// startWorkerThread
//

bool udpRemoteClient::startWorkerThread(void) 
{
    if (pthread_create(&m_udpClientWorkerThread,
                       NULL,
                       UdpClientWorkerThread,
                       this)) {
        syslog(LOG_ERR,
               "Controlobject: Unable to start the UDP server thread.");
        return false;
    }



    return true;
}

////////////////////////////////////////////////////////////////////////////////
// sendFrames
//

int
udpRemoteClient::sendFrame(void)
{
    // We specify a send buffer that holds the maximum possible size
    unsigned char sendbuf[VSCP_MULTICAST_PACKET0_MAX]; // Send buffer
    uint8_t iv[16]; // Initialization vector

    // Check if there is an event to send
    if (!m_pClientItem->m_clientInputQueue.size()) {
        return false;
    }

    pthread_mutex_lock(&m_pClientItem->m_mutexClientInputQueue);
    vscpEvent *pEvent = m_pClientItem->m_clientInputQueue.front();
    m_pClientItem->m_clientInputQueue.pop_front();
    pthread_mutex_unlock(&m_pClientItem->m_mutexClientInputQueue);

    if (NULL == pEvent) return false;

    // Check that size is valid
    if (pEvent->sizeData > VSCP_LEVEL2_MAXDATA) {
        vscp_deleteEvent_v2(&pEvent);
        return false;
    }

    // Send the event to client
    //pthread_mutex_lock(&m_pCtrlObj->m_mutexUDPRemotes);

    //    for ( auto it = m_pCtrlObj->m_udpremotes.crbegin();
    //         it != m_pCtrlObj->m_udpremotes.crend();
    //         ++it) {

    //        udpRemoteClient *pRemoteUDPNode = *it;

           // Check if filtered out
        //    if (!vscp_doLevel2Filter(pEvent, &pRemoteUDPNode->m_filter)) {
        //        continue;
        //    }

    // Packet type
    sendbuf[VSCP_MULTICAST_PACKET0_POS_PKTTYPE] = SET_VSCP_MULTICAST_TYPE(0, m_nEncryption);

    // Get initialization vector
    getRandomIV(iv, 16);

    uint8_t wrkbuf[VSCP_MULTICAST_PACKET0_MAX];
    memset(wrkbuf, 0, sizeof(wrkbuf));

    if (!vscp_writeEventToFrame(wrkbuf,
                                sizeof(wrkbuf),
                                SET_VSCP_MULTICAST_TYPE(0, m_nEncryption),
                                pEvent)) {
        vscp_deleteEvent_v2(&pEvent);
        return VSCP_ERROR_OPERATION_FAILED;
    }

    // Write rolling index
    wrkbuf[VSCP_MULTICAST_PACKET0_POS_HEAD_LSB] &= 0xf8;
    wrkbuf[VSCP_MULTICAST_PACKET0_POS_HEAD_LSB] |= (0x07 & m_index);
    m_index++;

    size_t lenSend =
        1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + pEvent->sizeData + 2;
    if (0 == (lenSend = vscp_encryptFrame(sendbuf,
                                        wrkbuf,
                                        lenSend,
                                        m_pCtrlObj->getSystemKey(NULL),
                                        iv,
                                        m_nEncryption))) { 
        vscp_deleteEvent_v2(&pEvent); 
        return VSCP_ERROR_OPERATION_FAILED;
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
    
    // Send event to client
    int bytes_send = sendto(m_sockfd, 
                            sendbuf, lenSend, 0, 
                            (struct sockaddr *)&m_remoteAddress, sizeof(m_remoteAddress));

    return VSCP_ERROR_SUCCESS;
}

// ----------------------------------------------------------------------------

//                     * * * * UDPSrvObj * * * *

///////////////////////////////////////////////////////////////////////////////
// CTOR
//
//

UDPSrvObj::UDPSrvObj(CControlObject *pobj)
{
    m_bQuit       = false;
    
    // Default server information 
    m_servaddr.sin_family = AF_INET; // IPv4 
    m_servaddr.sin_addr.s_addr = INADDR_ANY; 
    m_servaddr.sin_port = htons(VSCP_DEFAULT_UDP_PORT); 
    
    m_pClientItem = NULL;
    setControlObjectPointer(pobj);
    pthread_mutex_init(&m_mutexUDPInfo, NULL);
}

///////////////////////////////////////////////////////////////////////////////
// DTOR
//

UDPSrvObj::~UDPSrvObj()
{
    pthread_mutex_destroy(&m_mutexUDPInfo);
}


////////////////////////////////////////////////////////////////////////////////
// receiveFrame
//

int
UDPSrvObj::receiveFrame(int sockfd,
                        CClientItem *pClientItem)
{
    uint8_t rcvbuf[VSCP_MULTICAST_PACKET0_MAX];
    vscpEvent *pEvent;
    int flags = MSG_DONTWAIT;
    struct sockaddr from;
    socklen_t addrlen = sizeof(from);

    // Check pointers
    if (NULL == pClientItem) return VSCP_ERROR_PARAMETER;

    memset(rcvbuf, 0, sizeof(rcvbuf));
    ssize_t rcvlen = recvfrom(sockfd, rcvbuf, sizeof(rcvbuf), flags, &from, &addrlen);
    
    // Null frame
    if ( 0 == rcvlen ) {
        // Packet to short
        syslog( LOG_ERR,
                "UDP receive server: The peer has performed an orderly shutdown, UDP frame have invalid length = %d",
                (int)rcvlen );
        if (m_bAck) {
            replyNackFrame(&from,
                           rcvbuf[VSCP_MULTICAST_PACKET0_POS_PKTTYPE],
                          (rcvbuf[VSCP_MULTICAST_PACKET0_POS_HEAD_LSB] & 0xf8));
        }
        return VSCP_ERROR_ERROR;
    }

    // Error
    if ( -1 == rcvlen ) {
        // Packet to short
        syslog( LOG_ERR,
                "UDP receive server: Error when receiving UDP frame = %d", errno );
        if (m_bAck) {
            replyNackFrame(&from,
                           rcvbuf[VSCP_MULTICAST_PACKET0_POS_PKTTYPE],
                          (rcvbuf[VSCP_MULTICAST_PACKET0_POS_HEAD_LSB] & 0xf8));
        }
        return VSCP_ERROR_ERROR;
    }

    // Incoming data
    // Must be at least a packet-type + header and a crc
    if (rcvlen < (1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2)) {

        // Packet to short
        syslog( LOG_ERR,
                "UDP receive server: UDP frame have invalid length = %d",
                (int)rcvlen );
        if (m_bAck) {
            replyNackFrame(&from,
                           rcvbuf[VSCP_MULTICAST_PACKET0_POS_PKTTYPE],
                          (rcvbuf[VSCP_MULTICAST_PACKET0_POS_HEAD_LSB] & 0xf8));
        }
        return VSCP_ERROR_ERROR;  
    }

    // If un-secure frames are not supported 
    // frames must be encrypted
    if (!m_bAllowUnsecure && !GET_VSCP_MULTICAST_PACKET_ENCRYPTION(rcvbuf[VSCP_MULTICAST_PACKET0_POS_PKTTYPE])) {
        syslog( LOG_ERR, "UDP receive server: UDP frame must be encrypted (or"
                            "m_bAllowUnsecure set to 'true') to be accepted."); 
        if (m_bAck) {
            replyNackFrame((struct sockaddr *)&from,
                           rcvbuf[VSCP_MULTICAST_PACKET0_POS_PKTTYPE],
                          (rcvbuf[VSCP_MULTICAST_PACKET0_POS_HEAD_LSB] & 0xf8));
        }
        return VSCP_ERROR_ERROR;
    }
        
    if (!vscp_decryptFrame( rcvbuf,
                            rcvbuf,
                            rcvlen, // actually len-16 but encrypt routine handle
                            m_pCtrlObj->getSystemKey(NULL), 
                            NULL,  // Will be copied from the last 16-bytes
                            GET_VSCP_MULTICAST_PACKET_ENCRYPTION(rcvbuf[0]))) {
        syslog( LOG_ERR, "UDP receive server: Decryption of UDP frame failed");
        if (m_bAck) {
            replyNackFrame((struct sockaddr *)&from,
                           rcvbuf[VSCP_MULTICAST_PACKET0_POS_PKTTYPE],
                          (rcvbuf[VSCP_MULTICAST_PACKET0_POS_HEAD_LSB] & 0xf8));
        }
        return VSCP_ERROR_ERROR;
    };

    if (m_bAck) {
        replyAckFrame((struct sockaddr *)&from,
                       rcvbuf[VSCP_MULTICAST_PACKET0_POS_PKTTYPE],
                      (rcvbuf[VSCP_MULTICAST_PACKET0_POS_HEAD_LSB] & 0xf8));
    }

    // Allocate a new event
    if (NULL == (pEvent = new vscpEvent)) return false;
    pEvent->pdata = NULL;

    if (!vscp_getEventFromFrame(pEvent, rcvbuf, rcvlen)) {
        vscp_deleteEvent_v2(&pEvent);
        return VSCP_ERROR_ERROR;
    }

    if (vscp_doLevel2Filter(pEvent, &m_pClientItem->m_filter)) {

        // Set obid to ourself so we don't get events we
        // receive
        pEvent->obid = pClientItem->m_clientID;

        // There must be room in the receive queue 
        if (m_pCtrlObj->m_maxItemsInClientReceiveQueue >
            m_pCtrlObj->m_clientOutputQueue.size()) {

            pthread_mutex_lock(&m_pCtrlObj->m_mutex_ClientOutputQueue);
            m_pCtrlObj->m_clientOutputQueue.push_back(pEvent);
            sem_post(&m_pCtrlObj->m_semClientOutputQueue);
            pthread_mutex_unlock(&m_pCtrlObj->m_mutex_ClientOutputQueue);

        } 
        else {
            vscp_deleteEvent_v2(&pEvent);
        }
    } 
    else {
        vscp_deleteEvent_v2(&pEvent);
    }

    return VSCP_ERROR_SUCCESS;
}



////////////////////////////////////////////////////////////////////////////////
// replyAckFrame
//

int
UDPSrvObj::replyAckFrame(struct sockaddr *to, uint8_t pkttype, uint8_t index)
{
    unsigned char sendbuf[VSCP_MULTICAST_PACKET0_MAX]; // Send buffer
    vscpEventEx ex;

    ex.head      = (index & 0xf8);
    ex.crc       = 0;
    ex.obid      = 0;
    ex.timestamp = vscp_makeTimeStamp();
    ex.year      = vscpdatetime::UTCNow().getYear();
    ex.month     = vscpdatetime::UTCNow().getMonth();
    ex.day       = vscpdatetime::UTCNow().getDay();
    ex.hour      = vscpdatetime::UTCNow().getHour();
    ex.minute    = vscpdatetime::UTCNow().getMinute();
    ex.second    = vscpdatetime::UTCNow().getSecond();
    memcpy(ex.GUID, m_pClientItem->m_guid.getGUID(), 16);
    ex.vscp_class = VSCP_CLASS1_ERROR;
    ex.vscp_type  = VSCP_TYPE_ERROR_SUCCESS;
    ex.sizeData   = 3;
    memset(ex.data, 0, 3); // index/zone/subzone = 0
    ex.data[0] = index;

    if (!vscp_writeEventExToFrame(sendbuf, sizeof(sendbuf), pkttype, &ex))
    { return false;
    }

    // Send to remote node
    size_t slen = sizeof(to);
    if ( -1 == sendto( m_sockfd,
                        sendbuf,
                        1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 + ex.sizeData,
                        0,
                        (struct sockaddr *)&to,
                        slen ) ) {
        ;
    }

    return VSCP_ERROR_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
// replyNackFrame
//

int
UDPSrvObj::replyNackFrame(struct sockaddr *to, uint8_t pkttype, uint8_t index)
{
    unsigned char sendbuf[VSCP_MULTICAST_PACKET0_MAX]; // Send buffer
    vscpEventEx ex;

    ex.head      = (index & 0xf8);
    ex.crc       = 0;
    ex.obid      = 0;
    ex.timestamp = vscp_makeTimeStamp();
    ex.year      = vscpdatetime::UTCNow().getYear();
    ex.month     = vscpdatetime::UTCNow().getMonth();
    ex.day       = vscpdatetime::UTCNow().getDay();
    ex.hour      = vscpdatetime::UTCNow().getHour();
    ex.minute    = vscpdatetime::UTCNow().getMinute();
    ex.second    = vscpdatetime::UTCNow().getSecond();
    memcpy(ex.GUID, m_pClientItem->m_guid.getGUID(), 16);
    ex.vscp_class = VSCP_CLASS1_ERROR;
    ex.vscp_type  = VSCP_TYPE_ERROR_ERROR;
    ex.sizeData   = 3;
    memset(ex.data, 0, 3); // index/zone/subzone = 0
    ex.data[0] = index;

    if (!vscp_writeEventExToFrame(sendbuf, sizeof(sendbuf), pkttype, &ex))
    { return false;
    }

    // Send to remote node
    size_t slen = sizeof(to);
    if ( -1 == sendto( m_sockfd,
                        sendbuf,
                        1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 + ex.sizeData,
                        0,
                        (struct sockaddr *)&to,
                        slen ) ) {
        ;
    }

    return VSCP_ERROR_SUCCESS;
}



///////////////////////////////////////////////////////////////////////////////
// UdpSrvWorkerThread
//

void *
UdpSrvWorkerThread(void *pData)
{
    int sockfd; 
    char rcvbuf[UDP_MAX_PACKAGE];
    struct sockaddr_in servaddr, cliaddr;

    UDPSrvObj *pObj = (UDPSrvObj *)pData;
    if (NULL == pObj) {
        syslog( LOG_ERR, "UDP RX Client: No thread worker object defined.");
        return NULL;
    }

    if (NULL == pObj->m_pCtrlObj) {
        syslog( LOG_ERR, "UDP RX Client: No control object defined.");
        return NULL;
    }

    // Init. CRC data
    crcInit();

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        syslog( LOG_ERR, "UDP RX Client: Socket could not be created.");
        return NULL;
    } 
      
    memset(&servaddr, 0, sizeof(servaddr));
    

    // Bind the socket with the server address 
    if ( bind(sockfd, 
                (const struct sockaddr *)&pObj->m_servaddr,
                sizeof(pObj->m_servaddr)) < 0 )
    { 
        syslog( LOG_ERR, "UDP RX Client: Unable to bind to server port."); 
        close(sockfd);
        return NULL; 
    } 

    syslog( LOG_ERR, "UDP RX Client: Bind to interface. [%s]",
                        get_ip_str((struct sockaddr *)&pObj->m_servaddr,
                                    rcvbuf,sizeof(rcvbuf)) );
      
    

    // We need to create a client object and add this object to the list of clients
    pObj->m_pClientItem = new CClientItem;
    if (NULL == pObj->m_pClientItem) {
        syslog( LOG_ERR,
            "[UDP RX Client] Unable to allocate memory for client - terminating."); 
        close(sockfd);    
        return NULL;
    }

    // This is now an active Client
    pObj->m_pClientItem->m_bOpen         = true;
    pObj->m_pClientItem->m_type          = CLIENT_ITEM_INTERFACE_TYPE_CLIENT_UDP;
    pObj->m_pClientItem->m_strDeviceName = "Internal UDP RX Client listener. [";
    pObj->m_pClientItem->m_strDeviceName += get_ip_str((struct sockaddr *)&pObj->m_servaddr,rcvbuf,sizeof(rcvbuf));
    pObj->m_pClientItem->m_strDeviceName += "]|Started at ";
    pObj->m_pClientItem->m_strDeviceName += vscpdatetime::Now().getISODateTime();

    // If a user is defined get user item
    vscp_trim(pObj->m_user);
    if (pObj->m_user.length()) {

        pthread_mutex_lock(&pObj->m_pCtrlObj->m_mutex_UserList);
        pObj->m_pClientItem->m_pUserItem = 
            pObj->m_pCtrlObj->m_userList.getUser(pObj->m_user);
        pthread_mutex_unlock(&pObj->m_pCtrlObj->m_mutex_UserList);

        if (NULL == pObj->m_pClientItem->m_pUserItem) {

            delete pObj->m_pClientItem;

            syslog( LOG_ERR,
                "[UDP RX Client] User [%s] NOT allowed to connect.",
                (const char *)pObj->m_user.c_str());
            close(sockfd);    
            return NULL;
        }
    } else {
        pObj->m_pClientItem->m_pUserItem = NULL; // No user defined
    }

    // Add the client to the Client List
    pthread_mutex_lock(&pObj->m_pCtrlObj->m_mutex_clientList);
    if (!pObj->m_pCtrlObj->addClient(pObj->m_pClientItem, 
                                        CLIENT_ITEM_INTERFACE_TYPE_CLIENT_UDP)) {
        // Failed to add client
        delete pObj->m_pClientItem;
        pObj->m_pClientItem = NULL;
        pthread_mutex_lock(&pObj->m_pCtrlObj->m_mutex_clientList);
        syslog( LOG_ERR,
            "UDP server: Failed to add client. Terminating thread.");
        close(sockfd);    
        return NULL;
    }
    pthread_mutex_unlock(&pObj->m_pCtrlObj->m_mutex_clientList);

    // Set receive filter
    memcpy(&pObj->m_pClientItem->m_filter,
            &pObj->m_filter,
            sizeof(vscpEventFilter));

    // Set GUID for channel
    if (!pObj->m_guid.isNULL()) {
        pObj->m_pClientItem->m_guid = pObj->m_guid;
    }

    syslog( LOG_ERR, "UDP RX Client: Thread started.");

    struct pollfd poll_set;
    poll_set.events = POLLIN;
    poll_set.fd = sockfd;
    poll_set.revents = 0;

    int poll_ret;
    while (!pObj->m_bQuit) {

        poll_ret = poll(&poll_set, 1, 500);
        if (poll_ret > 0) {            

            // OK receive the frame
            if (pObj->receiveFrame(sockfd, pObj->m_pClientItem)) {                
                ;
            }
            else {
                ;
            }

        }

        //pObj->sendFrames(sockfd, pObj->m_pClientItem);
    } // while

    if (NULL != pObj->m_pClientItem) {
        // Add the client to the Client List
        pthread_mutex_lock(&pObj->m_pCtrlObj->m_mutex_clientList);
        pObj->m_pCtrlObj->removeClient(pObj->m_pClientItem);
        pthread_mutex_unlock(&pObj->m_pCtrlObj->m_mutex_clientList);
    }

    syslog( LOG_ERR, "UDP RX ClientThread: Quit.");
    
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// UspClientWorkerThread
//

void *
UdpClientWorkerThread(void *pData)
{
    char rcvbuf[UDP_MAX_PACKAGE];

    // Init. CRC data
    crcInit();

    udpRemoteClient *pObj = (udpRemoteClient *)pData;
    if (NULL == pObj) {
        syslog( LOG_ERR, "UDP TX Client: No thread worker object defined.");
        return NULL;
    }

    if (NULL == pObj->m_pCtrlObj) {
        syslog( LOG_ERR, "UDP TX Client: No control object defined.");
        return NULL;
    }

    // // Creating socket file descriptor 
    // if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
    //     syslog( LOG_ERR,"UDP TX Client: socket creation failed"); 
    //     exit(EXIT_FAILURE); 
    // } 

    // memset(&client_addr, 0, sizeof(client_addr));
    // client_addr.sin_family = AF_INET;
    // client_addr.sin_addr.s_addr  = inet_addr(pObj->m_interface.c_str());
    // //servaddr.sin_addr.s_addr = INADDR_ANY;    // Broadcast
    // client_addr.sin_port   = htons(1024);


    // We need to create a client object and add this object to the list of clients
    pObj->m_pClientItem = new CClientItem;
    if (NULL == pObj->m_pClientItem) {
        syslog( LOG_ERR,
            "[UDP RX Client] Unable to allocate memory for client - terminating."); 
        close(pObj->m_sockfd);    
        return NULL;
    }

    // This is now an active Client
    pObj->m_pClientItem->m_bOpen         = true;
    pObj->m_pClientItem->m_type          = CLIENT_ITEM_INTERFACE_TYPE_CLIENT_UDP;
    pObj->m_pClientItem->m_strDeviceName = ("Internal UDP RX Client listener. [");
    pObj->m_pClientItem->m_strDeviceName += get_ip_str((struct sockaddr *)&pObj->m_remoteAddress,rcvbuf,sizeof(rcvbuf));
    pObj->m_pClientItem->m_strDeviceName += ("]|Started at ");
    pObj->m_pClientItem->m_strDeviceName +=
    vscpdatetime::Now().getISODateTime();

    // If a user is defined get user item
    vscp_trim(pObj->m_user);
    if (pObj->m_user.length()) {
        pthread_mutex_lock(&pObj->m_pCtrlObj->m_mutex_clientList);
        pObj->m_pClientItem->m_pUserItem = 
            pObj->m_pCtrlObj->m_userList.validateUser( pObj->m_user, 
                                                        pObj->m_password);
        pthread_mutex_lock(&pObj->m_pCtrlObj->m_mutex_UserList);

        if (NULL == pObj->m_pClientItem->m_pUserItem) {

            delete pObj->m_pClientItem;

            syslog( LOG_ERR,
                "[UDP RX Client] User [%s] NOT allowed to connect.",
                (const char *)pObj->m_user.c_str());
            close(pObj->m_sockfd);    
            return NULL;
        }
    } 
    else {
        pObj->m_pClientItem->m_pUserItem = NULL; // No user defined
    }

    // Add the client to the Client List
    pthread_mutex_lock(&pObj->m_pCtrlObj->m_mutex_clientList);
    if (!pObj->m_pCtrlObj->addClient(pObj->m_pClientItem, 
                                        CLIENT_ITEM_INTERFACE_TYPE_CLIENT_UDP)) {
        // Failed to add client
        delete pObj->m_pClientItem;
        pObj->m_pClientItem = NULL;
        pthread_mutex_lock(&pObj->m_pCtrlObj->m_mutex_clientList);
        syslog( LOG_ERR,
            "UDP server: Failed to add client. Terminating thread.");
        close(pObj->m_sockfd);    
        return NULL;
    }
    pthread_mutex_lock(&pObj->m_pCtrlObj->m_mutex_clientList);

    // Set receive filter
    memcpy(&pObj->m_pClientItem->m_filter,
            &pObj->m_filter,
            sizeof(vscpEventFilter));

    // Set GUID for channel
    if (!pObj->m_guid.isNULL()) {
        pObj->m_pClientItem->m_guid = pObj->m_guid;
    }

    syslog( LOG_ERR, "UDP RX Client: Thread started.");
    
    // Run run run...
    while (!pObj->m_bQuit) {

        // pObj->sendFrame();

    }

    if (NULL != pObj->m_pClientItem) {
        // Add the client to the Client List
        pthread_mutex_lock(&pObj->m_pCtrlObj->m_mutex_clientList);
        pObj->m_pCtrlObj->removeClient(pObj->m_pClientItem);
        pthread_mutex_unlock(&pObj->m_pCtrlObj->m_mutex_clientList);
    }

    syslog( LOG_ERR, "UDP RX ClientThread: Quit.");
    
    return NULL;
}

