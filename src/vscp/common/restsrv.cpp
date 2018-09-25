// restsrv.cpp
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

#include <json.hpp>             // Needs C++11  -std=c++11
#include <vscp.h>
#include <vscphelper.h>
#include <vscpeventhelper.h>
#include <tables.h>
#include <vscp_aes.h>
#include <version.h>
#include <controlobject.h>
#include <variablecodes.h> 
#include <actioncodes.h>
#include <devicelist.h>
#include <devicethread.h> 
#include <dm.h>
#include <mdf.h>
#include <httpd.h>
#include <websrv.h>
#include <restsrv.h>

using namespace std;

// https://github.com/nlohmann/json
using json = nlohmann::json;

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

///////////////////////////////////////////////////
//                 GLOBALS
///////////////////////////////////////////////////

extern CControlObject *gpobj;

// Prototypes

void
restsrv_doStatus( struct web_connection *conn,
                            struct restsrv_session *pSession,
                            int format );

void
restsrv_doOpen( struct web_connection *conn,
                        struct restsrv_session *pSession,
                        int format );

void
restsrv_doClose( struct web_connection *conn,
                        struct restsrv_session *pSession,
                        int format );

void
restsrv_doSendEvent( struct web_connection *conn,
                            struct restsrv_session *pSession,
                            int format,
                            vscpEvent *pEvent );

void
restsrv_doReceiveEvent( struct web_connection *conn,
                                struct restsrv_session *pSession,
                                int format,
                                size_t count );

void
restsrv_doReceiveEvent( struct web_connection *conn,
                                struct restsrv_session *pSession,
                                int format,
                                size_t count );

void
restsrv_doSetFilter( struct web_connection *conn,
                            struct restsrv_session *pSession,
                            int format,
                            vscpEventFilter& vscpfilter );

void
restsrv_doClearQueue( struct web_connection *conn,
                                struct restsrv_session *pSession,
                                int format );

void
restsrv_doListVariable( struct web_connection *conn,
                                struct restsrv_session *pSession,
                                int format,
                                wxString& strRegEx,
                                bool bShort );

void
restsrv_doListVariable( struct web_connection *conn,
                                struct restsrv_session *pSession,
                                int format,
                                wxString& strRegEx,
                                bool bShort );

void
restsrv_doReadVariable( struct web_connection *conn,
                                struct restsrv_session *pSession,
                                int format,
                                wxString& strVariableName );

void
restsrv_doWriteVariable( struct web_connection *conn,
                                struct restsrv_session *pSession,
                                int format,
                                wxString& strVariableName,
                                wxString& strValue );

void
restsrv_doCreateVariable( struct web_connection *conn,
                                    struct restsrv_session *pSession,
                                    int format,
                                    wxString& strVariable,
                                    wxString& strType,
                                    wxString& strValue,
                                    wxString& strPersistent,
                                    wxString& strAccessRight,
                                    wxString& strNote );

void
restsrv_doDeleteVariable( struct web_connection *conn,
                                    struct restsrv_session *pSession,
                                    int format,
                                    wxString& strVariable );

void
restsrv_doWriteMeasurement( struct web_connection *conn,
                                    struct restsrv_session *pSession,
                                    int format,
                                    wxString& strDateTime,
                                    wxString& strGuid,
                                    wxString& strLevel,
                                    wxString& strType,
                                    wxString& strValue,
                                    wxString& strUnit,
                                    wxString& strSensorIdx,
                                    wxString& strZone,
                                    wxString& strSubZone,
                                    wxString& strEventFormat );

void
websrc_renderTableData( struct web_connection *conn,
                                struct restsrv_session *pSession,
                                int format,
                                wxString& strName,
                                struct _vscpFileRecord *pRecords,
                                long nfetchedRecords );

void 
restsrv_doFetchMDF( struct web_connection *conn,
                                struct restsrv_session *pSession,
                                int format,
                                wxString& strURL );

void
websrc_renderTableData( struct web_connection *conn,
                                struct restsrv_session *pSession,
                                int format,
                                wxString& strName,
                                struct _vscpFileRecord *pRecords,
                                long nfetchedRecords );

void
restsrv_doGetTableData( struct web_connection *conn,
                                struct restsrv_session *pSession,
                                int format,
                                wxString& strName,
                                wxString& strFrom,
                                wxString& strTo );



const char* rest_errors[][ REST_FORMAT_COUNT + 1 ] = {
    { REST_PLAIN_ERROR_SUCCESS,                 REST_CSV_ERROR_SUCCESS,                 REST_XML_ERROR_SUCCESS,                 REST_JSON_ERROR_SUCCESS,                REST_JSONP_ERROR_SUCCESS                    },
    { REST_PLAIN_ERROR_GENERAL_FAILURE,         REST_CSV_ERROR_GENERAL_FAILURE,         REST_XML_ERROR_GENERAL_FAILURE,         REST_JSON_ERROR_GENERAL_FAILURE,        REST_JSONP_ERROR_GENERAL_FAILURE            },
    { REST_PLAIN_ERROR_INVALID_SESSION,         REST_CSV_ERROR_INVALID_SESSION,         REST_XML_ERROR_INVALID_SESSION,         REST_JSON_ERROR_INVALID_SESSION,        REST_JSONP_ERROR_INVALID_SESSION            },
    { REST_PLAIN_ERROR_UNSUPPORTED_FORMAT,      REST_CSV_ERROR_UNSUPPORTED_FORMAT,      REST_XML_ERROR_UNSUPPORTED_FORMAT,      REST_JSON_ERROR_UNSUPPORTED_FORMAT,     REST_JSONP_ERROR_UNSUPPORTED_FORMAT         },
    { REST_PLAIN_ERROR_COULD_NOT_OPEN_SESSION,  REST_CSV_ERROR_COULD_NOT_OPEN_SESSION,  REST_XML_ERROR_COULD_NOT_OPEN_SESSION,  REST_JSON_ERROR_COULD_NOT_OPEN_SESSION, REST_JSONP_ERROR_COULD_NOT_OPEN_SESSION     },
    { REST_PLAIN_ERROR_MISSING_DATA,            REST_CSV_ERROR_MISSING_DATA,            REST_XML_ERROR_MISSING_DATA,            REST_JSON_ERROR_MISSING_DATA,           REST_JSONP_ERROR_MISSING_DATA               },
    { REST_PLAIN_ERROR_INPUT_QUEUE_EMPTY,       REST_CSV_ERROR_INPUT_QUEUE_EMPTY,       REST_XML_ERROR_INPUT_QUEUE_EMPTY,       REST_JSON_ERROR_INPUT_QUEUE_EMPTY,      REST_JSONP_ERROR_INPUT_QUEUE_EMPTY          },
    { REST_PLAIN_ERROR_VARIABLE_NOT_FOUND,      REST_CSV_ERROR_VARIABLE_NOT_FOUND,      REST_XML_ERROR_VARIABLE_NOT_FOUND,      REST_JSON_ERROR_VARIABLE_NOT_FOUND,     REST_JSONP_ERROR_VARIABLE_NOT_FOUND         },
    { REST_PLAIN_ERROR_VARIABLE_NOT_CREATED,    REST_CSV_ERROR_VARIABLE_NOT_CREATED,    REST_XML_ERROR_VARIABLE_NOT_CREATED,    REST_JSON_ERROR_VARIABLE_NOT_CREATED,   REST_JSONP_ERROR_VARIABLE_NOT_CREATED       },
    { REST_PLAIN_ERROR_VARIABLE_FAIL_UPDATE,    REST_CSV_ERROR_VARIABLE_FAIL_UPDATE,    REST_XML_ERROR_VARIABLE_FAIL_UPDATE,    REST_JSON_ERROR_VARIABLE_FAIL_UPDATE,   REST_JSONP_ERROR_VARIABLE_FAIL_UPDATE       },   
    { REST_PLAIN_ERROR_NO_ROOM,                 REST_CSV_ERROR_NO_ROOM,                 REST_XML_ERROR_NO_ROOM,                 REST_JSON_ERROR_NO_ROOM,                REST_JSONP_ERROR_NO_ROOM                    },
    { REST_PLAIN_ERROR_TABLE_NOT_FOUND,         REST_CSV_ERROR_TABLE_NOT_FOUND,         REST_XML_ERROR_TABLE_NOT_FOUND,         REST_JSON_ERROR_TABLE_NOT_FOUND,        REST_JSONP_ERROR_TABLE_NOT_FOUND,           },
    { REST_PLAIN_ERROR_TABLE_NO_DATA,           REST_CSV_ERROR_TABLE_NO_DATA,           REST_XML_ERROR_TABLE_NO_DATA,           REST_JSON_ERROR_TABLE_NO_DATA,          REST_JSONP_ERROR_TABLE_NO_DATA              },
    { REST_PLAIN_ERROR_TABLE_RANGE,             REST_CSV_ERROR_TABLE_RANGE,             REST_XML_ERROR_TABLE_RANGE,             REST_JSON_ERROR_TABLE_RANGE,            REST_JSONP_ERROR_TABLE_RANGE                },
    { REST_PLAIN_ERROR_INVALID_USER,            REST_CSV_ERROR_INVALID_USER,            REST_XML_ERROR_INVALID_USER,            REST_JSON_ERROR_INVALID_USER,           REST_JSONP_ERROR_INVALID_USER               },
    { REST_PLAIN_ERROR_INVALID_ORIGIN,          REST_CSV_ERROR_INVALID_ORIGIN,          REST_XML_ERROR_INVALID_ORIGIN,          REST_JSON_ERROR_INVALID_ORIGIN,         REST_JSONP_ERROR_INVALID_ORIGIN             },
    { REST_PLAIN_ERROR_INVALID_PASSWORD,        REST_CSV_ERROR_INVALID_PASSWORD,        REST_XML_ERROR_INVALID_PASSWORD,        REST_JSON_ERROR_INVALID_PASSWORD,       REST_JSONP_ERROR_INVALID_PASSWORD           },
    { REST_PLAIN_ERROR_MEMORY,                  REST_CSV_ERROR_MEMORY,                  REST_XML_ERROR_MEMORY,                  REST_JSON_ERROR_MEMORY,                 REST_JSONP_ERROR_MEMORY                     },
    { REST_PLAIN_ERROR_VARIABLE_NOT_DELETE,     REST_CSV_ERROR_VARIABLE_NOT_DELETE,     REST_XML_ERROR_VARIABLE_NOT_DELETE,     REST_JSON_ERROR_VARIABLE_NOT_DELETE,    REST_JSONP_ERROR_VARIABLE_NOT_DELETE        },

};


//-----------------------------------------------------------------------------
//                                   REST
//-----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// restsrv_error
//

void
restsrv_error( struct web_connection *conn,
                    struct restsrv_session *pSession,
                    int format,
                    int errorcode)
{
    char buf[2048];
    int returncode = 200;

    if ( REST_FORMAT_PLAIN == format ) {

        websrv_sendheader( conn, returncode, REST_MIME_TYPE_PLAIN );
        web_write( conn, 
                    rest_errors[errorcode][REST_FORMAT_PLAIN], 
                    strlen( rest_errors[errorcode][REST_FORMAT_PLAIN] )  );
        web_write( conn, "", 0 );   // Terminator
        return;
 
    }
    else if ( REST_FORMAT_CSV == format ) {

        websrv_sendheader( conn, returncode, REST_MIME_TYPE_CSV );
        web_write( conn, 
                    rest_errors[errorcode][REST_FORMAT_CSV], 
                    strlen( rest_errors[errorcode][REST_FORMAT_CSV] )  );
        web_write( conn, "", 0 );   // Terminator
        return;

    }
    else if ( REST_FORMAT_XML == format ) {

        websrv_sendheader( conn, returncode, REST_MIME_TYPE_XML );
        web_write( conn, XML_HEADER, strlen( XML_HEADER ) );
        web_write( conn, 
                    rest_errors[errorcode][REST_FORMAT_XML], 
                    strlen( rest_errors[errorcode][REST_FORMAT_XML] ) );
        web_write( conn, "", 0 );   // Terminator
        return;

    }
    else if ( REST_FORMAT_JSON == format ) {

        websrv_sendheader( conn, returncode, REST_MIME_TYPE_JSON );
        web_write( conn, 
                        rest_errors[errorcode][REST_FORMAT_JSON], 
                        strlen( rest_errors[errorcode][REST_FORMAT_JSON] ) );
        web_write( conn, "", 0 );   // Terminator
        return;

    }
    else if ( REST_FORMAT_JSONP == format ) {

        websrv_sendheader( conn, returncode, REST_MIME_TYPE_JSONP );
        web_write( conn, 
                        rest_errors[errorcode][REST_FORMAT_JSONP], 
                        strlen( rest_errors[errorcode][REST_FORMAT_JSONP] ) );
        web_write( conn, "", 0 );   // Terminator
        return;

    }
    else {

        websrv_sendheader( conn, returncode, REST_MIME_TYPE_PLAIN );
        web_write( conn, 
                        REST_PLAIN_ERROR_UNSUPPORTED_FORMAT, 
                        strlen( REST_PLAIN_ERROR_UNSUPPORTED_FORMAT ) );
        web_write( conn, "", 0 );   // Terminator
        return;

    }
}

///////////////////////////////////////////////////////////////////////////////
// restsrv_sendHeader
//

void
restsrv_sendHeader( struct web_connection *conn,
                            int format,
                            int returncode )
{
    if ( REST_FORMAT_PLAIN == format ) {
        websrv_sendheader( conn, returncode, REST_MIME_TYPE_PLAIN );
    }
    else if ( REST_FORMAT_CSV == format ) {
        websrv_sendheader( conn, returncode, REST_MIME_TYPE_CSV );
    }
    else if ( REST_FORMAT_XML == format ) {
        websrv_sendheader( conn, returncode, REST_MIME_TYPE_XML );
    }
    else if ( REST_FORMAT_JSON == format ) {
        websrv_sendheader( conn, returncode, REST_MIME_TYPE_JSON );
    }
    else if ( REST_FORMAT_JSONP == format ) {
        websrv_sendheader( conn, returncode, REST_MIME_TYPE_JSONP );
    }

}

///////////////////////////////////////////////////////////////////////////////
// restsrv_get_session
//

struct restsrv_session *
restsrv_get_session( struct web_connection *conn,
                        wxString& sid )
{
    const struct restsrv_session *pSession = NULL;
    const struct web_request_info *reqinfo;
    
    // Check pointers
    if ( !conn || 
         !( reqinfo = web_get_request_info( conn ) ) ) {
        return NULL;
    }
    
    if ( 0 == sid.Length() ) return NULL;

    // find existing session
    gpobj->m_restSessionMutex.Lock();
    RESTSESSIONLIST::iterator iter;
    for ( iter = gpobj->m_rest_sessions.begin(); 
            iter != gpobj->m_rest_sessions.end(); 
            ++iter ) {
        struct restsrv_session *pSession = *iter;
        if ( 0 == strcmp( (const char *)sid.mbc_str(), 
                            pSession->m_sid ) ) {
            pSession->m_lastActiveTime = time( NULL );
            gpobj->m_restSessionMutex.Unlock();
            return pSession;
        }
    }
    gpobj->m_restSessionMutex.Unlock();
             
    return NULL;
}


///////////////////////////////////////////////////////////////////////////////
// restsrv_add_session
//

restsrv_session *
restsrv_add_session( struct web_connection *conn, CUserItem *pUserItem )
{
    char buf[512];
    wxString user;
    struct restsrv_session *pSession;
    const struct web_request_info *reqinfo;
    
    // Check pointers
    if ( !conn || 
         !( reqinfo = web_get_request_info( conn ) )  ) {
        return 0;
    }

    // Parse "Authorization:" header, fail fast on parse error
    /*const char *pheader = web_get_header( conn, "Authorization" );
    if ( NULL == pheader ) return NULL;

    wxArrayString valarray;
    wxString header = wxString::FromUTF8( pheader );
    websrv_parseHeader( valarray, header );
    
    // Get username
    if ( !websrv_getHeaderElement( valarray, 
                                "username",
                                user ) ) {
        return NULL;
    }*/
        
    // Create fresh session
    pSession = new struct restsrv_session;
    if  ( NULL == pSession ) {
        return NULL;
    }    
    memset( pSession, 0, sizeof( websrv_session ) );

    // Generate a random session ID
    unsigned char iv[16];
    char hexiv[33];
    getRandomIV( iv, 16 );  // Generate 16 random bytes
    memset( hexiv, 0, sizeof(hexiv) );
    vscp_byteArray2HexStr( hexiv, iv, 16 );
    
    memset( pSession->m_sid, 0, sizeof( pSession->m_sid ) );
    memcpy( pSession->m_sid, hexiv, 32 );

    pSession->m_lastActiveTime = time( NULL );
    
    pSession->m_pClientItem = new CClientItem();        // Create client
    if ( NULL == pSession->m_pClientItem ) {
        gpobj->logMsg(_("[restsrv] New session: Unable to create client object."));
        delete pSession;
        return NULL;
    }

    // Set client data
    pSession->m_pClientItem->bAuthenticated = true;                 // Authenticated 
    pSession->m_pClientItem->m_pUserItem = pUserItem;
    vscp_clearVSCPFilter(&pSession->m_pClientItem->m_filterVSCP);   // Clear filter
    pSession->m_pClientItem->m_bOpen = false;                       // Start out closed
    pSession->m_pClientItem->m_type = CLIENT_ITEM_INTERFACE_TYPE_CLIENT_WEBSOCKET;
    pSession->m_pClientItem->m_strDeviceName = _("Internal REST server client.");
    pSession->m_pClientItem->m_strDeviceName += _("|Started at ");
    wxDateTime now = wxDateTime::Now();
    pSession->m_pClientItem->m_strDeviceName += now.FormatISODate();
    pSession->m_pClientItem->m_strDeviceName += _(" ");
    pSession->m_pClientItem->m_strDeviceName += now.FormatISOTime();

    // Add the client to the Client List
    gpobj->m_wxClientMutex.Lock();
    if ( !gpobj->addClient( pSession->m_pClientItem ) ) {
        // Failed to add client
        delete pSession->m_pClientItem;
        pSession->m_pClientItem = NULL;
        gpobj->m_wxClientMutex.Unlock();
        gpobj->logMsg( _("REST server: Failed to add client. Terminating thread.") );
        return NULL;
    }
    gpobj->m_wxClientMutex.Unlock();  
    
    // Add to linked list
    gpobj->m_restSessionMutex.Lock();
    gpobj->m_rest_sessions.Append( pSession );
    gpobj->m_restSessionMutex.Unlock();

    return pSession;
}




///////////////////////////////////////////////////////////////////////////////
// websrv_expire_sessions
//

void
restsrv_expire_sessions( struct web_connection *conn  )
{
    time_t now;

    now = time( NULL );
    
    gpobj->m_restSessionMutex.Lock();
    RESTSESSIONLIST::iterator iter;
    for ( iter = gpobj->m_rest_sessions.begin(); 
            iter != gpobj->m_rest_sessions.end(); 
            ++iter ) {
        struct restsrv_session *pSession = *iter;
        if ( ( now - pSession->m_lastActiveTime ) > ( 60 * 60 ) ) {
            gpobj->m_rest_sessions.DeleteContents( true );
            gpobj->m_rest_sessions.DeleteObject( pSession );
        }
    }
    gpobj->m_restSessionMutex.Unlock();
    
}


// Hash with key value pairs
WX_DECLARE_STRING_HASH_MAP( wxString, hashArgs );


///////////////////////////////////////////////////////////////////////////////
// websrv_restapi
//

int
websrv_restapi( struct web_connection *conn, void *cbdata )
{
    char bufBody[32000];        // Buffer for body (POST) data
    int len_post_data;
    const char *pParams = NULL; // Pointer to query data or POST data
    int lenParam = 0;
    char buf[2048];
    char date[64];
    wxString str;
    time_t curtime = time(NULL);
    long format = REST_FORMAT_PLAIN;
    hashArgs keypairs;
    struct web_context * ctx;
    const struct web_request_info *reqinfo;
    struct restsrv_session *pSession = NULL;    
    CUserItem *pUserItem = NULL;

    memset( bufBody, 0, sizeof( bufBody ) );
    
    // Check pointer
    if ( !conn || 
         !( ctx = web_get_context( conn ) ) ||
         !( reqinfo = web_get_request_info( conn ) )  ) {
        return WEB_ERROR;
    }

    // Get method
    char method[33];
    memset( method, 0, sizeof( method ) );
    strncpy( method, 
                reqinfo->request_method, 
                strlen( reqinfo->request_method ) );

    // Make string with GMT time
    vscp_getTimeString( date, sizeof(date), &curtime );
    
    // Set defaults
    keypairs[_("FORMAT")] = _("plain");
    keypairs[_("OP")] = _("open");

    if ( NULL != strstr( method, ("POST") ) ) {

        const char *pHeader;
        
        // read POST data
        len_post_data = web_read( conn, bufBody, sizeof( bufBody ) );

        // user
        if ( NULL != ( pHeader = web_get_header( conn, "vscpuser" ) ) ) {
            memset( buf, 0, sizeof( buf ) );
            strncpy( buf, pHeader, MIN( sizeof( buf )-1, strlen( pHeader ) ) );
            keypairs[_("VSCPUSER")] = wxString::FromUTF8( buf );
        }

        // password
        if ( NULL != ( pHeader = web_get_header( conn, "vscpsecret" ) ) ) {
            memset( buf, 0, sizeof( buf ) );
            strncpy( buf, pHeader, MIN( sizeof( buf )-1, strlen( pHeader ) ) );
            keypairs[_("VSCPSECRET")] = wxString::FromUTF8( buf );
        }

        // session
        if ( NULL != ( pHeader = web_get_header( conn, "vscpsession" ) ) ) {
            memset( buf, 0, sizeof( buf ) );
            strncpy( buf, pHeader, MIN( sizeof( buf )-1, strlen( pHeader ) ) );
            keypairs[_("VSCPSESSION")] = wxString::FromUTF8( buf );
        }
        
        pParams = bufBody;    // Parameters is in the body  
        lenParam = len_post_data;

    }
    else {
        
        // get parameters for get

        if ( 0 < web_get_var( reqinfo->query_string, 
                                strlen( reqinfo->query_string ),
                                "vscpuser", 
                                buf, 
                                sizeof( buf ) ) ) {
            keypairs[ _("VSCPUSER") ] = wxString::FromUTF8( buf );
        }

        if ( 0 < web_get_var( reqinfo->query_string,  
                                strlen( reqinfo->query_string ),
                                "vscpsecret", 
                                buf, 
                                sizeof( buf ) ) ) {
            keypairs[ _("VSCPSECRET") ] = wxString::FromUTF8( buf );
        }

        if ( 0 < web_get_var( reqinfo->query_string,  
                                strlen( reqinfo->query_string ), 
                                "vscpsession", 
                                buf, 
                                sizeof( buf ) ) ) {
            keypairs[ _("VSCPSESSION") ] = wxString::FromUTF8( buf );
        }
        
        pParams = reqinfo->query_string;    // Parameters is in query string
        lenParam = strlen( reqinfo->query_string );
    }

    // format
    if ( 0 < web_get_var( pParams, lenParam, "format", buf, sizeof( buf ) ) ) {
        keypairs[_("FORMAT")] = wxString::FromUTF8( buf );
    }

    // op
    if ( 0 < web_get_var( pParams, lenParam, "op", buf, sizeof(buf) ) ) {
        keypairs[_("OP")] = wxString::FromUTF8( buf );
    }

    // vscpevent
    if ( 0 < web_get_var( pParams, lenParam, "vscpevent", buf, sizeof(buf) ) ) {
        keypairs[_("VSCPEVENT")] = wxString::FromUTF8( buf );
    }

    // count
    if ( 0 < web_get_var( pParams, lenParam, "count", buf, sizeof(buf) ) ) {
        keypairs[_("COUNT")] = wxString::FromUTF8( buf );
    }

    // vscpfilter
    if ( 0 < web_get_var( pParams, lenParam, "vscpfilter", buf, sizeof(buf) ) ) {
        keypairs[_("VSCPFILTER")] = wxString::FromUTF8( buf );
    }

    // vscpmask
    if ( 0 < web_get_var( pParams, lenParam, "vscpmask", buf, sizeof( buf ) ) ) {
        keypairs[ _( "VSCPMASK" ) ] = wxString::FromUTF8( buf );
    }

    // variable
    if ( 0 < web_get_var( pParams, lenParam, "variable", buf, sizeof(buf) ) ) {
        keypairs[_("VARIABLE")] = wxString::FromUTF8( buf );
    }

    // value
    if ( 0 < web_get_var( pParams, lenParam, "value", buf, sizeof( buf ) ) ) {
        keypairs[ _( "VALUE" ) ] = wxString::FromUTF8( buf );
    }

    // type (number or string)
    if ( 0 < web_get_var( pParams, lenParam, "type", buf, sizeof(buf) ) ) {
        keypairs[_("TYPE")] = wxString::FromUTF8( buf );
    }

    // persistent
    if ( 0 < web_get_var( pParams, lenParam, "persistent", buf, sizeof( buf ) ) ) {
        keypairs[ _( "PERSISTENT" ) ] = wxString::FromUTF8( buf );
    }
    
    // access-right  (hex or decimal)
    if ( 0 < web_get_var( pParams, lenParam, "accessright", buf, sizeof( buf ) ) ) {
        keypairs[ _( "ACCESSRIGHT" ) ] = wxString::FromUTF8( buf );
    }

    // note
    if ( 0 < web_get_var( pParams, lenParam, "note", buf, sizeof( buf ) ) ) {
        keypairs[ _( "NOTE" ) ] = wxString::FromUTF8( buf );
    }
    
    // listlong
    if ( 0 < web_get_var( pParams, lenParam, "listlong", buf, sizeof( buf ) ) ) {
        keypairs[ _( "LISTLONG" ) ] = wxString::FromUTF8( buf );
    }
    
    // regex
    if ( 0 < web_get_var( pParams, lenParam, "regex", buf, sizeof( buf ) ) ) {
        keypairs[ _( "REGEX" ) ] = wxString::FromUTF8( buf );
    }

    // unit
    if ( 0 < web_get_var( pParams, lenParam, "unit", buf, sizeof(buf) ) ) {
        keypairs[_("UNIT")] = wxString::FromUTF8( buf );
    }

    // sensoridx
    if ( 0 < web_get_var( pParams, lenParam, "sensoridx", buf, sizeof(buf) ) ) {
        keypairs[_("SENSORINDEX")] = wxString::FromUTF8( buf );
    }

    // level  ( VSCP level 1 or 2 )
    if ( 0 < web_get_var( pParams, lenParam, "level", buf, sizeof( buf ) ) ) {
        keypairs[ _( "LEVEL" ) ] = wxString::FromUTF8( buf );
    }

    // zone
    if ( 0 < web_get_var( pParams, lenParam, "zone", buf, sizeof( buf ) ) ) {
        keypairs[ _( "ZONE" ) ] = wxString::FromUTF8( buf );
    }

    // subzone
    if ( 0 < web_get_var( pParams, lenParam, "subzone", buf, sizeof( buf ) ) ) {
        keypairs[ _( "SUBZONE" ) ] = wxString::FromUTF8( buf );
    }

    // guid
    if ( 0 < web_get_var( pParams, lenParam, "guid", buf, sizeof( buf ) ) ) {
        keypairs[ _( "GUID" ) ] = wxString::FromUTF8( buf );
    }

    // name
    if ( 0 < web_get_var( pParams, lenParam, "name", buf, sizeof(buf) ) ) {
        keypairs[_("NAME")] = wxString::FromUTF8( buf );
    }

    // from
    if ( 0 < web_get_var( pParams, lenParam, "from", buf, sizeof(buf) ) ) {
        keypairs[_("FROM")] = wxString::FromUTF8( buf );
    }

    // to
    if ( 0 < web_get_var( pParams, lenParam, "to", buf, sizeof(buf) ) ) {
        keypairs[_("TO")] = wxString::FromUTF8( buf );
    }

    // url
    if ( 0 < web_get_var( pParams, lenParam, "url", buf, sizeof( buf ) ) ) {
        keypairs[ _( "URL" ) ] = wxString::FromUTF8( buf );
    }

    // eventformat
    if ( 0 < web_get_var( pParams, lenParam, "eventformat", buf, sizeof( buf ) ) ) {
        keypairs[ _( "EVENTFORMAT" ) ] = wxString::FromUTF8( buf );
    }
    
    // datetime
    if ( 0 < web_get_var( pParams, lenParam, "datetime", buf, sizeof( buf ) ) ) {
        keypairs[ _( "DATETIME" ) ] = wxString::FromUTF8( buf );
    }
    
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
        websrv_sendheader( conn, 400, "text/plain" );
        web_write( conn, 
                        REST_PLAIN_ERROR_UNSUPPORTED_FORMAT,
                        strlen( REST_PLAIN_ERROR_UNSUPPORTED_FORMAT ) );
        web_write( conn, "", 0 );   // Terminator
        return WEB_ERROR;
    }

    // If we have a session key we try to get the session
    if ( _("") != keypairs[_("VSCPSESSION")] ) {
 
        // Get session
        pSession = restsrv_get_session( conn, keypairs[_("VSCPSESSION")] );
        
    }

    if ( NULL == pSession ) {

        // Get user
        pUserItem = gpobj->m_userList.getUser( keypairs[_("VSCPUSER")] );

        // Check if user is valid
        if ( NULL == pUserItem ) {
            wxString strErr =
            wxString::Format( _("[REST Client] Host [%s] Invalid user [%s]\n"),
                                wxString::FromUTF8( reqinfo->remote_addr ).mbc_str(),
                                (const char *)keypairs[_("VSCPUSER")].mbc_str() );
            gpobj->logMsg( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_SECURITY );
            restsrv_error( conn, pSession, format, REST_ERROR_CODE_INVALID_USER );
            return WEB_ERROR;
        }

        // Check if remote ip is valid
        bool bValidHost;
        
        gpobj->m_mutexUserList.Lock();
        bValidHost = ( 1 == pUserItem->isAllowedToConnect( inet_addr( reqinfo->remote_addr ) ) );
        gpobj->m_mutexUserList.Unlock();
        if (!bValidHost) {
            wxString strErr =
            wxString::Format( _("[REST Client] Host [%s] NOT allowed to connect. User [%s]\n"),
                                wxString::FromUTF8( reqinfo->remote_addr ).mbc_str(),
                                (const char *)keypairs[_("VSCPUSER")].mbc_str() );
            gpobj->logMsg ( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_SECURITY );
            restsrv_error( conn, pSession, format, REST_ERROR_CODE_INVALID_ORIGIN );
            return WEB_ERROR;
        }

        // Is this an authorised user?
        gpobj->m_mutexUserList.Lock();
        CUserItem *pValidUser = 
                gpobj->m_userList.validateUser( keypairs[_("VSCPUSER")], 
                                                    keypairs[_("VSCPSECRET")] );
        gpobj->m_mutexUserList.Unlock();
        
        if ( NULL == pValidUser ) {    
            wxString strErr =
            wxString::Format( _("[REST Client] User [%s] NOT allowed to connect. Client [%s]\n"),
                                (const char *)keypairs[_("VSCPUSER")].mbc_str(),
                                wxString::FromUTF8( reqinfo->remote_addr  ).mbc_str() );
            gpobj->logMsg ( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_SECURITY );
            restsrv_error( conn, pSession, format, REST_ERROR_CODE_INVALID_PASSWORD );
            return WEB_ERROR;
        }
        
        if ( NULL == ( pSession = restsrv_add_session( conn, pUserItem ) ) ) {
            
            // Hm,,, did not work out well...
            
            wxString strErr =
                wxString::Format( _("[REST Client] Unable to create new session for user [%s]\n"),
                                    (const char *)keypairs[_("VSCPUSER")].mbc_str() );
            gpobj->logMsg ( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_GENERAL );
        
            restsrv_error( conn, pSession, format, REST_ERROR_CODE_INVALID_ORIGIN );
            return WEB_ERROR;
        }
        
        // Only the "open" command is allowed here
        if ( ( _("1") == keypairs[_("OP")] ) || 
              ( _("OPEN") == keypairs[_("OP")].Upper() ) ) {
            restsrv_doOpen( conn, pSession, format );
            return WEB_OK;
        }
        
        // !!! No meaning to go further - end it here !!!
        
        wxString strErr =
        wxString::Format( _("[REST Client] Unable to create new session for user [%s]\n"),
                                (const char *)keypairs[_("VSCPUSER")].mbc_str() );
        gpobj->logMsg ( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_GENERAL );
        
        restsrv_error( conn, pSession, format, REST_ERROR_CODE_INVALID_ORIGIN );
        return WEB_ERROR;

    }
    
    // Check if remote ip is valid
    bool bValidHost;
    gpobj->m_mutexUserList.Lock();
    bValidHost = 
            ( 1 == pSession->m_pClientItem->m_pUserItem->isAllowedToConnect( inet_addr( reqinfo->remote_addr ) ) );
    gpobj->m_mutexUserList.Unlock();
    if ( !bValidHost ) {
        wxString strErr =
        wxString::Format( _("[REST Client] Host [%s] NOT allowed to connect. User [%s]\n"),
                                wxString::FromUTF8( reqinfo->remote_addr  ).mbc_str(),
                                (const char *)keypairs[_("VSCPUSER")].mbc_str() );
        gpobj->logMsg ( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_SECURITY );
        restsrv_error( conn, pSession, format, REST_ERROR_CODE_INVALID_ORIGIN );
        return WEB_ERROR;
    }
    
    // ------------------------------------------------------------------------
    //                      * * * User is validated * * *
    // ------------------------------------------------------------------------

    wxString strErr =
        wxString::Format( _("[REST Client] User [%s] Host [%s] allowed to connect. \n"),
                            (const char *)keypairs[_("VSCPUSER")].mbc_str() ,
                            wxString::FromUTF8( reqinfo->remote_addr  ).mbc_str() );
        gpobj->logMsg ( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_SECURITY );

    

    //   *************************************************************
    //   * * * * * * * *  Status (hold session open)   * * * * * * * *
    //   *************************************************************
    if ( ( _("0") == keypairs[_("OP")] ) ||  
         ( _("STATUS") == keypairs[_("OP")].Upper() ) ) {
        restsrv_doStatus( conn, pSession, format );
    }

    //  ********************************************
    //  * * * * * * * * open session * * * * * * * *
    //  ********************************************
    else if ( ( _("1") == keypairs[_("OP")] ) || 
              ( _("OPEN") == keypairs[_("OP")].Upper() ) ) {
        restsrv_doOpen( conn, pSession, format );
    }

    //   **********************************************
    //   * * * * * * * * close session  * * * * * * * *
    //   **********************************************
    else if ( ( _("2") == keypairs[_("OP")] ) || 
              ( _("CLOSE") == keypairs[_("OP")].Upper() ) ) {
        restsrv_doClose( conn, pSession, format );
    }

    //  ********************************************
    //   * * * * * * * * Send event  * * * * * * * *
    //  ********************************************
    else if ( ( _("3") == keypairs[_("OP")] ) || 
              ( _("SENDEVENT") == keypairs[_("OP")].Upper() ) ) {
        vscpEvent vscpevent;
        if ( _("") != keypairs[_("VSCPEVENT")] ) {
            vscp_setVscpEventFromString( &vscpevent, keypairs[_("VSCPEVENT")] );
            restsrv_doSendEvent( conn, pSession, format, &vscpevent );
        }
        else {
            // Parameter missing - No Event
            restsrv_error( conn, 
                            pSession, 
                            format, 
                            REST_ERROR_CODE_MISSING_DATA );
        }
    }

    //  ********************************************
    //   * * * * * * * * Read event  * * * * * * * *
    //  ********************************************
    else if ( ( _("4") == keypairs[_("OP")] ) || 
              ( _("READEVENT") == keypairs[_("OP")].Upper() ) ) {
        long count = 1;
        if ( _("") != keypairs[_("COUNT")].Upper() ) {
            keypairs[_("COUNT")].ToLong( &count );
        }
        restsrv_doReceiveEvent( conn, pSession, format, count );
    }

    //   **************************************************
    //   * * * * * * * *     Set filter    * * * * * * * *
    //   **************************************************
    else if ( ( _("5") == keypairs[_("OP")] ) || 
              ( _("SETFILTER") == keypairs[_("OP")].Upper() ) ) {

        vscpEventFilter vscpfilter;
        vscp_clearVSCPFilter( &vscpfilter );

        if ( _("") != keypairs[_("VSCPFILTER")] ) {
            vscp_readFilterFromString( &vscpfilter, keypairs[_("VSCPFILTER")] );
        }
        else {
            restsrv_error( conn, pSession, format, REST_ERROR_CODE_MISSING_DATA );
        }

        if ( _( "" ) != keypairs[ _("VSCPMASK") ] ) {
            vscp_readMaskFromString( &vscpfilter, keypairs[ _( "VSCPMASK" ) ] );
        }
        else {
            restsrv_error( conn, pSession, format, REST_ERROR_CODE_MISSING_DATA );
        }

        restsrv_doSetFilter( conn, pSession, format, vscpfilter );

    }

    //   ****************************************************
    //   * * * * * * * *  clear input queue   * * * * * * * *
    //   ****************************************************
    else if ( ( _("6") == keypairs[_("OP")] ) || 
              ( _("CLEARQUEUE") == keypairs[_("OP")].Upper() ) ) {
        restsrv_doClearQueue( conn, pSession, format );
    }
        
    //   ****************************************************
    //   * * * * * * * *   list variables     * * * * * * * *
    //   ****************************************************
    else if ( ( _("12") == keypairs[_("OP")] ) || 
              ( _("LISTVAR") == keypairs[_("OP")].Upper() ) ) {
        
        bool bShort = true;        
        
        if ( wxNOT_FOUND !=  
                keypairs[ _( "LISTLONG" ) ].Upper().Find( _("TRUE") ) ) {
            bShort = false;
        }
        
        restsrv_doListVariable( conn, 
                                    pSession, 
                                    format, 
                                    keypairs[_("REGEX")], 
                                    bShort );
        
    }    

    //   ****************************************************
    //   * * * * * * * * read variable value  * * * * * * * *
    //   ****************************************************
    else if ( ( _("7") == keypairs[_("OP")] ) || 
              ( _("READVAR") == keypairs[_("OP")].Upper() ) ) {
        if ( _("") != keypairs[_("VARIABLE")] ) {
            restsrv_doReadVariable( conn, 
                                        pSession, 
                                        format, 
                                        keypairs[_("VARIABLE")] );
        }
        else {
            restsrv_error( conn, 
                                    pSession, 
                                    format, 
                                    REST_ERROR_CODE_MISSING_DATA );
        }
    }

    //   *****************************************************
    //   * * * * * * * * Write variable value  * * * * * * * *
    //   *****************************************************
    else if ( ( _("8") == keypairs[_("OP")] ) || 
              ( _("WRITEVAR") == keypairs[_("OP")].Upper() ) ) {

        if ( _("") != keypairs[_("VARIABLE")] ) {
            restsrv_doWriteVariable( conn, 
                                            pSession, 
                                            format, 
                                            keypairs[_("VARIABLE")], 
                                            keypairs[ _( "VALUE" ) ] );
        }
        else {
            restsrv_error( conn, 
                                    pSession, 
                                    format, 
                                    REST_ERROR_CODE_MISSING_DATA );
        }
    }


    //   *****************************************************
    //   * * * * * * * *   Create variable    * * * * * * * *
    //   *****************************************************

    else if ( ( _("9") == keypairs[_("OP")] ) || 
              ( _("CREATEVAR") == keypairs[_("OP")].Upper() ) ) {


        if ( _("") != keypairs[_("VARIABLE")] ) {
            restsrv_doCreateVariable( conn,
                                            pSession,
                                            format,
                                            keypairs[_("VARIABLE")],
                                            keypairs[ _( "TYPE" ) ],
                                            keypairs[ _( "VALUE" ) ],
                                            keypairs[ _( "PERISTENT" ) ],
                                            keypairs[ _( "ACCESSRIGHT" ) ],
                                            keypairs[ _( "NOTE" ) ] );
        }
        else {
            restsrv_error( conn, 
                                    pSession, 
                                    format, 
                                    REST_ERROR_CODE_MISSING_DATA );
        }
    }


    //   *****************************************************
    //   * * * * * * * *   Delete  variable    * * * * * * * *
    //   *****************************************************

    else if ( ( _("10") == keypairs[_("OP")] ) || 
              ( _("DELETEVAR") == keypairs[_("OP")].Upper() ) ) {


        if ( _("") != keypairs[_("VARIABLE")] ) {
            restsrv_doDeleteVariable( conn,
                                            pSession,
                                            format,
                                            keypairs[ _("VARIABLE") ] );
        }
        else {
            restsrv_error( conn, 
                                    pSession, 
                                    format, 
                                    REST_ERROR_CODE_MISSING_DATA );
        }
    }


    //   *************************************************
    //   * * * * * * * * Send measurement  * * * * * * * *
    //   *************************************************
    //   value,unit=0,sensor=0
    //
    else if ( ( _("10") == keypairs[_("OP")] ) || 
              ( _("MEASUREMENT") == keypairs[_("OP")].Upper() ) ) {

        if ( ( _("") != keypairs[_("VALUE")] ) && 
             ( _("") != keypairs[_("TYPE")]) ) {

            restsrv_doWriteMeasurement( conn, pSession, format,
                                                    keypairs[ _("DATETIME" ) ],
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
            restsrv_error( conn, 
                                    pSession, 
                                    format, 
                                    REST_ERROR_CODE_MISSING_DATA );
        }
    }

    //   *******************************************
    //   * * * * * * * * Table read  * * * * * * * *
    //   *******************************************
    else if ( ( _("11") == keypairs[_("OP")] ) || 
              ( _("TABLE") == keypairs[_("OP")].Upper() ) ) {

        if ( _("") != keypairs[_("NAME")] ) {

            restsrv_doGetTableData( conn, 
                                            pSession, 
                                            format,
                                            keypairs[_("NAME")],
                                            keypairs[_("FROM")],
                                            keypairs[_("TO")] );
        }
        else {
            restsrv_error( conn, 
                                    pSession, 
                                    format, 
                                    REST_ERROR_CODE_MISSING_DATA );
        }
    }

    //   *******************************************
    //   * * * * * * * * Fetch MDF  * * * * * * * *
    //   *******************************************
    else if ( ( _( "12" ) == keypairs[ _( "OP" ) ] ) || 
              ( _( "MDF" ) == keypairs[ _( "OP" ) ].Upper() ) ) {

        if ( _( "" ) != keypairs[ _( "URL" ) ] ) {

            restsrv_doFetchMDF( conn, 
                                        pSession, 
                                        format, 
                                        keypairs[ _( "URL" ) ] );
        }
        else {
            restsrv_error( conn, 
                                    pSession, 
                                    format, 
                                    REST_ERROR_CODE_MISSING_DATA );
        }
    }
        
    // Unrecognised operation        
        
    else {
        restsrv_error( conn, 
                                pSession, 
                                format, 
                                REST_ERROR_CODE_MISSING_DATA );
    }    

    return WEB_OK;    
}





///////////////////////////////////////////////////////////////////////////////
// restsrv_doOpen
//

void
restsrv_doOpen( struct web_connection *conn,
                    struct restsrv_session *pSession,
                    int format )
{
    char buf[ 2048 ];
    char wrkbuf[ 256 ];
    
    if ( NULL != pSession ) {

        // OK session
        
        // Note activity
        pSession->m_lastActiveTime = time( NULL );
        
        // Mark interface as open
        pSession->m_pClientItem->m_bOpen = true;

        if ( REST_FORMAT_PLAIN == format ) {

            websrv_sendSetCookieHeader( conn, 
                                            200, 
                                            REST_MIME_TYPE_PLAIN,
                                            pSession->m_sid  );

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
                                pSession->m_sid,
                                pSession->m_pClientItem->m_clientInputQueue.GetCount() );
#endif
            web_write( conn, wrkbuf, n );
            web_write( conn, "", 0 );  // Terminator
            return;
        }
        else if ( REST_FORMAT_CSV == format ) {

            websrv_sendSetCookieHeader( conn, 
                                            200, 
                                            REST_MIME_TYPE_CSV,
                                            pSession->m_sid  );

#ifdef WIN32
            int n = _snprintf( wrkbuf,
                               sizeof( wrkbuf ),
                               "success-code,error-code,message,description,"
                               "vscpsession,nEvents\r\n1,1,Success,Success. 1,1"
                               ",Success,Success,%s,%zd",
                               pSession->sid, 
                               pSession->pClientItem->m_clientInputQueue.GetCount() );
#else
            int n = snprintf( wrkbuf,
                              sizeof( wrkbuf ),
                              "success-code,error-code,message,description,"
                              "vscpsession,nEvents\r\n1,1,Success,Success. 1,1,"
                              "Success,Success,%s,%lu",
                              pSession->m_sid, 
                              pSession->m_pClientItem->m_clientInputQueue.GetCount() );
#endif
            web_write( conn, wrkbuf, strlen( wrkbuf ) );
            web_write( conn, "", 0 );  // Terminator
            return;
        }
        else if ( REST_FORMAT_XML == format ) {

            websrv_sendSetCookieHeader( conn, 
                                            200, 
                                            REST_MIME_TYPE_XML,
                                            pSession->m_sid  );

#ifdef WIN32
            int n = _snprintf( wrkbuf,
                               sizeof( wrkbuf ),
                               "<vscp-rest success = \"true\" code = "
                               "\"1\" message = \"Success.\" description = "
                               "\"Success.\" ><vscpsession>%s</vscpsession>"
                               "<nEvents>%zd</nEvents></vscp-rest>",
                               pSession->sid, 
                               pSession->pClientItem->m_clientInputQueue.GetCount() );
#else
            int n = snprintf( wrkbuf,
                              sizeof( wrkbuf ),
                              "<vscp-rest success = \"true\" code = \"1\" "
                              "message = \"Success.\" description = \"Success.\" "
                              "><vscpsession>%s</vscpsession><nEvents>%lu"
                              "</nEvents></vscp-rest>",
                              pSession->m_sid, 
                              pSession->m_pClientItem->m_clientInputQueue.GetCount() );
#endif
            web_write( conn, wrkbuf, strlen( wrkbuf ) );
            web_write( conn, "", 0 );  // Terminator
            return;
        }
        else if ( REST_FORMAT_JSON == format ) {
            
            json output;

            websrv_sendSetCookieHeader( conn, 
                                            200, 
                                            REST_MIME_TYPE_JSON,
                                            pSession->m_sid  );

            output["success"] = true;
            output["code"] = 1;
            output["message"] = "success";
            output["description"] = "Success";
            output["vscpsession"] = pSession->m_sid;
            output["nEvents"] = pSession->m_pClientItem->m_clientInputQueue.GetCount();
            std::string s = output.dump();
            web_write( conn, s.c_str(), s.length() );
                    
            return;
        }
        else if ( REST_FORMAT_JSONP == format ) {

            json output;
            
            websrv_sendSetCookieHeader( conn, 
                                            200, 
                                            REST_MIME_TYPE_JSONP,
                                            pSession->m_sid );

            // Write JSONP start block
            web_write( conn, 
                        REST_JSONP_START, 
                        strlen( REST_JSONP_START ) );
            
            output["success"] = true;
            output["code"] = 1;
            output["message"] = "success";
            output["description"] = "Success";
            output["vscpsession"] = pSession->m_sid;
            output["nEvents"] = pSession->m_pClientItem->m_clientInputQueue.GetCount();
            std::string s = output.dump();
            web_write( conn, s.c_str(), s.length() );
            
            // Write JSONP end block
            web_write( conn, 
                        REST_JSONP_END, 
                        strlen( REST_JSONP_END ) );            
            return;
        }
        else {
            websrv_sendheader( conn, 400, REST_MIME_TYPE_PLAIN );
            web_write( conn, 
                        REST_PLAIN_ERROR_UNSUPPORTED_FORMAT, 
                        strlen( REST_PLAIN_ERROR_UNSUPPORTED_FORMAT ) );
            return;
        }
    }
    else {      // Unable to create session
        restsrv_error( conn, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
    }

    return;
}

///////////////////////////////////////////////////////////////////////////////
// restsrv_doClose
//

void
restsrv_doClose( struct web_connection *conn,
                        struct restsrv_session *pSession,
                        int format )
{
    char buf[ 2048 ];
    char wrkbuf[ 256 ];

    if ( NULL != pSession ) {

        char sid[ 32 + 1 ];
        memset( sid, 0, sizeof( sid ) );
        memcpy( sid, pSession->m_sid, sizeof( sid ) );

        // We should close the session
        
        // Mark as closed
        pSession->m_pClientItem->m_bOpen = false;
        
        // Note activity
        pSession->m_lastActiveTime = time( NULL );

        if ( REST_FORMAT_PLAIN == format ) {

            websrv_sendheader( conn, 200, REST_MIME_TYPE_PLAIN );
            web_write( conn, REST_PLAIN_ERROR_SUCCESS, strlen( REST_PLAIN_ERROR_SUCCESS ) );
            web_write( conn, "", 0 );  // Terminator
            return;

        }
        else if ( REST_FORMAT_CSV == format ) {

            websrv_sendheader( conn, 200, REST_MIME_TYPE_CSV );
#ifdef WIN32
            int n = _snprintf( wrkbuf, sizeof( wrkbuf ), REST_CSV_ERROR_SUCCESS );
#else
            int n = snprintf( wrkbuf, sizeof( wrkbuf ), REST_CSV_ERROR_SUCCESS );
#endif
            web_write( conn, wrkbuf, strlen( wrkbuf ) );
            web_write( conn, "", 0 );  // Terminator
            return;
        }
        else if ( REST_FORMAT_XML == format ) {

            websrv_sendheader( conn, 200, REST_MIME_TYPE_XML );
#ifdef WIN32
            int n = _snprintf( wrkbuf, sizeof( wrkbuf ), REST_XML_ERROR_SUCCESS );
#else
            int n = snprintf( wrkbuf, sizeof( wrkbuf ), REST_XML_ERROR_SUCCESS );
#endif
            web_write( conn, wrkbuf, strlen( wrkbuf ) );
            web_write( conn, "", 0 );  // Terminator
            return;
        }
        else if ( REST_FORMAT_JSON == format ) {
            
            websrv_sendheader( conn, 200, REST_MIME_TYPE_JSON );
#ifdef WIN32
            int n = _snprintf( wrkbuf, sizeof( wrkbuf ), REST_JSON_ERROR_SUCCESS );
#else
            int n = snprintf( wrkbuf, sizeof( wrkbuf ), REST_JSON_ERROR_SUCCESS );
#endif
            web_write( conn, wrkbuf, strlen( wrkbuf ) );
            web_write( conn, "", 0 );  // Terminator
            return;
        }
        else if ( REST_FORMAT_JSONP == format ) {
            
            websrv_sendheader( conn, 200, REST_MIME_TYPE_JSONP );
#ifdef WIN32
            int n = _snprintf( wrkbuf, sizeof( wrkbuf ), REST_JSONP_ERROR_SUCCESS );
#else
            int n = snprintf( wrkbuf, sizeof( wrkbuf ), REST_JSONP_ERROR_SUCCESS );
#endif
            web_write( conn, wrkbuf, strlen( wrkbuf ) );
            web_write( conn, "", 0 );  // Terminator
            return;
        }
        else {
            websrv_sendheader( conn, 400, REST_MIME_TYPE_PLAIN );
            web_write( conn,
                            REST_PLAIN_ERROR_UNSUPPORTED_FORMAT,
                            strlen( REST_PLAIN_ERROR_UNSUPPORTED_FORMAT ) );
            web_write( conn, "", 0 ); // Terminator
            return;
        }

    }
    else {  // session not found
        restsrv_error( conn, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
    }

    return;
}

///////////////////////////////////////////////////////////////////////////////
// restsrv_doStatus
//

void
restsrv_doStatus( struct web_connection *conn,
                            struct restsrv_session *pSession,
                            int format )
{
    char buf[ 2048 ];
    char wrkbuf[ 256 ];

    if ( NULL != pSession ) {

        // Note activity
        pSession->m_lastActiveTime = time( NULL );

        if ( REST_FORMAT_PLAIN == format ) {
            websrv_sendheader( conn, 200, REST_MIME_TYPE_PLAIN );
            web_write( conn, 
                            REST_PLAIN_ERROR_SUCCESS, 
                            strlen( REST_PLAIN_ERROR_SUCCESS ) );
            memset( buf, 0, sizeof( buf ) );
#ifdef WIN32
            int n = _snprintf( wrkbuf, 
                        sizeof( wrkbuf ), 
                        "vscpsession=%s nEvents=%zd", 
                        pSession->sid, 
                        pSession->pClientItem->m_clientInputQueue.GetCount() );
#else
            int n = snprintf( wrkbuf,
                        sizeof( wrkbuf ),
                        "1 1 Success vscpsession=%s nEvents=%lu",
                        pSession->m_sid,
                        pSession->m_pClientItem->m_clientInputQueue.GetCount() );
#endif
            web_write( conn, wrkbuf, strlen( wrkbuf ) );
            web_write( conn, "", 0 );  // Terminator
            return;
        }
        else if ( REST_FORMAT_CSV == format ) {
            websrv_sendheader( conn, 200, REST_MIME_TYPE_CSV );
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
                              pSession->m_sid, pSession->m_pClientItem->m_clientInputQueue.GetCount() );
#endif
            web_write( conn, wrkbuf, strlen( wrkbuf ) );
            web_write( conn, "", 0 );  // Terminator
            return;
        }
        else if ( REST_FORMAT_XML == format ) {
            websrv_sendheader( conn, 200, REST_MIME_TYPE_XML );
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
                              pSession->m_sid, pSession->m_pClientItem->m_clientInputQueue.GetCount() );
#endif
            web_write( conn, wrkbuf, strlen( wrkbuf ) );
            web_write( conn, "", 0 );  // Terminator
            return;
        }
        else if ( ( REST_FORMAT_JSON == format ) ||
                  ( REST_FORMAT_JSONP == format ) ) {
            
            json output;

            if ( REST_FORMAT_JSON == format ) {
                websrv_sendheader( conn, 200, REST_MIME_TYPE_JSON );
            }
            else {
                
                websrv_sendheader( conn, 200, REST_MIME_TYPE_JSONP );
                
                // Write JSONP start block
                web_write( conn, 
                            REST_JSONP_START, 
                            strlen( REST_JSONP_START ) );
                
            }    
            
            output["success"] = true;
            output["code"] = 1;
            output["message"] = "success";
            output["description"] = "Success";
            output["vscpsession"] = pSession->m_sid;
            output["nEvents"] = pSession->m_pClientItem->m_clientInputQueue.GetCount();
            std::string s = output.dump();
            web_write( conn, s.c_str(), s.length() );
            
            if ( REST_FORMAT_JSONP == format ) {
                // Write JSONP end block
                web_write( conn, 
                            REST_JSONP_END, 
                            strlen( REST_JSONP_END ) );
            }

            return;
        }
        else {
            websrv_sendheader( conn, 400, REST_MIME_TYPE_PLAIN );
            web_write( conn, REST_PLAIN_ERROR_UNSUPPORTED_FORMAT, strlen( REST_PLAIN_ERROR_UNSUPPORTED_FORMAT ) );
            web_write( conn, "", 0 );  // Terminator
            return;
        }

    } // No session
    else {
        restsrv_error( conn, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
    }

    return;
}

///////////////////////////////////////////////////////////////////////////////
// restsrv_doSendEvent
//

void
restsrv_doSendEvent( struct web_connection *conn,
                        struct restsrv_session *pSession,
                        int format,
                        vscpEvent *pEvent )
{
    bool bSent = false;

    // Check pointer
    if (NULL == conn) return;

    if ( NULL != pSession ) {

        // Level II events between 512-1023 is recognised by the daemon and
        // sent to the correct interface as Level I events if the interface
        // is addressed by the client.
        if ( ( pEvent->vscp_class <= 1023 ) &&
                ( pEvent->vscp_class >= 512 ) &&
                ( pEvent->sizeData >= 16 ) ) {

            // This event should be sent to the correct interface if it is
            // available on this machine. If not it should be sent to
            // the rest of the network as normal

            cguid destguid;
            destguid.getFromArray( pEvent->pdata );
            destguid.setAt(0,0);
            destguid.setAt(1,0);

            if ( NULL != pSession->m_pClientItem ) {

                // Set client id
                pEvent->obid = pSession->m_pClientItem->m_clientID;

                // Check if filtered out
                if ( vscp_doLevel2Filter( pEvent, &pSession->m_pClientItem->m_filterVSCP ) ) {

                    // Lock client
                    gpobj->m_wxClientMutex.Lock();

                    // If the client queue is full for this client then the
                    // client will not receive the message
                    if (pSession->m_pClientItem->m_clientInputQueue.GetCount() <=
                            gpobj->m_maxItemsInClientReceiveQueue) {

                        vscpEvent *pNewEvent = new( vscpEvent );
                        if ( NULL != pNewEvent ) {

                            vscp_copyVSCPEvent(pNewEvent, pEvent);

                            // Add the new event to the input queue
                            pSession->m_pClientItem->m_mutexClientInputQueue.Lock();
                            pSession->m_pClientItem->m_clientInputQueue.Append( pNewEvent );
                            pSession->m_pClientItem->m_semClientInputQueue.Post();

                            bSent = true;
                        }
                        else {
                            bSent = false;
                        }

                    }
                    else {
                        // Overrun - No room for event
                        //vscp_deleteVSCPevent( pEvent );
                        bSent = true;
                    }

                    // Unlock client
                    gpobj->m_wxClientMutex.Unlock();

                } // filter

            } // Client found
        }

        if ( !bSent ) {

            if ( NULL != pSession->m_pClientItem ) {

                // Set client id
                pEvent->obid = pSession->m_pClientItem->m_clientID;

                // There must be room in the send queue
                if (gpobj->m_maxItemsInClientReceiveQueue >
                        gpobj->m_clientOutputQueue.GetCount()) {

                    vscpEvent *pNewEvent = new( vscpEvent );
                    if ( NULL != pNewEvent ) {
                        vscp_copyVSCPEvent(pNewEvent, pEvent);

                        gpobj->m_mutexClientOutputQueue.Lock();
                        gpobj->m_clientOutputQueue.Append(pNewEvent);
                        gpobj->m_semClientOutputQueue.Post();
                        gpobj->m_mutexClientOutputQueue.Unlock();

                        bSent = true;
                    }
                    else {
                        bSent = false;
                    }

                    restsrv_error( conn, pSession, format, REST_ERROR_CODE_SUCCESS );

                }
                else {
                    restsrv_error( conn, pSession, format, REST_ERROR_CODE_NO_ROOM );
                    vscp_deleteVSCPevent( pEvent );
                    bSent = false;
                }

            }
            else {
                restsrv_error( conn, pSession, format, REST_ERROR_CODE_GENERAL_FAILURE );
                vscp_deleteVSCPevent( pEvent );
                bSent = false;
            }

        } // not sent
    }
    else {
        restsrv_error( conn, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
        bSent = false;
    }

    return;
}


///////////////////////////////////////////////////////////////////////////////
// restsrv_doReceiveEvent
//

void
restsrv_doReceiveEvent( struct web_connection *conn,
                                struct restsrv_session *pSession,
                                int format,
                                size_t count )
{
    // Check pointer
    if (NULL == conn) return;

    if ( NULL != pSession ) {

        if ( !pSession->m_pClientItem->m_clientInputQueue.empty() ) {

            char buf[32000];
            char wrkbuf[32000];
            wxString out;
            size_t cntAvailable = 
                    pSession->m_pClientItem->m_clientInputQueue.GetCount();

            // Plain
            if ( REST_FORMAT_PLAIN == format ) {

                // Send header
                websrv_sendheader( conn, 200, REST_MIME_TYPE_PLAIN );

                if ( pSession->m_pClientItem->m_bOpen && cntAvailable ) {

                    sprintf( wrkbuf, "1 1 Success \r\n");
                    web_write( conn, wrkbuf, strlen( wrkbuf ) );
                    sprintf( wrkbuf,
#if WIN32
                                "%zd events requested of %zd available "
                                "(unfiltered) %zu will be retrieved\r\n",
#else
                                "%zd events requested of %zd available "
                                "(unfiltered) %lu will be retrieved\r\n",
#endif
                                count,
                                pSession->m_pClientItem->m_clientInputQueue.GetCount(),
                                MIN( count, cntAvailable ) );

                    web_write( conn, wrkbuf, strlen( wrkbuf ) );

                    for ( unsigned int i=0; i<MIN( count, cntAvailable ); i++ ) {

                        CLIENTEVENTLIST::compatibility_iterator nodeClient;
                        vscpEvent *pEvent;

                        pSession->m_pClientItem->m_mutexClientInputQueue.Lock();
                        nodeClient = pSession->m_pClientItem->m_clientInputQueue.GetFirst();
                        pEvent = nodeClient->GetData();
                        pSession->m_pClientItem->m_clientInputQueue.DeleteNode( nodeClient );
                        pSession->m_pClientItem->m_mutexClientInputQueue.Unlock();

                        if (NULL != pEvent) {

                            if ( vscp_doLevel2Filter( pEvent, 
                                    &pSession->m_pClientItem->m_filterVSCP ) ) {

                                wxString str;
                                if ( vscp_writeVscpEventToString( pEvent, str ) ) {

                                    // Write it out
                                    strcpy((char *) wrkbuf, (const char*) "- ");
                                    strcat((char *) wrkbuf, 
                                            (const char*) str.mb_str(wxConvUTF8));
                                    strcat((char *) wrkbuf, "\r\n" );
                                    web_write( conn, wrkbuf, strlen( wrkbuf) );

                                }
                                else {
                                    strcpy((char *) wrkbuf, 
                                            "- Malformed event (internal error)\r\n" );
                                    web_write( conn, wrkbuf, strlen( wrkbuf) );
                                }
                            }
                            else {
                                strcpy((char *) wrkbuf, "- Event filtered out\r\n" );
                                web_write( conn, wrkbuf, strlen( wrkbuf) );
                            }

                            // Remove the event
                            vscp_deleteVSCPevent(pEvent);

                        } // Valid pEvent pointer
                        else {
                            strcpy((char *) wrkbuf, 
                                    "- Event could not be fetched (internal error)\r\n" );
                            web_write( conn, wrkbuf, strlen( wrkbuf) );
                        }
                    } // for
                }
                else {   // no events available
                    sprintf( wrkbuf, REST_PLAIN_ERROR_INPUT_QUEUE_EMPTY"\r\n");
                    web_write( conn, wrkbuf, strlen( wrkbuf) );
                }

            }

            // CSV
            else if ( REST_FORMAT_CSV == format ) {

                // Send header
                websrv_sendheader( conn, 200, /*REST_MIME_TYPE_CSV*/ REST_MIME_TYPE_PLAIN );

                if ( pSession->m_pClientItem->m_bOpen && cntAvailable ) {

                    sprintf( wrkbuf, "success-code,error-code,message,"
                                     "description,Event\r\n1,1,Success,Success."
                                     ",NULL\r\n");
                    web_write( conn, wrkbuf, strlen( wrkbuf) );
                    sprintf( wrkbuf,
#if WIN32
                             "1,2,Info,%zd events requested of %d available "
                             "(unfiltered) %zu will be retrieved,NULL\r\n",
#else
                             "1,2,Info,%zd events requested of %lu available "
                             "(unfiltered) %lu will be retrieved,NULL\r\n",
#endif
                                count,
                                (unsigned long)cntAvailable,
                                (unsigned long)MIN( count, cntAvailable ) );
                    web_write( conn, wrkbuf, strlen( wrkbuf ) );
                    sprintf( wrkbuf,
                             "1,4,Count,%zu,NULL\r\n",
                             MIN( count, cntAvailable ) );
                    web_write( conn, wrkbuf, strlen( wrkbuf ) );

                    for ( unsigned int i=0; i<MIN( count, cntAvailable ); i++ ) {

                        CLIENTEVENTLIST::compatibility_iterator nodeClient;
                        vscpEvent *pEvent;

                        pSession->m_pClientItem->m_mutexClientInputQueue.Lock();
                        nodeClient = pSession->m_pClientItem->m_clientInputQueue.GetFirst();
                        pEvent = nodeClient->GetData();
                        pSession->m_pClientItem->m_clientInputQueue.DeleteNode(nodeClient);
                        pSession->m_pClientItem->m_mutexClientInputQueue.Unlock();

                        if ( NULL != pEvent ) {

                            if ( vscp_doLevel2Filter( pEvent, 
                                    &pSession->m_pClientItem->m_filterVSCP ) ) {

                                wxString str;
                                if ( vscp_writeVscpEventToString(pEvent, str) ) {

                                    // Write it out
                                    memset((char *) wrkbuf, 0, sizeof( wrkbuf ));
                                    strcpy((char *) wrkbuf, (const char*) "1,3,Data,Event,");
                                    strcat((char *) wrkbuf, (const char*) str.mb_str(wxConvUTF8));
                                    strcat((char *) wrkbuf, "\r\n" );
                                    web_write( conn, wrkbuf, strlen( wrkbuf) );

                                }
                                else {
                                    strcpy((char *) wrkbuf, 
                                            "1,2,Info,Malformed event (internal "
                                            "error)\r\n" );
                                    web_write( conn, wrkbuf, strlen( wrkbuf)  );
                                }
                            }
                            else {
                                strcpy((char *) wrkbuf, 
                                        "1,2,Info,Event filtered out\r\n" );
                                web_write( conn, wrkbuf, strlen( wrkbuf) );
                            }

                            // Remove the event
                            vscp_deleteVSCPevent( pEvent );

                        } // Valid pEvent pointer
                        else {
                            strcpy((char *) wrkbuf, 
                                    "1,2,Info,Event could not be fetched "
                                    "(internal error)\r\n" );
                            web_write( conn, wrkbuf, strlen( wrkbuf) );
                        }
                    } // for
                }
                else {   // no events available
                    sprintf( wrkbuf, REST_CSV_ERROR_INPUT_QUEUE_EMPTY"\r\n");
                    web_write( conn, wrkbuf, strlen( wrkbuf) );
                }

            }

            // XML
            else if ( REST_FORMAT_XML == format ) {

                int filtered = 0;
                int errors = 0;

                // Send header
                websrv_sendheader( conn, 200, REST_MIME_TYPE_XML );


                if ( pSession->m_pClientItem->m_bOpen && cntAvailable ) {
                    
                    
                    sprintf( wrkbuf, 
                                XML_HEADER"<vscp-rest success = \"true\" "
                                "code = \"1\" message = \"Success\" "
                                "description = \"Success.\" >");
                    web_write( conn, wrkbuf, strlen( wrkbuf) );
                    sprintf( wrkbuf,
                                "<info>%zd events requested of %lu available "
                                "(unfiltered) %zu will be retrieved</info>",
                                count,
                                cntAvailable,
                                MIN(count, cntAvailable ) );
                    web_write( conn, wrkbuf, strlen( wrkbuf) );

                    sprintf( wrkbuf,
                             "<count>%zu</count>",
                             MIN( count, cntAvailable ) );
                    web_write( conn, wrkbuf, strlen( wrkbuf ) );

                    for ( unsigned int i=0; i<MIN( (unsigned long)count, 
                                                    cntAvailable ); i++ ) {

                        CLIENTEVENTLIST::compatibility_iterator nodeClient;
                        vscpEvent *pEvent;

                        pSession->m_pClientItem->m_mutexClientInputQueue.Lock();
                        nodeClient = pSession->m_pClientItem->m_clientInputQueue.GetFirst();
                        pEvent = nodeClient->GetData();
                        pSession->m_pClientItem->m_clientInputQueue.DeleteNode(nodeClient);
                        pSession->m_pClientItem->m_mutexClientInputQueue.Unlock();

                        if (NULL != pEvent) {

                            if ( vscp_doLevel2Filter( pEvent, 
                                    &pSession->m_pClientItem->m_filterVSCP ) ) {

                                wxString str;

                                // Write it out
                                strcpy( (char *)wrkbuf, (const char*) "<event>");

                                // head
                                strcat((char *)wrkbuf, (const char*) "<head>");
                                strcat((char *)wrkbuf, 
                                        wxString::Format( _("%d"), 
                                        pEvent->head ).mbc_str() );
                                strcat((char *)wrkbuf, (const char*) "</head>");

                                // class
                                strcat((char *)wrkbuf, (const char*) "<vscpclass>");
                                strcat((char *)wrkbuf, 
                                        wxString::Format( _("%d"), 
                                        pEvent->vscp_class ).mbc_str() );
                                strcat((char *)wrkbuf, (const char*) "</vscpclass>");

                                // type
                                strcat((char *)wrkbuf, (const char*) "<vscptype>");
                                strcat((char *)wrkbuf, 
                                        wxString::Format( _("%d"), 
                                        pEvent->vscp_type ).mbc_str() );
                                strcat((char *)wrkbuf, (const char*) "</vscptype>");

                                // obid
                                strcat((char *)wrkbuf, (const char*) "<obid>");
                                strcat((char *)wrkbuf, 
                                        wxString::Format( _("%lu"), 
                                        pEvent->obid ).mbc_str() );
                                strcat((char *)wrkbuf, (const char*) "</obid>");
                                
                                // datetime
                                strcat((char *)wrkbuf, (const char*) "<datetime>");
                                wxString dt;
                                vscp_getDateStringFromEvent( pEvent, dt );
                                strcat( (char *)wrkbuf, 
                                            wxString::Format( _("%%s"), 
                                                 (const char *)dt.mbc_str() ) );
                                strcat((char *)wrkbuf, (const char*) "</datetime>");

                                // timestamp
                                strcat((char *)wrkbuf, (const char*) "<timestamp>");
                                strcat((char *)wrkbuf, 
                                            wxString::Format( _("%lu"), 
                                            pEvent->timestamp ).mbc_str() );
                                strcat((char *)wrkbuf, (const char*) "</timestamp>");

                                // guid
                                strcat((char *)wrkbuf, (const char*) "<guid>");
                                vscp_writeGuidToString( pEvent, str);
                                strcat((char *)wrkbuf, (const char *)str.mbc_str() );
                                strcat((char *)wrkbuf, (const char*) "</guid>");

                                // sizedate
                                strcat((char *)wrkbuf, (const char*) "<sizedata>");
                                strcat((char *)wrkbuf, 
                                            wxString::Format( _("%d"), 
                                            pEvent->sizeData ).mbc_str() );
                                strcat((char *)wrkbuf, (const char*) "</sizedata>");

                                // data
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
                                web_write( conn, wrkbuf, strlen( wrkbuf) );

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

                    web_write( conn, wrkbuf, strlen( wrkbuf ) );

                    // End tag
                    strcpy((char *) wrkbuf, "</vscp-rest>" );
                    web_write( conn, wrkbuf, strlen( wrkbuf) );

                }
                else {   // no events available

                    sprintf( wrkbuf, REST_XML_ERROR_INPUT_QUEUE_EMPTY"\r\n");
                    web_write( conn, wrkbuf, strlen( wrkbuf) );
                }

            }

            // JSON / JSONP
            else if ( ( REST_FORMAT_JSON == format ) || 
                      ( REST_FORMAT_JSONP == format ) ) {

                int sentEvents = 0;
                int filtered = 0;
                int errors = 0;
                json output;

                // Send header
                if ( REST_FORMAT_JSONP == format ) {
                    websrv_sendheader( conn, 200, REST_MIME_TYPE_JSONP );
                }
                else {
                    websrv_sendheader( conn, 200, REST_MIME_TYPE_JSON );
                }

                if ( pSession->m_pClientItem->m_bOpen && cntAvailable ) {

                    // typeof handler === 'function' &&
                    if ( REST_FORMAT_JSONP == format ) {
                        wxString str = _("typeof handler === 'function' && handler(");
                        web_write( conn, (const char *)str.mbc_str(), str.Length() );
                    }
                    
                    output["success"] = true;
                    output["code"] = 1;
                    output["message"] = "success";
                    output["description"] = "Success";
                    output["info"] = 
                            (const char *)wxString::Format( 
                                 "%zd events requested of %lu available "
                                 "(unfiltered) %zd will be retrieved",
                                 count,
                                 cntAvailable,
                                 MIN( count, cntAvailable ) ).mbc_str();

                    for ( unsigned int i=0; i<MIN( count, cntAvailable ); i++ ) {

                        CLIENTEVENTLIST::compatibility_iterator nodeClient;
                        vscpEvent *pEvent;

                        pSession->m_pClientItem->m_mutexClientInputQueue.Lock();
                        nodeClient = pSession->m_pClientItem->m_clientInputQueue.GetFirst();
                        pEvent = nodeClient->GetData();
                        pSession->m_pClientItem->m_clientInputQueue.DeleteNode(nodeClient);
                        pSession->m_pClientItem->m_mutexClientInputQueue.Unlock();

                        if ( NULL != pEvent ) {

                            if ( vscp_doLevel2Filter( pEvent, 
                                    &pSession->m_pClientItem->m_filterVSCP ) ) {

                                wxString str;
                                json ev;
                                ev["head"] = pEvent->head;
                                ev["vscpclass"] = pEvent->vscp_class;
                                ev["vscptype"] = pEvent->vscp_type;
                                vscp_getDateStringFromEvent( pEvent, str );
                                ev["datetime"] = (const char *)str.mbc_str();
                                ev["timestamp"] = pEvent->timestamp;
                                ev["obid"] = pEvent->obid;
                                vscp_writeGuidToString( pEvent, str);
                                ev["guid"] = (const char *)str.mbc_str();
                                ev["sizedata"] = pEvent->sizeData;
                                ev["data"] = json::array();
                                for ( uint16_t j=0; j<pEvent->sizeData; j++ ) {
                                    ev["data"].push_back( pEvent->pdata[j] );
                                }
                                
                                // Add event to event array
                                output["event"].push_back( ev );

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

                    } // for

                
                    // Mark end
                    output["count"] = sentEvents;
                    output["filtered"] = filtered;
                    output["errors"] = errors;
                    
                    std::string s = output.dump();
                    web_write( conn, s.c_str(), s.length() );
                    
                    if ( REST_FORMAT_JSONP == format ) {
                        web_write( conn, ");", 2 );
                    }

                } // if open and data
                else {   // no events available

                    if ( REST_FORMAT_JSON == format ) {
                        sprintf( wrkbuf, REST_JSON_ERROR_INPUT_QUEUE_EMPTY"\r\n");
                    }
                    else {
                        sprintf( wrkbuf, REST_JSONP_ERROR_INPUT_QUEUE_EMPTY"\r\n");
                    }

                    web_write( conn, wrkbuf, strlen( wrkbuf) );

                }

            }  // format

            web_write( conn, "", 0 );

        }
        else {    // Queue is empty
            restsrv_error( conn, pSession, format, RESR_ERROR_CODE_INPUT_QUEUE_EMPTY );
        }

    }
    else {
            restsrv_error( conn, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
    }

    return;
}


///////////////////////////////////////////////////////////////////////////////
// restsrv_doSetFilter
//

void
restsrv_doSetFilter( struct web_connection *conn,
                            struct restsrv_session *pSession,
                            int format,
                            vscpEventFilter& vscpfilter )
{
    if ( NULL != pSession ) {

        pSession->m_pClientItem->m_mutexClientInputQueue.Lock();
        memcpy( &pSession->m_pClientItem->m_filterVSCP, 
                &vscpfilter, 
                sizeof( vscpEventFilter ) );
        pSession->m_pClientItem->m_mutexClientInputQueue.Unlock();
        restsrv_error( conn, pSession, format, REST_ERROR_CODE_SUCCESS );
    }
    else {
        restsrv_error( conn, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
    }

    return;
}


///////////////////////////////////////////////////////////////////////////////
// restsrv_doClearQueue
//

void
restsrv_doClearQueue( struct web_connection *conn,
                                struct restsrv_session *pSession,
                                int format )
{
    // Check pointer
    if (NULL == conn) return;

    if ( NULL != pSession ) {

        CLIENTEVENTLIST::iterator iterVSCP;

        pSession->m_pClientItem->m_mutexClientInputQueue.Lock();

        for ( iterVSCP = pSession->m_pClientItem->m_clientInputQueue.begin();
                iterVSCP != pSession->m_pClientItem->m_clientInputQueue.end(); 
                ++iterVSCP) {
            vscpEvent *pEvent = *iterVSCP;
            vscp_deleteVSCPevent(pEvent);
        }

        pSession->m_pClientItem->m_clientInputQueue.Clear();
        pSession->m_pClientItem->m_mutexClientInputQueue.Unlock();

        restsrv_error( conn, pSession, format, REST_ERROR_CODE_SUCCESS );
    }
    else {
        restsrv_error( conn, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
    }

    return;
}


///////////////////////////////////////////////////////////////////////////////
// restsrv_doCreateVariable
//

void
restsrv_doCreateVariable( struct web_connection *conn,
                                    struct restsrv_session *pSession,
                                    int format,
                                    wxString& strVariable,
                                    wxString& strType,
                                    wxString& strValue,
                                    wxString& strPersistent,
                                    wxString& strAccessRight,
                                    wxString& strNote )
{
    int type = VSCP_DAEMON_VARIABLE_CODE_STRING;
    bool bPersistence = false;
    uint32_t accessright = 0x700;
    wxStringTokenizer tkz( strVariable, _(";") );

    // Check pointer
    if (NULL == conn) {
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
    
    // Get access rights
    strAccessRight.Trim();
    strAccessRight.Trim( false );
    if ( !strAccessRight.IsEmpty() ) {
        accessright = vscp_readStringValue( strAccessRight );
    }

    if ( NULL != pSession ) {

        // Add the variable
        if ( !gpobj->m_variables.add( strVariable,
                            strValue,
                            type,
                            pSession->m_pClientItem->m_pUserItem->getUserID(),
                            bPersistence,
                            accessright,
                            strNote ) ) {
            restsrv_error( conn,
                            pSession,
                            format,
                            REST_ERROR_CODE_VARIABLE_NOT_CREATED );
            return;
        }

        restsrv_error( conn,
                        pSession,
                        format,
                        REST_ERROR_CODE_SUCCESS );

    }
    else {
        restsrv_error( conn,
                        pSession,
                        format,
                        REST_ERROR_CODE_INVALID_SESSION );
    }

    return;
}

///////////////////////////////////////////////////////////////////////////////
// restsrv_doReadVariable
//

void
restsrv_doReadVariable( struct web_connection *conn,
                                struct restsrv_session *pSession,
                                int format,
                                wxString& strVariableName )
{
    char buf[512];
    char wrkbuf[512];

    // Check pointer
    if (NULL == conn) return;

    if ( NULL != pSession ) {

        CVSCPVariable variable;

        if ( 0 == gpobj->m_variables.find( strVariableName, variable ) ) {
            restsrv_error( conn, 
                            pSession, 
                            format, 
                            REST_ERROR_CODE_VARIABLE_NOT_FOUND );
            return;
        }

        if ( REST_FORMAT_PLAIN == format ) {

                websrv_sendheader( conn, 200, REST_MIME_TYPE_PLAIN );
                sprintf( wrkbuf, "1 1 Success \r\n");
                web_write( conn, wrkbuf, strlen( wrkbuf) );

                sprintf( wrkbuf,
                                "name=%s type=%d user=%lu access-right=%03X "
                                "persistent=%s last-change='%s' value='%s' "
                                "note='%s'\r\n",
                                (const char *)variable.getName().mbc_str(),
                                variable.getType(),
                                (unsigned long)variable.getOwnerID(),
                                variable.getAccessRights(),                                
                                variable.isPersistent() ? "true" : "false",
                                (const char *)variable.getLastChange().FormatISOCombined().mbc_str(),
                                (const char *)variable.getValue().mbc_str(),
                                (const char *)variable.getNote().mbc_str() );

                web_write( conn, wrkbuf, strlen( wrkbuf) );

        }
        else if ( REST_FORMAT_CSV == format ) {

            websrv_sendheader( conn, 200, REST_MIME_TYPE_CSV );
            sprintf( wrkbuf,
                "success-code,error-code,message,description,name,type,user,"
                "access-right,persistent,last-change,value,note\r\n1,1,Success,"
                "Success.,%s,%d,%lu,%03X,%s,%s,'%s','%s'\r\n",
                (const char *)strVariableName.mbc_str(),
                variable.getType(),
                (unsigned long)variable.getOwnerID(),
                variable.getAccessRights(),    
                variable.isPersistent() ? "true" : "false",
                (const char *)variable.getLastChange().FormatISOCombined().mbc_str(),    
                (const char *)variable.getValue().mbc_str(),
                (const char *)variable.getNote().mbc_str() );
            web_write( conn, wrkbuf, strlen( wrkbuf ) );

        }
        else if ( REST_FORMAT_XML == format ) {

            websrv_sendheader( conn, 200, REST_MIME_TYPE_XML );
            sprintf( wrkbuf, 
                    XML_HEADER"<vscp-rest success = \"true\" code "
                    "= \"1\" message = \"Success\" description = \"Success.\" >");
            web_write( conn, wrkbuf, strlen( wrkbuf) );

            sprintf( wrkbuf,
                        "<variable name=\"%s\" typecode=\"%d\" type=\"%s\" "
                        "user=\"%lu\" access-right=\"%03X\" persistent=\"%s\" "
                        "last-change=\"%s\" >",
                        (const char *)variable.getName().mbc_str(),
                        variable.getType(),
                        variable.getVariableTypeAsString( variable.getType() ),
                        (unsigned long)variable.getOwnerID(),
                        variable.getAccessRights(),                                                
                        variable.isPersistent() ? "true" : "false",
                        (const char *)variable.getLastChange().FormatISOCombined().mbc_str() );
            web_write( conn, wrkbuf, strlen( wrkbuf) );
            sprintf((char *) wrkbuf,
                            "<name>%s</name><value>%s</value><note>%s</note>",
                            (const char *)variable.getName().mbc_str(),
                            (const char *)variable.getValue().mbc_str(),
                            (const char *)variable.getNote().mbc_str() );

            web_write( conn, wrkbuf, strlen( wrkbuf) );

            // End tag
            strcpy((char *) wrkbuf, "</variable>" );
            web_write( conn, wrkbuf, strlen( wrkbuf) );

            // End tag
            strcpy((char *) wrkbuf, "</vscp-rest>" );
            web_write( conn, wrkbuf, strlen( wrkbuf));

        }
        else if ( ( REST_FORMAT_JSON == format ) || 
                  ( REST_FORMAT_JSONP == format ) ) {

            wxString wxstr;
            json output;
            
            if ( REST_FORMAT_JSONP == format ) {
                websrv_sendheader( conn, 200, REST_MIME_TYPE_JSONP );
            }
            else {
                websrv_sendheader( conn, 200, REST_MIME_TYPE_JSON );
            }

            if ( pSession->m_pClientItem->m_bOpen  ) {

                if ( REST_FORMAT_JSONP == format ) {
                    // Write JSONP start block
                    web_write( conn, 
                            REST_JSONP_START, 
                            strlen( REST_JSONP_START ) );
                }
                
                output["success"] = true;
                output["code"] = 1;
                output["message"] = "success";
                output["description"] = "Success";
                output["varname"] = (const char *)variable.getName().mbc_str();
                output["vartype"] = variable.getVariableTypeAsString( variable.getType() );
                output["vartypecode"] = variable.getType();
                output["varuser"] = variable.getOwnerID();
                output["varaccessright"] = variable.getAccessRights();
                output["varpersistence"] = variable.isPersistent();
                output["varlastchange"] = 
                        (const char *)variable.getLastChange().FormatISOCombined().mbc_str();
                output["varvalue"] = (const char *)variable.getValue().mbc_str();
                output["varnote"] = (const char *)variable.getNote().mbc_str();
                std::string s = output.dump();
                web_write( conn, s.c_str(), s.length() );
                
                if ( REST_FORMAT_JSONP == format ) {
                    // Write JSONP end block
                    web_write( conn, 
                            REST_JSONP_END, 
                            strlen( REST_JSONP_END ) );
                }
                
            }
            
        }

    }
    else {
        restsrv_error( conn, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
    }

    return;
}



///////////////////////////////////////////////////////////////////////////////
// restsrv_doListVariable
//

void
restsrv_doListVariable( struct web_connection *conn,
                                struct restsrv_session *pSession,
                                int format,
                                wxString& strRegEx,
                                bool bShort )
{
    char wrkbuf[8192];

    // Check pointer
    if (NULL == conn) return;

    strRegEx.Trim();

    if ( NULL != pSession ) {
        
        wxArrayString variable_array;
        if ( !gpobj->m_variables.getVarlistFromRegExp( variable_array, strRegEx ) ) {
            restsrv_error( conn, pSession, format, REST_ERROR_CODE_VARIABLE_NOT_FOUND );
            return; 
        }
       
        if ( REST_FORMAT_PLAIN == format ) {

                websrv_sendheader( conn, 200, REST_MIME_TYPE_PLAIN );
                sprintf( wrkbuf, "1 1 Success \r\n");
                web_write( conn, wrkbuf, strlen( wrkbuf) );
                
                for ( unsigned int i=0; i<variable_array.GetCount(); i++ ) {
                    
                    CVSCPVariable variable;

                    if ( 0 == gpobj->m_variables.find( variable_array[ i ], variable ) ) {
                        continue;
                    }

                    if ( bShort ) {
                        sprintf( wrkbuf,
                                    "name=%s type=%d user=%lu access-right=%03X last-change='%s' persistent=%s\r\n",
                                    (const char *)variable.getName().mbc_str(),
                                    variable.getType(),
                                    (unsigned long)variable.getOwnerID(),
                                    variable.getAccessRights(),
                                    (const char *)variable.getLastChange().FormatISOCombined().mbc_str(),
                                    variable.isPersistent() ? "true" : "false" );
                    }
                    else {
                        sprintf( wrkbuf,
                                    "name=%s type=%d user=%lu access-right=%03X last-change='%s' persistent=%s value=%s note=%s\r\n",
                                    (const char *)variable.getName().mbc_str(),
                                    variable.getType(),
                                    (unsigned long)variable.getOwnerID(),
                                    variable.getAccessRights(),
                                    (const char *)variable.getLastChange().FormatISOCombined().mbc_str(),
                                    variable.isPersistent() ? "true" : "false",
                                    (const char *)variable.getValue( true ).mbc_str(),
                                    (const char *)variable.getNote( true ).mbc_str() );                        
                    }
                    
                    web_write( conn, wrkbuf, strlen( wrkbuf) );
                    
                }

        }
        else if ( REST_FORMAT_CSV == format ) {

            websrv_sendheader( conn, 200, REST_MIME_TYPE_CSV );
                        
            if ( pSession->m_pClientItem->m_bOpen && variable_array.GetCount() ) {

                    sprintf( wrkbuf, "success-code,error-code,message,description,Variable\r\n1,1,Success,Success.,NULL\r\n");
                    web_write( conn, wrkbuf, strlen( wrkbuf) );
                    
                    sprintf( wrkbuf,
                             "1,2,Info,%zd variables found,NULL\r\n",
                                variable_array.GetCount() );
                    web_write( conn, wrkbuf, strlen( wrkbuf ) );
                    
                    sprintf( wrkbuf,
                             "1,5,Count,%zu,NULL\r\n",
                             variable_array.GetCount() );
                    web_write( conn, wrkbuf, strlen( wrkbuf ) );

                    for ( unsigned int i=0; i<variable_array.GetCount(); i++ ) {
                    
                        CVSCPVariable variable;

                        if ( 0 == gpobj->m_variables.find( variable_array[ i ], variable ) ) {
                            continue;
                        }

                        if ( bShort ) {
                            sprintf( wrkbuf,
                                        "1,3,Data,Variable,%s\r\n",
                                        (const char *)variable.getAsString( true ).mbc_str() );
                        }
                        else {
                            sprintf( wrkbuf,
                                        "1,3,Data,Variable,%s\r\n",
                                        (const char *)variable.getAsString( false ).mbc_str() );
                        }
                        
                        web_write( conn, wrkbuf, strlen( wrkbuf ) );
                        
                    } // for
                }
                else {   // no events available
                    sprintf( wrkbuf, REST_CSV_ERROR_INPUT_QUEUE_EMPTY"\r\n");
                    web_write( conn, wrkbuf, strlen( wrkbuf) );
                }

        }
        else if ( REST_FORMAT_XML == format ) {

            websrv_sendheader( conn, 200, REST_MIME_TYPE_XML );
            sprintf( wrkbuf, XML_HEADER"<vscp-rest success = \"true\" code = \"1\" message = \"Success\" description = \"Success.\" >");
            web_write( conn, wrkbuf, strlen( wrkbuf) );
            
            sprintf( wrkbuf, "<count>%zu</count>",variable_array.GetCount() );

            for ( unsigned int i=0; i<variable_array.GetCount(); i++ ) {
                
                CVSCPVariable variable;
                
                if ( 0 == gpobj->m_variables.find( variable_array[ i ], variable ) ) {
                    continue;
                }
                
                sprintf( wrkbuf,
                        "<variable name=\"%s\" typecode=\"%d\" type=\"%s\" user=\"%lu\" access-right=\"%03X\" persistent=\"%s\" last-change=\"%s\" >",
                        (const char *)variable.getName().mbc_str(),
                        variable.getType(),
                        variable.getVariableTypeAsString( variable.getType() ),
                        (unsigned long)variable.getOwnerID(),
                        variable.getAccessRights(),                                                
                        variable.isPersistent() ? "true" : "false",
                        (const char *)variable.getLastChange().FormatISOCombined().mbc_str() );
                web_write( conn, wrkbuf, strlen( wrkbuf) );
                
                if ( !bShort ) {
                    sprintf((char *) wrkbuf,
                                "<name>%s</name><value>%s</value><note>%s</note>",
                                (const char *)variable.getName().mbc_str(),
                                (const char *)variable.getValue(true).mbc_str(),
                                (const char *)variable.getNote(true).mbc_str() );

                    web_write( conn, wrkbuf, strlen( wrkbuf) );
                }
            }

            // End tag
            strcpy((char *) wrkbuf, "</variable>" );
            web_write( conn, wrkbuf, strlen( wrkbuf) );

            // End tag
            strcpy((char *) wrkbuf, "</vscp-rest>" );
            web_write( conn, wrkbuf, strlen( wrkbuf));

        }
        else if ( ( REST_FORMAT_JSON == format ) || 
                  ( REST_FORMAT_JSONP == format ) ) {

            json output;
            wxString wxstr;
            
            if ( REST_FORMAT_JSONP == format ) {
                websrv_sendheader( conn, 200, REST_MIME_TYPE_JSONP );
            }
            else {
                websrv_sendheader( conn, 200, REST_MIME_TYPE_JSON );
            }

            if ( pSession->m_pClientItem->m_bOpen  ) {
                
                if ( REST_FORMAT_JSONP == format ) {
                    // Write JSONP start block
                    web_write( conn, 
                            REST_JSONP_START, 
                            strlen( REST_JSONP_START ) );
                }
                
                output["success"] = true;
                output["code"] = 1;
                output["message"] = "success";
                output["description"] = "Success";
                output["info"] = 
                        (const char *)wxString::Format( _("\"%zd variables will be retrieved\""),
                                                        variable_array.Count() );
                output["variable"] = json::array();
                   
                uint32_t cntVariable = 0;
                uint32_t cntErrors = 0;
                for ( unsigned int i=0; i<variable_array.GetCount(); i++ ) {
                    
                    CVSCPVariable variable; 
                    json var;
                    
                    if ( 0 == gpobj->m_variables.find( variable_array[ i ], variable ) ) {
                        cntErrors++;
                        continue;
                    }
                    
                    var["success"] = true;
                    var["code"] = 1;
                    var["message"] = "success";
                    var["description"] = "Success";
                    var["varname"] = (const char *)variable.getName().mbc_str();
                    var["vartype"] = variable.getVariableTypeAsString( variable.getType() );
                    var["vartypecode"] = variable.getType();
                    var["varuser"] = variable.getOwnerID();
                    var["varaccessright"] = variable.getAccessRights();
                    var["varpersistence"] = variable.isPersistent();
                    var["varlastchange"] = 
                            (const char *)variable.getLastChange().FormatISOCombined().mbc_str();
                    if ( !bShort ) {
                        var["varvalue"] = (const char *)variable.getValue(true).mbc_str();
                        var["varnote"] = (const char *)variable.getNote(true).mbc_str();
                    }
                    
                    // Add event to event array
                    output["variable"].push_back( var );

                    cntVariable++;
                    
                } // for                                    

                // Mark end
                output["count"] = cntVariable;
                output["errors"] = cntErrors;
                    
                std::string s = output.dump();
                web_write( conn, s.c_str(), s.length() );
                    
                if ( REST_FORMAT_JSONP == format ) {
                    // Write JSONP end block
                    web_write( conn, 
                            REST_JSONP_END, 
                            strlen( REST_JSONP_END ) );
                }
                
            }
            
        }

    }
    else {
        restsrv_error( conn, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
    }

    return;
}



///////////////////////////////////////////////////////////////////////////////
// restsrv_doWriteVariable
//

void
restsrv_doWriteVariable( struct web_connection *conn,
                                struct restsrv_session *pSession,
                                int format,
                                wxString& strVariableName,
                                wxString& strValue )
{
    wxString strName;

    // Check pointer
    if (NULL == conn) {
        return;
    }

    if ( NULL != pSession ) {

        // Get variable name
        CVSCPVariable variable;
        if ( 0 == gpobj->m_variables.find( strVariableName, variable ) ) {
            restsrv_error( conn, pSession, format, REST_ERROR_CODE_VARIABLE_NOT_FOUND );
            return;
        }

        // Set variable value
        if ( !variable.setValueFromString( variable.getType(), strValue ) ) {
            restsrv_error( conn, pSession, format, REST_ERROR_CODE_MISSING_DATA );
            return;
        }
        
        if ( !gpobj->m_variables.update( variable ) ) {
            restsrv_error( conn, pSession, format, REST_ERROR_CODE_VARIABLE_FAIL_UPDATE );
            return;
        }

        restsrv_error( conn, pSession, format, REST_ERROR_CODE_SUCCESS );

    }
    else {
        restsrv_error( conn, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
    }

    return;
}


///////////////////////////////////////////////////////////////////////////////
// restsrv_doDeleteVariable
//

void
restsrv_doDeleteVariable( struct web_connection *conn,
                                    struct restsrv_session *pSession,
                                    int format,
                                    wxString& strVariable )
{
    int type = VSCP_DAEMON_VARIABLE_CODE_STRING;
    bool bPersistence = false;
    wxStringTokenizer tkz( strVariable, _(";") );

    // Check pointer
    if (NULL == conn) {
        return;
    }

    if ( NULL != pSession ) {

        // Add the variable
        if ( !gpobj->m_variables.remove( strVariable ) ) {
            restsrv_error( conn,
                                    pSession,
                                    format,
                                    REST_ERROR_CODE_VARIABLE_NOT_DELETED );
            return;
        }

        restsrv_error( conn,
                                pSession,
                                format,
                                REST_ERROR_CODE_SUCCESS );

    }
    else {
        restsrv_error( conn,
                                pSession,
                                format,
                                REST_ERROR_CODE_INVALID_SESSION );
    }

    return;
}

///////////////////////////////////////////////////////////////////////////////
// restsrv_doWriteMeasurement
//

void
restsrv_doWriteMeasurement( struct web_connection *conn,
                                    struct restsrv_session *pSession,
                                    int format,
                                    wxString& strDateTime,
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
        strValue.ToDouble( &value );            // Measurement value
        
        strUnit.ToLong( &unit );                // Measurement unit
        
        strSensorIdx.ToLong( &sensoridx );      // Sensor index
        
        strType.ToLong( &vscptype );            // VSCP event type
        
        strZone.ToLong( &zone );                // VSCP event type
        zone &= 0xff;
        
        strSubZone.ToLong( &subzone );          // VSCP event type
        subzone &= 0xff;
        
        // datetime
        wxDateTime dt;
        if ( !dt.ParseISOCombined( strDateTime ) ) {
            dt = wxDateTime::UNow();
        }

        strLevel.ToLong( &level );              // Level I or Level II (default)
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
            if ( sensoridx > 7 ) sensoridx = 0;
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
                        restsrv_error( conn, pSession, format, REST_ERROR_CODE_GENERAL_FAILURE );
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

                    restsrv_doSendEvent( conn, pSession, format, pEvent );

                }
                else {
                    restsrv_error( conn, 
                                    pSession, 
                                    format, 
                                    REST_ERROR_CODE_GENERAL_FAILURE );
                }
            }
            else {
                
                // String
                vscpEvent *pEvent = new vscpEvent;
                if ( NULL == pEvent ) {
                    restsrv_error( conn, 
                                    pSession, 
                                    format, 
                                    REST_ERROR_CODE_GENERAL_FAILURE );
                    return;
                }
                pEvent->pdata = NULL;

                if ( vscp_makeStringMeasurementEvent( pEvent,
                                                        value,
                                                        unit,
                                                        sensoridx ) ) {

                }
                else {
                    restsrv_error( conn, 
                                    pSession, 
                                    format, 
                                    REST_ERROR_CODE_GENERAL_FAILURE );
                }
            }
        }
        else {  // Level II
            
            if ( 0 == eventFormat ) {

                // Floating point
                vscpEvent *pEvent = new vscpEvent;
                if ( NULL == pEvent ) {
                    restsrv_error( conn, 
                                    pSession, 
                                    format, 
                                    REST_ERROR_CODE_GENERAL_FAILURE );
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
                uint64_t temp = wxUINT64_SWAP_ON_LE( *(data + 4) );
                memcpy( data+4, (void *)&temp, 8 );

                // Copy in data
                pEvent->pdata = new uint8_t[ 4 + 8 ];
                if ( NULL == pEvent->pdata ) {
                    restsrv_error( conn, 
                                    pSession, 
                                    format, 
                                    REST_ERROR_CODE_GENERAL_FAILURE );
                    delete pEvent;
                    return;
                }
                memcpy( pEvent->pdata, data, 4 + 8 );

                restsrv_doSendEvent( conn, pSession, format, pEvent );
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
                    restsrv_error( conn, 
                                    pSession, 
                                    format, 
                                    REST_ERROR_CODE_GENERAL_FAILURE );
                    delete pEvent;
                    return;
                }
                memcpy( data + 4, 
                            strValue.mbc_str(), 
                            strValue.Length() ); // copy in double

                restsrv_doSendEvent( conn, pSession, format, pEvent );
            }
        }

        restsrv_error( conn, pSession, format, REST_ERROR_CODE_SUCCESS );
    }
    else {
        restsrv_error( conn, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
    }

    return;
}


///////////////////////////////////////////////////////////////////////////////
// restsrv_doGetTableData
//

void
websrc_renderTableData( struct web_connection *conn,
                                struct restsrv_session *pSession,
                                int format,
                                wxString& strName,
                                struct _vscpFileRecord *pRecords,
                                long nfetchedRecords )
{
/*    
    char buf[ 2048 ];
    char wrkbuf[ 2048 ];
    long nRecords = 0;

    if ( REST_FORMAT_PLAIN == format ) {

        // Send header
        websrv_sendheader( conn, 200, REST_MIME_TYPE_PLAIN );
        sprintf( wrkbuf,
                 "1 1 Success\r\n%ld records will be returned from table %s.\r\n",
                 nfetchedRecords,
                 ( const char * )strName.mbc_str() );
        web_printf( conn, wrkbuf, strlen( wrkbuf ) );

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
            web_printf( conn, wrkbuf, strlen( wrkbuf ));

        }

    }
    else if ( REST_FORMAT_CSV == format ) {

        // Send header
        websrv_sendheader( conn, 200, REST_MIME_TYPE_CSV );
        sprintf( wrkbuf,
                 "success-code, error-code, message, description, EvenL\r\n1, 1, Success, Success., NULL\r\n" );
        web_printf( conn, wrkbuf, strlen( wrkbuf ) );

        sprintf( wrkbuf,
                 "1, 2, Info, Success %ld records will be returned from table %s.,NULL\r\n",
                 nfetchedRecords,
                 ( const char * )strName.mbc_str() );
        web_printf( conn, wrkbuf, strlen( wrkbuf ) );

        sprintf( wrkbuf,
                 "1, 4, Count, %ld, NULL\r\n",
                 nfetchedRecords );
        web_printf( conn, wrkbuf, strlen( wrkbuf ) );

        for ( long i = 0; i < nfetchedRecords; i++ ) {

            wxDateTime dt;
            dt.Set( ( time_t )pRecords[ i ].timestamp );
            wxString strDateTime = dt.FormatISODate() + _( " " ) + dt.FormatISOTime();

            sprintf( wrkbuf,
                     "1,3,Data,Table,%ld - Date=%s,Value=%f\r\n",
                     i,
                     ( const char * )strDateTime.mbc_str(),
                     pRecords->measurement );
            web_printf( conn, wrkbuf, strlen( wrkbuf ) );

        }

    }
    else if ( REST_FORMAT_XML == format ) {

        // Send header
        restsrv_sendHeader( conn, 200, REST_MIME_TYPE_XML );
        sprintf( wrkbuf,
                 "<vscp-rest success=\"true\" code=\"1\" message=\"Success\" description=\"Success.\">\r\n" );
        web_printf( conn, wrkbuf, strlen( wrkbuf ) );

        sprintf( wrkbuf,
                 "<count>%ld</count>\r\n<info>Fetched %ld elements of table %s</info>\r\n",
                 nfetchedRecords,
                 nfetchedRecords,
                 ( const char * )strName.mbc_str() );
        web_printf( conn, wrkbuf, strlen( wrkbuf )  );

        for ( long i = 0; i < nfetchedRecords; i++ ) {

            wxDateTime dt;
            dt.Set( ( time_t )pRecords[ i ].timestamp );
            wxString strDateTime = dt.FormatISODate() + _( " " ) + dt.FormatISOTime();

            sprintf( wrkbuf,
                     "<data id=\"%ld\" date=\"%s\" value=\"%f\"></data>\r\n",
                     i,
                     ( const char * )strDateTime.mbc_str(),
                     pRecords->measurement );
            web_printf( conn, wrkbuf, strlen( wrkbuf ) );

        }

        sprintf( wrkbuf,
                 "</vscp-rest>\r\n" );
        web_printf( conn, wrkbuf, strlen( wrkbuf ) );

    }
    else if ( ( REST_FORMAT_JSON == format ) || ( REST_FORMAT_JSONP == format ) ) {

        if ( REST_FORMAT_JSON == format ) {
            // Send header
            restsrv_sendHeader( conn, 200, REST_MIME_TYPE_JSON );
        }
        else {                                    // Send header
            restsrv_sendHeader( conn, 200, REST_MIME_TYPE_JSONP );
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
        web_printf( conn, wrkbuf, strlen( wrkbuf ) );
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

            web_printf( conn, wrkbuf, strlen( wrkbuf )  );

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

        web_printf( conn, wrkbuf, strlen( wrkbuf ) );

    }

    web_printf( conn, "", 0 );    // Terminator
*/
}


///////////////////////////////////////////////////////////////////////////////
// restsrv_rest_doGetTableData
//

void
restsrv_doGetTableData( struct web_connection *conn,
                                struct restsrv_session *pSession,
                                int format,
                                wxString& strName,
                                wxString& strFrom,
                                wxString& strTo )
{
/*   
    //char buf[ 2048 ];
    //char wrkbuf[ 2048 ];
    long nRecords = 0;

    // Check pointer
    if ( NULL == conn ) return;

    if ( NULL != pSession ) {

        // We need 'tablename' and optionally 'from' and 'to'

        CVSCPTable *ptblItem = NULL;
        gpobj->m_mutexTableList.Lock();
        
        listVSCPTables::iterator iter;

        for ( iter = gpobj->m_listTables.begin(); iter != gpobj->m_listTables.end(); ++iter ) {
            ptblItem = *iter;
            if ( 0 == strcmp( ptblItem->m_vscpFileHead.nameTable, ( const char * )strName.mbc_str() ) ) {
                break;
            }
            ptblItem = NULL;
        }

        gpobj->m_mutexTableList.Unlock();

        // If found pTable should not be NULL
        if ( NULL == ptblItem ) {
            restsrv_rest_error( nc, pSession, format, REST_ERROR_CODE_TABLE_NOT_FOUND );
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
            restsrv_rest_error( nc, pSession, format, REST_ERROR_CODE_TABLE_RANGE );
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
                restsrv_rest_error( nc, pSession, format, REST_ERROR_CODE_TABLE_NO_DATA );
                return;
            }

            struct _vscpFileRecord *pRecords = new struct _vscpFileRecord[ nRecords ];

            if ( NULL == pRecords ) {
                restsrv_rest_error( nc, pSession, format, REST_ERROR_CODE_GENERAL_FAILURE );
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
                restsrv_rest_error( nc, pSession, format, REST_ERROR_CODE_TABLE_NO_DATA );
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

            if ( REST_FORMAT_PLAIN == format ) {

                // Send header
                restsrv_sendHeader( conn, 200, REST_MIME_TYPE_PLAIN );

                sprintf( wrkbuf,
                         "1 1 Success\r\n%ld records will be returned from table %s.\r\n",
                         nfetchedRecords,
                         (const char *)strName.mbc_str() );
                web_printf( conn, wrkbuf, strlen( wrkbuf ) );

                for ( long i = 0; i < nfetchedRecords; i++ ) {

                    wxDateTime dt;
                    dt.Set( ( time_t )pRecords[ i ].timestamp );
                    wxString strDateTime = dt.FormatISODate() + _( " " ) + dt.FormatISOTime();

                    sprintf( wrkbuf,
                             "%ld - Date=%s,Value=%f\r\n",
                             i,
                             (const char *)strDateTime.mbc_str(),
                             pRecords->measurement );
                    web_printf( conn, wrkbuf, strlen( wrkbuf ) );

                }

            }
            else if ( REST_FORMAT_CSV == format ) {

                // Send header
                restsrv_sendHeader( conn, 200, REST_MIME_TYPE_CSV );

                sprintf( wrkbuf,
                         "success-code, error-code, message, description, EvenL\r\n1, 1, Success, Success., NULL\r\n" );
                web_printf( conn, wrkbuf, strlen( wrkbuf ) );

                sprintf( wrkbuf,
                         "1, 2, Info, Success %d records will be returned from table %s.,NULL\r\n",
                         nfetchedRecords,
                         ( const char * )strName.mbc_str() );
                web_printf( conn, wrkbuf, strlen( wrkbuf )  );

                sprintf( wrkbuf,
                         "1, 4, Count, %d, NULL\r\n",
                         nfetchedRecords );
                web_printf( conn, wrkbuf, strlen( wrkbuf ) );

                for ( long i = 0; i < nfetchedRecords; i++ ) {

                    wxDateTime dt;
                    dt.Set( ( time_t )pRecords[ i ].timestamp );
                    wxString strDateTime = dt.FormatISODate() + _( " " ) + dt.FormatISOTime();

                    sprintf( wrkbuf,
                             "1,3,Data,Table,%d - Date=%s,Value=%f\r\n",
                             i,
                             ( const char * )strDateTime.mbc_str(),
                             pRecords->measurement );
                    web_printf( conn, wrkbuf, strlen( wrkbuf ) );

                }

            }
            else if ( REST_FORMAT_XML == format ) {

                // Send header
                restsrv_sendHeader( conn, 200, REST_MIME_TYPE_XML );

                sprintf( wrkbuf,
                         "<vscp-rest success=\"true\" code=\"1\" message=\"Success\" description=\"Success.\">\r\n" );
                web_printf( conn, wrkbuf, strlen( wrkbuf ) );

                sprintf( wrkbuf,
                            "<count>%d</count>\r\n<info>Fetched %d elements of table %s</info>\r\n",
                            nfetchedRecords,
                            nfetchedRecords,
                            ( const char * )strName.mbc_str() );
                web_printf( conn, wrkbuf, strlen( wrkbuf ) );

                for ( long i = 0; i < nfetchedRecords; i++ ) {

                    wxDateTime dt;
                    dt.Set( ( time_t )pRecords[ i ].timestamp );
                    wxString strDateTime = dt.FormatISODate() + _( " " ) + dt.FormatISOTime();

                    sprintf( wrkbuf,
                             "<data id=\"%d\" date=\"%s\" value=\"%f\"></data>\r\n",
                             i,
                             ( const char * )strDateTime.mbc_str(),
                             pRecords->measurement );
                    web_printf( conn, wrkbuf, strlen( wrkbuf ) );

                }

                sprintf( wrkbuf,
                         "</vscp-rest>\r\n" );
                web_printf( conn, wrkbuf, strlen( wrkbuf ));

            }
            else if ( ( REST_FORMAT_JSON == format ) || ( REST_FORMAT_JSONP == format ) ) {

                if ( REST_FORMAT_JSON == format ) {
                    // Send header
                    restsrv_sendHeader( conn, 200, REST_MIME_TYPE_JSON );
                }
                else {                                    // Send header
                    restsrv_sendHeader( conn, 200, REST_MIME_TYPE_JSONP );
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

                web_printf( conn, wrkbuf, strlen( wrkbuf ) );
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

                    web_printf( conn, wrkbuf, strlen( wrkbuf ) );

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

                web_printf( conn, wrkbuf, strlen( wrkbuf )  );

            }

            web_printf( conn, "", 0 );    // Terminator

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
                    restsrv_rest_error( nc, pSession, format, REST_ERROR_CODE_GENERAL_FAILURE );
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

                    // First send start post with number if records
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
                                         nfetchedRecords );

                    // Deallocate storage
                    delete[] pRecords;

                    web_printf( conn, "", 0 );    // Terminator

                }

            }
            // No records
            else {
                restsrv_rest_error( nc, pSession, format, REST_ERROR_CODE_TABLE_NO_DATA );
                return;
            }



        }

*/
    return;
}


///////////////////////////////////////////////////////////////////////////////
// restsrv_convertXML2JSON
//
// XML to JSON convert
//

void restsrv_convertXML2JSON( const char *input, const char *output  )
{
    ifstream is( input );
    ostringstream oss;
    oss << is.rdbuf();

    // TODO
    string json_str; // = xml2json( oss.str().data() );

    ofstream myfile;
    myfile.open( output );
    myfile << json_str << endl;
    myfile.close();
}

///////////////////////////////////////////////////////////////////////////////
// restsrv_doFetchMDF
//

void 
restsrv_doFetchMDF( struct web_connection *conn,
                                struct restsrv_session *pSession,
                                int format,
                                wxString& strURL )
{
    CMDF mdf;

    if ( mdf.load( strURL, false, true ) )  {

        // Loaded OK

        if ( REST_FORMAT_PLAIN == format ) {
            restsrv_error( conn, pSession, format, REST_ERROR_CODE_GENERAL_FAILURE );
        }
        else if ( REST_FORMAT_CSV == format ) {
            restsrv_error( conn, pSession, format, REST_ERROR_CODE_GENERAL_FAILURE );
        }
        else if ( REST_FORMAT_XML == format ) {

            // Send header
            websrv_sendheader( conn, 200, REST_MIME_TYPE_XML );

            char buf[ 5000 ], wrkbuf[ 2000 ];
            ssize_t ss;
            wxFile file( mdf.getTempFilePath() );

            while ( !file.Eof() ) {
                ss = file.Read( wrkbuf, sizeof( wrkbuf ) );
                web_write( conn, wrkbuf, ss );
            }

            file.Close();

            web_write( conn, "", 0 );    // Terminator
        }
        else if ( ( REST_FORMAT_JSON == format ) || ( REST_FORMAT_JSONP == format ) ) {

            char buf[ 5000 ], wrkbuf[ 2000 ];
            wxString tempFileName = wxFileName::CreateTempFileName( _("__vscp__xml__") );
            if ( 0 == tempFileName.Length() ) {
                restsrv_error( conn, pSession, format, REST_ERROR_CODE_GENERAL_FAILURE );
            }

            std::string tempfile_out = std::string( tempFileName.mb_str() );
            std::string tempfile_mdf = std::string( mdf.getTempFilePath().mb_str() );

            // Convert to JSON
            restsrv_convertXML2JSON( (const char *)tempFileName.mbc_str(), 
                                (const char *)mdf.getTempFilePath().mbc_str() );

            // Send header
            if ( REST_FORMAT_JSON == format ) {
                websrv_sendheader( conn, 200, REST_MIME_TYPE_JSON );
            }
            else {
                websrv_sendheader( conn, 200, REST_MIME_TYPE_JSONP );
            }

            if ( REST_FORMAT_JSONP == format ) {
                web_write( conn, "typeof handler === 'function' && handler(", 41 );
            }

            ssize_t ss;
            wxString wxpath( tempfile_out.c_str(), wxConvUTF8 ); // Needed for 2.8.12
            wxFile file( wxpath );

            while ( !file.Eof() ) {
                ss = file.Read( wrkbuf, sizeof( wrkbuf ) );
                web_write( conn, wrkbuf, ss );
            }

            file.Close();

            if ( REST_FORMAT_JSONP == format ) {
                web_write( conn, ");", 2 );
            }

            web_write( conn, "", 0 );    // Terminator

        }

    }
    else {
        // Failed to load
        restsrv_error( conn, 
                        pSession, 
                        format, 
                        REST_ERROR_CODE_GENERAL_FAILURE );
    }

    return;
}
