// vscp_client_ws1.cpp
//
// Websocket if 1 client communication classes.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// Copyright:   (C) 2007-2021
// Ake Hedman, the VSCP project, <info@vscp.org>
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

#ifdef WIN32
#include <pch.h>
#endif

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> 
#include <semaphore.h> 
#ifndef WIN32
#include <unistd.h>
#endif
#include <vscp_aes.h>
#include <vscphelper.h>
#include "civetweb.h"

#include "vscp_client_ws1.h"


static int
ws1_client_data_handler(struct mg_connection *conn,
                              int flags,
                              char *data,
                              size_t data_len,
                              void *user_data)
{
	struct mg_context *ctx = mg_get_context(conn);	
	vscpClientWs1 *pObj =
	    (vscpClientWs1 *)mg_get_user_data(ctx);

    printf("--------------------------------------------------->\n");    

  // We may get some different message types (websocket opcodes).
	// We will handle these messages differently. 
	bool isText = ((flags & 0xf) == MG_WEBSOCKET_OPCODE_TEXT);
	bool isBin = ((flags & 0xf) == MG_WEBSOCKET_OPCODE_BINARY);
	bool isPing = ((flags & 0xf) == MG_WEBSOCKET_OPCODE_PING);
	bool isPong = ((flags & 0xf) == MG_WEBSOCKET_OPCODE_PONG);
	bool isClose = ((flags & 0xf) == MG_WEBSOCKET_OPCODE_CONNECTION_CLOSE); 

    // Check if we got a websocket PING request 
	if (isPing) {
		// PING requests are to check if the connection is broken.
		// They should be replied with a PONG with the same data.
		//
        printf("Ping\n");
		mg_websocket_client_write(pObj->m_conn,
		                            MG_WEBSOCKET_OPCODE_PONG,
		                            data,
		                            data_len);
		return 1;
	}

	// Check if we got a websocket PONG message 
	if (isPong) {
		// A PONG message may be a response to our PING, but
		// it is also allowed to send unsolicited PONG messages
		// send by the server to check some lower level TCP
		// connections. Just ignore all kinds of PONGs. 
        printf("Pong\n");
		return 1;
	}   

    // It we got a websocket TEXT message, handle it ... 
	if (isText) {

        printf("Client received data from server: ");
        std::string str(data,data_len);
        vscp_trim(str);
        printf("%s\n",str.c_str());

        // If type is event put in event queue or send
        // out on defined callbacks
        // If other type put in message queue

        

        if ( 'E' == str[0] ) {

            if (pObj->isEvCallback()) {
                vscpEvent *pev = new vscpEvent;
                if ( NULL == pev ) return 0;
                if ( !vscp_convertStringToEvent(pev, str) ) return 1;
                pObj->m_evcallback(pev, pObj->m_callbackObject);
            }   
            else if (pObj->isExCallback()) {
                vscpEventEx *pex = new vscpEventEx;
                if ( NULL == pex ) return 0;
                if ( !vscp_convertStringToEventEx(pex, str) ) return 1;
                pObj->m_excallback(pex, pObj->m_callbackObject);
            } 
            else {
                vscpEvent *pev = new vscpEvent;
                if ( NULL == pev ) return 0;
                if ( !vscp_convertStringToEvent(pev, str) ) return 1;

                // Add to event queue
                pObj->m_eventReceiveQueue.push_back(pev);
            }
        }
        else {
            pObj->m_msgReceiveQueue.push_back(str);
            sem_post(&pObj->m_sem_msg);
        }
    }

    // Another option would be BINARY data. 
	if (isBin) {
        ;
    }

    // It could be a CLOSE message as well. 
	if (isClose) {
		return 0;
	}

	return 1;
}

static void
ws1_client_close_handler(const struct mg_connection *conn,
                               void *user_data)
{
	struct mg_context *ctx = mg_get_context(conn);
	vscpClientWs1 *pObj =
	    (vscpClientWs1 *)mg_get_user_data(ctx);

    pObj->m_bConnected = false;
	printf("----------------> Client: Close handler\n");
}

///////////////////////////////////////////////////////////////////////////////
// CTOR
//

vscpClientWs1::vscpClientWs1()
{
    m_type = CVscpClient::connType::WS1;
    m_bConnected = false;
    m_conn = NULL;
    m_host = "localhost";
    m_port = 8884;
    m_bSSL = false;     // SSL/TLS not activated by default

    setConnectionTimeout();
    setResponseTimeout();

    sem_init(&m_sem_msg, 0, 0);
    mg_init_library(MG_FEATURES_SSL);
}

///////////////////////////////////////////////////////////////////////////////
// DTOR
//

vscpClientWs1::~vscpClientWs1()
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
// getConfigAsJson
//

std::string vscpClientWs1::getConfigAsJson(void) 
{
    std::string rv;

    return rv;
}


///////////////////////////////////////////////////////////////////////////////
// initFromJson
//

bool vscpClientWs1::initFromJson(const std::string& config)
{
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// init
//

int vscpClientWs1::init(const std::string host,
                        short port,
                        bool bSSL,
                        const std::string username,
                        const std::string password,
                        uint8_t* vscpkey,
                        uint32_t connection_timeout,
                        uint32_t response_timeout)
{    
    m_host = host;
    m_port = port;
    m_bSSL = bSSL;
    
    setConnectionTimeout(connection_timeout);
    setResponseTimeout(response_timeout);

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

int vscpClientWs1::connect(void) 
{
    std::deque<std::string> args;
    std::string reply;
    char ebuf[100] = {0};
	const char *path = "/ws1";

	m_conn = mg_connect_websocket_client( m_host.c_str(),
	                                       m_port,
	                                       m_bSSL ? 1 : 0,
	                                       ebuf,
	                                       sizeof(ebuf),
	                                       path,
	                                       NULL,
	                                       ws1_client_data_handler,
	                                       ws1_client_close_handler,
	                                       this);

	if (NULL == m_conn) {
		printf("Error: %s\n", ebuf);
        return VSCP_ERROR_CONNECTION;
	}

    if ( VSCP_ERROR_SUCCESS != waitForResponse( m_timeout_connect ) ) {
		printf("ERROR or TIMEOUT\n");
        disconnect();
		return VSCP_ERROR_TIMEOUT;
	}

    // Get response message
    reply = m_msgReceiveQueue.front();
    m_msgReceiveQueue.pop_front();

    // Get arguments
    args.clear();
    vscp_split(args,reply,";");

    if ( (3 != args.size()) ||  
         ("+" != args.at(0)) || 
         ("AUTH0" != args.at(1)) ) {
        printf("Mr strange\n");     
        disconnect();
        return VSCP_ERROR_TIMEOUT; 
    }

    std::string authcmd = "C;AUTH;";
    char ivbuf[50];
    vscp_byteArray2HexStr(ivbuf, m_iv, 16);
    authcmd += std::string(ivbuf);
    authcmd += ";";
    authcmd += m_credentials;

    mg_websocket_client_write(m_conn,
		                          MG_WEBSOCKET_OPCODE_TEXT,
		                          authcmd.c_str(),
		                          authcmd.length());
		
	if ( VSCP_ERROR_SUCCESS != waitForResponse( m_timeout_response ) ) {
		printf("User not validated\n");
        disconnect();
		return VSCP_ERROR_USER;
	}

    // Get response message
    reply = m_msgReceiveQueue.front();
    m_msgReceiveQueue.pop_front();

    // Get arguments
    args.clear();
    vscp_split(args,reply,";");

    if ( (args.size() >= 2 ) &&
         ("+" != args.at(0)) || 
         ("AUTH1" != args.at(1)) ) {
        printf("Invalid response on AUTH\n");     
        disconnect();
        return VSCP_ERROR_TIMEOUT; 
    }

    std::string opencmd ="C;OPEN";

    mg_websocket_client_write(m_conn,
		                          MG_WEBSOCKET_OPCODE_TEXT,
		                          opencmd.c_str(),
		                          opencmd.length());

    if ( VSCP_ERROR_SUCCESS != waitForResponse( m_timeout_response ) ) {
		printf("Unable to open connection\n");
        disconnect();
		return VSCP_ERROR_OPERATION_FAILED;
	}

    // Get response message
    reply = m_msgReceiveQueue.front();
    m_msgReceiveQueue.pop_front();

    // Get arguments
    args.clear();
    vscp_split(args,reply,";");

    if ( ("+" != args.at(0)) || 
         ("OPEN" != args.at(1)) ) {
        printf("2 Unable to open connection\n");     
        disconnect();
        return VSCP_ERROR_TIMEOUT; 
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// disconnect
//

int vscpClientWs1::disconnect(void)
{
    std::deque<std::string> args;
    std::string cmd = "C;CLOSE";

    mg_websocket_client_write(m_conn,
		                          MG_WEBSOCKET_OPCODE_TEXT,
		                          cmd.c_str(),
		                          cmd.length());

    mg_websocket_client_write(m_conn,
                                  MG_WEBSOCKET_OPCODE_CONNECTION_CLOSE,
                                  NULL,
                                  0);

    mg_close_connection(m_conn);
    m_conn = NULL;

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// isConnected
//

bool vscpClientWs1::isConnected(void)
{
    return m_bConnected;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int vscpClientWs1::send(vscpEvent &ev)
{
    std::deque<std::string> args;
    std::string strEvent;
    std::string cmd = "E;";

    if (!isConnected()) return VSCP_ERROR_NOT_CONNECTED;

    if ( !vscp_convertEventToString(strEvent, &ev) ) return VSCP_ERROR_ERROR;
    cmd += strEvent;

    mg_websocket_client_write(m_conn,
		                          MG_WEBSOCKET_OPCODE_TEXT,
		                          cmd.c_str(),
		                          cmd.length());

    if ( VSCP_ERROR_SUCCESS != waitForResponse( m_timeout_response ) ) {
		printf("ERROR or TIMEOUT\n");
		return VSCP_ERROR_TIMEOUT;
	}

    // Check return value
    std::string reply = m_msgReceiveQueue.front();
    m_msgReceiveQueue.pop_front();

    // Get arguments
    args.clear();
    vscp_split(args,reply,";");

    if (("+" != args.at(0)) &&
        ("E" != args.at(1))) {
        return VSCP_ERROR_OPERATION_FAILED;
    }
    
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int vscpClientWs1::send(vscpEventEx &ex)
{
    std::deque<std::string> args;
    std::string strEvent;    
    std::string cmd = "E;";

    if (!isConnected()) return VSCP_ERROR_NOT_CONNECTED;

    if ( !vscp_convertEventExToString(strEvent, &ex) ) return VSCP_ERROR_ERROR;
    cmd += strEvent;

    mg_websocket_client_write(m_conn,
		                          MG_WEBSOCKET_OPCODE_TEXT,
		                          cmd.c_str(),
		                          cmd.length());

    if ( VSCP_ERROR_SUCCESS != waitForResponse( m_timeout_response ) ) {
		printf("ERROR or TIMEOUT\n");
		return VSCP_ERROR_TIMEOUT;
	}

    // Check return value
    std::string reply = m_msgReceiveQueue.front();
    m_msgReceiveQueue.pop_front();

    // Get arguments
    args.clear();
    vscp_split(args,reply,";");

    if (("+" != args.at(0)) &&
        ("E" != args.at(1))) {
        return VSCP_ERROR_OPERATION_FAILED;
    }
    
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int vscpClientWs1::receive(vscpEvent &ev)
{
    if (!isConnected()) return VSCP_ERROR_NOT_CONNECTED;
    
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

int vscpClientWs1::receive(vscpEventEx &ex)
{
    if (!isConnected()) return VSCP_ERROR_NOT_CONNECTED;

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

int vscpClientWs1::setfilter(vscpEventFilter &filter)
{
    std::deque<std::string> args;
    std::string strGUID;
    std::string cmd = "C;SETFILTER;";

    if (!isConnected()) return VSCP_ERROR_NOT_CONNECTED;

    std::string strFilter,strMask;
    vscp_writeFilterToString(strFilter,&filter);
    vscp_writeMaskToString(strMask,&filter);
    cmd += strFilter;
    cmd += ";";
    cmd += strMask;

    mg_websocket_client_write(m_conn,
		                          MG_WEBSOCKET_OPCODE_TEXT,
		                          cmd.c_str(),
		                          cmd.length());

    if ( VSCP_ERROR_SUCCESS != waitForResponse( m_timeout_response ) ) {
		printf("ERROR or TIMEOUT\n");
		return VSCP_ERROR_TIMEOUT;
	}

    // Check return value
    std::string reply = m_msgReceiveQueue.front();
    m_msgReceiveQueue.pop_front();

    // Get arguments
    args.clear();
    vscp_split(args,reply,";");

    if (("+" != args.at(0)) &&
        ("SETFILTER" != args.at(1))) {
        return VSCP_ERROR_OPERATION_FAILED;
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getcount
//

int vscpClientWs1::getcount(uint16_t *pcount)
{
    if (NULL == pcount) return VSCP_ERROR_INVALID_POINTER;
    *pcount = (uint16_t)m_eventReceiveQueue.size();
    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getcount
//

int vscpClientWs1::getversion(uint8_t *pmajor,
                                uint8_t *pminor,
                                uint8_t *prelease,
                                uint8_t *pbuild)
{
    std::deque<std::string> args;
    std::string cmd = "C;VERSION";

    if (!isConnected()) return VSCP_ERROR_NOT_CONNECTED;

    // Check pointers
    if ( (NULL != pmajor) ||
         (NULL != pminor) ||
         (NULL != prelease) ||
         (NULL != pbuild) ) {
        return VSCP_ERROR_PARAMETER;
    }

    mg_websocket_client_write(m_conn,
		                          MG_WEBSOCKET_OPCODE_TEXT,
		                          cmd.c_str(),
		                          cmd.length());

    if ( VSCP_ERROR_SUCCESS != waitForResponse( m_timeout_response ) ) {
		printf("ERROR or TIMEOUT\n");
		return VSCP_ERROR_TIMEOUT;
	}

    // Check return value
    std::string reply = m_msgReceiveQueue.front();
    m_msgReceiveQueue.pop_front();

    // Get arguments
    args.clear();
    vscp_split(args,reply,";");

    if ( (args.size() >= 3) &&
        ("+" != args.at(0)) &&
        ("VERSION" != args.at(1))) {
        return VSCP_ERROR_OPERATION_FAILED;
    }

    // Version is delivered as a string on the form "major,minor,release,build"

    std::deque<std::string> ver;
    vscp_split(ver,args.at(2),",");
    if (4 != ver.size()) return VSCP_ERROR_MISSING;

    *pmajor = vscp_readStringValue(ver.at(0));
    *pminor = vscp_readStringValue(ver.at(1));
    *prelease = vscp_readStringValue(ver.at(2));
    *pbuild = vscp_readStringValue(ver.at(3));   

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// clear
//

int vscpClientWs1::clear(void) 
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

int vscpClientWs1::getinterfaces(std::deque<std::string> &iflist)
{
    std::deque<std::string> args;
    std::string strGUID;
    std::string cmd = "C;INTERFACES";

    if (!isConnected()) return VSCP_ERROR_NOT_CONNECTED;

    mg_websocket_client_write(m_conn,
		                          MG_WEBSOCKET_OPCODE_TEXT,
		                          cmd.c_str(),
		                          cmd.length());

    if ( VSCP_ERROR_SUCCESS != waitForResponse( m_timeout_response ) ) {
		printf("ERROR or TIMEOUT\n");
		return VSCP_ERROR_TIMEOUT;
	}

    // Check return value
    std::string reply = m_msgReceiveQueue.front();
    m_msgReceiveQueue.pop_front();

    // Get arguments
    args.clear();
    vscp_split(args,reply,";");

    if ( (args.size() >= 2) &&
        ("+" != args.at(0)) &&
        ("INTERFACES" != args.at(1))) {
        return VSCP_ERROR_OPERATION_FAILED;
    }

    for ( int i=3; i<args.size(); i++ ) {
        std::string str = args.front();
        args.pop_front();
        iflist.push_back(str);
    }

    return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getwcyd
//

int vscpClientWs1::getwcyd(uint64_t &wcyd)
{
    std::deque<std::string> args;
    std::string strGUID;
    std::string cmd = "C;WCYD";

    if (!isConnected()) return VSCP_ERROR_NOT_CONNECTED;

    mg_websocket_client_write(m_conn,
		                          MG_WEBSOCKET_OPCODE_TEXT,
		                          cmd.c_str(),
		                          cmd.length());

    if ( VSCP_ERROR_SUCCESS != waitForResponse( m_timeout_response ) ) {
		printf("ERROR or TIMEOUT\n");
		return VSCP_ERROR_TIMEOUT;
	}

    // Check return value
    std::string reply = m_msgReceiveQueue.front();
    m_msgReceiveQueue.pop_front();

    // Get arguments
    args.clear();
    vscp_split(args,reply,";");

    if ( (args.size() >= 10) &&
        ("+" != args.at(0)) &&
        ("WCYD" != args.at(1))) {
        return VSCP_ERROR_OPERATION_FAILED;
    }

    // capabilities is delivered on form
    // C;WCYD;8,7,6,5,4,3,2,1 
    // where "8" is MSB

    wcyd = ((uint64_t)vscp_readStringValue(args.at(2)) << 56) +
            ((uint64_t)vscp_readStringValue(args.at(3)) << 48) +
            ((uint64_t)vscp_readStringValue(args.at(4)) << 40) +
            ((uint64_t)vscp_readStringValue(args.at(5)) << 32) +
            ((uint64_t)vscp_readStringValue(args.at(6)) << 24) +
            ((uint64_t)vscp_readStringValue(args.at(7)) << 16) +
            ((uint64_t)vscp_readStringValue(args.at(8)) << 8) +
            (uint64_t)vscp_readStringValue(args.at(9));

    return VSCP_ERROR_SUCCESS;
}


int vscpClientWs1::encrypt_password(std::string& strout,
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
							(uint32_t)strCombined.length(), 
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

int vscpClientWs1::waitForResponse( uint32_t timeout )
{	
	time_t ts;
	time(&ts);

	struct timespec to;
	to.tv_nsec = 0;
	to.tv_sec = ts + timeout/1000;

	if ( -1 == sem_timedwait(&m_sem_msg,&to) ) {
		
		switch(errno) {

			case EINTR:
				return VSCP_ERROR_INTERRUPTED;

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