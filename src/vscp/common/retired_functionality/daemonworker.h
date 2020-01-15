// daemonworker.h
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

#if !defined(VSCP_DAEMON_WORKER_H__INCLUDED_)
#define VSCP_DAEMON_WORKER_H__INCLUDED_

#include <deque>

class CControlObject;
class CClientItem;
class cvscpnode;
class cguid;
class vscpdatetime;

// The actual worker thread
void *
daemonWorkerThread(void *pData);

/*!
    This class implement a one of thread that look
    for specific events and react on them appropriately.

*/

class daemonWorkerObj
{

  public:
    /// Constructor
    daemonWorkerObj(CControlObject *pObj);

    /// Destructor
    ~daemonWorkerObj();

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

    bool parseInterface(const std::string &ifaddr,
                        std::string &ip,
                        unsigned short *pPort);

    /*!
        Send multicast event
        @param sock Multicast socket to send on
        @param pEvent Event to send
        @return true on success, false on failure
    */
    bool sendMulticastEvent(int sock, vscpEvent *pEvent);

    /*!
        Send multicast event Ex
        @param sock Multicast socket to send on
        @param pEventEx Event ex to send
        @return true on success, false on failure
    */
    bool sendMulticastEventEx(int sock, vscpEventEx *pEventEx);

    /*!
        Send multicast information event
        @param sock Multicast socket to send on
        @param pNode Pointer to information node.
        @return true on success, false on failure
    */
    bool sendMulticastInformationProxyEvent(int sock, cvscpnode *pNode);

    // This object must be allocated externally
    //CDiscoveryObj *m_pDiscoveryObj;

    /*!
        Termination control
    */
    bool m_bQuit;

    CControlObject *m_pCtrlObject;

    // This list contains items (nodes) that are under discovery
    //std::deque<discoveredNodeInfo *> m_discoverList;
};

#endif
