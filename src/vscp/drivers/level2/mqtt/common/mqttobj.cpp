// mqttobj.cpp: implementation of the CMQTT class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP Project (http://www.vscp.org) 
//
// Copyright (C) 2000-2015 Ake Hedman, 
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
#ifndef WIN32
#include "unistd.h"
#include "syslog.h"
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <libgen.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <net/if.h>
#endif
#include "stdlib.h"
#include <string.h>
#include "limits.h"
#include <signal.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>

#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"
#include <wx/xml/xml.h>
#include <wx/listimpl.cpp>
#include <wx/thread.h>
#include <wx/tokenzr.h>
#include <wx/datetime.h>

#include <fossa.h>

#include <vscphelper.h>
#include <vscpremotetcpif.h>
#include <vscp_type.h>
#include <vscp_class.h>
#include "mqttobj.h"



//////////////////////////////////////////////////////////////////////
// on_message
//
/*
void mqtt_subscribe::on_message(const struct mosquitto_message *message)
{
	vscpEventEx eventEx;

	if ( !strcmp( message->topic, m_pObj->m_topic.ToAscii() ) ) {
        
		wxString str = wxString::FromAscii((const char *) message->payload);
		if (vscp_setVscpEventExFromString(&eventEx, str)) {
            
            vscpEvent *pEvent = new vscpEvent;
            if (NULL != pEvent) {
                
                pEvent->sizeData = 0;
                pEvent->pdata = NULL;
                
                if ( vscp_doLevel2FilterEx( &eventEx, &m_pObj->m_vscpfilter ) ) {
                    
                    if ( vscp_convertVSCPfromEx( pEvent, &eventEx ) ) {
                        m_pObj->m_mutexReceiveQueue.Lock();
                        m_pObj->m_receiveList.push_back(pEvent);
                        m_pObj->m_semReceiveQueue.Post();
                        m_pObj->m_mutexReceiveQueue.Unlock();
                    }
                    else {
                        
                    }
                }
                else {
                    vscp_deleteVSCPevent(pEvent);
                }
            }
		}
	}
}
*/


static void ev_handler( struct ns_connection *nc, int ev, void *p )
{
    struct ns_mqtt_message *msg = ( struct ns_mqtt_message * )p;
    Cmqttobj *pmqttobj = ( Cmqttobj *)nc->mgr->user_data;

    switch ( ev ) {

        case NS_CONNECT:
            ns_set_protocol_mqtt( nc );
            ns_send_mqtt_handshake( nc, "vscpd" );
            break;

        case NS_MQTT_CONNACK:
            if ( msg->connack_ret_code != NS_MQTT_CONNACK_ACCEPTED ) {
                printf( "Got mqtt connection error: %d\n", msg->connack_ret_code );
                pmqttobj->m_bQuit = true;
                return;
            }
            
            pmqttobj->m_bConnected = true;

            if ( pmqttobj->m_bSubscribe ) {
                ns_mqtt_subscribe( nc, pmqttobj->m_topic_list, 1, 42 );
            }
            else {
                ;
            }
            break;

        case NS_MQTT_PUBACK:
            printf( "Message publishing acknowledged (msg_id: %d)\n", msg->message_id );
            break;

        case NS_MQTT_SUBACK:
            printf( "Subscription acknowledged, forwarding to '/test'\n" );
            break;

        case NS_MQTT_PUBLISH:
        {
            printf( "Got incoming message %s: %.*s\n", msg->topic, ( int )msg->payload.len, msg->payload.p );
            vscpEventEx eventEx;

            if ( !strcmp( msg->topic, pmqttobj->m_topic.ToAscii() ) ) {

                wxString str = wxString::FromAscii( ( const char * )msg->payload.p );
                if ( vscp_setVscpEventExFromString( &eventEx, str ) ) {

                    vscpEvent *pEvent = new vscpEvent;
                    if ( NULL != pEvent ) {

                        pEvent->sizeData = 0;
                        pEvent->pdata = NULL;

                        if ( vscp_doLevel2FilterEx( &eventEx, &pmqttobj->m_vscpfilter ) ) {

                            if ( vscp_convertVSCPfromEx( pEvent, &eventEx ) ) {
                                pmqttobj->m_mutexReceiveQueue.Lock();
                                pmqttobj->m_receiveList.push_back( pEvent );
                                pmqttobj->m_semReceiveQueue.Post();
                                pmqttobj->m_mutexReceiveQueue.Unlock();
                            }
                            else {

                            }
                        }
                        else {
                            vscp_deleteVSCPevent( pEvent );
                        }
                    }
                }
            }
        }
        break;

        case NS_CLOSE:
            printf( "Connection closed\n" );
            pmqttobj->m_bConnected = false;
            pmqttobj->m_bQuit = true;
    }

}


//////////////////////////////////////////////////////////////////////
// Cmqttobj
//

Cmqttobj::Cmqttobj()
{
	m_bQuit = false;
    m_bConnected = false;
	m_pthreadWork = NULL;
	m_bSubscribe = true;
    m_bSimplify = false;
    m_simple_class = 0;
    m_simple_type = 0;
    m_simple_coding = 0;
    m_simple_zone = 0;
    m_simple_subzone = 0;

    m_topic_list[0].qos = 0;
    m_topic_list[ 0 ].topic = NULL,

	vscp_clearVSCPFilter(&m_vscpfilter); // Accept all events
	::wxInitialize();
}

//////////////////////////////////////////////////////////////////////
// ~Cmqttobj
//

Cmqttobj::~Cmqttobj()
{
	close();
	::wxUninitialize();
}


//////////////////////////////////////////////////////////////////////
// open
//
//

bool
Cmqttobj::open( const char *pUsername,
                    const char *pPassword,
                    const char *pHost,
                    const char *pPrefix,
                    const char *pConfig)
{
	bool rv = true;
	wxString str;
	wxString wxstr = wxString::FromAscii(pConfig);

	m_username = wxString::FromAscii(pUsername);
	m_password = wxString::FromAscii(pPassword);
	m_host = wxString::FromAscii(pHost);
	m_prefix = wxString::FromAscii(pPrefix);

	// Parse the configuration string. It should
	// have the following form
	// path
	// 
	wxStringTokenizer tkz(wxString::FromAscii(pConfig), _(";\n"));

	// Check if we should publish or subscribe
	if (tkz.HasMoreTokens()) {
		// Check for subscribe/publish
		str = tkz.GetNextToken();
		str.Trim();
		str.Trim(false);
		str.MakeUpper();
		if (wxNOT_FOUND != str.Find(_("PUBLISH"))) {
			m_bSubscribe = false;
		}
	}

	// Get topic from configuration string
	if (tkz.HasMoreTokens()) {
		m_topic = tkz.GetNextToken();
	}

	// Get MQTT host from configuration string
	if (tkz.HasMoreTokens()) {
		m_hostMQTT = tkz.GetNextToken();
	}

	// Get MQTT host port from configuration string
	if (tkz.HasMoreTokens()) {
		m_portMQTT = vscp_readStringValue(tkz.GetNextToken());
	}

	// Get MQTT user from configuration string
	if (tkz.HasMoreTokens()) {
		m_usernameMQTT = tkz.GetNextToken();
	}

	// Get MQTT password from configuration string
	if (tkz.HasMoreTokens()) {
		m_passwordMQTT = tkz.GetNextToken();
	}

	// Get MQTT keep alive from configuration string
	if (tkz.HasMoreTokens()) {
		m_keepalive = vscp_readStringValue(tkz.GetNextToken());
	}

	// First log on to the host and get configuration 
	// variables

	if ( VSCP_ERROR_SUCCESS !=  m_srv.doCmdOpen( m_host,
                                                    m_username,
                                                    m_password) ) {
#ifndef WIN32
		syslog(LOG_ERR,
				"%s",
				(const char *) "Unable to connect to VSCP TCP/IP interface. Terminating!");
#endif
		return false;
	}

	// Find the channel id
	uint32_t ChannelID;
	m_srv.doCmdGetChannelID(&ChannelID);

	// The server should hold configuration data for each sensor
	// we want to monitor.
	// 
	// We look for 
	//
	//	 _type	- “subscribe” to subscribe to a MQTT topic. ”publish” to 
	//				publish events to a MQTT topic. Defaults to “subscribe”.
	//
	//	 _topic	- This is a text string identifying the topic. It is 
	//				recommended that this string starts with “vscp/”. 
	//						Defaults to “vscp”
	//
    //	 _qos	- MQTT QOS value. Defaults to 0. 
    //
	//	 _host	- IP address + port or a DNS resolvable address + port on the 
    //              form host:port to the remote host. 
	//				Mandatory and must be declared either in the configuration 
	//				string or in this variable. Defaults to “localhost:1883”
	//
	//	 _user - Username used to log in on the remote sever. 
	//				Defaults to empty. 
	//
	//	 _password - Password used to login on the remote server. 
	//				Defaults to empty.
	//
	//	 _keepalive - Keepalive value for channel. Defaults to 60.
	//
	//   _filter - Standard VSCP filter in string form. 
	//				   1,0x0000,0x0006,
	//				   ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00
	//				as priority,class,type,GUID
	//				Used to filter what events that is received from 
	//				the mqtt interface. If not give all events 
	//				are received.
	//	 _mask - Standard VSCP mask in string form.
	//				   1,0x0000,0x0006,
	//				   ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00
	//				as priority,class,type,GUID
	//				Used to filter what events that is received from 
	//				the mqtt interface. If not give all events 
	//				are received. 
	//

	wxString strName = m_prefix +
			wxString::FromAscii("_type");
	m_srv.getVariableString(strName, &str);

	// Check for subscribe/publish
	str = tkz.GetNextToken();
	str.Trim();
	str.Trim(false);
	str.MakeUpper();
	if ( wxNOT_FOUND != str.Find(_("publish") ) ) {
		m_bSubscribe = false;
	}

	strName = m_prefix +
			wxString::FromAscii("_topic");
    if ( VSCP_ERROR_SUCCESS == m_srv.getVariableString( strName, &str ) ) {
        m_topic = str;
    }

	strName = m_prefix +
			wxString::FromAscii("_host");
    if ( VSCP_ERROR_SUCCESS == m_srv.getVariableString( strName, &m_hostMQTT ) ) {
        m_hostMQTT = str;
    }

	strName = m_prefix +
			wxString::FromAscii("_username");
    if ( VSCP_ERROR_SUCCESS == m_srv.getVariableString( strName, &str ) ) {
        m_usernameMQTT = str;
    }

	strName = m_prefix +
			wxString::FromAscii("_password");
    if ( VSCP_ERROR_SUCCESS == m_srv.getVariableString( strName, &m_passwordMQTT ) ) {
        m_passwordMQTT = str;
    }

	strName = m_prefix +
			wxString::FromAscii("_keepalive");
    int intval;
    if ( VSCP_ERROR_SUCCESS == m_srv.getVariableInt( strName, &intval ) ) {
        m_keepalive = intval;
    }

    strName = m_prefix +
        wxString::FromAscii( "_qos" );
    if ( VSCP_ERROR_SUCCESS == m_srv.getVariableInt( strName, &intval ) ) {
        m_topic_list[ 0 ].qos = intval;
    }

	strName = m_prefix +
			wxString::FromAscii("_filter");
    if ( VSCP_ERROR_SUCCESS == m_srv.getVariableString( strName, &str ) ) {
		vscp_readFilterFromString(&m_vscpfilter, str);
    }

	strName = m_prefix +
			wxString::FromAscii("_mask");
    if ( VSCP_ERROR_SUCCESS == m_srv.getVariableString( strName, &str ) ) {
		vscp_readMaskFromString(&m_vscpfilter, str);
	}
    
    strName = m_prefix +
			wxString::FromAscii("_simplify");
    if ( VSCP_ERROR_SUCCESS == m_srv.getVariableString( strName, &str ) ) {
        m_simplify = str;
    }
    
    if ( m_simplify.Length() ) {
        
        m_bSimplify = true; 
        
        wxStringTokenizer tkzSimple( m_simplify, _(",\n"));
        
        // simple class
        if (tkzSimple.HasMoreTokens()) {
            m_simple_class = vscp_readStringValue(tkzSimple.GetNextToken());
        }
        
        // simple type
        if (tkzSimple.HasMoreTokens()) {
            m_simple_type = vscp_readStringValue(tkzSimple.GetNextToken());
        }
        
        // simple coding
        if (tkzSimple.HasMoreTokens()) {
            m_simple_coding = vscp_readStringValue(tkzSimple.GetNextToken());
        }
        
        // simple zone
        if (tkzSimple.HasMoreTokens()) {
            m_simple_zone = vscp_readStringValue(tkzSimple.GetNextToken());
        }
        
        // simple subzone
        if (tkzSimple.HasMoreTokens()) {
            m_simple_subzone = vscp_readStringValue(tkzSimple.GetNextToken());
        }
        
    }
    else {
        m_bSimplify = false;
    }

    if ( m_bSubscribe ) {
        m_topic_list[ 0 ].topic = new char( m_topic.Length() + 1 );
        if ( NULL != m_topic_list[ 0 ].topic ) {
            memset( (void *)m_topic_list[ 0 ].topic, 0, m_topic.Length() + 1 );
            memcpy( ( void * )m_topic_list[ 0 ].topic, m_topic.mbc_str(), m_topic.Length() );
        }
    }
    
	// Close the channel
	m_srv.doCmdClose();

	// start the worker thread
	m_pthreadWork = new CWrkThread();
	if (NULL != m_pthreadWork) {
		m_pthreadWork->m_pObj = this;
		m_pthreadWork->Create();
		m_pthreadWork->Run();
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
Cmqttobj::close( void )
{
	// Do nothing if already terminated
	if (m_bQuit) return;

	m_bQuit = true; // terminate the thread
	wxSleep(1);		// Give the thread some time to terminate
}

//////////////////////////////////////////////////////////////////////
// addEvent2SendQueue
//

bool 
Cmqttobj::addEvent2SendQueue( const vscpEvent *pEvent )
{
    m_mutexSendQueue.Lock();
    m_sendList.push_back((vscpEvent *)pEvent);
	m_semSendQueue.Post();
	m_mutexSendQueue.Unlock();
    return true;
}

//////////////////////////////////////////////////////////////////////
//                Workerthread - CWrkThread
//////////////////////////////////////////////////////////////////////

CWrkThread::CWrkThread()
{
    m_pObj = NULL;
}

CWrkThread::~CWrkThread()
{
    ;
}


//////////////////////////////////////////////////////////////////////
// Entry
//

void *
CWrkThread::Entry()
{
    wxString str;
    struct ns_connection *nc;
    struct ns_mgr mgr;

    const char *address = "192.168.1.9:1883";

    mgr.user_data = m_pObj;
    ns_mgr_init( &mgr, m_pObj );

    if ( NULL == ( nc = ns_connect( &mgr, (const char *)m_pObj->m_hostMQTT.mbc_str(), ev_handler ) ) ) {
        fprintf( stderr, "ns_connect(%s) failed\n", address );
        return NULL;
    }

	if (m_pObj->m_bSubscribe) {

        while (!TestDestroy() && !m_pObj->m_bQuit) {
            ns_mgr_poll( &mgr, 100 );           
        }

    }
    // Publish
    else {
        
        while ( !TestDestroy() && !m_pObj->m_bQuit ) {

            ns_mgr_poll( &mgr, 100 );

            // Wait for connection
            if ( !m_pObj->m_bConnected ) {
                continue;
            }

            if ( wxSEMA_TIMEOUT == m_pObj->m_semSendQueue.WaitTimeout( 10 ) ) continue;            

            if ( m_pObj->m_sendList.size() ) {

                m_pObj->m_mutexSendQueue.Lock();
                vscpEvent *pEvent = m_pObj->m_sendList.front();
                m_pObj->m_sendList.pop_front();
                m_pObj->m_mutexSendQueue.Unlock();
                if (NULL == pEvent) continue;
                if (m_pObj->m_bSimplify) {
                    
                }
                else {
                    vscp_writeVscpEventToString( pEvent, str );
                    ns_mqtt_publish( nc, 
                                        "/vscp1", 
                                        65, 
                                        NS_MQTT_QOS( 0 ), 
                                        (const char *)str.mbc_str(), 
                                        str.Length() );
                }
                
                // We are done with the event - remove data if any
                if (NULL != pEvent->pdata) {
                    delete [] pEvent->pdata;
                    pEvent->pdata = NULL;
                }

            } // Event received


            ::wxMilliSleep(50);

        }
    
    }


    ns_mqtt_disconnect( nc );

    return NULL;

}

//////////////////////////////////////////////////////////////////////
// OnExit
//

void
CWrkThread::OnExit()
{
    ;
}

