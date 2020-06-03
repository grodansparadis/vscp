// ClientList.cpp: implementation of the CClientList class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// Copyright © 2000-2020 Ake Hedman,
// Grodans Paradis AB, <akhe@vscp.org>
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

#define _POSIX

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

#include <canal_macro.h>
#include <vscp.h>
#include <vscpdatetime.h>
#include <vscphelper.h>

#include "clientlist.h"

const char* interface_description[] = { "Unknown (you should not see this).",
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
                                        NULL };

///////////////////////////////////////////////////////////////////////////////
// CClientItem
//

CClientItem::CClientItem()
{
    m_bOpen                 = false; // Initially Not Open
    m_flags                 = 0;     // No flags
    m_status.channel_status = 0;
    m_clientID              = 0;
    m_type                  = CLIENT_ITEM_INTERFACE_TYPE_NONE;
    m_bUDPReceiveChannel    = false;

    m_dtutc = vscpdatetime::UTCNow();

    sem_init(&m_semClientInputQueue, 0, 0);
    sem_init(&m_hEventSend, 0, 0);
    pthread_mutex_init(&m_mutexClientInputQueue, NULL);

    // Nill GUID
    m_guid.clear();

    // Nill Level II mask (accept all)
    vscp_clearVSCPFilter(&m_filter);

    m_statistics.cntReceiveFrames  = 0; // # of receive frames
    m_statistics.cntTransmitFrames = 0; // # of transmitted frames
    m_statistics.cntReceiveData    = 0; // # of received data bytes
    m_statistics.cntTransmitData   = 0; // # of transmitted data bytes
    m_statistics.cntOverruns       = 0; // # of overruns
    m_statistics.cntBusWarnings    = 0; // # of bus warnings
    m_statistics.cntBusOff         = 0; // # of bus off's

    m_status.channel_status   = 0;
    m_status.lasterrorcode    = 0;
    m_status.lasterrorsubcode = 0;
    memset(m_status.lasterrorstr, 0, sizeof(m_status.lasterrorstr));

    ///////////////////////////////////////////////////////////////////////////
    //                 Working variable storage for clients
    //////////////////////////////////////////////////////////////////////////

    bAuthenticated = false;
    m_pUserItem    = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// ~CClientItem
//

CClientItem::~CClientItem()
{
    std::deque<vscpEvent*>::iterator iter;
    for (iter = m_clientInputQueue.begin(); iter != m_clientInputQueue.end();
         ++iter) {
        vscpEvent* pEvent = *iter;
        vscp_deleteEvent_v2(&pEvent);
    }
    m_clientInputQueue.clear();

    sem_destroy(&m_hEventSend);
    sem_destroy(&m_semClientInputQueue);
    pthread_mutex_destroy(&m_mutexClientInputQueue);
}

///////////////////////////////////////////////////////////////////////////////
// CommandStartWith
//

bool
CClientItem::CommandStartsWith(const std::string& cmd, bool bFix)
{
    if (!vscp_startsWith(vscp_upper(m_currentCommand), vscp_upper(cmd))) {
        return false;
    }

    // If asked to do so remove the command.
    if (bFix) {
        if (m_currentCommand.length() - cmd.length()) {
            m_currentCommand =
              vscp_str_right(m_currentCommand,
                             m_currentCommand.length() - cmd.length() - 1);
        } else {
            m_currentCommand.clear();
        }
        vscp_trim(m_currentCommand);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setDeviceName
//

void
CClientItem::setDeviceName(const std::string& name)
{
    m_strDeviceName = name;
    m_strDeviceName += "|Started at ";

    m_strDeviceName += vscpdatetime::Now().getISODateTime();
}

///////////////////////////////////////////////////////////////////////////////
// getAsString
//
// "id,type,GUID,name,dt-created(UTC),open-flag, flags"

std::string
CClientItem::getAsString(void)
{
    std::string str;

    str = vscp_str_format("%ud,", m_clientID);
    str += vscp_str_format("%d,", m_type);
    str += m_guid.toString();
    str += ",";
    str += m_strDeviceName;
    str += ",";
    str += m_dtutc.getISODateTime();
    str += ",";
    str += m_bOpen ? "true" : "false";
    str += ",";
    str += vscp_str_format("%ul", m_flags);

    return str;
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// compareClientItems
//
// Type of compare function for list sort operation (as in 'qsort')
//

bool
compareClientItems(const uint16_t element1, const uint16_t element2)
{
    return (element1 < element2);
}

///////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CClientList
//

CClientList::CClientList()
{
    pthread_mutex_init(&m_mutexItemList, NULL);
}

///////////////////////////////////////////////////////////////////////////////
// ~CClientList
//

CClientList::~CClientList()
{
    removeAllClients();
    pthread_mutex_destroy(&m_mutexItemList);
}

///////////////////////////////////////////////////////////////////////////////
// findFreeId
//

bool
CClientList::findFreeId(uint16_t* pid)
{
    std::list<uint16_t> sorterIdList;
    std::deque<CClientItem*>::iterator it;

    // Check pointer
    if (NULL == pid)
        return false;

    // Find next free id
    for (it = m_itemList.begin(); it != m_itemList.end(); ++it) {
        CClientItem* pItem = *it;
        sorterIdList.push_back(pItem->m_clientID);
    }

    // Sort list on client id
    sorterIdList.sort(compareClientItems);

    std::list<uint16_t>::iterator it_id;
    for (it_id = sorterIdList.begin(); it_id != sorterIdList.end(); ++it_id) {
        // As the list is sorted on id we have found an
        // unused id if the id is higher than the counter
        if (*pid < *it_id) {
            break;
        }

        (*pid)++;
        if (0 == *pid) {
            return false; // All client id's are in use
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// addClient
//

bool
CClientList::addClient(CClientItem* pClientItem, uint32_t id)
{
    // Check pointer
    if (NULL == pClientItem) {
        return false;
    }

    pClientItem->m_clientID = id ? id : 1;

    if (0 == id) {
        if (!findFreeId(&pClientItem->m_clientID)) {
            return false;
        }
    }

    // We try to assign requested id
    std::deque<CClientItem*>::iterator it;
    for (it = m_itemList.begin(); it != m_itemList.end(); ++it) {

        CClientItem* pItem = *it;

        // If id is already in use fail
        if (pClientItem->m_clientID == pItem->m_clientID) {
            return false;
        }
    }

    // Append to list
    m_itemList.push_back(pClientItem);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// addClient
//

bool
CClientList::addClient(CClientItem* pClientItem, cguid& guid)
{
    // Check pointer
    if (NULL == pClientItem) {
        return false;
    }

    if ( !addClient(pClientItem) ) {
        return false;
    }

    // Set the guid
    pClientItem->m_guid = guid;

    // Make sure nickname id is zero
    pClientItem->m_guid.setNicknameID(0);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// removeClient
//

bool
CClientList::removeClient(CClientItem* pClientItem)
{
    // Must be a valid pointer
    if (NULL == pClientItem) {
        syslog(LOG_ERR,"removeClient in clientlist but clinet obj is NULL");
        return false;
    }
     
    std::deque<vscpEvent*>::iterator iter;
    for (iter = pClientItem->m_clientInputQueue.begin();
         iter != pClientItem->m_clientInputQueue.end();
         ++iter) {
            
        vscpEvent* pEvent = *iter;
        vscp_deleteEvent_v2(&pEvent);
    }
    pClientItem->m_clientInputQueue.clear();

    // Take away the node
    for (std::deque<CClientItem*>::iterator it = m_itemList.begin();
         it != m_itemList.end();
         ++it) {
        if (*it == pClientItem) {
            m_itemList.erase(it);
            delete pClientItem;
            return true;
        }
    }

    return false;
}

bool
CClientList::removeAllClients()  
{
    pthread_mutex_lock(&m_mutexItemList);
    // Empty the client list
    std::deque<CClientItem*>::iterator it;
    for (it = m_itemList.begin(); it != m_itemList.end(); ++it) {
        removeClient(*it);
        delete *it;
    }
    m_itemList.clear();
    pthread_mutex_unlock(&m_mutexItemList);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getClientFromId
//

CClientItem*
CClientList::getClientFromId(uint16_t id)
{
    std::deque<CClientItem*>::iterator it;
    CClientItem* returnItem = NULL;

    for (it = m_itemList.begin(); it != m_itemList.end(); ++it) {

        CClientItem* pItem = *it;
        if (pItem->m_clientID == id) {
            returnItem = pItem;
            break;
        }
    }

    return returnItem;
}

///////////////////////////////////////////////////////////////////////////////
// getClientFromOrdinal
//

CClientItem*
CClientList::getClientFromOrdinal(uint16_t ordinal)
{
    if (!m_itemList.size())
        return NULL;
    if (ordinal > (m_itemList.size() - 1))
        return NULL;

    return m_itemList[ordinal];
}

///////////////////////////////////////////////////////////////////////////////
// getClientFromGUID
//

CClientItem*
CClientList::getClientFromGUID(cguid& guid)
{
    std::deque<CClientItem*>::iterator it;
    CClientItem* returnItem = NULL;

    for (it = m_itemList.begin(); it != m_itemList.end(); ++it) {

        CClientItem* pItem = *it;
        if (pItem->m_guid == guid) {
            returnItem = pItem;
            break;
        }
    }

    return returnItem;
}

///////////////////////////////////////////////////////////////////////////////
// getAllInterfacesAsString
//

std::string
CClientList::getAllClientsAsString(void)
{
    std::string str;

    pthread_mutex_lock(&m_mutexItemList);

    std::deque<CClientItem*>::iterator it;
    for (it = m_itemList.begin(); it != m_itemList.end(); ++it) {

        CClientItem* pItem = *it;
        str += pItem->getAsString();
        str += "\r\n";
    }

    pthread_mutex_unlock(&m_mutexItemList);

    return str;
}

///////////////////////////////////////////////////////////////////////////////
// getClient
//

bool
CClientList::getClient(uint16_t n, std::string& client)
{
    if (!m_itemList.size())
        return false;
    if (n > (m_itemList.size() - 1))
        return false;

    CClientItem* pClient = m_itemList[n];
    if (NULL == pClient)
        return false;
    client = pClient->getAsString();

    return true;
}