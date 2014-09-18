// ClientList.cpp: implementation of the CClientList class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2014 Ake Hedman, 
// Grodans Paradis AB, <akhe@grodansparadis.com>
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

#ifdef WIN32
#include <winsock2.h>
#endif

//#include <wx/wxprec.h>
#include <wx/wx.h>
#include <wx/defs.h>
#include <wx/app.h>
#include <wx/listimpl.cpp>

#ifdef WIN32

#include "canal_win32_ipc.h"

#else

#define _POSIX
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#endif


#include "canal_macro.h"
#include "vscp.h"
#include "vscphelper.h"
#include "clientlist.h"


WX_DEFINE_LIST(CLIENTEVENTLIST);
WX_DEFINE_LIST(VSCPCLIENTLIST);


///////////////////////////////////////////////////////////////////////////////
// CClientItem
//

CClientItem::CClientItem()
{
	m_bOpen = false;                        // Initially Not Open
	m_flags = 0;                            // No flags
	m_status.channel_status = 0;
	m_clientID = 0;
    m_type = CLIENT_ITEM_INTERFACE_TYPE_NONE;
    m_bUDPReceiveChannel = false;

	// Nill GUID
    m_guid.clear();

	// Nill Level II mask (accept all)
	vscp_clearVSCPFilter( &m_filterVSCP );
	
	m_statistics.cntReceiveFrames = 0;      // # of receive frames
	m_statistics.cntTransmitFrames = 0;     // # of transmitted frames
	m_statistics.cntReceiveData = 0;        // # of received data bytes
	m_statistics.cntTransmitData = 0;       // # of transmitted data bytes	
	m_statistics.cntOverruns = 0;           // # of overruns
	m_statistics.cntBusWarnings = 0;        // # of bys warnings
	m_statistics.cntBusOff = 0;             // # of bus off's

	m_status.channel_status = 0;
	m_status.lasterrorcode = 0;
	m_status.lasterrorsubcode = 0;
	memset( m_status.lasterrorstr, 0, sizeof( m_status.lasterrorstr ) );

	///////////////////////////////////////////////////////////////////////////
	//                 Working variable storage for clients
	//////////////////////////////////////////////////////////////////////////

	m_bAuthorized = false;
	m_pUserItem = NULL;

	/// Buffer for read data
	wxString m_readBuffer;
}

///////////////////////////////////////////////////////////////////////////////
// ~CClientItem
//

CClientItem::~CClientItem()
{
	CLIENTEVENTLIST::iterator iter;

	for ( iter = m_clientInputQueue.begin(); 
            iter != m_clientInputQueue.end(); ++iter ) {
		vscpEvent *pEvent = *iter;
		vscp_deleteVSCPevent( pEvent );
	}

	m_clientInputQueue.Clear();

}



///////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CClientList
//
// The list is sorted numeric so that messages with higher priority is
// fetched first of the double linked list
//

CClientList::CClientList()
{
	m_clientIDCounter = 1;	
	m_clientItemList.DeleteContents ( true );
}

///////////////////////////////////////////////////////////////////////////////
// ~CClientList
//

CClientList::~CClientList()
{
  
}

///////////////////////////////////////////////////////////////////////////////
// addClient
//

bool CClientList::addClient( CClientItem *pClientItem, uint32_t id )
{
    // Assign id and update id counter
    pClientItem->m_clientID = m_clientIDCounter++;
    
    // Set specific id if requested 
    if ( id ) pClientItem->m_clientID = id;

    m_clientItemList.Append( pClientItem );

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// removeClient
//

bool CClientList::removeClient( CClientItem *pClientItem )
{
    // Must be a valid pointer
    if ( NULL == pClientItem ) return false;

    // Take away the node
    if ( !m_clientItemList.DeleteObject( pClientItem ) ) {
        return false;
    }

    return true;
}
