///////////////////////////////////////////////////////////////////////////////
// fakedriver.cpp
//
// A minimal VSCP Level II driver used by the vscpClientLevel2 unit tests.
// Events written to it are looped back and can be read again. It records
// the configuration path, the configuration content and the GUID it was
// opened with so the tests can check them.
//
// A configuration containing the word "fail" makes VSCPOpen fail, and
// "throw" makes it throw an exception.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2026 Ake Hedman and contributors, the VSCP project
// <info@vscp.org>
//

#include <canal.h>
#include <vscp.h>

#include <chrono>
#include <condition_variable>
#include <cstring>
#include <deque>
#include <fstream>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>

#if defined(_WIN32)
#define FAKE_EXPORT extern "C" __declspec(dllexport)
#else
#define FAKE_EXPORT extern "C" __attribute__((visibility("default")))
#endif

namespace {

std::mutex g_mutex;
std::condition_variable g_cond;
std::deque<vscpEvent *> g_queue;
std::string g_configPath;
std::string g_configContent;
bool g_configExisted = false;
unsigned char g_guid[16];
int g_openCount  = 0;
int g_closeCount = 0;
long g_handle    = 0;

void
freeEvent(vscpEvent *pev)
{
  if (nullptr != pev) {
    delete[] pev->pdata;
    delete pev;
  }
}

} // namespace

FAKE_EXPORT long
VSCPOpen(const char *pPathConfig, const unsigned char *pguid)
{
  std::lock_guard<std::mutex> lock(g_mutex);
  g_openCount++;
  g_configPath = (nullptr != pPathConfig) ? pPathConfig : "";
  memset(g_guid, 0, sizeof(g_guid));
  if (nullptr != pguid) {
    memcpy(g_guid, pguid, sizeof(g_guid));
  }

  std::ifstream in(g_configPath, std::ios::binary);
  g_configExisted = in.good();
  std::stringstream ss;
  ss << in.rdbuf();
  g_configContent = g_configExisted ? ss.str() : "";

  if (std::string::npos != g_configContent.find("throw")) {
    throw std::runtime_error("fake driver failure");
  }
  if (!g_configExisted || (std::string::npos != g_configContent.find("fail"))) {
    return 0;
  }
  return g_handle = 1681;
}

FAKE_EXPORT int
VSCPClose(long handle)
{
  std::lock_guard<std::mutex> lock(g_mutex);
  if (handle != g_handle) {
    return CANAL_ERROR_PARAMETER;
  }
  g_closeCount++;
  g_handle = 0;
  while (!g_queue.empty()) {
    freeEvent(g_queue.front());
    g_queue.pop_front();
  }
  return CANAL_ERROR_SUCCESS;
}

FAKE_EXPORT int
VSCPWrite(long handle, const vscpEvent *pEvent, unsigned long /*timeout*/)
{
  if (nullptr == pEvent) {
    return CANAL_ERROR_PARAMETER;
  }
  vscpEvent *pev = new vscpEvent;
  *pev           = *pEvent;
  pev->pdata     = nullptr;
  if (pEvent->sizeData) {
    pev->pdata = new uint8_t[pEvent->sizeData];
    memcpy(pev->pdata, pEvent->pdata, pEvent->sizeData);
  }
  {
    std::lock_guard<std::mutex> lock(g_mutex);
    if (handle != g_handle) {
      freeEvent(pev);
      return CANAL_ERROR_PARAMETER;
    }
    g_queue.push_back(pev);
  }
  g_cond.notify_one();
  return CANAL_ERROR_SUCCESS;
}

FAKE_EXPORT int
VSCPRead(long handle, vscpEvent *pEvent, unsigned long timeout)
{
  if (nullptr == pEvent) {
    return CANAL_ERROR_PARAMETER;
  }
  std::unique_lock<std::mutex> lock(g_mutex);
  if (handle != g_handle) {
    return CANAL_ERROR_PARAMETER;
  }
  if (!g_cond.wait_for(lock, std::chrono::milliseconds(timeout), [] { return !g_queue.empty(); })) {
    return CANAL_ERROR_TIMEOUT;
  }
  vscpEvent *pev = g_queue.front();
  g_queue.pop_front();
  lock.unlock();

  // Same ownership as real drivers: the data buffer is allocated with new[]
  *pEvent = *pev;
  pev->pdata = nullptr;
  freeEvent(pev);
  return CANAL_ERROR_SUCCESS;
}

FAKE_EXPORT unsigned long
VSCPGetVersion(void)
{
  return (1UL << 24) | (2UL << 16) | (3UL << 8) | 4UL;
}

FAKE_EXPORT const char *
VSCPGetVendorString(void)
{
  return "Fake Level II driver";
}

// ----------------------------------------------------------------------------
// Test helpers (not part of the driver interface)
// ----------------------------------------------------------------------------

FAKE_EXPORT void
FakeGetState(char *path,
             size_t pathSize,
             char *content,
             size_t contentSize,
             unsigned char *guid,
             int *pOpenCount,
             int *pCloseCount,
             int *pConfigExisted)
{
  std::lock_guard<std::mutex> lock(g_mutex);
  snprintf(path, pathSize, "%s", g_configPath.c_str());
  snprintf(content, contentSize, "%s", g_configContent.c_str());
  memcpy(guid, g_guid, sizeof(g_guid));
  *pOpenCount     = g_openCount;
  *pCloseCount    = g_closeCount;
  *pConfigExisted = g_configExisted ? 1 : 0;
}
