///////////////////////////////////////////////////////////////////////////////
// VscpRemoteTcpIf.cpp
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (c) 2000-2016 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
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

#ifdef __GNUG__
    //#pragma implementation
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

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

#include <vscp.h>
#include <vscpremotetcpif.h>

class mg_poll_server;
class mg_poll_server;
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
    m_pvscpRemoteTcpIpIf = NULL;
}


///////////////////////////////////////////////////////////////////////////////
// tcpip_event_handler
//

void clientTcpIpWorkerThread::ev_handler( struct mg_connection *conn, 
                                            int ev, 
                                            void *pUser) 
{
    char rbuf[ 2048 ];
    int pos4lf = 0;
    struct mbuf *io = NULL;
    VscpRemoteTcpIf *pTcpIfSession = NULL;
    
    if ( NULL == conn ) return;

    io = &conn->recv_mbuf;
    pTcpIfSession = ( VscpRemoteTcpIf * )conn->mgr->user_data;

    if ( NULL == pTcpIfSession ) return;

    switch (ev) {

        case MG_EV_CONNECT: // connect() succeeded or failed. int *success_status
            {
                int connect_status = 0;
                if ( NULL == pUser ) return;
                connect_status = *( int * )pUser;

                if (connect_status == 0) {

                    wxLogDebug( _("ev_handler: TCP/IP connect OK.") );
                    pTcpIfSession->m_semConnected.Post();
                    pTcpIfSession->m_bConnected = true;
                    conn->flags |= MG_F_USER_1;  // We should terminate
                } 
                else {
                    wxLogDebug( _("ev_handler: TCP/IP connect fail.") );
                }
            }
            break;

        case MG_EV_CLOSE:
            wxLogDebug( _("ev_handler: TCP/IP close.") );
            pTcpIfSession->m_bConnected = false;
            break;

        case MG_EV_RECV:

            wxLogDebug( _("ev_handler: TCP/IP receive.") );

            // Read new data
            memset( rbuf, 0, sizeof( rbuf ) );
            if ( 0 < io->len ) {

                // Protect rbuf for out of bounce access
                if ( sizeof( rbuf ) < io->len ) return;

                memcpy( rbuf, io->buf, io->len );
                mbuf_remove(io, io->len); 

                pTcpIfSession->m_readBuffer += wxString::FromUTF8( rbuf );
                wxLogDebug( wxString::FromUTF8( rbuf ) );

                // Check if command already is in buffer
                while ( wxNOT_FOUND != ( pos4lf = pTcpIfSession->m_readBuffer.Find( (const char)0x0a ) ) ) {

                    wxString strCmdGo = pTcpIfSession->m_readBuffer.Mid( 0, pos4lf );
                    
                    // If in ReceiveLoop we don't store the "+OK"s
                    if ( pTcpIfSession->m_bModeReceiveLoop ) {
                        strCmdGo.Trim();
                        strCmdGo.Trim(false);
                        if ( _("+OK") == strCmdGo ) {
                            pTcpIfSession->m_readBuffer = 
                                pTcpIfSession->m_readBuffer.Right( pTcpIfSession->m_readBuffer.Length()-pos4lf-1 );
                            continue;
                        }
                    
                    }
                    // Add to array 
                    wxString wxlog = wxString::Format(_("TCP/IP line: %s "), 
                                            (const char *)strCmdGo.c_str() );
                    wxLogDebug( wxlog );
                    pTcpIfSession->m_mutexArray.Lock();
                    pTcpIfSession->m_inputStrArray.Add( strCmdGo );
                    pTcpIfSession->m_mutexArray.Unlock();
                    if ( pTcpIfSession->m_bModeReceiveLoop ) pTcpIfSession->m_psemInputArray->Post(); // Flag that event is available
                    pTcpIfSession->m_readBuffer = 
                        pTcpIfSession->m_readBuffer.Right( pTcpIfSession->m_readBuffer.Length()-pos4lf-1 );
                }
            }
            break;

    };
}

///////////////////////////////////////////////////////////////////////////////
// Entry
//

void *clientTcpIpWorkerThread::Entry()
{
    wxLogDebug( _("clientTcpIpWorkerThread: Starting.") );
    
    // Set up the net_skeleton communication engine
    mg_mgr_init( &m_mgrTcpIpConnection, m_pvscpRemoteTcpIpIf );
    
    if ( NULL == mg_connect( &m_mgrTcpIpConnection, 
                                (const char *)m_hostname.mbc_str(),
                                 clientTcpIpWorkerThread::ev_handler ) ) {

        wxLogDebug( _("clientTcpIpWorkerThread: Connect failed!") );
        return NULL;
    }

    wxLogDebug( _("clientTcpIpWorkerThread: Before loop.") );

    // Event loop
    while ( !TestDestroy() && m_bRun ) {
        mg_mgr_poll( &m_mgrTcpIpConnection, 10 );
        //Yield();
        wxMilliSleep( 10 );
    }

    // Free resources
    mg_mgr_free( &m_mgrTcpIpConnection );

    wxLogDebug( _("clientTcpIpWorkerThread: Terminating.") );
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
    m_responseTimeOut = TCPIP_DEFAULT_RESPONSE_TIMEOUT;
    m_afterCommandSleep = TCPIP_DEFAULT_AFTER_COMMAND_SLEEP;
    m_psemInputArray = new wxSemaphore( 0, 1 ); // not signaled, max=1

    // Init. register read parameters
    m_registerOpErrorTimeout = TCPIP_REGISTER_READ_ERROR_TIMEOUT;
    m_registerOpResendTimeout = TCPIP_REGISTER_READ_RESEND_TIMEOUT;
    m_registerOpMaxRetries = TCPIP_REGISTER_READ_MAX_TRIES;
}


VscpRemoteTcpIf::~VscpRemoteTcpIf()
{
    doCmdClose();
    delete m_psemInputArray;
}

///////////////////////////////////////////////////////////////////////////////
// checkReturnValue
//

bool VscpRemoteTcpIf::checkReturnValue( bool bClear )
{
    int last = 0;   // last read pos in array
    wxString strReply;
    
    
    wxLogDebug( _("------------------------------------------------------------") );
    wxLogDebug( _("checkReturnValue:  Queue before. Responsetime = %u. "), m_responseTimeOut );
    for ( uint16_t i=0; i<m_inputStrArray.Count(); i++) {
        wxLogDebug( "***** {" + m_inputStrArray[ i ] + "} *****" );
    }
    wxLogDebug( _("------------------------------------------------------------") );

    if ( bClear ) doClrInputQueue();

    wxLongLong start = wxGetLocalTimeMillis();
    while ( ( wxGetLocalTimeMillis() - start ) < m_responseTimeOut ) {

        for ( uint16_t i=last; i<getInputQueueCount(); i++) {

            m_mutexArray.Lock();
            strReply = m_inputStrArray[ i ];
            m_mutexArray.Unlock();
            
            wxLogDebug(strReply);

            if ( wxNOT_FOUND != strReply.Find(_("+OK")) ) {
                wxLogDebug( _("checkReturnValue: Command success!") );
                return true;
            }
            else if ( wxNOT_FOUND != strReply.Find(_("-OK")) ) {
                wxLogDebug( _("checkReturnValue: Command failed!") );
                return false;
            }

            last = i;

        }

        // Give the server some time to deliver the data
        //wxMilliSleep( 200 + m_afterCommandSleep );

    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////
//  doCommand
//

int VscpRemoteTcpIf::doCommand( wxString& cmd )
{
    wxLogDebug( _("doCommand: ") + cmd );
        
    doClrInputQueue();    
    mg_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                                  cmd.mbc_str(),
                                  cmd.Length() );
    
    // Give the server some time to deliver data
    //wxMilliSleep( m_afterCommandSleep  );
    
    /*
    wxLogDebug( _("------------------------------------------------------------") );
    wxLogDebug( _("After command  %u"), m_responseTimeOut );
    for ( uint16_t i=0; i<m_inputStrArray.Count(); i++) {
        wxLogDebug( "***** {" + m_inputStrArray[ i ] + "} *****" );
    }
    wxLogDebug( _("------------------------------------------------------------") );*/
    
    if ( !checkReturnValue( false ) ) {
        wxLogDebug( _("doCommand: checkReturnValue failed") );
        return VSCP_ERROR_ERROR;
    }
    
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getInputQueueCount
//

size_t VscpRemoteTcpIf::getInputQueueCount( void )
{	
    size_t count = 0;
    
    m_mutexArray.Lock();
    count = m_inputStrArray.Count();
    m_mutexArray.Unlock();
    
    return count;
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
    wxString wxstr;
    wxString strUsername;
    wxString strPassword;
    wxString strHostname;
    
    TCPIP_UNUSED(flags);
    
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
        wxLogDebug( _("Open: Unable to create thread.") );
        delete m_pClientTcpIpWorkerThread;
        return VSCP_ERROR_ERROR;
    }

    // Start the worker thread
    if (wxTHREAD_NO_ERROR != m_pClientTcpIpWorkerThread->Run() ) {
        wxLogDebug( _("Open: Unable to start thread.") );
        delete m_pClientTcpIpWorkerThread;
        return VSCP_ERROR_ERROR;
    }

    wxLogDebug( _("============================================================") );
    wxLogDebug( _("Connect in progress with server ") + strHostname );
    wxLogDebug( _("============================================================") );
    
    //while ( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections &  )

    int rv;
    if ( wxSEMA_NO_ERROR != ( rv = m_semConnected.WaitTimeout( 3000 * (m_responseTimeOut + 1 ) ) ) ) {
        m_pClientTcpIpWorkerThread->m_bRun = false;
        wxString wxlog = wxString::Format(_("Connection failed: Code=%d - "), rv);
        wxLogDebug( wxlog+ strHostname );
        wxMilliSleep( 500 );
        return VSCP_ERROR_TIMEOUT;
    }
    
    wxLogDebug( _("Checking server response") );
    wxMilliSleep( 200 + m_afterCommandSleep );

    bool bFound = false;
    for ( int i=0; i<1; i++ ) {	
        if ( checkReturnValue() ) {
            wxLogDebug( _("+OK found from server.") );
            bFound = true;
            break;
        }

        wxLogDebug( _("Still waiting... %d"), i );
    }

    if ( !bFound ) {
        m_pClientTcpIpWorkerThread->m_bRun = false;
        wxLogDebug( _("No +OK found ") + strHostname );
        return VSCP_ERROR_CONNECTION;
    }

    // Username
    wxstr = strUsername;
    wxstr.Trim(false);
    strBuf =  _("USER ") + wxstr + _("\r\n");
    
    if ( VSCP_ERROR_SUCCESS != doCommand( strBuf ) ) {
        return VSCP_ERROR_USER;
    }
    wxLogDebug( _("Username OK") );

    // Password
    wxstr = strPassword;
    wxstr.Trim(false);
    strBuf =  _("PASS ") + wxstr + _("\r\n");
    if ( VSCP_ERROR_SUCCESS != doCommand( strBuf ) ) {
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
        (void)mg_send( m_pClientTcpIpWorkerThread->m_mgrTcpIpConnection.active_connections,
                       strCmd.mbc_str(),
                       strCmd.Length() );

        // We skip the check here as the interfaces closes  

    }

    if ( NULL != m_pClientTcpIpWorkerThread ) {
        m_pClientTcpIpWorkerThread->m_bRun = false;
        wxMilliSleep( 500 );
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
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    return VSCP_ERROR_SUCCESS;
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
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    return VSCP_ERROR_SUCCESS;
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
    if ( ( pEvent->sizeData > 0 ) && ( NULL == pEvent->pdata ) ) return VSCP_ERROR_PARAMETER;

    // Validate datasize
    if ( pEvent->sizeData > VSCP_MAX_DATA ) return VSCP_ERROR_PARAMETER;

    //send head,class,type,obid,timestamp,GUID,data1,data2,data3....
    strBuf.Printf( _( "SEND %u,%u,%u,%u,%u," ),
                    pEvent->head,
                    pEvent->vscp_class,
                    pEvent->vscp_type,
                    pEvent->obid,
                    pEvent->timestamp );

    // GUID
    for ( i=0; i<16; i++ ) {
    
        guidsum += pEvent->GUID[ i ];
        strWrk.Printf( _( "%02X" ), pEvent->GUID[ i ] );
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

    if ( 0 < pEvent->sizeData ) {
        strBuf += _(",");
    }

    // Data
    for ( i=0; i<pEvent->sizeData; i++ ) {
        strWrk.Printf( _( "%u" ), pEvent->pdata[ i ] );

        strBuf += strWrk;
        if ( i != ( pEvent->sizeData - 1 ) ) {
            strBuf += _(",");
        }
    }

    strBuf += _("\r\n");

    if ( VSCP_ERROR_SUCCESS != doCommand( strBuf ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    return VSCP_ERROR_SUCCESS;  
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

    if ( NULL == pEvent ) return VSCP_ERROR_PARAMETER;

    // Validate datasize
    if ( pEvent->sizeData > VSCP_MAX_DATA ) return VSCP_ERROR_PARAMETER;
    
    //send head,class,type,obid,timestamp,GUID,data1,data2,data3....
    strBuf.Printf( _( "SEND %u,%u,%u,%u,%u,"),
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

    if ( 0 < pEvent->sizeData ) {
        strBuf += _(",");
    }

    // Data
    for ( i=0; i<pEvent->sizeData; i++ ) {
        strWrk.Printf( _("%u"), pEvent->data[ i ] );
        strBuf += strWrk;
        if ( i != ( pEvent->sizeData - 1 ) ) {
            strBuf += _(",");
        }
    }

    strBuf += _("\r\n");

    if ( VSCP_ERROR_SUCCESS != doCommand( strBuf ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    return VSCP_ERROR_SUCCESS;
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

    if ( NULL == pCanalMsg ) return VSCP_ERROR_PARAMETER;

    event.vscp_class = (unsigned short)( 0x1ff & ( pCanalMsg->id >> 16 ) );
    event.vscp_type = (unsigned char)( 0xff & ( pCanalMsg->id >> 8 ) ); 
    event.obid = pCanalMsg->obid;
    event.timestamp = pCanalMsg->timestamp;
    event.head = 0x00;

    memset( event.GUID, 0, 16); 

    event.GUID[ 0 ] = pCanalMsg->id & 0xff;

    // Protect event.data for out ouf bounce access
    if ( sizeof( event.data ) < pCanalMsg->sizeData ) return VSCP_ERROR_PARAMETER;

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
    if ( NULL == pEvent ) return VSCP_ERROR_PARAMETER;
  
     // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return VSCP_ERROR_PARAMETER;
        
    wxString strCmd(_("RETR 1\r\n"));
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }

    // Handle the data (if any)
    if ( getInputQueueCount() < 2 ) return VSCP_ERROR_ERROR;
    m_mutexArray.Lock();
    strLine = m_inputStrArray[ m_inputStrArray.Count() - 2 ];
    m_mutexArray.Unlock();
    
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
    
    wxString strCmd(_("RETR 1\r\n"));
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }

    
    // Handle the data (if any)
    if ( getInputQueueCount() < 2 ) return VSCP_ERROR_ERROR;
    m_mutexArray.Lock();
    strLine = m_inputStrArray[ m_inputStrArray.Count() - 2 ];
    m_mutexArray.Unlock();

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
    if ( m_bModeReceiveLoop ) return VSCP_ERROR_SUCCESS;
    
    wxString strCmd(_("RCVLOOP\r\n"));
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
  
    m_mutexArray.Lock();
    m_inputStrArray.Clear();
    m_mutexArray.Unlock();

    m_bModeReceiveLoop = true;
	
    return VSCP_ERROR_SUCCESS;
}



///////////////////////////////////////////////////////////////////////////////
// doCmdQuitReceiveLoop
//

int VscpRemoteTcpIf::doCmdQuitReceiveLoop( void )
{
    if ( !m_bConnected ) return VSCP_ERROR_CONNECTION;

    // If receive loop active terminate
    if ( !m_bModeReceiveLoop ) return VSCP_ERROR_SUCCESS;
    
    wxString strCmd(_("QUITLOOP\r\n"));
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_TIMEOUT;
    }
  
    m_bModeReceiveLoop = false;
    m_mutexArray.Lock();
    m_inputStrArray.Clear();
    m_mutexArray.Unlock();

    return VSCP_ERROR_SUCCESS;
}



///////////////////////////////////////////////////////////////////////////////
// doCmdBlockingReceive
//

int VscpRemoteTcpIf::doCmdBlockingReceive(vscpEvent *pEvent, uint32_t timeout)
{
    int rv = VSCP_ERROR_SUCCESS;
    wxString strLine;

    // Check pointer
    if (NULL == pEvent) return VSCP_ERROR_PARAMETER;

    // Must be connected
    if (!m_bConnected) return VSCP_ERROR_CONNECTION;

    // If not receive loop active terminate
    if (!m_bModeReceiveLoop) return VSCP_ERROR_PARAMETER;

    if ( !getInputQueueCount() ) {
        // No need to wait for stuff if already there
        if ( !getInputQueueCount() && (wxSEMA_TIMEOUT == m_psemInputArray->WaitTimeout(timeout) ) ) {
            return VSCP_ERROR_TIMEOUT;
        }
    }

    // We have a possible incoming event
    if ( !getInputQueueCount() ) {
        return VSCP_ERROR_FIFO_EMPTY;   
    }

    m_mutexArray.Lock();
    strLine = m_inputStrArray[ 0 ];
    m_inputStrArray.RemoveAt( 0 );
    m_mutexArray.Unlock();
    strLine.Trim();
    strLine.Trim(false);

    // Get the event
    if ( !getEventFromLine( strLine, pEvent ) ) {
        rv = VSCP_ERROR_PARAMETER;
    } 

    return rv;
  
}


///////////////////////////////////////////////////////////////////////////////
// doCmdBlockingReceive
//

int VscpRemoteTcpIf::doCmdBlockingReceive( vscpEventEx *pEventEx, uint32_t timeout )
{
    int rv;
    vscpEvent e;

    // Check pointer
    if ( NULL == pEventEx ) return VSCP_ERROR_PARAMETER;

    // Must be connected
    if ( !m_bConnected ) return VSCP_ERROR_CONNECTION;

    // If not receive loop active terminate
    if ( !m_bModeReceiveLoop ) return VSCP_ERROR_PARAMETER;

    if ( rv = ( VSCP_ERROR_SUCCESS != doCmdBlockingReceive( &e, timeout ) ) ) {
        return rv;
    }

    pEventEx->head = e.head;
    pEventEx->vscp_class = e.vscp_class;e;
    pEventEx->vscp_type = e.vscp_type;
    pEventEx->obid = e.obid;
    pEventEx->timestamp = e.timestamp;
    pEventEx->crc = e.crc;
    pEventEx->sizeData = e.sizeData;
    memcpy( pEventEx->GUID, e.GUID, 16 );
    if ( ( NULL != e.pdata ) && e.sizeData ) {
        memcpy( pEventEx->data, e.pdata, e.sizeData );
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
  
    if ( !m_bConnected ) return VSCP_ERROR_ERROR;
    
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return VSCP_ERROR_ERROR;

    wxString strCmd(_("CDTA\r\n"));
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    if ( getInputQueueCount() < 2 ) return VSCP_ERROR_ERROR;
    m_mutexArray.Lock();
    strLine = m_inputStrArray[ m_inputStrArray.Count() - 2 ];
    m_mutexArray.Unlock();
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
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    if ( getInputQueueCount() < 2 ) return VSCP_ERROR_ERROR;
    m_mutexArray.Lock();
    strLine = m_inputStrArray[ m_inputStrArray.Count() - 2 ];
    m_mutexArray.Unlock();

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
    strncpy( pStatus->lasterrorstr, strWrk.mbc_str(), sizeof(pStatus->lasterrorcode) );
    
    return VSCP_ERROR_SUCCESS;

}

///////////////////////////////////////////////////////////////////////////////
// doCmdState
//

int VscpRemoteTcpIf::doCmdStatus( VSCPStatus *pStatus )
{
    canalStatus status;
    int rv = doCmdStatus( &status );

    pStatus->channel_status = status.channel_status;
    pStatus->lasterrorcode = status.lasterrorcode;
    strncpy( pStatus->lasterrorstr, status.lasterrorstr, VSCP_STATUS_ERROR_STRING_SIZE );
    pStatus->lasterrorsubcode = status.lasterrorsubcode;

    return rv;
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
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    if ( getInputQueueCount() < 2 ) return VSCP_ERROR_ERROR;
    m_mutexArray.Lock();
    strLine = m_inputStrArray[ m_inputStrArray.Count() - 2 ];
    m_mutexArray.Unlock();
    
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

    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }

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
    
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
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
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    // Set mask
    strCmd = _("SMSK ") + mask + _("\r\n");
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdVersion
//

int VscpRemoteTcpIf::doCmdVersion( uint8_t *pMajorVer,
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
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    /*
    for ( uint16_t i=0; i<getInputQueueCount(); i++) {
        wxLogDebug( "[" + m_inputStrArray[ i ] + "]");
    }
    
    for ( uint16_t i=0; i<m_inputStrArray.Count(); i++) {
        wxLogDebug( "{" + m_inputStrArray[ i ] + "}" );
    }*/
    
    if ( getInputQueueCount() < 2 ) return VSCP_ERROR_ERROR;   
    m_mutexArray.Lock();
    strLine = m_inputStrArray[ m_inputStrArray.Count() - 2 ];
    m_mutexArray.Unlock();
    
    strLine.Trim();
    strLine.Trim(false);
   
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
    wxString strLine;
    wxStringTokenizer strTokens;

    if ( !m_bConnected ) return VSCP_ERROR_CONNECTION;
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return VSCP_ERROR_PARAMETER;
        
    wxString strCmd(_("SGID\r\n"));
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    if ( getInputQueueCount() < 2 ) return VSCP_ERROR_ERROR;   
    m_mutexArray.Lock();
    strLine = m_inputStrArray[ m_inputStrArray.Count() - 2 ];
    m_mutexArray.Unlock();
    
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
    wxString strLine;
    wxStringTokenizer strTokens;

    if ( !m_bConnected ) return VSCP_ERROR_CONNECTION;
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return VSCP_ERROR_PARAMETER;
        
    wxString strCmd(_("SGID\r\n"));
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    if ( getInputQueueCount() < 2 ) return VSCP_ERROR_ERROR;   
    m_mutexArray.Lock();
    strLine = m_inputStrArray[ m_inputStrArray.Count() - 2 ];
    m_mutexArray.Unlock();
    
    ifguid.getFromString(strLine);
    
    return VSCP_ERROR_SUCCESS;

}

///////////////////////////////////////////////////////////////////////////////
// doCmdSetGUID
//

int VscpRemoteTcpIf::doCmdSetGUID( const char *pGUID )
{
    wxString strLine;
    wxString strCmd;

    if ( !m_bConnected ) return VSCP_ERROR_CONNECTION;

    if ( NULL == pGUID ) return VSCP_ERROR_GENERIC;
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return VSCP_ERROR_PARAMETER;
        
    if ( getInputQueueCount() < 2 ) return VSCP_ERROR_ERROR;   
    m_mutexArray.Lock();
    strLine = m_inputStrArray[ m_inputStrArray.Count() - 2 ];
    m_mutexArray.Unlock();
    
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
    
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    return VSCP_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdGetChannelInfo
//


int VscpRemoteTcpIf::doCmdGetChannelInfo( VSCPChannelInfo *pChannelInfo )
{
    int rv;
    wxStringTokenizer strTokens;
    wxString strLine;

    if ( !m_bConnected ) return VSCP_ERROR_CONNECTION;
    
    // Must have a valid pointer
    if ( NULL == pChannelInfo ) return VSCP_ERROR_PARAMETER;
  
    // If receive loop active terminate
    if ( m_bModeReceiveLoop ) return VSCP_ERROR_PARAMETER;
    
    wxString strCmd(_("INFO\r\n"));
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    if ( getInputQueueCount() < 2 ) return VSCP_ERROR_ERROR;   
    m_mutexArray.Lock();
    strLine = m_inputStrArray[ m_inputStrArray.Count() - 2 ];
    m_mutexArray.Unlock();

    // Channel
    strTokens.SetString( strLine, _(",\r\n") );
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
    wxString strLine;

    if ( !m_bConnected ) return VSCP_ERROR_CONNECTION;
  
    // Check pointer
    if ( NULL == pChannelID ) return VSCP_ERROR_PARAMETER;
  
    wxString strCmd(_("CHID\r\n"));
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    if ( getInputQueueCount() < 2 ) return VSCP_ERROR_ERROR;   
    m_mutexArray.Lock();
    strLine = m_inputStrArray[ m_inputStrArray.Count() - 2 ];
    m_mutexArray.Unlock();

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

int VscpRemoteTcpIf::doCmdInterfaceList( wxArrayString& wxarray )
{
    if ( !m_bConnected ) return VSCP_ERROR_CONNECTION;

    wxString strCmd(_("INTERFACE LIST\r\n"));
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    if ( getInputQueueCount() < 2 ) return VSCP_ERROR_ERROR;   
    
    // Handle the data (if any)
    for (unsigned int i=0; i<getInputQueueCount(); i++ ) {
        m_mutexArray.Lock();
        if ( wxNOT_FOUND == m_inputStrArray[ i ].Find( _("+OK") ) ) {            
            wxarray.Add( m_inputStrArray[ i ] );            
        }
        m_mutexArray.Unlock();
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
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    return VSCP_ERROR_SUCCESS;
}






///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////






///////////////////////////////////////////////////////////////////////////////
// deleteVariable
//

int VscpRemoteTcpIf::deleteVariable( wxString& name )
{
    wxString strCmd;
    
    if ( !m_bConnected ) return VSCP_ERROR_NOT_OPEN; // Connection closed.
    
    strCmd = _("VARIABLE REMOVE ") + name + _("\r\n");
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    return VSCP_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// saveVariablesToDisk
//

int VscpRemoteTcpIf::createVariable( wxString& name, 
                                        wxString& type, 
                                        wxString& strValue, 
                                        bool bPersistent )
{
    wxString strCmd;
    
    if ( !m_bConnected ) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    wxString strPersistent = ( bPersistent ) ? _("TRUE") : _("FALSE");
    strCmd = _("VARIABLE WRITE ") + name + _(";") + 
                type + _(";") + strPersistent + _(";") + 
                strValue + _("\r\n");
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    return VSCP_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// saveVariablesToDisk
//

int VscpRemoteTcpIf::saveVariablesToDisk( void )
{
    wxString strCmd;
    
    if ( !m_bConnected ) return VSCP_ERROR_NOT_OPEN; // Connection closed.
    
    strCmd = _("VARIABLE SAVE\r\n");
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    return VSCP_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// getVariableString
//

int VscpRemoteTcpIf::getVariableString( wxString& name, wxString *strValue )
{
    wxString strLine;
    wxString strCmd;

    if ( !m_bConnected ) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    strCmd = _("VARIABLE READ ") + name + _("\r\n");
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    if ( getInputQueueCount() < 2 ) return VSCP_ERROR_ERROR;   
    m_mutexArray.Lock();
    //strLine = m_inputStrArray[ m_inputStrArray.Count() - 2 ];
    for ( unsigned int i = 0; i <= m_inputStrArray.Count() - 2; i++ ) {
        strLine += m_inputStrArray[ i ];
    }
    m_mutexArray.Unlock();

    wxStringTokenizer tkz( strLine, _("\r\n") );
    if ( !tkz.HasMoreTokens() ) return VSCP_ERROR_ERROR;
        
    // Get the string
    *strValue = tkz.GetNextToken();
    
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setVariableString
//

int VscpRemoteTcpIf::setVariableString( wxString& name, const wxString& strValue )
{
    wxString strCmd;
    
    if ( !m_bConnected ) return VSCP_ERROR_NOT_OPEN; // Connection closed.
    
    strCmd = _("VARIABLE WRITE ") + name + _(";STRING;;") + strValue + _("\r\n");
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getVariableBool
//

int VscpRemoteTcpIf::getVariableBool( wxString& name, bool *bValue )
{
    wxString strLine;
    wxString strCmd;

    if ( !m_bConnected ) return VSCP_ERROR_NOT_OPEN; // Connection closed.
    
    strCmd = _("VARIABLE READ ") + name + _("\r\n");
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    if ( getInputQueueCount() < 2 ) return VSCP_ERROR_ERROR;   
    m_mutexArray.Lock();
    strLine = m_inputStrArray[ m_inputStrArray.Count() - 2 ];
    m_mutexArray.Unlock();

    wxStringTokenizer tkz( strLine, _("\r\n") );
    if ( !tkz.HasMoreTokens() ) return VSCP_ERROR_ERROR;
        
    // Check the value
    if ( wxNOT_FOUND != tkz.GetNextToken().Find( _("true") ) ) {
        *bValue = true;
    }
    else {
        *bValue = false;
    }

    return VSCP_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// setVariableBool
//

int VscpRemoteTcpIf::setVariableBool( wxString& name, const bool bValue )
{
    wxString strCmd;
    wxString strValue;
    
    if ( !m_bConnected ) return VSCP_ERROR_NOT_OPEN; // Connection closed.
    
    if ( bValue ) {
        strValue = _("TRUE");
    }
    else {
        strValue = _("FALSE");
    }
    strCmd = _("VARIABLE WRITE ") + name + _(";BOOL;;") + strValue + _("\r\n");
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    return VSCP_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// getVariableInt
//

int VscpRemoteTcpIf::getVariableInt( wxString& name, int *value )
{
    wxString strLine;
    wxString strCmd;

    if ( !m_bConnected ) return VSCP_ERROR_NOT_OPEN; // Connection closed.
    
    strCmd = _("VARIABLE READ ") + name + _("\r\n");
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    if ( getInputQueueCount() < 2 ) return VSCP_ERROR_ERROR;   
    m_mutexArray.Lock();
    strLine = m_inputStrArray[ m_inputStrArray.Count() - 2 ];
    m_mutexArray.Unlock();

    wxStringTokenizer tkz( strLine, _("\r\n") );
    if ( !tkz.HasMoreTokens() ) return VSCP_ERROR_ERROR;
        
    // Get the value
    long retval;
    if ( tkz.GetNextToken().ToLong( &retval ) ) {
        *value = retval;
    }
    
    return VSCP_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// setVariableInt
//

int VscpRemoteTcpIf::setVariableInt( wxString& name, int value )
{
    wxString strCmd;
    wxString strValue;
    
    if ( !m_bConnected ) return VSCP_ERROR_NOT_OPEN; // Connection closed.
    
    strValue.Printf(  _("%d"), value );
    strCmd = _("VARIABLE WRITE ") + name + _(";INT;;") + strValue + _("\r\n");
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getVariableLong
//

int VscpRemoteTcpIf::getVariableLong( wxString& name, long *value )
{
    wxString strLine;
    wxString strCmd;

    if ( !m_bConnected ) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    strCmd = _("VARIABLE READ ") + name + _("\r\n");
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    if ( getInputQueueCount() < 2 ) return VSCP_ERROR_ERROR;   
    m_mutexArray.Lock();
    strLine = m_inputStrArray[ m_inputStrArray.Count() - 2 ];
    m_mutexArray.Unlock();

    wxStringTokenizer tkz( strLine, _("\r\n") );
    if ( !tkz.HasMoreTokens() ) return VSCP_ERROR_ERROR;
        
    // Get the value
    long retval;
    if ( tkz.GetNextToken().ToLong( &retval ) ) {
        *value = retval;
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setVariableLong
//

int VscpRemoteTcpIf::setVariableLong( wxString& name, long value )
{
    wxString strCmd;
    wxString strValue;
    
    if ( !m_bConnected ) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    strValue.Printf( _("%d"), value );
    strCmd = _("VARIABLE WRITE ") + name + _(";LONG;;") + strValue + _("\r\n");
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getVariableDouble
//

int VscpRemoteTcpIf::getVariableDouble( wxString& name, double *value )
{
    wxString strLine;
    wxString strCmd;

    if ( !m_bConnected ) return VSCP_ERROR_NOT_OPEN; // Connection closed.
    
    strCmd = _("VARIABLE READ ") + name + _("\r\n");
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    if ( getInputQueueCount() < 2 ) return VSCP_ERROR_ERROR;   
    m_mutexArray.Lock();
    strLine = m_inputStrArray[ m_inputStrArray.Count() - 2 ];
    m_mutexArray.Unlock();

    wxStringTokenizer tkz( strLine, _("\r\n") );
    if ( !tkz.HasMoreTokens() ) return VSCP_ERROR_ERROR;
        
    // Get the value
    double retval;
    if ( tkz.GetNextToken().ToDouble( &retval ) ) {
        *value = retval;
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setVariableDouble
//

int VscpRemoteTcpIf::setVariableDouble( wxString& name, double value )
{
    wxString strCmd;
    wxString strValue;
    
    if ( !m_bConnected ) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    strValue.Printf( _("%f"), value );
    strCmd = _("VARIABLE WRITE ") + name + _(";DOUBLE;;") + strValue + _("\r\n");
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getVariableMeasurement
//

int VscpRemoteTcpIf::getVariableMeasurement( wxString& name, wxString& strValue )
{
    wxString strLine;
    wxString strCmd;

    if ( !m_bConnected ) return VSCP_ERROR_NOT_OPEN; // Connection closed.
    
    strCmd = _("VARIABLE READ ") + name + _("\r\n");
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    if ( getInputQueueCount() < 2 ) return VSCP_ERROR_ERROR;   
    m_mutexArray.Lock();
    strLine = m_inputStrArray[ m_inputStrArray.Count() - 2 ];
    m_mutexArray.Unlock();

    wxStringTokenizer tkz( strLine, _("\r\n") );
    if ( !tkz.HasMoreTokens() ) return VSCP_ERROR_ERROR;
        
    // Get the value
    strValue = tkz.GetNextToken();

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setVariableMeasurement
//

int VscpRemoteTcpIf::setVariableMeasurement( wxString& name, wxString& strValue )
{
    wxString strCmd;

    if ( !m_bConnected ) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    strCmd = _("VARIABLE WRITE ") + name + _(";MEASUREMENT;;") + strValue + _("\r\n");
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    return VSCP_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// getVariableEvent
//

int VscpRemoteTcpIf::getVariableEvent( wxString& name, vscpEvent *pEvent )
{
    wxString strLine;
    wxString strCmd;

    if ( !m_bConnected ) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    // Check pointer
    if ( NULL == pEvent ) return VSCP_ERROR_ERROR;
    
    strCmd = _("VARIABLE READ ") + name + _("\r\n");
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    if ( getInputQueueCount() < 2 ) return VSCP_ERROR_ERROR;   
    m_mutexArray.Lock();
    strLine = m_inputStrArray[ m_inputStrArray.Count() - 2 ];
    m_mutexArray.Unlock();

    wxStringTokenizer tkz( strLine, _("\r\n") );
    if ( !tkz.HasMoreTokens() ) return VSCP_ERROR_ERROR;

    vscp_setVscpEventFromString( pEvent, tkz.GetNextToken() );

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setVariableEvent
//

int VscpRemoteTcpIf::setVariableEvent( wxString& name, vscpEvent *pEvent )
{
    wxString strCmd;
    wxString strValue;

    if ( !m_bConnected ) return VSCP_ERROR_NOT_OPEN; // Connection closed.
    
    vscp_writeVscpEventToString( pEvent, strValue );
    strCmd = _("VARIABLE WRITE ") + name + _(";EVENT;;") + strValue + _("\r\n");
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getVariableEventEx
//

int VscpRemoteTcpIf::getVariableEventEx( wxString& name, vscpEventEx *pEvent )
{    
    wxString strLine;
    wxString strCmd;
    
    if ( !m_bConnected ) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    // Check pointer
    if ( NULL == pEvent ) return VSCP_ERROR_ERROR;
    
    strCmd = _("VARIABLE READ ") + name + _("\r\n");
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    if ( getInputQueueCount() < 2 ) return VSCP_ERROR_ERROR;   
    m_mutexArray.Lock();
    strLine = m_inputStrArray[ m_inputStrArray.Count() - 2 ];
    m_mutexArray.Unlock();

    wxStringTokenizer tkz( strLine, _("\r\n") );
    if ( !tkz.HasMoreTokens() ) return VSCP_ERROR_ERROR;

    vscp_setVscpEventExFromString( pEvent, tkz.GetNextToken() );

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setVariableEventEx
//

int VscpRemoteTcpIf::setVariableEventEx( wxString& name, vscpEventEx *pEvent )
{
    wxString strCmd;
    wxString strValue;

    if ( !m_bConnected ) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    vscp_writeVscpEventExToString( pEvent, strValue );
    strCmd = _("VARIABLE WRITE ") + name + _(";EVENT;;") + strValue + _("\r\n");
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getVariableGUID
//

int VscpRemoteTcpIf::getVariableGUID( wxString& name, cguid& guid )
{    
    wxString strLine;
    wxString strCmd;
    
    if ( !m_bConnected ) return VSCP_ERROR_NOT_OPEN; // Connection closed.
    
    strCmd = _("VARIABLE READ ") + name + _("\r\n");
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    if ( getInputQueueCount() < 2 ) return VSCP_ERROR_ERROR;   
    m_mutexArray.Lock();
    strLine = m_inputStrArray[ m_inputStrArray.Count() - 2 ];
    m_mutexArray.Unlock();

    wxStringTokenizer tkz( strLine, _("\r\n") );
    if ( !tkz.HasMoreTokens() ) return VSCP_ERROR_ERROR;

    guid.getFromString( tkz.GetNextToken() );

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setVariableGUID
//

int VscpRemoteTcpIf::setVariableGUID( wxString& name, cguid& guid )
{
    wxString strCmd;
    wxString strValue;

    if ( !m_bConnected ) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    guid.toString( strValue );
    strCmd = _("VARIABLE WRITE ") + name + _(";EVENTGUID;;") + strValue + _("\r\n");
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getVariableVSCPdata
//

int VscpRemoteTcpIf::getVariableVSCPdata( wxString& name, uint8_t *pData, uint16_t *psize )
{
    wxString strLine;
    wxString strCmd;

    if ( !m_bConnected ) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    // Check pointer
    if ( NULL == pData ) return VSCP_ERROR_ERROR;
    
    strCmd = _("VARIABLE READ ") + name + _("\r\n");
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    if ( getInputQueueCount() < 2 ) return VSCP_ERROR_ERROR;   
    m_mutexArray.Lock();
    for ( uint16_t i=0; i< m_inputStrArray.Count()-1; i++ ) {
        strLine += m_inputStrArray[ i ];
        strLine.Trim();
        strLine.Trim(false);
    }
    m_mutexArray.Unlock();

    vscp_setVscpDataArrayFromString( pData, psize, strLine );

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setVariableVSCPdata
//

int VscpRemoteTcpIf::setVariableVSCPdata( wxString& name, uint8_t *pData, uint16_t size )
{
    wxString strCmd;
    wxString strValue;

    if ( !m_bConnected ) return VSCP_ERROR_NOT_OPEN; // Connection closed.
    
    vscp_writeVscpDataWithSizeToString( size, pData, strValue, false, false );
    strCmd = _("VARIABLE WRITE ") + name + _(";EVENTDATA;;") + strValue + _("\r\n");
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getVariableVSCPclass
//

int VscpRemoteTcpIf::getVariableVSCPclass( wxString& name, uint16_t *vscp_class )
{
    wxString strLine;
    wxString strCmd;

    if ( !m_bConnected ) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    // Check pointer
    if ( NULL == vscp_class ) return VSCP_ERROR_ERROR;
    
    strCmd = _("VARIABLE READ ") + name + _("\r\n");
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    if ( getInputQueueCount() < 2 ) return VSCP_ERROR_ERROR;   
    m_mutexArray.Lock();
    strLine = m_inputStrArray[ m_inputStrArray.Count() - 2 ];
    m_mutexArray.Unlock();

    wxStringTokenizer tkz( strLine, _("\r\n") );
    if ( !tkz.HasMoreTokens() ) return VSCP_ERROR_ERROR;

    long longVal;
    tkz.GetNextToken().ToLong( &longVal );
    *vscp_class = (uint16_t)longVal;

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getVariableVSCPclass
//

int VscpRemoteTcpIf::setVariableVSCPclass( wxString& name, uint16_t vscp_class )
{
    wxString strCmd;
    wxString strValue;

    if ( !m_bConnected ) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    strValue.Printf( _("%d"), vscp_class );
    strCmd = _("VARIABLE WRITE ") + name + _(";EVENTCLASS;;") + strValue + _("\r\n");
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    return VSCP_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// getVariableVSCPtype
//

int VscpRemoteTcpIf::getVariableVSCPtype( wxString& name, uint16_t *vscp_type )
{
    wxString strLine;
    wxString strCmd;
    
    if ( !m_bConnected ) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    // Check pointer
    if ( NULL == vscp_type ) return VSCP_ERROR_ERROR;
    
    strCmd =_("VARIABLE READ ") + name + _("\r\n");
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    if ( getInputQueueCount() < 2 ) return VSCP_ERROR_ERROR;   
    m_mutexArray.Lock();
    strLine = m_inputStrArray[ m_inputStrArray.Count() - 2 ];
    m_mutexArray.Unlock();

    wxStringTokenizer tkz( strLine, _("\r\n") );
    if ( !tkz.HasMoreTokens() ) return VSCP_ERROR_ERROR;

    long longVal;
    tkz.GetNextToken().ToLong( &longVal );
    *vscp_type = (uint16_t)longVal;

    return VSCP_ERROR_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// getVariableVSCPtype
//

int VscpRemoteTcpIf::setVariableVSCPtype( wxString& name, uint16_t vscp_type )
{
    wxString strCmd;
    wxString strValue;

    if ( !m_bConnected ) return VSCP_ERROR_NOT_OPEN; // Connection closed.

    strValue.Printf( _("%d"), vscp_type );
    strCmd = _("VARIABLE WRITE ") + name + _(";EVENTTYPE;;") + strValue + _("\r\n");
    if ( VSCP_ERROR_SUCCESS != doCommand( strCmd ) ) {
        return VSCP_ERROR_ERROR;
    }
    
    return VSCP_ERROR_SUCCESS;
}







////////////////////////////////////////////////////////////////////////////////
//                       R E G I S T E R   H E L P E R S
////////////////////////////////////////////////////////////////////////////////







//////////////////////////////////////////////////////////////////////////////
// readLevel2Register
//

int VscpRemoteTcpIf::readLevel2Register( uint32_t reg, 
                                                uint16_t page,
												uint8_t *pval,
                                                cguid& ifGUID,
                                                cguid *pdestGUID,                                                
                                                bool bLevel2 )
{
    int rv = VSCP_ERROR_SUCCESS;
    wxString strBuf;
    vscpEventEx e;

    // Check pointers
    if ( NULL == pval ) return false; 
    if (NULL == pdestGUID ) return false;
    
    e.head = VSCP_PRIORITY_NORMAL;
    e.timestamp = 0;
    e.obid = 0;

    // Check if a specific interface is used
    if ( !ifGUID.isNULL() ) {

        // Event should be sent to a specific interface
        e.vscp_class = VSCP_CLASS2_LEVEL1_PROTOCOL;
        e.vscp_type = VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_READ;

        memset( e.GUID, 0, 16 );	        // We use GUID for interface 
        e.sizeData = 16 + 5;		        // Interface GUID + nodeid + register to read

        ifGUID.writeGUID( e.data );

        e.data[ 16 ] = pdestGUID->getLSB();	// nodeid
        e.data[ 17 ] = page>>8;             // Page MSB
        e.data[ 18 ] = page&0xff;           // Page LSB
        e.data[ 19 ] = reg;                 // Register to read
        e.data[ 20 ] = 1;                   // Number of registers to read

    }
    else {

        // Event should be sent to all interfaces

        if ( bLevel2 ) {

            // True real Level II event

            e.head = VSCP_PRIORITY_NORMAL;
            e.vscp_class = VSCP_CLASS2_PROTOCOL;
            e.vscp_type = VSCP2_TYPE_PROTOCOL_READ_REGISTER;

            memset( e.GUID, 0, 16 );            // We use GUID for interface 

            e.sizeData = 22;                    // nodeid + register to read

            pdestGUID->writeGUID( e.data );	    // Destination GUID
            e.data[ 16 ] = ( reg >> 24 ) & 0xff;// Register to read
            e.data[ 17 ] = ( reg >> 16 ) & 0xff;
            e.data[ 18 ] = ( reg >> 8 ) & 0xff;
            e.data[ 19 ] = reg & 0xff;
            e.data[ 20 ] = 0x00;                // Read one register
            e.data[ 21 ] = 0x01;

        }
        else {

            // Level I over CLASS2 to all interfaces 
            e.head = VSCP_PRIORITY_NORMAL;
            e.vscp_class = VSCP_CLASS2_LEVEL1_PROTOCOL;
            e.vscp_type = VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_READ;

            memset( e.GUID, 0, 16 );            // We use GUID for interface 
            e.sizeData = 16 + 5;                // nodeid + register to read
            pdestGUID->writeGUID( e.data );     // Destination GUID

            e.data[ 16 ] = pdestGUID->getLSB();	// nodeid
            e.data[ 17 ] = page>>8;             // Page MSB
            e.data[ 18 ] = page&0xff;           // Page LSB
            e.data[ 19 ] = reg;                 // Register to read
            e.data[ 20 ] = 1;                   // Number of registers to read

        }
    }

    // Send the event
    doCmdClear();
    e.timestamp = 0;
    doCmdSendEx( &e );

    wxLongLong resendTime = m_registerOpResendTimeout;
    wxLongLong startTime = ::wxGetLocalTimeMillis();

    while ( true ) {

        if ( 0 < doCmdDataAvailable() ) {	// Message available

            if ( VSCP_ERROR_SUCCESS == doCmdReceiveEx( &e ) ) {	// Valid event

                // Check for correct reply event
                {
                    wxString str;
                    str = wxString::Format(_("Received Event: class=%d type=%d size=%d index=%d page=%d Register=%d content=%d"), 
                                                e.vscp_class, 
                                                e.vscp_type, 
                                                e.sizeData, 
                                                e.data[16],     // frame index
                                                (e.data[17]<<8) + e.data[18], // page
                                                e.data[19],     // register
                                                e.data[20] );   // content
                    wxLogDebug(str);
                }

                // Level I Read reply?
                if ( ( VSCP_CLASS1_PROTOCOL == e.vscp_class ) && 
                    ( VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE == e.vscp_type ) ) {   
                    
                    if ( ( 0 == e.data[ 0 ] ) &&                    // Frame index is first?
                            ( (page>>8) == e.data[ 1 ] ) && 
                            ( (page&0x0ff) == e.data[ 2 ] ) && 
                            ( reg == e.data[ 3 ] ) ) {	            // Requested register?

                        if ( pdestGUID->isSameGUID( e.GUID ) ) {    // From correct node?
                            if ( NULL != pval ) {
                                *pval = e.data[ 4 ];
                            }
                            break;
                        }
                        
                    } // Check for correct node
                }

                // Level II 512 Read reply?
                else if ( !ifGUID.isNULL() && !bLevel2 && 
                    ( VSCP_CLASS2_LEVEL1_PROTOCOL == e.vscp_class ) && 
                    ( VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE == e.vscp_type ) ) { 

                    if ( pdestGUID->isSameGUID( e.GUID ) ) {
                        
                        if ( ( 0 == e.data[ 16 ] ) &&           // Frame index is first?
                            ( (page>>8) == e.data[ 17 ] ) && 
                            ( (page&0x0ff) == e.data[ 18 ] ) && 
                            ( reg == e.data[ 19 ] ) ) {	        // Requested register?

                            // OK get the data
                            if ( NULL != pval ) {
                                *pval = e.data[ 20 ];
                                break;
                            }
                        }
                    }
                
                }
                // Level II Read reply?
                else if ( ifGUID.isNULL() && bLevel2 && 
                    ( VSCP_CLASS2_PROTOCOL == e.vscp_class ) && 
                    ( VSCP2_TYPE_PROTOCOL_READ_WRITE_RESPONSE == e.vscp_type ) ) { 

                    // from us
                    if ( pdestGUID->isSameGUID( e.GUID ) ) {

                        uint32_t retreg = ( e.data[ 0 ]  << 24 ) +
                            ( e.data[ 1 ]  << 16 ) +
                            ( e.data[ 2 ]  << 8 ) +
                                e.data[ 3 ];

                        // Reg we requested?
                        if ( retreg == reg ) {
                        
                            // OK get the data
                            if ( NULL != pval ) {
                                *pval = e.data[ 4 ];
                                break;
                            }
                        }
                    }
                }

            } // valid event
            
            
        }

        if ( ( ::wxGetLocalTimeMillis() - startTime ) >  m_registerOpErrorTimeout ) {
            rv = CANAL_ERROR_TIMEOUT;
            break;
        }
        else if ( ( ::wxGetLocalTimeMillis() - startTime ) > resendTime ) {
            // Send again
            e.timestamp = 0;
            doCmdSendEx( &e );
            resendTime += m_registerOpResendTimeout;
        }
        
        wxMilliSleep( 2 );

    } // while

    return rv;
}



//////////////////////////////////////////////////////////////////////////////
// readLevel2Registers
//

int VscpRemoteTcpIf::readLevel2Registers( uint32_t reg,
                                                uint16_t page,
                                                uint8_t count,
                                                uint8_t *pval,
                                                cguid& ifGUID,
                                                cguid *pdestGUID,
                                                bool bLevel2 )
{
    int rv = CANAL_ERROR_SUCCESS;
    wxString strBuf;
    vscpEventEx e;
    uint8_t data[256];    // This makes range checking simpler

    // Max 128 bytes can be read in one go
    if ( count > 128 ) return CANAL_ERROR_PARAMETER;

    // Check pointers
    if ( NULL == pval ) return CANAL_ERROR_PARAMETER; 
    if (NULL == pdestGUID ) return CANAL_ERROR_PARAMETER;
    
    e.head = VSCP_PRIORITY_NORMAL;
    e.timestamp = 0;
    e.obid = 0;

    // Check if a specific interface is used
    if ( !ifGUID.isNULL() ) {

        // Event should be sent to a specific interface
        e.vscp_class = VSCP_CLASS2_LEVEL1_PROTOCOL;
        e.vscp_type = VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_READ;

        memset( e.GUID, 0, 16 );            // We use GUID for interface 
        e.sizeData = 16 + 5;                // Interface GUID + nodeid + register to read

        ifGUID.writeGUID( e.data );

        e.data[ 16 ] = pdestGUID->getLSB();	// nodeid
        e.data[ 17 ] = page>>8;             // Page MSB
        e.data[ 18 ] = page&0xff;           // Page LSB
        e.data[ 19 ] = reg;                 // Register to read
        e.data[ 20 ] = count;               // Read count registers

    }
    else {

        // Event should be sent to all interfaces

        if ( bLevel2 ) {

            // True real Level II event

            e.head = VSCP_PRIORITY_NORMAL;
            e.vscp_class = VSCP_CLASS2_PROTOCOL;
            e.vscp_type = VSCP2_TYPE_PROTOCOL_READ_REGISTER;

            memset( e.GUID, 0, 16 );		    // We use GUID for interface 

            e.sizeData = 22;				    // nodeid + register to read

            pdestGUID->writeGUID( e.data );	    // Destination GUID
            e.data[ 16 ] = ( reg >> 24 ) & 0xff;// Register to read
            e.data[ 17 ] = ( reg >> 16 ) & 0xff;
            e.data[ 18 ] = ( reg >> 8 ) & 0xff;
            e.data[ 19 ] = reg & 0xff;
            e.data[ 20 ] = 0;			        // Read count registers
            e.data[ 21 ] = count;

        }
        else {

            // Level I over CLASS2 to all interfaces 
            e.head = VSCP_PRIORITY_NORMAL;
            e.vscp_class = VSCP_CLASS2_LEVEL1_PROTOCOL;
            e.vscp_type = VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_READ;

            memset( e.GUID, 0, 16 );		    // We use GUID for interface 
            e.sizeData = 16 + 5;			    // nodeid + register to read
            pdestGUID->writeGUID( e.data );     // Destination GUID

            e.data[ 16 ] = pdestGUID->getLSB();	// nodeid
            e.data[ 17 ] = page>>8;             // Page MSB
            e.data[ 18 ] = page&0xff;           // Page LSB
            e.data[ 19 ] = reg;                 // Register to read
            e.data[ 20 ] = count;               // Read count registers

        }
    }

    // Send the event
    doCmdClear();
    e.timestamp = 0;
    doCmdSendEx( &e );

    // We should get thirtytwo response frames back
    unsigned long receive_flags = 0;
    unsigned char nPages = count/4;
    unsigned char lastpageCnt = count%4;
    if ( lastpageCnt ) nPages++;
    unsigned long allRcvValue = pow(2.0,nPages) - 1;

    unsigned long resendTime = m_registerOpResendTimeout;
    wxLongLong startTime = ::wxGetLocalTimeMillis();

    while ( allRcvValue != (receive_flags & 0xffffffff ) ) {	// mask for systems where long is > 32 bits

        if ( 0 < doCmdDataAvailable() ) {	// Message available

            if ( CANAL_ERROR_SUCCESS == doCmdReceiveEx( &e ) ) {	// Valid event

                // Check for correct reply event
                {
                    wxString str;
                    str = wxString::Format(_("Received Event: class=%d type=%d size=%d index=%d page=%d Register=%d content=%d"), 
                                                e.vscp_class, 
                                                e.vscp_type, 
                                                e.sizeData, 
                                                e.data[16],     // frame index
                                                (e.data[17]<<8) + e.data[18], // page
                                                e.data[19],     // register
                                                e.data[20] );   // content
                    wxLogDebug(str);
                }

                // Level I Read reply?
                if ( ( VSCP_CLASS1_PROTOCOL == e.vscp_class ) && 
                        ( VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE == e.vscp_type ) ) {   
                    
                    if ( pdestGUID->isSameGUID( e.GUID ) ) {            // From correct node?
                    
                        if ( ( (page>>8) == e.data[ 1 ] ) && 
                                ( (page&0x0ff) == e.data[ 2 ] )  ) {    // Requested page?
                        
                            // Mark frame as received
                            receive_flags |= ( 1 << e.data[ 0 ] );

                            if ( lastpageCnt && (count - 1) == e.data[ 0 ] ) {
                                // Last frame
                                for ( int i=0; i<lastpageCnt; i++ ) {
                                    data[ e.data[ 0 ]*4 + i] = e.data[ 4 + i ];
                                }
                            }
                            else {
                                for ( int i=0; i<4; i++ ) {
                                    data[ e.data[ 0 ]*4 + i] = e.data[ 4 + i ];
                                }
                            }

                        }
                        
                    } // Check for response from correct node

                }

                // Level II 512 Read reply?
                else if ( !ifGUID.isNULL() && !bLevel2 && 
                            ( VSCP_CLASS2_LEVEL1_PROTOCOL == e.vscp_class ) && 
                            ( VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE == e.vscp_type ) ) { 

                    if ( pdestGUID->isSameGUID( e.GUID ) ) {
                        
                        if ( ( (page>>8) == e.data[ 17 ] ) && 
                                ( (page&0x0ff) == e.data[ 18 ] ) ) {	    // Requested register?

                            // Mark frame as received
                            receive_flags |= ( 1 << e.data[ 16 ] );

                            if ( lastpageCnt && (count - 1) == e.data[ 16 ] ) {
                                // Last frame
                                for ( int i=0; i<lastpageCnt; i++ ) {
                                    data[ e.data[ 16 ]*4 + i] = e.data[ 16 + 4 + i ];
                                }
                            }
                            else {
                                for ( int i=0; i<4; i++ ) {
                                    data[ e.data[ 16 ]*4 + i] = e.data[ 16 + 4 + i ];
                                }
                            }

                        }

                    } // Check for response from correct node
                
                } 

                // Level II Read reply?
                else if ( ifGUID.isNULL() && bLevel2 && 
                    ( VSCP_CLASS2_PROTOCOL == e.vscp_class ) && 
                    ( VSCP2_TYPE_PROTOCOL_READ_WRITE_RESPONSE == e.vscp_type ) ) { 

                    // from us
                    if ( pdestGUID->isSameGUID( e.GUID ) ) {	

                        uint32_t retreg = ( e.data[ 0 ]  << 24 ) +
                                ( e.data[ 1 ] << 16 ) +
                                ( e.data[ 2 ] << 8 ) +
                                e.data[ 3 ];

                        // Reg we requested?
                        if ( retreg == reg ) {
                            
                            // OK get the data
                            if ( NULL != pval ) {
                                *pval = e.data[ 4 ];
                                break;
                            }
                        }
                    }
                }

            } // valid event
                        
        } // Data available

        if ( ( ::wxGetLocalTimeMillis() - startTime ) >  m_registerOpErrorTimeout ) {
            rv = CANAL_ERROR_TIMEOUT;
            break;
        }
        else if ( ( ::wxGetLocalTimeMillis() - startTime ) > resendTime ) {
            // Send again
            doCmdSendEx( &e );
            resendTime += m_registerOpResendTimeout;
        }
        
        wxMilliSleep( 20 );

    } // while

    // Copy data on success
    if ( CANAL_ERROR_SUCCESS == rv ) {
        memcpy( pval, data, count );
    }

    return rv;
}


//////////////////////////////////////////////////////////////////////////////
// writeLevel2Register
//

int VscpRemoteTcpIf::writeLevel2Register( uint32_t reg,
                                            uint16_t page, 
                                            uint8_t *pval,
                                            cguid& ifGUID,
                                            cguid *pdestGUID,
                                            bool bLevel2 )
{
    int rv = CANAL_ERROR_SUCCESS;
    bool bInterface = false;                // No specific interface set
    wxString strBuf;
    vscpEventEx e;

    // Check pointers
    if ( NULL == pval ) return CANAL_ERROR_PARAMETER;

    if ( !ifGUID.isNULL() ) {

        e.head = VSCP_PRIORITY_NORMAL;
        e.vscp_class = VSCP_CLASS2_LEVEL1_PROTOCOL;
        e.vscp_type = VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_WRITE;

        memset( e.GUID, 0, 16 );            // We use interface GUID

        e.sizeData = 16 + 5;                

        pdestGUID->writeGUID( e.data );		// Destination GUID
        e.data[16] = pdestGUID->getLSB();	// nodeid
        e.data[17] = (page >> 8);
        e.data[18] = (page & 0xff);
        e.data[19] = reg;				    // Register to write
        e.data[20] = *pval;			        // value to write
        
    }
    else {

        if ( bLevel2 ) {

            e.head = VSCP_PRIORITY_NORMAL;
            e.vscp_class = VSCP_CLASS2_PROTOCOL;
            e.vscp_type = VSCP2_TYPE_PROTOCOL_WRITE_REGISTER;

            memset( e.GUID, 0, 16 );		        // We use interface GUID
            e.sizeData = 21;				        // nodeid + register to read
            pdestGUID->writeGUID( e.data );	        // Destination GUID

            e.data[ 16 ] = ( reg >> 24 ) & 0xff;	// Register to write
            e.data[ 17 ] = ( reg >> 16 ) & 0xff;
            e.data[ 18 ] = ( reg >> 8 ) & 0xff;
            e.data[ 19 ] = reg & 0xff;
            e.data[ 20 ] = *pval;		            // Data to write

        }
        else {

            // Level I over CLASS2 to all interfaces 

            e.head = VSCP_PRIORITY_NORMAL;
            e.vscp_class = VSCP_CLASS2_LEVEL1_PROTOCOL;
            e.vscp_type = VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_WRITE;

            memset( e.GUID, 0, 16 );		    // We use interface GUID
            e.sizeData = 16 + 5;				
            pdestGUID->writeGUID( e.data );	    // Destination GUID

            e.data[16] = pdestGUID->getLSB();	// nodeid
            e.data[17] = page>>8;               // Page MSB
            e.data[18] = page&0xff;             // Page LSB
            e.data[19] = reg;                   // Register to write
            e.data[20] = *pval;	                // value to write

        }

    }
    
    // Send the event
    doCmdSendEx( &e );

    unsigned long resendTime = m_registerOpResendTimeout;
    wxLongLong startTime = ::wxGetLocalTimeMillis();

    while ( true ) {

        if ( 0 < doCmdDataAvailable() ) {                           // Message available

            if ( CANAL_ERROR_SUCCESS == doCmdReceiveEx( &e ) ) {    // Valid event
            
                if ( ( VSCP_CLASS1_PROTOCOL == e.vscp_class ) && 
                    ( VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE == e.vscp_type ) ) {    // Read reply?

                    if ( ( 0 == e.data[ 0 ] ) &&                    // Frame index is first frame
                            ( (page>>8) == e.data[ 1 ] ) &&         // Page is same as requested
                            ( (page&0xff) == e.data[ 2 ] ) &&
                            ( reg == e.data[ 3 ] ) ) {	            // Requested register?
                    
                        if ( pdestGUID->isSameGUID( e.GUID ) ) {    // From correct node?	

                            // We got a rw reply from the correct node. Is
                            // the written data same as we expect?
                            if ( *pval != e.data[ 4 ] ) rv = CANAL_ERROR_REGISTER;
                            break;

                        }
                    }   
                }

                // Level II 512 Read reply?
                else if ( !ifGUID.isNULL() && !bLevel2 && 
                    ( VSCP_CLASS2_LEVEL1_PROTOCOL == e.vscp_class ) && 
                    ( VSCP_TYPE_PROTOCOL_EXTENDED_PAGE_RESPONSE == e.vscp_type ) ) { 

                    if (pdestGUID->getLSB() == e.data[15] ) {
                    
                        if ( ( 0 == e.data[ 16 ] ) &&               // Frame index is first frame
                                ( (page>>8) == e.data[ 17 ] ) &&    // Page is same as requested
                                ( (page&0xff) == e.data[ 18 ] ) &&
                                ( reg == e.data[ 19 ] ) ) {         // Reg we requested?

                            // We got a rw reply from the correct node. Is
                            // the written data same as we expect?
                            if ( *pval != e.data[ 17 ] ) rv = CANAL_ERROR_REGISTER;
                            break;
                        }

                    }

                }
                // Level II Read reply?
                else if ( !bInterface && bLevel2 && 
                    ( VSCP_CLASS2_PROTOCOL == e.vscp_class ) && 
                    ( VSCP2_TYPE_PROTOCOL_READ_WRITE_RESPONSE == e.vscp_type ) ) { 

                    // from us
                    if ( pdestGUID->isSameGUID( e.GUID ) ) {	

                        uint32_t retreg = ( e.data[ 0 ]  << 24 ) +
                                ( e.data[ 1 ] << 16 ) +
                                ( e.data[ 2 ] << 8 ) +
                                e.data[ 3 ];

                        // Reg we requested?
                        if ( retreg == reg ) {
                            
                            // OK get the data
                            if ( NULL != pval ) {
                                // We go a rw reply from the correct node is
                                // the written data same as we expect.
                                if ( *pval != e.data[ 4 ] ) rv = CANAL_ERROR_REGISTER;
                                break;
                            }
                        }
                    }
                }
            }
        }
        else {
            wxMilliSleep( 2 );
        }

        if ( ( ::wxGetLocalTimeMillis() - startTime ) >  m_registerOpErrorTimeout ) {
            rv = CANAL_ERROR_TIMEOUT;
            break;
        }
        else if ( ( ::wxGetLocalTimeMillis() - startTime ) > resendTime ) {
            // Send again
            doCmdSendEx( &e );
            resendTime += m_registerOpResendTimeout;
        }


    } // while

    return rv;
}




//////////////////////////////////////////////////////////////////////////////
// getMDFUrlFromLevel2Device
//

bool VscpRemoteTcpIf::getMDFUrlFromLevel2Device( cguid& ifGUID,
                                                    cguid& destGUID, 
                                                    wxString& strurl,
                                                    bool bLevel2 )
{
    char url[ 33 ];
    bool rv = true;

    memset( url, 0, sizeof( url ) );

    if ( bLevel2 ) {

        // Level 2 device
        uint8_t *p = (uint8_t *)url;
        for ( int i=0; i<32; i++ ) {

            if ( CANAL_ERROR_SUCCESS != readLevel2Register( 0xFFFFFFE0 + i, 
                                                                0,          // Page = 0  
                                                                p++,        // read value
                                                                ifGUID,
                                                                &destGUID,                                        
                                                                true ) ) {  // Yes Level II
                    rv = false;
                    goto error;
            }

        }

    }
    else {

        // Level 1 device
        uint8_t *p = (uint8_t *)url;
        for ( int i=0; i<32; i++ ) {

            if ( CANAL_ERROR_SUCCESS != readLevel2Register( 0xE0 + i, 
                                                            0,          // Page = 0
                                                            p++,        // read value
                                                            ifGUID,
                                                            &destGUID ) ) {
                    rv = false;
                    goto error;
            }

        }

    }

    strurl = strurl.From8BitData( url );
    if ( wxNOT_FOUND == strurl.Find( _("http://") ) ) {
        wxString str;
        str = _("http://");
        str += strurl;
        strurl = str;
    }

error:

    if ( !rv ) strurl.Empty();

    return rv;
}









///////////////////////////////////////////////////////////////////////////////
//                           Graphical helpers
///////////////////////////////////////////////////////////////////////////////





/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// fetchIterfaceGUID
//

int VscpRemoteTcpIf::fetchIterfaceGUID( const wxString& ifName, cguid& guid )
{
    wxString str;
    wxArrayString ifarray;
    uint8_t cnt = 0;

    if ( !m_bConnected ) {
        return VSCP_ERROR_NOT_OPEN;
    }

    // Get the interface list
    while ( true ) {

        if ( VSCP_ERROR_SUCCESS == doCmdInterfaceList( ifarray ) ) {

            if ( ifarray.Count() ) {

                cnt++;

                for ( unsigned int i = 0; i < ifarray.Count(); i++ ) {

                    wxStringTokenizer tkz( ifarray[ i ], _( "," ) );
                    wxString strOrdinal = tkz.GetNextToken();
                    wxString strType = tkz.GetNextToken();
                    wxString strIfGUID = tkz.GetNextToken();
                    wxString strDescription = tkz.GetNextToken();

                    int pos;
                    wxString strName;
                    if ( wxNOT_FOUND != ( pos = strDescription.Find( _( " " ) ) ) ) {
                        strName = strDescription.Left( pos );
                        strName.Trim();
                    }

                    if ( strName.Upper() == ifName.Upper() ) {

                        // Save interface GUID;
                        guid.getFromString( strIfGUID );
                        return VSCP_ERROR_SUCCESS;

                    }

                }

            }
#if ( wxUSE_GUI != 0 )
            else {
                if ( wxYES != wxMessageBox( _( "No interfaces found. Try to find again?" ),
                    _( "Fetching interfaces" ),
                    wxYES_NO ) ) {
                    break;
                }
                else {
                    cnt = 0;
                }
            }
#endif
        }
#if ( wxUSE_GUI != 0 )
        else {
            if ( wxYES != wxMessageBox( _( "Unable to get interface list from VSCP daemon. Try to get again?" ),
                _( "Fetching interfaces" ),
                wxYES_NO ) ) {
                break;
            }
            else {
                cnt = 0;
            }
        }        
#endif

        // Give up after tree tries
        if ( cnt > 3 ) break;

    }

    return VSCP_ERROR_OPERATION_FAILED;
}







////////////////////////////////////////////////////////////////////////////////
//                         T H R E A D   H E L P E R S
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
            ( rv = tcpifReceive.doCmdBlockingReceive( pEvent ) ) ) {
            
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

