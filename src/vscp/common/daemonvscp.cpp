// daemon_VSCP.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (http://www.vscp.org)
//
// Copyright (C) 2000-2014 
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
#include "vscpd.h"
#include "vscphelper.h"
#include "../../common/dllist.h"
#include "../../common/md5.h"
#include "controlobject.h"
#include "guid.h"


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

        // Check if automation event should be sent and send it if so
        vscpEventEx eventEx;
        if ( m_pCtrlObject->m_automation.doWork( &eventEx ) ) {
            
            // Yes should be sent
            eventEx.obid = pClientItem->m_clientID;
            pClientItem->m_guid.writeGUID( eventEx.GUID );

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
                            pnewEvent->pdata[ 0 ] = (uint8_t)(VSCP_DAEMON_SERVER_CAPABILITIES >> 8);
                            pnewEvent->pdata[ 1 ] = (uint8_t)(VSCP_DAEMON_SERVER_CAPABILITIES & 0x0f);
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

            } // New node on-line   - collect 
            else if ( ( VSCP_CLASS1_PROTOCOL == pEvent->vscp_class ) && 
                ( VSCP_TYPE_PROTOCOL_NEW_NODE_ONLINE == pEvent->vscp_type ) ) {
                
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


