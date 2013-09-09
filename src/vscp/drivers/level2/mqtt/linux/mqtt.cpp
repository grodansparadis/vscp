// mqtt.cpp: implementation of the CMQTT class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP Project (http://www.vscp.org) 
//
// Copyright (C) 2000-2013 Ake Hedman, 
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
#include <wx/xml/xml.h>
#include <wx/listimpl.cpp>
#include <wx/thread.h>
#include <wx/tokenzr.h>
#include <wx/datetime.h>

#include "../../../../common/vscphelper.h"
#include "../../../../common/vscptcpif.h"
#include "../../../../common/vscp_type.h"
#include "../../../../common/vscp_class.h"
#include "mqtt.h"

//////////////////////////////////////////////////////////////////////
//								Subscriber
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// mqtt_subscribe - Constructor
//

mqtt_subscribe::mqtt_subscribe(const wxString& usernameLocal,
                                    const wxString& passwordLocal,
                                    const wxString& hostLocal,
                                    const int portLocal,
                                    const wxString& topic,
                                    const wxString& hostMQTT,
                                    int portMQTT,
                                    int keepalive)
{
    m_hostLocal = hostLocal;
	m_portLocal = portLocal;
	m_usernameLocal = usernameLocal;
	m_passwordLocal = passwordLocal;
	m_topic = topic;
	m_hostMQTT = hostMQTT;
	m_portMQTT = portMQTT;
	m_keepalive = keepalive;

	// Connect to the VSCP Server
	if (m_srv.doCmdOpen(hostLocal,
			portLocal,
			usernameLocal,
			passwordLocal) <= 0) {
		syslog(LOG_ERR,
				"%s",
				(const char *) "mqtt_subscribe:Unable to connect to VSCP TCP/IP interface. Terminating!");
		return;
	}

	// Find the channel id
	uint32_t ChannelID;
	m_srv.doCmdGetChannelID(&ChannelID);

    
	/* Connect immediately. This could also be done by calling
	 * mqtt_tempconv->connect(). */
	//connect(host, port );
}

//////////////////////////////////////////////////////////////////////
// mqtt_subscribe Destructor
//

mqtt_subscribe::~mqtt_subscribe()
{
	// Close the VSCP channel
	m_srv.doCmdClose();
}

//////////////////////////////////////////////////////////////////////
// on_connect
//

void mqtt_subscribe::on_connect(int rc)
{
	if (0 == rc) {
		// Only attempt to subscribe on a successful connect. 
		subscribe(NULL, m_topic.ToAscii());
	}
}

//////////////////////////////////////////////////////////////////////
// on_message
//

void mqtt_subscribe::on_message(const struct mosquitto_message *message)
{
	vscpEventEx eventEx;

	if (!strcmp(message->topic, m_topic.ToAscii())) {
		wxString str = wxString::FromAscii((const char *) message->payload);
		if (getVscpEventExFromString(&eventEx, str)) {
			m_srv.doCmdSendEx(&eventEx);
		}
	}
}

//////////////////////////////////////////////////////////////////////
// on_subscribe
//

void mqtt_subscribe::on_subscribe(int mid, int qos_count, const int *granted_qos)
{
	;
}


//////////////////////////////////////////////////////////////////////
//								Publisher
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// mqtt_publish - Constructor
//

mqtt_publish::mqtt_publish(const wxString& usernameLocal,
                            const wxString& passwordLocal,
                            const wxString& hostLocal,
                            const int portLocal,
                            const wxString& topic,
                            const wxString& hostMQTT,
                            int portMQTT,
                            int keepalive)
{
	m_hostLocal = hostLocal;
	m_portLocal = portLocal;
	m_usernameLocal = usernameLocal;
	m_passwordLocal = passwordLocal;
	m_topic = topic;
	m_hostMQTT = hostMQTT;
	m_portMQTT = portMQTT;
	m_keepalive = keepalive;

	//int keepalive = 60;

	/* Connect immediately. This could also be done by calling
	 * mqtt_tempconv->connect(). */
	//connect(host, port, keepalive);
}

//////////////////////////////////////////////////////////////////////
// mqtt_subscribe Destructor
//

mqtt_publish::~mqtt_publish()
{
	;
}

//////////////////////////////////////////////////////////////////////
// on_connect
//

void mqtt_publish::on_connect(int rc)
{
	printf("Connected with code %d.\n", rc);
	if (rc == 0) {
		/* Only attempt to subscribe on a successful connect. */
		subscribe(NULL, "temperature/celsius");
	}
}

//////////////////////////////////////////////////////////////////////
// on_message
//

void mqtt_publish::on_message(const struct mosquitto_message *message)
{
	double temp_celsius, temp_farenheit;
	char buf[51];

	if (!strcmp(message->topic, "temperature/celsius")) {
		memset(buf, 0, 51 * sizeof(char));
		/* Copy N-1 bytes to ensure always 0 terminated. */
		memcpy(buf, message->payload, 50 * sizeof(char));
		temp_celsius = atof(buf);
		temp_farenheit = temp_celsius * 9.0 / 5.0 + 32.0;
		snprintf(buf, 50, "%f", temp_farenheit);
		publish(NULL, "temperature/farenheit", strlen(buf), buf);
	}
}

//////////////////////////////////////////////////////////////////////
// on_subscribe
//

void mqtt_publish::on_subscribe(int mid, int qos_count, const int *granted_qos)
{
	printf("Subscription succeeded.\n");
}

//////////////////////////////////////////////////////////////////////
// Cmqtt
//

Cmqtt::Cmqtt()
{
	m_bQuit = false;
	m_pthreadWork = NULL;
	bSubscribe = true;
	clearVSCPFilter(&m_vscpfilter); // Accept all events
	::wxInitialize();
}

//////////////////////////////////////////////////////////////////////
// ~Cmqtt
//

Cmqtt::~Cmqtt()
{
	close();
	::wxUninitialize();
}


//////////////////////////////////////////////////////////////////////
// open
//
//

bool
Cmqtt::open(const char *pUsername,
                const char *pPassword,
                const char *pHost,
                short port,
                const char *pPrefix,
                const char *pConfig)
{
	bool rv = true;
	wxString str;
	wxString wxstr = wxString::FromAscii(pConfig);

	m_username = wxString::FromAscii(pUsername);
	m_password = wxString::FromAscii(pPassword);
	m_host = wxString::FromAscii(pHost);
	m_port = port;
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
			bSubscribe = false;
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
		m_portMQTT = readStringValue(tkz.GetNextToken());
	}

	// Get MQTT user from configuration string
	if (tkz.HasMoreTokens()) {
		m_usernameMQTT = tkz.GetNextToken();
	}

	// Get MQTT password from configuration string
	if (tkz.HasMoreTokens()) {
		m_passwordMQTT = tkz.GetNextToken();
	}

	// Get MQTT keepalive from configuration string
	if (tkz.HasMoreTokens()) {
		m_keepalive = readStringValue(tkz.GetNextToken());
	}

	// First log on to the host and get configuration 
	// variables

	if (m_srv.doCmdOpen(m_host,
			m_port,
			m_username,
			m_password) <= 0) {
		syslog(LOG_ERR,
				"%s",
				(const char *) "Unable to connect to VSCP TCP/IP interface. Terminating!");
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
	//	 _host	- IP address or a DNS resolvable address to the remote host. 
	//				Mandatory and must be declared either in the configuration 
	//				string or in this variable. Defaults to “localhost”
	//
	//	 _port - The port to use on the remote host. Default is 1883.
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
	if (wxNOT_FOUND != str.Find(_("PUBLISH"))) {
		bSubscribe = false;
	}

	strName = m_prefix +
			wxString::FromAscii("_topic");
	m_srv.getVariableString(strName, &m_topic);

	strName = m_prefix +
			wxString::FromAscii("_host");
	m_srv.getVariableString(strName, &m_hostMQTT);

	strName = m_prefix +
			wxString::FromAscii("_port");
	m_srv.getVariableInt(strName, &m_portMQTT);

	strName = m_prefix +
			wxString::FromAscii("_username");
	m_srv.getVariableString(strName, &m_usernameMQTT);

	strName = m_prefix +
			wxString::FromAscii("_password");
	m_srv.getVariableString(strName, &m_passwordMQTT);

	strName = m_prefix +
			wxString::FromAscii("_keepalive");
	m_srv.getVariableInt(strName, &m_keepalive);

	strName = m_prefix +
			wxString::FromAscii("_filter");
	if (m_srv.getVariableString(strName, &str)) {
		readFilterFromString(&m_vscpfilter, str);
	}

	strName = m_prefix +
			wxString::FromAscii("_mask");
	if (m_srv.getVariableString(strName, &str)) {
		readMaskFromString(&m_vscpfilter, str);
	}

	// Close the channel
	m_srv.doCmdClose();


	// start the workerthread
	m_pthreadWork = new CWrkThread();
	if (NULL != m_pthreadWork) {
		m_pthreadWork->m_pObj = this;
		m_pthreadWork->Create();
		m_pthreadWork->Run();
	} else {
		rv = false;
	}

	return rv;
}


//////////////////////////////////////////////////////////////////////
// close
//

void
Cmqtt::close(void)
{
	// Do nothing if already terminated
	if (m_bQuit) return;

	m_bQuit = true; // terminate the thread
	wxSleep(1); // Give the thread some time to terminate

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
	bool bActivity;
	wxString str;
	int rv;

	mosqpp::lib_init();

	if (m_pObj->bSubscribe) {

		// S u b s c r i b e
		mqtt_subscribe *pSubscribe = new mqtt_subscribe(
				m_pObj->m_username,
				m_pObj->m_password,
				m_pObj->m_host,
				m_pObj->m_port,
				m_pObj->m_topic,
				m_pObj->m_hostMQTT,
				m_pObj->m_portMQTT,
				m_pObj->m_keepalive);

		while (!TestDestroy() && !m_pObj->m_bQuit) {

            vscpEvent *pEvent = new vscpEvent();
            if (NULL != pEvent) {
/*
                pEvent->pdata = new uint8_t[16 + frame.len];
                if (NULL == pEvent->pdata) {
                    delete pEvent;
                    continue;
                }

                // Interface GUID is set to all nulls as
                // this event should be sent to all clients.
                memset(pEvent->pdata, 0, 16);

                // GUID will be set to GUID of interface
                // by driver interface with LSB set to nickname
                memset(pEvent->GUID, 0, 16);
                pEvent->GUID[0] = frame.can_id & 0xff;

                // Set VSCP class + 512
                pEvent->vscp_class = getVSCPclassFromCANid(frame.can_id) + 512;

                // Set VSCP type
                pEvent->vscp_type = getVSCPtypeFromCANid(frame.can_id);

                // Copy data if any
                pEvent->sizeData = frame.len + 16;
                if (frame.len) {
                    memcpy(pEvent->pdata + 16, frame.data, frame.len);
                }
*/
                if (doLevel2Filter(pEvent, &m_pObj->m_vscpfilter)) {
                    m_pObj->m_mutexReceiveQueue.Lock();
                    //m_pObj->m_receiveQueue.Append(pEvent);
                    m_pObj->m_receiveList.push_back(pEvent);
                    m_pObj->m_semReceiveQueue.Post();
                    m_pObj->m_mutexReceiveQueue.Unlock();
                } else {
                    deleteVSCPevent(pEvent);
                }
            }

            rv = pSubscribe->loop();

            if (rv) {
                pSubscribe->reconnect();
            }

            ::wxMilliSleep(50);

		}
	} 
    else {
/*
		// Connect to the VSCP Server
		if (m_srv.doCmdOpen(m_pObj->m_host,
				m_pObj->m_port,
				m_pObj->m_username,
				m_pObj->m_password) <= 0) {
			syslog(LOG_ERR,
					"%s",
					(const char *) "mqtt_subscribe:Unable to connect to VSCP TCP/IP interface. Terminating!");
			return NULL;
		}

		// Find the channel id
		uint32_t ChannelID;
		m_srv.doCmdGetChannelID(&ChannelID);
 */ 
        
		// P u b l i s h
		mqtt_publish *pPublish = new mqtt_publish( m_pObj->m_username,
                                                        m_pObj->m_password,
                                                        m_pObj->m_host,
                                                        m_pObj->m_port,
                                                        m_pObj->m_topic,
                                                        m_pObj->m_hostMQTT,
                                                        m_pObj->m_portMQTT,
                                                        m_pObj->m_keepalive);

		while (!TestDestroy() && !m_pObj->m_bQuit) {

            if (wxSEMA_TIMEOUT == m_pObj->m_semSendQueue.WaitTimeout(300)) continue;

            if (m_pObj->m_sendList.size()) {

                m_pObj->m_mutexSendQueue.Lock();
                vscpEvent *pEvent = m_pObj->m_sendList.front();
                m_pObj->m_sendList.pop_front();
                m_pObj->m_mutexSendQueue.Unlock();

                if (NULL == pEvent) continue;

                //_pObj->writeEvent(&event);

                // We are done with the event - remove data if any
                if (NULL != pEvent->pdata) {
                    delete [] pEvent->pdata;
                    pEvent->pdata = NULL;
                }

            } // Event received

            rv = pPublish->loop();

            if (rv) {
                pPublish->reconnect();
            }

            ::wxMilliSleep(50);

        }
    }

	mosqpp::lib_cleanup();

	// Close the VSCP channel
	//m_srv.doCmdClose();

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


