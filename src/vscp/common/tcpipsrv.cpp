// tcpipsrv.cpp
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
//
// https://wiki.openssl.org/index.php/Simple_TLS_Server
// https://wiki.openssl.org/index.php/SSL/TLS_Client
// https://stackoverflow.com/questions/3919420/tutorial-on-using-openssl-with-pthreads
//

#ifdef WIN32
#include <winsock2.h>
#endif

//#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"
#include <wx/listimpl.cpp>
#include <wx/tokenzr.h>
#include <wx/stdpaths.h>
#include <wx/xml/xml.h>
#include <wx/base64.h>
#include <wx/sstream.h>

#include <time.h>

#ifndef DWORD
#define DWORD unsigned long
#endif

#include <vscp.h>
#include <vscp_debug.h>
#include "tcpipsrv.h"
#include <vscphelper.h>
#include <mongoose.h>
#include <version.h>
#include <controlobject.h>

WX_DEFINE_LIST( TCPIPCLIENTS );

#define TCPIPSRV_INACTIVITY_TIMOUT  (3600 * 12)




///////////////////////////////////////////////////
//                 GLOBALS
///////////////////////////////////////////////////

extern CControlObject *gpobj;





// ****************************************************************************
//                               Listen thread
// ****************************************************************************





///////////////////////////////////////////////////////////////////////////////
// TCPListenThread
//
// This thread listens for connection on a TCP socket and starts a new thread
// to handle client requests
//

TCPListenThread::TCPListenThread()
    : wxThread( wxTHREAD_DETACHED )
{
    // Init. the server comtext structure 
    memset( &m_srvctx, 0, sizeof( struct server_context ) );
    
    m_idCounter = 0;
}


TCPListenThread::~TCPListenThread()
{
    ;
}


///////////////////////////////////////////////////////////////////////////////
// Entry
//

void *TCPListenThread::Entry()
{
    int i;
    struct stcp_connection *conn;
    struct socket *psocket;
    struct stcp_secure_options opts;
    struct pollfd *pfd;
    memset( &opts, 0, sizeof( opts ) );


    // * * * Init. secure options * * *


    // Certificate
    if ( gpobj->m_tcpip_ssl_certificate.Length() ) {
        opts.pem = strdup( (const char *)gpobj->m_tcpip_ssl_certificate.mbc_str() );
    }

    // Certificate chain
    if ( gpobj->m_tcpip_ssl_certificate_chain.Length() ) {
        opts.chain = strdup( (const char *)gpobj->m_tcpip_ssl_certificate_chain.mbc_str() );
    }

    opts.verify_peer = gpobj->m_tcpip_ssl_verify_peer;

    // CA path
    if ( gpobj->m_tcpip_ssl_ca_path.Length() ) {
        opts.ca_path = strdup( (const char *)gpobj->m_tcpip_ssl_ca_path.mbc_str() );
    }

    // CA file
    if ( gpobj->m_tcpip_ssl_ca_file.Length() ) {
        opts.chain = strdup( (const char *)gpobj->m_tcpip_ssl_ca_file.mbc_str() );
    }

    opts.verify_depth = gpobj->m_tcpip_ssl_verify_depth;

    opts.default_verify_path =  gpobj->m_tcpip_ssl_default_verify_paths ? 1 : 0;

    opts.protocol_version = gpobj->m_tcpip_ssl_protocol_version;

    // chiper list
    if ( gpobj->m_tcpip_ssl_cipher_list.Length() ) {
        opts.chipher_list = strdup( (const char *)gpobj->m_tcpip_ssl_cipher_list.mbc_str() );
    }

    opts.short_trust = gpobj->m_tcpip_ssl_short_trust ? 1 : 0;

    // --------------------------------------------------------------------------------------

    // Init. SSL subsystem
    /*if ( 0 == stcp_init_ssl( m_srvctx.ssl_ctx, &opts ) ) {
        gpobj->logMsg( _("[TCP/IP srv thread] Failed to init. ssl.\n"), 
                        DAEMON_LOGMSG_NORMAL );                                           
        return NULL;
    }*/

    // Bind to selected interface
    if ( 0 == stcp_listening( &m_srvctx, 
                        (const char *)gpobj->m_strTcpInterfaceAddress.mbc_str() ) ) {
        gpobj->logMsg( _("[TCP/IP srv thread] Failed to init listening socket.\n"), 
                        DAEMON_LOGMSG_NORMAL );                                         
        return NULL;                                    
    }
    
    gpobj->logMsg( _("[TCP/IP srv listen thread] Started.\n"), DAEMON_LOGMSG_DEBUG  );
        
    while ( !TestDestroy() && !(gpobj->m_StopTcpIpSrv) ) {
                 
        pfd = m_srvctx.listening_socket_fds;
        memset( pfd, 0, sizeof(*pfd) );
        for ( i = 0; i < m_srvctx.num_listening_sockets; i++ ) {
            pfd[ i ].fd = m_srvctx.listening_sockets[i].sock;
            pfd[ i ].events = POLLIN;
            pfd[ i ].revents = 0;
        }
        
        int pollres;
        if ( ( pollres = stcp_poll( pfd, 
                                        m_srvctx.num_listening_sockets, 
                                        200, 
                                        &(gpobj->m_StopTcpIpSrv) ) ) > 0  ) {
            
            for ( i = 0; i < m_srvctx.num_listening_sockets; i++ ) {
                
                // NOTE(lsm): on QNX, poll() returns POLLRDNORM after the
                // successful poll, and POLLIN is defined as
                // (POLLRDNORM | POLLRDBAND)
                // Therefore, we're checking pfd[i].revents & POLLIN, not
                // pfd[i].revents == POLLIN. 
                if ( pfd[i].revents & POLLIN ) {

                    conn = new struct stcp_connection;    // New connection
                    if ( NULL == conn ) {
                       gpobj->logMsg( _("[TCP/IP srv] -- Memory problem when creating conn object.\n") );
                       continue; 
                    }

                    memset( conn, 0, sizeof(struct stcp_connection) );
                    conn->client.id = m_idCounter++;

                    if ( stcp_accept( &m_srvctx, &m_srvctx.listening_sockets[ i ], &(conn->client) ) ) {

                        stcp_init_client_connection( conn, &opts );
                        
                        gpobj->logMsg( _("[TCP/IP srv] -- Connection accept.\n") );

                        // Create the thread
                        TCPClientThread *pThread = new TCPClientThread;
                        if ( NULL == pThread ) {
                            gpobj->logMsg( _("[TCP/IP srv] -- Memory problem when creating client thread.\n") );
                            stcp_close_connection( conn );
                            conn == NULL;
                            continue;
                        }

                        pThread->m_conn = conn;
                        pThread->m_pParent = this;

                        wxThreadError err;
                        if ( wxTHREAD_NO_ERROR == ( err = pThread->Create() ) ) {
            
                            pThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY );
            
                            if ( wxTHREAD_NO_ERROR != ( err = pThread->Run() ) ) {
                                gpobj->logMsg(_("[TCP/IP srv] -- Unable to run TCP client thread.") );
                                stcp_close_connection( conn );
                                conn == NULL;
                                delete pThread;
                                continue;
                            }

                        }
                        else {
                            gpobj->logMsg( _("[TCP/IP srv] -- Unable to create TCP client thread.") );
                            stcp_close_connection( conn );
                            conn == NULL;
                            delete pThread;
                            continue;
                        }

                        // Add conn to list of active connections
                        gpobj->m_mutexTcpClientList.Lock();
                        m_clientList.Append( conn );
                        gpobj->m_mutexTcpClientList.Unlock();

                    }
                    else {
                        delete psocket;
                        psocket = NULL;
                    }

		        }

            } // for
                        
        } // poll

        pollres = 0;

    } // While
    
    stcp_close_all_listening_sockets( &m_srvctx );

    gpobj->logMsg( _("[TCP/IP srv listen thread] Preparing Exit.\n"), DAEMON_LOGMSG_DEBUG  );

    // Wait for clients to close terminate
    int loopCnt = 0;
    while ( true ) { 
        
        gpobj->m_mutexTcpClientList.Lock();
        if ( !m_clientList.GetCount() ) break;
        gpobj->m_mutexTcpClientList.Unlock();
        
        loopCnt++;
        if ( loopCnt > 5 ) {
            gpobj->logMsg( _("[TCP/IP srv listen thread] "
                             "Clients did not end as expected. "
                             "Terminate anyway.\n") );
            break;
        }

        wxSleep( 1 );

    }

    // * * * Deallocate allocated security options * * *

    if ( NULL != opts.pem ) {
        delete opts.pem;
        opts.pem = NULL;
    }

    if ( NULL != opts.chain ) {
        delete opts.chain;
        opts.chain = NULL;
    }

    if ( NULL != opts.ca_path ) {
        delete opts.ca_path;
        opts.ca_path = NULL;
    }

    if ( NULL != opts.ca_file ) {
        delete opts.ca_file;
        opts.ca_file = NULL;
    }

    if ( NULL != opts.chipher_list ) {
        delete opts.chipher_list;
        opts.chipher_list = NULL;
    }
    
    //stcp_uninit_ssl();
    
    gpobj->logMsg( _("[TCP/IP srv listen thread] Exit.\n"), DAEMON_LOGMSG_DEBUG  );
    
    // Inform main thread that we are ending
    gpobj->m_semTcpIpThread.Post();

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void TCPListenThread::OnExit()
{
    //gpobj->logMsg( _("[TCP/IP srv listen thread] Exit.\n"), DAEMON_LOGMSG_DEBUG );
}





// ****************************************************************************
//                              Client thread
// ****************************************************************************





///////////////////////////////////////////////////////////////////////////////
// VSCPWebServerThread
//
// This thread listens for connection on a TCP socket and starts a new thread
// to handle client requests
//

TCPClientThread::TCPClientThread()
    : wxThread( wxTHREAD_DETACHED )
{
    m_pClientItem = NULL;
    m_strResponse.Empty();  // For clearness
    m_rv = 0;               // No error code
    m_bReceiveLoop = false; // Not in receive loop
    m_conn = NULL;          // No connection yet
    m_pParent = NULL;       // No parent yet
}


TCPClientThread::~TCPClientThread()
{
    m_strResponse.Empty();
    m_commandArray.Clear();
}


///////////////////////////////////////////////////////////////////////////////
// Entry
//

void *TCPClientThread::Entry()
{
    // Check pointers
    if ( NULL == gpobj ) {
        gpobj->logMsg( _("[TCP/IP srv client thread] Error, "
                         "Control object not initialized.\n"), 
                         DAEMON_LOGMSG_NORMAL );
        return NULL;
    }

    gpobj->logMsg( _("[TCP/IP srv client thread] Thread started.\n"), DAEMON_LOGMSG_DEBUG  );

    m_pClientItem = new CClientItem();
    if ( NULL == m_pClientItem ) {
        gpobj->logMsg( _("[TCP/IP srv client thread] Memory error, "
                         "Cant allocate client structure.\n"), 
                         DAEMON_LOGMSG_NORMAL );
        return NULL;
    }
    
    m_pClientItem->m_bOpen = true;
    m_pClientItem->m_type =  CLIENT_ITEM_INTERFACE_TYPE_CLIENT_TCPIP;
    m_pClientItem->m_strDeviceName = _("Remote TCP/IP Server. [");
    m_pClientItem->m_strDeviceName += gpobj->m_strTcpInterfaceAddress;
    m_pClientItem->m_strDeviceName += _("]|Started at ");
    wxDateTime now = wxDateTime::Now();
    m_pClientItem->m_strDeviceName += wxDateTime::Now().FormatISODate();
    m_pClientItem->m_strDeviceName += _(" ");
    m_pClientItem->m_strDeviceName += wxDateTime::Now().FormatISOTime();

    // Start of activity
    m_pClientItem->m_clientActivity = wxGetUTCTime();

    // Add the client to the Client List
    gpobj->m_wxClientMutex.Lock();
    if ( !gpobj->addClient( m_pClientItem ) ) {
        // Failed to add client
        delete m_pClientItem;
        m_pClientItem = NULL;
        gpobj->m_wxClientMutex.Unlock();
        gpobj->logMsg( _("TCP/IP server: Failed to add client. Terminating thread.") );
        return NULL;
    }
    gpobj->m_wxClientMutex.Unlock();

    // Clear the filter (Allow everything )
    vscp_clearVSCPFilter( &m_pClientItem->m_filterVSCP );

    // Send welcome message
    wxString str = _(MSG_WELCOME);
    str += _("Version: ");
    str += _(VSCPD_DISPLAY_VERSION);
    str += _("\r\n");
    str += _(VSCPD_COPYRIGHT);
    str += _("\r\n");
    str += _(MSG_OK);
   write(  (const char*)str.mbc_str(), str.Length() );

    gpobj->logMsg( _("[TCP/IP srv] Ready to serve client.\n"),
                         DAEMON_LOGMSG_DEBUG );
    
        
    // Enter command loop
    char buf[8192];
    struct pollfd fd;
    while ( !TestDestroy() && !(gpobj->m_StopTcpIpSrv ) ) {

        // Check for client inactivity
        if ( ( wxGetUTCTime() - m_pClientItem->m_clientActivity ) > TCPIPSRV_INACTIVITY_TIMOUT ) {
            gpobj->logMsg( _("[TCP/IP srv client thread] Client closed due to inactivity.\n") );
            break;
        }

        // * * * Receiveloop * * *
        if ( m_bReceiveLoop ) {

            // Wait for data
            m_pClientItem->m_semClientInputQueue.WaitTimeout( 20 );

            // Send everything in the queue
            while( sendOneEventFromQueue( false ) );

            // Send '+OK<CR><LF>' every two seconds to indicate that the
            // link is open
            if ( ( wxGetUTCTime()-m_pClientItem->m_timeRcvLoop ) > 2 ) {
                m_pClientItem->m_timeRcvLoop = wxGetUTCTime();
                m_pClientItem->m_clientActivity = wxGetUTCTime();
                write( "+OK\r\n", 5 );
            }
                
        }
        else {

            // Set poll
            fd.fd = m_conn->client.sock;
            fd.events = POLLIN;
            fd.revents = 0;
            
            // Wait for data
            if ( stcp_poll( &fd, 
                                1, 
                                500, 
                                &(gpobj->m_StopTcpIpSrv) ) < 0  ) {
                continue;   // Nothing                                 
            }

            // Data in?
            if ( !( fd.revents & POLLIN ) ) {
                continue;
            }
        }
        
        // Read possible data from client
        //      If in receive loop we know we have delay
        //      in event waiting above.
        memset( buf, 0, sizeof( buf ) );
        int nRead = stcp_read( m_conn, 
                                buf, sizeof( buf ),
                                ( m_bReceiveLoop ) ? 0 : 0 );
        
        if ( 0 == nRead ) {
            ;   // Nothing more to read - Check for command and continue -> below
        }
        else if ( nRead < 0 ) {

            if ( STCP_ERROR_TIMEOUT == nRead ) {
                m_rv = VSCP_ERROR_TIMEOUT;
            }
            else if ( STCP_ERROR_STOPPED == nRead ) {
                m_rv = VSCP_ERROR_STOPPED;
                continue;
            }
            break;
        }
        else if ( nRead > 0 ) {
            m_strResponse += wxString::FromUTF8Unchecked( buf, nRead );
        }

        // Record client activity
        m_pClientItem->m_clientActivity = wxGetUTCTime();
        
        // get data up to "\r\n" if any
        int pos;
        if ( wxNOT_FOUND != ( pos = m_strResponse.Find("\n"))) {
            
            // Get the command
            wxString strCommand = m_strResponse.Left( pos + 1 );

            // Save the unhandled part
            m_strResponse = m_strResponse.Right( m_strResponse.Length() - pos - 1 );

            // Remove whitespace
            strCommand.Trim(true);
            strCommand.Trim(false);

            // If nothing to do do nothing - pretty obious if you think about it
            if ( 0 == strCommand.Length() ) continue;

            // Check for repeat command
            // +    - repear last command
            // +n   - Repeat n-th command
            // ++
            if ( m_commandArray.Count() && ( '+' == strCommand[0] ) ) {

                if ( strCommand.StartsWith( "++", &strCommand ) ) {
                    for ( int i=m_commandArray.Count()-1; i>=0; i-- ) {
                        wxString str = wxString::Format("%d - %s",
                                            m_commandArray.Count() - i - 1, 
                                            m_commandArray[ i ] );
                        str.Trim();                        
                        write( str, true );
                    }
                    continue;
                }

                // Get pos
                unsigned int n = 0;
                if ( strCommand.Length() > 1 ) {
                    strCommand = strCommand.Right( strCommand.Length() - 1 );    
                    n = atoi( strCommand.mbc_str() );
                }

                // Pos must be within range
                if ( n > m_commandArray.Count() ) {
                    n = m_commandArray.Count() - 1; 
                }

                // Get the command
                strCommand = m_commandArray[ m_commandArray.Count() - n - 1 ];

                // Write out the command
                write( strCommand, true );

            }

            m_commandArray.Add( strCommand );   // put at beginning of list
            if ( m_commandArray.Count() > VSCP_TCPIP_COMMAND_LIST_MAX ) {
                m_commandArray.RemoveAt( 0 );   // Remove last inserted item
            }

            // Execute command
            if ( VSCP_TCPIP_RV_CLOSE == CommandHandler( strCommand ) ) {
                break;
            }

        }

    } // while

    // Remove the client from the client queue
    gpobj->m_mutexTcpClientList.Lock();
    TCPIPCLIENTS::iterator iter;
    for ( iter = m_pParent->m_clientList.begin(); 
            iter != m_pParent->m_clientList.end(); 
            ++iter ) {
        
        struct stcp_connection *stored_conn = *iter;
        if ( stored_conn->client.id == m_conn->client.id ) {
            m_pParent->m_clientList.erase( iter );
            break;
        }
    }
    gpobj->m_mutexTcpClientList.Unlock();
    
    // Close the connection
    stcp_close_connection( m_conn );
    m_conn = NULL;

    // Close the channel
    m_pClientItem->m_bOpen = false;
    
    // Remove the client from the Client List
    gpobj->m_wxClientMutex.Lock();
    gpobj->removeClient( m_pClientItem );
    gpobj->m_wxClientMutex.Unlock();

    m_pClientItem = NULL;
    m_pParent = NULL;
    
    gpobj->logMsg( _("[TCP/IP srv client thread] Exit.\n"), DAEMON_LOGMSG_DEBUG );

    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void TCPClientThread::OnExit()
{
    gpobj->logMsg( _("[TCP/IP srv client thread] Exit.\n"), DAEMON_LOGMSG_DEBUG );;
}

///////////////////////////////////////////////////////////////////////////////
// write
//

bool TCPClientThread::write( wxString& str, bool bAddCRLF )
{
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return false;
    
    if ( bAddCRLF ) {
        str += _("\r\n");
    }
    
    m_rv = stcp_write( m_conn, (const char*)str.mbc_str(), str.Length() );
    if ( m_rv != str.Length() ) return false;
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// write
//

bool TCPClientThread::write( const char *buf, size_t len )
{
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return false;
    
    m_rv = stcp_write( m_conn, (const char*)buf, len );
    if ( m_rv != len ) return false;
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// read
//

bool TCPClientThread::read( wxString& str )
{
    int pos;
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return false;
    
    if ( wxNOT_FOUND != ( pos = m_strResponse.First('\n') ) ) {

        // Get the string
        str = m_strResponse.Left( pos + 1 );
        str.Trim(true);
        str.Trim(false);

        // Remove string from buffer
        m_strResponse = m_strResponse.Right( m_strResponse.Length() - pos - 1 );        
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// CommandHandler
//

int
TCPClientThread::CommandHandler( wxString& strCommand )
{
    bool repeat = false;
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) {
        return VSCP_TCPIP_RV_ERROR;
    }

    if ( NULL == gpobj ) {
        gpobj->logMsg( _( "[TCP/IP srv] ERROR: Control object pointer is NULL in command handler. \n" )  );
        return VSCP_TCPIP_RV_CLOSE; // Close connection
    }

    if ( NULL == m_pClientItem ) {
        gpobj->logMsg( _( "[TCP/IP srv] ERROR: ClientItem pointer is NULL in command handler.\n" )  );
        return VSCP_TCPIP_RV_CLOSE; // Close connection
    }

    m_pClientItem->m_currentCommand = strCommand;
    m_pClientItem->m_currentCommand.Trim(true);
    m_pClientItem->m_currentCommand.Trim(false);

    // If nothing to handle just return
    if ( 0 == m_pClientItem->m_currentCommand.Length() ) {
        write( MSG_OK, strlen ( MSG_OK ) );
        return VSCP_TCPIP_RV_OK;
    }


    //*********************************************************************
    //                            No Operation
    //*********************************************************************
    
    if ( m_pClientItem->CommandStartsWith( _("noop") ) ) {        
        write(   MSG_OK, strlen ( MSG_OK ) );
        return VSCP_TCPIP_RV_OK;
    }

    //*********************************************************************
    //                             Rcvloop
    //*********************************************************************

    else if ( m_pClientItem->CommandStartsWith( _("rcvloop") ) || 
                m_pClientItem->CommandStartsWith( _("receiveloop") ) ) {
        if ( checkPrivilege( 2 ) ) {
            m_pClientItem->m_timeRcvLoop = wxGetUTCTime();
            handleClientRcvLoop();
        }
    }

    //*********************************************************************
    //                             Quitloop
    //*********************************************************************

    else if ( m_pClientItem->CommandStartsWith( _("quitloop") ) ) {
              m_bReceiveLoop = false;
        write(  MSG_QUIT_LOOP, strlen ( MSG_QUIT_LOOP ) );
    }

    //*********************************************************************
    //                             Username
    //*********************************************************************

    else if ( m_pClientItem->CommandStartsWith( _("user") ) ) {
        handleClientUser();
    }

    //*********************************************************************
    //                            Password
    //*********************************************************************

    else if ( m_pClientItem->CommandStartsWith( _("pass") ) ) {    

        if ( !handleClientPassword() ) {
            gpobj->logMsg ( _( "[TCP/IP srv] Command: Password. Not authorized.\n" ),
                                    DAEMON_LOGMSG_NORMAL,
                                    DAEMON_LOGTYPE_SECURITY );
            return VSCP_TCPIP_RV_CLOSE;    // Close connection
        }

        if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_TCP ) {
            gpobj->logMsg( _( "[TCP/IP srv] Command: Password. PASS\n" ) );
        }

    }

    //*********************************************************************
    //                              Challenge
    //*********************************************************************

    else if ( m_pClientItem->CommandStartsWith( _("challenge") ) ) {
        handleChallenge();
    }

    // *********************************************************************
    //                                 QUIT
    // *********************************************************************

    else if ( m_pClientItem->CommandStartsWith( _("quit") ) ) {
        //long test = MG_F_CLOSE_IMMEDIATELY;
        if ( gpobj->m_debugFlags1 & VSCP_DEBUG1_TCP ) {
            gpobj->logMsg( _( "[TCP/IP srv] Command: Close.\n" ) );
        }

        write(  MSG_GOODBY, strlen ( MSG_GOODBY ) );
 
        return VSCP_TCPIP_RV_CLOSE; // Close connection
    }

    //*********************************************************************
    //                              Shutdown
    //*********************************************************************
    else if ( m_pClientItem->CommandStartsWith(_("shutdown") ) ) {
        if ( checkPrivilege( 15 ) ) {
            handleClientShutdown();            
        }
    }

    //*********************************************************************
    //                             Send event
    //*********************************************************************

    else if ( m_pClientItem->CommandStartsWith( _("send") ) ) {
        if ( checkPrivilege( 4 ) ) {
            handleClientSend();
        }
    }

    //*********************************************************************
    //                            Read event
    //********************************************************************* 

    else if ( m_pClientItem->CommandStartsWith( _("retr") ) ||
                m_pClientItem->CommandStartsWith( _("retrieve") ) ) {
        if ( checkPrivilege( 2 ) ) {
            handleClientReceive();
        }
    }

    //*********************************************************************
    //                            Data Available
    //*********************************************************************

    else if ( m_pClientItem->CommandStartsWith( _("cdta") ) ||
                m_pClientItem->CommandStartsWith( _("chkdata") ) ||
                m_pClientItem->CommandStartsWith( _("checkdata") ) ) {
        if ( checkPrivilege( 1 ) ) {
            handleClientDataAvailable();
        }
    }

    //*********************************************************************
    //                          Clear input queue
    //*********************************************************************

    else if ( m_pClientItem->CommandStartsWith( _("clra") ) ||
                m_pClientItem->CommandStartsWith( _("clearall") ) ||
                m_pClientItem->CommandStartsWith( _("clrall") ) ) {
        if ( checkPrivilege( 1 ) ) {
            handleClientClearInputQueue();
        }
    }


    //*********************************************************************
    //                           Get Statistics
    //*********************************************************************

    else if ( m_pClientItem->CommandStartsWith( _("stat") ) ) {
         if ( checkPrivilege( 1 ) ) {
             handleClientGetStatistics();
         }
    }

    //*********************************************************************
    //                            Get Status
    //*********************************************************************

    else if ( m_pClientItem->CommandStartsWith( _("info") ) ) {
        if ( checkPrivilege( 1 ) ) {
            handleClientGetStatus();
        }
    }

    //*********************************************************************
    //                           Get Channel ID
    //*********************************************************************

    else if ( m_pClientItem->CommandStartsWith( _("chid") ) ||
                m_pClientItem->CommandStartsWith( _("getchid") ) ) {
        if ( checkPrivilege( 1 ) ) {
            handleClientGetChannelID();
        }
    }

    //*********************************************************************
    //                          Set Channel GUID
    //*********************************************************************

    else if ( m_pClientItem->CommandStartsWith( _("sgid") ) ||
                m_pClientItem->CommandStartsWith( _("setguid") ) ) {
        if ( checkPrivilege( 6 ) ) {
            handleClientSetChannelGUID();
        }
    }

    //*********************************************************************
    //                          Get Channel GUID
    //*********************************************************************

    else if ( m_pClientItem->CommandStartsWith( _("ggid") ) ||
                m_pClientItem->CommandStartsWith( _("getguid") ) ) {
        if ( checkPrivilege( 1 ) ) {
            handleClientGetChannelGUID();
        }
    }

    //*********************************************************************
    //                           Get Version
    //*********************************************************************

    else if ( m_pClientItem->CommandStartsWith( _("version") ) ||
                    m_pClientItem->CommandStartsWith( _("vers") ) ) {
        handleClientGetVersion();
    }

    //*********************************************************************
    //                           Set Filter
    //*********************************************************************

    else if ( m_pClientItem->CommandStartsWith( _("sflt") ) ||
                m_pClientItem->CommandStartsWith( _("setfilter") ) ) {
        if ( checkPrivilege( 6 ) ) {
            handleClientSetFilter();
        }
    }

    //*********************************************************************
    //                           Set Mask
    //*********************************************************************

    else if ( m_pClientItem->CommandStartsWith( _("smsk") ) || 
                m_pClientItem->CommandStartsWith( _("setmask") ) ) {
        if ( checkPrivilege( 6 ) ) {
            handleClientSetMask();
        }
    }    

    //*********************************************************************
    //                             Help
    //*********************************************************************

    else if ( m_pClientItem->CommandStartsWith( _("help") ) ) {
        handleClientHelp();
    }

    //*********************************************************************
    //                             Restart

    else if ( m_pClientItem->CommandStartsWith( _("restart") ) ) {
        if ( checkPrivilege( 15 ) ) {
            handleClientRestart();
        }
    }

    //*********************************************************************
    //                             Driver
    //*********************************************************************

    else if ( m_pClientItem->CommandStartsWith( _("driver") ) ) {
        if ( checkPrivilege( 15 ) ) {
            handleClientDriver();
        }
    }

    //*********************************************************************
    //                               DM
    //*********************************************************************

    else if ( m_pClientItem->CommandStartsWith( _("dm") ) ) {
        if ( checkPrivilege( 15 ) ) {
            handleClientDm();
        }
    }

    //*********************************************************************
    //                             Variable
    //*********************************************************************

    else if ( m_pClientItem->CommandStartsWith( _("variable") ) ||
                m_pClientItem->CommandStartsWith( _("var") ) ) {
        if ( checkPrivilege( 4 ) ) {
            handleClientVariable();
        }
    }

    //*********************************************************************
    //                               File
    //*********************************************************************

    else if ( m_pClientItem->CommandStartsWith( _("file") ) ) {
        if ( checkPrivilege( 15 ) ) {
            handleClientFile();
        }
    }

    //*********************************************************************
    //                               UDP
    //*********************************************************************

    else if ( m_pClientItem->CommandStartsWith( _("udp") ) ) {
        if ( checkPrivilege( 15 ) ) {
            handleClientUdp();
        }
    }

    //*********************************************************************
    //                         Client/interface
    //*********************************************************************

    else if ( m_pClientItem->CommandStartsWith( _("client") ) ||
                m_pClientItem->CommandStartsWith( _("interface") ) ) {
        if ( checkPrivilege( 15 ) ) {
            handleClientInterface();
        }
    }


    //*********************************************************************
    //                               Test
    //*********************************************************************

    else if ( m_pClientItem->CommandStartsWith( _("test") ) ) {
        if ( checkPrivilege( 15 ) ) {
            handleClientTest();
        }
    }


    //*********************************************************************
    //                             WhatCanYouDo
    //*********************************************************************
    
    else if ( m_pClientItem->CommandStartsWith( _("wcyd") ) ||
                m_pClientItem->CommandStartsWith( _("whatcanyoudo") ) ) {
        handleClientCapabilityRequest();
    }

    //*********************************************************************
    //                             Measurement
    //*********************************************************************
    
    else if ( m_pClientItem->CommandStartsWith( _("measurement") ) ) {
        handleClientMeasurment();
    }

    //*********************************************************************
    //                                Table
    //*********************************************************************
    
    else if ( m_pClientItem->CommandStartsWith( _("table") ) ) {
        handleClientTable();
    }

    //*********************************************************************
    //                                What?
    //*********************************************************************
    else {
        write(   MSG_UNKNOWN_COMMAND, strlen ( MSG_UNKNOWN_COMMAND ) );
    }

    m_pClientItem->m_lastCommand = m_pClientItem->m_currentCommand;
    return VSCP_TCPIP_RV_OK;

} // clientcommand


///////////////////////////////////////////////////////////////////////////////
// handleClientMeasurment
//
// format,level,vscp-measurement-type,value,unit,guid,sensoridx,zone,subzone,dest-guid
//
// format                   float|string|0|1 - float=0, string=1.
// level                    1|0  1 = VSCP Level I event, 2 = VSCP Level II event.
// vscp-measurement-type    A valid vscp measurement type.
// value                    A floating point value. (use $ prefix for variable followed by name)
// unit                     Optional unit for this type. Default = 0.
// guid                     Optional GUID (or "-"). Default is "-".
// sensoridx                Optional sensor index. Default is 0.
// zone                     Optional zone. Default is 0.
// subzone                  Optional subzone- Default is 0.
// dest-guid                Optional destination GUID. For Level I over Level II.
//

void TCPClientThread::handleClientMeasurment( void )
{
    wxString wxstr;
    unsigned long l;
    double value = 0;
    cguid guid;             // Initialized to zero
    cguid destguid;         // Initialized to zero
    long level = VSCP_LEVEL2;
    long unit = 0;
    long vscptype;
    long sensoridx = 0;
    long zone = 0;
    long subzone = 0;
    long eventFormat = 0;   // float
    uint8_t data[ VSCP_MAX_DATA ];
    uint16_t sizeData;
    
    // Check object pointer
    if ( NULL == gpobj ) {
        write( MSG_INTERNAL_MEMORY_ERROR, strlen ( MSG_INTERNAL_MEMORY_ERROR ) );
        return;
    }
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;
    
    if ( NULL == m_pClientItem ) {
        write( MSG_INTERNAL_MEMORY_ERROR, strlen( MSG_INTERNAL_MEMORY_ERROR ) );
        return;
    }
    
    wxStringTokenizer tkz( m_pClientItem->m_currentCommand, _(",") );

    // If first character is $ user request us to send content from
    // a variable

    // * * * event format * * *
    
    // Get event format (float | string | 0 | 1 - float=0, string=1.)
    if ( !tkz.HasMoreTokens() ) {
        write(   MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    wxstr = tkz.GetNextToken();
    wxstr.MakeUpper();
    if ( wxstr.IsNumber() ) {
        
        l = 0;
  
        if ( wxstr.ToULong( &l ) && ( l > 1 ) ) {
            write( MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            return;
        }
        
        eventFormat = l;
        
    }
    else {
        if ( wxNOT_FOUND != wxstr.Find("STRING") ) {
            eventFormat = 1;
        }
        else if ( wxNOT_FOUND != wxstr.Find("FLOAT") ) {
            eventFormat = 0;
        }
        else {
            write(   MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            return;
        }
    }
    
    // * * * Level * * *
    
    if ( !tkz.HasMoreTokens() ) {
        write(   MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    wxstr = tkz.GetNextToken();
    
    if ( wxstr.IsNumber() ) {
        
        l = VSCP_LEVEL1; 
  
        if ( wxstr.ToULong( &l ) && ( l > VSCP_LEVEL2 ) ) {
            write(   MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            return;
        }
        
        level = l;
        
    }
    else {
        write(   MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // * * * vscp-measurement-type * * *
    
    if ( !tkz.HasMoreTokens() ) {
        write(   MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    wxstr = tkz.GetNextToken();
    
    if ( wxstr.IsNumber() ) {
        
        l = 0; 
  
        if ( wxstr.ToULong( &l ) ) {
            write(   MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            return;
        }
        
        vscptype = l;
        
    }
    else {
        write(   MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
        return;
    }
    

    // * * * value * * *
    
        
    if ( !tkz.HasMoreTokens() ) {
        write(   MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    wxstr = tkz.GetNextToken();
    wxstr.Trim(true);
    wxstr.Trim(false);
    
    // If first character is '$' user request us to send content from
    // a variable (that must be numeric)
    if ( '$' == wxstr[0] ) {
        
        CVSCPVariable variable;
        wxstr = wxstr.Right( wxstr.Length() - 1 );  // get variable name

        wxstr.MakeUpper();

        if ( gpobj->m_variables.find( wxstr, variable  ) ) {
            write(  MSG_VARIABLE_NOT_DEFINED, strlen ( MSG_VARIABLE_NOT_DEFINED ) );
            return;
        }
        
        // get the value
        wxstr = variable.getValue();
        if ( !wxstr.IsNumber() ) {
            write(  MSG_VARIABLE_NOT_NUMERIC, strlen ( MSG_VARIABLE_NOT_NUMERIC ) );
            return;
        }
        
        if ( wxstr.ToCDouble( &value ) ) {
            write(   MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            return;
        }
        
    }
    else {
        if ( !wxstr.ToCDouble( &value ) ) {
            write(   MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            return;
        }
    }
    
    
    // * * * unit * * *
    
    
    if ( tkz.HasMoreTokens() ) {
        wxstr = tkz.GetNextToken();
        if ( wxstr.IsNumber() ) {
            if ( wxstr.ToULong( &l ) ) {
                unit = l;
            }
        }
    }
    
    
    // * * * guid * * *
    
       
    if ( tkz.HasMoreTokens() ) {
        
        wxstr = tkz.GetNextToken();
        
        wxstr.Trim(true);
        wxstr.Trim(false);
        // If empty set to default.
        if ( 0 == wxstr.Length() ) wxstr = _("-");
        guid.getFromString( wxstr );
    }
    
    
    // * * * sensor index * * *
    
    
    if ( tkz.HasMoreTokens() ) {
        
        wxstr = tkz.GetNextToken();
        
        wxstr.Trim(true);
        wxstr.Trim(false);
        
        if ( wxstr.IsNumber() ) { 
            if ( wxstr.ToULong( &l ) ) {
                sensoridx = l;
            }
        }
    }
    
    
    // * * * zone * * *
    
    
    if ( tkz.HasMoreTokens() ) {
        
        wxstr = tkz.GetNextToken();
        
        wxstr.Trim(true);
        wxstr.Trim(false);
        
        if ( wxstr.IsNumber() ) { 
            if ( wxstr.ToULong( &l ) ) {
                zone = l;
                zone &= 0xff;
            }
        }
    }
    
    
    // * * * subzone * * *
    
    
    if ( tkz.HasMoreTokens() ) {
        
        wxstr = tkz.GetNextToken();
        
        wxstr.Trim(true);
        wxstr.Trim(false);
        
        if ( wxstr.IsNumber() ) { 
            if ( wxstr.ToULong( &l ) ) {
                subzone = l;
                subzone &= 0xff;
            }
        }
    }
    
    
    // * * * destguid * * *
    
        
    if ( tkz.HasMoreTokens() ) {
        
        wxstr = tkz.GetNextToken();
        
        wxstr.Trim(true);
        wxstr.Trim(false);
        
        // If empty set to default.
        if ( 0 == wxstr.Length() ) wxstr = _("-");
        destguid.getFromString( wxstr );
    }
    
    // Range checks
    if ( VSCP_LEVEL1 == level ) {
        if (unit > 3) unit = 0;
        if (sensoridx > 7) unit = 0;
        if (vscptype > 512) vscptype -= 512;
    } 
    else {  // VSCP_LEVEL2
        if (unit > 255) unit &= 0xff;
        if (sensoridx > 255) sensoridx &= 0xff;
    }
    
    if ( 1 == level ) {

        if ( 0 == eventFormat ) {

            // * * * Floating point * * *
            
            if ( vscp_convertFloatToFloatEventData( data,
                                                        &sizeData,
                                                        value,
                                                        unit,
                                                        sensoridx ) ) {
                if ( sizeData > 8 ) sizeData = 8;

                vscpEvent *pEvent = new vscpEvent;
                if ( NULL == pEvent ) {
                    write(   MSG_INTERNAL_MEMORY_ERROR, strlen ( MSG_INTERNAL_MEMORY_ERROR ) );
                    return;
                }
                
                pEvent->pdata = NULL;
                pEvent->head = VSCP_PRIORITY_NORMAL;
                pEvent->timestamp = 0;  // Let interface fill in 
                                        // Will fill in date/time block also
                guid.writeGUID( pEvent->GUID );
                pEvent->sizeData = sizeData;
                if ( sizeData > 0 ) {
                    pEvent->pdata = new uint8_t[ sizeData ];
                    memcpy( pEvent->pdata, data, sizeData );
                }
                pEvent->vscp_class = VSCP_CLASS1_MEASUREMENT;
                pEvent->vscp_type = vscptype;

                // send the event
                if ( !gpobj->sendEvent( m_pClientItem, pEvent ) ) {
                    write(   MSG_UNABLE_TO_SEND_EVENT, strlen ( MSG_UNABLE_TO_SEND_EVENT ) );
                    return;
                }

            } 
            else {
                write(   MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            }
        } 
        else {
            
            // * * * String * * *
            
            vscpEvent *pEvent = new vscpEvent;
            if (NULL == pEvent) {
                write(   MSG_INTERNAL_MEMORY_ERROR, strlen ( MSG_INTERNAL_MEMORY_ERROR ) );
                return;
            }
            
            pEvent->pdata = NULL;

            if ( !vscp_makeStringMeasurementEvent( pEvent,
                                                    value,
                                                    unit,
                                                    sensoridx ) ) {
                write(   MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            }
            
        }
    } 
    else {      // Level II
        
        if ( 0 == eventFormat ) {   // float and Level II

            // * * * Floating point * * *
            
            vscpEvent *pEvent = new vscpEvent;
            if ( NULL == pEvent ) {
                write(   MSG_INTERNAL_MEMORY_ERROR, strlen ( MSG_INTERNAL_MEMORY_ERROR ) );
                return;
            }

            pEvent->pdata = NULL;

            pEvent->obid = 0;
            pEvent->head = VSCP_PRIORITY_NORMAL;
            pEvent->timestamp = 0;  // Let interface fill in timestamp
                                    // Will fill in date/time block also
            guid.writeGUID( pEvent->GUID );
            pEvent->head = 0;
            pEvent->vscp_class = VSCP_CLASS2_MEASUREMENT_FLOAT;
            pEvent->vscp_type = vscptype;
            pEvent->sizeData = 12;

            data[ 0 ] = sensoridx;
            data[ 1 ] = zone;
            data[ 2 ] = subzone;
            data[ 3 ] = unit;

            memcpy(data + 4, (uint8_t *) & value, 8); // copy in double
            uint64_t temp = wxUINT64_SWAP_ON_LE(*(data + 4));
            memcpy(data + 4, (void *) &temp, 8);

            // Copy in data
            pEvent->pdata = new uint8_t[ 4 + 8 ];
            if (NULL == pEvent->pdata) {
                write(   MSG_INTERNAL_MEMORY_ERROR, strlen ( MSG_INTERNAL_MEMORY_ERROR ) );
                delete pEvent;
                return;
            }
            
            memcpy(pEvent->pdata, data, 4 + 8);

            // send the event
            if ( !gpobj->sendEvent( m_pClientItem, pEvent ) ) {
                write(   MSG_UNABLE_TO_SEND_EVENT, strlen ( MSG_UNABLE_TO_SEND_EVENT ) );
                return;
            }
            
        } 
        else {      // string & Level II
            
            // * * * String * * *
            
            vscpEvent *pEvent = new vscpEvent;
            pEvent->pdata = NULL;

            pEvent->obid = 0;
            pEvent->head = VSCP_PRIORITY_NORMAL;
            pEvent->timestamp = 0;      // Let interface fill in
                                        // Will fill in date/time block also
            guid.writeGUID( pEvent->GUID );
            pEvent->head = 0;
            pEvent->vscp_class = VSCP_CLASS2_MEASUREMENT_STR;
            pEvent->vscp_type = vscptype;
            pEvent->sizeData = 12;
            
            wxString strValue = wxString::Format(_("%f"), value );

            data[ 0 ] = sensoridx;
            data[ 1 ] = zone;
            data[ 2 ] = subzone;
            data[ 3 ] = unit;

            pEvent->pdata = new uint8_t[ 4 + strValue.Length() ];
            if (NULL == pEvent->pdata) {
                write(   MSG_INTERNAL_MEMORY_ERROR, strlen ( MSG_INTERNAL_MEMORY_ERROR ) );
                delete pEvent;
                return;
            }
            memcpy(data + 4, strValue.mbc_str(), strValue.Length()); // copy in double

            // send the event
            if ( !gpobj->sendEvent( m_pClientItem, pEvent ) ) {
                write(   MSG_UNABLE_TO_SEND_EVENT, strlen ( MSG_UNABLE_TO_SEND_EVENT ) );
                return;
            }
            
        }
    }
   
    write(  MSG_OK, strlen ( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleClientCapabilityRequest
//

void TCPClientThread::handleClientCapabilityRequest( void )
{
    wxString wxstr;
    uint8_t capabilities[16];
    
    gpobj->getVscpCapabilities( capabilities );
    wxstr = wxString::Format(_("%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X"),                                
                                capabilities[7], capabilities[6], capabilities[5], capabilities[4],
                                capabilities[3], capabilities[2], capabilities[1], capabilities[0] );
    write(   MSG_UNKNOWN_COMMAND, strlen ( MSG_UNKNOWN_COMMAND ) );
}


///////////////////////////////////////////////////////////////////////////////
// isVerified
//

bool TCPClientThread::isVerified( void )
{
    // Check object
    if ( NULL == gpobj ) {
        return false;
    }
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return false;

    // Must be accredited to do this
    if ( !m_pClientItem->bAuthenticated ) {
        write(  MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// checkPrivilege
//

bool TCPClientThread::checkPrivilege( unsigned char reqiredPrivilege )
{
    // Check objects
    if ( NULL == gpobj ) {
        return false;
    }
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return false;

    // Must be authenticated
    if ( !m_pClientItem->bAuthenticated ) {
        write( MSG_NOT_ACCREDITED, strlen( MSG_NOT_ACCREDITED ) );
        return false;
    }

    if ( NULL == m_pClientItem->m_pUserItem ) {
        write(   MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
        return false;
    }

    if ( (m_pClientItem->m_pUserItem->getUserRights( 0 ) & USER_PRIVILEGE_MASK ) < reqiredPrivilege ) {
        write(   MSG_LOW_PRIVILEGE_ERROR, strlen ( MSG_LOW_PRIVILEGE_ERROR ) );
        return false;
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientSend
//

void TCPClientThread::handleClientSend( void )
{
    bool bSent = false;
    bool bVariable = false;
    vscpEvent event;
    wxString nameVariable;
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;
    
    // Set timestamp block for event
    vscp_setEventDateTimeBlockToNow( &event );

    if ( NULL == gpobj ) {
        write(  MSG_PARAMETER_ERROR, strlen( MSG_PARAMETER_ERROR ) );
        return;
    }

    if ( NULL == m_pClientItem ) {
        write(  MSG_INTERNAL_ERROR, strlen( MSG_INTERNAL_ERROR ) );
        return;
    }

    // Must be accredited to do this
    if ( !m_pClientItem->bAuthenticated ) {
        write(  MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    wxString str;
    wxStringTokenizer tkz( m_pClientItem->m_currentCommand, _(",") );

    // If first character is $ user request us to send content from
    // a variable

    if ( tkz.HasMoreTokens() ) {
        
        str = tkz.GetNextToken();
        
        str.Trim(true);
        str.Trim(false);
        
        if ( wxNOT_FOUND == str.Find(_("$") ) ) {
            event.head = vscp_readStringValue( str );
        }
        else {
            
            CVSCPVariable variable;
            bVariable = true;   // Yes this is a variable send

            // Get the name of the variable
            nameVariable = str.Right( str.Length() - 1 );
            nameVariable.MakeUpper();

            if ( gpobj->m_variables.find( nameVariable, variable  ) ) {
                write(  MSG_VARIABLE_NOT_DEFINED, strlen ( MSG_VARIABLE_NOT_DEFINED ) );
                return;
            }

            // Must be event type
            if ( VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT != variable.getType() ) {
                write(  MSG_VARIABLE_MUST_BE_EVENT_TYPE, 
                          strlen ( MSG_VARIABLE_MUST_BE_EVENT_TYPE ) );
                return;
            }

            // Get the event
            variable.getValue( &event );

        }
    }
    else {
        write(  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
        return;
    }

    if ( !bVariable ) {
        
        // Not a variable

        // Get Class
        if ( tkz.HasMoreTokens() ) {
            str = tkz.GetNextToken();
            event.vscp_class = vscp_readStringValue( str );
        }
        else {
            write(   MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            return;
        }

        // Get Type
        if ( tkz.HasMoreTokens() ) {
            str = tkz.GetNextToken();
            event.vscp_type = vscp_readStringValue( str );
        }
        else {
            write(   MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            return;
        }

        // Get OBID  -  Kept here to be compatible with receive
        if ( tkz.HasMoreTokens() ) {
            str = tkz.GetNextToken();
            event.obid = vscp_readStringValue( str );
        }
        else {
            write(   MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            return;
        }
        
        // Get date/time - can be empty
        if ( tkz.HasMoreTokens() ) {
            str = tkz.GetNextToken();
            str.Trim();
            if ( str.Length() ) {
                wxDateTime dt;
                if ( dt.ParseISOCombined( str ) ) {
                    event.year = dt.GetYear();
                    event.month = dt.GetMonth();
                    event.day = dt.GetDay();
                    event.hour = dt.GetHour();
                    event.minute = dt.GetMinute();
                    event.second = dt.GetSecond();
                }
                else {
                    vscp_setEventDateTimeBlockToNow( &event );
                }
            }
            else {
                // set current time
                vscp_setEventDateTimeBlockToNow( &event );
            }

        }
        else {
            write(   MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            return;
        }

        // Get Timestamp - can be empty
        if ( tkz.HasMoreTokens() ) {
            str = tkz.GetNextToken();
            str.Trim();
            if ( str.Length() ) {
                event.timestamp = vscp_readStringValue( str );
            }
            else {
                event.timestamp = vscp_makeTimeStamp();
            }
        }
        else {
            write(   MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            return;
        }

        // Get GUID
        wxString strGUID;
        if ( tkz.HasMoreTokens() ) {
            strGUID = tkz.GetNextToken();
            
            // Check if i/f GUID should be used
            if ( '-' == strGUID[0] ) {
                // Copy in the i/f GUID
                m_pClientItem->m_guid.writeGUID( event.GUID );
            }
            else {
                vscp_getGuidFromString( &event, strGUID );
                
                // Check if i/f GUID should be used
                if ( true == vscp_isGUIDEmpty( event.GUID ) ) {
                    // Copy in the i/f GUID
                    m_pClientItem->m_guid.writeGUID( event.GUID );
                }
            }
        }
        else {
            write(   MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            return;
        }

        // Handle data
        if ( 512 < tkz.CountTokens() ) {
            write(  MSG_PARAMETER_ERROR, strlen( MSG_PARAMETER_ERROR ) );
            return;
        }

        event.sizeData = tkz.CountTokens();

        if ( event.sizeData > 0 ) {

            unsigned int index = 0;

            event.pdata = new uint8_t[ event.sizeData ];

            if ( NULL == event.pdata ) {
                write(  MSG_INTERNAL_MEMORY_ERROR, strlen( MSG_INTERNAL_MEMORY_ERROR ) );
                return;
            }

            while ( tkz.HasMoreTokens() && ( event.sizeData > index ) ) {
                str = tkz.GetNextToken();
                event.pdata[ index++ ] = vscp_readStringValue( str );
            }

            if ( tkz.HasMoreTokens() ) {
                write(  MSG_PARAMETER_ERROR, strlen( MSG_PARAMETER_ERROR ) );

                delete [] event.pdata;
                event.pdata = NULL;
                return;
            }
        }
        else {
            // No data
            event.pdata = NULL;
        }

    } // not variable send

    // Check if this user is allowed to send this event
    if ( !m_pClientItem->m_pUserItem->isUserAllowedToSendEvent( event.vscp_class, event.vscp_type ) ) {
        wxString strErr =
                        wxString::Format( _("[TCP/IP srv] User [%s] not allowed to send event class=%d type=%d.\n"),
                                                (const char *)m_pClientItem->m_pUserItem->getUserName().mbc_str(),
                                                event.vscp_class, event.vscp_type );

        gpobj->logMsg ( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_SECURITY );

        write(  MSG_MOT_ALLOWED_TO_SEND_EVENT, strlen ( MSG_MOT_ALLOWED_TO_SEND_EVENT ) );

        if ( NULL != event.pdata ) {
            delete [] event.pdata;
            event.pdata = NULL;
        }

        return;
    }
    
    // send event
    if ( !gpobj->sendEvent( m_pClientItem, &event ) ) {
        write(   MSG_BUFFER_FULL, strlen ( MSG_BUFFER_FULL ) );
        return;
    }
        
    write(  MSG_OK, strlen ( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleClientReceive
//

void TCPClientThread::handleClientReceive( void )
{
    unsigned short cnt = 0;	// # of messages to read
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    // Must be accredited to do this
    if ( !m_pClientItem->bAuthenticated ) {
        write(   MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    wxString str;
    cnt = vscp_readStringValue( m_pClientItem->m_currentCommand );

    if ( !cnt ) cnt = 1;	// No arg is "read one"


    // Read cnt messages
    while ( cnt ) {

        wxString strOut;

        if ( !m_pClientItem->m_bOpen ) {
            write(   MSG_NO_MSG, strlen ( MSG_NO_MSG ) );
            return;
        }
        else {
            if ( false == sendOneEventFromQueue() ) {
                return;
            }
        }

        cnt--;

    } // while

    write(   MSG_OK, strlen ( MSG_OK ) );

}

///////////////////////////////////////////////////////////////////////////////
// sendOneEventFromQueue
//

bool TCPClientThread::sendOneEventFromQueue( bool bStatusMsg )
{
    wxString strOut;
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return false;

    CLIENTEVENTLIST::compatibility_iterator nodeClient;

    if ( m_pClientItem->m_clientInputQueue.GetCount() ) {

        vscpEvent *pqueueEvent;
        m_pClientItem->m_mutexClientInputQueue.Lock();
        {
            nodeClient = m_pClientItem->m_clientInputQueue.GetFirst();
            pqueueEvent = nodeClient->GetData();

            // Remove the node
            m_pClientItem->m_clientInputQueue.DeleteNode ( nodeClient );
        }
        m_pClientItem->m_mutexClientInputQueue.Unlock();

        vscp_writeVscpEventToString( pqueueEvent, strOut );
        strOut += _("\r\n");
        write(   strOut.mb_str(), strlen ( strOut.mb_str() ) );

        //delete pqueueEvent;
        vscp_deleteVSCPevent( pqueueEvent );

        // Let the system work a little
        //ns_mgr_poll( &gpobj->m_mgrTcpIpServer, 1 );
    }
    else {
        if ( bStatusMsg ) {
            write(   MSG_NO_MSG, strlen ( MSG_NO_MSG ) );
        }

        return false;

    }

    return true;

}



///////////////////////////////////////////////////////////////////////////////
// handleClientDataAvailable
//

void TCPClientThread::handleClientDataAvailable( void )
{
    char outbuf[ 1024 ];
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    // Must be accredited to do this
    if ( !m_pClientItem->bAuthenticated ) {
        write(   MSG_NOT_ACCREDITED,
            strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    sprintf ( outbuf,
        "%zd\r\n%s\r\n",
        m_pClientItem->m_clientInputQueue.GetCount(),
        MSG_OK );
    write(   outbuf, strlen ( outbuf ) );


}

///////////////////////////////////////////////////////////////////////////////
// handleClientClearInputQueue
//

void TCPClientThread::handleClientClearInputQueue( void )
{
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    // Must be accredited to do this
    if ( !m_pClientItem->bAuthenticated ) {
        write(   MSG_NOT_ACCREDITED,
            strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    m_pClientItem->m_mutexClientInputQueue.Lock();
    m_pClientItem->m_clientInputQueue.Clear();    
    m_pClientItem->m_mutexClientInputQueue.Unlock();

    write(   MSG_QUEUE_CLEARED, strlen ( MSG_QUEUE_CLEARED ) );
}


///////////////////////////////////////////////////////////////////////////////
// handleClientGetStatistics
//

void TCPClientThread::handleClientGetStatistics( void )
{
    char outbuf[ 1024 ];
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    // Must be accredited to do this
    if ( !m_pClientItem->bAuthenticated ) {
        write(   MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    sprintf ( outbuf, "%lu,%lu,%lu,%lu,%lu,%lu,%lu\r\n%s\r\n",
        m_pClientItem->m_statistics.cntBusOff,
        m_pClientItem->m_statistics.cntBusWarnings,
        m_pClientItem->m_statistics.cntOverruns,
        m_pClientItem->m_statistics.cntReceiveData,
        m_pClientItem->m_statistics.cntReceiveFrames,
        m_pClientItem->m_statistics.cntTransmitData,
        m_pClientItem->m_statistics.cntTransmitFrames,
        MSG_OK );

    write( outbuf, strlen ( outbuf ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleClientGetStatus
//

void TCPClientThread::handleClientGetStatus( void )
{
    char outbuf[ 1024 ];
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    // Must be accredited to do this
    if ( !m_pClientItem->bAuthenticated ) {
        write(   MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    sprintf( outbuf, "%lu,%lu,%lu,\"%s\"\r\n%s\r\n",
                m_pClientItem->m_status.channel_status,
                m_pClientItem->m_status.lasterrorcode,
                m_pClientItem->m_status.lasterrorsubcode,
                m_pClientItem->m_status.lasterrorstr,
                MSG_OK );

    write( outbuf, strlen( outbuf ) );


}

///////////////////////////////////////////////////////////////////////////////
// handleClientGetChannelID
//

void TCPClientThread::handleClientGetChannelID( void )
{
    char outbuf[ 1024 ];
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    // Must be accredited to do this
    if ( !m_pClientItem->bAuthenticated ) {
        write(   MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    sprintf( outbuf, "%lu\r\n%s",
            (unsigned long)m_pClientItem->m_clientID, MSG_OK );

    write(   outbuf, strlen ( outbuf ) );

}


///////////////////////////////////////////////////////////////////////////////
// handleClientSetChannelGUID
//

void TCPClientThread::handleClientSetChannelGUID( void )
{
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    // Must be accredited to do this
    if ( !m_pClientItem->bAuthenticated ) {
        write(   MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    m_pClientItem->m_currentCommand.Trim(true);
    m_pClientItem->m_currentCommand.Trim(false);
    
    m_pClientItem->m_guid.getFromString( m_pClientItem->m_currentCommand );
    write( MSG_OK, strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleClientGetChannelGUID
//

void TCPClientThread::handleClientGetChannelGUID( void )
{
    wxString strBuf;
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    // Must be accredited to do this
    if ( !m_pClientItem->bAuthenticated ) {
        write(  MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }


    m_pClientItem->m_guid.toString( strBuf );
    strBuf += _("\r\n");
    strBuf += _(MSG_OK);

    write( strBuf );
}

///////////////////////////////////////////////////////////////////////////////
// handleClientGetVersion
//

void TCPClientThread::handleClientGetVersion( void )
{
    char outbuf[ 1024 ];

    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    // Must be accredited to do this
    if ( !m_pClientItem->bAuthenticated ) {
        write( MSG_NOT_ACCREDITED, strlen( MSG_NOT_ACCREDITED ) );
        return;
    }


    sprintf ( outbuf,
                "%d,%d,%d,%d\r\n%s\r\n",
                VSCPD_MAJOR_VERSION,
                VSCPD_MINOR_VERSION,
                VSCPD_RELEASE_VERSION,
                VSCPD_BUILD_VERSION,
                MSG_OK );

    write( outbuf, strlen( outbuf ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleClientSetFilter
//

void TCPClientThread::handleClientSetFilter( void )
{
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    // Must be accredited to do this
    if ( !m_pClientItem->bAuthenticated ) {
        write( MSG_NOT_ACCREDITED, strlen( MSG_NOT_ACCREDITED ) );
        return;
    }

    wxString str;
    m_pClientItem->m_currentCommand.Trim(true);
    m_pClientItem->m_currentCommand.Trim(false);
    wxStringTokenizer tkz( m_pClientItem->m_currentCommand, _(",") );

    // Get priority
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        m_pClientItem->m_filterVSCP.filter_priority = vscp_readStringValue( str );
    }
    else {
        write( MSG_PARAMETER_ERROR, strlen( MSG_PARAMETER_ERROR ) );
        return;
    }

    // Get Class
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        m_pClientItem->m_filterVSCP.filter_class = vscp_readStringValue( str );
    }
    else {
        write( MSG_PARAMETER_ERROR, strlen( MSG_PARAMETER_ERROR ) );
        return;
    }


    // Get Type
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        m_pClientItem->m_filterVSCP.filter_type = vscp_readStringValue( str );
    }
    else {
        write( MSG_PARAMETER_ERROR, strlen( MSG_PARAMETER_ERROR ) );
        return;
    }

    // Get GUID
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        vscp_getGuidFromStringToArray( m_pClientItem->m_filterVSCP.filter_GUID, str );
    }
    else {
        write( MSG_PARAMETER_ERROR, strlen( MSG_PARAMETER_ERROR ) );
        return;
    }

    write( MSG_OK, strlen ( MSG_OK ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleClientSetMask
//

void TCPClientThread::handleClientSetMask( void )
{
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    // Must be accredited to do this
    if ( !m_pClientItem->bAuthenticated ) {
        write( MSG_NOT_ACCREDITED, strlen( MSG_NOT_ACCREDITED ) );
        return;
    }

    wxString str;
    m_pClientItem->m_currentCommand.Trim(true);
    m_pClientItem->m_currentCommand.Trim(false);
    wxStringTokenizer tkz( m_pClientItem->m_currentCommand, _(",") );

    // Get priority
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        m_pClientItem->m_filterVSCP.mask_priority = vscp_readStringValue( str );
    }
    else {
        write( MSG_PARAMETER_ERROR, strlen( MSG_PARAMETER_ERROR ) );
        return;
    }

    // Get Class
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        m_pClientItem->m_filterVSCP.mask_class = vscp_readStringValue( str );
    }
    else {
        write( MSG_PARAMETER_ERROR, strlen( MSG_PARAMETER_ERROR ) );
        return;
    }


    // Get Type
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        m_pClientItem->m_filterVSCP.mask_type = vscp_readStringValue( str );
    }
    else {
        write( MSG_PARAMETER_ERROR, strlen( MSG_PARAMETER_ERROR ) );
        return;
    }

    // Get GUID
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        vscp_getGuidFromStringToArray( m_pClientItem->m_filterVSCP.mask_GUID, str );
    }
    else {
        write( MSG_PARAMETER_ERROR, strlen( MSG_PARAMETER_ERROR ) );
        return;
    }

    write( MSG_OK, strlen( MSG_OK ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleClientUser
//

void TCPClientThread::handleClientUser( void )
{
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    if ( m_pClientItem->bAuthenticated ) {
        write(   MSG_OK, strlen ( MSG_OK ) );
        return;
    }

    m_pClientItem->m_UserName = m_pClientItem->m_currentCommand;
    m_pClientItem->m_UserName.Trim(true);     // Trim right side
    m_pClientItem->m_UserName.Trim(false );   // Trim left
    if ( m_pClientItem->m_UserName.IsEmpty() ) {
        write( MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
        return;
    }

    write( MSG_USENAME_OK, strlen ( MSG_USENAME_OK ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleClientPassword
//

bool TCPClientThread::handleClientPassword( void )
{
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return false;

    if ( m_pClientItem->bAuthenticated ) {
        write( MSG_OK, strlen ( MSG_OK ) );
        return true;
    }

    // Must have username before password can be entered.
    if ( 0 == m_pClientItem->m_UserName.Length() ) {
        write( MSG_NEED_USERNAME, strlen ( MSG_NEED_USERNAME ) );
        return true;
    }

    wxString strPassword = m_pClientItem->m_currentCommand;
    strPassword.Trim(true);         // Trim right side
    strPassword.Trim(false);        // Trim left

    if ( strPassword.IsEmpty() ) {
        m_pClientItem->m_UserName = _("");
        write(   MSG_PARAMETER_ERROR, strlen( MSG_PARAMETER_ERROR ) );
        return false;
    }

    gpobj->m_mutexUserList.Lock();
#if  0
    ::wxLogDebug( _("Username: ") + m_UserName );
    ::wxLogDebug( _("Password: ") + strPassword );
    ::wxLogDebug( _("MD5 of Password: ") + md5Password );
#endif
    m_pClientItem->m_pUserItem = 
            gpobj->m_userList.validateUser( m_pClientItem->m_UserName, 
                                            strPassword );
    gpobj->m_mutexUserList.Unlock();

    if ( NULL == m_pClientItem->m_pUserItem ) {

        wxString strErr =
            wxString::Format(_("[TCP/IP srv] User [%s][%s] not allowed to connect.\n"),
            (const char *)m_pClientItem->m_UserName.mbc_str(), 
            (const char *)strPassword.mbc_str() );

        gpobj->logMsg( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_SECURITY );
        write( MSG_PASSWORD_ERROR, strlen ( MSG_PASSWORD_ERROR ) );
        return false;
    }

    // Get remote address
    struct sockaddr_in cli_addr;
    socklen_t clilen = 0;
    clilen = sizeof (cli_addr);
    (void)getpeername( m_conn->client.sock, (struct sockaddr *)&cli_addr, &clilen);
    wxString remoteaddr = wxString::FromAscii( inet_ntoa( cli_addr.sin_addr ) );

    // Check if this user is allowed to connect from this location
    gpobj->m_mutexUserList.Lock();
    bool bValidHost =
        ( 1 == m_pClientItem->m_pUserItem->isAllowedToConnect( cli_addr.sin_addr.s_addr ) );
    gpobj->m_mutexUserList.Unlock();

    if ( !bValidHost ) {
        wxString strErr = wxString::Format(_("[TCP/IP srv] Host [%s] not allowed to connect.\n"),
            (const char *)remoteaddr.c_str() );

        gpobj->logMsg ( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_SECURITY );
        write(   MSG_INVALID_REMOTE_ERROR, strlen ( MSG_INVALID_REMOTE_ERROR ) );
        return false;
    }

    // Copy in the user filter
    memcpy( &m_pClientItem->m_filterVSCP,
                m_pClientItem->m_pUserItem->getFilter(),
                sizeof( vscpEventFilter ) );

    wxString strErr =
        wxString::Format( _("[TCP/IP srv] Host [%s] User [%s] allowed to connect.\n"),
                            (const char *)remoteaddr.c_str(),
                            (const char *)m_pClientItem->m_UserName.c_str() );

    gpobj->logMsg ( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_SECURITY );

    m_pClientItem->bAuthenticated = true;
    write(   MSG_OK, strlen ( MSG_OK ) );

    return true;

}

///////////////////////////////////////////////////////////////////////////////
// handleChallenge
//

void TCPClientThread::handleChallenge( void )
{
    wxString wxstr;
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    m_pClientItem->m_currentCommand.Trim(true);
    m_pClientItem->m_currentCommand.Trim(false);

    memset( m_pClientItem->m_sid, 0, sizeof( m_pClientItem->m_sid ) );
    if ( !gpobj->generateSessionId( (const char *)m_pClientItem->m_currentCommand.mbc_str(), 
                                    m_pClientItem->m_sid ) ) {
        write( MSG_FAILED_TO_GENERATE_SID, strlen ( MSG_FAILED_TO_GENERATE_SID ) );
        return; 
    }
    
    wxstr = _("+OK - ") + wxString::FromUTF8( m_pClientItem->m_sid ) + _("\r\n");
    write( wxstr );
}


///////////////////////////////////////////////////////////////////////////////
// handleClientRcvLoop
//

void TCPClientThread::handleClientRcvLoop( void )
{
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    write(   MSG_RECEIVE_LOOP, strlen ( MSG_RECEIVE_LOOP ) );
    m_bReceiveLoop = true; // Mark connection as being in receive loop
    
    m_pClientItem->m_readBuffer.Empty();
    
    return;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientTest
//

void TCPClientThread::handleClientTest( void )
{
    write( MSG_OK, strlen ( MSG_OK ) );
    return;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientRestart
//

void TCPClientThread::handleClientRestart( void )
{
    write(  MSG_OK, strlen ( MSG_OK ) );
    return;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientShutdown
//

void TCPClientThread::handleClientShutdown( void )
{
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    if ( !m_pClientItem->bAuthenticated ) {
        write(   MSG_OK, strlen ( MSG_OK ) );
    }

    write(   MSG_GOODBY, strlen ( MSG_GOODBY ) );
    stcp_close_connection( m_conn );
    m_conn == NULL;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientRemote
//

void TCPClientThread::handleClientRemote( void )
{
    return;
}






// -----------------------------------------------------------------------------
//                            I N T E R F A C E
// -----------------------------------------------------------------------------







///////////////////////////////////////////////////////////////////////////////
// handleClientInterface
//
// list     List interfaces.
// unique   Aquire selected interface uniquely. Full format is INTERFACE UNIQUE id
// normal   Normal access to interfaces. Full format is INTERFACE NORMAL id
// close    Close interfaces. Full format is INTERFACE CLOSE id

void TCPClientThread::handleClientInterface( void )
{
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    gpobj->logMsg ( m_pClientItem->m_currentCommand, DAEMON_LOGMSG_NORMAL );

    if ( m_pClientItem->CommandStartsWith(_("list") ) ) {
        handleClientInterface_List();
    }
    else if ( m_pClientItem->CommandStartsWith(_("unique") ) ) {
        handleClientInterface_Unique();
    }
    else if ( m_pClientItem->CommandStartsWith(_("normal") ) ) {
        handleClientInterface_Normal();
    }
    else if ( m_pClientItem->CommandStartsWith(_("close") ) ) {
        handleClientInterface_Close();
    }
}

///////////////////////////////////////////////////////////////////////////////
// handleClientInterface_List
//

void TCPClientThread::handleClientInterface_List( void )
{
    wxString strGUID;
    wxString strBuf;

    // Display Interface List
    gpobj->m_wxClientMutex.Lock();

    std::list<CClientItem*>::iterator it;
    for (it = gpobj->m_clientList.m_clientItemList.begin();
        it != gpobj->m_clientList.m_clientItemList.end();
        ++it) {

            CClientItem *pItem = *it;
            
            pItem->m_guid.toString( strGUID );
            strBuf = wxString::Format(_("%d,"), pItem->m_clientID );
            strBuf += wxString::Format(_("%d,"), pItem->m_type );
            strBuf += strGUID;
            strBuf += _(",");
            strBuf += pItem->m_strDeviceName;
            strBuf += _("\r\n");

            write(   strBuf.mb_str(),
                                    strlen( strBuf.mb_str() ) );

    }

    write(  MSG_OK, strlen ( MSG_OK ) );

    gpobj->m_wxClientMutex.Unlock();
}

///////////////////////////////////////////////////////////////////////////////
// handleClientInterface_Unique
//

void TCPClientThread::handleClientInterface_Unique( void )
{
    unsigned char ifGUID[ 16 ];
    memset( ifGUID, 0, 16 );

    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    // Get GUID
    m_pClientItem->m_currentCommand.Trim(true);
    m_pClientItem->m_currentCommand.Trim(false);
    vscp_getGuidFromStringToArray( ifGUID, m_pClientItem->m_currentCommand );

    // Add the client to the Client List
    // TODO

    write(  MSG_INTERFACE_NOT_FOUND, strlen( MSG_INTERFACE_NOT_FOUND ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleClientInterface_Normal
//

void TCPClientThread::handleClientInterface_Normal( void )
{
    // TODO
}

///////////////////////////////////////////////////////////////////////////////
// handleClientInterface_Close
//

void TCPClientThread::handleClientInterface_Close( void )
{
    // TODO
}





// -----------------------------------------------------------------------------
//                          E N D   I N T E R F A C E
// -----------------------------------------------------------------------------







// -----------------------------------------------------------------------------
//                                  U D P
// -----------------------------------------------------------------------------




///////////////////////////////////////////////////////////////////////////////
// handleClientUdp
//

void TCPClientThread::handleClientUdp( void )
{
    // TODO
}





// -----------------------------------------------------------------------------
//                            E N D   U D P
// -----------------------------------------------------------------------------





// -----------------------------------------------------------------------------
//                                 F I L E
// -----------------------------------------------------------------------------






///////////////////////////////////////////////////////////////////////////////
// handleClientFile
//

void TCPClientThread::handleClientFile( void )
{
    // TODO
}





// -----------------------------------------------------------------------------
//                            E N D   F I L E
// -----------------------------------------------------------------------------





// -----------------------------------------------------------------------------
//                         S T A R T   T A B L E
// -----------------------------------------------------------------------------






///////////////////////////////////////////////////////////////////////////////
// handleClientTable
//

void TCPClientThread::handleClientTable( void )
{
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    m_pClientItem->m_currentCommand.Trim(true);
    m_pClientItem->m_currentCommand.Trim(false);    
    
    // List tables or table definition
    if ( m_pClientItem->CommandStartsWith(_("list") ) ) {
        handleClientTable_List();
    }
    // Get rawtable content
    else if ( m_pClientItem->CommandStartsWith(_("getraw") ) ) {
        handleClientTable_GetRaw();
    }
    // Get table content
    else if ( m_pClientItem->CommandStartsWith(_("get") ) ) {
        handleClientTable_Get();
    }    
    // Delete table data
    else if ( m_pClientItem->CommandStartsWith(_("clear") ) ) {
        handleClientTable_Clear();
    }
    // New table (create)
    else if ( m_pClientItem->CommandStartsWith(_("create") ) ) {
        handleClientTable_Create();
    }
    // Delete table
    else if ( m_pClientItem->CommandStartsWith(_("delete") ) ||
                m_pClientItem->CommandStartsWith(_("del") ) ) {
        handleClientTable_Delete();
    }
    // Log data use SQL
    else if ( m_pClientItem->CommandStartsWith(_("logsql") ) ) {
        handleClientTable_LogSQL();
    }
    // Log data
    else if ( m_pClientItem->CommandStartsWith(_("log") ) ) {
        handleClientTable_Log();
    }    
    // Get number of records
    else if ( m_pClientItem->CommandStartsWith(_("records") ) ) {
        handleClientTable_NumberOfRecords();
    }
    // Get first date
    else if ( m_pClientItem->CommandStartsWith(_("firstdate") ) ) {
        handleClientTable_FirstDate();
    }
    // Get last date
    else if ( m_pClientItem->CommandStartsWith(_("lastdate") ) ) {
        handleClientTable_FirstDate();
    }
    // Get sum
    else if ( m_pClientItem->CommandStartsWith(_("sum") ) ) {
        handleClientTable_Sum();
    }
    // Get min
    else if ( m_pClientItem->CommandStartsWith(_("min") ) ) {
        handleClientTable_Min();
    }
    // Get max
    else if ( m_pClientItem->CommandStartsWith(_("max") ) ) {
        handleClientTable_Min();
    }
    // Get average
    else if ( m_pClientItem->CommandStartsWith(_("average") ) ) {
        handleClientTable_Average();
    }
    // Get median
    else if ( m_pClientItem->CommandStartsWith(_("median") ) ) {
        handleClientTable_Median();
    }
    // Get stddev
    else if ( m_pClientItem->CommandStartsWith(_("stddev") ) ) {
        handleClientTable_StdDev();
    }
    // Get variance
    else if ( m_pClientItem->CommandStartsWith(_("variance") ) ) {
        handleClientTable_Variance();
    }
    // Get mode
    else if ( m_pClientItem->CommandStartsWith(_("mode") ) ) {
        handleClientTable_Mode();
    }
    // Get lowerq
    else if ( m_pClientItem->CommandStartsWith(_("lowerq") ) ) {
        handleClientTable_LowerQ();
    }
    // Get upperq
    else if ( m_pClientItem->CommandStartsWith(_("upperq") ) ) {
        handleClientTable_UpperQ();
    }
    
    // unrecognised
    else {
        write(  MSG_UNKNOWN_COMMAND, strlen( MSG_UNKNOWN_COMMAND ) );
    }
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_Create
//
// name, bEnable, bInMemory, type, size, owner, rights, title, xname, yname, note, 
//  sqlcreate, sqlinsert, sqldelete, description,
//  vscpclass, vscptype, 
// Optional parameter follow
//  sensorindex, unit, zone, subzone
//
// Table name must be unique
//


void TCPClientThread::handleClientTable_Create( void )
{
    wxString wxstr;
    wxString strName;
    bool bEnable = true;
    bool bInMemory = false;  
    vscpTableType type = VSCP_TABLE_DYNAMIC;
    uint32_t size = 0;
    wxString strOwner;
    uint16_t rights;
    wxString strTitle;
    wxString strXname;
    wxString strYname;   
    wxString strNote;
    wxString strSqlCreate;
    wxString strSqlInsert;
    wxString strSqlDelete;
    wxString strDescription;
    uint8_t vscpclass;
    uint8_t vscptype;
    uint8_t sensorindex = 0;
    uint8_t unit = 0;
    uint8_t zone = 255;
    uint8_t subzone = 255;
      
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;
    
    if ( !gpobj->m_userTableObjects.createTableFromXML( m_pClientItem->m_currentCommand ) ) {
        write( 
                    MSG_FAILED_TO_CREATE_TABLE, 
                    strlen( MSG_FAILED_TO_CREATE_TABLE ) );
        return;
    }
       
    // All went well
    write(  MSG_OK,  strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_Delete
//


void TCPClientThread::handleClientTable_Delete( void )
{
    wxString strTable;
    bool bRemoveFile = false;
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;   
        
    wxStringTokenizer tkz( m_pClientItem->m_currentCommand, _(" ") );
    
    // Get table name 
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        write( 
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // Should table db be removed 
    if ( tkz.HasMoreTokens() ) {
        wxString str = tkz.GetNextToken();
        if ( wxNOT_FOUND != str.Upper().Find("TRUE") ) {
            bRemoveFile = true;
        }
    }
    
    // Remove the table from the internal system
    if ( !gpobj->m_userTableObjects.removeTable( strTable ), bRemoveFile ) {
         // Failed
        write(  
                    MSG_FAILED_TO_REMOVE_TABLE, 
                    strlen( MSG_FAILED_TO_REMOVE_TABLE ) );
    }
    
    write(  MSG_OK, strlen( MSG_OK ) );
    
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_List
//

void TCPClientThread::handleClientTable_List( void )
{
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;
    
    m_pClientItem->m_currentCommand.Trim(true);
    m_pClientItem->m_currentCommand.Trim(false); 
    
    if ( 0 == m_pClientItem->m_currentCommand.Length() ) {
        
        // list without argument - list all defined tables
        wxArrayString arrayTblNames;
        gpobj->m_mutexUserTables.Lock();
        if ( gpobj->m_userTableObjects.getTableNames( arrayTblNames ) ) {
            
            // OK
            
            wxString str = wxString::Format( _("%zu rows \r\n"), 
                                                arrayTblNames.Count() );
            write(  
                        (const char *)str.mbc_str(), 
                        strlen( (const char *)str.mbc_str() ) );
            
            for ( int i=0; i<arrayTblNames.Count(); i++ ) {
                
                CVSCPTable *pTable = 
                    gpobj->m_userTableObjects.getTable( arrayTblNames[i] );
                
                str = arrayTblNames[i];
                str += _(";");
                if ( NULL != pTable ) {
                    
                    switch( pTable->getType() ) {
                        case VSCP_TABLE_DYNAMIC:
                            str += _("dynamic");
                            break;
                            
                        case VSCP_TABLE_STATIC:
                            str += _("static");
                            break;
                            
                        case VSCP_TABLE_MAX:
                            str += _("max");
                            break;
                            
                        default:
                            str += _("Error: Invalid type");
                            break;
                    }
                    
                }
                str += _(";");
                if ( NULL != pTable ) {
                    str += pTable->getDescription();
                }
                str += _("\r\n");
                write(  
                            (const char *)str.mbc_str(), 
                            strlen( (const char *)str.mbc_str() ) );
            }
            
            write( 
                        MSG_OK, 
                        strlen( MSG_OK ) );
            
        }
        else {
            
            // Failed
            write(  
                        MSG_FAILED_GET_TABLE_NAMES, 
                        strlen( MSG_FAILED_GET_TABLE_NAMES ) );
        }
        gpobj->m_mutexUserTables.Unlock();
    }
    else {
        
        // list with argument - list info about table given as argument
        // if table name is followed by another argument 'xml' the table should 
        // be in XML format.
        
        bool bXmlFormat = false;
        wxStringTokenizer tkz( m_pClientItem->m_currentCommand, _(" ") );
        wxString tblName = tkz.GetNextToken();
        
        if ( tkz.HasMoreTokens() ) {
            wxString str = tkz.GetNextToken();
            if ( wxNOT_FOUND != str.Upper().Find( _("XML") ) ) {
                bXmlFormat = true;  // Output XML format
            }
        }
        
        gpobj->m_mutexUserTables.Lock();
        
        CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( tblName );
        if ( NULL == pTable ) {
            // Failed
            write(  
                        MSG_FAILED_UNKNOWN_TABLE, 
                        strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        }   
        else {
            
            wxString str;
            
            if ( bXmlFormat ) {
                str = _("<table \r\n");
            }

            str += _("name=") + pTable->getTableName();
            str += _("\r\n");
            
            str += _("enabled=");
            str += pTable->isEnabled() ? _("true") : _("false");
            str += _("\r\n");
            
            str += _("type=");
            switch( pTable->getType() ) {
                case VSCP_TABLE_DYNAMIC:
                    str += _("dynamic");
                    break;
                            
                case VSCP_TABLE_STATIC:
                    str += _("static");
                    break;
                            
                case VSCP_TABLE_MAX:
                    str += _("max");
                    break;
                            
                default:
                    str += _("Error: Invalid type");
                    break;
            }
            str += _("\r\n");
            
            str += _("bmemory=");
            str += pTable->isInMemory() ? _("true") : _("false");
            str += _("\r\n");
            
            str += _("size=");
            str += wxString::Format( _("%lu "), (unsigned long)pTable->getSize() );
            str += _("\r\n");
            
            str += _("owner=");
            CUserItem *pUserItem = pTable->getUserItem();
            if ( NULL == pUserItem ) {
                str += _("ERROR");
            }
            else {
                str += pUserItem->getUserName();
            }
            str += _("\r\n");
            
            str += _("permission=");
            str += wxString::Format( _("0x%0X "), (int)pTable->getRights() );
            str += _("\r\n");
            
            str += _("description=");
            str += pTable->getDescription();
            str += _("\r\n");
            
            str += _("xname=");
            str += pTable->getLabelX();
            str += _("\r\n");
            
            str += _("yname=");
            str += pTable->getLabelY();
            str += _("\r\n");
            
            str += _("title=");
            str += pTable->getTitle();
            str += _("\r\n");
            
            str += _("note=");
            str += pTable->getNote();
            str += _("\r\n");
            
            str += _("vscp-class=");
            str += wxString::Format( _("%d"), (int)pTable->getVSCPClass() );
            str += _("\r\n");
            
            str += _("vscp-type=");
            str += wxString::Format( _("%d"), (int)pTable->getVSCPType() );
            str += _("\r\n");
            
            str += _("vscp-sensor-index=");
            str += wxString::Format( _("%d"), (int)pTable->getVSCPSensorIndex() );
            str += _("\r\n");
            
            str += _("vscp-unit=");
            str += wxString::Format( _("%d"), (int)pTable->getVSCPUnit() );
            str += _("\r\n");
            
            str += _("vscp-zone=");
            str += wxString::Format( _("%d"), (int)pTable->getVSCPZone() );
            str += _("\r\n");
            
            str += _("vscp-subzone=");
            str += wxString::Format( _("%d"), (int)pTable->getVSCPSubZone() );
            str += _("\r\n");
            
            str += _("vscp-create=");
            str += pTable->getSQLCreate();
            str += _("\r\n");
            
            str += _("vscp-insert=");
            str += pTable->getSQLInsert();
            str += _("\r\n");
            
            str += _("vscp-delete=");
            str += pTable->getSQLDelete();
            str += _("\r\n");
            
            if ( bXmlFormat ) {
                str += _("/> \r\n");
                str = wxBase64Encode( str.ToUTF8(), strlen( str.ToUTF8() ) );
                str += _("\r\n");
            }
            
            // Send response
            write( 
                        (const char *)str.mbc_str(), 
                        strlen( (const char *)str.mbc_str() ) );
            
            // Tell user we are content with things.
            write( 
                        MSG_OK, 
                        strlen( MSG_OK ) );
            
        }
        gpobj->m_mutexUserTables.Unlock();
    }
        
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_Get
//
// get 'table-name' start end ["full"]
//

void TCPClientThread::handleClientTable_Get( void )
{
    wxString strTable;
    wxDateTime wxStart;   
    wxDateTime wxEnd;
    bool bAll = false;
    
    // Initialize date range to 'all'
    wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
    wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;    
    
    m_pClientItem->m_currentCommand.Trim(true);
    m_pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( m_pClientItem->m_currentCommand, _(" ") );
    
    // Get table name 
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        write( 
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // The start and end parameters must be present
    
    // start
    if ( tkz.HasMoreTokens() ) {
        
        wxStart.ParseISOCombined( tkz.GetNextToken() );
        
        if ( !wxStart.IsValid() ) {
            write( 
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
    }
    else {
        // Problems: A start date must be given
        write( 
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // end
    if ( tkz.HasMoreTokens() ) {
        
        wxEnd.ParseISOCombined( tkz.GetNextToken() );
        
        if ( !wxEnd.IsValid() ) {
            write( 
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
    }
    else {
        // Problems: An end date must be given
        write( 
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // "full" flag
    if ( tkz.HasMoreTokens() ) {
        wxString str = tkz.GetNextToken();
        if ( str.Upper().StartsWith( _("FULL") ) ) {
            bAll = true;
        }
    }
    
    gpobj->m_mutexUserTables.Lock();
    
    CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    if ( NULL == pTable ) {
        // Failed
        write(  
                    MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    }   

    sqlite3_stmt *ppStmt;
    if ( !pTable->prepareRangeOfData( wxStart, wxEnd, &ppStmt, bAll ) ) {
        // Failed
        write(  
                    MSG_FAILED_TO_PREPARE_TABLE, 
                    strlen( MSG_FAILED_TO_PREPARE_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    wxString str;
    wxArrayString strArray;
    while ( pTable->getRowRangeOfData( ppStmt, str ) ) {
        strArray.Add( str );
    }
    
    str = wxString::Format( _("%d rows.\r\n"), (int)strArray.Count() );
    write(  
                (const char *)str.mbc_str(), 
                strlen( (const char *)str.mbc_str() ) );
    
    if ( strArray.Count() ) {
        for ( int i=0; i<strArray.Count(); i++ ) {
            str = strArray[i];
            str += _("\r\n");
            write(  
                    (const char *)str.mbc_str(), 
                    strlen( (const char *)str.mbc_str() ) );
        }
    }    
    
    if ( !pTable->finalizeRangeOfData( ppStmt )  ) {
        // Failed
        write(  
                    MSG_FAILED_TO_FINALIZE_TABLE, 
                    strlen( MSG_FAILED_TO_FINALIZE_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    gpobj->m_mutexUserTables.Unlock();
        
    // Everything is OK
    write(  MSG_OK, strlen( MSG_OK ) );
}


///////////////////////////////////////////////////////////////////////////////
// handleClientTable_GetRaw
//
// get 'table-name' start end ["full"]
//

void TCPClientThread::handleClientTable_GetRaw( void )
{
    wxString strTable;
    wxDateTime wxStart;   
    wxDateTime wxEnd;
    
    // Initialize date range to 'all'
    wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
    wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;    
    
    m_pClientItem->m_currentCommand.Trim(true);
    m_pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( m_pClientItem->m_currentCommand, _(" ") );
    
    // Get table name 
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        write( MSG_PARAMETER_ERROR, 
                strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // The start and end parameters must be present
    
    // start
    if ( tkz.HasMoreTokens() ) {
        
        wxStart.ParseISOCombined( tkz.GetNextToken() );
        
        if ( !wxStart.IsValid() ) {
            write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
    }
    else {
        // Problems: A start date must be given
        write( MSG_PARAMETER_ERROR, 
                strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // end
    if ( tkz.HasMoreTokens() ) {
        
        wxEnd.ParseISOCombined( tkz.GetNextToken() );
        
        if ( !wxEnd.IsValid() ) {
            write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
    }
    else {
        // Problems: An end date must be given
        write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
        
    gpobj->m_mutexUserTables.Lock();
    
    CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    if ( NULL == pTable ) {
        // Failed
        write( MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    }   

    sqlite3_stmt *ppStmt;
    if ( !pTable->prepareRangeOfData( wxStart, wxEnd, &ppStmt, true ) ) {
        // Failed
        write( MSG_FAILED_TO_PREPARE_TABLE, 
                    strlen( MSG_FAILED_TO_PREPARE_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    wxString str;
    wxArrayString strArray;
    while ( pTable->getRowRangeOfDataRaw( ppStmt, str ) ) {
        strArray.Add( str );
    }
    
    str = wxString::Format( _("%d rows.\r\n"), (int)strArray.Count() );
    write(  
                (const char *)str.mbc_str(), 
                strlen( (const char *)str.mbc_str() ) );
    
    if ( strArray.Count() ) {
        for ( int i=0; i<strArray.Count(); i++ ) {
            str = strArray[i];
            str += _("\r\n");
            write(  
                    (const char *)str.mbc_str(), 
                    strlen( (const char *)str.mbc_str() ) );
        }
    }    
    
    if ( !pTable->finalizeRangeOfData( ppStmt )  ) {
        // Failed
        write( MSG_FAILED_TO_FINALIZE_TABLE, 
                    strlen( MSG_FAILED_TO_FINALIZE_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    gpobj->m_mutexUserTables.Unlock();
        
    // Everything is OK
    write( MSG_OK, strlen( MSG_OK ) );
}


///////////////////////////////////////////////////////////////////////////////
// handleClientTable_Clear
//
// clear 'table-name' [to,from]
//

void TCPClientThread::handleClientTable_Clear( void )
{    
    wxString strTable;
    wxDateTime wxStart;   
    wxDateTime wxEnd;
    bool bClearAll = false;
    
    // Initialize date range to 'all'
    wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
    wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;
    
    m_pClientItem->m_currentCommand.Trim(true);
    m_pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( m_pClientItem->m_currentCommand, _(" ") );
    
    // Get table name
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // get range if given
    if ( tkz.CountTokens() >= 2 ) {
        
        if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
            write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
        if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
            write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
    }
    else {
        bClearAll = true;
    }
    
    gpobj->m_mutexUserTables.Lock();
    
    CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
    if ( NULL == pTable ) {
        // Failed
        write( MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    } 
    
    if ( bClearAll ) {
        if ( !pTable->clearTable() ) {
            write( MSG_FAILED_TO_CLEAR_TABLE, 
                    strlen( MSG_FAILED_TO_CLEAR_TABLE ) );
            gpobj->m_mutexUserTables.Unlock();
            return;
        }
    }
    else {
        if ( !pTable->clearTableRange( wxStart, wxEnd ) ) {
            write( MSG_FAILED_TO_CLEAR_TABLE, 
                    strlen( MSG_FAILED_TO_CLEAR_TABLE ) );
            gpobj->m_mutexUserTables.Unlock();
            return;
        }
    }
            
    gpobj->m_mutexUserTables.Unlock();
    
    write( MSG_OK,  
                strlen( MSG_OK ) );
    
    return;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientTable_Log
//
// log table-name value [datetime]
//

void TCPClientThread::handleClientTable_Log( void )
{
    wxString strTable;
    double value;
    wxDateTime dt;
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;    
    
    m_pClientItem->m_currentCommand.Trim(true);
    m_pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( m_pClientItem->m_currentCommand, _(" ") );
    
    // Get table name 
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // Get the value
    if ( tkz.HasMoreTokens() ) {
        wxString str = tkz.GetNextToken();
        if ( !str.ToDouble( &value ) ) {
            // Problems: The value is not in a valid format
            write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;            
        }
    }
    else {
        // Problems: A value must be given
        write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // Get the datetime if its there
    if ( tkz.HasMoreTokens() ) {
        
        uint32_t ms = 0;
        
        wxString str = tkz.GetNextToken();
        str.Trim(true);
        str.Trim(false);
        
        if ( !dt.ParseISOCombined( str ) ) {
            // Problems: The value is not in a valid format
            write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;            
        }
        
        // Get possible millisecond part
        str = str.AfterFirst('.');
        str.Trim();
        if ( str.Length() ) {
            ms = vscp_readStringValue( str );
        }    
        
        dt.SetMillisecond( ms );
        
    }
    else {
        // Set to now
        dt = wxDateTime::UNow();
    }
    
    
    gpobj->m_mutexUserTables.Lock();
    
    CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
    if ( NULL == pTable ) {
        // Failed
        write( MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    // Log data
    if ( !pTable->logData( dt, value ) ) {
        gpobj->m_mutexUserTables.Unlock();
        write( MSG_FAILED_TO_WRITE_TABLE,  
                    strlen( MSG_FAILED_TO_WRITE_TABLE ) );
        return;
    }
    
    gpobj->m_mutexUserTables.Unlock();
    
    write( MSG_OK,  
                strlen( MSG_OK ) );
}


///////////////////////////////////////////////////////////////////////////////
// handleClientTable_LogSQL
//

void TCPClientThread::handleClientTable_LogSQL( void )
{
    wxString strTable, strSQL;
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;    
    
    m_pClientItem->m_currentCommand.Trim(true);
    m_pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( m_pClientItem->m_currentCommand, _(" ") );
    
    // Get table name 
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        write( 
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    
    // Get SQL expression 
    if ( tkz.HasMoreTokens() ) {
        strSQL = tkz.GetNextToken();
        strSQL.Trim(true);
        strSQL.Trim(false);
    }
    else {
        // Problems: A SQL expression must be given
        write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    
    gpobj->m_mutexUserTables.Lock();
    
    CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
    if ( NULL == pTable ) {
        // Failed
        write( MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    // Log data
    if ( !pTable->logData( strSQL ) ) {
        gpobj->m_mutexUserTables.Unlock();
        write( MSG_FAILED_TO_WRITE_TABLE,  
                    strlen( MSG_FAILED_TO_WRITE_TABLE ) );
        return;
    }
    
    gpobj->m_mutexUserTables.Unlock();
    
}


///////////////////////////////////////////////////////////////////////////////
// handleClientTable_NumberOfRecords
//
// records 'table-name' [to,from]
//

void TCPClientThread::handleClientTable_NumberOfRecords( void )
{    
    wxString strTable;
    wxDateTime wxStart;   
    wxDateTime wxEnd;
    
    // Initialize date range to 'all'
    wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
    wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;
    
    m_pClientItem->m_currentCommand.Trim(true);
    m_pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( m_pClientItem->m_currentCommand, _(" ") );
    
    // Get table name
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // get range if given
    if ( tkz.CountTokens() >= 2 ) {
        
        if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
            write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
        if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
            write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
    }
    
    gpobj->m_mutexUserTables.Lock();
    
    CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
    if ( NULL == pTable ) {
        // Failed
        write( MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    } 
    
    double count;
    if ( !pTable->getNumberOfRecordsForRange( wxStart, wxEnd, &count ) ) {
        write( MSG_FAILED_TO_CLEAR_TABLE, 
                    strlen( MSG_FAILED_TO_CLEAR_TABLE ) );
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    gpobj->m_mutexUserTables.Unlock();
    
    wxString strReply = wxString::Format(_("%f\r\n"), count );
    write( (const char *)strReply.mbc_str(),  
                strlen( (const char *)strReply.mbc_str() ) );
    
    write( MSG_OK,  
                strlen( MSG_OK ) );
    
    return;
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_FirstDate
//
// firstdate 'table-name' [to,from]
//

void TCPClientThread::handleClientTable_FirstDate( void )
{    
    wxString strTable;
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;
    
    m_pClientItem->m_currentCommand.Trim(true);
    m_pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( m_pClientItem->m_currentCommand, _(" ") );
    
    // Get table name
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    
    gpobj->m_mutexUserTables.Lock();
    
    CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
    if ( NULL == pTable ) {
        // Failed
        write( MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    } 
    
    wxDateTime first;
    if ( !pTable->getFirstDate( first ) ) {
        write( MSG_FAILED_TO_CLEAR_TABLE, 
                    strlen( MSG_FAILED_TO_CLEAR_TABLE ) );
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    gpobj->m_mutexUserTables.Unlock();
    
    wxString strReply = wxString::Format(_("%s\r\n"), 
                            (const char *)first.FormatISOCombined().mbc_str() );
    write( (const char *)strReply.mbc_str(),  
                strlen( (const char *)strReply.mbc_str() ) );
    
    write( MSG_OK,  
                strlen( MSG_OK ) );
    
    return;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientTable_LastDate
//
// lastdate 'table-name' [to,from]
//

void TCPClientThread::handleClientTable_LastDate( void )
{    
    wxString strTable;
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;
    
    m_pClientItem->m_currentCommand.Trim(true);
    m_pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( m_pClientItem->m_currentCommand, _(" ") );
    
    // Get table name
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    gpobj->m_mutexUserTables.Lock();
    
    CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
    if ( NULL == pTable ) {
        // Failed
        write( MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    } 
    
    wxDateTime last;
    if ( !pTable->getLastDate( last) ) {
        write( MSG_FAILED_TO_CLEAR_TABLE, 
                    strlen( MSG_FAILED_TO_CLEAR_TABLE ) );
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    gpobj->m_mutexUserTables.Unlock();
    
    wxString strReply = wxString::Format(_("%s\r\n"), 
                            (const char *)last.FormatISOCombined().mbc_str() );
    write( (const char *)strReply.mbc_str(),  
                strlen( (const char *)strReply.mbc_str() ) );
    
    write( MSG_OK,  
                strlen( MSG_OK ) );
    
    return;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientTable_Sum
//
// sum 'table-name' [to,from]
//

void TCPClientThread::handleClientTable_Sum( void )
{    
    wxString strTable;
    wxDateTime wxStart;   
    wxDateTime wxEnd;
    
    // Initialize date range to 'all'
    wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
    wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;
    
    m_pClientItem->m_currentCommand.Trim(true);
    m_pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( m_pClientItem->m_currentCommand, _(" ") );
    
    // Get table name
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // get range if given
    if ( tkz.CountTokens() >= 2 ) {
        
        if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
            write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
        if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
            write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
    }
    
    gpobj->m_mutexUserTables.Lock();
    
    CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
    if ( NULL == pTable ) {
        // Failed
        write( MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    } 
    
    double sum;
    if ( !pTable->getSumValue( wxStart, wxEnd, &sum ) ) {
        write( MSG_FAILED_TO_CLEAR_TABLE, 
                    strlen( MSG_FAILED_TO_CLEAR_TABLE ) );
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    gpobj->m_mutexUserTables.Unlock();
    
    wxString strReply = wxString::Format(_("%f\r\n"), sum );
    write( (const char *)strReply.mbc_str(),  
                strlen( (const char *)strReply.mbc_str() ) );
    
    write( MSG_OK,  
                strlen( MSG_OK ) );
    
    return;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientTable_Min
//
// min 'table-name' [to,from]
//

void TCPClientThread::handleClientTable_Min( void )
{    
    wxString strTable;
    wxDateTime wxStart;   
    wxDateTime wxEnd;
    
    // Initialize date range to 'all'
    wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
    wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;
    
    m_pClientItem->m_currentCommand.Trim(true);
    m_pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( m_pClientItem->m_currentCommand, _(" ") );
    
    // Get table name
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // get range if given
    if ( tkz.CountTokens() >= 2 ) {
        
        if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
            write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
        if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
            write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
    }
    
    gpobj->m_mutexUserTables.Lock();
    
    CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
    if ( NULL == pTable ) {
        // Failed
        write( MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    } 
    
    double min;
    if ( !pTable->getMinValue( wxStart, wxEnd, &min ) ) {
        write( MSG_FAILED_TO_CLEAR_TABLE, 
                    strlen( MSG_FAILED_TO_CLEAR_TABLE ) );
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    gpobj->m_mutexUserTables.Unlock();
    
    wxString strReply = wxString::Format(_("%f\r\n"), min );
    write( (const char *)strReply.mbc_str(),  
                strlen( (const char *)strReply.mbc_str() ) );
    
    write( MSG_OK,  
                strlen( MSG_OK ) );
    
    return;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientTable_Max
//
// max 'table-name' [to,from]
//

void TCPClientThread::handleClientTable_Max( void )
{    
    wxString strTable;
    wxDateTime wxStart;   
    wxDateTime wxEnd;
    
    // Initialize date range to 'all'
    wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
    wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;
    
    m_pClientItem->m_currentCommand.Trim(true);
    m_pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( m_pClientItem->m_currentCommand, _(" ") );
    
    // Get table name
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // get range if given
    if ( tkz.CountTokens() >= 2 ) {
        
        if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
            write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
        if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
            write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
    }
    
    gpobj->m_mutexUserTables.Lock();
    
    CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
    if ( NULL == pTable ) {
        // Failed
        write( MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    } 
    
    double max;
    if ( !pTable->getMaxValue( wxStart, wxEnd, &max ) ) {
        write( MSG_FAILED_TO_CLEAR_TABLE, 
                    strlen( MSG_FAILED_TO_CLEAR_TABLE ) );
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    gpobj->m_mutexUserTables.Unlock();
    
    wxString strReply = wxString::Format(_("%f\r\n"), max );
    write( (const char *)strReply.mbc_str(),  
                strlen( (const char *)strReply.mbc_str() ) );
    
    write( MSG_OK,  
                strlen( MSG_OK ) );
    
    return;
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_Average
//
// average 'table-name' [to,from]
//

void TCPClientThread::handleClientTable_Average( void )
{    
    wxString strTable;
    wxDateTime wxStart;   
    wxDateTime wxEnd;
    
    // Initialize date range to 'all'
    wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
    wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;
    
    m_pClientItem->m_currentCommand.Trim(true);
    m_pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( m_pClientItem->m_currentCommand, _(" ") );
    
    // Get table name
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // get range if given
    if ( tkz.CountTokens() >= 2 ) {
        
        if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
            write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
        if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
            write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
    }
    
    gpobj->m_mutexUserTables.Lock();
    
    CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
    if ( NULL == pTable ) {
        // Failed
        write( MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    } 
    
    double average;
    if ( !pTable->getAverageValue( wxStart, wxEnd, &average ) ) {
        write( MSG_FAILED_TO_CLEAR_TABLE, 
                    strlen( MSG_FAILED_TO_CLEAR_TABLE ) );
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    gpobj->m_mutexUserTables.Unlock();
    
    wxString strReply = wxString::Format(_("%f\r\n"), average );
    write( (const char *)strReply.mbc_str(),  
                strlen( (const char *)strReply.mbc_str() ) );
    
    write( MSG_OK,  
                strlen( MSG_OK ) );
    
    return;
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_Median
//
// median 'table-name' [to,from]
//

void TCPClientThread::handleClientTable_Median( void )
{    
    wxString strTable;
    wxDateTime wxStart;   
    wxDateTime wxEnd;
    
    // Initialize date range to 'all'
    wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
    wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;
    
    m_pClientItem->m_currentCommand.Trim(true);
    m_pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( m_pClientItem->m_currentCommand, _(" ") );
    
    // Get table name
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // get range if given
    if ( tkz.CountTokens() >= 2 ) {
        
        if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
            write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
        if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
            write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
    }
    
    gpobj->m_mutexUserTables.Lock();
    
    CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
    if ( NULL == pTable ) {
        // Failed
        write( MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    } 
    
    double median;
    if ( !pTable->getMedianValue( wxStart, wxEnd, &median ) ) {
        write( MSG_FAILED_TO_CLEAR_TABLE, 
                    strlen( MSG_FAILED_TO_CLEAR_TABLE ) );
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    gpobj->m_mutexUserTables.Unlock();
    
    wxString strReply = wxString::Format(_("%f\r\n"), median );
    write( (const char *)strReply.mbc_str(),  
                strlen( (const char *)strReply.mbc_str() ) );
    
    write( MSG_OK,  
                strlen( MSG_OK ) );
    
    return;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientTable_StdDev
//
// stddev 'table-name' [to,from]
//

void TCPClientThread::handleClientTable_StdDev( void )
{    
    wxString strTable;
    wxDateTime wxStart;   
    wxDateTime wxEnd;
    
    // Initialize date range to 'all'
    wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
    wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;
    
    m_pClientItem->m_currentCommand.Trim(true);
    m_pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( m_pClientItem->m_currentCommand, _(" ") );
    
    // Get table name
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // get range if given
    if ( tkz.CountTokens() >= 2 ) {
        
        if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
            write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
        if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
            write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
    }
    
    gpobj->m_mutexUserTables.Lock();
    
    CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
    if ( NULL == pTable ) {
        // Failed
        write( MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    } 
    
    double stdev;
    if ( !pTable->getStdevValue( wxStart, wxEnd, &stdev ) ) {
        write( MSG_FAILED_TO_CLEAR_TABLE, 
                    strlen( MSG_FAILED_TO_CLEAR_TABLE ) );
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    gpobj->m_mutexUserTables.Unlock();
    
    wxString strReply = wxString::Format(_("%f\r\n"), stdev );
    write( (const char *)strReply.mbc_str(),  
                strlen( (const char *)strReply.mbc_str() ) );
    
    write( MSG_OK,  
                strlen( MSG_OK ) );
    
    return;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientTable_Variance
//
// variance 'table-name' [to,from]
//

void TCPClientThread::handleClientTable_Variance( void )
{    
    wxString strTable;
    wxDateTime wxStart;   
    wxDateTime wxEnd;
    
    // Initialize date range to 'all'
    wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
    wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;
    
    m_pClientItem->m_currentCommand.Trim(true);
    m_pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( m_pClientItem->m_currentCommand, _(" ") );
    
    // Get table name
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // get range if given
    if ( tkz.CountTokens() >= 2 ) {
        
        if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
            write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
        if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
            write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
    }
    
    gpobj->m_mutexUserTables.Lock();
    
    CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
    if ( NULL == pTable ) {
        // Failed
        write( MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    } 
    
    double variance;
    if ( !pTable->getVarianceValue( wxStart, wxEnd, &variance ) ) {
        write( MSG_FAILED_TO_CLEAR_TABLE, 
                    strlen( MSG_FAILED_TO_CLEAR_TABLE ) );
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    gpobj->m_mutexUserTables.Unlock();
    
    wxString strReply = wxString::Format(_("%f\r\n"), variance );
    write( (const char *)strReply.mbc_str(),  
                strlen( (const char *)strReply.mbc_str() ) );
    
    write( MSG_OK,  
                strlen( MSG_OK ) );
    
    return;
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_Mode
//
// mode 'table-name' [to,from]
//

void TCPClientThread::handleClientTable_Mode( void )
{    
    wxString strTable;
    wxDateTime wxStart;   
    wxDateTime wxEnd;
    
    // Initialize date range to 'all'
    wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
    wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;
    
    m_pClientItem->m_currentCommand.Trim(true);
    m_pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( m_pClientItem->m_currentCommand, _(" ") );
    
    // Get table name
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // get range if given
    if ( tkz.CountTokens() >= 2 ) {
        
        if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
            write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
        if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
            write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
    }
    
    gpobj->m_mutexUserTables.Lock();
    
    CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
    if ( NULL == pTable ) {
        // Failed
        write( MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    } 
    
    double mode;
    if ( !pTable->getModeValue( wxStart, wxEnd, &mode ) ) {
        write( MSG_FAILED_TO_CLEAR_TABLE, 
                    strlen( MSG_FAILED_TO_CLEAR_TABLE ) );
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    gpobj->m_mutexUserTables.Unlock();
    
    wxString strReply = wxString::Format(_("%f\r\n"), mode );
    write( (const char *)strReply.mbc_str(),  
                strlen( (const char *)strReply.mbc_str() ) );
    
    write( MSG_OK,  
                strlen( MSG_OK ) );
    
    return;
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_LowerQ
//
// lowerq 'table-name' [to,from]
//

void TCPClientThread::handleClientTable_LowerQ( void )
{    
    wxString strTable;
    wxDateTime wxStart;   
    wxDateTime wxEnd;
    
    // Initialize date range to 'all'
    wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
    wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;
    
    m_pClientItem->m_currentCommand.Trim(true);
    m_pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( m_pClientItem->m_currentCommand, _(" ") );
    
    // Get table name
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // get range if given
    if ( tkz.CountTokens() >= 2 ) {
        
        if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
            write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
        if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
            write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
    }
    
    gpobj->m_mutexUserTables.Lock();
    
    CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
    if ( NULL == pTable ) {
        // Failed
        write( MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    } 
    
    double lq;
    if ( !pTable->getLowerQuartileValue( wxStart, wxEnd, &lq ) ) {
        write( MSG_FAILED_TO_CLEAR_TABLE, 
                    strlen( MSG_FAILED_TO_CLEAR_TABLE ) );
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    gpobj->m_mutexUserTables.Unlock();
    
    wxString strReply = wxString::Format(_("%f\r\n"), lq );
    write( (const char *)strReply.mbc_str(),  
                strlen( (const char *)strReply.mbc_str() ) );
    
    write( MSG_OK,  
                strlen( MSG_OK ) );
    
    return;
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_upperq
//
// upperq 'table-name' [to,from]
//

void TCPClientThread::handleClientTable_UpperQ( void )
{    
    wxString strTable;
    wxDateTime wxStart;   
    wxDateTime wxEnd;
    
    // Initialize date range to 'all'
    wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
    wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;
    
    m_pClientItem->m_currentCommand.Trim(true);
    m_pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( m_pClientItem->m_currentCommand, _(" ") );
    
    // Get table name
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // get range if given
    if ( tkz.CountTokens() >= 2 ) {
        
        if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
            write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
        if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
            write( MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
    }
    
    gpobj->m_mutexUserTables.Lock();
    
    CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
    if ( NULL == pTable ) {
        // Failed
        write( MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    } 
    
    double uq;
    if ( !pTable->getUppeQuartileValue( wxStart, wxEnd, &uq ) ) {
        write( MSG_FAILED_TO_CLEAR_TABLE, 
                    strlen( MSG_FAILED_TO_CLEAR_TABLE ) );
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    gpobj->m_mutexUserTables.Unlock();
    
    wxString strReply = wxString::Format(_("%f\r\n"), uq );
    write( (const char *)strReply.mbc_str(),  
                strlen( (const char *)strReply.mbc_str() ) );
    
    write( MSG_OK,  
                strlen( MSG_OK ) );
    
    return;
}











// -----------------------------------------------------------------------------
//                            E N D   T A B L E
// -----------------------------------------------------------------------------






// -----------------------------------------------------------------------------
//                            E N D   T A B L E
// -----------------------------------------------------------------------------





///////////////////////////////////////////////////////////////////////////////
// handleClientVariable
//

void TCPClientThread::handleClientVariable( void )
{
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    m_pClientItem->m_currentCommand.Trim(false);
    m_pClientItem->m_currentCommand.Trim(true);

    if ( m_pClientItem->CommandStartsWith(_("list") ) ) {
        handleVariable_List();
    }
    else if ( m_pClientItem->CommandStartsWith(_("writevalue") ) ) {
        handleVariable_WriteValue();
    }
    else if ( m_pClientItem->CommandStartsWith(_("writenote") ) ) {
        handleVariable_WriteNote();
    }
    else if ( m_pClientItem->CommandStartsWith(_("write") ) ) {
        handleVariable_Write();
    }
    else if ( m_pClientItem->CommandStartsWith(_("readvalue") ) ) {
        handleVariable_ReadValue();
    }
    else if ( m_pClientItem->CommandStartsWith(_("readnote") ) ) {
        handleVariable_ReadNote();
    }
    else if ( m_pClientItem->CommandStartsWith(_("readreset") ) ) {
        handleVariable_ReadReset();
    }  
    else if ( m_pClientItem->CommandStartsWith(_("readremove") ) ) {
        handleVariable_ReadRemove();
    }
    else if ( m_pClientItem->CommandStartsWith(_("read") ) ) {
        handleVariable_Read();
    }  
    else if ( m_pClientItem->CommandStartsWith(_("reset") ) ) {
        handleVariable_Reset();
    }
    else if ( m_pClientItem->CommandStartsWith(_("remove") ) ) {
        handleVariable_Remove();
    }    
    else if ( m_pClientItem->CommandStartsWith(_("length") ) ) {
        handleVariable_Length();
    }
    else if ( m_pClientItem->CommandStartsWith(_("load") ) ) {
        handleVariable_Load();
    }
    else if ( m_pClientItem->CommandStartsWith(_("save") ) ) {
        handleVariable_Save();
    }
    else {
        write( MSG_UNKNOWN_COMMAND, strlen( MSG_UNKNOWN_COMMAND ) );
    }
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_List
//
// variable list - List all variables.
// variable list test - List all variables with "test" in there name
//

void TCPClientThread::handleVariable_List( void )
{
    CVSCPVariable variable;
    wxString wxstr;
    wxString strWork;
    wxString strSearch;
    int type = 0;   // All variables
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    m_pClientItem->m_currentCommand.Trim(false);
    m_pClientItem->m_currentCommand.Trim(true);

    wxStringTokenizer tkz( m_pClientItem->m_currentCommand, _(" ") );
                                       
    // Check for variable name
    if ( tkz.HasMoreTokens() ) {
        
        wxString token = tkz.GetNextToken();
        
        token.Trim(true);
        token.Trim(false);
        if ( !token.empty() ) {
            strSearch = token;
        }
        else {
            strSearch = _("(.*))");     // List all
        }
    }
    else {
        strSearch = _("(.*)");          // List all
    }
    
    // Check for variable type
    if ( tkz.HasMoreTokens() ) {
        wxString str = tkz.GetNextToken();
        str.Trim(true);
        str.Trim(false);
        type = vscp_readStringValue( str );
    }

    // Get all variable names
    wxArrayString arrayVars;
    gpobj->m_variables.getVarlistFromRegExp( arrayVars, strSearch );
    
    if ( arrayVars.Count() ) {
        
        wxstr = wxString::Format( _("%zu rows.\r\n"), arrayVars.Count() );
        write(   wxstr.mb_str(), wxstr.Length() );
    
        int cnt = 0;
        for ( int i=0; i<arrayVars.Count(); i++ ) {
            if ( 0 != gpobj->m_variables.find( arrayVars[ i ], variable ) ) {
                if ( ( 0 == type ) || ( variable.getType() == type ) ) {
                    wxstr = wxString::Format( _("%d;"), cnt );
                    wxstr += variable.getAsString();
                    wxstr += _("\r\n");
                    write(   wxstr.mb_str(), wxstr.Length() );
                    cnt++;
                }
            }
        }
    
    }
    else {
        wxstr = _("0 rows.\r\n");
        write(   wxstr.mb_str(), wxstr.Length() );
    } 

    write(   MSG_OK, strlen ( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_Write
//
// Format is: "variable name";"type";"persistence";"owner";"rights";"value";"note"
//
// test31;string;true;0;0x777;dGhpcyBpcyBhIHRlc3Q=;VGhpcyBpcyBhIG5vdGUgZm9yIGEgdGVzdCB2YXJpYWJsZQ==
//

void TCPClientThread::handleVariable_Write( void )
{
    CVSCPVariable variable;
    wxString wxstr;
    
    bool bPersistence = false;
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    m_pClientItem->m_currentCommand.Trim(false);
    m_pClientItem->m_currentCommand.Trim(true);
    
    if ( !variable.setVariableFromString( m_pClientItem->m_currentCommand, 
                                                false, 
                                                m_pClientItem->m_UserName ) ) {
        // Failed to parse
        write( MSG_PARAMETER_ERROR, strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    if ( gpobj->m_variables.exist( variable.getName() ) ) {
        
        // Update in database
        if ( !gpobj->m_variables.update( variable ) ) {
            write(  MSG_VARIABLE_NO_SAVE, strlen ( MSG_VARIABLE_NO_SAVE ) );
            return;
        }
        
    }
    else {
       
        // If the variable exist change value 
        // if not - add it.
        if ( !gpobj->m_variables.add( variable ) ) {
            write(  MSG_VARIABLE_UNABLE_ADD, strlen ( MSG_VARIABLE_UNABLE_ADD ) );
            return;
        }
        
    }
    
    write( MSG_OK, strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_WriteValue
//

void TCPClientThread::handleVariable_WriteValue( void )
{
    wxString name;
    wxString value;
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;
    
    m_pClientItem->m_currentCommand.Trim(false);
    m_pClientItem->m_currentCommand.Trim(true);
    
    wxStringTokenizer tkz( m_pClientItem->m_currentCommand, _(" \r\n") );
    
    // Variable name
    if ( tkz.HasMoreTokens() ) {
        name = tkz.GetNextToken();
    }
    else {
        write( MSG_PARAMETER_ERROR, strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // Variable value
    if ( tkz.HasMoreTokens() ) {
        
        value = tkz.GetNextToken();
        
    }
    else {
        write( MSG_PARAMETER_ERROR, strlen( MSG_PARAMETER_ERROR ) );
        return;
    }

    CVSCPVariable variable;
    if ( 0 != gpobj->m_variables.find( name, variable ) ) {
        
        // Set value and encode as BASE64 when expected
        variable.setValueFromString( variable.getType(), value, false );
        
        // Update in database
        if ( !gpobj->m_variables.update( variable ) ) {
            write( MSG_VARIABLE_NO_SAVE, strlen ( MSG_VARIABLE_NO_SAVE ) );
            return;
        }
        
    }
    else {
        write( MSG_VARIABLE_NOT_DEFINED, strlen ( MSG_VARIABLE_NOT_DEFINED ) );
    }
    
    write(  MSG_OK, strlen ( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_WriteNote
//

void TCPClientThread::handleVariable_WriteNote( void )
{
    wxString str;
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;
    
    m_pClientItem->m_currentCommand.Trim(false);
    m_pClientItem->m_currentCommand.Trim(true);
    
    wxString name;
    wxString note;
    
    wxStringTokenizer tkz( m_pClientItem->m_currentCommand, _(" ") );
    
    // Variable name
    if ( tkz.HasMoreTokens() ) {
        name = tkz.GetNextToken();
    }
    else {
        write( MSG_PARAMETER_ERROR, strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // Variable value
    if ( tkz.HasMoreTokens() ) {
        
        note = tkz.GetNextToken();
        
    }
    else {
        write( MSG_PARAMETER_ERROR, strlen( MSG_PARAMETER_ERROR ) );
        return;
    }

    CVSCPVariable variable;
    if ( 0 != gpobj->m_variables.find( name, variable ) ) {
        
        // Set value and encode as BASE64 when expected
        variable.setNote( note, true );
        
        // Update in database
        if ( !gpobj->m_variables.update( variable ) ) {
            write( MSG_VARIABLE_NO_SAVE, strlen ( MSG_VARIABLE_NO_SAVE ) );
            return;
        }
        
    }
    else {
        write( MSG_VARIABLE_NOT_DEFINED, strlen ( MSG_VARIABLE_NOT_DEFINED ) );
    }
    
    write(  MSG_OK, strlen ( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_Read
//

void TCPClientThread::handleVariable_Read( bool bOKResponse )
{
    wxString str;
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    m_pClientItem->m_currentCommand.Trim(false);
    m_pClientItem->m_currentCommand.Trim(true);

    CVSCPVariable variable;
    if ( 0 != gpobj->m_variables.find( m_pClientItem->m_currentCommand,variable ) ) {
        
        str = variable.getAsString( false );
        str = str + _("\r\n");
        write( str );

        write( MSG_OK, strlen ( MSG_OK ) );
        
    }
    else {
        write( MSG_VARIABLE_NOT_DEFINED, strlen ( MSG_VARIABLE_NOT_DEFINED ) );
    }
    
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_ReadVal
//

void TCPClientThread::handleVariable_ReadValue( bool bOKResponse )
{
    wxString str;
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    m_pClientItem->m_currentCommand.Trim(false);
    m_pClientItem->m_currentCommand.Trim(true);

    CVSCPVariable variable;
    if ( 0 != gpobj->m_variables.find( m_pClientItem->m_currentCommand, variable ) ) {
        
        variable.writeValueToString( str, true );
        str += _("\r\n");
        write(   str.mbc_str(), strlen( str.mbc_str() ) );

        write( MSG_OK, strlen ( MSG_OK ) );
        
    }
    else {
        write( MSG_VARIABLE_NOT_DEFINED, strlen ( MSG_VARIABLE_NOT_DEFINED ) );
    }
}


///////////////////////////////////////////////////////////////////////////////
// handleVariable_ReadNote
//

void TCPClientThread::handleVariable_ReadNote( bool bOKResponse )
{
    wxString str;
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    m_pClientItem->m_currentCommand.Trim(false);
    m_pClientItem->m_currentCommand.Trim(true);

    CVSCPVariable variable;
    if ( 0 != gpobj->m_variables.find( m_pClientItem->m_currentCommand,variable ) ) {
        
        variable.getNote( str, true );
        str = _("+OK - ") + str + _("\r\n");
        write(   str.mbc_str(), strlen( str.mbc_str() ) );

        write(  MSG_OK, strlen ( MSG_OK ) );
        
    }
    else {
        write(  MSG_VARIABLE_NOT_DEFINED, strlen ( MSG_VARIABLE_NOT_DEFINED ) );
    }
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_Reset
//

void TCPClientThread::handleVariable_Reset( void  )
{
    wxString str;
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    m_pClientItem->m_currentCommand.Trim(false);
    m_pClientItem->m_currentCommand.Trim(true);

    CVSCPVariable variable;
    
    if ( 0 != gpobj->m_variables.find( m_pClientItem->m_currentCommand, variable ) ) {
        
        variable.Reset();
        
        // Update in database
        if ( !gpobj->m_variables.update( variable ) ) {
            write( MSG_VARIABLE_NO_SAVE, strlen ( MSG_VARIABLE_NO_SAVE ) );
            return;
        }
        
    }
    else {
        write( MSG_VARIABLE_NOT_DEFINED, strlen ( MSG_VARIABLE_NOT_DEFINED ) );
    }

    write( MSG_OK, strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_ReadReset
//

void TCPClientThread::handleVariable_ReadReset( void )
{
    wxString str;
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    m_pClientItem->m_currentCommand.Trim(false);
    m_pClientItem->m_currentCommand.Trim(true);
    
    if ( m_pClientItem->CommandStartsWith(_("vscp.") ) ) {
        write( MSG_VARIABLE_NOT_STOCK, strlen ( MSG_VARIABLE_NOT_STOCK ) );
        return;
    }

    CVSCPVariable variable;
    
    if ( 0 != gpobj->m_variables.find( m_pClientItem->m_currentCommand, variable ) ) {
        
        variable.writeValueToString( str );
        str = _("+OK - ") + str + _("\r\n");
        write(   str.mbc_str(), strlen( str.mbc_str() ) );
    
        variable.Reset();
        
        // Update in database
        if ( !gpobj->m_variables.update( variable ) ) {
            write( MSG_VARIABLE_NO_SAVE, strlen ( MSG_VARIABLE_NO_SAVE ) );
            return;
        }
        
    }
    else {
        write( MSG_VARIABLE_NOT_DEFINED, strlen ( MSG_VARIABLE_NOT_DEFINED ) );
    }

    write( MSG_OK, strlen( MSG_OK ) );
}


///////////////////////////////////////////////////////////////////////////////
// handleVariable_Remove
//

void TCPClientThread::handleVariable_Remove( void )
{
    wxString str;
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    m_pClientItem->m_currentCommand.Trim(false);
    m_pClientItem->m_currentCommand.Trim(true);
    
    if ( m_pClientItem->CommandStartsWith(_("vscp.") ) ) {
        write(  MSG_VARIABLE_NOT_STOCK, strlen ( MSG_VARIABLE_NOT_STOCK ) );
        return;
    }
    
    if ( !gpobj->m_variables.remove( m_pClientItem->m_currentCommand ) ) {
        write(  MSG_VARIABLE_NOT_DEFINED, strlen ( MSG_VARIABLE_NOT_DEFINED ) );        
    }

    write(  MSG_OK, strlen( MSG_OK ) );
}


///////////////////////////////////////////////////////////////////////////////
// handleVariable_ReadRemove
//

void TCPClientThread::handleVariable_ReadRemove( void )
{
    wxString str;
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    m_pClientItem->m_currentCommand.Trim(false);
    m_pClientItem->m_currentCommand.Trim(true);
    
    if ( m_pClientItem->CommandStartsWith(_("vscp.") ) ) {
        write(  MSG_VARIABLE_NOT_STOCK, strlen ( MSG_VARIABLE_NOT_STOCK ) );
        return;
    }

    CVSCPVariable variable;
    if ( 0 != gpobj->m_variables.find( m_pClientItem->m_currentCommand, variable ) ) {
        
        variable.writeValueToString( str );
        str = _("+OK - ") + str + _("\r\n");
        write(   str.mbc_str(), strlen( str.mbc_str() ) );
    
        gpobj->m_variables.remove( variable );
        
    }
    else {
        write(  MSG_VARIABLE_NOT_DEFINED, strlen ( MSG_VARIABLE_NOT_DEFINED ) );
    }

    write(  MSG_OK, strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_Length
//

void TCPClientThread::handleVariable_Length( void )
{
    wxString str;
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    m_pClientItem->m_currentCommand.Trim(false);
    m_pClientItem->m_currentCommand.Trim(true);

    CVSCPVariable variable;
    if ( 0 != gpobj->m_variables.find( m_pClientItem->m_currentCommand, variable ) ) {
        
        str = wxString::Format( _("%zu"), variable.getLength() );
        str = str + _("\r\n");
        write(   str.mbc_str(), strlen( str.mbc_str() ) );
    
        gpobj->m_variables.remove( variable );
        
    }
    else {
        write(  MSG_VARIABLE_NOT_DEFINED, strlen ( MSG_VARIABLE_NOT_DEFINED ) );
    }

    write(  MSG_OK, strlen( MSG_OK ) );
}



///////////////////////////////////////////////////////////////////////////////
// handleVariable_Load
//

void TCPClientThread::handleVariable_Load( void )
{
    wxString path;  // Empty to load from default path
    gpobj->m_variables.loadFromXML( path );  // TODO add path + type

    write(  MSG_OK, strlen( MSG_OK ) );
}


///////////////////////////////////////////////////////////////////////////////
// handleVariable_Save
//

void TCPClientThread::handleVariable_Save( void )
{
    wxString path;
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    m_pClientItem->m_currentCommand.Trim(false);
    m_pClientItem->m_currentCommand.Trim(true);

    // Construct path to save to (always relative to root)
    // may not contain ".."
    path = gpobj->m_rootFolder;
    path += m_pClientItem->m_currentCommand;
    
    if ( wxNOT_FOUND != path.Find( _("..") ) ) {
        write(  MSG_INVALID_PATH, strlen( MSG_INVALID_PATH ) );
        return;
    }

    gpobj->m_variables.save( path );

    write(  MSG_OK, strlen( MSG_OK ) );
}




// ****************************************************************************



///////////////////////////////////////////////////////////////////////////////
// handleClientDm
//

void TCPClientThread::handleClientDm( void )
{
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    gpobj->logMsg ( m_pClientItem->m_currentCommand, DAEMON_LOGMSG_NORMAL );

    m_pClientItem->m_currentCommand.Trim(false);
    m_pClientItem->m_currentCommand.Trim(true);

    if ( m_pClientItem->CommandStartsWith(_("enable") ) ) {
        handleDM_Enable();
    }
    else if (m_pClientItem->CommandStartsWith(_("disable") ) ) {
        handleDM_Enable();
    }
    else if ( m_pClientItem->CommandStartsWith(_("list") ) ) {
        handleDM_List();
    }
    else if ( m_pClientItem->CommandStartsWith(_("add") ) ) {
        handleDM_Add();
    }
    else if ( m_pClientItem->CommandStartsWith(_("delete") ) ) {
        handleDM_Delete();
    }
    else if ( m_pClientItem->CommandStartsWith(_("reset") ) ) {
        handleDM_Reset();
    }
    else if ( m_pClientItem->CommandStartsWith(_("trig") ) ) {
        handleDM_Trigger();
    }
    else if ( m_pClientItem->CommandStartsWith(_("clrtrig") ) ) {
        handleDM_ClearTriggerCount();
    }
    else if ( m_pClientItem->CommandStartsWith(_("clrerr") ) ) {
        handleDM_ClearErrorCount();
    }
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_Enable
//

void TCPClientThread::handleDM_Enable( void )
{
    unsigned short pos;
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    if ( m_pClientItem->CommandStartsWith(_("all") ) ) {

        gpobj->m_dm.m_mutexDM.Lock();

        DMLIST::iterator iter;
        for (iter = gpobj->m_dm.m_DMList.begin(); iter != gpobj->m_dm.m_DMList.end(); ++iter)
        {
            dmElement *pDMItem = *iter;
            pDMItem->enableRow();
        }

        gpobj->m_dm.m_mutexDM.Unlock();

    }
    else {

        // Get the position
        pos = 0;

        gpobj->m_dm.m_mutexDM.Lock();

        if ( pos > ( gpobj->m_dm.m_DMList.GetCount() - 1 ) ) {
            write(   MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            gpobj->m_dm.m_mutexDM.Unlock();
            return;
        }

        DMLIST::compatibility_iterator node = gpobj->m_dm.m_DMList.Item( pos );
        ( node->GetData() )->enableRow();
        gpobj->m_dm.m_mutexDM.Unlock();

    }

    write(  MSG_OK, strlen( MSG_OK ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleDM_Disable
//

void TCPClientThread::handleDM_Disable( void )
{
    unsigned short pos;
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    if ( m_pClientItem->CommandStartsWith(_("all") ) ) {

        gpobj->m_dm.m_mutexDM.Lock();

        DMLIST::iterator iter;
        for (iter = gpobj->m_dm.m_DMList.begin();
            iter != gpobj->m_dm.m_DMList.end();
            ++iter )
        {
            dmElement *pDMItem = *iter;
            pDMItem->disableRow();
        }
        gpobj->m_dm.m_mutexDM.Unlock();

    }
    else {

        // Get the position
        wxStringTokenizer tkz( m_pClientItem->m_currentCommand, _(",") );
        while ( tkz.HasMoreTokens() ) {

            pos = vscp_readStringValue( tkz.GetNextToken() );

            gpobj->m_dm.m_mutexDM.Lock();

            if ( pos > ( gpobj->m_dm.m_DMList.GetCount() - 1 ) ) {
                write(   MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
                gpobj->m_dm.m_mutexDM.Unlock();
                return;
            }

            DMLIST::compatibility_iterator node = gpobj->m_dm.m_DMList.Item( pos );
            ( node->GetData() )->disableRow();

            gpobj->m_dm.m_mutexDM.Unlock();

        }

    }

    write(  MSG_OK, strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_List
//

void TCPClientThread::handleDM_List( void )
{
    // Valid commands at this point
    // dm list
    // dm list all
    // dm list *
    // dm list 1
    // dm list 1,2,3,4,98

    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    m_pClientItem->m_currentCommand.Trim(false);
    m_pClientItem->m_currentCommand.Trim(true);

    // if "list" add "all"
    if ( 0 == m_pClientItem->m_currentCommand.Length() ) {
        m_pClientItem->m_currentCommand = _("ALL");
    }

    else if ( m_pClientItem->CommandStartsWith(_("new") ) ||
                m_pClientItem->CommandStartsWith(_("*") ) ) {

        gpobj->m_dm.m_mutexDM.Lock();

        DMLIST::iterator iter;
        for (iter = gpobj->m_dm.m_DMList.begin();
                iter != gpobj->m_dm.m_DMList.end();
                ++iter) {

            dmElement *pDMItem = *iter;
            wxString strRow = pDMItem->getAsString();

            write(   strRow.mb_str(),
                                        strlen ( strRow.mb_str() ) );

        }

        gpobj->m_dm.m_mutexDM.Unlock();

    }
    else {

        // We have a list with specific rows  "list 1,8,9"
        //      first parse the argument to get the rows
        //WX_DEFINE_ARRAY_INT( int, ArrayOfSortedInts );
        wxArrayInt rowArray;
        wxStringTokenizer tok( m_pClientItem->m_currentCommand, _(",") );
        while ( tok.HasMoreTokens() ) {

            int n = vscp_readStringValue( tok.GetNextToken() );
            rowArray.Add( n );

        }

    }


    write(  MSG_OK, strlen ( MSG_OK ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleDM_Add
//

void TCPClientThread::handleDM_Add( void )
{
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;
    
    dmElement *pDMItem = new dmElement;

    wxStringTokenizer tkz( m_pClientItem->m_currentCommand, _(",") );

    // Priority
    pDMItem->m_vscpfilter.mask_priority = vscp_readStringValue( tkz.GetNextToken() );
    pDMItem->m_vscpfilter.filter_priority = vscp_readStringValue( tkz.GetNextToken() );

    // Class
    pDMItem->m_vscpfilter.mask_class = vscp_readStringValue( tkz.GetNextToken() );
    pDMItem->m_vscpfilter.filter_class = vscp_readStringValue( tkz.GetNextToken() );

    // Type
    pDMItem->m_vscpfilter.mask_type = vscp_readStringValue( tkz.GetNextToken() );
    pDMItem->m_vscpfilter.filter_type = vscp_readStringValue( tkz.GetNextToken() );

    // GUID
    wxString strGUID;
    strGUID =tkz.GetNextToken();
    vscp_getGuidFromStringToArray( pDMItem->m_vscpfilter.mask_GUID, strGUID );
    strGUID = tkz.GetNextToken();
    vscp_getGuidFromStringToArray( pDMItem->m_vscpfilter.filter_GUID, strGUID );

    // action code
    pDMItem->m_actionCode = vscp_readStringValue( tkz.GetNextToken() );

    // action parameters
    pDMItem->m_actionparam = tkz.GetNextToken();

    // add the DM row to the matrix
    gpobj->m_dm.m_mutexDM.Lock();
    gpobj->m_dm.addMemoryElement ( pDMItem );
    gpobj->m_dm.m_mutexDM.Unlock();

    write(  MSG_OK, strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_Delete
//

void TCPClientThread::handleDM_Delete( void )
{
    unsigned short pos;
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    if ( m_pClientItem->CommandStartsWith(_("all") ) ) {

        gpobj->m_dm.m_mutexDM.Lock();

        DMLIST::iterator iter;
        for (iter = gpobj->m_dm.m_DMList.begin();
            iter != gpobj->m_dm.m_DMList.end();
            ++iter) {
                dmElement *pDMItem = *iter;
                delete pDMItem;
        }
        gpobj->m_dm.m_DMList.Clear();
        gpobj->m_dm.m_mutexDM.Unlock();

    }
    else {

        // Get the position
        wxStringTokenizer tkz( m_pClientItem->m_currentCommand, _(",") );
        while ( tkz.HasMoreTokens() ) {

            pos = vscp_readStringValue( tkz.GetNextToken() );

            gpobj->m_dm.m_mutexDM.Lock();

            if ( pos > ( gpobj->m_dm.m_DMList.GetCount() - 1 ) ) {
                write(   MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
                gpobj->m_dm.m_mutexDM.Unlock();
                return;
            }

            DMLIST::compatibility_iterator node = gpobj->m_dm.m_DMList.Item( pos );
            dmElement *pDMItem = node->GetData();
            gpobj->m_dm.m_DMList.DeleteNode( node );    // Delete the node
            delete pDMItem;                                     // Delete the object

            gpobj->m_dm.m_mutexDM.Unlock();

        }

    }

    write(   MSG_OK, strlen ( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_Reset
//

void TCPClientThread::handleDM_Reset( void )
{
    gpobj->stopDaemonWorkerThread();
    gpobj->startDaemonWorkerThread();

    write(  MSG_OK, strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_Trigger
//

void TCPClientThread::handleDM_Trigger( void )
{
    unsigned short pos;
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    // Get the position
    wxStringTokenizer tkz( m_pClientItem->m_currentCommand, _(",") );
    while ( tkz.HasMoreTokens() ) {

        pos = vscp_readStringValue( tkz.GetNextToken() );

        gpobj->m_dm.m_mutexDM.Lock();

        if ( pos > ( gpobj->m_dm.m_DMList.GetCount() - 1 ) ) {
            write(   MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            gpobj->m_dm.m_mutexDM.Unlock();
            return;
        }

        DMLIST::compatibility_iterator node = gpobj->m_dm.m_DMList.Item( pos );
        dmElement *pDMItem = node->GetData();
        pDMItem->doAction( NULL );

        gpobj->m_dm.m_mutexDM.Unlock();

    }

    write(  MSG_OK, strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_ClearTriggerCount
//


void TCPClientThread::handleDM_ClearTriggerCount( void )
{
    unsigned short pos;
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    if ( m_pClientItem->CommandStartsWith(_("all") ) ) {

        gpobj->m_dm.m_mutexDM.Lock();

        DMLIST::iterator iter;
        for (iter = gpobj->m_dm.m_DMList.begin();
            iter != gpobj->m_dm.m_DMList.end();
            ++iter) {
                dmElement *pDMItem = *iter;
                pDMItem->m_triggCounter = 0;
        }

        gpobj->m_dm.m_mutexDM.Unlock();

    }
    else {

        // Get the position
        wxStringTokenizer tkz( m_pClientItem->m_currentCommand, _(",") );
        while ( tkz.HasMoreTokens() ) {

            pos = vscp_readStringValue( tkz.GetNextToken() );

            gpobj->m_dm.m_mutexDM.Lock();

            if ( pos > ( gpobj->m_dm.m_DMList.GetCount() - 1 ) ) {
                write(   MSG_PARAMETER_ERROR,
                    strlen ( MSG_PARAMETER_ERROR ) );
                gpobj->m_dm.m_mutexDM.Unlock();
                return;
            }

            DMLIST::compatibility_iterator node = gpobj->m_dm.m_DMList.Item( pos );
            dmElement *pDMItem = node->GetData();
            pDMItem->m_triggCounter = 0;

            gpobj->m_dm.m_mutexDM.Unlock();

        }

    }

    write(  MSG_OK, strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_ClearErrorCount
//


void TCPClientThread::handleDM_ClearErrorCount( void )
{
    unsigned short pos;
    
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    if ( m_pClientItem->CommandStartsWith(_("all") ) ) {

        gpobj->m_dm.m_mutexDM.Lock();

        DMLIST::iterator iter;
        for (iter = gpobj->m_dm.m_DMList.begin();
            iter != gpobj->m_dm.m_DMList.end();
            ++iter) {
                dmElement *pDMItem = *iter;
                pDMItem->m_errorCounter = 0;
        }

        gpobj->m_dm.m_mutexDM.Unlock();

    }
    else {

        // Get the position
        wxStringTokenizer tkz( m_pClientItem->m_currentCommand, _(",") );
        while ( tkz.HasMoreTokens() ) {

            pos = vscp_readStringValue( tkz.GetNextToken() );

            gpobj->m_dm.m_mutexDM.Lock();

            if ( pos > ( gpobj->m_dm.m_DMList.GetCount() - 1 ) ) {
                write(   MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
                gpobj->m_dm.m_mutexDM.Unlock();
                return;
            }

            DMLIST::compatibility_iterator node = gpobj->m_dm.m_DMList.Item( pos );
            dmElement *pDMItem = node->GetData();
            pDMItem->m_errorCounter = 0;

            gpobj->m_dm.m_mutexDM.Unlock();

        }

    }

    write(  MSG_OK, strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleClientList
//

void TCPClientThread::handleClientList( void )
{
    // TODO
    write(  MSG_OK, strlen ( MSG_OK ) );
}


///////////////////////////////////////////////////////////////////////////////
// handleClientDriver
//

void TCPClientThread::handleClientDriver( void )
{
    // TODO
}


///////////////////////////////////////////////////////////////////////////////
// handleClientHelp
//

void TCPClientThread::handleClientHelp( void )
{
    // Must be connected
    if ( STCP_CONN_STATE_CONNECTED != m_conn->conn_state ) return;

    m_pClientItem->m_currentCommand.Trim(true);
    m_pClientItem->m_currentCommand.Trim(false);

    if ( 0 == m_pClientItem->m_currentCommand.Length() ) {

        wxString str = _("Help for the VSCP tcp/ip interface\r\n");
                str += _("====================================================================\r\n");
                str += _("To get more information about a specific command issue 'HELP command'\r\n");
                str += _("+                 - Repeat last command.\r\n");
                str += _("+n                - Repeat command 'n' (0 is last).\r\n");
                str += _("++                - List repeatable commands.\r\n");
                str += _("NOOP              - No operation. Does nothing.\r\n");
                str += _("QUIT              - Close the connection.\r\n");
                str += _("USER 'username'   - Username for login. \r\n");
                str += _("PASS 'password'   - Password for login.  \r\n");
                str += _("CHALLENGE 'token' - Get session id.  \r\n");
                str += _("SEND 'event'      - Send an event.   \r\n");
                str += _("RETR 'count'      - Retrive n events from input queue.   \r\n");
                str += _("RCVLOOP           - Will retrieve events in an endless loop until "
                                              "the connection is closed by the client or QUITLOOP is sent.\r\n");
                str += _("QUITLOOP          - Terminate RCVLOOP.\r\n");
                str += _("CDTA/CHKDATA      - Check if there is data in the input queue.\r\n");
                str += _("CLRA/CLRALL       - Clear input queue.\r\n");
                str += _("STAT              - Get statistical information.\r\n");
                str += _("INFO              - Get status info.\r\n");
                str += _("CHID              - Get channel id.\r\n");
                str += _("SGID/SETGUID      - Set GUID for channel.\r\n");
                str += _("GGID/GETGUID      - Get GUID for channel.\r\n");
                str += _("VERS/VERSION      - Get VSCP daemon version.\r\n");
                str += _("SFLT/SETFILTER    - Set incoming event filter.\r\n");
                str += _("SMSK/SETMASK      - Set incoming event mask.\r\n");
                str += _("HELP [command]    - This command.\r\n");
                str += _("TEST              - Do test sequence. Only used for debugging.\r\n");
                str += _("SHUTDOWN          - Shutdown the daemon.\r\n");
                str += _("RESTART           - Restart the daemon.\r\n");
                str += _("DRIVER            - Driver manipulation.\r\n");
                str += _("FILE              - File handling.\r\n");
                str += _("UDP               - UDP.\r\n");
                str += _("REMOTE            - User handling.\r\n");
                str += _("INTERFACE         - Interface handling. \r\n");
                str += _("DM                - Decision Matrix manipulation.\r\n");
                str += _("VAR               - Variable handling. \r\n");
                str += _("WCYD/WHATCANYOUDO - Check server capabilities. \r\n");
                write(  (const char *)str.mbc_str(), str.Length() );
    }
    else if ( m_pClientItem->CommandStartsWith( _("+") ) ) {
        wxString str = _("'+' repeats the last given command.\r\n");
        write(  (const char *)str.mbc_str(), str.Length() );
    }
    else if ( m_pClientItem->CommandStartsWith( _("noop") ) ) {
        wxString str = _("'NOOP' Does absolutly nothing but giving a success in return.\r\n");
        write(  (const char *)str.mbc_str(), str.Length() );
    }
    else if ( m_pClientItem->CommandStartsWith( _("quit") ) ) {
        wxString str = _("'QUIT' Quit a session with the VSCP daemon and closes the m_connection.\r\n");
        write(  (const char *)str.mbc_str(), str.Length() );
    }
    else if ( m_pClientItem->CommandStartsWith( _("user") ) ) {
        wxString str = _("'USER' Used to login to the system together with PASS. Connection will be closed if bad credentials are given.\r\n");
        write(  (const char *)str.mbc_str(), str.Length() );
    }
    else if ( m_pClientItem->CommandStartsWith( _("pass") ) ) {
        wxString str = _("'PASS' Used to login to the system together with USER. Connection will be closed if bad credentials are given.\r\n");
        write(  (const char *)str.mbc_str(), str.Length() );
    }
    else if ( m_pClientItem->CommandStartsWith( _("quit") ) ) {
        wxString str = _("'QUIT' Quit a session with the VSCP daemon and closes the m_connection.\r\n");
        write(  (const char *)str.mbc_str(), str.Length() );
    }
    else if ( m_pClientItem->CommandStartsWith( _("send") ) ) {
        wxString str = _("'SEND event'.\r\nThe event is given as 'head,class,type,obid,datetime,time-stamp,GUID,data1,data2,data3....' \r\n");
        str += _("Normally set 'head' and 'obid' to zero. \r\nIf timestamp is set to zero it will be set by the server. \r\nIf GUID is given as '-' ");
        str += _("the GUID of the interface will be used. \r\nThe GUID should be given on the form MSB-byte:MSB-byte-1:MSB-byte-2. \r\n");
        write(  (const char *)str.mbc_str(), str.Length() );
    }
    else if ( m_pClientItem->CommandStartsWith( _("retr") ) ) {
        wxString str = _("'RETR count' - Retrieve one (if no argument) or 'count' event(s). ");
        str += _("Events are retrived on the form head,class,type,obid,datetime,time-stamp,GUID,data0,data1,data2,...........\r\n");
        write(  (const char *)str.mbc_str(), str.Length() );
    }
    else if ( m_pClientItem->CommandStartsWith( _("rcvloop") ) ) {
        wxString str = _("'RCVLOOP' - Enter the receive loop and receive events continously or until ");
        str += _("terminated with 'QUITLOOP'. Events are retrived on the form head,class,type,obid,time-stamp,GUID,data0,data1,data2,...........\r\n");
        write(  (const char *)str.mbc_str(), str.Length() );
    }
    else if ( m_pClientItem->CommandStartsWith( _("quitloop") ) ) {
        wxString str = _("'QUITLOOP' - End 'RCVLOOP' event receives.\r\n");
        write(  (const char *)str.mbc_str(), str.Length() );
    }
    else if ( m_pClientItem->CommandStartsWith( _("cdta") ) ||
                m_pClientItem->CommandStartsWith( _("chkdata") ) ) {
        wxString str = _("'CDTA' or 'CHKDATA' - Check if there is events in the input queue.\r\n");
        write(  (const char *)str.mbc_str(), str.Length() );
    }
    else if ( m_pClientItem->CommandStartsWith( _("clra") ) ||
                m_pClientItem->CommandStartsWith( _("clrall") ) ) {
        wxString str = _("'CLRA' or 'CLRALL' - Clear input queue.\r\n");
        write(  (const char *)str.mbc_str(), str.Length() );
    }
    else if ( m_pClientItem->CommandStartsWith( _("stat") ) ) {
        wxString str = _("'STAT' - Get statistical information.\r\n");
        write(  (const char *)str.mbc_str(), str.Length() );
    }
    else if ( m_pClientItem->CommandStartsWith( _("info") ) ) {
        wxString str = _("'INFO' - Get status information.\r\n");
        write(  (const char *)str.mbc_str(), str.Length() );
    }
    else if ( m_pClientItem->CommandStartsWith( _("chid") ) ||
                m_pClientItem->CommandStartsWith( _("getchid") ) ) {
        wxString str = _("'CHID' or 'GETCHID' - Get channel id.\r\n");
        write(  (const char *)str.mbc_str(), str.Length() );
    }
    else if ( m_pClientItem->CommandStartsWith( _("sgid") ) ||
                m_pClientItem->CommandStartsWith( _("setguid") ) ) {
        wxString str = _("'SGID' or 'SETGUID' - Set GUID for channel.\r\n");
        write(  (const char *)str.mbc_str(), str.Length() );
    }
    else if ( m_pClientItem->CommandStartsWith( _("ggid") ) ||
                m_pClientItem->CommandStartsWith( _("getguid") ) ) {
        wxString str = _("'GGID' or 'GETGUID' - Get GUID for channel.\r\n");
        write(  (const char *)str.mbc_str(), str.Length() );
    }
    else if ( m_pClientItem->CommandStartsWith( _("vers") ) ||
                m_pClientItem->CommandStartsWith( _("version") ) ) {
        wxString str = _("'VERS' or 'VERSION' - Get version of VSCP daemon.\r\n");
        write(  (const char *)str.mbc_str(), str.Length() );
    }
    else if ( m_pClientItem->CommandStartsWith( _("sflt") ) ||
                m_pClientItem->CommandStartsWith( _("setfilter") ) ) {
        wxString str = _("'SFLT' or 'SETFILTER' - Set filter for channel. ");
        str += _("The format is 'filter-priority, filter-class, filter-type, filter-GUID' \r\n");
        str += _("Example:  \r\nSETFILTER 1,0x0000,0x0006,ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00\r\n");
        write(  (const char *)str.mbc_str(), str.Length() );
    }
    else if ( m_pClientItem->CommandStartsWith( _("smsk") ) ||
                m_pClientItem->CommandStartsWith( _("setmask") ) ) {
        wxString str = _("'SMSK' or 'SETMASK' - Set mask for channel. ");
        str += _("The format is 'mask-priority, mask-class, mask-type, mask-GUID' \r\n");
        str += _("Example:  \r\nSETMASK 0x0f,0xffff,0x00ff,ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00 \r\n");
        write(  (const char *)str.mbc_str(), str.Length() );
    }
    else if ( m_pClientItem->CommandStartsWith( _("help") ) ) {
        wxString str = _("'HELP [command]' This command. Gives help about available commands and the usage.\r\n");
        write(  (const char *)str.mbc_str(), str.Length() );
    }
    else if ( m_pClientItem->CommandStartsWith( _("test") ) ) {
        wxString str = _("'TEST [sequency]' Test command for debugging.\r\n");
        write(  (const char *)str.mbc_str(), str.Length() );
    }
    else if ( m_pClientItem->CommandStartsWith( _("shutdown") ) ) {
        wxString str = _("'SHUTDOWN' Shutdown the daemon.\r\n");
        write(  (const char *)str.mbc_str(), str.Length() );
    }
    else if ( m_pClientItem->CommandStartsWith( _("restart") ) ) {
        wxString str = _("'RESTART' Restart the daemon.\r\n");
        write(  (const char *)str.mbc_str(), str.Length() );
    }
    else if ( m_pClientItem->CommandStartsWith( _("driver") ) ) {
        wxString str = _("'DRIVER' Handle (load/unload/update/start/stop) Level I/Level II drivers.\r\n");
        str += _("'DRIVER install package' .\r\n");
        str += _("'DRIVER uninstall package' .\r\n");
        str += _("'DRIVER upgrade package' .\r\n");
        str += _("'DRIVER start package' .\r\n");
        str += _("'DRIVER stop package' .\r\n");
        str += _("'DRIVER reload package' .\r\n");
        write(  (const char *)str.mbc_str(), str.Length() );
    }
    else if ( m_pClientItem->CommandStartsWith( _("file") ) ) {
        wxString str = _("'FILE' Handle daemon files.\r\n");
        str += _("'FILE dir'.\r\n");
        str += _("'FILE copy'.\r\n");
        str += _("'FILE move'.\r\n");
        str += _("'FILE delete'.\r\n");
        str += _("'FILE list'.\r\n");
        write(  (const char *)str.mbc_str(), str.Length() );
    }
    else if ( m_pClientItem->CommandStartsWith( _("udp") ) ) {
        wxString str = _("'UDP' Handle UDP interface.\r\n");
        str += _("'UDP enable'.\r\n");
        str += _("'UDP disable' .\r\n");
        write(  (const char *)str.mbc_str(), str.Length() );
    }
    else if ( m_pClientItem->CommandStartsWith( _("remote") ) ) {
        wxString str = _("'REMOTE' User management.\r\n");
        str += _("'REMOTE list'.\r\n");
        str += _("'REMOTE add 'username','MD5 password','from-host(s)','access-right-list','event-list','filter','mask''. Add a user.\r\n");
        str += _("'REMOTE remove username'.\r\n");
        str += _("'REMOTE privilege 'username','access-right-list''.\r\n");
        str += _("'REMOTE password 'username','MD5 for password' '.\r\n");
        str += _("'REMOTE host-list 'username','host-list''.\r\n");
        str += _("'REMOTE event-list 'username','event-list''.\r\n");
        str += _("'REMOTE filter 'username','filter''.\r\n");
        str += _("'REMOTE mask 'username','mask''.\r\n");
        write(  (const char *)str.mbc_str(), str.Length() );
    }
    else if ( m_pClientItem->CommandStartsWith( _("interface") ) ) {
        wxString str = _("'INTERFACE' Handle interfaces on the daemon.\r\n");
        str += _("'INTERFACE list'.\r\n");
        str += _("'INTERFACE close'.\r\n");
        write(  (const char *)str.mbc_str(), str.Length() );
    }
    else if ( m_pClientItem->CommandStartsWith( _("dm") ) ) {
        wxString str = _("'DM' Handle decision matrix on the daemon.\r\n");
        str += _("'DM enable'.\r\n");
        str += _("'DM disable'.\r\n");
        str += _("'DM list'.\r\n");
        str += _("'DM add'.\r\n");
        str += _("'DM delete'.\r\n");
        str += _("'DM reset'.\r\n");
        str += _("'DM clrtrig'.\r\n");
        str += _("'DM clrerr'.\r\n");
        str += _("'DM load'.\r\n");
        str += _("'DM save'.\r\n");
        write(  (const char *)str.mbc_str(), str.Length() );
    }
    else if ( m_pClientItem->CommandStartsWith( _("var") ) ||
                m_pClientItem->CommandStartsWith( _("variable") ) ) {
        wxString str = _("'VARIABLE' Handle variables on the daemon.\r\n");
        str += _("'VARIABLE list <regular-expression>'.\r\n");        
        str += _("'VARIABLE read <variable-name>'.\r\n");
        str += _("'VARIABLE readvalue <variable-name>'.\r\n");
        str += _("'VARIABLE readnote <variable-name>'.\r\n");
        str += _("'VARIABLE write <variable-name> <variable>'.\r\n");
        str += _("'VARIABLE writevalue <variable-name> <value>'.\r\n");
        str += _("'VARIABLE writenote <variable-name>' <note>.\r\n");
        str += _("'VARIABLE reset <variable-name>'.\r\n");
        str += _("'VARIABLE readreset <variable-name>'.\r\n");
        str += _("'VARIABLE remove <variable-name>'.\r\n");
        str += _("'VARIABLE readremove <variable-name>'.\r\n");
        str += _("'VARIABLE length <variable-name>'.\r\n");
        str += _("'VARIABLE save <path> <selection>'.\r\n");
        write(  (const char *)str.mbc_str(), str.Length() );
    }

    write( MSG_OK, strlen(MSG_OK) );
    return;
}



