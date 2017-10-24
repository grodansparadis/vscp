// ControlObject.h: interface for the CControlObject class.
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

#if !defined(CONTROLOBJECT_H__INCLUDED_)
#define CONTROLOBJECT_H__INCLUDED_

#if defined(_WIN32)
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#endif

#include "wx/wx.h"
#include <wx/thread.h>
#include <wx/filename.h>
#include <wx/list.h>

#include <mongoose.h>
#include <sqlite3.h>

#include <devicelist.h>
#include <clientlist.h>
#include <interfacelist.h>
#include <userlist.h>
#include <tables.h>
#include <vscp.h>
#include <remotevariable.h>
#include <automation.h>
#include <tcpipsrv.h>
#include <udpsrv.h>
#include <multicastsrv.h>
#include <websrv.h>
#include <restsrv.h>
#include <websocket.h>
#include <daemonworker.h>
#include <dm.h>
#include <knownnodes.h>
#include <vscp.h>

// Forward declarations
class VSCPClientThread;
class CVSCPAutomation;

// Log level
enum {
    DAEMON_LOGMSG_NONE = 0,
    DAEMON_LOGMSG_NORMAL,
    DAEMON_LOGMSG_DEBUG,
};

// Log type
enum {
    DAEMON_LOGTYPE_GENERAL = 0,
    DAEMON_LOGTYPE_SECURITY,
    DAEMON_LOGTYPE_ACCESS,
    DAEMON_LOGTYPE_DM
};

// TTL     Scope
// ----------------------------------------------------------------------
// 0       Restricted to the same host.Won't be output by any interface.
// 1       Restricted to the same subnet.Won't be forwarded by a router.
// <32     Restricted to the same site, organization or department.
// <64     Restricted to the same region.
// <128    Restricted to the same continent.
// <255    Unrestricted in scope.Global.
#define IP_MULTICAST_DEFAULT_TTL    1

// Needed on Linux
#ifndef VSCPMIN
#define VSCPMIN(X,Y) ((X) < (Y) ? (X) : (Y))
#endif

#ifndef VSCPMAX
 #define VSCPMAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
#endif


#define MAX_ITEMS_RECEIVE_QUEUE                 1021
#define MAX_ITEMS_SEND_QUEUE                    1021
#define MAX_ITEMS_CLIENT_RECEIVE_QUEUE          8192

// VSCP daemon defines from vscp.h
#define VSCP_MAX_CLIENTS                        4096    // abs. max. is 0xffff
#define VSCP_MAX_DEVICES                        1024    // abs. max. is 0xffff

///////////////////////////////////////////////////////////////////////
// vscp_md5
//
// Replacement for abandon Cesanta version
//

//void vscp_md5( char *digest, const unsigned char *buf, size_t len ) ;


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
    CControlObject( void );

    /*!
        Destructor
     */
    virtual ~CControlObject( void );

    /*!
        Generate a random session key from a string key
        @param pKey Null terminated string key (max 255 characters)
        @param pSid Pointer to 33 byte sid that will receive sid
     */
    bool generateSessionId( const char *pKey, char *pSid );

    /*!
        Get server capabilities (64-bit array)
        @param pCapability Pointer to 64 bit capabilities array
        @return True on success.
     */
    bool getVscpCapabilities( uint8_t *pCapability );

    /*!
      Write log message -
    */
    void logMsg( const wxString& wxstr,
                    const uint8_t level = DAEMON_LOGMSG_NORMAL,
                    const uint8_t nType = DAEMON_LOGTYPE_GENERAL );

    /*!
     * Get records from log database. The variable vscp.log.database.sql
     * contains the sql expression (without "select * from log")
     * @param sql Search expression
     * @param strResult The search result will be returned in this string.
     * @return True on success.
     */
    bool searchLogDB( const char * sql, wxString& strResult );

    /*!
        General initialisation
     */
    bool init( wxString& strcfgfile, wxString& rootFolder );

    /*!
        Clean up used resources
     */
    bool cleanup( void );

    /*!
        The main worker thread
     */
    bool run( void );

    /*!
        Start worker threads for devices
        @return true on success
     */
    bool startDeviceWorkerThreads( void );

    /*!
        Stop worker threads for devices
        @return true on success
     */
    bool stopDeviceWorkerThreads( void );

    /*!
        Starting daemon worker thread
        @return true on success
     */
    bool startDaemonWorkerThread( void );

    /*!
        Stop daemon worker thread
        @return true on success
     */
    bool stopDaemonWorkerThread( void );


    /*!
        Starting TCP/IP worker thread
        @return true on success
     */
    bool startTcpWorkerThread( void );

    /*!
        Stop the TCP/IP worker thread
        @return true on success
     */
    bool stopTcpWorkerThread( void );

    /*!
        Start the UDP worker thread
    */
    bool startUDPWorkerThread( void );

    /*!
        Stop the UDP Workerthread
    */
    bool stopUDPWorkerThread( void );

    /*!
        Start the Multicast worker threads
    */
    bool startMulticastWorkerThreads( void );

    /*!
        Stop the Multicast Workerthreads
    */
    bool stopMulticastWorkerThreads( void );


    /*!
        Starting Client worker thread
        @return true on success
     */
    bool startClientWorkerThread( void );

    /*!
        Stop Client worker thread
        @return true on success
     */
    bool stopClientWorkerThread( void );

    /*!
        Add a new client to the clinet list

        @param Pointer to client that should be added.
        @param Normally not used but can be used to set a special
        client id.
     */
    void addClient( CClientItem *pClientItem, uint32_t id = 0 );

    /*!
        Add a known node
        @param guid Real GUID for node
        @param name Symbolic name for node.
    */
    void addKnownNode( cguid& guid, cguid& ifguid, wxString& name );

    /*!
        Remove a new client from the client list

        @param pClientItem Pointer to client that should be added.
     */
    void removeClient( CClientItem *pClientItem );

    /*!
        Get device address for primary ehernet adapter

        @param guid class
     */
    bool getMacAddress( cguid& guid );


    /*!
        Get the first IP address computer is known under

        @param pGUID Pointer to GUID class
     */
    bool getIPAddress( cguid& guid );

    /*!
        Read XML configuration GENEAL data
        @param strcfgfile path to configuration file.
        @return Returns true on success false on failure.
     */
    bool readXMLConfigurationGeneral( wxString& strcfgfile );

    /*!
        Read configuration data
        @param strcfgfile path to configuration file.
        @return Returns true on success false on failure.
     */
    bool readConfigurationXML( wxString& strcfgfile );

    /*!
         Save configuration data
     */
    bool saveConfiguration( void );


    /*!
        send level II message to all clients
     */
    void sendEventToClient( CClientItem *pClientItem, vscpEvent *pEvent );

    /*!
        Send Level II event to all clients with exception
     */
    void sendEventAllClients( vscpEvent *pEvent, uint32_t excludeID = 0 );

    /*!
     * Send event
     * @param pClientItem Client that send the event.
     * @param pEvent Event to send
     * @return True on success false on failure.
     */
    bool sendEvent( CClientItem *pClientItem, vscpEvent *peventToSend );


    /*!
        Get clientmap index from a client id
     */
    uint32_t getClientMapFromId( uint32_t clid );

    /*!
        Get a client id from a client map index
     */
    uint32_t getClientMapFromIndex( uint32_t idx );

    /*!
        Add a client id to the clientmap
     */
    uint32_t addIdToClientMap( uint32_t clid );

    /*!
        Remove a client id to the clientmap
        @return True on success
     */
    bool removeIdFromClientMap( uint32_t clid );


    /*!
        Get the VSCP TCP/IP thread thread
        @return Pointer to the VSCP TCP/IP thread thread
    */
    VSCPClientThread *getTCPIPServer( void ) { return m_pVSCPClientThread; };

    /*!
     * Read configuration data from database.
     * The configuration database record is read after the XML file has
     * been read and will replace duplicate values, if any.
     * @return true on success
     */
     bool readConfigurationDB( void );

     /*!
      * Write configuration datapait to configuration database.
      *
      * @return true on success
      */
     bool addConfigurationValueToDatabase( const char *pName,
                                             const char *pValue );

     /*!
      * Create configuration table
      * @return true on success
      */
     bool doCreateConfigurationTable( void );


     /*
      * Update field in settings table
      */
     bool updateConfigurationRecordItem( const wxString& strName,
                                            const wxString& strValue );

     /*!
      * Read in UDP nodes from the database
      */
     bool readUdpNodes( void );

     /*!
      * Read in multicast channels from the database
      */
     bool readMulticastChannels( void );

    /*!
     * Create log database
     * @return true on success
     */
    bool doCreateLogTable( void );

    /*!
     * Create udpnode database
     * @return true on success
     */
    bool doCreateUdpNodeTable( void );

    /*!
     * Create multicast database
     * @return true on success
     */
    bool doCreateMulticastTable( void );

    /*!
     * Create user table
     */
    bool doCreateUserTable( void );

    /*!
     * Create driver table
     */
    bool doCreateDriverTable( void );

    /*!
     * Create guid table
     */
    bool doCreateGuidTable( void );

    /*!
     * Create location table
     */
    bool doCreateLocationTable( void );

    /*!
     * Create mdf table
     */
    bool doCreateMdfCacheTable( void );

    /*!
     * Create simpleui table
     */
    bool doCreateSimpleUiTable( void );

    /*!
     * Create simpleui item table
     */
    bool doCreateSimpleUiItemTable( void );

    /*!
     * Create zone table
     */
    bool doCreateZoneTable( void );

    /*!
     * Create subzone table
     */
    bool doCreateSubZoneTable( void );

    /*!
     * Create userdef table
     */
    bool doCreateUserdefTableTable( void );

    /*!
     * Get current number of records in log database
     */
    long getCountRecordsLogDB( void );


    /*!
     * Get the system key
     *
     * @param pKey Buffer that will get 32-byte key. Can be NULL in which
     *              case the key is not copied to the param.
     * @return Pointer to the 32 byte key
     */
    uint8_t *getSystemKey( uint8_t *pKey );

    /*!
     * Get MD5 of system key (vscptoken)
     *
     * @param Reference to string that will receive the MD5 of the key.
     */
    void getSystemKeyMD5( wxString &strKey );

    /*!
     * Create the folder structure that the VSCP daemon is expecting
     * http://www.vscp.org/docs/vscpd/doku.php?id=files_and_directory_structure
     */
    bool createFolderStructure( void );

public:

#ifdef BUILD_VSCPD_SERVICE
    HANDLE m_hEvntSource;
#endif

    wxLocale m_locale;

    // In the configuration database configurations are stored in records.
    // Normally record = 1 (default) is used )
    uint16_t m_nConfiguration;

    // This is the root folder for the VSCP daemon, it will look for
    // the configuration database here
    wxString m_rootFolder;

    /*!
        true if we should quit
     */
    bool m_bQuit;

    /*!
     * Debug flags
     * See vscp_debug.h for possible flags.
     */
    uint32_t m_debugFlags1;
    uint32_t m_debugFlags2;
    uint32_t m_debugFlags3;
    uint32_t m_debugFlags4;
    uint32_t m_debugFlags5;
    uint32_t m_debugFlags6;
    uint32_t m_debugFlags7;
    uint32_t m_debugFlags8;

    //**************************************************************************
    //                                 Security
    //**************************************************************************

    // Password is MD5 hash over "username:domain:password"
    wxString m_admin_user;          // Defaults to "admin"
    wxString m_admin_password;
    // Default password salt;key
    // E2D453EF99FB3FCD19E67876554A8C27;A4A86F7D7E119BA3F0CD06881E371B989B33B6D606A863B633EF529D64544F8E
    wxString m_admin_allowfrom;     // Remotes allowed to connect from as admin. Defaults to "*"
    wxString m_vscptoken;
    // A4A86F7D7E119BA3F0CD06881E371B989B33B6D606A863B633EF529D64544F8E
    // { 0xA4,0xA8,0x6F,0x7D,0x7E,0x11,0x9B,0xA3,0xF0,0xCD,0x06,0x88,0x1E,0x37,0x1B,0x98,
    //   0x9B,0x33,0xB6,0xD6,0x06,0xA8,0x63,0xB6,0x33,0xEF,0x52,0x9D,0x64,0x54,0x4F,0x8E };
    uint8_t m_systemKey[32];

    /*!
        User to run as for Unix
        if not ""
    */
    wxString m_runAsUser;

    /*!
        Maximum number of items in receive queue for clients
     */
    uint32_t m_maxItemsInClientReceiveQueue;

    /*!
        Name of this server
     */
    wxString m_strServerName;

    /*!
        Server GUID
        This is the GUID for the server
    */
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


    //**************************************************************************
    //                             Logging
    //**************************************************************************

    wxMutex m_mutexLogWrite;

    /*!
        Log Level
     */
    uint8_t m_logLevel;


    /*!
        Log to syslog
     */
    bool m_bLogToSysLog;



    //**************************************************************************
    //                           Communication
    //**************************************************************************



    /////////////////////////////////////////////////////////
    //                       TCP/IP
    /////////////////////////////////////////////////////////

    /// net_skeleton structure
    struct mg_mgr m_mgrTcpIpServer;

    bool m_enableTcpip;

    /// Interface(s) used for TCP/IP connection
    wxString m_strTcpInterfaceAddress;






    /////////////////////////////////////////////////////////
    //                        UDP
    /////////////////////////////////////////////////////////


    /*!
     *  Info fro UDP subsystem
     */
    udpServerInfo m_udpInfo;

    /*!
     *  Mutex that protect the UDP info structure
     */
    wxMutex m_mutexUDPInfo;


    /////////////////////////////////////////////////////////
    //                      MULTICAST
    /////////////////////////////////////////////////////////

    // CHANNEL

    /*!
        Multicast channel interface
    */

    multicastInfo m_multicastInfo;

    /*!
     *  Mutex that protect the Multicast info structure
     */
    wxMutex m_mutexMulticastInfo;

    // ANNOUNCE

    /*!
        Enable Multicast announce interface
    */
    bool m_bMulticastAnounce;

    /// Interface(s) used for multicast announce
    wxString m_strMulticastAnnounceAddress;

    // ttl for multicast announce
    uint8_t m_ttlMultiCastAnnounce;



    //*****************************************************
    //               webserver interface
    //*****************************************************

    // Context for web server
    struct web_context *m_web_ctx;

    // Enable webserver
    bool m_web_bEnable;

    // See http://www.vscp.org/docs/vscpd/doku.php?id=configuring_the_vscp_daemon#webserver
    wxString m_web_document_root;
    wxString m_web_listening_ports;
    wxString m_web_index_files;
    wxString m_web_authentication_domain;
    bool m_enable_auth_domain_check;
    wxString m_web_ssl_certificate;
    wxString m_web_ssl_certificate_chain;
    bool m_web_ssl_verify_peer;
    wxString m_web_ssl_ca_path;
    wxString m_web_ssl_ca_file;
    uint16_t m_web_ssl_verify_depth;
    bool m_web_ssl_default_verify_paths;
    wxString m_web_ssl_cipher_list;
    uint8_t m_web_ssl_protocol_version;
    bool m_web_ssl_short_trust;
    wxString m_web_cgi_interpreter;
    wxString m_web_cgi_patterns;
    wxString m_web_cgi_environment;
    wxString m_web_protect_uri;
    wxString m_web_trottle;
    bool m_web_enable_directory_listing;
    bool m_web_enable_keep_alive;
    long m_web_keep_alive_timeout_ms;
    wxString m_web_access_control_list;
    wxString m_web_extra_mime_types;
    int m_web_num_threads;
    wxString m_web_run_as_user;
    wxString m_web_url_rewrite_patterns;
    wxString m_web_hide_file_patterns;
    long m_web_request_timeout_ms;
    long m_web_linger_timeout_ms;    // Set negative to not set
    bool m_web_decode_url;
    wxString m_web_global_auth_file;
    wxString m_web_per_directory_auth_file;
    wxString m_web_ssi_patterns;
    wxString m_web_access_control_allow_origin;
    wxString m_web_access_control_allow_methods;
    wxString m_web_access_control_allow_headers;
    wxString m_web_error_pages;
    long m_web_tcp_nodelay;
    long m_web_static_file_max_age;
    long m_web_strict_transport_security_max_age;
    bool m_web_allow_sendfile_call;
    wxString m_web_additional_header;
    long m_web_max_request_size;
    bool m_web_allow_index_script_resource;
    wxString m_web_duktape_script_patterns;
    wxString m_web_lua_preload_file;
    wxString m_web_lua_script_patterns;
    wxString m_web_lua_server_page_patterns;
    wxString m_web_lua_websocket_patterns;
    wxString m_web_lua_background_script;
    wxString m_web_lua_background_script_params;

    // Protects the web session object
    wxMutex m_websrvSessionMutex;

    // Linked list of all active sessions. (webserv.h)
    WEBSRVSESSIONLIST m_web_sessions;



    // Protects the REST session object
    wxMutex m_restSessionMutex;

    // Session structure for REST API
    RESTSESSIONLIST m_rest_sessions;


    //**************************************************************************
    //                              WEBSOCKETS
    //**************************************************************************

    bool m_bWebsocketsEnable;   // Enable web socket functionality
    wxString m_websocket_document_root;
    long m_websocket_timeout_ms;

    // * * Websockets * *

    // Protects the websocket session object
    wxMutex m_websocketSessionMutex;

    // List of active websocket sessions
    WEBSOCKETSESSIONLIST m_websocketSessions;


    //**************************************************************************
    //                               MQTT
    //**************************************************************************

    bool m_enableMqttBroker;
    wxString m_strMQTTBrokerInterfaceAddress;

    //**************************************************************************
    //                               CoAP
    //**************************************************************************

    bool m_enableCOAP;
    wxString m_strCoapInterfaceAddress;

    //**************************************************************************
    //                               VARIABLES
    //**************************************************************************


    /// Hash table for variables
    CVariableStorage m_variables;

    /// Mutex to protect variables
    wxMutex m_variableMutex;



    //**************************************************************************
    //                                SQLite3
    //**************************************************************************



    //*****************************************************
    //                    Databases
    //*****************************************************

    wxFileName m_path_db_vscp_daemon;   // Path to the VSCP daemon database
    sqlite3 *m_db_vscp_daemon;
    /// Mutex to protect variables
    wxMutex m_db_vscp_configMutex;      // Mutex for the configuration table

    wxFileName m_path_db_vscp_data;     // Path to the VSCP data database
    sqlite3 *m_db_vscp_data;

    wxFileName m_path_db_vscp_log;      // Path to the VSCP log database
    sqlite3 *m_db_vscp_log;


    /*!
        Event source for NT event reporting
     */
#ifdef BUILD_VSCPD_SERVICE
    HANDLE m_hEventSource;
#endif


    /*!
        Mutex for client queue
     */
    wxMutex m_wxClientMutex;


    /*!
        Mutex for device queue
     */
    wxMutex m_wxDeviceMutex;



    /// Daemon Decision Matrix Object
    CDM m_dm;


    /// Automation Object
    CVSCPAutomation m_automation;


    /*!
        Username for level II drivers
     */
    wxString m_driverUsername;

    /*!
        Password for Level II drivers
     */
    wxString m_driverPassword;



    //**************************************************************************
    //                                Lists
    //**************************************************************************
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
        The list of users
     */
    CUserList m_userList;
    wxMutex m_mutexUserList;

    /*!
        This is a list with defined tables
    */
    CUserTableObjList m_userTableObjects;
    wxMutex m_mutexUserTables;


    /*!
        This is the list with known nodes in the system
    */
    CKnownNodes m_knownNodes;
    wxMutex m_mutexKnownNodes;

    // *************************************************************************


    /*!
        Send queue

        This is the send queue for all clients attached to the system. A client
        place events here and the system distribute it to all other clients.
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

    //**************************************************************************
    //                          Threads
    //**************************************************************************

    /*!
        controlobject device thread
     */
    clientMsgWorkerThread *m_pclientMsgWorkerThread;
    wxMutex m_mutexclientMsgWorkerThread;

    /*!
        The server thread for the TCP connection interface
     */
    VSCPClientThread *m_pVSCPClientThread;
    wxMutex m_mutexTcpClientListenThread;

    /*!
        The server thread for the VSCP daemon
     */
    daemonVSCPThread *m_pdaemonVSCPThread;
    wxMutex m_mutexdaemonVSCPThread;

    /*!
        The server thread for the VSCP daemon
     */
    VSCPUDPClientThread *m_pVSCPClientUDPThread;
    wxMutex m_mutexVSCPClientnUDPThread;

    /*!
        UDP Worker threads
     */
    VSCPUDPClientThread m_pudpClientThread;
    wxMutex m_mutexudpClientThread;


};




#endif // !defined(AFX_CONTROLOBJECT_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_)
