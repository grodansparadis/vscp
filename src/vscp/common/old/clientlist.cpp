// ClientList.cpp: implementation of the CClientList class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2008 Ake Hedman, D of Scandinavia, <akhe@eurosource.se>
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
// $RCSfile: clientlist.cpp,v $                                       
// $Date: 2005/01/05 12:50:53 $                                  
// $Author: akhe $                                              
// $Revision: 1.2 $ 
//////////////////////////////////////////////////////////////////////

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


#ifdef WIN32


#else	// U N I X

#define _POSIX
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

#endif

#include "vscp.h"
#include "clientlist.h"


///////////////////////////////////////////////////////////////////////////////
// CClientItem
//

CClientItem::CClientItem()
{
	m_bOpen = false;		// Initially Not Open
	m_flags = 0;			// No flags
#ifndef WIN32
	m_clientMutex = PTHREAD_MUTEX_INITIALIZER;
#endif

	dll_init( &m_inputQueue, SORT_NONE );	
}

///////////////////////////////////////////////////////////////////////////////
// ~CClientItem
//

CClientItem::~CClientItem()
{
	//dll_removeAllNodes( &m_inputQueue );
	while ( NULL != m_inputQueue.pHead ) {
		
		CClientItem *pClientItem = (CClientItem *)m_inputQueue.pTail->pObject;		
		
		if ( NULL != pClientItem ) {
	
			if ( NULL != ( (vscpMsg2*)pClientItem )->pdata ) {
				delete [] ( ( (vscpMsg2*)pClientItem )->pdata );
				( (vscpMsg2*)pClientItem )->pdata = NULL;
			}
			
		}

		dll_removeNode( &m_inputQueue, m_inputQueue.pTail );

	}
}


///////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CClientList
//
// The list is sorted numeric so that messages with higher priority is
// fetched firts of the double linked list
//

CClientList::CClientList()
{
	m_clientIDCounter = 1;
	dll_init( &m_clientList, SORT_NUMERIC );	// Numeric => priotities are handled	 
}

///////////////////////////////////////////////////////////////////////////////
// ~CClientList
//

CClientList::~CClientList()
{
	//dll_removeAllNodes( &m_list );	
	while ( NULL != m_clientList.pHead ) {
		CClientItem *pClientItem = (CClientItem *)m_clientList.pTail->pObject;
		if ( NULL != pClientItem ) delete pClientItem;
		m_clientList.pTail->pObject = NULL;
		dll_removeNode( &m_clientList, m_clientList.pTail );
	}	 
}

///////////////////////////////////////////////////////////////////////////////
// addClient
//

bool CClientList::addClient( CClientItem *pClientItem )
{
	char buf[32];
	
	pClientItem->m_clientID = m_clientIDCounter++;
	dllnode *pNode = new dllnode;

	if ( NULL != pNode ) {
		pNode->pKey = &pClientItem->m_clientID;
		pNode->pObject = pClientItem;
		sprintf( buf, "%s%i", VSCPD_CLIENT_MUTEX, pClientItem->m_clientID );
#ifdef WIN32		
		pClientItem->m_hclientMutex = CreateMutex( NULL, false, buf );
#endif
		dll_addNode( &m_clientList, pNode );
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// removeClient
//

bool CClientList::removeClient( CClientItem *pClientItem )
{
	if ( NULL == pClientItem ) return false;

	dllnode *pNode = dll_findNodeFromID( &m_clientList, pClientItem->m_clientID );
	
	removeData( &pClientItem->m_inputQueue );
	if ( NULL != pNode ) {
		dll_removeNode( &m_clientList, pNode );
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// removeData
//

void CClientList::removeData( DoubleLinkedList* pdll ) 
{
	// Must be something to worl with
	if ( NULL == pdll ) return;
	
	while ( NULL != pdll->pHead ) {
		
		CClientItem *pClientItem = (CClientItem *)pdll->pTail->pObject;		
		
		if ( NULL != pClientItem ) {
	
			if ( NULL != ( (vscpMsg2*)pClientItem )->pdata ) {
				delete [] ( ( (vscpMsg2*)pClientItem )->pdata );
				( (vscpMsg2*)pClientItem )->pdata = NULL;
			}
			
		}

		dll_removeNode( pdll, pdll->pTail );

	}
}

