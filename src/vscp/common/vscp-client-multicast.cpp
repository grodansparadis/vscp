// vscp_client_multicast.cpp
//
// Multicast client communication classes.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// Copyright:   (C) 2007-2025
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
#include "vscp-client-multicast.h"

// https://github.com/nlohmann/json
using json = nlohmann::json;
using namespace kainjow::mustache;

#pragma comment(lib, "Ws2_32.lib")

#define BUFFER_SIZE             1024
#define DEFAULT_MULTICAST_GROUP "224.0.23.158" // Default multicast group address (VSCP)
#define DEFAULT_MULTICAST_PORT  9598           // Default multicast port (VSCP)

// Prototypes
void
workerThread(vscpClientMulticast *pClient);

///////////////////////////////////////////////////////////////////////////////
// CTor
//

vscpClientMulticast::vscpClientMulticast()
  : CVscpClient()
{
  m_type               = CVscpClient::connType::MULTICAST;
  m_interface          = ""; // Default interface is all
  m_multicastGroupAddr = DEFAULT_MULTICAST_GROUP;
  m_multicastPort      = DEFAULT_MULTICAST_PORT;
  m_ttl                = 1;
  m_bEncrypt           = false;
  m_encryptType        = VSCP_ENCRYPTION_NONE;
  memset(m_key, 0, sizeof(m_key));
  m_bActiveCallbackEv = false;
  m_bActiveCallbackEx = false;
  m_callbackObject    = NULL;

  m_sock = 0;

  // Set default key (obviously not safe and should not be used in production)
  vscp_hexStr2ByteArray(m_key, 32, VSCP_DEFAULT_KEY16);

#ifdef WIN32
  // Initialize Winsock
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    fprintf(stderr, "WSAStartup failed\n");
    return EXIT_FAILURE;
  }
#endif

  crcInit();

  vscp_clearVSCPFilter(&m_filter); // Accept all events

#ifdef WIN32
  m_semReceiveQueue = CreateSemaphore(NULL, 0, 0x7fffffff, NULL);
#else
  sem_init(&m_semReceiveQueue, 0, 0);
#endif

  pthread_mutex_init(&m_mutexSocket, NULL);
  pthread_mutex_init(&m_mutexReceiveQueue, NULL);
}

///////////////////////////////////////////////////////////////////////////////
// DTor
//

vscpClientMulticast::~vscpClientMulticast()
{
  // Disconnect if connected
  if (isConnected()) {
    disconnect();
  }

#ifdef WIN32
  CloseHandle(m_semReceiveQueue);
#else
  sem_destroy(&m_semReceiveQueue);
#endif

  pthread_mutex_destroy(&m_mutexSocket);
  pthread_mutex_destroy(&m_mutexReceiveQueue);
}

///////////////////////////////////////////////////////////////////////////////
// getConfigAsJson
//

std::string
vscpClientMulticast::getConfigAsJson(void)
{
  std::string str;
  char buf[65];
  json j;

  j["interface"]       = m_interface.c_str();
  std::string strGroup = "udp://";
  strGroup += m_multicastGroupAddr;
  strGroup += ":";
  strGroup += std::to_string(m_multicastPort);
  j["ip"]         = strGroup.c_str(); // addr:port
  j["ttl"]        = m_ttl;
  j["encryption"] = m_encryptType;
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
vscpClientMulticast::initFromJson(const std::string &config)
{
  try {
    m_j_config = json::parse(config);

    if (m_j_config.contains("interface") && m_j_config["interface"].is_string()) {
      m_interface = m_j_config["interface"].get<std::string>();
    }

    // Can be of form [udp://]addr[:port]
    if (m_j_config.contains("ip") && m_j_config["ip"].is_string()) {
      std::string strAddr = m_j_config["ip"].get<std::string>();

      if (strAddr.find("udp://") == 0) {
        strAddr = strAddr.substr(6); // Remove "udp://"
      }
      size_t pos = strAddr.find(':');
      if (pos != std::string::npos) {
        // If there is a port number, split it off
        std::string portStr = strAddr.substr(pos + 1);
        if (vscp_isNumber(portStr)) {
          m_multicastPort = vscp_readStringValue(portStr);
        }
        strAddr = strAddr.substr(0, pos); // Remove port from address
      }

      m_multicastGroupAddr = strAddr;
    }

    if (m_j_config.contains("port") && m_j_config["port"].is_number()) {
      m_multicastPort = m_j_config["port"].get<uint16_t>();
    }

    if (m_j_config.contains("ttl") && m_j_config["ttl"].is_number()) {
      m_ttl = m_j_config["ttl"].get<uint8_t>();
    }

    if (m_j_config.contains("encryption") && m_j_config["encryption"].is_number()) {
      m_encryptType = m_j_config["encryption"].get<uint8_t>();
    }

    if (m_j_config.contains("key") && m_j_config["key"].is_string()) {
      switch (m_encryptType) {

        case VSCP_ENCRYPTION_NONE:
          memset(m_key, 0, sizeof(m_key));
          break;

        case VSCP_ENCRYPTION_AES128:
          vscp_hexStr2ByteArray(m_key, 16, m_j_config["key"].get<std::string>().c_str());
          break;

        case VSCP_ENCRYPTION_AES192:
          vscp_hexStr2ByteArray(m_key, 24, m_j_config["key"].get<std::string>().c_str());
          break;

        case VSCP_ENCRYPTION_AES256:
          vscp_hexStr2ByteArray(m_key, 32, m_j_config["key"].get<std::string>().c_str());
          break;

        default:
          memset(m_key, 0, sizeof(m_key));
          break;
      }

      // Filter
      try {
        std::string str;
        m_filter.filter_priority = m_j_config["priority-filter"].get<uint8_t>();
        m_filter.mask_priority   = m_j_config["priority-mask"].get<uint8_t>();
        m_filter.filter_class    = m_j_config["class-filter"].get<uint16_t>();
        m_filter.mask_class      = m_j_config["class-mask"].get<uint16_t>();
        m_filter.filter_type     = m_j_config["type-filter"].get<uint16_t>();
        m_filter.mask_type       = m_j_config["type-mask"].get<uint16_t>();
        vscp_getGuidFromStringToArray(m_filter.filter_GUID, str);
        vscp_getGuidFromStringToArray(m_filter.mask_GUID, str);
      }
      catch (const std::exception &ex) {
        // spdlog::error("SOCKETCAN client: Failed to read 'filter' Error='{}'", ex.what());
        return false;
      }
    }
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
vscpClientMulticast::connect(void)
{
  if (isConnected()) {
    // Already connected
    return VSCP_ERROR_SUCCESS;
  }

  // Create a multicast socket
  if ((m_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("Socket creation failed");
#ifdef WIN32
    WSACleanup();
#endif
    return VSCP_ERROR_CONNECTION;
  }

  std::string interfaceName;
  char interfaceAddress[INET_ADDRSTRLEN];

  std::deque<std::string> interfaceSplit;
  vscp_split(interfaceSplit, m_interface, " ");
  if (interfaceSplit.empty()) {
    // If no specific interface is set, use the default one
    interfaceName = ""; // We use all interfaces
  }
  else {
    // Use the first element ("interface ip")
    interfaceName = interfaceSplit.front();
#ifdef WIN32
#else
    struct ifreq ifr;
    strncpy(ifr.ifr_name, interfaceName.c_str(), IFNAMSIZ);
    ioctl(m_sock, SIOCGIFADDR, &ifr);
    inet_ntop(AF_INET, &(((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr), interfaceAddress, INET_ADDRSTRLEN);
    printf("IP Address: %s %s\n", interfaceAddress, inet_ntoa(((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr));
#endif
  }

  // Set the outgoing interface (optional, for multi-homed hosts)
  // https://github.com/bk138/Multicast-Client-Server-Example/blob/master/src/msock.c
  struct in_addr localInterface;
  if (interfaceName.empty()) {
    // Use all interfaces
    localInterface.s_addr = htonl(INADDR_ANY);
  }
  else {
#ifdef WIN32
    // For Windows, we need to convert the interface name to an address
#else
    localInterface.s_addr = inet_addr(interfaceAddress);
#endif
  }

  if (setsockopt(m_sock, IPPROTO_IP, IP_MULTICAST_IF, (char *) &localInterface, sizeof(localInterface)) < 0) {
    perror("setsockopt (IP_MULTICAST_IF)");
    close(m_sock);
    return VSCP_ERROR_PARAMETER;
  }

  // Set TTL (time to live)
  unsigned char ttl = m_ttl;
  if (setsockopt(m_sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) {
    perror("setsockopt (IP_MULTICAST_TTL)");
    close(m_sock);
    return VSCP_ERROR_PARAMETER;
  }

  // Allow broadcast (for UDP)
  int broadcastPermission = 1; // 0 = disable, 1 = enable
  if (setsockopt(m_sock, SOL_SOCKET, SO_BROADCAST, &broadcastPermission, sizeof(broadcastPermission)) < 0) {
    perror("setsockopt failed");
    close(m_sock);
    return VSCP_ERROR_PARAMETER;
  }

  /*
    Disable multicast loopback
    If disabled, multicast packets sent by the host will not be received by the host itself.
    This is also true for other programs running on the host.
  */
  unsigned char loop = 0; // 0 = disable, 1 = enable
  if (setsockopt(m_sock, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop)) < 0) {
    perror("setsockopt IP_MULTICAST_LOOP failed");
    close(m_sock);
    return VSCP_ERROR_PARAMETER;
  }

  // Allow multiple sockets to use the same port
  int reuse = 1; // 0 = disable, 1 = enable
  if (setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse, sizeof(reuse)) < 0) {
    perror("Setting SO_REUSEADDR failed");
    close(m_sock);
    return VSCP_ERROR_PARAMETER;
  }

  struct sockaddr_in bind_addr;
  bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  bind_addr.sin_port        = htons(m_multicastPort);
  bind_addr.sin_family      = AF_INET;

  // bind server address to socket descriptor
  bind(m_sock, (struct sockaddr *) &bind_addr, sizeof(bind_addr));

  // Join the multicast group
  struct ip_mreq multicast_request;
  multicast_request.imr_multiaddr.s_addr = inet_addr(m_multicastGroupAddr.c_str());
  multicast_request.imr_interface.s_addr = htonl(INADDR_ANY);

  if (setsockopt(m_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *) &multicast_request, sizeof(multicast_request)) < 0) {
    perror("Adding multicast group failed");
    close(m_sock);
    return VSCP_ERROR_OPERATION_FAILED;
  }

  // Create and start receive worker thread
  m_bRun          = true;
  m_pworkerthread = new std::thread(workerThread, this);
  m_bRun          = true;

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// disconnect
//

int
vscpClientMulticast::disconnect(void)
{
#ifdef WIN32
  closesocket(m_sock);
  WSACleanup();
#else
  close(m_sock);
#endif

  m_sock = 0;

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// isConnected
//

bool
vscpClientMulticast::isConnected(void)
{
  if (m_sock <= 0) {
    return false; // Not connected if socket is invalid
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int
vscpClientMulticast::send(vscpEvent &ev)
{
  if (!isConnected()) {
    return VSCP_ERROR_NOT_CONNECTED;
  }

  // Find the needed buffer length
  size_t framelen = vscp_getFrameSizeFromEvent(&ev);

  uint8_t *pframe = new uint8_t[BUFFER_SIZE];
  if (NULL == pframe) {
    return VSCP_ERROR_MEMORY;
  }

  memset(pframe, 0, BUFFER_SIZE);

  // Write event to frame
  if (!vscp_writeEventToFrame(pframe, BUFFER_SIZE, 0, &ev)) {
    free(pframe);
    fprintf(stderr, "Error writing event to frame.\n");
    return VSCP_ERROR_INVALID_FRAME;
  }

  // Encrypt frame as needed
  if (m_encryptType) {

    uint16_t newlen             = 0;
    uint8_t encbuf[BUFFER_SIZE] = { 0 };

    if (0 == (newlen = vscp_encryptFrame(encbuf, pframe, framelen, m_key, NULL, m_encryptType))) {
      fprintf(stderr, "Error encrypting frame. newlen = %d\n", newlen);
      exit(EXIT_FAILURE);
    }

    memcpy(pframe, encbuf, newlen);
    pframe[0] = (pframe[0] & 0xF0) | (m_encryptType & 0x0F); // Set encryption type
    // Set the new length (may be padded to be modulo 16 + 1)
    framelen = newlen;

    if (0) {
      printf("Encrypted frame:\n");
      for (int i = 0; i < framelen; i++) {
        printf("%02x ", pframe[i]);
      }
      printf("\nNew length: %d\n", (int) framelen);
    }
  }

  pthread_mutex_lock(&m_mutexSocket);

  // Send the frame to the multicast group
  struct sockaddr_in multicastAddr;
  multicastAddr.sin_family      = AF_INET;
  multicastAddr.sin_addr.s_addr = inet_addr(m_multicastGroupAddr.c_str());
  multicastAddr.sin_port        = htons(m_multicastPort);

  ssize_t nSent =
    sendto(m_sock, (const char *) pframe, framelen, 0, (struct sockaddr *) &multicastAddr, sizeof(multicastAddr));

  // Frame buffer not needed anymore
  free(pframe);

  if (nSent < 0) {
    pthread_mutex_unlock(&m_mutexSocket);
    perror("sendto failed."); // errno=%d", errno
    return VSCP_ERROR_COMMUNICATION;
  }
  if (0 == nSent) {
    pthread_mutex_unlock(&m_mutexSocket);
    fprintf(stderr, "sendto returned 0 bytes sent.\n");
    return VSCP_ERROR_COMMUNICATION;
  }
  if (nSent != (int) framelen) {
    pthread_mutex_unlock(&m_mutexSocket);
    fprintf(stderr, "sendto sent %d bytes but expected %zu bytes.\n", (int) nSent, framelen);
    return VSCP_ERROR_COMMUNICATION;
  }
  if (0) {
    printf("Sent %d bytes to multicast group %s:%d\n", (int) nSent, m_multicastGroupAddr.c_str(), m_multicastPort);
  }
  pthread_mutex_unlock(&m_mutexSocket);

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int
vscpClientMulticast::send(vscpEventEx &ex)
{
  vscpEvent ev;
  memset(&ev, 0, sizeof(vscpEvent));

  if (!vscp_convertEventExToEvent(&ev, &ex)) {
    return VSCP_ERROR_INVALID_FRAME;
  }

  send(ev);

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// send
//

int
vscpClientMulticast::send(canalMsg &msg)
{
  vscpEventEx ex;

  uint8_t guid[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  if (vscp_convertCanalToEventEx(&ex, &msg, guid)) {
    return VSCP_ERROR_INVALID_FRAME;
  }

  return send(ex);
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int
vscpClientMulticast::receive(vscpEvent &ev)
{
  vscpEvent *pev = nullptr;

  if (m_receiveQueue.size()) {

    pev = m_receiveQueue.front();
    m_receiveQueue.pop_front();
    if (nullptr == pev) {
      return VSCP_ERROR_MEMORY;
    }

    if (!vscp_copyEvent(&ev, pev)) {
      return VSCP_ERROR_MEMORY;
    }

    vscp_deleteEvent_v2(&pev);
  }
  else {
    return VSCP_ERROR_FIFO_EMPTY;
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int
vscpClientMulticast::receive(vscpEventEx &ex)
{
  vscpEvent *pev = nullptr;

  if (m_receiveQueue.size()) {

    pev = m_receiveQueue.front();
    m_receiveQueue.pop_front();
    if (nullptr == pev)
      return VSCP_ERROR_MEMORY;

    if (!vscp_convertEventToEventEx(&ex, pev)) {
      return VSCP_ERROR_MEMORY;
    }

    vscp_deleteEvent_v2(&pev);
  }
  else {
    return VSCP_ERROR_FIFO_EMPTY;
  }

  return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// receive
//

int
vscpClientMulticast::receive(canalMsg &msg)
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
vscpClientMulticast::receiveBlocking(vscpEvent &ev, long timeout)
{
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
vscpClientMulticast::receiveBlocking(vscpEventEx &ex, long timeout)
{
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
vscpClientMulticast::receiveBlocking(canalMsg &msg, long timeout)
{
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

//////////////////////////////////////////////////////////////////////////////
// setfilter
//

int
vscpClientMulticast::setfilter(vscpEventFilter &filter)
{
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
vscpClientMulticast::getcount(uint16_t *pcount)
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
vscpClientMulticast::clear()
{
  pthread_mutex_lock(&m_mutexReceiveQueue);
  while (!m_receiveQueue.empty()) {
    vscpEvent *pev = m_receiveQueue.front();
    m_receiveQueue.pop_front();
    if (NULL != pev) {
      vscp_deleteEvent_v2(&pev);
    }
  }
  pthread_mutex_unlock(&m_mutexReceiveQueue);
  return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// getversion
//

int
vscpClientMulticast::getversion(uint8_t *pmajor, uint8_t *pminor, uint8_t *prelease, uint8_t *pbuild)
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

  *pmajor   = VSCP_MAJOR_MULTICAST_CLIENT_VERSION;
  *pminor   = VSCP_MINOR_MULTICAST_CLIENT_VERSION;
  *prelease = VSCP_RELEASE_MULTICAST_CLIENT_VERSION;
  *pbuild   = VSCP_BUILD_MULTICAST_CLIENT_VERSION;

  return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// getinterfaces
//

int
vscpClientMulticast::getinterfaces(std::deque<std::string> &iflist)
{
  iflist.clear();
  return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// getwcyd
//

int
vscpClientMulticast::getwcyd(uint64_t &wcyd)
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
vscpClientMulticast::setConnectionTimeout(uint32_t /*timeout*/)
{
  ;
}

//////////////////////////////////////////////////////////////////////////////
// getConnectionTimeout
//

uint32_t
vscpClientMulticast::getConnectionTimeout(void)
{
  return 0;
}

//////////////////////////////////////////////////////////////////////////////
// setResponseTimeout
//

void
vscpClientMulticast::setResponseTimeout(uint32_t /*timeout*/)
{
  ;
}

//////////////////////////////////////////////////////////////////////////////
// getResponseTimeout
//

uint32_t
vscpClientMulticast::getResponseTimeout(void)
{
  return 0;
}

//////////////////////////////////////////////////////////////////////////////
// workerThread
//

void
workerThread(vscpClientMulticast *pClient)
{
  uint8_t buf[BUFFER_SIZE];
  int rv;
  fd_set readfds;
  struct timeval timeout;
  struct sockaddr_in senderAddr;
  socklen_t addrLen = sizeof(senderAddr);

  // Check pointer
  if (nullptr == pClient) {
    return;
  }

  while (pClient->m_bRun) {

    // Clear and set the file descriptor set
    FD_ZERO(&readfds);
    FD_SET(pClient->m_sock, &readfds);

    // Set timeout (optional, e.g., 1 seconds)
    timeout.tv_sec  = 1;
    timeout.tv_usec = 0;

    // Wait for data to be received
    int activity = select(pClient->m_sock + 1, &readfds, NULL, NULL, &timeout);

    if (activity < 0) {
      perror("select failed");
      break;
    }
    else if (activity == 0) {
      // Timeout occurred, work on
    }
    else {
      // Data is available to read
      if (FD_ISSET(pClient->m_sock, &readfds)) {

        pthread_mutex_lock(&pClient->m_mutexSocket);
        int nReceived = recvfrom(pClient->m_sock, buf, BUFFER_SIZE, 0, (struct sockaddr *) &senderAddr, &addrLen);
        pthread_mutex_unlock(&pClient->m_mutexSocket);

        if (nReceived > 0) {

          // If encrypted frame decrypt it
          if (buf[0] & 0x0F) {

            if (0) {
              printf("Encrypted frame detected. Type: %d\n", buf[0] & 0x0F);
            }

            uint8_t encbuf[BUFFER_SIZE] = { 0 };
            if (VSCP_ERROR_SUCCESS != vscp_decryptFrame(encbuf,
                                                        buf,
                                                        nReceived - 16,
                                                        pClient->m_key,
                                                        buf + nReceived - 16,
                                                        VSCP_ENCRYPTION_FROM_TYPE_BYTE)) {
              fprintf(stderr, "Error decrypting frame.\n");
              continue;
            }
            if (0) {
              printf("Decrypted frame:\n");
              printf("Length: %d\n", nReceived);
              for (int i = 0; i < nReceived; i++) {
                printf("%02x ", encbuf[i]);
              }
              printf("\n");
            }

            // Copy decrypted frame back to buffer
            memcpy(buf, encbuf, nReceived);

          } // encrypted

          vscpEvent ev;
          memset(&ev, 0, sizeof(ev));

          if (!vscp_getEventFromFrame(&ev, buf, nReceived)) {
            fprintf(stderr, "Error reading event from frame. rv=%d\n", rv);
            continue;
          }

          if (vscp_doLevel2Filter(&ev, &pClient->m_filter)) {

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
              pClient->m_receiveQueue.push_back(&ev);
#ifdef WIN32
              ReleaseSemaphore(pClient->m_semReceiveQueue, 1, NULL);
#else
              sem_post(&pClient->m_semReceiveQueue);
#endif
              pthread_mutex_unlock(&pClient->m_mutexReceiveQueue);
            }
          } // filter

          vscp_deleteEvent(&ev);

        } // nReceived > 0
      } // socket is set
    } // data received

    // Terminate if we are not connected
    if (!pClient->isConnected()) {
      pClient->m_bRun = false;
    }
  } // loop
}