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
#include <time.h> 

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
#include "../../common/net_skeleton.h"
#include "version.h"
#include "controlobject.h"


//WX_DEFINE_LIST(TCPCLIENTS);


///////////////////////////////////////////////////////////////////////////////
// VSCPWebServerThread
//
// This thread listens for conection on a TCP socket and starts a new thread
// to handle client requests
//

VSCPClientThread::VSCPClientThread()
: wxThread(wxTHREAD_JOINABLE)
{
    //OutputDebugString( "TCP ClientThread: Create");
    m_bQuit = false;
    m_pCtrlObject = NULL;
    //m_numClients = 0;
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
	if ( NULL == m_pCtrlObject ) return NULL;

	ns_mgr_init( &m_pCtrlObject->m_mgrTcpIpServer, m_pCtrlObject, VSCPClientThread::ev_handler );

	m_pCtrlObject->m_strTcpInterfaceAddress.Trim();
	m_pCtrlObject->m_strTcpInterfaceAddress.Trim( false );
	wxStringTokenizer tkz( m_pCtrlObject->m_strTcpInterfaceAddress, _(" ") );
	while ( tkz.HasMoreTokens() ) {
		
		wxString str = tkz.GetNextToken();
		str.Trim();
		str.Trim( false );
		if ( 0 == str.Length() ) continue;

		// Bind to this interface
		ns_bind( &m_pCtrlObject->m_mgrTcpIpServer, (const char *)str.mbc_str(), NULL ); 
	}

	m_pCtrlObject->logMsg(_T("TCP Client: Thread started.\n"), DAEMON_LOGMSG_INFO);

	while ( !TestDestroy() && !m_bQuit ) {
		ns_mgr_poll( &m_pCtrlObject->m_mgrTcpIpServer, 50 );
		Yield();
	}

	// release the server
	ns_mgr_free( &m_pCtrlObject->m_mgrTcpIpServer );

    m_pCtrlObject->logMsg( _T( "TCP ClientThread: Quit.\n" ), DAEMON_LOGMSG_INFO );

    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void VSCPClientThread::OnExit()
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
// ev_handler
//

void 
VSCPClientThread::ev_handler(struct ns_connection *conn, enum ns_event ev, void *pUser) 
{
	char rbuf[ 2048 ];
    int pos4lf; 

	struct iobuf *io = &conn->recv_iobuf;
	CControlObject *pCtrlObject = (CControlObject *)conn->mgr->user_data;
	CClientItem *pClientItem = ( CClientItem *)conn->user_data;

	switch (ev) {
	
		case NS_CONNECT: // connect() succeeded or failed. int *success_status
			pCtrlObject->logMsg(_T("TCP Client: Connect.\n"), DAEMON_LOGMSG_INFO);
			break;

		case NS_ACCEPT:	// New connection accept()-ed. union socket_address *remote_addr
			{
				pCtrlObject->logMsg(_T("TCP Client: --Accept.\n"), DAEMON_LOGMSG_INFO);

				// We need to create a clientobject and add this object to the list
				pClientItem = new CClientItem;
				if ( NULL == pClientItem ) {
					pCtrlObject->logMsg ( _T ( "[TCP/IP Client] Unable to allocate memory for client.\n" ), DAEMON_LOGMSG_ERROR );
					conn->flags |= NSF_CLOSE_IMMEDIATELY;	// Close connection
					return;
				}

				// save the client item
				conn->user_data = pClientItem;

				// This is now an active Client
				pClientItem->m_bOpen = true; 
				pClientItem->m_type =  CLIENT_ITEM_INTERFACE_TYPE_CLIENT_TCPIP;
				pClientItem->m_strDeviceName = _("Remote TCP/IP Client. Started at ");
				wxDateTime now = wxDateTime::Now(); 
				pClientItem->m_strDeviceName += now.FormatISODate();
				pClientItem->m_strDeviceName += _(" ");
				pClientItem->m_strDeviceName += now.FormatISOTime();
				
				// Add the client to the Client List
				pCtrlObject->m_wxClientMutex.Lock();
				pCtrlObject->addClient( pClientItem );
				pCtrlObject->m_wxClientMutex.Unlock();
				
				// Clear the filter (Allow everything )
				vscp_clearVSCPFilter( &pClientItem->m_filterVSCP );

				// Send welcome message
				wxString str = _(MSG_WELCOME);
				str += _("+OK Version: ");
				str += _(VSCPD_DISPLAY_VERSION);
				str += _("\r\n");
				str += _("+OK ");
				str += _(VSCPD_COPYRIGHT);
				str += _("\r\n");
				str += _(MSG_OK);
				ns_send( conn, (const char*)str.mbc_str(), str.Length() );	 

				pCtrlObject->logMsg(_T("TCP Client: Ready to serve client.\n"), DAEMON_LOGMSG_DEBUG);
			}
			break;

		case NS_CLOSE:

			// Close client
			pClientItem->m_bOpen = false;
			conn->flags |= NSF_CLOSE_IMMEDIATELY;	// Close connection
			
			// Remove the client from the Client List
			pCtrlObject->m_wxClientMutex.Lock();
			pCtrlObject->removeClient( pClientItem );
			pCtrlObject->m_wxClientMutex.Unlock();
			break;

		case NS_RECV:

			// Read new data
			memset( rbuf, 0, sizeof( rbuf ) );
			memcpy( rbuf, io->buf, io->len );
			iobuf_remove(io, io->len); 
            pClientItem->m_readBuffer += wxString::FromUTF8( rbuf );

			// Check if command already in buffer
			while ( wxNOT_FOUND != ( pos4lf = pClientItem->m_readBuffer.Find ( (const char)0x0a ) ) ) {
				wxString strCmdGo = pClientItem->m_readBuffer.Mid( 0, pos4lf );
				pCtrlObject->getTCPIPServer()->CommandHandler( conn, 
										pCtrlObject, 
										strCmdGo );
				pClientItem->m_readBuffer = pClientItem->m_readBuffer.Right( pClientItem->m_readBuffer.Length()-pos4lf-1 );
			}
			break;

		case NS_SEND:
			break;

		case NS_POLL:
			if ( conn->flags & NSF_USER_1) {
				
				pCtrlObject->getTCPIPServer()->sendOneEventFromQueue( conn, pCtrlObject, false );
				
				// Send '+OK<CR><LF>' every two seconds to indicate that the
				// link is open		
				if ( ( wxGetUTCTime()-pClientItem->m_timeRcvLoop ) > 2 ) {
					pClientItem->m_timeRcvLoop = wxGetUTCTime();
					ns_send( conn, "+OK\r\n", 5 );
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
VSCPClientThread::CommandHandler( struct ns_connection *conn, CControlObject *pCtrlObject, wxString& strCommand )
{
	CClientItem *pClientItem = (CClientItem *)conn->user_data;
	
	if ( NULL == pClientItem ) {
		pCtrlObject->logMsg ( _T ( "[TCP/IP Client] ClientItem pointer is NULL in command handler.\n" ), DAEMON_LOGMSG_ERROR );
		conn->flags |= NSF_CLOSE_IMMEDIATELY;	// Close connection
	}

	if ( NULL == pCtrlObject ) {
		pCtrlObject->logMsg ( _T ( "[TCP/IP Client] ControlObject pointer is NULL in command handeler.\n" ), DAEMON_LOGMSG_ERROR );
		conn->flags |= NSF_CLOSE_IMMEDIATELY;	// Close connection
	}

	pClientItem->m_currentCommand = strCommand;
	pClientItem->m_currentCommandUC = pClientItem->m_currentCommand.Upper();
    pClientItem->m_currentCommand.Trim();
    pClientItem->m_currentCommand.Trim( false );

	// If nothing to handle just return
	if ( 0 == pClientItem->m_currentCommand.Length() ) {
        ns_send( conn,  MSG_OK, strlen ( MSG_OK ) );      
        return;
    }
    
	pClientItem->m_currentCommandUC.Trim();
    pClientItem->m_currentCommandUC.Trim( false );
	//wxLogDebug( _("Argument = ") + pClientItem->m_currentCommandUC );

REPEAT_COMMAND:

    

    //*********************************************************************
    //                            No Operation
    //*********************************************************************
    if ( 0 == pClientItem->m_currentCommandUC.Find ( _( "NOOP" ) ) ) {
	    ns_send( conn,  MSG_OK, strlen ( MSG_OK ) );
    }

	//*********************************************************************
    //                             Send event
    //*********************************************************************
    else if ( 0 == pClientItem->m_currentCommandUC.Find ( _( "SEND " ) ) ) {
        if ( checkPrivilege( conn, pCtrlObject, 4 ) ) {
			handleClientSend( conn, pCtrlObject );
		}
    }

    //*********************************************************************
    //                            Read event
    //*********************************************************************
    else if ( 0 == pClientItem->m_currentCommandUC.Find ( _( "RETR" ) ) ) {
        if ( checkPrivilege( conn, pCtrlObject, 2 ) ) {
			handleClientReceive( conn, pCtrlObject );
		}
    }

    //*********************************************************************
    //                            Data Available
    //*********************************************************************
    else if ( ( 0 == pClientItem->m_currentCommandUC.Find ( _( "CDTA" ) ) ) || 
               ( 0 == pClientItem->m_currentCommandUC.Find ( _( "CHKDATA" ) ) ) ) {
        if ( checkPrivilege( conn, pCtrlObject, 1 ) ) {
			handleClientDataAvailable( conn, pCtrlObject );
		}
    }

    //*********************************************************************
    //                          Clear input queue
    //*********************************************************************
    else if ( ( 0 == pClientItem->m_currentCommandUC.Find ( _( "CLRA" ) ) ) ||
                ( 0 == pClientItem->m_currentCommandUC.Find ( _( "CLRALL" ) ) ) ) {
        if ( checkPrivilege( conn, pCtrlObject, 1 ) ) {
			handleClientClearInputQueue( conn, pCtrlObject );
		}
    }


    //*********************************************************************
    //                           Get Statistics
    //*********************************************************************
    else if ( 0 == pClientItem->m_currentCommandUC.Find ( _( "STAT" ) ) ) {
         if ( checkPrivilege( conn, pCtrlObject, 1 ) ) {
			 handleClientGetStatistics( conn, pCtrlObject );
		 }
    }

    //*********************************************************************
    //                            Get Status
    //*********************************************************************
    else if ( 0 == pClientItem->m_currentCommandUC.Find ( _( "INFO" ) ) ) {
        if ( checkPrivilege( conn, pCtrlObject, 1 ) ) {
			handleClientGetStatus( conn, pCtrlObject );
		}
    }

    //*********************************************************************
    //                           Get Channel ID
    //*********************************************************************
    else if ( ( 0 == pClientItem->m_currentCommandUC.Find ( _( "CHID" ) ) ) || 
                ( 0 == pClientItem->m_currentCommandUC.Find ( _( "GETCHID" ) ) ) ) {
        if ( checkPrivilege( conn, pCtrlObject, 1 ) ) {
			handleClientGetChannelID( conn, pCtrlObject );
		}
    }

    //*********************************************************************
    //                          Set Channel GUID
    //*********************************************************************
    else if ( ( 0 == pClientItem->m_currentCommandUC.Find ( _( "SGID" ) ) ) ||
                ( 0 == pClientItem->m_currentCommandUC.Find ( _( "SETGUID" ) ) )) {
        if ( checkPrivilege( conn, pCtrlObject, 6 ) ) {
			handleClientSetChannelGUID( conn, pCtrlObject );
		}
    }

    //*********************************************************************
    //                          Get Channel GUID
    //*********************************************************************
    else if ( ( 0 == pClientItem->m_currentCommandUC.Find ( _( "GGID" ) ) ) ||
                ( 0 == pClientItem->m_currentCommandUC.Find ( _( "GETGUID" ) ) )  ) {
        if ( checkPrivilege( conn, pCtrlObject, 1 ) ) {
			handleClientGetChannelGUID( conn, pCtrlObject );
		}
    }

    //*********************************************************************
    //                           Get Version
    //*********************************************************************
    else if ( ( 0 == pClientItem->m_currentCommandUC.Find ( _( "VERS" ) ) ) ||
            ( 0 == pClientItem->m_currentCommandUC.Find ( _( "VERSION" ) ) ) ) {
        handleClientGetVersion( conn, pCtrlObject );
    }

    //*********************************************************************
    //                           Set Filter
    //*********************************************************************
    else if ( ( 0 == pClientItem->m_currentCommandUC.Find ( _( "SFLT" ) ) ) ||
               ( 0 == pClientItem->m_currentCommandUC.Find ( _( "SETFILTER" ) ) )  ) {
        if ( checkPrivilege( conn, pCtrlObject, 6 ) ) {
			handleClientSetFilter( conn, pCtrlObject );
		}
    }

    //*********************************************************************
    //                           Set Mask
    //*********************************************************************
    else if ( ( 0 == pClientItem->m_currentCommandUC.Find ( _( "SMSK" ) ) ) || 
                ( 0 == pClientItem->m_currentCommandUC.Find ( _( "SETMASK" ) ) ) ) {
        if ( checkPrivilege( conn, pCtrlObject, 6 ) ) {
			handleClientSetMask( conn, pCtrlObject );
		}
    }

    //*********************************************************************
    //                           Username
    //*********************************************************************
    else if ( 0 == pClientItem->m_currentCommandUC.Find ( _( "USER " ) ) ) {
        handleClientUser( conn, pCtrlObject );
    }

    //*********************************************************************
    //                            Password
    //*********************************************************************
    else if ( 0 == pClientItem->m_currentCommandUC.Find ( _( "PASS " ) ) ) {
        if ( !handleClientPassword( conn, pCtrlObject ) ) {
            pCtrlObject->logMsg ( _( "[TCP/IP Clinet] Command: Password. Not authorized.\n" ), DAEMON_LOGMSG_INFO );
			conn->flags |= NSF_CLOSE_IMMEDIATELY;	// Close connection
			return;
        }
    }  

    //*********************************************************************
    //                        + (repeat last command)
    //*********************************************************************
    else if ( 0 == pClientItem->m_currentCommandUC.Find ( _( "+" ) ) ) {
        // Repeat last command
		pClientItem->m_currentCommand = pClientItem->m_lastCommand;
		pClientItem->m_currentCommandUC = pClientItem->m_lastCommand.Upper();
		goto REPEAT_COMMAND;
    }

	//*********************************************************************
	//                               Rcvloop
	//*********************************************************************
	else if ( 0 == pClientItem->m_currentCommandUC.Find ( _( "RCVLOOP" ) ) ) {
		if ( checkPrivilege( conn, pCtrlObject, 2 ) ) {
			pClientItem->m_timeRcvLoop = wxGetUTCTime();
			handleClientRcvLoop( conn, pCtrlObject );
		}
	}

	//*********************************************************************
	//                           Quitloop
	//*********************************************************************
	else if ( 0 == pClientItem->m_currentCommandUC.Find ( _( "QUITLOOP" ) ) ) {
		// Turn of receive loop
		conn->flags &= ~(unsigned int)NSF_USER_1;
		ns_send( conn, MSG_QUIT_LOOP, strlen ( MSG_QUIT_LOOP ) );
	}

	// *********************************************************************
    //                                 QUIT
    // *********************************************************************
	else if ( 0 == pClientItem->m_currentCommandUC.Find ( _( "QUIT" ) ) ) {
        //long test = NSF_CLOSE_IMMEDIATELY;
		pCtrlObject->logMsg( _( "[TCP/IP Client] Command: Close.\n" ), 
								DAEMON_LOGMSG_INFO );
		ns_send( conn, MSG_GOODBY, strlen ( MSG_GOODBY ) );
		conn->flags = NSF_FINISHED_SENDING_DATA;	// Close connection
        return;
    }

	//*********************************************************************
	//                             Help
	//*********************************************************************
	else if ( 0 == pClientItem->m_currentCommandUC.Find ( _( "HELP" ) ) ) {
		handleClientHelp( conn, pCtrlObject );
	}

	//*********************************************************************
	//                             Restart
	//*********************************************************************
	else if ( 0 == pClientItem->m_currentCommandUC.Find ( _( "RESTART" ) ) ) {
		if ( checkPrivilege( conn, pCtrlObject, 15 ) ) {
            handleClientRestart( conn, pCtrlObject );
        }
	}

	//*********************************************************************
	//                             Driver
	//*********************************************************************
	else if ( 0 == pClientItem->m_currentCommandUC.Find ( _( "DRIVER " ) ) ) {
		if ( checkPrivilege( conn, pCtrlObject, 15 ) ) {
            handleClientDriver( conn, pCtrlObject );
        }
	}

	//*********************************************************************
	//                               DM
	//*********************************************************************
	else if ( 0 == pClientItem->m_currentCommandUC.Find ( _( "DM " ) ) ) {
		if ( checkPrivilege( conn, pCtrlObject, 15 ) ) {
            handleClientDm( conn, pCtrlObject );
        }
	}

	//*********************************************************************
	//                             Variable
	//*********************************************************************
	else if ( 0 == pClientItem->m_currentCommandUC.Find ( _( "VARIABLE " ) ) ) {
		if ( checkPrivilege( conn, pCtrlObject, 4 ) ) {
            handleClientVariable( conn, pCtrlObject );
        }
	}

	//*********************************************************************
	//                               File
	//*********************************************************************
	else if ( 0 == pClientItem->m_currentCommandUC.Find ( _( "FILE " ) ) ) {
		if ( checkPrivilege( conn, pCtrlObject, 15 ) ) {
            handleClientFile( conn, pCtrlObject );
        }
	}

	//*********************************************************************
	//                               UDP
	//*********************************************************************
	else if ( 0 == pClientItem->m_currentCommandUC.Find ( _( "UDP " ) ) ) {
		if ( checkPrivilege( conn, pCtrlObject, 15 ) ) {
            handleClientUdp( conn, pCtrlObject );
        }
	}

	//*********************************************************************
	//                         Client/interface
	//*********************************************************************
	else if ( 0 == pClientItem->m_currentCommandUC.Find ( _( "CLIENT " ) ) ) {
		pClientItem->m_currentCommandUC = pClientItem->m_currentCommandUC.Right( pClientItem->m_currentCommandUC.Length()-7 ); // Remove "CLIENT "
		if ( checkPrivilege( conn, pCtrlObject, 15 ) ) {
            handleClientInterface( conn, pCtrlObject );
        }
	}
	else if ( 0 == pClientItem->m_currentCommandUC.Find ( _( "INTERFACE " ) ) ) {
		pClientItem->m_currentCommandUC = pClientItem->m_currentCommandUC.Right( pClientItem->m_currentCommandUC.Length()-10 ); // Remove "INTERFACE "
		if ( checkPrivilege( conn, pCtrlObject, 15 ) ) {
            handleClientInterface( conn, pCtrlObject );
        }
	}

	//*********************************************************************
	//                               User
	//*********************************************************************
	else if ( 0 == pClientItem->m_currentCommandUC.Find ( _( "USER " ) ) ) {
		handleClientUser( conn, pCtrlObject );
	}

	//*********************************************************************
	//                               Test
	//*********************************************************************
	else if ( 0 == pClientItem->m_currentCommandUC.Find ( _( "TEST" ) ) ) {
		if ( checkPrivilege( conn, pCtrlObject, 15 ) ) {
            handleClientTest( conn, pCtrlObject );
        }
	}


	//*********************************************************************
	//                              Shutdown
	//*********************************************************************
	else if ( 0 == pClientItem->m_currentCommandUC.Find ( _( "SHUTDOWN" ) ) ) {
		if ( checkPrivilege( conn, pCtrlObject, 15 ) ) {
            handleClientShutdown( conn, pCtrlObject );
        }
	}

    //*********************************************************************
	//                             WhatCanYouDo
	//*********************************************************************
	else if ( 0 == pClientItem->m_currentCommandUC.Find ( _( "WHATCANYOUDO" ) ) ) {
            handleClientCapabilityRquest( conn, pCtrlObject );
	}

	//*********************************************************************
	//                                Que?
	//*********************************************************************
	else {
		ns_send( conn,  MSG_UNKNOWN_COMMAND, strlen ( MSG_UNKNOWN_COMMAND ) );
	}

	pClientItem->m_lastCommand = pClientItem->m_currentCommand;

}


///////////////////////////////////////////////////////////////////////////////
// handleClientCapabilityRquest
//

void VSCPClientThread::handleClientCapabilityRquest( struct ns_connection *conn, CControlObject *pCtrlObject )
{

}


///////////////////////////////////////////////////////////////////////////////
// isVerified
//

bool VSCPClientThread::isVerified( struct ns_connection *conn, CControlObject *pCtrlObject )
{ 
	CClientItem *pClientItem = (CClientItem *)conn->user_data;

    // Must be accredited to do this
    if ( !pClientItem->m_bAuthorized ) {
        ns_send( conn, MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// checkPrivilege
//

bool VSCPClientThread::checkPrivilege( struct ns_connection *conn, CControlObject *pCtrlObject, unsigned char reqiredPrivilege )
{
	CClientItem *pClientItem = (CClientItem *)conn->user_data;

    // Must be loged on
    if ( !pClientItem->m_bAuthorized ) {
        ns_send( conn,  MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
        return false;
    }

    if ( NULL == pClientItem->m_pUserItem ) {
        ns_send( conn,  MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
        return false;
    }

    if ( (pClientItem->m_pUserItem->m_userRights & 0xf) < reqiredPrivilege ) {
        ns_send( conn,  MSG_LOW_PRIVILEGE_ERROR, strlen ( MSG_LOW_PRIVILEGE_ERROR ) );
        return false;	
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientSend
//

void VSCPClientThread::handleClientSend( struct ns_connection *conn, CControlObject *pCtrlObject )
{
    bool bSent = false;
    bool bVariable = false;
    char data[ 512 ];
    vscpEvent event;
    wxString nameVariable;

	CClientItem *pClientItem = (CClientItem *)conn->user_data;

    // Must be accredited to do this
    if ( !pClientItem->m_bAuthorized ) {
        ns_send( conn, MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    wxString str = pClientItem->m_currentCommand.Right( pClientItem->m_currentCommand.Length() - 5 );
    wxStringTokenizer tkz( str, _(",") );

    // If first character is $ user request us to send content from
    // a variable

    // Get head
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        str.Trim( false ); 
        if ( wxNOT_FOUND == str.Find(_("$") ) ) {
            event.head = vscp_readStringValue( str );
        }
        else {
            CVSCPVariable *pVariable;
            bVariable = true;   // Yes this is a variable send
            
            nameVariable = str.Right( str.Length() - 1 );
            nameVariable.MakeUpper();
            
            if ( NULL == ( pVariable = m_pCtrlObject->m_VSCP_Variables.find( nameVariable ) ) ) {
                ns_send( conn, MSG_VARIABLE_NOT_DEFINED, strlen ( MSG_VARIABLE_NOT_DEFINED ) );
                return;
            }

            // Must be event type
            if ( VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT != pVariable->getType() ) {
                ns_send( conn, MSG_VARIABLE_MUST_BE_EVENT_TYPE, strlen ( MSG_VARIABLE_MUST_BE_EVENT_TYPE ) );
                return;
            }

            // Get the event
            pVariable->getValue( &event );

        }
    }
    else {
        ns_send( conn, MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
        return;	
    }

    if ( !bVariable ) {

        // Get Class
        if ( tkz.HasMoreTokens() ) {
            str = tkz.GetNextToken();
            event.vscp_class = vscp_readStringValue( str );
        }
        else {
            ns_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            return;	
        }
    
        // Get Type
        if ( tkz.HasMoreTokens() ) {
            str = tkz.GetNextToken();
            event.vscp_type = vscp_readStringValue( str );
        }
        else {
            ns_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            return;	
        }

        // Get OBID  -  Kept here to be compatible with receive
        if ( tkz.HasMoreTokens() ) {
            str = tkz.GetNextToken();
            event.obid = vscp_readStringValue( str );
        }   
        else {
            ns_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            return;	
        }

        // Get Timestamp
        if ( tkz.HasMoreTokens() ) {
            str = tkz.GetNextToken();
            event.timestamp = vscp_readStringValue( str );
            if ( !event.timestamp ) {
                event.timestamp = vscp_makeTimeStamp();
            }
        }
        else {
            ns_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
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
            ns_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            return;	
        }

        // Handle data
        event.sizeData = 0;
        while ( tkz.HasMoreTokens() ) {
            str = tkz.GetNextToken();
            data[ event.sizeData++ ] = vscp_readStringValue( str );
        }
        
        if ( event.sizeData > 0 ) {
            // Copy in data
            event.pdata = new uint8_t[ event.sizeData ];
            if ( NULL != event.pdata ) {
                memcpy ( event.pdata, data, event.sizeData );
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
                        wxString::Format( _("[tcp/ip Client] User [%s] not allowed to send event class=%d type=%d.\n"), 
                                                (const char *)pClientItem->m_pUserItem->m_user.wc_str(), 
                                                event.vscp_class, event.vscp_type );			
		
	    pCtrlObject->logMsg ( strErr, DAEMON_LOGMSG_INFO, DAEMON_LOGTYPE_SECURITY );
        
        ns_send( conn, MSG_MOT_ALLOWED_TO_SEND_EVENT, strlen ( MSG_MOT_ALLOWED_TO_SEND_EVENT ) );
        return;
    }

    vscpEvent *pEvent = new vscpEvent;		// Create new VSCP Event
    if ( NULL != pEvent ) {

        // Copy event
        vscp_copyVSCPEvent( pEvent, &event );
        
        // We don't need the original event anymore
        if (pEvent->sizeData) delete [] event.pdata;
        event.pdata = NULL;

        // Save the originating clients id so
        // this client don't get the message back
        pEvent->obid = pClientItem->m_clientID;

        /*wxString dbgStr = 
            wxString::Format( _(" obid = %d clientid = %d"), 
            pEvent->obid,  
            pClientItem->m_clientID ); 
        m_pCtrlObject->logMsg( dbgStr, DAEMON_LOGMSG_INFO );*/

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
                    m_pCtrlObject->logMsg( dbgStr, DAEMON_LOGMSG_DEBUG );
                
            // Find client
            m_pCtrlObject->m_wxClientMutex.Lock();

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
                    m_pCtrlObject->logMsg( dbgStr, DAEMON_LOGMSG_DEBUG );
                                  
                    if ( pItem->m_guid == destguid ) {
                        // Found
                        pDestClientItem = pItem;
				        bSent = true;
                        dbgStr = _("Match ");    
                        m_pCtrlObject->logMsg( dbgStr, DAEMON_LOGMSG_DEBUG );
					    m_pCtrlObject->sendEventToClient( pItem, pEvent );
                        break;
                    }

                }

				m_pCtrlObject->m_wxClientMutex.Unlock();

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
                    pClientItem->m_statistics.cntTransmitData += pEvent->sizeData;
                    pClientItem->m_statistics.cntTransmitFrames++;

            }
            else {

                pClientItem->m_statistics.cntOverruns++;

                vscp_deleteVSCPevent( pEvent );
                ns_send( conn,  MSG_BUFFER_FULL, strlen ( MSG_BUFFER_FULL ) );
            }

        }

    } // Valid pEvent

   ns_send( conn, MSG_OK, strlen ( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleClientReceive
//

void VSCPClientThread::handleClientReceive ( struct ns_connection *conn, CControlObject *pCtrlObject )
{
    unsigned short cnt=1;	// # of messages to read
	CClientItem *pClientItem = (CClientItem *)conn->user_data;

    // Must be accredited to do this
    if ( !pClientItem->m_bAuthorized ) {
        ns_send( conn,  MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    wxString str = pClientItem->m_currentCommand.Right( pClientItem->m_currentCommand.Length() - 4 );
    cnt = vscp_readStringValue( str );

    if ( !cnt ) cnt++;	// No arg is "read one"


    // Read cnt messages
    while ( cnt-- ) {
        wxString strOut;

        if ( !pClientItem->m_bOpen ) {
            ns_send( conn,  MSG_NO_MSG, strlen ( MSG_NO_MSG ) );
            return;
        }
        else {
            if ( false == sendOneEventFromQueue( conn, pCtrlObject ) ) {
				return;
			}
        }

    } // while

    ns_send( conn,  MSG_OK, strlen ( MSG_OK ) );

}

///////////////////////////////////////////////////////////////////////////////
// sendOneEventFromQueue
//

bool VSCPClientThread::sendOneEventFromQueue( struct ns_connection *conn, CControlObject *pCtrlObject, bool bStatusMsg )
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

        strOut.Printf( _("%d,%d,%d,%d,%d,"),
                            pqueueEvent->head,
                            pqueueEvent->vscp_class,
                            pqueueEvent->vscp_type,
                            pqueueEvent->obid,
                            pqueueEvent->timestamp );
		 
        wxString strGUID;
        vscp_writeGuidToString( pqueueEvent, strGUID );
        strOut += strGUID;

        // Handle data
        if ( NULL != pqueueEvent->pdata ) {

            strOut += _(",");
            for ( int i=0; i<pqueueEvent->sizeData; i++ ) {
                wxString wrk;
                wrk.Printf(_("%d"), pqueueEvent->pdata[ i ] );
                if ( ( pqueueEvent->sizeData - 1 ) != i ) {
                    wrk += _(",");
                }

                strOut += wrk;

            }

        }

        strOut += _("\r\n");
        ns_send( conn,  strOut.mb_str(), strlen ( strOut.mb_str() ) );

        //delete pqueueEvent;
		vscp_deleteVSCPevent( pqueueEvent );

		// Let the system work a little
		//ns_mgr_poll( &m_pCtrlObject->m_mgrTcpIpServer, 1 );
    }
    else {
        if ( bStatusMsg ) {
            ns_send( conn,  MSG_NO_MSG, strlen ( MSG_NO_MSG ) );
        }

        return false;

    }

    return true;

}



///////////////////////////////////////////////////////////////////////////////
// handleClientDataAvailable
//

void VSCPClientThread::handleClientDataAvailable ( struct ns_connection *conn, CControlObject *pCtrlObject )
{
    char outbuf[ 1024 ];
	CClientItem *pClientItem = (CClientItem *)conn->user_data;

    // Must be accredited to do this
    if ( !pClientItem->m_bAuthorized ) {
        ns_send( conn,  MSG_NOT_ACCREDITED,
            strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    sprintf ( outbuf,
        "%ld\r\n%s",
        pClientItem->m_clientInputQueue.GetCount(),
        MSG_OK );
	ns_send( conn,  outbuf, strlen ( outbuf ) );


}

///////////////////////////////////////////////////////////////////////////////
// handleClientClearInputQueue
//

void VSCPClientThread::handleClientClearInputQueue ( struct ns_connection *conn, CControlObject *pCtrlObject )
{
	CClientItem *pClientItem = (CClientItem *)conn->user_data;

    // Must be accredited to do this
    if ( !pClientItem->m_bAuthorized ) {
        ns_send( conn,  MSG_NOT_ACCREDITED,
            strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    pClientItem->m_mutexClientInputQueue.Lock();
    pClientItem->m_clientInputQueue.Clear();
    pClientItem->m_mutexClientInputQueue.Unlock();

    ns_send( conn,  MSG_QUEUE_CLEARED, strlen ( MSG_QUEUE_CLEARED ) );
}


///////////////////////////////////////////////////////////////////////////////
// handleClientGetStatistics
//

void VSCPClientThread::handleClientGetStatistics ( struct ns_connection *conn, CControlObject *pCtrlObject )
{
    char outbuf[ 1024 ];
	CClientItem *pClientItem = (CClientItem *)conn->user_data;

    // Must be accredited to do this
    if ( !pClientItem->m_bAuthorized ) {
        ns_send( conn,  MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
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


    ns_send( conn, outbuf, strlen ( outbuf ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleClientGetStatus
//

void VSCPClientThread::handleClientGetStatus ( struct ns_connection *conn, CControlObject *pCtrlObject )
{
    char outbuf[ 1024 ];
	CClientItem *pClientItem = (CClientItem *)conn->user_data;

    // Must be accredited to do this
    if ( !pClientItem->m_bAuthorized ) {
        ns_send( conn,  MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    sprintf ( outbuf, "%lu,%lu,%lu,\"%s\"\r\n%s",
        pClientItem->m_status.channel_status,
        pClientItem->m_status.lasterrorcode,
        pClientItem->m_status.lasterrorsubcode,
        pClientItem->m_status.lasterrorstr,
        MSG_OK );

    ns_send( conn, outbuf, strlen ( outbuf ) );


}

///////////////////////////////////////////////////////////////////////////////
// handleClientGetChannelID
//

void VSCPClientThread::handleClientGetChannelID ( struct ns_connection *conn, CControlObject *pCtrlObject )
{
    char outbuf[ 1024 ];
	CClientItem *pClientItem = (CClientItem *)conn->user_data;

    // Must be accredited to do this
    if ( !pClientItem->m_bAuthorized ) {
        ns_send( conn,  MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    sprintf ( outbuf, "%lu\r\n%s",
        (unsigned long)pClientItem->m_clientID, MSG_OK );

    ns_send( conn,  outbuf, strlen ( outbuf ) );

}


///////////////////////////////////////////////////////////////////////////////
// handleClientSetChannelGUID
//

void VSCPClientThread::handleClientSetChannelGUID ( struct ns_connection *conn, CControlObject *pCtrlObject )
{
	CClientItem *pClientItem = (CClientItem *)conn->user_data;

    // Must be accredited to do this
    if ( !pClientItem->m_bAuthorized ) {
        ns_send( conn,  MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    wxString str;
    if ( ( 0 == pClientItem->m_currentCommandUC.Find ( _( "SGID" ) ) ) ) { 
        str = pClientItem->m_currentCommandUC.Right( pClientItem->m_currentCommandUC.Length() - 5 ); // remove: command + space
    }
    else {
        str = pClientItem->m_currentCommandUC.Right( pClientItem->m_currentCommandUC.Length() - 8 ); // remove: command + space
    }

    pClientItem->m_guid.getFromString(str);

    ns_send( conn,  MSG_OK, strlen ( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleClientGetChannelGUID
//

void VSCPClientThread::handleClientGetChannelGUID ( struct ns_connection *conn, CControlObject *pCtrlObject )
{
    wxString strBuf;
    //char outbuf[ 1024 ];
    //char wrkbuf[ 20 ];
    //int i;
	CClientItem *pClientItem = (CClientItem *)conn->user_data;

    // Must be accredited to do this
    if ( !pClientItem->m_bAuthorized ) {
        ns_send( conn, MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }


    pClientItem->m_guid.toString( strBuf );
    strBuf += _("\r\n");
    strBuf += _(MSG_OK);

    ns_send( conn, strBuf.mb_str(), strlen( strBuf.mb_str() ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleClientGetVersion
//

void VSCPClientThread::handleClientGetVersion ( struct ns_connection *conn, CControlObject *pCtrlObject )
{
    char outbuf[ 1024 ];

	CClientItem *pClientItem = (CClientItem *)conn->user_data;

    // Must be accredited to do this
    if ( !pClientItem->m_bAuthorized ) {
        ns_send( conn,  MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }


    sprintf ( outbuf,
                "%d,%d,%d\r\n%s",
                VSCPD_MAJOR_VERSION,
                VSCPD_MINOR_VERSION,
                VSCPD_SUB_VERSION,
                MSG_OK );

    ns_send( conn,  outbuf, strlen ( outbuf ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleClientSetFilter
//

void VSCPClientThread::handleClientSetFilter ( struct ns_connection *conn, CControlObject *pCtrlObject )
{
	CClientItem *pClientItem = (CClientItem *)conn->user_data;

    // Must be accredited to do this
    if ( !pClientItem->m_bAuthorized ) {
        ns_send( conn,  MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    wxString str;
    if (  0 == pClientItem->m_currentCommandUC.Find ( _( "SFLT" ) ) ) { 
        str = pClientItem->m_currentCommand.Right( pClientItem->m_currentCommand.Length() - 5 );
    }
    else {
        str = pClientItem->m_currentCommand.Right( pClientItem->m_currentCommand.Length() - 10 );  // SETFILTER
    }
    wxStringTokenizer tkz( str, _(",") );

    // Get priority
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        pClientItem->m_filterVSCP.filter_priority = vscp_readStringValue( str );
    }
    else {
        ns_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
        return;	
    }

    // Get Class
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        pClientItem->m_filterVSCP.filter_class = vscp_readStringValue( str );
    }
    else {
        ns_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
        return;	
    }


    // Get Type
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        pClientItem->m_filterVSCP.filter_type = vscp_readStringValue( str );
    }
    else {
        ns_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
        return;	
    }

    // Get GUID
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        vscp_getGuidFromStringToArray( pClientItem->m_filterVSCP.filter_GUID, str );
    }
    else {
        ns_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
        return;	
    }

    ns_send( conn,  MSG_OK, strlen ( MSG_OK ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleClientSetMask
//

void VSCPClientThread::handleClientSetMask ( struct ns_connection *conn, CControlObject *pCtrlObject )
{
	CClientItem *pClientItem = (CClientItem *)conn->user_data;

    // Must be accredited to do this
    if ( !pClientItem->m_bAuthorized ) {
        ns_send( conn,  MSG_NOT_ACCREDITED, strlen ( MSG_NOT_ACCREDITED ) );
        return;
    }

    wxString str;
    if (  0 == pClientItem->m_currentCommandUC.Find ( _( "SMSK" ) ) ) { 
        str = pClientItem->m_currentCommand.Right( pClientItem->m_currentCommand.Length() - 5 );
    }
    else {  
        str = pClientItem->m_currentCommand.Right( pClientItem->m_currentCommand.Length() - 8 );  // SETMASK
    }
    wxStringTokenizer tkz( str, _(",") );

    // Get priority
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        pClientItem->m_filterVSCP.mask_priority = vscp_readStringValue( str );
    }
    else {
        ns_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
        return;	
    }

    // Get Class
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        pClientItem->m_filterVSCP.mask_class = vscp_readStringValue( str );
    }
    else {
        ns_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
        return;	
    }


    // Get Type
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        pClientItem->m_filterVSCP.mask_type = vscp_readStringValue( str );
    }
    else {
        ns_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
        return;	
    }

    // Get GUID
    if ( tkz.HasMoreTokens() ) {
        str = tkz.GetNextToken();
        vscp_getGuidFromStringToArray( pClientItem->m_filterVSCP.mask_GUID, str );
    }
    else {
        ns_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
        return;	
    }

    ns_send( conn,  MSG_OK, strlen ( MSG_OK ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleClientUser
//

void VSCPClientThread::handleClientUser ( struct ns_connection *conn, CControlObject *pCtrlObject )
{
	CClientItem *pClientItem = (CClientItem *)conn->user_data;

    if ( pClientItem->m_bAuthorized ) {
        ns_send( conn,  MSG_OK, strlen ( MSG_OK ) );
        return;
    }

    pClientItem->m_UserName = pClientItem->m_currentCommand.Right( pClientItem->m_currentCommand.Length() - 4 );
    pClientItem->m_UserName.Trim();			// Trim right side	
    pClientItem->m_UserName.Trim( false ); 	// Trim left
    if ( pClientItem->m_UserName.IsEmpty() ) {
        ns_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
        return;
    }

    ns_send( conn,  MSG_USENAME_OK, strlen ( MSG_USENAME_OK ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleClientPassword
//

bool VSCPClientThread::handleClientPassword ( struct ns_connection *conn, CControlObject *pCtrlObject )
{
	CClientItem *pClientItem = (CClientItem *)conn->user_data;

    if ( pClientItem->m_bAuthorized ) {
        ns_send( conn,  MSG_OK, strlen ( MSG_OK ) );
        return true;
    }

    // Must have username before password can be entered.
    if ( 0 == pClientItem->m_UserName.Length() ) {
        ns_send( conn,  MSG_NEED_USERNAME, strlen ( MSG_NEED_USERNAME ) );
        return true;
    }

    wxString strPassword = pClientItem->m_currentCommand.Right( pClientItem->m_currentCommand.Length() - 4 );
    strPassword.Trim();             // Trim right side	
    strPassword.Trim( false );      // Trim left
    if ( strPassword.IsEmpty() ) {
		pClientItem->m_UserName = _("");
        ns_send( conn,  MSG_PARAMETER_ERROR, strlen( MSG_PARAMETER_ERROR ) );
        return false;
    }

    // Calculate MD5 for username:autdomain:password
    char buf[2148];
    memset( buf, 0, sizeof( buf ) );
	strncpy( buf, (const char *)pClientItem->m_UserName.mbc_str(), pClientItem->m_UserName.Length() );
	strncat( buf, ":", sizeof( buf ) );
	strncat( buf, (const char *)pCtrlObject->m_authDomain.mbc_str(), pCtrlObject->m_authDomain.Length() );
	strncat( buf, ":", sizeof( buf ) );
    strncat( (char *)buf, strPassword.mb_str(), strPassword.Length() );
	//wxString wrk = pClientItem->m_UserName + _(":");
	//wrk += pCtrlObject->m_authDomain + _(":");
	//wrk += strPassword;
	//strcpy( buf, (const char *)wrk.c_str() );
	
    Cmd5 md5 ( (unsigned char *)buf );
    if ( NULL == md5.getDigest() ) return false; 
    wxString md5Password = wxString( md5.getDigest(), wxConvUTF8 );
    m_pCtrlObject->m_mutexUserList.Lock();
#if  0 
    ::wxLogDebug( _("Username: ") + m_UserName );
    ::wxLogDebug( _("Password: ") + strPassword );
    ::wxLogDebug( _("MD5 of Password: ") + md5Password );
#endif
    pClientItem->m_pUserItem = m_pCtrlObject->m_userList.checkUser( pClientItem->m_UserName, md5Password );
    m_pCtrlObject->m_mutexUserList.Unlock();

    if ( NULL == pClientItem->m_pUserItem ) {
#if wxMAJOR_VERSION >= 3
		wxLogDebug( _("Password/Username failure.") );
#else		
        ::wxLogDebug( _("Password/Username failure.") );
#endif		
        wxString strErr = 
			wxString::Format(_("[TCP/IP Client] User [%s][%s] not allowed to connect.\n"), 	
			(const char *)pClientItem->m_UserName.mb_str(), (const char *)strPassword.mb_str() );

		pCtrlObject->logMsg ( strErr, DAEMON_LOGMSG_WARNING, DAEMON_LOGTYPE_SECURITY );
		ns_send( conn,  MSG_PASSWORD_ERROR, strlen ( MSG_PASSWORD_ERROR ) );
        return false;
    }

	// Get remte address
	struct sockaddr_in cli_addr;
	socklen_t clilen = 0;    
    clilen = sizeof (cli_addr);
	int ret = getpeername( conn->sock, (struct sockaddr *)&cli_addr, &clilen);
    struct sockaddr_in *s = (struct sockaddr_in *)&cli_addr;
    wxString remoteaddr = wxString::FromAscii( inet_ntoa( cli_addr.sin_addr ) );

    // Check if this user is allowed to connect from this location
    m_pCtrlObject->m_mutexUserList.Lock();
    bool bValidHost = 
            pClientItem->m_pUserItem->isAllowedToConnect( remoteaddr );
    m_pCtrlObject->m_mutexUserList.Unlock();

    if ( !bValidHost ) {
		wxString strErr = wxString::Format(_("[TCP/IP Client] Host [%s] not allowed to connect.\n"), 
			(const char *)remoteaddr.wc_str() );
		
		pCtrlObject->logMsg ( strErr, DAEMON_LOGMSG_WARNING, DAEMON_LOGTYPE_SECURITY );
        ns_send( conn,  MSG_INVALID_REMOTE_ERROR, strlen ( MSG_INVALID_REMOTE_ERROR ) );
        return false;
    }

    // Copy in the user filter
    memcpy( &pClientItem->m_filterVSCP, 
				&pClientItem->m_pUserItem->m_filterVSCP, 
				sizeof( vscpEventFilter ) );

    wxString strErr = 
        wxString::Format( _("[TCP/IP Client] Host [%s] User [%s] allowed to connect.\n"), 
							(const char *)remoteaddr.wc_str(), 
                            (const char *)pClientItem->m_UserName.wc_str() );

	pCtrlObject->logMsg ( strErr, DAEMON_LOGMSG_WARNING, DAEMON_LOGTYPE_SECURITY );

    pClientItem->m_bAuthorized = true;
    ns_send( conn,  MSG_OK, strlen ( MSG_OK ) );

    return true;

}


///////////////////////////////////////////////////////////////////////////////
// handleClientRcvLoop
//

void VSCPClientThread::handleClientRcvLoop( struct ns_connection *conn, CControlObject *pCtrlObject  )
{
	CClientItem *pClientItem = (CClientItem *)conn->user_data;

    ns_send( conn,  MSG_RECEIVE_LOOP, strlen ( MSG_RECEIVE_LOOP ) );
	conn->flags |= NSF_USER_1;	// Mark socket as being in receive loop
/*
    // Loop until the connection is lost
    while ( !TestDestroy() && !m_bQuit && (conn->flags & NSF_USER_1 ) ) {
		
        if ( m_pClientSocket->IsDisconnected() ) m_bQuit = true;

        // Wait for event
        if ( wxSEMA_TIMEOUT == 
            pClientItem->m_semClientInputQueue.WaitTimeout( 1000 ) ) {
                ns_send( conn, "+OK\r\n", 5 );
                continue;
        }

        // Send one event if someting in the queue
        sendOneEventFromQueue( false );

    } // While
*/
    return;
}

///////////////////////////////////////////////////////////////////////////////
// handleClientHelp
//

void VSCPClientThread::handleClientHelp( struct ns_connection *conn, CControlObject *pCtrlObject )
{
	CClientItem *pClientItem = (CClientItem *)conn->user_data;
	
	wxString strcmd = pClientItem->m_currentCommandUC.Right( pClientItem->m_currentCommandUC.Length()-4 );
	strcmd.Trim();
	strcmd.Trim(false);

	if ( _("") == strcmd ) {

		wxString str = _("Help for the VSCP tcp/ip interface\r\n");
				str += _("====================================================================\r\n");
				str += _("To get more information about a specific command issue 'HELP comman'\r\n");
				str += _("+               - Repeat last command.\r\n");
				str += _("NOOP            - No operation. Does nothing.\r\n");
				str += _("QUIT            - Close the connection.\r\n");
				str += _("USER 'username' - Username for login. \r\n");
				str += _("PASS 'password' - Password for login.  \r\n");
				str += _("SEND 'event'    - Send an event.   \r\n");
                str += _("RETR 'count'    - Rtrive n events from input queue.   \r\n");
				str += _("RCVLOOP         - Will retrieve events in an endless loop until the connection is closed by the client or QUITLOOP is sent.\r\n");
				str += _("QUITLOOP        - Terminate RCVLOOP.\r\n");
				str += _("CDTA/CHKDATA    - Check if there is data in the input queue.\r\n");
				str += _("CLRA/CLRALL     - Clear input queue.\r\n");
				str += _("STAT            - Get statistical information.\r\n");
				str += _("INFO            - Get status info.\r\n");
				str += _("CHID            - Get channel id.\r\n");
				str += _("SGID/SETGUID    - Set GUID for channel.\r\n");
				str += _("GGID/GETGUID    - Get GUID for channel.\r\n");
				str += _("VERS/VERSION    - Get VSCP daemon version.\r\n");
				str += _("SFLT/SETFILTER  - Set incoming event filter.\r\n");
				str += _("SMSK/SETMASK    - Set incoming event mask.\r\n");
				str += _("HELP [command]  - This command.\r\n");
				str += _("TEST            - Do test sequence. Only used for debugging.\r\n");
				str += _("SHUTDOWN        - Shutdown the daemon.\r\n");
				str += _("RESTART         - Restart the daemon.\r\n");
				str += _("DRIVER          - Driver manipulation.\r\n");
				str += _("FILE            - File handling.\r\n");
				str += _("UDP             - UDP.\r\n");
				str += _("REMOTE          - User manipulation.\r\n");
				str += _("INTERFACE       - Interface manipulation. \r\n");
				str += _("DM              - Decision Matrix manipulation.\r\n");
				str += _("VARIABLE        - Variable handling. \r\n");
				int ttt = str.Length();
				ns_send( conn, (const char *)str.mbc_str(), str.Length() );
	}
	else if ( _("+") == strcmd ) {
		wxString str = _("'+' repeats the last given command.\r\n");
		ns_send( conn, (const char *)str.mbc_str(), str.Length() );
	}
	else if ( _("NOOP") == strcmd ) {
		wxString str = _("'NOOP' Does absolutly nothing but giving a success in return.\r\n");
		ns_send( conn, (const char *)str.mbc_str(), str.Length() );
	}
	else if ( _("QUIT") == strcmd ) {
		wxString str = _("'QUIT' Quit a session with the VSCP daemon and closes the connection.\r\n");
		ns_send( conn, (const char *)str.mbc_str(), str.Length() );
	}
	else if ( _("USER") == strcmd ) {
		wxString str = _("'USER' Used to login to the system together with PASS. Connection will be closed if bad credentials are given.\r\n");
		ns_send( conn, (const char *)str.mbc_str(), str.Length() );
	}
	else if ( _("PASS") == strcmd ) {
		wxString str = _("'PASS' Used to login to the system together with USER. Connection will be closed if bad credentials are given.\r\n");
		ns_send( conn, (const char *)str.mbc_str(), str.Length() );
	}
	else if ( _("QUIT") == strcmd ) {
		wxString str = _("'QUIT' Quit a session with the VSCP daemon and closes the connection.\r\n");
		ns_send( conn, (const char *)str.mbc_str(), str.Length() );
	}
	else if ( _("SEND") == strcmd ) {
		wxString str = _("'SEND event'.\r\nThe event is given as 'head,class,type,obid,time-stamp,GUID,data1,data2,data3....' \r\n");
		str += _("Normally set 'head' and 'obid' to zero. \r\nIf timestamp is set to zero it will be set by the server. \r\nIf GUID is given as '-' ");
		str += _("the GUID of the interface will be used. \r\nThe GUID should be given on the form MSB-byte:MSB-byte-1:MSB-byte-2. \r\n");
		ns_send( conn, (const char *)str.mbc_str(), str.Length() );
	}
	else if ( _("RETR") == strcmd ) {
		wxString str = _("'RETR count' - Retrieve one (if no argument) or 'count' event(s). ");
		str += _("Events are retrived on the form head,class,type,obid,time-stamp,GUID,data0,data1,data2,...........\r\n");
		ns_send( conn, (const char *)str.mbc_str(), str.Length() );
	}
	else if ( _("RCVLOOP") == strcmd ) {
		wxString str = _("'RCVLOOP' - Enter the receive loop and receive events continously or until ");
		str += _("terminated with 'QUITLOOP'. Events are retrived on the form head,class,type,obid,time-stamp,GUID,data0,data1,data2,...........\r\n");
		ns_send( conn, (const char *)str.mbc_str(), str.Length() );
	}
	else if ( _("QUITLOOP") == strcmd ) {
		wxString str = _("'QUITLOOP' - End 'RCVLOOP' event receives.\r\n");
		ns_send( conn, (const char *)str.mbc_str(), str.Length() );
	}
	else if ( ( _("CDTA") == strcmd ) || ( ( _("CHKDATA") == strcmd ) ) ) {
		wxString str = _("'CDTA' or 'CHKDATA' - Check if there is events in the input queue.\r\n");
		ns_send( conn, (const char *)str.mbc_str(), str.Length() );
	}
	else if ( ( _("CLRA") == strcmd ) || ( ( _("CLRALL") == strcmd ) ) ) {
		wxString str = _("'CLRA' or 'CLRALL' - Clear input queue.\r\n");
		ns_send( conn, (const char *)str.mbc_str(), str.Length() );
	}
	else if ( _("STAT") == strcmd ) {
		wxString str = _("'STAT' - Get statistical information.\r\n");
		ns_send( conn, (const char *)str.mbc_str(), str.Length() );
	}
	else if ( _("INFO") == strcmd ) {
		wxString str = _("'INFO' - Get status information.\r\n");
		ns_send( conn, (const char *)str.mbc_str(), str.Length() );
	}
	else if ( ( _("CHID") == strcmd ) || ( ( _("GETCHID") == strcmd ) ) ) {
		wxString str = _("'CHID' or 'GETCHID' - Get channel id.\r\n");
		ns_send( conn, (const char *)str.mbc_str(), str.Length() );
	}
	else if ( ( _("SGID") == strcmd ) || ( ( _("SETGUID") == strcmd ) ) ) {
		wxString str = _("'SGID' or 'SETGUID' - Set GUID for channel.\r\n");
		ns_send( conn, (const char *)str.mbc_str(), str.Length() );
	}
	else if ( ( _("GGID") == strcmd ) || ( ( _("GETGUID") == strcmd ) ) ) {
		wxString str = _("'GGID' or 'GETGUID' - Get GUID for channel.\r\n");
		ns_send( conn, (const char *)str.mbc_str(), str.Length() );
	}
	else if ( ( _("VERS") == strcmd ) || ( ( _("VERSION") == strcmd ) ) ) {
		wxString str = _("'VERS' or 'VERSION' - Get version of VSCP daemon.\r\n");
		ns_send( conn, (const char *)str.mbc_str(), str.Length() );
	}
	else if ( ( _("SFLT") == strcmd ) || ( ( _("SETFILTER") == strcmd ) ) ) {
		wxString str = _("'SFLT' or 'SETFILTER' - Set filter for channel. ");
		str += _("The format is 'filter-priority, filter-class, filter-type, filter-GUID' \r\n");
		str += _("Example:  \r\nSETFILTER 1,0x0000,0x0006,ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00\r\n");
		ns_send( conn, (const char *)str.mbc_str(), str.Length() );
	}
	else if ( ( _("SMSK") == strcmd ) || ( ( _("SETMASK") == strcmd ) ) ) {
		wxString str = _("'SMSK' or 'SETMASK' - Set mask for channel. ");
		str += _("The format is 'mask-priority, mask-class, mask-type, mask-GUID' \r\n");
		str += _("Example:  \r\nSETMASK 0x0f,0xffff,0x00ff,ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00 \r\n");
		ns_send( conn, (const char *)str.mbc_str(), str.Length() );
	}
	else if ( _("HELP") == strcmd ) {
		wxString str = _("'HELP [command]' This command. Gives help about available commands and the usage.\r\n");
		ns_send( conn, (const char *)str.mbc_str(), str.Length() );
	}
	else if ( _("TEST") == strcmd ) {
		wxString str = _("'TEST [sequency]' Test command for debugging.\r\n");
		ns_send( conn, (const char *)str.mbc_str(), str.Length() );
	}
	else if ( _("SHUTDOWN") == strcmd ) {
		wxString str = _("'SHUTDOWN' Shutdown the daemon.\r\n");
		ns_send( conn, (const char *)str.mbc_str(), str.Length() );
	}
	else if ( _("RESTART") == strcmd ) {
		wxString str = _("'RESTART' Restart the daemon.\r\n");
		ns_send( conn, (const char *)str.mbc_str(), str.Length() );
	}
	else if ( _("DRIVER") == strcmd ) {
		wxString str = _("'DRIVER' Handle (load/unload/update/start/stop) Level I/Level II drivers.\r\n");
		str += _("'DRIVER install package' .\r\n");
		str += _("'DRIVER uninstall package' .\r\n");
		str += _("'DRIVER upgrade package' .\r\n");
		str += _("'DRIVER start package' .\r\n");
		str += _("'DRIVER stop package' .\r\n");
		str += _("'DRIVER reload package' .\r\n");
		ns_send( conn, (const char *)str.mbc_str(), str.Length() );
	}
	else if ( _("FILE") == strcmd ) {
		wxString str = _("'FILE' Handle daemon files.\r\n");
		str += _("'FILE dir'.\r\n");
		str += _("'FILE copy'.\r\n");
		str += _("'FILE move'.\r\n");
		str += _("'FILE delete'.\r\n");
		str += _("'FILE list'.\r\n");
		ns_send( conn, (const char *)str.mbc_str(), str.Length() );
	}
	else if ( _("UDP") == strcmd ) {
		wxString str = _("'UDP' Handle UDP interface.\r\n");
		str += _("'UDP enable'.\r\n");
		str += _("'UDP disable' .\r\n");
		ns_send( conn, (const char *)str.mbc_str(), str.Length() );
	}
	else if ( _("REMOTE") == strcmd ) {
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
		ns_send( conn, (const char *)str.mbc_str(), str.Length() );
	}
	else if ( _("INTERFACE") == strcmd ) {
		wxString str = _("'INTERFACE' Handle interfaces on the daemon.\r\n");
		str += _("'INTERFACE list'.\r\n");
		str += _("'INTERFACE close'.\r\n");
		ns_send( conn, (const char *)str.mbc_str(), str.Length() );
	}
	else if ( _("DM") == strcmd ) {
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
		ns_send( conn, (const char *)str.mbc_str(), str.Length() );
	}
	else if ( _("VARIABLE") == strcmd ) {
		wxString str = _("'VARIABLE' Handle variables on the daemon.\r\n");
		str += _("'VARIABLE list'.\r\n");
		str += _("'VARIABLE write'.\r\n");
		str += _("'VARIABLE read'.\r\n");
		str += _("'VARIABLE reset'.\r\n");
		str += _("'VARIABLE readreset'.\r\n");
		str += _("'VARIABLE remove'.\r\n");
		str += _("'VARIABLE readremove'.\r\n");
		str += _("'VARIABLE length'.\r\n");
		str += _("'VARIABLE save'.\r\n");
		ns_send( conn, (const char *)str.mbc_str(), str.Length() );
	}

	ns_send( conn, MSG_OK, strlen(MSG_OK) );
	return;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientTest
//

void VSCPClientThread::handleClientTest ( struct ns_connection *conn, CControlObject *pCtrlObject )
{
	CClientItem *pClientItem = (CClientItem *)conn->user_data;

	ns_send( conn, MSG_OK, strlen ( MSG_OK ) );
	return;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientRestart
//

void VSCPClientThread::handleClientRestart ( struct ns_connection *conn, CControlObject *pCtrlObject )
{
	CClientItem *pClientItem = (CClientItem *)conn->user_data;

	ns_send( conn, MSG_OK, strlen ( MSG_OK ) );
	return;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientShutdown
//

void VSCPClientThread::handleClientShutdown ( struct ns_connection *conn, CControlObject *pCtrlObject )
{
	CClientItem *pClientItem = (CClientItem *)conn->user_data;

    if ( !pClientItem->m_bAuthorized ) {
        ns_send( conn,  MSG_OK, strlen ( MSG_OK ) );
    }

	ns_send( conn,  MSG_GOODBY, strlen ( MSG_GOODBY ) );
	conn->flags |= NSF_CLOSE_IMMEDIATELY;
    //m_pCtrlObject->m_bQuit = true;
    //m_bRun = false;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientRemote
//

void VSCPClientThread::handleClientRemote( struct ns_connection *conn, CControlObject *pCtrlObject )
{
	CClientItem *pClientItem = (CClientItem *)conn->user_data;

	return;
}


///////////////////////////////////////////////////////////////////////////////
// handleClientInterface
//
// list 	 	List interfaces.
// unique 	Aquire selected interface uniquely. Full format is INTERFACE UNIQUE id
// normal 	Normal access to interfaces. Full format is INTERFACE NORMAL id
// close		Close interfaces. Full format is INTERFACE CLOSE id

void VSCPClientThread::handleClientInterface( struct ns_connection *conn, CControlObject *pCtrlObject )
{
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    m_pCtrlObject->logMsg ( pClientItem->m_currentCommandUC, DAEMON_LOGMSG_INFO );

    if ( wxNOT_FOUND != pClientItem->m_currentCommandUC.Find ( _( "LIST" ) ) )	{
        handleClientInterface_List( conn, pCtrlObject );
    }
    else if ( wxNOT_FOUND != pClientItem->m_currentCommandUC.Find ( _( "UNIQUE " ) ) )	{
        pClientItem->m_currentCommandUC = pClientItem->m_currentCommandUC.Right( pClientItem->m_currentCommandUC.Length()-7 );            // Remove "UNIQUE "
        handleClientInterface_Unique( conn, pCtrlObject );
    }
    else if ( wxNOT_FOUND != pClientItem->m_currentCommandUC.Find ( _( "NORMAL" ) ) )	{
        handleClientInterface_Normal( conn, pCtrlObject );
    }
    else if ( wxNOT_FOUND != pClientItem->m_currentCommandUC.Find ( _( "CLOSE" ) ) )	{
        handleClientInterface_Close( conn, pCtrlObject );
    }
}

///////////////////////////////////////////////////////////////////////////////
// handleClientInterface_List
//

void VSCPClientThread::handleClientInterface_List( struct ns_connection *conn, CControlObject *pCtrlObject )
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

            ns_send( conn,  strBuf.mb_str(),
									strlen( strBuf.mb_str() ) );

    }

    ns_send( conn, MSG_OK, strlen ( MSG_OK ) );

    m_pCtrlObject->m_wxClientMutex.Unlock();
}

///////////////////////////////////////////////////////////////////////////////
// handleClientInterface_Unique
//

void VSCPClientThread::handleClientInterface_Unique( struct ns_connection *conn, CControlObject *pCtrlObject )
{
    unsigned char ifGUID[ 16 ];
    memset( ifGUID, 0, 16 );

    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    // Get GUID
    pClientItem->m_currentCommandUC.Trim( false );
    vscp_getGuidFromStringToArray( ifGUID, pClientItem->m_currentCommandUC );

    // Add the client to the Client List
    // TODO

    ns_send( conn, MSG_INTERFACE_NOT_FOUND, strlen( MSG_INTERFACE_NOT_FOUND ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleClientInterface_Normal
//

void VSCPClientThread::handleClientInterface_Normal( struct ns_connection *conn, CControlObject *pCtrlObject )
{
    // TODO
}

///////////////////////////////////////////////////////////////////////////////
// handleClientInterface_Close
//

void VSCPClientThread::handleClientInterface_Close( struct ns_connection *conn, CControlObject *pCtrlObject )
{
    // TODO
}


///////////////////////////////////////////////////////////////////////////////
// handleClientUdp
//

void VSCPClientThread::handleClientUdp( struct ns_connection *conn, CControlObject *pCtrlObject )
{
    // TODO
}


///////////////////////////////////////////////////////////////////////////////
// handleClientFile
//

void VSCPClientThread::handleClientFile( struct ns_connection *conn, CControlObject *pCtrlObject )
{
    // TODO
}


///////////////////////////////////////////////////////////////////////////////
// handleClientVariable
//

void VSCPClientThread::handleClientVariable( struct ns_connection *conn, CControlObject *pCtrlObject )
{
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    m_pCtrlObject->logMsg ( pClientItem->m_currentCommandUC + _("\r\b"), DAEMON_LOGMSG_INFO );

    pClientItem->m_currentCommandUC = pClientItem->m_currentCommandUC.Right( pClientItem->m_currentCommandUC.Length()-9 );    // remove "VARIABLE "
    pClientItem->m_currentCommand = pClientItem->m_currentCommand.Right( pClientItem->m_currentCommand.Length()-9 );          // remove "VARIABLE "
    pClientItem->m_currentCommandUC.Trim( false );
    pClientItem->m_currentCommand.Trim( false );

    if ( 0 == pClientItem->m_currentCommandUC.Find ( wxT( "LIST " ) ) )	{
        handleVariable_List( conn, pCtrlObject );
    }
    // Hack to handle "variable list"
    else if ( 0 == pClientItem->m_currentCommandUC.Find ( wxT( "LIST" ) ) )	{
        pClientItem->m_currentCommandUC += _(" ");
        pClientItem->m_currentCommand += _(" ");
        handleVariable_List( conn, pCtrlObject );
    }
    else if ( 0 == pClientItem->m_currentCommandUC.Find ( wxT( "WRITE " ) ) )	{
        handleVariable_Write( conn, pCtrlObject );
    }
    else if ( 0 == pClientItem->m_currentCommandUC.Find ( wxT( "READ " ) ) )	{
        handleVariable_Read( conn, pCtrlObject );
    }
    else if ( 0 == pClientItem->m_currentCommandUC.Find ( wxT( "READRESET " ) ) )	{
        handleVariable_ReadReset( conn, pCtrlObject );
    }
    else if ( 0 == pClientItem->m_currentCommandUC.Find ( wxT( "RESET" ) ) )	{
        handleVariable_Reset( conn, pCtrlObject );
    }
    else if ( 0 == pClientItem->m_currentCommandUC.Find ( wxT( "REMOVE " ) ) )	{
        handleVariable_Remove( conn, pCtrlObject );
    }
    else if ( 0 == pClientItem->m_currentCommandUC.Find ( wxT( "READREMOVE " ) ) )	{
        handleVariable_ReadRemove( conn, pCtrlObject );
    }
    else if ( 0 == pClientItem->m_currentCommandUC.Find ( wxT( "LENGTH " ) ) )	{
        handleVariable_Length( conn, pCtrlObject );
    }
    else if ( 0 == pClientItem->m_currentCommandUC.Find ( wxT( "LOAD" ) ) )	{
        handleVariable_Load( conn, pCtrlObject );
    }
    else if ( 0 == pClientItem->m_currentCommandUC.Find ( wxT( "SAVE" ) ) )	{
        handleVariable_Save( conn, pCtrlObject );
    }
    else {
        ns_send( conn, MSG_UNKNOWN_COMMAND, strlen( MSG_UNKNOWN_COMMAND ) );
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

void VSCPClientThread::handleVariable_List( struct ns_connection *conn, CControlObject *pCtrlObject )
{
    CVSCPVariable *pVariable;
    wxString str;
    wxString strWork;
    listVscpVariable::iterator it;
	static int l;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    pClientItem->m_currentCommandUC = pClientItem->m_currentCommandUC.Right( pClientItem->m_currentCommandUC.Length()-5 );    // remove "LIST "
    pClientItem->m_currentCommand = pClientItem->m_currentCommand.Right( pClientItem->m_currentCommand.Length()-5 );          // remove "LIST "
    pClientItem->m_currentCommandUC.Trim();
    pClientItem->m_currentCommandUC.Trim( false );
    pClientItem->m_currentCommand.Trim();
    pClientItem->m_currentCommand.Trim( false );

    // if "variable list" we add "all"
    if ( 0 == pClientItem->m_currentCommandUC.Length() ) {
        pClientItem->m_currentCommandUC +=  _("ALL");
        pClientItem->m_currentCommand += _("all");
    }

    // If "*" wildcard we add "all"
    if ( ( 1 == pClientItem->m_currentCommand.Length() ) && 
        ( 0 == pClientItem->m_currentCommandUC.Find ( _( "*" ) ) ) ) {
            pClientItem->m_currentCommandUC += _("ALL");
            pClientItem->m_currentCommand += _("all");	
    }

    if ( ( wxNOT_FOUND != pClientItem->m_currentCommandUC.Find ( _( "ALL" ) ) ) )	{

        m_pCtrlObject->m_variableMutex.Lock();

        for( it = m_pCtrlObject->m_VSCP_Variables.m_listVariable.begin(); 
            it != m_pCtrlObject->m_VSCP_Variables.m_listVariable.end(); 
            ++it ) {

                if ( NULL == ( pVariable = *it ) ) continue;

                str = pVariable->getName();
                str += _(";");
                strWork.Printf( _("%d"), pVariable->getType() ) ;
                str += strWork;
                if ( pVariable->isPersistent() ) {
                    str += _(";true;");
                }
                else {
                    str += _(";false;");
                }
				
                pVariable->writeValueToString( strWork );
                str += strWork;
                str += _("\r\n");

				l = str.Length();
                ns_send( conn,  str.mb_str(), str.Length() );
				//l = m_pClientSocket->LastCount();
				l = 1;
        }

        m_pCtrlObject->m_variableMutex.Unlock();

    }
    else {

        // variables (or wildcards) to list are separated
        // with spaces
        wxStringTokenizer tkz( pClientItem->m_currentCommandUC, _(" ") );

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
                                    str += _(";");
                                    strWork.Printf( _("%d"), pVariable->getType() ) ;
                                    str += strWork;
                                    if ( pVariable->isPersistent() ) {
                                        str += _(";true;");
                                    }
                                    else {
                                        str += _(";false;");
                                    }
                                    pVariable->writeValueToString( strWork );
                                    str += strWork;
                                    str += _("\r\n");

                                    ns_send( conn,  str.mb_str(), str.Length() );

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
                            str += _(";");
                            strWork.Printf( _("%d"), pVariable->getType() ) ;
                            str += strWork;
                            if ( pVariable->isPersistent() ) {
                                str += _(";true;");
                            }
                            else {
                                str += _(";false;");
                            }
                            pVariable->writeValueToString( strWork );
                            str += strWork;
                            str += _("\r\n");

                            ns_send( conn, str.mb_str(), str.Length() );

                        }

                }

                m_pCtrlObject->m_variableMutex.Unlock();

            }

        }

    }

    ns_send( conn,  MSG_OK, strlen ( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_Write
//

void VSCPClientThread::handleVariable_Write( struct ns_connection *conn, CControlObject *pCtrlObject )
{
    wxString strName;
    wxString strValue;
    //int type = VSCP_DAEMON_VARIABLE_CODE_STRING;
    wxString strType = _("STRING");   
    bool bPersistence = false;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    m_pCtrlObject->logMsg( pClientItem->m_currentCommandUC, DAEMON_LOGMSG_INFO );

    pClientItem->m_currentCommand = pClientItem->m_currentCommand.Right( pClientItem->m_currentCommand.Length() - 6 ); // remove "WRITE "
    pClientItem->m_currentCommand.Trim( false );
    pClientItem->m_currentCommand.Trim( true );

    wxStringTokenizer tkz( pClientItem->m_currentCommand, _(";") );

    // Get name of variable
    if ( tkz.HasMoreTokens() ) {
        strName = tkz.GetNextToken();
        strName.MakeUpper();
    }
    else {
        ns_send( conn, MSG_PARAMETER_ERROR, strlen( MSG_PARAMETER_ERROR ) );
        return;	
    }

    // type - string default. can be numerical r string
    if ( tkz.HasMoreTokens() ) {
        strType = tkz.GetNextToken();
        strType.Trim();
        strType.Trim( false );
        strType.MakeUpper();
    }
    else {
        ns_send( conn, MSG_PARAMETER_ERROR, strlen( MSG_PARAMETER_ERROR ) );
        return;	
    }

    // persistence - false default
    if ( tkz.HasMoreTokens() ) {
        wxString str = tkz.GetNextToken();
        str = str.Upper();
        if ( wxNOT_FOUND != str.Find( _("TRUE") ) ) {
            bPersistence = true;		
        }
    }
    else {
        ns_send( conn, MSG_PARAMETER_ERROR, strlen( MSG_PARAMETER_ERROR ) );
        return;	
    }

    // Value
    if ( tkz.HasMoreTokens() ) {
        strValue = tkz.GetString();
    }
    else {
        ns_send( conn, MSG_PARAMETER_ERROR, strlen( MSG_PARAMETER_ERROR ) );
        return;	
    }

    // If the variable exist change value
    // if not add it. This is handled in add.
    m_pCtrlObject->m_VSCP_Variables.addWithStringType( strName, strValue, strType, bPersistence );
	
	// Save decision matrix
    m_pCtrlObject->m_dm.save();

    ns_send( conn, MSG_OK, strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_Read
//

void VSCPClientThread::handleVariable_Read( struct ns_connection *conn, CControlObject *pCtrlObject, bool bOKResponse )
{
    wxString str;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    pClientItem->m_currentCommandUC = pClientItem->m_currentCommandUC.Right( pClientItem->m_currentCommandUC.Length() - 5 ); // remove "READ "
    pClientItem->m_currentCommandUC.Trim( false );
    pClientItem->m_currentCommandUC.Trim( true );

    CVSCPVariable * pVariable;

    if ( NULL == ( pVariable = m_pCtrlObject->m_VSCP_Variables.find( pClientItem->m_currentCommandUC ) ) ) {
        ns_send( conn, MSG_VARIABLE_NOT_DEFINED, strlen ( MSG_VARIABLE_NOT_DEFINED ) );
        return;
    }

    pVariable->writeValueToString( str );
    str += _("\r\n");
    ns_send( conn,  str.ToAscii(), strlen( str.ToAscii() ) );

    ns_send( conn, MSG_OK, strlen ( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_Reset
//

void VSCPClientThread::handleVariable_Reset( struct ns_connection *conn, CControlObject *pCtrlObject  )
{
    wxString str;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    pClientItem->m_currentCommandUC = pClientItem->m_currentCommandUC.Right( pClientItem->m_currentCommandUC.Length() - 10 ); // remove "READRESET "
    pClientItem->m_currentCommandUC.Trim( false );
    pClientItem->m_currentCommandUC.Trim( true );

    CVSCPVariable * pVariable;

    if ( NULL == ( pVariable = m_pCtrlObject->m_VSCP_Variables.find( pClientItem->m_currentCommandUC ) ) ) {

        // Must create the variable
        m_pCtrlObject->m_VSCP_Variables.add( pClientItem->m_currentCommandUC, 
                                                _(""), 
                                                VSCP_DAEMON_VARIABLE_CODE_STRING, 
                                                false );

        // Try again, Should be possible to find now
        if ( NULL == ( pVariable = m_pCtrlObject->m_VSCP_Variables.find( pClientItem->m_currentCommandUC ) ) ) {
            ns_send( conn, MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            return;
        }
    }

    pVariable->Reset();

    ns_send( conn, MSG_OK, strlen( MSG_OK ) );	
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_ReadReset
//

void VSCPClientThread::handleVariable_ReadReset( struct ns_connection *conn, CControlObject *pCtrlObject )
{
    wxString str;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    pClientItem->m_currentCommandUC = pClientItem->m_currentCommandUC.Right( pClientItem->m_currentCommandUC.Length() - 10 ); // remove "READRESET "
    pClientItem->m_currentCommandUC.Trim( false );
    pClientItem->m_currentCommandUC.Trim( true );

    CVSCPVariable * pVariable;

    if ( NULL == ( pVariable = m_pCtrlObject->m_VSCP_Variables.find( pClientItem->m_currentCommandUC ) ) ) {

        // Must create the variable
        m_pCtrlObject->m_VSCP_Variables.add( pClientItem->m_currentCommandUC, 
                                                _(""), 
                                                VSCP_DAEMON_VARIABLE_CODE_STRING, 
                                                false );
        // Try again, Should be possible to find now
        if ( NULL == ( pVariable = m_pCtrlObject->m_VSCP_Variables.find( pClientItem->m_currentCommandUC ) ) ) {
            ns_send( conn, MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            return;
        }
    }

    pVariable->writeValueToString( str );
    str += _("\r\n");
    ns_send( conn,  str.ToAscii(), str.Length() );

    handleVariable_Reset(conn, pCtrlObject);
}


///////////////////////////////////////////////////////////////////////////////
// handleVariable_Remove
//

void VSCPClientThread::handleVariable_Remove( struct ns_connection *conn, CControlObject *pCtrlObject )
{
    wxString str;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    pClientItem->m_currentCommandUC = pClientItem->m_currentCommandUC.Right( pClientItem->m_currentCommandUC.Length() - 7 ); // remove "REMOVE "
    pClientItem->m_currentCommandUC.Trim( false );
    pClientItem->m_currentCommandUC.Trim( true );

    m_pCtrlObject->m_variableMutex.Lock();
    m_pCtrlObject->m_VSCP_Variables.remove( pClientItem->m_currentCommandUC );
    m_pCtrlObject->m_variableMutex.Unlock();

    ns_send( conn, MSG_OK, strlen( MSG_OK ) );
}


///////////////////////////////////////////////////////////////////////////////
// handleVariable_ReadRemove
//

void VSCPClientThread::handleVariable_ReadRemove( struct ns_connection *conn, CControlObject *pCtrlObject )
{
    wxString str;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    pClientItem->m_currentCommandUC = pClientItem->m_currentCommandUC.Right( pClientItem->m_currentCommandUC.Length() - 11 ); // remove "READREMOVE "
    pClientItem->m_currentCommandUC.Trim( false );
    pClientItem->m_currentCommandUC.Trim( true );

    CVSCPVariable * pVariable;

    if ( NULL == ( pVariable = m_pCtrlObject->m_VSCP_Variables.find( pClientItem->m_currentCommandUC ) ) ) {

        // Must create the variable
        m_pCtrlObject->m_VSCP_Variables.add( pClientItem->m_currentCommandUC, 
                                                _(""), 
                                                VSCP_DAEMON_VARIABLE_CODE_STRING, 
                                                false );
        // Try again, Should be possible to find now
        if ( NULL == ( pVariable = m_pCtrlObject->m_VSCP_Variables.find( pClientItem->m_currentCommandUC ) ) ) {
            ns_send( conn, MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            return;
        }
    }

    pVariable->writeValueToString( str );
    str += _("\r\n");
    ns_send( conn,  str.ToAscii(), str.Length() );

    handleVariable_Remove( conn, pCtrlObject );
}

///////////////////////////////////////////////////////////////////////////////
// handleVariable_Length
//

void VSCPClientThread::handleVariable_Length( struct ns_connection *conn, CControlObject *pCtrlObject )
{
    wxString str;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    pClientItem->m_currentCommandUC = pClientItem->m_currentCommandUC.Right( pClientItem->m_currentCommandUC.Length() - 7 ); // remove "LENGTH "
    pClientItem->m_currentCommandUC.Trim( false );
    pClientItem->m_currentCommandUC.Trim( true );

    CVSCPVariable * pVariable;

    if ( NULL == ( pVariable = m_pCtrlObject->m_VSCP_Variables.find( pClientItem->m_currentCommandUC ) ) ) {

        // Must create the variable
        m_pCtrlObject->m_VSCP_Variables.add( pClientItem->m_currentCommandUC, 
                                                _(""), 
                                                VSCP_DAEMON_VARIABLE_CODE_STRING, 
                                                false );
        // Try again, Should be possible to find now
        if ( NULL == ( pVariable = m_pCtrlObject->m_VSCP_Variables.find( pClientItem->m_currentCommandUC ) ) ) {
            ns_send( conn, MSG_PARAMETER_ERROR, strlen( MSG_PARAMETER_ERROR ) );
            return;
        }
    }

    if ( pVariable->getType() ) {
        str.Printf( _("%d\r\n"), pVariable->m_strValue.Length()  );
        ns_send( conn,  str.mb_str(), str.Length() );
    }
    else {
        ns_send( conn, "0\r\n", strlen( "0\r\n" ) );	
    }

    ns_send( conn,  MSG_OK, strlen( MSG_OK ) );
}



///////////////////////////////////////////////////////////////////////////////
// handleVariable_Load
//

void VSCPClientThread::handleVariable_Load( struct ns_connection *conn, CControlObject *pCtrlObject )
{
    m_pCtrlObject->m_VSCP_Variables.load();

    ns_send( conn, MSG_OK, strlen( MSG_OK ) );
}


///////////////////////////////////////////////////////////////////////////////
// handleVariable_Save
//

void VSCPClientThread::handleVariable_Save( struct ns_connection *conn, CControlObject *pCtrlObject )
{
    wxString path;
    //wxStandardPaths stdPath;

    // Set the default variable configuration path
    path = wxStandardPaths::Get().GetConfigDir();
    path += _("/vscp/variable.xml");

    m_pCtrlObject->m_VSCP_Variables.save( path );

    ns_send( conn, MSG_OK, strlen( MSG_OK ) );
}




// ****************************************************************************



///////////////////////////////////////////////////////////////////////////////
// handleClientDm
//

void VSCPClientThread::handleClientDm( struct ns_connection *conn, CControlObject *pCtrlObject  )
{
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    m_pCtrlObject->logMsg ( pClientItem->m_currentCommandUC, DAEMON_LOGMSG_INFO );

    pClientItem->m_currentCommandUC = pClientItem->m_currentCommandUC.Right( pClientItem->m_currentCommandUC.Length()-3 );
    pClientItem->m_currentCommandUC.Trim( false );

    if ( wxNOT_FOUND != pClientItem->m_currentCommandUC.Find ( _( "ENABLE " ) ) )	{
        handleDM_Enable( conn, pCtrlObject );
    }
    else if ( wxNOT_FOUND != pClientItem->m_currentCommandUC.Find ( _( "DISABLE " ) ) )	{
        handleDM_Enable( conn, pCtrlObject );
    }
    else if ( wxNOT_FOUND != pClientItem->m_currentCommandUC.Find ( _( "LIST" ) ) )	{
        handleDM_List( conn, pCtrlObject );
    }
    else if ( wxNOT_FOUND != pClientItem->m_currentCommandUC.Find ( _( "ADD " ) ) )	{
        handleDM_Add( conn, pCtrlObject );
    }
    else if ( wxNOT_FOUND != pClientItem->m_currentCommandUC.Find ( _( "DELETE " ) ) )	{
        handleDM_Delete( conn, pCtrlObject );
    }
    else if ( wxNOT_FOUND != pClientItem->m_currentCommandUC.Find ( _( "RESET" ) ) )	{
        handleDM_Reset( conn, pCtrlObject );
    }
    else if ( wxNOT_FOUND != pClientItem->m_currentCommandUC.Find ( _( "TRIG " ) ) )	{
        handleDM_Trigger( conn, pCtrlObject );
    }
    else if ( wxNOT_FOUND != pClientItem->m_currentCommandUC.Find ( _( "CLRTRIG " ) ) )	{
        handleDM_ClearTriggerCount( conn, pCtrlObject );
    }
    else if ( wxNOT_FOUND != pClientItem->m_currentCommandUC.Find ( _( "CLRERR " ) ) )	{
        handleDM_ClearErrorCount( conn, pCtrlObject );
    }
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_Enable
//

void VSCPClientThread::handleDM_Enable( struct ns_connection *conn, CControlObject *pCtrlObject  )
{
    unsigned short pos;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    if ( wxNOT_FOUND != pClientItem->m_currentCommandUC.Find ( _( "ALL" ) ) )	{

        m_pCtrlObject->m_dm.m_mutexDM.Lock();

        DMLIST::iterator iter;
        for (iter = m_pCtrlObject->m_dm.m_DMList.begin(); iter != m_pCtrlObject->m_dm.m_DMList.end(); ++iter)
        {
            dmElement *pDMItem = *iter;
            pDMItem->enableRow();
        }	

        m_pCtrlObject->m_dm.m_mutexDM.Unlock();

    }
    else {

        // Get the position
        pos = 0;

        m_pCtrlObject->m_dm.m_mutexDM.Lock();

        if ( pos > ( m_pCtrlObject->m_dm.m_DMList.GetCount() - 1 ) ) {
            ns_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            m_pCtrlObject->m_dm.m_mutexDM.Unlock();
            return;
        }

        DMLIST::compatibility_iterator node = m_pCtrlObject->m_dm.m_DMList.Item( pos );
        ( node->GetData() )->enableRow();
        m_pCtrlObject->m_dm.m_mutexDM.Unlock();

    }

    ns_send( conn, MSG_OK, strlen( MSG_OK ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleDM_Disable
//

void VSCPClientThread::handleDM_Disable( struct ns_connection *conn, CControlObject *pCtrlObject  )
{
    unsigned short pos;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    if ( wxNOT_FOUND != pClientItem->m_currentCommandUC.Find ( _( "ALL" ) ) )	{

        m_pCtrlObject->m_dm.m_mutexDM.Lock();

        DMLIST::iterator iter;
        for (iter = m_pCtrlObject->m_dm.m_DMList.begin(); 
            iter != m_pCtrlObject->m_dm.m_DMList.end(); 
            ++iter )
        {
            dmElement *pDMItem = *iter;
            pDMItem->disableRow();
        }	
        m_pCtrlObject->m_dm.m_mutexDM.Unlock();

    }
    else {

        // Get the position
        wxStringTokenizer tkz( pClientItem->m_currentCommandUC, _(",") );
        while ( tkz.HasMoreTokens() ) {

            pos = vscp_readStringValue( tkz.GetNextToken() );

            m_pCtrlObject->m_dm.m_mutexDM.Lock();

            if ( pos > ( m_pCtrlObject->m_dm.m_DMList.GetCount() - 1 ) ) {
                ns_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
                m_pCtrlObject->m_dm.m_mutexDM.Unlock();
                return;
            }

            DMLIST::compatibility_iterator node = m_pCtrlObject->m_dm.m_DMList.Item( pos );
            ( node->GetData() )->disableRow();

            m_pCtrlObject->m_dm.m_mutexDM.Unlock();

        }

    }

    ns_send( conn, MSG_OK, strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_List
//

void VSCPClientThread::handleDM_List( struct ns_connection *conn, CControlObject *pCtrlObject  )
{
    // Valid commands at this point
    // dm list
    // dm list all
    // dm list *
    // dm list 1
    // dm list 1,2,3,4,98

    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    // Remove "LIST"
    pClientItem->m_currentCommandUC = pClientItem->m_currentCommandUC.Right( pClientItem->m_currentCommandUC.Length() - 4 );
    pClientItem->m_currentCommandUC.Trim( false );

    // if "list" add "all"
    if ( 0 == pClientItem->m_currentCommandUC.Length() ) {
        pClientItem->m_currentCommandUC = _("ALL");
    }

    if ( ( wxNOT_FOUND != pClientItem->m_currentCommandUC.Find ( _( "ALL" ) ) ) ||
            ( wxNOT_FOUND != pClientItem->m_currentCommandUC.Find ( _( "*" ) ) ) )	{
    
        m_pCtrlObject->m_dm.m_mutexDM.Lock();

        DMLIST::iterator iter;
        for (iter = m_pCtrlObject->m_dm.m_DMList.begin(); 
                iter != m_pCtrlObject->m_dm.m_DMList.end(); 
                ++iter) {

            dmElement *pDMItem = *iter;
            wxString strRow = pDMItem->getAsRealText();

            ns_send( conn,  strRow.mb_str(),
                                        strlen ( strRow.mb_str() ) );

        }	

        m_pCtrlObject->m_dm.m_mutexDM.Unlock();
    
    }
    else {

        // We have a list with specific rows  "list 1,8,9"
        //      first parse the argument to get the rows
        //WX_DEFINE_ARRAY_INT( int, ArrayOfSortedInts );
        wxArrayInt rowArray;
        wxStringTokenizer tok( pClientItem->m_currentCommandUC, _(",") );
        while ( tok.HasMoreTokens() ) {
            
            int n = vscp_readStringValue( tok.GetNextToken() );
            rowArray.Add( n );

        }

    }


    ns_send( conn, MSG_OK, strlen ( MSG_OK ) );

}

///////////////////////////////////////////////////////////////////////////////
// handleDM_Add
//

void VSCPClientThread::handleDM_Add( struct ns_connection *conn, CControlObject *pCtrlObject  )
{
    CClientItem *pClientItem = (CClientItem *)conn->user_data;
    dmElement *pDMItem = new dmElement;

    wxStringTokenizer tkz( pClientItem->m_currentCommandUC, _(",") );

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

    // control
    pDMItem->m_control = vscp_readStringValue( tkz.GetNextToken() );

    // action code
    pDMItem->m_action = vscp_readStringValue( tkz.GetNextToken() );

    // action parameters
    pDMItem->m_actionparam = tkz.GetNextToken();

    // add the DM row to the matrix
    m_pCtrlObject->m_dm.m_mutexDM.Lock();
    m_pCtrlObject->m_dm.addElement ( pDMItem );
    m_pCtrlObject->m_dm.m_mutexDM.Unlock();

    ns_send( conn, MSG_OK, strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_Delete
//

void VSCPClientThread::handleDM_Delete( struct ns_connection *conn, CControlObject *pCtrlObject  )
{
    unsigned short pos;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    if ( wxNOT_FOUND != pClientItem->m_currentCommandUC.Find ( _( "ALL" ) ) )	{

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
        wxStringTokenizer tkz( pClientItem->m_currentCommandUC, _(",") );
        while ( tkz.HasMoreTokens() ) {

            pos = vscp_readStringValue( tkz.GetNextToken() );

            m_pCtrlObject->m_dm.m_mutexDM.Lock();

            if ( pos > ( m_pCtrlObject->m_dm.m_DMList.GetCount() - 1 ) ) {
                ns_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
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

    ns_send( conn,  MSG_OK, strlen ( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_Reset
//

void VSCPClientThread::handleDM_Reset( struct ns_connection *conn, CControlObject *pCtrlObject  )
{
    m_pCtrlObject->stopDaemonWorkerThread();
    m_pCtrlObject->startDaemonWorkerThread();

    ns_send( conn, MSG_OK, strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_Trigger
//

void VSCPClientThread::handleDM_Trigger( struct ns_connection *conn, CControlObject *pCtrlObject )
{
    unsigned short pos;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    // Get the position
    wxStringTokenizer tkz( pClientItem->m_currentCommandUC, _(",") );
    while ( tkz.HasMoreTokens() ) {

        pos = vscp_readStringValue( tkz.GetNextToken() );

        m_pCtrlObject->m_dm.m_mutexDM.Lock();

        if ( pos > ( m_pCtrlObject->m_dm.m_DMList.GetCount() - 1 ) ) {
            ns_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
            m_pCtrlObject->m_dm.m_mutexDM.Unlock();
            return;
        }

        DMLIST::compatibility_iterator node = m_pCtrlObject->m_dm.m_DMList.Item( pos );
        dmElement *pDMItem = node->GetData();
        pDMItem->doAction( NULL );

        m_pCtrlObject->m_dm.m_mutexDM.Unlock();

    }

    ns_send( conn, MSG_OK, strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_ClearTriggerCount
//


void VSCPClientThread::handleDM_ClearTriggerCount( struct ns_connection *conn, CControlObject *pCtrlObject )
{
    unsigned short pos;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    if ( wxNOT_FOUND != pClientItem->m_currentCommandUC.Find ( _( "ALL" ) ) )	{

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
        wxStringTokenizer tkz( pClientItem->m_currentCommandUC, _(",") );
        while ( tkz.HasMoreTokens() ) {

            pos = vscp_readStringValue( tkz.GetNextToken() );

            m_pCtrlObject->m_dm.m_mutexDM.Lock();

            if ( pos > ( m_pCtrlObject->m_dm.m_DMList.GetCount() - 1 ) ) {
                ns_send( conn,  MSG_PARAMETER_ERROR,
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

    ns_send( conn, MSG_OK, strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleDM_ClearErrorCount
//


void VSCPClientThread::handleDM_ClearErrorCount( struct ns_connection *conn, CControlObject *pCtrlObject )
{
    unsigned short pos;
    CClientItem *pClientItem = (CClientItem *)conn->user_data;

    if ( wxNOT_FOUND != pClientItem->m_currentCommandUC.Find ( _( "ALL" ) ) )	{

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
        wxStringTokenizer tkz( pClientItem->m_currentCommandUC, _(",") );
        while ( tkz.HasMoreTokens() ) {

            pos = vscp_readStringValue( tkz.GetNextToken() );

            m_pCtrlObject->m_dm.m_mutexDM.Lock();

            if ( pos > ( m_pCtrlObject->m_dm.m_DMList.GetCount() - 1 ) ) {
                ns_send( conn,  MSG_PARAMETER_ERROR, strlen ( MSG_PARAMETER_ERROR ) );
                m_pCtrlObject->m_dm.m_mutexDM.Unlock();
                return;
            }

            DMLIST::compatibility_iterator node = m_pCtrlObject->m_dm.m_DMList.Item( pos );
            dmElement *pDMItem = node->GetData();
            pDMItem->m_errorCounter = 0;

            m_pCtrlObject->m_dm.m_mutexDM.Unlock();

        }

    }

    ns_send( conn, MSG_OK, strlen( MSG_OK ) );
}

///////////////////////////////////////////////////////////////////////////////
// handleClientList
//

void VSCPClientThread::handleClientList( struct ns_connection *conn, CControlObject *pCtrlObject  )
{
    ns_send( conn, MSG_OK, strlen ( MSG_OK ) );
}


///////////////////////////////////////////////////////////////////////////////
// handleClientDriver
//

void VSCPClientThread::handleClientDriver( struct ns_connection *conn, CControlObject *pCtrlObject  )
{

}

