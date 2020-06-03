// ClientList.h: interface for the CClientList class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// Copyright © 2000-2020 Ake Hedman,
// Ake Hedman, Grodans Paradis AB, <akhe@vscp.org>
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

#if !defined(CLIENTLIST_H__B0190EE5_E0E8_497F_92A0_A8616296AF3E__INCLUDED_)
#define CLIENTLIST_H__B0190EE5_E0E8_497F_92A0_A8616296AF3E__INCLUDED_

#include <list>

#include <devicelist.h>
#include <vscpdatetime.h>
#include <guid.h>
#include <userlist.h>
#include <vscp.h>

// Predefined client id's
#define CLIENT_ID_DAEMON_WORKER 0xffff
#define CLIENT_ID_INTERNAL 0xfffe

//
// defines for levels
//

enum
{
    CLIENT_ITEM_LEVEL1 = 0,
    CLIENT_ITEM_LEVEL2,
    CLIENT_ITEM_LEVEL3
};

enum
{
    CLIENT_ITEM_INTERFACE_TYPE_NONE = 0,
    CLIENT_ITEM_INTERFACE_TYPE_CLIENT_INTERNAL,     // 1 Daemon internal
    CLIENT_ITEM_INTERFACE_TYPE_DRIVER_LEVEL1,       // 2 Level I drivers
    CLIENT_ITEM_INTERFACE_TYPE_DRIVER_LEVEL2,       // 3 Level II drivers
    CLIENT_ITEM_INTERFACE_TYPE_DRIVER_LEVEL3,       // 4 Level III drivers
    CLIENT_ITEM_INTERFACE_TYPE_CLIENT_TCPIP,        // 5 TCP/IP interface
    CLIENT_ITEM_INTERFACE_TYPE_CLIENT_UDP,          // 6 UDP interface
    CLIENT_ITEM_INTERFACE_TYPE_CLIENT_WEB,          // 7 WEB interface
    CLIENT_ITEM_INTERFACE_TYPE_CLIENT_WEBSOCKET,    // 8 Websocket interface
    CLIENT_ITEM_INTERFACE_TYPE_CLIENT_REST,         // 9 REST interface
    CLIENT_ITEM_INTERFACE_TYPE_CLIENT_MULTICAST,    // 10 Multicast interface
    CLIENT_ITEM_INTERFACE_TYPE_CLIENT_MULTICAST_CH, // 11 Multicast channel
                                                    // interface
    CLIENT_ITEM_INTERFACE_TYPE_CLIENT_MQTT,         // 12 MQTT interface
    CLIENT_ITEM_INTERFACE_TYPE_CLIENT_COAP,         // 13 COAP interface
    CLIENT_ITEM_INTERFACE_TYPE_CLIENT_DISCOVERY,    // 14 Discovery interface
    CLIENT_ITEM_INTERFACE_TYPE_CLIENT_JAVASCRIPT,   // 15 JavaScript interface
    CLIENT_ITEM_INTERFACE_TYPE_CLIENT_LUA,          // 16 LUA Script interface
};

// Description of each interface type (for web interface etc)
// const char **interface_description;

// Both the UDP send and receive id clients use this is. This way
// received UDP events will not be transmitted on the UDP interface.
#define CLIENT_ITEM_SPECIAL_ID_UDP 0xffffffff

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
        Check if the command line start with the command
        The command is checked case insensitive
        @param cmd The command to look for.
        @param bFix The command string have the command removed.
        @return true if command is found
     */
    bool CommandStartsWith(const std::string &cmd, bool bFix = true);

    /*!
        Set device name
        @param name Name to set
    */
    void setDeviceName(const std::string &name);

    /*!
        Get client on string form
        "id,type,GUID,name,dt-created(UTC),open-flag"
        @return Client info on string form
    */
    std::string getAsString(void);

  public:
    // Input Queue
    std::deque<vscpEvent*> m_clientInputQueue;

    // Semaphore to indicate that an event has been received
    sem_t m_semClientInputQueue;

    // Mutex handle that is used for sharing of the client object
    pthread_mutex_t m_mutexClientInputQueue;

    // Client ID for this client item
    uint16_t m_clientID;

    // Flag for open/closed channel
    bool m_bOpen;

    // Channel flags
    uint32_t m_flags;

    // Filter/mask for VSCP
    vscpEventFilter m_filter;

    /*!
        Interface GUID

        The GUID for a client have the following form MSB -> LSB

        0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFD ip-address ip-address ip-address
       ip-address Client-ID Client-ID 0 0

        ip-address ver 4 interface IP address
        Client-ID mapped id for this client

        This is the default address and it can be changed by the client
       application

    */
    cguid m_guid;

    // Interface name
    std::string m_strDeviceName;

    // Datetime UTC when created
    vscpdatetime m_dtutc;

    // Channel state information
    canalStatus m_status;

    // Channel statistics
    canalStatistics m_statistics;

    // Event to indicate that there is an event to send
    sem_t m_hEventSend;

    // Interface type: CANAL, TCP/IP
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

    // UTC time since last client activity
    long m_clientActivity;

    // RCVLOOP clock (UTC time for last sent "+OK")
    uint64_t m_timeRcvLoop;

    // Username given by user
    std::string m_UserName;

    // Password given by user
    std::string m_Password;

    // Session id
    char m_sid[33];

    // True if the credentials has been checked.
    bool bAuthenticated;

    // pointer ti the user
    CUserItem *m_pUserItem;

    // Read buffer
    std::string m_readBuffer;

    // Last command executed
    std::string m_lastCommand;

    // Current command
    std::string m_currentCommand;

    // Current token is the first space separated
    // item in the command string
    std::string m_currentToken;
};

// ----------------------------------------------------------------------------

class CClientList
{

  public:
    /// Constructor
    CClientList();

    /// Destructor
    virtual ~CClientList();

    /*!
        Find a free client id
        pid Pointer to uint16_t that return free id.
        @return True if id could be found
    */
    bool findFreeId(uint16_t *pid );

    /*!
        Add a client to the list
        @param pClientItem Client to add
        @param id Normally not used but can be used to set a specific id
        @return true om success.
    */
    bool addClient(CClientItem *pClientItem, uint32_t id = 0);

    /*!
        Add a client to the list using set GUID
        @param pClientItem Client to add
        @param guid The guid that is used for the client. Two least
        significant bytes will be set to zero.
        @return true om success.
    */
    bool addClient(CClientItem* pClientItem, cguid& guid);

    /*!
        Remove a client from the list
        @param pClientItem Pointer to client item
        @return true on success
    */
    bool removeClient(CClientItem *pClientItem);

    /*!
        Remove all clinets
        @return true on success
    */
    bool removeAllClients(void);

    /*!
        Get client form client id
        @param id Numeric id for the client
        @return A pointer to a clientitem on success or NULL on failure.
    */
    CClientItem *getClientFromId(uint16_t id);

    /*!
        Get client form ordinal
        @param id Numeric ordinal for the client
        @return A pointer to a clientitem on success or NULL on failure.
    */
    CClientItem *getClientFromOrdinal(uint16_t ordinal);

    /*!
        Get Client from GUID
        @param guid Guid for the client
        @return A pointer to a cientitem on success or NULL on failure.
    */
    CClientItem *getClientFromGUID(cguid &guid);

    /*!
        Get current number of clients

        @return Current number of client.
    */
    size_t getClientCount(void) { return m_itemList.size(); };

    /*!
        Get all interfaces as string
        @return List of all interfaces
    */
    std::string getAllClientsAsString(void);

    /*!
        Get a client from it's ordinal
        @param n Ordinal for client in list
        @param client [out] Client data on string form
        @return true on success
    */
    bool getClient(uint16_t n, std::string &client);

  public:

    // List with clients
    std::deque<CClientItem*> m_itemList;

    // Mutex that protect the list
    pthread_mutex_t m_mutexItemList;
};

#endif // !defined(CLIENTLIST_H__B0190EE5_E0E8_497F_92A0_A8616296AF3E__INCLUDED_)
