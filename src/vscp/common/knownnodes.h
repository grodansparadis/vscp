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
    NODE_TYPE_COMMON_GUID,  // Common GUID.
    NODE_TYPE_INTERFACE,    // Interface on this machine,
    NODE_TYPE_HARDWARE_L1,  // Level I hardware. Lives on one of the interfaces of this daemon.
    NODE_TYPE_HARDWARE_L2,  // Level II hardware. Have an address.
    NODE_TYPE_HARDWARE_L3,  // Level III hardware. Software/driver pretending to be hardware
    NODE_TYPE_DRIVER,       // Driver
    NODE_TYPE_LOCATION,     // Location.
};

///////////////////////////////////////////////////////////////////////////////
// CVSCPServerInformation
//
// This class holds information about a known server in the system.
//

class CVSCPServerInformation
{

public:

    CVSCPServerInformation();
    ~CVSCPServerInformation();

    CVSCPServerInformation& operator=( const CVSCPServerInformation& node ) 
    {
        // Check for self-assignment!
        if ( this == &node ) {      // Same object?
            return *this;           // Yes, so skip assignment, and just return *this.
        }

        m_bUpdated = node.m_bUpdated;
        m_capabilities = node.m_capabilities;
        m_guid = node.m_guid;
        m_nameOfServer = node.m_nameOfServer;
        m_ipaddress = node.m_ipaddress;
        memcpy( m_ports, node.m_ports, sizeof( m_ports ) );

        return *this;
    };

    // This flag can be used by a client to mark this item as seen
    // by an update routine.
    bool m_bUpdated;

    // Capabilities for this server
    uint64_t m_capabilities;    

    // GUID for server
    cguid m_guid;

    // Name of server
    wxString m_nameOfServer;

    // Address for server (IPv4 or IPv6)
    wxString m_ipaddress;

    // Ports related to services.
    int m_ports[ 64 ];

};

// GUID -> Server info
WX_DECLARE_STRING_HASH_MAP( CVSCPServerInformation*, VSCP_HASH_KNOWN_SERVERS );

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
// ClientObject can be received from clientid' which also is set in obid of the event.
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

    CNodeInformation& operator=( const CNodeInformation& node )
    {
        // Check for self-assignment!
        if ( this == &node ) {	// Same object?
            return *this;	// Yes, so skip assignment, and just return *this.
        }

        m_bUpdated = node.m_bUpdated;
        m_bInvestigated = node.m_bInvestigated;
        m_bProxy = node.m_bProxy;
        m_realguid = node.m_realguid;
        m_interfaceguid = node.m_interfaceguid;
        m_clientID = node.m_clientID;
        m_mdfPath = node.m_mdfPath;
        m_lastHeartBeat = node.m_lastHeartBeat;
        memcpy( m_stdreg, node.m_stdreg, sizeof( m_stdreg ) );
        m_strNodeName = node.m_strNodeName;
        m_deviceName = node.m_deviceName;
        m_address = node.m_address;
        m_level = node.m_level;

        return *this;
    };

    // This flag can be used by a client to mark this item as seen
    // by an update routine.
    bool m_bUpdated;

    // True if this node has been checked
    bool m_bInvestigated;

    // This flag is true if a node is using one or several
    // interface guid's + nicknames. That is communication should
    // be done through the proxy guid not the realguid
    bool m_bProxy;

    // Full GUID for node (real GUID)
    cguid m_realguid;

    // Full GUID for interface 
    cguid m_interfaceguid;

    // Client id - Used by the VSCP daemon
    uint32_t m_clientID;

    // MDF path for node
    wxString m_mdfPath;

    // Last heartbeat from this node
    wxDateTime m_lastHeartBeat;

    // Standard registers
    uint8_t m_stdreg[ 0x80 ];

    // This is the name of the node if any.
    // Will be truncated to 64 byte when sent out or reported
    wxString m_strNodeName;

    // If a driver is associated with the 
    wxString m_deviceName;

    // IP/MAC/... address (if any) associated with the node
    wxString m_address;

    // Node level 1=Level I, 2 = Level II, 3 = Level III
    uint8_t m_level;

};

// GUID -> Node info
WX_DECLARE_STRING_HASH_MAP( CNodeInformation*, VSCP_HASH_KNOWN_NODES );


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

        m_nodeType = node.m_nodeType;
        m_bUpdated = node.m_bUpdated;
        m_bInvestigated = node.m_bInvestigated;
        m_bProxy = node.m_bProxy;
        m_realguid = node.m_realguid;
        m_interfaceguid = node.m_interfaceguid;
        m_clientID = node.m_clientID;
        m_mdfPath = node.m_mdfPath;
        m_lastHeartBeat = node.m_lastHeartBeat;
        memcpy( m_stdreg, node.m_stdreg, sizeof( m_stdreg ) );
        m_strNodeName = node.m_strNodeName;
        m_deviceName = node.m_deviceName;
        m_address = node.m_address;
        m_level = node.m_level;
        
        // Server
        m_capabilities = node.m_capabilities;
        memcpy( (void *)m_ports, (void *)node.m_ports, 64 * sizeof( int ) );
        
        return *this;
    };

    // This flag can be used by a client to mark this item as seen
    // by an update routine.
    bool m_bUpdated;
    
    // This is the type of node
    uint8_t m_nodeType;
    
    // Capabilities for server
    uint64_t m_capabilities; 
    
    // Ports related to services.
    int m_ports[ 64 ];

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

    // MDF path for node
    wxString m_mdfPath;

    // Last heartbeat from this node
    wxDateTime m_lastHeartBeat;

    // Standard registers
    uint8_t m_stdreg[ 0x80 ];

    // This is the name of the node if any.
    // Will be truncated to 64 byte when sent out or reported
    wxString m_strNodeName;

    // If a driver is associated with the 
    wxString m_deviceName;

    // IP/MAC/... address (if any) associated with the node
    wxString m_address;

    // Node level 0=Level I, 1 = Level II
    uint8_t m_level;    
};


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
        Find server
        @param guid GUID for server.
        @return Return CServerInformation class if found, NULL if not.
    */
    CVSCPServerInformation *findServer( cguid& guid );

    /*!
        Add node
        @param guid GUID for node.
        @return Return CNodeInformation class if added, NULL if not.
    */
    CNodeInformation *addNode( cguid& guid );

    /*!
        Add server
        @param guid GUID for server.
        @return Return CVSCPServerInformation class if added, NULL if not.
    */
    CVSCPServerInformation *addServer( cguid& guid );

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
        Save the list of known nodes
    */
    void save( wxString& path );

    /*!
        Load the list of known nodes
    */
    void load( wxString& path );

    // Protect the lists
    wxMutex m_mutexKnownNodes;

    // This list holds information about each discovered
    // node in the system and. 
    // The content of the list can be
    // preserved in the filesystem over time
    VSCP_HASH_KNOWN_NODES m_nodes;

    // This list holds information about each discovered
    // "server" in the system. A server is defines as 
    // a high level node with at least one connectable interface.
    // The content of the list can be
    // preserved in the filesystem over time
    VSCP_HASH_KNOWN_SERVERS m_servers;

};


#endif
