// ControlObject.cpp: m_path_db_vscp_logimplementation of the CControlObject class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (http://www.vscp.org)
//
// Copyright (C) 2000-2016 Ake Hedman,
// Grodans Paradis AB, <akhe@grodansparadis.com>
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
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for VSCP & Friends may be arranged by contacting
// Grodans Paradis AB at http://www.grodansparadis.com
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

#ifdef WIN32
#include <winsock2.h>
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"
#include <wx/xml/xml.h>

#ifdef WIN32

#include <winsock2.h>
//#include <winsock.h>
#include "canal_win32_ipc.h"

#else   // UNIX

#define _POSIX
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
#include <wx/listimpl.cpp>
#include <wx/xml/xml.h>
#include <wx/mimetype.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>

#include "web_css.h"
#include "web_js.h"
#include "web_template.h"

#ifndef VSCP_DISABLE_LUA
#include <lua.hpp>
#endif

#ifndef VSCP_DISABLE_SQLITE
#include <sqlite3.h>
#endif

#include <mongoose.h>

#include "canal_macro.h"
#include <vscp.h>
#include <vscphelper.h>
#include <vscpeventhelper.h>
#include <tables.h>
#include <configfile.h>
#include <crc.h>
#include <randpassword.h>
#include <version.h>
#include <vscpdb.h>
#include <variablecodes.h>
#include <actioncodes.h>
#include <devicelist.h>
#include <devicethread.h>
#include <dm.h>
#include <vscpeventhelper.h>
#include <vscpwebserver.h>
#include <vscpd_caps.h>
#include <controlobject.h>


#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif


// Lists
WX_DEFINE_LIST(TRIGGERLIST)     // websocket triggers
WX_DEFINE_LIST(CanalMsgList);
WX_DEFINE_LIST(VSCPEventList);

//#define DEBUGPRINT

CControlObject *gpctrlObj;

#ifdef WIN32

typedef struct _ASTAT_ {
    ADAPTER_STATUS adapt;
    NAME_BUFFER NameBuff [30];
} ASTAT, * PASTAT;

ASTAT Adapter;


WORD wVersionRequested = MAKEWORD(1, 1);    // WSA functions
WSADATA wsaData;                            // WSA functions

#endif

// Prototypes


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CControlObject::CControlObject()
{
    int i;
    m_bQuit = false;            // true if we should quit
    gpctrlObj = this;           // needed by websocket static callbacks, variables etc
    
    // Default admin user credentials
    m_admin_user = _("admin");
    m_admin_password = _("secret");
    m_admin_allowfrom = _("*");
    
    m_nConfiguration = 1;       // Default configuration record is read.
    
    // Log to database as default
    m_bLogToDatabase = true;
    
    // Log to syslog
    m_bLogToSysLog = true;

    m_automation.setControlObject( this );
    m_maxItemsInClientReceiveQueue = MAX_ITEMS_CLIENT_RECEIVE_QUEUE;

    // Nill the GUID
    m_guid.clear();

    // Initialise the client map
    // to all unused
    for (i = 0; i < VSCP_MAX_CLIENTS; i++) {
        m_clientMap[ i ] = 0;
    }

    // Web server security should be used.
    m_bDisableSecurityWebServer = false;

    // Local domain
    strcpy( m_authDomain, "mydomain.com" );

    // Set Default Log Level
    m_logLevel = DAEMON_LOGMSG_NORMAL;

    // General logfile is enabled by default
    m_bLogGeneralEnable = true;

#ifdef WIN32
    m_logGeneralFileName.Assign( wxStandardPaths::Get().GetConfigDir() +
                                        _("/vscp/logs/vscp_log_general.txt") );
#else
    m_logGeneralFileName.Assign( _("/srv/vscp/logs/vscp_log_general") );
#endif

    // Security logfile is enabled by default
    m_bLogSecurityEnable = true;

#ifdef WIN32
    m_logSecurityFileName.Assign( wxStandardPaths::Get().GetConfigDir() +
                                            _("/vscp/vscp_log_security.txt") );
#else
    m_logSecurityFileName.Assign( _("/srv/vscp/logs/vscp_log_security") );
#endif

    // Access logfile is enabled by default
    m_bLogAccessEnable = true;

#ifdef WIN32
    m_logAccessFileName.Assign( wxStandardPaths::Get().GetConfigDir() +
                                            _("/vscp/vscp_log_access.txt") );
#else
    m_logAccessFileName.Assign( _("/srv/vscp/logs/vscp_log_access") );
#endif

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
    m_path_db_vscp_data.Assign( _("/srv/vsc/vscp_data.sqlite3") );
#endif
 
    
#ifdef WIN32
    m_path_db_vscp_log.Assign( wxStandardPaths::Get().GetConfigDir() +
                                            _("/vscp/vscp_log.sqlite3") );
#else
    m_path_db_vscp_log.Assign( _("/var/log/vscp/vscpd_log.sqlite3") );
#endif   
        

    // No databases opened yet
    m_db_vscp_daemon = NULL;
    m_db_vscp_data = NULL;
    m_dm.m_db_vscp_dm = NULL;

    // Control UDP Interface
    m_bUDP = false;

    // Enable MQTT broker
    m_bMQTTBroker = true;

    // Enable CoAP server
    m_bCoAPServer = true;

    // Default TCP/IP interface
    m_strTcpInterfaceAddress = _("9598");

    // Default multicast announce port
    m_strMulticastAnnounceAddress = _( "9598" );

    // default multicast announce ttl
    m_ttlMultiCastAnnounce = IP_MULTICAST_DEFAULT_TTL;

    // Default UDP interface
    m_strUDPInterfaceAddress = _("udp://:9598");

    // Default MQTT broker interface
    m_strMQTTBrokerInterfaceAddress = _("1883");

    // Default CoAP server interface
    m_strCoAPServerInterfaceAddress = _("udp://:5683");

    m_pclientMsgWorkerThread = NULL;
    m_pVSCPClientThread = NULL;
    m_pdaemonVSCPThread = NULL;
    m_pwebServerThread = NULL;
    m_pMQTTBrookerThread = NULL;
    m_pCoAPServerThread = NULL;

    // Websocket interface
    m_bAuthWebsockets = true;   // Authentication is needed
    memset( m_pathCert, 0, sizeof( m_pathCert ) );

    // Webserver interface
    m_strWebServerInterfaceAddress = _("8080");

#ifdef WIN32
    //m_pathWebRoot = _("/programdata/vscp/www");
    strcpy( m_pathWebRoot, "/programdata/vscp/www" );
#else
    //m_pathWebRoot = _("/srv/vscp/www");
    strcpy( m_pathWebRoot, "/srv/vscp/www" );
#endif

    memset( m_cgiInterpreter, 0, sizeof( m_cgiInterpreter ) );
    memset( m_cgiPattern, 0, sizeof( m_cgiPattern ) );
    memset( m_hideFilePatterns, 0, sizeof( m_hideFilePatterns ) );
    memset( m_indexFiles, 0, sizeof( m_indexFiles ) );
    memset( m_urlRewrites, 0, sizeof( m_urlRewrites ) );
    memset( m_per_directory_auth_file, 0, sizeof( m_per_directory_auth_file ) );
    memset( m_global_auth_file, 0, sizeof( m_global_auth_file ) );
    memset( m_ssi_pattern, 0, sizeof( m_ssi_pattern ) );
    memset( m_ip_acl, 0, sizeof( m_ip_acl ) );
    memset( m_dav_document_root, 0, sizeof( m_dav_document_root ) );

    // Directory listings on by default
    strcpy( m_EnableDirectoryListings, "yes" );

    // Set control object
    m_dm.setControlObject(this);

#ifdef WIN32

    // Initialise winsock layer
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

    // Initialise the CRC
    crcInit();

#if (0)
    CVSCPTable testtable( "c:/tmp/test.tbl" );

    testtable.logData( 1, 100 );
    testtable.logData( 8, 800 );
    testtable.logData( 12, 1200 );
    testtable.logData( 19, 2000 );
    testtable.logData( 20, 2000 );
    testtable.logData( 21, 2100 );
    testtable.logData( 30, 3000 );
    testtable.logData( 48, 4800 );

    long size = testtable.GetRangeOfData( 22, 48, NULL, 0 );
    size = testtable.GetRangeOfData( 1, 8, NULL, 0 );
    size = testtable.GetRangeOfData( 20, 48, NULL, 0 );
#endif

if ( 0 ) {
    time_t rawtime;
    struct tm *timeinfo;

    time ( &rawtime );
    timeinfo = localtime( &rawtime );
    timeinfo->tm_hour = 18;
    timeinfo->tm_min = 0;
    timeinfo->tm_sec = 0;
    timeinfo->tm_mday = 3;
    timeinfo->tm_mon = 10;
    timeinfo->tm_year = 2015;
    time_t ttt = mktime ( timeinfo );
    ttt = ttt + 1;
}

#ifdef CS_ENABLE_DEBUG
    FILE *logfile;
    cs_log_set_level( LL_VERBOSE_DEBUG );
    logfile = fopen( "/mongoose_logfile.txt", "w+" );
    cs_log_set_file( logfile );
#endif

}

///////////////////////////////////////////////////////////////////////////////
// Destructor
//

CControlObject::~CControlObject()
{
    // Remove objects in Client send queue
    VSCPEventList::iterator iterVSCP;

    m_mutexClientOutputQueue.Lock();
    for (iterVSCP = m_clientOutputQueue.begin();
            iterVSCP != m_clientOutputQueue.end(); ++iterVSCP) {
        vscpEvent *pEvent = *iterVSCP;
        vscp_deleteVSCPevent(pEvent);
    }

    m_clientOutputQueue.Clear();
    m_mutexClientOutputQueue.Unlock();
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

    memset( psid, 0, sizeof( 33 ) );
    cs_md5( psid, buf, strlen( buf ), NULL );
    
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
    
    // Fill info in MSB of 16-bit code
    pCapability[ 14 ] = VSCP_DAEMON_SERVER_CAPABILITIES_6;
            
    // Fill info in LSB of 16-bit code
    pCapability[ 15 ] = VSCP_DAEMON_SERVER_CAPABILITIES_7;
    
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// logMsg
//

void CControlObject::logMsg(const wxString& wxstr, const uint8_t level, const uint8_t nType )
{
    m_mutexLogWrite.Lock();

    wxDateTime datetime( wxDateTime::GetTimeNow() );
    wxString wxdebugmsg;

    wxdebugmsg = datetime.FormatISODate() + _("T") + datetime.FormatISOTime() + _(" - ") + wxstr;

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

    if ( !m_bLogToDatabase ) {

        if ( level >= m_logLevel ) {

            if ( DAEMON_LOGTYPE_GENERAL == nType ) {

                // Write to general log file
                if ( m_fileLogGeneral.IsOpened() ) {
                    m_fileLogGeneral.Write( wxdebugmsg );
                }

            }

        }

        if ( DAEMON_LOGTYPE_SECURITY == nType ) {
            // Write to Security log file
            if ( m_fileLogSecurity.IsOpened() ) {
                m_fileLogSecurity.Write( wxdebugmsg );
            }
        }

        if ( DAEMON_LOGTYPE_ACCESS == nType ) {
            // Write to Access log file
            if ( m_fileLogAccess.IsOpened() ) {
                m_fileLogAccess.Write( wxdebugmsg );
            }
        }
    }
    // Log to database
    else {    

    }

#ifndef WIN32

    // Print to console if there is one
    if ( m_logLevel >= level ) {
        wxPrintf( wxdebugmsg );
    }

    if ( m_bLogToSysLog ) {
        
        switch (level) {

        case DAEMON_LOGMSG_NORMAL:
            syslog(LOG_INFO, "%s", (const char *) wxdebugmsg.ToAscii());
            break;

        case DAEMON_LOGMSG_DEBUG:
            syslog(LOG_DEBUG, "%s", (const char *) wxdebugmsg.ToAscii());
            break;

        };
        
    }

#endif

     m_mutexLogWrite.Unlock();

}


/////////////////////////////////////////////////////////////////////////////
// init

bool CControlObject::init( wxString& strcfgfile, wxString& rootFolder )
{
    wxString str;
    
    // Save root folder for later use.
    m_rootFolder = rootFolder;
    
    //wxLog::AddTraceMask( "wxTRACE_doWorkLoop" );
    //wxLog::AddTraceMask(_("wxTRACE_vscpd_receiveQueue")); // Receive queue
    //wxLog::AddTraceMask(_("wxTRACE_vscpd_Msg"));
    //wxLog::AddTraceMask(_("wxTRACE_VSCP_Msg"));
    //wxLog::AddTraceMask(_("wxTRACE_vscpd_ReceiveMutex"));
    //wxLog::AddTraceMask(_("wxTRACE_vscpd_sendMutexLevel1"));
    //wxLog::AddTraceMask(_("wxTRACE_vscpd_LevelII"));
    //wxLog::AddTraceMask( _( "wxTRACE_vscpd_dm" ) );

    // Change locale to get the correct decimal point
    //setlocale( LC_NUMERIC, "English_United States.1252" );  // Why not!
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

    // Root folder must exist
    if ( !wxFileName::DirExists( rootFolder ) ) {
        fprintf(stderr,"The specified rootfolder does not exist (%s).\n",
                (const char *)rootFolder.mbc_str() );
        return false;
    } 
    
    // A configuration file must be available
    if ( !wxFile::Exists( strcfgfile ) ) {
        printf("No configuration file. Can't initialise!.");
        fprintf( stderr, "No configuration file. Can't initialise!.\n" );
        str = _("Path = .") + strcfgfile + _("\n");
        fprintf( stderr, str.mbc_str() );
        return false;
    }

    // Initialise the SQLite library
    if ( SQLITE_OK != sqlite3_initialize() ) {
        fprintf( stderr, "Unable to initialise SQLite library!." );
        return false;
    }

    
    // The root folder is the basis for the configuration file
    m_path_db_vscp_daemon.Assign( rootFolder + _("/vscpd.sqlite3") );

    
    
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
            fprintf( stderr, str.mbc_str() );
            if ( NULL != m_db_vscp_daemon ) sqlite3_close( m_db_vscp_daemon );
            m_db_vscp_daemon = NULL;
            return false;
            
        }
        else {
            // Database is open. Read configuration data from it
            dbReadConfiguration();
        }

    }
    else {

        if ( m_path_db_vscp_daemon.IsOk() ) {
            
            // We need to create the database from scratch. This may not work if
            // the database is in a read only location.
            fprintf( stderr, "VSCP Daemon configuration database does not exist - will be created.\n" );
            str.Printf(_("Path=%s\n"), (const char *)m_path_db_vscp_daemon.GetFullPath().mbc_str() );
            fprintf( stderr, str.mbc_str() );
            
            if ( SQLITE_OK == sqlite3_open( (const char *)m_path_db_vscp_daemon.GetFullPath().mbc_str(),
                                            &m_db_vscp_daemon ) ) {
                
                // create the configuration database.
                doCreateConfigurationTable(); 
                
                // Create user table
                doCreateUserTable();
        
                // Create driver table
                doCreateDriverTable();

                // Create guid table
                doCreateGuidTable();

                // Create location table
                doCreateLocationTable();

                // Create mdf table
                doCreateMdfCacheTable();
    
                // Create simpleui table
                doCreateSimpleUiTable();
    
                // Create simpleui item table
                doCreateSimpleUiItemTable();
    
                // Create zone table
                doCreateZoneTable();
    
                // Create subzone table
                doCreateSubZoneTable();
        
                // Create userdef table
                doCreateUserdefTableTable();
            
            }
            
        }
        else {
            fprintf( stderr, "VSCP Daemon configuration database path invalid - will exit.\n" );
            str.Printf(_("Path=%s\n"),(const char *)m_path_db_vscp_daemon.GetFullPath().mbc_str() );
            fprintf( stderr, str.mbc_str() );
            return false;
        }

    }
    
    
    // * * * VSCP Daemon logging database * * *
    
    
    // Check filename 
    if ( m_path_db_vscp_log.IsOk() && m_path_db_vscp_log.FileExists() ) {

        if ( SQLITE_OK != sqlite3_open( (const char *)m_path_db_vscp_log.GetFullPath().mbc_str(),
                                            &m_db_vscp_log ) ) {

            // Failed to open/create the database file
            fprintf( stderr, "VSCP Daemon logging database could not be opened. - Will not be used.\n" );
            str.Printf( _("Path=%s error=%s\n"),
                            (const char *)m_path_db_vscp_log.GetFullPath().mbc_str(),
                            sqlite3_errmsg( m_db_vscp_log ) );
            fprintf( stderr, str.mbc_str() );
            if ( NULL != m_db_vscp_log ) sqlite3_close( m_db_vscp_log );
            m_db_vscp_log = NULL;

        }

    }
    else {

        if ( m_path_db_vscp_log.IsOk() ) {
            // We need to create the database from scratch. This may not work if
            // the database is in a read only location.
            fprintf( stderr, "VSCP Daemon logging database does not exist - will be created.\n" );
            str.Printf(_("Path=%s\n"), (const char *)m_path_db_vscp_log.GetFullPath().mbc_str() );
            fprintf( stderr, (const char *)str.mbc_str() );
            
            if ( SQLITE_OK == sqlite3_open( (const char *)m_path_db_vscp_log.GetFullPath().mbc_str(),
                                            &m_db_vscp_log ) ) {            
                // create the config. database.
                doCreateLogTable();
            }
            else {
                fprintf( stderr, "Failed to create vscp log database - will not be used.\n" );
            }
        }
        else {
            fprintf( stderr, "VSCP Daemon logging database path invalid - will not be used.\n" );
            str.Printf(_("Path=%s\n"), (const char *)m_path_db_vscp_log.GetFullPath().mbc_str() );
            fprintf( stderr, str.mbc_str() );
        }

    }
    
    
    
    // * * * VSCP Daemon data database - NEVER created * * *

    if ( SQLITE_OK != sqlite3_open( m_path_db_vscp_data.GetFullPath().mbc_str(),
                                            &m_db_vscp_data ) ) {

        // Failed to open/create the database file
        fprintf( stderr, "The VSCP data database could not be opened. - Will not be used.\n" );
        str.Printf( _("Path=%s error=%s\n"),
                        (const char *)m_path_db_vscp_data.GetFullPath().mbc_str(),
                        sqlite3_errmsg( m_db_vscp_data ) );
        fprintf( stderr, str.mbc_str() );
        if ( NULL != m_db_vscp_data ) sqlite3_close( m_db_vscp_data );
        m_db_vscp_data = NULL;

    }
    
    

#ifndef WIN32
    if ( m_runAsUser.Length() ) {
        struct passwd *pw;
        if ( NULL == ( pw = getpwnam(m_runAsUser.mbc_str() ) ) ) {
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

    str = _("Using configuration file: ") + strcfgfile + _("\n");
    fprintf( stderr, str.mbc_str() );

       

    // Read XML configuration
    if ( !readXMLConfiguration( strcfgfile ) ) {
        fprintf( stderr, "Unable to open/parse configuration file. Can't initialise!\n" );
        str = _("Path = .") + strcfgfile + _("\n");
        fprintf( stderr, str.mbc_str() );
        return FALSE;
    }
    
    //==========================================================================
    //                           Add admin user
    //==========================================================================

    m_userList.addUser( m_admin_user,
                            m_admin_password,
                            _("Admin user"),            // note
                            NULL,
                            _("admin"),
                            m_admin_allowfrom,          // Remotes allows to connect     
                            _("*"),                     // All events
                            VSCP_ADD_USER_FLAG_ADMIN ); // Not in DB
    
    //==========================================================================
    //                           Add driver user
    //==========================================================================
    
    // Generate username and password for drivers
    char buf[ 512 ];
    randPassword pw(3);

    // Level II Driver Username
    pw.generatePassword(32, buf);
    m_driverUsername = wxString::FromAscii(buf);

    // Level II Driver Password
    pw.generatePassword(32, buf);
    m_driverPassword = wxString::FromAscii( buf );

    wxString driverhash = m_driverUsername;
    driverhash += _(":");
    driverhash += wxString::FromUTF8( m_authDomain );
    driverhash += _(":");
    driverhash += m_driverPassword;

    memset( buf, 0, sizeof( buf ) );
    strncpy( buf,(const char *)driverhash.mbc_str(), driverhash.Length() );

    char digest[33];
    memset( digest, 0, sizeof( digest ) );
    static const size_t len_buf = strlen( buf );
    cs_md5( digest, buf, len_buf, NULL );

    m_userList.addUser( m_driverUsername,
                            wxString::FromUTF8( digest ),
                            _("System added driver user."), // note
                            NULL,
                            _("driver"),
                            _("127.0.0.1"),                 // Only local
                            _("*"),                         // All events
                            VSCP_ADD_USER_FLAG_LOCAL ); 
    
    // Open up the General logging file.
    if ( m_bLogGeneralEnable ) {
        if ( !m_fileLogGeneral.Open( m_logGeneralFileName.GetFullPath(), wxFile::write_append ) ) {
            logMsg( wxString::Format(_("Could not open general log file. Path=%s\n"),
                    (const char *)m_logGeneralFileName.GetFullPath() ) );
        }
    }

    // Open up the Security logging file
    if ( m_bLogSecurityEnable ) {
        if ( !m_fileLogSecurity.Open( m_logSecurityFileName.GetFullPath(), wxFile::write_append ) ) {
            logMsg( wxString::Format(_("Could not open security log file. Path=%s\n"),
                    (const char *)m_logSecurityFileName.GetFullPath() ) );
        }
    }

    // Open up the Access logging file
    if ( m_bLogAccessEnable ) {
        if ( !m_fileLogAccess.Open( m_logAccessFileName.GetFullPath(), wxFile::write_append ) ) {
            logMsg( wxString::Format(_("Could not open access log file. Path=%s\n"),
                    (const char *)m_logAccessFileName.GetFullPath() ) );
        }
    }

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
        m_strServerName = _( "VSCP Daemon @ " );;
        wxString strguid;
        m_guid.toString( strguid );
        m_strServerName += strguid;
    }
    
    str = _("VSCP Daemon started\n");
    str += _("Version: ");
    str += _(VSCPD_DISPLAY_VERSION);
    str += _("\n");
    str += _(VSCPD_COPYRIGHT);
    str += _("\n");
    logMsg( str );

    str.Printf(_("Log Level=%d\n"), m_logLevel );
    logMsg( str );

    // Initialise DM storage
    logMsg(_("Init DM.\n") );
    m_dm.init();

    // Load decision matrix if mechanism is enabled    
    logMsg(_("Loading DM.\n") );
    m_dm.loadFromXML();

    // Initialise variable storage
    logMsg(_("Initialise variables.\n") );
    m_VSCP_Variables.init();
    
    // Load variables if mechanism is enabled
    logMsg(_("Loading persistent variables from XML variable default path.\n") );
    m_VSCP_Variables.loadFromXML();

    // Start daemon internal client worker thread
    logMsg(_("Starting client worker thread.\n") );
    startClientWorkerThread();

    // Start TCP/IP interface
    logMsg(_("Starting TCP/IP interface.\n") );
    startTcpWorkerThread();

    // Start web sockets
    logMsg(_("Starting websockets interface.\n") );
    startWebServerThread();

    // Load drivers
    logMsg(_("Starting drivers.\n") );
    startDeviceWorkerThreads();

    // Start MQTT Broker if enabled
    if ( m_bMQTTBroker ) {
        logMsg(_("Starting MQTT interface.\n") );
        logMsg(_("MQTT Broker enabled.\n") );
        startMQTTBrokerThread();
    }
    else {
        logMsg(_("MQTTBroker disabled.\n") );
    }

    // Start CoAP server if enabled
    if ( m_bCoAPServer ) {
        logMsg(_("Starting CoAP interface.\n") );
        logMsg(_("CoAP Server enabled.\n") );
        startCoAPServerThread();
    }
    else {
        logMsg(_("CoAP Server disabled.\n") );
    }
    
    // Start daemon worker thread
    logMsg(_("Starting VSCP daemon worker thread.\n") );
    startDaemonWorkerThread();

    return true;
}


/////////////////////////////////////////////////////////////////////////////
// run - Program main loop
//
// Most work is done in the threads at the moment
//

bool CControlObject::run(void)
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

    // Init. table files
    m_mutexTableList.Lock();
    listVSCPTables::iterator iter;
    for ( iter = m_listTables.begin(); iter != m_listTables.end(); ++iter ) {
        CVSCPTable *pTable = *iter;
        pTable->m_mutexThisTable.Lock();
        pTable->init();
        pTable->m_mutexThisTable.Unlock();
    }
    m_mutexTableList.Unlock();

    // We need to create a clientItem and add this object to the list
    CClientItem *pClientItem = new CClientItem;
    if ( NULL == pClientItem ) {
        wxLogDebug(_("ControlObject: Unable to allocate Client item, Ending"));
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
    pClientItem->m_strDeviceName = _("Internal Daemon DM Client. Started at ");
    wxDateTime now = wxDateTime::Now();
    pClientItem->m_strDeviceName += now.FormatISODate();
    pClientItem->m_strDeviceName += _(" ");
    pClientItem->m_strDeviceName += now.FormatISOTime();

    // Add the client to the Client List
    m_wxClientMutex.Lock();
    addClient( pClientItem );
    m_wxClientMutex.Unlock();

    // Feed startup event
    m_dm.feed(&EventStartUp);


    //-------------------------------------------------------------------------
    //                            MAIN - LOOP
    //-------------------------------------------------------------------------

    // DM Loop
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

            // Put the LOOP event on the queue
            m_dm.feed( &EventLoop );
            continue;

        }

        //----------------------------------------------------------------------
        //                         Event received here
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

        } // Event in queue

    }  // LOOP

    // Do shutdown event
    m_dm.feed( &EventShutDown );

    // Remove messages in the client queues
    m_wxClientMutex.Lock();
    removeClient( pClientItem );
    m_wxClientMutex.Unlock();

    wxLogDebug(_("ControlObject: Done"));
    return true;
}


/////////////////////////////////////////////////////////////////////////////
// cleanup

bool CControlObject::cleanup(void)
{
    stopDeviceWorkerThreads();
    stopTcpWorkerThread();
    stopWebServerThread();
    stopClientWorkerThread();
    stopDaemonWorkerThread();

    // kill table files
    m_mutexTableList.Lock();
    listVSCPTables::iterator iter;
    for ( iter = m_listTables.begin(); iter != m_listTables.end(); ++iter )
    {
        CVSCPTable *pTable = *iter;
        delete pTable;
    }
    m_mutexTableList.Unlock();

    // Close logfile
    if ( m_bLogGeneralEnable ) {
        m_fileLogGeneral.Close();
    }

    // Close security file
    if ( m_bLogSecurityEnable ) {
        m_fileLogSecurity.Close();
    }

    // Close access file
    if ( m_bLogAccessEnable ) {
        m_fileLogAccess.Close();
    }
    
    // Close the VSCP data database
    sqlite3_close( m_db_vscp_data );
    
    
    
    // Close the DM database
    sqlite3_close( m_dm.m_db_vscp_dm );
    
    // Close the vscpd database
    sqlite3_close( m_db_vscp_daemon );
    
    // Close log database
    sqlite3_close( m_db_vscp_log );

    // Clean up SQLite lib allocations
    sqlite3_shutdown();

    wxLogDebug(_("ControlObject: Cleanup done"));
    return true;
}




/////////////////////////////////////////////////////////////////////////////
// startClientWorkerThread
//

bool CControlObject::startClientWorkerThread(void)
{
    /////////////////////////////////////////////////////////////////////////////
    // Load controlobject client message handler
    /////////////////////////////////////////////////////////////////////////////
    m_pclientMsgWorkerThread = new clientMsgWorkerThread;

    if (NULL != m_pclientMsgWorkerThread) {
        m_pclientMsgWorkerThread->m_pCtrlObject = this;
        wxThreadError err;
        if (wxTHREAD_NO_ERROR == (err = m_pclientMsgWorkerThread->Create())) {
            //m_ptcpListenThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY );
            if (wxTHREAD_NO_ERROR != (err = m_pclientMsgWorkerThread->Run())) {
                logMsg( _("Unable to run controlobject client thread.") );
            }
        } else {
            logMsg( _("Unable to create controlobject client thread.") );
        }
    } else {
        logMsg( _("Unable to allocate memory for controlobject client thread.") );
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// stopTcpWorkerThread
//

bool CControlObject::stopClientWorkerThread(void)
{
    if (NULL != m_pclientMsgWorkerThread) {
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

    m_pVSCPClientThread = new VSCPClientThread;

    if (NULL != m_pVSCPClientThread) {
        m_pVSCPClientThread->m_pCtrlObject = this;
        wxThreadError err;
        if (wxTHREAD_NO_ERROR == (err = m_pVSCPClientThread->Create())) {
                //m_ptcpListenThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY );
            if (wxTHREAD_NO_ERROR != (err = m_pVSCPClientThread->Run())) {
                logMsg(_("Unable to run TCP thread.") );
            }
        }
        else {
            logMsg( _("Unable to create TCP thread.") );
        }
    }
    else {
        logMsg(_("Unable to allocate memory for TCP thread.") );
    }


    return true;
}


/////////////////////////////////////////////////////////////////////////////
// stopTcpWorkerThread
//

bool CControlObject::stopTcpWorkerThread(void)
{
    if ( NULL != m_pVSCPClientThread ) {
        m_mutexTcpClientListenThread.Lock();
        m_pVSCPClientThread->m_bQuit = true;
        m_pVSCPClientThread->Wait();
        delete m_pVSCPClientThread;
        m_pVSCPClientThread = NULL;
        m_mutexTcpClientListenThread.Unlock();
    }

    return true;
}



/////////////////////////////////////////////////////////////////////////////
// startUDPWorkerThread
//

bool CControlObject::startUDPWorkerThread(void)
{
    /////////////////////////////////////////////////////////////////////////////
    // Run the UDP server thread
    /////////////////////////////////////////////////////////////////////////////
    if ( m_bUDP ) {

        m_pVSCPClientUDPThread = new VSCPUDPClientThread;

        if (NULL != m_pVSCPClientUDPThread) {
            m_pVSCPClientUDPThread->m_pCtrlObject = this;
            wxThreadError err;
            if (wxTHREAD_NO_ERROR == (err = m_pVSCPClientUDPThread->Create())) {
                //m_ptcpListenThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY );
                if (wxTHREAD_NO_ERROR != (err = m_pVSCPClientUDPThread->Run())) {
                    logMsg( _("Unable to run TCP thread.") );
                }
            }
            else {
                logMsg( _("Unable to create TCP thread.") );
            }
        }
        else {
            logMsg( _("Unable to allocate memory for TCP thread.") );
        }
    }

    return true;
}


/////////////////////////////////////////////////////////////////////////////
// stopUDPWorkerThread
//

bool CControlObject::stopUDPWorkerThread(void)
{
    if ( NULL != m_pVSCPClientUDPThread ) {
        m_mutexVSCPClientnUDPThread.Lock();
        m_pVSCPClientUDPThread->m_bQuit = true;
        m_pVSCPClientUDPThread->Wait();
        delete m_pVSCPClientThread;
        m_mutexVSCPClientnUDPThread.Unlock();
    }

    return true;
}


/////////////////////////////////////////////////////////////////////////////
// startWebServerThread
//

bool CControlObject::startWebServerThread(void)
{
    /////////////////////////////////////////////////////////////////////////////
    // Run the WebServer server thread
    /////////////////////////////////////////////////////////////////////////////

    m_pwebServerThread = new VSCPWebServerThread;

    if (NULL != m_pwebServerThread) {
        m_pwebServerThread->m_pCtrlObject = this;
        wxThreadError err;
        if (wxTHREAD_NO_ERROR == (err = m_pwebServerThread->Create())) {
            //m_ptcpListenThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY );
            if (wxTHREAD_NO_ERROR != (err = m_pwebServerThread->Run())) {
                logMsg( _("Unable to run WeServer thread.") );
            }
        }
        else {
            logMsg( _("Unable to create WebServer thread.") );
        }
    }
    else {
        logMsg( _("Unable to allocate memory for WebServer thread.") );
    }

    return true;
}


/////////////////////////////////////////////////////////////////////////////
// stopWebServerThread
//

bool CControlObject::stopWebServerThread(void)
{
    if (NULL != m_pwebServerThread) {
        m_mutexwebServerThread.Lock();
        m_pwebServerThread->m_bQuit = true;
        m_pwebServerThread->Wait();
        delete m_pwebServerThread;
        m_pwebServerThread = NULL;
        m_mutexwebServerThread.Unlock();
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// startDaemonWorkerThread
//

bool CControlObject::startDaemonWorkerThread(void)
{
    /////////////////////////////////////////////////////////////////////////////
    // Run the VSCP daemon thread
    /////////////////////////////////////////////////////////////////////////////
    m_pdaemonVSCPThread = new daemonVSCPThread;

    if (NULL != m_pdaemonVSCPThread) {
            m_pdaemonVSCPThread->m_pCtrlObject = this;

        wxThreadError err;
        if (wxTHREAD_NO_ERROR == (err = m_pdaemonVSCPThread->Create())) {
            m_pdaemonVSCPThread->SetPriority(WXTHREAD_DEFAULT_PRIORITY);
            if (wxTHREAD_NO_ERROR != (err = m_pdaemonVSCPThread->Run())) {
                logMsg( _("Unable to start TCP VSCP daemon thread.") );
            }
        }
        else {
            logMsg( _("Unable to create TCP VSCP daemon thread.") );
        }
    }
    else {
        logMsg( _("Unable to start VSCP daemon thread.") );
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// stopDaemonWorkerThread
//

bool CControlObject::stopDaemonWorkerThread(void)
{
    if (NULL != m_pdaemonVSCPThread) {
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

bool CControlObject::startDeviceWorkerThreads(void)
{
    CDeviceItem *pDeviceItem;

    VSCPDEVICELIST::iterator iter;
    for (iter = m_deviceList.m_devItemList.begin();
            iter != m_deviceList.m_devItemList.end();
            ++iter) {

        pDeviceItem = *iter;
        if (NULL != pDeviceItem) {

            // Just start if enabled
            if ( !pDeviceItem->m_bEnable ) continue;

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
    for (iter = m_deviceList.m_devItemList.begin();
            iter != m_deviceList.m_devItemList.end();
            ++iter) {

        pDeviceItem = *iter;
        if (NULL != pDeviceItem) {
            pDeviceItem->stopDriver();
        }

    }

    return true;
}



/////////////////////////////////////////////////////////////////////////////
// startMQTTBrokerWorkerThread
//

bool CControlObject::startMQTTBrokerThread(void)
{
    /////////////////////////////////////////////////////////////////////////////
    // Run the MQTT Broker thread if enabled
    /////////////////////////////////////////////////////////////////////////////
    if ( m_bMQTTBroker ) {

#ifdef MG_ENABLE_MQTT_BROKER        
        m_pMQTTBrookerThread = new VSCPMQTTBrokerThread;

        if (NULL != m_pMQTTBrookerThread) {
            m_pMQTTBrookerThread->m_pCtrlObject = this;
            wxThreadError err;
            if (wxTHREAD_NO_ERROR == (err = m_pMQTTBrookerThread->Create())) {
                //m_ptcpListenThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY );
                if (wxTHREAD_NO_ERROR != (err = m_pMQTTBrookerThread->Run())) {
                    logMsg( _("Unable to run TCP thread.") );
                }
            }
            else {
                logMsg( _("Unable to create TCP thread.") );
            }
        }
        else {
            logMsg( _("Unable to allocate memory for TCP thread.") );
         }
#endif         
    }

    return true;
}


/////////////////////////////////////////////////////////////////////////////
// stopMQTTBrokerWorkerThread
//

bool CControlObject::stopMQTTBrokerThread(void)
{
    if ( NULL != m_pMQTTBrookerThread ) {
        m_mutexMQTTBrokerThread.Lock();
        m_pMQTTBrookerThread->m_bQuit = true;
        m_pMQTTBrookerThread->Wait();
        delete m_pVSCPClientThread;
        m_mutexMQTTBrokerThread.Unlock();
    }

    return true;
}


/////////////////////////////////////////////////////////////////////////////
// startCoAPServerWorkerThread
//

bool CControlObject::startCoAPServerThread(void)
{
    /////////////////////////////////////////////////////////////////////////////
    // Run the CoAP Server thread if enabled
    /////////////////////////////////////////////////////////////////////////////
    if ( m_bCoAPServer ) {

 #ifdef MG_ENABLE_COAP       
        m_pCoAPServerThread = new VSCPCoAPServerThread;

        if (NULL != m_pCoAPServerThread) {
            m_pCoAPServerThread->m_pCtrlObject = this;
            wxThreadError err;
            if (wxTHREAD_NO_ERROR == (err = m_pCoAPServerThread->Create())) {
                //m_ptcpListenThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY );
                if (wxTHREAD_NO_ERROR != (err = m_pCoAPServerThread->Run())) {
                    logMsg( _("Unable to run TCP thread.") );
                }
            }
            else {
                logMsg( _("Unable to create TCP thread.") );
            }
        }
        else {
            logMsg( _("Unable to allocate memory for TCP thread.") );
        }
#endif
        
    }

    return true;
}


/////////////////////////////////////////////////////////////////////////////
// stopCoAPServerWorkerThread
//

bool CControlObject::stopCoAPServerThread(void)
{
    if ( NULL != m_pCoAPServerThread ) {
        m_mutexCoAPServerThread.Lock();
        m_pCoAPServerThread->m_bQuit = true;
        m_pCoAPServerThread->Wait();
        delete m_pVSCPClientThread;
        m_mutexCoAPServerThread.Unlock();
    }

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
    VSCPCLIENTLIST::iterator it;

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

        if ( ( NULL != pClientItem ) && ( excludeID != pClientItem->m_clientID ) ) {
            sendEventToClient( pClientItem, pEvent );
            wxLogTrace(_("wxTRACE_vscpd_receiveQueue"),
                    _(" ControlObject: Sent to client %d"),
                    pClientItem->m_clientID);
        }
    }

    m_wxClientMutex.Unlock();

}



//
// The clientmap holds free client id's in an array
// They are aquired when a client connects and released when a
// client disconnects.
//
// Interfaces can be fetched by investigating the map.
//
// Not used at the moment.



///////////////////////////////////////////////////////////////////////////////
//  getClientMapFromId
//

uint32_t CControlObject::getClientMapFromId( uint32_t clid )
{
    for (uint32_t i = 0; i < VSCP_MAX_CLIENTS; i++) {
        if (clid == m_clientMap[ i ]) return i;
    }

    return 0;
}


///////////////////////////////////////////////////////////////////////////////
//  getClientMapFromIndex
//

uint32_t CControlObject::getClientMapFromIndex( uint32_t idx )
{
    return m_clientMap[ idx ];
}


///////////////////////////////////////////////////////////////////////////////
//  addIdToClientMap
//

uint32_t CControlObject::addIdToClientMap( uint32_t clid )
{
    for ( uint32_t i = 1; i < VSCP_MAX_CLIENTS; i++ ) {
        if (0 == m_clientMap[ i ]) {
            m_clientMap[ i ] = clid;
            return clid;
        }
    }

    return 0;
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

void CControlObject::addClient( CClientItem *pClientItem,
                                    uint32_t id )
{
    // Add client to client list
    m_clientList.addClient(pClientItem, id);

    // Add mapped item
    addIdToClientMap(pClientItem->m_clientID);

    // Set GUID for interface
    pClientItem->m_guid = m_guid;

    pClientItem->m_guid.setNicknameID( 0 );
    pClientItem->m_guid.setClientID( pClientItem->m_clientID );
}


//////////////////////////////////////////////////////////////////////////////
// removeClient
//

void CControlObject::removeClient( CClientItem *pClientItem )
{
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
        logMsg( wxString::Format( _(" Ethernet MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n"),
                                        s.ifr_addr.sa_data[ 0 ],
                                        s.ifr_addr.sa_data[ 1 ],
                                        s.ifr_addr.sa_data[ 2 ],
                                        s.ifr_addr.sa_data[ 3 ],
                                        s.ifr_addr.sa_data[ 4 ],
                                        s.ifr_addr.sa_data[ 5 ] ), DAEMON_LOGMSG_DEBUG  );

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
#ifdef WIN32
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
// readConfiguration
//
// Read the configuration XML file
//

bool CControlObject::readXMLConfiguration( wxString& strcfgfile )
{
    unsigned long val;
    wxXmlDocument doc;

    wxString wxlogmsg = wxString::Format(_("Reading XML configuration from [%s]\n"),
                                            (const char *)strcfgfile.c_str() );
    logMsg( wxlogmsg  );

    if (!doc.Load(strcfgfile)) {
        logMsg(_("Can't load logfile. Is path correct?\n")  );
        return false;
    }

    // start processing the XML file
    if (doc.GetRoot()->GetName() != wxT("vscpconfig")) {
        logMsg(_("Can't read logfile. Maybe it is invalid!\n")  );
        return false;
    }

    wxXmlNode *child = doc.GetRoot()->GetChildren();
    while (child) {

        if (child->GetName() == wxT("general")) {

            wxXmlNode *subchild = child->GetChildren();
            while (subchild) {

                if (subchild->GetName() == wxT("secret")) {
                    m_admin_user = subchild->GetAttribute(wxT("user"), wxT("admin"));
                    m_admin_password = subchild->GetAttribute(wxT("password"), wxT("secret"));
                    m_admin_allowfrom = subchild->GetAttribute(wxT("allowfrom"), wxT("*"));                    
                }
                else if (subchild->GetName() == wxT("loglevel")) {
                    
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
                    // For old deprecated values
                    else if ( str.IsSameAs(_("INFO"), false)) {
                        m_logLevel = DAEMON_LOGMSG_NORMAL;
                    }
                    else if ( str.IsSameAs(_("NOTICE"), false)) {
                        m_logLevel = DAEMON_LOGMSG_NORMAL;
                    }
                    else if ( str.IsSameAs(_("WARNING"), false)) {
                        m_logLevel = DAEMON_LOGMSG_NORMAL;
                    }
                    else if ( str.IsSameAs(_("ERROR"), false)) {
                        m_logLevel = DAEMON_LOGMSG_NORMAL;
                    }
                    else if ( str.IsSameAs(_("CRITICAL"), false)) {
                        m_logLevel = DAEMON_LOGMSG_NORMAL;
                    }
                    else if ( str.IsSameAs(_("ALERT"), false)) {
                        m_logLevel = DAEMON_LOGMSG_NORMAL;
                    }
                    else if ( str.IsSameAs(_("EMERGENCY"), false)) {
                        m_logLevel = DAEMON_LOGMSG_NORMAL;
                    }
                    else {
                        m_logLevel = vscp_readStringValue( str );
                        if ( m_logLevel > DAEMON_LOGMSG_DEBUG ) {
                            m_logLevel = DAEMON_LOGMSG_NORMAL;
                        }
                    }
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_LogLevel"), 
                                                    wxString::Format(_("%d"), m_logLevel ) );
                    
                }
                else if (subchild->GetName() == wxT("runasuser")) {
                    
                    m_runAsUser = subchild->GetNodeContent();
                    m_runAsUser.Trim();
                    m_runAsUser.Trim(false);
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_RunAsUser"), 
                                                    wxString::Format(_("%s"), 
                                                    (const char *)m_runAsUser.mbc_str() ) );
                    
                }
                else if (subchild->GetName() == wxT("logsyslog")) {

                    wxString attribute = subchild->GetAttribute(wxT("enable"), wxT("true"));
                    attribute.MakeLower();
                    if (attribute.IsSameAs(_("false"), false)) {
                        m_bLogToSysLog = false;
                    }
                    else {
                        m_bLogToSysLog = true;
                    }
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_Syslog_Enable"), 
                                                    wxString::Format(_("%d"), 
                                                    m_bLogToSysLog ? 1 : 0 ) );
                    
                }
                else if (subchild->GetName() == wxT("logdatabase")) {

                    wxString attribute = subchild->GetAttribute(wxT("enable"), wxT("true"));
                    attribute.MakeLower();
                    if (attribute.IsSameAs(_("false"), false)) {
                        m_bLogToDatabase = false;
                    }
                    else {
                        m_bLogToDatabase = true;
                    }
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_LogDB_Enable"), 
                                                    wxString::Format(_("%d"), 
                                                    m_bLogToDatabase ? 1 : 0 ) );
                    
                }
                else if (subchild->GetName() == wxT("generallogfile")) {

                    wxFileName fileName;
                    wxString attribute = subchild->GetAttribute(wxT("enable"), wxT("true"));
                    attribute.MakeLower();
                    if (attribute.IsSameAs(_("false"), false)) {
                        m_bLogGeneralEnable = false;
                    }
                    else {
                        m_bLogGeneralEnable = true;
                    }
  
                    // New form
                    attribute = subchild->GetAttribute( wxT("path"), wxT("/srv/vscp/logs/vscp_log_general") );
                                      
                    fileName.Assign( subchild->GetNodeContent() );
                    if ( fileName.IsOk() ) {
                        m_logGeneralFileName = fileName;
                    }
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_GeneralLogFile_Enable"), 
                                                    wxString::Format(_("%d"), 
                                                    m_bLogGeneralEnable ? 1 : 0 ) );
                    
                    updateConfigurationRecordItem( _("vscpd_GeneralLogFile_Path"), 
                                                    wxString::Format(_("%s"), 
                                                    (const char *)m_logGeneralFileName.GetFullPath().mbc_str() ) );
                    
                }
                else if (subchild->GetName() == wxT("securitylogfile")) {

                    wxFileName fileName;
                    wxString attribute = subchild->GetAttribute(wxT("enable"), wxT("true"));
                    attribute.MakeLower();
                    if (attribute.IsSameAs(_("false"), false)) {
                        m_bLogSecurityEnable = false;
                    }
                    else {
                        m_bLogSecurityEnable = true;
                    }
 
                    // New form
                    attribute = subchild->GetAttribute(wxT("path"), wxT("/srv/vscp/logs/vscp_log_security"));
                                      
                    fileName.Assign( subchild->GetNodeContent() );
                    if ( fileName.IsOk() ) {
                        m_logSecurityFileName = fileName;
                    } 
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_SecurityLogFile_Enable"), 
                                                    wxString::Format(_("%d"), 
                                                    m_bLogSecurityEnable ? 1 : 0 ) );
                    
                    updateConfigurationRecordItem( _("vscpd_SecurityLogFile_Path"), 
                                                    wxString::Format(_("%s"), 
                                                    (const char *)m_logSecurityFileName.GetFullPath().mbc_str() ) );
                    
                }
                else if (subchild->GetName() == wxT("accesslogfile")) {

                    wxFileName fileName;
                    wxString attribute = subchild->GetAttribute(wxT("enable"), wxT("true"));
                    attribute.MakeLower();
                    if (attribute.IsSameAs(_("false"), false)) {
                        m_bLogAccessEnable = false;
                    }
                    else {
                        m_bLogAccessEnable = true;
                    }

                    // New form
                    attribute = subchild->GetAttribute(wxT("path"), wxT("/srv/vscp/logs/vscp_log_access"));
                                      
                    fileName.Assign( subchild->GetNodeContent() );
                    if ( fileName.IsOk() ) {
                        m_logAccessFileName = fileName;
                    } 
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_AccessLogFile_Enable"), 
                                                    wxString::Format(_("%d"), 
                                                    m_bLogAccessEnable ? 1 : 0 ) );
                    
                    updateConfigurationRecordItem( _("vscpd_AccessLogFile_Path"), 
                                                    wxString::Format(_("%s"), 
                                                    (const char *)m_logAccessFileName.GetFullPath().mbc_str() ) );
                    
                }                
                else if (subchild->GetName() == wxT("tcpip")) {

                    m_strTcpInterfaceAddress = subchild->GetAttribute(wxT("interface"), wxT(""));
                    
                    updateConfigurationRecordItem( _("vscpd_TcpipInterface_Address"), 
                                                    wxString::Format(_("%s"), 
                                                    (const char *)m_strTcpInterfaceAddress.mbc_str() ) );

                }
                else if ( subchild->GetName() == wxT( "multicast-announce" ) ) {

                    m_strMulticastAnnounceAddress = subchild->GetAttribute( wxT( "interface" ), wxT( "" ) );

                    m_ttlMultiCastAnnounce = vscp_readStringValue( subchild->GetAttribute( wxT( "ttl" ), wxT( "1" ) ) );
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_MulticastInterface_Address"), 
                                                    wxString::Format(_("%s"), 
                                                    (const char *)m_strMulticastAnnounceAddress.mbc_str() ) );
                                        
                    updateConfigurationRecordItem( _("vscpd_MulicastInterface_ttl"), 
                                                    wxString::Format(_("%d"), 
                                                    m_ttlMultiCastAnnounce ) );

                }
                else if (subchild->GetName() == wxT("udp")) {
                    wxString attribut = subchild->GetAttribute(wxT("enable"), wxT("true"));
                    attribut.MakeLower();
                    if (attribut.IsSameAs(_("false"), false)) {
                        m_bUDP = false;
                    }
                    else {
                        m_bUDP = true;
                    }

                    m_strUDPInterfaceAddress = subchild->GetAttribute(wxT("interface"), wxT(""));
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_UdpSimpleInterface_Enable"), 
                                                    wxString::Format(_("%d"), 
                                                    m_bUDP ? 1 : 0 ) );
                    
                    updateConfigurationRecordItem( _("vscpd_UdpSimpleInterface_Address"), 
                                                    wxString::Format(_("%s"), 
                                                    (const char *)m_strUDPInterfaceAddress.mbc_str() ) );

                }
                else if (subchild->GetName() == wxT("mqttbroker")) {
                    wxString attribut = subchild->GetAttribute(wxT("enable"), wxT("true"));
                    attribut.MakeLower();
                    if (attribut.IsSameAs(_("false"), false)) {
                        m_bMQTTBroker = false;
                    }
                    else {
                        m_bMQTTBroker = true;
                    }

                    m_strMQTTBrokerInterfaceAddress = subchild->GetAttribute(wxT("interface"), wxT(""));
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_MqttBroker_Enable"), 
                                                    wxString::Format(_("%d"), 
                                                    m_bMQTTBroker ? 1 : 0 ) );
                    
                    updateConfigurationRecordItem( _("vscpd_MqttBroker_Address"), 
                                                    wxString::Format(_("%s"), 
                                                    (const char *)m_strMQTTBrokerInterfaceAddress.mbc_str() ) );

                }
                else if (subchild->GetName() == wxT("coapsrv")) {
                    wxString attribut = subchild->GetAttribute(wxT("enable"), wxT("true"));
                    attribut.MakeLower();
                    if (attribut.IsSameAs(_("false"), false)) {
                        m_bCoAPServer = false;
                    }
                    else {
                        m_bCoAPServer = true;
                    }

                    m_strCoAPServerInterfaceAddress = subchild->GetAttribute(wxT("interface"), wxT(""));
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_CoapServer_Enable"), 
                                                    wxString::Format(_("%d"), 
                                                    m_bCoAPServer ? 1 : 0 ) );
                    
                    updateConfigurationRecordItem( _("vscpd_CoapServer_Address"), 
                                                    wxString::Format(_("%s"), 
                                                    (const char *)m_strCoAPServerInterfaceAddress.mbc_str() ) );

                }
                else if (subchild->GetName() == wxT("dm")) {
                    
                    // Should the internal DM be disabled
                    wxString attribut;
                    
                    // Get the path to the DM file  (Deprecated)
                    attribut = subchild->GetAttribute( wxT("path"), wxT("") );
                    if ( attribut.Length() ) {
                        m_dm.m_staticXMLPath = attribut;
                    }
                    
                    // Get the path to the DM file
                    attribut = subchild->GetAttribute(wxT("pathxml"), wxT(""));
                    if ( attribut.Length() ) {
                        m_dm.m_staticXMLPath = attribut;
                    }
                    
                    // Get the path to the DM db file
                    attribut = subchild->GetAttribute(wxT("pathdb"), wxT(""));
                    if ( attribut.Length() ) {
                        m_dm.m_path_db_vscp_dm.Assign( attribut );
                    }

                    // logging enable
                    m_dm.m_bLogEnable = true;
                    attribut = subchild->GetAttribute(wxT("enablelogging"), wxT("true"));
                    attribut.MakeLower();
                    if (attribut.IsSameAs(_("false"), false)) {
                        m_dm.m_bLogEnable = false;
                    }

                    // Get the DM Logpath
                    wxFileName fileName;
                    fileName.Assign( subchild->GetAttribute( wxT("logpath"), wxT("") ) );
                    if ( fileName.IsOk() ) {
                        m_dm.m_logPath = fileName;
                    }
                    
                    // Get the loglevel
                    wxString str = subchild->GetAttribute(wxT("loglevel"), wxT("NORMAL"));
                    str.Trim();
                    str.Trim(false);
                    str.MakeUpper();
                    if ( str.IsSameAs(_("NONE"), false)) {
                        m_dm.m_logLevel = LOG_DM_NONE;
                    }
                    else if ( str.IsSameAs(_("0"), false)) {
                        m_dm.m_logLevel = LOG_DM_NONE;
                    }
                    else if ( str.IsSameAs(_("DEBUG"), false)) {
                        m_dm.m_logLevel = LOG_DM_DEBUG;
                    }
                    else if ( str.IsSameAs(_("1"), false)) {
                        m_dm.m_logLevel = LOG_DM_DEBUG;
                    }
                    else if ( str.IsSameAs(_("NORMAL"), false)) {
                        m_dm.m_logLevel = LOG_DM_NORMAL;
                    }
                    else if ( str.IsSameAs(_("2"), false)) {
                        m_dm.m_logLevel = LOG_DM_NORMAL;
                    }
                    else if ( str.IsSameAs(_("EXTRA"), false)) {
                        m_dm.m_logLevel = LOG_DM_EXTRA;
                    }
                    else if ( str.IsSameAs(_("3"), false)) {
                        m_dm.m_logLevel = LOG_DM_EXTRA;
                    }
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_DM_DB_Path"), 
                                                    wxString::Format(_("%s"), 
                                                    (const char *)m_dm.m_staticXMLPath.mbc_str() ) );
                    
                    updateConfigurationRecordItem( _("vscpd_DM_XML_Path"), 
                                                    wxString::Format(_("%s"), 
                                                    (const char *)m_dm.m_path_db_vscp_dm.GetFullPath().mbc_str() ) );
                    
                    updateConfigurationRecordItem( _("vscpd_DM_Logging_Enable"), 
                                                    wxString::Format(_("%d"), 
                                                    m_dm.m_bLogEnable ? 1 : 0 ) );
                    
                    updateConfigurationRecordItem( _("vscpd_DM_Logging_Path"), 
                                                    wxString::Format(_("%s"), 
                                                    (const char *)m_dm.m_logPath.GetPath().mbc_str() ) );
                    
                    updateConfigurationRecordItem( _("vscpd_DM_Logging_Level"), 
                                                    wxString::Format(_("%d"), 
                                                    m_dm.m_logLevel ) );

                }
                else if (subchild->GetName() == wxT("variables")) {
                    
                    // Should the internal DM be disabled
                    wxFileName fileName;
                    wxString attrib;

                    // Get the path to the DM file
                    attrib = subchild->GetAttribute(wxT("path"), wxT(""));
                    fileName.Assign( attrib );
                    if ( fileName.IsOk() ) {
                        m_VSCP_Variables.m_xmlPath = fileName.GetFullPath();
                    }
                    
                    attrib = subchild->GetAttribute(wxT("pathxml"), wxT("/srv/vscp/variable.xml"));
                    fileName.Assign( attrib );
                    if ( fileName.IsOk() ) {
                        m_VSCP_Variables.m_xmlPath = fileName.GetFullPath();
                    }
                    
                    attrib = subchild->GetAttribute(wxT("pathdb"), wxT("/srv/vscp/variable.sqlite3"));
                    fileName.Assign( attrib );
                    if ( fileName.IsOk() ) {
                        m_VSCP_Variables.m_dbFilename = fileName;
                    } 
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_Variables_XML_Path"), 
                                                    wxString::Format(_("%s"), 
                                                    (const char *)m_VSCP_Variables.m_xmlPath.mbc_str() ) );

                    updateConfigurationRecordItem( _("vscpd_Variables_DB_Path"), 
                                                    wxString::Format(_("%s"), 
                                                    (const char *)m_VSCP_Variables.m_dbFilename.GetPath().mbc_str() ) );
                    
                }
                else if (subchild->GetName() == wxT("guid")) {
                    
                    wxString str = subchild->GetNodeContent();
                    m_guid.getFromString(str);
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_GUID"), 
                                                    wxString::Format(_("%s"), 
                                                    (const char *)m_guid.getAsString().mbc_str() ) );
                    
                }
                else if ( subchild->GetName() == wxT( "servername" ) ) {
                    
                    m_strServerName = subchild->GetNodeContent();
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_servername"), 
                                                    wxString::Format(_("%s"), 
                                                    (const char *)m_strServerName.mbc_str() ) );
                    
                }
                else if (subchild->GetName() == wxT("clientbuffersize")) {
                    
                    wxString str = subchild->GetNodeContent();
                    m_maxItemsInClientReceiveQueue = vscp_readStringValue(str);
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_clientbuffersize"), 
                                                    wxString::Format(_("%lu"), 
                                                    (unsigned long)m_maxItemsInClientReceiveQueue ) );
                    
                }
                else if (subchild->GetName() == wxT("webserver")) {

                    wxString attribute;

                    attribute = subchild->GetAttribute( wxT( "disableauthentication" ), wxT( "false" ) );

                    if ( attribute.IsSameAs( _( "true" ), true ) ) {                        
                        m_bDisableSecurityWebServer = true;
                    }
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_webserver_authentication_enable"), 
                                                    wxString::Format(_("%d"), 
                                                    m_bDisableSecurityWebServer ? 0 : 1 ) );

                    attribute = subchild->GetAttribute(wxT("port"), wxT("8080"));
                    attribute.Trim();
                    attribute.Trim(false);
                    if ( attribute.Length() ) {
                        m_strWebServerInterfaceAddress = attribute;
                    }
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_webserver_address"), 
                                                    wxString::Format(_("%s"), 
                                                    (const char *)m_strWebServerInterfaceAddress.mbc_str() ) );

                    attribute = subchild->GetAttribute(wxT("extra_mime_types"), wxT(""));
                    attribute.Trim();
                    attribute.Trim(false);
                    if ( attribute.Length() ) {
                        //m_pathWebRoot = attribute;
                        strcpy( m_extraMimeTypes, attribute.mbc_str() );
                    }
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_webserver_extramimetypes"), 
                                                    wxString::Format(_("%s"), 
                                                    m_extraMimeTypes ) );

                    attribute = subchild->GetAttribute(wxT("webrootpath"), wxT(""));
                    attribute.Trim();
                    attribute.Trim(false);
                    if ( attribute.Length() ) {
                        //m_pathWebRoot = attribute;
                        strcpy( m_pathWebRoot, attribute.mbc_str() );
                    }
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_webserver_rootpath"), 
                                                    wxString::Format(_("%s"), 
                                                    m_pathWebRoot ) );

                    attribute = subchild->GetAttribute(wxT("authdoamin"), wxT(""));
                    attribute.Trim();
                    attribute.Trim(false);
                    if ( attribute.Length() ) {
                        strcpy( m_authDomain, attribute.mbc_str() );
                    }
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_webserver_authdomain"), 
                                                    wxString::Format(_("%s"), 
                                                    m_authDomain ) );

                    attribute = subchild->GetAttribute(wxT("pathcert"), wxT(""));
                    attribute.Trim();
                    attribute.Trim(false);
                    if ( attribute.Length() ) {
                        strcpy( m_pathCert, attribute.mbc_str() );
                    }
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_webserver_pathcert"), 
                                                    wxString::Format(_("%s"), 
                                                    m_pathCert ) );

                    attribute = subchild->GetAttribute(wxT("cgi_interpreter"), wxT(""));
                    attribute.Trim();
                    attribute.Trim(false);
                    if ( attribute.Length() ) {
                        strcpy( m_cgiInterpreter, attribute.mbc_str() );                    
                    }
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_webserver_cgiinterpreter"), 
                                                    wxString::Format(_("%s"), 
                                                    m_cgiInterpreter ) );

                    attribute = subchild->GetAttribute(wxT("cgi_pattern"), wxT(""));
                    attribute.Trim();
                    attribute.Trim(false);
                    if ( attribute.Length() ) {
                        strcpy( m_cgiPattern, attribute.mbc_str() );
                    }
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_webserver_cgipattern"), 
                                                    wxString::Format(_("%s"), 
                                                    m_cgiPattern ) );

                    attribute = subchild->GetAttribute(wxT("enable_directory_listing"), wxT("true"));
                    attribute.Trim();
                    attribute.Trim(false);
                    attribute.MakeUpper();
                    if ( attribute.IsSameAs(_("FALSE"), false ) ) {
                        strcpy( m_EnableDirectoryListings, "no" );
                    }
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_webserver_enabledirectorylistings"), 
                                                    wxString::Format(_("%d"), 
                                                    ( wxNOT_FOUND != attribute.Find(_("true") ) ? 1 : 0 ) ) );

                    attribute = subchild->GetAttribute(wxT("hide_file_patterns"), wxT(""));
                    attribute.Trim();
                    attribute.Trim(false);
                    if ( attribute.Length() ) {
                        strcpy( m_hideFilePatterns, attribute.mbc_str() );
                    }
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_webserver_hidefilepatterns"), 
                                                    wxString::Format(_("%s"), 
                                                    m_hideFilePatterns ) );

                    attribute = subchild->GetAttribute(wxT("index_files"), wxT(""));
                    attribute.Trim();
                    attribute.Trim(false);
                    if ( attribute.Length() ) {
                        strcpy( m_indexFiles, attribute.mbc_str() );
                    }
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_webserver_indexfiles"), 
                                                    wxString::Format(_("%s"), 
                                                    m_indexFiles ) );

                    attribute = subchild->GetAttribute(wxT("url_rewrites"), wxT(""));
                    attribute.Trim();
                    attribute.Trim(false);
                    if ( attribute.Length() ) {
                        strcpy( m_urlRewrites, attribute.mbc_str() );
                    }
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_webserver_urlrewrites"), 
                                                    wxString::Format(_("%s"), 
                                                    m_urlRewrites ) );

                    attribute = subchild->GetAttribute(wxT("per_directory_auth_file"), wxT(""));
                    attribute.Trim();
                    attribute.Trim(false);
                    if ( attribute.Length() ) {
                        strcpy( m_per_directory_auth_file, attribute.mbc_str() );
                    }
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_webserver_perdirectoryauthfile"), 
                                                    wxString::Format(_("%s"), 
                                                    m_per_directory_auth_file ) );

                    attribute = subchild->GetAttribute(wxT("global_auth_file"), wxT(""));
                    attribute.Trim();
                    attribute.Trim(false);
                    if ( attribute.Length() ) {
                        strcpy( m_global_auth_file, attribute.mbc_str() );
                    }
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_webserver_globalauthfile"), 
                                                    wxString::Format(_("%s"), 
                                                    m_global_auth_file ) );

                    attribute = subchild->GetAttribute(wxT("ssi_pattern"), wxT(""));
                    attribute.Trim();
                    attribute.Trim(false);
                    if ( attribute.Length() ) {
                        strcpy( m_ssi_pattern, attribute.mbc_str() );
                    }
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_webserver_ssipattern"), 
                                                    wxString::Format(_("%s"), 
                                                    m_ssi_pattern ) );

                    attribute = subchild->GetAttribute(wxT("ip_acl"), wxT(""));
                    attribute.Trim();
                    attribute.Trim(false);
                    if ( attribute.Length() ) {
                        strcpy( m_ip_acl, attribute.mbc_str() );
                    }
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_webserver_ipacl"), 
                                                    wxString::Format(_("%s"), 
                                                    m_ip_acl ) );

                    attribute = subchild->GetAttribute(wxT("dav_document_root"), wxT(""));
                    attribute.Trim();
                    attribute.Trim(false);
                    if ( attribute.Length() ) {
                        strcpy( m_dav_document_root, attribute.mbc_str() );
                    }
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_webserver_davdocumentroot"), 
                                                    wxString::Format(_("%s"), 
                                                    m_dav_document_root ) );

                    attribute = subchild->GetAttribute(wxT("run_as_user"), wxT(""));
                    attribute.Trim();
                    attribute.Trim(false);
                    m_runAsUserWeb = attribute;

                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_webserver_runasuser"), 
                                                    wxString::Format(_("%s"), 
                                                    (const char *)m_runAsUserWeb.mbc_str() ) );

                    // Get webserver sub components
                    wxXmlNode *subsubchild = subchild->GetChildren();
                    while (subsubchild) {

                        // Deprecated (moved outside of webserver)
                        if (subsubchild->GetName() == wxT("websockets")) {

                            wxString property;

                            property = subsubchild->GetAttribute(wxT("auth"), wxT("true"));

                                if (property.IsSameAs(_("false"), false)) {
                                 m_bAuthWebsockets = false;
                            }
                            
                            // Write into settings database
                            updateConfigurationRecordItem( _("vscpd_websocket_enableauth"), 
                                                    wxString::Format(_("%d"), 
                                                    m_bAuthWebsockets ? 1 : 0 ) );

                        }

                        subsubchild = subsubchild->GetNext();
                    }

                }
                else if (subchild->GetName() == wxT("websockets")) {

                    wxString property;

                    property =subchild->GetAttribute(wxT("auth"), wxT("true"));

                    if (property.IsSameAs(_("false"), false)) {
                        m_bAuthWebsockets = false;
                    }
                            
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_websocket_enableauth"), 
                                                    wxString::Format(_("%d"), 
                                                    m_bAuthWebsockets ? 1 : 0 ) );

                }

                subchild = subchild->GetNext();
                
            }

            wxString content = child->GetNodeContent();

        }
        
        else if ( child->GetName() == wxT("remoteuser") ) {

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

                vscp_clearVSCPFilter(&VSCPFilter); // Allow all frames

                if ( subchild->GetName() == wxT("user") ) {

                    wxXmlNode *subsubchild = subchild->GetChildren();

                    while (subsubchild) {

                        if (subsubchild->GetName() == wxT("name")) {
                            name = subsubchild->GetNodeContent();
                            bUser = true;
                        }
                        else if (subsubchild->GetName() == wxT("password")) {
                            md5 = subsubchild->GetNodeContent();
                        }
                        else if (subsubchild->GetName() == wxT("privilege")) {
                            privilege = subsubchild->GetNodeContent();
                        }
                        else if (subsubchild->GetName() == wxT("filter")) {
                            bFilterPresent = true;

                            wxString str_vscp_priority = subchild->GetAttribute(wxT("priority"), wxT("0"));

                            val = 0;
                            str_vscp_priority.ToULong(&val);
                            VSCPFilter.filter_priority = val;

                            wxString str_vscp_class = subchild->GetAttribute(wxT("class"), wxT("0"));

                            val = 0;
                            str_vscp_class.ToULong(&val);
                            VSCPFilter.filter_class = val;

                            wxString str_vscp_type = subchild->GetAttribute(wxT("type"), wxT("0"));

                            val = 0;
                            str_vscp_type.ToULong(&val);
                            VSCPFilter.filter_type = val;

                            wxString str_vscp_guid = subchild->GetAttribute(wxT("guid"),
                                                        wxT("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"));
                            vscp_getGuidFromStringToArray(VSCPFilter.filter_GUID, str_vscp_guid);
                        }
                        else if (subsubchild->GetName() == wxT("mask")) {

                            bMaskPresent = true;

                            wxString str_vscp_priority = subchild->GetAttribute(wxT("priority"), wxT("0"));

                            val = 0;
                            str_vscp_priority.ToULong(&val);
                            VSCPFilter.mask_priority = val;

                            wxString str_vscp_class = subchild->GetAttribute(wxT("class"), wxT("0"));

                            val = 0;
                            str_vscp_class.ToULong(&val);
                            VSCPFilter.mask_class = val;

                            wxString str_vscp_type = subchild->GetAttribute(wxT("type"), wxT("0"));

                            val = 0;
                            str_vscp_type.ToULong(&val);
                            VSCPFilter.mask_type = val;

                            wxString str_vscp_guid = subchild->GetAttribute(wxT("guid"),
                                    wxT("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"));
                            vscp_getGuidFromStringToArray(VSCPFilter.mask_GUID, str_vscp_guid);
                        }
                        else if (subsubchild->GetName() == wxT("allowfrom")) {
                            allowfrom = subsubchild->GetNodeContent();
                        }
                        else if (subsubchild->GetName() == wxT("allowevent")) {
                            allowevent = subsubchild->GetNodeContent();
                        }

                        subsubchild = subsubchild->GetNext();

                    }

                }

                // Add user
                if ( bUser ) {

                    if (bFilterPresent && bMaskPresent) {
                        m_userList.addUser(name, md5, _(""), &VSCPFilter, privilege, allowfrom, allowevent);
                    }
                    else {
                        m_userList.addUser(name, md5, _(""), NULL, privilege, allowfrom, allowevent);
                    }

                    bUser = false;
                    bFilterPresent = false;
                    bMaskPresent = false;

                }

                subchild = subchild->GetNext();

            }

        }

        // Level I driver
        else if ( ( child->GetName() == wxT("canaldriver") ) || ( child->GetName() == wxT("level1driver") ) ) {

            wxXmlNode *subchild = child->GetChildren();
            while (subchild) {

                wxString strName;
                wxString strConfig;
                wxString strPath;
                unsigned long flags;
                cguid guid;
                bool bEnabled = false;
                bool bCanalDriver = false;

                if ( subchild->GetName() == wxT("driver") ) {

                    wxXmlNode *subsubchild = subchild->GetChildren();

                    wxString attribute = subchild->GetAttribute(wxT("enable"), wxT("false"));

                    if ( attribute.IsSameAs( _("true"), false ) ) {
                        bEnabled = true;
                    }

                    while (subsubchild) {

                        if ( subsubchild->GetName() == wxT("name") ) {
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
                        else if ( subsubchild->GetName() == wxT("config") ||
                                subsubchild->GetName() == wxT("parameter") ) {
                            strConfig = subsubchild->GetNodeContent();
                            strConfig.Trim();
                            strConfig.Trim(false);
                        }
                        else if ( subsubchild->GetName() == wxT("path") ) {
                            strPath = subsubchild->GetNodeContent();
                            strPath.Trim();
                            strPath.Trim(false);
                        }
                        else if ( subsubchild->GetName() == wxT("flags") ) {
                            wxString str = subsubchild->GetNodeContent();
                            flags = vscp_readStringValue(str);
                        }
                        else if ( subsubchild->GetName() == wxT("guid") ) {
                            guid.getFromString( subsubchild->GetNodeContent() );
                        }
                        else if ( subsubchild->GetName() == wxT( "known-nodes" ) ) {

                            wxXmlNode *subsubsubchild = subchild->GetChildren();

                            while ( subsubsubchild ) {

                                if ( subsubsubchild->GetName() == wxT( "node" ) ) {
                                    cguid knownguid;

                                    knownguid.getFromString( subchild->GetAttribute( wxT( "guid" ), wxT( "-" ) ) );
                                    wxString name = subchild->GetAttribute( wxT( "name" ), wxT( "" )  ) ;
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

                // Add the device
                if (bCanalDriver && bEnabled ) {

                    if (!m_deviceList.addItem( strName,
                                                strConfig,
                                                strPath,
                                                flags,
                                                guid,
                                                VSCP_DRIVER_LEVEL1,
                                                bEnabled ) ) {
                        wxString errMsg = _("Driver not added. Path does not exist. - [ ") +
                                strPath + _(" ]\n");
                        logMsg(errMsg);
                        //wxLogDebug(errMsg);
                    }
                    else {
                        wxString errMsg = _("Level I driver added. - [ ") +
                                strPath + _(" ]\n");
                        logMsg(errMsg);
                    }

                    bCanalDriver = false;

                }

                // Next driver
                subchild = subchild->GetNext();

            }

        }
        // Level II driver
        else if ( ( child->GetName() == wxT("vscpdriver") ) || ( child->GetName() == wxT("level2driver") ) ) {

            wxXmlNode *subchild = child->GetChildren();

            while ( subchild ) {

                wxString strName;
                wxString strConfig;
                wxString strPath;
                cguid guid;
                bool bEnabled = false;
                bool bLevel2Driver = false;

                if ( subchild->GetName() == wxT("driver") ) {

                    wxXmlNode *subsubchild = subchild->GetChildren();

                    wxString attribute = subchild->GetAttribute(wxT("enable"), wxT("false"));

                    if (attribute.IsSameAs(_("true"), false)) {
                        bEnabled = true;
                    }

                    while ( subsubchild ) {
                        
                        if (subsubchild->GetName() == wxT("name")) {
                            
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
                        else if (subsubchild->GetName() == wxT("config") ||
                                subsubchild->GetName() == wxT("parameter")) {
                            
                            strConfig = subsubchild->GetNodeContent();
                            strConfig.Trim();
                            strConfig.Trim(false);
                            
                        }
                        else if (subsubchild->GetName() == wxT("path")) {
                            
                            strPath = subsubchild->GetNodeContent();
                            strPath.Trim();
                            strPath.Trim(false);
                            
                        }
                        else if (subsubchild->GetName() == wxT("guid")) {
                            guid.getFromString( subsubchild->GetNodeContent() );
                        }
                        else if ( subsubchild->GetName() == wxT( "known-nodes" ) ) {

                            wxXmlNode *subsubsubchild = subchild->GetChildren();

                            while ( subsubsubchild ) {

                                if ( subsubsubchild->GetName() == wxT( "node" ) ) {
                                    cguid knownguid;

                                    knownguid.getFromString( subchild->GetAttribute( wxT( "guid" ), wxT( "-" ) ) );
                                    wxString name = subchild->GetAttribute( wxT( "name" ), wxT( "" ) );
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

                // Add the device
                if ( bLevel2Driver && bEnabled ) {

                    if (!m_deviceList.addItem(strName,
                                                strConfig,
                                                strPath,
                                                0,
                                                guid,
                                                VSCP_DRIVER_LEVEL2,
                                                bEnabled )) {
                        wxString errMsg = _("Driver not added. Path does not exist. - [ ") +
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
        // <table name="jhjhdjhsdjh" description="jsjdsjhdhsjh" xaxis="lfdlfldk"
        //      yaxis="dfddfd" path="path" type="normal|static" size="n" class="n"
        //      type="n" unit="n" />
        else if (child->GetName() == wxT("tables")) {

            wxXmlNode *subchild = child->GetChildren();
            while (subchild) {

                if (subchild->GetName() == wxT("table")) {

                    int nType = VSCP_TABLE_DYNAMIC;
                    wxString attribute =  subchild->GetAttribute( wxT("type"), wxT("0") );
                    attribute.MakeUpper();
                    if ( wxNOT_FOUND  != attribute.Find(_("DYNAMIC")) ) {
                        nType = VSCP_TABLE_DYNAMIC;
                    }
                    else if ( wxNOT_FOUND  != attribute.Find(_("STATIC")) ) {
                        nType = VSCP_TABLE_STATIC;
                    }
                    else {
                        nType = vscp_readStringValue( attribute );
                    }

                    CVSCPTable *pTable = new CVSCPTable();
                    if ( NULL != pTable ) {
                        memset( &pTable->m_vscpFileHead, 0, sizeof(_vscptableInfo) );
                        pTable->setTableInfo( subchild->GetAttribute( wxT("path"), wxT("") ).mbc_str(),
                                                    nType,
                                                    subchild->GetAttribute( wxT("name"), wxT("") ).Upper().mbc_str(),
                                                    subchild->GetAttribute( wxT("description"), wxT("") ).mbc_str(),
                                                    subchild->GetAttribute( wxT("labelx"), wxT("") ).mbc_str(),
                                                    subchild->GetAttribute( wxT("labely"), wxT("") ).mbc_str(),
                                                    vscp_readStringValue( subchild->GetAttribute( wxT("size"), wxT("0") ) ),
                                                    vscp_readStringValue( subchild->GetAttribute( wxT("vscpclass"), wxT("10") ) ),
                                                    vscp_readStringValue( subchild->GetAttribute( wxT("vscptype"), wxT("6") ) ),
                                                    vscp_readStringValue( subchild->GetAttribute( wxT("vscpunit"), wxT("0") ) ) );
                        m_mutexTableList.Lock();
                        m_listTables.Append( pTable ) ;
                        m_mutexTableList.Unlock();
                    }
                }

                subchild = subchild->GetNext();

            } // while

        }
        else if ( child->GetName() == wxT("automation") ) {

            wxString attribut = child->GetAttribute(wxT("enable"), wxT("true"));
            attribut.MakeLower();
            if (attribut.IsSameAs(_("false"), false)) {
                m_automation.disableAutomation();
            }
            else {
                m_automation.enableAutomation();
            }
            
            // Write into settings database
            updateConfigurationRecordItem( _("vscpd_automation_enable"), 
                                            wxString::Format(_("%d"), 
                                            m_automation.isAutomationEnabled() ? 1 : 0 ) );

            wxXmlNode *subchild = child->GetChildren();
            while (subchild) {

                if (subchild->GetName() == wxT("zone")) {
                    
                    long zone;
                    wxString strZone = subchild->GetNodeContent();
                    strZone.ToLong( &zone );
                    m_automation.setZone( zone );
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_automation_zone"), 
                                            wxString::Format(_("%d"), 
                                            m_automation.getZone() ) );
            
                }
                else if (subchild->GetName() == wxT("sub-zone")) {
                    
                    long subzone;
                    wxString strSubZone = subchild->GetNodeContent();
                    strSubZone.ToLong( &subzone );
                    m_automation.setSubzone( subzone );
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_automation_subzone"), 
                                            wxString::Format(_("%d"), 
                                            m_automation.getSubzone() ) );
                    
                }
                else if (subchild->GetName() == wxT("longitude")) {
                    
                    wxString strLongitude = subchild->GetNodeContent();
                    double d;
                    strLongitude.ToDouble( &d );
                    m_automation.setLongitude( d );
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_automation_longitude"), 
                                            wxString::Format(_("%f"), 
                                            m_automation.getLongitude() ) );
                }
                else if (subchild->GetName() == wxT("latitude")) {
                    
                    wxString strLatitude = subchild->GetNodeContent();
                    double d;
                    strLatitude.ToDouble( &d );
                    m_automation.setLatitude( d );
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_automation_latitude"), 
                                            wxString::Format(_("%f"), 
                                            m_automation.getLatitude() ) );
                    
                }
                else if (subchild->GetName() == wxT("sunrise")) {
                    
                    m_automation.enableSunRiseEvent();
                    wxString attribute = subchild->GetAttribute(wxT("enable"), wxT("true"));
                    if (attribute.IsSameAs(_("false"), false)) {
                        m_automation.disableSunRiseEvent();
                    }
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_automation_sunrise_enable"), 
                                            wxString::Format(_("%d"), 
                                            m_automation.isSendSunriseEvent() ? 1 : 0 ) );
                    
                }
                
                else if (subchild->GetName() == wxT("sunrise-twilight")) {
                    
                    m_automation.enableSunRiseTwilightEvent();
                    wxString attribute = subchild->GetAttribute(wxT("enable"), wxT("true"));
                    if (attribute.IsSameAs(_("false"), false)) {
                        m_automation.disableSunRiseTwilightEvent();
                    }
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_automation_sunrisetwilight_enable"), 
                                            wxString::Format(_("%d"), 
                                            m_automation.isSendSunriseTwilightEvent() ? 1 : 0 ) );
                    
                }
                else if (subchild->GetName() == wxT("sunset")) {
                    
                    m_automation.enableSunSetEvent();
                    wxString attribute = subchild->GetAttribute(wxT("enable"), wxT("true"));
                    if (attribute.IsSameAs(_("false"), false)) {
                        m_automation.disableSunSetEvent();
                    }
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_automation_sunset_enable"), 
                                            wxString::Format(_("%d"), 
                                            m_automation.isSendSunsetEvent() ? 1 : 0 ) );
                    
                }
                else if (subchild->GetName() == wxT("sunset-twilight")) {
                    
                    m_automation.enableSunSetTwilightEvent();
                    wxString attribute = subchild->GetAttribute(wxT("enable"), wxT("true"));
                    if (attribute.IsSameAs(_("false"), false)) {
                        m_automation.disableSunSetTwilightEvent();
                    }
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_automation_sunrisetwilight_enable"), 
                                            wxString::Format(_("%d"), 
                                            m_automation.isSendSunsetTwilightEvent() ? 1 : 0 ) );
                    
                }
                else if (subchild->GetName() == wxT("segmentcontrol-event")) {
                    
                    m_automation.enableSegmentControllerHeartbeat();
                    wxString attribute = subchild->GetAttribute(wxT("enable"), wxT("true"));
                    if (attribute.IsSameAs(_("false"), false)) {
                        m_automation.disableSegmentControllerHeartbeat();
                    }

                    attribute = subchild->GetAttribute(wxT("interval"), wxT("60"));
                    long interval;
                    attribute.ToLong( &interval );
                    m_automation.setSegmentControllerHeartbeatInterval( interval );
                    
                    // Write into settings database
                    updateConfigurationRecordItem( _("vscpd_automation_segmentcontrollerevent_enable"), 
                                            wxString::Format(_("%d"), 
                                            m_automation.isSegmentControllerHeartbeat() ? 1 : 0 ) );
                    
                    updateConfigurationRecordItem( _("vscpd_automation_segmentcontrollerevent_interval"), 
                                            wxString::Format(_("%ld"), 
                                            m_automation.getSegmentControllerHeartbeatInterval() ) );
                    
                }
                else if (subchild->GetName() == wxT("heartbeat-event")) {
                    
                    m_automation.enableHeartbeatEvent();
                    m_automation.setHeartbeatEventInterval( 30 );

                    wxString attribute = subchild->GetAttribute(wxT("enable"), wxT("true"));
                    if (attribute.IsSameAs(_("false"), false)) {
                        m_automation.disableHeartbeatEvent();
                    }

                    attribute = subchild->GetAttribute(wxT("interval"), wxT("30"));
                    long interval;
                    attribute.ToLong( &interval );
                    m_automation.setHeartbeatEventInterval( interval );
                    
                    // Write into settings database
                    updateConfigurationRecordItem(_("vscpd_automation_heartbeatevent_enable"), 
                                            wxString::Format(_("%d"), 
                                            m_automation.isHeartbeatEvent() ? 1 : 0 ) );
                    
                    updateConfigurationRecordItem(_("vscpd_automation_heartbeatevent_interval"), 
                                            wxString::Format(_("%ld"), 
                                            m_automation.getHeartbeatEventInterval() ) );
                    
                }

                subchild = subchild->GetNext();

            } // while

        }

        child = child->GetNext();

    }

    return true;
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
    const char *psql = VSCPDB_CONFIG_CREATE;
    
    fprintf( stderr, "Creating settings database..\n" );
    
    // Check if database is open
    if ( NULL == m_db_vscp_daemon ) return false;
    
    m_db_vscp_configMutex.Lock();
    
    if ( SQLITE_OK  !=  sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg ) ) {
        fprintf( stderr, 
                    "Creation of the VSCP settings database failed with message %s", 
                    pErrMsg );
        return false;
    }
    
    fprintf( stderr, "Writing default database content..\n" );

    // Add default settings (set as defaults in SQL create expression))
    psql = " INSERT INTO 'settings' (vscpd_dbversion) VALUES ( 1 )";
    if ( SQLITE_OK  !=  sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg ) ) {
        fprintf( stderr, 
                    "Failed to insert default settings data with error %s.\n",
                    pErrMsg ); 
        return false;
    }
    
    m_db_vscp_configMutex.Unlock();
    
    return true;
}


///////////////////////////////////////////////////////////////////////////////
// dbReadConfiguration
//
// Read the configuration database record
//
// Note that fprintf needs to be used here as the logging mechanism
// is not activated yet.
//

bool CControlObject::dbReadConfiguration( void )
{
    char *pErrMsg = 0;
    const char *psql = "SELECT * from SETTINGS";
    sqlite3_stmt *ppStmt;

    // Check if database is open
    if ( NULL == m_db_vscp_daemon ) {
        fprintf( stderr, 
                    "Failed to read VSCP settings database - not open." );
        return false;
    }
    
    if ( SQLITE_OK != sqlite3_prepare( m_db_vscp_daemon,
                                        psql,
                                        -1,
                                        &ppStmt,
                                        NULL ) ) {
        fprintf( stderr, 
                    "Failed to read VSCP settings database - prepare query." );
        return false;
    }
    
    if ( SQLITE_ROW  == sqlite3_step( ppStmt ) ) {
        
        const unsigned char * p;
        
        // Get the version of this db file
        int dbVersion = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_CONFIG_DBVERSION );
        
        // Debug level
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_LOGLEVEL ) ) {
            m_logLevel = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_CONFIG_LOGLEVEL );
            if ( m_logLevel > DAEMON_LOGMSG_DEBUG ) {
                m_logLevel = DAEMON_LOGMSG_DEBUG;
            }
        }

        // Run as user
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_RUNASUSER ) ) {
            m_runAsUser = wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, 
                    VSCPDB_ORDINAL_CONFIG_RUNASUSER ) );
        }
        
        // Server GUID
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_GUID ) ) {
            m_guid.getFromString( (const char *)sqlite3_column_text( ppStmt, 
                    VSCPDB_ORDINAL_CONFIG_GUID ) );
        }
        
        // Server name
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_SERVER_NAME ) ) {
            m_strServerName = wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, 
                    VSCPDB_ORDINAL_CONFIG_SERVER_NAME ) );
        }
         
        // Syslog enable
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_SYSLOG_ENABLE ) ) {
            m_bLogToSysLog = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_CONFIG_SYSLOG_ENABLE ) ? true : false;
        }
        
        // Database logging enable
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_LOGDB_ENABLE ) ) {
            m_bLogToDatabase = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_CONFIG_LOGDB_ENABLE ) ? true : false;
        }
        
        // General log file enable
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_GENERALLOGFILE_ENABLE ) ) {
            m_bLogGeneralEnable = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_CONFIG_GENERALLOGFILE_ENABLE ) ? true : false;
        }
        
        // Path for general log file
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_GENERALLOGFILE_PATH ) ) {
            m_logGeneralFileName.Assign( wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, 
                                                                VSCPDB_ORDINAL_CONFIG_GENERALLOGFILE_PATH ) ) );
        }
        
        // Security log file enable
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_SECURITYLOGFILE_ENABLE ) ) {
            m_bLogSecurityEnable = sqlite3_column_int( ppStmt, 
                                                          VSCPDB_ORDINAL_CONFIG_SECURITYLOGFILE_ENABLE ) ? true : false;
        }
        
        // Path to security log file
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_SECURITYLOGFILE_PATH ) ) {
            m_logSecurityFileName.Assign( wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt,  
                                            VSCPDB_ORDINAL_CONFIG_SECURITYLOGFILE_PATH ) ) );
        }
        
        // Access log file enable
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_ACCESSLOGFILE_ENABLE ) ) {
            m_bLogAccessEnable = sqlite3_column_int( ppStmt,
                                                    VSCPDB_ORDINAL_CONFIG_ACCESSLOGFILE_ENABLE ) ? true : false;
        }
        
        // Path to access log file
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_ACCESSLOGFILE_PATH ) ) {
            m_logAccessFileName.Assign( wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, 
                                                            VSCPDB_ORDINAL_CONFIG_ACCESSLOGFILE_PATH ) ) );
        }
        
        // TCP/IP port
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_TCPIPINTERFACE_PORT ) ) {
            m_strTcpInterfaceAddress = wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, 
                                        VSCPDB_ORDINAL_CONFIG_TCPIPINTERFACE_PORT ) );
        }
        
        // Port for Multicast interface
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_MULTICASTINTERFACE_PORT ) ) {
            m_strMulticastAnnounceAddress = wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, 
                                            VSCPDB_ORDINAL_CONFIG_MULTICASTINTERFACE_PORT ) );
        }
        
        // TTL for Multicast i/f
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_MULICASTINTERFACE_TTL ) ) {
            m_ttlMultiCastAnnounce = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_CONFIG_MULICASTINTERFACE_TTL );
        }
        
        // Enable UDP interface
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_UDPSIMPLEINTERFACE_ENABLE ) ) {
            m_bUDP = sqlite3_column_int( ppStmt, 
                                        VSCPDB_ORDINAL_CONFIG_UDPSIMPLEINTERFACE_ENABLE ) ? true : false;
        }
        
        // UDP interface port
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_UDPSIMPLEINTERFACE_PORT ) ) {
            m_strUDPInterfaceAddress = wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, 
                                            VSCPDB_ORDINAL_CONFIG_UDPSIMPLEINTERFACE_PORT ) );
        }

        // Path to DM database file
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_DM_DB_PATH ) ) {
            m_dm.m_path_db_vscp_dm.Assign( wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, 
                                        VSCPDB_ORDINAL_CONFIG_DM_DB_PATH ) ) );
        }
        
        // Path to DM XML file
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_DM_XML_PATH ) ) {
            m_dm.m_staticXMLPath = wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, 
                                        VSCPDB_ORDINAL_CONFIG_DM_XML_PATH ) );
        }
  
        // Enable DM logging
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_DM_LOGGING_ENABLE ) ) {
            m_dm.m_bLogEnable = sqlite3_column_int( ppStmt, 
                                        VSCPDB_ORDINAL_CONFIG_DM_LOGGING_ENABLE ) ? true : false;
        }
        
        // Path to DM log file
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_DM_LOGGING_PATH ) ) {
            m_dm.m_logPath = wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, 
                                        VSCPDB_ORDINAL_CONFIG_DM_LOGGING_PATH ) );
        }
        
        // DM logging level
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_DM_LOGGING_LEVEL ) ) {
            m_dm.m_logLevel = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_CONFIG_DM_LOGGING_LEVEL );
        }
        
        // Path to variable database
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_VARIABLES_DB_PATH ) ) {
            m_VSCP_Variables.m_dbFilename.Assign( wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, 
                                        VSCPDB_ORDINAL_CONFIG_VARIABLES_DB_PATH ) ) );
        }
        
        // Path to variable XML
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_VARIABLES_XML_PATH ) ) {
            m_VSCP_Variables.m_xmlPath = wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, 
                                        VSCPDB_ORDINAL_CONFIG_VARIABLES_XML_PATH ) );
        }
        
        // Client buffer size
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_VSCPD_DEFAULTCLIENTBUFFERSIZE ) ) {
            m_maxItemsInClientReceiveQueue = sqlite3_column_int( ppStmt, 
                                        VSCPDB_ORDINAL_CONFIG_VSCPD_DEFAULTCLIENTBUFFERSIZE );
        }
 
        // Disable web server security
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_WEBSERVER_DISABLEAUTHENTICATION ) ) {
            m_bDisableSecurityWebServer = sqlite3_column_int( ppStmt,  
                                        VSCPDB_ORDINAL_CONFIG_WEBSERVER_DISABLEAUTHENTICATION ) ? true : false;
        }
        
        // Web server root path
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_WEBSERVER_ROOTPATH ) ) {
            p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_WEBSERVER_ROOTPATH );
            if ( NULL != p ) {
                strncpy( m_pathWebRoot, 
                            (const char *)p, 
                            MIN( strlen( (const char *)p ), MAX_PATH_SIZE ) );
            }
        }
        
        // Port for web server
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_WEBSERVER_PORT ) ) {
            m_strWebServerInterfaceAddress = wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt,  
                                            VSCPDB_ORDINAL_CONFIG_WEBSERVER_PORT ) );
        }
        
        // Path to cert file
        p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_WEBSERVER_PATHCERT );
        if ( NULL != p ) {
            strncpy( m_pathCert, 
                        (const char *)p, 
                        MIN( strlen( (const char *)p ), MAX_PATH_SIZE ) );
        }
        
        // Authdomain
        p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_WEBSERVER_AUTHDOMAIN );
        if ( NULL != p ) {
            strncpy( m_authDomain, 
                        (const char *)p, 
                        MIN( strlen( (const char *)p ), MAX_PATH_SIZE ) );
        }
        
        // CGI interpreter
        p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_WEBSERVER_CGIINTERPRETER );
        if ( NULL != p ) {
            strncpy( m_cgiInterpreter, 
                        (const char *)p, 
                        MIN( strlen( (const char *)p ), MAX_PATH_SIZE ) );
        }
       
        // CGI pattern
        p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_WEBSERVER_CGIPATTERN );
        if ( NULL != p ) {
            strncpy( m_cgiPattern, 
                        (const char *)p, 
                        MIN( strlen( (const char *)p ), MAX_PATH_SIZE ) );
        }
        
        // Enable directory listings
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_WEBSERVER_ENABLEDIRECTORYLISTINGS ) ) {
            if ( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_CONFIG_WEBSERVER_ENABLEDIRECTORYLISTINGS ) ) {
                strcpy( m_EnableDirectoryListings, "yes" );
            }
            else {
                strcpy( m_EnableDirectoryListings, "no" );
            }
        }
   
        // Hide file patterns
        p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_WEBSERVER_HIDEFILEPATTERNS );
        if ( NULL != p ) {
            strncpy( m_hideFilePatterns, 
                        (const char *)p, 
                        MIN( strlen( (const char *)p ), MAX_PATH_SIZE ) );
        }
        
        // Index files
        p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_WEBSERVER_INDEXFILES );
        if ( NULL != p ) {
            strncpy( m_indexFiles, 
                        (const char *)p, 
                        MIN( strlen( (const char *)p ), MAX_PATH_SIZE ) );
        }

        // Extra mime types
        p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_WEBSERVER_EXTRAMIMETYPES );
        if ( NULL != p ) {
            strncpy( m_extraMimeTypes, 
                        (const char *)p, 
                        MIN( strlen( (const char *)p ), MAX_PATH_SIZE ) );
        }
        
        // URL rewrites
        p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_WEBSERVER_URLREWRITES );
        if ( NULL != p ) {
            strncpy( m_urlRewrites, 
                        (const char *)p, 
                        MIN( strlen( (const char *)p ), MAX_PATH_SIZE ) );
        }
        
        // SSI patterns
        p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_WEBSERVER_SSIPATTERN );
        if ( NULL != p ) {
            strncpy( m_ssi_pattern, 
                        (const char *)p, 
                        MIN( strlen( (const char *)p ), MAX_PATH_SIZE ) );
        }
        
        // Webserver user
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_WEBSERVER_RUNASUSER ) ) {
            m_runAsUserWeb = wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, 
                        VSCPDB_ORDINAL_CONFIG_WEBSERVER_RUNASUSER ) );
        }
        
        // Per directory auth. file
        p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_WEBSERVER_PERDIRECTORYAUTHFILE );
        if ( NULL != p ) {
            strncpy( m_per_directory_auth_file, 
                        (const char *)p, 
                        MIN( strlen( (const char *)p ), MAX_PATH_SIZE ) );
        }
        
        // Global auth. file
        p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_WEBSERVER_GLOBALAUTHFILE );
        if ( NULL != p ) {
            strncpy( m_global_auth_file, 
                        (const char *)p, 
                        MIN( strlen( (const char *)p ), MAX_PATH_SIZE ) );
        }

        // IP ACL
        p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_WEBSERVER__IPACL );
        if ( NULL != p ) {
            strncpy( m_ip_acl, 
                        (const char *)p, 
                        MIN( strlen( (const char *)p ), MAX_PATH_SIZE ) );
        }
        
        // DAV path
        p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_WEBSERVER_DAVDOCUMENTROOT );
        if ( NULL != p ) {
            strncpy( m_dav_document_root, 
                        (const char *)p, 
                        MIN( strlen( (const char *)p ), MAX_PATH_SIZE ) );
        }
        
        // Enable web socket authentication
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_WEBSOCKET_ENABLEAUTH ) ) {
            m_bAuthWebsockets = sqlite3_column_int( ppStmt, 
                    VSCPDB_ORDINAL_CONFIG_WEBSOCKET_ENABLEAUTH ) ? true : false;
        }
        
        
        // Enable MQTT broker
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_MQTTBROKER_ENABLE ) ) {
            m_bMQTTBroker = sqlite3_column_int( ppStmt, 
                    VSCPDB_ORDINAL_CONFIG_MQTTBROKER_ENABLE ) ? true : false;
        }
        
        // MQTT broker port
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_MQTTBROKER_PORT ) ) {
            m_strMQTTBrokerInterfaceAddress = 
                    wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, 
                    VSCPDB_ORDINAL_CONFIG_MQTTBROKER_PORT ) );
        }
        
        // Enable COAP server
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_COAPSERVER_ENABLE ) ) {
            m_bCoAPServer = sqlite3_column_int( ppStmt, 
                                    VSCPDB_ORDINAL_CONFIG_COAPSERVER_ENABLE ) ? true : false;
        }
        
        // COAP port
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_COAPSERVER_PORT ) ) {
            m_strCoAPServerInterfaceAddress = wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, 
                                    VSCPDB_ORDINAL_CONFIG_COAPSERVER_PORT ) );
        }
        
        // Automation zone
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_AUTOMATION_ZONE ) ) {
            m_automation.setZone( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_CONFIG_AUTOMATION_ZONE ) );
        }
        
        // Automation sub zone
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_AUTOMATION_SUBZONE ) ) {
            m_automation.setZone( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_CONFIG_AUTOMATION_SUBZONE ) );
        }
        
        // Automation longitude
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_AUTOMATION_LONGITUDE ) ) {
            m_automation.setLongitude( sqlite3_column_double( ppStmt, VSCPDB_ORDINAL_CONFIG_AUTOMATION_LONGITUDE ) );
        }
        
        // Automation latitude
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_AUTOMATION_LATITUDE ) ) {
            m_automation.setLongitude( sqlite3_column_double( ppStmt, VSCPDB_ORDINAL_CONFIG_AUTOMATION_LATITUDE ) );
        }
        
        // Automation enable sun rise event
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_AUTOMATION_SUNRISE_ENABLE ) ) {
            if ( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_CONFIG_AUTOMATION_SUNRISE_ENABLE ) ) {
                m_automation.enableSunRiseEvent();
            }
            else {
                m_automation.disableSunRiseEvent();
            }
        }
        
        // Automation enable sun set event
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_AUTOMATION_SUNSET_ENABLE ) ) {
            if ( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_CONFIG_AUTOMATION_SUNSET_ENABLE ) ) {
                m_automation.enableSunSetEvent();
            }
            else {
                m_automation.disableSunSetEvent();
            }
        }
        
        // Automation enable sunset twilight event
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_AUTOMATION_SUNSETTWILIGHT_ENABLE ) ) {
            if ( sqlite3_column_int( ppStmt, 
                                VSCPDB_ORDINAL_CONFIG_AUTOMATION_SUNSETTWILIGHT_ENABLE ) ) {
                m_automation.enableSunSetTwilightEvent();
            }
            else {
                m_automation.disableSunSetTwilightEvent();
            }
        }
        
        // Automation enable sunrise twilight event
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_AUTOMATION_SUNRISETWILIGHT_ENABLE ) ) {
            if ( sqlite3_column_int( ppStmt, 
                                VSCPDB_ORDINAL_CONFIG_AUTOMATION_SUNRISETWILIGHT_ENABLE ) ) {
                m_automation.enableSunRiseTwilightEvent();
            }
            else {
                m_automation.disableSunRiseTwilightEvent();
            }
        }
        
        // Automation segment controller event enable
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_AUTOMATION_SEGMENTCONTROLLEREVENT_ENABLE ) ) {
            if ( sqlite3_column_int( ppStmt, 
                                VSCPDB_ORDINAL_CONFIG_AUTOMATION_SEGMENTCONTROLLEREVENT_ENABLE ) ) {
                m_automation.enableSegmentControllerHeartbeat();
            }
            else {
                m_automation.disableSegmentControllerHeartbeat();
            }
        }
        
        // Automation, segment controller heartbeat interval
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_AUTOMATION_SEGMENTCONTROLLEREVENT_INTERVAL ) ) {
            m_automation.setSegmentControllerHeartbeatInterval( sqlite3_column_int( ppStmt, 
                                VSCPDB_ORDINAL_CONFIG_AUTOMATION_SEGMENTCONTROLLEREVENT_INTERVAL ) );
        }
        
        // Automation heartbeat event enable
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_AUTOMATION_HEARTBEATEVENT_ENABLE ) ) {
            if ( sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_CONFIG_AUTOMATION_HEARTBEATEVENT_ENABLE ) ) {
                m_automation.enableHeartbeatEvent();
            }
            else {
                m_automation.disableHeartbeatEvent();
            }
        }
        
        // Automation heartbeat interval
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_AUTOMATION_HEARTBEATEVENT_INTERVAL ) ) {
            m_automation.setHeartbeatEventInterval( sqlite3_column_int( ppStmt, 
                                VSCPDB_ORDINAL_CONFIG_AUTOMATION_HEARTBEATEVENT_INTERVAL ) );
        }
        
        // VSCP data database path
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_DB_VSCPDATA_PATH ) ) {    
            m_path_db_vscp_data.Assign( wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, 
                                    VSCPDB_ORDINAL_CONFIG_DB_VSCPDATA_PATH ) ) );
        }
        
        // VSCP log database path
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_DB_LOG_PATH ) ) {
            m_path_db_vscp_log.Assign( wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, 
                                VSCPDB_ORDINAL_CONFIG_DB_LOG_PATH ) ) );
        }
        
    }
    
    sqlite3_finalize( ppStmt );

    return true;
}

///////////////////////////////////////////////////////////////////////////////
// updateConfigurationRecordItem
//

bool CControlObject::updateConfigurationRecordItem( const wxString& strUpdateField, 
                                                        const wxString& strUpdateValue )
{
    char *pErrMsg;
            
    // Database file must be open
    if ( NULL == m_db_vscp_daemon ) {
        logMsg( _("Settings update: Update record. Database file is not open.\n") );
        return false;
    }
    
    m_db_vscp_configMutex.Lock();  
    
    char *sql = sqlite3_mprintf( VSCPDB_CONFIG_UPDATE_ITEM, 
                                    (const char *)strUpdateField.mbc_str(),
                                    (const char *)strUpdateValue.mbc_str(),
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
// doCreateLogTable
//
// Create the log database
//
//

bool CControlObject::doCreateLogTable( void )
{
    char *pErrMsg = 0;
    const char *psql = VSCPDB_LOG_CREATE;
    
    fprintf( stderr, "Creating database log table..\n" );
    
    // Check if database is open
    if ( NULL == m_db_vscp_log ) {
        fprintf( stderr, 
                    "Failed to create VSCP log database - closed.\n" );
        return false;
    }
    
    m_db_vscp_configMutex.Lock();
    
    if ( SQLITE_OK  !=  sqlite3_exec(m_db_vscp_log, psql, NULL, NULL, &pErrMsg ) ) {
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
    
    if ( SQLITE_OK  !=  sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg ) ) {
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
    
    if ( SQLITE_OK  !=  sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg ) ) {
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
    
    fprintf( stderr, "Creating discovery table..\n" );
    
    // Check if database is open
    if ( NULL == m_db_vscp_daemon ) {
        fprintf( stderr, 
                    "Failed to create VSCP guid table - closed.\n" );
        return false;
    }
    
    if ( SQLITE_OK  !=  sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg ) ) {
        fprintf( stderr, 
                    "Failed to create VSCP guid table with error %s.\n",
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
    
    if ( SQLITE_OK  !=  sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg ) ) {
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
    const char *psql = VSCPDB_MDF_CACHE_CREATE;
    
    fprintf( stderr, "Creating MDF cache table..\n" );
    
    // Check if database is open
    if ( NULL == m_db_vscp_daemon ) {
        fprintf( stderr, 
                    "Failed to create VSCP mdf cache table - closed.\n" );
        return false;
    }
    
    if ( SQLITE_OK  !=  sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg ) ) {
        fprintf( stderr, 
                    "Failed to create VSCP mdf cache table with error %s.\n",
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
    
    if ( SQLITE_OK  !=  sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg ) ) {
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
    
    if ( SQLITE_OK  !=  sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg ) ) {
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
    
    if ( SQLITE_OK  !=  sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg ) ) {
        fprintf( stderr, 
                    "Failed to create VSCP zone table with error %s.\n",
                    pErrMsg );
        return false;
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
    
    if ( SQLITE_OK  !=  sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg ) ) {
        fprintf( stderr, 
                    "Failed to create VSCP subzone table with error %s.\n",
                    pErrMsg );
        return false;
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
    const char *psql = VSCPDB_USERDEF_TABLE_CREATE;
    
    fprintf( stderr, "Creating userdef table..\n" );
    
    // Check if database is open
    if ( NULL == m_db_vscp_daemon ) {
        fprintf( stderr, 
                    "Failed to create VSCP userdef table - closed.\n" );
        return false;
    }
    
    if ( SQLITE_OK  !=  sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg ) ) {
        fprintf( stderr, 
                    "Failed to create VSCP userdef table with error %s.\n",
                    pErrMsg );
        return false;
    }
    
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

