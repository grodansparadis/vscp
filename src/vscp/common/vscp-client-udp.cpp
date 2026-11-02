// vscp_client_udp.cpp
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

#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <pch.h>
#include <iphlpapi.h> // GetAdaptersAddresses
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif
#include <stdbool.h>
#include <getopt.h>
#include <vscp.h>
#include <vscphelper.h>
#include <crc.h>

#include <mustache.hpp>
#include <nlohmann/json.hpp> // Needs C++11  -std=c++11

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "vscphelper.h"
#include "vscp-client-udp.h"
#include "vscp-client-datagram.h"

// https://github.com/nlohmann/json
using json = nlohmann::json;
using namespace kainjow::mustache;

#pragma comment(lib, "Ws2_32.lib")

#define BUFFER_SIZE 1024

// Prototypes
void
workerThread(vscpClientUdp *pClient);

///////////////////////////////////////////////////////////////////////////////
// CTor
//

vscpClientUdp::vscpClientUdp()
  : CVscpClient(),
    m_bRun(false),
    m_connected(false),
    m_socketOpen(false),
    m_semReady(false),
#ifdef WIN32
    m_winsockReady(false),
#endif
    m_destination{},
    m_destinationLength(0),
    m_joining(false),
    m_pworkerthread(nullptr)
{
  m_type      = CVscpClient::connType::UDP;
  m_interface = "";                    // Default interface is all
  m_udpAddr   = "255.255.255.255";     // Default destination address is broadcast
  m_udpPort   = VSCP_DEFAULT_UDP_PORT; // Default port is VSCP default port

  m_bEncrypt    = false;
  m_encryptType = VSCP_ENCRYPTION_NONE;
  memset(m_key, 0, sizeof(m_key));
  m_bActiveCallbackEv = false;
  m_bActiveCallbackEx = false;
  m_callbackObject    = NULL;

  m_sock = 0;

  // Set default key (obviously not safe and should not be used in production)
  vscp_hexStr2ByteArray(m_key, 32, VSCP_DEFAULT_KEY16);

#ifdef WIN32
  // Initialize Winsock
  m_winsockReady = WSAStartup(MAKEWORD(2, 2), &m_wsaData) == 0;
  if (!m_winsockReady) {
    fprintf(stderr, "WSAStartup failed\n");
  }
#endif

  crcInit();

  vscp_clearVSCPFilter(&m_filter); // Accept all events

#ifdef WIN32
  m_semReceiveQueue = CreateSemaphore(NULL, 0, 0x7fffffff, NULL);
  m_semReady = m_semReceiveQueue != NULL;
#elif !defined(__APPLE__)
  m_semReady = sem_init(&m_semReceiveQueue, 0, 0) == 0;
#endif
#ifndef __APPLE__
  if (!m_semReady) {
    fprintf(stderr, "Cannot initialize the UDP receive semaphore\n");
  }
#endif

  pthread_mutex_init(&m_mutexSocket, NULL);
  pthread_mutex_init(&m_mutexReceiveQueue, NULL);
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

vscpClientUdp::~vscpClientUdp()
{
  disconnect();
  clear();

#ifdef WIN32
  if (m_semReady) {
    CloseHandle(m_semReceiveQueue);
  }
  if (m_winsockReady) {
    WSACleanup();
  }
#elif !defined(__APPLE__)
  if (m_semReady) {
    sem_destroy(&m_semReceiveQueue);
  }
#endif

  pthread_mutex_destroy(&m_mutexSocket);
  pthread_mutex_destroy(&m_mutexReceiveQueue);
}

///////////////////////////////////////////////////////////////////////////////
// getConfigAsJson
//

std::string
vscpClientUdp::getConfigAsJson(void)
{
  std::lock_guard<std::mutex> lifecycle(m_lifecycleMutex);
  vscp_datagram::PthreadMutex mutex{&m_mutexReceiveQueue};
  std::lock_guard<vscp_datagram::PthreadMutex> queueLock(mutex);
  std::string str;
  char buf[65];
  json j;

  j["interface"]          = m_interface.c_str();
  j["ip"]         = vscp_datagram::formatEndpoint(m_udpAddr, m_udpPort);
  j["encryption"] = m_encryptType;
  j["padding"]    = "pkcs7";
  switch (m_encryptType) {

    case VSCP_ENCRYPTION_NONE:
      j["key"] = "";
      break;

    case VSCP_ENCRYPTION_AES128:
      vscp_byteArray2HexStr(buf, m_key, 16);
      j["key"] = buf;
      break;

    case VSCP_ENCRYPTION_AES192:
      vscp_byteArray2HexStr(buf, m_key, 24);
      j["key"] = buf;
      break;

    case VSCP_ENCRYPTION_AES256:
      vscp_byteArray2HexStr(buf, m_key, 32);
      j["key"] = buf;
      break;

    default:
      vscp_writeGuidArrayToString(str, m_key);
      j["key"] = "";
      break;
  }

  // Filter
  j["priority-filter"] = m_filter.filter_priority;
  j["priority-mask"]   = m_filter.mask_priority;
  j["class-filter"]    = m_filter.filter_class;
  j["class-mask"]      = m_filter.mask_class;
  j["type-filter"]     = m_filter.filter_type;
  j["type-mask"]       = m_filter.mask_type;
  vscp_writeGuidArrayToString(str, m_filter.filter_GUID);
  j["guid-filter"] = str.c_str();
  vscp_writeGuidArrayToString(str, m_filter.mask_GUID);
  j["guid-mask"] = str.c_str();

  return j.dump();
}

///////////////////////////////////////////////////////////////////////////////
// initFromJson
//

bool
vscpClientUdp::initFromJson(const std::string &config)
{
  std::lock_guard<std::mutex> lifecycle(m_lifecycleMutex);
  if (m_connected.load() || m_joining || m_pworkerthread) {
    fprintf(stderr, "Disconnect the UDP client before changing its configuration\n");
    return false;
  }
  vscp_datagram::PthreadMutex mutex{&m_mutexReceiveQueue};
  std::lock_guard<vscp_datagram::PthreadMutex> queueLock(mutex);
  try {
    json nextConfig = json::parse(config);
    if (!nextConfig.is_object()) {
      fprintf(stderr, "Invalid UDP configuration object\n");
      return false;
    }
    std::string interfaceName = m_interface;
    std::string address = m_udpAddr;
    uint16_t port = m_udpPort;
    uint8_t encryption = m_encryptType;
    uint8_t key[sizeof(m_key)];
    memcpy(key, m_key, sizeof(key));
    vscpEventFilter filter = m_filter;
    if (nextConfig.contains("interface")) {
      if (!nextConfig["interface"].is_string()) {
        fprintf(stderr, "Invalid UDP interface\n");
        return false;
      }
      interfaceName = nextConfig["interface"].get<std::string>();
    }
    if (nextConfig.contains("ip")) {
      if (!nextConfig["ip"].is_string() ||
          !vscp_datagram::parseEndpoint(nextConfig["ip"].get<std::string>(), address, port)) {
        fprintf(stderr, "Invalid UDP destination\n");
        return false;
      }
    }
    if (nextConfig.contains("port") && !vscp_datagram::jsonPort(nextConfig["port"], port)) {
      fprintf(stderr, "Invalid UDP port\n");
      return false;
    }
    if (!vscp_datagram::eventOptions(nextConfig, encryption, key, filter)) {
      return false;
    }
    m_interface = interfaceName;
    m_udpAddr = address;
    m_udpPort = port;
    m_encryptType = encryption;
    memcpy(m_key, key, sizeof(m_key));
    m_filter = filter;
    m_j_config = std::move(nextConfig);
  }
  catch (const json::exception &ex) {
    fprintf(stderr, "Invalid UDP configuration: %s\n", ex.what());
    return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// connect
//

int
vscpClientUdp::connect(void)
{
  std::unique_lock<std::mutex> lifecycle(m_lifecycleMutex);
  if (m_connected.load()) {
    return VSCP_ERROR_SUCCESS;
  }
  while (m_joining) {
    if (m_pworkerthread && m_pworkerthread->get_id() == std::this_thread::get_id()) {
      fprintf(stderr, "Cannot reconnect UDP from its receive callback\n");
      return VSCP_ERROR_OPERATION_FAILED;
    }
    m_lifecycleCondition.wait(lifecycle);
    if (m_connected.load()) {
      return VSCP_ERROR_SUCCESS;
    }
  }
  if (m_pworkerthread) {
    if (m_pworkerthread->get_id() == std::this_thread::get_id()) {
      fprintf(stderr, "Cannot reconnect UDP from its receive callback\n");
      return VSCP_ERROR_OPERATION_FAILED;
    }
    m_bRun.store(false);
    m_joining = true;
    std::thread *worker = m_pworkerthread;
    lifecycle.unlock();
    worker->join();
    lifecycle.lock();
    delete worker;
    m_pworkerthread = nullptr;
    vscp_datagram::closeClientSocket(*this, m_socketOpen);
    m_joining = false;
    m_lifecycleCondition.notify_all();
  }
#ifdef WIN32
  if (!m_winsockReady) {
    return VSCP_ERROR_CONNECTION;
  }
#endif
  vscp_datagram::Socket sock;
  sockaddr_storage destination{};
  int destinationLength = 0;
  const int result = vscp_datagram::openSocket(m_udpAddr, m_udpPort, m_interface, false, 1,
                                                sock, destination, destinationLength);
  if (result != VSCP_ERROR_SUCCESS) {
    return result;
  }
  {
    vscp_datagram::PthreadMutex mutex{&m_mutexSocket};
    std::lock_guard<vscp_datagram::PthreadMutex> lock(mutex);
    m_sock = sock;
    m_socketOpen = true;
    m_destination = destination;
    m_destinationLength = destinationLength;
    m_connected.store(true);
  }
  m_bRun.store(true);
  try {
    m_pworkerthread = new std::thread(workerThread, this);
  }
  catch (const std::system_error &ex) {
    fprintf(stderr, "Cannot start UDP receive worker: %s\n", ex.what());
    m_connected.store(false);
    m_bRun.store(false);
    vscp_datagram::closeClientSocket(*this, m_socketOpen);
    return VSCP_ERROR_OPERATION_FAILED;
  }
  catch (const std::bad_alloc &) {
    fprintf(stderr, "Cannot allocate UDP receive worker\n");
    m_connected.store(false);
    m_bRun.store(false);
    vscp_datagram::closeClientSocket(*this, m_socketOpen);
    return VSCP_ERROR_MEMORY;
  }
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// disconnect
//

int
vscpClientUdp::disconnect(void)
{
  std::unique_lock<std::mutex> lifecycle(m_lifecycleMutex);
  if (m_pworkerthread && m_pworkerthread->get_id() == std::this_thread::get_id()) {
    m_connected.store(false);
    m_bRun.store(false);
    vscp_datagram::closeClientSocket(*this, m_socketOpen);
    return VSCP_ERROR_SUCCESS;
  }
  while (m_joining) {
    m_lifecycleCondition.wait(lifecycle);
  }
  m_connected.store(false);
  m_bRun.store(false);
  if (m_pworkerthread) {
    m_joining = true;
    std::thread *worker = m_pworkerthread;
    lifecycle.unlock();
    worker->join();
    lifecycle.lock();
    delete worker;
    m_pworkerthread = nullptr;
    vscp_datagram::closeClientSocket(*this, m_socketOpen);
    m_joining = false;
    m_lifecycleCondition.notify_all();
    return VSCP_ERROR_SUCCESS;
  }
  vscp_datagram::closeClientSocket(*this, m_socketOpen);
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// isConnected
//

bool
vscpClientUdp::isConnected(void)
{
  return m_connected.load();
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int
vscpClientUdp::send(vscpEvent &ev)
{
  std::lock_guard<std::mutex> lifecycle(m_lifecycleMutex);
  return vscp_datagram::sendEvent(*this, m_connected, m_destination, m_destinationLength, ev);
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int
vscpClientUdp::send(vscpEventEx &ex)
{
  vscpEvent ev{};
  if (ex.sizeData > VSCP_LEVEL2_MAXDATA || !vscp_convertEventExToEvent(&ev, &ex)) {
    return VSCP_ERROR_INVALID_FRAME;
  }
  const int result = send(ev);
  vscp_deleteEvent(&ev);
  return result;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int
vscpClientUdp::send(canalMsg &msg)
{
  vscpEvent ev{};
  uint8_t guid[16]{};
  if (!vscp_convertCanalToEvent(&ev, &msg, guid)) {
    return VSCP_ERROR_INVALID_FRAME;
  }
  const int result = send(ev);
  vscp_deleteEvent(&ev);
  return result;
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int
vscpClientUdp::receive(vscpEvent &ev)
{
  return vscp_datagram::receiveEvent(*this, m_receiveCondition, m_semReady, ev, false, 0);
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int
vscpClientUdp::receive(vscpEventEx &ex)
{
  return vscp_datagram::receiveEvent(*this, m_receiveCondition, m_semReady, ex, false, 0);
}

//////////////////////////////////////////////////////////////////////////////
// receive
//

int
vscpClientUdp::receive(canalMsg &msg)
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
vscpClientUdp::receiveBlocking(vscpEvent &ev, long timeout)
{
  return vscp_datagram::receiveEvent(*this, m_receiveCondition, m_semReady, ev, true, timeout);
}

///////////////////////////////////////////////////////////////////////////////
// receiveBlocking
//

int
vscpClientUdp::receiveBlocking(vscpEventEx &ex, long timeout)
{
  return vscp_datagram::receiveEvent(*this, m_receiveCondition, m_semReady, ex, true, timeout);
}

///////////////////////////////////////////////////////////////////////////////
// receiveBlocking
//

int
vscpClientUdp::receiveBlocking(canalMsg &msg, long timeout)
{
  vscpEventEx ex{};
  const int result = receiveBlocking(ex, timeout);
  if (result != VSCP_ERROR_SUCCESS) {
    return result;
  }
  return vscp_convertEventExToCanal(&msg, &ex) ? VSCP_ERROR_SUCCESS : VSCP_ERROR_INVALID_FRAME;
}

//////////////////////////////////////////////////////////////////////////////
// setfilter
//

int
vscpClientUdp::setfilter(vscpEventFilter &filter)
{
  vscp_datagram::PthreadMutex mutex{&m_mutexReceiveQueue};
  std::lock_guard<vscp_datagram::PthreadMutex> lock(mutex);
  // Set filter
  m_filter.filter_priority = filter.filter_priority;
  m_filter.mask_priority   = filter.mask_priority;
  m_filter.filter_class    = filter.filter_class;
  m_filter.mask_class      = filter.mask_class;
  m_filter.filter_type     = filter.filter_type;
  m_filter.mask_type       = filter.mask_type;

  // Copy GUIDs
  memcpy(m_filter.filter_GUID, filter.filter_GUID, sizeof(m_filter.filter_GUID));
  memcpy(m_filter.mask_GUID, filter.mask_GUID, sizeof(m_filter.mask_GUID));

  return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// getcount
//

int
vscpClientUdp::getcount(uint16_t *pcount)
{
  if (NULL == pcount) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  pthread_mutex_lock(&m_mutexReceiveQueue);
  *pcount = m_receiveQueue.size();
  pthread_mutex_unlock(&m_mutexReceiveQueue);

  return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// clear
//

int
vscpClientUdp::clear()
{
  return vscp_datagram::clearQueue(*this, m_semReady);
}

//////////////////////////////////////////////////////////////////////////////
// getversion
//

int
vscpClientUdp::getversion(uint8_t *pmajor, uint8_t *pminor, uint8_t *prelease, uint8_t *pbuild)
{
  if (NULL == pmajor) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  if (NULL == pminor) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  if (NULL == prelease) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  if (NULL == pbuild) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  *pmajor   = VSCP_MAJOR_UDP_CLIENT_VERSION;
  *pminor   = VSCP_MINOR_UDP_CLIENT_VERSION;
  *prelease = VSCP_RELEASE_UDP_CLIENT_VERSION;
  *pbuild   = VSCP_BUILD_UDP_CLIENT_VERSION;

  return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// getinterfaces
//

int
vscpClientUdp::getinterfaces(std::deque<std::string> &iflist)
{
  iflist.clear();
  return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// getwcyd
//

int
vscpClientUdp::getwcyd(uint64_t &wcyd)
{
  // This is not applicable for multicast client
  wcyd = 0;

  // Return success
  return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// setConnectionTimeout
//

void
vscpClientUdp::setConnectionTimeout(uint32_t /*timeout*/)
{
  ;
}

//////////////////////////////////////////////////////////////////////////////
// getConnectionTimeout
//

uint32_t
vscpClientUdp::getConnectionTimeout(void)
{
  return 0;
}

//////////////////////////////////////////////////////////////////////////////
// setResponseTimeout
//

void
vscpClientUdp::setResponseTimeout(uint32_t /*timeout*/)
{
  ;
}

//////////////////////////////////////////////////////////////////////////////
// getResponseTimeout
//

uint32_t
vscpClientUdp::getResponseTimeout(void)
{
  return 0;
}

//////////////////////////////////////////////////////////////////////////////
// workerThread
//

void
workerThread(vscpClientUdp *pClient)
{
  if (pClient) {
    vscp_datagram::receiveLoop(*pClient, pClient->m_connected, pClient->m_receiveCondition, pClient->m_semReady);
  }
}
