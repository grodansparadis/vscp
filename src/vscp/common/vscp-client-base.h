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
// Copyright © 2000-2025 Ake Hedman, Grodans Paradis AB
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

#include "canal.h"
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
// std::function<void(vscpEvent &ev, void *pobj)>
// std::function<void(vscpEventEx &ex, void *pobj)>
// typedef void(__stdcall *CALLBACK_EV)(vscpEvent *pev, void *pobj);   // Event callback
// typedef void(__stdcall *CALLBACK_EX)(vscpEventEx *pex, void *pobj); // Event ex callback
#else
// typedef void (*CALLBACK_EV)(vscpEvent *pev, void *pobj);   // Event callback
// typedef void (*CALLBACK_EX)(vscpEventEx *pex, void *pobj); // Event ex callback
#endif

class CVscpClient {

public:
  CVscpClient();
  virtual ~CVscpClient();

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
    SOCKETCAN,
    WS1,
    WS2,
    MQTT,
    UDP,
    MULTICAST,
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
      Blocking receive of VSCP event ex from remote host
      @param ev VSCP event that will get the result.
      @param timeout Timeout in milliseconds. Zero is no wait. Default is 100 ms.
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int receiveBlocking(vscpEvent &ev, long timeout = 100) = 0;

  /*!
      Receive VSCP event ex from remote host
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int receive(vscpEventEx &ex) = 0;

  /*!
     Blocking receive of VSCP event ex from remote host
     @param ex VSCP event ex that will get the result.
     @param timeout Timeout in milliseconds. Zero is no wait. Default is 100 ms.
     @return Return VSCP_ERROR_SUCCESS of OK and error code else.
 */
  virtual int receiveBlocking(vscpEventEx &ex, long timeout = 100) = 0;

  /*!
      Receive CAN(AL) message from remote host
      @param msg CANAL message that will get the result.
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int receive(canalMsg &msg) = 0;

  /*!
      Blocking receive of VSCP event ex from remote host
      @param msg CANAL message that will get the result.
      @param timeout Timeout in milliseconds. Zero is no wait. Default is 100 ms.
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int receiveBlocking(canalMsg &msg, long timeout = 100) = 0;

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
      @param callback Callback to call when an event is received
      @param pData User defined data to pass in callback call
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int setCallbackEv(std::function<void(vscpEvent &ev, void *pobj)> callback, void *pData = nullptr);

  /*!
      Set (and enable) receive callback ex events
      @param callback Callback to call when an event is received
      @param pData User defined data to pass in callback call
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int setCallbackEx(std::function<void(vscpEventEx &ex, void *pobj)> callback, void *pData = nullptr);

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
  bool isCallbackEvActive(void) { return m_bActiveCallbackEv; }

  /*!
      Check if ex callback is defined
      @return true if callback is defined
  */
  bool isCallbackExActive(void) { return m_bActiveCallbackEx; }

  /*!
    Activate or deactivate Ev callback
    @param b Set to true (default) to activate.
  */
  void setCallbackEvActive(bool b = true) { m_bActiveCallbackEv = b; };

  /*!
    Activate or deactivate Ex callback
    @param b Set to true (default) to activate.
  */
  void setCallbackExActive(bool b = true) { m_bActiveCallbackEx = b; };

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
  void setFullLevel2(bool b = true) { m_bFullLevel2 = b; };

  /*!
    Check if client is afull level II client
    @return true If the client is a full level II client
  */
  bool isFullLevel2(void) { return m_bFullLevel2; };

  /*!
    Set callback object
    @param Pointer to object to set
  */

  void setCallbackObj(void *pobj) { m_callbackObject = pobj; };

  /*!
    Return Callback object
    @return Pointer to object
  */
  void *getCallbackObj(void) { return m_callbackObject; };

public:
  /*!
      Callback for events
  */
  std::function<void(vscpEvent &ev, void *pobj)> m_callbackev;

  /*!
      Callback for ex events
  */
  std::function<void(vscpEventEx &ex, void *pobj)> m_callbackex;

protected:
  /// Type of connection object
  connType m_type = CVscpClient::connType::NONE;

  /*!
    Flag that is true when an ev  callback is installed
  */
  bool m_bActiveCallbackEv;

  /*!
    Flag that is true when an ex  callback is installed
  */
  bool m_bActiveCallbackEx;

  /*!
      This data pointer is set by the callback
      setter and is sent with the callback call
  */
  void *m_callbackObject;

  /// Name for connection object
  std::string m_name;

  /*!
    True if communication client work with FULL GUID's
    and not nickname and interfaces
  */
  bool m_bFullLevel2;
};

#endif
