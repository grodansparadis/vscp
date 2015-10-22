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

// This class holds information about a known node. This can be 
// either a node that is assigned as known from the configuration
// file or a node that is discovered.

class CNodeInformation
{

public:

    CNodeInformation();
    ~CNodeInformation();

private:

    /*!
    Client originating this
    Event
    */
    CClientItem *m_pClientItem;

    // Full GUID for node
    cguid m_guid;

    // MDF path for node
    wxString m_mdfPath;

    // Last heartbeat from this node
    wxDateTime m_lastHeartBeat;

    // Standard registers
    uint8_t m_reg[ 0x80 ];

    // This is the name of the node if any.
    // Will be truncated to 64 byte when sent out or reported
    wxString strName;
};


WX_DECLARE_LIST( cnodeInformation, VSCP_KNOWN_NODES_LIST );


class CKnownNodes
{

public:

    /*!
        Find node
    */
    cnodeInformation *findKnownNode( cguid& guid );

    /*!
        Save the list of known nodes
    */
    void saveKnownNodes( void );


    /*!
        Load the list of known nodes
    */
    void loadKnownNodes( void );

    // This array holds guid's for nodes that
    // we have received a heartbeat from. The GUID
    // is not always the nodes actual GUID but often 
    // the interface GUID plus nickname.
    wxArrayString m_knownGUIDs;

    // This list holds information about each discovered
    // node in the system and the content in the list
    // is preserved in the filesystem over time
    VSCP_KNOWN_NODES_LIST m_knownNodes;

};


#endif
