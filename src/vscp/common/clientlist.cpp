// ClientList.cpp: implementation of the CClientList class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (c) 2000-2018 Ake Hedman, 
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

#include <wx/wx.h>
#include <wx/defs.h>
#include <wx/app.h>
#include <wx/listimpl.cpp>
#include <wx/tokenzr.h>
#include <wx/hashmap.h>

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

const char *interface_description[] = {
    "Unknown (you should not see this).",
    "Internal VSCP server client.",
    "Level I (CANAL) Driver.",
    "Level II Driver.", 
    "TCP/IP Client.",
    "UDP Client.",
    "Web Server Client.",
    "WebSocket Client.",
    "REST client",
    "Multicast client",
    "Multicast channel client",
    "MQTT client",
    "COAP client",
    "Discovery client",
    "JavaScript client",
    "Lua client",
    NULL
};


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
    m_statistics.cntBusWarnings = 0;        // # of bus warnings
    m_statistics.cntBusOff = 0;             // # of bus off's

    m_status.channel_status = 0;
    m_status.lasterrorcode = 0;
    m_status.lasterrorsubcode = 0;
    memset( m_status.lasterrorstr, 0, sizeof( m_status.lasterrorstr ) );
    

    ///////////////////////////////////////////////////////////////////////////
    //                 Working variable storage for clients
    //////////////////////////////////////////////////////////////////////////

    bAuthenticated = false;
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
// CommandStartWith
//

bool CClientItem::CommandStartsWith( const wxString &cmd, bool bFix )
{      
    if ( !m_currentCommand.Upper().StartsWith( cmd.Upper() ) ) {
        return false;
    }
    
    // If asked to do so remove the command.
    if ( bFix ) {
        m_currentCommand = m_currentCommand.Right( m_currentCommand.length() - cmd.Length() );
        m_currentCommand.Trim(false);        
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// compareClientItems
//
// Type of compare function for list sort operation (as in 'qsort')
//

bool compareClientItems( const void *element1, const void *element2 )
{
    return (int)(((CClientItem *)element1)->m_clientID) < 
                (int)(((CClientItem *)element2)->m_clientID);
}


///////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// CClientList
//

CClientList::CClientList()
{
    //m_clientIDCounter = 1;	
    
}

///////////////////////////////////////////////////////////////////////////////
// ~CClientList
//

CClientList::~CClientList()
{
    // Empty the client list
    std::list<CClientItem*>::iterator it;
    for (it=m_clientItemList.begin(); it!=m_clientItemList.end(); ++it) {
        delete *it;
    }

    m_clientItemList.clear();
}

///////////////////////////////////////////////////////////////////////////////
// addClient
//

bool CClientList::addClient( CClientItem *pClientItem, uint32_t id )
{
    std::list<CClientItem*>::iterator it;
    pClientItem->m_clientID = id ? id : 1;

    if ( 0 == id ) {
        // Find next free id
        for (it=m_clientItemList.begin(); it!=m_clientItemList.end(); ++it) {    
            CClientItem *pItem = *it;
            // s the list is sorted on ClientId we have found an
            // unused id if the ClientId is higher than the counter
            if ( pClientItem->m_clientID < pItem->m_clientID ) {
                break;
            }

            pClientItem->m_clientID++;
            if ( 0 == pClientItem->m_clientID ) {
                return false; // All client id's are in use
            }
        }
    }
   
    // We try to assign requested id
    for (it=m_clientItemList.begin(); it!=m_clientItemList.end(); ++it) {    
        CClientItem *pItem = *it;
        // If id is already in use fail
        if ( pClientItem->m_clientID == pItem->m_clientID ) {
            return false;    
        }
    } 
    
    // Append to list
    m_clientItemList.push_back( pClientItem );

    // Sort list on client id
    m_clientItemList.sort( compareClientItems );

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// removeClient
//

bool CClientList::removeClient( CClientItem *pClientItem )
{
    // Must be a valid pointer
    if ( NULL == pClientItem ) return false;
    
    pClientItem->m_clientInputQueue.Clear();
    
    // Take away the node
    m_clientItemList.remove( pClientItem );
    delete pClientItem;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getClientFromId
//

CClientItem *CClientList::getClientFromId( uint32_t id )
{
    std::list<CClientItem*>::iterator it;
    CClientItem *returnItem = NULL;

    for (it=m_clientItemList.begin(); it!=m_clientItemList.end(); ++it) {    
        CClientItem *pItem = *it;
        if ( pItem->m_clientID == id ) {
            returnItem = pItem;
            break;
        }
    }

    return returnItem;
}

///////////////////////////////////////////////////////////////////////////////
// getClientFromGUID
//

CClientItem *CClientList::getClientFromGUID( cguid& guid )
{
    std::list<CClientItem*>::iterator it;
    CClientItem *returnItem = NULL;

    for (it=m_clientItemList.begin(); it!=m_clientItemList.end(); ++it) {    
        CClientItem *pItem = *it;
        if ( pItem->m_guid == guid ) {
            returnItem = pItem;
            break;
        }
    }

    return returnItem;
}