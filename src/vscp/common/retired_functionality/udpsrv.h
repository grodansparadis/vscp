// udpclientthread.h
//
// This file is part of the VSCP (http://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2020 Ake Hedman, Grodans Paradis AB
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

#if !defined(UDPCLIENTTHREAD_H__INCLUDED_)
#define UDPCLIENTTHREAD_H__INCLUDED_

#include "controlobject.h"
#include "userlist.h"

// Prototypes
void *
UDPThread(void *pData);

// Client structure (Will receive events from VSCP server)
typedef struct
{
    bool m_bEnable; // Enable the client
    std::string
      m_remoteAddress; // Address for remote client UDP://192.168.1.99:1234
    vscpEventFilter m_filter; // Filter for outgoing events to this UDP client
    uint8_t m_nEncryption;    // Encryption algorithm to use for this client
    bool m_bSetBroadcast;     // Set broadcast flag  MG_F_ENABLE_BROADCAST
    uint8_t m_index;          // Rolling send index (only low tree bits used)
} udpRemoteClientInfo;

typedef struct
{
    uint8_t m_index;     // Frame index
    vscpEvent *m_pEvent; // Event to send
    uint32_t m_lastSent; // Last sent
    uint8_t m_nRetry;    // Retry counter
} eventResendItem;

// XX_DECLARE_LIST(eventResendItem, udpResendItemList);

/*!
    This class implement the listen thread for
    the vscpd connections on the UDP interface
*/

class UDPSrvObj
{

  public:
    /// Constructor
    UDPSrvObj(CControlObject *pobj = NULL);

    /// Destructor
    ~UDPSrvObj();

    /*!
     *  Connection handler
     *
     * @param nc Mongoose connection handle
     * @param ev Mongoose event.
     * @param p Pointer to user data.
     */
    static void ev_handler(struct mg_connection *nc, int ev, void *p);

    /*!
     * Receive UDP frame
     *
     * @param nc Mongoose connection handle
     * @param pClientItem Client item for this user. Normally "USP"
     * @param pRxFilter Pointer to receive filter. Can be NULL to accept
     *          all events.
     * @param index Running packet index.-
     * @return True on success, false on failure.
     */
    static bool receiveFrame(struct mg_connection *nc,
                             CClientItem *pClientItem,
                             vscpEventFilter *pRxFilter);

    /*!
     *  Send ACk reply
     *
     *  @pram nc Pointer to connection
     *  @param pkttype Packet type
     *  @param index Running index 0-7
     */
    static bool replyAckFrame(struct mg_connection *nc,
                              uint8_t pkttype,
                              uint8_t index);

    /*!
     *  Send NACk reply
     *
     *  @pram nc Pointer to connection
     *  @param pkttype Packet type
     *  @param index Running index 0-7
     */
    static bool replyNackFrame(struct mg_connection *nc,
                               uint8_t pkttype,
                               uint8_t index);

    /*!
     * Send frame to all clients
     *
     * @param pmgt Pointer to communication structure
     * @param pClientItem Pointer to client
     * @return True on success false on failure
     */
    bool sendFrame(struct mg_mgr *pmgr, CClientItem *pClientItem);

    /*!
        Assign control object
    */
    void setControlObjectPointer(CControlObject *pobj) { m_pobj = pobj; };

    // --- Member variables ---

    /*!
     *  Mutex that protect the UDP info structure
     */
    pthread_mutex_t m_mutexUDPInfo;

    bool m_bQuit;

    // TODO move private????

    bool m_bEnable;           // Enable UDP
    bool m_bAllowUnsecure;    // Allow un encrypted datagrams
    bool m_bAck;              // ACK received datagram
    std::string m_user;       // User account to use for UDP
    std::string m_password;   // Password for user account
    std::string m_interface;  // Interface to bind to "UDP://33333"
    cguid m_guid;             // GUID to use for server client object
    vscpEventFilter m_filter; // Filter for incoming events to this UDP client
    std::deque<udpRemoteClientInfo *>
      m_remotes; // List containing remote receiving clients

  private:
    // UDP Client item
    CClientItem *m_pClientItem;

    // The global control object
    CControlObject *m_pobj;
};

/*!
    This class implement a worker thread that
    logs UDP received data.
*/

class UDPWorkerObj
{

  public:
    /// Constructor
    UDPWorkerObj();

    /// Destructor
    ~UDPWorkerObj();

    // --- Member variables ---

    /*!
        Termination control
    */
    bool m_bQuit;
};

#endif
