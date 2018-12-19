// vscptcpip.cpp: implementation of the CTcpipLink class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP Project (http://www.vscp.org) 
//
// Copyright (C) 2000-2018 Ake Hedman, 
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

#include <stdio.h>
#include "unistd.h"
#include "stdlib.h"
#include <string.h>
#include "limits.h"
#include "syslog.h"
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <signal.h>
#include <ctype.h>
#include <libgen.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <net/if.h>

#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"
#include <wx/sstream.h>
#include <wx/xml/xml.h>
#include <wx/listimpl.cpp>
#include <wx/thread.h>
#include <wx/tokenzr.h>
#include <wx/datetime.h>

#include <vscphelper.h>
#include <vscpremotetcpif.h>
#include <vscp_type.h>
#include <vscp_class.h>
#include "vscptcpiplink.h"


//////////////////////////////////////////////////////////////////////
// CTcpipLink
//

CTcpipLink::CTcpipLink()
{
    m_bQuit = false;
    m_pthreadSend = NULL;
    m_pthreadReceive = NULL;
    vscp_clearVSCPFilter(&m_rxfilter); // Accept all events
    vscp_clearVSCPFilter(&m_txfilter); // Send all events
    m_responseTimeout = TCPIP_DEFAULT_INNER_RESPONSE_TIMEOUT;
    ::wxInitialize();
}

//////////////////////////////////////////////////////////////////////
// ~CTcpipLink
//

CTcpipLink::~CTcpipLink()
{
    close();
    ::wxUninitialize();
}


//////////////////////////////////////////////////////////////////////
// open
//
//

bool
CTcpipLink::open( const char *pUsername,
                    const char *pPassword,
                    const char *pHost,
                    short port,
                    const char *pPrefix,
                    const char *pConfig)
{
    bool rv = true;
    wxString wxstr = wxString::FromAscii(pConfig);

    m_usernameLocal = wxString::FromAscii(pUsername);
    m_passwordLocal = wxString::FromAscii(pPassword);
    m_hostLocal = wxString::FromAscii(pHost);
    m_portLocal = port;
    m_prefix = wxString::FromAscii(pPrefix);

    // Parse the configuration string.
    wxStringTokenizer tkz(wxString::FromAscii(pConfig), _(";"));

    // Check for remote host in configuration string
    if (tkz.HasMoreTokens()) {
        // Get remote interface
        m_hostRemote = tkz.GetNextToken();
    }
    
    // Check for remote port in configuration string
    if (tkz.HasMoreTokens()) {
        // Get remote port
        m_portRemote = vscp_readStringValue(tkz.GetNextToken());
    }
    
    // Check for remote user in configuration string
    if (tkz.HasMoreTokens()) {
        // Get remote username
        m_usernameRemote = tkz.GetNextToken();
    }
    
    // Check for remote password in configuration string
    if (tkz.HasMoreTokens()) {
        // Get remote password
        m_passwordRemote = tkz.GetNextToken();
    }
    
    wxString strRxFilter;
    // Check for filter in configuration string
    if (tkz.HasMoreTokens()) {
        // Get filter
        strRxFilter = tkz.GetNextToken();
        vscp_readFilterFromString(&m_rxfilter, strRxFilter);
    }
    
    // Check for mask in configuration string
    wxString strRxMask;
    if (tkz.HasMoreTokens()) {
        // Get mask
        strRxMask = tkz.GetNextToken();
        vscp_readMaskFromString(&m_rxfilter, strRxMask);
    }

    wxString strTxFilter;
    // Check for filter in configuration string
    if (tkz.HasMoreTokens()) {
        // Get filter
        strTxFilter = tkz.GetNextToken();
        vscp_readFilterFromString(&m_txfilter, strTxFilter);
    }
    
    // Check for mask in configuration string
    wxString strTxMask;
    if (tkz.HasMoreTokens()) {
        // Get mask
        strTxMask = tkz.GetNextToken();
        vscp_readMaskFromString(&m_txfilter, strTxMask);
    }

    // Check for response timout in configuration string
    wxString strResponseTimout;
    if (tkz.HasMoreTokens()) {
        // Get response timout
        strResponseTimout = tkz.GetNextToken();
        m_responseTimeout = vscp_readStringValue( strResponseTimout );
    }
    
    // First log on to the host and get configuration 
    // variables

    if ( VSCP_ERROR_SUCCESS !=  
                    m_srvLocal.doCmdOpen( m_hostLocal.ToStdString(),
                                            port,
                                            m_usernameLocal.ToStdString(),
                                            m_passwordLocal.ToStdString() ) ) {
        syslog(LOG_ERR,
                "%s %s ",
                VSCP_TCPIPLINK_SYSLOG_DRIVER_ID,
                (const char *) "Unable to connect to local VSCP TCP/IP interface. Terminating!");
        return false;
    }

    // Find the channel id
    uint32_t ChannelID;
    m_srvLocal.doCmdGetChannelID( &ChannelID );

    // The server should hold configuration data 
    // 
    // We look for 
    //
    //   _host_remote       - The remote host to which we should connect
    //
    //   _port_remote       - The port to connect to at the remote host.
    //
    //   _user_remote       - Username to login at remote host
    //
    //   _password_remote   - Username to login at remote host
    //
    //   _filter (_rxfiter)- Standard VSCP filter in string form for receive-
    //             1,0x0000,0x0006,
    //                 ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00
    //              as priority,class,type,GUID
    //              Used to filter what events that is received from 
    //              the socketcan interface. If not give all events 
    //              are received.
    //
    //  _mask (_rxmask)- Standard VSCP mask in string form for receive.
    //              1,0x0000,0x0006,
    //                 ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00
    //              as priority,class,type,GUID
    //              Used to filter what events that is received from 
    //              the socketcan interface. If not give all events 
    //              are received. 
    //
    // _responsetimeout - The time in milliseconds we should wait for a response
    //                      from the remote server
    //
    // XML configuration
    // -----------------
    //
    // <setup host="localhost"
    //          port="9598"
    //          user="admin"
    //          password="secret"
    //          rxfilter=""
    //          rxmask="" 
    //          txfilter=""
    //          txmask=""
    //          responsetimeout="2000" />    
    //

    wxString str;
    wxString strName = m_prefix +
            wxString::FromAscii("_host_remote");
    m_srvLocal.getRemoteVariableValue(strName, m_hostRemote);
    
    strName = m_prefix +
            wxString::FromAscii("_port_remote");
    m_srvLocal.getRemoteVariableInt(strName, &m_portRemote);
    
    strName = m_prefix +
            wxString::FromAscii("_user_remote");
    m_srvLocal.getRemoteVariableValue(strName, m_usernameRemote);
    
    strName = m_prefix +
            wxString::FromAscii("_password_remote");
    m_srvLocal.getRemoteVariableValue(strName, m_passwordRemote);

    // Old format
    strName = m_prefix +
            wxString::FromAscii("_filter");
    if (VSCP_ERROR_SUCCESS == m_srvLocal.getRemoteVariableValue(strName, str)) {
        vscp_readFilterFromString(&m_rxfilter, str);
    }

    strName = m_prefix +
            wxString::FromAscii("_mask");
    if (VSCP_ERROR_SUCCESS == m_srvLocal.getRemoteVariableValue(strName, str)) {
        vscp_readMaskFromString(&m_rxfilter, str);
    }

    //  New format
    strName = m_prefix +
            wxString::FromAscii("_rxfilter");
    if (VSCP_ERROR_SUCCESS == m_srvLocal.getRemoteVariableValue(strName, str)) {
        vscp_readFilterFromString(&m_rxfilter, str);
    }

    strName = m_prefix +
            wxString::FromAscii("_rxmask");
    if (VSCP_ERROR_SUCCESS == m_srvLocal.getRemoteVariableValue(strName, str)) {
        vscp_readMaskFromString(&m_rxfilter, str);
    }

    strName = m_prefix +
            wxString::FromAscii("_txfilter");
    if (VSCP_ERROR_SUCCESS == m_srvLocal.getRemoteVariableValue(strName, str)) {
        vscp_readFilterFromString(&m_txfilter, str);
    }

    strName = m_prefix +
            wxString::FromAscii("_txmask");
    if (VSCP_ERROR_SUCCESS == m_srvLocal.getRemoteVariableValue(strName, str)) {
        vscp_readMaskFromString(&m_txfilter, str);
    }

    strName = m_prefix +
            wxString::FromAscii("_response_timout");
    if (VSCP_ERROR_SUCCESS == m_srvLocal.getRemoteVariableValue( strName, str ) ) {
        m_responseTimeout = vscp_readStringValue( str );
    }

    /////////////////////////////////////////////////////////////
    //                    XML Configuration
    /////////////////////////////////////////////////////////////
    
    wxString setupXml;
    strName = m_prefix +
            wxString::FromAscii("_setup");
    if (VSCP_ERROR_SUCCESS != m_srvLocal.getRemoteVariableValue( strName, setupXml ) ) {
        // Not here, we use empty mock-up
        setupXml =  _("<?xml version = \"1.0\" encoding = \"UTF-8\" ?><setup><!-- empty --></setup>");
    }        

    wxStringInputStream xmlstream( setupXml );
    wxXmlDocument doc;

    if ( doc.Load( xmlstream ) ) {

        // start processing the XML file
        if ( (doc.GetRoot()->GetName() == _("setup") ) ) {

            wxString attribute;

            // response timeout
            attribute = doc.GetRoot()->GetAttribute("response-timeout", "");
            if ( attribute.Length() ) {
                m_responseTimeout = vscp_readStringValue( attribute );
            }

            // Remote host 
            attribute = doc.GetRoot()->GetAttribute("host", "");
            if ( attribute.Length() ) {
                m_hostRemote = attribute;
            }

            // Remote port m_usernameRemote
            attribute = doc.GetRoot()->GetAttribute("port", "");
            if ( attribute.Length() ) {
                m_portRemote = vscp_readStringValue( attribute );
            }

            // Remote port m_usernameRemote
            attribute = doc.GetRoot()->GetAttribute("user", "");
            if ( attribute.Length() ) {
                m_usernameRemote = attribute;
            }

            // Remote port 
            attribute = doc.GetRoot()->GetAttribute("password", "");
            if ( attribute.Length() ) {
                m_passwordRemote = attribute;
            }

            // RX filter 
            attribute = doc.GetRoot()->GetAttribute("rxfilter", "");
            if ( attribute.Length() ) {
                vscp_readFilterFromString(&m_rxfilter, attribute);
            }

            // RX mask 
            attribute = doc.GetRoot()->GetAttribute("rxmask", "");
            if ( attribute.Length() ) {
                vscp_readMaskFromString(&m_rxfilter, attribute);
            }

            // TX filter 
            attribute = doc.GetRoot()->GetAttribute("txfilter", "");
            if ( attribute.Length() ) {
                vscp_readFilterFromString(&m_txfilter, attribute);
            }

            // TX mask 
            attribute = doc.GetRoot()->GetAttribute("txmask", "");
            if ( attribute.Length() ) {
                vscp_readMaskFromString(&m_txfilter, attribute);
            }
            
        }
        else {
            syslog( LOG_ERR,
		    		    "%s %s ",
                        (const char *)VSCP_TCPIPLINK_SYSLOG_DRIVER_ID,
				        (const char *)"Malformed configuration XML (<setup> tag missing). Terminating!");
        }
        
    }
    else {
        syslog( LOG_ERR,
				    "%s %s ",
                    (const char *)VSCP_TCPIPLINK_SYSLOG_DRIVER_ID,
				    (const char *)"Unable to parse XML config. Maybe just not used.");
        
    }

    



    // Close the channel
    m_srvLocal.doCmdClose();

    // start the workerthread
    m_pthreadSend = new CWrkSendTread();
    if (NULL != m_pthreadSend) {
        m_pthreadSend->m_pObj = this;
        m_pthreadSend->Create();
        m_pthreadSend->Run();
    } 
    else {
        rv = false;
    }
    
    // start the worker thread
    m_pthreadReceive = new CWrkReceiveTread();
    if (NULL != m_pthreadReceive) {
        m_pthreadReceive->m_pObj = this;
        m_pthreadReceive->Create();
        m_pthreadReceive->Run();
    } 
    else {
        rv = false;
    }

    return rv;
}


//////////////////////////////////////////////////////////////////////
// close
//

void
CTcpipLink::close(void)
{
    // Do nothing if already terminated
    if (m_bQuit) return;

    m_bQuit = true; // terminate the thread
    wxSleep(1); // Give the thread some time to terminate
}


//////////////////////////////////////////////////////////////////////
// addEvent2SendQueue
//

bool 
CTcpipLink::addEvent2SendQueue(const vscpEvent *pEvent)
{
    m_mutexSendQueue.Lock();
    m_sendList.push_back((vscpEvent *)pEvent);
    m_semSendQueue.Post();
    m_mutexSendQueue.Unlock();
    return true;
}

//////////////////////////////////////////////////////////////////////
//                Workerthread Send - CWrkSendTread
//////////////////////////////////////////////////////////////////////

CWrkSendTread::CWrkSendTread()
{
    m_pObj = NULL;
}

CWrkSendTread::~CWrkSendTread()
{
    ;
}


//////////////////////////////////////////////////////////////////////
// Entry
//

void *
CWrkSendTread::Entry()
{
    bool bRemoteConnectionLost = false;

    ::wxInitialize();
            
    // Check pointers
    if (NULL == m_pObj) return NULL;
    
 retry_send_connect:

    // Open remote interface
    if ( VSCP_ERROR_SUCCESS != 
            m_srvRemote.doCmdOpen( m_pObj->m_hostRemote.ToStdString(),
                                    m_pObj->m_portRemote,
                                    m_pObj->m_usernameRemote.ToStdString(),
                                    m_pObj->m_passwordRemote.ToStdString() ) ) {
        syslog(LOG_ERR,
                "%s %s ",
                VSCP_TCPIPLINK_SYSLOG_DRIVER_ID,
                (const char *) "Error while opening remote VSCP TCP/IP interface. Terminating!");
        
        // Give the server some time to become active
        for ( int loopcnt=0;loopcnt<VSCP_TCPIPLINK_DEFAULT_RECONNECT_TIME; loopcnt++ ) {
            wxSleep(1);        
            if ( TestDestroy() || m_pObj->m_bQuit ) return NULL;
        }

        goto retry_send_connect;
    }

    syslog(LOG_ERR,
                "%s %s ",
                VSCP_TCPIPLINK_SYSLOG_DRIVER_ID,
                (const char *) "Connect to remote VSCP TCP/IP interface [SEND].");
    
    // Find the channel id
    m_srvRemote.doCmdGetChannelID( &m_pObj->txChannelID );

    while ( !TestDestroy() && !m_pObj->m_bQuit ) {

        // Make sure the remote connection is up
        if ( !m_srvRemote.isConnected() ) {

            if ( !bRemoteConnectionLost ) {
                bRemoteConnectionLost = true;
                m_srvRemote.doCmdClose();
                syslog(LOG_ERR,
                        "%s %s ",
                        VSCP_TCPIPLINK_SYSLOG_DRIVER_ID,
                        (const char *) "Lost connection to remote host [SEND].");
            }

            // Wait before we try to connect again
            ::wxSleep(VSCP_TCPIPLINK_DEFAULT_RECONNECT_TIME);

            if ( VSCP_ERROR_SUCCESS != 
                    m_srvRemote.doCmdOpen( m_pObj->m_hostRemote.ToStdString(),
                                            m_pObj->m_portRemote,
                                            m_pObj->m_usernameRemote.ToStdString(),
                                            m_pObj->m_passwordRemote.ToStdString() ) ) {
                syslog(LOG_ERR,
                        "%s %s ",
                        VSCP_TCPIPLINK_SYSLOG_DRIVER_ID,
                        (const char *) "Reconnected to remote host [SEND].");
                
                // Find the channel id
                m_srvRemote.doCmdGetChannelID(&m_pObj->txChannelID);
    
                bRemoteConnectionLost = false;
            }
            
            continue;

        }

        if ( wxSEMA_TIMEOUT == m_pObj->m_semSendQueue.WaitTimeout( 500 ) ) continue;
        
        // Check if there is event(s) to send
        if ( m_pObj->m_sendList.size() ) {

            // Yes there are data to send
            m_pObj->m_mutexSendQueue.Lock();
            vscpEvent *pEvent = m_pObj->m_sendList.front();
            // Check if event should be filtered away
            if ( !vscp_doLevel2Filter( pEvent, &m_pObj->m_txfilter ) ) {
                m_pObj->m_mutexSendQueue.Unlock();
                continue;
            }
            m_pObj->m_sendList.pop_front();
            m_pObj->m_mutexSendQueue.Unlock();

            if ( NULL == pEvent ) continue;
            
            // Yes there are data to send
            // Send it out to the remote server
                
            m_srvRemote.doCmdSend( pEvent );
            vscp_deleteVSCPevent_v2( &pEvent );
            
        }
                
    }

    // Close the channel
    m_srvRemote.doCmdClose();

    syslog(LOG_ERR,
                "%s %s ",
                VSCP_TCPIPLINK_SYSLOG_DRIVER_ID,
                (const char *) "Disconnect from remote VSCP TCP/IP interface [SEND].");


    return NULL;

}

//////////////////////////////////////////////////////////////////////
// OnExit
//

void
CWrkSendTread::OnExit()
{
    ;
}




//////////////////////////////////////////////////////////////////////
//                Workerthread Receive - CWrkReceiveTread
//////////////////////////////////////////////////////////////////////

CWrkReceiveTread::CWrkReceiveTread()
{
    m_pObj = NULL;
}

CWrkReceiveTread::~CWrkReceiveTread()
{
    ;
}


//////////////////////////////////////////////////////////////////////
// Entry
//

void *
CWrkReceiveTread::Entry()
{
    bool bRemoteConnectionLost = false;
    bool bActivity = false;

    ::wxInitialize();
            
    // Check pointers
    if (NULL == m_pObj) return NULL;

retry_receive_connect:

    // Open remote interface
    if ( VSCP_ERROR_SUCCESS != 
                m_srvRemote.doCmdOpen( m_pObj->m_hostRemote.ToStdString(),
                                        m_pObj->m_portRemote,
                                        m_pObj->m_usernameRemote.ToStdString(),
                                        m_pObj->m_passwordRemote.ToStdString() ) ) {
        syslog(LOG_ERR,
                "%s %s ",
                VSCP_TCPIPLINK_SYSLOG_DRIVER_ID,
                (const char *) "Error while opening remote VSCP TCP/IP interface. Terminating!");
        
        // Give the server some time to become active
        for ( int loopcnt=0;loopcnt<VSCP_TCPIPLINK_DEFAULT_RECONNECT_TIME; loopcnt++ ) {
            wxSleep(1);        
            if ( TestDestroy() || m_pObj->m_bQuit ) return NULL;
        }

        goto retry_receive_connect;
        
    }

    syslog( LOG_ERR,
                "%s %s ",
                VSCP_TCPIPLINK_SYSLOG_DRIVER_ID,
                (const char *) "Connect to remote VSCP TCP/IP interface [RECEIVE].");
    
    // Set receive filter
    if ( VSCP_ERROR_SUCCESS != m_srvRemote.doCmdFilter( &m_pObj->m_rxfilter ) ) {
        syslog(LOG_ERR,
                "%s %s ",
                VSCP_TCPIPLINK_SYSLOG_DRIVER_ID,
                (const char *) "Failed to set receiving filter.");

    }

    // Enter the receive loop
    m_srvRemote.doCmdEnterReceiveLoop();

    vscpEventEx eventEx;
    while ( !TestDestroy() && !m_pObj->m_bQuit ) {

        // Make sure the remote connection is up
        if ( !m_srvRemote.isConnected() || 
              ( ( wxGetLocalTimeMillis() - m_srvRemote.getlastResponseTime() ) > ( VSCP_TCPIPLINK_DEFAULT_RECONNECT_TIME * 1000 ) ) ) {

            if ( !bRemoteConnectionLost ) {

                bRemoteConnectionLost = true;
                m_srvRemote.doCmdClose();
                syslog(LOG_ERR,
                        "%s %s ",
                        VSCP_TCPIPLINK_SYSLOG_DRIVER_ID,
                        (const char *) "Lost connection to remote host [Receive].");
            }

            // Wait before we try to connect again
            ::wxSleep(VSCP_TCPIPLINK_DEFAULT_RECONNECT_TIME);

            if ( VSCP_ERROR_SUCCESS != 
                        m_srvRemote.doCmdOpen( m_pObj->m_hostRemote.ToStdString(),
                                                m_pObj->m_portRemote,
                                                m_pObj->m_usernameRemote.ToStdString(),
                                                m_pObj->m_passwordRemote.ToStdString() ) ) {
                syslog(LOG_ERR,
                        "%s %s ",
                        VSCP_TCPIPLINK_SYSLOG_DRIVER_ID,
                        (const char *) "Reconnected to remote host [Receive].");
                bRemoteConnectionLost = false;
            }
            
            // Enter the receive loop
            m_srvRemote.doCmdEnterReceiveLoop();
            
            continue;

        }   
        
        // Check if remote server has something to send to us
        vscpEvent *pEvent = new vscpEvent;
        if (NULL != pEvent) {
            
            pEvent->sizeData = 0;
            pEvent->pdata = NULL;
            
            if ( CANAL_ERROR_SUCCESS == m_srvRemote.doCmdBlockingReceive( pEvent ) ) {

                // Filter is handled at server side. We check so we don't receive
                // things we send ourself.
                if ( m_pObj->txChannelID != pEvent->obid ) {
                    m_pObj->m_mutexReceiveQueue.Lock();
                    m_pObj->m_receiveList.push_back( pEvent );
                    m_pObj->m_semReceiveQueue.Post();
                    m_pObj->m_mutexReceiveQueue.Unlock();
                }
                else {
                    vscp_deleteVSCPevent( pEvent );
                }
                
            }
            else {
                vscp_deleteVSCPevent( pEvent );
            }

        }
                
    }

    // Close the channel
    m_srvRemote.doCmdClose();

    syslog(LOG_ERR,
                "%s %s ",
                VSCP_TCPIPLINK_SYSLOG_DRIVER_ID,
                (const char *) "Disconnect from remote VSCP TCP/IP interface [RECEIVE].");

    return NULL;

}

//////////////////////////////////////////////////////////////////////
// OnExit
//

void
CWrkReceiveTread::OnExit()
{
    ;
}


