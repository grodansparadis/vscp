// vscp_client_ws2.cpp
//
// Websocket if 2 client communication classes.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// Copyright:   (C) 2007-2020
// Ake Hedman, Grodans Paradis AB, <akhe@vscp.org>
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

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> 
#include <semaphore.h> 
#include <unistd.h> 
#include <vscp_aes.h>
#include <vscphelper.h>
#include "civetweb.h"

#include "vscp_client_ws2.h"

#include <json.hpp> // Needs C++11  -std=c++11

// for convenience
using json = nlohmann::json;


static int
ws2_client_data_handler(struct mg_connection *conn,
                              int flags,
                              char *data,
                              size_t data_len,
                              void *user_data)
{
	struct mg_context *ctx = mg_get_context(conn);	
	vscpClientWs2 *pObj =
	    (vscpClientWs2 *)mg_get_user_data(ctx);

	printf("Client received data from server: ");
	std::string str(data,data_len);
	printf("%s\n",str.c_str());

    // If type is event put in event queue or send
    // out on defined callbacks
    // If other type put in message queue

    json j;
    try {
        j = json::parse(str.c_str());
    }
    catch (...) {
        return 0;
    }

    if ( "event" == j["type"]) {

        if (pObj->isEvCallback()) {
            vscpEvent *pev = new vscpEvent;
            if ( NULL == pev ) return 0;
            std::string str = j["event"].dump();
            if ( !vscp_convertJSONToEvent(pev, str) ) return 0;
        }   
        else if (pObj->isExCallback()) {
            vscpEventEx *pex = new vscpEventEx;
            if ( NULL == pex ) return 0;
            std::string str = j["event"].dump();
            if ( !vscp_convertJSONToEventEx(pex, str) ) return 0;
        } 
        else {
            vscpEvent *pev = new vscpEvent;
            if ( NULL == pev ) return 0;

            std::string str = j["event"].dump();

            // Add to event queue
            pObj->m_eventReceiveQueue.push_back(pev);
        }
    }
    else {
	    pObj->m_msgReceiveQueue.push_back(j);
	    sem_post(&pObj->m_sem_msg);
    }

	return 1;
}

static void
ws2_client_close_handler(const struct mg_connection *conn,
                               void *user_data)
{
	struct mg_context *ctx = mg_get_context(conn);
	vscpClientWs2 *pObj =
	    (vscpClientWs2 *)mg_get_user_data(ctx);

    pObj->m_bConnected = false;
	printf("Client: Close handler\n");
}

///////////////////////////////////////////////////////////////////////////////
// CTOR
//

vscpClientWs2::vscpClientWs2()
{
    m_bConnected = false;
    m_conn = NULL;
    m_host = "localhost";
    m_port = 8884;

    sem_init(&m_sem_msg, 0, 0);
    mg_init_library(MG_FEATURES_SSL);
}

///////////////////////////////////////////////////////////////////////////////
// DTOR
//

vscpClientWs2::~vscpClientWs2()
{
    disconnect();

    while (m_eventReceiveQueue.size() ) {
        vscpEvent *pev = m_eventReceiveQueue.front();
        m_eventReceiveQueue.pop_front();
        vscp_deleteEvent_v2(&pev);
    }

    sem_destroy(&m_sem_msg);
}

///////////////////////////////////////////////////////////////////////////////
// init
//

int vscpClientWs2::init(const std::string host,
                        short port,
                        const std::string username,
                        const std::string password,
                        uint8_t* vscpkey,
                        uint32_t connection_timeout,
                        uint32_t response_timeout)
{
    m_bSSL = false;     // SSL/TLS not activated by default
    setConnectionTimeout(connection_timeout);
    setResponeTimeout(response_timeout);

    // Get iv
    vscp_getSalt(m_iv, 16);

    // Store encrypted password
    encrypt_password( m_credentials,
						username,
						password,
						vscpkey,
						m_iv );


    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// connect
//

int vscpClientWs2::connect(void) 
{
    char ebuf[100] = {0};
	const char *path = "/ws2";
	m_conn = mg_connect_websocket_client( m_host.c_str(),
	                                       m_port,
	                                       m_bSSL ? 1 : 0,
	                                       ebuf,
	                                       sizeof(ebuf),
	                                       path,
	                                       NULL,
	                                       ws2_client_data_handler,
	                                       ws2_client_close_handler,
	                                       this);

	if (NULL == m_conn) {
		printf("Error: %s\n", ebuf);
        return VSCP_TYPE_ERROR_CONNECTION;
	}

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// disconnect
//

int vscpClientWs2::disconnect(void)
{
    json cmd;
    cmd["type"] = "cmd";
	cmd["command"] = "close";
    cmd["args"] = nullptr;

    mg_websocket_client_write(m_conn,
		                          MG_WEBSOCKET_OPCODE_TEXT,
		                          cmd.dump().c_str(),
		                          cmd.dump().length());

    if ( VSCP_ERROR_SUCCESS != waitForResponse( WS2_RESPONSE_TIMEOUT ) ) {
		return VSCP_ERROR_TIMEOUT;
	}

    // Check return value
    json j = m_msgReceiveQueue.front();
    m_msgReceiveQueue.pop_front();
    if ( "+" != j["type"]) VSCP_ERROR_OPERATION_FAILED;

    mg_close_connection(m_conn);

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// isConnected
//

bool vscpClientWs2::isConnected(void)
{
    return m_bConnected;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int vscpClientWs2::send(vscpEvent &ev)
{
    json cmd;
    cmd["type"] = "event";
	cmd["event"]["head"] = ev.head;
    cmd["event"]["obid"] = ev.obid;
    cmd["event"]["timestamp"] = ev.timestamp;
    cmd["event"]["class"] = ev.vscp_class;
    cmd["event"]["type"] = ev.vscp_type;

    std::string dt;
    vscp_getDateStringFromEvent(dt,&ev);
    cmd["event"]["datetime"] = dt;
    
    std::string strGuid;
    vscp_writeGuidToString(strGuid,&ev);
    cmd["event"]["guid"] = strGuid;

    if ((NULL != ev.pdata) && ev.sizeData) {
        std::deque<uint8_t> data;
        for ( int i; i<ev.sizeData; i++ ) {
            data.push_back(ev.pdata[i]);
        }
        cmd["event"]["data"] = data;        
    }
    else {
        cmd["event"]["data"] = nullptr;    
    }

    mg_websocket_client_write(m_conn,
		                          MG_WEBSOCKET_OPCODE_TEXT,
		                          cmd.dump().c_str(),
		                          cmd.dump().length());

    if ( VSCP_ERROR_SUCCESS != waitForResponse( WS2_RESPONSE_TIMEOUT ) ) {
		printf("ERROR or TIMEOUT\n");
		return VSCP_ERROR_TIMEOUT;
	}

    // Check return value
    json j = m_msgReceiveQueue.front();
    m_msgReceiveQueue.pop_front();
    if ( "+" != j["type"]) VSCP_ERROR_OPERATION_FAILED;
    
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int vscpClientWs2::send(vscpEventEx &ex)
{
    json cmd;
    cmd["type"] = "event";
	cmd["event"]["head"] = ex.head;
    cmd["event"]["obid"] = ex.obid;
    cmd["event"]["timestamp"] = ex.timestamp;
    cmd["event"]["class"] = ex.vscp_class;
    cmd["event"]["type"] = ex.vscp_type;

    std::string dt;
    vscp_getDateStringFromEventEx(dt,&ex);
    cmd["event"]["datetime"] = dt;
    
    std::string strGuid;
    vscp_writeGuidToStringEx(strGuid,&ex);
    cmd["event"]["guid"] = strGuid;

    if (ex.sizeData) {
        std::deque<uint8_t> data;
        for ( int i; i<ex.sizeData; i++ ) {
            data.push_back(ex.data[i]);
        }
        cmd["event"]["data"] = data;        
    }
    else {
        cmd["event"]["data"] = nullptr;    
    }

    mg_websocket_client_write(m_conn,
		                          MG_WEBSOCKET_OPCODE_TEXT,
		                          cmd.dump().c_str(),
		                          cmd.dump().length());

    if ( VSCP_ERROR_SUCCESS != waitForResponse( WS2_RESPONSE_TIMEOUT ) ) {
		printf("ERROR or TIMEOUT\n");
		return VSCP_ERROR_TIMEOUT;
	}

    // Check return value
    json j = m_msgReceiveQueue.front();
    m_msgReceiveQueue.pop_front();
    if ( "+" != j["type"]) VSCP_ERROR_OPERATION_FAILED;

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int vscpClientWs2::receive(vscpEvent &ev)
{
    // check if there are anything to fetch
    if (!m_eventReceiveQueue.size()) {
        return VSCP_ERROR_RCV_EMPTY;
    }

    // only valid if no callback is defined
    if ((nullptr != m_evcallback) || (nullptr != m_excallback)) {
        return VSCP_ERROR_NOT_SUPPORTED;
    }

    vscpEvent *pev = m_eventReceiveQueue.front();
    m_eventReceiveQueue.pop_front();

    // Must be a avalid pointer
    if ( NULL == pev ) {
        return VSCP_ERROR_MEMORY;
    }

    vscp_copyEvent(&ev,pev);
    vscp_deleteEvent_v2(&pev);

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int vscpClientWs2::receive(vscpEventEx &ex)
{
    // check if there are anything to fetch
    if (!m_eventReceiveQueue.size()) {
        return VSCP_ERROR_RCV_EMPTY;
    }

    // only valid if no callback is defined
    if ((nullptr != m_evcallback) || (nullptr != m_excallback)) {
        return VSCP_ERROR_NOT_SUPPORTED;
    }

    vscpEvent *pev = m_eventReceiveQueue.front();
    m_eventReceiveQueue.pop_front();

    // Must be a avalid pointer
    if ( NULL == pev ) {
        return VSCP_ERROR_MEMORY;
    }

    vscp_convertEventToEventEx(&ex,pev);
    vscp_deleteEvent_v2(&pev);

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setfilter
//

int vscpClientWs2::setfilter(vscpEventFilter &filter)
{
    std::string strGUID;
    json cmd;

    cmd["type"] = "cmd";
	cmd["command"] = "setfilter";

    cmd["args"]["filter_priority"] = filter.filter_priority;
    cmd["args"]["filter_class"] = filter.filter_class;
    cmd["args"]["filter_type"] = filter.filter_type;
    vscp_writeGuidArrayToString(strGUID,filter.filter_GUID);
    cmd["args"]["filter_guid"] = strGUID;

    cmd["args"]["mask_priority"] = filter.mask_priority;
    cmd["args"]["mask_class"] = filter.mask_class;
    cmd["args"]["mask_type"] = filter.mask_type;
    vscp_writeGuidArrayToString(strGUID,filter.mask_GUID);
    cmd["args"]["mask_guid"] = strGUID;

    mg_websocket_client_write(m_conn,
		                          MG_WEBSOCKET_OPCODE_TEXT,
		                          cmd.dump().c_str(),
		                          cmd.dump().length());

    if ( VSCP_ERROR_SUCCESS != waitForResponse( WS2_RESPONSE_TIMEOUT ) ) {
		printf("ERROR or TIMEOUT\n");
		return VSCP_ERROR_TIMEOUT;
	}

    // Check return value
    json j = m_msgReceiveQueue.front();
    m_msgReceiveQueue.pop_front();
    if ( "+" != j["type"]) VSCP_ERROR_OPERATION_FAILED;

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getcount
//

int vscpClientWs2::getcount(uint16_t *pcount)
{
    if (NULL == pcount) return VSCP_ERROR_INVALID_POINTER;
    *pcount = m_eventReceiveQueue.size();
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// clear
//

int vscpClientWs2::clear(void) 
{
    while (m_eventReceiveQueue.size() ) {
        vscpEvent *pev = m_eventReceiveQueue.front();
        m_eventReceiveQueue.pop_front();
        vscp_deleteEvent_v2(&pev);
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getinterfaces
//

int vscpClientWs2::getinterfaces(std::deque<std::string> &iflist)
{
    std::string strGUID;
    json cmd;

    cmd["type"] = "cmd";
	cmd["command"] = "interfaces";

    cmd["args"] = nullptr;

    mg_websocket_client_write(m_conn,
		                          MG_WEBSOCKET_OPCODE_TEXT,
		                          cmd.dump().c_str(),
		                          cmd.dump().length());

    if ( VSCP_ERROR_SUCCESS != waitForResponse( WS2_RESPONSE_TIMEOUT ) ) {
		printf("ERROR or TIMEOUT\n");
		return VSCP_ERROR_TIMEOUT;
	}

    // Check return value
    json j = m_msgReceiveQueue.front();
    m_msgReceiveQueue.pop_front();
    if ( "+" != j["type"]) VSCP_ERROR_OPERATION_FAILED;

    std::deque<std::string> args = j["args"];
    iflist = args;

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getwcyd
//

int vscpClientWs2::getwcyd(uint64_t &wcyd)
{
    std::string strGUID;
    json cmd;

    cmd["type"] = "cmd";
	cmd["command"] = "wcyd";

    cmd["args"] = nullptr;

    mg_websocket_client_write(m_conn,
		                          MG_WEBSOCKET_OPCODE_TEXT,
		                          cmd.dump().c_str(),
		                          cmd.dump().length());

    if ( VSCP_ERROR_SUCCESS != waitForResponse( WS2_RESPONSE_TIMEOUT ) ) {
		printf("ERROR or TIMEOUT\n");
		return VSCP_ERROR_TIMEOUT;
	}

    // Check return value
    json j = m_msgReceiveQueue.front();
    m_msgReceiveQueue.pop_front();
    if ( "+" != j["type"]) VSCP_ERROR_OPERATION_FAILED;

    std::deque<std::string> args = j["args"];
    if (!args.size()) {
        return VSCP_ERROR_PARAMETER;
    }

    wcyd = j["args"][0];

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setCallback
//

int vscpClientWs2::setCallback(vscpEvent &ev)
{
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setCallback
//

int vscpClientWs2::setCallback(vscpEventEx &ex)
{
    return VSCP_ERROR_SUCCESS;
}

int vscpClientWs2::encrypt_password(std::string& strout,
							        std::string struser,
							        std::string strpassword,
							        uint8_t *vscpkey,
							        uint8_t *iv) 
{
	uint8_t buf[200]; 
	
	std::string strCombined;
	strCombined = struser;
	strCombined += ":";
	strCombined += strpassword;
	
	AES_CBC_encrypt_buffer(AES128, 
							buf,
							(uint8_t *)strCombined.c_str(), 
							strCombined.length(), 
							vscpkey, 
							iv);	
    
    char out[50];                            					
	vscp_byteArray2HexStr(out, buf, 16);
	strout = out;

	return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// waitForResponse
//

int vscpClientWs2::waitForResponse( uint32_t timeout )
{	
	time_t ts;
	time(&ts);

	struct timespec to;
	to.tv_nsec = 0;
	to.tv_sec = ts + timeout/1000;

	if ( -1 == sem_timedwait(&m_sem_msg,&to) ) {
		
		switch(errno) {

			case EINTR:
				return VSCP_ERROR_INTERUPTED;

			case EINVAL:
				return VSCP_ERROR_PARAMETER;

			case EAGAIN:
				return VSCP_ERROR_ERROR;

			case ETIMEDOUT:	
			default:
				return VSCP_ERROR_TIMEOUT;
		}

	}

	return VSCP_ERROR_SUCCESS;
}