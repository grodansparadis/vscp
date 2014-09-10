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

#include "wx/wx.h"
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

#include "../../common/slre.h"
#include "../../common/frozen.h"
#include "../../common/net_skeleton.h"
#include "../../common/mongoose.h"

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


#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif


// List for websocket triggers
WX_DEFINE_LIST(TRIGGERLIST);

WX_DEFINE_LIST(CanalMsgList);
WX_DEFINE_LIST(VSCPEventList);


///////////////////////////////////////////////////
//		          WEBSERVER
///////////////////////////////////////////////////



// Linked list of all active sessions. (webserv.h)
static struct websrv_Session *websrv_sessions;

// Session structure for REST API
struct websrv_rest_session *websrv_rest_sessions;

///////////////////////////////////////////////////
//					WEBSOCKETS
///////////////////////////////////////////////////

// Linked list of websocket sessions
static struct websock_session *websock_sessions;




//#define DEBUGPRINT

static CControlObject *gpctrlObj;

#ifdef WIN32

typedef struct _ASTAT_ {
    ADAPTER_STATUS adapt;
    NAME_BUFFER NameBuff [30];

} ASTAT, * PASTAT;

ASTAT Adapter;


WORD wVersionRequested = MAKEWORD(1, 1);	// WSA functions
WSADATA wsaData;							// WSA functions

#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CControlObject::CControlObject()
{
    int i;
    m_bQuit = false;	// true if we should quit
    gpctrlObj = this;	// needed by websocket static callbacks

    m_maxItemsInClientReceiveQueue = MAX_ITEMS_CLIENT_RECEIVE_QUEUE;

    // Nill the GUID
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

	// Loal doamin
	m_authDomain = _("mydomain.com");

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
    //m_portWebsockets = 7681;	// Deprecated
    m_bWebSockets = true;		// websocket interface ia active
	m_bAuthWebsockets = true;	// Authentication is needed
    m_pathCert.Empty();
    m_pathKey.Empty();
    
    // Webserver interface
    m_portWebServer = 8080;
    m_bWebServer = true;

#ifdef WIN32
	m_pathRoot = _("/programdata/vscp/www");
#else
	m_pathRoot = _("/srv/vscp/www");
#endif

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

#endif

    // Initialize the CRC
    crcInit();

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
		
		// Create the server
		m_pwebserver = mg_create_server( gpctrlObj, CControlObject::websrv_event_handler );
		
		// Set options
		mg_set_option( m_pwebserver, "document_root", m_pathRoot.mb_str( wxConvUTF8 ) );		// Serve current directory
		str = wxString::Format(  _("%i"), m_portWebServer );
		mg_set_option( m_pwebserver, "listening_port", "8080" /*str.mb_str( wxConvUTF8 )*/ );				// Open web server port
		mg_set_option( m_pwebserver, "auth_domain", m_authDomain.mb_str( wxConvUTF8 ) );
		mg_set_option( m_pwebserver, "ssl_certificate", m_pathCert.mb_str( wxConvUTF8 ) );		// SSL certificat
		

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
    
    // DM Loop
    while (!m_bQuit) {

#ifdef WIN32
		// CLOCKS_PER_SEC 
		clock_t ticks,oldus;
		oldus = ticks = clock();
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

		mg_poll_server( m_pwebserver, 10 );
		websock_post_incomingEvents();


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

#ifdef WIN32
		if ( (ticks - oldus) > 50000) 	
#else
        if ( ( (unsigned int)tv.tv_usec - oldus) > 50000) 
#endif
		{     
            if (m_bWebSockets) {
   
            }

#ifdef WIN32
			oldus = clock();
#else
            oldus = tv.tv_usec;
#endif

        }


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

                if (vscp_doLevel2Filter(pEvent, &m_dm.m_DM_Table_filter)) {
                    // Feed event through matrix
                    m_dm.feed(pEvent);
                }

                // Remove the event
                vscp_deleteVSCPevent(pEvent);

            } // Valid pEvent pointer

        } // Event in queue

    }

    // Do shutdown event
    m_dm.feed(&EventShutDown);

    // Remove messages in the client queues
    m_wxClientMutex.Lock();
    removeClient(pClientItem);
    m_wxClientMutex.Unlock();

    wxLogDebug(_("ControlObject: Done"));
    return true;
}


/////////////////////////////////////////////////////////////////////////////
// cleanup

bool CControlObject::cleanup(void)
{
	// Kill w�b server
	mg_destroy_server( &m_pwebserver );

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
        // Overrun
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
                    m_TCPPort = vscp_readStringValue(str);
                }// Deprecated <==============
                else if (subchild->GetName() == wxT("udpport")) {
                    wxString str = subchild->GetNodeContent();
                    m_UDPPort = vscp_readStringValue(str);
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
                        m_logLevel = vscp_readStringValue(str);
                    }
                } else if (subchild->GetName() == wxT("tcpif")) {
#if wxMAJOR_VERSION > 3                    
                    wxString property = subchild->GetAttribute(wxT("enable"), wxT("true"));
#else
                    wxString property = subchild->GetPropVal(wxT("enable"), wxT("true"));
#endif                    
                    if (property.IsSameAs(_("false"), false)) {
                        m_bTCPInterface = false;
                    }

#if wxMAJOR_VERSION > 3
                    property = subchild->GetAttribute(wxT("port"), wxT("9598"));
#else
                    property = subchild->GetPropVal(wxT("port"), wxT("9598"));
#endif                    
                    if (property.IsNumber()) {
                        m_TCPPort = vscp_readStringValue(property);
                    }

#if wxMAJOR_VERSION > 3                    
                    m_strTcpInterfaceAddress = subchild->GetAttribute(wxT("ifaddress"), wxT(""));
#else
                    m_strTcpInterfaceAddress = subchild->GetPropVal(wxT("ifaddress"), wxT(""));
#endif                    

                } else if (subchild->GetName() == wxT("canaldriver")) {
#if wxMAJOR_VERSION > 3                    
                    wxString property = subchild->GetAttribute(wxT("enable"), wxT("true"));
#else 
                    wxString property = subchild->GetPropVal(wxT("enable"), wxT("true"));
#endif                    
                    if (property.IsSameAs(_("false"), false)) {
                        m_bCanalDrivers = false;
                    } else {
                        m_bCanalDrivers = true;
                    }
                } 
                else if (subchild->GetName() == wxT("dm")) {
                    // Should the internal DM be disabled
#if wxMAJOR_VERSION > 3                    
                    wxString property = subchild->GetAttribute(wxT("enable"), wxT("true"));
#else 
                    wxString property = subchild->GetPropVal(wxT("enable"), wxT("true"));
#endif                    
                    if (property.IsSameAs(_("false"), false)) {
                        m_bDM = false;
                    }

                    // Get the path to the DM file
#if wxMAJOR_VERSION > 3                    
                    m_dm.m_configPath = subchild->GetAttribute(wxT("path"), wxT(""));
#else 
                    m_dm.m_configPath = subchild->GetPropVal(wxT("path"), wxT(""));
#endif                    
                    m_dm.m_configPath.Trim();
                    m_dm.m_configPath.Trim(false);
                    
                }                 
                else if (subchild->GetName() == wxT("variables")) {
                    // Should the internal DM be disabled
#if wxMAJOR_VERSION > 3                    
                    wxString property = subchild->GetAttribute(wxT("enable"), wxT("true"));
#else 
                    wxString property = subchild->GetPropVal(wxT("enable"), wxT("true"));
#endif                    
                    if (property.IsSameAs(_("false"), false)) {
                        m_bVariables = false;
                    }

                    // Get the path to the DM file
#if wxMAJOR_VERSION > 3                    
                    m_VSCP_Variables.m_configPath = subchild->GetAttribute(wxT("path"), wxT(""));
#else 
                    m_VSCP_Variables.m_configPath = subchild->GetPropVal(wxT("path"), wxT(""));
#endif                    
                    m_VSCP_Variables.m_configPath.Trim();
                    m_VSCP_Variables.m_configPath.Trim(false);

                } else if (subchild->GetName() == wxT("vscp")) {
#if wxMAJOR_VERSION > 3                    
                    wxString property = subchild->GetAttribute(wxT("enable"), wxT("true"));
#else 
                    wxString property = subchild->GetPropVal(wxT("enable"), wxT("true"));
#endif                    
                    if (property.IsSameAs(_("false"), false)) {
                        m_bVSCPDaemon = false;
                    }
                } 
				else if (subchild->GetName() == wxT("guid")) {
                    wxString str = subchild->GetNodeContent();
                    //getGuidFromStringToArray(m_GUID, str);
                    m_guid.getFromString(str);
                } 
				else if (subchild->GetName() == wxT("clientbuffersize")) {
                    wxString str = subchild->GetNodeContent();
                    m_maxItemsInClientReceiveQueue = vscp_readStringValue(str);
                } 
				else if (subchild->GetName() == wxT("webrootpath")) {
                    CControlObject::m_pathRoot = subchild->GetNodeContent();
                    CControlObject::m_pathRoot.Trim();
                    CControlObject::m_pathRoot.Trim(false); 
				} 
				else if (subchild->GetName() == wxT("authdoamin")) {
                    CControlObject::m_authDomain = subchild->GetNodeContent();
                    CControlObject::m_authDomain.Trim();
                    CControlObject::m_authDomain.Trim(false);
                } 
				else if (subchild->GetName() == wxT("pathcert")) {
                    m_pathCert = subchild->GetNodeContent();
                    m_pathCert.Trim();
                    m_pathCert.Trim(false);
                } 
				else if (subchild->GetName() == wxT("pathkey")) {
                    m_pathKey = subchild->GetNodeContent();
                    m_pathKey.Trim();
                    m_pathKey.Trim(false);
                } 
				else if (subchild->GetName() == wxT("websockets")) {
#if wxMAJOR_VERSION > 3                    
                    wxString property = subchild->GetAttribute(wxT("enable"), wxT("true"));
#else 
                    wxString property = subchild->GetPropVal(wxT("enable"), wxT("true"));
#endif                    
                    if (property.IsSameAs(_("false"), false)) {
                        m_bWebSockets = false;
                    }

#if wxMAJOR_VERSION > 3                    
                    property = subchild->GetAttribute(wxT("port"), wxT("7681"));
#else 
                    property = subchild->GetPropVal(wxT("port"), wxT("7681"));
#endif                    
                    if (property.IsNumber()) {
                        //m_portWebsockets = vscp_readStringValue(property);
                    }

#if wxMAJOR_VERSION > 3                    
                    property = subchild->GetAttribute(wxT("auth"), wxT("true"));
#else 
                    property = subchild->GetPropVal(wxT("auth"), wxT("true"));
#endif                    
                    if (property.IsSameAs(_("false"), false)) {
                        m_bAuthWebsockets = false;
                    }

                } 
				else if (subchild->GetName() == wxT("webserver")) {
#if wxMAJOR_VERSION > 3                    
                    wxString property = subchild->GetAttribute(wxT("enable"), wxT("true"));
#else 
                    wxString property = subchild->GetPropVal(wxT("enable"), wxT("true"));
#endif                    
                    if (property.IsSameAs(_("false"), false)) {
                        m_bWebServer = false;
                    }

#if wxMAJOR_VERSION > 3                    
                    property = subchild->GetAttribute(wxT("port"), wxT("8080"));
#else 
                    property = subchild->GetPropVal(wxT("port"), wxT("8080"));
#endif                    
                    if (property.IsNumber()) {
                        m_portWebServer = vscp_readStringValue(property);
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

                vscp_clearVSCPFilter(&VSCPFilter); // Allow all frames

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
#if wxMAJOR_VERSION > 3                            
                            wxString str_vscp_priority = subchild->GetAttribute(wxT("priority"), wxT("0"));
#else 
                            wxString str_vscp_priority = subchild->GetPropVal(wxT("priority"), wxT("0"));
#endif                            
                            val = 0;
                            str_vscp_priority.ToULong(&val);
                            VSCPFilter.filter_priority = val;
#if wxMAJOR_VERSION > 3                            
                            wxString str_vscp_class = subchild->GetAttribute(wxT("class"), wxT("0"));
#else 
                            wxString str_vscp_class = subchild->GetPropVal(wxT("class"), wxT("0"));
#endif                            
                            val = 0;
                            str_vscp_class.ToULong(&val);
                            VSCPFilter.filter_class = val;
#if wxMAJOR_VERSION > 3                            
                            wxString str_vscp_type = subchild->GetAttribute(wxT("type"), wxT("0"));
#else 
                            wxString str_vscp_type = subchild->GetPropVal(wxT("type"), wxT("0"));
#endif                            
                            val = 0;
                            str_vscp_type.ToULong(&val);
                            VSCPFilter.filter_type = val;
#if wxMAJOR_VERSION > 3                            
                            wxString str_vscp_guid = subchild->GetAttribute(wxT("guid"),
#else 
                            wxString str_vscp_guid = subchild->GetPropVal(wxT("guid"),
#endif                                    
                                    wxT("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00"));
                            vscp_getGuidFromStringToArray(VSCPFilter.filter_GUID, str_vscp_guid);
                        } else if (subsubchild->GetName() == wxT("mask")) {
                            bMaskPresent = true;
#if wxMAJOR_VERSION > 3                            
                            wxString str_vscp_priority = subchild->GetAttribute(wxT("priority"), wxT("0"));
#else 
                            wxString str_vscp_priority = subchild->GetPropVal(wxT("priority"), wxT("0"));
#endif                            
                            val = 0;
                            str_vscp_priority.ToULong(&val);
                            VSCPFilter.mask_priority = val;
#if wxMAJOR_VERSION > 3                            
                            wxString str_vscp_class = subchild->GetAttribute(wxT("class"), wxT("0"));
#else 
                            wxString str_vscp_class = subchild->GetPropVal(wxT("class"), wxT("0"));
#endif                            
                            val = 0;
                            str_vscp_class.ToULong(&val);
                            VSCPFilter.mask_class = val;
#if wxMAJOR_VERSION > 3                            
                            wxString str_vscp_type = subchild->GetAttribute(wxT("type"), wxT("0"));
#else 
                            wxString str_vscp_type = subchild->GetPropVal(wxT("type"), wxT("0"));
#endif                            
                            val = 0;
                            str_vscp_type.ToULong(&val);
                            VSCPFilter.mask_type = val;
#if wxMAJOR_VERSION > 3                            
                            wxString str_vscp_guid = subchild->GetAttribute(wxT("guid"),
#else 
                            wxString str_vscp_guid = subchild->GetPropVal(wxT("guid"),
#endif                                    
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
                if (bUser) {

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
                    
#if wxMAJOR_VERSION > 3                    
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
                            flags = vscp_readStringValue(str);
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
                    vscp_getGuidFromStringToArray(GUID, strGUID);
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
                    
#if wxMAJOR_VERSION > 3                    
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
                    vscp_getGuidFromStringToArray(GUID, strGUID);
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
                    } 
					else {
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
    //unsigned long val;
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
#if wxMAJOR_VERSION > 3             
            wxString strEnable = child->GetAttribute(wxT("enable"), wxT("false"));
#else 
            wxString strEnable = child->GetPropVal(wxT("enable"), wxT("false"));
#endif 
#if wxMAJOR_VERSION > 3             
            wxString strExt = child->GetAttribute(wxT("extension"), wxT(""));
#else 
            wxString strExt = child->GetPropVal(wxT("extension"), wxT(""));            
#endif
#if wxMAJOR_VERSION > 3             
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


///////////////////////////////////////////////////////////////////////////////
//                               WEBSOCKET
///////////////////////////////////////////////////////////////////////////////





/*
int
CControlObject::callback_lws_vscp( struct libwebsocket_context *context,
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
        vscp_clearVSCPFilter(&pss->pClientItem->m_filterVSCP);

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
#ifdef WIN32
#else
                syslog(LOG_ERR, "ERROR writing to socket");
#endif
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

                if (vscp_doLevel2Filter(pEvent, &pss->pClientItem->m_filterVSCP)) {

                    if (vscp_writeVscpEventToString(pEvent, str)) {

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
#ifdef WIN32
#else
                            syslog(LOG_ERR, "ERROR writing to socket");
#endif
                        }
                    }
                }

                // Remove the event
                vscp_deleteVSCPevent(pEvent);

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
        if (vscp_getVscpEventFromString(&vscp_event, str)) {

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
            if (vscp_doLevel2Filter(pEvent, &pDestClientItem->m_filterVSCP)) {

                // If the client queue is full for this client then the
                // client will not receive the message
                if (pDestClientItem->m_clientInputQueue.GetCount() <=
                        m_maxItemsInClientReceiveQueue) {

                    // Create copy of event
                    vscpEvent *pnewEvent = new vscpEvent;
                    if (NULL != pnewEvent) {

                        vscp_copyVSCPEvent(pnewEvent, pEvent);

                        // Add the new event to the inputqueue
                        pDestClientItem->m_mutexClientInputQueue.Lock();
                        pDestClientItem->m_clientInputQueue.Append(pEvent);
                        pDestClientItem->m_semClientInputQueue.Post();
                        pDestClientItem->m_mutexClientInputQueue.Unlock();
                    }

                    bSent = true;

                } else {
                    // Overun - No room for event
                    vscp_deleteVSCPevent(pEvent);
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

                vscp_copyVSCPEvent(pnewEvent, pEvent);

                m_mutexClientOutputQueue.Lock();
                m_clientOutputQueue.Append(pnewEvent);
                m_semClientOutputQueue.Post();
                m_mutexClientOutputQueue.Unlock();
            }

        } else {
            vscp_deleteVSCPevent(pEvent);
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
            if (!vscp_readFilterFromString(&pss->pClientItem->m_filterVSCP,
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
            if (!vscp_readMaskFromString(&pss->pClientItem->m_filterVSCP,
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
            vscp_deleteVSCPevent(pEvent);
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
            type = vscp_readStringValue(tkz.GetNextToken());
        }

        // Get variable Persistent
        if (tkz.HasMoreTokens()) {
            int val = vscp_readStringValue(tkz.GetNextToken());
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
*/







///////////////////////////////////////////////////////////////////////////////
//                              WEB SERVER
///////////////////////////////////////////////////////////////////////////////







///////////////////////////////////////////////////////////////////////////////
// websock_command
//

int
CControlObject::websock_command( struct mg_connection *conn, 
										struct websock_session *pSession,
										wxString& strCmd )
{
	wxString strTok;
	int rv = MG_TRUE;

	// Check pointer
    if (NULL == conn) return MG_FALSE;
	if (NULL == pSession) return MG_FALSE;
	
	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

	//mg_websocket_write( conn, WEBSOCKET_OPCODE_TEXT, conn->content, conn->content_len );
	//if ( conn->content_len == 4 && memcmp( conn->content, "exit", 4 ) ) {
	//	return MG_FALSE;
	//}

	//mg_websocket_write( conn, WEBSOCKET_OPCODE_PING, NULL, 0 );

    wxStringTokenizer tkz( strCmd, _(";") );

    // Get command
    if (tkz.HasMoreTokens()) {
        strTok = tkz.GetNextToken();
        strTok.MakeUpper();
    } 
	else {
        mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_SYNTAX_ERROR, 
									WEBSOCK_STR_ERROR_SYNTAX_ERROR );
        return MG_TRUE;
    }

    if (0 == strTok.Find(_("NOOP"))) {
        mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, "+;NOOP" );
	}
	// AUTH;user;hash
	else if (0 == strTok.Find(_("AUTH"))) {
		wxString strUser = tkz.GetNextToken();
		wxString strKey = tkz.GetNextToken();
		if ( MG_TRUE == pObject->websock_authentication( conn, pSession, strUser, strKey ) ) {
			mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, "+;AUTH1" );
			pSession->bAuthenticated = true;	// Authenticated
		}
		else {
			mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_NOT_AUTHORIZED, 
									WEBSOCK_STR_ERROR_NOT_AUTHORIZED );
			pSession->bAuthenticated = false;	// Authenticated
		}
    } 
	else if (0 == strTok.Find(_("OPEN"))) {

		// Must be authorized to do this
		if ( !pSession->bAuthenticated ) {
			mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_NOT_AUTHORIZED,
									WEBSOCK_STR_ERROR_NOT_AUTHORIZED );
			return MG_TRUE;	// We still leave channel open
		}

        pSession->m_pClientItem->m_bOpen = true;
        mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, "+;OPEN" );
    } 
	else if (0 == strTok.Find(_("CLOSE"))) {

		// Must be authorized to do this
		if ( !pSession->bAuthenticated ) {
			mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_NOT_AUTHORIZED,
									WEBSOCK_STR_ERROR_NOT_AUTHORIZED );
			return MG_TRUE;	// We still leave channel open
		}

        pSession->m_pClientItem->m_bOpen = false;
        mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, "+;CLOSE" );
		rv = MG_FALSE;
    } 
	else if (0 == strTok.Find(_("SETFILTER"))) {

        unsigned char ifGUID[ 16 ];
        memset(ifGUID, 0, 16);

		// Must be authorized to do this
		if ( !pSession->bAuthenticated ) {
			mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_NOT_AUTHORIZED,
									WEBSOCK_STR_ERROR_NOT_AUTHORIZED );
			return MG_TRUE;	// We still leave channel open
		}


        // Get filter
        if (tkz.HasMoreTokens()) {
            strTok = tkz.GetNextToken();
			pSession->m_pClientItem->m_mutexClientInputQueue.Lock();
            if (!vscp_readFilterFromString( &pSession->m_pClientItem->m_filterVSCP, strTok ) ) {
				pSession->m_pClientItem->m_mutexClientInputQueue.Unlock();
                mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_SYNTAX_ERROR, 
									WEBSOCK_STR_ERROR_SYNTAX_ERROR );
                return MG_TRUE;
            }
			pSession->m_pClientItem->m_mutexClientInputQueue.Unlock();
        } 
		else {
            mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_SYNTAX_ERROR, 
									WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            return MG_TRUE;
        }

        // Get mask
        if (tkz.HasMoreTokens()) {
            strTok = tkz.GetNextToken();
			pSession->m_pClientItem->m_mutexClientInputQueue.Lock();
            if (!vscp_readMaskFromString( &pSession->m_pClientItem->m_filterVSCP, strTok)) {
				pSession->m_pClientItem->m_mutexClientInputQueue.Unlock();
                mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_SYNTAX_ERROR, 
									WEBSOCK_STR_ERROR_SYNTAX_ERROR );
				pSession->m_pClientItem->m_mutexClientInputQueue.Unlock();
                return MG_TRUE;
            }
        } 
		else {
            mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_SYNTAX_ERROR, 
									WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            return MG_TRUE;
        }

        // Positive response
        mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, "+;SETFILTER" );

    }// Clear the event queue
    else if (0 == strTok.Find(_("CLRQUE"))) {

        CLIENTEVENTLIST::iterator iterVSCP;

		// Must be authorized to do this
		if ( !pSession->bAuthenticated ) {
			mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_NOT_AUTHORIZED,
									WEBSOCK_STR_ERROR_NOT_AUTHORIZED );
			return MG_TRUE;	// We still leave channel open
		}

        pSession->m_pClientItem->m_mutexClientInputQueue.Lock();
        for (iterVSCP = pSession->m_pClientItem->m_clientInputQueue.begin();
                iterVSCP != pSession->m_pClientItem->m_clientInputQueue.end(); ++iterVSCP) {
            vscpEvent *pEvent = *iterVSCP;
            vscp_deleteVSCPevent(pEvent);
        }

        pSession->m_pClientItem->m_clientInputQueue.Clear();
        pSession->m_pClientItem->m_mutexClientInputQueue.Unlock();

        mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, "+;CLRQUE" );
    } 
	else if (0 == strTok.Find(_("WRITEVAR"))) {

		// Must be authorized to do this
		if ( !pSession->bAuthenticated ) {
			mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_NOT_AUTHORIZED,
									WEBSOCK_STR_ERROR_NOT_AUTHORIZED );
			return MG_TRUE;	// We still leave channel open
		}

        // Get variablename
        if (tkz.HasMoreTokens()) {

            CVSCPVariable *pvar;
            strTok = tkz.GetNextToken();
            if (NULL == (pvar = pObject->m_VSCP_Variables.find(strTok))) {
                mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_VARIABLE_UNKNOWN, 
									WEBSOCK_STR_ERROR_VARIABLE_UNKNOWN );
                return MG_TRUE;
            }

            // Get variable value
            if (tkz.HasMoreTokens()) {
                strTok = tkz.GetNextToken();
                if (!pvar->setValueFromString(pvar->getType(), strTok)) {
                    mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_SYNTAX_ERROR, 
									WEBSOCK_STR_ERROR_SYNTAX_ERROR );
                    return MG_TRUE;
                }
            } 
			else {
                mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_SYNTAX_ERROR, 
									WEBSOCK_STR_ERROR_SYNTAX_ERROR );
                return MG_TRUE;
            }
        } 
		else {
            mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_SYNTAX_ERROR, 
									WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            return MG_TRUE;
        }

        // Positive reply
        mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, "+;WRITEVAR" );

    } 
	else if (0 == strTok.Find(_("ADDVAR"))) {

        wxString name;
        wxString value;
        uint8_t type = VSCP_DAEMON_VARIABLE_CODE_STRING;
        bool bPersistent = false;

		// Must be authorized to do this
		if ( !pSession->bAuthenticated ) {
			mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_NOT_AUTHORIZED,
									WEBSOCK_STR_ERROR_NOT_AUTHORIZED );
			return MG_TRUE;	// We still leave channel open
		}

        // Get variable name
        if (tkz.HasMoreTokens()) {
            name = tkz.GetNextToken();
        } 
		else {
            mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_SYNTAX_ERROR, 
									WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            return MG_TRUE;
        }

        // Get variable value
        if (tkz.HasMoreTokens()) {
            value = tkz.GetNextToken();
        } 
		else {
            mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_SYNTAX_ERROR, 
									WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            return MG_TRUE;
        }

        // Get variable type
        if (tkz.HasMoreTokens()) {
            type = vscp_readStringValue(tkz.GetNextToken());
        }

        // Get variable Persistent
        if (tkz.HasMoreTokens()) {
            int val = vscp_readStringValue(tkz.GetNextToken());
        }

        // Add the variable
        if (!pObject->m_VSCP_Variables.add(name, value, type, bPersistent)) {
            mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_SYNTAX_ERROR, 
									WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            return MG_TRUE;
        } 
		else {
            mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_SYNTAX_ERROR, 
									WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            return MG_TRUE;
        }

        mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, "+;ADDVAR" );
        
    } 
	else if (0 == strTok.Find(_("READVAR"))) {

        CVSCPVariable *pvar;
        uint8_t type;
        wxString strvalue;

		// Must be authorized to do this
		if ( !pSession->bAuthenticated ) {
			mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_NOT_AUTHORIZED,
									WEBSOCK_STR_ERROR_NOT_AUTHORIZED );
			return MG_TRUE;	// We still leave channel open
		}

        strTok = tkz.GetNextToken();
        if (NULL == (pvar = pObject->m_VSCP_Variables.find(strTok))) {
			mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_VARIABLE_UNKNOWN, 
									WEBSOCK_STR_ERROR_VARIABLE_UNKNOWN );
            return MG_TRUE;
        }

        pvar->writeVariableToString(strvalue);
        type = pvar->getType();

        wxString resultstr = _("+;READVAR;");
        resultstr += wxString::Format(_("%d"), type);
        resultstr += _(";");
        resultstr += strvalue;
		mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, resultstr.mbc_str() );

    } 
	else if (0 == strTok.Find(_("SAVEVAR"))) {

		// Must be authorized to do this
		if ( !pSession->bAuthenticated ) {
			mg_websocket_printf( conn, 
									WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s",
									WEBSOCK_ERROR_NOT_AUTHORIZED,
									WEBSOCK_STR_ERROR_NOT_AUTHORIZED );
			return MG_TRUE;	// We still leave channel open
		}

        if (!pObject->m_VSCP_Variables.save()) {
            mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_SYNTAX_ERROR, 
									WEBSOCK_STR_ERROR_SYNTAX_ERROR );
            return MG_TRUE;
        }

        mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, "+;SAVEVAR" );
    } 
	else {
        mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, 
									"-;%d;%s", 
									WEBSOCK_ERROR_UNKNOWN_COMMAND, 
									WEBSOCK_STR_ERROR_UNKNOWN_COMMAND );
    }

	return rv;

}




///////////////////////////////////////////////////////////////////////////////
// websock_sendevent
//

int
CControlObject::websock_sendevent( struct mg_connection *conn, 
										struct websock_session *pSession,
										vscpEvent *pEvent )
{
    bool bSent = false;
    bool rv = true;

	// Check pointer
    if (NULL == conn) return MG_FALSE;
	if (NULL == pSession) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

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

        pObject->m_wxClientMutex.Lock();

        // Find client
        CClientItem *pDestClientItem = NULL;
        VSCPCLIENTLIST::iterator iter;
        for (iter = pObject->m_clientList.m_clientItemList.begin();
                iter != pObject->m_clientList.m_clientItemList.end();
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
            if (vscp_doLevel2Filter(pEvent, &pDestClientItem->m_filterVSCP)) {

                // If the client queue is full for this client then the
                // client will not receive the message
                if (pDestClientItem->m_clientInputQueue.GetCount() <=
                        pObject->m_maxItemsInClientReceiveQueue) {

                    // Create copy of event
                    vscpEvent *pnewEvent = new vscpEvent;
                    if (NULL != pnewEvent) {

                        vscp_copyVSCPEvent(pnewEvent, pEvent);

                        // Add the new event to the inputqueue
                        pDestClientItem->m_mutexClientInputQueue.Lock();
                        pDestClientItem->m_clientInputQueue.Append(pnewEvent);
                        pDestClientItem->m_semClientInputQueue.Post();
                        pDestClientItem->m_mutexClientInputQueue.Unlock();

						bSent = true;
                    }
					else {
						bSent = false;
					}

                } else {
                    // Overun - No room for event
                    //vscp_deleteVSCPevent(pEvent);
                    bSent = true;
                    rv = false;
                }

            } // filter

        } // Client found

        pObject->m_wxClientMutex.Unlock();

    }

    if (!bSent) {

        // There must be room in the send queue
        if (pObject->m_maxItemsInClientReceiveQueue >
                pObject->m_clientOutputQueue.GetCount()) {

            // Create copy of event
            vscpEvent *pnewEvent = new vscpEvent;
            if (NULL != pnewEvent) {

                vscp_copyVSCPEvent(pnewEvent, pEvent);

                pObject->m_mutexClientOutputQueue.Lock();
                pObject->m_clientOutputQueue.Append(pnewEvent);
                pObject->m_semClientOutputQueue.Post();
                pObject->m_mutexClientOutputQueue.Unlock();
            }

        } else {
            //vscp_deleteVSCPevent(pEvent);
            rv = false;
        }
    }

    return rv;
}


///////////////////////////////////////////////////////////////////////////////
// websrv_websocket_message
//

int 
CControlObject::websrv_websocket_message( struct mg_connection *conn )
{
	wxString str;
    char buf[ 2048 ];
	struct websock_session *pSession;
    const char *p = buf;

	memset( buf, 0, sizeof( buf ) );

	// Check pointer
    if (NULL == conn) return MG_FALSE;
	if ( conn->content_len ) memcpy( buf, conn->content, min(conn->content_len, sizeof( buf ) ) );

	pSession = websock_get_session( conn );
	if (NULL == pSession) return MG_FALSE;

	// Keep connection alive
	if ( ( conn->wsbits & 0x0f ) == WEBSOCKET_OPCODE_PING ) {
		mg_websocket_write(conn, WEBSOCKET_OPCODE_PONG, conn->content, conn->content_len);
		return MG_TRUE;
	}
	else if ( ( conn->wsbits & 0x0f ) == WEBSOCKET_OPCODE_PONG  ) {
		mg_websocket_write(conn, WEBSOCKET_OPCODE_PING, conn->content, conn->content_len);
		return MG_TRUE;
	}

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

    memset(buf, 0, sizeof( buf));
	memcpy(buf, (char *)conn->content, min( conn->content_len, sizeof(buf) ) );

    switch (*p) {

		// Command - | 'C' | command type (byte) | data |
		case 'C':
			p++;
			p++; // Point beyond initial info "C;"
			str = wxString::FromAscii( p );
			pObject->websock_command( conn, pSession, str );
			break;

        // Event | 'E' ; head(byte) , vscp_class(unsigned short) , vscp_type(unsigned
        //					short) , GUID(16*byte), data(0-487 bytes) |
		case 'E':
		{
			// Must be authorized to do this
			if ( !pSession->bAuthenticated ) {
				mg_websocket_printf( conn, 
											WEBSOCKET_OPCODE_TEXT, 
											"-;%d;%s",
											WEBSOCK_ERROR_NOT_AUTHORIZED,
											WEBSOCK_STR_ERROR_NOT_AUTHORIZED );
				return MG_TRUE;
			}

			p++;
			p++; // Point beyond initial info "E;"
			vscpEvent vscp_event;
			str = wxString::FromAscii( p );
			if (vscp_getVscpEventFromString( &vscp_event, str ) ) {

				vscp_event.obid = pSession->m_pClientItem->m_clientID;
				if ( pObject->websock_sendevent( conn, pSession, &vscp_event ) ) {
					mg_websocket_printf( conn, WEBSOCKET_OPCODE_TEXT, "+;EVENT" );
				} 
				else {
					mg_websocket_printf( conn, 
											WEBSOCKET_OPCODE_TEXT, 
											"-;%d;%s",
											WEBSOCK_ERROR_TX_BUFFER_FULL,
											WEBSOCK_STR_ERROR_TX_BUFFER_FULL );
				}
			}
		}
		break;

        // Unknow command
		default:
			break;

    }

	return MG_TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// websock_authentication
//
// client sends
//		"AUTH;user;hash"
//	where the hash is based on
//		"user:standard vscp password hash:server generated hash(sid)"
//	"user;hash" is reeived in strKey

int
CControlObject::websock_authentication( struct mg_connection *conn, 
											struct websock_session *pSession, 
											wxString& strUser, 
											wxString& strKey )
{
	int rv = MG_FALSE;
	char response[32 + 1];
	char expected_response[32 + 1];
	bool bValidHost = false;

	memset( response, 0, sizeof( response ) );
	memset( expected_response, 0, sizeof( expected_response ) );

	// Check pointer
    if (NULL == conn) return MG_FALSE;
	if (NULL == pSession) return MG_FALSE;
	
	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

	if ( m_bAuthWebsockets ) {

		// Check if user is valid			
		CUserItem *pUser = pObject->m_userList.getUser( strUser );
		if ( NULL == pUser ) return MG_FALSE;

		// Check if remote ip is valid
		pObject->m_mutexUserList.Lock();
		bValidHost = pObject->m_userList.checkRemote( pUser, 
														wxString::FromAscii( conn->remote_ip ) );
		pObject->m_mutexUserList.Unlock();
		if (!bValidHost) return MG_FALSE;

		strncpy( response, strKey.mb_str(), min( sizeof(response), strKey.Length() ) );

		mg_md5( expected_response,
					strUser.mb_str(), ":",
					pUser->m_md5Password.mb_str(), ":",
					pSession->m_sid, NULL );

		rv = ( vscp_strcasecmp( response, expected_response ) == 0 ) ? MG_TRUE : MG_FALSE;

	}

	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// websock_new_session
//

websock_session *
CControlObject::websock_new_session( struct mg_connection *conn, const char * pKey, const char * pVer )
{
	char buf[512];
	struct websock_session *ret;

	// Check pointer
    if (NULL == conn) return NULL;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return NULL;

	// create fresh session 
    ret = (struct websock_session *)calloc(1, sizeof(struct websock_session));
    if  (NULL == ret ) {
#ifndef WIN32
        syslog(LOG_ERR, "calloc error: %s\n", strerror(errno));
#endif
        return NULL;
    }

	memset( ret->m_sid, 0, sizeof( ret->m_sid ) );
	memset( ret->m_key, 0, sizeof( ret->m_key ) );

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

	Cmd5 md5( (unsigned char *)buf );
	strcpy( ret->m_sid, md5.getDigest() );


		
	// Init
	strcpy( ret->m_key, pKey );				// Save key
	ret->bAuthenticated = false;			// Not authenticated in yet
	ret->m_version = atoi( pVer );			// Store protocol version
	ret->m_pUserItem = NULL;				// Is set when authenticated
    ret->m_pClientItem = new CClientItem();	// Create client        
    vscp_clearVSCPFilter(&ret->m_pClientItem->m_filterVSCP); // // Clear filter
	ret->bTrigger = false;
	ret->triggerTimeout = 0;

	// This is an active client
    ret->m_pClientItem->m_bOpen = false;
    ret->m_pClientItem->m_type = CLIENT_ITEM_INTERFACE_TYPE_CLIENT_WEBSOCKET;
    ret->m_pClientItem->m_strDeviceName = _("Internal daemon websocket client. Started at ");
    wxDateTime now = wxDateTime::Now();
    ret->m_pClientItem->m_strDeviceName += now.FormatISODate();
    ret->m_pClientItem->m_strDeviceName += _(" ");
    ret->m_pClientItem->m_strDeviceName += now.FormatISOTime();

	// Add the client to the Client List
    pObject->m_wxClientMutex.Lock();
    pObject->addClient(ret->m_pClientItem);
    pObject->m_wxClientMutex.Unlock();

    // Add to linked list
    ret->m_referenceCount++;
    ret->lastActiveTime = time(NULL);
    ret->m_next = websock_sessions;
	websock_sessions = ret;

    return ret;
}


///////////////////////////////////////////////////////////////////////////////
// websock_get_session
//

struct websock_session *
CControlObject::websock_get_session( struct mg_connection *conn )
{
    struct websock_session *ret = NULL;

	// Check pointer
	if (NULL == conn) return NULL;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return NULL;

	// Get the session key
	const char *pKey = mg_get_header( conn, "sec-websocket-key" ); 
	if ( NULL == pKey) return NULL;

	// Get protocol version
	const char *pVer = mg_get_header( conn, "sec-websocket-version" ); 
	if ( NULL == pVer) return NULL;
        
    // find existing session 
    ret = websock_sessions;
    while (NULL != ret) {
		if ( 0 == strcmp( pKey, ret->m_key ) ) {
			break;
		}
        ret = ret->m_next;
	}
        
	if ( NULL != ret ) {
		ret->m_referenceCount++;
		ret->lastActiveTime = time(NULL);
        return ret;
    }
     
	// Return new session
    return pObject->websock_new_session( conn, pKey, pVer );
}


///////////////////////////////////////////////////////////////////////////////
// websock_expire_sessions
//

void
CControlObject::websock_expire_sessions( struct mg_connection *conn  )
{
    struct websock_session *pos;
    struct websock_session *prev;
    struct websock_session *next;
    time_t now;

	// Check pointer
	if (NULL == conn) return;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return;

    now = time( NULL );
    prev = NULL;
    pos = websock_sessions;
    
    while (NULL != pos) {
        
        next = pos->m_next;
        
        if ( ( now - pos->lastActiveTime ) > ( 60 * 60 ) ) {
        
            // expire sessions after 1h 
            if ( NULL == prev ) {
                websock_sessions = pos->m_next;
            }
            else {
                prev->m_next = next;
            }
            
			pObject->m_wxClientMutex.Lock();
			pObject->removeClient( pos->m_pClientItem );
			pObject->m_wxClientMutex.Unlock();

			// Free session data
            free(pos);
            
        } 
        else {
            prev = pos;
        }
        
        pos = next;
    }
}


///////////////////////////////////////////////////////////////////////////////
// websock_post_incomingEvent
//

void
CControlObject::websock_post_incomingEvents( void )
{
	struct mg_connection *conn;

	// Iterate over all connections, and push current time message to websocket ones.
	for (conn = mg_next( m_pwebserver, NULL); conn != NULL; conn = mg_next(m_pwebserver, conn)) {
		
		if ( conn->is_websocket ) {

			websock_session *pSession = websock_get_session( conn );
			if ( NULL == pSession) continue;

			if ( pSession->m_pClientItem->m_bOpen &&
					pSession->m_pClientItem->m_clientInputQueue.GetCount()) {

				CLIENTEVENTLIST::compatibility_iterator nodeClient;
				vscpEvent *pEvent;

				pSession->m_pClientItem->m_mutexClientInputQueue.Lock();
				nodeClient = pSession->m_pClientItem->m_clientInputQueue.GetFirst();
				pEvent = nodeClient->GetData();
				pSession->m_pClientItem->m_clientInputQueue.DeleteNode(nodeClient);
				pSession->m_pClientItem->m_mutexClientInputQueue.Unlock();

				if (NULL != pEvent) {

					if (vscp_doLevel2Filter(pEvent, &pSession->m_pClientItem->m_filterVSCP)) {

						wxString str;
						if (vscp_writeVscpEventToString(pEvent, str)) {

							// Write it out
							char buf[ 512 ];
							memset((char *) buf, 0, sizeof( buf));
							strcpy((char *) buf, (const char*) "E;");
							strcat((char *) buf, (const char*) str.mb_str(wxConvUTF8));
							mg_websocket_write(conn, 1, buf, strlen(buf) );
							
						}
					}

					// Remove the event
					vscp_deleteVSCPevent(pEvent);

				} // Valid pEvent pointer
			} // events available
		} // websocket
	} // for
}
	



///////////////////////////////////////////////////////////////////////////////
//                     WEBSERVER SESSION HANDLINO
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// websrv_get_session
//

struct websrv_Session *
CControlObject::websrv_get_session( struct mg_connection *conn )
{
	char buf[512];
    struct websrv_Session *ret = NULL;
    const char *cookie = NULL;

	// Get the session cookie
	const char *pheader = mg_get_header( conn, "cookie" ); 
	if ( NULL == pheader) return NULL;

	if ( MG_FALSE == mg_parse_header( pheader, "session", buf, sizeof( buf ) ) ) return NULL;
	cookie = buf;
	
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
			ret->lastActiveTime = time( NULL );
            return ret;
        }
    }
        
    return ret;
}


///////////////////////////////////////////////////////////////////////////////
// websrv_add_session_cookie
//

websrv_Session *
CControlObject::websrv_add_session_cookie( struct mg_connection *conn, const char *pUser )
{
	char buf[512];
	struct websrv_Session *ret;

	// Check pointer
	if (NULL == conn) return NULL;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return NULL;

	// create fresh session 
    ret = (struct websrv_Session *)calloc(1, sizeof(struct websrv_Session));
    if  (NULL == ret ) {
#ifndef WIN32
        syslog(LOG_ERR, "calloc error: %s\n", strerror(errno));
#endif
        return NULL;
    }

	// Generate a random session ID
    time_t t;
    t = time( NULL );
    sprintf( buf,
				"__VSCP__DAEMON_%X%X%X%X_be_hungry_stay_foolish_%X%X",
				(unsigned int)rand(),
				(unsigned int)rand(),
				(unsigned int)rand(),
				(unsigned int)t,
				(unsigned int)rand(), 
				1337 );

	Cmd5 md5( (unsigned char *)buf );
	strcpy( ret->m_sid, md5.getDigest() );
	
	sprintf( buf, "session=%s; max-age=3600; http-only", ret->m_sid );
	mg_send_header( conn, "Set-Cookie", buf );
	
	sprintf( buf, "user=%s", pUser );
	mg_send_header( conn, "Set-Cookie", buf );

	strcpy( buf, "original_url=/; max-age=0" );
	mg_send_header( conn, "Set-Cookie", buf );

	ret->m_pUserItem = pObject->m_userList.getUser( wxString::FromAscii( pUser ) );
	
    // Add to linked list
    ret->m_referenceCount++;
    ret->lastActiveTime = time(NULL);
    ret->m_next = websrv_sessions;
	websrv_sessions = ret;

    return ret;
}

///////////////////////////////////////////////////////////////////////////////
// websrv_GetCreateSession
//

struct websrv_Session * 
CControlObject::websrv_GetCreateSession( struct mg_connection *conn )
{
	const char *hdr;
	char user[256];
	struct websrv_Session *rv = NULL;

	// Check pointer
	if (NULL == conn) return NULL;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return NULL;

	if ( NULL == ( rv =  pObject->websrv_get_session( conn ) ) ) {

		if ( NULL == ( hdr = mg_get_header( conn, "Authorization") ) ||
						( vscp_strncasecmp( hdr, "Digest ", 7 ) != 0 ) ) {
			return NULL;
		}
				
		if (!mg_parse_header(hdr, "username", user, sizeof(user))) {
			return NULL;
		}

		// Add session cookie
		rv = pObject->websrv_add_session_cookie( conn, user );
	}

	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// websrv_expire_sessions
//

void
CControlObject::websrv_expire_sessions( struct mg_connection *conn )
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
        
        if (now - pos->lastActiveTime > 60 * 60) {
        
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
// websrv_check_password
//
// http://en.wikipedia.org/wiki/Digest_access_authentication
//

int 
CControlObject::websrv_check_password( const char *method, 
											const char *ha1, 
											const char *uri,
											const char *nonce, 
											const char *nc, 
											const char *cnonce,
											const char *qop, 
											const char *response )
{
	char ha2[32 + 1], expected_response[32 + 1];

#if 0
  // Check for authentication timeout
  if ((unsigned long) time(NULL) - (unsigned long) to64(nonce) > 3600 * 2) {
    return 0;
  }
#endif

	mg_md5(ha2, method, ":", uri, NULL);
	mg_md5(expected_response, ha1, ":", nonce, ":", nc,
					":", cnonce, ":", qop, ":", ha2, NULL);

	return ( vscp_strcasecmp( response, expected_response ) == 0 ) ? MG_TRUE : MG_FALSE;

}




///////////////////////////////////////////////////////////////////////////////
// websrv_event_handler
//

int 
CControlObject::websrv_event_handler( struct mg_connection *conn, enum mg_event ev )
{
	//char buf[2048];
	static time_t cleanupTime = time(NULL);
	const char *hdr;
	struct websock_session *pWebSockSession;
	struct websrv_Session * pWebSrvSession;
	char user[256], nonce[256],
			uri[32768], cnonce[256], resp[256], qop[256], nc[256];
	CUserItem *pUser;
	bool bValidHost;

	// Check pointer
	if (NULL == conn) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

	switch (ev) {

		case MG_AUTH: 
			
			if (conn->is_websocket) {	
				return MG_TRUE;	// Always accept websocket connections
			}

			
			// Validate REST interface user.
			if ( 0 == strncmp(conn->uri, "/vscp/rest",10) ) {
				return MG_TRUE;	// Always accept websocket connections
			}

			if ( NULL == ( hdr = mg_get_header( conn, "Authorization") ) ||
					( vscp_strncasecmp( hdr, "Digest ", 7 ) != 0 ) ) {
				return MG_FALSE;
			}
			if (!mg_parse_header(hdr, "username", user, sizeof(user))) return MG_FALSE;
			if (!mg_parse_header(hdr, "cnonce", cnonce, sizeof(cnonce))) return MG_FALSE;
			if (!mg_parse_header(hdr, "response", resp, sizeof(resp))) return MG_FALSE;
			if (!mg_parse_header(hdr, "uri", uri, sizeof(uri))) return MG_FALSE;
			if (!mg_parse_header(hdr, "qop", qop, sizeof(qop))) return MG_FALSE;
			if (!mg_parse_header(hdr, "nc", nc, sizeof(nc))) return MG_FALSE;
			if (!mg_parse_header(hdr, "nonce", nonce, sizeof(nonce))) return MG_FALSE;

			// Check if user is vali			
			pUser = pObject->m_userList.getUser( wxString::FromAscii( user ) );
			if ( NULL == pUser ) return MG_FALSE;

			// Check if remote ip is valid
			pObject->m_mutexUserList.Lock();
			bValidHost = 
			        pObject->m_userList.checkRemote( pUser, 
														wxString::FromAscii( conn->remote_ip ) );
			pObject->m_mutexUserList.Unlock();
			if (!bValidHost) return MG_FALSE;

			if ( MG_TRUE != 
				pObject->websrv_check_password( conn->request_method, (const char *)pUser->m_md5Password.mb_str(), uri, nonce, nc, cnonce, qop, resp ) ) 
				return MG_FALSE;

			pObject->websrv_add_session_cookie( conn, user );

			return MG_TRUE;

		case MG_REQUEST:

			if (conn->is_websocket) {
				return pObject->websrv_websocket_message( conn );
			} 
			else {

				if ( 0 == strcmp(conn->uri, "/vscp") ) {
					if ( NULL == ( pWebSrvSession = pObject->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					mg_send_header(conn, "Content-Type", "text/html");
					mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
											"pre-check=0, no-store, no-cache, must-revalidate");
					return pObject->websrv_mainpage( conn );
				}
				else if ( 0 == strcmp(conn->uri, "/test") ) {
					if ( NULL == ( pWebSrvSession = pObject->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					mg_send_header(conn, "Content-Type", "text/html");
					mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
											"pre-check=0, no-store, no-cache, must-revalidate");
					mg_send_data( conn, html_form, strlen(html_form) );
					return MG_TRUE;
				}
				else if ( 0 == strcmp(conn->uri, "/vscp/interfaces") ) {
					if ( NULL == ( pWebSrvSession = pObject->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					mg_send_header(conn, "Content-Type", "text/html");
					mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
											"pre-check=0, no-store, no-cache, must-revalidate");
					return pObject->websrv_interfaces( conn );
				}
				else if ( 0 == strcmp(conn->uri, "/vscp/dm") ) {
					if ( NULL == ( pWebSrvSession = pObject->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					mg_send_header(conn, "Content-Type", "text/html");
					mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
											"pre-check=0, no-store, no-cache, must-revalidate");
					return pObject->websrv_dmlist( conn );
				}
				else if ( 0 == strcmp(conn->uri, "/vscp/dmedit") ) {
					if ( NULL == ( pWebSrvSession = pObject->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					mg_send_header(conn, "Content-Type", "text/html");
					mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
											"pre-check=0, no-store, no-cache, must-revalidate");
					return pObject->websrv_dmedit( conn );
				}
				else if ( 0 == strcmp(conn->uri, "/vscp/dmpost") ) {
					if ( NULL == ( pWebSrvSession = pObject->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					mg_send_header(conn, "Content-Type", "text/html");
					mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
											"pre-check=0, no-store, no-cache, must-revalidate");
					return pObject->websrv_dmpost( conn );
				}
				else if ( 0 == strcmp(conn->uri, "/vscp/dmdelete") ) {
					if ( NULL == ( pWebSrvSession = pObject->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					mg_send_header(conn, "Content-Type", "text/html");
					mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
											"pre-check=0, no-store, no-cache, must-revalidate");
					return pObject->websrv_dmdelete( conn );
				}
				else if ( 0 == strcmp(conn->uri, "/vscp/variables") ) {
					if ( NULL == ( pWebSrvSession = pObject->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					mg_send_header(conn, "Content-Type", "text/html");
					mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
											"pre-check=0, no-store, no-cache, must-revalidate");
					return pObject->websrv_variables_list( conn );
				}
				else if ( 0 == strcmp(conn->uri, "/vscp/varedit") ) {
					if ( NULL == ( pWebSrvSession = pObject->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					mg_send_header(conn, "Content-Type", "text/html");
					mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
											"pre-check=0, no-store, no-cache, must-revalidate");
					return pObject->websrv_variables_edit( conn );
				}
				else if ( 0 == strcmp(conn->uri, "/vscp/varpost") ) {
					if ( NULL == ( pWebSrvSession = pObject->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					mg_send_header(conn, "Content-Type", "text/html");
					mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
											"pre-check=0, no-store, no-cache, must-revalidate");
					return pObject->websrv_variables_post( conn );
				}
				else if ( 0 == strcmp(conn->uri, "/vscp/vardelete") ) {
					if ( NULL == ( pWebSrvSession = pObject->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					mg_send_header(conn, "Content-Type", "text/html");
					mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
											"pre-check=0, no-store, no-cache, must-revalidate");
					return pObject->websrv_variables_delete( conn );
				}
				else if ( 0 == strcmp(conn->uri, "/vscp/varnew") ) {
					if ( NULL == ( pWebSrvSession = pObject->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					mg_send_header(conn, "Content-Type", "text/html");
					mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
											"pre-check=0, no-store, no-cache, must-revalidate");
					return pObject->websrv_variables_new( conn );
				}
				else if ( 0 == strcmp(conn->uri, "/vscp/discovery") ) {
					if ( NULL == ( pWebSrvSession = pObject->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					mg_send_header(conn, "Content-Type", "text/html");
					mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
											"pre-check=0, no-store, no-cache, must-revalidate");
					return pObject->websrv_discovery( conn );
				}
				else if ( 0 == strcmp(conn->uri, "/vscp/session") ) {
					if ( NULL == ( pWebSrvSession = pObject->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					mg_send_header(conn, "Content-Type", "text/html");
					mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
											"pre-check=0, no-store, no-cache, must-revalidate");
					return pObject->websrv_session( conn );
				}
				else if ( 0 == strcmp(conn->uri, "/vscp/configure") ) {
					if ( NULL == ( pWebSrvSession = pObject->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					mg_send_header(conn, "Content-Type", "text/html");
					mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
											"pre-check=0, no-store, no-cache, must-revalidate");
					return pObject->websrv_configure( conn );
				}
				else if ( 0 == strcmp(conn->uri, "/vscp/bootload") ) {
					if ( NULL == ( pWebSrvSession = pObject->websrv_GetCreateSession( conn ) ) ) return MG_FALSE;
					mg_send_header(conn, "Content-Type", "text/html");
					mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
											"pre-check=0, no-store, no-cache, must-revalidate");
					return pObject->websrv_bootload( conn );
				}
				else if ( 0 == strncmp(conn->uri, "/vscp/rest",10) ) {
					return pObject->websrv_restapi( conn );
				}
				else {
					return MG_FALSE;
				}
			}
			return MG_FALSE;

		case MG_WS_HANDSHAKE:
			if ( NULL != ( hdr = mg_get_header( conn, "Sec-WebSocket-Protocol") ) ) {
					if ( 0 == vscp_strncasecmp( hdr, "very-simple-control-protocol", 28 ) ) {
						//mg_send_header( conn, "Set-Sec-WebSocket-Protocol", "very-simple-control-protocol" );
					/*	
				Currently it is impossible to request a specific protocol
				This is what would be needed to do so.
				
				if ( NULL != ( hdr = mg_get_header( conn, "Sec-WebSocket-Protocol") ) ) {
					if ( 0 == vscp_strncasecmp( hdr, "very-simple-control-protocol", 28 ) ) {
						/*
						static const char *magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
						char buf[500], sha[20], b64_sha[sizeof(sha) * 2];
						SHA1_CTX sha_ctx;

						mg_snprintf(buf, sizeof(buf), "%s%s", key, magic);
						SHA1Init(&sha_ctx);
						SHA1Update(&sha_ctx, (unsigned char *) buf, strlen(buf));
						SHA1Final((unsigned char *) sha, &sha_ctx);
						base64_encode((unsigned char *) sha, sizeof(sha), b64_sha);
						mg_snprintf(buf, sizeof(buf), "%s%s%s",
							"HTTP/1.1 101 Switching Protocols\r\n"
							"Upgrade: websocket\r\n"
							"Connection: Upgrade\r\n"
							"Sec-WebSocket-Protocol: very-simple-control-protocol\r\n"
							"Sec-WebSocket-Accept: ", b64_sha, "\r\n\r\n");

						 mg_write(conn, buf, strlen(buf));
						 
					}		
				}*/
						return MG_FALSE;
					}
			}
			return MG_FALSE;

		case MG_WS_CONNECT:

			// New websocket connection. Send connection ID back to the client.
			if ( conn->is_websocket ) {

				// Get session
				pWebSockSession = websock_get_session( conn );
				if ( NULL == pWebSockSession ) {
					mg_websocket_printf( conn, 
											WEBSOCKET_OPCODE_TEXT, 
											"-;%d;%s",
											WEBSOCK_ERROR_NOT_AUTHORIZED,
											WEBSOCK_STR_ERROR_NOT_AUTHORIZED );
					return MG_FALSE;
				}

				if ( pObject->m_bAuthWebsockets ) {

					// Start authentication
					mg_websocket_printf( conn, 
											WEBSOCKET_OPCODE_TEXT, 
											"+;AUTH0;%s", 
											pWebSockSession->m_sid );
				}
				else {
					// No authenticateion will be performed
					
					pWebSockSession->bAuthenticated = true;	// Authenticated
					mg_websocket_printf( conn, 
											WEBSOCKET_OPCODE_TEXT, 
											"+;AUTH1" );
				}
			}

			return MG_TRUE;  // keep socket open

		case MG_REPLY:
			return MG_TRUE;

		case MG_POLL:
			if ( ( cleanupTime - time(NULL) ) > 60 ) {
				pObject->websrv_expire_sessions( conn );
				pObject->websock_expire_sessions( conn );
				pObject->websrv_expire_rest_sessions( conn );
				cleanupTime = time(NULL);
			}
			return MG_FALSE;

		case MG_HTTP_ERROR:
			return MG_FALSE;

		case MG_CLOSE:
			if ( conn->is_websocket ) {
				//	free( conn->connection_param );
				conn->connection_param = NULL;
				pWebSockSession = websock_get_session( conn );
				if ( NULL != pWebSockSession ) {
					pWebSockSession->lastActiveTime  = 0; // time(NULL) + 60 * 60 + 777;  // Mark as staled
					pObject->websock_expire_sessions( conn );
				}
			}
			return MG_TRUE;

		default: 
			return MG_FALSE;
	}
}


///////////////////////////////////////////////////////////////////////////////
// websrv_get_rest_session
//

struct websrv_rest_session *
CControlObject::websrv_new_rest_session( struct mg_connection *conn,
												CUserItem *pUser )
{
	char buf[2048];
	struct websrv_rest_session *ret = NULL;

	// Check pointer
    if (NULL == conn) return NULL;
	
	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return NULL;

	// create fresh session 
    ret = (struct websrv_rest_session *)calloc(1, sizeof(struct websrv_rest_session));
    if  (NULL == ret ) {
#ifndef WIN32
        syslog(LOG_ERR, "calloc error: %s\n", strerror(errno));
#endif
        return NULL;
    }

	// Generate a random session ID
    time_t t;
    t = time( NULL );
    sprintf( buf,
				"__VSCP__DAEMON_REST%X%X%X%X_be_hungry_stay_foolish_%X%X",
				(unsigned int)rand(),
				(unsigned int)rand(),
				(unsigned int)rand(),
				(unsigned int)t,
				(unsigned int)rand(), 
				1337 );

	Cmd5 md5( (unsigned char *)buf );
	strcpy( ret->m_sid, md5.getDigest() );

	// New client
	ret->m_pClientItem = new CClientItem();	// Create client        
    vscp_clearVSCPFilter(&ret->m_pClientItem->m_filterVSCP); // Clear filter
    ret->m_pClientItem->m_type = CLIENT_ITEM_INTERFACE_TYPE_CLIENT_WEB;
    ret->m_pClientItem->m_strDeviceName = _("Internal daemon websocket client. Started at ");
    wxDateTime now = wxDateTime::Now();
    ret->m_pClientItem->m_strDeviceName += now.FormatISODate();
    ret->m_pClientItem->m_strDeviceName += _(" ");
    ret->m_pClientItem->m_strDeviceName += now.FormatISOTime();
	ret->m_pClientItem->m_bOpen = true;		// Open client

	// Add the client to the Client List
    pObject->m_wxClientMutex.Lock();
    pObject->addClient(ret->m_pClientItem);
    pObject->m_wxClientMutex.Unlock();

	ret->m_pUserItem = pUser;
	ret->m_pClientItem = NULL;
	
    // Add to linked list
    ret->lastActiveTime = time(NULL);
    ret->m_next = websrv_rest_sessions;
	websrv_rest_sessions = ret;

	return ret;
}


///////////////////////////////////////////////////////////////////////////////
// websrv_get_rest_session
//

struct websrv_rest_session *
CControlObject::websrv_get_rest_session( struct mg_connection *conn,
												wxString &SessionId )
{
	struct websrv_rest_session *ret = NULL;

	// Check pointers
	if ( NULL == conn) return NULL;
        
	// find existing session 
	ret = websrv_rest_sessions;
    while (NULL != ret) {
		if  (0 == strcmp( SessionId.mb_str(), ret->m_sid) )  break;
        ret = ret->m_next;
	}
        
	if (NULL != ret) {
		ret->lastActiveTime = time( NULL );
        return ret;
	}
    
	return ret;
}

void
CControlObject::websrv_expire_rest_sessions( struct mg_connection *conn )
{
	struct websrv_rest_session *pos;
    struct websrv_rest_session *prev;
    struct websrv_rest_session *next;
    time_t now;

	// Check pointer
    if (NULL == conn) return;
	
	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return;

    now = time( NULL );
    prev = NULL;
    pos = websrv_rest_sessions;
    
    while (NULL != pos) {
        
        next = pos->m_next;
        
        if (now - pos->lastActiveTime > 10 * 60) {	// Ten minutes
        
            // expire sessions after 1h 
            if ( NULL == prev ) {
                websrv_rest_sessions = pos->m_next;
            }
            else {
                prev->m_next = next;
            }
            
			pObject->m_wxClientMutex.Lock();
			pObject->removeClient( pos->m_pClientItem );
			pObject->m_wxClientMutex.Unlock();

            free( pos );
            
        } 
        else {
            prev = pos;
        }
        
        pos = next;
    }
}


// Hash with key value pairs
WX_DECLARE_STRING_HASH_MAP( wxString, hashArgs );


///////////////////////////////////////////////////////////////////////////////
// make_chunk
//

static void webserv_util_make_chunk( char *obuf, const char *buf, int len) {
  char chunk_size[512];
#ifdef WIN32
  int n = _snprintf( chunk_size, sizeof(chunk_size), "%X\r\n", len);
#else
  int n = snprintf( chunk_size, sizeof(chunk_size), "%X\r\n", len);
#endif
  strncat( obuf, chunk_size, n);
  strncat( obuf, buf, len);
  strncat( obuf, "\r\n", 2);
}


static void webserv_util_sendheader( struct mg_connection *conn, const int returncode, const char *content )
{
	char date[64];
	time_t curtime = time(NULL);
	vscp_getTimeString( date, sizeof(date), &curtime );

	mg_send_status( conn, returncode );
	mg_send_header( conn, "Content-Type", content );
	mg_send_header( conn, "Date", date );
	mg_send_header( conn, "Connection", "keep-alive" );
	mg_send_header( conn, "Transfer-Encoding", "chunked" );
	mg_send_header(conn, "Cache-Control", "max-age=0, post-check=0, "
												"pre-check=0, no-store, no-cache, must-revalidate");
}


///////////////////////////////////////////////////////////////////////////////
// websrv_restapi
//

int
CControlObject::websrv_restapi( struct mg_connection *conn )
{
	char buf[2048];
	char date[64];
	int rv = MG_FALSE;
	wxString str;
	time_t curtime = time(NULL);
	long format = REST_FORMAT_PLAIN;
	hashArgs keypairs;
	struct websrv_rest_session *pSession = NULL;

	// Make string with GMT time
	vscp_getTimeString( date, sizeof(date), &curtime );

	// Check pointer
    if (NULL == conn) return MG_FALSE;
	
	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

	vscp_getTimeString( date, sizeof(date), &curtime );

	// get parameters
	wxStringTokenizer tkz( wxString::FromAscii( conn->query_string ), _("&") );
	while ( tkz.HasMoreTokens() ) {
		int pos;
		str = tkz.GetNextToken();
		if ( wxNOT_FOUND != ( pos = str.Find('=') ) ) {
			keypairs[ str.Left(pos).Upper() ] = str.Right( str.Length()-pos-1 ); 
		}
	}

	// Check if user is valid			
	CUserItem *pUser = pObject->m_userList.getUser( keypairs[_("USER")] );
	if ( NULL == pUser ) return MG_FALSE;

	// Check if remote ip is valid
	bool bValidHost;
	pObject->m_mutexUserList.Lock();
	bValidHost = pObject->m_userList.checkRemote( pUser, 
													wxString::FromAscii( conn->remote_ip ) );
	pObject->m_mutexUserList.Unlock();
	if (!bValidHost) return MG_FALSE;

	// Is this an authorized user?
	wxString str3 = keypairs[_("PASSWORD")];
	if ( keypairs[_("PASSWORD")] != pUser->m_md5Password ) return MG_FALSE;

	// Get format
	if ( _("PLAIN") == keypairs[_("FORMAT")].Upper() ) {
		format = REST_FORMAT_PLAIN;
	}
	else if ( _("CSV") == keypairs[_("FORMAT")].Upper() ) {
		format = REST_FORMAT_CSV;
	}
	else if ( _("XML") == keypairs[_("FORMAT")].Upper() ) {
		format = REST_FORMAT_XML;
	}
	else if ( _("JSON") == keypairs[_("FORMAT")].Upper() ) {
		format = REST_FORMAT_JSON;
	}
	else if ( _("JSONP") == keypairs[_("FORMAT")].Upper() ) {
		format = REST_FORMAT_JSONP;
	}
	else if ( _("") != keypairs[_("FORMAT")].Upper() ) {
		keypairs[_("FORMAT")].ToLong( &format );
	}
	else {
		webserv_util_sendheader( conn, 400, "text/plain" );

		mg_write( conn, "\r\n", 2 );		// head/body Separator
		memset( buf, 0, sizeof( buf ) );
		webserv_util_make_chunk( buf, REST_PLAIN_ERROR_UNSUPPORTED_FORMAT, strlen( REST_PLAIN_ERROR_UNSUPPORTED_FORMAT ) );
		mg_write( conn, buf, strlen( buf ) );

		mg_write( conn, "0\r\n\r\n", 5);	// Terminator

		return MG_TRUE;
	}

	// If we have a session key we try to get the session
	if ( _("") != keypairs[_("SESSION")] ) {
		pSession = websrv_get_rest_session( conn, keypairs[_("SESSION")] );
	}

	//   *************************************************************
	//   * * * * * * * *  Status (hold session open)   * * * * * * * *
	//   *************************************************************
	if ( ( '0' == keypairs[_("OP")] ) ||  ( _("STATUS") == keypairs[_("OP")] ) ) {
		rv = webserv_rest_doStatus( conn, pSession, format );
	}

	//  ********************************************
	//  * * * * * * * * open session * * * * * * * *
	//  ********************************************
	else if ( ( '1' == keypairs[_("OP")] ) || ( _("OPEN") == keypairs[_("OP")] ) ) {
		rv = webserv_rest_doOpen( conn, pSession, pUser, format );		
	}

	//   **********************************************
	//   * * * * * * * * close session  * * * * * * * *
	//   **********************************************
	else if ( ( '2' == keypairs[_("OP")] ) || ( _("CLOSE") == keypairs[_("OP")] ) ) {
		rv = webserv_rest_doOpen( conn, pSession, pUser, format );
	}

	//  ********************************************
	//   * * * * * * * * Send event  * * * * * * * *
	//  ********************************************
	else if ( ( '3' == keypairs[_("OP")] ) || ( _("SENDEVENT") == keypairs[_("OP")] ) ) {
		vscpEvent vscpevent;
		if ( _("") != keypairs[_("VSCPEVENT")] ) {
			;
			vscp_getVscpEventFromString( &vscpevent, keypairs[_("VSCPEVENT")] ); 
			rv = webserv_rest_doSendEvent( conn, pSession, format, &vscpevent );
		}
		else {
			// Parameter missing - No Event
			webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_MISSING_DATA );

		}
	}

	//  ********************************************
	//   * * * * * * * * Read event  * * * * * * * *
	//  ********************************************
	else if ( ( '4' == keypairs[_("OP")] ) || ( _("READEVENT") == keypairs[_("OP")] ) ) {
		long count = 1;
		if ( _("") != keypairs[_("COUNT")].Upper() ) {
			keypairs[_("COUNT")].ToLong( &count );
		}
		rv = webserv_rest_doReceiveEvent( conn, pSession, format, count );
	}

	//   **************************************************
	//   * * * * * * * * Set (read) filter  * * * * * * * *
	//   **************************************************
	else if ( ( '5' == keypairs[_("OP")] ) || ( _("SETFILTER") == keypairs[_("OP")] ) ) {
		
		vscpEventFilter vscpfilter;
		if ( _("") != keypairs[_("VSCPFILTER")] ) {
			;
			vscp_readFilterFromString( &vscpfilter, keypairs[_("VSCPFILTER")] ); 
			rv = webserv_rest_doSetFilter( conn, pSession, format, vscpfilter );
		}
		else {
			webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_MISSING_DATA );
		}
	}

	//   ****************************************************
	//   * * * * * * * *  clear input queue   * * * * * * * *
	//   ****************************************************
	else if ( ( '6' == keypairs[_("OP")] ) || ( _("CLEARQUEUE") == keypairs[_("OP")] ) ) {
		webserv_rest_doClearQueue( conn, pSession, format );
	}
	
	//   ****************************************************
	//   * * * * * * * * read variable value  * * * * * * * *
	//   ****************************************************
	else if ( ( '7' == keypairs[_("OP")] ) || ( _("READVAR") == keypairs[_("OP")] ) ) {
		if ( _("") != keypairs[_("VARIABLE")] ) {
			rv = webserv_rest_doReadVariable( conn, pSession, format, keypairs[_("VARIABLE")] );
		}
		else {
			webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_MISSING_DATA );
		}
	}

	//   *****************************************************
	//   * * * * * * * * Write variable value  * * * * * * * *
	//   *****************************************************
	else if ( ( '8' == keypairs[_("OP")] ) || ( _("WRITEVAR") == keypairs[_("OP")] ) ) {

		if ( _("") != keypairs[_("VARIABLE")] ) {
			rv = webserv_rest_doWriteVariable( conn, pSession, format, keypairs[_("VARIABLE")] );
		}
		else {
			webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_MISSING_DATA );
		}	
	}

	//   *************************************************
	//   * * * * * * * * Send measurement  * * * * * * * *
	//   *************************************************
	//	 value,unit=0,sensor=0
	//
	else if ( ( '9' == keypairs[_("OP")] ) || ( _("MEASUREMENT") == keypairs[_("OP")] ) ) {

		if ( ( _("") != keypairs[_("MEASUREMENT")] ) && (_("") != keypairs[_("TYPE")]) ) {
			
			rv = webserv_rest_doWriteMeasurement( conn, pSession, format, 
													keypairs[_("TYPE")],
													keypairs[_("MEASUREMENT")],
													keypairs[_("UNIT")],
													keypairs[_("SENSORIDX")] );
		}
		else {
			webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_MISSING_DATA );
		}	
	}

	//   *******************************************
	//   * * * * * * * * Table read  * * * * * * * *
	//   *******************************************
	else if ( ( '10' == keypairs[_("op")] ) || ( _("TABLE") == keypairs[_("op")] ) ) {
		if ( _("") != keypairs[_("NAME")] ) {
			
			rv = webserv_rest_doGetTableData( conn, pSession, format, 
													keypairs[_("NAME")],
													keypairs[_("FROM")],
													keypairs[_("TO")] );
		}
		else {
			webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_MISSING_DATA );
		}		
	}

	return rv;
}



///////////////////////////////////////////////////////////////////////////////
// webserv_rest_error
//

void
CControlObject::webserv_rest_error( struct mg_connection *conn, 
										struct websrv_rest_session *pSession, 
										int format,
										int errorcode)
{
	char buf[2048];

	if ( REST_FORMAT_PLAIN == format ) {
		webserv_util_sendheader( conn, 400, REST_MIME_TYPE_PLAIN );
				
		mg_write( conn, "\r\n", 2 );		// head/body Separator
				
		memset( buf, 0, sizeof( buf ));
		webserv_util_make_chunk( buf, rest_errors[errorcode][REST_FORMAT_PLAIN], strlen( rest_errors[errorcode][REST_FORMAT_PLAIN] ) );
		mg_write( conn, buf, strlen( buf ) );

		mg_write( conn, "0\r\n\r\n", 5);	// Terminator
		return;
	}
	else if ( REST_FORMAT_CSV == format ) {
		webserv_util_sendheader( conn, 400, REST_MIME_TYPE_CSV );
	
		mg_write( conn, "\r\n", 2 );		// head/body Separator
				
		memset( buf, 0, sizeof( buf ));
		webserv_util_make_chunk( buf, rest_errors[errorcode][REST_FORMAT_CSV], strlen( rest_errors[errorcode][REST_FORMAT_CSV] ) );
		mg_write( conn, buf, strlen( buf ) );

		mg_write( conn, "0\r\n\r\n", 5);	// Terminator
		return;
	}
	else if ( REST_FORMAT_XML == format ) {
		webserv_util_sendheader( conn, 400, REST_MIME_TYPE_XML );

		mg_write( conn, "\r\n", 2 );		// head/body Separator

		memset( buf, 0, sizeof( buf ) );
		webserv_util_make_chunk( buf, XML_HEADER, strlen( XML_HEADER ) );
		mg_write( conn, buf, strlen( buf ) );
				
		memset( buf, 0, sizeof( buf ) );				
		webserv_util_make_chunk( buf, rest_errors[errorcode][REST_FORMAT_XML], strlen( rest_errors[errorcode][REST_FORMAT_XML] ) );
		mg_write( conn, buf, strlen( buf ) );
				
		mg_write( conn, "0\r\n\r\n", 5);	// Terminator

		return;
	}
	else if ( REST_FORMAT_JSON == format ) {
		webserv_util_sendheader( conn, 400, REST_MIME_TYPE_JSON );

		mg_write( conn, "\r\n", 2 );		// head/body Separator
		memset( buf, 0, sizeof( buf ) );
		webserv_util_make_chunk( buf, rest_errors[errorcode][REST_FORMAT_JSON], strlen( rest_errors[errorcode][REST_FORMAT_JSON] ) );
		mg_write( conn, buf, strlen( buf ) );

		mg_write( conn, "0\r\n\r\n", 5);	// Terminator

		return;
	}
	else if ( REST_FORMAT_JSONP == format ) {
		webserv_util_sendheader( conn, 400, REST_MIME_TYPE_JSONP );

		mg_write( conn, "\r\n", 2 );		// head/body Separator
		memset( buf, 0, sizeof( buf ) );
		webserv_util_make_chunk( buf, rest_errors[errorcode][REST_FORMAT_JSONP], strlen( rest_errors[errorcode][REST_FORMAT_JSONP] ) );
		mg_write( conn, buf, strlen( buf ) );

		mg_write( conn, "0\r\n\r\n", 5);	// Terminator

		return;
	}
	else {
		webserv_util_sendheader( conn, 400, REST_MIME_TYPE_PLAIN );

		mg_write( conn, "\r\n", 2 );		// head/body Separator
		memset( buf, 0, sizeof( buf ) );
		webserv_util_make_chunk( buf, REST_PLAIN_ERROR_UNSUPPORTED_FORMAT, strlen( REST_PLAIN_ERROR_UNSUPPORTED_FORMAT ) );
		mg_write( conn, buf, strlen( buf ) );

		mg_write( conn, "0\r\n\r\n", 5);	// Terminator

		return;
	}	
}



///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doStatus
//

int
CControlObject::webserv_rest_doSendEvent( struct mg_connection *conn, 
											struct websrv_rest_session *pSession, 
											int format, 
											vscpEvent *pEvent )
{
	bool bSent = false;

	// Check pointer
    if (NULL == conn) return MG_FALSE;
	
	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

	if ( NULL != pSession ) {

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

			if (NULL != pSession->m_pClientItem ) {

				// Set client id 
				pEvent->obid = pSession->m_pClientItem->m_clientID;

				// Check if filtered out
				if (vscp_doLevel2Filter( pEvent, &pSession->m_pClientItem->m_filterVSCP)) {

					// Lock client
					pObject->m_wxClientMutex.Lock();

					// If the client queue is full for this client then the
					// client will not receive the message
					if (pSession->m_pClientItem->m_clientInputQueue.GetCount() <=
							pObject->m_maxItemsInClientReceiveQueue) {

						vscpEvent *pNewEvent = new( vscpEvent );
						if ( NULL != pNewEvent ) {

							vscp_copyVSCPEvent(pNewEvent, pEvent);

							// Add the new event to the inputqueue
							pSession->m_pClientItem->m_mutexClientInputQueue.Lock();
							pSession->m_pClientItem->m_clientInputQueue.Append( pNewEvent );
							pSession->m_pClientItem->m_semClientInputQueue.Post();

							bSent = true;
						}
						else {
							bSent = false;
						}

					} 
					else {
						// Overun - No room for event
						//vscp_deleteVSCPevent( pEvent );
						bSent = true;
					}

					// Unlock client
					pObject->m_wxClientMutex.Unlock();

				} // filter

			} // Client found
		}

		if (!bSent) {

			if (NULL != pSession->m_pClientItem ) {

				// Set client id 
				pEvent->obid = pSession->m_pClientItem->m_clientID;

				// There must be room in the send queue
				if (pObject->m_maxItemsInClientReceiveQueue >
						pObject->m_clientOutputQueue.GetCount()) {

					vscpEvent *pNewEvent = new( vscpEvent );
					if ( NULL != pNewEvent ) {
						vscp_copyVSCPEvent(pNewEvent, pEvent);
					
						pObject->m_mutexClientOutputQueue.Lock();
						pObject->m_clientOutputQueue.Append(pNewEvent);
						pObject->m_semClientOutputQueue.Post();
						pObject->m_mutexClientOutputQueue.Unlock();					

						bSent = true;
					}
					else {
						bSent = false;
					}
				}
				else {
					vscp_deleteVSCPevent( pEvent );
					bSent = false;
				}

			}
			else {
				vscp_deleteVSCPevent( pEvent );
				bSent = false;
			}

		}
	}
	else {
		webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
		bSent = false;
	}

	return MG_TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doReadEvent
//

int
CControlObject::webserv_rest_doReceiveEvent( struct mg_connection *conn, 
												struct websrv_rest_session *pSession, 
												int format,
												long count )
{
	// Check pointer
    if (NULL == conn) return MG_FALSE;
	
	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

	if ( NULL != pSession ) {

		if ( !pSession->m_pClientItem->m_clientInputQueue.empty() ) {

			char buf[32000];
			char wrkbuf[32000];
			wxString out;

			if ( REST_FORMAT_PLAIN == format ) {

				webserv_util_sendheader( conn, 400, REST_MIME_TYPE_PLAIN );
				
				mg_write( conn, "\r\n", 2 );		// head/body Separator

				if ( pSession->m_pClientItem->m_bOpen &&
					pSession->m_pClientItem->m_clientInputQueue.GetCount()) {

					memset( buf, 0, sizeof( buf ));
					sprintf( wrkbuf, "1 1 Success \r\n");
					webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
					mg_write( conn, buf, strlen( buf ) );

					memset( buf, 0, sizeof( buf ));
					sprintf( wrkbuf, 
								"%d events requested of %d available (unfiltered) %d will be retrived\r\n", 
								count, 
								pSession->m_pClientItem->m_clientInputQueue.GetCount(),
								min((unsigned long)count,pSession->m_pClientItem->m_clientInputQueue.GetCount()) );
					webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
					mg_write( conn, buf, strlen( buf ) );

					for ( unsigned int i=0; i<min((unsigned long)count,pSession->m_pClientItem->m_clientInputQueue.GetCount()); i++ ) {

						CLIENTEVENTLIST::compatibility_iterator nodeClient;
						vscpEvent *pEvent;

						pSession->m_pClientItem->m_mutexClientInputQueue.Lock();
						nodeClient = pSession->m_pClientItem->m_clientInputQueue.GetFirst();
						pEvent = nodeClient->GetData();
						pSession->m_pClientItem->m_clientInputQueue.DeleteNode(nodeClient);
						pSession->m_pClientItem->m_mutexClientInputQueue.Unlock();

						if (NULL != pEvent) {

							if (vscp_doLevel2Filter(pEvent, &pSession->m_pClientItem->m_filterVSCP)) {

								wxString str;
								if (vscp_writeVscpEventToString(pEvent, str)) {

									// Write it out
									memset((char *) wrkbuf, 0, sizeof( wrkbuf ));
									strcpy((char *) wrkbuf, (const char*) "- ");
									strcat((char *) wrkbuf, (const char*) str.mb_str(wxConvUTF8));
									strcat((char *) wrkbuf, "/r/n" );
									webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
									mg_write( conn, buf, strlen( buf ) );
							
								}	
								else {
									memset( buf, 0, sizeof( buf ) );
									strcpy((char *) wrkbuf, "- Malformed event (intenal error)/r/n" );
									webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
									mg_write( conn, buf, strlen( buf ) );
								}
							}
							else {
								memset( buf, 0, sizeof( buf ) );
								strcpy((char *) wrkbuf, "- Event filtered out/r/n" );
								webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
								mg_write( conn, buf, strlen( buf ) );
							}

							// Remove the event
							vscp_deleteVSCPevent(pEvent);

						} // Valid pEvent pointer
						else {
							memset( buf, 0, sizeof( buf ) );
							strcpy((char *) wrkbuf, "- Event could not be fetched (intenal error)/r/n" );
							webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
							mg_write( conn, buf, strlen( buf ) );
						}
					} // for
				}	 
				else {   // no events available
					memset( buf, 0, sizeof( buf ));
					sprintf( wrkbuf, REST_PLAIN_ERROR_INPUT_QUEUE_EMPTY"\r\n");
					webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
					mg_write( conn, buf, strlen( buf ) );
				}
					
			}
			else if ( REST_FORMAT_CSV == format ) {
				webserv_util_sendheader( conn, 400, REST_MIME_TYPE_CSV );
				
				mg_write( conn, "\r\n", 2 );		// head/body Separator

				if ( pSession->m_pClientItem->m_bOpen &&
					pSession->m_pClientItem->m_clientInputQueue.GetCount()) {

					memset( buf, 0, sizeof( buf ));
					sprintf( wrkbuf, "success-code,error-code,message,description,Event\r\n1,1,Success,Success.,NULL\r\n");
					webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
					mg_write( conn, buf, strlen( buf ) );

					memset( buf, 0, sizeof( buf ));
					sprintf( wrkbuf, 
								"1,2,Info,%d events requested of %d available (unfiltered) %d will be retrived,NULL\r\n", 
								count, 
								pSession->m_pClientItem->m_clientInputQueue.GetCount(),
								min((unsigned long)count,pSession->m_pClientItem->m_clientInputQueue.GetCount()) );
					webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
					mg_write( conn, buf, strlen( buf ) );

					for ( unsigned int i=0; i<min((unsigned long)count,pSession->m_pClientItem->m_clientInputQueue.GetCount()); i++ ) {

						CLIENTEVENTLIST::compatibility_iterator nodeClient;
						vscpEvent *pEvent;

						pSession->m_pClientItem->m_mutexClientInputQueue.Lock();
						nodeClient = pSession->m_pClientItem->m_clientInputQueue.GetFirst();
						pEvent = nodeClient->GetData();
						pSession->m_pClientItem->m_clientInputQueue.DeleteNode(nodeClient);
						pSession->m_pClientItem->m_mutexClientInputQueue.Unlock();

						if (NULL != pEvent) {

							if (vscp_doLevel2Filter(pEvent, &pSession->m_pClientItem->m_filterVSCP)) {

								wxString str;
								if (vscp_writeVscpEventToString(pEvent, str)) {

									// Write it out
									memset((char *) wrkbuf, 0, sizeof( wrkbuf ));
									strcpy((char *) wrkbuf, (const char*) "1,3,Data,Event.,");
									strcat((char *) wrkbuf, (const char*) str.mb_str(wxConvUTF8));
									strcat((char *) wrkbuf, "/r/n" );
									webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
									mg_write( conn, buf, strlen( buf ) );
							
								}	
								else {
									memset( buf, 0, sizeof( buf ) );
									strcpy((char *) wrkbuf, "1,2,Info,Malformed event (intenal error)/r/n" );
									webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
									mg_write( conn, buf, strlen( buf ) );
								}
							}
							else {
								memset( buf, 0, sizeof( buf ) );
								strcpy((char *) wrkbuf, "1,2,Info,Event filtered out/r/n" );
								webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
								mg_write( conn, buf, strlen( buf ) );
							}

							// Remove the event
							vscp_deleteVSCPevent(pEvent);

						} // Valid pEvent pointer
						else {
							memset( buf, 0, sizeof( buf ) );
							strcpy((char *) wrkbuf, "1,2,Info,Event could not be fetched (intenal error)/r/n" );
							webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
							mg_write( conn, buf, strlen( buf ) );
						}
					} // for
				}	 
				else {   // no events available
					memset( buf, 0, sizeof( buf ));
					sprintf( wrkbuf, REST_CSV_ERROR_INPUT_QUEUE_EMPTY"\r\n");
					webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
					mg_write( conn, buf, strlen( buf ) );
				}

			}
			else if ( REST_FORMAT_XML == format ) {

				int filtered = 0;
				int errors = 0;

				webserv_util_sendheader( conn, 400, REST_MIME_TYPE_XML );
				
				mg_write( conn, "\r\n", 2 );		// head/body Separator

				if ( pSession->m_pClientItem->m_bOpen &&
					pSession->m_pClientItem->m_clientInputQueue.GetCount()) {

					memset( buf, 0, sizeof( buf ));
					sprintf( wrkbuf, XML_HEADER"<vscp-rest success = \"true\" code = \"1\" massage = \"Success\" description = \"Success.\" >");
					webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
					mg_write( conn, buf, strlen( buf ) );

					memset( buf, 0, sizeof( buf ));
					sprintf( wrkbuf, 
								"<info>%d events requested of %d available (unfiltered) %d will be retrived</info>", 
								count, 
								pSession->m_pClientItem->m_clientInputQueue.GetCount(),
								min((unsigned long)count,pSession->m_pClientItem->m_clientInputQueue.GetCount()) );
					webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
					mg_write( conn, buf, strlen( buf ) );

					for ( unsigned int i=0; i<min((unsigned long)count,pSession->m_pClientItem->m_clientInputQueue.GetCount()); i++ ) {

						CLIENTEVENTLIST::compatibility_iterator nodeClient;
						vscpEvent *pEvent;

						pSession->m_pClientItem->m_mutexClientInputQueue.Lock();
						nodeClient = pSession->m_pClientItem->m_clientInputQueue.GetFirst();
						pEvent = nodeClient->GetData();
						pSession->m_pClientItem->m_clientInputQueue.DeleteNode(nodeClient);
						pSession->m_pClientItem->m_mutexClientInputQueue.Unlock();

						if (NULL != pEvent) {

							if (vscp_doLevel2Filter(pEvent, &pSession->m_pClientItem->m_filterVSCP)) {

								wxString str;

								// Write it out
								memset((char *) wrkbuf, 0, sizeof( wrkbuf ));
								strcpy((char *) wrkbuf, (const char*) "<event>");

								strcpy((char *) wrkbuf, (const char*) "<head>");
								strcpy((char *) wrkbuf, wxString::Format( _("%d"), pEvent->head ).mb_str() );
								strcpy((char *) wrkbuf, (const char*) "</head>");

								strcpy((char *) wrkbuf, (const char*) "<class>");
								strcpy((char *) wrkbuf, wxString::Format( _("%d"), pEvent->vscp_class ).mb_str() );
								strcpy((char *) wrkbuf, (const char*) "</class>");

								strcpy((char *) wrkbuf, (const char*) "<type>");
								strcpy((char *) wrkbuf, wxString::Format( _("%d"), pEvent->vscp_type ).mb_str() );
								strcpy((char *) wrkbuf, (const char*) "</type>");

								strcpy((char *) wrkbuf, (const char*) "<obid>");
								strcpy((char *) wrkbuf, wxString::Format( _("%d"), pEvent->obid ).mb_str() );
								strcpy((char *) wrkbuf, (const char*) "</obid>");

								strcpy((char *) wrkbuf, (const char*) "<timestamp>");
								strcpy((char *) wrkbuf, wxString::Format( _("%d"), pEvent->timestamp ).mb_str() );
								strcpy((char *) wrkbuf, (const char*) "</timestamp>");

								strcpy((char *) wrkbuf, (const char*) "<guid>");
								vscp_writeGuidToString( pEvent, str);
								strcpy((char *) wrkbuf, str.mbc_str() );
								strcpy((char *) wrkbuf, (const char*) "</guid>");

								strcpy((char *) wrkbuf, (const char*) "<sizedata>");
								strcpy((char *) wrkbuf, wxString::Format( _("%d"), pEvent->sizeData ).mb_str() );
								strcpy((char *) wrkbuf, (const char*) "</sizedata>");

								strcpy((char *) wrkbuf, (const char*) "<data>");
								vscp_writeVscpDataToString( pEvent, str);
								strcpy((char *) wrkbuf, str.mbc_str() );
								strcpy((char *) wrkbuf, (const char*) "</data>");

								strcat((char *) wrkbuf, "</event>" );
								webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
								mg_write( conn, buf, strlen( buf ) );

								if (vscp_writeVscpEventToString(pEvent, str)) {

									// Write it out
									memset((char *) wrkbuf, 0, sizeof( wrkbuf ));
									strcpy((char *) wrkbuf, (const char*) "<event>");
									strcat((char *) wrkbuf, (const char*) str.mb_str(wxConvUTF8));
									strcat((char *) wrkbuf, "</event>" );
									webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
									mg_write( conn, buf, strlen( buf ) );
							
								}	
								else {
									errors++;
								}
							}
							else {
								filtered++;
							}

							// Remove the event
							vscp_deleteVSCPevent(pEvent);

						} // Valid pEvent pointer
						else {
							errors++;
						}
					} // for

					strcpy((char *) wrkbuf, (const char*) "<filtered>");
					strcpy((char *) wrkbuf, wxString::Format( _("%d"), filtered ).mb_str() );
					strcpy((char *) wrkbuf, (const char*) "</filtered>");

					strcpy((char *) wrkbuf, (const char*) "<errors>");
					strcpy((char *) wrkbuf, wxString::Format( _("%d"), errors ).mb_str() );
					strcpy((char *) wrkbuf, (const char*) "</errors>");

					// End tag
					memset( buf, 0, sizeof( buf ) );
					strcpy((char *) wrkbuf, "</vscp-rest>" );
					webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
					mg_write( conn, buf, strlen( buf ) );

				}	 
				else {   // no events available
					memset( buf, 0, sizeof( buf ));
					sprintf( wrkbuf, REST_XML_ERROR_INPUT_QUEUE_EMPTY"\r\n");
					webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
					mg_write( conn, buf, strlen( buf ) );
				}

			}
			else if ( ( REST_FORMAT_JSON == format ) && ( REST_FORMAT_JSONP == format ) ) {
				
				int filtered = 0;
				int errors = 0;
				char *p = buf;
				webserv_util_sendheader( conn, 400, REST_MIME_TYPE_JSON );
				
				mg_write( conn, "\r\n", 2 );		// head/body Separator

				if ( pSession->m_pClientItem->m_bOpen &&
						pSession->m_pClientItem->m_clientInputQueue.GetCount() ) {

					if ( REST_FORMAT_JSONP == format ) {
						p += json_emit_raw_str( p, &buf[sizeof(buf)] - p, "func(" );
					}

					p += json_emit_raw_str( p, &buf[sizeof(buf)] - p, "{\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\"," );
					p += json_emit_quoted_str( p, &buf[sizeof(buf)] - p, "info" );
					p += json_emit_raw_str( p, &buf[sizeof(buf)] - p, ":" );					
					sprintf( wrkbuf, 
								"%d events requested of %d available (unfiltered) %d will be retrived", 
								count, 
								pSession->m_pClientItem->m_clientInputQueue.GetCount(),
								min((unsigned long)count,pSession->m_pClientItem->m_clientInputQueue.GetCount()) );
					p += json_emit_quoted_str( p, &buf[sizeof(buf)] - p, wrkbuf );
					p += json_emit_raw_str( p, &buf[sizeof(buf)] - p, "," );
					p += json_emit_quoted_str(p, &buf[sizeof(buf)] - p, "event");
					p += json_emit_raw_str( p, &buf[sizeof(buf)] - p, ":[" );

					webserv_util_make_chunk( wrkbuf, buf, strlen( buf) );
					mg_write( conn, buf, strlen( wrkbuf ) );
					memset( buf, 0, sizeof( buf ) );
					p = buf;

					for ( unsigned int i=0; i<min((unsigned long)count,pSession->m_pClientItem->m_clientInputQueue.GetCount()); i++ ) {

						CLIENTEVENTLIST::compatibility_iterator nodeClient;
						vscpEvent *pEvent;

						pSession->m_pClientItem->m_mutexClientInputQueue.Lock();
						nodeClient = pSession->m_pClientItem->m_clientInputQueue.GetFirst();
						pEvent = nodeClient->GetData();
						pSession->m_pClientItem->m_clientInputQueue.DeleteNode(nodeClient);
						pSession->m_pClientItem->m_mutexClientInputQueue.Unlock();

						if (NULL != pEvent) {

							if (vscp_doLevel2Filter(pEvent, &pSession->m_pClientItem->m_filterVSCP)) {

								wxString str;
								p += json_emit_raw_str( p, &buf[sizeof(buf)] - p, "{" );
								
								// head
								p += json_emit_quoted_str(p, &buf[sizeof(buf)] - p, "head");
								p += json_emit_raw_str( p, &buf[sizeof(buf)] - p, ":" );
								p += json_emit_int( p, &buf[sizeof(buf)] - p, pEvent->head );
								p += json_emit_raw_str( p, &buf[sizeof(buf)] - p, "," );
								
								// class
								p += json_emit_quoted_str(p, &buf[sizeof(buf)] - p, "class");
								p += json_emit_raw_str( p, &buf[sizeof(buf)] - p, ":" );
								p += json_emit_int( p, &buf[sizeof(buf)] - p, pEvent->vscp_class );
								p += json_emit_raw_str( p, &buf[sizeof(buf)] - p, "," );
								
								// type
								p += json_emit_quoted_str(p, &buf[sizeof(buf)] - p, "type");
								p += json_emit_raw_str( p, &buf[sizeof(buf)] - p, ":" );
								p += json_emit_int( p, &buf[sizeof(buf)] - p, pEvent->vscp_type );
								p += json_emit_raw_str( p, &buf[sizeof(buf)] - p, "," );
								
								// timestamp
								p += json_emit_quoted_str(p, &buf[sizeof(buf)] - p, "timestamp");
								p += json_emit_raw_str( p, &buf[sizeof(buf)] - p, ":" );
								p += json_emit_int( p, &buf[sizeof(buf)] - p, pEvent->timestamp );
								p += json_emit_raw_str( p, &buf[sizeof(buf)] - p, "," );
								
								// GUID
								vscp_writeGuidToString( pEvent, str);
								p += json_emit_quoted_str(p, &buf[sizeof(buf)] - p, "guid");
								p += json_emit_raw_str( p, &buf[sizeof(buf)] - p, ":" );
								p += json_emit_quoted_str(p, &buf[sizeof(buf)] - p, str.mb_str() );
								p += json_emit_raw_str( p, &buf[sizeof(buf)] - p, "," );
								
								// SizeData
								p += json_emit_quoted_str(p, &buf[sizeof(buf)] - p, "sizedata");
								p += json_emit_raw_str( p, &buf[sizeof(buf)] - p, ":" );
								p += json_emit_int( p, &buf[sizeof(buf)] - p, pEvent->sizeData );
								p += json_emit_raw_str( p, &buf[sizeof(buf)] - p, "," );
								
								// Data
								p += json_emit_quoted_str(p, &buf[sizeof(buf)] - p, "data");
								p += json_emit_raw_str( p, &buf[sizeof(buf)] - p, ":[" );
								for ( unsigned int j=0; j<pEvent->sizeData; j++ ) {
									p += json_emit_int( p, &buf[sizeof(buf)] - p, pEvent->pdata[j] );
									p += json_emit_raw_str( p, &buf[sizeof(buf)] - p, "," );
								}
								p += json_emit_raw_str( p, &buf[sizeof(buf)] - p, "]}," );

								webserv_util_make_chunk( wrkbuf, buf, strlen( buf) );
								mg_write( conn, buf, strlen( wrkbuf ) );
								memset( buf, 0, sizeof( buf ) );
								p = buf;

							}
							else {
								filtered++;;
							}

							// Remove the event
							vscp_deleteVSCPevent(pEvent);

						} // Valid pEvent pointer
						else {
							errors++;
						}

						// Buffer overflow
						if ( 0 == p ) break;

					} // for

					// Mark end
					p += json_emit_raw_str( p, &buf[sizeof(buf)] - p, "]," );
					p += json_emit_quoted_str(p, &buf[sizeof(buf)] - p, "filtered");
					p += json_emit_raw_str( p, &buf[sizeof(buf)] - p, ":" );
					p += json_emit_int( p, &buf[sizeof(buf)] - p, filtered );
					p += json_emit_raw_str( p, &buf[sizeof(buf)] - p, "," );
					p += json_emit_quoted_str(p, &buf[sizeof(buf)] - p, "errors");
					p += json_emit_raw_str( p, &buf[sizeof(buf)] - p, ":" );
					p += json_emit_int( p, &buf[sizeof(buf)] - p, errors );
					p += json_emit_raw_str( p, &buf[sizeof(buf)] - p, "}" );

					if ( REST_FORMAT_JSONP == format ) {
						p += json_emit_raw_str( p, &buf[sizeof(buf)] - p, ");" );
					}

					webserv_util_make_chunk( wrkbuf, buf, strlen( buf) );
					mg_write( conn, buf, strlen( wrkbuf ) );

				}	 
				else {   // no events available
					memset( buf, 0, sizeof( buf ));
					if ( REST_FORMAT_JSON == format ) {
						sprintf( wrkbuf, REST_JSON_ERROR_INPUT_QUEUE_EMPTY"\r\n");
					}
					else {
						sprintf( wrkbuf, REST_JSONP_ERROR_INPUT_QUEUE_EMPTY"\r\n");
					}
					webserv_util_make_chunk( buf, wrkbuf, strlen( wrkbuf) );
					mg_write( conn, buf, strlen( buf ) );
				}
					
			}
			
			mg_write( conn, "0\r\n\r\n", 5);	// Terminator

		}
		else {	// Que is empty
			webserv_rest_error( conn, pSession, format, RESR_ERROR_CODE_INPUT_QUEUE_EMPTY );
		}

	}
	else {
		if ( REST_FORMAT_PLAIN == format ) {
			webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
		}
	}

	return MG_TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doSetFilter
//

int
CControlObject::webserv_rest_doSetFilter( struct mg_connection *conn, 
											struct websrv_rest_session *pSession, 
											int format,
											vscpEventFilter& vscpfilter )
{
	if ( NULL != pSession ) {
		pSession->m_pClientItem->m_mutexClientInputQueue.Lock();
		memcpy( &pSession->m_pClientItem->m_filterVSCP, &vscpfilter, sizeof( vscpEventFilter ) );
		pSession->m_pClientItem->m_mutexClientInputQueue.Unlock();
		webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_SUCCESS );
	}
	else {
		webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
	}

	return MG_TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doClearQueue
//

int
CControlObject::webserv_rest_doClearQueue( struct mg_connection *conn, 
												struct websrv_rest_session *pSession, 
												int format )
{
	if ( NULL != pSession ) {

		CLIENTEVENTLIST::iterator iterVSCP;

		pSession->m_pClientItem->m_mutexClientInputQueue.Lock();
        
		for (iterVSCP = pSession->m_pClientItem->m_clientInputQueue.begin();
                iterVSCP != pSession->m_pClientItem->m_clientInputQueue.end(); ++iterVSCP) {
            vscpEvent *pEvent = *iterVSCP;
            vscp_deleteVSCPevent(pEvent);
        }

        pSession->m_pClientItem->m_clientInputQueue.Clear();
        pSession->m_pClientItem->m_mutexClientInputQueue.Unlock();
		
		webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_SUCCESS );
	}
	else {
		webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
	}

	return MG_TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doReadVariable
//

int
CControlObject::webserv_rest_doReadVariable( struct mg_connection *conn, 
												struct websrv_rest_session *pSession, 
												int format,
												wxString& strVariableName )
{
	if ( NULL != pSession ) {


		
		webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_SUCCESS );
	}
	else {
		webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
	}

	return MG_TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doWriteVariable
//

int
CControlObject::webserv_rest_doWriteVariable( struct mg_connection *conn, 
												struct websrv_rest_session *pSession, 
												int format,
												wxString& strVariable )
{
	if ( NULL != pSession ) {


		
		webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_SUCCESS );
	}
	else {
		webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
	}

	return MG_TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doWriteMeasurement
//

int
CControlObject::webserv_rest_doWriteMeasurement( struct mg_connection *conn, 
													struct websrv_rest_session *pSession, 
													int format,
													wxString& strType,
													wxString& strMeasurement,
													wxString& strUnit,
													wxString& strSensorIdx )
{
	if ( NULL != pSession ) {


		
		webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_SUCCESS );
	}
	else {
		webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
	}

	return MG_TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doGetTableData
//

int
CControlObject::webserv_rest_doGetTableData( struct mg_connection *conn, 
												struct websrv_rest_session *pSession, 
												int format,
												wxString& strName,
												wxString& strFrom,
												wxString& strTo )
{
	if ( NULL != pSession ) {


		
		webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_SUCCESS );
	}
	else {
		webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
	}

	return MG_TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doStatus
//

int
CControlObject::webserv_rest_doStatus( struct mg_connection *conn, 
											struct websrv_rest_session *pSession, 
											int format )
{
	char buf[2048];
	char wrkbuf[256];

	if ( NULL != pSession ) {

		pSession->lastActiveTime = time(NULL);

		if ( REST_FORMAT_PLAIN == format ) {
			webserv_util_sendheader( conn, 200, REST_MIME_TYPE_PLAIN );

			mg_write( conn, "\r\n", 2 );		// head/body Separator
				
			memset( buf, 0, sizeof( buf ));
			webserv_util_make_chunk( buf, REST_PLAIN_ERROR_SUCCESS, strlen( REST_PLAIN_ERROR_SUCCESS ) );
			mg_write( conn, buf, strlen( buf ) );

			memset( buf, 0, sizeof( buf ));
#ifdef WIN32
			int n = _snprintf( wrkbuf, sizeof(wrkbuf), "1 1 Success Session-id=%s nEvents=%d", pSession->m_sid, pSession->m_pClientItem->m_clientInputQueue.GetCount() );
#else
			int n = snprintf( wrkbuf, sizeof(wrkbuf), "1 1 Success Session-id=%s nEvents=%d", pSession->m_sid, pSession->m_pClientItem->m_clientInputQueue.GetCount() );
#endif
			webserv_util_make_chunk( buf, wrkbuf, n );
			mg_write( conn, buf, strlen( buf ) );

			mg_write( conn, "0\r\n\r\n", 5);	// Terminator
			return MG_TRUE;
		}
		else if ( REST_FORMAT_CSV == format ) {
			webserv_util_sendheader( conn, 200, REST_MIME_TYPE_CSV );

			mg_write( conn, "\r\n", 2 );		// head/body Separator

			memset( buf, 0, sizeof( buf ));
#ifdef WIN32
			int n = _snprintf( wrkbuf, 
					sizeof(wrkbuf), 
					"success-code,error-code,message,description,session-id,nEvents\r\n1,1,Success,Success. 1,1,Success,Sucess,%s,%d", 
					pSession->m_sid, pSession->m_pClientItem->m_clientInputQueue.GetCount() );
#else
			int n = snprintf( wrkbuf, 
					sizeof(wrkbuf), 
					"success-code,error-code,message,description,session-id,nEvents\r\n1,1,Success,Success. 1,1,Success,Sucess,%s,%d", 
					pSession->m_sid, pSession->m_pClientItem->m_clientInputQueue.GetCount() );
#endif
			webserv_util_make_chunk( buf, wrkbuf, n );
			mg_write( conn, buf, strlen( buf ) );

			mg_write( conn, "0\r\n\r\n", 5);	// Terminator
			return MG_TRUE;
		}
		else if ( REST_FORMAT_XML == format ) {
			webserv_util_sendheader( conn, 200, REST_MIME_TYPE_XML );

			mg_write( conn, "\r\n", 2 );		// head/body Separator
				
			memset( buf, 0, sizeof( buf ));
#ifdef WIN32
			int n = _snprintf( wrkbuf, 
					sizeof(wrkbuf), 
					"<vscp-rest success = \"true\" code = \"1\" massage = \"Success.\" description = \"Success.\" ><session-id>%s</session-id><nEvents>%d</nEvents></vscp-rest>", 
					pSession->m_sid, pSession->m_pClientItem->m_clientInputQueue.GetCount() );
#else
			int n = snprintf( wrkbuf, 
					sizeof(wrkbuf), 
					"<vscp-rest success = \"true\" code = \"1\" massage = \"Success.\" description = \"Success.\" ><session-id>%s</session-id><nEvents>%d</nEvents></vscp-rest>", 
					pSession->m_sid, pSession->m_pClientItem->m_clientInputQueue.GetCount() );
#endif
			webserv_util_make_chunk( buf, wrkbuf, n );
			mg_write( conn, buf, strlen( buf ) );

			mg_write( conn, "0\r\n\r\n", 5);	// Terminator
			return MG_TRUE;
		}
		else if ( REST_FORMAT_JSON == format ) {
			webserv_util_sendheader( conn, 200, REST_MIME_TYPE_JSON );

			mg_write( conn, "\r\n", 2 );		// head/body Separator
				
			memset( buf, 0, sizeof( buf ));
#ifdef WIN32
			int n = _snprintf( wrkbuf, 
					sizeof(wrkbuf), 
					"{\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\",\"session-id\":\"%s\",\"nEvents\":%d}", 
					pSession->m_sid, pSession->m_pClientItem->m_clientInputQueue.GetCount() );
#else
			int n = snprintf( wrkbuf, 
					sizeof(wrkbuf), 
					"{\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\",\"session-id\":\"%s\",\"nEvents\":%d}", 
					pSession->m_sid, pSession->m_pClientItem->m_clientInputQueue.GetCount() );
#endif
			webserv_util_make_chunk( buf, wrkbuf, n );
			mg_write( conn, buf, strlen( buf ) );
			
			mg_write( conn, "0\r\n\r\n", 5);	// Terminator
			return MG_TRUE;
		}
		else if ( REST_FORMAT_JSONP == format ) {
			webserv_util_sendheader( conn, 200, REST_MIME_TYPE_JSONP );

			mg_write( conn, "\r\n", 2 );		// head/body Separator
				
			memset( buf, 0, sizeof( buf ));
#ifdef WIN32
			int n = _snprintf( wrkbuf, 
					sizeof(wrkbuf), 
					"func({\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\",\"session-id\":\"%s\",\"nEvents\":%d});", 
					pSession->m_sid, pSession->m_pClientItem->m_clientInputQueue.GetCount() );
#else
			int n = snprintf( wrkbuf, 
					sizeof(wrkbuf), 
					"func({\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\",\"session-id\":\"%s\",\"nEvents\":%d});", 
					pSession->m_sid, pSession->m_pClientItem->m_clientInputQueue.GetCount() );
#endif
			webserv_util_make_chunk( buf, wrkbuf, n );
			mg_write( conn, buf, strlen( buf ) );
			
			mg_write( conn, "0\r\n\r\n", 5);	// Terminator
			return MG_TRUE;
		}
		else {
			webserv_util_sendheader( conn, 400, REST_MIME_TYPE_PLAIN );

			mg_write( conn, "\r\n", 2 );		// head/body Separator
			memset( buf, 0, sizeof( buf ) );
			webserv_util_make_chunk( buf, REST_PLAIN_ERROR_UNSUPPORTED_FORMAT, strlen( REST_PLAIN_ERROR_UNSUPPORTED_FORMAT ) );
			mg_write( conn, buf, strlen( buf ) );

			mg_write( conn, "0\r\n\r\n", 5);	// Terminator

			return MG_TRUE;
		}
			
	} // No session
	else {	
		webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
	}

	return MG_TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doOpen
//

int
CControlObject::webserv_rest_doOpen( struct mg_connection *conn, 
										struct websrv_rest_session *pSession,
										CUserItem *pUser,
										int format )
{
	char buf[2048];
	char wrkbuf[256];

	pSession = websrv_new_rest_session( conn, pUser );
	if ( NULL != pSession ) {
		
		// New session created

		if ( REST_FORMAT_PLAIN == format ) {
			webserv_util_sendheader( conn, 200, REST_MIME_TYPE_PLAIN );

			mg_write( conn, "\r\n", 2 );		// head/body Separator
				
			memset( buf, 0, sizeof( buf ));
			webserv_util_make_chunk( buf, REST_PLAIN_ERROR_SUCCESS, strlen( REST_PLAIN_ERROR_SUCCESS ) );
			mg_write( conn, buf, strlen( buf ) );

			memset( buf, 0, sizeof( buf ));
#ifdef WIN32
			int n = _snprintf( wrkbuf, sizeof(wrkbuf), "1 1 Success Session-id=%s nEvents=%d", pSession->m_sid, pSession->m_pClientItem->m_clientInputQueue.GetCount() );
#else
			int n = snprintf( wrkbuf, sizeof(wrkbuf), "1 1 Success Session-id=%s nEvents=%d", pSession->m_sid, pSession->m_pClientItem->m_clientInputQueue.GetCount() );
#endif
			webserv_util_make_chunk( buf, wrkbuf, n );
			mg_write( conn, buf, strlen( buf ) );

			mg_write( conn, "0\r\n\r\n", 5);	// Terminator
			return MG_TRUE;
		}
		else if ( REST_FORMAT_CSV == format ) {
			webserv_util_sendheader( conn, 200, REST_MIME_TYPE_CSV );

			mg_write( conn, "\r\n", 2 );		// head/body Separator

			memset( buf, 0, sizeof( buf ));
#ifdef WIN32
			int n = _snprintf( wrkbuf, 
					sizeof(wrkbuf), 
					"success-code,error-code,message,description,session-id,nEvents\r\n1,1,Success,Success. 1,1,Success,Sucess,%s,%d", 
					pSession->m_sid, pSession->m_pClientItem->m_clientInputQueue.GetCount() );
#else
			int n = snprintf( wrkbuf, 
					sizeof(wrkbuf), 
					"success-code,error-code,message,description,session-id,nEvents\r\n1,1,Success,Success. 1,1,Success,Sucess,%s,%d", 
					pSession->m_sid, pSession->m_pClientItem->m_clientInputQueue.GetCount() );
#endif
			webserv_util_make_chunk( buf, wrkbuf, n );
			mg_write( conn, buf, strlen( buf ) );

			mg_write( conn, "0\r\n\r\n", 5);	// Terminator
			return MG_TRUE;
		}
		else if ( REST_FORMAT_XML == format ) {
			webserv_util_sendheader( conn, 200, REST_MIME_TYPE_XML );

			mg_write( conn, "\r\n", 2 );		// head/body Separator
				
			memset( buf, 0, sizeof( buf ));
#ifdef WIN32
			int n = _snprintf( wrkbuf, 
					sizeof(wrkbuf), 
					"<vscp-rest success = \"true\" code = \"1\" massage = \"Success.\" description = \"Success.\" ><session-id>%s</session-id><nEvents>%d</nEvents></vscp-rest>", 
					pSession->m_sid, pSession->m_pClientItem->m_clientInputQueue.GetCount() );
#else
			int n = snprintf( wrkbuf, 
					sizeof(wrkbuf), 
					"<vscp-rest success = \"true\" code = \"1\" massage = \"Success.\" description = \"Success.\" ><session-id>%s</session-id><nEvents>%d</nEvents></vscp-rest>", 
					pSession->m_sid, pSession->m_pClientItem->m_clientInputQueue.GetCount() );
#endif
			webserv_util_make_chunk( buf, wrkbuf, n );
			mg_write( conn, buf, strlen( buf ) );

			mg_write( conn, "0\r\n\r\n", 5);	// Terminator
			return MG_TRUE;
		}
		else if ( REST_FORMAT_JSON == format ) {
			webserv_util_sendheader( conn, 200, REST_MIME_TYPE_JSON);

			mg_write( conn, "\r\n", 2 );		// head/body Separator
				
			memset( buf, 0, sizeof( buf ));
#ifdef WIN32
			int n = _snprintf( wrkbuf, 
					sizeof(wrkbuf), 
					"{\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\",\"session-id\":\"%s\",\"nEvents\":%d}", 
					pSession->m_sid, pSession->m_pClientItem->m_clientInputQueue.GetCount() );
#else
			int n = snprintf( wrkbuf, 
					sizeof(wrkbuf), 
					"{\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\",\"session-id\":\"%s\",\"nEvents\":%d}", 
					pSession->m_sid, pSession->m_pClientItem->m_clientInputQueue.GetCount() );
#endif
			webserv_util_make_chunk( buf, wrkbuf, n );
			mg_write( conn, buf, strlen( buf ) );
			
			mg_write( conn, "0\r\n\r\n", 5);	// Terminator
			return MG_TRUE;
		}
		else if ( REST_FORMAT_JSONP == format ) {
			webserv_util_sendheader( conn, 200, REST_MIME_TYPE_JSONP);

			mg_write( conn, "\r\n", 2 );		// head/body Separator
				
			memset( buf, 0, sizeof( buf ));
#ifdef WIN32
			int n = _snprintf( wrkbuf, 
					sizeof(wrkbuf), 
					"func({\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\",\"session-id\":\"%s\",\"nEvents\":%d});", 
					pSession->m_sid, pSession->m_pClientItem->m_clientInputQueue.GetCount() );
#else
			int n = snprintf( wrkbuf, 
					sizeof(wrkbuf), 
					"func({\"success\":true,\"code\":1,\"message\":\"success\",\"description\":\"Success\",\"session-id\":\"%s\",\"nEvents\":%d});", 
					pSession->m_sid, pSession->m_pClientItem->m_clientInputQueue.GetCount() );
#endif
			webserv_util_make_chunk( buf, wrkbuf, n );
			mg_write( conn, buf, strlen( buf ) );
			
			mg_write( conn, "0\r\n\r\n", 5);	// Terminator
			return MG_TRUE;
		}
		else {
			webserv_util_sendheader( conn, 400, REST_MIME_TYPE_PLAIN );

			mg_write( conn, "\r\n", 2 );		// head/body Separator
			memset( buf, 0, sizeof( buf ) );
			webserv_util_make_chunk( buf, REST_PLAIN_ERROR_UNSUPPORTED_FORMAT, strlen( REST_PLAIN_ERROR_UNSUPPORTED_FORMAT ) );
			mg_write( conn, buf, strlen( buf ) );

			mg_write( conn, "0\r\n\r\n", 5);	// Terminator

			return MG_TRUE;
		}
	}
	else {		// Unable to create session	
		webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
	}

	return MG_TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// webserv_rest_doClose
//

int
CControlObject::webserv_rest_doClose( struct mg_connection *conn, 
											struct websrv_rest_session *pSession, 
											int format )
{
	char buf[2048];
	char wrkbuf[256];

	if ( NULL != pSession ) {

		char sid[32 + 1 ];
		memset( sid, 0, sizeof( sid ) );
		memcpy( sid, pSession->m_sid, sizeof( sid ) );

		// We should close the session
		pSession->m_pClientItem->m_bOpen = false;
		pSession->lastActiveTime = 0;
		websrv_expire_rest_sessions( conn );

		if ( REST_FORMAT_PLAIN == format ) {
			webserv_util_sendheader( conn, 200, REST_MIME_TYPE_PLAIN );

			mg_write( conn, "\r\n", 2 );		// head/body Separator
				
			memset( buf, 0, sizeof( buf ));
			webserv_util_make_chunk( buf, REST_PLAIN_ERROR_SUCCESS, strlen( REST_PLAIN_ERROR_SUCCESS ) );
			mg_write( conn, buf, strlen( buf ) );

			memset( buf, 0, sizeof( buf ));
#ifdef WIN32
			int n = _snprintf( wrkbuf, sizeof(wrkbuf), REST_PLAIN_ERROR_SUCCESS );
#else
			int n = snprintf( wrkbuf, sizeof(wrkbuf), REST_PLAIN_ERROR_SUCCESS );
#endif
			webserv_util_make_chunk( buf, wrkbuf, n );
			mg_write( conn, buf, strlen( buf ) );

			mg_write( conn, "0\r\n\r\n", 5);	// Terminator
			return MG_TRUE;
		}
		else if ( REST_FORMAT_CSV == format ) {
			webserv_util_sendheader( conn, 200, REST_MIME_TYPE_CSV );

			mg_write( conn, "\r\n", 2 );		// head/body Separator

			memset( buf, 0, sizeof( buf ));
#ifdef WIN32
			int n = _snprintf( wrkbuf, sizeof(wrkbuf), REST_CSV_ERROR_SUCCESS );
#else
			int n = snprintf( wrkbuf, sizeof(wrkbuf), REST_CSV_ERROR_SUCCESS );
#endif
			webserv_util_make_chunk( buf, wrkbuf, n );
			mg_write( conn, buf, strlen( buf ) );

			mg_write( conn, "0\r\n\r\n", 5);	// Terminator
			return MG_TRUE;
		}
		else if ( REST_FORMAT_XML == format ) {
			webserv_util_sendheader( conn, 200, REST_MIME_TYPE_XML );

			mg_write( conn, "\r\n", 2 );		// head/body Separator
				
			memset( buf, 0, sizeof( buf ));
#ifdef WIN32
			int n = _snprintf( wrkbuf, sizeof(wrkbuf), REST_XML_ERROR_SUCCESS );
#else
			int n = snprintf( wrkbuf, sizeof(wrkbuf), REST_XML_ERROR_SUCCESS );
#endif
			webserv_util_make_chunk( buf, wrkbuf, n );
			mg_write( conn, buf, strlen( buf ) );

			mg_write( conn, "0\r\n\r\n", 5);	// Terminator
			return MG_TRUE;
		}
		else if ( REST_FORMAT_JSON == format ) {
			webserv_util_sendheader( conn, 200, REST_MIME_TYPE_JSON );

			mg_write( conn, "\r\n", 2 );		// head/body Separator
				
			memset( buf, 0, sizeof( buf ));
#ifdef WIN32
			int n = _snprintf( wrkbuf, sizeof(wrkbuf), REST_JSON_ERROR_SUCCESS );
#else
			int n = snprintf( wrkbuf, sizeof(wrkbuf), REST_JSON_ERROR_SUCCESS );
#endif
			webserv_util_make_chunk( buf, wrkbuf, n );
			mg_write( conn, buf, strlen( buf ) );
			
			mg_write( conn, "0\r\n\r\n", 5);	// Terminator
			return MG_TRUE;
		}
		else if ( REST_FORMAT_JSONP == format ) {
			webserv_util_sendheader( conn, 200, REST_MIME_TYPE_JSONP );

			mg_write( conn, "\r\n", 2 );		// head/body Separator
				
			memset( buf, 0, sizeof( buf ));
#ifdef WIN32
			int n = _snprintf( wrkbuf, sizeof(wrkbuf), REST_JSONP_ERROR_SUCCESS );
#else
			int n = snprintf( wrkbuf, sizeof(wrkbuf), REST_JSONP_ERROR_SUCCESS );
#endif
			webserv_util_make_chunk( buf, wrkbuf, n );
			mg_write( conn, buf, strlen( buf ) );
			
			mg_write( conn, "0\r\n\r\n", 5);	// Terminator
			return MG_TRUE;
		}
		else {
			webserv_util_sendheader( conn, 400, REST_MIME_TYPE_PLAIN );

			mg_write( conn, "\r\n", 2 );		// head/body Separator
			memset( buf, 0, sizeof( buf ) );
			webserv_util_make_chunk( buf, REST_PLAIN_ERROR_UNSUPPORTED_FORMAT, strlen( REST_PLAIN_ERROR_UNSUPPORTED_FORMAT ) );
			mg_write( conn, buf, strlen( buf ) );

			mg_write( conn, "0\r\n\r\n", 5);	// Terminator

			return MG_TRUE;
		}

	}
	else {	// session not found
		webserv_rest_error( conn, pSession, format, REST_ERROR_CODE_INVALID_SESSION );
	}

	return MG_TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// websrv_mainpage
//

int
CControlObject::websrv_mainpage( struct mg_connection *conn )
{
	wxString strHost;

	// Check pointer
	if (NULL == conn) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;
	
	// Get hostname
	const char *p = mg_get_header( conn, "Host" ); // conn->local_ip; //_("http://localhost:8080");
    strHost.FromAscii( p );

    wxString buildPage;
	mg_send_status( conn, 200 );
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Control"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code

    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
	// Insert server url into navigation menu   
    buildPage += _(WEB_COMMON_MENU);
        
    buildPage += _("<span align=\"center\">");
    buildPage += _("<h4> Welcome to the VSCP daemon control interface.</h4>");
    buildPage += _("</span>");
	buildPage += _("<span style=\"text-indent:50px;\"><p>");
	buildPage += _("<img src=\"http://vscp.org/images/vscp_logo.jpg\" width=\"100\">");
	buildPage += _("</p></span>");
    buildPage += _("<span style=\"text-indent:50px;\"><p>");
    buildPage += _(" <b>Version:</b> ");
    buildPage += _(VSCPD_DISPLAY_VERSION);
    buildPage += _("</p><p>");
    buildPage += _(VSCPD_COPYRIGHT_HTML);
    buildPage += _("</p></span>");
            
    buildPage += _(WEB_COMMON_END);     // Common end code
    
	// Server data
	mg_send_data( conn, buildPage.ToAscii(), buildPage.Length() );

	return MG_TRUE;	
}


///////////////////////////////////////////////////////////////////////////////
// websrv_interfaces
//

int 
CControlObject::websrv_interfaces( struct mg_connection *conn )
{
    // Check pointer
	if (NULL == conn) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

    wxString buildPage;
	mg_send_status( conn, 200 );
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Control"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code

    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
	// Insert server url into navigation menu   
    buildPage += _(WEB_COMMON_MENU);
    
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
	buildPage += _("5 - Web Server Client.<br>");
	buildPage += _("6 - WebSocket Client.<br>");

    buildPage += _(WEB_COMMON_END);     // Common end code
    
    char *ppage = new char[ buildPage.Length() + 1 ];
    memset(ppage, 0, buildPage.Length() + 1 );
    memcpy( ppage, buildPage.ToAscii(), buildPage.Length() );        
    
	buildPage += _(WEB_COMMON_END);     // Common end code
    
	// Server data
	mg_send_data( conn, buildPage.ToAscii(), buildPage.Length() );

	return MG_TRUE;	
}



///////////////////////////////////////////////////////////////////////////////
// websrv_dmlist
//

int
CControlObject::websrv_dmlist( struct mg_connection *conn )
{
	char buf[80];
    VSCPInformation vscpinfo;
    long upperLimit = 50;

	// Check pointer
	if (NULL == conn) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;
        
    // light
    bool bLight = false;
	if ( mg_get_var( conn, "light", buf, sizeof( buf ) ) <= 0 ) { 
		if ( strlen( buf ) && (NULL != strstr( "true", buf ) ) ) bLight = true;
	}
    
    // From
    long nFrom = 0;
	if ( mg_get_var( conn, "from", buf, sizeof( buf ) ) > 0 ) { 
		nFrom = atoi( buf );
	}
  
    // Check limits
    if (nFrom > pObject->m_dm.getRowCount()) nFrom = 0;
    
    // Count
    uint16_t nCount = 50;
	if ( mg_get_var( conn, "count", buf, sizeof( buf ) ) > 0 ) { 
		nCount = atoi( buf );
	}
    
    // Check limits
    if ((nFrom + nCount) > pObject->m_dm.getRowCount()) {
        upperLimit = pObject->m_dm.getRowCount()-nFrom;
    }
    else {
        upperLimit = nFrom+nCount;
    }
    
    // Navigation button
	if ( mg_get_var( conn, "navbtn", buf, sizeof( buf ) ) > 0 ) { 
	
		if (NULL != strstr("previous", buf) ) {
        
			if ( 0 != nFrom ) {    
            
				nFrom -= nCount;
				upperLimit = nFrom + nCount;
            
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
		else if (NULL != strstr("next",buf)) {

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
		else if (NULL != strstr("last",buf)) {
			
			nFrom = pObject->m_dm.getRowCount() - nCount;
			if ( nFrom < 0 ) {
				nFrom = 0;
				upperLimit = pObject->m_dm.getRowCount();
			}
			else {
				upperLimit = pObject->m_dm.getRowCount();
			}

		}
		else if ( NULL != strstr("first",buf) ) {

			nFrom = 0;
			if ((nFrom+nCount) > pObject->m_dm.getRowCount()) {
				upperLimit = pObject->m_dm.getRowCount()-nFrom;
			}
			else {
				upperLimit = nFrom+nCount;
			}
		}

	}
	else {  // No vaid navigation value

		//nFrom = 0;
        if ( (nFrom+nCount) > pObject->m_dm.getRowCount() ) {
            upperLimit = pObject->m_dm.getRowCount()-nFrom;
        }
        else {
            upperLimit = nFrom + nCount;
        }
		
    }    

    wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Decision Matrix"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    buildPage += _(WEB_COMMON_MENU);;  
    buildPage += _(WEB_DMLIST_BODY_START);
    
    {
		//wxString wxstrurl = wxString::Format( _("%s/vscp/dm"), conn->local_ip );
        wxString wxstrlight = ((bLight) ? _("true") : _("false"));
        buildPage += wxString::Format( _(WEB_COMMON_LIST_NAVIGATION),
                "/vscp/dm", //wxstrurl.GetData(),
                nFrom,
                ((nFrom + nCount) < pObject->m_dm.getRowCount()) ? 
                    nFrom + nCount - 1 : pObject->m_dm.getRowCount() - 1,
                pObject->m_dm.getRowCount(),
                nCount,
                nFrom,
#if wxMAJOR_VERSION > 3 
                wxstrlight );
#else           
				wxstrlight.c_str() );
                //wxstrlight.GetWriteBuf( wxstrlight.Length() ) );
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
                    wxString::Format(_("/vscp/dmedit?id=%d"),
										//conn->local_ip,
                                        i );
            wxString str = wxString::Format(_(WEB_COMMON_TR_CLICKABLE_ROW),
                                                url_dmedit.GetData() );
            buildPage += str;
        }

        // Client id    
        buildPage += _(WEB_IFLIST_TD_CENTERED);
        buildPage += wxString::Format(_("<form name=\"input\" action=\"/vscp/dmdelete?id=%d\" method=\"get\">%d<input type=\"submit\" value=\"x\"><input type=\"hidden\" name=\"id\"value=\"%d\"></form>"), 
										i, i, i );
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
                vscp_writeGuidArrayToString(pElement->m_vscpfilter.filter_GUID, strGUID);
                buildPage += strGUID;

                buildPage += _("<br>");

                buildPage += _("<b>Mask_priority: </b>");
                buildPage += wxString::Format(_("%d "), pElement->m_vscpfilter.mask_priority);

                buildPage += _("<b>Mask_class: </b>");
                buildPage += wxString::Format(_("%d "), pElement->m_vscpfilter.mask_class);

                buildPage += _("<b>Mask_type: </b>");
                buildPage += wxString::Format(_("%d "), pElement->m_vscpfilter.mask_type);

                buildPage += _("<b>Mask_GUID: </b>");
                vscp_writeGuidArrayToString(pElement->m_vscpfilter.mask_GUID, strGUID);
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
        //wxString wxstrurl = _("/vscp/dm");
        wxString wxstrlight = ((bLight) ? _("true") : _("false"));
        buildPage += wxString::Format( _(WEB_COMMON_LIST_NAVIGATION),
				"/vscp/dm", // wxstrurl,
                nFrom,
                ((nFrom + nCount) < pObject->m_dm.getRowCount()) ? 
                    nFrom + nCount - 1 : pObject->m_dm.getRowCount() - 1,
                pObject->m_dm.getRowCount(),
                nCount,
                nFrom,
#if (wxMAJOR_VERSION > 3)
                wxstrlight );
#else             
				wxstrlight.c_str() );
                //wxstrlight.GetWriteBuf( wxstrlight.Length() ) );
#endif        
    }
     
    buildPage += _(WEB_COMMON_END);     // Common end code
    
	// Server data
	mg_send_data( conn, buildPage.ToAscii(), buildPage.Length() );

	return MG_TRUE;	
}

///////////////////////////////////////////////////////////////////////////////
// websrv_dmedit
//

int 
CControlObject::websrv_dmedit( struct mg_connection *conn )
{
	char buf[80];
    wxString str;
    VSCPInformation vscpinfo;
    dmElement *pElement = NULL;

	// Check pointer
	if (NULL == conn) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

    // id
    long id = -1;
	if ( mg_get_var( conn, "id", buf, sizeof( buf ) ) > 0 ) {
		id = atoi(buf);
	}
    
    // Flag for new DM row
    bool bNew = false;
	if ( mg_get_var( conn, "new", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bNew = true;
	}
    
    wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Decision Matrix Edit"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    buildPage += _(WEB_COMMON_MENU);;
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
        buildPage += _("/vscp/dmpost");
        buildPage += _("\" name=\"dmedit\">");
        
        buildPage += wxString::Format(_("<input name=\"id\" value=\"%d\" type=\"hidden\"></input>"), id );
        
        if (bNew) {
            buildPage += _("<input name=\"new\" value=\"true\" type=\"hidden\"></input>");
        }
        else {
            buildPage += _("<input name=\"new\" value=\"false\" type=\"hidden\"></input>");
        }
        
        buildPage += _("<h4>Group id:</h4>");
        buildPage += _("<textarea cols=\"20\" rows=\"1\" name=\"groupid\">");
        if ( !bNew ) buildPage += pElement->m_strGroupID;
        buildPage += _("</textarea><br>");
        
        
        buildPage += _("<h4>Event:</h4> <span id=\"optiontext\">(leave items blank for don't care)</span><br>");

        buildPage += _("<table class=\"invisable\"><tbody><tr class=\"invisable\">");

        buildPage += _("<td class=\"invisable\">Priority:</td><td class=\"tbalign\">");

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
        buildPage += _("</td><td><textarea cols=\"5\" rows=\"1\" name=\"mask_priority\">");
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
        
        buildPage += _("</td><td> <textarea cols=\"10\" rows=\"1\" name=\"mask_vscpclass\">");
        if ( bNew ) {
            buildPage += _("0xFFFF");
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
        
        buildPage += _("</td><td> <textarea cols=\"10\" rows=\"1\" name=\"mask_vscptype\">");
        if ( bNew ) {
            buildPage += _("0xFFFF");;
        }
        else {
            buildPage += wxString::Format(_("0x%04x"), pElement->m_vscpfilter.mask_type);
        }
        buildPage += _("</textarea>");
        
        buildPage += _("</td></tr>"); 
        
        // GUID
        if ( !bNew ) vscp_writeGuidArrayToString( pElement->m_vscpfilter.filter_GUID, str );
        buildPage += _("<tr class=\"invisable\"><td class=\"invisable\">GUID:</td><td class=\"invisable\"><textarea cols=\"50\" rows=\"1\" name=\"filter_vscpguid\">");
        if ( bNew ) {
            buildPage += _("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00");
        }
        else {
            buildPage += wxString::Format(_("%s"), str.GetData() );
        }
        buildPage += _("</textarea></td>");
        
        if ( !bNew ) vscp_writeGuidArrayToString( pElement->m_vscpfilter.mask_GUID, str );
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
        buildPage += _("<span id=\"optiontext\">Enable row</span>&nbsp;&nbsp;"); 

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
        buildPage += _("<span id=\"optiontext\">End scan on this row</span>&nbsp;&nbsp;");

        //buildPage += _("<br>");

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
        buildPage += _("<span id=\"optiontext\">Check Index</span>&nbsp;&nbsp;");

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
        buildPage += _("<span id=\"optiontext\">Check Zone</span>&nbsp;&nbsp;"); 

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
        buildPage += _("<span id=\"optiontext\">Check Subzone</span>&nbsp;&nbsp;");
        buildPage += _("<br><br><br>");
        
        buildPage += _("<h4>Allowed From:</h4>");
		buildPage += _("<i>Enter * for beginning of time.</i><br>");
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
		buildPage += _("<i>Enter * for end of time (always).</i><br>");
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
		buildPage += _("<i>Enter * for always.</i><br>");
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
    
    buildPage += _(WEB_DMEDIT_SUBMIT);  
    buildPage += _("</form>");
    buildPage += _(WEB_COMMON_END);     // Common end code
    
	// Server data
	mg_send_data( conn, buildPage.ToAscii(), buildPage.Length() );

	return MG_TRUE;	
}


///////////////////////////////////////////////////////////////////////////////
// websrv_dmpost
//

int 
CControlObject::websrv_dmpost( struct mg_connection *conn )
{
	char buf[80];
    wxString str;
    VSCPInformation vscpinfo;
    dmElement *pElement = NULL;

	// Check pointer
	if (NULL == conn) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;
    
    // id
    long id = -1;
	if ( mg_get_var( conn, "id", buf, sizeof( buf ) ) > 0 ) {
		id = atoi( buf );
	}
    
    // Flag for new DM row
    bool bNew = false;
	if ( mg_get_var( conn, "new", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bNew = true;
	}

    wxString strGroupID;
	if ( mg_get_var( conn, "groupid", buf, sizeof( buf ) ) > 0 ) {
		strGroupID = wxString::FromAscii(buf);
	}
        
    int filter_priority = -1;
	if ( mg_get_var( conn, "filter_priority", buf, sizeof( buf ) ) > 0 ) {
		filter_priority = vscp_readStringValue( wxString::FromAscii( buf ) );
	}
         
    int mask_priority = 0;    
	if ( mg_get_var( conn, "mask_priority", buf, sizeof( buf ) ) > 0 ) {
		mask_priority = vscp_readStringValue( wxString::FromAscii( buf ) );
	}
    
    uint16_t filter_vscpclass = -1;
	if ( mg_get_var( conn, "filter_vscpclass", buf, sizeof( buf ) ) > 0 ) {
		wxString wrkstr = wxString::FromAscii( buf );
		filter_vscpclass = vscp_readStringValue( wrkstr );
	}
    
    uint16_t mask_vscpclass = 0;
	if ( mg_get_var( conn, "mask_vscpclass", buf, sizeof( buf ) ) > 0 ) {
		mask_vscpclass = vscp_readStringValue( wxString::FromAscii( buf ) );
	}
    
    uint16_t filter_vscptype = 0;
	if ( mg_get_var( conn, "filter_vscptype", buf, sizeof( buf ) ) > 0 ) {
		filter_vscptype = vscp_readStringValue( wxString::FromAscii( buf ) );
	}
    
    uint16_t mask_vscptype = 0;
	if ( mg_get_var( conn, "mask_vscptype", buf, sizeof( buf ) ) > 0 ) {
		mask_vscptype = vscp_readStringValue( wxString::FromAscii( buf ) );
	}
    
    wxString strFilterGuid;
	if ( mg_get_var( conn, "filter_vscpguid", buf, sizeof( buf ) ) > 0 ) {
		strFilterGuid = wxString::FromAscii( buf );
		strFilterGuid = strFilterGuid.Trim();
		strFilterGuid = strFilterGuid.Trim(false);
	}
    
    wxString strMaskGuid;
	if ( mg_get_var( conn, "mask_vscpguid", buf, sizeof( buf ) ) > 0 ) {
		strMaskGuid = wxString::FromAscii( buf );
		strMaskGuid = strMaskGuid.Trim();
		strMaskGuid = strMaskGuid.Trim(false);
	}
    
    uint8_t index = 0;
	if ( mg_get_var( conn, "vscpindex", buf, sizeof( buf ) ) > 0 ) {
		index = vscp_readStringValue( wxString::FromAscii( buf ) );
	}
    
    uint8_t zone = 0;
	if ( mg_get_var( conn, "vscpzone", buf, sizeof( buf ) ) > 0 ) {
		zone = vscp_readStringValue( wxString::FromAscii( buf ) );
	}
    
    uint8_t subzone = 0;
	if ( mg_get_var( conn, "vscpsubzone", buf, sizeof( buf ) ) > 0 ) {
		subzone = vscp_readStringValue( wxString::FromAscii( buf ) );
	}
    
    bool bEnableRow = false;
	if ( mg_get_var( conn, "check_enablerow", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bEnableRow = true;
	}
        
    bool bEndScan = false;
	if ( mg_get_var( conn, "check_endscan", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bEndScan = true;
	}
    
    bool bCheckIndex = false;
	if ( mg_get_var( conn, "check_index", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bCheckIndex = true;
	}
    
    bool bCheckZone = false;
	if ( mg_get_var( conn, "check_zone", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bCheckZone = true;
	}
    
    bool bCheckSubZone = false;
	if ( mg_get_var( conn, "check_subzone", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bCheckSubZone = true;
	}
    
    wxString strAllowedFrom;
	if ( mg_get_var( conn, "allowedfrom", buf, sizeof( buf ) ) > 0 ) {
		strAllowedFrom = wxString::FromAscii( buf );	
		strAllowedFrom.Trim( true );
		strAllowedFrom.Trim( false );
		if ( _("*") == strAllowedFrom ) {
			strAllowedFrom = _("0000-01-01 00:00:00");
		}
	}
    
    wxString strAllowedTo;
	if ( mg_get_var( conn, "allowedto", buf, sizeof( buf ) ) > 0 ) {
		strAllowedTo = wxString::FromAscii( buf );	
		strAllowedTo.Trim( true );
		strAllowedTo.Trim( false );
		if ( _("*") == strAllowedTo ) {
			strAllowedTo = _("9999-12-31 23:59:59");
		}
	}
    
    wxString strAllowedTime;
	if ( mg_get_var( conn, "allowedtime", buf, sizeof( buf ) ) > 0 ) {
		strAllowedTime = wxString::FromAscii( buf );  	
		strAllowedTime.Trim( true );
		strAllowedTime.Trim( false );
		if ( _("*") == strAllowedTime ) {
			strAllowedTime = _("* *");
		}
	}
    
    bool bCheckMonday = false;
	if ( mg_get_var( conn, "monday", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bCheckMonday = true;
	}
    
    bool bCheckTuesday = false;
	if ( mg_get_var( conn, "tuesday", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bCheckTuesday = true;
	}
    
    bool bCheckWednesday = false;
	if ( mg_get_var( conn, "wednesday", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bCheckWednesday = true;
	}
    
    bool bCheckThursday = false;
	if ( mg_get_var( conn, "thursday", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bCheckThursday = true;
	}
    
    bool bCheckFriday = false;
	if ( mg_get_var( conn, "friday", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bCheckFriday = true;
	}
    
    bool bCheckSaturday = false;
	if ( mg_get_var( conn, "saturday", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bCheckSaturday = true;
	}
    
    bool bCheckSunday = false;
	if ( mg_get_var( conn, "sunday", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bCheckSunday = true;
	}
    
    uint32_t action = 0;
	if ( mg_get_var( conn, "action", buf, sizeof( buf ) ) > 0 ) {
		action = vscp_readStringValue( wxString::FromAscii( buf ) );
	}
    
    wxString strActionParameter;
	if ( mg_get_var( conn, "actionparameter", buf, sizeof( buf ) ) > 0 ) {
		strActionParameter = wxString::FromAscii( buf );
	}
    
    wxString strComment;
	if ( mg_get_var( conn, "comment", buf, sizeof( buf ) ) > 0 ) {
		strComment = wxString::FromAscii( buf );
	}
    
    wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Decision Matrix Post"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += _("<meta http-equiv=\"refresh\" content=\"2;url=/vscp/dm");
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
                    vscp_getGuidFromStringToArray(pElement->m_vscpfilter.mask_GUID,
                            strMaskGuid);
                    vscp_getGuidFromStringToArray(pElement->m_vscpfilter.filter_GUID,
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

                pElement->m_timeAllow.m_fromTime.ParseDateTime( strAllowedFrom );
                pElement->m_timeAllow.m_endTime.ParseDateTime( strAllowedTo );
                pElement->m_timeAllow.parseActionTime( strAllowedTime );

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

                if ( bNew ) {
                    // add the DM row to the matrix
                    pObject->m_dm.addElement(pElement);
                }

                // Save decision matrix
                pObject->m_dm.save();

                buildPage += wxString::Format(_("<br><br>DM Entry has been saved. id=%d"), id);
            } 
			else {
                buildPage += wxString::Format(_("<br><br>Memory problem id=%d. Unable to save record"), id);
            }

        } else {
            buildPage += wxString::Format(_("<br><br>Record id=%d is to large. Unable to save record"), id);
        }
    } else {
        buildPage += wxString::Format(_("<br><br>Record id=%d is wrong. Unable to save record"), id);
    }

    buildPage += _(WEB_COMMON_END); // Common end code 

    // Server data
	mg_send_data( conn, buildPage.ToAscii(), buildPage.Length() );

	return MG_TRUE;	
}


///////////////////////////////////////////////////////////////////////////////
// websrv_dmdelete
//

int 
CControlObject::websrv_dmdelete( struct mg_connection *conn )
{
	char buf[80];
	wxString str;
    VSCPInformation vscpinfo;
    dmElement *pElement = NULL;

	// Check pointer
	if (NULL == conn) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;
   
    // id
    long id = -1;
	if ( mg_get_var( conn, "id", buf, sizeof( buf ) ) > 0 ) {
		id = atoi( buf );
	}
        
    wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Decision Matrix Delete"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += _("<meta http-equiv=\"refresh\" content=\"2;url=/vscp/dm");
    buildPage += wxString::Format(_("?from=%d"), id + 1 );
    buildPage += _("\">");
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    // Navigation menu 
    buildPage += _(WEB_COMMON_MENU);
    
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
    
    // Server data
	mg_send_data( conn, buildPage.ToAscii(), buildPage.Length() );
    
	return MG_TRUE;	
}


///////////////////////////////////////////////////////////////////////////////
// websrv_variables_list
//

int
CControlObject::websrv_variables_list( struct mg_connection *conn )
{
	char buf[80];
    VSCPInformation vscpinfo;
    unsigned long upperLimit = 50;

	// Check pointer
	if (NULL == conn) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;
              
    // From
    unsigned long nFrom = 0;
	if ( mg_get_var( conn, "from", buf, sizeof( buf ) ) > 0 ) {
		
		atoi( buf );

		// Check limits
		if (nFrom > pObject->m_VSCP_Variables.m_listVariable.GetCount()) nFrom = 0;
	}
    
    
    // Count
    uint16_t nCount = 50;
	if ( mg_get_var( conn, "count", buf, sizeof( buf ) ) > 0 ) {
		
		nCount = atoi( buf );
		
		// Check limits
		if ( (nFrom+nCount) > pObject->m_VSCP_Variables.m_listVariable.GetCount() ) {
			upperLimit = pObject->m_VSCP_Variables.m_listVariable.GetCount()-nFrom;
		}
		else {
			upperLimit = nFrom+nCount;
		}
	}
    
    // Navigation button
    if ( mg_get_var( conn, "navbtn", buf, sizeof( buf ) ) > 0 ) {
        //nFrom = 0;
        if ((nFrom+nCount) > pObject->m_VSCP_Variables.m_listVariable.GetCount()) {
            upperLimit = pObject->m_VSCP_Variables.m_listVariable.GetCount()-nFrom;
        }
        else {
            upperLimit = nFrom+nCount;
        }
    }
    else if (NULL != strstr("first", buf )) {
        nFrom = 0;
        if ((nFrom+nCount) > pObject->m_VSCP_Variables.m_listVariable.GetCount()) {
            upperLimit = pObject->m_VSCP_Variables.m_listVariable.GetCount()-nFrom;
        }
        else {
            upperLimit = nFrom+nCount;
        }
    }
    else if (NULL != strstr("previous", buf ) ) {
        
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
    else if (NULL != strstr("next", buf )) {

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
    else if (NULL != strstr("last", buf )) {
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
    
    // Navigation menu 
    buildPage += _(WEB_COMMON_MENU);
    
    buildPage += _(WEB_VARLIST_BODY_START);
    
    {
        wxString wxstrurl = _("/vscp/variables");
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
    
    for ( unsigned int i=nFrom;i<upperLimit;i++) {
        
        CVSCPVariable *pVariable = 
                pObject->m_VSCP_Variables.m_listVariable.Item( i )->GetData();
        
        {
            wxString url_dmedit = 
                    wxString::Format(_("/vscp/varedit?id=%d"),
                                        i );
            wxString str = wxString::Format(_(WEB_COMMON_TR_CLICKABLE_ROW),
                                                url_dmedit.GetData() );
            buildPage += str;
        }

        // Client id    
        buildPage += _(WEB_IFLIST_TD_CENTERED);
        buildPage += wxString::Format(_("<form name=\"input\" action=\"/vscp/vardelete?id=%d\" method=\"get\">%d<input type=\"submit\" value=\"x\"><input type=\"hidden\" name=\"id\"value=\"%d\"></form>"), 
                        i, i, i );
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
        wxString wxstrurl = _("/vscp/variables");
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

	// Server data
	mg_send_data( conn, buildPage.ToAscii(), buildPage.Length() );
    

	return MG_TRUE;	
}

///////////////////////////////////////////////////////////////////////////////
// websrv_variables_edit
//

int
CControlObject::websrv_variables_edit( struct mg_connection *conn )
{
	char buf[80];
    wxString str;
    VSCPInformation vscpinfo;
    CVSCPVariable *pVariable = NULL;

	// Check pointer
	if (NULL == conn) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;
        
    // id
    long id = -1;
	if ( mg_get_var( conn, "id", buf, sizeof( buf ) ) > 0 ) {
		id = atoi( buf );
	}
    
    // type
    uint8_t nType = VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED;
	if ( mg_get_var( conn, "type", buf, sizeof( buf ) ) > 0 ) {
		nType = atoi( buf );
	}
    
    // Flag for new variable row
    bool bNew = false;
	if ( mg_get_var( conn, "new", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bNew = true;
	}
    
    wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Variable Edit"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    // Navigation menu 
    buildPage += _(WEB_COMMON_MENU);
    
    buildPage += _(WEB_VAREDIT_BODY_START);

    if ( !bNew && ( id < (long)pObject->m_VSCP_Variables.m_listVariable.GetCount() ) ) {
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
                vscp_writeGuidArrayToString( pVariable->m_event.GUID, strGUID );
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
                vscp_writeVscpDataToString( &pVariable->m_event, strData );
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
                vscp_writeVscpDataToString( &pVariable->m_event, strData );
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

    } 
    else {
        buildPage += _("<br><b>Error: Non existent id</b>");
    }
    
    
    wxString wxstrurl = _("/vscp/varpost");
    buildPage += wxString::Format( _(WEB_VAREDIT_SUBMIT),
                                    wxstrurl.GetData() );
    
    buildPage += _("</form>");
    buildPage += _(WEB_COMMON_END);     // Common end code
    
    char *ppage = new char[ buildPage.Length() + 1 ];
    memset(ppage, 0, buildPage.Length() + 1 );
    memcpy( ppage, buildPage.ToAscii(), buildPage.Length() );        

	// Server data
	mg_send_data( conn, buildPage.ToAscii(), buildPage.Length() );

	return MG_TRUE;	
}


///////////////////////////////////////////////////////////////////////////////
// websrv_variables_post
//

int
CControlObject::websrv_variables_post( struct mg_connection *conn )
{
	char buf[80];
    wxString str;
    VSCPInformation vscpinfo;
    CVSCPVariable *pVariable = NULL;
    
    // Check pointer
	if (NULL == conn) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

    // id
    long id = -1;
	if ( mg_get_var( conn, "id", buf, sizeof( buf ) ) > 0 ) {
		id = atoi( buf );
	}
    
    uint8_t nType = VSCP_DAEMON_VARIABLE_CODE_UNASSIGNED;
	if ( mg_get_var( conn, "type", buf, sizeof( buf ) ) > 0 ) {
		nType = atoi( buf );
	}
    
    wxString strName;
	if ( mg_get_var( conn, "value_name", buf, sizeof( buf ) ) > 0 ) {
		strName = wxString::FromAscii( buf );
	}
    
    // Flag for new variable row
    bool bNew = false;
	if ( mg_get_var( conn, "new", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bNew = true;
	}
    
    // Flag for persistence
    bool bPersistent = true;
	if ( mg_get_var( conn, "persistent", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "false", buf ) ) bPersistent = false;
	}

    wxString strNote;
	if ( mg_get_var( conn, "note", buf, sizeof( buf ) ) > 0 ) {
		strNote = wxString::FromAscii( buf );
	}
    
    wxString strValueString;
	if ( mg_get_var( conn, "value_string", buf, sizeof( buf ) ) > 0 ) {
		strValueString = wxString::FromAscii( buf );
	}
               
    bool bValueBoolean = false;
	if ( mg_get_var( conn, "value_boolean", buf, sizeof( buf ) ) > 0 ) {
		if ( NULL != strstr( "true", buf ) ) bValueBoolean = true;
	}
    
    int value_integer = 0;
	if ( mg_get_var( conn, "value_integer", buf, sizeof( buf ) ) > 0 ) {
		str = wxString::FromAscii( buf );
		value_integer = vscp_readStringValue( str );
	}
       
    long value_long = 0;
	if ( mg_get_var( conn, "value_long", buf, sizeof( buf ) ) > 0 ) {
		str = wxString::FromAscii( buf );
		value_long = vscp_readStringValue( str );
	}
    
    double value_double = 0;
	if ( mg_get_var( conn, "value_double", buf, sizeof( buf ) ) > 0 ) {
		value_double = atof( buf );
	}
    
    wxString strMeasurement;
	if ( mg_get_var( conn, "value_measurement", buf, sizeof( buf ) ) > 0 ) {
		strMeasurement = wxString::FromAscii( buf );
	}
    
    uint16_t value_class = 0;
	if ( mg_get_var( conn, "value_class", buf, sizeof( buf ) ) > 0 ) {
		value_class = vscp_readStringValue( wxString::FromAscii( buf ) );
	}
    
    uint16_t value_type = 0;
	if ( mg_get_var( conn, "value_type", buf, sizeof( buf ) ) > 0 ) {
		value_type = vscp_readStringValue( wxString::FromAscii( buf ) );
	}
    
    wxString strGUID;
	if ( mg_get_var( conn, "value_guid", buf, sizeof( buf ) ) > 0 ) {
		strGUID = wxString::FromAscii( buf );
	}
    
    uint32_t value_timestamp = 0;
	if ( mg_get_var( conn, "value_timestamp", buf, sizeof( buf ) ) > 0 ) {
		value_timestamp = vscp_readStringValue( wxString::FromAscii( buf ) );
	}
    
    uint32_t value_obid = 0;
	if ( mg_get_var( conn, "value_obid", buf, sizeof( buf ) ) > 0 ) {
		value_obid = vscp_readStringValue( wxString::FromAscii( buf ) );
	}
    
    uint8_t value_head = 0;
	if ( mg_get_var( conn, "value_head", buf, sizeof( buf ) ) > 0 ) {
		value_head = vscp_readStringValue( wxString::FromAscii( buf ) );
	}
    
    uint32_t value_crc = 0;
	if ( mg_get_var( conn, "value_crc", buf, sizeof( buf ) ) > 0 ) {
		value_crc = vscp_readStringValue( wxString::FromAscii( buf ) );
	}
    
    uint16_t value_sizedata = 0;
	if ( mg_get_var( conn, "value_sizedata", buf, sizeof( buf ) ) > 0 ) {
		value_sizedata = vscp_readStringValue( wxString::FromAscii( buf ) );
	}
    
    wxString strData;
	if ( mg_get_var( conn, "value_data", buf, sizeof( buf ) ) > 0 ) {
		strData = wxString::FromAscii( buf );
	}
    
    wxString strDateTime;
	if ( mg_get_var( conn, "value_date_time", buf, sizeof( buf ) ) > 0 ) {
		strDateTime = wxString::FromAscii( buf );
	}
    
    wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Variable Post"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += _("<meta http-equiv=\"refresh\" content=\"2;url=/vscp/variables");
    buildPage += wxString::Format(_("?from=%d"), id );
    buildPage += _("\">");
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    // Navigation menu 
    buildPage += _(WEB_COMMON_MENU);
    
    buildPage += _(WEB_VARPOST_BODY_START);
        
    if (bNew) {
        pVariable = new CVSCPVariable;
    }

    if (bNew || (id >= 0)) {

        if (bNew || 
                ((0 == id) && !bNew) || 
                (id < (long)pObject->m_VSCP_Variables.m_listVariable.GetCount()) ) {

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
                    vscp_getVscpDataArrayFromString( pVariable->m_normInteger, 
                                                    &size,
                                                    strMeasurement );
                    pVariable->m_normIntSize = size;
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT:
                    pVariable->m_event.vscp_class = value_class;
                    pVariable->m_event.vscp_type = value_type;
                    vscp_getGuidFromStringToArray( pVariable->m_event.GUID, strGUID );
                    vscp_getVscpDataFromString( &pVariable->m_event,
                                            strData );
                    pVariable->m_event.crc = value_crc;
                    pVariable->m_event.head = value_head;
                    pVariable->m_event.obid = value_obid;
                    pVariable->m_event.timestamp = value_timestamp;        
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_GUID:
                    vscp_getGuidFromStringToArray( pVariable->m_event.GUID, strGUID );
                    break;

                case VSCP_DAEMON_VARIABLE_CODE_VSCP_EVENT_DATA:
                    vscp_getVscpDataFromString( &pVariable->m_event,
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
    
	// Server data
	mg_send_data( conn, buildPage.ToAscii(), buildPage.Length() );

	return MG_TRUE;	
}


///////////////////////////////////////////////////////////////////////////////
// websrv_variables_new
//

int
CControlObject::websrv_variables_new( struct mg_connection *conn )
{
    wxString str;
    VSCPInformation vscpinfo;
    CVSCPVariable *pVariable = NULL;

	// Check pointer
	if (NULL == conn) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;
         
    wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - New variable"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    // Navigation menu 
    buildPage += _(WEB_COMMON_MENU);
    
    buildPage += _(WEB_VAREDIT_BODY_START);

    buildPage += _("<br><div style=\"text-align:center\">");
    
    buildPage += _("<br><form method=\"get\" action=\"");
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

	// Server data
	mg_send_data( conn, buildPage.ToAscii(), buildPage.Length() );
     
	return MG_TRUE;	
}


///////////////////////////////////////////////////////////////////////////////
// websrv_variables_delete
//

int
CControlObject::websrv_variables_delete( struct mg_connection *conn )
{
	char buf[80];
    wxString str;
    VSCPInformation vscpinfo;
    CVSCPVariable *pVariable = NULL;

	// Check pointer
	if (NULL == conn) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

    // id
    long id = -1;
	if ( mg_get_var( conn, "id", buf, sizeof( buf ) ) > 0 ) {
		id = atoi( buf );
	}
        
    wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Variable Delete"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
    buildPage += _("<meta http-equiv=\"refresh\" content=\"2;url=/vscp/variables");
    buildPage += wxString::Format(_("?from=%d"), id + 1 );
    buildPage += _("\">");
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    // navigation menu 
    buildPage += _(WEB_COMMON_MENU);
    
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
    
	// Server data
	mg_send_data( conn, buildPage.ToAscii(), buildPage.Length() );

	return MG_TRUE;	
}


///////////////////////////////////////////////////////////////////////////////
// websrv_discovery
//

int
CControlObject::websrv_discovery( struct mg_connection *conn )
{
	//char buf[80];
    wxString str;
    VSCPInformation vscpinfo;
    CVSCPVariable *pVariable = NULL;

	// Check pointer
	if (NULL == conn) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

	wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Device discovery"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
	 buildPage += _("<meta http-equiv=\"refresh\" content=\"5;url=/vscp");
    buildPage += _("\">");
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    // navigation menu 
    buildPage += _(WEB_COMMON_MENU);
	buildPage += _("<b>Device discovery functionality is not yet implemented!</b>");
    
	// Server data
	mg_send_data( conn, buildPage.ToAscii(), buildPage.Length() );

	return MG_TRUE;	
}


///////////////////////////////////////////////////////////////////////////////
// websrv_session
//

int
CControlObject::websrv_session( struct mg_connection *conn )
{
	//char buf[80];
    wxString str;
    VSCPInformation vscpinfo;
    CVSCPVariable *pVariable = NULL;

	// Check pointer
	if (NULL == conn) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

	wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Device discovery"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
	 buildPage += _("<meta http-equiv=\"refresh\" content=\"5;url=/vscp");
    buildPage += _("\">");
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    // navigation menu 
    buildPage += _(WEB_COMMON_MENU);
	buildPage += _("<b>Session functionality is not yet implemented!</b>");
    
	// Server data
	mg_send_data( conn, buildPage.ToAscii(), buildPage.Length() );

	return MG_TRUE;	
}


///////////////////////////////////////////////////////////////////////////////
// websrv_configure
//

int
CControlObject::websrv_configure( struct mg_connection *conn )
{
	//char buf[80];
    wxString str;
    VSCPInformation vscpinfo;
    CVSCPVariable *pVariable = NULL;

	// Check pointer
	if (NULL == conn) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

	wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Device discovery"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
	 buildPage += _("<meta http-equiv=\"refresh\" content=\"5;url=/vscp");
    buildPage += _("\">");
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    // navigation menu 
    buildPage += _(WEB_COMMON_MENU);
	buildPage += _("<b>Configuration functionality is not yet implemented!</b>");
    
	// Server data
	mg_send_data( conn, buildPage.ToAscii(), buildPage.Length() );

	return MG_TRUE;	
}



///////////////////////////////////////////////////////////////////////////////
// websrv_bootload
//

int
CControlObject::websrv_bootload( struct mg_connection *conn )
{
	//char buf[80];
    wxString str;
    VSCPInformation vscpinfo;
    CVSCPVariable *pVariable = NULL;

	// Check pointer
	if (NULL == conn) return MG_FALSE;

	CControlObject *pObject = (CControlObject *)conn->server_param;
	if (NULL == pObject) return MG_FALSE;

	wxString buildPage;
    buildPage = wxString::Format(_(WEB_COMMON_HEAD), _("VSCP - Device discovery"));
    buildPage += _(WEB_STYLE_START);
    buildPage += _(WEB_COMMON_CSS);     // CSS style Code
    buildPage += _(WEB_STYLE_END);
    buildPage += _(WEB_COMMON_JS);      // Common Javascript code
	 buildPage += _("<meta http-equiv=\"refresh\" content=\"5;url=/vscp");
    buildPage += _("\">");
    buildPage += _(WEB_COMMON_HEAD_END_BODY_START);
    
    // navigation menu 
    buildPage += _(WEB_COMMON_MENU);
	buildPage += _("<b>Bootload functionality is not yet implemented!</b>");
    
	// Server data
	mg_send_data( conn, buildPage.ToAscii(), buildPage.Length() );

	return MG_TRUE;	
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
            if (NULL != pvscpEvent) vscp_deleteVSCPevent(pvscpEvent);

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

