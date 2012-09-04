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
// $RCSfile: udpReceiveThread.cpp,v $
// $Date: 2005/03/08 08:29:22 $
// $Author: akhe $
// $Revision: 1.8 $

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
#include "../../common/dllist.h"
#include "../../vscp/common/vscp.h"
#include "controlobject.h"



///////////////////////////////////////////////////////////////////////////////
// udpReceiveThread
//
//


void udpReceiveThread ( void *pObject )
{
	unsigned long errorCode = 0;

	unsigned long nRead = 0;
	unsigned long nWrite = 0;
	int nLen;
	HANDLE udpReceiveMutex;
	char buf[ 1024 ];
	unsigned char *p;

	CControlObject *pctrlObject = ( CControlObject * ) pObject;

	char szName[ 128 ];

	udpReceiveMutex = OpenMutex ( MUTEX_ALL_ACCESS, false, VSCPD_UDP_OBJ_MUTEX );

	if ( SOCKET_ERROR == gethostname ( szName, sizeof ( szName ) ) )
	{
		int err = WSAGetLastError();
	}

	LPHOSTENT lpLocalHostEntry;
	lpLocalHostEntry = gethostbyname ( szName );
	if ( NULL == lpLocalHostEntry )
	{
		ExitThread ( errorCode );
	}
	unsigned long * localaddr = ( unsigned long * ) lpLocalHostEntry->h_addr_list[ 0 ];

	//
	// Create a UDP/IP datagram socket
	//
	SOCKET theSocket;

	theSocket = socket ( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if ( theSocket == INVALID_SOCKET )
	{
		ExitThread ( errorCode );
	}

	//
	// Fill in the address structure
	//

	SOCKADDR_IN saServer;


	saServer.sin_family = AF_INET;
	saServer.sin_addr.s_addr = INADDR_ANY;	// Let WinSock assign address
	saServer.sin_port = htons ( VSCP_LEVEL2_UDP_PORT );

	//
	// bind the name to the socket
	//

	int nRet;

	nRet = bind ( theSocket,
	              ( LPSOCKADDR ) &saServer,
	              sizeof ( struct sockaddr ) );
	if ( nRet == SOCKET_ERROR )
	{
		ExitThread ( errorCode );
	}

	SOCKADDR_IN saClient;
	nLen = sizeof ( SOCKADDR );


	while ( !pctrlObject->m_bQuit )
	{

		memset ( buf, 0, sizeof ( buf ) );

		// Wait for data from the client
		nRet = recvfrom ( theSocket,
		                  buf,
		                  sizeof ( buf ),
		                  0,
		                  ( LPSOCKADDR ) &saClient,
		                  &nLen );

		if ( !nRet || ( nRet == SOCKET_ERROR ) || ( nRet < 25 ) )
		{
			continue;
		}

		// Handle received package

		// If this is a packet sent from this machine
		// we just disregards it
		if ( *localaddr == ( unsigned long ) saClient.sin_addr.S_un.S_addr ) continue;

		// Check size
		// The size member must be low enough for the data to fit
		// in the package
		unsigned short size;
		p = ( unsigned char * ) &size;
		*( p+0 ) = buf[ VSCP_POS_SIZE + 0 ];
		*( p+1 ) = buf[ VSCP_POS_SIZE + 1 ];

		if ( ( size + 25 ) > nRet ) {
			size = nRet - 25;
		}

		vscpMsg2 * pMsg = new vscpMsg2;

		if ( NULL != pMsg ) {

			// Head
			p = ( unsigned char * ) &pMsg->head;
			* ( p+0 ) = buf[ VSCP_POS_HEAD + 0 ];

			// VSCP class
			p = ( unsigned char * ) &pMsg->vscp_class;
			* ( p+0 ) = buf[ VSCP_POS_CLASS + 1 ];
			* ( p+1 ) = buf[ VSCP_POS_CLASS + 0 ];

			// VSCP type
			p = ( unsigned char * ) &pMsg->vscp_type;
			* ( p+0 ) = buf[ VSCP_POS_TYPE + 1 ];
			* ( p+1 ) = buf[ VSCP_POS_TYPE + 0 ];

			// Node address
			p = ( unsigned char * ) &pMsg->address;
			memcpy ( p, &buf[ VSCP_POS_ADDRESS + 0 ], 16 );

			// Number of valid data bytes
			pMsg->size = size;

			if ( pMsg->size > 487 ) pMsg->size = 487;

			// CRC
			p = ( unsigned char * ) &pMsg->crc;
			* ( p+0 ) = buf[ VSCP_POS_CRC + 1 ];
			* ( p+1 ) = buf[ VSCP_POS_CRC + 0 ];

			pMsg->pdata = NULL;


			if ( 0 != pMsg->size ) {

				// Allocate storage for data
				unsigned char *pDataArea = new unsigned char[ pMsg->size ];

				if ( NULL != pDataArea ) {

					// Data Max 487 (512- 25) bytes
					pMsg->pdata = pDataArea;

					// size = sz - command_byte - control_bytes
					memcpy ( pMsg->pdata, &buf[ VSCP_POS_DATA ], pMsg->size );

				}
			}

			// Get mutex
			WaitForSingleObject ( udpReceiveMutex, INFINITE );

			// Must be room for the package
			if ( pctrlObject->m_udpReceiveQueue.nCount <
			        MAX_ITEMS_UDP_RECEIVE_QUEUE )
			{

				dllnode *pnewudpmsg = new dllnode;
				if ( NULL != pnewudpmsg )
				{

					// Add to in queue
					pnewudpmsg->Key = 0;
					pnewudpmsg->pKey = &pnewudpmsg->Key ;
					pnewudpmsg->pObject = pMsg;

					if ( !dll_addNode ( &pctrlObject->m_udpReceiveQueue, pnewudpmsg ) )
					{
						delete pnewudpmsg;

					}
				}
			}

			// Release mutex
			ReleaseMutex ( udpReceiveMutex );

		}
	}

	closesocket ( theSocket );
	ExitThread ( errorCode );

}

