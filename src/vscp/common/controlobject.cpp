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
    // m_debugFlags1 = VSCP_DEBUG1_ALL;
    m_debugFlags1 = 0;
    // m_debugFlags1 |= VSCP_DEBUG1_DM;
    // m_debugFlags1 |= VSCP_DEBUG1_AUTOMATION;
    // m_debugFlags1 |= VSCP_DEBUG1_VARIABLE;
    // m_debugFlags1 |= VSCP_DEBUG1_MULTICAST;
    // m_debugFlags1 |= VSCP_DEBUG1_UDP;
    // m_debugFlags1 |= VSCP_DEBUG1_TCP;
    // m_debugFlags1 |= VSCP_DEBUG1_DRIVER

    m_logDays    = DEFAULT_LOGDAYS;
    m_rootFolder = ("/srv/vscp/");

    // Default admin user credentials
    m_admin_user      = ("admin");
    m_admin_password  = ("450ADCE88F2FDBB20F3318B65E53CA4A;"
                        "06D3311CC2195E80BE4F8EB12931BFEB5C"
                        "630F6B154B2D644ABE29CEBDBFB545");
    m_admin_allowfrom = ("*");
    m_vscptoken       = ("Carpe diem quam minimum credula postero");
    vscp_hexStr2ByteArray(m_systemKey,
                          32,
                          "A4A86F7D7E119BA3F0CD06881E371B989B"
                          "33B6D606A863B633EF529D64544F8E");

    m_nConfiguration = 1; // Default configuration record is read.

    // Log to syslog
    m_bLogToSysLog = true;

    m_automation.setControlObject(this);
    m_maxItemsInClientReceiveQueue = MAX_ITEMS_CLIENT_RECEIVE_QUEUE;

    // Nill the GUID
    m_guid.clear();

    // Initialize the client map
    // to all unused
    for (i = 0; i < VSCP_MAX_CLIENTS; i++) {
        m_clientMap[i] = 0;
    }

    // Local domain
    m_web_authentication_domain = "mydomain.com";

    // Set Default Log Level
    m_logLevel = DAEMON_LOGMSG_NORMAL;

    m_path_db_vscp_daemon = m_rootFolder + "vscpd.sqlite3";
    m_path_db_vscp_data   = m_rootFolder + "vscp_data.sqlite3";
    m_path_db_vscp_log    = m_rootFolder + "logs/vscpd_log.sqlite3";

    // No databases opened yet
    m_db_vscp_daemon = NULL;
    m_db_vscp_data   = NULL;
    m_db_vscp_log    = NULL;

    // Control UDP Interface
    m_udpSrvObj->m_bEnable = false;
    m_udpSrvObj->m_interface.empty();
    m_udpSrvObj->m_guid.clear();
    vscp_clearVSCPFilter(&m_udpSrvObj->m_filter);
    m_udpSrvObj->m_bAllowUnsecure = false;
    m_udpSrvObj->m_bAck           = false;

    // Default TCP/IP interface settings
    m_enableTcpip            = true;
    m_strTcpInterfaceAddress = ("9598");
    m_encryptionTcpip        = 0;
    m_tcpip_ssl_certificate.empty();
    m_tcpip_ssl_certificate_chain.empty();
    m_tcpip_ssl_verify_peer = 0; // no=0, optional=1, yes=2
    m_tcpip_ssl_ca_path.empty();
    m_tcpip_ssl_ca_file.empty();
    m_tcpip_ssl_verify_depth         = 9;
    m_tcpip_ssl_default_verify_paths = false;
    m_tcpip_ssl_cipher_list.empty();
    m_tcpip_ssl_protocol_version = 0;
    m_tcpip_ssl_short_trust      = false;

    // Default multicast announce port
    m_strMulticastAnnounceAddress = ("udp://:" + VSCP_ANNOUNCE_MULTICAST_PORT);

    // default multicast announce ttl
    m_ttlMultiCastAnnounce = IP_MULTICAST_DEFAULT_TTL;

    // Default UDP interface
    m_udpSrvObj->m_interface = ("udp://:" + VSCP_DEFAULT_UDP_PORT);

    // Web server SSL settings
    m_web_ssl_certificate          = m_rootFolder + "certs/server.pem";
    m_web_ssl_certificate_chain    = ("");
    m_web_ssl_verify_peer          = false;
    m_web_ssl_ca_path              = ("");
    m_web_ssl_ca_file              = ("");
    m_web_ssl_verify_depth         = 9;
    m_web_ssl_default_verify_paths = true;
    m_web_ssl_cipher_list      = "DES-CBC3-SHA:AES128-SHA:AES128-GCM-SHA256";
    m_web_ssl_protocol_version = 3;
    m_web_ssl_short_trust      = false;

    // Webserver interface
    m_web_bEnable         = true;
    m_web_listening_ports = ("[::]:9999r,[::]:8843s,8884");

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
    m_web_ssi_patterns                      = ("");
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

    pthread_mutex_lock(&m_udpSrvObj->m_mutexUDPInfo);
    std::deque<udpRemoteClientInfo *>::iterator iterUDP;
    for (iterUDP = m_udpSrvObj->m_remotes.begin();
         iterUDP != m_udpSrvObj->m_remotes.end();
         ++iterUDP) {
        if (NULL != *iterUDP) {
            delete *iterUDP;
            *iterUDP = NULL;
        }
    }
    m_udpSrvObj->m_remotes.clear();
    pthread_mutex_unlock(&m_udpSrvObj->m_mutexUDPInfo);

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
    m_path_db_vscp_log     = m_rootFolder + "/logs/vscpd_log.sqlite3";
    std::string strRootwww = m_rootFolder + "www";
    m_web_document_root    = strRootwww;

    // Change locale to get the correct decimal point "."
    // Set locale
    /* TODO
    int sys_lang = wxLocale::GetSystemLanguage();
    if ( sys_lang != wxLANGUAGE_DEFAULT ) {
        m_locale.Init( sys_lang );              // set custom locale
        m_locale.AddCatalogLookupPathPrefix( ("locale") );   // set "locale"
    prefix m_locale.AddCatalog( ("wxproton") );   // our private domain
        m_locale.AddCatalog( ("wxstd") );      // wx common domain is default
                                                // Restore "C" numeric locale
        setlocale( LC_NUMERIC, "C" );
    }*/

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
            vscp_string_format(str,
                               "Path=%s error=%s",
                               (const char *)m_path_db_vscp_daemon.c_str(),
                               sqlite3_errmsg(m_db_vscp_daemon));
            syslog(LOG_CRIT, "%s", (const char *)str.c_str());
            if (NULL != m_db_vscp_daemon) sqlite3_close(m_db_vscp_daemon);
            m_db_vscp_daemon = NULL;
            return false;
        } else {

            // Database is open.

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

    // * * * VSCP Daemon logging database * * *

    // Check filename
    if (vscp_fileExists(m_path_db_vscp_log)) {

        if (SQLITE_OK != sqlite3_open((const char *)m_path_db_vscp_log.c_str(),
                                      &m_db_vscp_log)) {

            // Failed to open/create the database file
            syslog(LOG_ERR,
                   "VSCP Server logging database could not be "
                   "opened. - Will not be used. Path=%s error=%s",
                   m_path_db_vscp_log.c_str(),
                   m_path_db_vscp_log.c_str());
            if (NULL != m_db_vscp_log) sqlite3_close(m_db_vscp_log);
            m_db_vscp_log = NULL;
        }
    } else {

        if (1) {

            // We need to create the database from scratch. This may not work if
            // the database is in a read only location.
            syslog(LOG_ERR,
                   "VSCP Server logging database does not exist - "
                   "will be created. Path=%s",
                   m_path_db_vscp_log.c_str());

            if (SQLITE_OK ==
                sqlite3_open((const char *)m_path_db_vscp_log.c_str(),
                             &m_db_vscp_log)) {
                // create the config. database.
                doCreateLogTable();
            } else {
                vscp_string_format(str,
                                   "Failed to create vscp log database - will "
                                   "not be used.  Error=%s",
                                   sqlite3_errmsg(m_db_vscp_log));
                syslog(LOG_ERR, "%s", (const char *)str.c_str());
                if (NULL != m_db_vscp_log) sqlite3_close(m_db_vscp_log);
                m_db_vscp_log = NULL;
            }
        } else {
            syslog(LOG_ERR,
                   "VSCP Server logging database path invalid - will not be "
                   "used. Path=%s",
                   m_path_db_vscp_log.c_str());
            if (NULL != m_db_vscp_log) sqlite3_close(m_db_vscp_log);
            m_db_vscp_log = NULL;
        }
    }

    // https://www.sqlite.org/wal.html
    // http://stackoverflow.com/questions/3852068/sqlite-insert-very-slow
    if (NULL != m_db_vscp_log) {
        sqlite3_exec(
          m_db_vscp_log, "PRAGMA journal_mode = WAL", NULL, NULL, NULL);
        sqlite3_exec(
          m_db_vscp_log, "PRAGMA synchronous = NORMAL", NULL, NULL, NULL);
    }

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
    m_driverUsername = ("drv_");
    m_driverUsername += std::string(buf);

    // Level II Driver Password (can't contain ";" character)
    memset(buf, 0, sizeof(buf));
    pw.generatePassword(32, buf);
    m_driverPassword = buf;

    std::string drvhash;
    vscp_makePasswordHash(drvhash, std::string(buf));

    m_userList.addUser(m_driverUsername,
                       drvhash,                       // salt;hash
                       ("System added driver user."), // full name
                       ("System added driver user."), // note
                       NULL,
                       ("driver"),
                       ("+127.0.0.0/24"), // Only local
                       ("*:*"),           // All events
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
        m_strServerName = ("VSCP Server @ ");
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
    pClientItem->m_strDeviceName = ("Internal Server DM Client.|Started at ");
    pClientItem->m_strDeviceName += vscpdatetime::setNow().getISODateTime();

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
        struct timespec ts;
        ts.tv_sec  = 0;
        ts.tv_nsec = 100000; // 100 ms
        if (ETIMEDOUT ==
            sem_timedwait(&pClientItem->m_semClientInputQueue, &ts)) {

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

    // Close log database
    if (NULL != m_db_vscp_log) sqlite3_close(m_db_vscp_log);
    m_db_vscp_log = NULL;

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

    if (!pthread_create(
          &m_clientMsgWorkerThread, NULL, clientMsgWorkerThread, this)) {

        syslog(LOG_CRIT,
               "Controlobject: Unable to allocate memory for controlobject "
               "client thread.");
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
    m_ptcpipSrvObject = new tcpipListenThreadObj(this);
    if (NULL == m_ptcpipSrvObject) {
        syslog(LOG_CRIT,
               "Controlobject: Failed to allocate storage for tcp/ip.");
    }

    if (!pthread_create(
          &m_tcpipListenThread, NULL, tcpipListenThread, m_ptcpipSrvObject)) {
        delete m_ptcpipSrvObject;
        syslog(LOG_CRIT,
               "Controlobject: Unable to create the tcp/ip listen thread.");
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

    syslog(LOG_DEBUG, "Controlobject: Starting UDP simple UDP interface...");

    m_udpSrvObj = new UDPSrvObj(this);
    if (NULL == m_udpSrvObj) {
        syslog(LOG_CRIT,
               "Controlobject: Unable to allocate memory for UDP objects");
    }

    if (!pthread_create(&m_UDPThread, NULL, UDPThread, m_udpSrvObj)) {

        syslog(LOG_CRIT,
               "Controlobject: Unable to allocate memory for USP thread.");
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

    m_udpSrvObj->m_bQuit = true;
    pthread_join(m_UDPThread, NULL);

    syslog(LOG_DEBUG, "Controlobject: Terminated UDP thread.");

    // Delete the USP server object
    delete m_udpSrvObj;
    m_udpSrvObj = NULL;

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
                   "Unable to start multicast channel interface thread.");
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
         ++it) {

        multicastChannelItem *pChannel = *it;
        if (NULL == pChannel) {
            syslog(LOG_ERR,
                   "Controlobject: Multicast end channel table invalid entry.");
            continue;
        }

        pChannel->m_quit = true;
        pthread_join(pChannel->m_workerThread, NULL);
        delete pChannel;
        m_multicastObj.m_channels.erase(it);
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

    if (!pthread_create(&m_clientMsgWorkerThread,
                        NULL,
                        daemonWorkerThread,
                        m_pdaemonWorkerObj)) {

        syslog(
          LOG_CRIT,
          "Controlobject: Unable to allocate memory for daemon worker thread.");
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
//
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

            // *** Level 3 Driver * * *   TODO    Check this!
            if (VSCP_DRIVER_LEVEL3 == pDeviceItem->m_driverLevel) {

                //  Startup Level III driver
                std::string executable = pDeviceItem->m_strPath;
                /*   TODO
                pDeviceItem->m_pDriver3Process = new Driver3Process;
                if ( 0 == ( pDeviceItem->m_pid =
                        wxExecute( executable.c_str(),
                                    wxEXEC_ASYNC,
                                    pDeviceItem->m_pDriver3Process ) ) ) {
                    syslog( LOG_ERR,
                                "Controlobject: Failed to load level III driver:
                %s", pDeviceItem->m_strName.c_str() ); return NULL;
                }
                else {

                }*/
            }

            syslog(LOG_DEBUG,
                   "Controlobject: [Driver] - Starting: %s ",
                   pDeviceItem->m_strName.c_str());

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

    memset(pCapability, 0, 8);

    // VSCP Multicast interface
    if (m_bEnableMulticast) {
        pCapability[8 - (VSCP_SERVER_CAPABILITY_MULTICAST_CHANNEL / 8)] |=
          (1 << (VSCP_SERVER_CAPABILITY_MULTICAST_CHANNEL % 8));
    }

    // VSCP TCP/IP interface
    if (m_enableTcpip) {
        pCapability[8 - (VSCP_SERVER_CAPABILITY_TCPIP / 8)] |=
          (1 << (VSCP_SERVER_CAPABILITY_TCPIP % 8));
    }

    // VSCP UDP interface
    if (m_udpSrvObj->m_bEnable) {
        pCapability[8 - (VSCP_SERVER_CAPABILITY_UDP / 8)] |=
          (1 << (VSCP_SERVER_CAPABILITY_UDP % 8));
    }

    // VSCP Multicast announce interface
    if (m_bEnableMulticastAnounce) {
        pCapability[8 - (VSCP_SERVER_CAPABILITY_MULTICAST_ANNOUNCE / 8)] |=
          (1 << (VSCP_SERVER_CAPABILITY_MULTICAST_ANNOUNCE % 8));
    }

    // VSCP raw Ethernet interface
    if (1) {
        pCapability[8 - (VSCP_SERVER_CAPABILITY_RAWETH / 8)] |=
          (1 << (VSCP_SERVER_CAPABILITY_RAWETH % 8));
    }

    // VSCP web server
    if (m_web_bEnable) {
        pCapability[8 - (VSCP_SERVER_CAPABILITY_WEB / 8)] |=
          (1 << (VSCP_SERVER_CAPABILITY_WEB % 8));
    }

    // VSCP websocket interface
    if (m_web_bEnable) {
        pCapability[8 - (VSCP_SERVER_CAPABILITY_WEBSOCKET / 8)] |=
          (1 << (VSCP_SERVER_CAPABILITY_WEBSOCKET % 8));
    }

    // VSCP websocket interface
    if (m_web_bEnable) {
        pCapability[8 - (VSCP_SERVER_CAPABILITY_REST / 8)] |=
          (1 << (VSCP_SERVER_CAPABILITY_REST % 8));
    }

    // IPv6 support
    if (0) {
        pCapability[8 - (VSCP_SERVER_CAPABILITY_IP6 / 8)] |=
          (1 << (VSCP_SERVER_CAPABILITY_IP6 % 8));
    }

    // IPv4 support
    if (0) {
        pCapability[8 - (VSCP_SERVER_CAPABILITY_IP4 / 8)] |=
          (1 << (VSCP_SERVER_CAPABILITY_IP4 % 8));
    }

    // SSL support
    if (1) {
        pCapability[8 - (VSCP_SERVER_CAPABILITY_SSL / 8)] |=
          (1 << (VSCP_SERVER_CAPABILITY_SSL % 8));
    }

    // +2 tcp/ip connections support
    if (m_enableTcpip) {
        pCapability[8 - (VSCP_SERVER_CAPABILITY_TWO_CONNECTIONS / 8)] |=
          (1 << (VSCP_SERVER_CAPABILITY_TWO_CONNECTIONS % 8));
    }

    // AES256
    pCapability[15] |= (1 << 2);

    // AES192
    pCapability[15] |= (1 << 1);

    // AES128
    pCapability[15] |= 1;

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// logMsg
//

void
CControlObject::logMsg(const std::string &msgin,
                       const uint8_t level,
                       const uint8_t nType)
{
    syslog(LOG_INFO, "%s", msgin.c_str());
    /*
    // Log to database
    if ( ( NULL != m_db_vscp_log ) &&
            ( m_logLevel >= level) ) {

        char *zErrMsg = NULL;

        char *sql = sqlite3_mprintf( VSCPDB_LOG_INSERT,
            nType,
            (const char *)(datetime.FormatISODate() + ("T") +
                datetime.FormatISOTime() ).c_str(),
            level,
            (const char *)msg.c_str() );

        if ( SQLITE_OK != sqlite3_exec( m_db_vscp_log,
                                        sql, NULL, NULL, &zErrMsg ) ) {
            wxPrintf( "Failed to write message to log database. Error is: "
                      "%s -- Message is: %s",
                        zErrMsg,
                        (const char *)msg.c_str() );
        }
        sqlite3_free( sql );

        // Clean up old entries
        if ( -1 != m_logDays ) {
            sql = sqlite3_mprintf( VSCPDB_LOG_DELETE_OLD, m_logDays );
            sqlite3_exec( m_db_vscp_log, sql, NULL, NULL, &zErrMsg );
            sqlite3_free( sql );
        }

    }
*/
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
      vscp_string_format("SELECT count(*)from %s", (const char *)table.c_str());

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

/////////////////////////////////////////////////////////////////////////////
// getCountRecordsLogDB
//

long
CControlObject::getCountRecordsLogDB(void)
{
    long count = 0;
    /*
    sqlite3_stmt *ppStmt;

    // If not open no records
    if ( NULL == m_db_vscp_log ) return 0;


    pthread_mutex_lock( &m_mutexLogWrite );

    if ( SQLITE_OK != sqlite3_prepare( m_db_vscp_log,
                                        VSCPDB_LOG_COUNT,
                                        -1,
                                        &ppStmt,
                                        NULL ) )  {
        wxPrintf( "Failed to prepare count for log database. SQL is %s",
                        VSCPDB_LOG_COUNT  );
        return 0;
    }

    if ( SQLITE_ROW == sqlite3_step( ppStmt ) ) {
        count = sqlite3_column_int( ppStmt, 0 );
    }

    pthread_mutex_unlock( &m_mutexLogWrite );
*/
    return count;
}

/////////////////////////////////////////////////////////////////////////////
// searchLogDB
//

bool
CControlObject::searchLogDB(const char *sql, std::string &strResult)
{
    /*
    sqlite3_stmt *ppStmt;

    // If not open no records
    if ( NULL == m_db_vscp_log ) return 0;


    pthread_mutex_lock( &m_mutexLogWrite );

    if ( SQLITE_OK != sqlite3_prepare( m_db_vscp_log,
                                        sql,
                                        -1,
                                        &ppStmt,
                                        NULL ) )  {
        syslog( LOG_ERR,
                    "Failed to get records from log "
                    "database. SQL is %s"),
                    sql.c_str() );
        pthread_mutex_unlock( &m_mutexLogWrite );
        return false;
    }

    while ( SQLITE_ROW == sqlite3_step( ppStmt ) ) {
        std::string str;
        str = sqlite3_column_text( ppStmt, 0 );
        str += (",");
        str += sqlite3_column_text( ppStmt, 1 );
        str += (",");
        str += sqlite3_column_text( ppStmt, 2 );
        str += (",");
        str += sqlite3_column_text( ppStmt, 3 );
        str += (",");
        str += sqlite3_column_text( ppStmt, 4 );

        str += ("|");
        strResult += str;
    }

    pthread_mutex_unlock( &m_mutexLogWrite );
*/
    return true;
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

///////////////////////////////////////////////////////////////////////////////
//  removeIdFromClientMap
//

bool
CControlObject::removeIdFromClientMap(uint32_t clid)
{
    for (uint32_t i = 0; i < VSCP_MAX_CLIENTS; i++) {
        if (clid == m_clientMap[i]) {
            m_clientMap[i] = 0;
            return true;
        }
    }

    return false;
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

    // Remove the mapped item
    removeIdFromClientMap(pClientItem->m_clientID);

    // Remove the client
    m_clientList.removeClient(pClientItem);
}

//////////////////////////////////////////////////////////////////////////////
// addKnowNode
//

void
CControlObject::addKnownNode(cguid &guid, cguid &ifguid, std::string &name)
{
    ; // TODO ???
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
#ifdef __WXDEBUG__
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

static int depth_general_config_parser   = 0;
static char *last_general_config_content = NULL;

static void
startGeneralConfigParser(void *data, const char *name, const char **attr)
{
    CControlObject *pObj = (CControlObject *)data;
    if (NULL == data) return;

    if (0 == depth_general_config_parser) {

        if ((0 == vscp_strcasecmp(name, "security"))) {

            for (int i = 0; attr[i]; i += 2) {

                std::string attribute = attr[i + 1];
                if (0 == vscp_strcasecmp(attr[i], "user")) {
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
        } else if ((0 == vscp_strcasecmp(name, "daemon"))) {
            for (int i = 0; attr[i]; i += 2) {

                std::string attribute = attr[i + 1];
                if (0 == vscp_strcasecmp(attr[i], "loglevel")) {

                    vscp_trim(attribute);

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
                            pObj->m_logLevel = DAEMON_LOGMSG_NORMAL;
                        }
                    }
                } else if (0 == vscp_strcasecmp(attr[i], "clientbuffersize")) {
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
        } else if ((0 == vscp_strcasecmp(name, "debugflags"))) {

            for (int i = 0; attr[i]; i += 2) {

                std::string attribute = attr[i + 1];
                if (0 == strcasecmp(attr[i], "flags1")) {
                    pObj->m_debugFlags1 = vscp_readStringValue(attribute);
                } else if (0 == strcasecmp(attr[i], "flags2")) {
                    pObj->m_debugFlags2 = vscp_readStringValue(attribute);
                } else if (0 == strcasecmp(attr[i], "flags3")) {
                    pObj->m_debugFlags3 = vscp_readStringValue(attribute);
                } else if (0 == strcasecmp(attr[i], "flags4")) {
                    pObj->m_debugFlags4 = vscp_readStringValue(attribute);
                } else if (0 == strcasecmp(attr[i], "flags5")) {
                    pObj->m_debugFlags5 = vscp_readStringValue(attribute);
                } else if (0 == strcasecmp(attr[i], "flags6")) {
                    pObj->m_debugFlags6 = vscp_readStringValue(attribute);
                } else if (0 == strcasecmp(attr[i], "flags7")) {
                    pObj->m_debugFlags7 = vscp_readStringValue(attribute);
                }
            }
        } else if ((0 == vscp_strcasecmp(name, "db_vscp_daemon"))) {
            for (int i = 0; attr[i]; i += 2) {

                std::string attribute = attr[i + 1];
                if (0 == strcasecmp(attr[i], "path")) {
                    vscp_trim(attribute);
                    if (attribute.length()) {
                        pObj->m_path_db_vscp_daemon = attribute;
                    }
                }
            }
        } else if ((0 == vscp_strcasecmp(name, "db_vscp_data"))) {
            for (int i = 0; attr[i]; i += 2) {

                std::string attribute = attr[i + 1];
                if (0 == strcasecmp(attr[i], "path")) {
                    vscp_trim(attribute);
                    if (attribute.length()) {
                        pObj->m_path_db_vscp_data = attribute;
                    }
                }
            }
        } else if ((0 == vscp_strcasecmp(name, "db_vscp_variable"))) {
            for (int i = 0; attr[i]; i += 2) {

                std::string attribute = attr[i + 1];
                if (0 == strcasecmp(attr[i], "path")) {
                    vscp_trim(attribute);
                    if (attribute.length()) {
                        pObj->m_variables.m_dbFilename = attribute;
                    }
                }
            }
        } else if ((0 == vscp_strcasecmp(name, "db_vscp_dm"))) {
            for (int i = 0; attr[i]; i += 2) {

                std::string attribute = attr[i + 1];
                if (0 == strcasecmp(attr[i], "path")) {
                    vscp_trim(attribute);
                    if (attribute.length()) {
                        pObj->m_dm.m_path_db_vscp_dm = attribute;
                    }
                }
            }
        } else if ((0 == vscp_strcasecmp(name, "accesslogfile"))) {
            for (int i = 0; attr[i]; i += 2) {

                std::string attribute = attr[i + 1];
                if (0 == strcasecmp(attr[i], "enable")) {
                } else if (0 == strcasecmp(attr[i], "path")) {
                    // TODO
                }
            }
        }
    }

    depth_general_config_parser++;
}

static void
handle_general_config_data(void *data, const char *content, int length)
{
    int prevLength = (NULL == last_general_config_content)
                       ? 0
                       : strlen(last_general_config_content);
    char *tmp = (char *)malloc(length + 1 + prevLength);
    strncpy(tmp, content, length);
    tmp[length] = '\0';

    if (NULL == last_general_config_content) {
        tmp = (char *)malloc(length + 1);
        strncpy(tmp, content, length);
        tmp[length]                 = '\0';
        last_general_config_content = tmp;
    } else {
        // Concatenate
        int newlen = length + 1 + strlen(last_general_config_content);
        last_general_config_content =
          (char *)realloc(last_general_config_content, newlen);
        strncat(tmp, content, length);
        last_general_config_content[newlen] = '\0';
    }
}

static void
endGeneralConfigParser(void *data, const char *name)
{
    if (NULL != last_general_config_content) {
        // Free the allocated data
        free(last_general_config_content);
        last_general_config_content = NULL;
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
    XML_SetCharacterDataHandler(xmlParser, handle_general_config_data);

    int bytes_read;
    void *buf = XML_GetBuffer(xmlParser, XML_BUFF_SIZE);
    if (NULL == buf) {
        XML_ParserFree(xmlParser);
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

///////////////////////////////////////////////////////////////////////////////
// readConfigurationXML
//
// Read the configuration XML file
//

bool
CControlObject::readConfigurationXML(const std::string &strcfgfile)
{
    /*
    unsigned long val;
    std::string attribute;
    wxXmlDocument doc;

    syslog( LOG_DEBUG,
                "Reading full XML configuration from [%s]",
                (const char *)strcfgfile.c_str()  );

    if ( !doc.Load( strcfgfile ) ) {
        syslog( LOG_CRIT,
                "Can't load logfile. Is path correct?" );
        return false;
    }

    // start processing the XML file
    if ( 0 != vscp_strcasecmp( doc.GetRoot()->GetName().c_str(), "vscpconfig" )
{ syslog( LOG_CRIT, "Can't read logfile. Maybe it is invalid!" ); return false;
    }

    wxXmlNode *child = doc.GetRoot()->GetChildren();
    while ( child ) {

        // The "general" settings are read in a pre-step
(readXMLConfigurationGeneral)

        if ( 0 == vscp_strcasecmp( child->GetName().c_str(), "tcpip" ) {

            // Enable
            attribute = child->GetAttribute(("enable"), ("true"));
            if ( 0 == vscp_strcasecmp( attribute, "false" ) ) {
                m_enableTcpip = false;
            }
            else {
                m_enableTcpip  = true;
            }

            m_strTcpInterfaceAddress = child->GetAttribute(("interface"),
m_strTcpInterfaceAddress ); m_strTcpInterfaceAddress.StartsWith("tcp://",
&m_strTcpInterfaceAddress ); vscp_trim( m_strTcpInterfaceAddress );

            m_tcpip_ssl_certificate = child->GetAttribute(("ssl_certificate"),
m_tcpip_ssl_certificate ); vscp_trim( m_tcpip_ssl_certificate );

            m_tcpip_ssl_certificate_chain =
child->GetAttribute(("ssl_verify_peer"), m_tcpip_ssl_certificate_chain );
            vscp_trim( m_tcpip_ssl_certificate_chain );

            m_tcpip_ssl_verify_peer = vscp_readStringValue( child->GetAttribute(
("ssl_certificate_chain"), ("0") ) );

            m_tcpip_ssl_ca_path = child->GetAttribute(("ssl_ca_path"), (""));
            vscp_trim( m_tcpip_ssl_ca_path );

            m_tcpip_ssl_ca_file = child->GetAttribute(("ssl_ca_file"), (""));
            vscp_trim( m_tcpip_ssl_ca_file );

            m_tcpip_ssl_verify_depth = vscp_readStringValue(
child->GetAttribute( ("ssl_verify_depth"), ("9") ) );

            attribute = child->GetAttribute(("ssl_default_verify_paths"),
("true")); if ( 0 == vscp_strcasecmp( attribute, "false" ) ) {
                m_tcpip_ssl_default_verify_paths = false;
            }
            else {
                m_tcpip_ssl_default_verify_paths  = true;
            }

            m_tcpip_ssl_cipher_list = child->GetAttribute(("ssl_cipher_list"),
m_tcpip_ssl_cipher_list ); vscp_trim( m_tcpip_ssl_cipher_list );

            if ( child->GetAttribute(("ssl_protocol_version"), &attribute ) ) {
                m_tcpip_ssl_protocol_version = vscp_readStringValue( attribute )
;
            }

            if ( child->GetAttribute(("ssl_short_trust"), &attribute ) ) {
                if ( 0 == vscp_strcasecmp( attribute, "false" ) ) {
                    m_tcpip_ssl_short_trust = false;
                }
                else {
                    m_tcpip_ssl_short_trust  = true;
                }
            }
        }
        else if ( 0 == vscp_strcasecmp( child->GetName().c_str(),
"multicast-announce" ) ) {

            // Enable
            if ( child->GetAttribute(("enable"), &attribute ) ) {
                if ( 0 == vscp_strcasecmp( attribute.c_str(), "false" ) ) {
                    m_bEnableMulticastAnounce = false;
                }
                else {
                    m_bEnableMulticastAnounce  = true;
                }
            }

            // interface
            if ( child->GetAttribute(("enable"), &attribute ) ) {
                m_strMulticastAnnounceAddress = child->GetAttribute( attribute
);
            }

            // ttl
            if ( child->GetAttribute(("ttl"), &attribute ) ) {
                m_ttlMultiCastAnnounce = vscp_readStringValue( attribute );
            }

        }
        else if ( 0 == vscp_strcasecmp( child->GetName().c_str(), "udp" ) {

            pthread_mutex_lock( &m_mutexUDPInfo );

            // Default is to let everything come through
            vscp_clearVSCPFilter( &m_udpSrvObj->m_filter );

            // Enable
            if ( child->GetAttribute(("enable"), &attribute ) ) {
                if ( 0 == vscp_strcasecmp( attribute.c_str(), "false" ) ) {
                    m_udpSrvObj->m_bEnable = false;
                }
                else {
                    m_udpSrvObj->m_bEnable  = true;
                }
            }

            // Allow insecure connections
            if ( child->GetAttribute(("bAllowUnsecure"), &attribute ) ) {
                if ( 0 == vscp_strcasecmp( attribute.c_str(), "false" ) ) {
                    m_udpSrvObj->m_bAllowUnsecure = false;
                }
                else {
                    m_udpSrvObj->m_bAllowUnsecure  = true;
                }
            }

            // Enable ACK
            if ( child->GetAttribute(("bSendAck"), &attribute ) ) {
                if ( 0 == vscp_strcasecmp( attribute.c_str(), "false" ) ) {
                    m_udpSrvObj->m_bAck = false;
                }
                else {
                    m_udpSrvObj->m_bAck = true;
                }
            }

            // Username
            m_udpSrvObj->m_user = child->GetAttribute( ("user"),
m_udpSrvObj->m_user );

            // Password
            m_udpSrvObj->m_password = child->GetAttribute( ("password"),
m_udpSrvObj->m_password );

            // Interface
            m_udpSrvObj->m_interface = child->GetAttribute( ("interface"),
m_udpSrvObj->m_interface );

            // GUID
            if ( child->GetAttribute(("guid"), &attribute ) ) {
                m_udpSrvObj->m_guid.getFromString( attribute );
            }

            // Filter
            if ( child->GetAttribute(("filter"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    vscp_readFilterFromString( &m_udpSrvObj->m_filter, attribute
);
                }
            }

            // Mask
            if ( child->GetAttribute(("mask"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    vscp_readMaskFromString( &m_udpSrvObj->m_filter, attribute
);
                }
            }

            wxXmlNode *subchild = child->GetChildren();
            while ( subchild ) {

                if ( subchild->GetName() == ("rxnode") ) {

                    udpRemoteClientInfo *pudpClient = new udpRemoteClientInfo;
                    if ( NULL == pudpClient ) {
                        syslog( LOG_ERR, "Failed to allocated UDP client remote
structure."); pthread_mutex_unlock( &m_udpSrvObj->m_mutexUDPInfo ); subchild =
subchild->GetNext(); continue;
                    }

                    // Default is to let everything come through
                    vscp_clearVSCPFilter( &pudpClient->m_filter );

                    if ( subchild->GetAttribute( ("enable"), &attribute ) ) {
                        if ( 0 == vscp_strcasecmp( attribute.c_str(), "false" )
) ) { pudpClient->m_bEnable = false;
                        }
                        else {
                            pudpClient->m_bEnable = true;
                        }
                    }

                    if ( !pudpClient->m_bEnable ) {
                        delete pudpClient;
                        pthread_mutex_unlock( &m_udpSrvObj->m_mutexUDPInfo );
                        subchild = subchild->GetNext();
                        continue;
                    }

                    // remote address
                    pudpClient->m_remoteAddress = subchild->GetAttribute(
("interface"), pudpClient->m_remoteAddress );

                    // Filter
                    if ( subchild->GetAttribute( ("filter"), &attribute ) ) {
                        vscp_trim( attribute );
                        if ( attribute.length() ) {
                            vscp_readFilterFromString( &pudpClient->m_filter,
attribute );
                        }
                    }

                    // Mask
                    if ( subchild->GetAttribute( ("mask"), &attribute ) ) {
                        vscp_trim( attribute );
                        if ( attribute.length() ) {
                            vscp_readMaskFromString( &pudpClient->m_filter,
attribute );
                        }
                    }

                    // broadcast
                    if ( subchild->GetAttribute( ("bsetbroadcast"), &attribute )
) { if ( 0 == vscp_strcasecmp( attribute.c_str(), "false" ) ) {
                            pudpClient->m_bSetBroadcast = false;
                        }
                        else {
                            pudpClient->m_bSetBroadcast = true;
                        }
                    }

                    // encryption
                    if ( subchild->GetAttribute( ("encryption"), &attribute ) )
{ pudpClient->m_nEncryption = vscp_getEncryptionCodeFromToken( attribute );
                    }

                    // add to list
                    pudpClient->m_index = 0;
                    m_udpSrvObj->m_remotes.push_back( pudpClient );

                }

                subchild = subchild->GetNext();

            } // while

            pthread_mutex_unlock( &m_udpSrvObj->m_mutexUDPInfo );

        } // udp

        else if ( 0 == vscp_strcasecmp( child->GetName().c_str(), "multicast" )
) {

            if ( child->GetAttribute( "enable", &attribute ) ) {
                if ( 0 == vscp_strcasecmp( attribute.c_str(), "false" ) ) {
                    m_bEnableMulticast = false;
                }
                else {
                    m_bEnableMulticast = true;
                }
            }

            wxXmlNode *subchild = child->GetChildren();
            while ( subchild ) {

                if ( 0 == vscp_strcasecmp( subchild->GetName().c_str(),
"channel" ) ) {

                    multicastChannelItem *pChannel = new multicastChannelItem;
                    if ( NULL == pChannel ) {
                        syslog( LOG_ERR, "Failed to allocated multicast channel
structure." ); subchild = subchild->GetNext(); continue;
                    }

                    pChannel->m_bEnable = false;
                    pChannel->m_bAllowUnsecure = false;
                    pChannel->m_port = 0;
                    pChannel->m_ttl = 1;
                    pChannel->m_nEncryption = 0;
                    pChannel->m_bSendAck = 0;
                    pChannel->m_index = 0;

                    // No worker thread started yet
                    pChannel->m_pWorkerThread = NULL;

                    // Default is to let everything come through
                    vscp_clearVSCPFilter( &pChannel->m_txFilter );
                    vscp_clearVSCPFilter( &pChannel->m_rxFilter );

                    // Enable
                    if ( subchild->GetAttribute( ("enable"), &attribute ) ) {
                        if ( 0 == vscp_strcasecmp( attribute.c_str(), "false" )
) { pChannel->m_bEnable = false;
                        }
                        else {
                            pChannel->m_bEnable = true;
                        }
                    }

                    // bSendAck
                    if ( subchild->GetAttribute( ("bsendack"), &attribute ) ) {
                        if ( 0 == vscp_strcasecmp( attribute, "false" ) ) {
                            pChannel->m_bSendAck = false;
                        }
                        else {
                            pChannel->m_bSendAck = true;
                        }
                    }

                    // bAllowUndsecure
                    if ( subchild->GetAttribute( ("ballowunsecure"), &attribute
) ) { if ( 0 == vscp_strcasecmp( attribute.c_str(), "false" ) ) {
                            pChannel->m_bAllowUnsecure = false;
                        }
                        else {
                            pChannel->m_bAllowUnsecure = true;
                        }
                    }

                    // Interface
                    subchild->GetAttribute( ("public"), pChannel->m_public );

                    // Interface
                    if ( subchild->GetAttribute( ("ballowunsecure"), &attribute
) ) { pChannel->m_port = vscp_readStringValue( attribute );
                    }

                    // Group
                    subchild->GetAttribute( ("group"), pChannel->m_gropupAddress
);

                    // ttl
                    if ( subchild->GetAttribute( ("ttl"), &attribute ) ) {
                        pChannel->m_ttl = vscp_readStringValue( attribute );
                    }

                    // guid
                    if ( subchild->GetAttribute( ("guid"), &attribute ) ) {
                        pChannel->m_guid.getFromString( attribute );
                    }

                    // TX Filter
                    if ( subchild->GetAttribute( ("txfilter"), &attribute ) ) {
                        vscp_trim( attribute );
                        if ( attribute.length() ) {
                            vscp_readFilterFromString( &pChannel->m_txFilter,
attribute );
                        }
                    }

                    // TX Mask
                    if ( subchild->GetAttribute( ("txmask"), &attribute ) ) {
                        vscp_trim( attribute );
                        if ( attribute.length() ) {
                            vscp_readMaskFromString( &pChannel->m_txFilter,
attribute );
                        }
                    }

                    // RX Filter
                    if ( subchild->GetAttribute( ("rxfilter"), &attribute ) ) {
                        vscp_trim( attribute );
                        if ( attribute.length() ) {
                            vscp_readFilterFromString( &pChannel->m_rxFilter,
attribute );
                        }
                    }

                    // RX Mask
                    if ( subchild->GetAttribute( ("rxmask"), &attribute ) ) {
                        vscp_trim( attribute );
                        if ( attribute.length() ) {
                            vscp_readMaskFromString( &pChannel->m_rxFilter,
attribute );
                        }
                    }

                    // encryption
                    if ( subchild->GetAttribute( ("encryption"), &attribute ) )
{ pChannel->m_nEncryption = vscp_getEncryptionCodeFromToken( attribute );
                    }

                    // add to list
                    pChannel->m_index = 0;
                    m_multicastInfo.m_channels.Append( pChannel );

                }

                subchild = subchild->GetNext();

            } // while

        }

        else if ( 0 == vscp_strcasecmp( child->GetName().c_str(), "dm" ) ) {

            // Should the internal DM be disabled
            std::string attribut;

            // Get the path to the DM file  (Deprecated)
            if ( child->GetAttribute( ("path"), &attribute ) ) {
                if ( attribut.length() ) {
                    m_dm.m_staticXMLPath = attribut;
                }
            }

            // Get the path to the DM file
            if ( child->GetAttribute( ("pathxml"), &attribute ) ) {
                if ( attribut.length() ) {
                    m_dm.m_staticXMLPath = attribut;
                }
            }

            // Get the path to the DM db file
            if ( child->GetAttribute( ("pathdb"), &attribute ) ) {
                if ( attribut.length() ) {
                    m_dm.m_path_db_vscp_dm = attribut;
                }
            }

            // Get the DM XML save flag
            if ( child->GetAttribute( ("allowxmlsave"), &attribute ) ) {
                if ( 0 == vscp_strcasecmp( attribute.c_str(), "true" ) ) {
                    m_dm.bAllowXMLsave = true;
                }
            }

            // Get the DM loglevel
            if ( child->GetAttribute( ("loglevel"), &attribute ) ) {
                if ( 0 == vscp_strcasecmp( attribute.c_str(), "debug" ) ) {
                    m_debugFlags1 |= VSCP_DEBUG1_DM;
                }
            }

        }

        else if ( 0 == vscp_strcasecmp( child->GetName().c_str(), "variables" )
) {

            // Get the path to the DM file
            if ( child->GetAttribute( ("path"), &attribute ) ) {
                m_variables.m_xmlPath = attribute;
            }

            if ( child->GetAttribute( ("pathxml"), &attribute ) ) {
                m_variables.m_xmlPath = attribute;
            }

            if ( child->GetAttribute( ("pathdb"), &attribute ) ) {
                m_variables.m_dbFilename = attribute;
            }

        }

        else if ( 0 == vscp_strcasecmp( child->GetName().c_str(), "webserver" )
) {

            std::string attribute;

            // Enable
            if ( child->GetAttribute( ("enable"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    if ( 0 == vscp_strcasecmp( attribute.c_str(), "false" )) ) {
                        m_web_bEnable = false;
                    }
                    else {
                        m_web_bEnable = true;
                    }
                }
            }

            if ( child->GetAttribute( ("document_root"), &attribute ) ) {
                vscy_trim( attribute );
                if ( attribute.length() ) {
                    m_web_document_root = attribute;
                }
            }

            if ( child->GetAttribute( ("listening_ports"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    m_web_listening_ports = attribute;
                }
            }

            if ( child->GetAttribute( ("index_files"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    m_web_index_files = attribute;
                }
            }

            if ( child->GetAttribute( ("authentication_domain"), &attribute ) )
{ vscp_trim( attribute ); if ( attribute.length() ) {
                    m_web_authentication_domain = attribute;
                }
            }

            if ( child->GetAttribute( ("enable_auth_domain_check"), &attribute )
) { vscp_trim( attribute ); if ( attribute.length() ) { if ( 0 ==
vscp_strcasecmp( attribute.c_str(), "false" ) ) { m_enable_auth_domain_check =
false;
                    }
                    else {
                        m_enable_auth_domain_check = true;
                    }
                }
            }

            if ( child->GetAttribute( ("ssl_certificat"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    m_web_ssl_certificate = attribute;
                }
            }

            if ( child->GetAttribute( ("ssl_certificat_chain"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    m_web_ssl_certificate_chain = attribute;
                }
            }

            if ( child->GetAttribute( ("ssl_verify_peer"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    if ( 0 == vscp_strcasecmp( attribute.c_str(), "false" ) ) {
                        m_web_ssl_verify_peer = false;
                    }
                    else {
                        m_web_ssl_verify_peer = true;
                    }
                }
            }

            if ( child->GetAttribute( ("ssl_ca_path"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    m_web_ssl_ca_path = attribute;
                }
            }

            if ( child->GetAttribute( ("ssl_ca_file"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    m_web_ssl_ca_file = attribute;
                }
            }

            if ( child->GetAttribute( ("ssl_verify_depth"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    m_web_ssl_verify_depth = atoi( (const char
*)attribute.c_str() );
                }
            }

            if ( child->GetAttribute( ("ssl_default_verify_paths"), &attribute )
) { vscp_trim( attribute ); if ( attribute.length() ) { if ( 0 ==
vscp_strcasecmp( attribute.c_str(), "false" ) ) { m_web_ssl_default_verify_paths
= false;
                    }
                    else {
                        m_web_ssl_default_verify_paths = true;
                    }
                }
            }

            if ( child->GetAttribute( ("ssl_cipher_list"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    m_web_ssl_cipher_list = attribute.c_str();
                }
            }

            if ( child->GetAttribute( ("ssl_protcol_version"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    m_web_ssl_protocol_version = atoi( (const char
*)attribute.c_str() );
                }
            }

            if ( child->GetAttribute( ("ssl_short_trust"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    if ( 0 == vscp_strcasecmp( attribute.c_str(), "false" ) ) {
                        m_web_ssl_short_trust = false;
                    }
                    else {
                        m_web_ssl_short_trust = true;
                    }
                }
            }

            if ( child->GetAttribute( ("cgi_interpreter"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    m_web_cgi_interpreter = attribute;
                }
            }

            if ( child->GetAttribute( ("cgi_pattern"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    m_web_cgi_patterns =attribute;
                }
            }

            if ( child->GetAttribute( ("cgi_environment"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    m_web_cgi_environment = attribute;
                }
            }

            if ( child->GetAttribute( ("protect_uri"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    m_web_protect_uri = attribute;
                }
            }

            if ( child->GetAttribute( ("trottle"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    m_web_trottle = attribute;
                }
            }

            if ( child->GetAttribute( ("enable_directory_listing"), &attribute )
) { vscp_trim( attribute ); if ( attribute.length() ) { if ( 0 ==
vscp_strcasecmp( attribute.c_str(), "false" ) ) { m_web_enable_directory_listing
= false;
                    }
                    else {
                        m_web_enable_directory_listing = true;
                    }
                }
            }

            if ( child->GetAttribute( ("enable_keep_alive"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    if ( 0 == vscp_strcasecmp( attribute.c_str(), "false" ) ) {
                        m_web_enable_keep_alive = false;
                    }
                    else {
                        m_web_enable_keep_alive = true;
                    }
                }
            }

            if ( child->GetAttribute( ("keep_alive_timeout_ms"), &attribute ) )
{ vscp_trim( attribute ); if ( attribute.length() ) {
                    m_web_keep_alive_timeout_ms = atol( (const char
*)attribute.c_str() );
                }
            }

            if ( child->GetAttribute( ("access_control_list"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    m_web_access_control_list = attribute;
                }
            }

            if ( child->GetAttribute( ("extra_mime_types"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    m_web_extra_mime_types = attribute;
                }
            }

            if ( child->GetAttribute( ("num_threads"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    m_web_num_threads = atoi( (const char *)attribute.c_str() );
                }
            }

            if ( child->GetAttribute( ("hide_file_pattern"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    m_web_hide_file_patterns = attribute;
                }
            }

            if ( child->GetAttribute( ("run_as_user"), &attribute ) ) {
                vscp_trim( attribute );
                m_web_run_as_user = attribute;
            }

            if ( child->GetAttribute( ("url_rewrite_patterns"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    m_web_url_rewrite_patterns = attribute;
                }
            }

            if ( child->GetAttribute( ("hide_file_patterns"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    m_web_hide_file_patterns = attribute;
                }
            }

            if ( child->GetAttribute( ("request_timeout_ms"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    m_web_request_timeout_ms = atol( (const char
*)attribute.c_str() );
                }
            }

            if ( child->GetAttribute( ("linger_timeout_ms"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    m_web_linger_timeout_ms = atol( (const char
*)attribute.c_str() );
                }
            }

            if ( child->GetAttribute( ("decode_url"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    if ( 0 == vscp_strcasecmp( attribute.c_str(), "false" ) ) {
                        m_web_decode_url = false;
                    }
                    else {
                        m_web_decode_url = true;
                    }
                }
            }

            if ( child->GetAttribute( ("global_auth_file"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    m_web_global_auth_file = attribute;
                }
            }

            if ( child->GetAttribute( ("web_per_directory_auth_file"),
&attribute ) ) { vscp_trim( attribute ); if ( attribute.length() ) {
                    m_web_per_directory_auth_file = attribute;
                }
            }

            if ( child->GetAttribute( ("ssi_pattern"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    m_web_ssi_patterns = attribute;
                }
            }

            if ( child->GetAttribute( ("access_control_allow_origin"),
&attribute ) ) { vscp_trim( attribute ); if ( attribute.length() ) {
                    m_web_access_control_allow_origin = attribute;
                }
            }

            if ( child->GetAttribute( ("access_control_allow_methods"),
&attribute ) ) { vscp_trim( attribute ); if ( attribute.length() ) {
                    m_web_access_control_allow_methods = attribute;
                }
            }

            if ( child->GetAttribute( ("access_control_allow_headers"),
&attribute ) ) { vscp_trim( attribute ); if ( attribute.length() ) {
                    m_web_access_control_allow_headers = attribute;
                }
            }

            if ( child->GetAttribute( ("error_pages"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    m_web_error_pages = attribute;
                }
            }

            if ( child->GetAttribute( ("tcp_nodelay"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    m_web_linger_timeout_ms = atol( (const char
*)attribute.c_str() );
                }
            }

            if ( child->GetAttribute( ("static_file_max_age"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    m_web_static_file_max_age = atol( (const char
*)attribute.c_str() );
                }
            }

            if ( child->GetAttribute( ("strict_transport_security_max_age"),
&attribute ) ) { vscp_trim( attribute ); if ( attribute.length() ) {
                    m_web_strict_transport_security_max_age = atol( (const char
*)attribute.c_str() );
                }
            }

            if ( child->GetAttribute( ("sendfile_call"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    if ( 0 == vscp_strcasecmp( attribute.c_str(), "false" ) ) {
                        m_web_allow_sendfile_call = false;
                    }
                    else {
                        m_web_allow_sendfile_call = true;
                    }
                }
            }

            if ( child->GetAttribute( ("additional_headers"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    m_web_additional_header = attribute;
                }
            }

            if ( child->GetAttribute( ("max_request_size"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    m_web_max_request_size = atol( (const char
*)attribute.c_str() );
                }
            }

            if ( child->GetAttribute( ("web_allow_index_script_resource"),
&attribute ) ) { vscp_trim( attribute ); if ( attribute.length() ) { if ( 0 ==
vscp_strcasecmp( attribute.c_str(), "false" ) ) {
                        m_web_allow_index_script_resource = false;
                    }
                    else {
                        m_web_allow_index_script_resource = true;
                    }
                }
            }

            if ( child->GetAttribute( ("duktape_script_patterns"), &attribute )
) { vscp_trim( attribute ); if ( attribute.length() ) {
                    m_web_duktape_script_patterns = attribute;
                }
            }

            if ( child->GetAttribute( ("lua_preload_file"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    m_web_lua_preload_file = attribute;
                }
            }

            if ( child->GetAttribute( ("lua_script_patterns"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    m_web_lua_script_patterns = attribute;
                }
            }

            if ( child->GetAttribute( ("lua_server_page_patterns"), &attribute )
) { vscp_trim( attribute ); if ( attribute.length() ) {
                    m_web_lua_server_page_patterns = attribute;
                }
            }

            if ( child->GetAttribute( ("lua_websockets_patterns"), &attribute )
) { attribute.Trim(); attribute.Trim(false); if ( attribute.length() ) {
                    m_web_lua_websocket_patterns = attribute;
                }
            }

            if ( child->GetAttribute( ("lua_background_script"), &attribute ) )
{ vscp_trim( attribute ); if ( attribute.length() ) {
                    m_web_lua_background_script = attribute;
                }
            }

            if ( child->GetAttribute( ("lua_background_script_params"),
&attribute ) ) { vscp_trim( attribute ); if ( attribute.length() ) {
                    m_web_lua_background_script_params = attribute;
                }
            }

        }

        else if ( 0 == vscp_strcasecmp( child->GetName().c_str(), "websockets" )
) {

            if ( child->GetAttribute( ("enable"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    if ( 0 == vscp_strcasecmp( attribute.c_str(), "false" ) ) {
                        m_bWebsocketsEnable = false;
                    }
                    else {
                        m_bWebsocketsEnable = true;
                    }
                }
            }

            if ( child->GetAttribute( ("document_root"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    m_websocket_document_root = attribute;
                }
            }

            if ( child->GetAttribute( ("timeout_ms"), &attribute ) ) {
                vscp_trim( attribute );
                if ( attribute.length() ) {
                    m_websocket_timeout_ms = atol( (const char
*)attribute.c_str() );
                }
            }

        }

        else if ( 0 == vscp_strcasecmp( child->GetName().c_str(), "remoteuser" )
) {

            wxXmlNode *subchild = child->GetChildren();
            while ( subchild ) {

                vscpEventFilter VSCPFilter;
                bool bFilterPresent = false;
                bool bMaskPresent = false;
                std::string name;
                std::string md5;
                std::string privilege;
                std::string allowfrom;
                std::string allowevent;
                bool bUser = false;

                vscp_clearVSCPFilter( &VSCPFilter ); // Allow all frames

                if ( 0 == vscp_strcasecmp( subchild->GetName().c_str(), "user" )
) {

                    wxXmlNode *subsubchild = subchild->GetChildren();

                    while (subsubchild) {

                        if ( 0 == vscp_strcasecmp(
subsubchild->GetName().c_str(), "name" ) ) { name =
subsubchild->GetNodeContent(); bUser = true;
                        }
                        else if ( 0 == vscp_strcasecmp(
subsubchild->GetName().c_str(), "password" ) ) { md5 =
subsubchild->GetNodeContent();
                        }
                        else if ( 0 == vscp_strcasecmp(
subsubchild->GetName().c_str(), "privilege" ) ) { privilege =
subsubchild->GetNodeContent();
                        }
                        else if ( 0 == vscp_strcasecmp(
subsubchild->GetName().c_str(), "filter" ) ) {

                            bFilterPresent = true;

                            std::string str_vscp_priority =
subchild->GetAttribute(("priority"), ("0") ); VSCPFilter.filter_priority =
str_vscp_type( str_vscp_priority );

                            std::string str_vscp_class =
subchild->GetAttribute(("class"), ("0")); VSCPFilter.filter_class =
str_vscp_type(str_vscp_class);

                            std::string str_vscp_type =
subchild->GetAttribute(("type"), ("0")); VSCPFilter.filter_type =
vscp_readStringValue(str_vscp_type);

                            std::string str_vscp_guid =
subchild->GetAttribute(("guid"),
                                                        ("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"));
                            vscp_getGuidFromStringToArray(VSCPFilter.filter_GUID,
str_vscp_guid);
                        }
                        else if ( 0 == vscp_strcasecmp(
subsubchild->GetName().c_str(), "mask" ) ) {

                            bMaskPresent = true;

                            std::string str_vscp_priority =
subchild->GetAttribute(("priority"), ("0")); VSCPFilter.mask_priority =
vscp_readStringValue( str_vscp_priority );

                            std::string str_vscp_class =
subchild->GetAttribute(("class"), ("0")); VSCPFilter.mask_class =
vscp_readStringValue( str_vscp_class );

                            std::string str_vscp_type =
subchild->GetAttribute(("type"), ("0")); VSCPFilter.mask_type =
vscp_readStringValue( str_vscp_type );

                            std::string str_vscp_guid =
subchild->GetAttribute(("guid"),
                                    ("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"));
                            vscp_getGuidFromStringToArray(VSCPFilter.mask_GUID,
str_vscp_guid);
                        }
                        else if ( 0 == vscp_strcasecmp(
subsubchild->GetName().c_str(), "allowfrom" ) ) { allowfrom =
subsubchild->GetNodeContent();
                        }
                        else if ( 0 == vscp_strcasecmp(
subsubchild->GetName().c_str(), "allowevent" ) ) { allowevent =
subsubchild->GetNodeContent();
                        }

                        subsubchild = subsubchild->GetNext();

                    }

                }

                // Add user
                if ( bUser ) {

                    if ( bFilterPresent && bMaskPresent ) {
                        m_userList.addUser( name,
                                                md5,
                                                "",
                                                "",
                                                &VSCPFilter,
                                                privilege,
                                                allowfrom,
                                                allowevent,
                                                VSCP_ADD_USER_FLAG_LOCAL );
                    }
                    else {
                        m_userList.addUser( name,
                                                md5,
                                                "",
                                                "",
                                                NULL,
                                                privilege,
                                                allowfrom,
                                                allowevent,
                                                VSCP_ADD_USER_FLAG_LOCAL );
                    }

                    bUser = false;
                    bFilterPresent = false;
                    bMaskPresent = false;

                }

                subchild = subchild->GetNext();

            }

        }

        // Level I driver
        else if ( ( 0 == vscp_strcasecmp( child->GetName().c_str(),
"canaldriver") ) || ( 0 == vscp_strcasecmp( child->GetName().c_str(),
"level1driver") ) ) {

            wxXmlNode *subchild = child->GetChildren();
            while ( subchild ) {

                std::string strName;
                std::string strConfig;
                std::string strPath;
                unsigned long flags;
                cguid guid;
                bool bEnabled = false;
                bool bCanalDriver = false;

                if ( 0 == vscp_strcasecmp( subchild->GetName().c_str(), "driver"
) ) {

                    wxXmlNode *subsubchild = subchild->GetChildren();
                    std::string attribute = subchild->GetAttribute(("enable"),
("false")); if ( 0 == vscp_strcasecmp( attribute.c_str(), "true" ) ) { bEnabled
= true;
                    }

                    while ( subsubchild ) {

                        if ( 0 == vscp_strcasecmp(
subsubchild->GetName().c_str(), "name" )  {

                            strName = subsubchild->GetNodeContent();
                            vscp_trim( strName );

                            // Replace spaces in name with underscore
                            std::string::size_type found;
                            while ( std::string::npos != ( found =
str.find_first_of(" ") ) ) {} str[found]='_';
                            }

                            bCanalDriver = true;    // This is a Level I driver

                        }
                        else if ( ( 0 == vscp_strcasecmp(
subsubchild->GetName().c_str(), "config" ) ) || (
subsubchild->GetName().c_str(), "parameter" ) )  {

                            strConfig = subsubchild->GetNodeContent();
                            vscp_trim( strConfig );
                        }
                        else if ( 0 == vscp_strcasecmp(
subsubchild->GetName().c_str(), "path" ) ) { strPath =
subsubchild->GetNodeContent(); vscp_trim( strConfig );
                        }
                        else if ( 0 == vscp_strcasecmp(
subsubchild->GetName().c_str(), "flags" ) ) { std::string str =
subsubchild->GetNodeContent(); flags = vscp_readStringValue(str);
                        }
                        else if ( 0 == vscp_strcasecmp(
subsubchild->GetName().c_str(), "guid" ) ) { guid.getFromString(
subsubchild->GetNodeContent() );
                        }
                        else if ( 0 == vscp_strcasecmp(
subsubchild->GetName().c_str(), "known-nodes" ) ) {

                            wxXmlNode *subsubsubchild = subchild->GetChildren();

                            while ( subsubsubchild ) {

                                if ( 0 == vscp_strcasecmp(
subsubsubchild->GetName().c_str(), "node" ) ) { cguid knownguid;

                                    knownguid.getFromString(
subchild->GetAttribute( "guid", "-" ) ); std::string name =
subchild->GetAttribute( "name", "" )  ) ; addKnownNode( knownguid, guid, name );
                                }

                                // Next driver item
                                subsubsubchild = subsubsubchild->GetNext();

                            } // while

                        }

                        // Next driver item
                        subsubchild = subsubchild->GetNext();

                    } // while

                }

                // Add the level I device
                if ( bCanalDriver && bEnabled ) {

                    if (!m_deviceList.addItem( strName,
                                                    strConfig,
                                                    strPath,
                                                    flags,
                                                    guid,
                                                    VSCP_DRIVER_LEVEL1,
                                                    bEnabled ) ) {
                        syslog( LOG_ERR,
                                    "Level I driver not added. "
                                             "Path does not exist. - [%s]",
                                    strPath.c_str() );
                    }
                    else {
                        syslog( LOG_DEBUG,
                                    "Level I driver added. - [%s]",
                                    strPath );
                    }

                    bCanalDriver = false;

                }

                // Next driver
                subchild = subchild->GetNext();

            }

        }

        // Level II driver
        else if ( ( 0 == vscp_strcasecmp( child->GetName().c_str(), "vscpdriver"
) ) || ( 0 == vscp_strcasecmp( child->GetName().c_str(), "level2driver" ) ) ) {

            wxXmlNode *subchild = child->GetChildren();

            while ( subchild ) {

                std::string strName;
                std::string strConfig;
                std::string strPath;
                cguid guid;
                bool bEnabled = false;
                bool bLevel2Driver = false;

                if ( 0 == vscp_strcasecmp( subchild->GetName().c_str(), "driver"
) ) {

                    wxXmlNode *subsubchild = subchild->GetChildren();
                    std::string attribute = subchild->GetAttribute( "enable",
                                                                    "false" );
                    if ( 0 == vscp_strcasecmp( attribute.c_str(), "true" ) ) {
                        bEnabled = true;
                    }

                    while ( subsubchild ) {

                        if ( 0 == vscp_strcasecmp(
subsubchild->GetName().c_str(), "name" ) ) {

                            strName = subsubchild->GetNodeContent();
                            vscp_Trim( strName );

                            // Replace spaces in name with underscore
                            int pos;
                            while ( strName.npos != ( pos = strName.find( " " )
) ) { strName[pos] = '_';
                            }

                            bLevel2Driver = true; // This is a Level II driver

                        }
                        else if ( ( 0 == vscp_strcasecmp(
subsubchild->GetName().c_str(), "config" ) || ( 0 == vscp_strcasecmp(
subsubchild->GetName().c_str(), "parameter" ) {

                            strConfig = subsubchild->GetNodeContent();
                            vscp_trim( strConfig );

                        }
                        else if ( 0 == vscp_strcasecmp(
subsubchild->GetName().c_str(), "path" ) {

                            strPath = subsubchild->GetNodeContent();
                            vscp_trim( strConfig );;

                        }
                        else if ( 0 == vscp_strcasecmp(
subsubchild->GetName().c_str(), "guid" ) ) { guid.getFromString(
subsubchild->GetNodeContent() );
                        }
                        else if ( 0 == vscp_strcasecmp(
subsubchild->GetName().c_str(), "known-nodes" ) ) {

                            wxXmlNode *subsubsubchild = subchild->GetChildren();

                            while ( subsubsubchild ) {

                                if ( 0 == vscp_strcasecmp(
subsubsubchild->GetName().c_str(), "node" ) ) { cguid knownguid;

                                    knownguid.getFromString(
subchild->GetAttribute( "guid",
                                                                                     "-" ) );
                                    std::string name =
                                            subchild->GetAttribute( ( "name" ),
( "" ) ); addKnownNode( knownguid, guid, name );
                                }

                                // Next driver item
                                subsubsubchild = subsubsubchild->GetNext();

                            } // while

                        }

                        // Next driver item
                        subsubchild = subsubchild->GetNext();

                    }

                }

                // Add the level II device
                if ( bLevel2Driver && bEnabled ) {

                    if ( !m_deviceList.addItem( strName,
                                                strConfig,
                                                strPath,
                                                0,
                                                guid,
                                                VSCP_DRIVER_LEVEL2,
                                                bEnabled ) ) {
                        syslog( LOG_ERR,
                                    "Level II driver was not added. "
                                    "Path does not exist. - [%s]",
                                    strPath.c_str() );

                    }
                    else {
                        syslog( LOG_DEBUG,
                                    "Level II driver added. - [%s]",
                                    strPath.c_str() );
                    }

                    bLevel2Driver = false;

                }

                // Next driver
                subchild = subchild->GetNext();

            }

        }
        // Level III driver
        else if ( 0 == vscp_strcasecmp( child->GetName().c_str(), "level3driver"
) ) {

            wxXmlNode *subchild = child->GetChildren();

            while ( subchild ) {

                std::string strName;
                std::string strConfig;
                std::string strPath;
                cguid guid;
                bool bEnabled = false;
                bool bLevel3Driver = false;

                if ( 0 == vscp_strcasecmp( subchild->GetName().c_str(), "driver"
) ) {

                    wxXmlNode *subsubchild = subchild->GetChildren();

                    std::string attribute = subchild->GetAttribute(("enable"),
                                                                ("false"));
                    if ( 0 == vscp_strcasecmp( attribute.c_str(), "true" ) ) {
                        bEnabled = true;
                    }

                    while ( subsubchild ) {

                        if ( 0 == vscp_strcasecmp(
subsubchild->GetName().c_str(), "name" ) ) {

                            strName = subsubchild->GetNodeContent();
                            vscp_trim( strName );

                            // Replace spaces in name with underscore
                            int pos;
                            while ( strName != ( pos = strName.find( " ") ) ) {
                                strName[pos] = '_' );
                            }

                            bLevel3Driver = true;

                        }
                        else if ( ( 0 == vscp_strcasecmp(
subsubchild->GetName().c_str(), "config" ) ) || ( 0 == vscp_strcasecmp(
subsubchild->GetName().c_str(), "parameter" ) ) ) {

                            strConfig = subsubchild->GetNodeContent();
                            vscp_trim( strConfig );

                        }
                        else if ( 0 == vscp_strcasecmp(
subsubchild->GetName()c_str(), "path" ) ) {

                            strPath = subsubchild->GetNodeContent();
                            vscp_trim( strPath );

                        }
                        else if ( 0 == vscp_strcasecmp(
subsubchild->GetName().c_str(), "guid" ) ) { guid.getFromString(
subsubchild->GetNodeContent() );
                        }
                        else if ( 0 == vscp_strcasecmp(
subsubchild->GetName().c_str(), "known-nodes" ) ) {

                            wxXmlNode *subsubsubchild = subchild->GetChildren();

                            while ( subsubsubchild ) {

                                if ( 0 == vscp_strcasecmp(
subsubsubchild->GetName().c_str(), "node" ) ) { cguid knownguid;

                                    knownguid.getFromString(
subchild->GetAttribute( ( "guid" ), ( "-" ) ) ); std::string name =
                                            subchild->GetAttribute( ( "name" ),
                                                                    ( "" ) );
                                    addKnownNode( knownguid, guid, name );
                                }

                                // Next driver item
                                subsubsubchild = subsubsubchild->GetNext();

                            } // while

                        }

                        // Next driver item
                        subsubchild = subsubchild->GetNext();

                    }

                }

                // Add the level II device
                if ( bLevel3Driver && bEnabled ) {

                    if (!m_deviceList.addItem( strName,
                                                strConfig,
                                                strPath,
                                                0,
                                                guid,
                                                VSCP_DRIVER_LEVEL3,
                                                bEnabled )) {
                        syslog( LOG_ERR,
                                    "Level III driver was not added. Path does
not exist. - [%s]", strPath.c_str() );

                    }
                    else {
                        syslog( LOG_DEBUG,
                                    "Level III driver added. - [%s]",
                                    strPath.c_str() );
                    }

                    bLevel3Driver = false;

                }

                // Next driver
                subchild = subchild->GetNext();

            }

        }
        // Tables
        else if ( 0 == vscp_strcasecmp( child->GetName().c_str(), "tables" ) ) {

            wxXmlNode *subchild = child->GetChildren();
            while ( subchild ) {

                if ( 0 == vscp_strcasecmp( subchild->GetName().c_str(), "table"
) ) {

                    // Check if enabled
                    std::string strEnabled = subchild->GetAttribute( ("enable"),
("true") ); if ( 0 == vscp_strcasecmp( strEnabled.c_str(), "false" ) ) { goto
xml_table_error;
                    }

                    // Get name of table
                    std::string name = subchild->GetAttribute( ("name"), ("")
);;

                    // Get type of table
                    vscpTableType type = VSCP_TABLE_DYNAMIC;
                    int size = 0;
                    std::string attribute = subchild->GetAttribute( ("type"),
("0") ); if ( 0 == vscp_strcasecmp( attribute.c_str(), "dynamic" ) ) { type =
VSCP_TABLE_DYNAMIC;
                    }
                    else if ( attribute.npos != attribute.find( ("static") ) ) {

                        type = VSCP_TABLE_STATIC;

                        // Get size
                        size = vscp_readStringValue( subchild->GetAttribute(
("size"), ("0") ) );

                    }
                    else {
                        // Invalid type
                        syslog( LOG_ERR, "Reading VSCP table xml info: Invalid
table type (static/dynamic)!" ); goto xml_table_error;
                    }

                    // Should it be created in memory?
                    bool bMemory = false;
                    std::string strInMemory = subchild->GetAttribute(
("bmemory"), ("false") ); if ( 0 == vscp_strcasecmp( strInMemory.c_str(), "true"
) ) { bMemory = true;
                    }

                    CVSCPTable *pTable = new CVSCPTable( m_rootFolder +
("table/"), name, true, bMemory, type, size ); if ( NULL != pTable ) {

                        if ( !pTable->setTableInfo( subchild->GetAttribute(
("owner"), ("admin") ), vscp_readStringValue( subchild->GetAttribute(
("rights"), ("0x700") ) ), subchild->GetAttribute( ("title"), ("") ),
                                                subchild->GetAttribute(
("labelx"), ("") ), subchild->GetAttribute( ("labely"), ("") ),
                                                subchild->GetAttribute(
("note"), ("") ), subchild->GetAttribute( ("sqlcreate"), ("") ),
                                                subchild->GetAttribute(
("sqlinsert"), ("") ), subchild->GetAttribute( ("sqldelete"), ("") ),
                                                subchild->GetAttribute(
("description"), ("") ) ) ) { syslog( LOG_ERR, "Reading table xml info: Could
not set table info!"); delete pTable; goto xml_table_error;
                        }

                        pTable->setTableEventInfo( vscp_readStringValue(
subchild->GetAttribute( ("vscpclass"), ("0") ) ), vscp_readStringValue(
subchild->GetAttribute( ("vscptype"), ("0") ) ), vscp_readStringValue(
subchild->GetAttribute( ("vscpsensorindex"), ("0") ) ), vscp_readStringValue(
subchild->GetAttribute( ("vscpunit"), ("0") ) ), vscp_readStringValue(
subchild->GetAttribute( ("vscpzone"), ("255") ) ), vscp_readStringValue(
subchild->GetAttribute( ("vscpsubzone"), ("255") ) )
                        );

                        if ( !m_userTableObjects.addTable( pTable ) ) {
                            delete pTable;
                            syslog( LOG_ERR, "Reading table xml info: Could not
add new table (name conflict?)!"));
                        };

                        // Add to database if not there already
                        //if ( !m_userTableObjects.isTableInDB( *pTable ) ) {
                        //    m_userTableObjects.addTableToDB( *pTable );
                        //
                        //else {
                        //    // Update
                        //    m_userTableObjects.updateTableToDB( *pTable );
                        //

                    }
                    else {
                        syslog( LOG_ERR, "Reading table xml info: Unable to
create table class!" );
                    }

                }

xml_table_error:

                subchild = subchild->GetNext();

            } // while

        }
        else if ( 0 == vscp_strcasecmp( child->GetName().c_str(), "automation" )
{

            std::string attribut = child->GetAttribute(("enable"), ("true"));
            if ( 0 == vscp_strcasecmp( attribut.c_str(), "false" ) ) {
                m_automation.disableAutomation();
            }
            else {
                m_automation.enableAutomation();
            }

            wxXmlNode *subchild = child->GetChildren();
            while (subchild) {

                if ( 0 == vscp_strcasecmp( subchild->GetName(), "zone" )  ) {

                    long zone;
                    std::string strZone = subchild->GetNodeContent();
                    strZone.ToLong( &zone );
                    m_automation.setZone( zone );

                }
                else if ( 0 == vscp_strcasecmp( subchild->GetName().c_str(),
"sub-zone") ) {

                    long subzone;
                    std::string strSubZone = subchild->GetNodeContent();
                    strSubZone.ToLong( &subzone );
                    m_automation.setSubzone( subzone );

                }
                else if ( 0 == vscp_strcasecmp( subchild->GetName().c_str(),
"longitude") ) {

                    std::string strLongitude = subchild->GetNodeContent();
                    double d = stod(strLongitude);
                    m_automation.setLongitude( d );

                }
                else if ( 0 == vscp_strcasecmp( subchild->GetName().c_str(),
"latitude" ) ) {

                    std::string strLatitude = subchild->GetNodeContent();
                    double d = stod(strLatitude);
                    m_automation.setLatitude( d );

                }
                else if ( 0 == vscp_strcasecmp( subchild->GetName().c_str(),
"sunrise" ) ) {

                    m_automation.enableSunRiseEvent();
                    std::string attribute = subchild->GetAttribute(("enable"),
("true")); if ( 0 == vscp_strcasecmp( attribute.c_str(), "false" ) ) {
                        m_automation.disableSunRiseEvent();
                    }


                }

                else if ( 0 == vscp_strcasecmp( subchild->GetName().c_str(),
"sunrise-twilight" ) ) {

                    m_automation.enableSunRiseTwilightEvent();
                    std::string attribute = subchild->GetAttribute(("enable"),
("true")); if ( 0 == vscp_strcasecmp( attribute.c_str(), "false" ) ) {
                        m_automation.disableSunRiseTwilightEvent();
                    }


                }
                else if ( 0 == vscp_strcasecmp( subchild->GetName().c_str(),
"sunset" ) ) {

                    m_automation.enableSunSetEvent();
                    std::string attribute = subchild->GetAttribute(("enable"),
("true")); if ( 0 == vscp_strcasecmp( attribute.c_str(), "false" ) ) {
                        m_automation.disableSunSetEvent();
                    }

                }
                else if ( 0 == vscp_strcasecmp( subchild->GetName().c_str(),
"sunset-twilight" ) ) {

                    m_automation.enableSunSetTwilightEvent();
                    std::string attribute = subchild->GetAttribute(("enable"),
("true")); if ( 0 == vscp_strcasecmp( attribute.c_str(), "false" ) ) {
                        m_automation.disableSunSetTwilightEvent();
                    }

                }
                else if ( 0 == vscp_strcasecmp( subchild->GetName().c_str(),
"segmentcontrol-event" ) ) {

                    m_automation.enableSegmentControllerHeartbeat();
                    std::string attribute = subchild->GetAttribute(("enable"),
("true")); if ( 0 == vscp_strcasecmp( attribute.c_str(), "false" ) ) {
                        m_automation.disableSegmentControllerHeartbeat();
                    }

                    attribute = subchild->GetAttribute(("interval"), ("60"));
                    long interval = vscp_readStringValue( attribute );
                    m_automation.setSegmentControllerHeartbeatInterval( interval
);

                }
                else if ( 0 == vscp_strcasecmp( subchild->GetName().c_str(),
"heartbeat-event" ) ) {

                    m_automation.enableHeartbeatEvent();
                    m_automation.setHeartbeatEventInterval( 30 );

                    std::string attribute = subchild->GetAttribute(("enable"),
("true")); if ( 0 == vscp_strcasecmp( attribute.c_str(), "false" ) ) {
                        m_automation.disableHeartbeatEvent();
                    }

                    attribute = subchild->GetAttribute(("interval"), ("30"));
                    long interval = vscp_readStringValue( attribute );
                    m_automation.setHeartbeatEventInterval( interval );

                }
                else if ( 0 == vscp_strcasecmp( subchild->GetName().c_str(),
"capabilities-event" ) ) {

                    m_automation.enableCapabilitiesEvent();
                    m_automation.setCapabilitiesEventInterval( 60 );

                    std::string attribute = subchild->GetAttribute(("enable"),
("true")); if ( 0 == vscp_strcasecmp( attribute.c_str(), "false" ) ) {
                        m_automation.disableCapabilitiesEvent();
                    }

                    attribute = subchild->GetAttribute(("interval"), ("30"));
                    long interval = vscp_readStringValue( attribute );
                    m_automation.setHeartbeatEventInterval( interval );

                }

                subchild = subchild->GetNext();

            } // while

        }

        child = child->GetNext();

    }
*/
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
    sql = vscp_string_format(sql, (const char *)strTblName.c_str());

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
    sql             = vscp_string_format(sql, (const char *)strTblName.c_str());

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
                                    VSCPDB_CONFIG_DEFAULT_DBVERSION);
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
    addConfigurationValueToDatabase(VSCPDB_CONFIG_NAME_PATH_DB_DATA,
                                    VSCPDB_CONFIG_DEFAULT_PATH_DB_DATA);

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

    // Check if database is open
    if (NULL == m_db_vscp_daemon) {
        syslog(LOG_ERR,
               "dbReadConfiguration: Failed to read VSCP settings database "
               "- Database is not open.");
        return false;
    }

    if (SQLITE_OK !=
        sqlite3_prepare(m_db_vscp_daemon, psql, -1, &ppStmt, NULL)) {
        syslog(LOG_ERR,
               "dbReadConfiguration: Failed to read VSCP settings database "
               "- prepare query.");
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
            pthread_mutex_lock(&m_udpSrvObj->m_mutexUDPInfo);
            m_udpSrvObj->m_bEnable = atoi((const char *)pValue) ? true : false;
            pthread_mutex_unlock(&m_udpSrvObj->m_mutexUDPInfo);
            continue;
        }

        // UDP interface address/port
        if (0 ==
            vscp_strcasecmp((const char *)pName, VSCPDB_CONFIG_NAME_UDP_ADDR)) {
            pthread_mutex_lock(&m_udpSrvObj->m_mutexUDPInfo);
            m_udpSrvObj->m_interface = std::string((const char *)pValue);
            pthread_mutex_unlock(&m_udpSrvObj->m_mutexUDPInfo);
            continue;
        }

        // UDP User
        if (0 ==
            vscp_strcasecmp((const char *)pName, VSCPDB_CONFIG_NAME_UDP_USER)) {
            pthread_mutex_lock(&m_udpSrvObj->m_mutexUDPInfo);
            m_udpSrvObj->m_user = std::string((const char *)pValue);
            pthread_mutex_unlock(&m_udpSrvObj->m_mutexUDPInfo);
            continue;
        }

        // UDP User Password
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_UDP_PASSWORD)) {
            pthread_mutex_lock(&m_udpSrvObj->m_mutexUDPInfo);
            m_udpSrvObj->m_password = std::string((const char *)pValue);
            pthread_mutex_unlock(&m_udpSrvObj->m_mutexUDPInfo);
            continue;
        }

        // UDP un-secure enable
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_UDP_UNSECURE_ENABLE)) {
            pthread_mutex_lock(&m_udpSrvObj->m_mutexUDPInfo);
            m_udpSrvObj->m_bAllowUnsecure =
              atoi((const char *)pValue) ? true : false;
            pthread_mutex_unlock(&m_udpSrvObj->m_mutexUDPInfo);
            continue;
        }

        // UDP Filter
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_UDP_FILTER)) {
            pthread_mutex_lock(&m_udpSrvObj->m_mutexUDPInfo);
            vscp_readFilterFromString(&m_udpSrvObj->m_filter,
                                      std::string((const char *)pValue));
            pthread_mutex_unlock(&m_udpSrvObj->m_mutexUDPInfo);
            continue;
        }

        // UDP Mask
        if (0 ==
            vscp_strcasecmp((const char *)pName, VSCPDB_CONFIG_NAME_UDP_MASK)) {
            pthread_mutex_lock(&m_udpSrvObj->m_mutexUDPInfo);
            vscp_readMaskFromString(&m_udpSrvObj->m_filter,
                                    std::string((const char *)pValue));
            pthread_mutex_unlock(&m_udpSrvObj->m_mutexUDPInfo);
            continue;
        }

        // UDP GUID
        if (0 ==
            vscp_strcasecmp((const char *)pName, VSCPDB_CONFIG_NAME_UDP_GUID)) {
            pthread_mutex_lock(&m_udpSrvObj->m_mutexUDPInfo);
            m_udpSrvObj->m_guid.getFromString((const char *)pValue);
            pthread_mutex_unlock(&m_udpSrvObj->m_mutexUDPInfo);
            continue;
        }

        // UDP Enable ACK
        if (0 == vscp_strcasecmp((const char *)pName,
                                 VSCPDB_CONFIG_NAME_UDP_ACK_ENABLE)) {
            pthread_mutex_lock(&m_udpSrvObj->m_mutexUDPInfo);
            m_udpSrvObj->m_bAck = atoi((const char *)pValue) ? true : false;
            pthread_mutex_unlock(&m_udpSrvObj->m_mutexUDPInfo);
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
                                 VSCPDB_CONFIG_NAME_PATH_DB_DATA)) {
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
    if (!m_udpSrvObj->m_bEnable) return true;

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

        pthread_mutex_lock(&m_udpSrvObj->m_mutexUDPInfo);

        udpRemoteClientInfo *pudpClient = new udpRemoteClientInfo;
        if (NULL == pudpClient) {
            syslog(LOG_ERR,
                   "readUdpNodes: Failed to allocate storage for UDP node.");
            pthread_mutex_unlock(&m_udpSrvObj->m_mutexUDPInfo);
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
        m_udpSrvObj->m_remotes.push_back(pudpClient);

        pthread_mutex_unlock(&m_udpSrvObj->m_mutexUDPInfo);
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
// doCreateLogTable
//
// Create the log database
//
//

bool
CControlObject::doCreateLogTable(void)
{
    char *pErrMsg    = 0;
    const char *psql = VSCPDB_LOG_CREATE;

    syslog(LOG_INFO, "Creating VSCP log database.");

    // Check if database is open
    if (NULL == m_db_vscp_log) {
        syslog(LOG_ERR, "Failed to create VSCP log database - closed.");
        return false;
    }

    pthread_mutex_lock(&m_db_vscp_configMutex);

    if (SQLITE_OK != sqlite3_exec(m_db_vscp_log, psql, NULL, NULL, &pErrMsg)) {
        syslog(
          LOG_ERR, "Failed to create VSCP log table with error %s.", pErrMsg);
        pthread_mutex_unlock(&m_db_vscp_configMutex);
        return false;
    }

    pthread_mutex_unlock(&m_db_vscp_configMutex);

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
        sql += vscp_string_format(("INSERT INTO 'zone' (idx_zone, name) "
                                   "VALUES( %d, 'zone%d' );"),
                                  i,
                                  i);
    }

    sql += vscp_string_format(VSCPDB_ZONE_UPDATE,
                              "All zones",
                              "Zone = 255 represents all zones.",
                              255L);
    sql += "COMMIT;";
    if (SQLITE_OK !=
        sqlite3_exec(
          m_db_vscp_daemon, (const char *)sql.c_str(), NULL, NULL, &pErrMsg)) {
        syslog(
          LOG_ERR,
          "Failed to insert last VSCP default zone table entry %d. Error %s",
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
        sql += vscp_string_format(("INSERT INTO 'subzone' (idx_subzone, name) "
                                   "VALUES( %d, 'subzone%d' );"),
                                  i,
                                  i);
    }

    sql +=
      vscp_string_format(VSCPDB_SUBZONE_UPDATE,
                         "All subzones",
                         "Subzone = 255 represents all subzones of a zone.",
                         255L);
    sql += "COMMIT;";
    if (SQLITE_OK !=
        sqlite3_exec(
          m_db_vscp_daemon, (const char *)sql.c_str(), NULL, NULL, &pErrMsg)) {
        syslog(
          LOG_ERR,
          "Failed to insert last VSCP default subzone table entry %d. Error %s",
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
        struct timespec ts;
        ts.tv_sec  = 0;
        ts.tv_nsec = 500000; // 500 ms
        if (ETIMEDOUT == sem_timedwait(&pObj->m_semClientOutputQueue, &ts)) {
            continue;
        }
        if (pObj->m_clientOutputQueue.size()) {

            pthread_mutex_lock(&pObj->m_mutexClientOutputQueue);
            pvscpEvent = pObj->m_clientOutputQueue.front();
            pObj->m_clientOutputQueue.pop_front();
            // pvscpEvent = *it;
            pthread_mutex_unlock(&pObj->m_mutexClientOutputQueue);

            if (NULL != pvscpEvent) {

                // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
                // * * * * *
                // * * * * Send event to all Level II clients (not to ourself )
                // * * * *
                // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

                pObj->sendEventAllClients(pvscpEvent, pvscpEvent->obid);

            } // Valid event

            // Delete the event
            if (NULL != pvscpEvent) vscp_deleteVSCPevent(pvscpEvent);
            pvscpEvent = NULL;

        } // while

    } // while

    return NULL;
}
