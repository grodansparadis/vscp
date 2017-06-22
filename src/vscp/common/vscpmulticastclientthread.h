// vscpmulticastclientthread.h
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


#if !defined(VSCP_MULTICAST_CLIENT_THREAD_H__INCLUDED_)
#define VSCP_MULTICAST_CLIENT_THREAD_H__INCLUDED_

#include "userlist.h"
#include "controlobject.h"

class VSCPMulticastClientThread;

// Client structure (Will receive events from VSCP server)
typedef struct {
    bool                        m_bEnable;          // Enable the channel
    wxString                    m_public;           // Public interface to bind to
    uint16_t                    m_port;             // Port for channel
    wxString                    m_gropupAddress;    // Multicast group address "udp://224.0.23.158:44444"
    int                         m_ttl;              // Multicast ttl (defaults to 1)
    cguid                       m_guid;             // GUID to use for channel
    vscpEventFilter             m_txFilter;         // Filter for outgoing events
    vscpEventFilter             m_rxFilter;         // Filter for incoming events
    uint8_t                     m_nEncryption;      // Encryption algorithm to use for this client
    bool                        m_bSendAck;         // Send response frame when receiving frame
    uint8_t                     m_index;            // Rolling index
    
    wxMutex                     m_mutexVSCPMulticastThread;     // Protect thread object
    VSCPMulticastClientThread   *m_pWorkerThread;   // Worker thread
} multicastChannelItem;

WX_DECLARE_LIST(multicastChannelItem, MULTICASTCHANNELLIST );

typedef struct  {       
    bool                    m_bEnable;          // Enable multicast interface    
    MULTICASTCHANNELLIST    m_channels;         // List containing multicast channels
} multicastInfo;



/*!
 * Class that handle one multicast channel
 */

class VSCPMulticastClientThread : public wxThread
{

public:
    
    /// Constructor
    VSCPMulticastClientThread();

    /// Destructor
    ~VSCPMulticastClientThread();

    /*!
        Thread code entry point
    */
    virtual void *Entry();


    /*! 
        called when the thread exits - whether it terminates normally or is
        stopped with Delete() (but not when it is Kill()ed!)
    */
    virtual void OnExit();
    
    
    static void ev_handler( struct mg_connection *nc, int ev, void *p );


    /*!
     * Receive Multicast channel frame
     * 
     * @param nc Mongoose connection handle
     * @param pClientItem Client item for this user. Normally "USP"
     * @param pRxFilter Pointer to receive filter. Can be NULL to accept 
     *          all events.
     * @return True on success, false on failure.
     */
    static bool receiveFrame( struct mg_connection *nc, 
                                CClientItem *pClientItem,
                                vscpEventFilter *pRxFilter );
    
    static bool replyAckFrame( struct mg_connection *nc, 
                                    uint8_t pkttype,
                                    uint8_t index );
    
    static bool replyNackFrame( struct mg_connection *nc, 
                                    uint8_t pkttype,
                                    uint8_t index );
    
    /*!
     * Sends an outgoing event if there is one in the client queue
     */
    bool sendFrame( struct mg_mgr *pmgr, 
                        CClientItem *pClientItem );

// --- Member variables ---

    /*!
        Termination control
    */
    bool m_bQuit;
    
    /// Info about this multicast channel
    multicastChannelItem *m_pChannel;

private:    
    
    /// UDP Client item
    CClientItem *m_pClientItem;
    
    // Groupaddess + port to send on
    // default: udp://224.0.23.158:44444
    wxString m_sendAddress;

};




#endif