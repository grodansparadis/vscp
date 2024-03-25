// vscp_client_canal.h
//
// CANAL client communication classes.
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

#ifndef WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif

#include <pthread.h>

#include "vscp_client_canal.h"
#include "vscphelper.h"

// Forward declaration
static void *
workerThread(void *pObj);

///////////////////////////////////////////////////////////////////////////////
// C-tor
//

vscpClientCanal::vscpClientCanal()
{
  m_type       = CVscpClient::connType::CANAL;
  m_bConnected = false; // Not connected
  // m_tid = 0;
  m_bRun = true;
  pthread_mutex_init(&m_mutexif, NULL);
}

///////////////////////////////////////////////////////////////////////////////
// D-tor
//

vscpClientCanal::~vscpClientCanal()
{
  disconnect();
  pthread_mutex_destroy(&m_mutexif);
}

///////////////////////////////////////////////////////////////////////////////
// init
//

int
vscpClientCanal::init(const std::string &strPath,
                      const std::string &strParameters,
                      unsigned long flags,
                      unsigned long datarate)
{
  if (!datarate) {
    m_canalif.CanalSetBaudrate(datarate); // Unusual implemention and return value may differ if
                                          // not implemented so we do not check
                                          // A standard implemention should return
                                          // CANAL_ERROR_NOT_SUPPORTED if this method is not
                                          // implemented
  }
  return (CANAL_ERROR_SUCCESS == m_canalif.init(strPath, strParameters, flags));
}

///////////////////////////////////////////////////////////////////////////////
// getConfigAsJson
//

std::string
vscpClientCanal::getConfigAsJson(void)
{
  json j;
  std::string rv;

  j["name"]   = getName();
  j["path"]   = m_canalif.getPath();
  j["config"] = m_canalif.getParameter();
  j["flags"]  = m_canalif.getDeviceFlags();

  return j.dump();
}

///////////////////////////////////////////////////////////////////////////////
// initFromJson
//

bool
vscpClientCanal::initFromJson(const std::string &config)
{
  json j;
  try {
    j = json::parse(config);

    if (!j["name"].is_string())
      return false; // Must be set
    if (!j["path"].is_string())
      return false; // Must be set

    if (j.contains("config")) {
      if (!j["config"].is_string())
        return false;
    }
    else {
      j["config"] = ""; // Set default
    }

    if (j.contains("flags")) {
      if (!j["flags"].is_number())
        return false;
    }
    else {
      j["flags"] = 0; // Set default
    }

    if (j.contains("datarate")) {
      if (!j["datarate"].is_number())
        return false;
    }
    else {
      j["datarate"] = 0; // Set default
    }

    setName(j["name"]);
    return (init(j["path"], j["config"], j["flags"], j["datarate"]));
  }
  catch (...) {
    return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// connect
//

int
vscpClientCanal::connect(void)
{
  int rv;

  // Yes we need to run to work and disconnect may have
  // instructed otherwise
  m_bRun = true;

  rv = m_canalif.CanalOpen();
  if (VSCP_ERROR_SUCCESS == rv) {
    m_bConnected = true;
  }

  // Start worker thread if a callback has been defined
  if ((NULL != m_evcallback) || (NULL != m_excallback)) {
    rv = pthread_create(&m_tid, NULL, workerThread, this);
  }

  return rv;
}

///////////////////////////////////////////////////////////////////////////////
// disconnect
//

int
vscpClientCanal::disconnect(void)
{
  m_bRun = false;
#ifdef WIN32
  Sleep(1000);
#else
  sleep(1); // Give thread some time to die
#endif

  pthread_mutex_unlock(&m_mutexif);
  if ((NULL != m_evcallback) || (NULL != m_excallback)) {
    pthread_join(m_tid, NULL);
  }
  pthread_mutex_destroy(&m_mutexif);

  m_bConnected = false;

  // Disable callbacks
  // m_evcallback = nullptr;
  // m_excallback = nullptr;

  return m_canalif.CanalClose();
}

///////////////////////////////////////////////////////////////////////////////
// isConnected
//

bool
vscpClientCanal::isConnected(void)
{
  return m_bConnected;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int
vscpClientCanal::send(vscpEvent &ev)
{
  canalMsg canalMsg;

  pthread_mutex_lock(&m_mutexif);
  if (!vscp_convertEventToCanal(&canalMsg, &ev)) {
    pthread_mutex_unlock(&m_mutexif);
    return VSCP_ERROR_PARAMETER;
  }
  pthread_mutex_unlock(&m_mutexif);

  return m_canalif.CanalSend(&canalMsg);
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int
vscpClientCanal::send(vscpEventEx &ex)
{
  canalMsg canalMsg;

  pthread_mutex_lock(&m_mutexif);
  if (!vscp_convertEventExToCanal(&canalMsg, &ex)) {
    pthread_mutex_unlock(&m_mutexif);
    return VSCP_ERROR_PARAMETER;
  }
  pthread_mutex_unlock(&m_mutexif);

  return m_canalif.CanalSend(&canalMsg);
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int
vscpClientCanal::receive(vscpEvent &ev)
{
  int rv;
  canalMsg canalMsg;
  uint8_t guid[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  pthread_mutex_lock(&m_mutexif);
  if (CANAL_ERROR_SUCCESS != (rv = m_canalif.CanalReceive(&canalMsg))) {
    pthread_mutex_unlock(&m_mutexif);
    return rv;
  }
  pthread_mutex_unlock(&m_mutexif);

  return vscp_convertCanalToEvent(&ev, &canalMsg, guid) ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int
vscpClientCanal::receive(vscpEventEx &ex)
{
  int rv;
  canalMsg canalMsg;
  uint8_t guid[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  pthread_mutex_lock(&m_mutexif);
  if (CANAL_ERROR_SUCCESS != (rv = m_canalif.CanalReceive(&canalMsg))) {
    pthread_mutex_unlock(&m_mutexif);
    return rv;
  }
  pthread_mutex_unlock(&m_mutexif);

  return vscp_convertCanalToEventEx(&ex, &canalMsg, guid) ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR;
}

///////////////////////////////////////////////////////////////////////////////
// setfilter
//

int
vscpClientCanal::setfilter(vscpEventFilter &filter)
{
  int rv;

  uint32_t _filter = ((unsigned long) filter.filter_priority << 26) | ((unsigned long) filter.filter_class << 16) |
                     ((unsigned long) filter.filter_type << 8) | filter.filter_GUID[0];
  if (CANAL_ERROR_SUCCESS == (rv = m_canalif.CanalSetFilter(_filter))) {
    return rv;
  }

  uint32_t _mask = ((unsigned long) filter.mask_priority << 26) | ((unsigned long) filter.mask_class << 16) |
                   ((unsigned long) filter.mask_type << 8) | filter.mask_GUID[0];
  return m_canalif.CanalSetMask(_mask);
}

///////////////////////////////////////////////////////////////////////////////
// getcount
//

int
vscpClientCanal::getcount(uint16_t *pcount)
{
  int cnt;

  if ((NULL == m_evcallback) && (NULL == m_excallback)) {    
    pthread_mutex_lock(&m_mutexif);
    cnt = m_canalif.CanalDataAvailable();
    pthread_mutex_unlock(&m_mutexif);
  }
  else {
    // we don't know as event has been sent to
    // use queue when it was received
    cnt = 0;
  }
  *pcount = cnt;
  return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// clear
//

int
vscpClientCanal::clear()
{
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getversion
//

int
vscpClientCanal::getversion(uint8_t *pmajor, uint8_t *pminor, uint8_t *prelease, uint8_t *pbuild)
{
  uint32_t ver = m_canalif.CanalGetDllVersion();

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getinterfaces
//

int
vscpClientCanal::getinterfaces(std::deque<std::string> &iflist)
{
  // No interfaces available
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getwcyd
//

int
vscpClientCanal::getwcyd(uint64_t &wcyd)
{
  wcyd = VSCP_SERVER_CAPABILITY_NONE; // No capabilities
  return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// setConnectionTimeout
//

void
vscpClientCanal::setConnectionTimeout(uint32_t timeout)
{
  ;
}

//////////////////////////////////////////////////////////////////////////////
// getConnectionTimeout
//

uint32_t
vscpClientCanal::getConnectionTimeout(void)
{
  return 0;
}

//////////////////////////////////////////////////////////////////////////////
// setResponseTimeout
//

void
vscpClientCanal::setResponseTimeout(uint32_t timeout)
{
  ;
}

//////////////////////////////////////////////////////////////////////////////
// getResponseTimeout
//

uint32_t
vscpClientCanal::getResponseTimeout(void)
{
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// setCallback
//

int
vscpClientCanal::setCallback(LPFNDLL_EV_CALLBACK evcallback)
{
  // Can not be called when connected
  if (m_bConnected)
    return VSCP_ERROR_ERROR;

  m_evcallback = evcallback;
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setCallback
//

int
vscpClientCanal::setCallback(LPFNDLL_EX_CALLBACK excallback)
{
  // Can not be called when connected
  if (m_bConnected)
    return VSCP_ERROR_ERROR;

  m_excallback = excallback;
  return VSCP_ERROR_SUCCESS;
}

#ifdef WIN32
static void
win_usleep(__int64 usec)
{
  HANDLE timer;
  LARGE_INTEGER ft;

  ft.QuadPart = -(10 * usec); // Convert to 100 nanosecond interval, negative value indicates relative time

  timer = CreateWaitableTimer(NULL, TRUE, NULL);
  SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
  WaitForSingleObject(timer, INFINITE);
  CloseHandle(timer);
}
#endif

///////////////////////////////////////////////////////////////////////////////
// Callback workerthread
//
// This thread call the appropriate callback when events are received
//

static void *
workerThread(void *pObj)
{
  uint8_t guid[]           = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  vscpClientCanal *pClient = (vscpClientCanal *) pObj;
  VscpCanalDeviceIf *pif   = (VscpCanalDeviceIf *) &(pClient->m_canalif);

  if (NULL == pif)
    return NULL;

  while (pClient->m_bRun) {

    pthread_mutex_lock(&pClient->m_mutexif);

    // Check if there are events to fetch
    int cnt;
    if ((cnt = pClient->m_canalif.CanalDataAvailable())) {

      while (cnt) {
        canalMsg msg;
        if (CANAL_ERROR_SUCCESS == pClient->m_canalif.CanalReceive(&msg)) {
          if (NULL != pClient->m_evcallback) {
            vscpEvent ev;
            if (vscp_convertCanalToEvent(&ev, &msg, guid)) {
              pClient->m_evcallback(&ev, pClient->m_callbackObject);
              printf("ev");
            }
          }
          if (NULL != pClient->m_excallback) {
            vscpEventEx ex;
            if (vscp_convertCanalToEventEx(&ex, &msg, guid)) {
              pClient->m_excallback(&ex, pClient->m_callbackObject);
              printf("ex");
            }
          }
        }
        cnt--;
      }
    }

    pthread_mutex_unlock(&pClient->m_mutexif);

#ifndef WIN32
    usleep(200);
#else
    win_usleep(200);
#endif
  } // while

  return NULL;
}
