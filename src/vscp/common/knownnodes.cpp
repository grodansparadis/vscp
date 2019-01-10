// FILE: knownnodes.cpp
//
// This file is part of the VSCP (http://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2019 Ake Hedman, Grodans Paradis AB
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

#define _POSIX

#include <pthread.h>

#include <map>
#include <string>

#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

#include <clientlist.h>
#include <controlobject.h>
#include <crc8.h>
#include <daemonworker.h>
#include <dllist.h>
#include <guid.h>
#include <vscp.h>
#include <vscpdb.h>
#include <vscphelper.h>

#include "knownnodes.h"

///////////////////////////////////////////////////
//                 GLOBALS
///////////////////////////////////////////////////

extern CControlObject *gpobj;

///////////////////////////////////////////////////////////////////////////////
//                           Node information
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CNodeInformation CTOR
//

cvscpnode::cvscpnode()
{
    m_dbId          = -1;
    m_bUpdated      = false;
    m_bInvestigated = false;
    m_realguid.clear();
    m_interfaceguid.clear();
    lint_to_mdf     = 0;
    m_lastHeartBeat = vscpdatetime::Now();
    m_strNodeName;
    m_deviceName;
    m_clientID = 0;
    m_level    = 0;

    memset(m_ports, 0, sizeof(m_ports));

    m_ports[15] = VSCP_DEFAULT_TCP_PORT;
    m_ports[14] = VSCP_DEFAULT_UDP_PORT;
    m_ports[13] = VSCP_ANNOUNCE_MULTICAST_PORT;
    m_ports[12] = 0;    // Raw Ethernet, no port.
    m_ports[11] = 8884; // web.
    m_ports[10] = 8884; // websocket.
    m_ports[9]  = 8884; // rest, no port.
    m_ports[8]  = VSCP_DEFAULT_MULTICAST_PORT;
    m_ports[7]  = 0; // reserved
    m_ports[6]  = 0; // IP6, no port.
    m_ports[5]  = 0; // IP4, no port.
    m_ports[4]  = 0; // SSL (web/websocket/rest port)
    m_ports[3]  = 0; // Two connections, no port.
    m_ports[2]  = 0; // AES256, no port.
    m_ports[1]  = 0; // AES192, no port.
    m_ports[0]  = 0; // AES128, no port.
}

///////////////////////////////////////////////////////////////////////////////
// cvscpnode DTOR
//

cvscpnode::~cvscpnode()
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
// getCapabilitiesFromString
//

void
cvscpnode::getCapabilitiesFromString(const std::string &strCapabilities)
{
    uint64_t caps = 0;
    std::deque<std::string> tokens;
    vscp_split(tokens, strCapabilities, ",");

    for (int i = 7; i > 0; i--) {
        if (!tokens.empty()) {
            uint8_t val = vscp_readStringValue(tokens.front());
            tokens.pop_front();
            caps |= (val << (i * 8));
        }
    }

    m_capabilities = caps;
}

///////////////////////////////////////////////////////////////////////////////
// getCapabilitiesFromString
//

void
cvscpnode::writeCapabilitiesToString(std::string &strCapabilities)
{
    strCapabilities =
      vscp_str_format("%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x",
                         (m_capabilities >> 56) & 0xff,
                         (m_capabilities >> 48) & 0xff,
                         (m_capabilities >> 40) & 0xff,
                         (m_capabilities >> 32) & 0xff,
                         (m_capabilities >> 24) & 0xff,
                         (m_capabilities >> 16) & 0xff,
                         (m_capabilities >> 8) & 0xff,
                         m_capabilities & 0xff);
}

///////////////////////////////////////////////////////////////////////////////
// CKnownNodes CTOR
//

CKnownNodes::CKnownNodes() {}

///////////////////////////////////////////////////////////////////////////////
// CKnownNodes DTOR
//

CKnownNodes::~CKnownNodes()
{
    // Remove hash content
    {
        std::map<std::string, cvscpnode *>::iterator it;
        for (it = m_nodes.begin(); it != m_nodes.end(); ++it) {
            std::string key  = it->first;
            cvscpnode *pNode = it->second;
            if (NULL != pNode) delete pNode;
            pNode = NULL;
        }
    }

    // Clear the map
    m_nodes.clear();
}

///////////////////////////////////////////////////////////////////////////////
// findNode
//

cvscpnode *
CKnownNodes::findNode(cguid &guid)
{
    std::string strGUID;

    guid.toString(strGUID);
    return m_nodes[strGUID];
}

///////////////////////////////////////////////////////////////////////////////
// addNode
//

cvscpnode *
CKnownNodes::addNode(cguid &guid)
{
    cvscpnode *pNode = findNode(guid);
    if (NULL == pNode) {
        pNode = new cvscpnode;
        if (NULL != pNode) {
            std::string strGUID;
            guid.toString(strGUID);
            m_nodes[strGUID] = pNode; // Assign the node
        }
    }

    return pNode;
}

///////////////////////////////////////////////////////////////////////////////
// removeNode
//

bool
CKnownNodes::removeNode(cguid &guid)
{
    std::string strGUID;
    bool rv          = false;
    cvscpnode *pNode = findNode(guid);
    if (NULL != pNode) delete pNode;
    pNode = NULL;

    guid.toString(strGUID);
    m_nodes[strGUID] = NULL;
    m_nodes.erase(strGUID);

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// saveNodes
//

void
CKnownNodes::save(void)
{
    // TODO
}

///////////////////////////////////////////////////////////////////////////////
// loadNodes
//

bool
CKnownNodes::load(void)
{
    const char *p;
    char *pErrMsg;
    sqlite3_stmt *ppStmt;

    pthread_mutex_lock(&gpobj->m_knownNodes.m_mutexKnownNodes);

    if (SQLITE_OK !=
        sqlite3_prepare_v2(
          gpobj->m_db_vscp_daemon, VSCPDB_GUID_SELECT_ALL, -1, &ppStmt, NULL)) {
    }

    int i;
    while (SQLITE_ROW == sqlite3_step(ppStmt)) {

        // GUID ( Must be present
        if (NULL == (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_GUID_GUID))) {
            continue;
        }

        cguid guid;
        guid.getFromString(p);
        cvscpnode *pNode = addNode(guid);

        // id in database
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_GUID_ID))) {
            pNode->m_dbId = atol(p);
        }

        // Nodename - Database given name for node
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_GUID_NAME))) {
            pNode->m_strNodeName = std::string(p);
        }

        // Type
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_GUID_TYPE))) {
            pNode->m_nodeType = (uint8_t)atoi(p);
        }

        // Last heartbeat is set to discovery date here
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_GUID_DATE))) {
            pNode->m_lastHeartBeat.set(p);
        }

        // MDF link
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_GUID_LINK_TO_MDF))) {
            uint32_t lint_to_mdf = (uint32_t)atol(p);
        }

        // Address
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_GUID_ADDRESS))) {
            pNode->m_address = std::string(p);
        }

        // Capabilities
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_GUID_CAPABILITIES))) {
            pNode->getCapabilitiesFromString(p);
        }

        // Non-standard ports
        if (NULL != (p = (const char *)sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_GUID_NONSTANDARD))) {
            std::string str = std::string(p);
            // pNode-> ->m_capabilities = std::string( p );
        }
    }

    pthread_mutex_unlock(&gpobj->m_knownNodes.m_mutexKnownNodes);

    return true;
}

/*

#include <xx/XML/xml.h>
 #include <xx/protocol/http.h>
//[...]
        std::string link;
        xxHTTP http;

        http.SetTimeout(6);
        http.Connect(_T("example.com"));
        // PHP file sending XML content
        xxInputStream *httpStream = http.GetInputStream(_T("/file.php"));

        if (http.GetError() == xxPROTO_NOERR)
        {
                // will crash here, if xml content is not formatted PERFECTLY
                xxXmlDocument xml(*httpStream);

                xxXmlNode *node = xml.GetRoot()->GetChildren();
                while (node)
                {
                        if (node->GetName() == _T("tagname1"))
                                staticText1->SetLabel(node->GetNodeContent());
                        else if(node->GetName() == _T("tagname2"))
                                staticText2->SetLabel(node->GetNodeContent());

                        // [...]

                        node = node->GetNext();
                }
        }
        else
                xxMessageBox(_T("Can't connect!"));

        http.Close();
        xxDELETE(httpStream);

 */