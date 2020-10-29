// ControlObject.h: interface for the CControlObject class.
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

#if !defined(CONTROLOBJECT_H__INCLUDED_)
#define CONTROLOBJECT_H__INCLUDED_



#include <automation.h>
#include <clientlist.h>
#include <devicelist.h>
#include <interfacelist.h>
#include <tcpipsrv.h>
#include <udpsrv.h>
#include <mqtt.h>
#include <userlist.h>
#include <vscp.h>
#include <websocket.h>
#include <websrv.h>

#include <set>

// Forward declarations
class TCPListenThread;

// TTL     Scope
// ----------------------------------------------------------------------
// 0       Restricted to the same host.Won't be output by any interface.
// 1       Restricted to the same subnet.Won't be forwarded by a router.
// <32     Restricted to the same site, organization or department.
// <64     Restricted to the same region.
// <128    Restricted to the same continent.
// <255    Unrestricted in scope.Global.
#define IP_MULTICAST_DEFAULT_TTL 1

// Needed on Linux
#ifndef VSCPMIN
#define VSCPMIN(X, Y) ((X) < (Y) ? (X) : (Y))
#endif

#ifndef VSCPMAX
#define VSCPMAX(a, b)                                                          \
    ({                                                                         \
        __typeof__(a) _a = (a);                                                \
        __typeof__(b) _b = (b);                                                \
        _a > _b ? _a : _b;                                                     \
    })
#endif

#define MAX_ITEMS_RECEIVE_QUEUE        1021
#define MAX_ITEMS_SEND_QUEUE           1021
#define MAX_ITEMS_CLIENT_RECEIVE_QUEUE 8192

// VSCP daemon defines from vscp.h
#define VSCP_MAX_CLIENTS 4096 // abs. max. is 0xffff
#define VSCP_MAX_DEVICES 1024 // abs. max. is 0xffff

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
        Generate a random session key from a string key
        @param pKey Null terminated string key (max 255 characters)
        @param pSid Pointer to 33 byte sid that will receive sid
     */
    bool generateSessionId(const char* pKey, char* pSid);

    /*!
        Get server capabilities (64-bit array)
        @param pCapability Pointer to 64 bit capabilities array
        @return True on success.
     */
    bool getVscpCapabilities(uint8_t* pCapability);

    /*!
        General initialisation
     */
    bool init(std::string& strcfgfile, std::string& rootFolder);

    /*!
        Clean up used resources
     */
    bool cleanup(void);

    /*!
        The main worker thread
     */
    bool run(void);

    /*!
        Send automation events
        @param pClientItem Pointer to client item that want to send
        automation events
        @return true on success
    */
    bool automation(CClientItem* pClientItem);

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
    bool startTcpipSrvThread(void);

    /*!
        Stop the TCP/IP worker thread
        @return true on success
     */
    bool stopTcpipSrvThread(void);

    /*!
        Start the UDP worker thread
    */
    bool startUDPSrvThreads(void);

    /*!
        Stop the UDP Workerthread
    */
    bool stopUDPSrvThreads(void);

    /*!
        Start the MQTT worker thread
    */
    bool startMQTTClientThreads(void);

    /*!
        Stop the MQTT Workerthread
    */
    bool stopMQTTClientThreads(void);

    /*!
        Start the Multicast worker threads
    */
    bool startMulticastWorkerThreads(void);

    /*!
        Stop the Multicast Workerthreads
    */
    bool stopMulticastWorkerThreads(void);

    /*!
        Starting Client worker thread
        @return true on success
     */
    bool startClientMsgWorkerThread(void);

    /*!
        Stop Client worker thread
        @return true on success
     */
    bool stopClientMsgWorkerThread(void);

    /*!
        Add a new client to the client list

        @param Pointer to client that should be added.
        @param Normally not used but can be used to set a special
        client id.
        @return True on success.
    */
    bool addClient(CClientItem* pClientItem, uint32_t id = 0);

    /*!
        Add a new client to the client list using GUID. 

        This add client method is for drivers that specify a
        full GUID (two lsb nilled).

        @param Pointer to client that should be added.
        @param guid The guid that is used for the client. Two least
        significant bytes will be set to zero.
        @return True on success.
     */
    bool addClient(CClientItem* pClientItem, cguid& guid);

    /*!
        Add a known node
        @param guid Real GUID for node
        @param name Symbolic name for node.
    */
    void addKnownNode(cguid& guid, cguid& ifguid, std::string& name);

    /*!
        Remove a new client from the client list

        @param pClientItem Pointer to client that should be added.
     */
    void removeClient(CClientItem* pClientItem);

    /*!
        Add one UDP client

        @param bEnable True to enable connection
        @param interface The remote interface of the clinet to connect to
        @param user User to connect as
        @param password Password for user
        @param filter Outgoing filter.
        @param encryption Encryption to use for frames. 0=none (default), 1=AES128, 2=AES192, 3=AES256
        @param setbroadcast Set to true to set broadcast bit (default=false).
    */
    void addUdpClient(bool bEnable, 
                        std::string& interface, 
                        std::string& user,
                        std::string& password,
                        vscpEventFilter& filter,
                        int encryption = VSCP_ENCRYPTION_NONE,
                        bool setbroadcast = false);

    /*!
        Add one MQTT client
    */                            
    void addMqttClient( std::string &interface,
			                std::string &user,
			                std::string &password,
			                vscpEventFilter &filter,
			                cguid &guid,
			                int qos,
			                bool bCleanSession,
			                bool bRetain,
			                uint32_t keepalive,
			                std::string &topicSubscribe,
			                std::string &topicPublish,
			                std::string &cafile,
			                std::string &capath,
			                std::string &certgile,
			                std::string &keyfile,
			                std::string &xpassword );

    /*!
        Get device address for primary ethernet adapter

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
    bool readConfiguration(const std::string& strcfgfile);

    /*!
        send level II message to all clients
        @param pClientItem Pointer to client object for client that should
                           receive the event
        @param pEvent Pointer to event that should be sent to client. Caller
                        must deallocate if needed.
        @return true on success
     */
    bool sendEventToClient(CClientItem* pClientItem, vscpEvent* pEvent);

    /*!
        Send Level II event to all clients with exception
        @param pEvent Pointer to event that should be sent. Caller must
                        must take care of deallocation.
        @param excludeID Client with this obid should not receive event.
        @return True on success
     */
    bool sendEventAllClients(vscpEvent* pEvent, uint32_t excludeID = 0);

    /*!
     * Send event
     * @param pClientItem Client that send the event.
     * @param pEvent Event to send. !!! pEventToSend must be
     *               deallocated by sender !!!
     * @return True on success false on failure.
     */
    bool sendEvent(CClientItem* pClientItem, vscpEvent* peventToSend);

    /*!
     * Send event
     * @param pClientItem Client that send the event.
     * @param pex Eventex to send.
     * @return True on success false on failure.
     */
    bool sendEvent(CClientItem* pClientItem, vscpEventEx* pex);

    /*!
     * Check if a driver name is free to us
     *
     * @param drvname Name of driver to check.
     * @return true if 'drvname' is not used
     */
    bool checkIfDriverNameFreeToUse(std::string& drvname)
    {
        return (m_driverNameSet.find("drvname") == m_driverNameSet.end());
    }

    /*!
     * Get the system key
     *
     * @param pKey Buffer that will get 32-byte key. Can be NULL in which
     *              case the key is not copied to the param.
     * @return Pointer to the 32 byte key
     */
    uint8_t* getSystemKey(uint8_t* pKey);

    /*!
     * Get MD5 of system key (vscptoken)
     *
     * @param Reference to string that will receive the MD5 of the key.
     */
    void getSystemKeyMD5(std::string& strKey);

    /*!
     * Create the folder structure that the VSCP daemon is expecting
     * https://www.vscp.org/docs/vscpd/doku.php?id=files_and_directory_structure
     */
    bool createFolderStructure(void);

  public:
    // This is the root folder for the VSCP daemon, it will look for
    // the configuration database here
    std::string m_rootFolder;

    // Set to true if we should quit application
    bool m_bQuit;

    // Set to true of the clientWorkerThread should terminate
    bool m_bQuit_clientMsgWorkerThread;

    /*!
     * Debug flags
     * See vscp_debug.h for possible flags.
     * Set to point to m_gdebugArray in startup
     */
    uint32_t* m_debugFlags;

    //**************************************************************************
    //                                 Security
    //**************************************************************************

    // Password is MD5 hash over "username:domain:password"
    std::string m_admin_user; // Defaults to "admin"
    std::string m_admin_password;
    // Default password salt;key
    // E2D453EF99FB3FCD19E67876554A8C27;A4A86F7D7E119BA3F0CD06881E371B989B33B6D606A863B633EF529D64544F8E
    std::string m_admin_allowfrom; // Remotes allowed to connect from as admin.
                                   // Defaults to ""
    std::string m_vscptoken;
    // A4A86F7D7E119BA3F0CD06881E371B989B33B6D606A863B633EF529D64544F8E
    // {
    // 0xA4,0xA8,0x6F,0x7D,0x7E,0x11,0x9B,0xA3,0xF0,0xCD,0x06,0x88,0x1E,0x37,0x1B,0x98,
    //   0x9B,0x33,0xB6,0xD6,0x06,0xA8,0x63,0xB6,0x33,0xEF,0x52,0x9D,0x64,0x54,0x4F,0x8E
    //   };
    uint8_t m_systemKey[32];

    /*!
        User to run as for Unix
        if not ""
    */
    std::string m_runAsUser;

    /*!
        Maximum number of items in receive queue for clients (ClientBufferSize)
     */
    uint32_t m_maxItemsInClientReceiveQueue;

    /*!
        Name of this server
     */
    std::string m_strServerName;

    /*!
        Server GUID
        This is the GUID for the server
    */
    cguid m_guid;

    //**************************************************************************
    //                            Communication
    //**************************************************************************

    /////////////////////////////////////////////////////////
    //                      TCP/IP server
    /////////////////////////////////////////////////////////

    // Server will be started if set to true (by configuration (db/xml)
    bool m_enableTcpip;

    // Enable encryption on tcp/ip interface if enabled.
    // 0 = Disabled
    // 1 = AES-128
    // 2 = AES-192
    // 3 = AES-256
    uint8_t m_encryptionTcpip;

    // Interface used for TCP/IP connection  (only one)
    std::string m_strTcpInterfaceAddress;

    // Data object for the tcp/ip Listen thread
    tcpipListenThreadObj* m_ptcpipSrvObject;

    // Listen thread for tcp/ip connections
    pthread_t m_tcpipListenThread;

    // tcp/ip SSL settings
    std::string m_tcpip_ssl_certificate;
    std::string m_tcpip_ssl_certificate_chain;
    uint8_t m_tcpip_ssl_verify_peer; // no=0, optional=1, yes=2
    std::string m_tcpip_ssl_ca_path;
    std::string m_tcpip_ssl_ca_file;
    uint8_t m_tcpip_ssl_verify_depth;
    bool m_tcpip_ssl_default_verify_paths;
    std::string m_tcpip_ssl_cipher_list;
    uint8_t m_tcpip_ssl_protocol_version;
    bool m_tcpip_ssl_short_trust;

    //*****************************************************
    //                 UDP interface
    //*****************************************************

    bool m_enableUdp;

    // UDP server functionality
    udpSrvObj m_udpsrv;

    // Protect UDP remote list
    pthread_mutex_t m_mutexUDPRemotes;

    // Worker thread for UDP server
    pthread_t m_udpSrvWorkerThread;

    // List containing remote receiving UDP clients
    std::deque<udpRemoteClient *>m_udpremotes;


    //*****************************************************
    //                MQTT client interface
    //*****************************************************

    bool m_enableMqtt;

    // Protect MQTT client list
    pthread_mutex_t m_mutexMqttClients;

    // List containing MQTT clients
    std::deque<vscpMqttObj *>m_mqttClients;

    //*****************************************************
    //               webserver interface
    //*****************************************************

    // Context for web server
    struct mg_context* m_web_ctx;

    // Enable webserver
    bool m_web_bEnable;

    // Enable web admin interface (general section in conf)
    bool m_enableWebAdminIf;

    // See
    // https://www.vscp.org/docs/vscpd/doku.php?id=configuring_the_vscp_daemon#webserver
    std::string m_web_document_root;
    std::string m_web_listening_ports;
    std::string m_web_index_files;
    std::string m_web_authentication_domain;
    bool m_enable_auth_domain_check;
    std::string m_web_ssl_certificate;
    std::string m_web_ssl_certificate_chain;
    bool m_web_ssl_verify_peer;
    std::string m_web_ssl_ca_path;
    std::string m_web_ssl_ca_file;
    uint16_t m_web_ssl_verify_depth;
    bool m_web_ssl_default_verify_paths;
    std::string m_web_ssl_cipher_list;
    uint8_t m_web_ssl_protocol_version;
    bool m_web_ssl_short_trust;
    std::string m_web_cgi_interpreter;
    std::string m_web_cgi_patterns;
    std::string m_web_cgi_environment;
    std::string m_web_protect_uri;
    std::string m_web_trottle;
    bool m_web_enable_directory_listing;
    bool m_web_enable_keep_alive;
    long m_web_keep_alive_timeout_ms;
    std::string m_web_access_control_list;
    std::string m_web_extra_mime_types;
    int m_web_num_threads;
    std::string m_web_url_rewrite_patterns;
    std::string m_web_hide_file_patterns;
    long m_web_request_timeout_ms;
    long m_web_linger_timeout_ms; // Set negative to not set
    bool m_web_decode_url;
    std::string m_web_global_auth_file;
    std::string m_web_per_directory_auth_file;
    std::string m_web_ssi_patterns;
    std::string m_web_access_control_allow_origin;
    std::string m_web_access_control_allow_methods;
    std::string m_web_access_control_allow_headers;
    std::string m_web_error_pages;
    long m_web_tcp_nodelay;
    std::string m_web_static_file_cache_control;
    long m_web_static_file_max_age;
    long m_web_strict_transport_security_max_age;
    bool m_web_allow_sendfile_call;
    std::string m_web_additional_header;
    long m_web_max_request_size;
    bool m_web_allow_index_script_resource;
    std::string m_web_duktape_script_patterns;
    std::string m_web_lua_preload_file;
    std::string m_web_lua_script_patterns;
    std::string m_web_lua_server_page_patterns;
    std::string m_web_lua_websocket_patterns;
    std::string m_web_lua_background_script;
    std::string m_web_lua_background_script_params;

    // Protects the web session object
    pthread_mutex_t m_mutex_websrvSession;

    // Linked list of all active sessions. (websrv.h)
    std::list<struct websrv_session*> m_web_sessions;

    //**************************************************************************
    //                              REST
    //**************************************************************************

    // Protects the REST session object
    pthread_mutex_t m_mutex_restSession;

    // Session structure for REST API
    std::list<struct restsrv_session*> m_rest_sessions;

    // Enable REST API
    bool m_bEnableRestApi;

    //**************************************************************************
    //                              WEBSOCKETS
    //**************************************************************************

    bool m_bWebsocketsEnable; // Enable web socket functionality
    std::string m_websocket_document_root;
    long m_websocket_timeout_ms;
    bool bEnable_websocket_ping_pong;
    std::string lua_websocket_pattern;

    // * * Websockets * *

    // Protects the websocket session object
    pthread_mutex_t m_mutex_websocketSession;

    // List of active websocket sessions
    std::list<websock_session*> m_websocketSessions;

    //**************************************************************************
    //                                 DRIVERS
    //*************************************************************************

    // The list with available devices.
    CDeviceList m_deviceList;
    pthread_mutex_t m_mutex_DeviceList;

    // This set holds driver names.
    // Returns true for an active driver
    // A driver can only be loaded if it have an unique name.

    std::set<std::string> m_driverNameSet;

    std::map<std::string, CDeviceItem> m_driverNameDeviceMap;

    // Mutex for device queue
    pthread_mutex_t m_mutex_deviceList;

    // Automation Object
    CAutomation m_automation;

    // Username for level III drivers
    std::string m_driverUsername; // TODO remove

    // Password for Level III drivers
    std::string m_driverPassword; // TODO remove

    //**************************************************************************
    //                                CLIENTS
    //**************************************************************************

    // The list with active clients. (protecting mutex in object)
    CClientList m_clientList;

    // Mutex for client queue
    pthread_mutex_t m_mutex_clientList;

    // The list of users
    CUserList m_userList; // deque
    pthread_mutex_t m_mutex_UserList;

    // *************************************************************************

    /*!
        Send queue

        This is the send queue for all clients attached to the system. A client
        place events here and the system distribute it to all other clients.
     */
    std::list<vscpEvent*> m_clientOutputQueue;

    /*!
       Event object to indicate that there is an event in the client output
       queue.
     */
    sem_t m_semClientOutputQueue;

    /*!
        Mutex for Level II message send queue
     */
    pthread_mutex_t m_mutex_ClientOutputQueue;

    /*!
        Semaphore that is signaled when workerthread
        have send an incoming event to all clients
    */
    sem_t m_semSentToAllClients;

    // *************************************************************************

  private:
    //**************************************************************************
    //                          Threads
    //**************************************************************************

    /*!
        controlobject device thread
     */
    pthread_t m_clientMsgWorkerThread;

    /*!
        The server thread for the VSCP daemon
     */
    // daemonWorkerObj *m_pdaemonWorkerObj;
    // pthread_t m_pdaemonWorkerThread;
};

#endif // !defined(CONTROLOBJECT_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_)
