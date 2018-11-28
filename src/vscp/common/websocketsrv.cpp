// websocketserver.cpp
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

#include <httpd.h>

#include <vscp.h>
#include <vscphelper.h>
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
#include <websrv.h>
#include <websocket.h>


#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

///////////////////////////////////////////////////
//                 GLOBALS
///////////////////////////////////////////////////

extern CControlObject *gpobj;

// Webserver
extern struct mg_mgr gmgr;

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

////////////////////////////////////////////////////
//            Forward declarations
////////////////////////////////////////////////////

void 
ws1_command( struct web_connection *conn,              
                    struct websock_session *pSession,
                    wxString& strCmd );

bool
ws1_message( struct web_connection *conn,
                websock_session *pSession,
                wxString &strWsPkt );

///////////////////////////////////////////////////
//                 WEBSOCKETS
///////////////////////////////////////////////////


// WEBSOCKETSESSIONLIST is declared in control object


// Linked list of websocket sessions
// Protected by the websocketSexxionMutex
//static struct websock_session *gp_websock_sessions;


websock_session::websock_session( void )
{
    m_conn = NULL;
    m_conn_state = WEBSOCK_CONN_STATE_NULL;
    memset( m_websocket_key, 0, 33 );
    memset( m_sid, 0, 33 );
    m_version = 0;
    lastActiveTime = 0;
    m_pClientItem = NULL;
    bEventTrigger = false;
    triggerTimeout = 0;
    bVariableTrigger = false;
};

websock_session::~websock_session( void )
{
    
};

///////////////////////////////////////////////////////////////////////////////
// websock_authentication
//
// client sends
//      "AUTH;iv;AES128("username:password)
//

bool
websock_authentication( struct web_connection *conn,              
                            struct websock_session *pSession,
                            wxString& strIV,
                            wxString& strCrypto )
{
    uint8_t buf[2048], secret[2048];
    uint8_t iv[16];
    wxString strUser, strPassword;
    
    struct web_context * ctx;
    const struct web_request_info *reqinfo;
    bool bValidHost = false;
    
    // Check pointers
    if ( ( NULL == conn ) || 
         ( NULL == pSession ) ||    
         !( ctx = web_get_context( conn ) ) ||
         !( reqinfo = web_get_request_info( conn ) )  ) {
        gpobj->logMsg ( _("[Websocket Client] Authentication: Invalid "
                          "pointers. "), 
                            DAEMON_LOGMSG_NORMAL, 
                            DAEMON_LOGTYPE_SECURITY );
        return false;
    }
    
    if ( -1 == vscp_hexStr2ByteArray( iv, 
                                        16, 
                                        (const char *)strIV.mbc_str() ) ) {
        gpobj->logMsg ( _("[Websocket Client] Authentication: No room "
                          "for iv block. "), 
                            DAEMON_LOGMSG_NORMAL, 
                            DAEMON_LOGTYPE_SECURITY );
        return false;   // Not enough room in buffer
    }
    
    size_t len;
    if ( -1 == ( len = vscp_hexStr2ByteArray( secret, 
                                        strCrypto.Length(), 
                                        (const char *)strCrypto.mbc_str() ) ) ) {
        gpobj->logMsg ( _("[Websocket Client] Authentication: No room "
                          "for crypto block. "), 
                            DAEMON_LOGMSG_NORMAL, 
                            DAEMON_LOGTYPE_SECURITY );
        return false;   // Not enough room in buffer
    }
    
    memset( buf, 0, sizeof( buf ) );
    AES_CBC_decrypt_buffer( AES128,
                                buf,
                                secret,
                                len,
                                gpobj->m_systemKey,
                                iv );

    wxString str = wxString::FromUTF8( (const char *)buf );
    wxStringTokenizer tkz( str, _(":"), wxTOKEN_RET_EMPTY_ALL );

    // Get username
    if (!tkz.HasMoreTokens()) {
        gpobj->logMsg ( _("[Websocket Client] Authentication: Missing "
                          "username from client. "), 
                            DAEMON_LOGMSG_NORMAL, 
                            DAEMON_LOGTYPE_SECURITY );
        return false;   // No username
    }
     
    strUser = tkz.GetNextToken();
    strUser.Trim();
    
    // Get password
    if (!tkz.HasMoreTokens()) {
        gpobj->logMsg ( _("[Websocket Client] Authentication: Missing "
                          "password from client. "), 
                            DAEMON_LOGMSG_NORMAL, 
                            DAEMON_LOGTYPE_SECURITY );
        return false;   // No username
    }
     
    strPassword = tkz.GetNextToken();
    strPassword.Trim();
   
    // Check if user is valid
    CUserItem *pUserItem = gpobj->m_userList.getUser( strUser );
    if ( NULL == pUserItem ) {
        gpobj->logMsg ( _("[Websocket Client] Authentication: CUserItem "
                          "allocation problem "), 
                            DAEMON_LOGMSG_NORMAL, 
                            DAEMON_LOGTYPE_SECURITY );
        return false;
    }

    // Check if remote ip is valid
    bValidHost = 
        pUserItem->isAllowedToConnect( inet_addr( reqinfo->remote_addr ) );

    if ( !bValidHost ) {
        // Log valid login
        wxString strErr =
        wxString::Format( _("[Websocket Client] Authentication: Host "
                                "[%s] NOT allowed to connect.\n"),
                                reqinfo->remote_addr );

        gpobj->logMsg ( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_SECURITY );
        return false;
    }
        
    if ( !vscp_isPasswordValid( pUserItem->getPassword(), strPassword  ) ) {
        
        wxString strErr =
            wxString::Format( _("[Websocket Client] Authentication: User %s at host "
                                "[%s] gave wrong password.\n"),
                        (const char *)strUser.mbc_str(),        
                        reqinfo->remote_addr );
        gpobj->logMsg ( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_SECURITY );
        return false;
    }

    pSession->m_pClientItem->bAuthenticated = true;

    // Add user to client
    pSession->m_pClientItem->m_pUserItem = pUserItem;

    // Copy in the user filter
    memcpy( &pSession->m_pClientItem->m_filterVSCP,
                pUserItem->getFilter(),
                sizeof( vscpEventFilter ) );

    // Log valid login
    wxString strErr =
        wxString::Format( _("[Websocket Client] Authentication: Host [%s] "
                            "User [%s] allowed to connect.\n"),
                            reqinfo->remote_addr,
                            (const char *)strUser.mbc_str() );

    gpobj->logMsg( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_SECURITY );

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// websock_new_session
//

websock_session *
websock_new_session( const struct web_connection *conn )
{
    const char *pHeader;
    char ws_version[10];
    char ws_key[33];
    websock_session *pSession = NULL;

    // Check pointer
    if ( NULL == conn ) return NULL;

    // user 
    memset( ws_version, 0, sizeof( ws_version ) );
    if ( NULL != ( pHeader = web_get_header( conn, "Sec-WebSocket-Version" ) ) ) {
        strncpy( ws_version, 
                    pHeader, 
                    MIN( strlen( pHeader ) + 1, sizeof( ws_version ) ) );
    }
    memset( ws_key, 0, sizeof( ws_key ) );
    if ( NULL != ( pHeader = web_get_header( conn, "Sec-WebSocket-Key" ) ) ) {
        strncpy( ws_key, 
                    pHeader, 
                    MIN( strlen( pHeader ) + 1, sizeof( ws_key ) ) );
    }

    // create fresh session
    pSession = new websock_session;
    if  (NULL == pSession ) {
        gpobj->logMsg(_("[Websockets] New session: Unable to create session object."));
        return NULL;
    }

    // Generate the sid
    unsigned char iv[16];
    char hexiv[33];
    getRandomIV( iv, 16 );  // Generate 16 random bytes
    memset( hexiv, 0, sizeof(hexiv) );
    vscp_byteArray2HexStr( hexiv, iv, 16 );
    
    memset( pSession->m_sid, 0, sizeof( pSession->m_sid ) );
    memcpy( pSession->m_sid, hexiv, 32 );    
    memset( pSession->m_websocket_key, 0, sizeof( pSession->m_websocket_key ) ); 

    // Init.
    strcpy( pSession->m_websocket_key, ws_key );           // Save key    
    pSession->m_conn = (struct web_connection *)conn;
    pSession->m_conn_state = WEBSOCK_CONN_STATE_CONNECTED;
    pSession->m_version = atoi( ws_version );              // Store protocol version
    
    pSession->m_pClientItem = new CClientItem();           // Create client
    if ( NULL == pSession->m_pClientItem ) {
        gpobj->logMsg(_("[Websockets] New session: Unable to create client object."));
        delete pSession;
        return NULL;
    }

    pSession->m_pClientItem->bAuthenticated = false;       // Not authenticated in yet
    vscp_clearVSCPFilter(&pSession->m_pClientItem->m_filterVSCP);    // Clear filter
    pSession->bEventTrigger = false;
    pSession->triggerTimeout = 0;
    pSession->bVariableTrigger = false;
    

    // This is an active client
    pSession->m_pClientItem->m_bOpen = false;
    pSession->m_pClientItem->m_type = CLIENT_ITEM_INTERFACE_TYPE_CLIENT_WEBSOCKET;
    pSession->m_pClientItem->m_strDeviceName = _("Internal websocket client.");
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
        gpobj->logMsg( _("Websocket server: Failed to add client. Terminating thread.") );
        return NULL;
    }
    gpobj->m_wxClientMutex.Unlock();

    gpobj->m_websocketSessionMutex.Lock();
    gpobj->m_websocketSessions.Append( pSession );
    gpobj->m_websocketSessionMutex.Unlock();
    
    // Use the session object as user data
    web_set_user_connection_data( pSession->m_conn,
                                  (void *)pSession );

    return pSession;
}


///////////////////////////////////////////////////////////////////////////////
// websock_expire_sessions
//

/*
void
websock_expire_sessions( struct web_connection *conn )
{
    websock_session *pos;
    websock_session *prev;
    websock_session *next;
    time_t now;

    //return;

    // Check pointer
    if (NULL == nc) return;

    CControlObject *pObject = (CControlObject *)nc->mgr->user_data;
    if (NULL == pObject) return;

    now = time( NULL );
    prev = NULL;
    pos = gp_websock_sessions;

    m_websockSessionMutex.Lock();

    while ( NULL != pos ) {

        next = pos->m_next;

        if ( ( now - pos->lastActiveTime ) > ( WEBSOCKET_EXPIRE_TIME) ) {

            // expire sessions after WEBSOCKET_EXPIRE_TIME
            if ( NULL == prev ) {
                // First pos in list
                gp_websock_sessions = pos->m_next;
            }
            else {
                // Point to item after this one
                prev->m_next = next;
            }

            // Remove client and session item
            pObject->m_wxClientMutex.Lock();
            pObject->removeClient( pos->m_pClientItem );
            pos->m_pClientItem = NULL;
            pObject->m_wxClientMutex.Unlock();
            
            // Free session data
            free( pos );

        }
        else {
            prev = pos;
        }

        pos = next;
    }

    m_websockSessionMutex.Unlock();
}
*/


///////////////////////////////////////////////////////////////////////////////
// websock_sendevent
//
// Send event to all other clients.
//

bool
websock_sendevent( struct web_connection *conn,
                        websock_session *pSession,
                        vscpEvent *pEvent )
{
    bool bSent = false;
    bool rv = true;

    // Check pointer
    if (NULL == conn) return false;
    if (NULL == pSession) return false;

    // Level II events between 512-1023 is recognized by the daemon and
    // sent to the correct interface as Level I events if the interface
    // is addressed by the client.
    if ((pEvent->vscp_class <= 1023) &&
            (pEvent->vscp_class >= 512) &&
            (pEvent->sizeData >= 16)) {

        // This event should be sent to the correct interface if it is
        // available on this machine. If not it should be sent to
        // the rest of the network as normal

        cguid destguid;
        destguid.getFromArray( pEvent->pdata );
        destguid.setAt(0,0);
        destguid.setAt(1,0);
        //unsigned char destGUID[16];  TODO ???
        //memcpy(destGUID, pEvent->pdata, 16); // get destination GUID
        //destGUID[0] = 0; // Interface GUID's have LSB bytes nilled
        //destGUID[1] = 0;

        gpobj->m_wxClientMutex.Lock();

        // Find client
        CClientItem *pDestClientItem = NULL;
        std::list<CClientItem*>::iterator it;
        for (it = gpobj->m_clientList.m_clientItemList.begin();
                it != gpobj->m_clientList.m_clientItemList.end();
                ++it) {

            CClientItem *pItem = *it;
            if ( pItem->m_guid == destguid ) {
                // Found
                pDestClientItem = pItem;
                break;
            }

        }

        if ( NULL != pDestClientItem ) {

            // If the client queue is full for this client then the
            // client will not receive the message
            if (pDestClientItem->m_clientInputQueue.GetCount() <=
                        gpobj->m_maxItemsInClientReceiveQueue) {

                // Create copy of event
                vscpEvent *pnewEvent = new vscpEvent;

                if (NULL != pnewEvent) {

                    pnewEvent->pdata = NULL;

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

        gpobj->m_wxClientMutex.Unlock();

    }

    if ( !bSent ) {

        // There must be room in the send queue
        if (gpobj->m_maxItemsInClientReceiveQueue >
                gpobj->m_clientOutputQueue.GetCount()) {

            // Create copy of event
            vscpEvent *pnewEvent = new vscpEvent;

            if (NULL != pnewEvent) {

                pnewEvent->pdata = NULL;

                vscp_copyVSCPEvent(pnewEvent, pEvent);

                gpobj->m_mutexClientOutputQueue.Lock();
                gpobj->m_clientOutputQueue.Append(pnewEvent);
                gpobj->m_semClientOutputQueue.Post();
                gpobj->m_mutexClientOutputQueue.Unlock();

            }

        }
        else {
            rv = false;
        }
    }

    return rv;
}





///////////////////////////////////////////////////////////////////////////////
// websocket_post_incomingEvent
//

void
websock_post_incomingEvents( void )
{
    gpobj->m_websocketSessionMutex.Lock();
    
    WEBSOCKETSESSIONLIST::iterator iter;
    for ( iter = gpobj->m_websocketSessions.begin(); 
            iter != gpobj->m_websocketSessions.end(); 
            ++iter ) {
        
        websock_session *pSession= *iter;
        if ( NULL == pSession) continue;
        
        // Should be a client item... hmm.... client disconnected 
        if ( NULL == pSession->m_pClientItem ) {
            continue;
        }
        
        if ( pSession->m_conn_state < WEBSOCK_CONN_STATE_CONNECTED ) continue;
        
        if ( NULL == pSession->m_conn ) continue;
        
        if ( pSession->m_pClientItem->m_bOpen &&
                pSession->m_pClientItem->m_clientInputQueue.GetCount() ) {

            CLIENTEVENTLIST::compatibility_iterator nodeClient;
            vscpEvent *pEvent;

            pSession->m_pClientItem->m_mutexClientInputQueue.Lock();    
            nodeClient = pSession->m_pClientItem->m_clientInputQueue.GetFirst();
            
            if ( NULL == nodeClient )  continue;
            
            pEvent = nodeClient->GetData();                             
            pSession->m_pClientItem->m_clientInputQueue.DeleteNode(nodeClient);
            pSession->m_pClientItem->m_mutexClientInputQueue.Unlock();

            if ( NULL != pEvent ) {

                // Run event through filter
                if ( vscp_doLevel2Filter( pEvent, 
                        &pSession->m_pClientItem->m_filterVSCP ) ) {

                    wxString str;
                    if ( vscp_writeVscpEventToString( pEvent, str ) ) {

                        // Write it out
                        str = _("E;") + str;
                        web_websocket_write( pSession->m_conn, 
                                                WEB_WEBSOCKET_OPCODE_TEXT, 
                                                (const char *)str.mbc_str(), 
                                                str.Length() );

                    }
                }

                // Remove the event
                vscp_deleteVSCPevent(pEvent);

            } // Valid pEvent pointer
                
        } // events available
        
    } // for
    
    gpobj->m_websocketSessionMutex.Unlock();

 }


///////////////////////////////////////////////////////////////////////////////
// websock_post_variableTrigger
//
// op = 0 - Variable changed    "V"
// op = 1 - Variable created    "N"
// op = 2 - Variable deleted    "D"
//

void
websock_post_variableTrigger( uint8_t op, CVSCPVariable* pVar )
{
    gpobj->m_websocketSessionMutex.Lock();
    
    WEBSOCKETSESSIONLIST::iterator iter;
    for ( iter = gpobj->m_websocketSessions.begin(); 
            iter != gpobj->m_websocketSessions.end(); 
            ++iter ) {
        
        websock_session *pSession= *iter;
        if ( NULL == pSession) continue;
        
        // Should be a client item... hmm.... client disconnected 
        if ( NULL == pSession->m_pClientItem ) {
            continue;
        }
        
        if ( pSession->m_conn_state < WEBSOCK_CONN_STATE_CONNECTED ) continue;
        
        if ( NULL == pSession->m_conn ) continue;
        
        if ( pSession->m_pClientItem->m_bOpen  ) {

            wxString outstr;
            outstr = _("V;"); // Variable trigger
            web_websocket_write( pSession->m_conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT, 
                                    (const char *)outstr.mbc_str(), 
                                    outstr.Length() );
                                              
        } // open
        
    } // for
    
    gpobj->m_websocketSessionMutex.Unlock();

 }


////////////////////////////////////////////////////////////////////////////////
// ws1_connectHandler
//

int
ws1_connectHandler( const struct web_connection *conn, void *cbdata ) 
{
    struct web_context *ctx = web_get_context( conn );
    int reject = 1;

    // Check pointers
    if ( NULL == conn ) return 1;
    if ( NULL == ctx ) return 1;
    
    web_lock_context( ctx );
    websock_session *pSession = websock_new_session( conn );
    
    if ( NULL != pSession ) {
        reject = 0;
    }
    web_unlock_context( ctx );

    gpobj->logMsg( wxString::Format( "[Websocket] Connection: client %s\r\n\r\n",
                                     ( reject ? "rejected" : "accepted" ) ) );
    
    return reject;
}

////////////////////////////////////////////////////////////////////////////////
// ws1_closeHandler
//

void
ws1_closeHandler(const struct web_connection *conn, void *cbdata) 
{
    struct web_context *ctx = web_get_context( conn );
    websock_session *pSession = 
                (websock_session *)web_get_user_connection_data( conn );

    if ( NULL == conn ) return;
    if ( NULL == pSession ) return;
    if ( pSession->m_conn != conn ) return;
    if ( pSession->m_conn_state < WEBSOCK_CONN_STATE_CONNECTED ) return;

    web_lock_context( ctx );
    
    // Record activity
    pSession->lastActiveTime = time(NULL);
    
    pSession->m_conn_state = WEBSOCK_CONN_STATE_NULL;
    pSession->m_conn = NULL;
    gpobj->m_clientList.removeClient( pSession->m_pClientItem );
    pSession->m_pClientItem = NULL;
    
    gpobj->m_websocketSessionMutex.Lock();
    gpobj->m_websocketSessions.DeleteContents( true ); 
    if ( !gpobj->m_websocketSessions.DeleteObject( pSession )  ) {
        gpobj->logMsg( _("[Websocket] Failed to delete session object."), 
                        DAEMON_LOGMSG_NORMAL, 
                        DAEMON_LOGTYPE_SECURITY );
    }
    gpobj->m_websocketSessionMutex.Unlock();
               
    web_unlock_context( ctx );
}

////////////////////////////////////////////////////////////////////////////////
// ws1_readyHandler
//

void
ws1_readyHandler( struct web_connection *conn, void *cbdata ) 
{
    websock_session *pSession = 
        (websock_session *)web_get_user_connection_data( conn );
    
    // Check pointers
    if ( NULL == conn ) return;
    if ( NULL == pSession ) return;
    if ( pSession->m_conn != conn ) return;
    if ( pSession->m_conn_state < WEBSOCK_CONN_STATE_CONNECTED ) return;
    
    // Record activity
    pSession->lastActiveTime = time(NULL);
                
    // Start authentication  
    wxString wxstr = wxString::Format( _("+;AUTH0;%s"),
                              pSession->m_sid );
    web_websocket_write( conn, 
                            WEB_WEBSOCKET_OPCODE_TEXT,
                            (const char *)wxstr.mbc_str(),
                            wxstr.Length() );

    pSession->m_conn_state = WEBSOCK_CONN_STATE_DATA;
}

////////////////////////////////////////////////////////////////////////////////
// ws1_dataHandler
//

int
ws1_dataHandler( struct web_connection *conn,
                    int bits,
                    char *data,
                    size_t len,
                    void *cbdata ) 
{
    wxString strWsPkt;
    websock_session *pSession = 
        (websock_session *)web_get_user_connection_data( conn );
    
    // Check pointers
    if ( NULL == conn ) return WEB_ERROR;
    if ( NULL == pSession ) return WEB_ERROR;
    if ( pSession->m_conn != conn ) return WEB_ERROR;
    if ( pSession->m_conn_state < WEBSOCK_CONN_STATE_CONNECTED ) return WEB_ERROR;
    
    // Record activity
    pSession->lastActiveTime = time(NULL);

    switch ( ( (unsigned char)bits ) & 0x0F ) {
        
        case WEB_WEBSOCKET_OPCODE_CONTINUATION:
            
            // Save and concatenate mesage
            pSession->m_strConcatenated += wxString::FromUTF8( data, len );
            
            // if last process is
            if ( 1 & bits ) {
                if ( !ws1_message( conn, 
                                    pSession, 
                                    pSession->m_strConcatenated ) ){
                    return WEB_ERROR;
                }
            }
            break;
            
        // https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API/Writing_WebSocket_servers    
        case WEB_WEBSOCKET_OPCODE_TEXT:
            if ( 1 & bits ) {
                strWsPkt = wxString::FromUTF8( data, len );                
                if ( !ws1_message( conn, pSession, strWsPkt ) ){
                    return WEB_ERROR;
                }
            }
            else {
                // Store first part 
                pSession->m_strConcatenated = wxString::FromUTF8( data, len );
            }
            break;
            
        case WEB_WEBSOCKET_OPCODE_BINARY:
            break;
            
        case WEB_WEBSOCKET_OPCODE_CONNECTION_CLOSE:
            break;
            
        case WEB_WEBSOCKET_OPCODE_PING:
            fprintf( stdout, "Ping received" ); 
            break;
            
        case WEB_WEBSOCKET_OPCODE_PONG:
            fprintf( stdout, "Pong received" ); 
            break;
            
        default:
            break;
            
    }
    
    return WEB_OK;
}

///////////////////////////////////////////////////////////////////////////////
// ws1_message
//

bool
ws1_message( struct web_connection *conn,
                websock_session *pSession,
                wxString &strWsPkt )
{
    wxString wxstr;

    // Check pointer
    if ( NULL == conn ) return false;
    if ( NULL == pSession ) return false;
    
    strWsPkt.Trim();
    strWsPkt.Trim(false);
    
    char c;
    strWsPkt[0].GetAsChar( &c );
    
    switch ( c ) {

        // Command - | 'C' | command type (byte) | data |
        case 'C':
            // Point beyond initial info "C;"
            strWsPkt = strWsPkt.Right( strWsPkt.Length() - 2 );
            ws1_command( conn, pSession, strWsPkt );
            break;

        // Event | 'E' ; head(byte) , vscp_class(unsigned short) , vscp_type(unsigned
        //              short) , GUID(16*byte), data(0-487 bytes) |
        case 'E':
        {
            // Must be authorised to do this
            if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
                
                wxstr = wxString::Format( _("-;%d;%s"),
                                            (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                            WEBSOCK_STR_ERROR_NOT_AUTHORISED );
                web_websocket_write( conn, 
                                        WEB_WEBSOCKET_OPCODE_TEXT,
                                        (const char *)wxstr,
                                        wxstr.Length() );
                return true;
            }

            // Point beyond initial info "E;"
            strWsPkt = strWsPkt.Right( strWsPkt.Length() - 2 );
            vscpEvent vscp_event;

            if ( vscp_setVscpEventFromString( &vscp_event, strWsPkt ) ) {
                
                // If GUID is all null give it GUID of interface
                if ( vscp_isGUIDEmpty( vscp_event.GUID ) ) {
                    pSession->m_pClientItem->m_guid.writeGUID( vscp_event.GUID );
                }

                // Check if this user is allowed to send this event
                if ( !pSession->m_pClientItem->m_pUserItem->isUserAllowedToSendEvent( 
                                 vscp_event.vscp_class, vscp_event.vscp_type ) ) {
                    wxString strErr =
                        wxString::Format( _("websocket] User [%s] not allowed to "
                                            "send event class=%d type=%d.\n"),
                            pSession->m_pClientItem->m_pUserItem->getUserName().mbc_str(),
                            vscp_event.vscp_class, vscp_event.vscp_type );

                    gpobj->logMsg( strErr, 
                                        DAEMON_LOGMSG_NORMAL, 
                                        DAEMON_LOGTYPE_SECURITY );
                }

                vscp_event.obid = pSession->m_pClientItem->m_clientID;
                if (  websock_sendevent( conn, pSession, &vscp_event ) ) {
                    web_websocket_write( conn, 
                                            WEB_WEBSOCKET_OPCODE_TEXT, 
                                            "+;EVENT",
                                            7 );
                }
                else {
                    wxstr = wxString::Format( _("-;%d;%s"),
                                                (int)WEBSOCK_ERROR_TX_BUFFER_FULL,
                                                WEBSOCK_STR_ERROR_TX_BUFFER_FULL );
                    web_websocket_write( conn, 
                                            WEB_WEBSOCKET_OPCODE_TEXT,
                                            (const char *)wxstr.mbc_str(),
                                            wxstr.Length() );
                }
            }

        }
        break;

        // Unknown command
        default:
            break;

    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
// ws1_command
//

void 
ws1_command( struct web_connection *conn,              
                    struct websock_session *pSession,
                    wxString& strCmd )
{
    wxString wxstr; // Worker string
    wxString strTok;

    // Check pointer
    if (NULL == conn) return;
    if (NULL == pSession) return;

    gpobj->logMsg( _("[Websocket] Command = ") + 
                    strCmd + _("\n"),
                    DAEMON_LOGMSG_DEBUG,
                    DAEMON_LOGTYPE_GENERAL );

    wxStringTokenizer tkz( strCmd, _(";"), wxTOKEN_RET_EMPTY_ALL );

    // Get command
    if (tkz.HasMoreTokens()) {
        strTok = tkz.GetNextToken();
        strTok.MakeUpper();
    }
    else {
        wxString wxstr = 
                wxString::Format( _("-;%d;%s"),
                                    (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                    WEBSOCK_STR_ERROR_SYNTAX_ERROR );
        web_websocket_write( conn, 
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)wxstr.mbc_str(),
                                wxstr.Length() );
        return;
    }
    
    
    // ------------------------------------------------------------------------
    //                                NOOP
    //-------------------------------------------------------------------------
    
    if ( 0 == strTok.Find( _("NOOP") ) ) {

        web_websocket_write( conn, 
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                "+;NOOP",
                                6 );

        // Send authentication challenge
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
            // TODO
        }
        
    }
    
    // ------------------------------------------------------------------------
    //                               CHALLENGE
    //-------------------------------------------------------------------------
    
    else if ( 0 == strTok.Find( _("CHALLENGE") ) ) {

        // Send authentication challenge
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
                        
            // Start authentication  
            wxstr = wxString::Format( _("+;AUTH0;%s"),
                                        pSession->m_sid );
            web_websocket_write( conn, 
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)wxstr.mbc_str(),
                                wxstr.Length() );
            
        }

    }
    
    // ------------------------------------------------------------------------
    //                                AUTH
    //-------------------------------------------------------------------------
    
    // AUTH;iv;aes128
    else if ( 0 == strTok.Find( _("AUTH") ) ) {

        wxString wxstr;
        wxString strUser;
        wxString strIV = tkz.GetNextToken();
        wxString strCrypto = tkz.GetNextToken();
        if ( websock_authentication( conn,              
                                        pSession,
                                        strIV,
                                        strCrypto ) ) {
            wxString userSettings;
            pSession->m_pClientItem->m_pUserItem->getAsString( userSettings );
            wxstr = wxString::Format( _("+;AUTH1;%s"),
                                        (const char *)userSettings.mbc_str() );
	    web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT, 
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        }
        else {
            
            wxstr = wxString::Format( _("-;AUTH;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                        WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            pSession->m_pClientItem->bAuthenticated  = false;   // Authenticated
        }
    }
    
    // ------------------------------------------------------------------------
    //                                OPEN
    //-------------------------------------------------------------------------
    
    else if ( 0 == strTok.Find( _("OPEN") ) ) {

        // Must be authorised to do this
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
            
            wxstr = wxString::Format( _("-;OPEN;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                        WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            
            return;     // We still leave channel open
        }

        pSession->m_pClientItem->m_bOpen = true;
        web_websocket_write( conn, 
                                WEB_WEBSOCKET_OPCODE_TEXT, 
                                "+;OPEN",
                                6 );
    }
    
    // ------------------------------------------------------------------------
    //                                CLOSE
    //-------------------------------------------------------------------------

    else if ( 0 == strTok.Find(_("CLOSE") ) ) {
        pSession->m_pClientItem->m_bOpen = false;
        web_websocket_write( conn, 
                                WEB_WEBSOCKET_OPCODE_TEXT, 
                                "+;CLOSE",
                                7 );
    }
    
    // ------------------------------------------------------------------------
    //                             SETFILTER/SF
    //-------------------------------------------------------------------------
    
    else if ( ( 0 == strTok.Find( _("SETFILTER") ) ) || 
            ( 0 == strTok.Find( _("SF") ) ) ) {

        unsigned char ifGUID[ 16 ];
        memset(ifGUID, 0, 16);

        // Must be authorized to do this
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
            
            wxstr = wxString::Format( _("-;SF;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                        WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );

            gpobj->logMsg ( _("[Websocket] User/host not authorised to set a filter.\n"),
                                        DAEMON_LOGMSG_NORMAL,
                                        DAEMON_LOGTYPE_SECURITY );

            return;     // We still leave channel open
        }

        // Check privilege
        if ( ( pSession->m_pClientItem->m_pUserItem->getUserRights( 0 ) & 0xf ) < 6 ) {
            
            wxstr = wxString::Format( _("-;SF;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,
                                        WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );

            wxString strErr =
                  wxString::Format( _("[Websocket] User [%s] not "
                                      "allowed to set a filter.\n"),
                  pSession->m_pClientItem->m_pUserItem->getUserName().mbc_str() );

            gpobj->logMsg ( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_SECURITY );
            return; // We still leave channel open
        }

        // Get filter
        if ( tkz.HasMoreTokens() ) {

            strTok = tkz.GetNextToken();

            pSession->m_pClientItem->m_mutexClientInputQueue.Lock();
            if ( !vscp_readFilterFromString( &pSession->m_pClientItem->m_filterVSCP, strTok ) ) {

                
                wxstr = wxString::Format( _("-;SF;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );

                pSession->m_pClientItem->m_mutexClientInputQueue.Unlock();                                    
                return;
            }

            pSession->m_pClientItem->m_mutexClientInputQueue.Unlock();
        }
        else {
            
            wxstr = wxString::Format( _("-;SF;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );

            return;
        }

        // Get mask
        if ( tkz.HasMoreTokens() ) {

            strTok = tkz.GetNextToken();

            pSession->m_pClientItem->m_mutexClientInputQueue.Lock();
            if (!vscp_readMaskFromString( &pSession->m_pClientItem->m_filterVSCP,
                                            strTok ) ) {
                
                wxstr = wxString::Format( _("-;SF;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );

                pSession->m_pClientItem->m_mutexClientInputQueue.Unlock();
                return;
                
            }
            
            pSession->m_pClientItem->m_mutexClientInputQueue.Unlock();
            
        }
        else {
                wxstr = wxString::Format( _("-;SF;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            return;
        }

        // Positive response
        web_websocket_write( conn, 
                                WEB_WEBSOCKET_OPCODE_TEXT, 
                                "+;SF",
                                4 );

    }
    
    // ------------------------------------------------------------------------
    //                           CLRQ/CLRQUEUE
    //-------------------------------------------------------------------------
    
    // Clear the event queue
    else if ( ( 0 == strTok.Find(_("CLRQUEUE") ) ) || 
                ( 0 == strTok.Find(_("CLRQ") ) ) ) {

        CLIENTEVENTLIST::iterator iterVSCP;

        // Must be authorised to do this
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
            
            wxstr = wxString::Format( _("-;CLRQ;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                        WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );

            gpobj->logMsg ( _("[Websocket] User/host not authorised to clear the queue.\n"),
                                        DAEMON_LOGMSG_NORMAL,
                                        DAEMON_LOGTYPE_SECURITY );

            return;     // We still leave channel open
        }

        // Check privilege
        if ( ( pSession->m_pClientItem->m_pUserItem->getUserRights( 0 ) & 0xf) < 1 ) {
            
            wxstr = wxString::Format( _("-;CLRQ;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,
                                        WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
     
            wxString strErr =
                        wxString::Format( _("[Websocket] User [%s] not allowed to clear the queue.\n"),
                                                pSession->m_pClientItem->m_pUserItem->getUserName().mbc_str() );

            gpobj->logMsg ( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_SECURITY );
            return; // We still leave channel open
        }

        pSession->m_pClientItem->m_mutexClientInputQueue.Lock();
        for ( iterVSCP = pSession->m_pClientItem->m_clientInputQueue.begin();
                iterVSCP != pSession->m_pClientItem->m_clientInputQueue.end(); ++iterVSCP ) {
            vscpEvent *pEvent = *iterVSCP;
            vscp_deleteVSCPevent(pEvent);
        }

        pSession->m_pClientItem->m_clientInputQueue.Clear();
        pSession->m_pClientItem->m_mutexClientInputQueue.Unlock();

        web_websocket_write( conn, 
                                WEB_WEBSOCKET_OPCODE_TEXT, 
                                "+;CLRQ",
                                6 );
        
    }
    
    ////////////////////////////////////////////////////////////////////////////
    //                              VARIABLES
    ////////////////////////////////////////////////////////////////////////////
    
    
    // ------------------------------------------------------------------------
    //                            CVAR/CREATEVAR
    //-------------------------------------------------------------------------
    
    else if ( ( 0 == strTok.Find(_("CREATEVAR") ) ) || 
                ( 0 == strTok.Find(_("CVAR") ) ) ) {

        wxString name;
        wxString value;
        wxString note;
        uint8_t type = VSCP_DAEMON_VARIABLE_CODE_STRING;
        bool bPersistent = false;
        uint32_t accessrights = 744;

        // Must be authorised to do this
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
            
            wxstr = wxString::Format( _("-;CVAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                        WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            
            gpobj->logMsg ( _("[Websocket] User/host not authorised to create a variable.\n"),
                                        DAEMON_LOGMSG_NORMAL,
                                        DAEMON_LOGTYPE_SECURITY );

            return;     // We still leave channel open
        }

        // Check privilege
        if ( (pSession->m_pClientItem->m_pUserItem->getUserRights( 0 ) & 0xf) < 6 ) {
            
            wxstr = wxString::Format( _("-;CVAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,
                                        WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
    
            wxString strErr =
                        wxString::Format( _("[Websocket] User [%s] not allowed to create a variable.\n"),
                                                pSession->m_pClientItem->m_pUserItem->getUserName().mbc_str() );

            gpobj->logMsg ( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_SECURITY );
            return ;    // We still leave channel open
        }
        
        // “C;CVAR;name;type;accessrights;bPersistens;value;note”

        // Get variable name
        if (tkz.HasMoreTokens()) {
            name = tkz.GetNextToken();
        }
        else {
            
            wxstr = wxString::Format( _("-;CVAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            
            return;
        }
        
        // Check if the variable exist
        CVSCPVariable variable;
        bool bVariableExist = false;
        if ( 0 != gpobj->m_variables.find( name, variable ) ) {
            bVariableExist = true;
        }
        
        // name can not start with "vscp." - reserved for a stock variable
        if ( name.Lower().StartsWith( "vscp.") ) {
            
            wxstr = wxString::Format( _("-;CVAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_VARIABLE_NO_STOCK,
                                        WEBSOCK_STR_ERROR_VARIABLE_NO_STOCK );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            
            return;
        }

        // Get variable type
        bool bInputType = false;
        if ( tkz.HasMoreTokens() ) {
            wxString wxstr = tkz.GetNextToken();
            wxstr = wxstr.Trim();
            if ( wxstr.Length() ) bInputType = true;
            type = vscp_readStringValue( wxstr );
        }
        
        // Get variable access rights
        bool bInputAccessRights = false;
        if ( tkz.HasMoreTokens() ) {
            wxString wxstr = tkz.GetNextToken();
            wxstr = wxstr.Trim();
            if ( wxstr.Length() ) bInputAccessRights = true;
            accessrights = vscp_readStringValue( wxstr );
        }

        // Get variable Persistence 0/1
        bool bInputPersistent = false;
        if ( tkz.HasMoreTokens() ) {
            wxString wxstr = tkz.GetNextToken();
            wxstr = wxstr.Trim();
            if ( wxstr.Length() ) bInputPersistent = true;
            int val = vscp_readStringValue( wxstr );

            if ( 0 == val ) {
                bPersistent = false;
            }
            else if ( 1 == val ) {
                bPersistent = true;
            }
            else {
                wxstr = wxString::Format( _("-;CVAR;%d;%s"),
                                            (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                            WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
                web_websocket_write( conn, 
                                        WEB_WEBSOCKET_OPCODE_TEXT,
                                        (const char *)wxstr.mbc_str(),
                                        wxstr.length() );
                return;
            }
        }

        // Get variable value
        bool bInputValue = false;
        if ( tkz.HasMoreTokens() ) {
            bInputValue = true;
            value = tkz.GetNextToken(); 
            vscp_base64_wxdecode( value );
        }
        
        // Get variable note
        bool bInputNote = false;
        if (tkz.HasMoreTokens()) {
            bInputNote = true;
            note = tkz.GetNextToken(); 
            vscp_base64_wxdecode( value );
        }

        if ( !bVariableExist ) {
            
            // Add the variable
            if ( !gpobj->m_variables.add( name, 
                                value, 
                                type, 
                                pSession->m_pClientItem->m_pUserItem->getUserID(), 
                                bPersistent, 
                                accessrights, 
                                note ) ) {
                
                wxstr = wxString::Format( _("-;CVAR;%d;%s"),
                                            (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                            WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
                web_websocket_write( conn, 
                                        WEB_WEBSOCKET_OPCODE_TEXT,
                                        (const char *)wxstr.mbc_str(),
                                        wxstr.length() );
                return;
            }
        }
        else {
            
            // Save changes to the variable 
            
            if ( bInputType ) {
                variable.setType( type );
            }
            
            if ( bInputAccessRights ) {
                variable.setAccessRights( accessrights );
            }
            
            // Persistence can't be changed
            
            if ( bInputValue ) {
                variable.setValue( value );
            }
            
            if ( bInputNote ) {
                variable.setNote( note );
            }
            
            // Save the changed variable
            if ( !gpobj->m_variables.update( variable ) ) {
                
                wxstr = wxString::Format( _("-;CVAR;%d;%s"),
                                            (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                            WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
                web_websocket_write( conn, 
                                        WEB_WEBSOCKET_OPCODE_TEXT,
                                        (const char *)wxstr.mbc_str(),
                                        wxstr.length() );
                return;
            }
            
        }

        wxString strResult = _("+;CVAR;");
        strResult += name;

        web_websocket_write( conn, 
                                WEB_WEBSOCKET_OPCODE_TEXT, 
                                (const char *)strResult.mbc_str(),
                                strResult.Length() );

    }
    
    // ------------------------------------------------------------------------
    //                              RVAR/READVAR
    //-------------------------------------------------------------------------
    
    else if ( ( 0 == strTok.Find(_("READVAR") ) ) || 
                ( 0 == strTok.Find(_("RVAR") ) ) ) {

        CVSCPVariable variable;
        uint8_t type;
        wxString strvalue;

        // Must be authorised to do this
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
            
            wxstr = wxString::Format( _("-;RVAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                        WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );

            gpobj->logMsg ( _("[Websocket] User/host not authorised to read a variable.\n"),
                                        DAEMON_LOGMSG_NORMAL,
                                        DAEMON_LOGTYPE_SECURITY );

            return;     // We still leave channel open
        }

        // Check privilege
        if ( (pSession->m_pClientItem->m_pUserItem->getUserRights( 0 ) & 0xf) < 4 ) {
            
            wxstr = wxString::Format( _("-;RVAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,
                                        WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            wxString strErr =
                        wxString::Format( _("[Websocket] User [%s] not allowed to read a variable.\n"),
                                                pSession->m_pClientItem->m_pUserItem->getUserName().mbc_str() );

            gpobj->logMsg ( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_SECURITY );
            return;     // We still leave channel open
        }

        strTok = tkz.GetNextToken();
        if ( 0 == gpobj->m_variables.find( strTok, variable ) ) {
   
            wxstr = wxString::Format( _("-;RVAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_VARIABLE_UNKNOWN,
                                        WEBSOCK_STR_ERROR_VARIABLE_UNKNOWN );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            return;
        }

        // name;type;bPersistent;userid;rights;lastchanged;value;note
        wxString strResult = _("+;RVAR;");
        strResult += variable.getAsString( false );
        web_websocket_write( conn, 
                                WEB_WEBSOCKET_OPCODE_TEXT, 
                                (const char *)strResult.mbc_str(),
                                strResult.Length() );
    }
    
    // ------------------------------------------------------------------------
    //                                WVAR/WRITEVAR
    //-------------------------------------------------------------------------
    
    else if ( ( 0 == strTok.Find( _("WRITEVAR") ) ) || 
                ( 0 == strTok.Find( _("WVAR") ) ) ) {

        CVSCPVariable variable;
        wxString strvalue;
        uint8_t type;

        // Must be authorised to do this
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
            
            wxstr = wxString::Format( _("-;WVAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                        WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            
            gpobj->logMsg ( _("[Websocket] User/host not authorised to write a variable.\n"),
                                        DAEMON_LOGMSG_NORMAL,
                                        DAEMON_LOGTYPE_SECURITY );

            return; // We still leave channel open
        }

        // Check privilege
        if ( (pSession->m_pClientItem->m_pUserItem->getUserRights( 0 ) & 0xf) < 6 ) {
            
            wxstr = wxString::Format( _("-;WVAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,
                                        WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            
            wxString strErr =
                        wxString::Format( _("[Websocket] User [%s] not allowed to do write variable.\n"),
                                                pSession->m_pClientItem->m_pUserItem->getUserName().mbc_str() );

            gpobj->logMsg ( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_SECURITY );
            return;     // We still leave channel open
        }

        // Get variable name
        wxString strVarName;
        if (tkz.HasMoreTokens()) {


            strVarName = tkz.GetNextToken();
            if ( 0 == gpobj->m_variables.find( strVarName.Upper(), variable ) ) {
                
                wxstr = wxString::Format( _("-;WVAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_VARIABLE_UNKNOWN,
                                        WEBSOCK_STR_ERROR_VARIABLE_UNKNOWN );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            
                return;
            }

            // Get variable value
            if ( tkz.HasMoreTokens() ) {
                
                strTok = tkz.GetNextToken();                
                
                if ( !variable.setValueFromString( variable.getType(), 
                                                        strTok,
                                                        true ) ) {     // decode
                    
                    wxstr = wxString::Format( _("-;WVAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
                    web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
                    return;
                }
                
                // Update the variable
                if ( !gpobj->m_variables.update( variable ) ) {
 
                    wxstr = wxString::Format( _("-;WVAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_VARIABLE_UPDATE );
            
                    web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
                    return;
                }
            }
            else {
                wxstr = wxString::Format( _("-;WVAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
                return;
            }
        }
        else {
            wxstr = wxString::Format( _("-;WVAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            return;
        }

        wxString strResult = _( "+;WVAR;" );
        strResult += strVarName;

        // Positive reply
        web_websocket_write( conn, 
                                WEB_WEBSOCKET_OPCODE_TEXT, 
                                (const char *)strResult.mbc_str(),
                                strResult.Length() );

    }
    
    // ------------------------------------------------------------------------
    //                             RSTVAR/RESETVAR
    //-------------------------------------------------------------------------
    
    else if ( ( 0 == strTok.Find(_("RESETVAR") ) ) || 
                    ( 0 == strTok.Find(_("RSTVAR") ) ) ) {

        CVSCPVariable variable;
        wxString strvalue;
        uint8_t type;

        // Must be authorised to do this
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
            
            wxstr = wxString::Format( _("-;RSTVAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                        WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );

            return;     // We still leave channel open
        }

        // Check privilege
        if ( (pSession->m_pClientItem->m_pUserItem->getUserRights( 0 ) & 0xf) < 6 ) {
            
            wxstr = wxString::Format( _("-;RSTVAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,
                                        WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
    
            wxString strErr =
                        wxString::Format( _("[Websocket] User [%s] not allowed to reset a variable.\n"),
                                                pSession->m_pClientItem->m_pUserItem->getUserName().mbc_str() );

            gpobj->logMsg ( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_SECURITY );
            return;     // We still leave channel open
        }

        strTok = tkz.GetNextToken();
        if ( 0 == gpobj->m_variables.find(strTok, variable )) {
            
            wxstr = wxString::Format( _("-;RSTVAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_VARIABLE_UNKNOWN,
                                        WEBSOCK_STR_ERROR_VARIABLE_UNKNOWN );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );

            gpobj->logMsg ( _("[Websocket] User/host not authorised to reset a variable.\n"),
                                        DAEMON_LOGMSG_NORMAL,
                                        DAEMON_LOGTYPE_SECURITY );

            return;
        }

        variable.Reset();

        variable.writeValueToString( strvalue );
        type = variable.getType();

        wxString strResult = _( "+;RSTVAR;" );
        strResult += strTok;
        strResult += _( ";" );
        strResult += wxString::Format( _( "%d" ), type );
        strResult += _( ";" );
        strResult += strvalue;

        // Positive reply
        web_websocket_write( conn, 
                                WEB_WEBSOCKET_OPCODE_TEXT, 
                                (const char *)strResult.mbc_str(),
                                strResult.Length() );

    }
    
    // ------------------------------------------------------------------------
    //                                 DELVAR/REMOVEVAR
    //-------------------------------------------------------------------------
    
    else if ( ( 0 == strTok.Find(_("DELVAR") ) ) || 
                ( 0 == strTok.Find(_("REMOVEVAR") ) ) ) {

        CVSCPVariable variable;
        wxString name;

        // Must be authorised to do this
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
            
            wxstr = wxString::Format( _("-;DELVAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                        WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            
            gpobj->logMsg ( _("[Websocket] User/host not authorised to delete a variable.\n"),
                                        DAEMON_LOGMSG_NORMAL,
                                        DAEMON_LOGTYPE_SECURITY );

            return;     // We still leave channel open
        }

        // Check privilege
        if ( (pSession->m_pClientItem->m_pUserItem->getUserRights( 0 ) & 0xf) < 6 ) {
            
            wxstr = wxString::Format( _("-;DELVAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,
                                        WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
    
            wxString strErr =
                        wxString::Format( _("[Websocket] User [%s] not allowed to delete a variable.\n"),
                                                pSession->m_pClientItem->m_pUserItem->getUserName().mbc_str() );

            gpobj->logMsg ( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_SECURITY );
            return;     // We still leave channel open
        }

        name = tkz.GetNextToken();
        if ( 0 == gpobj->m_variables.find( name, variable ) ) {
            
            wxstr = wxString::Format( _("-;DELVAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_VARIABLE_UNKNOWN,
                                        WEBSOCK_STR_ERROR_VARIABLE_UNKNOWN );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );

            return;
        }

        gpobj->m_variableMutex.Lock();
        gpobj->m_variables.remove( name );
        gpobj->m_variableMutex.Unlock();

        wxString strResult = _("+;DELVAR;");
        strResult += name;
        
        // Positive reply
        web_websocket_write( conn, 
                                WEB_WEBSOCKET_OPCODE_TEXT, 
                                (const char *)strResult.mbc_str(),
                                strResult.Length() );

    }
    
    // ------------------------------------------------------------------------
    //                             LENVAR/LENGTHVAR
    //-------------------------------------------------------------------------
    
    else if ( ( 0 == strTok.Find(_("LENGTHVAR") ) ) || 
                ( 0 == strTok.Find(_("LENVAR") ) ) ) {

        CVSCPVariable variable;

        // Must be authorised to do this
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
            
            wxstr = wxString::Format( _("-;LENVAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                        WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );

            gpobj->logMsg ( _("[Websocket] User/host not authorised to get length of variable.\n"),
                                        DAEMON_LOGMSG_NORMAL,
                                        DAEMON_LOGTYPE_SECURITY );

            return;     // We still leave channel open
        }

        // Check privilege
        if ( (pSession->m_pClientItem->m_pUserItem->getUserRights( 0 ) & 0xf) < 4 ) {
            
            wxstr = wxString::Format( _("-;LENVAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,
                                        WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );

            wxString strErr =
                        wxString::Format( _("[Websocket] User [%s] not allowed to get length of variable.\n"),
                                                pSession->m_pClientItem->m_pUserItem->getUserName().mbc_str() );

            gpobj->logMsg ( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_SECURITY );
            return;     // We still leave channel open
        }

        strTok = tkz.GetNextToken();
        if ( 0 == gpobj->m_variables.find(strTok, variable ) ) {
            
            wxstr = wxString::Format( _("-;LENVAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_VARIABLE_UNKNOWN,
                                        WEBSOCK_STR_ERROR_VARIABLE_UNKNOWN );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            return;
        }

        wxString strResult = _("+;LENVAR;");
        strResult += strTok;
        strResult += _(";");
        strResult += wxString::Format(_("%d"), variable.getValue().Length() );
        
        // Positive reply
        web_websocket_write( conn, 
                                WEB_WEBSOCKET_OPCODE_TEXT, 
                                (const char *)strResult.mbc_str(),
                                strResult.Length() );

    }
    
    // ------------------------------------------------------------------------
    //                           LCVAR/LASTCHANGEVAR
    //-------------------------------------------------------------------------
    
    else if ( ( 0 == strTok.Find(_("LASTCHANGEVAR") ) ) || 
                ( 0 == strTok.Find(_("LCVAR") ) ) ) {

        CVSCPVariable variable;
        wxString strvalue;

        // Must be authorised to do this
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
            
            wxstr = wxString::Format( _("-;LCVAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                        WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
 
            gpobj->logMsg ( _("[Websocket] User/host not authorised to get last change date of variable.\n"),
                                        DAEMON_LOGMSG_NORMAL,
                                        DAEMON_LOGTYPE_SECURITY );

            return;     // We still leave channel open
        }

        // Check privilege
        if ( (pSession->m_pClientItem->m_pUserItem->getUserRights( 0 ) & 0xf) < 4 ) {
            
            wxstr = wxString::Format( _("-;LCVAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,
                                        WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
    
            wxString strErr =
                wxString::Format( _("[Websocket] User [%s] not allowed "
                                    "to get last change date of variable.\n"),
                pSession->m_pClientItem->m_pUserItem->getUserName().mbc_str() );

            gpobj->logMsg ( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_SECURITY );
            return;     // We still leave channel open
        }

        strTok = tkz.GetNextToken();
        if ( 0 == gpobj->m_variables.find(strTok, variable ) ) {
            
            wxstr = wxString::Format( _("-;LCVAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_VARIABLE_UNKNOWN,
                                        WEBSOCK_STR_ERROR_VARIABLE_UNKNOWN );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );

            return;
        }

        variable.writeValueToString( strvalue );

        wxString strResult = _("+;LCVAR;");
        strResult += strTok;
        strResult += _( ";" );
        strResult += variable.getLastChange().FormatISODate() + _(" ") + variable.getLastChange().FormatISOTime();
        
        // Positive reply
        web_websocket_write( conn, 
                                WEB_WEBSOCKET_OPCODE_TEXT, 
                                (const char *)strResult.mbc_str(),
                                strResult.Length() );

    }
    
    // ------------------------------------------------------------------------
    //                               LSTVAR/LISTVAR
    //-------------------------------------------------------------------------
    
    else if ( ( 0 == strTok.Find( _("LISTVAR") ) ) || 
                    ( 0 == strTok.Find( _("LSTVAR") ) ) ) {

        CVSCPVariable variable;
        wxString strvalue;
        wxString strSearch;

        // Must be authorised to do this
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
            
            wxstr = wxString::Format( _("-;LSTVAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                        WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
   
            gpobj->logMsg ( _("[Websocket] User/host not authorised to list variable(s).\n"),
                                        DAEMON_LOGMSG_NORMAL,
                                        DAEMON_LOGTYPE_SECURITY );

            return;     // We still leave channel open
        }

        // Check privilege
        if ( (pSession->m_pClientItem->m_pUserItem->getUserRights( 0 ) & 0xf) < 4 ) {
            
            wxstr = wxString::Format( _("-;LSTVAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,
                                        WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
     
            wxString strErr =
                        wxString::Format( _("[Websocket] User [%s] not allowed to list variable(s).\n"),
                                                pSession->m_pClientItem->m_pUserItem->getUserName().mbc_str() );

            gpobj->logMsg ( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_SECURITY );
            return;     // We still leave channel open
        }

        int i = 0;
        wxString strResult;
        wxString strWork;
        gpobj->m_variableMutex.Lock();
        
        if ( tkz.HasMoreTokens() ) {
            strSearch = tkz.GetNextToken();
            strSearch.Trim();
            if ( strSearch.IsEmpty() ) {
                strSearch = _("(.*)");  // list all
            }
        }
        else {
            strSearch = _("(.*)");      // List all
        }
        
        wxString wxstr;
        wxArrayString arrayVars;
        gpobj->m_variables.getVarlistFromRegExp( arrayVars, strSearch );
    
        if ( arrayVars.Count() ) {
            
            // +;LSTVAR;ordinal;cnt;name;type;userid;accessrights;persistance;last_change
            for ( int i=0; i<arrayVars.Count(); i++ ) {
                if ( 0 != gpobj->m_variables.find( arrayVars[ i ], variable ) ) {
                    
                    wxstr = wxString::Format( _("+;LSTVAR;%d;%zu;"), i, arrayVars.Count() );
                    wxstr += variable.getAsString();
                    
                    web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT, 
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.Length() );
                }
            }
    
        }
        else {
            
            wxstr = wxString::Format( _("-;LSTVAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_VARIABLE_UNKNOWN,
                                        WEBSOCK_STR_ERROR_VARIABLE_UNKNOWN );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        }    
        
        
        gpobj->m_variableMutex.Unlock();
        
        
/* TODO
        // Send count
        strResult = wxString::Format( _( "+;LISTVAR;%zu" ), m_gpobj->m_VSCP_Variables.m_listVariable.GetCount() );
        mg_printf_websocket_frame( nc, WEBSOCKET_OP_TEXT, ( const char * )strResult.mbc_str() );

        
        listVscpVariable::iterator it;
        for( it = m_gpobj->m_VSCP_Variables.m_listVariable.begin();
                    it != m_gpobj->m_VSCP_Variables.m_listVariable.end();
                    ++it ) {

            if ( NULL == ( pvar = *it ) ) continue;

            strResult = _("+;LISTVAR;");
            strResult += wxString::Format( _("%d;"), i++ );
            strResult += pvar->getName();
            strResult += _(";");
            strWork.Printf( _("%d"), pvar->getType() ) ;
            strResult += strWork;
            if ( pvar->isPersistent() ) {
                strResult += _(";true;");
            }
            else {
                strResult += _(";false;");
            }

            pvar->writeValueToString( strWork );
            strResult += strWork;

            mg_printf_websocket_frame( nc, WEBSOCKET_OP_TEXT, (const char *)strResult.mbc_str() );
        }

        m_gpobj->m_variableMutex.Unlock();
        */

    }
    
    
    ////////////////////////////////////////////////////////////////////////////
    //                             USER TABLES
    ////////////////////////////////////////////////////////////////////////////
    
    
    
    // ------------------------------------------------------------------------
    //                               CREATETABLE
    //-------------------------------------------------------------------------
    
    else if ( 0 == strTok.Find( _("TBL_CREATE") ) ) {

        // Must be authorised to do this
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
            
            wxstr = wxString::Format( _("-;TBL_CREATE;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                        WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
   
            gpobj->logMsg ( _("[Websocket] User/host not authorised to "
                              "create a table.\n"),
                                        DAEMON_LOGMSG_NORMAL,
                                        DAEMON_LOGTYPE_SECURITY );

            return;     // We still leave channel open
        }
        
        // Check privilege
        if ( (pSession->m_pClientItem->m_pUserItem->getUserRights( 0 ) & 0xf) < 4 ) {
            
            wxstr = wxString::Format( _("-;TBL_CREATE;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,
                                        WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
     
            wxString strErr =
                        wxString::Format( _("[Websocket] User [%s] not allowed "
                                            "to create a table.\n"),
                 pSession->m_pClientItem->m_pUserItem->getUserName().mbc_str() );

            gpobj->logMsg ( strErr, DAEMON_LOGMSG_NORMAL, DAEMON_LOGTYPE_SECURITY );
            return;     // We still leave channel open
        }
        
        wxString table_create_str;
        if ( tkz.HasMoreTokens() ) {
            table_create_str = tkz.GetNextToken();
        }
        else {
            // Must have XML table create expression
            wxstr = wxString::Format( _("-;TBL_CREATE;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            return;     // We still leave channel open
        }
        
        if ( !gpobj->m_userTableObjects.createTableFromString( table_create_str ) ) {
            wxstr = wxString::Format( _("-;TBL_CREATE;%d;%s"),
                                        (int)WEBSOCK_ERROR_TABLE_CREATE_FORMAT,
                                        WEBSOCK_STR_ERROR_TABLE_CREATE_FORMAT );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        }
           
        // New table created OK
        wxString strResult = _("+;TBL_CREATE;");
        strResult += strTok;
        strResult += _( ";" );
        strResult += _("Table ");
        strResult += _(" created OK");
        
        // Positive reply
        web_websocket_write( conn, 
                                WEB_WEBSOCKET_OPCODE_TEXT, 
                                (const char *)strResult.mbc_str(),
                                strResult.Length() );        

    }
    
    // ------------------------------------------------------------------------
    //                             TBL_DELETE
    //-------------------------------------------------------------------------
    
    else if ( 0 == strTok.Find(_("TBL_DELETE") ) ) {
        
        // Must be authorised to do this
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
            
            wxstr = wxString::Format( _("-;TBL_DELETE;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                        WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
   
            gpobj->logMsg ( _("[Websocket] User/host not authorised to delete a table.\n"),
                                        DAEMON_LOGMSG_NORMAL,
                                        DAEMON_LOGTYPE_SECURITY );

            return;     // We still leave channel open
        }
        
        wxString strTable;                
        if ( tkz.HasMoreTokens() ) {
            strTable = tkz.GetNextToken();
        }
        else {
            // Must have XML table create expression
            wxstr = wxString::Format( _("-;TBL_DELETE;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            return;     // We still leave channel open
        }
        
        // Should table db be removed 
        bool bRemoveFile = false;
        if ( tkz.HasMoreTokens() ) {
            wxString str = tkz.GetNextToken();
            if ( wxNOT_FOUND != str.Upper().Find("TRUE") ) {
                bRemoveFile = true;
            }
        }
    
        // Remove the table from the internal system
        if ( !gpobj->m_userTableObjects.removeTable( strTable ), bRemoveFile ) {
                
            wxstr = wxString::Format( _("-;TBL_DLETE;%d;%s"),
                                        (int)WEBSOCK_ERROR_TABLE_DELETE_FAILED,
                                        WEBSOCK_STR_ERROR_TABLE_DELETE_FAILED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        }
           
        // New table created OK
        wxString strResult = _("+;TBL_DELETE;");
        strResult += strTok;
        strResult += _( ";" );
        strResult += _("Table ");
        strResult += strTable;
        strResult += _(" deleted OK");
        
        // Positive reply
        web_websocket_write( conn, 
                                WEB_WEBSOCKET_OPCODE_TEXT, 
                                (const char *)strResult.mbc_str(),
                                strResult.Length() );
    }
    
    // ------------------------------------------------------------------------
    //                             TBL_LIST
    //-------------------------------------------------------------------------
    
    else if ( 0 == strTok.Find(_("TBL_LIST") ) ) {
        
        // Must be authorised to do this
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
            
            wxstr = wxString::Format( _("-;TBL_LIST;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                        WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
   
            gpobj->logMsg ( _("[Websocket] User/host not authorised to list tables.\n"),
                                        DAEMON_LOGMSG_NORMAL,
                                        DAEMON_LOGTYPE_SECURITY );

            return;     // We still leave channel open
        }
        
        wxString strTable;                
        if ( tkz.HasMoreTokens() ) {
            strTable = tkz.GetNextToken();
            strTable.Trim();
        }
        
        if ( strTable.Length() ) {
            
            // list without argument - list all defined tables
            
            wxArrayString arrayTblNames;
            gpobj->m_mutexUserTables.Lock();
            if ( gpobj->m_userTableObjects.getTableNames( arrayTblNames ) ) {
            
                // OK
        
                for ( int i=0; i<arrayTblNames.Count(); i++ ) {
                
                    CVSCPTable *pTable = 
                        gpobj->m_userTableObjects.getTable( arrayTblNames[i] );
                
                    wxString str = 
                            wxString::Format( _("+;%i;%zu;"), 
                                                i, 
                                                arrayTblNames.Count() );
                    str += arrayTblNames[i];
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
                    
                    // Output row
                    web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.mbc_str(),
                                    str.length() );
                    
                }
                       
            }
            else {
            
                // Failed
                
                wxstr = wxString::Format( _("-;TBL_LIST;%d;%s"),
                                            (int)WEBSOCK_ERROR_TABLE_LIST_FAILED,
                                            WEBSOCK_STR_ERROR_TABLE_LIST_FAILED );
            
                web_websocket_write( conn, 
                                        WEB_WEBSOCKET_OPCODE_TEXT,
                                        (const char *)wxstr.mbc_str(),
                                        wxstr.length() );
                
            }
            
            gpobj->m_mutexUserTables.Unlock();
            
        }
        else {
            
            // list with argument - list specs for specific table
            
            wxArrayString arrayTblInfo;
            
            gpobj->m_mutexUserTables.Lock();

            CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
            if ( NULL == pTable ) {
                // Failed
                wxstr = wxString::Format( _("-;TBL_LIST;%d;%s"),
                                            (int)WEBSOCK_ERROR_TABLE_LIST_FAILED,
                                            WEBSOCK_STR_ERROR_TABLE_LIST_FAILED );
            
                web_websocket_write( conn, 
                                        WEB_WEBSOCKET_OPCODE_TEXT,
                                        (const char *)wxstr.mbc_str(),
                                        wxstr.length() );
            }   
            else {
            
                wxString str;                  

                // Name
                arrayTblInfo.Add( pTable->getTableName() );

                // Enable
                arrayTblInfo.Add( pTable->isEnabled() ? _("true") : _("false") );

                // Type
                switch( pTable->getType() ) {
                    case VSCP_TABLE_DYNAMIC:
                        str = _("dynamic");
                        break;
                            
                    case VSCP_TABLE_STATIC:
                        str = _("static");
                        break;
                            
                    case VSCP_TABLE_MAX:
                        str = _("max");
                        break;
                            
                    default:
                        str = _("Error: Invalid type");
                        break;
                }
                arrayTblInfo.Add( str );
            
                // bMemory
                arrayTblInfo.Add( pTable->isInMemory() ? _("true") : _("false") );
                            
                // Size
                arrayTblInfo.Add( wxString::Format( _("%lu "), 
                                           (unsigned long)pTable->getSize() ) );
            
                // Owner
                CUserItem *pUserItem = pTable->getUserItem();
                if ( NULL == pUserItem ) {
                    str = _("ERROR");
                }
                else {
                    str = pUserItem->getUserName();
                }
                arrayTblInfo.Add( str );
            
                // Access rights
                arrayTblInfo.Add( wxString::Format( _("0x%0X "), 
                                    (int)pTable->getRights() ) );
            
                // Description
                arrayTblInfo.Add( pTable->getDescription() );
            
                // X-name
                arrayTblInfo.Add( pTable->getLabelX() );
            
                // Y-name
                arrayTblInfo.Add(  pTable->getLabelY() );
            
                // Title
                arrayTblInfo.Add( pTable->getTitle() );
            
                // note
                arrayTblInfo.Add( pTable->getNote() );
            
                // vscp-class
                arrayTblInfo.Add(  wxString::Format( _("%d"), 
                                                (int)pTable->getVSCPClass() ) );
            
                // vscp-type
                arrayTblInfo.Add(  wxString::Format( _("%d"), 
                                                (int)pTable->getVSCPType() ) );
            
                // vscp-sensor-index
                arrayTblInfo.Add(  wxString::Format( _("%d"), 
                                                (int)pTable->getVSCPSensorIndex() ) );
            
                // vscp-unit
                arrayTblInfo.Add(  wxString::Format( _("%d"), 
                                                (int)pTable->getVSCPUnit() ) );
            
                // vscp-zone
                arrayTblInfo.Add(  wxString::Format( _("%d"), 
                                                (int)pTable->getVSCPZone() ) );
            
                // vscp-subzone
                arrayTblInfo.Add(  wxString::Format( _("%d"), 
                                                (int)pTable->getVSCPSubZone() ) );
            
                // SQL-create
                arrayTblInfo.Add(  pTable->getSQLCreate() );
            
                // SQL-insert
                arrayTblInfo.Add(  pTable->getSQLInsert() ); 
            
                // SQL-delete
                arrayTblInfo.Add(  pTable->getSQLDelete() );
                        
                // Send response
                for ( int i=0; i<arrayTblInfo.Count(); i++  ) {                    
                    str = wxString::Format( _("+;%i;%zu;"), 
                                               i, 
                                               arrayTblInfo.Count() );
                    str += arrayTblInfo[i];
                    // Output row
                    web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.mbc_str(),
                                    str.length() );
                }
            
            }
            
            gpobj->m_mutexUserTables.Unlock();
            
        }
        
    
    }
    
    // ------------------------------------------------------------------------
    //                             TBL_GET
    //-------------------------------------------------------------------------
    
    else if ( 0 == strTok.Find(_("TBL_GET") ) ) {
        
        // Initialize date range to 'all'
        wxDateTime start;
        wxDateTime end;
        
        start.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
        end.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
        
        // Must be authorised to do this
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
            
            wxstr = wxString::Format( _("-;TBL_GET;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                        WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
   
            gpobj->logMsg ( _("[Websocket] User/host not authorised to get data from a table.\n"),
                                        DAEMON_LOGMSG_NORMAL,
                                        DAEMON_LOGTYPE_SECURITY );

            return;     // We still leave channel open
        }
        
        // Get table name
        wxString strTable;                
        if ( tkz.HasMoreTokens() ) {
            strTable = tkz.GetNextToken();
            strTable.Trim();
        }
        else {
            // Error: Need tablename
            wxstr = wxString::Format( _("-;TBL_GET;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        }
                      
        if ( tkz.HasMoreTokens() ) {
            start.ParseISOCombined( tkz.GetNextToken() );        
            if ( !start.IsValid() ) {
                // Invalid date time
                wxstr = wxString::Format( _("-;TBL_GET;%d;%s"),
                                            (int)WEBSOCK_ERROR_INVALID_DATE,
                                            WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                        WEB_WEBSOCKET_OPCODE_TEXT,
                                        (const char *)wxstr.mbc_str(),
                                        wxstr.length() );
            }
               
        }
        else {
            wxstr = wxString::Format( _("-;TBL_GET;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        }
                        
        if ( tkz.HasMoreTokens() ) {
            end.ParseISOCombined( tkz.GetNextToken() );
            if ( !end.IsValid() ) {
                // Invalid date time
                wxstr = wxString::Format( _("-;TBL_GET;%d;%s"),
                                            (int)WEBSOCK_ERROR_INVALID_DATE,
                                            WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                        WEB_WEBSOCKET_OPCODE_TEXT,
                                        (const char *)wxstr.mbc_str(),
                                        wxstr.length() );
            }
        }
        else {
            wxstr = wxString::Format( _("-;TBL_GET;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        }
        
        bool bAll;                
        if ( tkz.HasMoreTokens() ) {
            wxString str = tkz.GetNextToken();
            str.Trim();
            str.MakeUpper();
            if ( wxNOT_FOUND != str.Find("FULL") ) {
                bAll = true;
            }
        }
    
        gpobj->m_mutexUserTables.Lock();
    
        CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
        if ( NULL == pTable ) {
            
            // Failed
            wxstr = wxString::Format( _("-;TBL_GET;%d;%s"),
                                        (int)WEBSOCK_ERROR_TABLE_FAILED_TO_GET,
                                        WEBSOCK_STR_ERROR_TABLE_FAILED_TO_GET );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        
            gpobj->m_mutexUserTables.Unlock();
            return;
        }   

        sqlite3_stmt *ppStmt;
        if ( !pTable->prepareRangeOfData( start, end, &ppStmt, bAll ) ) {
            
            // Failed
            wxstr = wxString::Format( _("-;TBL_GET;%d;%s"),
                                        (int)WEBSOCK_ERROR_TABLE_FAILED_GET_DATA,
                                        WEBSOCK_STR_ERROR_TABLE_FAILED_GET_DATA );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        
            gpobj->m_mutexUserTables.Unlock();
            return;
        }
    
        wxString str;
        wxArrayString strArray;
            while ( pTable->getRowRangeOfData( ppStmt, str ) ) {
            strArray.Add( str );
        }
        
        if ( strArray.Count() ) {
            for ( int i=0; i<strArray.Count(); i++ ) {
                
                str = wxString::Format(_("+;%d;%zu;"), i, strArray.Count() );
                str += strArray[i];
                
                // Output row
                web_websocket_write( conn, 
                                        WEB_WEBSOCKET_OPCODE_TEXT,
                                        (const char *)str.mbc_str(),
                                        str.length() );
            }
        }    
    
        pTable->finalizeRangeOfData( ppStmt );   
        gpobj->m_mutexUserTables.Unlock();
                
    }
    
    // ------------------------------------------------------------------------
    //                             TBL_GETRAW
    //-------------------------------------------------------------------------
    
    else if ( 0 == strTok.Find(_("TBL_GETRAW") ) ) {
        
        // Initialize date range to 'all'
        wxDateTime start;
        wxDateTime end;
        
        start.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
        end.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
        
        // Must be authorised to do this
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
            
            wxstr = wxString::Format( _("-;TBL_GETRAW;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                        WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
   
            gpobj->logMsg ( _("[Websocket] User/host not authorised to get raw data from a table.\n"),
                                        DAEMON_LOGMSG_NORMAL,
                                        DAEMON_LOGTYPE_SECURITY );

            return;     // We still leave channel open
        }
        
        // Get table name
        wxString strTable;                
        if ( tkz.HasMoreTokens() ) {
            strTable = tkz.GetNextToken();
            strTable.Trim();
        }
        else {
            // Error: Need tablename
            wxstr = wxString::Format( _("-;TBL_GETRAW;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        }
                      
        if ( tkz.HasMoreTokens() ) {
            start.ParseISOCombined( tkz.GetNextToken() );        
            if ( !start.IsValid() ) {
                // Invalid date time
                wxstr = wxString::Format( _("-;TBL_GETRAW;%d;%s"),
                                            (int)WEBSOCK_ERROR_INVALID_DATE,
                                            WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                        WEB_WEBSOCKET_OPCODE_TEXT,
                                        (const char *)wxstr.mbc_str(),
                                        wxstr.length() );
            }
               
        }
        else {
            wxstr = wxString::Format( _("-;TBL_GETRAW;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        }
                        
        if ( tkz.HasMoreTokens() ) {
            end.ParseISOCombined( tkz.GetNextToken() );
            if ( !end.IsValid() ) {
                // Invalid date time
                wxstr = wxString::Format( _("-;TBL_GETRAW;%d;%s"),
                                            (int)WEBSOCK_ERROR_INVALID_DATE,
                                            WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                        WEB_WEBSOCKET_OPCODE_TEXT,
                                        (const char *)wxstr.mbc_str(),
                                        wxstr.length() );
            }
        }
        else {
            wxstr = wxString::Format( _("-;TBL_GETRAW;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        }
        
        bool bAll;                
        if ( tkz.HasMoreTokens() ) {
            wxString str = tkz.GetNextToken();
            str.Trim();
            str.MakeUpper();
            if ( wxNOT_FOUND != str.Find("FULL") ) {
                bAll = true;
            }
        }
    
        gpobj->m_mutexUserTables.Lock();
    
        CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
        if ( NULL == pTable ) {
            
            // Failed
            wxstr = wxString::Format( _("-;TBL_GETRAW;%d;%s"),
                                        (int)WEBSOCK_ERROR_TABLE_FAILED_TO_GET,
                                        WEBSOCK_STR_ERROR_TABLE_FAILED_TO_GET );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        
            gpobj->m_mutexUserTables.Unlock();
            return;
        }   

        sqlite3_stmt *ppStmt;
        if ( !pTable->prepareRangeOfData( start, end, &ppStmt, bAll ) ) {
            
            // Failed
            wxstr = wxString::Format( _("-;TBL_GETRAW;%d;%s"),
                                        (int)WEBSOCK_ERROR_TABLE_FAILED_GET_DATA,
                                        WEBSOCK_STR_ERROR_TABLE_FAILED_GET_DATA );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        
            gpobj->m_mutexUserTables.Unlock();
            return;
        }
    
        wxString str;
        wxArrayString strArray;
            while ( pTable->getRowRangeOfDataRaw( ppStmt, str ) ) {
            strArray.Add( str );
        }
        
        if ( strArray.Count() ) {
            for ( int i=0; i<strArray.Count(); i++ ) {
                
                str = wxString::Format(_("+;%d;%zu;"), i, strArray.Count() );
                str += strArray[i];
                
                // Output row
                web_websocket_write( conn, 
                                        WEB_WEBSOCKET_OPCODE_TEXT,
                                        (const char *)str.mbc_str(),
                                        str.length() );
            }
        }    
    
        pTable->finalizeRangeOfData( ppStmt );   
        gpobj->m_mutexUserTables.Unlock();
    
    }
    
    // ------------------------------------------------------------------------
    //                             TBL_CLEAR
    //-------------------------------------------------------------------------
    
    else if ( 0 == strTok.Find(_("TBL_CLEAR") ) ) {
        
        // Initialize date range to 'all'
        wxDateTime start;
        wxDateTime end;
        
        start.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
        end.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
        
        // Must be authorised to do this
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
            
            wxstr = wxString::Format( _("-;TBL_CLEAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                        WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
   
            gpobj->logMsg ( _("[Websocket] User/host not authorised to log data to a table.\n"),
                                        DAEMON_LOGMSG_NORMAL,
                                        DAEMON_LOGTYPE_SECURITY );

            return;     // We still leave channel open
        }
        
        // Get table name
        wxString strTable;                
        if ( tkz.HasMoreTokens() ) {
            strTable = tkz.GetNextToken();
            strTable.Trim();
        }
        else {
            // Error: Need tablename
            wxstr = wxString::Format( _("-;TBL_CLEAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        }
                      
        bool bClearAll = true;
        if ( tkz.HasMoreTokens() ) {
            bClearAll = false;
            start.ParseISOCombined( tkz.GetNextToken() );        
            if ( !start.IsValid() ) {
                // Invalid date time
                wxstr = wxString::Format( _("-;TBL_CLEAR;%d;%s"),
                                            (int)WEBSOCK_ERROR_INVALID_DATE,
                                            WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                        WEB_WEBSOCKET_OPCODE_TEXT,
                                        (const char *)wxstr.mbc_str(),
                                        wxstr.length() );
            }
               
        }
                        
        if ( tkz.HasMoreTokens() ) {
            end.ParseISOCombined( tkz.GetNextToken() );
            if ( !end.IsValid() ) {
                // Invalid date time
                wxstr = wxString::Format( _("-;TBL_CLEAR;%d;%s"),
                                            (int)WEBSOCK_ERROR_INVALID_DATE,
                                            WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                        WEB_WEBSOCKET_OPCODE_TEXT,
                                        (const char *)wxstr.mbc_str(),
                                        wxstr.length() );
            }
        }
        
        
        gpobj->m_mutexUserTables.Lock();
    
        CVSCPTable *pTable = 
                        gpobj->m_userTableObjects.getTable( strTable );
    
        if ( NULL == pTable ) {
            // Failed
            wxstr = wxString::Format( _("-;TBL_CLEAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_TABLE_NOT_FOUND,
                                        WEBSOCK_STR_ERROR_TABLE_NOT_FOUND );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        
            gpobj->m_mutexUserTables.Unlock();
            return;
        } 
    
        if ( bClearAll ) {
            if ( !pTable->clearTable() ) {
                // Failed
                wxstr = wxString::Format( _("-;TBL_CLEAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_TABLE_FAILED_CLEAR,
                                        WEBSOCK_STR_ERROR_TABLE_FAILED_CLEAR );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
                gpobj->m_mutexUserTables.Unlock();
                return;
            }
        }
        else {
            if ( !pTable->clearTableRange( start, end ) ) {
                // Failed
                wxstr = wxString::Format( _("-;TBL_CLEAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_TABLE_FAILED_CLEAR,
                                        WEBSOCK_STR_ERROR_TABLE_FAILED_CLEAR );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
                gpobj->m_mutexUserTables.Unlock();
                return;
            }
        }
            
        gpobj->m_mutexUserTables.Unlock();
    
    }
    
    // ------------------------------------------------------------------------
    //                             TBL_LOG
    //-------------------------------------------------------------------------
    
    else if ( 0 == strTok.Find(_("TBL_LOG") ) ) {
        
        // Must be authorised to do this
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
            
            wxstr = wxString::Format( _("-;TBL_LOG;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                        WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
   
            gpobj->logMsg ( _("[Websocket] User/host not authorised to log data to a table.\n"),
                                        DAEMON_LOGMSG_NORMAL,
                                        DAEMON_LOGTYPE_SECURITY );

            return;     // We still leave channel open
        }
        
        // Get table name
        wxString strTable;                
        if ( tkz.HasMoreTokens() ) {
            strTable = tkz.GetNextToken();
            strTable.Trim();
        }
        else {
            // Error: Need tablename
            wxstr = wxString::Format( _("-;TBL_LOG;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        }
        
        // Get the value
        double value;
        if ( tkz.HasMoreTokens() ) {
            wxString str = tkz.GetNextToken();
            if ( !str.ToDouble( &value ) ) {
                // Problems: The value is not in a valid format
                wxstr = wxString::Format( _("-;TBL_LOG;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
                return;            
            }
        }
        else {
            // Problems: A value must be given 
            wxstr = wxString::Format( _("-;TBL_LOG;%d;%s"),
                                        (int)WEBSOCK_ERROR_TABLE_LOG_MISSING_VALUE,
                                        WEBSOCK_STR_ERROR_TABLE_LOG_MISSING_VALUE );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            return;
        }
    
        // Get the datetime if its there
        wxDateTime dt;
        if ( tkz.HasMoreTokens() ) {
        
            uint32_t ms = 0;
        
            wxString str = tkz.GetNextToken();
            str.Trim(true);
            str.Trim(false);
        
            if ( !dt.ParseISOCombined( str ) ) {
                // Problems: The value is not in a valid format 
                wxstr = wxString::Format( _("-;TBL_LOG;%d;%s"),
                                            (int)WEBSOCK_ERROR_INVALID_DATE,
                                            WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                        WEB_WEBSOCKET_OPCODE_TEXT,
                                        (const char *)wxstr.mbc_str(),
                                        wxstr.length() );
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
            wxstr = wxString::Format( _("-;TBL_LOG;%d;%s"),
                                            (int)WEBSOCK_ERROR_TABLE_NOT_FOUND,
                                            WEBSOCK_STR_ERROR_TABLE_NOT_FOUND );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        
            gpobj->m_mutexUserTables.Unlock();
            return;
        }
    
        // Log data
        if ( !pTable->logData( dt, value ) ) {
            gpobj->m_mutexUserTables.Unlock();
            wxstr = wxString::Format( _("-;TBL_LOG;%d;%s"),
                                            (int)WEBSOCK_ERROR_TABLE_LOG_FAILED,
                                            WEBSOCK_STR_ERROR_TABLE_LOG_FAILED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            return;
        }
    
        gpobj->m_mutexUserTables.Unlock();
    
    }
    
    // ------------------------------------------------------------------------
    //                             TBL_LOGSQL
    //-------------------------------------------------------------------------
    
    else if ( 0 == strTok.Find(_("TBL_LOGSQL") ) ) {
        
        // Must be authorised to do this
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
            
            wxstr = wxString::Format( _("-;TBL_LOGSQL;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                        WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
   
            gpobj->logMsg ( _("[Websocket] User/host not authorised to log sql data to a table.\n"),
                                        DAEMON_LOGMSG_NORMAL,
                                        DAEMON_LOGTYPE_SECURITY );

            return;     // We still leave channel open
        }
        
        // Get table name
        wxString strTable;                
        if ( tkz.HasMoreTokens() ) {
            strTable = tkz.GetNextToken();
            strTable.Trim();
        }
        else {
            // Error: Need tablename
            wxstr = wxString::Format( _("-;TBL_LOG;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        }
        
        // Get SQL expression 
        wxString strSQL;
        if ( tkz.HasMoreTokens() ) {
            strSQL = tkz.GetNextToken();
            strSQL.Trim(true);
            strSQL.Trim(false);
        }
        else {
            // Problems: A SQL expression must be given
            wxstr = wxString::Format( _("-;TBL_LOGSQL;%d;%s"),
                                        (int)WEBSOCK_ERROR_TABLE_NEED_SQL,
                                        WEBSOCK_STR_ERROR_TABLE_NEED_SQL );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            return;
        }

        gpobj->m_mutexUserTables.Lock();
    
        CVSCPTable *pTable = 
                    gpobj->m_userTableObjects.getTable( strTable );
    
        if ( NULL == pTable ) {
            // Failed
            wxstr = wxString::Format( _("-;TBL_LOGSQL;%d;%s"),
                                            (int)WEBSOCK_ERROR_TABLE_NOT_FOUND,
                                            WEBSOCK_STR_ERROR_TABLE_NOT_FOUND );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        
            gpobj->m_mutexUserTables.Unlock();
            return;
        }
    
        // Log data
        if ( !pTable->logData( strSQL ) ) {
            gpobj->m_mutexUserTables.Unlock();
            wxstr = wxString::Format( _("-;TBL_LOGSQL;%d;%s"),
                                            (int)WEBSOCK_ERROR_TABLE_LOG_FAILED,
                                            WEBSOCK_STR_ERROR_TABLE_LOG_FAILED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            return;
        }
    
        gpobj->m_mutexUserTables.Unlock();
    
    }
    
    // ------------------------------------------------------------------------
    //                             TBL_RECORDS
    //-------------------------------------------------------------------------
    
    else if ( 0 == strTok.Find(_("TBL_RECORDS") ) ) {
        
        wxDateTime wxStart;   
        wxDateTime wxEnd;
    
        // Initialize date range to 'all'
        wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
        wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
        
        // Must be authorised to do this
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
            
            wxstr = wxString::Format( _("-;TBL_RECORDS;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                        WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
   
            gpobj->logMsg ( _("[Websocket] User/host not authorised to do records on table.\n"),
                                        DAEMON_LOGMSG_NORMAL,
                                        DAEMON_LOGTYPE_SECURITY );

            return;     // We still leave channel open
        }
        
        // Get table name
        wxString strTable;                
        if ( tkz.HasMoreTokens() ) {
            strTable = tkz.GetNextToken();
            strTable.Trim();
        }
        else {
            // Error: Need tablename
            wxstr = wxString::Format( _("-;TBL_RECORDS;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        }
        
        // If available get Start data
        if ( tkz.HasMoreTokens() ) {
            if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
                wxstr = wxString::Format( _("-;TBL_RECORDS;%d;%s"),
                                        (int)WEBSOCK_ERROR_INVALID_DATE,
                                        WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            }
        }
        
        // If available get end date
        if ( tkz.HasMoreTokens() ) {
            if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
                wxstr = wxString::Format( _("-;TBL_RECORDS;%d;%s"),
                                        (int)WEBSOCK_ERROR_INVALID_DATE,
                                        WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            }
        }
        
        gpobj->m_mutexUserTables.Lock();
    
        CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
        if ( NULL == pTable ) {
            // Failed
            wxstr = wxString::Format( _("-;TBL_RECORDS;%d;%s"),
                                            (int)WEBSOCK_ERROR_TABLE_NOT_FOUND,
                                            WEBSOCK_STR_ERROR_TABLE_NOT_FOUND );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        
            gpobj->m_mutexUserTables.Unlock();
            return;
        } 
    
        double count;
        if ( !pTable->getNumberOfRecordsForRange( wxStart, wxEnd, &count ) ) { 
            wxstr = wxString::Format( _("-;TBL_RECORDS;%d;%s"),
                                            (int)WEBSOCK_ERROR_TABLE_FAILED_COMMAND_RECORDS,
                                            WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_RECORDS );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            gpobj->m_mutexUserTables.Unlock();
            return;
        }
    
        gpobj->m_mutexUserTables.Unlock();
        
        // Success
        wxstr = wxString::Format( _("+;TBL_RECORDS;%lf"), count );
    
        web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
    
    }
    
    
    // ------------------------------------------------------------------------
    //                              TBL_FIRSTDATE
    //-------------------------------------------------------------------------
    
    else if ( 0 == strTok.Find(_("TBL_FIRSTDATE") ) ) {
        
        wxDateTime wxStart;   
        wxDateTime wxEnd;
    
        // Initialize date range to 'all'
        wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
        wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
        
        // Must be authorised to do this
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
            
            wxstr = wxString::Format( _("-;TBL_FIRSTDATE;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                        WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
   
            gpobj->logMsg ( _("[Websocket] User/host not authorised to do "
                              "firstdate on table.\n"),
                                        DAEMON_LOGMSG_NORMAL,
                                        DAEMON_LOGTYPE_SECURITY );

            return;     // We still leave channel open
        }
        
        // Get table name
        wxString strTable;                
        if ( tkz.HasMoreTokens() ) {
            strTable = tkz.GetNextToken();
            strTable.Trim();
        }
        else {
            // Error: Need tablename
            wxstr = wxString::Format( _("-;TBL_FIRSTDATE;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        }
        
        // If available get Start data
        if ( tkz.HasMoreTokens() ) {
            if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
                wxstr = wxString::Format( _("-;TBL_FIRSTDATE;%d;%s"),
                                        (int)WEBSOCK_ERROR_INVALID_DATE,
                                        WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            }
        }
        
        // If available get end date
        if ( tkz.HasMoreTokens() ) {
            if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
                wxstr = wxString::Format( _("-;TBL_FIRSTDATE;%d;%s"),
                                        (int)WEBSOCK_ERROR_INVALID_DATE,
                                        WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            }
        }
        
        gpobj->m_mutexUserTables.Lock();
    
        CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
        if ( NULL == pTable ) {
            // Failed
            wxstr = wxString::Format( _("-;TBL_FIRSTDATE;%d;%s"),
                                            (int)WEBSOCK_ERROR_TABLE_NOT_FOUND,
                                            WEBSOCK_STR_ERROR_TABLE_NOT_FOUND );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        
            gpobj->m_mutexUserTables.Unlock();
            return;
        } 
        
        wxDateTime first;
        if ( !pTable->getFirstDate( first ) ) {
            wxstr = wxString::Format( _("-;TBL_FIRSTDATE;%d;%s"),
                                            (int)WEBSOCK_ERROR_TABLE_FAILED_COMMAND_FIRSTDATE,
                                            WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_FIRSTDATE );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            gpobj->m_mutexUserTables.Unlock();
            return;
        }
    
        gpobj->m_mutexUserTables.Unlock();
    
        // Success
        wxstr = wxString::Format( _("+;TBL_FIRSTDATE;%s"), 
                            (const char *)first.FormatISOCombined().mbc_str() );
    
        web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );    
    }
    
    // ------------------------------------------------------------------------
    //                              TBL_LASTDATE
    //-------------------------------------------------------------------------
    
    else if ( 0 == strTok.Find(_("TBL_LASTDATE") ) ) {
        
        wxDateTime wxStart;   
        wxDateTime wxEnd;
    
        // Initialize date range to 'all'
        wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
        wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
        
        // Must be authorised to do this
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
            
            wxstr = wxString::Format( _("-;TBL_LASTDATE;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                        WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
   
            gpobj->logMsg ( _("[Websocket] User/host not authorised to do "
                              "lastdate on table.\n"),
                                        DAEMON_LOGMSG_NORMAL,
                                        DAEMON_LOGTYPE_SECURITY );

            return;     // We still leave channel open
        }
        
        // Get table name
        wxString strTable;                
        if ( tkz.HasMoreTokens() ) {
            strTable = tkz.GetNextToken();
            strTable.Trim();
        }
        else {
            // Error: Need tablename
            wxstr = wxString::Format( _("-;TBL_LASTDATE;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        }
        
        // If available get Start data
        if ( tkz.HasMoreTokens() ) {
            if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
                wxstr = wxString::Format( _("-;TBL_LASTDATE;%d;%s"),
                                        (int)WEBSOCK_ERROR_INVALID_DATE,
                                        WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            }
        }
        
        // If available get end date
        if ( tkz.HasMoreTokens() ) {
            if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
                wxstr = wxString::Format( _("-;TBL_LASTDATE;%d;%s"),
                                        (int)WEBSOCK_ERROR_INVALID_DATE,
                                        WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            }
        }
        
        gpobj->m_mutexUserTables.Lock();
    
        CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
        if ( NULL == pTable ) {
            // Failed
            wxstr = wxString::Format( _("-;TBL_LASTDATE;%d;%s"),
                                            (int)WEBSOCK_ERROR_TABLE_NOT_FOUND,
                                            WEBSOCK_STR_ERROR_TABLE_NOT_FOUND );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        
            gpobj->m_mutexUserTables.Unlock();
            return;
        } 
    
        wxDateTime last;
        if ( !pTable->getLastDate( last ) ) {
            wxstr = wxString::Format( _("-;TBL_LASTDATE;%d;%s"),
                                            (int)WEBSOCK_ERROR_TABLE_FAILED_COMMAND_LASTDATE,
                                            WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_LASTDATE );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            gpobj->m_mutexUserTables.Unlock();
            return;
        }
    
        gpobj->m_mutexUserTables.Unlock();
    
        // Success
        wxstr = wxString::Format( _("+;TBL_LASTDATE;%s"), 
                            (const char *)last.FormatISOCombined().mbc_str() );
    
        web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );  
    }
    
    // ------------------------------------------------------------------------
    //                              TBL_SUM
    //-------------------------------------------------------------------------
    
    else if ( 0 == strTok.Find(_("TBL_SUM") ) ) {
        
        wxDateTime wxStart;   
        wxDateTime wxEnd;
    
        // Initialize date range to 'all'
        wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
        wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
        
        // Must be authorised to do this
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
            
            wxstr = wxString::Format( _("-;TBL_SUM;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                        WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
   
            gpobj->logMsg ( _("[Websocket] User/host not authorised to do sum "
                              "on table.\n"),
                                        DAEMON_LOGMSG_NORMAL,
                                        DAEMON_LOGTYPE_SECURITY );

            return;     // We still leave channel open
        }
        
        // Get table name
        wxString strTable;                
        if ( tkz.HasMoreTokens() ) {
            strTable = tkz.GetNextToken();
            strTable.Trim();
        }
        else {
            // Error: Need tablename
            wxstr = wxString::Format( _("-;TBL_SUM;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        }
        
        // If available get Start data
        if ( tkz.HasMoreTokens() ) {
            if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
                wxstr = wxString::Format( _("-;TBL_SUM;%d;%s"),
                                        (int)WEBSOCK_ERROR_INVALID_DATE,
                                        WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            }
        }
        
        // If available get end date
        if ( tkz.HasMoreTokens() ) {
            if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
                wxstr = wxString::Format( _("-;TBL_SUM;%d;%s"),
                                        (int)WEBSOCK_ERROR_INVALID_DATE,
                                        WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            }
        }
        
        gpobj->m_mutexUserTables.Lock();
    
        CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
        if ( NULL == pTable ) {
            // Failed
            wxstr = wxString::Format( _("-;TBL_SUM;%d;%s"),
                                            (int)WEBSOCK_ERROR_TABLE_NOT_FOUND,
                                            WEBSOCK_STR_ERROR_TABLE_NOT_FOUND );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        
            gpobj->m_mutexUserTables.Unlock();
            return;
        } 
        
        double sum;
        if ( !pTable->getSumValue( wxStart, wxEnd, &sum ) ) {
            wxstr = wxString::Format( _("-;TBL_SUM;%d;%s"),
                                            (int)WEBSOCK_ERROR_TABLE_FAILED_COMMAND_SUM,
                                            WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_SUM );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            gpobj->m_mutexUserTables.Unlock();
            return;
        }
    
        gpobj->m_mutexUserTables.Unlock();
    
        // Success
        wxstr = wxString::Format( _("+;TBL_SUM;%lf"), sum );
    
        web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() ); 
    
    }
    
    // ------------------------------------------------------------------------
    //                              TBL_MIN
    //-------------------------------------------------------------------------
    
    else if ( 0 == strTok.Find(_("TBL_MIN") ) ) {
        
        wxDateTime wxStart;   
        wxDateTime wxEnd;
    
        // Initialize date range to 'all'
        wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
        wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
        
        // Must be authorised to do this
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
            
            wxstr = wxString::Format( _("-;TBL_MIN;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                        WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
   
            gpobj->logMsg ( _("[Websocket] User/host not authorised to do min "
                              "on table.\n"),
                                        DAEMON_LOGMSG_NORMAL,
                                        DAEMON_LOGTYPE_SECURITY );

            return;     // We still leave channel open
        }
        
        // Get table name
        wxString strTable;                
        if ( tkz.HasMoreTokens() ) {
            strTable = tkz.GetNextToken();
            strTable.Trim();
        }
        else {
            // Error: Need tablename
            wxstr = wxString::Format( _("-;TBL_MIN;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        }
        
        // If available get Start data
        if ( tkz.HasMoreTokens() ) {
            if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
                wxstr = wxString::Format( _("-;TBL_MIN;%d;%s"),
                                        (int)WEBSOCK_ERROR_INVALID_DATE,
                                        WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            }
        }
        
        // If available get end date
        if ( tkz.HasMoreTokens() ) {
            if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
                wxstr = wxString::Format( _("-;TBL_MIN;%d;%s"),
                                        (int)WEBSOCK_ERROR_INVALID_DATE,
                                        WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            }
        }
        
        gpobj->m_mutexUserTables.Lock();
    
        CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
        if ( NULL == pTable ) {
            // Failed
            wxstr = wxString::Format( _("-;TBL_MIN;%d;%s"),
                                            (int)WEBSOCK_ERROR_TABLE_NOT_FOUND,
                                            WEBSOCK_STR_ERROR_TABLE_NOT_FOUND );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        
            gpobj->m_mutexUserTables.Unlock();
            return;
        } 
        
        double min;
        if ( !pTable->getMinValue( wxStart, wxEnd, &min ) ) {
            wxstr = wxString::Format( _("-;TBL_MIN;%d;%s"),
                                            (int)WEBSOCK_ERROR_TABLE_FAILED_COMMAND_MIN,
                                            WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_MIN );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            gpobj->m_mutexUserTables.Unlock();
            return;
        }
    
        gpobj->m_mutexUserTables.Unlock();
    
        // Success
        wxstr = wxString::Format( _("+;TBL_MIN;%lf"), min );
    
        web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
    
    }
    
    // ------------------------------------------------------------------------
    //                              TBL_MAX
    //-------------------------------------------------------------------------
    
    else if ( 0 == strTok.Find(_("TBL_MAX") ) ) {
        
        wxDateTime wxStart;   
        wxDateTime wxEnd;
    
        // Initialize date range to 'all'
        wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
        wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
        
        // Must be authorised to do this
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
            
            wxstr = wxString::Format( _("-;TBL_MAX;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                        WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
   
            gpobj->logMsg ( _("[Websocket] User/host not authorised to do "
                              "max on table.\n"),
                                        DAEMON_LOGMSG_NORMAL,
                                        DAEMON_LOGTYPE_SECURITY );

            return;     // We still leave channel open
        }
        
        // Get table name
        wxString strTable;                
        if ( tkz.HasMoreTokens() ) {
            strTable = tkz.GetNextToken();
            strTable.Trim();
        }
        else {
            // Error: Need tablename
            wxstr = wxString::Format( _("-;TBL_MAX;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        }
        
        // If available get Start data
        if ( tkz.HasMoreTokens() ) {
            if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
                wxstr = wxString::Format( _("-;TBL_MAX;%d;%s"),
                                        (int)WEBSOCK_ERROR_INVALID_DATE,
                                        WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            }
        }
        
        // If available get end date
        if ( tkz.HasMoreTokens() ) {
            if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
                wxstr = wxString::Format( _("-;TBL_MAX;%d;%s"),
                                        (int)WEBSOCK_ERROR_INVALID_DATE,
                                        WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            }
        }
        
        gpobj->m_mutexUserTables.Lock();
    
        CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
        if ( NULL == pTable ) {
            // Failed
            wxstr = wxString::Format( _("-;TBL_MAX;%d;%s"),
                                            (int)WEBSOCK_ERROR_TABLE_NOT_FOUND,
                                            WEBSOCK_STR_ERROR_TABLE_NOT_FOUND );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        
            gpobj->m_mutexUserTables.Unlock();
            return;
        } 
        
        double max;
        if ( !pTable->getMaxValue( wxStart, wxEnd, &max ) ) {
            wxstr = wxString::Format( _("-;TBL_MAX;%d;%s"),
                                            (int)WEBSOCK_ERROR_TABLE_FAILED_COMMAND_MAX,
                                            WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_MAX );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            gpobj->m_mutexUserTables.Unlock();
            return;
        }
    
        gpobj->m_mutexUserTables.Unlock();
    
        // Success
        wxstr = wxString::Format( _("+;TBL_MAX;%lf"), max );
    
        web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() ); 
    
    }
    
    // ------------------------------------------------------------------------
    //                              TBL_AVERAGE
    //-------------------------------------------------------------------------
    
    else if ( 0 == strTok.Find(_("TBL_AVERAGE") ) ) {
        
        wxDateTime wxStart;   
        wxDateTime wxEnd;
    
        // Initialize date range to 'all'
        wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
        wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
        
        // Must be authorised to do this
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
            
            wxstr = wxString::Format( _("-;TBL_AVERAGE;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                        WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
   
            gpobj->logMsg ( _("[Websocket] User/host not authorised to do "
                              "average on table.\n"),
                                        DAEMON_LOGMSG_NORMAL,
                                        DAEMON_LOGTYPE_SECURITY );

            return;     // We still leave channel open
        }
        
        // Get table name
        wxString strTable;                
        if ( tkz.HasMoreTokens() ) {
            strTable = tkz.GetNextToken();
            strTable.Trim();
        }
        else {
            // Error: Need tablename
            wxstr = wxString::Format( _("-;TBL_AVERAGE;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        }
        
        // If available get Start data
        if ( tkz.HasMoreTokens() ) {
            if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
                wxstr = wxString::Format( _("-;TBL_AVERAGE;%d;%s"),
                                        (int)WEBSOCK_ERROR_INVALID_DATE,
                                        WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            }
        }
        
        // If available get end date
        if ( tkz.HasMoreTokens() ) {
            if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
                wxstr = wxString::Format( _("-;TBL_AVERAGE;%d;%s"),
                                        (int)WEBSOCK_ERROR_INVALID_DATE,
                                        WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            }
        }
        
        gpobj->m_mutexUserTables.Lock();
    
        CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
        if ( NULL == pTable ) {
            // Failed
            wxstr = wxString::Format( _("-;TBL_AVERAGE;%d;%s"),
                                            (int)WEBSOCK_ERROR_TABLE_NOT_FOUND,
                                            WEBSOCK_STR_ERROR_TABLE_NOT_FOUND );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        
            gpobj->m_mutexUserTables.Unlock();
            return;
        }
        
        double average;
        if ( !pTable->getSumValue( wxStart, wxEnd, &average ) ) {
            wxstr = wxString::Format( _("-;TBL_AVERAGE;%d;%s"),
                                  (int)WEBSOCK_ERROR_TABLE_FAILED_COMMAND_AVERAGE,
                                  WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_AVERAGE );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            gpobj->m_mutexUserTables.Unlock();
            return;
        }
    
        gpobj->m_mutexUserTables.Unlock();
    
        // Success
        wxstr = wxString::Format( _("+;TBL_AVERAGE;%lf"), average );
    
        web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() ); 
    
    }
    
    // ------------------------------------------------------------------------
    //                              TBL_MEDIAN
    //-------------------------------------------------------------------------
    
    else if ( 0 == strTok.Find(_("TBL_MEDIAN") ) ) {
        
        wxDateTime wxStart;   
        wxDateTime wxEnd;
    
        // Initialize date range to 'all'
        wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
        wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
        
        // Must be authorised to do this
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
            
            wxstr = wxString::Format( _("-;TBL_MEDIAN;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                        WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
   
            gpobj->logMsg ( _("[Websocket] User/host not authorised to do "
                              "median on table.\n"),
                                        DAEMON_LOGMSG_NORMAL,
                                        DAEMON_LOGTYPE_SECURITY );

            return;     // We still leave channel open
        }
        
        // Get table name
        wxString strTable;                
        if ( tkz.HasMoreTokens() ) {
            strTable = tkz.GetNextToken();
            strTable.Trim();
        }
        else {
            // Error: Need tablename
            wxstr = wxString::Format( _("-;TBL_MEDIAN;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        }
        
        // If available get Start data
        if ( tkz.HasMoreTokens() ) {
            if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
                wxstr = wxString::Format( _("-;TBL_MEDIAN;%d;%s"),
                                        (int)WEBSOCK_ERROR_INVALID_DATE,
                                        WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            }
        }
        
        // If available get end date
        if ( tkz.HasMoreTokens() ) {
            if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
                wxstr = wxString::Format( _("-;TBL_MEDIAN;%d;%s"),
                                        (int)WEBSOCK_ERROR_INVALID_DATE,
                                        WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            }
        }
        
        gpobj->m_mutexUserTables.Lock();
    
        CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
        if ( NULL == pTable ) {
            // Failed
            wxstr = wxString::Format( _("-;TBL_MEDIAN;%d;%s"),
                                            (int)WEBSOCK_ERROR_TABLE_NOT_FOUND,
                                            WEBSOCK_STR_ERROR_TABLE_NOT_FOUND );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        
            gpobj->m_mutexUserTables.Unlock();
            return;
        } 
        
        double median;
        if ( !pTable->getMedianValue( wxStart, wxEnd, &median ) ) {
            wxstr = wxString::Format( _("-;TBL_MEDIAN;%d;%s"),
                                    (int)WEBSOCK_ERROR_TABLE_FAILED_COMMAND_MEDIAN,
                                    WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_MEDIAN );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            gpobj->m_mutexUserTables.Unlock();
            return;
        }
    
        gpobj->m_mutexUserTables.Unlock();
    
        // Success
        wxstr = wxString::Format( _("+;TBL_MEDIAN;%lf"), median );
    
        web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() ); 
    
    }
    
    // ------------------------------------------------------------------------
    //                              TBL_STDDEV
    //-------------------------------------------------------------------------
    
    else if ( 0 == strTok.Find(_("TBL_STDDEV") ) ) {
        
        wxDateTime wxStart;   
        wxDateTime wxEnd;
    
        // Initialize date range to 'all'
        wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
        wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
        
        // Must be authorised to do this
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
            
            wxstr = wxString::Format( _("-;TBL_STDDEV;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                        WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
   
            gpobj->logMsg ( _("[Websocket] User/host not authorised to do "
                              "stddev on table.\n"),
                                        DAEMON_LOGMSG_NORMAL,
                                        DAEMON_LOGTYPE_SECURITY );

            return;     // We still leave channel open
        }
        
        // Get table name
        wxString strTable;                
        if ( tkz.HasMoreTokens() ) {
            strTable = tkz.GetNextToken();
            strTable.Trim();
        }
        else {
            // Error: Need tablename
            wxstr = wxString::Format( _("-;TBL_STDDEV;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        }
        
        // If available get Start data
        if ( tkz.HasMoreTokens() ) {
            if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
                wxstr = wxString::Format( _("-;TBL_STDDEV;%d;%s"),
                                        (int)WEBSOCK_ERROR_INVALID_DATE,
                                        WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            }
        }
        
        // If available get end date
        if ( tkz.HasMoreTokens() ) {
            if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
                wxstr = wxString::Format( _("-;TBL_STDDEV;%d;%s"),
                                        (int)WEBSOCK_ERROR_INVALID_DATE,
                                        WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            }
        }
        
        gpobj->m_mutexUserTables.Lock();
    
        CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
        if ( NULL == pTable ) {
            // Failed
            wxstr = wxString::Format( _("-;TBL_STDDEV;%d;%s"),
                                            (int)WEBSOCK_ERROR_TABLE_NOT_FOUND,
                                            WEBSOCK_STR_ERROR_TABLE_NOT_FOUND );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        
            gpobj->m_mutexUserTables.Unlock();
            return;
        } 
        
        double stddev;
        if ( !pTable->getStdevValue( wxStart, wxEnd, &stddev ) ) {
            wxstr = wxString::Format( _("-;TBL_STDDEV;%d;%s"),
                                            (int)WEBSOCK_ERROR_TABLE_FAILED_COMMAND_STDDEV,
                                            WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_STDDEV );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            gpobj->m_mutexUserTables.Unlock();
            return;
        }
    
        gpobj->m_mutexUserTables.Unlock();
    
        // Success
        wxstr = wxString::Format( _("+;TBL_STDDEV;%lf"), stddev );
    
        web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() ); 
    
    }
    
    // ------------------------------------------------------------------------
    //                              TBL_VARIANCE
    //-------------------------------------------------------------------------
    
    else if ( 0 == strTok.Find(_("TBL_VARIANCE") ) ) {
        
        wxDateTime wxStart;   
        wxDateTime wxEnd;
    
        // Initialize date range to 'all'
        wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
        wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
        
        // Must be authorised to do this
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
            
            wxstr = wxString::Format( _("-;TBL_VARIANCE;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                        WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
   
            gpobj->logMsg ( _("[Websocket] User/host not authorised to do "
                              "variance on table.\n"),
                                        DAEMON_LOGMSG_NORMAL,
                                        DAEMON_LOGTYPE_SECURITY );

            return;     // We still leave channel open
        }
        
        // Get table name
        wxString strTable;                
        if ( tkz.HasMoreTokens() ) {
            strTable = tkz.GetNextToken();
            strTable.Trim();
        }
        else {
            // Error: Need tablename
            wxstr = wxString::Format( _("-;TBL_VARIANCE;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        }
        
        // If available get Start data
        if ( tkz.HasMoreTokens() ) {
            if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
                wxstr = wxString::Format( _("-;TBL_VARIANCE;%d;%s"),
                                        (int)WEBSOCK_ERROR_INVALID_DATE,
                                        WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            }
        }
        
        // If available get end date
        if ( tkz.HasMoreTokens() ) {
            if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
                wxstr = wxString::Format( _("-;TBL_VARIANCE;%d;%s"),
                                        (int)WEBSOCK_ERROR_INVALID_DATE,
                                        WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            }
        }
        
        gpobj->m_mutexUserTables.Lock();
    
        CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
        if ( NULL == pTable ) {
            // Failed
            wxstr = wxString::Format( _("-;TBL_VARIANCE;%d;%s"),
                                            (int)WEBSOCK_ERROR_TABLE_NOT_FOUND,
                                            WEBSOCK_STR_ERROR_TABLE_NOT_FOUND );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        
            gpobj->m_mutexUserTables.Unlock();
            return;
        } 
        
        double variance;
        if ( !pTable->getVarianceValue( wxStart, wxEnd, &variance ) ) {
            wxstr = wxString::Format( _("-;TBL_VARIANCE;%d;%s"),
                                            (int)WEBSOCK_ERROR_TABLE_FAILED_COMMAND_VARIANCE,
                                            WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_VARIANCE );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            gpobj->m_mutexUserTables.Unlock();
            return;
        }
    
        gpobj->m_mutexUserTables.Unlock();
    
        // Success
        wxstr = wxString::Format( _("+;TBL_VARIANCE;%lf"), variance );
    
        web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() ); 
    
    }
    
    // ------------------------------------------------------------------------
    //                              TBL_MODE
    //-------------------------------------------------------------------------
    
    else if ( 0 == strTok.Find(_("TBL_MODE") ) ) {
        
        wxDateTime wxStart;   
        wxDateTime wxEnd;
    
        // Initialize date range to 'all'
        wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
        wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
        
        // Must be authorised to do this
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
            
            wxstr = wxString::Format( _("-;TBL_MODE;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                        WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
   
            gpobj->logMsg ( _("[Websocket] User/host not authorised to do "
                              "mode on table.\n"),
                                        DAEMON_LOGMSG_NORMAL,
                                        DAEMON_LOGTYPE_SECURITY );

            return;     // We still leave channel open
        }
        
        // Get table name
        wxString strTable;                
        if ( tkz.HasMoreTokens() ) {
            strTable = tkz.GetNextToken();
            strTable.Trim();
        }
        else {
            // Error: Need tablename
            wxstr = wxString::Format( _("-;TBL_MODE;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        }
        
        // If available get Start data
        if ( tkz.HasMoreTokens() ) {
            if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
                wxstr = wxString::Format( _("-;TBL_MODE;%d;%s"),
                                        (int)WEBSOCK_ERROR_INVALID_DATE,
                                        WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            }
        }
        
        // If available get end date
        if ( tkz.HasMoreTokens() ) {
            if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
                wxstr = wxString::Format( _("-;TBL_MODE;%d;%s"),
                                        (int)WEBSOCK_ERROR_INVALID_DATE,
                                        WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            }
        }
        
        gpobj->m_mutexUserTables.Lock();
    
        CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
        if ( NULL == pTable ) {
            // Failed
            wxstr = wxString::Format( _("-;TBL_MODE;%d;%s"),
                                            (int)WEBSOCK_ERROR_TABLE_NOT_FOUND,
                                            WEBSOCK_STR_ERROR_TABLE_NOT_FOUND );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        
            gpobj->m_mutexUserTables.Unlock();
            return;
        } 
        
        double mode;
        if ( !pTable->getModeValue( wxStart, wxEnd, &mode ) ) {
            wxstr = wxString::Format( _("-;TBL_MODE;%d;%s"),
                                            (int)WEBSOCK_ERROR_TABLE_FAILED_COMMAND_MODE,
                                            WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_MODE );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            gpobj->m_mutexUserTables.Unlock();
            return;
        }
    
        gpobj->m_mutexUserTables.Unlock();
    
        // Success
        wxstr = wxString::Format( _("+;TBL_MODE;%lf"), mode );
    
        web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() ); 
    
    }    
    
    // ------------------------------------------------------------------------
    //                              TBL_LOWERQ
    //-------------------------------------------------------------------------
    
    else if ( 0 == strTok.Find(_("TBL_LOWERQ") ) ) {
        
        wxDateTime wxStart;   
        wxDateTime wxEnd;
    
        // Initialize date range to 'all'
        wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
        wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
        
        // Must be authorised to do this
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
            
            wxstr = wxString::Format( _("-;TBL_LOWERQ;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                        WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
   
            gpobj->logMsg ( _("[Websocket] User/host not authorised to do "
                              "lowerq on table.\n"),
                                        DAEMON_LOGMSG_NORMAL,
                                        DAEMON_LOGTYPE_SECURITY );

            return;     // We still leave channel open
        }
        
        // Get table name
        wxString strTable;                
        if ( tkz.HasMoreTokens() ) {
            strTable = tkz.GetNextToken();
            strTable.Trim();
        }
        else {
            // Error: Need tablename
            wxstr = wxString::Format( _("-;TBL_LOWERQ;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        }
        
        // If available get Start data
        if ( tkz.HasMoreTokens() ) {
            if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
                wxstr = wxString::Format( _("-;TBL_LOWERQ;%d;%s"),
                                        (int)WEBSOCK_ERROR_INVALID_DATE,
                                        WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            }
        }
        
        // If available get end date
        if ( tkz.HasMoreTokens() ) {
            if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
                wxstr = wxString::Format( _("-;TBL_LOWERQ;%d;%s"),
                                        (int)WEBSOCK_ERROR_INVALID_DATE,
                                        WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            }
        }
        
        gpobj->m_mutexUserTables.Lock();
    
        CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
        if ( NULL == pTable ) {
            // Failed
            wxstr = wxString::Format( _("-;TBL_LOWERQ;%d;%s"),
                                            (int)WEBSOCK_ERROR_TABLE_NOT_FOUND,
                                            WEBSOCK_STR_ERROR_TABLE_NOT_FOUND );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        
            gpobj->m_mutexUserTables.Unlock();
            return;
        } 
        
        double lowerq;
        if ( !pTable->getLowerQuartileValue( wxStart, wxEnd, &lowerq ) ) {
            wxstr = wxString::Format( _("-;TBL_LOWERQ;%d;%s"),
                                            (int)WEBSOCK_ERROR_TABLE_FAILED_COMMAND_LOWERQ,
                                            WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_LOWERQ );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            gpobj->m_mutexUserTables.Unlock();
            return;
        }
    
        gpobj->m_mutexUserTables.Unlock();
    
        // Success
        wxstr = wxString::Format( _("+;TBL_LOWERQ;%lf"), lowerq );
    
        web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() ); 
    
    }
    
    // ------------------------------------------------------------------------
    //                              TBL_UPPERQ
    //-------------------------------------------------------------------------
    
    else if ( 0 == strTok.Find(_("TBL_UPPERQ") ) ) {
        
        wxDateTime wxStart;   
        wxDateTime wxEnd;
    
        // Initialize date range to 'all'
        wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
        wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
        
        // Must be authorised to do this
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
            
            wxstr = wxString::Format( _("-;TBL_UPPERQ;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                        WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
   
            gpobj->logMsg ( _("[Websocket] User/host not authorised to do "
                              "upperq on table.\n"),
                                        DAEMON_LOGMSG_NORMAL,
                                        DAEMON_LOGTYPE_SECURITY );

            return;     // We still leave channel open
        }
        
        // Get table name
        wxString strTable;                
        if ( tkz.HasMoreTokens() ) {
            strTable = tkz.GetNextToken();
            strTable.Trim();
        }
        else {
            // Error: Need tablename
            wxstr = wxString::Format( _("-;TBL_UPPERQ;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        }
        
        // If available get Start data
        if ( tkz.HasMoreTokens() ) {
            if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
                wxstr = wxString::Format( _("-;TBL_UPPERQ;%d;%s"),
                                        (int)WEBSOCK_ERROR_INVALID_DATE,
                                        WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            }
        }
        
        // If available get end date
        if ( tkz.HasMoreTokens() ) {
            if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
                wxstr = wxString::Format( _("-;TBL_UPPERQ;%d;%s"),
                                        (int)WEBSOCK_ERROR_INVALID_DATE,
                                        WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            }
        }
        
        gpobj->m_mutexUserTables.Lock();
    
        CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
        if ( NULL == pTable ) {
            // Failed
            wxstr = wxString::Format( _("-;TBL_UPPERQ;%d;%s"),
                                            (int)WEBSOCK_ERROR_TABLE_NOT_FOUND,
                                            WEBSOCK_STR_ERROR_TABLE_NOT_FOUND );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        
            gpobj->m_mutexUserTables.Unlock();
            return;
        } 
        
        double upperq;
        if ( !pTable->getUppeQuartileValue( wxStart, wxEnd, &upperq ) ) {
            wxstr = wxString::Format( _("-;TBL_UPPERQ;%d;%s"),
                                            (int)WEBSOCK_ERROR_TABLE_FAILED_COMMAND_UPPERQ,
                                            WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_UPPERQ );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            gpobj->m_mutexUserTables.Unlock();
            return;
        }
    
        gpobj->m_mutexUserTables.Unlock();
    
        // Success
        wxstr = wxString::Format( _("+;TBL_UPPERQ;%lf"), upperq );
    
        web_websocket_write( conn, 
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)wxstr.mbc_str(),
                                wxstr.length() ); 
    
    }
    
    // ------------------------------------------------------------------------
    //                              TBL_CLEAR
    //-------------------------------------------------------------------------
    
    else if ( 0 == strTok.Find(_("TBL_CLEAR") ) ) {
        
        wxDateTime wxStart;   
        wxDateTime wxEnd;
    
        // Initialize date range to 'all'
        wxStart.ParseISOCombined( _("0000-01-01T00:00:00") );   // The first date
        wxEnd.ParseISOCombined( _("9999-12-31T23:59:59") );     // The last date
        
        // Must be authorised to do this
        if ( ( NULL == pSession->m_pClientItem ) || 
                !pSession->m_pClientItem->bAuthenticated ) {
            
            wxstr = wxString::Format( _("-;TBL_CLEAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                        WEBSOCK_STR_ERROR_NOT_AUTHORISED );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
   
            gpobj->logMsg ( _("[Websocket] User/host not authorised to clear "
                              "table.\n"),
                                        DAEMON_LOGMSG_NORMAL,
                                        DAEMON_LOGTYPE_SECURITY );

            return;     // We still leave channel open
        }
        
        // Get table name
        wxString strTable;                
        if ( tkz.HasMoreTokens() ) {
            strTable = tkz.GetNextToken();
            strTable.Trim();
        }
        else {
            // Error: Need tablename
            wxstr = wxString::Format( _("-;TBL_CLEAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                        WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        }
        
        bool bClearAll = false;
        
        // If available get Start data
        if ( tkz.HasMoreTokens() ) {
            if ( !wxStart.ParseISOCombined( tkz.GetNextToken() ) ) {
                wxstr = wxString::Format( _("-;TBL_CLEAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_INVALID_DATE,
                                        WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
            }
        }
        else {
            bClearAll = true;
        }
        
        // If available get end date
        if ( tkz.HasMoreTokens() ) {
            if ( !wxEnd.ParseISOCombined( tkz.GetNextToken() ) ) {
                wxstr = wxString::Format( _("-;TBL_CLEAR;%d;%s"),
                                            (int)WEBSOCK_ERROR_INVALID_DATE,
                                            WEBSOCK_STR_ERROR_INVALID_DATE );
            
                web_websocket_write( conn, 
                                        WEB_WEBSOCKET_OPCODE_TEXT,
                                        (const char *)wxstr.mbc_str(),
                                        wxstr.length() );
            }
        }
        
        gpobj->m_mutexUserTables.Lock();
    
        CVSCPTable *pTable = 
                gpobj->m_userTableObjects.getTable( strTable );
    
        if ( NULL == pTable ) {
            // Failed
            wxstr = wxString::Format( _("-;TBL_CLEAR;%d;%s"),
                                        (int)WEBSOCK_ERROR_TABLE_NOT_FOUND,
                                        WEBSOCK_STR_ERROR_TABLE_NOT_FOUND );
            
            web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
        
            gpobj->m_mutexUserTables.Unlock();
            return;
        } 
        
        if ( bClearAll ) {
            if ( !pTable->clearTable() ) {
                wxstr = wxString::Format( _("-;TBL_CLEAR;%d;%s"),
                                            (int)WEBSOCK_ERROR_TABLE_FAILED_COMMAND_CLEAR,
                                            WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_CLEAR );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
                gpobj->m_mutexUserTables.Unlock();
                return;
            }
        }
        else {
            if ( !pTable->clearTableRange( wxStart, wxEnd ) ) {
                wxstr = wxString::Format( _("-;TBL_CLEAR;%d;%s"),
                                            (int)WEBSOCK_ERROR_TABLE_FAILED_COMMAND_CLEAR,
                                            WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_CLEAR );
            
                web_websocket_write( conn, 
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)wxstr.mbc_str(),
                                    wxstr.length() );
                return;
            }
        }
            
        gpobj->m_mutexUserTables.Unlock();
            
        // Success
        wxstr = _("+;TBL_CLEAR;%lf");
    
        web_websocket_write( conn, 
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)wxstr.mbc_str(),
                                wxstr.length() ); 
    
    }
    
    // ------------------------------------------------------------------------
    //                              VERSION
    //-------------------------------------------------------------------------
    
    else if (0 == strTok.Find(_("VERSION"))) {
        
        wxString strvalue;

        wxString strResult = _("+;VERSION;");
        strResult += VSCPD_DISPLAY_VERSION;
        strResult += _(";");
        strResult += wxString::Format(_("%d.%d.%d.%d"), VSCPD_MAJOR_VERSION,
                                                            VSCPD_MINOR_VERSION,
                                                            VSCPD_RELEASE_VERSION,
                                                            VSCPD_BUILD_VERSION );
        // Positive reply
        web_websocket_write( conn, 
                                WEB_WEBSOCKET_OPCODE_TEXT, 
                                (const char *)strResult.mbc_str(),
                                strResult.Length() );
        
    }
    
    // ------------------------------------------------------------------------
    //                              COPYRIGHT
    //-------------------------------------------------------------------------
    
    else if (0 == strTok.Find(_("COPYRIGHT"))) {
        
        wxString strvalue;

        wxString strResult = _("+;COPYRIGHT;");
        strResult += VSCPD_COPYRIGHT;
        
        // Positive reply
        web_websocket_write( conn, 
                                WEB_WEBSOCKET_OPCODE_TEXT, 
                                (const char *)strResult.mbc_str(),
                                strResult.Length() );
        
    }

       
}
