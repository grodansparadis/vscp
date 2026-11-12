// vscp-websocket.h
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

#if !defined(VSCP_WEBSOCKET_H__INCLUDED_)
#define VSCP_WEBSOCKET_H__INCLUDED_

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "vscp.h"

// Default AES-128 key used to encrypt the credentials. It is the sample key
// the VSCP websocket server uses when no "key-file" is configured. Every real
// installation should use its own key.
#define VSCP_WS_DEFAULT_KEY "2DBB079A38985AF00EBEEFE22F9FFA0E"

/*!
  TLS settings for a wss:// connection. The names are the same as the JSON
  keys used by the tcp/ip client.
*/
struct vscpWsTlsOptions {
  bool bVerifyPeer = true; // Verify the server certificate and host name
  std::string cafile;      // PEM file with trusted CA certificates
  std::string capath;      // Directory with trusted CA certificates
  std::string certfile;    // Client certificate (optional)
  std::string keyfile;     // Client private key (optional)
  std::string pwkeyfile;   // Password for the client private key (optional)
};

/*!
  Parts of a ws:// or wss:// URL.
*/
struct vscpWsUrl {
  bool bTLS = false;
  std::string host;
  std::string port;
  std::string path;
};

/*!
  Websocket client connection.

  One thread may call receive() while other threads call sendText(); writes
  are serialized internally. Only text and binary messages are returned by
  receive(). Ping, pong and close frames are handled internally.
*/
class CVscpWebSocket {

public:
  CVscpWebSocket();
  ~CVscpWebSocket();

  CVscpWebSocket(const CVscpWebSocket &)            = delete;
  CVscpWebSocket &operator=(const CVscpWebSocket &) = delete;

  /*!
    Split a websocket URL.
    @param url URL on the form ws://host[:port][/path] or wss://... A missing
          scheme means ws://. http:// and https:// are accepted as aliases.
          IPv6 addresses are given in brackets, ws://[::1]:8884/ws1
    @param defaultPath Path to use when the URL has none (or only "/").
    @param parts Receives the parts of the URL.
    @return true if the URL could be parsed.
  */
  static bool parseUrl(const std::string &url, const std::string &defaultPath, vscpWsUrl &parts);

  /*!
    Open a connection and do the websocket handshake.
    @param url Parsed URL.
    @param timeout Timeout in milliseconds for the connect, TLS handshake and
          websocket upgrade together.
    @param tls TLS options, used for wss:// only.
    @return VSCP_ERROR_SUCCESS on success, VSCP_ERROR_TIMEOUT on timeout and
          VSCP_ERROR_CONNECTION for other errors (see getLastError()).
  */
  int connect(const vscpWsUrl &url, uint32_t timeout, const vscpWsTlsOptions &tls = vscpWsTlsOptions());

  /*!
    Send a close frame (if open) and close the connection.
  */
  void close(void);

  /*!
    @return true if the connection is open.
  */
  bool isOpen(void) const { return m_bOpen; }

  /*!
    Send a text message.
    @param msg Message to send.
    @param timeout Maximum time in milliseconds to wait for the socket.
    @return VSCP_ERROR_SUCCESS on success, error code else.
  */
  int sendText(const std::string &msg, uint32_t timeout = 5000);

  /*!
    Wait for the next text or binary message.
    @param msg Receives the message.
    @param bText Set to true for a text message, false for binary.
    @param timeout Maximum time to wait in milliseconds.
    @return VSCP_ERROR_SUCCESS when a message was received,
            VSCP_ERROR_TIMEOUT when no message arrived in time,
            VSCP_ERROR_CONNECTION when the connection is closed or broken.
  */
  int receive(std::string &msg, bool &bText, uint32_t timeout);

  /*!
    @return Text describing the last error.
  */
  std::string getLastError(void);

  // Largest message accepted from the server
  static const size_t MAX_MESSAGE_SIZE = 1024 * 1024;

  /*!
    Build a frame as a client would send it (masked). Exposed for tests.
  */
  static std::string buildFrame(uint8_t opcode, const std::string &payload, const uint8_t mask[4]);

  /*!
    Compute the Sec-WebSocket-Accept value for a key. Exposed for tests.
  */
  static std::string acceptKey(const std::string &key);

private:
  struct Impl;
  Impl *m_pimpl;

  std::atomic<bool> m_bOpen;
};

/*!
  Settings shared by the ws1 and ws2 clients, read from and written to the
  JSON connection configuration.

  JSON keys:
    "url"                 ws://host:port/ws1, wss://... for TLS
    "user"                User name ("username" is also accepted)
    "password"            Password
    "key"                 AES-128 key (32 hex characters) shared with the
                          server. Default is VSCP_WS_DEFAULT_KEY.
    "connection-timeout"  Milliseconds
    "response-timeout"    Milliseconds
    "bverifypeer", "cafile", "capath", "certfile", "keyfile", "pwkeyfile"
                          TLS settings (see vscpWsTlsOptions)
    "priority-filter", "priority-mask", "class-filter", "class-mask",
    "type-filter", "type-mask", "guid-filter", "guid-mask"
                          Receive filter. A "filter" object with the same
                          keys is also accepted.
*/
struct vscpWsSettings {
  std::string url;
  std::string user;
  std::string password;
  std::string key = VSCP_WS_DEFAULT_KEY;
  vscpWsTlsOptions tls;
  vscpEventFilter filter = {};

  /*!
    Read settings. Keys that are missing keep their current value.
    @param j JSON object.
    @param connTimeout Receives "connection-timeout" if present.
    @param respTimeout Receives "response-timeout" if present.
    @return false if a value has the wrong type or the key is invalid.
  */
  bool fromJson(const nlohmann::json &j, uint32_t &connTimeout, uint32_t &respTimeout);

  /*!
    Write settings to a JSON object.
  */
  void toJson(nlohmann::json &j, uint32_t connTimeout, uint32_t respTimeout) const;

  /*!
    Get the key as 16 bytes.
    @return false if the key is not 32 hexadecimal characters.
  */
  bool getKey(uint8_t key[16]) const;

  /*!
    @return true if the filter lets everything through.
  */
  bool isFilterOpen(void) const;
};

/*!
  Encrypt "user:password" for websocket authentication.
  AES-128-CBC with zero padding to a whole block, hex encoded.
  @param strout Receives the result.
  @param user User name.
  @param password Password.
  @param key 16 byte key.
  @param iv 16 byte initialization vector (the session id from the server).
  @return VSCP_ERROR_SUCCESS on success, error code else.
*/
int
vscp_ws_encryptCredentials(std::string &strout,
                           const std::string &user,
                           const std::string &password,
                           const uint8_t *key,
                           const uint8_t *iv);

#endif
