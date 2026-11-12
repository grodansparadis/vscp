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

#include "vscp-client-ws2.h"

using json = nlohmann::json;

namespace {

// Upper case value of the "type" member of a message
std::string
messageType(const json &j)
{
  if (!j.is_object() || !j.contains("type") || !j["type"].is_string()) {
    return "";
  }
  std::string str = j["type"].get<std::string>();
  vscp_trim(str);
  vscp_makeUpper(str);
  return str;
}

} // namespace

///////////////////////////////////////////////////////////////////////////////
// CTOR
//

vscpClientWs2::vscpClientWs2()
  : m_bConnected(false)
  , m_bQuit(false)
  , m_bLinkUp(false)
{
  m_type         = CVscpClient::connType::WS2;
  m_settings.url = "ws://localhost:8884/ws2";
  setConnectionTimeout();
  setResponseTimeout();
}

///////////////////////////////////////////////////////////////////////////////
// DTOR
//

vscpClientWs2::~vscpClientWs2()
{
  disconnect();
  clear();
}

///////////////////////////////////////////////////////////////////////////////
// getConfigAsJson
//

std::string
vscpClientWs2::getConfigAsJson(void)
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
vscpClientWs2::initFromJson(const std::string &config)
{
  json j;

  try {
    j = json::parse(config);
  }
  catch (...) {
    spdlog::error("WS2 client: Failed to parse JSON configuration.");
    return false;
  }

  if (j.contains("name") && j["name"].is_string()) {
    setName(j["name"].get<std::string>());
  }

  std::lock_guard<std::mutex> lock(m_mutexEvent); // Protects the filter
  if (!m_settings.fromJson(j, m_timeout_connect, m_timeout_response)) {
    spdlog::error("WS2 client: Invalid configuration (check that the key has 32 hexadecimal characters).");
    return false;
  }

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
  std::string strHost = (std::string::npos != host.find(':')) ? ("[" + host + "]") : host;
  m_settings.url      = std::string(bSSL ? "wss://" : "ws://") + strHost + ":" + std::to_string((unsigned short) port) + "/ws2";
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
vscpClientWs2::getLastError(void)
{
  std::lock_guard<std::mutex> lock(m_mutexError);
  return m_lastError;
}

///////////////////////////////////////////////////////////////////////////////
// handleMessage
//

void
vscpClientWs2::handleMessage(const std::string &msg)
{
  json j;

  try {
    j = json::parse(msg);
  }
  catch (...) {
    spdlog::warn("WS2 client: Invalid JSON from server [{}]", msg);
    return;
  }

  std::string type = messageType(j);

  // Event {"type":"EVENT","event":{...}}
  if (("EVENT" == type) || ("E" == type)) {

    if (!j.contains("event") || !j["event"].is_object()) {
      spdlog::warn("WS2 client: Event message without event [{}]", msg);
      return;
    }

    std::string strEvent = j["event"].dump();
    vscpEvent *pev       = new vscpEvent;
    memset(pev, 0, sizeof(vscpEvent));
    if (!vscp_convertJSONToEvent(pev, strEvent)) {
      spdlog::warn("WS2 client: Invalid event from server [{}]", strEvent);
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

  // Reply {"type":"+"...} or {"type":"-"...}
  if (("+" == type) || ("-" == type)) {
    std::lock_guard<std::mutex> lock(m_mutexMsg);
    m_msgReceiveQueue.push_back(j);
    m_cvMsg.notify_all();
  }

  // Other types (variables) are not used
}

///////////////////////////////////////////////////////////////////////////////
// workerThread
//

void
vscpClientWs2::workerThread(void)
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
        spdlog::error("WS2 client: Connection lost. {}", err);
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
vscpClientWs2::waitForReply(json &reply, uint32_t timeout)
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
vscpClientWs2::waitForResponse(uint32_t timeout)
{
  std::unique_lock<std::mutex> lock(m_mutexMsg);
  m_cvMsg.wait_for(lock, std::chrono::milliseconds(timeout), [this] { return !m_msgReceiveQueue.empty(); });
  return m_msgReceiveQueue.empty() ? VSCP_ERROR_TIMEOUT : VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// request
//

int
vscpClientWs2::request(const json &msg, json &reply, uint32_t timeout)
{
  int rv;

  std::lock_guard<std::mutex> cmdlock(m_mutexCommand);

  if (!m_bLinkUp) {
    return VSCP_ERROR_NOT_CONNECTED;
  }

  // Drop late replies to earlier commands
  {
    std::lock_guard<std::mutex> lock(m_mutexMsg);
    m_msgReceiveQueue.clear();
  }

  if (VSCP_ERROR_SUCCESS != (rv = m_ws.sendText(msg.dump(), timeout))) {
    return rv;
  }

  if (VSCP_ERROR_SUCCESS != (rv = waitForReply(reply, timeout))) {
    return rv;
  }

  if ("-" == messageType(reply)) {
    std::string errstr = (reply.contains("errstr") && reply["errstr"].is_string()) ? reply["errstr"].get<std::string>() : "";
    std::lock_guard<std::mutex> lock(m_mutexError);
    m_lastError = errstr;
    return VSCP_ERROR_ERROR;
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// command
//

int
vscpClientWs2::command(const std::string &cmd, const json &args, json &reply, uint32_t timeout)
{
  json msg;
  msg["type"]    = "CMD";
  msg["command"] = cmd;
  msg["args"]    = args;
  return request(msg, reply, timeout);
}

///////////////////////////////////////////////////////////////////////////////
// closeConnection
//

void
vscpClientWs2::closeConnection(void)
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
vscpClientWs2::connect(void)
{
  int rv;
  json reply;
  vscpWsUrl url;
  uint8_t key[16];

  closeConnection();

  {
    std::lock_guard<std::mutex> lock(m_mutexError);
    m_lastError.clear();
  }

  auto fail = [this](int code, const std::string &err) {
    spdlog::error("WS2 client: {}", err);
    {
      std::lock_guard<std::mutex> lock(m_mutexError);
      m_lastError = err;
    }
    closeConnection();
    return code;
  };

  if (!CVscpWebSocket::parseUrl(m_settings.url, "/ws2", url)) {
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
  m_thread  = std::thread(&vscpClientWs2::workerThread, this);

  // The server starts with {"type":"+","args":["AUTH0","sid"]}
  if (VSCP_ERROR_SUCCESS != (rv = waitForReply(reply, m_timeout_connect))) {
    return fail(VSCP_ERROR_TIMEOUT, "No authentication challenge from server");
  }

  std::string sid;
  if (("+" == messageType(reply)) && reply.contains("args") && reply["args"].is_array() &&
      (reply["args"].size() >= 2) && reply["args"][0].is_string() && ("AUTH0" == reply["args"][0].get<std::string>()) &&
      reply["args"][1].is_string()) {
    sid = reply["args"][1].get<std::string>();
    vscp_trim(sid);
  }
  else {
    return fail(VSCP_ERROR_CONNECTION, "Unexpected first message from server [" + reply.dump() + "]");
  }

  uint8_t iv[16];
  if ((32 != sid.length()) || (std::string::npos != sid.find_first_not_of("0123456789abcdefABCDEF")) ||
      (16 != vscp_hexStr2ByteArray(iv, sizeof(iv), sid.c_str()))) {
    return fail(VSCP_ERROR_CONNECTION, "Invalid session id from server");
  }

  std::string crypto;
  if (VSCP_ERROR_SUCCESS != vscp_ws_encryptCredentials(crypto, m_settings.user, m_settings.password, key, iv)) {
    return fail(VSCP_ERROR_PARAMETER, "Failed to encrypt credentials");
  }

  json args;
  args["iv"]     = sid;
  args["crypto"] = crypto;
  rv             = command("AUTH", args, reply, m_timeout_response);
  if (VSCP_ERROR_ERROR == rv) {
    return fail(VSCP_ERROR_USER, "Authentication failed (check user, password and key)");
  }
  if (VSCP_ERROR_SUCCESS != rv) {
    return fail(rv, "No reply to authentication");
  }

  rv = command("OPEN", nullptr, reply, m_timeout_response);
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
    spdlog::warn("WS2 client: Server did not accept the filter, filtering locally.");
  }

  spdlog::info("WS2 client: Connected to {}", m_settings.url);
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// disconnect
//

int
vscpClientWs2::disconnect(void)
{
  if (m_bLinkUp) {
    json reply;
    command("CLOSE", nullptr, reply, (std::min)(m_timeout_response, (uint32_t) 1000));
  }

  closeConnection();
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
  int rv;
  std::string strEvent;
  json msg, reply;

  if (!isConnected()) {
    return VSCP_ERROR_NOT_CONNECTED;
  }

  if (!vscp_convertEventToJSON(strEvent, &ev)) {
    return VSCP_ERROR_PARAMETER;
  }

  try {
    msg["type"]  = "EVENT";
    msg["event"] = json::parse(strEvent);
  }
  catch (...) {
    return VSCP_ERROR_PARAMETER;
  }

  rv = request(msg, reply, m_timeout_response);
  if (VSCP_ERROR_ERROR == rv) {
    spdlog::error("WS2 client: Server did not accept event [{}]", strEvent);
    return VSCP_ERROR_OPERATION_FAILED;
  }

  return rv;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int
vscpClientWs2::send(vscpEventEx &ex)
{
  int rv;
  std::string strEvent;
  json msg, reply;

  if (!isConnected()) {
    return VSCP_ERROR_NOT_CONNECTED;
  }

  if (!vscp_convertEventExToJSON(strEvent, &ex)) {
    return VSCP_ERROR_PARAMETER;
  }

  try {
    msg["type"]  = "EVENT";
    msg["event"] = json::parse(strEvent);
  }
  catch (...) {
    return VSCP_ERROR_PARAMETER;
  }

  rv = request(msg, reply, m_timeout_response);
  if (VSCP_ERROR_ERROR == rv) {
    spdlog::error("WS2 client: Server did not accept event [{}]", strEvent);
    return VSCP_ERROR_OPERATION_FAILED;
  }

  return rv;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int
vscpClientWs2::send(canalMsg &msg)
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
vscpClientWs2::receive(vscpEvent &ev)
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
vscpClientWs2::receive(vscpEventEx &ex)
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
// receiveBlocking
//

int
vscpClientWs2::receiveBlocking(vscpEvent &ev, long timeout)
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
vscpClientWs2::receiveBlocking(vscpEventEx &ex, long timeout)
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
vscpClientWs2::receiveBlocking(canalMsg &msg, long timeout)
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
vscpClientWs2::setfilter(vscpEventFilter &filter)
{
  int rv;
  std::string strFilter;
  json args, reply;

  {
    std::lock_guard<std::mutex> lock(m_mutexEvent);
    m_settings.filter = filter;
  }

  if (!isConnected()) {
    return VSCP_ERROR_NOT_CONNECTED;
  }

  if (!vscp_writeFilterMaskToJSON(&filter, strFilter)) {
    return VSCP_ERROR_PARAMETER;
  }

  try {
    args = json::parse(strFilter);
  }
  catch (...) {
    return VSCP_ERROR_PARAMETER;
  }

  rv = command("SETFILTER", args, reply, m_timeout_response);
  return (VSCP_ERROR_ERROR == rv) ? VSCP_ERROR_OPERATION_FAILED : rv;
}

///////////////////////////////////////////////////////////////////////////////
// getcount
//

int
vscpClientWs2::getcount(uint16_t *pcount)
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
vscpClientWs2::getversion(uint8_t *pmajor, uint8_t *pminor, uint8_t *prelease, uint8_t *pbuild)
{
  int rv;
  json reply;

  if (!isConnected()) {
    return VSCP_ERROR_NOT_CONNECTED;
  }

  // Check pointers
  if ((nullptr == pmajor) || (nullptr == pminor) || (nullptr == prelease) || (nullptr == pbuild)) {
    return VSCP_ERROR_PARAMETER;
  }

  rv = command("VERSION", nullptr, reply, m_timeout_response);
  if (VSCP_ERROR_SUCCESS != rv) {
    return (VSCP_ERROR_ERROR == rv) ? VSCP_ERROR_OPERATION_FAILED : rv;
  }

  // {"type":"+","command":"VERSION","args":{"version":"major.minor.release-build"}}
  if (!reply.contains("args") || !reply["args"].is_object() || !reply["args"].contains("version") ||
      !reply["args"]["version"].is_string()) {
    return VSCP_ERROR_OPERATION_FAILED;
  }

  std::string str = reply["args"]["version"].get<std::string>();
  std::replace(str.begin(), str.end(), '-', '.');
  std::deque<std::string> ver;
  vscp_split(ver, str, ".");
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
vscpClientWs2::clear(void)
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
    json reply;
    command("CLRQ", nullptr, reply, m_timeout_response);
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getinterfaces
//

int
vscpClientWs2::getinterfaces(std::deque<std::string> & /*iflist*/)
{
  if (!isConnected()) {
    return VSCP_ERROR_NOT_CONNECTED;
  }

  // Not part of the ws2 protocol
  return VSCP_ERROR_NOT_SUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
// getwcyd
//

int
vscpClientWs2::getwcyd(uint64_t & /*wcyd*/)
{
  if (!isConnected()) {
    return VSCP_ERROR_NOT_CONNECTED;
  }

  // Not part of the ws2 protocol
  return VSCP_ERROR_NOT_SUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
// encrypt_password
//

int
vscpClientWs2::encrypt_password(std::string &strout,
                                std::string struser,
                                std::string strpassword,
                                uint8_t *vscpkey,
                                uint8_t *iv)
{
  return vscp_ws_encryptCredentials(strout, struser, strpassword, vscpkey, iv);
}
