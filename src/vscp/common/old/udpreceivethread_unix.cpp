// udpReceiveThread.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (http://www.vscp.org)
//
// Copyright (C) 2000-2008 Ake Hedman, D of Scandinavia,<akhe@eurosource.se>
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
// $RCSfile: udpreceivethread_unix.cpp,v $
// $Date: 2006/03/11 13:57:05 $
// $Author: akhe $
// $Revision: 1.13 $

#define _POSIX

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>

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
#include "../../common/dllist.h"
#include "../../common/crc.h"
#include "../../vscp/common/vscp.h"
#include "controlobject.h"

///////////////////////////////////////////////////////////////////////////////
// udpReceiveThread
//
//

void udpReceiveThread ( void *pObject )
{
	unsigned long errorCode = 0;
	socklen_t nLen;

	unsigned char buf[ 1024 ];
	unsigned char *p;

	crcInit();
	CControlObject *pctrlObject = ( CControlObject * ) pObject;

	char szName[ 128 ];
	struct hostent * lpLocalHostEntry;

	if ( -1 == gethostname ( szName, sizeof ( szName ) ) )
	{
		wxLogError ( _ ( "UDP Receive Thread: Failed to get hostname\n" ) );
		pthread_exit ( &errorCode );
	}

	wxLogDebug ( _ ( "udpReceiceThread: Hostname: %s" ), szName );

	lpLocalHostEntry = gethostbyname ( szName );
	if ( NULL == lpLocalHostEntry )
	{
		wxLogError ( _ ( "UDP Receive Thread: Failed to get hostaddress\n" ) );
		pthread_exit ( &errorCode );
	}

	// Get the address
	//localaddr = (in_addr_t *)lpLocalHostEntry->h_addr_list[ 0 ];

	// Get all local addresses of the local machine
	int idx = -1;
	void *pAddr;
	in_addr_t localaddr[ 16 ]; // max 16 local addresses
	do
	{
		idx++;
		localaddr[ idx ] = 0;
		pAddr = ( in_addr_t * ) lpLocalHostEntry->h_addr_list[ idx ];
		if ( NULL != pAddr ) localaddr[ idx ] = * ( ( in_addr_t * ) pAddr );
		if ( NULL != pAddr ) wxLogDebug ( _ ( "udpReceiceThread: Local address: %X" ), * ( in_addr_t * ) pAddr );
	}
	while ( ( NULL != pAddr ) && ( idx < 16 ) );

	//
	// Create a UDP/IP datagram socket
	//
	int theSocket;

	theSocket = socket ( AF_INET, SOCK_DGRAM, 0 );
	if ( -1 == theSocket )
	{
		pthread_exit ( &errorCode );
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
	saServer.sin_port = htons ( pctrlObject->m_UDPPort );

	//
	// bind the name to the socket
	//

	int nRet;

	nRet = bind ( theSocket,
	              ( struct sockaddr * ) &saServer,
	              sizeof ( struct sockaddr ) );

	if ( -1 == nRet )
	{
		pthread_exit ( &errorCode );
	}

	struct sockaddr_in saClient;
	nLen = sizeof ( struct sockaddr_in );

	wxLogDebug ( _ ( "udpReceiceThread: Ready to Work!" ) );

	// Receive loop

	while ( !TestDestroy() && !pctrlObject->m_bQuit )
	{

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

		if ( ( 0 == nRet ) || ( -1 == nRet ) || ( nRet < 25 ) )
		{
			continue;
		}

		wxLogDebug ( _ ( "UDPReceiveThread: Incoming event. Addr=%X" ),
		             ( in_addr_t ) saClient.sin_addr.s_addr );


		// Handle received package

		// If this is a packet sent from this machine
		// we just disregards it
		//if ( *localaddr == saClient.sin_addr.s_addr ) continue;
		bool bLocalAddress = false;
		idx = 0;
		while ( ( 0 != localaddr[ idx ] ) && ( idx < 16 ) )
		{
			if ( localaddr[ idx ] == ( in_addr_t ) saClient.sin_addr.s_addr )
			{
				bLocalAddress = true;
			}
			idx++;
		}
		if ( bLocalAddress ) continue;

		wxLogDebug ( _ ( "UDPReceiveThread: It's from another machine." ) );

		// Check size
		// The size member must be low enough for the data to fit
		// in the package
		unsigned short size;
		p = ( unsigned char * ) &size;
		* ( p+0 ) = buf[ VSCP_UDP_POS_SIZE + 1 ];
		* ( p+1 ) = buf[ VSCP_UDP_POS_SIZE + 0 ];

		if ( ( size + 25 ) > nRet )
		{
			// Wrong size -> Package faulty
			continue;
		}

		// Calculate CRC if requested
		if ( ! ( buf[ VSCP_UDP_POS_HEAD + 0 ] & VSCP_NO_CRC_CALC ) )
		{
			if ( !crcFast ( buf, size ) ) continue;	// Faulty CRC
		}

		vscpEvent *pEvent = new vscpEvent;

		if ( NULL != pEvent )
		{

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

			if ( pEvent->sizeData > 487 )
			{
				// Can't be a VSCP package
				delete pEvent;
				continue;
			}

			// CRC
			p = ( unsigned char * ) &pEvent->crc;
			* ( p+0 ) = buf[ VSCP_UDP_POS_CRC + 1 ];
			* ( p+1 ) = buf[ VSCP_UDP_POS_CRC + 0 ];

			pEvent->pdata = NULL;

			if ( 0 != pEvent->sizeData )
			{

				// Allocate storage for data
				unsigned char *pDataArea = new unsigned char[ pEvent->sizeData ];

				if ( NULL != pDataArea )
				{

					// Data Max 487 (512- 25) bytes
					pEvent->pdata = pDataArea;

					// size = sz - command_byte - control_bytes
					memcpy ( pEvent->pdata, &buf[ VSCP_UDP_POS_DATA ], pEvent->sizeData );

				}
			}

			// Get mutex
			pctrlObject->m_wxUDPMutex.Lock();

			// Must be room for the package
			if ( pctrlObject->m_UDPQueue.nCount <
			        MAX_ITEMS_UDP_RECEIVE_QUEUE )
			{

				dllnode *pnewudpmsg = new dllnode;
				if ( NULL != pnewudpmsg )
				{

					// Add to in queue
					pnewudpmsg->Key = 0;
					pnewudpmsg->pKey = &pnewudpmsg->Key ;
					pnewudpmsg->pObject = pEvent;

					if ( !dll_addNode ( &pctrlObject->m_UDPQueue, pnewudpmsg ) )
					{
						delete pnewudpmsg;

					}
				}
			}

			// Release mutex
			pctrlObject->m_wxUDPMutex.Unlock();

		}
	}

	close ( theSocket );
	pthread_exit ( &errorCode );

}

