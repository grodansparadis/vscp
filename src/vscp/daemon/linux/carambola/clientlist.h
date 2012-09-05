// ClientList.h: interface for the CClientList class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2012 Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
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

#include "../../vscp/common/vscp.h"
#include "devicelist.h"


WX_DECLARE_LIST ( vscpEvent, CLIENTEVENTLIST );

//
// defines for levels
//
#define CLIENT_ITEM_LEVEL1                              0
#define CLIENT_ITEM_LEVEL2                              1

#define CLIENT_ITEM_INTERFACE_TYPE_NONE                 0
#define CLIENT_ITEM_INTERFACE_TYPE_CLIENT_INTERNAL      1
#define CLIENT_ITEM_INTERFACE_TYPE_DRIVER_CANAL         2
#define CLIENT_ITEM_INTERFACE_TYPE_DRIVER_TCPIP         3
#define CLIENT_ITEM_INTERFACE_TYPE_CLIENT_TCPIP         4

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
        Event to indicate that an event has been received
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

        0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFD ip-address ip-address ip-address ip-address Client-ID Client-ID Client-ID Client-ID

        ip-address ver 4 interface IP address
        Client-ID mapped id for this client

        This is the default address and it can be changed by the client application

    */
    uint8_t m_GUID[ 16 ];

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
        Interface type: canal, TCP/IP
    */
    uint8_t m_type;

    /*!
        Mark as UDP receive channel if set
        This is used by the UDP send routine to disregard
        events that are received by UDP.
    */
    bool m_bUDPReceiveChannel;

};

// List with clientitems
WX_DECLARE_LIST ( CClientItem, VSCPCLIENTLIST );


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
