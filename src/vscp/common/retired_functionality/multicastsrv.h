// multicastsrv.h
//
// This file is part of the VSCP (http://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2020 Ake Hedman, Grodans Paradis AB
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

#if !defined(VSCP_MULTICAST_CLIENT_THREAD_H__INCLUDED_)
#define VSCP_MULTICAST_CLIENT_THREAD_H__INCLUDED_

#include <netinet/in.h>

#include "controlobject.h"
#include "userlist.h"

void *
multicastClientThread(void *pData);

class MulticastObj;

// Client structure (Will receive events from VSCP server)
typedef struct
{
    bool m_bEnable;        // Enable the channel
    bool m_bAllowUnsecure; // Allow receive of un encrypted frames
    std::string m_public;  // Public interface to bind to
    uint16_t m_port;       // Port for channel
    std::string
      m_gropupAddress; // Multicast group address "udp://224.0.23.158:44444"
    int m_ttl;         // Multicast ttl (defaults to 1)
    cguid m_guid;      // GUID to use for channel
    vscpEventFilter m_txFilter; // Filter for outgoing events
    vscpEventFilter m_rxFilter; // Filter for incoming events
    uint8_t m_nEncryption;      // Encryption algorithm to use for this client
    bool m_bSendAck;            // Send response frame when receiving frame
    uint8_t m_index;            // Rolling index

    bool m_quit;              // Quit thread if true
    pthread_t m_workerThread; // Worker thread
} multicastChannelItem;

/*!
 * Class that handle one multicast channel
 */

class MulticastObj
{

  public:
    /// Constructor
    MulticastObj(CControlObject *pobj = NULL);

    /// Destructor
    ~MulticastObj();

    // static void ev_handler( struct mg_connection *nc, int ev, void *p );

    /*!
     * Receive Multicast channel frame
     *
     * @param nc Mongoose connection handle
     * @param pClientItem Client item for this user. Normally "USP"
     * @param pRxFilter Pointer to receive filter. Can be NULL to accept
     *          all events.
     * @return True on success, false on failure.
     */
    static bool receiveFrame(struct mg_connection *nc,
                             CClientItem *pClientItem,
                             vscpEventFilter *pRxFilter);

    static bool replyAckFrame(MulticastObj *pMulticastClientThread,
                              uint8_t pkttype,
                              uint8_t index);

    static bool replyNackFrame(MulticastObj *pMulticastClientThread,
                               uint8_t pkttype,
                               uint8_t index);

    /*!
     * Sends an outgoing event if there is one in the client queue
     */
    bool sendFrame(struct mg_mgr *pmgr, CClientItem *pClientItem);

    /*!
        Assign control object
    */
    void setControlObjectPointer(CControlObject *pobj) { m_pObj = pobj; };

    // --- Member variables ---

    /*!
        Termination control
    */
    bool m_bQuit;

    std::list<multicastChannelItem *>
      m_channels; // List containing multicast channels

  private:
    CControlObject *m_pObj;

    /// Multicast channel Client item
    CClientItem *m_pClientItem;

    // Groupaddess + port to send on
    // default: udp://224.0.23.158:44444
    // std::string m_sendAddress;

    // Socket for sending
    int m_sendSock;

    // Send address
    struct sockaddr_in m_mc_sendAddr;
};

#endif