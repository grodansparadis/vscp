// File: vscpmulticast.cpp
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (c) 2000-2015 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
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


#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "vscpworks.h"
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include <wx/socket.h>
#include <wx/listimpl.cpp>

#include <crc.h>
#include "vscp.h"
#include "vscpmulticast.h"

///////////////////////////////////////////////////////////////////////////////
// worksMulticastThread
//

worksMulticastThread::worksMulticastThread()
    : wxThread( wxTHREAD_JOINABLE )
{
    //m_pApp = NULL;
    m_bQuit = false;
}


worksMulticastThread::~worksMulticastThread()
{
    ;
}


///////////////////////////////////////////////////////////////////////////////
// Entry
//

void *worksMulticastThread::Entry()
{
    // Must have a valid pointer to the app. object
    //if ( NULL == m_pApp ) return NULL;

    crcInit();

    //*************************************************************************
    //                           Multicast init.
    //*************************************************************************

    int sock;                           // socket descriptor 
    struct ip_mreq mc_req;              // multicast request structure 
    struct sockaddr_in mc_addr;         // socket address structure  
    unsigned short port = 33333;        // multicast port 
    unsigned char ttl = 1;              // time to live (hop count) 
    int flag_on = 1;                    // socket option flag 

#ifdef WIN32

    WSADATA wsaData;                    // Windows socket DLL structure 

    // Load Winsock 2.0 DLL
    if ( WSAStartup( MAKEWORD( 2, 0 ), &wsaData ) != 0 ) {
        fprintf( stderr, "Multicast WSAStartup() failed" );
        return NULL;
    }

    // create a socket for sending to the multicast address 
    if ( ( sock = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) < 0 ) {
        perror( "Multicast socket() failed" );
        return NULL;
    }

    // set reuse port to on to allow multiple binds per host 
    if ( ( setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, ( char* )&flag_on,
                       sizeof( flag_on ) ) ) < 0 ) {
        perror( "Multicast setsockopt() failed" );
        return NULL;
    }

    // set the TTL (time to live/hop count) for the send 
    if ( ( setsockopt( sock, IPPROTO_IP, IP_MULTICAST_TTL,
                       ( const char* )&ttl, sizeof( ttl ) ) ) < 0 ) {
        perror( "Multicast setsockopt() failed" );
        return NULL;
    }

    // construct a multicast address structure 
    memset( &mc_addr, 0, sizeof( mc_addr ) );
    mc_addr.sin_family = AF_INET;
    mc_addr.sin_addr.s_addr = htonl( INADDR_ANY );
    mc_addr.sin_port = htons( port );

    // bind to multicast address to socket 
    if ( ( bind( sock, ( struct sockaddr * ) &mc_addr,
                 sizeof( mc_addr ) ) ) < 0 ) {
        perror( "Multicast bind() failed" );
        return NULL;
    }

    // construct an IGMP join request structure 
    mc_req.imr_multiaddr.s_addr = inet_addr( VSCP_MULTICAST_IPV4_ADDRESS_STR );
    mc_req.imr_interface.s_addr = htonl( INADDR_ANY );

    // send an ADD MEMBERSHIP message via setsockopt 
    if ( ( setsockopt( sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                       ( char* )&mc_req, sizeof( mc_req ) ) ) < 0 ) {
        perror( "Multicast setsockopt() failed" );
        return NULL;
    }

#else

    // create a socket for sending to the multicast address 
    if ( ( sock_mc = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) < 0 ) {
        perror( "socket() failed" );
        return NULL;
    }

    // set the TTL (time to live/hop count) for the send 
    if ( ( setsockopt( sock_mc, IPPROTO_IP, IP_MULTICAST_TTL,
                       ( void* )&mc_ttl, sizeof( mc_ttl ) ) ) < 0 ) {
        perror( "setsockopt() failed" );
        return NULL;
    }

    // construct a multicast address structure 
    memset( &mc_addr, 0, sizeof( mc_addr ) );
    mc_addr.sin_family = AF_INET;
    mc_addr.sin_addr.s_addr = inet_addr( "224.0.23.158" );
    mc_addr.sin_port = htons( mc_port );

#endif

    fd_set fds;
    struct timeval tv;

    // Go to work

    char buf[ 1024 ];               // buffer to receive string 
    int recv_len;                   // length of string received 
    struct sockaddr_in from_addr;   // packet source 
    int from_len;                   // source addr length
    int nRcv;

    while ( !m_bQuit && !TestDestroy() ) {

        // Set up the file descriptor set.
        FD_ZERO( &fds );
        FD_SET( sock, &fds );

        // Set up the struct timeval for the timeout.
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        // Wait until timeout or data received.
        nRcv = select( sock + 1, &fds, NULL, NULL, &tv );
        if ( nRcv == 0 ) {
            // Timeout - Just continue
            continue;   
        }
        else if ( nRcv == -1 ) {
            // Error  
            nRcv = WSAGetLastError();
            wxSleep( 1 );   // Protects from exhausing system if error takes over
            continue;
        }

#ifdef WIN32

        // clear the receive buffers & structs 
        memset( buf, 0, sizeof( buf ) );
        from_len = sizeof( from_addr );
        memset( &from_addr, 0, from_len );

        if ( ( recv_len = recvfrom( sock,
                                    buf, sizeof( buf ), 0,
                                    ( struct sockaddr* )&from_addr, &from_len ) ) < 0 ) {
            nRcv = WSAGetLastError();
        }

#else
        if ( ( recv_len = recvfrom( m_sock_mc, recv_str, 1024, MSG_DONTWAIT,
                                    ( struct sockaddr* )&from_addr, &from_len ) ) < 0 ) {

        }
#endif

        // We have data
        crc chksum = crcFast( (unsigned const char *)buf, recv_len  );
        wxUINT32_SWAP_ON_LE( chksum );

        if ( ( 0 == chksum )  && ( VSCP_MULTICAST_TYPE_EVENT  == GET_VSCP_MULTICAST_PACKET_TYPE( buf[ 0 ] ) ) ) {

            uint16_t vscp_class = ( ((uint16_t)buf[ VSCP_MULTICAST_PACKET0_POS_VSCP_CLASS ]) << 8 ) + buf[ VSCP_MULTICAST_PACKET0_POS_VSCP_CLASS + 1 ];
            wxUINT32_SWAP_ON_LE( vscp_class );

            uint16_t vscp_type = ( ( ( uint16_t )buf[ VSCP_MULTICAST_PACKET0_POS_VSCP_TYPE ] ) << 8 ) + buf[ VSCP_MULTICAST_PACKET0_POS_VSCP_TYPE + 1 ];
            wxUINT32_SWAP_ON_LE( vscp_type );

            if ( ( VSCP_CLASS2_INFORMATION  == vscp_class ) && ( VSCP2_TYPE_INFORMATION_PROXY_HEART_BEAT == vscp_type ) ) {

            }
        }
        

    }


    // send a DROP MEMBERSHIP message via setsockopt 
    if ( ( setsockopt( sock, IPPROTO_IP, IP_DROP_MEMBERSHIP,
                       ( const char * )&mc_req, sizeof( mc_req ) ) ) < 0 ) {
        perror( "Multicast setsockopt() failed" );
    }

    // Close the multicast socket
    closesocket( sock );

#ifdef WIN32
    // Cleanup Winsock
    WSACleanup();
#endif

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void worksMulticastThread::OnExit()
{

}