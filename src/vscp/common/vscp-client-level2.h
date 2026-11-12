// vscp-client-level2.h
//
// VSCP Level II driver client communication class.
//
// Loads a VSCP Level II driver (vscpl2drv-*), opens it with a
// configuration file and exchanges events with it. The configuration
// can be given as a path to an existing file, or embedded in the
// client configuration. An embedded configuration is written to a
// private temporary file when the client connects and removed when it
// disconnects.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// Copyright:  (C) 2007-2026
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

#if !defined(VSCPCLIENTLEVEL2_H__INCLUDED_)
#define VSCPCLIENTLEVEL2_H__INCLUDED_

#include "level2drvdef.h"
#include "vscp-client-base.h"
#include "vscp.h"

#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <thread>

class vscpClientLevel2 : public CVscpClient {

public:
  vscpClientLevel2();
  virtual ~vscpClientLevel2();

  /// Default response timeout in milliseconds (used for VSCPWrite)
  static const uint32_t DEFAULT_RESPONSE_TIMEOUT = 1000;

  /// Timeout in milliseconds for each VSCPRead call in the receive thread
  static const uint32_t READ_TIMEOUT = 100;

  /// Max number of events held in the receive queue
  static const size_t MAX_RECEIVE_QUEUE = 8192;

  /*!
    Initialize with a configuration file
    @param pathDriver Path to the Level II driver shared library
    @param pathConfig Path to the driver configuration file
    @param guid GUID for the driver as a string. Empty for all zeros.
    @return VSCP_ERROR_SUCCESS on success, error code otherwise.
  */
  int init(const std::string &pathDriver, const std::string &pathConfig, const std::string &guid = "");

  /*!
    Initialize with an embedded configuration. The configuration is written
    to a private temporary file that is given to the driver when the client
    connects, and removed when it disconnects.
    @param pathDriver Path to the Level II driver shared library
    @param config Driver configuration (typically JSON or XML text)
    @param guid GUID for the driver as a string. Empty for all zeros.
    @param suffix Suffix for the temporary file (".json", ".xml", ...).
      Empty to guess it from the content.
    @return VSCP_ERROR_SUCCESS on success, error code otherwise.
  */
  int initEmbedded(const std::string &pathDriver,
                   const std::string &config,
                   const std::string &guid   = "",
                   const std::string &suffix = "");

  virtual int connect(void);
  virtual int disconnect(void);
  virtual bool isConnected(void);

  virtual int send(vscpEvent &ev);
  virtual int send(vscpEventEx &ex);
  virtual int send(canalMsg &msg);

  virtual int receive(vscpEvent &ev);
  virtual int receive(vscpEventEx &ex);
  virtual int receive(canalMsg &msg);

  virtual int receiveBlocking(vscpEvent &ev, long timeout = 100);
  virtual int receiveBlocking(vscpEventEx &ex, long timeout = 100);
  virtual int receiveBlocking(canalMsg &msg, long timeout = 100);

  virtual int setfilter(vscpEventFilter &filter);
  virtual int getcount(uint16_t *pcount);
  virtual int clear(void);

  /*!
    Get the driver version (from VSCPGetVersion). The client must be connected.
  */
  virtual int getversion(uint8_t *pmajor, uint8_t *pminor, uint8_t *prelease, uint8_t *pbuild);

  /// A Level II driver has no interfaces. Returns an empty list.
  virtual int getinterfaces(std::deque<std::string> &iflist);

  virtual int getwcyd(uint64_t &wcyd);

  /*!
    Return a JSON representation of the connection
    @return JSON representation as string
  */
  virtual std::string getConfigAsJson(void);

  /*!
    Set the configuration from JSON

    {
      "name": "Name of connection",
      "path-driver": "/path/to/vscpl2drv-xxx.so",
      "path-config": "/path/to/config.json",
      "config": { ... } | "configuration text",
      "config-suffix": ".json",
      "guid": "FF:FF:FF:FF:FF:FF:FF:F5:00:00:00:00:00:00:00:00",
      "response-timeout": 1000,
      "priority-filter": 0, ... (standard filter options)
    }

    "config" (embedded configuration) takes precedence over "path-config"
    when it is a non empty object, array or string. An object or array is
    written as JSON. The configuration can not be changed while connected.

    @param config JSON representation as string
    @return True on success, false on failure. On failure nothing is changed.
  */
  virtual bool initFromJson(const std::string &config);

  virtual void setConnectionTimeout(uint32_t timeout);
  virtual uint32_t getConnectionTimeout(void);
  virtual void setResponseTimeout(uint32_t timeout);
  virtual uint32_t getResponseTimeout(void);

  /// Path to the driver
  std::string getDriverPath(void);

  /// Path to the configuration file (empty if embedded)
  std::string getConfigPath(void);

  /// Embedded configuration (empty if a file is used)
  std::string getEmbeddedConfig(void);

  /// True if the configuration is embedded
  bool isConfigEmbedded(void);

  /*!
    Path of the configuration file given to the driver. While connected
    with an embedded configuration this is the temporary file.
  */
  std::string getActiveConfigPath(void);

  /// Driver vendor string (VSCPGetVendorString) if the driver has one
  std::string getVendorString(void);

  /// Number of received events dropped because the receive queue was full
  uint64_t getDroppedEventCount(void) { return m_droppedEvents.load(); };

private:
  // Load the driver and resolve its entry points
  int loadDriver(void);

  // Unload the driver
  void unloadDriver(void);

  // Write the embedded configuration to a private temporary file
  int writeTempConfig(void);

  // Remove the temporary configuration file if there is one
  void removeTempConfig(void);

  // Receive thread
  void workerThread(void);

  // Deliver an event received from the driver
  void deliverEvent(vscpEvent &ev);

  // Serialise connect/disconnect and configuration changes
  std::mutex m_lifecycleMutex;

  // Held shared while calling into the driver from send(), and exclusive
  // while the driver is closed and unloaded
  std::shared_timed_mutex m_driverMutex;

  // Configuration
  std::string m_pathDriver;
  std::string m_pathConfig;
  std::string m_embeddedConfig;
  std::string m_configSuffix;
  uint8_t m_guid[16];
  uint32_t m_responseTimeout;
  uint32_t m_connectionTimeout;

  // Temporary configuration file (embedded configuration while connected)
  std::string m_tempConfigPath;

  // Driver
  void *m_hdll;
  long m_openHandle;
  LPFNDLL_VSCPOPEN m_procOpen;
  LPFNDLL_VSCPCLOSE m_procClose;
  LPFNDLL_VSCPWRITE m_procWrite;
  LPFNDLL_VSCPREAD m_procRead;
  LPFNDLL_VSCPGETVERSION m_procGetVersion;
  LPFNDLL_VSCPGETVENDORSTRING m_procGetVendorString;

  std::atomic<bool> m_bConnected;
  std::atomic<bool> m_bRun;
  std::thread m_workerThread;

  // Receive queue and filter
  std::mutex m_mutexReceiveQueue;
  std::condition_variable m_condReceiveQueue;
  std::deque<vscpEvent *> m_receiveQueue;
  vscpEventFilter m_filterIn;
  std::atomic<uint64_t> m_droppedEvents;
};

#endif // VSCPCLIENTLEVEL2_H__INCLUDED_
