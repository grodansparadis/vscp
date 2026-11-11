// Shared socket and receive-queue support for the UDP and multicast clients.
// Copyright (C) 2007-2026 Ake Hedman, the VSCP project
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef VSCP_CLIENT_DATAGRAM_H
#define VSCP_CLIENT_DATAGRAM_H

#include "vscp-socket-address.h"
#include <pthread.h>
#include "vscp-binary-protocol.h"
#include "vscphelper.h"

#include <nlohmann/json.hpp>
#include <openssl/crypto.h>

#include <algorithm>
#include <atomic>
#include <cerrno>
#include <cctype>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <limits>
#include <memory>
#include <mutex>
#include <new>
#include <string>
#include <utility>
#include <vector>

#ifdef WIN32
#include <iphlpapi.h>
#include <netioapi.h>
#else
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>
#include <poll.h>
#include <unistd.h>
#endif

namespace vscp_datagram {

#ifdef WIN32
using Socket = SOCKET;
const Socket invalidSocket = INVALID_SOCKET;
#else
using Socket = int;
const Socket invalidSocket = -1;
#endif

constexpr size_t bufferSize = 1024;
constexpr size_t minimumFrameSize = 1 + VSCP_BINARY_PACKET_FRAME0_HEADER_LENGTH + 2;

// Max number of received events waiting in a client receive queue
constexpr size_t maxQueuedEvents = 2000;

inline void
logSocketError(const char *operation)
{
#ifdef WIN32
  std::fprintf(stderr, "%s failed (Winsock error %d)\n", operation, WSAGetLastError());
#else
  std::fprintf(stderr, "%s failed: %s\n", operation, std::strerror(errno));
#endif
}

inline void
closeSocket(Socket sock)
{
#ifdef WIN32
  closesocket(sock);
#else
  close(sock);
#endif
}

inline bool
parsePort(const std::string &text, uint16_t &port)
{
  unsigned int value = 0;
  if (text.empty()) {
    return false;
  }
  for (char ch : text) {
    if (ch < '0' || ch > '9') {
      return false;
    }
    value = value * 10 + static_cast<unsigned int>(ch - '0');
    if (value > 65535) {
      return false;
    }
  }
  if (!value) {
    return false;
  }
  port = static_cast<uint16_t>(value);
  return true;
}

inline bool
jsonUnsigned(const nlohmann::json &value, uint64_t maximum, uint64_t &result)
{
  if (!value.is_number_integer()) {
    return false;
  }
  if (value.is_number_unsigned()) {
    result = value.get<uint64_t>();
  }
  else {
    const int64_t number = value.get<int64_t>();
    if (number < 0) {
      return false;
    }
    result = static_cast<uint64_t>(number);
  }
  return result <= maximum;
}

inline bool
jsonPort(const nlohmann::json &value, uint16_t &port)
{
  uint64_t number = 0;
  if (!jsonUnsigned(value, 65535, number) || !number) {
    return false;
  }
  port = static_cast<uint16_t>(number);
  return true;
}

template<typename T>
inline bool
optionalUnsigned(const nlohmann::json &config, const char *name, T &value)
{
  if (!config.contains(name)) {
    return true;
  }
  uint64_t number = 0;
  if (!jsonUnsigned(config[name], (std::numeric_limits<T>::max)(), number)) {
    std::fprintf(stderr, "Invalid datagram option '%s'\n", name);
    return false;
  }
  value = static_cast<T>(number);
  return true;
}

inline uint8_t
aesKeyLength(uint8_t encryption)
{
  switch (encryption) {
    case VSCP_ENCRYPTION_AES128: return 16;
    case VSCP_ENCRYPTION_AES192: return 24;
    case VSCP_ENCRYPTION_AES256: return 32;
    default: return 0;
  }
}

inline bool
authenticatedKeysValid(uint8_t encryption, const uint8_t *key, uint8_t keyLength, const uint8_t *hmacKey)
{
  const uint8_t expectedLength = aesKeyLength(encryption);
  return expectedLength && keyLength == expectedLength &&
         CRYPTO_memcmp(key, hmacKey, expectedLength) != 0;
}

inline bool
authenticationStateValid(uint8_t encryption, const uint8_t *key, uint8_t keyLength, const uint8_t *hmacKey,
                         uint8_t configuredEncryption, const uint8_t *configuredKey)
{
  return encryption == configuredEncryption && CRYPTO_memcmp(key, configuredKey, 32) == 0 &&
         authenticatedKeysValid(encryption, key, keyLength, hmacKey);
}

inline bool
authenticationOptionsExplicit(const nlohmann::json &config, uint8_t encryption, const uint8_t *key,
                              uint8_t configuredEncryption, const uint8_t *configuredKey)
{
  return (config.contains("encryption") || encryption == configuredEncryption) &&
         (config.contains("key") || CRYPTO_memcmp(key, configuredKey, 32) == 0);
}

inline bool
authenticationChanged(bool oldEnabled, bool newEnabled, uint8_t oldEncryption, uint8_t newEncryption,
                      const uint8_t *oldKey, const uint8_t *newKey,
                      const uint8_t *oldHmacKey, const uint8_t *newHmacKey)
{
  return (oldEnabled || newEnabled) &&
         (oldEnabled != newEnabled || oldEncryption != newEncryption ||
          CRYPTO_memcmp(oldKey, newKey, 32) != 0 || CRYPTO_memcmp(oldHmacKey, newHmacKey, 32) != 0);
}

inline bool
eventOptions(const nlohmann::json &config, uint8_t &encryption, uint8_t *key, uint8_t &keyLength,
             bool &hmacEnabled, uint8_t *hmacKey, vscpEventFilter &filter)
{
  if (config.contains("encryption")) {
    uint64_t number = 0;
    if (!jsonUnsigned(config["encryption"], VSCP_ENCRYPTION_AES256, number)) {
      std::fprintf(stderr, "Invalid datagram encryption algorithm\n");
      return false;
    }
    encryption = static_cast<uint8_t>(number);
  }
  if (config.contains("padding") &&
      (!config["padding"].is_string() || config["padding"].get<std::string>() != "pkcs7")) {
    std::fprintf(stderr, "Invalid datagram padding\n");
    return false;
  }
  if (config.contains("key")) {
    if (!config["key"].is_string()) {
      std::fprintf(stderr, "Invalid datagram encryption key\n");
      return false;
    }
    const std::string hex = config["key"].get<std::string>();
    if (encryption == VSCP_ENCRYPTION_NONE) {
      std::memset(key, 0, 32);
      keyLength = 0;
    }
    else {
      const uint8_t expectedLength = aesKeyLength(encryption);
      if (!expectedLength || hex.size() != expectedLength * 2 ||
          !std::all_of(hex.begin(), hex.end(), [](unsigned char ch) { return std::isxdigit(ch) != 0; })) {
        std::fprintf(stderr, "Invalid datagram encryption key\n");
        return false;
      }
      std::memset(key, 0, 32);
      if (vscp_hexStr2ByteArray(key, expectedLength, hex.c_str()) != expectedLength) {
        std::fprintf(stderr, "Cannot decode datagram encryption key\n");
        return false;
      }
      keyLength = expectedLength;
    }
  }
  if (config.contains("hmac-key")) {
    if (!config["hmac-key"].is_string()) {
      std::fprintf(stderr, "Invalid datagram HMAC key\n");
      return false;
    }
    const std::string hex = config["hmac-key"].get<std::string>();
    if (hex.empty()) {
      hmacEnabled = false;
      std::memset(hmacKey, 0, 32);
    }
    else {
      if (hex.size() != 64 ||
          !std::all_of(hex.begin(), hex.end(), [](unsigned char ch) { return std::isxdigit(ch) != 0; }) ||
          vscp_hexStr2ByteArray(hmacKey, 32, hex.c_str()) != 32) {
        std::fprintf(stderr, "Invalid datagram HMAC key\n");
        return false;
      }
      hmacEnabled = true;
    }
  }
  if (hmacEnabled) {
    const uint8_t expectedLength = aesKeyLength(encryption);
    if (!expectedLength || keyLength != expectedLength) {
      std::fprintf(stderr, "Authenticated datagrams require an explicitly configured AES key\n");
      return false;
    }
    if (CRYPTO_memcmp(key, hmacKey, expectedLength) == 0) {
      std::fprintf(stderr, "Datagram HMAC and AES keys must be independent\n");
      return false;
    }
  }
  if (!optionalUnsigned(config, "priority-filter", filter.filter_priority) ||
      !optionalUnsigned(config, "priority-mask", filter.mask_priority) ||
      !optionalUnsigned(config, "class-filter", filter.filter_class) ||
      !optionalUnsigned(config, "class-mask", filter.mask_class) ||
      !optionalUnsigned(config, "type-filter", filter.filter_type) ||
      !optionalUnsigned(config, "type-mask", filter.mask_type)) {
    return false;
  }
  for (const auto &entry : {std::make_pair("guid-filter", filter.filter_GUID),
                            std::make_pair("guid-mask", filter.mask_GUID)}) {
    if (config.contains(entry.first) &&
        (!config[entry.first].is_string() ||
         !vscp_parseGuid(entry.second, config[entry.first].get<std::string>()))) {
      std::fprintf(stderr, "Invalid datagram option '%s'\n", entry.first);
      return false;
    }
  }
  return true;
}

/*!
  Parse the optional "require-encryption" setting.
  When true a legacy (non-HMAC) client drops received frames that are not
  encrypted with the configured algorithm. It needs an AES algorithm and an
  explicitly configured key of matching length.
  @param config Configuration object
  @param encryption Encryption algorithm that will be in effect
  @param keyLength Length of the explicitly configured key (0 if none)
  @param requireEncryption Current value in, new value out
  @return true on success, false if the option is invalid
*/
inline bool
requireEncryptionOption(const nlohmann::json &config, uint8_t encryption, uint8_t keyLength,
                        bool &requireEncryption)
{
  if (config.contains("require-encryption")) {
    if (!config["require-encryption"].is_boolean()) {
      std::fprintf(stderr, "Invalid datagram option 'require-encryption'\n");
      return false;
    }
    requireEncryption = config["require-encryption"].get<bool>();
  }
  if (requireEncryption) {
    const uint8_t expectedLength = aesKeyLength(encryption);
    if (!expectedLength || keyLength != expectedLength) {
      std::fprintf(stderr, "'require-encryption' needs an AES algorithm and an explicitly configured key\n");
      return false;
    }
  }
  return true;
}

inline bool
validIpv6Literal(const std::string &host)
{
  const size_t percent = host.find('%');
  if (percent != std::string::npos &&
      (percent == 0 || percent + 1 == host.size() || host.find('%', percent + 1) != std::string::npos)) {
    return false;
  }
  const std::string address = host.substr(0, percent);
  in6_addr parsed{};
  return inet_pton(AF_INET6, address.c_str(), &parsed) == 1;
}

inline bool
parseEndpoint(const std::string &input, std::string &host, uint16_t &port)
{
  std::string value = input;
  const bool uri = value.compare(0, 6, "udp://") == 0;
  if (uri) {
    value.erase(0, 6);
  }
  if (value.empty() || value.find_first_of(" \t\r\n/?#") != std::string::npos) {
    return false;
  }

  std::string parsedHost;
  uint16_t parsedPort = port;
  if (value.front() == '[') {
    const size_t end = value.find(']');
    if (end == std::string::npos || end == 1) {
      return false;
    }
    parsedHost = value.substr(1, end - 1);
    if (uri) {
      const size_t encodedZone = parsedHost.find("%25");
      if (encodedZone != std::string::npos) {
        parsedHost.replace(encodedZone, 3, "%");
      }
    }
    if (!validIpv6Literal(parsedHost)) {
      return false;
    }
    if (end + 1 < value.size() &&
        (value[end + 1] != ':' || !parsePort(value.substr(end + 2), parsedPort))) {
      return false;
    }
  }
  else {
    if (value.find_first_of("[]") != std::string::npos) {
      return false;
    }
    const size_t colons = std::count(value.begin(), value.end(), ':');
    if (colons >= 2) {
      parsedHost = value;
      if (!validIpv6Literal(parsedHost)) {
        return false;
      }
    }
    else if (colons == 1) {
      const size_t colon = value.find(':');
      parsedHost = value.substr(0, colon);
      if (!parsePort(value.substr(colon + 1), parsedPort)) {
        return false;
      }
    }
    else {
      parsedHost = value;
    }
    if (parsedHost.empty() || (colons < 2 && parsedHost.find('%') != std::string::npos)) {
      return false;
    }
    if (colons < 2 && parsedHost.find_first_not_of("0123456789.") == std::string::npos) {
      in_addr parsed{};
      if (inet_pton(AF_INET, parsedHost.c_str(), &parsed) != 1) {
        return false;
      }
    }
  }
  host = parsedHost;
  port = parsedPort;
  return true;
}

inline std::string
formatEndpoint(const std::string &host, uint16_t port)
{
  if (host.find(':') == std::string::npos) {
    return "udp://" + host + ":" + std::to_string(port);
  }
  std::string encoded = host;
  const size_t zone = encoded.find('%');
  if (zone != std::string::npos) {
    encoded.replace(zone, 1, "%25");
  }
  return "udp://[" + encoded + "]:" + std::to_string(port);
}

inline bool
interfaceIndex(const std::string &value, unsigned int &index)
{
  index = 0;
  if (value.empty()) {
    return true;
  }

  auto lookup = [](const std::string &name) -> unsigned int {
    if (name.empty()) {
      return 0;
    }
    if (name.find_first_not_of("0123456789") == std::string::npos) {
      unsigned long number = 0;
      for (char ch : name) {
        if (number > ((std::numeric_limits<unsigned int>::max)() - (ch - '0')) / 10) {
          return 0;
        }
        number = number * 10 + static_cast<unsigned int>(ch - '0');
      }
      if (!number) {
        return 0;
      }
      char interfaceName[512];
      return if_indextoname(static_cast<unsigned int>(number), interfaceName) ?
               static_cast<unsigned int>(number) : 0;
    }
    return if_nametoindex(name.c_str());
  };

  index = lookup(value);
  if (!index) {
    const size_t space = value.find(' ');
    if (space != std::string::npos) {
      index = lookup(value.substr(0, space));
    }
  }
  if (!index) {
    std::fprintf(stderr, "Unknown network interface '%s'\n", value.c_str());
    return false;
  }
  return true;
}

inline bool
ipv4InterfaceAddress(unsigned int index, in_addr &address)
{
  address.s_addr = htonl(INADDR_ANY);
  if (!index) {
    return true;
  }
#ifdef WIN32
  ULONG size = 0;
  const ULONG flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER;
  ULONG result = GetAdaptersAddresses(AF_INET, flags, nullptr, nullptr, &size);
  if (result != ERROR_BUFFER_OVERFLOW) {
    std::fprintf(stderr, "GetAdaptersAddresses failed (error %lu)\n", static_cast<unsigned long>(result));
    return false;
  }
  std::vector<unsigned char> buffer(size);
  auto *adapters = reinterpret_cast<IP_ADAPTER_ADDRESSES *>(buffer.data());
  result = GetAdaptersAddresses(AF_INET, flags, nullptr, adapters, &size);
  if (result != NO_ERROR) {
    std::fprintf(stderr, "GetAdaptersAddresses failed (error %lu)\n", static_cast<unsigned long>(result));
    return false;
  }
  for (auto *adapter = adapters; adapter; adapter = adapter->Next) {
    if (adapter->IfIndex != index) {
      continue;
    }
    for (auto *unicast = adapter->FirstUnicastAddress; unicast; unicast = unicast->Next) {
      if (unicast->Address.lpSockaddr->sa_family == AF_INET) {
        address = reinterpret_cast<sockaddr_in *>(unicast->Address.lpSockaddr)->sin_addr;
        return true;
      }
    }
  }
#else
  ifaddrs *interfaces = nullptr;
  if (getifaddrs(&interfaces) != 0) {
    logSocketError("getifaddrs");
    return false;
  }
  bool found = false;
  for (ifaddrs *item = interfaces; item; item = item->ifa_next) {
    if (item->ifa_addr && item->ifa_addr->sa_family == AF_INET && if_nametoindex(item->ifa_name) == index) {
      address = reinterpret_cast<sockaddr_in *>(item->ifa_addr)->sin_addr;
      found = true;
      break;
    }
  }
  freeifaddrs(interfaces);
  if (found) {
    return true;
  }
#endif
  std::fprintf(stderr, "Network interface %u has no IPv4 address\n", index);
  return false;
}

inline bool
isMulticast(const sockaddr_storage &address)
{
  if (address.ss_family == AF_INET) {
    const auto *ipv4 = reinterpret_cast<const sockaddr_in *>(&address);
    return (ntohl(ipv4->sin_addr.s_addr) & 0xf0000000U) == 0xe0000000U;
  }
  if (address.ss_family == AF_INET6) {
    const auto *ipv6 = reinterpret_cast<const sockaddr_in6 *>(&address);
    return IN6_IS_ADDR_MULTICAST(&ipv6->sin6_addr);
  }
  return false;
}

inline bool
resolveAddress(const std::string &host, uint16_t port, bool multicast,
               sockaddr_storage &address, int &length)
{
  std::memset(&address, 0, sizeof(address));
  if (host.find(':') != std::string::npos) {
    const size_t percent = host.find('%');
    const std::string literal = host.substr(0, percent);
    auto *ipv6 = reinterpret_cast<sockaddr_in6 *>(&address);
    ipv6->sin6_family = AF_INET6;
#ifdef __APPLE__
    ipv6->sin6_len = sizeof(*ipv6);
#endif
    ipv6->sin6_port = htons(port);
    unsigned int scope = 0;
    if (inet_pton(AF_INET6, literal.c_str(), &ipv6->sin6_addr) != 1 ||
        (percent != std::string::npos && !interfaceIndex(host.substr(percent + 1), scope))) {
      return false;
    }
    ipv6->sin6_scope_id = scope;
    length = sizeof(*ipv6);
    if (multicast && !isMulticast(address)) {
      std::fprintf(stderr, "'%s' is not a multicast address\n", host.c_str());
      return false;
    }
    return true;
  }

  addrinfo hints{};
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_NUMERICSERV;
  addrinfo *results = nullptr;
  const std::string service = std::to_string(port);
  const int result = getaddrinfo(host.c_str(), service.c_str(), &hints, &results);
  if (result != 0) {
    std::fprintf(stderr, "Cannot resolve datagram address '%s' (error %d)\n", host.c_str(), result);
    return false;
  }
  bool found = false;
  for (addrinfo *item = results; item; item = item->ai_next) {
    if ((item->ai_family != AF_INET && item->ai_family != AF_INET6) ||
        item->ai_addrlen > sizeof(address)) {
      continue;
    }
    std::memcpy(&address, item->ai_addr, item->ai_addrlen);
    if (!multicast || isMulticast(address)) {
      length = static_cast<int>(item->ai_addrlen);
      found = true;
      break;
    }
  }
  freeaddrinfo(results);
  if (!found) {
    std::fprintf(stderr, "No usable %saddress for '%s'\n", multicast ? "multicast " : "", host.c_str());
  }
  return found;
}

inline int
openSocket(const std::string &host, uint16_t port, const std::string &interfaceName,
           bool multicast, uint8_t ttl, Socket &result,
           sockaddr_storage &destination, int &destinationLength)
{
  result = invalidSocket;
  if (!resolveAddress(host, port, multicast, destination, destinationLength)) {
    return VSCP_ERROR_PARAMETER;
  }
  unsigned int interface = 0;
  if (!interfaceIndex(interfaceName, interface)) {
    return VSCP_ERROR_PARAMETER;
  }

  const int family = destination.ss_family;
  if (family == AF_INET6) {
    auto *ipv6 = reinterpret_cast<sockaddr_in6 *>(&destination);
    if (interface && ipv6->sin6_scope_id && interface != ipv6->sin6_scope_id) {
      std::fprintf(stderr, "IPv6 address scope conflicts with configured interface\n");
      return VSCP_ERROR_PARAMETER;
    }
    if (!interface) {
      interface = ipv6->sin6_scope_id;
    }
    if (interface) {
      ipv6->sin6_scope_id = interface;
    }
    if (!interface &&
        (IN6_IS_ADDR_LINKLOCAL(&ipv6->sin6_addr) ||
         (IN6_IS_ADDR_MULTICAST(&ipv6->sin6_addr) && (ipv6->sin6_addr.s6_addr[1] & 0x0f) <= 2))) {
      std::fprintf(stderr, "A network interface is required for a scoped IPv6 destination\n");
      return VSCP_ERROR_PARAMETER;
    }
  }

  Socket sock = socket(family, SOCK_DGRAM, IPPROTO_UDP);
  if (sock == invalidSocket) {
    logSocketError("socket");
    return VSCP_ERROR_CONNECTION;
  }
  auto fail = [sock](const char *operation, int error) {
    logSocketError(operation);
    closeSocket(sock);
    return error;
  };
  int enabled = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&enabled), sizeof(enabled)) != 0) {
    return fail("setsockopt SO_REUSEADDR", VSCP_ERROR_PARAMETER);
  }

  in_addr ipv4Interface{};
  if (family == AF_INET) {
    if (!ipv4InterfaceAddress(interface, ipv4Interface)) {
      closeSocket(sock);
      return VSCP_ERROR_PARAMETER;
    }
    if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_IF,
                   reinterpret_cast<const char *>(&ipv4Interface), sizeof(ipv4Interface)) != 0) {
      return fail("setsockopt IP_MULTICAST_IF", VSCP_ERROR_PARAMETER);
    }
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST,
                   reinterpret_cast<const char *>(&enabled), sizeof(enabled)) != 0) {
      return fail("setsockopt SO_BROADCAST", VSCP_ERROR_PARAMETER);
    }
    if (multicast) {
      const unsigned char hops = ttl;
      const unsigned char loop = 0;
      if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL,
                     reinterpret_cast<const char *>(&hops), sizeof(hops)) != 0) {
        return fail("setsockopt IP_MULTICAST_TTL", VSCP_ERROR_PARAMETER);
      }
      if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP,
                     reinterpret_cast<const char *>(&loop), sizeof(loop)) != 0) {
        return fail("setsockopt IP_MULTICAST_LOOP", VSCP_ERROR_PARAMETER);
      }
    }
    sockaddr_in local{};
    local.sin_family = AF_INET;
#ifdef __APPLE__
    local.sin_len = sizeof(local);
#endif
    local.sin_addr.s_addr = htonl(INADDR_ANY);
    local.sin_port = htons(port);
    if (bind(sock, reinterpret_cast<const sockaddr *>(&local), sizeof(local)) != 0) {
      return fail("bind IPv4 datagram socket", VSCP_ERROR_CONNECTION);
    }
    if (multicast) {
      ip_mreq membership{};
      membership.imr_multiaddr = reinterpret_cast<sockaddr_in *>(&destination)->sin_addr;
      membership.imr_interface = ipv4Interface;
      if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                     reinterpret_cast<const char *>(&membership), sizeof(membership)) != 0) {
        return fail("setsockopt IP_ADD_MEMBERSHIP", VSCP_ERROR_OPERATION_FAILED);
      }
    }
  }
  else {
    auto *ipv6 = reinterpret_cast<sockaddr_in6 *>(&destination);
    const int onlyV6 = IN6_IS_ADDR_V4MAPPED(&ipv6->sin6_addr) ? 0 : 1;
    if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY,
                   reinterpret_cast<const char *>(&onlyV6), sizeof(onlyV6)) != 0) {
      return fail("setsockopt IPV6_V6ONLY", VSCP_ERROR_PARAMETER);
    }
    if (interface) {
      if (setsockopt(sock, IPPROTO_IPV6, IPV6_MULTICAST_IF,
                     reinterpret_cast<const char *>(&interface), sizeof(interface)) != 0) {
        return fail("setsockopt IPV6_MULTICAST_IF", VSCP_ERROR_PARAMETER);
      }
      if (!isMulticast(destination)) {
#if defined(__APPLE__) && defined(IPV6_BOUND_IF)
        const unsigned int boundInterface = interface;
        if (setsockopt(sock, IPPROTO_IPV6, IPV6_BOUND_IF,
                       reinterpret_cast<const char *>(&boundInterface), sizeof(boundInterface)) != 0) {
          return fail("setsockopt IPV6_BOUND_IF", VSCP_ERROR_PARAMETER);
        }
#elif defined(IPV6_UNICAST_IF)
#ifdef WIN32
        const unsigned int boundInterface = interface;
#else
        const unsigned int boundInterface = htonl(interface);
#endif
        if (setsockopt(sock, IPPROTO_IPV6, IPV6_UNICAST_IF,
                       reinterpret_cast<const char *>(&boundInterface), sizeof(boundInterface)) != 0) {
          return fail("setsockopt IPV6_UNICAST_IF", VSCP_ERROR_PARAMETER);
        }
#else
        std::fprintf(stderr, "Selecting an IPv6 unicast interface is not supported on this platform\n");
        closeSocket(sock);
        return VSCP_ERROR_NOT_SUPPORTED;
#endif
      }
    }
    if (multicast) {
      const int hops = ttl;
      const unsigned int loop = 0;
      if (setsockopt(sock, IPPROTO_IPV6, IPV6_MULTICAST_HOPS,
                     reinterpret_cast<const char *>(&hops), sizeof(hops)) != 0) {
        return fail("setsockopt IPV6_MULTICAST_HOPS", VSCP_ERROR_PARAMETER);
      }
      if (setsockopt(sock, IPPROTO_IPV6, IPV6_MULTICAST_LOOP,
                     reinterpret_cast<const char *>(&loop), sizeof(loop)) != 0) {
        return fail("setsockopt IPV6_MULTICAST_LOOP", VSCP_ERROR_PARAMETER);
      }
    }
    sockaddr_in6 local{};
    local.sin6_family = AF_INET6;
#ifdef __APPLE__
    local.sin6_len = sizeof(local);
#endif
    local.sin6_addr = in6addr_any;
    local.sin6_port = htons(port);
    if (bind(sock, reinterpret_cast<const sockaddr *>(&local), sizeof(local)) != 0) {
      return fail("bind IPv6 datagram socket", VSCP_ERROR_CONNECTION);
    }
    if (multicast) {
      ipv6_mreq membership{};
      membership.ipv6mr_multiaddr = ipv6->sin6_addr;
      membership.ipv6mr_interface = interface;
      if (setsockopt(sock, IPPROTO_IPV6, IPV6_JOIN_GROUP,
                     reinterpret_cast<const char *>(&membership), sizeof(membership)) != 0) {
        return fail("setsockopt IPV6_JOIN_GROUP", VSCP_ERROR_OPERATION_FAILED);
      }
    }
  }
  result = sock;
  return VSCP_ERROR_SUCCESS;
}

struct PthreadMutex {
  pthread_mutex_t *mutex;
  void lock() { pthread_mutex_lock(mutex); }
  void unlock() { pthread_mutex_unlock(mutex); }
};

template<typename Client>
inline void
closeClientSocket(Client &client, bool &socketOpen)
{
  PthreadMutex mutex{&client.m_mutexSocket};
  std::lock_guard<PthreadMutex> lock(mutex);
  if (socketOpen) {
    closeSocket(client.m_sock);
    client.m_sock = 0;
    socketOpen = false;
  }
}

template<typename Client>
inline void
consumeSignal(Client &client, bool semReady)
{
  if (!semReady) {
    return;
  }
#ifdef WIN32
  const DWORD result = WaitForSingleObject(client.m_semReceiveQueue, 0);
  if (result == WAIT_FAILED) {
    std::fprintf(stderr, "WaitForSingleObject failed (error %lu)\n", static_cast<unsigned long>(GetLastError()));
  }
#elif !defined(__APPLE__)
  int result;
  do {
    result = sem_trywait(&client.m_semReceiveQueue);
  } while (result != 0 && errno == EINTR);
  if (result != 0 && errno != EAGAIN) {
    std::fprintf(stderr, "sem_trywait failed: %s\n", std::strerror(errno));
  }
#endif
}

template<typename Client>
inline void
postSignal(Client &client, bool semReady)
{
  if (!semReady) {
    return;
  }
#ifdef WIN32
  if (!ReleaseSemaphore(client.m_semReceiveQueue, 1, nullptr)) {
    std::fprintf(stderr, "ReleaseSemaphore failed (error %lu)\n", static_cast<unsigned long>(GetLastError()));
  }
#elif !defined(__APPLE__)
  if (sem_post(&client.m_semReceiveQueue) != 0) {
    std::fprintf(stderr, "sem_post failed: %s\n", std::strerror(errno));
  }
#endif
}

template<typename Client>
inline int
popEvent(Client &client, std::condition_variable_any &condition, bool semReady,
         vscpEvent *&event, bool blocking, long timeout)
{
  if (blocking && timeout < 0) {
    return VSCP_ERROR_PARAMETER;
  }
  PthreadMutex mutex{&client.m_mutexReceiveQueue};
  std::unique_lock<PthreadMutex> lock(mutex);
  if (blocking) {
    if (!condition.wait_for(lock, std::chrono::milliseconds(timeout),
                            [&client] { return !client.m_receiveQueue.empty(); })) {
      return VSCP_ERROR_TIMEOUT;
    }
  }
  else if (client.m_receiveQueue.empty()) {
    return VSCP_ERROR_FIFO_EMPTY;
  }
  event = client.m_receiveQueue.front();
  client.m_receiveQueue.pop_front();
  consumeSignal(client, semReady);
  return event ? VSCP_ERROR_SUCCESS : VSCP_ERROR_MEMORY;
}

template<typename Client>
inline int
receiveEvent(Client &client, std::condition_variable_any &condition, bool semReady,
             vscpEvent &event, bool blocking, long timeout)
{
  vscpEvent *queued = nullptr;
  const int result = popEvent(client, condition, semReady, queued, blocking, timeout);
  if (result != VSCP_ERROR_SUCCESS) {
    return result;
  }
  const bool copied = vscp_copyEvent(&event, queued);
  vscp_deleteEvent_v2(&queued);
  return copied ? VSCP_ERROR_SUCCESS : VSCP_ERROR_INVALID_FRAME;
}

template<typename Client>
inline int
receiveEvent(Client &client, std::condition_variable_any &condition, bool semReady,
             vscpEventEx &event, bool blocking, long timeout)
{
  vscpEvent *queued = nullptr;
  const int result = popEvent(client, condition, semReady, queued, blocking, timeout);
  if (result != VSCP_ERROR_SUCCESS) {
    return result;
  }
  const bool copied = vscp_convertEventToEventEx(&event, queued);
  vscp_deleteEvent_v2(&queued);
  return copied ? VSCP_ERROR_SUCCESS : VSCP_ERROR_INVALID_FRAME;
}

template<typename Client>
inline int
clearQueue(Client &client, bool semReady)
{
  PthreadMutex mutex{&client.m_mutexReceiveQueue};
  std::lock_guard<PthreadMutex> lock(mutex);
  while (!client.m_receiveQueue.empty()) {
    vscpEvent *event = client.m_receiveQueue.front();
    client.m_receiveQueue.pop_front();
    consumeSignal(client, semReady);
    if (event) {
      vscp_deleteEvent_v2(&event);
    }
  }
  return VSCP_ERROR_SUCCESS;
}

template<typename Client>
inline int
sendEvent(Client &client, const std::atomic<bool> &connected, const sockaddr_storage &destination,
          int destinationLength, vscpEvent &event, uint8_t keyLength, const uint8_t *hmacKey,
          uint8_t configuredEncryption, const uint8_t *configuredKey)
{
  if (!connected.load()) {
    return VSCP_ERROR_NOT_CONNECTED;
  }
  if (event.sizeData > VSCP_LEVEL2_MAXDATA || (event.sizeData && !event.pdata)) {
    return VSCP_ERROR_INVALID_FRAME;
  }
  if (client.m_encryptType > VSCP_ENCRYPTION_AES256) {
    std::fprintf(stderr, "Unsupported datagram encryption algorithm\n");
    return VSCP_ERROR_INVALID_FRAME;
  }
  if (hmacKey && !authenticationStateValid(client.m_encryptType, client.m_key, keyLength, hmacKey,
                                            configuredEncryption, configuredKey)) {
    std::fprintf(stderr, "Invalid authenticated datagram encryption settings\n");
    return VSCP_ERROR_INVALID_FRAME;
  }
  uint8_t frame[bufferSize]{};
  size_t frameLength = vscp_getFrameSizeFromEvent(&event);
  if (frameLength > bufferSize - 32 || !vscp_writeEventToFrame(frame, sizeof(frame), 0, &event)) {
    std::fprintf(stderr, "Error writing datagram event frame\n");
    return VSCP_ERROR_INVALID_FRAME;
  }
  if (hmacKey) {
    try {
      std::vector<uint8_t> plaintext(frame, frame + frameLength);
      std::vector<uint8_t> wire;
      if (!vscp_binary::encode(std::move(plaintext), client.m_encryptType, client.m_key, wire,
                               vscp_binary::authenticatedVersion, hmacKey) || wire.size() > sizeof(frame)) {
        std::fprintf(stderr, "Error authenticating datagram event frame\n");
        return VSCP_ERROR_INVALID_FRAME;
      }
      std::memcpy(frame, wire.data(), wire.size());
      frameLength = wire.size();
    }
    catch (const std::bad_alloc &) {
      std::fprintf(stderr, "Cannot allocate authenticated datagram frame\n");
      return VSCP_ERROR_MEMORY;
    }
  }
  else if (client.m_encryptType != VSCP_ENCRYPTION_NONE) {
    uint8_t encrypted[bufferSize]{};
    const size_t encryptedLength =
      vscp_encryptFrame(encrypted, frame, frameLength, client.m_key, nullptr, client.m_encryptType);
    if (!encryptedLength || encryptedLength > sizeof(frame)) {
      std::fprintf(stderr, "Error encrypting datagram event frame\n");
      return VSCP_ERROR_INVALID_FRAME;
    }
    std::memcpy(frame, encrypted, encryptedLength);
    frameLength = encryptedLength;
  }
  PthreadMutex mutex{&client.m_mutexSocket};
  std::lock_guard<PthreadMutex> lock(mutex);
  if (!connected.load()) {
    return VSCP_ERROR_NOT_CONNECTED;
  }
  const int sent = static_cast<int>(sendto(client.m_sock, reinterpret_cast<const char *>(frame),
                                           static_cast<int>(frameLength), 0,
                                           reinterpret_cast<const sockaddr *>(&destination),
                                           destinationLength));
  if (sent < 0) {
    logSocketError("sendto");
    return VSCP_ERROR_COMMUNICATION;
  }
  if (static_cast<size_t>(sent) != frameLength) {
    std::fprintf(stderr, "sendto sent %d of %zu bytes\n", sent, frameLength);
    return VSCP_ERROR_COMMUNICATION;
  }
  return VSCP_ERROR_SUCCESS;
}

template<typename Client>
inline void
receiveLoop(Client &client, std::atomic<bool> &connected, std::condition_variable_any &condition,
            bool semReady, uint8_t keyLength, const uint8_t *hmacKey,
            uint8_t configuredEncryption, const uint8_t *configuredKey,
            bool requireEncryption = false)
{
  const Socket socket = client.m_sock;
  while (client.m_bRun.load()) {
#ifdef WIN32
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(socket, &readfds);
    timeval timeout{0, 100000};
    const int ready = select(0, &readfds, nullptr, nullptr, &timeout);
#else
    pollfd descriptor{socket, POLLIN, 0};
    const int ready = poll(&descriptor, 1, 100);
#endif
    if (!client.m_bRun.load()) {
      break;
    }
    if (ready < 0) {
#ifndef WIN32
      if (errno == EINTR) {
        continue;
      }
#endif
      logSocketError("datagram receive wait");
      connected.store(false);
      client.m_bRun.store(false);
      break;
    }
    if (!ready) {
      continue;
    }
    uint8_t buffer[bufferSize + 1]{};
    const int received = static_cast<int>(recv(socket, reinterpret_cast<char *>(buffer), sizeof(buffer), 0));
    if (received < 0) {
      logSocketError("recv");
      continue;
    }
    if (!received) {
      continue;
    }
    if (received > static_cast<int>(bufferSize)) {
      std::fprintf(stderr, "Datagram exceeds the receive buffer\n");
      continue;
    }
    size_t frameLength = static_cast<size_t>(received);
    if (hmacKey) {
      size_t innerLength = 0;
      uint8_t version = 0;
      if (!authenticationStateValid(client.m_encryptType, client.m_key, keyLength, hmacKey,
                                    configuredEncryption, configuredKey) ||
          frameLength < vscp_binary::headerSize + 1 ||
          !vscp_binary::parseHeader(buffer, innerLength, version) ||
          version != vscp_binary::authenticatedVersion ||
          innerLength != frameLength - vscp_binary::headerSize ||
          GET_VSCP_BINARY_PACKET_TYPE(buffer[vscp_binary::headerSize]) > VSCP_MULTICAST_TYPE_EVENT1 ||
          (buffer[vscp_binary::headerSize] & 0x0f) != client.m_encryptType) {
        std::fprintf(stderr, "Invalid authenticated datagram header or encryption settings\n");
        continue;
      }
      uint8_t decrypted[bufferSize]{};
      try {
        frameLength = vscp_decryptFrameV1(decrypted, sizeof(decrypted),
                                          buffer + vscp_binary::headerSize, innerLength,
                                          client.m_key, hmacKey);
      }
      catch (const std::bad_alloc &) {
        std::fprintf(stderr, "Cannot allocate authenticated datagram frame\n");
        client.m_bRun.store(false);
        break;
      }
      if (!frameLength) {
        std::fprintf(stderr, "Error authenticating datagram event frame\n");
        continue;
      }
      std::memcpy(buffer, decrypted, frameLength);
    }
    // Legacy (non-HMAC) mode. Security note: unless "require-encryption" is
    // set, plain text frames are accepted even when an AES key is configured
    // and the sender selects the algorithm in the type byte. AES-CBC frames
    // are never authenticated in this mode. Configure "hmac-key" to get
    // authenticated datagrams.
    else if (requireEncryption && ((buffer[0] & 0x0f) != client.m_encryptType)) {
      std::fprintf(stderr, "Datagram dropped: encryption %u does not match required encryption %u\n",
                   static_cast<unsigned>(buffer[0] & 0x0f), static_cast<unsigned>(client.m_encryptType));
      continue;
    }
    else if (buffer[0] & 0x0f) {
      if (frameLength < 1 + 16 + 16) {
        std::fprintf(stderr, "Encrypted datagram is too short\n");
        continue;
      }
      uint8_t decrypted[bufferSize]{};
      if (!vscp_decryptFrame(decrypted, buffer, frameLength, client.m_key, nullptr,
                             VSCP_ENCRYPTION_FROM_TYPE_BYTE)) {
        std::fprintf(stderr, "Error decrypting datagram event frame\n");
        continue;
      }
      frameLength -= 16;
      std::memcpy(buffer, decrypted, frameLength);
    }
    if (frameLength < minimumFrameSize) {
      std::fprintf(stderr, "Datagram event frame is too short\n");
      continue;
    }
    const size_t dataSize = (static_cast<size_t>(buffer[VSCP_BINARY_PACKET_FRAME0_POS_SIZE_MSB]) << 8) |
                             buffer[VSCP_BINARY_PACKET_FRAME0_POS_SIZE_LSB];
    if (dataSize > VSCP_LEVEL2_MAXDATA) {
      std::fprintf(stderr, "Datagram event frame exceeds the maximum data size\n");
      continue;
    }
    if (hmacKey && frameLength != minimumFrameSize + dataSize) {
      std::fprintf(stderr, "Invalid authenticated datagram event length\n");
      continue;
    }
    vscpEvent event{};
    if (!vscp_getEventFromFrame(&event, buffer, frameLength)) {
      std::fprintf(stderr, "Error reading datagram event frame\n");
      vscp_deleteEvent(&event);
      continue;
    }
    vscpEventFilter filter{};
    {
      PthreadMutex mutex{&client.m_mutexReceiveQueue};
      std::lock_guard<PthreadMutex> lock(mutex);
      filter = client.m_filter;
    }
    if (vscp_doLevel2Filter(&event, &filter)) {
      if (client.isCallbackEvActive()) {
        client.m_callbackev(event, client.getCallbackObj());
      }
      if (client.isCallbackExActive()) {
        vscpEventEx extended{};
        if (vscp_convertEventToEventEx(&extended, &event)) {
          client.m_callbackex(extended, client.getCallbackObj());
        }
      }
      // Queue unless both callbacks consume the event (the queue is capped)
      if (!client.isCallbackEvActive() || !client.isCallbackExActive()) {
        vscpEvent *queued = new (std::nothrow) vscpEvent(event);
        if (!queued) {
          std::fprintf(stderr, "Cannot allocate datagram receive event\n");
        }
        else {
          try {
            PthreadMutex mutex{&client.m_mutexReceiveQueue};
            std::lock_guard<PthreadMutex> lock(mutex);
            if (client.m_receiveQueue.size() >= maxQueuedEvents) {
              std::fprintf(stderr, "Datagram receive queue full (%zu events). Event dropped: class=%u type=%u\n",
                           maxQueuedEvents, static_cast<unsigned>(event.vscp_class),
                           static_cast<unsigned>(event.vscp_type));
              delete queued; // Shares pdata with event, which is freed below
            }
            else {
              client.m_receiveQueue.push_back(queued);
              event.pdata = nullptr;
              event.sizeData = 0;
              postSignal(client, semReady);
              condition.notify_one();
            }
          }
          catch (const std::bad_alloc &) {
            std::fprintf(stderr, "Cannot enqueue datagram receive event\n");
            delete queued;
          }
        }
      }
    }
    vscp_deleteEvent(&event);
  }
  connected.store(false);
}

} // namespace vscp_datagram

#endif
