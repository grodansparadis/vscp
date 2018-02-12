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
//                           Server information
///////////////////////////////////////////////////////////////////////////////


/*CVSCPServerInformation::CVSCPServerInformation()
{
    m_bUpdated = false;
    m_capabilities = 0;
    m_guid.clear();
    m_nameOfServer.Empty();
    m_ipaddress.Empty();

    // Set default ports
    memset( m_ports, 0, sizeof( m_ports ) );
  
    // Multicast announce port
    m_ports[ 16 ] = VSCP_ANNOUNCE_MULTICAST_PORT;

    // VSCP TCP/IP interface
    m_ports[ 15 ] = VSCP_DEFAULT_TCP_PORT;

    // UDP Interface
    m_ports[ 14 ] = VSCP_DEFAULT_UDP_PORT;

    // Multicast announce
    m_ports[ 13 ] = VSCP_ANNOUNCE_MULTICAST_PORT;

    // Raw Ethernet
    m_ports[ 12 ] = VSCP_DEFAULT_TCP_PORT;  // No port is used - This is the Ethernet frame id

    // Web server
    m_ports[ 11 ] = 8884;

    // Websocket interface
    m_ports[ 10 ] = 8884;

    // REST interface
    m_ports[ 9 ] = 8884;

    // MQTT interface
    m_ports[ 8 ] = 1883;

    // CoAP interface
    m_ports[ 7 ] = 40000;

}

CVSCPServerInformation::~CVSCPServerInformation()
{

}
*/

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
    m_mdfPath.Empty();
    m_lastHeartBeat = wxDateTime::Now();
    memset( m_stdreg, 0, 0x80 );
    m_strNodeName.Empty();
    m_deviceName.Empty();
    m_clientID = 0;
    m_level = 0;
}

///////////////////////////////////////////////////////////////////////////////
// CVSCPNode DTOR
//

CVSCPNode::~CVSCPNode()
{
    ;
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

void CKnownNodes::save( wxString& path )
{
    // TODO
}

///////////////////////////////////////////////////////////////////////////////
// loadNodes
//

bool CKnownNodes::load( void )
{
    char *pErrMsg;
    sqlite3_stmt *ppStmt;
    
    gpobj->m_knownNodes.m_mutexKnownNodes.Lock();
        
    if ( SQLITE_OK != sqlite3_prepare_v2( gpobj->m_db_vscp_daemon,
                                            VSCPDB_GUID_SELECT_ALL,
                                            -1,
                                            &ppStmt,
                                            NULL ) ) {
        
        // id
        const char *p;
        long id = 0;
        if ( NULL != 
                 ( p = (const char *)sqlite3_column_text( ppStmt, 
                                                    VSCPDB_ORDINAL_GUID_ID ) ) ) {
            id = atol( p );
        }
        
        return false;
    }
    
    int i;
    while ( SQLITE_ROW == sqlite3_step( ppStmt ) ) {
        
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