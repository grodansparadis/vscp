// fakewsserver.h
//
// Minimal websocket server for the ws1/ws2 client unit tests (POSIX only).
//
// Accepts one connection on 127.0.0.1, does the RFC 6455 handshake, sends the
// greeting messages and then calls a handler for each text message from the
// client. The handler returns the replies to send. The test can also send
// messages and raw frames from its own thread.
//
// SPDX-License-Identifier: MIT

#ifndef FAKEWSSERVER_H
#define FAKEWSSERVER_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <cstring>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <vscp-aes.h>
#include <vscp-websocket.h>
#include <vscphelper.h>

class FakeWsServer {

public:
  typedef std::function<std::vector<std::string>(const std::string &)> Handler;

  explicit FakeWsServer(Handler handler, std::vector<std::string> greeting = {})
    : m_handler(handler)
    , m_greeting(greeting)
  {
    m_listen = socket(AF_INET, SOCK_STREAM, 0);
    int on   = 1;
    setsockopt(m_listen, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port        = 0;
    bind(m_listen, (struct sockaddr *) &addr, sizeof(addr));
    listen(m_listen, 1);
    socklen_t len = sizeof(addr);
    getsockname(m_listen, (struct sockaddr *) &addr, &len);
    m_port   = ntohs(addr.sin_port);
    m_thread = std::thread(&FakeWsServer::run, this);
  }

  ~FakeWsServer()
  {
    m_bQuit = true;
    shutdown(m_listen, SHUT_RDWR);
    ::close(m_listen);
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      if (m_conn >= 0) {
        shutdown(m_conn, SHUT_RDWR);
      }
    }
    if (m_thread.joinable()) {
      m_thread.join();
    }
    if (m_conn >= 0) {
      ::close(m_conn);
    }
  }

  unsigned short port(void) const { return m_port; }

  std::string url(const std::string &path) const { return "ws://127.0.0.1:" + std::to_string(m_port) + path; }

  // Send a text message to the client
  void sendText(const std::string &msg) { sendFrame(0x81, msg); }

  // Send a raw frame (first byte = FIN + opcode)
  void sendFrame(uint8_t b0, const std::string &payload)
  {
    std::string frame;
    frame.push_back((char) b0);
    if (payload.size() < 126) {
      frame.push_back((char) payload.size());
    }
    else if (payload.size() <= 0xffff) {
      frame.push_back((char) 126);
      frame.push_back((char) ((payload.size() >> 8) & 0xff));
      frame.push_back((char) (payload.size() & 0xff));
    }
    else {
      frame.push_back((char) 127);
      for (int i = 7; i >= 0; i--) {
        frame.push_back((char) ((((uint64_t) payload.size()) >> (8 * i)) & 0xff));
      }
    }
    frame += payload;
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_conn >= 0) {
      ::send(m_conn, frame.data(), frame.size(), MSG_NOSIGNAL);
    }
  }

  // Close the connection from the server side
  void closeConnection(void)
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_conn >= 0) {
      shutdown(m_conn, SHUT_RDWR);
    }
  }

  // Messages received from the client (text frames)
  std::vector<std::string> received(void)
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_received;
  }

  // Pong payloads received from the client
  std::vector<std::string> pongs(void)
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_pongs;
  }

  // Wait until the client has sent at least count messages
  bool waitReceived(size_t count, int timeout_ms = 2000)
  {
    for (int i = 0; i < timeout_ms / 10; i++) {
      if (received().size() >= count) {
        return true;
      }
      usleep(10000);
    }
    return false;
  }

  bool gotClose(void) const { return m_bGotClose; }

  // Decrypt credentials as the VSCP websocket server does
  static std::string decrypt(const std::string &crypto, const std::string &sid, const std::string &hexkey)
  {
    uint8_t key[16], iv[16];
    vscp_hexStr2ByteArray(key, 16, hexkey.c_str());
    vscp_hexStr2ByteArray(iv, 16, sid.c_str());
    std::vector<uint8_t> in(crypto.size() / 2 + 1, 0);
    size_t n = vscp_hexStr2ByteArray(in.data(), in.size(), crypto.c_str());
    std::vector<uint8_t> out(n + 1, 0);
    AES_CBC_decrypt_buffer(AES128, out.data(), in.data(), (uint32_t) n, key, iv);
    return std::string(reinterpret_cast<char *>(out.data()));
  }

  static const char *sid(void) { return "0123456789ABCDEF0123456789ABCDEF"; }

private:
  bool readExact(std::string &buf, size_t n)
  {
    while (buf.size() < n) {
      char tmp[4096];
      ssize_t r = recv(m_conn, tmp, sizeof(tmp), 0);
      if (r <= 0) {
        return false;
      }
      buf.append(tmp, (size_t) r);
    }
    return true;
  }

  void run(void)
  {
    int c = accept(m_listen, nullptr, nullptr);
    if (c < 0) {
      return;
    }
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_conn = c;
    }

    // Handshake
    std::string buf;
    size_t end;
    while (std::string::npos == (end = buf.find("\r\n\r\n"))) {
      char tmp[1024];
      ssize_t r = recv(m_conn, tmp, sizeof(tmp), 0);
      if (r <= 0) {
        return;
      }
      buf.append(tmp, (size_t) r);
    }
    std::string req = buf.substr(0, end);
    buf.erase(0, end + 4);

    std::string key;
    size_t pos = req.find("Sec-WebSocket-Key: ");
    if (std::string::npos != pos) {
      key = req.substr(pos + 19, req.find("\r\n", pos) - pos - 19);
    }
    m_request = req;

    std::string resp = "HTTP/1.1 101 Switching Protocols\r\n"
                       "Upgrade: websocket\r\n"
                       "Connection: Upgrade\r\n"
                       "Sec-WebSocket-Accept: " +
                       CVscpWebSocket::acceptKey(key) + "\r\n\r\n";
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      ::send(m_conn, resp.data(), resp.size(), MSG_NOSIGNAL);
    }

    for (const auto &msg : m_greeting) {
      sendText(msg);
    }

    // Frames from the client
    while (!m_bQuit) {
      if (!readExact(buf, 2)) {
        return;
      }
      uint8_t b0    = (uint8_t) buf[0];
      uint8_t b1    = (uint8_t) buf[1];
      uint64_t plen = b1 & 0x7f;
      size_t hdr    = 2;
      if (126 == plen) {
        if (!readExact(buf, 4)) {
          return;
        }
        plen = ((uint8_t) buf[2] << 8) | (uint8_t) buf[3];
        hdr  = 4;
      }
      else if (127 == plen) {
        if (!readExact(buf, 10)) {
          return;
        }
        plen = 0;
        for (int i = 0; i < 8; i++) {
          plen = (plen << 8) | (uint8_t) buf[2 + i];
        }
        hdr = 10;
      }
      // Client frames must be masked
      if (!(b1 & 0x80)) {
        m_bUnmasked = true;
        return;
      }
      if (!readExact(buf, hdr + 4 + plen)) {
        return;
      }
      uint8_t mask[4];
      memcpy(mask, buf.data() + hdr, 4);
      std::string payload = buf.substr(hdr + 4, plen);
      for (size_t i = 0; i < payload.size(); i++) {
        payload[i] = (char) (payload[i] ^ mask[i % 4]);
      }
      buf.erase(0, hdr + 4 + plen);

      uint8_t opcode = b0 & 0x0f;
      if (0x8 == opcode) {
        m_bGotClose = true;
        sendFrame(0x88, payload.substr(0, 2));
        return;
      }
      if (0xA == opcode) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_pongs.push_back(payload);
        continue;
      }
      if (0x1 != opcode) {
        continue;
      }

      {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_received.push_back(payload);
      }
      for (const auto &reply : m_handler(payload)) {
        sendText(reply);
      }
    }
  }

public:
  std::string m_request;          // Handshake request from the client
  std::atomic<bool> m_bUnmasked{ false }; // Client sent an unmasked frame

private:
  Handler m_handler;
  std::vector<std::string> m_greeting;
  int m_listen = -1;
  int m_conn   = -1;
  unsigned short m_port = 0;
  std::thread m_thread;
  std::mutex m_mutex;
  std::vector<std::string> m_received;
  std::vector<std::string> m_pongs;
  std::atomic<bool> m_bQuit{ false };
  std::atomic<bool> m_bGotClose{ false };
};

#endif
