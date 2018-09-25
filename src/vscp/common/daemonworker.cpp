// daemonvscp.cpp
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (c) 2000-2018 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
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

#include <vscp_debug.h>
#include <crc.h>
#include "daemonworker.h"
#include "canal_win32_ipc.h"
#include "canal_macro.h"
#include "vscp.h"
#include "vscphelper.h"
#include "clientlist.h"
#include <dllist.h>
#include <mongoose.h>
#include <crc8.h>
#include "controlobject.h"
#include "guid.h"
#include "vscpd_caps.h"


WX_DEFINE_LIST( DISCOVERYLIST );


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
    unsigned short mc_port = VSCP_ANNOUNCE_MULTICAST_PORT;
    unsigned char mc_ttl =
        m_pCtrlObject->m_ttlMultiCastAnnounce; // time to live (hop count) 
    wxString ip;
    if ( !parseInterface( m_pCtrlObject->m_strMulticastAnnounceAddress, 
                            ip, 
                            &mc_port ) ) {
        m_pCtrlObject->logMsg( _("Multicast announce: Announce address "
                                 "has wrong format.\n" ) );
        return NULL;
    }
                
#ifdef WIN32

    WSADATA wsaData;            // Windows socket DLL structure

    // Load Winsock 2.0 DLL
    if ( WSAStartup( MAKEWORD( 2, 0 ), &wsaData ) != 0 ) {
        fprintf( stderr, "WSAStartup() failed" );
        m_pCtrlObject->logMsg( _( "Automation multicast announce WSAStartup() failed\n" ),
                               DAEMON_LOGMSG_NORMAL,
                                DAEMON_LOGTYPE_GENERAL );
        return NULL;
    }

    // create a socket for sending to the multicast address
    if ( ( sock_mc = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) < 0 ) {
        perror( "socket() failed" );
        m_pCtrlObject->logMsg( _( "Automation multicast announce sock() failed\n" ),
                               DAEMON_LOGMSG_NORMAL,
                                DAEMON_LOGTYPE_GENERAL );
        return NULL;
    }

    // set the TTL (time to live/hop count) for the send
    if ( ( setsockopt( sock_mc, IPPROTO_IP, IP_MULTICAST_TTL,
                       ( const char* )&mc_ttl, sizeof( mc_ttl ) ) ) < 0 ) {
        perror( "setsockopt() failed" );
        m_pCtrlObject->logMsg(  _( "Automation multicast announce setsockopt() failed\n" ),
                               DAEMON_LOGMSG_NORMAL,
                                DAEMON_LOGTYPE_GENERAL );
        return NULL;
    }

    // construct a multicast address structure
    memset( &mc_addr, 0, sizeof( mc_addr ) );
    mc_addr.sin_family = AF_INET;
    mc_addr.sin_addr.s_addr = inet_addr( VSCP_MULTICAST_IPV4_ADDRESS_STR );
    mc_addr.sin_port = htons( mc_port );

#else

    // create a socket for sending to the VSCP multicast address
    if ( ( sock_mc = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) < 0 ) {
        perror( "socket() failed" );
        return NULL;
    }

    // set the TTL (time to live/hop count) for the send
    if ( ( setsockopt( sock_mc, IPPROTO_IP, IP_MULTICAST_TTL,
                       ( void* )&mc_ttl, sizeof( mc_ttl ) ) ) < 0 ) {
        perror( "setsockopt() failed" );
        return NULL;
    }
    
    int one = 1;
    setsockopt( sock_mc, SOL_SOCKET, SO_REUSEADDR, &one, sizeof( one ) );

    // construct a multicast address structure
    memset( &mc_addr, 0, sizeof( mc_addr ) );
    mc_addr.sin_family = AF_INET;
    mc_addr.sin_addr.s_addr = inet_addr( VSCP_MULTICAST_IPV4_ADDRESS_STR );
    mc_addr.sin_port = htons( VSCP_ANNOUNCE_MULTICAST_PORT );

#endif

    // Must have a valid pointer to the control object
    if ( NULL == m_pCtrlObject ) return NULL;

    // We need to create a client item and add this object to the list
    // of clients
    CClientItem *pClientItem = new CClientItem;
    if ( NULL == pClientItem ) return NULL;

    // This is an active client
    pClientItem->m_bOpen = true;
    pClientItem->m_type =  CLIENT_ITEM_INTERFACE_TYPE_CLIENT_INTERNAL;
    pClientItem->m_strDeviceName = _("Internal VSCP Daemon Worker Client.");    
    pClientItem->m_strDeviceName += _("|Started at ");
    pClientItem->m_strDeviceName += wxDateTime::Now().FormatISODate();
    pClientItem->m_strDeviceName += _(" ");
    pClientItem->m_strDeviceName += wxDateTime::Now().FormatISOTime();

    // Add the client to the Client List
    m_pCtrlObject->m_wxClientMutex.Lock();
    if ( !m_pCtrlObject->addClient( pClientItem, CLIENT_ID_DAEMON_WORKER ) ) {
        // Failed to add client
        delete pClientItem;
        m_pCtrlObject->m_wxClientMutex.Unlock();
        m_pCtrlObject->logMsg( _("Deamon worker: Failed to add client. Terminating thread.") );
        return NULL;
    }
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
    
    
    ////////////////////////////////////////////////////////////////////////////
    //                 * * *  W O R K I N G  L O O P  * * *
    ////////////////////////////////////////////////////////////////////////////


    CLIENTEVENTLIST::compatibility_iterator nodeClient;
    while ( !TestDestroy() && !m_bQuit ) {

        // Automation
        if (  m_pCtrlObject->m_automation.isAutomationEnabled() ) {

            // Check if automation event should be sent and send it if
            // that is the case
            
            vscpEventEx eventEx;
            if ( m_pCtrlObject->m_automation.doWork( &eventEx ) ) {

                if ( m_pCtrlObject->m_debugFlags1 & VSCP_DEBUG1_AUTOMATION ) {
                    m_pCtrlObject->logMsg( wxString::Format( _("Automation event sent: "
                                                               "Class=%d Type=%d\n" ),
                                        eventEx.vscp_class, eventEx.vscp_type ),
                                        DAEMON_LOGMSG_DEBUG,
                                        DAEMON_LOGTYPE_GENERAL );
                }

                // Yes event should be sent
                eventEx.obid = pClientItem->m_clientID;
                pClientItem->m_guid.writeGUID( eventEx.GUID );

                if ( ( VSCP_CLASS1_PROTOCOL == eventEx.vscp_class ) &&
                     ( VSCP_TYPE_PROTOCOL_SEGCTRL_HEARTBEAT == eventEx.vscp_type ) ) {

                    // crc8 of VSCP daemon GUID should be indata byte 0
                    eventEx.data[ 0 ] = vscp_calcCRC4GUIDArray( m_pCtrlObject->m_guid.getGUID() );

                    // Send event on multicast information channel
                    sendMulticastEventEx( sock_mc, &eventEx );

                }
                else if ( ( VSCP_CLASS1_INFORMATION  == eventEx.vscp_class ) &&
                          ( VSCP_TYPE_INFORMATION_NODE_HEARTBEAT == eventEx.vscp_type ) ) {

                    // Send event on multicast information channel
                    sendMulticastEventEx( sock_mc, &eventEx );

                }
                else if ( ( VSCP_CLASS2_INFORMATION == eventEx.vscp_class ) &&
                          ( VSCP2_TYPE_INFORMATION_HEART_BEAT == eventEx.vscp_type ) ) {

                    // Send event on multicast information channel
                    sendMulticastEventEx( sock_mc, &eventEx );

                }
                else if ( ( VSCP_CLASS2_PROTOCOL == eventEx.vscp_class ) &&
                          ( VSCP2_TYPE_PROTOCOL_HIGH_END_SERVER_CAPS == eventEx.vscp_type ) ) {
                    // Send event on multicast information channel
                    sendMulticastEventEx( sock_mc, &eventEx );
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

        ///////////////////////////////////////////////////////////////////////
        //                          Input queue
        ///////////////////////////////////////////////////////////////////////

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
                        
                        pnewEvent->timestamp = vscp_makeTimeStamp();
                        vscp_setEventDateTimeBlockToNow( pnewEvent );
                        pnewEvent->obid = pClientItem->m_clientID;
                        pnewEvent->head = 0;
                        pnewEvent->vscp_class = 0;
                        pnewEvent->vscp_type = 28;
                        pnewEvent->sizeData = 8;

                        pClientItem->m_guid.writeGUID(pnewEvent->GUID);

                        pnewEvent->pdata = new unsigned char[ 8 ];
                        if ( NULL != pnewEvent->pdata ) {
                            pnewEvent->pdata[ 0 ] = VSCP_DAEMON_SERVER_CAPABILITIES_7;
                            pnewEvent->pdata[ 1 ] = VSCP_DAEMON_SERVER_CAPABILITIES_6;
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

            // New node on-line - collect
            else if ( ( VSCP_CLASS1_PROTOCOL == pEvent->vscp_class ) &&
                    ( VSCP_TYPE_PROTOCOL_NEW_NODE_ONLINE == pEvent->vscp_type ) ) {

                // Add node to knows nodes or return info if known
                CVSCPNode *pNode = addNodeIfNotKnown( pEvent );

                if ( NULL != pNode ) {

                    // Send multi cast information
                    sendMulticastInformationProxyEvent( sock_mc, pNode );

                }

            }

            // Level I node heart beat - collect
            else if ( ( VSCP_CLASS1_INFORMATION == pEvent->vscp_class ) &&
                    ( VSCP_TYPE_INFORMATION_NODE_HEARTBEAT == pEvent->vscp_type ) ) {

                // Add node to knows nodes or return info if known
                CVSCPNode *pNode = addNodeIfNotKnown( pEvent );

                if ( NULL != pNode ) {

                    // Send multi cast information
                    sendMulticastInformationProxyEvent( sock_mc, pNode );

                }

                /*cguid guid;      TODO
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
                            pdiscoverNode->clientId = pEvent->obid;     // Clientid
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

            }

            // Level II node heart beat   - collect
            else if ( ( VSCP_CLASS2_INFORMATION == pEvent->vscp_class ) &&
                    ( VSCP_TYPE_INFORMATION_NODE_HEARTBEAT == pEvent->vscp_type ) ) {

                // Send event on multicast information channel
                sendMulticastEvent( sock_mc, pEvent );

                // Add node to knows nodes or return info if known
                CVSCPNode *pNode = addNodeIfNotKnown( pEvent );

                if ( NULL != pNode ) {

                    // Send multi cast information
                    sendMulticastInformationProxyEvent( sock_mc, pNode );

                }

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

CVSCPNode * daemonVSCPThread::addNodeIfNotKnown( vscpEvent *pEvent )
{
    CVSCPNode *pNode;
    wxString strGUID;
    cguid guid;

    // Check pointer
    if ( NULL == pEvent ) return NULL;
    
    return NULL;

    guid.getFromArray( pEvent->GUID );
    guid.toString( strGUID );

    m_pCtrlObject->m_mutexKnownNodes.Lock();
    pNode = m_pCtrlObject->m_knownNodes.m_nodes[ strGUID ];
    m_pCtrlObject->m_mutexKnownNodes.Unlock();

    if ( NULL == pNode ) {

        // We have not seen this node before and we will
        // try to collect information from it
        pNode = new CVSCPNode;
        if ( NULL != pNode ) {

            // Add the node to known nodes
            m_pCtrlObject->m_mutexKnownNodes.Lock();
            m_pCtrlObject->m_knownNodes.m_nodes[ strGUID ] = pNode;
            m_pCtrlObject->m_mutexKnownNodes.Unlock();

            // Is there a device related to this node
            m_pCtrlObject->m_mutexDeviceList.Lock();
            CDeviceItem *pDeviceItem =
                m_pCtrlObject->m_deviceList.getDeviceItemFromClientId( pEvent->obid );
            m_pCtrlObject->m_mutexDeviceList.Unlock();
            
            if ( NULL != pDeviceItem ) {

                m_pCtrlObject->m_mutexDeviceList.Lock();
                
                pNode->m_level = pDeviceItem->m_driverLevel;

                // Construct a device name if no device name set
                // 'devicename_clientid'
                if ( pNode->m_deviceName.IsEmpty() ) {
                    pNode->m_deviceName = pDeviceItem->m_strName;
                    pNode->m_deviceName += _( "_" );
                    pNode->m_deviceName += wxString::Format( _( "%u" ),
                                             pDeviceItem->m_pClientItem->m_clientID );
                }

                // Save interface
                pNode->m_interfaceguid = pDeviceItem->m_interface_guid;

                // Save Client ID
                pNode->m_clientID = pEvent->obid;

                // We set name from client id
                // 'client_clientid_nickname'
                if ( pNode->m_strNodeName.IsEmpty() ) {
                    pNode->m_strNodeName = _( "client_" );
                    uint32_t clientID = pDeviceItem->m_pClientItem->m_clientID;
                    pNode->m_strNodeName += wxString::Format( _( "%u" ),
                                                                clientID );
                    pNode->m_strNodeName += _( "_" );
                    pNode->m_strNodeName += wxString::Format( _( "%u" ),
                                                ( pEvent->GUID[ 14 ] << 8 ) +
                                                    pEvent->GUID[ 15 ] );
                } 
                
                m_pCtrlObject->m_mutexDeviceList.Unlock();

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

                // We set name from client id
                // 'client_clientid_nickname'
                if ( pNode->m_strNodeName.IsEmpty() ) {
                    pNode->m_strNodeName = _( "client_" );
                    if ( NULL != pClientItem ) {
                        pNode->m_strNodeName +=                                            
                            wxString::Format( _( "%lu" ),
                                                (unsigned long)pClientItem->m_clientID );
                    }
                    else {
                        pNode->m_strNodeName += _("isNULL");
                    }
                    pNode->m_strNodeName += _( "_" );
                    pNode->m_strNodeName +=
                        wxString::Format( _( "%u" ),
                                            ( pEvent->GUID[ 14 ] << 8 ) +
                                                pEvent->GUID[ 15 ] );
                }

            }
            
        }

    }
    else {

        // This node is known so we don't need to do a thing. Hurray! A la

    }

    return pNode;
}


///////////////////////////////////////////////////////////////////////////////
// parseInterface
//

bool daemonVSCPThread::parseInterface( const wxString &ifaddr, 
                                            wxString &ip, 
                                            unsigned short *pPort )
{
    // Check pointer
    if ( NULL == pPort ) return false;
    
    wxString strAddress = ifaddr;
    strAddress.Trim();
    strAddress.MakeLower();
    
    // If no data return defaults
    if ( 0 == strAddress.Length() ) {
        ip = "";  // INADDR_ANY
        *pPort = VSCP_ANNOUNCE_MULTICAST_PORT;
        return true;
    }
    
    if ( !strAddress.StartsWith(_("udp://"), &strAddress ) ) {
        return false;
    }
    
    // Check for a.b.c.d;port
    int pos;
    if ( ( wxNOT_FOUND != ( pos = strAddress.Find(':') ) ) && 
           ( wxNOT_FOUND != strAddress.Find('.') )  ) {
        ip = strAddress.Left( pos );
        *pPort = vscp_readStringValue( strAddress.Right( strAddress.Length() - pos - 1 ) );
    }
    // Check for a.b.c.d  default port used
    else if ( ( wxNOT_FOUND != ( pos = strAddress.Find('.') ) ) ) {
        ip = strAddress;
        *pPort = VSCP_ANNOUNCE_MULTICAST_PORT;
    }
    // only port
    else {
        ip = "";  // INADDR_ANY
        *pPort = vscp_readStringValue( strAddress );
    }
            
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// sendMulticastEvent
//

bool daemonVSCPThread::sendMulticastEvent( int sock_mc,
                                            vscpEvent *pEvent )
{
    struct sockaddr_in mc_addr;     // socket address structure
    uint8_t buf[ 1024 ];            // frame to send
    
    // Check pointer
    if ( NULL == pEvent ) return false;

    // construct a multicast address structure
    memset( &mc_addr, 0, sizeof( mc_addr ) );
    mc_addr.sin_family = AF_INET;
    mc_addr.sin_addr.s_addr = inet_addr( VSCP_MULTICAST_IPV4_ADDRESS_STR );
    mc_addr.sin_port = htons( VSCP_ANNOUNCE_MULTICAST_PORT );

    // Clear buffer
    memset( buf, 0, sizeof( buf ) );
    
    // Wite data to buf
    if ( !vscp_writeEventToUdpFrame( buf, 
                                        sizeof(buf), 
                                        0, 
                                        pEvent ) ) {
        return false;
    }
    
    return ( ( 1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + pEvent->sizeData + 2 ) ==
             sendto( sock_mc,
                     ( const char * )buf,
                     1 + VSCP_MULTICAST_PACKET0_HEADER_LENGTH + pEvent->sizeData + 2,
                     0,
                     ( struct sockaddr * )&mc_addr,
                     sizeof( mc_addr ) ) );
}


///////////////////////////////////////////////////////////////////////////////
// sendMulticastEventEx
//

bool daemonVSCPThread::sendMulticastEventEx( int sock,
                                                vscpEventEx *pEventEx )
{
    bool rv;
    vscpEvent *pEvent = new vscpEvent;

    if ( NULL == pEvent ) return false;

    vscp_convertVSCPfromEx( pEvent, pEventEx );
    rv = sendMulticastEvent( sock, pEvent );
    vscp_deleteVSCPevent( pEvent );
    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// sendMulticastInformationProxyEvent
//

bool daemonVSCPThread::sendMulticastInformationProxyEvent( int sock,
                                                            CVSCPNode *pNode )
{
    struct sockaddr_in mc_addr;     // socket address structure
    uint8_t buf[ 1024 ];            // frame to send
    vscpEventEx ex;
    
    // Check pointer
    if ( NULL == pNode ) return false;

    // construct a multicast address structure
    memset( &mc_addr, 0, sizeof( mc_addr ) );
    mc_addr.sin_family = AF_INET;
    mc_addr.sin_addr.s_addr = inet_addr( VSCP_MULTICAST_IPV4_ADDRESS_STR );
    mc_addr.sin_port = htons( VSCP_ANNOUNCE_MULTICAST_PORT );

    // Clear buffer
    memset( buf, 0, sizeof( buf ) );
    
    ex.head = 0;
    ex.obid = 0;
    ex.timestamp = 0;
    ex.year = 0;
    ex.month= 0;
    ex.day = 0;
    ex.hour=0;
    ex.minute=0;
    ex.second=0;
    ex.vscp_class = VSCP_CLASS2_INFORMATION;
    ex.vscp_type = VSCP2_TYPE_INFORMATION_PROXY_HEART_BEAT;
    ex.sizeData = VSCP_MULTICAST_PROXY_HEARTBEAT_DATA_SIZE;
    
    // Originating GUID - Daemon GUID
    memcpy( ex.GUID, m_pCtrlObject->m_guid.m_id, 16 );
    
    //  Data
    //  ====================================================================================
    //  0-15 	    Real GUID for node (not interface GUID)
    //  16-31 	    Reserved
    //  32-47 	    GUID for interface on server the node is connected to.
    //  48 	        Interface Level( 0 == Level I, 1 == Level II )
    //  49-63 	    Reserved
    //  64-127 	    Real text name of node( if any ).Set to all zero if not available.
    //  128-191     Real text name of interface ( if any ).Set to all zero if not available.
    
    // Real GUID of node
    memcpy( ex.data,
                pNode->m_realguid.getGUID(),
                16 );

    // Interface GUID of node
    memcpy( ex.data + VSCP_MULTICAST_PROXY_HEARTBEAT_POS_IFGUID,
                pNode->m_interfaceguid.getGUID(),
                16 );

    ex.data[ VSCP_MULTICAST_PROXY_HEARTBEAT_POS_IFLEVEL  ] = pNode->m_level;

    // Name of node
    memcpy( ex.data + VSCP_MULTICAST_PROXY_HEARTBEAT_POS_NODENAME,
                pNode->m_strNodeName.mbc_str(),
                MIN( 64, strlen( pNode->m_strNodeName.mbc_str() ) ) );

    // Name of interface node is on
    memcpy( ex.data + VSCP_MULTICAST_PROXY_HEARTBEAT_POS_IFNAME,
            pNode->m_deviceName.mbc_str(),
            MIN( 64, strlen( pNode->m_deviceName.mbc_str() ) ) );
    
    // Wite data to buf
    if ( !vscp_writeEventExToUdpFrame( buf, 
                                        sizeof(buf), 
                                        0, 
                                        &ex ) ) {
        return false;
    }

    return ( ( VSCP_MULTICAST_PACKET0_HEADER_LENGTH +
                    VSCP_MULTICAST_PROXY_HEARTBEAT_DATA_SIZE + 2 ) ==
             sendto( sock,
                        (const char *)buf,
                        VSCP_MULTICAST_PACKET0_HEADER_LENGTH +
                            VSCP_MULTICAST_PROXY_HEARTBEAT_DATA_SIZE + 2,
                        0,
                        ( struct sockaddr * )&mc_addr,
                        sizeof( mc_addr ) ) );
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
    m_pClientItem->m_strDeviceName = _("VSCP node discovery thread.|Started at ");
    wxDateTime now = wxDateTime::Now();
    m_pClientItem->m_strDeviceName += now.FormatISODate();
    m_pClientItem->m_strDeviceName += _(" ");
    m_pClientItem->m_strDeviceName += now.FormatISOTime();

    // Add the client to the Client List
    m_pCtrlObject->m_wxClientMutex.Lock();
    if ( !m_pCtrlObject->addClient ( m_pClientItem, CLIENT_ID_DAEMON_WORKER ) ) {
        // Failed to add client
        delete m_pClientItem;
        m_pClientItem = NULL;
        m_pCtrlObject->m_wxClientMutex.Unlock();
        m_pCtrlObject->logMsg( _("Deamon discovery: Failed to add client. Terminating thread.") );        
        return NULL;
    }
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
        }

        // Check if we failed
        if ( error >= 3 ) {
            goto error_abort;
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

    pEvent->timestamp = 0;
    vscp_setEventDateTimeBlockToNow( pEvent );
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

    std::list<CClientItem*>::iterator it;
    for (it = m_pCtrlObject->m_clientList.m_clientItemList.begin();
            it != m_pCtrlObject->m_clientList.m_clientItemList.end();
            ++it) {

        CClientItem *pItem = *it;

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