// mqttobj.cpp: implementation of the CMQTT class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP Project (http://www.vscp.org) 
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

#include <mongoose.h>

#include <vscphelper.h>
#include <vscpremotetcpif.h>
#include <vscp_type.h>
#include <vscp_class.h>
#include "mqttobj.h"


////////////////////////////////////////////////////////////////////////////////
// ev_handler
//

static void ev_handler( struct mg_connection *nc, int ev, void *p )
{
    struct mg_mqtt_message *msg = ( struct mg_mqtt_message * )p;
    Cmqttobj *pmqttobj = ( Cmqttobj *)nc->mgr->user_data;

    switch ( ev ) {

        case MG_EV_CONNECT:
        {
            struct mg_send_mqtt_handshake_opts opts;
            
            opts.flags = MG_MQTT_CLEAN_SESSION;
            opts.keep_alive = pmqttobj->m_keepalive;
            
            // Username
            wxCharBuffer bufferedUserName = pmqttobj->m_username.ToUTF8();            
            if ( pmqttobj->m_username.IsEmpty() ) {
                opts.user_name = NULL;
            }
            else {
                opts.user_name = bufferedUserName;
                opts.flags |= MG_MQTT_HAS_USER_NAME;
            }
            
            // Password
            wxCharBuffer bufferedPassword = pmqttobj->m_password.ToUTF8();
            if ( pmqttobj->m_password.IsEmpty() ) {
                opts.password = NULL;
            }
            else {
                opts.password = bufferedPassword;
                opts.flags |= MG_MQTT_HAS_PASSWORD;
            }
            
            mg_set_protocol_mqtt( nc );
        
            if ( pmqttobj->m_bSubscribe ) {
            
                
                opts.password = NULL;
                opts.user_name = NULL;
                
            }
            else {
                
                opts.flags = MG_MQTT_CLEAN_SESSION;
                opts.keep_alive = 60;
                opts.password = NULL;
                opts.user_name = NULL;
                
                //mg_send_mqtt_handshake( nc, pmqttobj->m_sessionid.mbc_str() );
                
            }
            
            mg_send_mqtt_handshake_opt( nc, 
                                            pmqttobj->m_sessionid.mbc_str(), 
                                            opts );
        
#ifndef WIN32
        syslog(LOG_INFO,
                "VSCP MQTT Driver - Connection opened.\n" );
#endif   

        }       
        break;

        case MG_EV_MQTT_CONNACK:
            if ( msg->connack_ret_code != MG_EV_MQTT_CONNACK_ACCEPTED ) {
#ifndef WIN32
                syslog(LOG_ERR,
                    "VSCP MQTT Driver - Got MQTT connection error: %d\n",
                    msg->connack_ret_code );
#endif 
                pmqttobj->m_bQuit = true;
                return;
            }
            
            pmqttobj->m_bConnected = true;
            
            if ( pmqttobj->m_bSubscribe ) {
                mg_mqtt_subscribe( nc, pmqttobj->m_topic_list, 1, 42 );
            }
            break;

        case MG_EV_MQTT_PUBACK:
            break;

        case MG_EV_MQTT_SUBACK:
            break;

        case MG_EV_MQTT_PINGREQ:
            mg_mqtt_pong( nc );
            break;

        // Incoming message
        case MG_EV_MQTT_PUBLISH:
        {            
            vscpEventEx eventEx;
            
            eventEx.obid = 0;
            eventEx.timestamp = 0;
            eventEx.head = VSCP_PRIORITY_NORMAL;
            memset( eventEx.GUID, 0, 16 );

            if ( !strcmp( msg->topic, 
                            (const char *)pmqttobj->m_topic.mbc_str() ) ) {

                char valbuf[512];
                memset( valbuf, 0, sizeof( valbuf ) );
                memcpy( valbuf, msg->payload.p, MIN( 511, msg->payload.len ) );
                wxString str = 
                    wxString::FromAscii( valbuf );
                    
                if ( pmqttobj->m_bSimplify ) {
                
                        // Here the data will only be a value (Simple)
                                                
                        eventEx.vscp_class = pmqttobj->m_simple_vscpclass;
                        eventEx.vscp_type = pmqttobj->m_simple_vscptype;
                    
                        switch( pmqttobj->m_simple_vscpclass ) {
                         
                            case VSCP_CLASS2_MEASUREMENT_STR:
                            
                                // Sensor index
                                eventEx.data[ 0 ] = pmqttobj->m_simple_sensorindex;
                                
                                // Zone
                                eventEx.data[ 1 ] = pmqttobj->m_simple_zone;
                                
                                // Subzone
                                eventEx.data[ 2 ] = pmqttobj->m_simple_subzone;
                                
                                // Unit
                                eventEx.data[ 3 ] = pmqttobj->m_simple_unit;
                                
                                // Value can have max 7 characters                
                                if ( str.Length() > ( VSCP_LEVEL2_MAXDATA - 4 ) ) {
                                    str.Truncate( VSCP_LEVEL2_MAXDATA - 4 );
                                }
                                
                                // Set size
                                eventEx.sizeData = str.Length() + 4;
                                
                                // Copy in string data
                                memcpy( eventEx.data + 4, 
                                            str.mbc_str(),
                                            str.Length() );
                                
                                break;
                                
                            case VSCP_CLASS2_MEASUREMENT_FLOAT:
                                {
                                    // Sensor index
                                    eventEx.data[ 0 ] = pmqttobj->m_simple_sensorindex;
                                
                                    // Zone
                                    eventEx.data[ 1 ] = pmqttobj->m_simple_zone;
                                
                                    // Subzone
                                    eventEx.data[ 2 ] = pmqttobj->m_simple_subzone;
                                
                                    // Unit
                                    eventEx.data[ 3 ] = pmqttobj->m_simple_unit;
                                    
                                    double val;
#if  wxMAJOR_VERSION < 3                                    
                                    char *pNext;                                    
                                    val = strtod( str.mbc_str(), &pNext );
#else                                    
                                    str.ToCDouble( &val );
#endif                                    
                                    uint8_t *p = (uint8_t *)&val;
                                    
                                    if ( wxIsPlatformLittleEndian() ) {
                                        
                                        for ( int i=7; i>0; i--) {
                                            eventEx.data[ 4+7-i ] = *(p+i);
                                        }
                                        
                                    }
                                    else {
                                        memcpy ( eventEx.data + 4, p, 8 );
                                    }
                                    
                                    // Set data size
                                    eventEx.sizeData = 8 + 4;
                                    
                                }
                                break; 

                            default:
                            case VSCP_CLASS1_MEASUREMENT:
                            
                                if ( VSCP_DATACODING_STRING == 
                                                pmqttobj->m_simple_coding ) {
                                                    
                                    // * * * Present as string  * * * 

                                    // Coding                    
                                    eventEx.data[ 0 ] = 
                                            VSCP_DATACODING_STRING |
                                            ( ( pmqttobj->m_simple_unit << 3 ) & 
                                                    VSCP_MASK_DATACODING_UNIT ) |
                                            ( pmqttobj->m_simple_sensorindex & 
                                                    VSCP_MASK_DATACODING_INDEX );
                                    
                                    // Set string data
                            
                                    // Value can have max 7 characters                
                                    if ( str.Length() > 7 ) str.Truncate( 7 );
                                    
                                    memcpy( eventEx.data + 1, 
                                                str.mbc_str(), 
                                                str.Length() );
                                    
                                    // Set size
                                    eventEx.sizeData = str.Length() + 1;
                                    
                                }
                                else {
                                    
                                    // * * * Present as single precision floating  
                                    // point number * * *
                                    float val = atof( str.mbc_str() );
                                    
                                    wxUINT32_SWAP_ON_LE( val );
                                    memcpy( eventEx.data + 1, (uint8_t *)&val, 4 );
                                    
                                    eventEx.data[ 0 ] = VSCP_DATACODING_SINGLE |
                                            ( ( pmqttobj->m_simple_unit << 3 ) & 
                                                    VSCP_MASK_DATACODING_UNIT ) |
                                            ( pmqttobj->m_simple_sensorindex & 
                                                    VSCP_MASK_DATACODING_INDEX );
                                    
                                    // Set size
                                    eventEx.sizeData = 5; // coding + 32-bit value
                                    
                                }
                                break;
                                
                        }           

                        goto FEED_EVENT;
                    
                }
                else {
                    
                    if ( vscp_setVscpEventExFromString( &eventEx, str ) ) {

FEED_EVENT:                        
                        
                        vscpEvent *pEvent = new vscpEvent;
                        if ( NULL != pEvent ) {

                            pEvent->sizeData = 0;
                            pEvent->pdata = NULL;

                            if ( vscp_doLevel2FilterEx( &eventEx, 
                                                    &pmqttobj->m_vscpfilter ) ) {

                                if ( vscp_convertVSCPfromEx( pEvent, &eventEx ) ) {
                                    pmqttobj->m_mutexReceiveQueue.Lock();
                                    pmqttobj->m_receiveList.push_back( pEvent );
                                    pmqttobj->m_semReceiveQueue.Post();
                                    pmqttobj->m_mutexReceiveQueue.Unlock();
                                }
                                
                            }
                            else {
                                vscp_deleteVSCPevent( pEvent );
                            }
                        }
                    }   
                
                }
            }
        }
        break;

        case MG_EV_CLOSE:
        
#ifndef WIN32
        syslog(LOG_INFO,
                "VSCP MQTT Driver - Connection closed.\n" );
#endif           
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
    
    // Simple
    m_bSimplify = false;
    m_simple_vscpclass = VSCP_CLASS1_MEASUREMENT;
    m_simple_vscptype = 0;
    m_simple_coding = 0;
    m_simple_unit = 0;
    m_simple_sensorindex = 0;
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
    // *** All variables must be dynamic because of multientrance
    
    wxString *pwrkstr = new wxString;
    assert( NULL != pwrkstr ); 
    
    wxString *pstrName = new wxString;
    assert( NULL != pstrName );

    m_username = wxString::FromAscii(pUsername);
    m_password = wxString::FromAscii(pPassword);
    m_host = wxString::FromAscii(pHost);
    m_prefix = wxString::FromAscii(pPrefix);

    // Parse the configuration string. It should
    // have the following form
    // "sessionid";“subscribe”|”publish”;channel;host:port;user;password;keepalive;filter;mask
    // 
    wxStringTokenizer *ptkz = 
            new wxStringTokenizer(wxString::FromAscii(pConfig), _(";"));
    assert( NULL != ptkz );
    
    // Session id
    if ( ptkz->HasMoreTokens() ) {
        m_sessionid = ptkz->GetNextToken();
    }

    // Check if we should publish or subscribe
    if ( ptkz->HasMoreTokens() ) {
        // Check for subscribe/publish
        *pwrkstr = ptkz->GetNextToken();
        pwrkstr->Trim();
        pwrkstr->Trim(false);
        pwrkstr->MakeUpper();
        if ( wxNOT_FOUND != pwrkstr->Find( _("PUBLISH") ) ) {
            m_bSubscribe = false;
        }
    }

    // Get topic from configuration string
    if ( ptkz->HasMoreTokens() ) {
        m_topic = ptkz->GetNextToken();
    }

    // Get MQTT host from configuration string
    if ( ptkz->HasMoreTokens() ) {
        m_hostMQTT = ptkz->GetNextToken();
    }

    // Get MQTT user from configuration string
    if ( ptkz->HasMoreTokens() ) {
        m_usernameMQTT = ptkz->GetNextToken();
    }

    // Get MQTT password from configuration string
    if ( ptkz->HasMoreTokens() ) {
        m_passwordMQTT = ptkz->GetNextToken();
    }

    // Get MQTT keep alive from configuration string
    if ( ptkz->HasMoreTokens() ) {
        m_keepalive = vscp_readStringValue( ptkz->GetNextToken() );
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
    m_srv.doCmdGetChannelID( &ChannelID );

    // The server should hold configuration data for each sensor
    // we want to monitor.
    // 
    // We look for 
    //
    //   _sessionid - Unique id for MQTT session, e.g. "session2"
    //
    //   _type   - “subscribe” to subscribe to a MQTT topic. ”publish” to 
    //              publish events to a MQTT topic. Defaults to “subscribe”.
    //
    //   _topic - This is a text string identifying the topic. It is 
    //              recommended that this string starts with “vscp/”. 
    //              Defaults to “vscp”
    //
    //   _qos   - MQTT QOS value. Defaults to 0. 
    //
    //   _host  - IP address + port or a DNS resolvable address + port on the 
    //              form host:port to the remote host. 
    //              Mandatory and must be declared either in the configuration 
    //              string or in this variable. Defaults to “localhost:1883”
    //
    //   _user - Username used to log in on the remote sever. 
    //              Defaults to empty. 
    //
    //   _password - Password used to login on the remote server. 
    //              Defaults to empty.
    //
    //   _keepalive - Keepalive value for channel. Defaults to 60.
    //
    //   _filter - Standard VSCP filter in string form. 
    //                 1,0x0000,0x0006,
    //                 ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00
    //              as priority,class,type,GUID
    //              Used to filter what events that is received from 
    //              the mqtt interface. If not give all events 
    //              are received.
    //   _mask - Standard VSCP mask in string form.
    //                 1,0x0000,0x0006,
    //                 ff:ff:ff:ff:ff:ff:ff:01:00:00:00:00:00:00:00:00
    //              as priority,class,type,GUID
    //              Used to filter what events that is received from 
    //              the mqtt interface. If not give all events 
    //              are received. 
    //
    //   _simple - If available simlicty will be enabled which makes it 
    //              possible to send just numbers over MQTT but still get
    //              valid events into the system. The functionality works the 
    //              other way around to so measurement events can sen data
    //              over MQTT as just a number possibly using the topic as
    //              a way to tell what is sent.
    
    *pstrName = m_prefix +
                wxString::FromAscii("_sessionid");
    if ( VSCP_ERROR_SUCCESS == m_srv.getVariableString( *pstrName, pwrkstr ) ) {
        m_sessionid = *pwrkstr;
    }
    
    *pstrName = m_prefix +
                    wxString::FromAscii("_type");
    m_srv.getVariableString( *pstrName, pwrkstr );

    // Check for subscribe/publish
    pwrkstr->Trim();
    pwrkstr->Trim(false);
    pwrkstr->MakeUpper();
    if ( wxNOT_FOUND != pwrkstr->Find(_("publish") ) ) {
        m_bSubscribe = false;
    }

    *pstrName = m_prefix +
                wxString::FromAscii("_topic");
    if ( VSCP_ERROR_SUCCESS == m_srv.getVariableString( *pstrName, pwrkstr ) ) {
        m_topic = *pwrkstr;
    }

    *pstrName = m_prefix +
                wxString::FromAscii("_host");
    if ( VSCP_ERROR_SUCCESS == m_srv.getVariableString( *pstrName, &m_hostMQTT ) ) {
        m_hostMQTT = *pwrkstr;
    }

    *pstrName = m_prefix +
                wxString::FromAscii("_username");
    if ( VSCP_ERROR_SUCCESS == m_srv.getVariableString( *pstrName, pwrkstr ) ) {
        m_usernameMQTT = *pwrkstr;
    }

    *pstrName = m_prefix +
                wxString::FromAscii("_password");
    if ( VSCP_ERROR_SUCCESS == m_srv.getVariableString( *pstrName, &m_passwordMQTT ) ) {
        m_passwordMQTT = *pwrkstr;
    }

    *pstrName = m_prefix +
                wxString::FromAscii("_keepalive");
    int * pint = new int;
    assert( NULL != pint );
    if ( VSCP_ERROR_SUCCESS == m_srv.getVariableInt( *pstrName, pint ) ) {
        m_keepalive = *pint;
    }

    *pstrName = m_prefix +
        wxString::FromAscii( "_qos" );
    if ( VSCP_ERROR_SUCCESS == m_srv.getVariableInt( *pstrName, pint ) ) {
        m_topic_list[ 0 ].qos = *pint;
    }
    
    delete pint;

    *pstrName = m_prefix +
                wxString::FromAscii("_filter");
    if ( VSCP_ERROR_SUCCESS == m_srv.getVariableString( *pstrName, pwrkstr ) ) {
        vscp_readFilterFromString(&m_vscpfilter, *pwrkstr );
    }

    *pstrName = m_prefix +
                wxString::FromAscii("_mask");
    if ( VSCP_ERROR_SUCCESS == m_srv.getVariableString( *pstrName, pwrkstr ) ) {
        vscp_readMaskFromString(&m_vscpfilter, *pwrkstr );
    }
    
    // Type = 0
    // ====================================================================
    // Send CLASS1.MEASUREMENT  float or string
    // Coding: 10,float|string,vscp-type,sensorindex(0-7),unit(0-7)
    //
    // Type = 1
    // ====================================================================
    // Send CLASS2.MEASUREMENT.STR
    // Coding: 1040,vscp-type,sensorindex(0-255), unit (0-255), zone(0-255), subzone(0-255)
    //
    // Type = 2
    // ====================================================================
    // Send CLASS2.MEASUREMENT.FLOAT
    // Coding: 1060,vscp-type,sensorindex(0-255), unit (0-255), zone(0-255), subzone(0-255)
    //
    
    *pstrName = m_prefix +
                wxString::FromAscii("_simplify");
    if ( VSCP_ERROR_SUCCESS == 
                        m_srv.getVariableString( *pstrName, pwrkstr ) ) {
        m_simplify = *pwrkstr;
    }
    
    if ( m_simplify.Length() ) {
        
        m_bSimplify = true; 
        wxStringTokenizer tkzSimple( m_simplify, _(","));
        
        // simple type
        if ( tkzSimple.HasMoreTokens() ) {
            m_simple_vscpclass = 
                            vscp_readStringValue(tkzSimple.GetNextToken());
        }
        
        switch ( m_simple_vscpclass ) {

            case VSCP_CLASS2_MEASUREMENT_STR:
            
                // simple vscp-type
                if ( tkzSimple.HasMoreTokens() ) {
                    m_simple_vscptype = 
                        vscp_readStringValue(tkzSimple.GetNextToken());
                }
                
                // simple sensorindex
                if ( tkzSimple.HasMoreTokens() ) {
                    m_simple_sensorindex = 
                        vscp_readStringValue(tkzSimple.GetNextToken());
                }
                
                // simple unit
                if ( tkzSimple.HasMoreTokens() ) {
                    m_simple_unit = 
                        vscp_readStringValue(tkzSimple.GetNextToken());
                }
                
                // simple zone
                if ( tkzSimple.HasMoreTokens() ) {
                    m_simple_zone = 
                                vscp_readStringValue(tkzSimple.GetNextToken());
                }
        
                // simple subzone
                if (tkzSimple.HasMoreTokens()) {
                    m_simple_subzone = 
                                vscp_readStringValue(tkzSimple.GetNextToken());
                }
                
                break;
            
    
            case VSCP_CLASS2_MEASUREMENT_FLOAT:
            
                // simple vscp-type
                if ( tkzSimple.HasMoreTokens() ) {
                    m_simple_vscptype = 
                        vscp_readStringValue(tkzSimple.GetNextToken());
                }
            
                
                // simple sensorindex
                if ( tkzSimple.HasMoreTokens() ) {
                    m_simple_sensorindex = 
                        vscp_readStringValue(tkzSimple.GetNextToken());
                }
                
                // simple unit
                if ( tkzSimple.HasMoreTokens() ) {
                    m_simple_unit = 
                        vscp_readStringValue(tkzSimple.GetNextToken());
                }
                
                // simple zone
                if ( tkzSimple.HasMoreTokens() ) {
                    m_simple_zone = 
                                vscp_readStringValue(tkzSimple.GetNextToken());
                }
        
                // simple subzone
                if (tkzSimple.HasMoreTokens()) {
                    m_simple_subzone = 
                                vscp_readStringValue(tkzSimple.GetNextToken());
                }
                
                break;    
            
            case VSCP_CLASS1_MEASUREMENT:
            default:
            
                // simple vscp-type
                if ( tkzSimple.HasMoreTokens() ) {
                    m_simple_vscptype = 
                        vscp_readStringValue(tkzSimple.GetNextToken());
                }
                
                // Coding
                if ( tkzSimple.HasMoreTokens() ) {
                    wxString strcoding = tkzSimple.GetNextToken();
                    strcoding.Trim();
                    pwrkstr->Trim(false);
                    pwrkstr->MakeUpper();
                    m_simple_coding = VSCP_DATACODING_STRING;
                    if ( wxNOT_FOUND != pwrkstr->Find(_("FLOAT") ) ) {
                        m_simple_coding = VSCP_DATACODING_SINGLE;
                    }
                }
                
                // simple sensorindex
                if ( tkzSimple.HasMoreTokens() ) {
                    m_simple_sensorindex = 
                        vscp_readStringValue(tkzSimple.GetNextToken());
                }
                if ( m_simple_sensorindex > 7 ) m_simple_sensorindex = 7;
                
                // simple unit
                if ( tkzSimple.HasMoreTokens() ) {
                    m_simple_unit = 
                        vscp_readStringValue(tkzSimple.GetNextToken());
                }
                if ( m_simple_unit > 7 ) m_simple_unit = 7;
                
                break;
        }
        
        
    }
    else {
        m_bSimplify = false;
    }

    if ( m_bSubscribe ) {
        // QOS set from variable read or constructor
        m_topic_list[ 0 ].topic = new char( m_topic.Length() + 1 );
        assert( NULL != m_topic_list[ 0 ].topic ); 
        if ( NULL != m_topic_list[ 0 ].topic ) {
            memset( (void *)m_topic_list[ 0 ].topic, 
                                0, 
                                m_topic.Length() + 1 );
            memcpy( (void *)m_topic_list[ 0 ].topic, 
                                m_topic.mbc_str(), 
                                m_topic.Length() );
        }
    }
    
    // Close the channel
    m_srv.doCmdClose();
    
    // Clean up allocations
    delete pwrkstr;
    delete pstrName;
    delete ptkz;

    // start the worker thread
    m_pthreadWork = new CWrkThread();
    if (NULL != m_pthreadWork) {
        m_pthreadWork->m_pObj = this;
        m_pthreadWork->Create();
        m_pthreadWork->Run();
    }
    else {
        return false;
    }

    
    return true;
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
    wxSleep(1);	    // Give the thread some time to terminate
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
    int cnt_poll = 0;
    wxString str;
    struct mg_connection *nc;
    struct mg_mgr *pmgr = new mg_mgr;
    assert( NULL != pmgr );
    uint16_t msgid = 0;

    //mgr.user_data = m_pObj;
    mg_mgr_init( pmgr, m_pObj );

    if ( NULL == ( nc = mg_connect( pmgr, 
                                    (const char *)m_pObj->m_hostMQTT.mbc_str(), 
                                    ev_handler ) ) ) {
#ifdef DEBUG                                        
        fprintf( stderr, 
                    "ns_connect(%s) failed\n", 
                    (const char *)m_pObj->m_hostMQTT.mbc_str() );
#endif  
#ifndef WIN32
        syslog(LOG_INFO,
                "VSCP MQTTT Driver - ns_connet failed\n" );
#endif                  
        return NULL;
    }

    if ( m_pObj->m_bSubscribe ) {

        while ( !TestDestroy() && !m_pObj->m_bQuit ) {
            
                
            mg_mgr_poll( pmgr, 100 );
            cnt_poll++;
            
            
            // Keep the connection alive
            if ( cnt_poll > 600 ) {
                mg_mqtt_pong( nc );
                cnt_poll = 0;
            }
            
            if ( m_pObj->m_bConnected  ) {    
            
            }
            else {
                
                // Give system time to connect
                //wxMilliSleep( 1000 );
                
                // Try to connect again
                //nc = ns_connect( pmgr, 
                //                    (const char *)m_pObj->m_hostMQTT.mbc_str(), 
                //                    ev_handler );
                
                
                //ns_mqtt_subscribe( nc, m_pObj->m_topic_list, 1, 42 );
                
            }
        }

    }
    // Publish
    else {
        
        while ( !TestDestroy() && !m_pObj->m_bQuit ) {

            mg_mgr_poll( pmgr, 100 );

            // Wait for connection
            if ( !m_pObj->m_bConnected ) {
                continue;
            }

            if ( wxSEMA_TIMEOUT == 
                    m_pObj->m_semSendQueue.WaitTimeout( 10 ) ) continue;            

            if ( m_pObj->m_sendList.size() ) {

                m_pObj->m_mutexSendQueue.Lock();
                vscpEvent *pEvent = m_pObj->m_sendList.front();
                m_pObj->m_sendList.pop_front();
                m_pObj->m_mutexSendQueue.Unlock();
                if (NULL == pEvent) continue;
                
                // If simple there must also be data
                if ( m_pObj->m_bSimplify && ( NULL != pEvent->pdata ) ) {
                                    
                    switch( m_pObj->m_simple_vscpclass ) {
                        
                        case VSCP_CLASS2_MEASUREMENT_STR:
                        {
                            if ( VSCP_CLASS2_MEASUREMENT_STR != pEvent->vscp_class ) {
                                break;
                            }

                            // There must be at least one 
                            // character in the string
                            if ( pEvent->sizeData < 5 ) {
                                break;
                            }
                            
                            // Sensor index must be the same
                            if ( m_pObj->m_simple_sensorindex != 
                                pEvent->pdata[ 0 ] ) {
                                break;
                            }
                                
                            // Zone must be the same
                            if ( m_pObj->m_simple_zone != 
                                pEvent->pdata[ 1 ] ) {
                                break;
                            }
                                
                            // Subzone must be the same
                            if ( m_pObj->m_simple_zone != 
                                pEvent->pdata[ 2 ] ) {
                                break;
                            }
                                
                            // Unit must be the same
                            if ( m_pObj->m_simple_unit != 
                                pEvent->pdata[ 3 ] ) {
                                break;
                            }
                            
                            char buf[512];
                            memset( buf, 0, sizeof( buf ) );
                            memcpy( buf, pEvent->pdata + 4, pEvent->sizeData - 4 );
                            str = wxString::FromAscii( buf );
                            
                            goto PUBLISH;
                        }    
                        break;
                                
                        case VSCP_CLASS2_MEASUREMENT_FLOAT:
                        {
                            // There must be place for the  
                            // double in the data 4 + 8
                            if ( pEvent->sizeData < 12 ) {
                                break;
                            }
                            
                            // Sensor index must be the same
                            if ( m_pObj->m_simple_sensorindex != 
                                pEvent->pdata[ 0 ] ) {
                                break;
                            }
                                
                            // Zone must be the same
                            if ( m_pObj->m_simple_zone != 
                                pEvent->pdata[ 1 ] ) {
                                break;
                            }
                                
                            // Subzone must be the same
                            if ( m_pObj->m_simple_zone != 
                                pEvent->pdata[ 2 ] ) {
                                break;
                            }
                                
                            // Unit must be the same
                            if ( m_pObj->m_simple_unit != 
                                pEvent->pdata[ 3 ] ) {
                                break;
                            }
                            
                            uint8_t *p = pEvent->pdata + 4;
                            if ( wxIsPlatformLittleEndian() ) {
                                for ( int i=7; i>0; i--) {
                                    pEvent->pdata[ 4 + 7 - i ] = *(p+i);
                                }
                            }
                            
                            double val = *( (double *)( pEvent->pdata + 4 ) );
                            char buf[80];
                            sprintf( buf, "%g", val );
                            str = wxString::FromAscii( buf );
                                    
                            goto PUBLISH;
                        
                        }
                        break;    
                            
                        default:
                        case VSCP_CLASS1_MEASUREMENT:
                        {    
                            // Sensor index must be the same
                            if ( m_pObj->m_simple_sensorindex != 
                                VSCP_DATACODING_INDEX( pEvent->pdata[ 0 ] ) ) {
                                break;
                            }
                                
                            // Unit must be the same
                            if ( m_pObj->m_simple_unit != 
                                VSCP_DATACODING_UNIT( pEvent->pdata[ 0 ] ) ) {
                                break;
                            }
                            
                            //vscp_getVSCPMeasurementAsString( pEvent, str );
                            vscp_writeVscpEventToString( pEvent, str );
                            goto PUBLISH;
                            
                        /*    
                            if ( VSCP_DATACODING_SINGLE == 
                                ( pEvent->pdata[ 0 ] & VSCP_MASK_DATACODING_TYPE ) ) {
                                        
                                wxUINT32_SWAP_ON_LE( *( (uint32_t *)( pEvent->pdata + 1 ) ) );                                
                                float val = *( (float *)( pEvent->pdata + 1 ) );
                                char buf[80];
                                sprintf( buf, "%f", val );
                                str = wxString::FromAscii( buf );
                                    
                                goto PUBLISH;
                            }
                            else {  // STRING
                                
                                char buf[ 10 ];
                                memset( buf, 0, sizeof( buf ) );
                                if ( pEvent->sizeData >= 8 ) {
                                    memcpy( buf, pEvent->pdata + 1, 7 );
                                }
                                else {
                                    memcpy( buf, pEvent->pdata + 1, pEvent->sizeData - 1 );                                        
                                }
                                int ttt = pEvent->pdata[1];
                                ttt = pEvent->pdata[2];
                                ttt = pEvent->pdata[3];
                                str = wxString::FromAscii( buf );
                                goto PUBLISH;
                                    
                            }
                        */
                         
                        }
                        break;
                            
                    } // switch
                    
                }
                else {
                    
                    vscp_writeVscpEventToString( pEvent, str );
                    
PUBLISH:                    
                    
                    mg_mqtt_publish( nc, 
                                        m_pObj->m_topic.mbc_str(), 
                                        msgid++, 
                                        MG_MQTT_QOS( m_pObj->m_topic_list[0].qos ), 
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

    // Disconnect if we are connected.
    if ( m_pObj->m_bConnected ) {
        mg_mqtt_disconnect( nc );
    }
    
    delete pmgr;
    pmgr = NULL;

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

