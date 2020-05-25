// vscpwebserver.cpp
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2020 Ake Hedman, Grodans Paradis AB
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
#pragma implementation
#endif

#define _POSIX

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

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
#include <sys/utsname.h>
#include <syslog.h>
#include <unistd.h>

// For version and copyright info
#include <duktape.h>
#include <expat.h>
#include <json.hpp>
#include <lua.h>
#include <openssl/crypto.h>
#include <openssl/opensslv.h>
#include <sqlite3.h>

#include "web_css.h"
#include "web_js.h"
#include "web_template.h"

#include <civetweb.h>

#include "restsrv.h"
#include <actioncodes.h>
#include <canal_macro.h>
#include <controlobject.h>
#include <devicelist.h>
#include <fastpbkdf2.h>
#include <mdf.h>
#include <remotevariablecodes.h>
#include <restsrv.h>
#include <version.h>
#include <vscp.h>
#include <vscp_aes.h>
#include <vscpbase64.h>
#include <vscpdatetime.h>
#include <vscpdb.h>
#include <vscphelper.h>
#include <vscpmd5.h>
#include <websocket.h>

#include "websrv.h"

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

// Uncomment to compile stock test pages
#define WEB_EXAMPLES
#define EXAMPLE_URI "/civetwebtest"
#define EXIT_URI    "/civetwebtest_exit"

//#define USE_SSL_DH

// ip.v4 and ip.v6 at the same time
// https://github.com/civetweb/civetweb/issues/205
// https://stackoverflow.com/questions/1618240/how-to-support-both-ipv4-and-ipv6-connections

#define GUID_COUNT_TYPES 6 // Number of GUID types defined

// List GUID types
const char* pguid_types[] = { "Common GUID",
                              "Interface",
                              "Level I hardware",
                              "Level II hardware",
                              "User interface component",
                              "Location",
                              NULL };

///////////////////////////////////////////////////
//                 GLOBALS
///////////////////////////////////////////////////

extern CControlObject* gpobj;
extern bool gbRestart;   // Should be set true to restart the VSCP daemon
extern int gbStopDaemon; // Should be set true to stop the daemon

///////////////////////////////////////////////////
//                WEBSERVER
///////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// websrv_sendheader
//

void
websrv_sendheader(struct mg_connection* conn,
                  int returncode,
                  const char* pcontent)
{
    char date[64];
    time_t curtime = time(NULL);
    vscp_getTimeString(date, sizeof(date), &curtime);

    // Check pointers
    if (NULL == pcontent)
        return;

    mg_printf(conn,
              "HTTP/1.1 %d OK\r\n"
              "Content-Type: %s\r\n"
              "Date: %s\r"
              "Cache-Control: no-cache\r\n"
              "Cache-Control: no-store\r\n"
              "Cache-Control: must-revalidate\r\n\r\n",
              returncode,
              pcontent,
              date);
}

///////////////////////////////////////////////////////////////////////////////
// websrv_sendSetCookieHeader
//

void
websrv_sendSetCookieHeader(struct mg_connection* conn,
                           int returncode,
                           const char* pcontent,
                           const char* psid)
{
    char date[64];
    time_t curtime = time(NULL);
    vscp_getTimeString(date, sizeof(date), &curtime);

    // Check pointers
    if ((NULL == pcontent) || (NULL == psid))
        return;

    // https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Set-Cookie
    // https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Cache-Control
    mg_printf(conn,
              "HTTP/1.1 %d OK\r\n"
              "Content-Type: %s\r\n"
              "Date: %s\r"
              "Set-Cookie: sessionid=%s; http-only; path=/\r\n"
              "Cache-Control: no-cache\r\n"
              "Cache-Control: no-store\r\n"
              "Cache-Control: must-revalidate\r\n\r\n",
              returncode,
              pcontent,
              date,
              psid);
}

////////////////////////////////////////////////////////////////////////////////
// web_skip_quoted
//
// Skip the characters until one of the delimiters characters found.
// 0-terminate resulting word. Skip the delimiter and following whitespaces.
// Advance pointer to buffer to the next word. Return found 0-terminated
// word.
// Delimiters can be quoted with quotechar.
//

char*
web_skip_quoted(char** buf,
                const char* delimiters,
                const char* whitespace,
                char quotechar)
{
    char *p, *begin_word, *end_word, *end_whitespace;

    begin_word = *buf;
    end_word   = begin_word + strcspn(begin_word, delimiters);

    // Check for quotechar
    if (end_word > begin_word) {
        p = end_word - 1;
        while (*p == quotechar) {
            // While the delimiter is quoted, look for the next delimiter.

            // This happens, e.g., in calls from parse_auth_header,
            // if the user name contains a " character.

            // If there is anything beyond end_word, copy it.
            if (*end_word != '\0') {
                size_t end_off = strcspn(end_word + 1, delimiters);
                memmove(p, end_word, end_off + 1);
                p += end_off; // p must correspond to end_word - 1
                end_word += end_off + 1;
            } else {
                *p = '\0';
                break;
            }
        }

        for (p++; p < end_word; p++) {
            *p = '\0';
        }
    }

    if (*end_word == '\0') {
        *buf = end_word;
    } else {

#if defined(__GNUC__) || defined(__MINGW32__)
        // Disable spurious conversion warning for GCC
#if GCC_VERSION >= 40500
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif // GCC_VERSION >= 40500
#endif // defined(__GNUC__) || defined(__MINGW32__)

        end_whitespace = end_word + strspn(&end_word[1], whitespace) + 1;

#if defined(__GNUC__) || defined(__MINGW32__)
#if GCC_VERSION >= 40500
#pragma GCC diagnostic pop
#endif // GCC_VERSION >= 40500
#endif // defined(__GNUC__) || defined(__MINGW32__)

        for (p = end_word; p < end_whitespace; p++) {
            *p = '\0';
        }

        *buf = end_whitespace;
    }

    return begin_word;
}

/* Skip the characters until one of the delimiters characters found.
 * 0-terminate resulting word. Skip the delimiter and following whitespaces.
 * Advance pointer to buffer to the next word. Return found 0-terminated
 * word.
 * Delimiters can be quoted with quotechar. */
static char*
skip_quoted(char** buf,
            const char* delimiters,
            const char* whitespace,
            char quotechar)
{
    char *p, *begin_word, *end_word, *end_whitespace;

    begin_word = *buf;
    end_word   = begin_word + strcspn(begin_word, delimiters);

    /* Check for quotechar */
    if (end_word > begin_word) {
        p = end_word - 1;
        while (*p == quotechar) {
            /* While the delimiter is quoted, look for the next delimiter.
             */
            /* This happens, e.g., in calls from parse_auth_header,
             * if the user name contains a " character. */

            /* If there is anything beyond end_word, copy it. */
            if (*end_word != '\0') {
                size_t end_off = strcspn(end_word + 1, delimiters);
                memmove(p, end_word, end_off + 1);
                p += end_off; /* p must correspond to end_word - 1 */
                end_word += end_off + 1;
            } else {
                *p = '\0';
                break;
            }
        }
        for (p++; p < end_word; p++) {
            *p = '\0';
        }
    }

    if (*end_word == '\0') {
        *buf = end_word;
    } else {

#if defined(GCC_DIAGNOSTIC)
/* Disable spurious conversion warning for GCC */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif /* defined(GCC_DIAGNOSTIC) */

        end_whitespace = end_word + strspn(&end_word[1], whitespace) + 1;

#if defined(GCC_DIAGNOSTIC)
#pragma GCC diagnostic pop
#endif /* defined(GCC_DIAGNOSTIC) */

        for (p = end_word; p < end_whitespace; p++) {
            *p = '\0';
        }

        *buf = end_whitespace;
    }

    return begin_word;
}

///////////////////////////////////////////////////////////////////////////////
// websrv_parseHeader
//

bool
websrv_parseHeader(std::map<std::string, std::string>& hdrmap,
                   std::string& header)
{
    char *name, *value, *s;

    // Make modifiable copy of the auth header
    char* pbuf = new char[header.length() + 1];
    if (NULL == pbuf)
        return false;
    (void)vscp_strlcpy(pbuf, (const char*)header.c_str() + 7, sizeof(pbuf));
    s = pbuf;

    // Parse authorization header
    for (;;) {

        // Gobble initial spaces
        while (isspace(*(unsigned char*)s)) {
            s++;
        }

        name = skip_quoted(&s, "=", " ", 0);

        // Value is either quote-delimited, or ends at first comma or
        if (s[0] == '\"') {
            s++;
            value = skip_quoted(&s, "\"", " ", '\\');
            if (s[0] == ',') {
                s++;
            }
        } else {
            value = skip_quoted(&s, ", ", " ", 0); // IE uses commas, FF
                                                   // uses spaces
        }

        if (*name == '\0') {
            break;
        }

        // Add entry to map
        hdrmap[std::string(name)] = std::string(value);
    }

    delete[] pbuf;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// websrv_getHeaderElement
//

bool
websrv_getHeaderElement(std::map<std::string, std::string>& hdrmap,
                        const std::string& name,
                        std::string& value)
{
    // Must be value/name pairs
    if (!hdrmap.size())
        return false;

    // Check if key is available
    if (hdrmap.end() == hdrmap.find(name)) {
        return false; // no
    }

    // Get key value
    value = hdrmap[name];

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//                     WEBSERVER SESSION HANDLING
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// websrv_get_session
//

struct websrv_session*
websrv_get_session(struct mg_connection* conn)
{
    struct websrv_session* pSession = NULL;
    struct mg_context* ctx;
    const struct mg_request_info* reqinfo;

    // Check pointers
    if (!conn || !(ctx = mg_get_context(conn)) ||
        !(reqinfo = mg_get_request_info(conn))) {
        return NULL;
    }

    // Get the session cookie
    const char* pheader = mg_get_header(conn, "cookie");
    if (NULL == pheader)
        return NULL;

    std::map<std::string, std::string> hdrarray;
    std::string header = std::string(pheader);
    websrv_parseHeader(hdrarray, header);

    // Get session
    std::string value;
    if (!websrv_getHeaderElement(hdrarray, "vscp-web-sid", value)) {
        return NULL;
    }

    // find existing session
    pthread_mutex_lock(&gpobj->m_mutex_websrvSession);
    std::list<struct websrv_session*>::iterator iter;
    for (iter = gpobj->m_web_sessions.begin();
         iter != gpobj->m_web_sessions.end();
         ++iter) {
        pSession = *iter;
        if (0 == strcmp(value.c_str(), pSession->m_sid)) {
            pSession->lastActiveTime = time(NULL);
            break;
        }
    }
    pthread_mutex_unlock(&gpobj->m_mutex_websrvSession);

    return pSession;
}

///////////////////////////////////////////////////////////////////////////////
// websrv_add_session
//

websrv_session*
websrv_add_session(struct mg_connection* conn)
{
    std::string user;
    struct websrv_session* pSession;
    struct mg_context* ctx;
    const struct mg_request_info* reqinfo;

    // Check pointers
    if (!conn || !(ctx = mg_get_context(conn)) ||
        !(reqinfo = mg_get_request_info(conn))) {
        return 0;
    }

    // Parse "Authorization:" header, fail fast on parse error
    const char* pheader = mg_get_header(conn, "Authorization");
    if (NULL == pheader)
        return NULL;

    std::map<std::string, std::string> hdrarray;
    std::string header = std::string(pheader);
    websrv_parseHeader(hdrarray, header);

    // Get username
    if (!websrv_getHeaderElement(hdrarray, "username", user)) {
        return NULL;
    }

    // Create fresh session
    pSession = new struct websrv_session;
    if (NULL == pSession) {
        return NULL;
    }
    memset(pSession, 0, sizeof(websrv_session));

    // Generate a random session ID
    unsigned char iv[16];
    char hexiv[33];
    getRandomIV(iv, 16); // Generate 16 random bytes
    memset(hexiv, 0, sizeof(hexiv));
    vscp_byteArray2HexStr(hexiv, iv, 16);

    memset(pSession->m_sid, 0, sizeof(pSession->m_sid));
    memcpy(pSession->m_sid, hexiv, 32);

    mg_printf(conn,
              "HTTP/1.1 301 Found\r\n"
              "Set-Cookie: vscp-web-sid=%s; max-age=3600; http-only\r\n"
              "Location: /\r\n"
              "Content-Length: 0r\n\r\n",
              pSession->m_sid);

    pSession->m_pUserItem    = gpobj->m_userList.getUser(std::string(user));
    pSession->lastActiveTime = time(NULL);

    pSession->m_pClientItem = new CClientItem(); // Create client
    if (NULL == pSession->m_pClientItem) {
        syslog(LOG_ERR,
               "[websrv] New session: Unable to create client object.");
        delete pSession;
        return NULL;
    }
    pSession->m_pClientItem->bAuthenticated = false; // Not authenticated in yet
    vscp_clearVSCPFilter(&pSession->m_pClientItem->m_filter); // Clear filter

    // This is an active client
    pSession->m_pClientItem->m_bOpen = false;
    pSession->m_pClientItem->m_dtutc = vscpdatetime::Now();
    pSession->m_pClientItem->m_type =
      CLIENT_ITEM_INTERFACE_TYPE_CLIENT_WEBSOCKET;
    pSession->m_pClientItem->m_strDeviceName = ("Internal web server client.");
    

    // Add the client to the Client List
    pthread_mutex_lock(&gpobj->m_clientList.m_mutexItemList);
    if (!gpobj->addClient(pSession->m_pClientItem)) {
        // Failed to add client
        delete pSession->m_pClientItem;
        pSession->m_pClientItem = NULL;
        pthread_mutex_unlock(&gpobj->m_clientList.m_mutexItemList);
        syslog(LOG_ERR,
               ("WEB server: Failed to add client. Terminating thread."));
        return NULL;
    }
    pthread_mutex_unlock(&gpobj->m_clientList.m_mutexItemList);

    // Add to linked list
    pthread_mutex_lock(&gpobj->m_mutex_websrvSession);
    gpobj->m_web_sessions.push_back(pSession);
    pthread_mutex_unlock(&gpobj->m_mutex_websrvSession);

    return pSession;
}

///////////////////////////////////////////////////////////////////////////////
// websrv_GetCreateSession
//

struct websrv_session*
websrv_getCreateSession(struct mg_connection* conn)
{
    struct websrv_session* pSession;
    struct mg_context* ctx;
    const struct mg_request_info* reqinfo;

    // Check pointers
    if (!conn || !(ctx = mg_get_context(conn)) ||
        !(reqinfo = mg_get_request_info(conn))) {
        return NULL;
    }

    if (NULL == (pSession = websrv_get_session(conn))) {

        // Add session cookie
        pSession = websrv_add_session(conn);
    }

    return pSession;
}

///////////////////////////////////////////////////////////////////////////////
// websrv_expire_sessions
//

void
websrv_expire_sessions(struct mg_connection* conn)
{
    time_t now;

    now = time(NULL);

    pthread_mutex_lock(&gpobj->m_mutex_websrvSession);
    std::list<struct websrv_session*>::iterator it;
    for (it = gpobj->m_web_sessions.begin(); it != gpobj->m_web_sessions.end();
         /* inline */) {
        struct websrv_session* pSession = *it;
        if ((now - pSession->lastActiveTime) > (60 * 60)) {
            it = gpobj->m_web_sessions.erase(it);
            delete pSession;
        } else {
            ++it;
        }
    }
    pthread_mutex_unlock(&gpobj->m_mutex_websrvSession);
}

///////////////////////////////////////////////////////////////////////////////
// check_admin_authorization
//
// Only the admin user has access to /vscp/....
//

static int
check_admin_authorization(struct mg_connection* conn, void* cbdata)
{
    const char* auth_header;
    char buf[8192];
    CUserItem* pUserItem = NULL;
    bool bValidHost;
    struct mg_context* ctx;
    const struct mg_request_info* reqinfo;
    char decoded[2048];
    size_t len;

    memset(buf, 0, sizeof(buf));
    memset(decoded, 0, sizeof(decoded));

    // Check pointers
    if (!conn || !(ctx = mg_get_context(conn)) ||
        !(reqinfo = mg_get_request_info(conn))) {
        syslog(LOG_ERR,
               "[websrv] check_admin_authorization: Pointers are invalid.");
        return WEB_ERROR;
    }

    // Get admin user
    if (NULL ==
        (pUserItem = gpobj->m_userList.getUser(gpobj->m_admin_user.c_str()))) {
        syslog(LOG_ERR,
               "[websrv] check_admin_authorization: Admin user [%s] i not "
               "available.",
               gpobj->m_admin_user.c_str());
        mg_send_basic_access_authentication_request(conn, NULL);
        return 401;
    }

    if ((NULL == (auth_header = mg_get_header(conn, "Authorization"))) ||
        (vscp_strncasecmp(auth_header, "Basic ", 6) != 0)) {
        syslog(LOG_ERR,
               "[websrv] check_admin_authorization: Authorization header or "
               "digest missing for admin log in.");
        mg_send_basic_access_authentication_request(conn, NULL);
        return 401;
    }

    // Make modifiable copy of the auth header (after "Digest")
    (void)vscp_strlcpy(buf, auth_header + 6, sizeof(buf));

    const struct mg_request_info* pri = mg_get_request_info(conn);
    if (NULL == pri) {
        syslog(
          LOG_ERR,
          "[websrv] check_admin_authorization: Failed to to get request info.");
        return WEB_ERROR;
    }

    if (-1 == vscp_base64_decode((const unsigned char*)((const char*)buf),
                                 strlen(buf) + 1,
                                 decoded,
                                 &len)) {
        mg_send_basic_access_authentication_request(conn, NULL);
        return 401;
    }

    std::string str = std::string(decoded);

    std::deque<std::string> tokens;
    vscp_split(tokens, str, ":");

    std::string strUser;
    if (!tokens.empty()) {
        strUser = tokens.front();
        tokens.pop_front();
    }

    std::string strPassword;
    if (!tokens.empty()) {
        strPassword = tokens.front();
        tokens.pop_front();
    }

    pthread_mutex_lock(&gpobj->m_mutex_UserList);
    pUserItem = gpobj->m_userList.validateUser(strUser, strPassword);
    pthread_mutex_unlock(&gpobj->m_mutex_UserList);

    if (NULL == pUserItem) {
        // Password is not correct
        syslog(LOG_ERR,
               "[Webserver Client] Use on host [%s] NOT "
               "allowed connect. User [%s]. Wrong user/password",
               (const char*)reqinfo->remote_addr,
               (const char*)pUserItem->getUserName().c_str());
        mg_send_basic_access_authentication_request(conn, NULL);
        return 401;
    }

    // Check if remote ip is valid
    pthread_mutex_lock(&gpobj->m_mutex_UserList);
    bValidHost =
      (1 == pUserItem->isAllowedToConnect(inet_addr(reqinfo->remote_addr)));
    pthread_mutex_unlock(&gpobj->m_mutex_UserList);
    if (!bValidHost) {
        // Host is not allowed to connect
        syslog(LOG_ERR,
               "[Webserver Client] Host [%s] "
               "NOT allowed to connect. User [%s]",
               (const char*)reqinfo->remote_addr,
               (const char*)pUserItem->getUserName().c_str());
        mg_send_basic_access_authentication_request(conn, NULL);
        return 401;
    }

    return WEB_OK;
}

///////////////////////////////////////////////////////////////////////////////
// check_rest_authorization
//
// Dummy for REST authentication
//

static int
check_rest_authorization(struct mg_connection* conn, void* cbdata)
{
    return WEB_OK;
}

// -----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// Log a message
//

static int
log_message(const struct mg_connection* conn, const char* message)
{
    syslog(LOG_INFO, "[websrv] %s", message);
    return WEB_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Log server access
//

static int
log_access(const struct mg_connection* conn, const char* message)
{
    // TODO
    return WEB_OK;
}

////////////////////////////////////////////////////////////////////////////////
// vscp_mainPage
//

static int
vscp_mainpage(struct mg_connection* conn, void* cbdata)
{
    // Check pointer
    if (NULL == conn)
        return 0;

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n"
              "Content-Type: text/html; charset=utf-8\r\n"
              "Connection: close\r\n\r\n");

    mg_printf(conn, WEB_COMMON_HEAD, "VSCP - Control");
    mg_printf(conn, WEB_STYLE_START);
    mg_write(conn, WEB_COMMON_CSS, strlen(WEB_COMMON_CSS)); // CSS style Code
    mg_printf(conn, WEB_STYLE_END);
    mg_write(conn,
             WEB_COMMON_JS,
             strlen(WEB_COMMON_JS)); // Common Javascript code

    mg_printf(conn, WEB_COMMON_HEAD_END_BODY_START);
    // Insert server url into navigation menu
    mg_printf(conn, WEB_COMMON_MENU);

    mg_printf(conn, "<span align=\"center\">");
    mg_printf(
      conn,
      "<h4> Welcome to the VSCP server local admin control interface.</h4>");
    mg_printf(conn, "</span>");
    mg_printf(conn, "<span style=\"text-indent:50px;\"><p>");
    mg_printf(
      conn,
      "<img src=\"http://vscp.org/images/vscp_logo.png\" width=\"100\">");
    mg_printf(conn, "</p></span>");
    mg_printf(conn, "<span style=\"text-indent:50px;\"><p>");
    mg_printf(conn, " <b>Version:</b> ");
    mg_printf(conn, VSCPD_DISPLAY_VERSION);
    mg_printf(conn, "</p><p>");
    mg_printf(conn, VSCPD_COPYRIGHT_HTML);
    mg_printf(conn, "</p></span>");

    mg_printf(conn, WEB_COMMON_END, VSCPD_COPYRIGHT_HTML); // Common end code
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// vscp_password
//

static int
vscp_password(struct mg_connection* conn, void* cbdata)
{
    int i;
    uint8_t salt[16];
    char buf[512];
    uint8_t resultbuf[512];

    memset(buf, 0, sizeof(buf));

    // Check pointer
    if (NULL == conn)
        return 0;

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n"
              "Content-Type: text/html; charset=utf-8\r\n"
              "Connection: close\r\n\r\n");

    mg_printf(conn, WEB_COMMON_HEAD, "Password key generation");
    mg_printf(conn, WEB_STYLE_START);
    mg_write(conn, WEB_COMMON_CSS, strlen(WEB_COMMON_CSS)); // CSS style Code
    mg_printf(conn, WEB_STYLE_END);
    mg_write(conn,
             WEB_COMMON_JS,
             strlen(WEB_COMMON_JS)); // Common Javascript code
    mg_printf(conn, "<style>table, th, td { border: 0px solid black;}</style>");

    mg_printf(conn, WEB_COMMON_HEAD_END_BODY_START);
    mg_printf(conn, WEB_COMMON_MENU);

    const struct mg_request_info* reqinfo = mg_get_request_info(conn);
    if (NULL == reqinfo)
        return 0;

    // password
    const char* password = "";
    if (NULL != reqinfo->query_string) {
        if (mg_get_var(reqinfo->query_string,
                       strlen(reqinfo->query_string),
                       "pw",
                       buf,
                       sizeof(buf)) > 0) {
            password = buf;
        }
    }

    mg_printf(conn, "<h1 id=\"header\">Password key generation</h1><br>");

    mg_printf(conn, "<p>Enter clear text password to generate key.</p>");

    if (0 == strlen(password)) {
        mg_printf(conn, "<form action=\"/vscp/password\"><table>");
        mg_printf(conn,
                  "<tr><td width=\"10%%\"><b>Password</b></td><td><input "
                  "type=\"password\" "
                  "value=\"\" name=\"pw\"></td><tr>");
        mg_printf(
          conn,
          "<tr><td> <td><input type=\"submit\" value=\"Generate\"></td><tr>");
        mg_printf(conn, "</table></form>");
    } else {

        // Get random IV
        if (16 != getRandomIV(salt, 16)) {
            printf("Unable to generate IV. Terminating.\n");
            return -1;
        }

        fastpbkdf2_hmac_sha256((const uint8_t*)password,
                               strlen(password),
                               salt,
                               16,
                               70000,
                               resultbuf,
                               32);

        mg_printf(conn, "<table>");
        mg_printf(conn, "</td><tr>");
        mg_printf(conn, "<tr><td><b>Generated password: </b></td><td>");
        for (i = 0; i < 16; i++) {
            mg_printf(conn, "%02X", salt[i]);
        }
        mg_printf(conn, ";");
        for (i = 0; i < 32; i++) {
            mg_printf(conn, "%02X", resultbuf[i]);
        }
        mg_printf(conn, "</td><tr>");
        mg_printf(conn, "</table>");
    }

    return WEB_OK;
}

////////////////////////////////////////////////////////////////////////////////
// vscp_restart
//

static int
vscp_restart(struct mg_connection* conn, void* cbdata)
{
    // uint8_t salt[16];
    char buf[512];
    // uint8_t resultbuf[512];

    memset(buf, 0, sizeof(buf));

    // Check pointer
    if (NULL == conn)
        return 0;

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n"
              "Content-Type: text/html; charset=utf-8\r\n"
              "Connection: close\r\n\r\n");

    mg_printf(conn, WEB_COMMON_HEAD, "VSCP Server restart");
    mg_printf(conn, WEB_STYLE_START);
    mg_write(conn, WEB_COMMON_CSS, strlen(WEB_COMMON_CSS)); // CSS style Code
    mg_printf(conn, WEB_STYLE_END);
    mg_write(conn,
             WEB_COMMON_JS,
             strlen(WEB_COMMON_JS)); // Common JavaScript code
    mg_printf(conn, "<style>table, th, td { border: 0px solid black;}</style>");

    mg_printf(conn, WEB_COMMON_HEAD_END_BODY_START);
    mg_printf(conn, WEB_COMMON_MENU);

    const struct mg_request_info* reqinfo = mg_get_request_info(conn);
    if (NULL == reqinfo)
        return 0;

    // restart password
    const char* password = NULL;
    if (NULL != reqinfo->query_string) {
        if (mg_get_var(reqinfo->query_string,
                       strlen(reqinfo->query_string),
                       "pw",
                       buf,
                       sizeof(buf)) > 0) {
            password = buf;
        }
    }

    mg_printf(conn, "<h1 id=\"header\">Restart the VSCP Server</h1><br>");

    if (NULL == password) {
        mg_printf(conn, "<form action=\"/vscp/restart\"><table>");
        mg_printf(conn,
                  "<tr><td width=\"10%%\"><b>vscptoken</b></td><td><input "
                  "type=\"password\" "
                  "value=\"\" name=\"pw\"></td><tr>");
        mg_printf(
          conn,
          "<tr><td> <td><input type=\"submit\" value=\"RESTART\"></td><tr>");
        mg_printf(conn, "</table></form>");
    } else {

        if (gpobj->m_vscptoken != std::string(password)) {
            fprintf(stderr, "websrv: Passphrase is wrong.\n");
            mg_printf(conn, "Passphrase is wrong - will not restart!<br>");
            return WEB_OK;
        }

        gpobj->m_bQuit = true; // Quit main loop
        gbStopDaemon   = true;
        gbRestart      = true; // Restart NOT shutdown
        sleep(1);
        fprintf(stderr, "websrv: Shutdown in progress 1.\n");
        sleep(1);
        fprintf(stderr, "websrv: Shutdown in progress 2.\n");
        sleep(1);
        fprintf(stderr, "websrv: Shutdown in progress 3.\n");

        mg_printf(conn, "<table>");
        mg_printf(conn, "</td><tr>");
        mg_printf(
          conn,
          "<tr><td><b>VSCP Server will now be restarted... </b></td><td>");
        mg_printf(conn, "</td><tr>");
        mg_printf(conn, "</td><tr>");
        mg_printf(conn, "<tr><td><b>This may take a while... </b></td><td>");
        mg_printf(conn, "</td><tr>");
        mg_printf(conn, "</table>");
    }

    return WEB_OK;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_interface
//

static int
vscp_interface(struct mg_connection* conn, void* cbdata)
{
    // Check pointer
    if (NULL == conn)
        return 0;

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n"
              "Content-Type: text/html; charset=utf-8\r\n"
              "Connection: close\r\n\r\n");

    mg_printf(conn, WEB_COMMON_HEAD, "VSCP - Control");
    mg_printf(conn, WEB_STYLE_START);
    mg_write(conn, WEB_COMMON_CSS, strlen(WEB_COMMON_CSS)); // CSS style Code
    mg_printf(conn, WEB_STYLE_END);
    mg_write(conn,
             WEB_COMMON_JS,
             strlen(WEB_COMMON_JS)); // Common Javascript code

    mg_printf(conn, WEB_COMMON_HEAD_END_BODY_START);
    // Insert server url into navigation menu
    mg_printf(conn, WEB_COMMON_MENU);

    mg_printf(conn, WEB_IFLIST_BODY_START);
    mg_printf(conn, WEB_IFLIST_TR_HEAD);

    std::string strGUID;
    std::string strBuf;

    // Display Interface List
    pthread_mutex_lock(&gpobj->m_clientList.m_mutexItemList);
    std::deque<CClientItem*>::iterator it;
    for (it = gpobj->m_clientList.m_itemList.begin();
         it != gpobj->m_clientList.m_itemList.end();
         ++it) {

        CClientItem* pItem = *it;
        pItem->m_guid.toString(strGUID);

        mg_printf(conn, WEB_IFLIST_TR);

        // Client id
        mg_printf(conn, WEB_IFLIST_TD_CENTERED);
        mg_printf(conn, "%d", pItem->m_clientID);
        mg_printf(conn, "</td>");

        // Interface type
        mg_printf(conn, WEB_IFLIST_TD_CENTERED);
        mg_printf(conn, "%d", pItem->m_type);
        mg_printf(conn, "</td>");

        // GUID
        mg_printf(conn, WEB_IFLIST_TD_GUID);
        mg_printf(conn, "%s", (const char*)strGUID.substr(0, 24).c_str());
        mg_printf(conn, "<br>");
        mg_printf(conn, "%s", (const char*)strGUID.substr(24).c_str());
        mg_printf(conn, "</td>");

        // Interface name
        mg_printf(conn, "<td>");

        size_t pos;
        std::string strDeviceName;
        if (std::string::npos != (pos = pItem->m_strDeviceName.find("|"))) {
            strDeviceName = vscp_str_left(pItem->m_strDeviceName, pos);
            vscp_trim(strDeviceName);
        }

        mg_printf(conn, "%s", (const char*)strDeviceName.c_str());
        mg_printf(conn, "</td>");

        // Start date
        mg_printf(conn, "<td>");
        mg_printf(conn, "%s", pItem->m_dtutc.getISODateTime().c_str());
        mg_printf(conn, "</td>");

        mg_printf(conn, "</tr>");
    }

    pthread_mutex_unlock(&gpobj->m_clientList.m_mutexItemList);

    mg_printf(conn, WEB_IFLIST_TABLE_END);

    mg_printf(
      conn,
      "<br>All interfaces on the VSCP server is listed here. "
      "This is drivers as well as clients connected to one of the VSCP servers "
      "interfaces. It is possible to see events coming in on a on a "
      "specific interface and send events on just one of the interfaces. "
      "This is mostly used on the driver interfaces but is possible on "
      "all interfaces<br>");

    mg_printf(conn, "<br><b>Interface Types</b><br>");
    mg_printf(conn,
              "%d - Unknown (you should not see this).<br>",
              (uint8_t)CLIENT_ITEM_INTERFACE_TYPE_NONE);
    mg_printf(conn,
              "%d - Internal daemon client.<br>",
              (uint8_t)CLIENT_ITEM_INTERFACE_TYPE_CLIENT_INTERNAL);
    mg_printf(conn,
              "%d - Level I (CANAL) Driver.<br>",
              (uint8_t)CLIENT_ITEM_INTERFACE_TYPE_DRIVER_LEVEL1);
    mg_printf(conn,
              "%d - Level II Driver.<br>",
              (uint8_t)CLIENT_ITEM_INTERFACE_TYPE_DRIVER_LEVEL2);
    mg_printf(conn,
              "%d - TCP/IP Client.<br>",
              (uint8_t)CLIENT_ITEM_INTERFACE_TYPE_CLIENT_TCPIP);
    mg_printf(conn,
              "%d - UDP Client.<br>",
              (uint8_t)CLIENT_ITEM_INTERFACE_TYPE_CLIENT_UDP);
    mg_printf(conn,
              "%d - Web Server Client.<br>",
              (uint8_t)CLIENT_ITEM_INTERFACE_TYPE_CLIENT_WEB);
    mg_printf(conn,
              "%d - WebSocket Client.<br>",
              (uint8_t)CLIENT_ITEM_INTERFACE_TYPE_CLIENT_WEBSOCKET);
    mg_printf(conn,
              "%d - REST Client.<br>",
              (uint8_t)CLIENT_ITEM_INTERFACE_TYPE_CLIENT_REST);
    mg_printf(conn,
              "%d - Multicast.<br>",
              (uint8_t)CLIENT_ITEM_INTERFACE_TYPE_CLIENT_MULTICAST);
    mg_printf(conn,
              "%d - Multicast Client.<br>",
              (uint8_t)CLIENT_ITEM_INTERFACE_TYPE_CLIENT_MULTICAST_CH);
    mg_printf(conn,
              "%d - MQTT Client.<br>",
              (uint8_t)CLIENT_ITEM_INTERFACE_TYPE_CLIENT_MQTT);
    mg_printf(conn,
              "%d - COAP Client.<br>",
              (uint8_t)CLIENT_ITEM_INTERFACE_TYPE_CLIENT_COAP);
    mg_printf(conn,
              "%d - Discovery.<br>",
              (uint8_t)CLIENT_ITEM_INTERFACE_TYPE_CLIENT_DISCOVERY);
    mg_printf(conn,
              "%d - Javascript Client.<br>",
              (uint8_t)CLIENT_ITEM_INTERFACE_TYPE_CLIENT_JAVASCRIPT);
    mg_printf(conn,
              "%d - Lua Client.<br>",
              (uint8_t)CLIENT_ITEM_INTERFACE_TYPE_CLIENT_LUA);

    mg_printf(conn, WEB_COMMON_END, VSCPD_COPYRIGHT_HTML); // Common end code

    return WEB_OK;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_interface
//

static int
vscp_interface_info(struct mg_connection* conn, void* cbdata)
{
    // Check pointer
    if (NULL == conn)
        return 0;

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n"
              "Content-Type: text/html; charset=utf-8\r\n"
              "Connection: close\r\n\r\n");

    mg_printf(conn, WEB_COMMON_HEAD, "VSCP - Interface information");
    mg_printf(conn, WEB_STYLE_START);
    mg_write(conn, WEB_COMMON_CSS, strlen(WEB_COMMON_CSS)); // CSS style Code
    mg_printf(conn, WEB_STYLE_END);
    mg_write(conn,
             WEB_COMMON_JS,
             strlen(WEB_COMMON_JS)); // Common Javascript code

    mg_printf(conn, WEB_COMMON_HEAD_END_BODY_START);
    // Insert server url into navigation menu
    mg_printf(conn, WEB_COMMON_MENU);

    mg_printf(conn,
              "<h4>There is no extra information about this interface.</h4>");

    mg_printf(conn, WEB_COMMON_END, VSCPD_COPYRIGHT_HTML); // Common end code

    return WEB_OK;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_log
//

static int
vscp_log(struct mg_connection* conn, void* cbdata)
{
    // Check pointer
    if (NULL == conn)
        return 0;

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n"
              "Content-Type: text/html; charset=utf-8\r\n"
              "Connection: close\r\n\r\n");

    mg_printf(conn, WEB_COMMON_HEAD, "VSCP - Log");
    mg_printf(conn, WEB_STYLE_START);
    mg_write(conn, WEB_COMMON_CSS, strlen(WEB_COMMON_CSS)); // CSS style Code
    mg_printf(conn, WEB_STYLE_END);
    mg_write(conn,
             WEB_COMMON_JS,
             strlen(WEB_COMMON_JS)); // Common Javascript code

    mg_printf(conn, WEB_COMMON_HEAD_END_BODY_START);
    // Insert server url into navigation menu
    mg_printf(conn, WEB_COMMON_MENU);

    mg_printf(conn, WEB_LOGLIST_BODY_START);
    mg_printf(conn, WEB_LOGLIST_TR_HEAD);

    std::string line;
    std::ifstream logFile("/var/log/syslog");
    while (!logFile.fail() && !logFile.eof() && getline(logFile, line)) {
        if (std::string::npos != line.find("vscpd:")) {
            mg_printf(conn, "<tr><td>%s</td></tr>", line.c_str());
        }
    }

    mg_printf(conn, WEB_LOGLIST_TABLE_END);
    mg_printf(conn, WEB_COMMON_END, VSCPD_COPYRIGHT_HTML); // Common end code

    return WEB_OK;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_client
//

static int
vscp_client(struct mg_connection* conn, void* cbdata)
{
    // char buf[80];
    std::string str;

    // Check pointer
    if (NULL == conn)
        return 0;

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
              "close\r\n\r\n");

    mg_printf(conn, WEB_COMMON_HEAD, "VSCP - Session");
    mg_printf(conn, WEB_STYLE_START);
    mg_write(conn, WEB_COMMON_CSS, sizeof(WEB_COMMON_CSS)); // CSS style Code
    mg_printf(conn, WEB_STYLE_END);
    mg_write(conn,
             WEB_COMMON_JS,
             sizeof(WEB_COMMON_JS)); // Common JavaScript code
    mg_printf(conn, "<meta http-equiv=\"refresh\" content=\"5;url=/vscp");
    mg_printf(conn, "\">");
    mg_printf(conn, WEB_COMMON_HEAD_END_BODY_START);

    // navigation menu
    mg_printf(conn, WEB_COMMON_MENU);
    mg_printf(conn, "<b>Client functionality is not yet implemented!</b>");

    return WEB_OK;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_configure_list
//

static int
vscp_configure_list(struct mg_connection* conn, void* cbdata)
{
    std::string str;

    // Check pointer
    if (NULL == conn)
        return 0;

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n"
              "Content-Type: text/html; charset=utf-8\r\n"
              "Connection: close\r\n\r\n");

    mg_printf(conn, WEB_COMMON_HEAD, "vscpd - Configuration");
    mg_printf(conn, WEB_STYLE_START);
    mg_write(conn, WEB_COMMON_CSS, sizeof(WEB_COMMON_CSS)); // CSS style Code
    mg_printf(conn, WEB_STYLE_END);
    mg_write(conn,
             WEB_COMMON_JS,
             sizeof(WEB_COMMON_JS)); // Common JavaScript code
    mg_printf(conn, WEB_COMMON_HEAD_END_BODY_START);

    // navigation menu
    mg_printf(conn, WEB_COMMON_MENU);

    mg_printf(conn, "<br><br><br>");
    mg_printf(conn, "<h1 id=\"header\">VSCP - Current configuration</h1>");
    mg_printf(conn, "<br>");

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    mg_printf(conn, "<h4 id=\"header\" >&nbsp;Server</h4> ");
    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>VSCP Server version:</b> ");
    mg_printf(conn, VSCPD_DISPLAY_VERSION);
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Operating system:</b> ");
#ifdef _WIN32
    mg_printf(conn, "%s", "Windows 32-bit");
#elif _WIN64
#elif __APPLE__ || __MACH__
      mg_printf( conn, "%s", "Mac OSX";
#elif __linux__
    struct utsname uts;
    uname(&uts);
    mg_printf(conn, "%s - ", uts.sysname);
    mg_printf(conn, "%s - ", uts.nodename);
    mg_printf(conn, "%s - ", uts.release);
    mg_printf(conn, "%s - ", uts.version);
    mg_printf(conn, "%s <br>", uts.machine);
#elif __FreeBSD__
    mg_printf(conn, "%s", "FreeBSD");
#elif __unix || __unix__
    mg_printf(conn, "%s", "Unix");
#else
    mg_printf(conn, "%s", "Other");
#endif

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Type:</b> ");
    if (vscp_is64Bit()) {
        mg_printf(conn, " 64-bit ");
    } else {
        mg_printf(conn, " 32-bit ");
    }

    if (vscp_isLittleEndian()) {
        mg_printf(conn, " Little endian ");
    } else {
        mg_printf(conn, " Big endian ");
    }
    mg_printf(conn, "<br>");

    double vm_usage, resident_set;
    vscp_mem_usage(vm_usage, resident_set);
    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Memory usage:</b> ");
    mg_printf(conn,
              "<b>VM</b>: %0.0f  <b>RSS</b>: %0.0f",
              vm_usage,
              resident_set);
    mg_printf(conn, " <br>");

    // Debuglevel
    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Debug:</b> ");
    for (int i = 0; i < 8; i++) {
        mg_printf(conn, "%02X ", gpobj->m_debugFlags[i]);
    }

#ifdef NDEBUG
    mg_printf(conn, "Build is release ");
#else
    mg_printf(conn, " - Build is debug ");
#endif

    mg_printf(conn, "<br> ");

    // Server GUID
    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Server GUID:</b> ");
    gpobj->m_guid.toString(str);
    mg_printf(conn, "%s", (const char*)str.c_str());
    mg_printf(conn, "<br> ");

    // Client buffer size
    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Client buffer size:</b>");
    mg_printf(conn, "%d", gpobj->m_maxItemsInClientReceiveQueue);
    mg_printf(conn, "<br> ");

    mg_printf(conn, "<hr>");

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    mg_printf(conn,
              "&nbsp;&nbsp;&nbsp;&nbsp;<b>OpenSSL version:</b> %s <br>",
              SSLeay_version(SSLEAY_VERSION));
    mg_printf(conn,
              "&nbsp;&nbsp;&nbsp;&nbsp;<b>SQLite version:</b> %s <br>",
              SQLITE_VERSION);
    mg_printf(conn,
              "&nbsp;&nbsp;&nbsp;&nbsp;<b>Duktape version:</b> %s <br>",
              DUK_GIT_DESCRIBE);
    mg_printf(conn,
              "&nbsp;&nbsp;&nbsp;&nbsp;<b>Civetweb version:</b> %s <br>",
              CIVETWEB_VERSION);
    mg_printf(conn,
              "&nbsp;&nbsp;&nbsp;&nbsp;<b>Lua version:</b> %s <br>",
              LUA_COPYRIGHT);
    mg_printf(
      conn,
      "&nbsp;&nbsp;&nbsp;&nbsp;<b>nlohmann-json version:</b> %d.%d.%d <br>",
      NLOHMANN_JSON_VERSION_MAJOR,
      NLOHMANN_JSON_VERSION_MINOR,
      NLOHMANN_JSON_VERSION_PATCH);

    mg_printf(conn, "<hr>");

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    mg_printf(conn, "<h4 id=\"header\" >&nbsp;TCP/IP</h4> ");

    // TCP/IP interface
    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>TCP/IP interface:</b> ");
    mg_printf(conn, "enabled on <b>interface:</b> '");
    mg_printf(conn, "%s", (const char*)gpobj->m_strTcpInterfaceAddress.c_str());
    mg_printf(conn, "'");
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Encryption:</b> ");
    switch (gpobj->m_encryptionTcpip) {
        case 1:
            mg_printf(conn, "AES-128");
            break;

        case 2:
            mg_printf(conn, "AES-192");
            break;

        case 3:
            mg_printf(conn, "AES-256");
            break;

        default:
            mg_printf(conn, "none");
            break;
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>SSL certificat:</b> ");
    mg_printf(conn,
              "%s",
              (const char*)std::string(gpobj->m_tcpip_ssl_certificate).c_str());
    if (!gpobj->m_tcpip_ssl_certificate.length()) {
        mg_printf(conn, "%s", "Not defined (probably should be).");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>SSL certificat chain:</b>");
    mg_printf(
      conn,
      "%s",
      (const char*)std::string(gpobj->m_tcpip_ssl_certificate_chain).c_str());
    if (!gpobj->m_web_ssl_certificate_chain.length()) {
        mg_printf(conn, "%s", "Not defined (default).");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>SSL verify peer:</b> ");
    mg_printf(conn, "%s", gpobj->m_tcpip_ssl_verify_peer ? "true" : "false");
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>SSL CA path:</b> ");
    mg_printf(conn,
              "%s",
              (const char*)std::string(gpobj->m_tcpip_ssl_ca_path).c_str());
    if (!gpobj->m_web_ssl_ca_path.length()) {
        mg_printf(conn, "%s", "Not defined (default).");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>SSL CA file:</b> ");
    mg_printf(conn,
              "%s",
              (const char*)std::string(gpobj->m_tcpip_ssl_ca_file).c_str());
    if (!gpobj->m_web_ssl_ca_file.length()) {
        mg_printf(conn, "%s", "Not defined (default).");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>SSL verify depth:</b> ");
    mg_printf(conn, "%d", (int)gpobj->m_tcpip_ssl_verify_depth);
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>SSL verify paths:</b> ");
    mg_printf(conn,
              "%s",
              gpobj->m_tcpip_ssl_default_verify_paths ? "true" : "false");
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>SSL cipher list:</b> ");
    mg_printf(conn,
              "%s",
              (const char*)std::string(gpobj->m_tcpip_ssl_cipher_list).c_str());
    if (!gpobj->m_tcpip_ssl_cipher_list.length()) {
        mg_printf(conn, "%s", "Not defined (default).");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>SSL protocol version:</b>");
    mg_printf(conn, "%d", (int)gpobj->m_tcpip_ssl_protocol_version);
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>SSL short trust:</b> ");
    mg_printf(conn, "%s", gpobj->m_tcpip_ssl_short_trust ? "true" : "false");
    mg_printf(conn, "<br>");

    mg_printf(conn, "<hr>");

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * ** * * * * *

    mg_printf(conn, "<h4 id=\"header\" >&nbsp;Web server</h4> ");

    mg_printf(conn,
              "&nbsp;&nbsp;&nbsp;&nbsp;<b>Web server functionality </b>is ");
    if (gpobj->m_web_bEnable) {
        mg_printf(conn, "enabled.<br>");
    } else {
        mg_printf(conn, "disabled.<br>");
    }

    mg_printf(conn,
              "&nbsp;&nbsp;&nbsp;&nbsp;<b>Listening port(s)/interface(s):</b>");
    mg_printf(conn, "%s", (const char*)gpobj->m_web_listening_ports.c_str());
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Rootfolder:</b> ");
    mg_printf(conn,
              "%s",
              (const char*)std::string(gpobj->m_web_document_root).c_str());
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Authdomain:</b> ");
    mg_printf(
      conn,
      "%s",
      (const char*)std::string(gpobj->m_web_authentication_domain).c_str());
    if (gpobj->m_enable_auth_domain_check) {
        mg_printf(conn, " [will be checked].");
    } else {
        mg_printf(conn, " [will not be checked].");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Index files:</b> ");
    mg_printf(conn,
              "%s",
              (const char*)std::string(gpobj->m_web_index_files).c_str());
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>SSL certificat:</b> ");
    mg_printf(conn,
              "%s",
              (const char*)std::string(gpobj->m_web_ssl_certificate).c_str());
    if (!gpobj->m_web_ssl_certificate.length()) {
        mg_printf(conn, "%s", "Not defined (probably should be).");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>SSL certificat chain:</b>");
    mg_printf(
      conn,
      "%s",
      (const char*)std::string(gpobj->m_web_ssl_certificate_chain).c_str());
    if (!gpobj->m_web_ssl_certificate_chain.length()) {
        mg_printf(conn, "%s", "Not defined (default).");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>SSL verify peer:</b> ");
    mg_printf(conn, "%s", gpobj->m_web_ssl_verify_peer ? "true" : "false");
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>SSL CA path:</b> ");
    mg_printf(conn,
              "%s",
              (const char*)std::string(gpobj->m_web_ssl_ca_path).c_str());
    if (!gpobj->m_web_ssl_ca_path.length()) {
        mg_printf(conn, "%s", "Not defined (default).");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>SSL CA file:</b> ");
    mg_printf(conn,
              "%s",
              (const char*)std::string(gpobj->m_web_ssl_ca_file).c_str());
    if (!gpobj->m_web_ssl_ca_file.length()) {
        mg_printf(conn, "%s", "Not defined (default).");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>SSL verify depth:</b> ");
    mg_printf(conn, "%d", (int)gpobj->m_web_ssl_verify_depth);
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>SSL verify paths:</b> ");
    mg_printf(conn,
              "%s",
              gpobj->m_web_ssl_default_verify_paths ? "true" : "false");
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>SSL cipher list:</b> ");
    mg_printf(conn,
              "%s",
              (const char*)std::string(gpobj->m_web_ssl_cipher_list).c_str());
    if (!gpobj->m_web_ssl_cipher_list.length()) {
        mg_printf(conn, "%s", "Not defined (default).");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>SSL protocol version:</b>");
    mg_printf(conn, "%d", (int)gpobj->m_web_ssl_protocol_version);
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>SSL short trust:</b> ");
    mg_printf(conn, "%s", gpobj->m_web_ssl_short_trust ? "true" : "false");
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>CGI interpreter:</b> ");
    mg_printf(conn,
              "%s",
              (const char*)std::string(gpobj->m_web_cgi_interpreter).c_str());
    if (!gpobj->m_web_cgi_interpreter.length()) {
        mg_printf(conn, "%s", "Not defined (default).");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>CGI patterns:</b> ");
    mg_printf(conn,
              "%s",
              (const char*)std::string(gpobj->m_web_cgi_patterns).c_str());
    if (!gpobj->m_web_cgi_patterns.length()) {
        mg_printf(conn, "%s", "Not defined (probably should be defined).");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>CGI environment:</b> ");
    mg_printf(conn,
              "%s",
              (const char*)std::string(gpobj->m_web_cgi_environment).c_str());
    if (!gpobj->m_web_cgi_environment.length()) {
        mg_printf(conn, "%s", "Not defined (default).");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Protect URI:</b> ");
    mg_printf(conn,
              "%s",
              (const char*)std::string(gpobj->m_web_protect_uri).c_str());
    if (!gpobj->m_web_protect_uri.length()) {
        mg_printf(conn, "%s", "Not defined (default).");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Trottle:</b> ");
    mg_printf(conn,
              "%s",
              (const char*)std::string(gpobj->m_web_trottle).c_str());
    if (!gpobj->m_web_trottle.length()) {
        mg_printf(conn, "%s", "Not defined (default).");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn,
              "&nbsp;&nbsp;&nbsp;&nbsp;<b>Enable directory listings:</b>");
    mg_printf(conn,
              "%s",
              gpobj->m_web_enable_directory_listing ? "true" : "false");
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Enable keep alive:</b>");
    mg_printf(conn, "%s", gpobj->m_web_enable_keep_alive ? "true" : "false");
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Keep alive timeout in ms:</b>");
    if (-1 == gpobj->m_web_keep_alive_timeout_ms) {
        mg_printf(conn, "%ld", (long)gpobj->m_web_keep_alive_timeout_ms);
    } else {
        mg_printf(conn, "%s", "Not defined (Default: 500) ).");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn,
              "&nbsp;&nbsp;&nbsp;&nbsp;<b>Access control list (ACL):</b>");
    mg_printf(
      conn,
      "%s",
      (const char*)std::string(gpobj->m_web_access_control_list).c_str());
    if (!gpobj->m_web_access_control_list.length()) {
        mg_printf(conn, "%s", "Not defined (default).");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>ExtraMimeTypes:</b> ");
    mg_printf(conn,
              "%s",
              (const char*)std::string(gpobj->m_web_extra_mime_types).c_str());
    if (0 == gpobj->m_web_extra_mime_types.length()) {
        mg_printf(conn, "Set to default.");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Number of threads:</b>");
    if (-1 == gpobj->m_web_num_threads) {
        mg_printf(conn, "%d", (int)gpobj->m_web_num_threads);
    } else {
        mg_printf(conn, "%s", "Not defined (Default: 50) ).");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>HiddenFilePatterns:</b>");
    mg_printf(
      conn,
      "%s",
      (const char*)std::string(gpobj->m_web_hide_file_patterns).c_str());
    if (0 == gpobj->m_web_hide_file_patterns.length()) {
        mg_printf(conn, "Set to default.");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Run as user:</b> ");
    mg_printf(conn, "%s", (const char*)std::string(gpobj->m_runAsUser).c_str());
    if (0 == gpobj->m_runAsUser.length()) {
        mg_printf(conn, "Using vscpd user.");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>URL Rewrites:</b> ");
    mg_printf(
      conn,
      "%s",
      (const char*)std::string(gpobj->m_web_url_rewrite_patterns).c_str());
    if (0 == gpobj->m_web_url_rewrite_patterns.length()) {
        mg_printf(conn, "Set to default.");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Hide file patterns:</b>");
    mg_printf(
      conn,
      "%s",
      (const char*)std::string(gpobj->m_web_hide_file_patterns).c_str());
    if (0 == gpobj->m_web_hide_file_patterns.length()) {
        mg_printf(conn, "Set to default.");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Request timeout:</b> ");
    if (-1 == gpobj->m_web_request_timeout_ms) {
        mg_printf(conn, "%ld", (long)gpobj->m_web_request_timeout_ms);
    } else {
        mg_printf(conn, "%s", "Not defined (Default: 30000) ).");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Linger timeout:</b> ");
    if (-1 == gpobj->m_web_linger_timeout_ms) {
        mg_printf(conn, "%ld", (long)gpobj->m_web_linger_timeout_ms);
    } else {
        mg_printf(conn, "%s", "Not set.");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Enable decode URL:</b>");
    mg_printf(conn, " %s ", gpobj->m_web_decode_url ? " true " : "false");
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Path to global auth file:</b>");
    mg_printf(conn,
              "%s",
              (const char*)std::string(gpobj->m_web_global_auth_file).c_str());
    if (0 == gpobj->m_web_global_auth_file.length()) {
        mg_printf(conn, "Set to default.");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Per directory auth file:</b>");
    mg_printf(
      conn,
      "%s",
      (const char*)std::string(gpobj->m_web_per_directory_auth_file).c_str());
    if (0 == gpobj->m_web_per_directory_auth_file.length()) {
        mg_printf(conn, "Set to default.");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>SSI patterns:</b> ");
    mg_printf(conn,
              "%s",
              (const char*)std::string(gpobj->m_web_ssi_patterns).c_str());
    if (0 == gpobj->m_web_ssi_patterns.length()) {
        mg_printf(conn, "Set to default.");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn,
              "&nbsp;&nbsp;&nbsp;&nbsp;<b>Access control allow origin:</b>");
    mg_printf(conn,
              "%s",
              (const char*)std::string(gpobj->m_web_access_control_allow_origin)
                .c_str());
    if (0 == gpobj->m_web_access_control_allow_origin.length()) {
        mg_printf(conn, "Set to default.");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn,
              "&nbsp;&nbsp;&nbsp;&nbsp;<b>Access control allow methods:</b>");
    mg_printf(
      conn,
      "%s",
      (const char*)std::string(gpobj->m_web_access_control_allow_methods)
        .c_str());
    if (0 == gpobj->m_web_access_control_allow_methods.length()) {
        mg_printf(conn, "Set to default.");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn,
              "&nbsp;&nbsp;&nbsp;&nbsp;<b>Access control allowheaders:</b>");
    mg_printf(
      conn,
      "%s",
      (const char*)std::string(gpobj->m_web_access_control_allow_headers)
        .c_str());
    if (0 == gpobj->m_web_access_control_allow_headers.length()) {
        mg_printf(conn, "Set to default.");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Path to error pages:</b>");
    mg_printf(conn,
              "%s",
              (const char*)std::string(gpobj->m_web_error_pages).c_str());
    if (0 == gpobj->m_web_error_pages.length()) {
        mg_printf(conn, "Set to default.");
    }
    mg_printf(conn, "<br>");

    mg_printf(
      conn,
      "&nbsp;&nbsp;&nbsp;&nbsp;<b>Enable TCP_NODELAY socket option:</b>");
    if (-1 == gpobj->m_web_tcp_nodelay) {
        mg_printf(conn, "%ld", (long)gpobj->m_web_tcp_nodelay);
    } else {
        mg_printf(conn, "%s", "Not set.");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn,
              "&nbsp;&nbsp;&nbsp;&nbsp;<b>Static file max age(seconds):</b>");
    if (-1 == gpobj->m_web_static_file_max_age) {
        mg_printf(conn, "%ld", (long)gpobj->m_web_static_file_max_age);
    } else {
        mg_printf(conn, "%s", "Not set.");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn,
              "&nbsp;&nbsp;&nbsp;&nbsp;<b>Strict transport security max age"
              "(seconds):</b> ");
    if (-1 == gpobj->m_web_strict_transport_security_max_age) {
        mg_printf(conn,
                  "%ld",
                  (long)gpobj->m_web_strict_transport_security_max_age);
    } else {
        mg_printf(conn, "%s", "Not set.");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Allow sendfile call:</b>");
    mg_printf(conn, "%s", gpobj->m_web_allow_sendfile_call ? "true" : "false");
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Additional headers:</b>");
    mg_printf(conn,
              "%s",
              (const char*)std::string(gpobj->m_web_additional_header).c_str());
    if (0 == gpobj->m_web_additional_header.length()) {
        mg_printf(conn, "Set to default.");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Max request size:</b> ");
    if (-1 == gpobj->m_web_max_request_size) {
        mg_printf(conn, "%ld", (long)gpobj->m_web_max_request_size);
    } else {
        mg_printf(conn, "%s", "Not set.");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn,
              "&nbsp;&nbsp;&nbsp;&nbsp;<b>Allow index script resource:</b>");
    mg_printf(conn,
              "%s",
              gpobj->m_web_allow_index_script_resource ? "true" : "false");
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Duktape script patters:</b>");
    mg_printf(
      conn,
      "%s",
      (const char*)std::string(gpobj->m_web_duktape_script_patterns).c_str());
    if (0 == gpobj->m_web_duktape_script_patterns.length()) {
        mg_printf(conn, "Not set.");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Lua preload file:</b> ");
    mg_printf(conn,
              "%s",
              (const char*)std::string(gpobj->m_web_lua_preload_file).c_str());
    if (0 == gpobj->m_web_lua_preload_file.length()) {
        mg_printf(conn, "Not set.");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Lua script patterns:</b>");
    mg_printf(
      conn,
      "%s",
      (const char*)std::string(gpobj->m_web_lua_script_patterns).c_str());
    if (0 == gpobj->m_web_lua_script_patterns.length()) {
        mg_printf(conn, "Not set.");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Lua server page patterns:</b>");
    mg_printf(
      conn,
      "%s",
      (const char*)std::string(gpobj->m_web_lua_server_page_patterns).c_str());
    if (0 == gpobj->m_web_lua_server_page_patterns.length()) {
        mg_printf(conn, "Not set.");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Lua websocketpatterns:</b>");
    mg_printf(
      conn,
      "%s",
      (const char*)std::string(gpobj->m_web_lua_websocket_patterns).c_str());
    if (0 == gpobj->m_web_lua_websocket_patterns.length()) {
        mg_printf(conn, "Not set.");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Lua background script:</b>");
    mg_printf(
      conn,
      "%s",
      (const char*)std::string(gpobj->m_web_lua_background_script).c_str());
    if (0 == gpobj->m_web_lua_background_script.length()) {
        mg_printf(conn, "Not set.");
    }
    mg_printf(conn, "<br>");

    mg_printf(
      conn,
      "&nbsp;&nbsp;&nbsp;&nbsp;<b>Lua background script parameters:</b>");
    mg_printf(
      conn,
      "%s",
      (const char*)std::string(gpobj->m_web_lua_background_script_params)
        .c_str());
    if (0 == gpobj->m_web_lua_background_script_params.length()) {
        mg_printf(conn, "Not set.");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "<hr>");

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * ** * * * * *

    mg_printf(conn, "<h4 id=\"header\" >&nbsp;Websockets</h4> ");

    mg_printf(conn,
              "&nbsp;&nbsp;&nbsp;&nbsp;<b>Web sockets functionality </b> is ");
    if (gpobj->m_web_bEnable && gpobj->m_bWebsocketsEnable) {
        mg_printf(conn, "enabled.<br>");
    } else {
        mg_printf(conn, "disabled.<br>");
    }

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Websocket document root:</b>");
    mg_printf(
      conn,
      "%s",
      (const char*)std::string(gpobj->m_websocket_document_root).c_str());
    if (0 == gpobj->m_websocket_document_root.length()) {
        mg_printf(conn, "Not set == Same as web root folder");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Websocket timeout (ms):</b>");
    mg_printf(conn, "%ld", gpobj->m_websocket_timeout_ms);
    if (0 == gpobj->m_websocket_timeout_ms) {
        mg_printf(conn, "Set to default: 30000");
    }
    mg_printf(conn, "<br>");

    mg_printf(conn,
              "&nbsp;&nbsp;&nbsp;&nbsp;<b>Web sockets ping-pong functionality "
              "</b> is ");
    if (gpobj->m_web_bEnable && gpobj->bEnable_websocket_ping_pong) {
        mg_printf(conn, "enabled.<br>");
    } else {
        mg_printf(conn, "disabled.<br>");
    }

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Websocket lua pattern:</b>");
    mg_printf(conn,
              "%s",
              (const char*)std::string(gpobj->lua_websocket_pattern).c_str());
    mg_printf(conn, "<br>");

    mg_printf(conn, "<hr>");

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * ** * * * * *

    mg_printf(conn, "<h4 id=\"header\" >&nbsp;REST API</h4> ");

    mg_printf(conn,
              "&nbsp;&nbsp;&nbsp;&nbsp;<b>REST API functionality </b> is ");
    if (gpobj->m_web_bEnable && gpobj->m_bEnableRestApi) {
        mg_printf(conn, "enabled.<br>");
    } else {
        mg_printf(conn, "disabled.<br>");
    }

    mg_printf(conn, "<br>");
    mg_printf(conn, "<hr>");

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * ** * * * * *

    mg_printf(conn, "<h4 id=\"header\" >&nbsp;Level I drivers</h4> ");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Level I Drivers:</b> ");

    mg_printf(conn, "enabled<br>");
    mg_printf(conn,
              "&nbsp;&nbsp;&nbsp;&nbsp;----------------------------------<br>");

    CDeviceItem* pDeviceItem;
    std::deque<CDeviceItem*>::iterator it;
    for (it = gpobj->m_deviceList.m_devItemList.begin();
         it != gpobj->m_deviceList.m_devItemList.end();
         ++it) {
        pDeviceItem = *it;
        if ((NULL != pDeviceItem) &&
            (VSCP_DRIVER_LEVEL1 == pDeviceItem->m_driverLevel) &&
            pDeviceItem->m_bEnable) {
            mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Name:</b> ");
            mg_printf(conn, "%s", (const char*)pDeviceItem->m_strName.c_str());
            mg_printf(conn, "<br>");
            mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Config:</b> ");
            mg_printf(conn,
                      "%s",
                      (const char*)pDeviceItem->m_strParameter.c_str());
            mg_printf(conn, "<br>");
            mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Path:</b> ");
            mg_printf(conn, "%s", (const char*)pDeviceItem->m_strPath.c_str());
            mg_printf(conn, "<br>");
            mg_printf(
              conn,
              "&nbsp;&nbsp;&nbsp;&nbsp;----------------------------------<br>");
        }
    }

    mg_printf(conn, "<hr>");

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * ** * * * * *

    mg_printf(conn, "<h4 id=\"header\" >&nbsp;Level II drivers</h4> ");

    mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Level II Drivers:</b> ");
    mg_printf(conn, "enabled<br>");

    mg_printf(conn,
              "&nbsp;&nbsp;&nbsp;&nbsp;----------------------------------<br>");

    for (it = gpobj->m_deviceList.m_devItemList.begin();
         it != gpobj->m_deviceList.m_devItemList.end();
         ++it) {
        pDeviceItem = *it;
        if ((NULL != pDeviceItem) &&
            (VSCP_DRIVER_LEVEL2 == pDeviceItem->m_driverLevel) &&
            pDeviceItem->m_bEnable) {
            mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Name:</b> ");
            mg_printf(conn, "%s", (const char*)pDeviceItem->m_strName.c_str());
            mg_printf(conn, "<br>");
            mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Config:</b> ");
            mg_printf(conn,
                      "%s",
                      (const char*)pDeviceItem->m_strParameter.c_str());
            mg_printf(conn, "<br>");
            mg_printf(conn, "&nbsp;&nbsp;&nbsp;&nbsp;<b>Driver path:</b> ");
            mg_printf(conn, "%s", (const char*)pDeviceItem->m_strPath.c_str());
            mg_printf(conn, "<br>");
            mg_printf(
              conn,
              "&nbsp;&nbsp;&nbsp;&nbsp;----------------------------------<br>");
        }
    }

    mg_printf(conn, "<br>");
    mg_printf(conn, WEB_COMMON_END, VSCPD_COPYRIGHT_HTML); // Common end code

    return WEB_OK;
}

#ifdef WEB_EXAMPLES

// -----------------------------------------------------------------------------
//                          CIVETWEB test setup
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
// ExampleHandler
//

int
ExampleHandler(struct mg_connection* conn, void* cbdata)
{
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
              "close\r\n\r\n");
    mg_printf(conn, "<html><body>");
    mg_printf(conn, "<h2>This is an example text from a C handler</h2>");
    mg_printf(conn,
              "<p>To see a page from the A handler <a href=\"A\">click "
              "A</a></p>");
    mg_printf(conn,
              "<p>To see a page from the A handler <a href=\"A/A\">click "
              "A/A</a></p>");
    mg_printf(conn,
              "<p>To see a page from the A/B handler <a "
              "href=\"A/B\">click A/B</a></p>");
    mg_printf(conn,
              "<p>To see a page from the B handler (0) <a "
              "href=\"B\">click B</a></p>");
    mg_printf(conn,
              "<p>To see a page from the B handler (1) <a "
              "href=\"B/A\">click B/A</a></p>");
    mg_printf(conn,
              "<p>To see a page from the B handler (2) <a "
              "href=\"B/B\">click B/B</a></p>");
    mg_printf(conn,
              "<p>To see a page from the *.foo handler <a "
              "href=\"xy.foo\">click xy.foo</a></p>");
    mg_printf(conn,
              "<p>To see a page from the close handler <a "
              "href=\"close\">click close</a></p>");
    mg_printf(conn,
              "<p>To see a page from the FileHandler handler <a "
              "href=\"form\">click form</a> (the starting point of the "
              "<b>form</b> test)</p>");
    mg_printf(conn,
              "<p>To see a page from the CookieHandler handler <a "
              "href=\"cookie\">click cookie</a></p>");
    mg_printf(conn,
              "<p>To see a page from the PostResponser handler <a "
              "href=\"postresponse\">click post response</a></p>");
    mg_printf(conn,
              "<p>To see an example for parsing files on the fly <a "
              "href=\"on_the_fly_form\">click form</a> (form for "
              "uploading files)</p>");

    mg_printf(conn,
              "<p>To test websocket handler <a href=\"/websocket\">click "
              "websocket</a></p>");

    mg_printf(conn, "<p>To exit <a href=\"%s\">click exit</a></p>", EXIT_URI);
    mg_printf(conn, "</body></html>\n");
    return WEB_OK;
}

////////////////////////////////////////////////////////////////////////////////
// ExitHandler
//

int
ExitHandler(struct mg_connection* conn, void* cbdata)
{
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\nContent-Type: "
              "text/plain\r\nConnection: close\r\n\r\n");
    mg_printf(conn, "Server will shut down.\n");
    mg_printf(conn, "Bye!\n");

    return WEB_OK;
}

////////////////////////////////////////////////////////////////////////////////
// AHandler
//

int
AHandler(struct mg_connection* conn, void* cbdata)
{
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
              "close\r\n\r\n");
    mg_printf(conn, "<html><body>");
    mg_printf(conn, "<h2>This is the A handler!!!</h2>");
    mg_printf(conn, "</body></html>\n");

    return WEB_OK;
}

////////////////////////////////////////////////////////////////////////////////
// ABHandler
//

int
ABHandler(struct mg_connection* conn, void* cbdata)
{
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
              "close\r\n\r\n");
    mg_printf(conn, "<html><body>");
    mg_printf(conn, "<h2>This is the AB handler!!!</h2>");
    mg_printf(conn, "</body></html>\n");

    return WEB_OK;
}

////////////////////////////////////////////////////////////////////////////////
// BXHandler
//

int
BXHandler(struct mg_connection* conn, void* cbdata)
{
    // Handler may access the request info using mg_get_request_info
    const struct mg_request_info* req_info = mg_get_request_info(conn);

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
              "close\r\n\r\n");
    mg_printf(conn, "<html><body>");
    mg_printf(conn, "<h2>This is the BX handler %p!!!</h2>", cbdata);
    mg_printf(conn, "<p>The actual uri is %s</p>", req_info->local_uri);
    mg_printf(conn, "</body></html>\n");

    return WEB_OK;
}

////////////////////////////////////////////////////////////////////////////////
// FooHandler
//

int
FooHandler(struct mg_connection* conn, void* cbdata)
{
    // Handler may access the request info using mg_get_request_info
    const struct mg_request_info* req_info = mg_get_request_info(conn);

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
              "close\r\n\r\n");
    mg_printf(conn, "<html><body>");
    mg_printf(conn, "<h2>This is the Foo handler!!!</h2>");
    mg_printf(conn,
              "<p>The request was:<br><pre>%s %s HTTP/%s</pre></p>",
              req_info->request_method,
              req_info->local_uri,
              req_info->http_version);
    mg_printf(conn, "</body></html>\n");

    return WEB_OK;
}

////////////////////////////////////////////////////////////////////////////////
// CloseHandler
//

int
CloseHandler(struct mg_connection* conn, void* cbdata)
{
    // Handler may access the request info using mg_get_request_info
    // const struct mg_request_info *req_info =
    // mg_get_request_info(conn);

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
              "close\r\n\r\n");
    mg_printf(conn, "<html><body>");
    mg_printf(conn,
              "<h2>This handler will close the connection in a second</h2>");
#ifdef _WIN32
    Sleep(1000);
#else
    sleep(1);
#endif
    mg_printf(conn, "bye");
    printf("CloseHandler: close connection\n");
    mg_close_connection(conn);
    printf("CloseHandler: wait 10 sec\n");
#ifdef _WIN32
    Sleep(10000);
#else
    sleep(10);
#endif
    printf("CloseHandler: return from function\n");

    return WEB_OK;
}

////////////////////////////////////////////////////////////////////////////////
// FileHandler
//

int
FileHandler(struct mg_connection* conn, void* cbdata)
{
    // In this handler, we ignore the req_info and send the file
    // "fileName".
    const char* fileName = (const char*)cbdata;

    mg_send_file(conn, fileName);

    return WEB_OK;
}

////////////////////////////////////////////////////////////////////////////////
// field_found
//

// int
// field_found(const char *key,
//             const char *filename,
//             char *path,
//             size_t pathlen,
//             void *user_data)
// {
//     struct mg_connection *conn = (struct mg_connection *)user_data;

//     mg_printf(conn, "\r\n\r\n%s:\r\n", key);

//     if (filename && *filename) {
// #ifdef _WIN32
//         _snprintf(path, pathlen, "D:\\tmp\\%s", filename);
// #else
//         snprintf(path, pathlen, "/tmp/%s", filename);
// #endif
//         return WEB_FORM_FIELD_STORAGE_STORE;
//     }

//     return WEB_FORM_FIELD_STORAGE_GET;
// }

////////////////////////////////////////////////////////////////////////////////
// field_get
//

// int
// field_get(const char *key, const char *value, size_t valuelen, void
// *user_data)
// {
//     struct mg_connection *conn = (struct mg_connection *)user_data;

//     if (key[0]) {
//         mg_printf(conn, "%s = ", key);
//     }

//     mg_write(conn, value, valuelen);

//     return 0;
// }

// ////////////////////////////////////////////////////////////////////////////////
// // field_stored
// //

// int
// field_stored(const char *path, long long file_size, void *user_data)
// {
//     struct mg_connection *conn = (struct mg_connection *)user_data;

//     mg_printf(
//       conn, "stored as %s (%lu bytes)\r\n\r\n", path, (unsigned
//       long)file_size);

//     return WEB_OK;
// }

// ////////////////////////////////////////////////////////////////////////////////
// // FormHandler
// //

// int
// FormHandler(struct mg_connection *conn, void *cbdata)
// {
//     // Handler may access the request info using mg_get_request_info
//     const struct mg_request_info *req_info =
//     mg_get_request_info(conn); int ret; struct web_form_data_handler
//     fdh = {
//         field_found, field_get, field_stored, 0
//     };

//     // It would be possible to check the request info here before
//     calling
//     // web_handle_form_request.
//     (void)req_info;

//     mg_printf(conn,
//                "HTTP/1.1 200 OK\r\nContent-Type: "
//                "text/plain\r\nConnection: close\r\n\r\n");
//     fdh.user_data = (void *)conn;

//     // Call the form handler
//     mg_printf(conn, "Form data:");
//     ret = mg_handle_form_request(conn, &fdh);
//     mg_printf(conn, "\r\n%i fields found", ret);

//     return WEB_OK;
// }

// ////////////////////////////////////////////////////////////////////////////////
// // FileUploadForm
// //

// int
// FileUploadForm(struct mg_connection *conn, void *cbdata)
// {
//     mg_printf(conn,
//                "HTTP/1.1 200 OK\r\nContent-Type:
//                text/html\r\nConnection: " "close\r\n\r\n");

//     mg_printf(conn, "<!DOCTYPE html>\n");
//     mg_printf(conn, "<html>\n<head>\n");
//     mg_printf(conn, "<meta charset=\"UTF-8\">\n");
//     mg_printf(conn, "<title>File upload</title>\n");
//     mg_printf(conn, "</head>\n<body>\n");
//     mg_printf(conn,
//                "<form action=\"%s\" method=\"POST\" "
//                "enctype=\"multipart/form-data\">\n",
//                (const char *)cbdata);
//     mg_printf(conn, "<input type=\"file\" name=\"filesin\"
//     multiple>\n"); mg_printf(conn, "<input type=\"submit\"
//     value=\"Submit\">\n"); mg_printf(conn,
//     "</form>\n</body>\n</html>\n");

//     return WEB_OK;
// }

// struct tfile_checksum
// {
//     char name[128];
//     unsigned long long length;
//     md5_state_t chksum;
// };

// #define MAX_FILES (10)

// struct tfiles_checksums
// {
//     int index;
//     struct tfile_checksum file[MAX_FILES];
// };

// ////////////////////////////////////////////////////////////////////////////////
// // field_disp_read_on_the_fly
// //

// int
// field_disp_read_on_the_fly(const char *key,
//                            const char *filename,
//                            char *path,
//                            size_t pathlen,
//                            void *user_data)
// {
//     struct tfiles_checksums *context = (struct tfiles_checksums
//     *)user_data;

//     (void)key;
//     (void)path;
//     (void)pathlen;

//     if (context->index < MAX_FILES) {
//         context->index++;
//         strncpy(context->file[context->index - 1].name, filename,
//         128); context->file[context->index - 1].name[127] = 0;
//         context->file[context->index - 1].length    = 0;
//         vscpmd5_init(&(context->file[context->index - 1].chksum));
//         return WEB_FORM_FIELD_STORAGE_GET;
//     }

//     return WEB_FORM_FIELD_STORAGE_ABORT;
// }

// ////////////////////////////////////////////////////////////////////////////////
// // field_get_checksum
// //

// int
// field_get_checksum(const char *key,
//                    const char *value,
//                    size_t valuelen,
//                    void *user_data)
// {
//     struct tfiles_checksums *context = (struct tfiles_checksums
//     *)user_data; (void)key;

//     context->file[context->index - 1].length += valuelen;
//     vscpmd5_append(&(context->file[context->index - 1].chksum),
//                    (const md5_byte_t *)value,
//                    valuelen);

//     return 0;
// }

////////////////////////////////////////////////////////////////////////////////
// CheckSumHandler
//

// int
// CheckSumHandler(struct mg_connection *conn, void *cbdata)
// {
//     // Handler may access the request info using mg_get_request_info
//     const struct mg_request_info *req_info =
//     mg_get_request_info(conn); int i, j, ret; struct tfiles_checksums
//     chksums; md5_byte_t digest[16]; struct mg_form_data_handler fdh =
//     {
//         field_disp_read_on_the_fly, field_get_checksum, 0, (void
//         *)&chksums
//     };

//     /* It would be possible to check the request info here before
//     calling
//      * mg_handle_form_request. */
//     (void)req_info;

//     memset(&chksums, 0, sizeof(chksums));

//     mg_printf(conn,
//                "HTTP/1.1 200 OK\r\n"
//                "Content-Type: text/plain\r\n"
//                "Connection: close\r\n\r\n");

//     /* Call the form handler */
//     mg_printf(conn, "File checksums:");
//     ret = mg_handle_form_request(conn, &fdh);
//     for (i = 0; i < chksums.index; i++) {
//         vscpmd5_finish(&(chksums.file[i].chksum), digest);
//         /* Visual Studio 2010+ support llu */
//         mg_printf(
//           conn, "\r\n%s %llu ", chksums.file[i].name,
//           chksums.file[i].length);
//         for (j = 0; j < 16; j++) {
//             mg_printf(conn, "%02x", (unsigned int)digest[j]);
//         }
//     }
//     mg_printf(conn, "\r\n%i files\r\n", ret);

//     return WEB_OK;
// }

////////////////////////////////////////////////////////////////////////////////
// CookieHandler
//

int
CookieHandler(struct mg_connection* conn, void* cbdata)
{
    // Handler may access the request info using mg_get_request_info
    const struct mg_request_info* req_info = mg_get_request_info(conn);
    const char* cookie                     = mg_get_header(conn, "Cookie");
    char first_str[64], count_str[64];
    int count;

    (void)mg_get_cookie(cookie, "first", first_str, sizeof(first_str));
    (void)mg_get_cookie(cookie, "count", count_str, sizeof(count_str));

    mg_printf(conn, "HTTP/1.1 200 OK\r\nConnection: close\r\n");
    if (first_str[0] == 0) {
        time_t t       = time(0);
        struct tm* ptm = localtime(&t);
        mg_printf(conn,
                  "Set-Cookie: first=%04i-%02i-%02iT%02i:%02i:%02i\r\n",
                  ptm->tm_year + 1900,
                  ptm->tm_mon + 1,
                  ptm->tm_mday,
                  ptm->tm_hour,
                  ptm->tm_min,
                  ptm->tm_sec);
    }
    count = (count_str[0] == 0) ? 0 : atoi(count_str);
    mg_printf(conn, "Set-Cookie: count=%i\r\n", count + 1);
    mg_printf(conn, "Content-Type: text/html\r\n\r\n");

    mg_printf(conn, "<html><body>");
    mg_printf(conn, "<h2>This is the CookieHandler.</h2>");
    mg_printf(conn, "<p>The actual uri is %s</p>", req_info->local_uri);

    if (first_str[0] == 0) {
        mg_printf(conn, "<p>This is the first time, you opened this page</p>");
    } else {
        mg_printf(conn, "<p>You opened this page %i times before.</p>", count);
        mg_printf(conn, "<p>You first opened this page on %s.</p>", first_str);
    }

    mg_printf(conn, "</body></html>\n");

    return WEB_OK;
}

////////////////////////////////////////////////////////////////////////////////
// PostResponser
//

int
PostResponser(struct mg_connection* conn, void* cbdata)
{
    long long r_total = 0;
    int r, s;

    char buf[2048];

    const struct mg_request_info* ri = mg_get_request_info(conn);

    if (strcmp(ri->request_method, "POST")) {
        char buf[1024];
        int ret = mg_get_request_link(conn, buf, sizeof(buf));

        mg_printf(conn,
                  "HTTP/1.1 405 Method Not Allowed\r\nConnection: close\r\n");
        mg_printf(conn, "Content-Type: text/plain\r\n\r\n");
        mg_printf(conn,
                  "%s method not allowed in the POST handler\n",
                  ri->request_method);
        if (ret >= 0) {
            mg_printf(conn,
                      "use a web tool to send a POST request to %s\n",
                      buf);
        }
        return 1;
    }

    if (ri->content_length >= 0) {
        /* We know the content length in advance */
    } else {
        /* We must read until we find the end (chunked encoding
         * or connection close), indicated my mg_read returning 0 */
    }

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\nConnection: "
              "close\r\nTransfer-Encoding: chunked\r\n");
    mg_printf(conn, "Content-Type: text/plain\r\n\r\n");

    r = mg_read(conn, buf, sizeof(buf));
    while (r > 0) {
        r_total += r;
        s = mg_send_chunk(conn, buf, r);
        if (r != s) {
            /* Send error */
            break;
        }
        r = mg_read(conn, buf, sizeof(buf));
    }
    mg_printf(conn, "0\r\n");

    return WEB_OK;
}

////////////////////////////////////////////////////////////////////////////////
// WebSocketStartHandler
//

int
WebSocketStartHandler(struct mg_connection* conn, void* cbdata)
{
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: "
              "close\r\n\r\n");

    mg_printf(conn, "<!DOCTYPE html>\n");
    mg_printf(conn, "<html>\n<head>\n");
    mg_printf(conn, "<meta charset=\"UTF-8\">\n");
    mg_printf(conn, "<title>Embedded websocket example</title>\n");

    /* mg_printf(conn, "<script type=\"text/javascript\"><![CDATA[\n");
     * ... xhtml style */
    mg_printf(conn, "<script>\n");
    mg_printf(conn,
              "function load() {\n"
              "  var wsproto = (location.protocol === 'https:') ? "
              "'wss:' : 'ws:';\n"
              "  connection = new WebSocket(wsproto + '//' + "
              "window.location.host + "
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
    /* mg_printf(conn, "]]></script>\n"); ... xhtml style */
    mg_printf(conn, "</script>\n");
    mg_printf(conn, "</head>\n<body onload=\"load()\">\n");
    mg_printf(conn,
              "<div id='websock_text_field'>No websocket connection "
              "yet</div>\n");
    mg_printf(conn, "</body>\n</html>\n");

    return WEB_OK;
}

////////////////////////////////////////////////////////////////////////////////
// t_ws_client
//

// MAX_WS_CLIENTS defines how many clients can connect to a websocket at
// the same time. The value 5 is very small and used here only for
// demonstration; it can be easily tested to connect more than
// MAX_WS_CLIENTS clients. A real server should use a much higher
// number, or better use a dynamic list of currently connected websocket
// clients.
#define MAX_WS_CLIENTS (512)

struct t_ws_client
{
    struct mg_connection* conn;
    int state;
} static ws_clients[MAX_WS_CLIENTS];

#define ASSERT(x)                                                              \
    {                                                                          \
        if (!(x)) {                                                            \
            fprintf(stderr,                                                    \
                    "Assertion failed in line %u\n",                           \
                    (unsigned)__LINE__);                                       \
        }                                                                      \
    }

////////////////////////////////////////////////////////////////////////////////
// WebSocketConnectHandler
//

int
WebSocketConnectHandler(const struct mg_connection* conn, void* cbdata)
{
    struct mg_context* ctx = mg_get_context(conn);
    int reject             = 1;
    int i;

    mg_lock_context(ctx);
    for (i = 0; i < MAX_WS_CLIENTS; i++) {
        if (ws_clients[i].conn == NULL) {
            ws_clients[i].conn  = (struct mg_connection*)conn;
            ws_clients[i].state = 1;
            mg_set_user_connection_data(ws_clients[i].conn,
                                        (void*)(ws_clients + i));
            reject = 0;
            break;
        }
    }
    mg_unlock_context(ctx);

    fprintf(stdout,
            "Websocket client %s\r\n\r\n",
            (reject ? "rejected" : "accepted"));

    return reject;
}

////////////////////////////////////////////////////////////////////////////////
// WebSocketReadyHandler
//

void
WebSocketReadyHandler(struct mg_connection* conn, void* cbdata)
{
    const char* text = "Hello from the websocket ready handler";
    struct t_ws_client* client =
      (struct t_ws_client*)mg_get_user_connection_data(conn);

    mg_websocket_write(conn, MG_WEBSOCKET_OPCODE_TEXT, text, strlen(text));
    fprintf(stdout, "Greeting message sent to websocket client\r\n\r\n");
    ASSERT(client->conn == conn);
    ASSERT(client->state == 1);

    client->state = 2;
}

////////////////////////////////////////////////////////////////////////////////
// WebsocketDataHandler
//

int
WebsocketDataHandler(struct mg_connection* conn,
                     int bits,
                     char* data,
                     size_t len,
                     void* cbdata)
{
    struct t_ws_client* client =
      (struct t_ws_client*)mg_get_user_connection_data(conn);
    ASSERT(client->conn == conn);
    ASSERT(client->state >= 1);

    fprintf(stdout, "Websocket got %lu bytes of ", (unsigned long)len);
    switch (((unsigned char)bits) & 0x0F) {
        case MG_WEBSOCKET_OPCODE_CONTINUATION:
            fprintf(stdout, "continuation");
            break;
        case MG_WEBSOCKET_OPCODE_TEXT:
            fprintf(stdout, "text");
            break;
        case MG_WEBSOCKET_OPCODE_BINARY:
            fprintf(stdout, "binary");
            break;
        case MG_WEBSOCKET_OPCODE_CONNECTION_CLOSE:
            fprintf(stdout, "close");
            break;
        case MG_WEBSOCKET_OPCODE_PING:
            fprintf(stdout, "ping");
            break;
        case MG_WEBSOCKET_OPCODE_PONG:
            fprintf(stdout, "pong");
            break;
        default:
            fprintf(stdout, "unknown(%1xh)", ((unsigned char)bits) & 0x0F);
            break;
    }
    fprintf(stdout, " data:\r\n");
    fwrite(data, len, 1, stdout);
    fprintf(stdout, "\r\n\r\n");

    return WEB_OK;
}

////////////////////////////////////////////////////////////////////////////////
// WebSocketCloseHandler
//

void
WebSocketCloseHandler(const struct mg_connection* conn, void* cbdata)
{
    struct mg_context* ctx = mg_get_context(conn);
    struct t_ws_client* client =
      (struct t_ws_client*)mg_get_user_connection_data(conn);
    ASSERT(client->conn == conn);
    ASSERT(client->state >= 1);

    mg_lock_context(ctx);
    client->state = 0;
    client->conn  = NULL;
    mg_unlock_context(ctx);

    fprintf(stdout,
            "Client droped from the set of webserver connections\r\n\r\n");
}

////////////////////////////////////////////////////////////////////////////////
// informWebsockets
//

void
informWebsockets(struct mg_context* ctx)
{
    static unsigned long cnt = 0;
    char text[32];
    int i;

    sprintf(text, "%lu", ++cnt);

    mg_lock_context(ctx);
    for (i = 0; i < MAX_WS_CLIENTS; i++) {
        if (2 == ws_clients[i].state) {
            mg_websocket_write(ws_clients[i].conn,
                               MG_WEBSOCKET_OPCODE_TEXT,
                               text,
                               strlen(text));
        }
    }
    mg_unlock_context(ctx);
}

#endif // WEB_EXAMPLES

// https://security.stackexchange.com/questions/41205/diffie-hellman-and-its-tls-ssl-usage
#ifdef USE_SSL_DH
#include "openssl/dh.h"
#include "openssl/ec.h"
#include "openssl/ecdsa.h"
#include "openssl/evp.h"
#include "openssl/ssl.h"

DH*
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
    DH* dh;

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
init_ssl(void* ssl_context, void* user_data)
{
    /* Add application specific SSL initialization */
    // struct ssl_ctx_st *ctx = (struct ssl_ctx_st *)ssl_context;

#ifdef USE_SSL_DH
    /* example from https://github.com/civetweb/civetweb/issues/347 */
    DH* dh = get_dh2236();
    if (!dh)
        return -1;
    if (1 != SSL_CTX_set_tmp_dh(ctx, dh))
        return -1;
    DH_free(dh);

    EC_KEY* ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    if (!ecdh)
        return -1;
    if (1 != SSL_CTX_set_tmp_ecdh(ctx, ecdh))
        return -1;
    EC_KEY_free(ecdh);

    printf("ECDH ciphers initialized\n");
#endif
    return 0;
}

// static void
// delete_options(char **opts)
// {}

//#define mg_free(a) mg_free_ex(a, __FILE__, __LINE__);

///////////////////////////////////////////////////////////////////////////////
// start_webserver
//

int
start_webserver(void)
{
    syslog(LOG_ERR, ("Starting web server...\n"));

    if (gpobj->m_bWebsocketsEnable) {
        syslog(LOG_ERR, ("Websockets enable...\n"));
    }

    if (gpobj->m_bEnableRestApi) {
        syslog(LOG_ERR, ("REST API enable...\n"));
    }

    // This structure must be larger than the number of options to set
    const char** web_options = new const char*[120 + 2];

    struct mg_callbacks callbacks;

    // Set setup options from configuration
    int pos = 0;

    web_options[pos++] = vscp_strdup(VSCPDB_CONFIG_NAME_WEB_DOCUMENT_ROOT + 4);
    web_options[pos++] =
      vscp_strdup((const char*)gpobj->m_web_document_root.c_str());

    web_options[pos++] =
      vscp_strdup(VSCPDB_CONFIG_NAME_WEB_LISTENING_PORTS + 4);
    web_options[pos++] =
      vscp_strdup((const char*)gpobj->m_web_listening_ports.c_str());

    web_options[pos++] = vscp_strdup(VSCPDB_CONFIG_NAME_WEB_INDEX_FILES + 4);
    web_options[pos++] =
      vscp_strdup((const char*)gpobj->m_web_index_files.c_str());

    web_options[pos++] =
      vscp_strdup(VSCPDB_CONFIG_NAME_WEB_AUTHENTICATION_DOMAIN + 4);
    web_options[pos++] =
      vscp_strdup((const char*)gpobj->m_web_authentication_domain.c_str());

    // Set only if not default value
    if (!gpobj->m_enable_auth_domain_check) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_ENABLE_AUTH_DOMAIN_CHECK + 4);
        web_options[pos++] = vscp_strdup("no");
    }

    web_options[pos++] =
      vscp_strdup(VSCPDB_CONFIG_NAME_WEB_SSL_CERTIFICATE + 4);
    web_options[pos++] =
      vscp_strdup((const char*)gpobj->m_web_ssl_certificate.c_str());

    web_options[pos++] =
      vscp_strdup(VSCPDB_CONFIG_NAME_WEB_SSL_CERTIFICAT_CHAIN + 4);
    web_options[pos++] =
      vscp_strdup((const char*)gpobj->m_web_ssl_certificate_chain.c_str());

    // Set only if not default value
    if (gpobj->m_web_ssl_verify_peer) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_SSL_VERIFY_PEER + 4);
        web_options[pos++] = vscp_strdup("yes");
    }

    if (gpobj->m_web_ssl_ca_path.length()) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_SSL_CA_PATH + 4);
        web_options[pos++] =
          vscp_strdup((const char*)gpobj->m_web_ssl_ca_path.c_str());
    }

    if (gpobj->m_web_ssl_ca_file.length()) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_SSL_CA_FILE + 4);
        web_options[pos++] =
          vscp_strdup((const char*)gpobj->m_web_ssl_ca_file.c_str());
    }

    if (gpobj->m_web_ssl_verify_depth !=
        atoi(VSCPDB_CONFIG_DEFAULT_WEB_SSL_VERIFY_DEPTH)) {
        std::string str =
          vscp_str_format(("%d"), (int)gpobj->m_web_ssl_verify_depth);
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_SSL_VERIFY_DEPTH + 4);
        web_options[pos++] = vscp_strdup((const char*)str.c_str());
    }

    if (!gpobj->m_web_ssl_default_verify_paths) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_SSL_DEFAULT_VERIFY_PATHS + 4);
        web_options[pos++] = vscp_strdup("no");
    }

    web_options[pos++] =
      vscp_strdup(VSCPDB_CONFIG_NAME_WEB_SSL_CHIPHER_LIST + 4);
    web_options[pos++] =
      vscp_strdup((const char*)gpobj->m_web_ssl_cipher_list.c_str());

    if (gpobj->m_web_ssl_protocol_version !=
        atoi(VSCPDB_CONFIG_DEFAULT_WEB_SSL_PROTOCOL_VERSION)) {
        std::string str =
          vscp_str_format(("%d"), (int)gpobj->m_web_ssl_protocol_version);
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_SSL_PROTOCOL_VERSION + 4);
        web_options[pos++] = vscp_strdup((const char*)str.c_str());
    }

    if (!gpobj->m_web_ssl_short_trust) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_SSL_SHORT_TRUST + 4);
        web_options[pos++] = vscp_strdup("yes");
    }

    if (gpobj->m_web_cgi_interpreter.length()) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_CGI_INTERPRETER + 4);
        web_options[pos++] =
          vscp_strdup((const char*)gpobj->m_web_cgi_interpreter.c_str());
    }

    if (gpobj->m_web_cgi_patterns.length()) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_CGI_PATTERN + 4);
        web_options[pos++] =
          vscp_strdup((const char*)gpobj->m_web_cgi_patterns.c_str());
    }

    if (gpobj->m_web_cgi_environment.length()) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_CGI_ENVIRONMENT + 4);
        web_options[pos++] =
          vscp_strdup((const char*)gpobj->m_web_cgi_environment.c_str());
    }

    if (gpobj->m_web_protect_uri.length()) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_PROTECT_URI + 4);
        web_options[pos++] =
          vscp_strdup((const char*)gpobj->m_web_protect_uri.c_str());
    }

    if (gpobj->m_web_trottle.length()) {
        web_options[pos++] = vscp_strdup(VSCPDB_CONFIG_NAME_WEB_TROTTLE + 4);
        web_options[pos++] =
          vscp_strdup((const char*)gpobj->m_web_trottle.c_str());
    }

    if (!gpobj->m_web_enable_directory_listing) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_ENABLE_DIRECTORY_LISTING + 4);
        web_options[pos++] = vscp_strdup("no");
    }

    if (gpobj->m_web_enable_keep_alive) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_ENABLE_KEEP_ALIVE + 4);
        web_options[pos++] = vscp_strdup("yes");
    }

    if (gpobj->m_web_keep_alive_timeout_ms !=
        atol(VSCPDB_CONFIG_DEFAULT_WEB_KEEP_ALIVE_TIMEOUT_MS)) {
        std::string str =
          vscp_str_format(("%ld"), (long)gpobj->m_web_ssl_protocol_version);
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_KEEP_ALIVE_TIMEOUT_MS + 4);
        web_options[pos++] = vscp_strdup((const char*)str.c_str());
    }

    if (gpobj->m_web_access_control_list.length()) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_LIST + 4);
        web_options[pos++] =
          vscp_strdup((const char*)gpobj->m_web_access_control_list.c_str());
    }

    if (gpobj->m_web_extra_mime_types.length()) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_EXTRA_MIME_TYPES + 4);
        web_options[pos++] =
          vscp_strdup((const char*)gpobj->m_web_extra_mime_types.c_str());
    }

    if (gpobj->m_web_num_threads !=
        atoi(VSCPDB_CONFIG_DEFAULT_WEB_NUM_THREADS)) {
        std::string str =
          vscp_str_format(("%d"), (int)gpobj->m_web_num_threads);
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_EXTRA_MIME_TYPES + 4);
        web_options[pos++] = vscp_strdup((const char*)str.c_str());
    }

    if (gpobj->m_web_url_rewrite_patterns.length()) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_URL_REWRITE_PATTERNS + 4);
        web_options[pos++] =
          vscp_strdup((const char*)gpobj->m_web_url_rewrite_patterns.c_str());
    }

    if (gpobj->m_web_hide_file_patterns.length()) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_URL_REWRITE_PATTERNS + 4);
        web_options[pos++] =
          vscp_strdup((const char*)gpobj->m_web_hide_file_patterns.c_str());
    }

    if (gpobj->m_web_request_timeout_ms !=
        atol(VSCPDB_CONFIG_DEFAULT_WEB_REQUEST_TIMEOUT_MS)) {
        std::string str =
          vscp_str_format(("%ld"), (long)gpobj->m_web_request_timeout_ms);
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_REQUEST_TIMEOUT_MS + 4);
        web_options[pos++] = vscp_strdup((const char*)str.c_str());
    }

    if (-1 != gpobj->m_web_linger_timeout_ms) {
        std::string str =
          vscp_str_format(("%ld"), (long)gpobj->m_web_linger_timeout_ms);
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_LINGER_TIMEOUT_MS + 4);
        web_options[pos++] = vscp_strdup((const char*)str.c_str());
    }

    if (!gpobj->m_web_decode_url) {
        web_options[pos++] = vscp_strdup(VSCPDB_CONFIG_NAME_WEB_DECODE_URL + 4);
        web_options[pos++] = vscp_strdup("no");
    }

    if (gpobj->m_web_global_auth_file.length()) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_GLOBAL_AUTHFILE + 4);
        web_options[pos++] =
          vscp_strdup((const char*)gpobj->m_web_global_auth_file.c_str());
    }

    if (gpobj->m_web_per_directory_auth_file.length()) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_PER_DIRECTORY_AUTH_FILE + 4);
        web_options[pos++] = vscp_strdup(
          (const char*)gpobj->m_web_per_directory_auth_file.c_str());
    }

    if (gpobj->m_web_ssi_patterns.length()) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_SSI_PATTERNS + 4);
        web_options[pos++] =
          vscp_strdup((const char*)gpobj->m_web_ssi_patterns.c_str());
    }

    if (gpobj->m_web_access_control_allow_origin.length()) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_ALLOW_ORIGIN + 4);
        web_options[pos++] = vscp_strdup(
          (const char*)gpobj->m_web_access_control_allow_origin.c_str());
    }

    if (gpobj->m_web_access_control_allow_methods.length()) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_ALLOW_METHODS + 4);
        web_options[pos++] = vscp_strdup(
          (const char*)gpobj->m_web_access_control_allow_methods.c_str());
    }

    if (gpobj->m_web_access_control_allow_headers.length()) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_ALLOW_HEADERS + 4);
        web_options[pos++] = vscp_strdup(
          (const char*)gpobj->m_web_access_control_allow_headers.c_str());
    }

    if (gpobj->m_web_error_pages.length()) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_ERROR_PAGES + 4);
        web_options[pos++] =
          vscp_strdup((const char*)gpobj->m_web_error_pages.c_str());
    }

    if (-1 != gpobj->m_web_tcp_nodelay) {
        std::string str =
          vscp_str_format(("%ld"), (long)gpobj->m_web_tcp_nodelay);
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_TCP_NO_DELAY + 4);
        web_options[pos++] = vscp_strdup((const char*)str.c_str());
    }

    if (gpobj->m_web_static_file_cache_control.length()) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_ERROR_PAGES + 4);
        web_options[pos++] = vscp_strdup(
          (const char*)gpobj->m_web_static_file_cache_control.c_str());
    }

    if (gpobj->m_web_static_file_max_age !=
        atol(VSCPDB_CONFIG_DEFAULT_WEB_STATIC_FILE_MAX_AGE)) {
        std::string str =
          vscp_str_format(("%ld"), (long)gpobj->m_web_static_file_max_age);
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_STATIC_FILE_MAX_AGE + 4);
        web_options[pos++] = vscp_strdup((const char*)str.c_str());
    }

    if (-1 != gpobj->m_web_strict_transport_security_max_age) {
        std::string str =
          vscp_str_format(("%ld"),
                          (long)gpobj->m_web_strict_transport_security_max_age);
        web_options[pos++] = vscp_strdup(
          VSCPDB_CONFIG_NAME_WEB_STRICT_TRANSPORT_SECURITY_MAX_AGE + 4);
        web_options[pos++] = vscp_strdup((const char*)str.c_str());
    }

    if (!gpobj->m_web_allow_sendfile_call) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_ALLOW_SENDFILE_CALL + 4);
        web_options[pos++] = vscp_strdup("no");
    }

    if (gpobj->m_web_additional_header.length()) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_ADDITIONAL_HEADERS + 4);
        web_options[pos++] =
          vscp_strdup((const char*)gpobj->m_web_additional_header.c_str());
    }

    if (gpobj->m_web_max_request_size !=
        atol(VSCPDB_CONFIG_DEFAULT_WEB_MAX_REQUEST_SIZE)) {
        std::string str =
          vscp_str_format(("%ld"), (long)gpobj->m_web_max_request_size);
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_MAX_REQUEST_SIZE + 4);
        web_options[pos++] = vscp_strdup((const char*)str.c_str());
    }

    if (gpobj->m_web_allow_index_script_resource) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_ALLOW_INDEX_SCRIPT_RESOURCE + 4);
        web_options[pos++] = vscp_strdup("yes");
    }

    if (gpobj->m_web_duktape_script_patterns.length()) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_DUKTAPE_SCRIPT_PATTERN + 4);
        web_options[pos++] = vscp_strdup(
          (const char*)gpobj->m_web_duktape_script_patterns.c_str());
    }

    if (gpobj->m_web_lua_preload_file.length()) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_LUA_PRELOAD_FILE + 4);
        web_options[pos++] =
          vscp_strdup((const char*)gpobj->m_web_lua_preload_file.c_str());
    }

    if (gpobj->m_web_lua_script_patterns.length()) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_LUA_SCRIPT_PATTERN + 4);
        web_options[pos++] =
          vscp_strdup((const char*)gpobj->m_web_lua_script_patterns.c_str());
    }

    if (gpobj->m_web_lua_server_page_patterns.length()) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_LUA_SERVER_PAGE_PATTERN + 4);
        web_options[pos++] = vscp_strdup(
          (const char*)gpobj->m_web_lua_server_page_patterns.c_str());
    }

    if (gpobj->m_web_lua_websocket_patterns.length()) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_LUA_WEBSOCKET_PATTERN + 4);
        web_options[pos++] =
          vscp_strdup((const char*)gpobj->m_web_lua_websocket_patterns.c_str());
    }

    if (gpobj->m_web_lua_background_script.length()) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_LUA_BACKGROUND_SCRIPT + 4);
        web_options[pos++] =
          vscp_strdup((const char*)gpobj->m_web_lua_background_script.c_str());
    }

    if (gpobj->m_web_lua_background_script_params.length()) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_LUA_BACKGROUND_SCRIPT_PARAMS + 4);
        web_options[pos++] = vscp_strdup(
          (const char*)gpobj->m_web_lua_background_script_params.c_str());
    }

    if (gpobj->m_websocket_document_root.length()) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEBSOCKET_DOCUMENT_ROOT + 4);
        web_options[pos++] =
          vscp_strdup((const char*)gpobj->m_websocket_document_root.c_str());
    }

    if (gpobj->m_websocket_timeout_ms !=
        atol(VSCPDB_CONFIG_DEFAULT_WEBSOCKET_TIMEOUT_MS)) {
        std::string str =
          vscp_str_format(("%ld"), (long)gpobj->m_websocket_timeout_ms);
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEBSOCKET_TIMEOUT_MS + 4);
        web_options[pos++] = vscp_strdup((const char*)str.c_str());
    }

    if (gpobj->bEnable_websocket_ping_pong) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEBSOCKET_PING_PONG_ENABLE + 4);
        web_options[pos++] = vscp_strdup("yes");
    }

    if (gpobj->lua_websocket_pattern.length()) {
        web_options[pos++] =
          vscp_strdup(VSCPDB_CONFIG_NAME_WEB_LUA_WEBSOCKET_PATTERN + 4);
        web_options[pos++] =
          vscp_strdup((const char*)gpobj->lua_websocket_pattern.c_str());
    }

    // Mark end
    web_options[pos++] = NULL;
    web_options[pos++] = NULL;

    // Setup callbacks
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.init_ssl    = init_ssl;
    callbacks.log_message = log_message;
    callbacks.log_access  = log_access;

    // Start server
    gpobj->m_web_ctx = mg_start(&callbacks, 0, (const char**)web_options);

    // Delete allocated option data
    pos = 0;
    while (NULL != web_options[pos]) {
        free((void *)web_options[pos]);
        pos++;
        free((void *)web_options[pos]);
        pos++;
    }

    delete[] web_options;

    // Check return value:
    if (NULL == gpobj->m_web_ctx) {
        syslog(LOG_ERR, "websrv: Cannot start webserver - web_start failed.");
        return EXIT_FAILURE;
    }

    // The web examples enables some Civitweb test code

#ifdef WEB_EXAMPLES

    // Add handler EXAMPLE_URI, to explain the example
    mg_set_request_handler(gpobj->m_web_ctx, EXAMPLE_URI, ExampleHandler, 0);
    mg_set_request_handler(gpobj->m_web_ctx, EXIT_URI, ExitHandler, 0);

    // Add handler for /A* and special handler for /A/B
    mg_set_request_handler(gpobj->m_web_ctx, "/A", AHandler, 0);
    mg_set_request_handler(gpobj->m_web_ctx, "/A/B", ABHandler, 0);

    // Add handler for /B, /B/A, /B/B but not for /B*
    mg_set_request_handler(gpobj->m_web_ctx, "/B$", BXHandler, (void*)0);
    mg_set_request_handler(gpobj->m_web_ctx, "/B/A$", BXHandler, (void*)1);
    mg_set_request_handler(gpobj->m_web_ctx, "/B/B$", BXHandler, (void*)2);

    // Add handler for all files with .foo extention
    mg_set_request_handler(gpobj->m_web_ctx, "**.foo$", FooHandler, 0);

    // Add handler for /close extention
    mg_set_request_handler(gpobj->m_web_ctx, "/close", CloseHandler, 0);

    // Add handler for /cookie example
    mg_set_request_handler(gpobj->m_web_ctx, "/cookie", CookieHandler, 0);

    // Add handler for /postresponse example
    mg_set_request_handler(gpobj->m_web_ctx, "/postresponse", PostResponser, 0);

    // Add HTTP site to open a websocket connection
    mg_set_request_handler(gpobj->m_web_ctx,
                           "/websocket",
                           WebSocketStartHandler,
                           0);

    if (!gpobj->m_bWebsocketsEnable) {
        // WS site for the websocket connection
        mg_set_websocket_handler(gpobj->m_web_ctx,
                                 "/websocket",
                                 WebSocketConnectHandler,
                                 WebSocketReadyHandler,
                                 WebsocketDataHandler,
                                 WebSocketCloseHandler,
                                 0);
    }

#endif // WEB_EXAMPLES

    // Add handler for form data
    // mg_set_request_handler(gpobj->m_web_ctx,
    //                         "/handle_form.embedded_c.example.callback",
    //                         FormHandler,
    //                         (void *)0);

    // Add a file upload handler for parsing files on the fly
    // mg_set_request_handler(gpobj->m_web_ctx,
    //                         "/on_the_fly_form",
    //                         FileUploadForm,
    //                         (void *)"/on_the_fly_form.md5.callback");
    // mg_set_request_handler(gpobj->m_web_ctx,
    //                         "/on_the_fly_form.md5.callback",
    //                         CheckSumHandler,
    //                         (void *)0);

    // Add handler for /form  (serve a file outside the document root)
    mg_set_request_handler(gpobj->m_web_ctx,
                           "/form",
                           FileHandler,
                           (void*)"../../test/form.html");

    // Set authorization handlers
    if (gpobj->m_enableWebAdminIf) {
        mg_set_auth_handler(gpobj->m_web_ctx,
                            "/vscp",
                            check_admin_authorization,
                            NULL);
    }

    if (gpobj->m_bEnableRestApi) {
        mg_set_auth_handler(gpobj->m_web_ctx,
                            "/vscp/rest",
                            check_rest_authorization,
                            NULL);
    }

    if (gpobj->m_bWebsocketsEnable) {
        // WS1 path for the websocket connection
        mg_set_websocket_handler(gpobj->m_web_ctx,
                                 "/ws1",
                                 ws1_connectHandler,
                                 ws1_readyHandler,
                                 ws1_dataHandler,
                                 ws1_closeHandler,
                                 0);

        // WS2 path for the websocket connection
        mg_set_websocket_handler(gpobj->m_web_ctx,
                                 "/ws2",
                                 ws2_connectHandler,
                                 ws2_readyHandler,
                                 ws2_dataHandler,
                                 ws2_closeHandler,
                                 0);
    }

    // Set page handlers for admin i/f
    if (gpobj->m_enableWebAdminIf) {
        mg_set_request_handler(gpobj->m_web_ctx, "/vscp", vscp_mainpage, 0);
        mg_set_request_handler(gpobj->m_web_ctx,
                               "/vscp/session",
                               vscp_client,
                               0);
        mg_set_request_handler(gpobj->m_web_ctx,
                               "/vscp/configure",
                               vscp_configure_list,
                               0);
        mg_set_request_handler(gpobj->m_web_ctx,
                               "/vscp/interfaces",
                               vscp_interface,
                               0);
        mg_set_request_handler(gpobj->m_web_ctx,
                               "/vscp/ifinfo",
                               vscp_interface_info,
                               0);
        mg_set_request_handler(gpobj->m_web_ctx,
                               "/vscp/password",
                               vscp_password,
                               0);
        mg_set_request_handler(gpobj->m_web_ctx,
                               "/vscp/restart",
                               vscp_restart,
                               0);
        mg_set_request_handler(gpobj->m_web_ctx, "/vscp/log", vscp_log, 0);
    }

    // REST
    if (gpobj->m_bEnableRestApi) {
        mg_set_request_handler(gpobj->m_web_ctx,
                               "/vscp/rest",
                               websrv_restapi,
                               0);
    }

    return 1;
}

///////////////////////////////////////////////////////////////////////////////
// stop_webserver
//

int
stop_webserver(void)
{
    // Stop the web server
    mg_stop(gpobj->m_web_ctx);

    // Exit web-server interface
    mg_exit_library();

    return 1;
}
