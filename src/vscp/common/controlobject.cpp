// ControlObject.cpp: m_path_db_vscp_logimplementation of the CControlObject class.
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
// 
// Copyright (c) 2000-2018 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
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
// TraceMasks:
// ===========
//
//   wxTRACE_doWorkLoop - Workloop messages
//   wxTRACE_vscpd_Msg - Received messages.
//   wxTRACE_vscpd_ReceiveMutex  - Mutex lock
//   wxTRACE_VSCP_Msg - VSCP message mechanism
//

#if defined(_WIN32)
#include <winsock2.h>
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"
#include <wx/xml/xml.h>

#if defined(_WIN32)

#include <winsock2.h>
#include "canal_win32_ipc.h"

#else   // UNIX

#define _POSIX
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <syslog.h>
#include <sys/msg.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <string.h>
#include <netdb.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pwd.h>

#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/log.h"
#include "wx/socket.h"

#endif

#include <wx/config.h>
#include <wx/wfstream.h>
#include <wx/fileconf.h>
#include <wx/tokenzr.h>
#include <wx/list.h>
#include <wx/xml/xml.h>
#include <wx/mimetype.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>

#include "web_css.h"
#include "web_js.h"
#include "web_template.h"

#include <sqlite3.h>

#include <vscp_aes.h>
#include <fastpbkdf2.h>

#include <vscp_debug.h>
#include <canal_macro.h>
#include <vscp.h>
#include <vscphelper.h>
#include <vscpeventhelper.h>
#include <tables.h>
#include <configfile.h>
#include <crc.h>
#include <vscpmd5.h>
#include <randpassword.h>
#include <version.h>
#include <vscpdb.h>
#include <variablecodes.h>
#include <actioncodes.h>
#include <devicelist.h>
#include <devicethread.h>
#include <dm.h>
#include <httpd.h>
#include <websrv.h>
#include <websocket.h>
#include <vscpd_caps.h>
#include <controlobject.h>


#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif


// Lists
WX_DEFINE_LIST(WEBSOCKETSESSIONLIST);   // websocket sessions
WX_DEFINE_LIST(RESTSESSIONLIST);        // web server sessions
WX_DEFINE_LIST(WEBSRVSESSIONLIST);      // web server sessions
WX_DEFINE_LIST(TRIGGERLIST);            // websocket trigger
WX_DEFINE_LIST(CanalMsgList);           // List for CANAL messages
WX_DEFINE_LIST(VSCPEventList);          // List for VSCP events

//#define DEBUGPRINT

extern CControlObject *gpobj;

#if defined(_WIN32)

typedef struct _ASTAT_ {
    ADAPTER_STATUS adapt;
    NAME_BUFFER NameBuff [30];
} ASTAT, * PASTAT;

ASTAT Adapter;


WORD wVersionRequested = MAKEWORD(1, 1);    // WSA functions
WSADATA wsaData;                            // WSA functions

#endif

// Forward declares


// Prototypes


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CControlObject::CControlObject()
{
    int i;
    m_bQuit = false;            // true if we should quit

    // Debug flags
    //m_debugFlags1 = VSCP_DEBUG1_ALL;
    m_debugFlags1 = 0;
    //m_debugFlags1 |= VSCP_DEBUG1_DM;
    //m_debugFlags1 |= VSCP_DEBUG1_AUTOMATION;
    //m_debugFlags1 |= VSCP_DEBUG1_VARIABLE;
    //m_debugFlags1 |= VSCP_DEBUG1_MULTICAST;
    //m_debugFlags1 |= VSCP_DEBUG1_UDP;
    //m_debugFlags1 |= VSCP_DEBUG1_TCP;
    //m_debugFlags1 |= VSCP_DEBUG1_DRIVER
    
    m_logDays = DEFAULT_LOGDAYS;

#if defined(_WIN32)
    m_rootFolder = wxStandardPaths::Get().GetUserDataDir();
#else
    m_rootFolder = _("/srv/vscp/");
#endif

    // Delete objects from list when they are removed
    m_websocketSessions.DeleteContents( true );

    // Default admin user credentials
    m_admin_user = _("admin");
    m_admin_password = _("450ADCE88F2FDBB20F3318B65E53CA4A;"
                         "06D3311CC2195E80BE4F8EB12931BFEB5C"
                         "630F6B154B2D644ABE29CEBDBFB545");
    m_admin_allowfrom = _("*");
    m_vscptoken = _("Carpe diem quam minimum credula postero");
    vscp_hexStr2ByteArray( m_systemKey,
                                32,
                                "A4A86F7D7E119BA3F0CD06881E371B989B"
                                "33B6D606A863B633EF529D64544F8E" );

    m_nConfiguration = 1;       // Default configuration record is read.

    // Log to syslog
    m_bLogToSysLog = true;

    m_automation.setControlObject( this );
    m_maxItemsInClientReceiveQueue = MAX_ITEMS_CLIENT_RECEIVE_QUEUE;

    // Nill the GUID
    m_guid.clear();

    // Initialize the client map
    // to all unused
    for (i = 0; i < VSCP_MAX_CLIENTS; i++) {
        m_clientMap[ i ] = 0;
    }

    // Local domain
    m_web_authentication_domain = _( "mydomain.com" );

    // Set Default Log Level
    m_logLevel = DAEMON_LOGMSG_NORMAL;


// NOTE!!!  Dependent on the root folder
// Therefore this statement (currently) has no effect.
#ifdef WIN32
    m_path_db_vscp_daemon.Assign( wxStandardPaths::Get().GetConfigDir() +
                                            _("/vscp/vscpd.sqlite3") );
#else
    m_path_db_vscp_daemon.Assign( _("/srv/vscp/vscpd.sqlite3") );
#endif

#ifdef WIN32
    m_path_db_vscp_data.Assign( wxStandardPaths::Get().GetConfigDir() +
                                            _("/vscp/vscp-data.sqlite3") );
#else
    m_path_db_vscp_data.Assign( _("/srv/vscp/vscp_data.sqlite3") );
#endif


#ifdef WIN32
    m_path_db_vscp_log.Assign( wxStandardPaths::Get().GetConfigDir() +
                                            _("/vscp/vscp_log.sqlite3") );
#else
    m_path_db_vscp_log.Assign( m_rootFolder +_("/logs/vscpd_log.sqlite3") );
#endif


    // No databases opened yet
    m_db_vscp_daemon = NULL;
    m_db_vscp_data = NULL;
    m_db_vscp_log = NULL;

    // Control UDP Interface
    m_udpInfo.m_bEnable = false;
    m_udpInfo.m_interface.Empty();
    m_udpInfo.m_guid.clear();
    vscp_clearVSCPFilter( &m_udpInfo.m_filter );
    m_udpInfo.m_bAllowUnsecure = false;
    m_udpInfo.m_bAck = false;

    // Default TCP/IP interface settings
    m_StopTcpIpSrv = 0;
    m_enableTcpip = true;
    m_strTcpInterfaceAddress = _("9598");
    m_encryptionTcpip = 0;
    m_pTCPListenThread = NULL;
    m_tcpip_ssl_certificate.Empty();
    m_tcpip_ssl_certificate_chain.Empty();
    m_tcpip_ssl_verify_peer = 0;    // no=0, optional=1, yes=2
    m_tcpip_ssl_ca_path.Empty();
    m_tcpip_ssl_ca_file.Empty();
    m_tcpip_ssl_verify_depth = 9;
    m_tcpip_ssl_default_verify_paths = false;
    m_tcpip_ssl_cipher_list.Empty();
    m_tcpip_ssl_protocol_version = 0;
    m_tcpip_ssl_short_trust = false;

    // Default multicast announce port
    m_strMulticastAnnounceAddress = _( "udp://:" + VSCP_ANNOUNCE_MULTICAST_PORT );

    // default multicast announce ttl
    m_ttlMultiCastAnnounce = IP_MULTICAST_DEFAULT_TTL;

    // Default UDP interface
    m_udpInfo.m_interface = _("udp://:" + VSCP_DEFAULT_UDP_PORT );

    m_pclientMsgWorkerThread = NULL;
    m_pTCPListenThread = NULL;
    m_pdaemonVSCPThread = NULL;

    // Web server SSL settings
    m_web_ssl_certificate = _("/srv/vscp/certs/server.pem");
    m_web_ssl_certificate_chain = _("");
    m_web_ssl_verify_peer = false;
    m_web_ssl_ca_path = _("");
    m_web_ssl_ca_file = _("");
    m_web_ssl_verify_depth = 9;
    m_web_ssl_default_verify_paths = true;
    m_web_ssl_cipher_list = _("DES-CBC3-SHA:AES128-SHA:AES128-GCM-SHA256");
    m_web_ssl_protocol_version = 3;
    m_web_ssl_short_trust = false;


    // Webserver interface
    m_web_bEnable = true;
    m_web_listening_ports = _("[::]:8888r,[::]:8843s,8884");

    m_web_index_files = _("index.xhtml,index.html,index.htm,"
                          "index.lp,index.lsp,index.lua,index.cgi,"
                          "index.shtml,index.php");

#ifdef WIN32
    m_web_document_root = _("/programdata/vscp/www");
#else
    m_web_document_root = _("/srv/vscp/www");
#endif

    // Directory listings on by default
    m_web_enable_directory_listing = true;
    m_web_enable_keep_alive = false;
    m_web_keep_alive_timeout_ms = 0;
    m_web_access_control_list = _("");
    m_web_extra_mime_types = _("");
    m_web_num_threads=50;
    m_web_run_as_user = _("");
    m_web_url_rewrite_patterns = _("");
    m_web_hide_file_patterns = _("");
    m_web_global_auth_file = _("");
    m_web_per_directory_auth_file = _("");
    m_web_ssi_patterns = _("");
    m_web_url_rewrite_patterns = _("");
    m_web_request_timeout_ms = 10000;
    m_web_linger_timeout_ms = -1; // Do not set
    m_web_decode_url = true;
    m_web_ssi_patterns = _("");
    m_web_access_control_allow_origin = _("*"); 
    m_web_access_control_allow_methods = _("*");
    m_web_access_control_allow_headers = _("*");
    m_web_error_pages = _("");
    m_web_tcp_nodelay = 0;
    m_web_static_file_max_age = 3600;
    m_web_strict_transport_security_max_age = -1;
    m_web_allow_sendfile_call = true;
    m_web_additional_header = _("");
    m_web_max_request_size = 16384;
    m_web_allow_index_script_resource = false;
    m_web_duktape_script_patterns = _("**.ssjs$");
    m_web_lua_preload_file = _("");
    m_web_lua_script_patterns = _("**.lua$");
    m_web_lua_server_page_patterns = _("**.lp$|**.lsp$");
    m_web_lua_websocket_patterns = _("**.lua$");
    m_web_lua_background_script = _("");
    m_web_lua_background_script_params = _("");

    // Init. web server subsystem - All features enabled
    // ssl mt locks will we initiated here for openssl 1.0
    if ( 0 == web_init( 0xffff ) ) {
        fprintf(stderr,"Failed to initialize webserver subsystem.\n" );
    }

#ifdef WIN32

    // Initialize winsock layer
    WSAStartup(wVersionRequested, &wsaData);

    // Also for wx
    wxSocketBase::Initialize();

#ifdef BUILD_VSCPD_SERVICE
    if (!m_hEventSource) {
        m_hEvntSource = ::RegisterEventSource(NULL, // local machine
                _("vscpservice")); // source name
    }

#endif // windows service

#endif

    // Initialize the CRC
    crcInit();
    
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CControlObject::~CControlObject()
{
    fprintf( stderr, "ControlObject: Going away...\n");
    
    // Remove objects in Client send queue
    VSCPEventList::iterator iterVSCP;

    m_mutexClientOutputQueue.Lock();
    for (iterVSCP = m_clientOutputQueue.begin();
            iterVSCP != m_clientOutputQueue.end(); ++iterVSCP) {
        vscpEvent *pEvent = *iterVSCP;
        vscp_deleteVSCPevent( pEvent );
    }

    m_clientOutputQueue.Clear();
    m_mutexClientOutputQueue.Unlock();


    gpobj->m_mutexUDPInfo.Lock();
    udpRemoteClientList::iterator iterUDP;
    for (iterUDP = m_udpInfo.m_remotes.begin();
            iterUDP != m_udpInfo.m_remotes.end(); ++iterUDP) {
        if ( NULL != *iterUDP ) {
            delete *iterUDP;
            *iterUDP = NULL;
        }

    }
    m_udpInfo.m_remotes.Clear();
    m_mutexUDPInfo.Unlock();
    
    fprintf( stderr, "ControlObject: Gone!\n");

}

/////////////////////////////////////////////////////////////////////////////
// init
//

bool CControlObject::init( wxString& strcfgfile, wxString& rootFolder )
{
    wxString str;

    //wxLog::AddTraceMask( "wxTRACE_doWorkLoop" );
    //wxLog::AddTraceMask(_("wxTRACE_vscpd_receiveQueue")); // Receive queue
    //wxLog::AddTraceMask(_("wxTRACE_vscpd_Msg"));
    //wxLog::AddTraceMask(_("wxTRACE_VSCP_Msg"));
    //wxLog::AddTraceMask(_("wxTRACE_vscpd_ReceiveMutex"));
    //wxLog::AddTraceMask(_("wxTRACE_vscpd_sendMutexLevel1"));
    //wxLog::AddTraceMask(_("wxTRACE_vscpd_LevelII"));
    //wxLog::AddTraceMask( _( "wxTRACE_vscpd_dm" ) );
    
    // Save root folder for later use.
    m_rootFolder = rootFolder;

    // Root folder must exist
    if ( !wxFileName::DirExists( m_rootFolder ) ) {
        fprintf(stderr,"The specified rootfolder does not exist (%s).\n",
                (const char *)m_rootFolder.mbc_str() );
        return false;
    }

    m_path_db_vscp_daemon.Assign( m_rootFolder + _("vscpd.sqlite3") );
    m_path_db_vscp_data.Assign( m_rootFolder + _("vscp_data.sqlite3") );
    m_path_db_vscp_log.Assign( m_rootFolder + _("/logs/vscpd_log.sqlite3") );
    wxString strRootwww = m_rootFolder + _("www");
    m_web_document_root = strRootwww;

    // Change locale to get the correct decimal point "." 
    // Set locale
    int sys_lang = wxLocale::GetSystemLanguage();
    if ( sys_lang != wxLANGUAGE_DEFAULT ) {
        m_locale.Init( sys_lang );              // set custom locale
        m_locale.AddCatalogLookupPathPrefix( _("locale") );   // set "locale" prefix
        m_locale.AddCatalog( _("wxproton") );   // our private domain
        m_locale.AddCatalog( _("wxstd") );      // wx common domain is default
                                                // Restore "C" numeric locale
        setlocale( LC_NUMERIC, "C" );
    }


    // A configuration file must be available
    if ( !wxFile::Exists( strcfgfile ) ) {
        printf("No configuration file. Can't initialize!.");
        fprintf( stderr, "No configuration file. Can't initialize!.\n" );
        str = _("Path = .") + strcfgfile + _("\n");
        fprintf( stderr, "%s", (const char *)str.mbc_str() );
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////
    //                  Read XML configuration GENERAL section
    ////////////////////////////////////////////////////////////////////////////


    // Read XML configuration
    if ( !readXMLConfigurationGeneral( strcfgfile ) ) {
        fprintf( stderr, "General: Unable to open/parse configuration file. Can't initialize!\n" );
        str = _("Path = .") + strcfgfile + _("\n");
        fprintf( stderr, "%s", (const char *)str.mbc_str() );
        return FALSE;
    }



#ifndef WIN32
    if ( m_runAsUser.Length() ) {
        struct passwd *pw;
        if ( NULL == ( pw = getpwnam( m_runAsUser.mbc_str() ) ) ) {
            fprintf( stderr, "Unknown user.\n" );
        }
        else if (setgid(pw->pw_gid) != 0) {
            fprintf( stderr, "setgid() failed.\n" );
        }
        else if (setuid(pw->pw_uid) != 0) {
            fprintf( stderr, "setuid() failed.\n" );
        }
    }
#endif

    // Initialize the SQLite library
    if ( SQLITE_OK != sqlite3_initialize() ) {
        fprintf( stderr, "Unable to initialize SQLite library!." );
        return false;
    }


    // The root folder is the basis for the configuration file
    m_path_db_vscp_daemon.Assign( m_rootFolder + _("/vscpd.sqlite3") );



    // ======================================
    // * * * Open/Create database files * * *
    // ======================================



    // * * * VSCP Daemon configuration database * * *

    // Check filename
    if ( m_path_db_vscp_daemon.IsOk() && m_path_db_vscp_daemon.FileExists() ) {

        if ( SQLITE_OK != sqlite3_open( (const char *)m_path_db_vscp_daemon.GetFullPath().mbc_str(),
                                            &m_db_vscp_daemon ) ) {

            // Failed to open/create the database file
            fprintf( stderr, "VSCP Daemon configuration database could not be opened. - Will exit.\n" );
            str.Printf( _("Path=%s error=%s\n"),
                            (const char *)m_path_db_vscp_daemon.GetFullPath().mbc_str(),
                            sqlite3_errmsg( m_db_vscp_daemon ) );
            fprintf( stderr, "%s", (const char *)str.mbc_str() );
            if ( NULL != m_db_vscp_daemon ) sqlite3_close( m_db_vscp_daemon );
            m_db_vscp_daemon = NULL;
            return false;

        }
        else {
            
            // Database is open. 
                        
            // Add possible missing configuration values
            addDefaultConfigValues();
            
            // Read configuration data
            readConfigurationDB();
            
        }

    }
    else {

        if ( m_path_db_vscp_daemon.IsOk() ) {

            // We need to create the database from scratch. This may not work if
            // the database is in a read only location.
            fprintf( stderr, "VSCP Daemon configuration database does not exist - will be created.\n" );
            str.Printf(_("Path=%s\n"), (const char *)m_path_db_vscp_daemon.GetFullPath().mbc_str() );
            fprintf( stderr, "%s", (const char *)str.mbc_str() );

            if ( SQLITE_OK == sqlite3_open( (const char *)m_path_db_vscp_daemon.GetFullPath().mbc_str(),
                                             &m_db_vscp_daemon ) ) {

                // create the configuration database.
                if ( !doCreateConfigurationTable() ) {
                    fprintf( stderr, "Failed to create configuration table.\n" );
                }

                // Create the UDP node database
                if ( !doCreateUdpNodeTable() ) {
                    fprintf( stderr, "Failed to create udpnode table.\n" );
                }

                // Create the multicast database
                if ( !doCreateMulticastTable() ) {
                    fprintf( stderr, "Failed to create multicast table.\n" );
                }

                // Create user table
                if ( !doCreateUserTable() ) {
                    fprintf( stderr, "Failed to create user table.\n" );
                }

                // Create driver table
                if ( !doCreateDriverTable() ) {
                    fprintf( stderr, "Failed to create driver table.\n" );
                }

                // Create guid table
                if ( !doCreateGuidTable() ) {
                    fprintf( stderr, "Failed to create GUID table.\n" );
                }

                // Create location table
                if ( !doCreateLocationTable() ) {
                    fprintf( stderr, "Failed to create location table.\n" );
                }

                // Create mdf table
                if ( !doCreateMdfCacheTable() ) {
                    fprintf( stderr, "Failed to create MDF cache table.\n" );
                }

                // Create simpleui table
                if ( !doCreateSimpleUiTable() ) {
                    fprintf( stderr, "Failed to create Simple UI table.\n" );
                }

                // Create simpleui item table
                if ( !doCreateSimpleUiItemTable() ) {
                    fprintf( stderr, "Failed to create Simple UI item table.\n" );
                }

                // Create zone table
                if ( !doCreateZoneTable() ) {
                    fprintf( stderr, "Failed to create zone table.\n" );
                }

                // Create subzone table
                if ( !doCreateSubZoneTable() ) {
                    fprintf( stderr, "Failed to create sub zone table.\n" );
                }

                // Create userdef table
                if ( !doCreateUserdefTableTable() ) {
                    fprintf( stderr, "Failed to create user defined table.\n" );
                }

                // * * * All created * * *

                // Database is open. Read configuration data from it
                if ( !readConfigurationDB() ) {
                    fprintf( stderr, "Failed to read configuration from "
                                     "configuration database.\n" );
                }

            }

        }
        else {
            fprintf( stderr, "VSCP Server configuration database path invalid - will exit.\n" );
            str.Printf(_("Path=%s\n"),(const char *)m_path_db_vscp_daemon.GetFullPath().mbc_str() );
            fprintf( stderr, "%s", (const char *)str.mbc_str() );
            return false;
        }

    }

    // Read UDP nodes
    readUdpNodes();

    // Read multicast channels
    readMulticastChannels();


    // * * * VSCP Daemon logging database * * *


    // Check filename
    if ( m_path_db_vscp_log.IsOk() && m_path_db_vscp_log.FileExists() ) {

        if ( SQLITE_OK != 
                sqlite3_open( (const char *)m_path_db_vscp_log.GetFullPath().mbc_str(),
                                            &m_db_vscp_log ) ) {

            // Failed to open/create the database file
            fprintf( stderr, 
                      "VSCP Server logging database could not be "
                      "opened. - Will not be used.\n" );
            str.Printf( _("Path=%s error=%s\n"),
                            (const char *)m_path_db_vscp_log.GetFullPath().mbc_str(),
                            sqlite3_errmsg( m_db_vscp_log ) );
            fprintf( stderr, "%s", (const char *)str.mbc_str() );
            if ( NULL != m_db_vscp_log ) sqlite3_close( m_db_vscp_log );
            m_db_vscp_log = NULL;

        }

    }
    else {

        if ( m_path_db_vscp_log.IsOk() ) {
            
            // We need to create the database from scratch. This may not work if
            // the database is in a read only location.
            fprintf( stderr, 
                        "VSCP Server logging database does not exist - "
                        "will be created.\n" );
            str.Printf(_("Path=%s\n"), 
                    (const char *)m_path_db_vscp_log.GetFullPath().mbc_str() );
            fprintf( stderr, 
                        "%s", 
                        (const char *)str.mbc_str() );

            if ( SQLITE_OK == 
                    sqlite3_open( (const char *)m_path_db_vscp_log.GetFullPath().mbc_str(),
                                                    &m_db_vscp_log ) ) {
                // create the config. database.
                doCreateLogTable();
            }
            else {
                str.Printf( _("Failed to create vscp log database - will not be used.  Error=%s\n"), 
                            sqlite3_errmsg( m_db_vscp_log ) );
                fprintf( stderr, "%s", (const char *)str.mbc_str() );
                if ( NULL != m_db_vscp_log ) sqlite3_close( m_db_vscp_log );
                m_db_vscp_log = NULL;
            }
        }
        else {
            fprintf( stderr, "VSCP Server logging database path invalid - will not be used.\n" );
            str.Printf(_("Path=%s\n"), (const char *)m_path_db_vscp_log.GetFullPath().mbc_str() );
            fprintf( stderr, "%s", (const char *)str.mbc_str() );
            if ( NULL != m_db_vscp_log ) sqlite3_close( m_db_vscp_log );
            m_db_vscp_log = NULL;
        }

    }

    // https://www.sqlite.org/wal.html
    // http://stackoverflow.com/questions/3852068/sqlite-insert-very-slow
    if ( NULL != m_db_vscp_log ) {
        sqlite3_exec( m_db_vscp_log, "PRAGMA journal_mode = WAL", NULL, NULL, NULL );
        sqlite3_exec( m_db_vscp_log, "PRAGMA synchronous = NORMAL", NULL, NULL, NULL );
    }

    // * * * VSCP Server data database - NEVER created * * *

    if ( SQLITE_OK != sqlite3_open( m_path_db_vscp_data.GetFullPath().mbc_str(),
                                            &m_db_vscp_data ) ) {

        // Failed to open/create the database file
        fprintf( stderr, "The VSCP data database could not be opened. - Will not be used.\n" );
        str.Printf( _("Path=%s error=%s\n"),
                        (const char *)m_path_db_vscp_data.GetFullPath().mbc_str(),
                        sqlite3_errmsg( m_db_vscp_data ) );
        fprintf( stderr, "%s", (const char *)str.mbc_str() );
        if ( NULL != m_db_vscp_data ) sqlite3_close( m_db_vscp_data );
        m_db_vscp_data = NULL;

    }

    ////////////////////////////////////////////////////////////////////////////
    //                      Read full XML configuration
    ////////////////////////////////////////////////////////////////////////////

    str = _("Using configuration file: ") + strcfgfile + _("\n");
    fprintf( stderr, "%s", (const char *)str.mbc_str() );

    // Read XML configuration
    if ( !readConfigurationXML( strcfgfile ) ) {
        fprintf( stderr, "Unable to open/parse configuration file. Can't initialize!\n" );
        str = _("Path = .") + strcfgfile + _("\n");
        fprintf( stderr, "%s", (const char *)str.mbc_str() );
        return FALSE;
    }

    // Read users from database
    logMsg(_("loading users from users db...\n") );
    m_userList.loadUsers();



    //==========================================================================
    //                           Add admin user
    //==========================================================================

    m_userList.addSuperUser( m_admin_user,
                                m_admin_password,
                                m_admin_allowfrom ); // Remotes allows to connect


    //==========================================================================
    //                           Add driver user
    //==========================================================================

    // Generate username and password for drivers
    char buf[128];
    randPassword pw(4);

    // Level II Driver Username
    memset( buf, 0, sizeof( buf ) );
    pw.generatePassword( 32, buf );
    m_driverUsername = _("drv_");
    m_driverUsername += wxString::FromAscii( buf );

    // Level II Driver Password (can't contain ";" character)
    memset( buf, 0, sizeof( buf ) );
    pw.generatePassword( 32, buf );
    m_driverPassword = buf;

    wxString drvhash;
    vscp_makePasswordHash( drvhash,
                            wxString::FromAscii( buf ) );

    m_userList.addUser( m_driverUsername,
                            drvhash,        // salt;hash
                            _("System added driver user."), // full name
                            _("System added driver user."), // note
                            NULL,
                            _("driver"),
                            _("+127.0.0.0/24"),             // Only local
                            _("*:*"),                       // All events
                            VSCP_ADD_USER_FLAG_LOCAL );

    // Calculate sunset etc
    m_automation.calcSun();

    // Get GUID
    if ( m_guid.isNULL() ) {
        if ( !getMacAddress(m_guid ) ) {
            // We failed to create GUID from MAC address use
            // 'localhost' IP instead as the base.
            getIPAddress(m_guid);
        }
    }

    // If no server name set construct one
    if ( 0 == m_strServerName.Length() ) {
        m_strServerName = _( "VSCP Server @ " );;
        wxString strguid;
        m_guid.toString( strguid );
        m_strServerName += strguid;
    }

    str = _("VSCP Server started\n");
    str += _("Version: ");
    str += _(VSCPD_DISPLAY_VERSION);
    str += _("\n");
    str += _(VSCPD_COPYRIGHT);
    str += _("\n");
    logMsg( str );

    str.Printf(_("Log Level=%d\n"), m_logLevel );
    logMsg( str );

    // Load tables from database
    logMsg(_("Reading in user tables from DB.\n") );
    m_userTableObjects.loadTablesFromDB();

    logMsg(_("Initializing user tables.\n") );
    m_userTableObjects.init();

    // Initialize DM storage
    logMsg(_("Initializing DM.\n") );
    m_dm.init();

    // Load decision matrix from XML file if mechanism is enabled
    logMsg(_("Loading DM from XML file.\n") );
    m_dm.loadFromXML();

    // Load decision matrix from db if mechanism is enabled
    logMsg(_("Loading DM from database.\n") );
    m_dm.loadFromDatabase();

    // Initialize variable storage
    logMsg(_("Initialize variables.\n") );
    m_variables.init();

    // Load variables if mechanism is enabled
    logMsg(_("Loading persistent variables from XML variable default path.\n") );
    m_variables.loadFromXML();

    // Start daemon internal client worker thread
    startClientWorkerThread();

    // Start webserver and websockets
    // IMPORTANT!!!!!!!!
    // Must be started before the tcp/ip server as
    // ssl initializarion is done here
    start_webserver();

    // Start TCP/IP interface
    startTcpWorkerThread();

    // Start UDP interface
    startUDPWorkerThread();

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

bool CControlObject::run( void )
{
    CLIENTEVENTLIST::compatibility_iterator nodeClient;

    vscpEvent EventLoop;
    EventLoop.vscp_class = VSCP_CLASS2_VSCPD;
    EventLoop.vscp_type = VSCP2_TYPE_VSCPD_LOOP;
    EventLoop.sizeData = 0;
    EventLoop.pdata = NULL;

    vscpEvent EventStartUp;
    EventStartUp.vscp_class = VSCP_CLASS2_VSCPD;
    EventStartUp.vscp_type = VSCP2_TYPE_VSCPD_STARTING_UP;
    EventStartUp.sizeData = 0;
    EventStartUp.pdata = NULL;

    vscpEvent EventShutDown;
    EventShutDown.vscp_class = VSCP_CLASS2_VSCPD;
    EventShutDown.vscp_type = VSCP2_TYPE_VSCPD_SHUTTING_DOWN;
    EventShutDown.sizeData = 0;
    EventShutDown.pdata = NULL;

    // We need to create a clientItem and add this object to the list
    CClientItem *pClientItem = new CClientItem;
    if ( NULL == pClientItem ) {
        logMsg( _("Unable to allocate Client item, Ending.") );
        return false;
    }

    // Save a pointer to the client item
    m_dm.m_pClientItem = pClientItem;

    // Set Filter/Mask for full DM table
    memcpy( &pClientItem->m_filterVSCP,
                &m_dm.m_DM_Table_filter,
                sizeof( vscpEventFilter ) );

    // This is an active client
    pClientItem->m_bOpen = true;
    pClientItem->m_type = CLIENT_ITEM_INTERFACE_TYPE_CLIENT_INTERNAL;
    pClientItem->m_strDeviceName = _("Internal Server DM Client.|Started at ");
    pClientItem->m_strDeviceName += wxDateTime::Now().FormatISODate();
    pClientItem->m_strDeviceName += _(" ");
    pClientItem->m_strDeviceName += wxDateTime::Now().FormatISOTime();

    // Add the client to the Client List
    m_wxClientMutex.Lock();
    if ( !addClient( pClientItem, CLIENT_ID_DM ) ) {
        // Failed to add client
        delete pClientItem;
        m_dm.m_pClientItem = pClientItem = NULL;
        logMsg( _("ControlObject: Faild to add internal client.") );
        m_wxClientMutex.Unlock();
    }
    m_wxClientMutex.Unlock();

    // Feed startup event
    m_dm.feed( &EventStartUp );


    //-------------------------------------------------------------------------
    //                            MAIN - LOOP
    //-------------------------------------------------------------------------
    

    // DM Loop
    int cnt = 0;
    while ( !m_bQuit ) {
        
        // CLOCKS_PER_SEC
        clock_t ticks,oldus;
        oldus = ticks = clock();

        // Feed possible periodic event
        m_dm.feedPeriodicEvent();

        // Put the LOOP event on the queue
        // Garanties at least one lop event between every other
        // event feed to the queue
        m_dm.feed( &EventLoop );


        // Wait for event
        if ( wxSEMA_TIMEOUT ==
                pClientItem->m_semClientInputQueue.WaitTimeout( 100 ) ) {

            if ( m_bQuit ) continue;
#if 0            
            cnt++;
            if ( cnt > 100 ) m_bQuit = true;
#endif            
            // Put the LOOP event on the queue
            m_dm.feed( &EventLoop );
            continue;

        }

        //----------------------------------------------------------------------
        //                         Event received here
        //                   from one of the incoming source
        //----------------------------------------------------------------------


        if ( pClientItem->m_clientInputQueue.GetCount() ) {

            vscpEvent *pEvent;

            pClientItem->m_mutexClientInputQueue.Lock();
            nodeClient = pClientItem->m_clientInputQueue.GetFirst();
            pEvent = nodeClient->GetData();
            pClientItem->m_clientInputQueue.DeleteNode(nodeClient);
            pClientItem->m_mutexClientInputQueue.Unlock();

            if ( NULL != pEvent ) {

                if ( vscp_doLevel2Filter( pEvent, &m_dm.m_DM_Table_filter ) ) {
                    // Feed event through matrix
                    m_dm.feed( pEvent );
                }

                // Remove the event
                vscp_deleteVSCPevent(pEvent);

            } // Valid pEvent pointer

            // Send events to websocket clients
            websock_post_incomingEvents();

        } // Event in queue
        
    }  // while

    // Do shutdown event
    m_dm.feed( &EventShutDown );

    // Remove messages in the client queues
    m_wxClientMutex.Lock();
    removeClient( pClientItem );
    m_wxClientMutex.Unlock();

    fprintf(stderr, "ControlObject: Run - Done\n" );

    cleanup();

    return true;
}


/////////////////////////////////////////////////////////////////////////////
// cleanup

bool CControlObject::cleanup( void )
{
    /*m_mutexUDPInfo.Unlock();
    m_mutexLogWrite.Unlock();
    m_mutexTcpClientList.Unlock();
    m_mutexMulticastInfo.Unlock();
    m_websrvSessionMutex.Unlock();
    m_restSessionMutex.Unlock();
    m_websocketSessionMutex.Unlock();
    m_variableMutex.Unlock();
    m_db_vscp_configMutex.Unlock();
    m_wxClientMutex.Unlock();
    m_wxDeviceMutex.Unlock();
    m_mutexDeviceList.Unlock();
    m_mutexClientList.Unlock();
    m_mutexUserList.Unlock();
    m_mutexUserTables.Unlock();
    m_mutexKnownNodes.Unlock();
    m_mutexClientOutputQueue.Unlock();
    m_mutexclientMsgWorkerThread.Unlock();
    m_mutexdaemonVSCPThread.Unlock();
    m_mutexVSCPClientnUDPThread.Unlock();
    m_mutexudpClientThread.Unlock();*/
    
    fprintf( stderr, "ControlObject: cleanup - Giving worker threads time to stop operations...\n");    
    sleep( 2 ); // Give threads some time to end
    
    fprintf( stderr, "ControlObject: cleanup - Stopping device worker thread...\n");
    stopDeviceWorkerThreads();

    fprintf( stderr, "ControlObject: cleanup - Stopping VSCP Server worker thread...\n");
    stopDaemonWorkerThread();

    fprintf( stderr, "ControlObject: cleanup - Stopping client worker thread...\n");
    stopClientWorkerThread();  

    m_dm.cleanup();

    fprintf( stderr, "ControlObject: cleanup - Stopping Web Server worker thread...\n");
    stop_webserver();

    fprintf( stderr, "ControlObject: cleanup - Stopping UDP worker thread...\n");
    stopUDPWorkerThread();

    fprintf( stderr, "ControlObject: cleanup - Stopping Multicast worker threads...\n");
    stopMulticastWorkerThreads();

    fprintf( stderr, "ControlObject: cleanup - Stopping TCP/IP worker thread...\n");
    stopTcpWorkerThread();

    fprintf( stderr, "ControlObject: cleanup - Closing databases.\n");

    // Close the vscpd database
    if ( NULL != m_db_vscp_daemon ) sqlite3_close( m_db_vscp_daemon );
    m_db_vscp_daemon = NULL;

    // Close the VSCP data database
    if ( NULL != m_db_vscp_data ) sqlite3_close( m_db_vscp_data );
    m_db_vscp_data = NULL;

    // Close log database
    if ( NULL != m_db_vscp_log ) sqlite3_close( m_db_vscp_log );
    m_db_vscp_log = NULL;
    

    // Clean up SQLite lib allocations
    sqlite3_shutdown();

    fprintf( stderr, "Controlobject: ControlObject: Cleanup done.\n");
    return true;
}




/////////////////////////////////////////////////////////////////////////////
// startClientWorkerThread
//

bool CControlObject::startClientWorkerThread( void )
{
    /////////////////////////////////////////////////////////////////////////////
    // Load controlobject client message handler
    /////////////////////////////////////////////////////////////////////////////

    logMsg(_("Controlobject: Starting client worker thread...\n") );

    m_pclientMsgWorkerThread = new clientMsgWorkerThread;

    if (NULL != m_pclientMsgWorkerThread) {
        m_pclientMsgWorkerThread->m_pCtrlObject = this;
        wxThreadError err;
        if ( wxTHREAD_NO_ERROR == ( err = m_pclientMsgWorkerThread->Create() ) ) {
            //m_ptcpListenThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY );
            if ( wxTHREAD_NO_ERROR != ( err = m_pclientMsgWorkerThread->Run() ) ) {
                logMsg( _("Controlobject: Unable to run controlobject client thread.") );
            }
        }
        else {
            logMsg( _("Controlobject: Unable to create controlobject client thread.") );
        }
    }
    else {
        logMsg( _("Controlobject: Unable to allocate memory for controlobject client thread.") );
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// stopTcpWorkerThread
//

bool CControlObject::stopClientWorkerThread( void )
{
    if ( NULL != m_pclientMsgWorkerThread ) {
        m_mutexclientMsgWorkerThread.Lock();
        m_pclientMsgWorkerThread->m_bQuit = true;
        m_pclientMsgWorkerThread->Wait();
        delete m_pclientMsgWorkerThread;
        m_pclientMsgWorkerThread = NULL;
        m_mutexclientMsgWorkerThread.Unlock();
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// startTcpWorkerThread
//

bool CControlObject::startTcpWorkerThread(void)
{
    /////////////////////////////////////////////////////////////////////////////
    // Run the TCP server thread
    /////////////////////////////////////////////////////////////////////////////

    logMsg(_("Controlobject: Starting TCP/IP interface...\n") );

    m_StopTcpIpSrv = 0;
    m_pTCPListenThread = new TCPListenThread;

    if ( NULL != m_pTCPListenThread ) {

        wxThreadError err;
        if ( wxTHREAD_NO_ERROR == ( err = m_pTCPListenThread->Create() ) ) {
            
            m_pTCPListenThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY );
            
            if ( wxTHREAD_NO_ERROR != ( err = m_pTCPListenThread->Run() ) ) {
                logMsg(_("Controlobject: Unable to run TCP thread.") );
            }

        }
        else {
            logMsg( _("Controlobject:  Unable to create TCP thread.") );
        }

    }
    else {
        logMsg(_("Controlobject: Unable to allocate memory for TCP thread.") );
    }


    return true;
}


/////////////////////////////////////////////////////////////////////////////
// stopTcpWorkerThread
//

bool CControlObject::stopTcpWorkerThread( void )
{
    // Tell the thread it's time to quit
    m_StopTcpIpSrv = 1;

    logMsg(_("Controlobject: Terminating TCP thread.") );

    //if ( NULL != m_pTCPClientThread ) {

    // Wait for magic number confirming thread ending
    /*int cnt = 0;
    while ( VSCPD_QUIT_FLAG == m_confirmQuitTcpIpSrv ) {
        wxSleep( 1 );
        cnt++;
        if ( cnt > 5 ) {
            logMsg(_("Controlobject: No termination confirm from TCP thread. Quitting anyway") );
            break;
        }
    }*/

        //m_mutexTcpClientListenThread.Lock();        
        //delete m_pTCPClientThread;
        //m_pTCPClientThread = NULL;
        //m_mutexTcpClientListenThread.Unlock();
    //}
    
    // Wait for the tcp/ip listen thread to terminate
    m_semTcpIpThread.Wait();

    logMsg(_("Controlobject: Terminated TCP thread.") );

    return true;
}



/////////////////////////////////////////////////////////////////////////////
// startUDPWorkerThread
//

bool CControlObject::startUDPWorkerThread( void )
{
    /////////////////////////////////////////////////////////////////////////////
    // Run the UDP server thread
    /////////////////////////////////////////////////////////////////////////////
    if ( m_udpInfo.m_bEnable ) {

        logMsg(_("Controlobject: Starting UDP simple interface...\n") );

        m_pVSCPClientUDPThread = new VSCPUDPClientThread;

        if ( NULL != m_pVSCPClientUDPThread ) {
            wxThreadError err;
            if (wxTHREAD_NO_ERROR == (err = m_pVSCPClientUDPThread->Create())) {
                //m_ptcpListenThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY );
                if (wxTHREAD_NO_ERROR != (err = m_pVSCPClientUDPThread->Run())) {
                    logMsg( _("Controlobject: Unable to run UDP client thread.") );
                }
            }
            else {
                logMsg( _("Controlobject: Unable to create UDP client thread.") );
            }
        }
        else {
            logMsg( _("Controlobject: Unable to allocate memory for UDP client thread.") );
        }
    }

    return true;
}


/////////////////////////////////////////////////////////////////////////////
// stopUDPWorkerThread
//

bool CControlObject::stopUDPWorkerThread( void )
{
    if ( NULL != m_pVSCPClientUDPThread ) {
        m_mutexVSCPClientnUDPThread.Lock();
        m_pVSCPClientUDPThread->m_bQuit = true;
        m_pVSCPClientUDPThread->Wait();
        delete m_pVSCPClientUDPThread;
        m_mutexVSCPClientnUDPThread.Unlock();
    }

    return true;
}


/////////////////////////////////////////////////////////////////////////////
// startMulticastWorkerThreads
//

bool CControlObject::startMulticastWorkerThreads( void )
{
    /////////////////////////////////////////////////////////////////////////////
    // Start all enabled multicast channel threads
    /////////////////////////////////////////////////////////////////////////////

    if ( m_multicastInfo.m_bEnable  && m_multicastInfo.m_channels.GetCount() ) {

        MULTICASTCHANNELLIST::iterator iter;
        for ( iter = m_multicastInfo.m_channels.begin();
                   iter != m_multicastInfo.m_channels.end(); ++iter) {

            multicastChannelItem *pChannel = *iter;
            if ( NULL == pChannel ) {
                logMsg(_("Controlobject: Multicast start channel table invalid entry.\n") );
                continue;
            }

            logMsg(_("Controlobject: Starting multicast channel interface thread...\n") );
            pChannel->m_pWorkerThread = new VSCPMulticastClientThread;

            if ( NULL != pChannel->m_pWorkerThread) {

                // Share the multicast channel item
                pChannel->m_pWorkerThread->m_pChannel = pChannel;

                wxThreadError err;
                if (wxTHREAD_NO_ERROR == (err = pChannel->m_pWorkerThread->Create())) {
                    //m_ptcpListenThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY );
                    if (wxTHREAD_NO_ERROR != (err = pChannel->m_pWorkerThread->Run())) {
                        logMsg( _("Controlobject: Unable to run multicast channel thread.") );
                    }
                }
                else {
                    logMsg( _("Controlobject: Unable to create multicast channel thread.") );
                }
            }
            else {
                logMsg( _("Controlobject: Unable to allocate memory for multicast channel thread.") );
            }
        }
    }

    return true;
}


/////////////////////////////////////////////////////////////////////////////
// stopMulticastWorkerThreads
//

bool CControlObject::stopMulticastWorkerThreads( void )
{
    MULTICASTCHANNELLIST::iterator iter;
    for ( iter = m_multicastInfo.m_channels.begin();
                   iter != m_multicastInfo.m_channels.end(); ++iter) {

        multicastChannelItem *pChannel = *iter;
        if ( NULL == pChannel ) {
            logMsg(_("Controlobject: Multicast end channel table invalid entry.\n") );
            continue;
        }

        if ( NULL != pChannel->m_pWorkerThread ) {
            pChannel->m_mutexVSCPMulticastThread.Lock();
            pChannel->m_pWorkerThread->m_bQuit = true;
            pChannel->m_pWorkerThread->Wait();
            delete pChannel->m_pWorkerThread;
            pChannel->m_pWorkerThread = NULL;
            pChannel->m_mutexVSCPMulticastThread.Unlock();
        }

    }



    return true;
}


/////////////////////////////////////////////////////////////////////////////
// startDaemonWorkerThread
//

bool CControlObject::startDaemonWorkerThread( void )
{
    /////////////////////////////////////////////////////////////////////////////
    // Run the VSCP daemon thread
    /////////////////////////////////////////////////////////////////////////////

    logMsg(_("Controlobject: Starting VSCP Server worker thread,,.\n") );

    m_pdaemonVSCPThread = new daemonVSCPThread;

    if ( NULL != m_pdaemonVSCPThread ) {
            m_pdaemonVSCPThread->m_pCtrlObject = this;

        wxThreadError err;
        if ( wxTHREAD_NO_ERROR == ( err = m_pdaemonVSCPThread->Create() ) ) {
            m_pdaemonVSCPThread->SetPriority(WXTHREAD_DEFAULT_PRIORITY);
            if ( wxTHREAD_NO_ERROR != ( err = m_pdaemonVSCPThread->Run() ) ) {
                logMsg( _("Controlobject: Unable to start TCP VSCP Server thread.") );
            }
        }
        else {
            logMsg( _("Controlobject: Unable to create TCP VSCP Server thread.") );
        }
    }
    else {
        logMsg( _("Controlobject: Unable to start VSCP Server thread.") );
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// stopDaemonWorkerThread
//

bool CControlObject::stopDaemonWorkerThread( void )
{
    if ( NULL != m_pdaemonVSCPThread ) {
        m_mutexdaemonVSCPThread.Lock();
        m_pdaemonVSCPThread->m_bQuit = true;
        m_pdaemonVSCPThread->Wait();
        delete m_pdaemonVSCPThread;
        m_pdaemonVSCPThread = NULL;
        m_mutexdaemonVSCPThread.Unlock();
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
//
//

bool CControlObject::startDeviceWorkerThreads( void )
{
    CDeviceItem *pDeviceItem;

    logMsg(_("[Controlobject][Driver] - Starting drivers...\n") );

    VSCPDEVICELIST::iterator iter;
    for ( iter = m_deviceList.m_devItemList.begin();
            iter != m_deviceList.m_devItemList.end();
            ++iter ) {

        pDeviceItem = *iter;
        if ( NULL != pDeviceItem ) {

            logMsg( _("Controlobject: [Driver] - Preparing: ") + pDeviceItem->m_strName + _("\n") );            

            // Just start if enabled
            if ( !pDeviceItem->m_bEnable ) continue;
            
            if ( VSCP_DRIVER_LEVEL3 == pDeviceItem->m_driverLevel ) {
                        
                //  Startup Level III driver
                wxString executable = pDeviceItem->m_strPath;

                pDeviceItem->m_pDriver3Process = new Driver3Process;
                if ( 0 == ( pDeviceItem->m_pid = 
                        wxExecute( executable.mbc_str(),
                                    wxEXEC_ASYNC,
                                    pDeviceItem->m_pDriver3Process ) ) ) {
                    wxString str;
                    str = _("Controlobject: Failed to load level III driver: ");
                    str += pDeviceItem->m_strName;
                    str += _("\n");
                    logMsg(str);
                    wxLogDebug(str);
                    return NULL;
                }
                else {
                    
                }
                
            }

            // Start  the driver logic
            pDeviceItem->startDriver( this );
           

        } // Valid device item

    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
// stopDeviceWorkerThreads
//

bool CControlObject::stopDeviceWorkerThreads( void )
{
    CDeviceItem *pDeviceItem;

    VSCPDEVICELIST::iterator iter;
    for ( iter = m_deviceList.m_devItemList.begin();
            iter != m_deviceList.m_devItemList.end();
            ++iter) {

        pDeviceItem = *iter;
        if ( NULL != pDeviceItem ) {
            pDeviceItem->stopDriver();
        }

    }

    return true;
}


/////////////////////////////////////////////////////////////////////////////
// generateSessionId
//

bool CControlObject::generateSessionId( const char *pKey, char *psid )
{
    char buf[ 8193 ];

    // Check pointers
    if ( NULL == pKey ) return false;
    if ( NULL == psid ) return false;

    if ( strlen(pKey) > 256 ) return false;

    // Generate a random session ID
    time_t t;
    t = time( NULL );
    sprintf( buf,
                "__%s_%X%X%X%X_be_hungry_stay_foolish_%X%X",
                pKey,
                (unsigned int)rand(),
                (unsigned int)rand(),
                (unsigned int)rand(),
                (unsigned int)t,
                (unsigned int)rand(),
                1337 );

    vscp_md5( psid, (const unsigned char *)buf, strlen( buf ) );

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// getVscpCapabilities
//

bool CControlObject::getVscpCapabilities( uint8_t *pCapability )
{
    // Check pointer
    if ( NULL == pCapability ) return false;

    memset( pCapability, 0, 8 );

    // VSCP Multicast interface
    if ( m_multicastInfo.m_bEnable ) {
        pCapability[ 8 - ( VSCP_SERVER_CAPABILITY_MULTICAST_CHANNEL/8 ) ] |=
                    ( 1 << ( VSCP_SERVER_CAPABILITY_MULTICAST_CHANNEL % 8 ) );
    }

    // VSCP TCP/IP interface
    if ( m_enableTcpip ) {
        pCapability[ 8 - ( VSCP_SERVER_CAPABILITY_TCPIP/8 ) ] |=
                    ( 1 << ( VSCP_SERVER_CAPABILITY_TCPIP % 8 ) );
    }

    // VSCP UDP interface
    if ( m_udpInfo.m_bEnable ) {
        pCapability[ 8 - ( VSCP_SERVER_CAPABILITY_UDP/8 ) ] |=
                    ( 1 << ( VSCP_SERVER_CAPABILITY_UDP % 8 ) );
    }

    // VSCP Multicast announce interface
    if ( m_bMulticastAnounce ) {
        pCapability[ 8 - ( VSCP_SERVER_CAPABILITY_MULTICAST_ANNOUNCE/8 ) ] |=
                    ( 1 << ( VSCP_SERVER_CAPABILITY_MULTICAST_ANNOUNCE % 8 ) );
    }

    // VSCP raw Ethernet interface
    if ( 1 ) {
        pCapability[ 8 - ( VSCP_SERVER_CAPABILITY_RAWETH/8 ) ] |=
                    ( 1 << ( VSCP_SERVER_CAPABILITY_RAWETH % 8 ) );
    }

    // VSCP web server
    if ( m_web_bEnable ) {
        pCapability[ 8 - ( VSCP_SERVER_CAPABILITY_WEB/8 ) ] |=
                    ( 1 << ( VSCP_SERVER_CAPABILITY_WEB % 8 ) );
    }

    // VSCP websocket interface
    if ( m_web_bEnable ) {
        pCapability[ 8 - ( VSCP_SERVER_CAPABILITY_WEBSOCKET/8 ) ] |=
                    ( 1 << ( VSCP_SERVER_CAPABILITY_WEBSOCKET % 8 ) );
    }

    // VSCP websocket interface
    if ( m_web_bEnable ) {
        pCapability[ 8 - ( VSCP_SERVER_CAPABILITY_REST/8 ) ] |=
                    ( 1 << ( VSCP_SERVER_CAPABILITY_REST % 8 ) );
    }

    // IPv6 support
    if ( 0 ) {
        pCapability[ 8 - ( VSCP_SERVER_CAPABILITY_IP6/8 ) ] |=
                    ( 1 << ( VSCP_SERVER_CAPABILITY_IP6 % 8 ) );
    }

    // IPv4 support
    if ( 0 ) {
        pCapability[ 8 - ( VSCP_SERVER_CAPABILITY_IP4/8 ) ] |=
                    ( 1 << ( VSCP_SERVER_CAPABILITY_IP4 % 8 ) );
    }

    // SSL support
    if ( 1 ) {
        pCapability[ 8 - ( VSCP_SERVER_CAPABILITY_SSL/8 ) ] |=
                    ( 1 << ( VSCP_SERVER_CAPABILITY_SSL % 8 ) );
    }

    // +2 tcp/ip connections support
    if ( m_enableTcpip ) {
        pCapability[ 8 - ( VSCP_SERVER_CAPABILITY_TWO_CONNECTIONS/8 ) ] |=
                    ( 1 << ( VSCP_SERVER_CAPABILITY_TWO_CONNECTIONS % 8 ) );
    }

    // AES256
    pCapability[ 15 ] |= (1<<2);

    // AES192
    pCapability[ 15 ] |= (1<<1);

    // AES128
    pCapability[ 15 ] |= 1;

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// logMsg
//

void CControlObject::logMsg( const wxString& msgin, 
                                const uint8_t level, 
                                const uint8_t nType )
{
    wxString msg = msgin;
    
    m_mutexLogWrite.Lock();

    // Add CR if not set.
    if ( wxNOT_FOUND == msg.find( _("\n") ) ) {
        msg += _("\n");
    }

    wxDateTime datetime( wxDateTime::GetTimeNow() );
    wxString wxdebugmsg;

    wxdebugmsg = datetime.FormatISODate() + _("T") + 
                    datetime.FormatISOTime() + _(" - ") + msg;

#ifdef WIN32
#ifdef BUILD_VSCPD_SERVICE

    const char* ps[3];
    ps[ 0 ] = wxstr;
    ps[ 1 ] = NULL;
    ps[ 2 ] = NULL;

    int iStr = 0;
    for (int i = 0; i < 3; i++) {
        if (ps[i] != NULL) {
            iStr++;
        }
    }

    ::ReportEventA(m_hEventSource,
                    EVENTLOG_INFORMATION_TYPE,
                    0,
                    (1L << 30),
                    NULL, // sid
                    iStr,
                    0,
                    ps,
                    NULL);
#endif
#endif

    // Log to database
    if ( ( NULL != m_db_vscp_log ) &&
            ( m_logLevel >= level) ) {


        char *zErrMsg = NULL;

        char *sql = sqlite3_mprintf( VSCPDB_LOG_INSERT,
            nType,
            (const char *)(datetime.FormatISODate() + _("T") + 
                datetime.FormatISOTime() ).mbc_str(),
            level,
            (const char *)msg.mbc_str() );

        if ( SQLITE_OK != sqlite3_exec( m_db_vscp_log,
                                        sql, NULL, NULL, &zErrMsg ) ) {
            wxPrintf( "Failed to write message to log database. Error is: "
                      "%s -- Message is: %s\n",
                        zErrMsg,
                        (const char *)msg.mbc_str() );
        }
        sqlite3_free( sql );
        
        // Clean up old entries
        if ( -1 != m_logDays ) {
            sql = sqlite3_mprintf( VSCPDB_LOG_DELETE_OLD, m_logDays );        
            sqlite3_exec( m_db_vscp_log, sql, NULL, NULL, &zErrMsg );
            sqlite3_free( sql );
        }
        
    }

#ifndef WIN32

    // Print to console if there is one
    if ( m_logLevel >= level ) {
        wxPrintf( "%s", wxdebugmsg.mbc_str() );
    }

    if ( m_bLogToSysLog ) {

        switch (level) {

        case DAEMON_LOGMSG_NORMAL:
            syslog(LOG_INFO, "%s", (const char *)wxdebugmsg.mbc_str() );
            break;

        case DAEMON_LOGMSG_DEBUG:
        default:
            syslog(LOG_DEBUG, "%s", (const char *) wxdebugmsg.mbc_str() );
            break;

        };

    }

#endif

     m_mutexLogWrite.Unlock();

}

/////////////////////////////////////////////////////////////////////////////
// getCountRecordsDB
//

long CControlObject::getCountRecordsDB( sqlite3 *db, wxString& table )
{
    long count = 0;
    sqlite3_stmt *ppStmt;

    // If not open no records
    if ( NULL == db ) return 0;

    wxString sql = wxString::Format("SELECT count(*)from %s", 
                                    (const char *)table.mbc_str() );

    if ( SQLITE_OK != sqlite3_prepare( db,
                                        (const char *)sql.mbc_str(),
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

    return count;
}

/////////////////////////////////////////////////////////////////////////////
// getCountRecordsLogDB
//

long CControlObject::getCountRecordsLogDB( void )
{
    long count = 0;
    sqlite3_stmt *ppStmt;

    // If not open no records
    if ( NULL == m_db_vscp_log ) return 0;


    m_mutexLogWrite.Lock();

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

    m_mutexLogWrite.Unlock();

    return count;
}


/////////////////////////////////////////////////////////////////////////////
// searchLogDB
//

bool CControlObject::searchLogDB( const char * sql, wxString& strResult )
{
    sqlite3_stmt *ppStmt;

    // If not open no records
    if ( NULL == m_db_vscp_log ) return 0;


    m_mutexLogWrite.Lock();

    if ( SQLITE_OK != sqlite3_prepare( m_db_vscp_log,
                                        sql,
                                        -1,
                                        &ppStmt,
                                        NULL ) )  {
        logMsg( wxString::Format( _("Failed to get records from log "
                                    "database. SQL is %s"),
                                    sql ) );
        m_mutexLogWrite.Unlock();
        return false;
    }

    while ( SQLITE_ROW == sqlite3_step( ppStmt ) ) {
        wxString wxstr;
        wxstr = sqlite3_column_text( ppStmt, 0 );
        wxstr += _(",");
        wxstr += sqlite3_column_text( ppStmt, 1 );
        wxstr += _(",");
        wxstr += sqlite3_column_text( ppStmt, 2 );
        wxstr += _(",");
        wxstr += sqlite3_column_text( ppStmt, 3 );
        wxstr += _(",");
        wxstr += sqlite3_column_text( ppStmt, 4 );

        wxstr += _("|");
        strResult += wxstr;
    }

    m_mutexLogWrite.Unlock();

    return true;
}



///////////////////////////////////////////////////////////////////////////////
// sendEventToClient
//

void CControlObject::sendEventToClient( CClientItem *pClientItem,
                                            vscpEvent *pEvent)
{
    // Must be valid pointers
    if (NULL == pClientItem) return;
    if (NULL == pEvent) return;

    // Check if filtered out - if so do nothing here
    if (!vscp_doLevel2Filter(pEvent, &pClientItem->m_filterVSCP)) return;

    // If the client queue is full for this client then the
    // client will not receive the message
    if (pClientItem->m_clientInputQueue.GetCount() >
            m_maxItemsInClientReceiveQueue) {
        // Overrun
        pClientItem->m_statistics.cntOverruns++;
        return;
    }

    // Create an event
    vscpEvent *pnewvscpEvent = new vscpEvent;
    if (NULL != pnewvscpEvent) {

        // Copy in the new event
        memcpy( pnewvscpEvent, pEvent, sizeof( vscpEvent) );

        // And data...
        if ( ( NULL != pEvent->pdata ) && ( pEvent->sizeData > 0 ) ) {
            // Copy in data
            pnewvscpEvent->pdata = new uint8_t[ pEvent->sizeData ];
            if ( NULL != pnewvscpEvent->pdata ) {
                memcpy( pnewvscpEvent->pdata, pEvent->pdata, pEvent->sizeData );
            }
        }
        else {
            // No data
            pnewvscpEvent->pdata = NULL;
        }

        // Add the new event to the input queue
        pClientItem->m_mutexClientInputQueue.Lock();
        pClientItem->m_clientInputQueue.Append(pnewvscpEvent);
        pClientItem->m_semClientInputQueue.Post();
        pClientItem->m_mutexClientInputQueue.Unlock();

    }
}

///////////////////////////////////////////////////////////////////////////////
// sendEventAllClients
//

void CControlObject::sendEventAllClients( vscpEvent *pEvent,
                                            uint32_t excludeID )
{
    CClientItem *pClientItem;
    std::list<CClientItem*>::iterator it;

    if (NULL == pEvent) return;

    wxLogTrace(_("wxTRACE_vscpd_receiveQueue"),
            _(" ControlObject: event %d, excludeid = %d"),
            pEvent->obid, excludeID);

    m_wxClientMutex.Lock();
    for ( it = m_clientList.m_clientItemList.begin();
            it != m_clientList.m_clientItemList.end();
            ++it) {
        pClientItem = *it;

        if ( NULL != pClientItem ) {
            wxLogTrace(_("wxTRACE_vscpd_receiveQueue"),
                    _(" ControlObject: clientid = %d"),
                    pClientItem->m_clientID);
        }

        if ( ( NULL != pClientItem ) && 
             ( excludeID != pClientItem->m_clientID ) ) {
            sendEventToClient( pClientItem, pEvent );
            wxLogTrace(_("wxTRACE_vscpd_receiveQueue"),
                    _(" ControlObject: Sent to client %d"),
                    pClientItem->m_clientID);
        }
    }

    m_wxClientMutex.Unlock();

}


///////////////////////////////////////////////////////////////////////////////
// sendEvent
//

bool CControlObject::sendEvent( CClientItem *pClientItem,
                                    vscpEvent *peventToSend )
{
    bool bSent = false;

    // Check pointers
    if ( NULL == pClientItem ) return false;
    if ( NULL == peventToSend ) return false;

    // If timestamp is nulled make one
    if ( 0 == peventToSend->timestamp ) {
        peventToSend->timestamp = vscp_makeTimeStamp();
    }

    // If obid is nulled set client interface id
    if ( 0 == peventToSend->obid ) {
        peventToSend->obid = pClientItem->m_clientID;
    }

    // If GUID is all nilled set interface GUID
    if ( vscp_isGUIDEmpty( peventToSend->GUID ) ) {
        memcpy( peventToSend->GUID, pClientItem->m_guid.getGUID(), 16 );
    }

    vscpEvent *pEvent = new vscpEvent;  // Create new VSCP Event
    if ( NULL == pEvent ) {
        return false;
    }

    // Copy event
    vscp_copyVSCPEvent( pEvent, peventToSend );

    // We don't need the original event anymore
    if ( NULL != peventToSend->pdata ) {
        delete [] peventToSend->pdata;
        peventToSend->pdata = NULL;
        peventToSend->sizeData = 0;
    }

    // Save the originating clients id so
    // this client don't get the message back
    pEvent->obid = pClientItem->m_clientID;

        // Level II events between 512-1023 is recognised by the daemon and
        // sent to the correct interface as Level I events if the interface
        // is addressed by the client.
        if ( ( pEvent->vscp_class <= 1023 ) &&
            ( pEvent->vscp_class >= 512 ) &&
            ( pEvent->sizeData >= 16 )	) {

            // This event should be sent to the correct interface if it is
            // available on this machine. If not it should be sent to
            // the rest of the network as normal

            cguid destguid;
            destguid.getFromArray( pEvent->pdata );

            destguid.setAt(0,0);    // Interface GUID's have LSB bytes nilled
            destguid.setAt(1,0);

            wxString dbgStr =
                    wxString::Format( _("Level I event over Level II "
                                        "dest = %d:%d:%d:%d:%d:%d:%d:%d:"
                                        "%d:%d:%d:%d:%d:%d:%d:%d:"),
                    destguid.getAt(15),destguid.getAt(14),
                    destguid.getAt(13),destguid.getAt(12),
                    destguid.getAt(11),destguid.getAt(10),
                    destguid.getAt(9),destguid.getAt(8),
                    destguid.getAt(7),destguid.getAt(6),
                    destguid.getAt(5),destguid.getAt(4),
                    destguid.getAt(3),destguid.getAt(2),
                    destguid.getAt(1),destguid.getAt(0) );
                    logMsg( dbgStr, DAEMON_LOGMSG_DEBUG );

            // Find client
            m_wxClientMutex.Lock();

            CClientItem *pDestClientItem = NULL;
            std::list<CClientItem*>::iterator it;
            for (it = m_clientList.m_clientItemList.begin();
                    it != m_clientList.m_clientItemList.end();
                    ++it ) {

                CClientItem *pItem = *it;
                dbgStr =
                    wxString::Format( _("Test if = %d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:"),
                    pItem->m_guid.getAt(15),pItem->m_guid.getAt(14),pItem->m_guid.getAt(13),pItem->m_guid.getAt(12),
                    pItem->m_guid.getAt(11),pItem->m_guid.getAt(10),pItem->m_guid.getAt(9),pItem->m_guid.getAt(8),
                    pItem->m_guid.getAt(7),pItem->m_guid.getAt(6),pItem->m_guid.getAt(5),pItem->m_guid.getAt(4),
                    pItem->m_guid.getAt(3),pItem->m_guid.getAt(2),pItem->m_guid.getAt(1),pItem->m_guid.getAt(0) );
                    dbgStr += _(" ");
                    dbgStr += pItem->m_strDeviceName;
                    logMsg( dbgStr, DAEMON_LOGMSG_DEBUG );

                    if ( pItem->m_guid == destguid ) {
                        // Found
                        pDestClientItem = pItem;
                        bSent = true;
                        dbgStr = _("Match ");
                        logMsg( dbgStr, DAEMON_LOGMSG_DEBUG );
                        sendEventToClient( pItem, pEvent );
                        break;
                    }

                }

                m_wxClientMutex.Unlock();

        }


        if ( !bSent ) {

            // There must be room in the send queue
            if ( m_maxItemsInClientReceiveQueue >
                m_clientOutputQueue.GetCount() ) {

                    m_mutexClientOutputQueue.Lock();
                    m_clientOutputQueue.Append ( pEvent );
                    m_semClientOutputQueue.Post();
                    m_mutexClientOutputQueue.Unlock();

                    // TX Statistics
                    pClientItem->m_statistics.cntTransmitData += pEvent->sizeData;
                    pClientItem->m_statistics.cntTransmitFrames++;

            }
            else {

                pClientItem->m_statistics.cntOverruns++;

                vscp_deleteVSCPevent( pEvent );
                return false;

            }

        }


    return true;
}






///////////////////////////////////////////////////////////////////////////////
//  removeIdFromClientMap
//

bool CControlObject::removeIdFromClientMap( uint32_t clid )
{
    for ( uint32_t i = 0; i < VSCP_MAX_CLIENTS; i++ ) {
        if ( clid == m_clientMap[ i ] ) {
            m_clientMap[ i ] = 0;
            return true;
        }
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////
// addClient
//

bool CControlObject::addClient( CClientItem *pClientItem, uint32_t id )
{
    // Add client to client list
    if ( !m_clientList.addClient( pClientItem, id ) ) {
        return false;
    }

    // Set GUID for interface
    pClientItem->m_guid = m_guid;

    // Fill in client id
    pClientItem->m_guid.setNicknameID( 0 );
    pClientItem->m_guid.setClientID( pClientItem->m_clientID );

    return true;
}


//////////////////////////////////////////////////////////////////////////////
// removeClient
//

void CControlObject::removeClient( CClientItem *pClientItem )
{
    // Do not try to handle invalid clients
    if ( NULL == pClientItem ) return;
    
    // Remove the mapped item
    removeIdFromClientMap( pClientItem->m_clientID );

    // Remove the client
    m_clientList.removeClient( pClientItem );
}

//////////////////////////////////////////////////////////////////////////////
// addKnowNode
//

void CControlObject::addKnownNode( cguid& guid,
                                    cguid& ifguid,
                                    wxString& name )
{
    ;   // TODO ???
}

///////////////////////////////////////////////////////////////////////////////
//  getMacAddress
//

bool CControlObject::getMacAddress( cguid& guid )
{
#ifdef WIN32

    bool rv = false;
    NCB Ncb;
    UCHAR uRetCode;
    LANA_ENUM lenum;
    int i;

    // Clear the GUID
    guid.clear();

    memset( &Ncb, 0, sizeof( Ncb ) );
    Ncb.ncb_command = NCBENUM;
    Ncb.ncb_buffer = (UCHAR *) & lenum;
    Ncb.ncb_length = sizeof( lenum);
    uRetCode = Netbios( &Ncb );
    //printf( "The NCBENUM return code is: 0x%x \n", uRetCode );

    for (i = 0; i < lenum.length; i++) {
        memset(&Ncb, 0, sizeof( Ncb));
        Ncb.ncb_command = NCBRESET;
        Ncb.ncb_lana_num = lenum.lana[i];

        uRetCode = Netbios(&Ncb);

        memset(&Ncb, 0, sizeof( Ncb));
        Ncb.ncb_command = NCBASTAT;
        Ncb.ncb_lana_num = lenum.lana[i];

        strcpy((char *) Ncb.ncb_callname, "*               ");
        Ncb.ncb_buffer = (unsigned char *) &Adapter;
        Ncb.ncb_length = sizeof( Adapter);

        uRetCode = Netbios(&Ncb);

        if (uRetCode == 0) {
            guid.setAt( 0, 0xff );
            guid.setAt( 1, 0xff );
            guid.setAt( 2, 0xff );
            guid.setAt( 3, 0xff );
            guid.setAt( 4, 0xff );
            guid.setAt( 5, 0xff );
            guid.setAt( 6, 0xff );
            guid.setAt( 7, 0xfe );
            guid.setAt( 8, Adapter.adapt.adapter_address[ 0 ] );
            guid.setAt( 9, Adapter.adapt.adapter_address[ 1 ] );
            guid.setAt( 10, Adapter.adapt.adapter_address[ 2 ] );
            guid.setAt( 11, Adapter.adapt.adapter_address[ 3 ] );
            guid.setAt( 12, Adapter.adapt.adapter_address[ 4 ] );
            guid.setAt( 13, Adapter.adapt.adapter_address[ 5 ] );
            guid.setAt( 14, 0 );
            guid.setAt( 15, 0 );
#ifdef __WXDEBUG__
            char buf[256];
            sprintf(buf, "The Ethernet MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
                    guid.getAt(2),
                    guid.getAt(3),
                    guid.getAt(4),
                    guid.getAt(5),
                    guid.getAt(6),
                    guid.getAt(7));

            wxString str = wxString::FromUTF8(buf);
            wxLogDebug(str);
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
    if ( -1 == fd ) return false;

    memset( &s, 0, sizeof( s ) );
    strcpy( s.ifr_name, "eth0");

    if ( 0 == ioctl( fd, SIOCGIFHWADDR, &s ) ) {

        unsigned char *ptr;
        ptr = (unsigned char *) &s.ifr_ifru.ifru_hwaddr.sa_data[ 0 ];
        logMsg( wxString::Format( _("Ethernet MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n"),
                                        (uint8_t)s.ifr_addr.sa_data[ 0 ],
                                        (uint8_t)s.ifr_addr.sa_data[ 1 ],
                                        (uint8_t)s.ifr_addr.sa_data[ 2 ],
                                        (uint8_t)s.ifr_addr.sa_data[ 3 ],
                                        (uint8_t)s.ifr_addr.sa_data[ 4 ],
                                        (uint8_t)s.ifr_addr.sa_data[ 5 ] ), DAEMON_LOGMSG_DEBUG  );

        guid.setAt( 0, 0xff );
        guid.setAt( 1, 0xff );
        guid.setAt( 2, 0xff );
        guid.setAt( 3, 0xff );
        guid.setAt( 4, 0xff );
        guid.setAt( 5, 0xff );
        guid.setAt( 6, 0xff );
        guid.setAt( 7, 0xfe );
        guid.setAt( 8, s.ifr_addr.sa_data[ 0 ] );
        guid.setAt( 9, s.ifr_addr.sa_data[ 1 ] );
        guid.setAt( 10, s.ifr_addr.sa_data[ 2 ] );
        guid.setAt( 11, s.ifr_addr.sa_data[ 3 ] );
        guid.setAt( 12, s.ifr_addr.sa_data[ 4 ] );
        guid.setAt( 13, s.ifr_addr.sa_data[ 5 ] );
        guid.setAt( 14, 0 );
        guid.setAt( 15, 0 );
    }
    else {
        logMsg( _("Failed to get hardware address (must be root?).\n") );
        rv = false;
    }

    return rv;


#endif

}



///////////////////////////////////////////////////////////////////////////////
//  getIPAddress
//

bool CControlObject::getIPAddress( cguid& guid )
{
    // Clear the GUID
    guid.clear();

    guid.setAt( 0, 0xff );
    guid.setAt( 1, 0xff );
    guid.setAt( 2, 0xff );
    guid.setAt( 3, 0xff );
    guid.setAt( 4, 0xff );
    guid.setAt( 5, 0xff );
    guid.setAt( 6, 0xff );
    guid.setAt( 7, 0xfd );

    char szName[ 128 ];
    gethostname(szName, sizeof( szName));
#if defined(_WIN32)
    LPHOSTENT lpLocalHostEntry;
#else
    struct hostent *lpLocalHostEntry;
#endif
    lpLocalHostEntry = gethostbyname( szName );
    if (NULL == lpLocalHostEntry) {
        return false;
    }

    // Get all local addresses
    int idx = -1;
    void *pAddr;
    unsigned long localaddr[ 16 ]; // max 16 local addresses
    do {
        idx++;
        localaddr[ idx ] = 0;
        pAddr = lpLocalHostEntry->h_addr_list[ idx ];
        if (NULL != pAddr) localaddr[ idx ] = *((unsigned long *) pAddr);
    } while ((NULL != pAddr) && (idx < 16));

    guid.setAt( 8, (localaddr[ 0 ] >> 24) & 0xff );
    guid.setAt( 9, (localaddr[ 0 ] >> 16) & 0xff );
    guid.setAt( 10, (localaddr[ 0 ] >> 8) & 0xff );
    guid.setAt( 11, localaddr[ 0 ] & 0xff );

    guid.setAt( 12, 0 );
    guid.setAt( 13, 0 );
    guid.setAt( 14, 0 );
    guid.setAt( 15, 0 );

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// getSystemKey
//

uint8_t *CControlObject::getSystemKey( uint8_t *pKey )
{
    if ( NULL != pKey ) {
        memcpy( pKey, m_systemKey, 32 );
    }

    return m_systemKey;
}

///////////////////////////////////////////////////////////////////////////////
// getSystemKeyMD5
//

void CControlObject::getSystemKeyMD5( wxString &strKey )
{
    char digest[33];
    vscp_md5( digest, m_systemKey, 32 );
    strKey.FromAscii( digest );
}

///////////////////////////////////////////////////////////////////////////////
// readXMLConfigurationGeneral
//
// Read the configuration XML file
//

bool CControlObject::readXMLConfigurationGeneral( wxString& strcfgfile )
{
    wxXmlDocument doc;

    wxString wxlogmsg = wxString::Format(_("Reading XML GENERAL configuration from [%s]\n"),
                                            (const char *)strcfgfile.c_str() );
    fprintf( stderr, "%s", (const char *)wxlogmsg.mbc_str() );

    if ( !doc.Load( strcfgfile ) ) {
        fprintf( stderr, "Can't load logfile. Is path correct?\n" );
        return false;
    }

    // start processing the XML file
    if (doc.GetRoot()->GetName().Lower() != _("vscpconfig")) {
        fprintf( stderr, "Can't read logfile. Maybe it is invalid!\n" );
        return false;
    }

    wxXmlNode *child = doc.GetRoot()->GetChildren();
    while (child) {

        if (child->GetName().Lower() == _("general")) {

            wxXmlNode *subchild = child->GetChildren();
            while ( subchild ) {

                if ( subchild->GetName().Lower() == _("security") ) {

                    m_admin_user = subchild->GetAttribute( _("user"), _("admin") );
                    m_admin_password = subchild->GetAttribute( _("password"),
                            _("450ADCE88F2FDBB20F3318B65E53CA4A;06D3311CC2195E80BE4F8EB12931BFEB5C630F6B154B2D644ABE29CEBDBFB545") );
                    m_admin_allowfrom = subchild->GetAttribute( _("allowfrom"), _("*") );
                    m_vscptoken = subchild->GetAttribute( _("vscptoken"),
                                                            _("Carpe diem quam minimum credula postero") );
                    wxString str = subchild->GetAttribute( _("vscpkey"),
                                                            _("A4A86F7D7E119BA3F0CD06881E371B989B33B6D606A863B633EF529D64544F8E") );
                    if ( str.Length() ) {
                        vscp_hexStr2ByteArray( m_systemKey, 32, str );
                    }

                }
                else if ( subchild->GetName().Lower() == _("loglevel") ) {

                    wxString str = subchild->GetNodeContent();
                    str.Trim();
                    str.Trim(false);
                    str.MakeUpper();

                    if ( str.IsSameAs(_("NONE"), false)) {
                        m_logLevel = DAEMON_LOGMSG_NONE;
                    }
                    else if ( str.IsSameAs(_("NORMAL"), false)) {
                        m_logLevel = DAEMON_LOGMSG_NORMAL;
                    }
                    else if ( str.IsSameAs(_("DEBUG"), false)) {
                        m_logLevel = DAEMON_LOGMSG_DEBUG;
                    }
                    else {
                        m_logLevel = vscp_readStringValue( str );
                        if ( m_logLevel > DAEMON_LOGMSG_DEBUG ) {
                            m_logLevel = DAEMON_LOGMSG_NORMAL;
                        }
                    }

                }
                else if ( subchild->GetName().Lower() == _("runasuser") ) {

                    m_runAsUser = subchild->GetNodeContent();
                    m_runAsUser.Trim();
                    m_runAsUser.Trim(false);
                    // Also assign to web user
                    m_web_run_as_user = m_runAsUser;
                }
                else if ( subchild->GetName().Lower() == _("logsyslog") ) {

                    wxString attribute = subchild->GetAttribute(_("enable"), _("true"));
                    attribute.MakeLower();
                    if (attribute.IsSameAs(_("false"), false)) {
                        m_bLogToSysLog = false;
                    }
                    else {
                        m_bLogToSysLog = true;
                    }

                }
                else if (subchild->GetName().Lower() == _("guid")) {

                    wxString str = subchild->GetNodeContent();
                    m_guid.getFromString(str);

                }
                else if ( subchild->GetName().Lower() == _( "servername" ) ) {

                    m_strServerName = subchild->GetNodeContent();

                }
                else if (subchild->GetName().Lower() == _("clientbuffersize")) {

                    wxString str = subchild->GetNodeContent();
                    m_maxItemsInClientReceiveQueue = vscp_readStringValue(str);

                }
                else if (subchild->GetName().Lower() == _("db_vscp_daemon")) {
                    wxString str = subchild->GetNodeContent().Trim();
                    if ( str.Length() ) {
                        m_path_db_vscp_daemon.Assign( str );
                    }
                }
                else if (subchild->GetName().Lower() == _("db_vscp_data")) {
                    wxString str = subchild->GetNodeContent().Trim();
                    if ( str.Length() ) {
                        m_path_db_vscp_data.Assign( str );
                    }
                }
                else if (subchild->GetName().Lower() == _("db_vscp_variable")) {
                    wxString str = subchild->GetNodeContent().Trim();
                    if ( str.Length() ) {
                        m_variables.m_dbFilename.Assign( str );
                    }
                }
                else if (subchild->GetName().Lower() == _("db_vscp_dm")) {
                    wxString str = subchild->GetNodeContent().Trim();
                    if ( str.Length() ) {
                        m_dm.m_path_db_vscp_dm.Assign( str );
                    }
                }
                else if (subchild->GetName().Lower() == _("db_vscp_log")) {
                    wxString str = subchild->GetNodeContent().Trim();
                    if ( str.Length() ) {
                        m_path_db_vscp_log.Assign( str );
                    }
                }
                else if ( subchild->GetName().Lower() == _("logdays") ) {
                    m_logDays = vscp_readStringValue( subchild->GetNodeContent() );                    
                }
                else if ( subchild->GetName().Lower() == _("debugflags1") ) {
                    m_debugFlags1 = vscp_readStringValue( subchild->GetNodeContent() );                    
                }
                else if ( subchild->GetName().Lower() == _("debugflags2") ) {
                    m_debugFlags2 = vscp_readStringValue( subchild->GetNodeContent() );                    
                }
                else if ( subchild->GetName().Lower() == _("debugflags3") ) {
                    m_debugFlags3 = vscp_readStringValue( subchild->GetNodeContent() );                    
                }
                else if ( subchild->GetName().Lower() == _("debugflags4") ) {
                    m_debugFlags4 = vscp_readStringValue( subchild->GetNodeContent() );                    
                }
                else if ( subchild->GetName().Lower() == _("debugflags5") ) {
                    m_debugFlags5 = vscp_readStringValue( subchild->GetNodeContent() );                    
                }
                else if ( subchild->GetName().Lower() == _("debugflags6") ) {
                    m_debugFlags6 = vscp_readStringValue( subchild->GetNodeContent() );                    
                }
                else if ( subchild->GetName().Lower() == _("debugflags7") ) {
                    m_debugFlags7 = vscp_readStringValue( subchild->GetNodeContent() );                    
                }
                else if ( subchild->GetName().Lower() == _("debugflags8") ) {
                    m_debugFlags8 = vscp_readStringValue( subchild->GetNodeContent() );                    
                }

                subchild = subchild->GetNext();

            } // while

            return true;    // We are done

        } // general

        child = child->GetNext();

    } // while

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// readConfigurationXML
//
// Read the configuration XML file
//

bool CControlObject::readConfigurationXML( wxString& strcfgfile )
{
    unsigned long val;
    wxString attribute;
    wxXmlDocument doc;

    wxString wxlogmsg = wxString::Format(_("Reading full XML configuration from [%s]\n"),
                                            (const char *)strcfgfile.c_str() );
    logMsg( wxlogmsg  );

    if ( !doc.Load( strcfgfile ) ) {
        logMsg(_("Can't load logfile. Is path correct?\n")  );
        return false;
    }

    // start processing the XML file
    if ( doc.GetRoot()->GetName().Lower() != _("vscpconfig") ) {
        logMsg(_("Can't read logfile. Maybe it is invalid!\n")  );
        return false;
    }

    wxXmlNode *child = doc.GetRoot()->GetChildren();
    while ( child ) {

        // The "general" settings are read in a pre-step (readXMLConfigurationGeneral)

        if ( child->GetName().Lower() == _("tcpip") ) {

            // Enable
            attribute = child->GetAttribute(_("enable"), _("true"));
            attribute.MakeLower();
            if (attribute.IsSameAs(_("false"), false)) {
                m_enableTcpip = false;
            }
            else {
                m_enableTcpip  = true;
            }

            m_strTcpInterfaceAddress = child->GetAttribute(_("interface"), m_strTcpInterfaceAddress );
            m_strTcpInterfaceAddress.StartsWith("tcp://", &m_strTcpInterfaceAddress );
            m_strTcpInterfaceAddress.Trim(true);
            m_strTcpInterfaceAddress.Trim(false);

            m_tcpip_ssl_certificate = child->GetAttribute(_("ssl_certificate"), m_tcpip_ssl_certificate );
            m_tcpip_ssl_certificate.Trim(true);
            m_tcpip_ssl_certificate.Trim(false);

            m_tcpip_ssl_certificate_chain = child->GetAttribute(_("ssl_verify_peer"), m_tcpip_ssl_certificate_chain );
            m_tcpip_ssl_certificate_chain.Trim(true);
            m_tcpip_ssl_certificate_chain.Trim(false);

            m_tcpip_ssl_verify_peer = vscp_readStringValue( child->GetAttribute( _("ssl_certificate_chain"), _("0") ) );

            m_tcpip_ssl_ca_path = child->GetAttribute(_("ssl_ca_path"), _(""));
            m_tcpip_ssl_ca_path.Trim(true);
            m_tcpip_ssl_ca_path.Trim(false);

            m_tcpip_ssl_ca_file = child->GetAttribute(_("ssl_ca_file"), _(""));
            m_tcpip_ssl_ca_file.Trim(true);
            m_tcpip_ssl_ca_file.Trim(false);

            m_tcpip_ssl_verify_depth = vscp_readStringValue( child->GetAttribute( _("ssl_verify_depth"), _("9") ) );

            attribute = child->GetAttribute(_("ssl_default_verify_paths"), _("true"));
            attribute.MakeLower();
            if (attribute.IsSameAs(_("false"), false)) {
                m_tcpip_ssl_default_verify_paths = false;
            }
            else {
                m_tcpip_ssl_default_verify_paths  = true;
            }

            m_tcpip_ssl_cipher_list = child->GetAttribute(_("ssl_cipher_list"), m_tcpip_ssl_cipher_list );
            m_tcpip_ssl_cipher_list.Trim(true);
            m_tcpip_ssl_cipher_list.Trim(false);

            if ( child->GetAttribute(_("ssl_protocol_version"), &attribute ) ) {
                m_tcpip_ssl_protocol_version = vscp_readStringValue( attribute ) ;
            }

            if ( child->GetAttribute(_("ssl_short_trust"), &attribute ) ) {
                attribute.MakeLower();
                if (attribute.IsSameAs(_("false"), false)) {
                    m_tcpip_ssl_short_trust = false;
                }
                else {
                    m_tcpip_ssl_short_trust  = true;
                }
            }
        }
        else if ( child->GetName().Lower() == _( "multicast-announce" ) ) {

            // Enable
            if ( child->GetAttribute(_("enable"), &attribute ) ) {
                attribute.MakeLower();
                if (attribute.IsSameAs(_("false"), false)) {
                    m_bMulticastAnounce = false;
                }
                else {
                    m_bMulticastAnounce  = true;
                }
            }

            // interface
            if ( child->GetAttribute(_("enable"), &attribute ) ) {
                m_strMulticastAnnounceAddress = child->GetAttribute( attribute );
            }

            // ttl
            if ( child->GetAttribute(_("ttl"), &attribute ) ) {
                m_ttlMultiCastAnnounce = vscp_readStringValue( attribute );
            }

        }
        else if (child->GetName().Lower() == _("udp")) {

            m_mutexUDPInfo.Lock();

            // Default is to let everything come through
            vscp_clearVSCPFilter( &m_udpInfo.m_filter );

            // Enable
            if ( child->GetAttribute(_("enable"), &attribute ) ) { 
                attribute.MakeLower();
                if (attribute.IsSameAs(_("false"), false)) {
                    m_udpInfo.m_bEnable = false;
                }
                else {
                    m_udpInfo.m_bEnable  = true;
                }
            }

            // Allow insecure connections
            if ( child->GetAttribute(_("bAllowUnsecure"), &attribute ) ) {
                if (attribute.Lower().IsSameAs(_("false"), false)) {
                    m_udpInfo.m_bAllowUnsecure = false;
                }
                else {
                    m_udpInfo.m_bAllowUnsecure  = true;
                }
            }

            // Enable ACK
            if ( child->GetAttribute(_("bSendAck"), &attribute ) ) {
                if (attribute.Lower().IsSameAs(_("false"), false)) {
                    m_udpInfo.m_bAck = false;
                }
                else {
                    m_udpInfo.m_bAck = true;
                }
            }

            // Username
            m_udpInfo.m_user = child->GetAttribute( _("user"), m_udpInfo.m_user );

            // Password
            m_udpInfo.m_password = child->GetAttribute( _("password"), m_udpInfo.m_password );

            // Interface
            m_udpInfo.m_interface = child->GetAttribute( _("interface"), m_udpInfo.m_interface );

            // GUID
            if ( child->GetAttribute(_("guid"), &attribute ) ) {
                m_udpInfo.m_guid.getFromString( attribute );
            }

            // Filter
            if ( child->GetAttribute(_("filter"), &attribute ) ) {
                if ( attribute.Trim().Length() ) {
                    vscp_readFilterFromString( &m_udpInfo.m_filter, attribute );
                }
            }

            // Mask
            if ( child->GetAttribute(_("mask"), &attribute ) ) {
                if ( attribute.Trim().Length() ) {
                    vscp_readMaskFromString( &m_udpInfo.m_filter, attribute );
                }
            }

            wxXmlNode *subchild = child->GetChildren();
            while ( subchild ) {

                if ( subchild->GetName().Lower() == _("rxnode") ) {

                    udpRemoteClientInfo *pudpClient = new udpRemoteClientInfo;
                    if ( NULL == pudpClient ) {
                        logMsg( _("Failed to allocated UDP client remote structure.\n") );
                        gpobj->m_mutexUDPInfo.Unlock();
                        subchild = subchild->GetNext();
                        continue;
                    }

                    // Default is to let everything come through
                    vscp_clearVSCPFilter( &pudpClient->m_filter );

                    if ( subchild->GetAttribute( _("enable"), &attribute ) ) { 
                        if ( attribute.Lower().IsSameAs(_("false"), false ) ) {
                            pudpClient->m_bEnable = false;
                        }
                        else {
                            pudpClient->m_bEnable = true;
                        }
                    }

                    if ( !pudpClient->m_bEnable ) {
                        delete pudpClient;
                        gpobj->m_mutexUDPInfo.Unlock();
                        subchild = subchild->GetNext();
                        continue;
                    }

                    // remote address
                    pudpClient->m_remoteAddress = subchild->GetAttribute( _("interface"), pudpClient->m_remoteAddress );

                    // Filter
                    if ( subchild->GetAttribute( _("filter"), &attribute ) ) { 
                        if ( attribute.Trim().Length() ) {
                            vscp_readFilterFromString( &pudpClient->m_filter, attribute );
                        }
                    }

                    // Mask
                    if ( subchild->GetAttribute( _("mask"), &attribute ) ) {
                        if ( attribute.Trim().Length() ) {
                            vscp_readMaskFromString( &pudpClient->m_filter, attribute );
                        }
                    }

                    // broadcast
                    if ( subchild->GetAttribute( _("bsetbroadcast"), &attribute ) ) {
                        if ( attribute.Lower().IsSameAs(_("false"), false ) ) {
                            pudpClient->m_bSetBroadcast = false;
                        }
                        else {
                            pudpClient->m_bSetBroadcast = true;
                        }
                    }

                    // encryption
                    if ( subchild->GetAttribute( _("encryption"), &attribute ) ) {
                        pudpClient->m_nEncryption = vscp_getEncryptionCodeFromToken( attribute );
                    }

                    // add to list
                    pudpClient->m_index = 0;
                    m_udpInfo.m_remotes.Append( pudpClient );

                }

                subchild = subchild->GetNext();

            } // while

            gpobj->m_mutexUDPInfo.Unlock();

        } // udp

        else if (child->GetName().Lower() == _("multicast")) {

            gpobj->m_mutexMulticastInfo.Lock();

            if ( child->GetAttribute( _("enable"), &attribute ) ) {
                attribute.MakeLower();
                if (attribute.IsSameAs(_("false"), false)) {
                    m_multicastInfo.m_bEnable = false;
                }
                else {
                    m_multicastInfo.m_bEnable = true;
                }
            }

            wxXmlNode *subchild = child->GetChildren();
            while ( subchild ) {

                if ( subchild->GetName().Lower() == _("channel") ) {

                    multicastChannelItem *pChannel = new multicastChannelItem;
                    if ( NULL == pChannel ) {
                        logMsg( _("Failed to allocated multicast channel structure.\n") );
                        gpobj->m_mutexMulticastInfo.Unlock();
                        subchild = subchild->GetNext();
                        continue;
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
                    if ( subchild->GetAttribute( _("enable"), &attribute ) ) {
                        attribute.MakeLower();
                        if (attribute.IsSameAs(_("false"), false)) {
                            pChannel->m_bEnable = false;
                        }
                        else {
                            pChannel->m_bEnable = true;
                        }
                    }

                    // bSendAck
                    if ( subchild->GetAttribute( _("bsendack"), &attribute ) ) {
                        attribute.MakeLower();
                        if (attribute.IsSameAs(_("false"), false)) {
                            pChannel->m_bSendAck = false;
                        }
                        else {
                            pChannel->m_bSendAck = true;
                        }
                    }

                    // bAllowUndsecure
                    if ( subchild->GetAttribute( _("ballowunsecure"), &attribute ) ) {
                        attribute.MakeLower();
                        if ( attribute.IsSameAs(_("false"), false) ) {
                            pChannel->m_bAllowUnsecure = false;
                        }
                        else {
                            pChannel->m_bAllowUnsecure = true;
                        }
                    }

                    // Interface
                    subchild->GetAttribute( _("public"), pChannel->m_public );

                    // Interface
                    if ( subchild->GetAttribute( _("ballowunsecure"), &attribute ) ) {
                        pChannel->m_port =
                                vscp_readStringValue( attribute );
                    }

                    // Group
                    subchild->GetAttribute( _("group"), pChannel->m_gropupAddress );

                    // ttl
                    if ( subchild->GetAttribute( _("ttl"), &attribute ) ) {
                        pChannel->m_ttl = vscp_readStringValue( attribute );
                    }

                    // guid
                    if ( subchild->GetAttribute( _("guid"), &attribute ) ) {                                                    
                        pChannel->m_guid.getFromString( attribute );
                    }

                    // TX Filter
                    if ( subchild->GetAttribute( _("txfilter"), &attribute ) ) {
                        if ( attribute.Trim().Length() ) {
                            vscp_readFilterFromString( &pChannel->m_txFilter, attribute );
                        }
                    }

                    // TX Mask
                    if ( subchild->GetAttribute( _("txmask"), &attribute ) ) {
                        if ( attribute.Trim().Length() ) {
                            vscp_readMaskFromString( &pChannel->m_txFilter, attribute );
                        }
                    }

                    // RX Filter
                    if ( subchild->GetAttribute( _("rxfilter"), &attribute ) ) {
                        if ( attribute.Trim().Length() ) {
                            vscp_readFilterFromString( &pChannel->m_rxFilter, attribute );
                        }
                    }

                    // RX Mask
                    if ( subchild->GetAttribute( _("rxmask"), &attribute ) ) {
                        if ( attribute.Trim().Length() ) {
                            vscp_readMaskFromString( &pChannel->m_rxFilter, attribute );
                        }
                    }

                    // encryption
                    if ( subchild->GetAttribute( _("encryption"), &attribute ) ) {
                        pChannel->m_nEncryption = vscp_getEncryptionCodeFromToken( attribute );
                    }

                    // add to list
                    pChannel->m_index = 0;
                    m_multicastInfo.m_channels.Append( pChannel );

                }

                subchild = subchild->GetNext();

            } // while

            gpobj->m_mutexMulticastInfo.Unlock();

        }

        else if (child->GetName().Lower() == _("dm")) {

            // Should the internal DM be disabled
            wxString attribut;

            // Get the path to the DM file  (Deprecated)
            if ( child->GetAttribute( _("path"), &attribute ) ) {
                if ( attribut.Length() ) {
                    m_dm.m_staticXMLPath = attribut;
                }
            }

            // Get the path to the DM file
            if ( child->GetAttribute( _("pathxml"), &attribute ) ) {
                if ( attribut.Length() ) {
                    m_dm.m_staticXMLPath = attribut;
                }
            }

            // Get the path to the DM db file
            if ( child->GetAttribute( _("pathdb"), &attribute ) ) {
                if ( attribut.Length() ) {
                    m_dm.m_path_db_vscp_dm.Assign( attribut );
                }
            }

            // Get the DM XML save flag
            if ( child->GetAttribute( _("allowxmlsave"), &attribute ) ) {
                attribute.MakeLower();
                if ( wxNOT_FOUND != attribute.Find( _("true") ) ) {
                    m_dm.bAllowXMLsave = true;
                }
            }

            // Get the DM loglevel
            if ( child->GetAttribute( _("loglevel"), &attribute ) ) {
                attribute.MakeLower();
                if ( wxNOT_FOUND != attribute.Find( _("debug") ) ) {
                    m_debugFlags1 |= VSCP_DEBUG1_DM;
                }
            }

        }

        else if (child->GetName().Lower() == _("variables")) {

            // Should the internal DM be disabled
            wxFileName fileName;

            // Get the path to the DM file
            if ( child->GetAttribute( _("path"), &attribute ) ) {
                fileName.Assign( attribute );
                if ( fileName.IsOk() ) {
                    m_variables.m_xmlPath = fileName.GetFullPath();
                }
            }

            if ( child->GetAttribute( _("pathxml"), &attribute ) ) {
                fileName.Assign( attribute );
                if ( fileName.IsOk() ) {
                    m_variables.m_xmlPath = fileName.GetFullPath();
                }
            }

            if ( child->GetAttribute( _("pathdb"), &attribute ) ) {
                fileName.Assign( attribute );
                if ( fileName.IsOk() ) {
                    m_variables.m_dbFilename = fileName;
                }
            }

        }

        else if (child->GetName().Lower() == _("webserver")) {

            wxString attribute;

            // Enable
            if ( child->GetAttribute( _("enable"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    attribute.MakeUpper();
                    if ( attribute.IsSameAs(_("FALSE"), false ) ) {
                        m_web_bEnable = false;
                    }
                    else {
                        m_web_bEnable = true;
                    }
                }
            }

            if ( child->GetAttribute( _("document_root"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_document_root = attribute;
                }
            }

            if ( child->GetAttribute( _("listening_ports"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_listening_ports = attribute;
                }
            }

            if ( child->GetAttribute( _("index_files"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_index_files = attribute;
                }
            }

            if ( child->GetAttribute( _("authentication_domain"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_authentication_domain = attribute;
                }
            }

            if ( child->GetAttribute( _("enable_auth_domain_check"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    attribute.MakeUpper();
                    if ( attribute.IsSameAs(_("FALSE"), false ) ) {
                        m_enable_auth_domain_check = false;
                    }
                    else {
                        m_enable_auth_domain_check = true;
                    }
                }
            }

            if ( child->GetAttribute( _("ssl_certificat"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_ssl_certificate = attribute;
                }
            }

            if ( child->GetAttribute( _("ssl_certificat_chain"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_ssl_certificate_chain = attribute;
                }
            }

            if ( child->GetAttribute( _("ssl_verify_peer"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    attribute.MakeUpper();
                    if ( attribute.IsSameAs(_("FALSE"), false ) ) {
                        m_web_ssl_verify_peer = false;
                    }
                    else {
                        m_web_ssl_verify_peer = true;
                    }
                }
            }

            if ( child->GetAttribute( _("ssl_ca_path"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_ssl_ca_path = attribute;
                }
            }

            if ( child->GetAttribute( _("ssl_ca_file"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_ssl_ca_file = attribute;
                }
            }

            if ( child->GetAttribute( _("ssl_verify_depth"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_ssl_verify_depth = atoi( (const char *)attribute.mbc_str() );
                }
            }

            if ( child->GetAttribute( _("ssl_default_verify_paths"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    attribute.MakeUpper();
                    if ( attribute.IsSameAs(_("FALSE"), false ) ) {
                        m_web_ssl_default_verify_paths = false;
                    }
                    else {
                        m_web_ssl_default_verify_paths = true;
                    }
                }
            }

            if ( child->GetAttribute( _("ssl_cipher_list"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_ssl_cipher_list = attribute.mbc_str();
                }
            }

            if ( child->GetAttribute( _("ssl_protcol_version"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_ssl_protocol_version = atoi( (const char *)attribute.mbc_str() );
                }
            }

            if ( child->GetAttribute( _("ssl_short_trust"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    attribute.MakeUpper();
                    if ( attribute.IsSameAs(_("FALSE"), false ) ) {
                        m_web_ssl_short_trust = false;
                    }
                    else {
                        m_web_ssl_short_trust = true;
                    }
                }
            }

            if ( child->GetAttribute( _("cgi_interpreter"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_cgi_interpreter = attribute;
                }
            }

            if ( child->GetAttribute( _("cgi_pattern"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_cgi_patterns =attribute;
                }
            }

            if ( child->GetAttribute( _("cgi_environment"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_cgi_environment = attribute;
                }
            }

            if ( child->GetAttribute( _("protect_uri"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_protect_uri = attribute;
                }
            }

            if ( child->GetAttribute( _("trottle"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_trottle = attribute;
                }
            }

            if ( child->GetAttribute( _("enable_directory_listing"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    attribute.MakeUpper();
                    if ( attribute.IsSameAs(_("FALSE"), false ) ) {
                        m_web_enable_directory_listing = false;
                    }
                    else {
                        m_web_enable_directory_listing = true;
                    }
                }
            }

            if ( child->GetAttribute( _("enable_keep_alive"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    attribute.MakeUpper();
                    if ( attribute.IsSameAs(_("FALSE"), false ) ) {
                        m_web_enable_keep_alive = false;
                    }
                    else {
                        m_web_enable_keep_alive = true;
                    }
                }
            }

            if ( child->GetAttribute( _("keep_alive_timeout_ms"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_keep_alive_timeout_ms = atol( (const char *)attribute.mbc_str() );
                }
            }

            if ( child->GetAttribute( _("access_control_list"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_access_control_list = attribute;
                }
            }

            if ( child->GetAttribute( _("extra_mime_types"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_extra_mime_types = attribute;
                }
            }

            if ( child->GetAttribute( _("num_threads"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_num_threads = atoi( (const char *)attribute.mbc_str() );
                }
            }

            if ( child->GetAttribute( _("hide_file_pattern"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_hide_file_patterns = attribute;
                }
            }

            if ( child->GetAttribute( _("run_as_user"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                m_web_run_as_user = attribute;
            }

            if ( child->GetAttribute( _("url_rewrite_patterns"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_url_rewrite_patterns = attribute;
                }
            }

            if ( child->GetAttribute( _("hide_file_patterns"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_hide_file_patterns = attribute;
                }
            }

            if ( child->GetAttribute( _("request_timeout_ms"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_request_timeout_ms = atol( (const char *)attribute.mbc_str() );
                }
            }

            if ( child->GetAttribute( _("linger_timeout_ms"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_linger_timeout_ms = atol( (const char *)attribute.mbc_str() );
                }
            }

            if ( child->GetAttribute( _("decode_url"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    attribute.MakeUpper();
                    if ( attribute.IsSameAs(_("FALSE"), false ) ) {
                        m_web_decode_url = false;
                    }
                    else {
                        m_web_decode_url = true;
                    }
                }
            }

            if ( child->GetAttribute( _("global_auth_file"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_global_auth_file = attribute;
                }
            }

            if ( child->GetAttribute( _("web_per_directory_auth_file"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_per_directory_auth_file = attribute;
                }
            }

            if ( child->GetAttribute( _("ssi_pattern"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_ssi_patterns = attribute;
                }
            }

            if ( child->GetAttribute( _("access_control_allow_origin"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_access_control_allow_origin = attribute;
                }
            }

            if ( child->GetAttribute( _("access_control_allow_methods"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_access_control_allow_methods = attribute;
                }
            }

            if ( child->GetAttribute( _("access_control_allow_headers"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_access_control_allow_headers = attribute;
                }
            }

            if ( child->GetAttribute( _("error_pages"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_error_pages = attribute;
                }
            }

            if ( child->GetAttribute( _("tcp_nodelay"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_linger_timeout_ms = atol( (const char *)attribute.mbc_str() );
                }
            }

            if ( child->GetAttribute( _("static_file_max_age"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_static_file_max_age = atol( (const char *)attribute.mbc_str() );
                }
            }

            if ( child->GetAttribute( _("strict_transport_security_max_age"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_strict_transport_security_max_age = atol( (const char *)attribute.mbc_str() );
                }
            }

            if ( child->GetAttribute( _("sendfile_call"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    attribute.MakeUpper();
                    if ( attribute.IsSameAs(_("FALSE"), false ) ) {
                        m_web_allow_sendfile_call = false;
                    }
                    else {
                        m_web_allow_sendfile_call = true;
                    }
                }
            }

            if ( child->GetAttribute( _("additional_headers"), &attribute ) ) {    
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_additional_header = attribute;
                }
            }

            if ( child->GetAttribute( _("max_request_size"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_max_request_size = atol( (const char *)attribute.mbc_str() );
                }
            }

            if ( child->GetAttribute( _("web_allow_index_script_resource"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    attribute.MakeUpper();
                    if ( attribute.IsSameAs(_("FALSE"), false ) ) {
                        m_web_allow_index_script_resource = false;
                    }
                    else {
                        m_web_allow_index_script_resource = true;
                    }
                }
            }

            if ( child->GetAttribute( _("duktape_script_patterns"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_duktape_script_patterns = attribute;
                }
            }

            if ( child->GetAttribute( _("lua_preload_file"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_lua_preload_file = attribute;
                }
            }

            if ( child->GetAttribute( _("lua_script_patterns"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_lua_script_patterns = attribute;
                }
            }

            if ( child->GetAttribute( _("lua_server_page_patterns"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_lua_server_page_patterns = attribute;
                }
            }

            if ( child->GetAttribute( _("lua_websockets_patterns"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_lua_websocket_patterns = attribute;
                }
            }

            if ( child->GetAttribute( _("lua_background_script"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_lua_background_script = attribute;
                }
            }

            if ( child->GetAttribute( _("lua_background_script_params"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_web_lua_background_script_params = attribute;
                }
            }

        }

        else if (child->GetName().Lower() == _("websockets")) {

            if ( child->GetAttribute( _("enable"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    attribute.MakeUpper();
                    if ( attribute.IsSameAs(_("FALSE"), false ) ) {
                        m_bWebsocketsEnable = false;
                    }
                    else {
                        m_bWebsocketsEnable = true;
                    }
                }
            }

            if ( child->GetAttribute( _("document_root"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_websocket_document_root = attribute;
                }
            }

            if ( child->GetAttribute( _("timeout_ms"), &attribute ) ) {
                attribute.Trim();
                attribute.Trim(false);
                if ( attribute.Length() ) {
                    m_websocket_timeout_ms = atol( (const char *)attribute.mbc_str() );
                }
            }

        }

        else if ( child->GetName().Lower() == _("remoteuser") ) {

            wxXmlNode *subchild = child->GetChildren();
            while ( subchild ) {

                vscpEventFilter VSCPFilter;
                bool bFilterPresent = false;
                bool bMaskPresent = false;
                wxString name;
                wxString md5;
                wxString privilege;
                wxString allowfrom;
                wxString allowevent;
                bool bUser = false;

                vscp_clearVSCPFilter( &VSCPFilter ); // Allow all frames

                if ( subchild->GetName().Lower() == _("user") ) {

                    wxXmlNode *subsubchild = subchild->GetChildren();

                    while (subsubchild) {

                        if (subsubchild->GetName().Lower() == _("name")) {
                            name = subsubchild->GetNodeContent();
                            bUser = true;
                        }
                        else if (subsubchild->GetName().Lower() == _("password")) {
                            md5 = subsubchild->GetNodeContent();
                        }
                        else if (subsubchild->GetName().Lower() == _("privilege")) {
                            privilege = subsubchild->GetNodeContent();
                        }
                        else if (subsubchild->GetName().Lower() == _("filter")) {
                            bFilterPresent = true;

                            wxString str_vscp_priority = subchild->GetAttribute(_("priority"), _("0"));

                            val = 0;
                            str_vscp_priority.ToULong(&val);
                            VSCPFilter.filter_priority = val;

                            wxString str_vscp_class = subchild->GetAttribute(_("class"), _("0"));

                            val = 0;
                            str_vscp_class.ToULong(&val);
                            VSCPFilter.filter_class = val;

                            wxString str_vscp_type = subchild->GetAttribute(_("type"), _("0"));

                            val = 0;
                            str_vscp_type.ToULong(&val);
                            VSCPFilter.filter_type = val;

                            wxString str_vscp_guid = subchild->GetAttribute(_("guid"),
                                                        _("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"));
                            vscp_getGuidFromStringToArray(VSCPFilter.filter_GUID, str_vscp_guid);
                        }
                        else if (subsubchild->GetName().Lower() == _("mask")) {

                            bMaskPresent = true;

                            wxString str_vscp_priority = subchild->GetAttribute(_("priority"), _("0"));

                            val = 0;
                            str_vscp_priority.ToULong(&val);
                            VSCPFilter.mask_priority = val;

                            wxString str_vscp_class = subchild->GetAttribute(_("class"), _("0"));

                            val = 0;
                            str_vscp_class.ToULong(&val);
                            VSCPFilter.mask_class = val;

                            wxString str_vscp_type = subchild->GetAttribute(_("type"), _("0"));

                            val = 0;
                            str_vscp_type.ToULong(&val);
                            VSCPFilter.mask_type = val;

                            wxString str_vscp_guid = subchild->GetAttribute(_("guid"),
                                    _("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"));
                            vscp_getGuidFromStringToArray(VSCPFilter.mask_GUID, str_vscp_guid);
                        }
                        else if (subsubchild->GetName().Lower() == _("allowfrom")) {
                            allowfrom = subsubchild->GetNodeContent();
                        }
                        else if (subsubchild->GetName().Lower() == _("allowevent")) {
                            allowevent = subsubchild->GetNodeContent();
                        }

                        subsubchild = subsubchild->GetNext();

                    }

                }

                // Add user
                if ( bUser ) {

                    if ( bFilterPresent && bMaskPresent ) {
                        m_userList.addUser( name,
                                                md5,
                                                _(""),
                                                _(""),
                                                &VSCPFilter,
                                                privilege,
                                                allowfrom,
                                                allowevent,
                                                VSCP_ADD_USER_FLAG_LOCAL );
                    }
                    else {
                        m_userList.addUser( name,
                                                md5,
                                                _(""),
                                                _(""),
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
        else if ( ( child->GetName().Lower() == _("canaldriver") ) || 
                  ( child->GetName().Lower() == _("level1driver") ) ) {

            wxXmlNode *subchild = child->GetChildren();
            while ( subchild ) {

                wxString strName;
                wxString strConfig;
                wxString strPath;
                unsigned long flags;
                cguid guid;
                bool bEnabled = false;
                bool bCanalDriver = false;

                if ( subchild->GetName().Lower() == _("driver") ) {

                    wxXmlNode *subsubchild = subchild->GetChildren();

                    wxString attribute = subchild->GetAttribute(_("enable"), _("false"));

                    if ( attribute.IsSameAs( _("true"), false ) ) {
                        bEnabled = true;
                    }

                    while (subsubchild) {

                        if ( subsubchild->GetName().Lower() == _("name") ) {
                            strName = subsubchild->GetNodeContent();
                            strName.Trim();
                            strName.Trim(false);

                            // Replace spaces in name with underscore
                            int pos;
                            while ( wxNOT_FOUND != (pos = strName.Find(_(" ") ) ) ) {
                                strName.SetChar(pos, wxChar('_'));
                            }
                            bCanalDriver = true;
                        }
                        else if ( subsubchild->GetName().Lower() == _("config") ||
                                subsubchild->GetName().Lower() == _("parameter") ) {
                            strConfig = subsubchild->GetNodeContent();
                            strConfig.Trim();
                            strConfig.Trim(false);
                        }
                        else if ( subsubchild->GetName().Lower() == _("path") ) {
                            strPath = subsubchild->GetNodeContent();
                            strPath.Trim();
                            strPath.Trim(false);
                        }
                        else if ( subsubchild->GetName().Lower() == _("flags") ) {
                            wxString str = subsubchild->GetNodeContent();
                            flags = vscp_readStringValue(str);
                        }
                        else if ( subsubchild->GetName().Lower() == _("guid") ) {
                            guid.getFromString( subsubchild->GetNodeContent() );
                        }
                        else if ( subsubchild->GetName().Lower() == _( "known-nodes" ) ) {

                            wxXmlNode *subsubsubchild = subchild->GetChildren();

                            while ( subsubsubchild ) {

                                if ( subsubsubchild->GetName().Lower() == _( "node" ) ) {
                                    cguid knownguid;

                                    knownguid.getFromString( subchild->GetAttribute( _( "guid" ), 
                                                                                     _( "-" ) ) );
                                    wxString name = subchild->GetAttribute( _( "name" ), 
                                                                            _( "" )  ) ;
                                    addKnownNode( knownguid, guid, name );
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
                        wxString errMsg = _("Level I driver not added. "
                                            "Path does not exist. - [ ") +
                                strPath + _(" ]\n");
                        logMsg( errMsg );
                    }
                    else {
                        wxString errMsg = _("Level I driver added. - [ ") +
                                strPath + _(" ]\n");
                        logMsg( errMsg );
                    }

                    bCanalDriver = false;

                }

                // Next driver
                subchild = subchild->GetNext();

            }

        }
        
        // Level II driver
        else if ( ( child->GetName().Lower() == _("vscpdriver") ) || 
                  ( child->GetName().Lower() == _("level2driver") ) ) {

            wxXmlNode *subchild = child->GetChildren();

            while ( subchild ) {

                wxString strName;
                wxString strConfig;
                wxString strPath;
                cguid guid;
                bool bEnabled = false;
                bool bLevel2Driver = false;

                if ( subchild->GetName().Lower() == _("driver") ) {

                    wxXmlNode *subsubchild = subchild->GetChildren();

                    wxString attribute = subchild->GetAttribute(_("enable"),
                                                                _("false"));

                    if (attribute.IsSameAs(_("true"), false)) {
                        bEnabled = true;
                    }

                    while ( subsubchild ) {

                        if (subsubchild->GetName().Lower() == _("name")) {

                            strName = subsubchild->GetNodeContent();
                            strName.Trim();
                            strName.Trim(false);

                            // Replace spaces in name with underscore
                            int pos;
                            while (wxNOT_FOUND != (pos = strName.Find(_(" ")))) {
                                strName.SetChar(pos, wxChar('_'));
                            }

                            bLevel2Driver = true;

                        }
                        else if (subsubchild->GetName().Lower() == _("config") ||
                                subsubchild->GetName().Lower() == _("parameter")) {

                            strConfig = subsubchild->GetNodeContent();
                            strConfig.Trim();
                            strConfig.Trim(false);

                        }
                        else if (subsubchild->GetName().Lower() == _("path")) {

                            strPath = subsubchild->GetNodeContent();
                            strPath.Trim();
                            strPath.Trim(false);

                        }
                        else if (subsubchild->GetName().Lower() == _("guid")) {
                            guid.getFromString( subsubchild->GetNodeContent() );
                        }
                        else if ( subsubchild->GetName().Lower() == _( "known-nodes" ) ) {

                            wxXmlNode *subsubsubchild = subchild->GetChildren();

                            while ( subsubsubchild ) {

                                if ( subsubsubchild->GetName().Lower() == _( "node" ) ) {
                                    cguid knownguid;

                                    knownguid.getFromString( subchild->GetAttribute( _( "guid" ), 
                                                                                     _( "-" ) ) );
                                    wxString name = 
                                            subchild->GetAttribute( _( "name" ), 
                                                                    _( "" ) );
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
                if ( bLevel2Driver && bEnabled ) {

                    if (!m_deviceList.addItem( strName,
                                                strConfig,
                                                strPath,
                                                0,
                                                guid,
                                                VSCP_DRIVER_LEVEL2,
                                                bEnabled )) {
                        wxString errMsg = _("Level II driver was not added. "
                                            "Path does not exist. - [ ") +
                                    strPath + _(" ]\n");
                        logMsg(errMsg);

                    }
                    else {
                        wxString errMsg = _("Level II driver added. - [ ") +
                                    strPath + _(" ]\n");
                        logMsg(errMsg);
                    }

                    bLevel2Driver = false;

                }

                // Next driver
                subchild = subchild->GetNext();

            }

        }
        // Level III driver
        else if ( child->GetName().Lower() == _("level3driver") ) {

            wxXmlNode *subchild = child->GetChildren();

            while ( subchild ) {

                wxString strName;
                wxString strConfig;
                wxString strPath;
                cguid guid;
                bool bEnabled = false;
                bool bLevel3Driver = false;

                if ( subchild->GetName().Lower() == _("driver") ) {

                    wxXmlNode *subsubchild = subchild->GetChildren();

                    wxString attribute = subchild->GetAttribute(_("enable"), 
                                                                _("false"));

                    if (attribute.IsSameAs(_("true"), false)) {
                        bEnabled = true;
                    }

                    while ( subsubchild ) {

                        if (subsubchild->GetName().Lower() == _("name")) {

                            strName = subsubchild->GetNodeContent();
                            strName.Trim();
                            strName.Trim(false);

                            // Replace spaces in name with underscore
                            int pos;
                            while (wxNOT_FOUND != (pos = strName.Find(_(" ")))) {
                                strName.SetChar(pos, wxChar('_'));
                            }

                            bLevel3Driver = true;

                        }
                        else if (subsubchild->GetName().Lower() == _("config") ||
                                subsubchild->GetName().Lower() == _("parameter")) {

                            strConfig = subsubchild->GetNodeContent();
                            strConfig.Trim();
                            strConfig.Trim(false);

                        }
                        else if (subsubchild->GetName().Lower() == _("path")) {

                            strPath = subsubchild->GetNodeContent();
                            strPath.Trim();
                            strPath.Trim(false);

                        }
                        else if (subsubchild->GetName().Lower() == _("guid")) {
                            guid.getFromString( subsubchild->GetNodeContent() );
                        }
                        else if ( subsubchild->GetName().Lower() == 
                                    _( "known-nodes" ) ) {

                            wxXmlNode *subsubsubchild = subchild->GetChildren();

                            while ( subsubsubchild ) {

                                if ( subsubsubchild->GetName().Lower() == _( "node" ) ) {
                                    cguid knownguid;

                                    knownguid.getFromString( subchild->GetAttribute( _( "guid" ), 
                                                                                    _( "-" ) ) );
                                    wxString name = 
                                            subchild->GetAttribute( _( "name" ), 
                                                                    _( "" ) );
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
                        wxString errMsg = _("Level III driver was not added. Path does not exist. - [ ") +
                                    strPath + _(" ]\n");
                        logMsg(errMsg);

                    }
                    else {
                        wxString errMsg = _("Level III driver added. - [ ") +
                                    strPath + _(" ]\n");
                        logMsg(errMsg);
                    }

                    bLevel3Driver = false;

                }

                // Next driver
                subchild = subchild->GetNext();

            }

        }
        // Tables
        else if ( child->GetName().Lower() == _("tables") ) {

            wxXmlNode *subchild = child->GetChildren();
            while ( subchild ) {

                if ( subchild->GetName().Lower() == _("table") ) {

                    // Check if enabled
                    wxString strEnabled = subchild->GetAttribute( _("enable"), _("true") );
                    strEnabled.MakeUpper();
                    if ( wxNOT_FOUND != strEnabled.Find( _("FALSE") ) ) {
                        goto xml_table_error;
                    }

                    // Get name of table
                    wxString name = subchild->GetAttribute( _("name"), _("") );;

                    // Get type of table
                    vscpTableType type = VSCP_TABLE_DYNAMIC;
                    int size = 0;
                    wxString attribute = subchild->GetAttribute( _("type"), _("0") );
                    attribute.MakeUpper();
                    if ( wxNOT_FOUND  != attribute.Find(_("DYNAMIC") ) ) {
                        type = VSCP_TABLE_DYNAMIC;
                    }
                    else if ( wxNOT_FOUND  != attribute.Find( _("STATIC") ) ) {

                        type = VSCP_TABLE_STATIC;

                        // Get size
                        size = vscp_readStringValue( subchild->GetAttribute( _("size"), _("0") ) );

                    }
                    else {
                        // Invalid type
                        logMsg( _("Reading VSCP table xml info: Invalid table type (static/dynamic)!\n") );
                        goto xml_table_error;
                    }

                    // Should it be created in memory?
                    bool bMemory = false;
                    wxString strInMemory = subchild->GetAttribute( _("bmemory"), _("false") );
                    strInMemory.MakeUpper();
                    if ( wxNOT_FOUND != strInMemory.Find( _("TRUE") ) ) {
                        bMemory = true;
                    }

                    CVSCPTable *pTable = new CVSCPTable( m_rootFolder + _("table/"), name, true, bMemory, type, size );
                    if ( NULL != pTable ) {

                        if ( !pTable->setTableInfo( subchild->GetAttribute( _("owner"), _("admin") ),
                                                vscp_readStringValue( subchild->GetAttribute( _("rights"), _("0x700") ) ),
                                                subchild->GetAttribute( _("title"), _("") ),
                                                subchild->GetAttribute( _("labelx"), _("") ),
                                                subchild->GetAttribute( _("labely"), _("") ),
                                                subchild->GetAttribute( _("note"), _("") ),
                                                subchild->GetAttribute( _("sqlcreate"), _("") ),
                                                subchild->GetAttribute( _("sqlinsert"), _("") ),
                                                subchild->GetAttribute( _("sqldelete"), _("") ),
                                                subchild->GetAttribute( _("description"), _("") ) ) ) {
                            logMsg(_("Reading table xml info: Could not set table info!\n"));
                            delete pTable;
                            goto xml_table_error;
                        }

                        pTable->setTableEventInfo( vscp_readStringValue( subchild->GetAttribute( _("vscpclass"), _("0") ) ),
                                vscp_readStringValue( subchild->GetAttribute( _("vscptype"), _("0") ) ),
                                vscp_readStringValue( subchild->GetAttribute( _("vscpsensorindex"), _("0") ) ),
                                vscp_readStringValue( subchild->GetAttribute( _("vscpunit"), _("0") ) ),
                                vscp_readStringValue( subchild->GetAttribute( _("vscpzone"), _("255") ) ),
                                vscp_readStringValue( subchild->GetAttribute( _("vscpsubzone"), _("255") ) )
                        );

                        if ( !m_userTableObjects.addTable( pTable ) ) {
                            delete pTable;
                            logMsg(_("Reading table xml info: Could not add new table (name conflict?)!\n"));
                        };

                        // Add to database if not there already
                        /*if ( !m_userTableObjects.isTableInDB( *pTable ) ) {
                            m_userTableObjects.addTableToDB( *pTable );
                        }
                        else {
                            // Update
                            m_userTableObjects.updateTableToDB( *pTable );
                        }*/

                    }
                    else {
                        logMsg( _("Reading table xml info: Unable to create table class!") );
                    }

                }

xml_table_error:

                subchild = subchild->GetNext();

            } // while

        }
        else if ( child->GetName().Lower() == _("automation") ) {

            wxString attribut = child->GetAttribute(_("enable"), _("true"));
            attribut.MakeLower();
            if (attribut.IsSameAs(_("false"), false)) {
                m_automation.disableAutomation();
            }
            else {
                m_automation.enableAutomation();
            }

            wxXmlNode *subchild = child->GetChildren();
            while (subchild) {

                if (subchild->GetName().Lower() == _("zone")) {

                    long zone;
                    wxString strZone = subchild->GetNodeContent();
                    strZone.ToLong( &zone );
                    m_automation.setZone( zone );

                }
                else if (subchild->GetName().Lower() == _("sub-zone")) {

                    long subzone;
                    wxString strSubZone = subchild->GetNodeContent();
                    strSubZone.ToLong( &subzone );
                    m_automation.setSubzone( subzone );

                }
                else if (subchild->GetName().Lower() == _("longitude")) {

                    wxString strLongitude = subchild->GetNodeContent();
                    double d;
                    strLongitude.ToDouble( &d );
                    m_automation.setLongitude( d );

                }
                else if (subchild->GetName().Lower() == _("latitude")) {

                    wxString strLatitude = subchild->GetNodeContent();
                    double d;
                    strLatitude.ToDouble( &d );
                    m_automation.setLatitude( d );

                }
                else if (subchild->GetName().Lower() == _("sunrise")) {

                    m_automation.enableSunRiseEvent();
                    wxString attribute = subchild->GetAttribute(_("enable"), _("true"));
                    if (attribute.IsSameAs(_("false"), false)) {
                        m_automation.disableSunRiseEvent();
                    }


                }

                else if (subchild->GetName().Lower() == _("sunrise-twilight")) {

                    m_automation.enableSunRiseTwilightEvent();
                    wxString attribute = subchild->GetAttribute(_("enable"), _("true"));
                    if (attribute.IsSameAs(_("false"), false)) {
                        m_automation.disableSunRiseTwilightEvent();
                    }


                }
                else if (subchild->GetName().Lower() == _("sunset")) {

                    m_automation.enableSunSetEvent();
                    wxString attribute = subchild->GetAttribute(_("enable"), _("true"));
                    if (attribute.IsSameAs(_("false"), false)) {
                        m_automation.disableSunSetEvent();
                    }

                }
                else if (subchild->GetName().Lower() == _("sunset-twilight")) {

                    m_automation.enableSunSetTwilightEvent();
                    wxString attribute = subchild->GetAttribute(_("enable"), _("true"));
                    if (attribute.IsSameAs(_("false"), false)) {
                        m_automation.disableSunSetTwilightEvent();
                    }

                }
                else if (subchild->GetName().Lower() == _("segmentcontrol-event")) {

                    m_automation.enableSegmentControllerHeartbeat();
                    wxString attribute = subchild->GetAttribute(_("enable"), _("true"));
                    if (attribute.IsSameAs(_("false"), false)) {
                        m_automation.disableSegmentControllerHeartbeat();
                    }

                    attribute = subchild->GetAttribute(_("interval"), _("60"));
                    long interval;
                    attribute.ToLong( &interval );
                    m_automation.setSegmentControllerHeartbeatInterval( interval );

                }
                else if (subchild->GetName().Lower() == _("heartbeat-event")) {

                    m_automation.enableHeartbeatEvent();
                    m_automation.setHeartbeatEventInterval( 30 );

                    wxString attribute = subchild->GetAttribute(_("enable"), _("true"));
                    if (attribute.IsSameAs(_("false"), false)) {
                        m_automation.disableHeartbeatEvent();
                    }

                    attribute = subchild->GetAttribute(_("interval"), _("30"));
                    long interval;
                    attribute.ToLong( &interval );
                    m_automation.setHeartbeatEventInterval( interval );

                }
                else if (subchild->GetName().Lower() == _("capabilities-event")) {

                    m_automation.enableCapabilitiesEvent();
                    m_automation.setCapabilitiesEventInterval( 60 );

                    wxString attribute = subchild->GetAttribute(_("enable"), _("true"));
                    if (attribute.IsSameAs(_("false"), false)) {
                        m_automation.disableCapabilitiesEvent();
                    }

                    attribute = subchild->GetAttribute(_("interval"), _("30"));
                    long interval;
                    attribute.ToLong( &interval );
                    m_automation.setHeartbeatEventInterval( interval );

                }

                subchild = subchild->GetNext();

            } // while

        }

        child = child->GetNext();

    }

    return true;
} // XML config

///////////////////////////////////////////////////////////////////////////////
// isDbTableExist
//

bool CControlObject::isDbTableExist( sqlite3 *db,
                                          const wxString& strTblName  )
{
    sqlite3_stmt *pSelectStatement = NULL;
    int iResult = SQLITE_ERROR;
    bool rv = false;
    
    // Database file must be open
    if ( NULL == db ) {
        logMsg( _("isDbFieldExistent. Database file is not open.\n") );
        return false;
    }
 
    wxString sql = "SELECT name FROM sqlite_master WHERE type='table' AND name='%s'";
    sql = wxString::Format( sql, (const char *)strTblName.mbc_str() );
    
    iResult = sqlite3_prepare16_v2( db, 
                                    (const char *)sql, 
                                    -1, 
                                    &pSelectStatement, 
                                    0 );
    
    if ( ( iResult == SQLITE_OK ) && 
         ( pSelectStatement != NULL ) ) {                   
        
        iResult = sqlite3_step( pSelectStatement );

        // was found?
        if ( iResult == SQLITE_ROW ) {
            rv = true;
            sqlite3_clear_bindings( pSelectStatement );
            sqlite3_reset( pSelectStatement );
        }
    
        iResult = sqlite3_finalize( pSelectStatement );
    }
    
    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// isDbFieldExist
//

bool CControlObject::isDbFieldExist( sqlite3 *db,
                                          const wxString& strTblName,
                                          const wxString& strFieldName )
{
    bool rv = false;
    sqlite3_stmt *ppStmt;
    char *pErrMsg = 0;
    
    // Database file must be open
    if ( NULL == db ) {
        logMsg( _("isDbFieldExist. Database file is not open.\n") );
        return false;
    }
 
    wxString sql = "PRAGMA table_info(%s);";
    sql = wxString::Format( sql, (const char *)strTblName.mbc_str() );
    
    if ( SQLITE_OK != sqlite3_prepare( m_db_vscp_daemon,
                                        (const char *)sql.mbc_str(),
                                        -1,
                                        &ppStmt,
                                        NULL ) ) {
        logMsg( "isDbFieldExist: Failed to read VSCP settings database - prepare query." );
        return false;
    }

    while ( SQLITE_ROW  == sqlite3_step( ppStmt ) ) {
        
        const unsigned char *p;
        
        // Get column name
        if ( NULL == ( p = sqlite3_column_text( ppStmt, 1 ) ) ) {
            continue;
        }

        // database version
        if ( vscp_strcasecmp( (const char *)p, 
                                (const char *)strFieldName.mbc_str() ) ) {
            rv = true;
            break;
        }
        
    }
    
    sqlite3_finalize( ppStmt );
    
    return rv;
}


///////////////////////////////////////////////////////////////////////////////
// updateConfigurationRecordItem
//

bool CControlObject::updateConfigurationRecordItem( const wxString& strName,
                                                        const wxString& strValue )
{
    char *pErrMsg;

    // Database file must be open
    if ( NULL == m_db_vscp_daemon ) {
        logMsg( _("Settings update: Update record. Database file is not open.\n") );
        return false;
    }

    m_db_vscp_configMutex.Lock();

    char *sql = sqlite3_mprintf( VSCPDB_CONFIG_UPDATE_ITEM,
                                    (const char *)strValue.mbc_str(),
                                    (const char *)strName.mbc_str(),
                                    m_nConfiguration );
    if ( SQLITE_OK != sqlite3_exec( m_db_vscp_daemon,
                                            sql, NULL, NULL, &pErrMsg)) {
        sqlite3_free( sql );
        m_db_vscp_configMutex.Unlock();
        fprintf( stderr,
                    "Failed to update setting with error %s.\n",
                    pErrMsg );
        return false;
    }

    sqlite3_free( sql );

    m_db_vscp_configMutex.Unlock();

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// addConfigurationValueToDatabase
//

bool CControlObject::addConfigurationValueToDatabase( const char *pName,
                                                        const char *pValue )
{
    sqlite3_stmt *ppStmt;
    char *pErrMsg = 0;
    char *psql;

    // Check if database is open
    if ( NULL == m_db_vscp_daemon ) return false;
    
    // Check if the variable is defined already
    //      if it is - just return true
    psql = sqlite3_mprintf( VSCPDB_CONFIG_FIND_ITEM, pName );
    if ( SQLITE_OK != sqlite3_prepare( m_db_vscp_daemon,
                                        psql,
                                        -1,
                                        &ppStmt,
                                        NULL ) ) {
        sqlite3_free( psql );
        fprintf( stderr, 
                    "Failed to check if %s = %s is already in configuration database\n", 
                    pName, 
                    pValue );
        return false; 
    }

    sqlite3_free( psql );
    
    if ( SQLITE_ROW  == sqlite3_step( ppStmt ) ) {
        return true; // Record is there already
    }
   
    m_db_vscp_configMutex.Lock();
    
    fprintf( stderr, "Add %s = %s to configuration database\n", pName, pValue );

    // Create settings in db
    psql = sqlite3_mprintf( VSCPDB_CONFIG_INSERT, pName, pValue );

    if ( SQLITE_OK != sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg ) ) {

        sqlite3_free( psql );
        m_db_vscp_configMutex.Unlock();

        fprintf( stderr,
                    "Inserting new entry into configuration database failed with message %s",
                    pErrMsg );
        return false;
    }

    sqlite3_free( psql );
    m_db_vscp_configMutex.Unlock();
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// addDefaultConfigValues
//

void CControlObject::addDefaultConfigValues( void ) 
{
    // Add default settings (set as defaults in SQL create expression))
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_DBVERSION, VSCPDB_CONFIG_DEFAULT_DBVERSION );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_CLIENTBUFFERSIZE, VSCPDB_CONFIG_DEFAULT_CLIENTBUFFERSIZE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_GUID, VSCPDB_CONFIG_DEFAULT_GUID );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_SERVERNAME, VSCPDB_CONFIG_DEFAULT_SERVERNAME );    
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_ANNOUNCE_ADDR, VSCPDB_CONFIG_DEFAULT_ANNOUNCE_ADDR );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_ANNOUNCE_TTL, VSCPDB_CONFIG_DEFAULT_ANNOUNCE_TTL );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_PATH_DB_DATA, VSCPDB_CONFIG_DEFAULT_PATH_DB_DATA );

    // UDP
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_UDP_ENABLE, VSCPDB_CONFIG_DEFAULT_UDP_ENABLE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_UDP_ADDR, VSCPDB_CONFIG_DEFAULT_UDP_ADDR );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_UDP_USER, VSCPDB_CONFIG_DEFAULT_UDP_USER );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_UDP_PASSWORD, VSCPDB_CONFIG_DEFAULT_UDP_PASSWORD );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_UDP_UNSECURE_ENABLE, VSCPDB_CONFIG_DEFAULT_UDP_UNSECURE_ENABLE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_UDP_FILTER, VSCPDB_CONFIG_DEFAULT_UDP_FILTER );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_UDP_MASK, VSCPDB_CONFIG_DEFAULT_UDP_MASK );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_UDP_GUID, VSCPDB_CONFIG_DEFAULT_UDP_GUID );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_UDP_ACK_ENABLE, VSCPDB_CONFIG_DEFAULT_UDP_ACK_ENABLE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_MULTICAST_ENABLE, VSCPDB_CONFIG_DEFAULT_MULTICAST_ENABLE );

    // TCP/IP
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_TCPIP_ADDR, VSCPDB_CONFIG_DEFAULT_TCPIP_ADDR );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_TCPIP_ENCRYPTION, VSCPDB_CONFIG_DEFAULT_TCPIP_ENCRYPTION );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_TCPIP_SSL_CERTIFICATE, VSCPDB_CONFIG_DEFAULT_TCPIP_SSL_CERTIFICATE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_TCPIP_SSL_CERTIFICAT_CHAIN, VSCPDB_CONFIG_DEFAULT_TCPIP_SSL_CERTIFICAT_CHAIN );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_TCPIP_SSL_VERIFY_PEER, VSCPDB_CONFIG_DEFAULT_TCPIP_SSL_VERIFY_PEER );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_TCPIP_SSL_CA_PATH, VSCPDB_CONFIG_DEFAULT_TCPIP_SSL_CA_PATH );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_TCPIP_SSL_CA_FILE, VSCPDB_CONFIG_DEFAULT_TCPIP_SSL_CA_FILE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_TCPIP_SSL_VERIFY_DEPTH, VSCPDB_CONFIG_DEFAULT_TCPIP_SSL_VERIFY_DEPTH );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_TCPIP_SSL_DEFAULT_VERIFY_PATHS, VSCPDB_CONFIG_DEFAULT_TCPIP_SSL_DEFAULT_VERIFY_PATHS );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_TCPIP_SSL_CHIPHER_LIST, VSCPDB_CONFIG_DEFAULT_TCPIP_SSL_CHIPHER_LIST );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_TCPIP_SSL_PROTOCOL_VERSION, VSCPDB_CONFIG_DEFAULT_TCPIP_SSL_PROTOCOL_VERSION );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_TCPIP_SSL_SHORT_TRUST, VSCPDB_CONFIG_DEFAULT_TCPIP_SSL_SHORT_TRUST );

    // DM
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_DM_PATH_DB, VSCPDB_CONFIG_DEFAULT_DM_PATH_DB );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_DM_PATH_XML, VSCPDB_CONFIG_DEFAULT_DM_PATH_XML );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_DM_ALLOW_XML_SAVE, VSCPDB_CONFIG_DEFAULT_DM_ALLOW_XML_SAVE );

    // Variables
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_VARIABLES_PATH_DB, VSCPDB_CONFIG_DEFAULT_VARIABLES_PATH_DB );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_VARIABLES_PATH_XML, VSCPDB_CONFIG_DEFAULT_VARIABLES_PATH_XML );

    // WEB server
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_ENABLE, VSCPDB_CONFIG_DEFAULT_WEB_ENABLE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_DOCUMENT_ROOT , VSCPDB_CONFIG_DEFAULT_WEB_DOCUMENT_ROOT );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_LISTENING_PORTS, VSCPDB_CONFIG_DEFAULT_WEB_LISTENING_PORTS );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_INDEX_FILES, VSCPDB_CONFIG_DEFAULT_WEB_INDEX_FILES );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_AUTHENTICATION_DOMAIN, VSCPDB_CONFIG_DEFAULT_WEB_AUTHENTICATION_DOMAIN );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_ENABLE_AUTH_DOMAIN_CHECK, VSCPDB_CONFIG_DEFAULT_WEB_ENABLE_AUTH_DOMAIN_CHECK );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_SSL_CERTIFICATE, VSCPDB_CONFIG_DEFAULT_WEB_SSL_CERTIFICATE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_SSL_CERTIFICAT_CHAIN, VSCPDB_CONFIG_DEFAULT_WEB_SSL_CERTIFICAT_CHAIN );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_SSL_VERIFY_PEER, VSCPDB_CONFIG_DEFAULT_WEB_SSL_VERIFY_PEER );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_SSL_CA_PATH, VSCPDB_CONFIG_DEFAULT_WEB_SSL_CA_PATH );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_SSL_CA_FILE, VSCPDB_CONFIG_DEFAULT_WEB_SSL_CA_FILE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_SSL_VERIFY_DEPTH, VSCPDB_CONFIG_DEFAULT_WEB_SSL_VERIFY_DEPTH );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_SSL_DEFAULT_VERIFY_PATHS, VSCPDB_CONFIG_DEFAULT_WEB_SSL_DEFAULT_VERIFY_PATHS );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_SSL_CHIPHER_LIST, VSCPDB_CONFIG_DEFAULT_WEB_SSL_CHIPHER_LIST );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_SSL_PROTOCOL_VERSION, VSCPDB_CONFIG_DEFAULT_WEB_SSL_PROTOCOL_VERSION );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_SSL_SHORT_TRUST, VSCPDB_CONFIG_DEFAULT_WEB_SSL_SHORT_TRUST );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_CGI_PATTERN, VSCPDB_CONFIG_DEFAULT_WEB_CGI_PATTERN );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_CGI_INTERPRETER, VSCPDB_CONFIG_DEFAULT_WEB_CGI_INTERPRETER );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_CGI_ENVIRONMENT, VSCPDB_CONFIG_DEFAULT_WEB_CGI_ENVIRONMENT );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_PROTECT_URI, VSCPDB_CONFIG_DEFAULT_WEB_PROTECT_URI );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_TROTTLE, VSCPDB_CONFIG_DEFAULT_WEB_TROTTLE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_ENABLE_DIRECTORY_LISTING, VSCPDB_CONFIG_DEFAULT_WEB_ENABLE_DIRECTORY_LISTING );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_ENABLE_KEEP_ALIVE, VSCPDB_CONFIG_DEFAULT_WEB_ENABLE_KEEP_ALIVE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_LIST, VSCPDB_CONFIG_DEFAULT_WEB_ACCESS_CONTROL_LIST );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_EXTRA_MIME_TYPES, VSCPDB_CONFIG_DEFAULT_WEB_EXTRA_MIME_TYPES );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_NUM_THREADS, VSCPDB_CONFIG_DEFAULT_WEB_NUM_THREADS );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_HIDE_FILE_PATTERNS, VSCPDB_CONFIG_DEFAULT_WEB_HIDE_FILE_PATTERNS );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_RUN_AS_USER, VSCPDB_CONFIG_DEFAULT_WEB_RUN_AS_USER );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_URL_REWRITE_PATTERNS, VSCPDB_CONFIG_DEFAULT_WEB_URL_REWRITE_PATTERNS );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_REQUEST_TIMEOUT_MS, VSCPDB_CONFIG_DEFAULT_WEB_REQUEST_TIMEOUT_MS );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_LINGER_TIMEOUT_MS, VSCPDB_CONFIG_DEFAULT_WEB_LINGER_TIMEOUT_MS );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_DECODE_URL, VSCPDB_CONFIG_DEFAULT_WEB_DECODE_URL );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_GLOBAL_AUTHFILE, VSCPDB_CONFIG_DEFAULT_WEB_GLOBAL_AUTHFILE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_PER_DIRECTORY_AUTH_FILE, VSCPDB_CONFIG_DEFAULT_WEB_PER_DIRECTORY_AUTH_FILE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_SSI_PATTERNS, VSCPDB_CONFIG_DEFAULT_WEB_SSI_PATTERNS );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_ALLOW_ORIGIN, VSCPDB_CONFIG_DEFAULT_WEB_ACCESS_CONTROL_ALLOW_ORIGIN );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_ALLOW_METHODS, VSCPDB_CONFIG_DEFAULT_WEB_ACCESS_CONTROL_ALLOW_METHODS );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_ALLOW_HEADERS, VSCPDB_CONFIG_DEFAULT_WEB_ACCESS_CONTROL_ALLOW_HEADERS );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_ERROR_PAGES, VSCPDB_CONFIG_DEFAULT_WEB_ERROR_PAGES );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_TCP_NO_DELAY, VSCPDB_CONFIG_DEFAULT_WEB_TCP_NO_DELAY );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_STATIC_FILE_MAX_AGE, VSCPDB_CONFIG_DEFAULT_WEB_STATIC_FILE_MAX_AGE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_STRICT_TRANSPORT_SECURITY_MAX_AGE, VSCPDB_CONFIG_DEFAULT_WEB_STRICT_TRANSPORT_SECURITY_MAX_AGE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_ALLOW_SENDFILE_CALL, VSCPDB_CONFIG_DEFAULT_WEB_ALLOW_SENDFILE_CALL );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_ADDITIONAL_HEADERS, VSCPDB_CONFIG_DEFAULT_WEB_ADDITIONAL_HEADERS );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_MAX_REQUEST_SIZE, VSCPDB_CONFIG_DEFAULT_WEB_MAX_REQUEST_SIZE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_ALLOW_INDEX_SCRIPT_RESOURCE, VSCPDB_CONFIG_DEFAULT_WEB_ALLOW_INDEX_SCRIPT_RESOURCE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_DUKTAPE_SCRIPT_PATTERN, VSCPDB_CONFIG_DEFAULT_WEB_DUKTAPE_SCRIPT_PATTERN );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_LUA_PRELOAD_FILE, VSCPDB_CONFIG_DEFAULT_WEB_LUA_PRELOAD_FILE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_LUA_SCRIPT_PATTERN, VSCPDB_CONFIG_DEFAULT_WEB_LUA_SCRIPT_PATTERN );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_LUA_SERVER_PAGE_PATTERN, VSCPDB_CONFIG_DEFAULT_WEB_LUA_SERVER_PAGE_PATTERN );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_LUA_WEBSOCKET_PATTERN, VSCPDB_CONFIG_DEFAULT_WEB_LUA_WEBSOCKET_PATTERN );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_LUA_BACKGROUND_SCRIPT, VSCPDB_CONFIG_DEFAULT_WEB_LUA_BACKGROUND_SCRIPT );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_LUA_BACKGROUND_SCRIPT_PARAMS, VSCPDB_CONFIG_DEFAULT_WEB_LUA_BACKGROUND_SCRIPT_PARAMS );

    // Websockets
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEBSOCKET_ENABLE, VSCPDB_CONFIG_DEFAULT_WEBSOCKET_ENABLE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEBSOCKET_DOCUMENT_ROOT, VSCPDB_CONFIG_DEFAULT_WEBSOCKET_DOCUMENT_ROOT );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEBSOCKET_TIMEOUT_MS, VSCPDB_CONFIG_DEFAULT_WEBSOCKET_TIMEOUT_MS );

    // Automation
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_AUTOMATION_ENABLE, VSCPDB_CONFIG_DEFAULT_AUTOMATION_ENABLE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_AUTOMATION_ZONE, VSCPDB_CONFIG_DEFAULT_AUTOMATION_ZONE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_AUTOMATION_SUBZONE, VSCPDB_CONFIG_DEFAULT_AUTOMATION_SUBZONE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_AUTOMATION_LONGITUDE, VSCPDB_CONFIG_DEFAULT_AUTOMATION_LONGITUDE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_AUTOMATION_LATITUDE, VSCPDB_CONFIG_DEFAULT_AUTOMATION_LATITUDE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_AUTOMATION_SUNRISE_ENABLE, VSCPDB_CONFIG_DEFAULT_AUTOMATION_SUNRISE_ENABLE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_AUTOMATION_SUNSET_ENABLE, VSCPDB_CONFIG_DEFAULT_AUTOMATION_SUNSET_ENABLE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_AUTOMATION_SUNSET_TWILIGHT_ENABLE, VSCPDB_CONFIG_DEFAULT_AUTOMATION_SUNSET_TWILIGHT_ENABLE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_AUTOMATION_SUNRISE_TWILIGHT_ENABLE, VSCPDB_CONFIG_DEFAULT_AUTOMATION_SUNRISE_TWILIGHT_ENABLE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_AUTOMATION_SEGMENT_CTRL_ENABLE, VSCPDB_CONFIG_DEFAULT_AUTOMATION_SEGMENT_CTRL_ENABLE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_AUTOMATION_SEGMENT_CTRL_INTERVAL, VSCPDB_CONFIG_DEFAULT_AUTOMATION_SEGMENT_CTRL_INTERVAL );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_AUTOMATION_HEARTBEAT_ENABLE, VSCPDB_CONFIG_DEFAULT_AUTOMATION_HEARTBEAT_ENABLE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_AUTOMATION_CAPABILITIES_ENABLE, VSCPDB_CONFIG_DEFAULT_AUTOMATION_CAPABILITIES_ENABLE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_AUTOMATION_CAPABILITIES_INTERVAL, VSCPDB_CONFIG_DEFAULT_AUTOMATION_CAPABILITIES_INTERVAL );
    
}

///////////////////////////////////////////////////////////////////////////////
// doCreateConfiguration
//
// Create configuration table.
//
// Note that fprintf needs to be used here as the logging mechanism
// is not activated yet.
//

bool CControlObject::doCreateConfigurationTable( void )
{
    char *pErrMsg = 0;
    const char *psql;

    fprintf( stderr, "Creating settings database..\n" );

    // Check if database is open
    if ( NULL == m_db_vscp_daemon ) return false;

    m_db_vscp_configMutex.Lock();

    // Create settings db
    psql = VSCPDB_CONFIG_CREATE;
    if ( SQLITE_OK  !=  sqlite3_exec( m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg ) ) {
        fprintf( stderr,
                    "Creation of the VSCP settings database failed with message %s",
                    pErrMsg );
        return false;
    }

    // Create name index
    psql = VSCPDB_CONFIG_CREATE_INDEX;
    if ( SQLITE_OK  !=  sqlite3_exec( m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg ) ) {
        m_db_vscp_configMutex.Unlock();
        fprintf( stderr,
                    "Creation of the VSCP settings index failed with message %s",
                    pErrMsg );
        return false;
    }

    fprintf( stderr, "Writing default configuration database content..\n" );
    addDefaultConfigValues();
    
    m_db_vscp_configMutex.Unlock();

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// readConfigurationDB
//
// Read the configuration database record
//
// Note that fprintf needs to be used here as the logging mechanism
// is not activated yet.
//

bool CControlObject::readConfigurationDB( void )
{
    char *pErrMsg = 0;
    const char *psql = VSCPDB_CONFIG_FIND_ALL;
    sqlite3_stmt *ppStmt;
    int dbVersion = 0;

    // Check if database is open
    if ( NULL == m_db_vscp_daemon ) {
        fprintf( stderr,
                    "dbReadConfiguration: Failed to read VSCP settings database - Database is not open." );
        return false;
    }

    if ( SQLITE_OK != sqlite3_prepare( m_db_vscp_daemon,
                                        psql,
                                        -1,
                                        &ppStmt,
                                        NULL ) ) {
        fprintf( stderr,
                    "dbReadConfiguration: Failed to read VSCP settings database - prepare query." );
        return false;
    }

    while ( SQLITE_ROW  == sqlite3_step( ppStmt ) ) {

        const unsigned char *pName  = NULL;
        const unsigned char *pValue = NULL;

        if ( NULL == ( pName = sqlite3_column_text( ppStmt,
                        VSCPDB_ORDINAL_CONFIG_NAME ) ) ) {
            fprintf( stderr,
                        "dbReadConfiguration: Failed to read 'name' from settings record." );
            continue;
        }

        if ( NULL == ( pValue = sqlite3_column_text( ppStmt,
                        VSCPDB_ORDINAL_CONFIG_VALUE ) ) ) {
            fprintf( stderr,
                        "dbReadConfiguration: Failed to read 'value' from settings record." );
            continue;
        }

        // database version
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_DBVERSION ) ) {
            dbVersion = atoi( (const char *)pValue );
            continue;
        }

        // client buffer size
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_CLIENTBUFFERSIZE ) ) {
            m_maxItemsInClientReceiveQueue = atol( (const char *)pValue );
            continue;
        }

        // Server GUID
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_GUID ) ) {
            m_guid.getFromString( (const char *)pValue );
            continue;
        }

        // Server name
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_SERVERNAME ) ) {
            m_strServerName = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // TCP/IP interface address
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_TCPIP_ADDR ) ) {
            m_strTcpInterfaceAddress = wxString::FromUTF8( (const char *)pValue );
            m_strTcpInterfaceAddress.StartsWith("tcp://", &m_strTcpInterfaceAddress );
            if ( m_strTcpInterfaceAddress.StartsWith("ssl://", &m_strTcpInterfaceAddress ) ) {
                m_strTcpInterfaceAddress += "s";
            }
            m_strTcpInterfaceAddress.Trim(true);
            m_strTcpInterfaceAddress.Trim(false);
            continue;
        }

        // TCP/IP encryption
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_TCPIP_ENCRYPTION ) ) {
            m_encryptionTcpip = atoi( (const char *)pValue );
            continue;
        }

        // TCP/IP SSL certificat
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_TCPIP_SSL_CERTIFICATE ) ) {
            m_tcpip_ssl_certificate = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // TCP/IP SSL certificat chain
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_TCPIP_SSL_CERTIFICAT_CHAIN ) ) {
            m_tcpip_ssl_certificate_chain = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // TCP/IP SSL verify peer
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_TCPIP_SSL_VERIFY_PEER ) ) {
            m_tcpip_ssl_verify_peer = atoi( (const char *)pValue );
            continue;
        }

        // TCP/IP SSL CA path
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_TCPIP_SSL_CA_PATH ) ) {
            m_tcpip_ssl_ca_path = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // TCP/IP SSL CA file
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_TCPIP_SSL_CA_FILE ) ) {
            m_tcpip_ssl_ca_file = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // TCP/IP SSL verify depth
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_TCPIP_SSL_VERIFY_DEPTH ) ) {
            m_tcpip_ssl_verify_depth = atoi( (const char *)pValue );
            continue;
        }

        // TCP/IP SSL verify paths
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_TCPIP_SSL_DEFAULT_VERIFY_PATHS ) ) {
            m_tcpip_ssl_default_verify_paths = atoi( (const char *)pValue ) ? true : false;
            continue;
        }

        // TCP/IP SSL Chipher list
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_TCPIP_SSL_CHIPHER_LIST ) ) {
            m_tcpip_ssl_cipher_list = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // TCP/IP SSL protocol version
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_TCPIP_SSL_PROTOCOL_VERSION ) ) {
            m_tcpip_ssl_protocol_version = atoi( (const char *)pValue );
            continue;
        }

        // TCP/IP SSL short trust
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_TCPIP_SSL_SHORT_TRUST ) ) {
            m_tcpip_ssl_short_trust = atoi( (const char *)pValue ) ? true : false;
            continue;
        }

        // Announce multicast interface address
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_ANNOUNCE_ADDR )  ) {
            m_strMulticastAnnounceAddress = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // TTL for the multicast i/f
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_ANNOUNCE_TTL )  ) {
            m_ttlMultiCastAnnounce = atoi( (const char *)pValue );
            continue;
        }

        // Enable UDP interface
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_UDP_ENABLE )  ) {
            gpobj->m_mutexUDPInfo.Lock();
            m_udpInfo.m_bEnable = atoi( (const char *)pValue ) ? true : false;
            gpobj->m_mutexUDPInfo.Unlock();
            continue;
        }
        
        // UDP interface address/port
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_UDP_ADDR )  ) {
            gpobj->m_mutexUDPInfo.Lock();
            m_udpInfo.m_interface = wxString::FromUTF8( (const char *)pValue );
            gpobj->m_mutexUDPInfo.Unlock();
            continue;
        }

        // UDP User
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_UDP_USER )  ) {
            gpobj->m_mutexUDPInfo.Lock();
            m_udpInfo.m_user = wxString::FromUTF8( (const char *)pValue );
            gpobj->m_mutexUDPInfo.Unlock();
            continue;
        }

        // UDP User Password
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_UDP_PASSWORD )  ) {
            gpobj->m_mutexUDPInfo.Lock();
            m_udpInfo.m_password = 
                    wxString::FromUTF8( (const char *)pValue );
            gpobj->m_mutexUDPInfo.Unlock();
            continue;
        }

        // UDP un-secure enable
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_UDP_UNSECURE_ENABLE )  ) {
            gpobj->m_mutexUDPInfo.Lock();
            m_udpInfo.m_bAllowUnsecure = 
                    atoi( (const char *)pValue ) ? true : false;
            gpobj->m_mutexUDPInfo.Unlock();
            continue;
        }

        // UDP Filter
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_UDP_FILTER )  ) {
            gpobj->m_mutexUDPInfo.Lock();
            vscp_readFilterFromString( &m_udpInfo.m_filter, 
                                        wxString::FromUTF8( (const char *)pValue ) );
            gpobj->m_mutexUDPInfo.Unlock();
            continue;
        }

        // UDP Mask
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_UDP_MASK )  ) {
            gpobj->m_mutexUDPInfo.Lock();
            vscp_readMaskFromString( &m_udpInfo.m_filter, 
                                        wxString::FromUTF8( (const char *)pValue ) );
            gpobj->m_mutexUDPInfo.Unlock();
            continue;
        }

        // UDP GUID
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_UDP_GUID )  ) {
            gpobj->m_mutexUDPInfo.Lock();
            m_udpInfo.m_guid.getFromString( (const char *)pValue );
            gpobj->m_mutexUDPInfo.Unlock();
            continue;
        }

        // UDP Enable ACK
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_UDP_ACK_ENABLE )  ) {
            gpobj->m_mutexUDPInfo.Lock();
            m_udpInfo.m_bAck = atoi( (const char *)pValue ) ? true : false;
            gpobj->m_mutexUDPInfo.Unlock();
            continue;
        }

        // Enable Multicast interface
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_MULTICAST_ENABLE )  ) {
            m_multicastInfo.m_bEnable = 
                    atoi( (const char *)pValue ) ? true : false;
            continue;        
        }

        // Path to DM database file
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_DM_PATH_DB )  ) {
            m_dm.m_path_db_vscp_dm.Assign( wxString::FromUTF8( (const char *)pValue ) );
            continue;
        }

        // Path to DM XML file
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_DM_PATH_XML )  ) {
            m_dm.m_staticXMLPath = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // Path to variable database
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_VARIABLES_PATH_DB )  ) {
            m_variables.m_dbFilename.Assign( wxString::FromUTF8( (const char *)pValue ) );
            continue;
        }

        // Path to variable XML
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_VARIABLES_PATH_XML )  ) {
            m_variables.m_xmlPath = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // VSCP data database path
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_PATH_DB_DATA )  ) {
            m_path_db_vscp_data.Assign( wxString::FromUTF8( (const char *)pValue ) );
            continue;
        }


        // * * * WEB server * * *


        // Web server enable
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_ENABLE )  ) {
            if ( atoi( (const char *)pValue ) ) {
                m_web_bEnable = true;
            }
            else {
                m_web_bEnable = false;
            }
            continue;
        }

        // Web server document root
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_DOCUMENT_ROOT )  ) {
            m_web_document_root = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // listening ports for web server
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_LISTENING_PORTS )  ) {
            m_web_listening_ports = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // Index files
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_INDEX_FILES )  ) {
            m_web_index_files = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // Authdomain
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_AUTHENTICATION_DOMAIN )  ) {
            m_web_authentication_domain = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // Enable authdomain check
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_ENABLE_AUTH_DOMAIN_CHECK )  ) {
            if ( atoi( (const char *)pValue ) ) {
                m_enable_auth_domain_check = true;
            }
            else {
                m_enable_auth_domain_check = false;
            }
            continue;
        }

        // Path to cert file
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_SSL_CERTIFICATE )  ) {
            m_web_ssl_certificate = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // SSL certificate chain
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_SSL_CERTIFICAT_CHAIN )  ) {
            m_web_ssl_certificate_chain = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // SSL verify peer
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_SSL_VERIFY_PEER )  ) {
            if ( atoi( (const char *)pValue ) ) {
                m_web_ssl_verify_peer = true;
            }
            else {
                m_web_ssl_verify_peer = false;
            }
            continue;
        }

        // SSL CA path
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_SSL_CA_FILE )  ) {
            m_web_ssl_ca_path = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // SSL CA file
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_SSL_CA_FILE )  ) {
            m_web_ssl_ca_file = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // SSL verify depth
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_SSL_VERIFY_DEPTH )  ) {
            m_web_ssl_verify_depth = atoi( (const char *)pValue );
            continue;
        }

        // SSL default verify path
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_SSL_DEFAULT_VERIFY_PATHS )  ) {
            if ( atoi( (const char *)pValue ) ) {
                m_web_ssl_default_verify_paths = true;
            }
            else {
                m_web_ssl_default_verify_paths = false;
            }
            continue;
        }

        // SSL chipher list
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_SSL_CHIPHER_LIST )  ) {
            m_web_ssl_cipher_list = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // SSL protocol version
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_SSL_PROTOCOL_VERSION )  ) {
            m_web_ssl_protocol_version = atoi( (const char *)pValue );
            continue;
        }

        // SSL short trust
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_SSL_SHORT_TRUST )  ) {
            if ( atoi( (const char *)pValue ) ) {
                m_web_ssl_short_trust = true;
            }
            else {
                m_web_ssl_short_trust = false;
            }
            continue;
        }

        // CGI interpreter
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_CGI_INTERPRETER )  ) {
            m_web_cgi_interpreter = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // CGI pattern
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_CGI_PATTERN )  ) {
            m_web_cgi_patterns = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // CGI environment
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_CGI_ENVIRONMENT )  ) {
            m_web_cgi_environment = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // Protect URI
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_PROTECT_URI )  ) {
            m_web_protect_uri = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // Web trottle
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_TROTTLE )  ) {
            m_web_trottle = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // Enable directory listings
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_ENABLE_DIRECTORY_LISTING )  ) {
            if ( atoi( (const char *)pValue ) ) {
                m_web_enable_directory_listing = true;
            }
            else {
                m_web_enable_directory_listing = false;
            }
            continue;
        }

        // Enable keep alive
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_ENABLE_KEEP_ALIVE )  ) {
            if ( atoi( (const char *)pValue ) ) {
                m_web_enable_keep_alive = true;
            }
            else {
                m_web_enable_keep_alive = false;
            }
            continue;
        }

        // Keep alive timout ms
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_KEEP_ALIVE_TIMEOUT_MS )  ) {
            m_web_keep_alive_timeout_ms = atol( (const char *)pValue );
            continue;
        }

        // IP ACL
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_LIST )  ) {
            m_web_access_control_list = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // Extra mime types
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_EXTRA_MIME_TYPES )  ) {
            m_web_extra_mime_types = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // Number of threads
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_NUM_THREADS )  ) {
            m_web_num_threads = atoi( (const char *)pValue );
            continue;
        }

        // Hide file patterns
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_HIDE_FILE_PATTERNS )  ) {
            m_web_hide_file_patterns = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // Run as user
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_RUN_AS_USER )  ) {
            m_web_run_as_user = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // URL rewrites
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_URL_REWRITE_PATTERNS )  ) {
            m_web_url_rewrite_patterns = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // Hide file patterns
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_HIDE_FILE_PATTERNS )  ) {
            m_web_hide_file_patterns = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // web request timout
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_REQUEST_TIMEOUT_MS )  ) {
            m_web_request_timeout_ms = atol( (const char *)pValue );
            continue;
        }

        // web linger timout
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_LINGER_TIMEOUT_MS )  ) {
            m_web_linger_timeout_ms = atol( (const char *)pValue );
            continue;
        }

        // Decode URL
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_DECODE_URL )  ) {
            if ( atoi( (const char *)pValue ) ) {
                m_web_decode_url = true;
            }
            else {
                m_web_decode_url = false;
            }
            continue;
        }

        // Global auth. file
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_GLOBAL_AUTHFILE )  ) {
            m_web_global_auth_file = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // Per directory auth. file
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_PER_DIRECTORY_AUTH_FILE )  ) {
            m_web_per_directory_auth_file = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // SSI patterns
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_SSI_PATTERNS )  ) {
            m_web_ssi_patterns = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // Access control allow origin
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_ALLOW_ORIGIN )  ) {
            m_web_access_control_allow_origin = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // Access control allow methods
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_ALLOW_METHODS )  ) {
            m_web_access_control_allow_methods = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // Access control alow heraders
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_ALLOW_HEADERS )  ) {
            m_web_access_control_allow_headers = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // Error pages
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_ERROR_PAGES )  ) {
            m_web_error_pages = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // TCP no delay
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_TCP_NO_DELAY )  ) {
            m_web_tcp_nodelay = atol( (const char *)pValue );
            continue;
        }

        // File max age
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_STATIC_FILE_MAX_AGE )  ) {
            m_web_static_file_max_age = atol( (const char *)pValue );
            continue;
        }

        // Transport security max age
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_STRICT_TRANSPORT_SECURITY_MAX_AGE )  ) {
            m_web_strict_transport_security_max_age = atol( (const char *)pValue );
            continue;
        }

        // Enable sendfile call
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_ALLOW_SENDFILE_CALL )  ) {
            if ( atoi( (const char *)pValue ) ) {
                m_web_allow_sendfile_call = true;
            }
            else {
                m_web_allow_sendfile_call = false;
            }
            continue;
        }

        // Additional headers
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_ADDITIONAL_HEADERS )  ) {
            m_web_additional_header = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // Max request size
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_MAX_REQUEST_SIZE )  ) {
            m_web_max_request_size = atol( (const char *)pValue );
            continue;
        }

        // Allow index script resource
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_ALLOW_INDEX_SCRIPT_RESOURCE )  ) {
            if ( atoi( (const char *)pValue ) ) {
                m_web_allow_index_script_resource = true;
            }
            else {
                m_web_allow_index_script_resource = false;
            }
            continue;
        }

        // Duktape script patterns
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_DUKTAPE_SCRIPT_PATTERN )  ) {
            m_web_duktape_script_patterns = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // Lua preload file
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_LUA_PRELOAD_FILE )  ) {
            m_web_lua_preload_file = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // Lua script patterns
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_LUA_SCRIPT_PATTERN )  ) {
            m_web_lua_script_patterns = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // Lua server page patterns
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_LUA_SERVER_PAGE_PATTERN )  ) {
            m_web_lua_server_page_patterns = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // Lua websocket patterns
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_LUA_WEBSOCKET_PATTERN )  ) {
            m_web_lua_websocket_patterns = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // Lua background script
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_LUA_BACKGROUND_SCRIPT )  ) {
            m_web_lua_background_script = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // Lua background script params
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEB_LUA_BACKGROUND_SCRIPT_PARAMS )  ) {
            m_web_lua_background_script_params = wxString::FromUTF8( (const char *)pValue );
            continue;
        }


        // * * * Websockets * * *


        // Web server enable
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEBSOCKET_ENABLE )  ) {
            if ( atoi( (const char *)pValue ) ) {
                m_bWebsocketsEnable = true;
            }
            else {
                m_bWebsocketsEnable = false;
            }
            continue;
        }

        // Document root for websockets
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEBSOCKET_DOCUMENT_ROOT )  ) {
            m_websocket_document_root = wxString::FromUTF8( (const char *)pValue );
            continue;
        }

        // Websocket timeout
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_WEBSOCKET_TIMEOUT_MS )  ) {
            m_websocket_timeout_ms = atol( (const char *)pValue );
            continue;
        }

        // * * * Automation * * *


        // Enable automation
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_AUTOMATION_ENABLE ) ) {

            if ( atoi( (const char *)pValue ) ) {
                m_automation.enableAutomation();
            }
            else {
                m_automation.enableAutomation();
            }
            continue;
        }

        // Automation zone
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_AUTOMATION_ZONE ) ) {
            m_automation.setZone( atoi( (const char *)pValue ) );
            continue;
        }

        // Automation sub zone
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_AUTOMATION_SUBZONE ) ) {
            m_automation.setSubzone( atoi( (const char *)pValue ) );
            continue;
        }

        // Automation longitude
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_AUTOMATION_LONGITUDE ) ) {
            m_automation.setLongitude( atof( (const char *)pValue ) );
            continue;
        }

        // Automation latitude
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_AUTOMATION_LATITUDE ) ) {
            m_automation.setLatitude( atof( (const char *)pValue ) );
            continue;
        }

        // Automation enable sun rise event
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_AUTOMATION_SUNRISE_ENABLE ) ) {
            if ( atoi( (const char *)pValue ) ) {
                m_automation.enableSunRiseEvent();
            }
            else {
                m_automation.disableSunRiseEvent();
            }
            continue;
        }

        // Automation enable sun set event
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_AUTOMATION_SUNSET_ENABLE ) ) {
            if ( atoi( (const char *)pValue ) ) {
                m_automation.enableSunSetEvent();
            }
            else {
                m_automation.disableSunSetEvent();
            }
            continue;
        }

        // Automation enable sunset twilight event
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_AUTOMATION_SUNSET_TWILIGHT_ENABLE ) ) {
            if ( atoi( (const char *)pValue ) ) {
                m_automation.enableSunSetTwilightEvent();
            }
            else {
                m_automation.disableSunSetTwilightEvent();
            }
            continue;
        }

        // Automation enable sunrise twilight event
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_AUTOMATION_SUNRISE_TWILIGHT_ENABLE ) ) {
            if ( atoi( (const char *)pValue ) ) {
                m_automation.enableSunRiseTwilightEvent();
            }
            else {
                m_automation.disableSunRiseTwilightEvent();
            }
            continue;
        }

        // Automation segment controller event enable
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_AUTOMATION_SEGMENT_CTRL_ENABLE ) ) {
            if ( atoi( (const char *)pValue ) ) {
                m_automation.enableSegmentControllerHeartbeat();
            }
            else {
                m_automation.disableSegmentControllerHeartbeat();
            }
            continue;
        }

        // Automation, segment controller heartbeat interval
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_AUTOMATION_SEGMENT_CTRL_INTERVAL ) ) {
            m_automation.setSegmentControllerHeartbeatInterval( atol( (const char *)pValue ) );
            continue;
        }


        // Automation heartbeat event enable
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_AUTOMATION_HEARTBEAT_ENABLE ) ) {
            if ( atoi( (const char *)pValue ) ) {
                m_automation.enableHeartbeatEvent();
            }
            else {
                m_automation.disableHeartbeatEvent();
            }
            continue;
        }

        // Automation heartbeat interval
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_AUTOMATION_HEARTBEAT_INTERVAL ) ) {
            m_automation.setHeartbeatEventInterval( atol( (const char *)pValue ) );
            continue;
        }

        // Automation capabilities event enable
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_AUTOMATION_CAPABILITIES_ENABLE ) ) {
            if ( atoi( (const char *)pValue ) ) {
                m_automation.enableCapabilitiesEvent();
            }
            else {
                m_automation.disableCapabilitiesEvent();
            }
            continue;
        }

        // Automation capabilities interval
        if ( !vscp_strcasecmp( (const char *)pName,
                        VSCPDB_CONFIG_NAME_AUTOMATION_CAPABILITIES_INTERVAL ) ) {
            m_automation.setCapabilitiesEventInterval( atol( (const char *)pValue ) );
            continue;
        }

    
    }

    sqlite3_finalize( ppStmt );

    return true;
}




///////////////////////////////////////////////////////////////////////////////
// readUdpNodes
//
// Read in defined UDP nodes
//
//

bool CControlObject::readUdpNodes( void )
{
    char *pErrMsg = 0;
    const char *psql = "SELECT * FROM udpnode";
    sqlite3_stmt *ppStmt;

    // If UDP is disabled we are done
    if ( !m_udpInfo.m_bEnable ) return true;

    // Check if database is open
    if ( NULL == m_db_vscp_daemon ) {
        fprintf( stderr,
                    "readUdpNodes: Database is not open." );
        return false;
    }

    if ( SQLITE_OK != sqlite3_prepare( m_db_vscp_daemon,
                                        psql,
                                        -1,
                                        &ppStmt,
                                        NULL ) ) {
        fprintf( stderr, "readUdpNodes: prepare query." );
        return false;
    }

    while ( SQLITE_ROW  == sqlite3_step( ppStmt ) ) {

        const unsigned char * p;

        // If not enabled move on
        if ( !sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_UDPNODE_ENABLE ) ) continue;

        gpobj->m_mutexUDPInfo.Lock();

        udpRemoteClientInfo *pudpClient = new udpRemoteClientInfo;
        if ( NULL == pudpClient ) {
            fprintf( stderr, "readUdpNodes: Failed to allocate storage for UDP node." );
            gpobj->m_mutexUDPInfo.Unlock();
            continue;
        }

        // Broadcast
        pudpClient->m_bSetBroadcast = false;
        if ( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_UDPNODE_SET_BROADCAST ) ) {
            pudpClient->m_bSetBroadcast = true;
        }

        // Interface
        p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_UDPNODE_INTERFACE );
        if ( NULL != p ) {
            pudpClient->m_remoteAddress = wxString::FromUTF8Unchecked( (const char *)p );
        }

        //  Filter
        p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_UDPNODE_FILTER );
        if ( NULL != p ) {
            wxString wxstr = wxString::FromUTF8Unchecked( (const char *)p );
            if ( !vscp_readFilterFromString( &pudpClient->m_filter, wxstr ) ) {
                fprintf( stderr, "readUdpNodes: Failed to set filter for UDP node." );
            }
        }

        // Mask
        p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_UDPNODE_MASK );
        if ( NULL != p ) {
            wxString wxstr = wxString::FromUTF8Unchecked( (const char *)p );
            if ( !vscp_readMaskFromString( &pudpClient->m_filter, wxstr ) ) {
                fprintf( stderr, "readUdpNodes: Failed to set mask for UDP node." );
            }
        }

        // Encryption
        p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_UDPNODE_ENCRYPTION );
        if ( NULL != p ) {
            wxString wxstr = wxString::FromUTF8Unchecked( (const char *)p );
            pudpClient->m_nEncryption = vscp_getEncryptionCodeFromToken( wxstr );
        }

        // Add to list
        pudpClient->m_index = 0;
        m_udpInfo.m_remotes.Append( pudpClient );

        gpobj->m_mutexUDPInfo.Unlock();

    }

    sqlite3_finalize( ppStmt );

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// readMulticastChannels
//
// Read in defined multicast channels
//
//

bool CControlObject::readMulticastChannels( void )
{
    char *pErrMsg = 0;
    const char *psql = "SELECT * FROM multicast";
    sqlite3_stmt *ppStmt;

    // If multicast is disabled we are done
    if ( !m_multicastInfo.m_bEnable ) return true;

    // Check if database is open
    if ( NULL == m_db_vscp_daemon ) {
        fprintf( stderr,
                    "readMulticastChannels: Database is not open." );
        return false;
    }

    if ( SQLITE_OK != sqlite3_prepare( m_db_vscp_daemon,
                                        psql,
                                        -1,
                                        &ppStmt,
                                        NULL ) ) {
        fprintf( stderr, "readMulticastChannels: prepare query." );
        return false;
    }

    while ( SQLITE_ROW  == sqlite3_step( ppStmt ) ) {

        const unsigned char * p;

        // If not enabled move on
        if ( !sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_MULTICAST_ENABLE ) ) continue;

        gpobj->m_mutexMulticastInfo.Lock();

        multicastChannelItem *pChannel = new multicastChannelItem;
        if ( NULL == pChannel ) {
            fprintf( stderr, "readMulticastChannels: Failed to allocate storage for multicast node." );
            m_mutexMulticastInfo.Unlock();
            continue;
        }

        // Default is to let everything come through
        vscp_clearVSCPFilter( &pChannel->m_txFilter );
        vscp_clearVSCPFilter( &pChannel->m_rxFilter );

        // public interface
        p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_MULTICAST_PUBLIC );
        if ( NULL != p ) {
            pChannel->m_public =
                    wxString::FromUTF8Unchecked( (const char *)p );
        }

        // Port
        p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_MULTICAST_PORT );

        // group
        p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_MULTICAST_GROUP );
        if ( NULL != p ) {
            pChannel->m_gropupAddress =
                    wxString::FromUTF8Unchecked( (const char *)p );
        }

        // ttl
        pChannel->m_ttl = 
                sqlite3_column_int( ppStmt, 
                                    VSCPDB_ORDINAL_MULTICAST_TTL );

        // bAck
        pChannel->m_bSendAck = 
                sqlite3_column_int( ppStmt, 
                                VSCPDB_ORDINAL_MULTICAST_SENDACK ) ? true : false;

        // Allow unsecure
        pChannel->m_bAllowUnsecure = 
                sqlite3_column_int( ppStmt, 
                                VSCPDB_ORDINAL_MULTICAST_ALLOW_UNSECURE ) ? true : false;

        // GUID
        p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_MULTICAST_GUID );
        if ( NULL != p ) {
            pChannel->m_guid.getFromString( p );
        }

        //  TX Filter
        p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_MULTICAST_TXFILTER );
        if ( NULL != p ) {
            wxString wxstr = wxString::FromUTF8Unchecked( (const char *)p );
            if ( !vscp_readFilterFromString( &pChannel->m_txFilter, wxstr ) ) {
                fprintf( stderr, 
                          "readMulticastChannels: Failed to set TX "
                          "filter for multicast channel." );
            }
        }

        // TX Mask
        p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_MULTICAST_TXMASK );
        if ( NULL != p ) {
            wxString wxstr = wxString::FromUTF8Unchecked( (const char *)p );
            if ( !vscp_readMaskFromString( &pChannel->m_txFilter, wxstr ) ) {
                fprintf( stderr, 
                           "readMulticastChannels: Failed to set TX "
                           "mask for multicast channel." );
            }
        }

        //  RX Filter
        p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_MULTICAST_RXFILTER );
        if ( NULL != p ) {
            wxString wxstr = wxString::FromUTF8Unchecked( (const char *)p );
            if ( !vscp_readFilterFromString( &pChannel->m_rxFilter, wxstr ) ) {
                fprintf( stderr, 
                            "readMulticastChannels: Failed to set RX "
                            "filter for multicast channel." );
            }
        }

        // RX Mask
        p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_MULTICAST_RXMASK );
        if ( NULL != p ) {
            wxString wxstr = wxString::FromUTF8Unchecked( (const char *)p );
            if ( !vscp_readMaskFromString( &pChannel->m_rxFilter, wxstr ) ) {
                fprintf( stderr, 
                           "readMulticastChannels: Failed to set RX "
                           "mask for multicast channel." );
            }
        }

        // Encryption
        p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_UDPNODE_ENCRYPTION );
        if ( NULL != p ) {
            wxString wxstr = wxString::FromUTF8Unchecked( (const char *)p );
            pChannel->m_nEncryption = vscp_getEncryptionCodeFromToken( wxstr );
        }

        // Add to list
        pChannel->m_index = 0;
        m_multicastInfo.m_channels.Append( pChannel );

        m_mutexMulticastInfo.Unlock();

    }

    sqlite3_finalize( ppStmt );

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doCreateLogTable
//
// Create the log database
//
//

bool CControlObject::doCreateLogTable( void )
{
    char *pErrMsg = 0;
    const char *psql = VSCPDB_LOG_CREATE;

    fprintf( stderr, "Creating VSCP log database.\n" );

    // Check if database is open
    if ( NULL == m_db_vscp_log ) {
        fprintf( stderr,
                    "Failed to create VSCP log database - closed.\n" );
        return false;
    }

    m_db_vscp_configMutex.Lock();

    if ( SQLITE_OK  !=  sqlite3_exec( m_db_vscp_log, psql, NULL, NULL, &pErrMsg ) ) {
        fprintf( stderr,
                    "Failed to create VSCP log table with error %s.\n",
                    pErrMsg );
        m_db_vscp_configMutex.Unlock();
        return false;
    }

    m_db_vscp_configMutex.Unlock();

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doCreateUdpNodeTable
//
// Create the UDP node database
//

bool CControlObject::doCreateUdpNodeTable( void )
{
    char *pErrMsg = 0;
    const char *psql = VSCPDB_UDPNODE_CREATE;

    fprintf( stderr, "Creating udpnode table.\n" );

    // Check if database is open
    if ( NULL == m_db_vscp_daemon ) {
        fprintf( stderr,
                    "Failed to create VSCP udpnode table - database closed.\n" );
        return false;
    }

    m_db_vscp_configMutex.Lock();

    if ( SQLITE_OK  !=  sqlite3_exec( m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg ) ) {
        fprintf( stderr,
                    "Failed to create VSCP udpnode table with error %s.\n",
                    pErrMsg );
        m_db_vscp_configMutex.Unlock();
        return false;
    }

    m_db_vscp_configMutex.Unlock();

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doCreateMulticastTable
//
// Create the multicast database
//
//

bool CControlObject::doCreateMulticastTable( void )
{
    char *pErrMsg = 0;
    const char *psql = VSCPDB_MULTICAST_CREATE;

    fprintf( stderr, "Creating multicast table.\n" );

    // Check if database is open
    if ( NULL == m_db_vscp_daemon ) {
        fprintf( stderr,
                    "Failed to create VSCP multicast table - database closed.\n" );
        return false;
    }

    m_db_vscp_configMutex.Lock();

    if ( SQLITE_OK  !=  sqlite3_exec( m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg ) ) {
        fprintf( stderr,
                    "Failed to create VSCP multicast table with error %s.\n",
                    pErrMsg );
        m_db_vscp_configMutex.Unlock();
        return false;
    }

    m_db_vscp_configMutex.Unlock();

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// doCreateUserTable
//
// Create the user table
//
//

bool CControlObject::doCreateUserTable( void )
{
    char *pErrMsg = 0;
    const char *psql = VSCPDB_USER_CREATE;

    fprintf( stderr, "Creating user table..\n" );

    // Check if database is open
    if ( NULL == m_db_vscp_daemon ) {
        fprintf( stderr,
                    "Failed to create VSCP user table - closed.\n" );
        return false;
    }

    if ( SQLITE_OK  !=  sqlite3_exec( m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg ) ) {
        fprintf( stderr,
                    "Failed to create VSCP user table with error %s.\n",
                    pErrMsg );
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

bool CControlObject::doCreateDriverTable( void )
{
    char *pErrMsg = 0;
    const char *psql = VSCPDB_DRIVER_CREATE;

    fprintf( stderr, "Creating driver table..\n" );

    // Check if database is open
    if ( NULL == m_db_vscp_daemon ) {
        fprintf( stderr,
                    "Failed to create VSCP driver table - closed.\n" );
        return false;
    }

    if ( SQLITE_OK  !=  sqlite3_exec( m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg ) ) {
        fprintf( stderr,
                    "Failed to create VSCP driver table with error %s.\n",
                    pErrMsg );
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

bool CControlObject::doCreateGuidTable( void )
{
    char *pErrMsg = 0;
    const char *psql = VSCPDB_GUID_CREATE;

    fprintf( stderr, "Creating GUID discovery table..\n" );

    // Check if database is open
    if ( NULL == m_db_vscp_daemon ) {
        fprintf( stderr,
                    "Failed to create VSCP GUID table - closed.\n" );
        return false;
    }

    if ( SQLITE_OK  !=  sqlite3_exec( m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg ) ) {
        fprintf( stderr,
                    "Failed to create VSCP GUID table with error %s.\n",
                    pErrMsg );
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

bool CControlObject::doCreateLocationTable( void )
{
    char *pErrMsg = 0;
    const char *psql = VSCPDB_LOCATION_CREATE;

    fprintf( stderr, "Creating location table..\n" );

    // Check if database is open
    if ( NULL == m_db_vscp_daemon ) {
        fprintf( stderr,
                    "Failed to create VSCP location table - closed.\n" );
        return false;
    }

    if ( SQLITE_OK  !=  sqlite3_exec( m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg ) ) {
        fprintf( stderr,
                    "Failed to create VSCP location table with error %s.\n",
                    pErrMsg );
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

bool CControlObject::doCreateMdfCacheTable( void )
{
    char *pErrMsg = 0;
    const char *psql = VSCPDB_MDF_CREATE;

    fprintf( stderr, "Creating MDF table..\n" );

    // Check if database is open
    if ( NULL == m_db_vscp_daemon ) {
        fprintf( stderr,
                    "Failed to create VSCP mdf table - closed.\n" );
        return false;
    }

    if ( SQLITE_OK  !=  sqlite3_exec( m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg ) ) {
        fprintf( stderr,
                    "Failed to create VSCP mdf table with error %s.\n",
                    pErrMsg );
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

bool CControlObject::doCreateSimpleUiTable( void )
{
    char *pErrMsg = 0;
    const char *psql = VSCPDB_SIMPLE_UI_CREATE;

    fprintf( stderr, "Creating simple ui table..\n" );

    // Check if database is open
    if ( NULL == m_db_vscp_daemon ) {
        fprintf( stderr,
                    "Failed to create VSCP simple ui table - closed.\n" );
        return false;
    }

    if ( SQLITE_OK  !=  sqlite3_exec( m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg ) ) {
        fprintf( stderr,
                    "Failed to create VSCP simple ui table with error %s.\n",
                    pErrMsg );
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

bool CControlObject::doCreateSimpleUiItemTable( void )
{
    char *pErrMsg = 0;
    const char *psql = VSCPDB_SIMPLE_UI_ITEM_CREATE;

    fprintf( stderr, "Creating simple ui item table..\n" );

    // Check if database is open
    if ( NULL == m_db_vscp_daemon ) {
        fprintf( stderr,
                    "Failed to create VSCP simple UI item table - closed.\n" );
        return false;
    }

    if ( SQLITE_OK != sqlite3_exec( m_db_vscp_daemon, 
                                       psql, NULL, NULL, &pErrMsg ) ) {
        fprintf( stderr,
                    "Failed to create VSCP simple UI item table with error %s.\n",
                    pErrMsg );
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doCreateZoneTable
//
// Create the zone table
//

bool CControlObject::doCreateZoneTable( void )
{
    char *pErrMsg = 0;
    const char *psql = VSCPDB_ZONE_CREATE;

    fprintf( stderr, "Creating zone table..\n" );

    // Check if database is open
    if ( NULL == m_db_vscp_daemon ) {
        fprintf( stderr,
                    "Failed to create VSCP zone table - closed.\n" );
        return false;
    }

    if ( SQLITE_OK != sqlite3_exec( m_db_vscp_daemon, 
                                        psql, NULL, NULL, &pErrMsg ) ) {
        fprintf( stderr,
                    "Failed to create VSCP zone table with error %s.\n",
                    pErrMsg );
        return false;
    }
    
    // Fill with default info
    wxString sql = "BEGIN;";
    for ( int i=0; i<256; i++ ) {
        sql += 
             wxString::Format( _("INSERT INTO 'zone' (idx_zone, name) "
                                 "VALUES( %d, 'zone%d' );"),
                                 i, i );
    }
    
    sql += wxString::Format( VSCPDB_ZONE_UPDATE,
                                        "All zones",
                                        "Zone = 255 represents all zones.",
                                        255L );
    sql += "COMMIT;";
    if ( SQLITE_OK != sqlite3_exec( m_db_vscp_daemon, 
                                        (const char *)sql.mbc_str(), 
                                        NULL, NULL, &pErrMsg ) ) {
        fprintf( stderr,
                    "Failed to insert last VSCP default zone table entry %d. Error %s\n",
                    255, pErrMsg );
    } 
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doCreateSubZoneTable
//
// Create the subzone table
//
//

bool CControlObject::doCreateSubZoneTable( void )
{
    char *pErrMsg = 0;
    const char *psql = VSCPDB_SUBZONE_CREATE;

    fprintf( stderr, "Creating sub-zone table..\n" );

    // Check if database is open
    if ( NULL == m_db_vscp_daemon ) {
        fprintf( stderr,
                    "Failed to create VSCP subzone table - closed.\n" );
        return false;
    }

    if ( SQLITE_OK  !=  sqlite3_exec( m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg ) ) {
        fprintf( stderr,
                    "Failed to create VSCP subzone table with error %s.\n",
                    pErrMsg );
        return false;
    }
        
    // Fill with default info
    wxString sql = "BEGIN;";
    for ( int i=0; i<256; i++ ) {
        sql += 
             wxString::Format( _("INSERT INTO 'subzone' (idx_subzone, name) "
                                 "VALUES( %d, 'subzone%d' );"),
                                 i, i );
    }
    
    sql += wxString::Format( VSCPDB_SUBZONE_UPDATE,
                                        "All subzones",
                                        "Subzone = 255 represents all subzones of a zone.",
                                        255L ); 
    sql += "COMMIT;";
    if ( SQLITE_OK != sqlite3_exec( m_db_vscp_daemon, 
                                        (const char *)sql.mbc_str(), 
                                        NULL, NULL, &pErrMsg ) ) {
        fprintf( stderr,
                    "Failed to insert last VSCP default subzone table entry %d. Error %s\n",
                    255, pErrMsg );
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// doCreateUserdefTableTable
//
// Create the userdef table
//
//

bool CControlObject::doCreateUserdefTableTable( void )
{
    char *pErrMsg = 0;
    const char *psql = VSCPDB_TABLE_CREATE;

    fprintf( stderr, "Creating userdef table..\n" );

    // Check if database is open
    if ( NULL == m_db_vscp_daemon ) {
        fprintf( stderr,
                    "Failed to create VSCP userdef table - closed.\n" );
        return false;
    }

    if ( SQLITE_OK  !=  sqlite3_exec( m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg ) ) {
        fprintf( stderr,
                    "Failed to create VSCP userdef table with error %s.\n",
                    pErrMsg );
        return false;
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// createFolderStructure
//
// Create the folder structure under the base directory
//
// http://www.vscp.org/docs/vscpd/doku.php?id=files_and_directory_structure
//

bool CControlObject::createFolderStructure( void )
{
    // Create folder structure
    wxFileName::Mkdir( m_rootFolder, 0x777, wxPATH_MKDIR_FULL );
    wxFileName::Mkdir( m_rootFolder + _("/logs"), 0x777, wxPATH_MKDIR_FULL );
    wxFileName::Mkdir( m_rootFolder + _("/actions"), 0x777, wxPATH_MKDIR_FULL );
    wxFileName::Mkdir( m_rootFolder + _("/scripts"), 0x777, wxPATH_MKDIR_FULL );
    wxFileName::Mkdir( m_rootFolder + _("/scripts/javascript"), 0x777, wxPATH_MKDIR_FULL );
    wxFileName::Mkdir( m_rootFolder + _("/scripts/lua"), 0x777, wxPATH_MKDIR_FULL );
    wxFileName::Mkdir( m_rootFolder + _("/upload"), 0x777, wxPATH_MKDIR_FULL );
    wxFileName::Mkdir( m_rootFolder + _("/ux"), 0x777, wxPATH_MKDIR_FULL );
    wxFileName::Mkdir( m_rootFolder + _("/web"), 0x777, wxPATH_MKDIR_FULL );
    wxFileName::Mkdir( m_rootFolder + _("/certs"), 0x777, wxPATH_MKDIR_FULL );
    wxFileName::Mkdir( m_rootFolder + _("/drivers"), 0x777, wxPATH_MKDIR_FULL );
    wxFileName::Mkdir( m_rootFolder + _("/drivers/level1"), 0x777, wxPATH_MKDIR_FULL );
    wxFileName::Mkdir( m_rootFolder + _("/drivers/level2"), 0x777, wxPATH_MKDIR_FULL );
    wxFileName::Mkdir( m_rootFolder + _("/drivers/level3"), 0x777, wxPATH_MKDIR_FULL );
    wxFileName::Mkdir( m_rootFolder + _("/web/css"), 0x777, wxPATH_MKDIR_FULL );
    wxFileName::Mkdir( m_rootFolder + _("/web/images"), 0x777, wxPATH_MKDIR_FULL );
    wxFileName::Mkdir( m_rootFolder + _("/web/lib"), 0x777, wxPATH_MKDIR_FULL );
    wxFileName::Mkdir( m_rootFolder + _("/web/testws"), 0x777, wxPATH_MKDIR_FULL );
    wxFileName::Mkdir( m_rootFolder + _("/web/service"), 0x777, wxPATH_MKDIR_FULL );
    wxFileName::Mkdir( m_rootFolder + _("/tables"), 0x777, wxPATH_MKDIR_FULL );
    wxFileName::Mkdir( m_rootFolder + _("/mdf"), 0x777, wxPATH_MKDIR_FULL );

    // check if main configuration file is in place
    // create a new one if not

    // copy in example cert

    // dopy inexample dm.xml

    // copy in example variable.xml file

    return true;
}






///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////








///////////////////////////////////////////////////////////////////////////////
// clientMsgWorkerThread
//

clientMsgWorkerThread::clientMsgWorkerThread()
: wxThread(wxTHREAD_JOINABLE)
{
    m_bQuit = false;
    m_pCtrlObject = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// deviceWorkerThread
//

clientMsgWorkerThread::~clientMsgWorkerThread()
{
    ;
}

///////////////////////////////////////////////////////////////////////////////
// Entry
//
// Is there any messages to send from Level II clients. Send it/them to all
// devices/clients except for itself.
//

void *clientMsgWorkerThread::Entry()
{
    VSCPEventList::compatibility_iterator nodeVSCP;
    vscpEvent *pvscpEvent = NULL;

    // Must be a valid control object pointer
    if (NULL == m_pCtrlObject) return NULL;

    while (!TestDestroy() && !m_bQuit) {

        // Wait for event
        if ( wxSEMA_TIMEOUT ==
                m_pCtrlObject->m_semClientOutputQueue.WaitTimeout(500) ) continue;

        if ( m_pCtrlObject->m_clientOutputQueue.GetCount() ) {

            m_pCtrlObject->m_mutexClientOutputQueue.Lock();
            nodeVSCP = m_pCtrlObject->m_clientOutputQueue.GetFirst();
            pvscpEvent = nodeVSCP->GetData();
            m_pCtrlObject->m_clientOutputQueue.DeleteNode(nodeVSCP);
            m_pCtrlObject->m_mutexClientOutputQueue.Unlock();

            if ( NULL != pvscpEvent ) {

                // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
                // * * * * Send event to all Level II clients (not to ourself )  * * * *
                // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

                m_pCtrlObject->sendEventAllClients( pvscpEvent, pvscpEvent->obid );

            } // Valid event

            // Delete the event
            if (NULL != pvscpEvent) vscp_deleteVSCPevent( pvscpEvent );
            pvscpEvent = NULL;

        } // while

    } // while

    return NULL;

}


///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void clientMsgWorkerThread::OnExit()
{
    ;
}





