// udpsrv.h
//
// This file is part of the VSCP (https://www.vscp.org)
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

#if !defined(UDPCLIENTTHREAD_H__INCLUDED_)
#define UDPCLIENTTHREAD_H__INCLUDED_

#include "controlobject.h"
#include "userlist.h"

// Prototypes
void *UdpReceiveWorkerThread(void *pData);
void *UdpSendWorkerThread(void *pData);

#define UDP_MAX_PACKAGE     2048

typedef struct
{
    uint8_t m_index;     // Frame index
    vscpEvent *m_pEvent; // Event to send
    uint32_t m_lastSent; // Last sent
    uint8_t m_nRetry;    // Retry counter
} eventResendItem;


// Remote UDP Client structure. One is defined for each
// remote client.
class udpRemoteClient
{
public:
    udpRemoteClient(CControlObject *pobj = NULL);
    ~udpRemoteClient();

    /*!
        Initialize the UDP remote client
        @param host Host to connect to
        @param port Port on host to connect to.
        @param nEncryption Encryption to use. None, AES128, AES192, AES256
        @param bBroadcast True to set broadcast bit on sent frame.
        @return VSCP_ERROR_SUCCESS if all goes well, otherwise error.
    */
    int init(std::string host, 
                short port = VSCP_DEFAULT_UDP_PORT, 
                uint8_t nEncryption = VSCP_ENCRYPTION_NONE, 
                bool bSetBroadcast = false);

    /*!
        Send event to UDP remote client
    */
    int sendFrame(void);

    // Assign control object
    void setControlObjectPointer(CControlObject *pCtrlObj) { m_pCtrlObj = pCtrlObj; };

private:

    struct sockaddr_in m_to;
    int m_sockfd;                 // Sending socket

    uint8_t m_nEncryption;        // Encryption algorithm to use for this client
    bool m_bSetBroadcast;         // Set broadcast flag  MG_F_ENABLE_BROADCAST
    uint8_t m_index;              // Rolling send index (only low tree bits used)    

    CClientItem *m_pClientItem;   // Client we are working as     

    // The global control object
    CControlObject *m_pCtrlObj;
};


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
     * Receive UDP frame
     *
     * @param sockfd UDP socket descriptor for listening socket.
     * @param pClientItem Client item for this user. Normally "UDP"
     * @return VSCP_ERROR_SUCCESS on success, errorcode on failure.
     */
    int receiveFrame(int sockfd,
                      CClientItem *pClientItem);

    /*!
     *  Send ACK reply
     *
     *  @param pkttype Packet type
     *  @param index Running index 0-7
     *  @return VSCP_ERROR_SUCCESS on success, errorcode on failure.
     */
    int replyAckFrame(struct sockaddr *to, uint8_t pkttype, uint8_t index);

    /*!
     *  Send NACK reply
     *
     *  @param pkttype Packet type
     *  @param index Running index 0-7
     *  @return VSCP_ERROR_SUCCESS on success, errorcode on failure.
     */
    int replyNackFrame(struct sockaddr *to, uint8_t pkttype,uint8_t index);

    // Assign control object
    void setControlObjectPointer(CControlObject *pCtrlObj) { m_pCtrlObj = pCtrlObj; };

    // --- Member variables ---

    // Mutex that protect the UDP info structure
    pthread_mutex_t m_mutexUDPInfo;

    bool m_bQuit;

    bool m_bEnable;                 // Enable UDP
    struct sockaddr_in m_servaddr;  // Bind address + port
    int m_sockfd;                   // Socket used to send ACK/NACK)
    bool m_bAllowUnsecure;          // Allow un encrypted datagrams
    bool m_bAck;                    // ACK received datagram
    std::string m_user;             // User account to use for UDP
    std::string m_password;         // Password for user account
    std::string m_interface;        // Interface to bind to "UDP://33333"
    cguid m_guid;                   // GUID to use for server client object
    vscpEventFilter m_filter;       // Filter for incoming events to this UDP client

    // UDP Client item
    CClientItem *m_pClientItem;

    // The global control object
    CControlObject *m_pCtrlObj;
};


#endif
