// tcpipclientthread.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (http://www.vscp.org)
//
// Copyright (C) 2000-2017
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
#include "vscptcpipclientthread.h"
#include <vscphelper.h>
#include <mongoose.h>
#include <version.h>
#include <controlobject.h>


///////////////////////////////////////////////////
//                 GLOBALS
///////////////////////////////////////////////////

extern CControlObject *gpobj;



///////////////////////////////////////////////////////////////////////////////
// VSCPWebServerThread
//
// This thread listens for connection on a TCP socket and starts a new thread
// to handle client requests
//

VSCPClientThread::VSCPClientThread()
    : wxThread(wxTHREAD_JOINABLE)
{
    m_bQuit = false;
}


VSCPClientThread::~VSCPClientThread()
{
    ;
}


///////////////////////////////////////////////////////////////////////////////
// Entry
//

void *VSCPClientThread::Entry()
{
    // Check pointers
    if ( NULL == gpobj ) return NULL;

    mg_mgr_init( &gpobj->m_mgrTcpIpServer, gpobj );

    // Construct bind interface address
    //[PROTO://][IP_ADDRESS]:PORT where host part is optional
    
    wxStringTokenizer tkz( gpobj->m_strTcpInterfaceAddress, _(" ") );
    while ( tkz.HasMoreTokens() ) {

        wxString str = tkz.GetNextToken();
        str.Trim(true);
        str.Trim(false);
        if ( 0 == str.Length() ) continue;

        // Bind to this interface
        mg_bind( &gpobj->m_mgrTcpIpServer,
                    (const char *)str.mbc_str(),
                    VSCPClientThread::ev_handler );

    }

    gpobj->logMsg( _("[TCP/IP srv] Thread started.\n"), DAEMON_LOGMSG_DEBUG  );

    while ( !TestDestroy() && !m_bQuit ) {
        mg_mgr_poll( &gpobj->m_mgrTcpIpServer, 50 );
        Yield();
    }
    
    gpobj->logMsg( _("[TCP/IP srv] Free.\n"), DAEMON_LOGMSG_DEBUG );

    // release the server
    mg_mgr_free( &gpobj->m_mgrTcpIpServer );

    gpobj->logMsg( _("[TCP/IP srv] Quit.\n"), DAEMON_LOGMSG_DEBUG );

    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void VSCPClientThread::OnExit()
{
    gpobj->logMsg( _("[TCP/IP srv] Exit.\n"), DAEMON_LOGMSG_DEBUG );;
}

///////////////////////////////////////////////////////////////////////////////
// ev_handler
//

void VSCPClientThread::ev_handler( struct mg_connection *conn,
                                    int ev,
                                    void *pUser)
{
    char rbuf[ 2048 ];
    int pos4lf;

    CControlObject *pCtrlObject = NULL;
    CClientItem *pClientItem = NULL;

    if ( NULL == conn ) {
        return;
    }

    pClientItem = ( CClientItem * )conn->user_data;

    if ( ( NULL == conn->mgr ) || ( NULL == conn->mgr->user_data ) ) {
        conn->flags |= MG_F_CLOSE_IMMEDIATELY; // Close connection
        return;
    }
    pCtrlObject = ( CControlObject * )conn->mgr->user_data;

    switch (ev) {

        case MG_EV_CONNECT: // connect() succeeded or failed. int *success_status
            pCtrlObject->logMsg(_("[TCP/IP srv] Connect.\n") );
            break;

        case MG_EV_ACCEPT:	// New connection accept()-ed. union socket_address *remote_addr
            {
                pCtrlObject->logMsg(_("[TCP/IP srv] -- Accept.\n") );

                // We need to create a clientobject and add this object to the list
                pClientItem = new CClientItem;
                if ( NULL == pClientItem ) {
                    pCtrlObject->logMsg ( _( "[TCP/IP srv] Unable to allocate memory for client.\n" )  );
                    conn->flags |= MG_F_CLOSE_IMMEDIATELY;	// Close connection
                    return;
                }

                // save the client item
                conn->user_data = pClientItem;

                // This is now an active Client
                pClientItem->m_bOpen = true;
                pClientItem->m_type =  CLIENT_ITEM_INTERFACE_TYPE_CLIENT_TCPIP;
                pClientItem->m_strDeviceName = _("Remote TCP/IP Server. [");
                pClientItem->m_strDeviceName += gpobj->m_strTcpInterfaceAddress;
                pClientItem->m_strDeviceName += _("] Started at ");
                wxDateTime now = wxDateTime::Now();
                pClientItem->m_strDeviceName += wxDateTime::Now().FormatISODate();
                pClientItem->m_strDeviceName += _(" ");
                pClientItem->m_strDeviceName += wxDateTime::Now().FormatISOTime();

                // Add the client to the Client List
                pCtrlObject->m_wxClientMutex.Lock();
                pCtrlObject->addClient( pClientItem );
                pCtrlObject->m_wxClientMutex.Unlock();

                // Clear the filter (Allow everything )
                vscp_clearVSCPFilter( &pClientItem->m_filterVSCP );

                // Send welcome message
                wxString str = _(MSG_WELCOME);
                str += _("Version: ");
                str += _(VSCPD_DISPLAY_VERSION);
                str += _("\r\n");
                str += _(VSCPD_COPYRIGHT);
                str += _("\r\n");
                str += _(MSG_OK);
                mg_send( conn, (const char*)str.mbc_str(), str.Length() );

                pCtrlObject->logMsg(_("[TCP/IP srv] Ready to serve client.\n"),
                                        DAEMON_LOGMSG_DEBUG);
            }
            break;

        case MG_EV_CLOSE:

            // Close client
            pClientItem->m_bOpen = false;
            conn->flags |= MG_F_CLOSE_IMMEDIATELY;   // Close connection

            // Remove the client from the Client List
            pCtrlObject->m_wxClientMutex.Lock();
            pCtrlObject->removeClient( pClientItem );
            pCtrlObject->m_wxClientMutex.Unlock();
            // Remove client item
            conn->user_data = NULL;
            break;

        case MG_EV_RECV:

            if ( NULL == pClientItem ) {
                pCtrlObject->logMsg( _( "[TCP/IP srv] Remote client died\n" )  );
                conn->flags |= MG_F_CLOSE_IMMEDIATELY; // Close connection
                return;
            }

            if ( sizeof( rbuf ) < conn->recv_mbuf.len ) {
                pCtrlObject->logMsg( _("[TCP/IP srv] Received io->buf size exceeds limit.\n" )  );
                conn->flags |= MG_F_CLOSE_IMMEDIATELY; // Close connection
                return;
            }

            // Read new data
            memset( rbuf, 0, sizeof( rbuf ) );
            memcpy( rbuf, conn->recv_mbuf.buf, conn->recv_mbuf.len );
            
            mbuf_remove( &conn->recv_mbuf, conn->recv_mbuf.len );
            pClientItem->m_readBuffer += wxString::FromUTF8( rbuf );

            // Check if command already in buffer
            while ( wxNOT_FOUND != ( pos4lf = pClientItem->m_readBuffer.Find ( (const char)0x0a ) ) ) {
                wxString strCmdGo = pClientItem->m_readBuffer.Mid( 0, pos4lf );
                pCtrlObject->getTCPIPServer()->CommandHandler( conn,
                                                                pCtrlObject,
                                                                strCmdGo );
                pClientItem->m_readBuffer =
                        pClientItem->m_readBuffer.Right( pClientItem->m_readBuffer.Length()-pos4lf-1 );
            }
            break;

        case MG_EV_SEND:
            break;

        case MG_EV_POLL:
            
            if ( conn->flags & MG_F_USER_1) {

                pCtrlObject->getTCPIPServer()->sendOneEventFromQueue( conn, pCtrlObject, false );

                // Send '+OK<CR><LF>' every two seconds to indicate that the
                // link is open
                if ( ( wxGetUTCTime()-pClientItem->m_timeRcvLoop ) > 2 ) {
                    pClientItem->m_timeRcvLoop = wxGetUTCTime();
                    mg_send( conn, "+OK\r\n", 5 );
                }
                
            }
            break;

        default:
            break;
    }
}


///////////////////////////////////////////////////////////////////////////////
// CommandHandler
//

void
VSCPClientThread::CommandHandler( struct mg_connection *conn,
                                    CControlObject *pCtrlObject,
                                    wxString& strCommand )
{
    CClientItem *pClientItem = NULL;
    bool repeat = false;
        if ( NULL == conn ) {
        return;
    }

    pClientItem = ( CClientItem * )conn->user_data;

    if ( NULL == pCtrlObject ) {
        conn->flags |= MG_F_CLOSE_IMMEDIATELY;  // Close connection
        return;
    }

    if ( NULL == pClientItem ) {
        pCtrlObject->logMsg ( _( "[TCP/IP srv] ClientItem pointer is NULL in command handler.\n" )  );
        conn->flags |= MG_F_CLOSE_IMMEDIATELY;  // Close connection
        return;
    }

    pClientItem->m_currentCommand = strCommand;
    pClientItem->m_currentCommand.Trim(true);
    pClientItem->m_currentCommand.Trim(false);

    // If nothing to handle just return
    if ( 0 == pClientItem->m_currentCommand.Length() ) {
        mg_send( conn,  MSG_OK, strlen ( MSG_OK ) );
        return;
    }

    // If we are in a receive loop only the quitloop command works
    if ( conn->flags & MG_F_USER_1 ) {
        if ( pClientItem->CommandStartsWith( _("quitloop") ) ) {
            conn->flags &= ~(unsigned int)MG_F_USER_1;
            mg_send( conn, MSG_QUIT_LOOP, strlen ( MSG_QUIT_LOOP ) );
            return;
        }
        else {
            return;
        }
    }
     
REPEAT_COMMAND:

    //*********************************************************************
    //                            No Operation
    //*********************************************************************
    
    if ( pClientItem->CommandStartsWith( _("noop") ) ) {        
        mg_send( conn,  MSG_OK, strlen ( MSG_OK ) );
    }

    //*********************************************************************
    //                        + (repeat last command)
    //*********************************************************************

    else if ( pClientItem->CommandStartsWith( _("+") ) ) {
        // Repeat last command
        pClientItem->m_currentCommand = pClientItem->m_lastCommand;
        goto REPEAT_COMMAND;
    }

    //*********************************************************************
    //                           Username
    //*********************************************************************

    else if ( pClientItem->CommandStartsWith( _("user") ) ) {
        handleClientUser( conn, pCtrlObject );
    }

    //*********************************************************************
    //                            Password
    //*********************************************************************

    else if ( pClientItem->CommandStartsWith( _("pass") ) ) {                                                        
        if ( !handleClientPassword( conn, pCtrlObject ) ) {
            pCtrlObject->logMsg ( _( "[TCP/IP srv] Command: Password. Not authorized.\n" ),
                                    DAEMON_LOGMSG_NORMAL,
                                    DAEMON_LOGTYPE_SECURITY );
            conn->flags |= MG_F_CLOSE_IMMEDIATELY;  // Close connection
            return;
        }
    }

    //*********************************************************************
    //                           Challenge
    //*********************************************************************

    else if ( pClientItem->CommandStartsWith( _("challenge") ) ) {
        handleChallenge( conn, pCtrlObject );
    }

    // *********************************************************************
    //                                 QUIT
    // *********************************************************************

    else if ( pClientItem->CommandStartsWith( _("quit") ) ) {
        //long test = MG_F_CLOSE_IMMEDIATELY;
        pCtrlObject->logMsg( _( "[TCP/IP srv] Command: Close.\n" ) );
        mg_send( conn, MSG_GOODBY, strlen ( MSG_GOODBY ) );
        //conn->flags = NSF_FINISHED_SENDING_DATA;    // Close connection
        conn->flags = MG_F_SEND_AND_CLOSE;  // Close connection
        return;
    }

    //*********************************************************************
    //                              Shutdown
    //*********************************************************************
    else if ( pClientItem->CommandStartsWith(_("shutdown") ) ) {
        if ( checkPrivilege( conn, pCtrlObject, 15 ) ) {
            handleClientShutdown( conn, pCtrlObject );
        }
    }

    //*********************************************************************
    //                             Send event
    //*********************************************************************

    else if ( pClientItem->CommandStartsWith( _("send") ) ) {
        if ( checkPrivilege( conn, pCtrlObject, 4 ) ) {
            handleClientSend( conn, pCtrlObject );
        }
    }

    //*********************************************************************
    //                            Read event
    //********************************************************************* 

    else if ( pClientItem->CommandStartsWith( _("retr") ) ||
                pClientItem->CommandStartsWith( _("retrieve") ) ) {
        if ( checkPrivilege( conn, pCtrlObject, 2 ) ) {
            handleClientReceive( conn, pCtrlObject );
        }
    }

    //*********************************************************************
    //                            Data Available
    //*********************************************************************

    else if ( pClientItem->CommandStartsWith( _("cdta") ) ||
                pClientItem->CommandStartsWith( _("chkdata") ) ||
                pClientItem->CommandStartsWith( _("checkdata") ) ) {
        if ( checkPrivilege( conn, pCtrlObject, 1 ) ) {
            handleClientDataAvailable( conn, pCtrlObject );
        }
    }

    //*********************************************************************
    //                          Clear input queue
    //*********************************************************************

    else if ( pClientItem->CommandStartsWith( _("clra") ) ||
                pClientItem->CommandStartsWith( _("clearall") ) ||
                pClientItem->CommandStartsWith( _("clrall") ) ) {
        if ( checkPrivilege( conn, pCtrlObject, 1 ) ) {
            handleClientClearInputQueue( conn, pCtrlObject );
        }
    }


    //*********************************************************************
    //                           Get Statistics
    //*********************************************************************

    else if ( pClientItem->CommandStartsWith( _("stat") ) ) {
         if ( checkPrivilege( conn, pCtrlObject, 1 ) ) {
             handleClientGetStatistics( conn, pCtrlObject );
         }
    }

    //*********************************************************************
    //                            Get Status
    //*********************************************************************

    else if ( pClientItem->CommandStartsWith( _("info") ) ) {
        if ( checkPrivilege( conn, pCtrlObject, 1 ) ) {
            handleClientGetStatus( conn, pCtrlObject );
        }
    }

    //*********************************************************************
    //                           Get Channel ID
    //*********************************************************************

    else if ( pClientItem->CommandStartsWith( _("chid") ) ||
                pClientItem->CommandStartsWith( _("getchid") ) ) {
        if ( checkPrivilege( conn, pCtrlObject, 1 ) ) {
            handleClientGetChannelID( conn, pCtrlObject );
        }
    }

    //*********************************************************************
    //                          Set Channel GUID
    //*********************************************************************

    else if ( pClientItem->CommandStartsWith( _("sgid") ) ||
                pClientItem->CommandStartsWith( _("setguid") ) ) {
        if ( checkPrivilege( conn, pCtrlObject, 6 ) ) {
            handleClientSetChannelGUID( conn, pCtrlObject );
        }
    }

    //*********************************************************************
    //                          Get Channel GUID
    //*********************************************************************

    else if ( pClientItem->CommandStartsWith( _("ggid") ) ||
                pClientItem->CommandStartsWith( _("getguid") ) ) {
        if ( checkPrivilege( conn, pCtrlObject, 1 ) ) {
            handleClientGetChannelGUID( conn, pCtrlObject );
        }
    }

    //*********************************************************************
    //                           Get Version
    //*********************************************************************

    else if ( pClientItem->CommandStartsWith( _("version") ) ||
                    pClientItem->CommandStartsWith( _("vers") ) ) {
        handleClientGetVersion( conn, pCtrlObject );
    }

    //*********************************************************************
    //                           Set Filter
    //*********************************************************************

    else if ( pClientItem->CommandStartsWith( _("sflt") ) ||
                pClientItem->CommandStartsWith( _("setfilter") ) ) {
        if ( checkPrivilege( conn, pCtrlObject, 6 ) ) {
            handleClientSetFilter( conn, pCtrlObject );
        }
    }

    //*********************************************************************
    //                           Set Mask
    //*********************************************************************

    else if ( pClientItem->CommandStartsWith( _("smsk") ) || 
                pClientItem->CommandStartsWith( _("setmask") ) ) {
        if ( checkPrivilege( conn, pCtrlObject, 6 ) ) {
            handleClientSetMask( conn, pCtrlObject );
        }
    }


    //*********************************************************************
    //                             Rcvloop
    //*********************************************************************

    else if ( pClientItem->CommandStartsWith( _("rcvloop") ) || 
                pClientItem->CommandStartsWith( _("receiveloop") ) ) {
        if ( checkPrivilege( conn, pCtrlObject, 2 ) ) {
            pClientItem->m_timeRcvLoop = wxGetUTCTime();
            handleClientRcvLoop( conn, pCtrlObject );
        }
    }


    //*********************************************************************
    //                             Help
    //*********************************************************************

    else if ( pClientItem->CommandStartsWith( _("help") ) ) {
        handleClientHelp( conn, pCtrlObject );
    }

    //*********************************************************************
    //                             Restart

    else if ( pClientItem->CommandStartsWith( _("restart") ) ) {
        if ( checkPrivilege( conn, pCtrlObject, 15 ) ) {
            handleClientRestart( conn, pCtrlObject );
        }
    }

    //*********************************************************************
    //                             Driver
    //*********************************************************************

    else if ( pClientItem->CommandStartsWith( _("driver") ) ) {
        if ( checkPrivilege( conn, pCtrlObject, 15 ) ) {
            handleClientDriver( conn, pCtrlObject );
        }
    }

    //*********************************************************************
    //                               DM
    //*********************************************************************

    else if ( pClientItem->CommandStartsWith( _("dm") ) ) {
        if ( checkPrivilege( conn, pCtrlObject, 15 ) ) {
            handleClientDm( conn, pCtrlObject );
        }
    }

    //*********************************************************************
    //                             Variable
    //*********************************************************************

    else if ( pClientItem->CommandStartsWith( _("variable") ) ||
                pClientItem->CommandStartsWith( _("var") ) ) {
        if ( checkPrivilege( conn, pCtrlObject, 4 ) ) {
            handleClientVariable( conn, pCtrlObject );
        }
    }

    //*********************************************************************
    //                               File
    //*********************************************************************

    else if ( pClientItem->CommandStartsWith( _("file") ) ) {
        if ( checkPrivilege( conn, pCtrlObject, 15 ) ) {
            handleClientFile( conn, pCtrlObject );
        }
    }

    //*********************************************************************
    //                               UDP
    //*********************************************************************

    else if ( pClientItem->CommandStartsWith( _("udp") ) ) {
        if ( checkPrivilege( conn, pCtrlObject, 15 ) ) {
            handleClientUdp( conn, pCtrlObject );
        }
    }

    //*********************************************************************
    //                         Client/interface
    //*********************************************************************

    else if ( pClientItem->CommandStartsWith( _("client") ) ||
                pClientItem->CommandStartsWith( _("interface") ) ) {
        if ( checkPrivilege( conn, pCtrlObject, 15 ) ) {
            handleClientInterface( conn, pCtrlObject );
        }
    }


    //*********************************************************************
    //                               Test
    //*********************************************************************

    else if ( pClientItem->CommandStartsWith( _("test") ) ) {
        if ( checkPrivilege( conn, pCtrlObject, 15 ) ) {
            handleClientTest( conn, pCtrlObject );
        }
    }


    //*********************************************************************
    //                             WhatCanYouDo
    //*********************************************************************
    
    else if ( pClientItem->CommandStartsWith( _("wcyd") ) ||
                pClientItem->CommandStartsWith( _("whatcanyoudo") ) ) {
        handleClientCapabilityRequest( conn, pCtrlObject );
    }

    //*********************************************************************
    //                             Measurement
    //*********************************************************************
    
    else if ( pClientItem->CommandStartsWith( _("measurement") ) ) {
        handleClientMeasurment( conn, pCtrlObject );
    }

    //*********************************************************************
    //                                Table
    //*********************************************************************
    
    else if ( pClientItem->CommandStartsWith( _("table") ) ) {
        handleClientTable( conn, pCtrlObject );
    }

    //*********************************************************************
    //                                What?
    //*********************************************************************
    else {
        mg_send( conn,  MSG_UNKNOWN_COMMAND, strlen ( MSG_UNKNOWN_COMMAND ) );
    }

    pClientItem->m_lastCommand = pClientItem->m_currentCommand;

}


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

void VSCPClientThread::handleClientMeasurment( struct mg_connection *conn,
                                                 CControlObject *pCtrlObject )
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
    
    // Check objects
    if ( ( NULL == conn ) || (NULL == pCtrlObject )  ) {
        mg_send( conn,  MSG_INTERNAL_MEMORY_ERROR, strlen ( MSG_INTERNAL_MEMORY_ERROR ) );
        return;
    }
    
    CClientItem *pClientItem = (CClientItem *)conn->user_data;
    if ( NULL == pClientItem ) {
        mg_send( conn,  MSG_INTERNAL_MEMORY_ERROR, strlen ( MSG_INTERNAL_MEMORY_ERROR ) );
        return;
    }
    
    wxStringTokenizer tkz( pClientItem->m_currentCommand,
                            _(",") );

    // If first character is $ user request us to send content from
    // a variable

    // * * * event format * * *
    
    // Get event format (float | string | 0 | 1 - float=0, string=1.)
    if ( !tkz.HasMoreTokens() ) {
        mg_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    wxstr = tkz.GetNextToken();
    wxstr.MakeUpper();
    if ( wxstr.IsNumber() ) {
        
        l = 0;
  
        if ( wxstr.ToULong( &l ) && ( l > 1 ) ) {
            mg_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
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
            mg_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            return;
        }
    }
    
    // * * * Level * * *
    
    if ( !tkz.HasMoreTokens() ) {
        mg_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    wxstr = tkz.GetNextToken();
    
    if ( wxstr.IsNumber() ) {
        
        l = VSCP_LEVEL1; 
  
        if ( wxstr.ToULong( &l ) && ( l > VSCP_LEVEL2 ) ) {
            mg_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            return;
        }
        
        level = l;
        
    }
    else {
        mg_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // * * * vscp-measurement-type * * *
    
    if ( !tkz.HasMoreTokens() ) {
        mg_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    wxstr = tkz.GetNextToken();
    
    if ( wxstr.IsNumber() ) {
        
        l = 0; 
  
        if ( wxstr.ToULong( &l ) ) {
            mg_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            return;
        }
        
        level = l;
        
    }
    else {
        mg_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
        return;
    }
    

    // * * * value * * *
    
        
    if ( !tkz.HasMoreTokens() ) {
        mg_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
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
            mg_send( conn, MSG_VARIABLE_NOT_DEFINED, strlen ( MSG_VARIABLE_NOT_DEFINED ) );
            return;
        }
        
        // get the value
        wxstr = variable.getValue();
        if ( !wxstr.IsNumber() ) {
            mg_send( conn, MSG_VARIABLE_NOT_NUMERIC, strlen ( MSG_VARIABLE_NOT_NUMERIC ) );
            return;
        }
        
        if ( wxstr.ToCDouble( &value ) ) {
            mg_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            return;
        }
        
    }
    else {
        if ( !wxstr.ToCDouble( &value ) ) {
            mg_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
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
                    mg_send( conn,  MSG_INTERNAL_MEMORY_ERROR, strlen ( MSG_INTERNAL_MEMORY_ERROR ) );
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
                if ( !pCtrlObject->sendEvent( pClientItem, pEvent ) ) {
                    mg_send( conn,  MSG_UNABLE_TO_SEND_EVENT, strlen ( MSG_UNABLE_TO_SEND_EVENT ) );
                    return;
                }

            } 
            else {
                mg_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            }
        } 
        else {
            
            // * * * String * * *
            
            vscpEvent *pEvent = new vscpEvent;
            if (NULL == pEvent) {
                mg_send( conn,  MSG_INTERNAL_MEMORY_ERROR, strlen ( MSG_INTERNAL_MEMORY_ERROR ) );
                return;
            }
            
            pEvent->pdata = NULL;

            if ( !vscp_makeStringMeasurementEvent( pEvent,
                                                    value,
                                                    unit,
                                                    sensoridx ) ) {
                mg_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            }
            
        }
    } 
    else {      // Level II
        
        if ( 0 == eventFormat ) {   // float and Level II

            // * * * Floating point * * *
            
            vscpEvent *pEvent = new vscpEvent;
            if ( NULL == pEvent ) {
                mg_send( conn,  MSG_INTERNAL_MEMORY_ERROR, strlen ( MSG_INTERNAL_MEMORY_ERROR ) );
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
                mg_send( conn,  MSG_INTERNAL_MEMORY_ERROR, strlen ( MSG_INTERNAL_MEMORY_ERROR ) );
                delete pEvent;
                return;
            }
            
            memcpy(pEvent->pdata, data, 4 + 8);

            // send the event
            if ( !pCtrlObject->sendEvent( pClientItem, pEvent ) ) {
                mg_send( conn,  MSG_UNABLE_TO_SEND_EVENT, strlen ( MSG_UNABLE_TO_SEND_EVENT ) );
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
                mg_send( conn,  MSG_INTERNAL_MEMORY_ERROR, strlen ( MSG_INTERNAL_MEMORY_ERROR ) );
                delete pEvent;
                return;
            }
            memcpy(data + 4, strValue.mbc_str(), strValue.Length()); // copy in double

            // send the event
            if ( !pCtrlObject->sendEvent( pClientItem, pEvent ) ) {
                mg_send( conn,  MSG_UNABLE_TO_SEND_EVENT, strlen ( MSG_UNABLE_TO_SEND_EVENT ) );
                return;
            }
            
        }
    }
   
    mg_send( conn, MSG_OK, strlen ( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleClientCapabilityRequest
//

void VSCPClientThread::handleClientCapabilityRequest( struct mg_connection *conn,
                                                        CControlObject *pCtrlObject )
{
    wxString wxstr;
    uint8_t capabilities[16];
    
    gpobj->getVscpCapabilities( capabilities );
    wxstr = wxString::Format(_("%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X"),                                
                                capabilities[7], capabilities[6], capabilities[5], capabilities[4],
                                capabilities[3], capabilities[2], capabilities[1], capabilities[0] );
    mg_send( conn,  MSG_UNKNOWN_COMMAND, strlen ( MSG_UNKNOWN_COMMAND ) );
}


///////////////////////////////////////////////////////////////////////////////
// isVerified
//

bool VSCPClientThread::isVerified( struct mg_connection *conn,
                                        CControlObject *pCtrlObject )
{
    // Check objects
    if ( ( NULL == conn ) || (NULL == pCtrlObject )  ) {
        return false;
    }
    
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    // Must be accredited to do this
    if ( !pClientItem->bAuthenticated ) {
        mg_send( conn, MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// checkPrivilege
//

bool VSCPClientThread::checkPrivilege( struct mg_connection *conn,
                                            CControlObject *pCtrlObject,
                                            unsigned char reqiredPrivilege )
{
    // Check objects
    if ( ( NULL == conn ) || (NULL == pCtrlObject )  ) {
        return false;
    }
    
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    // Must be loged on
    if ( !pClientItem->bAuthenticated ) {
        mg_send( conn,  MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
        return false;
    }

    if ( NULL == pClientItem->m_pUserItem ) {
        mg_send( conn,  MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
        return false;
    }

    if ( (pClientItem->m_pUserItem->getUserRights( 0 ) & USER_PRIVILEGE_MASK ) < reqiredPrivilege ) {
        mg_send( conn,  MSG_LOW_PRIVILEGE_ERROR, strlen ( MSG_LOW_PRIVILEGE_ERROR ) );
        return false;
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientSend
//

void VSCPClientThread::handleClientSend( struct mg_connection *conn,
                                            CControlObject *pCtrlObject )
{
    bool bSent = false;
    bool bVariable = false;
    vscpEvent event;
    wxString nameVariable;
    CClientItem *pClientItem = NULL;
    if ( NULL == conn ) {
        return;
    }
    
    // Set timestamp block for event
    vscp_setEventDateTimeBlockToNow( &event );

    if ( NULL == pCtrlObject ) {
        mg_send( conn, MSG_PARAMETER_ERROR, strlen( MSG_PARAMETER_ERROR ) );
        return;
    }

    pClientItem = ( CClientItem * )conn->user_data;

    if ( NULL == pClientItem ) {
        mg_send( conn, MSG_INTERNAL_ERROR, strlen( MSG_INTERNAL_ERROR ) );
        return;
    }

    // Must be accredited to do this
    if ( !pClientItem->bAuthenticated ) {
        mg_send( conn, MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    wxString str;
    wxStringTokenizer tkz( pClientItem->m_currentCommand, _(",") );

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
                mg_send( conn, MSG_VARIABLE_NOT_DEFINED, strlen ( MSG_VARIABLE_NOT_DEFINED ) );
                return;
            }

            // Must be event type
            if ( VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT != variable.getType() ) {
                mg_send( conn, MSG_VARIABLE_MUST_BE_EVENT_TYPE, strlen ( MSG_VARIABLE_MUST_BE_EVENT_TYPE ) );
                return;
            }

            // Get the event
            variable.getValue( &event );

        }
    }
    else {
        mg_send( conn, MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
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
            mg_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            return;
        }

        // Get Type
        if ( tkz.HasMoreTokens() ) {
            str = tkz.GetNextToken();
            event.vscp_type = vscp_readStringValue( str );
        }
        else {
            mg_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            return;
        }

        // Get OBID  -  Kept here to be compatible with receive
        if ( tkz.HasMoreTokens() ) {
            str = tkz.GetNextToken();
            event.obid = vscp_readStringValue( str );
        }
        else {
            mg_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
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
            mg_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
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
            mg_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            return;
        }

        // Get GUID
        wxString strGUID;
        if ( tkz.HasMoreTokens() ) {
            strGUID = tkz.GetNextToken();

            // Check if i/f GUID should be used
            if ( ( '-' == strGUID[0] ) || vscp_isGUIDEmpty( event.GUID ) ) {
                // Copy in the i/f GUID
                pClientItem->m_guid.writeGUID( event.GUID );
            }
            else {
                vscp_getGuidFromString( &event, strGUID );
            }

        }
        else {
            mg_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            return;
        }

        // Handle data
        if ( 512 < tkz.CountTokens() ) {
            mg_send( conn, MSG_PARAMETER_ERROR, strlen( MSG_PARAMETER_ERROR ) );
            return;
        }

        event.sizeData = tkz.CountTokens();

        if ( event.sizeData > 0 ) {

            unsigned int index = 0;

            event.pdata = new uint8_t[ event.sizeData ];

            if ( NULL == event.pdata ) {
                mg_send( conn, MSG_INTERNAL_MEMORY_ERROR, strlen( MSG_INTERNAL_MEMORY_ERROR ) );
                return;
            }

            while ( tkz.HasMoreTokens() && ( event.sizeData > index ) ) {
                str = tkz.GetNextToken();
                event.pdata[ index++ ] = vscp_readStringValue( str );
            }

            if ( tkz.HasMoreTokens() ) {
                mg_send( conn, MSG_PARAMETER_ERROR, strlen( MSG_PARAMETER_ERROR ) );
                return;
            }
        }
        else {
            // No data
            event.pdata = NULL;
        }

    } // not variable send

    // Check if this user is allowed to send this event
    if ( !pClientItem->m_pUserItem->isUserAllowedToSendEvent( event.vscp_class, event.vscp_type ) ) {
        wxString strErr =
                        wxString::Format( _("[TCP/IP srv] User [%s] not allowed to send event class=%d type=%d.\n"),
                                                (const char *)pClientItem->m_pUserItem->getUserName().mbc_str(),
                                                event.vscp_class, event.vscp_type );

        pCtrlObject->logMsg ( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_SECURITY );

        mg_send( conn, MSG_MOT_ALLOWED_TO_SEND_EVENT, strlen ( MSG_MOT_ALLOWED_TO_SEND_EVENT ) );

        if ( NULL != event.pdata ) {
            delete [] event.pdata;
            event.pdata = NULL;
        }

        return;
    }
    
    // send event
    if ( !gpobj->sendEvent( pClientItem, &event ) ) {
        mg_send( conn,  MSG_BUFFER_FULL, strlen ( MSG_BUFFER_FULL ) );
        return;
    }
    
/*
    vscpEvent *pEvent = new vscpEvent;		// Create new VSCP Event
    if ( NULL != pEvent ) {

        // Copy event
        vscp_copyVSCPEvent( pEvent, &event );

        // We don't need the original event anymore
        if ( NULL != event.pdata ) {
            delete [] event.pdata;
            event.pdata = NULL;
            event.sizeData = 0;
        }

        // Save the originating clients id so
        // this client don't get the message back
        pEvent->obid = pClientItem->m_clientID;


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
                    gpobj->logMsg( dbgStr, DAEMON_LOGMSG_DEBUG );

            // Find client
            gpobj->m_wxClientMutex.Lock();

            CClientItem *pDestClientItem = NULL;
            VSCPCLIENTLIST::iterator iter;
            for (iter = gpobj->m_clientList.m_clientItemList.begin();
                    iter != gpobj->m_clientList.m_clientItemList.end();
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
                    gpobj->logMsg( dbgStr, DAEMON_LOGMSG_DEBUG );

                    if ( pItem->m_guid == destguid ) {
                        // Found
                        pDestClientItem = pItem;
                        bSent = true;
                        dbgStr = _("Match ");
                        gpobj->logMsg( dbgStr, DAEMON_LOGMSG_DEBUG );
                        gpobj->sendEventToClient( pItem, pEvent );
                        break;
                    }

                }

                gpobj->m_wxClientMutex.Unlock();

        }


        if ( !bSent ) {

            // There must be room in the send queue
            if ( gpobj->m_maxItemsInClientReceiveQueue >
                gpobj->m_clientOutputQueue.GetCount() ) {

                    gpobj->m_mutexClientOutputQueue.Lock();
                    gpobj->m_clientOutputQueue.Append ( pEvent );
                    gpobj->m_semClientOutputQueue.Post();
                    gpobj->m_mutexClientOutputQueue.Unlock();

                    // TX Statistics
                    pClientItem->m_statistics.cntTransmitData += pEvent->sizeData;
                    pClientItem->m_statistics.cntTransmitFrames++;

            }
            else {

                pClientItem->m_statistics.cntOverruns++;

                vscp_deleteVSCPevent( pEvent );
                mg_send( conn,  MSG_BUFFER_FULL, strlen ( MSG_BUFFER_FULL ) );
            }

        }

    } // Valid pEvent
*/    

   mg_send( conn, MSG_OK, strlen ( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleClientReceive
//

void VSCPClientThread::handleClientReceive ( struct mg_connection *conn,
                                                CControlObject *pCtrlObject )
{
    unsigned short cnt = 0;	// # of messages to read
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    // Must be accredited to do this
    if ( !pClientItem->bAuthenticated ) {
        mg_send( conn,  MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    wxString str;
    cnt = vscp_readStringValue( pClientItem->m_currentCommand );

    if ( !cnt ) cnt = 1;	// No arg is "read one"


    // Read cnt messages
    while ( cnt ) {

        wxString strOut;

        if ( !pClientItem->m_bOpen ) {
            mg_send( conn,  MSG_NO_MSG, strlen ( MSG_NO_MSG ) );
            return;
        }
        else {
            if ( false == sendOneEventFromQueue( conn, pCtrlObject ) ) {
                return;
            }
        }

        cnt--;

    } // while

    mg_send( conn,  MSG_OK, strlen ( MSG_OK ) );

}

///////////////////////////////////////////////////////////////////////////////
// sendOneEventFromQueue
//

bool VSCPClientThread::sendOneEventFromQueue( struct mg_connection *conn,
                                                CControlObject *pCtrlObject,
                                                bool bStatusMsg )
{
    wxString strOut;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    CLIENTEVENTLIST::compatibility_iterator nodeClient;

    if ( pClientItem->m_clientInputQueue.GetCount() ) {

        vscpEvent *pqueueEvent;
        pClientItem->m_mutexClientInputQueue.Lock();
        {
            nodeClient = pClientItem->m_clientInputQueue.GetFirst();
            pqueueEvent = nodeClient->GetData();

            // Remove the node
            pClientItem->m_clientInputQueue.DeleteNode ( nodeClient );
        }
        pClientItem->m_mutexClientInputQueue.Unlock();

        vscp_writeVscpEventToString( pqueueEvent, strOut );
        strOut += _("\r\n");
        mg_send( conn,  strOut.mb_str(), strlen ( strOut.mb_str() ) );

        //delete pqueueEvent;
        vscp_deleteVSCPevent( pqueueEvent );

        // Let the system work a little
        //ns_mgr_poll( &gpobj->m_mgrTcpIpServer, 1 );
    }
    else {
        if ( bStatusMsg ) {
            mg_send( conn,  MSG_NO_MSG, strlen ( MSG_NO_MSG ) );
        }

        return false;

    }

    return true;

}



///////////////////////////////////////////////////////////////////////////////
// handleClientDataAvailable
//

void VSCPClientThread::handleClientDataAvailable ( struct mg_connection *conn,
                                                    CControlObject *pCtrlObject )
{
    char outbuf[ 1024 ];
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    // Must be accredited to do this
    if ( !pClientItem->bAuthenticated ) {
        mg_send( conn,  MSG_NOT_ACCREDITED,
            strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    sprintf ( outbuf,
        "%zd\r\n%s",
        pClientItem->m_clientInputQueue.GetCount(),
        MSG_OK );
    mg_send( conn,  outbuf, strlen ( outbuf ) );


}

///////////////////////////////////////////////////////////////////////////////
// handleClientClearInputQueue
//

void VSCPClientThread::handleClientClearInputQueue ( struct mg_connection *conn,
                                                        CControlObject *pCtrlObject )
{
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    // Must be accredited to do this
    if ( !pClientItem->bAuthenticated ) {
        mg_send( conn,  MSG_NOT_ACCREDITED,
            strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    pClientItem->m_mutexClientInputQueue.Lock();
    pClientItem->m_clientInputQueue.Clear();
    pClientItem->m_mutexClientInputQueue.Unlock();

    mg_send( conn,  MSG_QUEUE_CLEARED, strlen ( MSG_QUEUE_CLEARED ) );
}


///////////////////////////////////////////////////////////////////////////////
// handleClientGetStatistics
//

void VSCPClientThread::handleClientGetStatistics ( struct mg_connection *conn,
                                                        CControlObject *pCtrlObject )
{
    char outbuf[ 1024 ];
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    // Must be accredited to do this
    if ( !pClientItem->bAuthenticated ) {
        mg_send( conn,  MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    sprintf ( outbuf, "%lu,%lu,%lu,%lu,%lu,%lu,%lu\r\n%s",
        pClientItem->m_statistics.cntBusOff,
        pClientItem->m_statistics.cntBusWarnings,
        pClientItem->m_statistics.cntOverruns,
        pClientItem->m_statistics.cntReceiveData,
        pClientItem->m_statistics.cntReceiveFrames,
        pClientItem->m_statistics.cntTransmitData,
        pClientItem->m_statistics.cntTransmitFrames,
        MSG_OK );


    mg_send( conn, outbuf, strlen ( outbuf ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleClientGetStatus
//

void VSCPClientThread::handleClientGetStatus ( struct mg_connection *conn,
                                                    CControlObject *pCtrlObject )
{
    char outbuf[ 1024 ];
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    // Must be accredited to do this
    if ( !pClientItem->bAuthenticated ) {
        mg_send( conn,  MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    sprintf ( outbuf, "%lu,%lu,%lu,\"%s\"\r\n%s",
        pClientItem->m_status.channel_status,
        pClientItem->m_status.lasterrorcode,
        pClientItem->m_status.lasterrorsubcode,
        pClientItem->m_status.lasterrorstr,
        MSG_OK );

    mg_send( conn, outbuf, strlen ( outbuf ) );


}

///////////////////////////////////////////////////////////////////////////////
// handleClientGetChannelID
//

void VSCPClientThread::handleClientGetChannelID ( struct mg_connection *conn,
                                                    CControlObject *pCtrlObject )
{
    char outbuf[ 1024 ];
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    // Must be accredited to do this
    if ( !pClientItem->bAuthenticated ) {
        mg_send( conn,  MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    sprintf ( outbuf, "%lu\r\n%s",
        (unsigned long)pClientItem->m_clientID, MSG_OK );

    mg_send( conn,  outbuf, strlen ( outbuf ) );

}


///////////////////////////////////////////////////////////////////////////////
// handleClientSetChannelGUID
//

void VSCPClientThread::handleClientSetChannelGUID ( struct mg_connection *conn,
                                                        CControlObject *pCtrlObject )
{
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    // Must be accredited to do this
    if ( !pClientItem->bAuthenticated ) {
        mg_send( conn,  MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    pClientItem->m_currentCommand.Trim(true);
    pClientItem->m_currentCommand.Trim(false);
    
    pClientItem->m_guid.getFromString( pClientItem->m_currentCommand );
    mg_send( conn,  MSG_OK, strlen ( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleClientGetChannelGUID
//

void VSCPClientThread::handleClientGetChannelGUID ( struct mg_connection *conn,
                                                        CControlObject *pCtrlObject )
{
    wxString strBuf;
    //char outbuf[ 1024 ];
    //char wrkbuf[ 20 ];
    //int i;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    // Must be accredited to do this
    if ( !pClientItem->bAuthenticated ) {
        mg_send( conn, MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }


    pClientItem->m_guid.toString( strBuf );
    strBuf += _("\r\n");
    strBuf += _(MSG_OK);

    mg_send( conn, strBuf.mb_str(), strlen( strBuf.mb_str() ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleClientGetVersion
//

void VSCPClientThread::handleClientGetVersion ( struct mg_connection *conn,
                                                    CControlObject *pCtrlObject )
{
    char outbuf[ 1024 ];

    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    // Must be accredited to do this
    if ( !pClientItem->bAuthenticated ) {
        mg_send( conn,  MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }


    sprintf ( outbuf,
                "%d,%d,%d\r\n%s",
                VSCPD_MAJOR_VERSION,
                VSCPD_MINOR_VERSION,
                VSCPD_RELEASE_VERSION,
                MSG_OK );

    mg_send( conn,  outbuf, strlen ( outbuf ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleClientSetFilter
//

void VSCPClientThread::handleClientSetFilter ( struct mg_connection *conn,
                                                    CControlObject *pCtrlObject )
{
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    // Must be accredited to do this
    if ( !pClientItem->bAuthenticated ) {
        mg_send( conn,  MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    wxString str;
    pClientItem->m_currentCommand.Trim(true);
    pClientItem->m_currentCommand.Trim(false);
    wxStringTokenizer tkz( pClientItem->m_currentCommand, _(",") );

    // Get priority
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        pClientItem->m_filterVSCP.filter_priority = vscp_readStringValue( str );
    }
    else {
        mg_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
        return;
    }

    // Get Class
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        pClientItem->m_filterVSCP.filter_class = vscp_readStringValue( str );
    }
    else {
        mg_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
        return;
    }


    // Get Type
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        pClientItem->m_filterVSCP.filter_type = vscp_readStringValue( str );
    }
    else {
        mg_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
        return;
    }

    // Get GUID
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        vscp_getGuidFromStringToArray( pClientItem->m_filterVSCP.filter_GUID, str );
    }
    else {
        mg_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
        return;
    }

    mg_send( conn,  MSG_OK, strlen ( MSG_OK ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleClientSetMask
//

void VSCPClientThread::handleClientSetMask ( struct mg_connection *conn,
                                                CControlObject *pCtrlObject )
{
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    // Must be accredited to do this
    if ( !pClientItem->bAuthenticated ) {
        mg_send( conn,  MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    wxString str;
    pClientItem->m_currentCommand.Trim(true);
    pClientItem->m_currentCommand.Trim(false);
    wxStringTokenizer tkz( pClientItem->m_currentCommand, _(",") );

    // Get priority
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        pClientItem->m_filterVSCP.mask_priority = vscp_readStringValue( str );
    }
    else {
        mg_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
        return;
    }

    // Get Class
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        pClientItem->m_filterVSCP.mask_class = vscp_readStringValue( str );
    }
    else {
        mg_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
        return;
    }


    // Get Type
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        pClientItem->m_filterVSCP.mask_type = vscp_readStringValue( str );
    }
    else {
        mg_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
        return;
    }

    // Get GUID
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        vscp_getGuidFromStringToArray( pClientItem->m_filterVSCP.mask_GUID, str );
    }
    else {
        mg_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
        return;
    }

    mg_send( conn,  MSG_OK, strlen ( MSG_OK ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleClientUser
//

void VSCPClientThread::handleClientUser ( struct mg_connection *conn,
                                            CControlObject *pCtrlObject )
{
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    if ( pClientItem->bAuthenticated ) {
        mg_send( conn,  MSG_OK, strlen ( MSG_OK ) );
        return;
    }

    pClientItem->m_UserName = pClientItem->m_currentCommand;
    pClientItem->m_UserName.Trim(true);     // Trim right side
    pClientItem->m_UserName.Trim(false );   // Trim left
    if ( pClientItem->m_UserName.IsEmpty() ) {
        mg_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
        return;
    }

    mg_send( conn,  MSG_USENAME_OK, strlen ( MSG_USENAME_OK ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleClientPassword
//

bool VSCPClientThread::handleClientPassword ( struct mg_connection *conn,
                                                CControlObject *pCtrlObject )
{
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    if ( pClientItem->bAuthenticated ) {
        mg_send( conn,  MSG_OK, strlen ( MSG_OK ) );
        return true;
    }

    // Must have username before password can be entered.
    if ( 0 == pClientItem->m_UserName.Length() ) {
        mg_send( conn,  MSG_NEED_USERNAME, strlen ( MSG_NEED_USERNAME ) );
        return true;
    }

    wxString strPassword = pClientItem->m_currentCommand;
    strPassword.Trim(true);         // Trim right side
    strPassword.Trim(false);        // Trim left

    if ( strPassword.IsEmpty() ) {
        pClientItem->m_UserName = _("");
        mg_send( conn,  MSG_PARAMETER_ERROR, strlen( MSG_PARAMETER_ERROR ) );
        return false;
    }

    gpobj->m_mutexUserList.Lock();
#if  0
    ::wxLogDebug( _("Username: ") + m_UserName );
    ::wxLogDebug( _("Password: ") + strPassword );
    ::wxLogDebug( _("MD5 of Password: ") + md5Password );
#endif
    pClientItem->m_pUserItem = gpobj->m_userList.validateUser( pClientItem->m_UserName, strPassword );
    gpobj->m_mutexUserList.Unlock();

    if ( NULL == pClientItem->m_pUserItem ) {

        wxString strErr =
            wxString::Format(_("[TCP/IP srv] User [%s][%s] not allowed to connect.\n"),
            (const char *)pClientItem->m_UserName.mbc_str(), (const char *)strPassword.mbc_str() );

        pCtrlObject->logMsg( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_SECURITY );
        mg_send( conn,  MSG_PASSWORD_ERROR, strlen ( MSG_PASSWORD_ERROR ) );
        return false;
    }

    // Get remote address
    struct sockaddr_in cli_addr;
    socklen_t clilen = 0;
    clilen = sizeof (cli_addr);
    (void)getpeername( conn->sock, (struct sockaddr *)&cli_addr, &clilen);
    wxString remoteaddr = wxString::FromAscii( inet_ntoa( cli_addr.sin_addr ) );

    // Check if this user is allowed to connect from this location
    gpobj->m_mutexUserList.Lock();
    bool bValidHost =
            pClientItem->m_pUserItem->isAllowedToConnect( remoteaddr );
    gpobj->m_mutexUserList.Unlock();

    if ( !bValidHost ) {
        wxString strErr = wxString::Format(_("[TCP/IP srv] Host [%s] not allowed to connect.\n"),
            (const char *)remoteaddr.c_str() );

        pCtrlObject->logMsg ( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_SECURITY );
        mg_send( conn,  MSG_INVALID_REMOTE_ERROR, strlen ( MSG_INVALID_REMOTE_ERROR ) );
        return false;
    }

    // Copy in the user filter
    memcpy( &pClientItem->m_filterVSCP,
                pClientItem->m_pUserItem->getFilter(),
                sizeof( vscpEventFilter ) );

    wxString strErr =
        wxString::Format( _("[TCP/IP srv] Host [%s] User [%s] allowed to connect.\n"),
                            (const char *)remoteaddr.c_str(),
                            (const char *)pClientItem->m_UserName.c_str() );

    pCtrlObject->logMsg ( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_SECURITY );

    pClientItem->bAuthenticated = true;
    mg_send( conn,  MSG_OK, strlen ( MSG_OK ) );

    return true;

}

///////////////////////////////////////////////////////////////////////////////
// handleChallenge
//

void VSCPClientThread::handleChallenge( struct mg_connection *conn,
                                            CControlObject *pCtrlObject )
{
    wxString wxstr;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    pClientItem->m_currentCommand.Trim(true);
    pClientItem->m_currentCommand.Trim(false);

    memset( pClientItem->m_sid, 0, sizeof( pClientItem->m_sid ) );
    if ( !gpobj->generateSessionId( (const char *)pClientItem->m_currentCommand.mbc_str(), 
                                    pClientItem->m_sid ) ) {
        mg_send( conn,  MSG_FAILD_TO_GENERATE_SID, strlen ( MSG_FAILD_TO_GENERATE_SID ) );
        return; 
    }
    
    wxstr = _("+OK - ") + wxString::FromUTF8( pClientItem->m_sid ) + _("\r\n");
    mg_send( conn, (const char *)wxstr.mbc_str(), strlen ( (const char *)wxstr.mbc_str() ) );
    
    //mg_send( conn, MSG_OK, strlen ( MSG_OK ) );
}


///////////////////////////////////////////////////////////////////////////////
// handleClientRcvLoop
//

void VSCPClientThread::handleClientRcvLoop( struct mg_connection *conn,
                                                CControlObject *pCtrlObject  )
{

    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    mg_send( conn,  MSG_RECEIVE_LOOP, strlen ( MSG_RECEIVE_LOOP ) );
    conn->flags |= MG_F_USER_1; // Mark socket as being in receive loop
    
    pClientItem->m_readBuffer.Empty();

    // Loop until the connection is lost
    /*
    while ( !TestDestroy() && !m_bQuit && (conn->flags & MG_F_USER_1 ) ) {

        // Wait for event
        if ( wxSEMA_TIMEOUT ==
            pClientItem->m_semClientInputQueue.WaitTimeout( 1000 ) ) {
                mg_send( conn, "+OK\r\n", 5 );
                continue;
        }
        
        // We must handle the polling here while in the loop
        mg_mgr_poll( &gpobj->m_mgrTcpIpServer, 50 );


    } // While 
    */
    
    return;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientTest
//

void VSCPClientThread::handleClientTest ( struct mg_connection *conn,
                                            CControlObject *pCtrlObject )
{
    mg_send( conn, MSG_OK, strlen ( MSG_OK ) );
    return;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientRestart
//

void VSCPClientThread::handleClientRestart ( struct mg_connection *conn,
                                                    CControlObject *pCtrlObject )
{
    mg_send( conn, MSG_OK, strlen ( MSG_OK ) );
    return;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientShutdown
//

void VSCPClientThread::handleClientShutdown ( struct mg_connection *conn,
                                                CControlObject *pCtrlObject )
{
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    if ( !pClientItem->bAuthenticated ) {
        mg_send( conn,  MSG_OK, strlen ( MSG_OK ) );
    }

    mg_send( conn,  MSG_GOODBY, strlen ( MSG_GOODBY ) );
    conn->flags |= MG_F_CLOSE_IMMEDIATELY;
    //gpobj->m_bQuit = true;
    //m_bRun = false;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientRemote
//

void VSCPClientThread::handleClientRemote( struct mg_connection *conn,
                                                CControlObject *pCtrlObject )
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

void VSCPClientThread::handleClientInterface( struct mg_connection *conn, 
                                                CControlObject *pCtrlObject )
{
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    gpobj->logMsg ( pClientItem->m_currentCommand, DAEMON_LOGMSG_NORMAL );

    if ( pClientItem->CommandStartsWith(_("list") ) ) {
        handleClientInterface_List( conn, pCtrlObject );
    }
    else if ( pClientItem->CommandStartsWith(_("unique") ) ) {
        handleClientInterface_Unique( conn, pCtrlObject );
    }
    else if ( pClientItem->CommandStartsWith(_("normal") ) ) {
        handleClientInterface_Normal( conn, pCtrlObject );
    }
    else if ( pClientItem->CommandStartsWith(_("close") ) ) {
        handleClientInterface_Close( conn, pCtrlObject );
    }
}

///////////////////////////////////////////////////////////////////////////////
// handleClientInterface_List
//

void VSCPClientThread::handleClientInterface_List( struct mg_connection *conn,
                                                        CControlObject *pCtrlObject )
{
    wxString strGUID;
    wxString strBuf;

    // Display Interface List
    gpobj->m_wxClientMutex.Lock();
    VSCPCLIENTLIST::iterator iter;
    for (iter = gpobj->m_clientList.m_clientItemList.begin();
        iter != gpobj->m_clientList.m_clientItemList.end();
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

            mg_send( conn,  strBuf.mb_str(),
                                    strlen( strBuf.mb_str() ) );

    }

    mg_send( conn, MSG_OK, strlen ( MSG_OK ) );

    gpobj->m_wxClientMutex.Unlock();
}

///////////////////////////////////////////////////////////////////////////////
// handleClientInterface_Unique
//

void VSCPClientThread::handleClientInterface_Unique( struct mg_connection *conn,
                                                        CControlObject *pCtrlObject )
{
    unsigned char ifGUID[ 16 ];
    memset( ifGUID, 0, 16 );

    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    // Get GUID
    pClientItem->m_currentCommand.Trim(true);
    pClientItem->m_currentCommand.Trim(false);
    vscp_getGuidFromStringToArray( ifGUID, pClientItem->m_currentCommand );

    // Add the client to the Client List
    // TODO

    mg_send( conn, MSG_INTERFACE_NOT_FOUND, strlen( MSG_INTERFACE_NOT_FOUND ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleClientInterface_Normal
//

void VSCPClientThread::handleClientInterface_Normal( struct mg_connection *conn,
                                                        CControlObject *pCtrlObject )
{
    // TODO
}

///////////////////////////////////////////////////////////////////////////////
// handleClientInterface_Close
//

void VSCPClientThread::handleClientInterface_Close( struct mg_connection *conn,
                                                        CControlObject *pCtrlObject )
{
    // TODO
}





// -----------------------------------------------------------------------------
//                          E N D   I N T E R F A C E
// -----------------------------------------------------------------------------








///////////////////////////////////////////////////////////////////////////////
// handleClientUdp
//

void VSCPClientThread::handleClientUdp( struct mg_connection *conn,
                                            CControlObject *pCtrlObject )
{
    // TODO
}





// -----------------------------------------------------------------------------
//                            E N D   U D P
// -----------------------------------------------------------------------------





// -----------------------------------------------------------------------------
//                               F I L E
// -----------------------------------------------------------------------------






///////////////////////////////////////////////////////////////////////////////
// handleClientFile
//

void VSCPClientThread::handleClientFile( struct mg_connection *conn,
                                            CControlObject *pCtrlObject )
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

void VSCPClientThread::handleClientTable( struct mg_connection *conn,
                                            CControlObject *pCtrlObject )
{
    CClientItem *pClientItem = (CClientItem *)conn->user_data;
    if ( NULL == pClientItem ) return;

    pClientItem->m_currentCommand.Trim(true);
    pClientItem->m_currentCommand.Trim(false);    
    
    // List tables or table definition
    if ( pClientItem->CommandStartsWith(_("list") ) ) {
        handleClientTable_List( conn );
    }
    // Get rawtable content
    else if ( pClientItem->CommandStartsWith(_("getraw") ) ) {
        handleClientTable_GetRaw( conn );
    }
    // Get table content
    else if ( pClientItem->CommandStartsWith(_("get") ) ) {
        handleClientTable_Get( conn );
    }    
    // Delete table data
    else if ( pClientItem->CommandStartsWith(_("clear") ) ) {
        handleClientTable_Clear( conn );
    }
    // New table (create)
    else if ( pClientItem->CommandStartsWith(_("create") ) ) {
        handleClientTable_Create( conn );
    }
    // Delete table
    else if ( pClientItem->CommandStartsWith(_("delete") ) ||
                pClientItem->CommandStartsWith(_("del") ) ) {
        handleClientTable_Delete( conn );
    }
    // Log data use SQL
    else if ( pClientItem->CommandStartsWith(_("logsql") ) ) {
        handleClientTable_LogSQL( conn );
    }
    // Log data
    else if ( pClientItem->CommandStartsWith(_("log") ) ) {
        handleClientTable_Log( conn );
    }    
    // Get number of records
    else if ( pClientItem->CommandStartsWith(_("records") ) ) {
        handleClientTable_NumberOfRecords( conn );
    }
    // Get first date
    else if ( pClientItem->CommandStartsWith(_("firstdate") ) ) {
        handleClientTable_FirstDate( conn );
    }
    // Get last date
    else if ( pClientItem->CommandStartsWith(_("lastdate") ) ) {
        handleClientTable_FirstDate( conn );
    }
    // Get sum
    else if ( pClientItem->CommandStartsWith(_("sum") ) ) {
        handleClientTable_Sum( conn );
    }
    // Get min
    else if ( pClientItem->CommandStartsWith(_("min") ) ) {
        handleClientTable_Min( conn );
    }
    // Get max
    else if ( pClientItem->CommandStartsWith(_("max") ) ) {
        handleClientTable_Min( conn );
    }
    // Get average
    else if ( pClientItem->CommandStartsWith(_("average") ) ) {
        handleClientTable_Average( conn );
    }
    // Get median
    else if ( pClientItem->CommandStartsWith(_("median") ) ) {
        handleClientTable_Median( conn );
    }
    // Get stddev
    else if ( pClientItem->CommandStartsWith(_("stddev") ) ) {
        handleClientTable_StdDev( conn );
    }
    // Get variance
    else if ( pClientItem->CommandStartsWith(_("variance") ) ) {
        handleClientTable_Variance( conn );
    }
    // Get mode
    else if ( pClientItem->CommandStartsWith(_("mode") ) ) {
        handleClientTable_Mode( conn );
    }
    // Get lowerq
    else if ( pClientItem->CommandStartsWith(_("lowerq") ) ) {
        handleClientTable_LowerQ( conn );
    }
    // Get upperq
    else if ( pClientItem->CommandStartsWith(_("upperq") ) ) {
        handleClientTable_UpperQ( conn );
    }
    
    // unrecognised
    else {
        mg_send( conn, MSG_UNKNOWN_COMMAND, strlen( MSG_UNKNOWN_COMMAND ) );
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


void VSCPClientThread::handleClientTable_Create( struct mg_connection *conn )
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
      
    CClientItem *pClientItem = (CClientItem *)conn->user_data;
    if ( NULL == pClientItem ) return;
    
    if ( !gpobj->m_userTableObjects.createTableFromXML( pClientItem->m_currentCommand ) ) {
        mg_send( conn,
                    MSG_FAILED_TO_CREATE_TABLE, 
                    strlen( MSG_FAILED_TO_CREATE_TABLE ) );
        return;
    }
       
    // All went well
    mg_send( conn, MSG_OK,  strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_Delete
//


void VSCPClientThread::handleClientTable_Delete( struct mg_connection *conn )
{
    wxString strTable;
    bool bRemoveFile = false;
    
    CClientItem *pClientItem = (CClientItem *)conn->user_data;
    if ( NULL == pClientItem ) return;    
        
    wxStringTokenizer tkz( pClientItem->m_currentCommand, _(" ") );
    
    // Get table name 
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // Get table name 
    if ( tkz.HasMoreTokens() ) {
        wxString str = tkz.GetNextToken();
        if ( wxNOT_FOUND != str.Upper().Find("TRUE") ) {
            bRemoveFile = true;
        }
    }
    
    // Remove the table from the internal system
    if ( !gpobj->m_userTableObjects.removeTable( strTable ), bRemoveFile ) {
         // Failed
        mg_send( conn, 
                    MSG_FAILED_TO_REMOVE_TABLE, 
                    strlen( MSG_FAILED_TO_REMOVE_TABLE ) );
    }
    
    mg_send( conn, MSG_OK, strlen( MSG_OK ) );
    
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_List
//

void VSCPClientThread::handleClientTable_List( struct mg_connection *conn )
{
    CClientItem *pClientItem = (CClientItem *)conn->user_data;
    if ( NULL == pClientItem ) return;
    
    pClientItem->m_currentCommand.Trim(true);
    pClientItem->m_currentCommand.Trim(false); 
    
    if ( 0 == pClientItem->m_currentCommand.Length() ) {
        
        // list without argument - list all defined tables
        wxArrayString arrayNames;
        gpobj->m_mutexUserTables.Lock();
        if ( gpobj->m_userTableObjects.getTableNames( arrayNames ) ) {
            
            // OK
            
            wxString str = wxString::Format( _("%zu rows \r\n"), 
                                                arrayNames.Count() );
            mg_send( conn, 
                        (const char *)str.mbc_str(), 
                        strlen( (const char *)str.mbc_str() ) );
            
            for ( int i=0; i<arrayNames.Count(); i++ ) {
                
                CVSCPTable *pTable = 
                    gpobj->m_userTableObjects.getTable( arrayNames[i] );
                
                str = arrayNames[i];
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
                mg_send( conn, 
                            (const char *)str.mbc_str(), 
                            strlen( (const char *)str.mbc_str() ) );
            }
            
            mg_send( conn,
                        MSG_OK, 
                        strlen( MSG_OK ) );
            
        }
        else {
            
            // Failed
            mg_send( conn, 
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
        wxStringTokenizer tkz( pClientItem->m_currentCommand, _(" ") );
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
            mg_send( conn, 
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
            mg_send( conn,
                        (const char *)str.mbc_str(), 
                        strlen( (const char *)str.mbc_str() ) );
            
            // Tell user we are content with things.
            mg_send( conn,
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

void VSCPClientThread::handleClientTable_Get( struct mg_connection *conn )
{
    wxString strTable;
    wxDateTime wxStart;   
    wxDateTime wxEnd;
    bool bAll = false;
    
    // Initialize date range to 'all'
    wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
    wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
    
    CClientItem *pClientItem = (CClientItem *)conn->user_data;
    if ( NULL == pClientItem ) return;    
    
    pClientItem->m_currentCommand.Trim(true);
    pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( pClientItem->m_currentCommand, _(" ") );
    
    // Get table name 
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // The start and end parameters must be present
    
    // start
    if ( tkz.HasMoreTokens() ) {
        
        wxStart.ParseISOCombined( tkz.GetNextToken() );
        
        if ( !wxStart.IsValid() ) {
            mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
    }
    else {
        // Problems: A start date must be given
        mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // end
    if ( tkz.HasMoreTokens() ) {
        
        wxEnd.ParseISOCombined( tkz.GetNextToken() );
        
        if ( !wxEnd.IsValid() ) {
            mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
    }
    else {
        // Problems: An end date must be given
        mg_send( conn,
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
        mg_send( conn, 
                    MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    }   

    sqlite3_stmt *ppStmt;
    if ( !pTable->prepareRangeOfData( wxStart, wxEnd, &ppStmt, bAll ) ) {
        // Failed
        mg_send( conn, 
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
    mg_send( conn, 
                (const char *)str.mbc_str(), 
                strlen( (const char *)str.mbc_str() ) );
    
    if ( strArray.Count() ) {
        for ( int i=0; i<strArray.Count(); i++ ) {
            str = strArray[i];
            str += _("\r\n");
            mg_send( conn, 
                    (const char *)str.mbc_str(), 
                    strlen( (const char *)str.mbc_str() ) );
        }
    }    
    
    if ( !pTable->finalizeRangeOfData( ppStmt )  ) {
        // Failed
        mg_send( conn, 
                    MSG_FAILED_TO_FINALIZE_TABLE, 
                    strlen( MSG_FAILED_TO_FINALIZE_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    gpobj->m_mutexUserTables.Unlock();
        
    // Everything is OK
    mg_send( conn, MSG_OK, strlen( MSG_OK ) );
}


///////////////////////////////////////////////////////////////////////////////
// handleClientTable_GetRaw
//
// get 'table-name' start end ["full"]
//

void VSCPClientThread::handleClientTable_GetRaw( struct mg_connection *conn )
{
    wxString strTable;
    wxDateTime wxStart;   
    wxDateTime wxEnd;
    
    // Initialize date range to 'all'
    wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
    wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
    
    CClientItem *pClientItem = (CClientItem *)conn->user_data;
    if ( NULL == pClientItem ) return;    
    
    pClientItem->m_currentCommand.Trim(true);
    pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( pClientItem->m_currentCommand, _(" ") );
    
    // Get table name 
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // The start and end parameters must be present
    
    // start
    if ( tkz.HasMoreTokens() ) {
        
        wxStart.ParseISOCombined( tkz.GetNextToken() );
        
        if ( !wxStart.IsValid() ) {
            mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
    }
    else {
        // Problems: A start date must be given
        mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // end
    if ( tkz.HasMoreTokens() ) {
        
        wxEnd.ParseISOCombined( tkz.GetNextToken() );
        
        if ( !wxEnd.IsValid() ) {
            mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
    }
    else {
        // Problems: An end date must be given
        mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
        
    gpobj->m_mutexUserTables.Lock();
    
    CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    if ( NULL == pTable ) {
        // Failed
        mg_send( conn, 
                    MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    }   

    sqlite3_stmt *ppStmt;
    if ( !pTable->prepareRangeOfData( wxStart, wxEnd, &ppStmt, true ) ) {
        // Failed
        mg_send( conn, 
                    MSG_FAILED_TO_PREPARE_TABLE, 
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
    mg_send( conn, 
                (const char *)str.mbc_str(), 
                strlen( (const char *)str.mbc_str() ) );
    
    if ( strArray.Count() ) {
        for ( int i=0; i<strArray.Count(); i++ ) {
            str = strArray[i];
            str += _("\r\n");
            mg_send( conn, 
                    (const char *)str.mbc_str(), 
                    strlen( (const char *)str.mbc_str() ) );
        }
    }    
    
    if ( !pTable->finalizeRangeOfData( ppStmt )  ) {
        // Failed
        mg_send( conn, 
                    MSG_FAILED_TO_FINALIZE_TABLE, 
                    strlen( MSG_FAILED_TO_FINALIZE_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    gpobj->m_mutexUserTables.Unlock();
        
    // Everything is OK
    mg_send( conn, MSG_OK, strlen( MSG_OK ) );
}


///////////////////////////////////////////////////////////////////////////////
// handleClientTable_Clear
//
// clear 'table-name' [to,from]
//

void VSCPClientThread::handleClientTable_Clear( struct mg_connection *conn )
{    
    wxString strTable;
    wxDateTime wxStart;   
    wxDateTime wxEnd;
    bool bClearAll = false;
    
    // Initialize date range to 'all'
    wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
    wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
    
    CClientItem *pClientItem = (CClientItem *)conn->user_data;
    if ( NULL == pClientItem ) return;
    
    pClientItem->m_currentCommand.Trim(true);
    pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( pClientItem->m_currentCommand, _(" ") );
    
    // Get table name
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // get range if given
    if ( tkz.CountTokens() >= 2 ) {
        
        if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
            mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
        if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
            mg_send( conn,
                    MSG_PARAMETER_ERROR, 
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
        mg_send( conn, 
                    MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    } 
    
    if ( bClearAll ) {
        if ( !pTable->clearTable() ) {
            mg_send( conn, 
                    MSG_FAILED_TO_CLEAR_TABLE, 
                    strlen( MSG_FAILED_TO_CLEAR_TABLE ) );
            gpobj->m_mutexUserTables.Unlock();
            return;
        }
    }
    else {
        if ( !pTable->clearTableRange( wxStart, wxEnd ) ) {
            mg_send( conn, 
                    MSG_FAILED_TO_CLEAR_TABLE, 
                    strlen( MSG_FAILED_TO_CLEAR_TABLE ) );
            gpobj->m_mutexUserTables.Unlock();
            return;
        }
    }
            
    gpobj->m_mutexUserTables.Unlock();
    
    mg_send( conn,
                MSG_OK,  
                strlen( MSG_OK ) );
    
    return;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientTable_NumberOfRecords
//
// records 'table-name' [to,from]
//

void VSCPClientThread::handleClientTable_NumberOfRecords( struct mg_connection *conn )
{    
    wxString strTable;
    wxDateTime wxStart;   
    wxDateTime wxEnd;
    
    // Initialize date range to 'all'
    wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
    wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
    
    CClientItem *pClientItem = (CClientItem *)conn->user_data;
    if ( NULL == pClientItem ) return;
    
    pClientItem->m_currentCommand.Trim(true);
    pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( pClientItem->m_currentCommand, _(" ") );
    
    // Get table name
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // get range if given
    if ( tkz.CountTokens() >= 2 ) {
        
        if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
            mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
        if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
            mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
    }
    
    gpobj->m_mutexUserTables.Lock();
    
    CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
    if ( NULL == pTable ) {
        // Failed
        mg_send( conn, 
                    MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    } 
    
    double count;
    if ( !pTable->getNumberOfRecordsForRange( wxStart, wxEnd, &count ) ) {
        mg_send( conn, 
                    MSG_FAILED_TO_CLEAR_TABLE, 
                    strlen( MSG_FAILED_TO_CLEAR_TABLE ) );
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    gpobj->m_mutexUserTables.Unlock();
    
    wxString strReply = wxString::Format(_("%f\r\n"), count );
    mg_send( conn,
                (const char *)strReply.mbc_str(),  
                strlen( (const char *)strReply.mbc_str() ) );
    
    mg_send( conn,
                MSG_OK,  
                strlen( MSG_OK ) );
    
    return;
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_FirstDate
//
// firstdate 'table-name' [to,from]
//

void VSCPClientThread::handleClientTable_FirstDate( struct mg_connection *conn )
{    
    wxString strTable;
    
    CClientItem *pClientItem = (CClientItem *)conn->user_data;
    if ( NULL == pClientItem ) return;
    
    pClientItem->m_currentCommand.Trim(true);
    pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( pClientItem->m_currentCommand, _(" ") );
    
    // Get table name
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    
    gpobj->m_mutexUserTables.Lock();
    
    CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
    if ( NULL == pTable ) {
        // Failed
        mg_send( conn, 
                    MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    } 
    
    wxDateTime first;
    if ( !pTable->getFirstDate( first ) ) {
        mg_send( conn, 
                    MSG_FAILED_TO_CLEAR_TABLE, 
                    strlen( MSG_FAILED_TO_CLEAR_TABLE ) );
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    gpobj->m_mutexUserTables.Unlock();
    
    wxString strReply = wxString::Format(_("%s\r\n"), (const char *)first.FormatISOCombined().mbc_str() );
    mg_send( conn,
                (const char *)strReply.mbc_str(),  
                strlen( (const char *)strReply.mbc_str() ) );
    
    mg_send( conn,
                MSG_OK,  
                strlen( MSG_OK ) );
    
    return;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientTable_LastDate
//
// lastdate 'table-name' [to,from]
//

void VSCPClientThread::handleClientTable_LastDate( struct mg_connection *conn )
{    
    wxString strTable;
    
    CClientItem *pClientItem = (CClientItem *)conn->user_data;
    if ( NULL == pClientItem ) return;
    
    pClientItem->m_currentCommand.Trim(true);
    pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( pClientItem->m_currentCommand, _(" ") );
    
    // Get table name
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    gpobj->m_mutexUserTables.Lock();
    
    CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
    if ( NULL == pTable ) {
        // Failed
        mg_send( conn, 
                    MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    } 
    
    wxDateTime last;
    if ( !pTable->getLastDate( last) ) {
        mg_send( conn, 
                    MSG_FAILED_TO_CLEAR_TABLE, 
                    strlen( MSG_FAILED_TO_CLEAR_TABLE ) );
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    gpobj->m_mutexUserTables.Unlock();
    
    wxString strReply = wxString::Format(_("%s\r\n"), (const char *)last.FormatISOCombined().mbc_str() );
    mg_send( conn,
                (const char *)strReply.mbc_str(),  
                strlen( (const char *)strReply.mbc_str() ) );
    
    mg_send( conn,
                MSG_OK,  
                strlen( MSG_OK ) );
    
    return;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientTable_Sum
//
// sum 'table-name' [to,from]
//

void VSCPClientThread::handleClientTable_Sum( struct mg_connection *conn )
{    
    wxString strTable;
    wxDateTime wxStart;   
    wxDateTime wxEnd;
    
    // Initialize date range to 'all'
    wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
    wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
    
    CClientItem *pClientItem = (CClientItem *)conn->user_data;
    if ( NULL == pClientItem ) return;
    
    pClientItem->m_currentCommand.Trim(true);
    pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( pClientItem->m_currentCommand, _(" ") );
    
    // Get table name
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // get range if given
    if ( tkz.CountTokens() >= 2 ) {
        
        if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
            mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
        if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
            mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
    }
    
    gpobj->m_mutexUserTables.Lock();
    
    CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
    if ( NULL == pTable ) {
        // Failed
        mg_send( conn, 
                    MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    } 
    
    double sum;
    if ( !pTable->getSumValue( wxStart, wxEnd, &sum ) ) {
        mg_send( conn, 
                    MSG_FAILED_TO_CLEAR_TABLE, 
                    strlen( MSG_FAILED_TO_CLEAR_TABLE ) );
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    gpobj->m_mutexUserTables.Unlock();
    
    wxString strReply = wxString::Format(_("%f\r\n"), sum );
    mg_send( conn,
                (const char *)strReply.mbc_str(),  
                strlen( (const char *)strReply.mbc_str() ) );
    
    mg_send( conn,
                MSG_OK,  
                strlen( MSG_OK ) );
    
    return;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientTable_Min
//
// min 'table-name' [to,from]
//

void VSCPClientThread::handleClientTable_Min( struct mg_connection *conn )
{    
    wxString strTable;
    wxDateTime wxStart;   
    wxDateTime wxEnd;
    
    // Initialize date range to 'all'
    wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
    wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
    
    CClientItem *pClientItem = (CClientItem *)conn->user_data;
    if ( NULL == pClientItem ) return;
    
    pClientItem->m_currentCommand.Trim(true);
    pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( pClientItem->m_currentCommand, _(" ") );
    
    // Get table name
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // get range if given
    if ( tkz.CountTokens() >= 2 ) {
        
        if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
            mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
        if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
            mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
    }
    
    gpobj->m_mutexUserTables.Lock();
    
    CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
    if ( NULL == pTable ) {
        // Failed
        mg_send( conn, 
                    MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    } 
    
    double min;
    if ( !pTable->getMinValue( wxStart, wxEnd, &min ) ) {
        mg_send( conn, 
                    MSG_FAILED_TO_CLEAR_TABLE, 
                    strlen( MSG_FAILED_TO_CLEAR_TABLE ) );
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    gpobj->m_mutexUserTables.Unlock();
    
    wxString strReply = wxString::Format(_("%f\r\n"), min );
    mg_send( conn,
                (const char *)strReply.mbc_str(),  
                strlen( (const char *)strReply.mbc_str() ) );
    
    mg_send( conn,
                MSG_OK,  
                strlen( MSG_OK ) );
    
    return;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientTable_Max
//
// max 'table-name' [to,from]
//

void VSCPClientThread::handleClientTable_Max( struct mg_connection *conn )
{    
    wxString strTable;
    wxDateTime wxStart;   
    wxDateTime wxEnd;
    
    // Initialize date range to 'all'
    wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
    wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
    
    CClientItem *pClientItem = (CClientItem *)conn->user_data;
    if ( NULL == pClientItem ) return;
    
    pClientItem->m_currentCommand.Trim(true);
    pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( pClientItem->m_currentCommand, _(" ") );
    
    // Get table name
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // get range if given
    if ( tkz.CountTokens() >= 2 ) {
        
        if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
            mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
        if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
            mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
    }
    
    gpobj->m_mutexUserTables.Lock();
    
    CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
    if ( NULL == pTable ) {
        // Failed
        mg_send( conn, 
                    MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    } 
    
    double max;
    if ( !pTable->getMaxValue( wxStart, wxEnd, &max ) ) {
        mg_send( conn, 
                    MSG_FAILED_TO_CLEAR_TABLE, 
                    strlen( MSG_FAILED_TO_CLEAR_TABLE ) );
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    gpobj->m_mutexUserTables.Unlock();
    
    wxString strReply = wxString::Format(_("%f\r\n"), max );
    mg_send( conn,
                (const char *)strReply.mbc_str(),  
                strlen( (const char *)strReply.mbc_str() ) );
    
    mg_send( conn,
                MSG_OK,  
                strlen( MSG_OK ) );
    
    return;
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_Average
//
// average 'table-name' [to,from]
//

void VSCPClientThread::handleClientTable_Average( struct mg_connection *conn )
{    
    wxString strTable;
    wxDateTime wxStart;   
    wxDateTime wxEnd;
    
    // Initialize date range to 'all'
    wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
    wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
    
    CClientItem *pClientItem = (CClientItem *)conn->user_data;
    if ( NULL == pClientItem ) return;
    
    pClientItem->m_currentCommand.Trim(true);
    pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( pClientItem->m_currentCommand, _(" ") );
    
    // Get table name
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // get range if given
    if ( tkz.CountTokens() >= 2 ) {
        
        if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
            mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
        if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
            mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
    }
    
    gpobj->m_mutexUserTables.Lock();
    
    CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
    if ( NULL == pTable ) {
        // Failed
        mg_send( conn, 
                    MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    } 
    
    double average;
    if ( !pTable->getAverageValue( wxStart, wxEnd, &average ) ) {
        mg_send( conn, 
                    MSG_FAILED_TO_CLEAR_TABLE, 
                    strlen( MSG_FAILED_TO_CLEAR_TABLE ) );
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    gpobj->m_mutexUserTables.Unlock();
    
    wxString strReply = wxString::Format(_("%f\r\n"), average );
    mg_send( conn,
                (const char *)strReply.mbc_str(),  
                strlen( (const char *)strReply.mbc_str() ) );
    
    mg_send( conn,
                MSG_OK,  
                strlen( MSG_OK ) );
    
    return;
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_Median
//
// median 'table-name' [to,from]
//

void VSCPClientThread::handleClientTable_Median( struct mg_connection *conn )
{    
    wxString strTable;
    wxDateTime wxStart;   
    wxDateTime wxEnd;
    
    // Initialize date range to 'all'
    wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
    wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
    
    CClientItem *pClientItem = (CClientItem *)conn->user_data;
    if ( NULL == pClientItem ) return;
    
    pClientItem->m_currentCommand.Trim(true);
    pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( pClientItem->m_currentCommand, _(" ") );
    
    // Get table name
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // get range if given
    if ( tkz.CountTokens() >= 2 ) {
        
        if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
            mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
        if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
            mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
    }
    
    gpobj->m_mutexUserTables.Lock();
    
    CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
    if ( NULL == pTable ) {
        // Failed
        mg_send( conn, 
                    MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    } 
    
    double median;
    if ( !pTable->getMedianValue( wxStart, wxEnd, &median ) ) {
        mg_send( conn, 
                    MSG_FAILED_TO_CLEAR_TABLE, 
                    strlen( MSG_FAILED_TO_CLEAR_TABLE ) );
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    gpobj->m_mutexUserTables.Unlock();
    
    wxString strReply = wxString::Format(_("%f\r\n"), median );
    mg_send( conn,
                (const char *)strReply.mbc_str(),  
                strlen( (const char *)strReply.mbc_str() ) );
    
    mg_send( conn,
                MSG_OK,  
                strlen( MSG_OK ) );
    
    return;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientTable_StdDev
//
// stddev 'table-name' [to,from]
//

void VSCPClientThread::handleClientTable_StdDev( struct mg_connection *conn )
{    
    wxString strTable;
    wxDateTime wxStart;   
    wxDateTime wxEnd;
    
    // Initialize date range to 'all'
    wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
    wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
    
    CClientItem *pClientItem = (CClientItem *)conn->user_data;
    if ( NULL == pClientItem ) return;
    
    pClientItem->m_currentCommand.Trim(true);
    pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( pClientItem->m_currentCommand, _(" ") );
    
    // Get table name
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // get range if given
    if ( tkz.CountTokens() >= 2 ) {
        
        if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
            mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
        if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
            mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
    }
    
    gpobj->m_mutexUserTables.Lock();
    
    CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
    if ( NULL == pTable ) {
        // Failed
        mg_send( conn, 
                    MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    } 
    
    double stdev;
    if ( !pTable->getStdevValue( wxStart, wxEnd, &stdev ) ) {
        mg_send( conn, 
                    MSG_FAILED_TO_CLEAR_TABLE, 
                    strlen( MSG_FAILED_TO_CLEAR_TABLE ) );
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    gpobj->m_mutexUserTables.Unlock();
    
    wxString strReply = wxString::Format(_("%f\r\n"), stdev );
    mg_send( conn,
                (const char *)strReply.mbc_str(),  
                strlen( (const char *)strReply.mbc_str() ) );
    
    mg_send( conn,
                MSG_OK,  
                strlen( MSG_OK ) );
    
    return;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientTable_Variance
//
// variance 'table-name' [to,from]
//

void VSCPClientThread::handleClientTable_Variance( struct mg_connection *conn )
{    
    wxString strTable;
    wxDateTime wxStart;   
    wxDateTime wxEnd;
    
    // Initialize date range to 'all'
    wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
    wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
    
    CClientItem *pClientItem = (CClientItem *)conn->user_data;
    if ( NULL == pClientItem ) return;
    
    pClientItem->m_currentCommand.Trim(true);
    pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( pClientItem->m_currentCommand, _(" ") );
    
    // Get table name
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // get range if given
    if ( tkz.CountTokens() >= 2 ) {
        
        if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
            mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
        if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
            mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
    }
    
    gpobj->m_mutexUserTables.Lock();
    
    CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
    if ( NULL == pTable ) {
        // Failed
        mg_send( conn, 
                    MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    } 
    
    double variance;
    if ( !pTable->getVarianceValue( wxStart, wxEnd, &variance ) ) {
        mg_send( conn, 
                    MSG_FAILED_TO_CLEAR_TABLE, 
                    strlen( MSG_FAILED_TO_CLEAR_TABLE ) );
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    gpobj->m_mutexUserTables.Unlock();
    
    wxString strReply = wxString::Format(_("%f\r\n"), variance );
    mg_send( conn,
                (const char *)strReply.mbc_str(),  
                strlen( (const char *)strReply.mbc_str() ) );
    
    mg_send( conn,
                MSG_OK,  
                strlen( MSG_OK ) );
    
    return;
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_Mode
//
// mode 'table-name' [to,from]
//

void VSCPClientThread::handleClientTable_Mode( struct mg_connection *conn )
{    
    wxString strTable;
    wxDateTime wxStart;   
    wxDateTime wxEnd;
    
    // Initialize date range to 'all'
    wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
    wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
    
    CClientItem *pClientItem = (CClientItem *)conn->user_data;
    if ( NULL == pClientItem ) return;
    
    pClientItem->m_currentCommand.Trim(true);
    pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( pClientItem->m_currentCommand, _(" ") );
    
    // Get table name
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // get range if given
    if ( tkz.CountTokens() >= 2 ) {
        
        if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
            mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
        if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
            mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
    }
    
    gpobj->m_mutexUserTables.Lock();
    
    CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
    if ( NULL == pTable ) {
        // Failed
        mg_send( conn, 
                    MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    } 
    
    double mode;
    if ( !pTable->getModeValue( wxStart, wxEnd, &mode ) ) {
        mg_send( conn, 
                    MSG_FAILED_TO_CLEAR_TABLE, 
                    strlen( MSG_FAILED_TO_CLEAR_TABLE ) );
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    gpobj->m_mutexUserTables.Unlock();
    
    wxString strReply = wxString::Format(_("%f\r\n"), mode );
    mg_send( conn,
                (const char *)strReply.mbc_str(),  
                strlen( (const char *)strReply.mbc_str() ) );
    
    mg_send( conn,
                MSG_OK,  
                strlen( MSG_OK ) );
    
    return;
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_LowerQ
//
// lowerq 'table-name' [to,from]
//

void VSCPClientThread::handleClientTable_LowerQ( struct mg_connection *conn )
{    
    wxString strTable;
    wxDateTime wxStart;   
    wxDateTime wxEnd;
    
    // Initialize date range to 'all'
    wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
    wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
    
    CClientItem *pClientItem = (CClientItem *)conn->user_data;
    if ( NULL == pClientItem ) return;
    
    pClientItem->m_currentCommand.Trim(true);
    pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( pClientItem->m_currentCommand, _(" ") );
    
    // Get table name
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // get range if given
    if ( tkz.CountTokens() >= 2 ) {
        
        if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
            mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
        if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
            mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
    }
    
    gpobj->m_mutexUserTables.Lock();
    
    CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
    if ( NULL == pTable ) {
        // Failed
        mg_send( conn, 
                    MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    } 
    
    double lq;
    if ( !pTable->getLowerQuartileValue( wxStart, wxEnd, &lq ) ) {
        mg_send( conn, 
                    MSG_FAILED_TO_CLEAR_TABLE, 
                    strlen( MSG_FAILED_TO_CLEAR_TABLE ) );
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    gpobj->m_mutexUserTables.Unlock();
    
    wxString strReply = wxString::Format(_("%f\r\n"), lq );
    mg_send( conn,
                (const char *)strReply.mbc_str(),  
                strlen( (const char *)strReply.mbc_str() ) );
    
    mg_send( conn,
                MSG_OK,  
                strlen( MSG_OK ) );
    
    return;
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_upperq
//
// upperq 'table-name' [to,from]
//

void VSCPClientThread::handleClientTable_UpperQ( struct mg_connection *conn )
{    
    wxString strTable;
    wxDateTime wxStart;   
    wxDateTime wxEnd;
    
    // Initialize date range to 'all'
    wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
    wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
    
    CClientItem *pClientItem = (CClientItem *)conn->user_data;
    if ( NULL == pClientItem ) return;
    
    pClientItem->m_currentCommand.Trim(true);
    pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( pClientItem->m_currentCommand, _(" ") );
    
    // Get table name
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // get range if given
    if ( tkz.CountTokens() >= 2 ) {
        
        if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
            mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
        if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
            mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
        
    }
    
    gpobj->m_mutexUserTables.Lock();
    
    CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
    if ( NULL == pTable ) {
        // Failed
        mg_send( conn, 
                    MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    } 
    
    double uq;
    if ( !pTable->getUppeQuartileValue( wxStart, wxEnd, &uq ) ) {
        mg_send( conn, 
                    MSG_FAILED_TO_CLEAR_TABLE, 
                    strlen( MSG_FAILED_TO_CLEAR_TABLE ) );
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    gpobj->m_mutexUserTables.Unlock();
    
    wxString strReply = wxString::Format(_("%f\r\n"), uq );
    mg_send( conn,
                (const char *)strReply.mbc_str(),  
                strlen( (const char *)strReply.mbc_str() ) );
    
    mg_send( conn,
                MSG_OK,  
                strlen( MSG_OK ) );
    
    return;
}

///////////////////////////////////////////////////////////////////////////////
// handleClientTable_Log
//
// log table-name value [datetime]
//

void VSCPClientThread::handleClientTable_Log( struct mg_connection *conn )
{
    wxString strTable;
    double value;
    wxDateTime dt;
    
    CClientItem *pClientItem = (CClientItem *)conn->user_data;
    if ( NULL == pClientItem ) return;    
    
    pClientItem->m_currentCommand.Trim(true);
    pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( pClientItem->m_currentCommand, _(" ") );
    
    // Get table name 
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // Get the value
    if ( tkz.HasMoreTokens() ) {
        wxString str = tkz.GetNextToken();
        if ( !str.ToDouble( &value ) ) {
            // Problems: The value is not in a valid format
            mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
            return;            
        }
    }
    else {
        // Problems: A value must be given
        mg_send( conn,
                    MSG_PARAMETER_ERROR, 
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
            mg_send( conn,
                    MSG_PARAMETER_ERROR, 
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
        mg_send( conn, 
                    MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    // Log data
    if ( !pTable->logData( dt, value ) ) {
        gpobj->m_mutexUserTables.Unlock();
        mg_send( conn,
                    MSG_FAILED_TO_WRITE_TABLE,  
                    strlen( MSG_FAILED_TO_WRITE_TABLE ) );
        return;
    }
    
    gpobj->m_mutexUserTables.Unlock();
    
    mg_send( conn,
                MSG_OK,  
                strlen( MSG_OK ) );
}


///////////////////////////////////////////////////////////////////////////////
// handleClientTable_LogSQL
//

void VSCPClientThread::handleClientTable_LogSQL( struct mg_connection *conn )
{
    wxString strTable, strSQL;
    
    CClientItem *pClientItem = (CClientItem *)conn->user_data;
    if ( NULL == pClientItem ) return;    
    
    pClientItem->m_currentCommand.Trim(true);
    pClientItem->m_currentCommand.Trim(false);
    
    wxStringTokenizer tkz( pClientItem->m_currentCommand, _(" ") );
    
    // Get table name 
    if ( tkz.HasMoreTokens() ) {
        strTable = tkz.GetNextToken();
        strTable.Trim(true);
        strTable.Trim(false);
    }
    else {
        // Problems: A table name must be given
        mg_send( conn,
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
        mg_send( conn,
                    MSG_PARAMETER_ERROR, 
                    strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    
    gpobj->m_mutexUserTables.Lock();
    
    CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
    if ( NULL == pTable ) {
        // Failed
        mg_send( conn, 
                    MSG_FAILED_UNKNOWN_TABLE, 
                    strlen( MSG_FAILED_UNKNOWN_TABLE ) );
        
        gpobj->m_mutexUserTables.Unlock();
        return;
    }
    
    // Log data
    if ( !pTable->logData( strSQL ) ) {
        gpobj->m_mutexUserTables.Unlock();
        mg_send( conn,
                    MSG_FAILED_TO_WRITE_TABLE,  
                    strlen( MSG_FAILED_TO_WRITE_TABLE ) );
        return;
    }
    
    gpobj->m_mutexUserTables.Unlock();
    
}









// -----------------------------------------------------------------------------
//                            E N D   T A B L E
// -----------------------------------------------------------------------------







///////////////////////////////////////////////////////////////////////////////
// handleClientVariable
//

void VSCPClientThread::handleClientVariable( struct mg_connection *conn,
                                                CControlObject *pCtrlObject )
{
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    pClientItem->m_currentCommand.Trim(false);
    pClientItem->m_currentCommand.Trim(true);

    if ( pClientItem->CommandStartsWith(_("list") ) ) {
        handleVariable_List( conn, pCtrlObject );
    }
    else if ( pClientItem->CommandStartsWith(_("writevalue") ) ) {
        handleVariable_WriteValue( conn, pCtrlObject );
    }
    else if ( pClientItem->CommandStartsWith(_("writenote") ) ) {
        handleVariable_WriteNote( conn, pCtrlObject );
    }
    else if ( pClientItem->CommandStartsWith(_("write") ) ) {
        handleVariable_Write( conn, pCtrlObject );
    }
    else if ( pClientItem->CommandStartsWith(_("readvalue") ) ) {
        handleVariable_ReadValue( conn, pCtrlObject );
    }
    else if ( pClientItem->CommandStartsWith(_("readnote") ) ) {
        handleVariable_ReadNote( conn, pCtrlObject );
    }
    else if ( pClientItem->CommandStartsWith(_("readreset") ) ) {
        handleVariable_ReadReset( conn, pCtrlObject );
    }  
    else if ( pClientItem->CommandStartsWith(_("readremove") ) ) {
        handleVariable_ReadRemove( conn, pCtrlObject );
    }
    else if ( pClientItem->CommandStartsWith(_("read") ) ) {
        handleVariable_Read( conn, pCtrlObject );
    }  
    else if ( pClientItem->CommandStartsWith(_("reset") ) ) {
        handleVariable_Reset( conn, pCtrlObject );
    }
    else if ( pClientItem->CommandStartsWith(_("remove") ) ) {
        handleVariable_Remove( conn, pCtrlObject );
    }    
    else if ( pClientItem->CommandStartsWith(_("length") ) ) {
        handleVariable_Length( conn, pCtrlObject );
    }
    else if ( pClientItem->CommandStartsWith(_("load") ) ) {
        handleVariable_Load( conn, pCtrlObject );
    }
    else if ( pClientItem->CommandStartsWith(_("save") ) ) {
        handleVariable_Save( conn, pCtrlObject );
    }
    else {
        mg_send( conn, MSG_UNKNOWN_COMMAND, strlen( MSG_UNKNOWN_COMMAND ) );
    }
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_List
//
// variable list - List all variables.
// variable list test - List all variables with "test" in there name
//

void VSCPClientThread::handleVariable_List( struct mg_connection *conn,
                                                CControlObject *pCtrlObject )
{
    CVSCPVariable variable;
    wxString wxstr;
    wxString strWork;
    wxString strSearch;
    int type = 0;   // All variables
    
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    pClientItem->m_currentCommand.Trim(false);
    pClientItem->m_currentCommand.Trim(true);

    wxStringTokenizer tkz( pClientItem->m_currentCommand, _(" ") );
                                       
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
        mg_send( conn,  wxstr.mb_str(), wxstr.Length() );
    
        int cnt = 0;
        for ( int i=0; i<arrayVars.Count(); i++ ) {
            if ( 0 != gpobj->m_variables.find( arrayVars[ i ], variable ) ) {
                if ( ( 0 == type ) || ( variable.getType() == type ) ) {
                    wxstr = wxString::Format( _("%d;"), cnt );
                    wxstr += variable.getAsString();
                    wxstr += _("\r\n");
                    mg_send( conn,  wxstr.mb_str(), wxstr.Length() );
                    cnt++;
                }
            }
        }
    
    }
    else {
        wxstr = _("0 rows.\r\n");
        mg_send( conn,  wxstr.mb_str(), wxstr.Length() );
    } 

    mg_send( conn,  MSG_OK, strlen ( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_Write
//
// Format is: "variable name";"type";"persistence";"owner";"rights";"value";"note"
//
// test31;string;true;0;0x777;dGhpcyBpcyBhIHRlc3Q=;VGhpcyBpcyBhIG5vdGUgZm9yIGEgdGVzdCB2YXJpYWJsZQ==
//

void VSCPClientThread::handleVariable_Write( struct mg_connection *conn,
                                                CControlObject *pCtrlObject )
{
    CVSCPVariable variable;
    wxString wxstr;
    
    bool bPersistence = false;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    pClientItem->m_currentCommand.Trim(false);
    pClientItem->m_currentCommand.Trim(true);
    
    if ( !variable.getVariableFromString( pClientItem->m_currentCommand, 
                                                false, 
                                                pClientItem->m_UserName ) ) {
        // Failed to parse
        mg_send( conn, MSG_PARAMETER_ERROR, strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    if ( gpobj->m_variables.exist( variable.getName() ) ) {
        
        // Update in database
        if ( !gpobj->m_variables.update( variable ) ) {
            mg_send( conn, MSG_VARIABLE_NO_SAVE, strlen ( MSG_VARIABLE_NO_SAVE ) );
            return;
        }
        
    }
    else {
       
        // If the variable exist change value 
        // if not - add it.
        if ( !gpobj->m_variables.add( variable ) ) {
            mg_send( conn, MSG_VARIABLE_UNABLE_ADD, strlen ( MSG_VARIABLE_UNABLE_ADD ) );
            return;
        }
        
    }
    
    mg_send( conn, MSG_OK, strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_WriteValue
//

void VSCPClientThread::handleVariable_WriteValue( struct mg_connection *conn,
                                                    CControlObject *pCtrlObject )
{
    wxString name;
    wxString value;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;
    
    pClientItem->m_currentCommand.Trim(false);
    pClientItem->m_currentCommand.Trim(true);
    
    wxStringTokenizer tkz( pClientItem->m_currentCommand, _(" \r\n") );
    
    // Variable name
    if ( tkz.HasMoreTokens() ) {
        name = tkz.GetNextToken();
    }
    else {
        mg_send( conn, MSG_PARAMETER_ERROR, strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // Variable value
    if ( tkz.HasMoreTokens() ) {
        
        value = tkz.GetNextToken();
        
    }
    else {
        mg_send( conn, MSG_PARAMETER_ERROR, strlen( MSG_PARAMETER_ERROR ) );
        return;
    }

    CVSCPVariable variable;
    if ( 0 != gpobj->m_variables.find( name, variable ) ) {
        
        // Set value and encode as BASE64 when expected
        variable.setValueFromString( variable.getType(), value, false );
        
        // Update in database
        if ( !gpobj->m_variables.update( variable ) ) {
            mg_send( conn, MSG_VARIABLE_NO_SAVE, strlen ( MSG_VARIABLE_NO_SAVE ) );
            return;
        }
        
    }
    else {
        mg_send( conn, MSG_VARIABLE_NOT_DEFINED, strlen ( MSG_VARIABLE_NOT_DEFINED ) );
    }
    
    mg_send( conn, MSG_OK, strlen ( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_WriteNote
//

void VSCPClientThread::handleVariable_WriteNote( struct mg_connection *conn,
                                                    CControlObject *pCtrlObject )
{
    wxString str;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;
    
    pClientItem->m_currentCommand.Trim(false);
    pClientItem->m_currentCommand.Trim(true);
    
    wxString name;
    wxString note;
    
    wxStringTokenizer tkz( pClientItem->m_currentCommand, _(" ") );
    
    // Variable name
    if ( tkz.HasMoreTokens() ) {
        name = tkz.GetNextToken();
    }
    else {
        mg_send( conn, MSG_PARAMETER_ERROR, strlen( MSG_PARAMETER_ERROR ) );
        return;
    }
    
    // Variable value
    if ( tkz.HasMoreTokens() ) {
        
        note = tkz.GetNextToken();
        
    }
    else {
        mg_send( conn, MSG_PARAMETER_ERROR, strlen( MSG_PARAMETER_ERROR ) );
        return;
    }

    CVSCPVariable variable;
    if ( 0 != gpobj->m_variables.find( name, variable ) ) {
        
        // Set value and encode as BASE64 when expected
        variable.setNote( note, true );
        
        // Update in database
        if ( !gpobj->m_variables.update( variable ) ) {
            mg_send( conn, MSG_VARIABLE_NO_SAVE, strlen ( MSG_VARIABLE_NO_SAVE ) );
            return;
        }
        
    }
    else {
        mg_send( conn, MSG_VARIABLE_NOT_DEFINED, strlen ( MSG_VARIABLE_NOT_DEFINED ) );
    }
    
    mg_send( conn, MSG_OK, strlen ( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_Read
//

void VSCPClientThread::handleVariable_Read( struct mg_connection *conn,
                                                CControlObject *pCtrlObject, 
                                                bool bOKResponse )
{
    wxString str;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    pClientItem->m_currentCommand.Trim(false);
    pClientItem->m_currentCommand.Trim(true);

    CVSCPVariable variable;
    if ( 0 != gpobj->m_variables.find( pClientItem->m_currentCommand,variable ) ) {
        
        str = variable.getAsString( false );
        str = _("+OK - ") + str + _("\r\n");
        mg_send( conn,  str.mbc_str(), strlen( str.mbc_str() ) );

        mg_send( conn, MSG_OK, strlen ( MSG_OK ) );
        
    }
    else {
        mg_send( conn, MSG_VARIABLE_NOT_DEFINED, strlen ( MSG_VARIABLE_NOT_DEFINED ) );
    }
    
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_ReadVal
//

void VSCPClientThread::handleVariable_ReadValue( struct mg_connection *conn,
                                                    CControlObject *pCtrlObject, 
                                                    bool bOKResponse )
{
    wxString str;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    pClientItem->m_currentCommand.Trim(false);
    pClientItem->m_currentCommand.Trim(true);

    CVSCPVariable variable;
    if ( 0 != gpobj->m_variables.find( pClientItem->m_currentCommand, variable ) ) {
        
        variable.writeValueToString( str );
        str += _("\r\n");
        mg_send( conn,  str.mbc_str(), strlen( str.mbc_str() ) );

        mg_send( conn, MSG_OK, strlen ( MSG_OK ) );
        
    }
    else {
        mg_send( conn, MSG_VARIABLE_NOT_DEFINED, strlen ( MSG_VARIABLE_NOT_DEFINED ) );
    }
}


///////////////////////////////////////////////////////////////////////////////
// handleVariable_ReadNote
//

void VSCPClientThread::handleVariable_ReadNote( struct mg_connection *conn,
                                                    CControlObject *pCtrlObject, bool bOKResponse )
{
    wxString str;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    pClientItem->m_currentCommand.Trim(false);
    pClientItem->m_currentCommand.Trim(true);

    CVSCPVariable variable;
    if ( 0 != gpobj->m_variables.find( pClientItem->m_currentCommand,variable ) ) {
        
        str = variable.getNote();
        str = _("+OK - ") + str + _("\r\n");
        mg_send( conn,  str.mbc_str(), strlen( str.mbc_str() ) );

        mg_send( conn, MSG_OK, strlen ( MSG_OK ) );
        
    }
    else {
        mg_send( conn, MSG_VARIABLE_NOT_DEFINED, strlen ( MSG_VARIABLE_NOT_DEFINED ) );
    }
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_Reset
//

void VSCPClientThread::handleVariable_Reset( struct mg_connection *conn,
                                                CControlObject *pCtrlObject  )
{
    wxString str;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    pClientItem->m_currentCommand.Trim(false);
    pClientItem->m_currentCommand.Trim(true);

    CVSCPVariable variable;
    
    if ( 0 != gpobj->m_variables.find( pClientItem->m_currentCommand, variable ) ) {
        
        variable.Reset();
        
        // Update in database
        if ( !gpobj->m_variables.update( variable ) ) {
            mg_send( conn, MSG_VARIABLE_NO_SAVE, strlen ( MSG_VARIABLE_NO_SAVE ) );
            return;
        }
        
    }
    else {
        mg_send( conn, MSG_VARIABLE_NOT_DEFINED, strlen ( MSG_VARIABLE_NOT_DEFINED ) );
    }

    mg_send( conn, MSG_OK, strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_ReadReset
//

void VSCPClientThread::handleVariable_ReadReset( struct mg_connection *conn,
                                                    CControlObject *pCtrlObject )
{
    wxString str;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    pClientItem->m_currentCommand.Trim(false);
    pClientItem->m_currentCommand.Trim(true);
    
    if ( pClientItem->CommandStartsWith(_("vscp.") ) ) {
        mg_send( conn, MSG_VARIABLE_NOT_STOCK, strlen ( MSG_VARIABLE_NOT_STOCK ) );
        return;
    }

    CVSCPVariable variable;
    
    if ( 0 != gpobj->m_variables.find( pClientItem->m_currentCommand, variable ) ) {
        
        variable.writeValueToString( str );
        str = _("+OK - ") + str + _("\r\n");
        mg_send( conn,  str.mbc_str(), strlen( str.mbc_str() ) );
    
        variable.Reset();
        
        // Update in database
        if ( !gpobj->m_variables.update( variable ) ) {
            mg_send( conn, MSG_VARIABLE_NO_SAVE, strlen ( MSG_VARIABLE_NO_SAVE ) );
            return;
        }
        
    }
    else {
        mg_send( conn, MSG_VARIABLE_NOT_DEFINED, strlen ( MSG_VARIABLE_NOT_DEFINED ) );
    }

    mg_send( conn, MSG_OK, strlen( MSG_OK ) );
}


///////////////////////////////////////////////////////////////////////////////
// handleVariable_Remove
//

void VSCPClientThread::handleVariable_Remove( struct mg_connection *conn,
                                                CControlObject *pCtrlObject )
{
    wxString str;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    pClientItem->m_currentCommand.Trim(false);
    pClientItem->m_currentCommand.Trim(true);
    
    if ( pClientItem->CommandStartsWith(_("vscp.") ) ) {
        mg_send( conn, MSG_VARIABLE_NOT_STOCK, strlen ( MSG_VARIABLE_NOT_STOCK ) );
        return;
    }
    
    if ( !gpobj->m_variables.remove( pClientItem->m_currentCommand ) ) {
        mg_send( conn, MSG_VARIABLE_NOT_DEFINED, strlen ( MSG_VARIABLE_NOT_DEFINED ) );        
    }

    mg_send( conn, MSG_OK, strlen( MSG_OK ) );
}


///////////////////////////////////////////////////////////////////////////////
// handleVariable_ReadRemove
//

void VSCPClientThread::handleVariable_ReadRemove( struct mg_connection *conn,
                                                    CControlObject *pCtrlObject )
{
    wxString str;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    pClientItem->m_currentCommand.Trim(false);
    pClientItem->m_currentCommand.Trim(true);
    
    if ( pClientItem->CommandStartsWith(_("vscp.") ) ) {
        mg_send( conn, MSG_VARIABLE_NOT_STOCK, strlen ( MSG_VARIABLE_NOT_STOCK ) );
        return;
    }

    CVSCPVariable variable;
    if ( 0 != gpobj->m_variables.find( pClientItem->m_currentCommand, variable ) ) {
        
        variable.writeValueToString( str );
        str = _("+OK - ") + str + _("\r\n");
        mg_send( conn,  str.mbc_str(), strlen( str.mbc_str() ) );
    
        gpobj->m_variables.remove( variable );
        
    }
    else {
        mg_send( conn, MSG_VARIABLE_NOT_DEFINED, strlen ( MSG_VARIABLE_NOT_DEFINED ) );
    }

    mg_send( conn, MSG_OK, strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_Length
//

void VSCPClientThread::handleVariable_Length( struct mg_connection *conn,
                                                CControlObject *pCtrlObject )
{
    wxString str;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    pClientItem->m_currentCommand.Trim(false);
    pClientItem->m_currentCommand.Trim(true);

    CVSCPVariable variable;
    if ( 0 != gpobj->m_variables.find( pClientItem->m_currentCommand, variable ) ) {
        
        str = wxString::Format( _("%zu"), variable.getLength() );
        str = _("+OK - ") + str + _("\r\n");
        mg_send( conn,  str.mbc_str(), strlen( str.mbc_str() ) );
    
        gpobj->m_variables.remove( variable );
        
    }
    else {
        mg_send( conn, MSG_VARIABLE_NOT_DEFINED, strlen ( MSG_VARIABLE_NOT_DEFINED ) );
    }

    mg_send( conn, MSG_OK, strlen( MSG_OK ) );
}



///////////////////////////////////////////////////////////////////////////////
// handleVariable_Load
//

void VSCPClientThread::handleVariable_Load( struct mg_connection *conn,
                                                CControlObject *pCtrlObject )
{
    wxString path;  // Empty to load from default path
    gpobj->m_variables.loadFromXML( path );  // TODO add path + type

    mg_send( conn, MSG_OK, strlen( MSG_OK ) );
}


///////////////////////////////////////////////////////////////////////////////
// handleVariable_Save
//

void VSCPClientThread::handleVariable_Save( struct mg_connection *conn,
                                                CControlObject *pCtrlObject )
{
    wxString path;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    pClientItem->m_currentCommand.Trim(false);
    pClientItem->m_currentCommand.Trim(true);

    // Construct path to save to (always relative to root)
    // may not contain ".."
    path = gpobj->m_rootFolder;
    path += pClientItem->m_currentCommand;
    
    if ( wxNOT_FOUND != path.Find( _("..") ) ) {
        mg_send( conn, MSG_INVALID_PATH, strlen( MSG_INVALID_PATH ) );
        return;
    }

    gpobj->m_variables.save( path );

    mg_send( conn, MSG_OK, strlen( MSG_OK ) );
}




// ****************************************************************************



///////////////////////////////////////////////////////////////////////////////
// handleClientDm
//

void VSCPClientThread::handleClientDm( struct mg_connection *conn,
                                        CControlObject *pCtrlObject  )
{
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    gpobj->logMsg ( pClientItem->m_currentCommand, DAEMON_LOGMSG_NORMAL );

    pClientItem->m_currentCommand.Trim(false);
    pClientItem->m_currentCommand.Trim(true);

    if ( pClientItem->CommandStartsWith(_("enable") ) ) {
        handleDM_Enable( conn, pCtrlObject );
    }
    else if ( pClientItem->CommandStartsWith(_("disable") ) ) {
        handleDM_Enable( conn, pCtrlObject );
    }
    else if ( pClientItem->CommandStartsWith(_("list") ) ) {
        handleDM_List( conn, pCtrlObject );
    }
    else if ( pClientItem->CommandStartsWith(_("add") ) ) {
        handleDM_Add( conn, pCtrlObject );
    }
    else if ( pClientItem->CommandStartsWith(_("delete") ) ) {
        handleDM_Delete( conn, pCtrlObject );
    }
    else if ( pClientItem->CommandStartsWith(_("reset") ) ) {
        handleDM_Reset( conn, pCtrlObject );
    }
    else if ( pClientItem->CommandStartsWith(_("trig") ) ) {
        handleDM_Trigger( conn, pCtrlObject );
    }
    else if ( pClientItem->CommandStartsWith(_("clrtrig") ) ) {
        handleDM_ClearTriggerCount( conn, pCtrlObject );
    }
    else if ( pClientItem->CommandStartsWith(_("clrerr") ) ) {
        handleDM_ClearErrorCount( conn, pCtrlObject );
    }
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_Enable
//

void VSCPClientThread::handleDM_Enable( struct mg_connection *conn,
                                            CControlObject *pCtrlObject  )
{
    unsigned short pos;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    if ( pClientItem->CommandStartsWith(_("all") ) ) {

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
            mg_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            gpobj->m_dm.m_mutexDM.Unlock();
            return;
        }

        DMLIST::compatibility_iterator node = gpobj->m_dm.m_DMList.Item( pos );
        ( node->GetData() )->enableRow();
        gpobj->m_dm.m_mutexDM.Unlock();

    }

    mg_send( conn, MSG_OK, strlen( MSG_OK ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleDM_Disable
//

void VSCPClientThread::handleDM_Disable( struct mg_connection *conn,
                                            CControlObject *pCtrlObject  )
{
    unsigned short pos;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    if ( pClientItem->CommandStartsWith(_("all") ) ) {

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
        wxStringTokenizer tkz( pClientItem->m_currentCommand, _(",") );
        while ( tkz.HasMoreTokens() ) {

            pos = vscp_readStringValue( tkz.GetNextToken() );

            gpobj->m_dm.m_mutexDM.Lock();

            if ( pos > ( gpobj->m_dm.m_DMList.GetCount() - 1 ) ) {
                mg_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
                gpobj->m_dm.m_mutexDM.Unlock();
                return;
            }

            DMLIST::compatibility_iterator node = gpobj->m_dm.m_DMList.Item( pos );
            ( node->GetData() )->disableRow();

            gpobj->m_dm.m_mutexDM.Unlock();

        }

    }

    mg_send( conn, MSG_OK, strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_List
//

void VSCPClientThread::handleDM_List( struct mg_connection *conn,
                                        CControlObject *pCtrlObject  )
{
    // Valid commands at this point
    // dm list
    // dm list all
    // dm list *
    // dm list 1
    // dm list 1,2,3,4,98

    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    pClientItem->m_currentCommand.Trim(false);
    pClientItem->m_currentCommand.Trim(true);

    // if "list" add "all"
    if ( 0 == pClientItem->m_currentCommand.Length() ) {
        pClientItem->m_currentCommand = _("ALL");
    }

    else if ( pClientItem->CommandStartsWith(_("new") ) ||
                pClientItem->CommandStartsWith(_("*") ) ) {

        gpobj->m_dm.m_mutexDM.Lock();

        DMLIST::iterator iter;
        for (iter = gpobj->m_dm.m_DMList.begin();
                iter != gpobj->m_dm.m_DMList.end();
                ++iter) {

            dmElement *pDMItem = *iter;
            wxString strRow = pDMItem->getAsString();

            mg_send( conn,  strRow.mb_str(),
                                        strlen ( strRow.mb_str() ) );

        }

        gpobj->m_dm.m_mutexDM.Unlock();

    }
    else {

        // We have a list with specific rows  "list 1,8,9"
        //      first parse the argument to get the rows
        //WX_DEFINE_ARRAY_INT( int, ArrayOfSortedInts );
        wxArrayInt rowArray;
        wxStringTokenizer tok( pClientItem->m_currentCommand, _(",") );
        while ( tok.HasMoreTokens() ) {

            int n = vscp_readStringValue( tok.GetNextToken() );
            rowArray.Add( n );

        }

    }


    mg_send( conn, MSG_OK, strlen ( MSG_OK ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleDM_Add
//

void VSCPClientThread::handleDM_Add( struct mg_connection *conn,
                                        CControlObject *pCtrlObject  )
{
    CClientItem *pClientItem = (CClientItem *)conn->user_data;
    dmElement *pDMItem = new dmElement;

    wxStringTokenizer tkz( pClientItem->m_currentCommand, _(",") );

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

    mg_send( conn, MSG_OK, strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_Delete
//

void VSCPClientThread::handleDM_Delete( struct mg_connection *conn,
                                            CControlObject *pCtrlObject  )
{
    unsigned short pos;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    if ( pClientItem->CommandStartsWith(_("all") ) ) {

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
        wxStringTokenizer tkz( pClientItem->m_currentCommand, _(",") );
        while ( tkz.HasMoreTokens() ) {

            pos = vscp_readStringValue( tkz.GetNextToken() );

            gpobj->m_dm.m_mutexDM.Lock();

            if ( pos > ( gpobj->m_dm.m_DMList.GetCount() - 1 ) ) {
                mg_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
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

    mg_send( conn,  MSG_OK, strlen ( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_Reset
//

void VSCPClientThread::handleDM_Reset( struct mg_connection *conn, CControlObject *pCtrlObject  )
{
    gpobj->stopDaemonWorkerThread();
    gpobj->startDaemonWorkerThread();

    mg_send( conn, MSG_OK, strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_Trigger
//

void VSCPClientThread::handleDM_Trigger( struct mg_connection *conn,
                                            CControlObject *pCtrlObject )
{
    unsigned short pos;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    // Get the position
    wxStringTokenizer tkz( pClientItem->m_currentCommand, _(",") );
    while ( tkz.HasMoreTokens() ) {

        pos = vscp_readStringValue( tkz.GetNextToken() );

        gpobj->m_dm.m_mutexDM.Lock();

        if ( pos > ( gpobj->m_dm.m_DMList.GetCount() - 1 ) ) {
            mg_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            gpobj->m_dm.m_mutexDM.Unlock();
            return;
        }

        DMLIST::compatibility_iterator node = gpobj->m_dm.m_DMList.Item( pos );
        dmElement *pDMItem = node->GetData();
        pDMItem->doAction( NULL );

        gpobj->m_dm.m_mutexDM.Unlock();

    }

    mg_send( conn, MSG_OK, strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_ClearTriggerCount
//


void VSCPClientThread::handleDM_ClearTriggerCount( struct mg_connection *conn,
                                                    CControlObject *pCtrlObject )
{
    unsigned short pos;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    if ( pClientItem->CommandStartsWith(_("all") ) ) {

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
        wxStringTokenizer tkz( pClientItem->m_currentCommand, _(",") );
        while ( tkz.HasMoreTokens() ) {

            pos = vscp_readStringValue( tkz.GetNextToken() );

            gpobj->m_dm.m_mutexDM.Lock();

            if ( pos > ( gpobj->m_dm.m_DMList.GetCount() - 1 ) ) {
                mg_send( conn,  MSG_PARAMETER_ERROR,
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

    mg_send( conn, MSG_OK, strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_ClearErrorCount
//


void VSCPClientThread::handleDM_ClearErrorCount( struct mg_connection *conn,
                                                    CControlObject *pCtrlObject )
{
    unsigned short pos;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    if ( pClientItem->CommandStartsWith(_("all") ) ) {

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
        wxStringTokenizer tkz( pClientItem->m_currentCommand, _(",") );
        while ( tkz.HasMoreTokens() ) {

            pos = vscp_readStringValue( tkz.GetNextToken() );

            gpobj->m_dm.m_mutexDM.Lock();

            if ( pos > ( gpobj->m_dm.m_DMList.GetCount() - 1 ) ) {
                mg_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
                gpobj->m_dm.m_mutexDM.Unlock();
                return;
            }

            DMLIST::compatibility_iterator node = gpobj->m_dm.m_DMList.Item( pos );
            dmElement *pDMItem = node->GetData();
            pDMItem->m_errorCounter = 0;

            gpobj->m_dm.m_mutexDM.Unlock();

        }

    }

    mg_send( conn, MSG_OK, strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleClientList
//

void VSCPClientThread::handleClientList( struct mg_connection *conn,
                                            CControlObject *pCtrlObject  )
{
    // TODO
    mg_send( conn, MSG_OK, strlen ( MSG_OK ) );
}


///////////////////////////////////////////////////////////////////////////////
// handleClientDriver
//

void VSCPClientThread::handleClientDriver( struct mg_connection *conn,
                                                CControlObject *pCtrlObject  )
{
    // TODO
}


///////////////////////////////////////////////////////////////////////////////
// handleClientHelp
//

void VSCPClientThread::handleClientHelp( struct mg_connection *conn,
                                            CControlObject *pCtrlObject )
{
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    pClientItem->m_currentCommand.Trim(true);
    pClientItem->m_currentCommand.Trim(false);

    if ( 0 == pClientItem->m_currentCommand.Length() ) {

        wxString str = _("Help for the VSCP tcp/ip interface\r\n");
                str += _("====================================================================\r\n");
                str += _("To get more information about a specific command issue 'HELP command'\r\n");
                str += _("+                 - Repeat last command.\r\n");
                str += _("NOOP              - No operation. Does nothing.\r\n");
                str += _("QUIT              - Close the connection.\r\n");
                str += _("USER 'username'   - Username for login. \r\n");
                str += _("PASS 'password'   - Password for login.  \r\n");
                str += _("CHALLENGE 'token' - Get session id.  \r\n");
                str += _("SEND 'event'      - Send an event.   \r\n");
                str += _("RETR 'count'      - Retrive n events from input queue.   \r\n");
                str += _("RCVLOOP           - Will retrieve events in an endless loop until the connection is closed by the client or QUITLOOP is sent.\r\n");
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
                mg_send( conn, (const char *)str.mbc_str(), str.Length() );
    }
    else if ( pClientItem->CommandStartsWith( _("+") ) ) {
        wxString str = _("'+' repeats the last given command.\r\n");
        mg_send( conn, (const char *)str.mbc_str(), str.Length() );
    }
    else if ( pClientItem->CommandStartsWith( _("noop") ) ) {
        wxString str = _("'NOOP' Does absolutly nothing but giving a success in return.\r\n");
        mg_send( conn, (const char *)str.mbc_str(), str.Length() );
    }
    else if ( pClientItem->CommandStartsWith( _("quit") ) ) {
        wxString str = _("'QUIT' Quit a session with the VSCP daemon and closes the connection.\r\n");
        mg_send( conn, (const char *)str.mbc_str(), str.Length() );
    }
    else if ( pClientItem->CommandStartsWith( _("user") ) ) {
        wxString str = _("'USER' Used to login to the system together with PASS. Connection will be closed if bad credentials are given.\r\n");
        mg_send( conn, (const char *)str.mbc_str(), str.Length() );
    }
    else if ( pClientItem->CommandStartsWith( _("pass") ) ) {
        wxString str = _("'PASS' Used to login to the system together with USER. Connection will be closed if bad credentials are given.\r\n");
        mg_send( conn, (const char *)str.mbc_str(), str.Length() );
    }
    else if ( pClientItem->CommandStartsWith( _("quit") ) ) {
        wxString str = _("'QUIT' Quit a session with the VSCP daemon and closes the connection.\r\n");
        mg_send( conn, (const char *)str.mbc_str(), str.Length() );
    }
    else if ( pClientItem->CommandStartsWith( _("send") ) ) {
        wxString str = _("'SEND event'.\r\nThe event is given as 'head,class,type,obid,datetime,time-stamp,GUID,data1,data2,data3....' \r\n");
        str += _("Normally set 'head' and 'obid' to zero. \r\nIf timestamp is set to zero it will be set by the server. \r\nIf GUID is given as '-' ");
        str += _("the GUID of the interface will be used. \r\nThe GUID should be given on the form MSB-byte:MSB-byte-1:MSB-byte-2. \r\n");
        mg_send( conn, (const char *)str.mbc_str(), str.Length() );
    }
    else if ( pClientItem->CommandStartsWith( _("retr") ) ) {
        wxString str = _("'RETR count' - Retrieve one (if no argument) or 'count' event(s). ");
        str += _("Events are retrived on the form head,class,type,obid,datetime,time-stamp,GUID,data0,data1,data2,...........\r\n");
        mg_send( conn, (const char *)str.mbc_str(), str.Length() );
    }
    else if ( pClientItem->CommandStartsWith( _("rcvloop") ) ) {
        wxString str = _("'RCVLOOP' - Enter the receive loop and receive events continously or until ");
        str += _("terminated with 'QUITLOOP'. Events are retrived on the form head,class,type,obid,time-stamp,GUID,data0,data1,data2,...........\r\n");
        mg_send( conn, (const char *)str.mbc_str(), str.Length() );
    }
    else if ( pClientItem->CommandStartsWith( _("quitloop") ) ) {
        wxString str = _("'QUITLOOP' - End 'RCVLOOP' event receives.\r\n");
        mg_send( conn, (const char *)str.mbc_str(), str.Length() );
    }
    else if ( pClientItem->CommandStartsWith( _("cdta") ) ||
                pClientItem->CommandStartsWith( _("chkdata") ) ) {
        wxString str = _("'CDTA' or 'CHKDATA' - Check if there is events in the input queue.\r\n");
        mg_send( conn, (const char *)str.mbc_str(), str.Length() );
    }
    else if ( pClientItem->CommandStartsWith( _("clra") ) ||
                pClientItem->CommandStartsWith( _("clrall") ) ) {
        wxString str = _("'CLRA' or 'CLRALL' - Clear input queue.\r\n");
        mg_send( conn, (const char *)str.mbc_str(), str.Length() );
    }
    else if ( pClientItem->CommandStartsWith( _("stat") ) ) {
        wxString str = _("'STAT' - Get statistical information.\r\n");
        mg_send( conn, (const char *)str.mbc_str(), str.Length() );
    }
    else if ( pClientItem->CommandStartsWith( _("info") ) ) {
        wxString str = _("'INFO' - Get status information.\r\n");
        mg_send( conn, (const char *)str.mbc_str(), str.Length() );
    }
    else if ( pClientItem->CommandStartsWith( _("chid") ) ||
                pClientItem->CommandStartsWith( _("getchid") ) ) {
        wxString str = _("'CHID' or 'GETCHID' - Get channel id.\r\n");
        mg_send( conn, (const char *)str.mbc_str(), str.Length() );
    }
    else if ( pClientItem->CommandStartsWith( _("sgid") ) ||
                pClientItem->CommandStartsWith( _("setguid") ) ) {
        wxString str = _("'SGID' or 'SETGUID' - Set GUID for channel.\r\n");
        mg_send( conn, (const char *)str.mbc_str(), str.Length() );
    }
    else if ( pClientItem->CommandStartsWith( _("ggid") ) ||
                pClientItem->CommandStartsWith( _("getguid") ) ) {
        wxString str = _("'GGID' or 'GETGUID' - Get GUID for channel.\r\n");
        mg_send( conn, (const char *)str.mbc_str(), str.Length() );
    }
    else if ( pClientItem->CommandStartsWith( _("vers") ) ||
                pClientItem->CommandStartsWith( _("version") ) ) {
        wxString str = _("'VERS' or 'VERSION' - Get version of VSCP daemon.\r\n");
        mg_send( conn, (const char *)str.mbc_str(), str.Length() );
    }
    else if ( pClientItem->CommandStartsWith( _("sflt") ) ||
                pClientItem->CommandStartsWith( _("setfilter") ) ) {
        wxString str = _("'SFLT' or 'SETFILTER' - Set filter for channel. ");
        str += _("The format is 'filter-priority, filter-class, filter-type, filter-GUID' \r\n");
        str += _("Example:  \r\nSETFILTER 1,0x0000,0x0006,ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00\r\n");
        mg_send( conn, (const char *)str.mbc_str(), str.Length() );
    }
    else if ( pClientItem->CommandStartsWith( _("smsk") ) ||
                pClientItem->CommandStartsWith( _("setmask") ) ) {
        wxString str = _("'SMSK' or 'SETMASK' - Set mask for channel. ");
        str += _("The format is 'mask-priority, mask-class, mask-type, mask-GUID' \r\n");
        str += _("Example:  \r\nSETMASK 0x0f,0xffff,0x00ff,ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00 \r\n");
        mg_send( conn, (const char *)str.mbc_str(), str.Length() );
    }
    else if ( pClientItem->CommandStartsWith( _("help") ) ) {
        wxString str = _("'HELP [command]' This command. Gives help about available commands and the usage.\r\n");
        mg_send( conn, (const char *)str.mbc_str(), str.Length() );
    }
    else if ( pClientItem->CommandStartsWith( _("test") ) ) {
        wxString str = _("'TEST [sequency]' Test command for debugging.\r\n");
        mg_send( conn, (const char *)str.mbc_str(), str.Length() );
    }
    else if ( pClientItem->CommandStartsWith( _("shutdown") ) ) {
        wxString str = _("'SHUTDOWN' Shutdown the daemon.\r\n");
        mg_send( conn, (const char *)str.mbc_str(), str.Length() );
    }
    else if ( pClientItem->CommandStartsWith( _("restart") ) ) {
        wxString str = _("'RESTART' Restart the daemon.\r\n");
        mg_send( conn, (const char *)str.mbc_str(), str.Length() );
    }
    else if ( pClientItem->CommandStartsWith( _("driver") ) ) {
        wxString str = _("'DRIVER' Handle (load/unload/update/start/stop) Level I/Level II drivers.\r\n");
        str += _("'DRIVER install package' .\r\n");
        str += _("'DRIVER uninstall package' .\r\n");
        str += _("'DRIVER upgrade package' .\r\n");
        str += _("'DRIVER start package' .\r\n");
        str += _("'DRIVER stop package' .\r\n");
        str += _("'DRIVER reload package' .\r\n");
        mg_send( conn, (const char *)str.mbc_str(), str.Length() );
    }
    else if ( pClientItem->CommandStartsWith( _("file") ) ) {
        wxString str = _("'FILE' Handle daemon files.\r\n");
        str += _("'FILE dir'.\r\n");
        str += _("'FILE copy'.\r\n");
        str += _("'FILE move'.\r\n");
        str += _("'FILE delete'.\r\n");
        str += _("'FILE list'.\r\n");
        mg_send( conn, (const char *)str.mbc_str(), str.Length() );
    }
    else if ( pClientItem->CommandStartsWith( _("udp") ) ) {
        wxString str = _("'UDP' Handle UDP interface.\r\n");
        str += _("'UDP enable'.\r\n");
        str += _("'UDP disable' .\r\n");
        mg_send( conn, (const char *)str.mbc_str(), str.Length() );
    }
    else if ( pClientItem->CommandStartsWith( _("remote") ) ) {
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
        mg_send( conn, (const char *)str.mbc_str(), str.Length() );
    }
    else if ( pClientItem->CommandStartsWith( _("interface") ) ) {
        wxString str = _("'INTERFACE' Handle interfaces on the daemon.\r\n");
        str += _("'INTERFACE list'.\r\n");
        str += _("'INTERFACE close'.\r\n");
        mg_send( conn, (const char *)str.mbc_str(), str.Length() );
    }
    else if ( pClientItem->CommandStartsWith( _("dm") ) ) {
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
        mg_send( conn, (const char *)str.mbc_str(), str.Length() );
    }
    else if ( pClientItem->CommandStartsWith( _("var") ) ||
                pClientItem->CommandStartsWith( _("variable") ) ) {
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
        mg_send( conn, (const char *)str.mbc_str(), str.Length() );
    }

    mg_send( conn, MSG_OK, strlen(MSG_OK) );
    return;
}