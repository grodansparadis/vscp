// daemon_VSCP.h
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2017 
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


#if !defined(DAEMONVSCP_H__INCLUDED_)
#define DAEMONVSCP_H__INCLUDED_


#include "wx/thread.h"
#include "wx/socket.h"
#include <wx/datetime.h>

#include "guid.h"


class CControlObject;
class CClientItem;
class CNodeInformation;

// This thread is used for node discovery


class discoveryVSCPThread : public wxThread
{

public:

    /// Constructor
    discoveryVSCPThread();

    /// Destructor
    ~discoveryVSCPThread();

    /*!
        Thread code entry point
    */
    virtual void *Entry();

    /*! 
        called when the thread exits - whether it terminates normally or is
        stopped with Delete() (but not when it is Kill()ed!)
    */
    virtual void OnExit();

    /*!
        Send event to client with specific id
    */
    bool sendEvent( vscpEvent *pEvent, uint32_t obid );

    /*!
        Read a level I register
    */
    bool readLevel1Register( uint8_t nodeid, 
                                uint8_t reg, 
                                uint8_t *pcontent,
                                uint32_t clientID,
                                uint32_t timeout = 1000 );

    /*!
        Termination control
    */
    bool m_bQuit;

    // Node id for the node to investigate
    uint8_t m_nodeid;

    // Client id for interface the node is located at
    uint32_t m_clientID;

    // Clientitem for this thread
    CClientItem *m_pClientItem;

    // Pointer to the control object
    CControlObject *m_pCtrlObject;
};

// This structure holds information about nodes that need to have
// information collected from them.

struct discoveredNodeInfo {
    int bStatus;                    // 0 = working, -1=failed, 777=success.
    discoveryVSCPThread *pThread;   // Discover thread
    uint8_t nodeid;                 // nodeid for the node to investigate
    uint32_t clientId;              // Clientid for node to investigate
    cguid guid;                     // GUID for node.
    wxString mdfPath;               // MDF path for node.
};

WX_DECLARE_LIST ( discoveredNodeInfo, DISCOVERYLIST );



/*!
    This class implement a one of thread that look
    for specific events and react on them appropriately.

*/

class daemonVSCPThread : public wxThread
{

public:
    
    /// Constructor
    daemonVSCPThread();

    /// Destructor
    ~daemonVSCPThread();

    /*!
        Thread code entry point
    */
    virtual void *Entry();


    /*! 
        called when the thread exits - whether it terminates normally or is
        stopped with Delete() (but not when it is Kill()ed!)
    */
    virtual void OnExit();
    
    /*!
     *  Parse the interface string and fetch address and port.
     * 
     * @param ifadder Interface address. Three forms are valid
     *                  udp://9598
     *                  udp://192.168.1.10:9598
     *                  udp://192.168.1.10   (port defaults to 9598)
     * @paran ip ip address part is returned here.
     * @param pPort Pointer to port whcih get port information.
     * 
     */
    
    bool parseInterface( const wxString &ifaddr, 
                            wxString &ip, 
                            unsigned short *pPort );

    /*!
        Add a known node if it is not already known
        @param pEvent Event that initiated the add
        @return Pointer to new or old information object or NULL if failure.
    */
    CNodeInformation *addNodeIfNotKnown( vscpEvent *pEvent );

    /*!
        Send multicast event
        @param sock Multicast socket to send on
        @param pEvent Event to send
        @param port Port to send multicast frame on.
        @return true on success, false on failure
    */
    bool sendMulticastEvent( int sock,
                                vscpEvent *pEvent,
                                int port = VSCP_ANNNOUNCE_MULTICAST_PORT );

    /*!
        Send multicast event Ex
        @param sock Multicast socket to send on
        @param pEventEx Event ex to send
        @param port Port to send multicast frame on.
        @return true on success, false on failure
    */
    bool sendMulticastEventEx( int sock,
                                vscpEventEx *pEventEx,
                                int port = VSCP_ANNNOUNCE_MULTICAST_PORT);

    /*!
        Send multicast information event
        @param sock Multicast socket to send on
        @param pNode Pointer to information node.
        @param port Port to send multicast frame on.
        @return true on success, false on failure
    */
    bool sendMulticastInformationProxyEvent( int sock, 
                                                CNodeInformation *pNode,
                                                int port = VSCP_ANNNOUNCE_MULTICAST_PORT );

    /*!
        Termination control
    */
    bool m_bQuit;

    CControlObject *m_pCtrlObject;

    // This list contains items (nodes) that are under discovery
    DISCOVERYLIST m_discoverList;

};




#endif



