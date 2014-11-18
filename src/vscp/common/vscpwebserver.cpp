// vscpwebserver.cpp
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

#ifdef __GNUG__
//#pragma implementation
#endif


#ifdef WIN32
#include <winsock2.h>
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"
#include <wx/xml/xml.h>
#include <wx/txtstrm.h>
#include <wx/platinfo.h>

#ifdef WIN32

#include <winsock2.h>
//#include <winsock.h>
#include "canal_win32_ipc.h"

#else 	// UNIX

#define _POSIX
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <syslog.h>
#include <sys/msg.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <string.h>
#include <netdb.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/log.h"
#include "wx/socket.h"

#endif

#include <wx/config.h>
#include <wx/wfstream.h>
#include <wx/fileconf.h>
#include <wx/tokenzr.h>
#include <wx/listimpl.cpp>
#include <wx/xml/xml.h>
#include <wx/mimetype.h>
#include <wx/filename.h>

#include "web_css.h"
#include "web_js.h"
#include "web_template.h"

#include "../../common/slre.h"
#include "../../common/frozen.h"
#include "../../common/net_skeleton.h"
#include "../../common/mongoose.h"

#include "canal_macro.h"
#include "../common/vscp.h"
#include "../common/vscphelper.h"
#include "../common/vscpeventhelper.h"
#include "../common/tables.h"
#include "../../common/configfile.h"
#include "../../common/crc.h"
#include "../../common/md5.h"
#include "../../common/randpassword.h"
#include "../common/version.h"
#include "variablecodes.h"
#include "actioncodes.h"
#include "devicelist.h"
#include "devicethread.h"
#include "dm.h"
#include "vscpeventhelper.h"
#include "vscpwebserver.h"
#include "controlobject.h"
#include "../common/webserver.h"


#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

///////////////////////////////////////////////////
//		          WEBSERVER
///////////////////////////////////////////////////

// Linked list of all active sessions. (webserv.h)
static struct websrv_Session *websrv_sessions;

// Session structure for REST API
struct websrv_rest_session *websrv_rest_sessions;

///////////////////////////////////////////////////
//					WEBSOCKETS
///////////////////////////////////////////////////

// Linked list of websocket sessions
static struct websock_session *websock_sessions;


///////////////////////////////////////////////////
//					HELPERS
///////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// make_chunk
//

static void webserv_util_make_chunk( char *obuf, const char *buf, int len) {
  char chunk_size[512];
#ifdef WIN32
  int n = _snprintf( chunk_size, sizeof(chunk_size), "%X\r\n", len);
#else
  int n = snprintf( chunk_size, sizeof(chunk_size), "%X\r\n", len);
#endif
  strncat( obuf, chunk_size, n);
  strncat( obuf, buf, len);
  strncat( obuf, "\r\n", 2);
}


static void webserv_util_sendheader( struct mg_connection *conn, const int returncode, const char *content )
{
	char date[64];
	time_t curtime = time(NULL);
	vscp_getTimeString( date, sizeof(date), &curtime );

	mg_send_status( conn, returncode );
	mg_send_header( conn, "Content-Type", content );
	mg_send_header( conn, "Date", date );
	mg_send_header( conn, "Connection", "keep-alive" );
	mg_send_header( conn, "Transfer-Encoding", "chunked" );
	mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
												"pre-check=0, no-store, no-cache, must-revalidate");
}



///////////////////////////////////////////////////////////////////////////////
// TcpClientListenThread
//
// This thread listens for conection on a TCP socket and starts a new thread
// to handle client requests
//

VSCPWebServerThread::VSCPWebServerThread()
: wxThread(wxTHREAD_JOINABLE)
{
    //OutputDebugString( "TCP ClientThread: Create");
    m_bQuit = false;
    m_pCtrlObject = NULL;
}


VSCPWebServerThread::~VSCPWebServerThread()
{
	;
}


///////////////////////////////////////////////////////////////////////////////
// Entry
//

void *VSCPWebServerThread::Entry()
{
	clock_t ticks,oldus;
		
	// Create the server
	m_pCtrlObject->m_pwebserver = mg_create_server( m_pCtrlObject, VSCPWebServerThread::websrv_event_handler );
		
	// Set options

    // Serve current directory
	mg_set_option( m_pCtrlObject->m_pwebserver, 
						"document_root", 
						m_pCtrlObject->m_pathWebRoot.mb_str( wxConvUTF8 ) );		

    // Web server port(s)
    if ( m_pCtrlObject->m_portWebServer.Length() ) {
	    mg_set_option( m_pCtrlObject->m_pwebserver, 
						    "listening_port", 
						    m_pCtrlObject->m_portWebServer.mb_str( wxConvUTF8 ) );
    }

    // Authdomain
    if ( m_pCtrlObject->m_authDomain.Length() ) {
	    mg_set_option( m_pCtrlObject->m_pwebserver, 
		    				"auth_domain", 
			    			m_pCtrlObject->m_authDomain.mb_str( wxConvUTF8 ) );			
    }


    // Extra mime types
    if ( m_pCtrlObject->m_extraMimeTypes.Length() ) {
	    mg_set_option( m_pCtrlObject->m_pwebserver, 
		    				"extra_mime_types", 
			    			m_pCtrlObject->m_extraMimeTypes.mb_str( wxConvUTF8 ) );
    }

    // Path to SSL cert
    if ( m_pCtrlObject->m_pathCert.Length() ) {
	    mg_set_option( m_pCtrlObject->m_pwebserver, 
		    				"ssl_certificate", 
			    			m_pCtrlObject->m_pathCert.mb_str( wxConvUTF8 ) );
    }

    // CGI interpreter to use
    if ( m_pCtrlObject->m_cgiInterpreter.Length() ) {
	    mg_set_option( m_pCtrlObject->m_pwebserver, 
		    				"cgi_interpreter", 
			    			m_pCtrlObject->m_cgiInterpreter.mb_str( wxConvUTF8 ) );
    }

    // CGI pattern
    if ( m_pCtrlObject->m_cgiPattern.Length() ) {
	    mg_set_option( m_pCtrlObject->m_pwebserver, 
		    				"cgi_pattern", 
			    			m_pCtrlObject->m_cgiPattern.mb_str( wxConvUTF8 ) );
    }
	
    // enable directory listing
    if ( !m_pCtrlObject->bEnableDirectoryListing ) {
        mg_set_option( m_pCtrlObject->m_pwebserver, 
		    				"enable_directory_listing", 
			    			"no" );
    }

    // Hide file patterns
    if ( m_pCtrlObject->m_hideFilePatterns.Length() ) {
	    mg_set_option( m_pCtrlObject->m_pwebserver, 
		    				"hide_files_patterns", 
			    			m_pCtrlObject->m_hideFilePatterns.mb_str( wxConvUTF8 ) );
    }

    // Index files
    if ( m_pCtrlObject->m_indexFiles.Length() ) {
	    mg_set_option( m_pCtrlObject->m_pwebserver, 
		    				"index_files", 
			    			m_pCtrlObject->m_indexFiles.mb_str( wxConvUTF8 ) );
    }


    // Index files
    if ( m_pCtrlObject->m_indexFiles.Length() ) {
	    mg_set_option( m_pCtrlObject->m_pwebserver, 
		    				"index_files", 
			    			m_pCtrlObject->m_indexFiles.mb_str( wxConvUTF8 ) );
    }

    // URL Rewrites
    if ( m_pCtrlObject->m_urlRewrites.Length() ) {
	    mg_set_option( m_pCtrlObject->m_pwebserver, 
		    				"url_rewrites", 
			    			m_pCtrlObject->m_urlRewrites.mb_str( wxConvUTF8 ) );
    }

    // Run as user
    if ( m_pCtrlObject->m_runAsUserWeb.Length() ) {
	    mg_set_option( m_pCtrlObject->m_pwebserver, 
		    				"run_as_user", 
			    			m_pCtrlObject->m_runAsUserWeb.mb_str( wxConvUTF8 ) );
    }
    

	while ( !TestDestroy() && !m_bQuit ) {
	
		// CLOCKS_PER_SEC 	
		oldus = ticks = clock();
		
#ifndef WIN32
        struct timeval tv;
        gettimeofday(&tv, NULL);
#endif
		
		mg_poll_server( m_pCtrlObject->m_pwebserver, 50 );
		websock_post_incomingEvents();

#ifdef WIN32
			oldus = clock();
#else
            oldus = tv.tv_usec;
#endif

	}

	// Kill web server
	mg_destroy_server( & m_pCtrlObject->m_pwebserver );

    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void VSCPWebServerThread::OnExit()
{
	time_t tttt = time(NULL);;
}




///////////////////////////////////////////////////////////////////////////////
// readMimeTypes
//
// Read the Mime Types XML file 
//

bool VSCPWebServerThread::readMimeTypes(wxString& path)
{
    //unsigned long val;
    wxXmlDocument doc;
    if (!doc.Load(path)) {
        return false;
    }

    // start processing the XML file
    if (doc.GetRoot()->GetName() != wxT("mimetypes")) {
        return false;
    }

    wxXmlNode *child = doc.GetRoot()->GetChildren();
    while (child) {  
        
        if (child->GetName() == wxT("mimetype")) {           
            wxString strEnable = child->GetAttribute(wxT("enable"), wxT("false"));         
            wxString strExt = child->GetAttribute(wxT("extension"), wxT(""));            
            wxString strType = child->GetAttribute(wxT("mime"), wxT(""));
            
            if ( strEnable.IsSameAs(_("true"),false )) {
                m_hashMimeTypes[strExt] = strType;
            }
        }
        
        child = child->GetNext();

    }
    
    return true;
}










///////////////////////////////////////////////////////////////////////////////
//                               WEBSOCKET
///////////////////////////////////////////////////////////////////////////////












///////////////////////////////////////////////////////////////////////////////
// websock_command
//

int
VSCPWebServerThread::websock_command( struct mg_connection *conn, 
										struct websock_session *pSession,
										wxString& strCmd )
{
	wxString strTok;
	int rv = MG_TRUE;

	// Check pointer
    if (NULL == conn) return MG_FALSE;
	if (NULL == pSession) return MG_FALSE;
	
	CControlObject *pCtrlObject = (CControlObject *)conn->server_param;
	if (NULL == pCtrlObject) return MG_FALSE;

	//mg_websocket_write( conn, WEBSOCKET_OPCODE_TEXT, conn->content, conn->content_len );
	//if ( conn->content_len == 4 && memcmp( conn->content, "exit", 4 ) ) {
	//	return MG_FALSE;
	//}

	//mg_websocket_write( conn, WEBSOCKET_OPCODE_PING, NULL, 0 );

    wxStringTokenizer tkz( strCmd, _(";") );

    // Get command
    if (tkz.HasMoreTokens()) {
        strTok = tkz.GetNextToken();
        strTok.MakeUpper();
    } 
	else {
        mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_SYNTAX_ERROR, 
									WEBSOCK_STR_ERROR_SYNTAX_ERROR );
        return MG_TRUE;
    }
    // ------------------------------------------------------------------------
    //                                NOOP
    //-------------------------------------------------------------------------
    if (0 == strTok.Find(_("NOOP"))) {
        mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, "+;NOOP" );
	}
    // ------------------------------------------------------------------------
    //                                AUTH
    //-------------------------------------------------------------------------
	// AUTH;user;hash
	else if (0 == strTok.Find(_("AUTH"))) {
		wxString strUser = tkz.GetNextToken();
		wxString strKey = tkz.GetNextToken();
		if ( MG_TRUE == pCtrlObject->getWebServer()->websock_authentication( conn, pSession, strUser, strKey ) ) {
			mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, "+;AUTH1" );
			pSession->bAuthenticated = true;	// Authenticated
		}
		else {
			mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_NOT_AUTHORIZED, 
									WEBSOCK_STR_ERROR_NOT_AUTHORIZED );
			pSession->bAuthenticated = false;	// Authenticated
		}
    } 
    // ------------------------------------------------------------------------
    //                                OPEN
    //-------------------------------------------------------------------------
	else if (0 == strTok.Find(_("OPEN"))) {

		// Must be authorized to do this
		if ( !pSession->bAuthenticated ) {
			mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_NOT_AUTHORIZED,
									WEBSOCK_STR_ERROR_NOT_AUTHORIZED );
			return MG_TRUE;	// We still leave channel open
		}



        pSession->m_pClientItem->m_bOpen = true;
        mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, "+;OPEN" );
    } 

	else if (0 == strTok.Find(_("CLOSE"))) {
        pSession->m_pClientItem->m_bOpen = false;
        mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, "+;CLOSE" );
		rv = MG_FALSE;
    } 
    // ------------------------------------------------------------------------
    //                                SETFILTER
    //-------------------------------------------------------------------------
	else if (0 == strTok.Find(_("SETFILTER"))) {

        unsigned char ifGUID[ 16 ];
        memset(ifGUID, 0, 16);

		// Must be authorized to do this
		if ( !pSession->bAuthenticated ) {
			mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_NOT_AUTHORIZED,
									WEBSOCK_STR_ERROR_NOT_AUTHORIZED );
            pCtrlObject->logMsg ( _("[Websocket] User/host not authorized to do that.\n"), 
										DAEMON_LOGMSG_INFO, 
										DAEMON_LOGTYPE_SECURITY );
			
			return MG_TRUE;	// We still leave channel open
		}

        // Check privilege
        if ( (pSession->m_pClientItem->m_pUserItem->m_userRights & 0xf) < 6 ) {
            mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,
									WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT );
			wxString strErr = 
                        wxString::Format( _("[Websocket] User [%s] not allowed to do that.\n"), 
                                                (const char *)pSession->m_pClientItem->m_pUserItem->m_user.c_str() );			
		
	        pCtrlObject->logMsg ( strErr, DAEMON_LOGMSG_INFO, DAEMON_LOGTYPE_SECURITY );
            return MG_TRUE;	// We still leave channel open	
        }

        // Get filter
        if (tkz.HasMoreTokens()) {
            strTok = tkz.GetNextToken();
			pSession->m_pClientItem->m_mutexClientInputQueue.Lock();
            if (!vscp_readFilterFromString( &pSession->m_pClientItem->m_filterVSCP, strTok ) ) {
				pSession->m_pClientItem->m_mutexClientInputQueue.Unlock();
                mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_SYNTAX_ERROR, 
									WEBSOCK_STR_ERROR_SYNTAX_ERROR );
                return MG_TRUE;
            }
			pSession->m_pClientItem->m_mutexClientInputQueue.Unlock();
        } 
		else {
            mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_SYNTAX_ERROR, 
									WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            return MG_TRUE;
        }

        // Get mask
        if (tkz.HasMoreTokens()) {
            strTok = tkz.GetNextToken();
			pSession->m_pClientItem->m_mutexClientInputQueue.Lock();
            if (!vscp_readMaskFromString( &pSession->m_pClientItem->m_filterVSCP, strTok)) {
				pSession->m_pClientItem->m_mutexClientInputQueue.Unlock();
                mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_SYNTAX_ERROR, 
									WEBSOCK_STR_ERROR_SYNTAX_ERROR );
				pSession->m_pClientItem->m_mutexClientInputQueue.Unlock();
                return MG_TRUE;
            }
            pSession->m_pClientItem->m_mutexClientInputQueue.Unlock();
        } 
		else {
            mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_SYNTAX_ERROR, 
									WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            return MG_TRUE;
        }

        // Positive response
        mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, "+;SETFILTER" );

    }
    // ------------------------------------------------------------------------
    //                                CLRQUEUE
    //-------------------------------------------------------------------------
    // Clear the event queue
    else if (0 == strTok.Find(_("CLRQUEUE"))) {

        CLIENTEVENTLIST::iterator iterVSCP;

		// Must be authorized to do this
		if ( !pSession->bAuthenticated ) {
			mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_NOT_AUTHORIZED,
									WEBSOCK_STR_ERROR_NOT_AUTHORIZED );
            pCtrlObject->logMsg ( _("[Websocket] User/host not authorized to do that.\n"), 
										DAEMON_LOGMSG_INFO, 
										DAEMON_LOGTYPE_SECURITY );
			
			return MG_TRUE;	// We still leave channel open
		}

        // Check privilege
        if ( (pSession->m_pClientItem->m_pUserItem->m_userRights & 0xf) < 1 ) {
            mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,
									WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT );
			wxString strErr = 
                        wxString::Format( _("[Websocket] User [%s] not allowed to do that.\n"), 
                                                (const char *)pSession->m_pClientItem->m_pUserItem->m_user.c_str() );			
		
	        pCtrlObject->logMsg ( strErr, DAEMON_LOGMSG_INFO, DAEMON_LOGTYPE_SECURITY );
            return MG_TRUE;	// We still leave channel open	
        }

        pSession->m_pClientItem->m_mutexClientInputQueue.Lock();
        for (iterVSCP = pSession->m_pClientItem->m_clientInputQueue.begin();
                iterVSCP != pSession->m_pClientItem->m_clientInputQueue.end(); ++iterVSCP) {
            vscpEvent *pEvent = *iterVSCP;
            vscp_deleteVSCPevent(pEvent);
        }

        pSession->m_pClientItem->m_clientInputQueue.Clear();
        pSession->m_pClientItem->m_mutexClientInputQueue.Unlock();

        mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, "+;CLRQUE" );
    } 
    // ------------------------------------------------------------------------
    //                                  WRITEVAR
    //-------------------------------------------------------------------------
	else if (0 == strTok.Find(_("WRITEVAR"))) {

		// Must be authorized to do this
		if ( !pSession->bAuthenticated ) {
			mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_NOT_AUTHORIZED,
									WEBSOCK_STR_ERROR_NOT_AUTHORIZED );
            pCtrlObject->logMsg ( _("[Websocket] User/host not authorized to do that.\n"), 
										DAEMON_LOGMSG_INFO, 
										DAEMON_LOGTYPE_SECURITY );
			
			return MG_TRUE;	// We still leave channel open
		}

        // Check privilege
        if ( (pSession->m_pClientItem->m_pUserItem->m_userRights & 0xf) < 6 ) {
            mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,
									WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT );
			wxString strErr = 
                        wxString::Format( _("[Websocket] User [%s] not allowed to do that.\n"), 
                                                (const char *)pSession->m_pClientItem->m_pUserItem->m_user.c_str() );			
		
	        pCtrlObject->logMsg ( strErr, DAEMON_LOGMSG_INFO, DAEMON_LOGTYPE_SECURITY );
            return MG_TRUE;	// We still leave channel open	
        }

        // Get variablename
        if (tkz.HasMoreTokens()) {

            uint8_t type = VSCP_DAEMON_VARIABLE_CODE_STRING;
            bool bPersistent = false;

            CVSCPVariable *pvar;
            strTok = tkz.GetNextToken();
            if (NULL == (pvar = pCtrlObject->m_VSCP_Variables.find(strTok))) {
                mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_VARIABLE_UNKNOWN, 
									WEBSOCK_STR_ERROR_VARIABLE_UNKNOWN );
                return MG_TRUE;
            }


            // Get variable value
            if (tkz.HasMoreTokens()) {
                strTok = tkz.GetNextToken();
                if (!pvar->setValueFromString(pvar->getType(), strTok)) {
                    mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_SYNTAX_ERROR, 
									WEBSOCK_STR_ERROR_SYNTAX_ERROR );
                    return MG_TRUE;
                }
            } 
			else {
                mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_SYNTAX_ERROR, 
									WEBSOCK_STR_ERROR_SYNTAX_ERROR );
                return MG_TRUE;
            }
        } 
		else {
            mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_SYNTAX_ERROR, 
									WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            return MG_TRUE;
        }

        wxString resultstr = _("+;WRITEVAR;");
        resultstr += strTok;

        // Positive reply
        mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, "+;WRITEVAR" );

    } 
    // ------------------------------------------------------------------------
    //                               CREATEVAR
    //-------------------------------------------------------------------------
	else if (0 == strTok.Find(_("CREATEVAR"))) {

        wxString name;
        wxString value;
        uint8_t type = VSCP_DAEMON_VARIABLE_CODE_STRING;
        bool bPersistent = false;

		// Must be authorized to do this
		if ( !pSession->bAuthenticated ) {
			mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_NOT_AUTHORIZED,
									WEBSOCK_STR_ERROR_NOT_AUTHORIZED );
            pCtrlObject->logMsg ( _("[Websocket] User/host not authorized to do that.\n"), 
										DAEMON_LOGMSG_INFO, 
										DAEMON_LOGTYPE_SECURITY );
			
			return MG_TRUE;	// We still leave channel open
		}

        // Check privilege
        if ( (pSession->m_pClientItem->m_pUserItem->m_userRights & 0xf) < 6 ) {
            mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,
									WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT );
			wxString strErr = 
                        wxString::Format( _("[Websocket] User [%s] not allowed to do that.\n"), 
                                                (const char *)pSession->m_pClientItem->m_pUserItem->m_user.c_str() );			
		
	        pCtrlObject->logMsg ( strErr, DAEMON_LOGMSG_INFO, DAEMON_LOGTYPE_SECURITY );
            return MG_TRUE;	// We still leave channel open	
        }

        // Get variable name
        if (tkz.HasMoreTokens()) {
            name = tkz.GetNextToken();
        } 
		else {
            mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_SYNTAX_ERROR, 
									WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            return MG_TRUE;
        }   

        // Get variable type
        if (tkz.HasMoreTokens()) {
            type = vscp_readStringValue(tkz.GetNextToken());
        }

        // Get variable Persistence
        if (tkz.HasMoreTokens()) {
            int val = vscp_readStringValue(tkz.GetNextToken());
        }

        // Get variable value
        if (tkz.HasMoreTokens()) {
            value = tkz.GetNextToken();
        } 
		else {
            mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_SYNTAX_ERROR, 
									WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            return MG_TRUE;
        }

        // Add the variable
        if (!pCtrlObject->m_VSCP_Variables.add(name, value, type, bPersistent)) {
            mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_SYNTAX_ERROR, 
									WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            return MG_TRUE;
        } 

        wxString resultstr = _("+;CREATEVAR;");
        resultstr += name;

        mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, (const char *)resultstr.c_str() );
        
    } 
    // ------------------------------------------------------------------------
    //                               READVAR
    //-------------------------------------------------------------------------
	else if (0 == strTok.Find(_("READVAR"))) {

        CVSCPVariable *pvar;
        uint8_t type;
        wxString strvalue;

		// Must be authorized to do this
		if ( !pSession->bAuthenticated ) {
			mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_NOT_AUTHORIZED,
									WEBSOCK_STR_ERROR_NOT_AUTHORIZED );
					
			pCtrlObject->logMsg ( _("[Websocket] User/host not authorized to do that.\n"), 
										DAEMON_LOGMSG_INFO, 
										DAEMON_LOGTYPE_SECURITY );
			
			return MG_TRUE;	// We still leave channel open
		}

        // Check privilege
        if ( (pSession->m_pClientItem->m_pUserItem->m_userRights & 0xf) < 4 ) {
            mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,
									WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT );
			wxString strErr = 
                        wxString::Format( _("[Websocket] User [%s] not allowed to do that.\n"), 
                                                (const char *)pSession->m_pClientItem->m_pUserItem->m_user.c_str() );			
		
	        pCtrlObject->logMsg ( strErr, DAEMON_LOGMSG_INFO, DAEMON_LOGTYPE_SECURITY );
            return MG_TRUE;	// We still leave channel open	
        }

        strTok = tkz.GetNextToken();
        if (NULL == (pvar = pCtrlObject->m_VSCP_Variables.find(strTok))) {
			mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_VARIABLE_UNKNOWN, 
									WEBSOCK_STR_ERROR_VARIABLE_UNKNOWN );
            return MG_TRUE;
        }

        pvar->writeValueToString(strvalue);
        type = pvar->getType();

        wxString resultstr = _("+;READVAR;");
        resultstr += strTok;
        resultstr += _(";");
        resultstr += wxString::Format(_("%d"), type);
        resultstr += _(";");
        resultstr += strvalue;
		mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, (const char *)resultstr.c_str() );

    } 
    // ------------------------------------------------------------------------
    //                               RESETVAR
    //-------------------------------------------------------------------------
    else if (0 == strTok.Find(_("RESETVAR"))) {

        CVSCPVariable *pvar;

		// Must be authorized to do this
		if ( !pSession->bAuthenticated ) {
			mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_NOT_AUTHORIZED,
									WEBSOCK_STR_ERROR_NOT_AUTHORIZED );
			return MG_TRUE;	// We still leave channel open
		}

        // Check privilege
        if ( (pSession->m_pClientItem->m_pUserItem->m_userRights & 0xf) < 6 ) {
            mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,
									WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT );
			wxString strErr = 
                        wxString::Format( _("[Websocket] User [%s] not allowed to do that.\n"), 
                                                (const char *)pSession->m_pClientItem->m_pUserItem->m_user.c_str() );			
		
	        pCtrlObject->logMsg ( strErr, DAEMON_LOGMSG_INFO, DAEMON_LOGTYPE_SECURITY );
            return MG_TRUE;	// We still leave channel open	
        }

        strTok = tkz.GetNextToken();
        if (NULL == (pvar = pCtrlObject->m_VSCP_Variables.find(strTok))) {
			mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_VARIABLE_UNKNOWN, 
									WEBSOCK_STR_ERROR_VARIABLE_UNKNOWN );
            pCtrlObject->logMsg ( _("[Websocket] User/host not authorized to do that.\n"), 
										DAEMON_LOGMSG_INFO, 
										DAEMON_LOGTYPE_SECURITY );
			
            return MG_TRUE;
        }

        pvar->Reset();

		wxString strResult = _("+;RESETVAR;");
        strResult += strTok;
		mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, (const char *)strResult.c_str() );

    }
    // ------------------------------------------------------------------------
    //                              REMOVEVAR
    //-------------------------------------------------------------------------
    else if (0 == strTok.Find(_("REMOVEVAR"))) {

        CVSCPVariable *pvar;
        wxString strvalue;

		// Must be authorized to do this
		if ( !pSession->bAuthenticated ) {
			mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_NOT_AUTHORIZED,
									WEBSOCK_STR_ERROR_NOT_AUTHORIZED );
            pCtrlObject->logMsg ( _("[Websocket] User/host not authorized to do that.\n"), 
										DAEMON_LOGMSG_INFO, 
										DAEMON_LOGTYPE_SECURITY );
			
			return MG_TRUE;	// We still leave channel open
		}

        // Check privilege
        if ( (pSession->m_pClientItem->m_pUserItem->m_userRights & 0xf) < 6 ) {
            mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,
									WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT );
			wxString strErr = 
                        wxString::Format( _("[Websocket] User [%s] not allowed to do that.\n"), 
                                                (const char *)pSession->m_pClientItem->m_pUserItem->m_user.c_str() );			
		
	        pCtrlObject->logMsg ( strErr, DAEMON_LOGMSG_INFO, DAEMON_LOGTYPE_SECURITY );
            return MG_TRUE;	// We still leave channel open	
        }

        strTok = tkz.GetNextToken();
        if (NULL == (pvar = pCtrlObject->m_VSCP_Variables.find(strTok))) {
			mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_VARIABLE_UNKNOWN, 
									WEBSOCK_STR_ERROR_VARIABLE_UNKNOWN );
            return MG_TRUE;
        }

        pCtrlObject->m_variableMutex.Lock();
        pCtrlObject->m_VSCP_Variables.remove( strTok );
        pCtrlObject->m_variableMutex.Unlock();

        wxString strResult = _("+;REMOVEVAR;");
        strResult += strTok;
		mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, (const char *)strResult.c_str() );

    }
    // ------------------------------------------------------------------------
    //                             LENGTHVAR
    //-------------------------------------------------------------------------
    else if (0 == strTok.Find(_("LENGTHVAR"))) {

        CVSCPVariable *pvar;

		// Must be authorized to do this
		if ( !pSession->bAuthenticated ) {
			mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_NOT_AUTHORIZED,
									WEBSOCK_STR_ERROR_NOT_AUTHORIZED );
            pCtrlObject->logMsg ( _("[Websocket] User/host not authorized to do that.\n"), 
										DAEMON_LOGMSG_INFO, 
										DAEMON_LOGTYPE_SECURITY );
			
			return MG_TRUE;	// We still leave channel open
		}

        // Check privilege
        if ( (pSession->m_pClientItem->m_pUserItem->m_userRights & 0xf) < 4 ) {
            mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,
									WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT );
			wxString strErr = 
                        wxString::Format( _("[Websocket] User [%s] not allowed to do that.\n"), 
                                                (const char *)pSession->m_pClientItem->m_pUserItem->m_user.c_str() );			
		
	        pCtrlObject->logMsg ( strErr, DAEMON_LOGMSG_INFO, DAEMON_LOGTYPE_SECURITY );
            return MG_TRUE;	// We still leave channel open	
        }

        strTok = tkz.GetNextToken();
        if (NULL == (pvar = pCtrlObject->m_VSCP_Variables.find(strTok))) {
			mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_VARIABLE_UNKNOWN, 
									WEBSOCK_STR_ERROR_VARIABLE_UNKNOWN );
            return MG_TRUE;
        }

        wxString resultstr = _("+;LENGTHVAR;");
        resultstr += strTok;
        resultstr += _(";");
        resultstr += wxString::Format(_("%d"), pvar->m_strValue.Length() );
		mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, (const char *)resultstr.c_str() );

    }
    // ------------------------------------------------------------------------
    //                           LASTCHANGEVAR
    //-------------------------------------------------------------------------
    else if (0 == strTok.Find(_("LASTCHANGEVAR"))) {

        CVSCPVariable *pvar;
        uint8_t type;
        wxString strvalue;

		// Must be authorized to do this
		if ( !pSession->bAuthenticated ) {
			mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_NOT_AUTHORIZED,
									WEBSOCK_STR_ERROR_NOT_AUTHORIZED );
            pCtrlObject->logMsg ( _("[Websocket] User/host not authorized to do that.\n"), 
										DAEMON_LOGMSG_INFO, 
										DAEMON_LOGTYPE_SECURITY );
			
			return MG_TRUE;	// We still leave channel open
		}

        // Check privilege
        if ( (pSession->m_pClientItem->m_pUserItem->m_userRights & 0xf) < 4 ) {
            mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,
									WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT );
			wxString strErr = 
                        wxString::Format( _("[Websocket] User [%s] not allowed to do that.\n"), 
                                                (const char *)pSession->m_pClientItem->m_pUserItem->m_user.c_str() );			
		
	        pCtrlObject->logMsg ( strErr, DAEMON_LOGMSG_INFO, DAEMON_LOGTYPE_SECURITY );
            return MG_TRUE;	// We still leave channel open	
        }

        strTok = tkz.GetNextToken();
        if (NULL == (pvar = pCtrlObject->m_VSCP_Variables.find(strTok))) {
			mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_VARIABLE_UNKNOWN, 
									WEBSOCK_STR_ERROR_VARIABLE_UNKNOWN );
            return MG_TRUE;
        }

        pvar->writeValueToString(strvalue);
        type = pvar->getType();

        wxString resultstr = _("+;LASTCHANGEVAR;");
        resultstr +=  pvar->getLastChange().FormatISODate() + _(" ") +  pvar->getLastChange().FormatISOTime();
		mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, (const char *)resultstr.c_str() );

    }
    // ------------------------------------------------------------------------
    //                               LISTVAR
    //-------------------------------------------------------------------------
    else if (0 == strTok.Find(_("LISTVAR"))) {

        CVSCPVariable *pvar;
        wxString strvalue;

		// Must be authorized to do this
		if ( !pSession->bAuthenticated ) {
			mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_NOT_AUTHORIZED,
									WEBSOCK_STR_ERROR_NOT_AUTHORIZED );
            pCtrlObject->logMsg ( _("[Websocket] User/host not authorized to do that.\n"), 
										DAEMON_LOGMSG_INFO, 
										DAEMON_LOGTYPE_SECURITY );
			
			return MG_TRUE;	// We still leave channel open
		}

        // Check privilege
        if ( (pSession->m_pClientItem->m_pUserItem->m_userRights & 0xf) < 4 ) {
            mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,
									WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT );
			wxString strErr = 
                        wxString::Format( _("[Websocket] User [%s] not allowed to do that.\n"), 
                                                (const char *)pSession->m_pClientItem->m_pUserItem->m_user.c_str() );			
		
	        pCtrlObject->logMsg ( strErr, DAEMON_LOGMSG_INFO, DAEMON_LOGTYPE_SECURITY );
            return MG_TRUE;	// We still leave channel open	
        }
      
        int i = 0;
        wxString resultstr;
        wxString strWork;
        m_pCtrlObject->m_variableMutex.Lock();

        listVscpVariable::iterator it;
        for( it = m_pCtrlObject->m_VSCP_Variables.m_listVariable.begin(); 
                    it != m_pCtrlObject->m_VSCP_Variables.m_listVariable.end(); 
                    ++it ) {

            if ( NULL == ( pvar = *it ) ) continue;

            wxString resultstr = _("+;LISTVAR;");
            resultstr += wxString::Format( _("%d;"), i++ );
            resultstr += pvar->getName();
            resultstr += _(";");
            strWork.Printf( _("%d"), pvar->getType() ) ;
            resultstr += strWork;
            if ( pvar->isPersistent() ) {
                resultstr += _(";true;");
            }
            else {
                resultstr += _(";false;");
            }
				
            pvar->writeValueToString( strWork );
            resultstr += strWork;

		    mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, (const char *)resultstr.c_str() );
        }

        m_pCtrlObject->m_variableMutex.Unlock();


    }
    // ------------------------------------------------------------------------
    //                                SAVEVAR
    //-------------------------------------------------------------------------
	else if (0 == strTok.Find(_("SAVEVAR"))) {

		// Must be authorized to do this
		if ( !pSession->bAuthenticated ) {
			mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_NOT_AUTHORIZED,
									WEBSOCK_STR_ERROR_NOT_AUTHORIZED );
            pCtrlObject->logMsg ( _("[Websocket] User/host not authorized to do that.\n"), 
										DAEMON_LOGMSG_INFO, 
										DAEMON_LOGTYPE_SECURITY );
			
			return MG_TRUE;	// We still leave channel open
		}

        // Check privilege
        if ( (pSession->m_pClientItem->m_pUserItem->m_userRights & 0xf) < 4 ) {
            mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,
									WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT );
			wxString strErr = 
                        wxString::Format( _("[Websocket] User [%s] not allowed to do that.\n"), 
                                                (const char *)pSession->m_pClientItem->m_pUserItem->m_user.c_str() );			
		
	        pCtrlObject->logMsg ( strErr, DAEMON_LOGMSG_INFO, DAEMON_LOGTYPE_SECURITY );
            return MG_TRUE;	// We still leave channel open	
        }

        if (!pCtrlObject->m_VSCP_Variables.save()) {
            mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_SYNTAX_ERROR, 
									WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            return MG_TRUE;
        }

        mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, "+;SAVEVAR" );
    } 
    // ------------------------------------------------------------------------
    //                                GETTABLE
    //-------------------------------------------------------------------------
	else if (0 == strTok.Find(_("GT"))) {

        // Format is:
        //      tablename;from;to

        // Must be authorized to do this
		if ( !pSession->bAuthenticated ) {
			mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_NOT_AUTHORIZED,
									WEBSOCK_STR_ERROR_NOT_AUTHORIZED );
            pCtrlObject->logMsg ( _("[Websocket] User/host not authorized to do that.\n"), 
										DAEMON_LOGMSG_INFO, 
										DAEMON_LOGTYPE_SECURITY );
			
			return MG_TRUE;	// We still leave channel open
		}

        // Check privilege
        if ( (pSession->m_pClientItem->m_pUserItem->m_userRights & 0xf) < 4 ) {
            mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,
									WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT );
			wxString strErr = 
                        wxString::Format( _("[Websocket] User [%s] not allowed to do that.\n"), 
                                                (const char *)pSession->m_pClientItem->m_pUserItem->m_user.c_str() );			
		
	        pCtrlObject->logMsg ( strErr, DAEMON_LOGMSG_INFO, DAEMON_LOGTYPE_SECURITY );
            return MG_TRUE;	// We still leave channel open	
        }

        wxString tblName;
        wxDateTime timeFrom = wxDateTime::Now();
        wxDateTime timeTo = wxDateTime::Now();
        int nRange = 0;

        // Get name of table
        if ( tkz.HasMoreTokens() ) {
            tblName = tkz.GetNextToken();
            tblName.Trim();
            tblName.Trim(false);
            tblName.MakeUpper();
        }
        else {
            // Must have a table name
            mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_MUST_HAVE_TABLE_NAME,
									WEBSOCK_STR_ERROR_MUST_HAVE_TABLE_NAME );
			wxString strErr = 
                      wxString::Format( _("[Websocket] Must have a tablename to read a table.\n") );					
	        pCtrlObject->logMsg ( strErr, DAEMON_LOGMSG_INFO, DAEMON_LOGTYPE_GENERAL );
            return MG_TRUE;
        }

        // Get from-date for range
        if ( tkz.HasMoreTokens() ) {
            wxString wxstr = tkz.GetNextToken();
            timeFrom.ParseDateTime( wxstr );
            wxstr = timeFrom.FormatISODate() + _(" ") + timeFrom.FormatISOTime();
            nRange++; // Start date entered
        }

        // Get to-date for range
        if ( tkz.HasMoreTokens() ) {
            wxString wxstr = tkz.GetNextToken();
            timeTo.ParseDateTime( wxstr);
            wxstr = timeTo.FormatISODate() + _(" ") + timeTo.FormatISOTime();
            nRange++; // End date entered
        }

        if ( (nRange > 1) && timeTo.IsEarlierThan( timeFrom ) ) {

            // To date must be later then from date
            mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_END_DATE_IS_WRONG,
									WEBSOCK_STR_ERROR_END_DATE_IS_WRONG );
			wxString strErr = 
                      wxString::Format( _("[Websocket] The end date must be later than the start date.\n") );					
	        pCtrlObject->logMsg ( strErr, DAEMON_LOGMSG_INFO, DAEMON_LOGTYPE_GENERAL );
            return MG_TRUE;

        }

        CVSCPTable *ptblItem = NULL;
        pCtrlObject->m_mutexTableList.Lock();
        listVSCPTables::iterator iter;
        for (iter = pCtrlObject->m_listTables.begin(); iter != pCtrlObject->m_listTables.end(); ++iter) {
            ptblItem = *iter;
            if ( tblName== wxString::FromUTF8( ptblItem->m_vscpFileHead.nameTable ) ) {
                break;
            }
            ptblItem = NULL;
        }
        pCtrlObject->m_mutexTableList.Unlock();

        // Did we find it?
        if ( NULL == ptblItem ) {
            // nope
            mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_TABLE_NOT_FOUND,
									WEBSOCK_STR_ERROR_TABLE_NOT_FOUND );
			wxString strErr = 
                wxString::Format( _("[Websocket] Table not found. [name=%s]\n"), (const char *)tblName.c_str() );					
	        pCtrlObject->logMsg ( strErr, DAEMON_LOGMSG_INFO, DAEMON_LOGTYPE_GENERAL );
            return MG_TRUE;
        }

        if ( VSCP_TABLE_DYNAMIC == ptblItem->m_vscpFileHead.type ) {

            uint64_t start,end;
            if ( 0 == nRange ) {
                // Neither 'from' or 'to' given 
                // Use value from header
                start = ptblItem->getTimeStampStart();
                end = ptblItem->getTimeStampStart();
            }
            else if ( 1 == nRange ) { 
                // From given but no end
                start = timeFrom.GetTicks();
                end = ptblItem->getTimeStampStart();
            }
            else {
                // range given
                start = timeFrom.GetTicks();
                end = timeTo.GetTicks();
            }

            // Fetch number of records in set 
            ptblItem->m_mutexThisTable.Lock();
            long nRecords = ptblItem->getRangeOfData(start, end );
            ptblItem->m_mutexThisTable.Unlock();

            if ( nRecords > 0 ) {

                struct _vscpFileRecord *pRecords = new struct _vscpFileRecord[nRecords];

                if ( NULL == pRecords ) {
                    mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_MEMORY_ALLOCATION,
									WEBSOCK_STR_ERROR_MEMORY_ALLOCATION );
			        wxString strErr = 
                            wxString::Format( _("[Websocket] Having problems to allocate memory. [name=%s]\n"), (const char *)tblName.c_str() );					
	                pCtrlObject->logMsg ( strErr, DAEMON_LOGMSG_ERROR, DAEMON_LOGTYPE_GENERAL );
                    return MG_TRUE;
                }
                
                ptblItem->m_mutexThisTable.Lock();
                long nfetchedRecords = ptblItem->getRangeOfData(start, end, (void *)pRecords, nRecords* sizeof(struct _vscpFileRecord )  );
                ptblItem->m_mutexThisTable.Unlock();

                if ( 0 == nfetchedRecords ) {
                    mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_TABLE_ERROR_READING,
									WEBSOCK_STR_ERROR_TABLE_ERROR_READING );
			        wxString strErr = 
                        wxString::Format( _("[Websocket] Problem when reading table. [name=%s]\n"), (const char *)tblName.c_str() );					
	                pCtrlObject->logMsg ( strErr, DAEMON_LOGMSG_ERROR, DAEMON_LOGTYPE_GENERAL );
                    return MG_TRUE;
                }
                else {

                    wxDateTime dtStart;
                    dtStart.Set( (time_t)ptblItem->getTimeStampStart() );
                    wxString strDateTimeStart = dtStart.FormatISODate() + _("T") + dtStart.FormatISOTime();

                    wxDateTime dtEnd;
                    dtEnd.Set( (time_t)ptblItem->getTimeStampEnd() );
                    wxString strDateTimeEnd = dtEnd.FormatISODate() + _("T") + dtEnd.FormatISOTime();

                    // First send start post with number if records
                    wxString wxstr = wxString::Format(_("+;GT;START;%d;%d;%s;%s"),
                                                nfetchedRecords,
                                                ptblItem->getNumberOfRecords(),
                                                (const char *)strDateTimeStart.c_str(),
                                                (const char *)strDateTimeEnd.c_str() );
                    mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									(const char *)wxstr.c_str() );

                    // Then send measurement records
                    for ( long i=0; i<nfetchedRecords; i++ ) {
                        wxDateTime dt;
                        dt.Set( (time_t)pRecords[i].timestamp );
                        wxString strDateTime = dt.FormatISODate() + _(" ") + dt.FormatISOTime();
                        wxString wxstr = wxString::Format(_("+;GT;%d;%s;%f"),
                                                i, 
                                                (const char *)strDateTime.c_str(), 
                                                pRecords[i].measurement );
                        mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									(const char *)wxstr.c_str() );
                    }

                    // Last send end post with number if records
                    mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"+;GT;END;%d",
                                    nfetchedRecords );
                }

                // Deallocate storage
                delete[] pRecords;

            }
            else {
                if ( 0 == nRecords ) {
                    mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_TABLE_NO_DATA,
									WEBSOCK_STR_ERROR_TABLE_NO_DATA );
			        wxString strErr = 
                         wxString::Format( _("[Websocket] No data in table. [name=%s]\n"), (const char *)tblName.c_str() );					
	                pCtrlObject->logMsg ( strErr, DAEMON_LOGMSG_INFO, DAEMON_LOGTYPE_GENERAL );
                    return MG_TRUE;
                }
                else {
                    mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_TABLE_ERROR_READING,
									WEBSOCK_STR_ERROR_TABLE_ERROR_READING );
			        wxString strErr = 
                         wxString::Format( _("[Websocket] Problem when reading table. [name=%s]\n"), (const char *)tblName.c_str() );					
	                pCtrlObject->logMsg ( strErr, DAEMON_LOGMSG_ERROR, DAEMON_LOGTYPE_GENERAL );
                    return MG_TRUE;
                }
            }

        }
        // OK STATIC table
        else {

            // Fetch number of records in set 
            ptblItem->m_mutexThisTable.Lock();
            long nRecords = ptblItem->getStaticRequiredBuffSize();
            ptblItem->m_mutexThisTable.Unlock();
            
            if ( nRecords > 0 ) {
            
                ptblItem->m_mutexThisTable.Lock();
                long nfetchedRecords = ptblItem->getStaticRequiredBuffSize();
                ptblItem->m_mutexThisTable.Unlock();

                if ( 0 == nfetchedRecords ) {
                    mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_TABLE_ERROR_READING,
									WEBSOCK_STR_ERROR_TABLE_ERROR_READING );
			        wxString strErr = 
                         wxString::Format( _("[Websocket] Problem when reading table. [name=%s]\n"), (const char *)tblName.c_str() );					
	                pCtrlObject->logMsg ( strErr, DAEMON_LOGMSG_ERROR, DAEMON_LOGTYPE_GENERAL );
                    return MG_TRUE;
                }
                else {

                    struct _vscpFileRecord *pRecords = new struct _vscpFileRecord[nfetchedRecords];

                    if ( NULL == pRecords ) {
                        mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_MEMORY_ALLOCATION,
									WEBSOCK_STR_ERROR_MEMORY_ALLOCATION );
			            wxString strErr = 
                             wxString::Format( _("[Websocket] Having problems to allocate memory. [name=%s]\n"), (const char *)tblName.c_str() );					
	                    pCtrlObject->logMsg ( strErr, DAEMON_LOGMSG_ERROR, DAEMON_LOGTYPE_GENERAL );
                        return MG_TRUE;
                    }

                    // First send start post with number if records
                    mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"+;GT;START;%d",
                                    nfetchedRecords );

                    // Then send measurement records
                    for ( long i=0; i<nfetchedRecords; i++ ) {
                        mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"+;GT;%d;%d;%f",
                                    i, pRecords[i].timestamp, pRecords[i].measurement );
                    }

                    // Last send end post with number if records
                    mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"+;GT;END;%d",
                                    nfetchedRecords );

                    // Deallocate storage
                    delete[] pRecords;

                }

            }
            else {
                if ( 0 == nRecords ) {
                    mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_TABLE_NO_DATA,
									WEBSOCK_STR_ERROR_TABLE_NO_DATA );
			        wxString strErr = 
                         wxString::Format( _("[Websocket] No data in table. [name=%s]\n"), (const char *)tblName.c_str() );					
	                pCtrlObject->logMsg ( strErr, DAEMON_LOGMSG_INFO, DAEMON_LOGTYPE_GENERAL );
                    return MG_TRUE;
                }
                else {
                    mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_TABLE_ERROR_READING,
									WEBSOCK_STR_ERROR_TABLE_ERROR_READING );
			        wxString strErr = 
                         wxString::Format( _("[Websocket] Problem when reading table. [name=%s]\n"), (const char *)tblName.c_str() );					
	                pCtrlObject->logMsg ( strErr, DAEMON_LOGMSG_ERROR, DAEMON_LOGTYPE_GENERAL );
                    return MG_TRUE;
                }
            }

        }

    }
	else {
        mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_UNKNOWN_COMMAND, 
									WEBSOCK_STR_ERROR_UNKNOWN_COMMAND );
    }


	return rv;

}




///////////////////////////////////////////////////////////////////////////////
// websock_sendevent
//

int
VSCPWebServerThread::websock_sendevent( struct mg_connection *conn, 
										struct websock_session *pSession,
										vscpEvent *pEvent )
{
    bool bSent = false;
    bool rv = true;

	// Check pointer
    if (NULL == conn) return MG_FALSE;
	if (NULL == pSession) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

    // Level II events betwen 512-1023 is recognized by the daemon and 
    // sent to the correct interface as Level I events if the interface  
    // is addressed by the client.
    if ((pEvent->vscp_class <= 1023) &&
            (pEvent->vscp_class >= 512) &&
            (pEvent->sizeData >= 16)) {

        // This event shold be sent to the correct interface if it is
        // available on this machine. If not it should be sent to 
        // the rest of the network as normal

        cguid destguid;
        destguid.getFromArray(pEvent->pdata);
        destguid.setAt(0,0);
        destguid.setAt(1,0);
        //unsigned char destGUID[16];
        //memcpy(destGUID, pEvent->pdata, 16); // get destination GUID
        //destGUID[0] = 0; // Interface GUID's have LSB bytes nilled
        //destGUID[1] = 0;

        pObject->m_wxClientMutex.Lock();

        // Find client
        CClientItem *pDestClientItem = NULL;
        VSCPCLIENTLIST::iterator iter;
        for (iter = pObject->m_clientList.m_clientItemList.begin();
                iter != pObject->m_clientList.m_clientItemList.end();
                ++iter) {

            CClientItem *pItem = *iter;
            if ( pItem->m_guid == destguid ) {
                // Found
                pDestClientItem = pItem;
                break;
            }

        }

        if (NULL != pDestClientItem) {

            // If the client queue is full for this client then the
            // client will not receive the message
            if (pDestClientItem->m_clientInputQueue.GetCount() <=
                        pObject->m_maxItemsInClientReceiveQueue) {

                // Create copy of event
                vscpEvent *pnewEvent = new vscpEvent;
                if (NULL != pnewEvent) {

                    vscp_copyVSCPEvent(pnewEvent, pEvent);

                    // Add the new event to the inputqueue
                    pDestClientItem->m_mutexClientInputQueue.Lock();
                    pDestClientItem->m_clientInputQueue.Append(pnewEvent);
                    pDestClientItem->m_semClientInputQueue.Post();
                    pDestClientItem->m_mutexClientInputQueue.Unlock();

                    bSent = true;
                }
                else {
                    bSent = false;
                }

            } 
            else {
                // Overun - No room for event
                //vscp_deleteVSCPevent(pEvent);
                bSent = true;
                rv = false;
            }

        } // Client found

        pObject->m_wxClientMutex.Unlock();

    }

    if (!bSent) {

        // There must be room in the send queue
        if (pObject->m_maxItemsInClientReceiveQueue >
                pObject->m_clientOutputQueue.GetCount()) {

            // Create copy of event
            vscpEvent *pnewEvent = new vscpEvent;
            if (NULL != pnewEvent) {

                vscp_copyVSCPEvent(pnewEvent, pEvent);

                pObject->m_mutexClientOutputQueue.Lock();
                pObject->m_clientOutputQueue.Append(pnewEvent);
                pObject->m_semClientOutputQueue.Post();
                pObject->m_mutexClientOutputQueue.Unlock();
            }

        } else {
            //vscp_deleteVSCPevent(pEvent);
            rv = false;
        }
    }

    return rv;
}


///////////////////////////////////////////////////////////////////////////////
// websrv_websocket_message
//

int 
VSCPWebServerThread::websrv_websocket_message( struct mg_connection *conn )
{
	wxString str;
    char buf[ 2048 ];
	struct websock_session *pSession;
    const char *p = buf;

	memset( buf, 0, sizeof( buf ) );

	// Check pointer
    if (NULL == conn) return MG_FALSE;
	if ( conn->content_len ) memcpy( buf, conn->content, MIN(conn->content_len, sizeof( buf ) ) );

	pSession = websock_get_session( conn );
	if (NULL == pSession) return MG_FALSE;

	// Keep connection alive
	if ( ( conn->wsbits & 0x0f ) == WEBSOCKET_OPCODE_PING ) {
		mg_websocket_write(conn, WEBSOCKET_OPCODE_PONG, conn->content, conn->content_len);
		return MG_TRUE;
	}
	else if ( ( conn->wsbits & 0x0f ) == WEBSOCKET_OPCODE_PONG  ) {
		//mg_websocket_write(conn, WEBSOCKET_OPCODE_PING, conn->content, conn->content_len);
		return MG_TRUE;
	}

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

    memset(buf, 0, sizeof( buf));
	memcpy(buf, (char *)conn->content, MIN( conn->content_len, sizeof(buf) ) );

    switch (*p) {

		// Command - | 'C' | command type (byte) | data |
		case 'C':
			p++;
			p++; // Point beyond initial info "C;"
			str = wxString::FromAscii( p );
			websock_command( conn, pSession, str );
			break;

        // Event | 'E' ; head(byte) , vscp_class(unsigned short) , vscp_type(unsigned
        //					short) , GUID(16*byte), data(0-487 bytes) |
		case 'E':
		{
			// Must be authorized to do this
			if ( !pSession->bAuthenticated ) {
				mg_websocket_printf( conn, 
											WEBSOCKET_OPCODE_TEXT, 
											"-;%d;%s",
											WEBSOCK_ERROR_NOT_AUTHORIZED,
											WEBSOCK_STR_ERROR_NOT_AUTHORIZED );
				return MG_TRUE;
			}

			p++;
			p++; // Point beyond initial info "E;"
			vscpEvent vscp_event;
			str = wxString::FromAscii( p );
			if (vscp_setVscpEventFromString( &vscp_event, str ) ) {

                // Check if this user is allowed to send this event
                if ( !pSession->m_pClientItem->m_pUserItem->isUserAllowedToSendEvent( vscp_event.vscp_class, vscp_event.vscp_type ) ) {
                    wxString strErr = 
                        wxString::Format( _("websocket] User [%s] not allowed to send event class=%d type=%d.\n"), 
                                                (const char *)pSession->m_pClientItem->m_pUserItem->m_user.c_str(), 
                                                vscp_event.vscp_class, vscp_event.vscp_type );			
		
	                pObject->logMsg ( strErr, DAEMON_LOGMSG_INFO, DAEMON_LOGTYPE_SECURITY );
                }

				vscp_event.obid = pSession->m_pClientItem->m_clientID;
				if ( pObject->getWebServer()->websock_sendevent( conn, pSession, &vscp_event ) ) {
					mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, "+;EVENT" );
				} 
				else {
					mg_websocket_printf( conn, 
											WEBSOCKET_OPCODE_TEXT, 
											"-;%d;%s",
											WEBSOCK_ERROR_TX_BUFFER_FULL,
											WEBSOCK_STR_ERROR_TX_BUFFER_FULL );
				}
			}
		}
		break;

        // Unknow command
		default:
			break;

    }

	return MG_TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// websock_authentication
//
// client sends
//		"AUTH;user;hash"
//	where the hash is based on
//		"user:standard vscp password hash:server generated hash(sid)"
//	"user;hash" is reeived in strKey

int
VSCPWebServerThread::websock_authentication( struct mg_connection *conn, 
											struct websock_session *pSession, 
											wxString& strUser, 
											wxString& strKey )
{
	int rv = MG_FALSE;
	char response[32 + 1];
	char expected_response[32 + 1];
	bool bValidHost = false;

	memset( response, 0, sizeof( response ) );
	memset( expected_response, 0, sizeof( expected_response ) );

	// Check pointer
    if (NULL == conn) return MG_FALSE;
	if (NULL == pSession) return MG_FALSE;
	
	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

	if ( pObject->m_bAuthWebsockets ) {

		// Check if user is valid			
		CUserItem *pUser = pObject->m_userList.getUser( strUser );
		if ( NULL == pUser ) return MG_FALSE;

		// Check if remote ip is valid
		bValidHost = pUser->isAllowedToConnect( wxString::FromAscii( conn->remote_ip ) );
		if (!bValidHost) {
            // Log valid login
            wxString strErr = 
            wxString::Format( _("[Websocket Client] Host [%s] NOT allowed to connect.\n"),
											(const char *)wxString::FromAscii( (const char *)conn->remote_ip ).c_str() );

	        pObject->logMsg ( strErr, DAEMON_LOGMSG_WARNING, DAEMON_LOGTYPE_SECURITY );
            return MG_FALSE;
        }

		strncpy( response, strKey.mb_str(), MIN( sizeof(response), strKey.Length() ) );

		mg_md5( expected_response,
					(const char *)strUser.mb_str(), ":",
					(const char *)pUser->m_md5Password.mb_str(), ":",
					pSession->m_sid,
					NULL );

		rv = ( vscp_strcasecmp( response, expected_response ) == 0 ) ? MG_TRUE : MG_FALSE;

        if (  MG_TRUE == rv ) {

            pSession->m_pClientItem->m_bAuthorized = true;

            // Add user to client
            pSession->m_pClientItem->m_pUserItem = pUser;

            // Copy in the user filter
            memcpy( &pSession->m_pClientItem->m_filterVSCP, 
				        &pUser->m_filterVSCP, 
				        sizeof( vscpEventFilter ) );

            // Log valid login
            wxString strErr = 
                        wxString::Format( _("[Websocket Client] Host [%s] User [%s] allowed to connect.\n"), 
                                                 (const char *)wxString::FromAscii( (const char *)conn->remote_ip ).c_str(), 
                                                 (const char *)strUser.mbc_str() );			
		
	        pObject->logMsg ( strErr, DAEMON_LOGMSG_INFO, DAEMON_LOGTYPE_SECURITY );

        }
        else {
            // Log valid login
            wxString strErr = 
            wxString::Format( _("[Websocket Client] user [%s] NOT allowed to connect.\n"), 
                                             (const char *)strUser.c_str() );			
			
	        pObject->logMsg ( strErr, DAEMON_LOGMSG_WARNING, DAEMON_LOGTYPE_SECURITY );
        }
	}

	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// websock_new_session
//

websock_session *
VSCPWebServerThread::websock_new_session( struct mg_connection *conn, const char * pKey, const char * pVer )
{
	char buf[512];
	struct websock_session *ret;

	// Check pointer
    if (NULL == conn) return NULL;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return NULL;

	// create fresh session 
    ret = (struct websock_session *)calloc(1, sizeof(struct websock_session));
    if  (NULL == ret ) {
#ifndef WIN32
        syslog(LOG_ERR, "calloc error: %s\n", strerror(errno));
#endif
        return NULL;
    }

	memset( ret->m_sid, 0, sizeof( ret->m_sid ) );
	memset( ret->m_key, 0, sizeof( ret->m_key ) );

	// Generate a random session ID
    time_t t;
    t = time( NULL );
    sprintf( buf,
				"__%s_%X%X%X%X_be_hungry_stay_foolish_%X%X",
				pKey,
				(unsigned int)rand(),
				(unsigned int)rand(),
				(unsigned int)rand(),
				(unsigned int)t,
				(unsigned int)rand(), 
				1337 );

	Cmd5 md5( (unsigned char *)buf );
	strcpy( ret->m_sid, md5.getDigest() );
	
	// Init
	strcpy( ret->m_key, pKey );				// Save key
	ret->bAuthenticated = false;			// Not authenticated in yet
	ret->m_version = atoi( pVer );			// Store protocol version
    ret->m_pClientItem = new CClientItem();	// Create client        
    vscp_clearVSCPFilter(&ret->m_pClientItem->m_filterVSCP);    // Clear filter
	ret->bTrigger = false;
	ret->triggerTimeout = 0;

	// This is an active client
    ret->m_pClientItem->m_bOpen = false;
    ret->m_pClientItem->m_type = CLIENT_ITEM_INTERFACE_TYPE_CLIENT_WEBSOCKET;
    ret->m_pClientItem->m_strDeviceName = _("Internal daemon websocket client. Started at ");
    wxDateTime now = wxDateTime::Now();
    ret->m_pClientItem->m_strDeviceName += now.FormatISODate();
    ret->m_pClientItem->m_strDeviceName += _(" ");
    ret->m_pClientItem->m_strDeviceName += now.FormatISOTime();

	// Add the client to the Client List
    pObject->m_wxClientMutex.Lock();
    pObject->addClient(ret->m_pClientItem);
    pObject->m_wxClientMutex.Unlock();

    // Add to linked list
    ret->m_referenceCount++;
    ret->lastActiveTime = time(NULL);
    ret->m_next = websock_sessions;
	websock_sessions = ret;

    return ret;
}


///////////////////////////////////////////////////////////////////////////////
// websock_get_session
//

struct websock_session *
VSCPWebServerThread::websock_get_session( struct mg_connection *conn )
{
    struct websock_session *ret = NULL;

	// Check pointer
	if (NULL == conn) return NULL;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return NULL;

	// Get the session key
	const char *pKey = mg_get_header( conn, "sec-websocket-key" ); 
	if ( NULL == pKey) return NULL;

	// Get protocol version
	const char *pVer = mg_get_header( conn, "sec-websocket-version" ); 
	if ( NULL == pVer) return NULL;
        
    // find existing session 
    ret = websock_sessions;
    while (NULL != ret) {
		if ( 0 == strcmp( pKey, ret->m_key ) ) {
			break;
		}
        ret = ret->m_next;
	}
        
	if ( NULL != ret ) {
		ret->m_referenceCount++;
		ret->lastActiveTime = time(NULL);
        return ret;
    }
     
	// Return new session
    return pObject->getWebServer()->websock_new_session( conn, pKey, pVer );
}


///////////////////////////////////////////////////////////////////////////////
// websock_expire_sessions
//

void
VSCPWebServerThread::websock_expire_sessions( struct mg_connection *conn  )
{
    struct websock_session *pos;
    struct websock_session *prev;
    struct websock_session *next;
    time_t now;

	// Check pointer
	if (NULL == conn) return;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return;

    now = time( NULL );
    prev = NULL;
    pos = websock_sessions;
    
    while (NULL != pos) {
        
        next = pos->m_next;
        
        if ( ( now - pos->lastActiveTime ) > ( 60 * 60 ) ) {
        
            // expire sessions after 1h 
            if ( NULL == prev ) {
                websock_sessions = pos->m_next;
            }
            else {
                prev->m_next = next;
            }
            
			pObject->m_wxClientMutex.Lock();
			pObject->removeClient( pos->m_pClientItem );
			pObject->m_wxClientMutex.Unlock();

			// Free session data
            free(pos);
            
        } 
        else {
            prev = pos;
        }
        
        pos = next;
    }
}


///////////////////////////////////////////////////////////////////////////////
// websock_post_incomingEvent
//

void
VSCPWebServerThread::websock_post_incomingEvents( void )
{
	struct mg_connection *conn;

	// Iterate over all connections, and push current time message to websocket ones.
	for ( conn = mg_next( m_pCtrlObject->m_pwebserver, NULL); 
               conn != NULL; 
               conn = mg_next(m_pCtrlObject->m_pwebserver, conn)) {
		
		if ( conn->is_websocket ) {

			websock_session *pSession = websock_get_session( conn );
			if ( NULL == pSession) continue;

			if ( pSession->m_pClientItem->m_bOpen &&
					pSession->m_pClientItem->m_clientInputQueue.GetCount()) {

				CLIENTEVENTLIST::compatibility_iterator nodeClient;
				vscpEvent *pEvent;

				pSession->m_pClientItem->m_mutexClientInputQueue.Lock();
				nodeClient = pSession->m_pClientItem->m_clientInputQueue.GetFirst();
				pEvent = nodeClient->GetData();
				pSession->m_pClientItem->m_clientInputQueue.DeleteNode(nodeClient);
				pSession->m_pClientItem->m_mutexClientInputQueue.Unlock();

				if (NULL != pEvent) {

					if (vscp_doLevel2Filter(pEvent, &pSession->m_pClientItem->m_filterVSCP)) {

						wxString str;
						if (vscp_writeVscpEventToString(pEvent, str)) {

							// Write it out
							char buf[ 512 ];
							memset((char *) buf, 0, sizeof( buf));
							strcpy((char *) buf, (const char*) "E;");
							strcat((char *) buf, (const char*) str.mb_str(wxConvUTF8));
							mg_websocket_write(conn, 1, buf, strlen(buf) );
							
						}
					}

					// Remove the event
					vscp_deleteVSCPevent(pEvent);

				} // Valid pEvent pointer
			} // events available
		} // websocket
	} // for
}
	








///////////////////////////////////////////////////////////////////////////////
//                              WEB SERVER
///////////////////////////////////////////////////////////////////////////////









///////////////////////////////////////////////////////////////////////////////
//                     WEBSERVER SESSION HANDLINO
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// websrv_get_session
//

struct websrv_Session *
VSCPWebServerThread::websrv_get_session( struct mg_connection *conn )
{
	char buf[512];
    struct websrv_Session *ret = NULL;
    const char *cookie = NULL;

	// Get the session cookie
	const char *pheader = mg_get_header( conn, "cookie" ); 
	if ( NULL == pheader) return NULL;

	if ( MG_FALSE == mg_parse_header( pheader, "session", buf, sizeof( buf ) ) ) return NULL;
	cookie = buf;
	
    if (cookie != NULL) {
        
        // find existing session 
        ret = websrv_sessions;
        while (NULL != ret) {
            if (0 == strcmp(cookie, ret->m_sid))
                break;
            ret = ret->m_next;
        }
        
        if (NULL != ret) {
            ret->m_referenceCount++;
			ret->lastActiveTime = time( NULL );
            return ret;
        }
    }
        
    return ret;
}


///////////////////////////////////////////////////////////////////////////////
// websrv_add_session_cookie
//

websrv_Session *
VSCPWebServerThread::websrv_add_session_cookie( struct mg_connection *conn, const char *pUser )
{
	char buf[512];
	struct websrv_Session *ret;

	// Check pointer
	if (NULL == conn) return NULL;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return NULL;

	// create fresh session 
    ret = (struct websrv_Session *)calloc(1, sizeof(struct websrv_Session));
    if  (NULL == ret ) {
#ifndef WIN32
        syslog(LOG_ERR, "calloc error: %s\n", strerror(errno));
#endif
        return NULL;
    }

	// Generate a random session ID
    time_t t;
    t = time( NULL );
    sprintf( buf,
				"__VSCP__DAEMON_%X%X%X%X_be_hungry_stay_foolish_%X%X",
				(unsigned int)rand(),
				(unsigned int)rand(),
				(unsigned int)rand(),
				(unsigned int)t,
				(unsigned int)rand(), 
				1337 );

	Cmd5 md5( (unsigned char *)buf );
	strcpy( ret->m_sid, md5.getDigest() );
	
	sprintf( buf, "session=%s; max-age=3600; http-only", ret->m_sid );
	mg_send_header( conn, "Set-Cookie", buf );
	
	sprintf( buf, "user=%s", pUser );
	mg_send_header( conn, "Set-Cookie", buf );

	strcpy( buf, "original_url=/; max-age=0" );
	mg_send_header( conn, "Set-Cookie", buf );

	ret->m_pUserItem = pObject->m_userList.getUser( wxString::FromAscii( pUser ) );
	
    // Add to linked list
    ret->m_referenceCount++;
    ret->lastActiveTime = time(NULL);
    ret->m_next = websrv_sessions;
	websrv_sessions = ret;

    return ret;
}

///////////////////////////////////////////////////////////////////////////////
// websrv_GetCreateSession
//

struct websrv_Session * 
VSCPWebServerThread::websrv_GetCreateSession( struct mg_connection *conn )
{
	const char *hdr;
	char user[256];
	struct websrv_Session *rv = NULL;

	// Check pointer
	if (NULL == conn) return NULL;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return NULL;

	if ( NULL == ( rv =  pObject->getWebServer()->websrv_get_session( conn ) ) ) {

		if ( NULL == ( hdr = mg_get_header( conn, "Authorization") ) ||
						( vscp_strncasecmp( hdr, "Digest ", 7 ) != 0 ) ) {
			return NULL;
		}
				
		if (!mg_parse_header(hdr, "username", user, sizeof(user))) {
			return NULL;
		}

		// Add session cookie
		rv = pObject->getWebServer()->websrv_add_session_cookie( conn, user );
	}

	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// websrv_expire_sessions
//

void
VSCPWebServerThread::websrv_expire_sessions( struct mg_connection *conn )
{
    struct websrv_Session *pos;
    struct websrv_Session *prev;
    struct websrv_Session *next;
    time_t now;

    now = time( NULL );
    prev = NULL;
    pos = websrv_sessions;
    
    while (NULL != pos) {
        
        next = pos->m_next;
        
        if (now - pos->lastActiveTime > 60 * 60) {
        
            // expire sessions after 1h 
            if ( NULL == prev ) {
                websrv_sessions = pos->m_next;
            }
            else {
                prev->m_next = next;
            }
            
            free(pos);
            
        } 
        else {
            prev = pos;
        }
        
        pos = next;
    }
}


///////////////////////////////////////////////////////////////////////////////
// websrv_check_password
//
// http://en.wikipedia.org/wiki/Digest_access_authentication
//

int 
VSCPWebServerThread::websrv_check_password( const char *method, 
											const char *ha1, 
											const char *uri,
											const char *nonce, 
											const char *nc, 
											const char *cnonce,
											const char *qop, 
											const char *response )
{
	char ha2[32 + 1], expected_response[32 + 1];

#if 0
  // Check for authentication timeout
  if ((unsigned long) time(NULL) - (unsigned long) to64(nonce) > 3600 * 2) {
    return 0;
  }
#endif

	mg_md5(ha2, method, ":", uri, NULL);
	mg_md5(expected_response, ha1, ":", nonce, ":", nc,
					":", cnonce, ":", qop, ":", ha2, NULL);

	return ( vscp_strcasecmp( response, expected_response ) == 0 ) ? MG_TRUE : MG_FALSE;

}




///////////////////////////////////////////////////////////////////////////////
// websrv_event_handler
//

int 
VSCPWebServerThread::websrv_event_handler( struct mg_connection *conn, enum mg_event ev )
{
	static time_t cleanupTime = time(NULL);
	const char *hdr;
    wxString strErr;
	struct websock_session *pWebSockSession;
	struct websrv_Session * pWebSrvSession;
	char user[256], nonce[256],
			uri[32768], cnonce[256], resp[256], qop[256], nc[256];
	CUserItem *pUser;
	bool bValidHost;

	// Check pointer
	if (NULL == conn) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

	switch (ev) {

		case MG_AUTH: 
			
			if (conn->is_websocket) {	
				return MG_TRUE;	// Always accept websocket connections
			}
			
			// Validate REST interface user.
			if ( 0 == strncmp(conn->uri, "/vscp/rest",10 ) ) {
				return MG_TRUE;	 // Always accept websocket connections
			}

            // Check if a session is available. If so everything is OK.
            if ( NULL != ( pWebSrvSession = pObject->getWebServer()->websrv_GetCreateSession( conn ) ) ) return MG_TRUE;

			if ( NULL == ( hdr = mg_get_header( conn, "Authorization") ) ||
								( vscp_strncasecmp( hdr, "Digest ", 7 ) != 0 ) ) {
				return MG_FALSE;
			}
			if (!mg_parse_header(hdr, "username", user, sizeof(user))) return MG_FALSE;
			if (!mg_parse_header(hdr, "cnonce", cnonce, sizeof(cnonce))) return MG_FALSE;
			if (!mg_parse_header(hdr, "response", resp, sizeof(resp))) return MG_FALSE;
			if (!mg_parse_header(hdr, "uri", uri, sizeof(uri))) return MG_FALSE;
			if (!mg_parse_header(hdr, "qop", qop, sizeof(qop))) return MG_FALSE;
			if (!mg_parse_header(hdr, "nc", nc, sizeof(nc))) return MG_FALSE;
			if (!mg_parse_header(hdr, "nonce", nonce, sizeof(nonce))) return MG_FALSE;

			// Check if user is vali			
			pUser = pObject->m_userList.getUser( wxString::FromAscii( user ) );
			if ( NULL == pUser ) return MG_FALSE;

			// Check if remote ip is valid
			pObject->m_mutexUserList.Lock();
			bValidHost = pUser->isAllowedToConnect( wxString::FromAscii( conn->remote_ip ) );
			pObject->m_mutexUserList.Unlock();
			if (!bValidHost) {
                // Host wrong
                strErr = 
                        wxString::Format( _("[Webserver Client] Host [%s] NOT allowed to connect. User [%s]\n"), 
                                                 (const char *)wxString::FromAscii( (const char *)conn->remote_ip ).c_str(), 
                                                 (const char *)pUser->m_user.c_str() );				
			
                pObject->logMsg ( strErr, DAEMON_LOGMSG_WARNING, DAEMON_LOGTYPE_SECURITY );
                return MG_FALSE;
            }

			if ( MG_TRUE != 
				pObject->getWebServer()->websrv_check_password( conn->request_method, 
																	(const char *)pUser->m_md5Password.mb_str(), 
																	uri, nonce, nc, cnonce, qop, resp ) ) {
                    // Username/password wrong
                    strErr = 
                        wxString::Format( _("[Webserver Client] Host [%s] User [%s] NOT allowed to connect.\n"), 
                                                 (const char *)wxString::FromAscii((const char *)conn->remote_ip ).c_str(), 
                                                 (const char *)pUser->m_user.c_str() );
	                pObject->logMsg ( strErr, DAEMON_LOGMSG_WARNING, DAEMON_LOGTYPE_SECURITY );                                                                        
				    return MG_FALSE;
            }

			pObject->getWebServer()->websrv_add_session_cookie( conn, user );


            // Valid credentials
            strErr = 
                    wxString::Format( _("[Webserver Client] Host [%s] User [%s] allowed to connect.\n"), 
                                        (const char *)wxString::FromAscii( (const char *)conn->remote_ip ).c_str(), 
                                        (const char *)pUser->m_user.c_str() );			
			
	        pObject->logMsg ( strErr, DAEMON_LOGMSG_INFO, DAEMON_LOGTYPE_SECURITY ); 
            
			return MG_TRUE;

		case MG_REQUEST:

			if (conn->is_websocket) {
				return pObject->getWebServer()->websrv_websocket_message( conn );
			}
			else {

                // Log access
                strErr = 
                wxString::Format( _("Webserver: Host=[%s] - req=[%s] query=[%s] method=[%s] \n"), 
                                (const char *)wxString::FromAscii((const char *)conn->remote_ip ).c_str(),
                                (const char *)wxString::FromAscii((const char *)conn->uri).c_str(), 
                                (const char *)wxString::FromAscii((const char *)conn->query_string).c_str(), 
                                (const char *)wxString::FromAscii((const char *)conn->request_method).c_str() );						
	            pObject->logMsg ( strErr, DAEMON_LOGMSG_INFO, DAEMON_LOGTYPE_ACCESS );

				if ( 0 == strcmp(conn->uri, "/vscp") ) {
					if ( NULL == ( pWebSrvSession = pObject->getWebServer()->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					mg_send_header(conn, "Content-Type", "text/html");
					mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
											"pre-check=0, no-store, no-cache, must-revalidate");
					return pObject->getWebServer()->websrv_mainpage( conn );
				}
				else if ( 0 == strcmp(conn->uri, "/test") ) {
					if ( NULL == ( pWebSrvSession = pObject->getWebServer()->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					mg_send_header(conn, "Content-Type", "text/html");
					mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
											"pre-check=0, no-store, no-cache, must-revalidate");
					mg_send_data( conn, html_form, strlen(html_form) );
					return MG_TRUE;
				}
				else if ( 0 == strcmp(conn->uri, "/vscp/interfaces") ) {
					if ( NULL == ( pWebSrvSession = pObject->getWebServer()->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					mg_send_header(conn, "Content-Type", "text/html");
					mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
											"pre-check=0, no-store, no-cache, must-revalidate");
					return pObject->getWebServer()->websrv_interfaces( conn );
				}
				else if ( 0 == strcmp(conn->uri, "/vscp/dm") ) {
					if ( NULL == ( pWebSrvSession = pObject->getWebServer()->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					mg_send_header(conn, "Content-Type", "text/html");
					mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
											"pre-check=0, no-store, no-cache, must-revalidate");
					return pObject->getWebServer()->websrv_dmlist( conn );
				}
				else if ( 0 == strcmp(conn->uri, "/vscp/dmedit") ) {
					if ( NULL == ( pWebSrvSession = pObject->getWebServer()->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					mg_send_header(conn, "Content-Type", "text/html");
					mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
											"pre-check=0, no-store, no-cache, must-revalidate");
					return pObject->getWebServer()->websrv_dmedit( conn );
				}
				else if ( 0 == strcmp(conn->uri, "/vscp/dmpost") ) {
					if ( NULL == ( pWebSrvSession = pObject->getWebServer()->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					mg_send_header(conn, "Content-Type", "text/html");
					mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
											"pre-check=0, no-store, no-cache, must-revalidate");
					return pObject->getWebServer()->websrv_dmpost( conn );
				}
				else if ( 0 == strcmp(conn->uri, "/vscp/dmdelete") ) {
					if ( NULL == ( pWebSrvSession = pObject->getWebServer()->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					mg_send_header(conn, "Content-Type", "text/html");
					mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
											"pre-check=0, no-store, no-cache, must-revalidate");
					return pObject->getWebServer()->websrv_dmdelete( conn );
				}
				else if ( 0 == strcmp(conn->uri, "/vscp/variables") ) {
					if ( NULL == ( pWebSrvSession = pObject->getWebServer()->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					mg_send_header(conn, "Content-Type", "text/html");
					mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
											"pre-check=0, no-store, no-cache, must-revalidate");
					return pObject->getWebServer()->websrv_variables_list( conn );
				}
				else if ( 0 == strcmp(conn->uri, "/vscp/varedit") ) {
					if ( NULL == ( pWebSrvSession = pObject->getWebServer()->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					mg_send_header(conn, "Content-Type", "text/html");
					mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
											"pre-check=0, no-store, no-cache, must-revalidate");
					return pObject->getWebServer()->websrv_variables_edit( conn );
				}
				else if ( 0 == strcmp(conn->uri, "/vscp/varpost") ) {
					if ( NULL == ( pWebSrvSession = pObject->getWebServer()->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					mg_send_header(conn, "Content-Type", "text/html");
					mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
											"pre-check=0, no-store, no-cache, must-revalidate");
					return pObject->getWebServer()->websrv_variables_post( conn );
				}
				else if ( 0 == strcmp(conn->uri, "/vscp/vardelete") ) {
					if ( NULL == ( pWebSrvSession = pObject->getWebServer()->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					mg_send_header(conn, "Content-Type", "text/html");
					mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
											"pre-check=0, no-store, no-cache, must-revalidate");
					return pObject->getWebServer()->websrv_variables_delete( conn );
				}
				else if ( 0 == strcmp(conn->uri, "/vscp/varnew") ) {
					if ( NULL == ( pWebSrvSession = pObject->getWebServer()->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					mg_send_header(conn, "Content-Type", "text/html");
					mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
											"pre-check=0, no-store, no-cache, must-revalidate");
					return pObject->getWebServer()->websrv_variables_new( conn );
				}
				else if ( 0 == strcmp(conn->uri, "/vscp/discovery") ) {
					if ( NULL == ( pWebSrvSession = pObject->getWebServer()->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					mg_send_header(conn, "Content-Type", "text/html");
					mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
											"pre-check=0, no-store, no-cache, must-revalidate");
					return pObject->getWebServer()->websrv_discovery( conn );
				}
				else if ( 0 == strcmp(conn->uri, "/vscp/session") ) {
					if ( NULL == ( pWebSrvSession = pObject->getWebServer()->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					mg_send_header(conn, "Content-Type", "text/html");
					mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
											"pre-check=0, no-store, no-cache, must-revalidate");
					return pObject->getWebServer()->websrv_session( conn );
				}
				else if ( 0 == strcmp(conn->uri, "/vscp/configure") ) {
					if ( NULL == ( pWebSrvSession = pObject->getWebServer()->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					mg_send_header(conn, "Content-Type", "text/html");
					mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
											"pre-check=0, no-store, no-cache, must-revalidate");
					return pObject->getWebServer()->websrv_configure( conn );
				}
				else if ( 0 == strcmp(conn->uri, "/vscp/bootload") ) {
					if ( NULL == ( pWebSrvSession = pObject->getWebServer()->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					mg_send_header(conn, "Content-Type", "text/html");
					mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
											"pre-check=0, no-store, no-cache, must-revalidate");
					return pObject->getWebServer()->websrv_bootload( conn );
				}
                else if ( 0 == strncmp(conn->uri, "/vscp/log/general",17) ) {
					if ( NULL == ( pWebSrvSession = pObject->getWebServer()->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
                    if ( NULL == ( pWebSrvSession = pObject->getWebServer()->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					wxString header = _("Log File 'General'");
					return pObject->getWebServer()->websrv_listFile( conn, 
                                                            pObject->m_logGeneralFileName, 
                                                            header );
				}
                else if ( 0 == strncmp(conn->uri, "/vscp/log/security",18) ) {   
					if ( NULL == ( pWebSrvSession = pObject->getWebServer()->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					wxString header = _("Log File 'Security'");
					return pObject->getWebServer()->websrv_listFile( conn, 
                                                            pObject->m_logSecurityFileName, 
                                                            header );
				}
                else if ( 0 == strncmp(conn->uri, "/vscp/log/access",16) ) {
                    if ( NULL == ( pWebSrvSession = pObject->getWebServer()->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					wxString header = _("Log File 'Security'");
					return pObject->getWebServer()->websrv_listFile( conn, 
                                                            pObject->m_logAccessFileName, 
                                                            header );
				}
                else if ( 0 == strncmp(conn->uri, "/vscp/log/dm",12) ) {
					if ( NULL == ( pWebSrvSession = pObject->getWebServer()->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					wxString header = _("Log File 'Decision Matrix'");
					return pObject->getWebServer()->websrv_listFile( conn, 
                                                            pObject->m_dm.m_logFileName, 
                                                            header );
				}
                else if ( 0 == strncmp(conn->uri, "/vscp/table",12) ) {
					if ( NULL == ( pWebSrvSession = pObject->getWebServer()->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					return pObject->getWebServer()->websrv_table( conn );
				}


                else if ( 0 == strncmp(conn->uri, "/vscp/tablelist",12) ) {
					if ( NULL == ( pWebSrvSession = pObject->getWebServer()->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					return pObject->getWebServer()->websrv_tablelist( conn );
				}
                
				else if ( 0 == strncmp(conn->uri, "/vscp/rest",10) ) {
					return pObject->getWebServer()->websrv_restapi( conn );
				}
                /*
				else {
                    wxString path = pObject->m_pathRoot + wxString::FromAscii( conn->uri );
                    mg_send_file( conn, "index.html" );
					return MG_TRUE;
				}*/
			}
			return MG_FALSE;

		case MG_WS_HANDSHAKE:
			if ( NULL != ( hdr = mg_get_header( conn, "Sec-WebSocket-Protocol") ) ) {
					if ( 0 == vscp_strncasecmp( hdr, "very-simple-control-protocol", 28 ) ) {
						//mg_send_header( conn, "Set-Sec-WebSocket-Protocol", "very-simple-control-protocol" );
					/*	
				Currently it is impossible to request a specific protocol
				This is what would be needed to do so.
				
				if ( NULL != ( hdr = mg_get_header( conn, "Sec-WebSocket-Protocol") ) ) {
					if ( 0 == vscp_strncasecmp( hdr, "very-simple-control-protocol", 28 ) ) {
						/*
						static const char *magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
						char buf[500], sha[20], b64_sha[sizeof(sha) * 2];
						SHA1_CTX sha_ctx;

						mg_snprintf(buf, sizeof(buf), "%s%s", key, magic);
						SHA1Init(&sha_ctx);
						SHA1Update(&sha_ctx, (unsigned char *) buf, strlen(buf));
						SHA1Final((unsigned char *) sha, &sha_ctx);
						base64_encode((unsigned char *) sha, sizeof(sha), b64_sha);
						mg_snprintf(buf, sizeof(buf), "%s%s%s",
							"HTTP/1.1 101 Switching Protocols\r\n"
							"Upgrade: websocket\r\n"
							"Connection: Upgrade\r\n"
							"Sec-WebSocket-Protocol: very-simple-control-protocol\r\n"
							"Sec-WebSocket-Accept: ", b64_sha, "\r\n\r\n");

						 mg_write(conn, buf, strlen(buf));
						 
					}		
				}*/
						return MG_FALSE;
					}
			}
			return MG_FALSE;

		case MG_WS_CONNECT:

			// New websocket connection. Send connection ID back to the client.
			if ( conn->is_websocket ) {

				// Get session
				pWebSockSession = websock_get_session( conn );
				if ( NULL == pWebSockSession ) {
					mg_websocket_printf( conn, 
											WEBSOCKET_OPCODE_TEXT, 
											"-;%d;%s",
											WEBSOCK_ERROR_NOT_AUTHORIZED,
											WEBSOCK_STR_ERROR_NOT_AUTHORIZED );
					return MG_FALSE;
				}

				if ( pObject->m_bAuthWebsockets ) {

					// Start authentication
					mg_websocket_printf( conn, 
											WEBSOCKET_OPCODE_TEXT, 
											"+;AUTH0;%s", 
											pWebSockSession->m_sid );
				}
				else {
					// No authenticateion will be performed
					
					pWebSockSession->bAuthenticated = true;	// Authenticated
					mg_websocket_printf( conn, 
											WEBSOCKET_OPCODE_TEXT, 
											"+;AUTH1" );
				}
			}
			return MG_TRUE;  // keep socket open

		case MG_REPLY:
			return MG_TRUE;

		case MG_POLL:
			if ( ( cleanupTime - time(NULL) ) > 60 ) {
				pObject->getWebServer()->websrv_expire_sessions( conn );
				pObject->getWebServer()->websock_expire_sessions( conn );
				pObject->getWebServer()->websrv_expire_rest_sessions( conn );
				cleanupTime = time(NULL);
			}
			return MG_FALSE;

		case MG_HTTP_ERROR:
            {
			    pObject->logMsg( _("WEBServer: Error \r\n"), DAEMON_LOGMSG_DEBUG );
                //mg_send_status( conn, 304);
                //mg_send_header(conn, "Content-Type", "text/html");
            }
			return MG_FALSE;

		case MG_CLOSE:
			if ( conn->is_websocket ) {
				//	free( conn->connection_param );
				conn->connection_param = NULL;
				pWebSockSession = websock_get_session( conn );
				if ( NULL != pWebSockSession ) {
					pWebSockSession->lastActiveTime  = 0;	// Mark as staled
					pObject->getWebServer()->websock_expire_sessions( conn );
                    return MG_TRUE;
				}
			}
			return MG_FALSE;

		default: 
			return MG_FALSE;
	}
}


///////////////////////////////////////////////////////////////////////////////
// websrv_listFile
//

int VSCPWebServerThread::websrv_listFile( struct mg_connection *conn, wxFileName& logfile, wxString& textHeader )
{
    mg_send_header(conn, "Content-Type", "text/html");
    mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
        "pre-check=0, no-store, no-cache, must-revalidate");

    bool bFirstRow = false;
    wxString buildPage;
    wxString strHeader = wxString::Format(_("VSCP - %s"), (const char *)textHeader.c_str() );
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), (const char *)strHeader.c_str() );
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);

    // Navigation menu 
    buildPage += _(WEB_COMMON_MENU);

    buildPage += wxString::Format( _("<b>%s</b><br><br>"), (const char *)textHeader.c_str() );
    buildPage += _("<b>Path</b>=<i>");
    buildPage += logfile.GetFullPath();
    buildPage += _("</i><br>");
    buildPage += _("-----------------------------------------------------------------------------------------<br>");

    uint32_t nLines = 0, startLine = 0, n2Lines = 0;
    wxULongLong fileLength = logfile.GetSize();
    double pos;
    if ( fileLength.ToDouble() > 100000 ) {
        pos = fileLength.ToDouble() - 100000;
        buildPage += wxString::Format(_("<br><span style=\"color: red;\">File has been truncated due to size. Filesize is %dMB last %dKB shown.</span><br><br>"), 
            (int)fileLength.ToDouble()/(1024*1024), 10 );
        bFirstRow = true;
    }

    if ( 0 == fileLength.ToDouble() ) {
        buildPage += _("<span style=\"color: red;\">File is empty.</span><br><br>"); 
    }

    wxFile file;
    if ( file.Open( logfile.GetFullPath() ) ) {

        wxFileInputStream input( file);
        wxTextInputStream text( input );

        // Got to beginning of file
        file.Seek( pos );

        while ( !file.Eof() ) {
            if ( !bFirstRow ) {
                buildPage += _("&nbsp;&nbsp;&nbsp;") + text.ReadLine()  + _("<br>"); 
            }
            else {
                bFirstRow = false;
                text.ReadLine();    // First row looks ugly due to positioning
            }
        }

        file.Close();

    }
    else {
        buildPage += _("Error: Unable to open file<br><br>");
    }

    // Serve data
    buildPage += _("&nbsp;&nbsp;&nbsp;<strong>The End</strong>");
    buildPage += _(WEB_COMMON_END);
    mg_send_data( conn, buildPage.ToAscii(), buildPage.Length() );

    return MG_TRUE;
}








//-----------------------------------------------------------------------------
//                                   REST
//-----------------------------------------------------------------------------









///////////////////////////////////////////////////////////////////////////////
// websrv_get_rest_session
//

struct websrv_rest_session *
VSCPWebServerThread::websrv_new_rest_session( struct mg_connection *conn,
												CUserItem *pUser )
{
	char buf[2048];
	struct websrv_rest_session *ret = NULL;

	// Check pointer
    if (NULL == conn) return NULL;
	
	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return NULL;

	// create fresh session 
    ret = (struct websrv_rest_session *)calloc(1, sizeof(struct websrv_rest_session));
    if  (NULL == ret ) {
#ifndef WIN32
        syslog(LOG_ERR, "calloc error: %s\n", strerror(errno));
#endif
        return NULL;
    }

	// Generate a random session ID
    time_t t;
    t = time( NULL );
    sprintf( buf,
				"__VSCP__DAEMON_REST%X%X%X%X_be_hungry_stay_foolish_%X%X",
				(unsigned int)rand(),
				(unsigned int)rand(),
				(unsigned int)rand(),
				(unsigned int)t,
				(unsigned int)rand(), 
				1337 );

	Cmd5 md5( (unsigned char *)buf );
	strcpy( ret->sid, md5.getDigest() );

	// New client
	ret->pClientItem = new CClientItem();	// Create client        
    vscp_clearVSCPFilter(&ret->pClientItem->m_filterVSCP); // Clear filter
    ret->pClientItem->m_type = CLIENT_ITEM_INTERFACE_TYPE_CLIENT_REST;
    ret->pClientItem->m_strDeviceName = _("REST client. Started at ");
    wxDateTime now = wxDateTime::Now();
    ret->pClientItem->m_strDeviceName += now.FormatISODate();
    ret->pClientItem->m_strDeviceName += _(" ");
    ret->pClientItem->m_strDeviceName += now.FormatISOTime();
	ret->pClientItem->m_bOpen = true;		// Open client

	// Add the client to the Client List
    pObject->m_wxClientMutex.Lock();
    pObject->addClient(ret->pClientItem);
    pObject->m_wxClientMutex.Unlock();

	ret->pUserItem = pUser;
	
    // Add to linked list
    ret->lastActiveTime = time(NULL);
    ret->m_next = websrv_rest_sessions;
	websrv_rest_sessions = ret;

	return ret;
}


///////////////////////////////////////////////////////////////////////////////
// websrv_get_rest_session
//

struct websrv_rest_session *
VSCPWebServerThread::websrv_get_rest_session( struct mg_connection *conn,
												wxString &SessionId )
{
	struct websrv_rest_session *ret = NULL;

	// Check pointers
	if ( NULL == conn) return NULL;
        
	// find existing session 
	ret = websrv_rest_sessions;
    while (NULL != ret) {
		if  (0 == strcmp( SessionId.mb_str(), ret->sid) )  break;
        ret = ret->m_next;
	}
        
	if (NULL != ret) {
		ret->lastActiveTime = time( NULL );
        return ret;
	}
    
	return ret;
}

void
VSCPWebServerThread::websrv_expire_rest_sessions( struct mg_connection *conn )
{
	struct websrv_rest_session *pos;
    struct websrv_rest_session *prev;
    struct websrv_rest_session *next;
    time_t now;

	// Check pointer
    if (NULL == conn) return;
	
	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return;

    now = time( NULL );
    prev = NULL;
    pos = websrv_rest_sessions;
    
    while (NULL != pos) {
        
        next = pos->m_next;
        
        if (now - pos->lastActiveTime > 10 * 60) {	// Ten minutes
        
            // expire sessions after 1h 
            if ( NULL == prev ) {
                websrv_rest_sessions = pos->m_next;
            }
            else {
                prev->m_next = next;
            }
            
			pObject->m_wxClientMutex.Lock();
			pObject->removeClient( pos->pClientItem );
			pObject->m_wxClientMutex.Unlock();

            free( pos );
            
        } 
        else {
            prev = pos;
        }
        
        pos = next;
    }
}


// Hash with key value pairs
WX_DECLARE_STRING_HASH_MAP( wxString, hashArgs );


///////////////////////////////////////////////////////////////////////////////
// websrv_restapi
//

int
VSCPWebServerThread::websrv_restapi( struct mg_connection *conn )
{
	char buf[2048];
	char date[64];
	int rv = MG_FALSE;
	wxString str;
	time_t curtime = time(NULL);
	long format = REST_FORMAT_PLAIN;
	hashArgs keypairs;
	struct websrv_rest_session *pSession = NULL;
    CUserItem *pUser = NULL;
    wxString method;

	// Check pointer
    if (NULL == conn) return MG_FALSE;
	
    // Get method
    wxString method = wxString::FromAscii( conn->request_method );
    method.Trim();
    method.Trim( false );
    method.MakeUpper();

	// Make string with GMT time
	vscp_getTimeString( date, sizeof(date), &curtime );
    
	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

	vscp_getTimeString( date, sizeof(date), &curtime );

	
    if ( _("POST") == method ) {

        const char *p;

        // user
        p = mg_get_header( conn, "VSCP-User" );
        keypairs[_("USER")] = wxString::FromAscii( p );

        // password
        p = mg_get_header( conn, "VSCP-Password" );
        keypairs[_("PASSWORD")] = wxString::FromAscii( p );

        // session
        p = mg_get_header( conn, "VSCP-Session" );
        keypairs[_("SESSION")] = wxString::FromAscii( p );

        // format
        mg_get_var(conn, "format", buf, sizeof(buf) );
        keypairs[_("FORMAT")] = wxString::FromAscii( buf );

        // op
        mg_get_var(conn, "op", buf, sizeof(buf) );
        keypairs[_("OP")] = wxString::FromAscii( buf );

        // vscpevent
        mg_get_var(conn, "vscpevent", buf, sizeof(buf) );
        keypairs[_("VSCPEVENT")] = wxString::FromAscii( buf );

        // count
        mg_get_var(conn, "count", buf, sizeof(buf) );
        keypairs[_("COUNT")] = wxString::FromAscii( buf );

        // vscpfilter
        mg_get_var(conn, "vscpfilter", buf, sizeof(buf) );
        keypairs[_("VSCPFILTER")] = wxString::FromAscii( buf );

        // variable
        mg_get_var(conn, "variable", buf, sizeof(buf) );
        keypairs[_("VARIABLE")] = wxString::FromAscii( buf );

        // type
        mg_get_var(conn, "type", buf, sizeof(buf) );
        keypairs[_("TYPE")] = wxString::FromAscii( buf );

        // measurement
        mg_get_var(conn, "measurement", buf, sizeof(buf) );
        keypairs[_("MEASUREMENT")] = wxString::FromAscii( buf );

        // unit
        mg_get_var(conn, "unit", buf, sizeof(buf) );
        keypairs[_("UNIT")] = wxString::FromAscii( buf );

        // sensoridx
        mg_get_var(conn, "sensoridx", buf, sizeof(buf) );
        keypairs[_("SENSORIDX")] = wxString::FromAscii( buf );

        // name
        mg_get_var(conn, "name", buf, sizeof(buf) );
        keypairs[_("NAME")] = wxString::FromAscii( buf );

        // from
        mg_get_var(conn, "from", buf, sizeof(buf) );
        keypairs[_("FROM")] = wxString::FromAscii( buf );

        // to
        mg_get_var(conn, "to", buf, sizeof(buf) );
        keypairs[_("TO")] = wxString::FromAscii( buf );
    }
    else {
        // get parameters for get
	    wxStringTokenizer tkz( wxString::FromAscii( conn->query_string ), _("&") );
	    while ( tkz.HasMoreTokens() ) {
		    int pos;
		    str = tkz.GetNextToken();
		    if ( wxNOT_FOUND != ( pos = str.Find('=') ) ) {
			    keypairs[ str.Left(pos).Upper() ] = str.Right( str.Length()-pos-1 ); 
		    }
	    }
    }

    // If we have a session key we try to get the session
	if ( _("") != keypairs[_("SESSION")] ) {
		pSession = websrv_get_rest_session( conn, keypairs[_("SESSION")] );
        if ( NULL != pSession )  pUser = pSession->pUserItem;
	}

    if ( NULL == pSession ) {

        // Get user
        pUser = pObject->m_userList.getUser( keypairs[_("USER")] ); 

	    // Check if user is valid	
	    if ( NULL == pUser ) {
            wxString strErr = 
            wxString::Format( _("[REST Client] Host [%s] Invalid user [%s]\n"), 
                                (const char *)wxString::FromAscii( (const char *)conn->remote_ip ).c_str(), 
                                (const char *)keypairs[_("USER")].c_str() );
	        pObject->logMsg( strErr, DAEMON_LOGMSG_WARNING, DAEMON_LOGTYPE_SECURITY );
            return MG_FALSE;
        }

	    // Check if remote ip is valid
	    bool bValidHost;
	    pObject->m_mutexUserList.Lock();
	    bValidHost = pUser->isAllowedToConnect( wxString::FromAscii( conn->remote_ip ) );
	    pObject->m_mutexUserList.Unlock();
	    if (!bValidHost) {
            wxString strErr = 
            wxString::Format( _("[REST Client] Host [%s] NOT allowed to connect. User [%s]\n"), 
                                (const char *)wxString::FromAscii( (const char *)conn->remote_ip ).c_str(), 
                                (const char *)keypairs[_("USER")].c_str() );
	        pObject->logMsg ( strErr, DAEMON_LOGMSG_WARNING, DAEMON_LOGTYPE_SECURITY );
            return MG_FALSE;
        }

	    // Is this an authorized user?
	    wxString str3 = keypairs[_("PASSWORD")];
	    if ( keypairs[_("PASSWORD")] != pUser->m_md5Password ) {
            wxString strErr = 
            wxString::Format( _("[REST Client] User [%s] NOT allowed to connect. Client [%s]\n"), 
                                (const char *)keypairs[_("USER")].c_str(), 
                                (const char *)wxString::FromAscii( (const char *)conn->remote_ip ).c_str() );
	        pObject->logMsg ( strErr, DAEMON_LOGMSG_WARNING, DAEMON_LOGTYPE_SECURITY );
            return MG_FALSE;
        }

    }
    else {
        // check that the origin is valid
        // Check if remote ip is valid
	    bool bValidHost;
	    pObject->m_mutexUserList.Lock();
	    bValidHost = pSession->pUserItem->isAllowedToConnect( wxString::FromAscii( conn->remote_ip ) );
	    pObject->m_mutexUserList.Unlock();
	    if (!bValidHost) {
            wxString strErr = 
            wxString::Format( _("[REST Client] Host [%s] NOT allowed to connect. User [%s]\n"), 
                                (const char *)wxString::FromAscii( (const char *)conn->remote_ip ).c_str(), 
                                (const char *)keypairs[_("USER")].c_str() );
	        pObject->logMsg ( strErr, DAEMON_LOGMSG_WARNING, DAEMON_LOGTYPE_SECURITY );
            return MG_FALSE;
        }
    }

    // ------------------------------------------------------------------------
    //                      * * * User is validated * * *
    // ------------------------------------------------------------------------

    wxString strErr = 
        wxString::Format( _("[REST Client] User [%s] Host [%s] allowed to connect. \n"), 
                            (const char *)keypairs[_("USER")].c_str() , 
                            (const char *)wxString::FromAscii( (const char *)conn->remote_ip ).c_str() );
	    pObject->logMsg ( strErr, DAEMON_LOGMSG_INFO, DAEMON_LOGTYPE_SECURITY );

	// Get format
	if ( _("PLAIN") == keypairs[_("FORMAT")].Upper() ) {
		format = REST_FORMAT_PLAIN;
	}
	else if ( _("CSV") == keypairs[_("FORMAT")].Upper() ) {
		format = REST_FORMAT_CSV;
	}
	else if ( _("XML") == keypairs[_("FORMAT")].Upper() ) {
		format = REST_FORMAT_XML;
	}
	else if ( _("JSON") == keypairs[_("FORMAT")].Upper() ) {
		format = REST_FORMAT_JSON;
	}
	else if ( _("JSONP") == keypairs[_("FORMAT")].Upper() ) {
		format = REST_FORMAT_JSONP;
	}
	else if ( _("") != keypairs[_("FORMAT")].Upper() ) {
		keypairs[_("FORMAT")].ToLong( &format );
	}
	else {
		webserv_util_sendheader( conn, 400, "text/plain" );

		mg_write( conn, "\r\n", 2 );		// head/body Separator
		memset( buf, 0, sizeof( buf ) );
		webserv_util_make_chunk( buf, REST_PLAIN_ERROR_UNSUPPORTED_FORMAT, strlen( REST_PLAIN_ERROR_UNSUPPORTED_FORMAT ) );
		mg_write( conn, buf, strlen( buf ) );

		mg_write( conn, "0\r\n\r\n", 5);	// Terminator

		return MG_TRUE;
	}

	//   *************************************************************
	//   * * * * * * * *  Status (hold session open)   * * * * * * * *
	//   *************************************************************
	if ( ( _("0") == keypairs[_("OP")] ) ||  ( _("STATUS") == keypairs[_("OP")].Upper() ) ) {
		rv = webserv_rest_doStatus( conn, pSession, format );
	}

	//  ********************************************
	//  * * * * * * * * open session * * * * * * * *
	//  ********************************************
    else if ( ( _("1") == keypairs[_("OP")] ) || ( _("OPEN") == keypairs[_("OP")].Upper() ) ) {
		rv = webserv_rest_doOpen( conn, pSession, pUser, format );		
	}

	//   **********************************************
	//   * * * * * * * * close session  * * * * * * * *
	//   **********************************************
	else if ( ( _("2") == keypairs[_("OP")] ) || ( _("CLOSE") == keypairs[_("OP")].Upper() ) ) {
		rv = webserv_rest_doOpen( conn, pSession, pUser, format );
	}

	//  ********************************************
	//   * * * * * * * * Send event  * * * * * * * *
	//  ********************************************
	else if ( ( _("3") == keypairs[_("OP")] ) || ( _("SENDEVENT") == keypairs[_("OP")].Upper() ) ) {
		vscpEvent vscpevent;
		if ( _("") != keypairs[_("VSCPEVENT")] ) {
			vscp_setVscpEventFromString( &vscpevent, keypairs[_("VSCPEVENT")] ); 
			rv = webserv_rest_doSendEvent( conn, pSession, format, &vscpevent );
		}
		else {
			// Parameter missing - No Event
			webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_MISSING_DATA );
		}
	}

	//  ********************************************
	//   * * * * * * * * Read event  * * * * * * * *
	//  ********************************************
	else if ( ( _("4") == keypairs[_("OP")] ) || ( _("READEVENT") == keypairs[_("OP")].Upper() ) ) {
		long count = 1;
		if ( _("") != keypairs[_("COUNT")].Upper() ) {
			keypairs[_("COUNT")].ToLong( &count );
		}
		rv = webserv_rest_doReceiveEvent( conn, pSession, format, count );
	}

	//   **************************************************
	//   * * * * * * * * Set (read) filter  * * * * * * * *
	//   **************************************************
	else if ( ( _("5") == keypairs[_("OP")] ) || ( _("SETFILTER") == keypairs[_("OP")].Upper() ) ) {
		
		vscpEventFilter vscpfilter;
		if ( _("") != keypairs[_("VSCPFILTER")] ) {
			;
			vscp_readFilterFromString( &vscpfilter, keypairs[_("VSCPFILTER")] ); 
			rv = webserv_rest_doSetFilter( conn, pSession, format, vscpfilter );
		}
		else {
			webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_MISSING_DATA );
		}
	}

	//   ****************************************************
	//   * * * * * * * *  clear input queue   * * * * * * * *
	//   ****************************************************
	else if ( ( _("6") == keypairs[_("OP")] ) || ( _("CLEARQUEUE") == keypairs[_("OP")].Upper() ) ) {
		webserv_rest_doClearQueue( conn, pSession, format );
	}
	
	//   ****************************************************
	//   * * * * * * * * read variable value  * * * * * * * *
	//   ****************************************************
	else if ( ( _("7") == keypairs[_("OP")] ) || ( _("READVAR") == keypairs[_("OP")].Upper() ) ) {
		if ( _("") != keypairs[_("VARIABLE")] ) {
			rv = webserv_rest_doReadVariable( conn, pSession, format, keypairs[_("VARIABLE")] );
		}
		else {
			webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_MISSING_DATA );
		}
	}

	//   *****************************************************
	//   * * * * * * * * Write variable value  * * * * * * * *
	//   *****************************************************
	else if ( ( _("8") == keypairs[_("OP")] ) || ( _("WRITEVAR") == keypairs[_("OP")].Upper() ) ) {

		if ( _("") != keypairs[_("VARIABLE")] ) {
			rv = webserv_rest_doWriteVariable( conn, pSession, format, keypairs[_("VARIABLE")] );
		}
		else {
			webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_MISSING_DATA );
		}	
	}


    //   *****************************************************
	//   * * * * * * * *   Create variable    * * * * * * * *
	//   *****************************************************

	else if ( ( _("9") == keypairs[_("OP")] ) || ( _("CREATEVAR") == keypairs[_("OP")].Upper() ) ) {


		if ( _("") != keypairs[_("VARIABLE")] ) {
			rv = webserv_rest_doWriteVariable( conn, pSession, format, keypairs[_("VARIABLE")] );
		}
		else {
			webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_MISSING_DATA );
		}	
	}


	//   *************************************************
	//   * * * * * * * * Send measurement  * * * * * * * *
	//   *************************************************
	//	 value,unit=0,sensor=0
	//
	else if ( ( _("10") == keypairs[_("OP")] ) || ( _("MEASUREMENT") == keypairs[_("OP")].Upper() ) ) {

		if ( ( _("") != keypairs[_("MEASUREMENT")] ) && (_("") != keypairs[_("TYPE")]) ) {
			
			rv = webserv_rest_doWriteMeasurement( conn, pSession, format, 
													keypairs[_("TYPE")],
													keypairs[_("MEASUREMENT")],
													keypairs[_("UNIT")],
													keypairs[_("SENSORIDX")] );
		}
		else {
			webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_MISSING_DATA );
		}	
	}

	//   *******************************************
	//   * * * * * * * * Table read  * * * * * * * *
	//   *******************************************
	else if ( ( _("11") == keypairs[_("op")] ) || ( _("TABLE") == keypairs[_("op")].Upper() ) ) {

		if ( _("") != keypairs[_("NAME")] ) {
			
			rv = webserv_rest_doGetTableData( conn, pSession, format, 
													keypairs[_("NAME")],
													keypairs[_("FROM")],
													keypairs[_("TO")] );
		}
		else {
			webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_MISSING_DATA );
		}		
	}

	return rv;
}



///////////////////////////////////////////////////////////////////////////////
// webserv_rest_error
//

void
VSCPWebServerThread::webserv_rest_error( struct mg_connection *conn, 
										    struct websrv_rest_session *pSession, 
										    int format,
										    int errorcode)
{
	char buf[2048];

	if ( REST_FORMAT_PLAIN == format ) {
		webserv_util_sendheader( conn, 400, REST_MIME_TYPE_PLAIN );
				
		mg_write( conn, "\r\n", 2 );		// head/body Separator
				
		memset( buf, 0, sizeof( buf ));
		webserv_util_make_chunk( buf, rest_errors[errorcode][REST_FORMAT_PLAIN], strlen( rest_errors[errorcode][REST_FORMAT_PLAIN] ) );
		mg_write( conn, buf, strlen( buf ) );

		mg_write( conn, "0\r\n\r\n", 5);	// Terminator
		return;
	}
	else if ( REST_FORMAT_CSV == format ) {
		webserv_util_sendheader( conn, 400, REST_MIME_TYPE_CSV );
	
		mg_write( conn, "\r\n", 2 );		// head/body Separator
				
		memset( buf, 0, sizeof( buf ));
		webserv_util_make_chunk( buf, rest_errors[errorcode][REST_FORMAT_CSV], strlen( rest_errors[errorcode][REST_FORMAT_CSV] ) );
		mg_write( conn, buf, strlen( buf ) );

		mg_write( conn, "0\r\n\r\n", 5);	// Terminator
		return;
	}
	else if ( REST_FORMAT_XML == format ) {
		webserv_util_sendheader( conn, 400, REST_MIME_TYPE_XML );

		mg_write( conn, "\r\n", 2 );		// head/body Separator

		memset( buf, 0, sizeof( buf ) );
		webserv_util_make_chunk( buf, XML_HEADER, strlen( XML_HEADER ) );
		mg_write( conn, buf, strlen( buf ) );
				
		memset( buf, 0, sizeof( buf ) );				
		webserv_util_make_chunk( buf, rest_errors[errorcode][REST_FORMAT_XML], strlen( rest_errors[errorcode][REST_FORMAT_XML] ) );
		mg_write( conn, buf, strlen( buf ) );
				
		mg_write( conn, "0\r\n\r\n", 5);	// Terminator

		return;
	}
	else if ( REST_FORMAT_JSON == format ) {
		webserv_util_sendheader( conn, 400, REST_MIME_TYPE_JSON );

		mg_write( conn, "\r\n", 2 );		// head/body Separator
		memset( buf, 0, sizeof( buf ) );
		webserv_util_make_chunk( buf, rest_errors[errorcode][REST_FORMAT_JSON], strlen( rest_errors[errorcode][REST_FORMAT_JSON] ) );
		mg_write( conn, buf, strlen( buf ) );

		mg_write( conn, "0\r\n\r\n", 5);	// Terminator

		return;
	}
	else if ( REST_FORMAT_JSONP == format ) {
		webserv_util_sendheader( conn, 400, REST_MIME_TYPE_JSONP );

		mg_write( conn, "\r\n", 2 );		// head/body Separator
		memset( buf, 0, sizeof( buf ) );
		webserv_util_make_chunk( buf, rest_errors[errorcode][REST_FORMAT_JSONP], strlen( rest_errors[errorcode][REST_FORMAT_JSONP] ) );
		mg_write( conn, buf, strlen( buf ) );

		mg_write( conn, "0\r\n\r\n", 5);	// Terminator

		return;
	}
	else {
		webserv_util_sendheader( conn, 400, REST_MIME_TYPE_PLAIN );

		mg_write( conn, "\r\n", 2 );		// head/body Separator
		memset( buf, 0, sizeof( buf ) );
		webserv_util_make_chunk( buf, REST_PLAIN_ERROR_UNSUPPORTED_FORMAT, strlen( REST_PLAIN_ERROR_UNSUPPORTED_FORMAT ) );
		mg_write( conn, buf, strlen( buf ) );

		mg_write( conn, "0\r\n\r\n", 5);	// Terminator

		return;
	}	
}



///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doStatus
//

int
VSCPWebServerThread::webserv_rest_doSendEvent( struct mg_connection *conn, 
											        struct websrv_rest_session *pSession, 
											        int format, 
											        vscpEvent *pEvent )
{
	bool bSent = false;

	// Check pointer
    if (NULL == conn) return MG_FALSE;
	
	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

	if ( NULL != pSession ) {

		// Level II events betwen 512-1023 is recognized by the daemon and 
		// sent to the correct interface as Level I events if the interface  
		// is addressed by the client.
		if ((pEvent->vscp_class <= 1023) &&
			    (pEvent->vscp_class >= 512) &&
				(pEvent->sizeData >= 16)) {

			// This event shold be sent to the correct interface if it is
			// available on this machine. If not it should be sent to 
			// the rest of the network as normal

			cguid destguid;
			destguid.getFromArray(pEvent->pdata);
			destguid.setAt(0,0);
			destguid.setAt(1,0);

			if (NULL != pSession->pClientItem ) {

				// Set client id 
				pEvent->obid = pSession->pClientItem->m_clientID;

				// Check if filtered out
				if (vscp_doLevel2Filter( pEvent, &pSession->pClientItem->m_filterVSCP)) {

					// Lock client
					pObject->m_wxClientMutex.Lock();

					// If the client queue is full for this client then the
					// client will not receive the message
					if (pSession->pClientItem->m_clientInputQueue.GetCount() <=
							pObject->m_maxItemsInClientReceiveQueue) {

						vscpEvent *pNewEvent = new( vscpEvent );
						if ( NULL != pNewEvent ) {

							vscp_copyVSCPEvent(pNewEvent, pEvent);

							// Add the new event to the inputqueue
							pSession->pClientItem->m_mutexClientInputQueue.Lock();
							pSession->pClientItem->m_clientInputQueue.Append( pNewEvent );
							pSession->pClientItem->m_semClientInputQueue.Post();

							bSent = true;
						}
						else {
							bSent = false;
						}

					} 
					else {
						// Overun - No room for event
						//vscp_deleteVSCPevent( pEvent );
						bSent = true;
					}

					// Unlock client
					pObject->m_wxClientMutex.Unlock();

				} // filter

			} // Client found
		}

		if (!bSent) {

			if (NULL != pSession->pClientItem ) {

				// Set client id 
				pEvent->obid = pSession->pClientItem->m_clientID;

				// There must be room in the send queue
				if (pObject->m_maxItemsInClientReceiveQueue >
						pObject->m_clientOutputQueue.GetCount()) {

					vscpEvent *pNewEvent = new( vscpEvent );
					if ( NULL != pNewEvent ) {
						vscp_copyVSCPEvent(pNewEvent, pEvent);
					
						pObject->m_mutexClientOutputQueue.Lock();
						pObject->m_clientOutputQueue.Append(pNewEvent);
						pObject->m_semClientOutputQueue.Post();
						pObject->m_mutexClientOutputQueue.Unlock();					

						bSent = true;
					}
					else {
						bSent = false;
					}
				}
				else {
					vscp_deleteVSCPevent( pEvent );
					bSent = false;
				}

			}
			else {
				vscp_deleteVSCPevent( pEvent );
				bSent = false;
			}

		}
	}
	else {
		webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
		bSent = false;
	}

	return MG_TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doReadEvent
//

int
VSCPWebServerThread::webserv_rest_doReceiveEvent( struct mg_connection *conn, 
												        struct websrv_rest_session *pSession, 
												        int format,
												        long count )
{
	// Check pointer
    if (NULL == conn) return MG_FALSE;
	
	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

	if ( NULL != pSession ) {

		if ( !pSession->pClientItem->m_clientInputQueue.empty() ) {

			char buf[32000];
			char wrkbuf[32000];
			wxString out;

			if ( REST_FORMAT_PLAIN == format ) {

				webserv_util_sendheader( conn, 400, REST_MIME_TYPE_PLAIN );
				
				mg_write( conn, "\r\n", 2 );		// head/body Separator

				if ( pSession->pClientItem->m_bOpen &&
					pSession->pClientItem->m_clientInputQueue.GetCount()) {

					memset( buf, 0, sizeof( buf ));
					sprintf( wrkbuf, "1 1 Success \r\n");
					webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
					mg_write( conn, buf, strlen( buf ) );

					memset( buf, 0, sizeof( buf ));
					sprintf( wrkbuf, 
								"%ld events requested of %lu available (unfiltered) %lu will be retrived\r\n", 
								count, 
								pSession->pClientItem->m_clientInputQueue.GetCount(),
								MIN((unsigned long)count,pSession->pClientItem->m_clientInputQueue.GetCount()) );
					webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
					mg_write( conn, buf, strlen( buf ) );

					for ( unsigned int i=0; i<MIN((unsigned long)count,pSession->pClientItem->m_clientInputQueue.GetCount()); i++ ) {

						CLIENTEVENTLIST::compatibility_iterator nodeClient;
						vscpEvent *pEvent;

						pSession->pClientItem->m_mutexClientInputQueue.Lock();
						nodeClient = pSession->pClientItem->m_clientInputQueue.GetFirst();
						pEvent = nodeClient->GetData();
						pSession->pClientItem->m_clientInputQueue.DeleteNode(nodeClient);
						pSession->pClientItem->m_mutexClientInputQueue.Unlock();

						if (NULL != pEvent) {

							if (vscp_doLevel2Filter(pEvent, &pSession->pClientItem->m_filterVSCP)) {

								wxString str;
								if (vscp_writeVscpEventToString(pEvent, str)) {

									// Write it out
									memset((char *) wrkbuf, 0, sizeof( wrkbuf ));
									strcpy((char *) wrkbuf, (const char*) "- ");
									strcat((char *) wrkbuf, (const char*) str.mb_str(wxConvUTF8));
									strcat((char *) wrkbuf, "/r/n" );
									webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
									mg_write( conn, buf, strlen( buf ) );
							
								}	
								else {
									memset( buf, 0, sizeof( buf ) );
									strcpy((char *) wrkbuf, "- Malformed event (intenal error)/r/n" );
									webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
									mg_write( conn, buf, strlen( buf ) );
								}
							}
							else {
								memset( buf, 0, sizeof( buf ) );
								strcpy((char *) wrkbuf, "- Event filtered out/r/n" );
								webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
								mg_write( conn, buf, strlen( buf ) );
							}

							// Remove the event
							vscp_deleteVSCPevent(pEvent);

						} // Valid pEvent pointer
						else {
							memset( buf, 0, sizeof( buf ) );
							strcpy((char *) wrkbuf, "- Event could not be fetched (intenal error)/r/n" );
							webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
							mg_write( conn, buf, strlen( buf ) );
						}
					} // for
				}	 
				else {   // no events available
					memset( buf, 0, sizeof( buf ));
					sprintf( wrkbuf, REST_PLAIN_ERROR_INPUT_QUEUE_EMPTY"\r\n");
					webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
					mg_write( conn, buf, strlen( buf ) );
				}
					
			}
			else if ( REST_FORMAT_CSV == format ) {
				webserv_util_sendheader( conn, 400, REST_MIME_TYPE_CSV );
				
				mg_write( conn, "\r\n", 2 );		// head/body Separator

				if ( pSession->pClientItem->m_bOpen &&
					pSession->pClientItem->m_clientInputQueue.GetCount()) {

					memset( buf, 0, sizeof( buf ));
					sprintf( wrkbuf, "success-code,error-code,message,description,Event\r\n1,1,Success,Success.,NULL\r\n");
					webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
					mg_write( conn, buf, strlen( buf ) );

					memset( buf, 0, sizeof( buf ));
					sprintf( wrkbuf, 
								"1,2,Info,%ld events requested of %lu available (unfiltered) %lu will be retrived,NULL\r\n", 
								count, 
								pSession->pClientItem->m_clientInputQueue.GetCount(),
								MIN((unsigned long)count,pSession->pClientItem->m_clientInputQueue.GetCount()) );
					webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
					mg_write( conn, buf, strlen( buf ) );

					for ( unsigned int i=0; i<MIN((unsigned long)count,pSession->pClientItem->m_clientInputQueue.GetCount()); i++ ) {

						CLIENTEVENTLIST::compatibility_iterator nodeClient;
						vscpEvent *pEvent;

						pSession->pClientItem->m_mutexClientInputQueue.Lock();
						nodeClient = pSession->pClientItem->m_clientInputQueue.GetFirst();
						pEvent = nodeClient->GetData();
						pSession->pClientItem->m_clientInputQueue.DeleteNode(nodeClient);
						pSession->pClientItem->m_mutexClientInputQueue.Unlock();

						if (NULL != pEvent) {

							if (vscp_doLevel2Filter(pEvent, &pSession->pClientItem->m_filterVSCP)) {

								wxString str;
								if (vscp_writeVscpEventToString(pEvent, str)) {

									// Write it out
									memset((char *) wrkbuf, 0, sizeof( wrkbuf ));
									strcpy((char *) wrkbuf, (const char*) "1,3,Data,Event.,");
									strcat((char *) wrkbuf, (const char*) str.mb_str(wxConvUTF8));
									strcat((char *) wrkbuf, "/r/n" );
									webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
									mg_write( conn, buf, strlen( buf ) );
							
								}	
								else {
									memset( buf, 0, sizeof( buf ) );
									strcpy((char *) wrkbuf, "1,2,Info,Malformed event (intenal error)/r/n" );
									webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
									mg_write( conn, buf, strlen( buf ) );
								}
							}
							else {
								memset( buf, 0, sizeof( buf ) );
								strcpy((char *) wrkbuf, "1,2,Info,Event filtered out/r/n" );
								webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
								mg_write( conn, buf, strlen( buf ) );
							}

							// Remove the event
							vscp_deleteVSCPevent(pEvent);

						} // Valid pEvent pointer
						else {
							memset( buf, 0, sizeof( buf ) );
							strcpy((char *) wrkbuf, "1,2,Info,Event could not be fetched (intenal error)/r/n" );
							webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
							mg_write( conn, buf, strlen( buf ) );
						}
					} // for
				}	 
				else {   // no events available
					memset( buf, 0, sizeof( buf ));
					sprintf( wrkbuf, REST_CSV_ERROR_INPUT_QUEUE_EMPTY"\r\n");
					webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
					mg_write( conn, buf, strlen( buf ) );
				}

			}
			else if ( REST_FORMAT_XML == format ) {

				int filtered = 0;
				int errors = 0;

				webserv_util_sendheader( conn, 400, REST_MIME_TYPE_XML );
				
				mg_write( conn, "\r\n", 2 );		// head/body Separator

				if ( pSession->pClientItem->m_bOpen &&
					pSession->pClientItem->m_clientInputQueue.GetCount()) {

					memset( buf, 0, sizeof( buf ));
					sprintf( wrkbuf, XML_HEADER"<vscp-rest success = \"true\" code = \"1\" massage = \"Success\" description = \"Success.\" >");
					webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
					mg_write( conn, buf, strlen( buf ) );

					memset( buf, 0, sizeof( buf ));
					sprintf( wrkbuf, 
								"<info>%ld events requested of %lu available (unfiltered) %lu will be retrived</info>", 
								count, 
								pSession->pClientItem->m_clientInputQueue.GetCount(),
								MIN((unsigned long)count,pSession->pClientItem->m_clientInputQueue.GetCount()) );
					webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
					mg_write( conn, buf, strlen( buf ) );

					for ( unsigned int i=0; i<MIN((unsigned long)count,pSession->pClientItem->m_clientInputQueue.GetCount()); i++ ) {

						CLIENTEVENTLIST::compatibility_iterator nodeClient;
						vscpEvent *pEvent;

						pSession->pClientItem->m_mutexClientInputQueue.Lock();
						nodeClient = pSession->pClientItem->m_clientInputQueue.GetFirst();
						pEvent = nodeClient->GetData();
						pSession->pClientItem->m_clientInputQueue.DeleteNode(nodeClient);
						pSession->pClientItem->m_mutexClientInputQueue.Unlock();

						if (NULL != pEvent) {

							if (vscp_doLevel2Filter(pEvent, &pSession->pClientItem->m_filterVSCP)) {

								wxString str;

								// Write it out
								memset((char *) wrkbuf, 0, sizeof( wrkbuf ));
								strcpy((char *) wrkbuf, (const char*) "<event>");

								strcpy((char *) wrkbuf, (const char*) "<head>");
								strcpy((char *) wrkbuf, wxString::Format( _("%d"), pEvent->head ).mb_str() );
								strcpy((char *) wrkbuf, (const char*) "</head>");

								strcpy((char *) wrkbuf, (const char*) "<class>");
								strcpy((char *) wrkbuf, wxString::Format( _("%d"), pEvent->vscp_class ).mb_str() );
								strcpy((char *) wrkbuf, (const char*) "</class>");

								strcpy((char *) wrkbuf, (const char*) "<type>");
								strcpy((char *) wrkbuf, wxString::Format( _("%d"), pEvent->vscp_type ).mb_str() );
								strcpy((char *) wrkbuf, (const char*) "</type>");

								strcpy((char *) wrkbuf, (const char*) "<obid>");
								strcpy((char *) wrkbuf, wxString::Format( _("%lu"), pEvent->obid ).mb_str() );
								strcpy((char *) wrkbuf, (const char*) "</obid>");

								strcpy((char *) wrkbuf, (const char*) "<timestamp>");
								strcpy((char *) wrkbuf, wxString::Format( _("%lu"), pEvent->timestamp ).mb_str() );
								strcpy((char *) wrkbuf, (const char*) "</timestamp>");

								strcpy((char *) wrkbuf, (const char*) "<guid>");
								vscp_writeGuidToString( pEvent, str);
								strcpy((char *) wrkbuf, (const char *)str.mbc_str() );
								strcpy((char *) wrkbuf, (const char*) "</guid>");

								strcpy((char *) wrkbuf, (const char*) "<sizedata>");
								strcpy((char *) wrkbuf, wxString::Format( _("%d"), pEvent->sizeData ).mb_str() );
								strcpy((char *) wrkbuf, (const char*) "</sizedata>");

								strcpy((char *) wrkbuf, (const char*) "<data>");
								vscp_writeVscpDataToString( pEvent, str);
								strcpy((char *) wrkbuf, (const char *)str.mbc_str() );
								strcpy((char *) wrkbuf, (const char*) "</data>");

								strcat((char *) wrkbuf, "</event>" );
								webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
								mg_write( conn, buf, strlen( buf ) );

								if (vscp_writeVscpEventToString(pEvent, str)) {

									// Write it out
									memset((char *) wrkbuf, 0, sizeof( wrkbuf ));
									strcpy((char *) wrkbuf, (const char*) "<event>");
									strcat((char *) wrkbuf, (const char*) str.mb_str(wxConvUTF8));
									strcat((char *) wrkbuf, "</event>" );
									webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
									mg_write( conn, buf, strlen( buf ) );
							
								}	
								else {
									errors++;
								}
							}
							else {
								filtered++;
							}

							// Remove the event
							vscp_deleteVSCPevent(pEvent);

						} // Valid pEvent pointer
						else {
							errors++;
						}
					} // for

					strcpy((char *) wrkbuf, (const char*) "<filtered>");
					strcpy((char *) wrkbuf, wxString::Format( _("%d"), filtered ).mb_str() );
					strcpy((char *) wrkbuf, (const char*) "</filtered>");

					strcpy((char *) wrkbuf, (const char*) "<errors>");
					strcpy((char *) wrkbuf, wxString::Format( _("%d"), errors ).mb_str() );
					strcpy((char *) wrkbuf, (const char*) "</errors>");

					// End tag
					memset( buf, 0, sizeof( buf ) );
					strcpy((char *) wrkbuf, "</vscp-rest>" );
					webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
					mg_write( conn, buf, strlen( buf ) );

				}	 
				else {   // no events available
					memset( buf, 0, sizeof( buf ));
					sprintf( wrkbuf, REST_XML_ERROR_INPUT_QUEUE_EMPTY"\r\n");
					webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
					mg_write( conn, buf, strlen( buf ) );
				}

			}
			else if ( ( REST_FORMAT_JSON == format ) && ( REST_FORMAT_JSONP == format ) ) {
				
				int filtered = 0;
				int errors = 0;
				char *p = buf;
				webserv_util_sendheader( conn, 400, REST_MIME_TYPE_JSON );
				
				mg_write( conn, "\r\n", 2 );		// head/body Separator

				if ( pSession->pClientItem->m_bOpen &&
						pSession->pClientItem->m_clientInputQueue.GetCount() ) {

					if ( REST_FORMAT_JSONP == format ) {
						p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, "func(", 5 );
					}

					p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, 
						"{\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\",",
						strlen("{\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\",") );
					p += json_emit_quoted_str( p, &buf[sizeof(buf)] - p, "info", 4 );
					p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ":", 1 );					
					sprintf( wrkbuf, 
								"%ld events requested of %lu available (unfiltered) %lu will be retrived", 
								count, 
								pSession->pClientItem->m_clientInputQueue.GetCount(),
								MIN((unsigned long)count,pSession->pClientItem->m_clientInputQueue.GetCount()) );
					p += json_emit_quoted_str( p, &buf[sizeof(buf)] - p, wrkbuf, strlen(wrkbuf) );
					p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ",", 1 );
					p += json_emit_quoted_str(p, &buf[sizeof(buf)] - p, "event", 5);
					p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ":[", 2 );

					webserv_util_make_chunk( wrkbuf, buf, strlen( buf) );
					mg_write( conn, buf, strlen( wrkbuf ) );
					memset( buf, 0, sizeof( buf ) );
					p = buf;

					for ( unsigned int i=0; i<MIN((unsigned long)count,pSession->pClientItem->m_clientInputQueue.GetCount()); i++ ) {

						CLIENTEVENTLIST::compatibility_iterator nodeClient;
						vscpEvent *pEvent;

						pSession->pClientItem->m_mutexClientInputQueue.Lock();
						nodeClient = pSession->pClientItem->m_clientInputQueue.GetFirst();
						pEvent = nodeClient->GetData();
						pSession->pClientItem->m_clientInputQueue.DeleteNode(nodeClient);
						pSession->pClientItem->m_mutexClientInputQueue.Unlock();

						if (NULL != pEvent) {

							if (vscp_doLevel2Filter(pEvent, &pSession->pClientItem->m_filterVSCP)) {

								wxString str;
								p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, "{", 1 );
								
								// head
								p += json_emit_quoted_str(p, &buf[sizeof(buf)] - p, "head", 4 );
								p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ":", 1 );
								p += json_emit_long( p, &buf[sizeof(buf)] - p, pEvent->head );
								p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ",", 1 );
								
								// class
								p += json_emit_quoted_str(p, &buf[sizeof(buf)] - p, "class", 5 );
								p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ":", 1 );
								p += json_emit_long( p, &buf[sizeof(buf)] - p, pEvent->vscp_class );
								p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ",", 1 );
								
								// type
								p += json_emit_quoted_str(p, &buf[sizeof(buf)] - p, "type", 4);
								p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ":", 1 );
								p += json_emit_long( p, &buf[sizeof(buf)] - p, pEvent->vscp_type );
								p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ",", 1 );
								
								// timestamp
								p += json_emit_quoted_str(p, &buf[sizeof(buf)] - p, "timestamp", 9 );
								p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ":", 1 );
								p += json_emit_long( p, &buf[sizeof(buf)] - p, pEvent->timestamp );
								p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ",", 1 );
								
								// GUID
								vscp_writeGuidToString( pEvent, str);
								p += json_emit_quoted_str(p, &buf[sizeof(buf)] - p, "guid", 4 );
								p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ":", 1 );
								p += json_emit_quoted_str(p, &buf[sizeof(buf)] - p, str.mb_str(), str.Length() );
								p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ",", 1 );
								
								// SizeData
								p += json_emit_quoted_str(p, &buf[sizeof(buf)] - p, "sizedata", 8 );
								p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ":", 1 );
								p += json_emit_long( p, &buf[sizeof(buf)] - p, pEvent->sizeData );
								p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ",", 1 );
								
								// Data
								p += json_emit_quoted_str(p, &buf[sizeof(buf)] - p, "data", 4);
								p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ":[", 2 );
								for ( unsigned int j=0; j<pEvent->sizeData; j++ ) {
									p += json_emit_long( p, &buf[sizeof(buf)] - p, pEvent->pdata[j] );
									p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ",", 1 );
								}
								p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, "]},", 3 );

								webserv_util_make_chunk( wrkbuf, buf, strlen( buf) );
								mg_write( conn, buf, strlen( wrkbuf ) );
								memset( buf, 0, sizeof( buf ) );
								p = buf;

							}
							else {
								filtered++;;
							}

							// Remove the event
							vscp_deleteVSCPevent(pEvent);

						} // Valid pEvent pointer
						else {
							errors++;
						}

						// Buffer overflow
						if ( 0 == p ) break;

					} // for

					// Mark end
					p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, "],", 2 );
					p += json_emit_quoted_str(p, &buf[sizeof(buf)] - p, "filtered", 8 );
					p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ":", 1 );
					p += json_emit_long( p, &buf[sizeof(buf)] - p, filtered );
					p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ",", 1 );
					p += json_emit_quoted_str(p, &buf[sizeof(buf)] - p, "errors", 6 );
					p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ":", 1 );
					p += json_emit_long( p, &buf[sizeof(buf)] - p, errors );
					p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, "}", 1 );

					if ( REST_FORMAT_JSONP == format ) {
						p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ");", 2 );
					}

					webserv_util_make_chunk( wrkbuf, buf, strlen( buf) );
					mg_write( conn, buf, strlen( wrkbuf ) );

				}	 
				else {   // no events available
					memset( buf, 0, sizeof( buf ));
					if ( REST_FORMAT_JSON == format ) {
						sprintf( wrkbuf, REST_JSON_ERROR_INPUT_QUEUE_EMPTY"\r\n");
					}
					else {
						sprintf( wrkbuf, REST_JSONP_ERROR_INPUT_QUEUE_EMPTY"\r\n");
					}
					webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
					mg_write( conn, buf, strlen( buf ) );
				}
					
			}
			
			mg_write( conn, "0\r\n\r\n", 5);	// Terminator

		}
		else {	// Que is empty
			webserv_rest_error( conn, pSession, format, RESR_ERROR_CODE_INPUT_QUEUE_EMPTY );
		}

	}
	else {
		if ( REST_FORMAT_PLAIN == format ) {
			webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
		}
	}

	return MG_TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doSetFilter
//

int
VSCPWebServerThread::webserv_rest_doSetFilter( struct mg_connection *conn, 
											        struct websrv_rest_session *pSession, 
											        int format,
											        vscpEventFilter& vscpfilter )
{
	if ( NULL != pSession ) {
		pSession->pClientItem->m_mutexClientInputQueue.Lock();
		memcpy( &pSession->pClientItem->m_filterVSCP, &vscpfilter, sizeof( vscpEventFilter ) );
		pSession->pClientItem->m_mutexClientInputQueue.Unlock();
		webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_SUCCESS );
	}
	else {
		webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
	}

	return MG_TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doClearQueue
//

int
VSCPWebServerThread::webserv_rest_doClearQueue( struct mg_connection *conn, 
												    struct websrv_rest_session *pSession, 
												    int format )
{
	// Check pointer
    if (NULL == conn) return MG_FALSE;
	
	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;


	if ( NULL != pSession ) {

		CLIENTEVENTLIST::iterator iterVSCP;

		pSession->pClientItem->m_mutexClientInputQueue.Lock();
        
		for (iterVSCP = pSession->pClientItem->m_clientInputQueue.begin();
                iterVSCP != pSession->pClientItem->m_clientInputQueue.end(); ++iterVSCP) {
            vscpEvent *pEvent = *iterVSCP;
            vscp_deleteVSCPevent(pEvent);
        }

        pSession->pClientItem->m_clientInputQueue.Clear();
        pSession->pClientItem->m_mutexClientInputQueue.Unlock();
		
		webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_SUCCESS );
	}
	else {
		webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
	}

	return MG_TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doReadVariable
//

int
VSCPWebServerThread::webserv_rest_doReadVariable( struct mg_connection *conn, 
												    struct websrv_rest_session *pSession, 
												    int format,
												    wxString& strVariableName )
{
	char buf[512];
	char wrkbuf[512];

	// Check pointer
    if (NULL == conn) return MG_FALSE;
	
	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

	if ( NULL != pSession ) {

		CVSCPVariable *pvar;
        wxString strvalue;

        if ( NULL == (pvar = pObject->m_VSCP_Variables.find( strVariableName ) ) ) {
			webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_VARIABLE_NOT_FOUND );
            return MG_TRUE;
        }

        pvar->writeValueToString( strvalue );

		// Get variable value
		wxString strVariableValue;
		pvar->writeValueToString( strVariableValue );
		
		if ( REST_FORMAT_PLAIN == format ) {

				webserv_util_sendheader( conn, 400, REST_MIME_TYPE_PLAIN );
				
				mg_write( conn, "\r\n", 2 );		// head/body Separator

				memset( buf, 0, sizeof( buf ));
				sprintf( wrkbuf, "1 1 Success \r\n");
				webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
				mg_write( conn, buf, strlen( buf ) );

				memset( buf, 0, sizeof( buf ));
				sprintf( wrkbuf, 
								"variable=%s type=%d persistent=%s value=%s note=%s\r\n", 
								(const char *)pvar->getName().c_str(), 
								pvar->getType(),
								pvar->isPersistent() ? "true" : "false", 
								(const char *)strVariableValue.c_str(),
								(const char *)pvar->getNote().c_str() );				
			
				webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
				mg_write( conn, buf, strlen( buf ) );

										
		}
		else if ( REST_FORMAT_CSV == format ) {
			
			webserv_util_sendheader( conn, 400, REST_MIME_TYPE_CSV );
				
			mg_write( conn, "\r\n", 2 );		// head/body Separator

			memset( buf, 0, sizeof( buf ));
			sprintf( wrkbuf, 
				"success-code,error-code,message,description,Variable,Type, Value,Persistent,Note\r\n1,1,Success,Success.,%s,%d,%s,%s,%s\r\n",
				(const char *)strVariableName.c_str(), 
				pvar->getType(),
				(const char *)strVariableValue.c_str(),
				pvar->isPersistent() ? "true" : "false", 
				(const char *)pvar->getNote().c_str() );
			webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf ) );
			mg_write( conn, buf, strlen( buf ) );

		}
		else if ( REST_FORMAT_XML == format ) {

			int filtered = 0;
			int errors = 0;

			webserv_util_sendheader( conn, 400, REST_MIME_TYPE_XML );
				
			mg_write( conn, "\r\n", 2 );		// head/body Separator

			memset( buf, 0, sizeof( buf ));
			sprintf( wrkbuf, XML_HEADER"<vscp-rest success = \"true\" code = \"1\" massage = \"Success\" description = \"Success.\" >");
			webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
			mg_write( conn, buf, strlen( buf ) );

			memset( buf, 0, sizeof( buf ));
			sprintf( wrkbuf, 
						XML_HEADER"<variable type=%d(%s) persistent=%s >",
						pvar->getType(),
						pvar->getVariableTypeAsString( pvar->getType() ),
						pvar->isPersistent() ? "true" : "false" );
			webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
			mg_write( conn, buf, strlen( buf ) );

			memset( buf, 0, sizeof( buf ) );
			sprintf((char *) wrkbuf, 
							"<name>%s</name><value>%s</value><note>%s</note>",
							(const char *)pvar->getName().c_str(),
							(const char *)strVariableValue.c_str(),
							(const char *)pvar->getNote().c_str() );
			webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
			mg_write( conn, buf, strlen( buf ) );

			// End tag
			memset( buf, 0, sizeof( buf ) );
			strcpy((char *) wrkbuf, "</variable>" );
			webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
			mg_write( conn, buf, strlen( buf ) );

			// End tag
			memset( buf, 0, sizeof( buf ) );
			strcpy((char *) wrkbuf, "</vscp-rest>" );
			webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
			mg_write( conn, buf, strlen( buf ) );

		}
		else if ( ( REST_FORMAT_JSON == format ) && ( REST_FORMAT_JSONP == format ) ) {
				
			char *p = buf;
			webserv_util_sendheader( conn, 400, REST_MIME_TYPE_JSON );
				
			mg_write( conn, "\r\n", 2 );		// head/body Separator

			if ( pSession->pClientItem->m_bOpen &&
				pSession->pClientItem->m_clientInputQueue.GetCount() ) {

				if ( REST_FORMAT_JSONP == format ) {
					p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, "func(", 5 );
				}

				p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, 
					"{\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\",",
					strlen( "{\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\"," ) );
					
				p += json_emit_quoted_str( p, &buf[sizeof(buf)] - p, "variable-name", 13 );
				p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ":", 1 );					
				p += json_emit_quoted_str( p, &buf[sizeof(buf)] - p, pvar->getName().mb_str(), pvar->getName().Length() );
				p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ",", 1 );

				p += json_emit_quoted_str( p, &buf[sizeof(buf)] - p, "variable-type", 13 );
				p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ":", 1 );			
				wxString wxstr = wxString::FromAscii( pvar->getVariableTypeAsString( pvar->getType() ) );
				p += json_emit_quoted_str( p, &buf[sizeof(buf)] - p, wxstr.mbc_str(), wxstr.Length() );
				p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ",", 1 );

				p += json_emit_quoted_str( p, &buf[sizeof(buf)] - p, "variable-type-code", 18 );
				p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ":", 1 );
				p += json_emit_long( p, &buf[sizeof(buf)] - p, pvar->getType() );
				p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ",", 1 );

				p += json_emit_quoted_str( p, &buf[sizeof(buf)] - p, "variable-persistence", 20 );
				p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ":", 1 );
				wxstr = pvar->isPersistent() ? _("true") : _("false");
				p += json_emit_quoted_str( p, &buf[sizeof(buf)] - p, wxstr.mb_str(), wxstr.Length() );
				p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ",", 1 );

				p += json_emit_quoted_str( p, &buf[sizeof(buf)] - p, "variable-value", 14 );
				p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ":", 1 );					
				p += json_emit_quoted_str( p, &buf[sizeof(buf)] - p, strVariableValue.mb_str(), strVariableValue.Length() );
				p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ",", 1 );

				// Mark end
				p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, "}", 1 );

				if ( REST_FORMAT_JSONP == format ) {
					p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ");", 1 );
				}

				webserv_util_make_chunk( wrkbuf, buf, strlen( buf) );
				mg_write( conn, buf, strlen( wrkbuf ) );

			}	 					
		}

		mg_write( conn, "0\r\n\r\n", 5);	// Terminator
		
	}
	else {
		webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
	}

	return MG_TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doWriteVariable
//

int
VSCPWebServerThread::webserv_rest_doWriteVariable( struct mg_connection *conn, 
												        struct websrv_rest_session *pSession, 
													    int format,
													    wxString& strVariable )
{
	wxString strName;
    wxString strValue;
	wxStringTokenizer tkz( strVariable, _(";") );

	CControlObject *pObject = (CControlObject *)conn->server_param;

	if ( ( NULL == conn ) && ( NULL == pObject )  && ( NULL != pSession )  ) {

        // Get variablename
        if ( tkz.HasMoreTokens() ) {

            CVSCPVariable *pvar;
            strName = tkz.GetNextToken();
            if ( NULL == (pvar = pObject->m_VSCP_Variables.find( strName ) ) ) {
                webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_VARIABLE_NOT_FOUND );
				return MG_TRUE;
            }

            // Get variable value
            if (tkz.HasMoreTokens()) {
                strValue = tkz.GetNextToken();
                if (!pvar->setValueFromString( pvar->getType(), strValue ) ) {
                   webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_MISSING_DATA );
                   return MG_TRUE;
                }
            } 
			else {
                webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_MISSING_DATA );
                return MG_TRUE;
            }
		}

		webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_SUCCESS );

	}
	else {
		webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
	}

	return MG_TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doCreateVariable
//

int
VSCPWebServerThread::webserv_rest_doCreateVariable( struct mg_connection *conn, 
												        struct websrv_rest_session *pSession, 
													    int format,
													    wxString& strVariable )
{
	wxString strName;
    wxString strType;
    wxString strPersistence;
    wxString strValue;
    int type = VSCP_DAEMON_VARIABLE_CODE_STRING;
    bool bPersistence = false;
	wxStringTokenizer tkz( strVariable, _(";") );

	CControlObject *pObject = (CControlObject *)conn->server_param;

	if ( ( NULL == conn ) && ( NULL == pObject )  && ( NULL != pSession )  ) {

        // Get variablename
        if ( tkz.HasMoreTokens() ) {
            strName = tkz.GetNextToken();
        }
        else {
            webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_MISSING_DATA );
            return MG_TRUE;
        }

        // Get type
        if ( tkz.HasMoreTokens() ) {
            strType = tkz.GetNextToken();
            strType.Trim();
            strType.Trim( false );
            if ( strType.Length() ) {
                type = vscp_readStringValue( strType );
            }
        }
        else {
            webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_MISSING_DATA );
            return MG_TRUE;
        }

        // Get persistence
        if ( tkz.HasMoreTokens() ) {
            strPersistence = tkz.GetNextToken();
            strPersistence.MakeUpper();
            if ( strPersistence.Find( _("TRUE") ) ) {
                bPersistence = true;		
            }
        }
        else {
            webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_MISSING_DATA );
            return MG_TRUE;
        }

        // Get value
        if ( tkz.HasMoreTokens() ) {
            strValue = tkz.GetNextToken();
        }
        else {
            webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_MISSING_DATA );
            return MG_TRUE;
        }

        // Add the variable
        if ( !pObject->m_VSCP_Variables.add( strName, strValue, type, bPersistence ) ) {
            webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_VARIABLE_NOT_CREATED );
            return MG_TRUE;
        }

		webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_SUCCESS );

	}
	else {
		webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
	}

	return MG_TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doWriteMeasurement
//

int
VSCPWebServerThread::webserv_rest_doWriteMeasurement( struct mg_connection *conn, 
													    struct websrv_rest_session *pSession, 
													    int format,
													    wxString& strType,
													    wxString& strMeasurement,
													    wxString& strUnit,
													    wxString& strSensorIdx )
{
    if ( NULL != pSession ) {
        
        double value = 0;
        long unit;
        long sensoridx;
        uint8_t data[8];
        uint16_t sizeData;

        strMeasurement.ToDouble( &value );
        strUnit.ToLong( &unit );
        strSensorIdx.ToLong( &sensoridx );

        if ( vscp_convertFloatToFloatEventData( data,
                                                    &sizeData,
                                                    value, 
                                                    unit,
                                                    sensoridx ) ) {
            vscpEvent *pEvent = new vscpEvent;
            webserv_rest_doSendEvent( conn, pSession, format, pEvent );    
        }

        webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_SUCCESS );
	}
	else {
		webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
	}

	return MG_TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doGetTableData
//

int
VSCPWebServerThread::webserv_rest_doGetTableData( struct mg_connection *conn, 
												    struct websrv_rest_session *pSession, 
												    int format,
												    wxString& strName,
												    wxString& strFrom,
												    wxString& strTo )
{
	CControlObject *pObject = (CControlObject *)conn->server_param;

	if ( ( NULL == conn ) && ( NULL == pObject )  && ( NULL != pSession )  ) {

		// We need 'tablename' and optinally 'from' and 'to'

		pObject->m_mutexTableList.Lock();
		listVSCPTables::iterator iter;
		for (iter = pObject->m_listTables.begin(); iter != pObject->m_listTables.end(); ++iter) {
			CVSCPTable *pTable = *iter;
			if ( 0 == strcmp( pTable->m_vscpFileHead.nameTable, (const char *)strName.mbc_str() ) ) {
			//	pTable->logData( timestamp, value );
				break;
			}
		}
		pObject->m_mutexTableList.Unlock();
		
		webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_SUCCESS );
	}
	else {
		webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
	}

	return MG_TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doStatus
//

int
VSCPWebServerThread::webserv_rest_doStatus( struct mg_connection *conn, 
											    struct websrv_rest_session *pSession, 
											    int format )
{
	char buf[2048];
	char wrkbuf[256];

	if ( NULL != pSession ) {

		pSession->lastActiveTime = time(NULL);

		if ( REST_FORMAT_PLAIN == format ) {
			webserv_util_sendheader( conn, 200, REST_MIME_TYPE_PLAIN );

			mg_write( conn, "\r\n", 2 );		// head/body Separator
				
			memset( buf, 0, sizeof( buf ));
			webserv_util_make_chunk( buf, REST_PLAIN_ERROR_SUCCESS, strlen( REST_PLAIN_ERROR_SUCCESS ) );
			mg_write( conn, buf, strlen( buf ) );

			memset( buf, 0, sizeof( buf ));
#ifdef WIN32
			int n = _snprintf( wrkbuf, sizeof(wrkbuf), "Session-id=%s nEvents=%d", pSession->sid, pSession->pClientItem->m_clientInputQueue.GetCount() );
#else
			int n = snprintf( wrkbuf, 
								sizeof(wrkbuf), 
								"1 1 Success Session-id=%s nEvents=%lu", 
								pSession->sid, 
								pSession->pClientItem->m_clientInputQueue.GetCount() );
#endif
			webserv_util_make_chunk( buf, wrkbuf, n );
			mg_write( conn, buf, strlen( buf ) );

			mg_write( conn, "0\r\n\r\n", 5);	// Terminator
			return MG_TRUE;
		}
		else if ( REST_FORMAT_CSV == format ) {
			webserv_util_sendheader( conn, 200, REST_MIME_TYPE_CSV );

			mg_write( conn, "\r\n", 2 );		// head/body Separator

			memset( buf, 0, sizeof( buf ));
#ifdef WIN32
			int n = _snprintf( wrkbuf, 
					sizeof(wrkbuf), 
					"success-code,error-code,message,description,session-id,nEvents\r\n1,1,Success,Success. 1,1,Success,Sucess,%s,%d", 
					pSession->sid, pSession->pClientItem->m_clientInputQueue.GetCount() );
#else
			int n = snprintf( wrkbuf, 
					sizeof(wrkbuf), 
					"success-code,error-code,message,description,session-id,nEvents\r\n1,1,Success,Success. 1,1,Success,Sucess,%s,%lu", 
					pSession->sid, pSession->pClientItem->m_clientInputQueue.GetCount() );
#endif
			webserv_util_make_chunk( buf, wrkbuf, n );
			mg_write( conn, buf, strlen( buf ) );

			mg_write( conn, "0\r\n\r\n", 5);	// Terminator
			return MG_TRUE;
		}
		else if ( REST_FORMAT_XML == format ) {
			webserv_util_sendheader( conn, 200, REST_MIME_TYPE_XML );

			mg_write( conn, "\r\n", 2 );		// head/body Separator
				
			memset( buf, 0, sizeof( buf ));
#ifdef WIN32
			int n = _snprintf( wrkbuf, 
					sizeof(wrkbuf), 
					"<vscp-rest success = \"true\" code = \"1\" massage = \"Success.\" description = \"Success.\" ><session-id>%s</session-id><nEvents>%d</nEvents></vscp-rest>", 
					pSession->sid, pSession->pClientItem->m_clientInputQueue.GetCount() );
#else
			int n = snprintf( wrkbuf, 
					sizeof(wrkbuf), 
					"<vscp-rest success = \"true\" code = \"1\" massage = \"Success.\" description = \"Success.\" ><session-id>%s</session-id><nEvents>%lu</nEvents></vscp-rest>", 
					pSession->sid, pSession->pClientItem->m_clientInputQueue.GetCount() );
#endif
			webserv_util_make_chunk( buf, wrkbuf, n );
			mg_write( conn, buf, strlen( buf ) );

			mg_write( conn, "0\r\n\r\n", 5);	// Terminator
			return MG_TRUE;
		}
		else if ( REST_FORMAT_JSON == format ) {
			webserv_util_sendheader( conn, 200, REST_MIME_TYPE_JSON );

			mg_write( conn, "\r\n", 2 );		// head/body Separator
				
			memset( buf, 0, sizeof( buf ));
#ifdef WIN32
			int n = _snprintf( wrkbuf, 
					sizeof(wrkbuf), 
					"{\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\",\"session-id\":\"%s\",\"nEvents\":%d}", 
					pSession->sid, pSession->pClientItem->m_clientInputQueue.GetCount() );
#else
			int n = snprintf( wrkbuf, 
					sizeof(wrkbuf), 
					"{\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\",\"session-id\":\"%s\",\"nEvents\":%lu}", 
					pSession->sid, pSession->pClientItem->m_clientInputQueue.GetCount() );
#endif
			webserv_util_make_chunk( buf, wrkbuf, n );
			mg_write( conn, buf, strlen( buf ) );
			
			mg_write( conn, "0\r\n\r\n", 5);	// Terminator
			return MG_TRUE;
		}
		else if ( REST_FORMAT_JSONP == format ) {
			webserv_util_sendheader( conn, 200, REST_MIME_TYPE_JSONP );

			mg_write( conn, "\r\n", 2 );		// head/body Separator
				
			memset( buf, 0, sizeof( buf ));
#ifdef WIN32
			int n = _snprintf( wrkbuf, 
					sizeof(wrkbuf), 
					"func({\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\",\"session-id\":\"%s\",\"nEvents\":%d});", 
					pSession->sid, pSession->pClientItem->m_clientInputQueue.GetCount() );
#else
			int n = snprintf( wrkbuf, 
					sizeof(wrkbuf), 
					"func({\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\",\"session-id\":\"%s\",\"nEvents\":%lu});", 
					pSession->sid, pSession->pClientItem->m_clientInputQueue.GetCount() );
#endif
			webserv_util_make_chunk( buf, wrkbuf, n );
			mg_write( conn, buf, strlen( buf ) );
			
			mg_write( conn, "0\r\n\r\n", 5);	// Terminator
			return MG_TRUE;
		}
		else {
			webserv_util_sendheader( conn, 400, REST_MIME_TYPE_PLAIN );

			mg_write( conn, "\r\n", 2 );		// head/body Separator
			memset( buf, 0, sizeof( buf ) );
			webserv_util_make_chunk( buf, REST_PLAIN_ERROR_UNSUPPORTED_FORMAT, strlen( REST_PLAIN_ERROR_UNSUPPORTED_FORMAT ) );
			mg_write( conn, buf, strlen( buf ) );

			mg_write( conn, "0\r\n\r\n", 5);	// Terminator

			return MG_TRUE;
		}
			
	} // No session
	else {	
		webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
	}

	return MG_TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doOpen
//

int
VSCPWebServerThread::webserv_rest_doOpen( struct mg_connection *conn, 
										        struct websrv_rest_session *pSession,
										        CUserItem *pUser,
										        int format )
{
    char buf[2048];
	char wrkbuf[256];

	pSession = websrv_new_rest_session( conn, pUser );
	if ( NULL != pSession ) {
		
		// New session created

		if ( REST_FORMAT_PLAIN == format ) {
			webserv_util_sendheader( conn, 200, REST_MIME_TYPE_PLAIN );

			mg_write( conn, "\r\n", 2 );		// head/body Separator
				
			//memset( buf, 0, sizeof( buf ));
			//webserv_util_make_chunk( buf, REST_PLAIN_ERROR_SUCCESS, strlen( REST_PLAIN_ERROR_SUCCESS ) );
			//mg_write( conn, buf, strlen( buf ) );

			memset( buf, 0, sizeof( buf ));
#ifdef WIN32
			int n = _snprintf( wrkbuf, sizeof(wrkbuf), "1 1 Success Session-id=%s nEvents=%d", pSession->sid, pSession->pClientItem->m_clientInputQueue.GetCount() );
#else
			int n = snprintf( wrkbuf, sizeof(wrkbuf), "1 1 Success Session-id=%s nEvents=%lu", pSession->sid, pSession->pClientItem->m_clientInputQueue.GetCount() );
#endif
			webserv_util_make_chunk( buf, wrkbuf, n );
			mg_write( conn, buf, strlen( buf ) );

			mg_write( conn, "0\r\n\r\n", 5);	// Terminator
			return MG_TRUE;
		}
		else if ( REST_FORMAT_CSV == format ) {
			webserv_util_sendheader( conn, 200, REST_MIME_TYPE_CSV );

			mg_write( conn, "\r\n", 2 );		// head/body Separator

			memset( buf, 0, sizeof( buf ));
#ifdef WIN32
			int n = _snprintf( wrkbuf, 
					sizeof(wrkbuf), 
					"success-code,error-code,message,description,session-id,nEvents\r\n1,1,Success,Success. 1,1,Success,Success,%s,%d", 
					pSession->sid, pSession->pClientItem->m_clientInputQueue.GetCount() );
#else
			int n = snprintf( wrkbuf, 
					sizeof(wrkbuf), 
					"success-code,error-code,message,description,session-id,nEvents\r\n1,1,Success,Success. 1,1,Success,Success,%s,%lu", 
					pSession->sid, pSession->pClientItem->m_clientInputQueue.GetCount() );
#endif
			webserv_util_make_chunk( buf, wrkbuf, n );
			mg_write( conn, buf, strlen( buf ) );

			mg_write( conn, "0\r\n\r\n", 5);	// Terminator
			return MG_TRUE;
		}
		else if ( REST_FORMAT_XML == format ) {
			webserv_util_sendheader( conn, 200, REST_MIME_TYPE_XML );

			mg_write( conn, "\r\n", 2 );		// head/body Separator
				
			memset( buf, 0, sizeof( buf ));
#ifdef WIN32
			int n = _snprintf( wrkbuf, 
					sizeof(wrkbuf), 
					"<vscp-rest success = \"true\" code = \"1\" massage = \"Success.\" description = \"Success.\" ><session-id>%s</session-id><nEvents>%d</nEvents></vscp-rest>", 
					pSession->sid, pSession->pClientItem->m_clientInputQueue.GetCount() );
#else
			int n = snprintf( wrkbuf, 
					sizeof(wrkbuf), 
					"<vscp-rest success = \"true\" code = \"1\" massage = \"Success.\" description = \"Success.\" ><session-id>%s</session-id><nEvents>%lu</nEvents></vscp-rest>", 
					pSession->sid, pSession->pClientItem->m_clientInputQueue.GetCount() );
#endif
			webserv_util_make_chunk( buf, wrkbuf, n );
			mg_write( conn, buf, strlen( buf ) );

			mg_write( conn, "0\r\n\r\n", 5);	// Terminator
			return MG_TRUE;
		}
		else if ( REST_FORMAT_JSON == format ) {
			webserv_util_sendheader( conn, 200, REST_MIME_TYPE_JSON);

			mg_write( conn, "\r\n", 2 );		// head/body Separator
				
			memset( buf, 0, sizeof( buf ));
#ifdef WIN32
			int n = _snprintf( wrkbuf, 
					sizeof(wrkbuf), 
					"{\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\",\"session-id\":\"%s\",\"nEvents\":%d}", 
					pSession->sid, pSession->pClientItem->m_clientInputQueue.GetCount() );
#else
			int n = snprintf( wrkbuf, 
					sizeof(wrkbuf), 
					"{\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\",\"session-id\":\"%s\",\"nEvents\":%lu}", 
					pSession->sid, pSession->pClientItem->m_clientInputQueue.GetCount() );
#endif
			webserv_util_make_chunk( buf, wrkbuf, n );
			mg_write( conn, buf, strlen( buf ) );
			
			mg_write( conn, "0\r\n\r\n", 5);	// Terminator
			return MG_TRUE;
		}
		else if ( REST_FORMAT_JSONP == format ) {
			webserv_util_sendheader( conn, 200, REST_MIME_TYPE_JSONP);

			mg_write( conn, "\r\n", 2 );		// head/body Separator
				
			memset( buf, 0, sizeof( buf ));
#ifdef WIN32
			int n = _snprintf( wrkbuf, 
					sizeof(wrkbuf), 
					"func({\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\",\"session-id\":\"%s\",\"nEvents\":%d});", 
					pSession->sid, pSession->pClientItem->m_clientInputQueue.GetCount() );
#else
			int n = snprintf( wrkbuf, 
					sizeof(wrkbuf), 
					"func({\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\",\"session-id\":\"%s\",\"nEvents\":%lu});", 
					pSession->sid, pSession->pClientItem->m_clientInputQueue.GetCount() );
#endif
			webserv_util_make_chunk( buf, wrkbuf, n );
			mg_write( conn, buf, strlen( buf ) );
			
			mg_write( conn, "0\r\n\r\n", 5);	// Terminator
			return MG_TRUE;
		}
		else {
			webserv_util_sendheader( conn, 400, REST_MIME_TYPE_PLAIN );

			mg_write( conn, "\r\n", 2 );		// head/body Separator
			memset( buf, 0, sizeof( buf ) );
			webserv_util_make_chunk( buf, REST_PLAIN_ERROR_UNSUPPORTED_FORMAT, strlen( REST_PLAIN_ERROR_UNSUPPORTED_FORMAT ) );
			mg_write( conn, buf, strlen( buf ) );

			mg_write( conn, "0\r\n\r\n", 5);	// Terminator

			return MG_TRUE;
		}
	}
	else {		// Unable to create session	
		webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
	}

	return MG_TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doClose
//

int
VSCPWebServerThread::webserv_rest_doClose( struct mg_connection *conn, 
											struct websrv_rest_session *pSession, 
											int format )
{
	char buf[2048];
	char wrkbuf[256];

	if ( NULL != pSession ) {

		char sid[32 + 1 ];
		memset( sid, 0, sizeof( sid ) );
		memcpy( sid, pSession->sid, sizeof( sid ) );

		// We should close the session
		pSession->pClientItem->m_bOpen = false;
		pSession->lastActiveTime = 0;
		websrv_expire_rest_sessions( conn );

		if ( REST_FORMAT_PLAIN == format ) {
			webserv_util_sendheader( conn, 200, REST_MIME_TYPE_PLAIN );

			mg_write( conn, "\r\n", 2 );		// head/body Separator
				
			memset( buf, 0, sizeof( buf ));
			webserv_util_make_chunk( buf, REST_PLAIN_ERROR_SUCCESS, strlen( REST_PLAIN_ERROR_SUCCESS ) );
			mg_write( conn, buf, strlen( buf ) );

			memset( buf, 0, sizeof( buf ));
#ifdef WIN32
			int n = _snprintf( wrkbuf, sizeof(wrkbuf), REST_PLAIN_ERROR_SUCCESS );
#else
			int n = snprintf( wrkbuf, sizeof(wrkbuf), REST_PLAIN_ERROR_SUCCESS );
#endif
			webserv_util_make_chunk( buf, wrkbuf, n );
			mg_write( conn, buf, strlen( buf ) );

			mg_write( conn, "0\r\n\r\n", 5);	// Terminator
			return MG_TRUE;
		}
		else if ( REST_FORMAT_CSV == format ) {
			webserv_util_sendheader( conn, 200, REST_MIME_TYPE_CSV );

			mg_write( conn, "\r\n", 2 );		// head/body Separator

			memset( buf, 0, sizeof( buf ));
#ifdef WIN32
			int n = _snprintf( wrkbuf, sizeof(wrkbuf), REST_CSV_ERROR_SUCCESS );
#else
			int n = snprintf( wrkbuf, sizeof(wrkbuf), REST_CSV_ERROR_SUCCESS );
#endif
			webserv_util_make_chunk( buf, wrkbuf, n );
			mg_write( conn, buf, strlen( buf ) );

			mg_write( conn, "0\r\n\r\n", 5);	// Terminator
			return MG_TRUE;
		}
		else if ( REST_FORMAT_XML == format ) {
			webserv_util_sendheader( conn, 200, REST_MIME_TYPE_XML );

			mg_write( conn, "\r\n", 2 );		// head/body Separator
				
			memset( buf, 0, sizeof( buf ));
#ifdef WIN32
			int n = _snprintf( wrkbuf, sizeof(wrkbuf), REST_XML_ERROR_SUCCESS );
#else
			int n = snprintf( wrkbuf, sizeof(wrkbuf), REST_XML_ERROR_SUCCESS );
#endif
			webserv_util_make_chunk( buf, wrkbuf, n );
			mg_write( conn, buf, strlen( buf ) );

			mg_write( conn, "0\r\n\r\n", 5);	// Terminator
			return MG_TRUE;
		}
		else if ( REST_FORMAT_JSON == format ) {
			webserv_util_sendheader( conn, 200, REST_MIME_TYPE_JSON );

			mg_write( conn, "\r\n", 2 );		// head/body Separator
				
			memset( buf, 0, sizeof( buf ));
#ifdef WIN32
			int n = _snprintf( wrkbuf, sizeof(wrkbuf), REST_JSON_ERROR_SUCCESS );
#else
			int n = snprintf( wrkbuf, sizeof(wrkbuf), REST_JSON_ERROR_SUCCESS );
#endif
			webserv_util_make_chunk( buf, wrkbuf, n );
			mg_write( conn, buf, strlen( buf ) );
			
			mg_write( conn, "0\r\n\r\n", 5);	// Terminator
			return MG_TRUE;
		}
		else if ( REST_FORMAT_JSONP == format ) {
			webserv_util_sendheader( conn, 200, REST_MIME_TYPE_JSONP );

			mg_write( conn, "\r\n", 2 );		// head/body Separator
				
			memset( buf, 0, sizeof( buf ));
#ifdef WIN32
			int n = _snprintf( wrkbuf, sizeof(wrkbuf), REST_JSONP_ERROR_SUCCESS );
#else
			int n = snprintf( wrkbuf, sizeof(wrkbuf), REST_JSONP_ERROR_SUCCESS );
#endif
			webserv_util_make_chunk( buf, wrkbuf, n );
			mg_write( conn, buf, strlen( buf ) );
			
			mg_write( conn, "0\r\n\r\n", 5);	// Terminator
			return MG_TRUE;
		}
		else {
			webserv_util_sendheader( conn, 400, REST_MIME_TYPE_PLAIN );

			mg_write( conn, "\r\n", 2 );		// head/body Separator
			memset( buf, 0, sizeof( buf ) );
			webserv_util_make_chunk( buf, REST_PLAIN_ERROR_UNSUPPORTED_FORMAT, strlen( REST_PLAIN_ERROR_UNSUPPORTED_FORMAT ) );
			mg_write( conn, buf, strlen( buf ) );

			mg_write( conn, "0\r\n\r\n", 5);	// Terminator

			return MG_TRUE;
		}

	}
	else {	// session not found
		webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
	}

	return MG_TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// websrv_mainpage
//

int
VSCPWebServerThread::websrv_mainpage( struct mg_connection *conn )
{
	wxString strHost;

	// Check pointer
	if (NULL == conn) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;
	
	// Get hostname
	const char *p = mg_get_header( conn, "Host" ); // conn->local_ip; //_("http://localhost:8080");
    strHost.FromAscii( p );

    wxString buildPage;
	mg_send_status( conn, 200 );
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Control"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code

    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
	// Insert server url into navigation menu   
    buildPage += _(WEB_COMMON_MENU);
        
    buildPage += _("<span align=\"center\">");
    buildPage += _("<h4> Welcome to the VSCP daemon control interface.</h4>");
    buildPage += _("</span>");
	buildPage += _("<span style=\"text-indent:50px;\"><p>");
	buildPage += _("<img src=\"http://vscp.org/images/vscp_logo.jpg\" width=\"100\">");
	buildPage += _("</p></span>");
    buildPage += _("<span style=\"text-indent:50px;\"><p>");
    buildPage += _(" <b>Version:</b> ");
    buildPage += _(VSCPD_DISPLAY_VERSION);
    buildPage += _("</p><p>");
    buildPage += _(VSCPD_COPYRIGHT_HTML);
    buildPage += _("</p></span>");
            
    buildPage += _(WEB_COMMON_END);     // Common end code
    
	// Server data
	mg_send_data( conn, buildPage.ToAscii(), buildPage.Length() );

	return MG_TRUE;	
}


///////////////////////////////////////////////////////////////////////////////
// websrv_interfaces
//

int 
VSCPWebServerThread::websrv_interfaces( struct mg_connection *conn )
{
    // Check pointer
	if (NULL == conn) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

    wxString buildPage;
	mg_send_status( conn, 200 );
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Control"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code

    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
	// Insert server url into navigation menu   
    buildPage += _(WEB_COMMON_MENU);
    
    buildPage += _(WEB_IFLIST_BODY_START);
    buildPage += _(WEB_IFLIST_TR_HEAD);

    wxString strGUID;  
    wxString strBuf;

    // Display Interface List
    pObject->m_wxClientMutex.Lock();
    VSCPCLIENTLIST::iterator iter;
    for (iter = pObject->m_clientList.m_clientItemList.begin();
            iter != pObject->m_clientList.m_clientItemList.end();
            ++iter) {

        CClientItem *pItem = *iter;
        pItem->m_guid.toString(strGUID);

        buildPage += _(WEB_IFLIST_TR);

        // Client id
        buildPage += _(WEB_IFLIST_TD_CENTERED);
        buildPage += wxString::Format(_("%d"), pItem->m_clientID);
        buildPage += _("</td>");

        // Interface type
        buildPage += _(WEB_IFLIST_TD_CENTERED);
        buildPage += wxString::Format(_("%d"), pItem->m_type);
        buildPage += _("</td>");

        // GUID
        buildPage += _(WEB_IFLIST_TD_GUID);
        buildPage += strGUID.Left(23);
        buildPage += _("<br>");
        buildPage += strGUID.Right(23);
        buildPage += _("</td>");

        // Interface name
        buildPage += _("<td>");
        buildPage += pItem->m_strDeviceName.Left(pItem->m_strDeviceName.Length()-30);
        buildPage += _("</td>");

        // Start date
        buildPage += _("<td>");
        buildPage += pItem->m_strDeviceName.Right(19);
        buildPage += _("</td>");

        buildPage += _("</tr>");

    }
    
    pObject->m_wxClientMutex.Unlock();
    
    buildPage += _(WEB_IFLIST_TABLE_END);
    
    buildPage += _("<br>All interfaces to the daemon is listed here. This is drivers as well as clients on one of the daemons interfaces. It is possible to see events coming in on a on a specific interface and send events on just one of the interfaces. This is mostly used on the driver interfaces but is possible on all interfacs<br>");
    
    buildPage += _("<br><b>Interface Types</b><br>");
    buildPage += wxString::Format( _("%d - Unknown (you should not see this).<br>"), (uint8_t)CLIENT_ITEM_INTERFACE_TYPE_NONE );
    buildPage += wxString::Format( _("%d - Internal daemon client.<br>"), (uint8_t)CLIENT_ITEM_INTERFACE_TYPE_CLIENT_INTERNAL );
    buildPage += wxString::Format( _("%d - Level I (CANAL) Driver.<br>"), (uint8_t)CLIENT_ITEM_INTERFACE_TYPE_DRIVER_LEVEL1 );
    buildPage += wxString::Format( _("%d - Level II Driver.<br>"), (uint8_t)CLIENT_ITEM_INTERFACE_TYPE_DRIVER_LEVEL2 );
    buildPage += wxString::Format( _("%d - TCP/IP Client.<br>"), (uint8_t)CLIENT_ITEM_INTERFACE_TYPE_CLIENT_TCPIP );
	buildPage += wxString::Format( _("%d - UDP Client.<br>"), (uint8_t)CLIENT_ITEM_INTERFACE_TYPE_CLIENT_UDP );
	buildPage += wxString::Format( _("%d - Web Server Client.<br>"), (uint8_t)CLIENT_ITEM_INTERFACE_TYPE_CLIENT_WEB );
	buildPage += wxString::Format( _("%d - WebSocket Client.<br>"), (uint8_t)CLIENT_ITEM_INTERFACE_TYPE_CLIENT_WEBSOCKET );

    buildPage += _(WEB_COMMON_END);     // Common end code
    
	// Server data
	mg_send_data( conn, buildPage.ToAscii(), buildPage.Length() );

	return MG_TRUE;	
}



///////////////////////////////////////////////////////////////////////////////
// websrv_dmlist
//

int
VSCPWebServerThread::websrv_dmlist( struct mg_connection *conn )
{
	char buf[80];
    VSCPInformation vscpinfo;

	// Check pointer
	if (NULL == conn) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;
        
    // light
    bool bLight = false;
	if ( mg_get_var( conn, "light", buf, sizeof( buf ) ) <= 0 ) { 
		if ( strlen( buf ) && (NULL != strstr( "true", buf ) ) ) bLight = true;
	}
    
    // From
    long nFrom = 0;
	if ( mg_get_var( conn, "from", buf, sizeof( buf ) ) > 0 ) { 
		nFrom = atoi( buf );
	}
 
    // Count
    uint16_t nCount = 50;
	if ( mg_get_var( conn, "count", buf, sizeof( buf ) ) > 0 ) { 
		nCount = atoi( buf );
	}
    
    // Navigation button
	if ( mg_get_var( conn, "navbtn", buf, sizeof( buf ) ) > 0 ) { 
	
		if (NULL != strstr("previous", buf) ) {    
		    nFrom -= nCount;
			if ( nFrom < 0 )  nFrom = 0;
        }        
		else if (NULL != strstr("next",buf)) {
            nFrom += nCount;
            if ( nFrom > pObject->m_dm.getElementCount() - 1 ) {
                if ( pObject->m_dm.getElementCount() % nCount ) {
                    nFrom = pObject->m_dm.getElementCount()/nCount;
                }
                else {
                    nFrom = (pObject->m_dm.getElementCount()/nCount) - 1;
                }
            }
		}
		else if (NULL != strstr("last",buf)) {
            if ( pObject->m_dm.getElementCount() % nCount ) {
                nFrom = (pObject->m_dm.getElementCount()/nCount)*nCount;
            }
            else {
                nFrom = ((pObject->m_dm.getElementCount()/nCount) - 1)*nCount;
            }
		}
		else if ( NULL != strstr("first",buf) ) {
			nFrom = 0;
        }
	}
	else {  // No vaid navigation value
        //nFrom = 0;
    }   

    wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Decision Matrix"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    buildPage += _(WEB_COMMON_MENU);;  
    buildPage += _(WEB_DMLIST_BODY_START);
    
    {
		//wxString wxstrurl = wxString::Format( _("%s/vscp/dm"), conn->local_ip );
        wxString wxstrlight = ((bLight) ? _("true") : _("false"));
        buildPage += wxString::Format( _(WEB_COMMON_LIST_NAVIGATION),
                _("/vscp/dm"), //wxstrurl.GetData(),
                (unsigned long)nFrom+1,
                ( (unsigned long)(nFrom + nCount) < pObject->m_dm.getElementCount()) ? 
                    nFrom + nCount : pObject->m_dm.getElementCount(),
                (unsigned long)pObject->m_dm.getElementCount(),
                (unsigned long)nCount,
                (unsigned long)nFrom,
				(const char *)wxstrlight.c_str() );
               
        buildPage += _("<br>");
    } 

    wxString strGUID;  
    wxString strBuf;

    // Display DM List
    
    if ( 0 == pObject->m_dm.getElementCount() ) {
        buildPage += _("<br>Decision Matrix is empty!<br>");
    }
    else {
        buildPage += _(WEB_DMLIST_TR_HEAD);
    }
    
    if (nFrom < 0) nFrom = 0;
    
    for ( int i=0; i<nCount; i++) {

        // Check limits
        if ( ( nFrom+i ) >= pObject->m_dm.getElementCount() ) break;
        
        dmElement *pElement = pObject->m_dm.getElement(i);
        
        {
            wxString url_dmedit = 
                    wxString::Format(_("/vscp/dmedit?id=%l&from=%l&count=%l"),
										//conn->local_ip,
                                        (long)(nFrom+i), (long)nFrom, (long)nCount );
            wxString str = wxString::Format(_(WEB_COMMON_TR_CLICKABLE_ROW),
                                                (const char *)url_dmedit.c_str() );
            buildPage += str;
        }

        // Client id    
        buildPage += _(WEB_IFLIST_TD_CENTERED);
        buildPage += wxString::Format(_("<form name=\"input\" action=\"/vscp/dmdelete?id=%l\" method=\"get\"> %l <input type=\"submit\" value=\"x\"><input type=\"hidden\" name=\"id\"value=\"%l\"></form>"), 
										(long)nFrom+i, (long)nFrom+i+1, (long)nFrom+i );
        buildPage += _("</td>");

        // DM entry
        buildPage += _("<td>");
        
        if (NULL != pElement) {

            buildPage += _("<div id=\"small\">");

            // Group
            buildPage += _("<b>Group:</b> ");
            buildPage += pElement->m_strGroupID;
            buildPage += _("<br>");
            
            buildPage += _("<b>Comment:</b> ");
            buildPage += pElement->m_comment;
            buildPage += _("<br><hr width=\"90%\">");

            buildPage += _("<b>Control:</b> ");

            // Control - Enabled
            if (pElement->isEnabled()) {
                buildPage += _("[Row is enabled] ");
            } 
            else {
                buildPage += _("[Row is disabled] ");
            }

            // Control - End scan
            if (pElement->isScanDontContinueSet()) {
                buildPage += _("[End scan after this row] ");
            }

            // Control - Check index
            if (pElement->isCheckIndexSet()) {
                if (pElement->m_bMeasurement) {
                    buildPage += _("[Check Measurement Index] ");
                } 
                else {
                    buildPage += _("[Check Index] ");
                }
            } 

            // Control - Check zone
            if (pElement->isCheckZoneSet()) {
                buildPage += _("[Check Zone] ");
            }

            // Control - Check subzone
            if (pElement->isCheckSubZoneSet()) {
                buildPage += _("[Check Subzone] ");
            }

            buildPage += _("<br>");

            if (!bLight) {

                // * Filter

                buildPage += _("<b>Filter_priority: </b>");
                buildPage += wxString::Format(_("%d "),
                        pElement->m_vscpfilter.filter_priority);

                buildPage += _("<b>Filter_class: </b>");
                buildPage += wxString::Format(_("%d "),
                        pElement->m_vscpfilter.filter_class);
                buildPage += _(" [");
                buildPage += vscpinfo.getClassDescription( pElement->m_vscpfilter.filter_class );
                buildPage += _("] ");

                buildPage += _(" <b>Filter_type: </b>");
                buildPage += wxString::Format(_("%d "), pElement->m_vscpfilter.filter_type);
                buildPage += _(" [");
                buildPage += vscpinfo.getTypeDescription (pElement->m_vscpfilter.filter_class,
                                                            pElement->m_vscpfilter.filter_type );
                buildPage += _("]<br>");

                buildPage += _(" <b>Filter_GUID: </b>");
                vscp_writeGuidArrayToString(pElement->m_vscpfilter.filter_GUID, strGUID);
                buildPage += strGUID;

                buildPage += _("<br>");

                buildPage += _("<b>Mask_priority: </b>");
                buildPage += wxString::Format(_("%d "), pElement->m_vscpfilter.mask_priority);

                buildPage += _("<b>Mask_class: </b>");
                buildPage += wxString::Format(_("%d "), pElement->m_vscpfilter.mask_class);

                buildPage += _("<b>Mask_type: </b>");
                buildPage += wxString::Format(_("%d "), pElement->m_vscpfilter.mask_type);

                buildPage += _("<b>Mask_GUID: </b>");
                vscp_writeGuidArrayToString(pElement->m_vscpfilter.mask_GUID, strGUID);
                buildPage += strGUID;

                buildPage += _("<br>");

                buildPage += _("<b>Index: </b>");
                buildPage += wxString::Format(_("%d "), pElement->m_index);

                buildPage += _("<b>Zone: </b>");
                buildPage += wxString::Format(_("%d "), pElement->m_zone);

                buildPage += _("<b>Subzone: </b>");
                buildPage += wxString::Format(_("%d "), pElement->m_subzone);

                if ( pElement->m_bMeasurement ) {
                    buildPage += _("&nbsp;&nbsp;&nbsp;(Sensor index is used for index compare)");
                }

                buildPage += _("<br>");

                buildPage += _("<b>Allowed from:</b> ");
                buildPage += pElement->m_timeAllow.m_fromTime.FormatISODate();
                buildPage += _(" ");
                buildPage += pElement->m_timeAllow.m_fromTime.FormatISOTime();

                buildPage += _(" <b>Allowed to:</b> ");
                buildPage += pElement->m_timeAllow.m_endTime.FormatISODate();
                buildPage += _(" ");
                buildPage += pElement->m_timeAllow.m_endTime.FormatISOTime();

                buildPage += _(" <b>Weekdays:</b> ");
                buildPage += pElement->m_timeAllow.getWeekDays();
                buildPage += _("<br>");

                buildPage += _("<b>Allowed time:</b> ");
                buildPage += pElement->m_timeAllow.getActionTimeAsString();
                buildPage += _("<br>");

            } // mini

            buildPage += _("<b>Action:</b> ");
            buildPage += wxString::Format(_("%d "), pElement->m_action);

            buildPage += _(" <b>Action parameters:</b> ");
            buildPage += pElement->m_actionparam;
            buildPage += _("<br>");

            if (!bLight) {

                buildPage += _("<b>Trigger Count:</b> ");
                buildPage += wxString::Format(_("%d "), pElement->m_triggCounter);

                buildPage += _("<b>Error Count:</b> ");
                buildPage += wxString::Format(_("%d "), pElement->m_errorCounter);
                buildPage += _("<br>");

                buildPage += _("<b>Last Error String:</b> ");
                buildPage += pElement->m_strLastError;

            } // mini

            buildPage += _("</div>");

        } 
        else {
            buildPage += _("Internal error: Non existent DM entry.");
        }

        buildPage += _("</td>");
        buildPage += _("</tr>");

    }
       
    buildPage += _(WEB_DMLIST_TABLE_END);
    
    {
        //wxString wxstrurl = _("/vscp/dm");
        wxString wxstrlight = ((bLight) ? _("true") : _("false"));
        buildPage += wxString::Format( _(WEB_COMMON_LIST_NAVIGATION),
				_("/vscp/dm"), // wxstrurl,
                (unsigned long)nFrom,
                ((unsigned long)(nFrom + nCount) < pObject->m_dm.getElementCount()) ? 
                    nFrom + nCount - 1 : pObject->m_dm.getElementCount() - 1,
                (unsigned long)pObject->m_dm.getElementCount(),
                (unsigned long)nCount,
                (unsigned long)nFrom,          
				  (const char *)wxstrlight.c_str() );      
    }
     
    buildPage += _(WEB_COMMON_END);     // Common end code
    
	// Server data
	mg_send_data( conn, buildPage.ToAscii(), buildPage.Length() );

	return MG_TRUE;	
}

///////////////////////////////////////////////////////////////////////////////
// websrv_dmedit
//

int 
VSCPWebServerThread::websrv_dmedit( struct mg_connection *conn )
{
	char buf[80];
    wxString str;
    VSCPInformation vscpinfo;
    dmElement *pElement = NULL;

	// Check pointer
	if (NULL == conn) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

    // id
    long id = -1;
	if ( mg_get_var( conn, "id", buf, sizeof( buf ) ) > 0 ) {
		id = atoi(buf);
	}

    // From
    long nFrom = 0;
	if ( mg_get_var( conn, "from", buf, sizeof( buf ) ) > 0 ) { 
		nFrom = atoi( buf );
	}
 
    // Count
    uint16_t nCount = 50;
	if ( mg_get_var( conn, "count", buf, sizeof( buf ) ) > 0 ) { 
		nCount = atoi( buf );
	}
    
    // Flag for new DM row
    bool bNew = false;
	if ( mg_get_var( conn, "new", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bNew = true;
	}
    
    wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Decision Matrix Edit"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    buildPage += _(WEB_COMMON_MENU);;
    buildPage += _(WEB_DMEDIT_BODY_START);

    if ( !bNew && id < pObject->m_dm.getElementCount() ) {
        pElement = pObject->m_dm.getElement(id);
    }

    if (bNew || (NULL != pElement)) {    

        if ( bNew ) {
            buildPage += _("<span id=\"optiontext\">New record.</span><br>");
        }
        else {
            buildPage += wxString::Format(_("<span id=\"optiontext\">Record = %l.</span><br>"), id);
        }
        
        buildPage += _("<br><form method=\"get\" action=\"");
        buildPage += _("/vscp/dmpost");
        buildPage += _("\" name=\"dmedit\">");
        
        buildPage += wxString::Format(_("<input name=\"id\" value=\"%l\" type=\"hidden\"></input>"), id );
       

        if (bNew) {
            
            buildPage += _("<input name=\"new\" value=\"true\" type=\"hidden\"></input>");
            
            long nFrom;
             if ( pObject->m_dm.getElementCount() % nCount ) {
                nFrom = (pObject->m_dm.getElementCount()/nCount)*nCount;
            }
            else {
                nFrom = ((pObject->m_dm.getElementCount()/nCount) - 1)*nCount;
            }
    
            buildPage += wxString::Format( _("<input name=\"from\" value=\"%l\" type=\"hidden\">"), (long)nFrom );
            buildPage += wxString::Format( _("<input name=\"count\" value=\"%l\" type=\"hidden\">"), (long)nCount );

        }
        else {
            // Hidden from
            buildPage += wxString::Format(_("<input name=\"from\" value=\"%l\" type=\"hidden\">"), (long)nFrom );
            // Hidden count
            buildPage += wxString::Format(_("<input name=\"count\" value=\"%l\" type=\"hidden\">"), (long)nCount );
            buildPage += _("<input name=\"new\" value=\"false\" type=\"hidden\"></input>");
        }
        
        buildPage += _("<h4>Group id:</h4>");
        buildPage += _("<textarea cols=\"20\" rows=\"1\" name=\"groupid\">");
        if ( !bNew ) buildPage += pElement->m_strGroupID;
        buildPage += _("</textarea><br>");
        
        
        buildPage += _("<h4>Event:</h4> <span id=\"optiontext\">(blueish are masks)</span><br>");

        buildPage += _("<table class=\"invisable\"><tbody><tr class=\"invisable\">");

        buildPage += _("<td class=\"invisable\">Priority:</td><td class=\"tbalign\">");

        // Priority
        buildPage += _("<select name=\"filter_priority\">");
        buildPage += _("<option value=\"-1\" ");
        if (bNew) buildPage += _(" selected ");
        buildPage += _(">Don't care</option>");

        if ( !bNew ) str = (0 == pElement->m_vscpfilter.filter_priority) ? _("selected") : _(" ");
        buildPage += wxString::Format(_("<option value=\"0\" %s>0 - Highest</option>"),
                (const char *)str.c_str() );

        if ( !bNew ) str = (1 == pElement->m_vscpfilter.filter_priority) ? _("selected") : _(" ");
        buildPage += wxString::Format(_("<option value=\"1\" %s>1 - Very High</option>"),
                (const char *)str.c_str() );

        if ( !bNew ) str = (2 == pElement->m_vscpfilter.filter_priority) ? _("selected") : _(" ");
        buildPage += wxString::Format(_("<option value=\"2\" %s>2 - High</option>"),
                (const char *)str.c_str() );

        if ( !bNew ) str = (3 == pElement->m_vscpfilter.filter_priority) ? _("selected") : _(" ");
        buildPage += wxString::Format(_("<option value=\"3\" %s>3 - Normal</option>"),
                (const char *)str.c_str() );

        if ( !bNew ) str = (4 == pElement->m_vscpfilter.filter_priority) ? _("selected") : _(" ");
        buildPage += wxString::Format(_("<option value=\"4\" %s>4 - Low</option>"),
                (const char *)str.c_str() );

        if ( !bNew ) str = (5 == pElement->m_vscpfilter.filter_priority) ? _("selected") : _(" ");
        buildPage += wxString::Format(_("<option value=\"5\" %s>5 - Lower</option>"),
                (const char *)str.c_str() );

        if ( !bNew ) str = (6 == pElement->m_vscpfilter.filter_priority) ? _("selected") : _(" ");
        buildPage += wxString::Format(_("<option value=\"6\" %s>6 - Very Low</option>"),
                (const char *)str.c_str() );

        if ( !bNew ) str = (7 == pElement->m_vscpfilter.filter_priority) ? _("selected") : _(" ");
        buildPage += wxString::Format(_("<option value=\"7\" %s>7 - Lowest</option>"),
                (const char *)str.c_str() );

        buildPage += _("</select>");
        // Priority mask
        buildPage += _("</td><td><textarea style=\"background-color: #72A4D2;\" cols=\"5\" rows=\"1\" name=\"mask_priority\">");
        if ( bNew ) {
            buildPage += _("0x00");
        }
        else {
            buildPage += wxString::Format(_("%X"), pElement->m_vscpfilter.mask_priority );
        }
        buildPage += _("</textarea>");
        
        buildPage += _("</td></tr>");

        // Class
        buildPage += _("<tr class=\"invisable\"><td class=\"invisable\">Class:</td><td class=\"invisable\"><textarea cols=\"10\" rows=\"1\" name=\"filter_vscpclass\">");
        if ( bNew ) {
            buildPage += _("");;
        }
        else {
            buildPage += wxString::Format(_("%d"), pElement->m_vscpfilter.filter_class);
        }
        buildPage += _("</textarea>");
        
        buildPage += _("</td><td> <textarea style=\"background-color: #72A4D2;\" cols=\"10\" rows=\"1\" name=\"mask_vscpclass\">");
        if ( bNew ) {
            buildPage += _("0xFFFF");
        }
        else {
            buildPage += wxString::Format(_("0x%04x"), pElement->m_vscpfilter.mask_class);
        }
        buildPage += _("</textarea>");
        
        buildPage += _("</td></tr>");
        
        // Type
        buildPage += _("<tr class=\"invisable\"><td class=\"invisable\">Type:</td><td class=\"invisable\"><textarea cols=\"10\" rows=\"1\" name=\"filter_vscptype\">");
        if ( bNew ) {
            buildPage += _("");;
        }
        else {
            buildPage += wxString::Format(_("%d"), pElement->m_vscpfilter.filter_type);
        }
        buildPage += _("</textarea>");
        
        buildPage += _("</td><td> <textarea style=\"background-color: #72A4D2;\" cols=\"10\" rows=\"1\" name=\"mask_vscptype\">");
        if ( bNew ) {
            buildPage += _("0xFFFF");;
        }
        else {
            buildPage += wxString::Format(_("0x%04x"), pElement->m_vscpfilter.mask_type);
        }
        buildPage += _("</textarea>");
        
        buildPage += _("</td></tr>"); 
        
        // GUID
        if ( !bNew ) vscp_writeGuidArrayToString( pElement->m_vscpfilter.filter_GUID, str );
        buildPage += _("<tr class=\"invisable\"><td class=\"invisable\">GUID:</td><td class=\"invisable\"><textarea cols=\"50\" rows=\"1\" name=\"filter_vscpguid\">");
        if ( bNew ) {
            buildPage += _("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00");
        }
        else {
            buildPage += wxString::Format(_("%s"), (const char *)str.c_str()  );
        }
        buildPage += _("</textarea></td>");
        
        if ( !bNew ) vscp_writeGuidArrayToString( pElement->m_vscpfilter.mask_GUID, str );
        buildPage += _("<tr class=\"invisable\"><td class=\"invisable\"> </td><td class=\"invisable\"><textarea style=\"background-color: #72A4D2;\" cols=\"50\" rows=\"1\" name=\"mask_vscpguid\">");
        if ( bNew ) {
            buildPage += _("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00");
        }
        else {
            buildPage += wxString::Format(_("%s"), (const char *)str.c_str()  );
        }
        buildPage += _("</textarea></td>");
        
        buildPage += _("</tr>");
        
        // Index
        buildPage += _("<tr class=\"invisable\"><td class=\"invisable\">Index:</td><td class=\"invisable\"><textarea cols=\"10\" rows=\"1\" name=\"vscpindex\">");
        if ( bNew ) {
            buildPage += _("");
        }
        else {
            buildPage += wxString::Format(_("%d"), pElement->m_index );
        }
        buildPage += _("</textarea>");

        // Use measurement index
        buildPage += _("&nbsp;&nbsp;<input name=\"check_measurementindex\" value=\"true\" ");
        if ( bNew ) {
            buildPage += _("");
        }
        else {
            buildPage += wxString::Format(_("%s"), 
                pElement->m_bMeasurement ? _("checked") : _("") );
        }
        buildPage += _(" type=\"checkbox\">");            
        buildPage += _("<span id=\"optiontext\">Use measurement index (only for measurement events)</span>");
        buildPage += _("</td></tr>");

        // Zone
        buildPage += _("<tr class=\"invisable\"><td class=\"invisable\">Zone:</td><td class=\"invisable\"><textarea cols=\"10\" rows=\"1\" name=\"vscpzone\">");
        if ( bNew ) {
            buildPage += _("0");
        }
        else {
            buildPage += wxString::Format(_("%d"), pElement->m_zone );
        }
        buildPage += _("</textarea></td></tr>");
        
        // Subzone
        buildPage += _("<tr class=\"invisable\"><td class=\"invisable\">Subzone:</td><td class=\"invisable\"><textarea cols=\"10\" rows=\"1\" name=\"vscpsubzone\">");
        if ( bNew ) {
            buildPage += _("0");
        }
        else {
            buildPage += wxString::Format(_("%d"), pElement->m_subzone );
        }
        buildPage += _("</textarea>");
        buildPage += _("</td></tr>");
        
        buildPage += _("</tbody></table><br>");
        
        // Control
        buildPage += _("<h4>Control:</h4>");
        
        // Enable row
        buildPage += _("<input name=\"check_enablerow\" value=\"true\" ");
        if ( bNew ) {
            buildPage += _("");
        }
        else {
            buildPage += wxString::Format(_("%s"), 
            pElement->isEnabled() ? _("checked") : _("") );
        }
        buildPage += _(" type=\"checkbox\">");            
        buildPage += _("<span id=\"optiontext\">Enable row</span>&nbsp;&nbsp;"); 

        // End scan on this row
        buildPage += _("<input name=\"check_endscan\" value=\"true\"");
        if ( bNew ) {
            buildPage += _("");
        }
        else {
            buildPage += wxString::Format(_("%s"), 
                                            pElement->isScanDontContinueSet() ? _("checked") : _("") );
        }
        buildPage += _(" type=\"checkbox\">");           
        buildPage += _("<span id=\"optiontext\">End scan on this row</span>&nbsp;&nbsp;");

        //buildPage += _("<br>");

        // Check Index
        buildPage += _("<input name=\"check_index\" value=\"true\"");
        if ( bNew ) {
            buildPage += _("");
        }
        else {
            buildPage += wxString::Format(_("%s"), 
                                            pElement->isCheckIndexSet() ? _("checked") : _("") );
        }
        buildPage += _(" type=\"checkbox\">"); 
        buildPage += _("<span id=\"optiontext\">Check Index</span>&nbsp;&nbsp;");

        // Check Zone
        buildPage += _("<input name=\"check_zone\" value=\"true\"");
        if ( bNew ) {
            buildPage += _("");
        }
        else {
            buildPage += wxString::Format(_("%s"), 
                                            pElement->isCheckZoneSet() ? _("checked") : _("") );
        }
        buildPage += _(" type=\"checkbox\">"); 
        buildPage += _("<span id=\"optiontext\">Check Zone</span>&nbsp;&nbsp;"); 

        // Check subzone
        buildPage += _("<input name=\"check_subzone\" value=\"true\"");
        if ( bNew ) {
            buildPage += _("");
        }
        else {
            buildPage += wxString::Format(_("%s"), 
                                            pElement->isCheckSubZoneSet() ? _("checked") : _("") );
        }
        buildPage += _(" type=\"checkbox\">"); 
        buildPage += _("<span id=\"optiontext\">Check Subzone</span>&nbsp;&nbsp;");
        buildPage += _("<br><br><br>");
        
        buildPage += _("<h4>Allowed From:</h4>");
		buildPage += _("<i>Enter * for beginning of time.</i><br>");
        buildPage += _("<textarea cols=\"50\" rows=\"1\" name=\"allowedfrom\">");
        if ( bNew ) {
            buildPage += _("yyyy-mm-dd hh:mm:ss");
        }
        else {
            buildPage += pElement->m_timeAllow.m_fromTime.FormatISODate();
            buildPage += _(" ");
            buildPage += pElement->m_timeAllow.m_fromTime.FormatISOTime();
        }
        buildPage += _("</textarea>");

        buildPage += _("<h4>Allowed To:</h4>");
		buildPage += _("<i>Enter * for end of time (always).</i><br>");
        buildPage += _("<textarea cols=\"50\" rows=\"1\" name=\"allowedto\">");
        if ( bNew ) {
            buildPage += _("yyyy-mm-dd hh:mm:ss");
        }
        else {
            buildPage += pElement->m_timeAllow.m_endTime.FormatISODate();
            buildPage += _(" ");
            buildPage += pElement->m_timeAllow.m_endTime.FormatISOTime();
        }
        buildPage += _("</textarea>");
       
        buildPage += _("<h4>Allowed time:</h4>");
		buildPage += _("<i>Enter * for always.</i><br>");
        buildPage += _("<textarea cols=\"50\" rows=\"1\" name=\"allowedtime\">");
        if ( bNew ) {
            buildPage += _("yyyy-mm-dd hh:mm:ss");
        }
        else {
            buildPage += pElement->m_timeAllow.getActionTimeAsString();
        }
        buildPage += _("</textarea>");
        
        buildPage += _("<h4>Allowed days:</h4>");
        buildPage += _("<input name=\"monday\" value=\"true\" ");
        
        if ( !bNew ) buildPage += wxString::Format(_("%s"), 
                                        pElement->m_timeAllow.m_weekDay[0] ? _("checked") : _("") );
        buildPage += _(" type=\"checkbox\">Monday ");

        buildPage += _("<input name=\"tuesday\" value=\"true\" ");
        if ( !bNew ) buildPage += wxString::Format(_("%s"), 
                                        pElement->m_timeAllow.m_weekDay[1] ? _("checked") : _("") );
        buildPage += _(" type=\"checkbox\">Tuesday ");

        buildPage += _("<input name=\"wednesday\" value=\"true\" ");
        if ( !bNew ) buildPage += wxString::Format(_("%s"), 
                                        pElement->m_timeAllow.m_weekDay[2] ? _("checked") : _("") );
        buildPage += _(" type=\"checkbox\">Wednesday ");

        buildPage += _("<input name=\"thursday\" value=\"true\" ");
        if ( !bNew ) buildPage += wxString::Format(_("%s"), 
                                        pElement->m_timeAllow.m_weekDay[3] ? _("checked") : _("") );
        buildPage += _(" type=\"checkbox\">Thursday ");
         
        buildPage += _("<input name=\"friday\" value=\"true\" ");
        if ( !bNew ) buildPage += wxString::Format(_("%s"), 
                                        pElement->m_timeAllow.m_weekDay[4] ? _("checked") : _("") );
        buildPage += _(" type=\"checkbox\">Friday ");

        buildPage += _("<input name=\"saturday\" value=\"true\" ");
        if ( !bNew ) buildPage += wxString::Format(_("%s"), 
                                        pElement->m_timeAllow.m_weekDay[5] ? _("checked") : _("") );
        buildPage += _(" type=\"checkbox\">Saturday ");

        buildPage += _("<input name=\"sunday\" value=\"true\" ");
        if ( !bNew ) buildPage += wxString::Format(_("%s"), 
                                        pElement->m_timeAllow.m_weekDay[6] ? _("checked") : _("") );
        buildPage += _(" type=\"checkbox\">Sunday ");
        buildPage += _("<br>");
        
        buildPage += _("<h4>Action:</h4>");
        
        buildPage += _("<select name=\"action\">");
        buildPage += _("<option value=\"0\" ");
        if (bNew) buildPage += _(" selected ");
        buildPage += _(">No Operation</option>");

        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x10 == pElement->m_action ) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x10\" %s>Execute external program</option>"),
                (const char *)str.c_str() );

        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x12 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x12\" %s>Execute internal procedure</option>"),
                (const char *)str.c_str() );

        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x30 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x30\" %s>Execute library procedure</option>"),
                (const char *)str.c_str() );

        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x40 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x40\" %s>Send event</option>"),
                (const char *)str.c_str() );

        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x41 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x41\" %s>Send event Conditional</option>"),
                (const char *)str.c_str() );

        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x42 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x42\" %s>Send event(s) from file</option>"),
                (const char *)str.c_str() );

        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x43 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x43\" %s>Send event(s) to remote VSCP server</option>"),
                (const char *)str.c_str() );

        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x50 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x50\" %s>Store in variable</option>"),
                (const char *)str.c_str() );
        
        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x51 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x51\" %s>Store in array</option>"),
                (const char *)str.c_str() );
        
        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x52 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x52\" %s>Add to variable</option>"),
                (const char *)str.c_str() );
        
        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x53 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x53\" %s>Subtract from variable</option>"),
                (const char *)str.c_str() );
        
        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x54 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x54\" %s>Multiply variable</option>"),
                (const char *)str.c_str() );
        
        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x55 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x55\" %s>Divide variable</option>"),
                (const char *)str.c_str() );
        
        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x60 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x60\" %s>Start timer</option>"),
                (const char *)str.c_str() );
        
        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x61 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x61\" %s>Pause timer</option>"),
                (const char *)str.c_str() );
        
        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x62 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x62\" %s>Stop timer</option>"),
                (const char *)str.c_str() );
        
        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x63 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x63\" %s>Resume timer</option>"),
                (const char *)str.c_str() );
        
        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x70 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x70\" %s>Write file</option>"),
                (const char *)str.c_str() );
        
        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x75 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x75\" %s>Get/Put/Post URL</option>"),
                (const char *)str.c_str() );

        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x80 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x80\" %s>Write to table</option>"),
                (const char *)str.c_str() );

        buildPage += _("</select>");

        buildPage += _(" <a href=\"http://www.vscp.org/docs/vscpd/doku.php?id=vscp_daemon_decision_matrix#level_ii\" target=\"new\">Help for actions and parameters</a><br>");
             
        buildPage += _("<h4>Action parameter:</h4>");
        buildPage += _("<textarea cols=\"80\" rows=\"5\" name=\"actionparameter\">");
        if ( !bNew ) buildPage += pElement->m_actionparam;
        buildPage += _("</textarea>");


        buildPage += _("<h4>Comment:</h4>");
        buildPage += _("<textarea cols=\"80\" rows=\"5\" name=\"comment\">");
        if ( !bNew ) buildPage += pElement->m_comment;
        buildPage += _("</textarea>");
    } 
    else {
        buildPage += _("<br><b>Error: Non existent id</b>");
    }
    
    buildPage += _(WEB_DMEDIT_SUBMIT);  
    buildPage += _("</form>");
    buildPage += _(WEB_COMMON_END);     // Common end code
    
	// Server data
	mg_send_data( conn, buildPage.ToAscii(), buildPage.Length() );

	return MG_TRUE;	
}


///////////////////////////////////////////////////////////////////////////////
// websrv_dmpost
//

int 
VSCPWebServerThread::websrv_dmpost( struct mg_connection *conn )
{
	char buf[32000];
    wxString str;
    VSCPInformation vscpinfo;
    dmElement *pElement = NULL;

	// Check pointer
	if (NULL == conn) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;
    
    // id
    long id = -1;
	if ( mg_get_var( conn, "id", buf, sizeof( buf ) ) > 0 ) {
		id = atoi( buf );
	}

    // From
    long nFrom = 0;
	if ( mg_get_var( conn, "from", buf, sizeof( buf ) ) > 0 ) { 
		nFrom = atoi( buf );
	}
 
    // Count
    uint16_t nCount = 50;
	if ( mg_get_var( conn, "count", buf, sizeof( buf ) ) > 0 ) { 
		nCount = atoi( buf );
	}
    
    // Flag for new DM row
    bool bNew = false;
	if ( mg_get_var( conn, "new", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bNew = true;
	}

    wxString strGroupID;
	if ( mg_get_var( conn, "groupid", buf, sizeof( buf ) ) > 0 ) {
		strGroupID = wxString::FromAscii(buf);
	}
        
    int filter_priority = -1;
	if ( mg_get_var( conn, "filter_priority", buf, sizeof( buf ) ) > 0 ) {
		filter_priority = vscp_readStringValue( wxString::FromAscii( buf ) );
	}
         
    int mask_priority = 0;    
	if ( mg_get_var( conn, "mask_priority", buf, sizeof( buf ) ) > 0 ) {
		mask_priority = vscp_readStringValue( wxString::FromAscii( buf ) );
	}
    
    uint16_t filter_vscpclass = -1;
	if ( mg_get_var( conn, "filter_vscpclass", buf, sizeof( buf ) ) > 0 ) {
		wxString wrkstr = wxString::FromAscii( buf );
		filter_vscpclass = vscp_readStringValue( wrkstr );
	}
    
    uint16_t mask_vscpclass = 0;
	if ( mg_get_var( conn, "mask_vscpclass", buf, sizeof( buf ) ) > 0 ) {
		mask_vscpclass = vscp_readStringValue( wxString::FromAscii( buf ) );
	}
    
    uint16_t filter_vscptype = 0;
	if ( mg_get_var( conn, "filter_vscptype", buf, sizeof( buf ) ) > 0 ) {
		filter_vscptype = vscp_readStringValue( wxString::FromAscii( buf ) );
	}
    
    uint16_t mask_vscptype = 0;
	if ( mg_get_var( conn, "mask_vscptype", buf, sizeof( buf ) ) > 0 ) {
		mask_vscptype = vscp_readStringValue( wxString::FromAscii( buf ) );
	}
    
    wxString strFilterGuid;
	if ( mg_get_var( conn, "filter_vscpguid", buf, sizeof( buf ) ) > 0 ) {
		strFilterGuid = wxString::FromAscii( buf );
		strFilterGuid = strFilterGuid.Trim();
		strFilterGuid = strFilterGuid.Trim(false);
	}
    
    wxString strMaskGuid;
	if ( mg_get_var( conn, "mask_vscpguid", buf, sizeof( buf ) ) > 0 ) {
		strMaskGuid = wxString::FromAscii( buf );
		strMaskGuid = strMaskGuid.Trim();
		strMaskGuid = strMaskGuid.Trim(false);
	}
    
    uint8_t index = 0;
	if ( mg_get_var( conn, "vscpindex", buf, sizeof( buf ) ) > 0 ) {
		index = vscp_readStringValue( wxString::FromAscii( buf ) );
	}

    bool bUseMeasurementIndex = false;
	if ( mg_get_var( conn, "check_measurementindex", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bUseMeasurementIndex = true;
	}
    
    uint8_t zone = 0;
	if ( mg_get_var( conn, "vscpzone", buf, sizeof( buf ) ) > 0 ) {
		zone = vscp_readStringValue( wxString::FromAscii( buf ) );
	}
    
    uint8_t subzone = 0;
	if ( mg_get_var( conn, "vscpsubzone", buf, sizeof( buf ) ) > 0 ) {
		subzone = vscp_readStringValue( wxString::FromAscii( buf ) );
	}
    
    bool bEnableRow = false;
	if ( mg_get_var( conn, "check_enablerow", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bEnableRow = true;
	}
        
    bool bEndScan = false;
	if ( mg_get_var( conn, "check_endscan", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bEndScan = true;
	}
    
    bool bCheckIndex = false;
	if ( mg_get_var( conn, "check_index", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bCheckIndex = true;
	}
    
    bool bCheckZone = false;
	if ( mg_get_var( conn, "check_zone", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bCheckZone = true;
	}
    
    bool bCheckSubZone = false;
	if ( mg_get_var( conn, "check_subzone", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bCheckSubZone = true;
	}
    
    wxString strAllowedFrom;
	if ( mg_get_var( conn, "allowedfrom", buf, sizeof( buf ) ) > 0 ) {
		strAllowedFrom = wxString::FromAscii( buf );	
		strAllowedFrom.Trim( true );
		strAllowedFrom.Trim( false );
		if ( _("*") == strAllowedFrom ) {
			strAllowedFrom = _("0000-01-01 00:00:00");
		}
	}
    
    wxString strAllowedTo;
	if ( mg_get_var( conn, "allowedto", buf, sizeof( buf ) ) > 0 ) {
		strAllowedTo = wxString::FromAscii( buf );	
		strAllowedTo.Trim( true );
		strAllowedTo.Trim( false );
		if ( _("*") == strAllowedTo ) {
			strAllowedTo = _("9999-12-31 23:59:59");
		}
	}
    
    wxString strAllowedTime;
	if ( mg_get_var( conn, "allowedtime", buf, sizeof( buf ) ) > 0 ) {
		strAllowedTime = wxString::FromAscii( buf );  	
		strAllowedTime.Trim( true );
		strAllowedTime.Trim( false );
		if ( _("*") == strAllowedTime ) {
			strAllowedTime = _("* *");
		}
	}
    
    bool bCheckMonday = false;
	if ( mg_get_var( conn, "monday", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bCheckMonday = true;
	}
    
    bool bCheckTuesday = false;
	if ( mg_get_var( conn, "tuesday", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bCheckTuesday = true;
	}
    
    bool bCheckWednesday = false;
	if ( mg_get_var( conn, "wednesday", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bCheckWednesday = true;
	}
    
    bool bCheckThursday = false;
	if ( mg_get_var( conn, "thursday", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bCheckThursday = true;
	}
    
    bool bCheckFriday = false;
	if ( mg_get_var( conn, "friday", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bCheckFriday = true;
	}
    
    bool bCheckSaturday = false;
	if ( mg_get_var( conn, "saturday", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bCheckSaturday = true;
	}
    
    bool bCheckSunday = false;
	if ( mg_get_var( conn, "sunday", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bCheckSunday = true;
	}
    
    uint32_t action = 0;
	if ( mg_get_var( conn, "action", buf, sizeof( buf ) ) > 0 ) {
		action = vscp_readStringValue( wxString::FromAscii( buf ) );
	}
    
    wxString strActionParameter;
	if ( mg_get_var( conn, "actionparameter", buf, sizeof( buf ) ) > 0 ) {
		strActionParameter = wxString::FromAscii( buf );
	}
    
    wxString strComment;
	if ( mg_get_var( conn, "comment", buf, sizeof( buf ) ) > 0 ) {
		strComment = wxString::FromAscii( buf );
	}
    
    wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Decision Matrix Post"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += _("<meta http-equiv=\"refresh\" content=\"2;url=/vscp/dm");
    buildPage += wxString::Format(_("?from=%l&count=%l"), (long)nCount );
    buildPage += _("\">");
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    // Insert server url into navigation menu 
    wxString navstr = _(WEB_COMMON_MENU);
    int pos;
    while ( wxNOT_FOUND != ( pos = navstr.Find(_("%s")))) {
        buildPage += navstr.Left( pos );
        navstr = navstr.Right(navstr.Length() - pos - 2);
    }
    buildPage += navstr;
    
    buildPage += _(WEB_DMPOST_BODY_START);
        
    if (bNew) {
        pElement = new dmElement;
    }

    if ( bNew || ( id >= 0 ) ) {

        if ( bNew || ((0 == id) && !bNew) || ( id < pObject->m_dm.getElementCount() ) ) {

            if (!bNew) pElement = pObject->m_dm.getElement(id);

            if (NULL != pElement) {

                if ( bEnableRow ) {
                    pElement->enableRow();
                }
                else {
                    pElement->disableRow();
                }

                if (-1 == filter_priority) {
                    pElement->m_vscpfilter.mask_priority = 0;
                    pElement->m_vscpfilter.filter_priority = 0;
                } 
                else {
                    pElement->m_vscpfilter.mask_priority = mask_priority;
                    pElement->m_vscpfilter.filter_priority = filter_priority;
                }

                if (-1 == filter_vscpclass) {
                    pElement->m_vscpfilter.mask_class = 0;
                    pElement->m_vscpfilter.filter_class = 0;
                } 
                else {
                    pElement->m_vscpfilter.mask_class = mask_vscpclass;
                    pElement->m_vscpfilter.filter_class = filter_vscpclass;
                }

                if (-1 == filter_vscptype) {
                    pElement->m_vscpfilter.mask_type = 0;
                    pElement->m_vscpfilter.filter_type = 0;
                } 
                else {
                    pElement->m_vscpfilter.mask_type = mask_vscptype;
                    pElement->m_vscpfilter.filter_type = filter_vscptype;
                }

                if (0 == strFilterGuid.Length()) {
                    for (int i = 0; i < 16; i++) {
                        pElement->m_vscpfilter.mask_GUID[i] = 0;
                        pElement->m_vscpfilter.filter_GUID[i] = 0;
                    }
                } 
                else {
                    vscp_getGuidFromStringToArray(pElement->m_vscpfilter.mask_GUID,
                            strMaskGuid);
                    vscp_getGuidFromStringToArray(pElement->m_vscpfilter.filter_GUID,
                            strFilterGuid);
                }

                pElement->m_index = index;
                pElement->m_zone = zone;
                pElement->m_bMeasurement = bUseMeasurementIndex;
                pElement->m_subzone = subzone;

                pElement->m_control = 0;
                if (bEndScan) pElement->m_control |= DM_CONTROL_DONT_CONTINUE_SCAN;
                if (bCheckIndex) pElement->m_control |= DM_CONTROL_CHECK_INDEX;
                if (bCheckZone) pElement->m_control |= DM_CONTROL_CHECK_ZONE;
                if (bCheckSubZone) pElement->m_control |= DM_CONTROL_CHECK_SUBZONE;

                pElement->m_timeAllow.m_fromTime.ParseDateTime( strAllowedFrom );
                pElement->m_timeAllow.m_endTime.ParseDateTime( strAllowedTo );
                pElement->m_timeAllow.parseActionTime( strAllowedTime );

                wxString weekdays;

                if (bCheckMonday) weekdays = _("m"); else weekdays = _("-");
                if (bCheckTuesday) weekdays += _("t"); else weekdays += _("-");
                if (bCheckWednesday) weekdays += _("w"); else weekdays += _("-");
                if (bCheckThursday) weekdays += _("t"); else weekdays += _("-");
                if (bCheckFriday) weekdays += _("f"); else weekdays += _("-");
                if (bCheckSaturday) weekdays += _("s"); else weekdays += _("-");
                if (bCheckSunday) weekdays += _("s"); else weekdays += _("-");
                pElement->m_timeAllow.setWeekDays(weekdays);

                pElement->m_action = action;

                pElement->m_actionparam = strActionParameter;
                pElement->m_comment = strComment;
                
                pElement->m_strGroupID = strGroupID;

                pElement->m_triggCounter = 0;
                pElement->m_errorCounter = 0;

                if ( bNew ) {
                    // add the DM row to the matrix
                    pObject->m_dm.addElement(pElement);
                }

                // Save decision matrix
                pObject->m_dm.save();

                buildPage += wxString::Format(_("<br><br>DM Entry has been saved. id=%d"), id);
            } 
			else {
                buildPage += wxString::Format(_("<br><br>Memory problem id=%d. Unable to save record"), id);
            }

        } else {
            buildPage += wxString::Format(_("<br><br>Record id=%d is to large. Unable to save record"), id);
        }
    } else {
        buildPage += wxString::Format(_("<br><br>Record id=%d is wrong. Unable to save record"), id);
    }

    buildPage += _(WEB_COMMON_END); // Common end code 

    // Server data
	mg_send_data( conn, buildPage.ToAscii(), buildPage.Length() );

	return MG_TRUE;	
}


///////////////////////////////////////////////////////////////////////////////
// websrv_dmdelete
//

int 
VSCPWebServerThread::websrv_dmdelete( struct mg_connection *conn )
{
	char buf[80];
	wxString str;
    VSCPInformation vscpinfo;
    dmElement *pElement = NULL;

	// Check pointer
	if (NULL == conn) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;
   
    // id
    long id = -1;
	if ( mg_get_var( conn, "id", buf, sizeof( buf ) ) > 0 ) {
		id = atoi( buf );
	}

    // From
    long nFrom = 0;
	if ( mg_get_var( conn, "from", buf, sizeof( buf ) ) > 0 ) { 
		nFrom = atoi( buf );
	}
 
    // Count
    uint16_t nCount = 50;
	if ( mg_get_var( conn, "count", buf, sizeof( buf ) ) > 0 ) { 
		nCount = atoi( buf );
	}
        
    wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Decision Matrix Delete"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += _("<meta http-equiv=\"refresh\" content=\"2;url=/vscp/dm");
    buildPage += wxString::Format(_("?from=%d&count=%d"), nFrom, nCount );
    buildPage += _("\">");
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    // Navigation menu 
    buildPage += _(WEB_COMMON_MENU);
    
    buildPage += _(WEB_DMEDIT_BODY_START);
    
    if ( pObject->m_dm.removeElement( id ) ) {
        buildPage += wxString::Format(_("<br>Deleted record id = %d"), id);
        // Save decision matrix
        pObject->m_dm.save();
    }
    else {
        buildPage += wxString::Format(_("<br>Failed to remove record id = %d"), id);
    }
    
    buildPage += _(WEB_COMMON_END);     // Common end code
    
    // Server data
	mg_send_data( conn, buildPage.ToAscii(), buildPage.Length() );
    
	return MG_TRUE;	
}


///////////////////////////////////////////////////////////////////////////////
// websrv_variables_list
//

int
VSCPWebServerThread::websrv_variables_list( struct mg_connection *conn )
{
	char buf[80];
    VSCPInformation vscpinfo;
    //unsigned long upperLimit = 50;

	// Check pointer
	if (NULL == conn) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;
              
    // From
    long nFrom = 0;
	if ( mg_get_var( conn, "from", buf, sizeof( buf ) ) > 0 ) {	
		nFrom = atoi( buf );
	}
    
    
    // Count
    unsigned long nCount = 50;
	if ( mg_get_var( conn, "count", buf, sizeof( buf ) ) > 0 ) {
		nCount = atoi( buf );
	}
    

    // Navigation button
	if ( mg_get_var( conn, "navbtn", buf, sizeof( buf ) ) > 0 ) { 
	
		if (NULL != strstr("previous", buf) ) {    
		    nFrom -= nCount;
			if ( nFrom < 0 )  nFrom = 0;
        }        
		else if (NULL != strstr("next",buf)) {
            nFrom += nCount;
            if ( (unsigned long)nFrom > pObject->m_VSCP_Variables.m_listVariable.GetCount()-1 ) {
                if ( pObject->m_VSCP_Variables.m_listVariable.GetCount() % nCount ) {
                    nFrom = pObject->m_VSCP_Variables.m_listVariable.GetCount()/nCount;
                }
                else {
                    nFrom = (pObject->m_VSCP_Variables.m_listVariable.GetCount()/nCount) - 1;
                }
            }
		}
		else if (NULL != strstr("last",buf)) {
            if ( pObject->m_VSCP_Variables.m_listVariable.GetCount() % nCount ) {
                nFrom = (pObject->m_VSCP_Variables.m_listVariable.GetCount()/nCount)*nCount;
            }
            else {
                nFrom = ((pObject->m_VSCP_Variables.m_listVariable.GetCount()/nCount) - 1)*nCount;
            }
		}
		else if ( NULL != strstr("first",buf) ) {
			nFrom = 0;
        }
	}
	else {  // No vaid navigation value
        //nFrom = 0;
    }


    wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Variables"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    // Navigation menu 
    buildPage += _(WEB_COMMON_MENU);   
    buildPage += _(WEB_VARLIST_BODY_START);

    {
        wxString wxstrurl = _("/vscp/variables");
        buildPage += wxString::Format( _(WEB_COMMON_LIST_NAVIGATION),
                (const char *)wxstrurl.c_str(),
                (unsigned long)(nFrom+1),
                ( (unsigned long)(nFrom + nCount) < pObject->m_VSCP_Variables.m_listVariable.GetCount()) ? 
                    nFrom + nCount : pObject->m_VSCP_Variables.m_listVariable.GetCount(),
                (unsigned long)pObject->m_VSCP_Variables.m_listVariable.GetCount(),
                (unsigned long)nCount,
                (unsigned long)nFrom,
                _("false" ) );
        buildPage += _("<br>");
    } 

    wxString strBuf;

    // Display Variables List
    
    if ( 0 == pObject->m_VSCP_Variables.m_listVariable.GetCount() ) {
        buildPage += _("<br>Variables list is empty!<br>");
    }
    else {
        buildPage += _(WEB_VARLIST_TR_HEAD);
    }
    
    if (nFrom < 0) nFrom = 0;
    
    for ( unsigned int i=0;i<nCount;i++) {
        
        // Check if we are done
        if ( ( nFrom + i ) >= pObject->m_VSCP_Variables.m_listVariable.GetCount() ) break;

        CVSCPVariable *pVariable = 
                pObject->m_VSCP_Variables.m_listVariable.Item( i )->GetData();
        {
            wxString url_dmedit = 
                    wxString::Format(_("/vscp/varedit?id=%l&from=%l&count=%l"),
                                        (long)(nFrom+i), (long)nFrom, (long)nCount );
            wxString str = wxString::Format(_(WEB_COMMON_TR_CLICKABLE_ROW),
                                                (const char *)url_dmedit.c_str() );
            buildPage += str;
        }

        // Client id    
        buildPage += _(WEB_IFLIST_TD_CENTERED);
        buildPage += wxString::Format(_("<form name=\"input\" action=\"/vscp/vardelete?id=%l\" method=\"get\"> %l <input type=\"submit\" value=\"x\"><input type=\"hidden\" name=\"id\"value=\"%l\"></form>"), 
                        (long)(nFrom+i), (long)(nFrom+i+1), (long)(nFrom+i) );
        buildPage += _("</td>");
        
        if (NULL != pVariable) {

            // Variable type
            buildPage += _("<td>");
            switch (pVariable->getType()) {

            case VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED:
                buildPage += _("Unassigned");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_STRING:
                buildPage += _("String");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_BOOLEAN:
                buildPage += _("Boolean");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_INTEGER:
                buildPage += _("Integer");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_LONG:
                buildPage += _("Long");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_DOUBLE:
                buildPage += _("Double");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_MEASUREMENT:
                buildPage += _("Measurement");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT:
                buildPage += _("Event");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID:
                buildPage += _("GUID");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_DATA:
                buildPage += _("Event data");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_CLASS:
                buildPage += _("Event class");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TYPE:
                buildPage += _("Event type");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TIMESTAMP:
                buildPage += _("Event timestamp");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_DATETIME:
                buildPage += _("Date and time");
                break;

            default:
                buildPage += _("Unknown type");
                break;

            }
            buildPage += _("</td>");

            
            // Variable entry
            buildPage += _("<td>");

            buildPage += _("<div id=\"small\">");

            buildPage += _("<h4>");
            buildPage += pVariable->getName();
            buildPage += _("</h4>");
            
            wxString str;
            pVariable->writeValueToString(str);
            buildPage += _("<b>Value:</b> ");
            buildPage += str;
            
            buildPage += _("<br>");
            buildPage += _("<b>Note:</b> ");
            buildPage += pVariable->getNote();
            
            buildPage += _("<br>");
            buildPage += _("<b>Persistent: </b> ");
            if ( pVariable->isPersistent() ) {
                buildPage += _("yes");
            }
            else {
                buildPage += _("no");
            }

            buildPage += _("</div>");

        }
        else {
            buildPage += _("Internal error: Non existent variable entry.");
        }

        buildPage += _("</td>");
        buildPage += _("</tr>");

    } // for
       
    buildPage += _(WEB_DMLIST_TABLE_END);
    
    {
        wxString wxstrurl = _("/vscp/variables");
        buildPage += wxString::Format( _(WEB_COMMON_LIST_NAVIGATION),
                (const char *)wxstrurl.c_str(),
                (unsigned long)nFrom,
                ( (unsigned long)(nFrom + nCount) < pObject->m_VSCP_Variables.m_listVariable.GetCount()) ? 
                    nFrom + nCount - 1 : pObject->m_VSCP_Variables.m_listVariable.GetCount() - 1,
                (unsigned long)pObject->m_VSCP_Variables.m_listVariable.GetCount(),
                (unsigned long)nCount,
                (unsigned long)nFrom,
                _("false") );
    }
     
    buildPage += _(WEB_COMMON_END);     // Common end code
    
	// Server data
	mg_send_data( conn, buildPage.ToAscii(), buildPage.Length() );
    

	return MG_TRUE;	
}

///////////////////////////////////////////////////////////////////////////////
// websrv_variables_edit
//

int
VSCPWebServerThread::websrv_variables_edit( struct mg_connection *conn )
{
	char buf[80];
    wxString str;
    VSCPInformation vscpinfo;
    CVSCPVariable *pVariable = NULL;

	// Check pointer
	if (NULL == conn) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;
        
    // id
    long id = -1;
	if ( mg_get_var( conn, "id", buf, sizeof( buf ) ) > 0 ) {
		id = atoi( buf );
	}
    
    // type
    uint8_t nType = VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED;
	if ( mg_get_var( conn, "type", buf, sizeof( buf ) ) > 0 ) {
		nType = atoi( buf );
	}
    
    // Flag for new variable row
    bool bNew = false;
	if ( mg_get_var( conn, "new", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bNew = true;
	}

    // From
    long nFrom = 0;
	if ( mg_get_var( conn, "from", buf, sizeof( buf ) ) > 0 ) {	
		nFrom = atoi( buf );
	}
    
    
    // Count
    uint16_t nCount = 50;
	if ( mg_get_var( conn, "count", buf, sizeof( buf ) ) > 0 ) {
		nCount = atoi( buf );
	}
    
    wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Variable Edit"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    // Navigation menu 
    buildPage += _(WEB_COMMON_MENU);
    
    buildPage += _(WEB_VAREDIT_BODY_START);

    if ( !bNew && ( id < (long)pObject->m_VSCP_Variables.m_listVariable.GetCount() ) ) {
        pVariable = pObject->m_VSCP_Variables.m_listVariable.Item(id)->GetData();
    }

    if (bNew || (NULL != pVariable)) {
        
        if ( bNew ) {
            buildPage += _("<br><span id=\"optiontext\">New record.</span><br>");
        }
        else {
            buildPage += wxString::Format(_("<br><span id=\"optiontext\">Record = %d.</span><br>"), id);
        }
        
        buildPage += _("<br><form method=\"get\" action=\"");
        buildPage += _("/vscp/varpost");
        buildPage += _("\" name=\"varedit\">");
        
        // Hidden from
        buildPage += wxString::Format(_("<input name=\"from\" value=\"%d\" type=\"hidden\">"), nFrom );

        // Hidden count
        buildPage += wxString::Format(_("<input name=\"count\" value=\"%d\" type=\"hidden\">"), nCount );

        // Hidden id
        buildPage += wxString::Format(_("<input name=\"id\" value=\"%d\" type=\"hidden\">"), id );
        
        if (bNew) {     
            // Hidden new
            buildPage += _("<input name=\"new\" value=\"true\" type=\"hidden\">");
        }
        else {
            // Hidden new
            buildPage += _("<input name=\"new\" value=\"false\" type=\"hidden\">");
        }
        
        // Hidden type
        buildPage += _("<input name=\"type\" value=\"");
        buildPage += wxString::Format(_("%d"), ( bNew ? nType : pVariable->getType()) );
        buildPage += _("\" type=\"hidden\"></input>");
        
        buildPage += _("<h4>Variable:</h4> <span id=\"optiontext\"></span><br>");

        buildPage += _("<table class=\"invisable\"><tbody><tr class=\"invisable\">");

        buildPage += _("<td class=\"invisable\">Name:</td><td class=\"invisable\">");
        if ( !bNew ) {
            buildPage += pVariable->getName();
            buildPage += _("<input name=\"value_name\" value=\"");
            buildPage += pVariable->getName();
            buildPage += _("\" type=\"hidden\">");
        }
        else {
            buildPage += _("<textarea cols=\"50\" rows=\"1\" name=\"value_name\"></textarea>");
        }
        buildPage += _("</td></tr><tr>");
        buildPage += _("<td class=\"invisable\">Value:</td><td class=\"invisable\">");
        
        if (!bNew ) nType = pVariable->getType();
        
        if ( nType  == VSCP_DAEMON_VARIABLE_CODE_STRING ) {
            
            buildPage += _("<textarea cols=\"50\" rows=\"5\" name=\"value_string\">");
            if ( bNew ) {
                buildPage += _("");
            }
            else {
                wxString str;
                pVariable->getValue( str );
                buildPage += str;
            }
            
            buildPage += _("</textarea>");
            
        }
        else if ( nType == VSCP_DAEMON_VARIABLE_CODE_BOOLEAN ) {
            
            bool bValue = false;
            if ( !bNew ) pVariable->getValue( &bValue );
            
            buildPage += _("<input type=\"radio\" name=\"value_boolean\" value=\"true\" ");
            if ( !bNew ) 
                buildPage += wxString::Format(_("%s"), 
                                bValue ? _("checked >true ") : _(">true ") );
            else {
                buildPage += _(">true ");
            }
            
            buildPage += _("<input type=\"radio\" name=\"value_boolean\" value=\"false\" ");
            if ( !bNew ) 
                buildPage += wxString::Format(_("%s"), 
                                        !bValue ? _("checked >false ") : _(">false ") );
            else {
                buildPage += _(">false ");
            }
        }
        else if ( nType == VSCP_DAEMON_VARIABLE_CODE_INTEGER ) {
            
            buildPage += _("<textarea cols=\"10\" rows=\"1\" name=\"value_integer\">");
            if ( bNew ) {
                buildPage += _("");
            }
            else {
                int val;
                pVariable->getValue( &val );
                buildPage += wxString::Format(_("%d"), val );
            }
            
            buildPage += _("</textarea>");
            
        }
        else if ( nType == VSCP_DAEMON_VARIABLE_CODE_LONG ) {
            
            buildPage += _("<textarea cols=\"10\" rows=\"1\" name=\"value_long\">");
            if ( bNew ) {
                buildPage += _("");
            }
            else {
                long val;
                pVariable->getValue( &val );
                buildPage += wxString::Format(_("%ld"), val );
            }
            
            buildPage += _("</textarea>");
            
        }
        else if ( nType == VSCP_DAEMON_VARIABLE_CODE_DOUBLE ) {
            
            buildPage += _("<textarea cols=\"10\" rows=\"1\" name=\"value_double\">");
            if ( bNew ) {
                buildPage += _("");
            }
            else {
                double val;
                pVariable->getValue( &val );
                buildPage += wxString::Format(_("%f"), val );
            }
            
            buildPage += _("</textarea>");
            
        }
        else if ( nType == VSCP_DAEMON_VARIABLE_CODE_VSCP_MEASUREMENT ) {
            buildPage += _("<textarea cols=\"50\" rows=\"1\" name=\"value_measurement\">");
            if ( bNew ) {
                buildPage += _("");
            }
            else {
                wxString str;
                pVariable->writeValueToString( str );
                buildPage += str;
            }
            
            buildPage += _("</textarea>");
        }
        else if ( nType == VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT ) {
            
            buildPage += _("<table>");
            
            buildPage += _("<tr><td>");
            buildPage += _("VSCP class");
             buildPage += _("</td><td>");
            buildPage += _("<textarea cols=\"10\" rows=\"1\" name=\"value_class\">");
            if ( bNew ) {
                buildPage += _("");
            }
            else {
                buildPage += wxString::Format(_("0x%x"), pVariable->m_event.vscp_class );
            }
            
            buildPage += _("</textarea>");
            buildPage += _("</td></tr>");
            
            buildPage += _("<tr><td>");
            buildPage += _("VSCP type: ");
            buildPage += _("</td><td>");
            buildPage += _("<textarea cols=\"10\" rows=\"1\" name=\"value_type\">");
            if ( bNew ) {
                buildPage += _("");
            }
            else {
                buildPage += wxString::Format(_("0x%x"), pVariable->m_event.vscp_type );
            }
            
            buildPage += _("</textarea>");
            buildPage += _("</td></tr>");
            
            buildPage += _("<tr><td>");
            buildPage += _("GUID: ");
            buildPage += _("</td><td>");
            buildPage += _("<textarea cols=\"50\" rows=\"1\" name=\"value_guid\">");
            if ( bNew ) {
                buildPage += _("");
            }
            else {
                wxString strGUID;
                vscp_writeGuidArrayToString( pVariable->m_event.GUID, strGUID );
                buildPage += wxString::Format(_("%s"), (const char *)strGUID.c_str() );
            }
            
            buildPage += _("</textarea>");
            buildPage += _("</td></tr>");
            
            buildPage += _("<tr><td>");
            buildPage += _("Timestamp: ");
            buildPage += _("</td><td>");
            buildPage += _("<textarea cols=\"10\" rows=\"1\" name=\"value_timestamp\">");
            if ( bNew ) {
                buildPage += _("0");
            }
            else {
                buildPage += wxString::Format(_("0x%x"), pVariable->m_event.timestamp );
            }
            
            buildPage += _("</textarea>");
            buildPage += _("</td></tr>");
            
            buildPage += _("</textarea>");
            buildPage += _("</td></tr>");
            
            buildPage += _("<tr><td>");
            buildPage += _("OBID: ");
            buildPage += _("</td><td>");
            buildPage += _("<textarea cols=\"10\" rows=\"1\" name=\"value_obid\">");
            if ( bNew ) {
                buildPage += _("0");
            }
            else {
                buildPage += wxString::Format(_("0x%X"), pVariable->m_event.obid );
            }
            
            buildPage += _("</textarea>");
            buildPage += _("</td></tr>");
            
            buildPage += _("</textarea>");
            buildPage += _("</td></tr>");
            
            buildPage += _("</textarea>");
            buildPage += _("</td></tr>");
            
            buildPage += _("<tr><td>");
            buildPage += _("Head: ");
            buildPage += _("</td><td>");
            buildPage += _("<textarea cols=\"10\" rows=\"1\" name=\"value_head\">");
            if ( bNew ) {
                buildPage += _("0");
            }
            else {
                buildPage += wxString::Format(_("0x%02x"), pVariable->m_event.head );
            }
            
            buildPage += _("</textarea>");
            buildPage += _("</td></tr>");
            
            buildPage += _("<tr><td>");
            buildPage += _("CRC: ");
            buildPage += _("</td><td>");
            buildPage += _("<textarea cols=\"10\" rows=\"1\" name=\"value_crc\">");
            if ( bNew ) {
                buildPage += _("0");
            }
            else {
                buildPage += wxString::Format(_("0x%08x"), pVariable->m_event.crc );
            }
            
            buildPage += _("</textarea>");
            buildPage += _("</td></tr>");
            
            buildPage += _("</textarea>");
            buildPage += _("</td></tr>");
            
            buildPage += _("<tr><td>");
            buildPage += _("Data size: ");
            buildPage += _("</td><td>");
            //buildPage += _("<textarea cols=\"10\" rows=\"1\" name=\"value_sizedata\">");
            if ( bNew ) {
                buildPage += _("0");
            }
            else {
                buildPage += wxString::Format(_("%d"), pVariable->m_event.sizeData );
            }
            
            //buildPage += _("</textarea>");
            buildPage += _("</td></tr>");
            
            buildPage += _("<tr><td>");
            buildPage += _("Data: ");
            buildPage += _("</td><td>");
            buildPage += _("<textarea cols=\"50\" rows=\"4\" name=\"value_data\">");
            if ( bNew ) {
                buildPage += _("0");
            }
            else {
                wxString strData;
                vscp_writeVscpDataToString( &pVariable->m_event, strData );
                buildPage += wxString::Format(_("%s"), (const char *)strData.c_str() );
            }
            
            buildPage += _("</textarea>");
            buildPage += _("</td></tr>");
            
            buildPage += _("</table>");
            
        }
        else if ( nType == VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID ) {
            
            buildPage += _("<textarea cols=\"50\" rows=\"1\" name=\"value_guid\">");
            if ( bNew ) {
                buildPage += _("");
            }
            else {
                wxString strGUID;
                pVariable->writeValueToString(strGUID);
                buildPage += strGUID;
            }
            
            buildPage += _("</textarea>");
            
        }
        else if ( nType == VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_DATA ) {
            
            buildPage += _("<textarea cols=\"50\" rows=\"5\" name=\"value_data\">");
            if ( bNew ) {
                buildPage += _("");
            }
            else {
                wxString strData;
                vscp_writeVscpDataToString( &pVariable->m_event, strData );
                buildPage += strData;
            }
            
            buildPage += _("</textarea>");
            
        }
        else if ( nType == VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_CLASS ) {
            
            buildPage += _("<textarea cols=\"10\" rows=\"1\" name=\"value_class\">");
            if ( bNew ) {
                buildPage += _("");
            }
            else {
                wxString str;
                pVariable->writeValueToString( str );
                buildPage += str;
            }
            
            buildPage += _("</textarea>");
            
        }
        else if ( nType == VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TYPE ) {
            
            buildPage += _("<textarea cols=\"10\" rows=\"1\" name=\"value_type\">");
            if ( bNew ) {
                buildPage += _("");
            }
            else {
                wxString str;
                pVariable->writeValueToString( str );
                buildPage += str;
            }
            
            buildPage += _("</textarea>");
            
        }
        else if ( nType == VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TIMESTAMP ) {
            
            buildPage += _("<textarea cols=\"10\" rows=\"1\" name=\"value_timestamp\">");
            if ( bNew ) {
                buildPage += _("");
            }
            else {
                wxString str;
                pVariable->writeValueToString( str );
                buildPage += str;
            }
            
            buildPage += _("</textarea>");
            
        }
        else if ( nType == VSCP_DAEMON_VARIABLE_CODE_DATETIME ) {
            
            buildPage += _("<textarea cols=\"20\" rows=\"1\" name=\"value_date_time\">");
            if ( bNew ) {
                buildPage += _("");
            }
            else {
                wxString str;
                pVariable->writeValueToString( str );
                buildPage += str;
            }
            
            buildPage += _("</textarea>");
            
        }
        else {
            // Invalid type
            buildPage += _("Invalid type - Something is very wrong!");
        }
        
        
        buildPage += _("</tr><tr><td>Persistence: </td><td>");

        buildPage += _("<input type=\"radio\" name=\"persistent\" value=\"true\" ");
        
        if ( !bNew ) {
            buildPage += wxString::Format(_("%s"), 
                pVariable->isPersistent() ? _("checked >Persistent ") : _(">Persistent ") );
        }
        else {
            buildPage += _("checked >Persistent ");
        }
        
         buildPage += _("<input type=\"radio\" name=\"persistent\" value=\"false\" ");
         
        if ( !bNew ) {
            buildPage += wxString::Format(_("%s"), 
                !pVariable->isPersistent() ? _("checked >Non persistent ") : _(">Non persistent ") );
        }
        else {
            buildPage += _(">Non persistent ");
        }
        
        buildPage += _("</td></tr>");
        
        
        buildPage += _("</tr><tr><td>Note: </td><td>");
        buildPage += _("<textarea cols=\"50\" rows=\"5\" name=\"note\">");
        if (bNew) {
            buildPage += _("");
        } 
        else {
            buildPage += pVariable->getNote();
        }

        buildPage += _("</textarea>");

        buildPage += _("</td></tr></table>");

        buildPage += _(WEB_VAREDIT_TABLE_END);

    } 
    else {
        buildPage += _("<br><b>Error: Non existent id</b>");
    }
    
    
    wxString wxstrurl = _("/vscp/varpost");
    buildPage += wxString::Format( _(WEB_VAREDIT_SUBMIT),
                                    (const char *)wxstrurl.c_str() );
    
    buildPage += _("</form>");
    buildPage += _(WEB_COMMON_END);     // Common end code
    
	// Server data
	mg_send_data( conn, buildPage.ToAscii(), buildPage.Length() );

	return MG_TRUE;	
}


///////////////////////////////////////////////////////////////////////////////
// websrv_variables_post
//

int
VSCPWebServerThread::websrv_variables_post( struct mg_connection *conn )
{
	char buf[32000];
    wxString str;
    VSCPInformation vscpinfo;
    CVSCPVariable *pVariable = NULL;
    
    // Check pointer
	if (NULL == conn) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

    // id
    long id = -1;
	if ( mg_get_var( conn, "id", buf, sizeof( buf ) ) > 0 ) {
		id = atoi( buf );
	}

    // From
    long nFrom = 0;
	if ( mg_get_var( conn, "from", buf, sizeof( buf ) ) > 0 ) {	
		nFrom = atoi( buf );
	}
    
    // Count
    uint16_t nCount = 50;
	if ( mg_get_var( conn, "count", buf, sizeof( buf ) ) > 0 ) {
		nCount = atoi( buf );
	}
    
    uint8_t nType = VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED;
	if ( mg_get_var( conn, "type", buf, sizeof( buf ) ) > 0 ) {
		nType = atoi( buf );
	}
    
    wxString strName;
	if ( mg_get_var( conn, "value_name", buf, sizeof( buf ) ) > 0 ) {
		strName = wxString::FromAscii( buf );
	}
    
    // Flag for new variable row
    bool bNew = false;
	if ( mg_get_var( conn, "new", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bNew = true;
	}
    
    // Flag for persistence
    bool bPersistent = true;
	if ( mg_get_var( conn, "persistent", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "false", buf ) ) bPersistent = false;
	}

    wxString strNote;
	if ( mg_get_var( conn, "note", buf, sizeof( buf ) ) > 0 ) {
		strNote = wxString::FromAscii( buf );
	}
    
    wxString strValueString;
	if ( mg_get_var( conn, "value_string", buf, sizeof( buf ) ) > 0 ) {
		strValueString = wxString::FromAscii( buf );
	}
               
    bool bValueBoolean = false;
	if ( mg_get_var( conn, "value_boolean", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bValueBoolean = true;
	}
    
    int value_integer = 0;
	if ( mg_get_var( conn, "value_integer", buf, sizeof( buf ) ) > 0 ) {
		str = wxString::FromAscii( buf );
		value_integer = vscp_readStringValue( str );
	}
       
    long value_long = 0;
	if ( mg_get_var( conn, "value_long", buf, sizeof( buf ) ) > 0 ) {
		str = wxString::FromAscii( buf );
		value_long = vscp_readStringValue( str );
	}
    
    double value_double = 0;
	if ( mg_get_var( conn, "value_double", buf, sizeof( buf ) ) > 0 ) {
		value_double = atof( buf );
	}
    
    wxString strMeasurement;
	if ( mg_get_var( conn, "value_measurement", buf, sizeof( buf ) ) > 0 ) {
		strMeasurement = wxString::FromAscii( buf );
	}
    
    uint16_t value_class = 0;
	if ( mg_get_var( conn, "value_class", buf, sizeof( buf ) ) > 0 ) {
		value_class = vscp_readStringValue( wxString::FromAscii( buf ) );
	}
    
    uint16_t value_type = 0;
	if ( mg_get_var( conn, "value_type", buf, sizeof( buf ) ) > 0 ) {
		value_type = vscp_readStringValue( wxString::FromAscii( buf ) );
	}
    
    wxString strGUID;
	if ( mg_get_var( conn, "value_guid", buf, sizeof( buf ) ) > 0 ) {
		strGUID = wxString::FromAscii( buf );
        strGUID.Trim();
        if ( 0 == strGUID.Length() ) {
            strGUID = _("-");
        }
	}
    
    uint32_t value_timestamp = 0;
	if ( mg_get_var( conn, "value_timestamp", buf, sizeof( buf ) ) > 0 ) {
		value_timestamp = vscp_readStringValue( wxString::FromAscii( buf ) );
	}
    
    uint32_t value_obid = 0;
	if ( mg_get_var( conn, "value_obid", buf, sizeof( buf ) ) > 0 ) {
		value_obid = vscp_readStringValue( wxString::FromAscii( buf ) );
	}
    
    uint8_t value_head = 0;
	if ( mg_get_var( conn, "value_head", buf, sizeof( buf ) ) > 0 ) {
		value_head = vscp_readStringValue( wxString::FromAscii( buf ) );
	}
    
    uint32_t value_crc = 0;
	if ( mg_get_var( conn, "value_crc", buf, sizeof( buf ) ) > 0 ) {
		value_crc = vscp_readStringValue( wxString::FromAscii( buf ) );
	}
    
    uint16_t value_sizedata = 0;
	if ( mg_get_var( conn, "value_sizedata", buf, sizeof( buf ) ) > 0 ) {
		value_sizedata = vscp_readStringValue( wxString::FromAscii( buf ) );
	}
    
    wxString strData;
	if ( mg_get_var( conn, "value_data", buf, sizeof( buf ) ) > 0 ) {
		strData = wxString::FromAscii( buf );
	}
    
    wxString strDateTime;
	if ( mg_get_var( conn, "value_date_time", buf, sizeof( buf ) ) > 0 ) {
		strDateTime = wxString::FromAscii( buf );
	}
    
    wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Variable Post"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += _("<meta http-equiv=\"refresh\" content=\"1;url=/vscp/variables");
    buildPage += wxString::Format(_("?from=%d&count=%d"), nFrom, nCount );
    buildPage += _("\">");
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    // Navigation menu 
    buildPage += _(WEB_COMMON_MENU);
    
    buildPage += _(WEB_VARPOST_BODY_START);
        
    if (bNew) {
        pVariable = new CVSCPVariable;
    }

    if (bNew || (id >= 0)) {

        if (bNew || 
                ((0 == id) && !bNew) || 
                (id < (long)pObject->m_VSCP_Variables.m_listVariable.GetCount()) ) {

            if (!bNew) pVariable = pObject->m_VSCP_Variables.m_listVariable.Item(id)->GetData();

            if (NULL != pVariable) {

                // Set the type
                pVariable->setPersistent( bPersistent );
                pVariable->setType( nType );
                pVariable->m_note = strNote;
                pVariable->setName( strName );
                
                switch ( nType ) {

                case VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED:
                    buildPage += _("Error: Variable code is unassigned.<br>");
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_STRING:
                    pVariable->setValue( strValueString );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_BOOLEAN:
                    pVariable->setValue( bValueBoolean );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_INTEGER:
                    pVariable->setValue( value_integer );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_LONG:
                    pVariable->setValue( value_long );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_DOUBLE:
                    pVariable->setValue( value_double );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_VSCP_MEASUREMENT:
                    uint16_t size;
                    vscp_setVscpDataArrayFromString( pVariable->m_normInteger, 
                                                    &size,
                                                    strMeasurement );
                    pVariable->m_normIntSize = size;
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT:
                    pVariable->m_event.vscp_class = value_class;
                    pVariable->m_event.vscp_type = value_type;
                    vscp_getGuidFromStringToArray( pVariable->m_event.GUID, strGUID );
                    vscp_setVscpDataFromString( &pVariable->m_event,
                                            strData );
                    pVariable->m_event.crc = value_crc;
                    pVariable->m_event.head = value_head;
                    pVariable->m_event.obid = value_obid;
                    pVariable->m_event.timestamp = value_timestamp;        
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID:
                    vscp_getGuidFromStringToArray( pVariable->m_event.GUID, strGUID );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_DATA:
                    vscp_setVscpDataFromString( &pVariable->m_event,
                                            strData );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_CLASS:
                    pVariable->m_event.vscp_class = value_class;
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TYPE:
                    pVariable->m_event.vscp_type = value_type;
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TIMESTAMP:
                    pVariable->m_event.timestamp = value_timestamp;
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_DATETIME:
                    pVariable->m_timestamp.ParseDateTime( strDateTime );
                    break;

                default:
                    buildPage += _("Error: Variable code is unknown.<br>");
                    break;

                }
                
                // If new variable add it
                if (bNew ) {
                    pObject->m_VSCP_Variables.add( pVariable );
                }

                // Save variables
                pObject->m_VSCP_Variables.save();
                
                buildPage += wxString::Format(_("<br><br>Variable has been saved. id=%d"), id);

            }
            else {
                buildPage += wxString::Format(_("<br><br>Memory problem id=%d. Unable to save record"), id);
            }

        } 
        else {
            buildPage += wxString::Format(_("<br><br>Record id=%d is to large. Unable to save record"), id);
        }
    } 
    else {
        buildPage += wxString::Format(_("<br><br>Record id=%d is wrong. Unable to save record"), id);
    }


    buildPage += _(WEB_COMMON_END); // Common end code 
    
	// Server data
	mg_send_data( conn, buildPage.ToAscii(), buildPage.Length() );

	return MG_TRUE;	
}


///////////////////////////////////////////////////////////////////////////////
// websrv_variables_new
//

int
VSCPWebServerThread::websrv_variables_new( struct mg_connection *conn )
{
    wxString str;
    char buf[80];
    VSCPInformation vscpinfo;
    CVSCPVariable *pVariable = NULL;

	// Check pointer
	if (NULL == conn) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

    // Count
    uint16_t nCount = 50;
	if ( mg_get_var( conn, "count", buf, sizeof( buf ) ) > 0 ) {
		nCount = atoi( buf );
	}
         
    wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - New variable"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    // Navigation menu 
    buildPage += _(WEB_COMMON_MENU);
    
    buildPage += _(WEB_VAREDIT_BODY_START);

    buildPage += _("<br><div style=\"text-align:center\">");
    
    buildPage += _("<br><form method=\"get\" action=\"");
    buildPage += _("/vscp/varedit");
    buildPage += _("\" name=\"varnewstep1\">");

    long nFrom;
    if ( pObject->m_VSCP_Variables.m_listVariable.GetCount() % nCount ) {
        nFrom = (pObject->m_VSCP_Variables.m_listVariable.GetCount()/nCount)*nCount;
    }
    else {
        nFrom = ((pObject->m_VSCP_Variables.m_listVariable.GetCount()/nCount) - 1)*nCount;
    }
    
    buildPage += wxString::Format( _("<input name=\"from\" value=\"%d\" type=\"hidden\">"), nFrom );
    buildPage += wxString::Format( _("<input name=\"count\" value=\"%d\" type=\"hidden\">"), nCount );

    buildPage += _("<input name=\"new\" value=\"true\" type=\"hidden\">");
    
    buildPage += _("<select name=\"type\">");
    buildPage += _("<option value=\"1\">String value</option>");
    buildPage += _("<option value=\"2\">Boolean value</option>");
    buildPage += _("<option value=\"3\">Integer value</option>");
    buildPage += _("<option value=\"4\">Long value</option>");
    buildPage += _("<option value=\"5\">Floating point value</option>");
    buildPage += _("<option value=\"6\">VSCP data coding</option>");
    buildPage += _("<option value=\"7\">VSCP event (Level II)</option>");
    buildPage += _("<option value=\"8\">VSCP event GUID</option>");
    buildPage += _("<option value=\"9\">VSCP event data</option>");
    buildPage += _("<option value=\"10\">VSCP event class</option>");
    buildPage += _("<option value=\"11\">VSCP event type</option>");
    buildPage += _("<option value=\"12\">VSCP event timestamp</option>");
    buildPage += _("<option value=\"13\">Date + Time in iso format</option>");
    buildPage += _("</select>");
    
    buildPage += _("<br></div>");
    buildPage += _(WEB_VARNEW_SUBMIT);
    //wxString wxstrurl = wxString::Format(_("%s/vscp/varedit?new=true"), 
    //                                            (const char *)strHost.c_str() );
    //buildPage += wxString::Format( _(WEB_VARNEW_SUBMIT),
    //                                (const char *)wxstrurl.c_str() );
    
    buildPage += _("</form>");
    
    buildPage += _(WEB_COMMON_END); // Common end code
    
    // Server data
    mg_send_data( conn, buildPage.ToAscii(), buildPage.Length() );
    
    return MG_TRUE;	
}


///////////////////////////////////////////////////////////////////////////////
// websrv_variables_delete
//

int
VSCPWebServerThread::websrv_variables_delete( struct mg_connection *conn )
{
	char buf[80];
    wxString str;
    VSCPInformation vscpinfo;
    CVSCPVariable *pVariable = NULL;

	// Check pointer
	if (NULL == conn) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

    // id
    long id = -1;
	if ( mg_get_var( conn, "id", buf, sizeof( buf ) ) > 0 ) {
		id = atoi( buf );
	}

    // From
    long nFrom = 0;
	if ( mg_get_var( conn, "from", buf, sizeof( buf ) ) > 0 ) {	
		nFrom = atoi( buf );
	}
    
    // Count
    uint16_t nCount = 50;
	if ( mg_get_var( conn, "count", buf, sizeof( buf ) ) > 0 ) {
		nCount = atoi( buf );
	}
        
    wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Variable Delete"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += _("<meta http-equiv=\"refresh\" content=\"2;url=/vscp/variables");
    buildPage += wxString::Format(_("?from=%d&count=%d"), nFrom, nCount );
    buildPage += _("\">");
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    // navigation menu 
    buildPage += _(WEB_COMMON_MENU);
    
    buildPage += _(WEB_VAREDIT_BODY_START);
    
    pVariable = pObject->m_VSCP_Variables.m_listVariable.Item(id)->GetData();
    //wxlistVscpVariableNode *node = 
    //        pObject->m_VSCP_Variables.m_listVariable.Item( id );
    //if ( pObject->m_VSCP_Variables.m_listVariable.DeleteNode( node )  ) {//
    if ( pObject->m_VSCP_Variables.remove( pVariable ) )  {
        buildPage += wxString::Format(_("<br>Deleted record id = %d"), id);
        // Save variables
        pObject->m_VSCP_Variables.save();
    }
    else {
        buildPage += wxString::Format(_("<br>Failed to remove record id = %d"), id);
    }
    
    buildPage += _(WEB_COMMON_END);     // Common end code
    
	// Server data
	mg_send_data( conn, buildPage.ToAscii(), buildPage.Length() );

	return MG_TRUE;	
}


///////////////////////////////////////////////////////////////////////////////
// websrv_discovery
//

int
VSCPWebServerThread::websrv_discovery( struct mg_connection *conn )
{
	//char buf[80];
    wxString str;
    VSCPInformation vscpinfo;
    CVSCPVariable *pVariable = NULL;

	// Check pointer
	if (NULL == conn) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

	wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Device discovery"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
	 buildPage += _("<meta http-equiv=\"refresh\" content=\"5;url=/vscp");
    buildPage += _("\">");
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    // navigation menu 
    buildPage += _(WEB_COMMON_MENU);
	buildPage += _("<b>Device discovery functionality is not yet implemented!</b>");
    
	// Server data
	mg_send_data( conn, buildPage.ToAscii(), buildPage.Length() );

	return MG_TRUE;	
}


///////////////////////////////////////////////////////////////////////////////
// websrv_session
//

int
VSCPWebServerThread::websrv_session( struct mg_connection *conn )
{
	//char buf[80];
    wxString str;
    VSCPInformation vscpinfo;
    CVSCPVariable *pVariable = NULL;

	// Check pointer
	if (NULL == conn) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

	wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Session"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
	 buildPage += _("<meta http-equiv=\"refresh\" content=\"5;url=/vscp");
    buildPage += _("\">");
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    // navigation menu 
    buildPage += _(WEB_COMMON_MENU);
	buildPage += _("<b>Session functionality is not yet implemented!</b>");
    
	// Server data
	mg_send_data( conn, buildPage.ToAscii(), buildPage.Length() );

	return MG_TRUE;	
}


///////////////////////////////////////////////////////////////////////////////
// websrv_configure
//

int
VSCPWebServerThread::websrv_configure( struct mg_connection *conn )
{
    wxString str;
    VSCPInformation vscpinfo;
    CVSCPVariable *pVariable = NULL;

	// Check pointer
	if (NULL == conn) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

	wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Configuration"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    // navigation menu 
    buildPage += _(WEB_COMMON_MENU);

    buildPage += _("<br><br><br>");

    buildPage += _("<div id=\"small\"><span style=\"color: blue;\"><b>VSCP Daemon version:</b></span> ");
    buildPage += _(VSCPD_DISPLAY_VERSION);
    buildPage += _("<br>");
    buildPage += _("</div>");

    buildPage += _("<div id=\"small\"><span style=\"color: blue;\"><b>Operating system:</b></span> ");
    buildPage += wxGetOsDescription();
    if ( wxIsPlatform64Bit() ) {
        buildPage += _(" 64-bit ");
    }
    else {
        buildPage += _(" 32-bit ");
    }
    if ( wxIsPlatformLittleEndian() ) {
        buildPage += _(" Little endian ");
    }
    else {
        buildPage += _(" Big endian ");
    }
    buildPage += _("<br>");
    buildPage += _("</div>");
	

#ifndef WIN32
/*	
    buildPage += _("<div id=\"small\">");
    wxLinuxDistributionInfo info = wxGetLinuxDistributionInfo();
    buildPage += _("<span style=\"color: blue;\"><b>Linux distribution Info:</b></span><br>");
    buildPage += _("&nbsp;&nbsp;<b>id=:</b> ");
    buildPage += info.id;
    buildPage += _("&nbsp;&nbsp;<span style=\"color: blue;\"><b>release=:</b></span><br>");
    buildPage += info.Release;
    buildPage += _("&nbsp;&nbsp;<span style=\"color: blue;\"><b>codeName=:</b></span><br>");
    buildPage += info.Codename;
    buildPage += _("&nbsp;&nbsp;<span style=\"color: blue;\"><b>description=:</b></span><br>");
    buildPage += info.Description;	
    buildPage += _("<br>");
    buildPage += _("</dic>");
 */ 
#endif

    buildPage += _("<div id=\"small\">");
    wxMemorySize memsize;
    if ( -1 != ( memsize = wxGetFreeMemory() ) ) {
        buildPage += _("<b>Free memory:</b> ");
        buildPage += memsize.ToString();
        buildPage += _(" bytes<br>");
    }
    buildPage += _("</div>");

    buildPage += _("<div id=\"small\">");
    buildPage += _("<b>Hostname:</b> ");
    buildPage += wxGetFullHostName();	
    buildPage += _("<br>");
    buildPage += _("</div>");
	
	buildPage += _("<div id=\"small\">");
    buildPage += _("<b>wxWidgets version:</b> ");
    buildPage += wxString::Format(_("%d.%d.%d.%d"), 
						wxMAJOR_VERSION,
						wxMINOR_VERSION,
						wxRELEASE_NUMBER,
						wxSUBRELEASE_NUMBER );	
    buildPage += _("<br>");
    buildPage += _("</div>");

    buildPage += _("<hr>");

    // Debuglevel
    buildPage += _("<div id=\"small\">");
    buildPage += _("<b>Debuglevel:</b> ");
    buildPage += wxString::Format(_("%d "), pObject->m_logLevel );
    switch ( pObject->m_logLevel  ) {
        case DAEMON_LOGMSG_NONE:
            buildPage += _("(none)");
            break;
        case DAEMON_LOGMSG_DEBUG:
            buildPage += _("(debug)");
            break;
        case DAEMON_LOGMSG_INFO:
            buildPage += _("(info)");
            break;
        case DAEMON_LOGMSG_NOTICE:
            buildPage += _("(notice)");
            break;
        case DAEMON_LOGMSG_WARNING:
            buildPage += _("(warning)");
            break;
        case DAEMON_LOGMSG_ERROR:
            buildPage += _("(error)");
            break;
        case DAEMON_LOGMSG_CRITICAL:
            buildPage += _("(critical)");
            break;
        case DAEMON_LOGMSG_ALERT:
            buildPage += _("(alert)");
            break;
        case DAEMON_LOGMSG_EMERGENCY:
            buildPage += _("(emergency)");
            break;
        default:
            buildPage += _("(unkown)");
            break;
    }
    buildPage += _("</div>");

    // Server GUID
    buildPage += _("<div id=\"small\">");
    buildPage += _("<b>Server GUID:</b> ");
    pObject->m_guid.toString( str );
    buildPage += str;
    buildPage += _("</div>");

    // Client buffer size
    buildPage += _("<div id=\"small\">");
    buildPage += _("<b>Client buffer size:</b> ");
    buildPage += wxString::Format(_("%d"), pObject->m_maxItemsInClientReceiveQueue );
    buildPage += _("</div>");

    buildPage += _("<hr>");

    // TCP/IP interface
    buildPage += _("<div id=\"small\">");
    buildPage += _("<b>TCP/IP interface:</b> ");
    if ( pObject->m_bTCPInterface ) {
        buildPage += _("enabled on <b>interface:</b> '");
        buildPage += pObject->m_strTcpInterfaceAddress;
        buildPage += _("'");
    }
    else {
        buildPage += _("disabled");
    }
    buildPage += _("<br>");
    buildPage += _("</div>");

    buildPage += _("<hr>");

    // UDP interface
    buildPage += _("<div id=\"small\">");
    buildPage += _("<b>UDP interface:</b> ");
    if ( pObject->m_bUDPInterface ) {
        buildPage += _("enabled on <b>interface:</b> '");
        buildPage += pObject->m_strUDPInterfaceAddress;
        buildPage += _("'");
    }
    else {
        buildPage += _("disabled");
    }
    buildPage += _("<br>");
    buildPage += _("</div>");

    buildPage += _("<hr>");

    // VSCP Internal intelligence
    buildPage += _("<div id=\"small\">");
    buildPage += _("<b>VSCP internal event logic:</b> ");
    if ( pObject->m_bVSCPDaemon ) {
        buildPage += _("enabled.");
    }
    else {
        buildPage += _("disabled");
    }
    buildPage += _("<br>");
    buildPage += _("&nbsp;&nbsp;<b>Automation:</b> ");
    if ( pObject->m_automation.isAutomationEnabled() ) {
        buildPage += _("enabled.");
    }
    else {
        buildPage += _("disabled");
    }
    buildPage += _("</div>");

    buildPage += _("<hr>");

    // Web server interface
    buildPage += _("<div id=\"small\">");
    buildPage += _("<b>Web server <b>interface:</b></b> ");
    if ( pObject->m_bWebServer ) {
        buildPage += _("enabled on interface '");
        buildPage += pObject->m_portWebServer;
        buildPage += _("<br>");
        buildPage += _("&nbsp;&nbsp;<b>Rootfolder:</b> ");
        buildPage += pObject->m_pathWebRoot;
        buildPage += _("<br>");
        buildPage += _("&nbsp;&nbsp;<b>Authdomain:</b> ");
        buildPage += pObject->m_authDomain;
        if ( 0 == pObject->m_authDomain.Length() ) {
            buildPage += _("Set to default.");
        }
        buildPage += _("<br>");
        buildPage += _("&nbsp;&nbsp;<b>ExtraMimeTypes:</b> ");
        buildPage += pObject->m_extraMimeTypes;
        if ( 0 == pObject->m_extraMimeTypes.Length() ) {
            buildPage += _("Set to default.");
        }
        buildPage += _("<br>");
        buildPage += _("&nbsp;&nbsp;<b>CgiInterpreter:</b> ");
        buildPage += pObject->m_cgiInterpreter;
        if ( 0 == pObject->m_cgiInterpreter.Length() ) {
            buildPage += _("Set to default.");
        }
        buildPage += _("<br>");
        buildPage += _("&nbsp;&nbsp;<b>CgiPattern:</b> ");
        buildPage += pObject->m_cgiPattern;
        if ( 0 == pObject->m_cgiPattern.Length() ) {
            buildPage += _("Set to default.");
        }
        buildPage += _("<br>");
        buildPage += _("&nbsp;&nbsp;<b>HiddenFilePatterns:</b> ");
        buildPage += pObject->m_hideFilePatterns;
        if ( 0 == pObject->m_hideFilePatterns.Length() ) {
            buildPage += _("Set to default.");
        }
        buildPage += _("<br>");
        buildPage += _("&nbsp;&nbsp;<b>IndeFiles:</b> ");
        buildPage += pObject->m_indexFiles;
        if ( 0 == pObject->m_indexFiles.Length() ) {
            buildPage += _("Set to default.");
        }
        buildPage += _("<br>");
        if ( pObject->bEnableDirectoryListing ) {
            buildPage += _("&nbsp;&nbsp;<b>Directory listings</b> is enabled.");    
        }
        else {
            buildPage += _("&nbsp;&nbsp;<b>Directory listings</b> is disabled.");
        }
        buildPage += _("<br>");
        buildPage += _("&nbsp;&nbsp;<b>UrlReqrites:</b> ");
        buildPage += pObject->m_urlRewrites;
        if ( 0 == pObject->m_urlRewrites.Length() ) {
            buildPage += _("Set to default.");
        }
    }
    else {
        buildPage += _("disabled");
    }
    buildPage += _("</div>");

    buildPage += _("<hr>");

    // Websockets
    buildPage += _("<div id=\"small\">");
    buildPage += _("<b>VSCP websocket interface:</b> ");
    if ( pObject->m_bWebSockets ) {
        buildPage += _("enabled.");
        if ( pObject->m_bAuthWebsockets ) {
            buildPage += _("<b>Authentication</b> enabled.");
        }
        else {
            buildPage += _("<b>Authentication</b> disabled.");
        }
    }
    else {
        buildPage += _("disabled");
    }
    buildPage += _("</div>");

    buildPage += _("<hr>");

    // DM
    buildPage += _("<div id=\"small\">");
    buildPage += _("<b>Daemon internal decision matrix functionality:</b> ");
    if ( pObject->m_bDM ) {
        buildPage += _("enabled.");
        buildPage += _("<br>");
        buildPage += _("&nbsp;&nbsp;<b>Path to DM file:</b> ");
        buildPage += pObject->m_dm.m_configPath;
        buildPage += _("<br>");
        if ( pObject->m_dm.m_bLogEnable ) {
            buildPage += _("&nbsp;&nbsp;<b>DM logging</b> is enabled.");
            buildPage += _("<br>");
            buildPage += _("&nbsp;&nbsp;<b>Path to log file:</b> ");
            buildPage += pObject->m_dm.m_logFileName.GetFullPath();
        }
        else {
            buildPage += _("&nbsp;&nbsp;<b>DM logging</b> is disabled.");
        }
    }
    else {
        buildPage += _("disabled");
    }
    buildPage += _("</div>");

    buildPage += _("<hr>");

    // Variable handling
    buildPage += _("<div id=\"small\">");
    buildPage += _("<b>Variable handling :</b> ");
    if ( pObject->m_bVariables ) {
        buildPage += _("enabled.");
        buildPage += _("<br>");
        buildPage += _("&nbsp;&nbsp;<b>Path to variables:</b> ");
        buildPage += pObject->m_VSCP_Variables.m_configPath;
        buildPage += _("<br>");
        buildPage += _("&nbsp;&nbsp;<b>Autosave:</b> ");
        buildPage += wxString::Format(_("%d minutes."), pObject->m_VSCP_Variables.m_autosaveInterval );
    }
    else {
        buildPage += _("enabled.");
    }
    buildPage += _("</div>");

    buildPage += _("<hr>");

    // General log file
    buildPage += _("<div id=\"small\">");
    buildPage += _("<b>General log file:</b> ");
    if ( pObject->m_bLogGeneralEnable ) {
        buildPage += _("enabled, <b>path:</b> ");
        buildPage += pObject->m_logGeneralFileName.GetFullPath();
        buildPage += _("'");
    }
    else {
        buildPage += _("disabled.");
    }
    buildPage += _("<br>");
    buildPage += _("</div>");

    // Access log file
    buildPage += _("<div id=\"small\">");
    buildPage += _("<b>Access log file:</b> ");
    if ( pObject->m_bLogAccessEnable ) {
        buildPage += _("enabled, <b>path:</b> ");
        buildPage += pObject->m_logAccessFileName.GetFullPath();
        buildPage += _("'");
    }
    else {
        buildPage += _("disabled.");
    }
    buildPage += _("<br>");
    buildPage += _("</div>");

    // Security log file
    buildPage += _("<div id=\"small\">");
    buildPage += _("<b>Security log file:</b> ");
    if ( pObject->m_bLogSecurityEnable ) {
        buildPage += _("enabled, <b>path:</b> ");
        buildPage += pObject->m_logSecurityFileName.GetFullPath();
        buildPage += _("'");
    }
    else {
        buildPage += _("disabled.");
    }
    buildPage += _("<br>");
    buildPage += _("</div>");

    buildPage += _("<hr>");

    buildPage += _("<div id=\"small\">");
    buildPage += _("<b>Level I Drivers:</b> ");  
    if ( pObject->m_bEnableLevel1Drivers ) {

        buildPage += _("enabled<br>");

        CDeviceItem *pDeviceItem;
        VSCPDEVICELIST::iterator iter;
        for (iter = pObject->m_deviceList.m_devItemList.begin();
            iter != pObject->m_deviceList.m_devItemList.end();
            ++iter) {

            pDeviceItem = *iter;
            if  ( ( NULL != pDeviceItem ) &&   
                    ( CLIENT_ITEM_INTERFACE_TYPE_DRIVER_LEVEL2 == pDeviceItem->m_driverLevel ) &&
                    pDeviceItem->m_bEnable ) {
                buildPage += _("<b>Name:</b> ");
                buildPage += pDeviceItem->m_strName;
                buildPage += _("<br>");
                buildPage += _("<b>Config:</b> ");
                buildPage += pDeviceItem->m_strParameter;
                buildPage += _("<br>");
                buildPage += _("<b>Path:</b> ");
                buildPage += pDeviceItem->m_strPath;
                buildPage += _("<br>");
            }
        }
    }
    else {
        buildPage += _("disabled<br>");
    }
    buildPage += _("<br>");
    buildPage += _("</div>");

    buildPage += _("<hr>");


    buildPage += _("<div id=\"small\">");
    buildPage += _("<b>Level II Drivers:</b> ");  
    if ( pObject->m_bEnableLevel2Drivers ) {
        
        buildPage += _("enabled<br>");

        CDeviceItem *pDeviceItem;
        VSCPDEVICELIST::iterator iter;
        for (iter = pObject->m_deviceList.m_devItemList.begin();
            iter != pObject->m_deviceList.m_devItemList.end();
            ++iter) {

            pDeviceItem = *iter;
            if  ( ( NULL != pDeviceItem ) &&   
                    ( CLIENT_ITEM_INTERFACE_TYPE_DRIVER_LEVEL1 == pDeviceItem->m_driverLevel ) &&
                    pDeviceItem->m_bEnable ) {
                buildPage += _("<b>Name:</b> ");
                buildPage += pDeviceItem->m_strName;
                buildPage += _("<br>");
                buildPage += _("<b>Config:</b> ");
                buildPage += pDeviceItem->m_strParameter;
                buildPage += _("<br>");
                buildPage += _("<b>Path:</b> ");
                buildPage += pDeviceItem->m_strPath;
                buildPage += _("<br>");
            }
        }
    }
    else {
        buildPage += _("disabled<br>");
    }
    buildPage += _("<br>");
    buildPage += _("</div>");


	// Serve data
	mg_send_data( conn, buildPage.ToAscii(), buildPage.Length() );

	return MG_TRUE;	
}



///////////////////////////////////////////////////////////////////////////////
// websrv_bootload
//

int
VSCPWebServerThread::websrv_bootload( struct mg_connection *conn )
{
	//char buf[80];
    wxString str;
    VSCPInformation vscpinfo;
    CVSCPVariable *pVariable = NULL;

	// Check pointer
	if (NULL == conn) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

	wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Device discovery"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
	 buildPage += _("<meta http-equiv=\"refresh\" content=\"5;url=/vscp");
    buildPage += _("\">");
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    // navigation menu 
    buildPage += _(WEB_COMMON_MENU);
	buildPage += _("<b>Bootload functionality is not yet implemented!</b>");
    
	// Server data
	mg_send_data( conn, buildPage.ToAscii(), buildPage.Length() );

	return MG_TRUE;	
}


///////////////////////////////////////////////////////////////////////////////
// websrv_table
//

int
VSCPWebServerThread::websrv_table( struct mg_connection *conn )
{
    wxString str;
    VSCPInformation vscpinfo;
    CVSCPVariable *pVariable = NULL;

	// Check pointer
	if (NULL == conn) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

	wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Table view"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);

    // navigation menu 
    buildPage += _(WEB_COMMON_MENU);

    buildPage += _(WEB_TABLELIST_BODY_START);
    buildPage += _(WEB_TABLELIST_TR_HEAD);

    wxString tblName;
    CVSCPTable *ptblItem = NULL;
    pObject->m_mutexTableList.Lock();
    listVSCPTables::iterator iter;
    for (iter = pObject->m_listTables.begin(); iter != pObject->m_listTables.end(); ++iter) {
        ptblItem = *iter; 

        buildPage += wxString::Format(_(WEB_COMMON_TR_CLICKABLE_ROW),
                                            ( wxString( _("/vscp/tablelist?tblname=") + 
											wxString::FromUTF8( ptblItem->m_vscpFileHead.nameTable ) ) ).c_str()  );
        buildPage += _("<td><b>");
        buildPage += wxString::FromUTF8( ptblItem->m_vscpFileHead.nameTable );
        buildPage += _("</b><br>");
        buildPage += _("<div id=\"small\">");
        buildPage += _("<b>Filename:</b> ");
        buildPage += ptblItem->getFileName();
        buildPage += _(" <b>Filesize:</b> ");
        wxFileName ff( ptblItem->getFileName() );
        buildPage += ff.GetHumanReadableSize();
        buildPage += _("<br><b>First date:</b> ");
        wxDateTime dtStart = wxDateTime( (time_t)ptblItem->getTimeStampStart() );
        buildPage += dtStart.FormatISODate() + _(" ") + dtStart.FormatISOTime();
        buildPage += _(" <b>Last date:</b> ");
        wxDateTime dtEnd = wxDateTime( (time_t)ptblItem->getTimeStampEnd() );
        buildPage += dtEnd.FormatISODate() + _(" ") + dtEnd.FormatISOTime();
        buildPage += _(" <b>Number of records: </b> ");
        buildPage += wxString::Format(_("%d"), ptblItem->getNumberOfRecords() );
        buildPage += _("<br><b>X-label:</b> ");
        buildPage += wxString::FromUTF8( ptblItem->m_vscpFileHead.nameXLabel );
        buildPage += _(" <b>Y-label :</b> ");
        buildPage += wxString::FromUTF8( ptblItem->m_vscpFileHead.nameYLabel );
        buildPage += _(" <b>VSCP Class:</b> ");
        buildPage += wxString::Format(_("%d"), ptblItem->m_vscpFileHead.vscp_class );
        buildPage += _(" <b>VSCP Type:</b> ");
        buildPage += wxString::Format(_("%d"), ptblItem->m_vscpFileHead.vscp_type );
        buildPage += _(" <b>Unit :</b> ");
        buildPage += wxString::Format(_("%d"), ptblItem->m_vscpFileHead.vscp_unit );
        struct _vscptableInfo info;
		ptblItem->m_mutexThisTable.Lock();
        ptblItem->getInfo( &info );
		ptblItem->m_mutexThisTable.Unlock();
        buildPage += _("<br><b>Min-value:</b> ");
        buildPage += wxString::Format(_("%g"), info.minValue );
        buildPage += _(" <b>Max-value:</b> ");
        buildPage += wxString::Format(_("%g"), info.maxValue );
        buildPage += _(" <b>Mean-value:</b> ");
        buildPage += wxString::Format(_("%g"), info.meanValue );
        buildPage += _("<br><b>Description:</b> ");
        buildPage += wxString::FromUTF8( ptblItem->m_vscpFileHead.descriptionTable );
        buildPage += _("</div>");
        buildPage += _("</td>");

        // Type
        buildPage += _("<td>");
        buildPage += _("<div id=\"small\">");
        if ( VSCP_TABLE_DYNAMIC == ptblItem->m_vscpFileHead.type ) {
            buildPage += _("<b>Dynamic</b>");
        }
        else {
            buildPage += _("<b>Static</b>");
            buildPage += _("<br><b>Static size:</b> ");
            buildPage += wxString::Format(_("%d"), ptblItem->m_vscpFileHead.staticSize );
        }
        buildPage += _("</div>");
        buildPage += _("</td>");

        buildPage += _("</tr>");
        ptblItem = NULL;
    } // for
    pObject->m_mutexTableList.Unlock();
    
    
    buildPage += _(WEB_TABLELIST_TABLE_END);


	// Server data
	mg_send_data( conn, buildPage.ToAscii(), buildPage.Length() );

	return MG_TRUE;	
}



///////////////////////////////////////////////////////////////////////////////
// websrv_tablelist
//

int
VSCPWebServerThread::websrv_tablelist( struct mg_connection *conn )
{
	char buf[512];
    wxString str;
    VSCPInformation vscpinfo;
    CVSCPVariable *pVariable = NULL;

	// Check pointer
	if (NULL == conn) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

    // From
    long nFrom = 0;
	if ( mg_get_var( conn, "from", buf, sizeof( buf ) ) > 0 ) { 
		nFrom = atoi( buf );
	}
      
    // Count
    uint16_t nCount = 50;
	if ( mg_get_var( conn, "count", buf, sizeof( buf ) ) > 0 ) { 
		nCount = atoi( buf );
	}


    wxString tblName;
    if ( mg_get_var( conn, "tblname", buf, sizeof( buf ) ) > 0 ) { 
        tblName = wxString::FromUTF8( buf );
	}

	pObject->m_mutexTableList.Lock();
    bool bFound = false;
    CVSCPTable *ptblItem = NULL;
    listVSCPTables::iterator iter;
    for (iter = pObject->m_listTables.begin(); iter != pObject->m_listTables.end(); ++iter) {
        ptblItem = *iter; 
        if ( 0 == strcmp( ptblItem->m_vscpFileHead.nameTable, (const char *)tblName.mbc_str() ) ) {
            bFound = true;
		    break;
		}
    }
	pObject->m_mutexTableList.Unlock();
 
    // Navigation button
	if ( mg_get_var( conn, "navbtn", buf, sizeof( buf ) ) > 0 ) { 
	
		if (NULL != strstr("previous", buf) ) {    
		    nFrom -= nCount;
			if ( nFrom < 0 )  nFrom = 0;
        }        
		else if (NULL != strstr("next",buf)) {
            nFrom += nCount;
            if ( nFrom > ptblItem->getNumberOfRecords()-1 ) {
                if ( ptblItem->getNumberOfRecords() % nCount ) {
                    nFrom = ptblItem->getNumberOfRecords()/nCount;
                }
                else {
                    nFrom = (ptblItem->getNumberOfRecords()/nCount) - 1;
                }
            }
		}
		else if (NULL != strstr("last",buf)) {
            if ( ptblItem->getNumberOfRecords() % nCount ) {
                nFrom = (ptblItem->getNumberOfRecords()/nCount)*nCount;
            }
            else {
                nFrom = ((ptblItem->getNumberOfRecords()/nCount) - 1)*nCount;
            }
		}
		else if ( NULL != strstr("first",buf) ) {
			nFrom = 0;
        }
	}
	else {  // No vaid navigation value
        nFrom = 0;
    }   

	wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Table List"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);

    // navigation menu 
    buildPage += _(WEB_COMMON_MENU);

    buildPage += wxString::Format( WEB_TABLEVALUELIST_BODY_START, (const char *)tblName.c_str() );

    {
        buildPage += wxString::Format( _(WEB_TABLEVALUE_LIST_NAVIGATION),
                _("/vscp/tablelist"),
                nFrom,
                ((nFrom + nCount) < ptblItem->getNumberOfRecords() ) ? 
                      nFrom + nCount - 1 : ptblItem->getNumberOfRecords() - 1,
                ptblItem->getNumberOfRecords(),
                nCount,
                nFrom,
		(const char *)tblName.mbc_str() );
               
        buildPage += _("<br>");
    }

    if ( bFound ) {

        struct _vscpFileRecord *ptableInfo = new struct _vscpFileRecord[ nCount ];
        if ( NULL != ptableInfo ) {
            
            nCount = ptblItem->getRangeOfData( nFrom, nCount, ptableInfo  );

            buildPage += _(WEB_TABLEVALUELIST_TR_HEAD);
    
            for ( uint16_t i=0; i<nCount; i++ ) {

                buildPage += _("<tr>");

                // record
                buildPage += _("<td>");
                buildPage += wxString::Format(_("%u"), nFrom + i );
                buildPage += _("</td>");
                
                // Date
                buildPage += _("<td>");
                wxDateTime dt( (time_t)ptableInfo[i].timestamp );
                buildPage += dt.FormatISODate() + _(" ") + dt.FormatISOTime();
                buildPage += _("</td>");

                // value
                buildPage += _("<td>");
                buildPage += wxString::Format(_("%g"), ptableInfo[i].measurement );
                buildPage += _("</td>");

                buildPage += _("</tr>");

            }

            buildPage += _(WEB_TABLEVALUELIST_TABLE_END);

            delete[] ptableInfo;
        }
        else {
            buildPage += _("Failed to allocate memory for table.");
        }
    }
    else {
        buildPage += _("Table not found!");
    }


	// Server data
	mg_send_data( conn, buildPage.ToAscii(), buildPage.Length() );

	return MG_TRUE;	
}
