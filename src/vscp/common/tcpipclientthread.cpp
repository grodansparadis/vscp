// tcpipclientthread.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (http://www.vscp.org)
//
// Copyright (C) 2000-2014 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
//
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//

#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"
#include <wx/listimpl.cpp>
#include <wx/tokenzr.h>
#include <wx/stdpaths.h>

#ifndef DWORD
#define DWORD unsigned long
#endif

#include "vscp.h"
#include "tcpipclientthread.h"
#include "canal_win32_ipc.h"
#include "canal_macro.h"
#include "../common/canal.h"
#include "vscphelper.h"
#include "../../common/dllist.h"
#include "../../common/md5.h"
#include "version.h"
#include "controlobject.h"


WX_DEFINE_LIST(TCPCLIENTS);


///////////////////////////////////////////////////////////////////////////////
// TcpClientListenThread
//
// This thread listens for conection on a TCP socket and starts a new thread
// to handle client requests
//

TcpClientListenThread::TcpClientListenThread()
: wxThread(wxTHREAD_JOINABLE)
{
    //OutputDebugString( "TCP ClientThread: Create");
    m_bQuit = false;
    m_pCtrlObject = NULL;
    m_numClients = 0;
}


TcpClientListenThread::~TcpClientListenThread()
{

}


///////////////////////////////////////////////////////////////////////////////
// Entry
//

void *TcpClientListenThread::Entry()
{
    //OutputDebugString( "TCP ListenThread: Start");
    wxSocketServer *server;

	// Check pointers
	if ( NULL == m_pCtrlObject ) return NULL;

    // Create the address - defaults to localhost:0 initially
    wxIPV4address addr;
    addr.AnyAddress();	// TODO - change to multiple channels
    addr.Service ( m_pCtrlObject->m_tcpport );

	// Normally listens on all interfaces of a multi interface machine
	// by specifying an interface it will jnstead listen on that
	// specific interface. Default is to listen on all.
	if ( 0 != m_pCtrlObject->m_strTcpInterfaceAddress.Length() ) {
		addr.Hostname( m_pCtrlObject->m_strTcpInterfaceAddress );
	}

    // Create the socket
    server = new wxSocketServer( addr, wxSOCKET_BLOCK | wxSOCKET_REUSEADDR );

    m_pCtrlObject->logMsg( _T ( "TCP ClientListenThread: Start.\n" ), DAEMON_LOGMSG_INFO );

    if ( server->Ok() ) {

        wxSocketBase *pserv;
        while ( !TestDestroy() &!m_bQuit ) {

            // Remove dead clients - TODO

            // Wait for a connection
            if ( NULL == ( pserv = server->Accept( false ) ) ) {
                Sleep ( 200 );
                continue;
            }

            TcpClientThread *pClientThread =
                new TcpClientThread( wxTHREAD_JOINABLE );                                                                          

            if ( NULL != pClientThread ) {
				
				// Add to tcp client queue
                m_tcpclients.Append( pClientThread );

                pClientThread->Create();

                // Do we have a connection
                pClientThread->m_pClientSocket = pserv;
                pClientThread->m_pClientSocket->SetFlags( wxSOCKET_BLOCK | wxSOCKET_REUSEADDR  );

                // Another client
                m_numClients++;

                // Start the client
                pClientThread->m_pCtrlObject = m_pCtrlObject;
                pClientThread->m_pmumClients = &m_numClients;
                pClientThread->SetPriority( WXTHREAD_MAX_PRIORITY );
                pClientThread->Run();

            } // client thread
        } // while
    } // socket ok

    delete server;

    m_pCtrlObject->logMsg ( _T ( "TCP ClientThread: Quit.\n" ), DAEMON_LOGMSG_INFO );

    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void TcpClientListenThread::OnExit()
{
    TCPCLIENTS::iterator iter;

    // First stage - tell clients to terminate
    for (iter = m_tcpclients.begin(); iter != m_tcpclients.end(); ++iter) {
        TcpClientThread *pThread = *iter;
        if ( ( NULL != pThread )  ) {
            pThread->m_bQuit = true;
        }
    }

    // Second stage - Wait until they terminates
    for (iter = m_tcpclients.begin(); iter != m_tcpclients.end(); ++iter) {
        TcpClientThread *pThread = *iter;
        if ( ( NULL != pThread )  ) {
            pThread->Wait();
            delete pThread;
        }
    }

    m_tcpclients.Clear();

}




///////////////////////////////////////////////////////////////////////////////
//****************************************************************************
///////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////
// TcpClientThread
//
// This thread communicates with a specific client.
//

TcpClientThread::TcpClientThread( wxThreadKind kind )
    : wxThread ( kind )
{
    m_bQuit = false;

    m_pCtrlObject = NULL;
    m_pClientSocket = NULL;		
    m_pUserItem = NULL;             // No user
    m_bOK = false;
    m_bVerified = false;            // No user verified
    m_bUsername = false;            // No username entered
    m_bRun = true;                  // Yes run to the hills
    m_bBinaryMode = false;          // Standard ,mode
}


TcpClientThread::~TcpClientThread()
{
    m_pCtrlObject = NULL;           // Mark us as deleted
}


///////////////////////////////////////////////////////////////////////////////
// Entry
//

void *TcpClientThread::Entry()
{
    char rbuf[ 2048 ];
    wxString wxstr;
    int pos4lf;

    // Must have a valid pointer to the control object
    if ( NULL == m_pCtrlObject ) return NULL;

    // Must have a socket
    if ( NULL == m_pClientSocket ) return NULL;

    // Must be connected
    if ( m_pClientSocket->IsDisconnected() ) return NULL;

    m_pCtrlObject->logMsg(_T("TCP ClientThread: Start.\n"), DAEMON_LOGMSG_INFO);

    // We need to create a clientobject and add this object to the list
    m_pClientItem = new CClientItem;
    if ( NULL == m_pClientItem ) {
        return NULL;
    }

    //OutputDebugString( "TCP ClientThread: Start all pointers OK");

    // This is now an active Client
    m_pClientItem->m_bOpen = true; 
    m_pClientItem->m_type =  CLIENT_ITEM_INTERFACE_TYPE_CLIENT_TCPIP;
    m_pClientItem->m_strDeviceName = _("Remote TCP/IP Client connected at ");
    wxDateTime now = wxDateTime::Now(); 
    m_pClientItem->m_strDeviceName += now.FormatISODate();
    m_pClientItem->m_strDeviceName += _(" ");
    m_pClientItem->m_strDeviceName += now.FormatISOTime();

    // Add the client to the Client List
    m_pCtrlObject->m_wxClientMutex.Lock();
    m_pCtrlObject->addClient( m_pClientItem );
    m_pCtrlObject->m_wxClientMutex.Unlock();

    m_pClientSocket->SetTimeout( 1 );
    //m_pClientSocket->SetFlags( wxSOCKET_BLOCK | wxSOCKET_WAITALL );
    m_pClientSocket->SetFlags( wxSOCKET_BLOCK );


    if ( *m_pmumClients > VSCP_TCP_MAX_CLIENTS ) {

        m_pClientSocket->Write( MSG_MAX_NUMBER_OF_CLIENTS,
            strlen ( MSG_MAX_NUMBER_OF_CLIENTS ) );
        m_bRun = false;

    }
    else {
        // Send welcome message
        wxString str = _(MSG_WELCOME);
        str += _("+OK Version: ");
        str += _(VSCPD_DISPLAY_VERSION);
        str += _("\r\n");
		str += _("+OK ");
		str += _(VSCPD_COPYRIGHT);
        //str += _(MSG_COPYRIGHT);
        str += _(MSG_OK);

        m_pClientSocket->Write( str.mb_str(), str.Length() );
    }


    // Clear the filter (Allow everything )
    clearVSCPFilter( &m_pClientItem->m_filterVSCP );


    ///////////
    // LOOP  //
    ///////////


    wxString wxLastCommand;
    while ( !TestDestroy() && m_bRun && !m_bQuit ) {

        // Check if command already in buffer
        if ( wxNOT_FOUND == ( pos4lf = wxstr.Find ( (const char)0x0a ) ) ) {
            
            // Read new data
            memset( rbuf, 0, sizeof( rbuf ) );                  // nil rbuf
            m_pClientSocket->Read ( rbuf, sizeof ( rbuf ) );    

            if ( m_pClientSocket->Error() &&
                ( wxSOCKET_NOERROR != ( m_err = m_pClientSocket->LastError() ) ) ) {
                switch ( m_err ) {

                    case wxSOCKET_IOERR:
                    case wxSOCKET_INVSOCK:
                        m_bRun = false;	// socket error
                        break;

                    case wxSOCKET_TIMEDOUT:
                        wxMilliSleep( 200 );

                        continue;
                        break;

                }

            }
            else if ( 0 == m_pClientSocket->LastCount() ) {
                m_bRun = false; // No connection
            }
            else {
                rbuf[ m_pClientSocket->LastCount() ] = 0;
                wxstr += wxString ( rbuf, wxConvUTF8 );
            }
            /*

            if ( wxSOCKET_TIMEDOUT == m_err ) {
                continue;
            }
            else {
                m_bRun = false;
                m_pClientSocket->Close();
            }

            }
            else if ( 0 == m_pClientSocket->LastCount() ) {
                m_bRun = false;
                m_pClientSocket->Close();
            }
            else {
                rbuf[ m_pClientSocket->LastCount() ] = 0;
                wxstr += wxString ( rbuf, wxConvUTF8 );
            }

            */
        }


        // * * *  Check for a command  * * *

        else {
            
            m_bOK = true;

            m_wxcmd = wxstr.Mid ( 0, pos4lf );
            //wxLogDebug( _("Command = ") + m_wxcmd );
            m_wxcmdUC = m_wxcmd.Upper();
            m_wxcmd.Trim();
            m_wxcmd.Trim( false );
            m_wxcmdUC.Trim();
            m_wxcmdUC.Trim( false );
            wxstr = wxstr.Mid ( pos4lf + 1 );
            //wxLogDebug( _("Argument = ") + m_wxcmdUC );


            // *********************************************************************
            //                                 QUIT
            // *********************************************************************
            if ( 0 == m_wxcmdUC.Find ( _( "QUIT" ) ) ) {
                m_pClientSocket->Write ( MSG_GOODBY, strlen ( MSG_GOODBY ) );
                break;
            }

            //*********************************************************************
            //                            No Operation
            //*********************************************************************
            else if ( 0 == m_wxcmdUC.Find ( _( "NOOP" ) ) ) {
                m_pClientSocket->Write ( MSG_OK, strlen ( MSG_OK ) );
            }

            //*********************************************************************
            //                             Send event
            //*********************************************************************
            else if ( 0 == m_wxcmdUC.Find ( _( "SEND " ) ) ) {
                if ( checkPrivilege( 4 ) ) handleClientSend();
            }

            //*********************************************************************
            //                            Read event
            //*********************************************************************
            else if ( 0 == m_wxcmdUC.Find ( _( "RETR" ) ) ) {
                if ( checkPrivilege( 2 ) ) handleClientReceive();
            }

            //*********************************************************************
            //                            Data Available
            //*********************************************************************
            else if ( 0 == m_wxcmdUC.Find ( _( "CDTA" ) ) ) {
                if ( checkPrivilege( 1 ) ) handleClientDataAvailable();
            }


            //*********************************************************************
            //                          Clear input queue
            //*********************************************************************
            else if ( 0 == m_wxcmdUC.Find ( _( "CLRA" ) ) ) {
                if ( checkPrivilege( 1 ) ) handleClientClearInputQueue();
            }


            //*********************************************************************
            //                           Get Statistics
            //*********************************************************************
            else if ( 0 == m_wxcmdUC.Find ( _( "STAT" ) ) ) {
                if ( checkPrivilege( 1 ) ) handleClientGetStatistics();
            }

            //*********************************************************************
            //                            Get Status
            //*********************************************************************
            else if ( 0 == m_wxcmdUC.Find ( _( "INFO" ) ) ) {
                if ( checkPrivilege( 1 ) ) handleClientGetStatus();
            }

            //*********************************************************************
            //                           Get Channel ID
            //*********************************************************************
            else if ( 0 == m_wxcmdUC.Find ( _( "CHID" ) ) ) {
                if ( checkPrivilege( 1 ) ) handleClientGetChannelID();
            }

            //*********************************************************************
            //                          Set Channel GUID
            //*********************************************************************
            else if ( 0 == m_wxcmdUC.Find ( _( "SGID" ) ) ) {
                if ( checkPrivilege( 6 ) ) handleClientSetChannelGUID();
            }

            //*********************************************************************
            //                          Get Channel GUID
            //*********************************************************************
            else if ( 0 == m_wxcmdUC.Find ( _( "GGID" ) ) ) {
                if ( checkPrivilege( 1 ) ) handleClientGetChannelGUID();
            }

            //*********************************************************************
            //                             Get Version
            //*********************************************************************
            else if ( 0 == m_wxcmdUC.Find ( _( "VERS" ) ) ) {
                handleClientGetVersion();
            }

            //*********************************************************************
            //                             Set Filter
            //*********************************************************************
            else if ( 0 == m_wxcmdUC.Find ( _( "SFLT" ) ) ) {
                if ( checkPrivilege( 4 ) ) handleClientSetFilter();
            }

            //*********************************************************************
            //                             Set Mask
            //*********************************************************************
            else if ( 0 == m_wxcmdUC.Find ( _( "SMSK" ) ) ) {
                if ( checkPrivilege( 4 ) ) handleClientSetMask();
            }

            //*********************************************************************
            //                               Username
            //*********************************************************************
            else if ( 0 == m_wxcmdUC.Find ( _( "USER " ) ) ) {
                handleClientUser();
            }

            //*********************************************************************
            //                             Password
            //*********************************************************************
            else if ( 0 == m_wxcmdUC.Find ( _( "PASS " ) ) ) {
                if ( !handleClientPassword() ) {
                    break; // Disconnect
                }
            }


            //*********************************************************************
            //                        + (repeat last command)
            //*********************************************************************
            else if ( 0 == m_wxcmdUC.Find ( _( "+" ) ) ) {
                // Repeat last command
                wxstr = wxLastCommand + _("\n") + wxstr;
                m_wxcmdUC = wxLastCommand;
                continue;
            }

            //*********************************************************************
            //                               Rcvloop
            //*********************************************************************
            else if ( 0 == m_wxcmdUC.Find ( _( "RCVLOOP" ) ) ) {
                if ( checkPrivilege( 2 ) ) handleClientRcvLoop();
            }

            //*********************************************************************
            //                              Help
            //*********************************************************************
            else if ( 0 == m_wxcmdUC.Find ( _( "HELP" ) ) ) {
                handleClientHelp();
            }

            //*********************************************************************
            //                              Restart
            //*********************************************************************
            else if ( 0 == m_wxcmdUC.Find ( _( "RESTART" ) ) ) {
                if ( checkPrivilege( 15 ) ) handleClientRestart();
            }

            //*********************************************************************
            //                              Driver
            //*********************************************************************
            else if ( 0 == m_wxcmdUC.Find ( _( "DRIVER " ) ) ) {
                if ( checkPrivilege( 15 ) ) handleClientDriver();
            }


            //*********************************************************************
            //                               DM
            //*********************************************************************
            else if ( 0 == m_wxcmdUC.Find ( _( "DM " ) ) ) {
                if ( checkPrivilege( 15 ) ) handleClientDm();
            }

            //*********************************************************************
            //                             Variable
            //*********************************************************************
            else if ( 0 == m_wxcmdUC.Find ( _( "VARIABLE " ) ) ) {
                if ( checkPrivilege( 15 ) ) handleClientVariable();
            }

            //*********************************************************************
            //                               File
            //*********************************************************************
            else if ( 0 == m_wxcmdUC.Find ( _( "FILE " ) ) ) {
                if ( checkPrivilege( 15 ) ) handleClientFile();
            }

            //*********************************************************************
            //                               UDP
            //*********************************************************************
            else if ( 0 == m_wxcmdUC.Find ( _( "UDP " ) ) ) {
                if ( checkPrivilege( 15 ) ) handleClientUdp();
            }

            //*********************************************************************
            //                         Client/interface
            //*********************************************************************
            else if ( 0 == m_wxcmdUC.Find ( _( "CLIENT " ) ) ) {
                m_wxcmdUC = m_wxcmdUC.Right( m_wxcmdUC.Length()-7 ); // Remove "CLIENT "
                if ( checkPrivilege( 15 ) ) handleClientInterface();
            }
            else if ( 0 == m_wxcmdUC.Find ( _( "INTERFACE " ) ) ) {
                m_wxcmdUC = m_wxcmdUC.Right( m_wxcmdUC.Length()-10 ); // Remove "INTERFACE "
                if ( checkPrivilege( 15 ) ) handleClientInterface();
            }

            //*********************************************************************
            //                               User
            //*********************************************************************
            else if ( 0 == m_wxcmdUC.Find ( _( "USER " ) ) ) {
                handleClientUser();
            }

            //*********************************************************************
            //                               Test
            //*********************************************************************
            else if ( 0 == m_wxcmdUC.Find ( _( "TEST" ) ) ) {
                if ( checkPrivilege( 15 ) ) handleClientTest();
            }


            //*********************************************************************
            //                               Shutdown
            //*********************************************************************
            else if ( 0 == m_wxcmdUC.Find ( _( "SHUTDOWN" ) ) ) {
                if ( checkPrivilege( 15 ) ) handleClientShutdown();
            }


            //*********************************************************************
            //                               Que?
            //*********************************************************************
            else {

                m_pClientSocket->Write ( MSG_UNKNOWN_COMMAND,
                    strlen ( MSG_UNKNOWN_COMMAND ) );
            }

            wxLastCommand = m_wxcmdUC;


        } // command

    } // while

    // Remove messages in the client queues
    m_pCtrlObject->m_wxClientMutex.Lock();
    m_pCtrlObject->removeClient ( m_pClientItem );
    m_pCtrlObject->m_wxClientMutex.Unlock();

    ( *m_pmumClients )--; // One client less

    m_pClientSocket->Close();
    m_pClientSocket->Destroy();

    m_pCtrlObject->logMsg ( _T ( "TCP ClientThread: Quit.\n" ), DAEMON_LOGMSG_INFO );

    m_bQuit = true;

    return NULL;
}



///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void TcpClientThread::OnExit()
{
    ;
}


///////////////////////////////////////////////////////////////////////////////
// isVerified
//

bool TcpClientThread::isVerified( void )
{ 
    // Must be accredited to do this
    if ( !m_bVerified ) {
        m_pClientSocket->Write ( MSG_NOT_ACCREDITED,
            strlen ( MSG_NOT_ACCREDITED ) );
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// checkPrivilege
//

bool TcpClientThread::checkPrivilege( unsigned char reqiredPrivilege )
{
    // Must be loged on
    if ( !m_bVerified ) {
        m_pClientSocket->Write ( MSG_NOT_ACCREDITED,
            strlen ( MSG_NOT_ACCREDITED ) );
        return false;
    }

    if ( NULL == m_pUserItem ) {
        m_pClientSocket->Write ( MSG_NOT_ACCREDITED,
            strlen ( MSG_NOT_ACCREDITED ) );
        return false;
    }

    if ( (m_pUserItem->m_userRights & 0xf) < reqiredPrivilege ) {
        m_pClientSocket->Write ( MSG_LOW_PRIVILEGE_ERROR,
            strlen ( MSG_LOW_PRIVILEGE_ERROR ) );
        return false;	
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientSend
//

void TcpClientThread::handleClientSend ( void )
{
    bool bSent = false;
    vscpEvent event;
    m_bOK = true;

    // Must be accredited to do this
    if ( !m_bVerified ) {
        m_pClientSocket->Write ( MSG_NOT_ACCREDITED,
            strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    wxString str = m_wxcmd.Right( m_wxcmd.Length() - 5 );
    wxStringTokenizer tkz( str, _(",") );

    // Get head
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        event.head = readStringValue( str );
    }
    else {
        m_bOK = false;
        m_pClientSocket->Write ( MSG_PARAMETER_ERROR,
            strlen ( MSG_PARAMETER_ERROR ) );
        return;	
    }

    // Get Class
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        event.vscp_class = readStringValue( str );
    }
    else {
        m_bOK = false;
        m_pClientSocket->Write ( MSG_PARAMETER_ERROR,
            strlen ( MSG_PARAMETER_ERROR ) );
        return;	
    }
    
    // Get Type
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        event.vscp_type = readStringValue( str );
    }
    else {
        m_bOK = false;
        m_pClientSocket->Write ( MSG_PARAMETER_ERROR,
            strlen ( MSG_PARAMETER_ERROR ) );
        return;	
    }

    // Get OBID  -  Kept here to be compatible with receive
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        event.obid = readStringValue( str );
    }
    else {
        m_bOK = false;
        m_pClientSocket->Write ( MSG_PARAMETER_ERROR,
            strlen ( MSG_PARAMETER_ERROR ) );
        return;	
    }

    // Get Timestamp
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        event.timestamp = readStringValue( str );
        if ( !event.timestamp ) {
#ifdef WIN32
            event.timestamp = GetTickCount();
#else
			struct timespec ts;
			clock_gettime(CLOCK_MONOTONIC, &ts);
			event.timestamp = (unsigned long)ts.tv_sec * 1000000 + ts.tv_nsec/1000;  
#endif
        }
    }
    else {
        m_bOK = false;
        m_pClientSocket->Write ( MSG_PARAMETER_ERROR,
            strlen ( MSG_PARAMETER_ERROR ) );
        return;	
    }

    // Get GUID
    wxString strGUID;
    if ( tkz.HasMoreTokens() ) {
        strGUID = tkz.GetNextToken();
    }
    else {
        m_bOK = false;
        m_pClientSocket->Write( MSG_PARAMETER_ERROR,
								strlen ( MSG_PARAMETER_ERROR ) );
        return;	
    }

    // Handle data
    event.sizeData = 0;
    char data[ 512 ];
    while ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        data[ event.sizeData ] = readStringValue( str );
        event.sizeData++;
    }

    vscpEvent *pEvent = new vscpEvent;		// Create new VSCP Event
    if ( NULL != pEvent ) {

        // Check if i/f GUID should be used
        if ( ( '-' == strGUID[0] ) || isGUIDEmpty( event.GUID ) ) {
            // Copy in the i/f GUID
            m_pClientItem->m_guid.writeGUID( event.GUID );
        }
        else {
            getGuidFromString( &event, strGUID );
        }

        // Copy event
        memcpy ( pEvent, &event, sizeof ( vscpEvent ) );

        // Save the originating clients id so
        // this client don't get the message back
        pEvent->obid = m_pClientItem->m_clientID;

        wxString dbgStr = 
            wxString::Format( _(" obid = %d clientid = %d"), 
            pEvent->obid,  
            m_pClientItem->m_clientID ); 
        m_pCtrlObject->logMsg( dbgStr, DAEMON_LOGMSG_INFO );

        // And data...
        if ( pEvent->sizeData > 0 ) {

            // Copy in data
            pEvent->pdata = new uint8_t[ event.sizeData ];
            if ( NULL != pEvent->pdata ) {
                memcpy ( pEvent->pdata, data ,pEvent->sizeData );
            }

        }
        else {
            // No data
            pEvent->pdata = NULL;
        }

        // Level II events between 512-1023 is recognised by the daemon and 
        // sent to the correct interface as Level I events if the interface  
        // is addressed by the client.
        if (( pEvent->vscp_class <= 1023 ) && 
            ( pEvent->vscp_class >= 512 ) && 
            ( pEvent->sizeData >= 16 )	) {

                // This event should be sent to the correct interface if it is
                // available on this machine. If not it should be sent to 
                // the rest of the network as normal

                cguid destguid;
                destguid.getFromArray( pEvent->pdata );

                destguid.setAt(0,0);    // Interface GUID's have LSB bytes nilled
                destguid.setAt(1,0);
                
                wxString dbgStr = 
                    wxString::Format( _("Level I event over Level II dest = %d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:"), 
                    destguid.getAt(15),destguid.getAt(14),destguid.getAt(13),destguid.getAt(12),
                    destguid.getAt(11),destguid.getAt(10),destguid.getAt(9),destguid.getAt(8),
                    destguid.getAt(7),destguid.getAt(6),destguid.getAt(5),destguid.getAt(4),
                    destguid.getAt(3),destguid.getAt(2),destguid.getAt(1),destguid.getAt(0) );    
                    m_pCtrlObject->logMsg( dbgStr, DAEMON_LOGMSG_INFO );
                

                m_pCtrlObject->m_wxClientMutex.Lock();

                // Find client
                CClientItem *pDestClientItem = NULL;
                VSCPCLIENTLIST::iterator iter;
                for (iter = m_pCtrlObject->m_clientList.m_clientItemList.begin(); 
                    iter != m_pCtrlObject->m_clientList.m_clientItemList.end(); 
                    ++iter) {
                    
                        CClientItem *pItem = *iter;
                        dbgStr = 
                            wxString::Format( _("Test if = %d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:"), 
                            pItem->m_guid.getAt(15),pItem->m_guid.getAt(14),pItem->m_guid.getAt(13),pItem->m_guid.getAt(12),
                            pItem->m_guid.getAt(11),pItem->m_guid.getAt(10),pItem->m_guid.getAt(9),pItem->m_guid.getAt(8),
                            pItem->m_guid.getAt(7),pItem->m_guid.getAt(6),pItem->m_guid.getAt(5),pItem->m_guid.getAt(4),
                            pItem->m_guid.getAt(3),pItem->m_guid.getAt(2),pItem->m_guid.getAt(1),pItem->m_guid.getAt(0) );    
                            dbgStr += _(" ");
                            dbgStr += pItem->m_strDeviceName;
                            m_pCtrlObject->logMsg( dbgStr, DAEMON_LOGMSG_INFO );
                        
                        
                        if ( pItem->m_guid == destguid ) {
                            // Found
                            pDestClientItem = pItem;
							bSent = true;
                            dbgStr = _("Match ");    
                            m_pCtrlObject->logMsg( dbgStr, DAEMON_LOGMSG_INFO );
							m_pCtrlObject->sendEventToClient( pItem, pEvent );
                            break;
                        }

                }

				m_pCtrlObject->m_wxClientMutex.Unlock();

/*
                if ( NULL != pDestClientItem ) {
                    
                    // We must translate the data part of the event to standard format
                    //pEvent->sizeData = pEvent->sizeData - 16;
                    //memcpy( pEvent->pdata, pEvent->pdata + 16, pEvent->sizeData ); 
					//
                    //pEvent->vscp_class = pEvent->vscp_class - 512;
                         
                    // Check if filtered out
                    if ( doLevel2Filter( pEvent, &pDestClientItem->m_filterVSCP ) ) {

                        // If the client queue is full for this client then the
                        // client will not receive the message
                        if ( pDestClientItem->m_clientInputQueue.GetCount() <=		
                            m_pCtrlObject->m_maxItemsInClientReceiveQueue ) {

                                // RX Statistics 1 received frame,+ received data
                                pDestClientItem->m_statistics.cntReceiveFrames++;
                                pDestClientItem->m_statistics.cntReceiveData += pEvent->sizeData;

                                // Add the new event to the inputqueue
                                pDestClientItem->m_mutexClientInputQueue.Lock();
                                pDestClientItem->m_clientInputQueue.Append ( pEvent );
                                pDestClientItem->m_semClientInputQueue.Post();
                                pDestClientItem->m_mutexClientInputQueue.Unlock();

                                bSent = true;

                                // TX Statistics
                                m_pClientItem->m_statistics.cntTransmitData += pEvent->sizeData;
                                m_pClientItem->m_statistics.cntTransmitFrames++;

                        }
                        else {
                            // Overun - No room for event
                            pDestClientItem->m_statistics.cntOverruns++;
                            deleteVSCPevent( pEvent );
                            bSent = true;
                        }

                    } // filter

                } // Client found

                m_pCtrlObject->m_wxClientMutex.Unlock();

*/
        }


        if ( !bSent ) {

            // There must be room in the send queue
            if ( m_pCtrlObject->m_maxItemsInClientReceiveQueue >
                m_pCtrlObject->m_clientOutputQueue.GetCount() ) {

                    m_pCtrlObject->m_mutexClientOutputQueue.Lock();
                    m_pCtrlObject->m_clientOutputQueue.Append ( pEvent );
                    m_pCtrlObject->m_semClientOutputQueue.Post();
                    m_pCtrlObject->m_mutexClientOutputQueue.Unlock();

                    // TX Statistics
                    m_pClientItem->m_statistics.cntTransmitData += pEvent->sizeData;
                    m_pClientItem->m_statistics.cntTransmitFrames++;

            }
            else {

                m_pClientItem->m_statistics.cntOverruns++;

                deleteVSCPevent( pEvent );
                m_pClientSocket->Write ( MSG_BUFFER_FULL,
                    strlen ( MSG_BUFFER_FULL ) );
            }

        }

    } // Valid pEvent

    if ( m_bOK ) {
        m_pClientSocket->Write ( MSG_OK,
            strlen ( MSG_OK ) );
    }
}

///////////////////////////////////////////////////////////////////////////////
// handleClientReceive
//

void TcpClientThread::handleClientReceive ( void )
{
    unsigned short cnt=1;	// # of messages to read

    // Must be accredited to do this
    if ( !m_bVerified ) {
        m_pClientSocket->Write ( MSG_NOT_ACCREDITED,
            strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    m_bOK = true;


    wxString str = m_wxcmd.Right( m_wxcmd.Length() - 4 );
    cnt = readStringValue ( str );

    if ( !cnt ) cnt++;	// No arg is "read one"


    // Read cnt messages
    while ( cnt-- ) {
        wxString strOut;

        if ( !m_pClientItem->m_bOpen ) {
            m_bOK = false;
            m_pClientSocket->Write( MSG_NO_MSG,
                strlen ( MSG_NO_MSG ) );
            break;
        }
        else {
            if ( false == ( m_bOK = sendOneEventFromQueue() ) ) break;

        }

    } // while

    if ( m_bOK ) {
        m_pClientSocket->Write ( MSG_OK,
            strlen ( MSG_OK ) );
    }

}

///////////////////////////////////////////////////////////////////////////////
// sendOneEventFromQueue
//

bool TcpClientThread::sendOneEventFromQueue( bool bStatusMsg )
{
    wxString strOut;

    CLIENTEVENTLIST::compatibility_iterator nodeClient;
    vscpEvent *pEvent = new vscpEvent;

    if ( ( NULL != pEvent ) && m_pClientItem->m_clientInputQueue.GetCount() ) {

        m_pClientItem->m_mutexClientInputQueue.Lock();
        {
            nodeClient = m_pClientItem->m_clientInputQueue.GetFirst();
            vscpEvent *pqueueEvent = nodeClient->GetData();

            // Copy message
            memcpy ( pEvent, pqueueEvent, sizeof ( vscpEvent ) );

            // Remove the node
            m_pClientItem->m_clientInputQueue.DeleteNode ( nodeClient );
        }

        m_pClientItem->m_mutexClientInputQueue.Unlock();

        strOut.Printf( _("%u,%u,%u,%lu,%lu,"),
                            pEvent->head,
                            pEvent->vscp_class,
                            pEvent->vscp_type,
                            pEvent->obid,
                            pEvent->timestamp );

        wxString strGUID;
        writeGuidToString( pEvent, strGUID );
        strOut += strGUID;

        // Handle data
        if ( NULL != pEvent->pdata ) {

            strOut += _(",");
            for ( int i=0; i<pEvent->sizeData; i++ ) {
                wxString wrk;
                wrk.Printf(_("%d"), pEvent->pdata[ i ] );
                if ( ( pEvent->sizeData - 1 ) != i ) {
                    wrk += _(",");
                }

                strOut += wrk;

            }

        }

        strOut += _("\r\n");
        m_pClientSocket->Write ( strOut.mb_str(),
            strlen ( strOut.mb_str() ) );

        // Remove the old data
        if ( NULL != pEvent->pdata ) delete pEvent->pdata;
        pEvent->pdata = NULL;	 // Data stored in message

        delete pEvent;


    }
    else {
        if ( bStatusMsg ) {
            m_pClientSocket->Write ( MSG_NO_MSG,
                strlen ( MSG_NO_MSG ) );
        }

        return false;

    }

    return true;

}



///////////////////////////////////////////////////////////////////////////////
// handleClientDataAvailable
//

void TcpClientThread::handleClientDataAvailable ( void )
{
    char outbuf[ 1024 ];

    // Must be accredited to do this
    if ( !m_bVerified ) {
        m_pClientSocket->Write ( MSG_NOT_ACCREDITED,
            strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    sprintf ( outbuf,
        "%ld\r\n%s",
        m_pClientItem->m_clientInputQueue.GetCount(),
        MSG_OK );
    m_pClientSocket->Write ( outbuf,
        strlen ( outbuf ) );


}

///////////////////////////////////////////////////////////////////////////////
// handleClientClearInputQueue
//

void TcpClientThread::handleClientClearInputQueue ( void )
{
    // Must be accredited to do this
    if ( !m_bVerified ) {
        m_pClientSocket->Write ( MSG_NOT_ACCREDITED,
            strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    m_pClientItem->m_mutexClientInputQueue.Lock();
    m_pClientItem->m_clientInputQueue.Clear();
    m_pClientItem->m_mutexClientInputQueue.Unlock();

    m_pClientSocket->Write ( MSG_QUEUE_CLEARED,
        strlen ( MSG_QUEUE_CLEARED ) );
}


///////////////////////////////////////////////////////////////////////////////
// handleClientGetStatistics
//

void TcpClientThread::handleClientGetStatistics ( void )
{
    char outbuf[ 1024 ];

    // Must be accredited to do this
    if ( !m_bVerified ) {
        m_pClientSocket->Write ( MSG_NOT_ACCREDITED,
            strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    sprintf ( outbuf, "%lu,%lu,%lu,%lu,%lu,%lu,%lu\r\n%s",
        m_pClientItem->m_statistics.cntBusOff,
        m_pClientItem->m_statistics.cntBusWarnings,
        m_pClientItem->m_statistics.cntOverruns,
        m_pClientItem->m_statistics.cntReceiveData,
        m_pClientItem->m_statistics.cntReceiveFrames,
        m_pClientItem->m_statistics.cntTransmitData,
        m_pClientItem->m_statistics.cntTransmitFrames,
        MSG_OK );


    m_pClientSocket->Write ( outbuf,
        strlen ( outbuf ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleClientGetStatus
//

void TcpClientThread::handleClientGetStatus ( void )
{
    char outbuf[ 1024 ];

    // Must be accredited to do this
    if ( !m_bVerified ) {
        m_pClientSocket->Write ( MSG_NOT_ACCREDITED,
            strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    sprintf ( outbuf, "%lu,%lu,%lu,\"%s\"\r\n%s",
        m_pClientItem->m_status.channel_status,
        m_pClientItem->m_status.lasterrorcode,
        m_pClientItem->m_status.lasterrorsubcode,
        m_pClientItem->m_status.lasterrorstr,
        MSG_OK );

    m_pClientSocket->Write ( outbuf,
                                strlen ( outbuf ) );


}

///////////////////////////////////////////////////////////////////////////////
// handleClientGetChannelID
//

void TcpClientThread::handleClientGetChannelID ( void )
{
    char outbuf[ 1024 ];

    // Must be accredited to do this
    if ( !m_bVerified ) {
        m_pClientSocket->Write ( MSG_NOT_ACCREDITED,
            strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    sprintf ( outbuf, "%lu\r\n%s",
        (unsigned long)m_pClientItem->m_clientID, MSG_OK );


    m_pClientSocket->Write ( outbuf,
        strlen ( outbuf ) );

}


///////////////////////////////////////////////////////////////////////////////
// handleClientSetChannelGUID
//

void TcpClientThread::handleClientSetChannelGUID ( void )
{
    // Must be accredited to do this
    if ( !m_bVerified ) {
        m_pClientSocket->Write ( MSG_NOT_ACCREDITED,
            strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    wxString str = m_wxcmdUC.Right( m_wxcmdUC.Length() - 5 ); // remove: command + space
    //getGuidFromStringToArray( m_pClientItem->m_GUID, str );
    m_pClientItem->m_guid.getFromString(str);
    m_bOK = true;
    m_pClientSocket->Write ( MSG_OK, strlen ( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleClientGetChannelGUID
//

void TcpClientThread::handleClientGetChannelGUID ( void )
{
    wxString strBuf;
    //char outbuf[ 1024 ];
    //char wrkbuf[ 20 ];
    int i;

    // Must be accredited to do this
    if ( !m_bVerified ) {
        m_pClientSocket->Write( MSG_NOT_ACCREDITED,
								strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    //*outbuf = 0;
/*
    for ( i=0; i<16; i++ ) {
        sprintf ( wrkbuf, "%d", m_pClientItem->m_guid.getAt(i) );
        if ( 15 != i ) {
            strcat ( wrkbuf, ":" );
        }

        strcat ( outbuf, wrkbuf );
    }
    strlen( strBuf.mb_str()

    strcat ( outbuf, "\r\n" );
    strcat ( outbuf, MSG_OK );
    */
    m_pClientItem->m_guid.toString( strBuf );
    strBuf += _("\r\n");
    strBuf += _(MSG_OK);

    m_pClientSocket->Write( strBuf.mb_str(),
                                strlen( strBuf.mb_str() ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleClientGetVersion
//

void TcpClientThread::handleClientGetVersion ( void )
{
    char outbuf[ 1024 ];

    // Must be accredited to do this
    if ( !m_bVerified ) {
        m_pClientSocket->Write ( MSG_NOT_ACCREDITED,
            strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }


    sprintf ( outbuf,
                "%d,%d,%d\r\n%s",
                VSCPD_MAJOR_VERSION,
                VSCPD_MINOR_VERSION,
                VSCPD_SUB_VERSION,
                MSG_OK );

    m_pClientSocket->Write ( outbuf,
                                strlen ( outbuf ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleClientSetFilter
//

void TcpClientThread::handleClientSetFilter ( void )
{
    // Must be accredited to do this
    if ( !m_bVerified ) {
        m_pClientSocket->Write ( MSG_NOT_ACCREDITED,
            strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    wxString str = m_wxcmd.Right( m_wxcmd.Length() - 5 );
    wxStringTokenizer tkz( str, _(",") );

    // Get priority
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        m_pClientItem->m_filterVSCP.filter_priority = readStringValue( str );
    }
    else {
        m_bOK = false;
        m_pClientSocket->Write ( MSG_PARAMETER_ERROR,
            strlen ( MSG_PARAMETER_ERROR ) );
        return;	
    }

    // Get Class
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        m_pClientItem->m_filterVSCP.filter_class = readStringValue( str );
    }
    else {
        m_bOK = false;
        m_pClientSocket->Write ( MSG_PARAMETER_ERROR,
            strlen ( MSG_PARAMETER_ERROR ) );
        return;	
    }


    // Get Type
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        m_pClientItem->m_filterVSCP.filter_type = readStringValue( str );
    }
    else {
        m_bOK = false;
        m_pClientSocket->Write ( MSG_PARAMETER_ERROR,
            strlen ( MSG_PARAMETER_ERROR ) );
        return;	
    }

    // Get GUID
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        getGuidFromStringToArray( m_pClientItem->m_filterVSCP.filter_GUID, str );
    }
    else {
        m_bOK = false;
        m_pClientSocket->Write( MSG_PARAMETER_ERROR,
								strlen ( MSG_PARAMETER_ERROR ) );
        return;	
    }

    m_pClientSocket->Write( MSG_OK,
							strlen ( MSG_OK ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleClientSetMask
//

void TcpClientThread::handleClientSetMask ( void )
{
    // Must be accredited to do this
    if ( !m_bVerified ) {
        m_pClientSocket->Write( MSG_NOT_ACCREDITED,
								strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    wxString str = m_wxcmd.Right( m_wxcmd.Length() - 5 );
    wxStringTokenizer tkz( str, _(",") );

    // Get priority
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        m_pClientItem->m_filterVSCP.mask_priority = readStringValue( str );
    }
    else {
        m_bOK = false;
        m_pClientSocket->Write ( MSG_PARAMETER_ERROR,
            strlen ( MSG_PARAMETER_ERROR ) );
        return;	
    }

    // Get Class
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        m_pClientItem->m_filterVSCP.mask_class = readStringValue( str );
    }
    else {
        m_bOK = false;
        m_pClientSocket->Write ( MSG_PARAMETER_ERROR,
            strlen ( MSG_PARAMETER_ERROR ) );
        return;	
    }


    // Get Type
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        m_pClientItem->m_filterVSCP.mask_type = readStringValue( str );
    }
    else {
        m_bOK = false;
        m_pClientSocket->Write ( MSG_PARAMETER_ERROR,
            strlen ( MSG_PARAMETER_ERROR ) );
        return;	
    }

    // Get GUID
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        getGuidFromStringToArray( m_pClientItem->m_filterVSCP.mask_GUID, str );
    }
    else {
        m_bOK = false;
        m_pClientSocket->Write ( MSG_PARAMETER_ERROR,
            strlen ( MSG_PARAMETER_ERROR ) );
        return;	
    }

    m_pClientSocket->Write ( MSG_OK,
        strlen ( MSG_OK ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleClientUser
//

void TcpClientThread::handleClientUser ( void )
{
    if ( m_bVerified ) {
        m_pClientSocket->Write ( MSG_OK,
            strlen ( MSG_OK ) );
        return;
    }

    m_wxUserName = m_wxcmd.Right( m_wxcmd.Length() - 4 );
    m_wxUserName.Trim();					// Trim right side	
    m_wxUserName.Trim( false ); 	// Trim left
    if ( m_wxUserName.IsEmpty() ) {
        m_bOK = false;
        m_pClientSocket->Write( MSG_PARAMETER_ERROR,
								strlen ( MSG_PARAMETER_ERROR ) );
        return;
    }

    m_bUsername = true;
    m_pClientSocket->Write( MSG_USENAME_OK,
							strlen ( MSG_USENAME_OK ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleClientPassword
//

bool TcpClientThread::handleClientPassword ( void )
{
    if ( m_bVerified ) {
        m_pClientSocket->Write ( MSG_OK,
            strlen ( MSG_OK ) );
        return true;
    }

    // Must have username before password can be entered.
    if ( !m_bUsername ) {
        m_pClientSocket->Write( MSG_NEED_USERNAME,
								strlen ( MSG_NEED_USERNAME ) );
        return true;
    }

    wxString strPassword = m_wxcmd.Right( m_wxcmd.Length() - 4 );
    strPassword.Trim();             // Trim right side	
    strPassword.Trim( false );      // Trim left
    if ( strPassword.IsEmpty() ) {
        m_bOK = false;
        m_pClientSocket->Write( MSG_PARAMETER_ERROR,
								strlen( MSG_PARAMETER_ERROR ) );
        return false;
    }

    // Calculate MD5 for entered password
    unsigned char buf[2148];
    strncpy( (char *)buf, strPassword.mb_str(), sizeof( buf ) );
    Cmd5 md5 ( buf );
    if ( NULL == md5.getDigest() ) return false; 
    wxString md5Password = wxString( md5.getDigest(), wxConvUTF8 );
    m_pCtrlObject->m_mutexUserList.Lock();
    //::wxLogDebug( _("Username: ") + m_wxUserName );
    //::wxLogDebug( _("Password: ") + strPassword );
    //::wxLogDebug( _("MD5 of Password: ") + md5Password );
    m_pUserItem = m_pCtrlObject->m_userList.checkUser( m_wxUserName, md5Password );
    m_pCtrlObject->m_mutexUserList.Unlock();

    if ( NULL == m_pUserItem ) {
        //::wxLogDebug ( _("Password/Username failure.") );
        m_pClientSocket->Write ( MSG_PASSWORD_ERROR,
            strlen ( MSG_PASSWORD_ERROR ) );
        return false;
    }

    wxIPV4address remoteaddr;
    if ( !m_pClientSocket->GetPeer( remoteaddr ) ) {
        m_pClientSocket->Write( MSG_INVALID_REMOTE_ERROR,
								strlen ( MSG_INVALID_REMOTE_ERROR ) );
        return false;
    } 

    // Check if this user is allowed to connect from this location
    m_pCtrlObject->m_mutexUserList.Lock();
    bool bValidHost = 
            m_pCtrlObject->m_userList.checkRemote( m_pUserItem, 
                                                    remoteaddr.IPAddress() );
    m_pCtrlObject->m_mutexUserList.Unlock();

    if ( !bValidHost ) {
        m_pClientSocket->Write( MSG_INVALID_REMOTE_ERROR,
								strlen ( MSG_INVALID_REMOTE_ERROR ) );
        return false;
    }

    // Copy in the user filter
    memcpy( &m_pClientItem->m_filterVSCP, 
				&m_pUserItem->m_filterVSCP, 
				sizeof( vscpEventFilter ) );


    m_bVerified = true;
    m_pClientSocket->Write ( MSG_OK,
        strlen ( MSG_OK ) );

    // Copy in user filter
    memcpy( &m_pClientItem->m_filterVSCP, 
				&m_pUserItem->m_filterVSCP, 
				sizeof( vscpEventFilter ) );

    return true;

}


///////////////////////////////////////////////////////////////////////////////
// handleClientRcvLoop
//

void TcpClientThread::handleClientRcvLoop()
{
    m_pClientSocket->Write ( MSG_RECEIVE_LOOP,
        strlen ( MSG_RECEIVE_LOOP ) );

    // Loop until the connection is lost
    while ( !TestDestroy() && m_bRun && !m_pCtrlObject->m_bQuit ) {

        if ( m_pClientSocket->Error() &&
            ( wxSOCKET_NOERROR != ( m_err = m_pClientSocket->LastError() ) ) ) {
            if ( wxSOCKET_TIMEDOUT != m_err ) m_bRun = false;
        }

        if ( m_pClientSocket->IsDisconnected() ) m_bRun = false;

        // Wait for event
        if ( wxSEMA_TIMEOUT == 
            m_pClientItem->m_semClientInputQueue.WaitTimeout( 1000 ) ) {
                m_pClientSocket->Write ( "+OK\r\n", 5 );
                continue;
        }

        // Send one event if someting in the queue
        sendOneEventFromQueue( false );

    } // While

    return;
}

///////////////////////////////////////////////////////////////////////////////
// handleClientHelp
//

void TcpClientThread::handleClientHelp(void)
{
	m_pClientSocket->Write(MSG_OK,
			strlen(MSG_OK));
	return;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientTest
//

void TcpClientThread::handleClientTest ( void )
{
	m_pClientSocket->Write ( MSG_OK,
        strlen ( MSG_OK ) );
	return;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientRestart
//

void TcpClientThread::handleClientRestart ( void )
{
	m_pClientSocket->Write ( MSG_OK,
        strlen ( MSG_OK ) );
	return;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientShutdown
//

void TcpClientThread::handleClientShutdown ( void )
{
    if ( !m_bVerified ) {
        m_pClientSocket->Write ( MSG_OK,
            strlen ( MSG_OK ) );
    }

    m_pCtrlObject->m_bQuit = true;
    m_bRun = false;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientRemote
//

void TcpClientThread::handleClientRemote( void )
{
	return;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientInterface
//
// list 	 	List interfaces.
// unique 	Aquire selected interface uniquely. Full format is INTERFACE UNIQUE id
// normal 	Normal access to interfaces. Full format is INTERFACE NORMAL id
// close		Close interfaces. Full format is INTERFACE CLOSE id

void TcpClientThread::handleClientInterface( void )
{
    m_pCtrlObject->logMsg ( m_wxcmdUC, DAEMON_LOGMSG_INFO );

    if ( wxNOT_FOUND != m_wxcmdUC.Find ( _( "LIST" ) ) )	{
        handleClientInterface_List();
    }
    else if ( wxNOT_FOUND != m_wxcmdUC.Find ( _( "UNIQUE " ) ) )	{
        m_wxcmdUC = m_wxcmdUC.Right( m_wxcmdUC.Length()-7 );            // Remove "UNIQUE "
        handleClientInterface_Unique();
    }
    else if ( wxNOT_FOUND != m_wxcmdUC.Find ( _( "NORMAL" ) ) )	{
        handleClientInterface_Normal();
    }
    else if ( wxNOT_FOUND != m_wxcmdUC.Find ( _( "CLOSE" ) ) )	{
        handleClientInterface_Close();
    }
}

///////////////////////////////////////////////////////////////////////////////
// handleClientInterface_List
//

void TcpClientThread::handleClientInterface_List( void )
{
    wxString strGUID;
    wxString strBuf;

    // Display Interface List
    m_pCtrlObject->m_wxClientMutex.Lock();
    VSCPCLIENTLIST::iterator iter;
    for (iter = m_pCtrlObject->m_clientList.m_clientItemList.begin(); 
        iter != m_pCtrlObject->m_clientList.m_clientItemList.end(); 
        ++iter) {

            CClientItem *pItem = *iter;
            //writeGuidArrayToString( pItem->m_GUID, strGUID );	// Get GUID
            pItem->m_guid.toString( strGUID );
            strBuf = wxString::Format(_("%d,"), pItem->m_clientID );
            strBuf += wxString::Format(_("%d,"), pItem->m_type );
            strBuf += strGUID;
            strBuf += _(",");
            strBuf += pItem->m_strDeviceName;
            strBuf += _("\r\n");

            m_pClientSocket->Write( strBuf.mb_str(),
									strlen( strBuf.mb_str() ) );

    }

    m_pClientSocket->Write( MSG_OK,
								strlen ( MSG_OK ) );

    m_pCtrlObject->m_wxClientMutex.Unlock();
}

///////////////////////////////////////////////////////////////////////////////
// handleClientInterface_Unique
//

void TcpClientThread::handleClientInterface_Unique( void )
{
    unsigned char ifGUID[ 16 ];
    memset( ifGUID, 0, 16 );

    // Get GUID
    m_wxcmdUC.Trim( false );
    getGuidFromStringToArray( ifGUID, m_wxcmdUC );

    // Add the client to the Client List
    // TODO

    m_pClientSocket->Write( MSG_INTERFACE_NOT_FOUND,
								strlen( MSG_INTERFACE_NOT_FOUND ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleClientInterface_Normal
//

void TcpClientThread::handleClientInterface_Normal( void )
{
    // TODO
}

///////////////////////////////////////////////////////////////////////////////
// handleClientInterface_Close
//

void TcpClientThread::handleClientInterface_Close( void )
{
    // TODO
}


///////////////////////////////////////////////////////////////////////////////
// handleClientUdp
//

void TcpClientThread::handleClientUdp( void )
{
    // TODO
}


///////////////////////////////////////////////////////////////////////////////
// handleClientFile
//

void TcpClientThread::handleClientFile( void )
{
    // TODO
}


///////////////////////////////////////////////////////////////////////////////
// handleClientVariable
//

void TcpClientThread::handleClientVariable( void )
{
    m_pCtrlObject->logMsg ( m_wxcmdUC, DAEMON_LOGMSG_INFO );

    m_wxcmdUC = m_wxcmdUC.Right( m_wxcmdUC.Length()-9 );    // remove "VARIABLE "
    m_wxcmd = m_wxcmd.Right( m_wxcmd.Length()-9 );          // remove "VARIABLE "
    m_wxcmdUC.Trim( false );
    m_wxcmd.Trim( false );

    if ( 0 == m_wxcmdUC.Find ( wxT( "LIST " ) ) )	{
        handleVariable_List();
    }
    // Hack to handle "variable list"
    else if ( 0 == m_wxcmdUC.Find ( wxT( "LIST" ) ) )	{
        m_wxcmdUC += _(" ");
        m_wxcmd += _(" ");
        handleVariable_List();
    }
    else if ( 0 == m_wxcmdUC.Find ( wxT( "WRITE " ) ) )	{
        handleVariable_Write();
    }
    else if ( 0 == m_wxcmdUC.Find ( wxT( "READ " ) ) )	{
        handleVariable_Read();
    }
    else if ( 0 == m_wxcmdUC.Find ( wxT( "READRESET " ) ) )	{
        handleVariable_ReadReset();
    }
    else if ( 0 == m_wxcmdUC.Find ( wxT( "RESET" ) ) )	{
        handleVariable_Reset();
    }
    else if ( 0 == m_wxcmdUC.Find ( wxT( "REMOVE " ) ) )	{
        handleVariable_Remove();
    }
    else if ( 0 == m_wxcmdUC.Find ( wxT( "READREMOVE " ) ) )	{
        handleVariable_ReadRemove();
    }
    else if ( 0 == m_wxcmdUC.Find ( wxT( "LENGTH " ) ) )	{
        handleVariable_Length();
    }
    else if ( 0 == m_wxcmdUC.Find ( wxT( "LOAD" ) ) )	{
        handleVariable_Load();
    }
    else if ( 0 == m_wxcmdUC.Find ( wxT( "SAVE" ) ) )	{
        handleVariable_Save();
    }
    else {
        m_pClientSocket->Write( MSG_UNKNOWN_COMMAND,
								strlen( MSG_UNKNOWN_COMMAND ) );
    }
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_List
//
// variable list all	- List all variables.
// variable list *		- List all variables.
// variable list		- List all variables.
// variable list name	- List variable with name 'name'.
// variable list name*	- Name with wildcard.
//

void TcpClientThread::handleVariable_List()
{
    CVSCPVariable *pVariable;
    wxString str;
    wxString strWork;
    listVscpVariable::iterator it;

    m_wxcmdUC = m_wxcmdUC.Right( m_wxcmdUC.Length()-5 );    // remove "LIST "
    m_wxcmd = m_wxcmd.Right( m_wxcmd.Length()-5 );          // remove "LIST "
    m_wxcmdUC.Trim();
    m_wxcmdUC.Trim( false );
    m_wxcmd.Trim();
    m_wxcmd.Trim( false );

    // if "variable list" we add "all"
    if ( 0 == m_wxcmdUC.Length() ) {
        m_wxcmdUC +=  _("ALL");
        m_wxcmd += _("all");
    }

    // If "*" wildcard we add "all"
    if ( ( 1 == m_wxcmd.Length() ) && 
        ( 0 == m_wxcmdUC.Find ( _( "*" ) ) ) ) {
            m_wxcmdUC += _("ALL");
            m_wxcmd += _("all");	
    }

    if ( ( wxNOT_FOUND != m_wxcmdUC.Find ( _( "ALL" ) ) ) )	{

        m_pCtrlObject->m_variableMutex.Lock();

        for( it = m_pCtrlObject->m_VSCP_Variables.m_listVariable.begin(); 
            it != m_pCtrlObject->m_VSCP_Variables.m_listVariable.end(); 
            ++it ) {

                if ( NULL == ( pVariable = *it ) ) continue;

                str = pVariable->getName();
                str += _(",");
                strWork.Printf( _("%d"), pVariable->getType() ) ;
                str += strWork;
                if ( pVariable->isPersistent() ) {
                    str += _(",true,");
                }
                else {
                    str += _(",false,");
                }
                pVariable->writeVariableToString( strWork );
                str += strWork;
                str += _("\r\n");

                m_pClientSocket->Write( str, str.Length() );

        }

        m_pCtrlObject->m_variableMutex.Unlock();

    }
    else {

        // variables (or wildcards) to list are separated
        // with spaces
        wxStringTokenizer tkz( m_wxcmdUC, _(" ") );

        while ( tkz.HasMoreTokens() ) {

            wxString token = tkz.GetNextToken();
            token.Trim();
            token.Trim( false );
            token = token.Upper();

            // Wildcard?
            if ( wxNOT_FOUND != token.Find(_("*")) ) {

                // Possibilities:
                //	    *aaaa
                //      aaaa*
                //      *aaaa*  

                // Find the pos for the wildcard char
                int pos = token.Find( '*' );

                // true if wildcard is on the left "*aaaa"
                bool bfromLeft = true;
                bool bDoubleWildcard = false;

                // remove wildcard
                if ( 0 == pos ) {

                    // Wildcard sits on the left side "*aaaa"
                    bfromLeft = true;

                    // Remove the wildcard	
                    token = token.Right( token.Length() - 1 );

                    // Can still be a wildcard left
                    if ( wxNOT_FOUND != ( pos = token.Find( '*' ) ) ) {

                        bDoubleWildcard = true;

                        // The wildcard must be on the oposit side

                        // Remove the wildcard	
                        token = token.Left( token.Length() - 1 );

                    }

                }
                else {

                    // Wildcard sits on the right side "aaaa*"
                    bfromLeft = false;

                    // Remove the wildcard	
                    token = token.Left( token.Length() - 1 );

                    // Can still be a wildcard left
                    if ( wxNOT_FOUND != ( pos = token.Find( '*' ) ) ) {

                        bDoubleWildcard = true;

                        // The wildcard must be on the oposit side

                        // Remove the wildcard	
                        token = token.Right( token.Length() - 1 );

                    }
                }

                // List a specific variable
                m_pCtrlObject->m_variableMutex.Lock();

                for( it = m_pCtrlObject->m_VSCP_Variables.m_listVariable.begin(); 
                    it != m_pCtrlObject->m_VSCP_Variables.m_listVariable.end(); 
                    ++it ) {

                        if ( NULL == ( pVariable = *it ) ) continue;

                        str = pVariable->getName();

                        int posFound;
                        if ( wxNOT_FOUND != ( posFound = str.Find( token ) ) ) {

                            if ( bDoubleWildcard ||
                                ( !bfromLeft && ( 0 == posFound ) ) || 
                                ( bfromLeft && ( ( str.Length() - token.Length() ) == posFound ) ) ) {

                                    str = pVariable->getName();
                                    str += _(",");
                                    strWork.Printf( _("%d"), pVariable->getType() ) ;
                                    str += strWork;
                                    if ( pVariable->isPersistent() ) {
                                        str += _(",true,");
                                    }
                                    else {
                                        str += _(",false,");
                                    }
                                    pVariable->writeVariableToString( strWork );
                                    str += strWork;
                                    str += _("\r\n");

                                    m_pClientSocket->Write( str, str.Length() );

                            }

                        }

                } // next token

                m_pCtrlObject->m_variableMutex.Unlock();

            }
            // Not a wildcard
            else {

                // List a specific variable
                m_pCtrlObject->m_variableMutex.Lock();

                for( it = m_pCtrlObject->m_VSCP_Variables.m_listVariable.begin(); 
                    it != m_pCtrlObject->m_VSCP_Variables.m_listVariable.end(); 
                    ++it ) {

                        if ( NULL == ( pVariable = *it ) ) continue;

                        if ( pVariable->getName() == token ) {

                            str = pVariable->getName();
                            str += _(",");
                            strWork.Printf( _("%d"), pVariable->getType() ) ;
                            str += strWork;
                            if ( pVariable->isPersistent() ) {
                                str += _(",true,");
                            }
                            else {
                                str += _(",false,");
                            }
                            pVariable->writeVariableToString( strWork );
                            str += strWork;
                            str += _("\r\n");

                            m_pClientSocket->Write( str, str.Length() );

                        }

                }

                m_pCtrlObject->m_variableMutex.Unlock();

            }

        }

    }

    m_pClientSocket->Write( MSG_OK,
								strlen ( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_Write
//

void TcpClientThread::handleVariable_Write()
{
    wxString strName;
    wxString strValue;
    int type = VSCP_DAEMON_VARIABLE_CODE_STRING;
    bool bPersistence = false;

    m_pCtrlObject->logMsg( m_wxcmdUC, DAEMON_LOGMSG_INFO );

    m_wxcmd = m_wxcmd.Right( m_wxcmd.Length() - 6 ); // remove "WRITE "
    m_wxcmd.Trim( false );
    m_wxcmd.Trim( true );

    wxStringTokenizer tkz( m_wxcmd, _(",") );

    // Get name of variable
    if ( tkz.HasMoreTokens() ) {
        strName = tkz.GetNextToken();
        strName.MakeUpper();
    }
    else {
        m_pClientSocket->Write ( MSG_PARAMETER_ERROR,
                                    strlen( MSG_PARAMETER_ERROR ) );
        return;	
    }

    // type (optional) - string default
    if ( tkz.HasMoreTokens() ) {
        wxString wxstr = tkz.GetNextToken();
        wxstr.Trim();
        wxstr.Trim( false );
        if ( wxstr.Length() ) {
            type = readStringValue( tkz.GetNextToken() );
        }
    }
    else {
        m_pClientSocket->Write ( MSG_PARAMETER_ERROR,
                                    strlen( MSG_PARAMETER_ERROR ) );
        return;	
    }

    // persistence (optional) - false default
    if ( tkz.HasMoreTokens() ) {
        wxString str = tkz.GetNextToken();
        str = str.Upper();
        if ( str.Find( _("TRUE") ) ) {
            bPersistence = true;		
        }
    }
    else {
        m_pClientSocket->Write( MSG_PARAMETER_ERROR,
                                    strlen( MSG_PARAMETER_ERROR ) );
        return;	
    }

    // Value
    if ( tkz.HasMoreTokens() ) {
        strValue = tkz.GetString();
    }
    else {
        m_pClientSocket->Write( MSG_PARAMETER_ERROR,
                                    strlen( MSG_PARAMETER_ERROR ) );
        return;	
    }

    // If the variable exist change value
    // if not add it. This is handled in add.
    m_pCtrlObject->m_VSCP_Variables.add( strName, strValue, type, bPersistence );

    m_pClientSocket->Write( MSG_OK,
                                strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_Read
//

void TcpClientThread::handleVariable_Read( bool bOKResponse )
{
    wxString str;

    m_wxcmdUC = m_wxcmdUC.Right( m_wxcmdUC.Length() - 5 ); // remove "READ "
    m_wxcmdUC.Trim( false );
    m_wxcmdUC.Trim( true );

    CVSCPVariable * pVariable;

    if ( NULL == ( pVariable = m_pCtrlObject->m_VSCP_Variables.find( m_wxcmdUC ) ) ) {
        m_pClientSocket->Write( MSG_VARIABLE_NOT_DEFINED,
                                strlen ( MSG_VARIABLE_NOT_DEFINED ) );
        return;
    }

    pVariable->writeVariableToString( str );
    str += _("\r\n");
    m_pClientSocket->Write( str.ToAscii(), strlen( str.ToAscii() ) );

    m_pClientSocket->Write( MSG_OK,
                                strlen ( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_Reset
//

void TcpClientThread::handleVariable_Reset()
{

    wxString str;

    m_wxcmdUC = m_wxcmdUC.Right( m_wxcmdUC.Length() - 10 ); // remove "READRESET "
    m_wxcmdUC.Trim( false );
    m_wxcmdUC.Trim( true );

    CVSCPVariable * pVariable;

    if ( NULL == ( pVariable = m_pCtrlObject->m_VSCP_Variables.find( m_wxcmdUC ) ) ) {

        // Must create the variable
        m_pCtrlObject->m_VSCP_Variables.add( m_wxcmdUC, 
                                                _(""), 
                                                VSCP_DAEMON_VARIABLE_CODE_STRING, 
                                                false );

        // Try again, Should be possible to find now
        if ( NULL == ( pVariable = m_pCtrlObject->m_VSCP_Variables.find( m_wxcmdUC ) ) ) {
            m_pClientSocket->Write( MSG_PARAMETER_ERROR,
                                        strlen ( MSG_PARAMETER_ERROR ) );
            return;
        }
    }

    pVariable->Reset();

    m_pClientSocket->Write( MSG_OK,
                                strlen( MSG_OK ) );	
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_ReadReset
//

void TcpClientThread::handleVariable_ReadReset()
{
    wxString str;

    m_wxcmdUC = m_wxcmdUC.Right( m_wxcmdUC.Length() - 10 ); // remove "READRESET "
    m_wxcmdUC.Trim( false );
    m_wxcmdUC.Trim( true );

    CVSCPVariable * pVariable;

    if ( NULL == ( pVariable = m_pCtrlObject->m_VSCP_Variables.find( m_wxcmdUC ) ) ) {

        // Must create the variable
        m_pCtrlObject->m_VSCP_Variables.add( m_wxcmdUC, 
                                                _(""), 
                                                VSCP_DAEMON_VARIABLE_CODE_STRING, 
                                                false );
        // Try again, Should be possible to find now
        if ( NULL == ( pVariable = m_pCtrlObject->m_VSCP_Variables.find( m_wxcmdUC ) ) ) {
            m_pClientSocket->Write( MSG_PARAMETER_ERROR,
                strlen ( MSG_PARAMETER_ERROR ) );
            return;
        }
    }

    pVariable->writeVariableToString( str );
    str += _("\r\n");
    m_pClientSocket->Write( str, str.Length() );

    handleVariable_Reset();
}


///////////////////////////////////////////////////////////////////////////////
// handleVariable_Remove
//

void TcpClientThread::handleVariable_Remove()
{
    wxString str;

    m_wxcmdUC = m_wxcmdUC.Right( m_wxcmdUC.Length() - 7 ); // remove "REMOVE "
    m_wxcmdUC.Trim( false );
    m_wxcmdUC.Trim( true );

    m_pCtrlObject->m_VSCP_Variables.remove( m_wxcmdUC );

    m_pClientSocket->Write( MSG_OK,
                                strlen( MSG_OK ) );
}


///////////////////////////////////////////////////////////////////////////////
// handleVariable_ReadRemove
//

void TcpClientThread::handleVariable_ReadRemove()
{
    wxString str;

    m_wxcmdUC = m_wxcmdUC.Right( m_wxcmdUC.Length() - 11 ); // remove "READREMOVE "
    m_wxcmdUC.Trim( false );
    m_wxcmdUC.Trim( true );

    CVSCPVariable * pVariable;

    if ( NULL == ( pVariable = m_pCtrlObject->m_VSCP_Variables.find( m_wxcmdUC ) ) ) {

        // Must create the variable
        m_pCtrlObject->m_VSCP_Variables.add( m_wxcmdUC, 
                                                _(""), 
                                                VSCP_DAEMON_VARIABLE_CODE_STRING, 
                                                false );
        // Try again, Should be possible to find now
        if ( NULL == ( pVariable = m_pCtrlObject->m_VSCP_Variables.find( m_wxcmdUC ) ) ) {
            m_pClientSocket->Write( MSG_PARAMETER_ERROR,
                                        strlen ( MSG_PARAMETER_ERROR ) );
            return;
        }
    }

    pVariable->writeVariableToString( str );
    str += _("\r\n");
    m_pClientSocket->Write( str, str.Length() );

    handleVariable_Remove();
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_Length
//

void TcpClientThread::handleVariable_Length()
{
    wxString str;

    m_wxcmdUC = m_wxcmdUC.Right( m_wxcmdUC.Length() - 7 ); // remove "LENGTH "
    m_wxcmdUC.Trim( false );
    m_wxcmdUC.Trim( true );

    CVSCPVariable * pVariable;

    if ( NULL == ( pVariable = m_pCtrlObject->m_VSCP_Variables.find( m_wxcmdUC ) ) ) {

        // Must create the variable
        m_pCtrlObject->m_VSCP_Variables.add( m_wxcmdUC, 
                                                _(""), 
                                                VSCP_DAEMON_VARIABLE_CODE_STRING, 
                                                false );
        // Try again, Should be possible to find now
        if ( NULL == ( pVariable = m_pCtrlObject->m_VSCP_Variables.find( m_wxcmdUC ) ) ) {
            m_pClientSocket->Write( MSG_PARAMETER_ERROR,
                                        strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
    }

    if ( pVariable->getType() ) {
        str.Printf( _("%d\r\n"), pVariable->m_strValue.Length()  );
        m_pClientSocket->Write( str.mb_str(),
                                    str.Length() );
    }
    else {
        m_pClientSocket->Write( "0\r\n",
                                    strlen( "0\r\n" ) );	
    }

    m_pClientSocket->Write( MSG_OK,
                                strlen( MSG_OK ) );
}



///////////////////////////////////////////////////////////////////////////////
// handleVariable_Load
//

void TcpClientThread::handleVariable_Load()
{
    m_pCtrlObject->m_VSCP_Variables.load();

    m_pClientSocket->Write( MSG_OK,
                                strlen( MSG_OK ) );
}


///////////////////////////////////////////////////////////////////////////////
// handleVariable_Save
//

void TcpClientThread::handleVariable_Save()
{
    wxString path;
    //wxStandardPaths stdPath;

    // Set the default variable configuration path
    path = wxStandardPaths::Get().GetConfigDir();
    path += _("/vscp/variable.xml");

    m_pCtrlObject->m_VSCP_Variables.save( path );

    m_pClientSocket->Write( MSG_OK,
                                strlen( MSG_OK ) );
}




// ****************************************************************************



///////////////////////////////////////////////////////////////////////////////
// handleClientDm
//

void TcpClientThread::handleClientDm( void )
{
    m_pCtrlObject->logMsg ( m_wxcmdUC, DAEMON_LOGMSG_INFO );

    m_wxcmdUC = m_wxcmdUC.Right( m_wxcmdUC.Length()-3 );
    m_wxcmdUC.Trim( false );

    if ( wxNOT_FOUND != m_wxcmdUC.Find ( _( "ENABLE " ) ) )	{
        handleDM_Enable();
    }
    else if ( wxNOT_FOUND != m_wxcmdUC.Find ( _( "DISABLE " ) ) )	{
        handleDM_Enable();
    }
    else if ( wxNOT_FOUND != m_wxcmdUC.Find ( _( "LIST" ) ) )	{
        handleDM_List();
    }
    else if ( wxNOT_FOUND != m_wxcmdUC.Find ( _( "ADD " ) ) )	{
        handleDM_Add();
    }
    else if ( wxNOT_FOUND != m_wxcmdUC.Find ( _( "DELETE " ) ) )	{
        handleDM_Delete();
    }
    else if ( wxNOT_FOUND != m_wxcmdUC.Find ( _( "RESET" ) ) )	{
        handleDM_Reset();
    }
    else if ( wxNOT_FOUND != m_wxcmdUC.Find ( _( "TRIG " ) ) )	{
        handleDM_Trigger();
    }
    else if ( wxNOT_FOUND != m_wxcmdUC.Find ( _( "CLRTRIG " ) ) )	{
        handleDM_ClearTriggerCount();
    }
    else if ( wxNOT_FOUND != m_wxcmdUC.Find ( _( "CLRERR " ) ) )	{
        handleDM_ClearErrorCount();
    }
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_Enable
//

void TcpClientThread::handleDM_Enable( void )
{
    unsigned short pos;

    if ( wxNOT_FOUND != m_wxcmdUC.Find ( _( "ALL" ) ) )	{

        m_pCtrlObject->m_dm.m_mutexDM.Lock();

        DMLIST::iterator iter;
        for (iter = m_pCtrlObject->m_dm.m_DMList.begin(); iter != m_pCtrlObject->m_dm.m_DMList.end(); ++iter)
        {
            dmElement *pDMItem = *iter;
            pDMItem->enable();
        }	

        m_pCtrlObject->m_dm.m_mutexDM.Unlock();

    }
    else {

        // Get the position
        pos = 0;

        m_pCtrlObject->m_dm.m_mutexDM.Lock();

        if ( pos > ( m_pCtrlObject->m_dm.m_DMList.GetCount() - 1 ) ) {
            m_pClientSocket->Write ( MSG_PARAMETER_ERROR,
                strlen ( MSG_PARAMETER_ERROR ) );
            m_pCtrlObject->m_dm.m_mutexDM.Unlock();
            return;
        }

        DMLIST::compatibility_iterator node = m_pCtrlObject->m_dm.m_DMList.Item( pos );
        ( node->GetData() )->enable();
        m_pCtrlObject->m_dm.m_mutexDM.Unlock();

    }

    m_pClientSocket->Write( MSG_OK,
                                strlen( MSG_OK ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleDM_Disable
//

void TcpClientThread::handleDM_Disable( void )
{
    unsigned short pos;

    if ( wxNOT_FOUND != m_wxcmdUC.Find ( _( "ALL" ) ) )	{

        m_pCtrlObject->m_dm.m_mutexDM.Lock();

        DMLIST::iterator iter;
        for (iter = m_pCtrlObject->m_dm.m_DMList.begin(); 
            iter != m_pCtrlObject->m_dm.m_DMList.end(); 
            ++iter )
        {
            dmElement *pDMItem = *iter;
            pDMItem->disable();
        }	
        m_pCtrlObject->m_dm.m_mutexDM.Unlock();

    }
    else {

        // Get the position
        wxStringTokenizer tkz( m_wxcmdUC, _(",") );
        while ( tkz.HasMoreTokens() ) {

            pos = readStringValue( tkz.GetNextToken() );

            m_pCtrlObject->m_dm.m_mutexDM.Lock();

            if ( pos > ( m_pCtrlObject->m_dm.m_DMList.GetCount() - 1 ) ) {
                m_pClientSocket->Write ( MSG_PARAMETER_ERROR,
                    strlen ( MSG_PARAMETER_ERROR ) );
                m_pCtrlObject->m_dm.m_mutexDM.Unlock();
                return;
            }

            DMLIST::compatibility_iterator node = m_pCtrlObject->m_dm.m_DMList.Item( pos );
            ( node->GetData() )->disable();

            m_pCtrlObject->m_dm.m_mutexDM.Unlock();

        }

    }

    m_pClientSocket->Write( MSG_OK,
                                strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_List
//

void TcpClientThread::handleDM_List( void )
{
    // Valid commands at this point
    // dm list
    // dm list all
    // dm list *
    // dm list 1
    // dm list 1,2,3,4,98

    // Remove "LIST"
    m_wxcmdUC = m_wxcmdUC.Right( m_wxcmdUC.Length() - 4 );
    m_wxcmdUC.Trim( false );

    // if "list" add "all"
    if ( 0 == m_wxcmdUC.Length() ) {
        m_wxcmdUC = _("ALL");
    }

    if ( ( wxNOT_FOUND != m_wxcmdUC.Find ( _( "ALL" ) ) ) ||
            ( wxNOT_FOUND != m_wxcmdUC.Find ( _( "*" ) ) ) )	{
    
        m_pCtrlObject->m_dm.m_mutexDM.Lock();

        DMLIST::iterator iter;
        for (iter = m_pCtrlObject->m_dm.m_DMList.begin(); 
                iter != m_pCtrlObject->m_dm.m_DMList.end(); 
                ++iter) {

            dmElement *pDMItem = *iter;
            wxString strRow = pDMItem->getAsRealText();

            m_pClientSocket->Write( strRow.mb_str(),
                                        strlen ( strRow.mb_str() ) );

        }	

        m_pCtrlObject->m_dm.m_mutexDM.Unlock();
    
    }
    else {

        // We have a list with specific rows  "list 1,8,9"
        //      first parse the argument to get the rows
        //WX_DEFINE_ARRAY_INT( int, ArrayOfSortedInts );
        wxArrayInt rowArray;
        wxStringTokenizer tok( m_wxcmdUC, _(",") );
        while ( tok.HasMoreTokens() ) {
            
            int n = readStringValue( tok.GetNextToken() );
            rowArray.Add( n );

        }

    }


    m_pClientSocket->Write( MSG_OK, strlen ( MSG_OK ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleDM_Add
//

void TcpClientThread::handleDM_Add( void )
{
    dmElement *pDMItem = new dmElement;

    wxStringTokenizer tkz( m_wxcmdUC, _(",") );

    // Priority
    pDMItem->m_vscpfilter.mask_priority = readStringValue( tkz.GetNextToken() );
    pDMItem->m_vscpfilter.filter_priority = readStringValue( tkz.GetNextToken() );

    // Class
    pDMItem->m_vscpfilter.mask_class = readStringValue( tkz.GetNextToken() );
    pDMItem->m_vscpfilter.filter_class = readStringValue( tkz.GetNextToken() );

    // Type
    pDMItem->m_vscpfilter.mask_type = readStringValue( tkz.GetNextToken() );
    pDMItem->m_vscpfilter.filter_type = readStringValue( tkz.GetNextToken() );

    // GUID
    wxString strGUID;
    strGUID =tkz.GetNextToken();
    getGuidFromStringToArray( pDMItem->m_vscpfilter.mask_GUID, strGUID );	
    strGUID = tkz.GetNextToken();
    getGuidFromStringToArray( pDMItem->m_vscpfilter.filter_GUID, strGUID );	

    // control
    pDMItem->m_control = readStringValue( tkz.GetNextToken() );

    // action code
    pDMItem->m_action = readStringValue( tkz.GetNextToken() );

    // action parameters
    pDMItem->m_actionparam = tkz.GetNextToken();

    // add the DM row to the matrix
    m_pCtrlObject->m_dm.m_mutexDM.Lock();
    m_pCtrlObject->m_dm.addElement ( pDMItem );
    m_pCtrlObject->m_dm.m_mutexDM.Unlock();

    m_pClientSocket->Write( MSG_OK,
								strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_Delete
//

void TcpClientThread::handleDM_Delete( void )
{
    unsigned short pos;

    if ( wxNOT_FOUND != m_wxcmdUC.Find ( _( "ALL" ) ) )	{

        m_pCtrlObject->m_dm.m_mutexDM.Lock();

        DMLIST::iterator iter;
        for (iter = m_pCtrlObject->m_dm.m_DMList.begin(); 
            iter != m_pCtrlObject->m_dm.m_DMList.end(); 
            ++iter) {
                dmElement *pDMItem = *iter;
                delete pDMItem;
        }	
        m_pCtrlObject->m_dm.m_DMList.Clear();
        m_pCtrlObject->m_dm.m_mutexDM.Unlock();

    }
    else {

        // Get the position
        wxStringTokenizer tkz( m_wxcmdUC, _(",") );
        while ( tkz.HasMoreTokens() ) {

            pos = readStringValue( tkz.GetNextToken() );

            m_pCtrlObject->m_dm.m_mutexDM.Lock();

            if ( pos > ( m_pCtrlObject->m_dm.m_DMList.GetCount() - 1 ) ) {
                m_pClientSocket->Write ( MSG_PARAMETER_ERROR,
                    strlen ( MSG_PARAMETER_ERROR ) );
                m_pCtrlObject->m_dm.m_mutexDM.Unlock();
                return;
            }

            DMLIST::compatibility_iterator node = m_pCtrlObject->m_dm.m_DMList.Item( pos );
            dmElement *pDMItem = node->GetData();
            m_pCtrlObject->m_dm.m_DMList.DeleteNode( node );	// Delete the node
            delete pDMItem;										// Delete the object

            m_pCtrlObject->m_dm.m_mutexDM.Unlock();

        }

    }

    m_pClientSocket->Write ( MSG_OK,
        strlen ( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_Reset
//

void TcpClientThread::handleDM_Reset( void )
{
    m_pCtrlObject->stopDaemonWorkerThread();
    m_pCtrlObject->startDaemonWorkerThread();

    m_pClientSocket->Write( MSG_OK,
							strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_Trigger
//

void TcpClientThread::handleDM_Trigger()
{
    unsigned short pos;

    // Get the position
    wxStringTokenizer tkz( m_wxcmdUC, _(",") );
    while ( tkz.HasMoreTokens() ) {

        pos = readStringValue( tkz.GetNextToken() );

        m_pCtrlObject->m_dm.m_mutexDM.Lock();

        if ( pos > ( m_pCtrlObject->m_dm.m_DMList.GetCount() - 1 ) ) {
            m_pClientSocket->Write ( MSG_PARAMETER_ERROR,
                strlen ( MSG_PARAMETER_ERROR ) );
            m_pCtrlObject->m_dm.m_mutexDM.Unlock();
            return;
        }

        DMLIST::compatibility_iterator node = m_pCtrlObject->m_dm.m_DMList.Item( pos );
        dmElement *pDMItem = node->GetData();
        pDMItem->doAction( NULL );

        m_pCtrlObject->m_dm.m_mutexDM.Unlock();

    }

    m_pClientSocket->Write( MSG_OK,
							strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_ClearTriggerCount
//


void TcpClientThread::handleDM_ClearTriggerCount()
{
    unsigned short pos;

    if ( wxNOT_FOUND != m_wxcmdUC.Find ( _( "ALL" ) ) )	{

        m_pCtrlObject->m_dm.m_mutexDM.Lock();

        DMLIST::iterator iter;
        for (iter = m_pCtrlObject->m_dm.m_DMList.begin(); 
            iter != m_pCtrlObject->m_dm.m_DMList.end(); 
            ++iter) {
                dmElement *pDMItem = *iter;
                pDMItem->m_triggCounter = 0;
        }	

        m_pCtrlObject->m_dm.m_mutexDM.Unlock();

    }
    else {

        // Get the position
        wxStringTokenizer tkz( m_wxcmdUC, _(",") );
        while ( tkz.HasMoreTokens() ) {

            pos = readStringValue( tkz.GetNextToken() );

            m_pCtrlObject->m_dm.m_mutexDM.Lock();

            if ( pos > ( m_pCtrlObject->m_dm.m_DMList.GetCount() - 1 ) ) {
                m_pClientSocket->Write ( MSG_PARAMETER_ERROR,
                    strlen ( MSG_PARAMETER_ERROR ) );
                m_pCtrlObject->m_dm.m_mutexDM.Unlock();
                return;
            }

            DMLIST::compatibility_iterator node = m_pCtrlObject->m_dm.m_DMList.Item( pos );
            dmElement *pDMItem = node->GetData();
            pDMItem->m_triggCounter = 0;

            m_pCtrlObject->m_dm.m_mutexDM.Unlock();

        }

    }

    m_pClientSocket->Write( MSG_OK,
							strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_ClearErrorCount
//


void TcpClientThread::handleDM_ClearErrorCount()
{
    unsigned short pos;

    if ( wxNOT_FOUND != m_wxcmdUC.Find ( _( "ALL" ) ) )	{

        m_pCtrlObject->m_dm.m_mutexDM.Lock();

        DMLIST::iterator iter;
        for (iter = m_pCtrlObject->m_dm.m_DMList.begin(); 
            iter != m_pCtrlObject->m_dm.m_DMList.end(); 
            ++iter) {
                dmElement *pDMItem = *iter;
                pDMItem->m_errorCounter = 0;
        }	

        m_pCtrlObject->m_dm.m_mutexDM.Unlock();

    }
    else {

        // Get the position
        wxStringTokenizer tkz( m_wxcmdUC, _(",") );
        while ( tkz.HasMoreTokens() ) {

            pos = readStringValue( tkz.GetNextToken() );

            m_pCtrlObject->m_dm.m_mutexDM.Lock();

            if ( pos > ( m_pCtrlObject->m_dm.m_DMList.GetCount() - 1 ) ) {
                m_pClientSocket->Write ( MSG_PARAMETER_ERROR,
                    strlen ( MSG_PARAMETER_ERROR ) );
                m_pCtrlObject->m_dm.m_mutexDM.Unlock();
                return;
            }

            DMLIST::compatibility_iterator node = m_pCtrlObject->m_dm.m_DMList.Item( pos );
            dmElement *pDMItem = node->GetData();
            pDMItem->m_errorCounter = 0;

            m_pCtrlObject->m_dm.m_mutexDM.Unlock();

        }

    }

    m_pClientSocket->Write( MSG_OK,
							strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleClientList
//

void TcpClientThread::handleClientList( void )
{
    m_pClientSocket->Write ( MSG_OK,
        strlen ( MSG_OK ) );
}


///////////////////////////////////////////////////////////////////////////////
// handleClientDriver
//

void TcpClientThread::handleClientDriver( void )
{

}



