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
// Copyright:   (C) 2007-2026
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

#include <stdio.h>
#include <stdlib.h>

#include <nlohmann/json.hpp> // Needs C++11  -std=c++11

#include <spdlog/spdlog.h>

#include <vscphelper.h>

#include <algorithm>
#include <chrono>
#include <cstring>

#include "vscp-client-ws1.h"

using json = nlohmann::json;

///////////////////////////////////////////////////////////////////////////////
// CTOR
//

vscpClientWs1::vscpClientWs1()
  : m_bConnected(false)
  , m_bQuit(false)
  , m_bLinkUp(false)
{
  m_type          = CVscpClient::connType::WS1;
  m_settings.url  = "ws://localhost:8884/ws1";
  setConnectionTimeout();
  setResponseTimeout();
}

///////////////////////////////////////////////////////////////////////////////
// DTOR
//

vscpClientWs1::~vscpClientWs1()
{
  disconnect();
  clear();
}

///////////////////////////////////////////////////////////////////////////////
// getConfigAsJson
//

std::string
vscpClientWs1::getConfigAsJson(void)
{
  json j;
  j["name"] = getName();
  m_settings.toJson(j, m_timeout_connect, m_timeout_response);
  return j.dump();
}

///////////////////////////////////////////////////////////////////////////////
// initFromJson
//

bool
vscpClientWs1::initFromJson(const std::string &config)
{
  json j;

  try {
    j = json::parse(config);
  }
  catch (...) {
    spdlog::error("WS1 client: Failed to parse JSON configuration.");
    return false;
  }

  if (j.contains("name") && j["name"].is_string()) {
    setName(j["name"].get<std::string>());
  }

  std::lock_guard<std::mutex> lock(m_mutexEvent); // Protects the filter
  if (!m_settings.fromJson(j, m_timeout_connect, m_timeout_response)) {
    spdlog::error("WS1 client: Invalid configuration (check that the key has 32 hexadecimal characters).");
    return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// init
//

int
vscpClientWs1::init(const std::string host,
                    short port,
                    bool bSSL,
                    const std::string username,
                    const std::string password,
                    uint8_t *vscpkey,
                    uint32_t connection_timeout,
                    uint32_t response_timeout)
{
  std::string strHost = (std::string::npos != host.find(':')) ? ("[" + host + "]") : host;
  m_settings.url      = std::string(bSSL ? "wss://" : "ws://") + strHost + ":" + std::to_string((unsigned short) port) + "/ws1";
  m_settings.user     = username;
  m_settings.password = password;

  if (nullptr != vscpkey) {
    char buf[33];
    vscp_byteArray2HexStr(buf, vscpkey, 16);
    buf[32]        = '\0';
    m_settings.key = buf;
  }

  setConnectionTimeout(connection_timeout);
  setResponseTimeout(response_timeout);

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getLastError
//

std::string
vscpClientWs1::getLastError(void)
{
  std::lock_guard<std::mutex> lock(m_mutexError);
  return m_lastError;
}

///////////////////////////////////////////////////////////////////////////////
// handleMessage
//

void
vscpClientWs1::handleMessage(const std::string &msg)
{
  std::string str = msg;
  vscp_trim(str);
  if (str.empty()) {
    return;
  }

  // Event "E;head,class,type,obid,datetime,timestamp,GUID,data..."
  if ((str.size() > 2) && ('E' == str[0]) && (';' == str[1])) {

    vscpEvent *pev = new vscpEvent;
    memset(pev, 0, sizeof(vscpEvent));
    if (!vscp_convertStringToEvent(pev, str.substr(2))) {
      spdlog::warn("WS1 client: Invalid event from server [{}]", str);
      vscp_deleteEvent_v2(&pev);
      return;
    }

    {
      std::lock_guard<std::mutex> lock(m_mutexEvent);
      if (!vscp_doLevel2Filter(pev, &m_settings.filter)) {
        vscp_deleteEvent_v2(&pev);
        return;
      }
    }

    if (isCallbackEvActive()) {
      m_callbackev(*pev, getCallbackObj());
      vscp_deleteEvent_v2(&pev);
    }
    else if (isCallbackExActive()) {
      vscpEventEx ex;
      if (vscp_convertEventToEventEx(&ex, pev)) {
        m_callbackex(ex, getCallbackObj());
      }
      vscp_deleteEvent_v2(&pev);
    }
    else {
      std::lock_guard<std::mutex> lock(m_mutexEvent);
      m_eventReceiveQueue.push_back(pev);
      m_cvEvent.notify_all();
    }
    return;
  }

  // Reply
  std::lock_guard<std::mutex> lock(m_mutexMsg);
  m_msgReceiveQueue.push_back(str);
  m_cvMsg.notify_all();
}

///////////////////////////////////////////////////////////////////////////////
// workerThread
//

void
vscpClientWs1::workerThread(void)
{
  while (!m_bQuit) {

    std::string msg;
    bool bText = true;
    int rv     = m_ws.receive(msg, bText, 100);

    if (VSCP_ERROR_SUCCESS == rv) {
      if (bText) {
        handleMessage(msg);
      }
    }
    else if (VSCP_ERROR_TIMEOUT != rv) {
      if (!m_bQuit) {
        std::string err = m_ws.getLastError();
        spdlog::error("WS1 client: Connection lost. {}", err);
        std::lock_guard<std::mutex> lock(m_mutexError);
        m_lastError = err;
      }
      break;
    }
  }

  m_bLinkUp    = false;
  m_bConnected = false;

  // Wake up anyone waiting for a reply or an event
  {
    std::lock_guard<std::mutex> lock(m_mutexMsg);
    m_cvMsg.notify_all();
  }
  {
    std::lock_guard<std::mutex> lock(m_mutexEvent);
    m_cvEvent.notify_all();
  }
}

///////////////////////////////////////////////////////////////////////////////
// waitForReply
//

int
vscpClientWs1::waitForReply(std::string &reply, uint32_t timeout)
{
  std::unique_lock<std::mutex> lock(m_mutexMsg);
  m_cvMsg.wait_for(lock, std::chrono::milliseconds(timeout), [this] {
    return !m_msgReceiveQueue.empty() || !m_bLinkUp;
  });

  if (m_msgReceiveQueue.empty()) {
    return m_bLinkUp ? VSCP_ERROR_TIMEOUT : VSCP_ERROR_NOT_CONNECTED;
  }

  reply = m_msgReceiveQueue.front();
  m_msgReceiveQueue.pop_front();
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// waitForResponse
//

int
vscpClientWs1::waitForResponse(uint32_t timeout)
{
  std::unique_lock<std::mutex> lock(m_mutexMsg);
  m_cvMsg.wait_for(lock, std::chrono::milliseconds(timeout), [this] { return !m_msgReceiveQueue.empty(); });
  return m_msgReceiveQueue.empty() ? VSCP_ERROR_TIMEOUT : VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// request
//

int
vscpClientWs1::request(const std::string &msg, std::deque<std::string> &reply, uint32_t timeout)
{
  int rv;
  std::string str;

  std::lock_guard<std::mutex> cmdlock(m_mutexCommand);

  if (!m_bLinkUp) {
    return VSCP_ERROR_NOT_CONNECTED;
  }

  // Drop late replies to earlier commands
  {
    std::lock_guard<std::mutex> lock(m_mutexMsg);
    m_msgReceiveQueue.clear();
  }

  if (VSCP_ERROR_SUCCESS != (rv = m_ws.sendText(msg, timeout))) {
    return rv;
  }

  if (VSCP_ERROR_SUCCESS != (rv = waitForReply(str, timeout))) {
    return rv;
  }

  reply.clear();
  vscp_split(reply, str, ";");
  if (reply.empty()) {
    return VSCP_ERROR_ERROR;
  }

  return ("+" == reply.front()) ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
// command
//

int
vscpClientWs1::command(const std::string &cmd, std::deque<std::string> &reply, uint32_t timeout)
{
  return request("C;" + cmd, reply, timeout);
}

///////////////////////////////////////////////////////////////////////////////
// closeConnection
//

void
vscpClientWs1::closeConnection(void)
{
  m_bQuit = true;
  if (m_thread.joinable() && (std::this_thread::get_id() != m_thread.get_id())) {
    m_thread.join();
  }

  m_ws.close();
  m_bLinkUp    = false;
  m_bConnected = false;

  std::lock_guard<std::mutex> lock(m_mutexMsg);
  m_msgReceiveQueue.clear();
}

///////////////////////////////////////////////////////////////////////////////
// connect
//

int
vscpClientWs1::connect(void)
{
  int rv;
  std::string str;
  std::deque<std::string> args;
  vscpWsUrl url;
  uint8_t key[16];

  closeConnection();

  {
    std::lock_guard<std::mutex> lock(m_mutexError);
    m_lastError.clear();
  }

  auto fail = [this](int code, const std::string &err) {
    spdlog::error("WS1 client: {}", err);
    {
      std::lock_guard<std::mutex> lock(m_mutexError);
      m_lastError = err;
    }
    closeConnection();
    return code;
  };

  if (!CVscpWebSocket::parseUrl(m_settings.url, "/ws1", url)) {
    return fail(VSCP_ERROR_PARAMETER, "Invalid URL [" + m_settings.url + "]");
  }

  if (!m_settings.getKey(key)) {
    return fail(VSCP_ERROR_PARAMETER, "Invalid key, it should be 32 hexadecimal characters");
  }

  if (VSCP_ERROR_SUCCESS != (rv = m_ws.connect(url, m_timeout_connect, m_settings.tls))) {
    return fail(rv, m_ws.getLastError());
  }

  m_bQuit   = false;
  m_bLinkUp = true;
  m_thread  = std::thread(&vscpClientWs1::workerThread, this);

  // The server starts with "+;AUTH0;sid"
  if (VSCP_ERROR_SUCCESS != (rv = waitForReply(str, m_timeout_connect))) {
    return fail(VSCP_ERROR_TIMEOUT, "No authentication challenge from server");
  }

  vscp_split(args, str, ";");
  if ((args.size() < 3) || ("+" != args.at(0)) || ("AUTH0" != args.at(1))) {
    return fail(VSCP_ERROR_CONNECTION, "Unexpected first message from server [" + str + "]");
  }

  std::string sid = args.at(2);
  vscp_trim(sid);
  uint8_t iv[16];
  if ((32 != sid.length()) || (std::string::npos != sid.find_first_not_of("0123456789abcdefABCDEF")) ||
      (16 != vscp_hexStr2ByteArray(iv, sizeof(iv), sid.c_str()))) {
    return fail(VSCP_ERROR_CONNECTION, "Invalid session id from server");
  }

  std::string crypto;
  if (VSCP_ERROR_SUCCESS != vscp_ws_encryptCredentials(crypto, m_settings.user, m_settings.password, key, iv)) {
    return fail(VSCP_ERROR_PARAMETER, "Failed to encrypt credentials");
  }

  rv = command("AUTH;" + sid + ";" + crypto, args, m_timeout_response);
  if (VSCP_ERROR_ERROR == rv) {
    return fail(VSCP_ERROR_USER, "Authentication failed (check user, password and key)");
  }
  if (VSCP_ERROR_SUCCESS != rv) {
    return fail(rv, "No reply to authentication");
  }
  if ((args.size() < 2) || ("AUTH1" != args.at(1))) {
    return fail(VSCP_ERROR_USER, "Authentication failed");
  }

  rv = command("OPEN", args, m_timeout_response);
  if (VSCP_ERROR_SUCCESS != rv) {
    return fail((VSCP_ERROR_ERROR == rv) ? VSCP_ERROR_OPERATION_FAILED : rv, "Server refused to open the channel");
  }

  m_bConnected = true;

  // Let the server filter too, so filtered events never cross the network.
  // The filter is always applied locally as well, so a user without the
  // right to set a filter still gets filtered events.
  vscpEventFilter filter;
  {
    std::lock_guard<std::mutex> lock(m_mutexEvent);
    filter = m_settings.filter;
  }
  if (!m_settings.isFilterOpen() && (VSCP_ERROR_SUCCESS != setfilter(filter))) {
    spdlog::warn("WS1 client: Server did not accept the filter, filtering locally.");
  }

  spdlog::info("WS1 client: Connected to {}", m_settings.url);
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// disconnect
//

int
vscpClientWs1::disconnect(void)
{
  if (m_bLinkUp) {
    std::deque<std::string> args;
    command("CLOSE", args, (std::min)(m_timeout_response, (uint32_t) 1000));
  }

  closeConnection();
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// isConnected
//

bool
vscpClientWs1::isConnected(void)
{
  return m_bConnected;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int
vscpClientWs1::send(vscpEvent &ev)
{
  int rv;
  std::string strEvent;
  std::deque<std::string> args;

  if (!isConnected()) {
    return VSCP_ERROR_NOT_CONNECTED;
  }

  if (!vscp_convertEventToString(strEvent, &ev)) {
    return VSCP_ERROR_PARAMETER;
  }

  rv = request("E;" + strEvent, args, m_timeout_response);
  if (VSCP_ERROR_ERROR == rv) {
    spdlog::error("WS1 client: Server did not accept event [{}]", strEvent);
    return VSCP_ERROR_OPERATION_FAILED;
  }

  return rv;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int
vscpClientWs1::send(vscpEventEx &ex)
{
  int rv;
  std::string strEvent;
  std::deque<std::string> args;

  if (!isConnected()) {
    return VSCP_ERROR_NOT_CONNECTED;
  }

  if (!vscp_convertEventExToString(strEvent, &ex)) {
    return VSCP_ERROR_PARAMETER;
  }

  rv = request("E;" + strEvent, args, m_timeout_response);
  if (VSCP_ERROR_ERROR == rv) {
    spdlog::error("WS1 client: Server did not accept event [{}]", strEvent);
    return VSCP_ERROR_OPERATION_FAILED;
  }

  return rv;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int
vscpClientWs1::send(canalMsg &msg)
{
  vscpEventEx ex;

  uint8_t guid[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  if (!vscp_convertCanalToEventEx(&ex, &msg, guid)) {
    return VSCP_ERROR_INVALID_FRAME;
  }

  return send(ex);
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int
vscpClientWs1::receive(vscpEvent &ev)
{
  if (!isConnected()) {
    return VSCP_ERROR_NOT_CONNECTED;
  }

  std::lock_guard<std::mutex> lock(m_mutexEvent);

  // check if there are anything to fetch
  if (m_eventReceiveQueue.empty()) {
    return VSCP_ERROR_RCV_EMPTY;
  }

  // only valid if no callback is defined
  if (m_bActiveCallbackEv || m_bActiveCallbackEx) {
    return VSCP_ERROR_NOT_SUPPORTED;
  }

  vscpEvent *pev = m_eventReceiveQueue.front();
  m_eventReceiveQueue.pop_front();

  // Must be a valid pointer
  if (nullptr == pev) {
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
vscpClientWs1::receive(vscpEventEx &ex)
{
  if (!isConnected()) {
    return VSCP_ERROR_NOT_CONNECTED;
  }

  std::lock_guard<std::mutex> lock(m_mutexEvent);

  // check if there are anything to fetch
  if (m_eventReceiveQueue.empty()) {
    return VSCP_ERROR_RCV_EMPTY;
  }

  // only valid if no callback is defined
  if (m_bActiveCallbackEv || m_bActiveCallbackEx) {
    return VSCP_ERROR_NOT_SUPPORTED;
  }

  vscpEvent *pev = m_eventReceiveQueue.front();
  m_eventReceiveQueue.pop_front();

  // Must be a valid pointer
  if (nullptr == pev) {
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
vscpClientWs1::receive(canalMsg &msg)
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
// receiveBlocking
//

int
vscpClientWs1::receiveBlocking(vscpEvent &ev, long timeout)
{
  if (!isConnected()) {
    return VSCP_ERROR_NOT_CONNECTED;
  }

  {
    std::unique_lock<std::mutex> lock(m_mutexEvent);
    if (!m_cvEvent.wait_for(lock, std::chrono::milliseconds((timeout > 0) ? timeout : 0), [this] {
          return !m_eventReceiveQueue.empty() || !m_bConnected;
        })) {
      return VSCP_ERROR_TIMEOUT;
    }
  }

  return receive(ev);
}

///////////////////////////////////////////////////////////////////////////////
// receiveBlocking
//

int
vscpClientWs1::receiveBlocking(vscpEventEx &ex, long timeout)
{
  if (!isConnected()) {
    return VSCP_ERROR_NOT_CONNECTED;
  }

  {
    std::unique_lock<std::mutex> lock(m_mutexEvent);
    if (!m_cvEvent.wait_for(lock, std::chrono::milliseconds((timeout > 0) ? timeout : 0), [this] {
          return !m_eventReceiveQueue.empty() || !m_bConnected;
        })) {
      return VSCP_ERROR_TIMEOUT;
    }
  }

  return receive(ex);
}

///////////////////////////////////////////////////////////////////////////////
// receiveBlocking
//

int
vscpClientWs1::receiveBlocking(canalMsg &msg, long timeout)
{
  int rv;
  vscpEventEx ex;

  if (VSCP_ERROR_SUCCESS != (rv = receiveBlocking(ex, timeout))) {
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
vscpClientWs1::setfilter(vscpEventFilter &filter)
{
  int rv;
  std::deque<std::string> args;

  {
    std::lock_guard<std::mutex> lock(m_mutexEvent);
    m_settings.filter = filter;
  }

  if (!isConnected()) {
    return VSCP_ERROR_NOT_CONNECTED;
  }

  std::string strFilter, strMask;
  vscp_writeFilterToString(strFilter, &filter);
  vscp_writeMaskToString(strMask, &filter);

  rv = command("SETFILTER;" + strFilter + ";" + strMask, args, m_timeout_response);
  return (VSCP_ERROR_ERROR == rv) ? VSCP_ERROR_OPERATION_FAILED : rv;
}

///////////////////////////////////////////////////////////////////////////////
// getcount
//

int
vscpClientWs1::getcount(uint16_t *pcount)
{
  if (nullptr == pcount) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  std::lock_guard<std::mutex> lock(m_mutexEvent);
  *pcount = (uint16_t) (std::min)(m_eventReceiveQueue.size(), (size_t) 0xffff);
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getversion
//

int
vscpClientWs1::getversion(uint8_t *pmajor, uint8_t *pminor, uint8_t *prelease, uint8_t *pbuild)
{
  int rv;
  std::deque<std::string> args;

  if (!isConnected()) {
    return VSCP_ERROR_NOT_CONNECTED;
  }

  // Check pointers
  if ((nullptr == pmajor) || (nullptr == pminor) || (nullptr == prelease) || (nullptr == pbuild)) {
    return VSCP_ERROR_PARAMETER;
  }

  rv = command("VERSION", args, m_timeout_response);
  if (VSCP_ERROR_SUCCESS != rv) {
    return (VSCP_ERROR_ERROR == rv) ? VSCP_ERROR_OPERATION_FAILED : rv;
  }

  // "+;VERSION;<server version>;<major>.<minor>.<release>.<build>"
  // where the last field is the ws1 protocol version
  if ((args.size() < 3) || ("VERSION" != args.at(1))) {
    return VSCP_ERROR_OPERATION_FAILED;
  }

  std::deque<std::string> ver;
  vscp_split(ver, args.back(), ".");
  if (ver.size() < 4) {
    ver.clear();
    vscp_split(ver, args.back(), ",");
  }
  if (ver.size() < 4) {
    return VSCP_ERROR_MISSING;
  }

  *pmajor   = (uint8_t) vscp_readStringValue(ver.at(0));
  *pminor   = (uint8_t) vscp_readStringValue(ver.at(1));
  *prelease = (uint8_t) vscp_readStringValue(ver.at(2));
  *pbuild   = (uint8_t) vscp_readStringValue(ver.at(3));

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// clear
//

int
vscpClientWs1::clear(void)
{
  {
    std::lock_guard<std::mutex> lock(m_mutexEvent);
    while (!m_eventReceiveQueue.empty()) {
      vscpEvent *pev = m_eventReceiveQueue.front();
      m_eventReceiveQueue.pop_front();
      vscp_deleteEvent_v2(&pev);
    }
  }

  // Also clear the queue on the server
  if (isConnected()) {
    std::deque<std::string> args;
    command("CLRQ", args, m_timeout_response);
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getinterfaces
//

int
vscpClientWs1::getinterfaces(std::deque<std::string> & /*iflist*/)
{
  if (!isConnected()) {
    return VSCP_ERROR_NOT_CONNECTED;
  }

  // Not part of the ws1 protocol
  return VSCP_ERROR_NOT_SUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
// getwcyd
//

int
vscpClientWs1::getwcyd(uint64_t & /*wcyd*/)
{
  if (!isConnected()) {
    return VSCP_ERROR_NOT_CONNECTED;
  }

  // Not part of the ws1 protocol
  return VSCP_ERROR_NOT_SUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
// encrypt_password
//

int
vscpClientWs1::encrypt_password(std::string &strout,
                                std::string struser,
                                std::string strpassword,
                                uint8_t *vscpkey,
                                uint8_t *iv)
{
  return vscp_ws_encryptCredentials(strout, struser, strpassword, vscpkey, iv);
}
