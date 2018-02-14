// FILE: knownnodes.cpp 
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


#ifdef WIN32
#include <sys/types.h>   // for type definitions 
#include <winsock2.h>
#include <ws2tcpip.h>    // for win socket structs 
#endif

#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"
#include <wx/datetime.h>
#include <wx/listimpl.cpp>
#include <wx/hashmap.h>

#ifdef WIN32
#else

#define _POSIX

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#endif

#include "daemonworker.h"
#include "vscp.h"
#include "vscphelper.h"
#include "clientlist.h"
#include <dllist.h>
#include <crc8.h>
#include <vscpdb.h>
#include "controlobject.h"
#include "guid.h"
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

CVSCPNode::CVSCPNode()
{
    m_dbId = -1;
    m_bUpdated = false;
    m_bInvestigated = false;
    m_realguid.clear();
    m_interfaceguid.clear();
    lint_to_mdf = 0;
    m_lastHeartBeat = wxDateTime::Now();
    m_strNodeName.Empty();
    m_deviceName.Empty();
    m_clientID = 0;
    m_level = 0;
        
    memset( m_ports, 0, sizeof(m_ports) * sizeof(int) );
    
    m_ports[15] = VSCP_DEFAULT_TCP_PORT;
    m_ports[14] = VSCP_DEFAULT_UDP_PORT;    
    m_ports[13] = VSCP_ANNOUNCE_MULTICAST_PORT;
    m_ports[12] = 0; // Raw Ethernet, no port.
    m_ports[11] = 8884; // web.
    m_ports[10] = 8884; // websocket.
    m_ports[9] = 8884;  // rest, no port.  
    m_ports[8] = VSCP_DEFAULT_MULTICAST_PORT;
    m_ports[7] = 0;  // reserved
    m_ports[6] = 0;  // IP6, no port.
    m_ports[5] = 0;  // IP4, no port.
    m_ports[4] = 0;  // SSL (web/websocket/rest port)
    m_ports[3] = 0;  // Two connections, no port.
    m_ports[2] = 0;  // AES256, no port.
    m_ports[1] = 0;  // AES192, no port.
    m_ports[0] = 0;  // AES128, no port.
}

///////////////////////////////////////////////////////////////////////////////
// CVSCPNode DTOR
//

CVSCPNode::~CVSCPNode()
{
    ;
}


///////////////////////////////////////////////////////////////////////////////
// getCapabilitiesFromString
//

void CVSCPNode::getCapabilitiesFromString( const wxString& strCapabilities )
{
    uint64_t caps = 0;
    wxStringTokenizer tkz( strCapabilities, _(",") );
    
    for ( int i=7; i>0; i-- ) {
        if ( tkz.HasMoreTokens() ) {
            uint8_t val = vscp_readStringValue( tkz.GetNextToken() );
            caps |= ( val << ( i*8 ) ); 
        }
    }
    
    m_capabilities = caps;
}


///////////////////////////////////////////////////////////////////////////////
// getCapabilitiesFromString
//

void CVSCPNode::writeCapabilitiesToString( wxString& strCapabilities )
{
    strCapabilities.Printf( "%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x",
                                ( m_capabilities >> 56 ) & 0xff,
                                ( m_capabilities >> 48 ) & 0xff,
                                ( m_capabilities >> 40 ) & 0xff,
                                ( m_capabilities >> 32 ) & 0xff,
                                ( m_capabilities >> 24 ) & 0xff,
                                ( m_capabilities >> 16 ) & 0xff,
                                ( m_capabilities >> 8 ) & 0xff,
                                m_capabilities & 0xff );
}

///////////////////////////////////////////////////////////////////////////////
// CKnownNodes CTOR
//

CKnownNodes::CKnownNodes()
{
    
}

///////////////////////////////////////////////////////////////////////////////
// CKnownNodes DTOR
//

CKnownNodes::~CKnownNodes()
{
    // Remove hash content
    {
        VSCP_HASH_KNOWN_NODES::iterator it;
        for ( it = m_nodes.begin(); it != m_nodes.end(); ++it )
        {
            wxString key = it->first;
            CVSCPNode *pNode = it->second;
            if ( NULL != pNode ) delete pNode;
            pNode = NULL;
        }
    }
    
    // Clear the map
    m_nodes.clear();

}

///////////////////////////////////////////////////////////////////////////////
// findNode
//

CVSCPNode *CKnownNodes::findNode( cguid& guid )
{
    wxString strGUID;

    guid.toString( strGUID  );
    return m_nodes[ strGUID ];
}


///////////////////////////////////////////////////////////////////////////////
// addNode
//

CVSCPNode *CKnownNodes::addNode( cguid& guid )
{
    CVSCPNode *pNode = findNode( guid );
    if ( NULL == pNode ) {
        pNode = new CVSCPNode;
        if ( NULL != pNode ) {
            wxString strGUID;
            guid.toString( strGUID );
            m_nodes[ strGUID ] = pNode;    // Assign the node
        }
    }

    return pNode;
}


///////////////////////////////////////////////////////////////////////////////
// removeNode
//

bool CKnownNodes::removeNode( cguid& guid )
{
    wxString strGUID;
    bool rv = false;
    CVSCPNode *pNode = findNode( guid );
    if ( NULL != pNode ) delete pNode;
    pNode = NULL;
    
    guid.toString( strGUID );
    m_nodes[ strGUID ] = NULL;
    m_nodes.erase( strGUID );

    return rv;
}



///////////////////////////////////////////////////////////////////////////////
// saveNodes
//

void CKnownNodes::save( void )
{
    // TODO
}

///////////////////////////////////////////////////////////////////////////////
// loadNodes
//

bool CKnownNodes::load( void )
{
    const char *p;
    char *pErrMsg;
    sqlite3_stmt *ppStmt;
    
    gpobj->m_knownNodes.m_mutexKnownNodes.Lock();
    
    if ( SQLITE_OK != sqlite3_prepare_v2( gpobj->m_db_vscp_daemon,
                                            VSCPDB_GUID_SELECT_ALL,
                                            -1,
                                            &ppStmt,
                                            NULL ) ) {
        
        
    }
    
    int i;
    while ( SQLITE_ROW == sqlite3_step( ppStmt ) ) {
        
        // GUID ( Must be present
        if ( NULL == 
                 ( p = (const char *)sqlite3_column_text( ppStmt, 
                                        VSCPDB_ORDINAL_GUID_GUID ) ) ) {            
            continue;
        }
        
        cguid guid;
        guid.getFromString( p );
        CVSCPNode *pNode = addNode( guid );
        
        // id in database
        if ( NULL != 
                 ( p = (const char *)sqlite3_column_text( ppStmt, 
                                                    VSCPDB_ORDINAL_GUID_ID ) ) ) {
            pNode->m_dbId = atol( p );
        }
        
        // Nodename - Database given name for node
        if ( NULL != 
                 ( p = (const char *)sqlite3_column_text( ppStmt, 
                                               VSCPDB_ORDINAL_GUID_NAME ) ) ) {
            pNode->m_strNodeName = wxString::FromUTF8( p );
        }
        
        
        // Type
        if ( NULL != 
                 ( p = (const char *)sqlite3_column_text( ppStmt, 
                                        VSCPDB_ORDINAL_GUID_TYPE ) ) ) {            
            pNode->m_nodeType = (uint8_t)atoi(p);
        }
        
        // Last heartbeat is set to discovery date here
        if ( NULL != 
                 ( p = (const char *)sqlite3_column_text( ppStmt, 
                                        VSCPDB_ORDINAL_GUID_DATE ) ) ) {
            pNode->m_lastHeartBeat.ParseISOCombined( p );
        }        
                        
        // MDF link
        if ( NULL != 
                 ( p = (const char *)sqlite3_column_text( ppStmt, 
                                        VSCPDB_ORDINAL_GUID_LINK_TO_MDF ) ) ) {
           uint32_t lint_to_mdf = (uint32_t)atol( p );
        }
        
        // Address
        if ( NULL != 
                 ( p = (const char *)sqlite3_column_text( ppStmt, 
                                        VSCPDB_ORDINAL_GUID_ADDRESS ) ) ) {
            pNode->m_address = wxString::FromUTF8( p );
        }
        
        // Capabilities
        if ( NULL != 
                 ( p = (const char *)sqlite3_column_text( ppStmt, 
                                        VSCPDB_ORDINAL_GUID_CAPABILITIES ) ) ) {
            pNode->getCapabilitiesFromString( p );
        }
        
        // Non-standard ports
        if ( NULL != 
                 ( p = (const char *)sqlite3_column_text( ppStmt, 
                                        VSCPDB_ORDINAL_GUID_NONSTANDARD ) ) ) {
            wxString str = wxString::FromUTF8( p );
            //pNode-> ->m_capabilities = wxString::FromUTF8( p );
        }        
        
    }
    
    gpobj->m_knownNodes.m_mutexKnownNodes.Unlock();
    
    return true;
}


/*
 
 #include <wx/protocol/http.h>
#include <wx/XML/xml.h>
//[...]
	wxString link;
	wxHTTP http;
 
	http.SetTimeout(6);
	http.Connect(_T("example.com"));
        // PHP file sending XML content
	wxInputStream *httpStream = http.GetInputStream(_T("/file.php"));
 
	if (http.GetError() == wxPROTO_NOERR)
	{
                // will crash here, if xml content is not formatted PERFECTLY
		wxXmlDocument xml(*httpStream);
 
		wxXmlNode *node = xml.GetRoot()->GetChildren();
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
		wxMessageBox(_T("Can't connect!"));
 
	http.Close();
	wxDELETE(httpStream);
 
 */