///////////////////////////////////////////////////////////////////////////////
// vscptcpif.cpp: 
//
// This file is part is part of VSCP, Very Simple Control Protocol
// http://www.vscp.org)
//
// Copyright (C) 2000-2013 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//



#ifdef __GNUG__
    //#pragma implementation
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#ifdef __WXMSW__
    #include  "wx/ownerdrw.h"
#endif

#ifdef WIN32
#include "winsock.h"
#endif

#include <wx/thread.h>
#include <wx/datetime.h>
#include <wx/socket.h>
#include <wx/tokenzr.h>
#include <wx/listimpl.cpp>
#include <wx/event.h>

#include "canal_macro.h"
#include "canal_win32_ipc.h"
#include "../common/canal.h"
#include "vscptcpif.h"

WX_DEFINE_LIST( EVENT_RX_QUEUE );
WX_DEFINE_LIST( EVENT_TX_QUEUE );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

VscpTcpIf::VscpTcpIf()
{	
    m_psock = NULL;	
    m_bModeReceiveLoop = false;
    m_responseTimeOut = DEFAULT_RESPONSE_TIMEOUT;
}


VscpTcpIf::~VscpTcpIf()
{	
    if ( NULL != m_psock ) delete m_psock;
}

///////////////////////////////////////////////////////////////////////////////
// checkReturnValue
//

bool VscpTcpIf::checkReturnValue( void )
{
    bool rv = false;
    bool bDone = false;
    wxString wxstr;
    char buf[ 512 ];
    wxDateTime start,now;
    
    start.SetToCurrent(); 

    m_strReply.Empty();
    
    do {

        wxTimeSpan tt = wxDateTime::Now() - start;

        memset( buf, 0, sizeof( buf ) - 1 );

        m_psock->Read( buf, sizeof( buf ) - 1 );
        if ( m_psock->Error() ) { 
            unsigned long errorcode = m_psock->LastError();
            wxLogDebug( _("CHECKRETURNVALUE: TCP/IP Communication error = %d"), errorcode );
            if ( errorcode != wxSOCKET_TIMEDOUT ) {
                return false; // return on all errors except timeout
            }
        }

        if ( m_psock->LastCount() > 0 ) {
            buf[ m_psock->LastCount() ] = 0;
            //wxLogDebug( _("Length of reply = %d"), m_psock->LastCount() );
            wxString str(buf, wxConvUTF8);
            m_strReply += str;
            //wxLogDebug( _("CHECKRETURNVALUE :") + str );
        }
    
        if ( wxNOT_FOUND != m_strReply.Find(_("+OK")) ) {
            //wxLogDebug( _("CHECKRETURNVALUE: Command suceeded!") );
            bDone = TRUE;
            rv = true;
        }
        else if ( wxNOT_FOUND != m_strReply.Find(_("-OK")) ) {
            //wxLogDebug( _("CHECKRETURNVALUE: Command failed!") );
            bDone = TRUE;
            rv = false;
        }

        // Check for timeout
        if ( tt.GetSeconds() > m_responseTimeOut ) {
            wxLogDebug( _("CHECKRETURNVALUE: TCP/IP Timeout") );
            return false;
        }

    }
    while ( !bDone );

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// doCommand
//

bool VscpTcpIf::doCommand( uint8_t cmd )
{	
    cmd = cmd;	// supress warning	
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdOpen
//

long VscpTcpIf::doCmdOpen( const wxString& strInterface, uint32_t flags )
{
    //int pos;
    bool bUsePassword = false;
    short port = VSCP_LEVEL2_TCP_PORT;

    wxString wxstr;
    wxString strUsername;
    wxString strPassword;
    wxString strHostname;
    
    flags=flags;	// supress warning
    
    wxLogDebug( _("strInterface = ") );
    wxLogDebug( strInterface ); 
    
    // Create working copy
    wxString strBuf = strInterface;
    
    wxStringTokenizer tkz( strInterface, _(";") );
    
    // Username
    strUsername = tkz.GetNextToken();
    strUsername.Trim(false);
    strUsername.Trim();
    if ( strUsername.length() ) {
        // If a username is given a password is required
        bUsePassword = true;
    }
    
    wxLogDebug( _("strUsername = ") );
    wxLogDebug( strUsername );
    
    // Password
    strPassword = tkz.GetNextToken();
    strPassword.Trim(false);
    strPassword.Trim();
    wxLogDebug( _("strPassword = ") );
    wxLogDebug( strPassword );
    
    // Hostname
    wxstr = tkz.GetNextToken();
    if ( wxstr.length() ) {
        strHostname = wxstr;
    }
    strHostname.Trim(false);
    strHostname.Trim();

    wxLogDebug( _("strHostname = ") );
    wxLogDebug( strHostname );
    
    // Port
    long val;
    wxstr = tkz.GetNextToken();
    if ( wxstr.length() ) {
        wxstr.ToLong( &val );
        port = (short)val;
    }	


    return doCmdOpen( strHostname, 
                            port, 
                            strUsername, 
                            strPassword );
}

///////////////////////////////////////////////////////////////////////////////
// doCmdOpen
//

long VscpTcpIf::doCmdOpen( const wxString& strHostname, 
                            const short port, 
                            const wxString& strUsername, 
                            const wxString& strPassword )
{
    wxString strBuf;
    wxString wxstr;

    // Create the socket 
    if ( NULL == ( m_psock = new wxSocketClient( wxSOCKET_BLOCK ) ) ) return CANAL_ERROR_GENERIC;

    // Here if we are connected 
    //m_psock->SetFlags( wxSOCKET_NOWAIT );
    m_psock->SetTimeout(2);

    wxstr = strHostname;
    wxstr.Trim(false);
    wxIPV4address addr;
    addr.Hostname( wxstr );
    addr.Service( port );

    // Connect to server
    m_psock->Connect( addr, false );
    
    wxLogDebug( _("Connect in progress with server ") + strHostname );
    
    if ( !m_psock->WaitOnConnect( 30 ) ) {	
        return 0; // Zero for error in open
    }
    
  
    wxLogDebug( _("Checking server respons") );

    if ( !checkReturnValue() ) {
        wxLogDebug( _("No response from ") + strHostname );
        wxLogDebug( _("------------------------------------------------------") );
        wxLogDebug( _("No response from ") + strHostname );
        wxLogDebug( _("------------------------------------------------------") );
        return 0; // Zero for error in open
     }
    
    wxLogDebug( _("Successfully connected to ") + strHostname );

    // Username
    wxstr = strUsername;
    wxstr.Trim(false);
    strBuf =  _("USER ") + wxstr + _("\r\n"); 
    m_psock->Write( strBuf.mb_str(), strBuf.length() );  
    if ( !checkReturnValue() ) return 0;  // Zero for error in open

    // Password
    wxstr = strPassword;
    wxstr.Trim(false);
    strBuf =  _("PASS ") + wxstr + _("\r\n");
    m_psock->Write( strBuf.mb_str(), strBuf.length() );
    if ( !checkReturnValue() ) return CANAL_ERROR_USER;
    
    wxLogDebug( _("Successful log in to VSCP server") );
  
    return 1922;  // Driver handle == any positiv value -- 1922 is a good value ;-)
}




///////////////////////////////////////////////////////////////////////////////
// close
//

int VscpTcpIf::doCmdClose( void )
{	
    int rv;

    if ( NULL == m_psock ) return CANAL_ERROR_GENERIC;	// Must have a valid socket
    if ( !m_psock->IsOk() ) return CANAL_ERROR_GENERIC;	// Must be connected
    
    wxString strCmd(_("QUIT\r\n"));
    m_psock->Write( strCmd.mb_str(), strCmd.length() );

    if ( checkReturnValue() ) {
        rv = CANAL_ERROR_SUCCESS;
    }
    else {
        rv = CANAL_ERROR_GENERIC;
    }

    m_psock->Close();
    if ( NULL != m_psock ) delete m_psock;
    m_psock = NULL;

    return rv;  
}



///////////////////////////////////////////////////////////////////////////////
// doCmdNOOP
//

int VscpTcpIf::doCmdNOOP( void )
{	
    if ( NULL == m_psock ) return CANAL_ERROR_PARAMETER;	// Must have a valid socket
    if ( !m_psock->IsOk() ) return CANAL_ERROR_PARAMETER;	// Must be connected
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return CANAL_ERROR_PARAMETER;
    
    wxString strCmd(_("NOOP\r\n"));
    m_psock->Write( strCmd.mb_str(), strCmd.length() );

    if ( checkReturnValue() ) {
        return CANAL_ERROR_SUCCESS;
    }
    else {
        return CANAL_ERROR_GENERIC;
    }
}

///////////////////////////////////////////////////////////////////////////////
// doCmdClear
//

int VscpTcpIf::doCmdClear( void )
{	
    if ( NULL == m_psock ) return CANAL_ERROR_PARAMETER;	// Must have a valid socket
    if ( !m_psock->IsOk() ) return CANAL_ERROR_PARAMETER;	// Must be connected
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return CANAL_ERROR_PARAMETER;
    
    wxString strCmd(_("CLRA\r\n"));
    m_psock->Write( strCmd.mb_str(), strCmd.length() );

    if ( checkReturnValue() ) {
        return CANAL_ERROR_SUCCESS;
    }
    else {
        return CANAL_ERROR_GENERIC;
    }
}


///////////////////////////////////////////////////////////////////////////////
// doCmdSend
//


int VscpTcpIf::doCmdSend( const vscpEvent *pEvent )
{	
    uint16_t i;
    
    if ( NULL == m_psock ) return CANAL_ERROR_PARAMETER;	  // Must have a valid socket
    if ( !m_psock->IsOk() ) return CANAL_ERROR_PARAMETER;	  // Must be connected
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return CANAL_ERROR_PARAMETER;
    
    wxString strBuf, strWrk, strGUID;
    unsigned char guidsum = 0;
    
    // Must be a valid data pointer if data 
    if ( ( pEvent->sizeData > 0 ) && ( NULL == pEvent->pdata ) ) return CANAL_ERROR_GENERIC;

    //send head,class,type,obid,timestamp,GUID,data1,data2,data3....
    strBuf.Printf( _("SEND %d,%d,%d,%lu,%lu,"),
                        pEvent->head,
                        pEvent->vscp_class,
                        pEvent->vscp_type,
                        pEvent->obid,
                        pEvent->timestamp );

    // GUID
    for ( i=0; i<16; i++ ) {
    
        guidsum += pEvent->GUID[ i ];
        strWrk.Printf( _("%d"), pEvent->GUID[ i ] );	
        if ( i != 15 ) {
            strWrk += _(":");
        }
        
        strGUID += strWrk;
    }

    if ( 0 == guidsum ) {
        strBuf += _("-");
    }
    else {
        strBuf += strGUID;
    }

    strBuf += _(",");

    // Data
    for ( i=0; i<pEvent->sizeData; i++ ) {
        strWrk.Printf( _("%d"), pEvent->pdata[ i ] );
        strBuf += strWrk;
        if ( i != ( pEvent->sizeData - 1 ) ) {
            strBuf += _(",");
        }
    }

    strBuf += _("\r\n");

    m_psock->Write( strBuf.mb_str(), strBuf.length() );
  
    if ( checkReturnValue() ) {
        return CANAL_ERROR_SUCCESS;
    }
    else {
        return CANAL_ERROR_GENERIC;
    }
  
}



///////////////////////////////////////////////////////////////////////////////
// doCmdSendEx
//

int VscpTcpIf::doCmdSendEx( const vscpEventEx *pEvent )
{	
    uint16_t i;
    wxString strBuf, strWrk, strGUID;
    unsigned char guidsum = 0;
    
    if ( NULL == m_psock ) return CANAL_ERROR_PARAMETER;	  // Must have a valid socket
    if ( !m_psock->IsOk() ) return CANAL_ERROR_PARAMETER;	  // Must be connected
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return CANAL_ERROR_PARAMETER;
    
    //send head,class,type,obid,timestamp,GUID,data1,data2,data3....
    strBuf.Printf( _("SEND %d,%d,%d,%lu,%lu,"),
                    pEvent->head,
                              pEvent->vscp_class,
                              pEvent->vscp_type,
                              pEvent->obid,
                              pEvent->timestamp );

    // GUID
    for ( i=0; i<16; i++ ) {
    
        guidsum += pEvent->GUID[ i ];
        strWrk.Printf(_("%02X"), pEvent->GUID[ i ] );
        
        if ( i != 15 ) {
            strWrk += _(":");
        }
        strGUID += strWrk;
    }

    if ( 0 == guidsum ) {
        strBuf += _("-");
    }
    else {
        strBuf += strGUID;
    }

    strBuf += _(",");

    // Data
    for ( i=0; i<pEvent->sizeData; i++ ) {
        strWrk.Printf( _("%d"), pEvent->data[ i ] );
        strBuf += strWrk;
        if ( i != ( pEvent->sizeData - 1 ) ) {
            strBuf += _(",");
        }
    }

    strBuf += _("\r\n");

    m_psock->Write( strBuf.mb_str(), strBuf.length() );
  
    if ( checkReturnValue() ) {
        return CANAL_ERROR_SUCCESS;
    }
    else {
        return CANAL_ERROR_GENERIC;
    }
  
}




///////////////////////////////////////////////////////////////////////////////
// doCmdSendLevel1
//

int VscpTcpIf::doCmdSendLevel1( const canalMsg *pCanalMsg )
{
    vscpEventEx event;
    
    if ( NULL == m_psock ) return CANAL_ERROR_PARAMETER;	  // Must have a valid socket
    if ( !m_psock->IsOk() ) return CANAL_ERROR_PARAMETER;	  // Must be connected
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return CANAL_ERROR_PARAMETER;
    
    event.vscp_class = (unsigned short)( 0x1ff & ( pCanalMsg->id >> 16 ) );
    event.vscp_type = (unsigned char)( 0xff & ( pCanalMsg->id >> 8 ) ); 
    event.obid = pCanalMsg->obid;
    event.timestamp = pCanalMsg->timestamp;
    event.head = 0x00;

    memset( event.GUID, 0, 16); 

    event.GUID[ 0 ] = pCanalMsg->id & 0xff;

    event.sizeData = pCanalMsg->sizeData;
    memcpy( event.data, pCanalMsg->data, pCanalMsg->sizeData );

    return doCmdSendEx( &event );

}

///////////////////////////////////////////////////////////////////////////////
// getEventFromLine
//

bool VscpTcpIf::getEventFromLine( const wxString& strLine, vscpEvent *pEvent )
{
    wxStringTokenizer strTokens;
    wxString strWrk;
    wxString strGUID;
    long val;
  
    // Check pointer
    if ( NULL == pEvent ) return false;
  
    // Tokinize the string
    strTokens.SetString( strLine, _(",\r\n") );

    // Get head
    pEvent->head = 0;
    if ( strTokens.HasMoreTokens() ) {
        strWrk = strTokens.GetNextToken();   
        
        strWrk.ToLong( &val );
        pEvent->head = (uint8_t)val; 
        
    }
    
    // Get Class
    pEvent->vscp_class = 0;
    if ( strTokens.HasMoreTokens() ) {
        
        strWrk = strTokens.GetNextToken();  
        
        strWrk.ToLong( &val );
        pEvent->vscp_class = (uint16_t)val; 
        
    }
    

    // Get Type
    pEvent->vscp_type = 0;
    if ( strTokens.HasMoreTokens() ) {
        
        strWrk = strTokens.GetNextToken();   
        
        strWrk.ToLong( &val );
        pEvent->vscp_type = (uint16_t)val; 
        
    }



    // Get OBID
    pEvent->obid = 0;
    if ( strTokens.HasMoreTokens() ) {
        
        strWrk = strTokens.GetNextToken();
        
        strWrk.ToLong( &val );
        pEvent->obid = (uint16_t)val; 
        
    }
    
    
    // Get Timestamp
    pEvent->timestamp = 0;
    if ( strTokens.HasMoreTokens() ) {
        
        strWrk = strTokens.GetNextToken();
        
        strWrk.ToLong( &val );
        pEvent->timestamp = (uint16_t)val; 
        
    }
    

    // Get GUID
    if ( strTokens.HasMoreTokens() ) {
        strGUID = strTokens.GetNextToken();
    }
    
    // Must have a GUID
    if ( 0 == strGUID.length() ) return false;
    
                
    // Handle data
    pEvent->sizeData = 0;
    char data[ 512 ];

    while ( strTokens.HasMoreTokens() && ( pEvent->sizeData < 512 ) ) {

        strWrk = strTokens.GetNextToken();
        data[ pEvent->sizeData ] = readStringValue( strWrk );
        pEvent->sizeData++;

    }

    // Continue to handle GUID
    getGuidFromString( pEvent, strGUID );
  

    // Copy in the data
    pEvent->pdata = new unsigned char[ pEvent->sizeData ];
    if ( NULL != pEvent->pdata ) {
        memcpy( pEvent->pdata, data, pEvent->sizeData );
    }
  
    return true;
  
}


///////////////////////////////////////////////////////////////////////////////
// doCmdReceive
//

int VscpTcpIf::doCmdReceive( vscpEvent *pEvent )
{	
    int pos;
    wxStringTokenizer strTokens;
    wxString strLine;
    wxString strWrk;
    wxString strGUID;
    
    if ( NULL == m_psock ) return CANAL_ERROR_PARAMETER;	  // Must have a valid socket
    if ( !m_psock->IsOk() ) return CANAL_ERROR_PARAMETER;	  // Must be connected
    
    if ( NULL == pEvent ) return CANAL_ERROR_GENERIC;
  
     // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return CANAL_ERROR_PARAMETER;
        
    wxString strCmd(_("RETR\r\n"));
    m_psock->Write( strCmd.mb_str(), strCmd.length() );
    if ( !checkReturnValue() ) return CANAL_ERROR_GENERIC;

    // Handle the data (if any)
    pos = m_strReply.Find(_("\r"));
    if ( !pos ) {
        return CANAL_ERROR_GENERIC; // No reply data
    }
    
    // Save the line
    strLine = m_strReply.Left( pos );
    
    // Take away the line we work on
    m_strReply = m_strReply.Right( m_strReply.length() - pos - 1 );
  
     if ( !getEventFromLine( strLine, pEvent ) ) return CANAL_ERROR_PARAMETER;
    
    return CANAL_ERROR_SUCCESS;

}


///////////////////////////////////////////////////////////////////////////////
// doCmdReceiveEx
//

int VscpTcpIf::doCmdReceiveEx( vscpEventEx *pEventEx )
{	
    int pos;
    wxStringTokenizer strTokens;
    wxString strLine;
    wxString strBuf;
    wxString strWrk;
    wxString strGUID;
    
    if ( NULL == m_psock ) return CANAL_ERROR_PARAMETER;	  // Must have a valid socket
    if ( !m_psock->IsOk() ) return CANAL_ERROR_PARAMETER;	  // Must be connected
    if ( NULL == pEventEx ) return CANAL_ERROR_PARAMETER;
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return CANAL_ERROR_PARAMETER;
    
    wxString strCmd(_("RETR\r\n"));
    m_psock->Write( strCmd.mb_str(), strCmd.length() );
    if ( !checkReturnValue() ) return CANAL_ERROR_GENERIC;

    // Handle the data (if any)
    pos = m_strReply.Find(_("\r") );
    if ( !pos ) {
        return CANAL_ERROR_GENERIC; // No reply data
    }
    
    // Save the line
    strLine = m_strReply.Left( pos );
  
    vscpEvent *pEvent = new vscpEvent;
    if ( NULL == pEvent) return CANAL_ERROR_PARAMETER;
  
    if ( !getEventFromLine( strLine, pEvent ) ) return CANAL_ERROR_PARAMETER;
  
    if ( !convertVSCPtoEx( pEventEx, pEvent ) ) {
        deleteVSCPevent( pEvent );
        return CANAL_ERROR_PARAMETER;
    }
    
    deleteVSCPevent( pEvent );
  
    return CANAL_ERROR_SUCCESS;

}



///////////////////////////////////////////////////////////////////////////////
// doCmdReceiveLevel1
//

int VscpTcpIf::doCmdReceiveLevel1( canalMsg *pCanalMsg )
{
    vscpEventEx event;

    if ( NULL == m_psock ) return CANAL_ERROR_PARAMETER;	// Must have a valid socket
    if ( !m_psock->IsOk() ) return CANAL_ERROR_PARAMETER;	// Must be connected
    
    // Must have a valid pointer
    if ( NULL == pCanalMsg ) return CANAL_ERROR_PARAMETER;
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return CANAL_ERROR_PARAMETER;
    
    // Fetch event
    if ( CANAL_ERROR_SUCCESS != doCmdReceiveEx( &event ) ) return CANAL_ERROR_GENERIC;

    // No CAN or Level I event if data is > 8
    if ( event.sizeData > 8 ) return CANAL_ERROR_GENERIC;


    pCanalMsg->id = (unsigned long)( ( event.head >> 5 ) << 20 ) |
                             ( (unsigned long)event.vscp_class << 16 ) |
                             ( (unsigned long)event.vscp_type << 8) |
                             event.GUID[ 15 ];	
               
    pCanalMsg->obid = event.obid;
    pCanalMsg->sizeData = event.sizeData;
    if ( pCanalMsg->sizeData ) {
        memcpy( pCanalMsg->data, event.data, event.sizeData ); 
    }

    pCanalMsg->timestamp = event.timestamp;

    return CANAL_ERROR_SUCCESS;

}


///////////////////////////////////////////////////////////////////////////////
// doCmdEnterReceiveLoop
//

int VscpTcpIf::doCmdEnterReceiveLoop( void )
{
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return CANAL_ERROR_PARAMETER;
    
    wxString strCmd(_("RCVLOOP\r\n"));
    m_psock->SetTimeout( 2 );
    m_psock->Write( strCmd.mb_str(), strCmd.length() );
    if ( !checkReturnValue() ) return CANAL_ERROR_GENERIC;
  
    m_strReply = _("");
    m_bModeReceiveLoop = true;
    return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdBlockReceive
//

int VscpTcpIf::doCmdBlockReceive( vscpEvent *pEvent, uint32_t timeout )
{
    char buf[512];
    static wxString strBuf = _("");
    wxString strReply;
    
    timeout = timeout;

    // If not receive loop active terminate
    if ( !m_bModeReceiveLoop ) return CANAL_ERROR_PARAMETER;
    
    // TODO This eats 100% CPU time on Windows. Why!?
    //if ( !m_psock->WaitForRead( 0, timeout ) ) {
    //  return CANAL_ERROR_TIMEOUT;
    //}

    m_psock->Read( buf, sizeof( buf ) - 1 );
  
    if ( m_psock->Error() ) {
        if ( wxSOCKET_TIMEDOUT == m_psock->LastError() ) {
            return CANAL_ERROR_TIMEOUT;
        }
        else if ( wxSOCKET_INVOP == m_psock->LastError() ) {
            return CANAL_ERROR_COMMUNICATION;
        }   
        else if ( wxSOCKET_IOERR == m_psock->LastError() ) {
            return CANAL_ERROR_COMMUNICATION;
        }
        else if ( wxSOCKET_INVADDR == m_psock->LastError() ) {
            return CANAL_ERROR_COMMUNICATION;
        }
        else if ( wxSOCKET_INVSOCK == m_psock->LastError() ) {
            return CANAL_ERROR_COMMUNICATION;
        }
        else if ( wxSOCKET_NOHOST == m_psock->LastError() ) {
            return CANAL_ERROR_COMMUNICATION;
        }
        else if ( wxSOCKET_INVPORT == m_psock->LastError() ) {
            return CANAL_ERROR_COMMUNICATION;
        }
        else if ( wxSOCKET_WOULDBLOCK == m_psock->LastError() ) {
            return CANAL_ERROR_FIFO_EMPTY;
        }
        else if ( wxSOCKET_MEMERR == m_psock->LastError() ) {
            return CANAL_ERROR_COMMUNICATION;
        }
        else {
            return CANAL_ERROR_COMMUNICATION;
        }

    }
  
    if ( m_psock->LastCount() > 0 ) {
        buf[ m_psock->LastCount() ] = 0;
        //wxLogDebug( _("Length of reply = %d"), m_psock->LastCount() );
        strBuf += wxString( buf, wxConvUTF8 );
        //wxLogDebug( strBuf );
    }

    int pos;
    int rv = CANAL_ERROR_SUCCESS;
    while ( ( pos = strBuf.Find( (wxChar) 0x0A ) ) ) {
      
        if ( wxNOT_FOUND == pos ) return CANAL_ERROR_FIFO_EMPTY;
        if ( 0 == pos ) {
            //wxLogDebug( _("pos == NULL") );
            strBuf = strBuf.Right( strBuf.Length() - 1 );
            //wxLogDebug( strBuf );
            rv = CANAL_ERROR_PARAMETER;
            continue;
        }

        wxString strLine;
        wxString bufcpy = strBuf;
        strLine = strBuf.Mid( 0, pos );
        if ( strBuf.Length() > (unsigned int)( pos + 1 ) ) {
            strBuf = strBuf.Mid( pos + 1 );
        }
        else {
            strBuf = _("");
        }
    
        // wxLogDebug( _("[") + strLine + _("]") );

        if ( wxNOT_FOUND != strLine.Find(_("+OK")) ) {
            rv = CANAL_ERROR_FIFO_EMPTY; // No data
            continue;
        }

        // wxMessageBox( _("Line=") + strLine );
        // wxMessageBox( _("Buf=") + strBuf );
 
        // Get the event
        if ( !getEventFromLine( strLine, pEvent ) ) {
            rv = CANAL_ERROR_PARAMETER;
            continue;
        }

        return CANAL_ERROR_SUCCESS;

    } 

    return rv;
  
}


///////////////////////////////////////////////////////////////////////////////
// doCmdDataAvailable
//

int VscpTcpIf::doCmdDataAvailable( void )
{
    wxString strLine;
    int pos;
    int nMsg = 0;	
  
    
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return 0;

    wxString strCmd(_("CDTA\r\n"));
    m_psock->Write( strCmd.mb_str(), strCmd.length() );
    if ( !checkReturnValue() ) return CANAL_ERROR_GENERIC;
    
    // Handle the data (if any)
    pos = m_strReply.Find(_("\r") );
    if ( !pos ) {
        return 0; // No reply data
    }
    
    // Save the line
    strLine = m_strReply.Left( pos );

    long val;
    if ( strLine.ToLong(&val) ) {
        nMsg = (uint16_t)val;
    }
    
    return nMsg;
    
}


///////////////////////////////////////////////////////////////////////////////
// doCmdState
//

int VscpTcpIf::doCmdStatus( canalStatus *pStatus )
{	
    int pos;
    long val;
    wxString strBuf;
    wxString strWrk;
    wxString strLine;
    wxStringTokenizer strTokens;

    if ( NULL == m_psock ) return CANAL_ERROR_PARAMETER;	  // Must have a valid socket
    if ( !m_psock->IsOk() ) return CANAL_ERROR_PARAMETER;	  // Must be connected
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return CANAL_ERROR_PARAMETER;
    
    wxString strCmd(_("INFO\r\n"));
    m_psock->Write( strCmd.mb_str(), strCmd.length() );
    if ( !checkReturnValue() ) return CANAL_ERROR_GENERIC;

    // Handle the data (if any)
    pos = m_strReply.Find(_("\r") );
    if ( !pos ) {
        return CANAL_ERROR_GENERIC; // No reply data
    }
    
    // Save the line
    strLine = m_strReply.Left( pos );

    // channelstatus
    strTokens.SetString( strLine, _(",\r\n") );
    
    // lasterrorcode
    if ( !strTokens.HasMoreTokens() ) return CANAL_ERROR_GENERIC;
    ( strTokens.GetNextToken() ).ToLong( &val );
    pStatus->lasterrorcode = val;
    
    
    // lasterrorsubcode
    if ( !strTokens.HasMoreTokens() ) return CANAL_ERROR_GENERIC;
    ( strTokens.GetNextToken() ).ToLong( &val );
    pStatus->lasterrorsubcode = val;
    

    // lasterrorsubcode
    if ( !strTokens.HasMoreTokens() ) return CANAL_ERROR_GENERIC;
    strWrk = strTokens.GetNextToken();
    memcpy( pStatus->lasterrorstr, strWrk, sizeof(pStatus->lasterrorstr) );
    
    return CANAL_ERROR_SUCCESS;

}


///////////////////////////////////////////////////////////////////////////////
// doCmdStatistics
//

int VscpTcpIf::doCmdStatistics( canalStatistics *pStatistics )
{	
    int pos;
    long val;
    wxString strBuf;
    wxString strWrk;
    wxString strLine;
    wxStringTokenizer strTokens;

    if ( NULL == m_psock ) return CANAL_ERROR_PARAMETER;	  // Must have a valid socket
    if ( !m_psock->IsOk() ) return CANAL_ERROR_PARAMETER;	// Must be connected
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return CANAL_ERROR_PARAMETER;
        
    wxString strCmd(_("STAT\r\n"));
    m_psock->Write( strCmd.mb_str(), strCmd.length() );
    if ( !checkReturnValue() ) return CANAL_ERROR_GENERIC;

    // Handle the data (if any)
    pos = m_strReply.Find(_("\r") );
    if ( !pos ) {
        return CANAL_ERROR_GENERIC; // No reply data
    }
    
    // Save the line
    strLine = m_strReply.Left( pos );
    
    strTokens.SetString( strLine, _(",\r\n"));

    // Busoff
    pStatistics->cntBusOff = 0;
    if ( ( strTokens.GetNextToken() ).ToLong( &val ) ) {
        pStatistics->cntBusOff = val;
    }
    else {
        return CANAL_ERROR_GENERIC;
    }
    

    // Buswarnings
    pStatistics->cntBusWarnings = 0;
    if ( ( strTokens.GetNextToken() ).ToLong( &val ) ) {
        pStatistics->cntBusWarnings = val;
    }
    else {
        return CANAL_ERROR_GENERIC;
    }
    
    

    // Overruns
    pStatistics->cntOverruns = 0;
    if ( ( strTokens.GetNextToken() ).ToLong( &val ) ) {
        pStatistics->cntOverruns = val;
    }
    else {
        return CANAL_ERROR_GENERIC;
    }
    

    // Received data
    pStatistics->cntReceiveData = 0;
    if ( ( strTokens.GetNextToken() ).ToLong( &val ) ) {
        pStatistics->cntReceiveData = val;
    }
    else {
        return CANAL_ERROR_GENERIC;
    }
    
    

    // Received Frames
    pStatistics->cntReceiveFrames = 0;
    if ( ( strTokens.GetNextToken() ).ToLong( &val ) ) {
        pStatistics->cntReceiveFrames = val;
    }
    else {
        return CANAL_ERROR_GENERIC;
    }
    
    
    
    // Transmitted data
    pStatistics->cntTransmitData = 0;
    if ( ( strTokens.GetNextToken() ).ToLong( &val ) ) {
        pStatistics->cntReceiveFrames = val;
    }
    else {
        return CANAL_ERROR_GENERIC;
    }
    

    // Transmitted frames
    pStatistics->cntTransmitFrames = 0;
    if ( ( strTokens.GetNextToken() ).ToLong( &val ) ) {
        pStatistics->cntTransmitFrames = val;
    }
    else {
        return CANAL_ERROR_GENERIC;
    }
    
    return CANAL_ERROR_SUCCESS;

}


///////////////////////////////////////////////////////////////////////////////
// doCmdFilter
//

int VscpTcpIf::doCmdFilter( const vscpEventFilter *pFilter )
{	
    wxString strCmd;
    
    if ( NULL == m_psock ) return CANAL_ERROR_PARAMETER;		// Must have a valid socket
    if ( !m_psock->IsOk() ) return CANAL_ERROR_PARAMETER;		// Must be connected
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return CANAL_ERROR_PARAMETER;
    
    // filter-priority, filter-class, filter-type, filter-GUID
    strCmd.Printf( _("SFLT %d,%d,%d,%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n"),
                    pFilter->filter_priority,
                    pFilter->filter_class,
                    pFilter->filter_type,
                    pFilter->filter_GUID[ 15 ],
                    pFilter->filter_GUID[ 14 ],
                    pFilter->filter_GUID[ 13 ],
                    pFilter->filter_GUID[ 12 ],
                    pFilter->filter_GUID[ 11 ],
                    pFilter->filter_GUID[ 10 ],
                    pFilter->filter_GUID[ 9 ],
                    pFilter->filter_GUID[ 8 ],
                    pFilter->filter_GUID[ 7 ],
                    pFilter->filter_GUID[ 6 ],
                    pFilter->filter_GUID[ 5 ],
                    pFilter->filter_GUID[ 4 ],
                    pFilter->filter_GUID[ 3 ],
                    pFilter->filter_GUID[ 2 ],
                    pFilter->filter_GUID[ 1 ],
                    pFilter->filter_GUID[ 0 ] );

    m_psock->Write( strCmd.mb_str(), strCmd.length() );
    if ( !checkReturnValue() ) return CANAL_ERROR_GENERIC;


    // mask-priority, mask-class, mask-type, mask-GUID
    strCmd.Printf( _("SMSK %d,%d,%d,%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n"),
                    pFilter->mask_priority,
                    pFilter->mask_class,
                    pFilter->mask_type,
                    pFilter->mask_GUID[ 15 ],
                    pFilter->mask_GUID[ 14 ],
                    pFilter->mask_GUID[ 13 ],
                    pFilter->mask_GUID[ 12 ],
                    pFilter->mask_GUID[ 11 ],
                    pFilter->mask_GUID[ 10 ],
                    pFilter->mask_GUID[ 9 ],
                    pFilter->mask_GUID[ 8 ],
                    pFilter->mask_GUID[ 7 ],
                    pFilter->mask_GUID[ 6 ],
                    pFilter->mask_GUID[ 5 ],
                    pFilter->mask_GUID[ 4 ],
                    pFilter->mask_GUID[ 3 ],
                    pFilter->mask_GUID[ 2 ],
                    pFilter->mask_GUID[ 1 ],
                    pFilter->mask_GUID[ 0 ] );
    
    m_psock->Write( strCmd.mb_str(), strCmd.length() );
    if ( !checkReturnValue() ) return CANAL_ERROR_GENERIC;
    
    return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdFilter
//

int VscpTcpIf::doCmdFilter( const wxString& filter, const wxString& mask )
{	
    wxString strCmd;

    if ( NULL == m_psock ) return CANAL_ERROR_PARAMETER;		// Must have a valid socket
    if ( !m_psock->IsOk() ) return CANAL_ERROR_PARAMETER;		// Must be connected
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return CANAL_ERROR_PARAMETER;

    // Set filter
    strCmd = _("SFLT ") + filter + _("\r\n");
    m_psock->Write( strCmd.mb_str(), strCmd.length() );
    if ( !checkReturnValue() ) return CANAL_ERROR_GENERIC;

    // Set mask
    strCmd = _("SMSK ") + mask + _("\r\n");
    m_psock->Write( strCmd.mb_str(), strCmd.length() );
    if ( !checkReturnValue() ) return CANAL_ERROR_GENERIC;

    return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdVersion
//

unsigned long VscpTcpIf::doCmdVersion( void )
{
    long val;
    int pos;
    unsigned char version[3];
    wxString strLine;
    wxStringTokenizer strTokens;

    if ( NULL == m_psock ) return 0;	// Must have a valid socket
    if ( !m_psock->IsOk() ) return 0;	// Must be connected
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return CANAL_ERROR_PARAMETER;
        
    wxString strCmd(_("VERS\r\n"));
    m_psock->Write( strCmd.mb_str(), strCmd.length() );
    if ( !checkReturnValue() ) return 0;

    // Handle the data (if any)
    pos = m_strReply.Find(_("\r") );
    if ( !pos ) {
        return 0; // No reply data
    }
    
    // Save the line
    strLine = m_strReply.Left( pos );
    
    strTokens.SetString( strLine, _(",\r\n"));

    // Major version
    version[ 0 ] = 0;
    if ( ( strTokens.GetNextToken() ).ToLong( &val ) ) {
        version[ 0 ] = (uint8_t)val;
    }
    else {
        return 0;
    }

    // Minor version
    version[ 1 ] = 0;
    if ( ( strTokens.GetNextToken() ).ToLong( &val ) ) {
        version[ 1 ] = (uint8_t)val;
    }
    else {
        return 0;
    }

    // Sub minor version
    version[ 2 ] = 0;
    if ( ( strTokens.GetNextToken() ).ToLong( &val ) ) {
        version[ 2 ] = (uint8_t)val;
    }
    else {
        return 0;
    }

    return ( version[0] << 16 ) + ( version[1] << 8 ) + version[0];

}


///////////////////////////////////////////////////////////////////////////////
// doCmdDLLVersion
//

unsigned long VscpTcpIf::doCmdDLLVersion( void  )
{
    return TCPIP_DLL_VERSION;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdVendorString
//

const char * VscpTcpIf::doCmdVendorString( void )
{
  return wxString( TCPIP_VENDOR_STRING ).mb_str();
}

///////////////////////////////////////////////////////////////////////////////
// doCmdGetDriverInfo
//

const char * VscpTcpIf::doCmdGetDriverInfo( void )
{
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdGetGUID
//

int VscpTcpIf::doCmdGetGUID( char *pGUID )
{
    long val;
    int pos;
    wxString strLine;
    wxStringTokenizer strTokens;

    if ( NULL == m_psock ) return CANAL_ERROR_PARAMETER;	// Must have a valid socket
    if ( !m_psock->IsOk() ) return CANAL_ERROR_PARAMETER;	// Must be connected
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return CANAL_ERROR_PARAMETER;
        
    wxString strCmd(_("SGID\r\n"));
    m_psock->Write( strCmd.mb_str(), strCmd.length() );
    if ( !checkReturnValue() ) return CANAL_ERROR_GENERIC;

    // Handle the data (if any)
    pos = m_strReply.Find(_("\r") );
    if ( !pos ) {
        return CANAL_ERROR_GENERIC; // No reply data
    }
    
    // Save the line
    strLine = m_strReply.Left( pos );
    
    strTokens.SetString( strLine, _(",\r\n"));
    
    if ( strTokens.HasMoreTokens() ) {
         
        int idx = 0; 
        wxStringTokenizer wrkToken( strTokens.GetNextToken(), _(":") );
        while ( wrkToken.HasMoreTokens() && ( idx < 16 ) ) {
            
            (wrkToken.GetNextToken()).ToLong( &val );
            pGUID[ idx ] = (uint8_t)val;
            idx++;
            
        }
        
        if ( idx != 16 ) return CANAL_ERROR_GENERIC;
        
    }
    else {
        return CANAL_ERROR_GENERIC;
    }
    
    return CANAL_ERROR_SUCCESS;

}


///////////////////////////////////////////////////////////////////////////////
// doCmdGetGUID
//

int VscpTcpIf::doCmdGetGUID( cguid& ifguid )
{
    long val;
    int pos;
    wxString strLine;
    wxStringTokenizer strTokens;

    if ( NULL == m_psock ) return CANAL_ERROR_PARAMETER;	// Must have a valid socket
    if ( !m_psock->IsOk() ) return CANAL_ERROR_PARAMETER;	// Must be connected
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return CANAL_ERROR_PARAMETER;
        
    wxString strCmd(_("SGID\r\n"));
    m_psock->Write( strCmd.mb_str(), strCmd.length() );
    if ( !checkReturnValue() ) return CANAL_ERROR_GENERIC;

    // Handle the data (if any)
    pos = m_strReply.Find(_("\r") );
    if ( !pos ) {
        return CANAL_ERROR_GENERIC; // No reply data
    }
    
    // Save the line
    strLine = m_strReply.Left( pos );
    
	ifguid.getFromString(strLine);
    
    return CANAL_ERROR_SUCCESS;

}

///////////////////////////////////////////////////////////////////////////////
// doCmdSetGUID
//

int VscpTcpIf::doCmdSetGUID( const char *pGUID )
{
    int pos;
    wxString strLine;
    wxString strCmd;

    if ( NULL == m_psock ) return CANAL_ERROR_PARAMETER;	// Must have a valid socket
    if ( !m_psock->IsOk() ) return CANAL_ERROR_PARAMETER;	// Must be connected

    if ( NULL == pGUID ) return CANAL_ERROR_GENERIC;
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return CANAL_ERROR_PARAMETER;
        
    // Handle the data (if any)
    
    pos = m_strReply.Find(_("\r"));
    if ( !pos ) {
        return CANAL_ERROR_GENERIC;  
    }
    
    // Save the line
    strLine = m_strReply.Left( pos );
    
    strCmd.Printf( _("SGID %d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d\r\n"), 
                    pGUID[ 0 ],
                    pGUID[ 1 ],
                    pGUID[ 2 ],
                    pGUID[ 3 ],
                    pGUID[ 4 ],
                    pGUID[ 5 ],
                    pGUID[ 6 ],
                    pGUID[ 7 ],
                    pGUID[ 8 ],
                    pGUID[ 9 ],
                    pGUID[ 10 ],
                    pGUID[ 11 ],
                    pGUID[ 12 ],
                    pGUID[ 13 ],
                    pGUID[ 14 ],
                    pGUID[ 15 ]
                );	
    
    
    m_psock->Write( strCmd.mb_str(), strCmd.length() );
    return checkReturnValue();
}


///////////////////////////////////////////////////////////////////////////////
// doCmdGetChannelInfo
//


int VscpTcpIf::doCmdGetChannelInfo( VSCPChannelInfo *pChannelInfo )
{
    int rv;
    wxStringTokenizer strTokens;
    wxString strWrk;
    
    // Must have a valid pointer
    if ( NULL == pChannelInfo ) return CANAL_ERROR_PARAMETER;
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return CANAL_ERROR_PARAMETER;
    
    m_psock->Write( _("CHID\r\n"), 6 );
    if ( !checkReturnValue() ) return CANAL_ERROR_GENERIC;
    
    strWrk = m_strReply;

    // Channel
    strTokens.SetString( strWrk, _(",\r\n") );
    if ( strTokens.HasMoreTokens() ) {
        long val;
        ( strTokens.GetNextToken() ).ToLong( &val );
        pChannelInfo->channel = (uint16_t)val;
    }
    else {
        return CANAL_ERROR_GENERIC;
    }
        
    // Set the interface level/type
    pChannelInfo->channelType = CANAL_COMMAND_OPEN_VSCP_LEVEL2;

    // Get the channel GUID
    rv = doCmdGetGUID( pChannelInfo->GUID );

    return rv;

}

///////////////////////////////////////////////////////////////////////////////
// doCmdGetChannelID
//

int VscpTcpIf::doCmdGetChannelID( uint32_t *pChannelID )
{
    int pos;
    wxString strLine;
  
    // Check pointer
    if ( NULL == pChannelID ) return CANAL_ERROR_PARAMETER;
  
    wxString strCmd(_("CHID\r\n"));
    m_psock->Write( strCmd.mb_str(), strCmd.length() );
    if ( !checkReturnValue() ) return CANAL_ERROR_GENERIC;
    
    // Handle the data (if any)

    pos = m_strReply.Find(_("\r"));
    if ( !pos ) {
        return CANAL_ERROR_GENERIC; // No reply data
    }
    
    // Save the line
    strLine = m_strReply.Left( pos );

    unsigned long val;
    if ( !strLine.ToULong( &val ) ) {
        return CANAL_ERROR_GENERIC;
    }
    *pChannelID = val;
  
  return CANAL_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdInterfaceList
//

int VscpTcpIf::doCmdInterfaceList( wxArrayString& array )
{
    wxString strCmd(_("INTERFACE LIST\r\n"));
    m_psock->Write( strCmd.mb_str(), strCmd.length() );
    if ( !checkReturnValue() ) return CANAL_ERROR_GENERIC;
    
    // Handle the data (if any)
    wxStringTokenizer tkz( m_strReply, _("\r\n") );
    while ( tkz.HasMoreTokens() ) {
        wxString str = tkz.GetNextToken();
        if ( wxNOT_FOUND == str.Find( _("+OK") ) ) {
            array.Add( str );
        }
    }
  
    return CANAL_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdShutdown
//

int VscpTcpIf::doCmdShutDown( void )
{
  wxString strCmd(_("SHUTDOWN\r\n"));
  m_psock->Write( strCmd.mb_str(), strCmd.length() );
  if ( !checkReturnValue() ) return CANAL_ERROR_GENERIC;

  return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////
// getVariableString
//

bool VscpTcpIf::getVariableString( wxString& name, wxString *strValue )
{
    wxString strCmd;
    strCmd = _("VARIABLE READ ") + name + _("\r\n");
    m_psock->Write( strCmd.ToAscii(), strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue() ) return false;

    wxStringTokenizer tkz( m_strReply, _("\r\n") );
    if ( !tkz.HasMoreTokens() ) return false;
        
    // Get the string
    *strValue = tkz.GetNextToken();
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setVariableString
//

bool VscpTcpIf::setVariableString( wxString& name, const wxString& strValue )
{
    wxString strCmd;
    strCmd = _("VARIABLE WRITE ") + name + _(",,,") + strValue + _("\r\n");
    m_psock->Write( strCmd.ToAscii(), strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue() ) return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getVariableBool
//

bool VscpTcpIf::getVariableBool( wxString& name, bool *bValue )
{
    wxString strCmd;
    strCmd = _("VARIABLE READ ") + name + _("\r\n");
    m_psock->Write( strCmd.ToAscii(), strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue() ) return false;

    wxStringTokenizer tkz( m_strReply, _("\r\n") );
    if ( !tkz.HasMoreTokens() ) return false;
        
    // Check the value
    if ( wxNOT_FOUND != tkz.GetNextToken().Find( _("true") ) ) {
        *bValue = true;
    }
    else {
        *bValue = false;
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// setVariableBool
//

bool VscpTcpIf::setVariableBool( wxString& name, const bool bValue )
{
    wxString strCmd;
    wxString strValue;
    
    if ( bValue ) {
        strValue = _("TRUE");
    }
    else {
        strValue = _("FALSE");
    }
    strCmd = _("VARIABLE WRITE ") + name + _(",,,") + strValue + _("\r\n");
    m_psock->Write( strCmd.ToAscii(), strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue() ) return false;

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// getVariableInt
//

bool VscpTcpIf::getVariableInt( wxString& name, int *value )
{
    wxString strCmd;
    strCmd = _("VARIABLE READ ") + name + _("\r\n");
    m_psock->Write( strCmd.ToAscii(), strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue() ) return false;

    wxStringTokenizer tkz( m_strReply, _("\r\n") );
    if ( !tkz.HasMoreTokens() ) return false;
        
    // Get the value
    long retval;
    if ( tkz.GetNextToken().ToLong( &retval ) ) {
        *value = retval;
    }
    
    return true;
}


///////////////////////////////////////////////////////////////////////////////
// setVariableInt
//

bool VscpTcpIf::setVariableInt( wxString& name, int value )
{
    wxString strCmd;
    wxString strValue;
    
    strValue.Printf(  _("%d"), value );
    strCmd = _("VARIABLE WRITE ") + name + _(",,,") + strValue + _("\r\n");
    m_psock->Write( strCmd.ToAscii(), strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue() ) return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getVariableLong
//

bool VscpTcpIf::getVariableLong( wxString& name, long *value )
{
    wxString strCmd;
    strCmd = _("VARIABLE READ ") + name + _("\r\n");
    m_psock->Write( strCmd.ToAscii(), strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue() ) return false;

    wxStringTokenizer tkz( m_strReply, _("\r\n") );
    if ( !tkz.HasMoreTokens() ) return false;
        
    // Get the value
    long retval;
    if ( tkz.GetNextToken().ToLong( &retval ) ) {
        *value = retval;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setVariableLong
//

bool VscpTcpIf::setVariableLong( wxString& name, long value )
{
    wxString strCmd;
    wxString strValue;
    
    strValue.Printf( _("%d"), value );
    strCmd = _("VARIABLE WRITE ") + name + _(",,,") + strValue + _("\r\n");
    m_psock->Write( strCmd.ToAscii(), strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue() ) return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getVariableDouble
//

bool VscpTcpIf::getVariableDouble( wxString& name, double *value )
{
    wxString strCmd;
    strCmd = _("VARIABLE READ ") + name + _("\r\n");
    m_psock->Write( strCmd.ToAscii(), strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue() ) return false;

    wxStringTokenizer tkz( m_strReply, _("\r\n") );
    if ( !tkz.HasMoreTokens() ) return false;
        
    // Get the value
    double retval;
    if ( tkz.GetNextToken().ToDouble( &retval ) ) {
        *value = retval;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setVariableDouble
//

bool VscpTcpIf::setVariableDouble( wxString& name, double value )
{
    wxString strCmd;
    wxString strValue;
    
    strValue.Printf( _("%f"), value );
    strCmd = _("VARIABLE WRITE ") + name + _(",,,") + strValue + _("\r\n");
    m_psock->Write( strCmd.ToAscii(), strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue() ) return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getVariableMeasurement
//

bool VscpTcpIf::getVariableMeasurement( wxString& name, wxString& strValue )
{
    wxString strCmd;
    strCmd = _("VARIABLE READ ") + name + _("\r\n");
    m_psock->Write( strCmd.ToAscii(), strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue() ) return false;

    wxStringTokenizer tkz( m_strReply, _("\r\n") );
    if ( !tkz.HasMoreTokens() ) return false;
        
    // Get the value
    strValue = tkz.GetNextToken();

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setVariableMeasurement
//

bool VscpTcpIf::setVariableMeasurement( wxString& name, wxString& strValue )
{
    wxString strCmd;

    strCmd = _("VARIABLE WRITE ") + name + _(",,,") + strValue + _("\r\n");
    m_psock->Write( strCmd.ToAscii(), strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue() ) return false;

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// getVariableEvent
//

bool VscpTcpIf::getVariableEvent( wxString& name, vscpEvent *pEvent )
{
    // Check pointer
    if ( NULL == pEvent ) return false;
    
    wxString strCmd;
    strCmd = _("VARIABLE READ ") + name + _("\r\n");
    m_psock->Write( strCmd.ToAscii(), strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue() ) return false;

    wxStringTokenizer tkz( m_strReply, _("\r\n") );
    if ( !tkz.HasMoreTokens() ) return false;

    getVscpEventFromString( pEvent, tkz.GetNextToken() );

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setVariableEvent
//

bool VscpTcpIf::setVariableEvent( wxString& name, vscpEvent *pEvent )
{
    wxString strCmd;
    wxString strValue;

    writeVscpEventToString( pEvent, strValue );
    strCmd = _("VARIABLE WRITE ") + name + _(",,,") + strValue + _("\r\n");
    m_psock->Write( strCmd.ToAscii(), strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue() ) return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getVariableEventEx
//

bool VscpTcpIf::getVariableEventEx( wxString& name, vscpEventEx *pEvent )
{    
    // Check pointer
    if ( NULL == pEvent ) return false;
    
    wxString strCmd;
    strCmd = _("VARIABLE READ ") + name + _("\r\n");
    m_psock->Write( strCmd.ToAscii(), strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue() ) return false;

    wxStringTokenizer tkz( m_strReply, _("\r\n") );
    if ( !tkz.HasMoreTokens() ) return false;

    getVscpEventExFromString( pEvent, tkz.GetNextToken() );

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setVariableEventEx
//

bool VscpTcpIf::setVariableEventEx( wxString& name, vscpEventEx *pEvent )
{
    wxString strCmd;
    wxString strValue;

    writeVscpEventExToString( pEvent, strValue );
    strCmd = _("VARIABLE WRITE ") + name + _(",,,") + strValue + _("\r\n");
    m_psock->Write( strCmd.ToAscii(), strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue() ) return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getVariableGUID
//

bool VscpTcpIf::getVariableGUID( wxString& name, cguid& guid )
{    
    wxString strCmd;
    strCmd = _("VARIABLE READ ") + name + _("\r\n");
    m_psock->Write( strCmd.ToAscii(), strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue() ) return false;

    wxStringTokenizer tkz( m_strReply, _("\r\n") );
    if ( !tkz.HasMoreTokens() ) return false;

    guid.getFromString( tkz.GetNextToken() );

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setVariableGUID
//

bool VscpTcpIf::setVariableGUID( wxString& name, cguid& guid )
{
    wxString strCmd;
    wxString strValue;

    guid.toString( strValue );
    strCmd = _("VARIABLE WRITE ") + name + _(",,,") + strValue + _("\r\n");
    m_psock->Write( strCmd.ToAscii(), strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue() ) return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getVariableVSCPdata
//

bool VscpTcpIf::getVariableVSCPdata( wxString& name, uint16_t *psizeData, uint8_t *pData )
{
    // Check pointer
    if ( NULL == pData ) return false;
    
    wxString strCmd;
    strCmd = _("VARIABLE READ ") + name + _("\r\n");
    m_psock->Write( strCmd.ToAscii(), strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue() ) return false;

    wxStringTokenizer tkz( m_strReply, _("\r\n") );
    if ( !tkz.HasMoreTokens() ) return false;

    getVscpDataArrayFromString( pData, psizeData, tkz.GetNextToken() );

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setVariableVSCPdata
//

bool VscpTcpIf::setVariableVSCPdata( wxString& name, uint16_t sizeData, uint8_t *pData )
{
    wxString strCmd;
    wxString strValue;

    writeVscpDataWithSizeToString( sizeData, pData, strValue );
    strCmd = _("VARIABLE WRITE ") + name + _(",,,") + strValue + _("\r\n");
    m_psock->Write( strCmd.ToAscii(), strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue() ) return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getVariableVSCPclass
//

bool VscpTcpIf::getVariableVSCPclass( wxString& name, uint16_t *vscp_class )
{
    // Check pointer
    if ( NULL == vscp_class ) return false;
    
    wxString strCmd;
    strCmd = _("VARIABLE READ ") + name + _("\r\n");
    m_psock->Write( strCmd.ToAscii(), strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue() ) return false;

    wxStringTokenizer tkz( m_strReply, _("\r\n") );
    if ( !tkz.HasMoreTokens() ) return false;

    long longVal;
    tkz.GetNextToken().ToLong( &longVal );
    *vscp_class = (uint16_t)longVal;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getVariableVSCPclass
//

bool VscpTcpIf::setVariableVSCPclass( wxString& name, uint16_t vscp_class )
{
    wxString strCmd;
    wxString strValue;

    strValue.Printf( _("%d"), vscp_class );
    strCmd = _("VARIABLE WRITE ") + name + _(",,,") + strValue + _("\r\n");
    m_psock->Write( strCmd.ToAscii(), strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue() ) return false;

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// getVariableVSCPtype
//

bool VscpTcpIf::getVariableVSCPtype( wxString& name, uint8_t *vscp_type )
{
    // Check pointer
    if ( NULL == vscp_type ) return false;
    
    wxString strCmd;
    strCmd =_("VARIABLE READ ") + name + _("\r\n");
    m_psock->Write( strCmd.ToAscii(), strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue() ) return false;

    wxStringTokenizer tkz( m_strReply, _("\r\n") );
    if ( !tkz.HasMoreTokens() ) return false;

    long longVal;
    tkz.GetNextToken().ToLong( &longVal );
    *vscp_type = (uint16_t)longVal;

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// getVariableVSCPtype
//

bool VscpTcpIf::setVariableVSCPtype( wxString& name, uint16_t vscp_type )
{
    wxString strCmd;
    wxString strValue;

    strValue.Printf( _("%d"), vscp_type );
    strCmd = _("VARIABLE WRITE ") + name + _(",,,") + strValue + _("\r\n");
    m_psock->Write( strCmd.ToAscii(), strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue() ) return false;

    return true;
}








// Variable handling








////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// Constructor
//

ctrlObjVscpTcpIf::ctrlObjVscpTcpIf()
{
    m_strUsername = _("admin");
    m_strPassword = _("secret");
    m_strHost = _("localhost");
    m_port = VSCP_LEVEL2_TCP_PORT;
    m_rxState = RX_TREAD_STATE_NONE;
    m_bQuit = false; 	 				// Dont even think of quiting yet...
    m_error = 0;      				    // No error
    m_rxChannelID = 0;				    // No receive channel
    m_txChannelID = 0;				    // No transmit channel
    m_bFilterOwnTx = false;		        // Don't filter TX
    m_bUseRXTXEvents = false;           // No events
    m_pWnd = NULL;						// No message window
    m_wndID = 0;                        // No meaage window id
    m_maxRXqueue = MAX_TREAD_RECEIVE_EVENTS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Destructor
//

ctrlObjVscpTcpIf::~ctrlObjVscpTcpIf()
{

}



////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////



DEFINE_EVENT_TYPE(wxVSCPTCPIF_RX_EVENT);
DEFINE_EVENT_TYPE(wxVSCPTCPIF_CONNECTION_LOST_EVENT);



///////////////////////////////////////////////////////////////////////////////
// VSCPTCPIP_RX_WorkerThread
//

VSCPTCPIP_RX_WorkerThread::VSCPTCPIP_RX_WorkerThread()
{
    m_pCtrlObject = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// VSCPTCPIP_RX_WorkerThread
//

VSCPTCPIP_RX_WorkerThread::~VSCPTCPIP_RX_WorkerThread()
{
  ;
}

///////////////////////////////////////////////////////////////////////////////
// Entry
//
// Is there any messages to send from Level II clients. Send it/them to all
// devices/clients except for itself.
//

void *VSCPTCPIP_RX_WorkerThread::Entry()
{
    // Must be a valid control object pointer
    if ( NULL == m_pCtrlObject ) return NULL;
    

    int rv;
    VscpTcpIf tcpifReceive; // TODO
    //wxCommandEvent eventReceive( wxVSCPTCPIF_RX_EVENT, m_pCtrlObject->m_wndID );
    //wxCommandEvent eventConnectionLost( wxVSCPTCPIF_CONNECTION_LOST_EVENT, m_pCtrlObject->m_wndID );
  
    // Must be a valid control object pointer
    if ( NULL == m_pCtrlObject ) return NULL;
  
    // Connect to the server with the control interface
    if ( CANAL_ERROR_SUCCESS != 
        tcpifReceive.doCmdOpen( m_pCtrlObject->m_strHost,
                                    m_pCtrlObject->m_port,
                                    m_pCtrlObject->m_strUsername,
                                    m_pCtrlObject->m_strPassword ) ) {
        if ( m_pCtrlObject->m_bUseRXTXEvents ) {
            // TODO if ( NULL != m_pCtrlObject->m_pWnd ) wxPostEvent( m_pCtrlObject->m_pWnd, eventConnectionLost );
        }

        m_pCtrlObject->m_rxState = RX_TREAD_STATE_FAIL_DISCONNECTED;
    
        return NULL;
  
    }

    m_pCtrlObject->m_rxState = RX_TREAD_STATE_CONNECTED;
  
    // Find the channel id
    tcpifReceive.doCmdGetChannelID( &m_pCtrlObject->m_rxChannelID );

    // Start Receive Loop
    tcpifReceive.doCmdEnterReceiveLoop();

  
    while ( !TestDestroy() && !m_pCtrlObject->m_bQuit ) {

        vscpEvent *pEvent = new vscpEvent;
        if ( NULL == pEvent ) break;

        if ( CANAL_ERROR_SUCCESS == 
            ( rv = tcpifReceive.doCmdBlockReceive( pEvent ) ) ) {
            
            if ( m_pCtrlObject->m_bFilterOwnTx && ( m_pCtrlObject->m_txChannelID == pEvent->obid ) )  {
                deleteVSCPevent( pEvent );
                continue;
            }

            if ( m_pCtrlObject->m_bUseRXTXEvents ) {
                //eventReceive.SetClientData( pEvent );
                //if ( NULL != m_pCtrlObject->m_pWnd ) wxPostEvent( m_pCtrlObject->m_pWnd, eventReceive );
            }
            else {
                if ( m_pCtrlObject->m_rxQueue.GetCount() <= m_pCtrlObject->m_maxRXqueue ) {
                    // Add the event to the in queue
                    m_pCtrlObject->m_mutexRxQueue.Lock();
                    m_pCtrlObject->m_rxQueue.Append( pEvent );
                    m_pCtrlObject->m_semRxQueue.Post();
                    m_pCtrlObject->m_mutexRxQueue.Unlock();
                }
                else {
                    delete pEvent;
                }
            }
        }
        else {
            delete pEvent;
            if ( CANAL_ERROR_COMMUNICATION == rv ) {
                m_pCtrlObject->m_rxState = RX_TREAD_STATE_FAIL_DISCONNECTED;
                m_pCtrlObject->m_bQuit = true;
            }
        }
    } // while

    // Close the interface
    tcpifReceive.doCmdClose();

    if ( m_pCtrlObject->m_bUseRXTXEvents ) {
        // TODO if ( NULL != m_pCtrlObject->m_pWnd ) wxPostEvent( m_pCtrlObject->m_pWnd, eventConnectionLost );
    }

    if ( m_pCtrlObject->m_rxState != RX_TREAD_STATE_FAIL_DISCONNECTED ) {
        m_pCtrlObject->m_rxState = RX_TREAD_STATE_DISCONNECTED;
    }

    return NULL;

}

///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void VSCPTCPIP_RX_WorkerThread::OnExit()
{

}








///////////////////////////////////////////////////////////////////////////////
// VSCPTCPIP_TX_WorkerThread
//

VSCPTCPIP_TX_WorkerThread::VSCPTCPIP_TX_WorkerThread()
{
    m_pCtrlObject = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// VSCPTCPIP_TX_WorkerThread
//

VSCPTCPIP_TX_WorkerThread::~VSCPTCPIP_TX_WorkerThread()
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
// Entry
//
// Is there any messages to send from Level II clients. Send it/them to all
// devices/clients except for itself.
//

void *VSCPTCPIP_TX_WorkerThread::Entry()
{
    // Must be a valid control object pointer
    if ( NULL == m_pCtrlObject ) return NULL;
    
     VscpTcpIf tcpifTransmit;
    //wxCommandEvent eventConnectionLost( wxVSCPTCPIF_CONNECTION_LOST_EVENT, m_pCtrlObject->m_wndID );
  
    // Must be a valid control object pointer
    if ( NULL == m_pCtrlObject ) return NULL;
  
    // Connect to the server with the control interface
    if ( CANAL_ERROR_SUCCESS != 
        tcpifTransmit.doCmdOpen( m_pCtrlObject->m_strHost,
                                        m_pCtrlObject->m_port,
                                        m_pCtrlObject->m_strUsername,
                                        m_pCtrlObject->m_strPassword ) ) {
        if ( m_pCtrlObject->m_bUseRXTXEvents ) {
            // TODO if ( NULL != m_pCtrlObject->m_pWnd ) wxPostEvent( m_pCtrlObject->m_pWnd, eventConnectionLost );
        }
        
        m_pCtrlObject->m_rxState = RX_TREAD_STATE_FAIL_DISCONNECTED;
        return NULL;
    }

    m_pCtrlObject->m_rxState = RX_TREAD_STATE_CONNECTED;
  
    // Find the channel id
    tcpifTransmit.doCmdGetChannelID( &m_pCtrlObject->m_txChannelID );
  
    EVENT_TX_QUEUE::compatibility_iterator node;
    vscpEvent *pEvent;
    
    while ( !TestDestroy() && !m_pCtrlObject->m_bQuit ) {
        
        if ( wxSEMA_TIMEOUT == m_pCtrlObject->m_semTxQueue.WaitTimeout( 500 ) ) continue;
        m_pCtrlObject->m_mutexTxQueue.Lock();
        node = m_pCtrlObject->m_txQueue.GetFirst();
        pEvent = node->GetData();
        tcpifTransmit.doCmdSend( pEvent );
        m_pCtrlObject->m_mutexTxQueue.Unlock();
        
    } // while

    // Close the interface
    tcpifTransmit.doCmdClose();

    if ( m_pCtrlObject->m_bUseRXTXEvents ) {
        // TODO if ( NULL != m_pCtrlObject->m_pWnd ) wxPostEvent( m_pCtrlObject->m_pWnd, eventConnectionLost );
    }

    if ( m_pCtrlObject->m_rxState != RX_TREAD_STATE_FAIL_DISCONNECTED ) {
        m_pCtrlObject->m_rxState = RX_TREAD_STATE_DISCONNECTED;
    }

    return NULL;

}

///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void VSCPTCPIP_TX_WorkerThread::OnExit()
{

}

