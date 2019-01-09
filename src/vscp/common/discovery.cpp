// discovery.cpp
//
// This file is part of the VSCP (http://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2019 Ake Hedman, Grodans Paradis AB
// <info@grodansparadis.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
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

#include <deque>
#include <list>
#include <string>

#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

#include <canal_macro.h>
#include <canal_win32_ipc.h>
#include <clientlist.h>
#include <controlobject.h>
#include <crc.h>
#include <crc8.h>
#include <daemonworker.h>
#include <dllist.h>
#include <guid.h>
#include <knownnodes.h>
#include <vscp.h>
#include <vscp_debug.h>
#include <vscpd_caps.h>
#include <vscpdatetime.h>
#include <vscphelper.h>

#include "discovery.h"

///////////////////////////////////////////////////////////////////////////////
//                              Discovery
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CDiscoveryObj
//

CDiscoveryObj::CDiscoveryObj()
{
    m_bQuit       = false;
    m_nodeid      = 0;
    m_clientID    = 0;
    m_pClientItem = NULL;
    m_pCtrlObject = NULL;
}

CDiscoveryObj::~CDiscoveryObj()
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
// threadDiscovery
//

void *
threadDiscovery(void *data)
{
    CDiscoveryObj *pObj = (CDiscoveryObj *)data;

    // Must have a valid pointer to the control object
    if (NULL == pObj->m_pCtrlObject) return NULL;

    // We need to create a clientobject and add this object to the list
    pObj->m_pClientItem = new CClientItem;
    if (NULL == pObj->m_pClientItem) return NULL;

    // This is an active client
    pObj->m_pClientItem->m_bOpen = true;
    pObj->m_pClientItem->m_type  = CLIENT_ITEM_INTERFACE_TYPE_CLIENT_INTERNAL;
    pObj->m_pClientItem->m_strDeviceName =
      "VSCP node discovery thread.|Started at ";
    pObj->m_pClientItem->m_strDeviceName +=
      vscpdatetime::setNow().getISODateTime();

    // Add the client to the Client List
    pthread_mutex_lock(&pObj->m_pCtrlObject->m_clientMutex);
    if (!pObj->m_pCtrlObject->addClient(pObj->m_pClientItem,
                                        CLIENT_ID_DAEMON_WORKER)) {
        // Failed to add client
        delete pObj->m_pClientItem;
        pObj->m_pClientItem = NULL;
        pthread_mutex_unlock(&pObj->m_pCtrlObject->m_clientMutex);
        syslog(LOG_ERR,
               "Deamon discovery: Failed to add client. Terminating thread.");
        return NULL;
    }
    pthread_mutex_unlock(&pObj->m_pCtrlObject->m_clientMutex);

    // Read GUID of node
    cguid newguid;
    uint8_t content;
    for (int i = 0xd0; i < 0xe0; i++) {

        int error; // Resend errors
        for (error = 0; error < 3; error++) {
            if (pObj->readLevel1Register(
                  pObj->m_nodeid, i, &content, pObj->m_clientID, 1000)) {
                newguid.setAt(i, content);
                break;
            }
        }

        // Check if we failed
        if (error >= 3) {
            goto error_abort;
        }
    }

error_abort:

    // Remove messages in the client queues
    pthread_mutex_lock(&pObj->m_pCtrlObject->m_clientMutex);
    pObj->m_pCtrlObject->removeClient(pObj->m_pClientItem);
    pthread_mutex_unlock(&pObj->m_pCtrlObject->m_clientMutex);

    return NULL;
}

//////////////////////////////////////////////////////////////////////////////
// readLevel1Register
//

bool
CDiscoveryObj::readLevel1Register(uint8_t nodeid,
                                  uint8_t reg,
                                  uint8_t *pcontent,
                                  uint32_t clientID,
                                  uint32_t timeout)
{
    bool rv = true;
    std::string strBuf;

    // Check pointer
    if (NULL == pcontent) return false;

    // New event
    vscpEvent *pEvent = new vscpEvent;
    if (NULL == pEvent) return false;

    pEvent->timestamp = 0;
    vscp_setEventDateTimeBlockToNow(pEvent);
    pEvent->vscp_class = VSCP_CLASS1_PROTOCOL;
    pEvent->vscp_type  = VSCP_TYPE_PROTOCOL_READ_REGISTER;
    pEvent->sizeData   = 2;

    // Allocate data
    pEvent->pdata = new uint8_t[2];
    if (NULL == pEvent->pdata) return false;

    pEvent->pdata[0] = nodeid; // Node to read from
    pEvent->pdata[1] = reg;    // Register to read

    // Send event
    sendEvent(pEvent, clientID);

    uint32_t startTime = vscp_getMsTimeStamp();

    while (true) {

        // Check for read error timeout
        if ((vscp_getMsTimeStamp() - startTime) > timeout) {
            return false;
        }

        // Wait for incoming event
        if ((-1 == vscp_sem_wait(&m_pClientItem->m_semClientInputQueue, 500)) &&
            errno == ETIMEDOUT) {
            continue;
        }

        if (m_pClientItem->m_clientInputQueue.size()) {

            pthread_mutex_lock(&m_pClientItem->m_mutexClientInputQueue);
            vscpEvent *pEvent = m_pClientItem->m_clientInputQueue.front();
            m_pClientItem->m_clientInputQueue.pop_front();
            pthread_mutex_unlock(&m_pClientItem->m_mutexClientInputQueue);

            // Check if this is a response to our register read
            if ((VSCP_CLASS1_PROTOCOL == pEvent->vscp_class) &&
                (VSCP_TYPE_PROTOCOL_RW_RESPONSE == pEvent->vscp_type &&
                 (nodeid == pEvent->GUID[15]))) {
                if ((2 == pEvent->sizeData) && (reg == pEvent->pdata[1])) {
                    // We have a response
                    *pcontent = pEvent->pdata[1];
                }
            }

            vscp_deleteVSCPevent(pEvent);
        }

    } // while

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// sendEvent
//

bool
CDiscoveryObj::sendEvent(vscpEvent *pEvent, uint32_t obid)
{
    bool bSent = false;

    // Find client
    pthread_mutex_lock(&m_pCtrlObject->m_clientMutex);

    std::list<CClientItem *>::iterator it;
    for (it = m_pCtrlObject->m_clientList.m_itemList.begin();
         it != m_pCtrlObject->m_clientList.m_itemList.end();
         ++it) {

        CClientItem *pItem = *it;

        if (pItem->m_clientID == obid) {
            // Found
            bSent = true;
            m_pCtrlObject->sendEventToClient(pItem, pEvent);
            break;
        }
    }

    pthread_mutex_unlock(&m_pCtrlObject->m_clientMutex);

    return bSent;
}

///////////////////////////////////////////////////////////////////////////////
// addNodeIfNotKnown
//

cvscpnode *
CDiscoveryObj::addNodeIfNotKnown(vscpEvent *pEvent)
{
    cvscpnode *pNode;
    std::string strGUID;
    cguid guid;
    /*
        // Check pointer
        if ( NULL == pEvent ) return NULL;

        return NULL;

        guid.getFromArray( pEvent->GUID );
        guid.toString( strGUID );

        pthread_mutex_lock( &m_pCtrlObject->m_mutexKnownNodes );
        pNode = m_pCtrlObject->m_knownNodes[ strGUID ];
        pthread_mutex_lock( &m_pCtrlObject->m_mutexKnownNodes );

        if ( NULL == pNode ) {

            // We have not seen this node before and we will
            // try to collect information from it
            pNode = new cvscpnode;
            if ( NULL != pNode ) {

                // Add the node to known nodes
                pthread_mutex_lock( &m_pCtrlObject->m_mutexKnownNodes );
                m_pCtrlObject->m_knownNodes[ strGUID ] = pNode;
                pthread_mutex_unlock( &m_pCtrlObject->m_mutexKnownNodes );

                // Is there a device related to this node
                pthread_mutex_lock( &m_pCtrlObject->m_mutexDeviceList );
                // TODO
                CDeviceItem *pDeviceItem; // =
                //    m_pCtrlObject->m_deviceList.getDeviceItemFromClientId(
       pEvent->obid ); pthread_mutex_unlock( &m_pCtrlObject->m_mutexDeviceList
       );

                if ( NULL != pDeviceItem ) {

                    pthread_mutex_lock( &m_pCtrlObject->m_mutexDeviceList );

                    TODO
                    pNode->m_level = pDeviceItem->m_driverLevel;

                    // Construct a device name if no device name set
                    // 'devicename_clientid'
                    if ( pNode->m_deviceName.empty() ) {
                        pNode->m_deviceName = pDeviceItem->m_strName;
                        pNode->m_deviceName += "_";
                        pNode->m_deviceName += vscp_str_format( "%u",
                                                 pDeviceItem->m_pClientItem->m_clientID
       );
                    }

                    // Save interface
                    pNode->m_interfaceguid = pDeviceItem->m_interface_guid;

                    // Save Client ID
                    pNode->m_clientID = pEvent->obid;

                    // We set name from client id
                    // 'client_clientid_nickname'
                    if ( pNode->m_strNodeName.empty() ) {
                        pNode->m_strNodeName = "client_";
                        uint32_t clientID =
       pDeviceItem->m_pClientItem->m_clientID; pNode->m_strNodeName +=
       vscp_str_format( "%u", clientID ); pNode->m_strNodeName += "_";
                        pNode->m_strNodeName += vscp_str_format( "%u",
                                                    ( pEvent->GUID[ 14 ] << 8 )
       + pEvent->GUID[ 15 ] );
                    }

                    pthread_mutex_unlock( &m_pCtrlObject->m_mutexDeviceList );

                    // Now let the discovery thread do the rest of the work

                }
                else {

                    // No device associated with this event - but there must be
       a client

                    TODO
                    // Save Client ID
                    pNode->m_clientID = pEvent->obid;

                    pthread_mutex_lock( &m_pCtrlObject->m_mutexDeviceList );
                    // TODO
                    //CClientItem *pClientItem =
                    //    m_pCtrlObject->m_clientList.getClientFromId(
       pEvent->obid ); if ( NULL != pClientItem ) {
                        // Save interface
                        pNode->m_interfaceguid = pClientItem->m_guid;
                    }
                    pthread_mutex_unlock( &m_pCtrlObject->m_mutexDeviceList );

                    // We set name from client id
                    // 'client_clientid_nickname'
                    if ( pNode->m_strNodeName.empty() ) {
                        pNode->m_strNodeName = "client_";
                        if ( NULL != pClientItem ) {
                            pNode->m_strNodeName +=
                                vscp_str_format( "%lu",
                                                    (unsigned
       long)pClientItem->m_clientID );
                        }
                        else {
                            pNode->m_strNodeName += "isNULL";
                        }
                        pNode->m_strNodeName += "_";
                        pNode->m_strNodeName +=
                            vscp_str_format( "%u",
                                                ( pEvent->GUID[ 14 ] << 8 ) +
                                                    pEvent->GUID[ 15 ] );
                    }

                }

            }

        }
        else {

            // This node is known so we don't need to do a thing. Hurray! A la

        }
    */
    return pNode;
}

///////////////////////////////////////////////////////////////////////////////
// threadDiscovery
//

void *
nodeDiscoveryThread(void *threadData)
{
    /*
        TODO

        // Must have a valid pointer to the control object
        if ( NULL == m_pCtrlObject ) return NULL;

        // We need to create a clientobject and add this object to the list
        m_pClientItem = new CClientItem;
        if ( NULL == m_pClientItem ) return NULL;

        // This is an active client
        m_pClientItem->m_bOpen = true;
        m_pClientItem->m_type =  CLIENT_ITEM_INTERFACE_TYPE_CLIENT_INTERNAL;
        m_pClientItem->m_strDeviceName = "VSCP node discovery thread.|Started at
    "; vscpdatetime now = vscpdatetime::Now(); m_pClientItem->m_strDeviceName +=
    now.FormatISODate(); m_pClientItem->m_strDeviceName += " ";
        m_pClientItem->m_strDeviceName += now.FormatISOTime();

        // Add the client to the Client List
        pthread_mutex_lock( &m_pCtrlObject->m_clientMutex );
        if ( !m_pCtrlObject->addClient ( m_pClientItem, CLIENT_ID_DAEMON_WORKER
    ) ) {
            // Failed to add client
            delete m_pClientItem;
            m_pClientItem = NULL;
            m_pCtrlObject->m_clientMutex.Unlock();
            m_pCtrlObject->logMsg( "Deamon discovery: Failed to add client.
    Terminating thread." ); return NULL;
        }
        pthread_mutex_unlock( &m_pCtrlObject->m_clientMutex );

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
        pthread_mutex_lock( &m_pCtrlObject->m_clientMutex );
        m_pCtrlObject->removeClient( m_pClientItem );
        pthread_mutex_unlock( &m_pCtrlObject->m_clientMutex );
    */
    return NULL;
}