// ClientList.h: interface for the CClientList class.
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


#if !defined(AFX_CLIENTLIST_H__B0190EE5_E0E8_497F_92A0_A8616296AF3E__INCLUDED_)
#define AFX_CLIENTLIST_H__B0190EE5_E0E8_497F_92A0_A8616296AF3E__INCLUDED_

#ifdef WIN32
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#endif

#ifdef WIN32

#else

#include <limits.h>

#endif

#include <vscp.h>
#include <guid.h>
#include <userlist.h>
#include <devicelist.h>


WX_DECLARE_LIST ( vscpEvent, CLIENTEVENTLIST );

//
// defines for levels
//

enum {
    CLIENT_ITEM_LEVEL1 = 0,
    CLIENT_ITEM_LEVEL2
};


enum {
    CLIENT_ITEM_INTERFACE_TYPE_NONE = 0,
    CLIENT_ITEM_INTERFACE_TYPE_CLIENT_INTERNAL,     // Daemon internal 
    CLIENT_ITEM_INTERFACE_TYPE_DRIVER_LEVEL1,       // Level I drivers
    CLIENT_ITEM_INTERFACE_TYPE_DRIVER_LEVEL2,       // Level II drivers
    CLIENT_ITEM_INTERFACE_TYPE_CLIENT_TCPIP,        // TCP/IP interface
    CLIENT_ITEM_INTERFACE_TYPE_CLIENT_UDP,          // UDP interface
    CLIENT_ITEM_INTERFACE_TYPE_CLIENT_WEB,          // WEB interface
    CLIENT_ITEM_INTERFACE_TYPE_CLIENT_WEBSOCKET,    // Websocket interface
    CLIENT_ITEM_INTERFACE_TYPE_CLIENT_REST,         // REST interface
    CLIENT_ITEM_INTERFACE_TYPE_CLIENT_MULTICAST,    // Multicast interface
    CLIENT_ITEM_INTERFACE_TYPE_CLIENT_MQTT,         // MQTT interface
    CLIENT_ITEM_INTERFACE_TYPE_CLIENT_COAP,         // COAP interface
    CLIENT_ITEM_INTERFACE_TYPE_CLIENT_DISCOVERY,    // Discovery interface
};


// Both the UDP send and receive id clients use this is. This way
// received UDP events will not be transmitted on the UDP interface. 
#define CLIENT_ITEM_SPECIAL_ID_UDP                      0xffffffff

/*!
    Client Item
*/

class CClientItem
{

public:

    /// Constructor
    CClientItem();

    /// Destructor
    ~CClientItem();


public:
 
    /*!
        Input Queue
    */
    CLIENTEVENTLIST m_clientInputQueue;

    /*!
        Semaphore to indicate that an event has been received
    */
    wxSemaphore m_semClientInputQueue;

    /*!
        Mutex handle that is used for sharing of the client object
    */
    wxMutex m_mutexClientInputQueue;

    /*!
        Client ID
    */
    uint32_t m_clientID;

    /*!
        Flag for open/closed channel
    */
    bool m_bOpen;

    /*!
        Channel flags
    */
    uint32_t m_flags;

    /*!
        Filter mask for VSCP
    */
    vscpEventFilter m_filterVSCP;

    /*!
        Interface GUID

        The GUID for a client have the following form MSB -> LSB

        0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFD ip-address ip-address ip-address ip-address Client-ID Client-ID 0 0

        ip-address ver 4 interface IP address
        Client-ID mapped id for this client

        This is the default address and it can be changed by the client application

    */
    cguid m_guid;

    /*!
        Interface name
    */
    wxString m_strDeviceName;

    /*!
        Channel state information
    */
    canalStatus m_status;

    /*!
        Channel statistics
    */
    canalStatistics m_statistics;

    /*!
        Event to indicate that there is an event to send
    */
    wxSemaphore m_hEventSend;

    /*!
        Interface type: CANAL, TCP/IP
    */
    uint8_t m_type;

    /*!
        Mark as UDP receive channel if set
        This is used by the UDP send routine to disregard
        events that are received by UDP.
    */
    bool m_bUDPReceiveChannel;

    ///////////////////////////////////////////////////////////////////////////
    //                       Used by TCP/IP client thread
    //////////////////////////////////////////////////////////////////////////
    
    /*!
        RCVLOOP clock
    */
    wxLongLong m_timeRcvLoop;
    
    /// Username given by user
    wxString m_UserName;
    
    /// Password given by user
    wxString m_Password;
    
    /// Session id
    char m_sid[33];

    /// True if the credentials has been checked.
    bool bAuthenticated;

    /// pointer ti the user
    CUserItem *m_pUserItem;

    /// Read buffer
    wxString m_readBuffer;

    /// Last command executed
    wxString m_lastCommand;

    // Current command
    wxString m_currentCommand;

    /// Current command i all upper case
    wxString m_currentCommandUC;
};

// List with clientitems
WX_DECLARE_LIST( CClientItem, VSCPCLIENTLIST );


class CClientList  
{

public:

    /// Constructor
    CClientList();

    /// Destructor
    virtual ~CClientList();

    /*1
        Add a client to the list
        @param pClientItem Client to add
        @param id Normally not used but can be used to set a specific id
        @return true om success.
    */
    bool addClient( CClientItem *pClientItem, uint32_t id = 0 );

    /*!
        Remove a client from the list
    */
    bool removeClient( CClientItem *pClientItem );

    /*!
        Get client form client id
        @param id Numeric id for the client
        @return A pointer to a cientitem on success or NULL on failure.
    */
    CClientItem *getClientFromId( uint32_t id );

    /*!
        Get Client from GUID
        @param guid Guid for the client
        @return A pointer to a cientitem on success or NULL on failure.
    */
    CClientItem *getClientFromGUID( cguid& guid );

public:

    /*!
        List with clients
    */
    VSCPCLIENTLIST m_clientItemList;

    /// Counter for client id's
    uint32_t m_clientIDCounter;

protected:


    /*!
        System assigned ID for device
    */
    uint32_t m_clientID;

};

#endif // !defined(AFX_CLIENTLIST_H__B0190EE5_E0E8_497F_92A0_A8616296AF3E__INCLUDED_)
