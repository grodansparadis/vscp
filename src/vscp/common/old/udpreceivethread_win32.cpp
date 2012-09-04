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
// $RCSfile: udpreceivethread_win32.cpp,v $                                       
// $Date: 2005/09/09 07:42:41 $                                  
// $Author: akhe $                                              
// $Revision: 1.14 $ 


// Common includes
#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"
#include "version.h"
#include "canal_macro.h"
#include "canal_win32_ipc.h"
#include "../../common/dllist.h"
#include "../../common/crc.h"
#include "../../vscp/common/vscp.h"
#include "controlobject.h"


///////////////////////////////////////////////////////////////////////////////
// udpReceiveThread
//
//  TODO Byteorder switch

void udpReceiveThread( void *pObject )
{
	unsigned long errorCode = 0;
	 
	unsigned long nRead = 0;
	unsigned long nWrite = 0;
	int nLen;
	unsigned char buf[ 1024 ];
	unsigned char *p;

	crcInit();
	CControlObject *pctrlObject = (CControlObject *)pObject;
	
	char szName[ 128 ];
 	gethostname( szName, sizeof( szName ) );
	LPHOSTENT lpLocalHostEntry;
	lpLocalHostEntry = gethostbyname( szName );
	if ( NULL == lpLocalHostEntry ) {	
		ExitThread( errorCode );
	}
	

	// Get all local addresses
	int idx = -1;
	void *pAddr;
	unsigned long localaddr[ 16 ]; // max 16 local addresses
	do {
		idx++;
		localaddr[ idx ] = 0;
		pAddr = lpLocalHostEntry->h_addr_list[ idx ];
		if ( NULL != pAddr ) localaddr[ idx ] = *( ( unsigned long *)pAddr);	
	} while ( ( NULL != pAddr ) && ( idx < 16 )  );

	//
	// Create a UDP/IP datagram socket
	//	
	SOCKET theSocket;
	
	theSocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if ( theSocket == INVALID_SOCKET ) {
		ExitThread( errorCode );
	}

	int on = 1; // As suggested by Charles Tewiah
	setsockopt( theSocket, 
					SOL_SOCKET,
					SO_REUSEADDR,
					(const char *)&on, 
					sizeof ( on ) ); 
 
	//
	// Fill in the address structure
	//
 	
	SOCKADDR_IN saServer;

	saServer.sin_family = AF_INET;
	saServer.sin_addr.s_addr = INADDR_ANY;	// Let WinSock assign address
	saServer.sin_port = htons( pctrlObject->m_UDPPort  );	   

	//
	// bind the name to the socket
	//

	int nRet;

	nRet = bind( theSocket,		
					( LPSOCKADDR )&saServer, 
					sizeof (struct sockaddr ) );					
	if ( nRet == SOCKET_ERROR ) {
		ExitThread( errorCode );		
	}
 
	SOCKADDR_IN saClient;
	nLen = sizeof( SOCKADDR );
 
	// Receive loop

	while ( !pctrlObject->m_bQuit ) {

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

		// Handle received package

		// If this is a packet sent from this machine
		// we just disregards it		
		bool bLocalAddress = false;
		idx = 0;
		while ( ( 0 != localaddr[ idx ] ) && ( idx < 16 ) ) {
			if ( localaddr[ idx ] == (unsigned long)saClient.sin_addr.S_un.S_addr ) {
				bLocalAddress = true;
			}

			idx++;
		}
    

		if ( bLocalAddress ) continue;

 		
		// Check size
		// The size member must be low enough for the data to fit
		// in the package
		unsigned short size;
		p = (unsigned char *)&size;
		*(p+0) = buf[ VSCP_UDP_POS_SIZE + 1 ];
		*(p+1) = buf[ VSCP_UDP_POS_SIZE + 0 ];
	
		if ( ( size + 25 ) > nRet  ) {
			// Size wrong. Faulty package
			continue; 		
		}

		// Check if CRC should be calculated
		if ( !( buf[ VSCP_UDP_POS_HEAD + 0 ] & VSCP_NO_CRC_CALC ) ) {
			if ( !crcFast( buf, size ) ) continue;	// Faulty CRC	
		}	 

		vscpEvent * pEvent = new vscpEvent;

		if ( NULL != pEvent ) {

			// Head
			p = (unsigned char *)&pEvent->head;
			*(p+0) = buf[ VSCP_UDP_POS_HEAD + 0 ];
									
			// VSCP class
			p = (unsigned char *)&pEvent->vscp_class;
			*(p+0) = buf[ VSCP_UDP_POS_CLASS + 1 ];
			*(p+1) = buf[ VSCP_UDP_POS_CLASS + 0 ]; 

			// VSCP type
			p = (unsigned char *)&pEvent->vscp_type;
			*(p+0) = buf[ VSCP_UDP_POS_TYPE + 1 ];
			*(p+1) = buf[ VSCP_UDP_POS_TYPE + 0 ];
									
			// Node address
			p = (unsigned char *)&pEvent->GUID;
			memcpy( p, &buf[ VSCP_UDP_POS_GUID + 0 ], 16 );

			// Number of valid data bytes
			pEvent->sizeData = size;

			if ( pEvent->sizeData > 487 ) {
				// Can't be a VSCP package
				delete pEvent;
				continue;
			}
									
			// CRC
			p = (unsigned char *)&pEvent->crc;
			*(p+0) = buf[ VSCP_UDP_POS_DATA + size + 1 ];
			*(p+1) = buf[ VSCP_UDP_POS_DATA + size + 0 ];

			pEvent->pdata = NULL;


			if ( 0 != pEvent->sizeData ) {
			
				// Allocate storage for data
				unsigned char *pDataArea = new unsigned char[ pEvent->sizeData ];

				if ( NULL != pDataArea ) {
			
					// Data Max 487 (512- 25) bytes
					pEvent->pdata = pDataArea;

					// size = sz - command_byte - control_bytes
					memcpy( pEvent->pdata, &buf[ VSCP_UDP_POS_DATA ], pEvent->sizeData ); 

				}
			}
		
			// Get access to UDP queue		
			pctrlObject->m_wxUDPMutex.Lock();

			// Must be room for the package
			if (  pctrlObject->m_UDPQueue.nCount < MAX_ITEMS_UDP_RECEIVE_QUEUE ) {

				dllnode *pnewudpmsg = new dllnode;
				if ( NULL != pnewudpmsg ) {

					// Add to in queue
					pnewudpmsg->Key = 0;
					pnewudpmsg->pKey = &pnewudpmsg->Key ;
					pnewudpmsg->pObject = pEvent;
							
					if ( !dll_addNode( &pctrlObject->m_UDPQueue, pnewudpmsg ) ) {
						delete pnewudpmsg;
					}
					else {
						// Signal UDP data available
						SetEvent( pctrlObject->m_hEventReceiveUDP );
					}
				}
			}

			// Release the UDP queue
			pctrlObject->m_wxUDPMutex.Unlock();
 
		}
	}

	closesocket( theSocket );
	ExitThread( errorCode );

}

