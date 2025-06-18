// vscp_client_multicast.h
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
// Copyright: (C) 2007-2025
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

#if !defined(VSCPCLIENTMULTICAST_H__INCLUDED_)
#define VSCPCLIENTMULTICAST_H__INCLUDED_

#include "vscp.h"
#include "vscp-client-base.h"

#include <mutex>
#include <thread>

class vscpClientMulticast : public CVscpClient {

public:
  vscpClientMulticast();
  virtual ~vscpClientMulticast();

  static const uint8_t VSCP_MAJOR_MULTICAST_CLIENT_VERSION   = 1;
  static const uint8_t VSCP_MINOR_MULTICAST_CLIENT_VERSION   = 0;
  static const uint8_t VSCP_RELEASE_MULTICAST_CLIENT_VERSION = 0;
  static const uint8_t VSCP_BUILD_MULTICAST_CLIENT_VERSION   = 0;

  /*!
      Connect to remote host
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
  virtual int receiveBlocking(vscpEvent &ev, long timeout = 100);

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
  virtual int receiveBlocking(vscpEventEx &ex, long timeout = 100);

  /*!
      Receive blocking CAN(AL) message from remote host
      @param msg CANAL message that will get the result.
      @param timeout Timeout in milliseconds. Default is 100 ms.
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int receiveBlocking(canalMsg &msg, long timeout = 100);

  /*!
   Receive CAN(AL) message from remote host
   @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int receive(canalMsg &msg);

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

  /*!
      Getter/setters for connection timeout
      Time is in milliseconds
  */
  virtual void setConnectionTimeout(uint32_t timeout);
  virtual uint32_t getConnectionTimeout(void);

  /*!
      Getter/setters for response timeout
      Time is in milliseconds
  */
  virtual void setResponseTimeout(uint32_t timeout);
  virtual uint32_t getResponseTimeout(void);

public:
  // JSON configuration
  json m_j_config;

  /// Flag for worker thread run as long it's true
  bool m_bRun;

  /*!
    True if sent frames should be encrypted
   */
  bool m_bEncrypt;

  /*!
    Encryption algorithm. Defines in vscp.h
      0 = None
      1 = AES-128
      2 = AES-192
      3 = AES-256
  */
  uint8_t m_encryptType;

  /*!
    Encryption key
    16-bit for AES-128
    24-bit for AES-192
    32-bit for AES-256
  */
  uint8_t m_key[32]; // AES-(128/192/256) key

  // Queue for received events
  std::list<vscpEvent *> m_receiveQueue;

  /// Mutex to protect communication socket
  pthread_mutex_t m_mutexSocket;

  /// Mutex to protect receive queue
  pthread_mutex_t m_mutexReceiveQueue;

/*!
  Event object to indicate that there is an event
  in the receive queue
*/
#ifdef WIN32
  HANDLE m_semReceiveQueue;
#else
  sem_t m_semReceiveQueue;
#endif

#ifdef WIN32
  WSADATA m_wsaData;
  SOCKET m_sock;
#else
  int m_sock;
#endif

  /// Filters for input
  vscpEventFilter m_filter;

private:
  /*!
    Interface to use for multicast communication.
    Default is empty string which means all interfaces.
  */
  std::string m_interface;

  /*!
      Set the multicast address. Default is VSCP multicast address 224.0.23.158
      @param addr Multicast address to set
  */
  std::string m_multicastGroupAddr;

  /*!
      Set the multicast port. Default is 9598
      @param port Multicast port to set
  */
  short m_multicastPort;

  /*!
    Time to live for multicast packets
    Default is 1 (local network only)
  */
  uint8_t m_ttl;

  // ------------------------------------------------------------------------

  /// Workerthread
  std::thread *m_pworkerthread;
};

#endif
