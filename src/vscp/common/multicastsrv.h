// multicastsrv.h
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


#if !defined(VSCP_MULTICAST_CLIENT_THREAD_H__INCLUDED_)
#define VSCP_MULTICAST_CLIENT_THREAD_H__INCLUDED_

#include "userlist.h"
#include "controlobject.h"

class VSCPMulticastClientThread;

// Client structure (Will receive events from VSCP server)
typedef struct {
    bool                        m_bEnable;          // Enable the channel
    bool                        m_bAllowUnsecure;   // Allow receive of un encrypted frames
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
    VSCPMulticastClientThread   *m_pWorkerThread;               // Worker thread
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
    
    static bool replyAckFrame( VSCPMulticastClientThread *pMulticastClientThread, 
                                    uint8_t pkttype,
                                    uint8_t index );
    
    static bool replyNackFrame( VSCPMulticastClientThread *pMulticastClientThread, 
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
    
    /// Multicast channel Client item
    CClientItem *m_pClientItem;
    
    // Groupaddess + port to send on
    // default: udp://224.0.23.158:44444
    //wxString m_sendAddress;
    
    // Socket for sending
    int m_sendSock;
    
    // Send address
    struct sockaddr_in m_mc_sendAddr;     

};




#endif