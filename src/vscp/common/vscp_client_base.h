// vscp_base_client.h.
//
// Virtual base class for client communication classes.
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

#if !defined(VSCPCLIENTBASE_H__INCLUDED_)
#define VSCPCLIENTBASE_H__INCLUDED_

#include "vscp.h"

#include <nlohmann/json.hpp> // nlohmann

#include <deque>
#include <list>
#include <string>

// for convenience
using json = nlohmann::json;

// Callback to application if used. None, one or both can be used.
// The second argument is a pointer tha application can set and use
// as it wants.

#ifdef WIN32
typedef void(__stdcall *LPFNDLL_EV_CALLBACK)(vscpEvent *pev, void *pobj);   // Event callback
typedef void(__stdcall *LPFNDLL_EX_CALLBACK)(vscpEventEx *pex, void *pobj); // Event ex callbac
#else
typedef void (*LPFNDLL_EV_CALLBACK)(vscpEvent *pev, void *pobj);   // Event callback
typedef void (*LPFNDLL_EX_CALLBACK)(vscpEventEx *pex, void *pobj); // Event ex callback
#endif

class CVscpClient {

public:
  CVscpClient();
  ~CVscpClient();

  /*!
      vscp-client class types
      =======================
      - NONE - Undefined
      - TCPIP - VSCP tcp/ip link protocol.
      - CANAL - The CANAL protocol. This is the same as a VSCP level I driver.
      - SOCKETCAN - VSCP events sent ovr socketcan.
      - WS1 - VSCP websocket ws1 protocol.
      - WS2 - VSCP websocket ws2 protocol.
      - MQTT - VSCP over MQTT.
      - UDP - VSCP over UDP.
      - MULTICAST - VSCP multicast protocol.
      - REST - VSCP REST interface.
      - RAWCAN - Handle standard CAN and CANFD.
      - RAWMQTT - Handle standard MQTT.
  */

  typedef enum class connType {
    NONE = 0,
    TCPIP,
    CANAL,
    LEVEL2,
    SOCKETCAN,
    WS1,
    WS2,
    MQTT,
    UDP,
    MULTICAST,
    RAWCAN,
    RAWMQTT
  } connType;

  /*!
      Connect to remote host
      @param bPoll If true polling is used.
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int connect(void) = 0;

  /*!
      Disconnect from remote host
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int disconnect(void) = 0;

  /*!
      Check if connected.
      @return true if connected, false otherwise.
  */
  virtual bool isConnected(void) = 0;

  /*!
      Send VSCP event to remote host.
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int send(vscpEvent &ev) = 0;

  /*!
      Send CAN(AL) message to remote host.
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int send(canalMsg &msg) = 0;

  /*!
      Send VSCP event to remote host.
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int send(vscpEventEx &ex) = 0;

  /*!
      Receive VSCP event from remote host
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int receive(vscpEvent &ev) = 0;

  /*!
      Receive VSCP event ex from remote host
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int receive(vscpEventEx &ex) = 0;

  /*!
      Receive CAN(AL) message from remote host
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int receive(canalMsg &msg) = 0;

  /*!
      Set interface filter
      @param filter VSCP Filter to set.
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int setfilter(vscpEventFilter &filter) = 0;

  /*!
      Get number of events waiting to be received on remote
      interface
      @param pcount Pointer to an unsigned integer that get the count of events.
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int getcount(uint16_t *pcount) = 0;

  /*!
      Clear the input queue
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int clear(void) = 0;

  /*!
      Get version from interface
      @param pmajor Pointer to uint8_t that get major version of interface.
      @param pminor Pointer to uint8_t that get minor version of interface.
      @param prelease Pointer to uint8_t that get release version of interface.
      @param pbuild Pointer to uint8_t that get build version of interface.
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int getversion(uint8_t *pmajor, uint8_t *pminor, uint8_t *prelease, uint8_t *pbuild) = 0;

  /*!
      Get interfaces
      @param iflist Get a list of available interfaces
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int getinterfaces(std::deque<std::string> &iflist) = 0;

  /*!
      Get capabilities (wcyd) from remote interface
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int getwcyd(uint64_t &wcyd) = 0;

  /*!
      Set (and enable) receive callback for events
      @param LPFNDLL_EX_CALLBACK Callback to call when an event is received
      @param pData User defined data to pass in callback call
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int setCallback(LPFNDLL_EV_CALLBACK evcallback, void *pData = nullptr);

  /*!
      Set (and enable) receive callback ex events
      @param LPFNDLL_EX_CALLBACK Callback to call when an event is received
      @param pData User defined data to pass in callback call
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int setCallback(LPFNDLL_EX_CALLBACK excallback, void *pData = nullptr);

  /*!
      Getter/setters for connection timeout
      Time is in milliseconds
  */
  virtual void setConnectionTimeout(uint32_t timeout) = 0;
  virtual uint32_t getConnectionTimeout(void)         = 0;

  /*!
      Getter/setters for response timeout
      Time is in milliseconds
  */
  virtual void setResponseTimeout(uint32_t timeout) = 0;
  virtual uint32_t getResponseTimeout(void)         = 0;

  /*!
      Check if ev callback is defined
      @return true if callback is defined
  */
  bool isEvCallback(void) { return (nullptr != m_evcallback); }

  /*!
      Check if ex callback is defined
      @return true if callback is defined
  */
  bool isExCallback(void) { return (nullptr != m_excallback); }

  /*!
      Return a JSON representation of connection
      @return JSON representation as string
  */
  virtual std::string getConfigAsJson(void) = 0;

  /*!
      Set member variables from JSON representation of connection
      @param config JSON representation as string
      @return True on success, false on failure.
  */
  virtual bool initFromJson(const std::string &config) = 0;

  // ------------------------------------------------------------------------

  /*!
      Get connection type
      @return Type for the connection
  */
  connType getType(void) { return m_type; };

  /*!
      Set name for communication object
  */
  virtual void setName(const std::string &name) { m_name = name; };

  /*!
      Get name for communication object
  */
  virtual std::string getName(void) { return m_name; };

  /*!
    Mark as full level II or not
    @param b Set to true to mark as full level II working client
  */
  void setFullLevel2(bool b=true) { m_bFullLevel2 = b; };

  /*!
    Check if client is afull level II client
    @return true If the client is a full level II client
  */
  bool isFullLevel2(void) { return m_bFullLevel2; };

public:
  /*!
      Callback for events
  */
  LPFNDLL_EV_CALLBACK m_evcallback;

  /*!
      Callback for ex events
  */
  LPFNDLL_EX_CALLBACK m_excallback;

  /*!
      This data pointer is set by the callback
      setter and is sent with the callback call
  */
  void *m_callbackObject;

  /// Type of connection object
  connType m_type = CVscpClient::connType::NONE;

  /// Name for connection object
  std::string m_name;

  /*!
    True if communication client work with FULL GUID's
    and not nickname and interfaces
  */
  bool m_bFullLevel2;
};

#endif
