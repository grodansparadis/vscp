// fakecanal.cpp
//
// Minimal CANAL driver for the vscpClientCanal unit tests.
//
// The device string is a comma separated list of 29-bit CAN ids. CanalOpen()
// queues one frame (one data byte, the index of the frame) for each id.
// CanalSetFilter/CanalSetMask are recorded and can be read back with
// fake_lastFilter()/fake_lastMask(). The driver itself does not filter.
//
// SPDX-License-Identifier: MIT

#include <canal.h>

#include <cstdlib>
#include <cstring>
#include <deque>
#include <mutex>
#include <string>

namespace {
std::mutex g_mutex;
std::deque<canalMsg> g_frames;
unsigned long g_filter = 0;
unsigned long g_mask   = 0;
int g_setCount         = 0;
}

extern "C" {

unsigned long
fake_lastFilter(void)
{
  return g_filter;
}

unsigned long
fake_lastMask(void)
{
  return g_mask;
}

int
fake_setCount(void)
{
  return g_setCount;
}

long
CanalOpen(const char *pDevice, unsigned long /*flags*/)
{
  std::lock_guard<std::mutex> lock(g_mutex);
  g_frames.clear();
  g_filter   = 0;
  g_mask     = 0;
  g_setCount = 0;
  std::string dev = pDevice ? pDevice : "";
  size_t pos      = 0;
  uint8_t idx     = 0;
  while (pos < dev.size()) {
    size_t end = dev.find(',', pos);
    if (std::string::npos == end) {
      end = dev.size();
    }
    std::string item = dev.substr(pos, end - pos);
    if (!item.empty()) {
      canalMsg msg = {};
      msg.flags    = CANAL_IDFLAG_EXTENDED;
      msg.id       = (uint32_t) strtoul(item.c_str(), nullptr, 0);
      msg.sizeData = 1;
      msg.data[0]  = idx++;
      g_frames.push_back(msg);
    }
    pos = end + 1;
  }
  return 1;
}

int
CanalClose(long /*handle*/)
{
  return CANAL_ERROR_SUCCESS;
}

unsigned long
CanalGetLevel(long /*handle*/)
{
  return CANAL_LEVEL_STANDARD;
}

int
CanalSend(long /*handle*/, const PCANALMSG /*pCanalMsg*/)
{
  return CANAL_ERROR_SUCCESS;
}

int
CanalBlockingSend(long /*handle*/, const PCANALMSG /*pCanalMsg*/, unsigned long /*timeout*/)
{
  return CANAL_ERROR_SUCCESS;
}

int
CanalReceive(long /*handle*/, PCANALMSG pCanalMsg)
{
  std::lock_guard<std::mutex> lock(g_mutex);
  if (g_frames.empty()) {
    return CANAL_ERROR_FIFO_EMPTY;
  }
  *pCanalMsg = g_frames.front();
  g_frames.pop_front();
  return CANAL_ERROR_SUCCESS;
}

int
CanalBlockingReceive(long handle, PCANALMSG pCanalMsg, unsigned long /*timeout*/)
{
  return CanalReceive(handle, pCanalMsg);
}

int
CanalDataAvailable(long /*handle*/)
{
  std::lock_guard<std::mutex> lock(g_mutex);
  return (int) g_frames.size();
}

int
CanalGetStatus(long /*handle*/, PCANALSTATUS pCanalStatus)
{
  memset(pCanalStatus, 0, sizeof(*pCanalStatus));
  return CANAL_ERROR_SUCCESS;
}

int
CanalGetStatistics(long /*handle*/, PCANALSTATISTICS pCanalStatistics)
{
  memset(pCanalStatistics, 0, sizeof(*pCanalStatistics));
  return CANAL_ERROR_SUCCESS;
}

int
CanalSetFilter(long /*handle*/, unsigned long filter)
{
  std::lock_guard<std::mutex> lock(g_mutex);
  g_filter = filter;
  g_setCount++;
  return CANAL_ERROR_SUCCESS;
}

int
CanalSetMask(long /*handle*/, unsigned long mask)
{
  std::lock_guard<std::mutex> lock(g_mutex);
  g_mask = mask;
  g_setCount++;
  return CANAL_ERROR_SUCCESS;
}

unsigned long
CanalGetVersion(void)
{
  return 1;
}

unsigned long
CanalGetDllVersion(void)
{
  return 1;
}

const char *
CanalGetVendorString(void)
{
  return "fakecanal";
}

const char *
CanalGetDriverInfo(void)
{
  return "";
}

} // extern "C"
