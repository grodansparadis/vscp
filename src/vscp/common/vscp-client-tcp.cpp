// vscp_client_tcp.cpp
//
// tcp/ip client communication classes.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// Copyright:  (C) 2007-2026
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

#include <mustache.hpp>
#include <nlohmann/json.hpp> // Needs C++11  -std=c++11

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <exception>
#include <limits>
#include <memory>
#include <new>
#include <system_error>
#include <utility>

#include "vscp-client-tcp.h"
#include "vscphelperlogging.h"

void workerThread(vscpClientTcp *pObj);
void pollWorkerThread(vscpClientTcp *pObj);

namespace {

class SingleChannelPause {
public:
  SingleChannelPause(std::atomic<bool> &requested, std::mutex &mutex, std::condition_variable &condition)
    : m_requested(requested), m_mutex(mutex), m_condition(condition)
  {
    m_requested = true;
  }

  ~SingleChannelPause()
  {
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_requested = false;
    }
    m_condition.notify_all();
  }

  SingleChannelPause(const SingleChannelPause &) = delete;
  SingleChannelPause &operator=(const SingleChannelPause &) = delete;

private:
  std::atomic<bool> &m_requested;
  std::mutex &m_mutex;
  std::condition_variable &m_condition;
};

int
tcpClientFailure(const char *function, int error)
{
  vscphelper_logFunctionFailure("vscpClientTcp", function, error);
  return error;
}

}

///////////////////////////////////////////////////////////////////////////////
// C-tor
//

vscpClientTcp::vscpClientTcp()
{
  m_type          = CVscpClient::connType::TCPIP;
  m_pworkerthread = nullptr;
  m_bRun          = false;

  // Default connect parameters
  m_strHostname = "tcp://localhost:9598";
  m_strUsername = "admin";
  m_strPassword = "secret";
  m_guidif.clear();
  m_bPolling                  = false;
  m_bSingleChannelReceiveLoop = false;
  m_obid                      = 0;

  m_bTLS        = false;
  m_tlsMode     = tls_mode::auto_select;
  m_bVerifyPeer = true;

  vscp_clearVSCPFilter(&m_filterIn);  // Accept all events
  vscp_clearVSCPFilter(&m_filterOut); // Send all events

#ifdef WIN32
  m_semReceiveQueue = CreateSemaphore(NULL, 0, 0x7fffffff, NULL);
#else
  sem_init(&m_semReceiveQueue, 0, 0);
#endif

  pthread_mutex_init(&m_mutexTcpIpObject, NULL);
  pthread_mutex_init(&m_mutexReceiveQueue, NULL);
}

///////////////////////////////////////////////////////////////////////////////
// D-tor
//

vscpClientTcp::~vscpClientTcp()
{
  // Just to be sure
  disconnect();

#ifdef WIN32
  CloseHandle(m_semReceiveQueue);
#else
  sem_destroy(&m_semReceiveQueue);
#endif

  pthread_mutex_destroy(&m_mutexTcpIpObject);
  pthread_mutex_destroy(&m_mutexReceiveQueue);

  // Clear the input queue (if needed)
  while (m_receiveList.size()) {
    vscpEvent *pev = m_receiveList.front();
    m_receiveList.pop_front();
    vscp_deleteEvent(pev);
  }
}

///////////////////////////////////////////////////////////////////////////////
// getConfigAsJson
//

std::string
vscpClientTcp::getConfigAsJson(void)
{
  std::string str;
  json j;
  vscpEventFilter filter;
  if (m_bSingleChannelReceiveLoop) {
    std::lock_guard<std::mutex> lock(m_singleChannelMutex);
    filter = m_filterIn;
  }
  else {
    filter = m_filterIn;
  }

  j["host"]               = m_strHostname;
  j["user"]               = m_strUsername;
  j["password"]           = m_strPassword;
  j["bpoll"]              = m_bPolling;
  j["single-channel"]     = m_bSingleChannelReceiveLoop;
  j["connection-timeout"] = 0;
  j["response-timeout"]   = 0;
  j["bfull-l2"]           = isFullLevel2();
  j["selected-interface"] = m_guidif.toString();

  // TLS

  j["btls"]        = (tls_mode::force_tls == m_tlsMode);
  j["tls-mode"]    = (tls_mode::force_tls == m_tlsMode)
                       ? "force-tls"
                       : ((tls_mode::force_plain == m_tlsMode) ? "force-plain" : "auto");
  j["bverifypeer"] = m_bVerifyPeer;
  j["cafile"]      = m_cafile;
  j["capath"]      = m_capath;
  j["certfile"]    = m_certfile;
  j["keyfile"]     = m_keyfile;
  j["pwkeyfile"]   = m_pwKeyfile;

  // Filter In

  j["priority-filter"] = filter.filter_priority;
  j["priority-mask"]   = filter.mask_priority;
  j["class-filter"]    = filter.filter_class;
  j["class-mask"]      = filter.mask_class;
  j["type-filter"]     = filter.filter_type;
  j["type-mask"]       = filter.mask_type;
  vscp_writeGuidArrayToString(str, filter.filter_GUID);
  j["guid-filter"] = str;
  vscp_writeGuidArrayToString(str, filter.mask_GUID);
  j["guid-mask"] = str;

  return j.dump();
}

///////////////////////////////////////////////////////////////////////////////
// initFromJson
//

bool
vscpClientTcp::initFromJson(const std::string &config)
{
  json j;
  bool bHasTlsMode = false;

  try {
    j = json::parse(config);

    const bool polling = j.value("bpoll", m_bPolling);
    const bool singleChannel = j.value("single-channel", m_bSingleChannelReceiveLoop);
    if ((polling && singleChannel) ||
        (m_bSingleChannelReceiveLoop &&
         (m_singleChannelRunning || m_singleChannelWorkerThread.joinable())) ||
        (singleChannel != m_bSingleChannelReceiveLoop &&
         (m_singleChannelWorkerThread.joinable() || nullptr != m_pworkerthread ||
          m_tcp.isConnected() || m_tcpReceive.isConnected()))) {
      tcpClientFailure(__func__, VSCP_ERROR_PARAMETER);
      return false;
    }

    if (j.contains("host")) {
      m_strHostname = j["host"].get<std::string>();
    }

    if (j.contains("user")) {
      m_strUsername = j["user"].get<std::string>();
    }

    if (j.contains("password")) {
      m_strPassword = j["password"].get<std::string>();
    }

    if (j.contains("connection-timeout")) {
      // m_bPolling = j["connection-timeout"].get<int>();
    }

    if (j.contains("response-timeout")) {
      // m_bPolling = j["response-timeout"].get<int>();
    }

    if (j.contains("bfull-l2")) {
      setFullLevel2(j["bfull-l2"].get<bool>());
    }

    if (j.contains("selected-interface")) {
      std::string str = j["selected-interface"].get<std::string>();
      m_guidif.getFromString(str);
    }

    // TLS

    if (j.contains("btls")) {
      m_bTLS = j["btls"].get<bool>();
    }

    if (j.contains("tls-mode")) {
      std::string mode = vscp_lower(j["tls-mode"].get<std::string>());
      if ("force-tls" == mode || "tls" == mode || "on" == mode || "true" == mode) {
        m_tlsMode = tls_mode::force_tls;
      }
      else if ("force-plain" == mode || "plain" == mode || "off" == mode || "false" == mode) {
        m_tlsMode = tls_mode::force_plain;
      }
      else {
        m_tlsMode = tls_mode::auto_select;
      }

      // Keep legacy flag consistent with the selected mode
      m_bTLS       = (tls_mode::force_tls == m_tlsMode);
      bHasTlsMode  = true;
    }

    if (j.contains("bverifypeer")) {
      m_bVerifyPeer = j["bverifypeer"].get<bool>();
    }

    if (j.contains("cafile")) {
      m_cafile = j["cafile"].get<std::string>();
    }

    if (j.contains("capath")) {
      m_capath = j["capath"].get<std::string>();
    }

    if (j.contains("certfile")) {
      m_certfile = j["certfile"].get<std::string>();
    }

    if (j.contains("keyfile")) {
      m_keyfile = j["keyfile"].get<std::string>();
    }

    if (j.contains("pwkeyfile")) {
      m_pwKeyfile = j["pwkeyfile"].get<std::string>();
    }

    if (!bHasTlsMode) {
      m_tlsMode = m_bTLS ? tls_mode::force_tls : tls_mode::auto_select;
    }

    // Filter

    if (j.contains("priority-filter")) {
      m_filterIn.filter_priority = j["priority-filter"].get<uint8_t>();
    }

    if (j.contains("priority-mask")) {
      m_filterIn.mask_priority = j["priority-mask"].get<uint8_t>();
    }

    if (j.contains("class-filter")) {
      m_filterIn.filter_class = j["class-filter"].get<uint16_t>();
    }

    if (j.contains("class-mask")) {
      m_filterIn.mask_class = j["class-mask"].get<uint16_t>();
    }

    if (j.contains("type-filter")) {
      m_filterIn.filter_type = j["type-filter"].get<uint16_t>();
    }

    if (j.contains("type-mask")) {
      m_filterIn.mask_type = j["type-mask"].get<uint16_t>();
    }

    if (j.contains("guid-filter")) {
      std::string str = j["guid-filter"].get<std::string>();
      vscp_getGuidFromStringToArray(m_filterIn.filter_GUID, str);
    }

    if (j.contains("guid-mask")) {
      std::string str = j["guid-mask"].get<std::string>();
      vscp_getGuidFromStringToArray(m_filterIn.mask_GUID, str);
    }

    m_bPolling = polling;
    m_bSingleChannelReceiveLoop = singleChannel;
  }
  catch (const json::exception &) {
    tcpClientFailure(__func__, VSCP_ERROR_PARAMETER);
    return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// init
//

int
vscpClientTcp::init(const std::string &strHostname,
                    const std::string &strUsername,
                    const std::string &strPassword,
                    bool bPolling)
{
  if (m_bSingleChannelReceiveLoop &&
      (bPolling || m_singleChannelRunning || m_singleChannelWorkerThread.joinable())) {
    return tcpClientFailure(__func__, VSCP_ERROR_PARAMETER);
  }

  m_strHostname = strHostname;
  m_strUsername = strUsername;
  m_strPassword = strPassword;
  m_bPolling    = bPolling;
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setSingleChannelReceiveLoop
//

int
vscpClientTcp::setSingleChannelReceiveLoop(bool enable)
{
  if ((enable && m_bPolling) || m_singleChannelRunning || m_singleChannelWorkerThread.joinable() ||
      nullptr != m_pworkerthread || m_tcp.isConnected() || m_tcpReceive.isConnected()) {
    return tcpClientFailure(__func__, VSCP_ERROR_PARAMETER);
  }

  m_bSingleChannelReceiveLoop = enable;
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setCallbackEv
//

int
vscpClientTcp::setCallbackEv(std::function<void(vscpEvent &ev, void *pobj)> callback, void *pData)
{
  if (m_bSingleChannelReceiveLoop && (m_singleChannelRunning || m_singleChannelWorkerThread.joinable())) {
    return tcpClientFailure(__func__, VSCP_ERROR_NOT_SUPPORTED);
  }
  return CVscpClient::setCallbackEv(std::move(callback), pData);
}

///////////////////////////////////////////////////////////////////////////////
// setCallbackEx
//

int
vscpClientTcp::setCallbackEx(std::function<void(vscpEventEx &ex, void *pobj)> callback, void *pData)
{
  if (m_bSingleChannelReceiveLoop && (m_singleChannelRunning || m_singleChannelWorkerThread.joinable())) {
    return tcpClientFailure(__func__, VSCP_ERROR_NOT_SUPPORTED);
  }
  return CVscpClient::setCallbackEx(std::move(callback), pData);
}

///////////////////////////////////////////////////////////////////////////////
// connect
//

int
vscpClientTcp::connect(void)
{
  int rv;

  if (m_bSingleChannelReceiveLoop &&
      (m_singleChannelRunning || m_singleChannelWorkerThread.joinable() || nullptr != m_pworkerthread ||
       m_tcp.isConnected() || m_tcpReceive.isConnected())) {
    return tcpClientFailure(__func__, VSCP_ERROR_ALREADY_DEFINED);
  }

  // Propagate TLS mode/settings to the remote tcp/ip interfaces
  m_tcp.setTLSOptions(m_bVerifyPeer, m_cafile, m_capath,
                      m_certfile, m_keyfile, m_pwKeyfile);
  m_tcpReceive.setTLSOptions(m_bVerifyPeer, m_cafile, m_capath,
                             m_certfile, m_keyfile, m_pwKeyfile);
  if (tls_mode::force_tls == m_tlsMode) {
    m_tcp.setTLSMode(VscpRemoteTcpIf::tls_mode::force_tls);
    m_tcpReceive.setTLSMode(VscpRemoteTcpIf::tls_mode::force_tls);
  }
  else if (tls_mode::force_plain == m_tlsMode) {
    m_tcp.setTLSMode(VscpRemoteTcpIf::tls_mode::force_plain);
    m_tcpReceive.setTLSMode(VscpRemoteTcpIf::tls_mode::force_plain);
  }
  else {
    m_tcp.setTLSMode(VscpRemoteTcpIf::tls_mode::auto_select);
    m_tcpReceive.setTLSMode(VscpRemoteTcpIf::tls_mode::auto_select);
  }

  if (VSCP_ERROR_SUCCESS != (rv = m_tcp.setSingleChannelReceiveLoop(m_bSingleChannelReceiveLoop))) {
    return rv;
  }

  if (m_bSingleChannelReceiveLoop) {
    if (VSCP_ERROR_SUCCESS != (rv = m_tcp.doCmdOpen(m_strHostname, m_strUsername, m_strPassword))) {
      return rv;
    }
    if (VSCP_ERROR_SUCCESS != (rv = m_tcp.doCmdGetChannelID(&m_obid))) {
      m_tcp.doCmdClose();
      return rv;
    }
    if (VSCP_ERROR_SUCCESS != (rv = m_tcp.doCmdEnterReceiveLoop())) {
      m_tcp.doCmdClose();
      return rv;
    }

    {
      std::lock_guard<std::mutex> lock(m_singleChannelMutex);
      m_singleChannelEvents.clear();
      m_singleChannelError = VSCP_ERROR_SUCCESS;
      m_singleChannelRunning = true;
    }
    try {
      m_singleChannelWorkerThread = std::thread(&vscpClientTcp::singleChannelWorker, this);
    }
    catch (const std::system_error &) {
      {
        std::lock_guard<std::mutex> lock(m_singleChannelMutex);
        m_singleChannelError = VSCP_ERROR_ERROR;
        m_singleChannelRunning = false;
      }
      m_singleChannelCondition.notify_all();
      m_tcp.doCmdClose();
      return tcpClientFailure(__func__, VSCP_ERROR_ERROR);
    }
    return VSCP_ERROR_SUCCESS;
  }

  if (m_bPolling) {
    // Open the single shared connection
    if (VSCP_ERROR_SUCCESS != (rv = m_tcp.doCmdOpen(m_strHostname, m_strUsername, m_strPassword))) {
      return rv;
    }
    m_tcp.doCmdGetChannelID(&m_obid);
    // Start the poll worker thread that drains the connection and fires callbacks
    m_bRun          = true;
    m_pworkerthread = new std::thread(pollWorkerThread, this);
  }
  else {
    
    // Open main interface
    if (VSCP_ERROR_SUCCESS != (rv = m_tcp.doCmdOpen(m_strHostname, m_strUsername, m_strPassword))) {
      return rv;
    }

    // Get channel id
    // Received can message check obid in received events to see if they
    // are sent from ourself and if so skip them
    m_tcp.doCmdGetChannelID(&m_obid);

    // Open receive interface
    if (VSCP_ERROR_SUCCESS != (rv = m_tcpReceive.doCmdOpen(m_strHostname, m_strUsername, m_strPassword))) {
      m_tcp.doCmdClose();
      return rv;
    }

    // Create receive worker thread
    m_bRun          = true;
    m_pworkerthread = new std::thread(workerThread, this);
    m_bRun          = true;
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// disconnect
//

int
vscpClientTcp::disconnect(void)
{
  int rv;

  if (m_bSingleChannelReceiveLoop) {
    {
      std::lock_guard<std::mutex> lock(m_singleChannelMutex);
      if (m_singleChannelWorkerId == std::this_thread::get_id()) {
        return tcpClientFailure(__func__, VSCP_ERROR_NOT_SUPPORTED);
      }
      m_singleChannelRunning = false;
      m_singleChannelError = VSCP_ERROR_NOT_CONNECTED;
      m_singleChannelEvents.clear();
    }
    m_singleChannelCondition.notify_all();
    {
      std::lock_guard<std::mutex> lock(m_singleChannelDispatchMutex);
      m_singleChannelDispatchCondition.notify_all();
    }
    if (m_singleChannelWorkerThread.joinable()) {
      m_singleChannelWorkerThread.join();
    }
    {
      std::lock_guard<std::mutex> lock(m_singleChannelMutex);
      m_singleChannelWorkerId = std::thread::id();
    }
    std::lock_guard<std::mutex> lock(m_singleChannelCommandMutex);
    return m_tcp.doCmdClose();
  }

  if (m_bPolling) {
    m_bRun = false;
    if (nullptr != m_pworkerthread) {
      m_pworkerthread->join();
      delete m_pworkerthread;
      m_pworkerthread = nullptr;
    }
    return m_tcp.doCmdClose();
  }
  else {

    if (nullptr != m_pworkerthread) {
      m_bRun = false;
      m_pworkerthread->join();
      m_pworkerthread = nullptr;
    }

    // Close receive connection
    if (VSCP_ERROR_SUCCESS != (rv = m_tcpReceive.doCmdClose())) {
      m_tcp.doCmdClose();
      return rv;
    }

    // Close main connection
    return m_tcp.doCmdClose();
  }
}

///////////////////////////////////////////////////////////////////////////////
// isConnected
//

bool
vscpClientTcp::isConnected(void)
{
  if (m_bSingleChannelReceiveLoop) {
    return m_singleChannelRunning;
  }

  if (m_bPolling) {
    return m_tcp.isConnected();
  }
  else {
    return (m_tcp.isConnected() && m_tcpReceive.isConnected());
  }
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int
vscpClientTcp::send(vscpEvent &ev)
{
  if (m_bSingleChannelReceiveLoop) {
    std::lock_guard<std::mutex> lock(m_singleChannelCommandMutex);
    if (!m_singleChannelRunning) {
      return tcpClientFailure(__func__, VSCP_ERROR_NOT_CONNECTED);
    }
    return m_tcp.doCmdSend(&ev);
  }

  if (m_bPolling) {
    pthread_mutex_lock(&m_mutexTcpIpObject);
    int rv = m_tcp.doCmdSend(&ev);
    pthread_mutex_unlock(&m_mutexTcpIpObject);
    return rv;
  }
  return m_tcp.doCmdSend(&ev);
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int
vscpClientTcp::send(vscpEventEx &ex)
{
  if (m_bSingleChannelReceiveLoop) {
    std::lock_guard<std::mutex> lock(m_singleChannelCommandMutex);
    if (!m_singleChannelRunning) {
      return tcpClientFailure(__func__, VSCP_ERROR_NOT_CONNECTED);
    }
    return m_tcp.doCmdSendEx(&ex);
  }

  if (m_bPolling) {
    pthread_mutex_lock(&m_mutexTcpIpObject);
    int rv = m_tcp.doCmdSendEx(&ex);
    pthread_mutex_unlock(&m_mutexTcpIpObject);
    return rv;
  }
  return m_tcp.doCmdSendEx(&ex);
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int
vscpClientTcp::send(canalMsg &msg)
{
  vscpEvent event = {};
  std::unique_ptr<vscpEvent, decltype(&vscp_deleteEvent)> cleanup(&event, vscp_deleteEvent);

  uint8_t guid[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  if (!vscp_convertCanalToEvent(&event, &msg, guid)) {
    return VSCP_ERROR_INVALID_FRAME;
  }

  return send(event);
}

///////////////////////////////////////////////////////////////////////////////
// receiveSingleChannel
//

int
vscpClientTcp::receiveSingleChannel(vscpEventEx &event, long timeout, bool blocking)
{
  if (timeout < 0) {
    return tcpClientFailure(__func__, VSCP_ERROR_PARAMETER);
  }
  if (isCallbackEvActive() || isCallbackExActive()) {
    return tcpClientFailure(__func__, VSCP_ERROR_NOT_SUPPORTED);
  }

  std::unique_lock<std::mutex> lock(m_singleChannelMutex);
  auto ready = [this]() { return !m_singleChannelEvents.empty() || !m_singleChannelRunning; };
  if (!ready()) {
    if (!blocking) {
      return VSCP_ERROR_RCV_EMPTY;
    }
    if (!m_singleChannelCondition.wait_for(lock, std::chrono::milliseconds(timeout), ready)) {
      return VSCP_ERROR_TIMEOUT;
    }
  }
  if (m_singleChannelEvents.empty()) {
    return m_singleChannelError == VSCP_ERROR_SUCCESS ? VSCP_ERROR_NOT_CONNECTED : m_singleChannelError;
  }

  event = m_singleChannelEvents.front();
  m_singleChannelEvents.pop_front();
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int
vscpClientTcp::receive(vscpEvent &ev)
{
  if (m_bSingleChannelReceiveLoop) {
    vscpEventEx event = {};
    int rv = receiveSingleChannel(event, 0, false);
    if (VSCP_ERROR_SUCCESS != rv) {
      return rv;
    }
    return vscp_convertEventExToEvent(&ev, &event) ? VSCP_ERROR_SUCCESS :
      tcpClientFailure(__func__, VSCP_ERROR_INVALID_FRAME);
  }

  return m_tcp.doCmdReceive(&ev);
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int
vscpClientTcp::receive(vscpEventEx &ex)
{
  if (m_bSingleChannelReceiveLoop) {
    return receiveSingleChannel(ex, 0, false);
  }

  return m_tcp.doCmdReceiveEx(&ex);
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int
vscpClientTcp::receive(canalMsg &msg)
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
vscpClientTcp::receiveBlocking(vscpEvent &ev, long timeout)
{
  if (m_bSingleChannelReceiveLoop) {
    vscpEventEx event = {};
    int rv = receiveSingleChannel(event, timeout, true);
    if (VSCP_ERROR_SUCCESS != rv) {
      return rv;
    }
    return vscp_convertEventExToEvent(&ev, &event) ? VSCP_ERROR_SUCCESS :
      tcpClientFailure(__func__, VSCP_ERROR_INVALID_FRAME);
  }

  if (-1 == vscp_sem_wait(&m_semReceiveQueue, timeout)) {
    if (errno == ETIMEDOUT) {
      return VSCP_ERROR_TIMEOUT;
    }
    else {
      return VSCP_ERROR_ERROR;
    }
  }

  return receive(ev);
}

///////////////////////////////////////////////////////////////////////////////
// receiveBlocking
//

int
vscpClientTcp::receiveBlocking(vscpEventEx &ex, long timeout)
{
  if (m_bSingleChannelReceiveLoop) {
    return receiveSingleChannel(ex, timeout, true);
  }

  if (-1 == vscp_sem_wait(&m_semReceiveQueue, timeout)) {
    if (errno == ETIMEDOUT) {
      return VSCP_ERROR_TIMEOUT;
    }
    else {
      return VSCP_ERROR_ERROR;
    }
  }

  return receive(ex);
}

///////////////////////////////////////////////////////////////////////////////
// receiveBlocking
//

int
vscpClientTcp::receiveBlocking(canalMsg &msg, long timeout)
{
  if (m_bSingleChannelReceiveLoop) {
    vscpEventEx event = {};
    int rv = receiveSingleChannel(event, timeout, true);
    if (VSCP_ERROR_SUCCESS != rv) {
      return rv;
    }
    return vscp_convertEventExToCanal(&msg, &event) ? VSCP_ERROR_SUCCESS :
      tcpClientFailure(__func__, VSCP_ERROR_INVALID_FRAME);
  }

  if (-1 == vscp_sem_wait(&m_semReceiveQueue, timeout)) {
    if (errno == ETIMEDOUT) {
      return VSCP_ERROR_TIMEOUT;
    }
    else {
      return VSCP_ERROR_ERROR;
    }
  }

  return receive(msg);
}

///////////////////////////////////////////////////////////////////////////////
// setfilter
//

int
vscpClientTcp::setfilter(vscpEventFilter &filter)
{
  if (m_bSingleChannelReceiveLoop) {
    std::lock_guard<std::mutex> lock(m_singleChannelMutex);
    m_filterIn = filter;
    return VSCP_ERROR_SUCCESS;
  }

  if (m_bPolling) {
    return m_tcp.doCmdFilter(&filter);
  }
  else {
    pthread_mutex_lock(&m_mutexTcpIpObject);
    return m_tcpReceive.doCmdFilter(&filter);
    pthread_mutex_unlock(&m_mutexTcpIpObject);
  }
}

///////////////////////////////////////////////////////////////////////////////
// getcount
//

int
vscpClientTcp::getcount(uint16_t *pcount)
{
  if (NULL == pcount) {
    return VSCP_ERROR_INVALID_POINTER;
  }
  if (m_bSingleChannelReceiveLoop) {
    std::lock_guard<std::mutex> lock(m_singleChannelMutex);
    if (!m_singleChannelRunning && m_singleChannelEvents.empty()) {
      return m_singleChannelError == VSCP_ERROR_SUCCESS ? VSCP_ERROR_NOT_CONNECTED : m_singleChannelError;
    }
    if (m_singleChannelEvents.size() > (std::numeric_limits<uint16_t>::max)()) {
      return tcpClientFailure(__func__, VSCP_ERROR_OVERRUN);
    }
    *pcount = static_cast<uint16_t>(m_singleChannelEvents.size());
    return VSCP_ERROR_SUCCESS;
  }

  *pcount = m_tcp.doCmdDataAvailable();
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// clear
//

int
vscpClientTcp::clear(void)
{
  if (m_bSingleChannelReceiveLoop) {
    std::lock_guard<std::mutex> commandLock(m_singleChannelCommandMutex);
    if (!m_singleChannelRunning) {
      return tcpClientFailure(__func__, VSCP_ERROR_NOT_CONNECTED);
    }
    SingleChannelPause pause(m_singleChannelPauseRequested,
                             m_singleChannelDispatchMutex,
                             m_singleChannelDispatchCondition);
    std::lock_guard<std::mutex> dispatchLock(m_singleChannelDispatchMutex);
    if (!m_singleChannelRunning) {
      return tcpClientFailure(__func__, VSCP_ERROR_NOT_CONNECTED);
    }
    int rv = m_tcp.doCmdClear();
    if (VSCP_ERROR_SUCCESS == rv) {
      std::lock_guard<std::mutex> lock(m_singleChannelMutex);
      m_singleChannelEvents.clear();
    }
    return rv;
  }

  return m_tcp.doCmdClear();
}

///////////////////////////////////////////////////////////////////////////////
// getversion
//

int
vscpClientTcp::getversion(uint8_t *pmajor, uint8_t *pminor, uint8_t *prelease, uint8_t *pbuild)
{
  if (m_bSingleChannelReceiveLoop) {
    return tcpClientFailure(__func__, VSCP_ERROR_NOT_SUPPORTED);
  }

  *pbuild = 0;
  return m_tcp.doCmdVersion(pmajor, pminor, prelease);
}

///////////////////////////////////////////////////////////////////////////////
// getinterfaces
//

int
vscpClientTcp::getinterfaces(std::deque<std::string> &iflist)
{
  if (m_bSingleChannelReceiveLoop) {
    return tcpClientFailure(__func__, VSCP_ERROR_NOT_SUPPORTED);
  }

  return m_tcp.doCmdInterfaceList(iflist);
}

///////////////////////////////////////////////////////////////////////////////
// getwcyd
//

int
vscpClientTcp::getwcyd(uint64_t & /*wcyd*/)
{
  if (m_bSingleChannelReceiveLoop) {
    return tcpClientFailure(__func__, VSCP_ERROR_NOT_SUPPORTED);
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setConnectionTimeout
//

void
vscpClientTcp::setConnectionTimeout(uint32_t timeout)
{
  m_tcp.setConnectTimeout(timeout / 1000);
}

///////////////////////////////////////////////////////////////////////////////
// getConnectionTimeout
//

uint32_t
vscpClientTcp::getConnectionTimeout(void)
{
  return m_tcp.getConnectTimeout();
}

///////////////////////////////////////////////////////////////////////////////
// setResponeTimeout
//

void
vscpClientTcp::setResponseTimeout(uint32_t timeout)
{
  m_tcp.setResponseTimeout(timeout);
}

///////////////////////////////////////////////////////////////////////////////
// getResponseTimeout
//

uint32_t
vscpClientTcp::getResponseTimeout(void)
{
  return m_tcp.getResponseTimeout();
}

///////////////////////////////////////////////////////////////////////////////
// sendToCallbacks
//

void
vscpClientTcp::sendToCallbacks(vscpEvent *pev)
{
  if (isCallbackEvActive()) {
    m_callbackev(*pev, m_callbackObject);
  }

  if (isCallbackExActive()) {
    vscpEventEx ex;
    vscp_convertEventToEventEx(&ex, pev);
    m_callbackex(ex, m_callbackObject);
  }
}

///////////////////////////////////////////////////////////////////////////////
// failSingleChannelWorker
//

void
vscpClientTcp::failSingleChannelWorker(int error)
{
  {
    std::lock_guard<std::mutex> lock(m_singleChannelMutex);
    if (!m_singleChannelRunning) {
      return;
    }
    m_singleChannelError = error;
    m_singleChannelRunning = false;
  }
  tcpClientFailure(__func__, error);
  m_singleChannelCondition.notify_all();
}

///////////////////////////////////////////////////////////////////////////////
// singleChannelWorker
//

void
vscpClientTcp::singleChannelWorker(void)
{
  {
    std::lock_guard<std::mutex> lock(m_singleChannelMutex);
    m_singleChannelWorkerId = std::this_thread::get_id();
  }

  try {
    while (m_singleChannelRunning) {
      vscpEvent event = {};
      std::unique_ptr<vscpEvent, decltype(&vscp_deleteEvent)> cleanup(&event, vscp_deleteEvent);
      std::unique_lock<std::mutex> dispatchLock(m_singleChannelDispatchMutex);
      m_singleChannelDispatchCondition.wait(dispatchLock, [this]() {
        return !m_singleChannelPauseRequested || !m_singleChannelRunning;
      });
      if (!m_singleChannelRunning) {
        break;
      }

      int rv = m_tcp.doCmdBlockingReceive(&event, 100);
      if (!m_singleChannelRunning) {
        break;
      }
      if (VSCP_ERROR_TIMEOUT == rv) {
        continue;
      }
      if (VSCP_ERROR_SUCCESS != rv) {
        dispatchLock.unlock();
        failSingleChannelWorker(rv);
        break;
      }

      bool callbacks = false;
      bool queued = false;
      int dispatchError = VSCP_ERROR_SUCCESS;
      {
        std::lock_guard<std::mutex> lock(m_singleChannelMutex);
        if (vscp_doLevel2Filter(&event, &m_filterIn)) {
          callbacks = isCallbackEvActive() || isCallbackExActive();
          if (!callbacks) {
            vscpEventEx converted = {};
            if (!vscp_convertEventToEventEx(&converted, &event)) {
              dispatchError = VSCP_ERROR_INVALID_FRAME;
            }
            else {
              m_singleChannelEvents.push_back(converted);
              queued = true;
            }
          }
        }
      }
      dispatchLock.unlock();

      if (VSCP_ERROR_SUCCESS != dispatchError) {
        failSingleChannelWorker(dispatchError);
        break;
      }
      if (queued) {
        m_singleChannelCondition.notify_one();
      }
      if (callbacks && m_singleChannelRunning) {
        sendToCallbacks(&event);
      }
    }
  }
  catch (const std::bad_alloc &) {
    failSingleChannelWorker(VSCP_ERROR_MEMORY);
  }
  catch (const std::exception &) {
    failSingleChannelWorker(VSCP_ERROR_ERROR);
  }

  std::lock_guard<std::mutex> lock(m_singleChannelMutex);
  m_singleChannelWorkerId = std::thread::id();
}

// ----------------------------------------------------------------------------
//                            Receive worker thread
// ----------------------------------------------------------------------------

void
workerThread(vscpClientTcp *pClient)
{
  vscpEvent ev;

  ev.sizeData = 0;
  ev.pdata    = NULL;

  // Check pointer
  if (nullptr == pClient) {
    return;
  }

  VscpRemoteTcpIf *m_pifReceive = pClient->getTcpReceive();

  m_pifReceive->doCmdEnterReceiveLoop();

  while (pClient->m_bRun) {

    pthread_mutex_lock(&pClient->m_mutexTcpIpObject);

    if (VSCP_ERROR_SUCCESS == m_pifReceive->doCmdBlockingReceive(&ev)) {

      if (vscp_doLevel2Filter(&ev, &pClient->m_filterIn)) {

        if (pClient->isCallbackEvActive()) {
          pClient->m_callbackev(ev, pClient->getCallbackObj());
        }

        if (pClient->isCallbackExActive()) {
          vscpEventEx ex;
          if (vscp_convertEventToEventEx(&ex, &ev)) {
            pClient->m_callbackex(ex, pClient->getCallbackObj());
          }
        }

        // Add to input queue only if no callback set
        if (!pClient->isCallbackEvActive() || !pClient->isCallbackExActive()) {
          pthread_mutex_lock(&pClient->m_mutexReceiveQueue);
          pClient->m_receiveList.push_back(&ev);
#ifdef WIN32
          ReleaseSemaphore(pClient->m_semReceiveQueue, 1, NULL);
#else
          sem_post(&pClient->m_semReceiveQueue);
#endif
          pthread_mutex_unlock(&pClient->m_mutexReceiveQueue);
        }
      }
      vscp_deleteEvent(&ev);
    }

    // Terminate if we are not connected
    if (!m_pifReceive->isConnected()) {
      pClient->m_bRun = false;
    }

    pthread_mutex_unlock(&pClient->m_mutexTcpIpObject);
} // workerThread loop
}

// ----------------------------------------------------------------------------
//                             Poll worker thread
// Used when m_bPolling = true: one connection, non-blocking receive loop.
// Events are dispatched through sendToCallbacks() so the caller's registered
// callback receives them exactly as in non-poll mode.
// ----------------------------------------------------------------------------

void
pollWorkerThread(vscpClientTcp *pClient)
{
  if (nullptr == pClient) {
    return;
  }

  while (pClient->m_bRun) {

    vscpEvent ev;
    ev.sizeData = 0;
    ev.pdata    = nullptr;

    pthread_mutex_lock(&pClient->m_mutexTcpIpObject);
    int rv = pClient->receive(ev);
    pthread_mutex_unlock(&pClient->m_mutexTcpIpObject);

    if (VSCP_ERROR_SUCCESS == rv) {

      if (vscp_doLevel2Filter(&ev, &pClient->m_filterIn)) {

        // Fire registered callbacks
        pClient->sendToCallbacks(&ev);

        // Also push to receive queue for callers using blocking receive
        if (!pClient->isCallbackEvActive() && !pClient->isCallbackExActive()) {
          vscpEvent *pnew = new vscpEvent;
          pnew->sizeData  = 0;
          pnew->pdata     = nullptr;
          vscp_copyEvent(pnew, &ev);
          pthread_mutex_lock(&pClient->m_mutexReceiveQueue);
          pClient->m_receiveList.push_back(pnew);
#ifdef WIN32
          ReleaseSemaphore(pClient->m_semReceiveQueue, 1, NULL);
#else
          sem_post(&pClient->m_semReceiveQueue);
#endif
          pthread_mutex_unlock(&pClient->m_mutexReceiveQueue);
        }
      }

      vscp_deleteEvent(&ev);

    } else {
      // No event available — yield before next poll to avoid busy-waiting
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Stop if connection dropped
    if (!pClient->isConnected()) {
      pClient->m_bRun = false;
    }
  }
}