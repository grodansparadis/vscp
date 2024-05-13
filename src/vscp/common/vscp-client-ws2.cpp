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
// Copyright:  (C) 2007-2024
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

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <vscp-aes.h>
#include <vscphelper.h>
// #include "civetweb.h"

#include "vscp-client-ws2.h"

#include <nlohmann/json.hpp> // Needs C++11  -std=c++11

// for convenience
using json = nlohmann::json;

/*
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
        printf("%s\n",str.c_str());

        // If type is event put in event queue or send
        // out on defined callbacks
        // If other type put in message queue

        json j;
        try {
            j = json::parse(str.c_str());
        }
        catch (...) {
            return 1;
        }

        if ( "event" == j["type"]) {

            if (pObj->isEvCallback()) {
                vscpEvent *pev = new vscpEvent;
                if ( NULL == pev ) return 0;
                std::string str = j["event"].dump();
                if ( !vscp_convertJSONToEvent(pev, str) ) return 1;
                pObj->m_evcallback(pev, pObj->m_callbackObject);
            }
            else if (pObj->isExCallback()) {
                vscpEventEx *pex = new vscpEventEx;
                if ( NULL == pex ) return 0;
                std::string str = j["event"].dump();
                if ( !vscp_convertJSONToEventEx(pex, str) ) return 1;
                pObj->m_excallback(pex, pObj->m_callbackObject);
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
ws2_client_close_handler(const struct mg_connection *conn,
                               void *user_data)
{
  struct mg_context *ctx = mg_get_context(conn);
  vscpClientWs2 *pObj =
      (vscpClientWs2 *)mg_get_user_data(ctx);

    pObj->m_bConnected = false;
  printf("----------------> Client: Close handler\n");
}
*/

///////////////////////////////////////////////////////////////////////////////
// CTOR
//

vscpClientWs2::vscpClientWs2()
{
  m_type       = CVscpClient::connType::WS2;
  m_bConnected = false;
  m_conn       = NULL;
  m_host       = "localhost";
  m_port       = 8884;
  m_bSSL       = false; // SSL/TLS not activated by default

  setConnectionTimeout();
  setResponseTimeout();

  sem_init(&m_sem_msg, 0, 0);
  // mg_init_library(MG_FEATURES_SSL);
}

///////////////////////////////////////////////////////////////////////////////
// DTOR
//

vscpClientWs2::~vscpClientWs2()
{
  disconnect();

  while (m_eventReceiveQueue.size()) {
    vscpEvent *pev = m_eventReceiveQueue.front();
    m_eventReceiveQueue.pop_front();
    vscp_deleteEvent_v2(&pev);
  }

  sem_destroy(&m_sem_msg);
}

///////////////////////////////////////////////////////////////////////////////
// getConfigAsJson
//

std::string
vscpClientWs2::getConfigAsJson(void)
{
  std::string rv;

  return rv;
}

///////////////////////////////////////////////////////////////////////////////
// initFromJson
//

bool
vscpClientWs2::initFromJson(const std::string &config)
{
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// init
//

int
vscpClientWs2::init(const std::string host,
                    short port,
                    bool bSSL,
                    const std::string username,
                    const std::string password,
                    uint8_t *vscpkey,
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
  encrypt_password(m_credentials, username, password, vscpkey, m_iv);

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// connect
//

int
vscpClientWs2::connect(void)
{
  json j;
  char ebuf[100]   = { 0 };
  const char *path = "/ws2";

  // m_conn = mg_connect_websocket_client( m_host.c_str(),
  //                                        m_port,
  //                                        m_bSSL ? 1 : 0,
  //                                        ebuf,
  //                                        sizeof(ebuf),
  //                                        path,
  //                                        NULL,
  //                                        ws2_client_data_handler,
  //                                        ws2_client_close_handler,
  //                                        this);

  if (NULL == m_conn) {
    printf("Error: %s\n", ebuf);
    return VSCP_ERROR_CONNECTION;
  }

  if (VSCP_ERROR_SUCCESS != waitForResponse(m_timeout_connect)) {
    printf("ERROR or TIMEOUT\n");
    disconnect();
    return VSCP_ERROR_TIMEOUT;
  }

  // Get response message
  j = m_msgReceiveQueue.front();
  m_msgReceiveQueue.pop_front();

  // Get arguments (AUTH0,iv)
  std::deque<std::string> args = j["args"];

  if ((2 != args.size()) || ("+" != j["type"]) || ("AUTH0" != j["args"][0])) {
    printf("Mr strange\n");
    disconnect();
    return VSCP_ERROR_TIMEOUT;
  }

  json authcmd;
  authcmd["type"]    = "cmd";
  authcmd["command"] = "auth";
  char ivbuf[50];
  vscp_byteArray2HexStr(ivbuf, m_iv, 16);
  authcmd["args"]["iv"]     = std::string(ivbuf);
  authcmd["args"]["crypto"] = m_credentials;

  // mg_websocket_client_write(m_conn,
  //                           MG_WEBSOCKET_OPCODE_TEXT,
  //                           authcmd.dump().c_str(),
  //                           authcmd.dump().length());

  if (VSCP_ERROR_SUCCESS != waitForResponse(m_timeout_response)) {
    printf("User not validated\n");
    disconnect();
    return VSCP_ERROR_USER;
  }

  // Get response message
  j = m_msgReceiveQueue.front();
  m_msgReceiveQueue.pop_front();

  if (("+" != j["type"]) || ("AUTH" != j["command"])) {
    printf("Invalid response on AUTH\n");
    disconnect();
    return VSCP_ERROR_TIMEOUT;
  }

  json opencmd;
  authcmd["type"]    = "cmd";
  authcmd["command"] = "open";
  authcmd["args"]    = nullptr;

  // mg_websocket_client_write(m_conn,
  //                           MG_WEBSOCKET_OPCODE_TEXT,
  //                           authcmd.dump().c_str(),
  //                           authcmd.dump().length());

  if (VSCP_ERROR_SUCCESS != waitForResponse(m_timeout_response)) {
    printf("Unable to open connection\n");
    disconnect();
    return VSCP_ERROR_OPERATION_FAILED;
  }

  // Get response message
  j = m_msgReceiveQueue.front();
  m_msgReceiveQueue.pop_front();

  if (("+" != j["type"]) || ("OPEN" != j["command"])) {
    printf("2 Unable to open connection\n");
    disconnect();
    return VSCP_ERROR_TIMEOUT;
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// disconnect
//

int
vscpClientWs2::disconnect(void)
{
  json cmd;
  cmd["type"]    = "cmd";
  cmd["command"] = "close";
  cmd["args"]    = nullptr;

  // mg_websocket_client_write(m_conn,
  //                           MG_WEBSOCKET_OPCODE_TEXT,
  //                           cmd.dump().c_str(),
  //                           cmd.dump().length());

  // We do not check results of the op to make sure we
  // disconnect even if something goes wrong command wise

  // if ( VSCP_ERROR_SUCCESS != waitForResponse( m_timeout_response ) ) {
  // 	return VSCP_ERROR_TIMEOUT;
  // }

  // // Check return value
  // json j = m_msgReceiveQueue.front();
  // m_msgReceiveQueue.pop_front();
  // if ( "+" != j["type"]) return VSCP_ERROR_OPERATION_FAILED;

  // mg_websocket_client_write(m_conn,
  //                               MG_WEBSOCKET_OPCODE_CONNECTION_CLOSE,
  //                               NULL,
  //                               0);

  // mg_close_connection(m_conn);
  m_conn = NULL;

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// isConnected
//

bool
vscpClientWs2::isConnected(void)
{
  return m_bConnected;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int
vscpClientWs2::send(vscpEvent &ev)
{
  json cmd;

  if (!isConnected())
    return VSCP_ERROR_NOT_CONNECTED;

  cmd["type"]               = "event";
  cmd["event"]["head"]      = ev.head;
  cmd["event"]["obid"]      = ev.obid;
  cmd["event"]["timestamp"] = ev.timestamp;
  cmd["event"]["class"]     = ev.vscp_class;
  cmd["event"]["type"]      = ev.vscp_type;

  std::string dt;
  vscp_getDateStringFromEvent(dt, &ev);
  cmd["event"]["datetime"] = dt;

  std::string strGuid;
  vscp_writeGuidToString(strGuid, &ev);
  cmd["event"]["guid"] = strGuid;

  if ((NULL != ev.pdata) && ev.sizeData) {
    std::deque<uint8_t> data;
    for (int i = 0; i < ev.sizeData; i++) {
      data.push_back(ev.pdata[i]);
    }
    cmd["event"]["data"] = data;
  }
  else {
    cmd["event"]["data"] = nullptr;
  }

  // mg_websocket_client_write(m_conn,
  //                           MG_WEBSOCKET_OPCODE_TEXT,
  //                           cmd.dump().c_str(),
  //                           cmd.dump().length());

  if (VSCP_ERROR_SUCCESS != waitForResponse(m_timeout_response)) {
    printf("ERROR or TIMEOUT\n");
    return VSCP_ERROR_TIMEOUT;
  }

  // Check return value
  json j = m_msgReceiveQueue.front();
  m_msgReceiveQueue.pop_front();
  if ("+" != j["type"])
    VSCP_ERROR_OPERATION_FAILED;

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int
vscpClientWs2::send(vscpEventEx &ex)
{
  json cmd;

  if (!isConnected())
    return VSCP_ERROR_NOT_CONNECTED;

  cmd["type"]               = "event";
  cmd["event"]["head"]      = ex.head;
  cmd["event"]["obid"]      = ex.obid;
  cmd["event"]["timestamp"] = ex.timestamp;
  cmd["event"]["class"]     = ex.vscp_class;
  cmd["event"]["type"]      = ex.vscp_type;

  std::string dt;
  vscp_getDateStringFromEventEx(dt, &ex);
  cmd["event"]["datetime"] = dt;

  std::string strGuid;
  vscp_writeGuidToStringEx(strGuid, &ex);
  cmd["event"]["guid"] = strGuid;

  if (ex.sizeData) {
    std::deque<uint8_t> data;
    for (int i = 0; i < ex.sizeData; i++) {
      data.push_back(ex.data[i]);
    }
    cmd["event"]["data"] = data;
  }
  else {
    cmd["event"]["data"] = nullptr;
  }

  // mg_websocket_client_write(m_conn,
  //                           MG_WEBSOCKET_OPCODE_TEXT,
  //                           cmd.dump().c_str(),
  //                           cmd.dump().length());

  if (VSCP_ERROR_SUCCESS != waitForResponse(m_timeout_response)) {
    printf("ERROR or TIMEOUT\n");
    return VSCP_ERROR_TIMEOUT;
  }

  // Check return value
  json j = m_msgReceiveQueue.front();
  m_msgReceiveQueue.pop_front();
  if ("+" != j["type"])
    VSCP_ERROR_OPERATION_FAILED;

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int
vscpClientWs2::send(canalMsg &msg)
{
  vscpEventEx ex;

  uint8_t guid[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  if (vscp_convertCanalToEventEx(&ex, &msg, guid)) {
    return VSCP_ERROR_INVALID_FRAME;
  }

  return send(ex);
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int
vscpClientWs2::receive(vscpEvent &ev)
{
  if (!isConnected())
    return VSCP_ERROR_NOT_CONNECTED;

  // check if there are anything to fetch
  if (!m_eventReceiveQueue.size()) {
    return VSCP_ERROR_RCV_EMPTY;
  }

  // only valid if no callback is defined
  if (isCallbackEvActive() || isCallbackExActive()) {
    return VSCP_ERROR_NOT_SUPPORTED;
  }

  vscpEvent *pev = m_eventReceiveQueue.front();
  m_eventReceiveQueue.pop_front();

  // Must be a avalid pointer
  if (NULL == pev) {
    return VSCP_ERROR_MEMORY;
  }

  vscp_copyEvent(&ev, pev);
  vscp_deleteEvent_v2(&pev);

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int
vscpClientWs2::receive(vscpEventEx &ex)
{
  if (!isConnected())
    return VSCP_ERROR_NOT_CONNECTED;

  // check if there are anything to fetch
  if (!m_eventReceiveQueue.size()) {
    return VSCP_ERROR_RCV_EMPTY;
  }

  // only valid if no callback is defined
  if (isCallbackEvActive() || isCallbackExActive()) {
    return VSCP_ERROR_NOT_SUPPORTED;
  }

  vscpEvent *pev = m_eventReceiveQueue.front();
  m_eventReceiveQueue.pop_front();

  // Must be a avalid pointer
  if (NULL == pev) {
    return VSCP_ERROR_MEMORY;
  }

  vscp_convertEventToEventEx(&ex, pev);
  vscp_deleteEvent_v2(&pev);

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int
vscpClientWs2::receive(canalMsg &msg)
{
  int rv;
  vscpEventEx ex;

  if (VSCP_ERROR_SUCCESS != (rv = receive(ex))) {
    return rv;
  }

  if (!vscp_convertEventExToCanal(&msg, &ex)) {
    return VSCP_ERROR_INVALID_FRAME;
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setfilter
//

int
vscpClientWs2::setfilter(vscpEventFilter &filter)
{
  std::string strGUID;
  json cmd;

  if (!isConnected())
    return VSCP_ERROR_NOT_CONNECTED;

  cmd["type"]    = "cmd";
  cmd["command"] = "setfilter";

  cmd["args"]["filter_priority"] = filter.filter_priority;
  cmd["args"]["filter_class"]    = filter.filter_class;
  cmd["args"]["filter_type"]     = filter.filter_type;
  vscp_writeGuidArrayToString(strGUID, filter.filter_GUID);
  cmd["args"]["filter_guid"] = strGUID;

  cmd["args"]["mask_priority"] = filter.mask_priority;
  cmd["args"]["mask_class"]    = filter.mask_class;
  cmd["args"]["mask_type"]     = filter.mask_type;
  vscp_writeGuidArrayToString(strGUID, filter.mask_GUID);
  cmd["args"]["mask_guid"] = strGUID;

  // mg_websocket_client_write(m_conn,
  //                           MG_WEBSOCKET_OPCODE_TEXT,
  //                           cmd.dump().c_str(),
  //                           cmd.dump().length());

  if (VSCP_ERROR_SUCCESS != waitForResponse(m_timeout_response)) {
    printf("ERROR or TIMEOUT\n");
    return VSCP_ERROR_TIMEOUT;
  }

  // Check return value
  json j = m_msgReceiveQueue.front();
  m_msgReceiveQueue.pop_front();
  if ("+" != j["type"])
    VSCP_ERROR_OPERATION_FAILED;

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getcount
//

int
vscpClientWs2::getcount(uint16_t *pcount)
{
  if (NULL == pcount)
    return VSCP_ERROR_INVALID_POINTER;
  *pcount = (uint16_t) m_eventReceiveQueue.size();
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getcount
//

int
vscpClientWs2::getversion(uint8_t *pmajor, uint8_t *pminor, uint8_t *prelease, uint8_t *pbuild)
{
  json cmd;

  if (!isConnected())
    return VSCP_ERROR_NOT_CONNECTED;

  // Check pointers
  if ((NULL != pmajor) || (NULL != pminor) || (NULL != prelease) || (NULL != pbuild)) {
    return VSCP_ERROR_PARAMETER;
  }

  cmd["type"]    = "cmd";
  cmd["command"] = "version";

  cmd["args"] = nullptr;

  // mg_websocket_client_write(m_conn,
  //                           MG_WEBSOCKET_OPCODE_TEXT,
  //                           cmd.dump().c_str(),
  //                           cmd.dump().length());

  if (VSCP_ERROR_SUCCESS != waitForResponse(m_timeout_response)) {
    printf("ERROR or TIMEOUT\n");
    return VSCP_ERROR_TIMEOUT;
  }

  // Check return value
  json j = m_msgReceiveQueue.front();
  m_msgReceiveQueue.pop_front();
  if ("+" != j["type"])
    VSCP_ERROR_OPERATION_FAILED;

  // Version is delivered as array on form [major,minor,release,build]

  std::deque<std::string> args = j["args"];
  if (4 != args.size())
    return VSCP_ERROR_MISSING;

  *pmajor   = j["args"][0];
  *pminor   = j["args"][1];
  *prelease = j["args"][2];
  *pbuild   = j["args"][3];

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// clear
//

int
vscpClientWs2::clear(void)
{
  while (m_eventReceiveQueue.size()) {
    vscpEvent *pev = m_eventReceiveQueue.front();
    m_eventReceiveQueue.pop_front();
    vscp_deleteEvent_v2(&pev);
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getinterfaces
//

int
vscpClientWs2::getinterfaces(std::deque<std::string> &iflist)
{
  std::string strGUID;
  json cmd;

  if (!isConnected())
    return VSCP_ERROR_NOT_CONNECTED;

  cmd["type"]    = "cmd";
  cmd["command"] = "interfaces";

  cmd["args"] = nullptr;

  // mg_websocket_client_write(m_conn,
  //                           MG_WEBSOCKET_OPCODE_TEXT,
  //                           cmd.dump().c_str(),
  //                           cmd.dump().length());

  if (VSCP_ERROR_SUCCESS != waitForResponse(m_timeout_response)) {
    printf("ERROR or TIMEOUT\n");
    return VSCP_ERROR_TIMEOUT;
  }

  // Check return value
  json j = m_msgReceiveQueue.front();
  m_msgReceiveQueue.pop_front();
  if ("+" != j["type"])
    VSCP_ERROR_OPERATION_FAILED;

  std::deque<std::string> args = j["args"];
  iflist                       = args;

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getwcyd
//

int
vscpClientWs2::getwcyd(uint64_t &wcyd)
{
  std::string strGUID;
  json cmd;

  if (!isConnected())
    return VSCP_ERROR_NOT_CONNECTED;

  cmd["type"]    = "cmd";
  cmd["command"] = "wcyd";
  cmd["args"]    = nullptr;

  // mg_websocket_client_write(m_conn,
  //                           MG_WEBSOCKET_OPCODE_TEXT,
  //                           cmd.dump().c_str(),
  //                           cmd.dump().length());

  if (VSCP_ERROR_SUCCESS != waitForResponse(m_timeout_response)) {
    printf("ERROR or TIMEOUT\n");
    return VSCP_ERROR_TIMEOUT;
  }

  // Check return value
  json j = m_msgReceiveQueue.front();
  m_msgReceiveQueue.pop_front();
  if ("+" != j["type"])
    VSCP_ERROR_OPERATION_FAILED;

  std::deque<std::string> args = j["args"];
  if (!args.size()) {
    return VSCP_ERROR_PARAMETER;
  }

  wcyd = j["args"][0];

  return VSCP_ERROR_SUCCESS;
}

int
vscpClientWs2::encrypt_password(std::string &strout,
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

  AES_CBC_encrypt_buffer(AES128, buf, (uint8_t *) strCombined.c_str(), (uint32_t) strCombined.length(), vscpkey, iv);

  char out[50];
  vscp_byteArray2HexStr(out, buf, 16);
  strout = out;

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// waitForResponse
//

int
vscpClientWs2::waitForResponse(uint32_t timeout)
{
  time_t ts;
  time(&ts);

  struct timespec to;
  to.tv_nsec = 0;
  to.tv_sec  = ts + timeout / 1000;

  if (-1 == sem_timedwait(&m_sem_msg, &to)) {

    switch (errno) {

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