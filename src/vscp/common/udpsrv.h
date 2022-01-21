// udpsrv.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2022 Ake Hedman, the VSCP project
// <info@vscp.org>
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

#include <guid.h>
#include <vscp.h>

#include <arpa/inet.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>

#include <string>

class CControlObject;
class CClientItem;

// Prototypes
void*
udpSrvWorkerThread(void* pData);
void*
udpClientWorkerThread(void* pData);

#define UDP_MAX_PACKAGE 2048

// Remote UDP Client structure. One is defined for each
// remote client.
class udpRemoteClient {

  public:
    /*!
        CTOR
        @param Pointer to control object
    */
    udpRemoteClient();

    // DTOR
    ~udpRemoteClient();

    /*!
        Initialize the UDP remote client
        @param pobj Pointer to control object
        @param host Host to connect to
        @param port Port on host to connect to.
        @param nEncryption Encryption to use. None, AES128, AES192, AES256
        @param bBroadcast True to set broadcast bit on sent frame.
        @return VSCP_ERROR_SUCCESS if all goes well, otherwise error.
    */
    int init(CControlObject* pobj,
             uint8_t nEncryption = VSCP_ENCRYPTION_NONE,
             bool bSetBroadcast  = false);

    /*!
        Start the client worker thread
        @return True on success, false on failure.
    */
    bool startWorkerThread(void);

    /*!
        Send event to UDP remote client
    */
    int sendFrame(void);

    // Getter/setters
    void enable(bool bEnable = true) { m_bEnable = bEnable; }

    void setRemoteAddress(const std::string& remoteAddress)
    {
        m_remoteAddress = remoteAddress;
    }
    std::string getRemoteAddress(void) { return m_remoteAddress; }

    void setRemotePort(int16_t remotePort) { m_remotePort = remotePort; }
    int16_t getRemotePort(void) { return m_remotePort; }

    void setUser(const std::string& user) { m_user = user; }
    std::string getUser(void) { return m_user; }

    void setEncryption(uint8_t encryption = VSCP_ENCRYPTION_NONE)
    {
        if (encryption <= VSCP_ENCRYPTION_AES256)
            m_nEncryption = encryption;
    };

    uint8_t getEncryption(void) { return m_nEncryption; }

    void SetBroadcastBit(bool enable = true) { m_bSetBroadcast = enable; }
    bool isBroadCastBitSet(void) { return m_bSetBroadcast; }

    void setFilter(const vscpEventFilter& filter)
    {
        memcpy(&m_filter, &filter, sizeof(vscpEventFilter));
    }

    // Assign control object
    void setControlObjectPointer(CControlObject* pCtrlObj)
    {
        m_pCtrlObj = pCtrlObj;
    }

  public:

    // Client thread will run as long as false
    bool m_bQuit;

    // Client we are working as
    CClientItem* m_pClientItem;

    // The global control object
    CControlObject* m_pCtrlObj;

  public:
    bool m_bEnable;

    int m_sockfd;
    struct sockaddr_in m_clientAddress;

    // Interface to connect to
    std::string m_remoteAddress;
    int16_t m_remotePort;

    vscpEventFilter m_filter; // Outgoing filter for this remote client.
    cguid m_guid;             // GUID for outgoing channel.

    std::string m_user;     // user the client act as
    std::string m_password; // Password for user we act as

    uint8_t m_nEncryption; // Encryption algorithm
    bool m_bSetBroadcast;  // Set broadcast flag  MG_F_ENABLE_BROADCAST.
    uint8_t m_index;       // Rolling send index
                           // (only low tree bits used).

    // The thread that do the actual sending
    pthread_t m_udpClientWorkerThread;
};

/*!
    This class implement the listen thread for
    the vscpd connections on the UDP interface
*/

class udpSrvObj {

  public:
    /// Constructor
    udpSrvObj();

    /// Destructor
    ~udpSrvObj();

    /*!
        Init UDP server object

        @param pobj Pointer to VSCP daemon control object
    */
    void init(CControlObject* pobj) { m_pCtrlObj = pobj; }

    /*!
     * Receive UDP frame
     *
     * @param sockfd UDP socket descriptor for listening socket.
     * @param pClientItem Client item for this user. Normally "UDP"
     * @return VSCP_ERROR_SUCCESS on success, errorcode on failure.
     */
    int receiveFrame(int sockfd, CClientItem* pClientItem);

    /*!
     *  Send ACK reply
     *
     *  @param pkttype Packet type
     *  @param index Running index 0-7
     *  @return VSCP_ERROR_SUCCESS on success, errorcode on failure.
     */
    int replyAckFrame(struct sockaddr* to, uint8_t pkttype, uint8_t index);

    /*!
     *  Send NACK reply
     *
     *  @param pkttype Packet type
     *  @param index Running index 0-7
     *  @return VSCP_ERROR_SUCCESS on success, errorcode on failure.
     */
    int replyNackFrame(struct sockaddr* to, uint8_t pkttype, uint8_t index);

    // Assign control object
    void setControlObjectPointer(CControlObject* pCtrlObj)
    {
        m_pCtrlObj = pCtrlObj;
    }

    // --- Member variables ---

    // Mutex that protect the UDP info structure
    pthread_mutex_t m_mutexUDPInfo;

    bool m_bQuit;

    bool m_bEnable;                // Enable UDP
    std::string m_interface;       // Interface to bind to "UDP://33333"
    struct sockaddr_in m_servaddr; // Bind address + port
    int m_sockfd;                  // Socket used to send ACK/NACK)

    bool m_bAllowUnsecure;    // Allow un encrypted datagrams
    bool m_bAck;              // ACK received datagram
    std::string m_user;       // User account to use for UDP
    std::string m_password;   // Password for user account
    cguid m_guid;             // GUID to use for server client object
    vscpEventFilter m_filter; // Filter for incoming events

    // UDP Client item
    CClientItem* m_pClientItem;

    // The global control object
    CControlObject* m_pCtrlObj;
};

#endif
