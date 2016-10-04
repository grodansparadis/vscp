// vscpwebserver.h
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

#if !defined(VSCPWEBSERVER_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_)
#define VSCPWEBSERVER_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_


#include "wx/thread.h"
#include "wx/socket.h"

#include "userlist.h"
#include "controlobject.h"


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
                            struct websock_session *pSession,
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
                                struct websock_session *pSession,
                                wxString& strUser, wxString& strKey );

    /*!
        Handle incoming websocket command
        @param nc Webserver connection handle
    */
    void
    websock_command( struct mg_connection *nc,
                        struct http_message *hm,
                        struct websocket_message *wm,
                        struct websock_session *pSession,
                        wxString& strCommand );

    /*!
        Create a new websocket session
        @param nc Webserver connection handle
        @return websock object or NULL if failure
    */
    struct websock_session *
    websock_new_session( struct mg_connection *nc,
                            struct http_message *hm );

    /*!
        Remove staled sessions
    */
    void
    websock_expire_sessions( struct mg_connection *nc,
                                struct http_message *hm );

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
