// DeviceList.cpp:
//
// This file is part of the VSCP (http://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2019 Ake Hedman, Grodans Paradis AB
// <info@grodansparadis.com>
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
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>

#include "devicelist.h"
#include <canal.h>
#include <clientlist.h>
#include <controlobject.h>
#include <devicethread.h>
#include <dllist.h>
#include <guid.h>
#include <vscp.h>

///////////////////////////////////////////////////
//                 GLOBALS
///////////////////////////////////////////////////

extern CControlObject *gpobj;

Driver3Process::Driver3Process()
{
    // TODO;
}

Driver3Process::~Driver3Process()
{
    // TODO;
}

void
Driver3Process::OnTerminate(int pid, int status)
{
    // TODO
    // http://man7.org/linux/man-pages/man2/waitpid.2.html
    syslog(LOG_DEBUG, "[Diver Level III] - Terminating.");
}

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

    m_strName.clear();      // No Device Name
    m_strParameter.clear(); // No Parameters
    m_strPath.clear();      // No path
    m_DeviceFlags = 0;      // Default: No flags.
    m_driverLevel = 0;      // Standard Canal messages is the default

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
    m_proc_VSCPOpen               = NULL;
    m_proc_VSCPClose              = NULL;
    m_proc_VSCPBlockingSend       = NULL;
    m_proc_VSCPBlockingReceive    = NULL;
    m_proc_VSCPGetLevel           = NULL;
    m_proc_VSCPGetVersion         = NULL;
    m_proc_VSCPGetDllVersion      = NULL;
    m_proc_VSCPGetVendorString    = NULL;
    m_proc_VSCPGetdriverInfo      = NULL;
    m_proc_VSCPGetWebPageTemplate = NULL;
    m_proc_VSCPGetWebPageInfo     = NULL;
    m_proc_VSCPWebPageupdate      = NULL;

    // VSCP Level III
    m_pid = 0;
    // m_pDriver3Process = NULL;
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
    str += vscp_str_format("%d,%ul,", (int)m_driverLevel, m_DeviceFlags);
    str += m_interface_guid.getAsString() + ",";
    str += vscp_str_format("%04X", m_translation);

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
        syslog(LOG_INFO,
               "[Driver %s] Start - VSCP driver is disabled.",
               m_strName.c_str());
        return true;
    }

    // *****************************************
    //  Create the worker thread for the device
    // *****************************************

    // Share control object
    m_pCtrlObject = pCtrlObject;

    if (pthread_create(&m_deviceThreadHandle, NULL, deviceThread, this)) {
        syslog(LOG_CRIT,
               "[Driver %s] - Unable to start the device thread.",
               m_strName.c_str());
        return false;
    }

    syslog(
      LOG_INFO, "[Driver %s] - Started VSCP device driver.", m_strName.c_str());

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
        syslog(LOG_INFO,
               "Driver %s: Driver asked to stop operation.",
               m_strName.c_str());

        pthread_mutex_lock(&m_mutexdeviceThread);
        pthread_join(m_deviceThreadHandle, NULL);
        pthread_mutex_unlock(&m_mutexdeviceThread);

        syslog(LOG_ERR,
               "CDeviceItem: Driver stopping. [%s]\n",
               (const char *)m_strName.c_str());
    } else {
        if (!m_bEnable) {
            syslog(LOG_INFO,
                   "[Driver %s] Stop - VSCP driver is disabled.",
                   m_strName.c_str());
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
        if (pItem) delete pItem;
    }

    m_devItemList.clear();
}

///////////////////////////////////////////////////////////////////////////////
// addItem
//
// An one device to the list
//

bool
CDeviceList::addItem(const std::string &strName,
                     const std::string &strParameter,
                     const std::string &strPath,
                     uint32_t flags,
                     const cguid &guid,
                     uint8_t level,
                     bool bEnable,
                     uint32_t translation)
{
    bool rv                  = true;
    CDeviceItem *pDeviceItem = new CDeviceItem();
    if (NULL == pDeviceItem) return false;

    if (NULL != pDeviceItem) {

        if (vscp_fileExists(strPath)) {

            m_devItemList.push_back(pDeviceItem);

            pDeviceItem->m_bEnable = bEnable;

            pDeviceItem->m_driverLevel    = level;
            pDeviceItem->m_strName        = strName;
            pDeviceItem->m_strParameter   = strParameter;
            pDeviceItem->m_strPath        = strPath;
            pDeviceItem->m_interface_guid = guid;

            // Set buffer sizes and flags
            pDeviceItem->m_DeviceFlags = flags;

        } else {
            // Driver does not exist at this path
            delete pDeviceItem;
            rv = false;
        }
    }

    return rv;
}

///////////////////////////////////////////////////////////////////////////////
// removeItem
//

bool
CDeviceList::removeItem(unsigned long id)
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
        if (pItem->m_interface_guid == guid) {
            returnItem = pItem;
            break;
        }
    }

    return returnItem;
}

///////////////////////////////////////////////////////////////////////////////
// getDeviceItemFromClientId
//

CDeviceItem *
CDeviceList::getDeviceItemFromClientId(uint32_t id)
{
    CDeviceItem *returnItem = NULL;

    std::deque<CDeviceItem *>::iterator iter;
    for (iter = m_devItemList.begin(); iter != m_devItemList.end(); ++iter) {
        CDeviceItem *pItem = *iter;
        if ((NULL != pItem->m_pClientItem) &&
            (pItem->m_pClientItem->m_clientID == id)) {
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
                            if (pItem->m_driverLevel) count++;
                            break;

                        case VSCP_DRIVER_LEVEL2:
                            if (pItem->m_driverLevel) count++;
                            break;

                        case VSCP_DRIVER_LEVEL3:
                            if (pItem->m_driverLevel) count++;
                            break;
                    }
                }
            } else {
                if (pItem->m_bEnable) {
                    switch (type) {
                        case 0:
                            count++;
                            break;

                        case VSCP_DRIVER_LEVEL1:
                            if (pItem->m_driverLevel) count++;
                            break;

                        case VSCP_DRIVER_LEVEL2:
                            if (pItem->m_driverLevel) count++;
                            break;

                        case VSCP_DRIVER_LEVEL3:
                            if (pItem->m_driverLevel) count++;
                            break;
                    }
                }
            }
        }
    }
}