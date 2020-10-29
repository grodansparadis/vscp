// ControlObject.cpp: m_path_db_vscp_logimplementation of the CControlObject
// class.
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
//

#define _POSIX

#include <controlobject.h>

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
#include <pwd.h>
#include <signal.h>
#include <stdarg.h>
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
#ifdef WITH_SYSTEMD
#include <systemd/sd-daemon.h>
#endif

#include <civetweb.h>

#include <algorithm>
#include <deque>
#include <list>
#include <map>
#include <set>
#include <string>

#include <web_css.h>
#include <web_js.h>
#include <web_template.h>

#include <expat.h>

#include <fastpbkdf2.h>
#include <vscp_aes.h>

#include <actioncodes.h>
#include <automation.h>
#include <canal_macro.h>
#include <configfile.h>
#include <crc.h>
#include <devicelist.h>
#include <devicethread.h>
#include <randpassword.h>
#include <remotevariablecodes.h>
#include <version.h>
#include <vscp.h>
#include <vscp_debug.h>
#include <vscpd_caps.h>
#include <vscpdb.h>
#include <vscphelper.h>
#include <vscpmd5.h>
#include <websocket.h>
#include <websrv.h>

#define UNUSED(x) (void)(x)
void
foo(const int i)
{
    UNUSED(i);
}

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#define XML_BUFF_SIZE 0xffff

// Prototypes
void
createFolderStuct(std::string& rootFolder); // from vscpd.cpp

void*
clientMsgWorkerThread(void* userdata); // this
void*
tcpipListenThread(void* pData); // tcpipsev.cpp



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CControlObject::CControlObject()
{
    // Open syslog
    openlog("vscpd", LOG_CONS, LOG_DAEMON);

    if (__VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG, "Starting the vscpd daemon");
    }

    m_bQuit = false; // true  for app termination
    m_bQuit_clientMsgWorkerThread =
      false; // true for clientWorkerThread termination

    if (-1 == sem_init(&m_semClientOutputQueue, 0, 0)) {
        syslog(LOG_ERR, "Unable to init m_semClientOutputQueue");
        return;
    }

    if (-1 == sem_init(&m_semSentToAllClients, 0, 0)) {
        syslog(LOG_ERR, "Unable to init m_semSentToAllClients");
        return;
    }

    if (0 != pthread_mutex_init(&m_mutex_ClientOutputQueue, NULL)) {
        syslog(LOG_ERR, "Unable to init m_mutex_ClientOutputQueue");
        return;
    }

    if (0 != pthread_mutex_init(&m_mutex_websrvSession, NULL)) {
        syslog(LOG_ERR, "Unable to init m_mutex_websrvSession");
        return;
    }

    if (0 != pthread_mutex_init(&m_mutex_restSession, NULL)) {
        syslog(LOG_ERR, "Unable to init m_mutex_restSession");
        return;
    }

    if (0 != pthread_mutex_init(&m_mutex_websocketSession, NULL)) {
        syslog(LOG_ERR, "Unable to init m_mutex_websocketSession");
        return;
    }

    if (0 != pthread_mutex_init(&m_mutex_DeviceList, NULL)) {
        syslog(LOG_ERR, "Unable to init m_mutex_DeviceList");
        return;
    }

    if (0 != pthread_mutex_init(&m_mutex_clientList, NULL)) {
        syslog(LOG_ERR, "Unable to init m_mutex_clientList");
        return;
    }

    if (0 != pthread_mutex_init(&m_mutex_UserList, NULL)) {
        syslog(LOG_ERR, "Unable to init m_mutex_UserList");
        return;
    }
    
    if (0 != pthread_mutex_init(&m_mutexUDPRemotes, NULL)) {
        syslog(LOG_ERR, "Unable to init m_mutexUDPRemotes");
        return;
    }

    m_rootFolder = "/var/lib/vscp/vscpd/";

    // Default admin user credentials
    m_admin_user     = "admin";
    m_admin_password = "450ADCE88F2FDBB20F3318B65E53CA4A;"
                       "06D3311CC2195E80BE4F8EB12931BFEB5C"
                       "630F6B154B2D644ABE29CEBDBFB545";
    m_admin_allowfrom = ""; // All access
    m_vscptoken       = "Carpe diem quam minimum credula postero";
    vscp_hexStr2ByteArray(m_systemKey,
                          32,
                          "A4A86F7D7E119BA3F0CD06881E371B989B"
                          "33B6D606A863B633EF529D64544F8E");

    m_automation.setControlObject(this);
    m_maxItemsInClientReceiveQueue = MAX_ITEMS_CLIENT_RECEIVE_QUEUE;

    // Nill the GUID
    m_guid.clear();

    // web admin interface
    m_enableWebAdminIf = true;

    // Local domain
    m_web_authentication_domain = "mydomain.com";

    // Default TCP/IP interface settings
    m_enableTcpip            = true;
    m_strTcpInterfaceAddress = "9598";
    m_encryptionTcpip        = 0;
    m_tcpip_ssl_certificate.clear();
    m_tcpip_ssl_certificate_chain.clear();
    m_tcpip_ssl_verify_peer = 0; // no=0, optional=1, yes=2
    m_tcpip_ssl_ca_path.clear();
    m_tcpip_ssl_ca_file.clear();
    m_tcpip_ssl_verify_depth         = 9;
    m_tcpip_ssl_default_verify_paths = false;
    m_tcpip_ssl_cipher_list.clear();
    m_tcpip_ssl_protocol_version = 0;
    m_tcpip_ssl_short_trust      = false;

    // UDP Server
    m_udpsrv.init(this);
    m_enableUdp = false;        // Disabled by default

    // MQTT functionality
    m_enableMqtt = false;       // Disabled by default

    // Web server SSL settings
    m_web_ssl_certificate          = "/etc/vscp/certs/server.pem";
    m_web_ssl_certificate_chain    = "";
    m_web_ssl_verify_peer          = false;
    m_web_ssl_ca_path              = "";
    m_web_ssl_ca_file              = "";
    m_web_ssl_verify_depth         = 9;
    m_web_ssl_default_verify_paths = true;
    m_web_ssl_cipher_list      = "DES-CBC3-SHA:AES128-SHA:AES128-GCM-SHA256";
    m_web_ssl_protocol_version = 3;
    m_web_ssl_short_trust      = false;

    // Webserver interface
    m_web_bEnable         = true;
    m_web_listening_ports = "[::]:9999r,[::]:8843s,8884";

    m_web_index_files = "index.xhtml,index.html,index.htm,"
                        "index.lp,index.lsp,index.lua,index.cgi,"
                        "index.shtml,index.php";

    m_web_document_root = m_rootFolder + "www/html";

    // Directory listings on by default
    m_web_enable_directory_listing          = true;
    m_web_enable_keep_alive                 = false;
    m_web_keep_alive_timeout_ms             = 0;
    m_web_access_control_list               = "";
    m_web_extra_mime_types                  = "";
    m_web_num_threads                       = 50;
    m_web_url_rewrite_patterns              = "";
    m_web_hide_file_patterns                = "";
    m_web_global_auth_file                  = "";
    m_web_per_directory_auth_file           = "";
    m_web_ssi_patterns                      = "";
    m_web_url_rewrite_patterns              = "";
    m_web_request_timeout_ms                = 10000;
    m_web_linger_timeout_ms                 = -1; // Do not set
    m_web_decode_url                        = true;
    m_web_ssi_patterns                      = "";
    m_web_access_control_allow_origin       = "*";
    m_web_access_control_allow_methods      = "*";
    m_web_access_control_allow_headers      = "*";
    m_web_error_pages                       = "";
    m_web_tcp_nodelay                       = 0;
    m_web_static_file_cache_control         = "";
    m_web_static_file_max_age               = 3600;
    m_web_strict_transport_security_max_age = -1;
    m_web_allow_sendfile_call               = true;
    m_web_additional_header                 = "";
    m_web_max_request_size                  = 16384;
    m_web_allow_index_script_resource       = false;
    m_web_duktape_script_patterns           = "**.ssjs$";
    m_web_lua_preload_file                  = "";
    m_web_lua_script_patterns               = "**.lua$";
    m_web_lua_server_page_patterns          = "**.lp$|**.lsp$";
    m_web_lua_websocket_patterns            = "**.lua$";
    m_web_lua_background_script             = "";
    m_web_lua_background_script_params      = "";

    m_bWebsocketsEnable       = true;
    m_websocket_document_root = "";
    m_websocket_timeout_ms = atoi(VSCPDB_CONFIG_DEFAULT_WEBSOCKET_TIMEOUT_MS);
    bEnable_websocket_ping_pong = false;
    lua_websocket_pattern =
      std::string(VSCPDB_CONFIG_DEFAULT_WEB_LUA_WEBSOCKET_PATTERN);

    m_bEnableRestApi = true;

    // Init. web server subsystem - All features enabled
    // ssl mt locks will we initiated here for openssl 1.0
    if (0 == mg_init_library(MG_FEATURES_IPV6 | MG_FEATURES_WEBSOCKET |
                             MG_FEATURES_LUA | MG_FEATURES_SSJS |
                             MG_FEATURES_COMPRESSION)) {
        syslog(LOG_ERR, "Failed to initialize webserver subsystem.");
    }

    // Initialize the CRC
    crcInit();
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CControlObject::~CControlObject()
{
    if (__VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG, "Cleaning up");
    }

    // Delete all UDP clients
    while (m_udpremotes.size()) {
        udpRemoteClient *pClient = m_udpremotes.front();
        m_udpremotes.front();
        delete pClient;
    }

    // Remove objects in Client send queue
    std::list<vscpEvent*>::iterator iterVSCP;
    pthread_mutex_lock(&m_mutex_ClientOutputQueue);
    for (iterVSCP = m_clientOutputQueue.begin();
         iterVSCP != m_clientOutputQueue.end();
         ++iterVSCP) {
        vscpEvent* pEvent = *iterVSCP;
        vscp_deleteEvent_v2(&pEvent);
    }
    m_clientOutputQueue.clear();
    pthread_mutex_unlock(&m_mutex_ClientOutputQueue);

    // Remove all clients
    m_clientList.removeAllClients();

    // Clean up civetweb
    mg_exit_library();

    if (0 != sem_destroy(&m_semClientOutputQueue)) {
        syslog(LOG_ERR, "Unable to destroy m_semClientOutputQueue");
    }

    if (0 != sem_destroy(&m_semSentToAllClients)) {
        syslog(LOG_ERR, "Unable to destroy m_semSentToAllClients");
    }

    if (0 != pthread_mutex_destroy(&m_mutex_ClientOutputQueue)) {
        syslog(LOG_ERR, "Unable to destroy m_mutex_ClientOutputQueue");
        return;
    }

    if (0 != pthread_mutex_destroy(&m_mutex_websrvSession)) {
        syslog(LOG_ERR, "Unable to destroy m_mutex_websrvSession");
        return;
    }

    if (0 != pthread_mutex_destroy(&m_mutex_restSession)) {
        syslog(LOG_ERR, "Unable to destroy m_mutex_restSession");
        return;
    }

    if (0 != pthread_mutex_destroy(&m_mutex_websocketSession)) {
        syslog(LOG_ERR, "Unable to destroy m_mutex_websocketSession");
        return;
    }

    if (0 != pthread_mutex_destroy(&m_mutex_DeviceList)) {
        syslog(LOG_ERR, "Unable to destroy m_mutex_DeviceList");
        return;
    }

    if (0 != pthread_mutex_destroy(&m_mutex_clientList)) {
        syslog(LOG_ERR, "Unable to destroy m_mutex_clientList");
        return;
    }

    if (0 != pthread_mutex_destroy(&m_mutex_UserList)) {
        syslog(LOG_ERR, "Unable to destroy m_mutex_UserList");
        return;
    }

    if (0 != pthread_mutex_destroy(&m_mutexUDPRemotes)) {
        syslog(LOG_ERR, "Unable to destroy m_mutexUDPRemotes");
        return;
    }

    if (__VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG, "Terminating the vscpd daemon");
    }

    // Close syslog
    closelog();
}

/////////////////////////////////////////////////////////////////////////////
// init
//

bool
CControlObject::init(std::string& strcfgfile, std::string& rootFolder)
{
    std::string str;

    // Save root folder for later use.
    m_rootFolder = rootFolder;

    // Root folder must exist
    if (!vscp_fileExists(m_rootFolder.c_str())) {
        syslog(LOG_ERR,
               "The specified rootfolder does not exist (%s).",
               (const char*)m_rootFolder.c_str());
        return false;
    }

    std::string strRootwww = m_rootFolder + "www/html";
    m_web_document_root    = strRootwww;

    // Change locale to get the correct decimal point "."
    setlocale(LC_NUMERIC, "C");

    // A configuration file must be available
    if (!vscp_fileExists(strcfgfile.c_str())) {
        printf("No configuration file. Can't initialize!.");
        syslog(LOG_ERR,
               "No configuration file. Can't initialize!. Path=%s",
               strcfgfile.c_str());
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////
    //                         Read XML configuration
    ////////////////////////////////////////////////////////////////////////////

    // Read XML configuration
    if (__VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG, "Reading configuration file");
    }

    // Read XML configuration
    try {
        if (!readConfiguration(strcfgfile)) {
            syslog(LOG_ERR,
                   "Unable to open/parse configuration file. Can't initialize! "
                   "Path =%s",
                   strcfgfile.c_str());
            return FALSE;
        }
    }
    catch (...) {
        syslog(LOG_ERR, "Exception when reading configuration file");
        return FALSE;
    }

#ifndef WIN32
    if (m_runAsUser.length()) {
        struct passwd* pw;
        if (NULL == (pw = getpwnam(m_runAsUser.c_str()))) {
            syslog(LOG_ERR, "Unknown user.");
        }
        else if (setgid(pw->pw_gid) != 0) {
            syslog(LOG_ERR, "setgid() failed. [%s]", strerror(errno));
        }
        else if (setuid(pw->pw_uid) != 0) {
            syslog(LOG_ERR, "setuid() failed. [%s]", strerror(errno));
        }
    }
#endif

    if (__VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG, "Using configuration file: %s", strcfgfile.c_str());
    }

    //==========================================================================
    //                           Add admin user
    //==========================================================================

    m_userList.addSuperUser(m_admin_user,
                            m_admin_password,
                            m_web_authentication_domain,
                            m_admin_allowfrom); // Remotes allows to connect

    //==========================================================================
    //                           Add driver user
    //==========================================================================

    // Generate username and password for drivers
    char buf[128];
    randPassword pw(4);

    // Level III Driver Username
    memset(buf, 0, sizeof(buf));
    pw.generatePassword(32, buf);
    m_driverUsername = "drv_";
    m_driverUsername += std::string(buf);

    // Level III Driver Password (can't contain ";" character)
    memset(buf, 0, sizeof(buf));
    pw.generatePassword(32, buf);
    m_driverPassword = buf;

    std::string drvhash;
    vscp_makePasswordHash(drvhash, std::string(buf));

    m_userList.addUser(m_driverUsername,
                       drvhash,                     // salt;hash
                       "System added driver user.", // full name
                       "System added driver user.", // note
                       m_web_authentication_domain,
                       NULL,
                       "driver",
                       "+127.0.0.0/24", // Only local
                       "*:*",           // All events
                       0);

    // Get GUID
    if (m_guid.isNULL()) {
        if (!getMacAddress(m_guid)) {
            // We failed to create GUID from MAC address use
            // 'localhost' IP instead as the base.
            getIPAddress(m_guid);
        }
    }

    // If no server name set construct one
    if (0 == m_strServerName.length()) {
        m_strServerName = "VSCP Server @ ";
        std::string strguid;
        m_guid.toString(strguid);
        m_strServerName += std::string(strguid);
    }

    str = "VSCP Server started - ";
    str += "Version: ";
    str += VSCPD_DISPLAY_VERSION;
    str += " - ";
    str += VSCPD_COPYRIGHT;
    syslog(LOG_INFO, "%s", str.c_str());

    // Start daemon internal client worker thread
    try {
        startClientMsgWorkerThread();
    }
    catch (...) {
        syslog(LOG_ERR, "Exception when starting message worker thread");
        return FALSE;
    }

    // Start webserver and websockets
    // IMPORTANT!!!!!!!!
    // Must be started before the tcp/ip server as
    // ssl initializarion is done here
    try {
        start_webserver();
    }
    catch (...) {
        syslog(LOG_ERR, "Exception when starting web server");
        return FALSE;
    }

    // Start TCP/IP interface
    try {
        startTcpipSrvThread();
    }
    catch (...) {
        syslog(LOG_ERR, "Exception when starting tcp/ip server");
        return FALSE;
    }

    // Start UDP interface
    try {
        startUDPSrvThreads();
    }
    catch (...) {
        syslog(LOG_ERR, "Exception when starting UDP server");
        return FALSE;
    }

    // Load drivers
    try {
        startDeviceWorkerThreads();
    }
    catch (...) {
        syslog(LOG_ERR, "Exception when loading drivers");
        return FALSE;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// run - Program main loop
//
// Most work is done in the threads at the moment
//

bool
CControlObject::run(void)
{
    std::deque<CClientItem*>::iterator nodeClient;

    // We need to create a clientItem and add this object to the list
    CClientItem* pClientItem = new CClientItem;
    if (NULL == pClientItem) {
        syslog(LOG_ERR, "Unable to allocate Client item, Ending.");
        return false;
    }

    // This is an active client
    pClientItem->m_bOpen         = true;
    pClientItem->m_type          = CLIENT_ITEM_INTERFACE_TYPE_CLIENT_INTERNAL;
    pClientItem->m_strDeviceName = "Internal Server Client.|Started at ";
    pClientItem->m_strDeviceName += vscpdatetime::Now().getISODateTime();

    // Add the client to the Client List
    pthread_mutex_lock(&m_clientList.m_mutexItemList);
    if (!addClient(pClientItem, CLIENT_ID_INTERNAL)) {
        // Failed to add client
        delete pClientItem;
        syslog(LOG_ERR, "ControlObject: Failed to add internal client.");
        pthread_mutex_unlock(&m_clientList.m_mutexItemList);
        delete pClientItem;
        return false;
    }
    pthread_mutex_unlock(&m_clientList.m_mutexItemList);

    if (__VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG, "Mainloop starting");
    }

#ifdef WITH_SYSTEMD
    sd_notify(0, "READY=1");
#endif

    //-------------------------------------------------------------------------
    //                            MAIN - LOOP
    //-------------------------------------------------------------------------

    struct timespec now, old_now;
    clock_gettime(CLOCK_REALTIME, &old_now);
    old_now.tv_sec -= 60;  // Do firts send right away

    while (!m_bQuit) {

        clock_gettime(CLOCK_REALTIME, &now);

        // We send heartbeat every minute
        if ((now.tv_sec-old_now.tv_sec) > 60) {

            // Save time
            clock_gettime(CLOCK_REALTIME, &old_now);

            if (!automation(pClientItem)) {
                syslog(LOG_ERR, "Failed to send automation events!");
            }
        }

        // Wait for event
        if ((-1 == vscp_sem_wait(&m_semSentToAllClients, 10)) &&
            errno == ETIMEDOUT) {
            continue;
        }

        // Send events to websocket clients
        websock_post_incomingEvents();

        //----------------------------------------------------------------------
        //                         Event received here
        //                   from one of the incoming source
        //----------------------------------------------------------------------

        if (pClientItem->m_clientInputQueue.size()) {

            vscpEvent* pEvent;

            pthread_mutex_lock(&pClientItem->m_mutexClientInputQueue);
            pEvent = pClientItem->m_clientInputQueue.front();
            pClientItem->m_clientInputQueue.pop_front();
            pthread_mutex_unlock(&pClientItem->m_mutexClientInputQueue);

            if (NULL != pEvent) {
            }

            vscp_deleteEvent_v2(&pEvent);

        } // Event in queue

    } // while

    // Remove messages in the client queues
    pthread_mutex_lock(&m_clientList.m_mutexItemList);
    removeClient(pClientItem);
    pthread_mutex_unlock(&m_clientList.m_mutexItemList);

    // Clean up is called in main file

    if (__VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG, "Mainloop ending");
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// automation

bool
CControlObject::automation(CClientItem* pClientItem)
{
    vscpEventEx ex;

    // Send VSCP_CLASS1_INFORMATION,
    // Type=9/VSCP_TYPE_INFORMATION_NODE_HEARTBEAT
    ex.obid      = 0; // IMPORTANT Must be set by caller before event is sent
    ex.head      = 0;
    ex.timestamp = vscp_makeTimeStamp();
    vscp_setEventExToNow(&ex); // Set time to current time
    ex.vscp_class = VSCP_CLASS1_INFORMATION;
    ex.vscp_type  = VSCP_TYPE_INFORMATION_NODE_HEARTBEAT;
    ex.sizeData   = 3;

    // GUID
    memcpy(ex.data + VSCP_CAPABILITY_OFFSET_GUID, m_guid.getGUID(), 16);

    ex.data[0] = 0; // index
    ex.data[1] = 0; // zone
    ex.data[2] = 0; // subzone

    if (!sendEvent(pClientItem, &ex)) {
        syslog(LOG_ERR, "Failed to send Class1 heartbeat");
    }

    // Send VSCP_CLASS2_INFORMATION,
    // Type=2/VSCP2_TYPE_INFORMATION_HEART_BEAT
    ex.obid      = 0; // IMPORTANT Must be set by caller before event is sent
    ex.head      = 0;
    ex.timestamp = vscp_makeTimeStamp();
    vscp_setEventExToNow(&ex); // Set time to current time
    ex.vscp_class = VSCP_CLASS2_INFORMATION;
    ex.vscp_type  = VSCP2_TYPE_INFORMATION_HEART_BEAT;
    ex.sizeData   = 64;

    // GUID
    memcpy(ex.data + VSCP_CAPABILITY_OFFSET_GUID, m_guid.getGUID(), 16);

    memset(ex.data, 0, sizeof(ex.data));
    memcpy(ex.data,
           m_strServerName.c_str(),
           std::min((int)strlen(m_strServerName.c_str()), 64));

    if (!sendEvent(pClientItem, &ex)) {
        syslog(LOG_ERR, "Failed to send Class2 heartbeat");
    }

    // Send VSCP_CLASS1_PROTOCOL,
    // Type=1/VSCP_TYPE_PROTOCOL_SEGCTRL_HEARTBEAT
    ex.obid      = 0; // IMPORTANT Must be set by caller before event is sent
    ex.head      = 0;
    ex.timestamp = vscp_makeTimeStamp();
    vscp_setEventExToNow(&ex); // Set time to current time
    ex.vscp_class = VSCP_CLASS1_PROTOCOL;
    ex.vscp_type  = VSCP_TYPE_PROTOCOL_SEGCTRL_HEARTBEAT;
    ex.sizeData   = 5;

    // GUID
    memcpy(ex.data + VSCP_CAPABILITY_OFFSET_GUID, m_guid.getGUID(), 16);

    time_t tnow;
    time(&tnow);
    uint32_t time32 = (uint32_t)tnow;

    ex.data[0] = 0; // 8 - bit crc for VSCP daemon GUID
    ex.data[1] = (uint8_t)((time32 >> 24) & 0xff); // Time since epoch MSB
    ex.data[2] = (uint8_t)((time32 >> 16) & 0xff);
    ex.data[3] = (uint8_t)((time32 >> 8) & 0xff);
    ex.data[4] = (uint8_t)((time32)&0xff); // Time since epoch LSB

    if (!sendEvent(pClientItem, &ex)) {
        syslog(LOG_ERR, "Failed to send segment controller heartbeat");
    }

    // Send VSCP_CLASS2_PROTOCOL,
    // Type=20/VSCP2_TYPE_PROTOCOL_HIGH_END_SERVER_CAPS
    ex.obid      = 0; // IMPORTANT Must be set by caller before event is sent
    ex.head      = 0;
    ex.timestamp = vscp_makeTimeStamp();
    vscp_setEventExToNow(&ex); // Set time to current time
    ex.vscp_class = VSCP_CLASS2_PROTOCOL;
    ex.vscp_type  = VSCP2_TYPE_PROTOCOL_HIGH_END_SERVER_CAPS;

    // Fill in data
    memset(ex.data, 0, sizeof(ex.data));

    // GUID
    memcpy(ex.data + VSCP_CAPABILITY_OFFSET_GUID, m_guid.getGUID(), 16);

    // Server ip address
    cguid guid;
    if (getIPAddress(guid)) {
        ex.data[VSCP_CAPABILITY_OFFSET_IP_ADDR]     = guid.getAt(8);
        ex.data[VSCP_CAPABILITY_OFFSET_IP_ADDR + 1] = guid.getAt(9);
        ex.data[VSCP_CAPABILITY_OFFSET_IP_ADDR + 2] = guid.getAt(10);
        ex.data[VSCP_CAPABILITY_OFFSET_IP_ADDR + 3] = guid.getAt(11);
    }

    // Server name
    memcpy(ex.data + VSCP_CAPABILITY_OFFSET_SRV_NAME,
           (const char*)m_strServerName.c_str(),
           std::min((int)strlen((const char*)m_strServerName.c_str()), 64));

    // Capabilities array
    getVscpCapabilities(ex.data);

    // non-standard ports
    // TODO

    ex.sizeData = 104;

    if (!sendEvent(pClientItem, &ex)) {
        syslog(LOG_ERR, "Failed to send high end server capabilities.");
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// cleanup

bool
CControlObject::cleanup(void)
{
    if (__VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG,
               "ControlObject: cleanup - Giving worker threads time to stop "
               "operations...");
    }

    if (__VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG,
               "ControlObject: cleanup - Stopping device worker thread...");
    }

    try {
        stopDeviceWorkerThreads();
    }
    catch (...) {
        syslog(LOG_ERR,
               "REST: Exception occurred when stoping device worker threads");
    }

    if (__VSCP_DEBUG_EXTRA) {
        syslog(
          LOG_DEBUG,
          "ControlObject: cleanup - Stopping VSCP Server worker thread...");
    }

    // stopDaemonWorkerThread(); *****

    if (__VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG,
               "ControlObject: cleanup - Stopping client worker thread...");
    }

    try {
        stopClientMsgWorkerThread();
    }
    catch (...) {
        syslog(LOG_ERR, "Exception occurred when stoping client worker thread");
    }

    if (__VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG,
               "ControlObject: cleanup - Stopping Web Server worker thread...");
    }

    try {
        stop_webserver();
    }
    catch (...) {
        syslog(LOG_ERR, "REST: Exception occurred when stoping web server");
    }

    if (__VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG,
               "ControlObject: cleanup - Stopping TCP/IP worker thread...");
    }

    try {
        stopUDPSrvThreads();
    }
    catch (...) {
        syslog(LOG_ERR, "REST: Exception occurred when stoping UDP server");
    }

    try {
        stopTcpipSrvThread();
    }
    catch (...) {
        syslog(LOG_ERR, "REST: Exception occurred when stoping tcp/ip server");
    }

    if (__VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG, "Controlobject: ControlObject: Cleanup done.");
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// startClientMsgWorkerThread
//

bool
CControlObject::startClientMsgWorkerThread(void)
{
    if (__VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG, "Controlobject: Starting client worker thread...");
    }

    if (pthread_create(&m_clientMsgWorkerThread,
                       NULL,
                       clientMsgWorkerThread,
                       this)) {

        syslog(LOG_ERR, "Controlobject: Unable to start client thread.");
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// stopClientMsgWorkerThread
//

bool
CControlObject::stopClientMsgWorkerThread(void)
{
    // Request therad to terminate
    m_bQuit_clientMsgWorkerThread = true;
    pthread_join(m_clientMsgWorkerThread, NULL);

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// startTcpWorkerThread
//

bool
CControlObject::startTcpipSrvThread(void)
{
    if (!m_enableTcpip) {
        if (__VSCP_DEBUG_TCP) {
            syslog(LOG_DEBUG, "Controlobject: TCP/IP interface disabled.");
        }
        return true;
    }

    if (__VSCP_DEBUG_TCP) {
        syslog(LOG_DEBUG, "Controlobject: Starting TCP/IP interface...");
    }

    // Create the tcp/ip server data object
    m_ptcpipSrvObject = (tcpipListenThreadObj*)new tcpipListenThreadObj(this);
    if (NULL == m_ptcpipSrvObject) {
        syslog(LOG_ERR,
               "Controlobject: Failed to allocate storage for tcp/ip.");
    }

    // Set the port to listen for connections on
    m_ptcpipSrvObject->setListeningPort(m_strTcpInterfaceAddress);

    if (pthread_create(&m_tcpipListenThread,
                       NULL,
                       tcpipListenThread,
                       m_ptcpipSrvObject)) {
        delete m_ptcpipSrvObject;
        m_ptcpipSrvObject = NULL;
        syslog(LOG_ERR,
               "Controlobject: Unable to start the tcp/ip listen thread.");
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// stopTcpWorkerThread
//

bool
CControlObject::stopTcpipSrvThread(void)
{
    // Tell the thread it's time to quit
    m_ptcpipSrvObject->m_nStopTcpIpSrv = VSCP_TCPIP_SRV_STOP;

    if (__VSCP_DEBUG_TCP) {
        syslog(LOG_DEBUG, "Controlobject: Terminating TCP thread.");
    }

    pthread_join(m_tcpipListenThread, NULL);
    delete m_ptcpipSrvObject;
    m_ptcpipSrvObject = NULL;

    if (__VSCP_DEBUG_TCP) {
        syslog(LOG_DEBUG, "Controlobject: Terminated TCP thread.");
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// startUSPSrvThreads
//

bool
CControlObject::startUDPSrvThreads(void)
{
    if (!m_enableUdp) {
        if (__VSCP_DEBUG_TCP) {
            syslog(LOG_DEBUG, "Controlobject: UDP interface disabled.");
        }
        return true;
    }

    if (pthread_create(&m_udpSrvWorkerThread,
                       NULL,
                       udpSrvWorkerThread,
                       &m_udpsrv)) {
        syslog(LOG_ERR,
               "Controlobject: Unable to start the UDP server thread.");
        return false;
    }

    // Create UDP client threads.
    for (auto it = m_udpremotes.cbegin(); it != m_udpremotes.cend(); ++it) {
		
        udpRemoteClient *pClient =  *it;
        if ( NULL == pClient ) {
            syslog(LOG_ERR,
                        "UDP client object is NULL. Will skip it");
            continue;
        }

        if (!pClient->startWorkerThread()) {
            syslog(LOG_ERR,"Failed to start UDP sender workerthread for interface %s",
                pClient->getRemoteAddress().c_str());
        }
        // TODO debug message
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// stopUSPSrvThreads
//

bool
CControlObject::stopUDPSrvThreads(void)
{
    // Tell the thread it's time to quit
    m_udpsrv.m_bQuit = VSCP_TCPIP_SRV_STOP;

    if (__VSCP_DEBUG_TCP) {
        syslog(LOG_DEBUG, "Controlobject: Terminating UDP thread.");
    }

    pthread_join(m_udpSrvWorkerThread, NULL);


    if (__VSCP_DEBUG_TCP) {
        syslog(LOG_DEBUG, "Controlobject: Terminated UDP thread.");
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// startMQTTClientThreads
//

bool
CControlObject::startMQTTClientThreads(void)
{
    if (!m_enableMqtt) {
        if (__VSCP_DEBUG_TCP) {
            syslog(LOG_DEBUG, "Controlobject: MQTT interface disabled.");
        }
        return true;
    }

    // Create and start MQTT client threads.
    for (auto it = m_mqttClients.cbegin(); it != m_mqttClients.cend(); ++it) {

        vscpMqttObj *pMqttObj =  *it;
        if ( NULL == pMqttObj ) {
            syslog(LOG_ERR,
                    "MQTT client object is NULL. Will skip it");
            continue;
        }

        if (!pMqttObj->startWorkerThread(this)) {
            syslog(LOG_ERR,
                    "Failed to start MQTT client workerthread for interface %s",
                    pMqttObj->getInterface().c_str());
        }

        // TODO debug message
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// stopMQTTClientThreads
//

bool
CControlObject::stopMQTTClientThreads(void)
{
    // Tell the thread it's time to quit
    m_udpsrv.m_bQuit = VSCP_TCPIP_SRV_STOP;

    if (__VSCP_DEBUG_TCP) {
        syslog(LOG_DEBUG, "Controlobject: Terminating MQTT thread.");
    }
    
    // Create and start MQTT client threads.
    //for (auto it = m_mqttClients.cbegin(); it != m_mqttClients.cend(); ++it) {
    while (m_mqttClients.size()) {

        vscpMqttObj *pMqttObj =  m_mqttClients.front();
        m_mqttClients.pop_front();
        if ( NULL == pMqttObj ) {
            syslog(LOG_ERR,
                    "MQTT client object is NULL. Will skip it");
            continue;
        }

        // End workerthread
        pMqttObj->m_bQuit = true;
        pthread_join(pMqttObj->m_mqttClientWorkerThread, NULL);

        delete pMqttObj;

        // TODO debug message
    }

    if (__VSCP_DEBUG_TCP) {
        syslog(LOG_DEBUG, "Controlobject: Terminated MQTT thread.");
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
// startDeviceWorkerThreads
//

bool
CControlObject::startDeviceWorkerThreads(void)
{
    CDeviceItem* pDeviceItem;
    if (__VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG, "[Controlobject][Driver] - Starting drivers...");
    }

    std::deque<CDeviceItem*>::iterator it;
    for (it = m_deviceList.m_devItemList.begin();
         it != m_deviceList.m_devItemList.end();
         ++it) {

        pDeviceItem = *it;
        if (NULL != pDeviceItem) {

            if (__VSCP_DEBUG_EXTRA) {
                syslog(LOG_DEBUG,
                       "Controlobject: [Driver] - Preparing: %s ",
                       pDeviceItem->m_strName.c_str());
            }

            // Just start if enabled
            if (!pDeviceItem->m_bEnable)
                continue;

            if (__VSCP_DEBUG_EXTRA) {
                syslog(LOG_DEBUG,
                       "Controlobject: [Driver] - Starting: %s ",
                       pDeviceItem->m_strName.c_str());
            }

            // Start  the driver logic
            pDeviceItem->startDriver(this);

        } // Valid device item
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
// stopDeviceWorkerThreads
//

bool
CControlObject::stopDeviceWorkerThreads(void)
{
    CDeviceItem* pDeviceItem;

    if (__VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG, "[Controlobject][Driver] - Stopping drivers...");
    }
    std::deque<CDeviceItem*>::iterator iter;
    for (iter = m_deviceList.m_devItemList.begin();
         iter != m_deviceList.m_devItemList.end();
         ++iter) {

        pDeviceItem = *iter;
        if (NULL != pDeviceItem) {
            if (__VSCP_DEBUG_EXTRA) {
                syslog(LOG_DEBUG,
                       "Controlobject: [Driver] - Stopping: %s ",
                       pDeviceItem->m_strName.c_str());
            }
            pDeviceItem->stopDriver();
        }
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// generateSessionId
//

bool
CControlObject::generateSessionId(const char* pKey, char* psid)
{
    char buf[8193];

    // Check pointers
    if (NULL == pKey)
        return false;
    if (NULL == psid)
        return false;

    if (strlen(pKey) > 256)
        return false;

    // Generate a random session ID
    time_t t;
    t = time(NULL);
    sprintf(buf,
            "__%s_%X%X%X%X_be_hungry_stay_foolish_%X%X",
            pKey,
            (unsigned int)rand(),
            (unsigned int)rand(),
            (unsigned int)rand(),
            (unsigned int)t,
            (unsigned int)rand(),
            1337);

    vscp_md5(psid, (const unsigned char*)buf, strlen(buf));

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// getVscpCapabilities
//

bool
CControlObject::getVscpCapabilities(uint8_t* pCapability)
{
    // Check pointer
    if (NULL == pCapability)
        return false;

    uint64_t caps = 0;
    memset(pCapability, 0, 8);

    // VSCP Multicast interface
    // if (m_bEnableMulticast) {
    //     caps |= VSCP_SERVER_CAPABILITY_MULTICAST_CHANNEL;
    // }

    // VSCP TCP/IP interface
    if (m_enableTcpip) {
        caps |= VSCP_SERVER_CAPABILITY_TCPIP;
    }

    // VSCP UDP interface
    // if (m_udpSrvObj.m_bEnable) {
    //     caps |= VSCP_SERVER_CAPABILITY_UDP;
    // }

    // VSCP Multicast announce interface
    // if (m_bEnableMulticastAnnounce) {
    //     caps |= VSCP_SERVER_CAPABILITY_MULTICAST_ANNOUNCE;
    // }

    // VSCP raw Ethernet interface
    if (1) {
        caps |= VSCP_SERVER_CAPABILITY_RAWETH;
    }

    // VSCP web server
    if (m_web_bEnable) {
        caps |= VSCP_SERVER_CAPABILITY_WEB;
    }

    // VSCP websocket interface
    if (m_web_bEnable) {
        caps |= VSCP_SERVER_CAPABILITY_WEBSOCKET;
    }

    // VSCP websocket interface
    if (m_web_bEnable) {
        caps |= VSCP_SERVER_CAPABILITY_REST;
    }

    // IPv6 support
    if (0) {
        caps |= VSCP_SERVER_CAPABILITY_IP6;
    }

    // IPv4 support
    if (0) {
        caps |= VSCP_SERVER_CAPABILITY_IP4;
    }

    // SSL support
    if (1) {
        caps |= VSCP_SERVER_CAPABILITY_SSL;
    }

    // +2 tcp/ip connections support
    if (m_enableTcpip) {
        caps |= VSCP_SERVER_CAPABILITY_TWO_CONNECTIONS;
    }

    // AES256
    caps |= VSCP_SERVER_CAPABILITY_AES256;

    // AES192
    caps |= VSCP_SERVER_CAPABILITY_AES192;

    // AES128
    caps |= VSCP_SERVER_CAPABILITY_AES128;

    for (int i = 0; i < 8; i++) {
        pCapability[i] = caps & 0xff;
        caps           = caps >> 8;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// sendEventToClient
//

bool
CControlObject::sendEventToClient(CClientItem* pClientItem, vscpEvent* pEvent)
{
    // Must be valid pointers
    if (NULL == pClientItem) {
        syslog(LOG_ERR, "sendEventToClient - Pointer to clientitem is null");
        return false;
    }
    if (NULL == pEvent) {
        syslog(LOG_ERR, "sendEventToClient - Pointer to event is null");
        return false;
    }

    // Check if filtered out - if so do nothing here
    if (!vscp_doLevel2Filter(pEvent, &pClientItem->m_filter)) {
        if (__VSCP_DEBUG_EXTRA) {
            syslog(LOG_DEBUG, "sendEventToClient - Filtered out");
        }
        return false;
    }

    // If the client queue is full for this client then the
    // client will not receive the message
    if (pClientItem->m_clientInputQueue.size() >
        m_maxItemsInClientReceiveQueue) {
        if (__VSCP_DEBUG_EXTRA) {
            syslog(LOG_DEBUG, "sendEventToClient - overrun");
        }
        // Overrun
        pClientItem->m_statistics.cntOverruns++;
        return false;
    }

    // Create a new event
    vscpEvent* pnewvscpEvent = new vscpEvent;
    if (NULL != pnewvscpEvent) {

        // Copy in the new event
        if (!vscp_copyEvent(pnewvscpEvent, pEvent)) {
            vscp_deleteEvent_v2(&pnewvscpEvent);
            return false;
        }

        // Add the new event to the input queue
        pthread_mutex_lock(&pClientItem->m_mutexClientInputQueue);
        pClientItem->m_clientInputQueue.push_back(pnewvscpEvent);
        pthread_mutex_unlock(&pClientItem->m_mutexClientInputQueue);
        sem_post(&pClientItem->m_semClientInputQueue);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// sendEventAllClients
//

bool
CControlObject::sendEventAllClients(vscpEvent* pEvent, uint32_t excludeID)
{
    CClientItem* pClientItem;
    std::deque<CClientItem*>::iterator it;

    if (NULL == pEvent) {
        syslog(LOG_ERR, "sendEventAllClients - null event");
        return false;
    }

    pthread_mutex_lock(&m_clientList.m_mutexItemList);
    for (it = m_clientList.m_itemList.begin();
         it != m_clientList.m_itemList.end();
         ++it) {
        pClientItem = *it;

        if ((NULL != pClientItem) && (excludeID != pClientItem->m_clientID)) {
            if (__VSCP_DEBUG_EXTRA) {
                syslog(LOG_DEBUG,
                       "Send event to client [%s]",
                       pClientItem->m_strDeviceName.c_str());
            }
            if (!sendEventToClient(pClientItem, pEvent)) {
                syslog(LOG_ERR, "sendEventAllClients - Failed to send event");
            }
        }
    }

    pthread_mutex_unlock(&m_clientList.m_mutexItemList);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// sendEvent
//
// !!! pEventToSend must be deallocated by sender !!!
//

bool
CControlObject::sendEvent(CClientItem* pClientItem, vscpEvent* peventToSend)
{
    bool bSent = false;

    // Check pointers
    if (NULL == pClientItem) {
        syslog(LOG_ERR, "sendEvent - null clientItem");
        return false;
    }
    if (NULL == peventToSend) {
        syslog(LOG_ERR, "sendEvent - null event");
        return false;
    }

    // If timestamp is nulled make one
    if (0 == peventToSend->timestamp) {
        peventToSend->timestamp = vscp_makeTimeStamp();
    }

    // If obid is nulled set client interface id
    if (0 == peventToSend->obid) {
        peventToSend->obid = pClientItem->m_clientID;
    }

    // If GUID is all nilled set interface GUID
    if (vscp_isGUIDEmpty(peventToSend->GUID)) {
        memcpy(peventToSend->GUID, pClientItem->m_guid.getGUID(), 16);
    }

    vscpEvent* pEvent = new vscpEvent; // Create new VSCP Event
    if (NULL == pEvent) {
        syslog(LOG_ERR, "sendEvent - Allocation of event failed");
        return false;
    }

    pEvent->pdata = NULL;

    // Copy event
    if (!vscp_copyEvent(pEvent, peventToSend)) {
        vscp_deleteEvent_v2(&pEvent);
        syslog(LOG_ERR, "sendEvent - Event copy failed");
        return false;
    }

    // Save the originating clients id so
    // this client don't get the message back
    if ( 0 == pEvent->obid) {
        pEvent->obid = pClientItem->m_clientID;
    }

    // Level II events between 512-1023 is recognised by the daemon and
    // sent to the correct interface as Level I events if the interface
    // is addressed by the client.
    if ((pEvent->vscp_class <= 1023) && (pEvent->vscp_class >= 512) &&
        (pEvent->sizeData >= 16)) {

        // This event should be sent to the correct interface if it is
        // available on this machine. If not it should be sent to
        // the rest of the network as normal

        cguid destguid;
        destguid.getFromArray(pEvent->pdata);

        destguid.setAt(14, 0); // Interface GUID's have LSB bytes nilled
        destguid.setAt(15, 0);

        if (__VSCP_DEBUG_EXTRA) {
            syslog(LOG_DEBUG,
                   "Level I event over Level II "
                   "dest = %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:"
                   "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:",
                   destguid.getAt(0),
                   destguid.getAt(1),
                   destguid.getAt(2),
                   destguid.getAt(3),
                   destguid.getAt(4),
                   destguid.getAt(5),
                   destguid.getAt(6),
                   destguid.getAt(7),
                   destguid.getAt(8),
                   destguid.getAt(9),
                   destguid.getAt(10),
                   destguid.getAt(11),
                   destguid.getAt(12),
                   destguid.getAt(13),
                   destguid.getAt(14),
                   destguid.getAt(15));
        }

        // Find client
        pthread_mutex_lock(&m_clientList.m_mutexItemList);

        std::deque<CClientItem*>::iterator it;
        for (it = m_clientList.m_itemList.begin();
             it != m_clientList.m_itemList.end();
             ++it) {

            CClientItem* pItem = *it;
            if (__VSCP_DEBUG_EXTRA) {
                syslog(LOG_DEBUG,
                       "Test if = "
                       "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:  - %s",
                       pItem->m_guid.getAt(0),
                       pItem->m_guid.getAt(1),
                       pItem->m_guid.getAt(2),
                       pItem->m_guid.getAt(3),
                       pItem->m_guid.getAt(4),
                       pItem->m_guid.getAt(5),
                       pItem->m_guid.getAt(6),
                       pItem->m_guid.getAt(7),
                       pItem->m_guid.getAt(8),
                       pItem->m_guid.getAt(9),
                       pItem->m_guid.getAt(10),
                       pItem->m_guid.getAt(11),
                       pItem->m_guid.getAt(12),
                       pItem->m_guid.getAt(13),
                       pItem->m_guid.getAt(14),
                       pItem->m_guid.getAt(15),
                       pItem->m_strDeviceName.c_str());
                syslog(LOG_DEBUG,"Match = %s",
                        (pItem->m_guid == destguid) ? "true" : "false" );       
            }

            if (pItem->m_guid == destguid) {
                // Found
                // pDestClientItem = pItem;
                bSent = true;
                if (!sendEventToClient(pItem, pEvent)) {
                    syslog(LOG_DEBUG,"sendEventToClient: Failed!");
                }
                break;
            }
        }

        pthread_mutex_unlock(&m_clientList.m_mutexItemList);
    }

    if (!bSent) {

        // There must be room in the send queue
        if (m_maxItemsInClientReceiveQueue > m_clientOutputQueue.size()) {

            pthread_mutex_lock(&m_mutex_ClientOutputQueue);
            m_clientOutputQueue.push_back(pEvent);

            // TX Statistics
            pClientItem->m_statistics.cntTransmitData += pEvent->sizeData;
            pClientItem->m_statistics.cntTransmitFrames++;

            pthread_mutex_unlock(&m_mutex_ClientOutputQueue);

            sem_post(&m_semClientOutputQueue);
        }
        else {
            if (__VSCP_DEBUG_EXTRA) {
                syslog(LOG_DEBUG, "sendEvent - overrun");
            }
            pClientItem->m_statistics.cntOverruns++;
            vscp_deleteEvent_v2(&pEvent);
            return false;
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// sendEvent
//

bool
CControlObject::sendEvent(CClientItem* pClientItem, vscpEventEx* pex)
{
    bool rv;
    vscpEvent ev;

    if (!vscp_convertEventExToEvent(&ev, pex)) {
        syslog(LOG_ERR, "sendEvent: Failed in vscp_convertEventExToEvent");
        return false;
    }

    if (!(rv = sendEvent(pClientItem, &ev))) {
        syslog(LOG_ERR, "sendEvent: Failed to send event");
    }

    vscp_deleteEvent(&ev);

    return rv;
}

//////////////////////////////////////////////////////////////////////////////
// addClient
//

bool
CControlObject::addClient(CClientItem* pClientItem, uint32_t id)
{
    // Check pointer
    if ( NULL == pClientItem ) {
        return false;
    }

    // Add client to client list
    if (!m_clientList.addClient(pClientItem, id)) {
        return false;
    }

    // Set GUID for interface
    pClientItem->m_guid = m_guid;

    // Fill in client id
    pClientItem->m_guid.setNicknameID(0);
    pClientItem->m_guid.setClientID(pClientItem->m_clientID);

    return true;
}

//////////////////////////////////////////////////////////////////////////////
// addClient - GUID (for drivers with set GUID)
//

bool
CControlObject::addClient(CClientItem* pClientItem, cguid& guid)
{
    // Check pointer
    if ( NULL == pClientItem ) {
        return false;
    }

    // Add client to client list
    if (!m_clientList.addClient(pClientItem, guid)) {
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////
// removeClient
//

void
CControlObject::removeClient(CClientItem* pClientItem)
{
    // Do not try to handle invalid clients
    if (NULL == pClientItem)
        return;

    // Remove the client
    m_clientList.removeClient(pClientItem);
}

//////////////////////////////////////////////////////////////////////////////
// addUdpClient
//

void CControlObject::addUdpClient(bool bEnable,
                                    std::string& interface,
                                    std::string& user,
                                    std::string& password,
                                    vscpEventFilter& filter,
                                    int encryption,
                                    bool setbroadcast)
{
    udpRemoteClient *pClient = new udpRemoteClient();
    if ( NULL == pClient ) {
      syslog(LOG_ERR,"Unable to allocate client structure for UDP client. if=%s",interface.c_str());
      return;
    }

    // udp:://1.2.3.4:1234
    // udp://a.b.c:1234
    // udp://1234
    // udp://1.2.3.4
    long unsigned int pos;
    std::string remoteAddress = "127.0.0.1";
    short port = VSCP_DEFAULT_UDP_PORT;

    vscp_startsWith(interface, "udp://", &interface);
    vscp_trim(interface);
    if ( interface.length()) {
        if (std::string::npos != (pos = interface.find_first_of(':'))) {
            remoteAddress = interface.substr(0,pos);
            port = atoi(interface.substr(pos+1).c_str());
        }
        else {
            remoteAddress = interface;
        }
    }

    char buf[50];
    if ( VSCP_ERROR_SUCCESS == vscp_hostname_to_ip(buf,remoteAddress.c_str()) ) {
        remoteAddress = buf;       
    }

    pClient->enable(bEnable);
    pClient->setControlObjectPointer(this);
    pClient->setRemoteAddress(remoteAddress);
    pClient->setRemotePort(port);
    pClient->setUser(user);
    pClient->setFilter(filter);
    pClient->setEncryption(encryption);
    pClient->SetBroadcastBit(setbroadcast);

    m_udpremotes.push_back(pClient);
}

//////////////////////////////////////////////////////////////////////////////
// addMqttClient
// 

void CControlObject::addMqttClient( std::string &interface,
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
			                std::string &certfile,
			                std::string &keyfile,
			                std::string &xpassword )
{
    // TODO
}


//////////////////////////////////////////////////////////////////////////////
// addKnowNode
//

void
CControlObject::addKnownNode(cguid& guid, cguid& ifguid, std::string& name)
{
    ; // TODO
}

///////////////////////////////////////////////////////////////////////////////
//  getMacAddress
//

bool
CControlObject::getMacAddress(cguid& guid)
{
#ifdef WIN32

    bool rv = false;
    NCB Ncb;
    UCHAR uRetCode;
    LANA_ENUM lenum;
    int i;

    // Clear the GUID
    guid.clear();

    memset(&Ncb, 0, sizeof(Ncb));
    Ncb.ncb_command = NCBENUM;
    Ncb.ncb_buffer  = (UCHAR*)&lenum;
    Ncb.ncb_length  = sizeof(lenum);
    uRetCode        = Netbios(&Ncb);
    // printf( "The NCBENUM return code is: 0x%x ", uRetCode );

    for (i = 0; i < lenum.length; i++) {
        memset(&Ncb, 0, sizeof(Ncb));
        Ncb.ncb_command  = NCBRESET;
        Ncb.ncb_lana_num = lenum.lana[i];

        uRetCode = Netbios(&Ncb);

        memset(&Ncb, 0, sizeof(Ncb));
        Ncb.ncb_command  = NCBASTAT;
        Ncb.ncb_lana_num = lenum.lana[i];

        strcpy((char*)Ncb.ncb_callname, "*               ");
        Ncb.ncb_buffer = (unsigned char*)&Adapter;
        Ncb.ncb_length = sizeof(Adapter);

        uRetCode = Netbios(&Ncb);

        if (uRetCode == 0) {
            guid.setAt(0, 0xff);
            guid.setAt(1, 0xff);
            guid.setAt(2, 0xff);
            guid.setAt(3, 0xff);
            guid.setAt(4, 0xff);
            guid.setAt(5, 0xff);
            guid.setAt(6, 0xff);
            guid.setAt(7, 0xfe);
            guid.setAt(8, Adapter.adapt.adapter_address[0]);
            guid.setAt(9, Adapter.adapt.adapter_address[1]);
            guid.setAt(10, Adapter.adapt.adapter_address[2]);
            guid.setAt(11, Adapter.adapt.adapter_address[3]);
            guid.setAt(12, Adapter.adapt.adapter_address[4]);
            guid.setAt(13, Adapter.adapt.adapter_address[5]);
            guid.setAt(14, 0);
            guid.setAt(15, 0);
#ifdef DEBUG__
            char buf[256];
            sprintf(buf,
                    "The Ethernet MAC Address: %02x:%02x:%02x:%02x:%02x:%02x",
                    guid.getAt(2),
                    guid.getAt(3),
                    guid.getAt(4),
                    guid.getAt(5),
                    guid.getAt(6),
                    guid.getAt(7));

            std::string str = std::string(buf);
#endif

            rv = true;
        }
    }

    return rv;

#else
    // cat /sys/class/net/eth0/address
    bool rv = true;
    struct ifreq s;
    int fd;

    // Clear the GUID
    guid.clear();

    fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (-1 == fd)
        return false;

    memset(&s, 0, sizeof(s));
    strcpy(s.ifr_name, "eth0");

    if (0 == ioctl(fd, SIOCGIFHWADDR, &s)) {

        // ptr = (unsigned char *)&s.ifr_ifru.ifru_hwaddr.sa_data[0];
        if (__VSCP_DEBUG_EXTRA) {
            syslog(LOG_DEBUG,
                   "Ethernet MAC address: %02X:%02X:%02X:%02X:%02X:%02X",
                   (uint8_t)s.ifr_addr.sa_data[0],
                   (uint8_t)s.ifr_addr.sa_data[1],
                   (uint8_t)s.ifr_addr.sa_data[2],
                   (uint8_t)s.ifr_addr.sa_data[3],
                   (uint8_t)s.ifr_addr.sa_data[4],
                   (uint8_t)s.ifr_addr.sa_data[5]);
        }

        guid.setAt(0, 0xff);
        guid.setAt(1, 0xff);
        guid.setAt(2, 0xff);
        guid.setAt(3, 0xff);
        guid.setAt(4, 0xff);
        guid.setAt(5, 0xff);
        guid.setAt(6, 0xff);
        guid.setAt(7, 0xfe);
        guid.setAt(8, s.ifr_addr.sa_data[0]);
        guid.setAt(9, s.ifr_addr.sa_data[1]);
        guid.setAt(10, s.ifr_addr.sa_data[2]);
        guid.setAt(11, s.ifr_addr.sa_data[3]);
        guid.setAt(12, s.ifr_addr.sa_data[4]);
        guid.setAt(13, s.ifr_addr.sa_data[5]);
        guid.setAt(14, 0);
        guid.setAt(15, 0);
    }
    else {
        syslog(LOG_ERR, "Failed to get hardware address (must be root?).");
        rv = false;
    }

    return rv;

#endif
}

///////////////////////////////////////////////////////////////////////////////
//  getIPAddress
//

bool
CControlObject::getIPAddress(cguid& guid)
{
    // Clear the GUID
    guid.clear();

    guid.setAt(0, 0xff);
    guid.setAt(1, 0xff);
    guid.setAt(2, 0xff);
    guid.setAt(3, 0xff);
    guid.setAt(4, 0xff);
    guid.setAt(5, 0xff);
    guid.setAt(6, 0xff);
    guid.setAt(7, 0xfd);

    char szName[128];
    gethostname(szName, sizeof(szName));
#if defined(_WIN32)
    LPHOSTENT lpLocalHostEntry;
#else
    struct hostent* lpLocalHostEntry;
#endif
    lpLocalHostEntry = gethostbyname(szName);
    if (NULL == lpLocalHostEntry) {
        return false;
    }

    // Get all local addresses
    int idx = -1;
    void* pAddr;
    unsigned long localaddr[16]; // max 16 local addresses
    do {
        idx++;
        localaddr[idx] = 0;
        pAddr          = lpLocalHostEntry->h_addr_list[idx];
        if (NULL != pAddr)
            localaddr[idx] = *((unsigned long*)pAddr);
    } while ((NULL != pAddr) && (idx < 16));

    guid.setAt(8, (localaddr[0] >> 24) & 0xff);
    guid.setAt(9, (localaddr[0] >> 16) & 0xff);
    guid.setAt(10, (localaddr[0] >> 8) & 0xff);
    guid.setAt(11, localaddr[0] & 0xff);

    guid.setAt(12, 0);
    guid.setAt(13, 0);
    guid.setAt(14, 0);
    guid.setAt(15, 0);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getSystemKey
//

uint8_t*
CControlObject::getSystemKey(uint8_t* pKey)
{
    if (NULL != pKey) {
        memcpy(pKey, m_systemKey, 32);
    }

    return m_systemKey;
}

///////////////////////////////////////////////////////////////////////////////
// getSystemKeyMD5
//

void
CControlObject::getSystemKeyMD5(std::string& strKey)
{
    char digest[33];
    vscp_md5(digest, m_systemKey, 32);
    strKey = digest;
}

// ----------------------------------------------------------------------------
// FULL XML configuration callbacks
// ----------------------------------------------------------------------------

static int depth_full_config_parser = 0;
static int bVscpConfigFound         = 0;
static int bGeneralConfigFound      = 0;
static int bRemoteUserConfigFound   = 0;
static int bUDPConfigFound          = 0;
static int bMQTTConfigFound         = 0;
static int bLevel1DriverConfigFound = 0;
static int bLevel2DriverConfigFound = 0;
static int bLevel3DriverConfigFound = 0;

static void
startFullConfigParser(void* data, const char* name, const char** attr)
{
    CControlObject* pObj = (CControlObject*)data;
    if (NULL == data)
        return;

    // fprintf(stderr, "%s\n", name);

    if ((0 == depth_full_config_parser) &&
        (0 == vscp_strcasecmp(name, "vscpconfig"))) {
        bVscpConfigFound = TRUE;
    }
    else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "general"))) {
        bGeneralConfigFound = TRUE;

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "clientbuffersize")) {
                pObj->m_maxItemsInClientReceiveQueue =
                  vscp_readStringValue(attribute);
            }
            else if (0 == vscp_strcasecmp(attr[i], "runasuser")) {
                vscp_trim(attribute);
                pObj->m_runAsUser = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "guid")) {
                pObj->m_guid.getFromString(attribute);
            }
            else if (0 == vscp_strcasecmp(attr[i], "servername")) {
                pObj->m_strServerName = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "webadminif")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_enableWebAdminIf = true;
                }
                else {
                    pObj->m_enableWebAdminIf = false;
                }
            }
        }
    }
    else if (bVscpConfigFound && bGeneralConfigFound &&
             (2 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "debug"))) {

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "byte1")) {
                if (attribute.length()) {
                    pObj->m_debugFlags[0] = vscp_readStringValue(attribute);
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "byte2")) {
                if (attribute.length()) {
                    pObj->m_debugFlags[1] = vscp_readStringValue(attribute);
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "byte3")) {
                if (attribute.length()) {
                    pObj->m_debugFlags[2] = vscp_readStringValue(attribute);
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "byte4")) {
                if (attribute.length()) {
                    pObj->m_debugFlags[3] = vscp_readStringValue(attribute);
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "byte5")) {
                if (attribute.length()) {
                    pObj->m_debugFlags[4] = vscp_readStringValue(attribute);
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "byte6")) {
                if (attribute.length()) {
                    pObj->m_debugFlags[5] = vscp_readStringValue(attribute);
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "byte7")) {
                if (attribute.length()) {
                    pObj->m_debugFlags[6] = vscp_readStringValue(attribute);
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "byte8")) {
                if (attribute.length()) {
                    pObj->m_debugFlags[7] = vscp_readStringValue(attribute);
                }
            }
        }
    }
    else if (bVscpConfigFound && bGeneralConfigFound &&
             (2 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "security"))) {

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "admin")) {
                pObj->m_admin_user = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "password")) {
                pObj->m_admin_password = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "allowfrom")) {
                pObj->m_admin_allowfrom = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "vscptoken")) {
                pObj->m_vscptoken = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "vscpkey")) {
                if (attribute.length()) {
                    vscp_hexStr2ByteArray(pObj->m_systemKey,
                                          32,
                                          attribute.c_str());
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "authentication_domain")) {
                if (attribute.length()) {
                    pObj->m_web_authentication_domain = attribute;
                }
            }
        }
    }
    else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "tcpip"))) {

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "enable")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_enableTcpip = true;
                }
                else {
                    pObj->m_enableTcpip = false;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "interface")) {
                vscp_startsWith(attribute, "tcp://", &attribute);
                vscp_trim(attribute);
                pObj->m_strTcpInterfaceAddress = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "ssl_certificate")) {
                pObj->m_tcpip_ssl_certificate = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "ssl_verify_peer")) {
                pObj->m_tcpip_ssl_verify_peer = vscp_readStringValue(attribute);
            }
            else if (0 == vscp_strcasecmp(attr[i], "ssl_certificate_chain")) {
                pObj->m_tcpip_ssl_certificate_chain = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "ssl_ca_path")) {
                pObj->m_tcpip_ssl_ca_path = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "ssl_ca_file")) {
                pObj->m_tcpip_ssl_ca_file = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "ssl_verify_depth")) {
                pObj->m_tcpip_ssl_verify_depth =
                  vscp_readStringValue(attribute);
            }
            else if (0 ==
                     vscp_strcasecmp(attr[i], "ssl_default_verify_paths")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_tcpip_ssl_default_verify_paths = true;
                }
                else {
                    pObj->m_tcpip_ssl_default_verify_paths = false;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "ssl_cipher_list")) {
                pObj->m_tcpip_ssl_cipher_list = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "ssl_protocol_version")) {
                pObj->m_tcpip_ssl_verify_depth =
                  vscp_readStringValue(attribute);
            }
            else if (0 == vscp_strcasecmp(attr[i], "ssl_short_trust")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_tcpip_ssl_short_trust = true;
                }
                else {
                    pObj->m_tcpip_ssl_short_trust = false;
                }
            }
        }
    }

    else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "webserver"))) {

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "enable")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_web_bEnable = true;
                }
                else {
                    pObj->m_web_bEnable = false;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "document_root")) {
                if (attribute.length()) {
                    pObj->m_web_document_root = attribute;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "listening_ports")) {
                if (attribute.length()) {
                    pObj->m_web_listening_ports = attribute;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "index_files")) {
                if (attribute.length()) {
                    pObj->m_web_index_files = vscp_trim_copy(attribute);
                    ;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "authentication_domain")) {
                if (attribute.length()) {
                    pObj->m_web_authentication_domain = attribute;
                }
            }
            else if (0 ==
                     vscp_strcasecmp(attr[i], "enable_auth_domain_check")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_enable_auth_domain_check = true;
                }
                else {
                    pObj->m_enable_auth_domain_check = false;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "ssl_certificat")) {
                if (attribute.length()) {
                    pObj->m_web_ssl_certificate = attribute;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "ssl_certificat_chain")) {
                if (attribute.length()) {
                    pObj->m_web_ssl_certificate_chain = attribute;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "ssl_verify_peer")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_web_ssl_verify_peer = true;
                }
                else {
                    pObj->m_web_ssl_verify_peer = false;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "ssl_ca_path")) {
                if (attribute.length()) {
                    pObj->m_web_ssl_ca_path = attribute;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "ssl_ca_file")) {
                if (attribute.length()) {
                    pObj->m_web_ssl_ca_file = attribute;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "ssl_verify_depth")) {
                if (attribute.length()) {
                    pObj->m_web_ssl_verify_depth =
                      vscp_readStringValue(attribute);
                }
            }
            else if (0 ==
                     vscp_strcasecmp(attr[i], "ssl_default_verify_paths")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_web_ssl_default_verify_paths = true;
                }
                else {
                    pObj->m_web_ssl_default_verify_paths = false;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "ssl_cipher_list")) {
                if (attribute.length()) {
                    pObj->m_web_ssl_cipher_list = attribute;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "ssl_protcol_version")) {
                if (attribute.length()) {
                    pObj->m_web_ssl_protocol_version =
                      vscp_readStringValue(attribute);
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "ssl_short_trust")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_web_ssl_short_trust = true;
                }
                else {
                    pObj->m_web_ssl_short_trust = false;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "cgi_interpreter")) {
                if (attribute.length()) {
                    pObj->m_web_cgi_interpreter = attribute;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "cgi_pattern")) {
                if (attribute.length()) {
                    pObj->m_web_cgi_patterns = attribute;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "cgi_environment")) {
                if (attribute.length()) {
                    pObj->m_web_cgi_environment = attribute;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "protect_uri")) {
                if (attribute.length()) {
                    pObj->m_web_protect_uri = attribute;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "trottle")) {
                if (attribute.length()) {
                    pObj->m_web_trottle = attribute;
                }
            }
            else if (0 ==
                     vscp_strcasecmp(attr[i], "enable_directory_listing")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_web_enable_directory_listing = true;
                }
                else {
                    pObj->m_web_enable_directory_listing = false;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "enable_keep_alive")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_web_enable_keep_alive = true;
                }
                else {
                    pObj->m_web_enable_keep_alive = false;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "keep_alive_timeout_ms")) {
                if (attribute.length()) {
                    pObj->m_web_keep_alive_timeout_ms =
                      vscp_readStringValue(attribute);
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "access_control_list")) {
                if (attribute.length()) {
                    pObj->m_web_access_control_list = attribute;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "extra_mime_types")) {
                if (attribute.length()) {
                    pObj->m_web_extra_mime_types = attribute;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "num_threads")) {
                if (attribute.length()) {
                    pObj->m_web_num_threads = vscp_readStringValue(attribute);
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "hide_file_pattern")) {
                if (attribute.length()) {
                    pObj->m_web_hide_file_patterns = attribute;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "url_rewrite_patterns")) {
                if (attribute.length()) {
                    pObj->m_web_url_rewrite_patterns = attribute;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "hide_file_patterns")) {
                if (attribute.length()) {
                    pObj->m_web_hide_file_patterns = attribute;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "request_timeout_ms")) {
                if (attribute.length()) {
                    pObj->m_web_request_timeout_ms =
                      vscp_readStringValue(attribute);
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "linger_timeout_ms")) {
                if (attribute.length()) {
                    pObj->m_web_linger_timeout_ms =
                      vscp_readStringValue(attribute);
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "decode_url")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_web_decode_url = true;
                }
                else {
                    pObj->m_web_decode_url = false;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "global_auth_file")) {
                if (attribute.length()) {
                    pObj->m_web_global_auth_file = attribute;
                }
            }
            else if (0 ==
                     vscp_strcasecmp(attr[i], "web_per_directory_auth_file")) {
                if (attribute.length()) {
                    pObj->m_web_per_directory_auth_file = attribute;
                }
            }
            else if (0 ==
                     vscp_strcasecmp(attr[i], "access_control_allow_origin")) {
                if (attribute.length()) {
                    pObj->m_web_access_control_allow_methods = attribute;
                }
            }
            else if (0 ==
                     vscp_strcasecmp(attr[i], "access_control_allow_methods")) {
                if (attribute.length()) {
                    pObj->m_web_access_control_allow_methods = attribute;
                }
            }
            else if (0 ==
                     vscp_strcasecmp(attr[i], "access_control_allow_headers")) {
                if (attribute.length()) {
                    pObj->m_web_access_control_allow_headers = attribute;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "error_pages")) {
                if (attribute.length()) {
                    pObj->m_web_error_pages = attribute;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "tcp_nodelay")) {
                if (attribute.length()) {
                    pObj->m_web_linger_timeout_ms =
                      vscp_readStringValue(attribute);
                }
            }
            else if (0 ==
                     vscp_strcasecmp(attr[i], "static_file_cache_control")) {
                if (attribute.length()) {
                    pObj->m_web_static_file_cache_control = attribute;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "static_file_max_age")) {
                if (attribute.length()) {
                    pObj->m_web_static_file_max_age =
                      vscp_readStringValue(attribute);
                }
            }
            else if (0 ==
                     vscp_strcasecmp(attr[i],
                                     "strict_transport_security_max_age")) {
                if (attribute.length()) {
                    pObj->m_web_strict_transport_security_max_age =
                      vscp_readStringValue(attribute);
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "sendfile_call")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_web_allow_sendfile_call = true;
                }
                else {
                    pObj->m_web_allow_sendfile_call = false;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "additional_headers")) {
                if (attribute.length()) {
                    pObj->m_web_additional_header = attribute;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "max_request_size")) {
                if (attribute.length()) {
                    pObj->m_web_max_request_size =
                      vscp_readStringValue(attribute);
                }
            }
            else if (0 == vscp_strcasecmp(attr[i],
                                          "web_allow_index_script_resource")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_web_allow_index_script_resource = true;
                }
                else {
                    pObj->m_web_allow_index_script_resource = false;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "duktape_script_patterns")) {
                if (attribute.length()) {
                    pObj->m_web_duktape_script_patterns = attribute;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "lua_preload_file")) {
                if (attribute.length()) {
                    pObj->m_web_lua_preload_file = attribute;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "lua_script_patterns")) {
                if (attribute.length()) {
                    pObj->m_web_lua_script_patterns = attribute;
                }
            }
            else if (0 ==
                     vscp_strcasecmp(attr[i], "lua_server_page_patterns")) {
                if (attribute.length()) {
                    pObj->m_web_lua_server_page_patterns = attribute;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "lua_websockets_patterns")) {
                if (attribute.length()) {
                    pObj->m_web_lua_websocket_patterns = attribute;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "lua_background_script")) {
                if (attribute.length()) {
                    pObj->m_web_lua_background_script = attribute;
                }
            }
            else if (0 ==
                     vscp_strcasecmp(attr[i], "lua_background_script_params")) {
                if (attribute.length()) {
                    pObj->m_web_lua_background_script_params = attribute;
                }
            }
        }
    }
    else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "restapi"))) {

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "enable")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_bEnableRestApi = true;
                }
                else {
                    pObj->m_bEnableRestApi = false;
                }
            }
        }
    }
    else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "websockets"))) {

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "enable")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_bWebsocketsEnable = true;
                }
                else {
                    pObj->m_bWebsocketsEnable = false;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "document_root")) {
                if (attribute.length()) {
                    pObj->m_websocket_document_root = attribute;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "timeout_ms")) {
                if (attribute.length()) {
                    pObj->m_websocket_timeout_ms =
                      vscp_readStringValue(attribute);
                }
            }
            else if (0 ==
                     vscp_strcasecmp(attr[i], "enable_websocket_ping_pong")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->bEnable_websocket_ping_pong = true;
                }
                else {
                    pObj->bEnable_websocket_ping_pong = false;
                }
            }
            else if (0 ==
                     vscp_strcasecmp(attr[i], "web-lua_websocket_pattern")) {
                if (attribute.length()) {
                    pObj->lua_websocket_pattern = attribute;
                }
            }
        }
    }
    else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "remoteuser"))) {
        bRemoteUserConfigFound = TRUE;
    }
    else if (bVscpConfigFound && bRemoteUserConfigFound &&
             (2 == depth_full_config_parser) /*&&
             (0 == vscp_strcasecmp(name, "user"))*/) {

        vscpEventFilter VSCPFilter;
        bool bFilterPresent = false;
        bool bMaskPresent   = false;
        std::string name;
        std::string md5;
        std::string privilege;
        std::string allowfrom;
        std::string allowevent;
        std::string fullname;
        std::string note;

        vscp_clearVSCPFilter(&VSCPFilter); // Allow all frames

        for (int i=0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "name")) {
                name = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "password")) {
                md5 = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "fullname")) {
                fullname = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "note")) {
                note = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "privilege")) {
                privilege = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "allowfrom")) {
                allowfrom = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "allowevent")) {
                allowevent = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "filter")) {
                if (attribute.length()) {
                    if (vscp_readFilterFromString(&VSCPFilter, attribute)) {
                        bFilterPresent = true;
                    }
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "mask")) {
                if (attribute.length()) {
                    if (vscp_readMaskFromString(&VSCPFilter, attribute)) {
                        bMaskPresent = true;
                    }
                }
            }            
        }

        if (bFilterPresent && bMaskPresent) {
            pObj->m_userList.addUser(name,
                                        md5,
                                        fullname,
                                        note,
                                        pObj->m_web_authentication_domain,
                                        &VSCPFilter,
                                        privilege,
                                        allowfrom,
                                        allowevent,
                                        0);
        }
        else {
            pObj->m_userList.addUser(name,
                                        md5,
                                        fullname,
                                        note,
                                        pObj->m_web_authentication_domain,
                                        NULL,
                                        privilege,
                                        allowfrom,
                                        allowevent,
                                        0);
        }
    }
    else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "udp"))) {
        
        bVscpConfigFound = TRUE;

        for (int i=0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "enable")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_udpsrv.m_bEnable = true;
                }
                else {
                    pObj->m_udpsrv.m_bEnable = false;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "interface")) {
                vscp_startsWith(attribute, "udp://", &attribute);
                vscp_trim(attribute);
                pObj->m_udpsrv.m_interface = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "guid")) {
                pObj->m_udpsrv.m_guid.getFromString(attribute);
            }
            else if (0 == vscp_strcasecmp(attr[i], "user")) {
                pObj->m_udpsrv.m_user = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "password")) {
                pObj->m_udpsrv.m_password = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "allowunsecure")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_udpsrv.m_bAllowUnsecure = true;
                }
                else {
                    pObj->m_udpsrv.m_bAllowUnsecure = false;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "back")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_udpsrv.m_bAck = true;
                }
                else {
                    pObj->m_udpsrv.m_bAck = false;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "filter")) {
                vscp_readFilterFromString(&pObj->m_udpsrv.m_filter,attribute);
            }
            else if (0 == vscp_strcasecmp(attr[i], "mask")) {
                vscp_readMaskFromString(&pObj->m_udpsrv.m_filter,attribute);
            }
            
        }
    }
    else if (bVscpConfigFound && bUDPConfigFound &&
             (2 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "client"))) {

        bool bEnable = false;
        std::string interface;
        std::string user;
        std::string password;
        vscpEventFilter filter;
        int nEncryption = 0;
        bool bSetBroadcast = false;

        for (int i=0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "enable")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    bEnable = true;
                }
                else {
                    bEnable = false;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "interface")) {
                vscp_startsWith(attribute, "udp://", &attribute);
                vscp_trim(attribute);
                interface = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "user")) {
                user = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "password")) {
                password = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "filter")) {
                vscp_readFilterFromString(&filter,attribute);
            }
            else if (0 == vscp_strcasecmp(attr[i], "mask")) {
                vscp_readMaskFromString(&filter,attribute);
            }
            else if (0 == vscp_strcasecmp(attr[i], "Setbroadcast")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    bSetBroadcast = true;
                }
                else {
                    bSetBroadcast = false;
                }
            }

        }

        // Add UDP client if enabled
        pObj->addUdpClient(bEnable,
                            interface,
                            user,
                            password,
                            filter,
                            nEncryption,
                            bSetBroadcast);
        
    }
    else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "mqtt"))) {
        
        bVscpConfigFound = TRUE;

        for (int i=0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "enable")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_enableMqtt = true;
                }
                else {
                    pObj->m_enableMqtt = false;
                }
            }            
            
        }
    }
    else if (bVscpConfigFound && bMQTTConfigFound &&
             (2 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "client"))) {

        bool bEnable = false;
        std::string interface = "localhost:1883";
        std::string user = "mqtt";
        std::string password = "secret";
        vscpEventFilter filter;
        memset(&filter,0,sizeof(vscpEventFilter));
        cguid guid;
        int qos = 0;
        bool bCleanSession = false;
        bool bRetain = false;
        int keepalive = -1;
        std::string topic_subscribe = "vscp/#";
        std::string topic_publish = "vscp/%guid%/%class%/%type%/";
        std::string cafile = "";
    	std::string capath = "";
    	std::string certfile = "";
    	std::string keyfile = "";
    	std::string xpassword = "";

        for (int i=0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "enable")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    bEnable = true;
                }
                else {
                    bEnable = false;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "interface")) {
                vscp_startsWith(attribute, "tcp://", &attribute);
                vscp_startsWith(attribute, "stcp://", &attribute);
                vscp_trim(attribute);
                pObj->m_udpsrv.m_interface = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "user")) {
                user = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "password")) {
                password = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "filter")) {
                vscp_readFilterFromString(&filter,attribute);
            }
            else if (0 == vscp_strcasecmp(attr[i], "mask")) {
                vscp_readMaskFromString(&filter,attribute);
            }
            else if (0 == vscp_strcasecmp(attr[i], "qos")) {
                qos = vscp_readStringValue(attribute);
                if (qos>3) {
                    syslog(LOG_ERR,"Config: MQTT qos > 3. Set to 0.");
                    qos = 0;                    
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "guid")) {
                guid.getFromString(attribute);
            }
            else if (0 == vscp_strcasecmp(attr[i], "bcleansession")) {  // bCleanSession
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    bCleanSession = true;
                }
                else {
                    bCleanSession = false;
                }
            }                        
            else if (0 == vscp_strcasecmp(attr[i], "bretain")) {        // bRetain
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    bRetain = true;
                }
                else {
                    bRetain = false;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "keepalive")) {   
                keepalive = vscp_readStringValue(attribute);
            }
            else if (0 == vscp_strcasecmp(attr[i], "topic-subscribe")) {
                topic_subscribe = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "topic-publish")) {
                topic_publish = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "cafile")) {
                cafile = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "capath")) {
                capath = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "certfile")) {
                certfile = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "keyfile")) {
                keyfile = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "xpassword")) {
                xpassword = attribute;
            }

        }

        if (bEnable) {
            pObj->addMqttClient(interface,
			                        user,
			                        password,
			                        filter,
			                        guid,
			                        qos,
			                        bCleanSession,
			                        bRetain,
			                        keepalive,
			                        topic_subscribe,
			                        topic_publish,
			                        cafile,
			                        capath,
			                        certfile,
			                        keyfile,
			                        xpassword );
        }
        
    }
    else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
             ((0 == vscp_strcasecmp(name, "level1driver")) ||
              (0 == vscp_strcasecmp(name, "canal1driver")))) {
        bLevel1DriverConfigFound = TRUE;
    }
    else if (bVscpConfigFound && bLevel1DriverConfigFound &&
             (2 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "driver"))) {

        std::string strName;
        std::string strConfig;
        std::string strPath;
        unsigned long flags  = 0;
        uint32_t translation = 0;
        cguid guid;
        bool bEnabled = false;

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "enable")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    bEnabled = true;
                }
                else {
                    bEnabled = false;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "name")) {
                strName = attribute;
                // Replace possible spaces in name with underscore
                std::string::size_type found;
                while (std::string::npos !=
                       (found = strName.find_first_of(" "))) {
                    strName[found] = '_';
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "config")) {
                strConfig = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i],
                                          "parameter")) { // deprecated
                strConfig = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "path")) {
                strPath = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "flags")) {
                flags = vscp_readStringValue(attribute);
            }
            else if (0 == vscp_strcasecmp(attr[i], "guid")) {
                guid.getFromString(attribute);
            }
            else if (0 == vscp_strcasecmp(attr[i], "translation")) {
                translation = vscp_readStringValue(attribute);
            }
        } // for

        if (bEnabled) {
            // Add the level I device
            if (!pObj->m_deviceList.addItem(strName,
                                            strConfig,
                                            strPath,
                                            flags,
                                            guid,
                                            VSCP_DRIVER_LEVEL1,
                                            bEnabled,
                                            translation)) {
                syslog(LOG_ERR,
                       "Level I driver not added name=%s. "
                       "Path does not exist. - [%s]",
                       strName.c_str(),
                       strPath.c_str());
            }
            else {
                if (__VSCP_DEBUG_DRIVER1) {
                    syslog(LOG_DEBUG,
                           "Level I driver added. name = %s - [%s]",
                           strName.c_str(),
                           strPath.c_str());
                }
            }
        }
    }
    else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
             ((0 == vscp_strcasecmp(name, "level2driver")))) {
        bLevel2DriverConfigFound = TRUE;
    }
    else if (bVscpConfigFound && bLevel2DriverConfigFound &&
             (2 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "driver"))) {

        std::string strName;
        std::string strConfig;
        std::string strPath;
        cguid guid;
        bool bEnabled = false;

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "enable")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    bEnabled = true;
                }
                else {
                    bEnabled = false;
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "name")) {
                strName = attribute;
                // Replace spaces in name with underscore
                std::string::size_type found;
                while (std::string::npos !=
                       (found = strName.find_first_of(" "))) {
                    strName[found] = '_';
                }
            }
            else if (0 == vscp_strcasecmp(attr[i], "path-config")) {
                strConfig = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i],
                                          "parameter")) { // deprecated
                strConfig = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "path-driver")) {
                strPath = attribute;
            }
            else if (0 == vscp_strcasecmp(attr[i], "guid")) {
                guid.getFromString(attribute);
            }
        } // for

        // Add the level II device
        if (bEnabled) {
            if (!pObj->m_deviceList.addItem(strName,
                                            strConfig,
                                            strPath,
                                            0,
                                            guid,
                                            VSCP_DRIVER_LEVEL2,
                                            bEnabled)) {
                if (__VSCP_DEBUG_DRIVER2) {
                    syslog(LOG_ERR,
                           "Level II driver was not added. name = %s"
                           "Path does not exist. - [%s]",
                           strName.c_str(),
                           strPath.c_str());
                }
            }
            else {
                if (__VSCP_DEBUG_DRIVER2) {
                    syslog(LOG_DEBUG,
                           "Level II driver added. name = %s- [%s]",
                           strName.c_str(),
                           strPath.c_str());
                }
            }
        }
    }

    depth_full_config_parser++;
}

static void
handleFullConfigData(void* data, const char* content, int length)
{
}

static void
endFullConfigParser(void* data, const char* name)
{
    depth_full_config_parser--;

    if (1 == depth_full_config_parser &&
        (0 == vscp_strcasecmp(name, "vscpconfig"))) {
        bVscpConfigFound = FALSE;
    }
    if (bVscpConfigFound && (1 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "remoteuser"))) {
        bRemoteUserConfigFound = FALSE;
    }
    else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "udp"))) {
        bUDPConfigFound = FALSE;
    }
    else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "mqtt"))) {
        bMQTTConfigFound = FALSE;
    }
    else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
        ((0 == vscp_strcasecmp(name, "level1driver")) ||
         (0 == vscp_strcasecmp(name, "canal1driver")))) {
        bLevel1DriverConfigFound = FALSE;
    }
    else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "level2driver"))) {
        bLevel2DriverConfigFound = FALSE;
    }
    else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
             (0 == vscp_strcasecmp(name, "level3driver"))) {
        bLevel3DriverConfigFound = FALSE;
    }
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// readConfiguration
//
// Read the configuration XML file
//

bool
CControlObject::readConfiguration(const std::string& strcfgfile)
{
    FILE* fp;

    if (__VSCP_DEBUG_EXTRA) {
        syslog(LOG_DEBUG,
               "Reading full XML configuration from [%s]",
               (const char*)strcfgfile.c_str());
    }

    fp = fopen(strcfgfile.c_str(), "r");
    if (NULL == fp) {
        syslog(LOG_ERR,
               "Failed to open configuration file [%s]",
               strcfgfile.c_str());
        return false;
    }

    XML_Parser xmlParser = XML_ParserCreate("UTF-8");
    XML_SetUserData(xmlParser, this);
    XML_SetElementHandler(xmlParser,
                          startFullConfigParser,
                          endFullConfigParser);
    XML_SetCharacterDataHandler(xmlParser, handleFullConfigData);

    void* buf = XML_GetBuffer(xmlParser, XML_BUFF_SIZE);
    if (NULL == buf) {
        XML_ParserFree(xmlParser);
        fclose(fp);
        syslog(LOG_ERR,
               "Failed to allocate buffer for configuration file [%s]",
               strcfgfile.c_str());
        return false;
    }

    size_t file_size = 0;
    file_size        = fread(buf, sizeof(char), XML_BUFF_SIZE, fp);

    if (!XML_ParseBuffer(xmlParser, file_size, file_size == 0)) {
        syslog(LOG_ERR, "Failed parse XML configuration file.");
        fclose(fp);
        XML_ParserFree(xmlParser);
        return false;
    }

    fclose(fp);
    XML_ParserFree(xmlParser);

    return true;
} // XML config

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// clientMsgWorkerThread
//
// Is there any messages to send from Level II clients. Send it/them to all
// devices/clients except for itself.
//

void*
clientMsgWorkerThread(void* userdata)
{
    std::list<vscpEvent*>::iterator it;
    vscpEvent* pvscpEvent = NULL;

    // Must be a valid control object pointer
    CControlObject* pObj = (CControlObject*)userdata;
    if (NULL == pObj)
        return NULL;

    while (!pObj->m_bQuit_clientMsgWorkerThread) {

        // Wait for event
        if ((-1 == vscp_sem_wait(&pObj->m_semClientOutputQueue, 10)) &&
            errno == ETIMEDOUT) {
            continue;
        }

        if (pObj->m_clientOutputQueue.size()) {

            pthread_mutex_lock(&pObj->m_mutex_ClientOutputQueue);
            pvscpEvent = pObj->m_clientOutputQueue.front();
            pObj->m_clientOutputQueue.pop_front();
            pthread_mutex_unlock(&pObj->m_mutex_ClientOutputQueue);

            if (NULL != pvscpEvent) {

                // * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
                //
                // Send event to all Level II clients (not to
                // ourself )
                //
                // * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

                pObj->sendEventAllClients(pvscpEvent, pvscpEvent->obid);
                // Tell main thread that there are work to do
                sem_post(&pObj->m_semSentToAllClients);

            } // Valid event

            // Delete the event - we are done with it
            vscp_deleteEvent_v2(&pvscpEvent);

        } // Events in queue

    } // while

    return NULL;
}
