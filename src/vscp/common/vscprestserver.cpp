// vscprestserver.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (http://www.vscp.org)
//
// Copyright (C) 2000-2016 
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
#include <wx/filename.h>

#include <iostream>
#include <sstream>
#include <fstream>

#ifdef WIN32

#include <winsock2.h>
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

#include <slre.h>
#include <mongoose.h>

#include <canal_macro.h>
#include <vscp.h>
#include <vscphelper.h>
#include <vscpeventhelper.h>
#include <tables.h>
#include <configfile.h>
#include <crc.h>
#include <xml2json.hpp>
#include <randpassword.h>
#include <version.h>
#include <variablecodes.h>
#include <actioncodes.h>
#include <devicelist.h>
#include <devicethread.h>
#include <dm.h>
#include <mdf.h>
#include <vscpeventhelper.h>
#include <vscpwebserver.h>
#include <webserver_rest.h>
#include <controlobject.h>
#include <webserver.h>

using namespace std;

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

// Linked list of all active sessions. (webserv.h)
extern struct websrv_Session *gp_websrv_sessions;

// Session structure for REST API
extern struct websrv_rest_session *gp_websrv_rest_sessions;


// Prototypes



int webserv_url_decode( const char *src, int src_len, 
                            char *dst, int dst_len,
                            int is_form_url_encoded );
void webserv_util_sendheader( struct mg_connection *nc, 
                                        const int returncode, 
                                        const char *content );                            


//-----------------------------------------------------------------------------
//                                   REST
//-----------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////
// websrv_get_rest_session
//

struct websrv_rest_session *
VSCPWebServerThread::websrv_new_rest_session( struct mg_connection *nc,
                                                CUserItem *pUser )
{
    char buf[2048];
    struct websrv_rest_session *ret = NULL;

    // Check pointer
    if (NULL == nc) return NULL;
    
    CControlObject *pObject = (CControlObject *)nc->mgr->user_data;
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
    
    MD5_CTX ctx;
    MD5_Init( &ctx );
    MD5_Update( &ctx, (const unsigned char *)buf, strlen( buf ) );
    unsigned char bindigest[16];
    MD5_Final( bindigest, &ctx );
    char digest[33];
    memset( digest, 0, sizeof( digest ) );
    cs_to_hex( ret->sid, bindigest, 16 );

    // New client
    ret->pClientItem = new CClientItem();   // Create client        
    vscp_clearVSCPFilter(&ret->pClientItem->m_filterVSCP); // Clear filter
    ret->pClientItem->m_type = CLIENT_ITEM_INTERFACE_TYPE_CLIENT_REST;
    ret->pClientItem->m_strDeviceName = _("REST client. Started at ");
    wxDateTime now = wxDateTime::Now();
    ret->pClientItem->m_strDeviceName += now.FormatISODate();
    ret->pClientItem->m_strDeviceName += _(" ");
    ret->pClientItem->m_strDeviceName += now.FormatISOTime();
    ret->pClientItem->m_bOpen = true;        // Open client

    // Add the client to the Client List
    pObject->m_wxClientMutex.Lock();
    pObject->addClient(ret->pClientItem);
    pObject->m_wxClientMutex.Unlock();

    ret->pUserItem = pUser;
    
    // Add to linked list
    ret->lastActiveTime = time(NULL);
    ret->m_next = gp_websrv_rest_sessions;
    gp_websrv_rest_sessions = ret;

    return ret;
}


///////////////////////////////////////////////////////////////////////////////
// websrv_get_rest_session
//

struct websrv_rest_session *
VSCPWebServerThread::websrv_get_rest_session( struct mg_connection *nc,
                                                wxString &SessionId )
{
    struct websrv_rest_session *ret = NULL;

    // Check pointers
    if ( NULL == nc) return NULL;
        
    // find existing session 
    ret = gp_websrv_rest_sessions;
    while (NULL != ret) {
        if  (0 == strcmp( SessionId.mbc_str(), ret->sid) )  break;
        ret = ret->m_next;
    }
        
    if (NULL != ret) {
        ret->lastActiveTime = time( NULL );
        return ret;
    }
    
    return ret;
}

void
VSCPWebServerThread::websrv_expire_rest_sessions( struct mg_connection *nc )
{
    struct websrv_rest_session *pos;
    struct websrv_rest_session *prev;
    struct websrv_rest_session *next;
    time_t now;

    // Check pointer
    if (NULL == nc) return;
    
    CControlObject *pObject = (CControlObject *)nc->mgr->user_data;
    if (NULL == pObject) return;

    now = time( NULL );
    prev = NULL;
    pos = gp_websrv_rest_sessions;
    
    while (NULL != pos) {
        
        next = pos->m_next;
        
        if (now - pos->lastActiveTime > 10 * 60) {  // Ten minutes
        
            // expire sessions after 1h 
            if ( NULL == prev ) {
                gp_websrv_rest_sessions = pos->m_next;
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

void
VSCPWebServerThread::websrv_restapi( struct mg_connection *nc,
                                        struct http_message *hm )
{
    char buf[2048];
    char date[64];
    wxString str;
    time_t curtime = time(NULL);
    long format = REST_FORMAT_PLAIN;
    hashArgs keypairs;
    struct websrv_rest_session *pSession = NULL;
    CUserItem *pUser = NULL;

    // Check pointer
    if (NULL == nc) return;
    
    // Get method
    char method[33];
    memset( method, 0, sizeof( method ) );
    strncpy( method, hm->method.p, hm->method.len );

    // Make string with GMT time
    vscp_getTimeString( date, sizeof(date), &curtime );
    
    CControlObject *pObject = (CControlObject *)nc->mgr->user_data;
    if (NULL == pObject) return;

    vscp_getTimeString( date, sizeof(date), &curtime );

    
    if ( NULL != strstr( method, ("POST") ) ) {

        mg_str *phdr;

        // user
        if ( NULL != ( phdr = mg_get_http_header( hm, "vscpuser" ) ) ) {
            memset( buf, 0, sizeof( buf ) );
            strncpy( buf, phdr->p, phdr->len );
            keypairs[_("VSCPUSER")] = wxString::FromUTF8( buf );        
        }

        // password
        if ( NULL != ( phdr = mg_get_http_header( hm, "vscpsecret" ) ) ) {
            memset( buf, 0, sizeof( buf ) );
            strncpy( buf, phdr->p, phdr->len );
            keypairs[_("VSCPSECRET")] = wxString::FromUTF8( buf );
        }

        // session
        if ( NULL != ( phdr = mg_get_http_header( hm, "vscpsession" ) ) ) {
            memset( buf, 0, sizeof( buf ) );
            strncpy( buf, phdr->p, phdr->len );
            keypairs[_("VSCPSESSION")] = wxString::FromUTF8( buf );        
        }
        
    }
    else {
        // get parameters for get

        if ( 0 < mg_get_http_var( &hm->query_string, "vscpuser", buf, sizeof( buf ) ) ) {
            keypairs[ _("VSCPUSER") ] = wxString::FromUTF8( buf );
        }
        
        if ( 0 < mg_get_http_var( &hm->query_string, "vscpsecret", buf, sizeof( buf ) ) ) {
            keypairs[ _("VSCPSECRET") ] = wxString::FromUTF8( buf );
        }
        
        if ( 0 < mg_get_http_var( &hm->query_string, "vscpsession", buf, sizeof( buf ) ) ) {
            keypairs[ _("VSCPSESSION") ] = wxString::FromUTF8( buf );
        }
    }

    // format
    if ( 0 < mg_get_http_var( &hm->query_string, "format", buf, sizeof( buf ) ) ) {
        keypairs[_("FORMAT")] = wxString::FromUTF8( buf );
    }

    // op
    if ( 0 < mg_get_http_var( &hm->query_string, "op", buf, sizeof(buf) ) ) {
        keypairs[_("OP")] = wxString::FromUTF8( buf );
    }

    // vscpevent
    if ( 0 < mg_get_http_var( &hm->query_string, "vscpevent", buf, sizeof(buf) ) ) {
        keypairs[_("VSCPEVENT")] = wxString::FromUTF8( buf );
    }

    // count
    if ( 0 < mg_get_http_var( &hm->query_string, "count", buf, sizeof(buf) ) ) {
        keypairs[_("COUNT")] = wxString::FromUTF8( buf );
    }

    // vscpfilter
    if ( 0 < mg_get_http_var( &hm->query_string, "vscpfilter", buf, sizeof(buf) ) ) {
        keypairs[_("VSCPFILTER")] = wxString::FromUTF8( buf );
    }

    // vscpmask
    if ( 0 < mg_get_http_var( &hm->query_string, "vscpmask", buf, sizeof( buf ) ) ) {
        keypairs[ _( "VSCPMASK" ) ] = wxString::FromUTF8( buf );
    }

    // variable
    if ( 0 < mg_get_http_var( &hm->query_string, "variable", buf, sizeof(buf) ) ) {
        keypairs[_("VARIABLE")] = wxString::FromUTF8( buf );
    }

    // value
    if ( 0 < mg_get_http_var( &hm->query_string, "value", buf, sizeof( buf ) ) ) {
        keypairs[ _( "VALUE" ) ] = wxString::FromUTF8( buf );
    }

    // type
    if ( 0 < mg_get_http_var( &hm->query_string, "type", buf, sizeof(buf) ) ) {
        keypairs[_("TYPE")] = wxString::FromUTF8( buf );
    }

    // persistent
    if ( 0 < mg_get_http_var( &hm->query_string, "persistent", buf, sizeof( buf ) ) ) {
        keypairs[ _( "PERSISTENT" ) ] = wxString::FromUTF8( buf );
    }

    // note
    if ( 0 < mg_get_http_var( &hm->query_string, "note", buf, sizeof( buf ) ) ) {
        keypairs[ _( "NOTE" ) ] = wxString::FromUTF8( buf );
    }

    // unit
    if ( 0 < mg_get_http_var( &hm->query_string, "unit", buf, sizeof(buf) ) ) {
        keypairs[_("UNIT")] = wxString::FromUTF8( buf );
    }

    // sensoridx
    if ( 0 < mg_get_http_var( &hm->query_string, "sensoridx", buf, sizeof(buf) ) ) {
        keypairs[_("SENSORINDEX")] = wxString::FromUTF8( buf );
    }

    // level  ( VSCP level 1 or 2 )
    if ( 0 < mg_get_http_var( &hm->query_string, "level", buf, sizeof( buf ) ) ) {
        keypairs[ _( "LEVEL" ) ] = wxString::FromUTF8( buf );
    }

    // zone
    if ( 0 < mg_get_http_var( &hm->query_string, "zone", buf, sizeof( buf ) ) ) {
        keypairs[ _( "ZONE" ) ] = wxString::FromUTF8( buf );
    }

    // subzone
    if ( 0 < mg_get_http_var( &hm->query_string, "subzone", buf, sizeof( buf ) ) ) {
        keypairs[ _( "SUBZONE" ) ] = wxString::FromUTF8( buf );
    }

    // guid
    if ( 0 < mg_get_http_var( &hm->query_string, "guid", buf, sizeof( buf ) ) ) {
        keypairs[ _( "GUID" ) ] = wxString::FromUTF8( buf );
    }

    // name
    if ( 0 < mg_get_http_var( &hm->query_string, "name", buf, sizeof(buf) ) ) {
        keypairs[_("NAME")] = wxString::FromUTF8( buf );
    }

    // from
    if ( 0 < mg_get_http_var( &hm->query_string, "from", buf, sizeof(buf) ) ) {
        keypairs[_("FROM")] = wxString::FromUTF8( buf );
    }

    // to
    if ( 0 < mg_get_http_var( &hm->query_string, "to", buf, sizeof(buf) ) ) {
        keypairs[_("TO")] = wxString::FromUTF8( buf );
    }

    // url
    if ( 0 < mg_get_http_var( &hm->query_string, "url", buf, sizeof( buf ) ) ) {
        keypairs[ _( "URL" ) ] = wxString::FromUTF8( buf );
    }

    // eventformat
    if ( 0 < mg_get_http_var( &hm->query_string, "eventformat", buf, sizeof( buf ) ) ) {
        keypairs[ _( "EVENTFORMAT" ) ] = wxString::FromUTF8( buf );
    }

    // If we have a session key we try to get the session
    if ( _("") != keypairs[_("VSCPSESSION")] ) {
        pSession = websrv_get_rest_session( nc, keypairs[_("VSCPSESSION")] );
        if ( NULL != pSession )  pUser = pSession->pUserItem;
    }

    if ( NULL == pSession ) {

        // Get user
        pUser = pObject->m_userList.getUser( keypairs[_("VSCPUSER")] ); 

        // Check if user is valid
        if ( NULL == pUser ) {
            wxString strErr = 
            wxString::Format( _("[REST Client] Host [%s] Invalid user [%s]\n"), 
                                wxString::FromAscii( (const char *)inet_ntoa( nc->sa.sin.sin_addr )).wx_str(), 
                                keypairs[_("VSCPUSER")].wx_str() );
            pObject->logMsg( strErr, DAEMON_LOGMSG_WARNING, DAEMON_LOGTYPE_SECURITY );
            webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_INVALID_USER );
            return;
        }

        // Check if remote ip is valid
        bool bValidHost;
        pObject->m_mutexUserList.Lock();
        bValidHost = pUser->isAllowedToConnect( wxString::FromAscii( inet_ntoa( nc->sa.sin.sin_addr ) ) );
        pObject->m_mutexUserList.Unlock();
        if (!bValidHost) {
            wxString strErr = 
            wxString::Format( _("[REST Client] Host [%s] NOT allowed to connect. User [%s]\n"), 
                                wxString::FromAscii( (const char *)inet_ntoa( nc->sa.sin.sin_addr ) ).wx_str(), 
                                keypairs[_("VSCPUSER")].wx_str() );
            pObject->logMsg ( strErr, DAEMON_LOGMSG_WARNING, DAEMON_LOGTYPE_SECURITY );
            webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_INVALID_ORIGIN );
            return;
        }

        // Is this an authorized user?
        wxString str3 = keypairs[_("VSCPSECRET")];
        if ( keypairs[_("VSCPSECRET")] != pUser->m_md5Password ) {
            wxString strErr = 
            wxString::Format( _("[REST Client] User [%s] NOT allowed to connect. Client [%s]\n"), 
                                keypairs[_("VSCPUSER")].wx_str(), 
                                wxString::FromAscii( (const char *)inet_ntoa( nc->sa.sin.sin_addr ) ).wx_str() );
            pObject->logMsg ( strErr, DAEMON_LOGMSG_WARNING, DAEMON_LOGTYPE_SECURITY );
            webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_INVALID_PASSWORD );
            return;
        }

    }
    else {
        // check that the origin is valid
        // Check if remote ip is valid
        bool bValidHost;
        pObject->m_mutexUserList.Lock();
        bValidHost = pSession->pUserItem->isAllowedToConnect( wxString::FromAscii( inet_ntoa( nc->sa.sin.sin_addr ) ) );
        pObject->m_mutexUserList.Unlock();
        if (!bValidHost) {
            wxString strErr = 
            wxString::Format( _("[REST Client] Host [%s] NOT allowed to connect. User [%s]\n"), 
                                wxString::FromAscii( (const char *)inet_ntoa( nc->sa.sin.sin_addr ) ).wx_str(), 
                                keypairs[_("VSCPUSER")].wx_str() );
            pObject->logMsg ( strErr, DAEMON_LOGMSG_WARNING, DAEMON_LOGTYPE_SECURITY );
            webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_INVALID_ORIGIN );
            return;
        }
    }

    // ------------------------------------------------------------------------
    //                      * * * User is validated * * *
    // ------------------------------------------------------------------------

    wxString strErr = 
        wxString::Format( _("[REST Client] User [%s] Host [%s] allowed to connect. \n"), 
                            keypairs[_("VSCPUSER")].wx_str() , 
                            wxString::FromAscii( (const char *)inet_ntoa( nc->sa.sin.sin_addr ) ).wx_str() );
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
        webserv_util_sendheader( nc, 400, "text/plain" );
        mg_send_http_chunk( nc, REST_PLAIN_ERROR_UNSUPPORTED_FORMAT, 
                                    strlen( REST_PLAIN_ERROR_UNSUPPORTED_FORMAT ) );
        mg_send_http_chunk( nc, "", 0 );   // Terminator
        return;
    }

    //   *************************************************************
    //   * * * * * * * *  Status (hold session open)   * * * * * * * *
    //   *************************************************************
    if ( ( _("0") == keypairs[_("OP")] ) ||  ( _("STATUS") == keypairs[_("OP")].Upper() ) ) {
        webserv_rest_doStatus( nc, pSession, format );
    }

    //  ********************************************
    //  * * * * * * * * open session * * * * * * * *
    //  ********************************************
    else if ( ( _("1") == keypairs[_("OP")] ) || ( _("OPEN") == keypairs[_("OP")].Upper() ) ) {
        webserv_rest_doOpen( nc, pSession, pUser, format );        
    }

    //   **********************************************
    //   * * * * * * * * close session  * * * * * * * *
    //   **********************************************
    else if ( ( _("2") == keypairs[_("OP")] ) || ( _("CLOSE") == keypairs[_("OP")].Upper() ) ) {
        webserv_rest_doClose( nc, pSession, format );
    }

    //  ********************************************
    //   * * * * * * * * Send event  * * * * * * * *
    //  ********************************************
    else if ( ( _("3") == keypairs[_("OP")] ) || ( _("SENDEVENT") == keypairs[_("OP")].Upper() ) ) {
        vscpEvent vscpevent;
        if ( _("") != keypairs[_("VSCPEVENT")] ) {
            vscp_setVscpEventFromString( &vscpevent, keypairs[_("VSCPEVENT")] ); 
            webserv_rest_doSendEvent( nc, pSession, format, &vscpevent );
        }
        else {
            // Parameter missing - No Event
            webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_MISSING_DATA );
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
        webserv_rest_doReceiveEvent( nc, pSession, format, count );
    }

    //   **************************************************
    //   * * * * * * * *      Set filter    * * * * * * * *
    //   **************************************************
    else if ( ( _("5") == keypairs[_("OP")] ) || ( _("SETFILTER") == keypairs[_("OP")].Upper() ) ) {
        
        vscpEventFilter vscpfilter;
        vscp_clearVSCPFilter( &vscpfilter );

        if ( _("") != keypairs[_("VSCPFILTER")] ) {
            vscp_readFilterFromString( &vscpfilter, keypairs[_("VSCPFILTER")] ); 
        }
        else {
            webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_MISSING_DATA );
        }

        if ( _( "" ) != keypairs[ _("VSCPMASK") ] ) {
            vscp_readMaskFromString( &vscpfilter, keypairs[ _( "VSCPMASK" ) ] );
        }
        else {
            webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_MISSING_DATA );
        }

        webserv_rest_doSetFilter( nc, pSession, format, vscpfilter );

    }

    //   ****************************************************
    //   * * * * * * * *  clear input queue   * * * * * * * *
    //   ****************************************************
    else if ( ( _("6") == keypairs[_("OP")] ) || ( _("CLEARQUEUE") == keypairs[_("OP")].Upper() ) ) {
        webserv_rest_doClearQueue( nc, pSession, format );
    }
    
    //   ****************************************************
    //   * * * * * * * * read variable value  * * * * * * * *
    //   ****************************************************
    else if ( ( _("7") == keypairs[_("OP")] ) || ( _("READVAR") == keypairs[_("OP")].Upper() ) ) {
        if ( _("") != keypairs[_("VARIABLE")] ) {
            webserv_rest_doReadVariable( nc, pSession, format, keypairs[_("VARIABLE")] );
        }
        else {
            webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_MISSING_DATA );
        }
    }

    //   *****************************************************
    //   * * * * * * * * Write variable value  * * * * * * * *
    //   *****************************************************
    else if ( ( _("8") == keypairs[_("OP")] ) || ( _("WRITEVAR") == keypairs[_("OP")].Upper() ) ) {

        if ( _("") != keypairs[_("VARIABLE")] ) {
            webserv_rest_doWriteVariable( nc, pSession, format, keypairs[_("VARIABLE")], keypairs[ _( "VALUE" ) ] );
        }
        else {
            webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_MISSING_DATA );
        }
    }


    //   *****************************************************
    //   * * * * * * * *   Create variable    * * * * * * * *
    //   *****************************************************

    else if ( ( _("9") == keypairs[_("OP")] ) || ( _("CREATEVAR") == keypairs[_("OP")].Upper() ) ) {


        if ( _("") != keypairs[_("VARIABLE")] ) {
            webserv_rest_doCreateVariable( nc, 
                                                    pSession, 
                                                    format, 
                                                    keypairs[_("VARIABLE")], 
                                                    keypairs[ _( "TYPE" ) ],
                                                    keypairs[ _( "VALUE" ) ],
                                                    keypairs[ _( "PERISTENT" ) ]  );
        }
        else {
            webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_MISSING_DATA );
        }
    }


    //   *************************************************
    //   * * * * * * * * Send measurement  * * * * * * * *
    //   *************************************************
    //   value,unit=0,sensor=0
    //
    else if ( ( _("10") == keypairs[_("OP")] ) || ( _("MEASUREMENT") == keypairs[_("OP")].Upper() ) ) {

        if ( ( _("") != keypairs[_("VALUE")] ) && (_("") != keypairs[_("TYPE")]) ) {
            
            webserv_rest_doWriteMeasurement( nc, pSession, format,
                                                    keypairs[ _("GUID" ) ],
                                                    keypairs[ _("LEVEL") ],
                                                    keypairs[ _("TYPE") ],
                                                    keypairs[ _("VALUE") ],
                                                    keypairs[ _("UNIT") ],
                                                    keypairs[ _("SENSORIDX") ],
                                                    keypairs[ _( "ZONE" ) ],
                                                    keypairs[ _( "SUBZONE" ) ],
                                                    keypairs[ _( "SUBZONE" ) ] );
        }
        else {
            webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_MISSING_DATA );
        }
    }

    //   *******************************************
    //   * * * * * * * * Table read  * * * * * * * *
    //   *******************************************
    else if ( ( _("11") == keypairs[_("OP")] ) || ( _("TABLE") == keypairs[_("OP")].Upper() ) ) {

        if ( _("") != keypairs[_("NAME")] ) {
            
            webserv_rest_doGetTableData( nc, pSession, format, 
                                                    keypairs[_("NAME")],
                                                    keypairs[_("FROM")],
                                                    keypairs[_("TO")] );
        }
        else {
            webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_MISSING_DATA );
        }        
    }

    //   *******************************************
    //   * * * * * * * * Fetch MDF  * * * * * * * *
    //   *******************************************
    else if ( ( _( "12" ) == keypairs[ _( "OP" ) ] ) || ( _( "MDF" ) == keypairs[ _( "OP" ) ].Upper() ) ) {

        if ( _( "" ) != keypairs[ _( "URL" ) ] ) {

            webserv_rest_doFetchMDF( nc, pSession, format, keypairs[ _( "URL" ) ] );
        }
        else {
            webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_MISSING_DATA );
        }
    }

    return;
}



///////////////////////////////////////////////////////////////////////////////
// webserv_rest_error
//

void
VSCPWebServerThread::webserv_rest_error( struct mg_connection *nc, 
                                            struct websrv_rest_session *pSession, 
                                            int format,
                                            int errorcode)
{
    char buf[2048];
    int returncode = 200;

    if ( REST_FORMAT_PLAIN == format ) {

        webserv_util_sendheader( nc, returncode, REST_MIME_TYPE_PLAIN );   
        mg_send_http_chunk( nc, rest_errors[errorcode][REST_FORMAT_PLAIN], strlen( rest_errors[errorcode][REST_FORMAT_PLAIN] )  );
        mg_send_http_chunk( nc, "", 0 );   // Terminator
        return;
        
    }
    else if ( REST_FORMAT_CSV == format ) {

        webserv_util_sendheader( nc, returncode, REST_MIME_TYPE_CSV );     
        mg_send_http_chunk( nc, rest_errors[errorcode][REST_FORMAT_CSV], strlen( rest_errors[errorcode][REST_FORMAT_CSV] )  );
        mg_send_http_chunk( nc, "", 0 );   // Terminator
        return;
        
    }
    else if ( REST_FORMAT_XML == format ) {

        webserv_util_sendheader( nc, returncode, REST_MIME_TYPE_XML );
        mg_send_http_chunk( nc, XML_HEADER, strlen( XML_HEADER ) );            
        mg_send_http_chunk( nc, rest_errors[errorcode][REST_FORMAT_XML], strlen( rest_errors[errorcode][REST_FORMAT_XML] ) );
        mg_send_http_chunk( nc, "", 0 );   // Terminator
        return;
        
    }
    else if ( REST_FORMAT_JSON == format ) {

        webserv_util_sendheader( nc, returncode, REST_MIME_TYPE_JSON );
        mg_send_http_chunk( nc, rest_errors[errorcode][REST_FORMAT_JSON], strlen( rest_errors[errorcode][REST_FORMAT_JSON] ) );
        mg_send_http_chunk( nc, "", 0 );   // Terminator
        return;
        
    }
    else if ( REST_FORMAT_JSONP == format ) {

        webserv_util_sendheader( nc, returncode, REST_MIME_TYPE_JSONP );
        mg_send_http_chunk( nc, rest_errors[errorcode][REST_FORMAT_JSONP], strlen( rest_errors[errorcode][REST_FORMAT_JSONP] ) );
        mg_send_http_chunk( nc, "", 0 );   // Terminator
        return;
        
    }
    else {

        webserv_util_sendheader( nc, returncode, REST_MIME_TYPE_PLAIN );
        mg_send_http_chunk( nc, REST_PLAIN_ERROR_UNSUPPORTED_FORMAT, strlen( REST_PLAIN_ERROR_UNSUPPORTED_FORMAT ) );
        mg_send_http_chunk( nc, "", 0 );   // Terminator
        return;
        
    }
}

///////////////////////////////////////////////////////////////////////////////
// webserv_rest_sendHeader
//

void
VSCPWebServerThread::webserv_rest_sendHeader( struct mg_connection *nc,
                                                int format,
                                                int returncode )
{
    if ( REST_FORMAT_PLAIN == format ) {
        webserv_util_sendheader( nc, returncode, REST_MIME_TYPE_PLAIN );
    }
    else if ( REST_FORMAT_CSV == format ) {
        webserv_util_sendheader( nc, returncode, REST_MIME_TYPE_CSV );
    }
    else if ( REST_FORMAT_XML == format ) {
        webserv_util_sendheader( nc, returncode, REST_MIME_TYPE_XML );
    }
    else if ( REST_FORMAT_JSON == format ) {
        webserv_util_sendheader( nc, returncode, REST_MIME_TYPE_JSON );
    }
    else if ( REST_FORMAT_JSONP == format ) {
        webserv_util_sendheader( nc, returncode, REST_MIME_TYPE_JSONP );
    }

}

///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doOpen
//

void
VSCPWebServerThread::webserv_rest_doOpen( struct mg_connection *nc,
struct websrv_rest_session *pSession,
    CUserItem *pUser,
    int format )
{
    char buf[ 2048 ];
    char wrkbuf[ 256 ];

    pSession = websrv_new_rest_session( nc, pUser );
    if ( NULL != pSession ) {

        // New session created

        if ( REST_FORMAT_PLAIN == format ) {

            webserv_util_sendheader( nc, 200, REST_MIME_TYPE_PLAIN );
  
#ifdef WIN32
            int n = _snprintf( wrkbuf, 
                                sizeof( wrkbuf ), 
                                "1 1 Success vscpsession=%s nEvents=%zd", 
                                pSession->sid, 
                                pSession->pClientItem->m_clientInputQueue.GetCount() );
#else
            int n = snprintf( wrkbuf, 
                                sizeof( wrkbuf ), 
                                "1 1 Success vscpsession=%s nEvents=%lu", 
                                pSession->sid, 
                                pSession->pClientItem->m_clientInputQueue.GetCount() );
#endif
            mg_send_http_chunk( nc, wrkbuf, n );
            mg_send_http_chunk( nc, "", 0 );  // Terminator
            return;
        }
        else if ( REST_FORMAT_CSV == format ) {

            webserv_util_sendheader( nc, 200, REST_MIME_TYPE_CSV );

#ifdef WIN32
            int n = _snprintf( wrkbuf,
                               sizeof( wrkbuf ),
                               "success-code,error-code,message,description,vscpsession,nEvents\r\n1,1,Success,Success. 1,1,Success,Success,%s,%zd",
                               pSession->sid, pSession->pClientItem->m_clientInputQueue.GetCount() );
#else
            int n = snprintf( wrkbuf,
                              sizeof( wrkbuf ),
                              "success-code,error-code,message,description,vscpsession,nEvents\r\n1,1,Success,Success. 1,1,Success,Success,%s,%lu",
                              pSession->sid, pSession->pClientItem->m_clientInputQueue.GetCount() );
#endif
            mg_send_http_chunk( nc, buf, strlen( buf ) );
            mg_send_http_chunk( nc, "", 0 );  // Terminator
            return;
        }
        else if ( REST_FORMAT_XML == format ) {

            webserv_util_sendheader( nc, 200, REST_MIME_TYPE_XML );

#ifdef WIN32
            int n = _snprintf( wrkbuf,
                               sizeof( wrkbuf ),
                               "<vscp-rest success = \"true\" code = \"1\" message = \"Success.\" description = \"Success.\" ><vscpsession>%s</vscpsession><nEvents>%zd</nEvents></vscp-rest>",
                               pSession->sid, pSession->pClientItem->m_clientInputQueue.GetCount() );
#else
            int n = snprintf( wrkbuf,
                              sizeof( wrkbuf ),
                              "<vscp-rest success = \"true\" code = \"1\" message = \"Success.\" description = \"Success.\" ><vscpsession>%s</vscpsession><nEvents>%lu</nEvents></vscp-rest>",
                              pSession->sid, pSession->pClientItem->m_clientInputQueue.GetCount() );
#endif
            mg_send_http_chunk( nc, buf, strlen( buf ) );
            mg_send_http_chunk( nc, "", 0 );  // Terminator
            return;
        }
        else if ( REST_FORMAT_JSON == format ) {

            webserv_util_sendheader( nc, 200, REST_MIME_TYPE_JSON );

#ifdef WIN32
            int n = _snprintf( wrkbuf,
                               sizeof( wrkbuf ),
                               "{\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\",\"vscpsession\":\"%s\",\"nEvents\":%zd}",
                               pSession->sid,
                               pSession->pClientItem->m_clientInputQueue.GetCount() );
#else
            int n = snprintf( wrkbuf,
                              sizeof( wrkbuf ),
                              "{\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\",\"vscpsession\":\"%s\",\"nEvents\":%lu}",
                              pSession->sid, pSession->pClientItem->m_clientInputQueue.GetCount() );
#endif
            mg_send_http_chunk( nc, buf, strlen( buf ) );
            mg_send_http_chunk( nc, "", 0 );  // Terminator
            return;
        }
        else if ( REST_FORMAT_JSONP == format ) {

            webserv_util_sendheader( nc, 200, REST_MIME_TYPE_JSONP );

#ifdef WIN32
            // typeof handler === 'function' && 
            int n = _snprintf( wrkbuf,
                               sizeof( wrkbuf ),
                               "typeof handler === 'function' && handler({\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\",\"vscpsession\":\"%s\",\"nEvents\":%zd});",
                               pSession->sid, pSession->pClientItem->m_clientInputQueue.GetCount() );
#else
            int n = snprintf( wrkbuf,
                              sizeof( wrkbuf ),
                              "typeof handler === 'function' && handler({\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\",\"vscpsession\":\"%s\",\"nEvents\":%lu});",
                              pSession->sid, pSession->pClientItem->m_clientInputQueue.GetCount() );
#endif
            mg_send_http_chunk( nc, buf, strlen( buf ) );
            mg_send_http_chunk( nc, "", 0 ); // Terminator
            return;
        }
        else {
            webserv_util_sendheader( nc, 400, REST_MIME_TYPE_PLAIN );
            mg_send_http_chunk( nc, REST_PLAIN_ERROR_UNSUPPORTED_FORMAT, strlen( REST_PLAIN_ERROR_UNSUPPORTED_FORMAT ) );
            mg_send_http_chunk( nc, "", 0 ); // Terminator

            return;
        }
    }
    else {      // Unable to create session
        webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
    }

    return;
}

///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doClose
//

void
VSCPWebServerThread::webserv_rest_doClose( struct mg_connection *nc,
struct websrv_rest_session *pSession,
    int format )
{
    char buf[ 2048 ];
    char wrkbuf[ 256 ];

    if ( NULL != pSession ) {

        char sid[ 32 + 1 ];
        memset( sid, 0, sizeof( sid ) );
        memcpy( sid, pSession->sid, sizeof( sid ) );

        // We should close the session
        pSession->pClientItem->m_bOpen = false;
        pSession->lastActiveTime = 0;
        websrv_expire_rest_sessions( nc );

        if ( REST_FORMAT_PLAIN == format ) {

            webserv_util_sendheader( nc, 200, REST_MIME_TYPE_PLAIN );
            mg_send_http_chunk( nc, REST_PLAIN_ERROR_SUCCESS, strlen( REST_PLAIN_ERROR_SUCCESS ) );
            mg_send_http_chunk( nc, "", 0 );  // Terminator
            return;

        }
        else if ( REST_FORMAT_CSV == format ) {

            webserv_util_sendheader( nc, 200, REST_MIME_TYPE_CSV );
#ifdef WIN32
            int n = _snprintf( wrkbuf, sizeof( wrkbuf ), REST_CSV_ERROR_SUCCESS );
#else
            int n = snprintf( wrkbuf, sizeof( wrkbuf ), REST_CSV_ERROR_SUCCESS );
#endif
            mg_send_http_chunk( nc, buf, strlen( buf ) );
            mg_send_http_chunk( nc, "", 0 );  // Terminator
            return;
        }
        else if ( REST_FORMAT_XML == format ) {

            webserv_util_sendheader( nc, 200, REST_MIME_TYPE_XML );
#ifdef WIN32
            int n = _snprintf( wrkbuf, sizeof( wrkbuf ), REST_XML_ERROR_SUCCESS );
#else
            int n = snprintf( wrkbuf, sizeof( wrkbuf ), REST_XML_ERROR_SUCCESS );
#endif
            mg_send_http_chunk( nc, buf, strlen( buf ) );
            mg_send_http_chunk( nc, "", 0 );  // Terminator
            return;
        }
        else if ( REST_FORMAT_JSON == format ) {

            webserv_util_sendheader( nc, 200, REST_MIME_TYPE_JSON );
#ifdef WIN32
            int n = _snprintf( wrkbuf, sizeof( wrkbuf ), REST_JSON_ERROR_SUCCESS );
#else
            int n = snprintf( wrkbuf, sizeof( wrkbuf ), REST_JSON_ERROR_SUCCESS );
#endif
            mg_send_http_chunk( nc, buf, strlen( buf ) );
            mg_send_http_chunk( nc, "", 0 );  // Terminator
            return;
        }
        else if ( REST_FORMAT_JSONP == format ) {

            webserv_util_sendheader( nc, 200, REST_MIME_TYPE_JSONP );
#ifdef WIN32
            int n = _snprintf( wrkbuf, sizeof( wrkbuf ), REST_JSONP_ERROR_SUCCESS );
#else
            int n = snprintf( wrkbuf, sizeof( wrkbuf ), REST_JSONP_ERROR_SUCCESS );
#endif
            mg_send_http_chunk( nc, buf, strlen( buf ) );
            mg_send_http_chunk( nc, "", 0 );  // Terminator
            return;
        }
        else {
            webserv_util_sendheader( nc, 400, REST_MIME_TYPE_PLAIN );
            mg_send_http_chunk( nc, 
                            REST_PLAIN_ERROR_UNSUPPORTED_FORMAT, 
                            strlen( REST_PLAIN_ERROR_UNSUPPORTED_FORMAT ) );
            mg_send_http_chunk( nc, "", 0 ); // Terminator
            return;
        }

    }
    else {  // session not found
        webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
    }

    return;
}

///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doStatus
//

void
VSCPWebServerThread::webserv_rest_doStatus( struct mg_connection *nc,
                                                struct websrv_rest_session *pSession,
    int format )
{
    char buf[ 2048 ];
    char wrkbuf[ 256 ];

    if ( NULL != pSession ) {

        pSession->lastActiveTime = time( NULL );

        if ( REST_FORMAT_PLAIN == format ) {
            webserv_util_sendheader( nc, 200, REST_MIME_TYPE_PLAIN );
            mg_send_http_chunk( nc, REST_PLAIN_ERROR_SUCCESS, strlen( REST_PLAIN_ERROR_SUCCESS ) );
            memset( buf, 0, sizeof( buf ) );
#ifdef WIN32
            int n = _snprintf( wrkbuf, sizeof( wrkbuf ), "vscpsession=%s nEvents=%zd", pSession->sid, pSession->pClientItem->m_clientInputQueue.GetCount() );
#else
            int n = snprintf( wrkbuf,
                              sizeof( wrkbuf ),
                              "1 1 Success vscpsession=%s nEvents=%lu",
                              pSession->sid,
                              pSession->pClientItem->m_clientInputQueue.GetCount() );
#endif
            mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ) );
            mg_send_http_chunk( nc, "", 0 );  // Terminator
            return;
        }
        else if ( REST_FORMAT_CSV == format ) {
            webserv_util_sendheader( nc, 200, REST_MIME_TYPE_CSV );
#ifdef WIN32
            int n = _snprintf( wrkbuf,
                               sizeof( wrkbuf ),
                               "success-code,error-code,message,description,vscpsession,nEvents\r\n1,1,Success,Success. 1,1,Success,Sucess,%s,%zd",
                               pSession->sid,
                               pSession->pClientItem->m_clientInputQueue.GetCount() );
#else
            int n = snprintf( wrkbuf,
                              sizeof( wrkbuf ),
                              "success-code,error-code,message,description,vscpsession,nEvents\r\n1,1,Success,Success. 1,1,Success,Sucess,%s,%lu",
                              pSession->sid, pSession->pClientItem->m_clientInputQueue.GetCount() );
#endif
            mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ) );
            mg_send_http_chunk( nc, "", 0 );  // Terminator
            return;
        }
        else if ( REST_FORMAT_XML == format ) {
            webserv_util_sendheader( nc, 200, REST_MIME_TYPE_XML );
#ifdef WIN32
            int n = _snprintf( wrkbuf,
                               sizeof( wrkbuf ),
                               "<vscp-rest success = \"true\" code = \"1\" message = \"Success.\" description = \"Success.\" ><vscpsession>%s</vscpsession><nEvents>%zd</nEvents></vscp-rest>",
                               pSession->sid,
                               pSession->pClientItem->m_clientInputQueue.GetCount() );
#else
            int n = snprintf( wrkbuf,
                              sizeof( wrkbuf ),
                              "<vscp-rest success = \"true\" code = \"1\" message = \"Success.\" description = \"Success.\" ><vscpsession>%s</vscpsession><nEvents>%lu</nEvents></vscp-rest>",
                              pSession->sid, pSession->pClientItem->m_clientInputQueue.GetCount() );
#endif
            mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ) );
            mg_send_http_chunk( nc, "", 0 );  // Terminator
            return;
        }
        else if ( REST_FORMAT_JSON == format ) {

            webserv_util_sendheader( nc, 200, REST_MIME_TYPE_JSON );
#ifdef WIN32
            int n = _snprintf( wrkbuf,
                               sizeof( wrkbuf ),
                               "{\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\",\"vscpsession\":\"%s\",\"nEvents\":%zd}",
                               pSession->sid,
                               pSession->pClientItem->m_clientInputQueue.GetCount() );
#else
            int n = snprintf( wrkbuf,
                              sizeof( wrkbuf ),
                              "{\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\",\"vscpsession\":\"%s\",\"nEvents\":%lu}",
                              pSession->sid, pSession->pClientItem->m_clientInputQueue.GetCount() );
#endif
            mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ) );
            mg_send_http_chunk( nc, "", 0 );  // Terminator
            return;
        }
        else if ( REST_FORMAT_JSONP == format ) {

            webserv_util_sendheader( nc, 200, REST_MIME_TYPE_JSONP );
#ifdef WIN32
            int n = _snprintf( wrkbuf,
                               sizeof( wrkbuf ),
                               "typeof handler === 'function' && handler({\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\",\"vscpsession\":\"%s\",\"nEvents\":%zd});",
                               pSession->sid,
                               pSession->pClientItem->m_clientInputQueue.GetCount() );
#else
            int n = snprintf( wrkbuf,
                              sizeof( wrkbuf ),
                              "typeof handler === 'function' && handler({\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\",\"vscpsession\":\"%s\",\"nEvents\":%lu});",
                              pSession->sid, pSession->pClientItem->m_clientInputQueue.GetCount() );
#endif
            mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ) );
            mg_send_http_chunk( nc, "", 0 );  // Terminator
            return;
        }
        else {
            webserv_util_sendheader( nc, 400, REST_MIME_TYPE_PLAIN );
            mg_send_http_chunk( nc, REST_PLAIN_ERROR_UNSUPPORTED_FORMAT, strlen( REST_PLAIN_ERROR_UNSUPPORTED_FORMAT ) );
            mg_send_http_chunk( nc, "", 0 );  // Terminator
            return;
        }

    } // No session
    else {
        webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
    }

    return;
}

///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doSendEvent
//

void
VSCPWebServerThread::webserv_rest_doSendEvent( struct mg_connection *nc, 
                                                    struct websrv_rest_session *pSession, 
                                                    int format, 
                                                    vscpEvent *pEvent )
{
    bool bSent = false;

    // Check pointer
    if (NULL == nc) return;
    
    CControlObject *pObject = (CControlObject *)nc->mgr->user_data;
    if (NULL == pObject) return;

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

                    webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_SUCCESS );

                }
                else {
                    webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_NO_ROOM );
                    vscp_deleteVSCPevent( pEvent );
                    bSent = false;
                } 

            }
            else {
                webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_GENERAL_FAILURE );
                vscp_deleteVSCPevent( pEvent );
                bSent = false;
            }            

        } // not sent
    }
    else {
        webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
        bSent = false;
    }

    return;
}


///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doReadEvent
//

void
VSCPWebServerThread::webserv_rest_doReceiveEvent( struct mg_connection *nc, 
                                                        struct websrv_rest_session *pSession, 
                                                        int format,
                                                        size_t count )
{
    // Check pointer
    if (NULL == nc) return;
    
    CControlObject *pObject = (CControlObject *)nc->mgr->user_data;
    if (NULL == pObject) return;

    if ( NULL != pSession ) {

        if ( !pSession->pClientItem->m_clientInputQueue.empty() ) {

            char buf[32000];
            char wrkbuf[32000];
            wxString out;
            size_t cntAvailable = pSession->pClientItem->m_clientInputQueue.GetCount();

            // Plain
            if ( REST_FORMAT_PLAIN == format ) {

                // Send header
                webserv_util_sendheader( nc, 200, REST_MIME_TYPE_PLAIN );

                if ( pSession->pClientItem->m_bOpen && cntAvailable ) {

                    sprintf( wrkbuf, "1 1 Success \r\n");
                    mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ) );
                    sprintf( wrkbuf, 
#if WIN32
                                "%zd events requested of %zd available (unfiltered) %zu will be retrieved\r\n",
#else
                                "%zd events requested of %zd available (unfiltered) %lu will be retrieved\r\n", 
#endif
                                count, 
                                pSession->pClientItem->m_clientInputQueue.GetCount(),
                                MIN( count, cntAvailable ) );

                    mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ) );

                    for ( unsigned int i=0; i<MIN( count, cntAvailable ); i++ ) {

                        CLIENTEVENTLIST::compatibility_iterator nodeClient;
                        vscpEvent *pEvent;

                        pSession->pClientItem->m_mutexClientInputQueue.Lock();
                        nodeClient = pSession->pClientItem->m_clientInputQueue.GetFirst();
                        pEvent = nodeClient->GetData();
                        pSession->pClientItem->m_clientInputQueue.DeleteNode( nodeClient );
                        pSession->pClientItem->m_mutexClientInputQueue.Unlock();

                        if (NULL != pEvent) {

                            if ( vscp_doLevel2Filter( pEvent, &pSession->pClientItem->m_filterVSCP ) ) {

                                wxString str;
                                if ( vscp_writeVscpEventToString( pEvent, str ) ) {

                                    // Write it out
                                    strcpy((char *) wrkbuf, (const char*) "- ");
                                    strcat((char *) wrkbuf, (const char*) str.mb_str(wxConvUTF8));
                                    strcat((char *) wrkbuf, "\r\n" );
                                    mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf) );
                            
                                }
                                else {
                                    strcpy((char *) wrkbuf, "- Malformed event (intenal error)\r\n" );
                                    mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf) );
                                }
                            }
                            else {
                                strcpy((char *) wrkbuf, "- Event filtered out\r\n" );
                                mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf) );
                            }

                            // Remove the event
                            vscp_deleteVSCPevent(pEvent);

                        } // Valid pEvent pointer
                        else {
                            strcpy((char *) wrkbuf, "- Event could not be fetched (intenal error)\r\n" );
                            mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf) );
                        }
                    } // for
                }
                else {   // no events available
                    sprintf( wrkbuf, REST_PLAIN_ERROR_INPUT_QUEUE_EMPTY"\r\n");
                    mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf) );
                }
                
            }

            // CSV
            else if ( REST_FORMAT_CSV == format ) {

                // Send header
                webserv_util_sendheader( nc, 200, /*REST_MIME_TYPE_CSV*/ REST_MIME_TYPE_PLAIN );                
                
                if ( pSession->pClientItem->m_bOpen && cntAvailable ) {

                    sprintf( wrkbuf, "success-code,error-code,message,description,Event\r\n1,1,Success,Success.,NULL\r\n");
                    mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf) );
                    sprintf( wrkbuf, 
#if WIN32
                             "1,2,Info,%zd events requested of %d available (unfiltered) %zu will be retrieved,NULL\r\n",
#else
                             "1,2,Info,%zd events requested of %ul available (unfiltered) %lu will be retrieved,NULL\r\n", 
#endif
                                count, 
                                cntAvailable,                
                                MIN( count, cntAvailable ) );
                    mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ) );
                    sprintf( wrkbuf,
                             "1,4,Count,%zu,NULL\r\n",
                             MIN( count, cntAvailable ) );
                    mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ) );

                    for ( unsigned int i=0; i<MIN( count, cntAvailable ); i++ ) {

                        CLIENTEVENTLIST::compatibility_iterator nodeClient;
                        vscpEvent *pEvent;

                        pSession->pClientItem->m_mutexClientInputQueue.Lock();
                        nodeClient = pSession->pClientItem->m_clientInputQueue.GetFirst();
                        pEvent = nodeClient->GetData();
                        pSession->pClientItem->m_clientInputQueue.DeleteNode(nodeClient);
                        pSession->pClientItem->m_mutexClientInputQueue.Unlock();

                        if ( NULL != pEvent ) {

                            if ( vscp_doLevel2Filter( pEvent, &pSession->pClientItem->m_filterVSCP ) ) {

                                wxString str;
                                if ( vscp_writeVscpEventToString(pEvent, str) ) {

                                    // Write it out
                                    memset((char *) wrkbuf, 0, sizeof( wrkbuf ));
                                    strcpy((char *) wrkbuf, (const char*) "1,3,Data,Event,");
                                    strcat((char *) wrkbuf, (const char*) str.mb_str(wxConvUTF8));
                                    strcat((char *) wrkbuf, "\r\n" );
                                    mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf) );
                            
                                }
                                else {
                                    strcpy((char *) wrkbuf, "1,2,Info,Malformed event (intenal error)\r\n" );
                                    mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf)  );
                                }
                            }
                            else {
                                strcpy((char *) wrkbuf, "1,2,Info,Event filtered out\r\n" );
                                mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf) );
                            }

                            // Remove the event
                            vscp_deleteVSCPevent( pEvent );

                        } // Valid pEvent pointer
                        else {
                            strcpy((char *) wrkbuf, "1,2,Info,Event could not be fetched (intenal error)\r\n" );
                            mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf) );
                        }
                    } // for
                } 
                else {   // no events available
                    sprintf( wrkbuf, REST_CSV_ERROR_INPUT_QUEUE_EMPTY"\r\n");
                    mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf) );
                }

            }

            // XML
            else if ( REST_FORMAT_XML == format ) {

                int filtered = 0;
                int errors = 0;

                // Send header
                webserv_util_sendheader( nc, 200, REST_MIME_TYPE_XML );                
                

                if ( pSession->pClientItem->m_bOpen && cntAvailable ) {
                    sprintf( wrkbuf, XML_HEADER"<vscp-rest success = \"true\" code = \"1\" message = \"Success\" description = \"Success.\" >");
                    mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf) );
                    sprintf( wrkbuf, 
                                "<info>%zd events requested of %lu available (unfiltered) %zu will be retrieved</info>", 
                                count, 
                                cntAvailable,
                                MIN(count, cntAvailable ) );
                    mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf) );

                    sprintf( wrkbuf,
                             "<count>%zu</count>",
                             MIN( count, cntAvailable ) );
                    mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ) );

                    for ( unsigned int i=0; i<MIN( (unsigned long)count, cntAvailable ); i++ ) {

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
                                strcpy((char *)wrkbuf, (const char*) "<event>");

                                strcat((char *)wrkbuf, (const char*) "<head>");
                                strcat((char *)wrkbuf, wxString::Format( _("%d"), pEvent->head ).mbc_str() );
                                strcat((char *)wrkbuf, (const char*) "</head>");

                                strcat((char *)wrkbuf, (const char*) "<vscpclass>");
                                strcat((char *)wrkbuf, wxString::Format( _("%d"), pEvent->vscp_class ).mbc_str() );
                                strcat((char *)wrkbuf, (const char*) "</vscpclass>");

                                strcat((char *)wrkbuf, (const char*) "<vscptype>");
                                strcat((char *)wrkbuf, wxString::Format( _("%d"), pEvent->vscp_type ).mbc_str() );
                                strcat((char *)wrkbuf, (const char*) "</vscptype>");

                                strcat((char *)wrkbuf, (const char*) "<obid>");
                                strcat((char *)wrkbuf, wxString::Format( _("%lu"), pEvent->obid ).mbc_str() );
                                strcat((char *)wrkbuf, (const char*) "</obid>");

                                strcat((char *)wrkbuf, (const char*) "<timestamp>");
                                strcat((char *)wrkbuf, wxString::Format( _("%lu"), pEvent->timestamp ).mbc_str() );
                                strcat((char *)wrkbuf, (const char*) "</timestamp>");

                                strcat((char *)wrkbuf, (const char*) "<guid>");
                                vscp_writeGuidToString( pEvent, str);
                                strcat((char *)wrkbuf, (const char *)str.mbc_str() );
                                strcat((char *)wrkbuf, (const char*) "</guid>");

                                strcat((char *)wrkbuf, (const char*) "<sizedata>");
                                strcat((char *)wrkbuf, wxString::Format( _("%d"), pEvent->sizeData ).mbc_str() );
                                strcat((char *)wrkbuf, (const char*) "</sizedata>");

                                strcat((char *)wrkbuf, (const char*) "<data>");
                                vscp_writeVscpDataToString( pEvent, str);
                                strcat((char *)wrkbuf, (const char *)str.mbc_str() );
                                strcat((char *)wrkbuf, (const char*) "</data>");

                                if ( vscp_writeVscpEventToString( pEvent, str ) ) {
                                    strcat( ( char * )wrkbuf, ( const char* ) "<raw>" );
                                    strcat( ( char * )wrkbuf, ( const char* )str.mbc_str() );
                                    strcat( ( char * )wrkbuf, ( const char* ) "</raw>" );
                                }

                                strcat((char *)wrkbuf, "</event>" );
                                mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf) );

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
                    strcat((char *) wrkbuf, wxString::Format( _("%d"), filtered ).mbc_str() );
                    strcat((char *) wrkbuf, (const char*) "</filtered>");

                    strcat((char *) wrkbuf, (const char*) "<errors>");
                    strcat((char *) wrkbuf, wxString::Format( _("%d"), errors ).mbc_str() );
                    strcat((char *) wrkbuf, (const char*) "</errors>");

                    mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ) );

                    // End tag
                    strcpy((char *) wrkbuf, "</vscp-rest>" );
                    mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf) );

                }     
                else {   // no events available

                    sprintf( wrkbuf, REST_XML_ERROR_INPUT_QUEUE_EMPTY"\r\n");
                    mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf) );
                }

            }

            // JSON / JSONP
            else if ( ( REST_FORMAT_JSON == format ) || ( REST_FORMAT_JSONP == format ) ) {

                int sentEvents = 0; 
                int filtered = 0;
                int errors = 0;
                char *p;

                // Send header
                if ( REST_FORMAT_JSONP == format ) {
                    webserv_util_sendheader( nc, 200, REST_MIME_TYPE_JSONP );
                }
                else {
                    webserv_util_sendheader( nc, 200, REST_MIME_TYPE_JSON );
                }

                if ( pSession->pClientItem->m_bOpen && cntAvailable ) {

                    memset( buf, 0, sizeof( buf ) );
                    memset( wrkbuf, 0, sizeof( wrkbuf ) );
                    p = wrkbuf;

                    // typeof handler === 'function' &&
                    if ( REST_FORMAT_JSONP == format ) {
                        p += json_emit_unquoted_str( p, 
                                                       &wrkbuf[ sizeof( wrkbuf ) ] - p,
                                                       "typeof handler === 'function' && handler(",
                                                       strlen( "typeof handler === 'function' && handler(" ) );
                    }

                    p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p,
                        "{\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\",",
                        strlen("{\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\",") );
                    p += json_emit_quoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, "info", 4 );
                    p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ":", 1 );
                    {
                        char buf2[200];
                        sprintf( buf2,
                                 "\"%zd events requested of %lu available (unfiltered) %zd will be retrieved\"",
                                 count,
                                 cntAvailable,
                                 MIN( count, cntAvailable ) );
                        p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, buf2, strlen( buf2 ) );
                    }
                    p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ",", 1 );
                    p += json_emit_quoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, "event", 5);
                    p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ":[", 2 );

                    mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf));

                    for ( unsigned int i=0; i<MIN( count, cntAvailable ); i++ ) {

                        CLIENTEVENTLIST::compatibility_iterator nodeClient;
                        vscpEvent *pEvent;

                        pSession->pClientItem->m_mutexClientInputQueue.Lock();
                        nodeClient = pSession->pClientItem->m_clientInputQueue.GetFirst();
                        pEvent = nodeClient->GetData();
                        pSession->pClientItem->m_clientInputQueue.DeleteNode(nodeClient);
                        pSession->pClientItem->m_mutexClientInputQueue.Unlock();

                        if ( NULL != pEvent ) {

                            if ( vscp_doLevel2Filter( pEvent, &pSession->pClientItem->m_filterVSCP ) ) {

                                memset( buf, 0, sizeof( buf ) );
                                memset( wrkbuf, 0, sizeof( wrkbuf ) );
                                p = wrkbuf;

                                wxString str;
                                p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, "{", 1 );
                                
                                // head
                                p += json_emit_quoted_str(p, &wrkbuf[ sizeof( wrkbuf ) ] - p, "head", 4 );
                                p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ":", 1 );
                                p += json_emit_long( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, pEvent->head );
                                p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ",", 1 );
                                
                                // class
                                p += json_emit_quoted_str(p, &wrkbuf[ sizeof( wrkbuf ) ] - p, "vscpclass", 9 );
                                p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ":", 1 );
                                p += json_emit_long( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, pEvent->vscp_class );
                                p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ",", 1 );
                                
                                // type
                                p += json_emit_quoted_str(p, &wrkbuf[ sizeof( wrkbuf ) ] - p, "vscptype", 8);
                                p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ":", 1 );
                                p += json_emit_long( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, pEvent->vscp_type );
                                p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ",", 1 );
                                
                                // timestamp
                                p += json_emit_quoted_str(p, &wrkbuf[ sizeof( wrkbuf ) ] - p, "timestamp", 9 );
                                p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ":", 1 );
                                p += json_emit_long( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, pEvent->timestamp );
                                p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ",", 1 );

                                // obid
                                p += json_emit_quoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, "obid", 4 );
                                p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ":", 1 );
                                p += json_emit_long( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, pEvent->obid );
                                p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ",", 1 );
                                
                                // GUID
                                vscp_writeGuidToString( pEvent, str);
                                p += json_emit_quoted_str(p, &wrkbuf[ sizeof( wrkbuf ) ] - p, "guid", 4 );
                                p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ":", 1 );
                                p += json_emit_quoted_str(p, &wrkbuf[ sizeof( wrkbuf ) ] - p, str.mbc_str(), str.Length() );
                                p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ",", 1 );
                                
                                // SizeData
                                p += json_emit_quoted_str(p, &wrkbuf[ sizeof( wrkbuf ) ] - p, "sizedata", 8 );
                                p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ":", 1 );
                                p += json_emit_long( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, pEvent->sizeData );
                                p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ",", 1 );
                                
                                // Data
                                p += json_emit_quoted_str(p, &wrkbuf[ sizeof( wrkbuf ) ] - p, "data", 4);
                                p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ":[", 2 );
                                for ( uint16_t j=0; j<pEvent->sizeData; j++ ) {
                                    p += json_emit_long( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, pEvent->pdata[j] );
                                    if (j < ( pEvent->sizeData-1 ) ) {
                                        p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ",", 1 );
                                    }
                                }
                                p += json_emit_unquoted_str( p, &wrkbuf[sizeof( wrkbuf )] - p, "]}", 2 );

                                if ( i < ( MIN( count, cntAvailable ) - 1 ) ) {
                                    p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ",", 1 );
                                }

                                mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ) );
                                sentEvents++;

                            }
                            else {
                                filtered++;
                            }

                            // Remove the event
                            vscp_deleteVSCPevent( pEvent );

                        } // Valid pEvent pointer
                        else {
                            errors++;
                        }

                        // Buffer overflow
                        if ( 0 == p ) break;

                    } // for
                    
                    memset( buf, 0, sizeof( buf ) );
                    memset( wrkbuf, 0, sizeof( wrkbuf ) );
                    p = wrkbuf;

                    // Mark end
                    p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, "],", 2 );
                    p += json_emit_quoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, "count", 5 );
                    p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ":", 1 );
                    p += json_emit_long( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, sentEvents );
                    p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ",", 1 );
                    p += json_emit_quoted_str(p, &wrkbuf[ sizeof( wrkbuf ) ] - p, "filtered", 8 );
                    p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ":", 1 );
                    p += json_emit_long( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, filtered );
                    p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ",", 1 );
                    p += json_emit_quoted_str(p, &wrkbuf[ sizeof( wrkbuf ) ] - p, "errors", 6 );
                    p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ":", 1 );
                    p += json_emit_long( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, errors );
                    p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, "}", 1 );

                    if ( REST_FORMAT_JSONP == format ) {
                        p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ");", 2 );
                    }

                    mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ) );

                    memset( buf, 0, sizeof( buf ) );
                    memset( wrkbuf, 0, sizeof( wrkbuf ) );
                    p = wrkbuf;

                } // if open and data 
                else {   // no events available

                    if ( REST_FORMAT_JSON == format ) {
                        sprintf( wrkbuf, REST_JSON_ERROR_INPUT_QUEUE_EMPTY"\r\n");
                    }
                    else {
                        sprintf( wrkbuf, REST_JSONP_ERROR_INPUT_QUEUE_EMPTY"\r\n");
                    }

                    mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf) );

                }
                    
            }  // format
            
            mg_send_http_chunk( nc, "", 0 );

        }
        else {    // Queue is empty
            webserv_rest_error( nc, pSession, format, RESR_ERROR_CODE_INPUT_QUEUE_EMPTY );
        }

    }
    else {
            webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
    }

    return;
}


///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doSetFilter
//

void
VSCPWebServerThread::webserv_rest_doSetFilter( struct mg_connection *nc, 
                                                    struct websrv_rest_session *pSession, 
                                                    int format,
                                                    vscpEventFilter& vscpfilter )
{
    if ( NULL != pSession ) {
        
        pSession->pClientItem->m_mutexClientInputQueue.Lock();
        memcpy( &pSession->pClientItem->m_filterVSCP, &vscpfilter, sizeof( vscpEventFilter ) );
        pSession->pClientItem->m_mutexClientInputQueue.Unlock();
        webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_SUCCESS );
    }
    else {
        webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
    }

    return;
}


///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doClearQueue
//

void
VSCPWebServerThread::webserv_rest_doClearQueue( struct mg_connection *nc, 
                                                    struct websrv_rest_session *pSession, 
                                                    int format )
{
    // Check pointer
    if (NULL == nc) return;
    
    CControlObject *pObject = (CControlObject *)nc->mgr->user_data;
    if (NULL == pObject) return;


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
        
        webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_SUCCESS );
    }
    else {
        webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
    }

    return;
}


///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doReadVariable
//

void
VSCPWebServerThread::webserv_rest_doReadVariable( struct mg_connection *nc, 
                                                    struct websrv_rest_session *pSession, 
                                                    int format,
                                                    wxString& strVariableName )
{
    char buf[512];
    char wrkbuf[512];

    // Check pointer
    if (NULL == nc) return;
    
    CControlObject *pObject = (CControlObject *)nc->mgr->user_data;
    if (NULL == pObject) return;

    if ( NULL != pSession ) {

        CVSCPVariable *pvar;
        wxString strvalue;

        if ( NULL == (pvar = pObject->m_VSCP_Variables.find( strVariableName ) ) ) {
            webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_VARIABLE_NOT_FOUND );
            return;
        }

        pvar->writeValueToString( strvalue );

        // Get variable value
        wxString strVariableValue;
        pvar->writeValueToString( strVariableValue );
        
        if ( REST_FORMAT_PLAIN == format ) {

                webserv_util_sendheader( nc, 200, REST_MIME_TYPE_PLAIN );
                sprintf( wrkbuf, "1 1 Success \r\n");
                mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf) );

                sprintf( wrkbuf, 
                                "variable=%s type=%d persistent=%s value='%s' note='%s'\r\n", 
                                (const char *)pvar->getName().mbc_str(), 
                                pvar->getType(),
                                pvar->isPersistent() ? "true" : "false", 
                                (const char *)strVariableValue.mbc_str(),
                                (const char *)pvar->getNote().mbc_str() );
            
                mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf) );
                                        
        }
        else if ( REST_FORMAT_CSV == format ) {
            
            webserv_util_sendheader( nc, 200, REST_MIME_TYPE_CSV );
            sprintf( wrkbuf, 
                "success-code,error-code,message,description,Variable,Type,Persistent,Value,Note\r\n1,1,Success,Success.,%s,%d,%s,'%s','%s'\r\n",
                (const char *)strVariableName.mbc_str(), 
                pvar->getType(),
                pvar->isPersistent() ? "true" : "false",
                (const char *)strVariableValue.mbc_str(),
                (const char *)pvar->getNote().mbc_str() );
            mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ) );

        }
        else if ( REST_FORMAT_XML == format ) {

            webserv_util_sendheader( nc, 200, REST_MIME_TYPE_XML );
            sprintf( wrkbuf, XML_HEADER"<vscp-rest success = \"true\" code = \"1\" message = \"Success\" description = \"Success.\" >");
            mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf) );

            sprintf( wrkbuf, 
                        "<variable type=\"%d(%s)\" persistent=\"%s\" >",
                        pvar->getType(),
                        pvar->getVariableTypeAsString( pvar->getType() ),
                        pvar->isPersistent() ? "true" : "false" );
            mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf) );
            sprintf((char *) wrkbuf, 
                            "<name>%s</name><value>%s</value><note>%s</note>",
                            (const char *)pvar->getName().mbc_str(),
                            (const char *)strVariableValue.mbc_str(),
                            (const char *)pvar->getNote().mbc_str() );

            mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf) );

            // End tag
            strcpy((char *) wrkbuf, "</variable>" );
            mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf) );

            // End tag
            strcpy((char *) wrkbuf, "</vscp-rest>" );
            mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf));

        }
        else if ( ( REST_FORMAT_JSON == format ) || ( REST_FORMAT_JSONP == format ) ) {
                
            char *p = buf;
            if ( REST_FORMAT_JSONP == format ) {
                webserv_util_sendheader( nc, 200, REST_MIME_TYPE_JSONP );
            }
            else {
                webserv_util_sendheader( nc, 200, REST_MIME_TYPE_JSON );
            }

            if ( pSession->pClientItem->m_bOpen  ) {

                memset( buf, 0, sizeof( buf ) );

                if ( REST_FORMAT_JSONP == format ) {
                    p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, "typeof handler === 'function' && handler(", 41 );
                }

                p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, 
                    "{\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\",",
                    strlen( "{\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\"," ) );
                    
                p += json_emit_quoted_str( p, &buf[sizeof(buf)] - p, "varname", 7 );
                p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ":", 1 );                    
                p += json_emit_quoted_str( p, &buf[sizeof(buf)] - p, pvar->getName().mbc_str(), pvar->getName().Length() );
                p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ",", 1 );

                p += json_emit_quoted_str( p, &buf[sizeof(buf)] - p, "vartype", 7 );
                p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ":", 1 );            
                wxString wxstr = wxString::FromAscii( pvar->getVariableTypeAsString( pvar->getType() ) );
                p += json_emit_quoted_str( p, &buf[sizeof(buf)] - p, wxstr.mbc_str(), wxstr.Length() );
                p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ",", 1 );

                p += json_emit_quoted_str( p, &buf[sizeof(buf)] - p, "vartypecode", 11 );
                p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ":", 1 );
                p += json_emit_long( p, &buf[sizeof(buf)] - p, pvar->getType() );
                p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ",", 1 );

                p += json_emit_quoted_str( p, &buf[sizeof(buf)] - p, "varpersistence", 14 );
                p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ":", 1 );
                wxstr = pvar->isPersistent() ? _("true") : _("false");
                p += json_emit_quoted_str( p, &buf[sizeof(buf)] - p, wxstr.mbc_str(), wxstr.Length() );
                p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ",", 1 );

                p += json_emit_quoted_str( p, &buf[sizeof(buf)] - p, "varvalue", 8 );
                p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ":", 1 );                    
                p += json_emit_quoted_str( p, &buf[sizeof(buf)] - p, strVariableValue.mbc_str(), strVariableValue.Length() );
                p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ",", 1 );

                // Mark end
                p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, "}", 1 );

                if ( REST_FORMAT_JSONP == format ) {
                    p += json_emit_unquoted_str( p, &buf[sizeof(buf)] - p, ");", 2 );
                }

                mg_send_http_chunk( nc, buf, strlen( buf) );
                

            }                         
        }

        mg_send_http_chunk( nc, "", 0 );    // Terminator
        
    }
    else {
        webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
    }

    return;
}



///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doWriteVariable
//

void
VSCPWebServerThread::webserv_rest_doWriteVariable( struct mg_connection *nc, 
                                                        struct websrv_rest_session *pSession, 
                                                        int format,
                                                        wxString& strVariableName,
                                                        wxString& strValue )
{
    wxString strName;

    // Check pointer
    if (NULL == nc) {
        return;
    }

    CControlObject *pObject = (CControlObject *)nc->mgr->user_data;
    
    if (NULL == pObject) {
        return;
    }

    if ( NULL != pSession ) {

        // Get variablename
        CVSCPVariable *pvar;
        if ( NULL == (pvar = pObject->m_VSCP_Variables.find( strVariableName ) ) ) {
            webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_VARIABLE_NOT_FOUND );
            return;
        }

        // Set variable value
        if (!pvar->setValueFromString( pvar->getType(), strValue ) ) {
            webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_MISSING_DATA );
            return;
        }

        webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_SUCCESS );

    }
    else {
        webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
    }

    return;
}


///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doCreateVariable
//

void
VSCPWebServerThread::webserv_rest_doCreateVariable( struct mg_connection *nc, 
                                                        struct websrv_rest_session *pSession, 
                                                        int format,
                                                        wxString& strVariable,
                                                        wxString& strType,
                                                        wxString& strValue,
                                                        wxString& strPersistent )
{
    int type = VSCP_DAEMON_VARIABLE_CODE_STRING;
    bool bPersistence = false;
    wxStringTokenizer tkz( strVariable, _(";") );

    // Check pointer
    if (NULL == nc) {
        return;
    }

    // Get type
    if ( strType.IsNumber() ) {
        type = vscp_readStringValue( strType );
    }
    else {
        type = CVSCPVariable::getVariableTypeFromString( strType  );
    }

    // Get persistence
    strPersistent.Trim();
    strPersistent.Trim( false );
    strPersistent.MakeUpper();
    if ( wxNOT_FOUND != strPersistent.Find( _( "TRUE" ) ) ) {
        bPersistence = true;
    }

    CControlObject *pObject = (CControlObject *)nc->mgr->user_data;
    
    if (NULL == pObject) {
        webserv_rest_error( nc, 
                                pSession, 
                                format, 
                                REST_ERROR_CODE_VARIABLE_NOT_CREATED ); 
        return;
    }

    if ( NULL != pSession ) {

        // Add the variable
        if ( !pObject->m_VSCP_Variables.add( strVariable, 
                                                strValue, 
                                                type, 
                                                bPersistence  ) ) {
            webserv_rest_error( nc, 
                                    pSession, 
                                    format, 
                                    REST_ERROR_CODE_VARIABLE_NOT_CREATED );
            return;
        }

        webserv_rest_error( nc, 
                                pSession, 
                                format, 
                                REST_ERROR_CODE_SUCCESS );

    }
    else {
        webserv_rest_error( nc, 
                                pSession, 
                                format, 
                                REST_ERROR_CODE_INVALID_SESSION );
    }

    return;
}


///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doWriteMeasurement
//

void
VSCPWebServerThread::webserv_rest_doWriteMeasurement( struct mg_connection *nc, 
                                                        struct websrv_rest_session *pSession, 
                                                        int format,
                                                        wxString& strGuid,
                                                        wxString& strLevel,
                                                        wxString& strType,
                                                        wxString& strValue,
                                                        wxString& strUnit,
                                                        wxString& strSensorIdx,
                                                        wxString& strZone,
                                                        wxString& strSubZone,
                                                        wxString& strEventFormat )
{
    if ( NULL != pSession ) {

        double value = 0;
        uint8_t guid[ 16 ];
        long level = 2;
        long unit;
        long vscptype;
        long sensoridx;
        long zone;
        long subzone;
        long eventFormat = 0;    // float
        uint8_t data[ VSCP_MAX_DATA ];
        uint16_t sizeData;

        memset( guid, 0, 16 );
        vscp_getGuidFromStringToArray( guid, strGuid );

        strValue.Trim();
        strValue.Trim(false);
        strValue.ToDouble( &value );        // Measurement value
        strUnit.ToLong( &unit );            // Measurement unit
        strSensorIdx.ToLong( &sensoridx );  // Sensor indes
        strType.ToLong( &vscptype );        // VSCP event type
        strZone.ToLong( &zone );            // VSCP event type
        zone &= 0xff;
        strSubZone.ToLong( &subzone );      // VSCP event type
        subzone &= 0xff;

        strLevel.ToLong( &level );          // Level I or Level II (default) 
        if ( ( level > 2 ) || ( level < 1 ) ) {
            level = 2;
        }

        strEventFormat.Trim();
        strEventFormat.Trim( false );
        strEventFormat.MakeUpper();
        if ( wxNOT_FOUND != strEventFormat.Find( _( "STRING" ) ) ) {
            eventFormat = 1;
        }

        // Range checks
        if ( 1 == level ) {
            if ( unit > 3 ) unit = 0;
            if ( sensoridx > 7 ) unit = 0;
            if ( vscptype > 512 ) vscptype -= 512;
        }
        else if ( 2 == level ) {
            if ( unit > 255 ) unit &= 0xff;
            if ( sensoridx > 255 ) sensoridx &= 0xff;
        }

        if ( 1 == level ) {
            
            if ( 0 == eventFormat ) {

                // Floating point
                if ( vscp_convertFloatToFloatEventData( data,
                                                            &sizeData,
                                                            value,
                                                            unit,
                                                            sensoridx ) ) {
                    if ( sizeData > 8 ) sizeData = 8;

                    vscpEvent *pEvent = new vscpEvent;
                    if ( NULL == pEvent ) {
                        webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_GENERAL_FAILURE );
                        return;
                    }
                    pEvent->pdata = NULL;

                    pEvent->head = VSCP_PRIORITY_NORMAL;
                    pEvent->timestamp = 0; // Let interface fill in
                    memcpy( pEvent->GUID, guid, 16 );
                    pEvent->sizeData = sizeData;
                    if ( sizeData > 0 ) {
                        pEvent->pdata = new uint8_t[ sizeData ];
                        memcpy( pEvent->pdata, data, sizeData );
                    }
                    pEvent->vscp_class = VSCP_CLASS1_MEASUREMENT;
                    pEvent->vscp_type = vscptype;

                    webserv_rest_doSendEvent( nc, pSession, format, pEvent );

                }
                else {
                    webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_GENERAL_FAILURE );
                }
            }
            else {
                // String
                vscpEvent *pEvent = new vscpEvent;
                if ( NULL == pEvent ) {
                    webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_GENERAL_FAILURE );
                    return;
                }
                pEvent->pdata = NULL;

                if ( vscp_makeStringMeasurementEvent( pEvent,
                                                        value,
                                                        unit,
                                                        sensoridx ) ) {

                }
                else {
                    webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_GENERAL_FAILURE );
                }
            }
        }
        else {  // Level II
            if ( 0 == eventFormat ) {

                // Floating point
                vscpEvent *pEvent = new vscpEvent;
                if ( NULL == pEvent ) {
                    webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_GENERAL_FAILURE );
                    return;
                }

                pEvent->pdata = NULL;

                pEvent->obid = 0;
                pEvent->head = VSCP_PRIORITY_NORMAL;
                pEvent->timestamp = 0; // Let interface fill in
                memcpy( pEvent->GUID, guid, 16 );
                pEvent->head = 0;
                pEvent->vscp_class = VSCP_CLASS2_MEASUREMENT_FLOAT;
                pEvent->vscp_type = vscptype;
                pEvent->timestamp = 0;
                pEvent->sizeData = 12;

                data[ 0 ] = sensoridx;
                data[ 1 ] = zone;
                data[ 2 ] = subzone;
                data[ 3 ] = unit;

                memcpy( data + 4, ( uint8_t * )&value, 8 ); // copy in double
                wxUINT64_SWAP_ON_LE( data + 4 );
                // Copy in data
                pEvent->pdata = new uint8_t[ 4 + 8 ];
                if ( NULL == pEvent->pdata ) {
                    webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_GENERAL_FAILURE );
                    delete pEvent;
                    return;
                }
                memcpy( pEvent->pdata, data, 4 + 8 );

                webserv_rest_doSendEvent( nc, pSession, format, pEvent );
            }
            else {
                // String
                vscpEvent *pEvent = new vscpEvent;
                pEvent->pdata = NULL;

                pEvent->obid = 0;
                pEvent->head = VSCP_PRIORITY_NORMAL;
                pEvent->timestamp = 0; // Let interface fill in
                memcpy( pEvent->GUID, guid, 16 );
                pEvent->head = 0;                
                pEvent->vscp_class = VSCP_CLASS2_MEASUREMENT_STR;
                pEvent->vscp_type = vscptype;
                pEvent->timestamp = 0;
                pEvent->sizeData = 12;

                data[ 0 ] = sensoridx;
                data[ 1 ] = zone;
                data[ 2 ] = subzone;
                data[ 3 ] = unit;

                pEvent->pdata = new uint8_t[ 4 + strValue.Length() ];
                if ( NULL == pEvent->pdata ) {
                    webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_GENERAL_FAILURE );
                    delete pEvent;
                    return;
                }
                memcpy( data + 4, strValue.mbc_str(), strValue.Length() ); // copy in double

                webserv_rest_doSendEvent( nc, pSession, format, pEvent );
            }
        }

        webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_SUCCESS );
    }
    else {
        webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
    }

    return;
}


///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doGetTableData
//

void 
VSCPWebServerThread::websrc_rest_renderTableData( struct mg_connection *nc,
                                                        struct websrv_rest_session *pSession,
                                                        int format,
                                                        wxString& strName,
                                                        struct _vscpFileRecord *pRecords,
                                                        long nfetchedRecords )
{
    char buf[ 2048 ];
    char wrkbuf[ 2048 ];
    long nRecords = 0;

    if ( REST_FORMAT_PLAIN == format ) {

        // Send header
        webserv_util_sendheader( nc, 200, REST_MIME_TYPE_PLAIN );
        sprintf( wrkbuf,
                 "1 1 Success\r\n%ld records will be returned from table %s.\r\n",
                 nfetchedRecords,
                 ( const char * )strName.mbc_str() );
        mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ) );

        for ( long i = 0; i < nfetchedRecords; i++ ) {

            wxDateTime dt;
            dt.Set( ( time_t )pRecords[ i ].timestamp );
            wxString strDateTime = 
                            dt.FormatISODate() + _( " " ) + dt.FormatISOTime();

            sprintf( wrkbuf,
                     "%ld - Date=%s,Value=%f\r\n",
                     i,
                     ( const char * )strDateTime.mbc_str(),
                     pRecords->measurement );
            mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ));

        }

    }
    else if ( REST_FORMAT_CSV == format ) {

        // Send header
        webserv_util_sendheader( nc, 200, REST_MIME_TYPE_CSV );
        sprintf( wrkbuf,
                 "success-code, error-code, message, description, EvenL\r\n1, 1, Success, Success., NULL\r\n" );
        mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ) );

        sprintf( wrkbuf,
                 "1, 2, Info, Success %ld records will be returned from table %s.,NULL\r\n",
                 nfetchedRecords,
                 ( const char * )strName.mbc_str() );
        mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ) );

        sprintf( wrkbuf,
                 "1, 4, Count, %ld, NULL\r\n",
                 nfetchedRecords );
        mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ) );

        for ( long i = 0; i < nfetchedRecords; i++ ) {

            wxDateTime dt;
            dt.Set( ( time_t )pRecords[ i ].timestamp );
            wxString strDateTime = dt.FormatISODate() + _( " " ) + dt.FormatISOTime();

            sprintf( wrkbuf,
                     "1,3,Data,Table,%ld - Date=%s,Value=%f\r\n",
                     i,
                     ( const char * )strDateTime.mbc_str(),
                     pRecords->measurement );
            mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ) );

        }

    }
    else if ( REST_FORMAT_XML == format ) {

        // Send header
        webserv_util_sendheader( nc, 200, REST_MIME_TYPE_XML );
        sprintf( wrkbuf,
                 "<vscp-rest success=\"true\" code=\"1\" message=\"Success\" description=\"Success.\">\r\n" );
        mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ) );

        sprintf( wrkbuf,
                 "<count>%ld</count>\r\n<info>Fetched %ld elements of table %s</info>\r\n",
                 nfetchedRecords,
                 nfetchedRecords,
                 ( const char * )strName.mbc_str() );
        mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf )  );

        for ( long i = 0; i < nfetchedRecords; i++ ) {

            wxDateTime dt;
            dt.Set( ( time_t )pRecords[ i ].timestamp );
            wxString strDateTime = dt.FormatISODate() + _( " " ) + dt.FormatISOTime();

            sprintf( wrkbuf,
                     "<data id=\"%ld\" date=\"%s\" value=\"%f\"></data>\r\n",
                     i,
                     ( const char * )strDateTime.mbc_str(),
                     pRecords->measurement );
            mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ) );

        }

        sprintf( wrkbuf,
                 "</vscp-rest>\r\n" );
        mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ) );

    }
    else if ( ( REST_FORMAT_JSON == format ) || ( REST_FORMAT_JSONP == format ) ) {

        if ( REST_FORMAT_JSON == format ) {
            // Send header
            webserv_util_sendheader( nc, 200, REST_MIME_TYPE_JSON );
        }
        else {                                    // Send header
            webserv_util_sendheader( nc, 200, REST_MIME_TYPE_JSONP );
        }

        memset( buf, 0, sizeof( buf ) );
        char *p = wrkbuf;

        if ( REST_FORMAT_JSONP == format ) {
            p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, "typeof handler === 'function' && handler(", 41 );
        }

        p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p,
                                     "{\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\",",
                                     strlen( "{\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\"," ) );

        p += json_emit_quoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, "info", 4 );
        p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ":", 1 );
        {
            char buf2[ 200 ];
            sprintf( buf2,
                     "\"%ld rows will be retreived from table %s\"",
                     nfetchedRecords,
                     ( const char * )strName.mbc_str() );
            p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, buf2, strlen( buf2 ) );
        }
        p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ",", 1 );
        p += json_emit_quoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, "table", 5 );
        p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ":[", 2 );

        memset( buf, 0, sizeof( buf ) );
        mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ) );
        p = wrkbuf;

        for ( long i = 0; i < nfetchedRecords; i++ ) {

            wxDateTime dt;
            dt.Set( ( time_t )pRecords[ i ].timestamp );
            wxString strDateTime = dt.FormatISODate() + _("T") + dt.FormatISODate(); //dt.git pull();

            memset( wrkbuf, 0, sizeof( wrkbuf ) );
            p = wrkbuf;

            sprintf( buf,
                     "{\"id\":%ld,\"date\":\"%s\",\"value\":%f}",
                     i,
                     ( const char * )strDateTime.mbc_str(),
                     pRecords->measurement );

            p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, buf, strlen( buf ) );

            if ( i < ( nfetchedRecords - 1 ) ) {
                p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ",", 1 );
            }

            mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf )  );

        }

        memset( wrkbuf, 0, sizeof( wrkbuf ) );
        p = wrkbuf;

        // Mark end
        p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, "],", 2 );
        p += json_emit_quoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, "count", 5 );
        p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ":", 1 );
        p += json_emit_long( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, nfetchedRecords );
        p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ",", 1 );
        p += json_emit_quoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, "filtered", 8 );
        p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ":", 1 );
        p += json_emit_long( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, 0 );
        p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ",", 1 );
        p += json_emit_quoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, "errors", 6 );
        p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ":", 1 );
        p += json_emit_long( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, 0 );
        p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, "}", 1 );

        if ( REST_FORMAT_JSONP == format ) {
            p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ");", 2 );
        }

        mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ) );

    }

    mg_send_http_chunk( nc, "", 0 );    // Terminator
}


///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doGetTableData
//

void
VSCPWebServerThread::webserv_rest_doGetTableData( struct mg_connection *nc,
                                                        struct websrv_rest_session *pSession,
                                                        int format,
                                                        wxString& strName,
                                                        wxString& strFrom,
                                                        wxString& strTo )
{
    //char buf[ 2048 ];
    //char wrkbuf[ 2048 ];
    long nRecords = 0;

    // Check pointer
    if ( NULL == nc ) return;

    CControlObject *pObject = ( CControlObject * )nc->mgr->user_data;
    if ( NULL == pObject ) return;

    if ( NULL != pSession ) {

        // We need 'tablename' and optionally 'from' and 'to'

        CVSCPTable *ptblItem = NULL;
        pObject->m_mutexTableList.Lock();
        listVSCPTables::iterator iter;

        for ( iter = pObject->m_listTables.begin(); iter != pObject->m_listTables.end(); ++iter ) {
            ptblItem = *iter;
            if ( 0 == strcmp( ptblItem->m_vscpFileHead.nameTable, ( const char * )strName.mbc_str() ) ) {
                break;
            }
            ptblItem = NULL;
        }

        pObject->m_mutexTableList.Unlock();

        // If found pTable should not be NULL
        if ( NULL == ptblItem ) {
            webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_TABLE_NOT_FOUND );
            return;
        }

        // Get data from table
        int nRange = 0;     // 2 if to + from, 1 if from, 0 if non of them
        wxDateTime timeFrom = wxDateTime::Now();
        wxDateTime timeTo = wxDateTime::Now();

        // Get from-date for range
        strFrom.Trim();
        if ( strFrom.Length() ) {
            timeFrom.ParseDateTime( strFrom );
            //wxstr = timeFrom.FormatISODate() + _( " " ) + timeFrom.FormatISOTime();
            nRange++; // Start date entered
        }

        // Get to-date for range
        strTo.Trim();
        if ( strTo.Length() ) {
            timeTo.ParseDateTime( strTo );
            //wxstr = timeTo.FormatISODate() + _( " " ) + timeTo.FormatISOTime();
            nRange++; // End date entered
        }

        // Check range
        if ( ( nRange > 1 ) && timeTo.IsEarlierThan( timeFrom ) ) {
            webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_TABLE_RANGE );
            return;
        }

        ///////////////////////////////////////////////////////////////////////
        //                          Dynamic table
        ///////////////////////////////////////////////////////////////////////
        if ( VSCP_TABLE_DYNAMIC == ptblItem->m_vscpFileHead.type ) {

            uint64_t start, end;
            if ( 0 == nRange ) {
                // Neither 'from' or 'to' given 
                // Use value from header
                start = ptblItem->getTimeStampStart();
                end = ptblItem->getTimeStampEnd();
            }
            else if ( 1 == nRange ) {
                // From given but no end
                start = timeFrom.GetTicks();
                end = ptblItem->getTimeStampEnd();
            }
            else {
                // range given
                start = timeFrom.GetTicks();
                end = timeTo.GetTicks();
            }

            // Fetch number of records in set 
            ptblItem->m_mutexThisTable.Lock();
            long nRecords = ptblItem->getRangeOfData( start, end );
            ptblItem->m_mutexThisTable.Unlock();

            if ( 0 == nRecords ) {
                webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_TABLE_NO_DATA );
                return;
            }

            struct _vscpFileRecord *pRecords = new struct _vscpFileRecord[ nRecords ];

            if ( NULL == pRecords ) {
                webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_GENERAL_FAILURE );
                return;
            }

            ptblItem->m_mutexThisTable.Lock();
            long nfetchedRecords = ptblItem->getRangeOfData( start,
                                                                end,
                                                                ( void * )pRecords,
                                                                nRecords* sizeof( struct _vscpFileRecord ) );
            ptblItem->m_mutexThisTable.Unlock();

            if ( 0 == nfetchedRecords ) {
                delete[] pRecords;
                webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_TABLE_NO_DATA );
                return;
            }

            wxDateTime dtStart;
            dtStart.Set( ( time_t )ptblItem->getTimeStampStart() );
            wxString strDateTimeStart = dtStart.FormatISODate() + _( "T" ) + dtStart.FormatISOTime();

            wxDateTime dtEnd;
            dtEnd.Set( ( time_t )ptblItem->getTimeStampEnd() );
            wxString strDateTimeEnd = dtEnd.FormatISODate() + _( "T" ) + dtEnd.FormatISOTime();

            // Output table data
            websrc_rest_renderTableData( nc,
                                            pSession,
                                            format,
                                            strName,
                                            pRecords,
                                            nfetchedRecords );

            /*if ( REST_FORMAT_PLAIN == format ) {

                // Send header
                webserv_util_sendheader( nc, 200, REST_MIME_TYPE_PLAIN );

                sprintf( wrkbuf,
                         "1 1 Success\r\n%ld records will be returned from table %s.\r\n",
                         nfetchedRecords,
                         (const char *)strName.mbc_str() );
                mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ) );

                for ( long i = 0; i < nfetchedRecords; i++ ) {

                    wxDateTime dt;
                    dt.Set( ( time_t )pRecords[ i ].timestamp );
                    wxString strDateTime = dt.FormatISODate() + _( " " ) + dt.FormatISOTime();

                    sprintf( wrkbuf,
                             "%ld - Date=%s,Value=%f\r\n",
                             i,
                             (const char *)strDateTime.mbc_str(),
                             pRecords->measurement );
                    mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ) );

                }

            }
            else if ( REST_FORMAT_CSV == format ) {

                // Send header
                webserv_util_sendheader( nc, 200, REST_MIME_TYPE_CSV );

                sprintf( wrkbuf,
                         "success-code, error-code, message, description, EvenL\r\n1, 1, Success, Success., NULL\r\n" );
                mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ) );

                sprintf( wrkbuf,
                         "1, 2, Info, Success %d records will be returned from table %s.,NULL\r\n",
                         nfetchedRecords,
                         ( const char * )strName.mbc_str() );
                mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf )  );

                sprintf( wrkbuf,
                         "1, 4, Count, %d, NULL\r\n",
                         nfetchedRecords );
                mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ) );

                for ( long i = 0; i < nfetchedRecords; i++ ) {

                    wxDateTime dt;
                    dt.Set( ( time_t )pRecords[ i ].timestamp );
                    wxString strDateTime = dt.FormatISODate() + _( " " ) + dt.FormatISOTime();

                    sprintf( wrkbuf,
                             "1,3,Data,Table,%d - Date=%s,Value=%f\r\n",
                             i,
                             ( const char * )strDateTime.mbc_str(),
                             pRecords->measurement );
                    mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ) );

                }
                
            }
            else if ( REST_FORMAT_XML == format ) {

                // Send header
                webserv_util_sendheader( nc, 200, REST_MIME_TYPE_XML );

                sprintf( wrkbuf,
                         "<vscp-rest success=\"true\" code=\"1\" message=\"Success\" description=\"Success.\">\r\n" );
                mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ) );

                sprintf( wrkbuf,
                            "<count>%d</count>\r\n<info>Fetched %d elements of table %s</info>\r\n",
                            nfetchedRecords,
                            nfetchedRecords,
                            ( const char * )strName.mbc_str() );
                mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ) );

                for ( long i = 0; i < nfetchedRecords; i++ ) {

                    wxDateTime dt;
                    dt.Set( ( time_t )pRecords[ i ].timestamp );
                    wxString strDateTime = dt.FormatISODate() + _( " " ) + dt.FormatISOTime();

                    sprintf( wrkbuf,
                             "<data id=\"%d\" date=\"%s\" value=\"%f\"></data>\r\n",
                             i,
                             ( const char * )strDateTime.mbc_str(),
                             pRecords->measurement );
                    mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ) );

                }

                sprintf( wrkbuf,
                         "</vscp-rest>\r\n" );
                mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ));

            }
            else if ( ( REST_FORMAT_JSON == format ) || ( REST_FORMAT_JSONP == format ) ) {

                if ( REST_FORMAT_JSON == format ) {
                    // Send header
                    webserv_util_sendheader( nc, 200, REST_MIME_TYPE_JSON );
                }
                else {                                    // Send header
                    webserv_util_sendheader( nc, 200, REST_MIME_TYPE_JSONP );
                }

                char *p = wrkbuf;

                if ( REST_FORMAT_JSONP == format ) {
                    p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, "typeof handler === 'function' && handler(", 41 );
                }

                p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p,
                                             "{\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\",",
                                             strlen( "{\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\"," ) );

                p += json_emit_quoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, "info", 4 );
                p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ":", 1 );
                {
                    char buf2[ 200 ];
                    sprintf( buf2,
                             "\"%d rows will be retreived from table %s\"",
                             nfetchedRecords,
                             ( const char * )strName.mbc_str() );
                    p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, buf2, strlen( buf2 ) );
                }
                p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ",", 1 );
                p += json_emit_quoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, "table", 5 );
                p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ":[", 2 );

                mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ) );
                p = wrkbuf;
               
                for ( long i = 0; i < nfetchedRecords; i++ ) {

                    wxDateTime dt;
                    dt.Set( ( time_t )pRecords[ i ].timestamp );
                    wxString strDateTime = dt.FormatISOCombined();

                    memset( wrkbuf, 0, sizeof( wrkbuf ) );
                    p = wrkbuf;

                    sprintf( buf,
                             "{\"id\":%d,\"date\":\"%s\",\"value\":%f}",
                             i,
                             ( const char * )strDateTime.mbc_str(),
                             pRecords->measurement );

                    p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, buf, strlen( buf ) );

                    if ( i < ( nfetchedRecords - 1 ) ) {
                        p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ",", 1 );
                    }                

                    mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf ) );

                }
                
                memset( wrkbuf, 0, sizeof( wrkbuf ) );
                p = wrkbuf;

                // Mark end
                p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, "],", 2 );
                p += json_emit_quoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, "count", 5 );
                p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ":", 1 );
                p += json_emit_long( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, nfetchedRecords );
                p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ",", 1 );
                p += json_emit_quoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, "filtered", 8 );
                p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ":", 1 );
                p += json_emit_long( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, 0 );
                p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ",", 1 );
                p += json_emit_quoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, "errors", 6 );
                p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ":", 1 );
                p += json_emit_long( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, 0 );
                p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, "}", 1 );

                if ( REST_FORMAT_JSONP == format ) {
                    p += json_emit_unquoted_str( p, &wrkbuf[ sizeof( wrkbuf ) ] - p, ");", 2 );
                }

                mg_send_http_chunk( nc, wrkbuf, strlen( wrkbuf )  );

            }*/

            mg_send_http_chunk( nc, "", 0 );    // Terminator

            // De allocate storage
            delete[] pRecords;

        }
        ///////////////////////////////////////////////////////////////////////
        //                          Static table
        ///////////////////////////////////////////////////////////////////////
        else if ( VSCP_TABLE_STATIC == ptblItem->m_vscpFileHead.type ) {

            // Fetch number of records in set 
            ptblItem->m_mutexThisTable.Lock();
            long nRecords = ptblItem->getStaticRequiredBuffSize();
            ptblItem->m_mutexThisTable.Unlock();

            if ( nRecords > 0 ) {

                struct _vscpFileRecord *pRecords = new struct _vscpFileRecord[ nRecords ];

                if ( NULL == pRecords ) {
                    webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_GENERAL_FAILURE );
                    return;
                }

                // Fetch data
                long nfetchedRecords = ptblItem->getStaticData( pRecords, sizeof( pRecords ) );

                // Output table data
                websrc_rest_renderTableData( nc,
                                             pSession,
                                             format,
                                             strName,
                                             pRecords,
                                             nfetchedRecords );

                    /*// First send start post with number if records
                    mg_printf_websocket_frame( nc,
                                         WEBSOCKET_OP_TEXT,
                                         "+;GT;START;%d",
                                         nfetchedRecords );

                    // Then send measurement records
                    for ( long i = 0; i<nfetchedRecords; i++ ) {
                        mg_printf_websocket_frame( nc,
                                             WEBSOCKET_OP_TEXT,
                                             "+;GT;%d;%d;%f",
                                             i, pRecords[ i ].timestamp, pRecords[ i ].measurement );
                    }

                    // Last send end post with number if records
                    mg_printf_websocket_frame( nc,
                                         WEBSOCKET_OP_TEXT,
                                         "+;GT;END;%d",
                                         nfetchedRecords );*/

                    // Deallocate storage
                    delete[] pRecords;

                    mg_send_http_chunk( nc, "", 0 );    // Terminator

                }

            }
            // No records
            else {
                webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_TABLE_NO_DATA );
                return;
            }

            

        }


    return;
}


///////////////////////////////////////////////////////////////////////////////
// XML to JSON convert
//

void convert( const char *input, const char *output  ) 
{
    ifstream is( input );
    ostringstream oss;
    oss << is.rdbuf();

    string json_str = xml2json( oss.str().data() );

    ofstream myfile;
    myfile.open( output );
    myfile << json_str << endl;
    myfile.close();
}

///////////////////////////////////////////////////////////////////////////////
// websrv_mainpage
//

void VSCPWebServerThread::webserv_rest_doFetchMDF( struct mg_connection *nc,
                                struct websrv_rest_session *pSession,
                                int format,
                                wxString& strURL )
{
    CMDF mdf;
    
    if ( mdf.load( strURL, false, true ) )  {
        
        // Loaded OK

        if ( REST_FORMAT_PLAIN == format ) {
            webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_GENERAL_FAILURE );
        }
        else if ( REST_FORMAT_CSV == format ) {
            webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_GENERAL_FAILURE );
        }
        else if ( REST_FORMAT_XML == format ) {

            // Send header
            webserv_util_sendheader( nc, 200, REST_MIME_TYPE_XML );

            char buf[ 5000 ], wrkbuf[ 2000 ];
            ssize_t ss;
            wxFile file( mdf.getTempFilePath() );

            while ( !file.Eof() ) {
                ss = file.Read( wrkbuf, sizeof( wrkbuf ) );
                mg_send_http_chunk( nc, wrkbuf, ss );
            }

            file.Close();

            mg_send_http_chunk( nc, "", 0 );    // Terminator
        }
        else if ( ( REST_FORMAT_JSON == format ) || ( REST_FORMAT_JSONP == format ) ) {

            char buf[ 5000 ], wrkbuf[ 2000 ];
            wxString tempFileName = wxFileName::CreateTempFileName( _("__vscp__xml__") );
            if ( 0 == tempFileName.Length() ) {
                webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_GENERAL_FAILURE );
            }

            std::string tempfile_out = std::string( tempFileName.mb_str() );
            std::string tempfile_mdf = std::string( mdf.getTempFilePath().mb_str() );

            // Convert to JSON
            convert( (const char *)tempFileName.mbc_str(), (const char *)mdf.getTempFilePath().mbc_str() );

            // Send header
            if ( REST_FORMAT_JSON == format ) {
                webserv_util_sendheader( nc, 200, REST_MIME_TYPE_JSON );
            }
            else {
                webserv_util_sendheader( nc, 200, REST_MIME_TYPE_JSONP );
            }

            if ( REST_FORMAT_JSONP == format ) {
                mg_send_http_chunk( nc, "typeof handler === 'function' && handler(", 41 );
            }

            ssize_t ss;
            wxString wxpath( tempfile_out.c_str(), wxConvUTF8 ); // Needed for 2.8.12
            wxFile file( wxpath );

            while ( !file.Eof() ) {
                ss = file.Read( wrkbuf, sizeof( wrkbuf ) );
                mg_send_http_chunk( nc, wrkbuf, ss );
            }

            file.Close();

            if ( REST_FORMAT_JSONP == format ) {
                mg_send_http_chunk( nc, ");", 2 );
            }

            mg_send_http_chunk( nc, "", 0 );    // Terminator

        }

    }
    else {
        // Failed to load
        webserv_rest_error( nc, pSession, format, REST_ERROR_CODE_GENERAL_FAILURE );
    }

    return;
}
