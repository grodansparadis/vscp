// vscpwebserver.h
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

#if !defined(VSCPWEBSERVER_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_)
#define VSCPWEBSERVER_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_


#include "wx/thread.h"
#include "wx/socket.h"

#include "userlist.h"
#include "controlobject.h"

/*!
 * Init the webserver sub system
 */
int init_webserver( void );

/*!
 * Init the webserver sub system
 */
int stop_webserver( void );



////////////////////////////////////////////////////////////////////////////////
//                           ws1  Websocket handlers
////////////////////////////////////////////////////////////////////////////////

int ws1_ConnectHandler( const struct web_connection *conn, void *cbdata );
void ws1_ReadyHandler(struct web_connection *conn, void *cbdata);
int ws1_DataHandler( struct web_connection *conn,
                        int bits,
                        char *data,
                        size_t len,
                        void *cbdata );
void ws1_CloseHandler(const struct web_connection *conn, void *cbdata );


#define WEBSRV_MAX_SESSIONS                     1000    // Max web server active sessions
#define WEBSRV_NAL_USERNAMELEN                  128	    // Max length for userdname


/**
 * Invalid method page.
 */
#define WEBSERVER_METHOD_ERROR "<html><head><title>Illegal request</title></head><body>Invalid method.</body></html>"

/**
 * Invalid URL page.
 */
#define WEBSERVER_NOT_FOUND_ERROR "<html><head><title>Not found</title></head><body>The page you are looking for is not available on this server.</body></html>"

/**
 * Start page
 */
#define WEBSERVER_PAGE_MAIN "<html><head><title>VSCP Daemon</title></head><body>Welcome to the VSCP Daemon.</body></html>"

/**
 * Start page
 */
#define WEBSERVER_PAGE "<html><head><title>VSCP Daemon</title></head><body>VSCP Daemon.</body></html>"

/**
 * Available in next version
 */
#define NEXTVERSION_PAGE "<html><head><title>VSCP Daemon</title></head><body>Sorry this functionality is not available until next version of VSCP & Friends.</body></html>"

/**
 * Invalid password
 */
#define WEBSERVER_DENIED "<html><head><title>Access denied</title></head><body>Access denied</body></html>"

/**
 * Name of our cookie.
 */
#define WEBSERVER_COOKIE_NAME "____vscp_session____"


static const char *html_form =
  "<html><body>POST example."
  "<form method=\"POST\" action=\"/handle_post_request\">"
  "Input 1: <input type=\"text\" name=\"input_1\" /> <br/>"
  "Input 2: <input type=\"text\" name=\"input_2\" /> <br/>"
  "<input type=\"submit\" />"
  "</form></body></html>";


/**
 * State we keep for each user/session/browser.
 */
struct websrv_Session
{
  // We keep all sessions in a linked list.
  struct websrv_Session *m_next;

  // Unique ID for this session. 
  char m_sid[33];

  // Reference counter giving the number of connections
  // currently using this session.
  unsigned int m_referenceCount;

  // Time when this session was last active.
  time_t lastActiveTime;

  // Client item for this session
  CClientItem *m_pClientItem;

  // User
  CUserItem *m_pUserItem;
    
  // String submitted via form.
  char value_1[64];

  // Another value submitted via form.
  char value_2[64];
  
  // Keypairs
  HashString m_keys;
  
};





/* Test Certificate */
const char cert_pem[] =
  "-----BEGIN CERTIFICATE-----\n"
  "MIICpjCCAZCgAwIBAgIESEPtjjALBgkqhkiG9w0BAQUwADAeFw0wODA2MDIxMjU0\n"
  "MzhaFw0wOTA2MDIxMjU0NDZaMAAwggEfMAsGCSqGSIb3DQEBAQOCAQ4AMIIBCQKC\n"
  "AQC03TyUvK5HmUAirRp067taIEO4bibh5nqolUoUdo/LeblMQV+qnrv/RNAMTx5X\n"
  "fNLZ45/kbM9geF8qY0vsPyQvP4jumzK0LOJYuIwmHaUm9vbXnYieILiwCuTgjaud\n"
  "3VkZDoQ9fteIo+6we9UTpVqZpxpbLulBMh/VsvX0cPJ1VFC7rT59o9hAUlFf9jX/\n"
  "GmKdYI79MtgVx0OPBjmmSD6kicBBfmfgkO7bIGwlRtsIyMznxbHu6VuoX/eVxrTv\n"
  "rmCwgEXLWRZ6ru8MQl5YfqeGXXRVwMeXU961KefbuvmEPccgCxm8FZ1C1cnDHFXh\n"
  "siSgAzMBjC/b6KVhNQ4KnUdZAgMBAAGjLzAtMAwGA1UdEwEB/wQCMAAwHQYDVR0O\n"
  "BBYEFJcUvpjvE5fF/yzUshkWDpdYiQh/MAsGCSqGSIb3DQEBBQOCAQEARP7eKSB2\n"
  "RNd6XjEjK0SrxtoTnxS3nw9sfcS7/qD1+XHdObtDFqGNSjGYFB3Gpx8fpQhCXdoN\n"
  "8QUs3/5ZVa5yjZMQewWBgz8kNbnbH40F2y81MHITxxCe1Y+qqHWwVaYLsiOTqj2/\n"
  "0S3QjEJ9tvklmg7JX09HC4m5QRYfWBeQLD1u8ZjA1Sf1xJriomFVyRLI2VPO2bNe\n"
  "JDMXWuP+8kMC7gEvUnJ7A92Y2yrhu3QI3bjPk8uSpHea19Q77tul1UVBJ5g+zpH3\n"
  "OsF5p0MyaVf09GTzcLds5nE/osTdXGUyHJapWReVmPm3Zn6gqYlnzD99z+DPIgIV\n"
  "RhZvQx74NQnS6g==\n" "-----END CERTIFICATE-----\n";

const char key_pem[] =
  "-----BEGIN RSA PRIVATE KEY-----\n"
  "MIIEowIBAAKCAQEAtN08lLyuR5lAIq0adOu7WiBDuG4m4eZ6qJVKFHaPy3m5TEFf\n"
  "qp67/0TQDE8eV3zS2eOf5GzPYHhfKmNL7D8kLz+I7psytCziWLiMJh2lJvb2152I\n"
  "niC4sArk4I2rnd1ZGQ6EPX7XiKPusHvVE6VamacaWy7pQTIf1bL19HDydVRQu60+\n"
  "faPYQFJRX/Y1/xpinWCO/TLYFcdDjwY5pkg+pInAQX5n4JDu2yBsJUbbCMjM58Wx\n"
  "7ulbqF/3lca0765gsIBFy1kWeq7vDEJeWH6nhl10VcDHl1PetSnn27r5hD3HIAsZ\n"
  "vBWdQtXJwxxV4bIkoAMzAYwv2+ilYTUOCp1HWQIDAQABAoIBAArOQv3R7gmqDspj\n"
  "lDaTFOz0C4e70QfjGMX0sWnakYnDGn6DU19iv3GnX1S072ejtgc9kcJ4e8VUO79R\n"
  "EmqpdRR7k8dJr3RTUCyjzf/C+qiCzcmhCFYGN3KRHA6MeEnkvRuBogX4i5EG1k5l\n"
  "/5t+YBTZBnqXKWlzQLKoUAiMLPg0eRWh+6q7H4N7kdWWBmTpako7TEqpIwuEnPGx\n"
  "u3EPuTR+LN6lF55WBePbCHccUHUQaXuav18NuDkcJmCiMArK9SKb+h0RqLD6oMI/\n"
  "dKD6n8cZXeMBkK+C8U/K0sN2hFHACsu30b9XfdnljgP9v+BP8GhnB0nCB6tNBCPo\n"
  "32srOwECgYEAxWh3iBT4lWqL6bZavVbnhmvtif4nHv2t2/hOs/CAq8iLAw0oWGZc\n"
  "+JEZTUDMvFRlulr0kcaWra+4fN3OmJnjeuFXZq52lfMgXBIKBmoSaZpIh2aDY1Rd\n"
  "RbEse7nQl9hTEPmYspiXLGtnAXW7HuWqVfFFP3ya8rUS3t4d07Hig8ECgYEA6ou6\n"
  "OHiBRTbtDqLIv8NghARc/AqwNWgEc9PelCPe5bdCOLBEyFjqKiT2MttnSSUc2Zob\n"
  "XhYkHC6zN1Mlq30N0e3Q61YK9LxMdU1vsluXxNq2rfK1Scb1oOlOOtlbV3zA3VRF\n"
  "hV3t1nOA9tFmUrwZi0CUMWJE/zbPAyhwWotKyZkCgYEAh0kFicPdbABdrCglXVae\n"
  "SnfSjVwYkVuGd5Ze0WADvjYsVkYBHTvhgRNnRJMg+/vWz3Sf4Ps4rgUbqK8Vc20b\n"
  "AU5G6H6tlCvPRGm0ZxrwTWDHTcuKRVs+pJE8C/qWoklE/AAhjluWVoGwUMbPGuiH\n"
  "6Gf1bgHF6oj/Sq7rv/VLZ8ECgYBeq7ml05YyLuJutuwa4yzQ/MXfghzv4aVyb0F3\n"
  "QCdXR6o2IYgR6jnSewrZKlA9aPqFJrwHNR6sNXlnSmt5Fcf/RWO/qgJQGLUv3+rG\n"
  "7kuLTNDR05azSdiZc7J89ID3Bkb+z2YkV+6JUiPq/Ei1+nDBEXb/m+/HqALU/nyj\n"
  "P3gXeQKBgBusb8Rbd+KgxSA0hwY6aoRTPRt8LNvXdsB9vRcKKHUFQvxUWiUSS+L9\n"
  "/Qu1sJbrUquKOHqksV5wCnWnAKyJNJlhHuBToqQTgKXjuNmVdYSe631saiI7PHyC\n"
  "eRJ6DxULPxABytJrYCRrNqmXi5TCiqR2mtfalEMOPxz8rUU8dYyx\n"
  "-----END RSA PRIVATE KEY-----\n";

/**
 * How many bytes of a file do we give to libmagic to determine the mime type?
 * 16k might be a bit excessive, but ought not hurt performance much anyway,
 * and should definitively be on the safe side.
 */
#define WEBSERVER_MAGIC_HEADER_SIZE (16 * 1024)


/*!
    This class implement the VSCP Webserver thread
*/

class VSCPWebServerThread : public wxThread
{

public:

    /// Constructor
    VSCPWebServerThread();

    /// Destructor
    ~VSCPWebServerThread();

    /*!
        Thread code entry point
    */
    virtual void *Entry();


    /*!
        Called when the thread exits - whether it terminates normally or is
        stopped with Delete() (but not when it is Kill()ed!)
    */
    virtual void OnExit();

    /////////////////////////////////////////////////
    //                   GENERAL
    /////////////////////////////////////////////////

    /*!
        Read in mime types
        @param path path to XML file holding mime types
        @return Returns true on success false on failure.
     */
    bool readMimeTypes(wxString& path);




    /////////////////////////////////////////////////
    //                 WEB SERVER
    /////////////////////////////////////////////////



    /*!
        Check web server password
        @param method
        @param ha1
        @param uri
        @param nonce
        @param nc
        @param cnonce
        @param qop
        @param reponse
        @return FALSE on failure to validate user or TRUE on success
    */
    static int
    websrv_check_password( const char *method,
                                const char *ha1,
                                const char *uri,
                                const char *nonce,
                                const char *nc,
                                const char *cnonce,
                                const char *qop,
                                const char *response );

    /*!
        Return the session handle for this connection, or
        create one if this is a new user.
    */
    struct websrv_Session *
    websrv_get_session( struct mg_connection *nc,
                            struct http_message *hm );

    /**
     * Add header to response to set a session cookie.
     *
     * @param nc Webserver connection handle
     * @param session session to use
     * @param response response to modify
     */
    websrv_Session *
    websrv_add_session_cookie( struct mg_connection *nc,
                                struct http_message *hm );

    /*!
        Get/Create web session
        @param session session to use
        @return Pointer to session or NULL if failure to get or create session
    */
    struct websrv_Session *
    websrv_GetCreateSession( struct mg_connection *nc,
                                struct http_message *hm );

    /**
        Clean up handles of sessions that have been idle for
        too long.
    */
    void
    websrv_expire_sessions( struct mg_connection *nc,
                                struct http_message *hm );


    /*!
        Handle websocket message
        @param nc Webserver connection handle
        @return TRUE on success or FALSE on failure.
    */
    bool
    websrv_websocket_message( struct mg_connection *nc,
                                struct http_message *hm,
                                struct websocket_message *wm );

    /*!
        Web server event handler
        @param nc Webserver connection handle
        @param ev Weberserver event
        @return TRUE on success or FALSE on failure.
    */
    static void websrv_event_handler( struct mg_connection *nc,
                                        int ev,
                                        void *pUser );


    ////////////////////////////////////////////////////////////////////////////
    //                           ADMIN INTERFACE                              //
    ////////////////////////////////////////////////////////////////////////////


    /*!
        List text file content. Built to display logfiles
        @param nc Webserver connection handle.
        @param file File to display
        @param textHeader Headtext to display
    */
    void websrv_listFile( struct mg_connection *nc,
                            wxFileName& logfile,
                            wxString& textHeader );


    /*!
        websrv_mainpage - Web server main page renderer.
        @param nc Webserver connection handle.
    */
    void
    websrv_mainpage( struct mg_connection *nc,
                        struct http_message *hm );

    /**
     * websrv_interfaces -  that displays the available interfaces.
     *
     * @param nc Webserver connection handle.
     */
    void
    websrv_interfaces( struct mg_connection *nc,
                        struct http_message *hm );

    /**
     * websrv_dmlist -  that displays the decision matrix list
     *
     * @param nc Webserver connection handle.
     */
    void
    websrv_dmlist( struct mg_connection *nc,
                    struct http_message *hm );

    /**
     * websrv_dmedit - edit of one decision matrix entry
     *
     * @param nc Webserver connection handle.
     */
    void
    websrv_dmedit( struct mg_connection *nc,
                    struct http_message *hm );

    /**
     * websrv_dmpost - post of one decision matrix entry
     *
     * @param nc Webserver connection handle.
     */
    void
    websrv_dmpost( struct mg_connection *nc,
                    struct http_message *hm );

    /**
     * websrv_dmdelete - Delete DMe entries
     *
     * @param nc Webserver connection handle.
     */
    void
    websrv_dmdelete( struct mg_connection *nc,
                        struct http_message *hm );

    /**
     * websrv_variables_list - that displays the variable list
     *
     * @param nc Webserver connection handle.
     */
    void
    websrv_variables_list( struct mg_connection *nc,
                            struct http_message *hm );

    /**
     * websrv_variables_edit -  edit of one variable entry
     *
     * @param nc Webserver connection handle.
     */
    void
    websrv_variables_edit( struct mg_connection *nc,
                            struct http_message *hm );

    /**
     * websrv_variables_post -  post of one variable entry
     *
     * @param nc Webserver connection handle.
     */
    void
    websrv_variables_post( struct mg_connection *nc,
                            struct http_message *hm );

   /**
     * websrv_variables_new - New variable initial type selection state
     *
     * @param nc Webserver connection handle.
     */
    void
    websrv_variables_new( struct mg_connection *nc,
                            struct http_message *hm );

    /**
     * websrv_variables_delete - Delete DMe entries
     *
     * @param nc Webserver connection handle.
     */
    void
    websrv_variables_delete( struct mg_connection *nc,
                                struct http_message *hm );

    /**
     * websrv_render_variables_delete - Discover nodes
     *
     * @param nc Webserver connection handle.
     */
    void
    websrv_discovery( struct mg_connection *nc,
                        struct http_message *hm );

    /**
     * websrv_session - Communication session
     *
     * @param nc Webserver connection handle.
     */
    void
    websrv_session( struct mg_connection *nc,
                        struct http_message *hm );

    /**
     * websrv_configure - Server configuration
     *
     * @param nc Webserver connection handle.
     */
    void
    websrv_configure( struct mg_connection *nc,
                        struct http_message *hm );

    /**
     * websrv_bootload - Device bootload
     *
     * @param nc Webserver connection handle.
     */
    void
    websrv_bootload( struct mg_connection *nc,
                        struct http_message *hm );

    /**
     * websrv_table - View infor about defined tables
     *
     * @param nc Webserver connection handle.
     */
    void
    websrv_table( struct mg_connection *nc,
                    struct http_message *hm );


    /**
     * websrv_tablelist - List table
     *
     * @param nc Webserver connection handle.
     */
    void
    websrv_tablelist( struct mg_connection *nc,
                        struct http_message *hm );



    /////////////////////////////////////////////////
    //                  WEBSOCKETS
    /////////////////////////////////////////////////



    /*!
        Handle web socket receive
    */
    //void
    //handleWebSocketReceive( struct mg_connection *nc );

    /*!
        Handle websocket send event
    */
    bool
    websock_sendevent( struct mg_connection *nc,
                            websock_session *pSession,
                            vscpEvent *pEvent );

    /*!
        Authenticate client
        @param nc Webserver connection handle
        @param strUser Username
        @param strKey hash of
            "user:standard vscp password hash:Sec-WebSocket-Key:session-hash"
            VSCP standard password hash =
            "admin:mydomain.com:secret" = 5ebe2294ecd0e0f08eab7690d2a6ee69
    */
    bool
    websock_authentication( struct mg_connection *nc,
                                struct http_message *hm,
                                websock_session *pSession,
                                wxString& strUser, 
                                wxString& strKey );

    /*!
        Handle incoming websocket command
        @param nc Webserver connection handle
    */
    void
    websock_command( struct mg_connection *nc,
                        struct http_message *hm,
                        struct websocket_message *wm,
                        websock_session *pSession,
                        wxString& strCommand );

    /*!
        Create a new websocket session
        @param nc Webserver connection handle
        @return websock object or NULL if failure
    */
    websock_session *
    websock_new_session( struct mg_connection *nc,
                            struct http_message *hm );

    /*!
        Remove staled sessions
    */
    /*void
    websock_expire_sessions( struct mg_connection *nc,
                                struct http_message *hm );*/

    /*!
        Post incomming event on open websockets
    */
    void
    websock_post_incomingEvents( void );



    //////////////////////////////////////////////////////////////////////////////
    //                           REST INTERFACE                                 //
    //////////////////////////////////////////////////////////////////////////////



    /*!
        websrv_new_rest_session - Create new rest session
        @param nc Webserver connection handle.
        @return pointer to session or NULL if session is not found.
    */
    struct websrv_rest_session *
    websrv_new_rest_session( struct mg_connection *nc, CUserItem *pUser );

    /*!
        websrv_get_rest_session - fetch rest sesson if it is available
        @param nc Webserver connection handle.
        @param SessinId Identifier for the session.
        @return pointer to session or NULL if session is not found.
    */
    struct websrv_rest_session *
    websrv_get_rest_session( struct mg_connection *nc, wxString &SessionId );

    /*!
        websrv_expire_rest_sessions - expire staled rest sesson(s)
        @param nc Webserver connection handle.
        @return pointer to session or NULL if session is not found.
    */
    void
    websrv_expire_rest_sessions( struct mg_connection *nc );

    /**
        websrv_restapi - REST interface main handler
        @param nc Webserver connection handle.
    */
    void
    websrv_restapi( struct mg_connection *nc,
                        struct http_message *hm );

    /*!
        webserv_rest_doStatus - Get session status
        @param nc Webserver connection handle.
        @param pSession Active session or NULL if no session active
        @param format The format output should be formated in, plain,
         *      csv, xml, json, jsonp
    */
    void
    webserv_rest_doStatus( struct mg_connection *nc,
                            struct websrv_rest_session *pSession,
                            int format );

    /*!
        webserv_rest_doOpen - Open session
        @param nc Webserver connection handle.
        @param pSession Active session or NULL if no session active
        @param format The format output should be formated in, plain, csv,
         *          xml, json, jsonp
    */
    void
    webserv_rest_doOpen( struct mg_connection *nc,
                            struct websrv_rest_session *pSession,
                            CUserItem *pUser, int format );


    /*!
        webserv_rest_doOpen - Close session
        @param nc Webserver connection handle.
        @param pSession Active session or NULL if no session active
        @param format The format output should be formated in, plain, csv,
                xml, json, jsonp
    */
    void
    webserv_rest_doClose( struct mg_connection *nc,
                            struct websrv_rest_session *pSession,
                            int format );


    /*!
        webserv_rest_doSendEvent - Send VSCP event
        @param nc Webserver connection handle.
        @param pSession Active session or NULL if no session active
        @param format The format output should be formated in, plain, csv,
                    xml, json, jsonp
        @param pEvent Pointer to VSCP event to send. The framework handels
                deletion of this pointer.
    */
    void
    webserv_rest_doSendEvent( struct mg_connection *nc,
                                            struct websrv_rest_session *pSession,
                                            int format,
                                            vscpEvent *pEvent );

    /*!
        webserv_rest_doReceiveEvent - Receive VSCP event
        @param nc Webserver connection handle.
        @param pSession Active session or NULL if no session active
        @param format The format output should be formated in, plain, csv,
                xml, json, jsonp
        @param count Number of events to read.
    */
    void
    webserv_rest_doReceiveEvent( struct mg_connection *nc,
                                            struct websrv_rest_session *pSession,
                                            int format,
                                            size_t count=1 );

    /*!
        webserv_rest_doSendEvent - Send VSCP event
        @param nc Webserver connection handle.
        @param pSession Active session or NULL if no session active
        @param format The format output should be formated in, plain, csv,
                    xml, json, jsonp
        @param vscpfilter Filter to set.
    */
    void
    webserv_rest_doSetFilter( struct mg_connection *nc,
                                            struct websrv_rest_session *pSession,
                                            int format,
                                            vscpEventFilter& vscpfilter );

    /*!
        webserv_rest_doSendEvent - Send VSCP event
        @param nc Webserver connection handle.
        @param pSession Active session or NULL if no session active
        @param format The format output should be formated in, plain, csv, xml, json, jsonp
    */
    void
    webserv_rest_doClearQueue( struct mg_connection *nc,
                                    struct websrv_rest_session *pSession,
                                    int format );

    /*!
        webserv_rest_doReadVariable - Read a variable value
        @param nc Webserver connection handle.
        @param pSession Active session or NULL if no session active
        @param format The format output should be formated in, plain, csv,
                xml, json, jsonp
        @param strVariableName Name of variable to read
    */
    void
    webserv_rest_doReadVariable( struct mg_connection *nc,
                                    struct websrv_rest_session *pSession,
                                    int format,
                                    wxString& strVariableName );
    
    /*!
        webserv_rest_doReadVariable - Read a variable value
        @param nc Webserver connection handle.
        @param pSession Active session or NULL if no session active
        @param format The format output should be formated in, plain, csv,
                xml, json, jsonp
        @param strRegEx Regular expression to select whcih variables to list or 
                    empty for all.
        @param bShort Short format (no value, no note) default.
    */
    void
    webserv_rest_doListVariable( struct mg_connection *nc,
                                    struct websrv_rest_session *pSession,
                                    int format,
                                    wxString& strRegEx,
                                    bool bShort = true );

    /*!
        webserv_rest_doWriteVariable - Write a variable value
        @param nc Webserver connection handle.
        @param pSession Active session or NULL if no session active
        @param format The format output should be formated in, plain,
          csv, xml, json, jsonp
        @param strVariableName Name of variable to write new value to.
        @param strValue Value for variable variable
    */
    void
    webserv_rest_doWriteVariable( struct mg_connection *nc,
                                        struct websrv_rest_session *pSession,
                                        int format,
                                        wxString& strVariableName,
                                        wxString& strValue );

    /*!
        webserv_rest_doCreateVariable - Create a variable of a specified type
        @param nc Webserver connection handle.
        @param pSession Active session or NULL if no session active
        @param format The format output should be formated in, plain, csv,
           xml, json, jsonp
        @param strVariable Name of variable to create
        @param strType Type of variable to create. Can be given on text or
           numerical form.
        @param strValue Value of variable to create
        @param strPersistent Should be set to "true" if variable should be
           persistent
        @param strAccessRights Accessrights in string form. Hex or decimal.
        @param strNote Note for variable (BASE64 encoded).
        
    */
    void
    webserv_rest_doCreateVariable( struct mg_connection *nc,
                                        struct websrv_rest_session *pSession,
                                        int format,
                                        wxString& strVariable,
                                        wxString& strType,
                                        wxString& strValue,
                                        wxString& strPersistent,
                                        wxString& strAccessRight,
                                        wxString& strNote );

    /*!
        webserv_rest_doDeleteVariable - Delete a variable
        @param nc Webserver connection handle.
        @param pSession Active session or NULL if no session active
        @param format The format output should be formated in, plain, csv,
           xml, json, jsonp
        @param strVariable Name of variable to delete
    */
    void
    webserv_rest_doDeleteVariable( struct mg_connection *nc,
                                        struct websrv_rest_session *pSession,
                                        int format,
                                        wxString& strVariable  );

    /*!
        webserv_rest_doWriteVariable - Write a variable value
        @param nc Webserver connection handle.
        @param pSession Active session or NULL if no session active
        @param format The format output should be formated in, plain, csv, xml, json, jsonp
        @param GUID GUID to use. Can be left blank for interface GUID to be used.
        @param strLevel VSCP Level event to send (1 or 2)
        @param strType Measurement type
        @param strMeasurement Measurement value (integer/long/float)
        @param strUnit Measurement unit
        @param strSensorIdx Sensor Index
        @param strZone Zone to use
        @param strSubZone SubZone to use
        @param eventFormat Can be string or float
    */
    void
    webserv_rest_doWriteMeasurement( struct mg_connection *nc,
                                                struct websrv_rest_session *pSession,
                                                int format,
                                                wxString& strGuid,
                                                wxString& strLevel,
                                                wxString& strType,
                                                wxString& strMeasurement,
                                                wxString& strUnit,
                                                wxString& strSensorIdx,
                                                wxString& strZone,
                                                wxString& strSubZone,
                                                wxString& strEventFormat );

    /*!
        Render REST table
        @param nc Webserver connection handle.
        @param pSession Active session or NULL if no session active
        @param format The format output should be formated in, plain, csv, xml, json, jsonp
    */
    void websrc_rest_renderTableData( struct mg_connection *nc,
                                        struct websrv_rest_session *pSession,
                                        int format,
                                        wxString& strName,
                                        struct _vscpFileRecord *pRecords,
                                        long nfetchedRecords );

    /*!
        webserv_rest_doWriteVariable - Write a variable value
        @param nc Webserver connection handle.
        @param pSession Active session or NULL if no session active
        @param format The format output should be formated in, plain, csv, xml, json, jsonp
        @param name Name of table
        @param from Date/time from which data should be collected
        @param to Date/time to which data should be collected
        @return TRUE on success or FALSE on failure.
    */
    void
    webserv_rest_doGetTableData( struct mg_connection *nc,
                                                struct websrv_rest_session *pSession,
                                                int format,
                                                wxString& strName,
                                                wxString& strFrom,
                                                wxString& strTo );

    /*!
        webserv_rest_doFetchMDF - Fetch MDF
        @param nc Webserver connection handle.
        @param pSession Active session or NULL if no session active
        @param format The format output should be formated in, plain, csv, xml, json, jsonp
        @param strURL URL for MDF.
        @return TRUE on success or FALSE on failure.
    */
    void
    webserv_rest_doFetchMDF( struct mg_connection *nc,
                                struct websrv_rest_session *pSession,
                                int format,
                                wxString& strURL );

    /*!
        webserv_rest_error - Display error
        @param nc Webserver connection handle.
        @param pSession Active session or NULL if no session active
        @param format The format output should be formated in, plain, csv, xml, json, jsonp
        @param errorcode Code for error.
    */
    void
    webserv_rest_error( struct mg_connection *nc,
                                        struct websrv_rest_session *pSession,
                                        int format,
                                        int errorcode);

    /*!
    webserv_rest_sendHeader - Send HTML header
    @param nc Webserver connection handle.
    @param format The format output should be formated in, plain, csv, xml, json, jsonp
    @param returncode HTML return code
    */
    void
        webserv_rest_sendHeader( struct mg_connection *nc,
                                 int format,
                                 int returncode );

// --- Member variables ---

    /*!
        Termination control
    */
    bool m_bQuit;

    /*!
        Pointer to owner
    */
    CControlObject *m_pCtrlObject;

    // Hash table with mime types, gives mime type from
    // file extension.
    HashString m_hashMimeTypes;

    wxMutex m_websockSessionMutex;   // Protects the session object

};





#endif
