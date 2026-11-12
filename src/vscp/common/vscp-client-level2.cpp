// vscp-client-level2.cpp
//
// VSCP Level II driver client communication class.
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
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <dlfcn.h>

#include <algorithm>
#include <chrono>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <random>
#include <vector>

#include "canal.h"
#include "vscp-client-level2.h"
#include "vscphelper.h"

#include <nlohmann/json.hpp>

#include <spdlog/spdlog.h>

using json = nlohmann::json;

namespace {

/*
  Guess a file suffix for an embedded configuration from its content.
  Drivers usually only care about the content, but some look at the
  suffix to select a parser.
*/
std::string
guessSuffix(const std::string &config)
{
  for (char ch : config) {
    if (std::isspace(static_cast<unsigned char>(ch))) {
      continue;
    }
    if (('{' == ch) || ('[' == ch)) {
      return ".json";
    }
    if ('<' == ch) {
      return ".xml";
    }
    break;
  }
  return ".conf";
}

// Only allow a short, plain suffix such as ".json"
bool
validSuffix(const std::string &suffix)
{
  if (suffix.empty()) {
    return true;
  }
  if ((suffix.size() > 16) || ('.' != suffix[0])) {
    return false;
  }
  for (size_t i = 1; i < suffix.size(); i++) {
    if (!std::isalnum(static_cast<unsigned char>(suffix[i]))) {
      return false;
    }
  }
  return true;
}

// Directory for temporary files
std::string
tempDirectory(void)
{
#ifdef WIN32
  char buf[MAX_PATH + 1];
  DWORD len = GetTempPathA(sizeof(buf), buf);
  if ((0 == len) || (len > MAX_PATH)) {
    return std::string();
  }
  std::string dir(buf, len);
#else
  const char *env = getenv("TMPDIR");
  std::string dir = ((nullptr != env) && *env) ? env : "/tmp";
#endif
  while ((dir.size() > 1) && (('/' == dir.back()) || ('\\' == dir.back()))) {
    dir.pop_back();
  }
  return dir;
}

// Log an exception thrown out of a driver. A driver should never let an
// exception escape its C interface, but if one does it must not take the
// application down.
void
logDriverException(const char *call, const std::exception *pex)
{
  spdlog::error("LEVEL2 CLIENT: Exception from driver in {}: {}", call, (nullptr != pex) ? pex->what() : "unknown");
}

// Write all of a buffer to a FILE
bool
writeAll(FILE *fp, const std::string &data)
{
  return (data.size() == fwrite(data.data(), 1, data.size(), fp)) && (0 == fflush(fp));
}

} // namespace

///////////////////////////////////////////////////////////////////////////////
// C-tor
//

vscpClientLevel2::vscpClientLevel2()
{
  m_type              = CVscpClient::connType::LEVEL2;
  m_responseTimeout   = DEFAULT_RESPONSE_TIMEOUT;
  m_connectionTimeout = 0;
  memset(m_guid, 0, sizeof(m_guid));

  m_hdll                = nullptr;
  m_openHandle          = 0;
  m_procOpen            = nullptr;
  m_procClose           = nullptr;
  m_procWrite           = nullptr;
  m_procRead            = nullptr;
  m_procGetVersion      = nullptr;
  m_procGetVendorString = nullptr;

  m_bConnected    = false;
  m_bRun          = false;
  m_droppedEvents = 0;

  vscp_clearVSCPFilter(&m_filterIn); // Accept all events
}

///////////////////////////////////////////////////////////////////////////////
// D-tor
//

vscpClientLevel2::~vscpClientLevel2()
{
  disconnect();
  clear();
}

///////////////////////////////////////////////////////////////////////////////
// init
//

int
vscpClientLevel2::init(const std::string &pathDriver, const std::string &pathConfig, const std::string &guid)
{
  std::lock_guard<std::mutex> lock(m_lifecycleMutex);
  if (m_bConnected) {
    return VSCP_ERROR_PARAMETER;
  }

  uint8_t newGuid[16] = { 0 };
  if (!guid.empty() && !vscp_parseGuid(newGuid, guid)) {
    return VSCP_ERROR_PARAMETER;
  }

  m_pathDriver = pathDriver;
  m_pathConfig = pathConfig;
  m_embeddedConfig.clear();
  m_configSuffix.clear();
  memcpy(m_guid, newGuid, sizeof(m_guid));
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// initEmbedded
//

int
vscpClientLevel2::initEmbedded(const std::string &pathDriver,
                               const std::string &config,
                               const std::string &guid,
                               const std::string &suffix)
{
  std::lock_guard<std::mutex> lock(m_lifecycleMutex);
  if (m_bConnected || config.empty() || !validSuffix(suffix)) {
    return VSCP_ERROR_PARAMETER;
  }

  uint8_t newGuid[16] = { 0 };
  if (!guid.empty() && !vscp_parseGuid(newGuid, guid)) {
    return VSCP_ERROR_PARAMETER;
  }

  m_pathDriver = pathDriver;
  m_pathConfig.clear();
  m_embeddedConfig = config;
  m_configSuffix   = suffix;
  memcpy(m_guid, newGuid, sizeof(m_guid));
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getConfigAsJson
//

std::string
vscpClientLevel2::getConfigAsJson(void)
{
  std::lock_guard<std::mutex> lock(m_lifecycleMutex);
  json j;
  std::string str;

  j["type"]        = static_cast<int>(m_type);
  j["name"]        = getName();
  j["path-driver"] = m_pathDriver;

  if (!m_embeddedConfig.empty()) {
    // Keep a JSON configuration as JSON so it stays readable
    json embedded = json::parse(m_embeddedConfig, nullptr, false);
    if (!embedded.is_discarded() && (embedded.is_object() || embedded.is_array())) {
      j["config"] = embedded;
    }
    else {
      j["config"] = m_embeddedConfig;
    }
    if (!m_configSuffix.empty()) {
      j["config-suffix"] = m_configSuffix;
    }
  }
  else {
    j["path-config"] = m_pathConfig;
  }

  vscp_writeGuidArrayToString(str, m_guid);
  j["guid"] = str;

  j["response-timeout"]   = m_responseTimeout;
  j["connection-timeout"] = m_connectionTimeout;

  {
    std::lock_guard<std::mutex> qlock(m_mutexReceiveQueue);
    j["priority-filter"] = m_filterIn.filter_priority;
    j["priority-mask"]   = m_filterIn.mask_priority;
    j["class-filter"]    = m_filterIn.filter_class;
    j["class-mask"]      = m_filterIn.mask_class;
    j["type-filter"]     = m_filterIn.filter_type;
    j["type-mask"]       = m_filterIn.mask_type;
    vscp_writeGuidArrayToString(str, m_filterIn.filter_GUID);
    j["guid-filter"] = str;
    vscp_writeGuidArrayToString(str, m_filterIn.mask_GUID);
    j["guid-mask"] = str;
  }

  return j.dump();
}

///////////////////////////////////////////////////////////////////////////////
// initFromJson
//

bool
vscpClientLevel2::initFromJson(const std::string &config)
{
  std::lock_guard<std::mutex> lock(m_lifecycleMutex);

  json j = json::parse(config, nullptr, false);
  if (j.is_discarded() || !j.is_object()) {
    spdlog::error("LEVEL2 CLIENT: Invalid JSON configuration");
    return false;
  }

  if (m_bConnected) {
    spdlog::error("LEVEL2 CLIENT: Configuration can not be changed while connected");
    return false;
  }

  // Parse everything into locals first so a failure changes nothing
  std::string name           = getName();
  std::string pathDriver     = m_pathDriver;
  std::string pathConfig     = m_pathConfig;
  std::string embeddedConfig = m_embeddedConfig;
  std::string configSuffix   = m_configSuffix;
  uint32_t responseTimeout   = m_responseTimeout;
  uint32_t connectionTimeout = m_connectionTimeout;
  uint8_t guid[16];
  memcpy(guid, m_guid, sizeof(guid));
  vscpEventFilter filter;
  {
    std::lock_guard<std::mutex> qlock(m_mutexReceiveQueue);
    filter = m_filterIn;
  }

  try {
    if (j.contains("name")) {
      if (!j["name"].is_string()) {
        spdlog::error("LEVEL2 CLIENT: 'name' must be a string");
        return false;
      }
      name = j["name"].get<std::string>();
    }

    if (j.contains("path-driver")) {
      if (!j["path-driver"].is_string()) {
        spdlog::error("LEVEL2 CLIENT: 'path-driver' must be a string");
        return false;
      }
      pathDriver = j["path-driver"].get<std::string>();
    }

    bool bConfigSet = false;
    if (j.contains("config") && !j["config"].is_null()) {
      const json &jc = j["config"];
      std::string text;
      if (jc.is_object() || jc.is_array()) {
        text = jc.dump(2);
      }
      else if (jc.is_string()) {
        text = jc.get<std::string>();
      }
      else {
        spdlog::error("LEVEL2 CLIENT: 'config' must be an object, an array or a string");
        return false;
      }
      if (!text.empty() && !(jc.is_object() && jc.empty())) {
        embeddedConfig = text;
        pathConfig.clear();
        bConfigSet = true;
      }
    }

    if (!bConfigSet && j.contains("path-config")) {
      if (!j["path-config"].is_string()) {
        spdlog::error("LEVEL2 CLIENT: 'path-config' must be a string");
        return false;
      }
      pathConfig = j["path-config"].get<std::string>();
      embeddedConfig.clear();
    }

    if (j.contains("config-suffix")) {
      if (!j["config-suffix"].is_string() || !validSuffix(j["config-suffix"].get<std::string>())) {
        spdlog::error("LEVEL2 CLIENT: 'config-suffix' must be a short suffix like \".json\"");
        return false;
      }
      configSuffix = j["config-suffix"].get<std::string>();
    }

    if (j.contains("guid")) {
      if (!j["guid"].is_string()) {
        spdlog::error("LEVEL2 CLIENT: 'guid' must be a string");
        return false;
      }
      std::string strGuid = j["guid"].get<std::string>();
      vscp_trim(strGuid);
      if (strGuid.empty() || ("-" == strGuid)) {
        memset(guid, 0, sizeof(guid));
      }
      else if (!vscp_parseGuid(guid, strGuid)) {
        spdlog::error("LEVEL2 CLIENT: Invalid 'guid'");
        return false;
      }
    }

    if (j.contains("response-timeout")) {
      if (!j["response-timeout"].is_number_unsigned()) {
        spdlog::error("LEVEL2 CLIENT: 'response-timeout' must be a positive number");
        return false;
      }
      responseTimeout = j["response-timeout"].get<uint32_t>();
    }

    if (j.contains("connection-timeout")) {
      if (!j["connection-timeout"].is_number_unsigned()) {
        spdlog::error("LEVEL2 CLIENT: 'connection-timeout' must be a positive number");
        return false;
      }
      connectionTimeout = j["connection-timeout"].get<uint32_t>();
    }

    // Filter
    if (j.contains("priority-filter")) {
      filter.filter_priority = j["priority-filter"].get<uint8_t>();
    }
    if (j.contains("priority-mask")) {
      filter.mask_priority = j["priority-mask"].get<uint8_t>();
    }
    if (j.contains("class-filter")) {
      filter.filter_class = j["class-filter"].get<uint16_t>();
    }
    if (j.contains("class-mask")) {
      filter.mask_class = j["class-mask"].get<uint16_t>();
    }
    if (j.contains("type-filter")) {
      filter.filter_type = j["type-filter"].get<uint16_t>();
    }
    if (j.contains("type-mask")) {
      filter.mask_type = j["type-mask"].get<uint16_t>();
    }
    if (j.contains("guid-filter")) {
      if (!j["guid-filter"].is_string() || !vscp_parseGuid(filter.filter_GUID, j["guid-filter"].get<std::string>())) {
        spdlog::error("LEVEL2 CLIENT: Invalid 'guid-filter'");
        return false;
      }
    }
    if (j.contains("guid-mask")) {
      if (!j["guid-mask"].is_string() || !vscp_parseGuid(filter.mask_GUID, j["guid-mask"].get<std::string>())) {
        spdlog::error("LEVEL2 CLIENT: Invalid 'guid-mask'");
        return false;
      }
    }
  }
  catch (const json::exception &ex) {
    spdlog::error("LEVEL2 CLIENT: Invalid configuration: {}", ex.what());
    return false;
  }

  setName(name);
  m_pathDriver        = pathDriver;
  m_pathConfig        = pathConfig;
  m_embeddedConfig    = embeddedConfig;
  m_configSuffix      = configSuffix;
  m_responseTimeout   = responseTimeout;
  m_connectionTimeout = connectionTimeout;
  memcpy(m_guid, guid, sizeof(m_guid));
  {
    std::lock_guard<std::mutex> qlock(m_mutexReceiveQueue);
    m_filterIn = filter;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// loadDriver
//

int
vscpClientLevel2::loadDriver(void)
{
  if (m_pathDriver.empty()) {
    spdlog::error("LEVEL2 CLIENT: No driver path set");
    return VSCP_ERROR_INVALID_PATH;
  }

  // RTLD_NODELETE keeps the code mapped after dlclose. A driver that leaves
  // a thread running after VSCPClose would otherwise crash the application.
  int flags = RTLD_NOW | RTLD_LOCAL;
#ifdef RTLD_NODELETE
  flags |= RTLD_NODELETE;
#endif

  dlerror();
  m_hdll = dlopen(m_pathDriver.c_str(), flags);
  if (nullptr == m_hdll) {
    const char *err = dlerror();
    spdlog::error("LEVEL2 CLIENT: Unable to load driver {}: {}", m_pathDriver, (nullptr != err) ? err : "?");
    return VSCP_ERROR_INIT_FAIL;
  }

  m_procOpen       = (LPFNDLL_VSCPOPEN) dlsym(m_hdll, "VSCPOpen");
  m_procClose      = (LPFNDLL_VSCPCLOSE) dlsym(m_hdll, "VSCPClose");
  m_procWrite      = (LPFNDLL_VSCPWRITE) dlsym(m_hdll, "VSCPWrite");
  m_procRead       = (LPFNDLL_VSCPREAD) dlsym(m_hdll, "VSCPRead");
  m_procGetVersion = (LPFNDLL_VSCPGETVERSION) dlsym(m_hdll, "VSCPGetVersion");

  // Optional
  m_procGetVendorString = (LPFNDLL_VSCPGETVENDORSTRING) dlsym(m_hdll, "VSCPGetVendorString");

  if ((nullptr == m_procOpen) || (nullptr == m_procClose) || (nullptr == m_procWrite) || (nullptr == m_procRead) ||
      (nullptr == m_procGetVersion)) {
    spdlog::error("LEVEL2 CLIENT: {} is not a VSCP Level II driver (missing VSCPOpen, VSCPClose, VSCPWrite, "
                  "VSCPRead or VSCPGetVersion)",
                  m_pathDriver);
    unloadDriver();
    return VSCP_ERROR_INIT_FAIL;
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// unloadDriver
//

void
vscpClientLevel2::unloadDriver(void)
{
  if (nullptr != m_hdll) {
    dlclose(m_hdll);
    m_hdll = nullptr;
  }
  m_procOpen            = nullptr;
  m_procClose           = nullptr;
  m_procWrite           = nullptr;
  m_procRead            = nullptr;
  m_procGetVersion      = nullptr;
  m_procGetVendorString = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
// writeTempConfig
//

int
vscpClientLevel2::writeTempConfig(void)
{
  const std::string suffix = m_configSuffix.empty() ? guessSuffix(m_embeddedConfig) : m_configSuffix;

  const std::string dir = tempDirectory();
  if (dir.empty()) {
    spdlog::error("LEVEL2 CLIENT: No temporary directory");
    return VSCP_ERROR_INVALID_PATH;
  }

#ifndef WIN32
  // mkstemps creates the file exclusively and readable by the owner only,
  // which matters as configurations often hold credentials and keys
  std::string tmpl = dir + "/vscpl2drv-XXXXXX" + suffix;
  std::vector<char> buf(tmpl.begin(), tmpl.end());
  buf.push_back('\0');
  int fd = mkstemps(buf.data(), static_cast<int>(suffix.size()));
  if (-1 == fd) {
    spdlog::error("LEVEL2 CLIENT: Unable to create temporary configuration file: {}", strerror(errno));
    return VSCP_ERROR_INVALID_PATH;
  }
  std::string path(buf.data());
  FILE *fp = fdopen(fd, "wb");
  if (nullptr == fp) {
    close(fd);
    unlink(path.c_str());
    return VSCP_ERROR_WRITE_ERROR;
  }
#else
  // The temp directory is per user on Windows. "x" creates the file
  // exclusively so an existing file is never reused.
  std::random_device rd;
  std::string path;
  FILE *fp = nullptr;
  for (int i = 0; (i < 16) && (nullptr == fp); i++) {
    char name[64];
    snprintf(name, sizeof(name), "vscpl2drv-%08x%08x", rd(), rd());
    path = dir + "\\" + name + suffix;
    fp   = fopen(path.c_str(), "wbx");
  }
  if (nullptr == fp) {
    spdlog::error("LEVEL2 CLIENT: Unable to create temporary configuration file");
    return VSCP_ERROR_INVALID_PATH;
  }
#endif

  bool ok = writeAll(fp, m_embeddedConfig);
  if (0 != fclose(fp)) {
    ok = false;
  }
  if (!ok) {
    spdlog::error("LEVEL2 CLIENT: Unable to write temporary configuration file {}", path);
    remove(path.c_str());
    return VSCP_ERROR_WRITE_ERROR;
  }

  m_tempConfigPath = path;
  spdlog::debug("LEVEL2 CLIENT: Embedded configuration written to {}", path);
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// removeTempConfig
//

void
vscpClientLevel2::removeTempConfig(void)
{
  if (m_tempConfigPath.empty()) {
    return;
  }
  if ((0 != remove(m_tempConfigPath.c_str())) && (ENOENT != errno)) {
    spdlog::warn("LEVEL2 CLIENT: Unable to remove temporary configuration file {}: {}",
                 m_tempConfigPath,
                 strerror(errno));
  }
  m_tempConfigPath.clear();
}

///////////////////////////////////////////////////////////////////////////////
// connect
//

int
vscpClientLevel2::connect(void)
{
  std::lock_guard<std::mutex> lock(m_lifecycleMutex);
  if (m_bConnected) {
    return VSCP_ERROR_SUCCESS;
  }

  if (m_embeddedConfig.empty() && m_pathConfig.empty()) {
    spdlog::error("LEVEL2 CLIENT: No driver configuration (path-config or config) set");
    return VSCP_ERROR_PARAMETER;
  }

  int rv = loadDriver();
  if (VSCP_ERROR_SUCCESS != rv) {
    return rv;
  }

  std::string configPath = m_pathConfig;
  if (!m_embeddedConfig.empty()) {
    if (VSCP_ERROR_SUCCESS != (rv = writeTempConfig())) {
      unloadDriver();
      return rv;
    }
    configPath = m_tempConfigPath;
  }

  try {
    m_openHandle = m_procOpen(configPath.c_str(), m_guid);
  }
  catch (const std::exception &ex) {
    logDriverException("VSCPOpen", &ex);
    m_openHandle = 0;
  }
  catch (...) {
    logDriverException("VSCPOpen", nullptr);
    m_openHandle = 0;
  }
  if (0 == m_openHandle) {
    spdlog::error("LEVEL2 CLIENT: Driver {} failed to open with configuration {}", m_pathDriver, configPath);
    removeTempConfig();
    unloadDriver();
    return VSCP_ERROR_CONNECTION;
  }

  m_bRun = true;
  try {
    m_workerThread = std::thread(&vscpClientLevel2::workerThread, this);
  }
  catch (const std::system_error &ex) {
    spdlog::error("LEVEL2 CLIENT: Unable to start receive thread: {}", ex.what());
    m_bRun = false;
    try {
      m_procClose(m_openHandle);
    }
    catch (const std::exception &dex) {
      logDriverException("VSCPClose", &dex);
    }
    catch (...) {
      logDriverException("VSCPClose", nullptr);
    }
    m_openHandle = 0;
    removeTempConfig();
    unloadDriver();
    return VSCP_ERROR_OPERATION_FAILED;
  }

  m_bConnected = true;
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// disconnect
//

int
vscpClientLevel2::disconnect(void)
{
  std::lock_guard<std::mutex> lock(m_lifecycleMutex);
  if (!m_bConnected) {
    return VSCP_ERROR_SUCCESS;
  }

  if (m_workerThread.joinable() && (std::this_thread::get_id() == m_workerThread.get_id())) {
    spdlog::error("LEVEL2 CLIENT: Can not disconnect from the receive callback");
    return VSCP_ERROR_OPERATION_FAILED;
  }

  m_bConnected = false;
  m_bRun       = false;
  if (m_workerThread.joinable()) {
    m_workerThread.join();
  }

  {
    // Wait for any send() in progress before the driver goes away
    std::unique_lock<std::shared_timed_mutex> dlock(m_driverMutex);
    try {
      m_procClose(m_openHandle);
    }
    catch (const std::exception &dex) {
      logDriverException("VSCPClose", &dex);
    }
    catch (...) {
      logDriverException("VSCPClose", nullptr);
    }
    m_openHandle = 0;
    unloadDriver();
  }

  removeTempConfig();

  // Wake anyone waiting for events
  m_condReceiveQueue.notify_all();
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// isConnected
//

bool
vscpClientLevel2::isConnected(void)
{
  return m_bConnected;
}

///////////////////////////////////////////////////////////////////////////////
// workerThread
//

void
vscpClientLevel2::workerThread(void)
{
  while (m_bRun) {
    vscpEvent ev;
    memset(&ev, 0, sizeof(ev));

    auto start = std::chrono::steady_clock::now();
    int rv     = VSCP_ERROR_ERROR;
    try {
      rv = m_procRead(m_openHandle, &ev, READ_TIMEOUT);
    }
    catch (const std::exception &ex) {
      logDriverException("VSCPRead", &ex);
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    catch (...) {
      logDriverException("VSCPRead", nullptr);
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    if (CANAL_ERROR_SUCCESS == rv) {
      if ((ev.sizeData > VSCP_LEVEL2_MAXDATA) || (ev.sizeData && (nullptr == ev.pdata))) {
        spdlog::warn("LEVEL2 CLIENT: Driver returned an invalid event");
      }
      else {
        deliverEvent(ev);
      }
    }
    else if ((std::chrono::steady_clock::now() - start) < std::chrono::milliseconds(5)) {
      // Some drivers do not block on an empty queue
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // The driver allocates the data with new[] (vscp_copyEvent)
    vscp_deleteEvent(&ev);
  }
}

///////////////////////////////////////////////////////////////////////////////
// deliverEvent
//

void
vscpClientLevel2::deliverEvent(vscpEvent &ev)
{
  if (0 == ev.timestamp) {
    ev.timestamp = vscp_makeTimeStamp();
  }

  {
    std::lock_guard<std::mutex> qlock(m_mutexReceiveQueue);
    if (!vscp_doLevel2Filter(&ev, &m_filterIn)) {
      return;
    }
  }

  if (isCallbackEvActive() && m_callbackev) {
    m_callbackev(ev, getCallbackObj());
  }

  if (isCallbackExActive() && m_callbackex) {
    vscpEventEx ex;
    if (vscp_convertEventToEventEx(&ex, &ev)) {
      m_callbackex(ex, getCallbackObj());
    }
  }

  if (isCallbackEvActive() || isCallbackExActive()) {
    return;
  }

  vscpEvent *pev = new (std::nothrow) vscpEvent;
  if (nullptr == pev) {
    m_droppedEvents++;
    return;
  }
  memset(pev, 0, sizeof(vscpEvent));
  if (!vscp_copyEvent(pev, &ev)) {
    delete pev;
    m_droppedEvents++;
    return;
  }

  {
    std::lock_guard<std::mutex> qlock(m_mutexReceiveQueue);
    if (m_receiveQueue.size() >= MAX_RECEIVE_QUEUE) {
      vscp_deleteEvent_v2(&pev);
      m_droppedEvents++;
      spdlog::warn("LEVEL2 CLIENT: Receive queue full, event dropped");
      return;
    }
    m_receiveQueue.push_back(pev);
  }
  m_condReceiveQueue.notify_one();
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int
vscpClientLevel2::send(vscpEvent &ev)
{
  if ((ev.sizeData > VSCP_LEVEL2_MAXDATA) || (ev.sizeData && (nullptr == ev.pdata))) {
    return VSCP_ERROR_PARAMETER;
  }
  std::shared_lock<std::shared_timed_mutex> dlock(m_driverMutex);
  if (!m_bConnected || (nullptr == m_procWrite)) {
    return VSCP_ERROR_NOT_CONNECTED;
  }
  try {
    return m_procWrite(m_openHandle, &ev, m_responseTimeout);
  }
  catch (const std::exception &ex) {
    logDriverException("VSCPWrite", &ex);
  }
  catch (...) {
    logDriverException("VSCPWrite", nullptr);
  }
  return VSCP_ERROR_WRITE_ERROR;
}

int
vscpClientLevel2::send(vscpEventEx &ex)
{
  vscpEvent ev;
  memset(&ev, 0, sizeof(ev));
  if (!vscp_convertEventExToEvent(&ev, &ex)) {
    return VSCP_ERROR_PARAMETER;
  }
  int rv = send(ev);
  vscp_deleteEvent(&ev);
  return rv;
}

int
vscpClientLevel2::send(canalMsg &msg)
{
  vscpEvent ev;
  memset(&ev, 0, sizeof(ev));
  uint8_t guid[16];
  memcpy(guid, m_guid, sizeof(guid));
  if (!vscp_convertCanalToEvent(&ev, &msg, guid)) {
    vscp_deleteEvent(&ev);
    return VSCP_ERROR_PARAMETER;
  }
  int rv = send(ev);
  vscp_deleteEvent(&ev);
  return rv;
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int
vscpClientLevel2::receive(vscpEvent &ev)
{
  vscpEvent *pev = nullptr;
  {
    std::lock_guard<std::mutex> qlock(m_mutexReceiveQueue);
    if (m_receiveQueue.empty()) {
      return VSCP_ERROR_RCV_EMPTY;
    }
    pev = m_receiveQueue.front();
    m_receiveQueue.pop_front();
  }

  // The caller owns ev, and its data buffer after this call
  bool ok = vscp_copyEvent(&ev, pev);
  vscp_deleteEvent_v2(&pev);
  return ok ? VSCP_ERROR_SUCCESS : VSCP_ERROR_MEMORY;
}

int
vscpClientLevel2::receive(vscpEventEx &ex)
{
  vscpEvent ev;
  memset(&ev, 0, sizeof(ev));
  int rv = receive(ev);
  if (VSCP_ERROR_SUCCESS != rv) {
    return rv;
  }
  bool ok = vscp_convertEventToEventEx(&ex, &ev);
  vscp_deleteEvent(&ev);
  return ok ? VSCP_ERROR_SUCCESS : VSCP_ERROR_PARAMETER;
}

int
vscpClientLevel2::receive(canalMsg &msg)
{
  vscpEvent ev;
  memset(&ev, 0, sizeof(ev));
  int rv = receive(ev);
  if (VSCP_ERROR_SUCCESS != rv) {
    return rv;
  }
  bool ok = vscp_convertEventToCanal(&msg, &ev);
  vscp_deleteEvent(&ev);
  return ok ? VSCP_ERROR_SUCCESS : VSCP_ERROR_PARAMETER;
}

///////////////////////////////////////////////////////////////////////////////
// receiveBlocking
//

int
vscpClientLevel2::receiveBlocking(vscpEvent &ev, long timeout)
{
  {
    std::unique_lock<std::mutex> qlock(m_mutexReceiveQueue);
    if (m_receiveQueue.empty()) {
      if (!m_bConnected) {
        return VSCP_ERROR_NOT_CONNECTED;
      }
      m_condReceiveQueue.wait_for(qlock, std::chrono::milliseconds((timeout < 0) ? 0 : timeout), [this] {
        return !m_receiveQueue.empty() || !m_bConnected;
      });
      if (m_receiveQueue.empty()) {
        return m_bConnected ? VSCP_ERROR_TIMEOUT : VSCP_ERROR_NOT_CONNECTED;
      }
    }
  }
  return receive(ev);
}

int
vscpClientLevel2::receiveBlocking(vscpEventEx &ex, long timeout)
{
  vscpEvent ev;
  memset(&ev, 0, sizeof(ev));
  int rv = receiveBlocking(ev, timeout);
  if (VSCP_ERROR_SUCCESS != rv) {
    return rv;
  }
  bool ok = vscp_convertEventToEventEx(&ex, &ev);
  vscp_deleteEvent(&ev);
  return ok ? VSCP_ERROR_SUCCESS : VSCP_ERROR_PARAMETER;
}

int
vscpClientLevel2::receiveBlocking(canalMsg &msg, long timeout)
{
  vscpEvent ev;
  memset(&ev, 0, sizeof(ev));
  int rv = receiveBlocking(ev, timeout);
  if (VSCP_ERROR_SUCCESS != rv) {
    return rv;
  }
  bool ok = vscp_convertEventToCanal(&msg, &ev);
  vscp_deleteEvent(&ev);
  return ok ? VSCP_ERROR_SUCCESS : VSCP_ERROR_PARAMETER;
}

///////////////////////////////////////////////////////////////////////////////
// setfilter
//

int
vscpClientLevel2::setfilter(vscpEventFilter &filter)
{
  std::lock_guard<std::mutex> qlock(m_mutexReceiveQueue);
  m_filterIn = filter;
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getcount
//

int
vscpClientLevel2::getcount(uint16_t *pcount)
{
  if (nullptr == pcount) {
    return VSCP_ERROR_PARAMETER;
  }
  std::lock_guard<std::mutex> qlock(m_mutexReceiveQueue);
  *pcount = static_cast<uint16_t>(std::min<size_t>(m_receiveQueue.size(), 0xffff));
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// clear
//

int
vscpClientLevel2::clear(void)
{
  std::lock_guard<std::mutex> qlock(m_mutexReceiveQueue);
  while (!m_receiveQueue.empty()) {
    vscpEvent *pev = m_receiveQueue.front();
    m_receiveQueue.pop_front();
    vscp_deleteEvent_v2(&pev);
  }
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getversion
//

int
vscpClientLevel2::getversion(uint8_t *pmajor, uint8_t *pminor, uint8_t *prelease, uint8_t *pbuild)
{
  if ((nullptr == pmajor) || (nullptr == pminor) || (nullptr == prelease) || (nullptr == pbuild)) {
    return VSCP_ERROR_PARAMETER;
  }

  std::lock_guard<std::mutex> lock(m_lifecycleMutex);
  if (!m_bConnected || (nullptr == m_procGetVersion)) {
    return VSCP_ERROR_NOT_CONNECTED;
  }

  unsigned long ver = 0;
  try {
    ver = m_procGetVersion();
  }
  catch (...) {
    logDriverException("VSCPGetVersion", nullptr);
    return VSCP_ERROR_ERROR;
  }
  *pmajor           = (ver >> 24) & 0xff;
  *pminor           = (ver >> 16) & 0xff;
  *prelease         = (ver >> 8) & 0xff;
  *pbuild           = ver & 0xff;
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getVendorString
//

std::string
vscpClientLevel2::getVendorString(void)
{
  std::lock_guard<std::mutex> lock(m_lifecycleMutex);
  if (!m_bConnected || (nullptr == m_procGetVendorString)) {
    return std::string();
  }
  const char *str = nullptr;
  try {
    str = m_procGetVendorString();
  }
  catch (...) {
    logDriverException("VSCPGetVendorString", nullptr);
  }
  return (nullptr != str) ? std::string(str) : std::string();
}

///////////////////////////////////////////////////////////////////////////////
// getinterfaces
//

int
vscpClientLevel2::getinterfaces(std::deque<std::string> &iflist)
{
  iflist.clear();
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getwcyd
//

int
vscpClientLevel2::getwcyd(uint64_t &wcyd)
{
  wcyd = VSCP_SERVER_CAPABILITY_NONE;
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// Timeouts
//

void
vscpClientLevel2::setConnectionTimeout(uint32_t timeout)
{
  m_connectionTimeout = timeout;
}

uint32_t
vscpClientLevel2::getConnectionTimeout(void)
{
  return m_connectionTimeout;
}

void
vscpClientLevel2::setResponseTimeout(uint32_t timeout)
{
  m_responseTimeout = timeout;
}

uint32_t
vscpClientLevel2::getResponseTimeout(void)
{
  return m_responseTimeout;
}

///////////////////////////////////////////////////////////////////////////////
// Configuration getters
//

std::string
vscpClientLevel2::getDriverPath(void)
{
  std::lock_guard<std::mutex> lock(m_lifecycleMutex);
  return m_pathDriver;
}

std::string
vscpClientLevel2::getConfigPath(void)
{
  std::lock_guard<std::mutex> lock(m_lifecycleMutex);
  return m_pathConfig;
}

std::string
vscpClientLevel2::getEmbeddedConfig(void)
{
  std::lock_guard<std::mutex> lock(m_lifecycleMutex);
  return m_embeddedConfig;
}

bool
vscpClientLevel2::isConfigEmbedded(void)
{
  std::lock_guard<std::mutex> lock(m_lifecycleMutex);
  return !m_embeddedConfig.empty();
}

std::string
vscpClientLevel2::getActiveConfigPath(void)
{
  std::lock_guard<std::mutex> lock(m_lifecycleMutex);
  return m_tempConfigPath.empty() ? m_pathConfig : m_tempConfigPath;
}
