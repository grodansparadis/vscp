// vscp_client_ws1.h
//
// Websocket if 1 client communication classes.
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

#if !defined(VSCPCLIENTWS1_H__INCLUDED_)
#define VSCPCLIENTWS1_H__INCLUDED_

#include <pthread.h>
#include <semaphore.h>

#ifndef WIN32

#else
#include <winSock2.h>
#endif

#include "vscp.h"
#include "vscp-client-base.h"

const uint32_t WS1_CONNECTION_TIMEOUT = 30000;
const uint32_t WS1_RESPONSE_TIMEOUT   = 2000;

class vscpClientWs1 : public CVscpClient {

public:
  vscpClientWs1();
  virtual ~vscpClientWs1();

  /*!
      Init client
      @param host Host to connect to.
      @param port Port on host to connect to.
      @param bSSL True to activate SSL/TLS.
      @param username Username credentials.
      @param password Password credentials.
      @param vscpkey Secret key.
      @param connection_timeout Connection timeout in ms.
      @param response_timeout Response timeout in ms.
  */
  virtual int init(const std::string host,
                   short port,
                   bool bSSL,
                   const std::string username,
                   const std::string password,
                   uint8_t *vscpkey,
                   uint32_t connection_timeout = WS1_CONNECTION_TIMEOUT,
                   uint32_t response_timeout   = WS1_RESPONSE_TIMEOUT);

  /*!
      Connect to remote host
      @param bPoll If true polling is used.
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int connect(void);

  /*!
      Disconnect from remote host
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int disconnect(void);

  /*!
      Check if connected.
      @return true if connected, false otherwise.
  */
  virtual bool isConnected(void);

  /*!
      Send VSCP event to remote host.
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int send(vscpEvent &ev);

  /*!
      Send VSCP event to remote host.
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int send(vscpEventEx &ex);

  /*!
    Send VSCP CAN(AL) message to remote host.
    @return Return VSCP_ERROR_SUCCESS of OK and error code else.
*/
  virtual int send(canalMsg &msg);

  /*!
      Receive VSCP event from remote host
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int receive(vscpEvent &ev);

  /*!
      Blocking receive of VSCP event ex from remote host
      @param ev VSCP event ex that will get the result.
      @param timeout Timeout in milliseconds. Default is 100 ms.
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int receiveBlocking(vscpEvent &ev, long timeout = 100 );

  /*!
      Receive VSCP event ex from remote host
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int receive(vscpEventEx &ex);

  /*!
      Blocking receive of VSCP event ex from remote host
      @param ex VSCP event ex that will get the result.
      @param timeout Timeout in milliseconds. Default is 100 ms.
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int receiveBlocking(vscpEventEx &ex, long timeout = 100 );

  /*!
    Receive CAN(AL) message from remote host
    @return Return VSCP_ERROR_SUCCESS of OK and error code else.
*/
  virtual int receive(canalMsg &msg);

  /*!
      Receive blocking CAN(AL) message from remote host
      @param msg CANAL message that will get the result.
      @param timeout Timeout in milliseconds. Default is 100 ms.
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int receiveBlocking(canalMsg &msg, long timeout = 100);

  /*!
      Set interface filter
      @param filter VSCP Filter to set.
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int setfilter(vscpEventFilter &filter);

  /*!
      Get number of events waiting to be received on remote
      interface
      @param pcount Pointer to an unsigned integer that get the count of events.
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int getcount(uint16_t *pcount);

  /*!
      Clear the input queue
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int clear(void);

  /*!
      Get version from interface
      @param pmajor Pointer to uint8_t that get major version of interface.
      @param pminor Pointer to uint8_t that get minor version of interface.
      @param prelease Pointer to uint8_t that get release version of interface.
      @param pbuild Pointer to uint8_t that get build version of interface.
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int getversion(uint8_t *pmajor, uint8_t *pminor, uint8_t *prelease, uint8_t *pbuild);

  /*!
      Get interfaces
      @param iflist Get a list of available interfaces
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int getinterfaces(std::deque<std::string> &iflist);

  /*!
      Get capabilities (wcyd) from remote interface
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int getwcyd(uint64_t &wcyd);

  /*!
      Return a JSON representation of connection
      @return JSON representation as string
  */
  virtual std::string getConfigAsJson(void);

  /*!
      Set member variables from JSON representation of connection
      @param config JSON representation as string
      @return True on success, false on failure.
  */
  virtual bool initFromJson(const std::string &config);

  virtual void setConnectionTimeout(uint32_t timeout = WS1_CONNECTION_TIMEOUT) { m_timeout_connect = timeout; };
  virtual uint32_t getConnectionTimeout(void) { return m_timeout_connect; };

  virtual void setResponseTimeout(uint32_t timeout = WS1_RESPONSE_TIMEOUT) { m_timeout_response = timeout; };
  virtual uint32_t getResponseTimeout(void) { return m_timeout_response; };

  /*!
      Encrypt the admin/password pair
      @param strout Receive encrypted password
      @param struser Username
      @param strpassword Password
      @param vscpkey 32 byte secret key known by client and server, This
              key should be read from disk and NOT be stored in code.
      @param iv initialization vector, 16 byte seed for encryption
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  int encrypt_password(std::string &strout,
                       std::string struser,
                       std::string strpassword,
                       uint8_t *vscpkey,
                       uint8_t *iv);

  /*!
      Wait for data yo arrive fromthe websocket
      @param timeout Maximum time to wait.
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
              VSC_ERROR_TIMEOUT is returned for timeout.
  */
  int waitForResponse(uint32_t timeout = WS1_RESPONSE_TIMEOUT);

public:
  // True if connected
  bool m_bConnected;

  // Connection object
  struct mg_connection *m_conn;

  // Semaphore for message receive queue
  sem_t m_sem_msg;

  // JSON message receive queue
  std::deque<std::string> m_msgReceiveQueue;

  // VSCP Event receive queue
  std::deque<vscpEvent *> m_eventReceiveQueue;

private:
  bool m_bSSL; // True for SSL/TSL

  // This is the encryption result over "username:password"
  // using the vscpkey and iv from server for encryption
  std::string m_credentials;

  // Initialization vector
  uint8_t m_iv[16];

  // Host to connect to (default is 8884)
  // (default is localhost)
  std::string m_host;

  // Port on host to connect to (8884/8843  443)
  // (default is 8884)
  short m_port;

  uint32_t m_timeout_connect;

  uint32_t m_timeout_response;
};

#endif
