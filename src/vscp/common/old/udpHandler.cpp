///////////////////////////////////////////////////////////////////////////////
// udpHandler.cpp : implementation file
//
// This file is part is part of CANAL (CAN Abstraction Layer)
// http://www.vscp.org)
//
// Copyright (C) 2000-2008 Ake Hedman, eurosource, <akhe@eurosource.se>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// $RCSfile: udpHandler.cpp,v $                                       
// $Date: 2005/01/05 12:16:10 $                                  
// $Author: akhe $                                              
// $Revision: 1.2 $ 
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "udpHandler.h"
#include "canalpipe.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CudpHandler

CudpHandler::CudpHandler()
{
 
}

CudpHandler::~CudpHandler()
{
	 
}


// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CudpHandler, CAsyncSocket)
	//{{AFX_MSG_MAP(CudpHandler)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////
// Create

void CudpHandler::Create( DoubleLinkedList * pList )  
{
	if ( NULL == pList ) return;

	m_pListUdpReceive = pList;

	m_udpReceiveMutex = OpenMutex( MUTEX_ALL_ACCESS, false, UDP_OBJ_MUTEX );

	if ( !CAsyncSocket::Create( CANAL_UDP_PORT, 
									SOCK_DGRAM, 
									FD_READ, 
									"0.0.0.0" ) ) {
		AfxMessageBox("Unable to create listen socket!");
		return;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CudpHandler member functions

void CudpHandler::OnReceive(int nErrorCode) 
{
	CUDPRecord UDPRecord;
	char buf[ 1024 ];	 
	unsigned short pktsize;
	unsigned char *p;
	
	
	CAsyncSocket::OnReceive( nErrorCode );

	if ( nErrorCode ) {
		return;
	}

	if ( NULL == m_pListUdpReceive ) return;
	
	pktsize = ReceiveFrom( buf, 
							sizeof( buf ), 
							UDPRecord.m_IPAddress,
							UDPRecord.m_Port );

	if ( !pktsize || ( pktsize == SOCKET_ERROR ) || ( pktsize < 25 ) ) { 
		return;
	}

	// Check size
	// The size member must be low enough for the data to fit
	// in the package
	unsigned short size;
	p = (unsigned char *)&size;
	*(p+0) = buf[ POS_DATASIZE + 0 ];
	*(p+1) = buf[ POS_DATASIZE + 1 ];
	
	if ( ( size + 25 ) > pktsize  ) {
		size = pktsize - 25;
	}

	canalMsg * pMsg = new canalMsg;

	if ( NULL != pMsg ) {
/*
		// Head
		p = (unsigned char *)&pMsg->vscp_type;
		*(p+0) = buf[ POS_HEAD + 0 ];
									
		// VSCP class
		p = (unsigned char *)&pMsg->vscp_class;
		*(p+0) = buf[ POS_CLASS + 0 ];
		*(p+1) = buf[ POS_CLASS + 1 ]; 

		// VSCP type
		p = (unsigned char *)&pMsg->vscp_type;
		*(p+0) = buf[ POS_TYPE + 0 ];
		*(p+1) = buf[ POS_TYPE + 1 ];
									
		// Node address
		p = (unsigned char *)&pMsg->address;
		memcpy( p, &buf[ POS_ADDRESS + 0 ], 16 );

		// Number of valid data bytes
		pMsg->size = size;

		if ( pMsg->size > 487 ) pMsg->size = 487;
									
		// CRC
		p = (unsigned char *)&pMsg->crc;
		*(p+0) = buf[ POS_CRC + 0 ];
		*(p+1) = buf[ POS_CRC + 1 ];

		pMsg->pdata = NULL;


		if ( 0 != pMsg->size ) {
			
			// Allocate storage for data
			unsigned char *pDataArea = new unsigned char[ pMsg->size ];

			if ( NULL != pDataArea ) {
			
				// Data Max 487 (512- 25) bytes
				pMsg->pdata = pDataArea;

				// size = sz - command_byte - control_bytes
				memcpy( pMsg->pdata, &buf[ POS_DATA ], pMsg->size ); 

			}
		}
*/		
		// Get mutex
		WaitForSingleObject( m_udpReceiveMutex, INFINITE );

		dllnode *pnewudpmsg = new dllnode;
		if ( NULL != pnewudpmsg ) {

			// Add to in queue
			pnewudpmsg->Key = 0;
			pnewudpmsg->pKey = &pnewudpmsg->Key ;
			pnewudpmsg->pObject = pMsg;
							
			if ( !dll_addNode( m_pListUdpReceive, 
									pnewudpmsg ) ) {
				delete pnewudpmsg;

			}
		}

		// Release mutex
		ReleaseMutex( m_udpReceiveMutex );

	}
}
