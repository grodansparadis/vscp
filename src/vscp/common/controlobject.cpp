// ControlObject.cpp: implementation of the CControlObject class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (http://www.vscp.org)
//
// Copyright (C) 2000-2014 Ake Hedman, 
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


#include "wx/defs.h"
#include "wx/app.h"
#include <wx/xml/xml.h>

#ifdef WIN32
#include <winsock2.h>
//#include <winsock.h>
#include "canal_win32_ipc.h"

#else 	// UNIX

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

#include "web_css.h"
#include "web_js.h"
#include "web_template.h"

#ifdef WIN32
#include "../../common/net_skeleton.h"
#include "../../common/mongoose.h"
#endif

#include "canal_macro.h"
#include "../common/vscp.h"
#include "../common/vscphelper.h"
#include "../common/vscpeventhelper.h"
#include "../../common/configfile.h"
#include "../../common/crc.h"
#include "../../common/md5.h"
#include "../../common/randpassword.h"
#include "../common/version.h"
#include "variablecodes.h"
#include "actioncodes.h"
#include "devicelist.h"
#include "devicethread.h"
#include "dm.h"
#include "vscpeventhelper.h"
#include "controlobject.h"
#include "../common/webserver.h"
#ifndef WIN32
//#include <microhttpd.h>
#endif
//#include <libwebsockets.h>

// List for websocket triggers
WX_DEFINE_LIST(TRIGGERLIST);

//#define DEBUGPRINT

static CControlObject *gpctrlObj;

#ifdef WIN32

typedef struct _ASTAT_ {
    ADAPTER_STATUS adapt;
    NAME_BUFFER NameBuff [30];

} ASTAT, * PASTAT;

ASTAT Adapter;

#endif


#ifdef WIN32

WORD wVersionRequested = MAKEWORD(1, 1); // WSA functions
WSADATA wsaData; // WSA functions

#endif

///////////////////////////////////////////////////
//		     WEBSOCKETS
///////////////////////////////////////////////////

#ifdef WIN32
#else

static int gbClose;

// lws http

struct per_session_data__http {
    int fd;
};

struct per_session_data__dumb_increment {
    int number;
};

// lws-mirror_protocol 

#define MAX_MIRROR_MESSAGE_QUEUE 64

struct per_session_data__lws_mirror {
    struct libwebsocket *wsi;
    int ringbuffer_tail;
};

struct mirrorws_message {
    void *payload;
    size_t len;
};

static struct mirrorws_message mirrorws_ringbuffer[ MAX_MIRROR_MESSAGE_QUEUE ];
static int mirrorws_ringbuffer_head;

// vscp websocket_protocol

#define MAX_VSCPWS_MESSAGE_QUEUE 512

struct per_session_data__lws_vscp {
    struct libwebsocket *wsi; // The websocket.
    wxArrayString *pMessageList; // Messages (not events) to client.
    CClientItem *pClientItem; // Client structure for websocket in VSCP world
    bool bTrigger; // True to activate trigger functionality.
    uint32_t triggerTimeout; // Time out before trigg (or errror) must occur.
    TRIGGERLIST listTriggerOK; // List with positive triggers.
    TRIGGERLIST listTriggerERR; // List with negative triggers.
};


// list of supported websocket protocols and callbacks 

static struct libwebsocket_protocols protocols[] = {

    // first protocol must always be HTTP handler 

    {
        "http-only", // name 
        CControlObject::callback_http, // callback 
        sizeof(struct per_session_data__http), // per_session_data_size 
        0, // max frame size / rx buffer 
    },
    {
        "dumb-increment-protocol",
        CControlObject::callback_dumb_increment,
        sizeof( struct per_session_data__dumb_increment),
        10,
    },
    {
        "lws-mirror-protocol",
        CControlObject::callback_lws_mirror,
        sizeof(struct per_session_data__lws_mirror),
        128,
    },
    {
        "very-simple-control-protocol",
        CControlObject::callback_lws_vscp,
        sizeof( struct per_session_data__lws_vscp),
        128
    },
    {
        NULL, NULL, 0, 0 // End of list 
    }
};

struct libwebsocket_extension libwebsocket_internal_extensions[] = {
    { /* terminator */
        NULL, NULL, 0
    }
};

#endif


///////////////////////////////////////////////////
//		          WEBSERVER
///////////////////////////////////////////////////

#ifdef WIN32
#else


/**
 * Linked list of all active sessions.  Yes, O(n) but a
 * hash table would be overkill for a simple example...
 */
static struct websrv_Session *websrv_sessions;







// List of all pages served by this HTTP server.
// If URL is NULL a coded page should be delivered. If it's there a
// static page is delivered
static struct Page pages[] = 
  {
    { "/vscp", "text/html",  &CControlObject::websrv_serve_mainpage, NULL },
    { "/vscp/", "text/html",  &CControlObject::websrv_serve_mainpage, NULL },
    { "/vscp/interfaces", "text/html", &CControlObject::websrv_serve_interfaces, NULL },
    { "/vscp/dm", "text/html", &CControlObject::websrv_serve_dmlist, NULL },
    { "/vscp/dmedit", "text/html", &CControlObject::websrv_serve_dmedit, NULL },
    { "/vscp/dmpost", "text/html", &CControlObject::websrv_serve_dmpost, NULL },
    { "/vscp/dmdelete", "text/html", &CControlObject::websrv_serve_dmdelete, NULL },
    { "/vscp/discovery", "text/html", &CControlObject::websrv_serve_simple_page, NEXTVERSION_PAGE },
    { "/vscp/session", "text/html", &CControlObject::websrv_serve_simple_page, NEXTVERSION_PAGE },
    { "/vscp/configure", "text/html", &CControlObject::websrv_serve_simple_page, NEXTVERSION_PAGE },
    { "/vscp/variables", "text/html", &CControlObject::websrv_serve_variables_list, NULL },
    { "/vscp/varedit", "text/html", &CControlObject::websrv_serve_variables_edit, NULL },
    { "/vscp/varpost", "text/html", &CControlObject::websrv_serve_variables_post, NULL },
    { "/vscp/vardelete", "text/html", &CControlObject::websrv_serve_variables_delete, NULL },
    { "/vscp/varnew", "text/html", &CControlObject::websrv_serve_variables_new, NULL },
    { "/vscp/bootload", "text/html", &CControlObject::websrv_serve_simple_page, NEXTVERSION_PAGE },
    { "/m2m", "text/html", &CControlObject::websrv_serve_simple_page, NEXTVERSION_PAGE },
    { NULL, NULL, &CControlObject::websrv_not_found_page, NULL } /* 404 */
  };

#endif

WX_DEFINE_LIST(CanalMsgList);
WX_DEFINE_LIST(VSCPEventList);


// Initialize statics
wxString CControlObject::m_pathRoot = _("/srv/vscp/www");


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CControlObject::CControlObject()
{
    int i;
    m_bQuit = false; // true if we should quit
    gpctrlObj = this; // needed by websocket static callbacks

    m_maxItemsInClientReceiveQueue = MAX_ITEMS_CLIENT_RECEIVE_QUEUE;

    // Nill the GUID
    //memset(m_GUID, 0, 16);
    m_guid.clear();

    // Initialize the client map
    // to all unused
    for (i = 0; i < VSCP_MAX_CLIENTS; i++) {
        m_clientMap[ i ] = 0;
    }


    // Set default TCP Port
    m_tcpport = VSCP_LEVEL2_TCP_PORT;

    // Set default UDP port
    m_UDPPort = VSCP_LEVEL2_UDP_PORT;

    // Set Default Log Level
    m_logLevel = 0;


    // Control TCP/IP Interface
    m_bTCPInterface = true;

    // Default TCP/IP interface
    m_strTcpInterfaceAddress = _("");

    // Canaldriver
    m_bCanalDrivers = true;

    // Control VSCP
    m_bVSCPDaemon = true;

    // Control DM
    m_bDM = true;

    // Use variables
    m_bVariables = true;

    m_pclientMsgWorkerThread = NULL;
    m_pTcpClientListenThread = NULL;
    m_pdaemonVSCPThread = NULL;
    m_pudpSendThread = NULL;
    m_pudpReceiveThread = NULL;
    
    // Websocket interface
    m_portWebsockets = 7681;
    m_bWebSockets = true;
    m_pathCert.Empty();
    m_pathKey.Empty();
    
    // Webserver interface
    m_portWebServer = 8080;
    m_bWebServer = true;

    // Set control object
    m_dm.setControlObject(this);

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

    CControlObject::m_pathRoot = _("c:\\temp");

#else

    CControlObject::m_pathRoot = _("/srv/vscp/www");

#endif

    // Initialize the CRC
    crcInit();

}

CControlObject::~CControlObject()
{
    // Remove objects in Client send queue
    VSCPEventList::iterator iterVSCP;

    m_mutexClientOutputQueue.Lock();
    for (iterVSCP = m_clientOutputQueue.begin();
            iterVSCP != m_clientOutputQueue.end(); ++iterVSCP) {
        vscpEvent *pEvent = *iterVSCP;
        deleteVSCPevent(pEvent);
    }

    m_clientOutputQueue.Clear();
    m_mutexClientOutputQueue.Unlock();

}



/////////////////////////////////////////////////////////////////////////////
// init

bool CControlObject::init(wxString& strcfgfile)
{
    //wxLog::AddTraceMask( "wxTRACE_doWorkLoop" );
    //wxLog::AddTraceMask(_("wxTRACE_vscpd_receiveQueue")); // Receive queue
    wxLog::AddTraceMask(_("wxTRACE_vscpd_Msg"));
    wxLog::AddTraceMask(_("wxTRACE_VSCP_Msg"));
    //wxLog::AddTraceMask(_("wxTRACE_vscpd_ReceiveMutex"));
    //wxLog::AddTraceMask(_("wxTRACE_vscpd_sendMutexLevel1"));
    //wxLog::AddTraceMask(_("wxTRACE_vscpd_LevelII"));
    //wxLog::AddTraceMask( _( "wxTRACE_vscpd_dm" ) );

    wxString str = _("VSCP Daemon started\n");
    str += _("Version: ");
    str += _(VSCPD_DISPLAY_VERSION);
    str += _("\n");
    str += _(VSCPD_COPYRIGHT);
    str += _("\n");
    logMsg(str);

    // A configuration file must be available
    if (!wxFile::Exists(strcfgfile)) {
        logMsg(_("No configuration file given. Can't initialise!.\n"), DAEMON_LOGMSG_CRITICAL);
        logMsg(_("Path = .") + strcfgfile + _("\n"), DAEMON_LOGMSG_CRITICAL);
        return FALSE;
    }

    //::wxLogDebug(_("Using configuration file: ") + strcfgfile + _("\n"));
    
    // Generate username and password for drivers
    char buf[ 64 ];
    randPassword pw(3);

    pw.generatePassword(32, buf);
    m_driverUsername = wxString::FromAscii(buf);
    pw.generatePassword(32, buf);
    Cmd5 md5((unsigned char *) buf);
    m_driverPassword = wxString::FromAscii(buf);

    m_userList.addUser(m_driverUsername,
            wxString::FromAscii(md5.getDigest()),
            _T("admin"),
            NULL,
            _T(""),
            _T(""));

    // Read configuration
    if (!readConfiguration(strcfgfile)) {
        logMsg(_("Unable to open/parse configuration file. Can't initialize!.\n"), DAEMON_LOGMSG_CRITICAL);
        logMsg(_("Path = .") + strcfgfile + _("\n"), DAEMON_LOGMSG_CRITICAL);
        return FALSE;
    }
    
    // Read mime types
    if (!readMimeTypes(m_pathToMimeTypeFile)) {
        logMsg(_("Unable to open/parse mime type file.\n"), DAEMON_LOGMSG_CRITICAL);
        logMsg(_("Path = .") + m_pathToMimeTypeFile + _("\n"), DAEMON_LOGMSG_CRITICAL);
    }
    
    str.Printf(_("Log Level = %d\n"), m_logLevel );
    logMsg(str);
    //printf("Loglevel=%n\n",m_logLevel);

    // Get GUID
    if ( m_guid.isNULL() ) {
        if (!getMacAddress(m_guid)) {
            // We failed to create GUID from MAC address use
            // 'localhost' IP instead as the base.
            getIPAddress(m_guid);
        }
    }

    // Load decision matrix if mechanism is enabled
    if (m_bDM) {
        logMsg(_("DM enabled.\n"), DAEMON_LOGMSG_INFO);
        m_dm.load();
    }
    else {
        logMsg(_("DM disabled.\n"), DAEMON_LOGMSG_INFO);
    }

    // Load variables if mechanism is enabled
    if (m_bVariables) {
        logMsg(_("Variables enabled.\n"), DAEMON_LOGMSG_INFO);
        m_VSCP_Variables.load();
    }
    else {
        logMsg(_("Variables disabled.\n"), DAEMON_LOGMSG_INFO);
    }

    startClientWorkerThread();

    if (m_bCanalDrivers) {
        logMsg(_("Level I drivers enabled.\n"), DAEMON_LOGMSG_INFO);
        startDeviceWorkerThreads();
    }
    else {
        logMsg(_("Level I drivers disabled.\n"), DAEMON_LOGMSG_INFO);
    }

    if (m_bTCPInterface) {
        logMsg(_("TCP/IP interface enabled.\n"), DAEMON_LOGMSG_INFO);
        startTcpWorkerThread();
    }
    else {
        logMsg(_("TCP/IP interface disabled.\n"), DAEMON_LOGMSG_INFO);
    }

    startDaemonWorkerThread();
    
    if ( m_bWebSockets ) {
        logMsg(_("WebSocket interface active.\n"), DAEMON_LOGMSG_INFO);
    }
    else {
        logMsg(_("WebSocket interface disabled.\n"), DAEMON_LOGMSG_INFO);
    }
    
    if ( m_bWebServer ) {
        logMsg(_("WebServer interface active.\n"), DAEMON_LOGMSG_INFO);
    }
    else {
        logMsg(_("WebServer interface disabled.\n"), DAEMON_LOGMSG_INFO);
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

    // We need to create a clientItem and add this object to the list
    CClientItem *pClientItem = new CClientItem;
    if (NULL == pClientItem) {
        wxLogDebug(_("ControlObject: Unable to allocate Client item, Ending"));
        logMsg(_("Unable to allocate Client item, Ending."), DAEMON_LOGMSG_CRITICAL);
        return false;
    }

    // Save a pointer to the client item
    m_dm.m_pClientItem = pClientItem;

    // Set Filter/Mask for full DM table
    memcpy(&pClientItem->m_filterVSCP, &m_dm.m_DM_Table_filter, sizeof( vscpEventFilter));

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
    addClient(pClientItem);
    m_wxClientMutex.Unlock();

    // Feed startup event
    m_dm.feed(&EventStartUp);

    // Initialize websockets
    int opts = 0;
    unsigned int oldus = 0;

    

#ifdef WIN32
	/*
	//char interface_name[ 128 ] = "";
    const char *websockif = NULL;
    struct libwebsocket_context *pcontext;
    unsigned char buf[ LWS_SEND_BUFFER_PRE_PADDING + 1024 +
            LWS_SEND_BUFFER_POST_PADDING ];

    m_context = libwebsocket_create_context(m_portWebsockets,
            websockif,
            protocols,
            libwebsocket_internal_extensions,
            NULL,
            NULL,
            -1,
            -1,
            opts);
	*/
#else
	//char interface_name[ 128 ] = "";
    const char *websockif = NULL;
    struct libwebsocket_context *pcontext;
    unsigned char buf[ LWS_SEND_BUFFER_PRE_PADDING + 1024 +
            LWS_SEND_BUFFER_POST_PADDING ];


    lws_context_creation_info info;
    info.port = m_portWebsockets;
    info.iface = websockif;
    info.protocols = protocols;
    info.extensions = libwebsocket_get_internal_extensions();
    info.ssl_ca_filepath = NULL;
    info.ssl_cert_filepath = NULL;
    info.ssl_cipher_list = NULL;
    info.ssl_private_key_filepath = NULL;
    info.gid = -1;
    info.uid = -1;
    info.options = opts;
    info.user = NULL;
    info.ka_time = 0;
    info.ka_probes = 0;
    info.ka_interval = 0;

    pcontext = libwebsocket_create_context(&info);
    if (NULL == pcontext) {
        logMsg(_("Unable to initialize websockets. Terminating!\n"), DAEMON_LOGMSG_CRITICAL);
        return FALSE;
    }

#endif
    
#ifdef WIN32
#else
    // Web server
    struct MHD_Daemon *pwebserver;

    pwebserver = MHD_start_daemon(
            MHD_USE_THREAD_PER_CONNECTION | MHD_USE_DEBUG /*| MHD_USE_SSL*/,
            m_portWebServer,
            &websrv_callback_check_address,
            (void *) this,
            &websrv_callback_webpage,
            (void *) this,
            MHD_OPTION_CONNECTION_MEMORY_LIMIT, (size_t)(256 * 1024),
            MHD_OPTION_PER_IP_CONNECTION_LIMIT, (unsigned int)(64),
            MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int)(120 /* seconds */),
            MHD_OPTION_NOTIFY_COMPLETED, &websrv_request_callback_completed, NULL,
            MHD_OPTION_END);
#endif
    
    // DM Loop
    while (!m_bQuit) {

#ifdef WIN32
		// CLOCKS_PER_SEC 
		clock_t t;
		t = clock();
#else
        struct timeval tv;
        gettimeofday(&tv, NULL);
#endif

        // Feed possible perodic event
        m_dm.feedPeriodicEvent();

        // Put the LOOP event on the queue
        // Garanties at least one lop event between every other
        // event feed to the queue
        m_dm.feed(&EventLoop);

        // tcp/ip clients uses joinable treads and therefor does not
        // delete themseves.  This is a garbage collect for unterminated 
        // tcp/ip connection threads.
        TCPCLIENTS::iterator iter;
        for (iter = m_pTcpClientListenThread->m_tcpclients.begin();
                iter != m_pTcpClientListenThread->m_tcpclients.end(); ++iter) {
            TcpClientThread *pThread = *iter;
            if ((NULL != pThread)) {
                if (pThread->m_bQuit) {
                    pThread->Wait();
                    m_pTcpClientListenThread->m_tcpclients.remove(pThread);
                    delete pThread;
                    break;
                }
            }
        }

#ifdef WIN32
#else

        /*
         * This broadcasts to all dumb-increment-protocol connections
         * at 20Hz.
         *
         * We're just sending a character 'x', in these examples the
         * callbacks send their own per-connection content.
         *
         * You have to send something with nonzero length to get the
         * callback actions delivered.
         *
         * We take care of pre-and-post padding allocation.
         */

        if (((unsigned int) tv.tv_usec - oldus) > 50000) {
            
            if (m_bWebSockets) {
                /*
                    libwebsockets_broadcast( &protocols[ PROTOCOL_DUMB_INCREMENT ],
                                    &buf[ LWS_SEND_BUFFER_PRE_PADDING ], 
                                    1 );
                    libwebsockets_broadcast( &protocols[ PROTOCOL_VSCP ],
                                    &buf[ LWS_SEND_BUFFER_PRE_PADDING ], 
                                    1 );
                 */
                libwebsocket_callback_on_writable_all_protocol(
                        &protocols[ PROTOCOL_DUMB_INCREMENT ]);

                libwebsocket_callback_on_writable_all_protocol(
                        &protocols[ PROTOCOL_VSCP ]);
            }
            oldus = tv.tv_usec;
        }
#endif

#ifdef WIN32
#else
        /*
         * This html server does not fork or create a thread for
         * websocket service, it all runs in this single loop.  So,
         * we have to give the websockets an opportunity to service
         * "manually".
         *
         * If no socket is needing service, the call below returns
         * immediately and quickly.
         */

        if ( m_bWebSockets ) libwebsocket_service(pcontext, 50);
#endif

        // Wait for event
        if (wxSEMA_TIMEOUT == pClientItem->m_semClientInputQueue.WaitTimeout(10)) {

            // Put the LOOP event on the queue
            m_dm.feed(&EventLoop);
            continue;

        }

        //---------------------------------------------------------------------------
        //                         Event received here
        //---------------------------------------------------------------------------

        if (pClientItem->m_clientInputQueue.GetCount()) {

            vscpEvent *pEvent;

            pClientItem->m_mutexClientInputQueue.Lock();
            nodeClient = pClientItem->m_clientInputQueue.GetFirst();
            pEvent = nodeClient->GetData();
            pClientItem->m_clientInputQueue.DeleteNode(nodeClient);
            pClientItem->m_mutexClientInputQueue.Unlock();

            if (NULL != pEvent) {

                if (doLevel2Filter(pEvent, &m_dm.m_DM_Table_filter)) {
                    // Feed event through matrix
                    m_dm.feed(pEvent);
                }

                // Remove the event
                deleteVSCPevent(pEvent);

            } // Valid pEvent pointer

        } // Event in queue

    }

    // Do shutdown event
    m_dm.feed(&EventShutDown);

    // Remove messages in the client queues
    m_wxClientMutex.Lock();
    removeClient(pClientItem);
    m_wxClientMutex.Unlock();

#ifdef WIN32
#else
    if ( m_bWebSockets ) libwebsocket_context_destroy(pcontext);
#endif

    wxLogDebug(_("ControlObject: Done"));
    return true;
}


/////////////////////////////////////////////////////////////////////////////
// cleanup

bool CControlObject::cleanup(void)
{
    stopDeviceWorkerThreads();
    stopTcpWorkerThread();
    stopClientWorkerThread();
    stopDaemonWorkerThread();

    wxLogDebug(_("ControlObject: Cleanup done"));
    return true;
}

/////////////////////////////////////////////////////////////////////////////
// logMsg
//

void CControlObject::logMsg(const wxString& wxstr, unsigned char level)
{

    wxString wxdebugmsg = wxstr;

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

    ::ReportEvent(m_hEventSource,
            EVENTLOG_INFORMATION_TYPE,
            0,
            (1L << 30),
            NULL, // sid
            iStr,
            0,
            ps,
            NULL);
#else
    //printf( wxdebugmsg.mb_str( wxConvUTF8 ) );
    if (m_logLevel >= level) {
        wxPrintf(wxdebugmsg);
    }
#endif
#else

    //::wxLogDebug(wxdebugmsg);

    if (m_logLevel >= level) {
        wxPrintf(wxdebugmsg);
    }

    switch (level) {
    case DAEMON_LOGMSG_DEBUG:
        syslog(LOG_DEBUG, "%s", (const char *) wxdebugmsg.ToAscii());
        break;

    case DAEMON_LOGMSG_INFO:
        syslog(LOG_INFO, "%s", (const char *) wxdebugmsg.ToAscii());
        break;

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

    };
#endif
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
                logMsg(_("Unable to run controlobject client thread."), DAEMON_LOGMSG_CRITICAL);
            }
        } else {
            logMsg(_("Unable to create controlobject client thread."), DAEMON_LOGMSG_CRITICAL);
        }
    } else {
        logMsg(_("Unable to allocate memory for controlobject client thread."), DAEMON_LOGMSG_CRITICAL);
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
    // Run the TCP server thread   --   TODO - multiport
    /////////////////////////////////////////////////////////////////////////////
    if (m_bTCPInterface) {
        
        m_pTcpClientListenThread = new TcpClientListenThread;

        if (NULL != m_pTcpClientListenThread) {
            m_pTcpClientListenThread->m_pCtrlObject = this;
            wxThreadError err;
            if (wxTHREAD_NO_ERROR == (err = m_pTcpClientListenThread->Create())) {
                //m_ptcpListenThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY );
                if (wxTHREAD_NO_ERROR != (err = m_pTcpClientListenThread->Run())) {
                    logMsg(_("Unable to run TCP thread."), DAEMON_LOGMSG_CRITICAL);
                }
            } else {
                logMsg(_("Unable to create TCP thread."), DAEMON_LOGMSG_CRITICAL);
            }
        } else {
            logMsg(_("Unable to allocate memory for TCP thread."), DAEMON_LOGMSG_CRITICAL);
        }
    }

    return true;
}


/////////////////////////////////////////////////////////////////////////////
// stopTcpWorkerThread
//

bool CControlObject::stopTcpWorkerThread(void)
{
    if (NULL != m_pTcpClientListenThread) {
        m_mutexTcpClientListenThread.Lock();
        m_pTcpClientListenThread->m_bQuit = true;
        m_pTcpClientListenThread->Wait();
        delete m_pTcpClientListenThread;
        m_mutexTcpClientListenThread.Unlock();
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
                    logMsg(_("Unable to start TCP VSCP daemon thread."), DAEMON_LOGMSG_CRITICAL);
                }
            } else {
                logMsg(_("Unable to create TCP VSCP daemon thread."), DAEMON_LOGMSG_CRITICAL);
            }
        } else {
            logMsg(_("Unable to start VSCP daemon thread."), DAEMON_LOGMSG_CRITICAL);
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
        m_mutexdaemonVSCPThread.Unlock();
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////
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
                
            // *****************************************
            //  Create the worker thread for the device
            // *****************************************

            pDeviceItem->m_pdeviceThread = new deviceThread();
            if (NULL != pDeviceItem->m_pdeviceThread) {

                pDeviceItem->m_pdeviceThread->m_pCtrlObject = this;
                pDeviceItem->m_pdeviceThread->m_pDeviceItem = pDeviceItem;

                wxThreadError err;
                if (wxTHREAD_NO_ERROR == (err = pDeviceItem->m_pdeviceThread->Create())) {
                    if (wxTHREAD_NO_ERROR != (err = pDeviceItem->m_pdeviceThread->Run())) {
                        logMsg(_("Unable to create DeviceThread."), DAEMON_LOGMSG_CRITICAL);
                    }
                } else {
                    logMsg(_("Unable to run DeviceThread."), DAEMON_LOGMSG_CRITICAL);
                }

            } else {
                logMsg(_("Unable to allocate memory for DeviceThread."), DAEMON_LOGMSG_CRITICAL);
            }

        } // Valid device item
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
// stopDeviceWorkerThreads
//

bool CControlObject::stopDeviceWorkerThreads(void)
{
    CDeviceItem *pDeviceItem;

    VSCPDEVICELIST::iterator iter;
    for (iter = m_deviceList.m_devItemList.begin();
            iter != m_deviceList.m_devItemList.end();
            ++iter) {

        pDeviceItem = *iter;
        if (NULL != pDeviceItem) {

            if (NULL != pDeviceItem->m_pdeviceThread) {
                pDeviceItem->m_mutexdeviceThread.Lock();
                pDeviceItem->m_bQuit = true;
                pDeviceItem->m_pdeviceThread->Wait();
                pDeviceItem->m_mutexdeviceThread.Unlock();
                delete pDeviceItem->m_pdeviceThread;
            }

        }

    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// sendEventToClient
//

void CControlObject::sendEventToClient(CClientItem *pClientItem,
        vscpEvent *pEvent)
{
    // Must be valid pointers
    if (NULL == pClientItem) return;
    if (NULL == pEvent) return;

    // Check if filtered out
    //if (!doLevel2Filter(pEvent, &pClientItem->m_filterVSCP)) return;

    // If the client queue is full for this client then the
    // client will not receive the message
    if (pClientItem->m_clientInputQueue.GetCount() >
            m_maxItemsInClientReceiveQueue) {
        // Overun
        pClientItem->m_statistics.cntOverruns++;
        return;
    }

    // Create an event
    vscpEvent *pnewvscpEvent = new vscpEvent;
    if (NULL != pnewvscpEvent) {
        
        // Copy in the new event
        memcpy(pnewvscpEvent, pEvent, sizeof( vscpEvent));

        // And data...
        if ((pEvent->sizeData > 0) && (NULL != pEvent->pdata)) {
            // Copy in data
            pnewvscpEvent->pdata = new uint8_t[ pEvent->sizeData ];
            memcpy(pnewvscpEvent->pdata, pEvent->pdata, pEvent->sizeData);
        } else {
            // No data
            pnewvscpEvent->pdata = NULL;
        }

        // Add the new event to the inputqueue
        pClientItem->m_mutexClientInputQueue.Lock();
        pClientItem->m_clientInputQueue.Append(pnewvscpEvent);
        pClientItem->m_semClientInputQueue.Post();
        pClientItem->m_mutexClientInputQueue.Unlock();

    }
}

///////////////////////////////////////////////////////////////////////////////
// sendEventAllClients
//

void CControlObject::sendEventAllClients(vscpEvent *pEvent, uint32_t excludeID)
{
    CClientItem *pClientItem;
    VSCPCLIENTLIST::iterator it;

    wxLogTrace(_("wxTRACE_vscpd_receiveQueue"),
            _(" ControlObject: event %d, excludeid = %d"),
            pEvent->obid, excludeID);

    if (NULL == pEvent) return;

    m_wxClientMutex.Lock();
    for (it = m_clientList.m_clientItemList.begin(); it != m_clientList.m_clientItemList.end(); ++it) {
        pClientItem = *it;

        wxLogTrace(_("wxTRACE_vscpd_receiveQueue"),
                _(" ControlObject: clientid = %d"),
                pClientItem->m_clientID);

        if ((NULL != pClientItem) && (excludeID != pClientItem->m_clientID)) {
            sendEventToClient(pClientItem, pEvent);
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

uint32_t CControlObject::getClientMapFromId(uint32_t clid)
{
    for (uint32_t i = 0; i < VSCP_MAX_CLIENTS; i++) {
        if (clid == m_clientMap[ i ]) return i;
    }

    return 0;
}


///////////////////////////////////////////////////////////////////////////////
//  getClientMapFromIndex
//

uint32_t CControlObject::getClientMapFromIndex(uint32_t idx)
{
    return m_clientMap[ idx ];
}


///////////////////////////////////////////////////////////////////////////////
//  addIdToClientMap
//

uint32_t CControlObject::addIdToClientMap(uint32_t clid)
{
    for (uint32_t i = 1; i < VSCP_MAX_CLIENTS; i++) {
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

bool CControlObject::removeIdFromClientMap(uint32_t clid)
{
    for (uint32_t i = 0; i < VSCP_MAX_CLIENTS; i++) {
        if (clid == m_clientMap[ i ]) {
            m_clientMap[ i ] = 0;
            return true;
        }
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////
// addClient
//

void CControlObject::addClient(CClientItem *pClientItem, uint32_t id)
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

void CControlObject::removeClient(CClientItem *pClientItem)
{
    // Remove the mapped item
    removeIdFromClientMap(pClientItem->m_clientID);

    // Remove the client
    m_clientList.removeClient(pClientItem);
}


///////////////////////////////////////////////////////////////////////////////
//  getMacAddress
//

bool CControlObject::getMacAddress(cguid& guid)
{
#ifdef WIN32

    bool rv = false;
    NCB Ncb;
    UCHAR uRetCode;
    LANA_ENUM lenum;
    int i;

    // Clear the GUID
    guid.clear();

    memset(&Ncb, 0, sizeof( Ncb));
    Ncb.ncb_command = NCBENUM;
    Ncb.ncb_buffer = (UCHAR *) & lenum;
    Ncb.ncb_length = sizeof( lenum);
    uRetCode = Netbios(&Ncb);
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
            guid.setAt( 15, 0xff );
            guid.setAt( 14, 0xff );
            guid.setAt( 13, 0xff );
            guid.setAt( 12, 0xff );
            guid.setAt( 11, 0xff );
            guid.setAt( 10, 0xff );
            guid.setAt( 9, 0xff );
            guid.setAt( 8, 0xfe );
            guid.setAt( 7, Adapter.adapt.adapter_address[ 0 ] );
            guid.setAt( 6, Adapter.adapt.adapter_address[ 1 ] );
            guid.setAt( 5, Adapter.adapt.adapter_address[ 2 ] );
            guid.setAt( 4, Adapter.adapt.adapter_address[ 3 ] );
            guid.setAt( 3, Adapter.adapt.adapter_address[ 4 ] );
            guid.setAt( 2, Adapter.adapt.adapter_address[ 5 ] );
            guid.setAt( 1, 0 );
            guid.setAt( 0, 0 );
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

    bool rv = true;
    struct ifreq ifr;
    int fd;

    // Clear the GUID
    guid.clear();

    fd = socket(PF_INET, SOCK_RAW, htons(ETH_P_ALL));
    memset(&ifr, 0, sizeof( ifr));
    strncpy(ifr.ifr_name, "eth0", sizeof( ifr.ifr_name));
    if (ioctl(fd, SIOCGIFHWADDR, &ifr) >= 0) {
        unsigned char *ptr;
        ptr = (unsigned char *) &ifr.ifr_ifru.ifru_hwaddr.sa_data[ 0 ];
        logMsg(wxString::Format(_(" Ethernet MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n"),
                *ptr,
                *(ptr + 1),
                *(ptr + 2),
                *(ptr + 3),
                *(ptr + 4),
                *(ptr + 5)), DAEMON_LOGMSG_INFO);
        guid.setAt( 15, 0xff );
        guid.setAt( 14, 0xff );
        guid.setAt( 13, 0xff );
        guid.setAt( 12, 0xff );
        guid.setAt( 11, 0xff );
        guid.setAt( 10, 0xff );
        guid.setAt( 9, 0xff );
        guid.setAt( 8, 0xfe );
        guid.setAt( 7, *ptr );
        guid.setAt( 6, *(ptr + 1) );
        guid.setAt( 5, *(ptr + 2) );
        guid.setAt( 4, *(ptr + 3) );
        guid.setAt( 3, *(ptr + 4));
        guid.setAt( 2, *(ptr + 5) );
        guid.setAt( 1, 0 );
        guid.setAt( 0, 0 );
    } else {
        logMsg(_("Failed to get hardware address (must be root?).\n"), DAEMON_LOGMSG_WARNING);
        rv = false;
    }

    return rv;


#endif

}



///////////////////////////////////////////////////////////////////////////////
//  getIPAddress
//

bool CControlObject::getIPAddress(cguid& guid)
{
    // Clear the GUID
    guid.clear();

    guid.setAt( 15, 0xff );
    guid.setAt( 14, 0xff );
    guid.setAt( 13, 0xff );
    guid.setAt( 12, 0xff );
    guid.setAt( 11, 0xff );
    guid.setAt( 10, 0xff );
    guid.setAt( 9, 0xff );
    guid.setAt( 8, 0xfd );

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

    guid.setAt( 7, (localaddr[ 0 ] >> 24) & 0xff );
    guid.setAt( 6, (localaddr[ 0 ] >> 16) & 0xff );
    guid.setAt( 5, (localaddr[ 0 ] >> 8) & 0xff );
    guid.setAt( 4, localaddr[ 0 ] & 0xff );
    
    return true;
}


///////////////////////////////////////////////////////////////////////////////
// readConfiguration
//
// Read the configuration XML file
//

bool CControlObject::readConfiguration(wxString& strcfgfile)
{
    unsigned long val;
    wxXmlDocument doc;
    if (!doc.Load(strcfgfile)) {
        return false;
    }

    // start processing the XML file
    if (doc.GetRoot()->GetName() != wxT("vscpconfig")) {
        return false;
    }

    wxXmlNode *child = doc.GetRoot()->GetChildren();
    while (child) {

        if (child->GetName() == wxT("general")) {

            wxXmlNode *subchild = child->GetChildren();
            while (subchild) {

                // Deprecated <==============
                if (subchild->GetName() == wxT("tcpport")) {
                    wxString str = subchild->GetNodeContent();
                    m_TCPPort = readStringValue(str);
                }// Deprecated <==============
                else if (subchild->GetName() == wxT("udpport")) {
                    wxString str = subchild->GetNodeContent();
                    m_UDPPort = readStringValue(str);
                } else if (subchild->GetName() == wxT("loglevel")) {
                    wxString str = subchild->GetNodeContent();
                    str.Trim();
                    str.Trim(false);
                    if ( str.IsSameAs(_("NONE"), false)) {
                        m_logLevel = DAEMON_LOGMSG_NONE;
                    }
                    else if ( str.IsSameAs(_("INFO"), false)) {
                        m_logLevel = DAEMON_LOGMSG_INFO;
                    }
                    else if ( str.IsSameAs(_("NOTICE"), false)) {
                        m_logLevel = DAEMON_LOGMSG_NOTICE;
                    }
                    else if ( str.IsSameAs(_("WARNING"), false)) {
                        m_logLevel = DAEMON_LOGMSG_WARNING;
                    }
                    else if ( str.IsSameAs(_("ERROR"), false)) {
                        m_logLevel = DAEMON_LOGMSG_ERROR;
                    }
                    else if ( str.IsSameAs(_("CRITICAL"), false)) {
                        m_logLevel = DAEMON_LOGMSG_CRITICAL;
                    }
                    else if ( str.IsSameAs(_("ALERT"), false)) {
                        m_logLevel = DAEMON_LOGMSG_ALERT;
                    }
                    else if ( str.IsSameAs(_("EMERGENCY"), false)) {
                        m_logLevel = DAEMON_LOGMSG_EMERGENCY;
                    }
                    else if ( str.IsSameAs(_("DEBUG"), false)) {
                        m_logLevel = DAEMON_LOGMSG_DEBUG;
                    }
                    else {
                        m_logLevel = readStringValue(str);
                    }
                } else if (subchild->GetName() == wxT("tcpif")) {
#if wxCHECK_VERSION(3,0,0)                    
                    wxString property = subchild->GetAttribute(wxT("enabled"), wxT("true"));
#else
                    wxString property = subchild->GetPropVal(wxT("enabled"), wxT("true"));
#endif                    
                    if (property.IsSameAs(_("false"), false)) {
                        m_bTCPInterface = false;
                    }

#if wxCHECK_VERSION(3,0,0)
                    property = subchild->GetAttribute(wxT("port"), wxT("9598"));
#else
                    property = subchild->GetPropVal(wxT("port"), wxT("9598"));
#endif                    
                    if (property.IsNumber()) {
                        m_TCPPort = readStringValue(property);
                    }

#if wxCHECK_VERSION(3,0,0)                    
                    m_strTcpInterfaceAddress = subchild->GetAttribute(wxT("ifaddress"), wxT(""));
#else
                    m_strTcpInterfaceAddress = subchild->GetPropVal(wxT("ifaddress"), wxT(""));
#endif                    

                } else if (subchild->GetName() == wxT("canaldriver")) {
#if wxCHECK_VERSION(3,0,0)                    
                    wxString property = subchild->GetAttribute(wxT("enabled"), wxT("true"));
#else 
                    wxString property = subchild->GetPropVal(wxT("enabled"), wxT("true"));
#endif                    
                    if (property.IsSameAs(_("false"), false)) {
                        m_bCanalDrivers = false;
                    } else {
                        m_bCanalDrivers = true;
                    }
                } 
                else if (subchild->GetName() == wxT("dm")) {
                    // Should the internal DM be disabled
#if wxCHECK_VERSION(3,0,0)                    
                    wxString property = subchild->GetAttribute(wxT("enabled"), wxT("true"));
#else 
                    wxString property = subchild->GetPropVal(wxT("enabled"), wxT("true"));
#endif                    
                    if (property.IsSameAs(_("false"), false)) {
                        m_bDM = false;
                    }

                    // Get the path to the DM file
#if wxCHECK_VERSION(3,0,0)                    
                    m_dm.m_configPath = subchild->GetAttribute(wxT("path"), wxT(""));
#else 
                    m_dm.m_configPath = subchild->GetPropVal(wxT("path"), wxT(""));
#endif                    
                    m_dm.m_configPath.Trim();
                    m_dm.m_configPath.Trim(false);
                    
                }                 
                else if (subchild->GetName() == wxT("variables")) {
                    // Should the internal DM be disabled
#if wxCHECK_VERSION(3,0,0)                    
                    wxString property = subchild->GetAttribute(wxT("enabled"), wxT("true"));
#else 
                    wxString property = subchild->GetPropVal(wxT("enabled"), wxT("true"));
#endif                    
                    if (property.IsSameAs(_("false"), false)) {
                        m_bVariables = false;
                    }

                    // Get the path to the DM file
#if wxCHECK_VERSION(3,0,0)                    
                    m_VSCP_Variables.m_configPath = subchild->GetAttribute(wxT("path"), wxT(""));
#else 
                    m_VSCP_Variables.m_configPath = subchild->GetPropVal(wxT("path"), wxT(""));
#endif                    
                    m_VSCP_Variables.m_configPath.Trim();
                    m_VSCP_Variables.m_configPath.Trim(false);

                } else if (subchild->GetName() == wxT("vscp")) {
#if wxCHECK_VERSION(3,0,0)                    
                    wxString property = subchild->GetAttribute(wxT("enabled"), wxT("true"));
#else 
                    wxString property = subchild->GetPropVal(wxT("enabled"), wxT("true"));
#endif                    
                    if (property.IsSameAs(_("false"), false)) {
                        m_bVSCPDaemon = false;
                    }
                } else if (subchild->GetName() == wxT("guid")) {
                    wxString str = subchild->GetNodeContent();
                    //getGuidFromStringToArray(m_GUID, str);
                    m_guid.getFromString(str);
                } else if (subchild->GetName() == wxT("clientbuffersize")) {
                    wxString str = subchild->GetNodeContent();
                    m_maxItemsInClientReceiveQueue = readStringValue(str);
                } else if (subchild->GetName() == wxT("webrootpath")) {
                    CControlObject::m_pathRoot = subchild->GetNodeContent();
                    CControlObject::m_pathRoot.Trim();
                    CControlObject::m_pathRoot.Trim(false);
                } else if (subchild->GetName() == wxT("pathcert")) {
                    m_pathCert = subchild->GetNodeContent();
                    m_pathCert.Trim();
                    m_pathCert.Trim(false);
                } else if (subchild->GetName() == wxT("pathkey")) {
                    m_pathKey = subchild->GetNodeContent();
                    m_pathKey.Trim();
                    m_pathKey.Trim(false);
                } else if (subchild->GetName() == wxT("websockets")) {
#if wxCHECK_VERSION(3,0,0)                    
                    wxString property = subchild->GetAttribute(wxT("enabled"), wxT("true"));
#else 
                    wxString property = subchild->GetPropVal(wxT("enabled"), wxT("true"));
#endif                    
                    if (property.IsSameAs(_("false"), false)) {
                        m_bWebSockets = false;
                    }

#if wxCHECK_VERSION(3,0,0)                    
                    property = subchild->GetAttribute(wxT("port"), wxT("7681"));
#else 
                    property = subchild->GetPropVal(wxT("port"), wxT("7681"));
#endif                    
                    if (property.IsNumber()) {
                        m_portWebsockets = readStringValue(property);
                    }
                } else if (subchild->GetName() == wxT("webserver")) {
#if wxCHECK_VERSION(3,0,0)                    
                    wxString property = subchild->GetAttribute(wxT("enabled"), wxT("true"));
#else 
                    wxString property = subchild->GetPropVal(wxT("enabled"), wxT("true"));
#endif                    
                    if (property.IsSameAs(_("false"), false)) {
                        m_bWebServer = false;
                    }

#if wxCHECK_VERSION(3,0,0)                    
                    property = subchild->GetAttribute(wxT("port"), wxT("8080"));
#else 
                    property = subchild->GetPropVal(wxT("port"), wxT("8080"));
#endif                    
                    if (property.IsNumber()) {
                        m_portWebServer = readStringValue(property);
                    }
                }
                else if (subchild->GetName() == wxT("pathtomimetypes")) {
                    m_pathToMimeTypeFile = subchild->GetNodeContent();
                    m_pathToMimeTypeFile.Trim();
                    m_pathToMimeTypeFile.Trim(false);
                } 

                subchild = subchild->GetNext();
            }

            wxString content = child->GetNodeContent();

        } 
        else if (child->GetName() == wxT("remoteuser")) {

            wxXmlNode *subchild = child->GetChildren();
            while (subchild) {
                vscpEventFilter VSCPFilter;
                bool bFilterPresent = false;
                bool bMaskPresent = false;
                wxString name;
                wxString md5;
                wxString privilege;
                wxString allowfrom;
                wxString allowevent;
                bool bUser = false;

                clearVSCPFilter(&VSCPFilter); // Allow all frames

                if (subchild->GetName() == wxT("user")) {

                    wxXmlNode *subsubchild = subchild->GetChildren();

                    while (subsubchild) {
                        if (subsubchild->GetName() == wxT("name")) {
                            name = subsubchild->GetNodeContent();
                            bUser = true;
                        } else if (subsubchild->GetName() == wxT("password")) {
                            md5 = subsubchild->GetNodeContent();
                        } else if (subsubchild->GetName() == wxT("privilege")) {
                            privilege = subsubchild->GetNodeContent();
                        } else if (subsubchild->GetName() == wxT("filter")) {
                            bFilterPresent = true;
#if wxCHECK_VERSION(3,0,0)                            
                            wxString str_vscp_priority = subchild->GetAttribute(wxT("priority"), wxT("0"));
#else 
                            wxString str_vscp_priority = subchild->GetPropVal(wxT("priority"), wxT("0"));
#endif                            
                            val = 0;
                            str_vscp_priority.ToULong(&val);
                            VSCPFilter.filter_priority = val;
#if wxCHECK_VERSION(3,0,0)                            
                            wxString str_vscp_class = subchild->GetAttribute(wxT("class"), wxT("0"));
#else 
                            wxString str_vscp_class = subchild->GetPropVal(wxT("class"), wxT("0"));
#endif                            
                            val = 0;
                            str_vscp_class.ToULong(&val);
                            VSCPFilter.filter_class = val;
#if wxCHECK_VERSION(3,0,0)                            
                            wxString str_vscp_type = subchild->GetAttribute(wxT("type"), wxT("0"));
#else 
                            wxString str_vscp_type = subchild->GetPropVal(wxT("type"), wxT("0"));
#endif                            
                            val = 0;
                            str_vscp_type.ToULong(&val);
                            VSCPFilter.filter_type = val;
#if wxCHECK_VERSION(3,0,0)                            
                            wxString str_vscp_guid = subchild->GetAttribute(wxT("guid"),
#else 
                            wxString str_vscp_guid = subchild->GetPropVal(wxT("guid"),
#endif                                    
                                    wxT("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"));
                            getGuidFromStringToArray(VSCPFilter.filter_GUID, str_vscp_guid);
                        } else if (subsubchild->GetName() == wxT("mask")) {
                            bMaskPresent = true;
#if wxCHECK_VERSION(3,0,0)                            
                            wxString str_vscp_priority = subchild->GetAttribute(wxT("priority"), wxT("0"));
#else 
                            wxString str_vscp_priority = subchild->GetPropVal(wxT("priority"), wxT("0"));
#endif                            
                            val = 0;
                            str_vscp_priority.ToULong(&val);
                            VSCPFilter.mask_priority = val;
#if wxCHECK_VERSION(3,0,0)                            
                            wxString str_vscp_class = subchild->GetAttribute(wxT("class"), wxT("0"));
#else 
                            wxString str_vscp_class = subchild->GetPropVal(wxT("class"), wxT("0"));
#endif                            
                            val = 0;
                            str_vscp_class.ToULong(&val);
                            VSCPFilter.mask_class = val;
#if wxCHECK_VERSION(3,0,0)                            
                            wxString str_vscp_type = subchild->GetAttribute(wxT("type"), wxT("0"));
#else 
                            wxString str_vscp_type = subchild->GetPropVal(wxT("type"), wxT("0"));
#endif                            
                            val = 0;
                            str_vscp_type.ToULong(&val);
                            VSCPFilter.mask_type = val;
#if wxCHECK_VERSION(3,0,0)                            
                            wxString str_vscp_guid = subchild->GetAttribute(wxT("guid"),
#else 
                            wxString str_vscp_guid = subchild->GetPropVal(wxT("guid"),
#endif                                    
                                    wxT("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"));
                            getGuidFromStringToArray(VSCPFilter.mask_GUID, str_vscp_guid);
                        } else if (subsubchild->GetName() == wxT("allowfrom")) {
                            allowfrom = subsubchild->GetNodeContent();
                        } else if (subsubchild->GetName() == wxT("allowevent")) {
                            allowevent = subsubchild->GetNodeContent();
                        }

                        subsubchild = subsubchild->GetNext();

                    }

                }

                // Add user
                if (bUser) {

                    if (bFilterPresent && bMaskPresent) {
                        m_userList.addUser(name, md5, privilege, &VSCPFilter, allowfrom, allowevent);
                    } else {
                        m_userList.addUser(name, md5, privilege, NULL, allowfrom, allowevent);
                    }

                    bUser = false;
                    bFilterPresent = false;
                    bMaskPresent = false;

                }

                subchild = subchild->GetNext();

            }

        } 
        else if (child->GetName() == wxT("interfaces")) {

            wxXmlNode *subchild = child->GetChildren();
            while (subchild) {

                wxString ip;
                wxString mac;
                wxString guid;
                bool bInterface = false;

                if (subchild->GetName() == wxT("interface")) {
                    wxXmlNode *subsubchild = subchild->GetChildren();

                    while (subsubchild) {

                        if (subsubchild->GetName() == wxT("ipaddress")) {
                            ip = subsubchild->GetNodeContent();
                            bInterface = true;
                        } 
                        else if (subsubchild->GetName() == wxT("macaddress")) {
                            mac = subsubchild->GetNodeContent();
                        } 
                        else if (subsubchild->GetName() == wxT("guid")) {
                            guid = subsubchild->GetNodeContent();
                        }

                        subsubchild = subsubchild->GetNext();

                    }

                }

                // Add interface
                if (bInterface) {
                    m_interfaceList.addInterface(ip, mac, guid);
                    bInterface = false;
                }

                subchild = subchild->GetNext();

            }

        }
            // Level I driver
        else if (child->GetName() == wxT("canaldriver")) {

            wxXmlNode *subchild = child->GetChildren();
            while (subchild) {
                wxString strName;
                wxString strConfig;
                wxString strPath;
                unsigned long flags;
                wxString strGUID;
                bool bEnabled = true;
                bool bCanalDriver = false;

                if (subchild->GetName() == wxT("driver")) {
                    
                    wxXmlNode *subsubchild = subchild->GetChildren();
                    
#if wxCHECK_VERSION(3,0,0)                    
                    wxString property = subchild->GetAttribute(wxT("enable"), wxT("true"));
#else 
                    wxString property = subchild->GetPropVal(wxT("enable"), wxT("true"));
#endif                    
                    if (property.IsSameAs(_("false"), false)) {
                        bEnabled = false;
                    }
                    
                    while (subsubchild) {
                        if (subsubchild->GetName() == wxT("name")) {
                            strName = subsubchild->GetNodeContent();
                            strName.Trim();
                            strName.Trim(false);
                            // Replace spaces in name with underscore
                            int pos;
                            while (wxNOT_FOUND != (pos = strName.Find(_(" ")))) {
                                strName.SetChar(pos, wxChar('_'));
                            }
                            bCanalDriver = true;
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
                        else if (subsubchild->GetName() == wxT("flags")) {
                            wxString str = subsubchild->GetNodeContent();
                            flags = readStringValue(str);
                        } 
                        else if (subsubchild->GetName() == wxT("guid")) {
                            strGUID = subsubchild->GetNodeContent();
                            strGUID.Trim();
                            strGUID.Trim(false);
                        }

                        // Next driver item
                        subsubchild = subsubchild->GetNext();

                    }

                }

                // Configuration data for one driver loaded
                uint8_t GUID[ 16 ];

                // Nill the GUID
                memset(GUID, 0, 16);

                if (strGUID.Length()) {
                    getGuidFromStringToArray(GUID, strGUID);
                }

                // Add the device
                if (bCanalDriver) {

                    if (!m_deviceList.addItem( strName,
                                                strConfig,
                                                strPath,
                                                flags,
                                                GUID,
                                                VSCP_DRIVER_LEVEL1,
                                                bEnabled ) ) {
                        wxString errMsg = _("Driver not added. Path does not exist. - [ ") +
                                strPath + _(" ]\n");
                        logMsg(errMsg, DAEMON_LOGMSG_ERROR);
                        //wxLogDebug(errMsg);
                    } 
                    else {
                        wxString errMsg = _("Level I driver added. - [ ") +
                                strPath + _(" ]\n");
                        logMsg(errMsg, DAEMON_LOGMSG_INFO);
                    }

                    bCanalDriver = false;

                }

                // Next driver
                subchild = subchild->GetNext();

            }

        }
            // Level II driver
        else if (child->GetName() == wxT("vscpdriver")) {

            wxXmlNode *subchild = child->GetChildren();
            
            while (subchild) {
                
                wxString strName;
                wxString strConfig;
                wxString strPath;
                wxString strGUID;
                bool bEnabled = true;
                bool bLevel2Driver = false;

                if (subchild->GetName() == wxT("driver")) {
                    
                    wxXmlNode *subsubchild = subchild->GetChildren();
                    
#if wxCHECK_VERSION(3,0,0)                    
                    wxString property = subchild->GetAttribute(wxT("enable"), wxT("true"));
#else 
                    wxString property = subchild->GetPropVal(wxT("enable"), wxT("true"));
#endif
                    if (property.IsSameAs(_("false"), false)) {
                        bEnabled = false;
                    }
                    
                    while (subsubchild) {
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
                            strGUID = subsubchild->GetNodeContent();
                            strGUID.Trim();
                            strGUID.Trim(false);
                        }

                        // Next driver item
                        subsubchild = subsubchild->GetNext();

                    }

                }

                // Configuration data for one driver loaded
                uint8_t GUID[ 16 ];

                // Nill the GUID
                memset(GUID, 0, 16);

                if (strGUID.Length()) {
                    getGuidFromStringToArray(GUID, strGUID);
                }

                // Add the device
                if (bLevel2Driver) {

                    if (!m_deviceList.addItem(strName,
                                                strConfig,
                                                strPath,
                                                0,
                                                GUID,
                                                VSCP_DRIVER_LEVEL2,
                                                bEnabled )) {
                        wxString errMsg = _("Driver not added. Path does not exist. - [ ") +
                                strPath + _(" ]\n");
                        logMsg(errMsg, DAEMON_LOGMSG_INFO);
                        //wxLogDebug(errMsg);
                    } else {
                        wxString errMsg = _("Level II driver added. - [ ") +
                                strPath + _(" ]\n");
                        logMsg(errMsg, DAEMON_LOGMSG_INFO);
                    }


                    bLevel2Driver = false;

                }

                // Next driver
                subchild = subchild->GetNext();

            }

        }


        child = child->GetNext();

    }

    return true;

}

///////////////////////////////////////////////////////////////////////////////
// readMimeTypes
//
// Read the Mime Types XML file 
//

bool CControlObject::readMimeTypes(wxString& path)
{
    unsigned long val;
    wxXmlDocument doc;
    if (!doc.Load(path)) {
        return false;
    }

    // start processing the XML file
    if (doc.GetRoot()->GetName() != wxT("mimetypes")) {
        return false;
    }

    wxXmlNode *child = doc.GetRoot()->GetChildren();
    while (child) {  
        
        if (child->GetName() == wxT("mimetype")) {
#if wxCHECK_VERSION(3,0,0)             
            wxString strEnable = child->GetAttribute(wxT("enable"), wxT("false"));
#else 
            wxString strEnable = child->GetPropVal(wxT("enable"), wxT("false"));
#endif 
#if wxCHECK_VERSION(3,0,0)             
            wxString strExt = child->GetAttribute(wxT("extension"), wxT(""));
#else 
            wxString strExt = child->GetPropVal(wxT("extension"), wxT(""));            
#endif
#if wxCHECK_VERSION(3,0,0)             
            wxString strType = child->GetAttribute(wxT("mime"), wxT(""));
#else 
            wxString strType = child->GetPropVal(wxT("mime"), wxT(""));            
#endif            
            if ( strEnable.IsSameAs(_("true"),false )) {
                m_hashMimeTypes[strExt] = strType;
            }
        }
        
        child = child->GetNext();

    }
    
    return true;
}


////////////////////////
// websocket callbaks
///////////////////////


#ifdef WIN32
#else


///////////////////////////////////////////////////////////////////////////////
// callback_http
//
// this protocol server (always the first one) just knows how to do HTTP 
//

int
CControlObject::callback_http(struct libwebsocket_context *context,
        struct libwebsocket *wsi,
        enum libwebsocket_callback_reasons reason,
        void *user,
        void *in,
        size_t len)
{
    char client_name[ 128 ];
    char client_ip[ 128 ];
    wxString path;

    switch (reason) {

    case LWS_CALLBACK_HTTP:
    {
        wxString mime;
        wxString args;
        wxString str((char *) in, wxConvUTF8);

        // If we have a '?' in the URI  we have arguments that should be removed.
        int n;
        if (wxNOT_FOUND != (n = str.Find(_("?")))) {
            args = str.Right(str.Length() - n);
            str = str.Left(n);
        }

        path = CControlObject::m_pathRoot;
        str.Trim();
        str.Trim(false);
        if (('\\' == str.Last()) || ('/' == str.Last())) {
            str += _("index.html");
        }
        path += str;

        wxFileName fname(path);
        //wxFileType *ptype = wxTheMimeTypesManager->GetFileTypeFromExtension( fname.GetExt() );

        // Add some extra common mime types if non found
        if (fname.GetExt() == _("")) {
            mime = _("text/plain");
        } else if (fname.GetExt() == _("txt")) {
            mime = _("text/plain");
        } else if (fname.GetExt() == _("cfg")) {
            mime = _("text/plain");
        } else if (fname.GetExt() == _("conf")) {
            mime = _("text/plain");
        } else if (fname.GetExt() == _("js")) {
            mime = _("application/javascript");
        } else if (fname.GetExt() == _("json")) {
            mime = _("application/json");
        } else if (fname.GetExt() == _("xml")) {
            mime = _("application/xml");
        } else if (fname.GetExt() == _("htm")) {
            mime = _("text/html");
        } else if (fname.GetExt() == _("html")) {
            mime = _("text/html");
        } else if (fname.GetExt() == _("css")) {
            mime = _("text/css");
        } else if (fname.GetExt() == _("ico")) {
            mime = _("image/x-icon");
        } else if (fname.GetExt() == _("gif")) {
            mime = _("image/x-gif");
        } else if (fname.GetExt() == _("png")) {
            mime = _("image/x-png");
        } else if (fname.GetExt() == _("bmp")) {
            mime = _("image/x-bmp");
        } else if (fname.GetExt() == _("jpg")) {
            mime = _("image/x-jpeg");
        } else if (fname.GetExt() == _("jpeg")) {
            mime = _("image/x-jpeg");
        } else if (fname.GetExt() == _("jpe")) {
            mime = _("image/x-jpeg");
        } else {
            mime = _("text/plain");
        }

        path += args;
#ifdef LWS_FEATURE_SERVE_HTTP_FILE_HAS_OTHER_HEADERS_ARG	
// This is fo version 1.3
        if (libwebsockets_serve_http_file(context,
                wsi,
                path.ToAscii(),
                mime.ToAscii(),
		NULL ) ) {
            
            return -1;
        }
#else
	if (libwebsockets_serve_http_file(context,
                wsi,
                path.ToAscii(),
                mime.ToAscii() ) ) {

            return -1;
        }
#endif

    }

        break;

    case LWS_CALLBACK_HTTP_FILE_COMPLETION:
        //		lwsl_info("LWS_CALLBACK_HTTP_FILE_COMPLETION seen\n");
        /* kill the connection after we sent one file */
        return -1;

        /*
         * callback for confirming to continue with client IP appear in
         * protocol 0 callback since no websocket protocol has been agreed
         * yet.  You can just ignore this if you won't filter on client IP
         * since the default uhandled callback return is 0 meaning let the
         * connection continue.
         */

    case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:

#if 1
        libwebsockets_get_peer_addresses(context, wsi, (int) (long) in, client_name,
                sizeof(client_name), client_ip, sizeof(client_ip));

        //fprintf(stderr, "Received network connect from %s (%s)\n",
        //        client_name, client_ip);
#endif

        /*  TODO
                libwebsockets_get_peer_addresses(m_context,
                    (int) (long) user,
                    client_name,
                    sizeof ( client_name),
                    client_ip,
                    sizeof ( client_ip));

                //fprintf(stderr,
                //    "Received network connect from %s (%s)\n",
                //    client_name, client_ip);
         */

        //syslog(LOG_ERR, "vscpd (ws): Received network connect from %s (%s)",
        //        client_name, client_ip);


        /* if we returned non-zero from here, we kill the connection */
        break;

    default:
        break;
    }

    return 0;
}


///////////////////////////////////////////////////////////////////////////////
// dump_handshake_info
//

/*
 * this is just an example of parsing handshake headers, you don't need this
 * in your code unless you will filter allowing connections by the header
 * content
 */

void
CControlObject::dump_handshake_info(struct lws_tokens *lwst)
{
    int n;
    static const char *token_names[ WSI_TOKEN_COUNT ] = {
        /*[WSI_TOKEN_GET_URI]		=*/ "GET URI",
        /*[WSI_TOKEN_HOST]			=*/ "Host",
        /*[WSI_TOKEN_CONNECTION]	=*/ "Connection",
        /*[WSI_TOKEN_KEY1]			=*/ "key 1",
        /*[WSI_TOKEN_KEY2]			=*/ "key 2",
        /*[WSI_TOKEN_PROTOCOL]		=*/ "Protocol",
        /*[WSI_TOKEN_UPGRADE]		=*/ "Upgrade",
        /*[WSI_TOKEN_ORIGIN]		=*/ "Origin",
        /*[WSI_TOKEN_DRAFT]			=*/ "Draft",
        /*[WSI_TOKEN_CHALLENGE]		=*/ "Challenge",

        /* new for 04 */
        /*[WSI_TOKEN_KEY]			=*/ "Key",
        /*[WSI_TOKEN_VERSION]		=*/ "Version",
        /*[WSI_TOKEN_SWORIGIN]		=*/ "Sworigin",

        /* new for 05 */
        /*[WSI_TOKEN_EXTENSIONS]	=*/ "Extensions",

        /* client receives these */
        /*[WSI_TOKEN_ACCEPT]		=*/ "Accept",
        /*[WSI_TOKEN_NONCE]			=*/ "Nonce",
        /*[WSI_TOKEN_HTTP]			=*/ "Http",
        /*[WSI_TOKEN_MUXURL]		=*/ "MuxURL",
    };

    for (n = 0; n < WSI_TOKEN_COUNT; n++) {

        if (lwst[n].token == NULL) continue;

        //fprintf(stderr, "    %s = %s\n", token_names[n], lwst[n].token);
    }
}

///////////////////////////////////////////////////////////////////////////////
// callback_dumb_increment
//

int
CControlObject::callback_dumb_increment(struct libwebsocket_context *context,
        struct libwebsocket *wsi,
        enum libwebsocket_callback_reasons reason,
        void *user,
        void *in,
        size_t len)
{
    int n;
    unsigned char buf[ LWS_SEND_BUFFER_PRE_PADDING + 512 +
            LWS_SEND_BUFFER_POST_PADDING ];
    unsigned char *p = &buf[ LWS_SEND_BUFFER_PRE_PADDING ];
    struct per_session_data__dumb_increment *pss =
            (struct per_session_data__dumb_increment *) user;

    switch (reason) {

    case LWS_CALLBACK_ESTABLISHED:
        //fprintf(stderr, "callback_dumb_increment: "
        //        "LWS_CALLBACK_ESTABLISHED\n");
        pss->number = 0;
        break;

        /*
         * in this protocol, we just use the broadcast action as the chance to
         * send our own connection-specific data and ignore the broadcast info
         * that is available in the 'in' parameter
         */

    case LWS_CALLBACK_SERVER_WRITEABLE:

        n = sprintf((char *) p, "%d", pss->number++);
        n = libwebsocket_write(wsi, p, n, LWS_WRITE_TEXT);

        if (n < 0) {
            syslog(LOG_ERR, "ERROR writing to socket");
            return 1;
        }

        if (gbClose && pss->number == 50) {
            //fprintf(stderr, "close testing limit, closing\n");
            lwsl_info("close tesing limit, closing\n");
            return -1;
        }
        break;

    case LWS_CALLBACK_RECEIVE:

        //fprintf(stderr, "rx %d\n", (int) len);

        if (len < 6)
            break;

        if (strcmp((char *) in, "reset\n") == 0)
            pss->number = 0;

        break;


    default:
        break;
    }

    return 0;
}


///////////////////////////////////////////////////////////////////////////////
// callback_lws_mirror
//

int
CControlObject::callback_lws_mirror(struct libwebsocket_context *context,
        struct libwebsocket *wsi,
        enum libwebsocket_callback_reasons reason,
        void *user,
        void *in,
        size_t len)
{
    int n;
    struct per_session_data__lws_mirror *pss = (per_session_data__lws_mirror *) user;

    switch (reason) {

    case LWS_CALLBACK_ESTABLISHED:

        //fprintf(stderr, "callback_lws_mirror: "
        //        "LWS_CALLBACK_ESTABLISHED\n");
        pss->ringbuffer_tail = mirrorws_ringbuffer_head;
        pss->wsi = wsi;
        break;

    case LWS_CALLBACK_SERVER_WRITEABLE:

        if (gbClose)
            break;

        // If there is data in the ringbuffer
        if (pss->ringbuffer_tail != mirrorws_ringbuffer_head) {

            // Write it out
            n = libwebsocket_write(wsi, (unsigned char *)
                    mirrorws_ringbuffer[ pss->ringbuffer_tail ].payload +
                    LWS_SEND_BUFFER_PRE_PADDING,
                    mirrorws_ringbuffer[ pss->ringbuffer_tail ].len,
                    LWS_WRITE_TEXT);
            if (n < 0) {
                syslog(LOG_ERR, "ERROR writing to socket");
            }

            if (pss->ringbuffer_tail == (MAX_MIRROR_MESSAGE_QUEUE - 1))
                pss->ringbuffer_tail = 0;
            else
                pss->ringbuffer_tail++;

            if (((mirrorws_ringbuffer_head - pss->ringbuffer_tail) %
                    MAX_MIRROR_MESSAGE_QUEUE) < (MAX_MIRROR_MESSAGE_QUEUE - 15))
                libwebsocket_rx_flow_control(wsi, 1);

            libwebsocket_callback_on_writable(context, wsi);

        }
        break;

    case LWS_CALLBACK_RECEIVE:

        if (mirrorws_ringbuffer[ mirrorws_ringbuffer_head ].payload)
            free(mirrorws_ringbuffer[ mirrorws_ringbuffer_head ].payload);

        mirrorws_ringbuffer[ mirrorws_ringbuffer_head ].payload =
                malloc(LWS_SEND_BUFFER_PRE_PADDING + len +
                LWS_SEND_BUFFER_POST_PADDING);
        mirrorws_ringbuffer[ mirrorws_ringbuffer_head ].len = len;

        memcpy((char *) mirrorws_ringbuffer[ mirrorws_ringbuffer_head ].payload +
                LWS_SEND_BUFFER_PRE_PADDING, in, len);

        if (mirrorws_ringbuffer_head == (MAX_MIRROR_MESSAGE_QUEUE - 1))
            mirrorws_ringbuffer_head = 0;
        else
            mirrorws_ringbuffer_head++;

        if (((mirrorws_ringbuffer_head - pss->ringbuffer_tail) %
                MAX_MIRROR_MESSAGE_QUEUE) > (MAX_MIRROR_MESSAGE_QUEUE - 10))
            libwebsocket_rx_flow_control(wsi, 0);

        libwebsocket_callback_on_writable_all_protocol(
                libwebsockets_get_protocol(wsi));
        break;
        /*
         * this just demonstrates how to use the protocol filter. If you won't
         * study and reject connections based on header content, you don't need
         * to handle this callback
         */

    case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
        dump_handshake_info((struct lws_tokens *) (long) user);
        /* you could return non-zero here and kill the connection */
        break;

    default:
        break;
    }

    return 0;
}


///////////////////////////////////////////////////////////////////////////////
// callback_lws_vscp
//

int
CControlObject::callback_lws_vscp(struct libwebsocket_context *context,
        struct libwebsocket *wsi,
        enum libwebsocket_callback_reasons reason,
        void *user,
        void *in,
        size_t len)
{
    wxString str;
    struct per_session_data__lws_vscp *pss = (per_session_data__lws_vscp *) user;

    switch (reason) {

        // after the server completes a handshake with
        // an incoming client
    case LWS_CALLBACK_ESTABLISHED:
    {
        //fprintf(stderr, "callback_lws_vscp: "
        //        "LWS_CALLBACK_ESTABLISHED\n");

        pss->wsi = wsi;
        // Create receive message list
        pss->pMessageList = new wxArrayString();
        // Create client
        pss->pClientItem = new CClientItem();
        // Clear filter
        clearVSCPFilter(&pss->pClientItem->m_filterVSCP);

        // Initialize session variables
        pss->bTrigger = false;
        pss->triggerTimeout = 0;

        // This is an active client
        pss->pClientItem->m_bOpen = false;
        pss->pClientItem->m_type = CLIENT_ITEM_INTERFACE_TYPE_CLIENT_INTERNAL;
        pss->pClientItem->m_strDeviceName = _("Internal daemon websocket client. Started at ");
        wxDateTime now = wxDateTime::Now();
        pss->pClientItem->m_strDeviceName += now.FormatISODate();
        pss->pClientItem->m_strDeviceName += _(" ");
        pss->pClientItem->m_strDeviceName += now.FormatISOTime();

        // Add the client to the Client List
        gpctrlObj->m_wxClientMutex.Lock();
        gpctrlObj->addClient(pss->pClientItem);
        gpctrlObj->m_wxClientMutex.Unlock();
    }
        break;

        // when the websocket session ends
    case LWS_CALLBACK_CLOSED:

        // Remove the receive message list
        if (NULL == pss->pMessageList) {
            pss->pMessageList->Clear();
            delete pss->pMessageList;
        }

        // Remove the client
        gpctrlObj->m_wxClientMutex.Lock();
        gpctrlObj->removeClient(pss->pClientItem);
        gpctrlObj->m_wxClientMutex.Unlock();
        //delete pss->pClientItem;
        pss->pClientItem = NULL;
        break;

        // data has appeared for this server endpoint from a
        // remote client, it can be found at *in and is
        // len bytes long
    case LWS_CALLBACK_RECEIVE:
        gpctrlObj->handleWebSocketReceive(context, wsi, pss, in, len);
        break;

        // signal to send to client (you would use
        // libwebsocket_write() taking care about the
        // special buffer requirements
        /*           
            case LWS_CALLBACK_BROADCAST:
                libwebsocket_callback_on_writable(context, wsi);
                break;
         */
        // If you call
        // libwebsocket_callback_on_writable() on a connection, you will
        // get one of these callbacks coming when the connection socket
        // is able to accept another write packet without blocking.
        // If it already was able to take another packet without blocking,
        // you'll get this callback at the next call to the service loop
        // function. 
    case LWS_CALLBACK_SERVER_WRITEABLE:
    {
        // If there is data to write
        if (pss->pMessageList->GetCount()) {

            str = pss->pMessageList->Item(0);
            pss->pMessageList->RemoveAt(0);

            // Write it out
            unsigned char buf[ 512 ];
            memset((char *) buf, 0, sizeof( buf));
            strcpy((char *) buf, (const char*) str.mb_str(wxConvUTF8));
            int n = libwebsocket_write(wsi,
                    buf,
                    strlen((char *) buf),
                    LWS_WRITE_TEXT);
            if (n < 0) {
                syslog(LOG_ERR, "ERROR writing to socket");
            }

            libwebsocket_callback_on_writable(context, wsi);

        }// Check if there is something to send out from 
            // the event list.
        else if (pss->pClientItem->m_bOpen &&
                pss->pClientItem->m_clientInputQueue.GetCount()) {

            CLIENTEVENTLIST::compatibility_iterator nodeClient;
            vscpEvent *pEvent;

            pss->pClientItem->m_mutexClientInputQueue.Lock();
            nodeClient = pss->pClientItem->m_clientInputQueue.GetFirst();
            pEvent = nodeClient->GetData();
            pss->pClientItem->m_clientInputQueue.DeleteNode(nodeClient);
            pss->pClientItem->m_mutexClientInputQueue.Unlock();

            if (NULL != pEvent) {

                if (doLevel2Filter(pEvent, &pss->pClientItem->m_filterVSCP)) {

                    if (writeVscpEventToString(pEvent, str)) {

                        // Write it out
                        char buf[ 512 ];
                        memset((char *) buf, 0, sizeof( buf));
                        strcpy((char *) buf, (const char*) "E;");
                        strcat((char *) buf, (const char*) str.mb_str(wxConvUTF8));
                        int n = libwebsocket_write(wsi, (unsigned char *)
                                buf,
                                strlen((char *) buf),
                                LWS_WRITE_TEXT);
                        if (n < 0) {
                            syslog(LOG_ERR, "ERROR writing to socket");
                        }
                    }
                }

                // Remove the event
                deleteVSCPevent(pEvent);

            } // Valid pEvent pointer

            libwebsocket_callback_on_writable(context, wsi);
        }
    }
        break;

    default:
        break;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// handleWebSocketReceive
//

void
CControlObject::handleWebSocketReceive(struct libwebsocket_context *context,
        struct libwebsocket *wsi,
        struct per_session_data__lws_vscp *pss,
        void *in,
        size_t len)
{
    wxString str;
    char buf[ 512 ];
    const char *p = buf;

    memset(buf, 0, sizeof( buf));
    memcpy(buf, (char *) in, len);

    switch (*p) {

        // Command - | 'C' | command type (byte) | data |
    case 'C':
        p++;
        p++; // Point beyond initial info "C;"
        handleWebSocketCommand(context,
                                wsi,
                                pss,
                                p);
        break;

        // Event | 'E' ; head(byte) , vscp_class(unsigned short) , vscp_type(unsigned
        //					short) , GUID(16*byte), data(0-487 bytes) |
    case 'E':
    {
        p++;
        p++; // Point beyond initial info "E;"
        vscpEvent vscp_event;
        str = wxString::FromAscii(p);
        if (getVscpEventFromString(&vscp_event, str)) {

            vscp_event.obid = pss->pClientItem->m_clientID;
            if (handleWebSocketSendEvent(&vscp_event)) {
                pss->pMessageList->Add(_("+;EVENT"));
            } else {
                pss->pMessageList->Add(_("-;3;Transmit buffer full"));
            }
        }
    }
        break;

        // Unknow command
    default:
        break;

    }

}

///////////////////////////////////////////////////////////////////////////////
// handleWebSocketSendEvent
//

bool
CControlObject::handleWebSocketSendEvent(vscpEvent *pEvent)
{
    bool bSent = false;
    bool rv = true;

    // Level II events betwen 512-1023 is recognized by the daemon and 
    // sent to the correct interface as Level I events if the interface  
    // is addressed by the client.
    if ((pEvent->vscp_class <= 1023) &&
            (pEvent->vscp_class >= 512) &&
            (pEvent->sizeData >= 16)) {

        // This event shold be sent to the correct interface if it is
        // available on this machine. If not it should be sent to 
        // the rest of the network as normal

        cguid destguid;
        destguid.getFromArray(pEvent->pdata);
        destguid.setAt(0,0);
        destguid.setAt(1,0);
        //unsigned char destGUID[16];
        //memcpy(destGUID, pEvent->pdata, 16); // get destination GUID
        //destGUID[0] = 0; // Interface GUID's have LSB bytes nilled
        //destGUID[1] = 0;

        m_wxClientMutex.Lock();

        // Find client
        CClientItem *pDestClientItem = NULL;
        VSCPCLIENTLIST::iterator iter;
        for (iter = m_clientList.m_clientItemList.begin();
                iter != m_clientList.m_clientItemList.end();
                ++iter) {

            CClientItem *pItem = *iter;
            if ( pItem->m_guid == destguid ) {
                // Found
                pDestClientItem = pItem;
                break;
            }

        }

        if (NULL != pDestClientItem) {

            // Check if filtered out
            if (doLevel2Filter(pEvent, &pDestClientItem->m_filterVSCP)) {

                // If the client queue is full for this client then the
                // client will not receive the message
                if (pDestClientItem->m_clientInputQueue.GetCount() <=
                        m_maxItemsInClientReceiveQueue) {

                    // Create copy of event
                    vscpEvent *pnewEvent = new vscpEvent;
                    if (NULL != pnewEvent) {

                        copyVSCPEvent(pnewEvent, pEvent);

                        // Add the new event to the inputqueue
                        pDestClientItem->m_mutexClientInputQueue.Lock();
                        pDestClientItem->m_clientInputQueue.Append(pEvent);
                        pDestClientItem->m_semClientInputQueue.Post();
                        pDestClientItem->m_mutexClientInputQueue.Unlock();
                    }

                    bSent = true;

                } else {
                    // Overun - No room for event
                    deleteVSCPevent(pEvent);
                    bSent = true;
                    rv = false;
                }

            } // filter

        } // Client found

        m_wxClientMutex.Unlock();

    }

    if (!bSent) {

        // There must be room in the send queue
        if (m_maxItemsInClientReceiveQueue >
                m_clientOutputQueue.GetCount()) {

            // Create copy of event
            vscpEvent *pnewEvent = new vscpEvent;
            if (NULL != pnewEvent) {

                copyVSCPEvent(pnewEvent, pEvent);

                m_mutexClientOutputQueue.Lock();
                m_clientOutputQueue.Append(pnewEvent);
                m_semClientOutputQueue.Post();
                m_mutexClientOutputQueue.Unlock();
            }

        } else {
            deleteVSCPevent(pEvent);
            rv = false;
        }
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// handleWebSocketCommand
//

void
CControlObject::handleWebSocketCommand(struct libwebsocket_context *context,
                                        struct libwebsocket *wsi,
                                        struct per_session_data__lws_vscp *pss,
                                        const char *pCommand)
{
    wxString strTok;
    wxString str = wxString::FromAscii(pCommand);

    // Check pointer
    if (NULL == pCommand) return;

    wxStringTokenizer tkz(str, _(";"));

    // Get command
    if (tkz.HasMoreTokens()) {
        strTok = tkz.GetNextToken();
        strTok.MakeUpper();
        //pEvent->head = readStringValue( str );
    } else {
        pss->pMessageList->Add(_("-;1;Syntax error"));
        return;
    }

    if (0 == strTok.Find(_("NOOP"))) {
        pss->pMessageList->Add(_("+;NOOP"));
    } else if (0 == strTok.Find(_("OPEN"))) {
        pss->pClientItem->m_bOpen = true;
        pss->pMessageList->Add(_("+;OPEN"));
    } else if (0 == strTok.Find(_("CLOSE"))) {
        pss->pClientItem->m_bOpen = false;
        pss->pMessageList->Add(_("+;CLOSE"));
    } else if (0 == strTok.Find(_("SETFILTER"))) {

        unsigned char ifGUID[ 16 ];
        memset(ifGUID, 0, 16);

        // Get filter
        if (tkz.HasMoreTokens()) {
            strTok = tkz.GetNextToken();
            if (!readFilterFromString(&pss->pClientItem->m_filterVSCP,
                    strTok)) {
                pss->pMessageList->Add(_("-;1;Syntax error"));
                return;
            }
        } else {
            pss->pMessageList->Add(_("-;1;Syntax error"));
            return;
        }

        // Get mask
        if (tkz.HasMoreTokens()) {
            strTok = tkz.GetNextToken();
            if (!readMaskFromString(&pss->pClientItem->m_filterVSCP,
                    strTok)) {
                pss->pMessageList->Add(_("-;1;Syntax error"));
                return;
            }
        } else {
            pss->pMessageList->Add(_("-;1;Syntax error"));
            return;
        }

        // Positive response
        pss->pMessageList->Add(_("+;SETFILTER"));

    }// Clear the event queue
    else if (0 == strTok.Find(_("CLRQUE"))) {

        CLIENTEVENTLIST::iterator iterVSCP;

        pss->pClientItem->m_mutexClientInputQueue.Lock();
        for (iterVSCP = pss->pClientItem->m_clientInputQueue.begin();
                iterVSCP != pss->pClientItem->m_clientInputQueue.end(); ++iterVSCP) {
            vscpEvent *pEvent = *iterVSCP;
            deleteVSCPevent(pEvent);
        }

        pss->pClientItem->m_clientInputQueue.Clear();
        pss->pClientItem->m_mutexClientInputQueue.Unlock();

        pss->pMessageList->Add(_("+;CLRQUE"));
    } else if (0 == strTok.Find(_("WRITEVAR"))) {

        // Get variablename
        if (tkz.HasMoreTokens()) {

            CVSCPVariable *pvar;
            strTok = tkz.GetNextToken();
            if (NULL == (pvar = m_VSCP_Variables.find(strTok))) {
                pss->pMessageList->Add(_("-;5;Unable to find variable"));
                return;
            }

            // Get variable value
            if (tkz.HasMoreTokens()) {
                strTok = tkz.GetNextToken();
                if (!pvar->setValueFromString(pvar->getType(), strTok)) {
                    pss->pMessageList->Add(_("-;1;Syntax error"));
                    return;
                }
            } else {
                pss->pMessageList->Add(_("-;1;Syntax error"));
                return;
            }
        } else {
            pss->pMessageList->Add(_("-;1;Syntax error"));
            return;
        }

        // Positive reply
        pss->pMessageList->Add(_("+;WRITEVAR"));

    } else if (0 == strTok.Find(_("ADDVAR"))) {

        wxString name;
        wxString value;
        uint8_t type = VSCP_DAEMON_VARIABLE_CODE_STRING;
        bool bPersistent = false;

        // Get variable name
        if (tkz.HasMoreTokens()) {
            name = tkz.GetNextToken();
        } else {
            pss->pMessageList->Add(_("-;1;Syntax error"));
            return;
        }

        // Get variable value
        if (tkz.HasMoreTokens()) {
            value = tkz.GetNextToken();
        } else {
            pss->pMessageList->Add(_("-;1;Syntax error"));
            return;
        }

        // Get variable type
        if (tkz.HasMoreTokens()) {
            type = readStringValue(tkz.GetNextToken());
        }

        // Get variable Persistent
        if (tkz.HasMoreTokens()) {
            int val = readStringValue(tkz.GetNextToken());
        }

        // Add the variable
        if (!m_VSCP_Variables.add(name, value, type, bPersistent)) {
            pss->pMessageList->Add(_("-;1;Syntax error"));
            return;
        } else {
            pss->pMessageList->Add(_("-;1;Syntax error"));
            return;
        }

        pss->pMessageList->Add(_("+;ADDVAR"));
        
    } else if (0 == strTok.Find(_("READVAR"))) {

        CVSCPVariable *pvar;
        uint8_t type;
        wxString strvalue;

        strTok = tkz.GetNextToken();
        if (NULL == (pvar = m_VSCP_Variables.find(strTok))) {
            pss->pMessageList->Add(_("-;5;Unable to find variable"));
            return;
        }

        pvar->writeVariableToString(strvalue);
        type = pvar->getType();

        wxString resultstr = _("+;READVAR;");
        resultstr += wxString::Format(_("%d"), type);
        resultstr += _(";");
        resultstr += strvalue;
        pss->pMessageList->Add(resultstr);

    } else if (0 == strTok.Find(_("SAVEVAR"))) {

        if (!m_VSCP_Variables.save()) {
            pss->pMessageList->Add(_("-;1;Syntax error"));
            return;
        }

        pss->pMessageList->Add(_("+;SAVEVAR"));
    } else {
        pss->pMessageList->Add(_("-;2;Unknown command"));
    }

}

#endif






///////////////////////////////////////////////////////////////////////////////
//                              WEB SERVER
///////////////////////////////////////////////////////////////////////////////



#ifdef WIN32
#else




///////////////////////////////////////////////////////////////////////////////
// file_reader
//

ssize_t
CControlObject::websrv_callback_file_free (void *cls, uint64_t pos, char *buf, size_t max)
{
  FILE *file = (FILE *)cls;

  (void)  fseek (file, pos, SEEK_SET);
  return fread (buf, 1, max, file);
}

///////////////////////////////////////////////////////////////////////////////
// websrv_callback_file_free
//

void
CControlObject::websrv_callback_file_free (void *cls)
{
  FILE *file = (FILE *)cls;
  fclose (file);
}


///////////////////////////////////////////////////////////////////////////////
// websrv_callback_check_address
//

int CControlObject::websrv_callback_check_address( void *cls,
                                                    const struct sockaddr *addr,
                                                    socklen_t addrlen )
{
    CControlObject *pObject = (CControlObject *)cls;
    int ret = MHD_YES;  // TODO
    
    // Check if this user is allowed to connect from this location
    pObject->m_mutexUserList.Lock();
    //bool bValidHost = 
    //        pObject->m_userList.checkRemote( m_pUserItem, 
    //                                            remoteaddr.IPAddress() );
    pObject->m_mutexUserList.Unlock();
    
    return ret;
}


///////////////////////////////////////////////////////////////////////////////
// callback_webpage
//

int CControlObject::websrv_callback_webpage(void *cls,
                                                struct MHD_Connection *connection,
                                                const char *url,
                                                const char *method,
                                                const char *version,
                                                const char *upload_data,
                                                size_t *upload_data_size,
                                                void **ptr)
{
    wxString strUser, strPassword;
    CControlObject *pObject = (CControlObject *)cls;
    const char *defaultPage = WEBSERVER_PAGE;
    struct MHD_Response *response;
    int ret;
    char *user;
    char *pass;
    bool bFail;
    struct websrv_Request *request;
    struct websrv_Session *session;
    unsigned int i;
    

    user = MHD_basic_auth_get_username_password(connection, &pass);
    
    if ( NULL != user ) {
        strUser = wxString::FromAscii(user);
        delete user;
        user = NULL;
    }
    
    if ( strUser.Length() && ( NULL != pass ) ) {
        Cmd5 md5;
        strPassword = wxString::FromAscii( md5.digest((unsigned char *)pass) );
        delete pass;
        pass = NULL;
    }

    bFail = ( 0 == strUser.Length() || 
             ( NULL == pObject->m_userList.checkUser(strUser,strPassword)));

    if ( bFail ) {
        
        // Send fail response
        response = 
            MHD_create_response_from_buffer( strlen(WEBSERVER_DENIED),
                                                (void *) WEBSERVER_DENIED,
                                                MHD_RESPMEM_PERSISTENT);
        
        ret = MHD_queue_basic_auth_fail_response( connection,
                                                    "VSCP Daemon",
                                                    response);
        
        return ret;
    }
    
    request = (struct websrv_Request *)*ptr;
    if (NULL == request) {
        
        // Allocate request structure on first call
        request = (struct websrv_Request *)calloc(1, sizeof(struct websrv_Request));
        if (NULL == request) {
            syslog(LOG_ERR, "calloc error: %s\n", strerror(errno));
            return MHD_NO;
        }
        
        *ptr = request;
        
        if ( 0 == strcmp( method, MHD_HTTP_METHOD_POST ) ) {
            
            // Assign post processor iterator
            request->pp = MHD_create_post_processor( connection, 
                                                        1024,
                                                        &websrv_post_iterator, 
                                                        request);
            
            if ( NULL == request->pp ) {
                syslog(LOG_ERR, 
                        "Failed to setup post processor for `%s'\n",
                        url);
                return MHD_NO; // internal error 
            }
            
        }
        else if ( 0 != strcmp(method, MHD_HTTP_METHOD_GET) ) {
            
            // Assign post processor iterator
            request->pp = MHD_create_post_processor( connection, 
                                                        1024,
                                                        &websrv_post_iterator, 
                                                        request);
            
            if ( NULL == request->pp ) {
                syslog(LOG_ERR, 
                        "Failed to setup post processor for `%s'\n",
                        url);
                return MHD_NO; // internal error 
            }
            
        }
        
        return MHD_YES;
    }

    // Get/create session
    if (NULL == request->session) {
        
        request->session = websrv_get_session( connection );
        
        if (NULL == request->session) {
            syslog(LOG_ERR, "Failed to setup session for `%s'\n", url);
            return MHD_NO; // internal error 
        }
        
    }
    
    session = request->session;
    session->start = time(NULL);
    
    if (0 == strcmp(method, MHD_HTTP_METHOD_POST)) {
        
        // evaluate POST data 
        MHD_post_process(request->pp,
                            upload_data,
                            *upload_data_size);
        
        if (0 != *upload_data_size) {
            *upload_data_size = 0;
            return MHD_YES;
        }
        
        // done with POST data, serve response 
        MHD_destroy_post_processor(request->pp);
        request->pp = NULL;
        
        method = MHD_HTTP_METHOD_GET; // fake 'GET' 
        
        if (NULL != request->post_url) {
            url = request->post_url;
        }
    }

    if ((0 == strcmp(method, MHD_HTTP_METHOD_GET)) ||
            (0 == strcmp(method, MHD_HTTP_METHOD_HEAD))) {

        // find out which page among the stock functionality to serve 
        i = 0;
        while ((pages[i].url != NULL) && (0 != strcmp(pages[i].url, url))) {
            i++;
        }

        if (NULL != pages[i].url) {
            ret = pages[i].handler( 
                    ( NULL != pages[i].handler_cls ) ? pages[i].handler_cls : pObject,
                    pages[i].mime,
                    session, connection);
        } 
        else {

            FILE *file;
            struct stat buf;
            
            wxString strPath2File; 
            strPath2File = pObject->m_pathRoot;
            strPath2File += _("/");
            strPath2File += wxString::FromAscii( &url[1] );

            char *p = new char[strPath2File.Length() + 1];
            
            if ( NULL != p ) {
                
                memset( p, 0, strPath2File.Length() + 1 );
                memcpy( p, strPath2File.ToAscii(), strPath2File.Length() );
            
                if ( 0 == stat( p, &buf ) ) {
                    file = fopen( p, "rb" );
                } 
                else {
                    file = NULL;
                }
            
                // Remove allocated string
                delete [] p;
            }
            
            wxFileName xfile(strPath2File);
            wxString file_extension = xfile.GetExt();
            wxString mimetype = pObject->m_hashMimeTypes[file_extension];

            if (file == NULL) {
                response =
                        MHD_create_response_from_buffer( strlen(WEBSERVER_NOT_FOUND_ERROR),
                        (void *) WEBSERVER_NOT_FOUND_ERROR,
                        MHD_RESPMEM_PERSISTENT);
                
                ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
            }
            else {
                
                response =
                        MHD_create_response_from_callback( buf.st_size,
                                                32 * 1024, /* 32k page size */
                                                &websrv_callback_file_free,
                                                file,
                                                &websrv_callback_file_free);
                if (response == NULL) {
                    fclose(file);
                    return MHD_NO;
                }

                //ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
                char *p = new char[mimetype.Length() + 1];
                if ( NULL != p ) {
                    
                    memset( p, 0, mimetype.Length() + 1 );
                    memcpy( p, mimetype.ToAscii(), mimetype.Length() );
                    
                    MHD_add_response_header( response,
                                                MHD_HTTP_HEADER_CONTENT_ENCODING,
                                                p );
                
                    ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
                    
                    MHD_destroy_response(response);
                    
                    // Remove allocated string
                    delete [] p;
                    
                }
            }

        }

        if (ret != MHD_YES) {
            syslog(LOG_ERR, "Failed to create page for `%s'\n", url);
        }

    }

    //*ptr = NULL; // reset when done 
    
    return ret;
}

///////////////////////////////////////////////////////////////////////////////
// get_session
//

int
CControlObject::websrv_callback_file(void *cls,
                                        struct MHD_Connection *connection,
                                        const char *url,
                                        const char *method,
                                        const char *version,
                                        const char *upload_data,
                                        size_t *upload_data_size, 
                                        void **ptr)
{
    static int aptr;
    struct MHD_Response *response;
    int ret;
    FILE *file;
    struct stat buf;

    if (0 != strcmp(method, MHD_HTTP_METHOD_GET)) {
        return MHD_NO;  // unexpected method 
    }
    
    if (&aptr != *ptr) {
        // do never respond on first call 
        *ptr = &aptr;
        return MHD_YES;
    }
    
    *ptr = NULL; // reset when done 
    
    if (0 == stat(&url[1], &buf)) {
        file = fopen(&url[1], "rb");
    }
    else {
        file = NULL;
    }
    
    if (file == NULL) {
        response = 
          MHD_create_response_from_buffer(strlen(WEBSERVER_NOT_FOUND_ERROR),
                (void *)WEBSERVER_NOT_FOUND_ERROR,
                MHD_RESPMEM_PERSISTENT);
        ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
        MHD_destroy_response(response);
    } 
    else {
        response = 
          MHD_create_response_from_callback( buf.st_size, 
                                                32 * 1024, /* 32k page size */
                                                &websrv_callback_file_free,
                                                file,
                                                &websrv_callback_file_free);
        if (response == NULL) {
            fclose(file);
            return MHD_NO;
        }
        
        ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
    }
    
    return ret;
}

///////////////////////////////////////////////////////////////////////////////
// get_session
//

struct websrv_Session *
CControlObject::websrv_get_session( struct MHD_Connection *connection )
{
    struct websrv_Session *ret;
    const char *cookie;

    cookie = MHD_lookup_connection_value( connection,
                                            MHD_COOKIE_KIND,
                                            WEBSERVER_COOKIE_NAME);
    
    if (cookie != NULL) {
        
        // find existing session 
        ret = websrv_sessions;
        while (NULL != ret) {
            if (0 == strcmp(cookie, ret->m_sid))
                break;
            ret = ret->m_next;
        }
        
        if (NULL != ret) {
            ret->m_referenceCount++;
            return ret;
        }
    }
    
    // create fresh session 
    ret = (struct websrv_Session *)calloc(1, sizeof(struct websrv_Session));
    if (NULL == ret) {
        syslog(LOG_ERR, "calloc error: %s\n", strerror(errno));
        return NULL;
    }
    
    // Generate a random session ID
    time_t t;
    t = time( NULL );
    snprintf(ret->m_sid,
            sizeof(ret->m_sid),
            "__VSCP__DAEMON_%X%X%X%X_be_hungry_stay_foolish_%X%X",
            (unsigned int)random(),
            (unsigned int)random(),
            (unsigned int)random(),
            (unsigned int)t,
            (unsigned int)random(), 1337 );
    
    ret->m_referenceCount++;
    ret->start = time(NULL);
    ret->m_next = websrv_sessions;
    websrv_sessions = ret;
    
    return ret;
}


///////////////////////////////////////////////////////////////////////////////
// add_session_cookie
//

void
CControlObject::websrv_add_session_cookie(struct websrv_Session *session,
                                            struct MHD_Response *response)
{
    char cstr[256];
    snprintf(cstr,
            sizeof(cstr),
            "%s=%s",
            WEBSERVER_COOKIE_NAME,
            session->m_sid);
    
    if (MHD_NO ==
            MHD_add_response_header(response,
            MHD_HTTP_HEADER_SET_COOKIE,
            cstr)) {
        syslog(LOG_ERR,
                "Failed to set session cookie header!\n");
    }
}

///////////////////////////////////////////////////////////////////////////////
// expire_sessions
//

void
CControlObject::websrv_expire_sessions(void)
{
    struct websrv_Session *pos;
    struct websrv_Session *prev;
    struct websrv_Session *next;
    time_t now;

    now = time( NULL );
    prev = NULL;
    pos = websrv_sessions;
    
    while (NULL != pos) {
        
        next = pos->m_next;
        
        if (now - pos->start > 60 * 60) {
        
            // expire sessions after 1h 
            if ( NULL == prev ) {
                websrv_sessions = pos->m_next;
            }
            else {
                prev->m_next = next;
            }
            
            free(pos);
            
        } 
        else {
            prev = pos;
        }
        
        pos = next;
    }
}

///////////////////////////////////////////////////////////////////////////////
// not_found_page
//

int
CControlObject::websrv_not_found_page(const void *cls,
                                        const char *mime,
                                        struct websrv_Session *session,
                                        struct MHD_Connection *connection )
{
    int ret;
    struct MHD_Response *response;

    // unsupported HTTP method 
    response = MHD_create_response_from_buffer( strlen(WEBSERVER_NOT_FOUND_ERROR),
                                                    (void *)WEBSERVER_NOT_FOUND_ERROR,
                                                    MHD_RESPMEM_PERSISTENT);
    
    ret = MHD_queue_response( connection,
                                MHD_HTTP_NOT_FOUND,
                                response);
    
    MHD_add_response_header( response,
                                MHD_HTTP_HEADER_CONTENT_ENCODING,
                                mime);
    
    MHD_destroy_response(response);
    
    return ret;
}

///////////////////////////////////////////////////////////////////////////////
// serve_simple_page
//

int
CControlObject::websrv_serve_simple_page(const void *cls,
                                            const char *mime,
                                            struct websrv_Session *session,
                                            struct MHD_Connection *connection)
{
    int ret;
    const char *page = (const char *)cls;
    struct MHD_Response *response;

    // return static page 
    response = MHD_create_response_from_buffer( strlen(page),
                                                    (void *)page,
                                                    MHD_RESPMEM_PERSISTENT);
    
    websrv_add_session_cookie( session, response);
    
    MHD_add_response_header( response,
                                MHD_HTTP_HEADER_CONTENT_ENCODING,
                                mime);
    
    ret = MHD_queue_response( connection,
                                MHD_HTTP_OK,
                                response);
    
    MHD_destroy_response(response);
    
    return ret;
}

///////////////////////////////////////////////////////////////////////////////
// websrv_serve_mainpage
//

int
CControlObject::websrv_serve_mainpage( const void *cls,
                                            const char *mime,
                                            struct websrv_Session *session,
                                            struct MHD_Connection *connection)
{
    int ret;
    CControlObject *pObject = (CControlObject *) cls;
    struct MHD_Response *response;

    // Get connection type
    const MHD_ConnectionInfo *pProtocolInfo = 
            MHD_get_connection_info(connection, 
                                        MHD_CONNECTION_INFO_PROTOCOL );
    // Get hostname
    wxString strHost = _("http://localhost:8080");
    const char *str_host = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "host");
    if ( NULL != str_host ) {
        strHost = wxString::FromAscii(str_host);
        if ( NULL != pProtocolInfo ) {
            strHost = _("https://") + strHost;
        }
        else {
            strHost = _("http://") + strHost;
        }
    }

    wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Control"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    // Insert server url into navigation menu 
    wxString navstr = _(WEB_COMMON_MENU);  
    int pos;
    while ( wxNOT_FOUND != ( pos = navstr.Find(_("%s")))) {
        buildPage += navstr.Left( pos );
        navstr = navstr.Right(navstr.Length() - pos - 2);
    }
    buildPage += navstr;
    
    buildPage += _("<span align=\"center\">");
    buildPage += _("<h4> Welcome to the VSCP daemon control interface.</h4>");
    buildPage += _("</span>");
    buildPage += _("<span style=\"text-indent:50px;\"><p>");
    buildPage += _(" <b>Version:</b> ");
    buildPage += _(VSCPD_DISPLAY_VERSION);
    buildPage += _("</p><p>");
    buildPage += _(VSCPD_COPYRIGHT);
    buildPage += _("</p></span>");
            
    buildPage += _(WEB_COMMON_END);     // Common end code
    
    char *ppage = new char[ buildPage.Length() + 1 ];
    memset(ppage, 0, buildPage.Length() + 1 );
    memcpy( ppage, buildPage.ToAscii(), buildPage.Length() );        
    
    // return page 
    response = MHD_create_response_from_buffer( strlen(ppage),
                                                    (void *)ppage,
                                                    MHD_RESPMEM_MUST_FREE );
    
    websrv_add_session_cookie(session, response);
    
    MHD_add_response_header( response,
                                MHD_HTTP_HEADER_CONTENT_ENCODING,
                                mime);
    
    ret = MHD_queue_response( connection,
                                MHD_HTTP_OK,
                                response);
    
    MHD_destroy_response( response );
    
    return ret;
}

///////////////////////////////////////////////////////////////////////////////
// websrv_serve_interfaces
//

int
CControlObject::websrv_serve_interfaces( const void *cls,
                                            const char *mime,
                                            struct websrv_Session *session,
                                            struct MHD_Connection *connection)
{
    int ret;
    CControlObject *pObject = (CControlObject *) cls;
    struct MHD_Response *response;

    // Get connection type
    const MHD_ConnectionInfo *pProtocolInfo = 
            MHD_get_connection_info(connection, 
                                        MHD_CONNECTION_INFO_PROTOCOL );
    // Get hostname
    wxString strHost = _("http://localhost:8080");
    const char *str_host = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "host");
    if ( NULL != str_host ) {
        strHost = wxString::FromAscii(str_host);
        if ( NULL != pProtocolInfo ) {
            strHost = _("https://") + strHost;
        }
        else {
            strHost = _("http://") + strHost;
        }
    }

    wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Interfaces"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    // Insert server url into navigation menu 
    wxString navstr = _(WEB_COMMON_MENU);
    int pos;
    while ( wxNOT_FOUND != ( pos = navstr.Find(_("%s")))) {
        buildPage += navstr.Left( pos );
        navstr = navstr.Right(navstr.Length() - pos - 2);
    }
    buildPage += navstr;
    
    buildPage += _(WEB_IFLIST_BODY_START);
    buildPage += _(WEB_IFLIST_TR_HEAD);

    wxString strGUID;  
    wxString strBuf;

    // Display Interface List
    pObject->m_wxClientMutex.Lock();
    VSCPCLIENTLIST::iterator iter;
    for (iter = pObject->m_clientList.m_clientItemList.begin();
            iter != pObject->m_clientList.m_clientItemList.end();
            ++iter) {

        CClientItem *pItem = *iter;
        pItem->m_guid.toString(strGUID);

        buildPage += _(WEB_IFLIST_TR);

        // Client id
        buildPage += _(WEB_IFLIST_TD_CENTERED);
        buildPage += wxString::Format(_("%d"), pItem->m_clientID);
        buildPage += _("</td>");

        // Interface type
        buildPage += _(WEB_IFLIST_TD_CENTERED);
        buildPage += wxString::Format(_("%d"), pItem->m_type);
        buildPage += _("</td>");

        // GUID
        buildPage += _(WEB_IFLIST_TD_GUID);
        buildPage += strGUID.Left(23);
        buildPage += _("<br>");
        buildPage += strGUID.Right(23);
        buildPage += _("</td>");

        // Interface name
        buildPage += _("<td>");
        buildPage += pItem->m_strDeviceName.Left(pItem->m_strDeviceName.Length() - 30);
        buildPage += _("</td>");

        // Start date
        buildPage += _("<td>");
        buildPage += pItem->m_strDeviceName.Right(19);
        buildPage += _("</td>");

        buildPage += _("</tr>");

    }
    
    pObject->m_wxClientMutex.Unlock();
    
    buildPage += _(WEB_IFLIST_TABLE_END);
    
    buildPage += _("<br>All interfaces to the daemon is listed here. This is drivers as well as clients on one of the daemons interfaces. It is possible to see events coming in on a on a specific interface and send events on just one of the interfaces. This is mostly used on the driver interfaces but is possible on all interfacs<br>");
    
    buildPage += _("<br><b>Interface Types</b><br>");
    buildPage += _("0 - Unknown (you should not see this).<br>");
    buildPage += _("1 - Internal daemon client.<br>");
    buildPage += _("2 - Level I (CANAL) Driver.<br>");
    buildPage += _("3 - Level II Driver.<br>");
    buildPage += _("4 - TCP/IP Client.<br>");

    buildPage += _(WEB_COMMON_END);     // Common end code
    
    char *ppage = new char[ buildPage.Length() + 1 ];
    memset(ppage, 0, buildPage.Length() + 1 );
    memcpy( ppage, buildPage.ToAscii(), buildPage.Length() );        
    
    // return page 
    response = MHD_create_response_from_buffer( strlen(ppage),
                                                    (void *)ppage,
                                                    MHD_RESPMEM_MUST_FREE );
    
    websrv_add_session_cookie(session, response);
    
    MHD_add_response_header( response,
                                MHD_HTTP_HEADER_CONTENT_ENCODING,
                                mime);
    
    ret = MHD_queue_response( connection,
                                MHD_HTTP_OK,
                                response);
    
    MHD_destroy_response( response );
    
    return ret;
}

///////////////////////////////////////////////////////////////////////////////
// websrv_serve_dmlist
//

int
CControlObject::websrv_serve_dmlist( const void *cls,
                                            const char *mime,
                                            struct websrv_Session *session,
                                            struct MHD_Connection *connection)
{
    int ret;
    VSCPInformation vscpinfo;
    CControlObject *pObject = (CControlObject *) cls;
    struct MHD_Response *response;
    long upperLimit = 50;
    
    // Get connection type
    const MHD_ConnectionInfo *pProtocolInfo = 
            MHD_get_connection_info(connection, 
                                        MHD_CONNECTION_INFO_PROTOCOL );
    // Get hostname
    wxString strHost = _("http://localhost:8080");
    const char *str_host = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "host");
    if ( NULL != str_host ) {
        strHost = wxString::FromAscii(str_host);
        if ( NULL != pProtocolInfo ) {
            strHost = _("https://") + strHost;
        }
        else {
            strHost = _("http://") + strHost;
        }
    }
          
    // light
    bool bLight = false;
    const char *str_light = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "light");
    if ( (NULL != str_light) && NULL != strstr("true",str_light) ) bLight = true;
    
    // From
    long nFrom = 0;
    const char *str_from = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "from");
    if ( NULL != str_from ) nFrom = atoi(str_from);
    // Check limits
    if (nFrom > pObject->m_dm.getRowCount()) nFrom = 0;
    
    // Count
    uint16_t nCount = 50;
    const char *str_count = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "count");
    if ( NULL != str_count ) nCount = atoi(str_count);
    // Check limits
    if ((nFrom+nCount) > pObject->m_dm.getRowCount()) {
        upperLimit = pObject->m_dm.getRowCount()-nFrom;
    }
    else {
        upperLimit = nFrom+nCount;
    }
    
    // Navigation button
    const char *str_navbtn = 
                    MHD_lookup_connection_value(connection, 
                                                    MHD_GET_ARGUMENT_KIND, 
                                                    "navbtn");
    if ( NULL == str_navbtn ) {
        //nFrom = 0;
        if ((nFrom+nCount) > pObject->m_dm.getRowCount()) {
            upperLimit = pObject->m_dm.getRowCount()-nFrom;
        }
        else {
            upperLimit = nFrom+nCount;
        }
    }
    else if (NULL != strstr("first",str_navbtn)) {
        nFrom = 0;
        if ((nFrom+nCount) > pObject->m_dm.getRowCount()) {
            upperLimit = pObject->m_dm.getRowCount()-nFrom;
        }
        else {
            upperLimit = nFrom+nCount;
        }
    }
    else if (NULL != strstr("previous", str_navbtn) ) {
        
        if ( 0 != nFrom ) {    
            
            nFrom -= nCount;
            upperLimit = nFrom+nCount;
            
            if ( nFrom < 0 ) {
                nFrom = 0;
                if ((nFrom-nCount) < 0) {
                    upperLimit = pObject->m_dm.getRowCount()- nFrom;
                }
                else {
                    upperLimit = nFrom-nCount;
                }
            }
            
            if (upperLimit < 0) {
                upperLimit = nCount;
            }
        }
        
    }
    else if (NULL != strstr("next",str_navbtn)) {

        if ( upperLimit < pObject->m_dm.getRowCount() ) {
            nFrom += nCount;
            if (nFrom >= pObject->m_dm.getRowCount()) {
                nFrom = pObject->m_dm.getRowCount() - nCount;
                if ( nFrom < 0 ) nFrom = 0;
            }
        
            if ((nFrom+nCount) > pObject->m_dm.getRowCount()) {
                upperLimit = pObject->m_dm.getRowCount();
            }
            else {
                upperLimit = nFrom+nCount;
            }
        }

    }
    else if (NULL != strstr("last",str_navbtn)) {
        nFrom = pObject->m_dm.getRowCount() - nCount;
        if ( nFrom < 0 ) {
            nFrom = 0;
            upperLimit = pObject->m_dm.getRowCount();
        }
        else {
            upperLimit = pObject->m_dm.getRowCount();
        }
    }

    wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Decision Matrix"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    // Insert server url into navigation menu 
    wxString navstr = _(WEB_COMMON_MENU);
    int pos;
    while ( wxNOT_FOUND != ( pos = navstr.Find(_("%s")))) {
        buildPage += navstr.Left( pos );
        navstr = navstr.Right(navstr.Length() - pos - 2);
    }
    buildPage += navstr;
    
    buildPage += _(WEB_DMLIST_BODY_START);
    
    {
        wxString wxstrurl = wxString::Format(_("%s/vscp/dm"), 
                                                strHost.GetData() );
        wxString wxstrlight = ((bLight) ? _("true") : _("false"));
        buildPage += wxString::Format( _(WEB_COMMON_LIST_NAVIGATION),
                wxstrurl.GetData(),
                nFrom,
                ((nFrom + nCount) < pObject->m_dm.getRowCount()) ? 
                    nFrom + nCount - 1 : pObject->m_dm.getRowCount() - 1,
                pObject->m_dm.getRowCount(),
                nCount,
                nFrom,
#if wxCHECK_VERSION(3,0,0)                
                wxstrlight );
#else                
                wxstrlight.GetWriteBuf(wxstrlight.Length()) );
#endif        
        buildPage += _("<br>");
    } 

    wxString strGUID;  
    wxString strBuf;

    // Display DM List
    
    if ( 0 == pObject->m_dm.getRowCount() ) {
        buildPage += _("<br>Decision Matrix is empty!<br>");
    }
    else {
        buildPage += _(WEB_DMLIST_TR_HEAD);
    }
    
    if (nFrom < 0) nFrom = 0;
    
    for ( int i=nFrom;i<upperLimit;i++) {
        
        dmElement *pElement = pObject->m_dm.getRow(i);
        
        {
            wxString url_dmedit = 
                    wxString::Format(_("%s/vscp/dmedit?id=%d"),
                                        strHost.GetData(),
                                        i );
            wxString str = wxString::Format(_(WEB_COMMON_TR_CLICKABLE_ROW),
                                                url_dmedit.GetData() );
            buildPage += str;
        }

        // Client id    
        buildPage += _(WEB_IFLIST_TD_CENTERED);
        buildPage += wxString::Format(_("<form name=\"input\" action=\"%s/vscp/dmdelete?id=%d\" method=\"get\">%d<input type=\"submit\" value=\"x\"><input type=\"hidden\" name=\"id\"value=\"%d\"></form>"), 
                        strHost.GetData(), i, i, i );
        buildPage += _("</td>");

        // DM entry
        buildPage += _("<td>");
        
        if (NULL != pElement) {

            buildPage += _("<div id=\"small\">");

            // Group
            buildPage += _("<b>Group:</b> ");
            buildPage += pElement->m_strGroupID;
            buildPage += _("<br>");
            
            buildPage += _("<b>Comment:</b> ");
            buildPage += pElement->m_comment;
            buildPage += _("<br><hr width=\"90%\">");

            buildPage += _("<b>Control:</b> ");

            // Control - Enabled
            if (pElement->isEnabled()) {
                buildPage += _("[Row is enabled] ");
            } 
            else {
                buildPage += _("[Row is disabled] ");
            }

            // Control - End scan
            if (pElement->isScanDontContinueSet()) {
                buildPage += _("[End scan after this row] ");
            }

            // Control - Check index
            if (pElement->isCheckIndexSet()) {
                if (pElement->m_bMeasurement) {
                    buildPage += _("[Check Measurement Index] ");
                } 
                else {
                    buildPage += _("[Check Index] ");
                }
            } 

            // Control - Check zone
            if (pElement->isCheckZoneSet()) {
                buildPage += _("[Check Zone] ");
            }

            // Control - Check subzone
            if (pElement->isCheckSubZoneSet()) {
                buildPage += _("[Check Subzone] ");
            }

            buildPage += _("<br>");

            if (!bLight) {

                // * Filter

                buildPage += _("<b>Filter_priority: </b>");
                buildPage += wxString::Format(_("%d "),
                        pElement->m_vscpfilter.filter_priority);

                buildPage += _("<b>Filter_class: </b>");
                buildPage += wxString::Format(_("%d "),
                        pElement->m_vscpfilter.filter_class);
                buildPage += _(" [");
                buildPage += vscpinfo.getClassDescription(
                        pElement->m_vscpfilter.filter_class);
                buildPage += _("] ");

                buildPage += _(" <b>Filter_type: </b>");
                buildPage += wxString::Format(_("%d "),
                        pElement->m_vscpfilter.filter_type);
                buildPage += _(" [");
                buildPage += vscpinfo.getTypeDescription(
                        pElement->m_vscpfilter.filter_class,
                        pElement->m_vscpfilter.filter_type);
                buildPage += _("]<br>");

                buildPage += _(" <b>Filter_GUID: </b>");
                writeGuidArrayToString(pElement->m_vscpfilter.filter_GUID, strGUID);
                buildPage += strGUID;

                buildPage += _("<br>");

                buildPage += _("<b>Mask_priority: </b>");
                buildPage += wxString::Format(_("%d "), pElement->m_vscpfilter.mask_priority);

                buildPage += _("<b>Mask_class: </b>");
                buildPage += wxString::Format(_("%d "), pElement->m_vscpfilter.mask_class);

                buildPage += _("<b>Mask_type: </b>");
                buildPage += wxString::Format(_("%d "), pElement->m_vscpfilter.mask_type);

                buildPage += _("<b>Mask_GUID: </b>");
                writeGuidArrayToString(pElement->m_vscpfilter.mask_GUID, strGUID);
                buildPage += strGUID;

                buildPage += _("<br>");

                buildPage += _("<b>Index: </b>");
                buildPage += wxString::Format(_("%d "), pElement->m_index);

                buildPage += _("<b>Zone: </b>");
                buildPage += wxString::Format(_("%d "), pElement->m_zone);

                buildPage += _("<b>Subzone: </b>");
                buildPage += wxString::Format(_("%d "), pElement->m_subzone);

                buildPage += _("<br>");

                buildPage += _("<b>Allowed from:</b> ");
                buildPage += pElement->m_timeAllow.m_fromTime.FormatISODate();
                buildPage += _(" ");
                buildPage += pElement->m_timeAllow.m_fromTime.FormatISOTime();

                buildPage += _(" <b>Allowed to:</b> ");
                buildPage += pElement->m_timeAllow.m_endTime.FormatISODate();
                buildPage += _(" ");
                buildPage += pElement->m_timeAllow.m_endTime.FormatISOTime();

                buildPage += _(" <b>Weekdays:</b> ");
                buildPage += pElement->m_timeAllow.getWeekDays();
                buildPage += _("<br>");

                buildPage += _("<b>Allowed time:</b> ");
                buildPage += pElement->m_timeAllow.getActionTimeAsString();
                buildPage += _("<br>");

            } // mini

            buildPage += _("<b>Action:</b> ");
            buildPage += wxString::Format(_("%d "), pElement->m_action);

            buildPage += _(" <b>Action parameters:</b> ");
            buildPage += pElement->m_actionparam;
            buildPage += _("<br>");

            if (!bLight) {

                buildPage += _("<b>Trigger Count:</b> ");
                buildPage += wxString::Format(_("%d "), pElement->m_triggCounter);

                buildPage += _("<b>Error Count:</b> ");
                buildPage += wxString::Format(_("%d "), pElement->m_errorCounter);
                buildPage += _("<br>");

                buildPage += _("<b>Last Error String:</b> ");
                buildPage += pElement->m_strLastError;

            } // mini

            buildPage += _("</div>");

        } 
        else {
            buildPage += _("Internal error: Non existent DM entry.");
        }

        buildPage += _("</td>");
        buildPage += _("</tr>");

    }
       
    buildPage += _(WEB_DMLIST_TABLE_END);
    
    {
        wxString wxstrurl = wxString::Format(_("%s/vscp/dm"), 
                                                strHost.GetData() );
        wxString wxstrlight = ((bLight) ? _("true") : _("false"));
        buildPage += wxString::Format( _(WEB_COMMON_LIST_NAVIGATION),
                wxstrurl.GetData(),
                nFrom,
                ((nFrom + nCount) < pObject->m_dm.getRowCount()) ? 
                    nFrom + nCount - 1 : pObject->m_dm.getRowCount() - 1,
                pObject->m_dm.getRowCount(),
                nCount,
                nFrom,
#if wxCHECK_VERSION(3,0,0)                
                wxstrlight );
#else                
                wxstrlight.GetWriteBuf(wxstrlight.Length()) );
#endif        
    }
     
    buildPage += _(WEB_COMMON_END);     // Common end code
    
    char *ppage = new char[ buildPage.Length() + 1 ];
    memset(ppage, 0, buildPage.Length() + 1 );
    memcpy( ppage, buildPage.ToAscii(), buildPage.Length() );        
    
    // return page 
    response = MHD_create_response_from_buffer( strlen(ppage),
                                                    (void *)ppage,
                                                    MHD_RESPMEM_MUST_FREE );
    
    websrv_add_session_cookie(session, response);
    
    MHD_add_response_header( response,
                                MHD_HTTP_HEADER_CONTENT_ENCODING,
                                mime);
    
    ret = MHD_queue_response( connection,
                                MHD_HTTP_OK,
                                response);
    
    MHD_destroy_response( response );
    
    return ret; 
}


///////////////////////////////////////////////////////////////////////////////
// websrv_serve_dmedit
//

int 
CControlObject::websrv_serve_dmedit( const void *cls,
							const char *mime,
                            struct websrv_Session *session,
                            struct MHD_Connection *connection)
{
    int ret,i;
    wxString str;
    VSCPInformation vscpinfo;
    CControlObject *pObject = (CControlObject *) cls;
    struct MHD_Response *response;
    dmElement *pElement = NULL;
    
    // Get connection type
    const MHD_ConnectionInfo *pProtocolInfo = 
            MHD_get_connection_info(connection, 
                                        MHD_CONNECTION_INFO_PROTOCOL );
    // Get hostname
    wxString strHost = _("http://localhost:8080");
    const char *str_host = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "host");
    if ( NULL != str_host ) {
        strHost = wxString::FromAscii(str_host);
        if ( NULL != pProtocolInfo ) {
            strHost = _("https://") + strHost;
        }
        else {
            strHost = _("http://") + strHost;
        }
    }
    
    // id
    long id = -1;
    const char *str_id = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "id");
    if ( NULL != str_id ) id = atoi(str_id);
    
    // Flag for new DM row
    bool bNew = false;
    const char *str_new = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "new");
    if ( (NULL != str_new) && NULL != strstr( "true", str_new ) ) bNew = true;
    
    wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Decision Matrix Edit"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    // Insert server url into navigation menu 
    wxString navstr = _(WEB_COMMON_MENU);
    int pos;
    while ( wxNOT_FOUND != ( pos = navstr.Find(_("%s")))) {
        buildPage += navstr.Left( pos );
        navstr = navstr.Right(navstr.Length() - pos - 2);
    }
    
    buildPage += navstr;
    
    buildPage += _(WEB_DMEDIT_BODY_START);

    if ( !bNew && id < pObject->m_dm.getRowCount() ) {
        pElement = pObject->m_dm.getRow(id);
    }

    if (bNew || (NULL != pElement)) {
        
        if ( bNew ) {
            buildPage += _("<span id=\"optiontext\">New record.</span><br>");
        }
        else {
            buildPage += wxString::Format(_("<span id=\"optiontext\">Record = %d.</span><br>"), id);
        }
        
        buildPage += _("<br><form method=\"get\" action=\"");
        buildPage += strHost;
        buildPage += _("/vscp/dmpost");
        buildPage += _("\" name=\"dmedit\">");
        
        buildPage += wxString::Format(_("<input name=\"id\" value=\"%d\" type=\"hidden\"></input>"), id );
        
        if (bNew) {
            buildPage += _("<input name=\"new\" value=\"true\" type=\"hidden\"></input>");
        }
        else {
            buildPage += _("<input name=\"new\" value=\"false\" type=\"hidden\">false</input>");
        }
        
        buildPage += _("<h4>Group id:</h4>");
        buildPage += _("<textarea cols=\"20\" rows=\"1\" name=\"groupid\">");
        if ( !bNew ) buildPage += pElement->m_strGroupID;
        buildPage += _("</textarea><br>");
        
        
        buildPage += _("<h4>Event:</h4> <span id=\"optiontext\">(leave items blank for don't care)</span><br>");

        buildPage += _("<table class=\"invisable\"><tbody><tr class=\"invisable\">");

        buildPage += _("<td class=\"invisable\">Priority:</td><td class=\"invisable\">");

        // Priority
        buildPage += _("<select name=\"filter_priority\">");
        buildPage += _("<option value=\"-1\" ");
        if (bNew) buildPage += _(" selected ");
        buildPage += _(">Don't care</option>");

        if ( !bNew ) str = (0 == pElement->m_vscpfilter.filter_priority) ? _("selected") : _(" ");
        buildPage += wxString::Format(_("<option value=\"0\" %s>0 - Highest</option>"),
                str.GetData());

        if ( !bNew ) str = (1 == pElement->m_vscpfilter.filter_priority) ? _("selected") : _(" ");
        buildPage += wxString::Format(_("<option value=\"1\" %s>1 - Very High</option>"),
                str.GetData());

        if ( !bNew ) str = (2 == pElement->m_vscpfilter.filter_priority) ? _("selected") : _(" ");
        buildPage += wxString::Format(_("<option value=\"2\" %s>2 - High</option>"),
                str.GetData());

        if ( !bNew ) str = (3 == pElement->m_vscpfilter.filter_priority) ? _("selected") : _(" ");
        buildPage += wxString::Format(_("<option value=\"3\" %s>3 - Normal</option>"),
                str.GetData());

        if ( !bNew ) str = (4 == pElement->m_vscpfilter.filter_priority) ? _("selected") : _(" ");
        buildPage += wxString::Format(_("<option value=\"4\" %s>4 - Low</option>"),
                str.GetData());

        if ( !bNew ) str = (5 == pElement->m_vscpfilter.filter_priority) ? _("selected") : _(" ");
        buildPage += wxString::Format(_("<option value=\"5\" %s>5 - Lower</option>"),
                str.GetData());

        if ( !bNew ) str = (6 == pElement->m_vscpfilter.filter_priority) ? _("selected") : _(" ");
        buildPage += wxString::Format(_("<option value=\"6\" %s>6 - Very Low</option>"),
                str.GetData());

        if ( !bNew ) str = (7 == pElement->m_vscpfilter.filter_priority) ? _("selected") : _(" ");
        buildPage += wxString::Format(_("<option value=\"7\" %s>7 - Lowest</option>"),
                str.GetData());

        buildPage += _("</select>");
        // Priority mask
        buildPage += _("<textarea cols=\"5\" rows=\"1\" name=\"mask_priority\">");
        if ( bNew ) {
            buildPage += _("0x00");
        }
        else {
            buildPage += wxString::Format(_("%X"), pElement->m_vscpfilter.mask_priority );
        }
        buildPage += _("</textarea>");
        
        buildPage += _("</td></tr>");

        // Class
        buildPage += _("<tr class=\"invisable\"><td class=\"invisable\">Class:</td><td class=\"invisable\"><textarea cols=\"10\" rows=\"1\" name=\"filter_vscpclass\">");
        if ( bNew ) {
            buildPage += _("");;
        }
        else {
            buildPage += wxString::Format(_("0x%X"), pElement->m_vscpfilter.filter_class);
        }
        buildPage += _("</textarea>");
        
        buildPage += _(" <textarea cols=\"10\" rows=\"1\" name=\"mask_vscpclass\">");
        if ( bNew ) {
            buildPage += _("0x0000");
        }
        else {
            buildPage += wxString::Format(_("0x%04x"), pElement->m_vscpfilter.mask_class);
        }
        buildPage += _("</textarea>");
        
        buildPage += _("</td></tr>");
        
        // Type
        buildPage += _("<tr class=\"invisable\"><td class=\"invisable\">Type:</td><td class=\"invisable\"><textarea cols=\"10\" rows=\"1\" name=\"filter_vscptype\">");
        if ( bNew ) {
            buildPage += _("");;
        }
        else {
            buildPage += wxString::Format(_("%d"), pElement->m_vscpfilter.filter_type);
        }
        buildPage += _("</textarea>");
        
        buildPage += _(" <textarea cols=\"10\" rows=\"1\" name=\"mask_vscptype\">");
        if ( bNew ) {
            buildPage += _("0x0000");;
        }
        else {
            buildPage += wxString::Format(_("0x%04x"), pElement->m_vscpfilter.mask_type);
        }
        buildPage += _("</textarea>");
        
        buildPage += _("</td></tr>"); 
        
        // GUID
        if ( !bNew ) writeGuidArrayToString( pElement->m_vscpfilter.filter_GUID, str );
        buildPage += _("<tr class=\"invisable\"><td class=\"invisable\">GUID:</td><td class=\"invisable\"><textarea cols=\"50\" rows=\"1\" name=\"filter_vscpguid\">");
        if ( bNew ) {
            buildPage += _("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00");
        }
        else {
            buildPage += wxString::Format(_("%s"), str.GetData() );
        }
        buildPage += _("</textarea></td>");
        
        if ( !bNew ) writeGuidArrayToString( pElement->m_vscpfilter.mask_GUID, str );
        buildPage += _("<tr class=\"invisable\"><td class=\"invisable\"> </td><td class=\"invisable\"><textarea cols=\"50\" rows=\"1\" name=\"mask_vscpguid\">");
        if ( bNew ) {
            buildPage += _("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00");
        }
        else {
            buildPage += wxString::Format(_("%s"), str.GetData() );
        }
        buildPage += _("</textarea></td>");
        
        buildPage += _("</tr>");
        
        // Index
        buildPage += _("<tr class=\"invisable\"><td class=\"invisable\">Index:</td><td class=\"invisable\"><textarea cols=\"10\" rows=\"1\" name=\"vscpindex\">");
        if ( bNew ) {
            buildPage += _("");
        }
        else {
            buildPage += wxString::Format(_("%d"), pElement->m_index );
        }
        buildPage += _("</textarea></td></tr>");

        // Zone
        buildPage += _("<tr class=\"invisable\"><td class=\"invisable\">Zone:</td><td class=\"invisable\"><textarea cols=\"10\" rows=\"1\" name=\"vscpzone\">");
        if ( bNew ) {
            buildPage += _("0");
        }
        else {
            buildPage += wxString::Format(_("%d"), pElement->m_zone );
        }
        buildPage += _("</textarea></td></tr>");
        
        // Subzone
        buildPage += _("<tr class=\"invisable\"><td class=\"invisable\">Subzone:</td><td class=\"invisable\"><textarea cols=\"10\" rows=\"1\" name=\"vscpsubzone\">");
        if ( bNew ) {
            buildPage += _("0");
        }
        else {
            buildPage += wxString::Format(_("%d"), pElement->m_subzone );
        }
        buildPage += _("</textarea>");
        buildPage += _("</td></tr>");
        
        buildPage += _("</tbody></table><br>");
        
        // Control
        buildPage += _("<h4>Control:</h4>");
        
        // Enable row
        buildPage += _("<input name=\"check_enablerow\" value=\"true\" ");
        if ( bNew ) {
            buildPage += _("");
        }
        else {
            buildPage += wxString::Format(_("%s"), 
                                            pElement->isEnabled() ? _("checked") : _("") );
        }
        buildPage += _(" type=\"checkbox\">");            
        buildPage += _("<span id=\"optiontext\">Enable row</span>"); 

        // End scan on this row
        buildPage += _("<input name=\"check_endscan\" value=\"true\"");
        if ( bNew ) {
            buildPage += _("");
        }
        else {
            buildPage += wxString::Format(_("%d"), 
                                            pElement->isScanDontContinueSet() ? _("checked") : _("") );
        }
        buildPage += _(" type=\"checkbox\">");           
        buildPage += _("<span id=\"optiontext\">End scan on this row</span>");

        buildPage += _("<br>");

        // Check Index
        buildPage += _("<input name=\"check_index\" value=\"true\"");
        if ( bNew ) {
            buildPage += _("");
        }
        else {
            buildPage += wxString::Format(_("%d"), 
                                            pElement->isCheckIndexSet() ? _("checked") : _("") );
        }
        buildPage += _(" type=\"checkbox\">"); 
        buildPage += _("<span id=\"optiontext\">Check Index</span>");

        // Check Zone
        buildPage += _("<input name=\"check_zone\" value=\"true\"");
        if ( bNew ) {
            buildPage += _("");
        }
        else {
            buildPage += wxString::Format(_("%d"), 
                                            pElement->isCheckZoneSet() ? _("checked") : _("") );
        }
        buildPage += _(" type=\"checkbox\">"); 
        buildPage += _("<span id=\"optiontext\">Check Zone</span>"); 

        // Check subzone
        buildPage += _("<input name=\"check_subzone\" value=\"true\"");
        if ( bNew ) {
            buildPage += _("");
        }
        else {
            buildPage += wxString::Format(_("%d"), 
                                            pElement->isCheckSubZoneSet() ? _("checked") : _("") );
        }
        buildPage += _(" type=\"checkbox\">"); 
        buildPage += _("<span id=\"optiontext\">Check Subzone</span>");
        buildPage += _("<br><br><br>");
        
        buildPage += _("<h4>Allowed From:</h4>");
        buildPage += _("<textarea cols=\"50\" rows=\"1\" name=\"allowedfrom\">");
        if ( bNew ) {
            buildPage += _("yyyy-mm-dd hh:mm:ss");
        }
        else {
            buildPage += pElement->m_timeAllow.m_fromTime.FormatISODate();
            buildPage += _(" ");
            buildPage += pElement->m_timeAllow.m_fromTime.FormatISOTime();
        }
        buildPage += _("</textarea>");

        buildPage += _("<h4>Allowed To:</h4>");
        buildPage += _("<textarea cols=\"50\" rows=\"1\" name=\"allowedto\">");
        if ( bNew ) {
            buildPage += _("yyyy-mm-dd hh:mm:ss");
        }
        else {
            buildPage += pElement->m_timeAllow.m_endTime.FormatISODate();
            buildPage += _(" ");
            buildPage += pElement->m_timeAllow.m_endTime.FormatISOTime();
        }
        buildPage += _("</textarea>");
       
        buildPage += _("<h4>Allowed time:</h4>");
        buildPage += _("<textarea cols=\"50\" rows=\"1\" name=\"allowedtime\">");
        if ( bNew ) {
            buildPage += _("yyyy-mm-dd hh:mm:ss");
        }
        else {
            buildPage += pElement->m_timeAllow.getActionTimeAsString();
        }
        buildPage += _("</textarea>");
        
        buildPage += _("<h4>Allowed days:</h4>");
        buildPage += _("<input name=\"monday\" value=\"true\" ");
        
        if ( !bNew ) buildPage += wxString::Format(_("%s"), 
                                        pElement->m_timeAllow.m_weekDay[0] ? _("checked") : _("") );
        buildPage += _(" type=\"checkbox\">Monday ");

        buildPage += _("<input name=\"tuesday\" value=\"true\" ");
        if ( !bNew ) buildPage += wxString::Format(_("%s"), 
                                        pElement->m_timeAllow.m_weekDay[1] ? _("checked") : _("") );
        buildPage += _(" type=\"checkbox\">Tuesday ");

        buildPage += _("<input name=\"wednesday\" value=\"true\" ");
        if ( !bNew ) buildPage += wxString::Format(_("%s"), 
                                        pElement->m_timeAllow.m_weekDay[2] ? _("checked") : _("") );
        buildPage += _(" type=\"checkbox\">Wednesday ");

        buildPage += _("<input name=\"thursday\" value=\"true\" ");
        if ( !bNew ) buildPage += wxString::Format(_("%s"), 
                                        pElement->m_timeAllow.m_weekDay[3] ? _("checked") : _("") );
        buildPage += _(" type=\"checkbox\">Thursday ");
         
        buildPage += _("<input name=\"friday\" value=\"true\" ");
        if ( !bNew ) buildPage += wxString::Format(_("%s"), 
                                        pElement->m_timeAllow.m_weekDay[4] ? _("checked") : _("") );
        buildPage += _(" type=\"checkbox\">Friday ");

        buildPage += _("<input name=\"saturday\" value=\"true\" ");
        if ( !bNew ) buildPage += wxString::Format(_("%s"), 
                                        pElement->m_timeAllow.m_weekDay[5] ? _("checked") : _("") );
        buildPage += _(" type=\"checkbox\">Saturday ");

        buildPage += _("<input name=\"sunday\" value=\"true\" ");
        if ( !bNew ) buildPage += wxString::Format(_("%s"), 
                                        pElement->m_timeAllow.m_weekDay[6] ? _("checked") : _("") );
        buildPage += _(" type=\"checkbox\">Sunday ");
        buildPage += _("<br>");
        
        buildPage += _("<h4>Action:</h4>");
        
        buildPage += _("<select name=\"action\">");
        buildPage += _("<option value=\"0\" ");
        if (bNew) buildPage += _(" selected ");
        buildPage += _(">No Operation</option>");

        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x10 == pElement->m_action ) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x10\" %s>Execute external program</option>"),
                str.GetData());

        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x12 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x12\" %s>Execute internal procedure</option>"),
                str.GetData());

        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x30 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x30\" %s>Execute library procedure</option>"),
                str.GetData());

        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x40 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x40\" %s>Send event</option>"),
                str.GetData());

        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x41 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x41\" %s>Send event Conditional</option>"),
                str.GetData());

        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x42 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x42\" %s>Send event(s) from file</option>"),
                str.GetData());

        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x43 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x43\" %s>Send event(s) to remote VSCP server</option>"),
                str.GetData());

        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x50 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x50\" %s>Store in variable</option>"),
                str.GetData());
        
        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x51 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x51\" %s>Store in array</option>"),
                str.GetData());
        
        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x52 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x52\" %s>Add to variable</option>"),
                str.GetData());
        
        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x53 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x53\" %s>Subtract from variable</option>"),
                str.GetData());
        
        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x54 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x54\" %s>Multiply variable</option>"),
                str.GetData());
        
        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x55 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x55\" %s>Divide variable</option>"),
                str.GetData());
        
        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x60 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x60\" %s>Start timer</option>"),
                str.GetData());
        
        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x61 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x61\" %s>Pause timer</option>"),
                str.GetData());
        
        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x62 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x62\" %s>Stop timer</option>"),
                str.GetData());
        
        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x63 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x63\" %s>Resume timer</option>"),
                str.GetData());
        
        if ( bNew ) {
            str = _("");
        }
        else {
            str = (0x70 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x70\" %s>Write file</option>"),
                str.GetData());
        
        if ( bNew ) {
            str = _("");
        }
        else { 
            str = (0x75 == pElement->m_action) ? _("selected") : _(" ");
        }
        buildPage += wxString::Format(_("<option value=\"0x75\" %s>Get/Put/Post URL</option>"),
                str.GetData());

        buildPage += _("</select>");
             
        buildPage += _("<h4>Action parameter:</h4>");
        buildPage += _("<textarea cols=\"80\" rows=\"1\" name=\"actionparameter\">");
        if ( !bNew ) buildPage += pElement->m_actionparam;
        buildPage += _("</textarea>");

        buildPage += _("<h4>Comment:</h4>");
        buildPage += _("<textarea cols=\"80\" rows=\"5\" name=\"comment\">");
        if ( !bNew ) buildPage += pElement->m_comment;
        buildPage += _("</textarea>");
    } 
    else {
        buildPage += _("<br><b>Error: Non existent id</b>");
    }
    
    
    wxString wxstrurl = wxString::Format(_("%s/vscp/dmpost"), 
                                                strHost.GetData() );
    buildPage += wxString::Format( _(WEB_DMEDIT_SUBMIT),
                                    wxstrurl.GetData() );
    
    buildPage += _("</form>");
    buildPage += _(WEB_COMMON_END);     // Common end code
    
    char *ppage = new char[ buildPage.Length() + 1 ];
    memset(ppage, 0, buildPage.Length() + 1 );
    memcpy( ppage, buildPage.ToAscii(), buildPage.Length() );        
    
    // return page 
    response = MHD_create_response_from_buffer( strlen(ppage),
                                                    (void *)ppage,
                                                    MHD_RESPMEM_MUST_FREE );
    
    websrv_add_session_cookie(session, response);
    
    MHD_add_response_header( response,
                                MHD_HTTP_HEADER_CONTENT_ENCODING,
                                mime);
    
    ret = MHD_queue_response( connection,
                                MHD_HTTP_OK,
                                response);
    
    MHD_destroy_response( response );
    
    return ret;
}



///////////////////////////////////////////////////////////////////////////////
// websrv_serve_dmpost
//

int 
CControlObject::websrv_serve_dmpost( const void *cls,
							const char *mime,
                            struct websrv_Session *session,
                            struct MHD_Connection *connection)
{
    int ret,i;
    wxString str;
    VSCPInformation vscpinfo;
    CControlObject *pObject = (CControlObject *) cls;
    struct MHD_Response *response;
    dmElement *pElement = NULL;
    
    // Get connection type
    const MHD_ConnectionInfo *pProtocolInfo = 
            MHD_get_connection_info(connection, 
                                        MHD_CONNECTION_INFO_PROTOCOL );
    // Get hostname
    wxString strHost = _("http://localhost:8080");
    const char *str_host = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "host");
    if ( NULL != str_host ) {
        strHost = wxString::FromAscii(str_host);
        if ( NULL != pProtocolInfo ) {
            strHost = _("https://") + strHost;
        }
        else {
            strHost = _("http://") + strHost;
        }
    }
    
    // id
    long id = -1;
    const char *str_id = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "id");
    if ( NULL != str_id ) id = atoi(str_id);
    
    // Flag for new DM row
    bool bNew = false;
    const char *str_new = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "new");
    if ( (NULL != str_new) && NULL != strstr( "true", str_new ) ) bNew = true;

    wxString strGroupID;
    const char *str_groupid = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "groupid");
    if ( NULL != str_groupid ) strGroupID = wxString::FromAscii(str_groupid);
        
    int filter_priority = -1;
    const char *str_filter_priority = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "filter_priority");
    if ( NULL != str_filter_priority ) filter_priority = readStringValue( wxString::FromAscii(str_filter_priority ) );
         
    int mask_priority = 0;    
    const char *str_mask_priority = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "mask_priority");
    if ( NULL != str_mask_priority ) mask_priority = readStringValue( wxString::FromAscii(str_mask_priority ) );
    
    uint16_t filter_vscpclass = -1;
    const char *str_filter_vscpclass = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "filter_vscpclass");
    if ( NULL != str_filter_vscpclass ) {
		wxString wrkstr = wxString::FromAscii( str_filter_vscpclass );
		filter_vscpclass = readStringValue( wrkstr );
	}
    
    uint16_t mask_vscpclass = 0;
    const char *str_mask_vscpclass = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "mask_vscpclass");
    if ( NULL != str_mask_vscpclass ) mask_vscpclass = readStringValue( wxString::FromAscii( str_mask_vscpclass ) );
    
    uint16_t filter_vscptype = 0;
    const char *str_filter_vscptype = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "filter_vscptype");
    if ( NULL != str_filter_vscptype ) filter_vscptype = readStringValue( wxString::FromAscii(str_filter_vscptype) );
    
    uint16_t mask_vscptype = 0;
    const char *str_mask_vscptype = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "mask_vscptype");
    if ( NULL != str_mask_vscptype ) mask_vscptype = readStringValue( wxString::FromAscii(str_mask_vscptype ) );
    
    wxString strFilterGuid;
    const char *str_filter_vscpguid = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "filter_vscpguid");
    if ( NULL != str_filter_vscpguid ) strFilterGuid = wxString::FromAscii(str_filter_vscpguid);
    strFilterGuid = strFilterGuid.Trim();
    strFilterGuid = strFilterGuid.Trim(false);
    
    wxString strMaskGuid;
    const char *str_mask_vscpguid = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "mask_vscpguid");
    if ( NULL != str_mask_vscpguid ) strMaskGuid = wxString::FromAscii(str_mask_vscpguid);
    strMaskGuid = strMaskGuid.Trim();
    strMaskGuid = strMaskGuid.Trim(false);
    
    uint8_t index = 0;
    const char *str_vscpindex = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "vscpindex");
    if ( NULL != str_vscpindex ) index = readStringValue( wxString::FromAscii(str_vscpindex) );
    
    uint8_t zone = 0;
    const char *str_vscpzone = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "vscpzone");
    if ( NULL != str_vscpzone ) zone = readStringValue( wxString::FromAscii(str_vscpzone) );
    
    uint8_t subzone = 0;
    const char *str_vscpsubzone = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "vscpsubzone");
    if ( NULL != str_vscpsubzone ) index = readStringValue( wxString::FromAscii(str_vscpsubzone) );
    
    bool bEnableRow = false;
    const char *str_check_enablerow = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "check_enablerow");
    if ( (NULL != str_check_enablerow) && NULL != strstr( "true", str_check_enablerow ) ) bEnableRow = true;
        
    bool bEndScan = false;
    const char *str_check_endscan = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "check_endscan");
    if ( (NULL != str_check_endscan) && NULL != strstr( "true", str_check_endscan ) ) bEndScan = true;
    
    bool bCheckIndex = false;
    const char *str_check_index = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "check_index");
    if ( (NULL != str_check_index) && NULL != strstr( "true", str_check_index ) ) bCheckIndex = true;
    
    bool bCheckZone = false;
    const char *str_check_zone = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "check_zone");
    if ( (NULL != str_check_zone) && NULL != strstr( "true", str_check_zone ) ) bCheckZone = true;
    
    bool bCheckSubZone = false;
    const char *str_check_subzone = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "check_subzone");
    if ( (NULL != str_check_subzone) && NULL != strstr( "true", str_check_subzone ) ) bCheckSubZone = true;
    
    wxString strAllowedFrom;
    const char *str_allowedfrom = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "allowedfrom");
    if ( NULL != str_allowedfrom ) strAllowedFrom = wxString::FromAscii(str_allowedfrom);
    
    wxString strAllowedTo;
    const char *str_allowedto = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "allowedto");
    if ( NULL != str_allowedto ) strAllowedTo = wxString::FromAscii(str_allowedto);
    
    wxString strAllowedTime;
    const char *str_allowedtime = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "allowedtime");
    if ( NULL != str_allowedtime ) strAllowedTime = wxString::FromAscii(str_allowedtime);
    
    bool bCheckMonday = false;
    const char *str_monday = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "monday");
    if ( (NULL != str_monday) && NULL != strstr( "true", str_monday ) ) bCheckMonday = true;
    
    bool bCheckTuesday = false;
    const char *str_tuesday = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "tuesday");
    if ( (NULL != str_tuesday) && NULL != strstr( "true", str_tuesday ) ) bCheckTuesday = true;
    
    bool bCheckWednesday = false;
    const char *str_wednesday = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "wednesday");
    if ( (NULL != str_wednesday) && NULL != strstr( "true", str_wednesday ) ) bCheckWednesday = true;
    
    bool bCheckThursday = false;
    const char *str_thursday = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "thursday");
    if ( (NULL != str_thursday) && NULL != strstr( "true", str_thursday ) ) bCheckThursday = true;
    
    bool bCheckFriday = false;
    const char *str_friday = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "friday");
    if ( (NULL != str_friday) && NULL != strstr( "true", str_friday ) ) bCheckFriday = true;
    
    bool bCheckSaturday = false;
    const char *str_saturday = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "saturday");
    if ( (NULL != str_saturday) && NULL != strstr( "true", str_saturday ) ) bCheckSaturday = true;
    
    bool bCheckSunday = false;
    const char *str_sunday = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "sunday");
    if ( (NULL != str_sunday) && NULL != strstr( "true", str_sunday ) ) bCheckSunday = true;
    
    uint32_t action = 0;
    const char *str_action = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "action");
    if ( NULL != str_action ) action = readStringValue( wxString::FromAscii(str_action) );
    
    wxString strActionParameter;
    const char *str_actionparameter = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "actionparameter");
    if ( NULL != str_actionparameter ) strActionParameter = wxString::FromAscii(str_actionparameter);
    
    wxString strComment;
    const char *str_comment = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "comment");
    if ( NULL != str_comment ) strComment = wxString::FromAscii(str_comment);
    
    wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Decision Matrix Post"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += wxString::Format(_("<meta http-equiv=\"refresh\" content=\"2;url=%s/vscp/dm"), strHost.GetData() );
    buildPage += wxString::Format(_("?from=%d"), id );
    buildPage += _("\">");
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    // Insert server url into navigation menu 
    wxString navstr = _(WEB_COMMON_MENU);
    int pos;
    while ( wxNOT_FOUND != ( pos = navstr.Find(_("%s")))) {
        buildPage += navstr.Left( pos );
        navstr = navstr.Right(navstr.Length() - pos - 2);
    }
    buildPage += navstr;
    
    buildPage += _(WEB_DMPOST_BODY_START);
        
    if (bNew) {
        pElement = new dmElement;
    }

    if ( bNew || ( id >= 0 ) ) {

        if ( bNew || ((0 == id) && !bNew) || ( id < pObject->m_dm.getRowCount() ) ) {

            if (!bNew) pElement = pObject->m_dm.getRow(id);

            if (NULL != pElement) {

                if (-1 == filter_priority) {
                    pElement->m_vscpfilter.mask_priority = 0;
                    pElement->m_vscpfilter.filter_priority = 0;
                } 
                else {
                    pElement->m_vscpfilter.mask_priority = mask_priority;
                    pElement->m_vscpfilter.filter_priority = filter_priority;
                }

                if (-1 == filter_vscpclass) {
                    pElement->m_vscpfilter.mask_class = 0;
                    pElement->m_vscpfilter.filter_class = 0;
                } 
                else {
                    pElement->m_vscpfilter.mask_class = mask_vscpclass;
                    pElement->m_vscpfilter.filter_class = filter_vscpclass;
                }

                if (-1 == filter_vscptype) {
                    pElement->m_vscpfilter.mask_type = 0;
                    pElement->m_vscpfilter.filter_type = 0;
                } 
                else {
                    pElement->m_vscpfilter.mask_type = mask_vscptype;
                    pElement->m_vscpfilter.filter_type = filter_vscptype;
                }

                if (0 == strFilterGuid.Length()) {
                    for (int i = 0; i < 16; i++) {
                        pElement->m_vscpfilter.mask_GUID[i] = 0;
                        pElement->m_vscpfilter.filter_GUID[i] = 0;
                    }
                } 
                else {
                    getGuidFromStringToArray(pElement->m_vscpfilter.mask_GUID,
                            strMaskGuid);
                    getGuidFromStringToArray(pElement->m_vscpfilter.filter_GUID,
                            strFilterGuid);
                }

                pElement->m_index = index;
                pElement->m_zone = zone;
                pElement->m_subzone = subzone;

                pElement->m_control = 0;
                if (bEnableRow) pElement->m_control |= DM_CONTROL_ENABLE;
                if (bEndScan) pElement->m_control |= DM_CONTROL_DONT_CONTINUE_SCAN;
                if (bCheckIndex) pElement->m_control |= DM_CONTROL_CHECK_INDEX;
                if (bCheckZone) pElement->m_control |= DM_CONTROL_CHECK_ZONE;
                if (bCheckSubZone) pElement->m_control |= DM_CONTROL_CHECK_SUBZONE;

                pElement->m_timeAllow.m_fromTime.ParseDateTime(strAllowedFrom);
                pElement->m_timeAllow.m_endTime.ParseDateTime(strAllowedTo);
                pElement->m_timeAllow.parseActionTime(strAllowedTime);

                wxString weekdays;

                if (bCheckMonday) weekdays = _("m"); else weekdays = _("-");
                if (bCheckTuesday) weekdays += _("t"); else weekdays += _("-");
                if (bCheckWednesday) weekdays += _("w"); else weekdays += _("-");
                if (bCheckThursday) weekdays += _("t"); else weekdays += _("-");
                if (bCheckFriday) weekdays += _("f"); else weekdays += _("-");
                if (bCheckSaturday) weekdays += _("s"); else weekdays += _("-");
                if (bCheckSunday) weekdays += _("s"); else weekdays += _("-");
                pElement->m_timeAllow.setWeekDays(weekdays);

                pElement->m_action = action;

                pElement->m_actionparam = strActionParameter;
                pElement->m_comment = strComment;
                
                pElement->m_strGroupID = strGroupID;

                pElement->m_triggCounter = 0;
                pElement->m_errorCounter = 0;

                if (bNew) {
                    // add the DM row to the matrix
                    pObject->m_dm.addElement(pElement);
                }

                // Save decision matrix
                pObject->m_dm.save();

                buildPage += wxString::Format(_("<br><br>DM Entry has been saved. id=%d"), id);
            } else {
                buildPage += wxString::Format(_("<br><br>Memory problem id=%d. Unable to save record"), id);
            }

        } else {
            buildPage += wxString::Format(_("<br><br>Record id=%d is to large. Unable to save record"), id);
        }
    } else {
        buildPage += wxString::Format(_("<br><br>Record id=%d is wrong. Unable to save record"), id);
    }


    buildPage += _(WEB_COMMON_END); // Common end code 
    
    char *ppage = new char[ buildPage.Length() + 1 ];
    memset(ppage, 0, buildPage.Length() + 1 );
    memcpy( ppage, buildPage.ToAscii(), buildPage.Length() );        
    
    // return page 
    response = MHD_create_response_from_buffer( strlen(ppage),
                                                    (void *)ppage,
                                                    MHD_RESPMEM_MUST_FREE );
    
    websrv_add_session_cookie(session, response);
    
    MHD_add_response_header( response,
                                MHD_HTTP_HEADER_CONTENT_ENCODING,
                                mime);
    
    ret = MHD_queue_response( connection,
                                MHD_HTTP_OK,
                                response);
    
    MHD_destroy_response( response );
    
    return ret;
}


///////////////////////////////////////////////////////////////////////////////
// websrv_serve_dmdelete
//

int 
CControlObject::websrv_serve_dmdelete( const void *cls,
                                        const char *mime,
                                        struct websrv_Session *session,
                                        struct MHD_Connection *connection)
{
    int ret,i;
    wxString str;
    VSCPInformation vscpinfo;
    CControlObject *pObject = (CControlObject *) cls;
    struct MHD_Response *response;
    dmElement *pElement = NULL;
 
       // Get connection type
    const MHD_ConnectionInfo *pProtocolInfo = 
            MHD_get_connection_info(connection, 
                                        MHD_CONNECTION_INFO_PROTOCOL );
    // Get hostname
    wxString strHost = _("http://localhost:8080");
    const char *str_host = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "host");
    if ( NULL != str_host ) {
        strHost = wxString::FromAscii(str_host);
        if ( NULL != pProtocolInfo ) {
            strHost = _("https://") + strHost;
        }
        else {
            strHost = _("http://") + strHost;
        }
    }
    
    // id
    long id = -1;
    const char *str_id = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "id");
    if ( NULL != str_id ) id = atoi(str_id);
        
    wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Decision Matrix Delete"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += wxString::Format(_("<meta http-equiv=\"refresh\" content=\"2;url=%s/vscp/dm"), strHost.GetData() );
    buildPage += wxString::Format(_("?from=%d"), id + 1 );
    buildPage += _("\">");
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    // Insert server url into navigation menu 
    wxString navstr = _(WEB_COMMON_MENU);
    int pos;
    while ( wxNOT_FOUND != ( pos = navstr.Find(_("%s")))) {
        buildPage += navstr.Left( pos );
        navstr = navstr.Right(navstr.Length() - pos - 2);
    }
    buildPage += navstr;
    
    buildPage += _(WEB_DMEDIT_BODY_START);
    
    if ( pObject->m_dm.removeRow( id ) ) {
        buildPage += wxString::Format(_("<br>Deleted record id = %d"), id);
        // Save decision matrix
        pObject->m_dm.save();
    }
    else {
        buildPage += wxString::Format(_("<br>Failed to remove record id = %d"), id);
    }
    
    buildPage += _(WEB_COMMON_END);     // Common end code
    
    char *ppage = new char[ buildPage.Length() + 1 ];
    memset(ppage, 0, buildPage.Length() + 1 );
    memcpy( ppage, buildPage.ToAscii(), buildPage.Length() );        
    
    // return page 
    response = MHD_create_response_from_buffer( strlen(ppage),
                                                    (void *)ppage,
                                                    MHD_RESPMEM_MUST_FREE );
    
    websrv_add_session_cookie(session, response);
    
    MHD_add_response_header( response,
                                MHD_HTTP_HEADER_CONTENT_ENCODING,
                                mime);
    
    ret = MHD_queue_response( connection,
                                MHD_HTTP_OK,
                                response);
    
    MHD_destroy_response( response );
    
    return ret;
}




///////////////////////////////////////////////////////////////////////////////
// websrv_serve_variables_list
//

int
CControlObject::websrv_serve_variables_list( const void *cls,
                                                const char *mime,
                                                struct websrv_Session *session,
                                                struct MHD_Connection *connection)
{
    int ret;
    VSCPInformation vscpinfo;
    CControlObject *pObject = (CControlObject *) cls;
    struct MHD_Response *response;
    long upperLimit = 50;
    
    // Get connection type
    const MHD_ConnectionInfo *pProtocolInfo = 
            MHD_get_connection_info(connection, 
                                        MHD_CONNECTION_INFO_PROTOCOL );
    // Get hostname
    wxString strHost = _("http://localhost:8080");
    const char *str_host = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "host");
    if ( NULL != str_host ) {
        strHost = wxString::FromAscii(str_host);
        if ( NULL != pProtocolInfo ) {
            strHost = _("https://") + strHost;
        }
        else {
            strHost = _("http://") + strHost;
        }
    }
            
    // From
    long nFrom = 0;
    const char *str_from = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "from");
    if ( NULL != str_from ) nFrom = atoi(str_from);
    // Check limits
    if (nFrom > pObject->m_VSCP_Variables.m_listVariable.GetCount()) nFrom = 0;
    
    // Count
    uint16_t nCount = 50;
    const char *str_count = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "count");
    if ( NULL != str_count ) nCount = atoi(str_count);
    // Check limits
    if ((nFrom+nCount) > pObject->m_VSCP_Variables.m_listVariable.GetCount()) {
        upperLimit = pObject->m_VSCP_Variables.m_listVariable.GetCount()-nFrom;
    }
    else {
        upperLimit = nFrom+nCount;
    }
    
    // Navigation button
    const char *str_navbtn = 
                    MHD_lookup_connection_value(connection, 
                                                    MHD_GET_ARGUMENT_KIND, 
                                                    "navbtn");
    if ( NULL == str_navbtn ) {
        //nFrom = 0;
        if ((nFrom+nCount) > pObject->m_VSCP_Variables.m_listVariable.GetCount()) {
            upperLimit = pObject->m_VSCP_Variables.m_listVariable.GetCount()-nFrom;
        }
        else {
            upperLimit = nFrom+nCount;
        }
    }
    else if (NULL != strstr("first",str_navbtn)) {
        nFrom = 0;
        if ((nFrom+nCount) > pObject->m_VSCP_Variables.m_listVariable.GetCount()) {
            upperLimit = pObject->m_VSCP_Variables.m_listVariable.GetCount()-nFrom;
        }
        else {
            upperLimit = nFrom+nCount;
        }
    }
    else if (NULL != strstr("previous", str_navbtn) ) {
        
        if ( 0 != nFrom ) {    
            
            nFrom -= nCount;
            upperLimit = nFrom+nCount;
            
            if ( nFrom < 0 ) {
                nFrom = 0;
                if ((nFrom-nCount) < 0) {
                    upperLimit = pObject->m_VSCP_Variables.m_listVariable.GetCount()- nFrom;
                }
                else {
                    upperLimit = nFrom-nCount;
                }
            }
            
            if (upperLimit < 0) {
                upperLimit = nCount;
            }
        }
        
    }
    else if (NULL != strstr("next",str_navbtn)) {

        if ( upperLimit < pObject->m_VSCP_Variables.m_listVariable.GetCount() ) {
            nFrom += nCount;
            if (nFrom >= pObject->m_VSCP_Variables.m_listVariable.GetCount()) {
                nFrom = pObject->m_VSCP_Variables.m_listVariable.GetCount() - nCount;
                if ( nFrom < 0 ) nFrom = 0;
            }
        
            if ((nFrom+nCount) > pObject->m_VSCP_Variables.m_listVariable.GetCount()) {
                upperLimit = pObject->m_VSCP_Variables.m_listVariable.GetCount();
            }
            else {
                upperLimit = nFrom+nCount;
            }
        }

    }
    else if (NULL != strstr("last",str_navbtn)) {
        nFrom = pObject->m_VSCP_Variables.m_listVariable.GetCount() - nCount;
        if ( nFrom < 0 ) {
            nFrom = 0;
            upperLimit = pObject->m_VSCP_Variables.m_listVariable.GetCount();
        }
        else {
            upperLimit = pObject->m_VSCP_Variables.m_listVariable.GetCount();
        }
    }

    wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Variables"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    // Insert server url into navigation menu 
    wxString navstr = _(WEB_COMMON_MENU);
    int pos;
    while ( wxNOT_FOUND != ( pos = navstr.Find(_("%s")))) {
        buildPage += navstr.Left( pos );
        navstr = navstr.Right(navstr.Length() - pos - 2);
    }
    buildPage += navstr;
    
    buildPage += _(WEB_VARLIST_BODY_START);
    
    {
        wxString wxstrurl = wxString::Format(_("%s/vscp/variables"), 
                                                strHost.GetData() );
        buildPage += wxString::Format( _(WEB_COMMON_LIST_NAVIGATION),
                wxstrurl.GetData(),
                nFrom,
                ((nFrom + nCount) < pObject->m_VSCP_Variables.m_listVariable.GetCount()) ? 
                    nFrom + nCount - 1 : pObject->m_VSCP_Variables.m_listVariable.GetCount() - 1,
                pObject->m_VSCP_Variables.m_listVariable.GetCount(),
                nCount,
                nFrom,
                _("false" ) );
        buildPage += _("<br>");
    } 

    wxString strBuf;

    // Display Variables List
    
    if ( 0 == pObject->m_VSCP_Variables.m_listVariable.GetCount() ) {
        buildPage += _("<br>Variables list is empty!<br>");
    }
    else {
        buildPage += _(WEB_VARLIST_TR_HEAD);
    }
    
    if (nFrom < 0) nFrom = 0;
    
    for ( int i=nFrom;i<upperLimit;i++) {
        
        CVSCPVariable *pVariable = 
                pObject->m_VSCP_Variables.m_listVariable.Item( i )->GetData();
        
        {
            wxString url_dmedit = 
                    wxString::Format(_("%s/vscp/varedit?id=%d"),
                                        strHost.GetData(),
                                        i );
            wxString str = wxString::Format(_(WEB_COMMON_TR_CLICKABLE_ROW),
                                                url_dmedit.GetData() );
            buildPage += str;
        }

        // Client id    
        buildPage += _(WEB_IFLIST_TD_CENTERED);
        buildPage += wxString::Format(_("<form name=\"input\" action=\"%s/vscp/vardelete?id=%d\" method=\"get\">%d<input type=\"submit\" value=\"x\"><input type=\"hidden\" name=\"id\"value=\"%d\"></form>"), 
                        strHost.GetData(), i, i, i );
        buildPage += _("</td>");
        
        if (NULL != pVariable) {

            // Variable type
            buildPage += _("<td>");
            switch (pVariable->getType()) {

            case VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED:
                buildPage += _("Unassigned");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_STRING:
                buildPage += _("String");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_BOOLEAN:
                buildPage += _("Boolean");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_INTEGER:
                buildPage += _("Integer");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_LONG:
                buildPage += _("Long");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_DOUBLE:
                buildPage += _("Double");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_MEASUREMENT:
                buildPage += _("Measurement");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT:
                buildPage += _("Event");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID:
                buildPage += _("GUID");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_DATA:
                buildPage += _("Event data");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_CLASS:
                buildPage += _("Event class");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TYPE:
                buildPage += _("Event type");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TIMESTAMP:
                buildPage += _("Event timestamp");
                break;

            case VSCP_DAEMON_VARIABLE_CODE_DATETIME:
                buildPage += _("Date and time");
                break;

            default:
                buildPage += _("Unknown type");
                break;

            }
            buildPage += _("</td>");

            
            // Variable entry
            buildPage += _("<td>");


            buildPage += _("<div id=\"small\">");

            buildPage += _("<h4>");
            buildPage += pVariable->getName();
            buildPage += _("</h4>");
            
            wxString str;
            pVariable->writeVariableToString(str);
            buildPage += _("<b>Value:</b> ");
            buildPage += str;
            
            buildPage += _("<br>");
            buildPage += _("<b>Note:</b> ");
            buildPage += pVariable->getNote();
            
            buildPage += _("<br>");
            buildPage += _("<b>Persistent: </b> ");
            if ( pVariable->isPersistent() ) {
                buildPage += _("yes");
            }
            else {
                buildPage += _("no");
            }

            buildPage += _("</div>");

        }
        else {
            buildPage += _("Internal error: Non existent variable entry.");
        }

        buildPage += _("</td>");
        buildPage += _("</tr>");

    }
       
    buildPage += _(WEB_DMLIST_TABLE_END);
    
    {
        wxString wxstrurl = wxString::Format(_("%s/vscp/variables"), 
                                                strHost.GetData() );
        buildPage += wxString::Format( _(WEB_COMMON_LIST_NAVIGATION),
                wxstrurl.GetData(),
                nFrom,
                ((nFrom + nCount) < pObject->m_VSCP_Variables.m_listVariable.GetCount()) ? 
                    nFrom + nCount - 1 : pObject->m_VSCP_Variables.m_listVariable.GetCount() - 1,
                pObject->m_VSCP_Variables.m_listVariable.GetCount(),
                nCount,
                nFrom,
                _("false") );
    }
     
    buildPage += _(WEB_COMMON_END);     // Common end code
    
    char *ppage = new char[ buildPage.Length() + 1 ];
    memset(ppage, 0, buildPage.Length() + 1 );
    memcpy( ppage, buildPage.ToAscii(), buildPage.Length() );        
    
    // return page 
    response = MHD_create_response_from_buffer( strlen(ppage),
                                                    (void *)ppage,
                                                    MHD_RESPMEM_MUST_FREE );
    
    websrv_add_session_cookie(session, response);
    
    MHD_add_response_header( response,
                                MHD_HTTP_HEADER_CONTENT_ENCODING,
                                mime);
    
    ret = MHD_queue_response( connection,
                                MHD_HTTP_OK,
                                response);
    
    MHD_destroy_response( response );
    
    return ret; 
}


///////////////////////////////////////////////////////////////////////////////
// websrv_serve_variables_edit
//

int 
CControlObject::websrv_serve_variables_edit( const void *cls,
                                                const char *mime,
                                                struct websrv_Session *session,
                                                struct MHD_Connection *connection)
{
    int ret,i;
    wxString str;
    VSCPInformation vscpinfo;
    CControlObject *pObject = (CControlObject *) cls;
    struct MHD_Response *response;
    CVSCPVariable *pVariable = NULL;
    
    // Get connection type
    const MHD_ConnectionInfo *pProtocolInfo = 
            MHD_get_connection_info(connection, 
                                        MHD_CONNECTION_INFO_PROTOCOL );
    // Get hostname
    wxString strHost = _("http://localhost:8080");
    const char *str_host = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "host");
    if ( NULL != str_host ) {
        strHost = wxString::FromAscii(str_host);
        if ( NULL != pProtocolInfo ) {
            strHost = _("https://") + strHost;
        }
        else {
            strHost = _("http://") + strHost;
        }
    }
    
    // id
    long id = -1;
    const char *str_id = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "id");
    if ( NULL != str_id ) id = atoi(str_id);
    
    // type
    uint8_t nType = VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED;
    const char *str_variable_type = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "type");
    if ( NULL != str_variable_type ) nType = atoi(str_variable_type);
    
    // Flag for new variable row
    bool bNew = false;
    const char *str_new = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "new");
    if ( (NULL != str_new) && NULL != strstr( "true", str_new ) ) bNew = true;
    
    wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Variable Edit"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    // Insert server url into navigation menu 
    wxString navstr = _(WEB_COMMON_MENU);
    int pos;
    while ( wxNOT_FOUND != ( pos = navstr.Find(_("%s") ) ) ) {
        buildPage += navstr.Left( pos );
        navstr = navstr.Right(navstr.Length() - pos - 2);
    }
    
    buildPage += navstr;
    
    buildPage += _(WEB_VAREDIT_BODY_START);

    if ( !bNew && ( id < pObject->m_VSCP_Variables.m_listVariable.GetCount() ) ) {
        pVariable = pObject->m_VSCP_Variables.m_listVariable.Item(id)->GetData();
    }

    if (bNew || (NULL != pVariable)) {
        
        if ( bNew ) {
            buildPage += _("<br><span id=\"optiontext\">New record.</span><br>");
        }
        else {
            buildPage += wxString::Format(_("<br><span id=\"optiontext\">Record = %d.</span><br>"), id);
        }
        
        buildPage += _("<br><form method=\"get\" action=\"");
        buildPage += strHost;
        buildPage += _("/vscp/varpost");
        buildPage += _("\" name=\"varedit\">");
        
        // Hidden id
        buildPage += wxString::Format(_("<input name=\"id\" value=\"%d\" type=\"hidden\">"), id );
        
        if (bNew) {     
            // Hidden new
            buildPage += _("<input name=\"new\" value=\"true\" type=\"hidden\">");
        }
        else {
            // Hidden new
            buildPage += _("<input name=\"new\" value=\"false\" type=\"hidden\">");
        }
        
        // Hidden type
        buildPage += _("<input name=\"type\" value=\"");
        buildPage += wxString::Format(_("%d"), ( bNew ? nType : pVariable->getType()) );
        buildPage += _("\" type=\"hidden\"></input>");
        
        buildPage += _("<h4>Variable:</h4> <span id=\"optiontext\"></span><br>");

        buildPage += _("<table class=\"invisable\"><tbody><tr class=\"invisable\">");

        buildPage += _("<td class=\"invisable\">Name:</td><td class=\"invisable\">");
        if ( !bNew ) {
            buildPage += pVariable->getName();
            buildPage += _("<input name=\"value_name\" value=\"");
            buildPage += pVariable->getName();
            buildPage += _("\" type=\"hidden\">");
        }
        else {
            buildPage += _("<textarea cols=\"50\" rows=\"1\" name=\"value_name\"></textarea>");
        }
        buildPage += _("</td></tr><tr>");
        buildPage += _("<td class=\"invisable\">Value:</td><td class=\"invisable\">");
        
        if (!bNew ) nType = pVariable->getType();
        
        if ( nType  == VSCP_DAEMON_VARIABLE_CODE_STRING ) {
            
            buildPage += _("<textarea cols=\"50\" rows=\"1\" name=\"value_string\">");
            if ( bNew ) {
                buildPage += _("");
            }
            else {
                wxString str;
                pVariable->getValue( &str );
                buildPage += str;
            }
            
            buildPage += _("</textarea>");
            
        }
        else if ( nType == VSCP_DAEMON_VARIABLE_CODE_BOOLEAN ) {
            
            bool bValue = false;
            if ( !bNew ) pVariable->getValue( &bValue );
            
            buildPage += _("<input type=\"radio\" name=\"value_boolean\" value=\"true\" ");
            if ( !bNew ) 
                buildPage += wxString::Format(_("%s"), 
                                bValue ? _("checked >true ") : _(">true ") );
            else {
                buildPage += _(">true ");
            }
            
            buildPage += _("<input type=\"radio\" name=\"value_boolean\" value=\"false\" ");
            if ( !bNew ) 
                buildPage += wxString::Format(_("%s"), 
                                        !bValue ? _("checked >false ") : _(">false ") );
            else {
                buildPage += _(">false ");
            }
        }
        else if ( nType == VSCP_DAEMON_VARIABLE_CODE_INTEGER ) {
            
            buildPage += _("<textarea cols=\"10\" rows=\"1\" name=\"value_integer\">");
            if ( bNew ) {
                buildPage += _("");
            }
            else {
                int val;
                pVariable->getValue( &val );
                buildPage += wxString::Format(_("%d"), val );
            }
            
            buildPage += _("</textarea>");
            
        }
        else if ( nType == VSCP_DAEMON_VARIABLE_CODE_LONG ) {
            
            buildPage += _("<textarea cols=\"10\" rows=\"1\" name=\"value_long\">");
            if ( bNew ) {
                buildPage += _("");
            }
            else {
                long val;
                pVariable->getValue( &val );
                buildPage += wxString::Format(_("%ld"), val );
            }
            
            buildPage += _("</textarea>");
            
        }
        else if ( nType == VSCP_DAEMON_VARIABLE_CODE_DOUBLE ) {
            
            buildPage += _("<textarea cols=\"10\" rows=\"1\" name=\"value_double\">");
            if ( bNew ) {
                buildPage += _("");
            }
            else {
                double val;
                pVariable->getValue( &val );
                buildPage += wxString::Format(_("%f"), val );
            }
            
            buildPage += _("</textarea>");
            
        }
        else if ( nType == VSCP_DAEMON_VARIABLE_CODE_VSCP_MEASUREMENT ) {
            buildPage += _("<textarea cols=\"50\" rows=\"1\" name=\"value_measurement\">");
            if ( bNew ) {
                buildPage += _("");
            }
            else {
                wxString str;
                pVariable->writeVariableToString( str );
                buildPage += str;
            }
            
            buildPage += _("</textarea>");
        }
        else if ( nType == VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT ) {
            
            buildPage += _("<table>");
            
            buildPage += _("<tr><td>");
            buildPage += _("VSCP class");
             buildPage += _("</td><td>");
            buildPage += _("<textarea cols=\"10\" rows=\"1\" name=\"value_class\">");
            if ( bNew ) {
                buildPage += _("");
            }
            else {
                buildPage += wxString::Format(_("0x%x"), pVariable->m_event.vscp_class );
            }
            
            buildPage += _("</textarea>");
            buildPage += _("</td></tr>");
            
            buildPage += _("<tr><td>");
            buildPage += _("VSCP type: ");
            buildPage += _("</td><td>");
            buildPage += _("<textarea cols=\"10\" rows=\"1\" name=\"value_type\">");
            if ( bNew ) {
                buildPage += _("");
            }
            else {
                buildPage += wxString::Format(_("0x%x"), pVariable->m_event.vscp_type );
            }
            
            buildPage += _("</textarea>");
            buildPage += _("</td></tr>");
            
            buildPage += _("<tr><td>");
            buildPage += _("GUID: ");
            buildPage += _("</td><td>");
            buildPage += _("<textarea cols=\"50\" rows=\"1\" name=\"value_guid\">");
            if ( bNew ) {
                buildPage += _("");
            }
            else {
                wxString strGUID;
                writeGuidArrayToString( pVariable->m_event.GUID, strGUID );
                buildPage += wxString::Format(_("%s"), strGUID.GetData() );
            }
            
            buildPage += _("</textarea>");
            buildPage += _("</td></tr>");
            
            buildPage += _("<tr><td>");
            buildPage += _("Timestamp: ");
            buildPage += _("</td><td>");
            buildPage += _("<textarea cols=\"10\" rows=\"1\" name=\"value_timestamp\">");
            if ( bNew ) {
                buildPage += _("0");
            }
            else {
                buildPage += wxString::Format(_("0x%x"), pVariable->m_event.timestamp );
            }
            
            buildPage += _("</textarea>");
            buildPage += _("</td></tr>");
            
            buildPage += _("</textarea>");
            buildPage += _("</td></tr>");
            
            buildPage += _("<tr><td>");
            buildPage += _("OBID: ");
            buildPage += _("</td><td>");
            buildPage += _("<textarea cols=\"10\" rows=\"1\" name=\"value_obid\">");
            if ( bNew ) {
                buildPage += _("0");
            }
            else {
                buildPage += wxString::Format(_("0x%X"), pVariable->m_event.obid );
            }
            
            buildPage += _("</textarea>");
            buildPage += _("</td></tr>");
            
            buildPage += _("</textarea>");
            buildPage += _("</td></tr>");
            
            buildPage += _("</textarea>");
            buildPage += _("</td></tr>");
            
            buildPage += _("<tr><td>");
            buildPage += _("Head: ");
            buildPage += _("</td><td>");
            buildPage += _("<textarea cols=\"10\" rows=\"1\" name=\"value_head\">");
            if ( bNew ) {
                buildPage += _("0");
            }
            else {
                buildPage += wxString::Format(_("0x%02x"), pVariable->m_event.head );
            }
            
            buildPage += _("</textarea>");
            buildPage += _("</td></tr>");
            
            buildPage += _("<tr><td>");
            buildPage += _("CRC: ");
            buildPage += _("</td><td>");
            buildPage += _("<textarea cols=\"10\" rows=\"1\" name=\"value_crc\">");
            if ( bNew ) {
                buildPage += _("0");
            }
            else {
                buildPage += wxString::Format(_("0x%08x"), pVariable->m_event.crc );
            }
            
            buildPage += _("</textarea>");
            buildPage += _("</td></tr>");
            
            buildPage += _("</textarea>");
            buildPage += _("</td></tr>");
            
            buildPage += _("<tr><td>");
            buildPage += _("Data size: ");
            buildPage += _("</td><td>");
            buildPage += _("<textarea cols=\"10\" rows=\"1\" name=\"value_sizedata\">");
            if ( bNew ) {
                buildPage += _("0");
            }
            else {
                buildPage += wxString::Format(_("%d"), pVariable->m_event.sizeData );
            }
            
            buildPage += _("</textarea>");
            buildPage += _("</td></tr>");
            
            buildPage += _("<tr><td>");
            buildPage += _("Data: ");
            buildPage += _("</td><td>");
            buildPage += _("<textarea cols=\"50\" rows=\"4\" name=\"value_data\">");
            if ( bNew ) {
                buildPage += _("0");
            }
            else {
                wxString strData;
                writeVscpDataToString( &pVariable->m_event, strData );
                buildPage += wxString::Format(_("%s"), strData.GetData() );
            }
            
            buildPage += _("</textarea>");
            buildPage += _("</td></tr>");
            
            buildPage += _("</table>");
            
        }
        else if ( nType == VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID ) {
            
            buildPage += _("<textarea cols=\"50\" rows=\"1\" name=\"value_guid\">");
            if ( bNew ) {
                buildPage += _("");
            }
            else {
                wxString strGUID;
                pVariable->writeVariableToString(strGUID);
                buildPage += strGUID;
            }
            
            buildPage += _("</textarea>");
            
        }
        else if ( nType == VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_DATA ) {
            
            buildPage += _("<textarea cols=\"50\" rows=\"5\" name=\"value_data\">");
            if ( bNew ) {
                buildPage += _("");
            }
            else {
                wxString strData;
                writeVscpDataToString( &pVariable->m_event, strData );
                buildPage += strData.GetData();
            }
            
            buildPage += _("</textarea>");
            
        }
        else if ( nType == VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_CLASS ) {
            
            buildPage += _("<textarea cols=\"10\" rows=\"1\" name=\"value_class\">");
            if ( bNew ) {
                buildPage += _("");
            }
            else {
                wxString str;
                pVariable->writeVariableToString( str );
                buildPage += str;
            }
            
            buildPage += _("</textarea>");
            
        }
        else if ( nType == VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TYPE ) {
            
            buildPage += _("<textarea cols=\"10\" rows=\"1\" name=\"value_type\">");
            if ( bNew ) {
                buildPage += _("");
            }
            else {
                wxString str;
                pVariable->writeVariableToString( str );
                buildPage += str;
            }
            
            buildPage += _("</textarea>");
            
        }
        else if ( nType == VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TIMESTAMP ) {
            
            buildPage += _("<textarea cols=\"10\" rows=\"1\" name=\"value_timestamp\">");
            if ( bNew ) {
                buildPage += _("");
            }
            else {
                wxString str;
                pVariable->writeVariableToString( str );
                buildPage += str;
            }
            
            buildPage += _("</textarea>");
            
        }
        else if ( nType == VSCP_DAEMON_VARIABLE_CODE_DATETIME ) {
            
            buildPage += _("<textarea cols=\"20\" rows=\"1\" name=\"value_date_time\">");
            if ( bNew ) {
                buildPage += _("");
            }
            else {
                wxString str;
                pVariable->writeVariableToString( str );
                buildPage += str;
            }
            
            buildPage += _("</textarea>");
            
        }
        else {
            // Invalid type
            buildPage += _("Invalid type - Something is very wrong!");
        }
        
        
        buildPage += _("</tr><tr><td>Persistence: </td><td>");

        buildPage += _("<input type=\"radio\" name=\"persistent\" value=\"true\" ");
        
        if ( !bNew ) {
            buildPage += wxString::Format(_("%s"), 
                pVariable->isPersistent() ? _("checked >Persistent ") : _(">Persistent ") );
        }
        else {
            buildPage += _("checked >Persistent ");
        }
        
         buildPage += _("<input type=\"radio\" name=\"persistent\" value=\"false\" ");
         
        if ( !bNew ) {
            buildPage += wxString::Format(_("%s"), 
                !pVariable->isPersistent() ? _("checked >Non persistent ") : _(">Non persistent ") );
        }
        else {
            buildPage += _(">Non persistent ");
        }
        
        buildPage += _("</td></tr>");
        
        
        buildPage += _("</tr><tr><td>Note: </td><td>");
        buildPage += _("<textarea cols=\"50\" rows=\"5\" name=\"note\">");
        if (bNew) {
            buildPage += _("");
        } 
        else {
            buildPage += pVariable->getNote();
        }

        buildPage += _("</textarea>");

        buildPage += _("</td></tr></table>");

        buildPage += _(WEB_VAREDIT_TABLE_END);

        //buildPage += _(WEB_VAREDIT_SUBMIT);
    } 
    else {
        buildPage += _("<br><b>Error: Non existent id</b>");
    }
    
    
    wxString wxstrurl = wxString::Format(_("%s/vscp/varpost"), 
                                                strHost.GetData() );
    buildPage += wxString::Format( _(WEB_VAREDIT_SUBMIT),
                                    wxstrurl.GetData() );
    
    buildPage += _("</form>");
    buildPage += _(WEB_COMMON_END);     // Common end code
    
    char *ppage = new char[ buildPage.Length() + 1 ];
    memset(ppage, 0, buildPage.Length() + 1 );
    memcpy( ppage, buildPage.ToAscii(), buildPage.Length() );        
    
    // return page 
    response = MHD_create_response_from_buffer( strlen(ppage),
                                                    (void *)ppage,
                                                    MHD_RESPMEM_MUST_FREE );
    
    websrv_add_session_cookie(session, response);
    
    MHD_add_response_header( response,
                                MHD_HTTP_HEADER_CONTENT_ENCODING,
                                mime);
    
    ret = MHD_queue_response( connection,
                                MHD_HTTP_OK,
                                response);
    
    MHD_destroy_response( response );
    
    return ret;
}



///////////////////////////////////////////////////////////////////////////////
// websrv_serve_variables_post
//

int 
CControlObject::websrv_serve_variables_post( const void *cls,
                                                const char *mime,
                                                struct websrv_Session *session,
                                                struct MHD_Connection *connection)
{
    int ret,i;
    wxString str;
    VSCPInformation vscpinfo;
    CControlObject *pObject = (CControlObject *) cls;
    struct MHD_Response *response;
    CVSCPVariable *pVariable = NULL;
    
    // Get connection type
    const MHD_ConnectionInfo *pProtocolInfo = 
            MHD_get_connection_info(connection, 
                                        MHD_CONNECTION_INFO_PROTOCOL );
    // Get hostname
    wxString strHost = _("http://localhost:8080");
    const char *str_host = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "host");
    if ( NULL != str_host ) {
        strHost = wxString::FromAscii(str_host);
        if ( NULL != pProtocolInfo ) {
            strHost = _("https://") + strHost;
        }
        else {
            strHost = _("http://") + strHost;
        }
    }
    
    // id
    long id = -1;
    const char *str_id = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "id");
    if ( NULL != str_id ) id = atoi(str_id);
    
    uint8_t nType = VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED;
    const char *str_variable_type = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "type");
    if ( NULL != str_variable_type ) nType = atoi(str_variable_type);
    
    wxString strName;
    const char *str_name = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "value_name");
    if ( NULL != str_name ) strName = wxString::FromAscii(str_name);
    
    // Flag for new variable row
    bool bNew = false;
    const char *str_new = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "new");
    if ( (NULL != str_new) && NULL != strstr( "true", str_new ) ) bNew = true;
    
    // Flag for persistence
    bool bPersistent = true;
    const char *str_persistent = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "persistent");
    if ( (NULL != str_persistent) && NULL != strstr( "false", str_persistent ) ) bPersistent = false;

    wxString strNote;
    const char *str_note = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "note");
    if ( NULL != str_note ) strNote = wxString::FromAscii(str_note);
    
    wxString strValueString;
    const char *str_string = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "value_string");
    if ( NULL != str_string ) strValueString = wxString::FromAscii(str_string);
    
           
    bool bValueBoolean = false;
    const char *str_boolean = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "value_boolean");
    if ( (NULL != str_boolean) && NULL != strstr( "true", str_boolean ) ) bValueBoolean = true;
    
    int value_integer = 0;
    const char *str_integer = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "value_integer");
    str = wxString::FromAscii(str_integer);
    if ( NULL != str_integer ) value_integer = readStringValue( str );
       
    long value_long = 0;
    const char *str_long = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "value_long");
    str = wxString::FromAscii(str_long);
    if ( NULL != str_long ) value_long = readStringValue( str );
    
    double value_double = 0;
    const char *str_double = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "value_double");
    if ( NULL != str_double ) value_double = atof( str_double );
    
    wxString strMeasurement;
    const char *str_measurement = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "value_measurement");
    if ( NULL != str_measurement ) strMeasurement = wxString::FromAscii(str_measurement);
    
    uint16_t value_class = 0;
    const char *str_class = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "value_class");
    if ( NULL != str_class ) value_class = readStringValue( wxString::FromAscii(str_class) );
    
    uint16_t value_type = 0;
    const char *str_type = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "value_type");
    if ( NULL != str_type ) value_type = readStringValue( wxString::FromAscii(str_type) );
    
    wxString strGUID;
    const char *str_guid = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "value_guid");
    if ( NULL != str_guid ) strGUID = wxString::FromAscii(str_guid);
    
    uint32_t value_timestamp = 0;
    const char *str_timestamp = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "value_timestamp");
    if ( NULL != str_timestamp ) value_timestamp = readStringValue( wxString::FromAscii(str_timestamp) );
    
    uint32_t value_obid = 0;
    const char *str_obid = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "value_obid");
    if ( NULL != str_obid ) value_obid = readStringValue( wxString::FromAscii(str_obid) );
    
    uint8_t value_head = 0;
    const char *str_head = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "value_head");
    if ( NULL != str_head ) value_head = readStringValue( wxString::FromAscii(str_head) );
    
    uint32_t value_crc = 0;
    const char *str_crc = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "value_crc");
    if ( NULL != str_crc ) value_crc = readStringValue( wxString::FromAscii(str_crc) );
    
    uint16_t value_sizedata = 0;
    const char *str_sizedata = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "value_sizedata");
    if ( NULL != str_sizedata ) value_sizedata = readStringValue( wxString::FromAscii(str_sizedata) );
    
    wxString strData;
    const char *str_data = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "value_data");
    if ( NULL != str_data ) strData = wxString::FromAscii(str_data);
    
    wxString strDateTime;
    const char *str_date_time = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "value_date_time");
    if ( NULL != str_date_time ) strDateTime = wxString::FromAscii(str_date_time);
    
    wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Variable Post"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += wxString::Format(_("<meta http-equiv=\"refresh\" content=\"2;url=%s/vscp/variables"), strHost.GetData() );
    buildPage += wxString::Format(_("?from=%d"), id );
    buildPage += _("\">");
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    // Insert server url into navigation menu 
    wxString navstr = _(WEB_COMMON_MENU);
    int pos;
    while ( wxNOT_FOUND != ( pos = navstr.Find(_("%s")))) {
        buildPage += navstr.Left( pos );
        navstr = navstr.Right(navstr.Length() - pos - 2);
    }
    buildPage += navstr;
    
    buildPage += _(WEB_VARPOST_BODY_START);
        
    if (bNew) {
        pVariable = new CVSCPVariable;
    }

    if (bNew || (id >= 0)) {

        if (bNew || 
                ((0 == id) && !bNew) || 
                (id < pObject->m_VSCP_Variables.m_listVariable.GetCount()) ) {

            if (!bNew) pVariable = pObject->m_VSCP_Variables.m_listVariable.Item(id)->GetData();

            if (NULL != pVariable) {

                // Set the type
                pVariable->setPersistent( bPersistent );
                pVariable->setType( nType );
                pVariable->m_note = strNote;
                pVariable->setName( strName );
                
                switch ( nType ) {

                case VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED:
                    buildPage += _("Error: Variable code is unassigned.<br>");
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_STRING:
                    pVariable->setValue( strValueString );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_BOOLEAN:
                    pVariable->setValue( bValueBoolean );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_INTEGER:
                    pVariable->setValue( value_integer );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_LONG:
                    pVariable->setValue( value_long );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_DOUBLE:
                    pVariable->setValue( value_double );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_VSCP_MEASUREMENT:
                    uint16_t size;
                    getVscpDataArrayFromString( pVariable->m_normInteger, 
                                                    &size,
                                                    strMeasurement );
                    pVariable->m_normIntSize = size;
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT:
                    pVariable->m_event.vscp_class = value_class;
                    pVariable->m_event.vscp_type = value_type;
                    getGuidFromStringToArray( pVariable->m_event.GUID, strGUID );
                    getVscpDataFromString( &pVariable->m_event,
                                            strData );
                    pVariable->m_event.crc = value_crc;
                    pVariable->m_event.head = value_head;
                    pVariable->m_event.obid = value_obid;
                    pVariable->m_event.timestamp = value_timestamp;        
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID:
                    getGuidFromStringToArray( pVariable->m_event.GUID, strGUID );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_DATA:
                    getVscpDataFromString( &pVariable->m_event,
                                            strData );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_CLASS:
                    pVariable->m_event.vscp_class = value_class;
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TYPE:
                    pVariable->m_event.vscp_type = value_type;
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_TIMESTAMP:
                    pVariable->m_event.timestamp = value_timestamp;
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_DATETIME:
                    pVariable->m_timestamp.ParseDateTime( strDateTime );
                    break;

                default:
                    buildPage += _("Error: Variable code is unknown.<br>");
                    break;

                }
                
                // If new variable add it
                if (bNew ) {
                    pObject->m_VSCP_Variables.add( pVariable );
                }

                // Save variables
                pObject->m_VSCP_Variables.save();
                
                buildPage += wxString::Format(_("<br><br>Variable has been saved. id=%d"), id);

            }
            else {
                buildPage += wxString::Format(_("<br><br>Memory problem id=%d. Unable to save record"), id);
            }

        } 
        else {
            buildPage += wxString::Format(_("<br><br>Record id=%d is to large. Unable to save record"), id);
        }
    } 
    else {
        buildPage += wxString::Format(_("<br><br>Record id=%d is wrong. Unable to save record"), id);
    }


    buildPage += _(WEB_COMMON_END); // Common end code 
    
    char *ppage = new char[ buildPage.Length() + 1 ];
    memset(ppage, 0, buildPage.Length() + 1 );
    memcpy( ppage, buildPage.ToAscii(), buildPage.Length() );        
    
    // return page 
    response = MHD_create_response_from_buffer( strlen(ppage),
                                                    (void *)ppage,
                                                    MHD_RESPMEM_MUST_FREE );
    
    websrv_add_session_cookie(session, response);
    
    MHD_add_response_header( response,
                                MHD_HTTP_HEADER_CONTENT_ENCODING,
                                mime);
    
    ret = MHD_queue_response( connection,
                                MHD_HTTP_OK,
                                response);
    
    MHD_destroy_response( response );
    
    return ret;
}


///////////////////////////////////////////////////////////////////////////////
// websrv_serve_variables_delete
//

int 
CControlObject::websrv_serve_variables_delete( const void *cls,
                                                const char *mime,
                                                struct websrv_Session *session,
                                                struct MHD_Connection *connection)
{
    int ret,i;
    wxString str;
    VSCPInformation vscpinfo;
    CControlObject *pObject = (CControlObject *) cls;
    struct MHD_Response *response;
    CVSCPVariable *pVariable = NULL;
 
       // Get connection type
    const MHD_ConnectionInfo *pProtocolInfo = 
            MHD_get_connection_info(connection, 
                                        MHD_CONNECTION_INFO_PROTOCOL );
    // Get hostname
    wxString strHost = _("http://localhost:8080");
    const char *str_host = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "host");
    if ( NULL != str_host ) {
        strHost = wxString::FromAscii(str_host);
        if ( NULL != pProtocolInfo ) {
            strHost = _("https://") + strHost;
        }
        else {
            strHost = _("http://") + strHost;
        }
    }
    
    // id
    long id = -1;
    const char *str_id = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "id");
    if ( NULL != str_id ) id = atoi(str_id);
        
    wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Variable Delete"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += wxString::Format(_("<meta http-equiv=\"refresh\" content=\"2;url=%s/vscp/variables"), strHost.GetData() );
    buildPage += wxString::Format(_("?from=%d"), id + 1 );
    buildPage += _("\">");
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    // Insert server url into navigation menu 
    wxString navstr = _(WEB_COMMON_MENU);
    int pos;
    while ( wxNOT_FOUND != ( pos = navstr.Find(_("%s")))) {
        buildPage += navstr.Left( pos );
        navstr = navstr.Right(navstr.Length() - pos - 2);
    }
    buildPage += navstr;
    
    buildPage += _(WEB_VAREDIT_BODY_START);
    
    wxlistVscpVariableNode *node = 
            pObject->m_VSCP_Variables.m_listVariable.Item( id );
    if ( pObject->m_VSCP_Variables.m_listVariable.DeleteNode( node )  ) {
        buildPage += wxString::Format(_("<br>Deleted record id = %d"), id);
        // Save variables
        pObject->m_VSCP_Variables.save();
    }
    else {
        buildPage += wxString::Format(_("<br>Failed to remove record id = %d"), id);
    }
    
    buildPage += _(WEB_COMMON_END);     // Common end code
    
    char *ppage = new char[ buildPage.Length() + 1 ];
    memset(ppage, 0, buildPage.Length() + 1 );
    memcpy( ppage, buildPage.ToAscii(), buildPage.Length() );        
    
    // return page 
    response = MHD_create_response_from_buffer( strlen(ppage),
                                                    (void *)ppage,
                                                    MHD_RESPMEM_MUST_FREE );
    
    websrv_add_session_cookie(session, response);
    
    MHD_add_response_header( response,
                                MHD_HTTP_HEADER_CONTENT_ENCODING,
                                mime);
    
    ret = MHD_queue_response( connection,
                                MHD_HTTP_OK,
                                response);
    
    MHD_destroy_response( response );
    
    return ret;
}


///////////////////////////////////////////////////////////////////////////////
// websrv_serve_variables_new
//

int 
CControlObject::websrv_serve_variables_new( const void *cls,
                                                const char *mime,
                                                struct websrv_Session *session,
                                                struct MHD_Connection *connection)
{
    int ret,i;
    wxString str;
    VSCPInformation vscpinfo;
    CControlObject *pObject = (CControlObject *) cls;
    struct MHD_Response *response;
    CVSCPVariable *pVariable = NULL;
 
       // Get connection type
    const MHD_ConnectionInfo *pProtocolInfo = 
            MHD_get_connection_info(connection, 
                                        MHD_CONNECTION_INFO_PROTOCOL );
    // Get hostname
    wxString strHost = _("http://localhost:8080");
    const char *str_host = MHD_lookup_connection_value(connection, MHD_HEADER_KIND, "host");
    if ( NULL != str_host ) {
        strHost = wxString::FromAscii(str_host);
        if ( NULL != pProtocolInfo ) {
            strHost = _("https://") + strHost;
        }
        else {
            strHost = _("http://") + strHost;
        }
    }
    
        
    wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - New variable"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    // Insert server url into navigation menu 
    wxString navstr = _(WEB_COMMON_MENU);
    int pos;
    while ( wxNOT_FOUND != ( pos = navstr.Find(_("%s")))) {
        buildPage += navstr.Left( pos );
        navstr = navstr.Right(navstr.Length() - pos - 2);
    }
    buildPage += navstr;
    
    buildPage += _(WEB_VAREDIT_BODY_START);

    buildPage += _("<br><div style=\"text-align:center\">");
    
    buildPage += _("<br><form method=\"get\" action=\"");
    buildPage += strHost;
    buildPage += _("/vscp/varedit");
    buildPage += _("\" name=\"varnewstep1\">");
    
    buildPage += _("<input name=\"new\" value=\"true\" type=\"hidden\">");
    
    buildPage += _("<select name=\"type\">");
    buildPage += _("<option value=\"1\">String value</option>");
    buildPage += _("<option value=\"2\">Boolean value</option>");
    buildPage += _("<option value=\"3\">Integer value</option>");
    buildPage += _("<option value=\"4\">Long value</option>");
    buildPage += _("<option value=\"5\">Floating point value</option>");
    buildPage += _("<option value=\"6\">VSCP data coding</option>");
    buildPage += _("<option value=\"7\">VSCP event (Level II)</option>");
    buildPage += _("<option value=\"8\">VSCP event GUID</option>");
    buildPage += _("<option value=\"9\">VSCP event data</option>");
    buildPage += _("<option value=\"10\">VSCP event class</option>");
    buildPage += _("<option value=\"11\">VSCP event type</option>");
    buildPage += _("<option value=\"12\">VSCP event timestamp</option>");
    buildPage += _("<option value=\"13\">Date + Time in iso format</option>");
    buildPage += _("</select>");
    
    buildPage += _("<br></div>");
    buildPage += _(WEB_VARNEW_SUBMIT);
    //wxString wxstrurl = wxString::Format(_("%s/vscp/varedit?new=true"), 
    //                                            strHost.GetData() );
    //buildPage += wxString::Format( _(WEB_VARNEW_SUBMIT),
    //                                wxstrurl.GetData() );
    
    buildPage += _("</form>");
    
    buildPage += _(WEB_COMMON_END); // Common end code
    
    char *ppage = new char[ buildPage.Length() + 1 ];
    memset(ppage, 0, buildPage.Length() + 1 );
    memcpy( ppage, buildPage.ToAscii(), buildPage.Length() );        
    
    // return page 
    response = MHD_create_response_from_buffer( strlen(ppage),
                                                    (void *)ppage,
                                                    MHD_RESPMEM_MUST_FREE );
    
    websrv_add_session_cookie(session, response);
    
    MHD_add_response_header( response,
                                MHD_HTTP_HEADER_CONTENT_ENCODING,
                                mime);
    
    ret = MHD_queue_response( connection,
                                MHD_HTTP_OK,
                                response);
    
    MHD_destroy_response( response );
    
    return ret;
}

///////////////////////////////////////////////////////////////////////////////
// request_completed_callback
//

int
CControlObject::websrv_post_iterator( void *cls,
                                        enum MHD_ValueKind kind,
                                        const char *key,
                                        const char *filename,
                                        const char *content_type,
                                        const char *transfer_encoding,
                                        const char *data, 
                                        uint64_t off, 
                                        size_t size )
{
    struct websrv_Request *request = (struct websrv_Request *)cls;
    struct websrv_Session *session = request->session;

    if ( 0 == strcmp("DONE", key) ) {
        //fprintf(stderr,
        //        "Session `%s' submitted `%s', `%s'\n",
        //        session->m_sid,
        //        session->value_1,
        //        session->value_2);
        return MHD_YES;
    }
    
    if (0 == strcmp("v1", key)) {
        
        if ( size + off > sizeof( session->value_1 ) )
            size = sizeof( session->value_1 ) - off;
        
        memcpy( &session->value_1[ off ],
                data,
                size );
        
        if ( size + off < sizeof( session->value_1 ) ) {
            session->value_1[ size + off ] = '\0';
        }
        
        return MHD_YES;
    }
    
    if ( 0 == strcmp("v2", key) ) {
    
        if ( size + off > sizeof( session->value_2 ) ) {
            size = sizeof( session->value_2 ) - off;
        }
        
        memcpy( &session->value_2[off],
                data,
                size );
        
        if ( size + off < sizeof( session->value_2 ) )
            session->value_2[ size + off ] = '\0';
        
        return MHD_YES;
    }
    
    syslog(LOG_ERR, "Unsupported form value `%s'\n", key);
    return MHD_YES;
}

///////////////////////////////////////////////////////////////////////////////
// websrv_request_callback_completed
//

void
CControlObject::websrv_request_callback_completed(void *cls,
                                            struct MHD_Connection *connection,
                                            void **con_cls,
                                            enum MHD_RequestTerminationCode toe)
{
    struct websrv_Request *request = (struct websrv_Request *) *con_cls;

    if (NULL == request) {
        return;
    }
    
    if (NULL != request->session) {
        request->session->m_referenceCount--;
    }
    
    if (NULL != request->pp) {
        MHD_destroy_post_processor(request->pp);
    }
    
    free(request);
}

#endif



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
        if (wxSEMA_TIMEOUT ==
                m_pCtrlObject->m_semClientOutputQueue.WaitTimeout(500)) continue;

        if (m_pCtrlObject->m_clientOutputQueue.GetCount()) {
            
            m_pCtrlObject->m_mutexClientOutputQueue.Lock();
            nodeVSCP = m_pCtrlObject->m_clientOutputQueue.GetFirst();
            pvscpEvent = nodeVSCP->GetData();
            m_pCtrlObject->m_clientOutputQueue.DeleteNode(nodeVSCP);
            m_pCtrlObject->m_mutexClientOutputQueue.Unlock();

            if ((NULL != pvscpEvent)) {
                // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
                // * * * * Send event to all Level II clients (not to ourself )  * * * *
                // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

                m_pCtrlObject->sendEventAllClients(pvscpEvent, pvscpEvent->obid);

            } // Valid event

            // Delete the event
            if (NULL != pvscpEvent) deleteVSCPevent(pvscpEvent);

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


