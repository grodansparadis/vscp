// daemon_VSCP.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (http://www.vscp.org)
//
// Copyright (C) 2000-2015 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
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

#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"
#include <wx/datetime.h>
#include <wx/listimpl.cpp>
#include <wx/hashmap.h>

#ifdef WIN32
#else

#define _POSIX

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#endif

#ifndef DWORD
#define DWORD unsigned long
#endif

#include "daemonvscp.h"
#include "canal_win32_ipc.h"
#include "canal_macro.h"
#include "vscp.h"
#include "vscphelper.h"
#include "clientlist.h"
#include <dllist.h>
#include <md5.h>
#include <crc8.h>
#include "controlobject.h"
#include "guid.h"
#include "vscpd_caps.h"


WX_DEFINE_LIST(DISCOVERYLIST);
WX_DEFINE_LIST(VSCP_KNOWN_NODES_LIST);


///////////////////////////////////////////////////////////////////////////////
// cnodeInformation CTOR
//

cnodeInformation::cnodeInformation()
{
    m_pClientItem = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// cnodeInforamtion DTOR
//

cnodeInformation::~cnodeInformation()
{
    ;
}



///////////////////////////////////////////////////////////////////////////////
// daemonVSCPThread
//
// This thread listens for connection on a TCP socket and starts a new thread
// to handle client requests
//

daemonVSCPThread::daemonVSCPThread()
		 : wxThread( wxTHREAD_JOINABLE )
{
    m_bQuit = false;
    m_pCtrlObject = NULL;
}


daemonVSCPThread::~daemonVSCPThread()
{
    ;
}


///////////////////////////////////////////////////////////////////////////////
// Entry
//

void *daemonVSCPThread::Entry()
{
    // Must have a valid pointer to the control object
    if ( NULL == m_pCtrlObject ) return NULL;

    // We need to create a clientobject and add this object to the list
    CClientItem *pClientItem = new CClientItem;
    if ( NULL == pClientItem ) return NULL;

    // This is an active client
    pClientItem->m_bOpen = true;
    pClientItem->m_type =  CLIENT_ITEM_INTERFACE_TYPE_CLIENT_INTERNAL;
    pClientItem->m_strDeviceName = _("Internal Daemon VSCP Worker Client. Started at ");
    wxDateTime now = wxDateTime::Now(); 
    pClientItem->m_strDeviceName += now.FormatISODate();
    pClientItem->m_strDeviceName += _(" ");
    pClientItem->m_strDeviceName += now.FormatISOTime();

    // Add the client to the Client List
    m_pCtrlObject->m_wxClientMutex.Lock();
    m_pCtrlObject->addClient ( pClientItem );
    m_pCtrlObject->m_wxClientMutex.Unlock();

    char szName[ 128 ];
#ifdef WIN32
    LPHOSTENT lpLocalHostEntry;
#else
    struct hostent *lpLocalHostEntry;
#endif
    gethostname ( szName, sizeof ( szName ) );
    lpLocalHostEntry = gethostbyname ( szName );
    if ( NULL == lpLocalHostEntry ) {
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

    CLIENTEVENTLIST::compatibility_iterator nodeClient;
    while ( !TestDestroy() && !m_bQuit ) {

        // Automation
        if (  m_pCtrlObject->m_automation.isAutomationEnabled() ) {
        
            // Check if automation event should be sent and send it if so
            vscpEventEx eventEx;
            if ( m_pCtrlObject->m_automation.doWork( &eventEx ) ) {
            
                // Yes event should be sent
                eventEx.obid = pClientItem->m_clientID;
                pClientItem->m_guid.writeGUID( eventEx.GUID );

                if ( VSCP_TYPE_PROTOCOL_SEGCTRL_HEARTBEAT == eventEx.vscp_type ) {
                    // crc8 of VSCP daemon GUID should be indata byte 0
                    eventEx.data[ 0 ] = 
					    vscp_calcCRC4GUIDArray( m_pCtrlObject->m_guid.getGUID() );
                }

                vscpEvent *pnewEvent = new vscpEvent;
                if ( NULL != pnewEvent ) {

                    // Convert event to correct format
                    vscp_convertVSCPfromEx( pnewEvent, &eventEx );

                    // Statistics
                    pClientItem->m_statistics.cntTransmitData += eventEx.sizeData;
                    pClientItem->m_statistics.cntTransmitFrames++;

                    // There must be room in the send queue
                    if ( m_pCtrlObject->m_maxItemsInClientReceiveQueue >
                                m_pCtrlObject->m_clientOutputQueue.GetCount() ) {
                        m_pCtrlObject->m_mutexClientOutputQueue.Lock();
                        m_pCtrlObject->m_clientOutputQueue.Append ( pnewEvent );
                        m_pCtrlObject->m_semClientOutputQueue.Post();
                        m_pCtrlObject->m_mutexClientOutputQueue.Unlock();
                    }
                }
            }
        }

        // Wait for incoming event
        if ( wxSEMA_TIMEOUT == pClientItem->m_semClientInputQueue.WaitTimeout( 500 ) ) continue;
	
        if ( pClientItem->m_clientInputQueue.GetCount() ) {

            pClientItem->m_mutexClientInputQueue.Lock();
            nodeClient = pClientItem->m_clientInputQueue.GetFirst();
            vscpEvent *pEvent = nodeClient->GetData();
            pClientItem->m_clientInputQueue.DeleteNode( nodeClient ); // Remove the node
            pClientItem->m_mutexClientInputQueue.Unlock();

            if ( NULL == pEvent ) continue;

            //*****************************************
            // First check for HIGH END SERVER PROBE (27)
            // and send out HIGH END SERVER RESPONSE (28) if
            // received.
            //*****************************************

            if ( ( VSCP_CLASS1_PROTOCOL == pEvent->vscp_class ) && 
                    ( VSCP_TYPE_PROTOCOL_HIGH_END_SERVER_PROBE == pEvent->vscp_type ) ) {

                for ( int i=0;i<cntAddr; i++ ) {

                    // Yes this is a "HIGH END SERVER PROBE"
                    // We should send a "HIGH END SERVER RESPONSE"
                    vscpEvent *pnewEvent = new vscpEvent;
                    if ( NULL != pnewEvent ) {
                        pnewEvent->obid = pClientItem->m_clientID;
                        pnewEvent->head = 0;
                        pnewEvent->vscp_class = 0;
                        pnewEvent->vscp_type = 28;
                        pnewEvent->sizeData = 8;
                        
                        pClientItem->m_guid.writeGUID(pnewEvent->GUID);

                        pnewEvent->pdata = new unsigned char[ 8 ];
                        if ( NULL != pnewEvent->pdata ) {
                            pnewEvent->pdata[ 0 ] = (uint8_t)( VSCP_DAEMON_SERVER_CAPABILITIES >> 8 );
                            pnewEvent->pdata[ 1 ] = (uint8_t)( VSCP_DAEMON_SERVER_CAPABILITIES & 0x0f);
                            pnewEvent->pdata[ 2 ] = ( localaddr[ i ] >> 24 );
                            pnewEvent->pdata[ 3 ] = ( localaddr[ i ] >> 16 );
                            pnewEvent->pdata[ 4 ] = ( localaddr[ i ] >> 8 );
                            pnewEvent->pdata[ 5 ] = ( localaddr[ i ] & 0xff );
                            pnewEvent->pdata[ 6 ] = 0x25;	// TODO Change can be multiple servers
                            pnewEvent->pdata[ 7 ] = 0x7E;
                        }

                        // Statistics
                        pClientItem->m_statistics.cntTransmitData += pEvent->sizeData;
                        pClientItem->m_statistics.cntTransmitFrames++;

                        // There must be room in the send queue
                        if ( m_pCtrlObject->m_maxItemsInClientReceiveQueue >
                                m_pCtrlObject->m_clientOutputQueue.GetCount() ) {
                            m_pCtrlObject->m_mutexClientOutputQueue.Lock();
                            m_pCtrlObject->m_clientOutputQueue.Append ( pnewEvent );
                            m_pCtrlObject->m_semClientOutputQueue.Post();
                            m_pCtrlObject->m_mutexClientOutputQueue.Unlock();
                        }
                    }

                } // for each server address

            } 
            
            // New node on-line   - collect 
            else if ( ( VSCP_CLASS1_PROTOCOL == pEvent->vscp_class ) && 
                ( VSCP_TYPE_PROTOCOL_NEW_NODE_ONLINE == pEvent->vscp_type ) ) {
                
                cguid guid;
                guid.getFromArray( pEvent->GUID );
                wxString str;
                guid.toString( str );
                if ( wxNOT_FOUND != m_knownGUIDs.Index( str, false ) ) {

                    // We have not seen this node before and we will
                    // try to collect information from it

                }

            }

            // Level I node heart beat   - collect 
            else if ( ( VSCP_CLASS1_INFORMATION == pEvent->vscp_class ) && 
                ( VSCP_TYPE_INFORMATION_NODE_HEARTBEAT == pEvent->vscp_type ) ) {
                
                cguid guid;
                guid.getFromArray( pEvent->GUID );
                wxString str;
                guid.toString( str );
                if ( wxNOT_FOUND != m_knownGUIDs.Index( str, false ) ) {

                    // We have not seen this node before and we will
                    // try to collect information from it
                    cguid guid;
                    guid.getFromArray( pEvent->GUID );
                    wxString str;
                    guid.toString( str );
                    if ( wxNOT_FOUND != m_knownGUIDs.Index( str, false ) ) {

                        // We have not seen this node before and we will
                        // try to collect information from it
                        struct discoveredNodeInfo *pdiscoverNode = new struct discoveredNodeInfo;
                        if ( NULL != pdiscoverNode ) {
                            pdiscoverNode->bStatus = 0;                 // Working
                            pdiscoverNode->nodeid = pEvent->GUID[15];   // node id
                            pdiscoverNode->clientId = pEvent->obid;     // clientid
                            pdiscoverNode->guid = guid;                 // Node guid
                            pdiscoverNode->pThread = NULL;              // No thread yet
                            pdiscoverNode->pThread = new discoveryVSCPThread();
                            if (NULL != pdiscoverNode->pThread) {
                                // Append the discovery node to the list
                                m_discoverList.Append( pdiscoverNode );
                            }
                            else {
                                delete pdiscoverNode;
                            }
                        }
                    }

                }

            }

            // Level II node heart beat   - collect 
            else if ( ( VSCP_CLASS2_INFORMATION == pEvent->vscp_class ) && 
                ( VSCP_TYPE_INFORMATION_NODE_HEARTBEAT == pEvent->vscp_type ) ) {
                
                

            }

            // Remove the event
            vscp_deleteVSCPevent( pEvent );

        } // event in queue

    } // while

    // Remove messages in the client queues
    m_pCtrlObject->m_wxClientMutex.Lock();
    m_pCtrlObject->removeClient( pClientItem );
    m_pCtrlObject->m_wxClientMutex.Unlock();

    return NULL;

}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void daemonVSCPThread::OnExit()
{

}











///////////////////////////////////////////////////////////////////////////////
// discoveryVSCPThread
//


discoveryVSCPThread::discoveryVSCPThread()
    : wxThread( wxTHREAD_JOINABLE )
{
    m_bQuit = false;
    m_nodeid = 0;
    m_clientID = 0;
    m_pClientItem = NULL;
    m_pCtrlObject = NULL;
}


discoveryVSCPThread::~discoveryVSCPThread()
{
    ;
}


///////////////////////////////////////////////////////////////////////////////
// Entry
//

void *discoveryVSCPThread::Entry()
{
    // Must have a valid pointer to the control object
    if ( NULL == m_pCtrlObject ) return NULL;

    // We need to create a clientobject and add this object to the list
    m_pClientItem = new CClientItem;
    if ( NULL == m_pClientItem ) return NULL;

    // This is an active client
    m_pClientItem->m_bOpen = true;
    m_pClientItem->m_type =  CLIENT_ITEM_INTERFACE_TYPE_CLIENT_INTERNAL;
    m_pClientItem->m_strDeviceName = _("VSCP node discovery thread. Started at ");
    wxDateTime now = wxDateTime::Now(); 
    m_pClientItem->m_strDeviceName += now.FormatISODate();
    m_pClientItem->m_strDeviceName += _(" ");
    m_pClientItem->m_strDeviceName += now.FormatISOTime();

    // Add the client to the Client List
    m_pCtrlObject->m_wxClientMutex.Lock();
    m_pCtrlObject->addClient ( m_pClientItem );
    m_pCtrlObject->m_wxClientMutex.Unlock();

    // Read GUID of node
    cguid newguid;
    uint8_t content;
    for ( int i=0xd0; i<0xe0; i++ ) {

        int error; // Resend errors
        for (error=0; error<3; error++ ) {
            if ( readLevel1Register( m_nodeid, 
								        i, 
								        &content,
                                        m_clientID,
                                        1000 ) ) {
                newguid.setAt( i, content );
                break;
            }

            // Check if we failed
            if ( error >= 3 ) {
                goto error_abort;
            }
        }
    }

error_abort:

    // Remove messages in the client queues
    m_pCtrlObject->m_wxClientMutex.Lock();
    m_pCtrlObject->removeClient( m_pClientItem );
    m_pCtrlObject->m_wxClientMutex.Unlock();

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void discoveryVSCPThread::OnExit()
{

}


//////////////////////////////////////////////////////////////////////////////
// readLevel1Register
//

bool discoveryVSCPThread::readLevel1Register( uint8_t nodeid, 
												uint8_t reg, 
												uint8_t *pcontent,
                                                uint32_t clientID,
                                                uint32_t timeout )
{
	bool rv = true;
	wxString strBuf;
    
	// Check pointer
	if ( NULL == pcontent ) return false;

    // New event
    vscpEvent *pEvent = new vscpEvent;
    if ( NULL == pEvent ) return false;

    pEvent->vscp_class = VSCP_CLASS1_PROTOCOL;
    pEvent->vscp_type = VSCP_TYPE_PROTOCOL_READ_REGISTER;
	pEvent->sizeData = 2;
    
    // Allocate data
    pEvent->pdata = new uint8_t[2];
    if ( NULL == pEvent->pdata ) return false;

	pEvent->pdata[ 16 ] = nodeid;      // Node to read from
    pEvent->pdata[ 17 ] = reg;         // Register to read
    
    // Send event
    sendEvent( pEvent, clientID );

	wxLongLong startTime = ::wxGetLocalTimeMillis();

    CLIENTEVENTLIST::compatibility_iterator nodeClient;
	while ( true ) {

        // Check for read error timeout
		if ( ( ::wxGetLocalTimeMillis() - startTime ) > timeout ) {
				return false;
		}

        // Wait for incoming event
        if ( wxSEMA_TIMEOUT == m_pClientItem->m_semClientInputQueue.WaitTimeout( 500 ) ) continue;
	
        if ( m_pClientItem->m_clientInputQueue.GetCount() ) {

            m_pClientItem->m_mutexClientInputQueue.Lock();
            nodeClient = m_pClientItem->m_clientInputQueue.GetFirst();
            vscpEvent *pEvent = nodeClient->GetData();
            m_pClientItem->m_clientInputQueue.DeleteNode( nodeClient ); // Remove the node
            m_pClientItem->m_mutexClientInputQueue.Unlock();

            // Check if this is a response to our register read
            if ( ( VSCP_CLASS1_PROTOCOL == pEvent->vscp_class ) && 
                    ( VSCP_TYPE_PROTOCOL_RW_RESPONSE == pEvent->vscp_type &&
                    ( nodeid == pEvent->GUID[15] ) ) ) {
                if ( ( 2 == pEvent->sizeData ) && ( reg == pEvent->pdata[1] ) ) {
                    // We have a response
                    *pcontent = pEvent->pdata[1];
                }
            }

            vscp_deleteVSCPevent( pEvent );
			
		}


	} // while

	return rv;
}


///////////////////////////////////////////////////////////////////////////////
// sendEvent
//

bool discoveryVSCPThread::sendEvent( vscpEvent *pEvent, uint32_t obid )
{
    bool bSent = false;

    // Find client
    m_pCtrlObject->m_wxClientMutex.Lock();

    VSCPCLIENTLIST::iterator iter;
    for (iter = m_pCtrlObject->m_clientList.m_clientItemList.begin(); 
            iter != m_pCtrlObject->m_clientList.m_clientItemList.end(); 
            ++iter) {
                    
        CClientItem *pItem = *iter;
                                  
        if ( pItem->m_clientID == obid ) {
            // Found
            bSent = true;
            m_pCtrlObject->sendEventToClient( pItem, pEvent );
            break;
        }

    }

	m_pCtrlObject->m_wxClientMutex.Unlock();

    return bSent;
}