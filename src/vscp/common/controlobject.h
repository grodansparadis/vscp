// ControlObject.h: interface for the CControlObject class.
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

#if !defined(CONTROLOBJECT_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_)
#define CONTROLOBJECT_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_

#ifdef WIN32
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#endif

// NS_ENABLE_SSL        // Enable webserver SSL functionality

#include "wx/wx.h"
#include <wx/thread.h>
#include <wx/filename.h>

#include <mongoose.h>
#include <sqlite3.h>

#include <devicelist.h>
#include <clientlist.h>
#include <interfacelist.h>
#include <userlist.h>
#include <tables.h>
#include <vscp.h>
#include <vscpvariable.h>
#include <vscpautomation.h>
#include <tcpipclientthread.h>
#include <udpclientthread.h>
#include <vscpmqttbroker.h>
#include <vscpcoapserver.h>
#include <udpthread.h>
#include <daemonvscp.h>
#include <dm.h>
#include <knownnodes.h>
#include <vscp.h>

// Forward declarations
class VSCPWebServerThread;
class VSCPClientThread;
class CVSCPAutomation;

enum {
    DAEMON_LOGMSG_NONE = 0,
    DAEMON_LOGMSG_NORMAL,
    DAEMON_LOGMSG_DEBUG,
    //DAEMON_LOGMSG_INFO,       // Deprecated from version 12.4.5
    //DAEMON_LOGMSG_NOTICE,     // Deprecated from version 12.4.5
    //DAEMON_LOGMSG_WARNING,    // Deprecated from version 12.4.5
    //DAEMON_LOGMSG_ERROR,      // Deprecated from version 12.4.5
    //DAEMON_LOGMSG_CRITICAL,   // Deprecated from version 12.4.5
    //DAEMON_LOGMSG_ALERT,      // Deprecated from version 12.4.5
    //DAEMON_LOGMSG_EMERGENCY,  // Deprecated from version 12.4.5
};

enum {
    DAEMON_LOGTYPE_GENERAL = 0,
    DAEMON_LOGTYPE_SECURITY,
    DAEMON_LOGTYPE_ACCESS,
    DAEMON_LOGTYPE_DM
};

// Configuration database ordinals
enum {
    DAEMON_DB_ORDINAL_CONFIG_ID = 0,
    DAEMON_DB_ORDINAL_CONFIG_DBVERSION,
    DAEMON_DB_ORDINAL_CONFIG_LOGLEVEL,
    DAEMON_DB_ORDINAL_CONFIG_RUNASUSER,
    DAEMON_DB_ORDINAL_CONFIG_GUID,
    DAEMON_DB_ORDINAL_CONFIG_NAME,
    DAEMON_DB_ORDINAL_CONFIG_GENERALLOGFILE_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_GENERALLOGFILE_PATH,
    DAEMON_DB_ORDINAL_CONFIG_SECURITYLOGFILE_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_SECURITYLOGFILE_PATH,
    DAEMON_DB_ORDINAL_CONFIG_ACCESSLOGFILE_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_ACCESSLOGFILE_PATH,
    DAEMON_DB_ORDINAL_CONFIG_TCPIPINTERFACE_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_TCPIPINTERFACE_PORT,
    DAEMON_DB_ORDINAL_CONFIG_MULTICASTINTERFACE_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_MULTICASTINTERFACE_PORT,
    DAEMON_DB_ORDINAL_CONFIG_MULICASTINTERFACE_TTL,
    DAEMON_DB_ORDINAL_CONFIG_UDPSIMPLEINTERFACE_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_UDPSIMPLEINTERFACE_PORT,
    DAEMON_DB_ORDINAL_CONFIG_INTERNALDAEMONFUNCTIONALITY_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_DM_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_DM_PATH,
    DAEMON_DB_ORDINAL_CONFIG_DM_LOGGING_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_DM_LOGGING_PATH,
    DAEMON_DB_ORDINAL_CONFIG_DM_LOGGING_LEVEL,
    DAEMON_DB_ORDINAL_CONFIG_VARIABLES_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_VARIABLES_PATH,
    DAEMON_DB_ORDINAL_CONFIG_VSCPD_DEFAULTCLIENTBUFFERSIZE,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_DISABLEAUTHENTICATION,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_ROOTPATH,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_PORT,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_PATHCERT,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_AUTHDOMAIN,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_CGIINTERPRETER,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_CGIPATTERN,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_ENABLEDIRECTORYLISTINGS,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_HIDEFILEPATTERNS,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_INDEXFILES,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_EXTRAMIMETYPES,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_URLREWRITES,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_SSIPATTERN,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_RUNASUSER,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_PERDIRECTORYAUTHFILE,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_GLOBALAUTHFILE,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER__IPACL,
    DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_DAVDOCUMENTROOT,
    DAEMON_DB_ORDINAL_CONFIG_WEBSOCKET_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_WEBSOCKET_ENABLEAUTH,
    DAEMON_DB_ORDINAL_CONFIG_MQTTBROKER_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_MQTTBROKER_PORT,
    DAEMON_DB_ORDINAL_CONFIG_COAPSERVER_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_COAPSERVER_PORT,
    DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_ZONE,
    DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_SUBZONE,
    DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_LONGITUDE,
    DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_LATITUDE,
    DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_TIMEZONE,
    DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_SUNRISE_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_SUNSET_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_SUNSETTWILIGHT_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_SUNRISETWILIGHT_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_SEGMENTCONTROLLEREVENT_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_SEGMENTCONTROLLEREVENT_INTERVAL,
    DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_HEARTBEATEVENT_ENABLE,
    DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_HEARTBEATEVENT_INTERVAL
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
#define MAX_ITEMS_CLIENT_RECEIVE_QUEUE          8191
#define VSCP_TCPIPIF_DEFAULT_PORT               9598


WX_DECLARE_LIST(canalMsg, CanalMsgList);
WX_DECLARE_LIST(vscpEvent, VSCPEventList);
WX_DECLARE_LIST(vscpEventFilter, TRIGGERLIST);
WX_DECLARE_STRING_HASH_MAP( wxString, HashString );


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
      Write log message -
    */
    void logMsg(const wxString& wxstr,
                    const uint8_t level = DAEMON_LOGMSG_NORMAL,
                    const uint8_t nType = DAEMON_LOGTYPE_GENERAL );

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
        Start the UDP worker thread
    */
    bool startUDPWorkerThread(void);

    /*!
        Stop the UDP Workerthread
    */
    bool stopUDPWorkerThread(void);

    /*!
     *  Start MQTT broker
     */
    bool startMQTTBrokerThread(void);

    /*!
     * Stop MQTT broker
     */
    bool stopMQTTBrokerThread(void);

    /*!
     * Start CoAP server
     */
    bool startCoAPServerThread(void);

    /*!
     *  Stop CoAP server
     */
    bool stopCoAPServerThread(void);

    /*!
        Starting WebServer worker thread
        @return true on success
     */
    bool startWebServerThread(void);

    /*!
        Stop the WebServer worker thread
        @return true on success
     */
    bool stopWebServerThread(void);

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
        Add a known node
        @param guid Real GUID for node
        @param name Symbolic name for node.
    */
    void addKnownNode( cguid& guid, cguid& ifguid, wxString& name );

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
    bool getIPAddress( cguid& guid );

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
        Get a client id from a client map index
     */
    uint32_t getClientMapFromIndex(uint32_t idx);

    /*!
        Add a client id to the clientmap
     */
    uint32_t addIdToClientMap(uint32_t clid);

    /*!
        Remove a client id to the clientmap
        @return True on success
     */
    bool removeIdFromClientMap(uint32_t clid);

    /*!
        Get the webserver thread
        @return Pointer to the webserver thread
    */
    VSCPWebServerThread *getWebServer( void ) { return m_pwebServerThread; };

    /*!
        Get the VSCP TCP/IP thread thread
        @return Pointer to the VSCP TCP/IP thread thread
    */
    VSCPClientThread *getTCPIPServer( void ) { return m_pVSCPClientThread; };

    /*!
     * Add stock variables
    */
    void addStockVariables( void );

    /*!
     * Read configuration data from database.
     * The configuration database record is read after the XML file has
     * been read and will replace duplicate values, if any.
     * @return true on success
     */
     bool dbReadConfiguration( void );

public:

#ifdef BUILD_VSCPD_SERVICE
    HANDLE m_hEvntSource;
#endif

    wxLocale m_locale;

    /*!
        true if we should quit
     */
    bool m_bQuit;

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

    /////////////////////////////////////////////////////////
    //                      Logging
    /////////////////////////////////////////////////////////

    wxMutex m_mutexLogWrite;

    /*!
        Log Level
     */
    uint8_t m_logLevel;

    /*!
        Path to general log file
    */
    bool m_bLogGeneralEnable;   // Enable general logfile
    wxFileName m_logGeneralFileName;
    wxFile m_fileLogGeneral;

    /*!
        Path to security log file
    */
    bool m_bLogSecurityEnable; // Enable security logfile
    wxFileName m_logSecurityFileName;
    wxFile m_fileLogSecurity;

    /*!
        Path to access log file
    */
    bool m_bLogAccessEnable; // Enable security logfile
    wxFileName m_logAccessFileName;
    wxFile m_fileLogAccess;


    /////////////////////////////////////////////////////////
    //              Enable/disable switches
    /////////////////////////////////////////////////////////

    /*!
        Enable to log to database instead of separate files
     */
    bool m_bLogToDatabase;
    
    /*!
        Log to syslog
     */
    bool m_bLogToSysLog;
    
    /*!
        Enable control (TCP/IP) interface
     */
    bool m_bTCP;

    /*!
        Enable UDP interface
    */
    bool m_bUDP;

    /*!
        Enable MQTT broker
    */
    bool m_bMQTTBroker;

    /*!
        Enable CoAP server
    */
    bool m_bCoAPServer;

    /*!
        Multicast announce interface
    */
    bool m_bMulticastAnnounce;

    /*!
        Enable DM functionality
     */
    bool m_bDM;

    /*!
        Enable CANAL Driver functionality
     */
    bool m_bEnableLevel1Drivers;

    /*!
        Enable Level2 Driver functionality
    */
    bool m_bEnableLevel2Drivers;

    /*!
        Enable VSCP Daemon internal functionality (Disovery)
     */
    bool m_bVSCPDaemon;

    /*!
        Enable variable usage
     */
    bool m_bVariables;


    /////////////////////////////////////////////////////////
    //                  Communication
    /////////////////////////////////////////////////////////

    /// net_skeleton structure
    struct mg_mgr m_mgrTcpIpServer;

    /// Interface(s) used for TCP/IP connection
    wxString m_strTcpInterfaceAddress;

    /// Interface(s) used for multicast announce
    wxString m_strMulticastAnnounceAddress;

    // ttl for multicast announce
    uint8_t m_ttlMultiCastAnnounce;

    /// Interface(s) to use for UDP
    wxString m_strUDPInterfaceAddress;

    /// Interface(s) to use for MQTT broker
    wxString m_strMQTTBrokerInterfaceAddress;

    /// Interface(s) to use for CoAP Server
    wxString m_strCoAPServerInterfaceAddress;


    /////////////////////////////////////////////////////////
    //                     VARIABLES
    /////////////////////////////////////////////////////////


    /// Hash table for variables
    CVariableStorage m_VSCP_Variables;

    /// Mutex to protect variables
    wxMutex m_variableMutex;



    /////////////////////////////////////////////////////////
    //                     SQLite3
    /////////////////////////////////////////////////////////



    //*****************************************************
    //                    Databases
    //*****************************************************

    wxFileName m_path_db_vscp_daemon;   // Path to the VSCP daemon database
    sqlite3 *m_db_vscp_daemon;

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


    //*****************************************************
    //            websocket/webserver interface
    //*****************************************************

    // Enable/disable full webserver
    bool m_bWebServer;

    /*!
        If true web server security is disabled and checks
        will not be performed on user nor remote address.
    */
    bool m_bDisableSecurityWebServer;

    //struct mg_mgr *m_pwebserver;     // Was mg_server

    // Path to web root
    char m_pathWebRoot[ MAX_PATH_SIZE ];

    // Domain for webserver and other net services
    char m_authDomain[ MAX_PATH_SIZE ];

    // Path to SSL certificate
    char m_pathCert[ MAX_PATH_SIZE ];

    /// Extra mime types
    char m_extraMimeTypes[ MAX_PATH_SIZE ];

    /// Extra mime types on the form "extension1=type1,extension2=type2,..."
    char m_ssi_pattern[ MAX_PATH_SIZE ];

    // IP ACL. By default, NULL, meaning all IPs are allowed to connect
    char m_ip_acl[ MAX_PATH_SIZE ];

    // CGI interpreter to use
    char m_cgiInterpreter[ MAX_PATH_SIZE ];

    // CGI Pattern
    char m_cgiPattern[ MAX_PATH_SIZE ];

    // Enable directory listing "yes"/"no"
    char m_EnableDirectoryListings[ 5 ];

    // Hide file pattern
    char m_hideFilePatterns[ MAX_PATH_SIZE ];

    // DAV document root. If NULL, DAV requests are going to fail.
    char m_dav_document_root[ MAX_PATH_SIZE ];

    // Index files
    char m_indexFiles[ MAX_PATH_SIZE ];

    // URL rewrites
    char m_urlRewrites[ MAX_PATH_SIZE ];

    // Leave as NULL to disable authentication.
    // To enable directory protection with authentication, set this to ".htpasswd"
    // Then, creating ".htpasswd" file in any directory automatically protects
    // it with digest authentication.
    // Use `mongoose` web server binary, or `htdigest` Apache utility to
    // create/manipulate passwords file.
    // Make sure `auth_domain` is set to a valid domain name.
    char m_per_directory_auth_file[ MAX_PATH_SIZE ];

    // Leave as NULL to disable authentication.
    // Normally, only selected directories in the document root are protected.
    // If absolutely every access to the web server needs to be authenticated,
    // regardless of the URI, set this option to the path to the passwords file.
    // Format of that file is the same as ".htpasswd" file. Make sure that file
    // is located outside document root to prevent people fetching it.
    char m_global_auth_file[ MAX_PATH_SIZE ];

    // webserver port as port "8080" or address + port "127.0.0.1:8080"
    // If only port will bind to all interfaces,
    wxString m_portWebServer;   // defaults to "8080"

    // Run as user
    wxString m_runAsUserWeb;



    // * * Websockets * *

    // Enable disable web socket interface
    bool m_bWebSockets;

    // websocket authentication is needed  (if true)
    bool m_bAuthWebsockets;



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
        The list of users
     */
    CUserList m_userList;
    wxMutex m_mutexUserList;

    /*!
        This is a list with defined tables
    */
    listVSCPTables m_listTables;
    wxMutex m_mutexTableList;

    /*!
        This is the list with knwon nodes in the system
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
        Webserver
    */
    VSCPWebServerThread *m_pwebServerThread;
    wxMutex m_mutexwebServerThread;

    /*!
        UDP Worker threads
     */
    //UDPSendThread *m_pudpSendThread;
    //UDPReceiveThread *m_pudpReceiveThread;

    //wxMutex m_mutexudpSendThread;
    //wxMutex m_mutexudpReceiveThread;

    /*!
        The server thread for the MQTT Broker
     */
    VSCPMQTTBrokerThread *m_pMQTTBrookerThread;
    wxMutex m_mutexMQTTBrokerThread;

    /*!
        The server thread for the CoAP server
     */
    VSCPCoAPServerThread *m_pCoAPServerThread;
    wxMutex m_mutexCoAPServerThread;
};




#endif // !defined(AFX_CONTROLOBJECT_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_)
