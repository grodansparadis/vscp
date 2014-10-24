///////////////////////////////////////////////////////////////////////////////
// VscpRemoteTcpIf.cpp: 
//
// This file is part is part of VSCP, Very Simple Control Protocol
// http://www.vscp.org)
//
// Copyright (C) 2000-2014 
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

//#include "VSCP_macro.h"
//#include "VSCP_win32_ipc.h"
//#include "../common/canal.h"
#include "vscp.h"
#include "vscpremotetcpif.h"

WX_DEFINE_LIST( EVENT_RX_QUEUE );
WX_DEFINE_LIST( EVENT_TX_QUEUE );


///////////////////////////////////////////////////////////////////////////////
// CTOR
//

clientTcpIpWorkerThread::clientTcpIpWorkerThread() : wxThread( wxTHREAD_JOINABLE )	
{
    m_bRun = true;  // Run my friend run
    m_pvscpRemoteTcpIpIf = NULL;
}


///////////////////////////////////////////////////////////////////////////////
// DTOR
//

clientTcpIpWorkerThread::~clientTcpIpWorkerThread()
{
    ;
}


///////////////////////////////////////////////////////////////////////////////
// tcpip_event_handler
//

void clientTcpIpWorkerThread::ev_handler(struct ns_connection *conn, enum ns_event ev, void *pUser) 
{
    char rbuf[ 2048 ];
    int pos4lf; 

	struct iobuf *io = &conn->recv_iobuf;
    VscpRemoteTcpIf *pTcpIfSession = (VscpRemoteTcpIf *)conn->mgr->user_data;
    if ( NULL == pTcpIfSession ) return;

    switch (ev) {
	
		case NS_CONNECT: // connect() succeeded or failed. int *success_status
            pTcpIfSession->m_bConnected = true;
            conn->flags |= NSF_USER_1;  // We should terminate 
			break;

        case NS_CLOSE:
            pTcpIfSession->m_bConnected = false;
            break;

        case NS_RECV:
			// Read new data
			memset( rbuf, 0, sizeof( rbuf ) );
			memcpy( rbuf, io->buf, io->len );
			iobuf_remove(io, io->len); 
            pTcpIfSession->m_readBuffer += wxString::FromUTF8( rbuf );

			// Check if command already is in buffer
			while ( wxNOT_FOUND != ( pos4lf = pTcpIfSession->m_readBuffer.Find( (const char)0x0a ) ) ) {
				wxString strCmdGo = pTcpIfSession->m_readBuffer.Mid( 0, pos4lf );
                // Add to array 
                pTcpIfSession->m_mutexArray.Lock();
				pTcpIfSession->m_inputStrArray.Add( strCmdGo );
                pTcpIfSession->m_mutexArray.Unlock();
				pTcpIfSession->m_readBuffer = pTcpIfSession->m_readBuffer.Right( pTcpIfSession->m_readBuffer.Length()-pos4lf-1 );
			}
			break;

    };
}

///////////////////////////////////////////////////////////////////////////////
// Entry
//

void *clientTcpIpWorkerThread::Entry()
{
    // Set up the net_skeleton communication enginee
    ns_mgr_init( &m_mgrTcpIpConnection, m_pvscpRemoteTcpIpIf, clientTcpIpWorkerThread::ev_handler );
    
    if ( NULL == ns_connect( &m_mgrTcpIpConnection, 
                                (const char *)m_hostname.mbc_str(),
                                 m_pvscpRemoteTcpIpIf ) ) {
        return NULL;
    }

    // Event loop
    while ( !TestDestroy() && m_bRun ) {
        ns_mgr_poll( &m_mgrTcpIpConnection, 50 );
    }

    // Free resources
    ns_mgr_free( &m_mgrTcpIpConnection );

    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void clientTcpIpWorkerThread::OnExit()
{

}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

VscpRemoteTcpIf::VscpRemoteTcpIf()
{	
    m_bConnected = false;
    m_pClientTcpIpWorkerThread = NULL;
    m_bModeReceiveLoop = false;
    m_responseTimeOut = DEFAULT_RESPONSE_TIMEOUT;
}


VscpRemoteTcpIf::~VscpRemoteTcpIf()
{	

}

///////////////////////////////////////////////////////////////////////////////
// checkReturnValue
//

bool VscpRemoteTcpIf::checkReturnValue( bool bClear )
{
    bool rv = false;
    int last = 0;   // last read pos in array
    wxString strReply;

    if ( bClear ) doClrInputQueue();

    long start = wxGetUTCTime();
    while ( ( wxGetUTCTime() - start ) < m_responseTimeOut ) {

        for ( uint16_t i=last; i<m_inputStrArray.Count(); i++) {

            m_mutexArray.Lock();
            strReply = m_inputStrArray[ i ];
            m_mutexArray.Unlock();
            wxLogDebug(strReply);

            if ( wxNOT_FOUND != strReply.Find(_("+OK")) ) {
                wxLogDebug( _("CHECKRETURNVALUE: Command success!") );
                return true;
            }
            else if ( wxNOT_FOUND != strReply.Find(_("-OK")) ) {
                wxLogDebug( _("CHECKRETURNVALUE: Command failed!") );
                return false;
            }

            last = i;

        }

        wxMilliSleep( 50 );

    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////
// doCommand
//

int VscpRemoteTcpIf::doCommand( wxString& cmd )
{	
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                cmd.mbc_str(),
                cmd.Length() ); 
    if ( !checkReturnValue( true ) ) {
        return VSCP_ERROR_ERROR;
    }	
    return VSCP_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// doClrInputQueue
//

void VscpRemoteTcpIf::doClrInputQueue( void )
{	
    m_mutexArray.Lock();
    m_inputStrArray.Clear();
    m_mutexArray.Unlock();
}

///////////////////////////////////////////////////////////////////////////////
// doCmdOpen
//

int VscpRemoteTcpIf::doCmdOpen( const wxString& strInterface, uint32_t flags )
{
    short port = 9598; // VSCP_LEVEL2_TCP_PORT;

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

    // Hostname
    if ( tkz.HasMoreTokens() ) {
        strHostname = tkz.GetNextToken();
    }
    
    wxLogDebug( _("strHostname = ") );
    wxLogDebug( strHostname );

    // Username
    if ( tkz.HasMoreTokens() ) {
        strUsername = tkz.GetNextToken();
    }
    
    wxLogDebug( _("strUsername = ") );
    wxLogDebug( strUsername );
    
    // Password
    if ( tkz.HasMoreTokens() ) {
        strPassword = tkz.GetNextToken();  
    }

    wxLogDebug( _("strPassword = ") );
    wxLogDebug( strPassword );
   
    return doCmdOpen( strHostname, 
                            strUsername, 
                            strPassword );
}



///////////////////////////////////////////////////////////////////////////////
// doCmdOpen
//

int VscpRemoteTcpIf::doCmdOpen( const wxString& strHostname, 
                                    const wxString& strUsername, 
                                    const wxString& strPassword )
{
    wxString strBuf;
    wxString wxstr;
   
    m_pClientTcpIpWorkerThread = new clientTcpIpWorkerThread;
    if ( NULL == m_pClientTcpIpWorkerThread ) return VSCP_ERROR_MEMORY;
    m_pClientTcpIpWorkerThread->m_pvscpRemoteTcpIpIf = this;
    m_pClientTcpIpWorkerThread->m_hostname = strHostname;
    
    // Create the worker thread
    if (wxTHREAD_NO_ERROR != m_pClientTcpIpWorkerThread->Create() ) {
        delete m_pClientTcpIpWorkerThread;
        return VSCP_ERROR_ERROR;
    }

    // Start the worker thread
    if (wxTHREAD_NO_ERROR != m_pClientTcpIpWorkerThread->Run() ) {
        delete m_pClientTcpIpWorkerThread;
        return VSCP_ERROR_ERROR;
    }

    wxLogDebug( _("Connect in progress with server ") + strHostname );
    
    // Wait for connection
    long start = wxGetUTCTime();
    while ( !m_bConnected ) {
        if ( checkReturnValue() ) break;
        if ( ( wxGetUTCTime() - start ) > (3 * m_responseTimeOut) ) {
            m_pClientTcpIpWorkerThread->m_bRun = false;
            wxLogDebug( _("No response from ") + strHostname );
            m_pClientTcpIpWorkerThread->m_bRun = false;
            return VSCP_ERROR_TIMEOUT;
        }
        wxMilliSleep( 50 );
    }
    
    wxLogDebug( _("Checking server respons") );

    // The server should reply "+OK - Success"
    bool bFound = false;
    m_mutexArray.Lock();
    for ( uint32_t i=0; i<m_inputStrArray.Count(); i++ ) {
        if ( wxNOT_FOUND != m_inputStrArray[i].Find(_("+OK - Success")) ) {
            wxLogDebug( _("Successfully connected to ") + strHostname );
            bFound = true;
            break;
        }
    }
    m_inputStrArray.Clear();
    m_mutexArray.Unlock();

    if ( !bFound ) {
        m_pClientTcpIpWorkerThread->m_bRun = false;
        wxLogDebug( _("No response from ") + strHostname );
        return VSCP_ERROR_CONNECTION;
    }

    // Username
    wxstr = strUsername;
    wxstr.Trim(false);
    strBuf =  _("USER ") + wxstr + _("\r\n"); 
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                strBuf.mbc_str(),
                strBuf.Length() ); 
    if ( !checkReturnValue(true) ) {
        return VSCP_ERROR_USER;
    }
    wxLogDebug( _("Username OK") );

    // Password
    wxstr = strPassword;
    wxstr.Trim(false);
    strBuf =  _("PASS ") + wxstr + _("\r\n");
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                strBuf.mbc_str(),
                strBuf.Length() ); 
    if ( !checkReturnValue(true) ) {
        return VSCP_ERROR_PASSWORD; 
    }
    wxLogDebug( _("Password OK") );
    
    wxLogDebug( _("Successful log in to VSCP server") );
  
    return VSCP_ERROR_SUCCESS;  
}



///////////////////////////////////////////////////////////////////////////////
// close
//

int VscpRemoteTcpIf::doCmdClose( void )
{	
    if ( m_bConnected ) {    
        // Try to behave
        wxString strCmd(_("QUIT\r\n"));
        ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                    strCmd.mbc_str(),
                    strCmd.Length() );

        if ( checkReturnValue(true) ) {
            wxLogDebug( _("Successful QUIT command.") );
        }   

    }

    if ( NULL != m_pClientTcpIpWorkerThread ) {
        m_pClientTcpIpWorkerThread->m_bRun = false;
        m_pClientTcpIpWorkerThread->Wait();
        delete m_pClientTcpIpWorkerThread;
        m_pClientTcpIpWorkerThread = NULL;
    }

    return VSCP_ERROR_SUCCESS;  
}



///////////////////////////////////////////////////////////////////////////////
// doCmdNOOP
//

int VscpRemoteTcpIf::doCmdNOOP( void )
{	
    if ( !m_bConnected ) return VSCP_ERROR_CONNECTION; // The socket is close
  
    // If receive loop actived terminate
    if ( m_bModeReceiveLoop ) return VSCP_ERROR_PARAMETER;
    
    wxString strCmd(_("NOOP\r\n"));
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                strCmd.mbc_str(),
                strCmd.Length() );

    if ( checkReturnValue(true) ) {
        wxLogDebug( _("Successful NOOP command.") );
        return VSCP_ERROR_SUCCESS;
    }
    else {
        wxLogDebug( _("Failed NOOP command.") );
        return VSCP_ERROR_ERROR;
    }
}

///////////////////////////////////////////////////////////////////////////////
// doCmdClear
//

int VscpRemoteTcpIf::doCmdClear( void )
{	
    if ( !m_bConnected ) return VSCP_ERROR_CONNECTION; 
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return VSCP_ERROR_PARAMETER;
    
    wxString strCmd(_("CLRA\r\n"));
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                    strCmd.mb_str(), 
                    strCmd.length() );

    if ( checkReturnValue(true) ) {
        return VSCP_ERROR_SUCCESS;
    }
    else {
        return VSCP_ERROR_ERROR;
    }
}


///////////////////////////////////////////////////////////////////////////////
// doCmdSend
//


int VscpRemoteTcpIf::doCmdSend( const vscpEvent *pEvent )
{	
    uint16_t i;
    
    if ( !m_bConnected ) return VSCP_ERROR_CONNECTION;
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return VSCP_ERROR_PARAMETER;
    
    wxString strBuf, strWrk, strGUID;
    unsigned char guidsum = 0;
    
    // Must be a valid data pointer if data 
    if ( ( pEvent->sizeData > 0 ) && ( NULL == pEvent->pdata ) ) return VSCP_ERROR_GENERIC;

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

    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                    strBuf.mb_str(), 
                    strBuf.length() );
  
    if ( checkReturnValue(true) ) {
        return VSCP_ERROR_SUCCESS;
    }
    else {
        return VSCP_ERROR_GENERIC;
    }
  
}



///////////////////////////////////////////////////////////////////////////////
// doCmdSendEx
//

int VscpRemoteTcpIf::doCmdSendEx( const vscpEventEx *pEvent )
{	
    uint16_t i;
    wxString strBuf, strWrk, strGUID;
    unsigned char guidsum = 0;
    
    if ( !m_bConnected ) return VSCP_ERROR_CONNECTION;
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return VSCP_ERROR_PARAMETER;
    
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

    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                    strBuf.mb_str(), 
                    strBuf.length() );
  
    if ( checkReturnValue(true) ) {
        return VSCP_ERROR_SUCCESS;
    }
    else {
        return VSCP_ERROR_GENERIC;
    }
  
}




///////////////////////////////////////////////////////////////////////////////
// doCmdSendLevel1
//

int VscpRemoteTcpIf::doCmdSendLevel1( const canalMsg *pCanalMsg )
{
    vscpEventEx event;
    
    if ( !m_bConnected ) return VSCP_ERROR_CONNECTION;
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return VSCP_ERROR_PARAMETER;
    
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

bool VscpRemoteTcpIf::getEventFromLine( const wxString& strLine, vscpEvent *pEvent )
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
        data[ pEvent->sizeData ] = vscp_readStringValue( strWrk );
        pEvent->sizeData++;

    }

    // Continue to handle GUID
    vscp_getGuidFromString( pEvent, strGUID );
  

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

int VscpRemoteTcpIf::doCmdReceive( vscpEvent *pEvent )
{	
    wxStringTokenizer strTokens;
    wxString strLine;
    wxString strWrk;
    wxString strGUID;
    
    if ( !m_bConnected ) return VSCP_ERROR_CONNECTION;   
    if ( NULL == pEvent ) return VSCP_ERROR_GENERIC;
  
     // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return VSCP_ERROR_PARAMETER;
        
    wxString strCmd(_("RETR\r\n"));
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                    strCmd.mb_str(),
                    strCmd.length() );
    if ( !checkReturnValue(true) ) return VSCP_ERROR_GENERIC;

    // Handle the data (if any)
    if ( m_inputStrArray.Count() < 2 ) return VSCP_ERROR_ERROR;
    strLine = m_inputStrArray[ m_inputStrArray.Count()-2 ];
    strLine.Trim();
    strLine.Trim(false);
  
     if ( !getEventFromLine( strLine, pEvent ) ) return VSCP_ERROR_PARAMETER;
    
    return VSCP_ERROR_SUCCESS;

}


///////////////////////////////////////////////////////////////////////////////
// doCmdReceiveEx
//

int VscpRemoteTcpIf::doCmdReceiveEx( vscpEventEx *pEventEx )
{	
    wxStringTokenizer strTokens;
    wxString strLine;
    wxString strBuf;
    wxString strWrk;
    wxString strGUID;
    
    if ( !m_bConnected ) return VSCP_ERROR_CONNECTION;
    if ( NULL == pEventEx ) return VSCP_ERROR_PARAMETER;
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return VSCP_ERROR_PARAMETER;
    
    wxString strCmd(_("RETR\r\n"));
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                    strCmd.mb_str(), 
                    strCmd.length() );
    if ( !checkReturnValue(true) ) return VSCP_ERROR_GENERIC;

    // Handle the data (if any)
    if ( m_inputStrArray.Count() < 2 ) return VSCP_ERROR_ERROR;
    strLine = m_inputStrArray[ m_inputStrArray.Count()-2 ];
    strLine.Trim();
    strLine.Trim(false);
  
    vscpEvent *pEvent = new vscpEvent;
    if ( NULL == pEvent) return VSCP_ERROR_PARAMETER;
  
    if ( !getEventFromLine( strLine, pEvent ) ) return VSCP_ERROR_PARAMETER;
  
    if ( !vscp_convertVSCPtoEx( pEventEx, pEvent ) ) {
        vscp_deleteVSCPevent( pEvent );
        return VSCP_ERROR_PARAMETER;
    }
    
    vscp_deleteVSCPevent( pEvent );
  
    return VSCP_ERROR_SUCCESS;

}



///////////////////////////////////////////////////////////////////////////////
// doCmdReceiveLevel1
//

int VscpRemoteTcpIf::doCmdReceiveLevel1( canalMsg *pCanalMsg )
{
    vscpEventEx event;

    if ( !m_bConnected ) return VSCP_ERROR_CONNECTION;
    
    // Must have a valid pointer
    if ( NULL == pCanalMsg ) return VSCP_ERROR_PARAMETER;
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return VSCP_ERROR_PARAMETER;
    
    // Fetch event
    if ( VSCP_ERROR_SUCCESS != doCmdReceiveEx( &event ) ) return VSCP_ERROR_GENERIC;

    // No CAN or Level I event if data is > 8
    if ( event.sizeData > 8 ) return VSCP_ERROR_GENERIC;


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

    return VSCP_ERROR_SUCCESS;

}


///////////////////////////////////////////////////////////////////////////////
// doCmdEnterReceiveLoop
//

int VscpRemoteTcpIf::doCmdEnterReceiveLoop( void )
{
    if ( !m_bConnected ) return VSCP_ERROR_CONNECTION;

    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return VSCP_ERROR_PARAMETER;
    
    wxString strCmd(_("RCVLOOP\r\n"));
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                strCmd.mb_str(), 
                strCmd.length() );
    if ( !checkReturnValue(true) ) return VSCP_ERROR_GENERIC;
  
    m_strReply = _("");
    m_bModeReceiveLoop = true;
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdBlockReceive
//

int VscpRemoteTcpIf::doCmdBlockReceive( vscpEvent *pEvent, uint32_t timeout )
{
    static wxString strBuf = _("");
    wxString strReply;
    
    if ( !m_bConnected ) return VSCP_ERROR_CONNECTION;

    timeout = timeout;

    // If not receive loop active terminate
    if ( !m_bModeReceiveLoop ) return VSCP_ERROR_PARAMETER;
    
    // TODO This eats 100% CPU time on Windows. Why!?
    //if ( !m_psock->WaitForRead( 0, timeout ) ) {
    //  return VSCP_ERROR_TIMEOUT;
    //}

    //m_psock->Read( buf, sizeof( buf ) - 1 );
 /* 
    if ( m_psock->Error() ) {
        if ( wxSOCKET_TIMEDOUT == m_psock->LastError() ) {
            return VSCP_ERROR_TIMEOUT;
        }
        else if ( wxSOCKET_INVOP == m_psock->LastError() ) {
            return VSCP_ERROR_COMMUNICATION;
        }   
        else if ( wxSOCKET_IOERR == m_psock->LastError() ) {
            return VSCP_ERROR_COMMUNICATION;
        }
        else if ( wxSOCKET_INVADDR == m_psock->LastError() ) {
            return VSCP_ERROR_COMMUNICATION;
        }
        else if ( wxSOCKET_INVSOCK == m_psock->LastError() ) {
            return VSCP_ERROR_COMMUNICATION;
        }
        else if ( wxSOCKET_NOHOST == m_psock->LastError() ) {
            return VSCP_ERROR_COMMUNICATION;
        }
        else if ( wxSOCKET_INVPORT == m_psock->LastError() ) {
            return VSCP_ERROR_COMMUNICATION;
        }
        else if ( wxSOCKET_WOULDBLOCK == m_psock->LastError() ) {
            return VSCP_ERROR_FIFO_EMPTY;
        }
        else if ( wxSOCKET_MEMERR == m_psock->LastError() ) {
            return VSCP_ERROR_COMMUNICATION;
        }
        else {
            return VSCP_ERROR_COMMUNICATION;
        }

    }

  
    if ( m_psock->LastCount() > 0 ) {
        buf[ m_psock->LastCount() ] = 0;
        //wxLogDebug( _("Length of reply = %d"), m_psock->LastCount() );
        strBuf += wxString( buf, wxConvUTF8 );
        //wxLogDebug( strBuf );
    }
*/

    int pos;
    int rv = VSCP_ERROR_SUCCESS;
    while ( ( pos = strBuf.Find( (wxChar) 0x0A ) ) ) {
      
        if ( wxNOT_FOUND == pos ) return VSCP_ERROR_FIFO_EMPTY;
        if ( 0 == pos ) {
            //wxLogDebug( _("pos == NULL") );
            strBuf = strBuf.Right( strBuf.Length() - 1 );
            //wxLogDebug( strBuf );
            rv = VSCP_ERROR_PARAMETER;
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
            rv = VSCP_ERROR_FIFO_EMPTY; // No data
            continue;
        }
 
        // Get the event
        if ( !getEventFromLine( strLine, pEvent ) ) {
            rv = VSCP_ERROR_PARAMETER;
            continue;
        }

        return VSCP_ERROR_SUCCESS;

    } 

    return rv;
  
}


///////////////////////////////////////////////////////////////////////////////
// doCmdDataAvailable
//

int VscpRemoteTcpIf::doCmdDataAvailable( void )
{
    wxString strLine;
    int nMsg = 0;	
  
    if ( !m_bConnected ) return VSCP_ERROR_CONNECTION;
    
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return 0;

    wxString strCmd(_("CDTA\r\n"));
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                strCmd.mb_str(), 
                strCmd.length() );

    if ( !checkReturnValue(true) ) return VSCP_ERROR_GENERIC;  
    if ( m_inputStrArray.Count() < 2 ) return VSCP_ERROR_ERROR;
    strLine = m_inputStrArray[ m_inputStrArray.Count()-2 ];
    strLine.Trim();
    strLine.Trim(false);

    long val;
    if ( strLine.ToLong(&val) ) {
        nMsg = (uint16_t)val;
    }
    

    return nMsg;
    
}


///////////////////////////////////////////////////////////////////////////////
// doCmdState
//

int VscpRemoteTcpIf::doCmdStatus( canalStatus *pStatus )
{	
    long val;
    wxString strBuf;
    wxString strWrk;
    wxString strLine;
    wxStringTokenizer strTokens;

    if ( !m_bConnected ) return VSCP_ERROR_CONNECTION;
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return VSCP_ERROR_PARAMETER;
    
    wxString strCmd(_("INFO\r\n"));
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                strCmd.mb_str(), 
                strCmd.length() );

    if ( !checkReturnValue(true) ) return VSCP_ERROR_GENERIC;
    if ( m_inputStrArray.Count() < 2 ) return VSCP_ERROR_ERROR;
    strLine = m_inputStrArray[ m_inputStrArray.Count()-2 ];

    // channelstatus
    strTokens.SetString( strLine, _(",\r\n") );
    
    // lasterrorcode
    if ( !strTokens.HasMoreTokens() ) return VSCP_ERROR_GENERIC;
    ( strTokens.GetNextToken() ).ToLong( &val );
    pStatus->lasterrorcode = val;
    
    
    // lasterrorsubcode
    if ( !strTokens.HasMoreTokens() ) return VSCP_ERROR_GENERIC;
    ( strTokens.GetNextToken() ).ToLong( &val );
    pStatus->lasterrorsubcode = val;
    

    // lasterrorsubcode
    if ( !strTokens.HasMoreTokens() ) return VSCP_ERROR_GENERIC;
    strWrk = strTokens.GetNextToken();
    memcpy( pStatus->lasterrorstr, strWrk, sizeof(pStatus->lasterrorstr) );
    
    return VSCP_ERROR_SUCCESS;

}


///////////////////////////////////////////////////////////////////////////////
// doCmdStatistics
//

int VscpRemoteTcpIf::doCmdStatistics( VSCPStatistics *pStatistics )
{	
    long val;
    wxString strBuf;
    wxString strWrk;
    wxString strLine;
    wxStringTokenizer strTokens;

    if ( !m_bConnected ) return VSCP_ERROR_CONNECTION;
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return VSCP_ERROR_PARAMETER;
        
    wxString strCmd(_("STAT\r\n"));
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                    strCmd.mb_str(), 
                    strCmd.length() );

    if ( !checkReturnValue(true) ) return VSCP_ERROR_GENERIC;
    if ( m_inputStrArray.Count() < 2 ) return VSCP_ERROR_ERROR;
    strLine = m_inputStrArray[ m_inputStrArray.Count()-2 ];
    
    strTokens.SetString( strLine, _(",\r\n"));

    // Undefined
    pStatistics->x = 0;
    if ( strTokens.HasMoreTokens() ) {
        if ( ( strTokens.GetNextToken() ).ToLong( &val ) ) {
            pStatistics->x = val;
        }
        else {
            return VSCP_ERROR_GENERIC;
        }
    }

    // Undefined
    pStatistics->y = 0;
    if ( strTokens.HasMoreTokens() ) {
        if ( ( strTokens.GetNextToken() ).ToLong( &val ) ) {
            pStatistics->y = val;
        }
        else {
            return VSCP_ERROR_GENERIC;
        }
    }

    // Undefined
    pStatistics->z = 0;
    if ( strTokens.HasMoreTokens() ) {
        if ( ( strTokens.GetNextToken() ).ToLong( &val ) ) {
            pStatistics->z = val;
        }
        else {
            return VSCP_ERROR_GENERIC;
        }
    }
    
    // Overruns
    pStatistics->cntOverruns = 0;
    if ( strTokens.HasMoreTokens() ) {
        if ( ( strTokens.GetNextToken() ).ToLong( &val ) ) {
            pStatistics->cntOverruns = val;
        }
        else {
            return VSCP_ERROR_GENERIC;
        }
    }

    // Received data
    pStatistics->cntReceiveData = 0;
    if ( strTokens.HasMoreTokens() ) {
        if ( ( strTokens.GetNextToken() ).ToLong( &val ) ) {
            pStatistics->cntReceiveData = val;
        }
        else {
            return VSCP_ERROR_GENERIC;
        }
    }
    

    // Received Frames
    pStatistics->cntReceiveFrames = 0;
    if ( strTokens.HasMoreTokens() ) {
        if ( ( strTokens.GetNextToken() ).ToLong( &val ) ) {
            pStatistics->cntReceiveFrames = val;
        }
        else {
            return VSCP_ERROR_GENERIC;
        }
    }
    
    
    // Transmitted data
    pStatistics->cntTransmitData = 0;
    if ( strTokens.HasMoreTokens() ) {
        if ( ( strTokens.GetNextToken() ).ToLong( &val ) ) {
            pStatistics->cntReceiveFrames = val;
        }
        else {
            return VSCP_ERROR_GENERIC;
        }
    }

    // Transmitted frames
    pStatistics->cntTransmitFrames = 0;
    if ( strTokens.HasMoreTokens() ) {
        if ( ( strTokens.GetNextToken() ).ToLong( &val ) ) {
            pStatistics->cntTransmitFrames = val;
        }
        else {
            return VSCP_ERROR_GENERIC;
        }
    }
    
    return VSCP_ERROR_SUCCESS;

}

///////////////////////////////////////////////////////////////////////////////
// doCmdStatistics
//

int VscpRemoteTcpIf::doCmdStatistics( canalStatistics *pStatistics )
{
	int rv;
	VSCPStatistics vscpstat;
	
	if ( NULL == pStatistics ) return VSCP_ERROR_PARAMETER;
	
	if ( VSCP_ERROR_SUCCESS != ( rv = doCmdStatistics( &vscpstat ) ) ) {
		return rv;
	}
	
	// It may be tempting to just do a copy here but don't they 
	// will be different in the future for sure.
	pStatistics->cntBusOff = 0;
	pStatistics->cntBusWarnings = 0;
	pStatistics->cntOverruns = 0;
	pStatistics->cntReceiveData = vscpstat.cntReceiveData;
	pStatistics->cntReceiveFrames = vscpstat.cntReceiveFrames;
	pStatistics->cntTransmitData = vscpstat.cntTransmitData;
	pStatistics->cntTransmitFrames = vscpstat.cntTransmitFrames;	 
	
	return VSCP_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdFilter
//

int VscpRemoteTcpIf::doCmdFilter( const vscpEventFilter *pFilter )
{	
    wxString strCmd;
    
    if ( !m_bConnected ) return VSCP_ERROR_CONNECTION;
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return VSCP_ERROR_PARAMETER;
    
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

    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                    strCmd.mb_str(), 
                    strCmd.length() );
    if ( !checkReturnValue(true) ) return VSCP_ERROR_GENERIC;


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
    
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                strCmd.mb_str(), 
                strCmd.length() );
    if ( !checkReturnValue(true) ) return VSCP_ERROR_GENERIC;
    
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdFilter
//

int VscpRemoteTcpIf::doCmdFilter( const wxString& filter, const wxString& mask )
{	
    wxString strCmd;

    if ( !m_bConnected ) return VSCP_ERROR_CONNECTION;
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return VSCP_ERROR_PARAMETER;

    // Set filter
    strCmd = _("SFLT ") + filter + _("\r\n");
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                    strCmd.mb_str(), 
                    strCmd.length() );
    if ( !checkReturnValue(true) ) return VSCP_ERROR_GENERIC;

    // Set mask
    strCmd = _("SMSK ") + mask + _("\r\n");
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                    strCmd.mb_str(), 
                    strCmd.length() );
    if ( !checkReturnValue(true) ) return VSCP_ERROR_GENERIC;

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdVersion
//

unsigned int VscpRemoteTcpIf::doCmdVersion( uint8_t *pMajorVer,
                                               uint8_t *pMinorVer,
                                               uint8_t *pSubMinorVer )
{
    long val;
    wxString strLine;
    wxStringTokenizer strTokens;

    if ( !m_bConnected ) return VSCP_ERROR_CONNECTION;
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return VSCP_ERROR_PARAMETER;
        
    wxString strCmd(_("VERS\r\n"));
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                strCmd.mb_str(), 
                strCmd.length() );
    if ( !checkReturnValue(true) ) return 0;

    if ( m_inputStrArray.Count() < 2 ) return VSCP_ERROR_ERROR;

   
    strLine = m_inputStrArray[ m_inputStrArray.Count()-2 ];
     
    strTokens.SetString( strLine, _(",\r\n"));

    // Major version
    *pMajorVer = 0;
    if ( ( strTokens.GetNextToken() ).ToLong( &val ) ) {
        *pMajorVer = (uint8_t)val;
    }
    else {
        return VSCP_ERROR_ERROR;
    }

    // Minor version
    *pMinorVer = 0;
    if ( ( strTokens.GetNextToken() ).ToLong( &val ) ) {
        *pMinorVer = (uint8_t)val;
    }
    else {
        return VSCP_ERROR_ERROR;
    }

    // Sub minor version
    *pSubMinorVer = 0;
    if ( ( strTokens.GetNextToken() ).ToLong( &val ) ) {
        *pSubMinorVer = (uint8_t)val;
    }
    else {
        return VSCP_ERROR_ERROR;
    }

    //return ( version[0] << 16 ) + ( version[1] << 8 ) + version[2];

    return VSCP_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdDLLVersion
//

unsigned long VscpRemoteTcpIf::doCmdDLLVersion( void  )
{
    return TCPIP_DLL_VERSION;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdVendorString
//

const char * VscpRemoteTcpIf::doCmdVendorString( void )
{
  return TCPIP_VENDOR_STRING;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdGetDriverInfo
//

const char * VscpRemoteTcpIf::doCmdGetDriverInfo( void )
{
    return DRIVER_INFO_STRING;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdGetGUID
//

int VscpRemoteTcpIf::doCmdGetGUID( char *pGUID )
{
    long val;
    int pos;
    wxString strLine;
    wxStringTokenizer strTokens;

    if ( !m_bConnected ) return VSCP_ERROR_CONNECTION;
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return VSCP_ERROR_PARAMETER;
        
    wxString strCmd(_("SGID\r\n"));
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                    strCmd.mb_str(), 
                    strCmd.length() );
    if ( !checkReturnValue(true) ) return VSCP_ERROR_GENERIC;

    // Handle the data (if any)
    pos = m_strReply.Find(_("\r") );
    if ( !pos ) {
        return VSCP_ERROR_GENERIC; // No reply data
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
        
        if ( idx != 16 ) return VSCP_ERROR_GENERIC;
        
    }
    else {
        return VSCP_ERROR_GENERIC;
    }
    
    return VSCP_ERROR_SUCCESS;

}


///////////////////////////////////////////////////////////////////////////////
// doCmdGetGUID
//

int VscpRemoteTcpIf::doCmdGetGUID( cguid& ifguid )
{
    //long val;
    int pos;
    wxString strLine;
    wxStringTokenizer strTokens;

    if ( !m_bConnected ) return VSCP_ERROR_CONNECTION;
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return VSCP_ERROR_PARAMETER;
        
    wxString strCmd(_("SGID\r\n"));
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                strCmd.mb_str(), 
                strCmd.length() );
    if ( !checkReturnValue(true) ) return VSCP_ERROR_GENERIC;

    // Handle the data (if any)
    pos = m_strReply.Find(_("\r") );
    if ( !pos ) {
        return VSCP_ERROR_GENERIC; // No reply data
    }
    
    // Save the line
    strLine = m_strReply.Left( pos );
    
	ifguid.getFromString(strLine);
    
    return VSCP_ERROR_SUCCESS;

}

///////////////////////////////////////////////////////////////////////////////
// doCmdSetGUID
//

int VscpRemoteTcpIf::doCmdSetGUID( const char *pGUID )
{
    int pos;
    wxString strLine;
    wxString strCmd;

    if ( !m_bConnected ) return VSCP_ERROR_CONNECTION;

    if ( NULL == pGUID ) return VSCP_ERROR_GENERIC;
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return VSCP_ERROR_PARAMETER;
        
    // Handle the data (if any)
    
    pos = m_strReply.Find(_("\r"));
    if ( !pos ) {
        return VSCP_ERROR_GENERIC;  
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
    
    
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                strCmd.mb_str(), 
                strCmd.length() );
    return checkReturnValue(true);
}


///////////////////////////////////////////////////////////////////////////////
// doCmdGetChannelInfo
//


int VscpRemoteTcpIf::doCmdGetChannelInfo( VSCPChannelInfo *pChannelInfo )
{
    int rv;
    wxStringTokenizer strTokens;
    wxString strWrk;

    if ( !m_bConnected ) return VSCP_ERROR_CONNECTION;
    
    // Must have a valid pointer
    if ( NULL == pChannelInfo ) return VSCP_ERROR_PARAMETER;
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return VSCP_ERROR_PARAMETER;
    
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                _("CHID\r\n"), 
                6 );
    if ( !checkReturnValue(true) ) return VSCP_ERROR_GENERIC;
    
    strWrk = m_strReply;

    // Channel
    strTokens.SetString( strWrk, _(",\r\n") );
    if ( strTokens.HasMoreTokens() ) {
        long val;
        ( strTokens.GetNextToken() ).ToLong( &val );
        pChannelInfo->channel = (uint16_t)val;
    }
    else {
        return VSCP_ERROR_GENERIC;
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

int VscpRemoteTcpIf::doCmdGetChannelID( uint32_t *pChannelID )
{
    int pos;
    wxString strLine;

    if ( !m_bConnected ) return VSCP_ERROR_CONNECTION;
  
    // Check pointer
    if ( NULL == pChannelID ) return VSCP_ERROR_PARAMETER;
  
    wxString strCmd(_("CHID\r\n"));
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                    strCmd.mb_str(), 
                    strCmd.length() );
    if ( !checkReturnValue(true) ) return VSCP_ERROR_GENERIC;
    
    // Handle the data (if any)

    pos = m_strReply.Find(_("\r"));
    if ( !pos ) {
        return VSCP_ERROR_GENERIC; // No reply data
    }
    
    // Save the line
    strLine = m_strReply.Left( pos );

    unsigned long val;
    if ( !strLine.ToULong( &val ) ) {
        return VSCP_ERROR_GENERIC;
    }
    *pChannelID = val;
  
  return VSCP_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdInterfaceList
//

int VscpRemoteTcpIf::doCmdInterfaceList( wxArrayString& array )
{
    if ( !m_bConnected ) return VSCP_ERROR_CONNECTION;

    wxString strCmd(_("INTERFACE LIST\r\n"));
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                strCmd.mb_str(), 
                strCmd.length() );
    if ( !checkReturnValue(true) ) return VSCP_ERROR_GENERIC;
    
    // Handle the data (if any)
    wxStringTokenizer tkz( m_strReply, _("\r\n") );
    while ( tkz.HasMoreTokens() ) {
        wxString str = tkz.GetNextToken();
        if ( wxNOT_FOUND == str.Find( _("+OK") ) ) {
            array.Add( str );
        }
    }
  
    return VSCP_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdShutdown
//

int VscpRemoteTcpIf::doCmdShutDown( void )
{
    if ( !m_bConnected ) return VSCP_ERROR_CONNECTION;

    wxString strCmd(_("SHUTDOWN\r\n"));
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                strCmd.mb_str(), 
                strCmd.length() );
    if ( !checkReturnValue(true) ) return VSCP_ERROR_GENERIC;

    return VSCP_ERROR_SUCCESS;
}






///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////






///////////////////////////////////////////////////////////////////////////////
// getVariableString
//

bool VscpRemoteTcpIf::getVariableString( wxString& name, wxString *strValue )
{
    if ( !m_bConnected ) return VSCP_ERROR_SUCCESS; // Already closed.

    wxString strCmd;
    strCmd = _("VARIABLE READ ") + name + _("\r\n");
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                strCmd.ToAscii(), 
                strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue(true) ) return false;

    wxStringTokenizer tkz( m_strReply, _("\r\n") );
    if ( !tkz.HasMoreTokens() ) return false;
        
    // Get the string
    *strValue = tkz.GetNextToken();
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setVariableString
//

bool VscpRemoteTcpIf::setVariableString( wxString& name, const wxString& strValue )
{
    if ( !m_bConnected ) return VSCP_ERROR_SUCCESS; // Already closed.

    wxString strCmd;
    strCmd = _("VARIABLE WRITE ") + name + _(",,,") + strValue + _("\r\n");
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                    strCmd.ToAscii(), 
                    strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue(true) ) return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getVariableBool
//

bool VscpRemoteTcpIf::getVariableBool( wxString& name, bool *bValue )
{
    if ( !m_bConnected ) return VSCP_ERROR_SUCCESS; // Already closed.

    wxString strCmd;
    strCmd = _("VARIABLE READ ") + name + _("\r\n");
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                    strCmd.ToAscii(), 
                    strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue(true) ) return false;

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

bool VscpRemoteTcpIf::setVariableBool( wxString& name, const bool bValue )
{
    if ( !m_bConnected ) return VSCP_ERROR_SUCCESS; // Already closed.

    wxString strCmd;
    wxString strValue;
    
    if ( bValue ) {
        strValue = _("TRUE");
    }
    else {
        strValue = _("FALSE");
    }
    strCmd = _("VARIABLE WRITE ") + name + _(",,,") + strValue + _("\r\n");
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                    strCmd.ToAscii(), 
                    strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue(true) ) return false;

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// getVariableInt
//

bool VscpRemoteTcpIf::getVariableInt( wxString& name, int *value )
{
    if ( !m_bConnected ) return VSCP_ERROR_SUCCESS; // Already closed.

    wxString strCmd;
    strCmd = _("VARIABLE READ ") + name + _("\r\n");
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                    strCmd.ToAscii(), 
                    strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue(true) ) return false;

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

bool VscpRemoteTcpIf::setVariableInt( wxString& name, int value )
{
    if ( !m_bConnected ) return VSCP_ERROR_SUCCESS; // Already closed.

    wxString strCmd;
    wxString strValue;
    
    strValue.Printf(  _("%d"), value );
    strCmd = _("VARIABLE WRITE ") + name + _(",,,") + strValue + _("\r\n");
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                strCmd.ToAscii(), 
                strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue(true) ) return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getVariableLong
//

bool VscpRemoteTcpIf::getVariableLong( wxString& name, long *value )
{
    if ( !m_bConnected ) return VSCP_ERROR_SUCCESS; // Already closed.

    wxString strCmd;
    strCmd = _("VARIABLE READ ") + name + _("\r\n");
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                strCmd.ToAscii(), 
                strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue(true) ) return false;

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

bool VscpRemoteTcpIf::setVariableLong( wxString& name, long value )
{
    if ( !m_bConnected ) return VSCP_ERROR_SUCCESS; // Already closed.

    wxString strCmd;
    wxString strValue;
    
    strValue.Printf( _("%d"), value );
    strCmd = _("VARIABLE WRITE ") + name + _(",,,") + strValue + _("\r\n");
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                strCmd.ToAscii(), 
                strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue(true) ) return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getVariableDouble
//

bool VscpRemoteTcpIf::getVariableDouble( wxString& name, double *value )
{
    if ( !m_bConnected ) return VSCP_ERROR_SUCCESS; // Already closed.

    wxString strCmd;
    strCmd = _("VARIABLE READ ") + name + _("\r\n");
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                    strCmd.ToAscii(), 
                    strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue(true) ) return false;

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

bool VscpRemoteTcpIf::setVariableDouble( wxString& name, double value )
{
    if ( !m_bConnected ) return VSCP_ERROR_SUCCESS; // Already closed.

    wxString strCmd;
    wxString strValue;
    
    strValue.Printf( _("%f"), value );
    strCmd = _("VARIABLE WRITE ") + name + _(",,,") + strValue + _("\r\n");
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                    strCmd.ToAscii(), 
                    strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue(true) ) return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getVariableMeasurement
//

bool VscpRemoteTcpIf::getVariableMeasurement( wxString& name, wxString& strValue )
{
    if ( !m_bConnected ) return VSCP_ERROR_SUCCESS; // Already closed.

    wxString strCmd;
    strCmd = _("VARIABLE READ ") + name + _("\r\n");
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                    strCmd.ToAscii(), 
                    strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue(true) ) return false;

    wxStringTokenizer tkz( m_strReply, _("\r\n") );
    if ( !tkz.HasMoreTokens() ) return false;
        
    // Get the value
    strValue = tkz.GetNextToken();

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setVariableMeasurement
//

bool VscpRemoteTcpIf::setVariableMeasurement( wxString& name, wxString& strValue )
{
    if ( !m_bConnected ) return VSCP_ERROR_SUCCESS; // Already closed.

    wxString strCmd;

    strCmd = _("VARIABLE WRITE ") + name + _(",,,") + strValue + _("\r\n");
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                    strCmd.ToAscii(), 
                    strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue(true) ) return false;

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// getVariableEvent
//

bool VscpRemoteTcpIf::getVariableEvent( wxString& name, vscpEvent *pEvent )
{
    if ( !m_bConnected ) return VSCP_ERROR_SUCCESS; // Already closed.

    // Check pointer
    if ( NULL == pEvent ) return false;
    
    wxString strCmd;
    strCmd = _("VARIABLE READ ") + name + _("\r\n");
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                strCmd.ToAscii(), 
                strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue(true) ) return false;

    wxStringTokenizer tkz( m_strReply, _("\r\n") );
    if ( !tkz.HasMoreTokens() ) return false;

    vscp_getVscpEventFromString( pEvent, tkz.GetNextToken() );

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setVariableEvent
//

bool VscpRemoteTcpIf::setVariableEvent( wxString& name, vscpEvent *pEvent )
{
    if ( !m_bConnected ) return VSCP_ERROR_SUCCESS; // Already closed.

    wxString strCmd;
    wxString strValue;

    vscp_writeVscpEventToString( pEvent, strValue );
    strCmd = _("VARIABLE WRITE ") + name + _(",,,") + strValue + _("\r\n");
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                    strCmd.ToAscii(), 
                    strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue(true) ) return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getVariableEventEx
//

bool VscpRemoteTcpIf::getVariableEventEx( wxString& name, vscpEventEx *pEvent )
{    
    if ( !m_bConnected ) return VSCP_ERROR_SUCCESS; // Already closed.

    // Check pointer
    if ( NULL == pEvent ) return false;
    
    wxString strCmd;
    strCmd = _("VARIABLE READ ") + name + _("\r\n");
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                strCmd.ToAscii(), 
                strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue(true) ) return false;

    wxStringTokenizer tkz( m_strReply, _("\r\n") );
    if ( !tkz.HasMoreTokens() ) return false;

    vscp_getVscpEventExFromString( pEvent, tkz.GetNextToken() );

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setVariableEventEx
//

bool VscpRemoteTcpIf::setVariableEventEx( wxString& name, vscpEventEx *pEvent )
{
    if ( !m_bConnected ) return VSCP_ERROR_SUCCESS; // Already closed.

    wxString strCmd;
    wxString strValue;

    vscp_writeVscpEventExToString( pEvent, strValue );
    strCmd = _("VARIABLE WRITE ") + name + _(",,,") + strValue + _("\r\n");
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                strCmd.ToAscii(), 
                strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue(true) ) return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getVariableGUID
//

bool VscpRemoteTcpIf::getVariableGUID( wxString& name, cguid& guid )
{    
    if ( !m_bConnected ) return VSCP_ERROR_SUCCESS; // Already closed.

    wxString strCmd;
    strCmd = _("VARIABLE READ ") + name + _("\r\n");
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                strCmd.ToAscii(), 
                strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue(true) ) return false;

    wxStringTokenizer tkz( m_strReply, _("\r\n") );
    if ( !tkz.HasMoreTokens() ) return false;

    guid.getFromString( tkz.GetNextToken() );

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setVariableGUID
//

bool VscpRemoteTcpIf::setVariableGUID( wxString& name, cguid& guid )
{
    if ( !m_bConnected ) return VSCP_ERROR_SUCCESS; // Already closed.

    wxString strCmd;
    wxString strValue;

    guid.toString( strValue );
    strCmd = _("VARIABLE WRITE ") + name + _(",,,") + strValue + _("\r\n");
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                strCmd.ToAscii(), 
                strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue(true) ) return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getVariableVSCPdata
//

bool VscpRemoteTcpIf::getVariableVSCPdata( wxString& name, uint16_t *psizeData, uint8_t *pData )
{
    if ( !m_bConnected ) return VSCP_ERROR_SUCCESS; // Already closed.

    // Check pointer
    if ( NULL == pData ) return false;
    
    wxString strCmd;
    strCmd = _("VARIABLE READ ") + name + _("\r\n");
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                strCmd.ToAscii(), 
                strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue(true) ) return false;

    wxStringTokenizer tkz( m_strReply, _("\r\n") );
    if ( !tkz.HasMoreTokens() ) return false;

    vscp_getVscpDataArrayFromString( pData, psizeData, tkz.GetNextToken() );

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// setVariableVSCPdata
//

bool VscpRemoteTcpIf::setVariableVSCPdata( wxString& name, uint16_t sizeData, uint8_t *pData )
{
    if ( !m_bConnected ) return VSCP_ERROR_SUCCESS; // Already closed.

    wxString strCmd;
    wxString strValue;

    vscp_writeVscpDataWithSizeToString( sizeData, pData, strValue );
    strCmd = _("VARIABLE WRITE ") + name + _(",,,") + strValue + _("\r\n");
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                    strCmd.ToAscii(), 
                    strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue(true) ) return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getVariableVSCPclass
//

bool VscpRemoteTcpIf::getVariableVSCPclass( wxString& name, uint16_t *vscp_class )
{
    if ( !m_bConnected ) return VSCP_ERROR_SUCCESS; // Already closed.

    // Check pointer
    if ( NULL == vscp_class ) return false;
    
    wxString strCmd;
    strCmd = _("VARIABLE READ ") + name + _("\r\n");
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                strCmd.ToAscii(), 
                strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue(true) ) return false;

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

bool VscpRemoteTcpIf::setVariableVSCPclass( wxString& name, uint16_t vscp_class )
{
    if ( !m_bConnected ) return VSCP_ERROR_SUCCESS; // Already closed.

    wxString strCmd;
    wxString strValue;

    strValue.Printf( _("%d"), vscp_class );
    strCmd = _("VARIABLE WRITE ") + name + _(",,,") + strValue + _("\r\n");
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                strCmd.ToAscii(), 
                strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue(true) ) return false;

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// getVariableVSCPtype
//

bool VscpRemoteTcpIf::getVariableVSCPtype( wxString& name, uint8_t *vscp_type )
{
    if ( !m_bConnected ) return VSCP_ERROR_SUCCESS; // Already closed.

    // Check pointer
    if ( NULL == vscp_type ) return false;
    
    wxString strCmd;
    strCmd =_("VARIABLE READ ") + name + _("\r\n");
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                strCmd.ToAscii(), 
                strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue(true) ) return false;

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

bool VscpRemoteTcpIf::setVariableVSCPtype( wxString& name, uint16_t vscp_type )
{
    if ( !m_bConnected ) return VSCP_ERROR_SUCCESS; // Already closed.

    wxString strCmd;
    wxString strValue;

    strValue.Printf( _("%d"), vscp_type );
    strCmd = _("VARIABLE WRITE ") + name + _(",,,") + strValue + _("\r\n");
    ns_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                strCmd.ToAscii(), 
                strlen( strCmd.ToAscii() ) );
    if ( !checkReturnValue(true) ) return false;

    return true;
}





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
    m_port = 9598;						// VSCP_LEVEL2_TCP_PORT;
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
    VscpRemoteTcpIf tcpifReceive; // TODO
    //wxCommandEvent eventReceive( wxVSCPTCPIF_RX_EVENT, m_pCtrlObject->m_wndID );
    //wxCommandEvent eventConnectionLost( wxVSCPTCPIF_CONNECTION_LOST_EVENT, m_pCtrlObject->m_wndID );
  
    // Must be a valid control object pointer
    if ( NULL == m_pCtrlObject ) return NULL;
  
    // Connect to the server with the control interface
    if ( VSCP_ERROR_SUCCESS != 
        tcpifReceive.doCmdOpen( m_pCtrlObject->m_strHost,
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

        if ( VSCP_ERROR_SUCCESS == 
            ( rv = tcpifReceive.doCmdBlockReceive( pEvent ) ) ) {
            
            if ( m_pCtrlObject->m_bFilterOwnTx && ( m_pCtrlObject->m_txChannelID == pEvent->obid ) )  {
                vscp_deleteVSCPevent( pEvent );
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
            if ( VSCP_ERROR_COMMUNICATION == rv ) {
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
    
     VscpRemoteTcpIf tcpifTransmit;
    //wxCommandEvent eventConnectionLost( wxVSCPTCPIF_CONNECTION_LOST_EVENT, m_pCtrlObject->m_wndID );
  
    // Must be a valid control object pointer
    if ( NULL == m_pCtrlObject ) return NULL;
  
    // Connect to the server with the control interface
    if ( VSCP_ERROR_SUCCESS != 
        tcpifTransmit.doCmdOpen( m_pCtrlObject->m_strHost,
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

