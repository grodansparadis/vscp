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

#if defined(_WIN32) 
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
#include <mongoose.h>

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
#include <vscpweb.h>
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
WX_DEFINE_LIST(CanalMsgList); 
WX_DEFINE_LIST(VSCPEventList);

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
    

    // Control UDP Interface
    m_udpInfo.m_bEnable = false;
    m_udpInfo.m_interface.Empty();
    m_udpInfo.m_guid.clear();
    vscp_clearVSCPFilter( &m_udpInfo.m_filter );
    m_udpInfo.m_bAllowUnsecure = false;
    m_udpInfo.m_bAck = false;

    // Default TCP/IP interface
    m_enableTcpip = true;
    m_strTcpInterfaceAddress = _("tcp://" + VSCP_DEFAULT_TCP_PORT);

    // Default multicast announce port
    m_strMulticastAnnounceAddress = _( "udp://:" + VSCP_ANNNOUNCE_MULTICAST_PORT );

    // default multicast announce ttl
    m_ttlMultiCastAnnounce = IP_MULTICAST_DEFAULT_TTL;

    // Default UDP interface
    m_udpInfo.m_interface = _("udp://:" + VSCP_DEFAULT_UDP_PORT );

    m_pclientMsgWorkerThread = NULL;
    m_pVSCPClientThread = NULL;
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
    
/*    
wxString ssss(_("12345678"));    
vscp_base64_wxencode( ssss );
vscp_base64_wxdecode( ssss );
ssss = _("1234");
*/
    
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
    
    // Exit weserver interface
    web_exit();
    
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
        
        
        char *zErrMsg = NULL;
                
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
        wxstr += sqlite3_column_text( ppStmt, 4 ); 
        
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
    vscpEventEx e;
    vscpEventEx *pEventEx = &e;
    wxLogDebug( wxString::Format(_("%p crc=%p obid=%p year=%p month=%p day=%p"), (void*)pEventEx, (void *)&(pEventEx->crc), (void*)&(pEventEx->obid), (void*)&(pEventEx->year), (void*)&(pEventEx->month), (void*)&(pEventEx->day) ));   
    wxLogDebug( wxString::Format(_("sizeof unsigned short %zu"), sizeof(unsigned short) ) );
    wxLogDebug( wxString::Format(_("sizeof unsigned uint16_t %zu"), sizeof(uint16_t) ) );
    wxLogDebug( wxString::Format(_("sizeof unsigned long %zu"), sizeof(unsigned long) ) );
    wxLogDebug( wxString::Format(_("sizeof unsigned uint32_t %zu"), sizeof(uint32_t) ) );
    wxLogDebug( wxString::Format(_("sizeof unsigned int %zu"), sizeof(int) ) );
*/    
    /*
    vscp_isPasswordValid( _("E2D453EF99FB3FCD19E67876554A8C27;A4A86F7D7E119BA3F0CD06881E371B989B33B6D606A863B633EF529D64544F8E"), 
            _("secrett") );
        
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
    m_web_document_root = strRootwww;
    
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
        fprintf( stderr, "%s", (const char *)str.mbc_str() );
        return false;
    }
    
    ////////////////////////////////////////////////////////////////////////////
    //                  Read XML configuration GENERAL section
    ////////////////////////////////////////////////////////////////////////////

    /*
    str = _("Using configuration file: ") + strcfgfile + _("\n");
    fprintf( stderr, str.mbc_str() );
    */ 

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
            // Database is open. Read configuration data from it
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
                    fprintf( stderr, "Failed to create subzone table.\n" );
                }
        
                // Create userdef table
                if ( !doCreateUserdefTableTable() ) {
                    fprintf( stderr, "Failed to create userdef table.\n" );
                }
                
                // * * * All created * * *
                
                // Database is open. Read configuration data from it
                if ( !readConfigurationDB() ) {
                    fprintf( stderr, "Failed to read configuration from configuration database.\n" );
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

        if ( SQLITE_OK != sqlite3_open( (const char *)m_path_db_vscp_log.GetFullPath().mbc_str(),
                                            &m_db_vscp_log ) ) {

            // Failed to open/create the database file
            fprintf( stderr, "VSCP Server logging database could not be opened. - Will not be used.\n" );
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
            fprintf( stderr, "VSCP Server logging database does not exist - will be created.\n" );
            str.Printf(_("Path=%s\n"), (const char *)m_path_db_vscp_log.GetFullPath().mbc_str() );
            fprintf( stderr, "%s", (const char *)str.mbc_str() );
            
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
            fprintf( stderr, "%s", (const char *)str.mbc_str() );
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
        fprintf( stderr, "%s", (const char *)str.mbc_str() );
        if ( NULL != m_db_vscp_data ) sqlite3_close( m_db_vscp_data );
        m_db_vscp_data = NULL;

    }
    
    
    ////////////////////////////////////////////////////////////////////////////
    //                      Read XML configuration
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
    
    if ( 0 == m_admin_allowfrom.Length() ) m_admin_allowfrom = _("*");
    m_userList.addSuperUser( m_admin_user,
                            m_admin_password,
                            m_admin_allowfrom );          // Remotes allows to connect                                                     
    
    
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
    m_variables.init();
    
    // Load variables if mechanism is enabled
    logMsg(_("Loading persistent variables from XML variable default path.\n") );
    m_variables.loadFromXML();

    // Start daemon internal client worker thread    
    startClientWorkerThread();

    // Start TCP/IP interface    
    startTcpWorkerThread();
    
    // Start UDP interface
    startUDPWorkerThread();
    
    // Start Multicast interface
    startMulticastWorkerThreads();

    // Start webserver and websockets        
    init_webserver(); 

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
    m_dm.feed( &EventStartUp );


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
            
            // Send events to websocket clients
            websock_post_incomingEvents();

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
    logMsg(_("Giving worker threads time to stop operations..."),DAEMON_LOGMSG_DEBUG);
    sleep( 2 ); // Give threads some time to end
    
    logMsg(_("Stopping VSCP Server worker thread..."),DAEMON_LOGMSG_DEBUG);
    stopDaemonWorkerThread();
    
    logMsg(_("Stopping client worker thread..."),DAEMON_LOGMSG_DEBUG);
    stopClientWorkerThread();
    
    logMsg(_("Stopping device worker thread..."),DAEMON_LOGMSG_DEBUG);
    stopDeviceWorkerThreads();
    
    logMsg(_("Stopping TCP/IP worker thread..."),DAEMON_LOGMSG_DEBUG);
    stopTcpWorkerThread();
    
    logMsg(_("Stopping UDP worker thread..."),DAEMON_LOGMSG_DEBUG);
    stopUDPWorkerThread();
    
    logMsg(_("Stopping Multicast worker threads..."),DAEMON_LOGMSG_DEBUG);
    stopMulticastWorkerThreads();
    
    logMsg(_("Stopping Web Server worker thread..."),DAEMON_LOGMSG_DEBUG);
    // TODO stop web server
   
   
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
                logMsg(_("Multicast start channel table invalid entry.\n") );
                continue;
            }
            
            logMsg(_("Starting multicast channel interface thread...\n") );      
            pChannel->m_pWorkerThread = new VSCPMulticastClientThread;

            if ( NULL != pChannel->m_pWorkerThread) {
                
                // Share the multicast channel item
                pChannel->m_pWorkerThread->m_pChannel = pChannel;
                
                wxThreadError err;
                if (wxTHREAD_NO_ERROR == (err = pChannel->m_pWorkerThread->Create())) {
                    //m_ptcpListenThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY );
                    if (wxTHREAD_NO_ERROR != (err = pChannel->m_pWorkerThread->Run())) {
                        logMsg( _("Unable to run multicast channel thread.") );
                    }
                }
                else {
                    logMsg( _("Unable to create multicast channel thread.") );
                }
            }
            else {
                logMsg( _("Unable to allocate memory for multicast channel thread.") );
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
            logMsg(_("Multicast end channel table invalid entry.\n") );
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
    
    logMsg(_("[Driver] - Starting drivers...\n") );

    VSCPDEVICELIST::iterator iter;
    for ( iter = m_deviceList.m_devItemList.begin();
            iter != m_deviceList.m_devItemList.end();
            ++iter ) {

        pDeviceItem = *iter;
        if ( NULL != pDeviceItem ) {

            logMsg( _("[Driver] - Preparing: ") + pDeviceItem->m_strName + _("\n") );
            
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
    unsigned long val;
    wxXmlDocument doc;

    wxString wxlogmsg = wxString::Format(_("Reading XML GENERAL configuration from [%s]\n"),
                                            (const char *)strcfgfile.c_str() );
    logMsg( wxlogmsg  );

    if (!doc.Load(strcfgfile)) {
        logMsg(_("Can't load logfile. Is path correct?\n")  );
        return false;
    }

    // start processing the XML file
    if (doc.GetRoot()->GetName() != _("vscpconfig")) {
        logMsg(_("Can't read logfile. Maybe it is invalid!\n")  );
        return false;
    }

    wxXmlNode *child = doc.GetRoot()->GetChildren();
    while (child) {

        if (child->GetName() == _("general")) {

            wxXmlNode *subchild = child->GetChildren();
            while ( subchild ) {

                if ( subchild->GetName() == _("security") ) {
                    
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
                else if ( subchild->GetName() == _("loglevel") ) {
                    
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
                else if ( subchild->GetName() == _("runasuser") ) {
                    
                    m_runAsUser = subchild->GetNodeContent();
                    m_runAsUser.Trim();
                    m_runAsUser.Trim(false);
                    // Also assign to web user
                    m_web_run_as_user = m_runAsUser;
                }
                else if ( subchild->GetName() == _("logsyslog") ) {

                    wxString attribute = subchild->GetAttribute(_("enable"), _("true"));
                    attribute.MakeLower();
                    if (attribute.IsSameAs(_("false"), false)) {
                        m_bLogToSysLog = false;
                    }
                    else {
                        m_bLogToSysLog = true;
                    }
                    
                } 
                else if (subchild->GetName() == _("guid")) {
                    
                    wxString str = subchild->GetNodeContent();
                    m_guid.getFromString(str);
                    
                }
                else if ( subchild->GetName() == _( "servername" ) ) {
                    
                    m_strServerName = subchild->GetNodeContent();
                    
                }
                else if (subchild->GetName() == _("clientbuffersize")) {
                    
                    wxString str = subchild->GetNodeContent();
                    m_maxItemsInClientReceiveQueue = vscp_readStringValue(str);
                    
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
    if ( doc.GetRoot()->GetName() != _("vscpconfig") ) {
        logMsg(_("Can't read logfile. Maybe it is invalid!\n")  );
        return false;
    }

    wxXmlNode *child = doc.GetRoot()->GetChildren();
    while ( child ) {

        // The "general" settings are read in a pre-step (readXMLConfigurationGeneral) 
        
        if ( child->GetName() == _("tcpip") ) {
            
            // Enable
            wxString attribute = child->GetAttribute(_("enable"), _("true"));
            attribute.MakeLower();          
            if (attribute.IsSameAs(_("false"), false)) {
                m_enableTcpip = false; 
            }
            else {
                m_enableTcpip  = true; 
            }

            m_strTcpInterfaceAddress = child->GetAttribute(_("interface"), _(""));
            m_strTcpInterfaceAddress.Trim(true);
            m_strTcpInterfaceAddress.Trim(false);

        }
        else if ( child->GetName() == _( "multicast-announce" ) ) {
                    
            // Enable
            wxString attribute = child->GetAttribute(_("enable"), _("true"));
            attribute.MakeLower();          
            if (attribute.IsSameAs(_("false"), false)) {
                m_bMulticastAnounce = false; 
            }
            else {
                m_bMulticastAnounce  = true; 
            }
                    
            // interface
            m_strMulticastAnnounceAddress = child->GetAttribute( _( "interface" ), _( "" ) );

            // ttl
            m_ttlMultiCastAnnounce = vscp_readStringValue( child->GetAttribute( _( "ttl" ), _( "1" ) ) );

        }
        else if (child->GetName() == _("udp")) {
   
            m_mutexUDPInfo.Lock();
            
            // Default is to let everything come through
            vscp_clearVSCPFilter( &m_udpInfo.m_filter );
                    
            // Enable
            wxString attribute = child->GetAttribute(_("enable"), _("true"));
            attribute.MakeLower();          
            if (attribute.IsSameAs(_("false"), false)) {
                m_udpInfo.m_bEnable = false; 
            }
            else {
                m_udpInfo.m_bEnable  = true; 
            }
                    
            // Allow insecure connections
            attribute = child->GetAttribute( _("bAllowUnsecure"), _("true") );
            if (attribute.Lower().IsSameAs(_("false"), false)) {
                m_udpInfo.m_bAllowUnsecure = false; 
            }
            else {
                m_udpInfo.m_bAllowUnsecure  = true; 
            }
                    
            // Enable ACK
            attribute = child->GetAttribute( _("bSendAck"), _("false") );                    
            if (attribute.Lower().IsSameAs(_("false"), false)) {
                m_udpInfo.m_bAck = false; 
            }
            else {
                m_udpInfo.m_bAck = true; 
            }                    
                    
            // Username
            m_udpInfo.m_user = child->GetAttribute( _("user"), _("") );
                        
            // Password
            m_udpInfo.m_password = child->GetAttribute( _("password"), _(""));

            // Interface
            m_udpInfo.m_interface = child->GetAttribute( _("interface"), _("udp://"+VSCP_DEFAULT_UDP_PORT));
                    
            // GUID
            attribute = child->GetAttribute( _("guid"), _("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00") );
            m_udpInfo.m_guid.getFromString( attribute );
                    
            // Filter
            attribute = child->GetAttribute( _("filter"), _("") );
            if ( attribute.Trim().Length() ) {
                vscp_readFilterFromString( &m_udpInfo.m_filter, attribute );
            }
                    
            // Mask
            attribute = child->GetAttribute( _("mask"), _("") );
            if ( attribute.Trim().Length() ) {                    
                vscp_readMaskFromString( &m_udpInfo.m_filter, attribute );
            }
                                    
            wxXmlNode *subchild = child->GetChildren();
            while ( subchild ) {
                        
                if ( subchild->GetName() == _("rxnode") ) {
                        
                    udpRemoteClientInfo *pudpClient = new udpRemoteClientInfo;
                    if ( NULL == pudpClient ) {
                        logMsg( _("Failed to allocated UDP client remote structure.\n") );
                        gpobj->m_mutexUDPInfo.Unlock();
                        subchild = subchild->GetNext(); 
                        continue;
                    }
                    
                    // Default is to let everything come through
                    vscp_clearVSCPFilter( &pudpClient->m_filter );
                                                        
                    attribute = subchild->GetAttribute( _("enable"), _("false") );
                    if ( attribute.Lower().IsSameAs(_("false"), false ) ) {
                        pudpClient->m_bEnable = false; 
                    }
                    else {
                        pudpClient->m_bEnable = true; 
                    }               
                            
                    if ( !pudpClient->m_bEnable ) {
                        delete pudpClient;
                        gpobj->m_mutexUDPInfo.Unlock();
                        subchild = subchild->GetNext(); 
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
                    pudpClient->m_index = 0;
                    m_udpInfo.m_remotes.Append( pudpClient );
                            
                }
                        
                subchild = subchild->GetNext();    
                        
            } // while
                    
            gpobj->m_mutexUDPInfo.Unlock();

        } // udp
        
        else if (child->GetName() == _("multicast")) {
            
            gpobj->m_mutexMulticastInfo.Lock();
                    
            attribute = child->GetAttribute(_("enable"), _("true"));
            attribute.MakeLower();
            if (attribute.IsSameAs(_("false"), false)) {
                m_multicastInfo.m_bEnable = false;
            }
            else {
                m_multicastInfo.m_bEnable = true;
            }
            
            wxXmlNode *subchild = child->GetChildren();
            while ( subchild ) {
                        
                if ( subchild->GetName() == _("channel") ) {
                        
                    multicastChannelItem *pChannel = new multicastChannelItem;
                    if ( NULL == pChannel ) {
                        logMsg( _("Failed to allocated multicast channel structure.\n") );
                        gpobj->m_mutexMulticastInfo.Unlock();
                        subchild = subchild->GetNext(); 
                        continue;
                    }
                    
                    // Default is to let everything come through
                    vscp_clearVSCPFilter( &pChannel->m_txFilter );
                    vscp_clearVSCPFilter( &pChannel->m_rxFilter );
                    
                    // Enable
                    attribute = subchild->GetAttribute(_("enable"), _("true"));
                    attribute.MakeLower();
                    if (attribute.IsSameAs(_("false"), false)) {
                        pChannel->m_bEnable = false;
                    }
                    else {
                        pChannel->m_bEnable = true;
                    }
                    
                    // bSendAck
                    attribute = subchild->GetAttribute(_("bSendAck"), _("false"));
                    attribute.MakeLower();
                    if (attribute.IsSameAs(_("false"), false)) {
                        pChannel->m_bSendAck = false;
                    }
                    else {
                        pChannel->m_bSendAck = true;
                    }
                    
                    // bAllowUndsecure
                    attribute = subchild->GetAttribute( _("bAllowUnsecure"), _("true") );
                    attribute.MakeLower();
                    if ( attribute.IsSameAs(_("false"), false) ) {
                        pChannel->m_bAllowUnsecure = false;
                    }
                    else {
                        pChannel->m_bAllowUnsecure = true;
                    }
                    
                    // Interface
                    pChannel->m_public =
                            subchild->GetAttribute( _("public"), _("") );
                    
                    // Interface
                    pChannel->m_port =
                            vscp_readStringValue( subchild->GetAttribute( _("port"), _("44444") ) );
                    
                    // Group
                    pChannel->m_gropupAddress =
                            subchild->GetAttribute( _("group"), _("udp://224.0.23.158:44444") );
                    
                    // ttl
                    pChannel->m_ttl = 
                            vscp_readStringValue( subchild->GetAttribute( _("ttl"), _("1") ) );
                    
                    // guid
                    attribute = 
                            subchild->GetAttribute( _("guid"), 
                                                    _("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00") );
                    pChannel->m_guid.getFromString( attribute );
                    
                    // TX Filter
                    attribute = subchild->GetAttribute( _("txfilter"), _("") );
                    if ( attribute.Trim().Length() ) {
                        vscp_readFilterFromString( &pChannel->m_txFilter, attribute );
                    }
                    
                    // TX Mask
                    attribute = subchild->GetAttribute( _("txmask"), _("") );
                    if ( attribute.Trim().Length() ) {
                        vscp_readMaskFromString( &pChannel->m_txFilter, attribute );
                    }
                    
                    // RX Filter
                    attribute = subchild->GetAttribute( _("rxfilter"), _("") );
                    if ( attribute.Trim().Length() ) {
                        vscp_readFilterFromString( &pChannel->m_rxFilter, attribute );
                    }
                    
                    // RX Mask
                    attribute = subchild->GetAttribute( _("rxmask"), _("") );
                    if ( attribute.Trim().Length() ) {
                        vscp_readMaskFromString( &pChannel->m_rxFilter, attribute );
                    }
                    
                    // encryption
                    attribute = subchild->GetAttribute( _("encryption"), _("") );
                    pChannel->m_nEncryption = vscp_getEncryptionCodeFromToken( attribute );
                            
                    // add to list
                    pChannel->m_index = 0;
                    m_multicastInfo.m_channels.Append( pChannel );
                    
                }
                
                subchild = subchild->GetNext(); 
                
            } // while       
            
            gpobj->m_mutexMulticastInfo.Unlock();
            
        }
        
        else if (child->GetName() == _("dm")) {
                    
            // Should the internal DM be disabled
            wxString attribut;
                    
            // Get the path to the DM file  (Deprecated)
            attribut = child->GetAttribute( _("path"), _("") );
            if ( attribut.Length() ) {
                m_dm.m_staticXMLPath = attribut;
            }
                    
            // Get the path to the DM file
            attribut = child->GetAttribute( _("pathxml"), _("") );
            if ( attribut.Length() ) {
                m_dm.m_staticXMLPath = attribut;
            }
                    
            // Get the path to the DM db file
            attribut = child->GetAttribute( _("pathdb"), _("") );
            if ( attribut.Length() ) {
                m_dm.m_path_db_vscp_dm.Assign( attribut );
            }
            
            // Get the DM loglevel
            attribut = child->GetAttribute( _("loglevel"), _("") );
            attribute.MakeLower();
            if ( wxNOT_FOUND != attribute.Find( _("debug") ) ) {
                m_debugFlags1 |= VSCP_DEBUG1_DM;
            }
                    
        }
        
        else if (child->GetName() == _("variables")) {
                    
            // Should the internal DM be disabled
            wxFileName fileName;
            wxString attrib;

            // Get the path to the DM file
            attrib = child->GetAttribute(_("path"), _(""));
            fileName.Assign( attrib );
            if ( fileName.IsOk() ) {
                m_variables.m_xmlPath = fileName.GetFullPath();
            }
                    
            attrib = child->GetAttribute(_("pathxml"), m_rootFolder + _("variable.xml"));
            fileName.Assign( attrib );
            if ( fileName.IsOk() ) {
                m_variables.m_xmlPath = fileName.GetFullPath();
            }
                    
            attrib = child->GetAttribute(_("pathdb"), m_rootFolder + _("variable.sqlite3"));
            fileName.Assign( attrib );
            if ( fileName.IsOk() ) {
                m_variables.m_dbFilename = fileName;
            } 
 
        }
        
        else if (child->GetName() == _("webserver")) {

            wxString attribute;
   
            // Enable
            attribute = child->GetAttribute( _("enable"), _("") );
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
            
            attribute = child->GetAttribute(_("document_root"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_document_root = attribute;
            }
            
            attribute = child->GetAttribute( _("listening_ports"), _("") );
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_listening_ports = attribute;
            }

            attribute = child->GetAttribute(_("index_files"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_index_files = attribute;
            }
            
            attribute = child->GetAttribute(_("authentication_domain"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_authentication_domain = attribute;
            }
            
            attribute = child->GetAttribute( _("enable_auth_domain_check"), _("") );
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
            
            attribute = child->GetAttribute(_("ssl_certificat"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_ssl_certificate = attribute;
            }
            
            attribute = child->GetAttribute(_("ssl_certificat_chain"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_ssl_certificate_chain = attribute;
            }
            
            attribute = child->GetAttribute( _("ssl_verify_peer"), _("") );
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

            attribute = child->GetAttribute(_("ssl_ca_path"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_ssl_ca_path = attribute;
            }

            attribute = child->GetAttribute(_("ssl_ca_file"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_ssl_ca_file = attribute;
            }

            attribute = child->GetAttribute(_("ssl_verify_depth"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_ssl_verify_depth = atoi( (const char *)attribute.mbc_str() );
            }
            
            attribute = child->GetAttribute(_("ssl_default_verify_paths"), _(""));
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
            
            attribute = child->GetAttribute(_("ssl_cipher_list"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_ssl_cipher_list = attribute.mbc_str();
            }
            
            attribute = child->GetAttribute(_("ssl_protcol_version"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_ssl_protocol_version = atoi( (const char *)attribute.mbc_str() );
            }
            
            attribute = child->GetAttribute(_("ssl_short_trust"), _(""));
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
            
            attribute = child->GetAttribute( _("cgi_interpreter"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_cgi_interpreter = attribute;                    
            }

            attribute = child->GetAttribute(_("cgi_pattern"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_cgi_patterns =attribute;
            }
            
            attribute = child->GetAttribute(_("cgi_environment"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_cgi_environment = attribute;
            }
            
            attribute = child->GetAttribute(_("protect_uri"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_protect_uri = attribute;
            }
            
            attribute = child->GetAttribute(_("trottle"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_trottle = attribute;
            }
            
            attribute = child->GetAttribute( _("enable_directory_listing"), _("") );
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
 
            attribute = child->GetAttribute( _("enable_keep_alive"), _("") );
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
            
            attribute = child->GetAttribute(_("keep_alive_timeout_ms"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_keep_alive_timeout_ms = atol( (const char *)attribute.mbc_str() );
            }
            
            attribute = child->GetAttribute(_("access_control_list"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_access_control_list = attribute;
            }
            
            attribute = child->GetAttribute(_("extra_mime_types"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_extra_mime_types = attribute;
            }
            
            attribute = child->GetAttribute(_("num_threads"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_num_threads = atoi( (const char *)attribute.mbc_str() );
            }

            attribute = child->GetAttribute(_("hide_file_pattern"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_hide_file_patterns = attribute;
            }

            attribute = child->GetAttribute(_("run_as_user"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            m_web_run_as_user = attribute;
 
            attribute = child->GetAttribute(_("url_rewrite_patterns"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_url_rewrite_patterns = attribute;
            }
            
            attribute = child->GetAttribute(_("hide_file_patterns"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_hide_file_patterns = attribute;
            }
            
            attribute = child->GetAttribute(_("request_timeout_ms"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_request_timeout_ms = atol( (const char *)attribute.mbc_str() );
            }
            
            attribute = child->GetAttribute(_("linger_timeout_ms"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_linger_timeout_ms = atol( (const char *)attribute.mbc_str() );
            }
            
            attribute = child->GetAttribute( _("decode_url"), _("") );
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

            attribute = child->GetAttribute(_("global_auth_file"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_global_auth_file = attribute;
            }
            
            attribute = child->GetAttribute(_("web_per_directory_auth_file"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_per_directory_auth_file = attribute;
            }

            attribute = child->GetAttribute(_("ssi_pattern"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_ssi_patterns = attribute;
            }

            attribute = child->GetAttribute(_("access_control_allow_origin"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_access_control_allow_origin = attribute;
            }
            
            attribute = child->GetAttribute(_("access_control_allow_methods"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_access_control_allow_methods = attribute;
            }
            
            attribute = child->GetAttribute(_("access_control_allow_headers"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_access_control_allow_headers = attribute;
            }

            attribute = child->GetAttribute(_("error_pages"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_error_pages = attribute;
            }
            
            attribute = child->GetAttribute(_("tcp_nodelay"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_linger_timeout_ms = atol( (const char *)attribute.mbc_str() );
            }
            
            attribute = child->GetAttribute(_("static_file_max_age"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_static_file_max_age = atol( (const char *)attribute.mbc_str() );
            }
            
            attribute = child->GetAttribute(_("strict_transport_security_max_age"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_strict_transport_security_max_age = atol( (const char *)attribute.mbc_str() );
            }
            
            attribute = child->GetAttribute( _("sendfile_call"), _("") );
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
            
            attribute = child->GetAttribute(_("additional_headers"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_additional_header = attribute;
            }

            attribute = child->GetAttribute(_("max_request_size"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_max_request_size = atol( (const char *)attribute.mbc_str() );
            }
            
            attribute = child->GetAttribute(_("web_allow_index_script_resource"), _("") );
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
            
            attribute = child->GetAttribute(_("duktape_script_patterns"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_duktape_script_patterns = attribute;
            }
            
            attribute = child->GetAttribute(_("lua_preload_file"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_lua_preload_file = attribute;
            }
            
            attribute = child->GetAttribute(_("lua_script_patterns"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_lua_script_patterns = attribute;
            }
            
            attribute = child->GetAttribute(_("lua_server_page_patterns"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_lua_server_page_patterns = attribute;
            }
            
            attribute = child->GetAttribute(_("lua_websockets_patterns"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_lua_websocket_patterns = attribute;
            }
            
            attribute = child->GetAttribute(_("lua_background_script"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_lua_background_script = attribute;
            }
            
            attribute = child->GetAttribute(_("lua_background_script_params"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_web_lua_background_script_params = attribute;
            }

        }
        
        else if (child->GetName() == _("websockets")) {

            attribute = child->GetAttribute(_("enable"), _("1") );
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
            
            attribute = child->GetAttribute(_("document_root"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_websocket_document_root = attribute;
            }
            
            attribute = child->GetAttribute(_("timeout_ms"), _(""));
            attribute.Trim();
            attribute.Trim(false);
            if ( attribute.Length() ) {
                m_websocket_timeout_ms = atol( (const char *)attribute.mbc_str() );
            }

        }
        
        else if ( child->GetName() == _("remoteuser") ) {

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

                if ( subchild->GetName() == _("user") ) {

                    wxXmlNode *subsubchild = subchild->GetChildren();

                    while (subsubchild) {

                        if (subsubchild->GetName() == _("name")) {
                            name = subsubchild->GetNodeContent();
                            bUser = true;
                        }
                        else if (subsubchild->GetName() == _("password")) {
                            md5 = subsubchild->GetNodeContent();
                        }
                        else if (subsubchild->GetName() == _("privilege")) {
                            privilege = subsubchild->GetNodeContent();
                        }
                        else if (subsubchild->GetName() == _("filter")) {
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
                        else if (subsubchild->GetName() == _("mask")) {

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
                        else if (subsubchild->GetName() == _("allowfrom")) {
                            allowfrom = subsubchild->GetNodeContent();
                        }
                        else if (subsubchild->GetName() == _("allowevent")) {
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
        else if ( ( child->GetName() == _("canaldriver") ) || ( child->GetName() == _("level1driver") ) ) {

            wxXmlNode *subchild = child->GetChildren();
            while (subchild) {

                wxString strName;
                wxString strConfig;
                wxString strPath;
                unsigned long flags;
                cguid guid;
                bool bEnabled = false;
                bool bCanalDriver = false;

                if ( subchild->GetName() == _("driver") ) {

                    wxXmlNode *subsubchild = subchild->GetChildren();

                    wxString attribute = subchild->GetAttribute(_("enable"), _("false"));

                    if ( attribute.IsSameAs( _("true"), false ) ) {
                        bEnabled = true;
                    }

                    while (subsubchild) {

                        if ( subsubchild->GetName() == _("name") ) {
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
                        else if ( subsubchild->GetName() == _("config") ||
                                subsubchild->GetName() == _("parameter") ) {
                            strConfig = subsubchild->GetNodeContent();
                            strConfig.Trim();
                            strConfig.Trim(false);
                        }
                        else if ( subsubchild->GetName() == _("path") ) {
                            strPath = subsubchild->GetNodeContent();
                            strPath.Trim();
                            strPath.Trim(false);
                        }
                        else if ( subsubchild->GetName() == _("flags") ) {
                            wxString str = subsubchild->GetNodeContent();
                            flags = vscp_readStringValue(str);
                        }
                        else if ( subsubchild->GetName() == _("guid") ) {
                            guid.getFromString( subsubchild->GetNodeContent() );
                        }
                        else if ( subsubchild->GetName() == _( "known-nodes" ) ) {

                            wxXmlNode *subsubsubchild = subchild->GetChildren();

                            while ( subsubsubchild ) {

                                if ( subsubsubchild->GetName() == _( "node" ) ) {
                                    cguid knownguid;

                                    knownguid.getFromString( subchild->GetAttribute( _( "guid" ), _( "-" ) ) );
                                    wxString name = subchild->GetAttribute( _( "name" ), _( "" )  ) ;
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
        else if ( ( child->GetName() == _("vscpdriver") ) || ( child->GetName() == _("level2driver") ) ) {

            wxXmlNode *subchild = child->GetChildren();

            while ( subchild ) {

                wxString strName;
                wxString strConfig;
                wxString strPath;
                cguid guid;
                bool bEnabled = false;
                bool bLevel2Driver = false;

                if ( subchild->GetName() == _("driver") ) {

                    wxXmlNode *subsubchild = subchild->GetChildren();

                    wxString attribute = subchild->GetAttribute(_("enable"), _("false"));

                    if (attribute.IsSameAs(_("true"), false)) {
                        bEnabled = true;
                    }

                    while ( subsubchild ) {
                        
                        if (subsubchild->GetName() == _("name")) {
                            
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
                        else if (subsubchild->GetName() == _("config") ||
                                subsubchild->GetName() == _("parameter")) {
                            
                            strConfig = subsubchild->GetNodeContent();
                            strConfig.Trim();
                            strConfig.Trim(false);
                            
                        }
                        else if (subsubchild->GetName() == _("path")) {
                            
                            strPath = subsubchild->GetNodeContent();
                            strPath.Trim();
                            strPath.Trim(false);
                            
                        }
                        else if (subsubchild->GetName() == _("guid")) {
                            guid.getFromString( subsubchild->GetNodeContent() );
                        }
                        else if ( subsubchild->GetName() == _( "known-nodes" ) ) {

                            wxXmlNode *subsubsubchild = subchild->GetChildren();

                            while ( subsubsubchild ) {

                                if ( subsubsubchild->GetName() == _( "node" ) ) {
                                    cguid knownguid;

                                    knownguid.getFromString( subchild->GetAttribute( _( "guid" ), _( "-" ) ) );
                                    wxString name = subchild->GetAttribute( _( "name" ), _( "" ) );
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
        else if ( child->GetName() == _("tables") ) {

            wxXmlNode *subchild = child->GetChildren();
            while ( subchild ) {

                if ( subchild->GetName() == _("table") ) {
                    
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
        else if ( child->GetName() == _("automation") ) {

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

                if (subchild->GetName() == _("zone")) {
                    
                    long zone;
                    wxString strZone = subchild->GetNodeContent();
                    strZone.ToLong( &zone );
                    m_automation.setZone( zone );
            
                }
                else if (subchild->GetName() == _("sub-zone")) {
                    
                    long subzone;
                    wxString strSubZone = subchild->GetNodeContent();
                    strSubZone.ToLong( &subzone );
                    m_automation.setSubzone( subzone );
                    
                }
                else if (subchild->GetName() == _("longitude")) {
                    
                    wxString strLongitude = subchild->GetNodeContent();
                    double d;
                    strLongitude.ToDouble( &d );
                    m_automation.setLongitude( d );

                }
                else if (subchild->GetName() == _("latitude")) {
                    
                    wxString strLatitude = subchild->GetNodeContent();
                    double d;
                    strLatitude.ToDouble( &d );
                    m_automation.setLatitude( d );
                    
                }
                else if (subchild->GetName() == _("sunrise")) {
                    
                    m_automation.enableSunRiseEvent();
                    wxString attribute = subchild->GetAttribute(_("enable"), _("true"));
                    if (attribute.IsSameAs(_("false"), false)) {
                        m_automation.disableSunRiseEvent();
                    }

                    
                }
                
                else if (subchild->GetName() == _("sunrise-twilight")) {
                    
                    m_automation.enableSunRiseTwilightEvent();
                    wxString attribute = subchild->GetAttribute(_("enable"), _("true"));
                    if (attribute.IsSameAs(_("false"), false)) {
                        m_automation.disableSunRiseTwilightEvent();
                    }

                    
                }
                else if (subchild->GetName() == _("sunset")) {
                    
                    m_automation.enableSunSetEvent();
                    wxString attribute = subchild->GetAttribute(_("enable"), _("true"));
                    if (attribute.IsSameAs(_("false"), false)) {
                        m_automation.disableSunSetEvent();
                    }
                    
                }
                else if (subchild->GetName() == _("sunset-twilight")) {
                    
                    m_automation.enableSunSetTwilightEvent();
                    wxString attribute = subchild->GetAttribute(_("enable"), _("true"));
                    if (attribute.IsSameAs(_("false"), false)) {
                        m_automation.disableSunSetTwilightEvent();
                    }
                    
                }
                else if (subchild->GetName() == _("segmentcontrol-event")) {
                    
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
                else if (subchild->GetName() == _("heartbeat-event")) {
                    
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
                else if (subchild->GetName() == _("capabilities-event")) {
                    
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
}

///////////////////////////////////////////////////////////////////////////////
// addConfigurationValueToDatabase
//

bool CControlObject::addConfigurationValueToDatabase( const char *pName, 
                                                        const char *pValue )
{
    char *pErrMsg = 0;
    char *psql;
    
    fprintf( stderr, "Add %s = %s to configuration database\n", pName, pValue );
    
    // Check if database is open
    if ( NULL == m_db_vscp_daemon ) return false;
    
    m_db_vscp_configMutex.Lock();
    
    // Create settings db
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
    if ( SQLITE_OK  !=  sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg ) ) {
        fprintf( stderr, 
                    "Creation of the VSCP settings database failed with message %s", 
                    pErrMsg );
        return false;
    }
    
    // Create name index
    psql = VSCPDB_CONFIG_CREATE_INDEX;
    if ( SQLITE_OK  !=  sqlite3_exec(m_db_vscp_daemon, psql, NULL, NULL, &pErrMsg ) ) { 
        m_db_vscp_configMutex.Unlock();
        fprintf( stderr, 
                    "Creation of the VSCP settings index failed with message %s", 
                    pErrMsg );
        return false;
    }
    
    fprintf( stderr, "Writing default configuration database content..\n" );

    // Add default settings (set as defaults in SQL create expression))
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_DBVERSION, VSCPDB_CONFIG_DEFAULT_DBVERSION );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_CLIENTBUFFERSIZE, VSCPDB_CONFIG_DEFAULT_CLIENTBUFFERSIZE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_GUID, VSCPDB_CONFIG_DEFAULT_GUID );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_SERVERNAME, VSCPDB_CONFIG_DEFAULT_SERVERNAME );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_PATH_LOGDB, VSCPDB_CONFIG_DEFAULT_PATH_LOGDB );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_TCPIP_ADDR, VSCPDB_CONFIG_DEFAULT_TCPIP_ADDR );
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
    
    // DM
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_DM_PATH_DB, VSCPDB_CONFIG_DEFAULT_DM_PATH_DB );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_DM_PATH_XML, VSCPDB_CONFIG_DEFAULT_DM_PATH_XML );
    
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
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_SSL_CERTIFICAT, VSCPDB_CONFIG_DEFAULT_WEB_SSL_CERTIFICAT );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_SSL_CERTIFICAT_CHAIN, VSCPDB_CONFIG_DEFAULT_WEB_SSL_CERTIFICAT_CHAIN );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_SSL_VERIFY_PEER, VSCPDB_CONFIG_DEFAULT_WEB_SSL_VERIFY_PEER );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_SSL_CA_PATH, VSCPDB_CONFIG_DEFAULT_WEB_SSL_CA_PATH );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_SSL_CA_FILE, VSCPDB_CONFIG_DEFAULT_WEB_SSL_CA_FILE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_SSL_VERIFY_DEPTH, VSCPDB_CONFIG_DEFAULT_WEB_SSL_VERIFY_DEPTH );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_SSL_DEFAULT_VERIFY_PATHS, VSCPDB_CONFIG_DEFAULT_WEB_SSL_DEFAULT_VERIFY_PATHS );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_SSL_CHIPHER_LIST, VSCPDB_CONFIG_DEFAULT_WEB_SSL_CHIPHER_LIST );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_SSL_PROTOCOL_VERSION, VSCPDB_CONFIG_DEFAULT_WEB_SSL_PROTOCOL_VERSION );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_SSL_SHORT_TRUST, VSCPDB_CONFIG_DEFAULT_WEB_SSL_SHORT_TRUST );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_CGI_PATTERNS, VSCPDB_CONFIG_DEFAULT_WEB_CGI_PATTERNS );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_CGI_INTERPRETER, VSCPDB_CONFIG_DEFAULT_WEB_CGI_INTERPRETER );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_CGI_ENVIRONMENT, VSCPDB_CONFIG_DEFAULT_WEB_CGI_ENVIRONMENT );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_PROTECT_URI, VSCPDB_CONFIG_DEFAULT_WEB_PROTECT_URI );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_TROTTLE, VSCPDB_CONFIG_DEFAULT_WEB_TROTTLE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_ENABLE_DIRECTORY_LISTING, VSCPDB_CONFIG_DEFAULT_WEB_ENABLE_DIRECTORY_LISTING );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_ENABLE_KEEP_ALIVE, VSCPDB_CONFIG_DEFAULT_WEB_ENABLE_KEEP_ALIVE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_LIST, VSCPDB_CONFIG_DEFAULT_WEB_ACCESS_CONTROL_LIST );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_MIME_TYPES, VSCPDB_CONFIG_DEFAULT_WEB_MIME_TYPES );
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
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_SENDFILE_CALL, VSCPDB_CONFIG_DEFAULT_WEB_SENDFILE_CALL );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_ADDITIONAL_HEADERS, VSCPDB_CONFIG_DEFAULT_WEB_ADDITIONAL_HEADERS );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_MAX_REQUEST_SIZE, VSCPDB_CONFIG_DEFAULT_WEB_MAX_REQUEST_SIZE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_ALLOW_INDEX_SCRIPT_RESOURCE, VSCPDB_CONFIG_DEFAULT_WEB_ALLOW_INDEX_SCRIPT_RESOURCE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_DUKTAPE_SCRIPT_PATTERNS, VSCPDB_CONFIG_DEFAULT_WEB_DUKTAPE_SCRIPT_PATTERNS );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_LUA_PRELOAD_FILE, VSCPDB_CONFIG_DEFAULT_WEB_LUA_PRELOAD_FILE );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_LUA_SCRIPT_PATTERNS, VSCPDB_CONFIG_DEFAULT_WEB_LUA_SCRIPT_PATTERNS );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_LUA_SERVER_PAGE_PATTERNS, VSCPDB_CONFIG_DEFAULT_WEB_LUA_SERVER_PAGE_PATTERNS );
    addConfigurationValueToDatabase( VSCPDB_CONFIG_NAME_WEB_LUA_WEBSOCKET_PATTERNS, VSCPDB_CONFIG_DEFAULT_WEB_LUA_WEBSOCKET_PATTERNS );
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
        if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_DBVERSION ) ) {  
            dbVersion = atoi( (const char * )pValue );       
        }
        // client buffer size
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_CLIENTBUFFERSIZE ) ) {
            m_maxItemsInClientReceiveQueue = atol( (const char * )pValue );            
        }  
        // Server GUID
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_GUID ) ) {
            m_guid.getFromString( (const char * )pValue );
        }
        // Server name
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_SERVERNAME ) ) {
            m_strServerName = wxString::FromUTF8( (const char * )pValue );
        }          
        // Path to log db
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_PATH_LOGDB ) ) {
            m_path_db_vscp_log.Assign( wxString::FromUTF8( (const char * )pValue ) );
        }              
        // TCP/IP interface address
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_TCPIP_ADDR ) ) {
            m_strTcpInterfaceAddress = wxString::FromUTF8( (const char *)pValue );
            m_strTcpInterfaceAddress.Trim(true);
            m_strTcpInterfaceAddress.Trim(false);
        }        
        // Announce multicast interface address
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_ANNOUNCE_ADDR )  ) {
            m_strMulticastAnnounceAddress = wxString::FromUTF8( (const char *)pValue );
        }        
        // TTL for the multicast i/f
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_ANNOUNCE_TTL )  ) {
            m_ttlMultiCastAnnounce = atoi( (const char *)pValue );
        }        
        // Enable UDP interface
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_UDP_ENABLE )  ) {
            gpobj->m_mutexUDPInfo.Lock();
            m_udpInfo.m_bEnable = atoi( (const char *)pValue ) ? true : false;
            gpobj->m_mutexUDPInfo.Unlock();
        }
        // UDP interface address/port
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_UDP_ADDR )  ) {
            gpobj->m_mutexUDPInfo.Lock();
            m_udpInfo.m_interface = wxString::FromUTF8( (const char *)pValue );
            gpobj->m_mutexUDPInfo.Unlock();
        }      
        // UDP User
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_UDP_USER )  ) {
            gpobj->m_mutexUDPInfo.Lock();
            m_udpInfo.m_user = wxString::FromUTF8( (const char *)pValue );
            gpobj->m_mutexUDPInfo.Unlock();
        }        
        // UDP User Password
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_UDP_PASSWORD )  ) {
            gpobj->m_mutexUDPInfo.Lock();
            m_udpInfo.m_password = wxString::FromUTF8( (const char *)pValue );
            gpobj->m_mutexUDPInfo.Unlock();
        }
        // UDP un-secure enable
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_UDP_UNSECURE_ENABLE )  ) {
            gpobj->m_mutexUDPInfo.Lock();
            m_udpInfo.m_bAllowUnsecure = atoi( (const char *)pValue ) ? true : false;
            gpobj->m_mutexUDPInfo.Unlock();
        }
        // UDP Filter
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_UDP_FILTER )  ) {
            gpobj->m_mutexUDPInfo.Lock();
            vscp_readFilterFromString( &m_udpInfo.m_filter, wxString::FromUTF8( (const char *)pValue ) );
            gpobj->m_mutexUDPInfo.Unlock();
        }
        // UDP Mask
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_UDP_MASK )  ) {
            gpobj->m_mutexUDPInfo.Lock();
            vscp_readMaskFromString( &m_udpInfo.m_filter, wxString::FromUTF8( (const char *)pValue ) );
            gpobj->m_mutexUDPInfo.Unlock();
        }
        // UDP GUID
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_UDP_GUID )  ) {
            gpobj->m_mutexUDPInfo.Lock();
            m_udpInfo.m_guid.getFromString( (const char *)pValue );
            gpobj->m_mutexUDPInfo.Unlock();
        } 
        // UDP Enable ACK
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_UDP_ACK_ENABLE )  ) {
            gpobj->m_mutexUDPInfo.Lock();
            m_udpInfo.m_bAck = atoi( (const char *)pValue ) ? true : false;
            gpobj->m_mutexUDPInfo.Unlock();
        }      
        // Enable Multicast interface
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_MULTICAST_ENABLE )  ) {
            m_multicastInfo.m_bEnable = atoi( (const char *)pValue ) ? true : false;
        }
        // Path to DM database file
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_DM_PATH_DB )  ) {
            m_dm.m_path_db_vscp_dm.Assign( wxString::FromUTF8( (const char *)pValue ) );
        }        
        // Path to DM XML file
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_DM_PATH_XML )  ) {
            m_dm.m_staticXMLPath = wxString::FromUTF8( (const char *)pValue );
        } 
        // Path to variable database
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_VARIABLES_PATH_DB )  ) {
            m_variables.m_dbFilename.Assign( wxString::FromUTF8( (const char *)pValue ) );
        } 
        // Path to variable XML
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_VARIABLES_PATH_XML )  ) {
            m_variables.m_xmlPath = wxString::FromUTF8( (const char *)pValue );
        }
        // VSCP data database path
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_PATH_DB_DATA )  ) {
            m_path_db_vscp_data.Assign( wxString::FromUTF8( (const char *)pValue ) );
        }
        
        
        // * * * WEB server * * *
        
        
        // Web server enable
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_ENABLE )  ) {
            if ( atoi( (const char *)pValue ) ) {
                m_web_bEnable = true;
            }
            else {
                m_web_bEnable = false;
            }
        }
        
        // Web server document root
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_DOCUMENT_ROOT )  ) {
            m_web_document_root = wxString::FromUTF8( (const char *)pValue );
        }        
        
        // listening ports for web server
        else if ( !vscp_strcasecmp( (const char * )pName, VSCPDB_CONFIG_NAME_WEB_LISTENING_PORTS )  ) {
            m_web_listening_ports = wxString::FromUTF8( (const char *)pValue );
        }
        
        // Index files
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_INDEX_FILES )  ) {
            m_web_index_files = wxString::FromUTF8( (const char *)pValue );
        }
        
        // Authdomain
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_AUTHENTICATION_DOMAIN )  ) {
            m_web_authentication_domain = wxString::FromUTF8( (const char *)pValue );
        }
        
        // Enable authdomain check
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_ENABLE_AUTH_DOMAIN_CHECK )  ) {
            if ( atoi( (const char *)pValue ) ) {
                m_enable_auth_domain_check = true;
            }
            else {
                m_enable_auth_domain_check = false;
            }
        }
        
        // Path to cert file
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_CGI_PATTERNS )  ) {
            m_web_ssl_certificate = wxString::FromUTF8( (const char *)pValue );
        }
        
        // SSL certificate chain
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_SSL_CERTIFICAT_CHAIN )  ) {
            m_web_ssl_certificate_chain = wxString::FromUTF8( (const char *)pValue );
        }
        
        // SSL verify peer
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_SSL_VERIFY_PEER )  ) {
            if ( atoi( (const char *)pValue ) ) {
                m_web_ssl_verify_peer = true;
            }
            else {
                m_web_ssl_verify_peer = false;
            }
        } 
        
        // SSL CA path
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_SSL_CA_FILE )  ) {
            m_web_ssl_ca_path = wxString::FromUTF8( (const char *)pValue );
        }
        
        // SSL CA file
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_SSL_CA_FILE )  ) {
            m_web_ssl_ca_file = wxString::FromUTF8( (const char *)pValue );
        }
        
        // SSL verify depth
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_SSL_VERIFY_DEPTH )  ) {
            m_web_ssl_verify_depth = atoi( (const char *)pValue );
        }
        
        // SSL default verify path
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_SSL_DEFAULT_VERIFY_PATHS )  ) {
            if ( atoi( (const char *)pValue ) ) {
                m_web_ssl_default_verify_paths = true;
            }
            else {
                m_web_ssl_default_verify_paths = false;
            }
        }
        
        // SSL chipher list
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_SSL_CHIPHER_LIST )  ) {
            m_web_ssl_cipher_list = wxString::FromUTF8( (const char *)pValue );
        }
        
        // SSL protocol version
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_SSL_PROTOCOL_VERSION )  ) {
            m_web_ssl_protocol_version = atoi( (const char *)pValue );
        }
         
        // SSL short trust
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_SSL_SHORT_TRUST )  ) {
            if ( atoi( (const char *)pValue ) ) {
                m_web_ssl_short_trust = true;
            }
            else {
                m_web_ssl_short_trust = false;
            }
        } 
        
        // CGI interpreter
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_CGI_INTERPRETER )  ) {
            m_web_cgi_interpreter = wxString::FromUTF8( (const char *)pValue );
        }     
        
        // CGI pattern
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_CGI_PATTERNS )  ) {
            m_web_cgi_patterns = wxString::FromUTF8( (const char *)pValue );
        } 
        
        // CGI environment
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_CGI_ENVIRONMENT )  ) {
            m_web_cgi_environment = wxString::FromUTF8( (const char *)pValue );
        }
        
        // Protect URI
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_PROTECT_URI )  ) {
            m_web_protect_uri = wxString::FromUTF8( (const char *)pValue );
        }
        
        // Web trottle
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_TROTTLE )  ) {
            m_web_trottle = wxString::FromUTF8( (const char *)pValue );
        }
        
        // Enable directory listings
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_ENABLE_DIRECTORY_LISTING )  ) {
            if ( atoi( (const char *)pValue ) ) {
                m_web_enable_directory_listing = true;
            }
            else {
                m_web_enable_directory_listing = false;
            }
        }
        
        // Enable keep alive
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_ENABLE_KEEP_ALIVE )  ) {
            if ( atoi( (const char *)pValue ) ) {
                m_web_enable_keep_alive = true;
            }
            else {
                m_web_enable_keep_alive = false;
            }
        }
        
        // Keep alive timout ms
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_KEEP_ALIVE_TIMEOUT_MS )  ) {
            m_web_keep_alive_timeout_ms = atol( (const char *)pValue );
        }
        
        // IP ACL
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_LIST )  ) {
            m_web_access_control_list = wxString::FromUTF8( (const char *)pValue );
        } 
        
        // Extra mime types
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_MIME_TYPES )  ) {
            m_web_extra_mime_types = wxString::FromUTF8( (const char *)pValue );
        } 
        
        // Number of threads
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_NUM_THREADS )  ) {
            m_web_num_threads = atoi( (const char *)pValue );
        }
        
        // Hide file patterns
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_HIDE_FILE_PATTERNS )  ) {
            m_web_hide_file_patterns = wxString::FromUTF8( (const char *)pValue );
        }
        
        // Run as user
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_RUN_AS_USER )  ) {
            m_web_run_as_user = wxString::FromUTF8( (const char *)pValue );
        } 
               
        // URL rewrites
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_URL_REWRITE_PATTERNS )  ) {
            m_web_url_rewrite_patterns = wxString::FromUTF8( (const char *)pValue );
        }  
        
        // Hide file patterns
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_HIDE_FILE_PATTERNS )  ) {
            m_web_hide_file_patterns = wxString::FromUTF8( (const char *)pValue );
        }
        
        // web request timout
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_REQUEST_TIMEOUT_MS )  ) {
            m_web_request_timeout_ms = atol( (const char *)pValue );
        } 
        
        // web linger timout
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_LINGER_TIMEOUT_MS )  ) {
            m_web_linger_timeout_ms = atol( (const char *)pValue );
        } 
        
        // Decode URL
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_DECODE_URL )  ) {
            if ( atoi( (const char *)pValue ) ) {
                m_web_decode_url = true;
            }
            else {
                m_web_decode_url = false;
            }
        }
        
        // Global auth. file
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_GLOBAL_AUTHFILE )  ) {
            m_web_global_auth_file = wxString::FromUTF8( (const char *)pValue );
        }
        
        // Per directory auth. file
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_PER_DIRECTORY_AUTH_FILE )  ) {
            m_web_per_directory_auth_file = wxString::FromUTF8( (const char *)pValue );
        }
        
        // SSI patterns
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_SSI_PATTERNS )  ) {
            m_web_ssi_patterns = wxString::FromUTF8( (const char *)pValue );
        } 
        
        // Access control allow origin
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_ALLOW_ORIGIN )  ) {
            m_web_access_control_allow_origin = wxString::FromUTF8( (const char *)pValue );
        } 
        
        // Access control allow methods
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_ALLOW_METHODS )  ) {
            m_web_access_control_allow_methods = wxString::FromUTF8( (const char *)pValue );
        } 
        
        // Access control alow heraders
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_ACCESS_CONTROL_ALLOW_HEADERS )  ) {
            m_web_access_control_allow_headers = wxString::FromUTF8( (const char *)pValue );
        } 
        
        // Error pages
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_ERROR_PAGES )  ) {
            m_web_error_pages = wxString::FromUTF8( (const char *)pValue );
        }
        
        // TCP no delay
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_TCP_NO_DELAY )  ) {
            m_web_tcp_nodelay = atol( (const char *)pValue );
        }
        
        // File max age
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_STATIC_FILE_MAX_AGE )  ) {
            m_web_static_file_max_age = atol( (const char *)pValue );
        }
        
        // Transport security max age
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_STRICT_TRANSPORT_SECURITY_MAX_AGE )  ) {
            m_web_strict_transport_security_max_age = atol( (const char *)pValue );
        }
        
        // Enable sendfile call
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_SENDFILE_CALL )  ) {
            if ( atoi( (const char *)pValue ) ) {
                m_web_allow_sendfile_call = true;
            }
            else {
                m_web_allow_sendfile_call = false;
            }
        }
        
        // Additional headers
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_ADDITIONAL_HEADERS )  ) {
            m_web_additional_header = wxString::FromUTF8( (const char *)pValue );
        }
        
        // Max request size
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_MAX_REQUEST_SIZE )  ) {
            m_web_max_request_size = atol( (const char *)pValue );
        }
        
        // Allow index script resource
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_ALLOW_INDEX_SCRIPT_RESOURCE )  ) {
            if ( atoi( (const char *)pValue ) ) {
                m_web_allow_index_script_resource = true;
            }
            else {
                m_web_allow_index_script_resource = false;
            }
        }
        
        // Duktape script patterns
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_DUKTAPE_SCRIPT_PATTERNS )  ) {
            m_web_duktape_script_patterns = wxString::FromUTF8( (const char *)pValue );
        }
        
        // Lua preload file
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_LUA_PRELOAD_FILE )  ) {
            m_web_lua_preload_file = wxString::FromUTF8( (const char *)pValue );
        }
        
        // Lua script patterns
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_LUA_SCRIPT_PATTERNS )  ) {
            m_web_lua_script_patterns = wxString::FromUTF8( (const char *)pValue );
        }
        
        // Lua server page patterns
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_LUA_SERVER_PAGE_PATTERNS )  ) {
            m_web_lua_server_page_patterns = wxString::FromUTF8( (const char *)pValue );
        }
        
        // Lua websocket patterns
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_LUA_WEBSOCKET_PATTERNS )  ) {
            m_web_lua_websocket_patterns = wxString::FromUTF8( (const char *)pValue );
        }
        
        // Lua background script
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_LUA_BACKGROUND_SCRIPT )  ) {
            m_web_lua_background_script = wxString::FromUTF8( (const char *)pValue );
        }
        
        // Lua background script params
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEB_LUA_BACKGROUND_SCRIPT_PARAMS )  ) {
            m_web_lua_background_script_params = wxString::FromUTF8( (const char *)pValue );
        }
        
        
        // * * * Websockets * * *
        
        
        // Web server enable
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEBSOCKET_ENABLE )  ) {
            if ( atoi( (const char *)pValue ) ) {
                m_bWebsocketsEnable = true;
            }
            else {
                m_bWebsocketsEnable = false;
            }
        }
        
        // Document root for websockets
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEBSOCKET_DOCUMENT_ROOT )  ) {
            m_websocket_document_root = wxString::FromUTF8( (const char *)pValue );
        }
        
        // Websocket timeout
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_WEBSOCKET_TIMEOUT_MS )  ) {
            m_websocket_timeout_ms = atol( (const char *)pValue );
        }
        
        
        // * * * Automation * * *
                     
        
        // Enable automation
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_AUTOMATION_ENABLE ) ) {
         
            if ( atoi( (const char *)pValue ) ) {
                m_automation.enableAutomation();
            }
            else {
                m_automation.enableAutomation();
            }
                    
        }        
        // Automation zone
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_AUTOMATION_ZONE ) ) {
            m_automation.setZone( atoi( (const char *)pValue ) );
        } 
        // Automation sub zone
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_AUTOMATION_SUBZONE ) ) {
            m_automation.setSubzone( atoi( (const char *)pValue ) );
        }        
        // Automation longitude
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_AUTOMATION_LONGITUDE ) ) {
            m_automation.setLongitude( atof( (const char *)pValue ) );
        }        
        // Automation latitude
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_AUTOMATION_LATITUDE ) ) {
            m_automation.setLatitude( atof( (const char *)pValue ) );
        }      
        // Automation enable sun rise event
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_AUTOMATION_SUNRISE_ENABLE ) ) {
            if ( atoi( (const char *)pValue ) ) {
                m_automation.enableSunRiseEvent();
            }
            else {
                m_automation.disableSunRiseEvent();
            }
        }      
        // Automation enable sun set event
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_AUTOMATION_SUNSET_ENABLE ) ) {
            if ( atoi( (const char *)pValue ) ) {
                m_automation.enableSunSetEvent();
            }
            else {
                m_automation.disableSunSetEvent();
            }
        }        
        // Automation enable sunset twilight event
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_AUTOMATION_SUNSET_TWILIGHT_ENABLE ) ) {
            if ( atoi( (const char *)pValue ) ) {
                m_automation.enableSunSetTwilightEvent();
            }
            else {
                m_automation.disableSunSetTwilightEvent();
            }
        }        
        // Automation enable sunrise twilight event
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_AUTOMATION_SUNRISE_TWILIGHT_ENABLE ) ) {
            if ( atoi( (const char *)pValue ) ) {
                m_automation.enableSunRiseTwilightEvent();
            }
            else {
                m_automation.disableSunRiseTwilightEvent();
            }
        }        
        // Automation segment controller event enable
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_AUTOMATION_SEGMENT_CTRL_ENABLE ) ) {
            if ( atoi( (const char *)pValue ) ) {
                m_automation.enableSegmentControllerHeartbeat();
            }
            else {
                m_automation.disableSegmentControllerHeartbeat();
            }
        }        
        // Automation, segment controller heartbeat interval
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_AUTOMATION_SEGMENT_CTRL_INTERVAL ) ) {
            m_automation.setSegmentControllerHeartbeatInterval( atol( (const char *)pValue ) );
        }
        
        // Automation heartbeat event enable
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_AUTOMATION_HEARTBEAT_ENABLE ) ) {
            if ( atoi( (const char *)pValue ) ) {
                m_automation.enableHeartbeatEvent();
            }
            else {
                m_automation.disableHeartbeatEvent();
            }
        }
        // Automation heartbeat interval
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_AUTOMATION_HEARTBEAT_INTERVAL ) ) {
            m_automation.setHeartbeatEventInterval( atol( (const char *)pValue ) );
        }        
        // Automation capabilities event enable
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_AUTOMATION_CAPABILITIES_ENABLE ) ) {
            if ( atoi( (const char *)pValue ) ) {
                m_automation.enableCapabilitiesEvent();
            }
            else {
                m_automation.disableCapabilitiesEvent();
            }
        }        
        // Automation capabilities interval
        else if ( !vscp_strcasecmp( (const char * )pName, 
                        VSCPDB_CONFIG_NAME_AUTOMATION_CAPABILITIES_INTERVAL ) ) {
            m_automation.setCapabilitiesEventInterval( atol( (const char *)pValue ) );
        }        
        else {
            // Unkown configuration value
        }
    }
    
    sqlite3_finalize( ppStmt );

    return true;
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
                                    (const char *)strName.mbc_str(),
                                    (const char *)strValue.mbc_str(),
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
        pChannel->m_ttl = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_MULTICAST_TTL );
        
        // bAck
        pChannel->m_bSendAck = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_MULTICAST_SENDACK ) ? true : false;
        
        // Allow unsecure
        pChannel->m_bAllowUnsecure = sqlite3_column_int( ppStmt, VSCPDB_ORDINAL_MULTICAST_ALLOW_UNSECURE ) ? true : false;
        
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
                fprintf( stderr, "readMulticastChannels: Failed to set TX filter for multicast channel." );
            }
        }
        
        // TX Mask
        p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_MULTICAST_TXMASK );
        if ( NULL != p ) {
            wxString wxstr = wxString::FromUTF8Unchecked( (const char *)p );
            if ( !vscp_readMaskFromString( &pChannel->m_txFilter, wxstr ) ) {
                fprintf( stderr, "readMulticastChannels: Failed to set TX mask for multicast channel." );
            }
        }
        
        //  RX Filter
        p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_MULTICAST_RXFILTER );
        if ( NULL != p ) {
            wxString wxstr = wxString::FromUTF8Unchecked( (const char *)p );
            if ( !vscp_readFilterFromString( &pChannel->m_rxFilter, wxstr ) ) {
                fprintf( stderr, "readMulticastChannels: Failed to set RX filter for multicast channel." );
            }
        }
        
        // RX Mask
        p = sqlite3_column_text( ppStmt, VSCPDB_ORDINAL_MULTICAST_RXMASK );
        if ( NULL != p ) {
            wxString wxstr = wxString::FromUTF8Unchecked( (const char *)p );
            if ( !vscp_readMaskFromString( &pChannel->m_rxFilter, wxstr ) ) {
                fprintf( stderr, "readMulticastChannels: Failed to set RX mask for multicast channel." );
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
    wxFileName::Mkdir( m_rootFolder + _("/web/css"), 0x777, wxPATH_MKDIR_FULL );
    wxFileName::Mkdir( m_rootFolder + _("/web/images"), 0x777, wxPATH_MKDIR_FULL );
    wxFileName::Mkdir( m_rootFolder + _("/web/lib"), 0x777, wxPATH_MKDIR_FULL );
    wxFileName::Mkdir( m_rootFolder + _("/web/testws"), 0x777, wxPATH_MKDIR_FULL );
    wxFileName::Mkdir( m_rootFolder + _("/web/service"), 0x777, wxPATH_MKDIR_FULL );
    wxFileName::Mkdir( m_rootFolder + _("/tables"), 0x777, wxPATH_MKDIR_FULL );
    
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





