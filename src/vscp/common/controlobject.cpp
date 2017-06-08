// ControlObject.cpp: m_path_db_vscp_logimplementation of the CControlObject class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (http://www.vscp.org)
//
// Copyright (C) 2000-2017 Ake Hedman,
// Grodans Paradis AB, <akhe@grodansparadis.com>
//
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License#include <udpclientthread.h>
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

#include <sqlite3.h>
#include <mongoose.h>
#include <v7.h>

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
#include <webserver_websocket.h>
#include <vscpd_caps.h>
#include <controlobject.h>


#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif


// Lists
WX_DEFINE_LIST(WEBSOCKETSESSIONLIST); // websocket sessions
WX_DEFINE_LIST(TRIGGERLIST);        // websocket triggers

WX_DEFINE_LIST(CanalMsgList); 
WX_DEFINE_LIST(VSCPEventList);

//#define DEBUGPRINT

extern CControlObject *gpobj;

#ifdef WIN32

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



void vscp_md5( char *digest, const unsigned char *buf, size_t len ) 
{
    unsigned char hash[16];
  
    MD5_CTX ctx;

    MD5_Init( &ctx );
    MD5_Update( &ctx, buf, len );
    MD5_Final( hash, &ctx );
    cs_to_hex( digest, hash, sizeof( hash ) );
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CControlObject::CControlObject()
{
    int i;
    m_bQuit = false;            // true if we should quit
    
#ifdef WIN32
    m_rootFolder = wxStandardPaths::Get().GetUserDataDir();
#else
    m_rootFolder = _("/srv/vscp/");
#endif            
    
    // Delete objects from list when they are removed
    m_websocketSessions.DeleteContents( true );
    
    // Default admin user credentials
    m_admin_user = _("admin");
    m_admin_password = _("secret");
    m_admin_allowfrom = _("*");
    m_vscptoken = _("Carpe diem quam minimum credula postero");
    
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

    // Web server security should be used.
    m_bDisableSecurityWebServer = false;

    // Local domain
    strcpy( m_authDomain, "mydomain.com" );

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
    

    // Control UDP Interface
    m_udpInfo.m_bEnable = false;
    m_udpInfo.m_interface.Empty();
    m_udpInfo.m_guid.clear();
    vscp_clearVSCPFilter( &m_udpInfo.m_filter );
    m_udpInfo.m_bAllowUnsecure = false;
    m_udpInfo.m_bAck = false;

    // Enable MQTT broker
    m_bMQTTBroker = true;

    // Default TCP/IP interface
    m_strTcpInterfaceAddress = _("tcp://9598");

    // Default multicast announce port
    m_strMulticastAnnounceAddress = _( "udp://:" + VSCP_MULTICAST_DEFAULT_PORT );

    // default multicast announce ttl
    m_ttlMultiCastAnnounce = IP_MULTICAST_DEFAULT_TTL;

    // Default UDP interface
    m_udpInfo.m_interface = _("udp://:" + VSCP_DEFAULT_UDP_PORT);

    // Default MQTT broker interface
    m_strMQTTBrokerInterfaceAddress = _("tcp://1883");

    m_pclientMsgWorkerThread = NULL;
    m_pVSCPClientThread = NULL;
    m_pdaemonVSCPThread = NULL;
    m_pwebServerThread = NULL;
    m_pMQTTBrookerThread = NULL;

    // Websocket interface
    m_bAuthWebsockets = true;   // Authentication is needed
    memset( m_pathCert, 0, sizeof( m_pathCert ) );

    // Webserver interface
    m_strWebServerInterfaceAddress = _("tcp://8080");

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

    //m_mutexClientOutputQueue.Lock();
    for (iterVSCP = m_clientOutputQueue.begin();
            iterVSCP != m_clientOutputQueue.end(); ++iterVSCP) {
        vscpEvent *pEvent = *iterVSCP;
        vscp_deleteVSCPevent( pEvent );
    }

    m_clientOutputQueue.Clear();
    //m_mutexClientOutputQueue.Unlock();
    
    //gpobj->m_mutexUDPInfo.Lock();
    udpRemoteClientList::iterator iterUDP;
    for (iterUDP = m_udpInfo.m_remotes.begin();
            iterUDP != m_udpInfo.m_remotes.end(); ++iterUDP) {
        if ( NULL != *iterUDP ) {
            delete *iterUDP;
            *iterUDP = NULL;
        }
        
    }
    //gpobj->m_mutexUDPInfo.Unlock();
    
    m_udpInfo.m_remotes.Clear();
    
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
    
    // Fill info in MSB of 16-bit code
    pCapability[ 14 ] = VSCP_DAEMON_SERVER_CAPABILITIES_6;
            
    // Fill info in LSB of 16-bit code
    pCapability[ 15 ] = VSCP_DAEMON_SERVER_CAPABILITIES_7;
    
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// logMsg
//

void CControlObject::logMsg(const wxString& msgin, const uint8_t level, const uint8_t nType )
{
    wxString msg = msgin;
    
    m_mutexLogWrite.Lock();
    
    // Add CR if not set.
    if ( wxNOT_FOUND == msg.find( _("\n") ) ) {
        msg += _("\n");
    }

    wxDateTime datetime( wxDateTime::GetTimeNow() );
    wxString wxdebugmsg;

    wxdebugmsg = datetime.FormatISODate() + _("T") + datetime.FormatISOTime() + _(" - ") + msg;

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
        
        
        char *zErrMsg = 0;
                
        char *sql = sqlite3_mprintf( VSCPDB_LOG_INSERT,
            nType, 
            (const char *)(datetime.FormatISODate() + _("T") + datetime.FormatISOTime() ).mbc_str(),
            level,
            (const char *)msg.mbc_str() );
                
        if ( SQLITE_OK != sqlite3_exec( m_db_vscp_log,  
                                        sql, NULL, NULL, &zErrMsg)) {
            wxPrintf( "Failed to write message to log database. Error is: %s Message is: %s\n",
                        zErrMsg,
                        (const char *)msg.mbc_str() );
        }

        sqlite3_free( sql );
        
    }

#ifndef WIN32

    // Print to console if there is one
    if ( m_logLevel >= level ) {
        wxPrintf( "%s", wxdebugmsg.mbc_str() );
    }

    if ( m_bLogToSysLog ) {
        
        switch (level) {

        case DAEMON_LOGMSG_NORMAL:
            syslog(LOG_INFO, "%s", (const char *) wxdebugmsg.ToAscii());
            break;

        case DAEMON_LOGMSG_DEBUG:
        default:    
            syslog(LOG_DEBUG, "%s", (const char *) wxdebugmsg.ToAscii());
            break;

        };
        
    }

#endif

     m_mutexLogWrite.Unlock();

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
        logMsg( wxString::Format( _("Failed to get records from log database. SQL is %s"),
                                    sql )  );
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
        wxString msg = sqlite3_column_text( ppStmt, 4 );
        wxstr += wxBase64Encode( msg, msg.Length() ); 
        wxstr += _(";");
        strResult += wxstr;
    }
        
    m_mutexLogWrite.Unlock();
    
    return true;
}



/////////////////////////////////////////////////////////////////////////////
// init
//

bool CControlObject::init( wxString& strcfgfile, wxString& rootFolder )
{
    wxString str;
        
    /*
    wxDateTime ttt;
    ttt.ParseISOCombined(_("1970-12-31T12:00:00+02:00"));
    //ttt = ttt.ToTimezone( wxDateTime::TimeZone(-2 ) ); // .ToUTC(true);
    str = ttt.FormatISOCombined();
    
    str = _("1970-12-31T12:00:00.345+02:00Z");
    str = str.AfterFirst('.');
    uint32_t ms = vscp_readStringValue( str );
    
    str = str.AfterFirst('+');
    if ( str.Length() ) {
        
    }
    else {
        str = str.AfterFirst('+');
    }
    
    wxString strMinus = str.AfterFirst('-');
    */
            
    /*
    // https://www.w3.org/TR/NOTE-datetime 
    wxDateTime ttt;
    ttt.ParseISOCombined(_("1970-12-31T12:00:00.444"));
    ttt.SetMillisecond( 123 );
    unsigned short ms = ttt.GetMillisecond();
    */
    
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
    strcpy( m_pathWebRoot, (const char *)strRootwww.mbc_str() );
    
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
        printf("No configuration file. Can't initialize!.");
        fprintf( stderr, "No configuration file. Can't initialize!.\n" );
        str = _("Path = .") + strcfgfile + _("\n");
        fprintf( stderr, str.mbc_str() );
        return false;
    }
    
    ////////////////////////////////////////////////////////////////////////////
    //                       Read XML configuration
    ////////////////////////////////////////////////////////////////////////////

    str = _("Using configuration file: ") + strcfgfile + _("\n");
    fprintf( stderr, str.mbc_str() );

    // Read XML configuration
    if ( !readXMLConfiguration( strcfgfile ) ) {
        fprintf( stderr, "Unable to open/parse configuration file. Can't initialize!\n" );
        str = _("Path = .") + strcfgfile + _("\n");
        fprintf( stderr, str.mbc_str() );
        return FALSE;
    }
    
    

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
                if ( !doCreateConfigurationTable() ) {
                    fprintf( stderr, "Failed to create configuration table.\n" );
                }
                
                // Create the multicast database
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
                if ( doCreateLocationTable() ) {
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
                    fprintf( stderr, "Failed to create subzone table.\n" );
                }
        
                // Create userdef table
                if ( !doCreateUserdefTableTable() ) {
                    fprintf( stderr, "Failed to create userdef table.\n" );
                }
                
                // * * * All created * * *
                
                // Database is open. Read configuration data from it
                if ( !dbReadConfiguration() ) {
                    fprintf( stderr, "Failed to rea configuration from configuration database.\n" );
                }
            
            }
            
        }
        else {
            fprintf( stderr, "VSCP Server configuration database path invalid - will exit.\n" );
            str.Printf(_("Path=%s\n"),(const char *)m_path_db_vscp_daemon.GetFullPath().mbc_str() );
            fprintf( stderr, str.mbc_str() );
            return false;
        }

    }
    
    // Read UDP nodes
    readUdpNodes();
    
    
    // * * * VSCP Daemon logging database * * *
    
    
    // Check filename 
    if ( m_path_db_vscp_log.IsOk() && m_path_db_vscp_log.FileExists() ) {

        if ( SQLITE_OK != sqlite3_open( (const char *)m_path_db_vscp_log.GetFullPath().mbc_str(),
                                            &m_db_vscp_log ) ) {

            // Failed to open/create the database file
            fprintf( stderr, "VSCP Server logging database could not be opened. - Will not be used.\n" );
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
            fprintf( stderr, "VSCP Server logging database does not exist - will be created.\n" );
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
            fprintf( stderr, "VSCP Server logging database path invalid - will not be used.\n" );
            str.Printf(_("Path=%s\n"), (const char *)m_path_db_vscp_log.GetFullPath().mbc_str() );
            fprintf( stderr, str.mbc_str() );
        }

    }
    
    // https://www.sqlite.org/wal.html
    // http://stackoverflow.com/questions/3852068/sqlite-insert-very-slow
    sqlite3_exec( m_db_vscp_log, "PRAGMA journal_mode = WAL", NULL, NULL, NULL );
    sqlite3_exec( m_db_vscp_log, "PRAGMA synchronous = NORMAL", NULL, NULL, NULL );
    
    // * * * VSCP Server data database - NEVER created * * *

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
    
        
      
    // Read users from database
    logMsg(_("loading users from users db...\n") );
    m_userList.loadUsers();
    
    
    
    //==========================================================================
    //                           Add admin user
    //==========================================================================
    
    if ( 0 == m_admin_allowfrom.Length() ) m_admin_allowfrom = _("*");
    m_userList.addUser( m_admin_user,
                            m_admin_password,
                            _("Admin user"),            // full name
                            _("Admin user"),            // note
                            NULL,
                            _("admin"),
                            m_admin_allowfrom,          // Remotes allows to connect     
                            _("*:*"),                   // All events
                            VSCP_ADD_USER_FLAG_ADMIN | 
                            VSCP_ADD_USER_FLAG_LOCAL ); 
    
    
    //==========================================================================
    //                           Add driver user
    //==========================================================================
    
    // Generate username and password for drivers
    char buf[128];
    randPassword pw;

    // Level II Driver Username
    memset( buf, 0, sizeof( buf ) );
    pw.generatePassword( 32, buf );
    m_driverUsername = _("drv_");
    m_driverUsername += wxString::FromAscii( buf );

    // Level II Driver Password
    memset( buf, 0, sizeof( buf ) );
    pw.generatePassword( 32, buf );
    m_driverPassword = wxString::FromAscii( buf );

    m_userList.addUser( m_driverUsername,
                            m_driverPassword,
                            _("System added driver user."), // full name
                            _("System added driver user."), // note
                            NULL,
                            _("driver"),
                            _("127.0.0.1"),                 // Only local
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

    // Load decision matrix if mechanism is enabled    
    logMsg(_("Loading DM.\n") );
    m_dm.loadFromXML();

    // Initialize variable storage
    logMsg(_("Initialize variables.\n") );
    m_VSCP_Variables.init();
    
    // Load variables if mechanism is enabled
    logMsg(_("Loading persistent variables from XML variable default path.\n") );
    m_VSCP_Variables.loadFromXML();

    // Start daemon internal client worker thread    
    startClientWorkerThread();

    // Start TCP/IP interface    
    startTcpWorkerThread();
    
    // Start UDP interface
    startUDPWorkerThread();

    // Start web sockets    
    startWebServerThread();

    // Load drivers    
    startDeviceWorkerThreads();

    // Start MQTT Broker if enabled
    startMQTTBrokerThread();
    
    // Start daemon worker thread    
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
    pClientItem->m_strDeviceName = _("Internal Server DM Client. Started at ");
    pClientItem->m_strDeviceName += wxDateTime::Now().FormatISODate();
    pClientItem->m_strDeviceName += _(" ");
    pClientItem->m_strDeviceName += wxDateTime::Now().FormatISOTime();

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

bool CControlObject::cleanup( void )
{
    logMsg(_("Giving threads time to stop operations."),DAEMON_LOGMSG_DEBUG);
    sleep( 2 ); // Give threads some time to end
    
    logMsg(_("Stopping VSCP Server worker thread."),DAEMON_LOGMSG_DEBUG);
    stopDaemonWorkerThread();
    
    logMsg(_("Stopping client worker thread."),DAEMON_LOGMSG_DEBUG);
    stopClientWorkerThread();
    
    logMsg(_("Stopping device worker thread."),DAEMON_LOGMSG_DEBUG);
    stopDeviceWorkerThreads();
    
    logMsg(_("Stopping TCP/IP worker thread."),DAEMON_LOGMSG_DEBUG);
    //stopTcpWorkerThread();
    
    logMsg(_("Stopping Web Server worker thread."),DAEMON_LOGMSG_DEBUG);
    stopWebServerThread();
    
   
    logMsg(_("Closing databases."),DAEMON_LOGMSG_DEBUG);
    
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
    
    wxLogDebug( _("ControlObject: Cleanup done.") );
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
    
    logMsg(_("Starting client worker thread...\n") );
    
    m_pclientMsgWorkerThread = new clientMsgWorkerThread;

    if (NULL != m_pclientMsgWorkerThread) {
        m_pclientMsgWorkerThread->m_pCtrlObject = this;
        wxThreadError err;
        if ( wxTHREAD_NO_ERROR == ( err = m_pclientMsgWorkerThread->Create() ) ) {
            //m_ptcpListenThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY );
            if ( wxTHREAD_NO_ERROR != ( err = m_pclientMsgWorkerThread->Run() ) ) {
                logMsg( _("Unable to run controlobject client thread.") );
            }
        } 
        else {
            logMsg( _("Unable to create controlobject client thread.") );
        }
    } 
    else {
        logMsg( _("Unable to allocate memory for controlobject client thread.") );
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
    
    logMsg(_("Starting TCP/IP interface...\n") );

    m_pVSCPClientThread = new VSCPClientThread;

    if ( NULL != m_pVSCPClientThread ) {
        
        wxThreadError err;
        if ( wxTHREAD_NO_ERROR == ( err = m_pVSCPClientThread->Create() ) ) {
                //m_ptcpListenThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY );
            if ( wxTHREAD_NO_ERROR != ( err = m_pVSCPClientThread->Run() ) ) {
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

bool CControlObject::stopTcpWorkerThread( void )
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

bool CControlObject::startUDPWorkerThread( void )
{
    /////////////////////////////////////////////////////////////////////////////
    // Run the UDP server thread
    /////////////////////////////////////////////////////////////////////////////
    if ( m_udpInfo.m_bEnable ) {

        logMsg(_("Starting UDP simple interface...\n") );
        
        m_pVSCPClientUDPThread = new VSCPUDPClientThread;

        if ( NULL != m_pVSCPClientUDPThread ) {
            wxThreadError err;
            if (wxTHREAD_NO_ERROR == (err = m_pVSCPClientUDPThread->Create())) {
                //m_ptcpListenThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY );
                if (wxTHREAD_NO_ERROR != (err = m_pVSCPClientUDPThread->Run())) {
                    logMsg( _("Unable to run UDP client thread.") );
                }
            }
            else {
                logMsg( _("Unable to create UDP client thread.") );
            }
        }
        else {
            logMsg( _("Unable to allocate memory for UDP client thread.") );
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
        delete m_pVSCPClientThread;
        m_mutexVSCPClientnUDPThread.Unlock();
    }

    return true;
}


/////////////////////////////////////////////////////////////////////////////
// startWebServerThread
//

bool CControlObject::startWebServerThread( void )
{
    /////////////////////////////////////////////////////////////////////////////
    // Run the WebServer server thread
    /////////////////////////////////////////////////////////////////////////////
    
    logMsg(_("Starting webserver/websockets interface...\n") );

    m_pwebServerThread = new VSCPWebServerThread;

    if ( NULL != m_pwebServerThread ) {
        m_pwebServerThread->m_pCtrlObject = this;
        wxThreadError err;
        if ( wxTHREAD_NO_ERROR == (err = m_pwebServerThread->Create() ) ) {
            //m_ptcpListenThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY );
            if ( wxTHREAD_NO_ERROR != ( err = m_pwebServerThread->Run() ) ) {
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

bool CControlObject::stopWebServerThread( void )
{
    if ( NULL != m_pwebServerThread ) {
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

bool CControlObject::startDaemonWorkerThread( void )
{
    /////////////////////////////////////////////////////////////////////////////
    // Run the VSCP daemon thread
    /////////////////////////////////////////////////////////////////////////////
    
    logMsg(_("Starting VSCP Server worker thread,,.\n") );
    
    m_pdaemonVSCPThread = new daemonVSCPThread;

    if ( NULL != m_pdaemonVSCPThread ) {
            m_pdaemonVSCPThread->m_pCtrlObject = this;

        wxThreadError err;
        if ( wxTHREAD_NO_ERROR == ( err = m_pdaemonVSCPThread->Create() ) ) {
            m_pdaemonVSCPThread->SetPriority(WXTHREAD_DEFAULT_PRIORITY);
            if ( wxTHREAD_NO_ERROR != ( err = m_pdaemonVSCPThread->Run() ) ) {
                logMsg( _("Unable to start TCP VSCP Server thread.") );
            }
        }
        else {
            logMsg( _("Unable to create TCP VSCP Server thread.") );
        }
    }
    else {
        logMsg( _("Unable to start VSCP Server thread.") );
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
    
    logMsg(_("Starting drivers...\n") );

    VSCPDEVICELIST::iterator iter;
    for ( iter = m_deviceList.m_devItemList.begin();
            iter != m_deviceList.m_devItemList.end();
            ++iter ) {

        pDeviceItem = *iter;
        if ( NULL != pDeviceItem ) {

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
// startMQTTBrokerWorkerThread
//

bool CControlObject::startMQTTBrokerThread(void)
{
    /////////////////////////////////////////////////////////////////////////////
    // Run the MQTT Broker thread if enabled
    /////////////////////////////////////////////////////////////////////////////
    if ( m_bMQTTBroker ) {

#ifdef MG_ENABLE_MQTT_BROKER        
        
        logMsg(_("MQTT Broker starting...\n") );
        
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

bool CControlObject::stopMQTTBrokerThread( void )
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
                    wxString::Format( _("Level I event over Level II dest = %d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:"),
                    destguid.getAt(15),destguid.getAt(14),destguid.getAt(13),destguid.getAt(12),
                    destguid.getAt(11),destguid.getAt(10),destguid.getAt(9),destguid.getAt(8),
                    destguid.getAt(7),destguid.getAt(6),destguid.getAt(5),destguid.getAt(4),
                    destguid.getAt(3),destguid.getAt(2),destguid.getAt(1),destguid.getAt(0) );
                    logMsg( dbgStr, DAEMON_LOGMSG_DEBUG );

            // Find client
            m_wxClientMutex.Lock();

            CClientItem *pDestClientItem = NULL;
            VSCPCLIENTLIST::iterator iter;
            for (iter = m_clientList.m_clientItemList.begin();
                    iter != m_clientList.m_clientItemList.end();
                    ++iter) {

                CClientItem *pItem = *iter;
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
    m_clientList.addClient( pClientItem, id );

    // Add mapped item
    addIdToClientMap( pClientItem->m_clientID );

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
        logMsg( wxString::Format( _("Ethernet MAC address: %02X:%02X:%02X:%02X:%02X\n"),
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
            while ( subchild ) {

                if ( subchild->GetName() == wxT("security") ) {
                    m_admin_user = subchild->GetAttribute( wxT("user"), wxT("admin") );
                    m_admin_password = subchild->GetAttribute( wxT("password"), wxT("secret") );
                    m_admin_allowfrom = subchild->GetAttribute( wxT("allowfrom"), wxT("*") ); 
                    m_vscptoken = subchild->GetAttribute( wxT("vscptoken"), 
                                                            wxT("Carpe diem quam minimum credula postero") );
                }
                else if ( subchild->GetName() == wxT("loglevel") ) {
                    
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
                else if (subchild->GetName() == wxT("tcpip")) {

                    m_strTcpInterfaceAddress = subchild->GetAttribute(wxT("interface"), wxT(""));
                    m_strTcpInterfaceAddress.Trim(true);
                    m_strTcpInterfaceAddress.Trim(false);

                }
                else if ( subchild->GetName() == wxT( "multicast-announce" ) ) {

                    m_strMulticastAnnounceAddress = subchild->GetAttribute( wxT( "interface" ), wxT( "" ) );

                    m_ttlMultiCastAnnounce = vscp_readStringValue( subchild->GetAttribute( wxT( "ttl" ), wxT( "1" ) ) );

                }
                else if (subchild->GetName() == wxT("udp")) {
   
                    gpobj->m_mutexUDPInfo.Lock();
                    
                    // Enable
                    wxString attribute = subchild->GetAttribute(wxT("enable"), wxT("true"));
                    attribute.MakeLower();          
                    if (attribute.IsSameAs(_("false"), false)) {
                        m_udpInfo.m_bEnable = false; 
                    }
                    else {
                        m_udpInfo.m_bEnable  = true; 
                    }
                    
                    // Allow insecure connections
                    attribute = subchild->GetAttribute( wxT("bAllowUnsecure"), wxT("true") );
                    if (attribute.Lower().IsSameAs(_("false"), false)) {
                        m_udpInfo.m_bAllowUnsecure = false; 
                    }
                    else {
                        m_udpInfo.m_bAllowUnsecure  = true; 
                    }
                    
                    // Enable ACK
                    attribute = subchild->GetAttribute( wxT("bAck"), wxT("false") );                    
                    if (attribute.Lower().IsSameAs(_("false"), false)) {
                        m_udpInfo.m_bAck = false; 
                    }
                    else {
                        m_udpInfo.m_bAck = true; 
                    }                    
                    
                    // Username
                    m_udpInfo.m_user = subchild->GetAttribute( _("user"), _("") );
                        
                    // Password
                    m_udpInfo.m_password = subchild->GetAttribute( _("password"), _(""));

                    // Interface
                    m_udpInfo.m_interface = subchild->GetAttribute( _("interface"), _("udp://"+VSCP_MULTICAST_DEFAULT_PORT));
                    
                    // GUID
                    attribute = subchild->GetAttribute( _("guid"), _("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00") );
                    m_udpInfo.m_guid.getFromString( attribute );
                    
                    // Filter
                    attribute = subchild->GetAttribute( _("filter"), _("") );
                    if ( attribute.Trim().Length() ) {
                        vscp_readFilterFromString( &m_udpInfo.m_filter, attribute );
                    }
                    
                    // Mask
                    attribute = subchild->GetAttribute( _("mask"), _("") );
                    if ( attribute.Trim().Length() ) {                    
                        vscp_readMaskFromString( &m_udpInfo.m_filter, attribute );
                    }
                                    
                    wxXmlNode *subsubchild = subchild->GetChildren();
                    while ( subsubchild ) {
                        
                        if ( subsubchild->GetName() == _("rxnode") ) {
                        
                            udpRemoteClientInfo *pudpClient = new udpRemoteClientInfo;
                            if ( NULL == pudpClient ) {
                                logMsg( _("Failed to allocated UDP client remote structure.\n") );
                                gpobj->m_mutexUDPInfo.Unlock();
                                continue;
                            }
                                                        
                            attribute = subchild->GetAttribute( wxT("enable"), wxT("false") );
                            if ( attribute.Lower().IsSameAs(_("false"), false ) ) {
                                pudpClient->m_bEnable = false; 
                            }
                            else {
                                pudpClient->m_bEnable = true; 
                            }               
                            
                            if ( !pudpClient->m_bEnable ) {
                                delete pudpClient;
                                gpobj->m_mutexUDPInfo.Unlock();
                                continue;
                            }
                            
                            // remote address
                            pudpClient->m_remoteAddress = subchild->GetAttribute( _("interface"), _("") );    
                                                                
                            // Filter
                            attribute = subchild->GetAttribute( _("filter"), _("") );
                            if ( attribute.Trim().Length() ) {
                                vscp_readFilterFromString( &pudpClient->m_filter, attribute );
                            }
                    
                            // Mask
                            attribute = subchild->GetAttribute( _("mask"), _("") );
                            if ( attribute.Trim().Length() ) {
                                vscp_readMaskFromString( &pudpClient->m_filter, attribute );
                            }
                            
                            // broadcast
                            attribute = subchild->GetAttribute( _("bSetBroadcast"), _("false") );
                            if ( attribute.Lower().IsSameAs(_("false"), false ) ) {
                                pudpClient->m_bSetBroadcast = false; 
                            }
                            else {
                                pudpClient->m_bSetBroadcast = true; 
                            }
                            
                            // encryption
                            attribute = subchild->GetAttribute( _("encryption"), _("") );
                            pudpClient->m_nEncryption = vscp_getEncryptionCodeFromToken( attribute );
                            
                            // add to list
                            m_udpInfo.m_remotes.Append( pudpClient->m_remoteAddress, pudpClient );
                            
                        }
                        
                        subsubchild = subsubchild->GetNext();    
                        
                    }
                    
                    gpobj->m_mutexUDPInfo.Unlock();

                }
                else if (subchild->GetName() == wxT("multicast")) {
                    
                    wxString attribut = subchild->GetAttribute(wxT("enable"), wxT("true"));
                    attribut.MakeLower();
                    if (attribut.IsSameAs(_("false"), false)) {
                        m_bMulticast = false;
                    }
                    else {
                        m_bMulticast = true;
                    }
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
                    
                    attrib = subchild->GetAttribute(wxT("pathxml"), m_rootFolder + _("variable.xml"));
                    fileName.Assign( attrib );
                    if ( fileName.IsOk() ) {
                        m_VSCP_Variables.m_xmlPath = fileName.GetFullPath();
                    }
                    
                    attrib = subchild->GetAttribute(wxT("pathdb"), m_rootFolder + _("variable.sqlite3"));
                    fileName.Assign( attrib );
                    if ( fileName.IsOk() ) {
                        m_VSCP_Variables.m_dbFilename = fileName;
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

                    wxString attribute;

                    attribute = subchild->GetAttribute( wxT( "disableauthentication" ), wxT( "false" ) );

                    if ( attribute.IsSameAs( _( "true" ), true ) ) {                        
                        m_bDisableSecurityWebServer = true;
                    }

                    attribute = subchild->GetAttribute(wxT("port"), wxT("8080"));
                    attribute.Trim();
                    attribute.Trim(false);
                    if ( attribute.Length() ) {
                        m_strWebServerInterfaceAddress = attribute;
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
                        strcpy( m_cgiInterpreter, attribute.mbc_str() );                    
                    }

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

                        // Deprecated (moved outside of webserver)
                        if (subsubchild->GetName() == wxT("websockets")) {

                            wxString property;

                            property = subsubchild->GetAttribute(wxT("auth"), wxT("true"));

                                if (property.IsSameAs(_("false"), false)) {
                                 m_bAuthWebsockets = false;
                            }

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
                if ( bCanalDriver && bEnabled ) {

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

                    if (!m_deviceList.addItem( strName,
                                                strConfig,
                                                strPath,
                                                0,
                                                guid,
                                                VSCP_DRIVER_LEVEL2,
                                                bEnabled )) {
                        wxString errMsg = _("Driver was not added. Path does not exist. - [ ") +
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
        // Tables
        else if ( child->GetName() == wxT("tables") ) {

            wxXmlNode *subchild = child->GetChildren();
            while ( subchild ) {

                if ( subchild->GetName() == wxT("table") ) {
                    
                    // Check if enabled
                    wxString strEnabled = subchild->GetAttribute( wxT("enable"), wxT("true") );
                    strEnabled.MakeUpper();
                    if ( wxNOT_FOUND != strEnabled.Find( _("FALSE") ) ) {
                        goto xml_table_error;
                    }

                    // Get name of table
                    wxString name = subchild->GetAttribute( wxT("name"), wxT("") );;
                    
                    // Get type of table
                    vscpTableType type = VSCP_TABLE_DYNAMIC;
                    int size = 0;
                    wxString attribute = subchild->GetAttribute( wxT("type"), wxT("0") );
                    attribute.MakeUpper();
                    if ( wxNOT_FOUND  != attribute.Find(_("DYNAMIC") ) ) {
                        type = VSCP_TABLE_DYNAMIC;
                    }
                    else if ( wxNOT_FOUND  != attribute.Find( _("STATIC") ) ) {
                        
                        type = VSCP_TABLE_STATIC;
                        
                        // Get size
                        size = vscp_readStringValue( subchild->GetAttribute( wxT("size"), wxT("0") ) );
                        
                    }
                    else {                       
                        // Invalid type
                        logMsg( _("Reading VSCP table xml info: Invalid table type (static/dynamic)!\n") );
                        goto xml_table_error;
                    }
                    
                    // Should it be created in memory?
                    bool bMemory = false;
                    wxString strInMemory = subchild->GetAttribute( wxT("bmemory"), wxT("false") );
                    strInMemory.MakeUpper();
                    if ( wxNOT_FOUND != strInMemory.Find( _("TRUE") ) ) {
                        bMemory = true;
                    }
                    
                    CVSCPTable *pTable = new CVSCPTable( m_rootFolder + _("table/"), name, true, bMemory, type, size );
                    if ( NULL != pTable ) {
                        
                        if ( !pTable->setTableInfo( subchild->GetAttribute( wxT("owner"), wxT("admin") ),
                                                vscp_readStringValue( subchild->GetAttribute( wxT("rights"), wxT("0x700") ) ),
                                                subchild->GetAttribute( wxT("title"), wxT("") ),
                                                subchild->GetAttribute( wxT("labelx"), wxT("") ),
                                                subchild->GetAttribute( wxT("labely"), wxT("") ),
                                                subchild->GetAttribute( wxT("note"), wxT("") ),
                                                subchild->GetAttribute( wxT("sqlcreate"), wxT("") ),
                                                subchild->GetAttribute( wxT("sqlinsert"), wxT("") ),
                                                subchild->GetAttribute( wxT("sqldelete"), wxT("") ),
                                                subchild->GetAttribute( wxT("description"), wxT("") ) ) ) {
                            logMsg(_("Reading table xml info: Could not set table info!\n"));
                            delete pTable;
                            goto xml_table_error;
                        }
                        
                        pTable->setTableEventInfo( vscp_readStringValue( subchild->GetAttribute( wxT("vscpclass"), wxT("0") ) ),
                                vscp_readStringValue( subchild->GetAttribute( wxT("vscptype"), wxT("0") ) ),
                                vscp_readStringValue( subchild->GetAttribute( wxT("vscpsensorindex"), wxT("0") ) ),
                                vscp_readStringValue( subchild->GetAttribute( wxT("vscpunit"), wxT("0") ) ),
                                vscp_readStringValue( subchild->GetAttribute( wxT("vscpzone"), wxT("255") ) ),
                                vscp_readStringValue( subchild->GetAttribute( wxT("vscpsubzone"), wxT("255") ) )
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
        
        // TCP/IP port
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_TCPIPINTERFACE_ADDRESS ) ) {
            m_strTcpInterfaceAddress = wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, 
                                        VSCPDB_ORDINAL_CONFIG_TCPIPINTERFACE_ADDRESS ) );
            m_strTcpInterfaceAddress.Trim(true);
            m_strTcpInterfaceAddress.Trim(false);
        }
        
        // Port for Multicast interface
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_ANNOUNCEINTERFACE_ADDRESS ) ) {
            m_strMulticastAnnounceAddress = wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, 
                                        VSCPDB_ORDINAL_CONFIG_ANNOUNCEINTERFACE_ADDRESS ) );
        }
        
        // TTL for Multicast i/f
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_ANNOUNCEINTERFACE_TTL ) ) {
            m_ttlMultiCastAnnounce = sqlite3_column_int( ppStmt, 
                                        VSCPDB_ORDINAL_CONFIG_ANNOUNCEINTERFACE_TTL );
        }
        
        gpobj->m_mutexUDPInfo.Lock();
        
        // Enable UDP interface
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_UDP_ENABLE ) ) {
            m_udpInfo.m_bEnable = sqlite3_column_int( ppStmt, 
                                        VSCPDB_ORDINAL_CONFIG_UDP_ENABLE ) ? true : false;
        }
        
        // UDP interface address/port
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_UDP_ADDRESS ) ) {
            m_udpInfo.m_interface = wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, 
                                        VSCPDB_ORDINAL_CONFIG_UDP_ADDRESS ) );
        }
        
        // UDP User
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_UDP_USER ) ) {
            m_udpInfo.m_user = wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, 
                                        VSCPDB_ORDINAL_CONFIG_UDP_USER ) );
        }
        
        // UDP User Password
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_UDP_PASSWORD ) ) {
            m_udpInfo.m_password = wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, 
                                        VSCPDB_ORDINAL_CONFIG_UDP_PASSWORD ) );
        }
        
        // UDP Filter
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_UDP_FILTER ) ) {
            wxString str = wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, 
                                        VSCPDB_ORDINAL_CONFIG_UDP_FILTER ) );
            vscp_readFilterFromString( &m_udpInfo.m_filter, str );
        }
        
        // USP Mask
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_UDP_MASK ) ) {
            wxString str = wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, 
                                        VSCPDB_ORDINAL_CONFIG_UDP_MASK ) );
            vscp_readMaskFromString( &m_udpInfo.m_filter, str );
        }
        
        // UDP GUID
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_UDP_GUID ) ) {
            wxString str = wxString::FromUTF8( (const char *)sqlite3_column_text( ppStmt, 
                                        VSCPDB_ORDINAL_CONFIG_UDP_GUID ) );
            m_udpInfo.m_guid.getFromString( str );
        }
        
        // Enable un-secure
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_UDP_UNSECURE_ENABLE ) ) {
            m_udpInfo.m_bAllowUnsecure = sqlite3_column_int( ppStmt, 
                                        VSCPDB_ORDINAL_CONFIG_UDP_UNSECURE_ENABLE ) ? true : false;
        }
        
        // Enable ack
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_UDP_ACK_ENABLE ) ) {
            m_udpInfo.m_bAck = sqlite3_column_int( ppStmt, 
                                        VSCPDB_ORDINAL_CONFIG_UDP_ACK_ENABLE ) ? true : false;
        }
        
        gpobj->m_mutexUDPInfo.Unlock();
        
        // Enable Multicast interface
        if ( NULL != sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_CONFIG_MULTICASTINTERFACE_ENABLE ) ) {
            m_bMulticast = sqlite3_column_int( ppStmt, 
                                        VSCPDB_ORDINAL_CONFIG_MULTICASTINTERFACE_ENABLE ) ? true : false;
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
// readUdpNodes
//
// Read in defined UDP nodes
//
//

bool CControlObject::readUdpNodes( void )
{
    char *pErrMsg = 0;
    const char *psql = "SELECT * from UDPNODE";
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
        m_udpInfo.m_remotes.Append( pudpClient->m_remoteAddress, pudpClient );
        
        gpobj->m_mutexUDPInfo.Unlock();
        
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
// doCreateUdpNodeTable
//
// Create the UDP node database
//
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
    
    if ( SQLITE_OK  !=  sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg ) ) {
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
    
    if ( SQLITE_OK  !=  sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg ) ) {
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
    const char *psql = VSCPDB_TABLE_CREATE;
    
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



