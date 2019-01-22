// websocketserver.cpp
//
// This file is part of the VSCP (http://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2019 Ake Hedman, Grodans Paradis AB
// <info@grodansparadis.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
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

#define _POSIX

#include <fstream>
#include <iostream>
#include <sstream>

#include <arpa/inet.h>
#include <errno.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

#include "web_css.h"
#include "web_js.h"
#include "web_template.h"

#include <httpd.h>

#include <actioncodes.h>
#include <controlobject.h>
#include <devicelist.h>
#include <dm.h>
#include <mdf.h>
#include <tables.h>
#include <variablecodes.h>
#include <version.h>
#include <vscp.h>
#include <vscp_aes.h>
#include <vscphelper.h>
#include <websrv.h>

#include "websocket.h"

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
int
webserv_url_decode(const char *src,
                   int src_len,
                   char *dst,
                   int dst_len,
                   int is_form_url_encoded);

void
webserv_util_sendheader(struct mg_connection *nc,
                        const int returncode,
                        const char *content);

////////////////////////////////////////////////////
//            Forward declarations
////////////////////////////////////////////////////

void
ws1_command(struct web_connection *conn,
            struct websock_session *pSession,
            std::string &strCmd);

bool
ws1_message(struct web_connection *conn,
            websock_session *pSession,
            std::string &strWsPkt);

///////////////////////////////////////////////////
//                 WEBSOCKETS
///////////////////////////////////////////////////

// Linked list of websocket sessions
// Protected by the websocketSexxionMutex
// static struct websock_session *gp_websock_sessions;

websock_session::websock_session(void)
{
    m_conn       = NULL;
    m_conn_state = WEBSOCK_CONN_STATE_NULL;
    memset(m_websocket_key, 0, 33);
    memset(m_sid, 0, 33);
    m_version        = 0;
    lastActiveTime   = 0;
    m_pClientItem    = NULL;
    bEventTrigger    = false;
    triggerTimeout   = 0;
    bVariableTrigger = false;
};

websock_session::~websock_session(void){

};

///////////////////////////////////////////////////////////////////////////////
// websock_authentication
//
// client sends
//      "AUTH;iv;AES128("username:password)
//

bool
websock_authentication(struct web_connection *conn,
                       struct websock_session *pSession,
                       std::string &strIV,
                       std::string &strCrypto)
{
    uint8_t buf[2048], secret[2048];
    uint8_t iv[16];
    std::string strUser, strPassword;

    struct web_context *ctx;
    const struct web_request_info *reqinfo;
    bool bValidHost = false;

    // Check pointers
    if ((NULL == conn) || (NULL == pSession) ||
        !(ctx = web_get_context(conn)) ||
        !(reqinfo = web_get_request_info(conn))) {
        syslog(LOG_ERR,
               "[Websocket Client] Authentication: Invalid "
               "pointers. ");
        return false;
    }

    if (-1 == vscp_hexStr2ByteArray(iv, 16, (const char *)strIV.c_str())) {
        syslog(LOG_ERR,
               "[Websocket Client] Authentication: No room "
               "for iv block. ");
        return false; // Not enough room in buffer
    }

    size_t len;
    if (-1 == (len = vscp_hexStr2ByteArray(secret,
                                           strCrypto.length(),
                                           (const char *)strCrypto.c_str()))) {
        syslog(LOG_ERR,
               "[Websocket Client] Authentication: No room "
               "for crypto block. ");
        return false; // Not enough room in buffer
    }

    memset(buf, 0, sizeof(buf));
    AES_CBC_decrypt_buffer(AES128, buf, secret, len, gpobj->m_systemKey, iv);

    std::string str = std::string((const char *)buf);
    std::deque<std::string> tokens;
    vscp_split(tokens, str, ":");
    // std::stringTokenizer tkz( str, (":"), xxTOKEN_RET_EMPTY_ALL );  // TODO
    // ?????

    // Get username
    if (tokens.empty()) {
        syslog(LOG_ERR,
               "[Websocket Client] Authentication: Missing "
               "username from client. ");
        return false; // No username
    }

    strUser = tokens.front();
    tokens.pop_front();
    vscp_trim(strUser);

    // Get password
    if (tokens.empty()) {
        syslog(LOG_ERR,
               "[Websocket Client] Authentication: Missing "
               "password from client. ");
        return false; // No username
    }

    strPassword = tokens.front();
    tokens.pop_front();
    vscp_trim(strPassword);

    // Check if user is valid
    CUserItem *pUserItem = gpobj->m_userList.getUser(strUser);
    if (NULL == pUserItem) {
        syslog(LOG_ERR,
               "[Websocket Client] Authentication: CUserItem "
               "allocation problem ");
        return false;
    }

    // Check if remote ip is valid
    bValidHost = pUserItem->isAllowedToConnect(inet_addr(reqinfo->remote_addr));

    if (!bValidHost) {
        // Log valid login
        syslog(LOG_ERR,
               "[Websocket Client] Authentication: Host "
               "[%s] NOT allowed to connect.",
               reqinfo->remote_addr);
        return false;
    }

    if (!vscp_isPasswordValid(pUserItem->getPassword(), strPassword)) {
        syslog(LOG_ERR,
               "[Websocket Client] Authentication: User %s at host "
               "[%s] gave wrong password.",
               (const char *)strUser.c_str(),
               reqinfo->remote_addr);
        return false;
    }

    pSession->m_pClientItem->bAuthenticated = true;

    // Add user to client
    pSession->m_pClientItem->m_pUserItem = pUserItem;

    // Copy in the user filter
    memcpy(&pSession->m_pClientItem->m_filterVSCP,
           pUserItem->getFilter(),
           sizeof(vscpEventFilter));

    // Log valid login
    syslog(LOG_ERR,
           "[Websocket Client] Authentication: Host [%s] "
           "User [%s] allowed to connect.",
           reqinfo->remote_addr,
           (const char *)strUser.c_str());

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// websock_new_session
//

websock_session *
websock_new_session(const struct web_connection *conn)
{
    const char *pHeader;
    char ws_version[10];
    char ws_key[33];
    websock_session *pSession = NULL;

    // Check pointer
    if (NULL == conn) return NULL;

    // user
    memset(ws_version, 0, sizeof(ws_version));
    if (NULL != (pHeader = web_get_header(conn, "Sec-WebSocket-Version"))) {
        strncpy(ws_version,
                pHeader,
                std::min(strlen(pHeader) + 1, sizeof(ws_version)));
    }
    memset(ws_key, 0, sizeof(ws_key));
    if (NULL != (pHeader = web_get_header(conn, "Sec-WebSocket-Key"))) {
        strncpy(ws_key, pHeader, std::min(strlen(pHeader) + 1, sizeof(ws_key)));
    }

    // create fresh session
    pSession = new websock_session;
    if (NULL == pSession) {
        syslog(LOG_ERR,
               "[Websockets] New session: Unable to create session object.");
        return NULL;
    }

    // Generate the sid
    unsigned char iv[16];
    char hexiv[33];
    getRandomIV(iv, 16); // Generate 16 random bytes
    memset(hexiv, 0, sizeof(hexiv));
    vscp_byteArray2HexStr(hexiv, iv, 16);

    memset(pSession->m_sid, 0, sizeof(pSession->m_sid));
    memcpy(pSession->m_sid, hexiv, 32);
    memset(pSession->m_websocket_key, 0, sizeof(pSession->m_websocket_key));

    // Init.
    strcpy(pSession->m_websocket_key, ws_key); // Save key
    pSession->m_conn       = (struct web_connection *)conn;
    pSession->m_conn_state = WEBSOCK_CONN_STATE_CONNECTED;
    pSession->m_version    = atoi(ws_version); // Store protocol version

    pSession->m_pClientItem = new CClientItem(); // Create client
    if (NULL == pSession->m_pClientItem) {
        syslog(LOG_ERR,
               "[Websockets] New session: Unable to create client object.");
        delete pSession;
        return NULL;
    }

    pSession->m_pClientItem->bAuthenticated = false; // Not authenticated in yet
    vscp_clearVSCPFilter(
      &pSession->m_pClientItem->m_filterVSCP); // Clear filter
    pSession->bEventTrigger    = false;
    pSession->triggerTimeout   = 0;
    pSession->bVariableTrigger = false;

    // This is an active client
    pSession->m_pClientItem->m_bOpen = false;
    pSession->m_pClientItem->m_type =
      CLIENT_ITEM_INTERFACE_TYPE_CLIENT_WEBSOCKET;
    pSession->m_pClientItem->m_strDeviceName = ("Internal websocket client.");
    pSession->m_pClientItem->m_strDeviceName += ("|Started at ");
    vscpdatetime now = vscpdatetime::Now();
    pSession->m_pClientItem->m_strDeviceName += now.getISODateTime();

    // Add the client to the Client List
    pthread_mutex_lock(&gpobj->m_clientMutex);
    if (!gpobj->addClient(pSession->m_pClientItem)) {
        // Failed to add client
        delete pSession->m_pClientItem;
        pSession->m_pClientItem = NULL;
        pthread_mutex_unlock(&gpobj->m_clientMutex);
        syslog(LOG_ERR,
               ("Websocket server: Failed to add client. Terminating thread."));
        return NULL;
    }
    pthread_mutex_unlock(&gpobj->m_clientMutex);

    pthread_mutex_lock(&gpobj->m_websocketSessionMutex);
    gpobj->m_websocketSessions.push_back(pSession);
    pthread_mutex_unlock(&gpobj->m_websocketSessionMutex);

    // Use the session object as user data
    web_set_user_connection_data(pSession->m_conn, (void *)pSession);

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

    pthread_mutex_lock(&m_websockSessionMutex);

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
            pthread_mutex_lock(&pObject->m_clientMutex);
            pObject->removeClient( pos->m_pClientItem );
            pos->m_pClientItem = NULL;
            pthread_mutex_unlock(&pObject->m_clientMutex);

            // Free session data
            free( pos );

        }
        else {
            prev = pos;
        }

        pos = next;
    }

    pthread_mutex_unlock(&m_websockSessionMutex);
}
*/

///////////////////////////////////////////////////////////////////////////////
// websock_sendevent
//
// Send event to all other clients.
//

bool
websock_sendevent(struct web_connection *conn,
                  websock_session *pSession,
                  vscpEvent *pEvent)
{
    bool bSent = false;
    bool rv    = true;

    // Check pointer
    if (NULL == conn) return false;
    if (NULL == pSession) return false;

    // Level II events between 512-1023 is recognized by the daemon and
    // sent to the correct interface as Level I events if the interface
    // is addressed by the client.
    if ((pEvent->vscp_class <= 1023) && (pEvent->vscp_class >= 512) &&
        (pEvent->sizeData >= 16)) {

        // This event should be sent to the correct interface if it is
        // available on this machine. If not it should be sent to
        // the rest of the network as normal

        cguid destguid;
        destguid.getFromArray(pEvent->pdata);
        destguid.setAt(0, 0);
        destguid.setAt(1, 0);
        // unsigned char destGUID[16];  TODO ???
        // memcpy(destGUID, pEvent->pdata, 16); // get destination GUID
        // destGUID[0] = 0; // Interface GUID's have LSB bytes nilled
        // destGUID[1] = 0;

        pthread_mutex_lock(&gpobj->m_clientMutex);

        // Find client
        CClientItem *pDestClientItem = NULL;
        std::list<CClientItem *>::iterator it;
        for (it = gpobj->m_clientList.m_itemList.begin();
             it != gpobj->m_clientList.m_itemList.end();
             ++it) {

            CClientItem *pItem = *it;
            if (pItem->m_guid == destguid) {
                // Found
                pDestClientItem = pItem;
                break;
            }
        }

        if (NULL != pDestClientItem) {

            // If the client queue is full for this client then the
            // client will not receive the message
            if (pDestClientItem->m_clientInputQueue.size() <=
                gpobj->m_maxItemsInClientReceiveQueue) {

                // Create copy of event
                vscpEvent *pnewEvent = new vscpEvent;

                if (NULL != pnewEvent) {

                    pnewEvent->pdata = NULL;

                    vscp_copyVSCPEvent(pnewEvent, pEvent);

                    // Add the new event to the inputqueue
                    pthread_mutex_lock(
                      &pDestClientItem->m_mutexClientInputQueue);
                    pDestClientItem->m_clientInputQueue.push_back(pnewEvent);
                    sem_post(&pDestClientItem->m_semClientInputQueue);
                    pthread_mutex_unlock(
                      &pDestClientItem->m_mutexClientInputQueue);

                    bSent = true;
                } else {
                    bSent = false;
                }

            } else {
                // Overun - No room for event
                // vscp_deleteVSCPevent(pEvent);
                bSent = true;
                rv    = false;
            }

        } // Client found

        pthread_mutex_unlock(&gpobj->m_clientMutex);
    }

    if (!bSent) {

        // There must be room in the send queue
        if (gpobj->m_maxItemsInClientReceiveQueue >
            gpobj->m_clientOutputQueue.size()) {

            // Create copy of event
            vscpEvent *pnewEvent = new vscpEvent;

            if (NULL != pnewEvent) {

                pnewEvent->pdata = NULL;

                vscp_copyVSCPEvent(pnewEvent, pEvent);

                pthread_mutex_lock(&gpobj->m_mutexClientOutputQueue);
                gpobj->m_clientOutputQueue.push_back(pnewEvent);
                sem_post(&gpobj->m_semClientOutputQueue);
                pthread_mutex_unlock(&gpobj->m_mutexClientOutputQueue);
            }

        } else {
            rv = false;
        }
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// websocket_post_incomingEvent
//

void
websock_post_incomingEvents(void)
{
    pthread_mutex_lock(&gpobj->m_websocketSessionMutex);

    std::list<websock_session *>::iterator iter;
    for (iter = gpobj->m_websocketSessions.begin();
         iter != gpobj->m_websocketSessions.end();
         ++iter) {

        websock_session *pSession = *iter;
        if (NULL == pSession) continue;

        // Should be a client item... hmm.... client disconnected
        if (NULL == pSession->m_pClientItem) {
            continue;
        }

        if (pSession->m_conn_state < WEBSOCK_CONN_STATE_CONNECTED) continue;

        if (NULL == pSession->m_conn) continue;

        if (pSession->m_pClientItem->m_bOpen &&
            pSession->m_pClientItem->m_clientInputQueue.size()) {

            vscpEvent *pEvent;

            pthread_mutex_lock(
              &pSession->m_pClientItem->m_mutexClientInputQueue);
            pEvent = pSession->m_pClientItem->m_clientInputQueue.front();
            pSession->m_pClientItem->m_clientInputQueue.pop_front();
            pthread_mutex_unlock(
              &pSession->m_pClientItem->m_mutexClientInputQueue);

            if (NULL != pEvent) {

                // Run event through filter
                if (vscp_doLevel2Filter(
                      pEvent, &pSession->m_pClientItem->m_filterVSCP)) {

                    std::string str;
                    if (vscp_writeVscpEventToString(pEvent, str)) {

                        // Write it out
                        str = ("E;") + str;
                        web_websocket_write(pSession->m_conn,
                                            WEB_WEBSOCKET_OPCODE_TEXT,
                                            (const char *)str.c_str(),
                                            str.length());
                    }
                }

                // Remove the event
                vscp_deleteVSCPevent(pEvent);

            } // Valid pEvent pointer

        } // events available

    } // for

    pthread_mutex_unlock(&gpobj->m_websocketSessionMutex);
}

///////////////////////////////////////////////////////////////////////////////
// websock_post_variableTrigger
//
// op = 0 - Variable changed    "V"
// op = 1 - Variable created    "N"
// op = 2 - Variable deleted    "D"
//

void
websock_post_variableTrigger(uint8_t op, CVariable *pVar)
{
    pthread_mutex_lock(&gpobj->m_websocketSessionMutex);

    std::list<websock_session *>::iterator iter;
    for (iter = gpobj->m_websocketSessions.begin();
         iter != gpobj->m_websocketSessions.end();
         ++iter) {

        websock_session *pSession = *iter;
        if (NULL == pSession) continue;

        // Should be a client item... hmm.... client disconnected
        if (NULL == pSession->m_pClientItem) {
            continue;
        }

        if (pSession->m_conn_state < WEBSOCK_CONN_STATE_CONNECTED) continue;

        if (NULL == pSession->m_conn) continue;

        if (pSession->m_pClientItem->m_bOpen) {

            std::string outstr;
            outstr = "V;"; // Variable trigger
            web_websocket_write(pSession->m_conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)outstr.c_str(),
                                outstr.length());

        } // open

    } // for

    pthread_mutex_unlock(&gpobj->m_websocketSessionMutex);
}

////////////////////////////////////////////////////////////////////////////////
// ws1_connectHandler
//

int
ws1_connectHandler(const struct web_connection *conn, void *cbdata)
{
    struct web_context *ctx = web_get_context(conn);
    int reject              = 1;

    // Check pointers
    if (NULL == conn) return 1;
    if (NULL == ctx) return 1;

    web_lock_context(ctx);
    websock_session *pSession = websock_new_session(conn);

    if (NULL != pSession) {
        reject = 0;
    }
    web_unlock_context(ctx);

    syslog(LOG_ERR,
           "[Websocket] Connection: client %s",
           (reject ? "rejected" : "accepted"));

    return reject;
}

////////////////////////////////////////////////////////////////////////////////
// ws1_closeHandler
//

void
ws1_closeHandler(const struct web_connection *conn, void *cbdata)
{
    struct web_context *ctx = web_get_context(conn);
    websock_session *pSession =
      (websock_session *)web_get_user_connection_data(conn);

    if (NULL == conn) return;
    if (NULL == pSession) return;
    if (pSession->m_conn != conn) return;
    if (pSession->m_conn_state < WEBSOCK_CONN_STATE_CONNECTED) return;

    web_lock_context(ctx);

    // Record activity
    pSession->lastActiveTime = time(NULL);

    pSession->m_conn_state = WEBSOCK_CONN_STATE_NULL;
    pSession->m_conn       = NULL;
    gpobj->m_clientList.removeClient(pSession->m_pClientItem);
    pSession->m_pClientItem = NULL;

    pthread_mutex_lock(&gpobj->m_websocketSessionMutex);
    /*  TODO - Must remove session - gpobj->m_websocketSessions.DeleteContents(
    true ); if ( !gpobj->m_websocketSessions.DeleteObject( pSession )  ) {
        syslog( LOG_ERR,"[Websocket] Failed to delete session object.");
    }*/
    pthread_mutex_unlock(&gpobj->m_websocketSessionMutex);

    web_unlock_context(ctx);
}

////////////////////////////////////////////////////////////////////////////////
// ws1_readyHandler
//

void
ws1_readyHandler(struct web_connection *conn, void *cbdata)
{
    websock_session *pSession =
      (websock_session *)web_get_user_connection_data(conn);

    // Check pointers
    if (NULL == conn) return;
    if (NULL == pSession) return;
    if (pSession->m_conn != conn) return;
    if (pSession->m_conn_state < WEBSOCK_CONN_STATE_CONNECTED) return;

    // Record activity
    pSession->lastActiveTime = time(NULL);

    // Start authentication
    std::string str = vscp_str_format(("+;AUTH0;%s"), pSession->m_sid);
    web_websocket_write(
      conn, WEB_WEBSOCKET_OPCODE_TEXT, (const char *)str.c_str(), str.length());

    pSession->m_conn_state = WEBSOCK_CONN_STATE_DATA;
}

////////////////////////////////////////////////////////////////////////////////
// ws1_dataHandler
//

int
ws1_dataHandler(
  struct web_connection *conn, int bits, char *data, size_t len, void *cbdata)
{
    std::string strWsPkt;
    websock_session *pSession =
      (websock_session *)web_get_user_connection_data(conn);

    // Check pointers
    if (NULL == conn) return WEB_ERROR;
    if (NULL == pSession) return WEB_ERROR;
    if (pSession->m_conn != conn) return WEB_ERROR;
    if (pSession->m_conn_state < WEBSOCK_CONN_STATE_CONNECTED) return WEB_ERROR;

    // Record activity
    pSession->lastActiveTime = time(NULL);

    switch (((unsigned char)bits) & 0x0F) {

        case WEB_WEBSOCKET_OPCODE_CONTINUATION:

            // Save and concatenate mesage
            pSession->m_strConcatenated += std::string(data, len);

            // if last process is
            if (1 & bits) {
                if (!ws1_message(conn, pSession, pSession->m_strConcatenated)) {
                    return WEB_ERROR;
                }
            }
            break;

        // https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API/Writing_WebSocket_servers
        case WEB_WEBSOCKET_OPCODE_TEXT:
            if (1 & bits) {
                strWsPkt = std::string(data, len);
                if (!ws1_message(conn, pSession, strWsPkt)) {
                    return WEB_ERROR;
                }
            } else {
                // Store first part
                pSession->m_strConcatenated = std::string(data, len);
            }
            break;

        case WEB_WEBSOCKET_OPCODE_BINARY:
            break;

        case WEB_WEBSOCKET_OPCODE_CONNECTION_CLOSE:
            break;

        case WEB_WEBSOCKET_OPCODE_PING:
            // fprintf( stdout, "Ping received" );
            break;

        case WEB_WEBSOCKET_OPCODE_PONG:
            // fprintf( stdout, "Pong received" );
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
ws1_message(struct web_connection *conn,
            websock_session *pSession,
            std::string &strWsPkt)
{
    std::string str;

    // Check pointer
    if (NULL == conn) return false;
    if (NULL == pSession) return false;

    vscp_trim(strWsPkt);

    switch (strWsPkt[0]) {

        // Command - | 'C' | command type (byte) | data |
        case 'C':
            // Point beyond initial info "C;"
            strWsPkt = vscp_str_right(strWsPkt, strWsPkt.length() - 2);
            ws1_command(conn, pSession, strWsPkt);
            break;

        // Event | 'E' ; head(byte) , vscp_class(unsigned short) ,
        // vscp_type(unsigned
        //              short) , GUID(16*byte), data(0-487 bytes) |
        case 'E': {
            // Must be authorised to do this
            if ((NULL == pSession->m_pClientItem) ||
                !pSession->m_pClientItem->bAuthenticated) {

                str = vscp_str_format(("-;%d;%s"),
                                         (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                         WEBSOCK_STR_ERROR_NOT_AUTHORISED);
                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
                return true;
            }

            // Point beyond initial info "E;"
            strWsPkt = vscp_str_right(strWsPkt, strWsPkt.length() - 2);
            vscpEvent vscp_event;

            if (vscp_setVscpEventFromString(&vscp_event, strWsPkt)) {

                // If GUID is all null give it GUID of interface
                if (vscp_isGUIDEmpty(vscp_event.GUID)) {
                    pSession->m_pClientItem->m_guid.writeGUID(vscp_event.GUID);
                }

                // Check if this user is allowed to send this event
                if (!pSession->m_pClientItem->m_pUserItem
                       ->isUserAllowedToSendEvent(vscp_event.vscp_class,
                                                  vscp_event.vscp_type)) {
                    syslog(LOG_ERR,
                           "websocket] User [%s] not allowed to "
                           "send event class=%d type=%d.",
                           pSession->m_pClientItem->m_pUserItem->getUserName()
                             .c_str(),
                           vscp_event.vscp_class,
                           vscp_event.vscp_type);
                }

                vscp_event.obid = pSession->m_pClientItem->m_clientID;
                if (websock_sendevent(conn, pSession, &vscp_event)) {
                    web_websocket_write(
                      conn, WEB_WEBSOCKET_OPCODE_TEXT, "+;EVENT", 7);
                } else {
                    str = vscp_str_format(("-;%d;%s"),
                                             (int)WEBSOCK_ERROR_TX_BUFFER_FULL,
                                             WEBSOCK_STR_ERROR_TX_BUFFER_FULL);
                    web_websocket_write(conn,
                                        WEB_WEBSOCKET_OPCODE_TEXT,
                                        (const char *)str.c_str(),
                                        str.length());
                }
            }

        } break;

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
ws1_command(struct web_connection *conn,
            struct websock_session *pSession,
            std::string &strCmd)
{
    std::string str; // Worker string
    std::string strTok;

    // Check pointer
    if (NULL == conn) return;
    if (NULL == pSession) return;

    syslog(LOG_ERR, "[Websocket] Command = %s", strCmd.c_str());

    std::deque<std::string> tokens;
    vscp_split(tokens, strCmd, ";");

    // Get command
    if (!tokens.empty()) {
        strTok = tokens.front();
        tokens.pop_front();
        vscp_trim(strTok);
        vscp_makeUpper(strTok);
    } else {
        std::string str = vscp_str_format(("-;%d;%s"),
                                             (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                             WEBSOCK_STR_ERROR_SYNTAX_ERROR);
        web_websocket_write(conn,
                            WEB_WEBSOCKET_OPCODE_TEXT,
                            (const char *)str.c_str(),
                            str.length());
        return;
    }

    // ------------------------------------------------------------------------
    //                                NOOP
    //-------------------------------------------------------------------------

    if (vscp_startsWith(strTok, "NOOP")) {

        web_websocket_write(conn, WEB_WEBSOCKET_OPCODE_TEXT, "+;NOOP", 6);

        // Send authentication challenge
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {
            // TODO
        }

    }

    // ------------------------------------------------------------------------
    //                               CHALLENGE
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "CHALLENGE")) {

        // Send authentication challenge
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {

            // Start authentication
            str = vscp_str_format(("+;AUTH0;%s"), pSession->m_sid);
            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
        }

    }

    // ------------------------------------------------------------------------
    //                                AUTH
    //-------------------------------------------------------------------------

    // AUTH;iv;aes128
    else if (vscp_startsWith(strTok, "AUTH")) {

        std::string str;
        std::string strUser;
        std::string strIV = tokens.front();
        tokens.pop_front();
        std::string strCrypto = tokens.front();
        tokens.pop_front();
        if (websock_authentication(conn, pSession, strIV, strCrypto)) {
            std::string userSettings;
            pSession->m_pClientItem->m_pUserItem->getAsString(userSettings);
            str = vscp_str_format(("+;AUTH1;%s"),
                                     (const char *)userSettings.c_str());
            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
        } else {

            str = vscp_str_format(("-;AUTH;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                     WEBSOCK_STR_ERROR_NOT_AUTHORISED);
            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
            pSession->m_pClientItem->bAuthenticated = false; // Authenticated
        }
    }

    // ------------------------------------------------------------------------
    //                                OPEN
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "OPEN")) {

        // Must be authorised to do this
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {

            str = vscp_str_format(("-;OPEN;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                     WEBSOCK_STR_ERROR_NOT_AUTHORISED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            return; // We still leave channel open
        }

        pSession->m_pClientItem->m_bOpen = true;
        web_websocket_write(conn, WEB_WEBSOCKET_OPCODE_TEXT, "+;OPEN", 6);
    }

    // ------------------------------------------------------------------------
    //                                CLOSE
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "CLOSE")) {
        pSession->m_pClientItem->m_bOpen = false;
        web_websocket_write(conn, WEB_WEBSOCKET_OPCODE_TEXT, "+;CLOSE", 7);
    }

    // ------------------------------------------------------------------------
    //                             SETFILTER/SF
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "SETFILTER") ||
             vscp_startsWith(strTok, "SF")) {

        unsigned char ifGUID[16];
        memset(ifGUID, 0, 16);

        // Must be authorized to do this
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {

            str = vscp_str_format(("-;SF;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                     WEBSOCK_STR_ERROR_NOT_AUTHORISED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User/host not authorised to set a filter.");

            return; // We still leave channel open
        }

        // Check privilege
        if ((pSession->m_pClientItem->m_pUserItem->getUserRights(0) & 0xf) <
            6) {

            str = vscp_str_format(("-;SF;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,
                                     WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User [%s] not "
                   "allowed to set a filter.\n",
                   pSession->m_pClientItem->m_pUserItem->getUserName().c_str());
            return; // We still leave channel open
        }

        // Get filter
        if (!tokens.empty()) {

            strTok = tokens.front();
            tokens.pop_front();

            pthread_mutex_lock(
              &pSession->m_pClientItem->m_mutexClientInputQueue);
            if (!vscp_readFilterFromString(
                  &pSession->m_pClientItem->m_filterVSCP, strTok)) {

                str = vscp_str_format(("-;SF;%d;%s"),
                                         (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                         WEBSOCK_STR_ERROR_SYNTAX_ERROR);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());

                pthread_mutex_unlock(
                  &pSession->m_pClientItem->m_mutexClientInputQueue);
                return;
            }

            pthread_mutex_unlock(
              &pSession->m_pClientItem->m_mutexClientInputQueue);
        } else {

            str = vscp_str_format(("-;SF;%d;%s"),
                                     (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                     WEBSOCK_STR_ERROR_SYNTAX_ERROR);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            return;
        }

        // Get mask
        if (!tokens.empty()) {

            strTok = tokens.front();
            tokens.pop_front();

            pthread_mutex_lock(
              &pSession->m_pClientItem->m_mutexClientInputQueue);
            if (!vscp_readMaskFromString(&pSession->m_pClientItem->m_filterVSCP,
                                         strTok)) {

                str = vscp_str_format(("-;SF;%d;%s"),
                                         (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                         WEBSOCK_STR_ERROR_SYNTAX_ERROR);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());

                pthread_mutex_unlock(
                  &pSession->m_pClientItem->m_mutexClientInputQueue);
                return;
            }

            pthread_mutex_unlock(
              &pSession->m_pClientItem->m_mutexClientInputQueue);

        } else {
            str = vscp_str_format(("-;SF;%d;%s"),
                                     (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                     WEBSOCK_STR_ERROR_SYNTAX_ERROR);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
            return;
        }

        // Positive response
        web_websocket_write(conn, WEB_WEBSOCKET_OPCODE_TEXT, "+;SF", 4);

    }

    // ------------------------------------------------------------------------
    //                           CLRQ/CLRQUEUE
    //-------------------------------------------------------------------------

    // Clear the event queue
    else if (vscp_startsWith(strTok, "CLRQUEUE") ||
             vscp_startsWith(strTok, "CLRQ")) {

        // Must be authorised to do this
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {

            str = vscp_str_format(("-;CLRQ;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                     WEBSOCK_STR_ERROR_NOT_AUTHORISED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User/host not authorised to clear the queue.");

            return; // We still leave channel open
        }

        // Check privilege
        if ((pSession->m_pClientItem->m_pUserItem->getUserRights(0) & 0xf) <
            1) {

            str = vscp_str_format(("-;CLRQ;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,
                                     WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User [%s] "
                   "not allowed to clear the queue.\n",
                   pSession->m_pClientItem->m_pUserItem->getUserName().c_str());
            return; // We still leave channel open
        }

        std::deque<vscpEvent *>::iterator it;
        pthread_mutex_lock(&pSession->m_pClientItem->m_mutexClientInputQueue);

        for (it = pSession->m_pClientItem->m_clientInputQueue.begin();
             it != pSession->m_pClientItem->m_clientInputQueue.end();
             ++it) {
            vscpEvent *pEvent =
              pSession->m_pClientItem->m_clientInputQueue.front();
            pSession->m_pClientItem->m_clientInputQueue.pop_front();
            vscp_deleteVSCPevent(pEvent);
        }

        pSession->m_pClientItem->m_clientInputQueue.clear();
        pthread_mutex_unlock(&pSession->m_pClientItem->m_mutexClientInputQueue);

        web_websocket_write(conn, WEB_WEBSOCKET_OPCODE_TEXT, "+;CLRQ", 6);

    }

    ////////////////////////////////////////////////////////////////////////////
    //                              VARIABLES
    ////////////////////////////////////////////////////////////////////////////

    // ------------------------------------------------------------------------
    //                            CVAR/CREATEVAR
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "CREATEVAR") ||
             vscp_startsWith(strTok, "CVAR")) {

        std::string name;
        std::string value;
        std::string note;
        uint8_t type          = VSCP_DAEMON_VARIABLE_CODE_STRING;
        bool bPersistent      = false;
        uint32_t accessrights = 744;

        // Must be authorised to do this
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {

            str = vscp_str_format(("-;CVAR;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                     WEBSOCK_STR_ERROR_NOT_AUTHORISED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(
              LOG_ERR,
              "[Websocket] User/host not authorised to create a variable.");

            return; // We still leave channel open
        }

        // Check privilege
        if ((pSession->m_pClientItem->m_pUserItem->getUserRights(0) & 0xf) <
            6) {

            str = vscp_str_format(("-;CVAR;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,
                                     WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User [%s] not "
                   "allowed to create a variable.",
                   pSession->m_pClientItem->m_pUserItem->getUserName().c_str());
            return; // We still leave channel open
        }

        // “C;CVAR;name;type;accessrights;bPersistens;value;note”

        // Get variable name
        if (!tokens.empty()) {
            name = tokens.front();
            tokens.pop_front();
        } else {

            str = vscp_str_format(("-;CVAR;%d;%s"),
                                     (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                     WEBSOCK_STR_ERROR_SYNTAX_ERROR);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            return;
        }

        // Check if the variable exist
        CVariable variable;
        bool bVariableExist = false;
        if (0 != gpobj->m_variables.find(name, pSession->m_pClientItem->m_pUserItem, variable)) {
            bVariableExist = true;
        }

        // name can not start with "vscp." - reserved for a stock variable
        if (vscp_startsWith(vscp_lower(name), "vscp.")) {

            str = vscp_str_format(("-;CVAR;%d;%s"),
                                     (int)WEBSOCK_ERROR_VARIABLE_NO_STOCK,
                                     WEBSOCK_STR_ERROR_VARIABLE_NO_STOCK);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            return;
        }

        // Get variable type
        bool bInputType = false;
        if (!tokens.empty()) {
            std::string str = tokens.front();
            tokens.pop_front();
            vscp_trim(str);
            if (str.length()) bInputType = true;
            type = vscp_readStringValue(str);
        }

        // Get variable access rights
        bool bInputAccessRights = false;
        if (!tokens.empty()) {
            std::string str = tokens.front();
            tokens.pop_front();
            vscp_trim(str);
            if (str.length()) bInputAccessRights = true;
            accessrights = vscp_readStringValue(str);
        }

        // Get variable Persistence 0/1
        bool bInputPersistent = false;
        if (!tokens.empty()) {
            std::string str = tokens.front();
            tokens.pop_front();
            vscp_trim(str);
            if (str.length()) bInputPersistent = true;
            int val = vscp_readStringValue(str);

            if (0 == val) {
                bPersistent = false;
            } else if (1 == val) {
                bPersistent = true;
            } else {
                str = vscp_str_format(("-;CVAR;%d;%s"),
                                         (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                         WEBSOCK_STR_ERROR_SYNTAX_ERROR);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
                return;
            }
        }

        // Get variable value
        bool bInputValue = false;
        if (!tokens.empty()) {
            bInputValue = true;
            value       = tokens.front();
            tokens.pop_front();
            vscp_base64_std_decode(value);
        }

        // Get variable note
        bool bInputNote = false;
        if (!tokens.empty()) {
            bInputNote = true;
            note       = tokens.front();
            tokens.pop_front();
            vscp_base64_std_decode(value);
        }

        if (!bVariableExist) {

            // Add the variable
            if (!gpobj->m_variables.add(
                  name,
                  value,
                  type,
                  pSession->m_pClientItem->m_pUserItem->getUserID(),
                  bPersistent,
                  accessrights,
                  note)) {

                str = vscp_str_format(("-;CVAR;%d;%s"),
                                         (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                         WEBSOCK_STR_ERROR_SYNTAX_ERROR);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
                return;
            }
        } else {

            // Save changes to the variable

            if (bInputType) {
                variable.setType(type);
            }

            if (bInputAccessRights) {
                variable.setAccessRights(accessrights);
            }

            // Persistence can't be changed

            if (bInputValue) {
                variable.setValue(value);
            }

            if (bInputNote) {
                variable.setNote(note);
            }

            // Save the changed variable
            if (!gpobj->m_variables.update(variable,pSession->m_pClientItem->m_pUserItem)) {

                str = vscp_str_format(("-;CVAR;%d;%s"),
                                         (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                         WEBSOCK_STR_ERROR_SYNTAX_ERROR);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
                return;
            }
        }

        std::string strResult = ("+;CVAR;");
        strResult += name;

        web_websocket_write(conn,
                            WEB_WEBSOCKET_OPCODE_TEXT,
                            (const char *)strResult.c_str(),
                            strResult.length());

    }

    // ------------------------------------------------------------------------
    //                              RVAR/READVAR
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "READVAR") ||
             vscp_startsWith(strTok, "RVAR")) {

        CVariable variable;
        uint8_t type;
        std::string strvalue;

        // Must be authorised to do this
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {

            str = vscp_str_format(("-;RVAR;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                     WEBSOCK_STR_ERROR_NOT_AUTHORISED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User/host not authorised to read a variable.");

            return; // We still leave channel open
        }

        // Check privilege
        if ((pSession->m_pClientItem->m_pUserItem->getUserRights(0) & 0xf) <
            4) {

            str = vscp_str_format(("-;RVAR;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,
                                     WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User [%s] "
                   "not allowed to read a variable.",
                   pSession->m_pClientItem->m_pUserItem->getUserName().c_str());
            return; // We still leave channel open
        }

        strTok = tokens.front();
        tokens.pop_front();
        if (0 == gpobj->m_variables.find(strTok, pSession->m_pClientItem->m_pUserItem,variable)) {

            str = vscp_str_format(("-;RVAR;%d;%s"),
                                     (int)WEBSOCK_ERROR_VARIABLE_UNKNOWN,
                                     WEBSOCK_STR_ERROR_VARIABLE_UNKNOWN);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
            return;
        }

        // name;type;bPersistent;userid;rights;lastchanged;value;note
        std::string strResult = ("+;RVAR;");
        strResult += variable.getAsString(false);
        web_websocket_write(conn,
                            WEB_WEBSOCKET_OPCODE_TEXT,
                            (const char *)strResult.c_str(),
                            strResult.length());
    }

    // ------------------------------------------------------------------------
    //                                WVAR/WRITEVAR
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "WRITEVAR") ||
             vscp_startsWith(strTok, "WVAR")) {

        CVariable variable;
        std::string strvalue;
        uint8_t type;

        // Must be authorised to do this
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {

            str = vscp_str_format(("-;WVAR;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                     WEBSOCK_STR_ERROR_NOT_AUTHORISED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User/host not authorised to write a variable.");

            return; // We still leave channel open
        }

        // Check privilege
        if ((pSession->m_pClientItem->m_pUserItem->getUserRights(0) & 0xf) <
            6) {

            str = vscp_str_format(("-;WVAR;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,
                                     WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User [%s] not allowed to do write variable.\n",
                   pSession->m_pClientItem->m_pUserItem->getUserName().c_str());
            return; // We still leave channel open
        }

        // Get variable name
        std::string strVarName;
        if (!tokens.empty()) {

            strVarName = tokens.front();
            tokens.pop_front();
            if (0 ==
                gpobj->m_variables.find(vscp_upper(strVarName), pSession->m_pClientItem->m_pUserItem,variable)) {

                str = vscp_str_format(("-;WVAR;%d;%s"),
                                         (int)WEBSOCK_ERROR_VARIABLE_UNKNOWN,
                                         WEBSOCK_STR_ERROR_VARIABLE_UNKNOWN);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());

                return;
            }

            // Get variable value
            if (!tokens.empty()) {

                strTok = tokens.front();
                tokens.pop_front();

                if (!variable.setValueFromString(variable.getType(),
                                                 strTok,
                                                 true)) { // decode

                    str = vscp_str_format(("-;WVAR;%d;%s"),
                                             (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                             WEBSOCK_STR_ERROR_SYNTAX_ERROR);

                    web_websocket_write(conn,
                                        WEB_WEBSOCKET_OPCODE_TEXT,
                                        (const char *)str.c_str(),
                                        str.length());
                    return;
                }

                // Update the variable
                if (!gpobj->m_variables.update(variable,pSession->m_pClientItem->m_pUserItem)) {

                    str = vscp_str_format(("-;WVAR;%d;%s"),
                                             (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                             WEBSOCK_STR_ERROR_VARIABLE_UPDATE);

                    web_websocket_write(conn,
                                        WEB_WEBSOCKET_OPCODE_TEXT,
                                        (const char *)str.c_str(),
                                        str.length());
                    return;
                }
            } else {
                str = vscp_str_format(("-;WVAR;%d;%s"),
                                         (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                         WEBSOCK_STR_ERROR_SYNTAX_ERROR);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
                return;
            }
        } else {
            str = vscp_str_format(("-;WVAR;%d;%s"),
                                     (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                     WEBSOCK_STR_ERROR_SYNTAX_ERROR);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
            return;
        }

        std::string strResult = ("+;WVAR;");
        strResult += strVarName;

        // Positive reply
        web_websocket_write(conn,
                            WEB_WEBSOCKET_OPCODE_TEXT,
                            (const char *)strResult.c_str(),
                            strResult.length());

    }

    // ------------------------------------------------------------------------
    //                             RSTVAR/RESETVAR
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "RESETVAR") ||
             vscp_startsWith(strTok, "RSTVAR")) {

        CVariable variable;
        std::string strvalue;
        uint8_t type;

        // Must be authorised to do this
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {

            str = vscp_str_format(("-;RSTVAR;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                     WEBSOCK_STR_ERROR_NOT_AUTHORISED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            return; // We still leave channel open
        }

        // Check privilege
        if ((pSession->m_pClientItem->m_pUserItem->getUserRights(0) & 0xf) <
            6) {

            str = vscp_str_format(("-;RSTVAR;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,
                                     WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User [%s] not allowed to reset a variable.\n",
                   pSession->m_pClientItem->m_pUserItem->getUserName().c_str());
            return; // We still leave channel open
        }

        strTok = tokens.front();
        tokens.pop_front();
        if (0 == gpobj->m_variables.find(strTok, pSession->m_pClientItem->m_pUserItem,variable)) {

            str = vscp_str_format(("-;RSTVAR;%d;%s"),
                                     (int)WEBSOCK_ERROR_VARIABLE_UNKNOWN,
                                     WEBSOCK_STR_ERROR_VARIABLE_UNKNOWN);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User/host not "
                   "authorised to reset a variable.");

            return;
        }

        variable.reset();

        variable.writeValueToString(strvalue);
        type = variable.getType();

        std::string strResult = ("+;RSTVAR;");
        strResult += strTok;
        strResult += (";");
        strResult += vscp_str_format(("%d"), type);
        strResult += (";");
        strResult += strvalue;

        // Positive reply
        web_websocket_write(conn,
                            WEB_WEBSOCKET_OPCODE_TEXT,
                            (const char *)strResult.c_str(),
                            strResult.length());

    }

    // ------------------------------------------------------------------------
    //                                 DELVAR/REMOVEVAR
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "DELVAR") ||
             vscp_startsWith(strTok, "REMOVEVAR")) {

        CVariable variable;
        std::string name;

        // Must be authorised to do this
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {

            str = vscp_str_format(("-;DELVAR;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                     WEBSOCK_STR_ERROR_NOT_AUTHORISED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(
              LOG_ERR,
              "[Websocket] User/host not authorised to delete a variable.");

            return; // We still leave channel open
        }

        // Check privilege
        if ((pSession->m_pClientItem->m_pUserItem->getUserRights(0) & 0xf) <
            6) {

            str = vscp_str_format(("-;DELVAR;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,
                                     WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User [%s] "
                   "not allowed to delete a variable.",
                   pSession->m_pClientItem->m_pUserItem->getUserName().c_str());
            return; // We still leave channel open
        }

        name = tokens.front();
        tokens.pop_front();
        if (0 == gpobj->m_variables.find(name, pSession->m_pClientItem->m_pUserItem,variable)) {

            str = vscp_str_format(("-;DELVAR;%d;%s"),
                                     (int)WEBSOCK_ERROR_VARIABLE_UNKNOWN,
                                     WEBSOCK_STR_ERROR_VARIABLE_UNKNOWN);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            return;
        }

        pthread_mutex_lock(&gpobj->m_variableMutex);
        gpobj->m_variables.remove(name,pSession->m_pClientItem->m_pUserItem);
        pthread_mutex_unlock(&gpobj->m_variableMutex);

        std::string strResult = ("+;DELVAR;");
        strResult += name;

        // Positive reply
        web_websocket_write(conn,
                            WEB_WEBSOCKET_OPCODE_TEXT,
                            (const char *)strResult.c_str(),
                            strResult.length());

    }

    // ------------------------------------------------------------------------
    //                             LENVAR/LENGTHVAR
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "LENGTHVAR") ||
             vscp_startsWith(strTok, "LENVAR")) {

        CVariable variable;

        // Must be authorised to do this
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {

            str = vscp_str_format(("-;LENVAR;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                     WEBSOCK_STR_ERROR_NOT_AUTHORISED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User/host not "
                   "authorised to get length of variable.");

            return; // We still leave channel open
        }

        // Check privilege
        if ((pSession->m_pClientItem->m_pUserItem->getUserRights(0) & 0xf) <
            4) {

            str = vscp_str_format(("-;LENVAR;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,
                                     WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(
              LOG_ERR,
              "[Websocket] User [%s] not allowed to get length of variable.",
              pSession->m_pClientItem->m_pUserItem->getUserName().c_str());
            return; // We still leave channel open
        }

        strTok = tokens.front();
        tokens.pop_front();
        if (0 == gpobj->m_variables.find(strTok, pSession->m_pClientItem->m_pUserItem,variable)) {

            str = vscp_str_format(("-;LENVAR;%d;%s"),
                                     (int)WEBSOCK_ERROR_VARIABLE_UNKNOWN,
                                     WEBSOCK_STR_ERROR_VARIABLE_UNKNOWN);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
            return;
        }

        std::string strResult = ("+;LENVAR;");
        strResult += strTok;
        strResult += (";");
        strResult += vscp_str_format(("%d"), variable.getValue().length());

        // Positive reply
        web_websocket_write(conn,
                            WEB_WEBSOCKET_OPCODE_TEXT,
                            (const char *)strResult.c_str(),
                            strResult.length());

    }

    // ------------------------------------------------------------------------
    //                           LCVAR/LASTCHANGEVAR
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "LASTCHANGEVAR") ||
             vscp_startsWith(strTok, "LCVAR")) {

        CVariable variable;
        std::string strvalue;

        // Must be authorised to do this
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {

            str = vscp_str_format(("-;LCVAR;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                     WEBSOCK_STR_ERROR_NOT_AUTHORISED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User/host not authorised "
                   "to get last change date of variable.");

            return; // We still leave channel open
        }

        // Check privilege
        if ((pSession->m_pClientItem->m_pUserItem->getUserRights(0) & 0xf) <
            4) {

            str = vscp_str_format(("-;LCVAR;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,
                                     WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User [%s] not allowed "
                   "to get last change date of variable.\n",
                   pSession->m_pClientItem->m_pUserItem->getUserName().c_str());
            return; // We still leave channel open
        }

        strTok = tokens.front();
        tokens.pop_front();
        if (0 == gpobj->m_variables.find(strTok, pSession->m_pClientItem->m_pUserItem,variable)) {

            str = vscp_str_format(("-;LCVAR;%d;%s"),
                                     (int)WEBSOCK_ERROR_VARIABLE_UNKNOWN,
                                     WEBSOCK_STR_ERROR_VARIABLE_UNKNOWN);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            return;
        }

        variable.writeValueToString(strvalue);

        std::string strResult = ("+;LCVAR;");
        strResult += strTok;
        strResult += (";");
        strResult += variable.getLastChange().getISODateTime();

        // Positive reply
        web_websocket_write(conn,
                            WEB_WEBSOCKET_OPCODE_TEXT,
                            (const char *)strResult.c_str(),
                            strResult.length());

    }

    // ------------------------------------------------------------------------
    //                               LSTVAR/LISTVAR
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "LISTVAR") ||
             vscp_startsWith(strTok, "LSTVAR")) {

        CVariable variable;
        std::string strvalue;
        std::string strSearch;

        // Must be authorised to do this
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {

            str = vscp_str_format(("-;LSTVAR;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                     WEBSOCK_STR_ERROR_NOT_AUTHORISED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User/host not "
                   "authorised to list variable(s).");

            return; // We still leave channel open
        }

        // Check privilege
        if ((pSession->m_pClientItem->m_pUserItem->getUserRights(0) & 0xf) <
            4) {

            str = vscp_str_format(("-;LSTVAR;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,
                                     WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User [%s] "
                   "not allowed to list variable(s).",
                   pSession->m_pClientItem->m_pUserItem->getUserName().c_str());
            return; // We still leave channel open
        }

        int i = 0;
        std::string strResult;
        std::string strWork;
        pthread_mutex_lock(&gpobj->m_variableMutex);

        if (!tokens.empty()) {
            strSearch = tokens.front();
            tokens.pop_front();
            vscp_trim(strSearch);
            if (strSearch.empty()) {
                strSearch = ("(.*)"); // list all
            }
        } else {
            strSearch = ("(.*)"); // List all
        }

        std::string str;
        std::deque<std::string> arrayVars;
        gpobj->m_variables.getVarlistFromRegExp(arrayVars, strSearch);

        if (arrayVars.size()) {

            // +;LSTVAR;ordinal;cnt;name;type;userid;accessrights;persistance;last_change
            for (int i = 0; i < arrayVars.size(); i++) {
                if (0 != gpobj->m_variables.find(arrayVars[i], pSession->m_pClientItem->m_pUserItem,variable)) {

                    str = vscp_str_format(
                      ("+;LSTVAR;%d;%zu;"), i, arrayVars.size());
                    str += variable.getAsString();

                    web_websocket_write(conn,
                                        WEB_WEBSOCKET_OPCODE_TEXT,
                                        (const char *)str.c_str(),
                                        str.length());
                }
            }

        } else {

            str = vscp_str_format(("-;LSTVAR;%d;%s"),
                                     (int)WEBSOCK_ERROR_VARIABLE_UNKNOWN,
                                     WEBSOCK_STR_ERROR_VARIABLE_UNKNOWN);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
        }

        pthread_mutex_unlock(&gpobj->m_variableMutex);

        /* TODO
                // Send count
                strResult = vscp_str_format( ( "+;LISTVAR;%zu" ),
           m_gpobj->m_VSCP_Variables.m_listVariable.size() );
                mg_printf_websocket_frame( nc, WEBSOCKET_OP_TEXT, ( const char *
           )strResult.c_str() );


                listVscpVariable::iterator it;
                for( it = m_gpobj->m_VSCP_Variables.m_listVariable.begin();
                            it !=
           m_gpobj->m_VSCP_Variables.m_listVariable.end();
                            ++it ) {

                    if ( NULL == ( pvar = *it ) ) continue;

                    strResult = ("+;LISTVAR;");
                    strResult += vscp_str_format( ("%d;"), i++ );
                    strResult += pvar->getName();
                    strResult += (";");
                    strWork.Printf( ("%d"), pvar->getType() ) ;
                    strResult += strWork;
                    if ( pvar->isPersistent() ) {
                        strResult += (";true;");
                    }
                    else {
                        strResult += (";false;");
                    }

                    pvar->writeValueToString( strWork );
                    strResult += strWork;

                    mg_printf_websocket_frame( nc, WEBSOCKET_OP_TEXT, (const
           char *)strResult.c_str() );
                }

                pthread_mutex_unlock(&m_gpobj->m_variableMutex);
                */

    }

    ////////////////////////////////////////////////////////////////////////////
    //                             USER TABLES
    ////////////////////////////////////////////////////////////////////////////

    // ------------------------------------------------------------------------
    //                               CREATETABLE
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "TBL_CREATE")) {

        // Must be authorised to do this
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {

            str = vscp_str_format(("-;TBL_CREATE;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                     WEBSOCK_STR_ERROR_NOT_AUTHORISED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User/host not authorised to "
                   "create a table.");

            return; // We still leave channel open
        }

        // Check privilege
        if ((pSession->m_pClientItem->m_pUserItem->getUserRights(0) & 0xf) <
            4) {

            str = vscp_str_format(("-;TBL_CREATE;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_ALLOWED_TO_DO_THAT,
                                     WEBSOCK_STR_ERROR_NOT_ALLOWED_TO_DO_THAT);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User [%s] not allowed "
                   "to create a table.",
                   pSession->m_pClientItem->m_pUserItem->getUserName().c_str());
            return; // We still leave channel open
        }

        std::string table_create_str;
        if (!tokens.empty()) {
            table_create_str = tokens.front();
            tokens.pop_front();
        } else {
            // Must have XML table create expression
            str = vscp_str_format(("-;TBL_CREATE;%d;%s"),
                                     (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                     WEBSOCK_STR_ERROR_SYNTAX_ERROR);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
            return; // We still leave channel open
        }

        if (!gpobj->m_userTableObjects.createTableFromString(
              table_create_str)) {
            str = vscp_str_format(("-;TBL_CREATE;%d;%s"),
                                     (int)WEBSOCK_ERROR_TABLE_CREATE_FORMAT,
                                     WEBSOCK_STR_ERROR_TABLE_CREATE_FORMAT);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
        }

        // New table created OK
        std::string strResult = ("+;TBL_CREATE;");
        strResult += strTok;
        strResult += (";");
        strResult += ("Table ");
        strResult += (" created OK");

        // Positive reply
        web_websocket_write(conn,
                            WEB_WEBSOCKET_OPCODE_TEXT,
                            (const char *)strResult.c_str(),
                            strResult.length());

    }

    // ------------------------------------------------------------------------
    //                             TBL_DELETE
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "TBL_DELETE")) {

        // Must be authorised to do this
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {

            str = vscp_str_format(("-;TBL_DELETE;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                     WEBSOCK_STR_ERROR_NOT_AUTHORISED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User/host not authorised to delete a table.");

            return; // We still leave channel open
        }

        std::string strTable;
        if (!tokens.empty()) {
            strTable = tokens.front();
            tokens.pop_front();
        } else {
            // Must have XML table create expression
            str = vscp_str_format(("-;TBL_DELETE;%d;%s"),
                                     (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                     WEBSOCK_STR_ERROR_SYNTAX_ERROR);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
            return; // We still leave channel open
        }

        // Should table db be removed
        bool bRemoveFile = false;
        if (!tokens.empty()) {
            std::string str = tokens.front();
            tokens.pop_front();
            if (0 == vscp_strcasecmp(str.c_str(), "TRUE")) {
                bRemoveFile = true;
            }
        }

        // Remove the table from the internal system
        if (!gpobj->m_userTableObjects.removeTable(strTable), bRemoveFile) {

            str = vscp_str_format(("-;TBL_DLETE;%d;%s"),
                                     (int)WEBSOCK_ERROR_TABLE_DELETE_FAILED,
                                     WEBSOCK_STR_ERROR_TABLE_DELETE_FAILED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
        }

        // New table created OK
        std::string strResult = ("+;TBL_DELETE;");
        strResult += strTok;
        strResult += (";");
        strResult += ("Table ");
        strResult += strTable;
        strResult += (" deleted OK");

        // Positive reply
        web_websocket_write(conn,
                            WEB_WEBSOCKET_OPCODE_TEXT,
                            (const char *)strResult.c_str(),
                            strResult.length());
    }

    // ------------------------------------------------------------------------
    //                             TBL_LIST
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "TBL_LIST")) {

        // Must be authorised to do this
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {

            str = vscp_str_format(("-;TBL_LIST;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                     WEBSOCK_STR_ERROR_NOT_AUTHORISED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User/host not authorised to list tables.");

            return; // We still leave channel open
        }

        std::string strTable;
        if (!tokens.empty()) {
            strTable = tokens.front();
            tokens.pop_front();
            vscp_trim(strTable);
        }

        if (strTable.length()) {

            // list without argument - list all defined tables

            std::deque<std::string> arrayTblNames;
            pthread_mutex_lock(&gpobj->m_mutexUserTables);
            if (gpobj->m_userTableObjects.getTableNames(arrayTblNames)) {

                // OK

                for (int i = 0; i < arrayTblNames.size(); i++) {

                    CVSCPTable *pTable =
                      gpobj->m_userTableObjects.getTable(arrayTblNames[i]);

                    std::string str = vscp_str_format(
                      ("+;%i;%zu;"), i, arrayTblNames.size());
                    str += arrayTblNames[i];
                    str += (";");
                    if (NULL != pTable) {

                        switch (pTable->getType()) {

                            case VSCP_TABLE_DYNAMIC:
                                str += ("dynamic");
                                break;

                            case VSCP_TABLE_STATIC:
                                str += ("static");
                                break;

                            case VSCP_TABLE_MAX:
                                str += ("max");
                                break;

                            default:
                                str += ("Error: Invalid type");
                                break;
                        }
                    }
                    str += (";");
                    if (NULL != pTable) {
                        str += pTable->getDescription();
                    }

                    // Output row
                    web_websocket_write(conn,
                                        WEB_WEBSOCKET_OPCODE_TEXT,
                                        (const char *)str.c_str(),
                                        str.length());
                }

            } else {

                // Failed

                str = vscp_str_format(("-;TBL_LIST;%d;%s"),
                                         (int)WEBSOCK_ERROR_TABLE_LIST_FAILED,
                                         WEBSOCK_STR_ERROR_TABLE_LIST_FAILED);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }

            pthread_mutex_unlock(&gpobj->m_mutexUserTables);

        } else {

            // list with argument - list specs for specific table

            std::deque<std::string> arrayTblInfo;

            pthread_mutex_lock(&gpobj->m_mutexUserTables);

            CVSCPTable *pTable = gpobj->m_userTableObjects.getTable(strTable);
            if (NULL == pTable) {
                // Failed
                str = vscp_str_format(("-;TBL_LIST;%d;%s"),
                                         (int)WEBSOCK_ERROR_TABLE_LIST_FAILED,
                                         WEBSOCK_STR_ERROR_TABLE_LIST_FAILED);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            } else {

                std::string str;

                // Name
                arrayTblInfo.push_back(pTable->getTableName());

                // Enable
                arrayTblInfo.push_back(pTable->isEnabled() ? ("true")
                                                           : ("false"));

                // Type
                switch (pTable->getType()) {
                    case VSCP_TABLE_DYNAMIC:
                        str = ("dynamic");
                        break;

                    case VSCP_TABLE_STATIC:
                        str = ("static");
                        break;

                    case VSCP_TABLE_MAX:
                        str = ("max");
                        break;

                    default:
                        str = ("Error: Invalid type");
                        break;
                }
                arrayTblInfo.push_back(str);

                // bMemory
                arrayTblInfo.push_back(pTable->isInMemory() ? "true" : "false");

                // Size
                arrayTblInfo.push_back(
                  vscp_str_format("%lu ", (unsigned long)pTable->getSize()));

                // Owner
                CUserItem *pUserItem = pTable->getUserItem();
                if (NULL == pUserItem) {
                    str = ("ERROR");
                } else {
                    str = pUserItem->getUserName();
                }
                arrayTblInfo.push_back(str);

                // Access rights
                arrayTblInfo.push_back(
                  vscp_str_format(("0x%0X "), (int)pTable->getRights()));

                // Description
                arrayTblInfo.push_back(pTable->getDescription());

                // X-name
                arrayTblInfo.push_back(pTable->getLabelX());

                // Y-name
                arrayTblInfo.push_back(pTable->getLabelY());

                // Title
                arrayTblInfo.push_back(pTable->getTitle());

                // note
                arrayTblInfo.push_back(pTable->getNote());

                // vscp-class
                arrayTblInfo.push_back(
                  vscp_str_format(("%d"), (int)pTable->getVSCPClass()));

                // vscp-type
                arrayTblInfo.push_back(
                  vscp_str_format(("%d"), (int)pTable->getVSCPType()));

                // vscp-sensor-index
                arrayTblInfo.push_back(vscp_str_format(
                  ("%d"), (int)pTable->getVSCPSensorIndex()));

                // vscp-unit
                arrayTblInfo.push_back(
                  vscp_str_format(("%d"), (int)pTable->getVSCPUnit()));

                // vscp-zone
                arrayTblInfo.push_back(
                  vscp_str_format(("%d"), (int)pTable->getVSCPZone()));

                // vscp-subzone
                arrayTblInfo.push_back(
                  vscp_str_format(("%d"), (int)pTable->getVSCPSubZone()));

                // SQL-create
                arrayTblInfo.push_back(pTable->getSQLCreate());

                // SQL-insert
                arrayTblInfo.push_back(pTable->getSQLInsert());

                // SQL-delete
                arrayTblInfo.push_back(pTable->getSQLDelete());

                // Send response
                for (int i = 0; i < arrayTblInfo.size(); i++) {
                    str =
                      vscp_str_format(("+;%i;%zu;"), i, arrayTblInfo.size());
                    str += arrayTblInfo[i];
                    // Output row
                    web_websocket_write(conn,
                                        WEB_WEBSOCKET_OPCODE_TEXT,
                                        (const char *)str.c_str(),
                                        str.length());
                }
            }

            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
        }

    }

    // ------------------------------------------------------------------------
    //                             TBL_GET
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "TBL_GET")) {

        // Initialize date range to 'all'
        vscpdatetime start;
        vscpdatetime end;

        start.set(("0000-01-01T00:00:00")); // The first date
        end.set(("9999-12-31T23:59:59"));   // The last date

        // Must be authorised to do this
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {

            str = vscp_str_format(("-;TBL_GET;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                     WEBSOCK_STR_ERROR_NOT_AUTHORISED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(
              LOG_ERR,
              "[Websocket] User/host not authorised to get data from a table.");

            return; // We still leave channel open
        }

        // Get table name
        std::string strTable;
        if (!tokens.empty()) {
            strTable = tokens.front();
            tokens.pop_front();
            vscp_trim(strTable);
        } else {
            // Error: Need tablename
            str = vscp_str_format(("-;TBL_GET;%d;%s"),
                                     (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                     WEBSOCK_STR_ERROR_SYNTAX_ERROR);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
        }

        if (!tokens.empty()) {
            start.set(tokens.front());
            tokens.pop_front();
            if (!start.isValid()) {
                // Invalid date time
                str = vscp_str_format(("-;TBL_GET;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }

        } else {
            str = vscp_str_format(("-;TBL_GET;%d;%s"),
                                     (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                     WEBSOCK_STR_ERROR_SYNTAX_ERROR);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
        }

        if (!tokens.empty()) {
            end.set(tokens.front());
            if (!end.isValid()) {
                // Invalid date time
                str = vscp_str_format(("-;TBL_GET;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }
            tokens.pop_front();
        } else {
            str = vscp_str_format(("-;TBL_GET;%d;%s"),
                                     (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                     WEBSOCK_STR_ERROR_SYNTAX_ERROR);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
        }

        bool bAll;
        if (!tokens.empty()) {
            std::string str = tokens.front();
            tokens.pop_front();
            vscp_trim(str);
            vscp_makeUpper(str);
            if (0 == vscp_strcasecmp(str.c_str(), "FULL")) {
                bAll = true;
            }
        }

        pthread_mutex_lock(&gpobj->m_mutexUserTables);

        CVSCPTable *pTable = gpobj->m_userTableObjects.getTable(strTable);
        if (NULL == pTable) {

            // Failed
            str = vscp_str_format(("-;TBL_GET;%d;%s"),
                                     (int)WEBSOCK_ERROR_TABLE_FAILED_TO_GET,
                                     WEBSOCK_STR_ERROR_TABLE_FAILED_TO_GET);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            return;
        }

        sqlite3_stmt *ppStmt;
        if (!pTable->prepareRangeOfData(start, end, &ppStmt, bAll)) {

            // Failed
            str = vscp_str_format(("-;TBL_GET;%d;%s"),
                                     (int)WEBSOCK_ERROR_TABLE_FAILED_GET_DATA,
                                     WEBSOCK_STR_ERROR_TABLE_FAILED_GET_DATA);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            return;
        }

        std::string str;
        std::deque<std::string> strArray;
        while (pTable->getRowRangeOfData(ppStmt, str)) {
            strArray.push_back(str);
        }

        if (strArray.size()) {
            for (int i = 0; i < strArray.size(); i++) {

                str = vscp_str_format(("+;%d;%zu;"), i, strArray.size());
                str += strArray[i];

                // Output row
                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }
        }

        pTable->finalizeRangeOfData(ppStmt);
        pthread_mutex_unlock(&gpobj->m_mutexUserTables);

    }

    // ------------------------------------------------------------------------
    //                             TBL_GETRAW
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "TBL_GETRAW")) {

        // Initialize date range to 'all'
        vscpdatetime start;
        vscpdatetime end;

        start.set(("0000-01-01T00:00:00")); // The first date
        end.set(("9999-12-31T23:59:59"));   // The last date

        // Must be authorised to do this
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {

            str = vscp_str_format(("-;TBL_GETRAW;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                     WEBSOCK_STR_ERROR_NOT_AUTHORISED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User/host not authorised to get raw data from "
                   "a table.");

            return; // We still leave channel open
        }

        // Get table name
        std::string strTable;
        if (!tokens.empty()) {
            strTable = tokens.front();
            tokens.pop_front();
            vscp_trim(strTable);
        } else {
            // Error: Need tablename
            str = vscp_str_format(("-;TBL_GETRAW;%d;%s"),
                                     (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                     WEBSOCK_STR_ERROR_SYNTAX_ERROR);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
        }

        if (!tokens.empty()) {
            start.set(tokens.front());
            if (!start.isValid()) {
                // Invalid date time
                str = vscp_str_format(("-;TBL_GETRAW;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }
            tokens.pop_front();
        } else {
            str = vscp_str_format(("-;TBL_GETRAW;%d;%s"),
                                     (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                     WEBSOCK_STR_ERROR_SYNTAX_ERROR);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
        }

        if (!tokens.empty()) {
            end.set(tokens.front());
            if (!end.isValid()) {
                // Invalid date time
                str = vscp_str_format(("-;TBL_GETRAW;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }
            tokens.pop_front();
        } else {
            str = vscp_str_format(("-;TBL_GETRAW;%d;%s"),
                                     (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                     WEBSOCK_STR_ERROR_SYNTAX_ERROR);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
        }

        bool bAll;
        if (!tokens.empty()) {
            std::string str = tokens.front();
            tokens.pop_front();
            vscp_trim(str);
            vscp_makeUpper(str);
            if (0 == vscp_strcasecmp(str.c_str(), "FULL")) {
                bAll = true;
            }
        }

        pthread_mutex_lock(&gpobj->m_mutexUserTables);

        CVSCPTable *pTable = gpobj->m_userTableObjects.getTable(strTable);
        if (NULL == pTable) {

            // Failed
            str = vscp_str_format(("-;TBL_GETRAW;%d;%s"),
                                     (int)WEBSOCK_ERROR_TABLE_FAILED_TO_GET,
                                     WEBSOCK_STR_ERROR_TABLE_FAILED_TO_GET);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            return;
        }

        sqlite3_stmt *ppStmt;
        if (!pTable->prepareRangeOfData(start, end, &ppStmt, bAll)) {

            // Failed
            str = vscp_str_format(("-;TBL_GETRAW;%d;%s"),
                                     (int)WEBSOCK_ERROR_TABLE_FAILED_GET_DATA,
                                     WEBSOCK_STR_ERROR_TABLE_FAILED_GET_DATA);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            return;
        }

        std::string str;
        std::deque<std::string> strArray;
        while (pTable->getRowRangeOfDataRaw(ppStmt, str)) {
            strArray.push_back(str);
        }

        if (strArray.size()) {
            for (int i = 0; i < strArray.size(); i++) {

                str = vscp_str_format(("+;%d;%zu;"), i, strArray.size());
                str += strArray[i];

                // Output row
                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }
        }

        pTable->finalizeRangeOfData(ppStmt);
        pthread_mutex_unlock(&gpobj->m_mutexUserTables);

    }

    // ------------------------------------------------------------------------
    //                             TBL_CLEAR
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "TBL_CLEAR")) {

        // Initialize date range to 'all'
        vscpdatetime start;
        vscpdatetime end;

        start.set(("0000-01-01T00:00:00")); // The first date
        end.set(("9999-12-31T23:59:59"));   // The last date

        // Must be authorised to do this
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {

            str = vscp_str_format(("-;TBL_CLEAR;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                     WEBSOCK_STR_ERROR_NOT_AUTHORISED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(
              LOG_ERR,
              "[Websocket] User/host not authorised to log data to a table.");

            return; // We still leave channel open
        }

        // Get table name
        std::string strTable;
        if (!tokens.empty()) {
            strTable = tokens.front();
            tokens.pop_front();
            vscp_trim(strTable);
        } else {
            // Error: Need tablename
            str = vscp_str_format(("-;TBL_CLEAR;%d;%s"),
                                     (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                     WEBSOCK_STR_ERROR_SYNTAX_ERROR);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
        }

        bool bClearAll = true;
        if (!tokens.empty()) {
            bClearAll = false;
            start.set(tokens.front());
            if (!start.isValid()) {
                // Invalid date time
                str = vscp_str_format(("-;TBL_CLEAR;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }
            tokens.pop_front();
        }

        if (!tokens.empty()) {
            end.set(tokens.front());
            tokens.pop_front();
            if (!end.isValid()) {
                // Invalid date time
                str = vscp_str_format(("-;TBL_CLEAR;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }
        }

        pthread_mutex_lock(&gpobj->m_mutexUserTables);

        CVSCPTable *pTable = gpobj->m_userTableObjects.getTable(strTable);

        if (NULL == pTable) {
            // Failed
            str = vscp_str_format(("-;TBL_CLEAR;%d;%s"),
                                     (int)WEBSOCK_ERROR_TABLE_NOT_FOUND,
                                     WEBSOCK_STR_ERROR_TABLE_NOT_FOUND);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            return;
        }

        if (bClearAll) {
            if (!pTable->clearTable()) {
                // Failed
                str = vscp_str_format(("-;TBL_CLEAR;%d;%s"),
                                         (int)WEBSOCK_ERROR_TABLE_FAILED_CLEAR,
                                         WEBSOCK_STR_ERROR_TABLE_FAILED_CLEAR);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
                pthread_mutex_unlock(&gpobj->m_mutexUserTables);
                return;
            }
        } else {
            if (!pTable->clearTableRange(start, end)) {
                // Failed
                str = vscp_str_format(("-;TBL_CLEAR;%d;%s"),
                                         (int)WEBSOCK_ERROR_TABLE_FAILED_CLEAR,
                                         WEBSOCK_STR_ERROR_TABLE_FAILED_CLEAR);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
                pthread_mutex_unlock(&gpobj->m_mutexUserTables);
                return;
            }
        }

        pthread_mutex_unlock(&gpobj->m_mutexUserTables);

    }

    // ------------------------------------------------------------------------
    //                             TBL_LOG
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "TBL_LOG")) {

        // Must be authorised to do this
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {

            str = vscp_str_format(("-;TBL_LOG;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                     WEBSOCK_STR_ERROR_NOT_AUTHORISED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(
              LOG_ERR,
              "[Websocket] User/host not authorised to log data to a table.");

            return; // We still leave channel open
        }

        // Get table name
        std::string strTable;
        if (!tokens.empty()) {
            strTable = tokens.front();
            tokens.pop_front();
            vscp_trim(strTable);
        } else {
            // Error: Need tablename
            str = vscp_str_format(("-;TBL_LOG;%d;%s"),
                                     (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                     WEBSOCK_STR_ERROR_SYNTAX_ERROR);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
        }

        // Get the value
        double value;
        if (!tokens.empty()) {
            std::string str = tokens.front();
            tokens.pop_front();
            value = std::stod(str);
        } else {
            // Problems: A value must be given
            str = vscp_str_format(("-;TBL_LOG;%d;%s"),
                                     (int)WEBSOCK_ERROR_TABLE_LOG_MISSING_VALUE,
                                     WEBSOCK_STR_ERROR_TABLE_LOG_MISSING_VALUE);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
            return;
        }

        // Get the datetime if its there
        vscpdatetime dt;
        if (!tokens.empty()) {

            uint32_t ms = 0;

            std::string str = tokens.front();
            tokens.pop_front();
            vscp_trim(str);

            if (!dt.set(str)) {
                // Problems: The value is not in a valid format
                str = vscp_str_format(("-;TBL_LOG;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
                return;
            }

            // Get possible millisecond part
            str = vscp_str_after(str, '.');
            vscp_trim(str);
            if (str.length()) {
                ms = vscp_readStringValue(str);
            }

            dt.SetMillisecond(ms);

        } else {
            // Set to now
            dt = vscpdatetime::UTCNow();
        }

        pthread_mutex_lock(&gpobj->m_mutexUserTables);

        CVSCPTable *pTable = gpobj->m_userTableObjects.getTable(strTable);

        if (NULL == pTable) {
            // Failed
            str = vscp_str_format(("-;TBL_LOG;%d;%s"),
                                     (int)WEBSOCK_ERROR_TABLE_NOT_FOUND,
                                     WEBSOCK_STR_ERROR_TABLE_NOT_FOUND);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            return;
        }

        // Log data
        if (!pTable->logData(dt, value)) {
            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            str = vscp_str_format(("-;TBL_LOG;%d;%s"),
                                     (int)WEBSOCK_ERROR_TABLE_LOG_FAILED,
                                     WEBSOCK_STR_ERROR_TABLE_LOG_FAILED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
            return;
        }

        pthread_mutex_unlock(&gpobj->m_mutexUserTables);

    }

    // ------------------------------------------------------------------------
    //                             TBL_LOGSQL
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "TBL_LOGSQL")) {

        // Must be authorised to do this
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {

            str = vscp_str_format(("-;TBL_LOGSQL;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                     WEBSOCK_STR_ERROR_NOT_AUTHORISED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User/host not authorised to log sql data to a "
                   "table.");

            return; // We still leave channel open
        }

        // Get table name
        std::string strTable;
        if (!tokens.empty()) {
            strTable = tokens.front();
            tokens.pop_front();
            vscp_trim(strTable);
        } else {
            // Error: Need tablename
            str = vscp_str_format(("-;TBL_LOG;%d;%s"),
                                     (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                     WEBSOCK_STR_ERROR_SYNTAX_ERROR);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
        }

        // Get SQL expression
        std::string strSQL;
        if (!tokens.empty()) {
            strSQL = tokens.front();
            tokens.pop_front();
            vscp_trim(strSQL);
        } else {
            // Problems: A SQL expression must be given
            str = vscp_str_format(("-;TBL_LOGSQL;%d;%s"),
                                     (int)WEBSOCK_ERROR_TABLE_NEED_SQL,
                                     WEBSOCK_STR_ERROR_TABLE_NEED_SQL);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
            return;
        }

        pthread_mutex_lock(&gpobj->m_mutexUserTables);

        CVSCPTable *pTable = gpobj->m_userTableObjects.getTable(strTable);

        if (NULL == pTable) {
            // Failed
            str = vscp_str_format(("-;TBL_LOGSQL;%d;%s"),
                                     (int)WEBSOCK_ERROR_TABLE_NOT_FOUND,
                                     WEBSOCK_STR_ERROR_TABLE_NOT_FOUND);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            return;
        }

        // Log data
        if (!pTable->logData(strSQL)) {
            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            str = vscp_str_format(("-;TBL_LOGSQL;%d;%s"),
                                     (int)WEBSOCK_ERROR_TABLE_LOG_FAILED,
                                     WEBSOCK_STR_ERROR_TABLE_LOG_FAILED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
            return;
        }

        pthread_mutex_unlock(&gpobj->m_mutexUserTables);

    }

    // ------------------------------------------------------------------------
    //                             TBL_RECORDS
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "TBL_RECORDS")) {

        vscpdatetime strStart;
        vscpdatetime strEnd;

        // Initialize date range to 'all'
        strStart.set(("0000-01-01T00:00:00")); // The first date
        strEnd.set(("9999-12-31T23:59:59"));   // The last date

        // Must be authorised to do this
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {

            str = vscp_str_format(("-;TBL_RECORDS;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                     WEBSOCK_STR_ERROR_NOT_AUTHORISED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(
              LOG_ERR,
              "[Websocket] User/host not authorised to do records on table.");

            return; // We still leave channel open
        }

        // Get table name
        std::string strTable;
        if (!tokens.empty()) {
            strTable = tokens.front();
            tokens.pop_front();
            vscp_trim(strTable);
        } else {
            // Error: Need tablename
            str = vscp_str_format(("-;TBL_RECORDS;%d;%s"),
                                     (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                     WEBSOCK_STR_ERROR_SYNTAX_ERROR);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
        }

        // If available get Start data
        if (!tokens.empty()) {
            if (!strStart.set(tokens.front())) {
                str = vscp_str_format(("-;TBL_RECORDS;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }

            tokens.pop_front();
        }

        // If available get end date
        if (!tokens.empty()) {
            if (!strEnd.set(tokens.front())) {
                str = vscp_str_format(("-;TBL_RECORDS;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }

            tokens.pop_front();
        }

        pthread_mutex_lock(&gpobj->m_mutexUserTables);

        CVSCPTable *pTable = gpobj->m_userTableObjects.getTable(strTable);

        if (NULL == pTable) {
            // Failed
            str = vscp_str_format(("-;TBL_RECORDS;%d;%s"),
                                     (int)WEBSOCK_ERROR_TABLE_NOT_FOUND,
                                     WEBSOCK_STR_ERROR_TABLE_NOT_FOUND);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            return;
        }

        double count;
        if (!pTable->getNumberOfRecordsForRange(strStart, strEnd, &count)) {
            str = vscp_str_format(
              ("-;TBL_RECORDS;%d;%s"),
              (int)WEBSOCK_ERROR_TABLE_FAILED_COMMAND_RECORDS,
              WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_RECORDS);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            return;
        }

        pthread_mutex_unlock(&gpobj->m_mutexUserTables);

        // Success
        str = vscp_str_format(("+;TBL_RECORDS;%lf"), count);

        web_websocket_write(conn,
                            WEB_WEBSOCKET_OPCODE_TEXT,
                            (const char *)str.c_str(),
                            str.length());

    }

    // ------------------------------------------------------------------------
    //                              TBL_FIRSTDATE
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "TBL_FIRSTDATE")) {

        vscpdatetime strStart;
        vscpdatetime strEnd;

        // Initialize date range to 'all'
        strStart.set(("0000-01-01T00:00:00")); // The first date
        strEnd.set(("9999-12-31T23:59:59"));   // The last date

        // Must be authorised to do this
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {

            str = vscp_str_format(("-;TBL_FIRSTDATE;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                     WEBSOCK_STR_ERROR_NOT_AUTHORISED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User/host not authorised to do "
                   "firstdate on table.");

            return; // We still leave channel open
        }

        // Get table name
        std::string strTable;
        if (!tokens.empty()) {
            strTable = tokens.front();
            tokens.pop_front();
            vscp_trim(strTable);
        } else {
            // Error: Need tablename
            str = vscp_str_format(("-;TBL_FIRSTDATE;%d;%s"),
                                     (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                     WEBSOCK_STR_ERROR_SYNTAX_ERROR);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
        }

        // If available get Start data
        if (!tokens.empty()) {
            if (!strStart.set(tokens.front())) {
                str = vscp_str_format(("-;TBL_FIRSTDATE;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }

            tokens.pop_front();
        }

        // If available get end date
        if (!tokens.empty()) {
            if (!strEnd.set(tokens.front())) {
                str = vscp_str_format(("-;TBL_FIRSTDATE;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }
            tokens.pop_front();
        }

        pthread_mutex_lock(&gpobj->m_mutexUserTables);

        CVSCPTable *pTable = gpobj->m_userTableObjects.getTable(strTable);

        if (NULL == pTable) {
            // Failed
            str = vscp_str_format(("-;TBL_FIRSTDATE;%d;%s"),
                                     (int)WEBSOCK_ERROR_TABLE_NOT_FOUND,
                                     WEBSOCK_STR_ERROR_TABLE_NOT_FOUND);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            return;
        }

        vscpdatetime first;
        if (!pTable->getFirstDate(first)) {
            str = vscp_str_format(
              ("-;TBL_FIRSTDATE;%d;%s"),
              (int)WEBSOCK_ERROR_TABLE_FAILED_COMMAND_FIRSTDATE,
              WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_FIRSTDATE);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            return;
        }

        pthread_mutex_unlock(&gpobj->m_mutexUserTables);

        // Success
        str = vscp_str_format(("+;TBL_FIRSTDATE;%s"),
                                 (const char *)first.getISODateTime().c_str());

        web_websocket_write(conn,
                            WEB_WEBSOCKET_OPCODE_TEXT,
                            (const char *)str.c_str(),
                            str.length());
    }

    // ------------------------------------------------------------------------
    //                              TBL_LASTDATE
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "TBL_LASTDATE")) {

        vscpdatetime strStart;
        vscpdatetime strEnd;

        // Initialize date range to 'all'
        strStart.set(("0000-01-01T00:00:00")); // The first date
        strEnd.set(("9999-12-31T23:59:59"));   // The last date

        // Must be authorised to do this
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {

            str = vscp_str_format(("-;TBL_LASTDATE;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                     WEBSOCK_STR_ERROR_NOT_AUTHORISED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User/host not authorised to do "
                   "lastdate on table.");

            return; // We still leave channel open
        }

        // Get table name
        std::string strTable;
        if (!tokens.empty()) {
            strTable = tokens.front();
            tokens.pop_front();
            vscp_trim(strTable);
        } else {
            // Error: Need tablename
            str = vscp_str_format(("-;TBL_LASTDATE;%d;%s"),
                                     (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                     WEBSOCK_STR_ERROR_SYNTAX_ERROR);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
        }

        // If available get Start data
        if (!tokens.empty()) {
            if (!strStart.set(tokens.front())) {
                str = vscp_str_format(("-;TBL_LASTDATE;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }
            tokens.pop_front();
        }

        // If available get end date
        if (!tokens.empty()) {
            if (!strEnd.set(tokens.front())) {
                str = vscp_str_format(("-;TBL_LASTDATE;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }
            tokens.pop_front();
        }

        pthread_mutex_lock(&gpobj->m_mutexUserTables);

        CVSCPTable *pTable = gpobj->m_userTableObjects.getTable(strTable);

        if (NULL == pTable) {
            // Failed
            str = vscp_str_format(("-;TBL_LASTDATE;%d;%s"),
                                     (int)WEBSOCK_ERROR_TABLE_NOT_FOUND,
                                     WEBSOCK_STR_ERROR_TABLE_NOT_FOUND);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            return;
        }

        vscpdatetime last;
        if (!pTable->getLastDate(last)) {
            str = vscp_str_format(
              ("-;TBL_LASTDATE;%d;%s"),
              (int)WEBSOCK_ERROR_TABLE_FAILED_COMMAND_LASTDATE,
              WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_LASTDATE);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            return;
        }

        pthread_mutex_unlock(&gpobj->m_mutexUserTables);

        // Success
        str = vscp_str_format(("+;TBL_LASTDATE;%s"),
                                 (const char *)last.getISODateTime().c_str());

        web_websocket_write(conn,
                            WEB_WEBSOCKET_OPCODE_TEXT,
                            (const char *)str.c_str(),
                            str.length());
    }

    // ------------------------------------------------------------------------
    //                              TBL_SUM
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "TBL_SUM")) {

        vscpdatetime strStart;
        vscpdatetime strEnd;

        // Initialize date range to 'all'
        strStart.set(("0000-01-01T00:00:00")); // The first date
        strEnd.set(("9999-12-31T23:59:59"));   // The last date

        // Must be authorised to do this
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {

            str = vscp_str_format(("-;TBL_SUM;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                     WEBSOCK_STR_ERROR_NOT_AUTHORISED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User/host not authorised to do sum "
                   "on table.");

            return; // We still leave channel open
        }

        // Get table name
        std::string strTable;
        if (!tokens.empty()) {
            strTable = tokens.front();
            tokens.pop_front();
            vscp_trim(strTable);
        } else {
            // Error: Need tablename
            str = vscp_str_format(("-;TBL_SUM;%d;%s"),
                                     (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                     WEBSOCK_STR_ERROR_SYNTAX_ERROR);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
        }

        // If available get Start data
        if (!tokens.empty()) {
            if (!strStart.set(tokens.front())) {
                str = vscp_str_format(("-;TBL_SUM;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }
            tokens.pop_front();
        }

        // If available get end date
        if (!tokens.empty()) {
            if (!strEnd.set(tokens.front())) {
                str = vscp_str_format(("-;TBL_SUM;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }
            tokens.pop_front();
        }

        pthread_mutex_lock(&gpobj->m_mutexUserTables);

        CVSCPTable *pTable = gpobj->m_userTableObjects.getTable(strTable);

        if (NULL == pTable) {
            // Failed
            str = vscp_str_format(("-;TBL_SUM;%d;%s"),
                                     (int)WEBSOCK_ERROR_TABLE_NOT_FOUND,
                                     WEBSOCK_STR_ERROR_TABLE_NOT_FOUND);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            return;
        }

        double sum;
        if (!pTable->getSumValue(strStart, strEnd, &sum)) {
            str =
              vscp_str_format(("-;TBL_SUM;%d;%s"),
                                 (int)WEBSOCK_ERROR_TABLE_FAILED_COMMAND_SUM,
                                 WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_SUM);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            return;
        }

        pthread_mutex_unlock(&gpobj->m_mutexUserTables);

        // Success
        str = vscp_str_format(("+;TBL_SUM;%lf"), sum);

        web_websocket_write(conn,
                            WEB_WEBSOCKET_OPCODE_TEXT,
                            (const char *)str.c_str(),
                            str.length());

    }

    // ------------------------------------------------------------------------
    //                              TBL_MIN
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "TBL_MIN")) {

        vscpdatetime strStart;
        vscpdatetime strEnd;

        // Initialize date range to 'all'
        strStart.set(("0000-01-01T00:00:00")); // The first date
        strEnd.set(("9999-12-31T23:59:59"));   // The last date

        // Must be authorised to do this
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {

            str = vscp_str_format(("-;TBL_MIN;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                     WEBSOCK_STR_ERROR_NOT_AUTHORISED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User/host not authorised to do min "
                   "on table.");

            return; // We still leave channel open
        }

        // Get table name
        std::string strTable;
        if (!tokens.empty()) {
            strTable = tokens.front();
            tokens.pop_front();
            vscp_trim(strTable);
        } else {
            // Error: Need tablename
            str = vscp_str_format(("-;TBL_MIN;%d;%s"),
                                     (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                     WEBSOCK_STR_ERROR_SYNTAX_ERROR);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
        }

        // If available get Start data
        if (!tokens.empty()) {
            if (!strStart.set(tokens.front())) {
                str = vscp_str_format(("-;TBL_MIN;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }
            tokens.pop_front();
        }

        // If available get end date
        if (!tokens.empty()) {
            if (!strEnd.set(tokens.front())) {
                str = vscp_str_format(("-;TBL_MIN;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }
            tokens.pop_front();
        }

        pthread_mutex_lock(&gpobj->m_mutexUserTables);

        CVSCPTable *pTable = gpobj->m_userTableObjects.getTable(strTable);

        if (NULL == pTable) {
            // Failed
            str = vscp_str_format(("-;TBL_MIN;%d;%s"),
                                     (int)WEBSOCK_ERROR_TABLE_NOT_FOUND,
                                     WEBSOCK_STR_ERROR_TABLE_NOT_FOUND);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            return;
        }

        double min;
        if (!pTable->getMinValue(strStart, strEnd, &min)) {
            str =
              vscp_str_format(("-;TBL_MIN;%d;%s"),
                                 (int)WEBSOCK_ERROR_TABLE_FAILED_COMMAND_MIN,
                                 WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_MIN);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            return;
        }

        pthread_mutex_unlock(&gpobj->m_mutexUserTables);

        // Success
        str = vscp_str_format(("+;TBL_MIN;%lf"), min);

        web_websocket_write(conn,
                            WEB_WEBSOCKET_OPCODE_TEXT,
                            (const char *)str.c_str(),
                            str.length());

    }

    // ------------------------------------------------------------------------
    //                              TBL_MAX
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "TBL_MAX")) {

        vscpdatetime strStart;
        vscpdatetime strEnd;

        // Initialize date range to 'all'
        strStart.set(("0000-01-01T00:00:00")); // The first date
        strEnd.set(("9999-12-31T23:59:59"));   // The last date

        // Must be authorised to do this
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {

            str = vscp_str_format(("-;TBL_MAX;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                     WEBSOCK_STR_ERROR_NOT_AUTHORISED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User/host not authorised to do "
                   "max on table.");

            return; // We still leave channel open
        }

        // Get table name
        std::string strTable;
        if (!tokens.empty()) {
            strTable = tokens.front();
            tokens.pop_front();
            vscp_trim(strTable);
        } else {
            // Error: Need tablename
            str = vscp_str_format(("-;TBL_MAX;%d;%s"),
                                     (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                     WEBSOCK_STR_ERROR_SYNTAX_ERROR);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
        }

        // If available get Start data
        if (!tokens.empty()) {
            if (!strStart.set(tokens.front())) {
                str = vscp_str_format(("-;TBL_MAX;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }
            tokens.pop_front();
        }

        // If available get end date
        if (!tokens.empty()) {
            if (!strEnd.set(tokens.front())) {
                str = vscp_str_format(("-;TBL_MAX;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }
            tokens.pop_front();
        }

        pthread_mutex_lock(&gpobj->m_mutexUserTables);

        CVSCPTable *pTable = gpobj->m_userTableObjects.getTable(strTable);

        if (NULL == pTable) {
            // Failed
            str = vscp_str_format(("-;TBL_MAX;%d;%s"),
                                     (int)WEBSOCK_ERROR_TABLE_NOT_FOUND,
                                     WEBSOCK_STR_ERROR_TABLE_NOT_FOUND);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            return;
        }

        double max;
        if (!pTable->getMaxValue(strStart, strEnd, &max)) {
            str =
              vscp_str_format(("-;TBL_MAX;%d;%s"),
                                 (int)WEBSOCK_ERROR_TABLE_FAILED_COMMAND_MAX,
                                 WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_MAX);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            return;
        }

        pthread_mutex_unlock(&gpobj->m_mutexUserTables);

        // Success
        str = vscp_str_format(("+;TBL_MAX;%lf"), max);

        web_websocket_write(conn,
                            WEB_WEBSOCKET_OPCODE_TEXT,
                            (const char *)str.c_str(),
                            str.length());

    }

    // ------------------------------------------------------------------------
    //                              TBL_AVERAGE
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "TBL_AVERAGE")) {

        vscpdatetime strStart;
        vscpdatetime strEnd;

        // Initialize date range to 'all'
        strStart.set(("0000-01-01T00:00:00")); // The first date
        strEnd.set(("9999-12-31T23:59:59"));   // The last date

        // Must be authorised to do this
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {

            str = vscp_str_format(("-;TBL_AVERAGE;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                     WEBSOCK_STR_ERROR_NOT_AUTHORISED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User/host not authorised to do "
                   "average on table.");

            return; // We still leave channel open
        }

        // Get table name
        std::string strTable;
        if (!tokens.empty()) {
            strTable = tokens.front();
            tokens.pop_front();
            vscp_trim(strTable);
        } else {
            // Error: Need tablename
            str = vscp_str_format(("-;TBL_AVERAGE;%d;%s"),
                                     (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                     WEBSOCK_STR_ERROR_SYNTAX_ERROR);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
        }

        // If available get Start data
        if (!tokens.empty()) {
            if (!strStart.set(tokens.front())) {
                str = vscp_str_format(("-;TBL_AVERAGE;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }
            tokens.pop_front();
        }

        // If available get end date
        if (!tokens.empty()) {
            if (!strEnd.set(tokens.front())) {
                str = vscp_str_format(("-;TBL_AVERAGE;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }
            tokens.pop_front();
        }

        pthread_mutex_lock(&gpobj->m_mutexUserTables);

        CVSCPTable *pTable = gpobj->m_userTableObjects.getTable(strTable);

        if (NULL == pTable) {
            // Failed
            str = vscp_str_format(("-;TBL_AVERAGE;%d;%s"),
                                     (int)WEBSOCK_ERROR_TABLE_NOT_FOUND,
                                     WEBSOCK_STR_ERROR_TABLE_NOT_FOUND);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            return;
        }

        double average;
        if (!pTable->getSumValue(strStart, strEnd, &average)) {
            str = vscp_str_format(
              ("-;TBL_AVERAGE;%d;%s"),
              (int)WEBSOCK_ERROR_TABLE_FAILED_COMMAND_AVERAGE,
              WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_AVERAGE);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            return;
        }

        pthread_mutex_unlock(&gpobj->m_mutexUserTables);

        // Success
        str = vscp_str_format(("+;TBL_AVERAGE;%lf"), average);

        web_websocket_write(conn,
                            WEB_WEBSOCKET_OPCODE_TEXT,
                            (const char *)str.c_str(),
                            str.length());

    }

    // ------------------------------------------------------------------------
    //                              TBL_MEDIAN
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "TBL_MEDIAN")) {

        vscpdatetime strStart;
        vscpdatetime strEnd;

        // Initialize date range to 'all'
        strStart.set(("0000-01-01T00:00:00")); // The first date
        strEnd.set(("9999-12-31T23:59:59"));   // The last date

        // Must be authorised to do this
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {

            str = vscp_str_format(("-;TBL_MEDIAN;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                     WEBSOCK_STR_ERROR_NOT_AUTHORISED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User/host not authorised to do "
                   "median on table.");

            return; // We still leave channel open
        }

        // Get table name
        std::string strTable;
        if (!tokens.empty()) {
            strTable = tokens.front();
            tokens.pop_front();
            vscp_trim(strTable);
        } else {
            // Error: Need tablename
            str = vscp_str_format(("-;TBL_MEDIAN;%d;%s"),
                                     (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                     WEBSOCK_STR_ERROR_SYNTAX_ERROR);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
        }

        // If available get Start data
        if (!tokens.empty()) {
            if (!strStart.set(tokens.front())) {
                str = vscp_str_format(("-;TBL_MEDIAN;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }
            tokens.pop_front();
        }

        // If available get end date
        if (!tokens.empty()) {
            if (!strEnd.set(tokens.front())) {
                str = vscp_str_format(("-;TBL_MEDIAN;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }
            tokens.pop_front();
        }

        pthread_mutex_lock(&gpobj->m_mutexUserTables);

        CVSCPTable *pTable = gpobj->m_userTableObjects.getTable(strTable);

        if (NULL == pTable) {
            // Failed
            str = vscp_str_format(("-;TBL_MEDIAN;%d;%s"),
                                     (int)WEBSOCK_ERROR_TABLE_NOT_FOUND,
                                     WEBSOCK_STR_ERROR_TABLE_NOT_FOUND);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            return;
        }

        double median;
        if (!pTable->getMedianValue(strStart, strEnd, &median)) {
            str =
              vscp_str_format(("-;TBL_MEDIAN;%d;%s"),
                                 (int)WEBSOCK_ERROR_TABLE_FAILED_COMMAND_MEDIAN,
                                 WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_MEDIAN);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            return;
        }

        pthread_mutex_unlock(&gpobj->m_mutexUserTables);

        // Success
        str = vscp_str_format(("+;TBL_MEDIAN;%lf"), median);

        web_websocket_write(conn,
                            WEB_WEBSOCKET_OPCODE_TEXT,
                            (const char *)str.c_str(),
                            str.length());

    }

    // ------------------------------------------------------------------------
    //                              TBL_STDDEV
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "TBL_STDDEV")) {

        vscpdatetime strStart;
        vscpdatetime strEnd;

        // Initialize date range to 'all'
        strStart.set(("0000-01-01T00:00:00")); // The first date
        strEnd.set(("9999-12-31T23:59:59"));   // The last date

        // Must be authorised to do this
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {

            str = vscp_str_format(("-;TBL_STDDEV;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                     WEBSOCK_STR_ERROR_NOT_AUTHORISED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User/host not authorised to do "
                   "stddev on table.");

            return; // We still leave channel open
        }

        // Get table name
        std::string strTable;
        if (!tokens.empty()) {
            strTable = tokens.front();
            tokens.pop_front();
            vscp_trim(strTable);
        } else {
            // Error: Need tablename
            str = vscp_str_format(("-;TBL_STDDEV;%d;%s"),
                                     (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                     WEBSOCK_STR_ERROR_SYNTAX_ERROR);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
        }

        // If available get Start data
        if (!tokens.empty()) {
            if (!strStart.set(tokens.front())) {
                str = vscp_str_format(("-;TBL_STDDEV;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }
            tokens.pop_front();
        }

        // If available get end date
        if (!tokens.empty()) {
            if (!strEnd.set(tokens.front())) {
                str = vscp_str_format(("-;TBL_STDDEV;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }
            tokens.pop_front();
        }

        pthread_mutex_lock(&gpobj->m_mutexUserTables);

        CVSCPTable *pTable = gpobj->m_userTableObjects.getTable(strTable);

        if (NULL == pTable) {
            // Failed
            str = vscp_str_format(("-;TBL_STDDEV;%d;%s"),
                                     (int)WEBSOCK_ERROR_TABLE_NOT_FOUND,
                                     WEBSOCK_STR_ERROR_TABLE_NOT_FOUND);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            return;
        }

        double stddev;
        if (!pTable->getStdevValue(strStart, strEnd, &stddev)) {
            str =
              vscp_str_format(("-;TBL_STDDEV;%d;%s"),
                                 (int)WEBSOCK_ERROR_TABLE_FAILED_COMMAND_STDDEV,
                                 WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_STDDEV);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            return;
        }

        pthread_mutex_unlock(&gpobj->m_mutexUserTables);

        // Success
        str = vscp_str_format(("+;TBL_STDDEV;%lf"), stddev);

        web_websocket_write(conn,
                            WEB_WEBSOCKET_OPCODE_TEXT,
                            (const char *)str.c_str(),
                            str.length());

    }

    // ------------------------------------------------------------------------
    //                              TBL_VARIANCE
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "TBL_VARIANCE")) {

        vscpdatetime strStart;
        vscpdatetime strEnd;

        // Initialize date range to 'all'
        strStart.set(("0000-01-01T00:00:00")); // The first date
        strEnd.set(("9999-12-31T23:59:59"));   // The last date

        // Must be authorised to do this
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {

            str = vscp_str_format(("-;TBL_VARIANCE;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                     WEBSOCK_STR_ERROR_NOT_AUTHORISED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User/host not authorised to do "
                   "variance on table.");

            return; // We still leave channel open
        }

        // Get table name
        std::string strTable;
        if (!tokens.empty()) {
            strTable = tokens.front();
            tokens.pop_front();
            vscp_trim(strTable);
        } else {
            // Error: Need tablename
            str = vscp_str_format(("-;TBL_VARIANCE;%d;%s"),
                                     (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                     WEBSOCK_STR_ERROR_SYNTAX_ERROR);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
        }

        // If available get Start data
        if (!tokens.empty()) {
            if (!strStart.set(tokens.front())) {
                str = vscp_str_format(("-;TBL_VARIANCE;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }
            tokens.pop_front();
        }

        // If available get end date
        if (!tokens.empty()) {
            if (!strEnd.set(tokens.front())) {
                str = vscp_str_format(("-;TBL_VARIANCE;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }
            tokens.pop_front();
        }

        pthread_mutex_lock(&gpobj->m_mutexUserTables);

        CVSCPTable *pTable = gpobj->m_userTableObjects.getTable(strTable);

        if (NULL == pTable) {
            // Failed
            str = vscp_str_format(("-;TBL_VARIANCE;%d;%s"),
                                     (int)WEBSOCK_ERROR_TABLE_NOT_FOUND,
                                     WEBSOCK_STR_ERROR_TABLE_NOT_FOUND);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            return;
        }

        double variance;
        if (!pTable->getVarianceValue(strStart, strEnd, &variance)) {
            str = vscp_str_format(
              ("-;TBL_VARIANCE;%d;%s"),
              (int)WEBSOCK_ERROR_TABLE_FAILED_COMMAND_VARIANCE,
              WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_VARIANCE);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            return;
        }

        pthread_mutex_unlock(&gpobj->m_mutexUserTables);

        // Success
        str = vscp_str_format(("+;TBL_VARIANCE;%lf"), variance);

        web_websocket_write(conn,
                            WEB_WEBSOCKET_OPCODE_TEXT,
                            (const char *)str.c_str(),
                            str.length());

    }

    // ------------------------------------------------------------------------
    //                              TBL_MODE
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "TBL_MODE")) {

        vscpdatetime strStart;
        vscpdatetime strEnd;

        // Initialize date range to 'all'
        strStart.set(("0000-01-01T00:00:00")); // The first date
        strEnd.set(("9999-12-31T23:59:59"));   // The last date

        // Must be authorised to do this
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {

            str = vscp_str_format(("-;TBL_MODE;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                     WEBSOCK_STR_ERROR_NOT_AUTHORISED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User/host not authorised to do "
                   "mode on table.");

            return; // We still leave channel open
        }

        // Get table name
        std::string strTable;
        if (!tokens.empty()) {
            strTable = tokens.front();
            tokens.pop_front();
            vscp_trim(strTable);
        } else {
            // Error: Need tablename
            str = vscp_str_format(("-;TBL_MODE;%d;%s"),
                                     (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                     WEBSOCK_STR_ERROR_SYNTAX_ERROR);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
        }

        // If available get Start data
        if (!tokens.empty()) {
            if (!strStart.set(tokens.front())) {
                str = vscp_str_format(("-;TBL_MODE;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }
            tokens.pop_front();
        }

        // If available get end date
        if (!tokens.empty()) {
            if (!strEnd.set(tokens.front())) {
                str = vscp_str_format(("-;TBL_MODE;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }
            tokens.pop_front();
        }

        pthread_mutex_lock(&gpobj->m_mutexUserTables);

        CVSCPTable *pTable = gpobj->m_userTableObjects.getTable(strTable);

        if (NULL == pTable) {
            // Failed
            str = vscp_str_format(("-;TBL_MODE;%d;%s"),
                                     (int)WEBSOCK_ERROR_TABLE_NOT_FOUND,
                                     WEBSOCK_STR_ERROR_TABLE_NOT_FOUND);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            return;
        }

        double mode;
        if (!pTable->getModeValue(strStart, strEnd, &mode)) {
            str =
              vscp_str_format(("-;TBL_MODE;%d;%s"),
                                 (int)WEBSOCK_ERROR_TABLE_FAILED_COMMAND_MODE,
                                 WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_MODE);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            return;
        }

        pthread_mutex_unlock(&gpobj->m_mutexUserTables);

        // Success
        str = vscp_str_format(("+;TBL_MODE;%lf"), mode);

        web_websocket_write(conn,
                            WEB_WEBSOCKET_OPCODE_TEXT,
                            (const char *)str.c_str(),
                            str.length());

    }

    // ------------------------------------------------------------------------
    //                              TBL_LOWERQ
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "TBL_LOWERQ")) {

        vscpdatetime strStart;
        vscpdatetime strEnd;

        // Initialize date range to 'all'
        strStart.set(("0000-01-01T00:00:00")); // The first date
        strEnd.set(("9999-12-31T23:59:59"));   // The last date

        // Must be authorised to do this
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {

            str = vscp_str_format(("-;TBL_LOWERQ;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                     WEBSOCK_STR_ERROR_NOT_AUTHORISED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User/host not authorised to do "
                   "lowerq on table.");

            return; // We still leave channel open
        }

        // Get table name
        std::string strTable;
        if (!tokens.empty()) {
            strTable = tokens.front();
            tokens.pop_front();
            vscp_trim(strTable);
        } else {
            // Error: Need tablename
            str = vscp_str_format(("-;TBL_LOWERQ;%d;%s"),
                                     (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                     WEBSOCK_STR_ERROR_SYNTAX_ERROR);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
        }

        // If available get Start data
        if (!tokens.empty()) {
            if (!strStart.set(tokens.front())) {
                str = vscp_str_format(("-;TBL_LOWERQ;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }
            tokens.pop_front();
        }

        // If available get end date
        if (!tokens.empty()) {
            if (!strEnd.set(tokens.front())) {
                str = vscp_str_format(("-;TBL_LOWERQ;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }
            tokens.pop_front();
        }

        pthread_mutex_lock(&gpobj->m_mutexUserTables);

        CVSCPTable *pTable = gpobj->m_userTableObjects.getTable(strTable);

        if (NULL == pTable) {
            // Failed
            str = vscp_str_format(("-;TBL_LOWERQ;%d;%s"),
                                     (int)WEBSOCK_ERROR_TABLE_NOT_FOUND,
                                     WEBSOCK_STR_ERROR_TABLE_NOT_FOUND);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            return;
        }

        double lowerq;
        if (!pTable->getLowerQuartileValue(strStart, strEnd, &lowerq)) {
            str =
              vscp_str_format(("-;TBL_LOWERQ;%d;%s"),
                                 (int)WEBSOCK_ERROR_TABLE_FAILED_COMMAND_LOWERQ,
                                 WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_LOWERQ);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            return;
        }

        pthread_mutex_unlock(&gpobj->m_mutexUserTables);

        // Success
        str = vscp_str_format(("+;TBL_LOWERQ;%lf"), lowerq);

        web_websocket_write(conn,
                            WEB_WEBSOCKET_OPCODE_TEXT,
                            (const char *)str.c_str(),
                            str.length());

    }

    // ------------------------------------------------------------------------
    //                              TBL_UPPERQ
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "TBL_UPPERQ")) {

        vscpdatetime strStart;
        vscpdatetime strEnd;

        // Initialize date range to 'all'
        strStart.set(("0000-01-01T00:00:00")); // The first date
        strEnd.set(("9999-12-31T23:59:59"));   // The last date

        // Must be authorised to do this
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {

            str = vscp_str_format(("-;TBL_UPPERQ;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                     WEBSOCK_STR_ERROR_NOT_AUTHORISED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User/host not authorised to do "
                   "upperq on table.");

            return; // We still leave channel open
        }

        // Get table name
        std::string strTable;
        if (!tokens.empty()) {
            strTable = tokens.front();
            tokens.pop_front();
            vscp_trim(strTable);
        } else {
            // Error: Need tablename
            str = vscp_str_format(("-;TBL_UPPERQ;%d;%s"),
                                     (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                     WEBSOCK_STR_ERROR_SYNTAX_ERROR);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
        }

        // If available get Start data
        if (!tokens.empty()) {
            if (!strStart.set(tokens.front())) {
                str = vscp_str_format(("-;TBL_UPPERQ;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }
            tokens.pop_front();
        }

        // If available get end date
        if (!tokens.empty()) {
            if (!strEnd.set(tokens.front())) {
                str = vscp_str_format(("-;TBL_UPPERQ;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }
            tokens.pop_front();
        }

        pthread_mutex_lock(&gpobj->m_mutexUserTables);

        CVSCPTable *pTable = gpobj->m_userTableObjects.getTable(strTable);

        if (NULL == pTable) {
            // Failed
            str = vscp_str_format(("-;TBL_UPPERQ;%d;%s"),
                                     (int)WEBSOCK_ERROR_TABLE_NOT_FOUND,
                                     WEBSOCK_STR_ERROR_TABLE_NOT_FOUND);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            return;
        }

        double upperq;
        if (!pTable->getUppeQuartileValue(strStart, strEnd, &upperq)) {
            str =
              vscp_str_format(("-;TBL_UPPERQ;%d;%s"),
                                 (int)WEBSOCK_ERROR_TABLE_FAILED_COMMAND_UPPERQ,
                                 WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_UPPERQ);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            return;
        }

        pthread_mutex_unlock(&gpobj->m_mutexUserTables);

        // Success
        str = vscp_str_format(("+;TBL_UPPERQ;%lf"), upperq);

        web_websocket_write(conn,
                            WEB_WEBSOCKET_OPCODE_TEXT,
                            (const char *)str.c_str(),
                            str.length());

    }

    // ------------------------------------------------------------------------
    //                              TBL_CLEAR
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "TBL_CLEAR")) {

        vscpdatetime strStart;
        vscpdatetime strEnd;

        // Initialize date range to 'all'
        strStart.set(("0000-01-01T00:00:00")); // The first date
        strEnd.set(("9999-12-31T23:59:59"));   // The last date

        // Must be authorised to do this
        if ((NULL == pSession->m_pClientItem) ||
            !pSession->m_pClientItem->bAuthenticated) {

            str = vscp_str_format(("-;TBL_CLEAR;%d;%s"),
                                     (int)WEBSOCK_ERROR_NOT_AUTHORISED,
                                     WEBSOCK_STR_ERROR_NOT_AUTHORISED);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            syslog(LOG_ERR,
                   "[Websocket] User/host not authorised to clear "
                   "table.");

            return; // We still leave channel open
        }

        // Get table name
        std::string strTable;
        if (!tokens.empty()) {
            strTable = tokens.front();
            tokens.pop_front();
            vscp_trim(strTable);
        } else {
            // Error: Need tablename
            str = vscp_str_format(("-;TBL_CLEAR;%d;%s"),
                                     (int)WEBSOCK_ERROR_SYNTAX_ERROR,
                                     WEBSOCK_STR_ERROR_SYNTAX_ERROR);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());
        }

        bool bClearAll = false;

        // If available get Start data
        if (!tokens.empty()) {
            if (!strStart.set(tokens.front())) {
                str = vscp_str_format(("-;TBL_CLEAR;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }
            tokens.pop_front();
        } else {
            bClearAll = true;
        }

        // If available get end date
        if (!tokens.empty()) {
            if (!strEnd.set(tokens.front())) {
                str = vscp_str_format(("-;TBL_CLEAR;%d;%s"),
                                         (int)WEBSOCK_ERROR_INVALID_DATE,
                                         WEBSOCK_STR_ERROR_INVALID_DATE);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
            }
            tokens.pop_front();
        }

        pthread_mutex_lock(&gpobj->m_mutexUserTables);

        CVSCPTable *pTable = gpobj->m_userTableObjects.getTable(strTable);

        if (NULL == pTable) {
            // Failed
            str = vscp_str_format(("-;TBL_CLEAR;%d;%s"),
                                     (int)WEBSOCK_ERROR_TABLE_NOT_FOUND,
                                     WEBSOCK_STR_ERROR_TABLE_NOT_FOUND);

            web_websocket_write(conn,
                                WEB_WEBSOCKET_OPCODE_TEXT,
                                (const char *)str.c_str(),
                                str.length());

            pthread_mutex_unlock(&gpobj->m_mutexUserTables);
            return;
        }

        if (bClearAll) {
            if (!pTable->clearTable()) {
                str = vscp_str_format(
                  ("-;TBL_CLEAR;%d;%s"),
                  (int)WEBSOCK_ERROR_TABLE_FAILED_COMMAND_CLEAR,
                  WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_CLEAR);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
                pthread_mutex_unlock(&gpobj->m_mutexUserTables);
                return;
            }
        } else {
            if (!pTable->clearTableRange(strStart, strEnd)) {
                str = vscp_str_format(
                  ("-;TBL_CLEAR;%d;%s"),
                  (int)WEBSOCK_ERROR_TABLE_FAILED_COMMAND_CLEAR,
                  WEBSOCK_STR_ERROR_TABLE_FAILED_COMMAND_CLEAR);

                web_websocket_write(conn,
                                    WEB_WEBSOCKET_OPCODE_TEXT,
                                    (const char *)str.c_str(),
                                    str.length());
                return;
            }
        }

        pthread_mutex_unlock(&gpobj->m_mutexUserTables);

        // Success
        str = ("+;TBL_CLEAR;%lf");

        web_websocket_write(conn,
                            WEB_WEBSOCKET_OPCODE_TEXT,
                            (const char *)str.c_str(),
                            str.length());

    }

    // ------------------------------------------------------------------------
    //                              VERSION
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "VERSION")) {

        std::string strvalue;

        std::string strResult = ("+;VERSION;");
        strResult += VSCPD_DISPLAY_VERSION;
        strResult += (";");
        strResult += vscp_str_format(("%d.%d.%d.%d"),
                                        VSCPD_MAJOR_VERSION,
                                        VSCPD_MINOR_VERSION,
                                        VSCPD_RELEASE_VERSION,
                                        VSCPD_BUILD_VERSION);
        // Positive reply
        web_websocket_write(conn,
                            WEB_WEBSOCKET_OPCODE_TEXT,
                            (const char *)strResult.c_str(),
                            strResult.length());

    }

    // ------------------------------------------------------------------------
    //                              COPYRIGHT
    //-------------------------------------------------------------------------

    else if (vscp_startsWith(strTok, "COPYRIGHT")) {

        std::string strvalue;

        std::string strResult = ("+;COPYRIGHT;");
        strResult += VSCPD_COPYRIGHT;

        // Positive reply
        web_websocket_write(conn,
                            WEB_WEBSOCKET_OPCODE_TEXT,
                            (const char *)strResult.c_str(),
                            strResult.length());
    }
}
