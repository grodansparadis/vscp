// FILE: knownnodes.h 
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (c) 2000-2015 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
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


#pragma once

#ifndef _KNOWNNODES_H_
#define _KNOWNNODES_H_

#include <wx/listimpl.cpp>
#include "guid.h"

class CControlObject;
class CClientItem;

///////////////////////////////////////////////////////////////////////////////
// CNodeInformation
//
// This class holds information about a known node. This can be 
// either a node that is assigned as known from the configuration
// file or a node that is discovered.
//
// A level II node always use it's own guid. A Level I node on the
// other hand may be using an interface guid + nickname. For such a node, when
// a heart beat is received, an entry is first made for the proxy id set as realid. 
// Another entry may be added when a node has been discovered. The proxy node will have 
// the proxy flag set and the interface guid will be set to the interface part of the
// guid (12 MSB bytes with 4 LSB bytes set to zero). The real node will not have the
// proxy flag set and the interface will be set to the interface it is on.
//
// ClientObject can be received from clientid which also is set in obid of the event.
//
// DeviceObject can be fetched from the client id with the getDeviceItemFromClientId method
// in the devicelist class.
//
// Names are formed from the interface they are at as 'interfacename.nodename' and if
// the node does not have a name 'node'+nickname id.

class CNodeInformation
{

public:

    CNodeInformation();
    ~CNodeInformation();

    // Thru if this node has been checked
    bool m_bInvestigated;

    // This flag is true if a node is using one or several
    // interface guid's + nicknames. That is communication should
    // be done through the proxy guid not the realguid
    bool m_bProxy;

    // Full GUID for node (real GUID)
    cguid m_realguid;

    // Full GUID for interface + nickname the node is on
    cguid m_proxyguid;

    // MDF path for node
    wxString m_mdfPath;

    // Last heartbeat from this node
    wxDateTime m_lastHeartBeat;

    // Standard registers
    uint8_t m_stdreg[ 0x80 ];

    // This is the name of the node if any.
    // Will be truncated to 64 byte when sent out or reported
    wxString strName;
};


WX_DECLARE_STRING_HASH_MAP( CNodeInformation*, VSCP_HASH_KNOWN_NODES );


///////////////////////////////////////////////////////////////////////////////
// CKnownNodes
//

class CKnownNodes
{

public:

    CKnownNodes();
    ~CKnownNodes();

    /*!
        Find node
        @param guid GUID for node.
        @return Return CNodeInformation class if found, NULL if not.
    */
    CNodeInformation *findNode( cguid& guid );

    /*!
        Add node
        @param guid GUID for node.
        @return Return CNodeInformation class if added, NULL if not.
    */
    CNodeInformation *addNode( cguid& guid );

    /*!
        Remove node
        @param guid GUID for node.
        @return Return true on success, false on failure.
    */
    bool removeNode( cguid& guid );

    /*!
        Save the list of known nodes
    */
    void saveNodes( void );

    /*!
        Load the list of known nodes
    */
    void loadNodes( void );

private:

    // Protect the lists
    wxMutex m_mutexKnownNodes;

    // This list holds information about each discovered
    // node in the system and the content in the list
    // is preserved in the filesystem over time
    VSCP_HASH_KNOWN_NODES m_knownNodes;

};


#endif
