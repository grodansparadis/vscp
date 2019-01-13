// File: vscpmulticast.cpp
//
// Receiving multicast frames (VSCP Works)
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

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "vscpworks.h"
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/listimpl.cpp>
#include <wx/socket.h>

#ifdef WIN32

#else

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#endif

#include "vscpmulticast.h"
#include <canal_macro.h>
#include <crc.h>
#include <vscp.h>

// Part of VSCP Works

///////////////////////////////////////////////////////////////////////////////
// worksMulticastThread
//

worksMulticastThread::worksMulticastThread()
  : wxThread(wxTHREAD_JOINABLE)
{
    // m_pApp = NULL;
    m_bQuit = false;
}

worksMulticastThread::~worksMulticastThread()
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
// Entry
//

void *
worksMulticastThread::Entry()
{
    crcInit();

    //*************************************************************************
    //                           Multicast init.
    //*************************************************************************

    int sock;                   // socket descriptor
    struct sockaddr_in mc_addr; // socket address structure
    unsigned short port = VSCP_DEFAULT_MULTICAST_PORT; // multicast port
    unsigned char ttl   = 1; // time to live (hop count)
    int flag_on         = 1; // socket option flag

#ifdef WIN32

    // create a socket for sending to the multicast address
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("Multicast socket() failed");
        return NULL;
    }

    // set reuse port to on to allow multiple binds per host
    if ((setsockopt(
          sock, SOL_SOCKET, SO_REUSEADDR, (char *)&flag_on, sizeof(flag_on))) <
        0) {
        perror("Multicast setsockopt() failed");
        closesocket(sock);
        return NULL;
    }

    // set the TTL (time to live/hop count) for the send
    /*if ( ( setsockopt( sock, IPPROTO_IP, IP_MULTICAST_TTL,
                       ( const char* )&ttl, sizeof( ttl ) ) ) < 0 ) {
        perror( "Multicast setsockopt() failed" );
        return NULL;
    }*/

    // construct a multicast address structure
    memset(&mc_addr, 0, sizeof(mc_addr));
    mc_addr.sin_family      = AF_INET;
    mc_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    mc_addr.sin_port        = htons(port);

    // bind to multicast address to socket
    if ((bind(sock, (struct sockaddr *)&mc_addr, sizeof(mc_addr))) < 0) {
        perror("Multicast bind() failed");
        closesocket(sock);
        return NULL;
    }

    // construct an IGMP join request structure
    struct ip_mreq mc_req; // multicast request structure
    mc_req.imr_multiaddr.s_addr = inet_addr(VSCP_MULTICAST_IPV4_ADDRESS_STR);
    mc_req.imr_interface.s_addr = htonl(INADDR_ANY);

    // send an ADD MEMBERSHIP message via setsockopt
    if ((setsockopt(sock,
                    IPPROTO_IP,
                    IP_ADD_MEMBERSHIP,
                    (char *)&mc_req,
                    sizeof(mc_req))) < 0) {
        perror("Multicast setsockopt() failed");
        closesocket(sock);
        return NULL;
    }

#else

    // create a socket for sending to the multicast address
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket() failed");
        return NULL;
    }

    // set the TTL (time to live/hop count) for the send
    if ((setsockopt(
          sock, IPPROTO_IP, IP_MULTICAST_TTL, (void *)&ttl, sizeof(ttl))) < 0) {
        perror("setsockopt() failed");
        close(sock);
        return NULL;
    }

    // construct a multicast address structure
    memset(&mc_addr, 0, sizeof(mc_addr));
    mc_addr.sin_family      = AF_INET;
    mc_addr.sin_addr.s_addr = inet_addr(VSCP_MULTICAST_IPV4_ADDRESS_STR);
    mc_addr.sin_port        = htons(port);

    // bind to multicast address to socket
    if ((bind(sock, (struct sockaddr *)&mc_addr, sizeof(mc_addr))) < 0) {
        perror("Multicast bind() failed");
        close(sock);
        return NULL;
    }

    // construct an IGMP join request structure
    struct ip_mreq mc_req; // multicast request structure
    mc_req.imr_multiaddr.s_addr = inet_addr(VSCP_MULTICAST_IPV4_ADDRESS_STR);
    mc_req.imr_interface.s_addr = htonl(INADDR_ANY);

    // send an ADD MEMBERSHIP message via setsockopt
    if ((setsockopt(sock,
                    IPPROTO_IP,
                    IP_ADD_MEMBERSHIP,
                    (void *)&mc_req,
                    sizeof(mc_req))) < 0) {
        perror("setsockopt() failed");
        close(sock);
        exit(1);
    }

#endif

    fd_set fds;
    struct timeval tv;

    // Go to work

    char buf[1024];               // buffer to receive string
    int recv_len;                 // length of string received
    struct sockaddr_in from_addr; // packet source
#ifdef WIN32
    int from_len; // source addr length
#else
    socklen_t from_len; // source addr length
#endif
    int nRcv;

    while (!m_bQuit && !TestDestroy()) {

        // Set up the file descriptor set.
        FD_ZERO(&fds);
        FD_SET(sock, &fds);

        // Set up the struct timeval for the timeout.
        tv.tv_sec  = 2;
        tv.tv_usec = 0;

        // Wait until timeout or data received.
        nRcv = select(sock + 1, &fds, NULL, NULL, &tv);
        if (nRcv == 0) {
            // Timeout - Just continue
            continue;
        } else if (nRcv == -1) {
            // Error
#ifdef WIN32
            // nRcv = WSAGetLastError();
#endif
            wxSleep(1); // Protects from exhausing system if error takes over
            continue;
        }

#ifdef WIN32

        // clear the receive buffers & structs
        memset(buf, 0, sizeof(buf));
        from_len = sizeof(from_addr);
        memset(&from_addr, 0, from_len);

        if ((recv_len = recvfrom(sock,
                                 buf,
                                 sizeof(buf),
                                 0,
                                 (struct sockaddr *)&from_addr,
                                 &from_len)) < 0) {
            // nRcv = WSAGetLastError();
        }

#else
        if ((recv_len = recvfrom(sock,
                                 buf,
                                 sizeof(buf),
                                 MSG_DONTWAIT,
                                 (struct sockaddr *)&from_addr,
                                 &from_len)) < 0) {
        }
#endif

        std::string originatingAddress;

#ifdef WIN32
        char *s                     = NULL;
        struct sockaddr_in *addr_in = (struct sockaddr_in *)&from_addr;
        s                           = inet_ntoa(addr_in->sin_addr);
        // TODO
        /*switch ( from_addr.sin_family ) {
        case AF_INET: {
        struct sockaddr_in *addr_in = ( struct sockaddr_in * )&from_addr;
        s = (char *)malloc( 128 );
        InetNtop( AF_INET, &( addr_in->sin_addr ), s, 128 );
        break;
        }
        case AF_INET6: {
        struct sockaddr_in6 *addr_in6 = ( struct sockaddr_in6 * )&from_addr;
        s = (char *)malloc( 128 );
        InetNtop( AF_INET6, &( addr_in6->sin6_addr ), s, 128 );
        break;
        }
        default:
        break;
        }*/

        originatingAddress = std::string::FromAscii(s);
        // free( s );
#else
        char *s = NULL;
        switch (from_addr.sin_family) {
            case AF_INET: {
                struct sockaddr_in *addr_in = (struct sockaddr_in *)&from_addr;
                s                           = (char *)malloc(INET_ADDRSTRLEN);
                inet_ntop(AF_INET, &(addr_in->sin_addr), s, INET_ADDRSTRLEN);
                break;
            }
            case AF_INET6: {
                struct sockaddr_in6 *addr_in6 =
                  (struct sockaddr_in6 *)&from_addr;
                s = (char *)malloc(INET6_ADDRSTRLEN);
                inet_ntop(
                  AF_INET6, &(addr_in6->sin6_addr), s, INET6_ADDRSTRLEN);
                break;
            }
            default:
                break;
        }
        originatingAddress.FromAscii(s);
        free(s);
#endif

        // We have data
        crc chksum = crcFast((unsigned const char *)buf, recv_len);
        chksum     = wxUINT32_SWAP_ON_LE(chksum);

        if ((0 == chksum) && (VSCP_MULTICAST_TYPE_EVENT ==
                              GET_VSCP_MULTICAST_PACKET_TYPE(buf[0]))) {

            uint16_t vscp_class =
              (((uint16_t)buf[VSCP_MULTICAST_PACKET0_POS_VSCP_CLASS]) << 8) +
              buf[VSCP_MULTICAST_PACKET0_POS_VSCP_CLASS + 1];
            vscp_class = wxUINT32_SWAP_ON_LE(vscp_class);

            uint16_t vscp_type =
              (((uint16_t)buf[VSCP_MULTICAST_PACKET0_POS_VSCP_TYPE]) << 8) +
              buf[VSCP_MULTICAST_PACKET0_POS_VSCP_TYPE + 1];
            vscp_type = wxUINT32_SWAP_ON_LE(vscp_type);

            // Level I node heart beat
            if ((VSCP_CLASS1_INFORMATION == vscp_class) &&
                (VSCP_TYPE_INFORMATION_NODE_HEARTBEAT == vscp_type)) {

                // * * * Should not see this event here normally * * *
                // This event will use the interface GUID as its originator

            }
            // Level II node Heart beat
            else if ((VSCP_CLASS2_INFORMATION == vscp_class) &&
                     (VSCP2_TYPE_INFORMATION_HEART_BEAT == vscp_type)) {

                // This will use the correct GUID as originator
                cguid guid;
                /* TODO
                                guid.getFromArray( ( unsigned const char * )buf
                   + VSCP_MULTICAST_PACKET0_POS_VSCP_GUID  ); CNodeInformation
                   *pNode = m_knownNodes.addNode( guid ); if ( NULL != pNode ) {

                                    // not a proxy
                                    pNode->m_bProxy = false;

                                    // Save the nodes GUID
                                    pNode->m_realguid = guid;

                                    uint16_t size = ( buf[
                   VSCP_MULTICAST_PACKET0_POS_VSCP_SIZE  ] << 8 ) + buf[
                   VSCP_MULTICAST_PACKET0_POS_VSCP_SIZE +1 ]; size =
                   wxUINT32_SWAP_ON_LE( size );

                                    // Update heart beat
                                    pNode->m_lastHeartBeat = wxDateTime::Now();

                                    // If a node name supplied. save it
                                    if ( size ) {

                                        // Name can be 64 byte max. We must make
                   sure it is null terminated unsigned char wrkbuf[ 65 ];
                                        memset( wrkbuf, 0, sizeof( wrkbuf ) );
                                        memcpy( wrkbuf, ( unsigned const char *
                   )buf + VSCP_MULTICAST_PACKET0_POS_VSCP_DATA, MIN( size, 64 )
                   ); pNode->m_strNodeName = std::string::FromUTF8( (const char
                   *)wrkbuf );

                                    }

                                    // Save the servers address

                                    pNode->m_address = originatingAddress;

                                }
                */
            }
            // Server proxy heart beats
            else if ((VSCP_CLASS2_INFORMATION == vscp_class) &&
                     (VSCP2_TYPE_INFORMATION_PROXY_HEART_BEAT == vscp_type)) {

                // This will use the correct GUID as originator
                /* TODO
                                // GUID is sending nodes/servers GUID
                                // For a Level I node connected to a server this
                   is the GUID for the server. cguid guid; guid.getFromArray( (
                   unsigned const char * )buf +
                   VSCP_MULTICAST_PACKET0_POS_VSCP_GUID );

                                // Interface GUID is the GUID the node uses.
                   This is not the real GUID of a node
                                // but instead a GUID constructed from the
                   interface the node is on. But this is the
                                // GUID the node is identified as here so it is
                   used as identifier for the node. cguid ifguid;
                                ifguid.getFromArray( ( unsigned const char *
                   )buf + VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 32 );

                                CNodeInformation *pNode = m_knownNodes.addNode(
                   ifguid ); if ( NULL != pNode ) {

                                    // Not a proxy
                                    pNode->m_bProxy = true;

                                    // Save the nodes real GUID
                                    cguid realguid;
                                    realguid.getFromArray( ( unsigned const char
                   * )buf + VSCP_MULTICAST_PACKET0_POS_VSCP_DATA ); if (
                   !realguid.isNULL() ) { pNode->m_realguid = guid;
                                    }

                                    // Save the nodes interface GUID
                                    if ( !ifguid.isNULL() ) {
                                        pNode->m_interfaceguid = ifguid;
                                    }

                                    // Time for last heatbeat
                                    pNode->m_lastHeartBeat = wxDateTime::Now();

                                    // Name can be 64 byte max. We must make
                   sure it is null terminates unsigned char wrkbuf[ 65 ];
                                    memset( wrkbuf, 0, sizeof( wrkbuf ) );
                                    memcpy( wrkbuf, ( unsigned const char * )buf
                   + VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 64, 64  );
                                    pNode->m_strNodeName =
                   std::string::FromUTF8( ( const char * )wrkbuf );

                                    // If the node does not have a name give it
                   one if ( 0 == pNode->m_strNodeName.Length() ) { std::string
                   wxstr; guid.toString( wxstr ); pNode->m_strNodeName = _("Node
                   without name @ "); pNode->m_strNodeName += wxstr;
                                        pNode->m_strNodeName += _(" IP=");
                                        pNode->m_strNodeName +=
                   originatingAddress;
                                    }

                                    // Save name for interface this node is on

                                    // Save the servers address
                                    pNode->m_address = originatingAddress;

                                }
                  */
            }
            // Server capabilities
            else if ((VSCP_CLASS2_PROTOCOL == vscp_class) &&
                     (VSCP2_TYPE_PROTOCOL_HIGH_END_SERVER_CAPS == vscp_type)) {

                // This will use the server GUID as originator
                // This will use the correct GUID as originator
                cguid guid;

                guid.getFromArray((unsigned const char *)buf +
                                  VSCP_MULTICAST_PACKET0_POS_VSCP_GUID);
                /*CVSCPServerInformation *pServer = m_knownNodes.addServer( guid
                ); if ( NULL != pServer ) {

                }  TODO */
            }
        }

    } // while

#ifdef WIN32
    // send a DROP MEMBERSHIP message via setsockopt
    if ((setsockopt(sock,
                    IPPROTO_IP,
                    IP_DROP_MEMBERSHIP,
                    (const char *)&mc_req,
                    sizeof(mc_req))) < 0) {
        perror("Multicast setsockopt() failed");
    }

    // Close the multicast socket
    closesocket(sock);
#else
    close(sock);
#endif

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void
worksMulticastThread::OnExit()
{}