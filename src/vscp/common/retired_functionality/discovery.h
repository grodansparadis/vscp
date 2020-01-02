// discovery.h
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

#if !defined(VSCP_DISCOVERY_H__INCLUDED_)
#define VSCP_DISCOVERY_H__INCLUDED_

#include <guid.h>

class cvscpnode;
class CClientItem;
class CControlObject;

// This thread is used for node discovery
void *
nodeDiscoveryThread(void *threadData);

// This structure holds information about nodes that need to have
// information collected from them.

struct discoveredNodeInfo
{
    int bStatus;         // 0 = working, -1=failed, 777=success.
    pthread_t *pThread;  // Discover thread
    uint8_t nodeid;      // nodeid for the node to investigate
    uint32_t clientId;   // Clientid for node to investigate
    cguid guid;          // GUID for node.
    std::string mdfPath; // MDF path for node.
};

// This object holds data for the discovery thread
class CDiscoveryObj
{

  public:
    /// Constructor
    CDiscoveryObj();

    /// Destructor
    ~CDiscoveryObj();

    /*!
        Send event to client with specific id
    */
    bool sendEvent(vscpEvent *pEvent, uint32_t obid);

    /*!
        Read a level I register
    */
    bool readLevel1Register(uint8_t nodeid,
                            uint8_t reg,
                            uint8_t *pcontent,
                            uint32_t clientID,
                            uint32_t timeout = 1000);

    /*!
        Add a known node if it is not already known
        @param pEvent Event that initiated the add
        @return Pointer to new or old information object or NULL if failure.
    */
    cvscpnode *addNodeIfNotKnown(vscpEvent *pEvent);

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

#endif