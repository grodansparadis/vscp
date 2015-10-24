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
#include <sys/types.h>   // for type definitions 
#include <winsock2.h>
#include <ws2tcpip.h>    // for win socket structs 
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

#include <crc.h>
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
    int sock_mc;                    // socket descriptor 
    struct sockaddr_in mc_addr;     // socket address structure  
    unsigned short mc_port = vscp_readStringValue( m_pCtrlObject->m_strMulticastAnnounceAddress ) ; // multicast port 
    unsigned char mc_ttl = m_pCtrlObject->m_ttlMultiCastAnnounce;                                   // time to live (hop count) 

#ifdef WIN32

    WSADATA wsaData;            // Windows socket DLL structure 

    // Load Winsock 2.0 DLL
    if ( WSAStartup( MAKEWORD( 2, 0 ), &wsaData ) != 0 ) {
        fprintf( stderr, "WSAStartup() failed" );
        m_pCtrlObject->logMsg( _( "Automation multicast announce WSAStartup() failed\r\n" ), DAEMON_LOGMSG_CRITICAL, DAEMON_LOGTYPE_GENERAL );
        return NULL;
    }

    // create a socket for sending to the multicast address 
    if ( ( sock_mc = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) < 0 ) {
        perror( "socket() failed" );
        m_pCtrlObject->logMsg( _( "Automation multicast announce sock() failed\r\n" ), DAEMON_LOGMSG_CRITICAL, DAEMON_LOGTYPE_GENERAL );
        return NULL;
    }

    // set the TTL (time to live/hop count) for the send 
    if ( ( setsockopt( sock_mc, IPPROTO_IP, IP_MULTICAST_TTL,
                       ( const char* )&mc_ttl, sizeof( mc_ttl ) ) ) < 0 ) {
        perror( "setsockopt() failed" );
        m_pCtrlObject->logMsg(  _( "Automation multicast announce setsockopt() failed\r\n" ), DAEMON_LOGMSG_CRITICAL, DAEMON_LOGTYPE_GENERAL );
        return NULL;
    }

    // construct a multicast address structure 
    memset( &mc_addr, 0, sizeof( mc_addr ) );
    mc_addr.sin_family = AF_INET;
    mc_addr.sin_addr.s_addr = inet_addr( "224.0.23.158" );
    mc_addr.sin_port = htons( mc_port );

    sendto( sock_mc, "LETS GO!\n", 9, 0, ( struct sockaddr * ) &mc_addr, sizeof( mc_addr ) );

#else

    // create a socket for sending to the multicast address 
    if ( ( sock = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) < 0 ) {
        perror( "socket() failed" );
        return NULL;
    }

    // set the TTL (time to live/hop count) for the send 
    if ( ( setsockopt( sock, IPPROTO_IP, IP_MULTICAST_TTL,
                       ( void* )&mc_ttl, sizeof( mc_ttl ) ) ) < 0 ) {
        perror( "setsockopt() failed" );
        return NULL;
    }

    // construct a multicast address structure 
    memset( &mc_addr, 0, sizeof( mc_addr ) );
    mc_addr.sin_family = AF_INET;
    mc_addr.sin_addr.s_addr = inet_addr( "224.0.23.158" );
    mc_addr.sin_port = htons( mc_port );

#endif

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
    m_pCtrlObject->addClient( pClientItem );
    m_pCtrlObject->m_wxClientMutex.Unlock();

    // Clear the filter (Allow everything )
    vscp_clearVSCPFilter( &pClientItem->m_filterVSCP );

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

                m_pCtrlObject->logMsg( wxString::Format( _( "Automation event sent: Class=%d Type=%d\r\n" ),
                                        eventEx.vscp_class, eventEx.vscp_type ),
                                        DAEMON_LOGMSG_INFO,
                                        DAEMON_LOGTYPE_GENERAL );
            
                // Yes event should be sent
                eventEx.obid = pClientItem->m_clientID;
                pClientItem->m_guid.writeGUID( eventEx.GUID );

                if ( VSCP_TYPE_PROTOCOL_SEGCTRL_HEARTBEAT == eventEx.vscp_type ) {
                    // crc8 of VSCP daemon GUID should be indata byte 0
                    eventEx.data[ 0 ] = vscp_calcCRC4GUIDArray( m_pCtrlObject->m_guid.getGUID() );
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

        int rv = pClientItem->m_clientInputQueue.GetCount();
        // Wait for incoming event
        if ( wxSEMA_TIMEOUT == pClientItem->m_semClientInputQueue.WaitTimeout( 100 ) ) continue;
	
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

                for ( int i=0; i<cntAddr; i++ ) {

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
                
                // Add node to knows nodes
                CNodeInformation *pNode = addNodeIfNotKnown( pEvent );
                
                if ( NULL != pNode ) {

                    // Send multi cast information
                    sendMulticastInformationEvent( sock_mc, pNode, &mc_addr );

                    sendto( sock_mc, "New node online\n", 16, 0, ( struct sockaddr * ) &mc_addr, sizeof( mc_addr ) );
                }

            }

            // Level I node heart beat - collect 
            else if ( ( VSCP_CLASS1_INFORMATION == pEvent->vscp_class ) && 
                ( VSCP_TYPE_INFORMATION_NODE_HEARTBEAT == pEvent->vscp_type ) ) {
                
                /*cguid guid;
                wxString str;
                guid.getFromArray( pEvent->GUID );                
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

                }*/

                sendto( sock_mc, "Node Heart Beat Level 1\n", 24, 0, ( struct sockaddr * ) &mc_addr, sizeof( mc_addr ) );

            }

            // Level II node heart beat   - collect 
            else if ( ( VSCP_CLASS2_INFORMATION == pEvent->vscp_class ) && 
                ( VSCP_TYPE_INFORMATION_NODE_HEARTBEAT == pEvent->vscp_type ) ) {
                
                sendto( sock_mc, "Node Heart Beat Level 2\n", 24, 0, ( struct sockaddr * ) &mc_addr, sizeof( mc_addr ) );

            }

            // Remove the event
            vscp_deleteVSCPevent( pEvent );

        } // event in queue

    } // while

    // Remove messages in the client queues
    m_pCtrlObject->m_wxClientMutex.Lock();
    m_pCtrlObject->removeClient( pClientItem );
    m_pCtrlObject->m_wxClientMutex.Unlock();

    // Close the multicast socket
    closesocket( sock_mc );

    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void daemonVSCPThread::OnExit()
{

}

///////////////////////////////////////////////////////////////////////////////
// addNodeIfNotKnown
//

CNodeInformation * daemonVSCPThread::addNodeIfNotKnown( vscpEvent *pEvent )
{
    CNodeInformation *pNode;
    wxString strGUID;
    cguid guid;

    // Check pointer
    if ( NULL == pEvent ) return false;

    guid.getFromArray( pEvent->GUID );
    guid.toString( strGUID );

    m_pCtrlObject->m_mutexKnownNodes.Lock();
    pNode = m_pCtrlObject->m_knownNodes.m_nodes[ strGUID ];
    m_pCtrlObject->m_mutexKnownNodes.Unlock();

    if ( NULL == pNode ) {

        // We have not seen this node before and we will
        // try to collect information from it
        pNode = new CNodeInformation;
        if ( NULL != pNode ) {

            // Add the node to known nodes
            m_pCtrlObject->m_mutexKnownNodes.Lock();
            m_pCtrlObject->m_knownNodes.m_nodes[ strGUID ] = pNode;
            m_pCtrlObject->m_mutexKnownNodes.Unlock();

            // Is there a device realted to this node
            m_pCtrlObject->m_mutexDeviceList.Lock();
            CDeviceItem *pDeviceItem =
                m_pCtrlObject->m_deviceList.getDeviceItemFromClientId( pEvent->obid );
            if ( NULL != pDeviceItem ) {

                // Construct a name if no name set
                // 'device.nickname'
                if ( pNode->m_strNodeName.IsEmpty() ) {
                    pNode->m_strNodeName = pDeviceItem->m_strName;
                    pNode->m_strNodeName += _( "." );
                    pNode->m_strNodeName += wxString::Format( _( "%lu" ), pDeviceItem->m_pClientItem->m_clientID );
                }

                // Save interface
                pNode->m_interfaceguid = pDeviceItem->m_interface_guid;

                // Save Client ID
                pNode->m_clientID = pEvent->obid;

                // We set name fron client id
                // 'client|clientid|.nickname'
                if ( pNode->m_strNodeName.IsEmpty() ) {
                    pNode->m_strNodeName = _( "client" );
                    pNode->m_strNodeName += wxString::Format( _( "%lu" ), pDeviceItem->m_pClientItem->m_clientID );
                    pNode->m_strNodeName += _( "." );
                    pNode->m_strNodeName += wxString::Format( _( "%u" ), ( pEvent->GUID[ 14 ] << 8 ) + pEvent->GUID[ 15 ] );
                }

                // Now let the discovery thread do the rest of the work

            }
            else {

                // No device associated with this event - but there must be a client

                // Save Client ID
                pNode->m_clientID = pEvent->obid;

                m_pCtrlObject->m_mutexDeviceList.Lock();
                CClientItem *pClientItem =
                    m_pCtrlObject->m_clientList.getClientFromId( pEvent->obid );
                if ( NULL != pClientItem ) {
                    // Save interface
                    pNode->m_interfaceguid = pClientItem->m_guid;
                }
                m_pCtrlObject->m_mutexDeviceList.Unlock();

                // We set name fron client id
                // 'client|clientid|.nickname'
                if ( pNode->m_strNodeName.IsEmpty() ) {
                    pNode->m_strNodeName = _( "client" );
                    pNode->m_strNodeName += wxString::Format( _( "%lu" ), pClientItem->m_clientID );
                    pNode->m_strNodeName += _( "." );
                    pNode->m_strNodeName += wxString::Format( _( "%u" ), ( pEvent->GUID[ 14 ] << 8 ) + pEvent->GUID[ 15 ] );
                }


            }
            m_pCtrlObject->m_mutexDeviceList.Unlock();
        }

    }
    else {

        // This node is known so we don't need to do a thing. Hurray!

    }

    return pNode;
}

///////////////////////////////////////////////////////////////////////////////
// sendMulticastInformationEvent
//

bool daemonVSCPThread::sendMulticastInformationEvent( int sock_mc, 
                                                        CNodeInformation *pNode,
                                                        struct sockaddr_in *pmc_addr )
{
    uint8_t mcsendBuf[ 1024 ];         // string to send 

    // Check pointer
    if ( NULL == pNode ) return false;

    //  0 	Packet type & encryption setings
    //  1 	HEAD
    //  2 	Timestamp microseconds MSB
    //  3 	Timestamp microseconds
    //  4 	Timestamp microseconds
    //  5 	Timestamp microseconds LSB
    //  6 	CLASS MSB
    //  7 	CLASS LSB
    //  8 	TYPE MSB
    //  9 	TYPE LSB
    //  10 - 24 	ORIGINATING GUID MSB
    //  25 	ORIGINATING GUID LSB
    //  26 	DATA SIZE MSB
    //  27 	DATA SIZE LSB
    //  28 - n 	data � limited to max 512 - 25 = 487 bytes
    //  len - 2 	CRC MSB( Calculated on HEAD + CLASS + TYPE + ADDRESS + SIZE + DATA� )
    //  len - 1 	CRC LSB

    // Packe type
    mcsendBuf[ VSCP_MULTICAST_PACKET0_POS_PKTTYPE ] = SET_VSCP_MULTICAST_TYPE( VSCP_MULTICAST_TYPE_EVENT , VSCP_MULTICAST_ENCRYPTION_NONE );
    
    // VSCP header
    mcsendBuf[ VSCP_MULTICAST_PACKET0_POS_HEAD_MSB ] = 0;
    mcsendBuf[ VSCP_MULTICAST_PACKET0_POS_HEAD_LSB ] = VSCP_PRIORITY_NORMAL;
    
    // Timestamp
    uint32_t timestamp = vscp_makeTimeStamp();
    wxUINT32_SWAP_ON_LE( timestamp );
    mcsendBuf[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP + 0 ] = ( timestamp >> 24 ) & 0xff;
    mcsendBuf[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP + 1 ] = ( timestamp >> 16 ) & 0xff;
    mcsendBuf[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP + 2 ] = ( timestamp >> 8 ) & 0xff;
    mcsendBuf[ VSCP_MULTICAST_PACKET0_POS_TIMESTAMP + 3 ] = timestamp & 0xff;

    // VSCP class
    uint16_t vscp_class = VSCP_CLASS2_INFORMATION;
    wxUINT32_SWAP_ON_LE( vscp_class );
    mcsendBuf[ VSCP_MULTICAST_PACKET0_POS_VSCP_CLASS + 0 ] = ( vscp_class >> 8 ) & 0xff;
    mcsendBuf[ VSCP_MULTICAST_PACKET0_POS_VSCP_CLASS + 1 ] = vscp_class & 0xff;

    // VSCP Type
    uint16_t vscp_type = VSCP2_TYPE_INFORMATION_PROXY_HEART_BEAT;
    wxUINT32_SWAP_ON_LE( vscp_type );
    mcsendBuf[ VSCP_MULTICAST_PACKET0_POS_VSCP_TYPE + 0 ] = ( vscp_type >> 8 ) & 0xff;
    mcsendBuf[ VSCP_MULTICAST_PACKET0_POS_VSCP_TYPE + 1 ] = vscp_type & 0xff;

    // Originating GUID 
    memcpy( mcsendBuf + VSCP_MULTICAST_PACKET0_POS_VSCP_GUID, m_pCtrlObject->m_guid.m_id, 16 );

    // Size of payload =  128 bytes
    mcsendBuf[ VSCP_MULTICAST_PACKET0_POS_VSCP_SIZE + 0 ] = 0x00;
    mcsendBuf[ VSCP_MULTICAST_PACKET0_POS_VSCP_SIZE + 1 ] = 0x80;

    // Clear data
    memset( mcsendBuf + VSCP_MULTICAST_PACKET0_POS_VSCP_DATA, 0, 128 );

    // Real GUID of node 
    memcpy( mcsendBuf + VSCP_MULTICAST_PACKET0_POS_VSCP_DATA, pNode->m_realguid.getGUID(), 16 );

    // Interface GUID of node 
    memcpy( mcsendBuf + VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 32, pNode->m_interfaceguid.getGUID(), 16 );

    // Name of node
    memcpy( mcsendBuf + VSCP_MULTICAST_PACKET0_POS_VSCP_DATA + 64, 
                pNode->m_deviceName.mbc_str(), 
                MIN( 64, strlen( pNode->m_deviceName.mbc_str() ) ) );

    // CRC
    crcInit();
    uint16_t crc = crcFast( mcsendBuf, VSCP_MULTICATS_PACKET0_HEADER_LENGTH + 128 );
    wxUINT32_SWAP_ON_LE( crc );
    mcsendBuf[ VSCP_MULTICATS_PACKET0_HEADER_LENGTH + 128 + 0 ] = ( crc >> 8 ) & 0xff;
    mcsendBuf[ VSCP_MULTICATS_PACKET0_HEADER_LENGTH + 128 + 1 ] = crc & 0xff;

    return ( ( VSCP_MULTICATS_PACKET0_HEADER_LENGTH + 128 + 1 ) == sendto( sock_mc,
                        (const char *)mcsendBuf, 
                        VSCP_MULTICATS_PACKET0_HEADER_LENGTH + 128 + 1,
                        0, 
                        ( struct sockaddr * )pmc_addr,
                        sizeof( *pmc_addr ) ) );
}


///////////////////////////////////////////////////////////////////////////////
//                              Discovery
///////////////////////////////////////////////////////////////////////////////





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

	pEvent->pdata[ 0 ] = nodeid;      // Node to read from
    pEvent->pdata[ 1 ] = reg;         // Register to read
    
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