// ControlObject.h: interface for the CControlObject class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (http://www.vscp.org)
//
// Copyright (C) 2000-2014 
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

#if !defined(CONTROLOBJECT_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_)
#define CONTROLOBJECT_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_

#ifdef WIN32
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#endif

// NS_ENABLE_SSL		// Enable webserver SSL functionality

#include "wx/wx.h"
#include <wx/thread.h>

#include "../../common/mongoose.h"

#include "devicelist.h"
#include "clientlist.h"
#include "interfacelist.h"
#include "userlist.h"
#include "tables.h"
#include "../../vscp/common/vscp.h"
#include "vscpvariable.h"
#include "tcpipclientthread.h"
#include "daemonvscp.h"
#include "udpthread.h"
#include "dm.h"
#include "vscp.h"


// List used for websocket triggers
WX_DECLARE_LIST(vscpEventFilter, TRIGGERLIST);

WX_DECLARE_STRING_HASH_MAP( wxString, HashString );

#define DAEMON_LOGMSG_NONE                      0
#define DAEMON_LOGMSG_INFO                      1		
#define DAEMON_LOGMSG_NOTICE                    2
#define DAEMON_LOGMSG_WARNING                   3
#define DAEMON_LOGMSG_ERROR                     4
#define DAEMON_LOGMSG_CRITICAL                  5
#define DAEMON_LOGMSG_ALERT                     6
#define DAEMON_LOGMSG_EMERGENCY                 7
#define DAEMON_LOGMSG_DEBUG                     8


#define MAX_ITEMS_RECEIVE_QUEUE                 1021
#define MAX_ITEMS_SEND_QUEUE                    1021
#define MAX_ITEMS_CLIENT_RECEIVE_QUEUE          8191


WX_DECLARE_LIST(canalMsg, CanalMsgList);
WX_DECLARE_LIST(vscpEvent, VSCPEventList);

/*!
    This class implement a thread that handles
    client receive events
 */

class clientMsgWorkerThread : public wxThread {
public:

    /// Constructor
    clientMsgWorkerThread();

    /// Destructor
    virtual ~clientMsgWorkerThread();

    /*!
        Thread code entry point
     */
    virtual void *Entry();


    /*! 
        called when the thread exits - whether it terminates normally or is
        stopped with Delete() (but not when it is Kill()ed!)
     */
    virtual void OnExit();

    /*!
        Termination control
     */
    bool m_bQuit;

    /*!
        Pointer to control object.
     */
    CControlObject *m_pCtrlObject;

};


/*!
	This is the class that does the main work in the daemon.
*/

class CControlObject {
public:

    /*!
        Constructor
     */
    CControlObject(void);

    /*!
        Destructor
     */
    virtual ~CControlObject(void);

    /*!
      logMsg
      write log message
    */ 
    void logMsg(const wxString& wxstr, unsigned char level = DAEMON_LOGMSG_INFO);

    /*!
        General initialization
     */
    bool init(wxString& strcfgfile);

    /*!
        Clean up used resources
     */
    bool cleanup(void);

    /*!
        The main worker thread
     */
    bool run(void);

    /*!
        Start worker threads for devices
        @return true on success
     */
    bool startDeviceWorkerThreads(void);

    /*!
        Stop worker threads for devices
        @return true on success	
     */
    bool stopDeviceWorkerThreads(void);

    /*!
        Starting daemon worker thread
        @return true on success
     */
    bool startDaemonWorkerThread(void);

    /*!
        Stop daemon worker thread
        @return true on success
     */
    bool stopDaemonWorkerThread(void);


    /*!
        Starting TCP/IP worker thread
        @return true on success
     */
    bool startTcpWorkerThread(void);

    /*!
        Stop the TCP/IP worker thread
        @return true on success
     */
    bool stopTcpWorkerThread(void);

    /*!
        Starting Client worker thread
        @return true on success
     */
    bool startClientWorkerThread(void);

    /*!
        Stop Client worker thread
        @return true on success
     */
    bool stopClientWorkerThread(void);

    /*!
        Add a new client to the clinet list

        @param Pointer to client that should be added.
        @param Normally not used but can be used to set a special 
        client id.
     */
    void addClient(CClientItem *pClientItem, uint32_t id = 0);

    /*!
        Remove a new client from the clinet list

        @param pClientItem Pointer to client that should be added.
     */
    void removeClient(CClientItem *pClientItem);


    /*!
        Get device address for primary etehernet adapter

        @param guid class
     */
    bool getMacAddress(cguid& guid);


    /*!
        Get the first IP address computer is known under

        @param pGUID Pointer to GUID class
     */
    bool getIPAddress(cguid& guid);

    /*!
        Read configuration data
        @param strcfgfile path to configuration file.
        @return Returns true on success false on failure.
     */
    bool readConfiguration(wxString& strcfgfile);

	/*!
         Save configuration data
     */
    bool saveConfiguration(void);

	
	/*!
		Read in mime types
		@param path path to XML file holding mime types
		@return Returns true on success false on failure.
	 */
	bool readMimeTypes(wxString& path);

    /*!
        send level II message to all clients
     */
    void sendEventToClient(CClientItem *pClientItem, vscpEvent *pEvent);

    /*!
        Send Level II event to all clients witch exception
     */
    void sendEventAllClients(vscpEvent *pEvent, uint32_t excludeID = 0);


    /*!
        Get clientmap index from a client id
     */
    uint32_t getClientMapFromId(uint32_t clid);

    /*!
        Get a client id from a clinet map index
     */
    uint32_t getClientMapFromIndex(uint32_t idx);

    /*!
        Add a client id to the clientmap
     */
    uint32_t addIdToClientMap(uint32_t clid);

    /*!
        Remove a client id to the clientmap
     */
    bool removeIdFromClientMap(uint32_t clid);


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
	websrv_check_password( const char *method, const char *ha1, const char *uri,
								const char *nonce, const char *nc, const char *cnonce,
								const char *qop, const char *response );

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
		@return MG_TRUE ocn sucess or MG_FALSE on failure.
	*/
	int 
	websrv_websocket_message( struct mg_connection *conn );
	
	/**
		Web server event handler
		@param conn Webserver connection handle
		@param ev Weberserver event
		@return MG_TRUE ocn sucess or MG_FALSE on failure.
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
		@return MG_TRUE ocn sucess or MG_FALSE on failure.
	*/
	int
	websrv_restapi( struct mg_connection *conn );

	/*!
		webserv_rest_doStatus - Get session ststua
		@param conn Webserver connection handle.
		@param pSession Active session or NULL if no session active
		@param format The format output should be formated in, plain, csv, xml, json, jsonp
		@return MG_TRUE ocn sucess or MG_FALSE on failure.
	*/
	int
	webserv_rest_doStatus( struct mg_connection *conn, struct websrv_rest_session *pSession, int format );

	/*!
		webserv_rest_doOpen - Open session
		@param conn Webserver connection handle.
		@param pSession Active session or NULL if no session active
		@param format The format output should be formated in, plain, csv, xml, json, jsonp
		@return MG_TRUE ocn sucess or MG_FALSE on failure.
	*/
	int
	webserv_rest_doOpen( struct mg_connection *conn, struct websrv_rest_session *pSession, CUserItem *pUser, int format );


	/*!
		webserv_rest_doOpen - Close session
		@param conn Webserver connection handle.
		@param pSession Active session or NULL if no session active
		@param format The format output should be formated in, plain, csv, xml, json, jsonp
		@return MG_TRUE ocn sucess or MG_FALSE on failure.
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
		@return MG_TRUE ocn sucess or MG_FALSE on failure.
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
		@return MG_TRUE ocn sucess or MG_FALSE on failure.
	*/
	int
	webserv_rest_doReceiveEvent( struct mg_connection *conn, 
											struct websrv_rest_session *pSession, 
											int format,
											long count=1 );

	/*!
		webserv_rest_doSendEvent - Send VSCP event
		@param conn Webserver connection handle.
		@param pSession Active session or NULL if no session active
		@param format The format output should be formated in, plain, csv, xml, json, jsonp
		@param vscpfilter Filter to set.
		@return MG_TRUE ocn sucess or MG_FALSE on failure.
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
		@return MG_TRUE ocn sucess or MG_FALSE on failure.
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
		@return MG_TRUE ocn sucess or MG_FALSE on failure.
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
		@return MG_TRUE ocn sucess or MG_FALSE on failure.
	*/
	int
	webserv_rest_doWriteVariable( struct mg_connection *conn, 
												struct websrv_rest_session *pSession, 
												int format,
												wxString& strVariable );

	/*!
		webserv_rest_doWriteVariable - Write a variable value
		@param conn Webserver connection handle.
		@param pSession Active session or NULL if no session active
		@param format The format output should be formated in, plain, csv, xml, json, jsonp
		@param strType Measurement type
		@param strMeasurement Measurement value (integer/long/float)
		@param strUnit Measurement unit
		@param strSensorIdx Sensor Index 
		@return MG_TRUE ocn sucess or MG_FALSE on failure.
	*/
	int
	webserv_rest_doWriteMeasurement( struct mg_connection *conn, 
												struct websrv_rest_session *pSession, 
												int format,
												wxString& strType,
												wxString& strMeasurement,
												wxString& strUnit,
												wxString& strSensorIdx );

	/*!
		webserv_rest_doWriteVariable - Write a variable value
		@param conn Webserver connection handle.
		@param pSession Active session or NULL if no session active
		@param format The format output should be formated in, plain, csv, xml, json, jsonp
		@param name Name of table
		@param from Date/time from which data should be collected
		@param to Date/time to which data should be collected
		@return MG_TRUE ocn sucess or MG_FALSE on failure.
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


	

	//////////////////////////////////////////////////////////////////////////////
	//							 ADMIN INTERFACE                                //
	//////////////////////////////////////////////////////////////////////////////


	/*!
		websrv_mainpage - Web server main page renderer.
		@param conn Webserver connection handle.
		@return MG_TRUE ocn sucess or MG_FALSE on failure.
	*/
	int
	websrv_mainpage( struct mg_connection *conn );

	/**
	 * websrv_interfaces -  that displays the available interfaces.
	 *
	 * @param conn Webserver connection handle.
	 * @return MG_TRUE ocn sucess or MG_FALSE on failure.
	 */
	int
	websrv_interfaces( struct mg_connection *conn );	
	
	/**
	 * websrv_dmlist -  that displays the decision matrix list 
	 *
	 * @param conn Webserver connection handle.
	 * @return MG_TRUE ocn sucess or MG_FALSE on failure.
	 */
	int
	websrv_dmlist( struct mg_connection *conn );
	
	/**
	 * websrv_dmedit - edit of one decision matrix entry 
	 *
	 * @param conn Webserver connection handle.
	 * @return MG_TRUE ocn sucess or MG_FALSE on failure.
	 */
	int
	websrv_dmedit( struct mg_connection *conn );
	
	/**
	 * websrv_dmpost - post of one decision matrix entry 
	 *
	 * @param conn Webserver connection handle.
	 * @return MG_TRUE ocn sucess or MG_FALSE on failure.
	 */
	int 
	websrv_dmpost( struct mg_connection *conn );
	
	/**
	 * websrv_dmdelete - Delete DMe entries 
	 *
	 * @param conn Webserver connection handle.
	 * @return MG_TRUE ocn sucess or MG_FALSE on failure.
	 */
	int 
	websrv_dmdelete( struct mg_connection *conn );
	
	/**
	 * websrv_variables_list - that displays the variable list 
	 *
	 * @param conn Webserver connection handle.
	 * @return MG_TRUE ocn sucess or MG_FALSE on failure.
	 */
	int
	websrv_variables_list( struct mg_connection *conn );
	
	/**
	 * websrv_variables_edit -  edit of one variable entry 
	 *
	 * @param conn Webserver connection handle.
	 * @return MG_TRUE ocn sucess or MG_FALSE on failure.
	 */
	int
	websrv_variables_edit( struct mg_connection *conn );
	
	/**
	 * websrv_variables_post -  post of one variable entry 
	 *
	 * @param conn Webserver connection handle.
	 * @return MG_TRUE ocn sucess or MG_FALSE on failure.
	 */
	int 
	websrv_variables_post( struct mg_connection *conn );
	
   /**
	 * websrv_variables_new - New variable initial type selection state 
	 *
	 * @param conn Webserver connection handle.
	 * @return MG_TRUE ocn sucess or MG_FALSE on failure.
	 */
	int 
	websrv_variables_new( struct mg_connection *conn );
	
	/**
	 * websrv_variables_delete - Delete DMe entries 
	 *
	 * @param conn Webserver connection handle.
	 * @return MG_TRUE ocn sucess or MG_FALSE on failure.
	 */
	int 
	websrv_variables_delete( struct mg_connection *conn );

	/**
	 * websrv_render_variables_delete - Discover nodes 
	 *
	 * @param conn Webserver connection handle.
	 * @return MG_TRUE ocn sucess or MG_FALSE on failure.
	 */
	int 
	websrv_discovery( struct mg_connection *conn );

	/**
	 * websrv_session - Communication session
	 *
	 * @param conn Webserver connection handle.
	 * @return MG_TRUE ocn sucess or MG_FALSE on failure.
	 */
	int 
	websrv_session( struct mg_connection *conn );

	/**
	 * websrv_configure - Server configuration
	 *
	 * @param conn Webserver connection handle.
	 * @return MG_TRUE ocn sucess or MG_FALSE on failure.
	 */
	int 
	websrv_configure( struct mg_connection *conn );

	/**
	 * websrv_bootload - Device bootload
	 *
	 * @param conn Webserver connection handle.
	 * @return MG_TRUE ocn sucess or MG_FALSE on failure.
	 */
	int 
	websrv_bootload( struct mg_connection *conn );


public:

#ifdef BUILD_VSCPD_SERVICE
    HANDLE m_hEvntSource;
#endif

    /*!
        true if we should quit
     */
    bool m_bQuit;



    /*!
        Maximum number of items in receivequeue for clients
     */
    uint32_t m_maxItemsInClientReceiveQueue;


    /*!
        Server GUID
        This is the GUID for the server
     */
    //uint8_t m_GUID[ 16 ];
	cguid m_guid;

    /*!
        ClientMap
        This structure maps client id's and
        The client map structure
        Maps unsigned log client id's to
        unsigned char id's for the
        GUID
     */
    uint32_t m_clientMap[ VSCP_MAX_CLIENTS ];


    /*!
        TCP Port for TCP Interface
     */
    unsigned short m_TCPPort;

    /*!
        UDP Port 
     */
    unsigned short m_UDPPort;


    /*!
        Log Level
     */
    uint8_t m_logLevel;

    /*!
        Enable control (TCP/IP) interface
     */
    bool m_bTCPInterface;

    /*!
        Interface used for TCP/IP connection
        Default: empty
     */
    wxString m_strTcpInterfaceAddress;


    /*!
        Enable CANAL Driver functionality
     */
    bool m_bCanalDrivers;

    /*!
        Enable VSCP Daemon functionality
     */
    bool m_bVSCPDaemon;

    /*!
        Enable variable usage
     */
    bool m_bVariables;

    /*!
        Hash table for variables
     */
    CVariableStorage m_VSCP_Variables;

    /*!
        Mutex to protect variables
     */
    wxMutex m_variableMutex;

    /*!
        Event source for NT event reporting
     */
#ifdef BUILD_VSCPD_SERVICE
    HANDLE m_hEventSource;
#endif

    // *************************************************************************

    /*!
        Mutex for client queue
     */
    wxMutex m_wxClientMutex;

    // *************************************************************************

    /*!
        Mutex for device queue
     */
    wxMutex m_wxDeviceMutex;

    // *************************************************************************

    /*!
        Enable DM functionality
     */
    bool m_bDM;

    // Daemon Decision Matrix
    CDM m_dm;

    /*!
        TCP Port
     */
    short m_tcpport;

    /*!
        TCP Username
     */
    wxString m_strTcpUserName;

    /*!
        TCP Password
        This is 32 hex characters as MD5 sum of password.
     */
    wxString m_strTcpPassword;


    /*!
        Username for level II divers
     */
    wxString m_driverUsername;

    /*!
        Password for Level II drivers
     */
    wxString m_driverPassword;

    //*****************************************************
    //            websocket/webserver interface
    //*****************************************************

	struct mg_server *m_pwebserver;

	/// Path to file holding mime types
    wxString m_pathToMimeTypeFile;
	
    // Path to filesystem root
    wxString m_pathRoot;

    // Enable disable web socket interface
    bool m_bWebSockets;

    // websocket port
    //uint16_t m_portWebsockets;		// defaults to 7681

	// websocket authentivcation is needed  (if true)
	bool m_bAuthWebsockets;
	
	// Domain for webserver and other net services
	wxString m_authDomain;

    // webserver port 
    unsigned short m_portWebServer;	// defaults to 8080
	
	// Enable/disable full webserver
    bool m_bWebServer;
	
	// Hash table with mime types, gives mime type from
	// file extension.
	HashString m_hashMimeTypes;

    //*****************************************************
    //                     Security
    //*****************************************************

    // Path to SSL certificate
    wxString m_pathCert;

    // Path to SSL key
    wxString m_pathKey;


    //*****************************************************
    //                      Lists
    //*****************************************************
    /*!
        The list with available devices.
     */
    CDeviceList m_deviceList;
    wxMutex m_mutexDeviceList;

    /*!
        The list with active clients.
     */
    CClientList m_clientList;
    wxMutex m_mutexClientList;

    /*!
        The list of active interfaces
     */
    CInterfaceList m_interfaceList;
    wxMutex m_mutexInterfaceList;

    /*!
        The list of users
     */
    CUserList m_userList;
    wxMutex m_mutexUserList;

	/*!
		This is a list with defined tables
	*/
	listVSCPTables m_listTables;
	wxMutex m_mutexTableList;

    // *************************************************************************


    /*!
        Send queue

        This is the send queue for all clients attached to the system. A client
	 *  place events here and the system distribute it to all other clients.
     */
    VSCPEventList m_clientOutputQueue;

    /*!
        Event object to indicate that there is an event in the client output queue.
     */
    wxSemaphore m_semClientOutputQueue;

    /*!
        Mutex for Level II message send queue
     */
    wxMutex m_mutexClientOutputQueue;

    // *************************************************************************


private:

    //*****************************************************
    //                          Threads
    //*****************************************************

    /*!
        controlobject device thread
     */
    clientMsgWorkerThread *m_pclientMsgWorkerThread;

    wxMutex m_mutexclientMsgWorkerThread;

    /*!
        The server thread for the TCP connection interface
     */
    TcpClientListenThread *m_pTcpClientListenThread;

    wxMutex m_mutexTcpClientListenThread;

    /*!
        The server thread for the VSCP daemon
     */
    daemonVSCPThread *m_pdaemonVSCPThread;

    wxMutex m_mutexdaemonVSCPThread;

    /*!
        UDP Worker threads
     */
    UDPSendThread *m_pudpSendThread;
    UDPReceiveThread *m_pudpReceiveThread;

    wxMutex m_mutexudpSendThread;
    wxMutex m_mutexudpReceiveThread;
};




#endif // !defined(AFX_CONTROLOBJECT_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_)
