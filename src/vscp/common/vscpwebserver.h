// vscpwebserver.h
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2015 
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
		called when the thread exits - whether it terminates normally or is
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
    //                  WEBSOCKETS
    /////////////////////////////////////////////////



	/*!
		Handle web socket receive
	*/
    //void
    //handleWebSocketReceive( struct mg_connection *conn );

	/*!
		Handle websocket send event
	*/
    int
    websock_sendevent( struct mg_connection *conn, 
							struct websock_session *pSession, 
							vscpEvent *pEvent );

	/*!
		Authenticate client
		@param conn Webserver connection handle
		@param strUser Username
		@param strKey hash of 
			"user:standard vscp password hash:Sec-WebSocket-Key:session-hash"
			VSCP standard password hash =
			"admin:mydomain.com:secret" = 5ebe2294ecd0e0f08eab7690d2a6ee69
	*/
	int
	websock_authentication( struct mg_connection *conn, 
								struct websock_session *pSession, 
								wxString& strUser, wxString& strKey );

	/*!
		Handle incoming websocket command
		@param conn Webserver connection handle
	*/
    int
    websock_command( struct mg_connection *conn, 
						struct websock_session *pSession, 
						wxString& strCommand );

	/*!
		Create a new websocket session
		@param conn Webserver connection handle
		@param pKey Sec-WebSocket-Key
		@param pVer Sec-WebSocket-Version
		@return websock object or NULL if failure
	*/
	struct websock_session *
	websock_new_session( struct mg_connection *conn, const char * pKey, const char * pVer );

	/*!
		Get new websocket session structure
		@param conn Webserver connection handle
		@return websock object or NULL if failure.
	*/
	static struct websock_session *
	websock_get_session( struct mg_connection *conn );

	/*!
		Remove staled sessions
	*/
	void
	websock_expire_sessions( struct mg_connection *conn );

	/*!
		Post incomming event on open websockets
	*/
	void 
	websock_post_incomingEvents( void );


	
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
		@return MG_FALSE on failure to validate user or MG_TRUE on success
	*/
	int 
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
	websrv_get_session( struct mg_connection *conn );

	/**
	 * Add header to response to set a session cookie.
	 *
	 * @param conn Webserver connection handle
	 * @param session session to use
	 * @param response response to modify
	 */
	websrv_Session *
	websrv_add_session_cookie( struct mg_connection *conn, const char * pUser );

	/*!
		Get/Create web session
		@param session session to use
		@return Pointer to session or NULL if failure to get or create session 
	*/
	struct websrv_Session * 
	websrv_GetCreateSession( struct mg_connection *conn );
	
	/**
		Clean up handles of sessions that have been idle for
		too long.
	*/
	void
	websrv_expire_sessions( struct mg_connection *conn );


	/*!
		Handle websocket message
		@param conn Webserver connection handle
		@return MG_TRUE on success or MG_FALSE on failure.
	*/
	int 
	websrv_websocket_message( struct mg_connection *conn );
	
	/**
		Web server event handler
		@param conn Webserver connection handle
		@param ev Weberserver event
		@return MG_TRUE on success or MG_FALSE on failure.
	*/
	static int 
	websrv_event_handler( struct mg_connection *conn, enum mg_event ev );


	//////////////////////////////////////////////////////////////////////////////
	//							 REST INTERFACE                                 //
	//////////////////////////////////////////////////////////////////////////////


	/*!
		websrv_new_rest_session - Create new rest session
		@param conn Webserver connection handle.
		@return pointer to session or NULL if session is not found.
	*/
	struct websrv_rest_session *
	websrv_new_rest_session( struct mg_connection *conn, CUserItem *pUser );

	/*!
		websrv_get_rest_session - fetch rest sesson if it is available	
		@param conn Webserver connection handle.
		@param SessinId Identifier for the session.
		@return pointer to session or NULL if session is not found.
	*/
	struct websrv_rest_session *
	websrv_get_rest_session( struct mg_connection *conn, wxString &SessionId );

	/*!
		websrv_expire_rest_sessions - expire staled rest sesson(s)
		@param conn Webserver connection handle.
		@return pointer to session or NULL if session is not found.
	*/
	void
	websrv_expire_rest_sessions( struct mg_connection *conn );

	/**
		websrv_restapi - REST interface main handler
		@param conn Webserver connection handle.
		@return MG_TRUE on success or MG_FALSE on failure.
	*/
	int
	websrv_restapi( struct mg_connection *conn );

	/*!
		webserv_rest_doStatus - Get session ststua
		@param conn Webserver connection handle.
		@param pSession Active session or NULL if no session active
		@param format The format output should be formated in, plain, csv, xml, json, jsonp
		@return MG_TRUE on success or MG_FALSE on failure.
	*/
	int
	webserv_rest_doStatus( struct mg_connection *conn, struct websrv_rest_session *pSession, int format );

	/*!
		webserv_rest_doOpen - Open session
		@param conn Webserver connection handle.
		@param pSession Active session or NULL if no session active
		@param format The format output should be formated in, plain, csv, xml, json, jsonp
		@return MG_TRUE on success or MG_FALSE on failure.
	*/
	int
	webserv_rest_doOpen( struct mg_connection *conn, struct websrv_rest_session *pSession, CUserItem *pUser, int format );


	/*!
		webserv_rest_doOpen - Close session
		@param conn Webserver connection handle.
		@param pSession Active session or NULL if no session active
		@param format The format output should be formated in, plain, csv, xml, json, jsonp
		@return MG_TRUE on success or MG_FALSE on failure.
	*/
	int
	webserv_rest_doClose( struct mg_connection *conn, struct websrv_rest_session *pSession, int format );


	/*!
		webserv_rest_doSendEvent - Send VSCP event
		@param conn Webserver connection handle.
		@param pSession Active session or NULL if no session active
		@param format The format output should be formated in, plain, csv, xml, json, jsonp
		@param pEvent Pointer to VSCP event to send. The framework handels deletion of
						this pointer.
		@return MG_TRUE on success or MG_FALSE on failure.
	*/
	int
	webserv_rest_doSendEvent( struct mg_connection *conn, 
											struct websrv_rest_session *pSession, 
											int format,
											vscpEvent *pEvent );

	/*!
		webserv_rest_doReceiveEvent - Receive VSCP event
		@param conn Webserver connection handle.
		@param pSession Active session or NULL if no session active
		@param format The format output should be formated in, plain, csv, xml, json, jsonp
		@param count Number of events to read.
		@return MG_TRUE on success or MG_FALSE on failure.
	*/
	int
	webserv_rest_doReceiveEvent( struct mg_connection *conn, 
											struct websrv_rest_session *pSession, 
											int format,
											size_t count=1 );

	/*!
		webserv_rest_doSendEvent - Send VSCP event
		@param conn Webserver connection handle.
		@param pSession Active session or NULL if no session active
		@param format The format output should be formated in, plain, csv, xml, json, jsonp
		@param vscpfilter Filter to set.
		@return MG_TRUE on success or MG_FALSE on failure.
	*/
	int
	webserv_rest_doSetFilter( struct mg_connection *conn, 
											struct websrv_rest_session *pSession, 
											int format,
											vscpEventFilter& vscpfilter );

	/*!
		webserv_rest_doSendEvent - Send VSCP event
		@param conn Webserver connection handle.
		@param pSession Active session or NULL if no session active
		@param format The format output should be formated in, plain, csv, xml, json, jsonp
		@return MG_TRUE on success or MG_FALSE on failure.
	*/
	int
	webserv_rest_doClearQueue( struct mg_connection *conn, 
												struct websrv_rest_session *pSession, 
												int format );

	/*!
		webserv_rest_doReadVariable - Read a variable value
		@param conn Webserver connection handle.
		@param pSession Active session or NULL if no session active
		@param format The format output should be formated in, plain, csv, xml, json, jsonp
		@param strVariableName Name of variable to read
		@return MG_TRUE on success or MG_FALSE on failure.
	*/
	int
	webserv_rest_doReadVariable( struct mg_connection *conn, 
												struct websrv_rest_session *pSession, 
												int format,
												wxString& strVariableName );	

	/*!
		webserv_rest_doWriteVariable - Write a variable value
		@param conn Webserver connection handle.
		@param pSession Active session or NULL if no session active
		@param format The format output should be formated in, plain, csv, xml, json, jsonp
        @param strVariableName Name of variable to write new value to.
        @param strValue Value for variable variable
		@return MG_TRUE on success or MG_FALSE on failure.
	*/
	int
	webserv_rest_doWriteVariable( struct mg_connection *conn, 
												struct websrv_rest_session *pSession, 
												int format,
												wxString& strVariableName,
                                                wxString& strValue );

    /*!
		webserv_rest_doCreateVariable - Create a variable of a specified type
		@param conn Webserver connection handle.
		@param pSession Active session or NULL if no session active
		@param format The format output should be formated in, plain, csv, xml, json, jsonp
        @param strVariable Name of variable to create
        @param strType Type of variable to create. Can be given on text or numerical form.
        @param strValue Value of variable to create
        @param strPersistent Should be set to "true" if variable should be persistent
		@return MG_TRUE on success or MG_FALSE on failure.
	*/
	int
	webserv_rest_doCreateVariable( struct mg_connection *conn, 
												struct websrv_rest_session *pSession, 
												int format,
												wxString& strVariable,
                                                wxString& strType,
                                                wxString& strValue,
                                                wxString& strPersistent );

	/*!
		webserv_rest_doWriteVariable - Write a variable value
		@param conn Webserver connection handle.
		@param pSession Active session or NULL if no session active
		@param format The format output should be formated in, plain, csv, xml, json, jsonp
        @param strLevel VSCP Level event to send (1 or 2)
		@param strType Measurement type
		@param strMeasurement Measurement value (integer/long/float)
		@param strUnit Measurement unit
		@param strSensorIdx Sensor Index 
        @param strZone Zone to use
        @param strSubZone SubZone to use
		@return MG_TRUE on success or MG_FALSE on failure.
	*/
	int
	webserv_rest_doWriteMeasurement( struct mg_connection *conn, 
												struct websrv_rest_session *pSession, 
												int format,
                                                wxString& strLevel,
												wxString& strType,
												wxString& strMeasurement,
												wxString& strUnit,
												wxString& strSensorIdx,
                                                wxString& strZone,
                                                wxString& strSubZone );

	/*!
		webserv_rest_doWriteVariable - Write a variable value
		@param conn Webserver connection handle.
		@param pSession Active session or NULL if no session active
		@param format The format output should be formated in, plain, csv, xml, json, jsonp
		@param name Name of table
		@param from Date/time from which data should be collected
		@param to Date/time to which data should be collected
		@return MG_TRUE on success or MG_FALSE on failure.
	*/
	int
	webserv_rest_doGetTableData( struct mg_connection *conn, 
												struct websrv_rest_session *pSession, 
												int format,
												wxString& strName,
												wxString& strFrom,
												wxString& strTo );

	/*!
		webserv_rest_error - Display error
		@param conn Webserver connection handle.
		@param pSession Active session or NULL if no session active
		@param format The format output should be formated in, plain, csv, xml, json, jsonp
		@param errorcode Code for error.
	*/
	void
	webserv_rest_error( struct mg_connection *conn, 
										struct websrv_rest_session *pSession, 
										int format,
										int errorcode);

    /*!
    webserv_rest_sendHeader - Send HTML header
    @param conn Webserver connection handle.
    @param format The format output should be formated in, plain, csv, xml, json, jsonp
    @param returncode HTML return code
    */
    void
        webserv_rest_sendHeader( struct mg_connection *conn,
                                 int format,
                                 int returncode );
	

	//////////////////////////////////////////////////////////////////////////////
	//							 ADMIN INTERFACE                                //
	//////////////////////////////////////////////////////////////////////////////

    /*!
        List text file content. Built to display logfiles
        @param conn Webserver connection handle.
        @param file File to display
        @param textHeader Headtext to display
	    @return MG_TRUE on success or MG_FALSE on failure.
    */
    int websrv_listFile( struct mg_connection *conn, wxFileName& logfile, wxString& textHeader );


	/*!
		websrv_mainpage - Web server main page renderer.
		@param conn Webserver connection handle.
		@return MG_TRUE on success or MG_FALSE on failure.
	*/
	int
	websrv_mainpage( struct mg_connection *conn );

	/**
	 * websrv_interfaces -  that displays the available interfaces.
	 *
	 * @param conn Webserver connection handle.
	 * @return MG_TRUE on success or MG_FALSE on failure.
	 */
	int
	websrv_interfaces( struct mg_connection *conn );	
	
	/**
	 * websrv_dmlist -  that displays the decision matrix list 
	 *
	 * @param conn Webserver connection handle.
	 * @return MG_TRUE on success or MG_FALSE on failure.
	 */
	int
	websrv_dmlist( struct mg_connection *conn );
	
	/**
	 * websrv_dmedit - edit of one decision matrix entry 
	 *
	 * @param conn Webserver connection handle.
	 * @return MG_TRUE on success or MG_FALSE on failure.
	 */
	int
	websrv_dmedit( struct mg_connection *conn );
	
	/**
	 * websrv_dmpost - post of one decision matrix entry 
	 *
	 * @param conn Webserver connection handle.
	 * @return MG_TRUE on success or MG_FALSE on failure.
	 */
	int 
	websrv_dmpost( struct mg_connection *conn );
	
	/**
	 * websrv_dmdelete - Delete DMe entries 
	 *
	 * @param conn Webserver connection handle.
	 * @return MG_TRUE on success or MG_FALSE on failure.
	 */
	int 
	websrv_dmdelete( struct mg_connection *conn );
	
	/**
	 * websrv_variables_list - that displays the variable list 
	 *
	 * @param conn Webserver connection handle.
	 * @return MG_TRUE on success or MG_FALSE on failure.
	 */
	int
	websrv_variables_list( struct mg_connection *conn );
	
	/**
	 * websrv_variables_edit -  edit of one variable entry 
	 *
	 * @param conn Webserver connection handle.
	 * @return MG_TRUE on success or MG_FALSE on failure.
	 */
	int
	websrv_variables_edit( struct mg_connection *conn );
	
	/**
	 * websrv_variables_post -  post of one variable entry 
	 *
	 * @param conn Webserver connection handle.
	 * @return MG_TRUE on success or MG_FALSE on failure.
	 */
	int 
	websrv_variables_post( struct mg_connection *conn );
	
   /**
	 * websrv_variables_new - New variable initial type selection state 
	 *
	 * @param conn Webserver connection handle.
	 * @return MG_TRUE on success or MG_FALSE on failure.
	 */
	int 
	websrv_variables_new( struct mg_connection *conn );
	
	/**
	 * websrv_variables_delete - Delete DMe entries 
	 *
	 * @param conn Webserver connection handle.
	 * @return MG_TRUE on success or MG_FALSE on failure.
	 */
	int 
	websrv_variables_delete( struct mg_connection *conn );

	/**
	 * websrv_render_variables_delete - Discover nodes 
	 *
	 * @param conn Webserver connection handle.
	 * @return MG_TRUE on success or MG_FALSE on failure.
	 */
	int 
	websrv_discovery( struct mg_connection *conn );

	/**
	 * websrv_session - Communication session
	 *
	 * @param conn Webserver connection handle.
	 * @return MG_TRUE on success or MG_FALSE on failure.
	 */
	int 
	websrv_session( struct mg_connection *conn );

	/**
	 * websrv_configure - Server configuration
	 *
	 * @param conn Webserver connection handle.
	 * @return MG_TRUE on success or MG_FALSE on failure.
	 */
	int 
	websrv_configure( struct mg_connection *conn );

	/**
	 * websrv_bootload - Device bootload
	 *
	 * @param conn Webserver connection handle.
	 * @return MG_TRUE on success or MG_FALSE on failure.
	 */
	int 
	websrv_bootload( struct mg_connection *conn );

    /**
	 * websrv_table - View infor about defined tables
	 *
	 * @param conn Webserver connection handle.
	 * @return MG_TRUE on success or MG_FALSE on failure.
	 */
    int
    websrv_table( struct mg_connection *conn );


    /**
	 * websrv_tablelist - List table
	 *
	 * @param conn Webserver connection handle.
	 * @return MG_TRUE on success or MG_FALSE on failure.
	 */
    int
    websrv_tablelist( struct mg_connection *conn );


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

};





#endif
