// Lmsensors.cpp: implementation of the Clmsensors class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP Project (http://www.vscp.org)
//
// Copyright (C) 2000-2019 Ake Hedman,
// Grodans Paradis AB, <akhe@grodansparadis.com>
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
// CPU load
// http://stackoverflow.com/questions/3017162/how-to-get-total-cpu-usage-in-linux-c
//
// Memory usage
// http://stackoverflow.com/questions/669438/how-to-get-memory-usage-at-run-time-in-c
//

#include <deque>
#include <list>
#include <string>

#include <limits.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>

#include <expat.h>

#include <vscp.h>
#include <vscp_class.h>
#include <vscp_type.h>
#include <vscphelper.h>
#include <vscpremotetcpif.h>

#include "lmsensors.h"
#include "vscpl2drv_lmsensors.h"

// Buffer for XML parser
#define XML_BUFF_SIZE 10000

// Forward declaration
void *workerThread(void *pData);

//////////////////////////////////////////////////////////////////////
// Clmsensors
//

Clmsensors::Clmsensors()
{
    m_bQuit = false;

    sem_init(&m_semSendQueue, 0, 0);
    sem_init(&m_semReceiveQueue, 0, 0);

    pthread_mutex_init(&m_mutexSendQueue, NULL);
    pthread_mutex_init(&m_mutexReceiveQueue, NULL);

    vscp_clearVSCPFilter(&m_vscpfilter); // Accept all events
}

//////////////////////////////////////////////////////////////////////
// ~Clmsensors
//

Clmsensors::~Clmsensors()
{
    // Signal that we are quitting
    close();

    // Terminate threads and deallocate objects
    std::deque<CWrkTreadObj *>::iterator it;
    for (it = m_objectList.begin(); it != m_objectList.end(); ++it) {
        CWrkTreadObj *pObj = *it;
        if (NULL != pObj) {

            // Wait for workerthread to quit
            pthread_join(pObj->m_pthreadWork, NULL);
            *it = NULL;

            // Delete the thread object
            delete pObj;
        }
    }

    // Clear the list
    m_objectList.clear();

    sem_destroy(&m_semSendQueue);
    sem_destroy(&m_semReceiveQueue);

    pthread_mutex_destroy(&m_mutexSendQueue);
    pthread_mutex_destroy(&m_mutexReceiveQueue);
}

// ----------------------------------------------------------------------------

/*
    XML Setup
    =========

    <setup>

    <sensor path="path-to-sensor-data"
            guid="GUID for sensor data"
            interval="Interval in seconds to report sensor data"
            class="VSCP class code for sensor data"
            type="VSCP type code for sensor data"
            index="Index to use for sensor data"
            zone="zone to use for sensor data"
            subzone="Subzone to use for sensor data"
            coding="Coding to use for sensor data"
            multiply="Multiply value to use for sensor data"
            divide="divide data to use for sensor data"
            offset="Offset for sensor data file read" />
    <sensor ....... />
    <sensor ....... />

    </setup>
*/

// ----------------------------------------------------------------------------

int depth_setup_parser = 0;

void
startSetupParser( void *data, const char *name, const char **attr )
{
    Clmsensors *pObj = (Clmsensors *)data;
    if (NULL == pObj) return;

    if ((0 == strcmp(name, "sensor")) && (1 == depth_setup_parser)) {

        CWrkTreadObj *pthreadObj = new CWrkTreadObj;
        if (NULL == pthreadObj) {
            return;
        }

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == strcasecmp(attr[i], "path")) {
                if (!attribute.empty()) {
                    pthreadObj->m_path = attribute;
                }
            } else if (0 == strcasecmp(attr[i], "filter")) {
                if (!attribute.empty()) {
                    if (!vscp_readFilterFromString(&pthreadObj->m_vscpfilter,
                                                   attribute)) {
                        syslog(LOG_ERR, "Unable to read event receive filter.");
                    }
                }
            } else if (0 == strcasecmp(attr[i], "mask")) {
                if (!attribute.empty()) {
                    if (!vscp_readMaskFromString(&pthreadObj->m_vscpfilter,
                                                 attribute)) {
                        syslog(LOG_ERR, "Unable to read event receive mask.");
                    }
                }
            } else if (0 == strcasecmp(attr[i], "guid")) {
                if (!attribute.empty()) {
                    pthreadObj->m_guid.getFromString(attribute);
                }
            } else if (0 == strcmp(attr[i], "interval")) {
                if (!attribute.empty()) {
                    pthreadObj->m_interval = vscp_readStringValue(attribute);
                }
            } else if (0 == strcmp(attr[i], "class")) {
                if (!attribute.empty()) {
                    pthreadObj->m_vscpclass = vscp_readStringValue(attribute);
                }
            } else if (0 == strcmp(attr[i], "vscpclass")) {
                if (!attribute.empty()) {
                    pthreadObj->m_vscpclass = vscp_readStringValue(attribute);
                }
            } else if (0 == strcmp(attr[i], "type")) {
                if (!attribute.empty()) {
                    pthreadObj->m_vscptype = vscp_readStringValue(attribute);
                }
            } else if (0 == strcmp(attr[i], "vscptype")) {
                if (!attribute.empty()) {
                    pthreadObj->m_vscptype = vscp_readStringValue(attribute);
                }
            } else if (0 == strcmp(attr[i], "index")) {
                if (!attribute.empty()) {
                    pthreadObj->m_index = vscp_readStringValue(attribute);
                }
            } else if (0 == strcmp(attr[i], "zone")) {
                if (!attribute.empty()) {
                    pthreadObj->m_zone = vscp_readStringValue(attribute);
                }
            } else if (0 == strcmp(attr[i], "subzone")) {
                if (!attribute.empty()) {
                    pthreadObj->m_subzone = vscp_readStringValue(attribute);
                }
            } else if (0 == strcmp(attr[i], "coding")) {
                if (!attribute.empty()) {
                    pthreadObj->m_datacoding = vscp_readStringValue(attribute);
                }
            } else if (0 == strcmp(attr[i], "unit")) {
                if (!attribute.empty()) {
                    pthreadObj->m_unit = vscp_readStringValue(attribute);
                }
            } else if (0 == strcmp(attr[i], "offset")) {
                if (!attribute.empty()) {
                    pthreadObj->m_readOffset = vscp_readStringValue(attribute);
                }
            } else if (0 == strcmp(attr[i], "multiply")) {
                if (!attribute.empty()) {
                    pthreadObj->m_multiplyValue =
                      vscp_readStringValue(attribute);
                }
            } else if (0 == strcmp(attr[i], "divide")) {
                if (!attribute.empty()) {
                    pthreadObj->m_divideValue = vscp_readStringValue(attribute);
                }
            }
        }

        pObj->m_objectList.push_back(pthreadObj);

    }

    depth_setup_parser++;
}

void
endSetupParser( void *data, const char *name )
{
    depth_setup_parser--;
}

// ----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////
// open
//
//

bool
Clmsensors::open(const char *pUsername,
                 const char *pPassword,
                 const char *pHost,
                 short port,
                 const char *pPrefix,
                 const char *pConfig)
{
    bool rv         = true;
    std::string str = std::string(pConfig);
    m_username      = std::string(pUsername);
    m_password      = std::string(pPassword);
    m_host          = std::string(pHost);
    m_port          = port;
    m_prefix        = std::string(pPrefix);

    // Parse the configuration string. It should
    // have the following form
    // path

    std::deque<std::string> tokens;
    vscp_split(tokens, str, ";");

    // Check for # of sensors in configuration string
    if (!tokens.empty()) {
        // Path
        m_nSensors = vscp_readStringValue(tokens.front());
        tokens.pop_front();
    }

    // First log on to the host and get configuration
    // variables

    if (VSCP_ERROR_SUCCESS !=
        m_srv.doCmdOpen(m_host, m_port, m_username, m_password)) {
        syslog(LOG_ERR,
               "Unable to connect to VSCP TCP/IP interface. "
               "Terminating!");
        return false;
    }

    // Find the channel id
    uint32_t ChannelID;
    m_srv.doCmdGetChannelID(&ChannelID);

    // The server should hold configuration data for each sensor
    // we want to monitor.
    //
    // We look for
    //
    //	 _numberofsensors - This is the number of sensors
    // 	   that the driver is supposed
    //	   to read. Sensors are numbered staring with zero.
    //
    //       _guidn - GUID for sensor n. guid0, guid1, guid2 etc
    //	 _pathn - Path to value for example
    //				/sys/class/hwmon/hwmon0/temp1_input
    //	 		  which is CPU temp for core 1
    //        on the system this is written at.
    //   _intervaln   - Interval in Seconds the event should be sent out.
    //						Zero disables.
    //   _vscptypen	  - VSCP_TYPE for CLASS=10 Measurement
    //   _confign	  - Coding for the value. See calss=10 in specification.
    //						measurement byte 0
    //   _dividen     - Divide value with this integer. Default is 1.
    //   _multiplyn   - Multiply value with this integer. Default is 1.
    //
    //

    // Get configuration data
    int varNumberOfSensors = 0;

    std::string strNumberOfSensors = m_prefix + std::string("_numberofsensors");

    if (VSCP_ERROR_SUCCESS !=
        m_srv.getRemoteVariableInt(strNumberOfSensors, &varNumberOfSensors)) {
        // The variable must be available - terminate
        syslog(LOG_ERR,
               "The variable prefix+NumberOfSensors is not "
               "available. Terminating!");
        // Close the channel
        m_srv.doCmdClose();
        return false;
    }

    if (0 == varNumberOfSensors) {
        syslog(LOG_ERR,
               "NumberOfSensors is zero. Must be at least one "
               "sensor. Terminating!");
        // Close the channel
        m_srv.doCmdClose();
        return false;
    }

    // Read in the configuration values for each sensor
    for (int i = 0; i < varNumberOfSensors; i++) {

        std::string strIteration;
        strIteration = vscp_str_format("%d", i);
        vscp_trim(strIteration);

        CWrkTreadObj *pthreadObj = new CWrkTreadObj;
        if (NULL != pthreadObj) {

            // Give object a link back to uss
            pthreadObj->m_pObj = this;

            // Get the path
            std::string strVariableName =
              m_prefix + std::string("_path") + strIteration;
            if (VSCP_ERROR_SUCCESS != m_srv.getRemoteVariableValue(
                                        strVariableName, pthreadObj->m_path)) {
                syslog(LOG_ERR,
                       "Failed to read variable _path. prefix=%s i=%d",
                       (const char *)m_prefix.c_str(),
                       i);
            }

            // Get GUID
            strVariableName = m_prefix + std::string("_guid") + strIteration;
            if (VSCP_ERROR_SUCCESS != m_srv.getRemoteVariableGUID(
                                        strVariableName, pthreadObj->m_guid)) {
                syslog(LOG_ERR,
                       "Failed to read variable _guid. prefix=%s i=%d",
                       (const char *)m_prefix.c_str(),
                       i);
            }

            // Get sample interval
            strVariableName =
              m_prefix + std::string("_interval") + strIteration;
            if (VSCP_ERROR_SUCCESS !=
                m_srv.getRemoteVariableInt(strVariableName,
                                           &pthreadObj->m_interval)) {
                syslog(LOG_ERR,
                       "Failed to read variable _interval. prefix=%s i=%d",
                       (const char *)m_prefix.c_str(),
                       i);
            }

            // Get VSCP type
            strVariableName =
              m_prefix + std::string("_vscptype") + strIteration;
            if (VSCP_ERROR_SUCCESS !=
                m_srv.getRemoteVariableInt(strVariableName,
                                           &pthreadObj->m_vscptype)) {
                syslog(LOG_ERR,
                       "Failed to read variable _vscptype. prefix=%s i=%d",
                       (const char *)m_prefix.c_str(),
                       i);
            }

            // Get measurement coding (first data byte)
            strVariableName =
              m_prefix + std::string("_datacoding") + strIteration;
            if (VSCP_ERROR_SUCCESS !=
                m_srv.getRemoteVariableInt(strVariableName,
                                           &pthreadObj->m_datacoding)) {
                syslog(LOG_ERR,
                       "Failed to read variable _coding. prefix=%s i=%d",
                       (const char *)m_prefix.c_str(),
                       i);
            }

            // Get divide value
            strVariableName = m_prefix + std::string("_divide") + strIteration;
            if (VSCP_ERROR_SUCCESS !=
                m_srv.getRemoteVariableDouble(strVariableName,
                                              &pthreadObj->m_divideValue)) {
                syslog(LOG_ERR,
                       "Failed to read variable _divide. prefix=%s i=%d",
                       (const char *)m_prefix.c_str(),
                       i);
            }

            // Get multiply value
            strVariableName =
              m_prefix + std::string("_multiply") + strIteration;
            if (VSCP_ERROR_SUCCESS !=
                m_srv.getRemoteVariableDouble(strVariableName,
                                              &pthreadObj->m_multiplyValue)) {
                syslog(LOG_ERR,
                       "Failed to read variable _multiply. prefix=%s i=%d",
                       (const char *)m_prefix.c_str(),
                       i);
            }

            // Get read offset
            strVariableName =
              m_prefix + std::string("_readoffset") + strIteration;
            if (VSCP_ERROR_SUCCESS !=
                m_srv.getRemoteVariableInt(strVariableName,
                                           &pthreadObj->m_readOffset)) {
                syslog(LOG_ERR,
                       "Failed to read variable _readoffset. prefix=%s i=%d",
                       (const char *)m_prefix.c_str(),
                       i);
            }

            // Get index
            strVariableName = m_prefix + std::string("_index") + strIteration;
            if (VSCP_ERROR_SUCCESS !=
                m_srv.getRemoteVariableInt(strVariableName,
                                           &pthreadObj->m_index)) {
                syslog(LOG_ERR,
                       "Failed to read variable _index. prefix=%s i=%d",
                       (const char *)m_prefix.c_str(),
                       i);
            }

            // Get zone
            strVariableName = m_prefix + std::string("_zone") + strIteration;
            if (VSCP_ERROR_SUCCESS != m_srv.getRemoteVariableInt(
                                        strVariableName, &pthreadObj->m_zone)) {
                syslog(LOG_ERR,
                       "Failed to read variable _zone. prefix=%s i=%d",
                       (const char *)m_prefix.c_str(),
                       i);
            }

            // Get subzone
            strVariableName = m_prefix + std::string("_subzone") + strIteration;
            if (VSCP_ERROR_SUCCESS !=
                m_srv.getRemoteVariableInt(strVariableName,
                                           &pthreadObj->m_subzone)) {
                syslog(LOG_ERR,
                       "Failed to read variable _subzone. prefix=%s i=%d",
                       (const char *)m_prefix.c_str(),
                       i);
            }

            // Get unit
            strVariableName = m_prefix + std::string("_unit") + strIteration;
            if (VSCP_ERROR_SUCCESS != m_srv.getRemoteVariableInt(
                                        strVariableName, &pthreadObj->m_unit)) {
                syslog(LOG_ERR,
                       "Failed to read variable _unit. prefix=%s i=%d",
                       (const char *)m_prefix.c_str(),
                       i);
            }

            m_objectList.push_back(pthreadObj);

        } else {
            syslog(LOG_ERR,
                   "Failed to start workerthread. prefix=%s i=%d",
                   (const char *)m_prefix.c_str(),
                   i);
            delete pthreadObj;
            rv = false;
        }
    }

    // XML setup
    std::string strSetupXML;
    std::string strName = m_prefix + std::string("_setup");
    if (VSCP_ERROR_SUCCESS ==
        m_srv.getRemoteVariableValue(strName, strSetupXML, true)) {
        XML_Parser xmlParser = XML_ParserCreate("UTF-8");
        XML_SetUserData(xmlParser, this);
        XML_SetElementHandler(xmlParser, startSetupParser, endSetupParser);

        int bytes_read;
        void *buff = XML_GetBuffer(xmlParser, XML_BUFF_SIZE);

        strncpy((char *)buff, strSetupXML.c_str(), strSetupXML.length());

        bytes_read = strSetupXML.length();
        if (!XML_ParseBuffer(xmlParser, bytes_read, bytes_read == 0)) {
            syslog(LOG_ERR, "Failed parse XML setup.");
        }

        XML_ParserFree(xmlParser);
    }

    // Close the channel
    m_srv.doCmdClose();

    // Start the worker threads
    std::deque<CWrkTreadObj *>::iterator it;
    for (it = m_objectList.begin(); it != m_objectList.end(); ++it) {

        CWrkTreadObj *pthreadObj = new CWrkTreadObj;
        if (NULL == pthreadObj) {
            continue;
        }

        if (pthread_create(
              &pthreadObj->m_pthreadWork, NULL, workerThread, pthreadObj)) {

            syslog(LOG_ERR,
                   "Unable to allocate memory for "
                   "controlobject client thread.");
            return false;
        }
    }

    return rv;
}

//////////////////////////////////////////////////////////////////////
// close
//

void
Clmsensors::close(void)
{
    // Do nothing if already terminated
    if (m_bQuit) return;

    m_bQuit = true; // terminate the thread
}

//////////////////////////////////////////////////////////////////////
// addEvent2SendQueue
//

bool
Clmsensors::addEvent2SendQueue(const vscpEvent *pEvent)
{
    pthread_mutex_lock(&m_mutexSendQueue);
    m_sendList.push_back((vscpEvent *)pEvent);
    sem_post(&m_semSendQueue);
    pthread_mutex_unlock(&m_mutexSendQueue);
    return true;
}

//////////////////////////////////////////////////////////////////////
//                           Workerthread
//////////////////////////////////////////////////////////////////////

CWrkTreadObj::CWrkTreadObj()
{
    m_pObj = NULL;
    m_path.clear();
    m_guid.clear(); // Interface GUID
    m_interval      = DEFAULT_INTERVAL;
    m_vscpclass     = VSCP_CLASS1_MEASUREMENT;
    m_vscptype      = 0;
    m_datacoding    = VSCP_TYPE_MEASUREMENT_TEMPERATURE;
    m_divideValue   = 0; // Zero means ignore
    m_multiplyValue = 0; // Zero means ignore
    m_readOffset    = 0; // Read numerical at offset 0
    m_index         = 0;
    m_zone          = 0;
    m_subzone       = 0;
}

CWrkTreadObj::~CWrkTreadObj()
{
    ;
}




//////////////////////////////////////////////////////////////////////
// Workerthread
//

void *
workerThread(void *pData)
{
    CWrkTreadObj *pWorkObj = (CWrkTreadObj *)pData;
    if (NULL == pWorkObj) {
        syslog(LOG_ERR, "Pointer error (no thread worker object)");
        return NULL;
    }

    // Check pointers
    if (NULL == pWorkObj->m_pObj) {
        syslog(LOG_ERR, "Pointer error (no driver object)");
        return NULL;
    }

    // Open the file
    FILE *pFile;
    if (NULL == (pFile = fopen(pWorkObj->m_path.c_str(), "rb"))) {
        syslog(LOG_ERR,
               "Workerthread. "
               "Failed to open lmsensors file Path=%s. Terminating!",
               pWorkObj->m_path.c_str());
        // Close the channel
        // m_srv.doCmdClose();
        return NULL;
    }

    char buf[25];
    double val;
    while (!pWorkObj->m_pObj->m_bQuit) {

        memset(buf, 0, sizeof(buf));
        if (0 != fseek(pFile, pWorkObj->m_readOffset, SEEK_SET)) {
            if (ferror(pFile)) {
                clearerr(pFile);
                syslog(LOG_ERR,
                       "Error seeking pos %d of file %s.",
                       pWorkObj->m_readOffset,
                       pWorkObj->m_path.c_str());
                goto error;
            }
        }

        memset(buf, 0, sizeof(buf));
        if (fread(buf, 1, sizeof(buf) - 1, pFile)) {

            std::string str = std::string(buf);
            val             = std::stod(buf);

            if (pWorkObj->m_divideValue) {
                val = val / pWorkObj->m_divideValue;
            }

            if (pWorkObj->m_multiplyValue) {
                val = val * pWorkObj->m_multiplyValue;
            }

            bool bNegative = false;
            if (val < 0) {
                bNegative = true;
                val       = (val < 0) ? -1.0 * val : val;
            }

            vscpEvent *pEvent = new vscpEvent();
            if (NULL != pEvent) {

                pEvent->pdata    = NULL;
                pEvent->sizeData = 0;

                pWorkObj->m_guid.writeGUID(pEvent->GUID);
                pEvent->vscp_class =
                  pWorkObj->m_vscpclass; // VSCP_CLASS1_MEASUREMENT;
                pEvent->vscp_type = pWorkObj->m_vscptype;

                pEvent->timestamp = vscp_makeTimeStamp();
                vscp_setEventDateTimeBlockToNow(pEvent);

                if (VSCP_CLASS1_MEASUREMENT == pWorkObj->m_vscpclass) {

                    switch (pWorkObj->m_datacoding &
                            VSCP_MASK_DATACODING_TYPE) {

                        case VSCP_DATACODING_BIT:
                        case VSCP_DATACODING_BYTE:
                        case VSCP_DATACODING_INTEGER:
                            if (val < 0xff) {
                                pEvent->sizeData = 2;
                                pEvent->pdata    = new uint8_t[2];
                                if (NULL != pEvent->pdata) {
                                    pEvent->pdata[1] = val;

                                    // Set data coding
                                    pEvent->pdata[0] = pWorkObj->m_datacoding;
                                } else {
                                    pEvent->sizeData = 0;
                                    pEvent->pdata    = NULL;
                                }
                            } else if (val < 0xffff) {
                                pEvent->sizeData = 3;
                                pEvent->pdata    = new uint8_t[3];
                                if (NULL != pEvent->pdata) {
                                    long lval        = val;
                                    pEvent->pdata[1] = (lval >> 8) & 0xff;
                                    pEvent->pdata[2] = lval & 0xff;

                                    // Set data coding
                                    pEvent->pdata[0] = pWorkObj->m_datacoding;
                                } else {
                                    pEvent->sizeData = 0;
                                    pEvent->pdata    = NULL;
                                }
                            } else if (val < 0xffffff) {
                                pEvent->sizeData = 4;
                                pEvent->pdata    = new uint8_t[4];
                                if (NULL != pEvent->pdata) {
                                    long lval        = val;
                                    pEvent->pdata[1] = (lval >> 16) & 0xff;
                                    pEvent->pdata[2] = (lval >> 8) & 0xff;
                                    pEvent->pdata[3] = lval & 0xff;

                                    // Set data coding
                                    pEvent->pdata[0] = pWorkObj->m_datacoding;
                                } else {
                                    pEvent->sizeData = 0;
                                    pEvent->pdata    = NULL;
                                }
                            } else {
                                pEvent->sizeData = 5;
                                pEvent->pdata    = new uint8_t[5];
                                if (NULL != pEvent->pdata) {
                                    long lval        = val;
                                    pEvent->pdata[1] = (lval >> 24) & 0xff;
                                    pEvent->pdata[2] = (lval >> 16) & 0xff;
                                    pEvent->pdata[3] = (lval >> 8) & 0xff;
                                    pEvent->pdata[4] = lval & 0xff;

                                    // Set data coding
                                    pEvent->pdata[0] = pWorkObj->m_datacoding;
                                } else {
                                    pEvent->sizeData = 0;
                                    pEvent->pdata    = NULL;
                                }
                            }
                            break;

                        case VSCP_DATACODING_NORMALIZED: {
                            uint8_t buf[8];
                            uint16_t size;
                            if (vscp_convertFloatToNormalizedEventData(
                                  buf,
                                  &size,
                                  val,
                                  ((pWorkObj->m_datacoding >> 3 & 3)),
                                  (pWorkObj->m_datacoding & 7)) &&
                                (0 != size)) {

                                pEvent->pdata = new uint8_t[size];
                                if (NULL != pEvent->pdata) {
                                    pEvent->sizeData = size;
                                    memcpy(pEvent->pdata, buf, size);
                                } else {
                                    pEvent->sizeData = 0;
                                    pEvent->pdata    = NULL;
                                }
                            }
                        } break;

                        case VSCP_DATACODING_STRING: {
                            pEvent->sizeData = 8;
                            pEvent->pdata    = new uint8_t[8];

                            if (NULL != pEvent->pdata) {
                                std::string str;
                                str = vscp_str_format("%lf", val);
                                if (str.length() > 7) {
                                    str = vscp_str_left(str, 7);
                                }

                                strcpy((char *)(pEvent->pdata + 1),
                                       str.c_str());

                            } else {
                                pEvent->sizeData = 0;
                                pEvent->pdata    = NULL;
                            }

                            // Set data coding
                            pEvent->pdata[0] = pWorkObj->m_datacoding;

                        } break;

                        case VSCP_DATACODING_SINGLE: {
                            pEvent->sizeData = 7;
                            pEvent->pdata    = new uint8_t[7];

                            if (NULL != pEvent->pdata) {
                                uint8_t buf[6];
                                float f    = (float)val;
                                uint8_t *p = (uint8_t *)&f;
                                memcpy(buf, p, 6);
                                // If on a little endian platform we
                                // have to change byte order
                                if (vscp_isLittleEndian()) {
                                    for (int i = 0; i < 6; i++) {
                                        pEvent->pdata[1 + i] = buf[5 - i];
                                    }
                                } else {
                                    memcpy(pEvent->pdata + 1, buf, 6);
                                }

                                // Set data coding
                                pEvent->pdata[0] = pWorkObj->m_datacoding;
                            } else {
                                pEvent->sizeData = 0;
                                pEvent->pdata    = NULL;
                            }
                        } break;
                    } // switch

                } else if ((VSCP_CLASS1_MEASUREZONE == pWorkObj->m_vscpclass) ||
                           (VSCP_CLASS1_SETVALUEZONE ==
                            pWorkObj->m_vscpclass)) {

                    // We pretend we are a standard measurement
                    uint8_t buf[8];
                    uint16_t size;
                    vscp_convertFloatToNormalizedEventData(
                      buf,
                      &size,
                      val,
                      ((pWorkObj->m_datacoding >> 3 & 3)),
                      (pWorkObj->m_datacoding & 7));

                    pEvent->pdata = new uint8_t[size];
                    if (NULL != pEvent->pdata) {
                        if (size <= 5) {
                            pEvent->sizeData = size;
                            memcpy(pEvent->pdata + 3, buf, size);
                        } else {
                            delete pEvent->pdata;
                            pEvent->sizeData = 0;
                            pEvent->pdata    = NULL;
                        }
                    } else {
                        pEvent->sizeData = 0;
                        pEvent->pdata    = NULL;
                    }

                } else if (VSCP_CLASS1_MEASUREMENT64 == pWorkObj->m_vscpclass) {
                    uint8_t buf[8];
                    {
                        uint8_t *p = (uint8_t *)&val;
                        memcpy(buf, p, 8);
                        // If on a little endian platform we
                        // have to change byte order
                        if (vscp_isLittleEndian()) {
                            for (int i = 0; i < 8; i++) {
                                pEvent->pdata[i] = buf[7 - i];
                            }
                        } else {
                            memcpy(pEvent->pdata, buf, 8);
                        }
                    }
                } else if (VSCP_CLASS2_MEASUREMENT_STR ==
                           pWorkObj->m_vscpclass) {
                    std::string str;
                    str = vscp_str_format("%d,%d,%d,%lf",
                                             pWorkObj->m_index,
                                             pWorkObj->m_zone,
                                             pWorkObj->m_subzone,
                                             pWorkObj->m_unit,
                                             val);
                    strcpy((char *)pEvent->pdata, str.c_str());
                }

                if (vscp_doLevel2Filter(pEvent,
                                        &pWorkObj->m_pObj->m_vscpfilter)) {
                    pthread_mutex_lock(&pWorkObj->m_pObj->m_mutexReceiveQueue);
                    pWorkObj->m_pObj->m_receiveList.push_back(pEvent);
                    sem_post(&pWorkObj->m_pObj->m_semReceiveQueue);
                    pthread_mutex_unlock(
                      &pWorkObj->m_pObj->m_mutexReceiveQueue);
                } else {
                    vscp_deleteVSCPevent(pEvent);
                }
            }
        }

    error:
        sleep(pWorkObj->m_interval ? pWorkObj->m_interval : 1);

    } // Worker loop

    // Close the file
    fclose(pFile);

    // Close the channel
    // m_srv.doCmdClose();

    return NULL;
}
