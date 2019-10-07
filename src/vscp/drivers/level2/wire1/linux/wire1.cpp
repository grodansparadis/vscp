// wire1.cpp: implementation
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

#include <string>
#include <list>
#include <deque>

#include "limits.h"
#include "stdlib.h"
#include "syslog.h"
#include "unistd.h"
#include <ctype.h>
#include <errno.h>
#include <libgen.h>
#include <net/if.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <time.h>

#include <expat.h>

#include <vscp.h>
#include <vscp_class.h>
#include <vscp_type.h>
#include <vscphelper.h>
#include <vscpremotetcpif.h>

#include "vscpl2drv-wire1.h"
#include "wire1.h"

// Buffer for XML parser
#define XML_BUFF_SIZE 10000

// Forward declaration
void *workerThread(void *pData);

// Dallas 1-wire GUID base
uint8_t guid1w[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

//////////////////////////////////////////////////////////////////////
//                           CSensor
//////////////////////////////////////////////////////////////////////

CSensor::CSensor()
{
    m_pW1Obj = NULL;
    m_path.clear();
    m_guid.clear(); // Interface GUID
    m_interval = DEFAULT_INTERVAL;
    m_index    = 0;
    m_unit     = DEFAULT_UNIT;
    m_coding   = DEFAULT_CODING;
}

CSensor::~CSensor()
{
    ;
}

//////////////////////////////////////////////////////////////////////
// CWire1
//

CWire1::CWire1()
{
    m_bQuit       = false;
    m_nSensors    = 1;                   // Default is one sensor
    vscp_clearVSCPFilter(&m_vscpfilter); // Accept all events

    sem_init(&m_semSendQueue, 0, 0);
    sem_init(&m_semReceiveQueue, 0, 0);

    pthread_mutex_init(&m_mutexSendQueue, NULL);
    pthread_mutex_init(&m_mutexReceiveQueue, NULL);
}

//////////////////////////////////////////////////////////////////////
// ~Csocketcan
//

CWire1::~CWire1()
{
    close();

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
        <sensor path="path to sensor data"
                guid=""
                interval=60"
                unit="1"
                index="0" />
    </setup>
*/

// ----------------------------------------------------------------------------

int depth_setup_parser = 0;

void
startSetupParser( void *data, const char *name, const char **attr )
{
    CWire1 *pObj = (CWire1 *)data;
    if (NULL == pObj) return;

    if ((0 == strcmp(name, "sensor")) && (1 == depth_setup_parser)) {

        CSensor *psensor = new CSensor;
        if (NULL == psensor) {
            return;
        }

        for (int i = 0; attr[i]; i += 2) {

            std::string attribute = attr[i + 1];
            vscp_trim(attribute);

            if (0 == strcasecmp(attr[i], "path")) {
                if (!attribute.empty()) {
                    psensor->m_path = attribute;
                }
            } else if (0 == strcasecmp(attr[i], "guid")) {
                if (!attribute.empty()) {
                    psensor->m_guid.getFromString(attribute);
                }
            } else if (0 == strcmp(attr[i], "interval")) {
                if (!attribute.empty()) {
                    psensor->m_interval = vscp_readStringValue(attribute);
                }
            } else if (0 == strcmp(attr[i], "index")) {
                if (!attribute.empty()) {
                    psensor->m_index = vscp_readStringValue(attribute);
                }
            } else if (0 == strcmp(attr[i], "unit")) {
                if (!attribute.empty()) {
                    psensor->m_unit = vscp_readStringValue(attribute);
                }
            }
        }

        pObj->m_sensorList.push_back(psensor);

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
CWire1::open(const char *pUsername,
             const char *pPassword,
             const char *pHost,
             short port,
             const char *pPrefix,
             const char *pConfig)
{
    bool rv           = true;
    std::string str = std::string(pConfig);

    m_username = std::string(pUsername);
    m_password = std::string(pPassword);
    m_host     = std::string(pHost);
    m_port     = port;
    m_prefix   = std::string(pPrefix);
    std::string strVariableName;

    // Parse the configuration string. It should
    // have the following form path
    std::deque<std::string> tokens;
    vscp_split( tokens, std::string(pConfig), ";" );

    // Check for # of sensors in configuration string
    if (!tokens.empty()) {
        // Path
        m_nSensors = vscp_readStringValue(tokens.front());
        tokens.pop_front();
    }

    // First log on to the host and get configuration
    // variables

    if (VSCP_ERROR_SUCCESS != m_srv.doCmdOpen(m_host,
                                              m_port,
                                              m_username,
                                              m_password)) {
        syslog(LOG_ERR,
               "%s",
               (const char
                  *)"Unable to connect to VSCP TCP/IP interface. Terminating!");
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
    //   _guid[n] - GUID for sensor n. guid0, guid1, guid2 etc. If not
    //     give the 1wire guid + the sensor id is used.
    //
    //	 _path[n] - Path to value for the sensor value file
    //				/sys/bus/w1/devices/10-00080192afa8/w1_slave
    //
    //   _interval[n]  - Interval in Seconds the event should be sent out.
    //						Zero disables.
    //
    //   _unit[n] - 0=Kelvin, 1=Celsius (default), 2=Fahrenheit
    //
    //   _index[n] - Sensorindex 0-7
    //
    // <setup>
    //  <sensor guid=""
    //          path="path to sensor data"
    //          interval=60"
    //          unit="1"
    //          index="0" />
    // </setup>

    // Get configuration data
    int varNumberOfSensors = 0;

    std::string strNumberOfSensors =
      m_prefix + std::string("_numberofsensors");

    if (!m_srv.getRemoteVariableInt(strNumberOfSensors, &varNumberOfSensors)) {
        // We use default 1 or config parameter supplied
    }

    // Read in the configuration values for each sensor
    for (int i = 0; i < varNumberOfSensors; i++) {

        std::string strIteration;
        strIteration = vscp_str_format( "%d", i);
        vscp_trim(strIteration);

        CSensor *psensor = new CSensor();
        if (NULL != psensor) {

            // Get sample interval
            strVariableName =
              m_prefix + std::string("_interval") + strIteration;
            if (VSCP_ERROR_SUCCESS ==
                m_srv.getRemoteVariableInt(strVariableName,
                                           &psensor->m_interval)) {
                // Node need to log error as optional
            }

            // Disabled if interval is zero
            if (0 == psensor->m_interval) continue;

            // Get the path
            strVariableName =
              m_prefix + std::string("_path") + strIteration;

            if (VSCP_ERROR_SUCCESS != m_srv.getRemoteVariableValue(
                                        strVariableName, psensor->m_path)) {
                syslog(LOG_ERR,
                       "%s prefix=%s i=%d",
                       (const char *)"Failed to read variable _path.",
                       (const char *)m_prefix.c_str(),
                       i);
            }

            // Get GUID
            strVariableName =
              m_prefix + std::string("_guid") + strIteration;
            if (!m_srv.getRemoteVariableGUID(strVariableName,
                                             psensor->m_guid)) {
                // Node need to log error as optional
            }

            // Get unit
            strVariableName =
              m_prefix + std::string("_unit") + strIteration;
            if (VSCP_ERROR_SUCCESS !=
                m_srv.getRemoteVariableInt(strVariableName,
                                           &psensor->m_unit)) {
                // Node need to log error as optional
            }

            // Check read unit value
            if (psensor->m_unit > 3)
                psensor->m_unit = DEFAULT_UNIT; // Celsius

            // Get index
            strVariableName =
              m_prefix + std::string("_index") + strIteration;
            if (VSCP_ERROR_SUCCESS !=
                m_srv.getRemoteVariableInt(strVariableName,
                                           &psensor->m_index)) {
                // Node need to log error as optional
            }

            // Check read index value
            if (psensor->m_index > 7) psensor->m_index = 0;

            // Get coding
            strVariableName =
              m_prefix + std::string("_coding") + strIteration;
            if (VSCP_ERROR_SUCCESS !=
                m_srv.getRemoteVariableInt(strVariableName,
                                           &psensor->m_coding)) {
                // Node need to log error as optional
            }

            // Check read coding value
            if (psensor->m_coding > 2) psensor->m_coding = 0;


            psensor->m_pW1Obj = this;

            // start the workerthread
            if (pthread_create(
                  &psensor->m_threadWork, NULL, workerThread, psensor)) {

                delete psensor;

                syslog(LOG_ERR,
                       "Controlobject: Unable to allocate memory for "
                       "controlobject client thread.");
                return false;
            }

            m_sensorList.push_back(psensor);

        } else {
            syslog(LOG_ERR,
                   "%s prefix=%s i=%d",
                   (const char *)"Failed to start workerthread.",
                   (const char *)m_prefix.c_str(),
                   i);
            rv = false;
        }
    }

    // XML setup
    std::string strSetupXML;
    strVariableName = m_prefix + std::string("_setup");
    if (VSCP_ERROR_SUCCESS ==
        m_srv.getRemoteVariableValue(strVariableName, strSetupXML, true)) {
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

    return rv;
}

//////////////////////////////////////////////////////////////////////
// close
//

void
CWire1::close(void)
{
    // Do nothing if already terminated
    if (m_bQuit) return;

    m_bQuit = true; // terminate the thread
    sleep(1);     // Give the thread some time to terminate
}

//////////////////////////////////////////////////////////////////////
// addEvent2SendQueue
//

bool
CWire1::addEvent2SendQueue(const vscpEvent *pEvent)
{
    pthread_mutex_lock( &m_mutexSendQueue);
    m_sendList.push_back((vscpEvent *)pEvent);
    sem_post(&m_semSendQueue);
    pthread_mutex_unlock( &m_mutexSendQueue);
    return true;
}



//////////////////////////////////////////////////////////////////////
// Worker thread
//

void *workerThread(void *pData)
{
    CSensor *pObj = (CSensor *)pData;
    if (NULL == pObj) return NULL;

    while (!pObj->m_pW1Obj->m_bQuit) {

        // Open the file
        if (NULL == (pObj->m_pFile = fopen(pObj->m_path.c_str(), "r"))) {
            syslog(
              LOG_ERR,
              "%s",
              (const char *)". File to open 1-wire data file. Terminating!");

            return NULL;
        }

        // Go to beginning
        fseek(pObj->m_pFile, SEEK_SET, 0);

        // Read line 1
        if (NULL == fgets(pObj->m_line1, sizeof(pObj->m_line1) - 1, pObj->m_pFile)) {

            // Close the file
            fclose(pObj->m_pFile);

            syslog(LOG_ERR,
                   "%s",
                   (const char
                      *)"Workerthread. Failed to read 1-wire data from file.");

            sleep(2); // We sleep before trying again.
            continue;   // Try again
        }

        // Read line 2
        if (NULL == fgets(pObj->m_line2, sizeof(pObj->m_line2) - 1, pObj->m_pFile)) {

            // Close the file
            fclose(pObj->m_pFile);

            syslog(LOG_ERR,
                   "%s",
                   (const char
                      *)"Workerthread. Failed to read 1-wire data from file.");

            sleep(2); // We sleep before trying again.
            continue;   // Try again
        }

        // 08 00 4b 46 ff ff 0d 10 ff : crc=ff YES
        sscanf(pObj->m_line1,
               "%02x %02x %02x %02x %02x %02x %02x %02x %02x",
               &pObj->m_id[0],
               &pObj->m_id[1],
               &pObj->m_id[2],
               &pObj->m_id[3],
               &pObj->m_id[4],
               &pObj->m_id[5],
               &pObj->m_id[6],
               &pObj->m_id[7],
               &pObj->m_id[8]);

        if (NULL == strstr(pObj->m_line1, "YES")) {
            // Close the file
            fclose(pObj->m_pFile);
            sleep(2); // We sleep before trying again.
            continue;   // Try again
        }

        // Close the file
        fclose(pObj->m_pFile);

        sscanf(
          pObj->m_line2, "%*s %*s %*s %*s %*s %*s %*s %*s %*s t=%d", &pObj->m_temperature);

        double val     = (double)pObj->m_temperature / 1000;
        bool bNegative = false;
        if (val < 0) {
            bNegative = true;
            val       = (val < 0) ? -1.0 * val : val;
        }

        // If unit is other than Celsius do conversion
        if (0 == pObj->m_unit) { // Kelvin?
            val += 273.15;
        } else if (2 == pObj->m_unit) { // Fahrenheit
            val = (9.0 / 5.0) * val + 32.0;
        }

        // If no GUID set we use the 1-wire GUID
        if (pObj->m_guid.isNULL()) {
            pObj->m_guid.getFromArray(guid1w);
            pObj->m_guid.setAt(8, pObj->m_id[0]);
            pObj->m_guid.setAt(9, pObj->m_id[1]);
            pObj->m_guid.setAt(10, pObj->m_id[2]);
            pObj->m_guid.setAt(11, pObj->m_id[3]);
            pObj->m_guid.setAt(12, pObj->m_id[4]);
            pObj->m_guid.setAt(13, pObj->m_id[5]);
            pObj->m_guid.setAt(14, pObj->m_id[6]);
            pObj->m_guid.setAt(15, pObj->m_id[7]);
        }

        vscpEvent *pEvent = new vscpEvent();
        if (NULL != pEvent) {

            pEvent->pdata    = NULL;
            pEvent->sizeData = 0;

            pObj->m_guid.writeGUID(pEvent->GUID);
            pEvent->vscp_class = VSCP_CLASS1_MEASUREMENT;
            pEvent->vscp_type  = VSCP_TYPE_MEASUREMENT_TEMPERATURE;

            pEvent->timestamp = vscp_makeTimeStamp();
            vscp_setEventDateTimeBlockToNow(pEvent);

            switch (pObj->m_coding) {

                case 0: // Normalized integer
                {
                    uint8_t buf[8];
                    uint16_t size;
                    if (vscp_convertFloatToNormalizedEventData(
                          buf, &size, val, pObj->m_unit, pObj->m_index)) {

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

                case 1: // String
                {
                    pEvent->sizeData = 8;
                    pEvent->pdata    = new uint8_t[8];

                    if (NULL != pEvent->pdata) {
                        std::string str;
                        str = vscp_str_format("%lf", val);
                        if (str.length() > 7) {
                            str = vscp_str_left(str,7);
                        }

                        strcpy((char *)(pEvent->pdata + 1), str.c_str());

                    } else {
                        pEvent->sizeData = 0;
                        pEvent->pdata    = NULL;
                    }

                    // Set data coding
                    pEvent->pdata[0] = VSCP_DATACODING_NORMALIZED;

                } break;

                case 2: {
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
                        pEvent->pdata[0] = VSCP_DATACODING_SINGLE;
                    } else {
                        pEvent->sizeData = 0;
                        pEvent->pdata    = NULL;
                    }
                } break;

            } // switch

            if (vscp_doLevel2Filter(pEvent, &pObj->m_pW1Obj->m_vscpfilter)) {
                pthread_mutex_lock( &pObj->m_pW1Obj->m_mutexReceiveQueue);
                pObj->m_pW1Obj->m_receiveList.push_back(pEvent);
                sem_post(&pObj->m_pW1Obj->m_semReceiveQueue);
                pthread_mutex_unlock( &pObj->m_pW1Obj->m_mutexReceiveQueue);
            } else {
                vscp_deleteVSCPevent(pEvent);
            }

        } // event

        sleep(pObj->m_interval ? pObj->m_interval : 10);
    }

    // Close the file
    // fclose (m_pFile);

    return NULL;
}
