// ControlObject.cpp: m_path_db_vscp_logimplementation of the CControlObject
// class.
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
//

#define _POSIX

#include <deque>
#include <list>
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

#include "web_css.h"
#include "web_js.h"
#include "web_template.h"

#include <expat.h>
#include <sqlite3.h>

#include <fastpbkdf2.h>
#include <vscp_aes.h>

#include <actioncodes.h>
#include <canal_macro.h>
#include <configfile.h>
#include <crc.h>
#include <devicelist.h>
#include <devicethread.h>
#include <dm.h>
#include <httpd.h>
#include <randpassword.h>
#include <tables.h>
#include <variablecodes.h>
#include <version.h>
#include <vscp.h>
#include <vscp_debug.h>
#include <vscpd_caps.h>
#include <vscpdb.h>
#include <vscpeventhelper.h>
#include <vscphelper.h>
#include <vscpmd5.h>
#include <websocket.h>
#include <websrv.h>

#include <controlobject.h>

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#define XML_BUFF_SIZE 0xffff

// Prototypes
void
createFolderStuct(std::string &rootFolder); // from vscpd.cpp

void *
clientMsgWorkerThread(void *userdata); // this
void *
tcpipListenThread(void *pData); // tcpipsev.cpp
void *
UDPThread(void *pData); // udpsrv.cpp

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CControlObject::CControlObject()
{
    int i;

    m_bQuit = false; // true  for app termination
    m_bQuit_clientMsgWorkerThread =
      false; // true for clientWorkerThread termination

    // Debug flags
    // m_debugFlags[0] = VSCP_DEBUG1_ALL;
    m_debugFlags[0] = 0;
    // m_debugFlags[0] |= VSCP_DEBUG1_DM;
    // m_debugFlags[0] |= VSCP_DEBUG1_AUTOMATION;
    // m_debugFlags[0] |= VSCP_DEBUG1_VARIABLE;
    // m_debugFlags[0] |= VSCP_DEBUG1_MULTICAST;
    // m_debugFlags[0] |= VSCP_DEBUG1_UDP;
    // m_debugFlags[0] |= VSCP_DEBUG1_TCP;
    // m_debugFlags[0] |= VSCP_DEBUG1_DRIVER

    m_rootFolder = "/srv/vscp/";

    // Default admin user credentials
    m_admin_user     = "admin";
    m_admin_password = "450ADCE88F2FDBB20F3318B65E53CA4A;"
                       "06D3311CC2195E80BE4F8EB12931BFEB5C"
                       "630F6B154B2D644ABE29CEBDBFB545";
    m_admin_allowfrom = "*";
    m_vscptoken       = "Carpe diem quam minimum credula postero";
    vscp_hexStr2ByteArray(m_systemKey,
                          32,
                          "A4A86F7D7E119BA3F0CD06881E371B989B"
                          "33B6D606A863B633EF529D64544F8E");

    m_nConfiguration = 1; // Default configuration record is read.

    m_automation.setControlObject(this);
    m_maxItemsInClientReceiveQueue = MAX_ITEMS_CLIENT_RECEIVE_QUEUE;

    // Nill the GUID
    m_guid.clear();

    // Local domain
    m_web_authentication_domain = "mydomain.com";

    // Set Default Log Level
    m_logLevel = DAEMON_LOGMSG_NORMAL;

    m_path_db_vscp_daemon = m_rootFolder + "vscpd.sqlite3";
    m_path_db_vscp_data   = m_rootFolder + "vscp_data.sqlite3";

    // No databases opened yet
    m_db_vscp_daemon = NULL;
    m_db_vscp_data   = NULL;

    // Control UDP Interface
    m_udpSrvObj.setControlObjectPointer(this);
    m_udpSrvObj.m_bEnable = false;
    m_udpSrvObj.m_interface.empty();
    m_udpSrvObj.m_guid.clear();
    vscp_clearVSCPFilter(&m_udpSrvObj.m_filter);
    m_udpSrvObj.m_bAllowUnsecure = false;
    m_udpSrvObj.m_bAck           = false;

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

    // Default multicast announce port
    m_strMulticastAnnounceAddress =
      vscp_str_format("udp://:%d", VSCP_ANNOUNCE_MULTICAST_PORT);

    // default multicast announce ttl
    m_ttlMultiCastAnnounce = IP_MULTICAST_DEFAULT_TTL;

    // Default UDP interface
    m_udpSrvObj.m_interface =
      vscp_str_format("udp://:%d", VSCP_DEFAULT_UDP_PORT);

    // Web server SSL settings
    m_web_ssl_certificate          = m_rootFolder + "certs/server.pem";
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

    m_web_document_root = m_rootFolder + "www";

    // Directory listings on by default
    m_web_enable_directory_listing          = true;
    m_web_enable_keep_alive                 = false;
    m_web_keep_alive_timeout_ms             = 0;
    m_web_access_control_list               = "";
    m_web_extra_mime_types                  = "";
    m_web_num_threads                       = 50;
    m_web_run_as_user                       = "";
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

    // Init. web server subsystem - All features enabled
    // ssl mt locks will we initiated here for openssl 1.0
    if (0 == web_init(0xffff)) {
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
    syslog(LOG_INFO, "ControlObject: Going away...");

    // Remove objects in Client send queue
    std::list<vscpEvent *>::iterator iterVSCP;

    pthread_mutex_lock(&m_mutexClientOutputQueue);
    for (iterVSCP = m_clientOutputQueue.begin();
         iterVSCP != m_clientOutputQueue.end();
         ++iterVSCP) {
        vscpEvent *pEvent = *iterVSCP;
        vscp_deleteVSCPevent(pEvent);
    }

    m_clientOutputQueue.clear();
    pthread_mutex_unlock(&m_mutexClientOutputQueue);

    pthread_mutex_lock(&m_udpSrvObj.m_mutexUDPInfo);
    std::deque<udpRemoteClientInfo *>::iterator iterUDP;
    for (iterUDP = m_udpSrvObj.m_remotes.begin();
         iterUDP != m_udpSrvObj.m_remotes.end();
         ++iterUDP) {
        if (NULL != *iterUDP) {
            delete *iterUDP;
            *iterUDP = NULL;
        }
    }
    m_udpSrvObj.m_remotes.clear();
    pthread_mutex_unlock(&m_udpSrvObj.m_mutexUDPInfo);

    syslog(LOG_INFO, "ControlObject: Gone!");
}

/////////////////////////////////////////////////////////////////////////////
// init
//

bool
CControlObject::init(std::string &strcfgfile, std::string &rootFolder)
{
    std::string str;

    // Save root folder for later use.
    m_rootFolder = rootFolder;

    // Root folder must exist
    if (!vscp_fileExists(m_rootFolder.c_str())) {
        syslog(LOG_CRIT,
               "The specified rootfolder does not exist (%s).",
               (const char *)m_rootFolder.c_str());
        return false;
    }

    m_path_db_vscp_daemon  = m_rootFolder + "vscpd.sqlite3";
    m_path_db_vscp_data    = m_rootFolder + "vscp_data.sqlite3";
    std::string strRootwww = m_rootFolder + "www";
    m_web_document_root    = strRootwww;

    // Change locale to get the correct decimal point "."
    setlocale(LC_NUMERIC, "C");

    // A configuration file must be available
    if (!vscp_fileExists(strcfgfile.c_str())) {
        printf("No configuration file. Can't initialize!.");
        syslog(LOG_CRIT,
               "No configuration file. Can't initialize!. Path=%s",
               strcfgfile.c_str());
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////
    //                  Read XML configuration GENERAL section
    ////////////////////////////////////////////////////////////////////////////

    // Read XML configuration
    if (!readXMLConfigurationGeneral(strcfgfile)) {
        syslog(LOG_CRIT,
               "General: Unable to open/parse configuration file [%s]. Can't "
               "initialize!",
               strcfgfile.c_str());
        return FALSE;
    }

#ifndef WIN32
    if (m_runAsUser.length()) {
        struct passwd *pw;
        if (NULL == (pw = getpwnam(m_runAsUser.c_str()))) {
            syslog(LOG_ERR, "Unknown user.");
        } else if (setgid(pw->pw_gid) != 0) {
            syslog(LOG_ERR, "setgid() failed.");
        } else if (setuid(pw->pw_uid) != 0) {
            syslog(LOG_ERR, "setuid() failed.");
        }
    }
#endif

    // Initialize the SQLite library
    if (SQLITE_OK != sqlite3_initialize()) {
        syslog(LOG_CRIT, "Unable to initialize SQLite library!.");
        return false;
    }

    // The root folder is the basis for the configuration file
    m_path_db_vscp_daemon = m_rootFolder + "/vscpd.sqlite3";

    // ======================================
    // * * * Open/Create database files * * *
    // ======================================

    // * * * VSCP Daemon configuration database * * *

    // Check filename
    if (vscp_fileExists(m_path_db_vscp_daemon)) {

        if (SQLITE_OK !=
            sqlite3_open((const char *)m_path_db_vscp_daemon.c_str(),
                         &m_db_vscp_daemon)) {

            // Failed to open/create the database file
            syslog(LOG_CRIT,
                   "VSCP Daemon configuration database could not be opened. - "
                   "Will exit.");
            vscp_str_format(str,
                            "Path=%s error=%s",
                            (const char *)m_path_db_vscp_daemon.c_str(),
                            sqlite3_errmsg(m_db_vscp_daemon));
            syslog(LOG_CRIT, "%s", (const char *)str.c_str());
            if (NULL != m_db_vscp_daemon) sqlite3_close(m_db_vscp_daemon);
            m_db_vscp_daemon = NULL;
            return false;
        } else {

            // Database is open.

            // Update the configuration database if it has evolved
            updateConfigDb();

            // Add possible missing configuration values
            addDefaultConfigValues();

            // Read configuration data
            readConfigurationDB();
        }
    } else {

        if (1) {

            // We need to create the database from scratch. This may not work if
            // the database is in a read only location.
            syslog(LOG_CRIT,
                   "VSCP Daemon configuration database does not exist - will "
                   "be created. Path=%s",
                   m_path_db_vscp_daemon.c_str());

            if (SQLITE_OK ==
                sqlite3_open((const char *)m_path_db_vscp_daemon.c_str(),
                             &m_db_vscp_daemon)) {

                // create the configuration database.
                if (!doCreateConfigurationTable()) {
                    syslog(LOG_ERR, "Failed to create configuration table.");
                }

                // Create the UDP node database
                if (!doCreateUdpNodeTable()) {
                    syslog(LOG_ERR, "Failed to create udpnode table.");
                }

                // Create the multicast database
                if (!doCreateMulticastTable()) {
                    syslog(LOG_ERR, "Failed to create multicast table.");
                }

                // Create user table
                if (!doCreateUserTable()) {
                    syslog(LOG_ERR, "Failed to create user table.");
                }

                // Create driver table
                if (!doCreateDriverTable()) {
                    syslog(LOG_ERR, "Failed to create driver table.");
                }

                // Create guid table
                if (!doCreateGuidTable()) {
                    syslog(LOG_ERR, "Failed to create GUID table.");
                }

                // Create location table
                if (!doCreateLocationTable()) {
                    syslog(LOG_ERR, "Failed to create location table.");
                }

                // Create mdf table
                if (!doCreateMdfCacheTable()) {
                    syslog(LOG_ERR, "Failed to create MDF cache table.");
                }

                // Create simpleui table
                if (!doCreateSimpleUiTable()) {
                    syslog(LOG_ERR, "Failed to create Simple UI table.");
                }

                // Create simpleui item table
                if (!doCreateSimpleUiItemTable()) {
                    syslog(LOG_ERR, "Failed to create Simple UI item table.");
                }

                // Create zone table
                if (!doCreateZoneTable()) {
                    syslog(LOG_ERR, "Failed to create zone table.");
                }

                // Create subzone table
                if (!doCreateSubZoneTable()) {
                    syslog(LOG_ERR, "Failed to create sub zone table.");
                }

                // Create userdef table
                if (!doCreateUserdefTableTable()) {
                    syslog(LOG_ERR, "Failed to create user defined table.");
                }

                // * * * All created * * *

                // Database is open. Read configuration data from it
                if (!readConfigurationDB()) {
                    syslog(LOG_ERR,
                           "Failed to read configuration from "
                           "configuration database.");
                }
            }
        } else {
            syslog(LOG_CRIT,
                   "VSCP Server configuration database path invalid - will "
                   "exit. Path=%s",
                   m_path_db_vscp_daemon.c_str());
            return false;
        }
    }

    // Read UDP nodes
    readUdpNodes();

    // Read multicast channels
    readMulticastChannels();

    // * * * VSCP Server data database - NEVER created * * *

    if (SQLITE_OK !=
        sqlite3_open(m_path_db_vscp_data.c_str(), &m_db_vscp_data)) {

        // Failed to open/create the database file
        syslog(LOG_ERR,
               "The VSCP data database could not be opened. - Will not be "
               "used. Path=%s error=%s",
               m_path_db_vscp_data.c_str(),
               sqlite3_errmsg(m_db_vscp_data));
        if (NULL != m_db_vscp_data) sqlite3_close(m_db_vscp_data);
        m_db_vscp_data = NULL;
    }

    ////////////////////////////////////////////////////////////////////////////
    //                      Read full XML configuration
    ////////////////////////////////////////////////////////////////////////////

    syslog(LOG_DEBUG, "Using configuration file: %s", strcfgfile.c_str());

    // Read XML configuration
    if (!readConfigurationXML(strcfgfile)) {
        syslog(
          LOG_ERR,
          "Unable to open/parse configuration file. Can't initialize! Path =%s",
          strcfgfile.c_str());
        return FALSE;
    }

    // Read users from database
    syslog(LOG_DEBUG, "loading users from users db...");
    m_userList.loadUsers();

    //==========================================================================
    //                           Add admin user
    //==========================================================================

    m_userList.addSuperUser(m_admin_user,
                            m_admin_password,
                            m_admin_allowfrom); // Remotes allows to connect

    //==========================================================================
    //                           Add driver user
    //==========================================================================

    // Generate username and password for drivers
    char buf[128];
    randPassword pw(4);

    // Level II Driver Username
    memset(buf, 0, sizeof(buf));
    pw.generatePassword(32, buf);
    m_driverUsername = "drv_";
    m_driverUsername += std::string(buf);

    // Level II Driver Password (can't contain ";" character)
    memset(buf, 0, sizeof(buf));
    pw.generatePassword(32, buf);
    m_driverPassword = buf;

    std::string drvhash;
    vscp_makePasswordHash(drvhash, std::string(buf));

    m_userList.addUser(m_driverUsername,
                       drvhash,                     // salt;hash
                       "System added driver user.", // full name
                       "System added driver user.", // note
                       NULL,
                       "driver",
                       "+127.0.0.0/24", // Only local
                       "*:*",           // All events
                       VSCP_ADD_USER_FLAG_LOCAL);

    // Calculate sunset etc
    m_automation.calcSun();

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
        ;
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

    syslog(LOG_DEBUG, "Log Level=%d", m_logLevel);

    // Load tables from database
    syslog(LOG_DEBUG, "Reading in user tables from DB.");
    m_userTableObjects.loadTablesFromDB();

    syslog(LOG_DEBUG, "Initializing user tables.");
    m_userTableObjects.init();

    // Initialize DM storage
    syslog(LOG_DEBUG, "Initializing DM.");
    m_dm.init();

    // Load decision matrix from XML file if mechanism is enabled
    syslog(LOG_DEBUG, "Loading DM from XML file.");
    m_dm.loadFromXML();

    // Load decision matrix from db if mechanism is enabled
    syslog(LOG_DEBUG, "Loading DM from database.");
    m_dm.loadFromDatabase();

    // Initialize variable storage
    syslog(LOG_DEBUG, "Initialize variables.");
    m_variables.init();

    // Load variables if mechanism is enabled
    syslog(LOG_DEBUG,
           "Loading persistent variables from XML variable default path.");
    m_variables.loadFromXML();

    // Start daemon internal client worker thread
    startClientMsgWorkerThread();

    // Start webserver and websockets
    // IMPORTANT!!!!!!!!
    // Must be started before the tcp/ip server as
    // ssl initializarion is done here
    start_webserver();

    // Start TCP/IP interface
    startTcpipSrvThread();

    // Start UDP interface
    startUDPSrvThread();

    // Start Multicast interface
    startMulticastWorkerThreads();

    // Load drivers
    startDeviceWorkerThreads();

    // Start daemon worker thread
    startDaemonWorkerThread();

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
    std::list<CClientItem *>::iterator nodeClient;

    vscpEvent EventLoop;
    EventLoop.vscp_class = VSCP_CLASS2_VSCPD;
    EventLoop.vscp_type  = VSCP2_TYPE_VSCPD_LOOP;
    EventLoop.sizeData   = 0;
    EventLoop.pdata      = NULL;

    vscpEvent EventStartUp;
    EventStartUp.vscp_class = VSCP_CLASS2_VSCPD;
    EventStartUp.vscp_type  = VSCP2_TYPE_VSCPD_STARTING_UP;
    EventStartUp.sizeData   = 0;
    EventStartUp.pdata      = NULL;

    vscpEvent EventShutDown;
    EventShutDown.vscp_class = VSCP_CLASS2_VSCPD;
    EventShutDown.vscp_type  = VSCP2_TYPE_VSCPD_SHUTTING_DOWN;
    EventShutDown.sizeData   = 0;
    EventShutDown.pdata      = NULL;

    // We need to create a clientItem and add this object to the list
    CClientItem *pClientItem = new CClientItem;
    if (NULL == pClientItem) {
        syslog(LOG_CRIT, "Unable to allocate Client item, Ending.");
        return false;
    }

    // Save a pointer to the client item
    m_dm.m_pClientItem = pClientItem;

    // Set Filter/Mask for full DM table
    memcpy(&pClientItem->m_filterVSCP,
           &m_dm.m_DM_Table_filter,
           sizeof(vscpEventFilter));

    // This is an active client
    pClientItem->m_bOpen         = true;
    pClientItem->m_type          = CLIENT_ITEM_INTERFACE_TYPE_CLIENT_INTERNAL;
    pClientItem->m_strDeviceName = "Internal Server DM Client.|Started at ";
    pClientItem->m_strDeviceName += vscpdatetime::Now().getISODateTime();

    // Add the client to the Client List
    pthread_mutex_lock(&m_clientMutex);
    if (!addClient(pClientItem, CLIENT_ID_DM)) {
        // Failed to add client
        delete pClientItem;
        m_dm.m_pClientItem = pClientItem = NULL;
        syslog(LOG_ERR, "ControlObject: Failed to add internal client.");
        pthread_mutex_unlock(&m_clientMutex);
    }
    pthread_mutex_unlock(&m_clientMutex);

    // Feed startup event
    m_dm.feed(&EventStartUp);

    //-------------------------------------------------------------------------
    //                            MAIN - LOOP
    //-------------------------------------------------------------------------

    // DM Loop
    int cnt = 0;
    while (!m_bQuit) {

        // CLOCKS_PER_SEC
        clock_t ticks, oldus;
        oldus = ticks = clock();

        // Feed possible periodic event
        m_dm.feedPeriodicEvent();

        // Put the LOOP event on the queue
        // Garanties at least one lop event between every other
        // event feed to the queue
        m_dm.feed(&EventLoop);

        // Wait for event
        if ((-1 == vscp_sem_wait(&pClientItem->m_semClientInputQueue, 10)) &&
            errno == ETIMEDOUT) {

            if (m_bQuit) continue; // Make quit request as fast as possible

            // Put the LOOP event on the queue
            m_dm.feed(&EventLoop);
            continue;
        }

        //----------------------------------------------------------------------
        //                         Event received here
        //                   from one of the incoming source
        //----------------------------------------------------------------------

        if (pClientItem->m_clientInputQueue.size()) {

            vscpEvent *pEvent;

            pthread_mutex_lock(&pClientItem->m_mutexClientInputQueue);
            pEvent = pClientItem->m_clientInputQueue.front();
            pClientItem->m_clientInputQueue.pop_front();
            pthread_mutex_unlock(&pClientItem->m_mutexClientInputQueue);

            if (NULL != pEvent) {

                if (vscp_doLevel2Filter(pEvent, &m_dm.m_DM_Table_filter)) {
                    // Feed event through matrix
                    m_dm.feed(pEvent);
                }

                // Remove the event
                vscp_deleteVSCPevent(pEvent);

            } // Valid pEvent pointer

            // Send events to websocket clients
            websock_post_incomingEvents();

        } // Event in queue

    } // while

    // Do shutdown event
    m_dm.feed(&EventShutDown);

    // Remove messages in the client queues
    pthread_mutex_lock(&m_clientMutex);
    removeClient(pClientItem);
    pthread_mutex_unlock(&m_clientMutex);

    syslog(LOG_DEBUG, "ControlObject: Run - Done");

    cleanup();

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// cleanup

bool
CControlObject::cleanup(void)
{

    syslog(LOG_DEBUG,
           "ControlObject: cleanup - Giving worker threads time to stop "
           "operations...");
    sleep(2); // Give threads some time to end

    syslog(LOG_DEBUG,
           "ControlObject: cleanup - Stopping device worker thread...");
    stopDeviceWorkerThreads();

    syslog(LOG_DEBUG,
           "ControlObject: cleanup - Stopping VSCP Server worker thread...");
    stopDaemonWorkerThread();

    syslog(LOG_DEBUG,
           "ControlObject: cleanup - Stopping client worker thread...");
    stopClientMsgWorkerThread();

    m_dm.cleanup();

    syslog(LOG_DEBUG,
           "ControlObject: cleanup - Stopping Web Server worker thread...");
    stop_webserver();

    syslog(LOG_DEBUG, "ControlObject: cleanup - Stopping UDP worker thread...");
    stopUDPSrvThread();

    syslog(LOG_DEBUG,
           "ControlObject: cleanup - Stopping Multicast worker threads...");
    stopMulticastWorkerThreads();

    syslog(LOG_DEBUG,
           "ControlObject: cleanup - Stopping TCP/IP worker thread...");
    stopTcpipSrvThread();

    syslog(LOG_DEBUG, "ControlObject: cleanup - Closing databases.");

    // Close the vscpd database
    if (NULL != m_db_vscp_daemon) sqlite3_close(m_db_vscp_daemon);
    m_db_vscp_daemon = NULL;

    // Close the VSCP data database
    if (NULL != m_db_vscp_data) sqlite3_close(m_db_vscp_data);
    m_db_vscp_data = NULL;

    // Clean up SQLite lib allocations
    sqlite3_shutdown();

    syslog(LOG_INFO, "Controlobject: ControlObject: Cleanup done.");
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// startClientMsgWorkerThread
//

bool
CControlObject::startClientMsgWorkerThread(void)
{
    syslog(LOG_INFO, "Controlobject: Starting client worker thread...");

    if (pthread_create(
          &m_clientMsgWorkerThread, NULL, clientMsgWorkerThread, this)) {

        syslog(LOG_CRIT, "Controlobject: Unable to start client thread.");
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
        syslog(LOG_DEBUG, "Controlobject: TCP/IP interface disabled.");
        return true;
    }

    syslog(LOG_DEBUG, "Controlobject: Starting TCP/IP interface...");

    // Create the tcp/ip server data object
    m_ptcpipSrvObject = (tcpipListenThreadObj *)new tcpipListenThreadObj(this);
    if (NULL == m_ptcpipSrvObject) {
        syslog(LOG_CRIT,
               "Controlobject: Failed to allocate storage for tcp/ip.");
    }

    // Set the port to listen for connections on
    m_ptcpipSrvObject->setListeningPort(m_strTcpInterfaceAddress);

    if (pthread_create(
          &m_tcpipListenThread, NULL, tcpipListenThread, m_ptcpipSrvObject)) {
        delete m_ptcpipSrvObject;
        m_ptcpipSrvObject = NULL;
        syslog(LOG_CRIT,
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

    syslog(LOG_DEBUG, "Controlobject: Terminating TCP thread.");

    pthread_join(m_tcpipListenThread, NULL);
    delete m_ptcpipSrvObject;
    m_ptcpipSrvObject = NULL;

    syslog(LOG_DEBUG, "Controlobject: Terminated TCP thread.");

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// startUDPSrvThread
//

bool
CControlObject::startUDPSrvThread(void)
{
    if (!m_enableUDP) {
        syslog(LOG_DEBUG, "UDP server disabled.");
        return false;
    }

    syslog(LOG_DEBUG, "Controlobject: Starting UDP simple server interface...");

    if (pthread_create(&m_UDPThread, NULL, UDPThread, &m_udpSrvObj)) {

        syslog(LOG_CRIT,
               "Controlobject: Unable to start the udp simple server thread.");
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// stopUDPSrvThread
//

bool
CControlObject::stopUDPSrvThread(void)
{
    syslog(LOG_DEBUG, "Controlobject: Terminating UDP thread.");

    m_udpSrvObj.m_bQuit = true;
    pthread_join(m_UDPThread, NULL);

    syslog(LOG_DEBUG, "Controlobject: Terminated UDP thread.");

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// startMulticastWorkerThreads
//

bool
CControlObject::startMulticastWorkerThreads(void)
{
    if (!m_bEnableMulticast) {
        syslog(LOG_DEBUG, "Multicast interface is disabled.");
        return true;
    }

    if (m_multicastObj.m_channels.empty()) {
        syslog(LOG_DEBUG, "No multicast channels defined.");
        return true;
    }

    // Bring up all multicast channels
    std::list<multicastChannelItem *>::iterator it;
    for (it = m_multicastObj.m_channels.begin();
         it != m_multicastObj.m_channels.end();
         ++it) {

        multicastChannelItem *pChannel = *it;
        if (NULL == pChannel) {
            syslog(
              LOG_ERR,
              "Controlobject: Multicast start channel table invalid entry.");
            continue;
        }

        syslog(LOG_DEBUG, "Starting multicast channel interface thread...");
        if (pthread_create(&pChannel->m_workerThread,
                           NULL,
                           multicastClientThread,
                           pChannel)) {
            syslog(LOG_ERR,
                   "Unable to start the multicast channel interface thread.");
        }
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// stopMulticastWorkerThreads
//

bool
CControlObject::stopMulticastWorkerThreads(void)
{
    std::list<multicastChannelItem *>::iterator it;

    for (it = m_multicastObj.m_channels.begin();
         it != m_multicastObj.m_channels.end();
         /* inline */) {

        multicastChannelItem *pChannel = *it;
        if (NULL == pChannel) {
            syslog(LOG_ERR,
                   "Controlobject: Multicast end channel table invalid entry.");
            continue;
        }

        pChannel->m_quit = true;
        pthread_join(pChannel->m_workerThread, NULL);
        delete pChannel;

        it = m_multicastObj.m_channels.erase(it);
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// startDaemonWorkerThread
//

bool
CControlObject::startDaemonWorkerThread(void)
{
    syslog(LOG_DEBUG, "Controlobject: Starting daemon worker thread,,.");

    m_pdaemonWorkerObj = new daemonWorkerObj(this);
    if (NULL == m_pdaemonWorkerObj) {
        syslog(
          LOG_CRIT,
          "Controlobject: Unable to allocate object for daemon worker thread.");
        return false;
    }

    m_pdaemonWorkerObj->m_pCtrlObject = this; // Give it a pointer to us

    if (pthread_create(&m_clientMsgWorkerThread,
                       NULL,
                       daemonWorkerThread,
                       m_pdaemonWorkerObj)) {

        syslog(LOG_CRIT,
               "Controlobject: Unable to start the daemon worker thread.");
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// stopDaemonWorkerThread
//

bool
CControlObject::stopDaemonWorkerThread(void)
{
    syslog(LOG_DEBUG, "Controlobject: Stopping daemon worker thread...");
    m_pdaemonWorkerObj->m_bQuit = true;
    pthread_join(m_clientMsgWorkerThread, NULL);
    syslog(LOG_DEBUG, "Controlobject: Stoped daemon worker thread.");

    return true;
}

////////////////////////////////////////////////////////////////////////////////
// startDeviceWorkerThreads
//

bool
CControlObject::startDeviceWorkerThreads(void)
{
    CDeviceItem *pDeviceItem;
    syslog(LOG_DEBUG, "[Controlobject][Driver] - Starting drivers...");

    std::deque<CDeviceItem *>::iterator it;
    for (it = m_deviceList.m_devItemList.begin();
         it != m_deviceList.m_devItemList.end();
         ++it) {

        pDeviceItem = *it;
        if (NULL != pDeviceItem) {

            syslog(LOG_DEBUG,
                   "Controlobject: [Driver] - Preparing: %s ",
                   pDeviceItem->m_strName.c_str());

            // Just start if enabled
            if (!pDeviceItem->m_bEnable) continue;

            syslog(LOG_DEBUG,
                   "Controlobject: [Driver] - Starting: %s ",
                   pDeviceItem->m_strName.c_str());

            // *** Level 3 Driver * * *
            if (VSCP_DRIVER_LEVEL3 == pDeviceItem->m_driverLevel) {

                // Startup Level III driver
                //      username
                //      password
                //      driver parameter string
                // The driver should return immediately after starting
                std::string strExecute = pDeviceItem->m_strPath;
                strExecute += " ";
                strExecute += m_driverUsername;
                strExecute += " ";
                strExecute += m_driverPassword;
                strExecute += " ";
                strExecute += pDeviceItem->m_strParameter;

                int status = system(pDeviceItem->m_strPath.c_str());

            } else {
                // Start  the driver logic
                pDeviceItem->startDriver(this);
            }

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
    CDeviceItem *pDeviceItem;

    syslog(LOG_DEBUG, "[Controlobject][Driver] - Stopping drivers...");
    std::deque<CDeviceItem *>::iterator iter;
    for (iter = m_deviceList.m_devItemList.begin();
         iter != m_deviceList.m_devItemList.end();
         ++iter) {

        pDeviceItem = *iter;
        if (NULL != pDeviceItem) {
            syslog(LOG_DEBUG,
                   "Controlobject: [Driver] - Stopping: %s ",
                   pDeviceItem->m_strName.c_str());
            pDeviceItem->stopDriver();
        }
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// generateSessionId
//

bool
CControlObject::generateSessionId(const char *pKey, char *psid)
{
    char buf[8193];

    // Check pointers
    if (NULL == pKey) return false;
    if (NULL == psid) return false;

    if (strlen(pKey) > 256) return false;

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

    vscp_md5(psid, (const unsigned char *)buf, strlen(buf));

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// getVscpCapabilities
//

bool
CControlObject::getVscpCapabilities(uint8_t *pCapability)
{
    // Check pointer
    if (NULL == pCapability) return false;

    uint64_t caps = 0;
    memset(pCapability, 0, 8);

    // VSCP Multicast interface
    if (m_bEnableMulticast) {
        caps |= VSCP_SERVER_CAPABILITY_MULTICAST_CHANNEL;
    }

    // VSCP TCP/IP interface
    if (m_enableTcpip) {
        caps |= VSCP_SERVER_CAPABILITY_TCPIP;
    }

    // VSCP UDP interface
    if (m_udpSrvObj.m_bEnable) {
        caps |= VSCP_SERVER_CAPABILITY_UDP;
    }

    // VSCP Multicast announce interface
    if (m_bEnableMulticastAnnounce) {
        caps |= VSCP_SERVER_CAPABILITY_MULTICAST_ANNOUNCE;
    }

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

/////////////////////////////////////////////////////////////////////////////
// getCountRecordsDB
//

long
CControlObject::getCountRecordsDB(sqlite3 *db, std::string &table)
{
    long count = 0;
    sqlite3_stmt *ppStmt;

    // If not open no records
    if (NULL == db) return 0;

    std::string sql =
      vscp_str_format("SELECT count(*)from %s", (const char *)table.c_str());

    if (SQLITE_OK !=
        sqlite3_prepare(db, (const char *)sql.c_str(), -1, &ppStmt, NULL)) {
        syslog(LOG_ERR,
               "Failed to prepare count for log database. SQL is %s",
               VSCPDB_LOG_COUNT);
        return 0;
    }

    if (SQLITE_ROW == sqlite3_step(ppStmt)) {
        count = sqlite3_column_int(ppStmt, 0);
    }
    return count;
}

///////////////////////////////////////////////////////////////////////////////
// sendEventToClient
//

void
CControlObject::sendEventToClient(CClientItem *pClientItem, vscpEvent *pEvent)
{
    // Must be valid pointers
    if (NULL == pClientItem) return;
    if (NULL == pEvent) return;

    // Check if filtered out - if so do nothing here
    if (!vscp_doLevel2Filter(pEvent, &pClientItem->m_filterVSCP)) return;

    // If the client queue is full for this client then the
    // client will not receive the message
    if (pClientItem->m_clientInputQueue.size() >
        m_maxItemsInClientReceiveQueue) {
        // Overrun
        pClientItem->m_statistics.cntOverruns++;
        return;
    }

    // Create an event
    vscpEvent *pnewvscpEvent = new vscpEvent;
    if (NULL != pnewvscpEvent) {

        // Copy in the new event
        memcpy(pnewvscpEvent, pEvent, sizeof(vscpEvent));

        // And data...
        if ((NULL != pEvent->pdata) && (pEvent->sizeData > 0)) {
            // Copy in data
            pnewvscpEvent->pdata = new uint8_t[pEvent->sizeData];
            if (NULL != pnewvscpEvent->pdata) {
                memcpy(pnewvscpEvent->pdata, pEvent->pdata, pEvent->sizeData);
            }
        } else {
            // No data
            pnewvscpEvent->pdata = NULL;
        }

        // Add the new event to the input queue
        pthread_mutex_lock(&pClientItem->m_mutexClientInputQueue);
        pClientItem->m_clientInputQueue.push_back(pnewvscpEvent);
        sem_post(&pClientItem->m_semClientInputQueue);
        pthread_mutex_unlock(&pClientItem->m_mutexClientInputQueue);
    }
}

///////////////////////////////////////////////////////////////////////////////
// sendEventAllClients
//

void
CControlObject::sendEventAllClients(vscpEvent *pEvent, uint32_t excludeID)
{
    CClientItem *pClientItem;
    std::list<CClientItem *>::iterator it;

    if (NULL == pEvent) return;

    pthread_mutex_lock(&m_clientMutex);
    for (it = m_clientList.m_itemList.begin();
         it != m_clientList.m_itemList.end();
         ++it) {
        pClientItem = *it;

        if ((NULL != pClientItem) && (excludeID != pClientItem->m_clientID)) {
            sendEventToClient(pClientItem, pEvent);
        }
    }

    pthread_mutex_unlock(&m_clientMutex);
}

///////////////////////////////////////////////////////////////////////////////
// sendEvent
//

bool
CControlObject::sendEvent(CClientItem *pClientItem, vscpEvent *peventToSend)
{
    bool bSent = false;

    // Check pointers
    if (NULL == pClientItem) return false;
    if (NULL == peventToSend) return false;

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

    vscpEvent *pEvent = new vscpEvent; // Create new VSCP Event
    if (NULL == pEvent) {
        return false;
    }

    // Copy event
    vscp_copyVSCPEvent(pEvent, peventToSend);

    // We don't need the original event anymore
    if (NULL != peventToSend->pdata) {
        delete[] peventToSend->pdata;
        peventToSend->pdata    = NULL;
        peventToSend->sizeData = 0;
    }

    // Save the originating clients id so
    // this client don't get the message back
    pEvent->obid = pClientItem->m_clientID;

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

        destguid.setAt(0, 0); // Interface GUID's have LSB bytes nilled
        destguid.setAt(1, 0);

        syslog(LOG_DEBUG,
               "Level I event over Level II "
               "dest = %d:%d:%d:%d:%d:%d:%d:%d:"
               "%d:%d:%d:%d:%d:%d:%d:%d:",
               destguid.getAt(15),
               destguid.getAt(14),
               destguid.getAt(13),
               destguid.getAt(12),
               destguid.getAt(11),
               destguid.getAt(10),
               destguid.getAt(9),
               destguid.getAt(8),
               destguid.getAt(7),
               destguid.getAt(6),
               destguid.getAt(5),
               destguid.getAt(4),
               destguid.getAt(3),
               destguid.getAt(2),
               destguid.getAt(1),
               destguid.getAt(0));

        // Find client
        pthread_mutex_lock(&m_clientMutex);

        CClientItem *pDestClientItem = NULL;
        std::list<CClientItem *>::iterator it;
        for (it = m_clientList.m_itemList.begin();
             it != m_clientList.m_itemList.end();
             ++it) {

            CClientItem *pItem = *it;
            syslog(
              LOG_DEBUG,
              "Test if = %d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d: %s",
              pItem->m_guid.getAt(15),
              pItem->m_guid.getAt(14),
              pItem->m_guid.getAt(13),
              pItem->m_guid.getAt(12),
              pItem->m_guid.getAt(11),
              pItem->m_guid.getAt(10),
              pItem->m_guid.getAt(9),
              pItem->m_guid.getAt(8),
              pItem->m_guid.getAt(7),
              pItem->m_guid.getAt(6),
              pItem->m_guid.getAt(5),
              pItem->m_guid.getAt(4),
              pItem->m_guid.getAt(3),
              pItem->m_guid.getAt(2),
              pItem->m_guid.getAt(1),
              pItem->m_guid.getAt(0),
              pItem->m_strDeviceName.c_str());

            if (pItem->m_guid == destguid) {
                // Found
                pDestClientItem = pItem;
                bSent           = true;
                syslog(LOG_DEBUG, "Match ");
                sendEventToClient(pItem, pEvent);
                break;
            }
        }

        pthread_mutex_unlock(&m_clientMutex);
    }

    if (!bSent) {

        // There must be room in the send queue
        if (m_maxItemsInClientReceiveQueue > m_clientOutputQueue.size()) {

            pthread_mutex_lock(&m_mutexClientOutputQueue);
            m_clientOutputQueue.push_back(pEvent);
            sem_post(&m_semClientOutputQueue);
            pthread_mutex_unlock(&m_mutexClientOutputQueue);

            // TX Statistics
            pClientItem->m_statistics.cntTransmitData += pEvent->sizeData;
            pClientItem->m_statistics.cntTransmitFrames++;
        } else {

            pClientItem->m_statistics.cntOverruns++;

            vscp_deleteVSCPevent(pEvent);
            return false;
        }
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////////
// addClient
//

bool
CControlObject::addClient(CClientItem *pClientItem, uint32_t id)
{
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
// removeClient
//

void
CControlObject::removeClient(CClientItem *pClientItem)
{
    // Do not try to handle invalid clients
    if (NULL == pClientItem) return;

    // Remove the client
    m_clientList.removeClient(pClientItem);
}

//////////////////////////////////////////////////////////////////////////////
// addKnowNode
//

void
CControlObject::addKnownNode(cguid &guid, cguid &ifguid, std::string &name)
{
    ; // TODO
}

///////////////////////////////////////////////////////////////////////////////
//  getMacAddress
//

bool
CControlObject::getMacAddress(cguid &guid)
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
    Ncb.ncb_buffer  = (UCHAR *)&lenum;
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

        strcpy((char *)Ncb.ncb_callname, "*               ");
        Ncb.ncb_buffer = (unsigned char *)&Adapter;
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
    if (-1 == fd) return false;

    memset(&s, 0, sizeof(s));
    strcpy(s.ifr_name, "eth0");

    if (0 == ioctl(fd, SIOCGIFHWADDR, &s)) {

        unsigned char *ptr;
        ptr = (unsigned char *)&s.ifr_ifru.ifru_hwaddr.sa_data[0];
        syslog(LOG_DEBUG,
               "Ethernet MAC address: %02X:%02X:%02X:%02X:%02X:%02X",
               (uint8_t)s.ifr_addr.sa_data[0],
               (uint8_t)s.ifr_addr.sa_data[1],
               (uint8_t)s.ifr_addr.sa_data[2],
               (uint8_t)s.ifr_addr.sa_data[3],
               (uint8_t)s.ifr_addr.sa_data[4],
               (uint8_t)s.ifr_addr.sa_data[5]);

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
    } else {
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
CControlObject::getIPAddress(cguid &guid)
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
    struct hostent *lpLocalHostEntry;
#endif
    lpLocalHostEntry = gethostbyname(szName);
    if (NULL == lpLocalHostEntry) {
        return false;
    }

    // Get all local addresses
    int idx = -1;
    void *pAddr;
    unsigned long localaddr[16]; // max 16 local addresses
    do {
        idx++;
        localaddr[idx] = 0;
        pAddr          = lpLocalHostEntry->h_addr_list[idx];
        if (NULL != pAddr) localaddr[idx] = *((unsigned long *)pAddr);
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

uint8_t *
CControlObject::getSystemKey(uint8_t *pKey)
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
CControlObject::getSystemKeyMD5(std::string &strKey)
{
    char digest[33];
    vscp_md5(digest, m_systemKey, 32);
    strKey = digest;
}

// ----------------------------------------------------------------------------
// General XML configuration callbacks
// ----------------------------------------------------------------------------

/*
<general runasuser="xxxx"
                guid="..."
                servername="sdfdsfsd"
                clientbuffersize="100" >

    <logging  loglevel="n"
                logdays="n"
                debugflags1="n"
                debugflags1="n"
                debugflags2="n"
                debugflags3="n"
                debugflags4="n"
                debugflags5="n"
                debugflags6="n"
                debugflags7="n"
                debugflags8="n" />

    <security admin="username"
        password="450ADCE88F2FDBB20F3318B65E53CA4A;06D3311CC2195E80BE4F8EB12931BFEB5C630F6B154B2D644ABE29CEBDBFB545"
        allowfrom="list of remotes"
        vscptoken="Carpe diem quam minimum credula postero"
        vscpkey="A4A86F7D7E119BA3F0CD06881E371B989B33B6D606A863B633EF529D64544F8E"
        digest="" />

    <database vscp-daemon="db-path"
            vscp-data="db-path"
            vscp-variable="db-path"
            vscp-dm="db-path" />
</general>
*/
static int depth_general_config_parser = 0;
static int bvscpConfigFound            = 0;
static int bGeneralConfigFound         = 0;

static void
startGeneralConfigParser(void *data, const char *name, const char **attr)
{
    CControlObject *pObj = (CControlObject *)data;
    if (NULL == data) return;

    if ((0 == depth_general_config_parser) &&
        (0 == vscp_strcasecmp(name, "vscpconfig"))) {
        bvscpConfigFound = TRUE;
    } else if (bvscpConfigFound && (1 == depth_general_config_parser) &&
               (0 == vscp_strcasecmp(name, "general"))) {
        bGeneralConfigFound = TRUE;

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "clientbuffersize")) {
                pObj->m_maxItemsInClientReceiveQueue =
                  vscp_readStringValue(attribute);
            } else if (0 == vscp_strcasecmp(attr[i], "runasuser")) {
                vscp_trim(attribute);
                pObj->m_runAsUser = attribute;
                // Also assign to web user
                pObj->m_web_run_as_user = attribute;
            } else if (0 == vscp_strcasecmp(attr[i], "guid")) {
                pObj->m_guid.getFromString(attribute);
            } else if (0 == vscp_strcasecmp(attr[i], "servername")) {
                pObj->m_strServerName = attribute;
            }
        }

    } else if (2 == depth_general_config_parser) {

        if (bGeneralConfigFound && (0 == vscp_strcasecmp(name, "logging"))) {

            for (int i = 0; attr[i]; i += 2) {

                std::string attribute = attr[i + 1];
                vscp_trim(attribute);

                if (0 == vscp_strcasecmp(attr[i], "loglevel")) {
                    if (0 == vscp_strcasecmp(attribute.c_str(), "none")) {
                        pObj->m_logLevel = DAEMON_LOGMSG_NONE;
                    } else if (0 ==
                               vscp_strcasecmp(attribute.c_str(), "normal")) {
                        pObj->m_logLevel = DAEMON_LOGMSG_NORMAL;
                    } else if (0 ==
                               vscp_strcasecmp(attribute.c_str(), "debug")) {
                        pObj->m_logLevel = DAEMON_LOGMSG_DEBUG;
                    } else {
                        pObj->m_logLevel = vscp_readStringValue(attribute);
                        if (pObj->m_logLevel > DAEMON_LOGMSG_DEBUG) {
                            pObj->m_logLevel = DAEMON_LOGMSG_DEBUG;
                        }
                    }
                } else if (0 == vscp_strcasecmp(attr[i], "debugflags1")) {
                    if (attribute.length()) {
                        pObj->m_debugFlags[0] = vscp_readStringValue(attribute);
                    }
                } else if (0 == vscp_strcasecmp(attr[i], "debugflags2")) {
                    if (attribute.length()) {
                        pObj->m_debugFlags[1] = vscp_readStringValue(attribute);
                    }
                } else if (0 == vscp_strcasecmp(attr[i], "debugflags3")) {
                    if (attribute.length()) {
                        pObj->m_debugFlags[2] = vscp_readStringValue(attribute);
                    }
                } else if (0 == vscp_strcasecmp(attr[i], "debugflags4")) {
                    if (attribute.length()) {
                        pObj->m_debugFlags[3] = vscp_readStringValue(attribute);
                    }
                } else if (0 == vscp_strcasecmp(attr[i], "debugflags5")) {
                    if (attribute.length()) {
                        pObj->m_debugFlags[4] = vscp_readStringValue(attribute);
                    }
                } else if (0 == vscp_strcasecmp(attr[i], "debugflags6")) {
                    if (attribute.length()) {
                        pObj->m_debugFlags[5] = vscp_readStringValue(attribute);
                    }
                } else if (0 == vscp_strcasecmp(attr[i], "debugflags7")) {
                    if (attribute.length()) {
                        pObj->m_debugFlags[6] = vscp_readStringValue(attribute);
                    }
                } else if (0 == vscp_strcasecmp(attr[i], "debugflags8")) {
                    if (attribute.length()) {
                        pObj->m_debugFlags[7] = vscp_readStringValue(attribute);
                    }
                }
            }

        } else if (bGeneralConfigFound &&
                   (0 == vscp_strcasecmp(name, "security"))) {

            for (int i = 0; attr[i]; i += 2) {

                std::string attribute = attr[i + 1];
                vscp_trim(attribute);

                if (0 == vscp_strcasecmp(attr[i], "admin")) {
                    pObj->m_admin_user = attribute;
                } else if (0 == vscp_strcasecmp(attr[i], "password")) {
                    pObj->m_admin_password = attribute;
                } else if (0 == vscp_strcasecmp(attr[i], "allowfrom")) {
                    pObj->m_admin_allowfrom = attribute;
                } else if (0 == vscp_strcasecmp(attr[i], "vscptoken")) {
                    pObj->m_vscptoken = attribute;
                } else if (0 == vscp_strcasecmp(attr[i], "vscpkey")) {
                    if (attribute.length()) {
                        vscp_hexStr2ByteArray(
                          pObj->m_systemKey, 32, attribute.c_str());
                    }
                }
            }
        } else if (bGeneralConfigFound &&
                   (0 == vscp_strcasecmp(name, "database"))) {

            for (int i = 0; attr[i]; i += 2) {

                std::string attribute = attr[i + 1];
                vscp_trim(attribute);

                if (0 == strcasecmp(attr[i], "daemon")) {
                    vscp_trim(attribute);
                    if (attribute.length()) {
                        pObj->m_path_db_vscp_daemon = attribute;
                    }
                } else if (0 == strcasecmp(attr[i], "data")) {
                    vscp_trim(attribute);
                    if (attribute.length()) {
                        pObj->m_path_db_vscp_data = attribute;
                    }
                }
            }
        }
    }

    depth_general_config_parser++;
}

static void
endGeneralConfigParser(void *data, const char *name)
{
    if ((0 == depth_general_config_parser) &&
        (0 == vscp_strcasecmp(name, "vscpconfig"))) {
        bvscpConfigFound = FALSE;
    } else if (bvscpConfigFound && (1 == depth_general_config_parser) &&
               (0 == vscp_strcasecmp(name, "general"))) {
        bGeneralConfigFound = FALSE;
    }

    depth_general_config_parser--;
}

///////////////////////////////////////////////////////////////////////////////
// readXMLConfigurationGeneral
//
// Read the configuration XML file
//

bool
CControlObject::readXMLConfigurationGeneral(const std::string &strcfgfile)
{
    FILE *fp;

    syslog(LOG_DEBUG,
           "Reading general XML configuration from [%s]",
           (const char *)strcfgfile.c_str());

    fp = fopen(strcfgfile.c_str(), "r");
    if (NULL == fp) {
        syslog(LOG_CRIT,
               "Failed to open configuration file [%s]",
               strcfgfile.c_str());
        return false;
    }

    XML_Parser xmlParser = XML_ParserCreate("UTF-8");
    XML_SetUserData(xmlParser, this);
    XML_SetElementHandler(
      xmlParser, startGeneralConfigParser, endGeneralConfigParser);

    int bytes_read;
    void *buf = XML_GetBuffer(xmlParser, XML_BUFF_SIZE);
    if (NULL == buf) {
        XML_ParserFree(xmlParser);
        fclose(fp);
        syslog(LOG_CRIT,
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
}

// ----------------------------------------------------------------------------
// FULL XML configuration callbacks
// ----------------------------------------------------------------------------

static int depth_full_config_parser   = 0;
static char *last_full_config_content = NULL;
static int bVscpConfigFound           = 0;
static int bUDPConfigFound            = 0;
static int bMulticastConfigFound      = 0;
static int bRemoteUserConfigFound     = 0;
static int bLevel1DriverConfigFound   = 0;
static int bLevel2DriverConfigFound   = 0;
static int bLevel3DriverConfigFound   = 0;
static int bKnownNodesConfigFound     = 0;
static int bTablesConfigFound         = 0;

static void
startFullConfigParser(void *data, const char *name, const char **attr)
{
    CControlObject *pObj = (CControlObject *)data;
    if (NULL == data) return;

    if ((0 == depth_full_config_parser) &&
        (0 == vscp_strcasecmp(name, "vscpconfig"))) {
        bVscpConfigFound = TRUE;
    } else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
               (0 == vscp_strcasecmp(name, "tcpip"))) {

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "enable")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_enableTcpip = true;
                } else {
                    pObj->m_enableTcpip = false;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "interface")) {
                vscp_startsWith(attribute, "tcp://", &attribute);
                vscp_trim(attribute);
                pObj->m_strTcpInterfaceAddress = attribute;
            } else if (0 == vscp_strcasecmp(attr[i], "ssl_certificate")) {
                pObj->m_tcpip_ssl_certificate = attribute;
            } else if (0 == vscp_strcasecmp(attr[i], "ssl_verify_peer")) {
                pObj->m_tcpip_ssl_verify_peer = vscp_readStringValue(attribute);
            } else if (0 == vscp_strcasecmp(attr[i], "ssl_certificate_chain")) {
                pObj->m_tcpip_ssl_certificate_chain = attribute;
            } else if (0 == vscp_strcasecmp(attr[i], "ssl_ca_path")) {
                pObj->m_tcpip_ssl_ca_path = attribute;
            } else if (0 == vscp_strcasecmp(attr[i], "ssl_ca_file")) {
                pObj->m_tcpip_ssl_ca_file = attribute;
            } else if (0 == vscp_strcasecmp(attr[i], "ssl_verify_depth")) {
                pObj->m_tcpip_ssl_verify_depth =
                  vscp_readStringValue(attribute);
            } else if (0 ==
                       vscp_strcasecmp(attr[i], "ssl_default_verify_paths")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_tcpip_ssl_default_verify_paths = true;
                } else {
                    pObj->m_tcpip_ssl_default_verify_paths = false;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "ssl_cipher_list")) {
                pObj->m_tcpip_ssl_cipher_list = attribute;
            } else if (0 == vscp_strcasecmp(attr[i], "ssl_protocol_version")) {
                pObj->m_tcpip_ssl_verify_depth =
                  vscp_readStringValue(attribute);
            } else if (0 == vscp_strcasecmp(attr[i], "ssl_short_trust")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_tcpip_ssl_short_trust = true;
                } else {
                    pObj->m_tcpip_ssl_short_trust = false;
                }
            }
        }
    } else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
               (0 == vscp_strcasecmp(name, "multicast-announce"))) {
        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "enable")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_bEnableMulticastAnnounce = true;
                } else {
                    pObj->m_bEnableMulticastAnnounce = false;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "interface")) {
                pObj->m_strMulticastAnnounceAddress = attribute;
            }
            if (0 == vscp_strcasecmp(attr[i], "ttl")) {
                pObj->m_ttlMultiCastAnnounce = vscp_readStringValue(attribute);
            }
        }
    } else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
               (0 == vscp_strcasecmp(name, "udp"))) {

        bUDPConfigFound = TRUE;

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "enable")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_udpSrvObj.m_bEnable = true;
                } else {
                    pObj->m_udpSrvObj.m_bEnable = false;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "bAllowUnsecure")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_udpSrvObj.m_bAllowUnsecure = true;
                } else {
                    pObj->m_udpSrvObj.m_bAllowUnsecure = false;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "bSendAck")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_udpSrvObj.m_bAck = true;
                } else {
                    pObj->m_udpSrvObj.m_bAck = false;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "user")) {
                pObj->m_udpSrvObj.m_user = attribute;
            } else if (0 == vscp_strcasecmp(attr[i], "password")) {
                pObj->m_udpSrvObj.m_password = attribute;
            } else if (0 == vscp_strcasecmp(attr[i], "interface")) {
                pObj->m_udpSrvObj.m_interface = attribute;
            } else if (0 == vscp_strcasecmp(attr[i], "guid")) {
                pObj->m_udpSrvObj.m_guid.getFromString(attribute);
            } else if (0 == vscp_strcasecmp(attr[i], "filter")) {
                if (attribute.length()) {
                    vscp_readFilterFromString(&pObj->m_udpSrvObj.m_filter,
                                              attribute);
                }
            } else if (0 == vscp_strcasecmp(attr[i], "mask")) {
                if (attribute.length()) {
                    vscp_readMaskFromString(&pObj->m_udpSrvObj.m_filter,
                                            attribute);
                }
            }
        }
    } else if (bVscpConfigFound && bUDPConfigFound &&
               (2 == depth_full_config_parser) &&
               (0 == vscp_strcasecmp(name, "rxnode"))) {

        for (int i = 0; attr[i]; i += 2) {

            udpRemoteClientInfo *pudpClient = new udpRemoteClientInfo;
            if (NULL == pudpClient) {
                syslog(LOG_ERR, "Unable to allocate storage for UDP client");
                return;
            }

            vscp_clearVSCPFilter(&pudpClient->m_filter);

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "enable")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pudpClient->m_bEnable = true;
                } else {
                    pudpClient->m_bEnable = false;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "interface")) {
                pudpClient->m_remoteAddress = attribute;
            } else if (0 == vscp_strcasecmp(attr[i], "filter")) {
                if (attribute.length()) {
                    vscp_readFilterFromString(&pudpClient->m_filter, attribute);
                }
            } else if (0 == vscp_strcasecmp(attr[i], "mask")) {
                if (attribute.length()) {
                    vscp_readMaskFromString(&pudpClient->m_filter, attribute);
                }
            } else if (0 == vscp_strcasecmp(attr[i], "broadcast")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pudpClient->m_bSetBroadcast = true;
                } else {
                    pudpClient->m_bSetBroadcast = false;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "encryption")) {
                pudpClient->m_nEncryption =
                  vscp_getEncryptionCodeFromToken(attribute);
            }

            // add to udp client list
            pudpClient->m_index = 0;
            pObj->m_udpSrvObj.m_remotes.push_back(pudpClient);
        }
    } else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
               (0 == vscp_strcasecmp(name, "multicast"))) {

        bMulticastConfigFound = TRUE;

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "enable")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_bEnableMulticast = true;
                } else {
                    pObj->m_bEnableMulticast = false;
                }
            }
        }

    } else if (bVscpConfigFound && bMulticastConfigFound &&
               (2 == depth_full_config_parser) &&
               (0 == vscp_strcasecmp(name, "channel"))) {

        multicastChannelItem *pChannel = new multicastChannelItem;
        if (NULL == pChannel) {
            syslog(LOG_ERR, "Unable to allocate storage for multicast client");
            return;
        }

        pChannel->m_bEnable        = false;
        pChannel->m_bAllowUnsecure = false;
        pChannel->m_port           = 0;
        pChannel->m_ttl            = 1;
        pChannel->m_nEncryption    = 0;
        pChannel->m_bSendAck       = 0;
        pChannel->m_index          = 0;

        // Default is to let everything come through
        vscp_clearVSCPFilter(&pChannel->m_txFilter);
        vscp_clearVSCPFilter(&pChannel->m_rxFilter);

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "enable")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pChannel->m_bEnable = true;
                } else {
                    pChannel->m_bEnable = false;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "bsendack")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pChannel->m_bSendAck = true;
                } else {
                    pChannel->m_bSendAck = false;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "bAllowUndsecure")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pChannel->m_bAllowUnsecure = true;
                } else {
                    pChannel->m_bAllowUnsecure = false;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "public")) {
                pChannel->m_public = attribute;
            } else if (0 == vscp_strcasecmp(attr[i], "port")) {
                pChannel->m_port = vscp_readStringValue(attribute);
            } else if (0 == vscp_strcasecmp(attr[i], "group")) {
                pChannel->m_gropupAddress = attribute;
            } else if (0 == vscp_strcasecmp(attr[i], "ttl")) {
                pChannel->m_ttl = vscp_readStringValue(attribute);
            } else if (0 == vscp_strcasecmp(attr[i], "guid")) {
                pChannel->m_guid.getFromString(attribute);
            } else if (0 == vscp_strcasecmp(attr[i], "txfilter")) {
                if (attribute.length()) {
                    vscp_readFilterFromString(&pChannel->m_txFilter, attribute);
                }
            } else if (0 == vscp_strcasecmp(attr[i], "txmask")) {
                if (attribute.length()) {
                    vscp_readMaskFromString(&pChannel->m_txFilter, attribute);
                }
            } else if (0 == vscp_strcasecmp(attr[i], "rxfilter")) {
                if (attribute.length()) {
                    vscp_readFilterFromString(&pChannel->m_rxFilter, attribute);
                }
            } else if (0 == vscp_strcasecmp(attr[i], "rxmask")) {
                if (attribute.length()) {
                    vscp_readMaskFromString(&pChannel->m_rxFilter, attribute);
                }
            } else if (0 == vscp_strcasecmp(attr[i], "encryption")) {
                if (attribute.length()) {
                    pChannel->m_nEncryption =
                      vscp_getEncryptionCodeFromToken(attribute);
                }
            }
        }

        // add to multicast client list
        pChannel->m_index = 0;
        pObj->m_multicastObj.m_channels.push_back(pChannel);
    } else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
               (0 == vscp_strcasecmp(name, "dm"))) {

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "enable")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_dm.m_bEnable = true;
                } else {
                    pObj->m_dm.m_bEnable = false;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "path")) { // Deprecated
                if (attribute.length()) {
                    pObj->m_dm.m_staticXMLPath = attribute;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "pathxml")) {
                if (attribute.length()) {
                    pObj->m_dm.m_staticXMLPath = attribute;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "loglevel")) {
                pObj->m_debugFlags[0] |= VSCP_DEBUG1_DM;
            }
        }

    } else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
               (0 == vscp_strcasecmp(name, "variables"))) {

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "path")) { // Deprecated
                if (attribute.length()) {
                    pObj->m_variables.m_dbFilename = attribute;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "pathxml")) {
                if (attribute.length()) {
                    pObj->m_variables.m_xmlPath = attribute;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "pathdb")) {
                if (attribute.length()) {
                    pObj->m_variables.m_dbFilename = attribute;
                }
            }
        }
    } else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
               (0 == vscp_strcasecmp(name, "webserver"))) {

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "enable")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_web_bEnable = true;
                } else {
                    pObj->m_web_bEnable = false;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "document_root")) {
                if (attribute.length()) {
                    pObj->m_web_document_root = attribute;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "listening_ports")) {
                if (attribute.length()) {
                    pObj->m_web_listening_ports = attribute;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "index_files")) {
                if (attribute.length()) {
                    pObj->m_web_index_files = attribute;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "authentication_domain")) {
                if (attribute.length()) {
                    pObj->m_web_authentication_domain = attribute;
                }
            } else if (0 ==
                       vscp_strcasecmp(attr[i], "enable_auth_domain_check")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_enable_auth_domain_check = true;
                } else {
                    pObj->m_enable_auth_domain_check = false;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "ssl_certificat")) {
                if (attribute.length()) {
                    pObj->m_web_ssl_certificate = attribute;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "ssl_certificat_chain")) {
                if (attribute.length()) {
                    pObj->m_web_ssl_certificate_chain = attribute;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "ssl_verify_peer")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_web_ssl_verify_peer = true;
                } else {
                    pObj->m_web_ssl_verify_peer = false;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "ssl_ca_path")) {
                if (attribute.length()) {
                    pObj->m_web_ssl_ca_path = attribute;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "ssl_ca_file")) {
                if (attribute.length()) {
                    pObj->m_web_ssl_ca_file = attribute;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "ssl_verify_depth")) {
                if (attribute.length()) {
                    pObj->m_web_ssl_verify_depth =
                      vscp_readStringValue(attribute);
                }
            } else if (0 ==
                       vscp_strcasecmp(attr[i], "ssl_default_verify_paths")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_web_ssl_default_verify_paths = true;
                } else {
                    pObj->m_web_ssl_default_verify_paths = false;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "ssl_cipher_list")) {
                if (attribute.length()) {
                    pObj->m_web_ssl_cipher_list = attribute;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "ssl_protcol_version")) {
                if (attribute.length()) {
                    pObj->m_web_ssl_protocol_version =
                      vscp_readStringValue(attribute);
                }
            } else if (0 == vscp_strcasecmp(attr[i], "ssl_short_trust")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_web_ssl_short_trust = true;
                } else {
                    pObj->m_web_ssl_short_trust = false;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "cgi_interpreter")) {
                if (attribute.length()) {
                    pObj->m_web_cgi_interpreter = attribute;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "cgi_pattern")) {
                if (attribute.length()) {
                    pObj->m_web_cgi_patterns = attribute;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "cgi_environment")) {
                if (attribute.length()) {
                    pObj->m_web_cgi_environment = attribute;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "protect_uri")) {
                if (attribute.length()) {
                    pObj->m_web_protect_uri = attribute;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "trottle")) {
                if (attribute.length()) {
                    pObj->m_web_trottle = attribute;
                }
            } else if (0 ==
                       vscp_strcasecmp(attr[i], "enable_directory_listing")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_web_enable_directory_listing = true;
                } else {
                    pObj->m_web_enable_directory_listing = false;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "enable_keep_alive")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_web_enable_keep_alive = true;
                } else {
                    pObj->m_web_enable_keep_alive = false;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "keep_alive_timeout_ms")) {
                if (attribute.length()) {
                    pObj->m_web_keep_alive_timeout_ms =
                      vscp_readStringValue(attribute);
                }
            } else if (0 == vscp_strcasecmp(attr[i], "access_control_list")) {
                if (attribute.length()) {
                    pObj->m_web_access_control_list = attribute;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "extra_mime_types")) {
                if (attribute.length()) {
                    pObj->m_web_extra_mime_types = attribute;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "num_threads")) {
                if (attribute.length()) {
                    pObj->m_web_num_threads = vscp_readStringValue(attribute);
                }
            } else if (0 == vscp_strcasecmp(attr[i], "hide_file_pattern")) {
                if (attribute.length()) {
                    pObj->m_web_hide_file_patterns = attribute;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "run_as_user")) {
                if (attribute.length()) {
                    pObj->m_web_run_as_user = attribute;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "url_rewrite_patterns")) {
                if (attribute.length()) {
                    pObj->m_web_url_rewrite_patterns = attribute;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "hide_file_patterns")) {
                if (attribute.length()) {
                    pObj->m_web_hide_file_patterns = attribute;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "request_timeout_ms")) {
                if (attribute.length()) {
                    pObj->m_web_request_timeout_ms =
                      vscp_readStringValue(attribute);
                }
            } else if (0 == vscp_strcasecmp(attr[i], "linger_timeout_ms")) {
                if (attribute.length()) {
                    pObj->m_web_linger_timeout_ms =
                      vscp_readStringValue(attribute);
                }
            } else if (0 == vscp_strcasecmp(attr[i], "decode_url")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_web_decode_url = true;
                } else {
                    pObj->m_web_decode_url = false;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "global_auth_file")) {
                if (attribute.length()) {
                    pObj->m_web_global_auth_file = attribute;
                }
            } else if (0 == vscp_strcasecmp(attr[i],
                                            "web_per_directory_auth_file")) {
                if (attribute.length()) {
                    pObj->m_web_per_directory_auth_file = attribute;
                }
            } else if (0 == vscp_strcasecmp(attr[i],
                                            "access_control_allow_origin")) {
                if (attribute.length()) {
                    pObj->m_web_access_control_allow_methods = attribute;
                }
            } else if (0 == vscp_strcasecmp(attr[i],
                                            "access_control_allow_methods")) {
                if (attribute.length()) {
                    pObj->m_web_access_control_allow_methods = attribute;
                }
            } else if (0 == vscp_strcasecmp(attr[i],
                                            "access_control_allow_headers")) {
                if (attribute.length()) {
                    pObj->m_web_access_control_allow_headers = attribute;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "error_pages")) {
                if (attribute.length()) {
                    pObj->m_web_error_pages = attribute;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "tcp_nodelay")) {
                if (attribute.length()) {
                    pObj->m_web_linger_timeout_ms =
                      vscp_readStringValue(attribute);
                }
            } else if (0 == vscp_strcasecmp(attr[i], "static_file_max_age")) {
                if (attribute.length()) {
                    pObj->m_web_static_file_max_age =
                      vscp_readStringValue(attribute);
                }
            } else if (0 == vscp_strcasecmp(
                              attr[i], "strict_transport_security_max_age")) {
                if (attribute.length()) {
                    pObj->m_web_strict_transport_security_max_age =
                      vscp_readStringValue(attribute);
                }
            } else if (0 == vscp_strcasecmp(attr[i], "sendfile_call")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_web_allow_sendfile_call = true;
                } else {
                    pObj->m_web_allow_sendfile_call = false;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "additional_headers")) {
                if (attribute.length()) {
                    pObj->m_web_additional_header = attribute;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "max_request_size")) {
                if (attribute.length()) {
                    pObj->m_web_max_request_size =
                      vscp_readStringValue(attribute);
                }
            } else if (0 == vscp_strcasecmp(
                              attr[i], "web_allow_index_script_resource")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_web_allow_index_script_resource = true;
                } else {
                    pObj->m_web_allow_index_script_resource = false;
                }
            } else if (0 ==
                       vscp_strcasecmp(attr[i], "duktape_script_patterns")) {
                if (attribute.length()) {
                    pObj->m_web_duktape_script_patterns = attribute;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "lua_preload_file")) {
                if (attribute.length()) {
                    pObj->m_web_lua_preload_file = attribute;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "lua_script_patterns")) {
                if (attribute.length()) {
                    pObj->m_web_lua_script_patterns = attribute;
                }
            } else if (0 ==
                       vscp_strcasecmp(attr[i], "lua_server_page_patterns")) {
                if (attribute.length()) {
                    pObj->m_web_lua_server_page_patterns = attribute;
                }
            } else if (0 ==
                       vscp_strcasecmp(attr[i], "lua_websockets_patterns")) {
                if (attribute.length()) {
                    pObj->m_web_lua_websocket_patterns = attribute;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "lua_background_script")) {
                if (attribute.length()) {
                    pObj->m_web_lua_background_script = attribute;
                }
            } else if (0 == vscp_strcasecmp(attr[i],
                                            "lua_background_script_params")) {
                if (attribute.length()) {
                    pObj->m_web_lua_background_script_params = attribute;
                }
            }
        }
    } else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
               (0 == vscp_strcasecmp(name, "websockets"))) {

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "enable")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_bWebsocketsEnable = true;
                } else {
                    pObj->m_bWebsocketsEnable = false;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "document_root")) {
                if (attribute.length()) {
                    pObj->m_websocket_document_root = attribute;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "timeout_ms")) {
                if (attribute.length()) {
                    pObj->m_websocket_timeout_ms =
                      vscp_readStringValue(attribute);
                }
            }
        }
    } else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
               (0 == vscp_strcasecmp(name, "remoteuser"))) {
        bRemoteUserConfigFound = TRUE;
    } else if (bVscpConfigFound && bRemoteUserConfigFound &&
               (2 == depth_full_config_parser) &&
               (0 == vscp_strcasecmp(name, "user"))) {

        vscpEventFilter VSCPFilter;
        bool bFilterPresent = false;
        bool bMaskPresent   = false;
        std::string name;
        std::string md5;
        std::string privilege;
        std::string allowfrom;
        std::string allowevent;
        bool bUser = false;

        vscp_clearVSCPFilter(&VSCPFilter); // Allow all frames

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "name")) {
                name = attribute;
            } else if (0 == vscp_strcasecmp(attr[i], "password")) {
                md5 = attribute;
            } else if (0 == vscp_strcasecmp(attr[i], "privilege")) {
                privilege = attribute;
            } else if (0 == vscp_strcasecmp(attr[i], "allowfrom")) {
                allowfrom = attribute;
            } else if (0 == vscp_strcasecmp(attr[i], "allowevent")) {
                allowevent = attribute;
            } else if (0 == vscp_strcasecmp(attr[i], "filter")) {
                if (attribute.length()) {
                    if (vscp_readFilterFromString(&VSCPFilter, attribute)) {
                        bFilterPresent = true;
                    }
                }
            } else if (0 == vscp_strcasecmp(attr[i], "mask")) {
                if (attribute.length()) {
                    if (vscp_readMaskFromString(&VSCPFilter, attribute)) {
                        bMaskPresent = true;
                    }
                }
            }

            if (bFilterPresent && bMaskPresent) {
                pObj->m_userList.addUser(name,
                                         md5,
                                         "",
                                         "",
                                         &VSCPFilter,
                                         privilege,
                                         allowfrom,
                                         allowevent,
                                         VSCP_ADD_USER_FLAG_LOCAL);
            } else {
                pObj->m_userList.addUser(name,
                                         md5,
                                         "",
                                         "",
                                         NULL,
                                         privilege,
                                         allowfrom,
                                         allowevent,
                                         VSCP_ADD_USER_FLAG_LOCAL);
            }
        }
    } else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
               ((0 == vscp_strcasecmp(name, "level1driver")) ||
                (0 == vscp_strcasecmp(name, "canal1driver")))) {
        bLevel1DriverConfigFound = TRUE;
    } else if (bVscpConfigFound && bLevel1DriverConfigFound &&
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
                } else {
                    bEnabled = false;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "name")) {
                strName = attribute;
                // Replace spaces in name with underscore
                std::string::size_type found;
                while (std::string::npos !=
                       (found = strName.find_first_of(" "))) {
                    strName[found] = '_';
                }
            } else if (0 == vscp_strcasecmp(attr[i], "config")) {
                strConfig = attribute;
            } else if (0 ==
                       vscp_strcasecmp(attr[i], "parameter")) { // deprecated
                strConfig = attribute;
            } else if (0 == vscp_strcasecmp(attr[i], "path")) {
                strPath = attribute;
            } else if (0 == vscp_strcasecmp(attr[i], "flags")) {
                flags = vscp_readStringValue(attribute);
            } else if (0 == vscp_strcasecmp(attr[i], "guid")) {
                guid.getFromString(attribute);
            } else if (0 == vscp_strcasecmp(attr[i], "translation")) {
                translation = vscp_readStringValue(attribute);
            }
        } // for

        // Add the level I device
        if (bEnabled) {

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
            } else {
                syslog(LOG_DEBUG,
                       "Level I driver added. name = %s - [%s]",
                       strName.c_str(),
                       strPath.c_str());
            }
        }

    } else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
               ((0 == vscp_strcasecmp(name, "level2driver")))) {
        bLevel2DriverConfigFound = TRUE;
    } else if (bVscpConfigFound && bLevel2DriverConfigFound &&
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
                } else {
                    bEnabled = false;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "name")) {
                strName = attribute;
                // Replace spaces in name with underscore
                std::string::size_type found;
                while (std::string::npos !=
                       (found = strName.find_first_of(" "))) {
                    strName[found] = '_';
                }
            } else if (0 == vscp_strcasecmp(attr[i], "config")) {
                strConfig = attribute;
            } else if (0 ==
                       vscp_strcasecmp(attr[i], "parameter")) { // deprecated
                strConfig = attribute;
            } else if (0 == vscp_strcasecmp(attr[i], "path")) {
                strPath = attribute;
            } else if (0 == vscp_strcasecmp(attr[i], "guid")) {
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
                syslog(LOG_ERR,
                       "Level II driver was not added. name = %s"
                       "Path does not exist. - [%s]",
                       strName.c_str(),
                       strPath.c_str());

            } else {
                syslog(LOG_DEBUG,
                       "Level II driver added. name = %s- [%s]",
                       strName.c_str(),
                       strPath.c_str());
            }
        }
    } else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
               ((0 == vscp_strcasecmp(name, "level3driver")))) {
        bLevel3DriverConfigFound = TRUE;
    } else if (bVscpConfigFound && bLevel3DriverConfigFound &&
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
                } else {
                    bEnabled = false;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "name")) {
                strName = attribute;
                // Replace spaces in name with underscore
                std::string::size_type found;
                while (std::string::npos !=
                       (found = strName.find_first_of(" "))) {
                    strName[found] = '_';
                }
            } else if (0 == vscp_strcasecmp(attr[i], "config")) {
                strConfig = attribute;
            } else if (0 ==
                       vscp_strcasecmp(attr[i], "parameter")) { // deprecated
                strConfig = attribute;
            } else if (0 == vscp_strcasecmp(attr[i], "path")) {
                strPath = attribute;
            } else if (0 == vscp_strcasecmp(attr[i], "guid")) {
                guid.getFromString(attribute);
            }
        }

        // Add the level III device
        if (bEnabled) {

            if (!pObj->m_deviceList.addItem(strName,
                                            strConfig,
                                            strPath,
                                            0,
                                            guid,
                                            VSCP_DRIVER_LEVEL3,
                                            bEnabled)) {
                syslog(LOG_ERR,
                       "Level III driver was not added. name = %s"
                       "Path does not exist. - [%s]",
                       strName.c_str(),
                       strPath.c_str());

            } else {
                syslog(LOG_DEBUG,
                       "Level III driver added. name = %s- [%s]",
                       strName.c_str(),
                       strPath.c_str());
            }
        }
    } else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
               ((0 == vscp_strcasecmp(name, "knownnodes")))) {
        bKnownNodesConfigFound = TRUE;
    } else if (bVscpConfigFound && bKnownNodesConfigFound &&
               (2 == depth_full_config_parser) &&
               (0 == vscp_strcasecmp(name, "node"))) {

        std::string strName;
        cguid guidif;
        cguid guid;

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "name")) {
                strName = attribute;
            } else if (0 == vscp_strcasecmp(attr[i], "guid")) {
                guid.getFromString(attribute);
            } else if (0 == vscp_strcasecmp(attr[i], "if")) {
                guidif.getFromString(attribute);
            }
        }

        pObj->addKnownNode(guid, guidif, strName);

    } else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
               ((0 == vscp_strcasecmp(name, "tables")))) {
        bTablesConfigFound = TRUE;
    } else if (bVscpConfigFound && bTablesConfigFound &&
               (2 == depth_full_config_parser) &&
               (0 == vscp_strcasecmp(name, "table"))) {

        bool bEnabled = false;
        std::string strName;
        vscpTableType type = VSCP_TABLE_DYNAMIC;
        int size           = 0;
        bool bMemory       = false;

        std::string owner = "admin";
        uint16_t rights   = 0x700;
        std::string title;
        std::string xname;
        std::string yname;
        std::string note;
        std::string sqlcreate;
        std::string sqlinsert;
        std::string sqldelete;
        std::string description;

        uint16_t vscp_class      = 0;
        uint16_t vscp_type       = 0;
        uint8_t vscp_sensorindex = 0;
        uint8_t vscp_unit        = 0;
        uint8_t vscp_zone        = 255;
        uint8_t vscp_subzone     = 255;

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "enable")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    bEnabled = true;
                } else {
                    bEnabled = false;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "name")) {
                strName = attribute;
                // Replace spaces in name with underscore
                std::string::size_type found;
                while (std::string::npos !=
                       (found = strName.find_first_of(" "))) {
                    strName[found] = '_';
                }
            } else if (0 == vscp_strcasecmp(attr[i], "type")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "static")) {
                    type = VSCP_TABLE_STATIC;
                }
            } else if (0 == vscp_strcasecmp(attr[i], "size")) {
                size = vscp_readStringValue(attribute);
            } else if (0 == vscp_strcasecmp(attr[i], "bmemory")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    bMemory = true;
                } else {
                    bMemory = false;
                }

            } else if (0 == vscp_strcasecmp(attr[i], "owner")) {
                owner = attribute;
            } else if (0 == vscp_strcasecmp(attr[i], "rights")) {
                rights = vscp_readStringValue(attribute);
            } else if (0 == vscp_strcasecmp(attr[i], "title")) {
                title = attribute;
            } else if (0 == vscp_strcasecmp(attr[i], "labelx")) {
                xname = attribute;
            } else if (0 == vscp_strcasecmp(attr[i], "lavely")) {
                yname = attribute;
            } else if (0 == vscp_strcasecmp(attr[i], "note")) {
                note = attribute;
            } else if (0 == vscp_strcasecmp(attr[i], "sqlcreate")) {
                sqlcreate = attribute;
            } else if (0 == vscp_strcasecmp(attr[i], "sqldelete")) {
                sqldelete = attribute;
            } else if (0 == vscp_strcasecmp(attr[i], "sqlinsert")) {
                sqlinsert = attribute;
            } else if (0 == vscp_strcasecmp(attr[i], "description")) {
                description = attribute;
            } else if (0 == vscp_strcasecmp(attr[i], "vscpclass")) {
                vscp_class = vscp_readStringValue(attribute);
                ;
            } else if (0 == vscp_strcasecmp(attr[i], "vscptype")) {
                vscp_type = vscp_readStringValue(attribute);
                ;
            } else if (0 == vscp_strcasecmp(attr[i], "vscpsensorindex")) {
                vscp_sensorindex = vscp_readStringValue(attribute);
                ;
            } else if (0 == vscp_strcasecmp(attr[i], "vscpunit")) {
                vscp_unit = vscp_readStringValue(attribute);
                ;
            } else if (0 == vscp_strcasecmp(attr[i], "vscpzone")) {
                vscp_zone = vscp_readStringValue(attribute);
                ;
            } else if (0 == vscp_strcasecmp(attr[i], "vscpsubzone")) {
                vscp_subzone = vscp_readStringValue(attribute);
                ;
            }
        }

        CVSCPTable *pTable = new CVSCPTable(
          pObj->m_rootFolder + "table/", strName, true, bMemory, type, size);
        if (NULL == pTable) {
            syslog(LOG_ERR, "Unable to create table %s", strName.c_str());
            return;
        }

        if (!pTable->setTableInfo(owner,
                                  rights,
                                  title,
                                  xname,
                                  yname,
                                  note,
                                  sqlcreate,
                                  sqlinsert,
                                  sqldelete,
                                  description)) {
            syslog(LOG_ERR,
                   "Unable to set table info for table %s",
                   strName.c_str());
            delete pTable;
            return;
        }

        pTable->setTableEventInfo(vscp_class,
                                  vscp_type,
                                  vscp_sensorindex,
                                  vscp_unit,
                                  vscp_zone,
                                  vscp_subzone);

        // Add the table
        if (!pObj->m_userTableObjects.addTable(pTable)) {
            delete pTable;
            syslog(LOG_ERR,
                   "Could not add new table (name conflict?)! nane=%s",
                   strName.c_str());
        };

    } else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
               ((0 == vscp_strcasecmp(name, "automation")))) {
        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == vscp_strcasecmp(attr[i], "enable")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_automation.enableAutomation();
                } else {
                    pObj->m_automation.disableAutomation();
                }
            } else if (0 == vscp_strcasecmp(attr[i], "zone")) {
                uint8_t zone = vscp_readStringValue(attribute);
                pObj->m_automation.setZone(zone);
            } else if (0 == vscp_strcasecmp(attr[i], "subzone")) {
                uint8_t subzone = vscp_readStringValue(attribute);
                pObj->m_automation.setSubzone(subzone);
            } else if (0 == vscp_strcasecmp(attr[i], "longitude")) {
                // Decimal point should be '.'
                std::string::size_type found;
                while (std::string::npos !=
                       (found = attribute.find_first_of(","))) {
                    attribute[found] = '.';
                }
                double d = std::stod(attribute);
                pObj->m_automation.setLongitude(d);
            } else if (0 == vscp_strcasecmp(attr[i], "latitude")) {
                // Decimal point should be '.'
                std::string::size_type found;
                while (std::string::npos !=
                       (found = attribute.find_first_of(","))) {
                    attribute[found] = '.';
                }
                double d = std::stod(attribute);
                pObj->m_automation.setLatitude(d);
            } else if (0 == vscp_strcasecmp(attr[i], "sunrise-event")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_automation.enableSunRiseEvent();
                } else {
                    pObj->m_automation.disableSunRiseEvent();
                }
            } else if (0 ==
                       vscp_strcasecmp(attr[i], "sunrise-twilight-event")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_automation.enableSunRiseTwilightEvent();
                } else {
                    pObj->m_automation.disableSunRiseTwilightEvent();
                }
            } else if (0 == vscp_strcasecmp(attr[i], "sunset-event")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_automation.enableSunSetEvent();
                } else {
                    pObj->m_automation.disableSunSetEvent();
                }
            } else if (0 == vscp_strcasecmp(attr[i], "sunset-twilight-event")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_automation.enableSunSetTwilightEvent();
                } else {
                    pObj->m_automation.disableSunSetTwilightEvent();
                }
            } else if (0 ==
                       vscp_strcasecmp(attr[i], "segment-controler-event")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_automation.enableSegmentControllerHeartbeat();
                } else {
                    pObj->m_automation.disableSegmentControllerHeartbeat();
                }
            } else if (0 ==
                       vscp_strcasecmp(attr[i], "segment-controler-interval")) {
                int interval = vscp_readStringValue(attribute);
                pObj->m_automation.setSegmentControllerHeartbeatInterval(
                  interval);
            } else if (0 == vscp_strcasecmp(attr[i], "heartbeat-event")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_automation.enableHeartbeatEvent();
                } else {
                    pObj->m_automation.disableHeartbeatEvent();
                }
            } else if (0 == vscp_strcasecmp(attr[i], "heartbeat-interval")) {
                int interval = vscp_readStringValue(attribute);
                pObj->m_automation.setHeartbeatEventInterval(interval);
            } else if (0 == vscp_strcasecmp(attr[i], "capability-event")) {
                if (0 == vscp_strcasecmp(attribute.c_str(), "true")) {
                    pObj->m_automation.enableCapabilitiesEvent();
                } else {
                    pObj->m_automation.disableCapabilitiesEvent();
                }
            } else if (0 == vscp_strcasecmp(attr[i], "capability-interval")) {
                int interval = vscp_readStringValue(attribute);
                pObj->m_automation.setCapabilitiesEventInterval(interval);
            }
        }
    }

    depth_full_config_parser++;
}

static void
handleFullConfigData(void *data, const char *content, int length)
{
    int prevLength =
      (NULL == last_full_config_content) ? 0 : strlen(last_full_config_content);
    char *tmp = (char *)malloc(length + 1 + prevLength);
    strncpy(tmp, content, length);
    tmp[length] = '\0';

    if (NULL == last_full_config_content) {
        tmp = (char *)malloc(length + 1);
        strncpy(tmp, content, length);
        tmp[length]              = '\0';
        last_full_config_content = tmp;
    } else {
        // Concatenate
        int newlen = length + 1 + strlen(last_full_config_content);
        last_full_config_content =
          (char *)realloc(last_full_config_content, newlen);
        strncat(tmp, content, length);
        last_full_config_content[newlen] = '\0';
    }
}

static void
endFullConfigParser(void *data, const char *name)
{
    /*if (NULL != last_full_config_content) {
        // Free the allocated data
        free(last_full_config_content);
        last_full_config_content = NULL;
    }*/

    depth_full_config_parser--;

    if (1 == depth_full_config_parser &&
        (0 == vscp_strcasecmp(name, "vscpconfig"))) {
        bVscpConfigFound = FALSE;
    }
    if (bVscpConfigFound && (1 == depth_full_config_parser) &&
        (0 == vscp_strcasecmp(name, "udp"))) {
        bUDPConfigFound = FALSE;
    } else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
               (0 == vscp_strcasecmp(name, "multicast"))) {
        bMulticastConfigFound = FALSE;
    } else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
               (0 == vscp_strcasecmp(name, "remoteuser"))) {
        bRemoteUserConfigFound = FALSE;
    } else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
               ((0 == vscp_strcasecmp(name, "level1driver")) ||
                (0 == vscp_strcasecmp(name, "canal1driver")))) {
        bLevel1DriverConfigFound = FALSE;
    } else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
               (0 == vscp_strcasecmp(name, "level2driver"))) {
        bLevel2DriverConfigFound = FALSE;
    } else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
               (0 == vscp_strcasecmp(name, "level3driver"))) {
        bLevel3DriverConfigFound = FALSE;
    } else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
               ((0 == vscp_strcasecmp(name, "knownnodes")))) {
        bKnownNodesConfigFound = FALSE;
    } else if (bVscpConfigFound && (1 == depth_full_config_parser) &&
               ((0 == vscp_strcasecmp(name, "tables")))) {
        bTablesConfigFound = FALSE;
    }
}

// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// readConfigurationXML
//
// Read the configuration XML file
//

bool
CControlObject::readConfigurationXML(const std::string &strcfgfile)
{
    FILE *fp;

    syslog(LOG_DEBUG,
           "Reading full XML configuration from [%s]",
           (const char *)strcfgfile.c_str());

    fp = fopen(strcfgfile.c_str(), "r");
    if (NULL == fp) {
        syslog(LOG_CRIT,
               "Failed to open configuration file [%s]",
               strcfgfile.c_str());
        return false;
    }

    XML_Parser xmlParser = XML_ParserCreate("UTF-8");
    XML_SetUserData(xmlParser, this);
    XML_SetElementHandler(
      xmlParser, startFullConfigParser, endFullConfigParser);
    // XML_SetCharacterDataHandler(xmlParser, handleFullConfigData);

    int bytes_read;
    void *buf = XML_GetBuffer(xmlParser, XML_BUFF_SIZE);
    if (NULL == buf) {
        XML_ParserFree(xmlParser);
        fclose(fp);
        syslog(LOG_CRIT,
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
// isDbTableExist
//

bool
CControlObject::isDbTableExist(sqlite3 *db, const std::string &strTblName)
{
    sqlite3_stmt *pSelectStatement = NULL;
    int iResult                    = SQLITE_ERROR;
    bool rv                        = false;

    // Database file must be open
    if (NULL == db) {
        syslog(LOG_ERR, "isDbFieldExistent. Database file is not open.");
        return false;
    }

    std::string sql =
      "SELECT name FROM sqlite_master WHERE type='table' AND name='%s'";
    sql = vscp_str_format(sql, (const char *)strTblName.c_str());

    iResult = sqlite3_prepare16_v2(
      db, (const char *)sql.c_str(), -1, &pSelectStatement, 0);

    if ((iResult == SQLITE_OK) && (pSelectStatement != NULL)) {

        iResult = sqlite3_step(pSelectStatement);

        // was found?
        if (iResult == SQLITE_ROW) {
            rv = true;
            sqlite3_clear_bindings(pSelectStatement);
            sqlite3_reset(pSelectStatement);
        }

        iResult = sqlite3_finalize(pSelectStatement);
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// isDbFieldExist
//

bool
CControlObject::isDbFieldExist(sqlite3 *db,
                               const std::string &strTblName,
                               const std::string &strFieldName)
{
    bool rv = false;
    sqlite3_stmt *ppStmt;
    char *pErrMsg = 0;

    // Database file must be open
    if (NULL == db) {
        syslog(LOG_ERR, "isDbFieldExist. Database file is not open.");
        return false;
    }

    std::string sql = "PRAGMA table_info(%s);";
    sql             = vscp_str_format(sql, (const char *)strTblName.c_str());

    if (SQLITE_OK !=
        sqlite3_prepare(
          m_db_vscp_daemon, (const char *)sql.c_str(), -1, &ppStmt, NULL)) {
        syslog(LOG_ERR,
               "isDbFieldExist: Failed to read VSCP settings database - "
               "prepare query.");
        return false;
    }

    while (SQLITE_ROW == sqlite3_step(ppStmt)) {

        const unsigned char *p;

        // Get column name
        if (NULL == (p = sqlite3_column_text(ppStmt, 1))) {
            continue;
        }

        // database version
        if (0 == vscp_strcasecmp((const char *)p,
                                 (const char *)strFieldName.c_str())) {
            rv = true;
            break;
        }
    }

    sqlite3_finalize(ppStmt);

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// updateConfigurationRecordName
//

bool
CControlObject::updateConfigurationRecordName(const std::string &strName,
                                              const std::string &strNewName)
{
    char *pErrMsg;

    // Database file must be open
    if (NULL == m_db_vscp_daemon) {
        syslog(LOG_ERR,
               "Settings update: Update record. Database file is not open.");
        return false;
    }

    pthread_mutex_lock(&m_db_vscp_configMutex);

    char *sql = sqlite3_mprintf(VSCPDB_CONFIG_UPDATE_CONFIG_NAME,        
                                (const char *)strNewName.c_str(),
                                (const char *)strName.c_str(),
                                m_nConfiguration);
    if (SQLITE_OK !=
        sqlite3_exec(m_db_vscp_daemon, sql, NULL, NULL, &pErrMsg)) {
        sqlite3_free(sql);
        pthread_mutex_unlock(&m_db_vscp_configMutex);
        syslog(LOG_ERR, "Failed to update setting with error %s.", pErrMsg);
        return false;
    }

    sqlite3_free(sql);

    pthread_mutex_unlock(&m_db_vscp_configMutex);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// updateConfigurationRecordItem
//

bool
CControlObject::updateConfigurationRecordItem(const std::string &strName,
                                              const std::string &strValue)
{
    char *pErrMsg;

    // Database file must be open
    if (NULL == m_db_vscp_daemon) {
        syslog(LOG_ERR,
               "Settings update: Update record. Database file is not open.");
        return false;
    }

    pthread_mutex_lock(&m_db_vscp_configMutex);

    char *sql = sqlite3_mprintf(VSCPDB_CONFIG_UPDATE_ITEM,
                                (const char *)strValue.c_str(),
                                (const char *)strName.c_str(),
                                m_nConfiguration);
    if (SQLITE_OK !=
        sqlite3_exec(m_db_vscp_daemon, sql, NULL, NULL, &pErrMsg)) {
        sqlite3_free(sql);
        pthread_mutex_unlock(&m_db_vscp_configMutex);
        syslog(LOG_ERR, "Failed to update setting with error %s.", pErrMsg);
        return false;
    }

    sqlite3_free(sql);

    pthread_mutex_unlock(&m_db_vscp_configMutex);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getConfigurationValueFromDatabase
//

bool
CControlObject::getConfigurationValueFromDatabase(const char *pName,
                                                  char *pBuf,
                                                  size_t len)
{
    sqlite3_stmt *ppStmt;
    char *pErrMsg = 0;
    char *psql;

    // Check if database is open
    if (NULL == m_db_vscp_daemon) return false;

    pthread_mutex_lock(&m_db_vscp_configMutex);

    // Check if the variable is defined already
    //      if it is - just return true
    psql = sqlite3_mprintf(VSCPDB_CONFIG_FIND_ITEM, pName);
    if (SQLITE_OK !=
        sqlite3_prepare(m_db_vscp_daemon, psql, -1, &ppStmt, NULL)) {
        sqlite3_free(psql);
        syslog(LOG_ERR, "Failed to find %s in configuration database", pName);
        pthread_mutex_unlock(&m_db_vscp_configMutex);
        return false;
    }

    sqlite3_free(psql);

    if (SQLITE_ROW == sqlite3_step(ppStmt)) {

        const unsigned char *p = NULL;

        if (NULL ==
            (p = sqlite3_column_text(ppStmt, VSCPDB_ORDINAL_CONFIG_VALUE))) {
            syslog(LOG_ERR,
                   "getConfigurationValueFromDatabase: Failed to read 'value' "
                   "for %s "
                   "from settings record.",
                   pName);
            sqlite3_finalize(ppStmt);
            pthread_mutex_unlock(&m_db_vscp_configMutex);
            return false;
        }

        // Copy in data
        strncpy(pBuf, (const char *)p, std::min(len, strlen((const char *)p)));

        sqlite3_finalize(ppStmt);
    }

    pthread_mutex_unlock(&m_db_vscp_configMutex);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// addConfigurationValueToDatabase
//

bool
CControlObject::addConfigurationValueToDatabase(const char *pName,
                                                const char *pValue)
{
    sqlite3_stmt *ppStmt;
    char *pErrMsg = 0;
    char *psql;

    // Check if database is open
    if (NULL == m_db_vscp_daemon) return false;

    // Check if the variable is defined already
    //      if it is - just return true
    psql = sqlite3_mprintf(VSCPDB_CONFIG_FIND_ITEM, pName);
    if (SQLITE_OK !=
        sqlite3_prepare(m_db_vscp_daemon, psql, -1, &ppStmt, NULL)) {
        sqlite3_free(psql);
        syslog(
          LOG_ERR,
          "Failed to check if %s = %s is already in configuration database",
          pName,
          pValue);
        return false;
    }

    sqlite3_free(psql);

    if (SQLITE_ROW == sqlite3_step(ppStmt)) {
        return true; // Record is there already
    }

    pthread_mutex_lock(&m_db_vscp_configMutex);

    syslog(LOG_DEBUG, "Add %s = %s to configuration database", pName, pValue);

    // Create settings in db
    psql = sqlite3_mprintf(VSCPDB_CONFIG_INSERT, pName, pValue);

    if (SQLITE_OK !=
        sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg)) {

        sqlite3_free(psql);
        pthread_mutex_unlock(&m_db_vscp_configMutex);

        syslog(LOG_ERR,
               "Inserting new entry into configuration database failed with "
               "message %s",
               pErrMsg);
        return false;
    }

    sqlite3_free(psql);
    pthread_mutex_unlock(&m_db_vscp_configMutex);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// addDefaultConfigValues
//

void
CControlObject::addDefaultConfigValues(void)
{
    // Add default settings (set as defaults in SQL create expression))
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_DBVERSION,
                                    VSCPDB_CONFIG_CURRENT_DBVERSION);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_CLIENTBUFFERSIZE,
                                    VSCPDB_CONFIG_DEFAULT_CLIENTBUFFERSIZE);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_GUID,
                                    VSCPDB_CONFIG_DEFAULT_GUID);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_SERVERNAME,
                                    VSCPDB_CONFIG_DEFAULT_SERVERNAME);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_ANNOUNCE_ADDR,
                                    VSCPDB_CONFIG_DEFAULT_ANNOUNCE_ADDR);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_ANNOUNCE_TTL,
                                    VSCPDB_CONFIG_DEFAULT_ANNOUNCE_TTL);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_PATH_DB_EVENTS,
                                    VSCPDB_CONFIG_DEFAULT_PATH_DB_EVENTS);

    // UDP
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_UDP_ENABLE,
                                    VSCPDB_CONFIG_DEFAULT_UDP_ENABLE);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_UDP_ADDR,
                                    VSCPDB_CONFIG_DEFAULT_UDP_ADDR);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_UDP_USER,
                                    VSCPDB_CONFIG_DEFAULT_UDP_USER);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_UDP_PASSWORD,
                                    VSCPDB_CONFIG_DEFAULT_UDP_PASSWORD);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_UDP_UNSECURE_ENABLE,
                                    VSCPDB_CONFIG_DEFAULT_UDP_UNSECURE_ENABLE);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_UDP_FILTER,
                                    VSCPDB_CONFIG_DEFAULT_UDP_FILTER);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_UDP_MASK,
                                    VSCPDB_CONFIG_DEFAULT_UDP_MASK);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_UDP_GUID,
                                    VSCPDB_CONFIG_DEFAULT_UDP_GUID);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_UDP_ACK_ENABLE,
                                    VSCPDB_CONFIG_DEFAULT_UDP_ACK_ENABLE);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_MULTICAST_ENABLE,
                                    VSCPDB_CONFIG_DEFAULT_MULTICAST_ENABLE);

    // TCP/IP
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_TCPIP_ADDR,
                                    VSCPDB_CONFIG_DEFAULT_TCPIP_ADDR);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_TCPIP_ENCRYPTION,
                                    VSCPDB_CONFIG_DEFAULT_TCPIP_ENCRYPTION);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_TCPIP_SSL_CERTIFICATE,
      VSCPDB_CONFIG_DEFAULT_TCPIP_SSL_CERTIFICATE);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_TCPIP_SSL_CERTIFICAT_CHAIN,
      VSCPDB_CONFIG_DEFAULT_TCPIP_SSL_CERTIFICAT_CHAIN);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_TCPIP_SSL_VERIFY_PEER,
      VSCPDB_CONFIG_DEFAULT_TCPIP_SSL_VERIFY_PEER);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_TCPIP_SSL_CA_PATH,
                                    VSCPDB_CONFIG_DEFAULT_TCPIP_SSL_CA_PATH);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_TCPIP_SSL_CA_FILE,
                                    VSCPDB_CONFIG_DEFAULT_TCPIP_SSL_CA_FILE);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_TCPIP_SSL_VERIFY_DEPTH,
      VSCPDB_CONFIG_DEFAULT_TCPIP_SSL_VERIFY_DEPTH);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_TCPIP_SSL_DEFAULT_VERIFY_PATHS,
      VSCPDB_CONFIG_DEFAULT_TCPIP_SSL_DEFAULT_VERIFY_PATHS);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_TCPIP_SSL_CHIPHER_LIST,
      VSCPDB_CONFIG_DEFAULT_TCPIP_SSL_CHIPHER_LIST);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_TCPIP_SSL_PROTOCOL_VERSION,
      VSCPDB_CONFIG_DEFAULT_TCPIP_SSL_PROTOCOL_VERSION);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_TCPIP_SSL_SHORT_TRUST,
      VSCPDB_CONFIG_DEFAULT_TCPIP_SSL_SHORT_TRUST);

    // DM
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_DM_PATH_DB,
                                    VSCPDB_CONFIG_DEFAULT_DM_PATH_DB);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_DM_PATH_XML,
                                    VSCPDB_CONFIG_DEFAULT_DM_PATH_XML);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_DM_ALLOW_XML_SAVE,
                                    VSCPDB_CONFIG_DEFAULT_DM_ALLOW_XML_SAVE);

    // Variables
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_VARIABLES_PATH_DB,
                                    VSCPDB_CONFIG_DEFAULT_VARIABLES_PATH_DB);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_VARIABLES_PATH_XML,
                                    VSCPDB_CONFIG_DEFAULT_VARIABLES_PATH_XML);

    // WEB server
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_WEB_ENABLE,
                                    VSCPDB_CONFIG_DEFAULT_WEB_ENABLE);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_WEB_DOCUMENT_ROOT,
                                    VSCPDB_CONFIG_DEFAULT_WEB_DOCUMENT_ROOT);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_WEB_LISTENING_PORTS,
                                    VSCPDB_CONFIG_DEFAULT_WEB_LISTENING_PORTS);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_WEB_INDEX_FILES,
                                    VSCPDB_CONFIG_DEFAULT_WEB_INDEX_FILES);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_WEB_AUTHENTICATION_DOMAIN,
      VSCPDB_CONFIG_DEFAULT_WEB_AUTHENTICATION_DOMAIN);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_WEB_ENABLE_AUTH_DOMAIN_CHECK,
      VSCPDB_CONFIG_DEFAULT_WEB_ENABLE_AUTH_DOMAIN_CHECK);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_WEB_SSL_CERTIFICATE,
                                    VSCPDB_CONFIG_DEFAULT_WEB_SSL_CERTIFICATE);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_WEB_SSL_CERTIFICAT_CHAIN,
      VSCPDB_CONFIG_DEFAULT_WEB_SSL_CERTIFICAT_CHAIN);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_WEB_SSL_VERIFY_PEER,
                                    VSCPDB_CONFIG_DEFAULT_WEB_SSL_VERIFY_PEER);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_WEB_SSL_CA_PATH,
                                    VSCPDB_CONFIG_DEFAULT_WEB_SSL_CA_PATH);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_WEB_SSL_CA_FILE,
                                    VSCPDB_CONFIG_DEFAULT_WEB_SSL_CA_FILE);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_WEB_SSL_VERIFY_DEPTH,
                                    VSCPDB_CONFIG_DEFAULT_WEB_SSL_VERIFY_DEPTH);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_WEB_SSL_DEFAULT_VERIFY_PATHS,
      VSCPDB_CONFIG_DEFAULT_WEB_SSL_DEFAULT_VERIFY_PATHS);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_WEB_SSL_CHIPHER_LIST,
                                    VSCPDB_CONFIG_DEFAULT_WEB_SSL_CHIPHER_LIST);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_WEB_SSL_PROTOCOL_VERSION,
      VSCPDB_CONFIG_DEFAULT_WEB_SSL_PROTOCOL_VERSION);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_WEB_SSL_SHORT_TRUST,
                                    VSCPDB_CONFIG_DEFAULT_WEB_SSL_SHORT_TRUST);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_WEB_CGI_PATTERN,
                                    VSCPDB_CONFIG_DEFAULT_WEB_CGI_PATTERN);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_WEB_CGI_INTERPRETER,
                                    VSCPDB_CONFIG_DEFAULT_WEB_CGI_INTERPRETER);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_WEB_CGI_ENVIRONMENT,
                                    VSCPDB_CONFIG_DEFAULT_WEB_CGI_ENVIRONMENT);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_WEB_PROTECT_URI,
                                    VSCPDB_CONFIG_DEFAULT_WEB_PROTECT_URI);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_WEB_TROTTLE,
                                    VSCPDB_CONFIG_DEFAULT_WEB_TROTTLE);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_WEB_ENABLE_DIRECTORY_LISTING,
      VSCPDB_CONFIG_DEFAULT_WEB_ENABLE_DIRECTORY_LISTING);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_WEB_ENABLE_KEEP_ALIVE,
      VSCPDB_CONFIG_DEFAULT_WEB_ENABLE_KEEP_ALIVE);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_LIST,
      VSCPDB_CONFIG_DEFAULT_WEB_ACCESS_CONTROL_LIST);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_WEB_EXTRA_MIME_TYPES,
                                    VSCPDB_CONFIG_DEFAULT_WEB_EXTRA_MIME_TYPES);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_WEB_NUM_THREADS,
                                    VSCPDB_CONFIG_DEFAULT_WEB_NUM_THREADS);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_WEB_HIDE_FILE_PATTERNS,
      VSCPDB_CONFIG_DEFAULT_WEB_HIDE_FILE_PATTERNS);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_WEB_RUN_AS_USER,
                                    VSCPDB_CONFIG_DEFAULT_WEB_RUN_AS_USER);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_WEB_URL_REWRITE_PATTERNS,
      VSCPDB_CONFIG_DEFAULT_WEB_URL_REWRITE_PATTERNS);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_WEB_REQUEST_TIMEOUT_MS,
      VSCPDB_CONFIG_DEFAULT_WEB_REQUEST_TIMEOUT_MS);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_WEB_LINGER_TIMEOUT_MS,
      VSCPDB_CONFIG_DEFAULT_WEB_LINGER_TIMEOUT_MS);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_WEB_DECODE_URL,
                                    VSCPDB_CONFIG_DEFAULT_WEB_DECODE_URL);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_WEB_GLOBAL_AUTHFILE,
                                    VSCPDB_CONFIG_DEFAULT_WEB_GLOBAL_AUTHFILE);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_WEB_PER_DIRECTORY_AUTH_FILE,
      VSCPDB_CONFIG_DEFAULT_WEB_PER_DIRECTORY_AUTH_FILE);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_WEB_SSI_PATTERNS,
                                    VSCPDB_CONFIG_DEFAULT_WEB_SSI_PATTERNS);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_ALLOW_ORIGIN,
      VSCPDB_CONFIG_DEFAULT_WEB_ACCESS_CONTROL_ALLOW_ORIGIN);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_ALLOW_METHODS,
      VSCPDB_CONFIG_DEFAULT_WEB_ACCESS_CONTROL_ALLOW_METHODS);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_ALLOW_HEADERS,
      VSCPDB_CONFIG_DEFAULT_WEB_ACCESS_CONTROL_ALLOW_HEADERS);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_WEB_ERROR_PAGES,
                                    VSCPDB_CONFIG_DEFAULT_WEB_ERROR_PAGES);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_WEB_TCP_NO_DELAY,
                                    VSCPDB_CONFIG_DEFAULT_WEB_TCP_NO_DELAY);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_WEB_STATIC_FILE_MAX_AGE,
      VSCPDB_CONFIG_DEFAULT_WEB_STATIC_FILE_MAX_AGE);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_WEB_STRICT_TRANSPORT_SECURITY_MAX_AGE,
      VSCPDB_CONFIG_DEFAULT_WEB_STRICT_TRANSPORT_SECURITY_MAX_AGE);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_WEB_ALLOW_SENDFILE_CALL,
      VSCPDB_CONFIG_DEFAULT_WEB_ALLOW_SENDFILE_CALL);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_WEB_ADDITIONAL_HEADERS,
      VSCPDB_CONFIG_DEFAULT_WEB_ADDITIONAL_HEADERS);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_WEB_MAX_REQUEST_SIZE,
                                    VSCPDB_CONFIG_DEFAULT_WEB_MAX_REQUEST_SIZE);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_WEB_ALLOW_INDEX_SCRIPT_RESOURCE,
      VSCPDB_CONFIG_DEFAULT_WEB_ALLOW_INDEX_SCRIPT_RESOURCE);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_WEB_DUKTAPE_SCRIPT_PATTERN,
      VSCPDB_CONFIG_DEFAULT_WEB_DUKTAPE_SCRIPT_PATTERN);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_WEB_LUA_PRELOAD_FILE,
                                    VSCPDB_CONFIG_DEFAULT_WEB_LUA_PRELOAD_FILE);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_WEB_LUA_SCRIPT_PATTERN,
      VSCPDB_CONFIG_DEFAULT_WEB_LUA_SCRIPT_PATTERN);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_WEB_LUA_SERVER_PAGE_PATTERN,
      VSCPDB_CONFIG_DEFAULT_WEB_LUA_SERVER_PAGE_PATTERN);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_WEB_LUA_WEBSOCKET_PATTERN,
      VSCPDB_CONFIG_DEFAULT_WEB_LUA_WEBSOCKET_PATTERN);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_WEB_LUA_BACKGROUND_SCRIPT,
      VSCPDB_CONFIG_DEFAULT_WEB_LUA_BACKGROUND_SCRIPT);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_WEB_LUA_BACKGROUND_SCRIPT_PARAMS,
      VSCPDB_CONFIG_DEFAULT_WEB_LUA_BACKGROUND_SCRIPT_PARAMS);

    // Websockets
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_WEBSOCKET_ENABLE,
                                    VSCPDB_CONFIG_DEFAULT_WEBSOCKET_ENABLE);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_WEBSOCKET_DOCUMENT_ROOT,
      VSCPDB_CONFIG_DEFAULT_WEBSOCKET_DOCUMENT_ROOT);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_WEBSOCKET_TIMEOUT_MS,
                                    VSCPDB_CONFIG_DEFAULT_WEBSOCKET_TIMEOUT_MS);

    // Automation
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_AUTOMATION_ENABLE,
                                    VSCPDB_CONFIG_DEFAULT_AUTOMATION_ENABLE);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_AUTOMATION_ZONE,
                                    VSCPDB_CONFIG_DEFAULT_AUTOMATION_ZONE);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_AUTOMATION_SUBZONE,
                                    VSCPDB_CONFIG_DEFAULT_AUTOMATION_SUBZONE);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_AUTOMATION_LONGITUDE,
                                    VSCPDB_CONFIG_DEFAULT_AUTOMATION_LONGITUDE);
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_AUTOMATION_LATITUDE,
                                    VSCPDB_CONFIG_DEFAULT_AUTOMATION_LATITUDE);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_AUTOMATION_SUNRISE_ENABLE,
      VSCPDB_CONFIG_DEFAULT_AUTOMATION_SUNRISE_ENABLE);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_AUTOMATION_SUNSET_ENABLE,
      VSCPDB_CONFIG_DEFAULT_AUTOMATION_SUNSET_ENABLE);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_AUTOMATION_SUNSET_TWILIGHT_ENABLE,
      VSCPDB_CONFIG_DEFAULT_AUTOMATION_SUNSET_TWILIGHT_ENABLE);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_AUTOMATION_SUNRISE_TWILIGHT_ENABLE,
      VSCPDB_CONFIG_DEFAULT_AUTOMATION_SUNRISE_TWILIGHT_ENABLE);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_AUTOMATION_SEGMENT_CTRL_ENABLE,
      VSCPDB_CONFIG_DEFAULT_AUTOMATION_SEGMENT_CTRL_ENABLE);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_AUTOMATION_SEGMENT_CTRL_INTERVAL,
      VSCPDB_CONFIG_DEFAULT_AUTOMATION_SEGMENT_CTRL_INTERVAL);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_AUTOMATION_HEARTBEAT_ENABLE,
      VSCPDB_CONFIG_DEFAULT_AUTOMATION_HEARTBEAT_ENABLE);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_AUTOMATION_CAPABILITIES_ENABLE,
      VSCPDB_CONFIG_DEFAULT_AUTOMATION_CAPABILITIES_ENABLE);
    addConfigurationValueToDatabase(
      VSCPDB_CONFIG_NAME_AUTOMATION_CAPABILITIES_INTERVAL,
      VSCPDB_CONFIG_DEFAULT_AUTOMATION_CAPABILITIES_INTERVAL);
}

///////////////////////////////////////////////////////////////////////////////
// updateConfigDb
//

bool
CControlObject::updateConfigDb()
{
    char buf[10];

    // Check if we have a version 1 database
    getConfigurationValueFromDatabase(
      VSCPDB_CONFIG_NAME_DBVERSION, buf, sizeof(buf));

    int version = atoi(buf);
    if (0 == version) return false;

    if (1 == version) {

        syslog(LOG_INFO,"Updating configuration database vscpd.slite3 from version 1 to version 2");

        // Update field names that have changed
        updateConfigurationRecordName("client_buffer_size",
                                      "client-buffer-size");
        updateConfigurationRecordName("announceinterface_address",
                                      "announceinterface-address");
        updateConfigurationRecordName("announceinterface_ttl",
                                      "announceinterface-ttl");
        updateConfigurationRecordName("tcpipinterface_address",
                                      "tcpipinterface-address");
        updateConfigurationRecordName("tcpip_encryption", "tcpip-encryption");
        updateConfigurationRecordName("tcpip_ssl_certificate",
                                      "tcpip-ssl-certificate");
        updateConfigurationRecordName("tcpip_ssl_certificate_chain",
                                      "tcpip-ssl-certificate-chain");
        updateConfigurationRecordName("tcpip_ssl_verify_peer",
                                      "tcpip-ssl-verify-peer");
        updateConfigurationRecordName("tcpip_ssl_ca_path", "tcpip-ssl-ca-path");
        updateConfigurationRecordName("tcpip_ssl_ca_file", "tcpip-ssl-ca-file");
        updateConfigurationRecordName("tcpip_ssl_verify_depth",
                                      "tcpip-ssl-verify-depth");
        updateConfigurationRecordName("tcpip_ssl_default_verify_paths",
                                      "tcpip-ssl-default-verify-paths");
        updateConfigurationRecordName("tcpip_ssl_cipher_list",
                                      "tcpip-ssl-cipher-list");
        updateConfigurationRecordName("tcpip_ssl_protocol_version",
                                      "tcpip-ssl-protocol-version");
        updateConfigurationRecordName("tcpip_ssl_short_trust",
                                      "tcpip-ssl-short-trust");
        updateConfigurationRecordName("udp_enable", "udp-enable");
        updateConfigurationRecordName("udp_address", "udp-address");
        updateConfigurationRecordName("udp_user", "udp-user");
        updateConfigurationRecordName("udp_password", "udp-password");
        updateConfigurationRecordName("udp_unsecure_enable", "udp-unsecure-enable");
        updateConfigurationRecordName("udp_filter", "udp-filter");
        updateConfigurationRecordName("udp_mask", "udp-mask");
        updateConfigurationRecordName("udp_guid", "udp-guid");
        updateConfigurationRecordName("udp_ack_enable", "udp-ack-enable");
        updateConfigurationRecordName("muticast_enable",
                                      "muticast-enable");
        updateConfigurationRecordName("dm_path_db", "dm-path-db");
        updateConfigurationRecordName("dm_path_xml", "dm-path-xml");
        updateConfigurationRecordName("dm_allow_xml_save", "dm-allow-xml-save");
        updateConfigurationRecordName("variable_path_db", "variable-path-db");
        updateConfigurationRecordName("variable_path_xml", "variable-path-xml");
        updateConfigurationRecordName("path_db_data",
                                      "path-db-event-data");
        updateConfigurationRecordName("web_enable", "web-enable");
        updateConfigurationRecordName("web_document_root", "web-document-root");
        updateConfigurationRecordName("web_listening_ports",
                                      "web-listening-ports");
        updateConfigurationRecordName("web_index_files", "web-index-files");
        updateConfigurationRecordName("web_authentication_domain",
                                      "web-authentication-domain");
        updateConfigurationRecordName("web_enable_auth_domain_check",
                                      "web-enable-auth-domain-check");
        updateConfigurationRecordName("web_ssl_certificate",
                                      "web-ssl-certificate");
        updateConfigurationRecordName("web_ssl_certificate_chain",
                                      "web-ssl-certificate-chain");
        updateConfigurationRecordName("web_ssl_verify_peer",
                                      "web-ssl-verify-peer");
        updateConfigurationRecordName("web_ssl_ca_path", "web-ssl-ca-path");
        updateConfigurationRecordName("web_ssl_ca_file", "web-ssl-ca-file");
        updateConfigurationRecordName("web_ssl_verify_depth",
                                      "web-ssl-verify-depth");
        updateConfigurationRecordName("web_ssl_default_verify_paths",
                                      "web-ssl-default-verify-paths");
        updateConfigurationRecordName("web_ssl_cipher_list",
                                      "web-ssl-cipher-list");
        updateConfigurationRecordName("web_ssl_protocol_version",
                                      "web-ssl-protocol-version");
        updateConfigurationRecordName("web_ssl_short_trust",
                                      "web-ssl-short-trust");
        updateConfigurationRecordName("web_cgi_interpreter",
                                      "web-cgi-interpreter");
        updateConfigurationRecordName("web_cgi_pattern", "web-cgi-pattern");
        updateConfigurationRecordName("web_cgi_environment",
                                      "web-cgi-environment");
        updateConfigurationRecordName("web_protect_uri", "web-protect-uri");
        updateConfigurationRecordName("web_trottle", "web-trottle");
        updateConfigurationRecordName("web_enable_directory_listing",
                                      "web-enable-directory-listing");
        updateConfigurationRecordName("web_enable_keep_alive",
                                      "web-enable-keep-alive");
        updateConfigurationRecordName("web_keep_alive_timeout_ms",
                                      "web-keep-alive-timeout-ms");
        updateConfigurationRecordName("web_access_control_list",
                                      "web-access-control-list");
        updateConfigurationRecordName("web_extra_mime_types",
                                      "web-extra-mime-types");
        updateConfigurationRecordName("web_num_threads", "web-num-threads");
        updateConfigurationRecordName("web_run_as_user", "web-run-as-user");
        updateConfigurationRecordName("web_url_rewrite_patterns",
                                      "web-url-rewrite-patterns");
        updateConfigurationRecordName("web_hide_file_patterns",
                                      "web-hide-file-patterns");
        updateConfigurationRecordName("web_request_timeout_ms",
                                      "web-request-timeout-ms");
        updateConfigurationRecordName("web_linger_timeout_ms",
                                      "web-linger-timeout-ms");
        updateConfigurationRecordName("web_decode_url", "web-decode-url");
        updateConfigurationRecordName("web_global_authfile",
                                      "web-global-authfile");
        updateConfigurationRecordName("web_per_directory_auth_file",
                                      "web-per-directory-auth-file");
        updateConfigurationRecordName("web_ssi_patterns", "web-ssi-patterns");
        updateConfigurationRecordName("web_access_control_allow_origin",
                                      "web-access-control-allow-origin");
        updateConfigurationRecordName("web_access_control_allow_methods",
                                      "web-access-control-allow-methods");
        updateConfigurationRecordName("web_access_control_allow_headers",
                                      "web-access-control-allow-headers");
        updateConfigurationRecordName("web_error_pages", "web-error-pages");
        updateConfigurationRecordName("web_tcp_nodelay", "web-tcp-nodelay");
        updateConfigurationRecordName("web_static_file_max_age",
                                      "web-static-file-max-age");
        updateConfigurationRecordName("web_strict_transport_security_max_age",
                                      "web-strict-transport-security-max-age");
        updateConfigurationRecordName("web_allow_sendfile_call",
                                      "web-allow-sendfile-call");
        updateConfigurationRecordName("web_additional_headers",
                                      "web-additional-headers");
        updateConfigurationRecordName("web_max_request_size",
                                      "web-max-request-size");
        updateConfigurationRecordName("web_allow_index_script_resource",
                                      "web-allow-index-script-resource");
        updateConfigurationRecordName("web_duktape_script_pattern",
                                      "web-duktape-script-pattern");
        updateConfigurationRecordName("web_lua_preload_file",
                                      "web-lua-preload-file");
        updateConfigurationRecordName("web_lua_script_pattern",
                                      "web-lua-script-pattern");
        updateConfigurationRecordName("web_lua_server_page_pattern",
                                      "web-lua-server-page-pattern");
        updateConfigurationRecordName("web_lua_websocket_pattern",
                                      "web-lua-websocket-pattern");
        updateConfigurationRecordName("web_lua_background_script",
                                      "web-lua-background-script");
        updateConfigurationRecordName("web_lua_background_script_params",
                                      "web-lua-background-script-params");
        updateConfigurationRecordName("websocket_enable", "websocket-enable");
        updateConfigurationRecordName("websocket_document_root",
                                      "websocket-document-root");
        updateConfigurationRecordName("websocket_timeout_ms",
                                      "websocket-timeout-ms");
        updateConfigurationRecordName("automation_enable", "automation-enable");
        updateConfigurationRecordName("automation_zone", "automation-zone");
        updateConfigurationRecordName("automation_subzone",
                                      "automation-subzone");
        updateConfigurationRecordName("automation_longitude",
                                      "automation-longitude");
        updateConfigurationRecordName("automation_latitude",
                                      "automation-latitude");
        updateConfigurationRecordName("automation_sunrise_enable",
                                      "automation-sunrise-enable");
        updateConfigurationRecordName("automation_sunset_enable",
                                      "automation-sunset-enable");
        updateConfigurationRecordName("automation_sunset_twilight_enable",
                                      "automation-sunset-twilight-enable");
        updateConfigurationRecordName("automation_sunrise_twilight_enable",
                                      "automation-sunrise-twilight-enable");
        updateConfigurationRecordName("automation_segment_ctrl_enable",
                                      "automation-segment-ctrl-enable");
        updateConfigurationRecordName("automation_segment_ctrl_interval",
                                      "automation-segment-ctrl-interval");
        updateConfigurationRecordName("automation_heartbeat_enable",
                                      "automation-heartbeat-enable");
        updateConfigurationRecordName("automation_heartbeat_interval",
                                      "automation-heartbeat-interval");
        updateConfigurationRecordName("automation_capabilities_enable",
                                      "automation-capabilities-enable");
        updateConfigurationRecordName("automation_capabilities_interval",
                                      "automation-capabilities-interval");

        // We are now at version 2
        updateConfigurationRecordItem(VSCPDB_CONFIG_NAME_DBVERSION ,"2");
    } // end 1 -> 2

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doCreateConfiguration
//
// Create configuration table.
//
//

bool
CControlObject::doCreateConfigurationTable(void)
{
    char *pErrMsg = 0;
    const char *psql;

    syslog(LOG_INFO, "Creating settings database.");

    // Check if database is open
    if (NULL == m_db_vscp_daemon) return false;

    pthread_mutex_lock(&m_db_vscp_configMutex);

    // Create settings db
    psql = VSCPDB_CONFIG_CREATE;
    if (SQLITE_OK !=
        sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg)) {
        syslog(LOG_ERR,
               "Creation of the VSCP settings database failed with message %s",
               pErrMsg);
        return false;
    }

    // Create name index
    psql = VSCPDB_CONFIG_CREATE_INDEX;
    if (SQLITE_OK !=
        sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg)) {
        pthread_mutex_unlock(&m_db_vscp_configMutex);
        syslog(LOG_ERR,
               "Creation of the VSCP settings index failed with message %s",
               pErrMsg);
        return false;
    }

    syslog(LOG_INFO, "Writing default configuration database content.");
    addDefaultConfigValues();

    pthread_mutex_unlock(&m_db_vscp_configMutex);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// readConfigurationDB
//
// Read the configuration database record
//
//

bool
CControlObject::readConfigurationDB(void)
{
    char *pErrMsg    = 0;
    const char *psql = VSCPDB_CONFIG_FIND_ALL;
    sqlite3_stmt *ppStmt;
    int dbVersion = 0;

    pthread_mutex_lock(&m_db_vscp_configMutex);

    // Check if database is open
    if (NULL == m_db_vscp_daemon) {
        syslog(LOG_ERR,
               "dbReadConfiguration: Failed to read VSCP settings database "
               "- Database is not open.");
        pthread_mutex_unlock(&m_db_vscp_configMutex);
        return false;
    }

    if (SQLITE_OK !=
        sqlite3_prepare(m_db_vscp_daemon, psql, -1, &ppStmt, NULL)) {
        syslog(LOG_ERR,
               "dbReadConfiguration: Failed to read VSCP settings database "
               "- prepare query.");
        pthread_mutex_unlock(&m_db_vscp_configMutex);
        return false;
    }

    while (SQLITE_ROW == sqlite3_step(ppStmt)) {

        const unsigned char *pName  = NULL;
        const unsigned char *pValue = NULL;

        if (NULL ==
            (pName = sqlite3_column_text(ppStmt, VSCPDB_ORDINAL_CONFIG_NAME))) {
            syslog(LOG_ERR,
                   "dbReadConfiguration: Failed to read 'name' "
                   "from settings record.");
            continue;
        }

        if (NULL == (pValue = sqlite3_column_text(
                       ppStmt, VSCPDB_ORDINAL_CONFIG_VALUE))) {
            syslog(LOG_ERR,
                   "dbReadConfiguration: Failed to read 'value' "
                   "from settings record.");
            continue;
        }

        // database version
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_DBVERSION)) {
            dbVersion = atoi((const char *)pValue);
            continue;
        }

        // client buffer size
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_CLIENTBUFFERSIZE)) {
            m_maxItemsInClientReceiveQueue = atol((const char *)pValue);
            continue;
        }

        // Server GUID
        if (0 ==
            vscp_strcasecmp((const char *)pName, VSCPDB_CONFIG_NAME_GUID)) {
            m_guid.getFromString((const char *)pValue);
            continue;
        }

        // Server name
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_SERVERNAME)) {
            m_strServerName = std::string((const char *)pValue);
            continue;
        }

        // TCP/IP interface address
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_TCPIP_ADDR)) {
            m_strTcpInterfaceAddress = std::string((const char *)pValue);
            // Remove possible "tcp://"" prefix
            vscp_startsWith(
              m_strTcpInterfaceAddress, "tcp://", &m_strTcpInterfaceAddress);
            if (vscp_startsWith(m_strTcpInterfaceAddress,
                                "ssl://",
                                &m_strTcpInterfaceAddress)) {
                m_strTcpInterfaceAddress += "s";
            }
            vscp_trim(m_strTcpInterfaceAddress);
            continue;
        }

        // TCP/IP encryption
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_TCPIP_ENCRYPTION)) {
            m_encryptionTcpip = atoi((const char *)pValue);
            continue;
        }

        // TCP/IP SSL certificat
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_TCPIP_SSL_CERTIFICATE)) {
            m_tcpip_ssl_certificate = std::string((const char *)pValue);
            continue;
        }

        // TCP/IP SSL certificat chain
        if (0 ==
            vscp_strcasecmp((const char *)pName,
                            VSCPDB_CONFIG_NAME_TCPIP_SSL_CERTIFICAT_CHAIN)) {
            m_tcpip_ssl_certificate_chain = std::string((const char *)pValue);
            continue;
        }

        // TCP/IP SSL verify peer
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_TCPIP_SSL_VERIFY_PEER)) {
            m_tcpip_ssl_verify_peer = atoi((const char *)pValue);
            continue;
        }

        // TCP/IP SSL CA path
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_TCPIP_SSL_CA_PATH)) {
            m_tcpip_ssl_ca_path = std::string((const char *)pValue);
            continue;
        }

        // TCP/IP SSL CA file
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_TCPIP_SSL_CA_FILE)) {
            m_tcpip_ssl_ca_file = std::string((const char *)pValue);
            continue;
        }

        // TCP/IP SSL verify depth
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_TCPIP_SSL_VERIFY_DEPTH)) {
            m_tcpip_ssl_verify_depth = atoi((const char *)pValue);
            continue;
        }

        // TCP/IP SSL verify paths
        if (0 == vscp_strcasecmp(
                   (const char *)pName,
                   VSCPDB_CONFIG_NAME_TCPIP_SSL_DEFAULT_VERIFY_PATHS)) {
            m_tcpip_ssl_default_verify_paths =
              atoi((const char *)pValue) ? true : false;
            continue;
        }

        // TCP/IP SSL Chipher list
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_TCPIP_SSL_CHIPHER_LIST)) {
            m_tcpip_ssl_cipher_list = std::string((const char *)pValue);
            continue;
        }

        // TCP/IP SSL protocol version
        if (0 ==
            vscp_strcasecmp((const char *)pName,
                            VSCPDB_CONFIG_NAME_TCPIP_SSL_PROTOCOL_VERSION)) {
            m_tcpip_ssl_protocol_version = atoi((const char *)pValue);
            continue;
        }

        // TCP/IP SSL short trust
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_TCPIP_SSL_SHORT_TRUST)) {
            m_tcpip_ssl_short_trust = atoi((const char *)pValue) ? true : false;
            continue;
        }

        // Announce multicast interface address
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_ANNOUNCE_ADDR)) {
            m_strMulticastAnnounceAddress = std::string((const char *)pValue);
            continue;
        }

        // TTL for the multicast i/f
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_ANNOUNCE_TTL)) {
            m_ttlMultiCastAnnounce = atoi((const char *)pValue);
            continue;
        }

        // Enable UDP interface
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_UDP_ENABLE)) {
            pthread_mutex_lock(&m_udpSrvObj.m_mutexUDPInfo);
            m_udpSrvObj.m_bEnable = atoi((const char *)pValue) ? true : false;
            pthread_mutex_unlock(&m_udpSrvObj.m_mutexUDPInfo);
            continue;
        }

        // UDP interface address/port
        if (0 ==
            vscp_strcasecmp((const char *)pName, VSCPDB_CONFIG_NAME_UDP_ADDR)) {
            pthread_mutex_lock(&m_udpSrvObj.m_mutexUDPInfo);
            m_udpSrvObj.m_interface = std::string((const char *)pValue);
            pthread_mutex_unlock(&m_udpSrvObj.m_mutexUDPInfo);
            continue;
        }

        // UDP User
        if (0 ==
            vscp_strcasecmp((const char *)pName, VSCPDB_CONFIG_NAME_UDP_USER)) {
            pthread_mutex_lock(&m_udpSrvObj.m_mutexUDPInfo);
            m_udpSrvObj.m_user = std::string((const char *)pValue);
            pthread_mutex_unlock(&m_udpSrvObj.m_mutexUDPInfo);
            continue;
        }

        // UDP User Password
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_UDP_PASSWORD)) {
            pthread_mutex_lock(&m_udpSrvObj.m_mutexUDPInfo);
            m_udpSrvObj.m_password = std::string((const char *)pValue);
            pthread_mutex_unlock(&m_udpSrvObj.m_mutexUDPInfo);
            continue;
        }

        // UDP un-secure enable
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_UDP_UNSECURE_ENABLE)) {
            pthread_mutex_lock(&m_udpSrvObj.m_mutexUDPInfo);
            m_udpSrvObj.m_bAllowUnsecure =
              atoi((const char *)pValue) ? true : false;
            pthread_mutex_unlock(&m_udpSrvObj.m_mutexUDPInfo);
            continue;
        }

        // UDP Filter
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_UDP_FILTER)) {
            pthread_mutex_lock(&m_udpSrvObj.m_mutexUDPInfo);
            vscp_readFilterFromString(&m_udpSrvObj.m_filter,
                                      std::string((const char *)pValue));
            pthread_mutex_unlock(&m_udpSrvObj.m_mutexUDPInfo);
            continue;
        }

        // UDP Mask
        if (0 ==
            vscp_strcasecmp((const char *)pName, VSCPDB_CONFIG_NAME_UDP_MASK)) {
            pthread_mutex_lock(&m_udpSrvObj.m_mutexUDPInfo);
            vscp_readMaskFromString(&m_udpSrvObj.m_filter,
                                    std::string((const char *)pValue));
            pthread_mutex_unlock(&m_udpSrvObj.m_mutexUDPInfo);
            continue;
        }

        // UDP GUID
        if (0 ==
            vscp_strcasecmp((const char *)pName, VSCPDB_CONFIG_NAME_UDP_GUID)) {
            pthread_mutex_lock(&m_udpSrvObj.m_mutexUDPInfo);
            m_udpSrvObj.m_guid.getFromString((const char *)pValue);
            pthread_mutex_unlock(&m_udpSrvObj.m_mutexUDPInfo);
            continue;
        }

        // UDP Enable ACK
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_UDP_ACK_ENABLE)) {
            pthread_mutex_lock(&m_udpSrvObj.m_mutexUDPInfo);
            m_udpSrvObj.m_bAck = atoi((const char *)pValue) ? true : false;
            pthread_mutex_unlock(&m_udpSrvObj.m_mutexUDPInfo);
            continue;
        }

        // Enable Multicast interface
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_MULTICAST_ENABLE)) {
            m_bEnableMulticast = atoi((const char *)pValue) ? true : false;
            continue;
        }

        // Path to DM database file
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_DM_PATH_DB)) {
            m_dm.m_path_db_vscp_dm = std::string((const char *)pValue);
            continue;
        }

        // Path to DM XML file
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_DM_PATH_XML)) {
            m_dm.m_staticXMLPath = std::string((const char *)pValue);
            continue;
        }

        // Path to variable database
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_VARIABLES_PATH_DB)) {
            m_variables.m_dbFilename = std::string((const char *)pValue);
            continue;
        }

        // Path to variable XML
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_VARIABLES_PATH_XML)) {
            m_variables.m_xmlPath = std::string((const char *)pValue);
            continue;
        }

        // VSCP data database path
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_PATH_DB_EVENTS)) {
            m_path_db_vscp_data = std::string((const char *)pValue);
            continue;
        }

        // * * * WEB server * * *

        // Web server enable
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_ENABLE)) {
            if (atoi((const char *)pValue)) {
                m_web_bEnable = true;
            } else {
                m_web_bEnable = false;
            }
            continue;
        }

        // Web server document root
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_DOCUMENT_ROOT)) {
            m_web_document_root = std::string((const char *)pValue);
            continue;
        }

        // listening ports for web server
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_LISTENING_PORTS)) {
            m_web_listening_ports = std::string((const char *)pValue);
            continue;
        }

        // Index files
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_INDEX_FILES)) {
            m_web_index_files = std::string((const char *)pValue);
            continue;
        }

        // Authdomain
        if (0 ==
            vscp_strcasecmp((const char *)pName,
                            VSCPDB_CONFIG_NAME_WEB_AUTHENTICATION_DOMAIN)) {
            m_web_authentication_domain = std::string((const char *)pValue);
            continue;
        }

        // Enable authdomain check
        if (0 ==
            vscp_strcasecmp((const char *)pName,
                            VSCPDB_CONFIG_NAME_WEB_ENABLE_AUTH_DOMAIN_CHECK)) {
            if (atoi((const char *)pValue)) {
                m_enable_auth_domain_check = true;
            } else {
                m_enable_auth_domain_check = false;
            }
            continue;
        }

        // Path to cert file
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_SSL_CERTIFICATE)) {
            m_web_ssl_certificate = std::string((const char *)pValue);
            continue;
        }

        // SSL certificate chain
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_SSL_CERTIFICAT_CHAIN)) {
            m_web_ssl_certificate_chain = std::string((const char *)pValue);
            continue;
        }

        // SSL verify peer
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_SSL_VERIFY_PEER)) {
            if (atoi((const char *)pValue)) {
                m_web_ssl_verify_peer = true;
            } else {
                m_web_ssl_verify_peer = false;
            }
            continue;
        }

        // SSL CA path
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_SSL_CA_FILE)) {
            m_web_ssl_ca_path = std::string((const char *)pValue);
            continue;
        }

        // SSL CA file
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_SSL_CA_FILE)) {
            m_web_ssl_ca_file = std::string((const char *)pValue);
            continue;
        }

        // SSL verify depth
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_SSL_VERIFY_DEPTH)) {
            m_web_ssl_verify_depth = atoi((const char *)pValue);
            continue;
        }

        // SSL default verify path
        if (0 ==
            vscp_strcasecmp((const char *)pName,
                            VSCPDB_CONFIG_NAME_WEB_SSL_DEFAULT_VERIFY_PATHS)) {
            if (atoi((const char *)pValue)) {
                m_web_ssl_default_verify_paths = true;
            } else {
                m_web_ssl_default_verify_paths = false;
            }
            continue;
        }

        // SSL chipher list
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_SSL_CHIPHER_LIST)) {
            m_web_ssl_cipher_list = std::string((const char *)pValue);
            continue;
        }

        // SSL protocol version
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_SSL_PROTOCOL_VERSION)) {
            m_web_ssl_protocol_version = atoi((const char *)pValue);
            continue;
        }

        // SSL short trust
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_SSL_SHORT_TRUST)) {
            if (atoi((const char *)pValue)) {
                m_web_ssl_short_trust = true;
            } else {
                m_web_ssl_short_trust = false;
            }
            continue;
        }

        // CGI interpreter
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_CGI_INTERPRETER)) {
            m_web_cgi_interpreter = std::string((const char *)pValue);
            continue;
        }

        // CGI pattern
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_CGI_PATTERN)) {
            m_web_cgi_patterns = std::string((const char *)pValue);
            continue;
        }

        // CGI environment
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_CGI_ENVIRONMENT)) {
            m_web_cgi_environment = std::string((const char *)pValue);
            continue;
        }

        // Protect URI
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_PROTECT_URI)) {
            m_web_protect_uri = std::string((const char *)pValue);
            continue;
        }

        // Web trottle
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_TROTTLE)) {
            m_web_trottle = std::string((const char *)pValue);
            continue;
        }

        // Enable directory listings
        if (0 ==
            vscp_strcasecmp((const char *)pName,
                            VSCPDB_CONFIG_NAME_WEB_ENABLE_DIRECTORY_LISTING)) {
            if (atoi((const char *)pValue)) {
                m_web_enable_directory_listing = true;
            } else {
                m_web_enable_directory_listing = false;
            }
            continue;
        }

        // Enable keep alive
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_ENABLE_KEEP_ALIVE)) {
            if (atoi((const char *)pValue)) {
                m_web_enable_keep_alive = true;
            } else {
                m_web_enable_keep_alive = false;
            }
            continue;
        }

        // Keep alive timout ms
        if (0 ==
            vscp_strcasecmp((const char *)pName,
                            VSCPDB_CONFIG_NAME_WEB_KEEP_ALIVE_TIMEOUT_MS)) {
            m_web_keep_alive_timeout_ms = atol((const char *)pValue);
            continue;
        }

        // IP ACL
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_LIST)) {
            m_web_access_control_list = std::string((const char *)pValue);
            continue;
        }

        // Extra mime types
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_EXTRA_MIME_TYPES)) {
            m_web_extra_mime_types = std::string((const char *)pValue);
            continue;
        }

        // Number of threads
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_NUM_THREADS)) {
            m_web_num_threads = atoi((const char *)pValue);
            continue;
        }

        // Hide file patterns
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_HIDE_FILE_PATTERNS)) {
            m_web_hide_file_patterns = std::string((const char *)pValue);
            continue;
        }

        // Run as user
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_RUN_AS_USER)) {
            m_web_run_as_user = std::string((const char *)pValue);
            continue;
        }

        // URL rewrites
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_URL_REWRITE_PATTERNS)) {
            m_web_url_rewrite_patterns = std::string((const char *)pValue);
            continue;
        }

        // Hide file patterns
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_HIDE_FILE_PATTERNS)) {
            m_web_hide_file_patterns = std::string((const char *)pValue);
            continue;
        }

        // web request timout
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_REQUEST_TIMEOUT_MS)) {
            m_web_request_timeout_ms = atol((const char *)pValue);
            continue;
        }

        // web linger timout
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_LINGER_TIMEOUT_MS)) {
            m_web_linger_timeout_ms = atol((const char *)pValue);
            continue;
        }

        // Decode URL
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_DECODE_URL)) {
            if (atoi((const char *)pValue)) {
                m_web_decode_url = true;
            } else {
                m_web_decode_url = false;
            }
            continue;
        }

        // Global auth. file
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_GLOBAL_AUTHFILE)) {
            m_web_global_auth_file = std::string((const char *)pValue);
            continue;
        }

        // Per directory auth. file
        if (0 ==
            vscp_strcasecmp((const char *)pName,
                            VSCPDB_CONFIG_NAME_WEB_PER_DIRECTORY_AUTH_FILE)) {
            m_web_per_directory_auth_file = std::string((const char *)pValue);
            continue;
        }

        // SSI patterns
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_SSI_PATTERNS)) {
            m_web_ssi_patterns = std::string((const char *)pValue);
            continue;
        }

        // Access control allow origin
        if (0 == vscp_strcasecmp(
                   (const char *)pName,
                   VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_ALLOW_ORIGIN)) {
            m_web_access_control_allow_origin =
              std::string((const char *)pValue);
            continue;
        }

        // Access control allow methods
        if (0 == vscp_strcasecmp(
                   (const char *)pName,
                   VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_ALLOW_METHODS)) {
            m_web_access_control_allow_methods =
              std::string((const char *)pValue);
            continue;
        }

        // Access control alow heraders
        if (0 == vscp_strcasecmp(
                   (const char *)pName,
                   VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_ALLOW_HEADERS)) {
            m_web_access_control_allow_headers =
              std::string((const char *)pValue);
            continue;
        }

        // Error pages
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_ERROR_PAGES)) {
            m_web_error_pages = std::string((const char *)pValue);
            continue;
        }

        // TCP no delay
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_TCP_NO_DELAY)) {
            m_web_tcp_nodelay = atol((const char *)pValue);
            continue;
        }

        // File max age
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_STATIC_FILE_MAX_AGE)) {
            m_web_static_file_max_age = atol((const char *)pValue);
            continue;
        }

        // Transport security max age
        if (0 == vscp_strcasecmp(
                   (const char *)pName,
                   VSCPDB_CONFIG_NAME_WEB_STRICT_TRANSPORT_SECURITY_MAX_AGE)) {
            m_web_strict_transport_security_max_age =
              atol((const char *)pValue);
            continue;
        }

        // Enable sendfile call
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_ALLOW_SENDFILE_CALL)) {
            if (atoi((const char *)pValue)) {
                m_web_allow_sendfile_call = true;
            } else {
                m_web_allow_sendfile_call = false;
            }
            continue;
        }

        // Additional headers
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_ADDITIONAL_HEADERS)) {
            m_web_additional_header = std::string((const char *)pValue);
            continue;
        }

        // Max request size
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_MAX_REQUEST_SIZE)) {
            m_web_max_request_size = atol((const char *)pValue);
            continue;
        }

        // Allow index script resource
        if (0 == vscp_strcasecmp(
                   (const char *)pName,
                   VSCPDB_CONFIG_NAME_WEB_ALLOW_INDEX_SCRIPT_RESOURCE)) {
            if (atoi((const char *)pValue)) {
                m_web_allow_index_script_resource = true;
            } else {
                m_web_allow_index_script_resource = false;
            }
            continue;
        }

        // Duktape script patterns
        if (0 ==
            vscp_strcasecmp((const char *)pName,
                            VSCPDB_CONFIG_NAME_WEB_DUKTAPE_SCRIPT_PATTERN)) {
            m_web_duktape_script_patterns = std::string((const char *)pValue);
            continue;
        }

        // Lua preload file
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_LUA_PRELOAD_FILE)) {
            m_web_lua_preload_file = std::string((const char *)pValue);
            continue;
        }

        // Lua script patterns
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEB_LUA_SCRIPT_PATTERN)) {
            m_web_lua_script_patterns = std::string((const char *)pValue);
            continue;
        }

        // Lua server page patterns
        if (0 ==
            vscp_strcasecmp((const char *)pName,
                            VSCPDB_CONFIG_NAME_WEB_LUA_SERVER_PAGE_PATTERN)) {
            m_web_lua_server_page_patterns = std::string((const char *)pValue);
            continue;
        }

        // Lua websocket patterns
        if (0 ==
            vscp_strcasecmp((const char *)pName,
                            VSCPDB_CONFIG_NAME_WEB_LUA_WEBSOCKET_PATTERN)) {
            m_web_lua_websocket_patterns = std::string((const char *)pValue);
            continue;
        }

        // Lua background script
        if (0 ==
            vscp_strcasecmp((const char *)pName,
                            VSCPDB_CONFIG_NAME_WEB_LUA_BACKGROUND_SCRIPT)) {
            m_web_lua_background_script = std::string((const char *)pValue);
            continue;
        }

        // Lua background script params
        if (0 == vscp_strcasecmp(
                   (const char *)pName,
                   VSCPDB_CONFIG_NAME_WEB_LUA_BACKGROUND_SCRIPT_PARAMS)) {
            m_web_lua_background_script_params =
              std::string((const char *)pValue);
            continue;
        }

        // * * * Websockets * * *

        // Web server enable
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEBSOCKET_ENABLE)) {
            if (atoi((const char *)pValue)) {
                m_bWebsocketsEnable = true;
            } else {
                m_bWebsocketsEnable = false;
            }
            continue;
        }

        // Document root for websockets
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEBSOCKET_DOCUMENT_ROOT)) {
            m_websocket_document_root = std::string((const char *)pValue);
            continue;
        }

        // Websocket timeout
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_WEBSOCKET_TIMEOUT_MS)) {
            m_websocket_timeout_ms = atol((const char *)pValue);
            continue;
        }

        // * * * Automation * * *

        // Enable automation
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_AUTOMATION_ENABLE)) {

            if (atoi((const char *)pValue)) {
                m_automation.enableAutomation();
            } else {
                m_automation.enableAutomation();
            }
            continue;
        }

        // Automation zone
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_AUTOMATION_ZONE)) {
            m_automation.setZone(atoi((const char *)pValue));
            continue;
        }

        // Automation sub zone
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_AUTOMATION_SUBZONE)) {
            m_automation.setSubzone(atoi((const char *)pValue));
            continue;
        }

        // Automation longitude
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_AUTOMATION_LONGITUDE)) {
            m_automation.setLongitude(atof((const char *)pValue));
            continue;
        }

        // Automation latitude
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_AUTOMATION_LATITUDE)) {
            m_automation.setLatitude(atof((const char *)pValue));
            continue;
        }

        // Automation enable sun rise event
        if (0 ==
            vscp_strcasecmp((const char *)pName,
                            VSCPDB_CONFIG_NAME_AUTOMATION_SUNRISE_ENABLE)) {
            if (atoi((const char *)pValue)) {
                m_automation.enableSunRiseEvent();
            } else {
                m_automation.disableSunRiseEvent();
            }
            continue;
        }

        // Automation enable sun set event
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_AUTOMATION_SUNSET_ENABLE)) {
            if (atoi((const char *)pValue)) {
                m_automation.enableSunSetEvent();
            } else {
                m_automation.disableSunSetEvent();
            }
            continue;
        }

        // Automation enable sunset twilight event
        if (0 == vscp_strcasecmp(
                   (const char *)pName,
                   VSCPDB_CONFIG_NAME_AUTOMATION_SUNSET_TWILIGHT_ENABLE)) {
            if (atoi((const char *)pValue)) {
                m_automation.enableSunSetTwilightEvent();
            } else {
                m_automation.disableSunSetTwilightEvent();
            }
            continue;
        }

        // Automation enable sunrise twilight event
        if (0 == vscp_strcasecmp(
                   (const char *)pName,
                   VSCPDB_CONFIG_NAME_AUTOMATION_SUNRISE_TWILIGHT_ENABLE)) {
            if (atoi((const char *)pValue)) {
                m_automation.enableSunRiseTwilightEvent();
            } else {
                m_automation.disableSunRiseTwilightEvent();
            }
            continue;
        }

        // Automation segment controller event enable
        if (0 == vscp_strcasecmp(
                   (const char *)pName,
                   VSCPDB_CONFIG_NAME_AUTOMATION_SEGMENT_CTRL_ENABLE)) {
            if (atoi((const char *)pValue)) {
                m_automation.enableSegmentControllerHeartbeat();
            } else {
                m_automation.disableSegmentControllerHeartbeat();
            }
            continue;
        }

        // Automation, segment controller heartbeat interval
        if (0 == vscp_strcasecmp(
                   (const char *)pName,
                   VSCPDB_CONFIG_NAME_AUTOMATION_SEGMENT_CTRL_INTERVAL)) {
            m_automation.setSegmentControllerHeartbeatInterval(
              atol((const char *)pValue));
            continue;
        }

        // Automation heartbeat event enable
        if (0 ==
            vscp_strcasecmp((const char *)pName,
                            VSCPDB_CONFIG_NAME_AUTOMATION_HEARTBEAT_ENABLE)) {
            if (atoi((const char *)pValue)) {
                m_automation.enableHeartbeatEvent();
            } else {
                m_automation.disableHeartbeatEvent();
            }
            continue;
        }

        // Automation heartbeat interval
        if (0 ==
            vscp_strcasecmp((const char *)pName,
                            VSCPDB_CONFIG_NAME_AUTOMATION_HEARTBEAT_INTERVAL)) {
            m_automation.setHeartbeatEventInterval(atol((const char *)pValue));
            continue;
        }

        // Automation capabilities event enable
        if (0 == vscp_strcasecmp(
                   (const char *)pName,
                   VSCPDB_CONFIG_NAME_AUTOMATION_CAPABILITIES_ENABLE)) {
            if (atoi((const char *)pValue)) {
                m_automation.enableCapabilitiesEvent();
            } else {
                m_automation.disableCapabilitiesEvent();
            }
            continue;
        }

        // Automation capabilities interval
        if (0 == vscp_strcasecmp(
                   (const char *)pName,
                   VSCPDB_CONFIG_NAME_AUTOMATION_CAPABILITIES_INTERVAL)) {
            m_automation.setCapabilitiesEventInterval(
              atol((const char *)pValue));
            continue;
        }
    }

    sqlite3_finalize(ppStmt);

    pthread_mutex_unlock(&m_db_vscp_configMutex);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// readUdpNodes
//
// Read in defined UDP nodes
//
//

bool
CControlObject::readUdpNodes(void)
{
    char *pErrMsg    = 0;
    const char *psql = "SELECT * FROM udpnode";
    sqlite3_stmt *ppStmt;

    // If UDP is disabled we are done
    if (!m_udpSrvObj.m_bEnable) return true;

    // Check if database is open
    if (NULL == m_db_vscp_daemon) {
        syslog(LOG_ERR, "readUdpNodes: Database is not open.");
        return false;
    }

    if (SQLITE_OK !=
        sqlite3_prepare(m_db_vscp_daemon, psql, -1, &ppStmt, NULL)) {
        syslog(LOG_ERR, "readUdpNodes: prepare query failed.");
        return false;
    }

    while (SQLITE_ROW == sqlite3_step(ppStmt)) {

        const unsigned char *p;

        // If not enabled move on
        if (!sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_UDPNODE_ENABLE))
            continue;

        pthread_mutex_lock(&m_udpSrvObj.m_mutexUDPInfo);

        udpRemoteClientInfo *pudpClient = new udpRemoteClientInfo;
        if (NULL == pudpClient) {
            syslog(LOG_ERR,
                   "readUdpNodes: Failed to allocate storage for UDP node.");
            pthread_mutex_unlock(&m_udpSrvObj.m_mutexUDPInfo);
            continue;
        }

        // Broadcast
        pudpClient->m_bSetBroadcast = false;
        if (sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_UDPNODE_SET_BROADCAST)) {
            pudpClient->m_bSetBroadcast = true;
        } // Interface
        p = sqlite3_column_text(ppStmt, VSCPDB_ORDINAL_UDPNODE_INTERFACE);
        if (NULL != p) {
            pudpClient->m_remoteAddress = std::string((const char *)p);
        }

        //  Filter
        p = sqlite3_column_text(ppStmt, VSCPDB_ORDINAL_UDPNODE_FILTER);
        if (NULL != p) {
            std::string wxstr = std::string((const char *)p);
            if (!vscp_readFilterFromString(&pudpClient->m_filter, wxstr)) {
                syslog(LOG_ERR,
                       "readUdpNodes: Failed to set filter for UDP node.");
            }
        }

        // Mask
        p = sqlite3_column_text(ppStmt, VSCPDB_ORDINAL_UDPNODE_MASK);
        if (NULL != p) {
            std::string wxstr = std::string((const char *)p);
            if (!vscp_readMaskFromString(&pudpClient->m_filter, wxstr)) {
                syslog(LOG_ERR,
                       "readUdpNodes: Failed to set mask for UDP node.");
            }
        }

        // Encryption
        p = sqlite3_column_text(ppStmt, VSCPDB_ORDINAL_UDPNODE_ENCRYPTION);
        if (NULL != p) {
            std::string wxstr         = std::string((const char *)p);
            pudpClient->m_nEncryption = vscp_getEncryptionCodeFromToken(wxstr);
        }

        // Add to list
        pudpClient->m_index = 0;
        m_udpSrvObj.m_remotes.push_back(pudpClient);

        pthread_mutex_unlock(&m_udpSrvObj.m_mutexUDPInfo);
    }

    sqlite3_finalize(ppStmt);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// readMulticastChannels
//
// Read in defined multicast channels
//
//

bool
CControlObject::readMulticastChannels(void)
{
    char *pErrMsg    = 0;
    const char *psql = "SELECT * FROM multicast";
    sqlite3_stmt *ppStmt;

    // If multicast is disabled we are done
    if (!m_bEnableMulticast) return true;

    // Check if database is open
    if (NULL == m_db_vscp_daemon) {
        syslog(LOG_ERR, "readMulticastChannels: Database is not open.");
        return false;
    }

    if (SQLITE_OK !=
        sqlite3_prepare(m_db_vscp_daemon, psql, -1, &ppStmt, NULL)) {
        syslog(LOG_ERR, "readMulticastChannels: prepare query failed.");
        return false;
    }

    while (SQLITE_ROW == sqlite3_step(ppStmt)) {

        const unsigned char *p;

        // If not enabled move on
        if (!sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_MULTICAST_ENABLE))
            continue;

        multicastChannelItem *pChannel = new multicastChannelItem;
        if (NULL == pChannel) {
            syslog(LOG_ERR,
                   "readMulticastChannels: Failed to allocate storage for "
                   "multicast node.");
            continue;
        }

        // Default is to let everything come through
        vscp_clearVSCPFilter(&pChannel->m_txFilter);
        vscp_clearVSCPFilter(&pChannel->m_rxFilter);

        // public interface
        p = sqlite3_column_text(ppStmt, VSCPDB_ORDINAL_MULTICAST_PUBLIC);
        if (NULL != p) {
            pChannel->m_public = std::string((const char *)p);
        }

        // Port
        p = sqlite3_column_text(ppStmt, VSCPDB_ORDINAL_MULTICAST_PORT);

        // group
        p = sqlite3_column_text(ppStmt, VSCPDB_ORDINAL_MULTICAST_GROUP);
        if (NULL != p) {
            pChannel->m_gropupAddress = std::string((const char *)p);
        } // ttl
        pChannel->m_ttl =
          sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_MULTICAST_TTL);

        // bAck
        pChannel->m_bSendAck =
          sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_MULTICAST_SENDACK) ? true
                                                                       : false;

        // Allow unsecure
        pChannel->m_bAllowUnsecure =
          sqlite3_column_int(ppStmt, VSCPDB_ORDINAL_MULTICAST_ALLOW_UNSECURE)
            ? true
            : false;

        // GUID
        p = sqlite3_column_text(ppStmt, VSCPDB_ORDINAL_MULTICAST_GUID);
        if (NULL != p) {
            pChannel->m_guid.getFromString((const char *)p);
        }

        //  TX Filter
        p = sqlite3_column_text(ppStmt, VSCPDB_ORDINAL_MULTICAST_TXFILTER);
        if (NULL != p) {
            std::string wxstr = std::string((const char *)p);
            if (!vscp_readFilterFromString(&pChannel->m_txFilter, wxstr)) {
                syslog(LOG_ERR,
                       "readMulticastChannels: Failed to set TX "
                       "filter for multicast channel.");
            }
        }

        // TX Mask
        p = sqlite3_column_text(ppStmt, VSCPDB_ORDINAL_MULTICAST_TXMASK);
        if (NULL != p) {
            std::string wxstr = std::string((const char *)p);
            if (!vscp_readMaskFromString(&pChannel->m_txFilter, wxstr)) {
                syslog(LOG_ERR,
                       "readMulticastChannels: Failed to set TX "
                       "mask for multicast channel.");
            }
        }

        //  RX Filter
        p = sqlite3_column_text(ppStmt, VSCPDB_ORDINAL_MULTICAST_RXFILTER);
        if (NULL != p) {
            std::string wxstr = std::string((const char *)p);
            if (!vscp_readFilterFromString(&pChannel->m_rxFilter, wxstr)) {
                syslog(LOG_ERR,
                       "readMulticastChannels: Failed to set RX "
                       "filter for multicast channel.");
            }
        }

        // RX Mask
        p = sqlite3_column_text(ppStmt, VSCPDB_ORDINAL_MULTICAST_RXMASK);
        if (NULL != p) {
            std::string wxstr = std::string((const char *)p);
            if (!vscp_readMaskFromString(&pChannel->m_rxFilter, wxstr)) {
                syslog(LOG_ERR,
                       "readMulticastChannels: Failed to set RX "
                       "mask for multicast channel.");
            }
        }

        // Encryption
        p = sqlite3_column_text(ppStmt, VSCPDB_ORDINAL_UDPNODE_ENCRYPTION);
        if (NULL != p) {
            std::string wxstr       = std::string((const char *)p);
            pChannel->m_nEncryption = vscp_getEncryptionCodeFromToken(wxstr);
        }

        // Add to list
        pChannel->m_index = 0;
        m_multicastObj.m_channels.push_back(pChannel);
    }

    sqlite3_finalize(ppStmt);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doCreateUdpNodeTable
//
// Create the UDP node database
//

bool
CControlObject::doCreateUdpNodeTable(void)
{
    char *pErrMsg    = 0;
    const char *psql = VSCPDB_UDPNODE_CREATE;

    syslog(LOG_INFO, "Creating udpnode table.");

    // Check if database is open
    if (NULL == m_db_vscp_daemon) {
        syslog(LOG_ERR,
               "Failed to create VSCP udpnode table - database closed.");
        return false;
    }

    pthread_mutex_lock(&m_db_vscp_configMutex);

    if (SQLITE_OK !=
        sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg)) {
        syslog(LOG_ERR,
               "Failed to create VSCP udpnode table with error %s.",
               pErrMsg);
        pthread_mutex_unlock(&m_db_vscp_configMutex);
        return false;
    }

    pthread_mutex_unlock(&m_db_vscp_configMutex);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doCreateMulticastTable
//
// Create the multicast database
//
//

bool
CControlObject::doCreateMulticastTable(void)
{
    char *pErrMsg    = 0;
    const char *psql = VSCPDB_MULTICAST_CREATE;

    syslog(LOG_INFO, "Creating multicast table.");

    // Check if database is open
    if (NULL == m_db_vscp_daemon) {
        syslog(LOG_ERR,
               "Failed to create VSCP multicast table - database closed.");
        return false;
    }

    pthread_mutex_lock(&m_db_vscp_configMutex);

    if (SQLITE_OK !=
        sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg)) {
        syslog(LOG_ERR,
               "Failed to create VSCP multicast table with error %s.",
               pErrMsg);
        pthread_mutex_unlock(&m_db_vscp_configMutex);
        return false;
    }

    pthread_mutex_unlock(&m_db_vscp_configMutex);

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doCreateUserTable
//
// Create the user table
//
//

bool
CControlObject::doCreateUserTable(void)
{
    char *pErrMsg    = 0;
    const char *psql = VSCPDB_USER_CREATE;

    syslog(LOG_INFO, "Creating user table.");

    // Check if database is open
    if (NULL == m_db_vscp_daemon) {
        syslog(LOG_ERR, "Failed to create VSCP user table - closed.");
        return false;
    }

    if (SQLITE_OK !=
        sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg)) {
        syslog(
          LOG_ERR, "Failed to create VSCP user table with error %s.", pErrMsg);
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doCreateDriverTable
//
// Create the driver table
//
//

bool
CControlObject::doCreateDriverTable(void)
{
    char *pErrMsg    = 0;
    const char *psql = VSCPDB_DRIVER_CREATE;

    syslog(LOG_INFO, "Creating driver table.");

    // Check if database is open
    if (NULL == m_db_vscp_daemon) {
        syslog(LOG_ERR, "Failed to create VSCP driver table - closed.");
        return false;
    }

    if (SQLITE_OK !=
        sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg)) {
        syslog(LOG_ERR,
               "Failed to create VSCP driver table with error %s.",
               pErrMsg);
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doCreateGuidTable
//
// Create the GUID table
//
//

bool
CControlObject::doCreateGuidTable(void)
{
    char *pErrMsg    = 0;
    const char *psql = VSCPDB_GUID_CREATE;

    syslog(LOG_INFO, "Creating GUID discovery table.");

    // Check if database is open
    if (NULL == m_db_vscp_daemon) {
        syslog(LOG_ERR, "Failed to create VSCP GUID table - closed.");
        return false;
    }

    if (SQLITE_OK !=
        sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg)) {
        syslog(
          LOG_ERR, "Failed to create VSCP GUID table with error %s.", pErrMsg);
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doCreateLocationTable
//
// Create the Location table
//
//

bool
CControlObject::doCreateLocationTable(void)
{
    char *pErrMsg    = 0;
    const char *psql = VSCPDB_LOCATION_CREATE;

    syslog(LOG_INFO, "Creating location table.");

    // Check if database is open
    if (NULL == m_db_vscp_daemon) {
        syslog(LOG_ERR, "Failed to create VSCP location table - closed.");
        return false;
    }

    if (SQLITE_OK !=
        sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg)) {
        syslog(LOG_ERR,
               "Failed to create VSCP location table with error %s.",
               pErrMsg);
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doCreateMdfCacheTable
//
// Create the mdf cache table
//
//

bool
CControlObject::doCreateMdfCacheTable(void)
{
    char *pErrMsg    = 0;
    const char *psql = VSCPDB_MDF_CREATE;

    syslog(LOG_INFO, "Creating MDF table.");

    // Check if database is open
    if (NULL == m_db_vscp_daemon) {
        syslog(LOG_ERR, "Failed to create VSCP mdf table - closed.");
        return false;
    }

    if (SQLITE_OK !=
        sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg)) {
        syslog(
          LOG_ERR, "Failed to create VSCP mdf table with error %s.", pErrMsg);
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doCreateSimpleUiTable
//
// Create the simple UI table
//
//

bool
CControlObject::doCreateSimpleUiTable(void)
{
    char *pErrMsg    = 0;
    const char *psql = VSCPDB_SIMPLE_UI_CREATE;

    syslog(LOG_INFO, "Creating simple ui table.");

    // Check if database is open
    if (NULL == m_db_vscp_daemon) {
        syslog(LOG_ERR, "Failed to create VSCP simple ui table - closed.");
        return false;
    }

    if (SQLITE_OK !=
        sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg)) {
        syslog(LOG_ERR,
               "Failed to create VSCP simple ui table with error %s.",
               pErrMsg);
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doCreateSimpleUiItemTable
//
// Create the simple UI item table
//
//

bool
CControlObject::doCreateSimpleUiItemTable(void)
{
    char *pErrMsg    = 0;
    const char *psql = VSCPDB_SIMPLE_UI_ITEM_CREATE;

    syslog(LOG_INFO, "Creating simple ui item table..");

    // Check if database is open
    if (NULL == m_db_vscp_daemon) {
        syslog(LOG_ERR, "Failed to create VSCP simple UI item table - closed.");
        return false;
    }

    if (SQLITE_OK !=
        sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg)) {
        syslog(LOG_ERR,
               "Failed to create VSCP simple UI item table with error %s.",
               pErrMsg);
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doCreateZoneTable
//
// Create the zone table
//

bool
CControlObject::doCreateZoneTable(void)
{
    char *pErrMsg    = 0;
    const char *psql = VSCPDB_ZONE_CREATE;

    syslog(LOG_INFO, "Creating zone table..");

    // Check if database is open
    if (NULL == m_db_vscp_daemon) {
        syslog(LOG_ERR, "Failed to create VSCP zone table - closed.");
        return false;
    }

    if (SQLITE_OK !=
        sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg)) {
        syslog(
          LOG_ERR, "Failed to create VSCP zone table with error %s.", pErrMsg);
        return false;
    }

    // Fill with default info
    std::string sql = "BEGIN;";
    for (int i = 0; i < 256; i++) {
        sql += vscp_str_format("INSERT INTO 'zone' (idx_zone, name) "
                               "VALUES( %d, 'zone%d' );",
                               i,
                               i);
    }

    sql += vscp_str_format(VSCPDB_ZONE_UPDATE,
                           "All zones",
                           "Zone = 255 represents all zones.",
                           255L);
    sql += "COMMIT;";
    if (SQLITE_OK !=
        sqlite3_exec(
          m_db_vscp_daemon, (const char *)sql.c_str(), NULL, NULL, &pErrMsg)) {
        syslog(LOG_ERR,
               "Failed to insert last VSCP default zone table entry %d. "
               "Error %s",
               255,
               pErrMsg);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doCreateSubZoneTable
//
// Create the subzone table
//
//

bool
CControlObject::doCreateSubZoneTable(void)
{
    char *pErrMsg    = 0;
    const char *psql = VSCPDB_SUBZONE_CREATE;

    syslog(LOG_INFO, "Creating sub-zone table.");

    // Check if database is open
    if (NULL == m_db_vscp_daemon) {
        syslog(LOG_ERR, "Failed to create VSCP subzone table - closed.");
        return false;
    }

    if (SQLITE_OK !=
        sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg)) {
        syslog(LOG_ERR,
               "Failed to create VSCP subzone table with error %s.",
               pErrMsg);
        return false;
    }

    // Fill with default info
    std::string sql = "BEGIN;";
    for (int i = 0; i < 256; i++) {
        sql += vscp_str_format("INSERT INTO 'subzone' (idx_subzone, name) "
                               "VALUES( %d, 'subzone%d' );",
                               i,
                               i);
    }

    sql += vscp_str_format(VSCPDB_SUBZONE_UPDATE,
                           "All subzones",
                           "Subzone = 255 represents all subzones of a zone.",
                           255L);
    sql += "COMMIT;";
    if (SQLITE_OK !=
        sqlite3_exec(
          m_db_vscp_daemon, (const char *)sql.c_str(), NULL, NULL, &pErrMsg)) {
        syslog(LOG_ERR,
               "Failed to insert last VSCP default subzone table entry %d. "
               "Error %s",
               255,
               pErrMsg);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doCreateUserdefTableTable
//
// Create the userdef table
//
//

bool
CControlObject::doCreateUserdefTableTable(void)
{
    char *pErrMsg    = 0;
    const char *psql = VSCPDB_TABLE_CREATE;

    syslog(LOG_INFO, "Creating userdef table.");

    // Check if database is open
    if (NULL == m_db_vscp_daemon) {
        syslog(LOG_ERR, "Failed to create VSCP userdef table - closed.");
        return false;
    }

    if (SQLITE_OK !=
        sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg)) {
        syslog(LOG_ERR,
               "Failed to create VSCP userdef table with error %s.",
               pErrMsg);
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// clientMsgWorkerThread
//
// Is there any messages to send from Level II clients. Send it/them to all
// devices/clients except for itself.
//

void *
clientMsgWorkerThread(void *userdata)
{
    std::list<vscpEvent *>::iterator it;
    vscpEvent *pvscpEvent = NULL;

    // Must be a valid control object pointer
    CControlObject *pObj = (CControlObject *)userdata;
    if (NULL == pObj) return NULL;

    while (!pObj->m_bQuit_clientMsgWorkerThread) {

        // Wait for event
        if ((-1 == vscp_sem_wait(&pObj->m_semClientOutputQueue, 500)) &&
            errno == ETIMEDOUT) {
            continue;
        }

        if (pObj->m_clientOutputQueue.size()) {

            pthread_mutex_lock(&pObj->m_mutexClientOutputQueue);
            pvscpEvent = pObj->m_clientOutputQueue.front();
            pObj->m_clientOutputQueue.pop_front();
            // pvscpEvent = *it;
            pthread_mutex_unlock(&pObj->m_mutexClientOutputQueue);

            if (NULL != pvscpEvent) {

                // * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
                //
                // Send event to all Level II clients (not to
                // ourself )
                //
                // * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

                pObj->sendEventAllClients(pvscpEvent, pvscpEvent->obid);

            } // Valid event

            // Delete the event
            if (NULL != pvscpEvent) vscp_deleteVSCPevent(pvscpEvent);
            pvscpEvent = NULL;

        } // while

    } // while

    return NULL;
}
