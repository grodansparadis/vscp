// daemonworker.cpp
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

#define _POSIX

#include <string>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <vscp_debug.h>
#include <crc.h>
#include <canal_win32_ipc.h>
#include <canal_macro.h>
#include <vscp.h>
#include <vscphelper.h>
#include <clientlist.h>
#include <dllist.h>
#include <crc8.h>
#include <controlobject.h>
#include <guid.h>
#include <vscpdatetime.h>
#include <vscpd_caps.h>

#include "daemonworker.h"

///////////////////////////////////////////////////////////////////////////////
// daemonWorkerObj
//
// This thread listens for connection on a TCP socket and starts a new thread
// to handle client requests
//

daemonWorkerObj::daemonWorkerObj( CControlObject *pObj )
{
    m_bQuit = false;
    m_pCtrlObject = pObj;
}


daemonWorkerObj::~daemonWorkerObj()
{
    ;
}


///////////////////////////////////////////////////////////////////////////////
// Daemon worker thread
//

void *daemonWorkerThread( void *threadData )
{
    int sock_mc;                    // socket descriptor for multicast socket
    struct sockaddr_in mc_addr;     // socket address structure
    unsigned short mc_port = VSCP_ANNOUNCE_MULTICAST_PORT;


    daemonWorkerObj *pWorkerObj = (daemonWorkerObj *)threadData;
    if ( NULL == pWorkerObj ) {
        syslog( LOG_ERR, "daemonWorkerThread - No object supplied. Terminating.");
        return NULL;
    }

    // Must have a valid pointer to the control object
    if ( NULL == pWorkerObj->m_pCtrlObject ) return NULL;
    CControlObject *pctrlObj = pctrlObj;    // shortcut

    unsigned char mc_ttl =
        pctrlObj->m_ttlMultiCastAnnounce; // time to live (hop count) 
    std::string ip;
    
    if ( !pWorkerObj->parseInterface( pctrlObj->m_strMulticastAnnounceAddress, 
                                        ip, 
                                        &mc_port ) ) {
        syslog( LOG_ERR, "daemonWorkerThread - Announce address has wrong format.\n" );
        return NULL;
    }
            
    // create a socket for sending to the VSCP multicast address
    if ( ( sock_mc = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) < 0 ) {
        syslog( LOG_ERR, "daemonWorkerThread - %s ", strerror(errno) );
        return NULL;
    }

    // set the TTL (time to live/hop count) for the send
    if ( ( setsockopt( sock_mc, IPPROTO_IP, IP_MULTICAST_TTL,
                       ( void* )&mc_ttl, sizeof( mc_ttl ) ) ) < 0 ) {
        syslog( LOG_ERR, "daemonWorkerThread - %s ", strerror(errno) );
        return NULL;
    }
    
    int one = 1;
    setsockopt( sock_mc, SOL_SOCKET, SO_REUSEADDR, &one, sizeof( one ) );

    // construct a multicast address structure
    memset( &mc_addr, 0, sizeof( mc_addr ) );
    mc_addr.sin_family = AF_INET;
    mc_addr.sin_addr.s_addr = inet_addr( VSCP_MULTICAST_IPV4_ADDRESS_STR );
    mc_addr.sin_port = htons( VSCP_ANNOUNCE_MULTICAST_PORT );


    // We need to create a client item and add this object to the list
    // of clients
    CClientItem *pClientItem = new CClientItem;
    if ( NULL == pClientItem ) return NULL;

    // This is an active client
    pClientItem->m_bOpen = true;
    pClientItem->m_type =  CLIENT_ITEM_INTERFACE_TYPE_CLIENT_INTERNAL;
    pClientItem->m_strDeviceName = "Internal VSCP Daemon Worker Client.";    
    pClientItem->m_strDeviceName += "|Started at ";
    pClientItem->m_strDeviceName += vscpdatetime::setNow().getISODateTime(); 

    // Add the client to the Client List
    pthread_mutex_lock( &pctrlObj->m_clientMutex );
    if ( !pctrlObj->addClient( pClientItem, CLIENT_ID_DAEMON_WORKER ) ) {
        // Failed to add client
        delete pClientItem;
        pthread_mutex_unlock( &pctrlObj->m_clientMutex );
        syslog( LOG_ERR, "daemonWorkerThread - Failed to add client. Terminating thread." );
        return NULL;
    }
    pthread_mutex_unlock( &pctrlObj->m_clientMutex );

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


    //CLIENTEVENTLIST::compatibility_iterator nodeClient;
    while ( !pWorkerObj->m_bQuit ) {

        // Automation
        if (  pctrlObj->m_automation.isAutomationEnabled() ) {

            // Check if automation event should be sent and send it if
            // that is the case
            
            vscpEventEx eventEx;
            if ( pctrlObj->m_automation.doWork( &eventEx ) ) {

                if ( ( pctrlObj->m_logLevel >= DAEMON_LOGMSG_DEBUG ) &&
                     ( VSCP_DEBUG1_AUTOMATION | pctrlObj->m_debugFlags1 ) ) {
                    syslog( LOG_DEBUG,  "Automation event sent: "
                                        "Class=%d Type=%d\n",
                                        eventEx.vscp_class, eventEx.vscp_type );
                }

                // Yes event should be sent
                eventEx.obid = pClientItem->m_clientID;
                pClientItem->m_guid.writeGUID( eventEx.GUID );

                if ( ( VSCP_CLASS1_PROTOCOL == eventEx.vscp_class ) &&
                     ( VSCP_TYPE_PROTOCOL_SEGCTRL_HEARTBEAT == eventEx.vscp_type ) ) {

                    // crc8 of VSCP daemon GUID should be indata byte 0
                    eventEx.data[ 0 ] = vscp_calcCRC4GUIDArray( pctrlObj->m_guid.getGUID() );

                    // Send event on multicast information channel
                    pWorkerObj->sendMulticastEventEx( sock_mc, &eventEx );

                }
                else if ( ( VSCP_CLASS1_INFORMATION  == eventEx.vscp_class ) &&
                          ( VSCP_TYPE_INFORMATION_NODE_HEARTBEAT == eventEx.vscp_type ) ) {

                    // Send event on multicast information channel
                    pWorkerObj->sendMulticastEventEx( sock_mc, &eventEx );

                }
                else if ( ( VSCP_CLASS2_INFORMATION == eventEx.vscp_class ) &&
                          ( VSCP2_TYPE_INFORMATION_HEART_BEAT == eventEx.vscp_type ) ) {

                    // Send event on multicast information channel
                    pWorkerObj->sendMulticastEventEx( sock_mc, &eventEx );

                }
                else if ( ( VSCP_CLASS2_PROTOCOL == eventEx.vscp_class ) &&
                          ( VSCP2_TYPE_PROTOCOL_HIGH_END_SERVER_CAPS == eventEx.vscp_type ) ) {
                    // Send event on multicast information channel
                    pWorkerObj->sendMulticastEventEx( sock_mc, &eventEx );
                }

                vscpEvent *pnewEvent = new vscpEvent;
                if ( NULL != pnewEvent ) {

                    // Convert event to correct format
                    vscp_convertVSCPfromEx( pnewEvent, &eventEx );

                    // Statistics
                    pClientItem->m_statistics.cntTransmitData += eventEx.sizeData;
                    pClientItem->m_statistics.cntTransmitFrames++;

                    // There must be room in the send queue
                    if ( pctrlObj->m_maxItemsInClientReceiveQueue >
                                pctrlObj->m_clientOutputQueue.size() ) {
                        pthread_mutex_lock( &pctrlObj->m_mutexClientOutputQueue );
                        pctrlObj->m_clientOutputQueue.push_back( pnewEvent );
                        sem_post( &pctrlObj->m_semClientOutputQueue );
                        pthread_mutex_unlock( &pctrlObj->m_mutexClientOutputQueue );
                    }
                }
            }
        }

        ///////////////////////////////////////////////////////////////////////
        //                          Input queue
        ///////////////////////////////////////////////////////////////////////

        int rv = pClientItem->m_clientInputQueue.size();
        
        // Wait for incoming event
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 100000;    //100 ms
        if ( ETIMEDOUT == sem_timedwait( &pClientItem->m_semClientInputQueue,
                                            &ts ) ) {
            continue;
        }

        if ( pClientItem->m_clientInputQueue.size() ) {

            pthread_mutex_lock( &pClientItem->m_mutexClientInputQueue );
            vscpEvent *pEvent = pClientItem->m_clientInputQueue.front();
            pClientItem->m_clientInputQueue.pop_front();
            pthread_mutex_unlock( &pClientItem->m_mutexClientInputQueue );

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
                        if ( pctrlObj->m_maxItemsInClientReceiveQueue >
                                pctrlObj->m_clientOutputQueue.size() ) {
                            pthread_mutex_lock( &pctrlObj->m_mutexClientOutputQueue );
                            pctrlObj->m_clientOutputQueue.push_back ( pnewEvent );
                            sem_post( &pctrlObj->m_semClientOutputQueue );
                            pthread_mutex_unlock( &pctrlObj->m_mutexClientOutputQueue );
                        }
                    }

                } // for each server address

            }

            // New node on-line - collect
            else if ( ( VSCP_CLASS1_PROTOCOL == pEvent->vscp_class ) &&
                    ( VSCP_TYPE_PROTOCOL_NEW_NODE_ONLINE == pEvent->vscp_type ) ) {

                // Add node to knows nodes or return info if known
                cvscpnode *pNode = pWorkerObj->m_pDiscoveryObj->addNodeIfNotKnown( pEvent );

                if ( NULL != pNode ) {
                    // Send multi cast information
                    pWorkerObj->sendMulticastInformationProxyEvent( sock_mc, pNode );
                }

            }

            // Level I node heart beat - collect
            else if ( ( VSCP_CLASS1_INFORMATION == pEvent->vscp_class ) &&
                    ( VSCP_TYPE_INFORMATION_NODE_HEARTBEAT == pEvent->vscp_type ) ) {

                // Add node to knows nodes or return info if known
                cvscpnode *pNode = pWorkerObj->m_pDiscoveryObj->addNodeIfNotKnown( pEvent );

                if ( NULL != pNode ) {
                    // Send multi cast information
                    pWorkerObj->sendMulticastInformationProxyEvent( sock_mc, pNode );
                }

                /*cguid guid;      TODO
                std::string str;
                guid.getFromArray( pEvent->GUID );
                guid.toString( str );
                if ( wxNOT_FOUND != m_knownGUIDs.Index( str, false ) ) {

                    // We have not seen this node before and we will
                    // try to collect information from it
                    cguid guid;
                    guid.getFromArray( pEvent->GUID );
                    std::string str;
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
                            pdiscoverNode->pThread = new CDiscoveryObj();
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
                pWorkerObj->sendMulticastEvent( sock_mc, pEvent );

                // Add node to knows nodes or return info if known
                cvscpnode *pNode = pWorkerObj->m_pDiscoveryObj->addNodeIfNotKnown( pEvent );

                if ( NULL != pNode ) {

                    // Send multi cast information
                    pWorkerObj->sendMulticastInformationProxyEvent( sock_mc, pNode );

                }

            }

            // Remove the event
            vscp_deleteVSCPevent( pEvent );

        } // event in queue

    } // while

    // Remove messages in the client queues
    pthread_mutex_lock( &pctrlObj->m_clientMutex );
    pctrlObj->removeClient( pClientItem );
    pthread_mutex_unlock( &pctrlObj->m_clientMutex );

    // Close the multicast socket
    close( sock_mc );

    return NULL;
}





///////////////////////////////////////////////////////////////////////////////
// parseInterface
//

bool daemonWorkerObj::parseInterface( const std::string &ifaddr, 
                                            std::string &ip, 
                                            unsigned short *pPort )
{
    // Check pointer
    if ( NULL == pPort ) return false;
    
    std::string strAddress = ifaddr;
    vscp_trim( strAddress );
    vscp_makeLower( strAddress );
    
    // If no data return defaults
    if ( 0 == strAddress.length() ) {
        ip = "";  // INADDR_ANY
        *pPort = VSCP_ANNOUNCE_MULTICAST_PORT;
        return true;
    }
    
    if ( !vscp_startsWith( strAddress, "udp://", &strAddress ) ) {
        return false;
    }
    
    // Check for a.b.c.d;port
    int pos;
    if ( ( strAddress.npos != ( pos = strAddress.find(':') ) ) && 
           ( strAddress.npos != strAddress.find('.') )  ) {
        ip = strAddress.substr( pos );
        *pPort = vscp_readStringValue( strAddress.substr( strAddress.length() - pos - 1 ) );
    }
    // Check for a.b.c.d  default port used
    else if ( ( strAddress.npos != ( pos = strAddress.find('.') ) ) ) {
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

bool daemonWorkerObj::sendMulticastEvent( int sock_mc,
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
    
    // Write data to buf
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

bool daemonWorkerObj::sendMulticastEventEx( int sock,
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

bool daemonWorkerObj::sendMulticastInformationProxyEvent( int sock,
                                                            cvscpnode *pNode )
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
                pNode->m_strNodeName.c_str(),
                MIN( 64, strlen( pNode->m_strNodeName.c_str() ) ) );

    // Name of interface node is on
    memcpy( ex.data + VSCP_MULTICAST_PROXY_HEARTBEAT_POS_IFNAME,
            pNode->m_deviceName.c_str(),
            MIN( 64, strlen( pNode->m_deviceName.c_str() ) ) );
    
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





