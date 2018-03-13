// FILE: knownnodes.h 
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


#pragma once

#ifndef _KNOWNNODES_H_
#define _KNOWNNODES_H_

#include <wx/listimpl.cpp>
#include "guid.h"

// The different types of nodes
enum {
    NODE_TYPE_COMMON_GUID = 0,  // Common GUID.
    NODE_TYPE_INTERFACE,        // Interface on this machine,
    NODE_TYPE_HARDWARE_L1,      // Level I hardware. Lives on one of the interfaces of this daemon.
    NODE_TYPE_HARDWARE_L2,      // Level II hardware. Have an address.
    NODE_TYPE_HARDWARE_L3,      // Level III hardware. Software/driver pretending to be hardware
    NODE_TYPE_DUMB,             // Dumb node (no registers, no MDF)
    NODE_TYPE_DRIVER,           // Driver
    NODE_TYPE_LOCATION,         // Location.
};


///////////////////////////////////////////////////////////////////////////////
// CNodeInformation
//
// This class holds information about a known node. This can be 
// either a node that is assigned as known from the configuration
// file or a node that is discovered. It can be a server or a client node.
//
// A level II node always use it's own guid. A Level I node on the
// other hand may be using an interface guid + nickname. For such a node, when
// a heart beat is received, an entry is first made for the proxy id set as real-id. 
// Another entry may be added when a node has been discovered. The proxy node will have 
// the proxy flag set and the interface guid will be set to the interface part of the
// guid (12 MSB bytes with 4 LSB bytes set to zero). The real node will not have the
// proxy flag set and the interface will be set to the interface it is on.
//
// ClientObject can be received from clientid' which also is set in obid of the event.
//
// DeviceObject can be fetched from the client id with the getDeviceItemFromClientId method
// in the devicelist class.
//
// Names are formed from the interface they are at as 'interfacename.nodename' and if
// the node does not have a name 'node'+nickname id.
//

class CVSCPNode
{

public:

    CVSCPNode();
    ~CVSCPNode();

    CVSCPNode& operator=( const CVSCPNode& node )
    {
        // Check for self-assignment!
        if ( this == &node ) {	// Same object?
            return *this;	// Yes, so skip assignment, 
                            // and just return *this.
        }

        m_dbId = node.m_dbId;
        m_nodeType = node.m_nodeType;
        m_bUpdated = node.m_bUpdated;
        m_bInvestigated = node.m_bInvestigated;
        m_bProxy = node.m_bProxy;
        m_realguid = node.m_realguid;
        m_interfaceguid = node.m_interfaceguid;
        m_clientID = node.m_clientID;
        lint_to_mdf = node.lint_to_mdf;
        m_lastHeartBeat = node.m_lastHeartBeat;
        m_strNodeName = node.m_strNodeName;
        m_deviceName = node.m_deviceName;
        m_address = node.m_address;
        m_level = node.m_level;
        
        // Server
        m_capabilities = node.m_capabilities;
        memcpy( (void *)m_ports, (void *)node.m_ports, sizeof( m_ports ) );
        
        return *this;
    };
    
    /*!
     * Write capabilities as a comma separated string
     * @param strCapabilities String to write result to.
     */
    void writeCapabilitiesToString( wxString& strCapabilities );
    
    /*!
     * Get node capabilities from a comma separated string
     * @param strCapabilities Capabilities in a comma separated string.
     */
    void getCapabilitiesFromString( const wxString& strCapabilities );
    
    
    
    // Database id
    long m_dbId;
    
    // This flag can be used by a client to mark this item as seen
    // by an update routine.
    bool m_bUpdated;
    
    // This is the type of node
    uint8_t m_nodeType;
    
    // Capabilities for server
    uint64_t m_capabilities; 
    
    // Ports related to services.
    uint16_t m_ports[ 64 ];

    // True if this node has been checked
    bool m_bInvestigated;

    // This flag is true if a node is using one or several
    // interface guid's + nicknames. That is communication should
    // be done through the proxy guid not the real guid
    bool m_bProxy;

    // Full GUID for node (real GUID)
    cguid m_realguid;

    // Full GUID for interface 
    cguid m_interfaceguid;

    // Client id - Used by the VSCP daemon
    uint32_t m_clientID;

    // Link to MDF record
    uint32_t lint_to_mdf;

    // Last heartbeat from this node
    wxDateTime m_lastHeartBeat;

    // This is the name of the node if any.
    // Will be truncated to 64 byte when sent out or reported
    wxString m_strNodeName;

    // If a driver is associated with the 
    wxString m_deviceName;

    // IP/MAC/... address (if any) associated with the node
    wxString m_address;

    // Node level 0 = unknown, 1 = Level I, 2 = Level II, 3 = Level III
    uint8_t m_level;    
};


// GUID -> Node info
WX_DECLARE_STRING_HASH_MAP( CVSCPNode*, VSCP_HASH_KNOWN_NODES );


///////////////////////////////////////////////////////////////////////////////
// CKnownNodes
//
// Holds information about all discovered and defined nodes in the system
//

class CKnownNodes
{

public:

    CKnownNodes();
    ~CKnownNodes();

    /*!
        Find node
        @param guid GUID for node.
        @return Return CVSCPNode class if found, NULL if not.
    */
    CVSCPNode *findNode( cguid& guid );


    /*!
        Add node
        @param guid GUID for node.
        @return Return CVSCPNode class if added or existing, NULL if error.
    */
    CVSCPNode *addNode( cguid& guid );


    /*!
        Remove node
        @param guid GUID for node.
        @return Return true on success, false on failure.
    */
    bool removeNode( cguid& guid );

    /*!
        Remove server
        @param guid GUID for node.
        @return Return true on success, false on failure.
    */
    bool removeServer( cguid& guid );

    /*!
        Save known nodes to database
    */
    void save( void );

    /*!
        Load known nodes from database
    */
    bool load( void );

    // Protect the lists
    wxMutex m_mutexKnownNodes;

    // This list holds information about each discovered
    // node in the system and. 
    // The content of the list can be
    // preserved in the filesystem over time
    VSCP_HASH_KNOWN_NODES m_nodes;


};


#endif
