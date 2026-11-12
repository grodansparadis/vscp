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

#include <algorithm>
#include <chrono>
#include <exception>
#include <limits>
#include <memory>
#include <new>
#include <system_error>
#include <utility>

#include <crc.h>

#include "vscp-client-tcp.h"
#include <openssl/crypto.h>
#include "vscphelperlogging.h"

void workerThread(vscpClientTcp *pObj);
void pollWorkerThread(vscpClientTcp *pObj);

namespace {

constexpr uint16_t binaryRetrCommand = 6;
constexpr uint16_t binaryOpenCommand = 7;
constexpr uint16_t binaryClearCommand = 10;
constexpr uint16_t binaryEventConfirmation = 0xffff;

uint16_t
binaryRead16(const uint8_t *data)
{
  return (static_cast<uint16_t>(data[0]) << 8) | data[1];
}

bool
binaryReply(const std::vector<uint8_t> &frame, uint16_t &command, uint16_t &error)
{
  if (frame.size() < 7 || frame[0] != VSCP_BINARY_PACKET_TYPE_RESPONSE ||
      crcFast(frame.data() + 1, static_cast<int>(frame.size() - 1))) {
    return false;
  }
  command = binaryRead16(frame.data() + 1);
  error = binaryRead16(frame.data() + 3);
  return true;
}

int
makeBinaryCommand(uint16_t command, std::vector<uint8_t> &frame)
{
  frame.resize(5);
  return vscp_writeCommandToFrame(frame.data(), frame.size(), command, nullptr, 0);
}

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
  m_bBinaryMode               = false;
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

  OPENSSL_cleanse(m_binaryKey, sizeof(m_binaryKey));
  OPENSSL_cleanse(m_binaryHmacKey, sizeof(m_binaryHmacKey));

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
    vscp_deleteEvent_v2(&pev);
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
  if (usesManagedReceive()) {
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
  j["binary-mode"]        = m_bBinaryMode;
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

  // Authenticated binary encryption (keys are part of the configuration,
  // like the password)
  j["binary-encryption"] = m_bBinaryAuth ? m_binaryEncryption : 0;
  if (m_bBinaryAuth) {
    char hex[65];
    vscp_byteArray2HexStr(hex, m_binaryKey, m_binaryKeyLength);
    j["binary-key"] = hex;
    vscp_byteArray2HexStr(hex, m_binaryHmacKey, sizeof(m_binaryHmacKey));
    j["binary-hmac-key"] = hex;
    OPENSSL_cleanse(hex, sizeof(hex));
  }
  else {
    j["binary-key"]      = "";
    j["binary-hmac-key"] = "";
  }

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
    const bool binary = j.value("binary-mode", m_bBinaryMode);
    if ((polling && singleChannel) ||
        (usesManagedReceive() &&
         (m_singleChannelRunning || m_singleChannelWorkerThread.joinable() ||
          m_binarySendWorkerThread.joinable())) ||
        ((singleChannel != m_bSingleChannelReceiveLoop || binary != m_bBinaryMode) &&
         (m_singleChannelWorkerThread.joinable() || m_binarySendWorkerThread.joinable() ||
          nullptr != m_pworkerthread ||
          m_tcp.isConnected() || m_tcpReceive.isConnected()))) {
      tcpClientFailure(__func__, VSCP_ERROR_PARAMETER);
      return false;
    }

    // Authenticated binary encryption. Parsed and checked before anything
    // is changed so an invalid configuration leaves the client as it was.
    bool binaryAuth          = m_bBinaryAuth;
    uint8_t binaryEncryption = m_binaryEncryption;
    uint8_t binaryKey[32];
    uint8_t binaryKeyLength = m_binaryKeyLength;
    uint8_t binaryHmacKey[32];
    memcpy(binaryKey, m_binaryKey, sizeof(binaryKey));
    memcpy(binaryHmacKey, m_binaryHmacKey, sizeof(binaryHmacKey));
    if (j.contains("binary-encryption") || j.contains("binary-key") || j.contains("binary-hmac-key")) {
      binaryEncryption        = j.value("binary-encryption", binaryEncryption);
      std::string strKey      = j.value("binary-key", std::string());
      std::string strHmacKey  = j.value("binary-hmac-key", std::string());
      vscp_trim(strKey);
      vscp_trim(strHmacKey);

      if (VSCP_ENCRYPTION_NONE == binaryEncryption) {
        binaryAuth = false;
      }
      else {
        size_t expected = 0;
        switch (binaryEncryption) {
          case VSCP_ENCRYPTION_AES128:
            expected = 16;
            break;
          case VSCP_ENCRYPTION_AES192:
            expected = 24;
            break;
          case VSCP_ENCRYPTION_AES256:
            expected = 32;
            break;
          default:
            break;
        }
        // Encryption over TCP is always authenticated: AES key and HMAC key
        // are both required, and only in binary mode
        if (!expected || !binary || (strKey.length() != 2 * expected) || (64 != strHmacKey.length()) ||
            (expected != vscp_hexStr2ByteArray(binaryKey, expected, strKey.c_str())) ||
            (32 != vscp_hexStr2ByteArray(binaryHmacKey, 32, strHmacKey.c_str())) ||
            (0 == CRYPTO_memcmp(binaryKey, binaryHmacKey, expected))) {
          OPENSSL_cleanse(binaryKey, sizeof(binaryKey));
          OPENSSL_cleanse(binaryHmacKey, sizeof(binaryHmacKey));
          tcpClientFailure(__func__, VSCP_ERROR_PARAMETER);
          return false;
        }
        binaryKeyLength = static_cast<uint8_t>(expected);
        binaryAuth      = true;
      }
    }
    else if (!binary) {
      binaryAuth = false; // Only possible in binary mode
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
    m_bBinaryMode = binary;

    m_bBinaryAuth      = binaryAuth;
    m_binaryEncryption = binaryAuth ? binaryEncryption : 0;
    m_binaryKeyLength  = binaryAuth ? binaryKeyLength : 0;
    memcpy(m_binaryKey, binaryKey, sizeof(m_binaryKey));
    memcpy(m_binaryHmacKey, binaryHmacKey, sizeof(m_binaryHmacKey));
    OPENSSL_cleanse(binaryKey, sizeof(binaryKey));
    OPENSSL_cleanse(binaryHmacKey, sizeof(binaryHmacKey));
    if (!binaryAuth) {
      OPENSSL_cleanse(m_binaryKey, sizeof(m_binaryKey));
      OPENSSL_cleanse(m_binaryHmacKey, sizeof(m_binaryHmacKey));
    }
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
  if ((m_bSingleChannelReceiveLoop && bPolling) ||
      (usesManagedReceive() &&
       (m_singleChannelRunning || m_singleChannelWorkerThread.joinable() ||
        m_binarySendWorkerThread.joinable()))) {
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
      m_binarySendWorkerThread.joinable() ||
      nullptr != m_pworkerthread || m_tcp.isConnected() || m_tcpReceive.isConnected()) {
    return tcpClientFailure(__func__, VSCP_ERROR_PARAMETER);
  }

  m_bSingleChannelReceiveLoop = enable;
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setBinaryMode
//

int
vscpClientTcp::setBinaryMode(bool enable)
{
  if (m_singleChannelRunning || m_singleChannelWorkerThread.joinable() ||
      m_binarySendWorkerThread.joinable() || nullptr != m_pworkerthread ||
      m_tcp.isConnected() || m_tcpReceive.isConnected()) {
    return tcpClientFailure(__func__, VSCP_ERROR_PARAMETER);
  }
  m_bBinaryMode = enable;
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setCallbackEv
//

int
vscpClientTcp::setCallbackEv(std::function<void(vscpEvent &ev, void *pobj)> callback, void *pData)
{
  if (usesManagedReceive() &&
      (m_singleChannelRunning || m_singleChannelWorkerThread.joinable() || m_binarySendWorkerThread.joinable())) {
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
  if (usesManagedReceive() &&
      (m_singleChannelRunning || m_singleChannelWorkerThread.joinable() || m_binarySendWorkerThread.joinable())) {
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

  if (usesManagedReceive() &&
      (m_singleChannelRunning || m_singleChannelWorkerThread.joinable() ||
       m_binarySendWorkerThread.joinable() || nullptr != m_pworkerthread ||
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

  // Authenticated binary encryption on both connections
  if (m_bBinaryMode && m_bBinaryAuth) {
    if ((VSCP_ERROR_SUCCESS !=
         (rv = m_tcp.setBinaryAuthentication(m_binaryEncryption, m_binaryKey, m_binaryKeyLength, m_binaryHmacKey))) ||
        (VSCP_ERROR_SUCCESS != (rv = m_tcpReceive.setBinaryAuthentication(m_binaryEncryption,
                                                                          m_binaryKey,
                                                                          m_binaryKeyLength,
                                                                          m_binaryHmacKey)))) {
      return tcpClientFailure(__func__, rv);
    }
  }
  else {
    m_tcp.clearBinaryAuthentication();
    m_tcpReceive.clearBinaryAuthentication();
  }

  if (VSCP_ERROR_SUCCESS != (rv = m_tcp.setSingleChannelReceiveLoop(!m_bBinaryMode && m_bSingleChannelReceiveLoop))) {
    return rv;
  }

  if (m_bBinaryMode) {
    return connectBinary();
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
// normalizeBinaryChannel
//

vscpClientTcp::binary_channel
vscpClientTcp::normalizeBinaryChannel(binary_channel channel) const
{
  return binaryUsesTwoChannels() ? channel : binary_channel::send;
}

///////////////////////////////////////////////////////////////////////////////
// binaryInterface
//

VscpRemoteTcpIf &
vscpClientTcp::binaryInterface(binary_channel channel)
{
  return normalizeBinaryChannel(channel) == binary_channel::receive ? m_tcpReceive : m_tcp;
}

///////////////////////////////////////////////////////////////////////////////
// isReceiveWorkerThread
//

bool
vscpClientTcp::isReceiveWorkerThread(void)
{
  std::lock_guard<std::mutex> lock(m_singleChannelMutex);
  const auto current = std::this_thread::get_id();
  return current == m_singleChannelWorkerId || current == m_binarySendWorkerId;
}

///////////////////////////////////////////////////////////////////////////////
// openBinaryReceiveChannel
//

int
vscpClientTcp::openBinaryReceiveChannel(VscpRemoteTcpIf &interface,
                                        std::deque<std::vector<uint8_t>> &buffered)
{
  std::vector<uint8_t> command;
  int result = makeBinaryCommand(binaryOpenCommand, command);
  if (VSCP_ERROR_SUCCESS != result ||
      VSCP_ERROR_SUCCESS != (result = interface.sendBinaryFrame(command))) {
    return tcpClientFailure(__func__, result);
  }

  const auto deadline = std::chrono::steady_clock::now() +
                        std::chrono::milliseconds(interface.getResponseTimeout());
  size_t bufferedBytes = 0;
  while (std::chrono::steady_clock::now() < deadline) {
    auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
      deadline - std::chrono::steady_clock::now()).count();
    std::vector<uint8_t> frame;
    result = interface.receiveBinaryFrame(frame, static_cast<uint32_t>((std::max<int64_t>)(0, remaining)));
    if (VSCP_ERROR_SUCCESS != result) {
      return tcpClientFailure(__func__, result);
    }

    if (frame.size() >= 3 && frame[0] == VSCP_BINARY_PACKET_TYPE_RESPONSE &&
        binaryRead16(frame.data() + 1) == binaryOpenCommand) {
      uint16_t commandCode, error;
      if (!binaryReply(frame, commandCode, error) || frame.size() != 7) {
        return tcpClientFailure(__func__, VSCP_ERROR_INVALID_FRAME);
      }
      return error == VSCP_ERROR_SUCCESS ? VSCP_ERROR_SUCCESS : tcpClientFailure(__func__, error);
    }

    bufferedBytes += frame.size();
    if (bufferedBytes > 1024 * 1024) {
      return tcpClientFailure(__func__, VSCP_ERROR_OVERRUN);
    }
    buffered.push_back(std::move(frame));
  }
  return tcpClientFailure(__func__, VSCP_ERROR_TIMEOUT);
}

///////////////////////////////////////////////////////////////////////////////
// connectBinary
//

int
vscpClientTcp::connectBinary(void)
{
  int result = m_tcp.doCmdOpen(m_strHostname, m_strUsername, m_strPassword);
  if (VSCP_ERROR_SUCCESS != result) {
    return result;
  }
  if (VSCP_ERROR_SUCCESS != (result = m_tcp.doCmdGetChannelID(&m_obid))) {
    m_tcp.doCmdClose();
    return result;
  }

  if (binaryUsesTwoChannels()) {
    m_tcpReceive.setConnectTimeout(m_tcp.getConnectTimeout());
    m_tcpReceive.setResponseTimeout(m_tcp.getResponseTimeout());
    if (VSCP_ERROR_SUCCESS != (result = m_tcpReceive.doCmdOpen(m_strHostname, m_strUsername, m_strPassword))) {
      m_tcp.doCmdClose();
      return result;
    }
  }

  result = m_tcp.doCmdBinary();
  if (VSCP_ERROR_SUCCESS == result && binaryUsesTwoChannels()) {
    result = m_tcpReceive.doCmdBinary();
  }
  std::deque<std::vector<uint8_t>> buffered;
  const binary_channel receiveChannel = normalizeBinaryChannel(binary_channel::receive);
  if (VSCP_ERROR_SUCCESS == result && !m_bPolling) {
    result = openBinaryReceiveChannel(binaryInterface(receiveChannel), buffered);
  }
  if (VSCP_ERROR_SUCCESS != result) {
    m_tcpReceive.doCmdClose();
    m_tcp.doCmdClose();
    return result;
  }

  {
    std::lock_guard<std::mutex> lock(m_singleChannelMutex);
    m_singleChannelEvents.clear();
    m_binaryFrames.clear();
    m_binaryRawPending.clear();
    m_binaryRawActive = false;
    m_binaryPollPending = false;
    m_binaryPollSawEvent = false;
    m_binaryControlPending = false;
    m_binaryControlComplete = false;
    m_binaryControlReply.clear();
    m_singleChannelError = VSCP_ERROR_SUCCESS;
    m_singleChannelRunning = true;
  }
  try {
    m_singleChannelWorkerThread = std::thread(&vscpClientTcp::binaryWorker, this,
                                               receiveChannel, std::move(buffered));
    if (binaryUsesTwoChannels()) {
      m_binarySendWorkerThread = std::thread(&vscpClientTcp::binaryWorker, this,
                                              binary_channel::send,
                                              std::deque<std::vector<uint8_t>>());
    }
  }
  catch (const std::system_error &) {
    failSingleChannelWorker(VSCP_ERROR_ERROR);
    if (m_singleChannelWorkerThread.joinable()) {
      m_singleChannelWorkerThread.join();
    }
    m_tcpReceive.doCmdClose();
    m_tcp.doCmdClose();
    return tcpClientFailure(__func__, VSCP_ERROR_ERROR);
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

  if (usesManagedReceive()) {
    {
      std::lock_guard<std::mutex> lock(m_singleChannelMutex);
      if (m_singleChannelWorkerId == std::this_thread::get_id() ||
          m_binarySendWorkerId == std::this_thread::get_id()) {
        return tcpClientFailure(__func__, VSCP_ERROR_NOT_SUPPORTED);
      }
      m_singleChannelRunning = false;
      m_singleChannelError = VSCP_ERROR_NOT_CONNECTED;
      m_singleChannelEvents.clear();
      m_binaryFrames.clear();
      m_binaryRawPending.clear();
    }
    m_singleChannelCondition.notify_all();
    m_binaryCondition.notify_all();
    {
      std::lock_guard<std::mutex> lock(m_singleChannelDispatchMutex);
      m_singleChannelDispatchCondition.notify_all();
    }
    if (m_singleChannelWorkerThread.joinable()) {
      m_singleChannelWorkerThread.join();
    }
    if (m_binarySendWorkerThread.joinable()) {
      m_binarySendWorkerThread.join();
    }
    {
      std::lock_guard<std::mutex> lock(m_singleChannelMutex);
      m_singleChannelWorkerId = std::thread::id();
      m_binarySendWorkerId = std::thread::id();
    }
    std::lock_guard<std::mutex> lock(m_singleChannelCommandMutex);
    rv = m_tcp.doCmdClose();
    if (m_bBinaryMode && binaryUsesTwoChannels()) {
      int receiveResult = m_tcpReceive.doCmdClose();
      if (VSCP_ERROR_SUCCESS == rv) {
        rv = receiveResult;
      }
    }
    return rv;
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
      delete m_pworkerthread;
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
  if (usesManagedReceive()) {
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
  if (usesManagedReceive()) {
    int result;
    {
      std::lock_guard<std::mutex> lock(m_singleChannelCommandMutex);
      if (!m_singleChannelRunning) {
        return tcpClientFailure(__func__, VSCP_ERROR_NOT_CONNECTED);
      }
      result = m_tcp.doCmdSend(&ev);
    }
    if (m_bBinaryMode && result != VSCP_ERROR_SUCCESS && !m_tcp.isConnected()) {
      failSingleChannelWorker(result);
    }
    return result;
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
  if (usesManagedReceive()) {
    int result;
    {
      std::lock_guard<std::mutex> lock(m_singleChannelCommandMutex);
      if (!m_singleChannelRunning) {
        return tcpClientFailure(__func__, VSCP_ERROR_NOT_CONNECTED);
      }
      result = m_tcp.doCmdSendEx(&ex);
    }
    if (m_bBinaryMode && result != VSCP_ERROR_SUCCESS && !m_tcp.isConnected()) {
      failSingleChannelWorker(result);
    }
    return result;
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
// sendBinaryFrame
//

int
vscpClientTcp::sendBinaryFrame(const std::vector<uint8_t> &frame, binary_channel channel)
{
  if (!m_bBinaryMode) {
    return tcpClientFailure(__func__, VSCP_ERROR_NOT_SUPPORTED);
  }
  if ((channel != binary_channel::send && channel != binary_channel::receive) ||
      frame.empty() || frame.size() > UINT16_MAX) {
    return tcpClientFailure(__func__, VSCP_ERROR_PARAMETER);
  }

  channel = normalizeBinaryChannel(channel);
  const bool isCommand = frame.size() >= 3 && frame[0] == VSCP_BINARY_PACKET_TYPE_COMMAND;
  const uint16_t code = isCommand ? binaryRead16(frame.data() + 1) : 0;
  const bool tracked = isCommand && (code == binaryClearCommand ||
                                     (m_bPolling && code == binaryRetrCommand));
  if (tracked && frame.size() < 5) {
    return tcpClientFailure(__func__, VSCP_ERROR_PARAMETER);
  }
  if (tracked && crcFast(frame.data() + 1, static_cast<int>(frame.size() - 1))) {
    return tcpClientFailure(__func__, VSCP_ERROR_INVALID_FRAME);
  }
  if (m_bPolling && isCommand && code == binaryRetrCommand &&
      (frame.size() != 5 && (frame.size() != 7 || binaryRead16(frame.data() + 3) > 1))) {
    return tcpClientFailure(__func__, VSCP_ERROR_NOT_SUPPORTED);
  }
  const auto key = std::make_pair(channel, code);
  int result;
  {
    std::lock_guard<std::mutex> commandLock(m_singleChannelCommandMutex);
    {
      std::lock_guard<std::mutex> lock(m_singleChannelMutex);
      if (!m_singleChannelRunning) {
        return tcpClientFailure(__func__, VSCP_ERROR_NOT_CONNECTED);
      }
      if (tracked && ((m_binaryPollPending && channel == binary_channel::send &&
                       code == binaryRetrCommand) ||
                      (m_binaryControlPending && channel == m_binaryControlChannel &&
                       code == m_binaryControlCode))) {
        return tcpClientFailure(__func__, VSCP_ERROR_ALREADY_DEFINED);
      }
      m_binaryRawActive = true;
      if (tracked) {
        m_binaryRawPending[key].push_back(std::chrono::steady_clock::now() +
                                         std::chrono::milliseconds(binaryInterface(channel).getResponseTimeout()));
      }
    }

    result = binaryInterface(channel).sendBinaryFrame(frame);
    if (VSCP_ERROR_SUCCESS != result && tracked) {
      std::lock_guard<std::mutex> lock(m_singleChannelMutex);
      auto pending = m_binaryRawPending.find(key);
      if (pending != m_binaryRawPending.end()) {
        pending->second.pop_back();
        if (pending->second.empty()) {
          m_binaryRawPending.erase(pending);
        }
      }
    }
  }
  if (VSCP_ERROR_SUCCESS != result && !binaryInterface(channel).isConnected()) {
    failSingleChannelWorker(result);
  }
  return result;
}

///////////////////////////////////////////////////////////////////////////////
// receiveBinaryFrame
//

int
vscpClientTcp::receiveBinaryFrame(std::vector<uint8_t> &frame, long timeout, binary_channel *channel)
{
  if (!m_bBinaryMode) {
    return tcpClientFailure(__func__, VSCP_ERROR_NOT_SUPPORTED);
  }
  if (timeout < 0) {
    return tcpClientFailure(__func__, VSCP_ERROR_PARAMETER);
  }

  std::unique_lock<std::mutex> lock(m_singleChannelMutex);
  if (!m_singleChannelRunning && m_binaryFrames.empty()) {
    return m_singleChannelError == VSCP_ERROR_SUCCESS ? VSCP_ERROR_NOT_CONNECTED : m_singleChannelError;
  }
  m_binaryRawActive = true;
  auto ready = [this]() { return !m_binaryFrames.empty() || !m_singleChannelRunning; };
  if (!ready()) {
    if (!timeout) {
      return VSCP_ERROR_RCV_EMPTY;
    }
    if (!m_binaryCondition.wait_for(lock, std::chrono::milliseconds(timeout), ready)) {
      return VSCP_ERROR_TIMEOUT;
    }
  }
  if (m_binaryFrames.empty()) {
    return m_singleChannelError == VSCP_ERROR_SUCCESS ? VSCP_ERROR_NOT_CONNECTED : m_singleChannelError;
  }
  raw_binary_frame received = std::move(m_binaryFrames.front());
  m_binaryFrames.pop_front();
  frame = std::move(received.data);
  if (nullptr != channel) {
    *channel = received.channel;
  }
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// binaryCommand
//

int
vscpClientTcp::binaryCommand(binary_channel channel, uint16_t command, std::vector<uint8_t> &reply)
{
  if (isReceiveWorkerThread()) {
    return tcpClientFailure(__func__, VSCP_ERROR_NOT_SUPPORTED);
  }
  std::lock_guard<std::mutex> controlLock(m_binaryControlMutex);
  channel = normalizeBinaryChannel(channel);
  const auto key = std::make_pair(channel, command);
  std::vector<uint8_t> request;
  int result = makeBinaryCommand(command, request);
  if (VSCP_ERROR_SUCCESS != result) {
    return tcpClientFailure(__func__, result);
  }

  {
    std::lock_guard<std::mutex> commandLock(m_singleChannelCommandMutex);
    {
      std::lock_guard<std::mutex> lock(m_singleChannelMutex);
      if (!m_singleChannelRunning) {
        return tcpClientFailure(__func__, VSCP_ERROR_NOT_CONNECTED);
      }
      if (m_binaryRawPending.find(key) != m_binaryRawPending.end() ||
          (m_binaryPollPending && channel == binary_channel::send && command == binaryRetrCommand)) {
        return tcpClientFailure(__func__, VSCP_ERROR_ALREADY_DEFINED);
      }
      m_binaryControlPending = true;
      m_binaryControlComplete = false;
      m_binaryControlChannel = channel;
      m_binaryControlCode = command;
      m_binaryControlReply.clear();
    }
    result = binaryInterface(channel).sendBinaryFrame(request);
  }
  if (VSCP_ERROR_SUCCESS != result) {
    {
      std::lock_guard<std::mutex> lock(m_singleChannelMutex);
      m_binaryControlPending = false;
    }
    if (!binaryInterface(channel).isConnected()) {
      failSingleChannelWorker(result);
    }
    return result;
  }

  std::unique_lock<std::mutex> lock(m_singleChannelMutex);
  const bool completed = m_binaryCondition.wait_for(
    lock, std::chrono::milliseconds(binaryInterface(channel).getResponseTimeout()),
    [this]() { return m_binaryControlComplete || !m_singleChannelRunning; });
  if (!completed) {
    m_binaryControlPending = false;
    lock.unlock();
    failSingleChannelWorker(VSCP_ERROR_TIMEOUT);
    return tcpClientFailure(__func__, VSCP_ERROR_TIMEOUT);
  }
  if (!m_binaryControlComplete) {
    m_binaryControlPending = false;
    return m_singleChannelError == VSCP_ERROR_SUCCESS ? VSCP_ERROR_NOT_CONNECTED : m_singleChannelError;
  }
  reply = std::move(m_binaryControlReply);
  m_binaryControlPending = false;
  uint16_t replyCommand, error;
  if (!binaryReply(reply, replyCommand, error) || replyCommand != command || reply.size() != 7) {
    lock.unlock();
    failSingleChannelWorker(VSCP_ERROR_INVALID_FRAME);
    return tcpClientFailure(__func__, VSCP_ERROR_INVALID_FRAME);
  }
  return error == VSCP_ERROR_SUCCESS ? VSCP_ERROR_SUCCESS : tcpClientFailure(__func__, error);
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
// popReceiveList
//

int
vscpClientTcp::popReceiveList(vscpEvent &ev, bool bConsumeSignal)
{
  pthread_mutex_lock(&m_mutexReceiveQueue);
  if (m_receiveList.empty()) {
    pthread_mutex_unlock(&m_mutexReceiveQueue);
    return VSCP_ERROR_RCV_EMPTY;
  }
  vscpEvent *pev = m_receiveList.front();
  m_receiveList.pop_front();
  if (bConsumeSignal) {
    // Keep the semaphore count in step with the queue length
#ifdef WIN32
    WaitForSingleObject(m_semReceiveQueue, 0);
#else
    sem_trywait(&m_semReceiveQueue);
#endif
  }
  pthread_mutex_unlock(&m_mutexReceiveQueue);

  if (nullptr == pev) {
    return VSCP_ERROR_MEMORY;
  }

  // Hand over the data buffer to the caller
  ev       = *pev;
  pev->pdata    = nullptr;
  pev->sizeData = 0;
  vscp_deleteEvent_v2(&pev);
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// clearReceiveList
//

void
vscpClientTcp::clearReceiveList(void)
{
  pthread_mutex_lock(&m_mutexReceiveQueue);
  while (!m_receiveList.empty()) {
    vscpEvent *pev = m_receiveList.front();
    m_receiveList.pop_front();
    vscp_deleteEvent_v2(&pev);
#ifdef WIN32
    WaitForSingleObject(m_semReceiveQueue, 0);
#else
    sem_trywait(&m_semReceiveQueue);
#endif
  }
  pthread_mutex_unlock(&m_mutexReceiveQueue);
}

///////////////////////////////////////////////////////////////////////////////
// pollServerReceive
//

int
vscpClientTcp::pollServerReceive(vscpEvent &ev)
{
  return m_tcp.doCmdReceive(&ev);
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int
vscpClientTcp::receive(vscpEvent &ev)
{
  if (usesManagedReceive()) {
    vscpEventEx event = {};
    int rv = receiveSingleChannel(event, 0, false);
    if (VSCP_ERROR_SUCCESS != rv) {
      return rv;
    }
    return vscp_convertEventExToEvent(&ev, &event) ? VSCP_ERROR_SUCCESS :
      tcpClientFailure(__func__, VSCP_ERROR_INVALID_FRAME);
  }

  // The worker thread owns the receive connection and queues events
  if (nullptr != m_pworkerthread) {
    return popReceiveList(ev, true);
  }

  return m_tcp.doCmdReceive(&ev);
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int
vscpClientTcp::receive(vscpEventEx &ex)
{
  if (usesManagedReceive()) {
    return receiveSingleChannel(ex, 0, false);
  }

  // The worker thread owns the receive connection and queues events
  if (nullptr != m_pworkerthread) {
    vscpEvent ev = {};
    int rv = popReceiveList(ev, true);
    if (VSCP_ERROR_SUCCESS == rv && !vscp_convertEventToEventEx(&ex, &ev)) {
      rv = VSCP_ERROR_INVALID_FRAME;
    }
    vscp_deleteEvent(&ev);
    return rv;
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
  if (usesManagedReceive()) {
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

  if (nullptr != m_pworkerthread) {
    return popReceiveList(ev, false);
  }

  return receive(ev);
}

///////////////////////////////////////////////////////////////////////////////
// receiveBlocking
//

int
vscpClientTcp::receiveBlocking(vscpEventEx &ex, long timeout)
{
  if (usesManagedReceive()) {
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

  if (nullptr != m_pworkerthread) {
    vscpEvent ev = {};
    int rv = popReceiveList(ev, false);
    if (VSCP_ERROR_SUCCESS == rv && !vscp_convertEventToEventEx(&ex, &ev)) {
      rv = VSCP_ERROR_INVALID_FRAME;
    }
    vscp_deleteEvent(&ev);
    return rv;
  }

  return receive(ex);
}

///////////////////////////////////////////////////////////////////////////////
// receiveBlocking
//

int
vscpClientTcp::receiveBlocking(canalMsg &msg, long timeout)
{
  if (usesManagedReceive()) {
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

  if (nullptr != m_pworkerthread) {
    vscpEvent ev = {};
    int rv = popReceiveList(ev, false);
    if (VSCP_ERROR_SUCCESS == rv && !vscp_convertEventToCanal(&msg, &ev)) {
      rv = VSCP_ERROR_INVALID_FRAME;
    }
    vscp_deleteEvent(&ev);
    return rv;
  }

  return receive(msg);
}

///////////////////////////////////////////////////////////////////////////////
// setfilter
//

int
vscpClientTcp::setfilter(vscpEventFilter &filter)
{
  if (usesManagedReceive()) {
    std::lock_guard<std::mutex> lock(m_singleChannelMutex);
    m_filterIn = filter;
    return VSCP_ERROR_SUCCESS;
  }

  if (m_bPolling) {
    return m_tcp.doCmdFilter(&filter);
  }
  else {
    pthread_mutex_lock(&m_mutexTcpIpObject);
    int rv = m_tcpReceive.doCmdFilter(&filter);
    pthread_mutex_unlock(&m_mutexTcpIpObject);
    return rv;
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
  if (usesManagedReceive()) {
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

  if (nullptr != m_pworkerthread) {
    pthread_mutex_lock(&m_mutexReceiveQueue);
    size_t cnt = m_receiveList.size();
    pthread_mutex_unlock(&m_mutexReceiveQueue);
    *pcount = (uint16_t) ((std::min)(cnt, (size_t) (std::numeric_limits<uint16_t>::max)()));
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
  if (m_bBinaryMode) {
    std::vector<uint8_t> reply;
    return binaryCommand(binary_channel::receive, binaryClearCommand, reply);
  }

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

  clearReceiveList();
  return m_tcp.doCmdClear();
}

///////////////////////////////////////////////////////////////////////////////
// getversion
//

int
vscpClientTcp::getversion(uint8_t *pmajor, uint8_t *pminor, uint8_t *prelease, uint8_t *pbuild)
{
  if (usesManagedReceive()) {
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
  if (usesManagedReceive()) {
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
  if (usesManagedReceive()) {
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
  m_binaryCondition.notify_all();
}

///////////////////////////////////////////////////////////////////////////////
// dispatchBinaryEvent
//

int
vscpClientTcp::dispatchBinaryEvent(binary_channel channel, const std::vector<uint8_t> &frame, bool acknowledge)
{
  vscpEvent event = {};
  std::unique_ptr<vscpEvent, decltype(&vscp_deleteEvent)> cleanup(&event, vscp_deleteEvent);
  int result = binaryInterface(channel).processBinaryEventFrame(frame, &event, acknowledge);
  if (VSCP_ERROR_SUCCESS != result) {
    return result;
  }

  bool callbacks = false;
  bool queued = false;
  {
    std::lock_guard<std::mutex> lock(m_singleChannelMutex);
    if (m_singleChannelRunning && vscp_doLevel2Filter(&event, &m_filterIn)) {
      callbacks = isCallbackEvActive() || isCallbackExActive();
      if (!callbacks) {
        vscpEventEx converted = {};
        if (!vscp_convertEventToEventEx(&converted, &event)) {
          return tcpClientFailure(__func__, VSCP_ERROR_INVALID_FRAME);
        }
        m_singleChannelEvents.push_back(converted);
        queued = true;
      }
    }
  }
  if (queued) {
    m_singleChannelCondition.notify_one();
  }
  if (callbacks) {
    std::lock_guard<std::mutex> callbackLock(m_binaryCallbackMutex);
    if (m_singleChannelRunning) {
      sendToCallbacks(&event);
    }
  }
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// processBinaryFrame
//

int
vscpClientTcp::processBinaryFrame(binary_channel channel, const std::vector<uint8_t> &frame)
{
  if (frame.empty()) {
    return tcpClientFailure(__func__, VSCP_ERROR_INVALID_FRAME);
  }
  if (!(frame[0] & 0x0f) && (frame[0] >> 4) <= 1) {
    if (m_bPolling) {
      std::lock_guard<std::mutex> lock(m_singleChannelMutex);
      if (m_binaryPollPending) {
        m_binaryPollSawEvent = true;
      }
    }
    return dispatchBinaryEvent(channel, frame, true);
  }

  if (frame.size() >= 3 && frame[0] == VSCP_BINARY_PACKET_TYPE_RESPONSE) {
    const uint16_t code = binaryRead16(frame.data() + 1);
    uint16_t replyCommand = 0, error = 0;
    const bool valid = binaryReply(frame, replyCommand, error);
    bool pollReply = false;
    bool pollSawEvent = false;
    bool controlReply = false;
    bool consumeConfirmation = false;
    {
      std::lock_guard<std::mutex> lock(m_singleChannelMutex);
      if (m_binaryPollPending && channel == binary_channel::send && code == binaryRetrCommand) {
        m_binaryPollPending = false;
        pollSawEvent = m_binaryPollSawEvent;
        pollReply = true;
      }
      else if (m_binaryControlPending && !m_binaryControlComplete &&
               channel == m_binaryControlChannel && code == m_binaryControlCode) {
        controlReply = true;
        if (valid && frame.size() == 7) {
          if (code == binaryClearCommand && error == VSCP_ERROR_SUCCESS) {
            m_singleChannelEvents.clear();
          }
          m_binaryControlReply = frame;
          m_binaryControlComplete = true;
        }
      }
      else {
        auto pending = m_binaryRawPending.find(std::make_pair(channel, code));
        if (valid && pending != m_binaryRawPending.end()) {
          pending->second.pop_front();
          if (pending->second.empty()) {
            m_binaryRawPending.erase(pending);
          }
        }
        else if (code == binaryEventConfirmation && !m_binaryRawActive) {
          consumeConfirmation = true;
        }
      }
    }
    if (controlReply) {
      if (!valid || frame.size() != 7) {
        return tcpClientFailure(__func__, VSCP_ERROR_INVALID_FRAME);
      }
      m_binaryCondition.notify_all();
      return VSCP_ERROR_SUCCESS;
    }
    if (pollReply) {
      if (!valid) {
        return tcpClientFailure(__func__, VSCP_ERROR_INVALID_FRAME);
      }
      if (error == VSCP_ERROR_RCV_EMPTY && frame.size() == 7) {
        return VSCP_ERROR_SUCCESS;
      }
      if (error != VSCP_ERROR_SUCCESS) {
        return tcpClientFailure(__func__, error);
      }
      if (frame.size() == 7 && pollSawEvent) {
        return VSCP_ERROR_SUCCESS;
      }
      if (frame.size() < 42) {
        return tcpClientFailure(__func__, VSCP_ERROR_INVALID_FRAME);
      }
      std::vector<uint8_t> eventFrame;
      eventFrame.reserve(frame.size() - 4);
      eventFrame.push_back(0x10);
      eventFrame.insert(eventFrame.end(), frame.begin() + 5, frame.end() - 2);
      const crc checksum = crcFast(eventFrame.data() + 1, static_cast<int>(eventFrame.size() - 1));
      eventFrame.push_back(static_cast<uint8_t>(checksum >> 8));
      eventFrame.push_back(static_cast<uint8_t>(checksum));
      return dispatchBinaryEvent(channel, eventFrame, false);
    }
    if (consumeConfirmation) {
      if (!valid || frame.size() != 13) {
        return tcpClientFailure(__func__, VSCP_ERROR_INVALID_FRAME);
      }
      return error == VSCP_ERROR_SUCCESS ? VSCP_ERROR_SUCCESS : tcpClientFailure(__func__, error);
    }
  }

  {
    std::lock_guard<std::mutex> lock(m_singleChannelMutex);
    m_binaryFrames.push_back({channel, frame});
  }
  m_binaryCondition.notify_one();
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// binaryWorker
//

void
vscpClientTcp::binaryWorker(binary_channel channel, std::deque<std::vector<uint8_t>> buffered)
{
  const bool sendWorker = binaryUsesTwoChannels() && channel == binary_channel::send;
  {
    std::lock_guard<std::mutex> lock(m_singleChannelMutex);
    (sendWorker ? m_binarySendWorkerId : m_singleChannelWorkerId) = std::this_thread::get_id();
  }

  try {
    VscpRemoteTcpIf &interface = binaryInterface(channel);
    auto nextPoll = std::chrono::steady_clock::now();
    while (m_singleChannelRunning) {
      const auto now = std::chrono::steady_clock::now();
      bool issuePoll = false;
      bool timedOut = false;
      {
        std::lock_guard<std::mutex> lock(m_singleChannelMutex);
        for (const auto &pending : m_binaryRawPending) {
          if (pending.first.first == channel && !pending.second.empty() && now >= pending.second.front()) {
            timedOut = true;
            break;
          }
        }
        if (m_bPolling) {
          timedOut = timedOut || (m_binaryPollPending && now >= m_binaryPollDeadline);
          if (!timedOut && !m_binaryPollPending && now >= nextPoll &&
              m_binaryRawPending.find(std::make_pair(channel, binaryRetrCommand)) == m_binaryRawPending.end()) {
            m_binaryPollPending = true;
            m_binaryPollSawEvent = false;
            m_binaryPollDeadline = now + std::chrono::milliseconds(interface.getResponseTimeout());
            issuePoll = true;
            nextPoll = now + std::chrono::milliseconds(50);
          }
        }
      }
      if (timedOut) {
        failSingleChannelWorker(VSCP_ERROR_TIMEOUT);
        break;
      }
      if (issuePoll) {
        std::vector<uint8_t> request;
        int result = makeBinaryCommand(binaryRetrCommand, request);
        if (VSCP_ERROR_SUCCESS == result) {
          result = interface.sendBinaryFrame(request);
        }
        if (VSCP_ERROR_SUCCESS != result) {
          failSingleChannelWorker(result);
          break;
        }
      }

      std::vector<uint8_t> frame;
      int result;
      if (!buffered.empty()) {
        frame = std::move(buffered.front());
        buffered.pop_front();
        result = VSCP_ERROR_SUCCESS;
      }
      else {
        result = interface.receiveBinaryFrame(frame, 50);
      }
      if (!m_singleChannelRunning) {
        break;
      }
      if (result == VSCP_ERROR_TIMEOUT) {
        continue;
      }
      if (result == VSCP_ERROR_SUCCESS) {
        result = processBinaryFrame(channel, frame);
      }
      if (result != VSCP_ERROR_SUCCESS) {
        failSingleChannelWorker(result);
        break;
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
  (sendWorker ? m_binarySendWorkerId : m_singleChannelWorkerId) = std::thread::id();
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

        // Add to input queue only if no callback set.
        // Queue a heap copy - ev is reused for the next event.
        if (!pClient->isCallbackEvActive() && !pClient->isCallbackExActive()) {
          vscpEvent *pnew = new (std::nothrow) vscpEvent;
          if (nullptr != pnew) {
            pnew->sizeData = 0;
            pnew->pdata    = nullptr;
            if (vscp_copyEvent(pnew, &ev)) {
              pthread_mutex_lock(&pClient->m_mutexReceiveQueue);
              if (pClient->m_receiveList.size() < VSCP_TCP_CLIENT_MAX_RECEIVE_QUEUE) {
                pClient->m_receiveList.push_back(pnew);
                pnew = nullptr;
#ifdef WIN32
                ReleaseSemaphore(pClient->m_semReceiveQueue, 1, NULL);
#else
                sem_post(&pClient->m_semReceiveQueue);
#endif
              }
              pthread_mutex_unlock(&pClient->m_mutexReceiveQueue);
              if (nullptr != pnew) {
                spdlog::warn("VSCP TCP CLIENT: Receive queue full ({0} events). Event dropped: class={1} type={2}",
                             VSCP_TCP_CLIENT_MAX_RECEIVE_QUEUE,
                             pnew->vscp_class,
                             pnew->vscp_type);
              }
            }
            else {
              spdlog::error("VSCP TCP CLIENT: Failed to copy received event. Event dropped: class={0} type={1}",
                            ev.vscp_class,
                            ev.vscp_type);
            }
            vscp_deleteEvent_v2(&pnew); // Not queued
          }
          else {
            spdlog::error("VSCP TCP CLIENT: Out of memory. Event dropped: class={0} type={1}",
                          ev.vscp_class,
                          ev.vscp_type);
          }
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
    int rv = pClient->pollServerReceive(ev);
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
          if (pClient->m_receiveList.size() < VSCP_TCP_CLIENT_MAX_RECEIVE_QUEUE) {
            pClient->m_receiveList.push_back(pnew);
            pnew = nullptr;
#ifdef WIN32
            ReleaseSemaphore(pClient->m_semReceiveQueue, 1, NULL);
#else
            sem_post(&pClient->m_semReceiveQueue);
#endif
          }
          pthread_mutex_unlock(&pClient->m_mutexReceiveQueue);
          if (nullptr != pnew) {
            spdlog::warn("VSCP TCP CLIENT: Receive queue full ({0} events). Event dropped: class={1} type={2}",
                         VSCP_TCP_CLIENT_MAX_RECEIVE_QUEUE,
                         pnew->vscp_class,
                         pnew->vscp_type);
            vscp_deleteEvent_v2(&pnew); // Not queued (queue full)
          }
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