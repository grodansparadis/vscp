// DeviceList.cpp:
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

#define _POSIX

#ifdef WIN32
#include <pch.h>
#else
#include <unistd.h>
#endif

#include "devicelist.h"




#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <canal.h>
#include <controlobject.h>
#include <devicethread.h>
#include <guid.h>
#include <vscp.h>
#include <vscp-client-mqtt.h>
#include <vscp-debug.h>
#include <vscphelper.h>

#include <nlohmann/json.hpp> // Needs C++11  -std=c++11
#include <mustache.hpp>

// https://github.com/nlohmann/json
using json = nlohmann::json;

using namespace kainjow::mustache;

#ifndef FALSE
#define FALSE 0
#endif

///////////////////////////////////////////////////
//                 GLOBALS
///////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction CDeviceList
//////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CDeviceItem -- Constructor
//

CDeviceItem::CDeviceItem()
{
  m_bQuit   = false;
  m_bEnable = false; // Default is that driver should not be started
  m_bActive = true;  // Not paused

  m_translation = NO_TRANSLATION; // Default is no translation

  bJsonMeasurementAdd = false;

  m_strName.clear();      // No Device Name
  m_strParameter.clear(); // No Parameters
  m_strPath.clear();      // No path
  m_DeviceFlags = 0;      // Default: No flags.
  m_driverLevel = 0;      // Standard Canal messages is the default

  // m_mqtt_format = jsonfmt; // JSON is default format

  // m_mqtt_reconnect_delay               = 2;
  // m_mqtt_reconnect_delay_max           = 30;
  // m_mqtt_reconnect_exponential_backoff = false;

  // VSCP Level I
  m_proc_CanalOpen            = NULL;
  m_proc_CanalClose           = NULL;
  m_proc_CanalGetLevel        = NULL;
  m_proc_CanalSend            = NULL;
  m_proc_CanalDataAvailable   = NULL;
  m_proc_CanalReceive         = NULL;
  m_proc_CanalGetStatus       = NULL;
  m_proc_CanalGetStatistics   = NULL;
  m_proc_CanalSetFilter       = NULL;
  m_proc_CanalSetMask         = NULL;
  m_proc_CanalSetBaudrate     = NULL;
  m_proc_CanalGetVersion      = NULL;
  m_proc_CanalGetDllVersion   = NULL;
  m_proc_CanalGetVendorString = NULL;

  // Generation 2
  m_proc_CanalBlockingSend    = NULL;
  m_proc_CanalBlockingReceive = NULL;
  m_proc_CanalGetdriverInfo   = NULL;

  // VSCP Level II
  m_proc_VSCPOpen       = NULL;
  m_proc_VSCPClose      = NULL;
  m_proc_VSCPWrite      = NULL;
  m_proc_VSCPRead       = NULL;
  m_proc_VSCPGetVersion = NULL;
  m_proc_VSCPGetVersion = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// ~CDeviceItem -- Destructor

CDeviceItem::~CDeviceItem(void)
{
  /*if ( NULL != m_pDriver3Process ) {
      m_pDriver3Process->Kill( m_pid );
      delete m_pDriver3Process;
      m_pDriver3Process = NULL;
  }*/
}

///////////////////////////////////////////////////////////////////////////////
// getAsString
//
// bEnable,bActive,name,path,param,level,flags,guid,translation
//

std::string
CDeviceItem::getAsString(void)
{
  std::string str;

  str = m_bEnable ? "true," : "false,";
  str += m_bActive ? "true," : "false,";
  str += m_strName + ",";
  str += m_strPath + ",";
  str += m_strParameter + ",";
  str += vscp_str_format("%d,%ul,", (int) m_driverLevel, m_DeviceFlags);
  str += m_guid.getAsString() + ",";
  str += vscp_str_format("%04X", m_translation);

  return str;
}

///////////////////////////////////////////////////////////////////////////////
// getAsJSON
//

const char *JSON_DRV_TEMPLATE = "{    \
            \"enable\"      : %s,     \
            \"active\"      : %s,     \
            \"name\"        : \"%s\", \
            \"path\"        : \"%s\", \
            \"param\"       : \"%s\", \
            \"level\"       : %d,     \
            \"flags\"       : %lu,    \
            \"guid\"        : \"%s\", \
            \"translation\" : %lu     \
        }";

std::string
CDeviceItem::getAsJSON(void)
{
  std::string str;
  str = vscp_str_format(JSON_DRV_TEMPLATE,
                        m_bEnable ? "true" : "false",
                        m_bActive ? "true" : "false",
                        m_strName.c_str(),
                        m_strPath.c_str(),
                        m_strParameter.c_str(),
                        (int) m_driverLevel,
                        m_DeviceFlags,
                        m_guid.getAsString().c_str(),
                        m_translation);
  return str;
}

///////////////////////////////////////////////////////////////////////////////
// startDriver
//
bool
CDeviceItem::startDriver(CControlObject *pCtrlObject)
{
  // Just start if enabled
  if (!m_bEnable) {
    spdlog::info("[Driver {}] Start - VSCP driver is disabled.", m_strName);
    return true;
  }

  // *****************************************
  //  Create the worker thread for the device
  // *****************************************

  if (pthread_create(&m_deviceThreadHandle, NULL, deviceThread, this)) {
    spdlog::error("[Driver {}] - Unable to start the device thread.", m_strName.c_str());
    return false;
  }

  spdlog::info("[Driver {}] - Started VSCP device driver.", m_strName);
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// stopDriver
//

bool
CDeviceItem::stopDriver()
{
  if (m_bEnable) {
    m_bQuit = true;
    spdlog::info("Driver {}: Driver asked to stop operation.", m_strName);

    pthread_mutex_lock(&m_mutexdeviceThread);
    pthread_join(m_deviceThreadHandle, NULL);
    pthread_mutex_unlock(&m_mutexdeviceThread);

    spdlog::info("CDeviceItem: Driver stopping. {}\n", m_strName);
  }
  else {
    if (!m_bEnable) {
      spdlog::info("[Driver {}] Stop - VSCP driver is disabled.", m_strName);
      return true;
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// pauseDriver
//

bool
CDeviceItem::pauseDriver(void)
{
  m_bActive = false;
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// resumeDriver
//

bool
CDeviceItem::resumeDriver(void)
{
  m_bActive = true;
  return true;
}

/////////////////////////////////////////////////////////////////////////////
// sendEvent
//

bool
CDeviceItem::sendEvent(vscpEvent *pev)
{
  int rv        = 0;
  uint8_t *pbuf = NULL; // Used for binary payload
  std::string strPayload;
  std::string strTopic;

  // Check pointer
  if (NULL == pev) {
    spdlog::error("ControlObject: sendEvent: Event is NULL pointer");
    return false;
  }

  // Send the event to the discovery routine
  m_pCtrlObj->discovery(pev);

  // Send the event
  m_mqttClient.send(*pev);

  return (0 == rv);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction CDeviceList
//////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CDeviceList - Constructor
//

CDeviceList::CDeviceList(void)
{
  ;
}

///////////////////////////////////////////////////////////////////////////////
// ~CDeviceList Destructor

CDeviceList::~CDeviceList(void)
{
  std::deque<CDeviceItem *>::iterator iter;
  for (iter = m_devItemList.begin(); iter != m_devItemList.end(); ++iter) {
    CDeviceItem *pItem = *iter;
    if (pItem)
      delete pItem;
  }

  m_devItemList.clear();
}

///////////////////////////////////////////////////////////////////////////////
// addItem
//
// An one device to the list
//

bool
CDeviceList::addItem(CControlObject *pCtrlObj,
                     const std::string &strName,
                     const std::string &strParameter,
                     const std::string &strPath,
                     uint32_t flags,
                     const cguid &guid,
                     uint8_t level,
                     uint32_t translation)
{
  bool rv = true;

  CDeviceItem *pDeviceItem = new CDeviceItem();
  if (NULL == pDeviceItem) {
    return false;
  }

  if (vscp_fileExists(strPath)) {
    m_devItemList.push_back(pDeviceItem);

    pDeviceItem->m_bEnable  = true;
    pDeviceItem->m_pCtrlObj = pCtrlObj;

    pDeviceItem->m_driverLevel  = level;
    pDeviceItem->m_strName      = strName;
    pDeviceItem->m_strParameter = strParameter;
    pDeviceItem->m_strPath      = strPath;
    pDeviceItem->m_guid         = guid;
    pDeviceItem->m_mqttClient.setIfGuid(guid);
    pDeviceItem->m_mqttClient.setSrvGuid(pCtrlObj->m_guid);
    pDeviceItem->m_translation  = translation;

    // Set buffer sizes and flags
    pDeviceItem->m_DeviceFlags = flags;
  }
  else {
    if (nullptr == spdlog::get("logger")) {
      fprintf(stderr, "Driver '%s' is not available at this path %s. Dropped!", strName.c_str(), strPath.c_str());
    }
    else {
      spdlog::error("Driver '{}' is not available at this path {}. Dropped!", strName, strPath);
    }

    // Driver does not exist at this path
    delete pDeviceItem;
    rv = false;
  }

  return rv;
}

///////////////////////////////////////////////////////////////////////////////
// removeItem
//

bool
CDeviceList::removeItem(uint16_t id)
{
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// getDeviceItemFromGUID
//

CDeviceItem *
CDeviceList::getDeviceItemFromGUID(cguid &guid)
{
  CDeviceItem *returnItem = NULL;

  std::deque<CDeviceItem *>::iterator iter;
  for (iter = m_devItemList.begin(); iter != m_devItemList.end(); ++iter) {
    CDeviceItem *pItem = *iter;
    if (pItem->m_guid == guid) {
      returnItem = pItem;
      break;
    }
  }

  return returnItem;
}

///////////////////////////////////////////////////////////////////////////////
// getDeviceItemFromName
//

CDeviceItem *
CDeviceList::getDeviceItemFromName(const std::string &name)
{
  CDeviceItem *returnItem = NULL;

  std::deque<CDeviceItem *>::iterator iter;
  for (iter = m_devItemList.begin(); iter != m_devItemList.end(); ++iter) {
    CDeviceItem *pItem = *iter;
    if (pItem->m_strName == name) {
      returnItem = pItem;
      break;
    }
  }

  return returnItem;
}

///////////////////////////////////////////////////////////////////////////////
// getAllAsString
//

std::string
CDeviceList::getAllAsString(void)
{
  std::string str;

  std::deque<CDeviceItem *>::iterator iter;
  for (iter = m_devItemList.begin(); iter != m_devItemList.end(); ++iter) {
    CDeviceItem *pItem = *iter;
    if (NULL != pItem) {
      str += pItem->getAsString() + "\r\n";
    }
  }

  return str;
}

///////////////////////////////////////////////////////////////////////////////
// getAllAsJSON
//

const char *driver_template = "{\
    \"drivers\" : [%s]\
}";

std::string
CDeviceList::getAllAsJSON(void)
{
  std::string str;

  std::deque<CDeviceItem *>::iterator iter;
  for (iter = m_devItemList.begin(); iter != m_devItemList.end(); ++iter) {
    CDeviceItem *pItem = *iter;
    if (NULL != pItem) {
      if (str.length())
        str += ",";
      str += pItem->getAsJSON();
    }
  }

  str = vscp_str_format(driver_template, str.c_str());

  return str;
}

///////////////////////////////////////////////////////////////////////////////
// getCountDrivers
//

uint16_t
CDeviceList::getCountDrivers(uint8_t type, bool bOnlyActive)
{
  uint16_t count = 0;

  std::deque<CDeviceItem *>::iterator iter;
  for (iter = m_devItemList.begin(); iter != m_devItemList.end(); ++iter) {
    CDeviceItem *pItem = *iter;
    if (NULL != pItem) {
      if (bOnlyActive) {
        if (pItem->m_bEnable) {
          switch (type) {
            case 0:
              count++;
              break;

            case VSCP_DRIVER_LEVEL1:
              if (pItem->m_driverLevel)
                count++;
              break;

            case VSCP_DRIVER_LEVEL2:
              if (pItem->m_driverLevel)
                count++;
              break;
          }
        }
      }
      else {
        if (pItem->m_bEnable) {
          switch (type) {
            case 0:
              count++;
              break;

            case VSCP_DRIVER_LEVEL1:
              if (pItem->m_driverLevel)
                count++;
              break;

            case VSCP_DRIVER_LEVEL2:
              if (pItem->m_driverLevel)
                count++;
              break;
          }
        }
      }
    }
  }

  return count;
}