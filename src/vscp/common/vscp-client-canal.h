// vscp_client_canal.cpp
//
// CANAL client communication classes.
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

#if !defined(VSCPCLIENTCANAL_H__INCLUDED_)
#define VSCPCLIENTCANAL_H__INCLUDED_

#include "vscp.h"
#include "vscp_client_base.h"
#include "vscpcanaldeviceif.h"

#include <pthread.h>

// When a callback is set and connect is called this object is shared
// with a workerthread that

class vscpClientCanal : public CVscpClient {

public:
  vscpClientCanal();
  ~vscpClientCanal();

  /*!
      Initialize the CANAL client
      @param strPath Path to CANAL driver.
      @param strParameters CANAL driver configuration string.
      @param flags CANAL driver configuration flags.
      @param datarate Set baudrate/bitrate. If zero baudrate is
                          not set.
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  int init(const std::string &strPath,
           const std::string &strParameters = "",
           unsigned long flags              = 0,
           unsigned long datarate           = 0);

  // Run wizard
  int runWizard(void);

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
    Send CAN(AL) message to remote host.
    @return Return VSCP_ERROR_SUCCESS of OK and error code else.
*/
  virtual int send(canalMsg &msg);

  /*!
      Receive VSCP event from remote host
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int receive(vscpEvent &ev);

  /*!
      Receive VSCP event ex from remote host
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int receive(vscpEventEx &ex);

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
      Set (and enable) receive callback for events
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int setCallbackEv(std::function<void(vscpEvent &ev, void *pobj)> callback);

  /*!
      Set (and enable) receive callback ex events
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int setCallbackEx(std::function<void(vscpEventEx &ex, void *pobj)> callback);

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
  // True as long as the worker thread should do it's work
  bool m_bRun;

  // Mutex that protect CANAL interface when callbacks are defined
  pthread_mutex_t m_mutexif;

  // CANAL functionality
  VscpCanalDeviceIf m_canalif;

private:
  /*!
      True of dll connection is open
  */
  bool m_bConnected;

  // Worker thread id
  pthread_t m_tid;
};

#endif
