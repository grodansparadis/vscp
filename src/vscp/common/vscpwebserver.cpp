// vscpwebserver.cpp
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

#ifdef __GNUG__
#pragma implementation
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
#include <wx/filename.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include <mongoose.h>

#ifdef WIN32

#include <winsock2.h>
//#include <winsock.h>
#include "canal_win32_ipc.h"

#else   // UNIX

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

#include <mongoose.h>

#include <canal_macro.h>
#include <vscp.h>
#include <vscphelper.h>
#include <vscpeventhelper.h>
#include <tables.h>
#include <configfile.h>
#include <crc.h>
#include <randpassword.h>
#include <version.h>
#include <variablecodes.h>
#include <actioncodes.h>
#include <devicelist.h>
#include <devicethread.h>
#include <dm.h>
#include <mdf.h>
#include <vscpeventhelper.h>
#include "vscpwebserver.h"
#include <controlobject.h>
#include <vscpmd5.h>
#include <vscpweb.h>
#include <webserver.h>
#include <webserver_websocket.h>

using namespace std;

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

// Uncomment to compile stock test pages
#define WEB_EXAMPLES
#define EXAMPLE_URI "/__test"
#define EXIT_URI "/__test_exit"

//
//#define USE_SSL_DH



// ip.v4 and ip.v6 at the same time
// https://github.com/civetweb/civetweb/issues/205
// https://stackoverflow.com/questions/1618240/how-to-support-both-ipv4-and-ipv6-connections




///////////////////////////////////////////////////
//                 GLOBALS
///////////////////////////////////////////////////

extern CControlObject *gpobj;



///////////////////////////////////////////////////
//                WEBSERVER
///////////////////////////////////////////////////

// Options
static struct mg_serve_http_opts g_http_server_opts;

// Webserver
struct mg_mgr gmgr;

// Linked list of all active sessions. (webserv.h)
static struct websrv_Session *gp_websrv_sessions;

// Session structure for REST API
struct websrv_rest_session *gp_websrv_rest_sessions;

// Used for authorization
struct read_auth_file_struct
{
    struct web_connection *conn;
    struct web_authorization_header ah;
    const char *domain;
    char buf[256 + 256 + 40];
};

///////////////////////////////////////////////////
//                  HELPERS
///////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////
// vscp_stristr
//

static char* vscp_stristr( char* str1, const char* str2 )
{
    char* p1 = str1 ;
    const char* p2 = str2 ;
    char* r = *p2 == 0 ? str1 : 0 ;

    while( *p1 != 0 && *p2 != 0 ) {
        if( tolower( *p1 ) == tolower( *p2 ) ) {
            if( r == 0 ) {
                r = p1 ;
            }

            p2++ ;
        }
        else {
            p2 = str2 ;
            if( tolower( *p1 ) == tolower( *p2 ) ) {
                r = p1 ;
                p2++ ;
            }
            else {
                r = 0 ;
            }
        }

        p1++ ;
    }

    return *p2 == 0 ? r : 0 ;
}

////////////////////////////////////////////////////////////////////////////////
// vscp_get_mtime
//

static time_t vscp_get_mtime( const char *path )
{
    struct stat statbuf;
    if (stat(path, &statbuf) == -1) {
        perror(path);
        exit(1);
    }
    return statbuf.st_mtime;
}


////////////////////////////////////////////////////////////////////////////////
// vscp_trimWhiteSpace
//

static char *vscp_trimWhiteSpace(char *str)
{
    char *end;

    // Trim leading space
    while(isspace(*str)) str++;

    if( 0 == *str ) {  // All spaces?
        return str;
    }

    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace(*end)) end--;

    // Write new null terminator
    *(end+1) = 0;

    return str;
}

///////////////////////////////////////////////////////////////////////////////
// webserv_util_sendheader
//

void webserv_util_sendheader( struct mg_connection *nc,
                                        const int returncode,
                                        const char *content )
{
    char buf[ 2048 ];
    char date[64];
    time_t curtime = time(NULL);
    vscp_getTimeString( date, sizeof(date), &curtime );
    
    int n = sprintf( buf, "HTTP/1.1 %d OK\r\n"
                                "Content-Type: %s\r\n"
                                "Date: %s\r"
                                "Connection: close\r\n\r\n",
                                returncode,
                                content,
                                date );
    mg_send( nc, buf, n );
}

///////////////////////////////////////////////////////////////////////////////
// webserv_url_decode
//

/*int webserv_url_decode( const char *src, int src_len,
                            char *dst, int dst_len,
                            int is_form_url_encoded) {
  int i, j, a, b;
#define HEXTOI(x) (isdigit(x) ? x - '0' : x - 'W')

    for (i = j = 0; i < src_len && j < dst_len - 1; i++, j++) {
        if (src[i] == '%') {
            if (i < src_len - 2 && isxdigit(*(const unsigned char *) (src + i + 1)) &&
                    isxdigit(*(const unsigned char *) (src + i + 2))) {
                a = tolower(*(const unsigned char *) (src + i + 1));
                b = tolower(*(const unsigned char *) (src + i + 2));
                dst[j] = (char) ((HEXTOI(a) << 4) | HEXTOI(b));
                i += 2;
            }
            else {
                return -1;
            }
        }
        else if (is_form_url_encoded && src[i] == '+') {
            dst[j] = ' ';
        }
        else {
            dst[j] = src[i];
        }
    }

    dst[j] = '\0'; // Null-terminate the destination 

    return i >= src_len ? j : -1;
}
*/

///////////////////////////////////////////////////////////////////////////////
// vscp_check_nonce
//
// Check for authentication timeout.
// Clients send time stamp encoded in nonce. Make sure it is not too old,
// to prevent replay attacks.
// Assumption: nonce is a hexadecimal number of seconds since 1970.
//

/*static int vscp_check_nonce( const char *nonce )
{
    unsigned long now = (unsigned long) time( NULL );
    unsigned long val = (unsigned long) strtoul( nonce, NULL, 16 );
    return (  || ( now < val ) || ( ( now - val ) < 3600 ) );
}*/


///////////////////////////////////////////////////////////////////////////////
// vscp_mkmd5resp
//

/*static void vscp_mkmd5resp( const char *method, 
                                size_t method_len, 
                                const char *uri,
                                size_t uri_len, 
                                const char *ha1, 
                                size_t ha1_len,
                                const char *nonce, 
                                size_t nonce_len, 
                                const char *nc,
                                size_t nc_len, 
                                const char *cnonce, 
                                size_t cnonce_len,
                                const char *qop, 
                                size_t qop_len, 
                                char *resp )
{
    char ha2[33];
    unsigned char hash[16]; 
    cs_md5_ctx ctx;

    cs_md5_init( &ctx );
    cs_md5_update( &ctx, (const unsigned char *)method, method_len );
    cs_md5_update( &ctx, (const unsigned char *)":", 1 );
    cs_md5_update( &ctx, (const unsigned char *)uri, uri_len );
    cs_md5_final( hash, &ctx );
    cs_to_hex( ha2, hash, sizeof( hash ) );
    
    cs_md5_init( &ctx );
    cs_md5_update( &ctx, (const unsigned char *)ha1, ha1_len ); 
    cs_md5_update( &ctx, (const unsigned char *)":", 1 );
    cs_md5_update( &ctx, (const unsigned char *)nonce, nonce_len );
    cs_md5_update( &ctx, (const unsigned char *)":", 1 );
    cs_md5_update( &ctx, (const unsigned char *)nc, nc_len ); 
    cs_md5_update( &ctx, (const unsigned char *)":", 1 ); 
    cs_md5_update( &ctx, (const unsigned char *)cnonce, cnonce_len );
    cs_md5_update( &ctx, (const unsigned char *)":", 1 ); 
    cs_md5_update( &ctx, (const unsigned char *)qop, qop_len );
    cs_md5_update( &ctx, (const unsigned char *)":", 1 ); 
    cs_md5_update( &ctx, (const unsigned char *)ha2, 32 );
    cs_md5_final( hash, &ctx );
    cs_to_hex( ha2, hash, sizeof( hash ) );
}
*/







///////////////////////////////////////////////////////////////////////////////
// readMimeTypes
//
// Read the Mime Types XML file
//

bool VSCPWebServerThread::readMimeTypes(wxString& path)
{
    //unsigned long val;
    wxXmlDocument doc;
    if ( !doc.Load( path )) {
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
//                              WEB SERVER
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
//                     WEBSERVER SESSION HANDLINO
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// websrv_get_session
//

struct websrv_Session *
VSCPWebServerThread::websrv_get_session( struct mg_connection *nc,
                                            struct http_message *hm )
{
    char buf[512];
    struct websrv_Session *ret = NULL;

    // Get the session cookie
    struct mg_str *pheader = mg_get_http_header( hm, "cookie" );
    if ( NULL == pheader ) return NULL;
    if ( 0 == pheader->len ) return NULL;

    // Get session
    if ( !mg_http_parse_header( pheader,
                                    "session",
                                    buf,
                                    sizeof( buf ) ) ) {
        return NULL;
    }

    // find existing session
    ret = gp_websrv_sessions;
    while (NULL != ret) {

        if (0 == strcmp( buf, ret->m_sid ) )
            break;
            ret = ret->m_next;

    }

    if (NULL != ret) {
        ret->m_referenceCount++;
        ret->lastActiveTime = time( NULL );
        return ret;
    }

    return ret;
}


///////////////////////////////////////////////////////////////////////////////
// websrv_add_session_cookie
//

websrv_Session *
VSCPWebServerThread::websrv_add_session_cookie( struct mg_connection *nc,
                                                    struct http_message *hm )
{
    char buf[512];
    char user[50];
    struct websrv_Session *ret;

    // Check pointer
    if (NULL == nc) return NULL;

    struct mg_str *hdr;
    memset( user, 0, sizeof( user ) );
    // Parse "Authorization:" header, fail fast on parse error
    if ( ( hdr = mg_get_http_header(hm, "Authorization")) == NULL ||
        mg_http_parse_header(hdr, "username", user, sizeof(user)) == 0 ) {
        return NULL;
    }

    // Create fresh session
    ret = (struct websrv_Session *)calloc( 1, sizeof( struct websrv_Session ) );
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
                "__VSCP__DAEMON_%X%X%X%X_be_hungry_stay_foolish_%X%X%s",
                (unsigned int)rand(),
                (unsigned int)rand(),
                (unsigned int)rand(),
                (unsigned int)t,
                (unsigned int)rand(),
                1337,
                user );

    cs_md5_ctx ctx;
    cs_md5_init( &ctx );
    cs_md5_update( &ctx, (const unsigned char *)buf, strlen( buf ) );
    unsigned char bindigest[16];
    cs_md5_final( bindigest, &ctx );
    char digest[33];
    memset( digest, 0, sizeof( digest ) );
    cs_to_hex( ret->m_sid, bindigest, 16 );

    char uri[2048];
    memset( uri, 0, sizeof(uri) );
    strncpy( uri, hm->uri.p, hm->uri.len );
    mg_printf( nc,
                "HTTP/1.1 301 Found\r\n"
                "Set-Cookie: session=%s; max-age=3600; http-only\r\n"
                "Set-Cookie: user=%s\r\n"
                "Set-Cookie: original_url=%s; max-age=3600; allow=yes\r\n"
                "Location: %s\r\n"
                "Content-Length: 0r\n\r\n",
                ret->m_sid,
                user,
                uri,
                uri );

    ret->m_pUserItem = gpobj->m_userList.getUser( wxString::FromAscii( user ) );

    // Add to linked list
    ret->m_referenceCount++;
    ret->lastActiveTime = time( NULL );
    ret->m_next = gp_websrv_sessions;
    gp_websrv_sessions = ret;

    return ret;
}

///////////////////////////////////////////////////////////////////////////////
// websrv_GetCreateSession
//

struct websrv_Session *
VSCPWebServerThread::websrv_GetCreateSession( struct mg_connection *nc,
                                                struct http_message *hm )
{
    struct mg_str *pheader;
    char user[256];
    struct websrv_Session *rv = NULL;

    // Check pointer
    if (NULL == nc) return NULL;

    /*if ( NULL == ( rv =  gpobj->getWebServer()->websrv_get_session( nc, hm ) ) ) {

        if ( NULL == ( pheader = mg_get_http_header( hm, "Authorization") ) ||
                        ( vscp_strncasecmp( pheader->p, "Digest ", 7 ) != 0 ) ) {
            return NULL;
        }

        if (!mg_http_parse_header(pheader, "username", user, sizeof(user))) {
            return NULL;
        }

        // Add session cookie
        rv = gpobj->getWebServer()->websrv_add_session_cookie( nc, hm );
    }*/

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// websrv_expire_sessions
//

void
VSCPWebServerThread::websrv_expire_sessions( struct mg_connection *nc,
                                                struct http_message *hm )
{
    struct websrv_Session *pos;
    struct websrv_Session *prev;
    struct websrv_Session *next;
    time_t now;

    now = time( NULL );
    prev = NULL;
    pos = gp_websrv_sessions;

    while (NULL != pos) {

        next = pos->m_next;

        if (now - pos->lastActiveTime > 60 * 60) {

            // expire sessions after 1h
            if ( NULL == prev ) {
                gp_websrv_sessions = pos->m_next;
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

/*int
VSCPWebServerThread::websrv_check_password( const char *method,
                                                const char *ha1,
                                                const char *uri,
                                                const char *nonce,
                                                const char *nc,
                                                const char *cnonce,
                                                const char *qop,
                                                const char *response )
{
    char ha2[33], expected_response[33];

#if 0
    // Check for authentication timeout
    if ( ( (unsigned long)time(NULL) - (unsigned long)to64( nonce ) ) > 3600 * 2) {
        return 0;
    }
#endif

    cs_md5_ctx ctx;
    unsigned char hash[16];

    cs_md5_init( &ctx );
    cs_md5_update( &ctx, (const unsigned char *)method, strlen( method ) );
    cs_md5_update( &ctx, (const unsigned char *)":", 1 );
    cs_md5_update( &ctx, (const unsigned char *)uri, strlen( uri ) );
    cs_md5_final( hash, &ctx );
    cs_to_hex( ha2, hash, sizeof( hash ) );
    
    cs_md5_init( &ctx );
    cs_md5_update( &ctx, (const unsigned char *)ha1, 32 );
    cs_md5_update( &ctx, (const unsigned char *)":", 1 );
    cs_md5_update( &ctx, (const unsigned char *)nonce, strlen( nonce ) );
    cs_md5_update( &ctx, (const unsigned char *)":", 1 );
    cs_md5_update( &ctx, (const unsigned char *)nc, strlen( nc ) );
    cs_md5_update( &ctx, (const unsigned char *)":", 1 );
    cs_md5_update( &ctx, (const unsigned char *)cnonce, strlen( cnonce ) );
    cs_md5_update( &ctx, (const unsigned char *)":", 1 );
    cs_md5_update( &ctx, (const unsigned char *)qop, strlen( qop ) );
    cs_md5_update( &ctx, (const unsigned char *)":", 1 );
    cs_md5_update( &ctx, (const unsigned char *)ha2, 32 );
    cs_md5_final( hash, &ctx );
    cs_to_hex( expected_response, hash, sizeof( hash ) );

    return ( vscp_strcasecmp( response, expected_response ) == 0 ) ? TRUE : FALSE;

}*/


////////////////////////////////////////////////////////////////////////////////
// vscp_mainPage
//

static int
vscp_mainpage( struct web_connection *conn, void *cbdata )
{
	// Check pointer
    if ( NULL == conn ) return 0;
    
    web_printf( conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n"
                  "Content-Type: text/html; charset=utf-8\r\n"
                  "Connection: close\r\n\r\n");

    web_printf( conn, WEB_COMMON_HEAD, "VSCP - Control" );
    web_printf( conn, WEB_STYLE_START );
    web_printf( conn, WEB_COMMON_CSS );     // CSS style Code
    web_printf( conn, WEB_STYLE_END );
    web_printf( conn, WEB_COMMON_JS );      // Common Javascript code

    web_printf( conn, WEB_COMMON_HEAD_END_BODY_START );
    // Insert server url into navigation menu
    web_printf( conn, WEB_COMMON_MENU );

    web_printf( conn, "<span align=\"center\">" );
    web_printf( conn, "<h4> Welcome to the VSCP daemon control interface.</h4>" );
    web_printf( conn, "</span>");
    web_printf( conn, "<span style=\"text-indent:50px;\"><p>" );
    web_printf( conn, "<img src=\"http://vscp.org/images/vscp_logo.png\" width=\"100\">" );
    web_printf( conn, "</p></span>" );
    web_printf( conn, "<span style=\"text-indent:50px;\"><p>" );
    web_printf( conn, " <b>Version:</b> " );
    web_printf( conn, VSCPD_DISPLAY_VERSION );
    web_printf( conn, "</p><p>" );
    web_printf( conn, VSCPD_COPYRIGHT_HTML );
    web_printf( conn, "</p></span>" );

    web_printf( conn, WEB_COMMON_END );     // Common end code       
    return 1;
}


///////////////////////////////////////////////////////////////////////////////
// check_admin_authorization
//
// Only the admin user has access to /vscp/....
//

static int 
check_admin_authorization( struct web_connection *conn, void *cbdata ) 
{
    char buf[8192];
    struct web_authorization_header ah;
    CUserItem *pUserItem;
    bool bValidHost;
    struct web_context * ctx;
    const struct web_request_info *reqinfo;
    
    // If security is disabled just allow access
    if ( gpobj->m_bDisableSecurityWebServer ) {
        return 1;
    }
    
    // Check pointers
    if ( !conn || 
         !( ctx = web_get_context( conn ) ) ||
         !( reqinfo = web_get_request_info( conn ) )  ) {
        return 0;
    }
    
    memset( &ah, 0, sizeof( ah ) );
    
    if ( !web_parse_auth_header( conn, buf, sizeof( buf ), &ah ) ) {
        web_send_digest_access_authentication_request( conn, NULL );
        return 0;
    }

    // Check if user is valid
    pUserItem = gpobj->m_userList.getUser( wxString::FromUTF8( ah.user ) );
    if ( NULL == pUserItem ) return 0;

    // Check if remote ip is valid
    gpobj->m_mutexUserList.Lock();
    bValidHost = pUserItem->isAllowedToConnect( reqinfo->remote_addr );
    gpobj->m_mutexUserList.Unlock();
    if ( !bValidHost ) {
        // Host is not allowed to connect
        wxString strErr =
            wxString::Format( _("[Webserver Client] Host [%s] NOT allowed to connect. User [%s]\n"),
                                    reqinfo->remote_addr,
                                    (const char *)pUserItem->getUser().mbc_str() );
            gpobj->logMsg( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_SECURITY );
            return 0;
    }

    uint8_t salt[16];   // Stored salt
    uint8_t hash[32];   // Stored hash
    
    if ( !vscp_getHashPasswordComponents( salt, 
                                            hash, 
                                            (const char *)pUserItem->getPassword().mbc_str() ) ) {
        web_send_digest_access_authentication_request( conn, NULL );
        return 0;
    }
    
    
    char ha1[33];
    vscpmd5_getDigestFromMultiStrings( ha1,
            (const char *)pUserItem->getUser().mbc_str(), ":",
            gpobj->m_authDomain, ":",
            (const char *)gpobj->m_vscptoken.mbc_str(),
            NULL );
    
    if ( !web_check_password( reqinfo->request_method,
                                ha1,
                                ah.uri,
                                ah.nonce,
                                ah.nc,
                                ah.cnonce,
                                ah.qop,
                                ah.response ) ) {
        // Username/password wrong
        wxString strErr =
                wxString::Format(_("[Webserver Client] Host [%s] User [%s] NOT allowed to connect.\n"),
                reqinfo->remote_addr,
                pUserItem->getUser().mbc_str());
        gpobj->logMsg(strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_SECURITY);
        web_send_digest_access_authentication_request( conn, NULL );
        return 0;
    }

    return 1;
}



////////////////////////////////////////////////////////////////////////////////
// vscp_settings
//

static int
vscp_settings( struct web_connection *conn, void *cbdata )
{
	web_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");
	web_printf(conn, "<html><body>");
	web_printf(conn, "<h2>Settings</h2>");
        
	web_printf( conn,
                            "<p><b>Admin user</b> = %s</p>", 
                            (const char *)gpobj->m_admin_user.mbc_str() );	
	web_printf( conn, "</body></html>\n");
        
        web_printf( conn, WEB_COMMON_HEAD, "VSCP - Control" );
        web_printf( conn, WEB_STYLE_START );
        web_printf( conn, WEB_COMMON_CSS );     // CSS style Code
        web_printf( conn, WEB_STYLE_END );
        web_printf( conn, WEB_COMMON_JS );      // Common Javascript code

        web_printf( conn, WEB_COMMON_HEAD_END_BODY_START );
        // Insert server url into navigation menu
        web_printf( conn, WEB_COMMON_MENU );

        web_printf( conn, WEB_IFLIST_BODY_START );
        web_printf( conn, WEB_IFLIST_TR_HEAD );
        
	return 1;
}


///////////////////////////////////////////////////////////////////////////////
// listFile
//

static int vscp_listFile( struct web_connection *conn, 
                        void *cbdata,
                        wxFileName& logfile,
                        wxString& textHeader )
{
    web_printf( conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");

    /*bool bFirstRow = false;
    wxString strHeader = wxString::Format(_("VSCP - %s"), textHeader.mbc_str() );
    web_printf( conn,WEB_COMMON_HEAD, strHeader.mbc_str() );
    web_printf( conn, WEB_STYLE_START);
    web_printf( conn, WEB_COMMON_CSS);     // CSS style Code
    web_printf( conn, WEB_STYLE_END);
    web_printf( conn, WEB_COMMON_JS);      // Common Javascript code
    web_printf( conn, WEB_COMMON_HEAD_END_BODY_START);

    // Navigation menu
    web_printf( conn, WEB_COMMON_MENU);

    web_printf( conn, "<b>%s</b><br><br>", textHeader.mbc_str() );
    web_printf( conn, "<b>Path</b>=<i>");
    web_printf( conn, logfile.GetFullPath() );
    web_printf( conn, "</i><br>");
    web_printf( conn, "-----------------------------------------------------------------------------------------<br>");

    wxULongLong fileLength = logfile.GetSize();
    double pos;
    if ( fileLength.ToDouble() > 100000 ) {
        pos = fileLength.ToDouble() - 100000;
        web_printf( conn, "<br><span style=\"color: red;\">File has been truncated due to size. Filesize is %dMB last %dKB shown.</span><br><br>"),
            (int)fileLength.ToDouble()/(1024*1024), 10 );
        bFirstRow = true;
    }

    if ( 0 == fileLength.ToDouble() ) {
        web_printf( conn, "<span style=\"color: red;\">File is empty.</span><br><br>");
    }

    wxFile file;
    if ( file.Open( logfile.GetFullPath() ) ) {

        wxFileInputStream input( file);
        wxTextInputStream text( input );

        // Got to beginning of file
        file.Seek( pos );

        while ( !file.Eof() ) {
            if ( !bFirstRow ) {
                web_printf( conn, "&nbsp;&nbsp;&nbsp;") + text.ReadLine()  + _("<br>");
            }
            else {
                bFirstRow = false;
                text.ReadLine();    // First row looks ugly due to positioning
            }
        }

        file.Close();

    }
    else {
        web_printf( conn, "Error: Unable to open file<br><br>");
    }

    // Serve data
    web_printf( conn, "&nbsp;&nbsp;&nbsp;<strong>The End</strong>");
    web_printf( conn, WEB_COMMON_END);
    */
    
    return 1;
}


///////////////////////////////////////////////////////////////////////////////
// vscp_interface
//

static int vscp_interface( struct web_connection *conn, void *cbdata )
{
    // Check pointer
    if  (NULL == conn ) return 0;

    web_printf( conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n"
                  "Content-Type: text/html; charset=utf-8\r\n"
                  "Connection: close\r\n\r\n");
    
    web_printf( conn, WEB_COMMON_HEAD, "VSCP - Control" );
    web_printf( conn, WEB_STYLE_START );
    web_printf( conn, WEB_COMMON_CSS );     // CSS style Code
    web_printf( conn, WEB_STYLE_END );
    web_printf( conn, WEB_COMMON_JS );      // Common Javascript code

    web_printf( conn, WEB_COMMON_HEAD_END_BODY_START );
    // Insert server url into navigation menu
    web_printf( conn, WEB_COMMON_MENU );

    web_printf( conn, WEB_IFLIST_BODY_START );
    web_printf( conn, WEB_IFLIST_TR_HEAD );

    wxString strGUID;
    wxString strBuf;

    // Display Interface List
    gpobj->m_wxClientMutex.Lock();
    VSCPCLIENTLIST::iterator iter;
    for (iter = gpobj->m_clientList.m_clientItemList.begin();
            iter != gpobj->m_clientList.m_clientItemList.end();
            ++iter) {

        CClientItem *pItem = *iter;
        pItem->m_guid.toString(strGUID);

        web_printf( conn, WEB_IFLIST_TR);

        // Client id
        web_printf( conn, WEB_IFLIST_TD_CENTERED);
        web_printf( conn, "%d", pItem->m_clientID );
        web_printf( conn, "</td>");

        // Interface type
        web_printf( conn, WEB_IFLIST_TD_CENTERED );
        web_printf( conn, "%d", pItem->m_type );
        web_printf( conn, "</td>");

        // GUID
        web_printf( conn, WEB_IFLIST_TD_GUID);
        web_printf( conn, strGUID.Left(23).mbc_str() );
        web_printf( conn, "<br>");
        web_printf( conn, strGUID.Right(23).mbc_str() );
        web_printf( conn, "</td>");

        // Interface name
        web_printf( conn, "<td>");
        web_printf( conn, pItem->m_strDeviceName.Left(pItem->m_strDeviceName.Length()-30).mbc_str() );
        web_printf( conn, "</td>");

        // Start date
        web_printf( conn, "<td>");
        web_printf( conn, pItem->m_strDeviceName.Right(19).mbc_str() );
        web_printf( conn, "</td>");

        web_printf( conn, "</tr>");

    }

    gpobj->m_wxClientMutex.Unlock();

    web_printf( conn, WEB_IFLIST_TABLE_END);

    web_printf( conn, 
        "<br>All interfaces to the daemon is listed here. "
            "This is drivers as well as clients on one of the daemons "
            "interfaces. It is possible to see events coming in on a on a "
            "specific interface and send events on just one of the interfaces. "
            "This is mostly used on the driver interfaces but is possible on "
            "all interfacs<br>");

    web_printf( conn, "<br><b>Interface Types</b><br>" );
    web_printf( conn, 
                    "%d - Unknown (you should not see this).<br>", 
                    (uint8_t)CLIENT_ITEM_INTERFACE_TYPE_NONE );
    web_printf( conn,  
                    "%d - Internal daemon client.<br>", 
                    (uint8_t)CLIENT_ITEM_INTERFACE_TYPE_CLIENT_INTERNAL );
    web_printf( conn,  
                    "%d - Level I (CANAL) Driver.<br>", 
                    (uint8_t)CLIENT_ITEM_INTERFACE_TYPE_DRIVER_LEVEL1 );
    web_printf( conn,  
                    "%d - Level II Driver.<br>", 
                    (uint8_t)CLIENT_ITEM_INTERFACE_TYPE_DRIVER_LEVEL2 );
    web_printf( conn,  
                    "%d - TCP/IP Client.<br>", 
                    (uint8_t)CLIENT_ITEM_INTERFACE_TYPE_CLIENT_TCPIP );
    web_printf( conn,  
                    "%d - UDP Client.<br>", 
                    (uint8_t)CLIENT_ITEM_INTERFACE_TYPE_CLIENT_UDP );
    web_printf( conn,  
                    "%d - Web Server Client.<br>", 
                    (uint8_t)CLIENT_ITEM_INTERFACE_TYPE_CLIENT_WEB );
    web_printf( conn,  
                    "%d - WebSocket Client.<br>", 
                    (uint8_t)CLIENT_ITEM_INTERFACE_TYPE_CLIENT_WEBSOCKET );

    web_printf( conn, WEB_COMMON_END );     // Common end code

    return 1;
}


//-----------------------------------------------------------------------------
//                                  DM
//-----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// vscp_dm_list
//

static int vscp_dm_list( struct web_connection *conn, void *cbdata )
{
    char buf[256];
    VSCPInformation vscpinfo;

    // Check pointer
    if ( NULL == conn ) return 0;
    
    const struct web_request_info *reqinfo =  
                web_get_request_info( conn );
    if ( NULL == reqinfo ) return 0;

    // light
    bool bLight = false;
    *buf = 0;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                strlen( reqinfo->query_string ), 
                "light", 
                buf, 
                sizeof( buf ) ) > 0 ) {
            if ( strlen( buf ) && 
                    ( NULL != strstr( "true", buf ) ) ) {
                bLight = true;
            }
        } 
    }

    // From
    long nFrom = 0;
    *buf = 0;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "from", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            nFrom = atoi( buf );
        }
    }

    // Count
    *buf = 0;
    uint16_t nCount = 50;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "count", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            nCount = atoi( buf );
        }
    }

    // Navigation button
    *buf = 0;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "navbtn", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {

            if ( NULL != strstr("previous", buf ) ) {
                nFrom -= nCount;
                if ( nFrom < 0 )  nFrom = 0;
            }
            else if ( NULL != strstr( "next",buf ) ) {
                nFrom += nCount;
                if ( nFrom > gpobj->m_dm.getMemoryElementCount() - 1 ) {
                    if ( gpobj->m_dm.getMemoryElementCount() % nCount ) {
                        nFrom = gpobj->m_dm.getMemoryElementCount()/nCount;
                    }
                    else {
                        nFrom = (gpobj->m_dm.getMemoryElementCount()/nCount) - 1;
                    }
                }
            }
            else if (NULL != strstr("last",buf)) {
                if ( gpobj->m_dm.getMemoryElementCount() % nCount ) {
                    nFrom = (gpobj->m_dm.getMemoryElementCount()/nCount)*nCount;
                }
                else {
                    nFrom = ((gpobj->m_dm.getMemoryElementCount()/nCount) - 1)*nCount;
                }
            }
            else if ( NULL != strstr("first",buf) ) {
                nFrom = 0;
            }
        }
    }

    web_printf( conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n"
                  "Content-Type: text/html; charset=utf-8\r\n"
                  "Connection: close\r\n\r\n");

    web_printf( conn, 
                    WEB_COMMON_HEAD,
                    "VSCP - Decision Matrix" );
    web_printf( conn, WEB_STYLE_START );
    web_printf( conn, WEB_COMMON_CSS );     // CSS style Code
    web_printf( conn, WEB_STYLE_END );
    web_printf( conn, WEB_COMMON_JS );      // Common Javascript code
    web_printf( conn, WEB_COMMON_HEAD_END_BODY_START );

    web_printf( conn, WEB_COMMON_MENU );
    web_printf( conn, WEB_DMLIST_BODY_START );

    {
        wxString wxstrlight = ( ( bLight ) ? _("true") : _("false") );
        web_printf( conn,  
                    WEB_COMMON_LIST_NAVIGATION,
                    "/vscp/dm",
                    (unsigned long)nFrom + 1,
                    ( (unsigned long)(nFrom + nCount) < gpobj->m_dm.getMemoryElementCount() ) ?
                        nFrom + nCount : gpobj->m_dm.getMemoryElementCount(),
                    (unsigned long)gpobj->m_dm.getMemoryElementCount(),
                    (unsigned long)nCount,
                    (unsigned long)nFrom,
                    (const char *)wxstrlight.mbc_str() );

        web_printf( conn, "<br>");
    }

    wxString strGUID;
    wxString strBuf;

    // Display DM List

    if ( 0 == gpobj->m_dm.getMemoryElementCount() ) {
        web_printf( conn, "<br>Decision Matrix is empty!<br>");
    }
    else {
        web_printf( conn, WEB_DMLIST_TR_HEAD);
    }

    if (nFrom < 0) nFrom = 0;

    for ( int i=0; i<nCount; i++) {

        // Check limits
        if ( ( nFrom+i ) >= gpobj->m_dm.getMemoryElementCount() ) break;

        dmElement *pElement = gpobj->m_dm.getMemoryElementFromRow( i );

        {
            wxString url_dmedit =
                    wxString::Format(_("/vscp/dmedit?id=%ld&from=%ld&count=%ld"),
                                        (long)(nFrom+i), (long)nFrom, (long)nCount );
            web_printf( conn, 
                            WEB_COMMON_TR_CLICKABLE_ROW,
                            (const char *)url_dmedit.mbc_str() );
        }

        // Client id
        web_printf( conn, WEB_IFLIST_TD_CENTERED);
        web_printf( conn,  
                        "<form name=\"input\" action=\"/vscp/dmdelete?id=%ld\" "
                        "method=\"get\"> %ld <input type=\"submit\" "
                        "value=\"x\"><input type=\"hidden\" "
                        "name=\"id\"value=\"%ld\"></form>",
                        (long)nFrom + i, 
                        (long)nFrom + i + 1, 
                        (long)nFrom + i );
        web_printf( conn, "</td>");

        // DM entry
        web_printf( conn, "<td>");

        if (NULL != pElement) {

            web_printf( conn, "<div id=\"small\">");

            // Group
            web_printf( conn, "<b>Group:</b> ");
            web_printf( conn, pElement->m_strGroupID.mbc_str() );
            web_printf( conn, "<br>");

            web_printf( conn, "<b>Comment:</b> ");
            web_printf( conn, pElement->m_comment.mbc_str() );
            web_printf( conn, "<br><hr width=\"90%\">");

            web_printf( conn, "<b>Control:</b> ");

            // Control - Enabled
            if (pElement->isEnabled()) {
                web_printf( conn, "[Row is enabled] ");
            }
            else {
                web_printf( conn, "[Row is disabled] ");
            }

            // Control - Check index
            if (pElement->isCheckIndexSet()) {
                if (pElement->m_bCheckMeasurementIndex) {
                    web_printf( conn, "[Check Measurement Index] ");
                }
                else {
                    web_printf( conn, "[Check Index] ");
                }
            }

            // Control - Check zone
            if (pElement->isCheckZoneSet()) {
                web_printf( conn, "[Check Zone] ");
            }

            // Control - Check subzone
            if (pElement->isCheckSubZoneSet()) {
                web_printf( conn, "[Check Subzone] ");
            }

            web_printf( conn, "<br>");

            if (!bLight) {

                // * Filter

                web_printf( conn, "<b>Filter_priority: </b>");
                web_printf( conn, 
                                "%d ",
                                pElement->m_vscpfilter.filter_priority );

                web_printf( conn, "<b>Filter_class: </b>");
                web_printf( conn,  
                                "%d ",
                                pElement->m_vscpfilter.filter_class );
                web_printf( conn, " [");
                web_printf( conn,  
                        vscpinfo.getClassDescription( pElement->m_vscpfilter.filter_class ).mbc_str() );
                web_printf( conn, "] ");

                web_printf( conn, " <b>Filter_type: </b>");
                web_printf( conn,  
                                    "%d ", 
                                    pElement->m_vscpfilter.filter_type );
                web_printf( conn, " [");
                web_printf( conn,  vscpinfo.getTypeDescription (pElement->m_vscpfilter.filter_class,
                                                            pElement->m_vscpfilter.filter_type ).mbc_str() );
                web_printf( conn, "]<br>");

                web_printf( conn, " <b>Filter_GUID: </b>" );
                vscp_writeGuidArrayToString(pElement->m_vscpfilter.filter_GUID, strGUID );
                web_printf( conn,  strGUID.mbc_str() );

                web_printf( conn, "<br>" );

                web_printf( conn, "<b>Mask_priority: </b>");
                web_printf( conn, 
                                "%d ", 
                                pElement->m_vscpfilter.mask_priority );

                web_printf( conn, "<b>Mask_class: </b>");
                web_printf( conn, 
                                "%d ", 
                                pElement->m_vscpfilter.mask_class );

                web_printf( conn, "<b>Mask_type: </b>");
                web_printf( conn, 
                                "%d ", 
                                pElement->m_vscpfilter.mask_type );

                web_printf( conn, "<b>Mask_GUID: </b>");
                vscp_writeGuidArrayToString(pElement->m_vscpfilter.mask_GUID, strGUID);
                web_printf( conn,  strGUID.mbc_str() );

                web_printf( conn, "<br>");

                web_printf( conn, "<b>Index: </b>");
                web_printf( conn, 
                                "%d ", 
                                pElement->m_index );

                web_printf( conn, "<b>Zone: </b>");
                web_printf( conn, 
                                "%d ", 
                                pElement->m_zone );

                web_printf( conn, "<b>Subzone: </b>");
                web_printf( conn, 
                                "%d ", 
                                pElement->m_subzone );

                if ( pElement->m_bCheckMeasurementIndex ) {
                    web_printf( conn, "&nbsp;&nbsp;&nbsp;(Sensor index is used for index compare)");
                }

                web_printf( conn, "<br>");

                web_printf( conn, "<b>Allowed from:</b> ");
                web_printf( conn, pElement->m_timeAllow.m_fromTime.FormatISODate().mbc_str() );
                web_printf( conn, " ");
                web_printf( conn, pElement->m_timeAllow.m_fromTime.FormatISOTime().mbc_str() );

                web_printf( conn, " <b>Allowed to:</b> ");
                web_printf( conn, pElement->m_timeAllow.m_endTime.FormatISODate().mbc_str() );
                web_printf( conn, " ");
                web_printf( conn, pElement->m_timeAllow.m_endTime.FormatISOTime().mbc_str() );

                web_printf( conn, " <b>Weekdays:</b> ");
                web_printf( conn, pElement->m_timeAllow.getWeekDays().mbc_str() );
                web_printf( conn, "<br>");

                web_printf( conn, "<b>Allowed time:</b> ");
                web_printf( conn, pElement->m_timeAllow.getActionTimeAsString().mbc_str() );
                web_printf( conn, "<br>");

            } // mini

            web_printf( conn, "<b>Action:</b> ");
            web_printf( conn, 
                            "%d ", 
                            pElement->m_actionCode );

            web_printf( conn, " <b>Action parameters:</b> ");
            web_printf( conn, pElement->m_actionparam.mbc_str() );
            web_printf( conn, "<br>");

            if (!bLight) {

                web_printf( conn, "<b>Trigger Count:</b> ");
                web_printf( conn, 
                                "%d ", 
                                pElement->m_triggCounter );

                web_printf( conn, "<b>Error Count:</b> ");
                web_printf( conn, 
                                "%d ", 
                                pElement->m_errorCounter );
                web_printf( conn, "<br>");

                web_printf( conn, "<b>Last Error String:</b> ");
                web_printf( conn, pElement->m_strLastError.mbc_str());

            } // mini

            web_printf( conn, "</div>");

        }
        else {
            web_printf( conn, "Internal error: Non existent DM entry.");
        }

        web_printf( conn, "</td>");
        web_printf( conn, "</tr>");

    }

    web_printf( conn, WEB_DMLIST_TABLE_END);

    {
        wxString wxstrlight = ((bLight) ? _("true") : _("false"));
        web_printf( conn, 
                        WEB_COMMON_LIST_NAVIGATION,
                        "/vscp/dm",
                        (unsigned long)nFrom + 1,
                        ( (unsigned long)(nFrom + nCount) < gpobj->m_dm.getMemoryElementCount() ) ?
                            nFrom + nCount : gpobj->m_dm.getMemoryElementCount(),
                        (unsigned long)gpobj->m_dm.getMemoryElementCount(),
                        (unsigned long)nCount,
                        (unsigned long)nFrom,
                        (const char *)wxstrlight.mbc_str() );
    }

    web_printf( conn, WEB_COMMON_END);     // Common end code

    return 1;
}


///////////////////////////////////////////////////////////////////////////////
// vscp_dm_edit
//

static int vscp_dm_edit( struct web_connection *conn, void *cbdata  )
{
    char buf[256];
    wxString str;
    VSCPInformation vscpinfo;
    dmElement *pElement = NULL;

    // Check pointer
    if ( NULL == conn ) return 0;
    
    const struct web_request_info *reqinfo =  
                web_get_request_info( conn );
    if ( NULL == reqinfo ) return 0;
        
    // id
    long id = -1;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string, 
                            strlen( reqinfo->query_string ), 
                            "id", 
                            buf,
                            sizeof( buf ) ) > 0 ) {
            id = atoi(buf);
        }
    }

    // From
    long nFrom = 0;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "from", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            nFrom = atoi( buf );
        }
    }

    // Count
    uint16_t nCount = 50;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "count", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            nCount = atoi( buf );
        }
    }

    // Flag for new DM row
    bool bNew = false;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "new", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            if ( NULL != strstr( "true", buf ) ) bNew = true;
        }
    }

    web_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");

    web_printf( conn, 
                    WEB_COMMON_HEAD, 
                    "VSCP - Decision Matrix Edit" );
    web_printf( conn, WEB_STYLE_START);
    web_printf( conn, WEB_COMMON_CSS);     // CSS style Code
    web_printf( conn, WEB_STYLE_END);
    web_printf( conn, WEB_COMMON_JS);      // Common Javascript code
    web_printf( conn, WEB_COMMON_HEAD_END_BODY_START);

    web_printf( conn, WEB_COMMON_MENU);;
    web_printf( conn, WEB_DMEDIT_BODY_START);

    if ( !bNew && id < gpobj->m_dm.getMemoryElementCount() ) {
        pElement = gpobj->m_dm.getMemoryElementFromRow( id );
    }

    if ( bNew || ( NULL != pElement ) ) {

        if ( bNew ) {
            web_printf( conn, "<span id=\"optiontext\">New record.</span><br>");
        }
        else {
            web_printf( conn, 
                        "<span id=\"optiontext\">Record = %ld.</span><br>", 
                        id );
        }

        web_printf( conn, "<br><form method=\"get\" action=\"");
        web_printf( conn, "/vscp/dmpost");
        web_printf( conn, "\" name=\"dmedit\">");

        web_printf( conn, 
                        "<input name=\"id\" value=\"%ld\" "
                        "type=\"hidden\"></input>", 
                        id );


        if ( bNew ) {

            web_printf( conn, 
                        "<input name=\"new\" "
                        "value=\"true\" type=\"hidden\"></input>");

            long nFrom;
             if ( gpobj->m_dm.getMemoryElementCount() % nCount ) {
                nFrom = (gpobj->m_dm.getMemoryElementCount()/nCount)*nCount;
            }
            else {
                nFrom = ((gpobj->m_dm.getMemoryElementCount()/nCount) - 1)*nCount;
            }

            web_printf( conn, 
                            "<input name=\"from\" value=\"%ld\" type=\"hidden\">", 
                            (long)nFrom );
            web_printf( conn, 
                            "<input name=\"count\" value=\"%ld\" type=\"hidden\">", 
                            (long)nCount );

        }
        else {
            // Hidden from
            web_printf( conn,
                            "<input name=\"from\" value=\"%ld\" type=\"hidden\">", 
                            (long)nFrom );
            // Hidden count
            web_printf( conn,
                            "<input name=\"count\" value=\"%ld\" type=\"hidden\">", 
                            (long)nCount );
            web_printf( conn, 
                            "<input name=\"new\" value=\"false\" "
                            "type=\"hidden\"></input>");
        }

        web_printf( conn, "<h4>Group id:</h4>");
        web_printf( conn, "<textarea cols=\"20\" rows=\"1\" name=\"groupid\">");
        if ( !bNew ) web_printf( conn, pElement->m_strGroupID.mbc_str() );
        web_printf( conn, "</textarea><br>");


        web_printf( conn, "<h4>Event:</h4> <span id=\"optiontext\">(bluish are masks)</span><br>");

        web_printf( conn, "<table class=\"invisable\"><tbody><tr class=\"invisable\">");

        web_printf( conn, "<td class=\"invisable\">Priority:</td><td class=\"tbalign\">");

        // Priority
        web_printf( conn, "<select name=\"filter_priority\">");
        web_printf( conn, "<option value=\"-1\" ");
        if ( bNew ) web_printf( conn, " selected ");
        web_printf( conn, ">Don't care</option>");

        if ( !bNew ) str = (0 == pElement->m_vscpfilter.filter_priority) ? _("selected") : _(" ");
        web_printf( conn,
                        "<option value=\"0\" %s>0 - Highest</option>",
                        (const char *)str.mbc_str() );

        if ( !bNew ) str = ( 1 == pElement->m_vscpfilter.filter_priority) ? _("selected") : _(" ");
        web_printf( conn,
                        "<option value=\"1\" %s>1 - Very High</option>",
                        (const char *)str.mbc_str() );

        if ( !bNew ) str = (2 == pElement->m_vscpfilter.filter_priority) ? _("selected") : _(" ");
        web_printf( conn,
                        "<option value=\"2\" %s>2 - High</option>",
                        (const char *)str.mbc_str() );

        if ( !bNew ) str = (3 == pElement->m_vscpfilter.filter_priority) ? _("selected") : _(" ");
        web_printf( conn,
                        "<option value=\"3\" %s>3 - Normal</option>",
                        (const char *)str.mbc_str() );

        if ( !bNew ) str = (4 == pElement->m_vscpfilter.filter_priority) ? _("selected") : _(" ");
        web_printf( conn,
                        "<option value=\"4\" %s>4 - Low</option>",
                        (const char *)str.mbc_str() );

        if ( !bNew ) str = (5 == pElement->m_vscpfilter.filter_priority) ? _("selected") : _(" ");
        web_printf( conn,
                        "<option value=\"5\" %s>5 - Lower</option>",
                        (const char *)str.mbc_str() );

        if ( !bNew ) str = (6 == pElement->m_vscpfilter.filter_priority) ? _("selected") : _(" ");
        web_printf( conn,
                        "<option value=\"6\" %s>6 - Very Low</option>",
                        (const char *)str.mbc_str() );

        if ( !bNew ) str = (7 == pElement->m_vscpfilter.filter_priority) ? _("selected") : _(" ");
        web_printf( conn,
                    "<option value=\"7\" %s>7 - Lowest</option>",
                    (const char *)str.mbc_str() );

        web_printf( conn, "</select>");
        // Priority mask
        web_printf( conn, 
                        "</td><td><textarea style=\"background-color: "
                        "#72A4D2;\" cols=\"5\" rows=\"1\" name=\"mask_priority\">");
        if ( bNew ) {
            web_printf( conn, "0x00");
        }
        else {
            web_printf( conn, "%X", pElement->m_vscpfilter.mask_priority );
        }
        
        web_printf( conn, "</textarea>");

        web_printf( conn, "</td></tr>");

        // Class
        web_printf( conn, 
                        "<tr class=\"invisable\"><td class=\"invisable\">"
                        "Class:</td><td class=\"invisable\"><textarea "
                        "cols=\"10\" rows=\"1\" name=\"filter_vscpclass\">" );
        if ( bNew ) {
            web_printf( conn, "");;
        }
        else {
            web_printf( conn, "%d", pElement->m_vscpfilter.filter_class );
        }
        web_printf( conn, "</textarea>");

        web_printf( conn, 
                        "</td><td> <textarea style=\"background-color: "
                        "#72A4D2;\" cols=\"10\" rows=\"1\" "
                        "name=\"mask_vscpclass\">" );
        if ( bNew ) {
            web_printf( conn, "0xFFFF");
        }
        else {
            web_printf( conn, "0x%04x", pElement->m_vscpfilter.mask_class );
        }
        web_printf( conn, "</textarea>");

        web_printf( conn, "</td></tr>");

        // Type
        web_printf( conn, 
                        "<tr class=\"invisable\"><td class=\"invisable\">"
                        "Type:</td><td class=\"invisable\"><textarea cols=\"10\" "
                        "rows=\"1\" name=\"filter_vscptype\">");
        if ( bNew ) {
            web_printf( conn, "");;
        }
        else {
            web_printf( conn,"%d", pElement->m_vscpfilter.filter_type );
        }
        web_printf( conn, "</textarea>");

        web_printf( conn, 
                        "</td><td> <textarea style=\"background-color: "
                        "#72A4D2;\" cols=\"10\" rows=\"1\" "
                        "name=\"mask_vscptype\">");
        if ( bNew ) {
            web_printf( conn, "0xFFFF");;
        }
        else {
            web_printf( conn,"0x%04x", pElement->m_vscpfilter.mask_type );
        }
        web_printf( conn, "</textarea>");

        web_printf( conn, "</td></tr>");

        // GUID
        if ( !bNew ) vscp_writeGuidArrayToString( pElement->m_vscpfilter.filter_GUID, str );
        web_printf( conn, 
                        "<tr class=\"invisable\"><td class=\"invisable\">"
                        "GUID:</td><td class=\"invisable\"><textarea "
                        "cols=\"50\" rows=\"1\" name=\"filter_vscpguid\">");
        if ( bNew ) {
            web_printf( conn, "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00");
        }
        else {
            web_printf( conn, "%s", (const char *)str.mbc_str() );
        }
        web_printf( conn, "</textarea></td>");

        if ( !bNew ) vscp_writeGuidArrayToString( pElement->m_vscpfilter.mask_GUID, str );
        web_printf( conn, 
                        "<tr class=\"invisable\"><td class=\"invisable\"> "
                        "</td><td class=\"invisable\"><textarea "
                        "style=\"background-color: #72A4D2;\" cols=\"50\" "
                        "rows=\"1\" name=\"mask_vscpguid\">" );
        if ( bNew ) {
            web_printf( conn, "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00");
        }
        else {
            web_printf( conn, "%s", (const char *)str.mbc_str() );
        }
        web_printf( conn, "</textarea></td>");

        web_printf( conn, "</tr>");

        // Index
        web_printf( conn, 
                        "<tr class=\"invisable\"><td class=\"invisable\">"
                        "Index:</td><td class=\"invisable\"><textarea "
                        "cols=\"10\" rows=\"1\" name=\"vscpindex\">");
        if ( bNew ) {
            web_printf( conn, "");
        }
        else {
            web_printf( conn, "%d", pElement->m_index );
        }
        web_printf( conn, "</textarea>");

        // Use measurement index
        web_printf( conn, 
                        "&nbsp;&nbsp;<input name=\"check_measurementindex\" "
                        "value=\"true\" ");
        if ( bNew ) {
            web_printf( conn, "");
        }
        else {
            web_printf( conn, "%s",
                pElement->m_bCheckMeasurementIndex ? "checked" : "" ) ;
        }
        web_printf( conn, " type=\"checkbox\">");
        web_printf( conn, 
                        "<span id=\"optiontext\">Use measurement index "
                        "(only for measurement events)</span>");
        web_printf( conn, "</td></tr>");

        // Zone
        web_printf( conn, 
                        "<tr class=\"invisable\"><td class=\"invisable\">"
                        "Zone:</td><td class=\"invisable\"><textarea "
                        "cols=\"10\" rows=\"1\" name=\"vscpzone\">" );
        if ( bNew ) {
            web_printf( conn, "0");
        }
        else {
            web_printf( conn, "%d", pElement->m_zone );
        }
        web_printf( conn, "</textarea></td></tr>");

        // Subzone
        web_printf( conn, "<tr class=\"invisable\"><td class=\"invisable\">Subzone:</td><td class=\"invisable\"><textarea cols=\"10\" rows=\"1\" name=\"vscpsubzone\">");
        if ( bNew ) {
            web_printf( conn, "0");
        }
        else {
            web_printf( conn, "%d", pElement->m_subzone );
        }
        web_printf( conn, "</textarea>");
        web_printf( conn, "</td></tr>");

        web_printf( conn, "</tbody></table><br>");

        // Control
        web_printf( conn, "<h4>Control:</h4>");

        // Enable row
        web_printf( conn, "<input name=\"check_enablerow\" value=\"true\" ");
        if ( bNew ) {
            web_printf( conn, "");
        }
        else {
            web_printf( conn, "%s",
                pElement->isEnabled() ? "checked" : "" );
        }
        web_printf( conn, " type=\"checkbox\">");
        web_printf( conn, "<span id=\"optiontext\">Enable row</span>&nbsp;&nbsp;");

        // Check Index
        web_printf( conn, "<input name=\"check_index\" value=\"true\"");
        if ( bNew ) {
            web_printf( conn, "");
        }
        else {
            web_printf( conn, 
                            "%s",
                            pElement->isCheckIndexSet() ? "checked" : "" );
        }
        web_printf( conn, " type=\"checkbox\">");
        web_printf( conn, "<span id=\"optiontext\">Check Index</span>&nbsp;&nbsp;");

        // Check Zone
        web_printf( conn, "<input name=\"check_zone\" value=\"true\"");
        if ( bNew ) {
            web_printf( conn, "");
        }
        else {
            web_printf( conn, 
                            "%s",
                            pElement->isCheckZoneSet() ? "checked" : "" );
        }
        web_printf( conn, " type=\"checkbox\">");
        web_printf( conn, "<span id=\"optiontext\">Check Zone</span>&nbsp;&nbsp;");

        // Check subzone
        web_printf( conn, "<input name=\"check_subzone\" value=\"true\"");
        if ( bNew ) {
            web_printf( conn, "");
        }
        else {
            web_printf( conn,
                            "%s",
                            pElement->isCheckSubZoneSet() ? "checked" : "" );
        }
        web_printf( conn, " type=\"checkbox\">");
        web_printf( conn, "<span id=\"optiontext\">Check Subzone</span>&nbsp;&nbsp;");
        web_printf( conn, "<br><br><br>");

        web_printf( conn, "<h4>Allowed From:</h4>");
        web_printf( conn, "<i>Enter * for beginning of time.</i><br>");
        web_printf( conn, "<textarea cols=\"50\" rows=\"1\" name=\"allowedfrom\">");
        if ( bNew ) {
            web_printf( conn, "yyyy-mm-dd hh:mm:ss");
        }
        else {
            web_printf( conn, 
                        pElement->m_timeAllow.m_fromTime.FormatISODate().mbc_str() );
            web_printf( conn, " ");
            web_printf( conn, 
                        pElement->m_timeAllow.m_fromTime.FormatISOTime().mbc_str() );
        }
        web_printf( conn, "</textarea>");

        web_printf( conn, "<h4>Allowed To:</h4>");
        web_printf( conn, "<i>Enter * for end of time (always).</i><br>");
        web_printf( conn, "<textarea cols=\"50\" rows=\"1\" name=\"allowedto\">");
        if ( bNew ) {
            web_printf( conn, "yyyy-mm-dd hh:mm:ss");
        }
        else {
            web_printf( conn, 
                        pElement->m_timeAllow.m_endTime.FormatISODate().mbc_str() );
            web_printf( conn, " ");
            web_printf( conn, 
                        pElement->m_timeAllow.m_endTime.FormatISOTime().mbc_str() );
        }
        web_printf( conn, "</textarea>");

        web_printf( conn, "<h4>Allowed time:</h4>");
        web_printf( conn, "<i>Enter * for always.</i><br>");
        web_printf( conn, "<textarea cols=\"50\" rows=\"1\" name=\"allowedtime\">");
        if ( bNew ) {
            web_printf( conn, "yyyy-mm-dd hh:mm:ss");
        }
        else {
            web_printf( conn, 
                        pElement->m_timeAllow.getActionTimeAsString().mbc_str() );
        }
        web_printf( conn, "</textarea>");

        web_printf( conn, "<h4>Allowed days:</h4>");
        web_printf( conn, "<input name=\"monday\" value=\"true\" ");

        if ( !bNew ) {
            web_printf( conn, 
                            "%s",
                            pElement->m_timeAllow.m_weekDay[0] ? "checked" : "" );
        }
        web_printf( conn, " type=\"checkbox\">Monday ");

        web_printf( conn, "<input name=\"tuesday\" value=\"true\" ");
        if ( !bNew ) {
            web_printf( conn, 
                            "%s",
                            pElement->m_timeAllow.m_weekDay[1] ? "checked" : "" );
        }
        web_printf( conn, " type=\"checkbox\">Tuesday ");

        web_printf( conn, "<input name=\"wednesday\" value=\"true\" ");
        if ( !bNew ) {
            web_printf( conn, 
                            "%s",
                            pElement->m_timeAllow.m_weekDay[2] ? "checked" : "" );
        }
        web_printf( conn, " type=\"checkbox\">Wednesday ");

        web_printf( conn, "<input name=\"thursday\" value=\"true\" ");
        if ( !bNew ) {
            web_printf( conn, 
                            "%s",
                            pElement->m_timeAllow.m_weekDay[3] ? "checked" : "" );
        }
        web_printf( conn, " type=\"checkbox\">Thursday ");

        web_printf( conn, "<input name=\"friday\" value=\"true\" ");
        if ( !bNew ) {
            web_printf( conn, 
                            "%s",
                            pElement->m_timeAllow.m_weekDay[4] ? "checked" : "" );
        }
        web_printf( conn, " type=\"checkbox\">Friday ");

        web_printf( conn, "<input name=\"saturday\" value=\"true\" ");
        if ( !bNew ) {
            web_printf( conn, 
                            "%s",
                            pElement->m_timeAllow.m_weekDay[5] ? "checked" : "" );
        }
        web_printf( conn, " type=\"checkbox\">Saturday ");

        web_printf( conn, "<input name=\"sunday\" value=\"true\" ");
        if ( !bNew ) {
            web_printf( conn, 
                            "%s",
                            pElement->m_timeAllow.m_weekDay[6] ? "checked" : "" );
        }
        web_printf( conn, " type=\"checkbox\">Sunday ");
        web_printf( conn, "<br>");

        web_printf( conn, "<h4>Action:</h4>");

        web_printf( conn, "<select name=\"action\">");
        web_printf( conn, "<option value=\"0\" ");
        if (bNew) web_printf( conn, " selected ");
        web_printf( conn, ">No Operation</option>");

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x10 == pElement->m_actionCode ) ? _("selected") : _(" ");
        }
        web_printf( conn, 
                        "<option value=\"0x10\" %s>"
                        "Execute external program</option>",
                        (const char *)str.mbc_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x12 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        web_printf( conn, 
                        "<option value=\"0x12\" %s>Execute internal "
                        "procedure</option>",
                        (const char *)str.mbc_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x30 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        web_printf( conn, 
                        "<option value=\"0x30\" %s>Execute library "
                        "procedure</option>" ,
                        (const char *)str.mbc_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x40 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        web_printf( conn, 
                        "<option value=\"0x40\" %s>Send event</option>",
                        (const char *)str.mbc_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x41 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        web_printf( conn, 
                        "<option value=\"0x41\" %s>Send event "
                        "Conditional</option>",
                        (const char *)str.mbc_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x42 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        web_printf( conn, 
                        "<option value=\"0x42\" %s>Send event(s) from "
                        "file</option>",
                        (const char *)str.mbc_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x43 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        web_printf( conn, 
                        "<option value=\"0x43\" %s>Send event(s) to remote "
                        "VSCP server</option>",
                        (const char *)str.mbc_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x50 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        web_printf( conn, 
                        "<option value=\"0x50\" %s>Store in variable</option>",
                        (const char *)str.mbc_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x51 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        web_printf( conn, 
                        "<option value=\"0x51\" %s>Store in array</option>",
                        (const char *)str.mbc_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x52 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        web_printf( conn, 
                        "<option value=\"0x52\" %s>Add to variable</option>",
                        (const char *)str.mbc_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = ( 0x53 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        web_printf( conn, 
                        "<option value=\"0x53\" %s>Subtract from variable</option>",
                        (const char *)str.mbc_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x54 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        web_printf( conn, 
                        "<option value=\"0x54\" %s>Multiply variable</option>",
                        (const char *)str.mbc_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x55 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        web_printf( conn, 
                        "<option value=\"0x55\" %s>Divide variable</option>",
                        (const char *)str.mbc_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x60 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        web_printf( conn, 
                        "<option value=\"0x60\" %s>Start timer</option>",
                        (const char *)str.mbc_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x61 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        web_printf( conn, 
                        "<option value=\"0x61\" %s>Pause timer</option>",
                        (const char *)str.mbc_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x62 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        web_printf( conn,
                        "<option value=\"0x62\" %s>Stop timer</option>",
                        (const char *)str.mbc_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x63 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        web_printf( conn,
                        "<option value=\"0x63\" %s>Resume timer</option>",
                        (const char *)str.mbc_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x70 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        web_printf( conn,
                        "<option value=\"0x70\" %s>Write file</option>",
                        (const char *)str.mbc_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x75 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        web_printf( conn,
                        "<option value=\"0x75\" %s>Get/Put/Post URL</option>",
                        (const char *)str.mbc_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x80 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        web_printf( conn,
                        "<option value=\"0x80\" %s>Write to table</option>",
                        (const char *)str.mbc_str() );

        web_printf( conn, "</select>");

        web_printf( conn, 
                        " <a href=\"http://www.vscp.org/docs/vscpd/doku.php"
                        "?id=vscp_daemon_decision_matrix#level_ii\" "
                        "target=\"new\">Help for actions and parameters</a><br>");

        web_printf( conn, "<h4>Action parameter:</h4>");
        web_printf( conn, "<textarea cols=\"80\" rows=\"5\" name=\"actionparameter\">");
        if ( !bNew ) web_printf( conn, pElement->m_actionparam.mbc_str() );
        web_printf( conn, "</textarea>");


        web_printf( conn, "<h4>Comment:</h4>");
        web_printf( conn, "<textarea cols=\"80\" rows=\"5\" name=\"comment\">");
        if ( !bNew ) web_printf( conn, pElement->m_comment.mbc_str() );
        web_printf( conn, "</textarea>");
    }
    else {
        web_printf( conn, "<br><b>Error: Non existent id</b>");
    }

    web_printf( conn, WEB_DMEDIT_SUBMIT);
    web_printf( conn, "</form>");
    web_printf( conn, WEB_COMMON_END);     // Common end code

    return 1;
}


///////////////////////////////////////////////////////////////////////////////
// vscp_dm_post
//

static int vscp_dm_post( struct web_connection *conn, void *cbdata )
{
    char buf[32000];
    wxString str;
    VSCPInformation vscpinfo;
    dmElement *pElement = NULL;

    // Check pointer
    if (NULL == conn) return 0;
    
    const struct web_request_info *reqinfo =  
                web_get_request_info( conn );
    if ( NULL == reqinfo ) return 0;

    // id
    long id = -1;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "id", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            id = atoi( buf );
        }
    }

    // From
    long nFrom = 0;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "from", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            nFrom = atoi( buf );
        }
    }

    // Count
    uint16_t nCount = 50;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "count", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            nCount = atoi( buf );
        }
    }

    // Flag for new DM row
    bool bNew = false;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "new", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            if ( NULL != strstr( "true", buf ) ) bNew = true;
        }
    }

    wxString strGroupID;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "groupid", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            strGroupID = wxString::FromAscii(buf);
        }
    }

    int filter_priority = -1;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "filter_priority", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            filter_priority = vscp_readStringValue( wxString::FromAscii( buf ) );
        }
    }

    int mask_priority = 0;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "mask_priority", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            mask_priority = vscp_readStringValue( wxString::FromAscii( buf ) );
        }
    }

    uint16_t filter_vscpclass = -1;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "filter_vscpclass", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            wxString wrkstr = wxString::FromAscii( buf );
            filter_vscpclass = vscp_readStringValue( wrkstr );
        }
    }

    uint16_t mask_vscpclass = 0;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "mask_vscpclass", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            mask_vscpclass = vscp_readStringValue( wxString::FromAscii( buf ) );
        }
    }

    uint16_t filter_vscptype = 0;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "filter_vscptype", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            filter_vscptype = vscp_readStringValue( wxString::FromAscii( buf ) );
        }
    }

    uint16_t mask_vscptype = 0;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "mask_vscptype", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            mask_vscptype = vscp_readStringValue( wxString::FromAscii( buf ) );
        }
    }

    wxString strFilterGuid;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "filter_vscpguid", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            strFilterGuid = wxString::FromAscii( buf );
            strFilterGuid = strFilterGuid.Trim();
            strFilterGuid = strFilterGuid.Trim(false);
        }
    }

    wxString strMaskGuid;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "mask_vscpguid", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            strMaskGuid = wxString::FromAscii( buf );
            strMaskGuid = strMaskGuid.Trim();
            strMaskGuid = strMaskGuid.Trim(false);
        }
    }

    uint8_t index = 0;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "vscpindex", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            index = vscp_readStringValue( wxString::FromAscii( buf ) );
        }
    }

    bool bUseMeasurementIndex = false;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "check_measurementindex", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            if ( NULL != strstr( "true", buf ) ) bUseMeasurementIndex = true;
        }
    }

    uint8_t zone = 0;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "vscpzone", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            zone = vscp_readStringValue( wxString::FromAscii( buf ) );
        }
    }

    uint8_t subzone = 0;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "vscpsubzone", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            subzone = vscp_readStringValue( wxString::FromAscii( buf ) );
        }
    }

    bool bEnableRow = false;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "check_enablerow", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            if ( NULL != strstr( "true", buf ) ) bEnableRow = true;
        }
    }

    bool bEndScan = false;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "check_endscan", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            if ( NULL != strstr( "true", buf ) ) bEndScan = true;
        }
    }

    bool bCheckIndex = false;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "check_index", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            if ( NULL != strstr( "true", buf ) ) bCheckIndex = true;
        }
    }

    bool bCheckZone = false;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "check_zone", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            if ( NULL != strstr( "true", buf ) ) bCheckZone = true;
        }
    }

    bool bCheckSubZone = false;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "check_subzone", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            if ( NULL != strstr( "true", buf ) ) bCheckSubZone = true;
        }
    }

    wxString strAllowedFrom;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "allowedfrom", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            strAllowedFrom = wxString::FromAscii( buf );
            strAllowedFrom.Trim( true );
            strAllowedFrom.Trim( false );
            if ( _("*") == strAllowedFrom ) {
                strAllowedFrom = _("0000-01-01 00:00:00");
            }
        }
    }
    
    wxString strAllowedTo;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "allowedto",
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            strAllowedTo = wxString::FromAscii( buf );
            strAllowedTo.Trim( true );
            strAllowedTo.Trim( false );
            if ( _("*") == strAllowedTo ) {
                strAllowedTo = _("9999-12-31 23:59:59");
            }
        }
    }
    
    wxString strAllowedTime;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "allowedtime", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            strAllowedTime = wxString::FromAscii( buf );
            strAllowedTime.Trim( true );
            strAllowedTime.Trim( false );
            if ( _("*") == strAllowedTime ) {
                strAllowedTime = _("* *");
            }
        }
    }
    
    
    bool bCheckMonday = false;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "monday", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            if ( NULL != strstr( "true", buf ) ) bCheckMonday = true;
        }
    }

    bool bCheckTuesday = false;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "tuesday", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            if ( NULL != strstr( "true", buf ) ) bCheckTuesday = true;
        }
    }

    bool bCheckWednesday = false;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "wednesday", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            if ( NULL != strstr( "true", buf ) ) bCheckWednesday = true;
        }
    }

    bool bCheckThursday = false;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "thursday", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            if ( NULL != strstr( "true", buf ) ) bCheckThursday = true;
        }
    }
    
    bool bCheckFriday = false;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "friday", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            if ( NULL != strstr( "true", buf ) ) bCheckFriday = true;
        }
    }
    

    bool bCheckSaturday = false;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "saturday", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            if ( NULL != strstr( "true", buf ) ) bCheckSaturday = true;
        }
    }
    

    bool bCheckSunday = false;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "sunday", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            if ( NULL != strstr( "true", buf ) ) bCheckSunday = true;
        }
    }

    uint32_t action = 0;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "action", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            action = vscp_readStringValue( wxString::FromAscii( buf ) );
        }
    }

    wxString strActionParameter;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "actionparameter", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            vscp_toXMLEscape( buf );
            strActionParameter = wxString::FromAscii( buf );
        }
    }

    wxString strComment;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "comment", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            vscp_toXMLEscape( buf );
            strComment = wxString::FromAscii( buf );
        }
    }

    web_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");

    web_printf( conn, 
                    WEB_COMMON_HEAD, 
                    "VSCP - Decision Matrix Post" );
    web_printf( conn, WEB_STYLE_START);
    web_printf( conn, WEB_COMMON_CSS);     // CSS style Code
    web_printf( conn, WEB_STYLE_END);
    web_printf( conn, WEB_COMMON_JS);      // Common Javascript code
    web_printf( conn, "<meta http-equiv=\"refresh\" content=\"2;url=/vscp/dm");
    web_printf( conn, 
                    "?from=%ld&count=%ld", 
                    (long)nFrom, 
                    (long)nCount );
    web_printf( conn, "\">");
    web_printf( conn, WEB_COMMON_HEAD_END_BODY_START);

    // Insert server url into navigation menu
    wxString navstr = _(WEB_COMMON_MENU);
    int pos;
    while ( wxNOT_FOUND != ( pos = navstr.Find(_("%s")))) {
        web_printf( conn, navstr.Left( pos ).mbc_str() );
        navstr = navstr.Right(navstr.Length() - pos - 2);
    }
    web_printf( conn, navstr.mbc_str() );

    web_printf( conn, WEB_DMPOST_BODY_START);

    if ( bNew ) {
        pElement = new dmElement;
    }

    if ( bNew || ( id >= 0 ) ) {

        if ( bNew || ((0 == id) && !bNew) || ( id < gpobj->m_dm.getMemoryElementCount() ) ) {

            if (!bNew) pElement = gpobj->m_dm.getMemoryElementFromRow(id);

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
                pElement->m_bCheckMeasurementIndex = bUseMeasurementIndex;
                pElement->m_subzone = subzone;

                bCheckIndex ? pElement->m_bCheckIndex = true : pElement->m_bCheckIndex = false;
                bCheckZone ? pElement->m_bCheckZone = true : pElement->m_bCheckZone = false;
                bCheckSubZone ? pElement->m_bCheckSubZone = true : pElement->m_bCheckSubZone = false;

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

                pElement->m_actionCode = action;

                pElement->m_actionparam = strActionParameter;
                pElement->m_comment = strComment;

                pElement->m_strGroupID = strGroupID;

                pElement->m_triggCounter = 0;
                pElement->m_errorCounter = 0;

                if ( bNew ) {
                    // add the DM row to the matrix
                    gpobj->m_dm.addMemoryElement(pElement);
                }

                // Save decision matrix
                gpobj->m_dm.saveToXML();

                web_printf( conn, 
                                "<br><br>DM Entry has been saved. id=%d", 
                                id);
            }
            else {
                web_printf( conn, 
                                "<br><br>Memory problem id=%d. Unable to save record", 
                                id);
            }

        } 
        else {
            web_printf( conn, 
                            "<br><br>Record id=%d is to large. Unable to save record",
                            id );
        }
    } 
    else {
        web_printf( conn, 
                        "<br><br>Record id=%d is wrong. Unable to save record", 
                        id );
    }

    web_printf( conn, WEB_COMMON_END); // Common end code

    return 1;
}


///////////////////////////////////////////////////////////////////////////////
// vscp_dm_delete
//

static int vscp_dm_delete( struct web_connection *conn, void *cbdata  )
{
    char buf[80];
    wxString str;
    VSCPInformation vscpinfo;

    // Check pointer
    if (NULL == conn) return 0;
    
    const struct web_request_info *reqinfo =  
                web_get_request_info( conn );
    if ( NULL == reqinfo ) return 0;

    // id
    long id = -1;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "id", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            id = atoi( buf );
        }
    }

    // From
    long nFrom = 0;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "from", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            nFrom = atoi( buf );
        }
    }

    // Count
    uint16_t nCount = 50;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "count", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            nCount = atoi( buf );
        }
    }

    web_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");

    web_printf( conn, 
                    WEB_COMMON_HEAD, 
                    "VSCP - Decision Matrix Delete" );
    web_printf( conn, WEB_STYLE_START);
    web_printf( conn, WEB_COMMON_CSS);     // CSS style Code
    web_printf( conn, WEB_STYLE_END);
    web_printf( conn, WEB_COMMON_JS);      // Common Javascript code
    web_printf( conn, "<meta http-equiv=\"refresh\" content=\"2;url=/vscp/dm");
    web_printf( conn, 
                    "?from=%d&count=%d", 
                    nFrom, 
                    nCount );
    web_printf( conn, "\">");
    web_printf( conn, WEB_COMMON_HEAD_END_BODY_START);

    // Navigation menu
    web_printf( conn, WEB_COMMON_MENU);

    web_printf( conn, WEB_DMEDIT_BODY_START);

    if ( gpobj->m_dm.removeMemoryElement( id ) ) {
        web_printf( conn, 
                        "<br>Deleted record id = %d", 
                        id );
        // Save decision matrix
        gpobj->m_dm.saveToXML();
    }
    else {
        web_printf( conn, 
                        "<br>Failed to remove record id = %d", 
                        id );
    }

    web_printf( conn, WEB_COMMON_END);     // Common end code

    return 1;
}

//-----------------------------------------------------------------------------
//                                    Variables
//-----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// vscp_variable_list
//

static int vscp_variable_list( struct web_connection *conn, void *cbdata  )
{
    char buf[80];
    VSCPInformation vscpinfo;
    unsigned long upperLimit = 50;
    
    // Check pointer
    if (NULL == conn) return 0;
    
    // get variable names
    wxArrayString nameArray;
    gpobj->m_VSCP_Variables.getVarlistFromRegExp( nameArray ); 

    const struct web_request_info *reqinfo =  
                web_get_request_info( conn );
    if ( NULL == reqinfo ) return 0;
    
    // From
    long nFrom = 0;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "from", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            nFrom = atoi( buf );
        }
    }


    // Count
    unsigned long nCount = 50;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "count", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            nCount = atoi( buf );
        }
    }


    // Navigation button
    if (NULL != reqinfo->query_string) {
        
        if (web_get_var(reqinfo->query_string,
                strlen(reqinfo->query_string),
                "navbtn",
                buf,
                sizeof ( buf)) > 0) {

            if (NULL != strstr("previous", buf)) {
                nFrom -= nCount;
                if (nFrom < 0) nFrom = 0;
            } 
            else if (NULL != strstr("next", buf)) {
                nFrom += nCount;
                
                if ( (unsigned long)nFrom > nameArray.Count()-1 ) {
                    if ( nameArray.Count() % nCount ) {
                        nFrom = nameArray.Count()/nCount;
                    }
                    else {
                        nFrom = (nameArray.Count()/nCount) - 1;
                    }
                }
            } 
            else if (NULL != strstr("last", buf)) {
                 if ( nameArray.Count() % nCount ) {
                    nFrom = (nameArray.Count()/nCount)*nCount;
                }
                else {
                    nFrom = ((nameArray.Count()/nCount) - 1)*nCount;
                }
            } 
            else if (NULL != strstr("first", buf)) {
                nFrom = 0;
            }
        } 
        else { // No vaid navigation value
            nFrom = 0;
        }
    }

    web_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");


    web_printf( conn, WEB_COMMON_HEAD, "VSCP - Variables" );
    web_printf( conn, WEB_STYLE_START );
    web_printf( conn, WEB_COMMON_CSS );     // CSS style Code
    web_printf( conn, WEB_STYLE_END );
    web_printf( conn, WEB_COMMON_JS );      // Common Javascript code
    web_printf( conn, WEB_COMMON_HEAD_END_BODY_START );

    // Navigation menu
    web_printf( conn, WEB_COMMON_MENU );
    web_printf( conn, WEB_VARLIST_BODY_START );

    {
        wxString wxstrurl = _("/vscp/variables");
        web_printf( conn, WEB_COMMON_LIST_NAVIGATION,
                (const char *)wxstrurl.mbc_str(),
                (unsigned long)( nFrom + 1 ),
                ( (unsigned long)(nFrom + nCount) < nameArray.Count() ) ?
                    nFrom + nCount : nameArray.Count(),
                (unsigned long)nameArray.Count(),
                (unsigned long)nCount,
                (unsigned long)nFrom,
                "false" );
        web_printf( conn, "<br>");
    }

    wxString strBuf;

    // Display Variables List

    if ( 0 == nameArray.Count() ) {
        web_printf( conn, "<br>Variables list is empty!<br>");
    }
    else {
        web_printf( conn, WEB_VARLIST_TR_HEAD);
    }

    if ( nFrom < 0 ) nFrom = 0;

    for ( unsigned int i=0; i<nCount; i++ ) {

        // Check if we are done
        if ( ( nFrom + i ) >= nameArray.Count() ) break;

        CVSCPVariable variable;
        if ( 0 == gpobj->m_VSCP_Variables.find( nameArray[ nFrom + i ], 
                                                    variable ) ) {
            web_printf( conn, 
                        "Internal error: Non existent variable entry. "
                        "name = %s idx= %ld<br>",
                        (const char *)variable.getName().mbc_str(),
                        (long)(nFrom + i) );
            continue;
        }
        
        if (  1 || !variable.isStockVariable() ) {
            
            wxString url_dmedit =
                    wxString::Format( _("/vscp/varedit?id=%ld&from=%ld&count=%ld"),
                                        (long)(nFrom+i), 
                                        (long)nFrom, 
                                        (long)nCount );
            wxString str = wxString::Format(_(WEB_COMMON_TR_CLICKABLE_ROW),
                                                url_dmedit.mbc_str() );
            web_printf( conn, str.mbc_str() );
            
        }

        // variable id
        web_printf( conn, WEB_IFLIST_TD_CENTERED );
        web_printf( conn, "<div style=\"font-weight: bold;\">%ld</div>", nFrom + i + 1 );
        web_printf( conn, "</td>" );

        // Type
        web_printf(conn, WEB_IFLIST_TD_CENTERED);
        switch (variable.getType()) {

            case VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED:
                web_printf(conn, "<div id=\"small\" style=\"font-weight: bold;\">Unassigned</div>");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_STRING:
                web_printf(conn, "<div id=\"small\" style=\"font-weight: bold;\">String</div>");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_BOOLEAN:
                web_printf(conn, "<div id=\"small\" style=\"font-weight: bold;\">Boolean</div>");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_INTEGER:
                web_printf(conn, "<div id=\"small\" style=\"font-weight: bold;\">Integer</div>");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_LONG:
                web_printf(conn, "<div id=\"small\" style=\"font-weight: bold;\">Long</div>");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_DOUBLE:
                web_printf(conn, "<div id=\"small\" style=\"font-weight: bold;\">Double</div>");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_MEASUREMENT:
                web_printf(conn, "<div id=\"small\" style=\"font-weight: bold;\">Measurement</div>");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT:
                web_printf(conn, "<div id=\"small\" style=\"font-weight: bold;\">Event</div>");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID:
                web_printf(conn, "<div id=\"small\" style=\"font-weight: bold;\">GUID</div>");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_DATA:
                web_printf(conn, "<div id=\"small\" style=\"font-weight: bold;\">Event data</div>");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_CLASS:
                web_printf(conn, "<div id=\"small\" style=\"font-weight: bold;\">Event class</div>");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TYPE:
                web_printf(conn, "<div id=\"small\" style=\"font-weight: bold;\">Event type</div>");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TIMESTAMP:
                web_printf(conn, "<div id=\"small\" style=\"font-weight: bold;\">Event timestamp</div>");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_DATETIME:
                web_printf(conn, "<div id=\"small\" style=\"font-weight: bold;\">Date and time</div>");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_DATE:
                web_printf(conn, "<div id=\"small\" style=\"font-weight: bold;\">Date</div>");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_TIME:
                web_printf(conn, "<div id=\"small\" style=\"font-weight: bold;\">Time</div>");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_BLOB:
                web_printf(conn, "<div id=\"small\" style=\"font-weight: bold;\">Blob</div>");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_MIME:
                web_printf(conn, "<div id=\"small\" style=\"font-weight: bold;\">Mime type</div>");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_HTML:
                web_printf(conn, "<div id=\"small\" style=\"font-weight: bold;\">HTML</div>");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_JAVASCRIPT:
                web_printf(conn, "<div id=\"small\" style=\"font-weight: bold;\">JavaScript</div>");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_JSON:
                web_printf(conn, "<div id=\"small\" style=\"font-weight: bold;\">JSON</div>");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_XML:
                web_printf(conn, "<div id=\"small\" style=\"font-weight: bold;\">XML</div>");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_SQL:
                web_printf(conn, "<div id=\"small\" style=\"font-weight: bold;\">SQL</div>");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_LUA:
                web_printf(conn, "<div id=\"small\" style=\"font-weight: bold;\">Lua</div>");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_LUA_RESULT:
                web_printf(conn, "<div id=\"small\" style=\"font-weight: bold;\">Lua result</div>");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_UX_TYPE1:
                web_printf(conn, "<div id=\"small\" style=\"font-weight: bold;\">UX type 1</div>");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_DM_ROW:
                web_printf(conn, "<div id=\"small\" style=\"font-weight: bold;\">DM row</div>");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_DRIVER:
                web_printf(conn, "<div id=\"small\" style=\"font-weight: bold;\">Driver</div>");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_USER:
                web_printf(conn, "<div id=\"small\" style=\"font-weight: bold;\">User</div>");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_FILTER:
                web_printf(conn, "<div id=\"small\" style=\"font-weight: bold;\">Filter</div>");
                break;

            default:
                web_printf(conn, "<div id=\"small\" style=\"font-weight: bold;\">Unknown type</div>");
                break;

        }

        web_printf(conn, "</td>");

        // Variable entry
        web_printf(conn, "<td>");

        web_printf(conn, "<div id=\"small\">");

        web_printf(conn, "<h4>");
        web_printf(conn, variable.getName().MakeLower().mbc_str() );
        web_printf(conn, "</h4>");

        wxString strValue;
        variable.writeValueToString( strValue, true );
        web_printf( conn, "<b>Value:</b> ");
        web_printf( conn, strValue.mbc_str() );

        web_printf( conn, "<br>");
        web_printf( conn, "<b>Note:</b> ");
        wxString strNote;
        variable.getNote(strNote, true);
        web_printf(conn, strNote.mbc_str());

        web_printf(conn, "<br>");
        web_printf(conn, "<b>Persistent: </b> ");
        if (variable.isPersistent()) {
            web_printf(conn, "yes");
        } 
        else {
            web_printf(conn, "no");
        }
        
        // User
        web_printf(conn, "<br>");
        web_printf(conn, "<b>Owner: </b> ");
        web_printf(conn, "id=%X = ", variable.getOwnerID() );
        CUserItem *pUser = gpobj->m_userList.getUserItemFromOrdinal( variable.getOwnerID() );
        if ( NULL == pUser ) {
            web_printf(conn, " Unknow user " );
        }
        web_printf(conn, 
                    "%s (%s)",
                    (const char *)pUser->getUser().mbc_str(),
                    (const char *)pUser->getFullname().mbc_str() );
        
        // Access rights
        web_printf(conn, "<br>");
        web_printf(conn, "<b>Access rights: </b> ");
        web_printf(conn, "%X ", variable.getAccessRights() );
        
        // Last change
        web_printf(conn, "<br>");
        web_printf(conn, "<b>Last changed: </b> ");
        web_printf(conn, "%s ", 
                (const char *)variable.getLastChange().FormatISOCombined().mbc_str() );

        web_printf(conn, "</div>");

   
        web_printf( conn, "</td>");
        
        // Delete button
        web_printf( conn, WEB_IFLIST_TD_CENTERED );
        if ( variable.isStockVariable() ) {
            web_printf( conn, "---" );
        }
        else {
            web_printf( conn, 
                            "<form name=\"input\" action=\"/vscp/vardelete?id=%ld\" "
                            "method=\"get\"><input type=\"submit\" value=\"x\" >"
                            "<input type=\"hidden\" name=\"id\"value=\"%ld\" ></form>",
                            (long)( nFrom + i ), 
                            (long)( nFrom + i ) );
        }
        web_printf( conn, "</td>" );
        
        
        web_printf( conn, "</tr>");

    } // for

    web_printf( conn, WEB_DMLIST_TABLE_END);

    {
        wxString wxstrurl = _("/vscp/variables");
        web_printf( conn, 
                    WEB_COMMON_LIST_NAVIGATION,
                    (const char *)wxstrurl.mbc_str(),
                    (unsigned long)(nFrom+1),
                    ( (unsigned long)(nFrom + nCount) < nameArray.Count() ) ?
                        nFrom + nCount : nameArray.Count(),
                    (unsigned long)nameArray.Count(),
                    (unsigned long)nCount,
                    (unsigned long)nFrom,
                    "false" );
    }

    web_printf( conn, WEB_COMMON_END);     // Common end code


    return 1;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_variable_edit
//

static int vscp_variable_edit( struct web_connection *conn, void *cbdata  )
{
    char buf[80];
    wxString str;
    VSCPInformation vscpinfo;
    CVSCPVariable variable;

    // Check pointer
    if (NULL == conn) return 0;
    
    const struct web_request_info *reqinfo =  
                web_get_request_info( conn );
    if ( NULL == reqinfo ) return 0;

    // id
    long id = -1;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "id", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            id = atoi( buf );
        }
    }

    // type
    uint8_t nType = VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "type", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            nType = atoi( buf );
        }
    }

    // Flag for new variable row
    bool bNew = false;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "new", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            if ( NULL != strstr( "true", buf ) ) bNew = true;
        }
    }

    // From
    long nFrom = 0;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "from", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            nFrom = atoi( buf );
        }
    }


    // Count
    uint16_t nCount = 50;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "count", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            nCount = atoi( buf );
        }
    }

    web_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");

    web_printf( conn, WEB_COMMON_HEAD, "VSCP - Variable Edit" );
    web_printf( conn, WEB_STYLE_START);
    web_printf( conn, WEB_COMMON_CSS);     // CSS style Code
    web_printf( conn, WEB_STYLE_END);
    web_printf( conn, WEB_COMMON_JS);      // Common Javascript code
    web_printf( conn, WEB_COMMON_HEAD_END_BODY_START);

    // Navigation menu
    web_printf( conn, WEB_COMMON_MENU);

    web_printf( conn, WEB_VAREDIT_BODY_START);
    
    // get variable names
    wxArrayString nameArray;
    gpobj->m_VSCP_Variables.getVarlistFromRegExp( nameArray ); 

    if ( !bNew ) {
                
        if ( 0 == gpobj->m_VSCP_Variables.find( nameArray[ id ], 
                                                    variable ) ) {
            web_printf( conn, 
                        "Internal error: Non existent variable entry. "
                        "name = %s idx= %ld<br>",
                        (const char *)variable.getName().mbc_str(),
                        (long)id );
            
            web_printf( conn, WEB_COMMON_END);     // Common end code
            return 1;
        }
        
    }

    if ( bNew ) {
        web_printf( conn,
                        "<br><span id=\"optiontext\">New record.</span><br>");
    } 
    else {
        web_printf( conn,
                        "<br><span id=\"optiontext\">Record = %d.</span><br>",
                        id );
    }

    web_printf(conn, "<br><form method=\"get\" action=\"");
    web_printf(conn, "/vscp/varpost");
    web_printf(conn, "\" name=\"varedit\">");

    // Hidden from
    web_printf( conn,
                    "<input name=\"from\" value=\"%d\" type=\"hidden\">",
                    nFrom );

    // Hidden count
    web_printf( conn,
                    "<input name=\"count\" value=\"%d\" type=\"hidden\">",
                    nCount );

    // Hidden id
    web_printf( conn,
                    "<input name=\"id\" value=\"%d\" type=\"hidden\">",
                    id );

    if ( bNew ) {
        // Hidden new
        web_printf(conn,
                "<input name=\"new\" value=\"true\" type=\"hidden\">");
    } 
    else {
        // Hidden new
        web_printf( conn,
                        "<input name=\"new\" value=\"false\" type=\"hidden\">");
    }

    // Hidden type
    web_printf( conn, "<input name=\"type\" value=\"");
    web_printf( conn, "%d", bNew ? nType : variable.getType());
    web_printf( conn, "\" type=\"hidden\"></input>");

    web_printf( conn, "<h4>Variable:</h4> <span id=\"optiontext\"></span><br>");

    web_printf( conn, "<table class=\"invisable\"><tbody><tr class=\"invisable\">");

    web_printf( conn, "<td class=\"invisable\"  style=\"font-weight: bold;\">Name:</td><td class=\"invisable\">");
    
    if ( !bNew ) {
        web_printf( conn, "<div style=\"font-weight: bold;\" >" );
        web_printf( conn, (const char *)variable.getName().MakeLower().mbc_str() );
        web_printf( conn, "</div>" );
        web_printf( conn, "<input name=\"value_name\" value=\"");
        web_printf( conn, variable.getName().mbc_str());
        web_printf( conn, "\" type=\"hidden\">");
    } 
    else {
        web_printf(conn, "<textarea cols=\"50\" rows=\"1\" name=\"value_name\"></textarea>");
    }
    
    web_printf(conn, "</td></tr><tr>");
    web_printf(conn, "<td class=\"invisable\" valign=\"middle\" "
                     "style=\"font-weight: bold;\">Value:</td>"
                     "<td class=\"invisable\">");

    if ( !bNew ) nType = variable.getType();

    if ( VSCP_DAEMON_VARIABLE_CODE_STRING == nType ) {

        web_printf(conn, "<textarea cols=\"50\" rows=\"10\" name=\"value_string\">");
        
        if ( bNew ) {
            web_printf(conn, "");
        } 
        else {
            wxString str;
            variable.writeValueToString( str, true );
            web_printf( conn, (const char *)str.mbc_str() );
        }

        web_printf(conn, "</textarea>");
        
        web_printf(conn, "<div id=\"small\">String Format: "
                "String value.</div>" );

    } 
    else if ( VSCP_DAEMON_VARIABLE_CODE_BOOLEAN == nType ) {

        bool bValue = false;
        if (! bNew ) variable.getValue( &bValue );

        web_printf(conn, "<input type=\"radio\" name=\"value_boolean\" "
                         "value=\"true\" ");
        
        if ( !bNew )
            web_printf( conn, "%s", bValue ? "checked >true " : ">true ");
        else {
            web_printf(conn, ">true ");
        }

        web_printf( conn, "<input type=\"radio\" name=\"value_boolean\" "
                          "value=\"false\" ");
        
        if ( !bNew )
            web_printf( conn, "%s", !bValue ? "checked >false " : ">false ");
        else {
            web_printf(conn, ">false ");
        }
        
    } 
    else if ( VSCP_DAEMON_VARIABLE_CODE_INTEGER == nType ) {

        web_printf(conn, "<textarea cols=\"10\" rows=\"1\" "
                         "name=\"value_integer\">");
        
        if ( bNew ) {
            web_printf(conn, "");
        } 
        else {
            wxString str;
            variable.writeValueToString( str );
            web_printf( conn, (const char *)str.mbc_str() );
        }

        web_printf(conn, "</textarea>");
        
        web_printf(conn, "<div id=\"small\">Integer value. Format: "
                "Decimal or hexadecimal (preceed with '0x') signed value.</div>" );

    } 
    else if ( VSCP_DAEMON_VARIABLE_CODE_LONG == nType ) {

        web_printf(conn, "<textarea cols=\"10\" rows=\"1\" name=\"value_long\">");
        
        if ( bNew ) {
            web_printf(conn, "");
        } 
        else {
            wxString str;
            variable.writeValueToString( str );
            web_printf( conn, (const char *)str.mbc_str() );
        }

        web_printf(conn, "</textarea>");
        
        web_printf(conn, "<div id=\"small\">Long value. Format: "
                "Decimal or hexadecimal (preceed with '0x') signed value.</div>" );

    } 
    else if ( VSCP_DAEMON_VARIABLE_CODE_DOUBLE == nType ) {

        web_printf(conn, "<textarea cols=\"10\" rows=\"1\" name=\"value_double\">");
        
        if ( bNew ) {
            web_printf(conn, "");
        } 
        else {
            wxString str;
            variable.writeValueToString( str );
            web_printf( conn, (const char *)str.mbc_str() );
        }

        web_printf(conn, "</textarea>");
        
        web_printf(conn, "<div id=\"small\">Decimal value. Format: "
                "Decimal signed value.</div>" );

    } 
    else if ( VSCP_DAEMON_VARIABLE_CODE_VSCP_MEASUREMENT == nType ) {
        
        web_printf(conn, "<textarea cols=\"50\" rows=\"1\" name=\"value_measurement\">");
        
        if ( bNew ) {
            web_printf(conn, "");
        } 
        else {
            wxString str;
            variable.writeValueToString( str );
            web_printf( conn, (const char *)str.mbc_str() );
        }

        web_printf(conn, "</textarea>");
        
        web_printf(conn, "<div id=\"small\">Measurement value. Format: "
                "value,unit,sensor-index,zone,subzone.</div>" );
    } 
    else if ( VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT == nType ) {
        
        web_printf(conn, "<textarea cols=\"50\" rows=\"1\" name=\"value\">");
        
        if ( bNew ) {
            web_printf(conn, "");
        } 
        else {
            wxString str;
            variable.writeValueToString( str );
            web_printf( conn, (const char *)str.mbc_str() );
        }

        web_printf(conn, "</textarea>");
        
        web_printf(conn, "<div id=\"small\">VSCP Event. Format: "
                "'head,class;type,obid,datetime,timestamp,GUID,data1,data2'</div>" );
       

    } 
    else if ( VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID == nType ) {
                
        web_printf(conn, "<textarea cols=\"50\" rows=\"1\" name=\"value_guid\">");        
        
        if (bNew) {
            web_printf(conn, "");
        } 
        else {
            wxString str;
            variable.writeValueToString( str );
            web_printf( conn, (const char *)str.mbc_str() );
        }

        web_printf(conn, "</textarea>");
        
        web_printf(conn, "<div id=\"small\">Format: "
                "'AA:BB:CC:DD:EE:FF:00:11:22:33:44:55:66:77:88:99'</div>" );

    } 
    else if ( VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_DATA == nType ) {

        web_printf(conn, "<textarea cols=\"50\" rows=\"5\" name=\"value_data\">");
        
        if ( bNew ) {
            web_printf(conn, "");
        } 
        else {
            wxString str;
            variable.writeValueToString( str );
            web_printf( conn, (const char *)str.mbc_str() );
        }

        web_printf(conn, "</textarea>");
        
        web_printf(conn, "<div id=\"small\">VSCP Event data. Format: "
                "'data1,data2,data3,…' Decimal and hex values allowed.</div>" );

    } 
    else if ( VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_CLASS == nType ) {

        web_printf(conn, "<textarea cols=\"10\" rows=\"1\" name=\"value_class\">");
        
        if ( bNew ) {
            web_printf(conn, "");
        } 
        else {
            wxString str;
            variable.writeValueToString( str );
            web_printf( conn, (const char *)str.mbc_str() );
        }

        web_printf(conn, "</textarea>");
        
        web_printf(conn, "<div id=\"small\">VSCP event class code. Format: "
                "Decimal or hexadecimal (preceed with '0x') value 0-65535.</div>" );

    } 
    else if ( VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TYPE == nType ) {

        web_printf(conn, "<textarea cols=\"10\" rows=\"1\" name=\"value_type\">");
        
        if ( bNew ) {
            web_printf(conn, "");
        } 
        else {
            wxString str;
            variable.writeValueToString( str );
            web_printf( conn, (const char *)str.mbc_str() );
        }

        web_printf(conn, "</textarea>");
        
        web_printf(conn, "<div id=\"small\">VSCP event type code. Format: "
                "Decimal or hexadecimal (preceed with '0x') value 0-65535.</div>" );

    } 
    else if ( VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TIMESTAMP == nType ) {

        web_printf(conn, "<textarea cols=\"10\" rows=\"1\" name=\"value_timestamp\">");
        
        if ( bNew ) {
            web_printf(conn, "");
        } 
        else {
            wxString str;
            variable.writeValueToString( str );
            web_printf( conn, (const char *)str.mbc_str() );
        }

        web_printf(conn, "</textarea>");
        
        web_printf(conn, "<div id=\"small\">Timestamp. Format: "
                "Binary or hexadecimal long value.</div>" );

    } 
    else if ( VSCP_DAEMON_VARIABLE_CODE_DATETIME == nType ) {

        web_printf(conn, "<textarea cols=\"20\" rows=\"1\" name=\"value_date_time\">");
        
        if ( bNew ) {
            web_printf(conn, "");
        } 
        else {
            wxString str;
            variable.writeValueToString( str );
            web_printf( conn, (const char *)str.mbc_str() );
        }

        web_printf(conn, "</textarea>");
        
        web_printf(conn, "<div id=\"small\">DateTime. Format: "
                "'yyyy-mm-ddTHH:MM:SS' ex. '2014-09-26T13:05:01'.</div>" );

    } 
    else if ( VSCP_DAEMON_VARIABLE_CODE_DATE == nType ) {
        
        web_printf(conn, "<textarea cols=\"20\" rows=\"1\" name=\"value\">");
        
        if ( bNew ) {
            web_printf(conn, "");
        } 
        else {
            wxString str;
            variable.writeValueToString( str );
            web_printf( conn, (const char *)str.mbc_str() );
        }

        web_printf(conn, "</textarea>");
        
        web_printf(conn, "<div id=\"small\">Date. Format: "
                "'yyyy-mm-dd' ex. '2014-09-26'.</div>" );
        
    }
    else if ( VSCP_DAEMON_VARIABLE_CODE_TIME == nType ) {
        
        web_printf(conn, "<textarea cols=\"20\" rows=\"1\" name=\"value\">");
        
        if ( bNew ) {
            web_printf(conn, "");
        } 
        else {
            wxString str;
            variable.writeValueToString( str );
            web_printf( conn, (const char *)str.mbc_str() );
        }

        web_printf(conn, "</textarea>");
        
        web_printf(conn, "<div id=\"small\">Time. Format: "
                "'HH:MM:SS' ex. '13:05:01'.</div>" );
        
    }
    else if ( VSCP_DAEMON_VARIABLE_CODE_BLOB == nType ) {
        
        web_printf(conn, "<textarea cols=\"20\" rows=\"10\" name=\"value\">");
        
        if ( bNew ) {
            web_printf(conn, "");
        } 
        else {
            wxString str;
            variable.writeValueToString( str );
            web_printf( conn, (const char *)str.mbc_str() );
        }

        web_printf(conn, "</textarea>");
        
        web_printf(conn, "<div id=\"small\">Blob. Format: "
                "BASE64 encoded data.</div>" );
        
    }
    else if ( VSCP_DAEMON_VARIABLE_CODE_MIME == nType ) {
        web_printf(conn, "<textarea cols=\"20\" rows=\"1\" name=\"value\">");
        
        if ( bNew ) {
            web_printf(conn, "");
        } 
        else {
            wxString str;
            variable.writeValueToString( str );
            web_printf( conn, (const char *)str.mbc_str() );
        }

        web_printf(conn, "</textarea>");
        
        web_printf(conn, "<div id=\"small\">Mime. Format: "
                "mime-identifier;base64 encoded content'.</div>" );
        
    }
    else if ( VSCP_DAEMON_VARIABLE_CODE_HTML == nType ) {
        web_printf(conn, "<textarea cols=\"10\" rows=\"1\" name=\"value\">");
        
        if ( bNew ) {
            web_printf(conn, "");
        } 
        else {
            wxString str;
            variable.writeValueToString( str );
            web_printf( conn, (const char *)str.mbc_str() );
        }

        web_printf(conn, "</textarea>");
        
        web_printf(conn, "<div id=\"small\">HTML. Format: "
                "Any text string.</div>" );
        
    }
    else if ( VSCP_DAEMON_VARIABLE_CODE_JAVASCRIPT == nType ) {
        web_printf(conn, "<textarea cols=\"20\" rows=\"10\" name=\"value\">");
        
        if ( bNew ) {
            web_printf(conn, "");
        } 
        else {
            wxString str;
            variable.writeValueToString( str );
            web_printf( conn, (const char *)str.mbc_str() );
        }

        web_printf(conn, "</textarea>");
        
        web_printf(conn, "<div id=\"small\">JavaScript. Format: "
                "Any text string.</div>" );
        
    }
    else if ( VSCP_DAEMON_VARIABLE_CODE_JSON == nType ) {
        web_printf(conn, "<textarea cols=\"20\" rows=\"10\" name=\"value\">");
        
        if ( bNew ) {
            web_printf(conn, "");
        } 
        else {
            wxString str;
            variable.writeValueToString( str );
            web_printf( conn, (const char *)str.mbc_str() );
        }

        web_printf(conn, "</textarea>");
        
        web_printf(conn, "<div id=\"small\">JSON. Format: "
                "Any text string.</div>" );
        
    }
    else if ( VSCP_DAEMON_VARIABLE_CODE_XML == nType ) {
        web_printf(conn, "<textarea cols=\"20\" rows=\"10\" name=\"value\">");
        
        if ( bNew ) {
            web_printf(conn, "");
        } 
        else {
            wxString str;
            variable.writeValueToString( str );
            web_printf( conn, (const char *)str.mbc_str() );
        }

        web_printf(conn, "</textarea>");
        
        web_printf(conn, "<div id=\"small\">XML. Format: "
                "Any text string.</div>" );
        
    }
    else if ( VSCP_DAEMON_VARIABLE_CODE_SQL == nType ) {
        web_printf(conn, "<textarea cols=\"20\" rows=\"10\" name=\"value\">");
        
        if ( bNew ) {
            web_printf(conn, "");
        } 
        else {
            wxString str;
            variable.writeValueToString( str );
            web_printf( conn, (const char *)str.mbc_str() );
        }

        web_printf(conn, "</textarea>");
        
        web_printf(conn, "<div id=\"small\">SQL. Format: "
                "Any text string.</div>" );
        
    }
    else if ( VSCP_DAEMON_VARIABLE_CODE_LUA == nType ) {
        web_printf(conn, "<textarea cols=\"20\" rows=\"10\" name=\"value\">");
        
        if ( bNew ) {
            web_printf(conn, "");
        } 
        else {
            wxString str;
            variable.writeValueToString( str );
            web_printf( conn, (const char *)str.mbc_str() );
        }

        web_printf(conn, "</textarea>");
        
        web_printf(conn, "<div id=\"small\">Lua. Format: "
                "Any text string.</div>" );
    }
    else if ( VSCP_DAEMON_VARIABLE_CODE_LUA_RESULT == nType ) {
        web_printf(conn, "<textarea cols=\"20\" rows=\"10\" name=\"value\">");
        
        if ( bNew ) {
            web_printf(conn, "");
        } 
        else {
            wxString str;
            variable.writeValueToString( str );
            web_printf( conn, (const char *)str.mbc_str() );
        }

        web_printf(conn, "</textarea>");
        
        web_printf(conn, "<div id=\"small\">Lua result. Format: "
                "Any text string.</div>" );
    }
    else if ( VSCP_DAEMON_VARIABLE_CODE_UX_TYPE1 == nType ) {
        web_printf(conn, "<textarea cols=\"20\" rows=\"10\" name=\"value\">");
        
        if ( bNew ) {
            web_printf(conn, "");
        } 
        else {
            wxString str;
            variable.writeValueToString( str );
            web_printf( conn, (const char *)str.mbc_str() );
        }

        web_printf(conn, "</textarea>");
        
        web_printf(conn, "<div id=\"small\">UX1. Format: "
                "Any text string.</div>" );
    }
    else if ( VSCP_DAEMON_VARIABLE_CODE_DM_ROW == nType ) {
        web_printf(conn, "<textarea cols=\"20\" rows=\"1\" name=\"value\">");
        
        if ( bNew ) {
            web_printf(conn, "");
        } 
        else {
            wxString str;
            variable.writeValueToString( str );
            web_printf( conn, (const char *)str.mbc_str() );
        }

        web_printf(conn, "</textarea>");
        
        web_printf(conn, "<div id=\"small\">DM row. Format: "
                "'enabled,from,to,weekday,time,mask,filter,index,zone,sub-zone,control-code,action-code,action-param,comment'.</div>" );
    }
    else if ( VSCP_DAEMON_VARIABLE_CODE_DRIVER == nType ) {
        web_printf(conn, "<textarea cols=\"20\" rows=\"1\" name=\"value\">");
        
        if ( bNew ) {
            web_printf(conn, "");
        } 
        else {
            wxString str;
            variable.writeValueToString( str );
            web_printf( conn, (const char *)str.mbc_str() );
        }

        web_printf(conn, "</textarea>");
        
        web_printf(conn, "<div id=\"small\">Driver. Format: "
                "Driver record string.</div>" );
        
    }
    else if ( VSCP_DAEMON_VARIABLE_CODE_USER == nType ) {
        web_printf(conn, "<textarea cols=\"20\" rows=\"1\" name=\"value\">");
        
        if ( bNew ) {
            web_printf(conn, "");
        } 
        else {
            wxString str;
            variable.writeValueToString( str );
            web_printf( conn, (const char *)str.mbc_str() );
        }

        web_printf(conn, "</textarea>");
        
        web_printf(conn, "<div id=\"small\">User record. Format: "
                "'name;password;fullname;filtermask;rights;remotes;events;note'.</div>" );
    }
    else if ( VSCP_DAEMON_VARIABLE_CODE_FILTER == nType ) {
        web_printf(conn, "<textarea cols=\"20\" rows=\"1\" name=\"value\">");
        
        if ( bNew ) {
            web_printf(conn, "");
        } 
        else {
            wxString str;
            variable.writeValueToString( str );
            web_printf( conn, (const char *)str.mbc_str() );
        }

        web_printf(conn, "</textarea>");
        
        web_printf(conn, "<div id=\"small\">VSCP Filter. Format: "
                "'filter-priority, filter-class, filter-type, filter-GUID'.</div>" );
    }    
    else {
        // Invalid type
        web_printf(conn, "Invalid type - Something is very wrong!");
    }

    // Persistence
    web_printf(conn, "</tr><tr><td style=\"font-weight: bold;\">Persistence: </td><td>");

    if ( bNew ) {
        
        web_printf(conn, "<input type=\"radio\" name=\"persistent\" value=\"true\" ");

        if ( !bNew ) {
            web_printf( conn,
                        "%s",
                        variable.isPersistent() ?
                        "checked >Persistent " : ">Persistent ");
        } 
        else {
            web_printf (conn, "checked >Persistent ");
        }
        
        web_printf( conn, "<input type=\"radio\" name=\"persistent\" value=\"false\" ");

        if ( !bNew ) {
            web_printf( conn,
                    "%s",
                    !variable.isPersistent() ?
                    "checked >Non persistent " : ">Non persistent ");
        } 
        else {
            web_printf( conn, ">Non persistent " );
        }
        
        
    }
    else {
        web_printf( conn,
                        "%s",
                        variable.isPersistent() ?
                        "Persistent " : "Non Persistent ");
    }

    web_printf(conn, "</td></tr>");

    // Owner
    web_printf(conn, "</tr><tr><td style=\"font-weight: bold;\">Owner: </td><td>");

    if ( bNew ) {
        web_printf(conn, "<textarea cols=\"20\" rows=\"1\" name=\"owner\">");
        web_printf(conn, "</textarea>");
    }
    else {
        web_printf(conn, "id=%X ", variable.getOwnerID() );
        CUserItem *pUser = gpobj->m_userList.getUserItemFromOrdinal( variable.getOwnerID() );
        if ( NULL == pUser ) {
            web_printf(conn, " Unknow user " );
        }
        web_printf(conn, 
                    "%s (%s)",
                    (const char *)pUser->getUser().mbc_str(),
                    (const char *)pUser->getFullname().mbc_str() );        
    }
    
    web_printf(conn, "</td></tr>");
    
    // Access rights
    web_printf(conn, "</tr><tr><td style=\"font-weight: bold;\">Rights: </td><td>");

    if ( bNew ) {
        web_printf(conn, "<textarea cols=\"20\" rows=\"1\" name=\"rights\">");
        web_printf(conn, "</textarea>");
    }
    else {
        wxString str;
        CVSCPVariable::makeAccessTightString( variable.getAccessRights(), str );
        web_printf( conn, 
                        "0x%03X %s (owner group other)", 
                        variable.getAccessRights(), 
                        (const char *)str.mbc_str() );      
    }
    
    web_printf(conn, "</td></tr>");
    
    // Last change
    web_printf(conn, "</tr><tr><td style=\"font-weight: bold;\">Last change: </td><td>");

    if ( bNew ) {
        web_printf( conn, 
                        "%s", 
                        (const char *)wxDateTime::Now().FormatISOCombined().mbc_str() );        
    }
    else {
        web_printf( conn, 
                        "%s", 
                        (const char *)variable.getLastChange().FormatISOCombined().mbc_str() );
    }

    web_printf(conn, "</td></tr>");
    
    
    
    // Note
    web_printf( conn, "</tr><tr><td style=\"font-weight: bold;\">Note: </td><td>");
    web_printf( conn, "<textarea cols=\"50\" rows=\"5\" name=\"note\">");
    
    if ( bNew ) {
        web_printf(conn, "");
    } 
    else { 
        wxString str;
        variable.getNote( str, true );
        web_printf(conn, str.mbc_str());
    }

    web_printf(conn, "</textarea>");

    web_printf(conn, "</td></tr></table>");

    web_printf(conn, WEB_VAREDIT_TABLE_END);
 
    web_printf( conn, 
                    WEB_VAREDIT_SUBMIT,  
                    "/vscp/varpost",
                    "/vscp" );

    web_printf( conn, "</form>");
    web_printf( conn, WEB_COMMON_END);     // Common end code

    return WEB_OK;
}


///////////////////////////////////////////////////////////////////////////////
// vscp_variable_post
//

static int vscp_variable_post( struct web_connection *conn, void *cbdata )
{
    char buf[32000];
    wxString str;
    VSCPInformation vscpinfo;
    CVSCPVariable variable;

    // Check pointer
    if (NULL == conn) return 0;
    
    const struct web_request_info *reqinfo =  
                web_get_request_info( conn );
    if ( NULL == reqinfo ) return 0;

    // id
    long id = -1;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "id", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            id = atoi( buf );
        }
    }

    // From
    long nFrom = 0;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "from", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            nFrom = atoi( buf );
        }
    }

    // Count
    uint16_t nCount = 50;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "count", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            nCount = atoi( buf );
        }
    }

    uint8_t nType = VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "type", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            nType = atoi( buf );
        }
    }

    // Name
    wxString strName;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "value_name", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            strName = wxString::FromAscii( buf );
        }
    }

    // bNew
    bool bNew = false;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "new", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            if ( NULL != strstr( "true", buf ) ) bNew = true;
        }
    }

    // Flag for persistence
    bool bPersistent = true;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "persistent", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            if ( NULL != strstr( "false", buf ) ) bPersistent = false;
        }
    }

    // Note
    wxString strNote;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "note", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            strNote = wxString::FromUTF8( buf );
        }
    }

    // Value
    wxString strValueString;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "value", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            strValueString = wxString::FromAscii( buf );
        }
    }

    bool bValueBoolean = false;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "value_boolean", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            if ( NULL != strstr( "true", buf ) ) bValueBoolean = true;
        }
    }

    int value_integer = 0;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "value_integer", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            str = wxString::FromAscii( buf );
            value_integer = vscp_readStringValue( str );
        }
    }

    long value_long = 0;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "value_long", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            str = wxString::FromAscii( buf );
            value_long = vscp_readStringValue( str );
        }
    }

    double value_double = 0;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "value_double", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            value_double = atof( buf );
        }
    }

    wxString strMeasurement;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "value_measurement", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            strMeasurement = wxString::FromAscii( buf );
        }
    }

    uint16_t value_class = 0;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "value_class", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            value_class = vscp_readStringValue( wxString::FromAscii( buf ) );
        }
    }

    uint16_t value_type = 0;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "value_type", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            value_type = vscp_readStringValue( wxString::FromAscii( buf ) );
        }
    }

    wxString strGUID;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "value_guid", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            strGUID = wxString::FromAscii( buf );
            strGUID.Trim();
            if ( 0 == strGUID.Length() ) {
                strGUID = _("-");
            }
        }
    }

    uint32_t value_timestamp = 0;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "value_timestamp", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            value_timestamp = vscp_readStringValue( wxString::FromAscii( buf ) );
        }
    }

    uint32_t value_obid = 0;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "value_obid", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            value_obid = vscp_readStringValue( wxString::FromAscii( buf ) );
        }
    }

    uint8_t value_head = 0;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "value_head", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            value_head = vscp_readStringValue( wxString::FromAscii( buf ) );
        }
    }

    uint32_t value_crc = 0;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "value_crc", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            value_crc = vscp_readStringValue( wxString::FromAscii( buf ) );
        }
    }

    uint16_t value_sizedata = 0;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "value_sizedata", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            value_sizedata = vscp_readStringValue( wxString::FromAscii( buf ) );
        }
    }

    wxString strData;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "value_data", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            strData = wxString::FromAscii( buf );
        }
    }

    wxString strDateTime;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "value_date_time", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            strDateTime = wxString::FromAscii( buf );
        }
    }

    web_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");

    web_printf( conn, 
                    WEB_COMMON_HEAD, 
                    "VSCP - Variable Post" );
    web_printf( conn, WEB_STYLE_START);
    web_printf( conn, WEB_COMMON_CSS);     // CSS style Code
    web_printf( conn, WEB_STYLE_END);
    web_printf( conn, WEB_COMMON_JS);      // Common Javascript code
    web_printf( conn, "<meta http-equiv=\"refresh\" content=\"1;url=/vscp/variables");
    web_printf( conn, 
                    "?from=%d&count=%d", 
                    nFrom, 
                    nCount );
    web_printf( conn, "\">");
    web_printf( conn, WEB_COMMON_HEAD_END_BODY_START);

    // Navigation menu
    web_printf( conn, WEB_COMMON_MENU);

    web_printf( conn, WEB_VARPOST_BODY_START);

    if (bNew) {
        //pVariable = new CVSCPVariable;
    }

    /* TODO
    if (bNew || (id >= 0)) {

        if (bNew ||
                ((0 == id) && !bNew) ||
                (id < (long)gpobj->m_VSCP_Variables.m_listVariable.GetCount()) ) {

            //if (!bNew) gpobj->m_VSCP_Variables.m_listVariable.Item(id)->GetData();

            if ( 0 ) {

                // Set the type
                variable.setPersistent( bPersistent );
                variable.setType( nType );
                variable.setNote( strNote );
                variable.setName( strName );

                switch ( nType ) {

                case VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED:
                    web_printf( conn, "Error: Variable code is unassigned.<br>");
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_STRING:
                    variable.setValue( strValueString );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_BOOLEAN:
                    variable.setValue( bValueBoolean );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_INTEGER:
                    variable.setValue( value_integer );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_LONG:
                    variable.setValue( value_long );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_DOUBLE:
                    variable.setValue( value_double );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_VSCP_MEASUREMENT:
                    uint16_t size;
                    vscp_setVscpDataArrayFromString( variable.m_normInteger,
                                                        &size,
                                                        strMeasurement );
                    variable.m_normIntSize = size;
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT:
                    variable.m_event.vscp_class = value_class;
                    variable.m_event.vscp_type = value_type;
                    vscp_getGuidFromStringToArray( variable.m_event.GUID, strGUID );
                    vscp_setVscpDataFromString( &variable.m_event,
                                                    strData );
                    variable.m_event.crc = value_crc;
                    variable.m_event.head = value_head;
                    variable.m_event.obid = value_obid;
                    variable.m_event.timestamp = value_timestamp;
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID:
                    vscp_getGuidFromStringToArray( variable.m_event.GUID, strGUID );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_DATA:
                    vscp_setVscpDataFromString( &variable.m_event,
                                                    strData );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_CLASS:
                    variable.m_event.vscp_class = value_class;
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TYPE:
                    variable.m_event.vscp_type = value_type;
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TIMESTAMP:
                    variable.m_event.timestamp = value_timestamp;
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_DATETIME:
                    variable.m_timestamp.ParseDateTime( strDateTime );
                    break;

                default:
                    web_printf( conn, "Error: Variable code is unknown.<br>");
                    break;

                }

                // If new variable add it
                if ( bNew ) {
                    //gpobj->m_VSCP_Variables.add( pVariable );
                }

                // Save variables
                gpobj->m_VSCP_Variables.save();

                web_printf( conn, "<br><br>Variable has been saved. id=%d", id);

            }
            else {
                web_printf( conn, "<br><br>Memory problem id=%d. Unable to save record", id);
            }

        }
        else {
            web_printf( conn, "<br><br>Record id=%d is to large. Unable to save record", id);
        }
    }
    else {
        web_printf( conn, "<br><br>Record id=%d is wrong. Unable to save record", id);
    }
    */

    web_printf( conn, WEB_COMMON_END); // Common end code

    return 1;
}


///////////////////////////////////////////////////////////////////////////////
// vscp_variable_new
//

static int vscp_variable_new( struct web_connection *conn, void *cbdata )
{
    wxString str;
    char buf[80];
    VSCPInformation vscpinfo;

    // Check pointer
    if (NULL == conn) return 0;

    const struct web_request_info *reqinfo =  
                web_get_request_info( conn );
    if ( NULL == reqinfo ) return 0;
    
    // Count
    uint16_t nCount = 50;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "count", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            nCount = atoi( buf );
        }
    }

    web_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");

    web_printf( conn, 
                    WEB_COMMON_HEAD, 
                    "VSCP - New variable" );
    web_printf( conn, WEB_STYLE_START );
    web_printf( conn, WEB_COMMON_CSS );     // CSS style Code
    web_printf( conn, WEB_STYLE_END );
    web_printf( conn, WEB_COMMON_JS );      // Common Javascript code
    web_printf( conn, WEB_COMMON_HEAD_END_BODY_START );

    // Navigation menu
    web_printf( conn, WEB_COMMON_MENU );

    web_printf( conn, WEB_VAREDIT_BODY_START);

    web_printf( conn, "<br><div style=\"text-align:center\">");

    web_printf( conn, "<br><form method=\"get\" action=\"");
    web_printf( conn, "/vscp/varedit");
    web_printf( conn, "\" name=\"varnewstep1\">");

    long nFrom;
    /* TODO
    if ( gpobj->m_VSCP_Variables.m_listVariable.GetCount() % nCount ) {
        nFrom = (gpobj->m_VSCP_Variables.m_listVariable.GetCount()/nCount)*nCount;
    }
    else {
        nFrom = ((gpobj->m_VSCP_Variables.m_listVariable.GetCount()/nCount) - 1)*nCount;
    }
     */

    web_printf( conn, 
                    "<input name=\"from\" value=\"%d\" type=\"hidden\">", 
                    nFrom );
    web_printf( conn, 
                    "<input name=\"count\" value=\"%d\" type=\"hidden\">", 
                    nCount );

    web_printf( conn, "<input name=\"new\" value=\"true\" type=\"hidden\">");

    web_printf( conn, "<select name=\"type\">");
    web_printf( conn, "<option value=\"1\">String value</option>");
    web_printf( conn, "<option value=\"2\">Boolean value</option>");
    web_printf( conn, "<option value=\"3\">Integer value</option>");
    web_printf( conn, "<option value=\"4\">Long value</option>");
    web_printf( conn, "<option value=\"5\">Floating point value</option>");
    web_printf( conn, "<option value=\"6\">VSCP data coding</option>");
    web_printf( conn, "<option value=\"7\">VSCP event (Level II)</option>");
    web_printf( conn, "<option value=\"8\">VSCP event GUID</option>");
    web_printf( conn, "<option value=\"9\">VSCP event data</option>");
    web_printf( conn, "<option value=\"10\">VSCP event class</option>");
    web_printf( conn, "<option value=\"11\">VSCP event type</option>");
    web_printf( conn, "<option value=\"12\">VSCP event timestamp</option>");
    web_printf( conn, "<option value=\"13\">Date + Time in iso format</option>");
    web_printf( conn, "</select>");

    web_printf( conn, "<br></div>");
    web_printf( conn, WEB_VARNEW_SUBMIT);
    //wxString wxstrurl = wxString::Format(_("%s/vscp/varedit?new=true"),
    //                                            strHost.mbc_str() );
    //web_printf( conn, WEB_VARNEW_SUBMIT,
    //                  wxstrurl.mbc_str() );

    web_printf( conn, "</form>");

    web_printf( conn, WEB_COMMON_END); // Common end code

    return 1;
}


///////////////////////////////////////////////////////////////////////////////
// vscp_variable_delete
//

static int
vscp_variable_delete( struct web_connection *conn, void *cbdata )
{
    char buf[80];
    wxString str;
    VSCPInformation vscpinfo;
    CVSCPVariable *pVariable = NULL;

    // Check pointer
    if (NULL == conn) return 0;
    
    const struct web_request_info *reqinfo =  
                web_get_request_info( conn );
    if ( NULL == reqinfo ) return 0;

    // id
    long id = -1;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "id", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            id = atoi( buf );
        }
    }

    // From
    long nFrom = 0;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "from", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            nFrom = atoi( buf );
        }
    }

    // Count
    uint16_t nCount = 50;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                            
                            strlen( reqinfo->query_string ), 
                            "count", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            nCount = atoi( buf );
        }
    }

    web_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");
    
    web_printf( conn, 
                    WEB_COMMON_HEAD, 
                    "VSCP - Variable Delete" );
    web_printf( conn, WEB_STYLE_START );
    web_printf( conn, WEB_COMMON_CSS );     // CSS style Code
    web_printf( conn, WEB_STYLE_END );
    web_printf( conn, WEB_COMMON_JS );      // Common Javascript code
    web_printf( conn, "<meta http-equiv=\"refresh\" content=\"2;url=/vscp/variables" ) ;
    web_printf( conn, 
                    "?from=%d&count=%d", 
                    nFrom, 
                    nCount );
    web_printf( conn, "\">" );
    web_printf( conn, WEB_COMMON_HEAD_END_BODY_START );

    // navigation menu
    web_printf( conn, WEB_COMMON_MENU);

    web_printf( conn, WEB_VAREDIT_BODY_START);
/* TODO
    pVariable = gpobj->m_VSCP_Variables.m_listVariable.Item(id)->GetData();
    //wxlistVscpVariableNode *node =
    //        gpobj->m_VSCP_Variables.m_listVariable.Item( id );
    //if ( gpobj->m_VSCP_Variables.m_listVariable.DeleteNode( node )  ) {//
    if ( gpobj->m_VSCP_Variables.remove( variable ) )  {
        web_printf( conn, "<br>Deleted record id = %d", id);
        // Save variables
        gpobj->m_VSCP_Variables.save();
    }
    else {
        web_printf( conn, "<br>Failed to remove record id = %d", id);
    }
*/
    web_printf( conn, WEB_COMMON_END);     // Common end code


    return 1;
}


///////////////////////////////////////////////////////////////////////////////
// vscp_discovery
//

static int
vscp_discovery( struct web_connection *conn, void *cbdata  )
{
    //char buf[80];
    wxString str;
    VSCPInformation vscpinfo;

    // Check pointer
    if (NULL == conn) return 0;

    web_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");
    
    web_printf( conn, 
                    WEB_COMMON_HEAD, 
                    "VSCP - Device discovery" );
    web_printf( conn, WEB_STYLE_START);
    web_printf( conn, WEB_COMMON_CSS);     // CSS style Code
    web_printf( conn, WEB_STYLE_END);
    web_printf( conn, WEB_COMMON_JS);      // Common Javascript code
    web_printf( conn, "<meta http-equiv=\"refresh\" content=\"5;url=/vscp");
    web_printf( conn, "\">");
    web_printf( conn, WEB_COMMON_HEAD_END_BODY_START);

    // navigation menu
    web_printf( conn, WEB_COMMON_MENU);
    web_printf( conn, "<b>Device discovery functionality is not yet implemented!</b>");

    return 1;
}


///////////////////////////////////////////////////////////////////////////////
// vscp_client
//

static int
vscp_client( struct web_connection *conn, void *cbdata )
{
    //char buf[80];
    wxString str;
    VSCPInformation vscpinfo;

    // Check pointer
    if (NULL == conn) return 0;

    web_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");
    
    web_printf( conn, 
                    WEB_COMMON_HEAD, 
                    "VSCP - Session" );
    web_printf( conn, WEB_STYLE_START);
    web_printf( conn, WEB_COMMON_CSS);     // CSS style Code
    web_printf( conn, WEB_STYLE_END);
    web_printf( conn, WEB_COMMON_JS);      // Common Javascript code
    web_printf( conn, "<meta http-equiv=\"refresh\" content=\"5;url=/vscp");
    web_printf( conn, "\">");
    web_printf( conn, WEB_COMMON_HEAD_END_BODY_START);

    // navigation menu
    web_printf( conn, WEB_COMMON_MENU);
    web_printf( conn, "<b>Client functionality is not yet implemented!</b>");

    return 1;
}


///////////////////////////////////////////////////////////////////////////////
// configure_list
//

static int
vscp_configure( struct web_connection *conn, void *cbdata )
{
    wxString str;
    VSCPInformation vscpinfo;

    // Check pointer
    if (NULL == conn) return 0;

    web_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");
    
    web_printf( conn, 
                    WEB_COMMON_HEAD, 
                    "VSCP - Configuration" );
    web_printf( conn, WEB_STYLE_START);
    web_printf( conn, WEB_COMMON_CSS);     // CSS style Code
    web_printf( conn, WEB_STYLE_END);
    web_printf( conn, WEB_COMMON_JS);      // Common Javascript code
    web_printf( conn, WEB_COMMON_HEAD_END_BODY_START);

    // navigation menu
    web_printf( conn, WEB_COMMON_MENU);

    web_printf( conn, "<br><br><br>");

    web_printf( conn, "<b>VSCP Daemon version:</b> ");
    web_printf( conn, VSCPD_DISPLAY_VERSION);
    web_printf( conn, "<br>");

    web_printf( conn, "<b>Operating system:</b> ");
    web_printf( conn, wxGetOsDescription().mbc_str() );
    if ( wxIsPlatform64Bit() ) {
        web_printf( conn, " 64-bit ");
    }
    else {
        web_printf( conn, " 32-bit ");
    }
    if ( wxIsPlatformLittleEndian() ) {
        web_printf( conn, " Little endian ");
    }
    else {
        web_printf( conn, " Big endian ");
    }
    web_printf( conn, "<br>");


#ifndef WIN32
/*
    wxLinuxDistributionInfo info = wxGetLinuxDistributionInfo();
    web_printf( conn, "<span style=\"color: blue;\"><b>Linux distribution Info:</b></span><br>");
    web_printf( conn, "&nbsp;&nbsp;<b>id=:</b> ");
    web_printf( conn, info.id;
    web_printf( conn, "&nbsp;&nbsp;<span style=\"color: blue;\"><b>release=:</b></span><br>");
    web_printf( conn, info.Release;
    web_printf( conn, "&nbsp;&nbsp;<span style=\"color: blue;\"><b>codeName=:</b></span><br>");
    web_printf( conn, info.Codename;
    web_printf( conn, "&nbsp;&nbsp;<span style=\"color: blue;\"><b>description=:</b></span><br>");
    web_printf( conn, info.Description;
    web_printf( conn, "<br>");
 */
#endif

    wxMemorySize memsize;
    if ( -1 != ( memsize = wxGetFreeMemory() ) ) {
        web_printf( conn, "<b>Free memory:</b> ");
        web_printf( conn, memsize.ToString().mbc_str() );
        web_printf( conn, " bytes<br>");
    }

    web_printf( conn, "<b>Hostname:</b> ");
    web_printf( conn, wxGetFullHostName().mbc_str() );
    web_printf( conn, "<br>");

    web_printf( conn, "<hr>");

    web_printf( conn, "<b>wxWidgets version:</b> ");
    web_printf( conn, 
                    "%d.%d.%d.%d" " Copyright (c) 1998-2005 Julian Smart, "
                    "Robert Roebling et al" ,
                    wxMAJOR_VERSION,
                    wxMINOR_VERSION,
                    wxRELEASE_NUMBER,
                    wxSUBRELEASE_NUMBER );
    web_printf( conn, "<br>");

    web_printf( conn, "<b>Mongoose version:</b> ");
    web_printf( conn, wxString::FromUTF8( MG_VERSION 
                                                " Copyright (c) 2013-2017 Cesanta Software Limited" ).mbc_str() );
    web_printf( conn, "<br>");

    web_printf( conn, "<hr>");

    // Debuglevel
    web_printf( conn, "<b>Debuglevel:</b> ");
    web_printf( conn, 
                    "%d ", 
                    gpobj->m_logLevel );
    switch ( gpobj->m_logLevel  ) {
        case DAEMON_LOGMSG_NONE:
            web_printf( conn, "(none)");
            break;
        case DAEMON_LOGMSG_DEBUG:
            web_printf( conn, "(debug)");
            break;
        case DAEMON_LOGMSG_NORMAL:
            web_printf( conn, "(info)");
            break;
        default:
            web_printf( conn, "(unkown)");
            break;
    }

    // Server GUID
    web_printf( conn, "<b>Server GUID:</b> ");
    gpobj->m_guid.toString( str );
    web_printf( conn, str.mbc_str() );

    // Client buffer size
    web_printf( conn, "<b>Client buffer size:</b> ");
    web_printf( conn, 
                    "%d", 
                    gpobj->m_maxItemsInClientReceiveQueue );

    web_printf( conn, "<hr>");

    // TCP/IP interface
    web_printf( conn, "<b>TCP/IP interface:</b> ");
    web_printf( conn, "enabled on <b>interface:</b> '");
    web_printf( conn, gpobj->m_strTcpInterfaceAddress.mbc_str() );
    web_printf( conn, "'");
    web_printf( conn, "<br>");

    web_printf( conn, "<hr>");

    // UDP interface
    web_printf( conn, "<b>UDP interface:</b> ");
    if ( gpobj->m_udpInfo.m_bEnable ) {
        web_printf( conn, "enabled on <b>interface:</b> '");
        web_printf( conn, gpobj->m_udpInfo.m_interface.mbc_str() );
        web_printf( conn, "'");
    }
    else {
        web_printf( conn, "disabled");
    }
    web_printf( conn, "<br>");

    web_printf( conn, "<hr>");


    // VSCP Internal automation intelligence
    web_printf( conn, "<b>VSCP internal event logic:</b> ");
    web_printf( conn, "enabled.");
    web_printf( conn, "<br>");
    web_printf( conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Automation:</b> ");
    if ( gpobj->m_automation.isAutomationEnabled() ) {
        web_printf( conn, "enabled.<br>");
    }
    else {
        web_printf( conn, "disabled<br>");
    }



    if ( gpobj->m_automation.isSendHeartbeat() ) {
        web_printf( conn,  "&nbsp;&nbsp;&nbsp;&nbsp;Heartbeat will be sent.<br>" );
    }
    else {
        web_printf( conn,  "&nbsp;&nbsp;&nbsp;&nbsp;Heartbeat will be sent<br>" );
    }

    web_printf( conn,  "&nbsp;&nbsp;&nbsp;&nbsp;<b>Period for heartbeat</b>: " );
    web_printf( conn, 
                    "%ld seconds<br>", 
                    gpobj->m_automation.getIntervalHeartbeat() ); 

    web_printf( conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Heartbeat last sent @</b>: " );
    web_printf( conn, gpobj->m_automation.getHeartbeatSent().FormatISODate().mbc_str() + _( " " ) );
    web_printf( conn, gpobj->m_automation.getHeartbeatSent().FormatISOTime().mbc_str() + _( "<br>" ) );



    if ( gpobj->m_automation.isSendSegmentControllerHeartbeat() ) {
        web_printf( conn, "&nbsp;&nbsp;&nbsp;&nbsp;Segment controller heartbeat will be sent.<br>" );
    }
    else {
        web_printf( conn, "&nbsp;&nbsp;&nbsp;&nbsp;Segment controller heartbeat will be sent<br>" );
    }

    web_printf( conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Period for Segment controller heartbeat</b>: " );
    web_printf( conn, 
                    "%ld seconds<br>", 
            gpobj->m_automation.getIntervalSegmentControllerHeartbeat() );

    web_printf( conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Segment controller heartbeat last sent @</b>: " );
    web_printf( conn, 
        gpobj->m_automation.getSegmentControllerHeartbeatSent().FormatISODate().mbc_str() +
        _( " " ) );
    web_printf( conn, 
        gpobj->m_automation.getSegmentControllerHeartbeatSent().FormatISOTime().mbc_str() + 
        _( "<br>" ) );



    web_printf( conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Longitude</b>: " );
    web_printf( conn, 
                    "%f<br>", 
                    gpobj->m_automation.getLongitude() );

    web_printf( conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Latitude</b>: " );
    web_printf( conn, 
                    "%f<br>", 
                    gpobj->m_automation.getLatitude()  );

    int hours, minutes;
    gpobj->m_automation.convert2HourMinute( gpobj->m_automation.getDayLength(), &hours, &minutes );
    web_printf( conn,  "&nbsp;&nbsp;&nbsp;&nbsp;<b>Length of day</b>: " );
    web_printf( conn, 
                    "%02d:%02d<br>", 
                    hours, 
                    minutes );

    web_printf( conn,  "&nbsp;&nbsp;&nbsp;&nbsp;<b>Declination</b>: " );
    web_printf( conn, 
                    "%f<br>", 
                    gpobj->m_automation.getDeclination() );

    web_printf( conn,  "&nbsp;&nbsp;&nbsp;&nbsp;<b>Sun max altitude</b>: " );
    web_printf( conn, 
                    "%f<br>", 
                    gpobj->m_automation.getSunMaxAltitude() );

    web_printf( conn,  "&nbsp;&nbsp;&nbsp;&nbsp;<b>Last Calculation</b>: " );
    web_printf( conn, 
                gpobj->m_automation.getLastCalculation().FormatISODate().mbc_str()  + 
                _( " " ) );
    web_printf( conn, 
                gpobj->m_automation.getLastCalculation().FormatISOTime().mbc_str() + 
                _("<br>") );

    if ( gpobj->m_automation.isSendSunriseTwilightEvent() ) {
        web_printf( conn,  "&nbsp;&nbsp;&nbsp;&nbsp;Sunrise twilight event will be sent.<br>" );
    }
    else {
        web_printf( conn,  "&nbsp;&nbsp;&nbsp;&nbsp;Sunrise twilight event will not be sent.<br>" );
    }

    web_printf( conn,  "&nbsp;&nbsp;&nbsp;&nbsp;<b>Civil Twilight Sunrise Time</b>: " );
    web_printf( conn, 
            gpobj->m_automation.getCivilTwilightSunriseTime().FormatISOTime().mbc_str() + _
            ( "<br>" ) );

    web_printf( conn,  "&nbsp;&nbsp;&nbsp;&nbsp;<b>Civil Twilight Sunrise Time sent @</b>: " );
    web_printf( conn, 
        gpobj->m_automation.getCivilTwilightSunriseTimeSent().FormatISODate().mbc_str() + 
        _( " " ) );
    web_printf( conn, 
        gpobj->m_automation.getCivilTwilightSunriseTimeSent().FormatISOTime().mbc_str() + 
        _( "<br>" ) );



    if ( gpobj->m_automation.isSendSunriseEvent() ) {
        web_printf( conn,  "&nbsp;&nbsp;&nbsp;&nbsp;Sunrise event will be sent.<br>" );
    }
    else {
        web_printf( conn,  "&nbsp;&nbsp;&nbsp;&nbsp;Sunrise event will not be sent.<br>" );
    }

    web_printf( conn,  "&nbsp;&nbsp;&nbsp;&nbsp;<b>Civil Sunrise Time</b>: " );
    web_printf( conn, 
        gpobj->m_automation.getSunriseTime().FormatISOTime().mbc_str() + 
       _( "<br>" ) );

    web_printf( conn,  "&nbsp;&nbsp;&nbsp;&nbsp;<b>Civil Sunrise Time sent @</b>: " );
    web_printf( conn, 
        gpobj->m_automation.getSunriseTimeSent().FormatISODate().mbc_str() + 
        _( " " ) );
    web_printf( conn, 
        gpobj->m_automation.getSunriseTimeSent().FormatISOTime().mbc_str() + 
        _( "<br>" ) );


    if ( gpobj->m_automation.isSendSunsetEvent() ) {
        web_printf( conn,  
            "&nbsp;&nbsp;&nbsp;&nbsp;Sunset event will be sent.<br>" );
    }
    else {
        web_printf( conn,  
            "&nbsp;&nbsp;&nbsp;&nbsp;Sunset event will not be sent.<br>" );
    }

    web_printf( conn,  
        "&nbsp;&nbsp;&nbsp;&nbsp;<b>Civil Sunset Time</b>: " );
    web_printf( conn, 
        gpobj->m_automation.getSunsetTime().FormatISOTime().mbc_str() + _( "<br>" ) );

    web_printf( conn,  
        "&nbsp;&nbsp;&nbsp;&nbsp;<b>Civil Sunset Time sent @</b>: " );
    web_printf( conn, 
        gpobj->m_automation.getSunsetTimeSent().FormatISODate().mbc_str() + _( " " ) );
    web_printf( conn, 
        gpobj->m_automation.getSunsetTimeSent().FormatISOTime().mbc_str() + _( "<br>" ) );



    if ( gpobj->m_automation.isSendSunsetTwilightEvent() ) {
        web_printf( conn,  "&nbsp;&nbsp;&nbsp;&nbsp;Twilight sunset event will be sent.<br>" );
    }
    else {
        web_printf( conn,  "&nbsp;&nbsp;&nbsp;&nbsp;Twilight sunset will not be sent.<br>" );
    }

    web_printf( conn,  "&nbsp;&nbsp;&nbsp;&nbsp;<b>Civil Twilight Sunset Time</b>: " );
    web_printf( conn, 
        gpobj->m_automation.getCivilTwilightSunsetTime().FormatISOTime().mbc_str() + 
        _( "<br>" ) );

    web_printf( conn,  "&nbsp;&nbsp;&nbsp;&nbsp;<b>Civil Twilight Sunset Time sent @</b>: " );
    web_printf( conn, 
        gpobj->m_automation.getCivilTwilightSunsetTimeSent().FormatISODate().mbc_str() + 
        _( " " ) );
    web_printf( conn, 
        gpobj->m_automation.getCivilTwilightSunsetTimeSent().FormatISOTime().mbc_str() + 
        _( "<br>" ) );


    if ( gpobj->m_automation.isSendCalculatedNoonEvent() ) {
        web_printf( conn,  
                "&nbsp;&nbsp;&nbsp;&nbsp;Calculated noon event will be sent.<br>" );
    }
    else {
        web_printf( conn,  
                "&nbsp;&nbsp;&nbsp;&nbsp;Calculated noon will not be sent.<br>" );
    }

    web_printf( conn,  "&nbsp;&nbsp;&nbsp;&nbsp;<b>Calculated Noon Time</b>: " );
    web_printf( conn, 
        gpobj->m_automation.getNoonTime().FormatISOTime().mbc_str() + 
        _( "<br>" ) );

    web_printf( conn,  
        "&nbsp;&nbsp;&nbsp;&nbsp;<b>Calculated Noon Time sent @</b>: " );
    web_printf( conn, 
        gpobj->m_automation.getNoonTimeSent().FormatISODate().mbc_str() + 
        _(" ") );
    web_printf( conn, 
        gpobj->m_automation.getNoonTimeSent().FormatISOTime().mbc_str() + 
        _("<br>") );


    web_printf( conn, "<hr>");

    // Web server interface
    web_printf( conn, "<b>Web server <b>interface:</b></b> ");

    web_printf( conn, "enabled on interface '");
    web_printf( conn, gpobj->m_strWebServerInterfaceAddress.mbc_str());
    web_printf( conn,  "<br>&nbsp;&nbsp;&nbsp;&nbsp;<b>Autentication:</b> " );
    if ( gpobj->m_bDisableSecurityWebServer ) {
        web_printf( conn,  "turned off." );
    }
    else {
        web_printf( conn,  "turned on." );
    }
    web_printf( conn, "<br>");
    web_printf( conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Rootfolder:</b> ");
    web_printf( conn, wxString::FromUTF8( gpobj->m_pathWebRoot ).mbc_str() );
    web_printf( conn, "<br>");
    web_printf( conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Authdomain:</b> ");
    web_printf( conn, wxString::FromUTF8( gpobj->m_authDomain ).mbc_str() );
    if ( 0 == strlen( gpobj->m_authDomain ) ) {
        web_printf( conn, "Set to default.");
    }
    web_printf( conn, "<br>");
    web_printf( conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Path certs:</b> ");
    web_printf( conn, wxString::FromUTF8( gpobj->m_pathCert ).mbc_str() );
    web_printf( conn, "<br>");
    web_printf( conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>ExtraMimeTypes:</b> ");
    web_printf( conn, wxString::FromUTF8( gpobj->m_extraMimeTypes ).mbc_str() );
    if ( 0 == strlen( gpobj->m_extraMimeTypes ) ) {
        web_printf( conn, "Set to default.");
    }
    web_printf( conn, "<br>");
    web_printf( conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>CgiInterpreter:</b> ");
    web_printf( conn, wxString::FromUTF8( gpobj->m_cgiInterpreter ).mbc_str() );
    if ( 0 == strlen( gpobj->m_cgiInterpreter ) ) {
        web_printf( conn, "Set to default.");
    }
    web_printf( conn, "<br>");
    web_printf( conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>CgiPattern:</b> ");
    web_printf( conn, wxString::FromUTF8( gpobj->m_cgiPattern ).mbc_str() );
    if ( 0 == strlen( gpobj->m_cgiPattern ) ) {
        web_printf( conn, "Set to default.");
    }
    web_printf( conn, "<br>");
    web_printf( conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>HiddenFilePatterns:</b> ");
    web_printf( conn, wxString::FromUTF8( gpobj->m_hideFilePatterns ).mbc_str() );
    if ( 0 == strlen( gpobj->m_hideFilePatterns ) ) {
        web_printf( conn, "Set to default.");
    }
    web_printf( conn, "<br>");
    web_printf( conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>IndexFiles:</b> ");
    web_printf( conn, wxString::FromUTF8( gpobj->m_indexFiles ).mbc_str() );
    if ( 0 == strlen( gpobj->m_indexFiles ) ) {
        web_printf( conn, "Set to default.");
    }
    web_printf( conn, "<br>");
    if ( NULL != strstr( gpobj->m_EnableDirectoryListings, "yes" ) ) {
        web_printf( conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Directory listings</b> is enabled.");
    }
    else {
       web_printf( conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Directory listings</b> is disabled.");
    }
    web_printf( conn, "<br>");
    web_printf( conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>UrlReqrites:</b> ");
    web_printf( conn, wxString::FromUTF8( gpobj->m_urlRewrites ).mbc_str() );
    if ( 0 == strlen( gpobj->m_urlRewrites ) ) {
       web_printf( conn, "Set to default.");
    }

    // TODO missing data


    web_printf( conn, "<hr>");

    // Websockets

    web_printf( conn, "<b>VSCP websocket interface:</b> ");
    if ( gpobj->m_bAuthWebsockets ) {
        web_printf( conn, "<b>Authentication</b> enabled.");
    }
    else {
        web_printf( conn, "<b>Authentication</b> disabled.");
    }

    web_printf( conn, "<hr>");

    // DM
    web_printf( conn, "<b>Daemon internal decision matrix functionality:</b> ");
    web_printf( conn, "enabled.");
    web_printf( conn, "<br>");
    web_printf( conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Path to DM file:</b> ");
    web_printf( conn, gpobj->m_dm.m_staticXMLPath.mbc_str() );
    web_printf( conn, "<br>");

    web_printf( conn, "<hr>");

    // Variable handling
    web_printf( conn, "<b>Variable handling :</b> ");
    web_printf( conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Path to variables:</b> ");
    web_printf( conn, gpobj->m_VSCP_Variables.m_xmlPath.mbc_str() );

    web_printf( conn, "<hr>");

    web_printf( conn, "<b>Level I Drivers:</b> ");

    web_printf( conn, "enabled<br>");

    CDeviceItem *pDeviceItem;
    VSCPDEVICELIST::iterator iter;
    for ( iter = gpobj->m_deviceList.m_devItemList.begin();
            iter != gpobj->m_deviceList.m_devItemList.end();
            ++iter ) {

        pDeviceItem = *iter;
        if  ( ( NULL != pDeviceItem ) &&
                    ( CLIENT_ITEM_INTERFACE_TYPE_DRIVER_LEVEL2 == pDeviceItem->m_driverLevel ) &&
                    pDeviceItem->m_bEnable ) {
            web_printf( conn, "<b>Name:</b> ");
            web_printf( conn, pDeviceItem->m_strName.mbc_str() );
            web_printf( conn, "<br>");
            web_printf( conn, "<b>Config:</b> ");
            web_printf( conn, pDeviceItem->m_strParameter.mbc_str() );
            web_printf( conn, "<br>");
            web_printf( conn, "<b>Path:</b> ");
            web_printf( conn, pDeviceItem->m_strPath.mbc_str() );
            web_printf( conn, "<br>");
        }
    }

    web_printf( conn, "<br>");

    web_printf( conn, "<hr>");

    web_printf( conn, "<b>Level II Drivers:</b> ");
    web_printf( conn, "enabled<br>");

    //CDeviceItem *pDeviceItem;
    //VSCPDEVICELIST::iterator iter;
    for (iter = gpobj->m_deviceList.m_devItemList.begin();
            iter != gpobj->m_deviceList.m_devItemList.end();
            ++iter) {

        pDeviceItem = *iter;
        if  ( ( NULL != pDeviceItem ) &&
                    ( CLIENT_ITEM_INTERFACE_TYPE_DRIVER_LEVEL1 == pDeviceItem->m_driverLevel ) &&
                    pDeviceItem->m_bEnable ) {
            web_printf( conn, "<b>Name:</b> ");
            web_printf( conn, pDeviceItem->m_strName.mbc_str() );
            web_printf( conn, "<br>");
            web_printf( conn, "<b>Config:</b> ");
            web_printf( conn, pDeviceItem->m_strParameter.mbc_str() );
            web_printf( conn, "<br>");
            web_printf( conn, "<b>Path:</b> ");
            web_printf( conn, pDeviceItem->m_strPath.mbc_str() );
            web_printf( conn, "<br>");
        }
    }
 
    web_printf( conn, "<br>");

    return 1;
}



///////////////////////////////////////////////////////////////////////////////
// bootload
//

static int bootload( struct web_connection *conn, void *cbdata  )
{
    //char buf[80];
    wxString str;
    VSCPInformation vscpinfo;

    // Check pointer
    if ( NULL == conn ) return 0;

    web_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");
    
    web_printf( conn, 
                    WEB_COMMON_HEAD, 
                    "VSCP - Device discovery" );
    web_printf( conn, WEB_STYLE_START );
    web_printf( conn, WEB_COMMON_CSS );     // CSS style Code
    web_printf( conn, WEB_STYLE_END );
    web_printf( conn, WEB_COMMON_JS );      // Common Javascript code
    web_printf( conn, "<meta http-equiv=\"refresh\" content=\"5;url=/vscp" );
    web_printf( conn, "\">" );
    web_printf( conn, WEB_COMMON_HEAD_END_BODY_START );

    // navigation menu
    web_printf( conn, WEB_COMMON_MENU);
    web_printf( conn, "<b>Bootload functionality is not yet implemented!</b>");

    return 1;
}


///////////////////////////////////////////////////////////////////////////////
// table
//

static int table( struct web_connection *conn, void *cbdata )
{
    
/*
    wxString str;
    VSCPInformation vscpinfo;

    // Check pointer
    if (NULL == conn) return 0;

    const struct web_request_info *reqinfo =  
                web_get_request_info( conn );
    if ( NULL == reqinfo ) return 0;
 
    web_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");

    web_printf( conn, WEB_COMMON_HEAD, "VSCP - Table view" );
    web_printf( conn, WEB_STYLE_START);
    web_printf( conn, WEB_COMMON_CSS);     // CSS style Code
    web_printf( conn, WEB_STYLE_END);
    web_printf( conn, WEB_COMMON_JS);      // Common Javascript code
    web_printf( conn, WEB_COMMON_HEAD_END_BODY_START);

    // navigation menu
    web_printf( conn, WEB_COMMON_MENU);

    web_printf( conn, WEB_TABLELIST_BODY_START);
    web_printf( conn, WEB_TABLELIST_TR_HEAD);

    wxString tblName;
    CVSCPTable *ptblItem = NULL;
    gpobj->m_mutexTableList.Lock();
    listVSCPTables::iterator iter;
    for (iter = gpobj->m_listTables.begin(); iter != gpobj->m_listTables.end(); ++iter) {

        ptblItem = *iter;

        if ( ( NULL == ptblItem ) || !ptblItem->isOpen() ) continue;

        web_printf( conn, WEB_COMMON_TR_CLICKABLE_ROW,
                           "/vscp/tablelist?tblname=" +
                                            wxString::FromUTF8( ptblItem->m_vscpFileHead.nameTable ) ) ) );
        web_printf( conn, "<td><b>");
        web_printf( conn, ptblItem->m_vscpFileHead.nameTable );
        web_printf( conn, "</b><br>");
        web_printf( conn, "<div id=\"small\">");
        web_printf( conn, "<b>Filename:</b> ");
        web_printf( conn, ptblItem->getFileName();
        web_printf( conn, " <b>Filesize:</b> ");
        wxFileName ff( ptblItem->getFileName() );
        web_printf( conn, ff.GetHumanReadableSize();
        web_printf( conn, "<br><b>First date:</b> ");
        wxDateTime dtStart = wxDateTime( (time_t)ptblItem->getTimeStampStart() );
        web_printf( conn, dtStart.FormatISODate() + _(" ") + dtStart.FormatISOTime();
        web_printf( conn, " <b>Last date:</b> ");
        wxDateTime dtEnd = wxDateTime( (time_t)ptblItem->getTimeStampEnd() );
        web_printf( conn, dtEnd.FormatISODate() + _(" ") + dtEnd.FormatISOTime();
        web_printf( conn, " <b>Number of records: </b> ");
        web_printf( conn,wxString::Format(_("%d"), ptblItem->getNumberOfRecords() );
        web_printf( conn, "<br><b>X-label:</b> ");
        web_printf( conn, wxString::FromUTF8( ptblItem->m_vscpFileHead.nameXLabel );
        web_printf( conn, " <b>Y-label :</b> ");
        web_printf( conn, wxString::FromUTF8( ptblItem->m_vscpFileHead.nameYLabel );
        web_printf( conn, " <b>VSCP Class:</b> ");
        web_printf( conn, "%d", ptblItem->m_vscpFileHead.vscp_class );
        web_printf( conn, " <b>VSCP Type:</b> ");
        web_printf( conn, "%d", ptblItem->m_vscpFileHead.vscp_type );
        web_printf( conn, " <b>Unit :</b> ");
        web_printf( conn, "%d", ptblItem->m_vscpFileHead.vscp_unit );
        struct _vscptableInfo info;
        ptblItem->m_mutexThisTable.Lock();
 */
/*        TODO
        ptblItem->getInfo( &info );
        ptblItem->m_mutexThisTable.Unlock();
        web_printf( conn, "<br><b>Min-value:</b> ");
        web_printf( conn, "%g", info.minValue );
        web_printf( conn, " <b>Max-value:</b> ");
        web_printf( conn, "%g", info.maxValue );
        web_printf( conn, " <b>Mean-value:</b> ");
        web_printf( conn, "%g", info.meanValue );
        web_printf( conn, "<br><b>Description:</b> ");
        web_printf( conn,wxString::FromUTF8( ptblItem->m_vscpFileHead.descriptionTable );
        web_printf( conn, "</div>");
        web_printf( conn, "</td>");

        // Type
        web_printf( conn, "<td>");
        web_printf( conn, "<div id=\"small\">");
        if ( VSCP_TABLE_DYNAMIC == ptblItem->m_vscpFileHead.type ) {
            web_printf( conn, "<b>Dynamic</b>");
        }
        else {
            web_printf( conn, "<b>Static</b>");
            web_printf( conn, "<br><b>Static size:</b> ");
            web_printf( conn,"%d", ptblItem->m_vscpFileHead.staticSize );
        }
        web_printf( conn, "</div>");
        web_printf( conn, "</td>");

        web_printf( conn, "</tr>");
        ptblItem = NULL;
    } // for
    gpobj->m_mutexTableList.Unlock();
*/
    /*

    web_printf( conn, WEB_TABLELIST_TABLE_END);

    return 1;
*/     
}



///////////////////////////////////////////////////////////////////////////////
// tablelist
//

static int tablelist( struct web_connection *conn, void *cbdata  )
{

    char buf[512];
    wxString str;
    VSCPInformation vscpinfo;

    // Check pointer
    if (NULL == conn) return 0;

    const struct web_request_info *reqinfo =  
                web_get_request_info( conn );
    if ( NULL == reqinfo ) return 0;

    // From
    long nFrom = 0;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "from", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            nFrom = atoi( buf );
        }
    }

    // Count
    uint16_t nCount = 50;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "count", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            nCount = atoi( buf );
        }
    }


    wxString tblName;
    if ( NULL != reqinfo->query_string ) {
        if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "tblname", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {
            tblName = wxString::FromUTF8( buf );
        }
    }
/*
    gpobj->m_mutexTableList.Lock();
    bool bFound = false;
    CVSCPTable *ptblItem = NULL;
    listVSCPTables::iterator iter;
    for (iter = gpobj->m_listTables.begin(); iter != gpobj->m_listTables.end(); ++iter) {
        ptblItem = *iter;
        if ( ( NULL != ptblItem ) &&
                ( 0 == strcmp( ptblItem->m_vscpFileHead.nameTable, (const char *)tblName.mbc_str() ) ) ) {
            if ( ptblItem->isOpen() ) bFound = true;
            break;
        }
    }
    gpobj->m_mutexTableList.Unlock();

    // Navigation button
    if ( NULL != reqinfo->query_string ) { 
    if ( web_get_var( reqinfo->query_string,                             
                            strlen( reqinfo->query_string ), 
                            "navbtn", 
                            buf, 
                            sizeof( buf ) ) > 0 ) {

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
 }
*/
    web_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");
 
    web_printf( conn, 
                    WEB_COMMON_HEAD, 
                    "VSCP - Table List" );
    web_printf( conn, WEB_STYLE_START );
    web_printf( conn, WEB_COMMON_CSS );     // CSS style Code
    web_printf( conn, WEB_STYLE_END );
    web_printf( conn, WEB_COMMON_JS );      // Common Javascript code
    web_printf( conn, WEB_COMMON_HEAD_END_BODY_START );

    // navigation menu
    web_printf( conn, WEB_COMMON_MENU );

    web_printf( conn, WEB_TABLEVALUELIST_BODY_START, (const char *)tblName.mbc_str() );

    {
        /*web_printf( conn, WEB_TABLEVALUE_LIST_NAVIGATION,
                _("/vscp/tablelist"),
                nFrom,
                ((nFrom + nCount) < ptblItem->getNumberOfRecords() ) ?
                      nFrom + nCount - 1 : ptblItem->getNumberOfRecords() - 1,
                ptblItem->getNumberOfRecords(),
                nCount,
                nFrom,
                tblName.mbc_str() ).mbc_str() );

        web_printf( conn, "<br>");*/
    }

    /*if ( bFound ) {

        struct _vscpFileRecord *ptableInfo = new struct _vscpFileRecord[ nCount ];
        if ( NULL != ptableInfo ) {

            nCount = ptblItem->getRangeOfData( nFrom, nCount, ptableInfo  );

            web_printf( conn, WEB_TABLEVALUELIST_TR_HEAD);

            for ( uint16_t i=0; i<nCount; i++ ) {

                web_printf( conn, "<tr>");

                // record
                web_printf( conn, "<td>");
                web_printf( conn, "%u", nFrom + i );
                web_printf( conn, "</td>");

                // Date
                web_printf( conn, "<td>");
                wxDateTime dt( (time_t)ptableInfo[i].timestamp );
                web_printf( conn, dt.FormatISODate() + _(" ") + dt.FormatISOTime();
                web_printf( conn, "</td>");

                // value
                web_printf( conn, "<td>");
                web_printf( conn, "%g", ptableInfo[i].measurement );
                web_printf( conn, "</td>");

                web_printf( conn, "</tr>");

            }

            web_printf( conn, WEB_TABLEVALUELIST_TABLE_END);

            delete[] ptableInfo;
        }
        else {
            web_printf( conn, "Failed to allocate memory for table.");
        }
    }
    else {
        web_printf( conn, "Table not found!");
    }*/


    // Server data
   

    return 1;
}



//-----------------------------------------------------------------------------
//                                Variable
//-----------------------------------------------------------------------------



////////////////////////////////////////////////////////////////////////////////
// vscp_variable
//

static int
vscp_variable( struct web_connection *conn, void *cbdata )
{
	web_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");
	web_printf(conn, "<html><body>");
	web_printf(conn, "<h2>Defined variables</h2>");
	web_printf(
	    conn,
	    "<p>To see a page from the A handler <a href=\"A\">click A</a></p>");	
	web_printf(conn, "</body></html>\n");
	return 1;
}



//-----------------------------------------------------------------------------
//                                   User
//-----------------------------------------------------------------------------



////////////////////////////////////////////////////////////////////////////////
// vscp_user_list
//

static int
vscp_user_list(struct web_connection *conn, void *cbdata)
{
	web_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");
	web_printf(conn, "<html><body>");
	web_printf(conn, "<h2>Defined users</h2>");
	web_printf(
	    conn,
	    "<p>To see a page from the A handler <a href=\"A\">click A</a></p>");	
	web_printf(conn, "</body></html>\n");
	return 1;
}


#ifdef WEB_EXAMPLES


// -----------------------------------------------------------------------------
//                          CIVETWEB test setup
// -----------------------------------------------------------------------------





////////////////////////////////////////////////////////////////////////////////
// ExampleHandler
//

int
ExampleHandler(struct web_connection *conn, void *cbdata)
{
	web_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");
	web_printf(conn, "<html><body>");
	web_printf(conn, "<h2>This is an example text from a C handler</h2>");
	web_printf(
	    conn,
	    "<p>To see a page from the A handler <a href=\"A\">click A</a></p>");
	web_printf(conn,
	          "<p>To see a page from the A handler <a href=\"A/A\">click "
	          "A/A</a></p>");
	web_printf(conn,
	          "<p>To see a page from the A/B handler <a "
	          "href=\"A/B\">click A/B</a></p>");
	web_printf(conn,
	          "<p>To see a page from the B handler (0) <a "
	          "href=\"B\">click B</a></p>");
	web_printf(conn,
	          "<p>To see a page from the B handler (1) <a "
	          "href=\"B/A\">click B/A</a></p>");
	web_printf(conn,
	          "<p>To see a page from the B handler (2) <a "
	          "href=\"B/B\">click B/B</a></p>");
	web_printf(conn,
	          "<p>To see a page from the *.foo handler <a "
	          "href=\"xy.foo\">click xy.foo</a></p>");
	web_printf(conn,
	          "<p>To see a page from the close handler <a "
	          "href=\"close\">click close</a></p>");
	web_printf(conn,
	          "<p>To see a page from the FileHandler handler <a "
	          "href=\"form\">click form</a> (the starting point of the "
	          "<b>form</b> test)</p>");
	web_printf(conn,
	          "<p>To see a page from the CookieHandler handler <a "
	          "href=\"cookie\">click cookie</a></p>");
	web_printf(conn,
	          "<p>To see a page from the PostResponser handler <a "
	          "href=\"postresponse\">click post response</a></p>");
	web_printf(conn,
	          "<p>To see an example for parsing files on the fly <a "
	          "href=\"on_the_fly_form\">click form</a> (form for "
	          "uploading files)</p>");

	web_printf(conn,
	          "<p>To test websocket handler <a href=\"/websocket\">click "
	          "websocket</a></p>");

	web_printf(conn, "<p>To exit <a href=\"%s\">click exit</a></p>", EXIT_URI);
	web_printf(conn, "</body></html>\n");
	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// ExitHandler
//

int
ExitHandler(struct web_connection *conn, void *cbdata)
{
	web_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: "
	          "text/plain\r\nConnection: close\r\n\r\n");
	web_printf(conn, "Server will shut down.\n");
	web_printf(conn, "Bye!\n");
	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// AHandler
//

int
AHandler(struct web_connection *conn, void *cbdata)
{
	web_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");
	web_printf(conn, "<html><body>");
	web_printf(conn, "<h2>This is the A handler!!!</h2>");
	web_printf(conn, "</body></html>\n");
	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// ABHandler
//

int
ABHandler(struct web_connection *conn, void *cbdata)
{
	web_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");
	web_printf(conn, "<html><body>");
	web_printf(conn, "<h2>This is the AB handler!!!</h2>");
	web_printf(conn, "</body></html>\n");
	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// BXHandler
//

int
BXHandler(struct web_connection *conn, void *cbdata)
{
	/* Handler may access the request info using web_get_request_info */
	const struct web_request_info *req_info = web_get_request_info(conn);

	web_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");
	web_printf(conn, "<html><body>");
	web_printf(conn, "<h2>This is the BX handler %p!!!</h2>", cbdata);
	web_printf(conn, "<p>The actual uri is %s</p>", req_info->local_uri);
	web_printf(conn, "</body></html>\n");
	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// FooHandler
//

int
FooHandler(struct web_connection *conn, void *cbdata)
{
	/* Handler may access the request info using web_get_request_info */
	const struct web_request_info *req_info = web_get_request_info(conn);

	web_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");
	web_printf(conn, "<html><body>");
	web_printf(conn, "<h2>This is the Foo handler!!!</h2>");
	web_printf(conn,
	          "<p>The request was:<br><pre>%s %s HTTP/%s</pre></p>",
	          req_info->request_method,
	          req_info->local_uri,
	          req_info->http_version);
	web_printf(conn, "</body></html>\n");
	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// CloseHandler
//

int
CloseHandler(struct web_connection *conn, void *cbdata)
{
	/* Handler may access the request info using web_get_request_info */
	const struct web_request_info *req_info = web_get_request_info(conn);

	web_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");
	web_printf(conn, "<html><body>");
	web_printf(conn,
	          "<h2>This handler will close the connection in a second</h2>");
#ifdef _WIN32
	Sleep(1000);
#else
	sleep(1);
#endif
	web_printf(conn, "bye");
	printf("CloseHandler: close connection\n");
	web_close_connection(conn);
	printf("CloseHandler: wait 10 sec\n");
#ifdef _WIN32
	Sleep(10000);
#else
	sleep(10);
#endif
	printf("CloseHandler: return from function\n");
	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// FileHandler
//

int
FileHandler(struct web_connection *conn, void *cbdata)
{
	/* In this handler, we ignore the req_info and send the file "fileName". */
	const char *fileName = (const char *)cbdata;

	web_send_file(conn, fileName);
	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// field_found
//

int
field_found(const char *key,
            const char *filename,
            char *path,
            size_t pathlen,
            void *user_data)
{
	struct web_connection *conn = (struct web_connection *)user_data;

	web_printf(conn, "\r\n\r\n%s:\r\n", key);

	if (filename && *filename) {
#ifdef _WIN32
		_snprintf(path, pathlen, "D:\\tmp\\%s", filename);
#else
		snprintf(path, pathlen, "/tmp/%s", filename);
#endif
		return FORM_FIELD_STORAGE_STORE;
	}
	return FORM_FIELD_STORAGE_GET;
}

////////////////////////////////////////////////////////////////////////////////
// field_get
//

int
field_get(const char *key, const char *value, size_t valuelen, void *user_data)
{
	struct web_connection *conn = (struct web_connection *)user_data;

	if (key[0]) {
		web_printf(conn, "%s = ", key);
	}
	web_write(conn, value, valuelen);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// field_stored
//

int
field_stored(const char *path, long long file_size, void *user_data)
{
	struct web_connection *conn = (struct web_connection *)user_data;

	web_printf(conn,
	          "stored as %s (%lu bytes)\r\n\r\n",
	          path,
	          (unsigned long)file_size);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// FormHandler
//

int
FormHandler(struct web_connection *conn, void *cbdata)
{
	/* Handler may access the request info using web_get_request_info */
	const struct web_request_info *req_info = web_get_request_info(conn);
	int ret;
	struct web_form_data_handler fdh = {field_found, field_get, field_stored, 0};

	/* It would be possible to check the request info here before calling
	 * web_handle_form_request. */
	(void)req_info;

	web_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: "
	          "text/plain\r\nConnection: close\r\n\r\n");
	fdh.user_data = (void *)conn;

	/* Call the form handler */
	web_printf(conn, "Form data:");
	ret = web_handle_form_request(conn, &fdh);
	web_printf(conn, "\r\n%i fields found", ret);

	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// FileUploadForm
//

int
FileUploadForm(struct web_connection *conn, void *cbdata)
{
	web_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");

	web_printf(conn, "<!DOCTYPE html>\n");
	web_printf(conn, "<html>\n<head>\n");
	web_printf(conn, "<meta charset=\"UTF-8\">\n");
	web_printf(conn, "<title>File upload</title>\n");
	web_printf(conn, "</head>\n<body>\n");
	web_printf(conn,
	          "<form action=\"%s\" method=\"POST\" "
	          "enctype=\"multipart/form-data\">\n",
	          (const char *)cbdata);
	web_printf(conn, "<input type=\"file\" name=\"filesin\" multiple>\n");
	web_printf(conn, "<input type=\"submit\" value=\"Submit\">\n");
	web_printf(conn, "</form>\n</body>\n</html>\n");
	return 1;
}

struct tfile_checksum {
    char name[128];
    unsigned long long length;
    md5_state_t chksum;
};

#define MAX_FILES (10)

struct tfiles_checksums {
	int index;
	struct tfile_checksum file[MAX_FILES];
};

////////////////////////////////////////////////////////////////////////////////
// field_disp_read_on_the_fly
//

int
field_disp_read_on_the_fly(const char *key,
                           const char *filename,
                           char *path,
                           size_t pathlen,
                           void *user_data)
{
	struct tfiles_checksums *context = (struct tfiles_checksums *)user_data;

	(void)key;
	(void)path;
	(void)pathlen;

	if (context->index < MAX_FILES) {
		context->index++;
		strncpy(context->file[context->index - 1].name, filename, 128);
		context->file[context->index - 1].name[127] = 0;
		context->file[context->index - 1].length = 0;
		vscpmd5_init(&(context->file[context->index - 1].chksum));
		return FORM_FIELD_STORAGE_GET;
	}
	return FORM_FIELD_STORAGE_ABORT;
}

////////////////////////////////////////////////////////////////////////////////
// field_get_checksum
//

int
field_get_checksum(const char *key,
                   const char *value,
                   size_t valuelen,
                   void *user_data)
{
	struct tfiles_checksums *context = (struct tfiles_checksums *)user_data;
	(void)key;

	context->file[context->index - 1].length += valuelen;
	vscpmd5_append(&(context->file[context->index - 1].chksum),
	           (const md5_byte_t *)value,
	           valuelen);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// CheckSumHandler
//

int
CheckSumHandler(struct web_connection *conn, void *cbdata)
{
	/* Handler may access the request info using web_get_request_info */
	const struct web_request_info *req_info = web_get_request_info(conn);
	int i, j, ret;
	struct tfiles_checksums chksums;
	md5_byte_t digest[16];
	struct web_form_data_handler fdh = {field_disp_read_on_the_fly,
	                                   field_get_checksum,
	                                   0,
	                                   (void *)&chksums};

	/* It would be possible to check the request info here before calling
	 * web_handle_form_request. */
	(void)req_info;

	memset(&chksums, 0, sizeof(chksums));

	web_printf(conn,
	          "HTTP/1.1 200 OK\r\n"
	          "Content-Type: text/plain\r\n"
	          "Connection: close\r\n\r\n");

	/* Call the form handler */
	web_printf(conn, "File checksums:");
	ret = web_handle_form_request(conn, &fdh);
	for (i = 0; i < chksums.index; i++) {
		vscpmd5_finish(&(chksums.file[i].chksum), digest);
		/* Visual Studio 2010+ support llu */
		web_printf(conn,
		          "\r\n%s %llu ",
		          chksums.file[i].name,
		          chksums.file[i].length);
		for (j = 0; j < 16; j++) {
			web_printf(conn, "%02x", (unsigned int)digest[j]);
		}
	}
	web_printf(conn, "\r\n%i files\r\n", ret);

	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// CookieHandler
//

int
CookieHandler(struct web_connection *conn, void *cbdata)
{
	/* Handler may access the request info using web_get_request_info */
	const struct web_request_info *req_info = web_get_request_info(conn);
	const char *cookie = web_get_header(conn, "Cookie");
	char first_str[64], count_str[64];
	int count;

	(void)web_get_cookie(cookie, "first", first_str, sizeof(first_str));
	(void)web_get_cookie(cookie, "count", count_str, sizeof(count_str));

	web_printf(conn, "HTTP/1.1 200 OK\r\nConnection: close\r\n");
	if (first_str[0] == 0) {
		time_t t = time(0);
		struct tm *ptm = localtime(&t);
		web_printf(conn,
		          "Set-Cookie: first=%04i-%02i-%02iT%02i:%02i:%02i\r\n",
		          ptm->tm_year + 1900,
		          ptm->tm_mon + 1,
		          ptm->tm_mday,
		          ptm->tm_hour,
		          ptm->tm_min,
		          ptm->tm_sec);
	}
	count = (count_str[0] == 0) ? 0 : atoi(count_str);
	web_printf(conn, "Set-Cookie: count=%i\r\n", count + 1);
	web_printf(conn, "Content-Type: text/html\r\n\r\n");

	web_printf(conn, "<html><body>");
	web_printf(conn, "<h2>This is the CookieHandler.</h2>");
	web_printf(conn, "<p>The actual uri is %s</p>", req_info->local_uri);

	if (first_str[0] == 0) {
		web_printf(conn, "<p>This is the first time, you opened this page</p>");
	} else {
		web_printf(conn, "<p>You opened this page %i times before.</p>", count);
		web_printf(conn, "<p>You first opened this page on %s.</p>", first_str);
	}

	web_printf(conn, "</body></html>\n");
	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// PostResponser
//

int
PostResponser(struct web_connection *conn, void *cbdata)
{
	long long r_total = 0;
	int r, s;

	char buf[2048];

	const struct web_request_info *ri = web_get_request_info(conn);

	if (strcmp(ri->request_method, "POST")) {
		char buf[1024];
		int ret = web_get_request_link(conn, buf, sizeof(buf));

		web_printf(conn,
		          "HTTP/1.1 405 Method Not Allowed\r\nConnection: close\r\n");
		web_printf(conn, "Content-Type: text/plain\r\n\r\n");
		web_printf(conn,
		          "%s method not allowed in the POST handler\n",
		          ri->request_method);
		if (ret >= 0) {
			web_printf(conn,
			          "use a web tool to send a POST request to %s\n",
			          buf);
		}
		return 1;
	}

	if (ri->content_length >= 0) {
		/* We know the content length in advance */
	} 
        else {
		/* We must read until we find the end (chunked encoding
		 * or connection close), indicated my web_read returning 0 */
	}

	web_printf(conn,
	          "HTTP/1.1 200 OK\r\nConnection: "
	          "close\r\nTransfer-Encoding: chunked\r\n");
	web_printf(conn, "Content-Type: text/plain\r\n\r\n");

	r = web_read(conn, buf, sizeof(buf));
	while (r > 0) {
		r_total += r;
		s = web_send_chunk(conn, buf, r);
		if (r != s) {
			/* Send error */
			break;
		}
		r = web_read(conn, buf, sizeof(buf));
	}
	web_printf(conn, "0\r\n");

	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// WebSocketStartHandler
//

int
WebSocketStartHandler(struct web_connection *conn, void *cbdata)
{
	web_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");

	web_printf(conn, "<!DOCTYPE html>\n");
	web_printf(conn, "<html>\n<head>\n");
	web_printf(conn, "<meta charset=\"UTF-8\">\n");
	web_printf(conn, "<title>Embedded websocket example</title>\n");

	/* web_printf(conn, "<script type=\"text/javascript\"><![CDATA[\n"); ...
	 * xhtml style */
	web_printf(conn, "<script>\n");
	web_printf(
	    conn,
	    "function load() {\n"
	    "  var wsproto = (location.protocol === 'https:') ? 'wss:' : 'ws:';\n"
	    "  connection = new WebSocket(wsproto + '//' + window.location.host + "
	    "'/websocket');\n"
	    "  websock_text_field = "
	    "document.getElementById('websock_text_field');\n"
	    "  connection.onmessage = function (e) {\n"
	    "    websock_text_field.innerHTML=e.data;\n"
	    "  }\n"
	    "  connection.onerror = function (error) {\n"
	    "    alert('WebSocket error');\n"
	    "    connection.close();\n"
	    "  }\n"
	    "}\n");
	/* web_printf(conn, "]]></script>\n"); ... xhtml style */
	web_printf(conn, "</script>\n");
	web_printf(conn, "</head>\n<body onload=\"load()\">\n");
	web_printf(
	    conn,
	    "<div id='websock_text_field'>No websocket connection yet</div>\n");
	web_printf(conn, "</body>\n</html>\n");

	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// t_ws_client
//



/* MAX_WS_CLIENTS defines how many clients can connect to a websocket at the
 * same time. The value 5 is very small and used here only for demonstration;
 * it can be easily tested to connect more than MAX_WS_CLIENTS clients.
 * A real server should use a much higher number, or better use a dynamic list
 * of currently connected websocket clients. */
#define MAX_WS_CLIENTS (5)

struct t_ws_client {
	struct web_connection *conn;
	int state;
} static ws_clients[MAX_WS_CLIENTS];


#define ASSERT(x)                                                              \
	{                                                                      \
		if (!(x)) {                                                    \
			fprintf(stderr,                                        \
			        "Assertion failed in line %u\n",               \
			        (unsigned)__LINE__);                           \
		}                                                              \
	}

////////////////////////////////////////////////////////////////////////////////
// WebSocketConnectHandler
//

int
WebSocketConnectHandler(const struct web_connection *conn, void *cbdata)
{
	struct web_context *ctx = web_get_context(conn);
	int reject = 1;
	int i;

	web_lock_context(ctx);
	for (i = 0; i < MAX_WS_CLIENTS; i++) {
		if (ws_clients[i].conn == NULL) {
			ws_clients[i].conn = (struct web_connection *)conn;
			ws_clients[i].state = 1;
			web_set_user_connection_data(ws_clients[i].conn,
			                            (void *)(ws_clients + i));
			reject = 0;
			break;
		}
	}
	web_unlock_context(ctx);

	fprintf(stdout,
	        "Websocket client %s\r\n\r\n",
	        (reject ? "rejected" : "accepted"));
	return reject;
}

////////////////////////////////////////////////////////////////////////////////
// WebSocketReadyHandler
//

void
WebSocketReadyHandler(struct web_connection *conn, void *cbdata)
{
	const char *text = "Hello from the websocket ready handler";
	struct t_ws_client *client = (struct t_ws_client *)web_get_user_connection_data(conn);

	web_websocket_write(conn, WEBSOCKET_OPCODE_TEXT, text, strlen(text));
	fprintf(stdout, "Greeting message sent to websocket client\r\n\r\n");
	ASSERT(client->conn == conn);
	ASSERT(client->state == 1);

	client->state = 2;
}

////////////////////////////////////////////////////////////////////////////////
// WebsocketDataHandler
//

int
WebsocketDataHandler(struct web_connection *conn,
                     int bits,
                     char *data,
                     size_t len,
                     void *cbdata)
{
	struct t_ws_client *client = (struct t_ws_client *)web_get_user_connection_data(conn);
	ASSERT(client->conn == conn);
	ASSERT(client->state >= 1);

	fprintf(stdout, "Websocket got %lu bytes of ", (unsigned long)len);
	switch (((unsigned char)bits) & 0x0F) {
	case WEBSOCKET_OPCODE_CONTINUATION:
		fprintf(stdout, "continuation");
		break;
	case WEBSOCKET_OPCODE_TEXT:
		fprintf(stdout, "text");
		break;
	case WEBSOCKET_OPCODE_BINARY:
		fprintf(stdout, "binary");
		break;
	case WEBSOCKET_OPCODE_CONNECTION_CLOSE:
		fprintf(stdout, "close");
		break;
	case WEBSOCKET_OPCODE_PING:
		fprintf(stdout, "ping");
		break;
	case WEBSOCKET_OPCODE_PONG:
		fprintf(stdout, "pong");
		break;
	default:
		fprintf(stdout, "unknown(%1xh)", ((unsigned char)bits) & 0x0F);
		break;
	}
	fprintf(stdout, " data:\r\n");
	fwrite(data, len, 1, stdout);
	fprintf(stdout, "\r\n\r\n");

	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// WebSocketCloseHandler
//

void
WebSocketCloseHandler(const struct web_connection *conn, void *cbdata)
{
	struct web_context *ctx = web_get_context(conn);
	struct t_ws_client *client = (struct t_ws_client *)web_get_user_connection_data(conn);
	ASSERT(client->conn == conn);
	ASSERT(client->state >= 1);

	web_lock_context(ctx);
	client->state = 0;
	client->conn = NULL;
	web_unlock_context(ctx);

	fprintf(stdout,
	        "Client droped from the set of webserver connections\r\n\r\n");
}

////////////////////////////////////////////////////////////////////////////////
// InformWebsockets
//

void
InformWebsockets(struct web_context *ctx)
{
	static unsigned long cnt = 0;
	char text[32];
	int i;

	sprintf(text, "%lu", ++cnt);

	web_lock_context(ctx);
	for (i = 0; i < MAX_WS_CLIENTS; i++) {
		if (ws_clients[i].state == 2) {
			web_websocket_write(ws_clients[i].conn,
			                   WEBSOCKET_OPCODE_TEXT,
			                   text,
			                   strlen(text));
		}
	}
	web_unlock_context(ctx);
}

#endif  // WEB_EXAMPLES

// https://security.stackexchange.com/questions/41205/diffie-hellman-and-its-tls-ssl-usage
#ifdef USE_SSL_DH
#include "openssl/ssl.h"
#include "openssl/dh.h"
#include "openssl/ec.h"
#include "openssl/evp.h"
#include "openssl/ecdsa.h"

DH *
get_dh2236()
{
	static unsigned char dh2236_p[] = {
	    0x0E, 0x97, 0x6E, 0x6A, 0x88, 0x84, 0xD2, 0xD7, 0x55, 0x6A, 0x17, 0xB7,
	    0x81, 0x9A, 0x98, 0xBC, 0x7E, 0xD1, 0x6A, 0x44, 0xB1, 0x18, 0xE6, 0x25,
	    0x3A, 0x62, 0x35, 0xF0, 0x41, 0x91, 0xE2, 0x16, 0x43, 0x9D, 0x8F, 0x7D,
	    0x5D, 0xDA, 0x85, 0x47, 0x25, 0xC4, 0xBA, 0x68, 0x0A, 0x87, 0xDC, 0x2C,
	    0x33, 0xF9, 0x75, 0x65, 0x17, 0xCB, 0x8B, 0x80, 0xFE, 0xE0, 0xA8, 0xAF,
	    0xC7, 0x9E, 0x82, 0xBE, 0x6F, 0x1F, 0x00, 0x04, 0xBD, 0x69, 0x50, 0x8D,
	    0x9C, 0x3C, 0x41, 0x69, 0x21, 0x4E, 0x86, 0xC8, 0x2B, 0xCC, 0x07, 0x4D,
	    0xCF, 0xE4, 0xA2, 0x90, 0x8F, 0x66, 0xA9, 0xEF, 0xF7, 0xFC, 0x6F, 0x5F,
	    0x06, 0x22, 0x00, 0xCB, 0xCB, 0xC3, 0x98, 0x3F, 0x06, 0xB9, 0xEC, 0x48,
	    0x3B, 0x70, 0x6E, 0x94, 0xE9, 0x16, 0xE1, 0xB7, 0x63, 0x2E, 0xAB, 0xB2,
	    0xF3, 0x84, 0xB5, 0x3D, 0xD7, 0x74, 0xF1, 0x6A, 0xD1, 0xEF, 0xE8, 0x04,
	    0x18, 0x76, 0xD2, 0xD6, 0xB0, 0xB7, 0x71, 0xB6, 0x12, 0x8F, 0xD1, 0x33,
	    0xAB, 0x49, 0xAB, 0x09, 0x97, 0x35, 0x9D, 0x4B, 0xBB, 0x54, 0x22, 0x6E,
	    0x1A, 0x33, 0x18, 0x02, 0x8A, 0xF4, 0x7C, 0x0A, 0xCE, 0x89, 0x75, 0x2D,
	    0x10, 0x68, 0x25, 0xA9, 0x6E, 0xCD, 0x97, 0x49, 0xED, 0xAE, 0xE6, 0xA7,
	    0xB0, 0x07, 0x26, 0x25, 0x60, 0x15, 0x2B, 0x65, 0x88, 0x17, 0xF2, 0x5D,
	    0x2C, 0xF6, 0x2A, 0x7A, 0x8C, 0xAD, 0xB6, 0x0A, 0xA2, 0x57, 0xB0, 0xC1,
	    0x0E, 0x5C, 0xA8, 0xA1, 0x96, 0x58, 0x9A, 0x2B, 0xD4, 0xC0, 0x8A, 0xCF,
	    0x91, 0x25, 0x94, 0xB4, 0x14, 0xA7, 0xE4, 0xE2, 0x1B, 0x64, 0x5F, 0xD2,
	    0xCA, 0x70, 0x46, 0xD0, 0x2C, 0x95, 0x6B, 0x9A, 0xFB, 0x83, 0xF9, 0x76,
	    0xE6, 0xD4, 0xA4, 0xA1, 0x2B, 0x2F, 0xF5, 0x1D, 0xE4, 0x06, 0xAF, 0x7D,
	    0x22, 0xF3, 0x04, 0x30, 0x2E, 0x4C, 0x64, 0x12, 0x5B, 0xB0, 0x55, 0x3E,
	    0xC0, 0x5E, 0x56, 0xCB, 0x99, 0xBC, 0xA8, 0xD9, 0x23, 0xF5, 0x57, 0x40,
	    0xF0, 0x52, 0x85, 0x9B,
	};
	static unsigned char dh2236_g[] = {
	    0x02,
	};
	DH *dh;

	if ((dh = DH_new()) == NULL)
		return (NULL);
	dh->p = BN_bin2bn(dh2236_p, sizeof(dh2236_p), NULL);
	dh->g = BN_bin2bn(dh2236_g, sizeof(dh2236_g), NULL);
	if ((dh->p == NULL) || (dh->g == NULL)) {
		DH_free(dh);
		return (NULL);
	}
	return (dh);
}
#endif 


int
init_ssl(void *ssl_context, void *user_data)
{
	/* Add application specific SSL initialization */
	struct ssl_ctx_st *ctx = (struct ssl_ctx_st *)ssl_context;

#ifdef USE_SSL_DH
	/* example from https://github.com/civetweb/civetweb/issues/347 */
	DH *dh = get_dh2236();
	if (!dh)
		return -1;
	if (1 != SSL_CTX_set_tmp_dh(ctx, dh))
		return -1;
	DH_free(dh);

	EC_KEY *ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
	if (!ecdh)
		return -1;
	if (1 != SSL_CTX_set_tmp_ecdh(ctx, ecdh))
		return -1;
	EC_KEY_free(ecdh);

	printf("ECDH ciphers initialized\n");
#endif
	return 0;
}



int
log_message( const struct web_connection *conn, const char *message )
{
    puts(message);
    return 1;
}


///////////////////////////////////////////////////////////////////////////////
// init_webserver
//

int init_webserver( void ) 
{   
    const char *options[] = 
    {
        "document_root",
	"/srv/vscp/web",
	
        "listening_ports",
	"[::]:8888r,[::]:8843s,8884",
	
        "request_timeout_ms",
	"10000",
	
        "error_log_file",
	"error.log",

	"websocket_timeout_ms",
	"3600000",
            
	"ssl_certificate",
	"/srv/vscp/certs/server.pem",
	
        "ssl_protocol_version",
	"3",
	
        "ssl_cipher_list",
#ifdef USE_SSL_DH
	"ECDHE-RSA-AES256-GCM-SHA384:DES-CBC3-SHA:AES128-SHA:AES128-GCM-SHA256",
#else
	"DES-CBC3-SHA:AES128-SHA:AES128-GCM-SHA256",
#endif

	"enable_auth_domain_check",
	"no",
	0
    };
    
    struct web_callbacks callbacks;
    //struct web_context *ctx;
    struct web_server_ports ports[32];
    int port_cnt, n;
    int err = 0;

    // Check if we have been built with all required features. 
    if ( !web_check_feature( 8 ) ) {
        fprintf( stderr,
                    "Error: Embedded example built with IPv6 support, "
		    "but webserver build without.\n");
        err = 1;
    }

    if ( !web_check_feature( 16 ) ) {
        fprintf( stderr,
                    "Error: Embedded example built with websocket support, "
		    "but webserver build without.\n");
        err = 1;
    }

    if ( !web_check_feature( 2 ) ) {
        fprintf( stderr,
                    "Error: Embedded example built with SSL support, "
                    "but webserver build without.\n" );
	err = 1;
    }

    if ( err ) {
	fprintf( stderr, "Cannot start webserver - inconsistent build.\n");
            return EXIT_FAILURE;
    }

    // Start CivetWeb web server 
    memset( &callbacks, 0, sizeof( callbacks ) );
    callbacks.init_ssl = init_ssl;

    callbacks.log_message = log_message;
    gpobj->webctx = web_start(&callbacks, 0, options);

    // Check return value: 
    if ( NULL == gpobj->webctx ) {
        fprintf( stderr, "Cannot start webserver - web_start failed.\n" );
	return EXIT_FAILURE;
    }
        
#ifdef WEB_EXAMPLES

    // Add handler EXAMPLE_URI, to explain the example 
    web_set_request_handler( gpobj->webctx, EXAMPLE_URI, ExampleHandler, 0);
    web_set_request_handler( gpobj->webctx, EXIT_URI, ExitHandler, 0);

    // Add handler for /A* and special handler for /A/B 
    web_set_request_handler( gpobj->webctx, "/A", AHandler, 0);
    web_set_request_handler( gpobj->webctx, "/A/B", ABHandler, 0);

    // Add handler for /B, /B/A, /B/B but not for /B* 
    web_set_request_handler( gpobj->webctx, "/B$", BXHandler, (void *) 0);
    web_set_request_handler( gpobj->webctx, "/B/A$", BXHandler, (void *) 1);
    web_set_request_handler( gpobj->webctx, "/B/B$", BXHandler, (void *) 2);

    // Add handler for all files with .foo extention 
    web_set_request_handler( gpobj->webctx, "**.foo$", FooHandler, 0);

    // Add handler for /close extention 
    web_set_request_handler( gpobj->webctx, "/close", CloseHandler, 0);

    // Add handler for /form  (serve a file outside the document root) 
    web_set_request_handler( gpobj->webctx,
                                    "/form",
                                    FileHandler,
                                    (void *) "../../test/form.html");

    // Add handler for form data 
    web_set_request_handler( gpobj->webctx,
                                    "/handle_form.embedded_c.example.callback",
                                    FormHandler,
                                    (void *)0 );

    // Add a file upload handler for parsing files on the fly 
    web_set_request_handler( gpobj->webctx,
            "/on_the_fly_form",
            FileUploadForm,
            (void *) "/on_the_fly_form.md5.callback");
    web_set_request_handler( gpobj->webctx,
            "/on_the_fly_form.md5.callback",
            CheckSumHandler,
            (void *) 0);

    /* Add handler for /cookie example */
    web_set_request_handler( gpobj->webctx, "/cookie", CookieHandler, 0);

    /* Add handler for /postresponse example */
    web_set_request_handler( gpobj->webctx, "/postresponse", PostResponser, 0);

    /* Add HTTP site to open a websocket connection */
    web_set_request_handler( gpobj->webctx, "/websocket", WebSocketStartHandler, 0);

    /* WS site for the websocket connection */
    web_set_websocket_handler( gpobj->webctx,
                                    "/websocket",
                                    WebSocketConnectHandler,
                                    WebSocketReadyHandler,
                                    WebsocketDataHandler,
                                    WebSocketCloseHandler,
                                    0 );
#endif  // WEB_EXAMPLES     
    
    // Set page handlers
    web_set_request_handler( gpobj->webctx, "/vscp", vscp_mainpage, 0);
    web_set_request_handler( gpobj->webctx, "/vscp/session", vscp_client, 0 );
    web_set_request_handler( gpobj->webctx, "/vscp/configure", vscp_configure, 0 );
    web_set_request_handler( gpobj->webctx, "/vscp/interfaces", vscp_interface, 0 );
    web_set_request_handler( gpobj->webctx, "/vscp/settings", vscp_settings, 0);
    web_set_request_handler( gpobj->webctx, "/vscp/variables", vscp_variable_list, 0);
    web_set_request_handler( gpobj->webctx, "/vscp/varedit", vscp_variable_edit, 0);
    web_set_request_handler( gpobj->webctx, "/vscp/varpost", vscp_variable_post, 0);
    web_set_request_handler( gpobj->webctx, "/vscp/varnew", vscp_variable_new, 0);
    web_set_request_handler( gpobj->webctx, "/vscp/vardelete", vscp_variable_delete, 0);
    web_set_request_handler( gpobj->webctx, "/vscp/dm", vscp_dm_list, 0);
    web_set_request_handler( gpobj->webctx, "/vscp/dmedit", vscp_dm_edit, 0);
    web_set_request_handler( gpobj->webctx, "/vscp/dmpost", vscp_dm_post, 0);
    web_set_request_handler( gpobj->webctx, "/vscp/dmdelete", vscp_dm_delete, 0);
    web_set_request_handler( gpobj->webctx, "/vscp/users", vscp_user_list, 0);
    
    // Set authorization handlers
    web_set_auth_handler( gpobj->webctx, "/vscp", check_admin_authorization, NULL );
}


///////////////////////////////////////////////////////////////////////////////
// stop_webserver
//

int stop_webserver( void )
{
    web_stop( gpobj->webctx );
}
