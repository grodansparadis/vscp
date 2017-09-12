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
#include <vscpweb.h>
#include <webserver.h>
#include <webserver_websocket.h>

using namespace std;

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif




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
    if ( -1 == stat( path, &statbuf ) ) {
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
    while( isspace(*str) ) str++;

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
                                "Connection: keep-alive\r\n"
                                "Transfer-Encoding: chunked\r\n"
                                "Cache-Control\r\n"
                                "max-age=0, post-check=0,\r\n"
                                "pre-check=0, no-store, no-cache,"
                                "must-revalidate\r\n\r\n",
                                returncode,
                                content,
                                date );
    mg_send( nc, buf, n );
}

///////////////////////////////////////////////////////////////////////////////
// webserv_url_decode
//

int webserv_url_decode( const char *src, int src_len,
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

    dst[j] = '\0'; /* Null-terminate the destination */

    return i >= src_len ? j : -1;
}


///////////////////////////////////////////////////////////////////////////////
// vscp_check_nonce
//
// Check for authentication timeout.
// Clients send time stamp encoded in nonce. Make sure it is not too old,
// to prevent replay attacks.
// Assumption: nonce is a hexadecimal number of seconds since 1970.
//

static int vscp_check_nonce( const char *nonce )
{
    unsigned long now = (unsigned long) time( NULL );
    unsigned long val = (unsigned long) strtoul( nonce, NULL, 16 );
    return ( /*1 ||*/ ( now < val ) || ( ( now - val ) < 3600 ) );
}


///////////////////////////////////////////////////////////////////////////////
// vscp_mkmd5resp
//

static void vscp_mkmd5resp( const char *method, size_t method_len, const char *uri,
                        size_t uri_len, const char *ha1, size_t ha1_len,
                        const char *nonce, size_t nonce_len, const char *nc,
                        size_t nc_len, const char *cnonce, size_t cnonce_len,
                        const char *qop, size_t qop_len, char *resp )
{
    char ha2[33];
    unsigned char hash[16]; 
    MD5_CTX ctx;

    MD5_Init( &ctx );
    MD5_Update( &ctx, (const unsigned char *)method, method_len );
    MD5_Update( &ctx, (const unsigned char *)":", 1 );
    MD5_Update( &ctx, (const unsigned char *)uri, uri_len );
    MD5_Final( hash, &ctx );
    cs_to_hex( ha2, hash, sizeof( hash ) );
    
    MD5_Init( &ctx );
    MD5_Update( &ctx, (const unsigned char *)ha1, ha1_len ); 
    MD5_Update( &ctx, (const unsigned char *)":", 1 );
    MD5_Update( &ctx, (const unsigned char *)nonce, nonce_len );
    MD5_Update( &ctx, (const unsigned char *)":", 1 );
    MD5_Update( &ctx, (const unsigned char *)nc, nc_len ); 
    MD5_Update( &ctx, (const unsigned char *)":", 1 ); 
    MD5_Update( &ctx, (const unsigned char *)cnonce, cnonce_len );
    MD5_Update( &ctx, (const unsigned char *)":", 1 ); 
    MD5_Update( &ctx, (const unsigned char *)qop, qop_len );
    MD5_Update( &ctx, (const unsigned char *)":", 1 ); 
    MD5_Update( &ctx, (const unsigned char *)ha2, 32 );
    MD5_Final( hash, &ctx );
    cs_to_hex( ha2, hash, sizeof( hash ) );
}

///////////////////////////////////////////////////////////////////////////////
// vscp_is_authorized
//

static int vscp_is_authorized( struct mg_connection *conn,
                                struct http_message *hm,
                                CControlObject *gpobj ) {

    CUserItem *pUserItem;
    bool bValidHost;

    // Check pointers
    if ( NULL == hm ) return FALSE;
    if ( NULL == gpobj ) return 0;

    struct mg_str *hdr;
    char user[50], cnonce[50], response[40], uri[200], qop[20], nc[20], nonce[50];

    // Parse "authorisation:" header, fail fast on parse error
    if ( ( hdr = mg_get_http_header( hm, "Authorization" ) ) == NULL ||
        mg_http_parse_header(hdr, "username", user, sizeof( user ) ) == 0 ||
        mg_http_parse_header(hdr, "cnonce", cnonce, sizeof( cnonce ) ) == 0 ||
        mg_http_parse_header(hdr, "response", response, sizeof( response ) ) == 0 ||
        mg_http_parse_header(hdr, "uri", uri, sizeof( uri ) ) == 0 ||
        mg_http_parse_header(hdr, "qop", qop, sizeof( qop ) ) == 0 ||
        mg_http_parse_header(hdr, "nc", nc, sizeof( nc ) ) == 0 ||
        mg_http_parse_header(hdr, "nonce", nonce, sizeof( nonce ) ) == 0 ||
        vscp_check_nonce( nonce ) == 0 ) {
        return 0;
    }

    if ( !gpobj->m_bDisableSecurityWebServer ) {

        // Check if user is valid
        pUserItem = gpobj->m_userList.getUser( wxString::FromAscii( user ) );
        if ( NULL == pUserItem ) return 0;

        // Check if remote ip is valid
        gpobj->m_mutexUserList.Lock();
        bValidHost = pUserItem->isAllowedToConnect( wxString::FromAscii(
                (const char *)inet_ntoa( conn->sa.sin.sin_addr ) ) );
        gpobj->m_mutexUserList.Unlock();
        if ( !bValidHost ) {
            // Host wrong
            wxString strErr =
                wxString::Format( _( "[Webserver Client] Host [%s] NOT allowed to connect. User [%s]\n" ),
                wxString::FromAscii( ( const char * )inet_ntoa( conn->sa.sin.sin_addr ) ).wx_str(),
                                            pUserItem->getUser().mbc_str() );
                gpobj->logMsg( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_SECURITY );
                return 0;
        }

        char method[33];
        memset( method, 0, sizeof( method ) );
        strncpy( method, hm->method.p, hm->method.len );

        // Check digest
        if ( TRUE !=
            gpobj->getWebServer()->websrv_check_password( method,
                            ( const char *)pUserItem->getPasswordDomain().mbc_str(),
                            uri,
                            nonce,
                            nc,
                            cnonce,
                            qop,
                            response ) ) {

            // Username/password wrong
            wxString strErr =
                wxString::Format( _( "[Webserver Client] Host [%s] User [%s] NOT allowed to connect.\n" ),
                        wxString::FromAscii( ( const char * )inet_ntoa( conn->sa.sin.sin_addr ) ).wx_str(),
                        pUserItem->getUser().mbc_str() );
                gpobj->logMsg( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_SECURITY );
                return 0;
        }

    }

    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_is_websocket
//

static int vscp_is_websocket( const struct mg_connection *nc )
{
    return nc->flags & MG_F_IS_WEBSOCKET;
}


static void construct_etag(char *buf, size_t buf_len, const cs_stat_t *st) {
  snprintf(buf, buf_len, "\"%lx.%" INT64_FMT "\"", (unsigned long) st->st_mtime,
           (int64_t) st->st_size);
}
static void gmt_time_string(char *buf, size_t buf_len, time_t *t) {
  strftime(buf, buf_len, "%a, %d %b %Y %H:%M:%S GMT", gmtime(t));
}


///////////////////////////////////////////////////////////////////////////////
// websrv_event_handler
//

void VSCPWebServerThread::websrv_event_handler( struct mg_connection *nc,
                                                    int ev,
                                                    void *p )
{
    static time_t cleanupTime = time(NULL);
    mg_str *hdr;
    wxString strErr;
    struct websock_session *pWebSockSession;
    struct websrv_Session * pWebSrvSession;
    char uri[ 2048 ];
    char *pstr;

    char *cookie = NULL;

    // Check pointer
    if ( NULL == nc ) return;

    // Web socket data
    struct websocket_message *wm = (struct websocket_message *)p;

    //CControlObject *gpobj = (CControlObject *)nc->mgr->user_data;
    //if ( NULL == gpobj ) return;

    // Message
    struct http_message *phm = (struct http_message *)p;

    switch (ev) {

        case MG_EV_ACCEPT:  // New connection accepted. union socket_address
            break;

        case MG_EV_RECV:    // Data has been received. int *num_bytes
            break;

        case MG_EV_POLL:    // Sent to each connection on each mg_mgr_poll() call

            // Post incoming web socket events
            gpobj->getWebServer()->websock_post_incomingEvents();  // 1

            /*if ( ( cleanupTime - time(NULL) ) > 60 ) {
                gpobj->getWebServer()->websrv_expire_sessions( nc, phm );
                gpobj->getWebServer()->websock_expire_sessions( nc, phm );
                gpobj->getWebServer()->websrv_expire_rest_sessions( nc );
                cleanupTime = time(NULL);
            }*/
            break;

        case MG_EV_CLOSE:   // Connection is closed. NULL
            if ( vscp_is_websocket( nc ) ) {
                
                pWebSockSession = (struct websock_session *)nc->user_data;
                if ( NULL != pWebSockSession ) {
                    
                    // Remove client and session item
                    gpobj->m_wxClientMutex.Lock();
                    gpobj->removeClient( pWebSockSession->m_pClientItem );
                    pWebSockSession->m_pClientItem = NULL;
                    gpobj->m_wxClientMutex.Unlock();
            
                    // Remove the session object from the list and delete it
                    gpobj->m_websocketSessions.DeleteObject( pWebSockSession );                                      
                    nc->user_data = NULL;
                }
            }
            break;

        case MG_EV_HTTP_REQUEST:

            // Don't authorise here for rest calls
            if ( 0 == strncmp(phm->uri.p, "/vscp/rest", 10 ) ) {
                gpobj->getWebServer()->websrv_restapi( nc, phm );
                return;
            }
            else {
                // Must be authorised
                if ( !vscp_is_authorized( nc, phm, gpobj ) ) {
                    mg_printf( nc,
                                "HTTP/1.1 401 Unauthorized\r\n"
                                "WWW-Authenticate: Digest qop=\"auth\", "
                                "realm=\"%s\", nonce=\"%lu\"\r\n"
                                "Content-Length: 0\r\n\r\n",
                                gpobj->m_authDomain,
                                (unsigned long)time( NULL ) );
                    return;
                
                }
            }

            // Get Session object
            if ( NULL == ( pWebSrvSession = gpobj->getWebServer()->websrv_get_session( nc, phm ) ) ) {
                gpobj->getWebServer()->websrv_add_session_cookie( nc, phm );
                return;
            }

            // Log access
            strErr =
            wxString::Format( _("Webserver: Host=[%s] - req=[%s] query=[%s] method=[%s] \n"),
                                    wxString::FromUTF8((const char *)inet_ntoa( nc->sa.sin.sin_addr ) ).wx_str(),
                                    wxString::FromUTF8((const char *)phm->uri.p, phm->uri.len ).wx_str(),
                                    wxString::FromUTF8((const char *)phm->query_string.p, phm->query_string.len).wx_str(),
                                    wxString::FromUTF8((const char *)phm->method.p, phm->method.len).wx_str() );
            gpobj->logMsg ( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_ACCESS );
            
            memset( uri, 0, sizeof(uri) );
            strncpy( uri, phm->uri.p, phm->uri.len );
            pstr = vscp_trimWhiteSpace( uri );
            strcpy( uri, pstr );

            if ( ( 0 == strcmp( uri, "/vscp" ) ) || ( 0 == strcmp( uri, "/vscp/" ) ) ) {
                gpobj->getWebServer()->websrv_mainpage( nc, phm );
            }
            else if ( 0 == strcmp( uri, "/test") ) {
                mg_printf( nc, "HTTP/1.1 200 OK\r\n"
                                        "Transfer-Encoding: chunked\r\n"
                                        "Content-Type: text/html; charset=utf-8\r\n"
                                        "Cache-Control: max-age=0,  post-check=0, pre-check=0, no-store, no-cache, must-revalidate\r\n\r\nTEST!");
                mg_printf_http_chunk( nc, "%s", html_form );
                mg_send_http_chunk( nc, "", 0); // Tell the client we're finished
            }
            else if ( 0 == strcmp( uri, "/vscp/interfaces") ) {
                gpobj->getWebServer()->websrv_interfaces( nc, phm );
            }
            else if ( 0 == strcmp( uri, "/vscp/dm") ) {
                gpobj->getWebServer()->websrv_dmlist( nc, phm );
            }
            else if ( 0 == strcmp( uri, "/vscp/dmedit") ) {
                gpobj->getWebServer()->websrv_dmedit( nc, phm );
            }
            else if ( 0 == strcmp( uri, "/vscp/dmpost") ) {
                gpobj->getWebServer()->websrv_dmpost( nc, phm );
            }
            else if ( 0 == strcmp( uri, "/vscp/dmdelete") ) {
                gpobj->getWebServer()->websrv_dmdelete( nc, phm );
            }
            else if ( 0 == strcmp( uri, "/vscp/variables") ) {
                gpobj->getWebServer()->websrv_variables_list( nc, phm );
            }
            else if ( 0 == strcmp( uri, "/vscp/varedit") ) {
                gpobj->getWebServer()->websrv_variables_edit( nc, phm );
            }
            else if ( 0 == strcmp( uri, "/vscp/varpost") ) {
                gpobj->getWebServer()->websrv_variables_post( nc, phm );
            }
            else if ( 0 == strcmp( uri, "/vscp/vardelete") ) {
                gpobj->getWebServer()->websrv_variables_delete( nc, phm );
            }
            else if ( 0 == strcmp( uri, "/vscp/varnew") ) {
                gpobj->getWebServer()->websrv_variables_new( nc, phm );
            }
            else if ( 0 == strcmp( uri, "/vscp/discovery") ) {
                gpobj->getWebServer()->websrv_discovery( nc, phm );
            }
            else if ( 0 == strcmp( uri, "/vscp/session") ) {
                gpobj->getWebServer()->websrv_session( nc, phm );
            }
            else if ( 0 == strcmp( uri, "/vscp/configure") ) {
                gpobj->getWebServer()->websrv_configure( nc, phm );
            }
            else if ( 0 == strcmp( uri, "/vscp/bootload") ) {
                gpobj->getWebServer()->websrv_bootload( nc, phm );
            }
            else if ( 0 == strncmp( uri, "/vscp/table",12) ) {
                gpobj->getWebServer()->websrv_table( nc, phm );
            }
            else if ( 0 == strncmp( uri, "/vscp/tablelist",12) ) {
                gpobj->getWebServer()->websrv_tablelist( nc, phm );
            }
            else if ( 0 == strncmp( uri, "/vscp/rest",10) ) {
                gpobj->getWebServer()->websrv_restapi( nc, phm );
            }
            else {
                // Check if uri ends with ".vscp"
                if ( ( NULL != ( pstr = strstr( uri, ".vscp") ) ) && ( 0 == *(pstr+5))  ) {

                    FILE * pFile;
                    int code;

                    if ( NULL == ( pFile = fopen( "/tmp/test.vscp", "rb") ) ) {

                        switch ( errno ) {

                            case EACCES:
                                code = 403;
                                break;

                            case ENOENT:
                                code = 404;
                                break;

                            default:
                                code = 500;
                        }

                        //send_http_error(nc, code, "Open failed");

                    }
                    else {

                        char path[ MAX_PATH_SIZE ];
                        strcpy( path, "/tmp/test.vscp" );
                        char etag[50], current_time[50], last_modified[50], range[50];
                        time_t t = time(NULL);
                        int64_t r1 = 0, r2 = 0, cl = 0;
                        int status_code = 200;

                        // Get file size
                        fseek( pFile, 0, SEEK_END );
                        cl = ftell( pFile );
                        fseek( pFile, 0, SEEK_SET );

                        gmt_time_string( current_time,
                                            sizeof( current_time ),
                                            &t );
                        time_t modtime =  vscp_get_mtime( path );
                        gmt_time_string( last_modified,
                                            sizeof( last_modified ),
                                            &modtime );

                        mg_printf( nc,
                                    "HTTP/1.1 200 OK\r\n"
                                    "Transfer-Encoding: chunked\r\n"
                                    "Date: %s\r\n"
                                    "Last-Modified: %s\r\n"
                                    "Content-Type: text/html\r\n"
                                    "\r\n",
                                        current_time,
                                        last_modified );

                        char *p;
                        char *pTagContent = NULL;
                        char *pbuf = new char[ cl+1 ];
                        memset( pbuf, 0, sizeof( pbuf ) );
                        size_t nRead;

                        if ( ( nRead = fread( pbuf, 1, cl, pFile ) ) > 0 ) {

                            while ( true  ) {

                                if ( NULL != ( p = vscp_stristr( pbuf, "<?VSCP" ) ) ) {

                                    *p = 0;
                                    //*(p + 6) = 0;   // Terminate string
                                    pTagContent = p + 6;

                                    // Send this chunk
                                    mg_send_http_chunk( nc, pbuf, strlen( pbuf ) );

                                    // Copy down remaining content
                                    strcpy( pbuf, pTagContent );


                                    // Search end tag
                                    char *pEnd;
                                    if ( NULL != ( pEnd = vscp_stristr( pbuf, " ?>" ) ) ) {

                                        *pEnd = 0;      // Mark tag content end
                                        p = pEnd + 3;   // Point at next chunk
                                        // If we had a beginning tag - work on it
                                        if ( NULL != pbuf ) {

                                            vscp_trimWhiteSpace( pbuf );

                                            vscpEvent e;
                                            // Lets add some dummy information just for fun
                                            e.vscp_class = VSCP_CLASS1_MEASUREMENT;
                                            VSCP_TYPE_MEASUREMENT_TEMPERATURE;
                                            e.vscp_type = VSCP_TYPE_MEASUREMENT_TEMPERATURE;
                                            vscp_getGuidFromString( &e,
                                                    wxString::FromUTF8("FF:FF:FF:FF:FF:FF:FF:FE:00:16:D4:C0:00:02:00:01") );
                                            e.pdata[0] = 0x8A;
                                            e.pdata[1] = 0x82;
                                            e.pdata[2] = 0x0E;
                                            e.pdata[3] = 0x5D;
                                            e.sizeData = 4;
                                            dmElement dm;
                                            dm.m_pDM = &gpobj->m_dm; // Needed
                                            wxString wxstr = wxString::FromUTF8( pbuf );
                                            dm.handleEscapes( &e, wxstr );
                                            mg_send_http_chunk( nc, wxstr.mbc_str(), wxstr.Length() );
                                            pTagContent = NULL;  // Prepare for next tag

                                        }

                                        // Copy down remaining content
                                        strcpy( pbuf, p );

                                    }

                                }
                                else {
                                    // Send last chunk
                                    if ( strlen( pbuf ) ) {
                                        mg_send_http_chunk( nc, pbuf, strlen( pbuf ) );
                                    }
                                    break;  // We are done
                                }

                            }
                        }
                        else {
                            // Problems

                        }

                        delete [] pbuf;

                        fclose( pFile );

                        //mg_send_http_chunk( nc, buildPage.ToAscii(), buildPage.Length() );
                        mg_send_http_chunk( nc, "", 0); // Tell the client we're finished

                    }





                }
                // uri ends with ".lua"
                else if ( ( NULL != ( pstr = strstr( uri, ".lua") ) ) && ( 0 == *(pstr+4))  ) {
                    printf("2");
                }
                else {
                    // Server standard page
                    mg_serve_http( nc, phm, g_http_server_opts );

                }
            }
            break;

        case MG_EV_HTTP_REPLY:  // Will not happen as we are server
            break;

        case MG_EV_WEBSOCKET_FRAME:
            gpobj->getWebServer()->websrv_websocket_message( nc, phm, wm );
            break;

        case MG_EV_WEBSOCKET_HANDSHAKE_REQUEST:

            // New websocket connection. Create a session
            nc->user_data = gpobj->getWebServer()->websock_new_session( nc, phm );

            /*
                Currently it is impossible to request a specific protocol
                This is what would be needed to do so.

                if ( NULL != ( hdr = mg_get_http_header( nc, "Sec-WebSocket-Protocol") ) ) {
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

                         mg_send(nc, buf, strlen(buf));

                    }
                }
            */
            break;

        case MG_EV_WEBSOCKET_HANDSHAKE_DONE:

            // New websocket connection. Send connection ID back to the client.

            // Get session
            
            /*
            pWebSockSession = (struct websock_session *)nc->user_data;
            if ( NULL == pWebSockSession ) {
                mg_printf_websocket_frame( nc,
                                            WEBSOCKET_OP_TEXT,
                                            "-;%d;%s",
                                            WEBSOCK_ERROR_NOT_AUTHORISED,
                                            WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            }

            if ( gpobj->m_bAuthWebsockets ) {

                // Start authentication
                mg_printf_websocket_frame( nc,
                                            WEBSOCKET_OP_TEXT,
                                            "+;AUTH0;%s",
                                            pWebSockSession->m_sid );
            }
            else {
                // No authentication will be performed

                pWebSockSession->bAuthenticated = true;	// Authenticated
                mg_printf_websocket_frame( nc,
                                            WEBSOCKET_OP_TEXT,
                                            "+;AUTH1" );
            }*/
            break;

        case MG_EV_WEBSOCKET_CONTROL_FRAME:
            break;

        default:
            break;
    }
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
    struct mg_connection *nc;

    mg_mgr_init( &gmgr, m_pCtrlObject );
    nc = mg_bind( &gmgr,
                    m_pCtrlObject->m_strWebServerInterfaceAddress.mbc_str(),
                    VSCPWebServerThread::websrv_event_handler );

    // Unable to bind to port
    if ( NULL == nc ) {
#ifndef WIN32
        syslog( LOG_ERR,
                "Webserver: Could not use port %s (%s). Terminating\n",
                (const char *)m_pCtrlObject->m_strWebServerInterfaceAddress.mbc_str(),
                strerror( errno ) );
#endif
        wxString strErr =
                 wxString::Format( _("Webserver: Could not use port %s (%s). Terminating\n"),
                                        (const char *)m_pCtrlObject->m_strWebServerInterfaceAddress.mbc_str(),
                                        strerror( errno ) );
                m_pCtrlObject->logMsg ( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_GENERAL );

        return NULL;
    }

    // Set up HTTP server parameters
    mg_set_protocol_http_websocket( nc );

    // Set options

    // Path to web root
    g_http_server_opts.document_root = m_pCtrlObject->m_pathWebRoot;

    // Set index files to look for
    g_http_server_opts.index_files = m_pCtrlObject->m_indexFiles;

    // Authorization domain (domain name of this web server)
    g_http_server_opts.auth_domain = m_pCtrlObject->m_authDomain;

    // Enable/disable directory listings
    g_http_server_opts.enable_directory_listing =
                                    m_pCtrlObject->m_EnableDirectoryListings;

    // SSI files pattern. If not set, "**.shtml$|**.shtm$" is used.
    if ( strlen( m_pCtrlObject->m_ssi_pattern ) ) {
        g_http_server_opts.ssi_pattern =
                                        m_pCtrlObject->m_ssi_pattern;
    }

    // Comma-separated list of `uri_pattern=file_or_directory_path` rewrites.
    // When HTTP request is received, Mongoose constructs a file name from the
    // requested URI by combining `document_root` and the URI. However, if the
    // rewrite option is used and `uri_pattern` matches requested URI, then
    // `document_root` is ignored. Instead, `file_or_directory_path` is used,
    // which should be a full path name or a path relative to the web server's
    // current working directory. Note that `uri_pattern`, as all Mongoose
    // patterns, is a prefix pattern.
    //
    // If uri_pattern starts with `@` symbol, then Mongoose compares it with the
    // HOST header of the request. If they are equal, Mongoose sets document root
    // to `file_or_directory_path`, implementing virtual hosts support.
    if ( strlen( m_pCtrlObject->m_urlRewrites ) ) {
        g_http_server_opts.url_rewrites =
                                        m_pCtrlObject->m_urlRewrites;
    }

    // DAV document root. If NULL, DAV requests are going to fail.
    if ( strlen( m_pCtrlObject->m_dav_document_root ) ) {
        g_http_server_opts.dav_document_root =
                                        m_pCtrlObject->m_dav_document_root;
    }

    // Glob pattern for the files to hide.
    if ( strlen( m_pCtrlObject->m_hideFilePatterns ) ) {
        g_http_server_opts.hidden_file_pattern =
                                        m_pCtrlObject->m_hideFilePatterns;
    }

    // Set to non-NULL to enable CGI, e.g. **.cgi$|**.php$"
    if ( strlen( m_pCtrlObject->m_cgiPattern ) ) {
        g_http_server_opts.cgi_file_pattern =
                                        m_pCtrlObject->m_cgiPattern;
    }

    // If not NULL, ignore CGI script hashbang and use this interpreter
    if ( strlen( m_pCtrlObject->m_cgiInterpreter ) ) {
        g_http_server_opts.cgi_interpreter =
                                        m_pCtrlObject->m_cgiInterpreter;
    }

    // Comma-separated list of Content-Type overrides for path suffixes, e.g.
    // * ".txt=text/plain; charset=utf-8,.c=text/plain"
    if ( strlen( m_pCtrlObject->m_extraMimeTypes ) ) {
        g_http_server_opts.custom_mime_types =
                                        m_pCtrlObject->m_extraMimeTypes;
    }

    // If security is enabled
    if ( !m_pCtrlObject->m_bDisableSecurityWebServer ) {

        if ( strlen( m_pCtrlObject->m_per_directory_auth_file ) ) {
            g_http_server_opts.per_directory_auth_file =
                                        m_pCtrlObject->m_per_directory_auth_file;
        }

        if  ( strlen( m_pCtrlObject->m_global_auth_file ) ) {
            g_http_server_opts.global_auth_file =
                                        m_pCtrlObject->m_global_auth_file;
        }

        // IP ACL. By default, NULL, meaning all IPs are allowed to connect
        if ( strlen( m_pCtrlObject->m_ip_acl ) ) {
            g_http_server_opts.ip_acl = m_pCtrlObject->m_ip_acl;
        }

    }

    while ( !TestDestroy() && !m_bQuit ) {

        // CLOCKS_PER_SEC
        oldus = ticks = clock();

#ifndef WIN32
        struct timeval tv;
        gettimeofday(&tv, NULL);
#endif

        mg_mgr_poll( &gmgr, 50 );

#ifdef WIN32
            oldus = clock();
#else
            oldus = tv.tv_usec;
#endif

    }

    // Kill web server
    mg_mgr_free( &gmgr );

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

    //CControlObject *gpobj = (CControlObject *)nc->mgr->user_data;
    //if (NULL == gpobj) return NULL;

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

    MD5_CTX ctx;
    MD5_Init( &ctx );
    MD5_Update( &ctx, (const unsigned char *)buf, strlen( buf ) );
    unsigned char bindigest[16];
    MD5_Final( bindigest, &ctx );
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

    //CControlObject *gpobj = (CControlObject *)nc->mgr->user_data;
    //if (NULL == gpobj) return NULL;

    if ( NULL == ( rv =  gpobj->getWebServer()->websrv_get_session( nc, hm ) ) ) {

        if ( NULL == ( pheader = mg_get_http_header( hm, "Authorization") ) ||
                        ( vscp_strncasecmp( pheader->p, "Digest ", 7 ) != 0 ) ) {
            return NULL;
        }

        if (!mg_http_parse_header(pheader, "username", user, sizeof(user))) {
            return NULL;
        }

        // Add session cookie
        rv = gpobj->getWebServer()->websrv_add_session_cookie( nc, hm );
    }

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
    char ha2[33], expected_response[33];

#if 0
    // Check for authentication timeout
    if ( ( (unsigned long)time(NULL) - (unsigned long)to64( nonce ) ) > 3600 * 2) {
        return 0;
    }
#endif

    MD5_CTX ctx;
    unsigned char hash[16];

    MD5_Init( &ctx );
    MD5_Update( &ctx, (const unsigned char *)method, strlen( method ) );
    MD5_Update( &ctx, (const unsigned char *)":", 1 );
    MD5_Update( &ctx, (const unsigned char *)uri, strlen( uri ) );
    MD5_Final( hash, &ctx );
    cs_to_hex( ha2, hash, sizeof( hash ) );
    
    MD5_Init( &ctx );
    MD5_Update( &ctx, (const unsigned char *)ha1, 32 );
    MD5_Update( &ctx, (const unsigned char *)":", 1 );
    MD5_Update( &ctx, (const unsigned char *)nonce, strlen( nonce ) );
    MD5_Update( &ctx, (const unsigned char *)":", 1 );
    MD5_Update( &ctx, (const unsigned char *)nc, strlen( nc ) );
    MD5_Update( &ctx, (const unsigned char *)":", 1 );
    MD5_Update( &ctx, (const unsigned char *)cnonce, strlen( cnonce ) );
    MD5_Update( &ctx, (const unsigned char *)":", 1 );
    MD5_Update( &ctx, (const unsigned char *)qop, strlen( qop ) );
    MD5_Update( &ctx, (const unsigned char *)":", 1 );
    MD5_Update( &ctx, (const unsigned char *)ha2, 32 );
    MD5_Final( hash, &ctx );
    cs_to_hex( expected_response, hash, sizeof( hash ) );

    return ( vscp_strcasecmp( response, expected_response ) == 0 ) ? TRUE : FALSE;

}


///////////////////////////////////////////////////////////////////////////////
// websrv_listFile
//

void VSCPWebServerThread::websrv_listFile( struct mg_connection *nc,
                                            wxFileName& logfile,
                                            wxString& textHeader )
{
    mg_printf( nc, "HTTP/1.1 200 OK\r\n"
                        "Transfer-Encoding: chunked\r\n"
                        "Content-Type: text/html; charset=utf-8\r\n"
                        "Cache-Control: max-age=0,  "
                        "post-check=0, pre-check=0, "
                        "no-store, no-cache, must-revalidate\r\n\r\n");

    bool bFirstRow = false;
    wxString buildPage;
    wxString strHeader = wxString::Format(_("VSCP - %s"), textHeader.wx_str() );
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), strHeader.wx_str() );
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);

    // Navigation menu
    buildPage += _(WEB_COMMON_MENU);

    buildPage += wxString::Format( _("<b>%s</b><br><br>"), textHeader.wx_str() );
    buildPage += _("<b>Path</b>=<i>");
    buildPage += logfile.GetFullPath();
    buildPage += _("</i><br>");
    buildPage += _("-----------------------------------------------------------------------------------------<br>");

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
    mg_send_http_chunk( nc, buildPage.ToAscii(), buildPage.Length() );
    mg_send_http_chunk( nc, "", 0); // Tell the client we're finished

    return;
}



///////////////////////////////////////////////////////////////////////////////
// websrv_mainpage
//

void
VSCPWebServerThread::websrv_mainpage( struct mg_connection *nc,
                                        struct http_message *hm )
{
    wxString strHost;

    // Check pointer
    if (NULL == nc) return;

    //CControlObject *gpobj = (CControlObject *)nc->mgr->user_data;
    //if (NULL == gpobj) return;

    // Get hostname
    mg_str *mgstr = mg_get_http_header( hm, "Host" ); // nc->local_ip; //_("http://localhost:8080");
    strHost.FromUTF8( mgstr->p, mgstr->len );

    wxString buildPage;

    mg_printf( nc, "HTTP/1.1 200 OK\r\n"
                    "Transfer-Encoding: chunked\r\n"
                    "Content-Type: text/html; charset=utf-8\r\n"
                    "Cache-Control: max-age=0,  post-check=0, pre-check=0, no-store, no-cache, must-revalidate\r\n\r\n");

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
    mg_send_http_chunk( nc, buildPage.ToAscii(), buildPage.Length() );
    mg_send_http_chunk( nc, "", 0 );

    return;
}


///////////////////////////////////////////////////////////////////////////////
// websrv_interfaces
//

void
VSCPWebServerThread::websrv_interfaces( struct mg_connection *nc,
                                            struct http_message *hm  )
{
    // Check pointer
    if (NULL == nc) return;

    //CControlObject *gpobj = (CControlObject *)nc->mgr->user_data;
    //if (NULL == gpobj) return;

    mg_printf( nc,
        "HTTP/1.1 200 OK\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Cache-Control: max-age=0,  "
        "post-check=0, pre-check=0, "
        "no-store, no-cache, must-revalidate\r\n\r\n");

    wxString buildPage;
    //_send_status( nc, 200 );
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
    gpobj->m_wxClientMutex.Lock();
    VSCPCLIENTLIST::iterator iter;
    for (iter = gpobj->m_clientList.m_clientItemList.begin();
            iter != gpobj->m_clientList.m_clientItemList.end();
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

    gpobj->m_wxClientMutex.Unlock();

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
    mg_send_http_chunk( nc, buildPage.ToAscii(), buildPage.Length() );
    mg_send_http_chunk( nc, "", 0 );

    return;
}



///////////////////////////////////////////////////////////////////////////////
// websrv_dmlist
//

void
VSCPWebServerThread::websrv_dmlist( struct mg_connection *nc,
                                        struct http_message *hm  )
{
    char buf[80];
    VSCPInformation vscpinfo;

    // Check pointer
    if (NULL == nc) return;

    //CControlObject *gpobj = (CControlObject *)nc->mgr->user_data;
    //if (NULL == gpobj) return;

    // light
    bool bLight = false;
    *buf = 0;
    if ( mg_get_http_var( &hm->query_string, "light", buf, sizeof( buf ) ) <= 0 ) {
        if ( strlen( buf ) && ( NULL != strstr( "true", buf ) ) ) bLight = true;
    }

    // From
    long nFrom = 0;
    *buf = 0;
    if ( mg_get_http_var( &hm->query_string, "from", buf, sizeof( buf ) ) > 0 ) {
        nFrom = atoi( buf );
    }

    // Count
    *buf = 0;
    uint16_t nCount = 50;
    if ( mg_get_http_var( &hm->query_string, "count", buf, sizeof( buf ) ) > 0 ) {
        nCount = atoi( buf );
    }

    // Navigation button
    *buf = 0;
    if ( mg_get_http_var( &hm->query_string, "navbtn", buf, sizeof( buf ) ) > 0 ) {

        if (NULL != strstr("previous", buf) ) {
            nFrom -= nCount;
            if ( nFrom < 0 )  nFrom = 0;
        }
        else if (NULL != strstr("next",buf)) {
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
    else {  // No vaid navigation value
        //nFrom = 0;
    }

    mg_printf( nc,
        "HTTP/1.1 200 OK\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Cache-Control: max-age=0,  "
        "post-check=0, pre-check=0, "
        "no-store, no-cache, must-revalidate\r\n\r\n");

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
        wxString wxstrlight = ((bLight) ? _("true") : _("false"));
        buildPage += wxString::Format( _(WEB_COMMON_LIST_NAVIGATION),
                _("/vscp/dm"),
                (unsigned long)nFrom+1,
                ( (unsigned long)(nFrom + nCount) < gpobj->m_dm.getMemoryElementCount()) ?
                    nFrom + nCount : gpobj->m_dm.getMemoryElementCount(),
                (unsigned long)gpobj->m_dm.getMemoryElementCount(),
                (unsigned long)nCount,
                (unsigned long)nFrom,
                (const char *)wxstrlight.mbc_str() );

        buildPage += _("<br>");
    }

    wxString strGUID;
    wxString strBuf;

    // Display DM List

    if ( 0 == gpobj->m_dm.getMemoryElementCount() ) {
        buildPage += _("<br>Decision Matrix is empty!<br>");
    }
    else {
        buildPage += _(WEB_DMLIST_TR_HEAD);
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
            wxString str = wxString::Format(_(WEB_COMMON_TR_CLICKABLE_ROW),
                                                url_dmedit.wx_str() );
            buildPage += str;
        }

        // Client id
        buildPage += _(WEB_IFLIST_TD_CENTERED);
        buildPage += wxString::Format(_("<form name=\"input\" action=\"/vscp/dmdelete?id=%ld\" method=\"get\"> %ld <input type=\"submit\" value=\"x\"><input type=\"hidden\" name=\"id\"value=\"%ld\"></form>"),
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

            // Control - Check index
            if (pElement->isCheckIndexSet()) {
                if (pElement->m_bCheckMeasurementIndex) {
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

                if ( pElement->m_bCheckMeasurementIndex ) {
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
            buildPage += wxString::Format(_("%d "), pElement->m_actionCode);

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
        wxString wxstrlight = ((bLight) ? _("true") : _("false"));
        buildPage += wxString::Format( _(WEB_COMMON_LIST_NAVIGATION),
                _("/vscp/dm"),
                (unsigned long)nFrom+1,
                ( (unsigned long)(nFrom + nCount) < gpobj->m_dm.getMemoryElementCount()) ?
                    nFrom + nCount : gpobj->m_dm.getMemoryElementCount(),
                (unsigned long)gpobj->m_dm.getMemoryElementCount(),
                (unsigned long)nCount,
                (unsigned long)nFrom,
                wxstrlight.wx_str() );
    }

    buildPage += _(WEB_COMMON_END);     // Common end code

    // Server data
    mg_send_http_chunk( nc, buildPage.ToAscii(), buildPage.Length() );
    mg_send_http_chunk( nc, "", 0 );

    return;
}

///////////////////////////////////////////////////////////////////////////////
// websrv_dmedit
//

void
VSCPWebServerThread::websrv_dmedit( struct mg_connection *nc,
                                        struct http_message *hm  )
{
    char buf[80];
    wxString str;
    VSCPInformation vscpinfo;
    dmElement *pElement = NULL;

    // Check pointer
    if (NULL == nc) return;

    //CControlObject *gpobj = (CControlObject *)nc->mgr->user_data;
    //if (NULL == gpobj) return;

    // id
    long id = -1;
    if ( mg_get_http_var( &hm->query_string, "id", buf, sizeof( buf ) ) > 0 ) {
        id = atoi(buf);
    }

    // From
    long nFrom = 0;
    if ( mg_get_http_var( &hm->query_string, "from", buf, sizeof( buf ) ) > 0 ) {
        nFrom = atoi( buf );
    }

    // Count
    uint16_t nCount = 50;
    if ( mg_get_http_var( &hm->query_string, "count", buf, sizeof( buf ) ) > 0 ) {
        nCount = atoi( buf );
    }

    // Flag for new DM row
    bool bNew = false;
    if ( mg_get_http_var( &hm->query_string, "new", buf, sizeof( buf ) ) > 0 ) {
        if ( NULL != strstr( "true", buf ) ) bNew = true;
    }

    mg_printf( nc,
        "HTTP/1.1 200 OK\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Cache-Control: max-age=0,  "
        "post-check=0, pre-check=0, "
        "no-store, no-cache, must-revalidate\r\n\r\n");

    wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Decision Matrix Edit"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);

    buildPage += _(WEB_COMMON_MENU);;
    buildPage += _(WEB_DMEDIT_BODY_START);

    if ( !bNew && id < gpobj->m_dm.getMemoryElementCount() ) {
        pElement = gpobj->m_dm.getMemoryElementFromRow(id);
    }

    if (bNew || (NULL != pElement)) {

        if ( bNew ) {
            buildPage += _("<span id=\"optiontext\">New record.</span><br>");
        }
        else {
            buildPage += wxString::Format(_("<span id=\"optiontext\">Record = %ld.</span><br>"), id);
        }

        buildPage += _("<br><form method=\"get\" action=\"");
        buildPage += _("/vscp/dmpost");
        buildPage += _("\" name=\"dmedit\">");

        buildPage += wxString::Format(_("<input name=\"id\" value=\"%ld\" type=\"hidden\"></input>"), id );


        if (bNew) {

            buildPage += _("<input name=\"new\" value=\"true\" type=\"hidden\"></input>");

            long nFrom;
             if ( gpobj->m_dm.getMemoryElementCount() % nCount ) {
                nFrom = (gpobj->m_dm.getMemoryElementCount()/nCount)*nCount;
            }
            else {
                nFrom = ((gpobj->m_dm.getMemoryElementCount()/nCount) - 1)*nCount;
            }

            buildPage += wxString::Format( _("<input name=\"from\" value=\"%ld\" type=\"hidden\">"), (long)nFrom );
            buildPage += wxString::Format( _("<input name=\"count\" value=\"%ld\" type=\"hidden\">"), (long)nCount );

        }
        else {
            // Hidden from
            buildPage += wxString::Format(_("<input name=\"from\" value=\"%ld\" type=\"hidden\">"), (long)nFrom );
            // Hidden count
            buildPage += wxString::Format(_("<input name=\"count\" value=\"%ld\" type=\"hidden\">"), (long)nCount );
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
                str.wx_str() );

        if ( !bNew ) str = (1 == pElement->m_vscpfilter.filter_priority) ? _("selected") : _(" ");
        buildPage += wxString::Format(_("<option value=\"1\" %s>1 - Very High</option>"),
                str.wx_str() );

        if ( !bNew ) str = (2 == pElement->m_vscpfilter.filter_priority) ? _("selected") : _(" ");
        buildPage += wxString::Format(_("<option value=\"2\" %s>2 - High</option>"),
                str.wx_str() );

        if ( !bNew ) str = (3 == pElement->m_vscpfilter.filter_priority) ? _("selected") : _(" ");
        buildPage += wxString::Format(_("<option value=\"3\" %s>3 - Normal</option>"),
                str.wx_str() );

        if ( !bNew ) str = (4 == pElement->m_vscpfilter.filter_priority) ? _("selected") : _(" ");
        buildPage += wxString::Format(_("<option value=\"4\" %s>4 - Low</option>"),
                str.wx_str() );

        if ( !bNew ) str = (5 == pElement->m_vscpfilter.filter_priority) ? _("selected") : _(" ");
        buildPage += wxString::Format(_("<option value=\"5\" %s>5 - Lower</option>"),
                str.wx_str() );

        if ( !bNew ) str = (6 == pElement->m_vscpfilter.filter_priority) ? _("selected") : _(" ");
        buildPage += wxString::Format(_("<option value=\"6\" %s>6 - Very Low</option>"),
                str.wx_str() );

        if ( !bNew ) str = (7 == pElement->m_vscpfilter.filter_priority) ? _("selected") : _(" ");
        buildPage += wxString::Format(_("<option value=\"7\" %s>7 - Lowest</option>"),
                str.wx_str() );

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
            buildPage += wxString::Format(_("%s"), str.wx_str()  );
        }
        buildPage += _("</textarea></td>");

        if ( !bNew ) vscp_writeGuidArrayToString( pElement->m_vscpfilter.mask_GUID, str );
        buildPage += _("<tr class=\"invisable\"><td class=\"invisable\"> </td><td class=\"invisable\"><textarea style=\"background-color: #72A4D2;\" cols=\"50\" rows=\"1\" name=\"mask_vscpguid\">");
        if ( bNew ) {
            buildPage += _("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00");
        }
        else {
            buildPage += wxString::Format(_("%s"), str.wx_str()  );
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
                pElement->m_bCheckMeasurementIndex ? _("checked") : _("") );
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
            str = (0x10 == pElement->m_actionCode ) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x10\" %s>Execute external program</option>"),
                str.wx_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x12 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x12\" %s>Execute internal procedure</option>"),
                str.wx_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x30 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x30\" %s>Execute library procedure</option>"),
                str.wx_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x40 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x40\" %s>Send event</option>"),
                str.wx_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x41 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x41\" %s>Send event Conditional</option>"),
                str.wx_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x42 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x42\" %s>Send event(s) from file</option>"),
                str.wx_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x43 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x43\" %s>Send event(s) to remote VSCP server</option>"),
                str.wx_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x50 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x50\" %s>Store in variable</option>"),
                str.wx_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x51 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x51\" %s>Store in array</option>"),
                str.wx_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x52 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x52\" %s>Add to variable</option>"),
                str.wx_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x53 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x53\" %s>Subtract from variable</option>"),
                str.wx_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x54 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x54\" %s>Multiply variable</option>"),
                str.wx_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x55 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x55\" %s>Divide variable</option>"),
                str.wx_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x60 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x60\" %s>Start timer</option>"),
                str.wx_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x61 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x61\" %s>Pause timer</option>"),
                str.wx_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x62 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x62\" %s>Stop timer</option>"),
                str.wx_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x63 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x63\" %s>Resume timer</option>"),
                str.wx_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x70 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x70\" %s>Write file</option>"),
                str.wx_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x75 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x75\" %s>Get/Put/Post URL</option>"),
                str.wx_str() );

        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x80 == pElement->m_actionCode) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x80\" %s>Write to table</option>"),
                str.wx_str() );

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
    mg_send_http_chunk( nc, buildPage.ToAscii(), buildPage.Length() );
    mg_send_http_chunk( nc, "", 0 );

    return;
}


///////////////////////////////////////////////////////////////////////////////
// websrv_dmpost
//

void
VSCPWebServerThread::websrv_dmpost( struct mg_connection *nc,
                                        struct http_message *hm  )
{
    char buf[32000];
    wxString str;
    VSCPInformation vscpinfo;
    dmElement *pElement = NULL;

    // Check pointer
    if (NULL == nc) return;

    //CControlObject *gpobj = (CControlObject *)nc->mgr->user_data;
    //if (NULL == gpobj) return;

    // id
    long id = -1;
    if ( mg_get_http_var( &hm->query_string, "id", buf, sizeof( buf ) ) > 0 ) {
        id = atoi( buf );
    }

    // From
    long nFrom = 0;
    if ( mg_get_http_var( &hm->query_string, "from", buf, sizeof( buf ) ) > 0 ) {
        nFrom = atoi( buf );
    }

    // Count
    uint16_t nCount = 50;
    if ( mg_get_http_var( &hm->query_string, "count", buf, sizeof( buf ) ) > 0 ) {
        nCount = atoi( buf );
    }

    // Flag for new DM row
    bool bNew = false;
    if ( mg_get_http_var( &hm->query_string, "new", buf, sizeof( buf ) ) > 0 ) {
        if ( NULL != strstr( "true", buf ) ) bNew = true;
    }

    wxString strGroupID;
    if ( mg_get_http_var( &hm->query_string, "groupid", buf, sizeof( buf ) ) > 0 ) {
        strGroupID = wxString::FromAscii(buf);
    }

    int filter_priority = -1;
    if ( mg_get_http_var( &hm->query_string, "filter_priority", buf, sizeof( buf ) ) > 0 ) {
        filter_priority = vscp_readStringValue( wxString::FromAscii( buf ) );
    }

    int mask_priority = 0;
    if ( mg_get_http_var( &hm->query_string, "mask_priority", buf, sizeof( buf ) ) > 0 ) {
        mask_priority = vscp_readStringValue( wxString::FromAscii( buf ) );
    }

    uint16_t filter_vscpclass = -1;
    if ( mg_get_http_var( &hm->query_string, "filter_vscpclass", buf, sizeof( buf ) ) > 0 ) {
        wxString wrkstr = wxString::FromAscii( buf );
        filter_vscpclass = vscp_readStringValue( wrkstr );
    }

    uint16_t mask_vscpclass = 0;
    if ( mg_get_http_var( &hm->query_string, "mask_vscpclass", buf, sizeof( buf ) ) > 0 ) {
        mask_vscpclass = vscp_readStringValue( wxString::FromAscii( buf ) );
    }

    uint16_t filter_vscptype = 0;
    if ( mg_get_http_var( &hm->query_string, "filter_vscptype", buf, sizeof( buf ) ) > 0 ) {
        filter_vscptype = vscp_readStringValue( wxString::FromAscii( buf ) );
    }

    uint16_t mask_vscptype = 0;
    if ( mg_get_http_var( &hm->query_string, "mask_vscptype", buf, sizeof( buf ) ) > 0 ) {
        mask_vscptype = vscp_readStringValue( wxString::FromAscii( buf ) );
    }

    wxString strFilterGuid;
    if ( mg_get_http_var( &hm->query_string, "filter_vscpguid", buf, sizeof( buf ) ) > 0 ) {
        strFilterGuid = wxString::FromAscii( buf );
        strFilterGuid = strFilterGuid.Trim();
        strFilterGuid = strFilterGuid.Trim(false);
    }

    wxString strMaskGuid;
    if ( mg_get_http_var( &hm->query_string, "mask_vscpguid", buf, sizeof( buf ) ) > 0 ) {
        strMaskGuid = wxString::FromAscii( buf );
        strMaskGuid = strMaskGuid.Trim();
        strMaskGuid = strMaskGuid.Trim(false);
    }

    uint8_t index = 0;
    if ( mg_get_http_var( &hm->query_string, "vscpindex", buf, sizeof( buf ) ) > 0 ) {
        index = vscp_readStringValue( wxString::FromAscii( buf ) );
    }

    bool bUseMeasurementIndex = false;
    if ( mg_get_http_var( &hm->query_string, "check_measurementindex", buf, sizeof( buf ) ) > 0 ) {
        if ( NULL != strstr( "true", buf ) ) bUseMeasurementIndex = true;
    }

    uint8_t zone = 0;
    if ( mg_get_http_var( &hm->query_string, "vscpzone", buf, sizeof( buf ) ) > 0 ) {
        zone = vscp_readStringValue( wxString::FromAscii( buf ) );
    }

    uint8_t subzone = 0;
    if ( mg_get_http_var( &hm->query_string, "vscpsubzone", buf, sizeof( buf ) ) > 0 ) {
        subzone = vscp_readStringValue( wxString::FromAscii( buf ) );
    }

    bool bEnableRow = false;
    if ( mg_get_http_var( &hm->query_string, "check_enablerow", buf, sizeof( buf ) ) > 0 ) {
        if ( NULL != strstr( "true", buf ) ) bEnableRow = true;
    }

    bool bEndScan = false;
    if ( mg_get_http_var( &hm->query_string, "check_endscan", buf, sizeof( buf ) ) > 0 ) {
        if ( NULL != strstr( "true", buf ) ) bEndScan = true;
    }

    bool bCheckIndex = false;
    if ( mg_get_http_var( &hm->query_string, "check_index", buf, sizeof( buf ) ) > 0 ) {
        if ( NULL != strstr( "true", buf ) ) bCheckIndex = true;
    }

    bool bCheckZone = false;
    if ( mg_get_http_var( &hm->query_string, "check_zone", buf, sizeof( buf ) ) > 0 ) {
        if ( NULL != strstr( "true", buf ) ) bCheckZone = true;
    }

    bool bCheckSubZone = false;
    if ( mg_get_http_var( &hm->query_string, "check_subzone", buf, sizeof( buf ) ) > 0 ) {
        if ( NULL != strstr( "true", buf ) ) bCheckSubZone = true;
    }

    wxString strAllowedFrom;
    if ( mg_get_http_var( &hm->query_string, "allowedfrom", buf, sizeof( buf ) ) > 0 ) {
        strAllowedFrom = wxString::FromAscii( buf );
        strAllowedFrom.Trim( true );
        strAllowedFrom.Trim( false );
        if ( _("*") == strAllowedFrom ) {
            strAllowedFrom = _("0000-01-01 00:00:00");
        }
    }

    wxString strAllowedTo;
    if ( mg_get_http_var( &hm->query_string, "allowedto", buf, sizeof( buf ) ) > 0 ) {
        strAllowedTo = wxString::FromAscii( buf );
        strAllowedTo.Trim( true );
        strAllowedTo.Trim( false );
        if ( _("*") == strAllowedTo ) {
            strAllowedTo = _("9999-12-31 23:59:59");
        }
    }

    wxString strAllowedTime;
    if ( mg_get_http_var( &hm->query_string, "allowedtime", buf, sizeof( buf ) ) > 0 ) {
        strAllowedTime = wxString::FromAscii( buf );
        strAllowedTime.Trim( true );
        strAllowedTime.Trim( false );
        if ( _("*") == strAllowedTime ) {
            strAllowedTime = _("* *");
        }
    }

    bool bCheckMonday = false;
    if ( mg_get_http_var( &hm->query_string, "monday", buf, sizeof( buf ) ) > 0 ) {
        if ( NULL != strstr( "true", buf ) ) bCheckMonday = true;
    }

    bool bCheckTuesday = false;
    if ( mg_get_http_var( &hm->query_string, "tuesday", buf, sizeof( buf ) ) > 0 ) {
        if ( NULL != strstr( "true", buf ) ) bCheckTuesday = true;
    }

    bool bCheckWednesday = false;
    if ( mg_get_http_var( &hm->query_string, "wednesday", buf, sizeof( buf ) ) > 0 ) {
        if ( NULL != strstr( "true", buf ) ) bCheckWednesday = true;
    }

    bool bCheckThursday = false;
    if ( mg_get_http_var( &hm->query_string, "thursday", buf, sizeof( buf ) ) > 0 ) {
        if ( NULL != strstr( "true", buf ) ) bCheckThursday = true;
    }

    bool bCheckFriday = false;
    if ( mg_get_http_var( &hm->query_string, "friday", buf, sizeof( buf ) ) > 0 ) {
        if ( NULL != strstr( "true", buf ) ) bCheckFriday = true;
    }

    bool bCheckSaturday = false;
    if ( mg_get_http_var( &hm->query_string, "saturday", buf, sizeof( buf ) ) > 0 ) {
        if ( NULL != strstr( "true", buf ) ) bCheckSaturday = true;
    }

    bool bCheckSunday = false;
    if ( mg_get_http_var( &hm->query_string, "sunday", buf, sizeof( buf ) ) > 0 ) {
        if ( NULL != strstr( "true", buf ) ) bCheckSunday = true;
    }

    uint32_t action = 0;
    if ( mg_get_http_var( &hm->query_string, "action", buf, sizeof( buf ) ) > 0 ) {
        action = vscp_readStringValue( wxString::FromAscii( buf ) );
    }

    wxString strActionParameter;
    if ( mg_get_http_var( &hm->query_string, "actionparameter", buf, sizeof( buf ) ) > 0 ) {
        vscp_toXMLEscape( buf );
        strActionParameter = wxString::FromAscii( buf );
    }

    wxString strComment;
    if ( mg_get_http_var( &hm->query_string, "comment", buf, sizeof( buf ) ) > 0 ) {
        vscp_toXMLEscape( buf );
        strComment = wxString::FromAscii( buf );
    }

    mg_printf( nc,
        "HTTP/1.1 200 OK\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Cache-Control: max-age=0,  "
        "post-check=0, pre-check=0, "
        "no-store, no-cache, must-revalidate\r\n\r\n");

    wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Decision Matrix Post"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += _("<meta http-equiv=\"refresh\" content=\"2;url=/vscp/dm");
    buildPage += wxString::Format(_("?from=%ld&count=%ld"), (long)nFrom, (long)nCount );
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

                buildPage += wxString::Format(_("<br><br>DM Entry has been saved. id=%d"), id);
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
    mg_send_http_chunk( nc, buildPage.ToAscii(), buildPage.Length() );
    mg_send_http_chunk( nc, "", 0 );

    return;
}


///////////////////////////////////////////////////////////////////////////////
// websrv_dmdelete
//

void
VSCPWebServerThread::websrv_dmdelete( struct mg_connection *nc,
                                        struct http_message *hm  )
{
    char buf[80];
    wxString str;
    VSCPInformation vscpinfo;

    // Check pointer
    if (NULL == nc) return;

    //CControlObject *gpobj = (CControlObject *)nc->mgr->user_data;
    //if (NULL == gpobj) return;

    // id
    long id = -1;
    if ( mg_get_http_var( &hm->query_string, "id", buf, sizeof( buf ) ) > 0 ) {
        id = atoi( buf );
    }

    // From
    long nFrom = 0;
    if ( mg_get_http_var( &hm->query_string, "from", buf, sizeof( buf ) ) > 0 ) {
        nFrom = atoi( buf );
    }

    // Count
    uint16_t nCount = 50;
    if ( mg_get_http_var( &hm->query_string, "count", buf, sizeof( buf ) ) > 0 ) {
        nCount = atoi( buf );
    }

    mg_printf( nc,
        "HTTP/1.1 200 OK\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Cache-Control: max-age=0,  "
        "post-check=0, pre-check=0, "
        "no-store, no-cache, must-revalidate\r\n\r\n");

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

    if ( gpobj->m_dm.removeMemoryElement( id ) ) {
        buildPage += wxString::Format(_("<br>Deleted record id = %d"), id);
        // Save decision matrix
        gpobj->m_dm.saveToXML();
    }
    else {
        buildPage += wxString::Format(_("<br>Failed to remove record id = %d"), id);
    }

    buildPage += _(WEB_COMMON_END);     // Common end code

    // Server data
    mg_send_http_chunk( nc, buildPage.ToAscii(), buildPage.Length() );
    mg_send_http_chunk( nc, "", 0 );

    return;
}


///////////////////////////////////////////////////////////////////////////////
// websrv_variables_list
//

void
VSCPWebServerThread::websrv_variables_list( struct mg_connection *nc,
                                        struct http_message *hm  )
{
    char buf[80];
    VSCPInformation vscpinfo;
    //unsigned long upperLimit = 50;

    // Check pointer
    if (NULL == nc) return;

    //CControlObject *gpobj = (CControlObject *)nc->mgr->user_data;
    //if (NULL == gpobj) return;

    // From
    long nFrom = 0;
    if ( mg_get_http_var( &hm->query_string, "from", buf, sizeof( buf ) ) > 0 ) {
        nFrom = atoi( buf );
    }


    // Count
    unsigned long nCount = 50;
    if ( mg_get_http_var( &hm->query_string, "count", buf, sizeof( buf ) ) > 0 ) {
        nCount = atoi( buf );
    }


    // Navigation button
    if ( mg_get_http_var( &hm->query_string, "navbtn", buf, sizeof( buf ) ) > 0 ) {

        if (NULL != strstr("previous", buf) ) {
            nFrom -= nCount;
            if ( nFrom < 0 )  nFrom = 0;
        }
        else if (NULL != strstr("next",buf)) {
            nFrom += nCount;
            /* TODO
            if ( (unsigned long)nFrom > gpobj->m_VSCP_Variables.m_listVariable.GetCount()-1 ) {
                if ( gpobj->m_VSCP_Variables.m_listVariable.GetCount() % nCount ) {
                    nFrom = gpobj->m_VSCP_Variables.m_listVariable.GetCount()/nCount;
                }
                else {
                    nFrom = (gpobj->m_VSCP_Variables.m_listVariable.GetCount()/nCount) - 1;
                }
            }*/
        }
        else if (NULL != strstr("last",buf)) {
            /* TODO
             if ( gpobj->m_VSCP_Variables.m_listVariable.GetCount() % nCount ) {
                nFrom = (gpobj->m_VSCP_Variables.m_listVariable.GetCount()/nCount)*nCount;
            }
            else {
                nFrom = ((gpobj->m_VSCP_Variables.m_listVariable.GetCount()/nCount) - 1)*nCount;
            }*/
        }
        else if ( NULL != strstr("first",buf) ) {
            nFrom = 0;
        }
    }
    else {  // No vaid navigation value
        //nFrom = 0;
    }

    mg_printf( nc,
        "HTTP/1.1 200 OK\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Cache-Control: max-age=0,  "
        "post-check=0, pre-check=0, "
        "no-store, no-cache, must-revalidate\r\n\r\n");


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

    /* TODO
    {
        wxString wxstrurl = _("/vscp/variables");
        buildPage += wxString::Format( _(WEB_COMMON_LIST_NAVIGATION),
                (const char *)wxstrurl.mbc_str(),
                (unsigned long)(nFrom+1),
                ( (unsigned long)(nFrom + nCount) < gpobj->m_VSCP_Variables.m_listVariable.GetCount()) ?
                    nFrom + nCount : gpobj->m_VSCP_Variables.m_listVariable.GetCount(),
                (unsigned long)gpobj->m_VSCP_Variables.m_listVariable.GetCount(),
                (unsigned long)nCount,
                (unsigned long)nFrom,
                _("false" ) );
        buildPage += _("<br>");
    }
     */

    wxString strBuf;

    // Display Variables List

    /* TODO
    if ( 0 == gpobj->m_VSCP_Variables.m_listVariable.GetCount() ) {
        buildPage += _("<br>Variables list is empty!<br>");
    }
    else {
        buildPage += _(WEB_VARLIST_TR_HEAD);
    }

    if (nFrom < 0) nFrom = 0;

    for ( unsigned int i=0;i<nCount;i++) {

        // Check if we are done
        if ( ( nFrom + i ) >= gpobj->m_VSCP_Variables.m_listVariable.GetCount() ) break;

        CVSCPVariable *pVariable =
                gpobj->m_VSCP_Variables.m_listVariable.Item( i )->GetData();
        {
            wxString url_dmedit =
                    wxString::Format(_("/vscp/varedit?id=%ld&from=%ld&count=%ld"),
                                        (long)(nFrom+i), (long)nFrom, (long)nCount );
            wxString str = wxString::Format(_(WEB_COMMON_TR_CLICKABLE_ROW),
                                                url_dmedit.wx_str() );
            buildPage += str;
        }

        // Client id
        buildPage += _(WEB_IFLIST_TD_CENTERED);
        buildPage += wxString::Format(_("<form name=\"input\" action=\"/vscp/vardelete?id=%ld\" method=\"get\"> %ld <input type=\"submit\" value=\"x\"><input type=\"hidden\" name=\"id\"value=\"%ld\"></form>"),
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
                wxstrurl.wx_str(),
                (unsigned long)(nFrom+1),
                ( (unsigned long)(nFrom + nCount) < gpobj->m_VSCP_Variables.m_listVariable.GetCount()) ?
                    nFrom + nCount : gpobj->m_VSCP_Variables.m_listVariable.GetCount(),
                (unsigned long)gpobj->m_VSCP_Variables.m_listVariable.GetCount(),
                (unsigned long)nCount,
                (unsigned long)nFrom,
                _("false" ) );
    }

    buildPage += _(WEB_COMMON_END);     // Common end code

    // Server data
    mg_send_http_chunk( nc, buildPage.ToAscii(), buildPage.Length() );
    mg_send_http_chunk( nc, "", 0 );
    */

    return;
}

///////////////////////////////////////////////////////////////////////////////
// websrv_variables_edit
//

void
VSCPWebServerThread::websrv_variables_edit( struct mg_connection *nc,
                                        struct http_message *hm  )
{
    char buf[80];
    wxString str;
    VSCPInformation vscpinfo;
    CVSCPVariable *pVariable = NULL;

    // Check pointer
    if (NULL == nc) return;

    //CControlObject *gpobj = (CControlObject *)nc->mgr->user_data;
    //if (NULL == gpobj) return;

    // id
    long id = -1;
    if ( mg_get_http_var( &hm->query_string, "id", buf, sizeof( buf ) ) > 0 ) {
        id = atoi( buf );
    }

    // type
    uint8_t nType = VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED;
    if ( mg_get_http_var( &hm->query_string, "type", buf, sizeof( buf ) ) > 0 ) {
        nType = atoi( buf );
    }

    // Flag for new variable row
    bool bNew = false;
    if ( mg_get_http_var( &hm->query_string, "new", buf, sizeof( buf ) ) > 0 ) {
        if ( NULL != strstr( "true", buf ) ) bNew = true;
    }

    // From
    long nFrom = 0;
    if ( mg_get_http_var( &hm->query_string, "from", buf, sizeof( buf ) ) > 0 ) {
        nFrom = atoi( buf );
    }


    // Count
    uint16_t nCount = 50;
    if ( mg_get_http_var( &hm->query_string, "count", buf, sizeof( buf ) ) > 0 ) {
        nCount = atoi( buf );
    }

    mg_printf( nc,
        "HTTP/1.1 200 OK\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Cache-Control: max-age=0,  "
        "post-check=0, pre-check=0, "
        "no-store, no-cache, must-revalidate\r\n\r\n");

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

    /* TODO
    if ( !bNew && ( id < (long)gpobj->m_VSCP_Variables.m_listVariable.GetCount() ) ) {
        pVariable = gpobj->m_VSCP_Variables.m_listVariable.Item(id)->GetData();
    }
     */

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
                str = pVariable->getValue();
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
                // TODO buildPage += wxString::Format(_("0x%x"), pVariable->m_event.vscp_class );
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
                // TODO buildPage += wxString::Format(_("0x%x"), pVariable->m_event.vscp_type );
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
                // TODO vscp_writeGuidArrayToString( pVariable->m_event.GUID, strGUID );
                buildPage += wxString::Format(_("%s"), strGUID.wx_str() );
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
                // TODO buildPage += wxString::Format(_("0x%x"), pVariable->m_event.timestamp );
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
                // TODO buildPage += wxString::Format(_("0x%X"), pVariable->m_event.obid );
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
                // TODO buildPage += wxString::Format(_("0x%02x"), pVariable->m_event.head );
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
                // TODO buildPage += wxString::Format(_("0x%08x"), pVariable->m_event.crc );
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
                // TODO buildPage += wxString::Format(_("%d"), pVariable->m_event.sizeData );
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
                // TODO vscp_writeVscpDataToString( &pVariable->m_event, strData );
                buildPage += wxString::Format(_("%s"), strData.wx_str() );
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
                // TODO vscp_writeVscpDataToString( &pVariable->m_event, strData );
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
                                    wxstrurl.wx_str() );

    buildPage += _("</form>");
    buildPage += _(WEB_COMMON_END);     // Common end code

    // Server data
    mg_send_http_chunk( nc, buildPage.ToAscii(), buildPage.Length() );
    mg_send_http_chunk( nc, "", 0 );

    return;
}


///////////////////////////////////////////////////////////////////////////////
// websrv_variables_post
//

void
VSCPWebServerThread::websrv_variables_post( struct mg_connection *nc,
                                        struct http_message *hm  )
{
    char buf[32000];
    wxString str;
    VSCPInformation vscpinfo;
    CVSCPVariable variable;

    // Check pointer
    if (NULL == nc) return;

    //CControlObject *gpobj = (CControlObject *)nc->mgr->user_data;
    //if (NULL == gpobj) return;

    // id
    long id = -1;
    if ( mg_get_http_var( &hm->query_string, "id", buf, sizeof( buf ) ) > 0 ) {
        id = atoi( buf );
    }

    // From
    long nFrom = 0;
    if ( mg_get_http_var( &hm->query_string, "from", buf, sizeof( buf ) ) > 0 ) {
        nFrom = atoi( buf );
    }

    // Count
    uint16_t nCount = 50;
    if ( mg_get_http_var( &hm->query_string, "count", buf, sizeof( buf ) ) > 0 ) {
        nCount = atoi( buf );
    }

    uint8_t nType = VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED;
    if ( mg_get_http_var( &hm->query_string, "type", buf, sizeof( buf ) ) > 0 ) {
        nType = atoi( buf );
    }

    wxString strName;
    if ( mg_get_http_var( &hm->query_string, "value_name", buf, sizeof( buf ) ) > 0 ) {
        strName = wxString::FromAscii( buf );
    }

    // Flag for new variable row
    bool bNew = false;
    if ( mg_get_http_var( &hm->query_string, "new", buf, sizeof( buf ) ) > 0 ) {
        if ( NULL != strstr( "true", buf ) ) bNew = true;
    }

    // Flag for persistence
    bool bPersistent = true;
    if ( mg_get_http_var( &hm->query_string, "persistent", buf, sizeof( buf ) ) > 0 ) {
        if ( NULL != strstr( "false", buf ) ) bPersistent = false;
    }

    wxString strNote;
    if ( mg_get_http_var( &hm->query_string, "note", buf, sizeof( buf ) ) > 0 ) {
        strNote = wxString::FromAscii( buf );
    }

    wxString strValueString;
    if ( mg_get_http_var( &hm->query_string, "value_string", buf, sizeof( buf ) ) > 0 ) {
        strValueString = wxString::FromAscii( buf );
    }

    bool bValueBoolean = false;
    if ( mg_get_http_var( &hm->query_string, "value_boolean", buf, sizeof( buf ) ) > 0 ) {
        if ( NULL != strstr( "true", buf ) ) bValueBoolean = true;
    }

    int value_integer = 0;
    if ( mg_get_http_var( &hm->query_string, "value_integer", buf, sizeof( buf ) ) > 0 ) {
        str = wxString::FromAscii( buf );
        value_integer = vscp_readStringValue( str );
    }

    long value_long = 0;
    if ( mg_get_http_var( &hm->query_string, "value_long", buf, sizeof( buf ) ) > 0 ) {
        str = wxString::FromAscii( buf );
        value_long = vscp_readStringValue( str );
    }

    double value_double = 0;
    if ( mg_get_http_var( &hm->query_string, "value_double", buf, sizeof( buf ) ) > 0 ) {
        value_double = atof( buf );
    }

    wxString strMeasurement;
    if ( mg_get_http_var( &hm->query_string, "value_measurement", buf, sizeof( buf ) ) > 0 ) {
        strMeasurement = wxString::FromAscii( buf );
    }

    uint16_t value_class = 0;
    if ( mg_get_http_var( &hm->query_string, "value_class", buf, sizeof( buf ) ) > 0 ) {
        value_class = vscp_readStringValue( wxString::FromAscii( buf ) );
    }

    uint16_t value_type = 0;
    if ( mg_get_http_var( &hm->query_string, "value_type", buf, sizeof( buf ) ) > 0 ) {
        value_type = vscp_readStringValue( wxString::FromAscii( buf ) );
    }

    wxString strGUID;
    if ( mg_get_http_var( &hm->query_string, "value_guid", buf, sizeof( buf ) ) > 0 ) {
        strGUID = wxString::FromAscii( buf );
        strGUID.Trim();
        if ( 0 == strGUID.Length() ) {
            strGUID = _("-");
        }
    }

    uint32_t value_timestamp = 0;
    if ( mg_get_http_var( &hm->query_string, "value_timestamp", buf, sizeof( buf ) ) > 0 ) {
        value_timestamp = vscp_readStringValue( wxString::FromAscii( buf ) );
    }

    uint32_t value_obid = 0;
    if ( mg_get_http_var( &hm->query_string, "value_obid", buf, sizeof( buf ) ) > 0 ) {
        value_obid = vscp_readStringValue( wxString::FromAscii( buf ) );
    }

    uint8_t value_head = 0;
    if ( mg_get_http_var( &hm->query_string, "value_head", buf, sizeof( buf ) ) > 0 ) {
        value_head = vscp_readStringValue( wxString::FromAscii( buf ) );
    }

    uint32_t value_crc = 0;
    if ( mg_get_http_var( &hm->query_string, "value_crc", buf, sizeof( buf ) ) > 0 ) {
        value_crc = vscp_readStringValue( wxString::FromAscii( buf ) );
    }

    uint16_t value_sizedata = 0;
    if ( mg_get_http_var( &hm->query_string, "value_sizedata", buf, sizeof( buf ) ) > 0 ) {
        value_sizedata = vscp_readStringValue( wxString::FromAscii( buf ) );
    }

    wxString strData;
    if ( mg_get_http_var( &hm->query_string, "value_data", buf, sizeof( buf ) ) > 0 ) {
        strData = wxString::FromAscii( buf );
    }

    wxString strDateTime;
    if ( mg_get_http_var( &hm->query_string, "value_date_time", buf, sizeof( buf ) ) > 0 ) {
        strDateTime = wxString::FromAscii( buf );
    }

    mg_printf( nc,
        "HTTP/1.1 200 OK\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Cache-Control: max-age=0,  "
        "post-check=0, pre-check=0, "
        "no-store, no-cache, must-revalidate\r\n\r\n");

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
                    buildPage += _("Error: Variable code is unassigned.<br>");
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
                    buildPage += _("Error: Variable code is unknown.<br>");
                    break;

                }

                // If new variable add it
                if ( bNew ) {
                    //gpobj->m_VSCP_Variables.add( pVariable );
                }

                // Save variables
                gpobj->m_VSCP_Variables.save();

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
    */

    buildPage += _(WEB_COMMON_END); // Common end code

    // Server data
    mg_send_http_chunk( nc, buildPage.ToAscii(), buildPage.Length() );
    mg_send_http_chunk( nc, "", 0 );

    return;
}


///////////////////////////////////////////////////////////////////////////////
// websrv_variables_new
//

void
VSCPWebServerThread::websrv_variables_new( struct mg_connection *nc,
                                        struct http_message *hm  )
{
    wxString str;
    char buf[80];
    VSCPInformation vscpinfo;

    // Check pointer
    if (NULL == nc) return;

    //CControlObject *gpobj = (CControlObject *)nc->mgr->user_data;
    //if (NULL == gpobj) return;

    // Count
    uint16_t nCount = 50;
    if ( mg_get_http_var( &hm->query_string, "count", buf, sizeof( buf ) ) > 0 ) {
        nCount = atoi( buf );
    }

    mg_printf( nc,
        "HTTP/1.1 200 OK\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Cache-Control: max-age=0,  "
        "post-check=0, pre-check=0, "
        "no-store, no-cache, must-revalidate\r\n\r\n");

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
    /* TODO
    if ( gpobj->m_VSCP_Variables.m_listVariable.GetCount() % nCount ) {
        nFrom = (gpobj->m_VSCP_Variables.m_listVariable.GetCount()/nCount)*nCount;
    }
    else {
        nFrom = ((gpobj->m_VSCP_Variables.m_listVariable.GetCount()/nCount) - 1)*nCount;
    }
     */

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
    //                                            strHost.wx_str() );
    //buildPage += wxString::Format( _(WEB_VARNEW_SUBMIT),
    //                                wxstrurl.wx_str() );

    buildPage += _("</form>");

    buildPage += _(WEB_COMMON_END); // Common end code

    // Server data
    mg_send_http_chunk( nc, buildPage.ToAscii(), buildPage.Length() );
    mg_send_http_chunk( nc, "", 0 );

    return;
}


///////////////////////////////////////////////////////////////////////////////
// websrv_variables_delete
//

void
VSCPWebServerThread::websrv_variables_delete( struct mg_connection *nc,
                                        struct http_message *hm  )
{
    char buf[80];
    wxString str;
    VSCPInformation vscpinfo;
    CVSCPVariable *pVariable = NULL;

    // Check pointer
    if (NULL == nc) return;

    //CControlObject *gpobj = (CControlObject *)nc->mgr->user_data;
    //if (NULL == gpobj) return;

    // id
    long id = -1;
    if ( mg_get_http_var( &hm->query_string, "id", buf, sizeof( buf ) ) > 0 ) {
        id = atoi( buf );
    }

    // From
    long nFrom = 0;
    if ( mg_get_http_var( &hm->query_string, "from", buf, sizeof( buf ) ) > 0 ) {
        nFrom = atoi( buf );
    }

    // Count
    uint16_t nCount = 50;
    if ( mg_get_http_var( &hm->query_string, "count", buf, sizeof( buf ) ) > 0 ) {
        nCount = atoi( buf );
    }

    mg_printf( nc,
        "HTTP/1.1 200 OK\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Cache-Control: max-age=0,  post-check=0, pre-check=0, no-store, no-cache, must-revalidate\r\n\r\n");

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
/* TODO
    pVariable = gpobj->m_VSCP_Variables.m_listVariable.Item(id)->GetData();
    //wxlistVscpVariableNode *node =
    //        gpobj->m_VSCP_Variables.m_listVariable.Item( id );
    //if ( gpobj->m_VSCP_Variables.m_listVariable.DeleteNode( node )  ) {//
    if ( gpobj->m_VSCP_Variables.remove( variable ) )  {
        buildPage += wxString::Format(_("<br>Deleted record id = %d"), id);
        // Save variables
        gpobj->m_VSCP_Variables.save();
    }
    else {
        buildPage += wxString::Format(_("<br>Failed to remove record id = %d"), id);
    }
*/
    buildPage += _(WEB_COMMON_END);     // Common end code

    // Server data
    mg_send_http_chunk( nc, buildPage.ToAscii(), buildPage.Length() );
    mg_send_http_chunk( nc, "", 0 );

    return;
}


///////////////////////////////////////////////////////////////////////////////
// websrv_discovery
//

void
VSCPWebServerThread::websrv_discovery( struct mg_connection *nc,
                                        struct http_message *hm  )
{
    //char buf[80];
    wxString str;
    VSCPInformation vscpinfo;

    // Check pointer
    if (NULL == nc) return;

    //CControlObject *gpobj = (CControlObject *)nc->mgr->user_data;
    //if (NULL == gpobj) return;

    mg_printf( nc,
        "HTTP/1.1 200 OK\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Cache-Control: max-age=0,  post-check=0, pre-check=0, no-store, no-cache, must-revalidate\r\n\r\n");

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
    mg_send_http_chunk( nc, buildPage.ToAscii(), buildPage.Length() );
    mg_send_http_chunk( nc, "", 0 );

    return;
}


///////////////////////////////////////////////////////////////////////////////
// websrv_session
//

void
VSCPWebServerThread::websrv_session( struct mg_connection *nc,
                                        struct http_message *hm  )
{
    //char buf[80];
    wxString str;
    VSCPInformation vscpinfo;

    // Check pointer
    if (NULL == nc) return;

    //CControlObject *gpobj = (CControlObject *)nc->mgr->user_data;
    //if (NULL == gpobj) return;

    mg_printf( nc,
        "HTTP/1.1 200 OK\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Cache-Control: max-age=0,  "
        "post-check=0, pre-check=0, "
        "no-store, no-cache, must-revalidate\r\n\r\n");

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
    mg_send_http_chunk( nc, buildPage.ToAscii(), buildPage.Length() );
    mg_send_http_chunk( nc, "", 0 );

    return;
}


///////////////////////////////////////////////////////////////////////////////
// websrv_configure
//

void
VSCPWebServerThread::websrv_configure( struct mg_connection *nc,
                                        struct http_message *hm  )
{
    wxString str;
    VSCPInformation vscpinfo;

    // Check pointer
    if (NULL == nc) return;

    //CControlObject *gpobj = (CControlObject *)nc->mgr->user_data;
    //if (NULL == gpobj) return;

    mg_printf( nc,
        "HTTP/1.1 200 OK\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Cache-Control: max-age=0,  post-check=0, pre-check=0, no-store, no-cache, must-revalidate\r\n\r\n");

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

    buildPage += _("<hr>");

    buildPage += _("<div id=\"small\">");
    buildPage += _("<b>wxWidgets version:</b> ");
    buildPage += wxString::Format(_("%d.%d.%d.%d" " Copyright (c) 1998-2005 Julian Smart, Robert Roebling et al"),
                        wxMAJOR_VERSION,
                        wxMINOR_VERSION,
                        wxRELEASE_NUMBER,
                        wxSUBRELEASE_NUMBER );
    buildPage += _("<br>");
    buildPage += _("</div>");

    buildPage += _("<div id=\"small\">");
    buildPage += _("<b>Mongoose version:</b> ");
    buildPage += wxString::FromUTF8( MG_VERSION " Copyright (c) 2013-2015 Cesanta Software Limited" );
    buildPage += _("<br>");
    buildPage += _("</div>");

    buildPage += _("<hr>");

    // Debuglevel
    buildPage += _("<div id=\"small\">");
    buildPage += _("<b>Debuglevel:</b> ");
    buildPage += wxString::Format(_("%d "), gpobj->m_logLevel );
    switch ( gpobj->m_logLevel  ) {
        case DAEMON_LOGMSG_NONE:
            buildPage += _("(none)");
            break;
        case DAEMON_LOGMSG_DEBUG:
            buildPage += _("(debug)");
            break;
        case DAEMON_LOGMSG_NORMAL:
            buildPage += _("(info)");
            break;
 /*
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
*/
        default:
            buildPage += _("(unkown)");
            break;
    }
    buildPage += _("</div>");

    // Server GUID
    buildPage += _("<div id=\"small\">");
    buildPage += _("<b>Server GUID:</b> ");
    gpobj->m_guid.toString( str );
    buildPage += str;
    buildPage += _("</div>");

    // Client buffer size
    buildPage += _("<div id=\"small\">");
    buildPage += _("<b>Client buffer size:</b> ");
    buildPage += wxString::Format(_("%d"), gpobj->m_maxItemsInClientReceiveQueue );
    buildPage += _("</div>");

    buildPage += _("<hr>");

    // TCP/IP interface
    buildPage += _("<div id=\"small\">");
    buildPage += _("<b>TCP/IP interface:</b> ");
    buildPage += _("enabled on <b>interface:</b> '");
    buildPage += gpobj->m_strTcpInterfaceAddress;
    buildPage += _("'");
    buildPage += _("<br>");
    buildPage += _("</div>");

    buildPage += _("<hr>");

    // UDP interface
    buildPage += _("<div id=\"small\">");
    buildPage += _("<b>UDP interface:</b> ");
    if ( gpobj->m_udpInfo.m_bEnable ) {
        buildPage += _("enabled on <b>interface:</b> '");
        buildPage += gpobj->m_udpInfo.m_interface;
        buildPage += _("'");
    }
    else {
        buildPage += _("disabled");
    }
    buildPage += _("<br>");
    buildPage += _("</div>");

    buildPage += _("<hr>");


    // VSCP Internal automation intelligence
    buildPage += _("<div id=\"small\">");
    buildPage += _("<b>VSCP internal event logic:</b> ");
    buildPage += _("enabled.");
    buildPage += _("<br>");
    buildPage += _("&nbsp;&nbsp;&nbsp;&nbsp;<b>Automation:</b> ");
    if ( gpobj->m_automation.isAutomationEnabled() ) {
        buildPage += _("enabled.<br>");
    }
    else {
        buildPage += _("disabled<br>");
    }



    if ( gpobj->m_automation.isSendHeartbeat() ) {
        buildPage += _( "&nbsp;&nbsp;&nbsp;&nbsp;Heartbeat will be sent.<br>" );
    }
    else {
        buildPage += _( "&nbsp;&nbsp;&nbsp;&nbsp;Heartbeat will be sent<br>" );
    }

    buildPage += _( "&nbsp;&nbsp;&nbsp;&nbsp;<b>Period for heartbeat</b>: " );
    buildPage += wxString::Format( _( "%ld" ), gpobj->m_automation.getIntervalHeartbeat() ) + _( " seconds<br>" );

    buildPage += _( "&nbsp;&nbsp;&nbsp;&nbsp;<b>Heartbeat last sent @</b>: " );
    buildPage += gpobj->m_automation.getHeartbeatSent().FormatISODate() + _( " " );
    buildPage += gpobj->m_automation.getHeartbeatSent().FormatISOTime() + _( "<br>" );



    if ( gpobj->m_automation.isSendSegmentControllerHeartbeat() ) {
        buildPage += _( "&nbsp;&nbsp;&nbsp;&nbsp;Segment controller heartbeat will be sent.<br>" );
    }
    else {
        buildPage += _( "&nbsp;&nbsp;&nbsp;&nbsp;Segment controller heartbeat will be sent<br>" );
    }

    buildPage += _( "&nbsp;&nbsp;&nbsp;&nbsp;<b>Period for Segment controller heartbeat</b>: " );
    buildPage += wxString::Format( _( "%ld" ), gpobj->m_automation.getIntervalSegmentControllerHeartbeat() ) + _( " seconds<br>" );

    buildPage += _( "&nbsp;&nbsp;&nbsp;&nbsp;<b>Segment controller heartbeat last sent @</b>: " );
    buildPage += gpobj->m_automation.getSegmentControllerHeartbeatSent().FormatISODate() + _( " " );
    buildPage += gpobj->m_automation.getSegmentControllerHeartbeatSent().FormatISOTime() + _( "<br>" );



    buildPage += _( "&nbsp;&nbsp;&nbsp;&nbsp;<b>Longitude</b>: " );
    buildPage += wxString::Format( _("%f"), gpobj->m_automation.getLongitude() ) + _( "<br>" );

    buildPage += _( "&nbsp;&nbsp;&nbsp;&nbsp;<b>Latitude</b>: " );
    buildPage += wxString::Format( _( "%f" ), gpobj->m_automation.getLatitude() ) + _( "<br>" );

    int hours, minutes;
    gpobj->m_automation.convert2HourMinute( gpobj->m_automation.getDayLength(), &hours, &minutes );
    buildPage += _( "&nbsp;&nbsp;&nbsp;&nbsp;<b>Length of day</b>: " );
    buildPage += wxString::Format( _( "%02d:%02d" ), hours, minutes ) + _( "<br>" );

    buildPage += _( "&nbsp;&nbsp;&nbsp;&nbsp;<b>Declination</b>: " );
    buildPage += wxString::Format( _( "%f" ), gpobj->m_automation.getDeclination() ) + _( "<br>" );

    buildPage += _( "&nbsp;&nbsp;&nbsp;&nbsp;<b>Sun max altitude</b>: " );
    buildPage += wxString::Format( _( "%f" ), gpobj->m_automation.getSunMaxAltitude() ) + _( "<br>" );

    buildPage += _( "&nbsp;&nbsp;&nbsp;&nbsp;<b>Last Calculation</b>: " );
    buildPage += gpobj->m_automation.getLastCalculation().FormatISODate() + _( " " );
    buildPage += gpobj->m_automation.getLastCalculation().FormatISOTime() + _("<br>");

    if ( gpobj->m_automation.isSendSunriseTwilightEvent() ) {
        buildPage += _( "&nbsp;&nbsp;&nbsp;&nbsp;Sunrise twilight event should be sent.<br>" );
    }
    else {
        buildPage += _( "&nbsp;&nbsp;&nbsp;&nbsp;Sunrise twilight event should not be sent.<br>" );
    }

    buildPage += _( "&nbsp;&nbsp;&nbsp;&nbsp;<b>Civil Twilight Sunrise Time</b>: " );
    buildPage += gpobj->m_automation.getCivilTwilightSunriseTime().FormatISOTime() + _( "<br>" );

    buildPage += _( "&nbsp;&nbsp;&nbsp;&nbsp;<b>Civil Twilight Sunrise Time sent @</b>: " );
    buildPage += gpobj->m_automation.getCivilTwilightSunriseTimeSent().FormatISODate() + _( " " );
    buildPage += gpobj->m_automation.getCivilTwilightSunriseTimeSent().FormatISOTime() + _( "<br>" );



    if ( gpobj->m_automation.isSendSunriseEvent() ) {
        buildPage += _( "&nbsp;&nbsp;&nbsp;&nbsp;Sunrise event should be sent.<br>" );
    }
    else {
        buildPage += _( "&nbsp;&nbsp;&nbsp;&nbsp;Sunrise event should not be sent.<br>" );
    }

    buildPage += _( "&nbsp;&nbsp;&nbsp;&nbsp;<b>Civil Sunrise Time</b>: " );
    buildPage += gpobj->m_automation.getSunriseTime().FormatISOTime() + _( "<br>" );

    buildPage += _( "&nbsp;&nbsp;&nbsp;&nbsp;<b>Civil Sunrise Time sent @</b>: " );
    buildPage += gpobj->m_automation.getSunriseTimeSent().FormatISODate() + _( " " );
    buildPage += gpobj->m_automation.getSunriseTimeSent().FormatISOTime() + _( "<br>" );


    if ( gpobj->m_automation.isSendSunsetEvent() ) {
        buildPage += _( "&nbsp;&nbsp;&nbsp;&nbsp;Sunset event should be sent.<br>" );
    }
    else {
        buildPage += _( "&nbsp;&nbsp;&nbsp;&nbsp;Sunset event should not be sent.<br>" );
    }

    buildPage += _( "&nbsp;&nbsp;&nbsp;&nbsp;<b>Civil Sunset Time</b>: " );
    buildPage += gpobj->m_automation.getSunsetTime().FormatISOTime() + _( "<br>" );

    buildPage += _( "&nbsp;&nbsp;&nbsp;&nbsp;<b>Civil Sunset Time sent @</b>: " );
    buildPage += gpobj->m_automation.getSunsetTimeSent().FormatISODate() + _( " " );
    buildPage += gpobj->m_automation.getSunsetTimeSent().FormatISOTime() + _( "<br>" );



    if ( gpobj->m_automation.isSendSunsetTwilightEvent() ) {
        buildPage += _( "&nbsp;&nbsp;&nbsp;&nbsp;Twilight sunset event should be sent.<br>" );
    }
    else {
        buildPage += _( "&nbsp;&nbsp;&nbsp;&nbsp;Twilight sunset should not be sent.<br>" );
    }

    buildPage += _( "&nbsp;&nbsp;&nbsp;&nbsp;<b>Civil Twilight Sunset Time</b>: " );
    buildPage += gpobj->m_automation.getCivilTwilightSunsetTime().FormatISOTime() + _( "<br>" );

    buildPage += _( "&nbsp;&nbsp;&nbsp;&nbsp;<b>Civil Twilight Sunset Time sent @</b>: " );
    buildPage += gpobj->m_automation.getCivilTwilightSunsetTimeSent().FormatISODate() + _( " " );
    buildPage += gpobj->m_automation.getCivilTwilightSunsetTimeSent().FormatISOTime() + _( "<br>" );


    if ( gpobj->m_automation.isSendCalculatedNoonEvent() ) {
        buildPage += _( "&nbsp;&nbsp;&nbsp;&nbsp;Calculated noon event should be sent.<br>" );
    }
    else {
        buildPage += _( "&nbsp;&nbsp;&nbsp;&nbsp;Calculated noon should not be sent.<br>" );
    }

    buildPage += _( "&nbsp;&nbsp;&nbsp;&nbsp;<b>Calculated Noon Time</b>: " );
    buildPage += gpobj->m_automation.getNoonTime().FormatISOTime() + _( "<br>" );

    buildPage += _( "&nbsp;&nbsp;&nbsp;&nbsp;<b>Calculated Noon Time sent @</b>: " );
    buildPage += gpobj->m_automation.getNoonTimeSent().FormatISODate() + _( " " );
    buildPage += gpobj->m_automation.getNoonTimeSent().FormatISOTime() + _( "<br>" );

    buildPage += _("</div>");


    buildPage += _("<hr>");

    // Web server interface
    buildPage += _("<div id=\"small\">");
    buildPage += _("<b>Web server <b>interface:</b></b> ");

    buildPage += _("enabled on interface '");
    buildPage += gpobj->m_strWebServerInterfaceAddress;
    buildPage += _( "<br>&nbsp;&nbsp;&nbsp;&nbsp;<b>Autentication:</b> " );
    if ( gpobj->m_bDisableSecurityWebServer ) {
        buildPage += _( "turned off." );
    }
    else {
        buildPage += _( "turned on." );
    }
    buildPage += _("<br>");
    buildPage += _("&nbsp;&nbsp;&nbsp;&nbsp;<b>Rootfolder:</b> ");
    buildPage += wxString::FromUTF8( gpobj->m_pathWebRoot );
    buildPage += _("<br>");
    buildPage += _("&nbsp;&nbsp;&nbsp;&nbsp;<b>Authdomain:</b> ");
    buildPage += wxString::FromUTF8( gpobj->m_authDomain );
    if ( 0 == strlen( gpobj->m_authDomain ) ) {
        buildPage += _("Set to default.");
    }
    buildPage += _("<br>");
    buildPage += _("&nbsp;&nbsp;&nbsp;&nbsp;<b>Path certs:</b> ");
    buildPage += wxString::FromUTF8( gpobj->m_pathCert );
    buildPage += _("<br>");
    buildPage += _("&nbsp;&nbsp;&nbsp;&nbsp;<b>ExtraMimeTypes:</b> ");
    buildPage += wxString::FromUTF8( gpobj->m_extraMimeTypes );
    if ( 0 == strlen( gpobj->m_extraMimeTypes ) ) {
        buildPage += _("Set to default.");
    }
    buildPage += _("<br>");
    buildPage += _("&nbsp;&nbsp;&nbsp;&nbsp;<b>CgiInterpreter:</b> ");
    buildPage += wxString::FromUTF8( gpobj->m_cgiInterpreter );
    if ( 0 == strlen( gpobj->m_cgiInterpreter ) ) {
        buildPage += _("Set to default.");
    }
    buildPage += _("<br>");
    buildPage += _("&nbsp;&nbsp;&nbsp;&nbsp;<b>CgiPattern:</b> ");
    buildPage += wxString::FromUTF8( gpobj->m_cgiPattern );
    if ( 0 == strlen( gpobj->m_cgiPattern ) ) {
        buildPage += _("Set to default.");
    }
    buildPage += _("<br>");
    buildPage += _("&nbsp;&nbsp;&nbsp;&nbsp;<b>HiddenFilePatterns:</b> ");
    buildPage += wxString::FromUTF8( gpobj->m_hideFilePatterns );
    if ( 0 == strlen( gpobj->m_hideFilePatterns ) ) {
        buildPage += _("Set to default.");
    }
    buildPage += _("<br>");
    buildPage += _("&nbsp;&nbsp;&nbsp;&nbsp;<b>IndexFiles:</b> ");
    buildPage += wxString::FromUTF8( gpobj->m_indexFiles );
    if ( 0 == strlen( gpobj->m_indexFiles ) ) {
        buildPage += _("Set to default.");
    }
    buildPage += _("<br>");
    if ( NULL != strstr( gpobj->m_EnableDirectoryListings, "yes" ) ) {
        buildPage += _("&nbsp;&nbsp;&nbsp;&nbsp;<b>Directory listings</b> is enabled.");
    }
    else {
       buildPage += _("&nbsp;&nbsp;&nbsp;&nbsp;<b>Directory listings</b> is disabled.");
    }
    buildPage += _("<br>");
    buildPage += _("&nbsp;&nbsp;&nbsp;&nbsp;<b>UrlReqrites:</b> ");
    buildPage += wxString::FromUTF8( gpobj->m_urlRewrites );
    if ( 0 == strlen( gpobj->m_urlRewrites ) ) {
       buildPage += _("Set to default.");
    }

    // TODO missing data

    buildPage += _("</div>");

    buildPage += _("<hr>");

    // Websockets
    buildPage += _("<div id=\"small\">");
    buildPage += _("<b>VSCP websocket interface:</b> ");
    if ( gpobj->m_bAuthWebsockets ) {
        buildPage += _("<b>Authentication</b> enabled.");
    }
    else {
        buildPage += _("<b>Authentication</b> disabled.");
    }
    buildPage += _("</div>");

    buildPage += _("<hr>");

    // DM
    buildPage += _("<div id=\"small\">");
    buildPage += _("<b>Daemon internal decision matrix functionality:</b> ");
    buildPage += _("enabled.");
    buildPage += _("<br>");
    buildPage += _("&nbsp;&nbsp;&nbsp;&nbsp;<b>Path to DM file:</b> ");
    buildPage += gpobj->m_dm.m_staticXMLPath;
    buildPage += _("<br>");

    buildPage += _("</div>");

    buildPage += _("<hr>");

    // Variable handling
    buildPage += _("<div id=\"small\">");
    buildPage += _("<b>Variable handling :</b> ");
    buildPage += _("&nbsp;&nbsp;&nbsp;&nbsp;<b>Path to variables:</b> ");
    buildPage += gpobj->m_VSCP_Variables.m_xmlPath;
    buildPage += _("</div>");

    buildPage += _("<hr>");

    buildPage += _("<div id=\"small\">");
    buildPage += _("<b>Level I Drivers:</b> ");

    buildPage += _("enabled<br>");

    CDeviceItem *pDeviceItem;
    VSCPDEVICELIST::iterator iter;
    for ( iter = gpobj->m_deviceList.m_devItemList.begin();
            iter != gpobj->m_deviceList.m_devItemList.end();
            ++iter ) {

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

    buildPage += _("<br>");
    buildPage += _("</div>");

    buildPage += _("<hr>");


    buildPage += _("<div id=\"small\">");
    buildPage += _("<b>Level II Drivers:</b> ");
    buildPage += _("enabled<br>");

    //CDeviceItem *pDeviceItem;
    //VSCPDEVICELIST::iterator iter;
    for (iter = gpobj->m_deviceList.m_devItemList.begin();
            iter != gpobj->m_deviceList.m_devItemList.end();
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
 
    buildPage += _("<br>");
    buildPage += _("</div>");


    // Serve data
    mg_send_http_chunk( nc, buildPage.ToAscii(), buildPage.Length() );
    mg_send_http_chunk( nc, "", 0 );

    return;
}



///////////////////////////////////////////////////////////////////////////////
// websrv_bootload
//

void
VSCPWebServerThread::websrv_bootload( struct mg_connection *nc,
                                        struct http_message *hm  )
{
    //char buf[80];
    wxString str;
    VSCPInformation vscpinfo;

    // Check pointer
    if (NULL == nc) return;

    //CControlObject *gpobj = (CControlObject *)nc->mgr->user_data;
    //if (NULL == gpobj) return;

    mg_printf( nc,
        "HTTP/1.1 200 OK\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Cache-Control: max-age=0,  "
        "post-check=0, pre-check=0, "
        "no-store, no-cache, must-revalidate\r\n\r\n");

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
    mg_send_http_chunk( nc, buildPage.ToAscii(), buildPage.Length() );
    mg_send_http_chunk( nc, "", 0 );

    return;
}


///////////////////////////////////////////////////////////////////////////////
// websrv_table
//

void
VSCPWebServerThread::websrv_table( struct mg_connection *nc,
                                        struct http_message *hm  )
{
    
/*
    wxString str;
    VSCPInformation vscpinfo;

    // Check pointer
    if (NULL == nc) return;

    CControlObject *gpobj = (CControlObject *)nc->mgr->user_data;
    if (NULL == gpobj) return;

    mg_printf( nc,
        "HTTP/1.1 200 OK\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Cache-Control: max-age=0,  "
        "post-check=0, pre-check=0, "
        "no-store, no-cache, must-revalidate\r\n\r\n");

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
    gpobj->m_mutexTableList.Lock();
    listVSCPTables::iterator iter;
    for (iter = gpobj->m_listTables.begin(); iter != gpobj->m_listTables.end(); ++iter) {

        ptblItem = *iter;

        if ( ( NULL == ptblItem ) || !ptblItem->isOpen() ) continue;

        buildPage += wxString::Format(_(WEB_COMMON_TR_CLICKABLE_ROW),
                                            ( wxString( _("/vscp/tablelist?tblname=") +
                                            wxString::FromUTF8( ptblItem->m_vscpFileHead.nameTable ) ) ).wx_str()  );
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
 */
/*        TODO
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
    gpobj->m_mutexTableList.Unlock();
*/
    /*

    buildPage += _(WEB_TABLELIST_TABLE_END);


    // Server data
    mg_send_http_chunk( nc, buildPage.ToAscii(), buildPage.Length() );
    mg_send_http_chunk( nc, "", 0 );

    return;
*/     
}



///////////////////////////////////////////////////////////////////////////////
// websrv_tablelist
//

void
VSCPWebServerThread::websrv_tablelist( struct mg_connection *nc,
                                        struct http_message *hm  )
{
/*
    char buf[512];
    wxString str;
    VSCPInformation vscpinfo;

    // Check pointer
    if (NULL == nc) return;

    CControlObject *gpobj = (CControlObject *)nc->mgr->user_data;
    if (NULL == gpobj) return;

    // From
    long nFrom = 0;
    if ( mg_get_http_var( &hm->query_string, "from", buf, sizeof( buf ) ) > 0 ) {
        nFrom = atoi( buf );
    }

    // Count
    uint16_t nCount = 50;
    if ( mg_get_http_var( &hm->query_string, "count", buf, sizeof( buf ) ) > 0 ) {
        nCount = atoi( buf );
    }


    wxString tblName;
    if ( mg_get_http_var( &hm->query_string, "tblname", buf, sizeof( buf ) ) > 0 ) {
        tblName = wxString::FromUTF8( buf );
    }

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
    if ( mg_get_http_var( &hm->query_string, "navbtn", buf, sizeof( buf ) ) > 0 ) {

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

    buildPage += wxString::Format( WEB_TABLEVALUELIST_BODY_START, tblName.wx_str() );

    {
        buildPage += wxString::Format( _(WEB_TABLEVALUE_LIST_NAVIGATION),
                _("/vscp/tablelist"),
                nFrom,
                ((nFrom + nCount) < ptblItem->getNumberOfRecords() ) ?
                      nFrom + nCount - 1 : ptblItem->getNumberOfRecords() - 1,
                ptblItem->getNumberOfRecords(),
                nCount,
                nFrom,
                tblName.wx_str() );

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
    mg_send_http_chunk( nc, buildPage.ToAscii(), buildPage.Length() );
    mg_send_http_chunk( nc, "", 0 );

    return;
*/ 
}



// -----------------------------------------------------------------------------
//                              N E W  T I M E S
// -----------------------------------------------------------------------------





///////////////////////////////////////////////////////////////////////////////
// init_webserver
//

int init_webserver( void ) 

{   
    const char *options[] = 
    {
        "document_root", DOCUMENT_ROOT,
            
	"listening_ports", PORT,
            
	"request_timeout_ms", "10000",
            
        "error_log_file", "error.log",

	"websocket_timeout_ms", "3600000",

	"ssl_certificate", "/tmp/resources/cert/server.pem",
            
	"ssl_protocol_version", "3",
            
	"ssl_cipher_list", "DES-CBC3-SHA:AES128-SHA:AES128-GCM-SHA256",

	"enable_auth_domain_check", "no",
            
	0 /* EOL */
};
        
    struct vscpweb_callbacks callbacks;
    struct vscpweb_context *ctx;
    struct vscpweb_server_ports ports[32];
    int port_cnt, n;
    int err = 0;
    
    // Start CivetWeb web server 
    memset( &callbacks, 0, sizeof( callbacks ) );
    
    callbacks.init_ssl = init_ssl;

    callbacks.log_message = log_message;
    ctx = vscpweb_start( &callbacks, 0, options );

    // Check return value: 
    if ( NULL == ctx ) {
        fprintf( stderr, "Cannot start CivetWeb - vscpweb_start failed.\n" );
	return EXIT_FAILURE;
}

/* Add handler EXAMPLE_URI, to explain the example */
vscpweb_set_request_handler(ctx, EXAMPLE_URI, ExampleHandler, 0);
vscpweb_set_request_handler(ctx, EXIT_URI, ExitHandler, 0);





// -----------------------------------------------------------------------------
//                          CIVETWEB test setup
// -----------------------------------------------------------------------------

//#define NO_SSL
#define USE_SSL_DH
#define USE_WEBSOCKET
#define USE_IPV6

#define DOCUMENT_ROOT "/srv/vscp/web"
// ip.v4 and ip.v6 at the same time
// https://github.com/civetweb/civetweb/issues/205
// https://stackoverflow.com/questions/1618240/how-to-support-both-ipv4-and-ipv6-connections
#ifdef USE_IPV6
#define PORT "[::]:8888r,[::]:8843s,8884"
#else
#define PORT "8888r,8843s,8884,9999"
#endif
#define EXAMPLE_URI "/example"
#define EXIT_URI "/exit"



////////////////////////////////////////////////////////////////////////////////
// ExampleHandler
//

int
ExampleHandler(struct vscpweb_connection *conn, void *cbdata)
{
	vscpweb_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");
	vscpweb_printf(conn, "<html><body>");
	vscpweb_printf(conn, "<h2>This is an example text from a C handler</h2>");
	vscpweb_printf(
	    conn,
	    "<p>To see a page from the A handler <a href=\"A\">click A</a></p>");
	vscpweb_printf(conn,
	          "<p>To see a page from the A handler <a href=\"A/A\">click "
	          "A/A</a></p>");
	vscpweb_printf(conn,
	          "<p>To see a page from the A/B handler <a "
	          "href=\"A/B\">click A/B</a></p>");
	vscpweb_printf(conn,
	          "<p>To see a page from the B handler (0) <a "
	          "href=\"B\">click B</a></p>");
	vscpweb_printf(conn,
	          "<p>To see a page from the B handler (1) <a "
	          "href=\"B/A\">click B/A</a></p>");
	vscpweb_printf(conn,
	          "<p>To see a page from the B handler (2) <a "
	          "href=\"B/B\">click B/B</a></p>");
	vscpweb_printf(conn,
	          "<p>To see a page from the *.foo handler <a "
	          "href=\"xy.foo\">click xy.foo</a></p>");
	vscpweb_printf(conn,
	          "<p>To see a page from the close handler <a "
	          "href=\"close\">click close</a></p>");
	vscpweb_printf(conn,
	          "<p>To see a page from the FileHandler handler <a "
	          "href=\"form\">click form</a> (the starting point of the "
	          "<b>form</b> test)</p>");
	vscpweb_printf(conn,
	          "<p>To see a page from the CookieHandler handler <a "
	          "href=\"cookie\">click cookie</a></p>");
	vscpweb_printf(conn,
	          "<p>To see a page from the PostResponser handler <a "
	          "href=\"postresponse\">click post response</a></p>");
	vscpweb_printf(conn,
	          "<p>To see an example for parsing files on the fly <a "
	          "href=\"on_the_fly_form\">click form</a> (form for "
	          "uploading files)</p>");

#ifdef USE_WEBSOCKET
	vscpweb_printf(conn,
	          "<p>To test websocket handler <a href=\"/websocket\">click "
	          "websocket</a></p>");
#endif
	vscpweb_printf(conn, "<p>To exit <a href=\"%s\">click exit</a></p>", EXIT_URI);
	vscpweb_printf(conn, "</body></html>\n");
	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// ExitHandler
//

int
ExitHandler(struct vscpweb_connection *conn, void *cbdata)
{
	vscpweb_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: "
	          "text/plain\r\nConnection: close\r\n\r\n");
	vscpweb_printf(conn, "Server will shut down.\n");
	vscpweb_printf(conn, "Bye!\n");
	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// AHandler
//

int
AHandler(struct vscpweb_connection *conn, void *cbdata)
{
	vscpweb_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");
	vscpweb_printf(conn, "<html><body>");
	vscpweb_printf(conn, "<h2>This is the A handler!!!</h2>");
	vscpweb_printf(conn, "</body></html>\n");
	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// ABHandler
//

int
ABHandler(struct vscpweb_connection *conn, void *cbdata)
{
	vscpweb_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");
	vscpweb_printf(conn, "<html><body>");
	vscpweb_printf(conn, "<h2>This is the AB handler!!!</h2>");
	vscpweb_printf(conn, "</body></html>\n");
	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// BXHandler
//

int
BXHandler(struct vscpweb_connection *conn, void *cbdata)
{
	/* Handler may access the request info using vscpweb_get_request_info */
	const struct vscpweb_request_info *req_info = vscpweb_get_request_info(conn);

	vscpweb_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");
	vscpweb_printf(conn, "<html><body>");
	vscpweb_printf(conn, "<h2>This is the BX handler %p!!!</h2>", cbdata);
	vscpweb_printf(conn, "<p>The actual uri is %s</p>", req_info->local_uri);
	vscpweb_printf(conn, "</body></html>\n");
	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// FooHandler
//

int
FooHandler(struct vscpweb_connection *conn, void *cbdata)
{
	/* Handler may access the request info using vscpweb_get_request_info */
	const struct vscpweb_request_info *req_info = vscpweb_get_request_info(conn);

	vscpweb_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");
	vscpweb_printf(conn, "<html><body>");
	vscpweb_printf(conn, "<h2>This is the Foo handler!!!</h2>");
	vscpweb_printf(conn,
	          "<p>The request was:<br><pre>%s %s HTTP/%s</pre></p>",
	          req_info->request_method,
	          req_info->local_uri,
	          req_info->http_version);
	vscpweb_printf(conn, "</body></html>\n");
	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// CloseHandler
//

int
CloseHandler(struct vscpweb_connection *conn, void *cbdata)
{
	/* Handler may access the request info using vscpweb_get_request_info */
	const struct vscpweb_request_info *req_info = vscpweb_get_request_info(conn);

	vscpweb_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");
	vscpweb_printf(conn, "<html><body>");
	vscpweb_printf(conn,
	          "<h2>This handler will close the connection in a second</h2>");
#ifdef _WIN32
	Sleep(1000);
#else
	sleep(1);
#endif
	vscpweb_printf(conn, "bye");
	printf("CloseHandler: close connection\n");
	vscpweb_close_connection(conn);
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
FileHandler(struct vscpweb_connection *conn, void *cbdata)
{
	/* In this handler, we ignore the req_info and send the file "fileName". */
	const char *fileName = (const char *)cbdata;

	vscpweb_send_file(conn, fileName);
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
	struct vscpweb_connection *conn = (struct vscpweb_connection *)user_data;

	vscpweb_printf(conn, "\r\n\r\n%s:\r\n", key);

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
	struct vscpweb_connection *conn = (struct vscpweb_connection *)user_data;

	if (key[0]) {
		vscpweb_printf(conn, "%s = ", key);
	}
	vscpweb_write(conn, value, valuelen);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// field_stored
//

int
field_stored(const char *path, long long file_size, void *user_data)
{
	struct vscpweb_connection *conn = (struct vscpweb_connection *)user_data;

	vscpweb_printf(conn,
	          "stored as %s (%lu bytes)\r\n\r\n",
	          path,
	          (unsigned long)file_size);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// FormHandler
//

int
FormHandler(struct vscpweb_connection *conn, void *cbdata)
{
	/* Handler may access the request info using vscpweb_get_request_info */
	const struct vscpweb_request_info *req_info = vscpweb_get_request_info(conn);
	int ret;
	struct vscpweb_form_data_handler fdh = {field_found, field_get, field_stored, 0};

	/* It would be possible to check the request info here before calling
	 * vscpweb_handle_form_request. */
	(void)req_info;

	vscpweb_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: "
	          "text/plain\r\nConnection: close\r\n\r\n");
	fdh.user_data = (void *)conn;

	/* Call the form handler */
	vscpweb_printf(conn, "Form data:");
	ret = vscpweb_handle_form_request(conn, &fdh);
	vscpweb_printf(conn, "\r\n%i fields found", ret);

	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// FileUploadForm
//

int
FileUploadForm(struct vscpweb_connection *conn, void *cbdata)
{
	vscpweb_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");

	vscpweb_printf(conn, "<!DOCTYPE html>\n");
	vscpweb_printf(conn, "<html>\n<head>\n");
	vscpweb_printf(conn, "<meta charset=\"UTF-8\">\n");
	vscpweb_printf(conn, "<title>File upload</title>\n");
	vscpweb_printf(conn, "</head>\n<body>\n");
	vscpweb_printf(conn,
	          "<form action=\"%s\" method=\"POST\" "
	          "enctype=\"multipart/form-data\">\n",
	          (const char *)cbdata);
	vscpweb_printf(conn, "<input type=\"file\" name=\"filesin\" multiple>\n");
	vscpweb_printf(conn, "<input type=\"submit\" value=\"Submit\">\n");
	vscpweb_printf(conn, "</form>\n</body>\n</html>\n");
	return 1;
}

#include <vscpmd5.h>

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
// 
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
// 
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
// 
//

int
CheckSumHandler(struct vscpweb_connection *conn, void *cbdata)
{
	/* Handler may access the request info using vscpweb_get_request_info */
	const struct vscpweb_request_info *req_info = vscpweb_get_request_info(conn);
	int i, j, ret;
	struct tfiles_checksums chksums;
	md5_byte_t digest[16];
	struct vscpweb_form_data_handler fdh = {field_disp_read_on_the_fly,
	                                   field_get_checksum,
	                                   0,
	                                   (void *)&chksums};

	/* It would be possible to check the request info here before calling
	 * vscpweb_handle_form_request. */
	(void)req_info;

	memset(&chksums, 0, sizeof(chksums));

	vscpweb_printf(conn,
	          "HTTP/1.1 200 OK\r\n"
	          "Content-Type: text/plain\r\n"
	          "Connection: close\r\n\r\n");

	/* Call the form handler */
	vscpweb_printf(conn, "File checksums:");
	ret = vscpweb_handle_form_request(conn, &fdh);
	for (i = 0; i < chksums.index; i++) {
		vscpmd5_finish(&(chksums.file[i].chksum), digest);
		/* Visual Studio 2010+ support llu */
		vscpweb_printf(conn,
		          "\r\n%s %llu ",
		          chksums.file[i].name,
		          chksums.file[i].length);
		for (j = 0; j < 16; j++) {
			vscpweb_printf(conn, "%02x", (unsigned int)digest[j]);
		}
	}
	vscpweb_printf(conn, "\r\n%i files\r\n", ret);

	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// 
//

int
CookieHandler(struct vscpweb_connection *conn, void *cbdata)
{
	/* Handler may access the request info using vscpweb_get_request_info */
	const struct vscpweb_request_info *req_info = vscpweb_get_request_info(conn);
	const char *cookie = vscpweb_get_header(conn, "Cookie");
	char first_str[64], count_str[64];
	int count;

	(void)vscpweb_get_cookie(cookie, "first", first_str, sizeof(first_str));
	(void)vscpweb_get_cookie(cookie, "count", count_str, sizeof(count_str));

	vscpweb_printf(conn, "HTTP/1.1 200 OK\r\nConnection: close\r\n");
	if (first_str[0] == 0) {
		time_t t = time(0);
		struct tm *ptm = localtime(&t);
		vscpweb_printf(conn,
		          "Set-Cookie: first=%04i-%02i-%02iT%02i:%02i:%02i\r\n",
		          ptm->tm_year + 1900,
		          ptm->tm_mon + 1,
		          ptm->tm_mday,
		          ptm->tm_hour,
		          ptm->tm_min,
		          ptm->tm_sec);
	}
	count = (count_str[0] == 0) ? 0 : atoi(count_str);
	vscpweb_printf(conn, "Set-Cookie: count=%i\r\n", count + 1);
	vscpweb_printf(conn, "Content-Type: text/html\r\n\r\n");

	vscpweb_printf(conn, "<html><body>");
	vscpweb_printf(conn, "<h2>This is the CookieHandler.</h2>");
	vscpweb_printf(conn, "<p>The actual uri is %s</p>", req_info->local_uri);

	if (first_str[0] == 0) {
		vscpweb_printf(conn, "<p>This is the first time, you opened this page</p>");
	} else {
		vscpweb_printf(conn, "<p>You opened this page %i times before.</p>", count);
		vscpweb_printf(conn, "<p>You first opened this page on %s.</p>", first_str);
	}

	vscpweb_printf(conn, "</body></html>\n");
	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// 
//

int
PostResponser(struct vscpweb_connection *conn, void *cbdata)
{
	long long r_total = 0;
	int r, s;

	char buf[2048];

	const struct vscpweb_request_info *ri = vscpweb_get_request_info(conn);

	if (strcmp(ri->request_method, "POST")) {
		char buf[1024];
		int ret = vscpweb_get_request_link(conn, buf, sizeof(buf));

		vscpweb_printf(conn,
		          "HTTP/1.1 405 Method Not Allowed\r\nConnection: close\r\n");
		vscpweb_printf(conn, "Content-Type: text/plain\r\n\r\n");
		vscpweb_printf(conn,
		          "%s method not allowed in the POST handler\n",
		          ri->request_method);
		if (ret >= 0) {
			vscpweb_printf(conn,
			          "use a web tool to send a POST request to %s\n",
			          buf);
		}
		return 1;
	}

	if (ri->content_length >= 0) {
		/* We know the content length in advance */
	} else {
		/* We must read until we find the end (chunked encoding
		 * or connection close), indicated my vscpweb_read returning 0 */
	}

	vscpweb_printf(conn,
	          "HTTP/1.1 200 OK\r\nConnection: "
	          "close\r\nTransfer-Encoding: chunked\r\n");
	vscpweb_printf(conn, "Content-Type: text/plain\r\n\r\n");

	r = vscpweb_read(conn, buf, sizeof(buf));
	while (r > 0) {
		r_total += r;
		s = vscpweb_send_chunk(conn, buf, r);
		if (r != s) {
			/* Send error */
			break;
		}
		r = vscpweb_read(conn, buf, sizeof(buf));
	}
	vscpweb_printf(conn, "0\r\n");

	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// 
//

int
WebSocketStartHandler(struct vscpweb_connection *conn, void *cbdata)
{
	vscpweb_printf(conn,
	          "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
	          "close\r\n\r\n");

	vscpweb_printf(conn, "<!DOCTYPE html>\n");
	vscpweb_printf(conn, "<html>\n<head>\n");
	vscpweb_printf(conn, "<meta charset=\"UTF-8\">\n");
	vscpweb_printf(conn, "<title>Embedded websocket example</title>\n");

#ifdef USE_WEBSOCKET
	/* vscpweb_printf(conn, "<script type=\"text/javascript\"><![CDATA[\n"); ...
	 * xhtml style */
	vscpweb_printf(conn, "<script>\n");
	vscpweb_printf(
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
	/* vscpweb_printf(conn, "]]></script>\n"); ... xhtml style */
	vscpweb_printf(conn, "</script>\n");
	vscpweb_printf(conn, "</head>\n<body onload=\"load()\">\n");
	vscpweb_printf(
	    conn,
	    "<div id='websock_text_field'>No websocket connection yet</div>\n");
#else
	vscpweb_printf(conn, "</head>\n<body>\n");
	vscpweb_printf(conn, "Example not compiled with USE_WEBSOCKET\n");
#endif
	vscpweb_printf(conn, "</body>\n</html>\n");

	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// 
//

#ifdef USE_WEBSOCKET

/* MAX_WS_CLIENTS defines how many clients can connect to a websocket at the
 * same time. The value 5 is very small and used here only for demonstration;
 * it can be easily tested to connect more than MAX_WS_CLIENTS clients.
 * A real server should use a much higher number, or better use a dynamic list
 * of currently connected websocket clients. */
#define MAX_WS_CLIENTS (5)

struct t_ws_client {
	struct vscpweb_connection *conn;
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
// 
//

int
WebSocketConnectHandler(const struct vscpweb_connection *conn, void *cbdata)
{
	struct vscpweb_context *ctx = vscpweb_get_context(conn);
	int reject = 1;
	int i;

	vscpweb_lock_context(ctx);
	for (i = 0; i < MAX_WS_CLIENTS; i++) {
		if (ws_clients[i].conn == NULL) {
			ws_clients[i].conn = (struct vscpweb_connection *)conn;
			ws_clients[i].state = 1;
			vscpweb_set_user_connection_data(ws_clients[i].conn,
			                            (void *)(ws_clients + i));
			reject = 0;
			break;
		}
	}
	vscpweb_unlock_context(ctx);

	fprintf(stdout,
	        "Websocket client %s\r\n\r\n",
	        (reject ? "rejected" : "accepted"));
	return reject;
}

////////////////////////////////////////////////////////////////////////////////
// 
//

void
WebSocketReadyHandler(struct vscpweb_connection *conn, void *cbdata)
{
	const char *text = "Hello from the websocket ready handler";
	struct t_ws_client *client = (struct t_ws_client *)vscpweb_get_user_connection_data(conn);

	vscpweb_websocket_write(conn, WEBSOCKET_OPCODE_TEXT, text, strlen(text));
	fprintf(stdout, "Greeting message sent to websocket client\r\n\r\n");
	ASSERT(client->conn == conn);
	ASSERT(client->state == 1);

	client->state = 2;
}

////////////////////////////////////////////////////////////////////////////////
// 
//

int
WebsocketDataHandler(struct vscpweb_connection *conn,
                     int bits,
                     char *data,
                     size_t len,
                     void *cbdata)
{
	struct t_ws_client *client = (struct t_ws_client *)vscpweb_get_user_connection_data(conn);
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
// 
//

void
WebSocketCloseHandler(const struct vscpweb_connection *conn, void *cbdata)
{
	struct vscpweb_context *ctx = vscpweb_get_context(conn);
	struct t_ws_client *client = (struct t_ws_client *)vscpweb_get_user_connection_data(conn);
	ASSERT(client->conn == conn);
	ASSERT(client->state >= 1);

	vscpweb_lock_context(ctx);
	client->state = 0;
	client->conn = NULL;
	vscpweb_unlock_context(ctx);

	fprintf(stdout,
	        "Client droped from the set of webserver connections\r\n\r\n");
}

////////////////////////////////////////////////////////////////////////////////
// 
//

void
InformWebsockets(struct vscpweb_context *ctx)
{
	static unsigned long cnt = 0;
	char text[32];
	int i;

	sprintf(text, "%lu", ++cnt);

	vscpweb_lock_context(ctx);
	for (i = 0; i < MAX_WS_CLIENTS; i++) {
		if (ws_clients[i].state == 2) {
			vscpweb_websocket_write(ws_clients[i].conn,
			                   WEBSOCKET_OPCODE_TEXT,
			                   text,
			                   strlen(text));
		}
	}
	vscpweb_unlock_context(ctx);
}
#endif

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


#ifndef NO_SSL
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
#endif


int
log_message(const struct vscpweb_connection *conn, const char *message)
{
    puts(message);
    return 1;
}


///////////////////////////////////////////////////////////////////////////////
// init_webserver
//

int init_webserver( void ) 
{   
    	const char *options[] = {
	    "document_root",
	    DOCUMENT_ROOT,
	    "listening_ports",
	    PORT,
	    "request_timeout_ms",
	    "10000",
	    "error_log_file",
	    "error.log",
#ifdef USE_WEBSOCKET
	    "websocket_timeout_ms",
	    "3600000",
#endif
#ifndef NO_SSL
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
#endif
	    "enable_auth_domain_check",
	    "no",
	    0};
	struct vscpweb_callbacks callbacks;
	struct vscpweb_context *ctx;
	struct vscpweb_server_ports ports[32];
	int port_cnt, n;
	int err = 0;

// Check if we have been built with all required features. 
#ifdef USE_IPV6
	if (!vscpweb_check_feature(8)) {
		fprintf(stderr,
		        "Error: Embedded example built with IPv6 support, "
		        "but civetweb library build without.\n");
		err = 1;
	}
#endif
#ifdef USE_WEBSOCKET
	if (!vscpweb_check_feature(16)) {
		fprintf(stderr,
		        "Error: Embedded example built with websocket support, "
		        "but civetweb library build without.\n");
		err = 1;
	}
#endif
#ifndef NO_SSL
	if (!vscpweb_check_feature(2)) {
		fprintf(stderr,
		        "Error: Embedded example built with SSL support, "
		        "but civetweb library build without.\n");
		err = 1;
	}
#endif
	if (err) {
		fprintf(stderr, "Cannot start CivetWeb - inconsistent build.\n");
		return EXIT_FAILURE;
	}

	/* Start CivetWeb web server */
	memset(&callbacks, 0, sizeof(callbacks));
#ifndef NO_SSL
	callbacks.init_ssl = init_ssl;
#endif
	callbacks.log_message = log_message;
	ctx = vscpweb_start(&callbacks, 0, options);

	/* Check return value: */
	if (ctx == NULL) {
		fprintf(stderr, "Cannot start CivetWeb - vscpweb_start failed.\n");
		return EXIT_FAILURE;
	}

	/* Add handler EXAMPLE_URI, to explain the example */
	vscpweb_set_request_handler(ctx, EXAMPLE_URI, ExampleHandler, 0);
	vscpweb_set_request_handler(ctx, EXIT_URI, ExitHandler, 0);

	/* Add handler for /A* and special handler for /A/B */
	vscpweb_set_request_handler(ctx, "/A", AHandler, 0);
	vscpweb_set_request_handler(ctx, "/A/B", ABHandler, 0);

	/* Add handler for /B, /B/A, /B/B but not for /B* */
	vscpweb_set_request_handler(ctx, "/B$", BXHandler, (void *)0);
	vscpweb_set_request_handler(ctx, "/B/A$", BXHandler, (void *)1);
	vscpweb_set_request_handler(ctx, "/B/B$", BXHandler, (void *)2);

	/* Add handler for all files with .foo extention */
	vscpweb_set_request_handler(ctx, "**.foo$", FooHandler, 0);

	/* Add handler for /close extention */
	vscpweb_set_request_handler(ctx, "/close", CloseHandler, 0);

	/* Add handler for /form  (serve a file outside the document root) */
	vscpweb_set_request_handler(ctx,
	                       "/form",
	                       FileHandler,
	                       (void *)"../../test/form.html");

	/* Add handler for form data */
	vscpweb_set_request_handler(ctx,
	                       "/handle_form.embedded_c.example.callback",
	                       FormHandler,
	                       (void *)0);

	/* Add a file upload handler for parsing files on the fly */
	vscpweb_set_request_handler(ctx,
	                       "/on_the_fly_form",
	                       FileUploadForm,
	                       (void *)"/on_the_fly_form.md5.callback");
	vscpweb_set_request_handler(ctx,
	                       "/on_the_fly_form.md5.callback",
	                       CheckSumHandler,
	                       (void *)0);

	/* Add handler for /cookie example */
	vscpweb_set_request_handler(ctx, "/cookie", CookieHandler, 0);

	/* Add handler for /postresponse example */
	vscpweb_set_request_handler(ctx, "/postresponse", PostResponser, 0);

	/* Add HTTP site to open a websocket connection */
	vscpweb_set_request_handler(ctx, "/websocket", WebSocketStartHandler, 0);

#ifdef USE_WEBSOCKET
	/* WS site for the websocket connection */
	vscpweb_set_websocket_handler(ctx,
	                         "/websocket",
	                         WebSocketConnectHandler,
	                         WebSocketReadyHandler,
	                         WebsocketDataHandler,
	                         WebSocketCloseHandler,
	                         0);
#endif
        
        
    
}


