// udpthread.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (http://www.vscp.org)
//
// Copyright (C) 2000-2012 Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
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
// http://tangentsoft.net/wskfaq/examples/ipaddr.html
// http://code.activestate.com/recipes/439094/
// http://mail.nl.linux.org/kernelnewbies/2003-05/msg00090.html
//

#define _POSIX

#ifdef WIN32

#include "Ws2tcpip.h"

#else

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <syslog.h>
#include <sys/msg.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <string.h>
#include <syslog.h>
#include <netdb.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <net/if_arp.h>
#include <arpa/inet.h>

#define inaddrr(x) (*(struct in_addr *) &ifr->x[sizeof sa.sin_port])
#define IFRSIZE   ((int)(size * sizeof (struct ifreq)))

#endif

#ifdef __GNUG__
//#pragma implementation
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef __WXMSW__
#include  "wx/ownerdrw.h"
#endif

// Common includes
#include "version.h"
#include "canal_macro.h"
#include "../../common/crc.h"
#include "vscp.h"
#include "vscphelper.h"
#include "clientlist.h"
#include "controlobject.h"
#include "udpthread.h"

///////////////////////////////////////////////////////////////////////////////
// UDPSendThread
//
// This thread listens for connection on a TCP socket and starts a new thread
// to handle client requests
//


UDPSendThread::UDPSendThread()
		: wxThread( wxTHREAD_JOINABLE )
{
    m_bQuit = false;
    m_pCtrlObject = NULL;
}


UDPSendThread::~UDPSendThread()
{
	;
}


///////////////////////////////////////////////////////////////////////////////
// Entry
//

void *UDPSendThread::Entry()
{
    // Must have a valid pointer to the control object
    if ( NULL == m_pCtrlObject ) return NULL;

    // We need to create a clientobject and add this object to the list
    CClientItem *pClientItem = new CClientItem;
    if ( NULL == pClientItem ) return NULL;

    // This is an active client
    pClientItem->m_bOpen = true;
    pClientItem->m_type = CLIENT_ITEM_INTERFACE_TYPE_CLIENT_INTERNAL;
    pClientItem->m_strDeviceName = _("Internal UDP Transmit Client. Started at ");
    wxDateTime now = wxDateTime::Now();
    pClientItem->m_strDeviceName += now.FormatISODate();
    pClientItem->m_strDeviceName += _(" ");
    pClientItem->m_strDeviceName += now.FormatISOTime();

    // Add the client to the Client List
    m_pCtrlObject->m_wxClientMutex.Lock();
    m_pCtrlObject->addClient( pClientItem/*, CLIENT_ITEM_SPECIAL_ID_UDP*/ );
    m_pCtrlObject->m_wxClientMutex.Unlock();

    // Create the socket for Level II UDP datagrams
    sockaddr_in addr;
    addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl ( INADDR_ANY );
	addr.sin_port = htons ( 0 );

	// Using the INADDR_ANY interface datagrams will be sent on the first found interface
	// on a machine with multiple interfaces. If the ifaddress is set to another valid
	// address that interface will be used instead.
	if ( 0 != m_pCtrlObject->m_strUdpInterfaceAddress.Length() ) {
#if ( WIN32 && (_WIN32_WINNT<0x0600) ) 
		addr.sin_addr.s_addr = 
			inet_addr( m_pCtrlObject->m_strUdpBindInterfaceAddress.ToAscii() );
#else
		inet_pton( AF_INET, m_pCtrlObject->m_strUdpInterfaceAddress.ToAscii(), (void *)&addr.sin_addr.s_addr );
#endif
	}

    if ( -1 != ( m_sendsock = socket ( AF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) ) {

        int on = 1;
        setsockopt ( m_sendsock,
                        SOL_SOCKET,
                        SO_BROADCAST,
                        ( const char* ) &on,
                        sizeof ( on ) );

        on = 1;     // As suggested by Charles Tewiah
        setsockopt ( m_sendsock,
                        SOL_SOCKET,
                        SO_REUSEADDR,
                        ( const char * ) &on,
                        sizeof ( on ) );

        if ( 1 == bind ( m_sendsock, ( struct sockaddr* ) &addr, sizeof ( addr ) ) )
        {
            m_pCtrlObject->logMsg( _("vscpd: Faild to bind send socket. Functionality disabled."), DAEMON_LOGMSG_ERROR );
#ifdef WIN32
            if ( NULL != m_sendsock ) closesocket ( m_sendsock );
            m_sendsock = NULL;
#else
            if ( 0 != m_sendsock ) close ( m_sendsock );
            m_sendsock = 0;
#endif

        }

    }
    else
    {
        m_pCtrlObject->logMsg ( _("vscpd: Faild to create send socket. Functionality disabled."), DAEMON_LOGMSG_ERROR );
#ifdef WIN32
        m_sendsock = NULL;
#else
        m_sendsock = 0;
#endif
    }

    char szName[ 128 ];
#ifdef WIN32
    LPHOSTENT lpLocalHostEntry;
#else
    struct hostent *lpLocalHostEntry;
#endif
    gethostname ( szName, sizeof ( szName ) );
    lpLocalHostEntry = gethostbyname ( szName );
    if ( NULL == lpLocalHostEntry ) 
    {
        return NULL;
    }

    // Get all local addresses for interface
    int cntAddr = -1;
    void *pAddr;
    unsigned long localaddr[ 16 ]; // max 16 local addresses
    do {
        cntAddr++;
        localaddr[ cntAddr ] = 0;
        pAddr = lpLocalHostEntry->h_addr_list[ cntAddr ];
        if ( NULL != pAddr ) localaddr[ cntAddr ] = * ( ( unsigned long * ) pAddr );
    }
    while ( ( NULL != pAddr ) && ( cntAddr < 16 ) );

    CLIENTEVENTLIST::compatibility_iterator nodeVSCP;
    vscpEvent *pvscpEvent = NULL;
    while ( !TestDestroy() && !m_bQuit ) {

        // Wait for event
        if ( wxSEMA_TIMEOUT == pClientItem->m_semClientInputQueue.WaitTimeout( 500 ) ) continue;

        if ( pClientItem->m_clientInputQueue.GetCount() ) {
            pClientItem->m_mutexClientInputQueue.Lock();
            nodeVSCP = pClientItem->m_clientInputQueue.GetFirst();
            pvscpEvent = nodeVSCP->GetData();

            // Remove event from sendqueue
            pClientItem->m_clientInputQueue.DeleteNode ( nodeVSCP );
            pClientItem->m_mutexClientInputQueue.Unlock();

            if ( ( NULL != pvscpEvent ) ) {
                sendUdpEvent( pvscpEvent );
            }

            // Delete the event
            deleteVSCPevent( pvscpEvent );

        }
		
	} // while

	// Remove the client
	m_pCtrlObject->m_wxClientMutex.Lock();
	m_pCtrlObject->removeClient ( pClientItem );
	m_pCtrlObject->m_wxClientMutex.Unlock();

	return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void UDPSendThread::OnExit()
{

}


//////////////////////////////////////////////////////////////////////////////
// sendUdpEvent
//
// TODO - Must check for endian compability
//

bool UDPSendThread::sendUdpEvent( vscpEvent *pEvent )
{	
	unsigned char buf[ 512 ];
	unsigned char *p;
	unsigned short size;

	// Must be a valid event pointer
  if ( NULL == pEvent ) return false;
		
	// Must have a valid socket to work from
#ifdef WIN32
	if ( NULL == m_sendsock ) return false;
#else
	if ( 0 == m_sendsock ) return false;
#endif

	if ( pEvent->sizeData > (512-25) ) pEvent->sizeData = (512-25);
	
	size = pEvent->sizeData + 25;
	if ( size > 512 ) return false;

	// Head
	p = (unsigned char *)&pEvent->head;
	buf[ VSCP_UDP_POS_HEAD + 0 ] = *(p+0);

	// VSCP class
	p = (unsigned char *)&pEvent->vscp_class;
	buf[ VSCP_UDP_POS_CLASS + 0 ] = *(p+1); 
	buf[ VSCP_UDP_POS_CLASS + 1 ] = *(p+0); 

	// VSCP type
	p = (unsigned char *)&pEvent->vscp_type;
	buf[ VSCP_UDP_POS_TYPE + 0 ] = *(p+1); 
	buf[ VSCP_UDP_POS_TYPE + 1 ] = *(p+0); 

	// Node address
	p = (unsigned char *)&pEvent->GUID;
	memcpy( &buf[ VSCP_UDP_POS_GUID + 0 ], p, 16 );

	// VSCP Size
	p = (unsigned char *)&pEvent->sizeData;
	buf[ VSCP_UDP_POS_SIZE + 0 ] = *(p+1); 
	buf[ VSCP_UDP_POS_SIZE + 1 ] = *(p+0);

	// Data Max 487 (512- 25) bytes
	// size = sz - command_byte - control_bytes 
	memcpy( &buf[ VSCP_UDP_POS_DATA + 0 ], pEvent->pdata, pEvent->sizeData );
	
	// Check if CRC should be calculated
	if ( !( pEvent->head & VSCP_NO_CRC_CALC ) ) {
		pEvent->crc = crcFast( buf, size );	
	}
 
	// VSCP CRC
	p = (unsigned char *)&pEvent->crc;
	buf[ pEvent->sizeData + VSCP_UDP_POS_DATA + 0 ] = *(p+1); 
	buf[ pEvent->sizeData + VSCP_UDP_POS_DATA + 1 ] = *(p+0);

	//
	// Fill in the address structure for the server
	//

#ifdef WIN32	
	SOCKADDR_IN broadcast_addr;
#else
	struct sockaddr_in broadcast_addr;
#endif

	memset( (void *)&broadcast_addr, 0, sizeof(broadcast_addr)  );
	broadcast_addr.sin_family = AF_INET;								 
	broadcast_addr.sin_addr.s_addr = htonl( INADDR_BROADCAST ); 	// =inet_addr(SERVERIP);
	broadcast_addr.sin_port = htons( m_pCtrlObject->m_UDPPort ); 	// htons( VSCP_LEVEL2_UDP_PORT );
	
	wxLogTrace( _("wxTRACE_vscpd_Msg"), _("Sending UDP event. Port=%d"), m_pCtrlObject->m_UDPPort  );

	//
	// Send data to the server
	//
	int nRet;
#ifdef WIN32	

	nRet = sendto( m_sendsock,				
						(const char *)buf,					
						size,			
						 0,								 
						( LPSOCKADDR )&broadcast_addr,	
						sizeof( broadcast_addr ) ); 
#else

	nRet = sendto( m_sendsock,
						(void *)buf,
						size,
						0,
						(struct sockaddr*)&broadcast_addr, 
						sizeof( broadcast_addr ) );
						
#endif

#ifdef WIN32
	
	if ( !nRet || ( nRet == SOCKET_ERROR ) || ( nRet != size ) ) { 
		
		// Failed
	 	wxLogTrace(_("wxTRACE_vscpd_Msg"), _("Sending UDP event. Failed"));
		return false;
	}
	
#else
	
	if ( ( 0 == nRet ) || ( nRet == -1 ) || ( nRet != size ) ) { 
		wxLogTrace(_("wxTRACE_vscpd_Msg"), _("Sending UDP event. Failed"));
		// Failed
		return false;
	}
#endif
	
 	wxLogTrace(_("wxTRACE_vscpd_Msg"), _("Sending UDP event. Success"));
	return true;
}







///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////







///////////////////////////////////////////////////////////////////////////////
// UDPReceiveThread
//

UDPReceiveThread::UDPReceiveThread()
		: wxThread( wxTHREAD_JOINABLE )
{
	m_bQuit = false;
	m_pCtrlObject = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// UDPReceiveThread
//

UDPReceiveThread::~UDPReceiveThread()
{

}

///////////////////////////////////////////////////////////////////////////////
// Entry
//
// Is there any messages to send from Level II clients. Send it/them to all
// devices/clients except for itself.
//

void *UDPReceiveThread::Entry()
{
	unsigned long errorCode = 0;
#ifdef WIN32
  int nLen;
#else
	socklen_t nLen;
#endif
	unsigned char buf[ 1024 ];
	unsigned char *p;

	CLIENTEVENTLIST::compatibility_iterator nodeVSCP;
	vscpEvent *pvscpEvent = NULL;

	// Must be a valid control object pointer
	if ( NULL == m_pCtrlObject ) return NULL;


    // We need to create a clientobject and add this object to the list
	CClientItem *pClientItem = new CClientItem;
	if ( NULL == pClientItem ) return NULL;

	// This is an active client
	pClientItem->m_bOpen = true;
	pClientItem->m_type =  CLIENT_ITEM_INTERFACE_TYPE_CLIENT_INTERNAL;
	pClientItem->m_strDeviceName = _("Internal UDP Receive Client. Started at ");
	wxDateTime now = wxDateTime::Now(); 
	pClientItem->m_strDeviceName += now.FormatISODate();
	pClientItem->m_strDeviceName += _(" ");
	pClientItem->m_strDeviceName += now.FormatISOTime();

  // Mark as UDP receive channel
  pClientItem->m_bUDPReceiveChannel = true;

	// Add the client to the Client List
	m_pCtrlObject->m_wxClientMutex.Lock();
	m_pCtrlObject->addClient( pClientItem/*, CLIENT_ITEM_SPECIAL_ID_UDP*/ );
	m_pCtrlObject->m_wxClientMutex.Unlock();


	crcInit();

	char szName[ 128 ];
	struct hostent * lpLocalHostEntry;

	if ( -1 == gethostname ( szName, sizeof ( szName ) ) )
	{
		wxLogError( _( "UDP Receive Thread: Failed to get hostname\n" ) );
		return NULL;
	}

	wxLogDebug( _("udpReceiceThread: Hostname: %s" ), szName );

	lpLocalHostEntry = gethostbyname ( szName );
	if ( NULL == lpLocalHostEntry )
	{
		wxLogError( _("UDP Receive Thread: Failed to get hostaddress\n" ) );
		return NULL;
	}

	// Get the address
	//localaddr = (in_addr_t *)lpLocalHostEntry->h_addr_list[ 0 ];

#ifndef WIN32

/*
  char ac[ 80 ];
  if ( gethostname( ac, sizeof( ac ) ) == -1 ) {
    ;
  }
 
  struct hostent *phe = gethostbyname( ac );

  for ( int i = 0; phe->h_addr_list[i] != 0; ++i ) {
    struct in_addr addr;
    memcpy( &addr, phe->h_addr_list[i], sizeof (struct in_addr ) );
    //cout << "Address " << i << ": " << inet_ntoa(addr) << endl;
    wxLogDebug( _("Interface address:  %X\n"), addr );
  }
*/

#endif

	// Get all local addresses of the local machine
	int idx = -1;
	void *pAddr;
#ifdef WIN32
  unsigned long localaddr[ 16 ]; // max 16 local addresses
#else
	in_addr_t localaddr[ 16 ]; // max 16 local addresses
#endif
	do
	{
		idx++;
		localaddr[ idx ] = 0;
#ifdef WIN32
		pAddr = lpLocalHostEntry->h_addr_list[ idx ];
		if ( NULL != pAddr ) localaddr[ idx ] = *( ( unsigned long *)pAddr);
#else
		pAddr = ( in_addr_t * ) lpLocalHostEntry->h_addr_list[ idx ];
		if ( NULL != pAddr ) localaddr[ idx ] = * ( ( in_addr_t * ) pAddr );
		if ( NULL != pAddr ) wxLogTrace( _("wxTRACE_vscpd_receiveQueue"), 
                                      _( "udpReceiceThread: Local address: %X" ), 
                                      * ( in_addr_t * ) pAddr );

#endif
		
		
	} while ( ( NULL != pAddr ) && ( idx < 16 ) );

	//
	// Create a UDP/IP datagram socket
	//
	int theSocket;

	theSocket = socket ( AF_INET, SOCK_DGRAM, 0 );
	if ( -1 == theSocket )
	{
		return NULL;
	}

	int on = 1; // As suggested by Charles Tewiah
	setsockopt ( theSocket,
	             SOL_SOCKET,
	             SO_REUSEADDR,
	             ( const char * ) &on,
	             sizeof ( on ) );

	//
	// Fill in the address structure
	//
	struct sockaddr_in saServer;

	saServer.sin_family = AF_INET;
	saServer.sin_addr.s_addr = INADDR_ANY;
	saServer.sin_port = htons ( m_pCtrlObject->m_UDPPort );

	// For multi interface machines.
	// If no bind address is given the default interface is used to receive
	// UDP datagrams which means datagrams will be received from all interfaces. Set
	// to ip of interface to select a specific interface.
	if ( 0 != m_pCtrlObject->m_strUdpBindInterfaceAddress.Length() ) {
#if ( WIN32 && (_WIN32_WINNT<0x0600) ) 
		saServer.sin_addr.s_addr = 
			inet_addr( m_pCtrlObject->m_strUdpBindInterfaceAddress.ToAscii() ); 
#else
		inet_pton( AF_INET, 
					m_pCtrlObject->m_strUdpBindInterfaceAddress.ToAscii(), 
					(void *)&saServer.sin_addr.s_addr );
#endif
	}

	//
	// bind the name to the socket
	//

	int nRet;

	nRet = bind ( theSocket,
	              ( struct sockaddr * ) &saServer,
	              sizeof ( struct sockaddr ) );

	if ( -1 == nRet ) {
		return NULL;
	}

	struct sockaddr_in saClient;
	nLen = sizeof ( struct sockaddr_in );

	wxLogDebug( _( "udpReceiceThread: Ready to Work!" ) );

	while ( !TestDestroy() && !m_bQuit ) {

#ifdef WIN32
		memset( buf, 0, sizeof( buf ) );
		
		// Wait for data from the client
		nRet = recvfrom( theSocket,				
							(char *)buf,				
							sizeof(buf ),		
							0,								
							( LPSOCKADDR )&saClient,
							&nLen );	
 							
		if ( !nRet || ( nRet == SOCKET_ERROR ) || ( nRet < 25 ) ) { 
			continue;
		}
#else
		bzero ( ( uint8_t * ) &saClient, sizeof ( saClient ) );
		nLen = sizeof ( struct sockaddr_in );
		bzero ( ( uint8_t * ) buf, sizeof ( buf ) );

		// Wait for data from the client

		nRet = recvfrom ( theSocket,
		                  buf,
		                  sizeof ( buf ),
		                  0,
		                  ( struct sockaddr * ) &saClient,
		                  &nLen );

		if ( ( 0 == nRet ) || ( -1 == nRet ) || ( nRet < 25 ) ) {
			continue;
		}

		wxLogDebug( _( "udpReceiveThread: Incoming event. Addr=%X" ),
		                ( in_addr_t ) saClient.sin_addr.s_addr );
#endif
        // Handle received package

#ifdef WIN32

     	// If this is a packet sent from this machine
		// we just disregards it		
		bool bLocalAddress = false;
		idx = 0;
		while ( ( 0 != localaddr[ idx ] ) && ( idx < 16 ) ) {
			if ( localaddr[ idx ] == (unsigned long)saClient.sin_addr.S_un.S_addr ) {
				bLocalAddress = true;
                break;
			}

			idx++;
		}

#else

		// If this is a packet sent from this machine
		// we just disregards it
		//if ( *localaddr == saClient.sin_addr.s_addr ) continue;
		bool bLocalAddress = false;
		idx = 0;
		while ( ( 0 != localaddr[ idx ] ) && ( idx < 16 ) ) {
		wxLogTrace( _("wxTRACE_vscpd_receiveQueue"),
                     _(" Received address = %x"), ( in_addr_t )
                      saClient.sin_addr.s_addr );
		if ( localaddr[ idx ] == ( in_addr_t ) saClient.sin_addr.s_addr ) {
			bLocalAddress = true;
			wxLogTrace( _("wxTRACE_vscpd_receiveQueue"), 
                      _("Skipped because event has local origin ") );
			break;
		}
		idx++;
	}

#endif

		if ( bLocalAddress ) continue; // From us get next event

		wxLogTrace( _("wxTRACE_vscpd_receiveQueue"), 
                  _( "udpReceiveThread: It's from another machine. Grab it." ) );

		// Check size
		// The size member must be low enough for the data to fit
		// in the package
		unsigned short size;
		p = ( unsigned char * ) &size;
		* ( p+0 ) = buf[ VSCP_UDP_POS_SIZE + 1 ];
		* ( p+1 ) = buf[ VSCP_UDP_POS_SIZE + 0 ];

		if ( ( size + 25 ) > nRet ) {
			// Wrong size -> Package faulty
			continue;
		}

		// Calculate CRC if requested
		if ( ! ( buf[ VSCP_UDP_POS_HEAD + 0 ] & VSCP_NO_CRC_CALC ) ) {
			if ( !crcFast ( buf, size ) ) continue;	// Faulty CRC
		}

		vscpEvent *pEvent = new vscpEvent;

		if ( NULL != pEvent ) {

			pEvent->obid = pClientItem->m_clientID;

			// Head
			p = ( unsigned char * ) &pEvent->head;
			* ( p+0 ) = buf[ VSCP_UDP_POS_HEAD + 0 ];

			// VSCP class
			p = ( unsigned char * ) &pEvent->vscp_class;
			* ( p+0 ) = buf[ VSCP_UDP_POS_CLASS + 1 ];
			* ( p+1 ) = buf[ VSCP_UDP_POS_CLASS + 0 ];

			// VSCP type
			p = ( unsigned char * ) &pEvent->vscp_type;
			* ( p+0 ) = buf[ VSCP_UDP_POS_TYPE + 1 ];
			* ( p+1 ) = buf[ VSCP_UDP_POS_TYPE + 0 ];

			// Node address
			p = ( unsigned char * ) &pEvent->GUID;
			memcpy ( p, &buf[ VSCP_UDP_POS_GUID + 0 ], 16 );

			// Number of valid data bytes
			pEvent->sizeData = size;

			if ( pEvent->sizeData > 487 ) {
				// Can't be a VSCP package
				delete pEvent;
				continue;
			}

			// CRC
			p = ( unsigned char * ) &pEvent->crc;
			* ( p+0 ) = buf[ VSCP_UDP_POS_CRC + 1 ];
			* ( p+1 ) = buf[ VSCP_UDP_POS_CRC + 0 ];

			pEvent->pdata = NULL;

			if ( 0 != pEvent->sizeData ) {

				// Allocate storage for data
				unsigned char *pDataArea = new unsigned char[ pEvent->sizeData ];

				if ( NULL != pDataArea ) {

					// Data Max 487 (512- 25) bytes
					pEvent->pdata = pDataArea;

					// size = sz - command_byte - control_bytes
					memcpy ( pEvent->pdata, &buf[ VSCP_UDP_POS_DATA ], pEvent->sizeData );

				}
			}

            // RX Statistics
            pClientItem->m_statistics.cntReceiveData += pEvent->sizeData;
		    pClientItem->m_statistics.cntReceiveFrames++;


            // There must be room in the send queue
			if ( m_pCtrlObject->m_maxItemsInClientReceiveQueue >
                    m_pCtrlObject->m_clientOutputQueue.GetCount() ) {
				m_pCtrlObject->m_mutexClientOutputQueue.Lock();
				m_pCtrlObject->m_clientOutputQueue.Append ( pEvent );
				m_pCtrlObject->m_semClientOutputQueue.Post();
				m_pCtrlObject->m_mutexClientOutputQueue.Unlock();
			}
			else {
                pClientItem->m_statistics.cntOverruns++;
                deleteVSCPevent( pEvent );
			}    

		}

	} // while

#ifdef WIN32
	_close ( theSocket );
#else
	close ( theSocket );
#endif

    // Remove the client
	m_pCtrlObject->m_wxClientMutex.Lock();
	m_pCtrlObject->removeClient ( pClientItem );
	m_pCtrlObject->m_wxClientMutex.Unlock();

	return NULL;

}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void UDPReceiveThread::OnExit()
{

}
