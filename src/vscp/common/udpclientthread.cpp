// udpclientthread.cpp
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
// VSCPUDPClientThread
//
// This thread listens for connection on a TCP socket and starts a new thread
// to handle client requests
//

VSCPUDPClientThread::VSCPUDPClientThread()
: wxThread(wxTHREAD_JOINABLE)
{
    //OutputDebugString( "TCP ClientThread: Create");
    m_bQuit = false;
    m_pCtrlObject = NULL;
	m_pClientItem = NULL;
}


VSCPUDPClientThread::~VSCPUDPClientThread()
{
	;
}


///////////////////////////////////////////////////////////////////////////////
// Entry
//

void *VSCPUDPClientThread::Entry()
{
	// Check pointers
	if ( NULL == m_pCtrlObject ) return NULL;

	// We need to create a clientobject and add this object to the list
	m_pClientItem = new CClientItem;
	if ( NULL == m_pClientItem ) {
		m_pCtrlObject->logMsg ( _T ( "[UDP Client] Unable to allocate memory for client.\n" ), DAEMON_LOGMSG_ERROR );
		return NULL;
	}

	// This is now an active Client
	m_pClientItem->m_bOpen = true; 
	m_pClientItem->m_type =  CLIENT_ITEM_INTERFACE_TYPE_CLIENT_UDP;
	m_pClientItem->m_strDeviceName = _("Remote UDP Client Listner. Started at ");
	wxDateTime now = wxDateTime::Now(); 
	m_pClientItem->m_strDeviceName += now.FormatISODate();
	m_pClientItem->m_strDeviceName += _(" ");
	m_pClientItem->m_strDeviceName += now.FormatISOTime();

	// Add the client to the Client List
	m_pCtrlObject->m_wxClientMutex.Lock();
	m_pCtrlObject->addClient( m_pClientItem );
	m_pCtrlObject->m_wxClientMutex.Unlock();

	// Clear the filter (Allow everything )
	vscp_clearVSCPFilter( &m_pClientItem->m_filterVSCP );

	ns_mgr_init( &m_pCtrlObject->m_mgrTcpIpServer, this, VSCPClientThread::ev_handler );

	//const char *port1 = "udp://:9598";
	m_pCtrlObject->m_strUDPInterfaceAddress.Trim();
	m_pCtrlObject->m_strUDPInterfaceAddress.Trim( false );
	wxStringTokenizer tkz( m_pCtrlObject->m_strUDPInterfaceAddress, _(" ") );
	while ( tkz.HasMoreTokens() ) {
		
		wxString str = tkz.GetNextToken();
		str.Trim();
		str.Trim( false );
		if ( 0 == str.Length() ) continue;

		if ( wxNOT_FOUND  != str.Find(_("udp://") ) ) {
			str = _("udp://") + str;
		}

		// Bind to this interface
		ns_bind( &m_pCtrlObject->m_mgrTcpIpServer, str.mbc_str(), NULL ); //
	}

	m_pCtrlObject->logMsg(_T("UDP Client: Thread started.\n"), DAEMON_LOGMSG_INFO);

	while ( !TestDestroy() && !m_bQuit ) {
		ns_mgr_poll( &m_pCtrlObject->m_mgrTcpIpServer, 50 );
	}

	// release the server
	ns_mgr_free( &m_pCtrlObject->m_mgrTcpIpServer );

    m_pCtrlObject->logMsg( _T( "UDP ClientThread: Quit.\n" ), DAEMON_LOGMSG_INFO );

    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void VSCPUDPClientThread::OnExit()
{
	if ( NULL != m_pClientItem ) {
		// Add the client to the Client List
		m_pCtrlObject->m_wxClientMutex.Lock();
		m_pCtrlObject->removeClient( m_pClientItem );
		m_pCtrlObject->m_wxClientMutex.Unlock();
	}
}

///////////////////////////////////////////////////////////////////////////////
// ev_handler
//

void 
VSCPUDPClientThread::ev_handler(struct ns_connection *conn, enum ns_event ev, void *pUser) 
{
	struct iobuf *io = &conn->recv_iobuf;
	VSCPUDPClientThread *pUDPClientThread = (VSCPUDPClientThread *)conn->mgr->user_data;

	switch (ev) {

		case NS_CLOSE:
			break;

		case NS_RECV: 
			{
				// user;md5-password-hash;head;vscpclass;vscptype;obid;time-stamp;GUID,data1,data2,data2....
				char buf[1024];
				wxString strUser, strPassword, vscpevent;
				wxStringTokenizer tkz;

				memset( buf, 0, sizeof( buf ) );
				memcpy( buf, io->buf, io->len );
				iobuf_remove(io, io->len);        // Discard message from recv buffer
				tkz.SetString( wxString::FromAscii( buf ), _(";") );
				
				if ( !tkz.HasMoreTokens() ) {
					pUDPClientThread->m_pCtrlObject->logMsg ( _T ( "[UDP Client] UDP datagram does not have a valid format (user;password;event).\n" ), DAEMON_LOGMSG_ERROR );
					return;
				}

				// Get username
				strUser = tkz.GetNextToken();

				if ( !tkz.HasMoreTokens() ) {
					pUDPClientThread->m_pCtrlObject->logMsg ( _T ( "[UDP Client] UDP datagram does not have a valid format (user;password;event).\n" ), DAEMON_LOGMSG_ERROR );
					return;
				}

				// Get password hash
				strPassword = tkz.GetNextToken();

				if ( !tkz.HasMoreTokens() ) {
					pUDPClientThread->m_pCtrlObject->logMsg ( _T ( "[UDP Client] UDP datagram does not have a valid format (user;password;event).\n" ), DAEMON_LOGMSG_ERROR );
					return;
				}

				// Get event
				vscpevent = tkz.GetNextToken();

				// Check if user is valid
				// Calculate MD5 for username:autdomain:password
				strncpy( buf, strUser.mbc_str(), strUser.Length() );
				strncat( buf, ":", sizeof( buf ) );
				strncat( buf, pUDPClientThread->m_pCtrlObject->m_authDomain.mbc_str(), sizeof( buf ) );
				strncat( buf, ":", sizeof( buf ) );
				strncat( (char *)buf, strPassword.mb_str(), strPassword.Length() );
	
				Cmd5 md5 ( (unsigned char *)buf );
				if ( NULL == md5.getDigest() ) return; 
				wxString md5Password = wxString( md5.getDigest(), wxConvUTF8 );
				pUDPClientThread->m_pCtrlObject->m_mutexUserList.Lock();
				//::wxLogDebug( _("Username: ") + m_wxUserName );
				//::wxLogDebug( _("Password: ") + strPassword );
				//::wxLogDebug( _("MD5 of Password: ") + md5Password );
				pUDPClientThread->m_pClientItem->m_pUserItem = 
						pUDPClientThread->m_pCtrlObject->m_userList.checkUser( strUser, strPassword );
				pUDPClientThread->m_pCtrlObject->m_mutexUserList.Unlock();

				if ( NULL == pUDPClientThread->m_pClientItem->m_pUserItem ) {
                    wxString strErr = 
                        wxString::Format( _("[UDP Client] User [%s] allowed to connect.\n"), 
#ifdef WIN32						
                                                pUDPClientThread->m_pClientItem->m_UserName );
#else 
												(const char *)pUDPClientThread->m_pClientItem->m_UserName.mbc_str() );
#endif					
	                    pUDPClientThread->m_pCtrlObject->logMsg ( strErr, DAEMON_LOGMSG_WARNING, DAEMON_LOGTYPE_SECURITY );
					::wxLogDebug ( _("Password/Username failure.") );
					ns_send( conn,  MSG_PASSWORD_ERROR, strlen ( MSG_PASSWORD_ERROR ) );
					return;
				}

				// Get remte address
				wxString remoteaddr = wxString::FromAscii( inet_ntoa( conn->sa.sin.sin_addr )  );

				// Check if this user is allowed to connect from this location
				pUDPClientThread->m_pCtrlObject->m_mutexUserList.Lock();
				bool bValidHost = 
						pUDPClientThread->m_pCtrlObject->m_userList.checkRemote( pUDPClientThread->m_pClientItem->m_pUserItem, remoteaddr );
				pUDPClientThread->m_pCtrlObject->m_mutexUserList.Unlock();

				if ( !bValidHost ) {
					wxString strErr = wxString::Format(_("[UDP Client] Host [%s] not allowed to connect.\n"), 
#ifdef WIN32
						                                     remoteaddr );
#else 
						                                     (const char *)remoteaddr.mbc_str() );					
#endif					
					pUDPClientThread->m_pCtrlObject->logMsg ( strErr, DAEMON_LOGMSG_INFO );
					ns_send( conn,  MSG_INVALID_REMOTE_ERROR, strlen ( MSG_INVALID_REMOTE_ERROR ) );
					return;
				}

				// Copy in the user filter
				memcpy( &pUDPClientThread->m_pClientItem->m_filterVSCP, 
							&pUDPClientThread->m_pClientItem->m_pUserItem->m_filterVSCP, 
							sizeof( vscpEventFilter ) );

                wxString strErr = 
                            wxString::Format( _("[UDP Client] Host [%s] User [%s] allowed to connect.\n"), 
#ifdef WIN32
                                                remoteaddr, 
                                                pUDPClientThread->m_pClientItem->m_UserName );
#else 
												(const char *)remoteaddr.mbc_str(), 
                                                (const char *)pUDPClientThread->m_pClientItem->m_UserName.mbc_str() );
#endif				
	            pUDPClientThread->m_pCtrlObject->logMsg ( strErr, DAEMON_LOGMSG_WARNING, DAEMON_LOGTYPE_SECURITY );
			}
			break;

		case NS_POLL:
			break;

		default:
			break;
	}
}




