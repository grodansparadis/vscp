// vscp_client_socketcan.cpp
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
// Copyright © 2000-2024 Ake Hedman, Grodans Paradis AB
// <info@grodansparadis.com>
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

// !!! Only Linux  !!!
#ifndef WIN32

#include <ctype.h>
#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>

#include <linux/can/raw.h>

#include <expat.h>

#include "vscp-client-socketcan.h"
#include <guid.h>
#include <vscp.h>
#include <vscp-class.h>
#include <vscp-type.h>
#include <vscphelper.h>

#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <string>

#include <mustache.hpp>
#include <nlohmann/json.hpp> // Needs C++11  -std=c++11

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

// https://github.com/nlohmann/json
using json = nlohmann::json;
using namespace kainjow::mustache;

// Forward declaration
static void *
workerThread(void *pObj);

// CAN DLC to real data length conversion helpers
static const unsigned char canal_tbldlc2len[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64 };

// get data length from can_dlc with sanitized can_dlc
unsigned char
canal_dlc2len(unsigned char can_dlc)
{
  return canal_tbldlc2len[can_dlc & 0x0F];
}

static const unsigned char canal_tbllen2dlc[] = { 0,  1,  2,  3,  4,  5,  6,  7,  8, /* 0 - 8 */
                                                  9,  9,  9,  9,                     /* 9 - 12 */
                                                  10, 10, 10, 10,                    /* 13 - 16 */
                                                  11, 11, 11, 11,                    /* 17 - 20 */
                                                  12, 12, 12, 12,                    /* 21 - 24 */
                                                  13, 13, 13, 13, 13, 13, 13, 13,    /* 25 - 32 */
                                                  14, 14, 14, 14, 14, 14, 14, 14,    /* 33 - 40 */
                                                  14, 14, 14, 14, 14, 14, 14, 14,    /* 41 - 48 */
                                                  15, 15, 15, 15, 15, 15, 15, 15,    /* 49 - 56 */
                                                  15, 15, 15, 15, 15, 15, 15, 15 };  /* 57 - 64 */

// map the sanitized data length to an appropriate data length code
unsigned char
canal_len2dlc(unsigned char len)
{
  if (len > 64) {
    return 0xF;
  }

  return canal_tbllen2dlc[len];
}

///////////////////////////////////////////////////////////////////////////////
// C-tor
//

vscpClientSocketCan::vscpClientSocketCan()
{
  m_type       = CVscpClient::connType::SOCKETCAN;
  m_bDebug     = false;
  m_bConnected = false; // Not connected
  m_threadWork = 0;
  m_bRun       = true;
  m_interface  = "vcan0";
  m_guid.getFromString("00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00");
  m_flags  = 0;
  m_socket = 0;
  m_mode   = CAN_MTU;

  setResponseTimeout(3); // Response timeout 3 ms
  pthread_mutex_init(&m_mutexSocket, NULL);

  vscp_clearVSCPFilter(&m_filterIn);  // Accept all events
  vscp_clearVSCPFilter(&m_filterOut); // Send all events

  sem_init(&m_semSendQueue, 0, 0);
  sem_init(&m_semReceiveQueue, 0, 0);

  pthread_mutex_init(&m_mutexSendQueue, NULL);
  pthread_mutex_init(&m_mutexReceiveQueue, NULL);
}

///////////////////////////////////////////////////////////////////////////////
// D-tor
//

vscpClientSocketCan::~vscpClientSocketCan()
{
  disconnect();
  pthread_mutex_destroy(&m_mutexSocket);

  sem_destroy(&m_semSendQueue);
  sem_destroy(&m_semReceiveQueue);

  pthread_mutex_destroy(&m_mutexSendQueue);
  pthread_mutex_destroy(&m_mutexReceiveQueue);
}

///////////////////////////////////////////////////////////////////////////////
// init
//

int
vscpClientSocketCan::init(const std::string &interface, const std::string &guid, unsigned long flags, uint32_t timeout)
{
   m_interface = interface;
  m_guid.getFromString(guid);
  m_flags = flags;
  setResponseTimeout(DEAULT_RESPONSE_TIMEOUT); // Response timeout 3 ms
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getConfigAsJson
//

std::string
vscpClientSocketCan::getConfigAsJson(void)
{
  json j;
  std::string rv;

  j["interface"]        = m_interface;
  j["flags"]            = m_flags;
  j["response-timeout"] = getResponseTimeout();

  return rv;
}

///////////////////////////////////////////////////////////////////////////////
// initFromJson
//

bool
vscpClientSocketCan::initFromJson(const std::string &config)
{
  json j;

  try {

    m_j_config = json::parse(config);

    // SocketCAN interface
    if (m_j_config.contains("device")) {
      m_interface = m_j_config["device"].get<std::string>();
      spdlog::debug("SOCKETCAN client: json socketcan init: interface set to {}.", m_interface);
    }

    // flags
    if (m_j_config.contains("flags")) {
      m_flags = m_j_config["flags"].get<uint32_t>();
      spdlog::debug("SOCKETCAN client: json socket init: host set to {}.", m_flags);
    }

    // Response timeout
    if (m_j_config.contains("response-timeout")) {
      uint32_t val = m_j_config["response-timeout"].get<uint32_t>();
      setResponseTimeout(val);
      spdlog::debug("SOCKETCAN client: json socket init: Response Timeout set to {}.", val);
    }

    // Filter
    if (m_j_config.contains("filter") && m_j_config["filter"].is_object()) {

      json j = m_j_config["filter"];

      // IN filter
      if (j.contains("in-filter")) {
        try {
          std::string str = j["in-filter"].get<std::string>();
          vscp_readFilterFromString(&m_filterIn, str.c_str());
        }
        catch (const std::exception &ex) {
          spdlog::error("SOCKETCAN client: Failed to read 'in-filter' Error='{}'", ex.what());
        }
        catch (...) {
          spdlog::error("SOCKETCAN client: Failed to read 'in-filter' due to unknown error.");
        }
      }
      else {
        spdlog::debug("SOCKETCAN client: Failed to read LOGGING 'in-filter' Defaults will be used.");
      }

      // IN mask
      if (j.contains("in-mask")) {
        try {
          std::string str = j["in-mask"].get<std::string>();
          vscp_readMaskFromString(&m_filterIn, str.c_str());
        }
        catch (const std::exception &ex) {
          spdlog::error("SOCKETCAN client: Failed to read 'in-mask' Error='{}'", ex.what());
        }
        catch (...) {
          spdlog::error("SOCKETCAN client: Failed to read 'in-mask' due to unknown error.");
        }
      }
      else {
        spdlog::debug("SOCKETCAN client: Failed to read 'in-mask' Defaults will be used.");
      }

      // OUT filter
      if (j.contains("out-filter")) {
        try {
          std::string str = j["in-filter"].get<std::string>();
          vscp_readFilterFromString(&m_filterOut, str.c_str());
        }
        catch (const std::exception &ex) {
          spdlog::error("SOCKETCAN client:  Failed to read 'out-filter' Error='{}'", ex.what());
        }
        catch (...) {
          spdlog::error("SOCKETCAN client:  Failed to read 'out-filter' due to unknown error.");
        }
      }
      else {
        spdlog::debug("SOCKETCAN client:  Failed to read 'out-filter' Defaults will be used.");
      }

      // OUT mask
      if (j.contains("out-mask")) {
        try {
          std::string str = j["out-mask"].get<std::string>();
          vscp_readMaskFromString(&m_filterOut, str.c_str());
        }
        catch (const std::exception &ex) {
          spdlog::error("SOCKETCAN client:  Failed to read 'out-mask' Error='{}'", ex.what());
        }
        catch (...) {
          spdlog::error("SOCKETCAN client:  Failed to read 'out-mask' due to unknown error.");
        }
      }
      else {
        spdlog::debug("SOCKETCAN client:  Failed to read 'out-mask' Defaults will be used.");
      }
    }
  }
  catch (const std::exception &ex) {
    spdlog::error("SOCKETCAN client: json socketcan init: Failed to parse json: {}", ex.what());
    return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// connect
//

int
vscpClientSocketCan::connect(void)
{
  int rv = VSCP_ERROR_SUCCESS;

  /*
      // open the socket
      if ( (m_socket = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0 )  {
          return CANAL_ERROR_SOCKET_CREATE;
      }

      int mtu, enable_canfd = 1;
      struct sockaddr_can addr;
      struct ifreq ifr;

      strncpy(ifr.ifr_name, m_interface.c_str(), IFNAMSIZ - 1);
      ifr.ifr_name[IFNAMSIZ - 1] = '\0';
      ifr.ifr_ifindex = if_nametoindex(ifr.ifr_name);
      if (!ifr.ifr_ifindex) {
          spdlog::error("SOCKETCAN client: Cant get socketcan index from {0}", m_interface);
          return VSCP_ERROR_ERROR;
      }

      addr.can_family = AF_CAN;
      addr.can_ifindex = ifr.ifr_ifindex;

      if (CANFD_MTU == m_mode) {
          // check if the frame fits into the CAN netdevice
          if (ioctl(m_socket, SIOCGIFMTU, &ifr) < 0) {
              spdlog::error("SOCKETCAN client: FD MTU does not fit for {0}", m_interface);
              return VSCP_TYPE_ERROR_FIFO_SIZE;
          }

          mtu = ifr.ifr_mtu;

          if (mtu != CANFD_MTU) {
              spdlog::error("SOCKETCAN client: CAN FD mode is not supported for {0}", m_interface);
              return VSCP_ERROR_NOT_SUPPORTED;
          }

          // interface is ok - try to switch the socket into CAN FD mode
          if (setsockopt(m_socket,
                              SOL_CAN_RAW,
                              CAN_RAW_FD_FRAMES,
                              &enable_canfd,
                              sizeof(enable_canfd)))
          {
              spdlog::error("SOCKETCAN client: Failed to switch socket to FD mode {0}", m_interface);
              return VSCP_ERROR_NOT_SUPPORTED;
          }

      }
  */
  //const int timestamping_flags = (SOF_TIMESTAMPING_SOFTWARE | \
    //    SOF_TIMESTAMPING_RX_SOFTWARE | \
    //    SOF_TIMESTAMPING_RAW_HARDWARE);

  // if (setsockopt(m_socket, SOL_SOCKET, SO_TIMESTAMPING,
  //     &timestamping_flags, sizeof(timestamping_flags)) < 0) {
  //     perror("setsockopt SO_TIMESTAMPING is not supported by your Linux kernel");
  // }

  // disable default receive filter on this RAW socket
  // This is obsolete as we do not read from the socket at all, but for
  // this reason we can remove the receive list in the Kernel to save a
  // little (really a very little!) CPU usage.
  // setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);

  /*
      struct timeval tv;
      tv.tv_sec = 0;
      tv.tv_usec = getResponseTimeout() * 1000;  // Not init'ing this can cause strange errors
      setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));

      if (bind(m_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
          return CANAL_ERROR_SOCKET_BIND;
      }
  */
  // start the workerthread
  m_bRun = true; // Workerthread should run, run, run...
  if (pthread_create(&m_threadWork, NULL, workerThread, this)) {
    spdlog::critical("SOCKETCAN client: Failed to start workerthread");
    return false;
  }  

  return rv;
}

///////////////////////////////////////////////////////////////////////////////
// disconnect
//

int
vscpClientSocketCan::disconnect(void)
{
  // Do nothing if already terminated
  if (!m_bRun) {
    return VSCP_ERROR_SUCCESS;
  }

  m_bRun = false; // terminate the thread
  // Wait for workerthread to to terminate
  pthread_join(m_threadWork, NULL);

  //::close(m_socket);
  m_bConnected = false;
  return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// isConnected
//

bool
vscpClientSocketCan::isConnected(void)
{
  return m_bConnected;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int
vscpClientSocketCan::send(vscpEvent &ev)
{
  if (m_socket <= 0) {
    return VSCP_ERROR_WRITE_ERROR;
  }

  canalMsg canalMsg;
  if (!vscp_convertEventToCanal(&canalMsg, &ev)) {
    return VSCP_ERROR_PARAMETER;
  }

  struct canfd_frame frame;
  memset(&frame, 0, sizeof(frame)); // init CAN FD frame, e.g. LEN = 0

  // convert CanFrame to canfd_frame
  frame.can_id = canalMsg.id;
  frame.can_id |= CAN_EFF_FLAG;
  frame.len   = canalMsg.sizeData;
  frame.flags = canalMsg.flags;
  memcpy(frame.data, canalMsg.data, canalMsg.sizeData);

  if (m_flags & FLAG_FD_MODE) {
    // Ensure discrete CAN FD length values 0..8, 12, 16, 20, 24, 32, 64
    frame.len = canal_dlc2len(canal_tbllen2dlc[frame.len]);
  }

  // send the frame
  pthread_mutex_lock(&m_mutexSocket);
  if (-1 == write(m_socket, &frame, sizeof(struct can_frame))) {
    pthread_mutex_unlock(&m_mutexSocket);
    return VSCP_ERROR_WRITE_ERROR;
  }
  pthread_mutex_unlock(&m_mutexSocket);

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int
vscpClientSocketCan::send(vscpEventEx &ex)
{
  if (m_socket <= 0) {
    return VSCP_ERROR_WRITE_ERROR;
  }

  canalMsg canalMsg;
  if (!vscp_convertEventExToCanal(&canalMsg, &ex)) {
    return VSCP_ERROR_PARAMETER;
  }

  struct canfd_frame frame;
  memset(&frame, 0, sizeof(frame)); // init CAN FD frame, e.g. LEN = 0

  // convert CanFrame to canfd_frame
  frame.can_id = canalMsg.id;
  frame.can_id |= CAN_EFF_FLAG;
  frame.len   = canalMsg.sizeData;
  frame.flags = canalMsg.flags;
  memcpy(frame.data, canalMsg.data, canalMsg.sizeData);

  if (m_flags & FLAG_FD_MODE) {
    // ensure discrete CAN FD length values 0..8, 12, 16, 20, 24, 32, 64
    frame.len = canal_dlc2len(canal_tbllen2dlc[frame.len]);
  }

  // send the frame
  pthread_mutex_lock(&m_mutexSocket);
  if (-1 == write(m_socket, &frame, sizeof(struct can_frame))) {
    pthread_mutex_unlock(&m_mutexSocket);
    return VSCP_ERROR_WRITE_ERROR;
  }
  pthread_mutex_unlock(&m_mutexSocket);

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int
vscpClientSocketCan::send(canalMsg &msg)
{
  if (m_socket <= 0) {
    return VSCP_ERROR_WRITE_ERROR;
  }

  struct canfd_frame frame;
  memset(&frame, 0, sizeof(frame)); // init CAN FD frame, e.g. LEN = 0

  // convert CanFrame to canfd_frame
  frame.can_id = msg.id;
  frame.can_id |= CAN_EFF_FLAG;
  frame.len   = msg.sizeData;
  frame.flags = msg.flags;
  memcpy(frame.data, msg.data, msg.sizeData);

  if (m_flags & FLAG_FD_MODE) {
    // ensure discrete CAN FD length values 0..8, 12, 16, 20, 24, 32, 64
    frame.len = canal_dlc2len(canal_tbllen2dlc[frame.len]);
  }

  // send the frame
  pthread_mutex_lock(&m_mutexSocket);
  if (-1 == write(m_socket, &frame, sizeof(struct can_frame))) {
    pthread_mutex_unlock(&m_mutexSocket);
    return VSCP_ERROR_WRITE_ERROR;
  }
  pthread_mutex_unlock(&m_mutexSocket);

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int
vscpClientSocketCan::receive(vscpEvent &ev)
{
  int rv;

  // Check if there are any events waiting
  if (!m_receiveList.size()) {
    return VSCP_ERROR_FIFO_EMPTY;
  }

  pthread_mutex_lock(&m_mutexReceiveQueue);
  vscpEvent *pev = m_receiveList.front();
  if (nullptr == pev) {
    pthread_mutex_unlock(&m_mutexReceiveQueue);
    return VSCP_ERROR_INVALID_POINTER;
  }
  vscp_copyEvent(&ev, pev);
  m_receiveList.pop_front();
  pthread_mutex_unlock(&m_mutexReceiveQueue);
  vscp_deleteEvent(pev);

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int
vscpClientSocketCan::receive(vscpEventEx &ex)
{
  int rv;
  vscpEvent ev;

  if (VSCP_ERROR_SUCCESS != (rv = receive(ev))) {
    return rv;
  }

  return (vscp_convertEventToEventEx(&ex, &ev) ? VSCP_ERROR_SUCCESS : VSCP_ERROR_ERROR);
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int
vscpClientSocketCan::receive(canalMsg &msg)
{
  int rv;

  // Check if there are any events waiting
  if (!m_receiveList.size()) {
    return VSCP_ERROR_FIFO_EMPTY;
  }

  pthread_mutex_lock(&m_mutexReceiveQueue);
  vscpEvent *pev = m_receiveList.front();
  if (nullptr == pev) {
    pthread_mutex_unlock(&m_mutexReceiveQueue);
    return VSCP_ERROR_INVALID_POINTER;
  }

  vscp_convertEventToCanal(&msg, pev, 1);
  m_receiveList.pop_front();
  pthread_mutex_unlock(&m_mutexReceiveQueue);
  vscp_deleteEvent(pev);
  pev = nullptr;

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setfilter
//

int
vscpClientSocketCan::setfilter(vscpEventFilter &filter)
{
  int rv;

  uint32_t _filter = ((unsigned long) filter.filter_priority << 26) | ((unsigned long) filter.filter_class << 16) |
                     ((unsigned long) filter.filter_type << 8) | filter.filter_GUID[0];
  // if ( CANAL_ERROR_SUCCESS == (rv = m_canalif.CanalSetFilter(_filter))) {
  //     return rv;
  // }

  uint32_t _mask = ((unsigned long) filter.mask_priority << 26) | ((unsigned long) filter.mask_class << 16) |
                   ((unsigned long) filter.mask_type << 8) | filter.mask_GUID[0];
  return 0; // m_canalif.CanalSetMask(_mask);
}

///////////////////////////////////////////////////////////////////////////////
// getcount
//

int
vscpClientSocketCan::getcount(uint16_t *pcount)
{
  *pcount = m_receiveList.size();
  return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// clear
//

int
vscpClientSocketCan::clear()
{
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getversion
//

int
vscpClientSocketCan::getversion(uint8_t *pmajor, uint8_t *pminor, uint8_t *prelease, uint8_t *pbuild)
{
  // uint32_t ver = m_canalif.CanalGetDllVersion();

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getinterfaces
//

int
vscpClientSocketCan::getinterfaces(std::deque<std::string> &iflist)
{
  // No interfaces available
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getwcyd
//

int
vscpClientSocketCan::getwcyd(uint64_t &wcyd)
{
  wcyd = VSCP_SERVER_CAPABILITY_NONE; // No capabilities
  return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// setConnectionTimeout
//

void
vscpClientSocketCan::setConnectionTimeout(uint32_t timeout)
{
  ;
}

//////////////////////////////////////////////////////////////////////////////
// getConnectionTimeout
//

uint32_t
vscpClientSocketCan::getConnectionTimeout(void)
{
  return 0;
}

//////////////////////////////////////////////////////////////////////////////
// setResponseTimeout
//

void
vscpClientSocketCan::setResponseTimeout(uint32_t timeout)
{
  ;
}

//////////////////////////////////////////////////////////////////////////////
// getResponseTimeout
//

uint32_t
vscpClientSocketCan::getResponseTimeout(void)
{
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// sendToCallbacks
//

void
vscpClientSocketCan::sendToCallbacks(vscpEvent *pev)
{
  if (isCallbackEvActive()) {
    m_callbackev(*pev, getCallbackObj());
  }

  if (isCallbackExActive()) {
    vscpEventEx ex;
    vscp_convertEventToEventEx(&ex, pev);
    m_callbackex(ex, getCallbackObj());
  }
}

///////////////////////////////////////////////////////////////////////////////
// setCallbackEv
//

int
vscpClientSocketCan::setCallbackEv(std::function<void(vscpEvent &ev, void *pobj)> callback)
{
  // Can not be called when connected
  if (m_bConnected) {
    return VSCP_ERROR_ERROR;
  }
  
  CVscpClient::setCallbackEv(callback);

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// setCallbackEx
//

int
vscpClientSocketCan::setCallbackEx(std::function<void(vscpEventEx &ex, void *pobj)> callback)
{
  // Can not be called when connected
  if (m_bConnected) {
    return VSCP_ERROR_ERROR;
  }

  CVscpClient::setCallbackEx(callback);
  return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
//                     Workerthread - Read
//////////////////////////////////////////////////////////////////////

void *
workerThread(void *pData)
{
  int mtu, enable_canfd = 1;
  fd_set rdfs;
  struct timeval tv;
  struct sockaddr_can addr;
  struct ifreq ifr;
  struct cmsghdr *cmsg;
  struct canfd_frame frame;
  char ctrlmsg[CMSG_SPACE(sizeof(struct timeval)) + CMSG_SPACE(sizeof(__u32))];
  const int canfd_on = 1;

  vscpClientSocketCan *pObj = (vscpClientSocketCan *) pData;
  if (NULL == pObj) {
    spdlog::error("SOCKETCAN client: No object data object supplied for worker thread");
    return NULL;
  }

  while (pObj->m_bRun) {

    pthread_mutex_lock(&pObj->m_mutexSocket);
    pObj->m_socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (pObj->m_socket < 0) {

      if (ENETDOWN == errno) {
        pthread_mutex_unlock(&pObj->m_mutexSocket);
        sleep(1);
        continue; // Try again
      }

      spdlog::error("SOCKETCAN client: wrkthread socketcan client: Error while opening socket. Terminating!");
      break;
    }

    strncpy(ifr.ifr_name, pObj->m_interface.c_str(), IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    ifr.ifr_ifindex            = if_nametoindex(ifr.ifr_name);
    if (!ifr.ifr_ifindex) {
      pthread_mutex_unlock(&pObj->m_mutexSocket);
      spdlog::error("SOCKETCAN client: Cant get socketcan index from {0}", pObj->m_interface);
      return NULL;
    }
    // ioctl(pObj->m_socket, SIOCGIFINDEX, &ifr);

    addr.can_family  = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (pObj->m_bDebug) {
      spdlog::debug("SOCKETCAN client: using interface name '{}'.", ifr.ifr_name);
    }

    // try to switch the socket into CAN FD mode
    // setsockopt(pObj->m_socket, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &canfd_on, sizeof(canfd_on));

    if (CANFD_MTU == pObj->m_mode) {

      // check if the frame fits into the CAN netdevice
      if (ioctl(pObj->m_socket, SIOCGIFMTU, &ifr) < 0) {
        pthread_mutex_unlock(&pObj->m_mutexSocket);
        spdlog::error("SOCKETCAN client: FD MTU does not fit for {0}", pObj->m_interface);
        // return VSCP_TYPE_ERROR_FIFO_SIZE;
        return NULL;
      }

      mtu = ifr.ifr_mtu;

      if (mtu != CANFD_MTU) {
        pthread_mutex_unlock(&pObj->m_mutexSocket);
        spdlog::error("SOCKETCAN client: CAN FD mode is not supported for {0}", pObj->m_interface);
        // return VSCP_ERROR_NOT_SUPPORTED;
        return NULL;
      }

      // interface is ok - try to switch the socket into CAN FD mode
      if (setsockopt(pObj->m_socket, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &enable_canfd, sizeof(enable_canfd))) {
        pthread_mutex_unlock(&pObj->m_mutexSocket);
        spdlog::error("SOCKETCAN client: Failed to switch socket to FD mode {0}", pObj->m_interface);
        // return VSCP_ERROR_NOT_SUPPORTED;
        return NULL;
      }
    }

    // Mark as connected
    pObj->setConnected(true);

    pthread_mutex_unlock(&pObj->m_mutexSocket);

    struct timeval tv;
    tv.tv_sec  = 0;
    tv.tv_usec = pObj->getResponseTimeout() * 1000; // Not init'ing this can cause strange errors
    setsockopt(pObj->m_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *) &tv, sizeof(struct timeval));

    if (bind(pObj->m_socket, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
      spdlog::error("SOCKETCAN client: wrkthread socketcan client: Error in socket bind. Terminating!");
      close(pObj->m_socket);
      sleep(2);
      // continue;
      return NULL;
    }

    bool bInnerLoop = true;
    while (pObj->m_bRun && bInnerLoop) {

      FD_ZERO(&rdfs);
      FD_SET(pObj->m_socket, &rdfs);

      tv.tv_sec  = 0;
      tv.tv_usec = 5000; // 5ms timeout

      int ret;
      if ((ret = select(pObj->m_socket + 1, &rdfs, NULL, NULL, &tv)) < 0) {
        // Error
        if (ENETDOWN == errno) {
          // We try to get contact with the net
          // again if it goes down
          bInnerLoop = false;
        }
        else {
          pObj->m_bRun = false;
        }
        continue;
      }

      if (ret) {

        // There is data to read

        pthread_mutex_lock(&pObj->m_mutexSocket);

        ret = read(pObj->m_socket, &frame, sizeof(struct can_frame));
        if (ret < 0) {
          pthread_mutex_unlock(&pObj->m_mutexSocket);
          if (ENETDOWN == errno) {
            // We try to get contact with the net
            // again if it goes down
            bInnerLoop = false;
            sleep(2);
          }
          else {
            pObj->m_bRun = true;
          }
          continue;
        }

        pthread_mutex_unlock(&pObj->m_mutexSocket);

        // Must be Extended
        if (!(frame.can_id & CAN_EFF_FLAG)) {
          continue;
        }

        // Mask of control bits
        frame.can_id &= CAN_EFF_MASK;

        vscpEvent *pEvent = new vscpEvent();
        if (nullptr != pEvent) {

          // This can lead to level I frames having to
          // much data. Later code will handel this case.
          pEvent->pdata = new uint8_t[frame.len];
          if (nullptr == pEvent->pdata) {
            delete pEvent;
            continue;
          }

          // GUID will be set to GUID of interface
          // by driver interface with LSB set to nickname
          // memcpy(pEvent->GUID, pObj->m_guid.getGUID(), 16);
          pEvent->GUID[VSCP_GUID_LSB] = frame.can_id & 0xff;

          // Set VSCP class
          pEvent->vscp_class = vscp_getVscpClassFromCANALid(frame.can_id);

          // Set VSCP type
          pEvent->vscp_type = vscp_getVscpTypeFromCANALid(frame.can_id);

          // Copy data if any
          pEvent->sizeData = frame.len;
          if (frame.len) {
            memcpy(pEvent->pdata, frame.data, frame.len);
          }

          if (vscp_doLevel2Filter(pEvent, &pObj->m_filterIn)) {

            if (pObj->isCallbackEvActive()) {
              pObj->m_callbackev(*pEvent, pObj->getCallbackObj());
            }

            if (pObj->isCallbackExActive()) {
              vscpEventEx ex;
              if (vscp_convertEventToEventEx(&ex, pEvent)) {
                pObj->m_callbackex(ex, pObj->getCallbackObj());
              }
            }

            // printf("Socketcan event: %X:%X\n", pEvent->vscp_class, pEvent->vscp_type);

            // Add to input queue only if no callback set
            if (!pObj->isCallbackEvActive() || !pObj->isCallbackExActive()) {
              // std::cout << "add to receive queue" << std::endl;
              pthread_mutex_lock(&pObj->m_mutexReceiveQueue);
              pObj->m_receiveList.push_back(pEvent);
              sem_post(&pObj->m_semReceiveQueue);
              pthread_mutex_unlock(&pObj->m_mutexReceiveQueue);
            }
          }
          else {
            vscp_deleteEvent(pEvent);
          }
        }
      }
      else {

        // Check if there is event(s) to send
        if (pObj->m_sendList.size()) {

          // Yes there are data to send
          // So send it out on the CAN bus

          pthread_mutex_lock(&pObj->m_mutexSendQueue);
          vscpEvent *pEvent = pObj->m_sendList.front();
          pObj->m_sendList.pop_front();
          pthread_mutex_unlock(&pObj->m_mutexSendQueue);

          if (NULL == pEvent) {
            continue;
          }

          // Class must be a Level I class or a Level II
          // mirror class
          if (pEvent->vscp_class < 512) {
            frame.can_id = vscp_getCANALidFromEvent(pEvent);
            frame.can_id |= CAN_EFF_FLAG; // Always extended
            if (0 != pEvent->sizeData) {
              frame.len = (pEvent->sizeData > 8 ? 8 : pEvent->sizeData);
              memcpy(frame.data, pEvent->pdata, frame.len);
            }
          }
          else if (pEvent->vscp_class < 1024) {
            pEvent->vscp_class -= 512;
            frame.can_id = vscp_getCANALidFromEvent(pEvent);
            frame.can_id |= CAN_EFF_FLAG; // Always extended
            if (0 != pEvent->sizeData) {
              frame.len = ((pEvent->sizeData - 16) > 8 ? 8 : pEvent->sizeData - 16);
              memcpy(frame.data, pEvent->pdata + 16, frame.len);
            }
          }

          // Remove the event
          pthread_mutex_lock(&pObj->m_mutexSendQueue);
          vscp_deleteEvent(pEvent);
          pthread_mutex_unlock(&pObj->m_mutexSendQueue);

          // Write the data
          int nbytes = write(pObj->m_socket, &frame, sizeof(struct can_frame));

        } // event to send

      } // No data to read

    } // Inner loop

    // Close the socket
    close(pObj->m_socket);

  } // Outer loop

  return NULL;
}

#endif // not windows
