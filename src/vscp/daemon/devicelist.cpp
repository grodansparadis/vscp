// DeviceList.cpp:
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2021 Ake Hedman, Grodans Paradis AB
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

#include "devicelist.h"

#define _POSIX

#ifndef WIN32
#include <unistd.h>
#endif

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <mosquitto.h>

#include <canal.h>
#include <devicethread.h>
#include <vscp_debug.h>
#include <vscphelper.h>
#include <guid.h>
#include <vscp.h>
#include <controlobject.h>

#include <json.hpp>  // Needs C++11  -std=c++11
#include <mustache.hpp>

// https://github.com/nlohmann/json
using json = nlohmann::json;

using namespace kainjow::mustache;


#ifndef FALSE
#define FALSE   0
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

    m_strName.clear();          // No Device Name
    m_strParameter.clear();     // No Parameters
    m_strPath.clear();          // No path
    m_DeviceFlags = 0;          // Default: No flags.
    m_driverLevel = 0;          // Standard Canal messages is the default

    m_mqtt_format = jsonfmt;    // JSON is default format

    m_mqtt_reconnect_delay = 2;
    m_mqtt_reconnect_delay_max = 30;
    m_mqtt_reconnect_exponential_backoff = false;

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
    m_proc_VSCPWrite              = NULL;
    m_proc_VSCPRead               = NULL;
    m_proc_VSCPGetVersion         = NULL;
    m_proc_VSCPGetVersion         = NULL;

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
                            (int)m_driverLevel,
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
        spdlog::get("logger")->info(
               "[Driver {}] Start - VSCP driver is disabled.",
               m_strName);
        return true;
    }

    // *****************************************
    //  Create the worker thread for the device
    // *****************************************

    if (pthread_create(&m_deviceThreadHandle, NULL, deviceThread, this)) {
        spdlog::get("logger")->error(
               "[Driver {}] - Unable to start the device thread.",
               m_strName.c_str());
        return false;
    }

    spdlog::get("logger")->info("[Driver {}] - Started VSCP device driver.", m_strName);
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
        spdlog::get("logger")->info(
               "Driver {}: Driver asked to stop operation.",
               m_strName);

        pthread_mutex_lock(&m_mutexdeviceThread);
        pthread_join(m_deviceThreadHandle, NULL);
        pthread_mutex_unlock(&m_mutexdeviceThread);

        spdlog::get("logger")->error(
               "CDeviceItem: Driver stopping. {}\n",
               m_strName);
    } else {
        if (!m_bEnable) {
            spdlog::get("logger")->info(
                   "[Driver {}] Stop - VSCP driver is disabled.",
                   m_strName);
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

bool CDeviceItem::sendEvent(vscpEvent *pev)
{
    int rv = 0;
    uint8_t *pbuf = NULL;   // Used for binary payload
    std::string strPayload;
    std::string strTopic;

    // Check pointer
    if (NULL == pev) {
        spdlog::get("logger")->error("ControlObject: sendEvent: Event is NULL pointer");
        return false;
    }

    // Send the event to the discovery routine
    m_pCtrlObj->discovery(pev);

    // Convert to configured format

    if ( m_mqtt_format == jsonfmt ) {

        if ( !vscp_convertEventToJSON(strPayload, pev) ) {
            spdlog::get("logger")->error("ControlObject: sendEvent: Failed to convert event to JSON");
            return false;
        }

        // If function is enable din configuration
        // add measurement info to JSON object
        //
        // "measurement" : {
        //     "value" : 1.23,
        //     "unit" : 0,
        //     "sensorindex" : 1,
        //     "zone" : 11,
        //     "subzone" : 22
        // }
        //

        if (vscp_isMeasurement(pev) && bJsonMeasurementAdd) {

            double value = 0;
            if (!vscp_getMeasurementAsDouble(&value, pev)) {
                spdlog::get("logger")->error("Driver: sendEvent: Failed to convert event to value.");
            } 
            else {
                try {
                    auto j = json::parse(strPayload);

                    j["measurement"]["value"] = value;
                    j["measurement"]["unit"] = vscp_getMeasurementUnit(pev);
                    j["measurement"]["sensorindex"] = vscp_getMeasurementSensorIndex(pev);
                    j["measurement"]["zone"] = vscp_getMeasurementZone(pev);
                    j["measurement"]["subzone"] = vscp_getMeasurementSubZone(pev);

                    if (bJsonMeasurementAdd) {
                        
                    }

                    strPayload = j.dump();
                }
                catch (...) {
                    spdlog::get("logger")->error("Driver: sendEvent: Failed to add measurement info to event.");
                }
            }
        } else  if (bJsonMeasurementAdd) {

        }

    } else if (m_mqtt_format == xmlfmt) {
        if ( !vscp_convertEventToXML(strPayload, pev) ) {
            spdlog::get("logger")->error("Driver: sendEvent: Failed to convert event to XML");
            return false;
        }
    } else if (m_mqtt_format == strfmt) {
        if ( !vscp_convertEventToString(strPayload, pev) ) {
            spdlog::get("logger")->error("ControlObject: sendEvent: Failed to convert event to STRING");
            return false;
        }
    } else if (m_mqtt_format == binfmt) {
        pbuf = new uint8_t[VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 + pev->sizeData];
        if (NULL == pbuf) {
            return false;
        }

        if (!vscp_writeEventToFrame(pbuf,
                                    VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 + pev->sizeData,
                                    VSCP_MULTICAST_TYPE_EVENT,
                                    pev) ) {
            spdlog::get("logger")->error("ControlObject: sendEvent: Failed to convert event to BINARY");
            return false;
        }
    }
    else {
        return VSCP_ERROR_NOT_SUPPORTED;
    }



    // Fix topic

    for (std::list<std::string>::const_iterator
            it = m_mqtt_publish.begin();
            it != m_mqtt_publish.end();
            ++it){

        std::string topic_template = *it;

        // Fix publish topics
        mustache subtemplate{topic_template};
        data data;
        cguid evguid(pev->GUID);    // Event GUID
        data.set("guid", evguid.getAsString());        
        for (int i=0; i<15; i++) {
            data.set(vscp_str_format("guid[%d]", i), vscp_str_format("%d", evguid.getAt(i)));
        }
        data.set("guid.msb", vscp_str_format("%d", evguid.getAt(0)));
        data.set("guid.lsb", vscp_str_format("%d", evguid.getMSB()));
        data.set("ifguid", m_guid.getAsString());

        if (NULL != pev->pdata) {
            for (int i=0; i<pev->sizeData; i++) {
                data.set(vscp_str_format("data[%d]", i), vscp_str_format("%d", pev->pdata[i]));
            }
        }

        for (int i=0; i<15; i++) {
            data.set(vscp_str_format("ifguid[%d]", i), vscp_str_format("%d", m_guid.getAt(i)));
        }
        data.set("nickname", vscp_str_format("%d", evguid.getNicknameID()));
        data.set("class", vscp_str_format("%d", pev->vscp_class));
        data.set("type", vscp_str_format("%d", pev->vscp_type));

        data.set("class-token", m_pCtrlObj->getTokenFromClassId(pev->vscp_class));
        data.set("type-token", m_pCtrlObj->getTokenFromTypeId(pev->vscp_class, pev->vscp_type));

        data.set("head", vscp_str_format("%d",pev->head));
        data.set("obid", vscp_str_format("%ul",pev->obid));
        data.set("timestamp", vscp_str_format("%ul",pev->timestamp));

        std::string dt;
        vscp_getDateStringFromEvent(dt,pev);
        data.set("datetime", dt);        
        data.set("year", vscp_str_format("%d",pev->year));
        data.set("month", vscp_str_format("%d",pev->month));
        data.set("day", vscp_str_format("%d",pev->day));
        data.set("hour", vscp_str_format("%d",pev->hour));
        data.set("minute", vscp_str_format("%d",pev->minute));
        data.set("second", vscp_str_format("%d",pev->second));
        
        data.set("clientid", m_mqtt_strClientId);
        data.set("user", m_mqtt_strUserName);
        data.set("host", m_mqtt_strHost);
        
        data.set("driver-name", m_strName);

        // guid-to-real   - GUID as realname
        // data-to-read   - Data to real

        strTopic = subtemplate.render(data);

        if (m_mqtt_format != binfmt) {
            rv = mosquitto_publish(m_mosq,
                                    NULL,
                                    strTopic.c_str(),
                                    strPayload.length(),
                                    strPayload.c_str(),
                                    m_mqtt_qos,
                                    m_mqtt_bRetain );
        }
        else {    
            // Binary format publish        
            rv = mosquitto_publish(m_mosq,
                                    NULL,
                                    strTopic.c_str(),
                                    VSCP_MULTICAST_PACKET0_HEADER_LENGTH + 2 + pev->sizeData,
                                    pbuf,
                                    m_mqtt_qos,
                                    true );
            //mosquitto_loop(m_mosq, 0, 1);                                             
        }

        // Translate mosquitto error code to VSCP error code
        switch (rv) {
            case MOSQ_ERR_INVAL:
                spdlog::get("logger")->error("ControlObject: sendEvent: Error Parameter");
                rv = false;
                break;
            case MOSQ_ERR_NOMEM:
                spdlog::get("logger")->error("ControlObject: sendEvent: Error Memory");
                rv = false;
                break;
            case MOSQ_ERR_NO_CONN:
                spdlog::get("logger")->error("ControlObject: sendEvent: Error Connection");
                rv = false;
                break;
            case MOSQ_ERR_PROTOCOL:
                spdlog::get("logger")->error("ControlObject: sendEvent: Error protocol");
                rv = false;
                return false;
            case MOSQ_ERR_PAYLOAD_SIZE:
                spdlog::get("logger")->error("ControlObject: sendEvent: Error payload size");
                rv = false;
                break;
            case MOSQ_ERR_MALFORMED_UTF8:
                spdlog::get("logger")->error("ControlObject: sendEvent: Error malformed utf8");
                rv = false;
                break;
#if defined(MOSQ_ERR_QOS_NOT_SUPPORTED)                  
            case MOSQ_ERR_QOS_NOT_SUPPORTED:
                spdlog::get("logger")->error("ControlObject: sendEvent: Error QOS not supported");
                rv = false;
                break;
#endif                
#if defined(MOSQ_ERR_OVERSIZE_PACKET)                
            case MOSQ_ERR_OVERSIZE_PACKET:
                spdlog::get("logger")->error("ControlObject: sendEvent: Error Oversized package");
                rv = false;
                break;
#endif                
        }

        // End if error
        if (!rv) break;

    }  // for

    // Clean up binary format buffer if used
    if (m_mqtt_format == binfmt) {                                 
        if (NULL != pbuf) {
            delete [] pbuf;
            pbuf = NULL;
        }
    }

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
    if (NULL == pDeviceItem) return false;

    if (NULL != pDeviceItem) {

        if ( vscp_fileExists(strPath) ) {
            m_devItemList.push_back(pDeviceItem);

            pDeviceItem->m_bEnable = true;
            pDeviceItem->m_pCtrlObj = pCtrlObj;

            pDeviceItem->m_driverLevel    = level;
            pDeviceItem->m_strName        = strName;
            pDeviceItem->m_strParameter   = strParameter;
            pDeviceItem->m_strPath        = strPath;
            pDeviceItem->m_guid           = guid;
            pDeviceItem->m_translation    = translation;

            // Set buffer sizes and flags
            pDeviceItem->m_DeviceFlags = flags;

        } else {
            spdlog::get("logger")->error(
                    "Driver '{}' is not available at this path {}. Dropped!",
                    strName,
                    strPath );

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
CDeviceList::getDeviceItemFromName(const std::string& name)
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

const char * interface_template = "{\
    \"interfaces\" : [%s]\
}";

std::string
CDeviceList::getAllAsJSON(void)
{
    std::string str;

    std::deque<CDeviceItem *>::iterator iter;
    for (iter = m_devItemList.begin(); iter != m_devItemList.end(); ++iter) {
        CDeviceItem *pItem = *iter;
        if (NULL != pItem) {
            str += pItem->getAsJSON();
            str += ",";
        }
    }

    str = vscp_str_format(interface_template, str.c_str());

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
                    }
                }
            }
        }
    }

    return count;
}