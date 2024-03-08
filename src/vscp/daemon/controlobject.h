// ControlObject.h: interface for the CControlObject class.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2024 Ake Hedman, the VSCP project
// <info@vscp.org>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#if !defined(CONTROLOBJECT_H__INCLUDED_)
#define CONTROLOBJECT_H__INCLUDED_

#include <sqlite3.h>

#include <devicelist.h>
#include <mqtt.h>
#include <vscp.h>
#include <vscpmqtt.h>

#include <map>
#include <set>

#include <nlohmann/json.hpp> // Needs C++11  -std=c++11
#include <mustache.hpp>

#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/spdlog.h"

// https://github.com/nlohmann/json
using json = nlohmann::json;

using namespace kainjow::mustache;

// Needed on Linux
#ifndef VSCPMIN
#define VSCPMIN(X, Y) ((X) < (Y) ? (X) : (Y))
#endif

#ifndef VSCPMAX
#define VSCPMAX(a, b)                                                                                                  \
  ({                                                                                                                   \
    __typeof__(a) _a = (a);                                                                                            \
    __typeof__(b) _b = (b);                                                                                            \
    _a > _b ? _a : _b;                                                                                                 \
  })
#endif

#define VSCP_MAX_DEVICES 1024 // abs. max. is 0xffff

/*!
    This is the class that does the main work in the daemon.
*/

class CControlObject {
public:
  /*!
      Constructor
   */
  CControlObject(void);

  /*!
      Destructor
   */
  virtual ~CControlObject(void);

  /*!
      Get server capabilities (64-bit array)
      @param pCapability Pointer to 64 bit capabilities array
      @return True on success.
   */
  bool getVscpCapabilities(uint8_t *pCapability);

  /*!
      General initialisation
      @param strcfgfile Path to configuration file location
      @param rootFolder Path to VSCP system root folder
      @return True on success, false on failure
   */
  bool init(std::string &strcfgfile, std::string &rootFolder);

  /*!
      Initialize MQTT sub system
      @return True on success, false on failure
  */
  bool init_mqtt(void);

  /*!
      Clean up used resources
   */
  bool cleanup(void);

  /*!
      The main worker thread
   */
  bool run(void);

  /*!
      Send event on all publishing channels
      @param pex Pointer to VSCP event ex
      @return true on success, false on failure
  */
  bool sendEvent(vscpEventEx *pex);

  /*!
      Handle and send periodic events
      (previous automation)
      @return true on success, false on failure
  */
  bool periodicEvents(void);

  /*!
      Start worker threads for devices
      @return true on success, false on failure
   */
  bool startDeviceWorkerThreads(void);

  /*!
      Stop worker threads for devices
      @return true on success, false on failure
   */
  bool stopDeviceWorkerThreads(void);

  /*!
      Get device address for primary ethernet adapter

      @param guid class
      @return true on success, false on failure
   */
  bool getMacAddress(cguid &guid);

  /*!
      Get the first IP address computer is known under

      @param pGUID Pointer to GUID class
      @return true on success, false on failure
   */
  bool getIPAddress(cguid &guid);

  /*!
      Read configuration data
      @param strcfgfile path to configuration file.
      @return Returns true on success false on failure.
   */
  bool readConfiguration(const std::string &strcfgfile);

  /*!
      Read the loaded JSON configuration
      @param j JSON object
      @return Returns true on success false on failure.
  */
  bool readJSON(const json &j);

  /*!
      Read the encryption key from a safe location
      @param path Path to file containing the 256 byte key as a
          comma separated value list
      @return true on success, false on failure.
  */
  bool readEncryptionKey(const std::string &path);

  /*!
      Discovery routine

      Collects GUID's in a database and allow them
      to be named.

      @param pev Pointer to VSCP event
  */
  void discovery(vscpEvent *pev);

  /*!
      Get VSCP class token from id

      @param id VSCP class
      @return VSCP class token as string
  */
  std::string getTokenFromClassId(uint16_t id) { return m_map_class_id2Token[id]; }

  /*!
      Get VSCP type token from class and type id

      @param cid VSCP class
      @param tid VSCP class
      @return VSCP class token as string
  */
  std::string getTokenFromTypeId(uint16_t cid, uint16_t tid) { return m_map_type_id2Token[((cid << 16) + tid)]; }

public:

  // Will quit if set to true
  bool m_bQuit;

  // User configurable server name
  std::string m_strServerName;

  // VSCP daemon root folder
  std::string m_rootFolder;

  /*!
      User to run as for Unix
      if not ""
  */
  std::string m_runAsUser;

  // GUID for this server
  // Set to MAC address if not set explicit
  cguid m_guid;

  /*!
      Enabling bEventOutLookup will add clear
      text VSCP event information to events in
      the following fields

      vscpClassToken
      vscpTypeToken

      This is not standard.
  */
  bool bEventOutLookup;

  //**************************************************************************
  //                                  DEBUG
  //**************************************************************************

  /*!
   * Debug flags
   * See vscp_debug.h for possible flags.
   */
  // uint64_t m_debugFlags;

  // replaces with gDebugLevel

  //**************************************************************************
  //                                DATABASE
  //**************************************************************************

  /*!
      Path to class/type definition database
  */
  std::string m_pathClassTypeDefinitionDb;

  std::map<uint16_t, std::string> m_map_class_id2Token; // vscp_class -> class_token
  std::map<std::string, uint16_t> m_map_class_token2Id; // class_token -> vscp_class

  std::map<uint32_t, std::string> m_map_type_id2Token; // ((vscp_class << 16) + vscp_type) -> type_token
  std::map<std::string, uint32_t> m_map_type_token2Id; // type_token -> ((vscp_class << 16) + vscp_type)

  /*!
      Path to discovery database
      Set empty to disable functionality
  */
  std::string m_pathMainDb;
  sqlite3 *m_db_vscp_daemon;

  std::map<std::string, std::string> m_map_discoveryGuidToName; // key = GUID, value = name

  //**************************************************************************
  //                                 DRIVERS
  //**************************************************************************

  // The list with available devices.
  CDeviceList m_deviceList;
  pthread_mutex_t m_mutex_DeviceList;

  // Mutex for device queue
  pthread_mutex_t m_mutex_deviceList;

  //**************************************************************************
  //                            LOGGER (SPDLOG)
  //**************************************************************************

  bool m_bEnableFileLog;
  spdlog::level::level_enum m_fileLogLevel;
  std::string m_fileLogPattern;
  std::string m_path_to_log_file;
  uint32_t m_max_log_size;
  uint16_t m_max_log_files;

  bool m_bEnableConsoleLog;
  spdlog::level::level_enum m_consoleLogLevel;
  std::string m_consoleLogPattern;

  //**************************************************************************
  //                                  MQTT
  //**************************************************************************

  /*!
    VSCP MQTT client
  */
  vscpClientMqtt m_mqttClient;

  json m_mqttConfig;

  /*!
   Base topic for VSCP daemon info. Should end with slash
 */
  std::string m_topicDaemonBase; 

  /*!
    Topic under daemon base which the dameon publish it's drivers
    Default is 'daemon-base'/drivers
  */
  std::string m_topicDrivers; 
  /*!
    Topic under daemon base on which the dameon publish it's new node discoveries
    Default is 'daemon-base'/discovery
  */
  std::string m_topicDiscovery;  

private:
  //struct mosquitto *m_mosq; // Handel for MQTT connection
};

#endif // !defined(CONTROLOBJECT_H__7D80016B_5EFD_40D5_94E3_6FD9C324CC7B__INCLUDED_)
