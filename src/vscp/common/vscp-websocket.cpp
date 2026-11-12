// vscp-websocket.cpp
//
// Minimal websocket client (RFC 6455) used by the VSCP ws1 and ws2 clients.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// Copyright:   (C) 2007-2026
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

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <deque>
#include <limits>

#include <vscp-aes.h>
#include <vscphelper.h>

#include "vscp-websocket.h"

using json = nlohmann::json;

namespace {

#ifdef WIN32
typedef SOCKET Socket;
const Socket invalidSocket = INVALID_SOCKET;
#else
typedef int Socket;
const Socket invalidSocket = -1;
#endif

#if defined(MSG_NOSIGNAL)
const int sendFlags = MSG_NOSIGNAL;
#else
const int sendFlags = 0;
#endif

typedef std::chrono::steady_clock Clock;

// Websocket opcodes
const uint8_t OP_CONTINUATION = 0x0;
const uint8_t OP_TEXT         = 0x1;
const uint8_t OP_BINARY       = 0x2;
const uint8_t OP_CLOSE        = 0x8;
const uint8_t OP_PING         = 0x9;
const uint8_t OP_PONG         = 0xA;

const char *const WS_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

int
lastSocketError(void)
{
#ifdef WIN32
  return WSAGetLastError();
#else
  return errno;
#endif
}

bool
wouldBlock(int error)
{
#ifdef WIN32
  return (WSAEWOULDBLOCK == error) || (WSAEINPROGRESS == error);
#else
  return (EAGAIN == error) || (EWOULDBLOCK == error) || (EINPROGRESS == error) || (EINTR == error);
#endif
}

void
closeSocket(Socket s)
{
  if (invalidSocket == s) {
    return;
  }
#ifdef WIN32
  closesocket(s);
#else
  ::close(s);
#endif
}

bool
setNonBlocking(Socket s)
{
#ifdef WIN32
  u_long enabled = 1;
  return (0 == ioctlsocket(s, FIONBIO, &enabled));
#else
  int flags = fcntl(s, F_GETFL, 0);
  return (flags >= 0) && (0 == fcntl(s, F_SETFL, flags | O_NONBLOCK));
#endif
}

// Milliseconds left until deadline (never negative)
int
remainingMs(const Clock::time_point &deadline)
{
  auto left = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - Clock::now()).count();
  if (left <= 0) {
    return 0;
  }
  if (left > (std::numeric_limits<int>::max)()) {
    return (std::numeric_limits<int>::max)();
  }
  return static_cast<int>(left);
}

// Wait for a socket to be readable or writable.
// Returns 1 when ready, 0 on timeout and -1 on error.
int
waitSocket(Socket s, bool bWrite, int timeout)
{
#ifdef WIN32
  WSAPOLLFD pfd;
  pfd.fd      = s;
  pfd.events  = bWrite ? POLLWRNORM : POLLRDNORM;
  pfd.revents = 0;
  int rv      = WSAPoll(&pfd, 1, timeout);
#else
  struct pollfd pfd;
  pfd.fd      = s;
  pfd.events  = bWrite ? POLLOUT : POLLIN;
  pfd.revents = 0;
  int rv;
  do {
    rv = poll(&pfd, 1, timeout);
  } while ((rv < 0) && (EINTR == errno));
#endif
  if (rv < 0) {
    return -1;
  }
  if (0 == rv) {
    return 0;
  }
  // POLLHUP/POLLERR also count as ready so the next read or write reports
  // the actual condition
  return 1;
}

std::string
toLower(std::string str)
{
  std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return (char) tolower(c); });
  return str;
}

std::string
trim(const std::string &str)
{
  const char *ws = " \t\r\n";
  size_t start   = str.find_first_not_of(ws);
  if (std::string::npos == start) {
    return "";
  }
  size_t end = str.find_last_not_of(ws);
  return str.substr(start, end - start + 1);
}

std::string
base64(const unsigned char *data, size_t len)
{
  std::vector<unsigned char> out(4 * ((len + 2) / 3) + 1, 0);
  int n = EVP_EncodeBlock(out.data(), data, static_cast<int>(len));
  return std::string(reinterpret_cast<char *>(out.data()), (n > 0) ? (size_t) n : 0);
}

std::string
opensslError(void)
{
  std::string str;
  unsigned long err;
  while (0 != (err = ERR_get_error())) {
    char buf[256];
    ERR_error_string_n(err, buf, sizeof(buf));
    if (!str.empty()) {
      str += "; ";
    }
    str += buf;
  }
  return str;
}

int
tlsPassword(char *buffer, int size, int /*rwflag*/, void *arg)
{
  const std::string &password = *static_cast<const std::string *>(arg);
  if ((size <= 0) || (password.size() > static_cast<size_t>(size))) {
    return 0;
  }
  return static_cast<int>(password.copy(buffer, static_cast<size_t>(size)));
}

bool
isHexKey(const std::string &key)
{
  return (32 == key.size()) && (std::string::npos == key.find_first_not_of("0123456789abcdefABCDEF"));
}

} // namespace

///////////////////////////////////////////////////////////////////////////////
// Impl
//

struct CVscpWebSocket::Impl {
  Socket sock           = invalidSocket;
  SSL_CTX *ctx          = nullptr;
  SSL *ssl              = nullptr;
  BIO_METHOD *bioMethod = nullptr;
  std::string pwkey;     // Private key password, must outlive the SSL_CTX
  std::mutex sslMutex;   // SSL_read/SSL_write
  std::mutex writeMutex; // Keep frames whole
  std::mutex errMutex;
  std::string lastError;

  // Receive side, only used by the thread calling receive()
  std::string rx;          // Bytes not parsed yet
  std::string fragments;   // Message being assembled
  uint8_t fragOpcode = 0;  // Opcode of message being assembled (0 = none)
  bool bCloseSent    = false;
#ifdef WIN32
  bool bWsaStarted = false;
#endif

  ~Impl() { reset(); }

  void setError(const std::string &str)
  {
    std::lock_guard<std::mutex> lock(errMutex);
    lastError = str;
  }

  void reset(void)
  {
    if (nullptr != ssl) {
      SSL_free(ssl);
      ssl = nullptr;
    }
    if (nullptr != ctx) {
      SSL_CTX_free(ctx);
      ctx = nullptr;
    }
    if (nullptr != bioMethod) {
      BIO_meth_free(bioMethod);
      bioMethod = nullptr;
    }
    closeSocket(sock);
    sock = invalidSocket;
    rx.clear();
    fragments.clear();
    fragOpcode = 0;
    bCloseSent = false;
  }

  // OpenSSL BIO that uses send/recv so a closed connection never raises
  // SIGPIPE.
  static int bioWrite(BIO *bio, const char *data, int len)
  {
    Impl *pimpl = static_cast<Impl *>(BIO_get_data(bio));
    BIO_clear_retry_flags(bio);
    int n = (int) ::send(pimpl->sock, data, len, sendFlags);
    if ((n < 0) && wouldBlock(lastSocketError())) {
      BIO_set_retry_write(bio);
    }
    return n;
  }

  static int bioRead(BIO *bio, char *data, int len)
  {
    Impl *pimpl = static_cast<Impl *>(BIO_get_data(bio));
    BIO_clear_retry_flags(bio);
    int n = (int) ::recv(pimpl->sock, data, len, 0);
    if ((n < 0) && wouldBlock(lastSocketError())) {
      BIO_set_retry_read(bio);
    }
    return n;
  }

  static long bioCtrl(BIO * /*bio*/, int cmd, long /*num*/, void * /*ptr*/)
  {
    return (BIO_CTRL_FLUSH == cmd) ? 1 : 0;
  }

  int tcpConnect(const vscpWsUrl &url, const Clock::time_point &deadline);
  int tlsConnect(const vscpWsUrl &url, const vscpWsTlsOptions &tls, const Clock::time_point &deadline);
  int readSome(int timeout);
  int writeAll(const char *data, size_t len, const Clock::time_point &deadline);
  int parseFrame(uint8_t &opcode, bool &bFin, std::string &payload);
};

///////////////////////////////////////////////////////////////////////////////
// tcpConnect
//

int
CVscpWebSocket::Impl::tcpConnect(const vscpWsUrl &url, const Clock::time_point &deadline)
{
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  struct addrinfo *result = nullptr;
  int rv                  = getaddrinfo(url.host.c_str(), url.port.c_str(), &hints, &result);
  if ((0 != rv) || (nullptr == result)) {
    setError("Unable to resolve host " + url.host);
    return VSCP_ERROR_CONNECTION;
  }

  int status = VSCP_ERROR_CONNECTION;
  setError("Unable to connect to " + url.host + ":" + url.port);

  for (struct addrinfo *ai = result; nullptr != ai; ai = ai->ai_next) {

    if (0 == remainingMs(deadline)) {
      status = VSCP_ERROR_TIMEOUT;
      setError("Timeout connecting to " + url.host + ":" + url.port);
      break;
    }

    Socket s = ::socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    if (invalidSocket == s) {
      continue;
    }

    if (!setNonBlocking(s)) {
      closeSocket(s);
      continue;
    }

#if defined(SO_NOSIGPIPE)
    int noSigpipe = 1;
    setsockopt(s, SOL_SOCKET, SO_NOSIGPIPE, &noSigpipe, sizeof(noSigpipe));
#endif

    rv = ::connect(s, ai->ai_addr, (int) ai->ai_addrlen);
    if ((0 != rv) && !wouldBlock(lastSocketError())) {
      closeSocket(s);
      continue;
    }

    if (0 != rv) {
      int ready = waitSocket(s, true, remainingMs(deadline));
      if (ready <= 0) {
        closeSocket(s);
        if (0 == ready) {
          status = VSCP_ERROR_TIMEOUT;
          setError("Timeout connecting to " + url.host + ":" + url.port);
        }
        continue;
      }
      int error = 0;
#ifdef WIN32
      int size = sizeof(error);
      getsockopt(s, SOL_SOCKET, SO_ERROR, reinterpret_cast<char *>(&error), &size);
#else
      socklen_t size = sizeof(error);
      getsockopt(s, SOL_SOCKET, SO_ERROR, &error, &size);
#endif
      if (0 != error) {
        closeSocket(s);
        continue;
      }
    }

    // Small messages, send them at once
    int noDelay = 1;
    setsockopt(s, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char *>(&noDelay), sizeof(noDelay));

    sock   = s;
    status = VSCP_ERROR_SUCCESS;
    break;
  }

  freeaddrinfo(result);
  return status;
}

///////////////////////////////////////////////////////////////////////////////
// tlsConnect
//

int
CVscpWebSocket::Impl::tlsConnect(const vscpWsUrl &url, const vscpWsTlsOptions &tls, const Clock::time_point &deadline)
{
  ERR_clear_error();

  ctx = SSL_CTX_new(TLS_client_method());
  if (nullptr == ctx) {
    setError("Unable to create TLS context: " + opensslError());
    return VSCP_ERROR_CONNECTION;
  }
  SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
  SSL_CTX_set_mode(ctx, SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
  SSL_CTX_set_verify(ctx, tls.bVerifyPeer ? SSL_VERIFY_PEER : SSL_VERIFY_NONE, nullptr);

  if (tls.bVerifyPeer) {
    if (!tls.cafile.empty() || !tls.capath.empty()) {
      if (1 != SSL_CTX_load_verify_locations(ctx,
                                             tls.cafile.empty() ? nullptr : tls.cafile.c_str(),
                                             tls.capath.empty() ? nullptr : tls.capath.c_str())) {
        setError("Unable to load TLS CA certificates: " + opensslError());
        return VSCP_ERROR_CONNECTION;
      }
    }
    else if (1 != SSL_CTX_set_default_verify_paths(ctx)) {
      setError("Unable to load the default TLS CA certificates: " + opensslError());
      return VSCP_ERROR_CONNECTION;
    }
  }

  if (!tls.certfile.empty() && (1 != SSL_CTX_use_certificate_chain_file(ctx, tls.certfile.c_str()))) {
    setError("Unable to load TLS client certificate: " + opensslError());
    return VSCP_ERROR_CONNECTION;
  }

  if (!tls.keyfile.empty() || !tls.certfile.empty()) {
    pwkey = tls.pwkeyfile;
    SSL_CTX_set_default_passwd_cb(ctx, tlsPassword);
    SSL_CTX_set_default_passwd_cb_userdata(ctx, &pwkey);
    const std::string &keyfile = tls.keyfile.empty() ? tls.certfile : tls.keyfile;
    if ((1 != SSL_CTX_use_PrivateKey_file(ctx, keyfile.c_str(), SSL_FILETYPE_PEM)) ||
        (!tls.certfile.empty() && (1 != SSL_CTX_check_private_key(ctx)))) {
      setError("Unable to load TLS client private key: " + opensslError());
      return VSCP_ERROR_CONNECTION;
    }
  }

  ssl = SSL_new(ctx);
  if (nullptr == ssl) {
    setError("Unable to create TLS connection: " + opensslError());
    return VSCP_ERROR_CONNECTION;
  }

  // An IP address is checked against the IP addresses of the certificate,
  // a name against its DNS names. SNI is only sent for names (RFC 6066).
  unsigned char address[sizeof(struct in6_addr)];
  const bool bIpAddress = (1 == inet_pton(AF_INET, url.host.c_str(), address)) ||
                          (1 == inet_pton(AF_INET6, url.host.c_str(), address));
  if (tls.bVerifyPeer) {
    int ok = bIpAddress ? X509_VERIFY_PARAM_set1_ip_asc(SSL_get0_param(ssl), url.host.c_str())
                        : SSL_set1_host(ssl, url.host.c_str());
    if (1 != ok) {
      setError("Unable to set TLS host name: " + opensslError());
      return VSCP_ERROR_CONNECTION;
    }
  }
  if (!bIpAddress && (1 != SSL_set_tlsext_host_name(ssl, url.host.c_str()))) {
    setError("Unable to set TLS server name: " + opensslError());
    return VSCP_ERROR_CONNECTION;
  }

  bioMethod = BIO_meth_new(BIO_get_new_index() | BIO_TYPE_SOURCE_SINK, "vscp websocket");
  if (nullptr == bioMethod) {
    setError("Unable to create TLS transport");
    return VSCP_ERROR_CONNECTION;
  }
  BIO_meth_set_write(bioMethod, bioWrite);
  BIO_meth_set_read(bioMethod, bioRead);
  BIO_meth_set_ctrl(bioMethod, bioCtrl);

  BIO *bio = BIO_new(bioMethod);
  if (nullptr == bio) {
    setError("Unable to create TLS transport");
    return VSCP_ERROR_CONNECTION;
  }
  BIO_set_data(bio, this);
  BIO_set_init(bio, 1);
  SSL_set_bio(ssl, bio, bio);

  while (true) {
    int rv = SSL_connect(ssl);
    if (1 == rv) {
      break;
    }
    int err = SSL_get_error(ssl, rv);
    if ((SSL_ERROR_WANT_READ != err) && (SSL_ERROR_WANT_WRITE != err)) {
      std::string str = "TLS handshake failed";
      long verify     = SSL_get_verify_result(ssl);
      if (X509_V_OK != verify) {
        str += ": ";
        str += X509_verify_cert_error_string(verify);
      }
      else {
        std::string ossl = opensslError();
        if (!ossl.empty()) {
          str += ": " + ossl;
        }
      }
      setError(str);
      return VSCP_ERROR_CONNECTION;
    }
    int ready = waitSocket(sock, (SSL_ERROR_WANT_WRITE == err), remainingMs(deadline));
    if (0 == ready) {
      setError("Timeout during TLS handshake");
      return VSCP_ERROR_TIMEOUT;
    }
    if (ready < 0) {
      setError("Socket error during TLS handshake");
      return VSCP_ERROR_CONNECTION;
    }
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// readSome
//
// Append received bytes to rx. Returns the number of bytes read, 0 on
// timeout and -1 when the connection is closed or broken.
//

int
CVscpWebSocket::Impl::readSome(int timeout)
{
  char buf[16384];
  const Clock::time_point deadline = Clock::now() + std::chrono::milliseconds(timeout);

  while (true) {

    bool bWantWrite = false;

    if (nullptr != ssl) {
      int n, err;
      {
        std::lock_guard<std::mutex> lock(sslMutex);
        ERR_clear_error();
        n   = SSL_read(ssl, buf, sizeof(buf));
        err = (n > 0) ? SSL_ERROR_NONE : SSL_get_error(ssl, n);
      }
      if (n > 0) {
        rx.append(buf, (size_t) n);
        return n;
      }
      if (SSL_ERROR_WANT_WRITE == err) {
        bWantWrite = true;
      }
      else if (SSL_ERROR_WANT_READ != err) {
        if (SSL_ERROR_ZERO_RETURN == err) {
          setError("Connection closed by server");
        }
        else {
          setError("TLS read failed " + opensslError());
        }
        return -1;
      }
    }
    else {
      int n = (int) ::recv(sock, buf, sizeof(buf), 0);
      if (n > 0) {
        rx.append(buf, (size_t) n);
        return n;
      }
      if (0 == n) {
        setError("Connection closed by server");
        return -1;
      }
      if (!wouldBlock(lastSocketError())) {
        setError("Socket read failed");
        return -1;
      }
    }

    int ready = waitSocket(sock, bWantWrite, remainingMs(deadline));
    if (0 == ready) {
      return 0;
    }
    if (ready < 0) {
      setError("Socket error");
      return -1;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// writeAll
//

int
CVscpWebSocket::Impl::writeAll(const char *data, size_t len, const Clock::time_point &deadline)
{
  size_t written = 0;

  while (written < len) {

    size_t chunk = (std::min)(len - written, (size_t) (std::numeric_limits<int>::max)());
    bool bWantRead = false;

    if (nullptr != ssl) {
      int n, err;
      {
        std::lock_guard<std::mutex> lock(sslMutex);
        ERR_clear_error();
        n   = SSL_write(ssl, data + written, (int) chunk);
        err = (n > 0) ? SSL_ERROR_NONE : SSL_get_error(ssl, n);
      }
      if (n > 0) {
        written += (size_t) n;
        continue;
      }
      if (SSL_ERROR_WANT_READ == err) {
        bWantRead = true;
      }
      else if (SSL_ERROR_WANT_WRITE != err) {
        setError("TLS write failed " + opensslError());
        return VSCP_ERROR_CONNECTION;
      }
    }
    else {
      int n = (int) ::send(sock, data + written, (int) chunk, sendFlags);
      if (n > 0) {
        written += (size_t) n;
        continue;
      }
      if ((n < 0) && !wouldBlock(lastSocketError())) {
        setError("Socket write failed");
        return VSCP_ERROR_CONNECTION;
      }
    }

    int ready = waitSocket(sock, !bWantRead, remainingMs(deadline));
    if (0 == ready) {
      setError("Timeout sending data");
      return VSCP_ERROR_TIMEOUT;
    }
    if (ready < 0) {
      setError("Socket error");
      return VSCP_ERROR_CONNECTION;
    }
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// parseFrame
//
// Take one complete frame from rx. Returns 1 if a frame was taken, 0 if more
// data is needed and -1 on a protocol error.
//

int
CVscpWebSocket::Impl::parseFrame(uint8_t &opcode, bool &bFin, std::string &payload)
{
  if (rx.size() < 2) {
    return 0;
  }

  const uint8_t *p = reinterpret_cast<const uint8_t *>(rx.data());

  // No extensions are negotiated so the reserved bits must be zero
  if (p[0] & 0x70) {
    setError("Invalid websocket frame (reserved bits set)");
    return -1;
  }

  bFin          = (0 != (p[0] & 0x80));
  opcode        = p[0] & 0x0f;
  bool bMasked  = (0 != (p[1] & 0x80));
  uint64_t plen = p[1] & 0x7f;
  size_t pos    = 2;

  if (126 == plen) {
    if (rx.size() < 4) {
      return 0;
    }
    plen = ((uint64_t) p[2] << 8) | p[3];
    pos  = 4;
  }
  else if (127 == plen) {
    if (rx.size() < 10) {
      return 0;
    }
    plen = 0;
    for (int i = 0; i < 8; i++) {
      plen = (plen << 8) | p[2 + i];
    }
    pos = 10;
  }

  if (plen > MAX_MESSAGE_SIZE) {
    setError("Websocket message too large");
    return -1;
  }

  // Control frames are short and never fragmented
  if ((opcode & 0x08) && ((plen > 125) || !bFin)) {
    setError("Invalid websocket control frame");
    return -1;
  }

  uint8_t mask[4] = { 0, 0, 0, 0 };
  if (bMasked) {
    if (rx.size() < pos + 4) {
      return 0;
    }
    memcpy(mask, p + pos, 4);
    pos += 4;
  }

  if (rx.size() < pos + plen) {
    return 0;
  }

  payload.assign(rx, pos, (size_t) plen);
  if (bMasked) {
    for (size_t i = 0; i < payload.size(); i++) {
      payload[i] = (char) (payload[i] ^ mask[i % 4]);
    }
  }

  rx.erase(0, pos + (size_t) plen);
  return 1;
}

///////////////////////////////////////////////////////////////////////////////
// CTOR
//

CVscpWebSocket::CVscpWebSocket()
  : m_pimpl(new Impl)
  , m_bOpen(false)
{
}

///////////////////////////////////////////////////////////////////////////////
// DTOR
//

CVscpWebSocket::~CVscpWebSocket()
{
  close();
#ifdef WIN32
  if (m_pimpl->bWsaStarted) {
    WSACleanup();
  }
#endif
  delete m_pimpl;
}

///////////////////////////////////////////////////////////////////////////////
// parseUrl
//

bool
CVscpWebSocket::parseUrl(const std::string &url, const std::string &defaultPath, vscpWsUrl &parts)
{
  std::string str = trim(url);
  std::string scheme("ws");

  size_t pos = str.find("://");
  if (std::string::npos != pos) {
    scheme = toLower(str.substr(0, pos));
    str    = str.substr(pos + 3);
  }

  if (("ws" == scheme) || ("http" == scheme)) {
    parts.bTLS = false;
    parts.port = "80";
  }
  else if (("wss" == scheme) || ("https" == scheme)) {
    parts.bTLS = true;
    parts.port = "443";
  }
  else {
    return false;
  }

  // Split host[:port] and path
  pos                  = str.find_first_of("/?#");
  std::string hostport = (std::string::npos == pos) ? str : str.substr(0, pos);
  std::string path     = (std::string::npos == pos) ? "" : str.substr(pos);
  if (!path.empty() && ('/' != path[0])) {
    path = "/" + path;
  }

  std::string port;
  if (!hostport.empty() && ('[' == hostport[0])) {
    size_t end = hostport.find(']');
    if (std::string::npos == end) {
      return false;
    }
    parts.host       = hostport.substr(1, end - 1);
    std::string rest = hostport.substr(end + 1);
    if (!rest.empty()) {
      if (':' != rest[0]) {
        return false;
      }
      port = rest.substr(1);
    }
  }
  else {
    size_t colon = hostport.rfind(':');
    if ((std::string::npos != colon) && (hostport.find(':') == colon)) {
      parts.host = hostport.substr(0, colon);
      port       = hostport.substr(colon + 1);
    }
    else {
      // No port, or a bare IPv6 address
      parts.host = hostport;
    }
  }

  if (parts.host.empty()) {
    return false;
  }

  if (!port.empty()) {
    if (std::string::npos != port.find_first_not_of("0123456789") || (port.size() > 5)) {
      return false;
    }
    unsigned long nport = strtoul(port.c_str(), nullptr, 10);
    if ((0 == nport) || (nport > 65535)) {
      return false;
    }
    parts.port = port;
  }

  parts.path = (path.empty() || ("/" == path)) ? defaultPath : path;
  if (parts.path.empty()) {
    parts.path = "/";
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// acceptKey
//

std::string
CVscpWebSocket::acceptKey(const std::string &key)
{
  std::string str = key + WS_GUID;
  unsigned char md[EVP_MAX_MD_SIZE];
  unsigned int mdlen = 0;
  if (1 != EVP_Digest(str.data(), str.size(), md, &mdlen, EVP_sha1(), nullptr)) {
    return "";
  }
  return base64(md, mdlen);
}

///////////////////////////////////////////////////////////////////////////////
// buildFrame
//

std::string
CVscpWebSocket::buildFrame(uint8_t opcode, const std::string &payload, const uint8_t mask[4])
{
  std::string frame;
  frame.reserve(payload.size() + 14);

  frame.push_back((char) (0x80 | (opcode & 0x0f)));

  const uint64_t len = payload.size();
  if (len < 126) {
    frame.push_back((char) (0x80 | len));
  }
  else if (len <= 0xffff) {
    frame.push_back((char) (0x80 | 126));
    frame.push_back((char) ((len >> 8) & 0xff));
    frame.push_back((char) (len & 0xff));
  }
  else {
    frame.push_back((char) (0x80 | 127));
    for (int i = 7; i >= 0; i--) {
      frame.push_back((char) ((len >> (8 * i)) & 0xff));
    }
  }

  frame.append(reinterpret_cast<const char *>(mask), 4);
  for (size_t i = 0; i < payload.size(); i++) {
    frame.push_back((char) (payload[i] ^ mask[i % 4]));
  }

  return frame;
}

///////////////////////////////////////////////////////////////////////////////
// connect
//

int
CVscpWebSocket::connect(const vscpWsUrl &url, uint32_t timeout, const vscpWsTlsOptions &tls)
{
  int rv;

  close();

#ifdef WIN32
  if (!m_pimpl->bWsaStarted) {
    WSADATA wsaData;
    if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData)) {
      m_pimpl->setError("Unable to initialize Winsock");
      return VSCP_ERROR_CONNECTION;
    }
    m_pimpl->bWsaStarted = true;
  }
#endif

  const Clock::time_point deadline = Clock::now() + std::chrono::milliseconds(timeout);

  if (VSCP_ERROR_SUCCESS != (rv = m_pimpl->tcpConnect(url, deadline))) {
    m_pimpl->reset();
    return rv;
  }

  if (url.bTLS && (VSCP_ERROR_SUCCESS != (rv = m_pimpl->tlsConnect(url, tls, deadline)))) {
    m_pimpl->reset();
    return rv;
  }

  // Websocket upgrade
  unsigned char nonce[16];
  if (1 != RAND_bytes(nonce, sizeof(nonce))) {
    m_pimpl->setError("Unable to create websocket key");
    m_pimpl->reset();
    return VSCP_ERROR_CONNECTION;
  }
  const std::string key = base64(nonce, sizeof(nonce));

  std::string host = (std::string::npos != url.host.find(':')) ? ("[" + url.host + "]") : url.host;
  std::string request;
  request = "GET " + url.path + " HTTP/1.1\r\n";
  request += "Host: " + host + ":" + url.port + "\r\n";
  request += "Upgrade: websocket\r\n";
  request += "Connection: Upgrade\r\n";
  request += "Sec-WebSocket-Key: " + key + "\r\n";
  request += "Sec-WebSocket-Version: 13\r\n";
  request += "\r\n";

  if (VSCP_ERROR_SUCCESS != (rv = m_pimpl->writeAll(request.data(), request.size(), deadline))) {
    m_pimpl->reset();
    return rv;
  }

  size_t end;
  while (std::string::npos == (end = m_pimpl->rx.find("\r\n\r\n"))) {
    if (m_pimpl->rx.size() > 16384) {
      m_pimpl->setError("Invalid websocket handshake response");
      m_pimpl->reset();
      return VSCP_ERROR_CONNECTION;
    }
    int n = m_pimpl->readSome(remainingMs(deadline));
    if (0 == n) {
      m_pimpl->setError("Timeout waiting for the websocket handshake response");
      m_pimpl->reset();
      return VSCP_ERROR_TIMEOUT;
    }
    if (n < 0) {
      m_pimpl->reset();
      return VSCP_ERROR_CONNECTION;
    }
  }

  std::string header = m_pimpl->rx.substr(0, end);
  m_pimpl->rx.erase(0, end + 4); // Anything after the header is frame data

  std::deque<std::string> lines;
  vscp_split(lines, header, "\r\n");
  if (lines.empty()) {
    m_pimpl->setError("Empty websocket handshake response");
    m_pimpl->reset();
    return VSCP_ERROR_CONNECTION;
  }

  // Status line "HTTP/1.1 101 Switching Protocols"
  std::string status = trim(lines.front());
  lines.pop_front();
  size_t sp = status.find(' ');
  if ((std::string::npos == sp) || (0 != status.compare(sp + 1, 3, "101"))) {
    m_pimpl->setError("Websocket upgrade refused by server: " + status);
    m_pimpl->reset();
    return VSCP_ERROR_CONNECTION;
  }

  std::string upgrade, accept;
  for (const auto &line : lines) {
    size_t colon = line.find(':');
    if (std::string::npos == colon) {
      continue;
    }
    std::string name  = toLower(trim(line.substr(0, colon)));
    std::string value = trim(line.substr(colon + 1));
    if ("upgrade" == name) {
      upgrade = toLower(value);
    }
    else if ("sec-websocket-accept" == name) {
      accept = value;
    }
  }

  if (("websocket" != upgrade) || (accept != acceptKey(key))) {
    m_pimpl->setError("Invalid websocket handshake response from server");
    m_pimpl->reset();
    return VSCP_ERROR_CONNECTION;
  }

  m_bOpen = true;
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// close
//

void
CVscpWebSocket::close(void)
{
  if (m_bOpen && !m_pimpl->bCloseSent) {
    // Normal closure (1000)
    uint8_t mask[4] = { 0, 0, 0, 0 };
    RAND_bytes(mask, sizeof(mask));
    std::string frame = buildFrame(OP_CLOSE, std::string("\x03\xe8", 2), mask);
    std::lock_guard<std::mutex> lock(m_pimpl->writeMutex);
    m_pimpl->writeAll(frame.data(), frame.size(), Clock::now() + std::chrono::milliseconds(500));
    m_pimpl->bCloseSent = true;
  }

  if (nullptr != m_pimpl->ssl) {
    std::lock_guard<std::mutex> lock(m_pimpl->sslMutex);
    SSL_shutdown(m_pimpl->ssl);
  }

  m_bOpen = false;
  m_pimpl->reset();
}

///////////////////////////////////////////////////////////////////////////////
// sendText
//

int
CVscpWebSocket::sendText(const std::string &msg, uint32_t timeout)
{
  if (!m_bOpen) {
    return VSCP_ERROR_NOT_CONNECTED;
  }

  uint8_t mask[4];
  if (1 != RAND_bytes(mask, sizeof(mask))) {
    return VSCP_ERROR_ERROR;
  }

  std::string frame = buildFrame(OP_TEXT, msg, mask);
  std::lock_guard<std::mutex> lock(m_pimpl->writeMutex);
  int rv = m_pimpl->writeAll(frame.data(), frame.size(), Clock::now() + std::chrono::milliseconds(timeout));
  if (VSCP_ERROR_CONNECTION == rv) {
    m_bOpen = false;
  }
  return rv;
}

///////////////////////////////////////////////////////////////////////////////
// receive
//

int
CVscpWebSocket::receive(std::string &msg, bool &bText, uint32_t timeout)
{
  const Clock::time_point deadline = Clock::now() + std::chrono::milliseconds(timeout);

  while (true) {

    if (!m_bOpen) {
      return VSCP_ERROR_CONNECTION;
    }

    uint8_t opcode;
    bool bFin;
    std::string payload;
    int rv;

    while (1 == (rv = m_pimpl->parseFrame(opcode, bFin, payload))) {

      switch (opcode) {

        case OP_CONTINUATION:
          if (0 == m_pimpl->fragOpcode) {
            m_pimpl->setError("Unexpected websocket continuation frame");
            rv = -1;
            break;
          }
          if (m_pimpl->fragments.size() + payload.size() > MAX_MESSAGE_SIZE) {
            m_pimpl->setError("Websocket message too large");
            rv = -1;
            break;
          }
          m_pimpl->fragments += payload;
          if (bFin) {
            bText = (OP_TEXT == m_pimpl->fragOpcode);
            msg.swap(m_pimpl->fragments);
            m_pimpl->fragments.clear();
            m_pimpl->fragOpcode = 0;
            return VSCP_ERROR_SUCCESS;
          }
          break;

        case OP_TEXT:
        case OP_BINARY:
          if (0 != m_pimpl->fragOpcode) {
            m_pimpl->setError("Websocket message started inside another message");
            rv = -1;
            break;
          }
          if (bFin) {
            bText = (OP_TEXT == opcode);
            msg.swap(payload);
            return VSCP_ERROR_SUCCESS;
          }
          m_pimpl->fragOpcode = opcode;
          m_pimpl->fragments.swap(payload);
          break;

        case OP_PING: {
          uint8_t mask[4] = { 0, 0, 0, 0 };
          RAND_bytes(mask, sizeof(mask));
          std::string frame = buildFrame(OP_PONG, payload, mask);
          std::lock_guard<std::mutex> lock(m_pimpl->writeMutex);
          m_pimpl->writeAll(frame.data(), frame.size(), Clock::now() + std::chrono::milliseconds(1000));
        } break;

        case OP_PONG:
          break;

        case OP_CLOSE: {
          // Echo the status code and close
          std::lock_guard<std::mutex> lock(m_pimpl->writeMutex);
          if (!m_pimpl->bCloseSent) {
            uint8_t mask[4] = { 0, 0, 0, 0 };
            RAND_bytes(mask, sizeof(mask));
            std::string frame = buildFrame(OP_CLOSE, payload.substr(0, 2), mask);
            m_pimpl->writeAll(frame.data(), frame.size(), Clock::now() + std::chrono::milliseconds(500));
            m_pimpl->bCloseSent = true;
          }
          m_pimpl->setError("Connection closed by server");
          m_bOpen = false;
          return VSCP_ERROR_CONNECTION;
        }

        default:
          m_pimpl->setError("Unknown websocket opcode");
          rv = -1;
          break;
      }

      if (rv < 0) {
        break;
      }
    }

    if (rv < 0) {
      // Protocol error (1002)
      uint8_t mask[4] = { 0, 0, 0, 0 };
      RAND_bytes(mask, sizeof(mask));
      std::string frame = buildFrame(OP_CLOSE, std::string("\x03\xea", 2), mask);
      std::lock_guard<std::mutex> lock(m_pimpl->writeMutex);
      if (!m_pimpl->bCloseSent) {
        m_pimpl->writeAll(frame.data(), frame.size(), Clock::now() + std::chrono::milliseconds(500));
        m_pimpl->bCloseSent = true;
      }
      m_bOpen = false;
      return VSCP_ERROR_CONNECTION;
    }

    int left = remainingMs(deadline);
    if ((0 == left) && (0 != timeout)) {
      return VSCP_ERROR_TIMEOUT;
    }

    int n = m_pimpl->readSome(left);
    if (0 == n) {
      return VSCP_ERROR_TIMEOUT;
    }
    if (n < 0) {
      m_bOpen = false;
      return VSCP_ERROR_CONNECTION;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// getLastError
//

std::string
CVscpWebSocket::getLastError(void)
{
  std::lock_guard<std::mutex> lock(m_pimpl->errMutex);
  return m_pimpl->lastError;
}

// ----------------------------------------------------------------------------
//                              vscpWsSettings
// ----------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
// fromJson
//

bool
vscpWsSettings::fromJson(const json &j, uint32_t &connTimeout, uint32_t &respTimeout)
{
  if (!j.is_object()) {
    return false;
  }

  try {

    auto getString = [&j](const char *name, std::string &value) {
      if (j.contains(name) && j[name].is_string()) {
        value = j[name].get<std::string>();
      }
    };

    getString("url", url);
    getString("username", user);
    getString("user", user);
    getString("password", password);

    if (j.contains("key") && j["key"].is_string()) {
      std::string str = trim(j["key"].get<std::string>());
      if (str.empty()) {
        key = VSCP_WS_DEFAULT_KEY;
      }
      else if (isHexKey(str)) {
        key = str;
      }
      else {
        return false;
      }
    }

    if (j.contains("connection-timeout") && j["connection-timeout"].is_number_unsigned()) {
      connTimeout = j["connection-timeout"].get<uint32_t>();
    }

    if (j.contains("response-timeout") && j["response-timeout"].is_number_unsigned()) {
      respTimeout = j["response-timeout"].get<uint32_t>();
    }

    // TLS
    if (j.contains("bverifypeer") && j["bverifypeer"].is_boolean()) {
      tls.bVerifyPeer = j["bverifypeer"].get<bool>();
    }
    getString("cafile", tls.cafile);
    getString("capath", tls.capath);
    getString("certfile", tls.certfile);
    getString("keyfile", tls.keyfile);
    getString("pwkeyfile", tls.pwkeyfile);

    // Filter, either at the top level or in a "filter" object
    const json &jf = (j.contains("filter") && j["filter"].is_object()) ? j["filter"] : j;
    if (jf.contains("priority-filter") && jf["priority-filter"].is_number()) {
      filter.filter_priority = jf["priority-filter"].get<uint8_t>();
    }
    if (jf.contains("priority-mask") && jf["priority-mask"].is_number()) {
      filter.mask_priority = jf["priority-mask"].get<uint8_t>();
    }
    if (jf.contains("class-filter") && jf["class-filter"].is_number()) {
      filter.filter_class = jf["class-filter"].get<uint16_t>();
    }
    if (jf.contains("class-mask") && jf["class-mask"].is_number()) {
      filter.mask_class = jf["class-mask"].get<uint16_t>();
    }
    if (jf.contains("type-filter") && jf["type-filter"].is_number()) {
      filter.filter_type = jf["type-filter"].get<uint16_t>();
    }
    if (jf.contains("type-mask") && jf["type-mask"].is_number()) {
      filter.mask_type = jf["type-mask"].get<uint16_t>();
    }
    if (jf.contains("guid-filter") && jf["guid-filter"].is_string()) {
      vscp_getGuidFromStringToArray(filter.filter_GUID, jf["guid-filter"].get<std::string>());
    }
    if (jf.contains("guid-mask") && jf["guid-mask"].is_string()) {
      vscp_getGuidFromStringToArray(filter.mask_GUID, jf["guid-mask"].get<std::string>());
    }
  }
  catch (...) {
    return false;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// toJson
//

void
vscpWsSettings::toJson(json &j, uint32_t connTimeout, uint32_t respTimeout) const
{
  std::string str;

  j["url"]                = url;
  j["user"]               = user;
  j["password"]           = password;
  j["key"]                = key;
  j["connection-timeout"] = connTimeout;
  j["response-timeout"]   = respTimeout;

  j["bverifypeer"] = tls.bVerifyPeer;
  j["cafile"]      = tls.cafile;
  j["capath"]      = tls.capath;
  j["certfile"]    = tls.certfile;
  j["keyfile"]     = tls.keyfile;
  j["pwkeyfile"]   = tls.pwkeyfile;

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
}

///////////////////////////////////////////////////////////////////////////////
// getKey
//

bool
vscpWsSettings::getKey(uint8_t outkey[16]) const
{
  if (!isHexKey(key)) {
    return false;
  }
  return (16 == vscp_hexStr2ByteArray(outkey, 16, key.c_str()));
}

///////////////////////////////////////////////////////////////////////////////
// isFilterOpen
//

bool
vscpWsSettings::isFilterOpen(void) const
{
  if (filter.mask_priority || filter.mask_class || filter.mask_type) {
    return false;
  }
  for (int i = 0; i < 16; i++) {
    if (filter.mask_GUID[i]) {
      return false;
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// vscp_ws_encryptCredentials
//

int
vscp_ws_encryptCredentials(std::string &strout,
                           const std::string &user,
                           const std::string &password,
                           const uint8_t *key,
                           const uint8_t *iv)
{
  if ((nullptr == key) || (nullptr == iv)) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  std::string strCombined = user + ":" + password;

  // AES works on whole 16 byte blocks. Zero pad the plain text to a block
  // boundary; the server stops at the first zero byte.
  const size_t len    = strCombined.length();
  const size_t padded = ((len + 15) / 16) * 16;
  if ((0 == padded) || (padded > UINT32_MAX)) {
    return VSCP_ERROR_PARAMETER;
  }

  std::vector<uint8_t> plain(padded, 0);
  std::vector<uint8_t> cipher(padded, 0);
  memcpy(plain.data(), strCombined.data(), len);

  uint8_t keycopy[16];
  uint8_t ivcopy[16];
  memcpy(keycopy, key, 16);
  memcpy(ivcopy, iv, 16);

  AES_CBC_encrypt_buffer(AES128, cipher.data(), plain.data(), (uint32_t) padded, keycopy, ivcopy);

  std::vector<char> out(2 * padded + 1, 0);
  vscp_byteArray2HexStr(out.data(), cipher.data(), padded);
  strout = out.data();

  // Do not leave the clear text credentials in memory
  std::fill(plain.begin(), plain.end(), 0);
  std::fill(strCombined.begin(), strCombined.end(), '\0');

  return VSCP_ERROR_SUCCESS;
}
