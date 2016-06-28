// ControlObject.cpp: implementation of the CControlObject class.
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

#ifdef __GNUG__
//#pragma implementation
#endif


#ifdef WIN32
#include <winsock2.h>
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

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
#include "variablecodes.h"
#include "actioncodes.h"
#include "devicelist.h"
#include "devicethread.h"
#include "dm.h"
#include "vscpeventhelper.h"
#include "vscpwebserver.h"
#include "controlobject.h"


#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif


// Lists
WX_DEFINE_LIST(TRIGGERLIST)     // websocket triggers
WX_DEFINE_LIST(CanalMsgList);
WX_DEFINE_LIST(VSCPEventList);

//#define DEBUGPRINT

static CControlObject *gpctrlObj;

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
    gpctrlObj = this;           // needed by websocket static callbacks
    
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

    // Web server secutity should be used.
    m_bDisableSecurityWebServer = false;

    // Local domain
    strcpy( m_authDomain, "mydomain.com" );

    // Set Default Log Level
    m_logLevel = 0;

    // General logfile is enabled by default
    m_bLogGeneralEnable = true;

#ifdef WIN32
    m_logGeneralFileName.SetName( wxStandardPaths::Get().GetConfigDir() +
                                        _("/vscp/logs/vscp_log_general.txt") );
#else
    m_logGeneralFileName.SetName( _("/srv/vscp/logs/vscp_log_general") );
#endif

    // Security logfile is enabled by default
    m_bLogSecurityEnable = true;

#ifdef WIN32
    m_logSecurityFileName.SetName( wxStandardPaths::Get().GetConfigDir() +
                                            _("/vscp/vscp_log_security.txt") );
#else
    m_logSecurityFileName.SetName( _("/srv/vscp/logs/vscp_log_security") );
#endif

    // Access logfile is enabled by default
    m_bLogAccessEnable = true;

#ifdef WIN32
    m_logAccessFileName.SetName( wxStandardPaths::Get().GetConfigDir() +
                                            _("/vscp/vscp_log_access.txt") );
#else
    m_logAccessFileName.SetName( _("/srv/vscp/logs/vscp_log_access") );
#endif


#ifdef WIN32
    m_path_db_vscp_daemon.SetName( wxStandardPaths::Get().GetConfigDir() +
                                            _("/vscp/vscpd.sqlite3") );
#else
    m_path_db_vscp_daemon.SetName( _("/srv/vscp/vscpd.sqlite3") );
#endif

#ifdef WIN32
    m_path_db_vscp_data.SetName( wxStandardPaths::Get().GetConfigDir() +
                                            _("/vscp/vscp_data.sqlite3") );
#else
    m_path_db_vscp_data.SetName( _("/srv/vsc/vscp_data.sqlite3") );
#endif



#ifdef WIN32
    m_dm.m_path_db_vscp_dm.SetName( wxStandardPaths::Get().GetConfigDir() +
                                            _("/vscp/vscp_dm.sqlite3") );
#else
    m_dm.m_path_db_vscp_dm.SetName( _("/srv/vscp/vscp_dm.sqlite3") );
#endif
    
    
#ifdef WIN32
    m_path_db_vscp_log.SetName( wxStandardPaths::Get().GetConfigDir() +
                                            _("/vscp/vscp_log.sqlite3") );
#else
    m_path_db_vscp_log.SetName( _("/srv/vscp/vscp_log.sqlite3") );
#endif    

    // No databases opened yet
    m_db_vscp_daemon = NULL;
    m_db_vscp_data = NULL;
    m_dm.m_db_vscp_dm = NULL;

    // Control TCP/IP Interface
    m_bTCP = true;

    // Multicast announce
    m_bMulticastAnnounce = true;

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

    // Level I (Canal) drivers
    m_bEnableLevel1Drivers = true;

    // Level II Drivers
    m_bEnableLevel2Drivers = true;

    // Control VSCP
    m_bVSCPDaemon = true;

    // Control DM
    m_bDM = true;

    // Use variables
    m_bVariables = true;

    m_pclientMsgWorkerThread = NULL;
    m_pVSCPClientThread = NULL;
    m_pdaemonVSCPThread = NULL;
    m_pwebServerThread = NULL;
    m_pMQTTBrookerThread = NULL;
    m_pCoAPServerThread = NULL;

    // Websocket interface
    m_bWebSockets = true;       // websocket interface ia active
    m_bAuthWebsockets = true;   // Authentication is needed
    memset( m_pathCert, 0, sizeof( m_pathCert ) );

    // Webserver interface
    m_portWebServer = _("8080");
    m_bWebServer = true;

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
/*
        case DAEMON_LOGMSG_NOTICE:
            syslog(LOG_NOTICE, "%s", (const char *) wxdebugmsg.ToAscii());
            break;

        case DAEMON_LOGMSG_WARNING:
            syslog(LOG_WARNING, "%s", (const char *) wxdebugmsg.ToAscii());
            break;

        case DAEMON_LOGMSG_ERROR:
            syslog(LOG_ERR, "%s", (const char *) wxdebugmsg.ToAscii());
            break;

        case DAEMON_LOGMSG_CRITICAL:
            syslog(LOG_CRIT, "%s", (const char *) wxdebugmsg.ToAscii());
            break;

        case DAEMON_LOGMSG_ALERT:
            syslog(LOG_ALERT, "%s", (const char *) wxdebugmsg.ToAscii());
            break;

        case DAEMON_LOGMSG_EMERGENCY:
            syslog(LOG_EMERG, "%s", (const char *) wxdebugmsg.ToAscii());
            break;
*/
        };
        
    }

#endif

     m_mutexLogWrite.Unlock();

}


/////////////////////////////////////////////////////////////////////////////
// init

bool CControlObject::init(wxString& strcfgfile)
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

    // A configuration file must be available
    if ( !wxFile::Exists( strcfgfile ) ) {
        printf("No configuration file. Can't initialise!.");
        fprintf( stderr, "No configuration file. Can't initialise!.\n" );
        str = _("Path = .") + strcfgfile + _("\n");
        fprintf( stderr, str.mbc_str() );
        return FALSE;
    }

    // Initialise the SQLite library
    if ( SQLITE_OK != sqlite3_initialize() ) {
        fprintf( stderr, "Unable to initialise SQLite library!." );
        return FALSE;
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
                            _("admin"),
                            NULL,
                            _(""),
                            _("") );

    // Read XML configuration
    if (!readConfiguration(strcfgfile)) {
        fprintf( stderr, "Unable to open/parse configuration file. Can't initialise!.\n" );
        str = _("Path = .") + strcfgfile + _("\n");
        fprintf( stderr, str.mbc_str() );
        return FALSE;
    }

    // Open up the General logging file.
    if ( m_bLogGeneralEnable ) {
        m_fileLogGeneral.Open( m_logGeneralFileName.GetFullPath(), wxFile::write_append );
    }

    // Open up the Security logging file
    if ( m_bLogSecurityEnable ) {
        m_fileLogSecurity.Open( m_logSecurityFileName.GetFullPath(), wxFile::write_append );
    }

    // Open up the Access logging file
    if ( m_bLogAccessEnable ) {
        m_fileLogAccess.Open( m_logAccessFileName.GetFullPath(), wxFile::write_append );
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
        m_strServerName = _( "VSCP Daemon @ " );
        //m_strServerName += _( VSCPD_DISPLAY_VERSION );
        wxString strguid;
        m_guid.toString( strguid );
        m_strServerName += strguid;
    }

    

    // ===============================
    // * * * Open database files * * *
    // ===============================
    
        

    // * * * VSCP Daemon configuration database * * *

    // Check filename
    if ( m_path_db_vscp_daemon.IsOk() && m_path_db_vscp_daemon.FileExists() ) {

        if ( SQLITE_OK != sqlite3_open( m_path_db_vscp_daemon.GetFullPath().mbc_str(),
                                            &m_db_vscp_daemon ) ) {

            // Failed to open/create the database file
            fprintf( stderr, "VSCP Daemon configuration database could not be opened. - Will not be used.\n" );
            str.Printf( _("Path=%s error=%s\n"),
                            m_path_db_vscp_daemon.GetFullPath().mbc_str(),
                            sqlite3_errmsg( m_db_vscp_daemon ) );
            fprintf( stderr, str.mbc_str() );
            if ( NULL != m_db_vscp_daemon ) sqlite3_close( m_db_vscp_daemon );
            m_db_vscp_daemon = NULL;

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
            str.Printf(_("Path=%s\n"), m_path_db_vscp_daemon.GetFullPath().mbc_str() );
            fprintf( stderr, str.mbc_str() );
        }
        else {
            fprintf( stderr, "VSCP Daemon configuration database path invalid - will not be used.\n" );
            str.Printf(_("Path=%s\n"), m_path_db_vscp_daemon.GetFullPath().mbc_str() );
            fprintf( stderr, str.mbc_str() );
        }

    }
    
    
    // * * * VSCP Daemon logging database * * *
    
    
    // Check filename
    if ( m_path_db_vscp_log.IsOk() && m_path_db_vscp_log.FileExists() ) {

        if ( SQLITE_OK != sqlite3_open( m_path_db_vscp_log.GetFullPath().mbc_str(),
                                            &m_db_vscp_daemon ) ) {

            // Failed to open/create the database file
            fprintf( stderr, "VSCP Daemon logging database could not be opened. - Will not be used.\n" );
            str.Printf( _("Path=%s error=%s\n"),
                            m_path_db_vscp_log.GetFullPath().mbc_str(),
                            sqlite3_errmsg( m_db_vscp_daemon ) );
            fprintf( stderr, str.mbc_str() );
            if ( NULL != m_db_vscp_daemon ) sqlite3_close( m_db_vscp_daemon );
            m_db_vscp_daemon = NULL;

        }

    }
    else {

        if ( m_path_db_vscp_log.IsOk() ) {
            // We need to create the database from scratch. This may not work if
            // the database is in a read only location.
            fprintf( stderr, "VSCP Daemon configuration database does not exist - will be created.\n" );
            str.Printf(_("Path=%s\n"), m_path_db_vscp_log.GetFullPath().mbc_str() );
            fprintf( stderr, str.mbc_str() );
        }
        else {
            fprintf( stderr, "VSCP Daemon configuration database path invalid - will not be used.\n" );
            str.Printf(_("Path=%s\n"), m_path_db_vscp_log.GetFullPath().mbc_str() );
            fprintf( stderr, str.mbc_str() );
        }

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


    // Load decision matrix if mechanism is enabled
    if ( m_bDM ) {
        logMsg(_("DM enabled.\n") );
        m_dm.init();
        m_dm.load();
    }
    else {
        logMsg(_("DM disabled.\n") );
    }

    // Load variables if mechanism is enabled
    if ( m_bVariables ) {
        logMsg(_("Variables enabled.\n") );
        m_VSCP_Variables.load();
    }
    else {
        logMsg(_("Variables disabled.\n") );
    }

    // Add stock variables if enables
    if ( m_bVariables ) {
        addStockVariables();
    }

    // Start daemon internal client worker thread
    startClientWorkerThread();

    // Start TCP/IP interface if enabled
    if ( m_bTCP ) {
        logMsg(_("TCP/IP interface enabled.\n") );
        startTcpWorkerThread();
    }
    else {
        logMsg(_("TCP/IP interface disabled.\n") );
    }

    // Start daemon worker thread if enabled.
    if ( m_bVSCPDaemon ) {
        startDaemonWorkerThread();
    }

    // Start web sockets if enabled
    if (m_bWebSockets) {
        logMsg(_("WebServer interface active.\n") );
        startWebServerThread();
    }
    else {
        logMsg(_("WebServer interface disabled.\n") );
    }

    // Load drivers if the are enabled
    if (m_bEnableLevel1Drivers) {
        logMsg(_("Level I drivers enabled.\n") );
        startDeviceWorkerThreads();
    }
    else {
        logMsg(_("Level I drivers disabled.\n") );
    }

    // Start MQTT Broker if enabled
    if ( m_bMQTTBroker ) {
        logMsg(_("MQTT Broker enabled.\n") );
        startMQTTBrokerThread();
    }
    else {
        logMsg(_("MQTTBroker disabled.\n") );
    }

    // Start CoAP server if enabled
    if ( m_bCoAPServer ) {
        logMsg(_("CoAP Server enabled.\n") );
        startCoAPServerThread();
    }
    else {
        logMsg(_("CoAP Server disabled.\n") );
    }

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

        // Feed possible perodic event
        m_dm.feedPeriodicEvent();

        // Put the LOOP event on the queue
        // Garanties at least one lop event between every other
        // event feed to the queue
        m_dm.feed( &EventLoop );


        // Autosave variables
        m_variableMutex.Lock();
        if ( m_VSCP_Variables.autoSave() ) {
            logMsg(_("Autosaved variables.\n") );
        }
        m_variableMutex.Unlock();

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
    if ( m_bDM ) {
        sqlite3_close( m_dm.m_db_vscp_dm );
    }
    
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
    if ( m_bTCP ) {

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
    if ( m_bWebServer) {

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
    if (m_bVSCPDaemon) {

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

    } // daemon enabled

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
                                        s.ifr_addr.sa_data[ 5 ] )  );

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
// addStockVariables
//

void CControlObject::addStockVariables( void )
{
    wxString wxstr;
    cguid guid;

    m_VSCP_Variables.add( _("vscp.version.major"),
                wxString::Format( _("%d"), VSCPD_MAJOR_VERSION ),
                VSCP_DAEMON_VARIABLE_CODE_INTEGER,
                VSCP_VAR_NON_PERISTENT,
                VSCP_VAR_READ_ONLY );

    m_VSCP_Variables.add( _("vscp.version.minor"),
                wxString::Format( _("%d"), VSCPD_MINOR_VERSION ),
                VSCP_DAEMON_VARIABLE_CODE_INTEGER,
                VSCP_VAR_NON_PERISTENT,
                VSCP_VAR_READ_ONLY );

    m_VSCP_Variables.add( _("vscp.version.sub"),
                wxString::Format( _("%d"), VSCPD_SUB_VERSION ),
                VSCP_DAEMON_VARIABLE_CODE_INTEGER,
                VSCP_VAR_NON_PERISTENT,
                VSCP_VAR_READ_ONLY );

    m_VSCP_Variables.add( _("vscp.version.build"),
                wxString::Format( _("%d"), VSCPD_BUILD_VERSION ),
                VSCP_DAEMON_VARIABLE_CODE_INTEGER,
                VSCP_VAR_NON_PERISTENT,
                VSCP_VAR_READ_ONLY );

    m_VSCP_Variables.add( _("vscp.version.str"),
                wxString::Format( _("%s"), VSCPD_DISPLAY_VERSION ),
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_NON_PERISTENT,
                VSCP_VAR_READ_ONLY );

    m_VSCP_Variables.add( _("vscp.version.wxwidgets.str"),
                wxString::Format( _("%s"), wxVERSION_STRING ),
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_NON_PERISTENT,
                VSCP_VAR_READ_ONLY );

    m_VSCP_Variables.add( _("vscp.version.wxwidgets.major"),
                wxString::Format( _("%d"), wxMAJOR_VERSION ),
                VSCP_DAEMON_VARIABLE_CODE_INTEGER,
                VSCP_VAR_NON_PERISTENT,
                VSCP_VAR_READ_ONLY );

    m_VSCP_Variables.add( _("vscp.version.wxwidgets.minor"),
                wxString::Format( _("%d"), wxMINOR_VERSION ),
                VSCP_DAEMON_VARIABLE_CODE_INTEGER,
                VSCP_VAR_NON_PERISTENT,
                VSCP_VAR_READ_ONLY );

    m_VSCP_Variables.add( _("vscp.version.wxwidgets.release"),
                wxString::Format( _("%d"), wxRELEASE_NUMBER ),
                VSCP_DAEMON_VARIABLE_CODE_INTEGER,
                VSCP_VAR_NON_PERISTENT,
                VSCP_VAR_READ_ONLY );

    m_VSCP_Variables.add( _("vscp.version.wxwidgets.sub"),
                wxString::Format( _("%d"), wxSUBRELEASE_NUMBER ),
                VSCP_DAEMON_VARIABLE_CODE_INTEGER,
                VSCP_VAR_NON_PERISTENT,
                VSCP_VAR_READ_ONLY );

    m_VSCP_Variables.add( _("vscp.copyright.vscp"),
                wxString::Format( _("%s"), VSCPD_COPYRIGHT ),
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_NON_PERISTENT,
                VSCP_VAR_READ_ONLY );

// *****************************************************************************
//                               wxWidgets
// *****************************************************************************

    m_VSCP_Variables.add( _("vscp.copyright.wxwidgets"),
                wxString::Format( _("%s"),
                    "Copyright (c) 1998-2005 Julian Smart, Robert Roebling et al" ),
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_NON_PERISTENT,
                VSCP_VAR_READ_ONLY );

// *****************************************************************************
//                               Mongoose
// *****************************************************************************

    m_VSCP_Variables.add( _("vscp.copyright.mongoose"),
                wxString::Format( _("%s"),
                    "Copyright (c) 2013-2015 Cesanta Software Limited" ),
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_NON_PERISTENT,
                VSCP_VAR_READ_ONLY );

// *****************************************************************************
//                                 LUA
// *****************************************************************************

#ifndef VSCP_DISABLE_LUA

    m_VSCP_Variables.add( _("vscp.copyright.lua"),
                wxString::Format( _("%s"), LUA_COPYRIGHT ),
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_NON_PERISTENT,
                VSCP_VAR_READ_ONLY );


    m_VSCP_Variables.add( _("vscp.version.lua.major"),
                wxString::Format( _("%s"), LUA_VERSION_MAJOR ),
                VSCP_DAEMON_VARIABLE_CODE_INTEGER,
                VSCP_VAR_NON_PERISTENT,
                VSCP_VAR_READ_ONLY );

    m_VSCP_Variables.add( _("vscp.version.lua.minor"),
                wxString::Format( _("%s"), LUA_VERSION_MINOR),
                VSCP_DAEMON_VARIABLE_CODE_INTEGER,
                VSCP_VAR_NON_PERISTENT,
                VSCP_VAR_READ_ONLY );

    m_VSCP_Variables.add( _("vscp.version.lua.release"),
                wxString::Format( _("%s"), LUA_VERSION_RELEASE ),
                VSCP_DAEMON_VARIABLE_CODE_INTEGER,
                VSCP_VAR_NON_PERISTENT,
                VSCP_VAR_READ_ONLY );

    m_VSCP_Variables.add( _("vscp.version.lua.str"),
                wxString::Format( _("%s.%s.%s"),
                                    LUA_VERSION_MAJOR,
                                    LUA_VERSION_MINOR,
                                    LUA_VERSION_RELEASE ),
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_NON_PERISTENT,
                VSCP_VAR_READ_ONLY );

#endif


// *****************************************************************************
//                                SQLite
// *****************************************************************************

#ifndef VSCP_DISABLE_SQLITE

    int major, minor, sub, build;
    sscanf( SQLITE_VERSION,
            "%d.%d.%d.%d",
            &major, &minor, &sub, &build );

    m_VSCP_Variables.add( _("vscp.version.sqlite.major"),
                wxString::Format( _("%d"), major ),
                VSCP_DAEMON_VARIABLE_CODE_INTEGER,
                VSCP_VAR_NON_PERISTENT,
                VSCP_VAR_READ_ONLY );

    m_VSCP_Variables.add( _("vscp.version.sqlite.minor"),
                wxString::Format( _("%d"), minor ),
                VSCP_DAEMON_VARIABLE_CODE_INTEGER,
                VSCP_VAR_NON_PERISTENT,
                VSCP_VAR_READ_ONLY );

    m_VSCP_Variables.add( _("vscp.version.sqlite.release"),
                wxString::Format( _("%d"), sub ),
                VSCP_DAEMON_VARIABLE_CODE_INTEGER,
                VSCP_VAR_NON_PERISTENT,
                VSCP_VAR_READ_ONLY );

    m_VSCP_Variables.add( _("vscp.version.sqlite.build"),
                wxString::Format( _("%d"), build ),
                VSCP_DAEMON_VARIABLE_CODE_INTEGER,
                VSCP_VAR_NON_PERISTENT,
                VSCP_VAR_READ_ONLY );

    m_VSCP_Variables.add( _("vscp.version.sqllite.str"),
                _(SQLITE_VERSION),
                VSCP_DAEMON_VARIABLE_CODE_INTEGER,
                VSCP_VAR_NON_PERISTENT,
                VSCP_VAR_READ_ONLY );

#endif

// *****************************************************************************
//                                OPENSSL
// *****************************************************************************
#ifndef WIN32
    m_VSCP_Variables.add( _("vscp.version.openssl.str"),
                _(OPENSSL_VERSION_TEXT),
                VSCP_DAEMON_VARIABLE_CODE_INTEGER,
                VSCP_VAR_NON_PERISTENT,
                VSCP_VAR_READ_ONLY );
#endif

    m_VSCP_Variables.add( _("vscp.os.str"),
                wxGetOsDescription(),
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_NON_PERISTENT,
                VSCP_VAR_READ_ONLY );

    if ( wxIsPlatform64Bit() ) {
        wxstr = _("64-bit ");
    }
    else {
        wxstr = _("32-bit ");
    }

    m_VSCP_Variables.add( _("vscp.os.wordwidth"),
                wxIsPlatform64Bit() ? _("64") : _("32"),
                VSCP_DAEMON_VARIABLE_CODE_INTEGER,
                VSCP_VAR_NON_PERISTENT,
                VSCP_VAR_READ_ONLY );

    m_VSCP_Variables.add( _("vscp.os.wordwidth.str"),
                wxstr,
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_NON_PERISTENT,
                VSCP_VAR_READ_ONLY );

    m_VSCP_Variables.add( _("vscp.os.width.is64bit"),
                wxIsPlatform64Bit() ? _("true") : _("false"),
                VSCP_DAEMON_VARIABLE_CODE_BOOLEAN,
                VSCP_VAR_NON_PERISTENT,
                VSCP_VAR_READ_ONLY );

    m_VSCP_Variables.add( _("vscp.os.width.is32bit"),
                !wxIsPlatform64Bit() ? _("true") : _("false"),
                VSCP_DAEMON_VARIABLE_CODE_BOOLEAN,
                VSCP_VAR_NON_PERISTENT,
                VSCP_VAR_READ_ONLY );

    if ( wxIsPlatformLittleEndian() ) {
        wxstr = _("Little endian");
    }
    else {
        wxstr = _("Big endian");
    }

    m_VSCP_Variables.add( _("vscp.os.endiness.str"),
                wxstr,
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_NON_PERISTENT,
                VSCP_VAR_READ_ONLY );

    m_VSCP_Variables.add( _("vscp.os.endiness.isLittleEndian"),
                wxIsPlatformLittleEndian() ? _("true") : _("false"),
                VSCP_DAEMON_VARIABLE_CODE_BOOLEAN,
                VSCP_VAR_NON_PERISTENT,
                VSCP_VAR_READ_ONLY );

   m_VSCP_Variables.add( _("vscp.host.fullname"),
                wxGetFullHostName(),
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_READ_ONLY,
                false );

    if ( getIPAddress( guid ) ) {
        m_VSCP_Variables.add( _("vscp.host.ip"),
                wxString::Format( _("%d.%d.%d.%d"),
                            guid.getAt( 11 ),
                            guid.getAt( 10 ),
                            guid.getAt( 9 ),
                            guid.getAt( 8 ) ),
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_READ_ONLY,
                false );
    }

    if ( getMacAddress( guid ) ) {
        m_VSCP_Variables.add( _("vscp.host.mac"),
                wxString::Format( _("%02X:%02X:%02X:%02X:%02X:%02X"),
                            guid.getAt( 13 ),
                            guid.getAt( 12 ),
                            guid.getAt( 11 ),
                            guid.getAt( 10 ),
                            guid.getAt( 9 ),
                            guid.getAt( 8 ) ),
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_READ_ONLY,
                false );
    }

    m_VSCP_Variables.add( _("vscp.host.userid"),
                wxGetUserId(),
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_READ_ONLY,
                false );


    m_VSCP_Variables.add( _("vscp.host.username"),
                wxGetUserName(),
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_READ_ONLY,
                false );

    m_guid.toString( wxstr );
    m_VSCP_Variables.add( _("vscp.host.guid"),
                wxstr,
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_READ_ONLY,
                false );

    m_guid.toString( wxstr );
    m_VSCP_Variables.add( _("vscp.loglevel"),
                wxString::Format( _("%d "), m_logLevel ),
                VSCP_DAEMON_VARIABLE_CODE_INTEGER,
                VSCP_VAR_READ_ONLY,
                false );

    switch ( m_logLevel  ) {
        case DAEMON_LOGMSG_NONE:
            wxstr = _("None");
            break;
        case DAEMON_LOGMSG_NORMAL:
            wxstr = _("Normal");
            break;
        case DAEMON_LOGMSG_DEBUG:
            wxstr = _("Debug");
            break;
/*
        case DAEMON_LOGMSG_NOTICE:
            wxstr = _("Notice");
            break;
        case DAEMON_LOGMSG_WARNING:
            wxstr = _("Warning");
            break;
        case DAEMON_LOGMSG_ERROR:
            wxstr = _("Error");
            break;
        case DAEMON_LOGMSG_CRITICAL:
            wxstr = _("Critical");
            break;
        case DAEMON_LOGMSG_ALERT:
            wxstr = _("Alert");
            break;
        case DAEMON_LOGMSG_EMERGENCY:
            wxstr = _("Emergency");
            break;
*/
        default:
            wxstr = _("Unknown");
            break;
    }

    m_VSCP_Variables.add( _("vscp.loglevel.str"),
                wxstr,
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_READ_ONLY,
                false );


    m_VSCP_Variables.add( _("vscp.client.ReceiveQueue.Max"),
                wxString::Format( _("%d"), m_maxItemsInClientReceiveQueue ),
                VSCP_DAEMON_VARIABLE_CODE_LONG,
                VSCP_VAR_READ_ONLY,
                false );

    m_VSCP_Variables.add( _("vscp.tcpip.isEnabled"),
                m_bTCP ? _("true") : _("false"),
                VSCP_DAEMON_VARIABLE_CODE_BOOLEAN,
                VSCP_VAR_READ_ONLY,
                false );

    m_VSCP_Variables.add( _("vscp.tcpip.addess"),
                m_strTcpInterfaceAddress,
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_READ_ONLY,
                false );

    m_VSCP_Variables.add( _("vscp.udp.isEnabled"),
                m_bUDP ? _("true") : _("false"),
                VSCP_DAEMON_VARIABLE_CODE_BOOLEAN,
                VSCP_VAR_READ_ONLY,
                false );

    m_VSCP_Variables.add( _("vscp.udp.addess"),
                m_strUDPInterfaceAddress,
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_READ_ONLY,
                false );

    m_VSCP_Variables.add( _("vscp.mqtt.broker.isEnabled"),
                m_bMQTTBroker ? _("true") : _("false"),
                VSCP_DAEMON_VARIABLE_CODE_BOOLEAN,
                VSCP_VAR_READ_ONLY,
                false );

    m_VSCP_Variables.add( _("vscp.mqtt.broker.addess"),
                m_strMQTTBrokerInterfaceAddress,
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_READ_ONLY,
                false );

    m_VSCP_Variables.add( _("vscp.coap.server.isEnabled"),
                m_bCoAPServer ? _("true") : _("false"),
                VSCP_DAEMON_VARIABLE_CODE_BOOLEAN,
                VSCP_VAR_READ_ONLY,
                false );

    m_VSCP_Variables.add( _("vscp.coap.server.addess"),
                m_strCoAPServerInterfaceAddress,
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_READ_ONLY,
                false );

    m_VSCP_Variables.add( _("vscp.discovery.isEnabled"),
                m_bVSCPDaemon ? _("true") : _("false"),
                VSCP_DAEMON_VARIABLE_CODE_BOOLEAN,
                VSCP_VAR_READ_ONLY,
                false );

    m_VSCP_Variables.add( _("vscp.automation.isEnabled"),
                m_automation.isAutomationEnabled() ? _("true") : _("false"),
                VSCP_DAEMON_VARIABLE_CODE_BOOLEAN,
                VSCP_VAR_READ_ONLY,
                false );

    m_VSCP_Variables.add( _("vscp.websrv.isEnabled"),
                m_bWebServer ? _("true") : _("false"),
                VSCP_DAEMON_VARIABLE_CODE_BOOLEAN,
                VSCP_VAR_READ_ONLY,
                false );

    if ( m_automation.isAutomationEnabled() ) {

        m_VSCP_Variables.add( _("vscp.automation.heartbeat.isEnabled"),
                m_automation.isSendHeartbeat() ? _("true") : _("false"),
                VSCP_DAEMON_VARIABLE_CODE_BOOLEAN,
                VSCP_VAR_READ_ONLY,
                false );

        m_VSCP_Variables.add( _("vscp.automation.heartbeat.period"),
                wxString::Format( _("%ld"), m_automation.getIntervalHeartbeat() ),
                VSCP_DAEMON_VARIABLE_CODE_LONG,
                VSCP_VAR_READ_ONLY,
                false );

        wxstr = m_automation.getHeartbeatSent().FormatISODate();
        wxstr += _( "T" );
        wxstr += m_automation.getHeartbeatSent().FormatISOTime();
        m_VSCP_Variables.add( _("vscp.automation.heartbeat.last"),
                wxstr,
                VSCP_DAEMON_VARIABLE_CODE_DATETIME,
                VSCP_VAR_READ_ONLY,
                false );

        m_VSCP_Variables.add( _("vscp.automation.segctrl-heartbeat.isEnabled"),
                m_automation.isSendSegmentControllerHeartbeat() ? _("true") : _("false"),
                VSCP_DAEMON_VARIABLE_CODE_BOOLEAN,
                VSCP_VAR_READ_ONLY,
                false );

        m_VSCP_Variables.add( _("vscp.automation.segctrl.heartbeat.period"),
                wxString::Format( _("%ld"), m_automation.getIntervalSegmentControllerHeartbeat() ),
                VSCP_DAEMON_VARIABLE_CODE_LONG,
                VSCP_VAR_READ_ONLY,
                false );

        wxstr = m_automation.getSegmentControllerHeartbeatSent().FormatISODate();
        wxstr += _( "T" );
        wxstr += m_automation.getSegmentControllerHeartbeatSent().FormatISOTime();
        m_VSCP_Variables.add( _("vscp.automation.segctrl.heartbeat.last"),
                wxstr,
                VSCP_DAEMON_VARIABLE_CODE_DATETIME,
                VSCP_VAR_READ_ONLY,
                false );

        m_VSCP_Variables.add( _("vscp.automation.longitude"),
                wxString::Format( _("%f"), m_automation.getLongitude() ),
                VSCP_DAEMON_VARIABLE_CODE_DOUBLE,
                VSCP_VAR_READ_ONLY,
                false );

        m_VSCP_Variables.add( _("vscp.automation.latitude"),
                wxString::Format( _("%f"), m_automation.getLatitude() ),
                VSCP_DAEMON_VARIABLE_CODE_DOUBLE,
                VSCP_VAR_READ_ONLY,
                false );

        m_VSCP_Variables.add( _("vscp.automation.isSendTwilightSunriseEvent"),
                m_automation.isSendSunriseTwilightEvent() ? _("true") : _("false"),
                VSCP_DAEMON_VARIABLE_CODE_BOOLEAN,
                VSCP_VAR_READ_ONLY,
                false );

        m_VSCP_Variables.add( _("vscp.automation.isSendSunriseEvent"),
                m_automation.isSendSunriseEvent() ? _("true") : _("false"),
                VSCP_DAEMON_VARIABLE_CODE_BOOLEAN,
                VSCP_VAR_READ_ONLY,
                false );

        m_VSCP_Variables.add( _("vscp.automation.isSendSunsetEvent"),
                m_automation.isSendSunsetEvent() ? _("true") : _("false"),
                VSCP_DAEMON_VARIABLE_CODE_BOOLEAN,
                VSCP_VAR_READ_ONLY,
                false );

        m_VSCP_Variables.add( _("vscp.automation.isSendTwilightSunsetEvent"),
                m_automation.isSendSunsetTwilightEvent() ? _("true") : _("false"),
                VSCP_DAEMON_VARIABLE_CODE_BOOLEAN,
                VSCP_VAR_READ_ONLY,
                false );

        m_VSCP_Variables.add( _("vscp.WorkingFolder"),
                wxGetCwd(),
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_READ_ONLY,
                false );

    }

// *****************************************************************************
//                                WEB-Server
// *****************************************************************************

    if ( m_bWebServer ) {

        m_VSCP_Variables.add( _("vscp.websrv.address"),
                m_portWebServer,
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_READ_ONLY,
                false );

        m_VSCP_Variables.add( _("vscp.websrv.authenticationOn"),
                !m_bDisableSecurityWebServer ? _("true") : _("false"),
                VSCP_DAEMON_VARIABLE_CODE_BOOLEAN,
                VSCP_VAR_READ_ONLY,
                false );

        m_VSCP_Variables.add( _("vscp.websrv.root.path"),
                wxString::FromUTF8( m_pathWebRoot ),
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_READ_ONLY,
                false );

        m_VSCP_Variables.add( _("vscp.websrv.authdomain"),
                ( 0 == strlen( m_authDomain ) ) ?
                    wxString::FromAscii( "mydomain.com" ) :
                    wxString::FromUTF8( m_authDomain ),
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_READ_ONLY,
                false );

        m_VSCP_Variables.add( _("vscp.websrv.cert.path"),
                wxString::FromUTF8( m_pathCert ),
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_READ_ONLY,
                false );

        m_VSCP_Variables.add( _("vscp.websrv.extramimetypes"),
                wxString::FromUTF8( m_extraMimeTypes ),
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_READ_ONLY,
                false );

        m_VSCP_Variables.add( _("vscp.websrv.ssipatterns"),
                wxString::FromUTF8( m_ssi_pattern ),
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_READ_ONLY,
                false );

        m_VSCP_Variables.add( _("vscp.websrv.ipacl"),
                wxString::FromUTF8( m_ip_acl ),
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_READ_ONLY,
                false );

        m_VSCP_Variables.add( _("vscp.websrv.cgi.interpreter"),
                wxString::FromUTF8( m_cgiInterpreter ),
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_READ_ONLY,
                false );

        m_VSCP_Variables.add( _("vscp.websrv.cgi.pattern"),
                wxString::FromUTF8( m_cgiPattern ),
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_READ_ONLY,
                false );

        m_VSCP_Variables.add( _("vscp.websrv.directorylistings.enable"),
                ( NULL != strstr( m_EnableDirectoryListings,"yes" ) ) ? _("true") : _("false"),
                VSCP_DAEMON_VARIABLE_CODE_BOOLEAN,
                VSCP_VAR_READ_ONLY,
                false );

        m_VSCP_Variables.add( _("vscp.websrv.hidefile.pattern"),
                wxString::FromUTF8( m_hideFilePatterns ),
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_READ_ONLY,
                false );

        m_VSCP_Variables.add( _("vscp.websrv.indexfiles"),
                wxString::FromUTF8( m_indexFiles ),
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_READ_ONLY,
                false );

        m_VSCP_Variables.add( _("vscp.websrv.urlrewrites"),
                wxString::FromUTF8( m_urlRewrites ),
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_READ_ONLY,
                false );

        m_VSCP_Variables.add( _("vscp.websrv.auth.file.directory"),
                wxString::FromUTF8( m_per_directory_auth_file ),
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_READ_ONLY,
                false );

        m_VSCP_Variables.add( _("vscp.websrv.auth.file.global"),
                wxString::FromUTF8( m_per_directory_auth_file ),
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_READ_ONLY,
                false );

    }

// *****************************************************************************
//                            Websocket-Server
// *****************************************************************************

    m_VSCP_Variables.add( _("vscp.websocket.enable"),
                m_bWebSockets ? _("true") : _("false"),
                VSCP_DAEMON_VARIABLE_CODE_BOOLEAN,
                VSCP_VAR_READ_ONLY,
                false );

    if ( m_bWebSockets ) {

        m_VSCP_Variables.add( _("vscp.websocket.auth.enable"),
                m_bAuthWebsockets ? _("true") : _("false"),
                VSCP_DAEMON_VARIABLE_CODE_BOOLEAN,
                VSCP_VAR_READ_ONLY,
                false );

    }

// *****************************************************************************
//                            Decision Matrix
// *****************************************************************************

    m_VSCP_Variables.add( _("vscp.dm.enable"),
                m_bDM ? _("true") : _("false"),
                VSCP_DAEMON_VARIABLE_CODE_BOOLEAN,
                VSCP_VAR_READ_ONLY,
                false );

    if ( m_bDM ) {

        m_VSCP_Variables.add( _("vscp.dm.logging.enable"),
                m_dm.m_bLogEnable ? _("true") : _("false"),
                VSCP_DAEMON_VARIABLE_CODE_BOOLEAN,
                VSCP_VAR_READ_ONLY,
                false );

        if ( m_dm.m_bLogEnable ) {

            m_VSCP_Variables.add( _("vscp.dm.logging.path"),
                m_dm.m_logPath.GetFullPath(),
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_READ_ONLY,
                false );

        }


        m_VSCP_Variables.add( _("vscp.dm.static.path"),
                m_dm.m_configPath,
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_READ_ONLY,
                false );

    }


// *****************************************************************************
//                             Variables
// *****************************************************************************

    m_VSCP_Variables.add( _("vscp.variable.enable"),
                _("true"),
                VSCP_DAEMON_VARIABLE_CODE_BOOLEAN,
                VSCP_VAR_READ_ONLY,
                false );

    m_VSCP_Variables.add( _("vscp.variable.path"),
                m_VSCP_Variables.m_configPath,
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_READ_ONLY,
                false );

    m_VSCP_Variables.add( _("vscp.variable.interval.save"),
                wxString::Format( _("%ud"), m_VSCP_Variables.m_autosaveInterval ),
                VSCP_DAEMON_VARIABLE_CODE_INTEGER,
                VSCP_VAR_READ_ONLY,
                false );

// *****************************************************************************
//                              Log files
// *****************************************************************************

    // General
    m_VSCP_Variables.add( _("vscp.log.general.enable"),
                m_bLogGeneralEnable ? _("true") : _("false"),
                VSCP_DAEMON_VARIABLE_CODE_BOOLEAN,
                VSCP_VAR_READ_ONLY,
                false );

    if ( m_bLogGeneralEnable ) {

        m_VSCP_Variables.add( _("vscp.log.general.path"),
                m_logGeneralFileName.GetFullPath(),
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_READ_ONLY,
                false );

    }

    // Access
    m_VSCP_Variables.add( _("vscp.log.access.enable"),
                m_bLogAccessEnable ? _("true") : _("false"),
                VSCP_DAEMON_VARIABLE_CODE_BOOLEAN,
                VSCP_VAR_READ_ONLY,
                false );

    if ( m_bLogAccessEnable ) {

        m_VSCP_Variables.add( _("vscp.log.access.path"),
                m_logAccessFileName.GetFullPath(),
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_READ_ONLY,
                false );

    }

    // Security
    m_VSCP_Variables.add( _("vscp.log.access.enable"),
                m_bLogAccessEnable ? _("true") : _("false"),
                VSCP_DAEMON_VARIABLE_CODE_BOOLEAN,
                VSCP_VAR_READ_ONLY,
                false );

    if ( m_bLogAccessEnable ) {

        m_VSCP_Variables.add( _("vscp.log.access.path"),
                m_logAccessFileName.GetFullPath(),
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_READ_ONLY,
                false );

    }

    // Security
    m_VSCP_Variables.add( _("vscp.log.security.enable"),
                m_bLogSecurityEnable ? _("true") : _("false"),
                VSCP_DAEMON_VARIABLE_CODE_BOOLEAN,
                VSCP_VAR_READ_ONLY,
                false );

    if ( m_bLogSecurityEnable ) {

        m_VSCP_Variables.add( _("vscp.log.security.path"),
                m_logSecurityFileName.GetFullPath(),
                VSCP_DAEMON_VARIABLE_CODE_STRING,
                VSCP_VAR_READ_ONLY,
                false );

    }

// *****************************************************************************
//                              Drivers
// *****************************************************************************


   m_VSCP_Variables.add( _("vscp.driver.level1.enable"),
                m_bEnableLevel1Drivers ? _("true") : _("false"),
                VSCP_DAEMON_VARIABLE_CODE_BOOLEAN,
                VSCP_VAR_READ_ONLY,
                false );


    m_VSCP_Variables.add( _("vscp.driver.level2.enable"),
                m_bEnableLevel2Drivers ? _("true") : _("false"),
                VSCP_DAEMON_VARIABLE_CODE_BOOLEAN,
                VSCP_VAR_READ_ONLY,
                false );

}


///////////////////////////////////////////////////////////////////////////////
// readConfiguration
//
// Read the configuration XML file
//

bool CControlObject::readConfiguration( wxString& strcfgfile )
{
    unsigned long val;
    wxXmlDocument doc;

    wxString wxlogmsg = wxString::Format(_("Reading configuration from [%s]\n"),
                                            (const char *)strcfgfile.c_str() );
    logMsg( wxlogmsg  );

    if (!doc.Load(strcfgfile)) {
        logMsg(_("Can't load logfile. Is path correct?")  );
        return false;
    }

    // start processing the XML file
    if (doc.GetRoot()->GetName() != wxT("vscpconfig")) {
        logMsg(_("Can't read logfile. Maybe it is invalid!")  );
        return false;
    }

    wxXmlNode *child = doc.GetRoot()->GetChildren();
    while (child) {

        if (child->GetName() == wxT("general")) {

            wxXmlNode *subchild = child->GetChildren();
            while (subchild) {

                if (subchild->GetName() == wxT("loglevel")) {
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
                }
                else if (subchild->GetName() == wxT("runasuser")) {
                    m_runAsUser = subchild->GetNodeContent();
                    m_runAsUser.Trim();
                    m_runAsUser.Trim(false);
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
                    
                }
                else if (subchild->GetName() == wxT("generallogfile")) {

                    wxString attribute = subchild->GetAttribute(wxT("enable"), wxT("true"));
                    attribute.MakeLower();
                    if (attribute.IsSameAs(_("false"), false)) {
                        m_bLogGeneralEnable = false;
                    }
                    else {
                        m_bLogGeneralEnable = true;
                    }
                    
                    // Old form
                    wxFileName fileName;
                    fileName.SetName( subchild->GetNodeContent() );
                    if ( fileName.IsOk() ) {
                        m_logGeneralFileName = fileName;
                    }
                    
                    // New form
                    attribute = subchild->GetAttribute(wxT("path"), wxT(""));
                                      
                    fileName.SetName( subchild->GetNodeContent() );
                    if ( fileName.IsOk() ) {
                        m_logGeneralFileName = attribute;
                    }   
                    
                }
                else if (subchild->GetName() == wxT("securitylogfile")) {

                    wxString attribute = subchild->GetAttribute(wxT("enable"), wxT("true"));
                    attribute.MakeLower();
                    if (attribute.IsSameAs(_("false"), false)) {
                        m_bLogSecurityEnable = false;
                    }
                    else {
                        m_bLogSecurityEnable = true;
                    }

                    // Old form
                    wxFileName fileName;
                    fileName.SetName( subchild->GetNodeContent() );
                    if ( fileName.IsOk() ) {
                        m_logSecurityFileName = fileName;
                    }
                    
                    // New form
                    attribute = subchild->GetAttribute(wxT("path"), wxT(""));
                                      
                    fileName.SetName( subchild->GetNodeContent() );
                    if ( fileName.IsOk() ) {
                        m_logSecurityFileName = attribute;
                    } 
                    
                }
                else if (subchild->GetName() == wxT("accesslogfile")) {

                    wxString attribute = subchild->GetAttribute(wxT("enable"), wxT("true"));
                    attribute.MakeLower();
                    if (attribute.IsSameAs(_("false"), false)) {
                        m_bLogAccessEnable = false;
                    }
                    else {
                        m_bLogAccessEnable = true;
                    }

                    wxFileName fileName;
                    fileName.SetName( subchild->GetNodeContent() );
                    if ( fileName.IsOk() ) {
                        m_logAccessFileName = fileName;
                    }
                    
                    // New form
                    attribute = subchild->GetAttribute(wxT("path"), wxT(""));
                                      
                    fileName.SetName( subchild->GetNodeContent() );
                    if ( fileName.IsOk() ) {
                        m_logAccessFileName = attribute;
                    } 
                    
                }                
                else if (subchild->GetName() == wxT("db_vscp_data")) {
                    wxFileName fileName;
                    fileName.SetName( subchild->GetNodeContent() );
                    if ( fileName.IsOk() ) {
                        m_path_db_vscp_data = fileName;
                    }
                }
                else if (subchild->GetName() == wxT("db_vscp_daemon")) {
                    wxFileName fileName;
                    fileName.SetName( subchild->GetNodeContent() );
                    if ( fileName.IsOk() ) {
                        m_path_db_vscp_daemon = fileName;
                    }
                }
                else if (subchild->GetName() == wxT("tcpip")) {
                    wxString attribute = subchild->GetAttribute(wxT("enable"), wxT("true"));
                    attribute.MakeLower();
                    if (attribute.IsSameAs(_("false"), false)) {
                        m_bTCP = false;
                    }
                    else {
                        m_bTCP = true;
                    }

                    m_strTcpInterfaceAddress = subchild->GetAttribute(wxT("interface"), wxT(""));

                }
                else if ( subchild->GetName() == wxT( "multicast-announce" ) ) {

                    wxString attribute = subchild->GetAttribute( wxT( "enable" ), wxT( "true" ) );
                    attribute.MakeLower();
                    if ( attribute.IsSameAs( _( "false" ), false ) ) {
                        m_bMulticastAnnounce = false;
                    }
                    else {
                        m_bMulticastAnnounce = true;
                    }

                    m_strMulticastAnnounceAddress = subchild->GetAttribute( wxT( "interface" ), wxT( "" ) );

                    m_ttlMultiCastAnnounce = vscp_readStringValue( subchild->GetAttribute( wxT( "ttl" ), wxT( "1" ) ) );

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

                }
                else if (subchild->GetName() == wxT("dm")) {
                    // Should the internal DM be disabled

                    wxString attribut = subchild->GetAttribute(wxT("enable"), wxT("true"));
                    attribut.MakeLower();
                    if (attribut.IsSameAs(_("false"), false)) {
                        m_bDM = false;
                    }

                    // Get the path to the DM file  (Deprecated)
                    attribut = subchild->GetAttribute( wxT("path"), wxT("") );
                    if ( attribut.Length() ) {
                        m_dm.m_configPath = attribut;
                    }
                    
                    // Get the path to the DM file
                    attribut = subchild->GetAttribute(wxT("pathxml"), wxT(""));
                    if ( attribut.Length() ) {
                        m_dm.m_configPath = attribut;
                    }

                    // Get the path to the DM database
                    attribut = subchild->GetAttribute(wxT("pathdb"), wxT(""));
                    if ( attribut.Length() ) {
                        wxFileName fileName;
                        fileName.SetName( attribut );
                        if ( fileName.IsOk() ) {
                            m_dm.m_path_db_vscp_dm = fileName;
                        }
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
                    fileName.SetName( subchild->GetAttribute( wxT("logpath"), wxT("") ) );
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

                }
                else if (subchild->GetName() == wxT("variables")) {
                    // Should the internal DM be disabled
                    wxFileName fileName;
                    wxString attrib = subchild->GetAttribute(wxT("enable"), wxT("true"));

                    if (attrib.IsSameAs(_("false"), false)) {
                        m_bVariables = false;
                    }

                    // Get the path to the DM file
                    attrib = subchild->GetAttribute(wxT("path"), wxT(""));
                    fileName.SetName( attrib );
                    if ( fileName.IsOk() ) {
                        m_VSCP_Variables.m_configPath = fileName.GetFullPath();
                    }
                    
                    attrib = subchild->GetAttribute(wxT("pathdb"), wxT(""));
                    fileName.SetName( attrib );
                    if ( fileName.IsOk() ) {
                        m_VSCP_Variables.m_path_db_vscp_variable = fileName;
                    }

                    // Autosave interval
                    long autosave = vscp_readStringValue( subchild->GetAttribute(wxT("autosave"), wxT("5")) );
                    m_VSCP_Variables.setAutoSaveInterval( autosave );

                }
                else if (subchild->GetName() == wxT("vscp")) {

                    wxString attribute = subchild->GetAttribute(wxT("enable"), wxT("true"));

                    if (attribute.IsSameAs(_("false"), false)) {
                        m_bVSCPDaemon = false;
                    }

                }
                else if (subchild->GetName() == wxT("guid")) {
                    wxString str = subchild->GetNodeContent();
                    m_guid.getFromString(str);
                }
                else if ( subchild->GetName() == wxT( "servername" ) ) {
                    m_strServerName = subchild->GetNodeContent();
                }
                else if (subchild->GetName() == wxT("clientbuffersize")) {
                    wxString str = subchild->GetNodeContent();
                    m_maxItemsInClientReceiveQueue = vscp_readStringValue(str);
                }
                else if (subchild->GetName() == wxT("webserver")) {

                    wxString attribute = subchild->GetAttribute(wxT("enable"), wxT("true"));

                    if (attribute.IsSameAs(_("false"), false)) {
                        m_bWebServer = false;
                    }

                    attribute = subchild->GetAttribute( wxT( "disableauthentication" ), wxT( "false" ) );

                    if ( attribute.IsSameAs( _( "true" ), true ) ) {
                        m_bDisableSecurityWebServer = true;
                    }

                    attribute = subchild->GetAttribute(wxT("port"), wxT("8080"));
                    attribute.Trim();
                    attribute.Trim(false);
                    if ( attribute.Length() ) {
                        m_portWebServer = attribute;
                    }

                    attribute = subchild->GetAttribute(wxT("extra_mime_types"), wxT(""));
                    attribute.Trim();
                    attribute.Trim(false);
                    if ( attribute.Length() ) {
                        //m_pathWebRoot = attribute;
                        strcpy( m_extraMimeTypes, attribute.mbc_str() );
                    }

                    attribute = subchild->GetAttribute(wxT("webrootpath"), wxT(""));
                    attribute.Trim();
                    attribute.Trim(false);
                    if ( attribute.Length() ) {
                        //m_pathWebRoot = attribute;
                        strcpy( m_pathWebRoot, attribute.mbc_str() );
                    }

                    attribute = subchild->GetAttribute(wxT("authdoamin"), wxT(""));
                    attribute.Trim();
                    attribute.Trim(false);
                    if ( attribute.Length() ) {
                        strcpy( m_authDomain, attribute.mbc_str() );
                    }

                    attribute = subchild->GetAttribute(wxT("pathcert"), wxT(""));
                    attribute.Trim();
                    attribute.Trim(false);
                    if ( attribute.Length() ) {
                        strcpy( m_pathCert, attribute.mbc_str() );
                    }

                    attribute = subchild->GetAttribute(wxT("cgi_interpreter"), wxT(""));
                    attribute.Trim();
                    attribute.Trim(false);
                    if ( attribute.Length() ) {
                        strcpy( m_cgiInterpreter, attribute.mbc_str() );                    }

                    attribute = subchild->GetAttribute(wxT("cgi_pattern"), wxT(""));
                    attribute.Trim();
                    attribute.Trim(false);
                    if ( attribute.Length() ) {
                        strcpy( m_cgiPattern, attribute.mbc_str() );
                    }

                    attribute = subchild->GetAttribute(wxT("enable_directory_listing"), wxT("true"));
                    attribute.Trim();
                    attribute.Trim(false);
                    attribute.MakeUpper();
                    if ( attribute.IsSameAs(_("FALSE"), false ) ) {
                        strcpy( m_EnableDirectoryListings, "no" );
                    }

                    attribute = subchild->GetAttribute(wxT("hide_file_patterns"), wxT(""));
                    attribute.Trim();
                    attribute.Trim(false);
                    if ( attribute.Length() ) {
                        strcpy( m_hideFilePatterns, attribute.mbc_str() );
                    }

                    attribute = subchild->GetAttribute(wxT("index_files"), wxT(""));
                    attribute.Trim();
                    attribute.Trim(false);
                    if ( attribute.Length() ) {
                        strcpy( m_indexFiles, attribute.mbc_str() );
                    }

                    attribute = subchild->GetAttribute(wxT("url_rewrites"), wxT(""));
                    attribute.Trim();
                    attribute.Trim(false);
                    if ( attribute.Length() ) {
                        strcpy( m_urlRewrites, attribute.mbc_str() );
                    }

                    attribute = subchild->GetAttribute(wxT("per_directory_auth_file"), wxT(""));
                    attribute.Trim();
                    attribute.Trim(false);
                    if ( attribute.Length() ) {
                        strcpy( m_per_directory_auth_file, attribute.mbc_str() );
                    }

                    attribute = subchild->GetAttribute(wxT("global_auth_file"), wxT(""));
                    attribute.Trim();
                    attribute.Trim(false);
                    if ( attribute.Length() ) {
                        strcpy( m_global_auth_file, attribute.mbc_str() );
                    }

                    attribute = subchild->GetAttribute(wxT("ssi_pattern"), wxT(""));
                    attribute.Trim();
                    attribute.Trim(false);
                    if ( attribute.Length() ) {
                        strcpy( m_ssi_pattern, attribute.mbc_str() );
                    }

                    attribute = subchild->GetAttribute(wxT("ip_acl"), wxT(""));
                    attribute.Trim();
                    attribute.Trim(false);
                    if ( attribute.Length() ) {
                        strcpy( m_ip_acl, attribute.mbc_str() );
                    }

                    attribute = subchild->GetAttribute(wxT("dav_document_root"), wxT(""));
                    attribute.Trim();
                    attribute.Trim(false);
                    if ( attribute.Length() ) {
                        strcpy( m_dav_document_root, attribute.mbc_str() );
                    }

                    attribute = subchild->GetAttribute(wxT("run_as_user"), wxT(""));
                    attribute.Trim();
                    attribute.Trim(false);
                    m_runAsUserWeb = attribute;


                    // Get webserver sub components
                    wxXmlNode *subsubchild = subchild->GetChildren();
                    while (subsubchild) {

                        if (subsubchild->GetName() == wxT("websockets")) {

                            wxString property = subsubchild->GetAttribute(wxT("enable"), wxT("true"));

                            if (property.IsSameAs(_("false"), false)) {
                                m_bWebSockets = false;
                            }

                            property = subsubchild->GetAttribute(wxT("auth"), wxT("true"));

                                if (property.IsSameAs(_("false"), false)) {
                                 m_bAuthWebsockets = false;
                            }

                        }

                        subsubchild = subsubchild->GetNext();
                    }

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
                        m_userList.addUser(name, md5, privilege, &VSCPFilter, allowfrom, allowevent);
                    }
                    else {
                        m_userList.addUser(name, md5, privilege, NULL, allowfrom, allowevent);
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

            wxString attribut = child->GetAttribute(wxT("enable"), wxT("true"));
            attribut.MakeLower();
            if (attribut.IsSameAs(_("false"), false)) {
                m_bEnableLevel1Drivers = false;
            }
            else {
                m_bEnableLevel1Drivers = true;
            }

            wxXmlNode *subchild = child->GetChildren();
            while (subchild) {

                wxString strName;
                wxString strConfig;
                wxString strPath;
                unsigned long flags;
                cguid guid;
                bool bEnabled = true;
                bool bCanalDriver = false;

                if ( subchild->GetName() == wxT("driver") ) {

                    wxXmlNode *subsubchild = subchild->GetChildren();

                    wxString attribute = subchild->GetAttribute(wxT("enable"), wxT("true"));

                    if (attribute.IsSameAs(_("false"), false)) {
                        bEnabled = false;
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

            wxString attribut = child->GetAttribute(wxT("enable"), wxT("true"));
            attribut.MakeLower();
            if (attribut.IsSameAs(_("false"), false)) {
                m_bEnableLevel2Drivers = false;
            }
            else {
                m_bEnableLevel2Drivers = true;
            }

            wxXmlNode *subchild = child->GetChildren();

            while (subchild) {

                wxString strName;
                wxString strConfig;
                wxString strPath;
                cguid guid;
                bool bEnabled = true;
                bool bLevel2Driver = false;

                if (subchild->GetName() == wxT("driver")) {

                    wxXmlNode *subsubchild = subchild->GetChildren();

                    wxString attribute = subchild->GetAttribute(wxT("enable"), wxT("true"));

                    if (attribute.IsSameAs(_("false"), false)) {
                        bEnabled = false;
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

            wxXmlNode *subchild = child->GetChildren();
            while (subchild) {

                if (subchild->GetName() == wxT("zone")) {
                    long zone;
                    wxString strZone = subchild->GetNodeContent();
                    strZone.ToLong( &zone );
                    m_automation.setZone( zone );
                }
                else if (subchild->GetName() == wxT("sub-zone")) {
                    long subzone;
                    wxString strSubZone = subchild->GetNodeContent();
                    strSubZone.ToLong( &subzone );
                    m_automation.setSubzone( subzone );
                }
                else if (subchild->GetName() == wxT("longitude")) {
                    wxString strLongitude = subchild->GetNodeContent();
                    double d;
                    strLongitude.ToDouble( &d );
                    m_automation.setLongitude( d );
                }
                else if (subchild->GetName() == wxT("latitude")) {
                    wxString strLatitude = subchild->GetNodeContent();
                    double d;
                    strLatitude.ToDouble( &d );
                    m_automation.setLatitude( d );
                }
                else if (subchild->GetName() == wxT("timezone")) {
                    wxString strTimezone = subchild->GetNodeContent();
                    double tz;
                    strTimezone.ToDouble( &tz );
                    m_automation.setTimezone( tz );
                }
                else if (subchild->GetName() == wxT("sunrise")) {
                    m_automation.enableSunRiseEvent();
                    wxString attribute = subchild->GetAttribute(wxT("enable"), wxT("true"));
                    if (attribute.IsSameAs(_("false"), false)) {
                        m_automation.disableSunRiseEvent();
                    }
                }
                else if (subchild->GetName() == wxT("sunrise-twilight")) {
                    m_automation.enableSunRiseTwilightEvent();
                    wxString attribute = subchild->GetAttribute(wxT("enable"), wxT("true"));
                    if (attribute.IsSameAs(_("false"), false)) {
                        m_automation.disableSunRiseTwilightEvent();
                    }
                }
                else if (subchild->GetName() == wxT("sunset")) {
                    m_automation.enableSunSetEvent();
                    wxString attribute = subchild->GetAttribute(wxT("enable"), wxT("true"));
                    if (attribute.IsSameAs(_("false"), false)) {
                        m_automation.disableSunSetEvent();
                    }
                }
                else if (subchild->GetName() == wxT("sunset-twilight")) {
                    m_automation.enableSunSetTwilightEvent();
                    wxString attribute = subchild->GetAttribute(wxT("enable"), wxT("true"));
                    if (attribute.IsSameAs(_("false"), false)) {
                        m_automation.disableSunSetTwilightEvent();
                    }
                }
                else if (subchild->GetName() == wxT("daylightsavingtime-start")) {
                    wxString daylightsave = subchild->GetNodeContent();
                    wxDateTime dt;
                    dt.ParseDateTime( daylightsave );
                    m_automation.setDaylightSavingStart( dt );
                }
                else if (subchild->GetName() == wxT("daylightsavingtime-end")) {
                    wxString daylightsave = subchild->GetNodeContent();
                    wxDateTime dt;
                    dt.ParseDateTime( daylightsave );
                    m_automation.setDaylightSavingEnd( dt );
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
                    m_automation.setIntervalSegmentControllerHeartbeat( interval );
                }
                else if (subchild->GetName() == wxT("heartbeat-event")) {
                    m_automation.enableHeartbeatEvent();
                    m_automation.setIntervalHeartbeatEvent( 30 );

                    wxString attribute = subchild->GetAttribute(wxT("enable"), wxT("true"));
                    if (attribute.IsSameAs(_("false"), false)) {
                        m_automation.disableHeartbeatEvent();
                    }

                    attribute = subchild->GetAttribute(wxT("interval"), wxT("30"));
                    long interval;
                    attribute.ToLong( &interval );
                    m_automation.setIntervalHeartbeatEvent( interval );
                }

                subchild = subchild->GetNext();

            } // while

        }

        child = child->GetNext();

    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// callback_daemonConfigurationRead
//

static int callback_daemonConfigurationRead( void *data,
                                                int argc,
                                                char **argv,
                                                char **azColName )
{
    wxString wxstr;
    CControlObject *pctrlObj = (CControlObject *)data;
    if ( NULL == pctrlObj ) return -1;

    int dbVersion = atoi( argv[ DAEMON_DB_ORDINAL_CONFIG_DBVERSION ] ); // Get the version of this db file

    // Check for db version 1 data
    if ( ( 1 == dbVersion ) &&
        ( DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_HEARTBEATEVENT_INTERVAL > argc ) ) {

        // Debug level
        pctrlObj->m_logLevel = atoi( argv[ DAEMON_DB_ORDINAL_CONFIG_LOGLEVEL ] );
        if ( pctrlObj->m_logLevel > DAEMON_LOGMSG_DEBUG ) {
            pctrlObj->m_logLevel = DAEMON_LOGMSG_DEBUG;
        }

        // Run as user
        pctrlObj->m_runAsUser = wxString::FromUTF8( argv[ DAEMON_DB_ORDINAL_CONFIG_RUNASUSER ] );

        // Server GUID
        pctrlObj->m_guid.getFromString( argv[ DAEMON_DB_ORDINAL_CONFIG_GUID ] );

        // Server name
        pctrlObj->m_strServerName = wxString::FromUTF8( argv[ DAEMON_DB_ORDINAL_CONFIG_NAME ] );

        // General log file enable
        pctrlObj->m_bLogGeneralEnable = atoi( argv[ DAEMON_DB_ORDINAL_CONFIG_GENERALLOGFILE_ENABLE ] ) ? true : false;

        // Path for general log file
        pctrlObj->m_logGeneralFileName.SetPath( wxString::FromUTF8( argv[ DAEMON_DB_ORDINAL_CONFIG_GENERALLOGFILE_PATH ] ) );

        // Security log file enable
        pctrlObj->m_bLogSecurityEnable = atoi( argv[ DAEMON_DB_ORDINAL_CONFIG_SECURITYLOGFILE_ENABLE ] ) ? true : false;

        // Path to security log file
        pctrlObj->m_logSecurityFileName.SetPath( wxString::FromUTF8( argv[ DAEMON_DB_ORDINAL_CONFIG_SECURITYLOGFILE_PATH ] ) );

        // Access log file enable
        pctrlObj->m_bLogAccessEnable = atoi( argv[ DAEMON_DB_ORDINAL_CONFIG_ACCESSLOGFILE_ENABLE ] ) ? true : false;

        // Path to access log file
        pctrlObj->m_logAccessFileName.SetPath( wxString::FromUTF8( argv[ DAEMON_DB_ORDINAL_CONFIG_ACCESSLOGFILE_PATH ] ) );

        // Enable TCP/IP interface
        pctrlObj->m_bTCP = atoi( argv[ DAEMON_DB_ORDINAL_CONFIG_TCPIPINTERFACE_ENABLE ] ) ? true : false;
        
        // TCP/IP port
        pctrlObj->m_strTcpInterfaceAddress = wxString::FromUTF8( argv[ DAEMON_DB_ORDINAL_CONFIG_TCPIPINTERFACE_PORT ] );

        // Enable Multicast interface
        pctrlObj->m_bMulticastAnnounce = atoi( argv[ DAEMON_DB_ORDINAL_CONFIG_MULTICASTINTERFACE_ENABLE ] ) ? true : false;
        
        // Port for Multicast interface
        pctrlObj->m_strMulticastAnnounceAddress = wxString::FromUTF8( argv[ DAEMON_DB_ORDINAL_CONFIG_MULTICASTINTERFACE_PORT ] );
        
        // TTL for Multicast i/f
        pctrlObj->m_ttlMultiCastAnnounce = atoi( argv[ DAEMON_DB_ORDINAL_CONFIG_MULICASTINTERFACE_TTL ] );
        
        // Enable UDP interface
        pctrlObj->m_bUDP = atoi( argv[ DAEMON_DB_ORDINAL_CONFIG_UDPSIMPLEINTERFACE_ENABLE ] ) ? true : false;
        
        // UDP interface port
        pctrlObj->m_strUDPInterfaceAddress = wxString::FromUTF8( argv[ DAEMON_DB_ORDINAL_CONFIG_UDPSIMPLEINTERFACE_PORT ] );
        
        // Enable internal server functionality
        pctrlObj->m_bVSCPDaemon = atoi( argv[ DAEMON_DB_ORDINAL_CONFIG_INTERNALDAEMONFUNCTIONALITY_ENABLE ] ) ? true : false;
        
        // Enable DM functionality
        pctrlObj->m_bDM = atoi( argv[ DAEMON_DB_ORDINAL_CONFIG_DM_ENABLE ] ) ? true : false;
        
        // Path to DM database file
        pctrlObj->m_dm.m_configPath = wxString::FromUTF8( argv[ DAEMON_DB_ORDINAL_CONFIG_DM_PATH ] );
        
        // Enable DM logging
        pctrlObj->m_dm.m_bLogEnable = atoi( argv[ DAEMON_DB_ORDINAL_CONFIG_DM_LOGGING_ENABLE ] ) ? true : false;
        
        // Path to DM log file
        pctrlObj->m_dm.m_logPath = wxString::FromUTF8( argv[ DAEMON_DB_ORDINAL_CONFIG_DM_LOGGING_PATH ] );
        
        // DM logging level
        pctrlObj->m_dm.m_logLevel = atoi( argv[ DAEMON_DB_ORDINAL_CONFIG_DM_LOGGING_LEVEL ] );
        
        // Variable handling enable
        pctrlObj->m_bVariables = atoi( argv[ DAEMON_DB_ORDINAL_CONFIG_VARIABLES_ENABLE ] ) ? true : false;
        
        // Path to variable database
        pctrlObj->m_VSCP_Variables.m_configPath = wxString::FromUTF8( argv[ DAEMON_DB_ORDINAL_CONFIG_VARIABLES_PATH ] );
        
        // Client buffer size
        pctrlObj->m_maxItemsInClientReceiveQueue = atoi( argv[ DAEMON_DB_ORDINAL_CONFIG_VSCPD_DEFAULTCLIENTBUFFERSIZE ] );
        
        // Enable web server
        pctrlObj->m_bWebServer = atoi( argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_ENABLE ] ) ? true : false;
        
        // Disable web server security
        pctrlObj->m_bDisableSecurityWebServer = atoi( argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_DISABLEAUTHENTICATION ] ) ? true : false;
        
        // Web server root path
        strncpy( pctrlObj->m_pathWebRoot, 
                    argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_ROOTPATH ], 
                    MIN( strlen(argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_ROOTPATH ] ), MAX_PATH_SIZE ) );
        
        // Port for web server
        pctrlObj->m_portWebServer = wxString::FromUTF8( argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_PORT ] );
        
        // Path to cert file
        strncpy( pctrlObj->m_pathCert, 
                    argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_PATHCERT ], 
                    MIN( strlen(argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_PATHCERT ] ), MAX_PATH_SIZE ) );
        
        // Authdomain
        strncpy( pctrlObj->m_authDomain, 
                    argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_AUTHDOMAIN ], 
                    MIN( strlen(argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_AUTHDOMAIN ] ), MAX_PATH_SIZE ) );
        
        // CGI interpreter
        strncpy( pctrlObj->m_cgiInterpreter, 
                    argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_CGIINTERPRETER ], 
                    MIN( strlen(argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_CGIINTERPRETER ] ), MAX_PATH_SIZE ) );
        
        // CGI pattern
        strncpy( pctrlObj->m_cgiPattern, 
                    argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_CGIPATTERN ], 
                    MIN( strlen(argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_CGIPATTERN ] ), MAX_PATH_SIZE ) );
        
        // Enable directory listings
        if ( atoi( argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_ENABLEDIRECTORYLISTINGS ] ) ) {
            strcpy( pctrlObj->m_EnableDirectoryListings, "yes" );
        }
        else {
            strcpy( pctrlObj->m_EnableDirectoryListings, "no" );
        }
        
        // Hide file patterns
        strncpy( pctrlObj->m_hideFilePatterns, 
                    argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_HIDEFILEPATTERNS ], 
                    MIN( strlen(argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_HIDEFILEPATTERNS ] ), MAX_PATH_SIZE ) );
        
        // Index files
        strncpy( pctrlObj->m_indexFiles, 
                    argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_INDEXFILES ], 
                    MIN( strlen(argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_INDEXFILES ] ), MAX_PATH_SIZE ) );
        
        // Extra mime types
        strncpy( pctrlObj->m_extraMimeTypes, 
                    argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_EXTRAMIMETYPES ], 
                    MIN( strlen(argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_EXTRAMIMETYPES ] ), MAX_PATH_SIZE ) );
        
        // URL rewrites
        strncpy( pctrlObj->m_urlRewrites, 
                    argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_URLREWRITES ], 
                    MIN( strlen(argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_URLREWRITES ] ), MAX_PATH_SIZE ) );
        
        // SSI patterns
        strncpy( pctrlObj->m_ssi_pattern, 
                    argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_SSIPATTERN ], 
                    MIN( strlen(argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_SSIPATTERN ] ), MAX_PATH_SIZE ) );
        
        // Webserver user
        pctrlObj->m_runAsUserWeb = wxString::FromUTF8( argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_RUNASUSER ] );
        
        // Per directory auth. file
        strncpy( pctrlObj->m_per_directory_auth_file, 
                    argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_PERDIRECTORYAUTHFILE ], 
                    MIN( strlen(argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_PERDIRECTORYAUTHFILE ] ), MAX_PATH_SIZE ) );
        
        // Global auth. file
        strncpy( pctrlObj->m_global_auth_file, 
                    argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_GLOBALAUTHFILE ], 
                    MIN( strlen(argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_GLOBALAUTHFILE ] ), MAX_PATH_SIZE ) );
        
        // IP ACL
        strncpy( pctrlObj->m_ip_acl, 
                    argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSERVER__IPACL ], 
                    MIN( strlen(argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSERVER__IPACL ] ), MAX_PATH_SIZE ) );
        
        // DAV path
        strncpy( pctrlObj->m_dav_document_root, 
                    argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_DAVDOCUMENTROOT ], 
                    MIN( strlen(argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSERVER_DAVDOCUMENTROOT ] ), MAX_PATH_SIZE ) );
        
        // Enable web socket interface
        pctrlObj->m_bWebSockets = atoi( argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSOCKET_ENABLE ] ) ? true : false;
        
        // Enable web socket authentication
        pctrlObj->m_bAuthWebsockets = atoi( argv[ DAEMON_DB_ORDINAL_CONFIG_WEBSOCKET_ENABLEAUTH ] ) ? true : false;
        
        // Enable MQTT broker
        pctrlObj->m_bMQTTBroker = atoi( argv[ DAEMON_DB_ORDINAL_CONFIG_MQTTBROKER_ENABLE ] ) ? true : false;
        
        // MQTT broker port
        pctrlObj->m_strMQTTBrokerInterfaceAddress = wxString::FromUTF8( argv[ DAEMON_DB_ORDINAL_CONFIG_MQTTBROKER_PORT ] );
        
        // Enable COAP server
        pctrlObj->m_bCoAPServer = atoi( argv[ DAEMON_DB_ORDINAL_CONFIG_COAPSERVER_ENABLE ] ) ? true : false;
        
        // COAP port
        pctrlObj->m_strCoAPServerInterfaceAddress = wxString::FromUTF8( argv[ DAEMON_DB_ORDINAL_CONFIG_COAPSERVER_PORT ] );
        
        // Enable automation
        if ( atoi( argv[ DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_ENABLE ] ) ) {
            pctrlObj->m_automation.enableAutomation();
        }
        else {
            pctrlObj->m_automation.disableAutomation();
        }
        
        // Automation zone
        pctrlObj->m_automation.setZone( atoi( argv[ DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_ZONE ] ) );
        
        // Automation sub zone
        pctrlObj->m_automation.setZone( atoi( argv[ DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_SUBZONE ] ) );
        
        // Automation longitude
        pctrlObj->m_automation.setLongitude( atof(argv[ DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_LONGITUDE ] ) );
        
        // Automation latitude
        pctrlObj->m_automation.setLongitude( atof(argv[ DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_LATITUDE ] ) );
        
        // Automation time zone
        pctrlObj->m_automation.setTimezone( atof( argv[ DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_TIMEZONE ] ) );
        
        // Automation enable sun rise event
        if ( argv[ DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_SUNRISE_ENABLE ] ) {
            pctrlObj->m_automation.enableSunRiseEvent();
        }
        else {
            pctrlObj->m_automation.disableSunRiseEvent();
        }
        
        // Automation enable sun set event
        if ( argv[ DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_SUNSET_ENABLE ] ) {
            pctrlObj->m_automation.enableSunSetEvent();
        }
        else {
            pctrlObj->m_automation.disableSunSetEvent();
        }
        
        // Automation enable sunset twilight event
        if ( argv[ DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_SUNSETTWILIGHT_ENABLE ] ) {
            pctrlObj->m_automation.enableSunSetTwilightEvent();
        }
        else {
            pctrlObj->m_automation.disableSunSetTwilightEvent();
        }
        
        // Automation enable sunrise twilight event
        if ( argv[ DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_SUNRISETWILIGHT_ENABLE ] ) {
            pctrlObj->m_automation.enableSunRiseTwilightEvent();
        }
        else {
            pctrlObj->m_automation.disableSunRiseTwilightEvent();
        }
        
        // Automation segment controller event enable
        if ( argv[ DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_SEGMENTCONTROLLEREVENT_ENABLE ] ) {
            pctrlObj->m_automation.enableSegmentControllerHeartbeat();
        }
        else {
            pctrlObj->m_automation.disableSegmentControllerHeartbeat();
        }
        
        // Automation, segment controller heartbeat interval
        pctrlObj->m_automation.setIntervalSegmentControllerHeartbeat( atoi( argv[ DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_SEGMENTCONTROLLEREVENT_INTERVAL ] ) );
        
        // Automation heartbeat event enable
        if ( argv[ DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_HEARTBEATEVENT_ENABLE ] ) {
            pctrlObj->m_automation.enableHeartbeatEvent();
        }
        else {
            pctrlObj->m_automation.disableHeartbeatEvent();
        }
        
        // Automation heartbeat interval
        pctrlObj->m_automation.setIntervalHeartbeatEvent( atoi( argv[ DAEMON_DB_ORDINAL_CONFIG_AUTOMATION_HEARTBEATEVENT_INTERVAL ] ) );
        
    }

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

    // Check if database is open
    if ( NULL == m_db_vscp_daemon ) return false;

    if ( SQLITE_OK != sqlite3_exec( m_db_vscp_daemon,
                                        psql,
                                        callback_daemonConfigurationRead,
                                        (void*)gpctrlObj,
                                        &pErrMsg ) ) {
        fprintf( stderr, "Error reading configuration (SQL): %s\n", pErrMsg );
        sqlite3_free( pErrMsg );
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

