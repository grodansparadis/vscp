// deviceList.h: interface for the CDeviceList class.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2025 Ake Hedman, the VSCP project
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

#if !defined(_DEVICELIST_H__0ED35EA7_E9E1_41CD_8A98_5EB3369B3194__INCLUDED_)
#define _DEVICELIST_H__0ED35EA7_E9E1_41CD_8A98_5EB3369B3194__INCLUDED_

#include <pthread.h>
#include <semaphore.h>

#include <canaldlldef.h>
#include <vscp.h>
#include <vscp-client-mqtt.h>
//#include <devicethread.h>
#include <guid.h>
#include <level2drvdef.h>

#include <deque>
#include <list>
#include <map>
#include <string>

#define NO_TRANSLATION 0 // No translation bit set

// Out - translation bit definitions
#define VSCP_DRIVER_OUT_TR_M1_M2F 0x01 // Level 1 measurement -> Level II measurement Float
#define VSCP_DRIVER_OUT_TR_M1_M2S 0x02 // Level I measurement -> Level II measurement String
#define VSCP_DRIVER_OUT_TR_ALL_L2 0x04 // All Level I events to Level I over level II events

// In - translation bit definitions

enum _driver_levels { VSCP_DRIVER_LEVEL1 = 1, VSCP_DRIVER_LEVEL2 };

class CClientItem;
class cguid;
class CControlObject;

///////////////////////////////////////////////////////////////////////////////
// CDeviceItem
//

class CDeviceItem {

public:
  /// Constructor
  CDeviceItem();

  /// Destructor
  virtual ~CDeviceItem();

  /*!
      Get driver info as string
      "bEnable,bActive,name,path,param,level,flags,guid,translation"
      @return Driver info
  */
  std::string getAsString(void);

  /*!
      Get driver info as a JSON object
      {
          "enable"      : true|false,
          "active"      : true:false,
          "name"        : "drivername",
          "path"        : "path to driver",
          "param"       : "Driver parameters",
          "level"       : 1/2,
          "flags"       : numerical,
          "guid"        : "guid on string form",
          "translation" : 1/2/3
      }
      @return Driver info on JSON format
  */
  std::string getAsJSON(void);

  /*!
      Send VSCP event to broker on all
      topics defined
  */
  bool sendEvent(vscpEvent *pev);

  /*!
      Start driver
      @param Pointer to control object
      @return true on success, false on failure
  */
  bool startDriver(CControlObject *pCtrlObject);

  /*!
      Pause driver
      @return true on success, false on failure
  */
  bool pauseDriver(void);

  /*!
      Resume driver
      @return true on success, false on failure
  */
  bool resumeDriver(void);

  /*!
      Stop driver
      @return true on success, false on failure
  */
  bool stopDriver(void);

public:
  // ------------------------------------------------------------------------
  //                         Configuration block
  // All values here points to corresponding values in
  // ControlObject
  // ------------------------------------------------------------------------

  CControlObject *m_pCtrlObj;

  // ------------------------------------------------------------------------

  // Name of device
  std::string m_strName;

  // Device configuration string for Level I drivers.
  // Oath to XML config file for level II drivers.
  std::string m_strParameter;

  // Driver DLL/DL path
  std::string m_strPath;

  // Canal/VSCP Driver Level
  uint8_t m_driverLevel;

  // True if driver should be started.
  bool m_bEnable;

  // Paused driver is inactive
  bool m_bActive;

  // termination control
  bool m_bQuit;

  /*!
      GUID to use for driver interface if set
      two msb should be zero for this GUID
  */
  cguid m_guid;

  // Worker thread for device
  pthread_t m_deviceThreadHandle;
  pthread_mutex_t m_mutexdeviceThread;

  // Device flags for CANAL DLL open
  uint32_t m_DeviceFlags;

  // Mutex handle that is used for sharing of the device.
  pthread_mutex_t m_deviceMutex;

  /*!
   *  Translation flags
   *  High 16-bits incoming.
   *  Low 16-bit outgoing.
   */
  uint32_t m_translation;

  /*!
      Added measurement objet to
      json output if event is
      measurement and this is
      configured.
  */
  bool bJsonMeasurementAdd;

  // Handle for dll/dl driver interface
  long m_openHandle;

  // ------------------------------------------------------------------------
  //                             input Queue
  // ------------------------------------------------------------------------

  // Input Queue
  std::deque<vscpEvent *> m_inputQueue;

  // Semaphore to indicate that an event has been received
  sem_t m_seminputQueue;

  // Mutex handle that is used for sharing of the client object
  pthread_mutex_t m_mutexinputQueue;

  // ------------------------------------------------------------------------
  //                                 MQTT
  // ------------------------------------------------------------------------

  /*!
    VSCP MQTT client
  */
  vscpClientMqtt m_mqttClient;

  /// Configuration
  json m_mqttConfig;

  // Level I (CANAL) driver methods
  LPFNDLL_CANALOPEN m_proc_CanalOpen;
  LPFNDLL_CANALCLOSE m_proc_CanalClose;
  LPFNDLL_CANALGETLEVEL m_proc_CanalGetLevel;
  LPFNDLL_CANALSEND m_proc_CanalSend;
  LPFNDLL_CANALRECEIVE m_proc_CanalReceive;
  LPFNDLL_CANALDATAAVAILABLE m_proc_CanalDataAvailable;
  LPFNDLL_CANALGETSTATUS m_proc_CanalGetStatus;
  LPFNDLL_CANALGETSTATISTICS m_proc_CanalGetStatistics;
  LPFNDLL_CANALSETFILTER m_proc_CanalSetFilter;
  LPFNDLL_CANALSETMASK m_proc_CanalSetMask;
  LPFNDLL_CANALSETBAUDRATE m_proc_CanalSetBaudrate;
  LPFNDLL_CANALGETVERSION m_proc_CanalGetVersion;
  LPFNDLL_CANALGETDLLVERSION m_proc_CanalGetDllVersion;
  LPFNDLL_CANALGETVENDORSTRING m_proc_CanalGetVendorString;

  // Generation 2
  LPFNDLL_CANALBLOCKINGSEND m_proc_CanalBlockingSend;
  LPFNDLL_CANALBLOCKINGRECEIVE m_proc_CanalBlockingReceive;
  LPFNDLL_CANALGETDRIVERINFO m_proc_CanalGetdriverInfo;

  // Level II driver methods
  LPFNDLL_VSCPOPEN m_proc_VSCPOpen;
  LPFNDLL_VSCPCLOSE m_proc_VSCPClose;
  LPFNDLL_VSCPWRITE m_proc_VSCPWrite;
  LPFNDLL_VSCPREAD m_proc_VSCPRead;
  LPFNDLL_VSCPGETVERSION m_proc_VSCPGetVersion;

  // ------------------------------------------------------------------------
};

// ----------------------------------------------------------------------------

class CDeviceList {
public:
  CDeviceList();
  virtual ~CDeviceList();

  /*!
      Add one driver item
      @param pdebugflags Pointer to configured debug flags
      @param strName Driver name
      @param strParameters Driver configuration string
      @param flags Driver flags
      @param guid Interface GUID
      @param level Mark as Level I or Level II driver
      @param translation Bits to set translations to be performed.
      @return True is returned if the driver was successfully added.
  */
  bool addItem(CControlObject *pCtrlObj,
               const std::string &strName,
               const std::string &strParameters,
               const std::string &strPath,
               uint32_t flags,
               const cguid &guid,
               uint8_t level        = VSCP_DRIVER_LEVEL1,
               uint32_t translation = NO_TRANSLATION);

  /*!
      Remove a driver item
      @param clientid for the driver to remove
      @return True if driver was removed successfully
              otherwise false.
  */
  bool removeItem(uint16_t id);

  /*!
      Get device from it's name
      @param name Name of device.
      @return Pointer to a device item or NULL if not found.
  */
  CDeviceItem *getDeviceItemFromName(const std::string &name);
  /*!
      Get device item from GUID
      @param guid for device to look for
      @return Pointer to device item or NULL if not found.
  */
  CDeviceItem *getDeviceItemFromGUID(cguid &guid);

  /*!
      Get all drivers as a string
      @return String with device item info lines separated
      with \r\n
  */
  std::string getAllAsString(void);

  /*!
      Get drivers on JSON format
      @return all drivers as a JSON object
  */
  std::string getAllAsJSON(void);

  /*!
      Count number of drivers
      @param type Type of driver to count (1/2/3) or all (0)
      @param bOnlyActive True if only enabled drivers should be counted.
      @return number of drivers
  */
  uint16_t getCountDrivers(uint8_t type = 0, bool bOnlyActive = false);

public:
  /*!
      List with devices
  */
  std::deque<CDeviceItem *> m_devItemList;
};

#endif // !defined(_DEVICELIST_H__0ED35EA7_E9E1_41CD_8A98_5EB3369B3194__INCLUDED_)
