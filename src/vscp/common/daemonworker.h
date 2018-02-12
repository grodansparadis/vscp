// daemon_VSCP.h
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


#if !defined(DAEMONVSCP_H__INCLUDED_)
#define DAEMONVSCP_H__INCLUDED_


#include "wx/thread.h"
#include "wx/socket.h"
#include <wx/datetime.h>

#include "guid.h"


class CControlObject;
class CClientItem;
class CVSCPNode;

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
    CVSCPNode *addNodeIfNotKnown( vscpEvent *pEvent );

    /*!
        Send multicast event
        @param sock Multicast socket to send on
        @param pEvent Event to send
        @return true on success, false on failure
    */
    bool sendMulticastEvent( int sock, vscpEvent *pEvent  );

    /*!
        Send multicast event Ex
        @param sock Multicast socket to send on
        @param pEventEx Event ex to send
        @return true on success, false on failure
    */
    bool sendMulticastEventEx( int sock, vscpEventEx *pEventEx );

    /*!
        Send multicast information event
        @param sock Multicast socket to send on
        @param pNode Pointer to information node.
        @return true on success, false on failure
    */
    bool sendMulticastInformationProxyEvent( int sock, 
                                                CVSCPNode *pNode );

    /*!
        Termination control
    */
    bool m_bQuit;

    CControlObject *m_pCtrlObject;

    // This list contains items (nodes) that are under discovery
    DISCOVERYLIST m_discoverList;

};




#endif



