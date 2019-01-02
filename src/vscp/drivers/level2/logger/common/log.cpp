// Log.cpp: implementation of the CVSCPLog class.
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

#include <deque>
#include <fstream>
#include <list>
#include <string>

#include <limits.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>

#include <expat.h>

#include <vscp_class.h>
#include <vscp_type.h>
#include <vscpdatetime.h>
#include <vscphelper.h>
#include <vscpremotetcpif.h>

#include "log.h"

// Buffer size for XML parser
#define XML_BUFF_SIZE   10000

// Forward declarations
void *
threadWorker(void *pData);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CVSCPLog
//

CVSCPLog::CVSCPLog()
{
    m_flags       = 0;
    m_bQuit       = false;

    sem_init(&m_semSendQueue, 0, 0);
    pthread_mutex_init(&m_mutexSendQueue, NULL);

    openlog("vscpl2drv_logger", LOG_PID, LOG_DAEMON);
}

//////////////////////////////////////////////////////////////////////
// ~CVSCPLog
//

CVSCPLog::~CVSCPLog()
{
    close();

    sem_destroy(&m_semSendQueue);
    pthread_mutex_destroy(&m_mutexSendQueue);

    closelog();
}

//////////////////////////////////////////////////////////////////////
// open
//
// filename
//      the name of the log file
//
// flags
//      bit 1 = 0 Append.
//      bit 1 = 1 Rewrite (=overwrite)
//      bit 2 - 0 Standard format.
//      bit 2 - 1 VSCP Works XML format.
//

bool
CVSCPLog::open(const char *pUsername,
               const char *pPassword,
               const char *pHost,
               short port,
               const char *pPrefix,
               const char *pConfig)
{
    std::string str = std::string(pConfig);

    m_username = std::string(pUsername);
    m_password = std::string(pPassword);
    m_host     = std::string(pHost);
    m_port     = port;
    m_prefix   = std::string(pPrefix);

    // Parse the configuration string. It should
    // have the following form
    // path
    //
    std::deque<std::string> tokens;
    vscp_split(tokens, std::string(pConfig), ";");

    // Check for path in configuration string
    if (tokens.empty()) {
        syslog(LOG_ERR, "A path must be configured.");
    }

    // Path
    m_path = tokens.front();
    tokens.pop_front();

    // Check for rewrite in configuration string
    // valid is "true|false"
    if (!tokens.empty()) {
        std::string str;
        str = tokens.front();
        tokens.pop_front();
        vscp_trim(str);
        if (0 == vscp_strcasecmp(str.c_str(), "TRUE")) {
            m_flags |= LOG_FILE_OVERWRITE;
        } else {
            m_flags &= ~LOG_FILE_OVERWRITE;
        }
    }

    // Check for vscpworksfmt in configuration string
    // valid is "true|false"
    if (!tokens.empty()) {
        std::string str;
        str = tokens.front();
        tokens.pop_front();
        vscp_trim(str);
        if (0 == vscp_strcasecmp(str.c_str(), "TRUE")) {
            m_flags |= LOG_FILE_VSCP_WORKS;
        } else {
            m_flags &= ~LOG_FILE_VSCP_WORKS;
        }
    }

    // Filter
    if (!tokens.empty()) {
        std::string str;
        str = tokens.front();
        tokens.pop_front();
        vscp_readFilterFromString(&m_vscpfilter, str);
    }

    // Mask
    if (!tokens.empty()) {
        std::string str;
        str = tokens.front();
        tokens.pop_front();
        vscp_readMaskFromString(&m_vscpfilter, str);
    }

    // start the worker thread
    m_pWorkObj = new CLogWrkThreadObj();
    if (NULL != m_pWorkObj) {

        m_pWorkObj->m_pLog = this;

        if (!pthread_create( &m_pWrkThread, NULL, threadWorker, m_pWorkObj)) {
            syslog(LOG_CRIT, "Unable to start logger driver worker thread.");
            return false;
        }
    } else {
        syslog(LOG_CRIT, "Unable to allocate thread object.");
        return false;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// close
//

void
CVSCPLog::close(void)
{
    if (m_logStream.is_open() && (m_flags | LOG_FILE_VSCP_WORKS)) {
        m_logStream.write("</vscprxdata>\n", strlen("</vscprxdata>\n"));
    }

    // Close the log-file
    m_logStream.close();

    // Do nothing if already terminated
    if (m_bQuit) return;

    m_bQuit = true; // terminate the thread
    sleep(1);       // Give the thread some time to terminate
}

//////////////////////////////////////////////////////////////////////
// doFilter
//

bool
CVSCPLog::doFilter(vscpEvent *pEvent)
{

    return true;
}

//////////////////////////////////////////////////////////////////////
// setFilter
//

void
CVSCPLog::setFilter(vscpEvent *pFilter)
{}

//////////////////////////////////////////////////////////////////////
// setMask
//

void
CVSCPLog::setMask(vscpEvent *pMask)
{}

//////////////////////////////////////////////////////////////////////
// openFile
//

bool
CVSCPLog::openFile(void)
{
    try {
        if (m_flags & LOG_FILE_OVERWRITE) {

            m_logStream.open(m_path, std::fstream::out);

            if (m_flags & LOG_FILE_VSCP_WORKS) {
                m_logStream
                  << "<?xml version = \"1.0\" encoding = \"UTF-8\" ?>\n";
                // RX data start
                m_logStream << "<vscprxdata>\n";
                return true;
            } else {
                return true;
            }

        } else {

            m_logStream.open(m_path, std::fstream::out);

            if (m_flags & LOG_FILE_VSCP_WORKS) {
                m_logStream
                  << "<?xml version = \"1.0\" encoding = \"UTF-8\" ?>\n";
                // RX data start
                m_logStream << "<vscprxdata>\n";
                return true;
            } else {
                return true;
            }
        }
    } catch (...) {
        syslog(LOG_CRIT, "Failed to open log file!");
        return false;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////
// addEvent2Queue
//

bool
CVSCPLog::addEvent2SendQueue(const vscpEvent *pEvent)
{
    pthread_mutex_lock(&m_mutexSendQueue);
    m_sendList.push_back((vscpEvent *)pEvent);
    sem_post(&m_semSendQueue);
    pthread_mutex_unlock(&m_mutexSendQueue);
    return true;
}

//////////////////////////////////////////////////////////////////////
// writeEvent
//

bool
CVSCPLog::writeEvent(vscpEvent *pEvent)
{

    if (m_flags & LOG_FILE_VSCP_WORKS) {

        std::string str;

        // * * * VSCP Works log format * * *

        // Event
        m_logStream << "<event>\n";
        m_logStream << "rx";
        m_logStream << "</dir>\n";

        m_logStream << "<time>";
        str = vscpdatetime::setNow().getISODateTime();
        m_logStream << str.c_str();
        m_logStream << "</time>\n";

        m_logStream << "<dt>";
        if (!vscp_getDateStringFromEvent(pEvent, str)) {
            str = "Failed to get date/time.";
        }
        m_logStream << str.c_str();
        m_logStream << "</dt>\n";

        m_logStream << "<head>" << pEvent->head;
        m_logStream << "</head>\n";

        m_logStream << "<class>";
        m_logStream << pEvent->vscp_class;
        m_logStream << "</class>\n";

        m_logStream << "<type>";
        m_logStream << pEvent->vscp_type;
        m_logStream << "</type>\n";

        m_logStream << "<guid>";
        vscp_writeGuidToString(pEvent, str);
        m_logStream << str.c_str();
        m_logStream << "</guid>\n";

        m_logStream << "<sizedata>"; // Not used by read routine
        m_logStream << pEvent->sizeData;
        m_logStream << "</sizedata>\n";

        if (0 != pEvent->sizeData) {
            m_logStream << "<data>";
            vscp_writeVscpDataToString(pEvent, str);
            m_logStream << str.c_str();
            m_logStream << "</data>\n";
        }

        m_logStream << "<timestamp>";
        m_logStream << pEvent->timestamp;
        m_logStream << "</timestamp>\n";

        m_logStream << "<note>";
        m_logStream << "</note>\n";

        m_logStream << "</event>\n";

    } else {

        // * * * Standard log format * * *
        std::string str;

        str = vscpdatetime::setNow().getISODateTime();
        m_logStream << str.c_str();

        str = vscp_string_format("head=%d ", pEvent->head);
        m_logStream << str.c_str();

        str = vscp_string_format("class=%d ", pEvent->vscp_class);
        m_logStream << str.c_str();

        str = vscp_string_format("type=%d ", pEvent->vscp_type);
        m_logStream << str.c_str();

        str = vscp_string_format("GUID=", pEvent->vscp_type);
        m_logStream << str.c_str();

        vscp_writeGuidToString(pEvent, str);
        m_logStream << str.c_str();

        str = vscp_string_format(" datasize=%d ", pEvent->sizeData);
        m_logStream << str.c_str();

        if (0 != pEvent->sizeData) {
            str = vscp_string_format("data=", pEvent->vscp_type);
            m_logStream << str.c_str();
            vscp_writeVscpDataToString(pEvent, str);
            m_logStream << str.c_str();
        }

        str = vscp_string_format(" Timestamp=%d\r\n", pEvent->timestamp);
        m_logStream << str.c_str();
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
//                           Workerthread
//////////////////////////////////////////////////////////////////////

CLogWrkThreadObj::CLogWrkThreadObj()
{
    m_pLog = NULL;
}

CLogWrkThreadObj::~CLogWrkThreadObj() {}





///////////////////////////////////////////////////////////////////////////////
// 								Worker thread
///////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------------

/*
    XML format
    ==========
    <setup path="path-to-log-file" 
            brewrite="true|false"
            bworksfmt="true|false"
            filter="VSCP filter on string format"
            mask="VSCP mask on string format" />
*/

// ----------------------------------------------------------------------------

int depth_setup_parser = 0;

void
startSetupParser( void *data, const char *name, const char **attr ) 
{
    CVSCPLog *pLog = (CVSCPLog *)data;
    if ( NULL == pLog ) return;

    if ( ( 0 == strcmp( name, "setup") ) && 
         ( 0 == depth_setup_parser ) ) {

        for ( int i = 0; attr[i]; i += 2 ) {

            std::string attribute = attr[i+1];
            vscp_trim(attribute);

            if ( 0 == strcmp( attr[i], "path") ) {
                if ( !attribute.empty() ) {
                    pLog->m_path =  attribute;
                }
            }
            else if ( 0 == strcmp( attr[i], "filter") ) {
                if ( !attribute.empty() ) {
                    if ( !vscp_readFilterFromString( &pLog->m_vscpfilter, attribute ) ) {
                        syslog( LOG_ERR, "Unable to read event receive filter.");
                    }
                }
            }
            else if ( 0 == strcmp( attr[i], "mask") ) {
                if ( !attribute.empty() ) {
                    if ( !vscp_readMaskFromString( &pLog->m_vscpfilter, attribute ) ) {
                        syslog( LOG_ERR, "Unable to read event receive mask.");
                    }
                }
            }
            else if ( 0 == strcmp( attr[i], "brewrite") ) {
                if ( !attribute.empty() ) {
                    if ( 0 == vscp_strcasecmp(attribute.c_str(),"TRUE") ){
                        pLog->m_flags |= LOG_FILE_OVERWRITE;
                    }
                }
            }
            else if ( 0 == strcmp( attr[i], "bworksfmt") ) {
                if ( !attribute.empty() ) {
                    if ( 0 == vscp_strcasecmp(attribute.c_str(),"TRUE")) {
                        pLog->m_flags |= LOG_FILE_VSCP_WORKS;
                    }
                }
            }
        }
    } 

    depth_setup_parser++;
}

void
endSetupParser( void *data, const char *name ) 
{
    depth_setup_parser--;
}

// ----------------------------------------------------------------------------

void *
threadWorker(void *pData)
{
    CLogWrkThreadObj *pObj = (CLogWrkThreadObj *)pData;
    if (NULL == pObj) {
        syslog(LOG_CRIT,
               "No thread object supplied to worker thread. Aborting!");
        return NULL;
    }

    // Check pointers
    if (NULL == pObj->m_pLog) {
        syslog(LOG_CRIT,
               "No valid logger object suppied to worker thread. Aborting!");
        return NULL;
    }

    // First log on to the host and get configuration
    // variables

    if (VSCP_ERROR_SUCCESS ==
        pObj->m_srv.doCmdOpen(pObj->m_pLog->m_host,
                              pObj->m_pLog->m_username,
                              pObj->m_pLog->m_password) <= 0) {
        return NULL;
    }

    // Find the channel id
    uint32_t ChannelID;
    pObj->m_srv.doCmdGetChannelID(&ChannelID);

    // It is possible that there is configuration data the server holds
    // that we need to read in.
    // We look for
    //      prefix_filter to find a filter. A string is expected.
    //      prefix_mask to find a mask. A string is expected.
    //      prefix_path to overide the file path for the log file. A string is
    //      expected. prefix_rewrite to override the overwrite flag. A bool is
    //      expected.

    // Get filter data
    std::string varFilter;
    std::string varMask;

    std::string strFilter = pObj->m_pLog->m_prefix + std::string("_filter");
    std::string strMask   = pObj->m_pLog->m_prefix + std::string("_mask");
    if ((VSCP_ERROR_SUCCESS ==
         pObj->m_srv.getRemoteVariableValue(strFilter, varFilter)) &&
        (VSCP_ERROR_SUCCESS ==
         pObj->m_srv.getRemoteVariableValue(strMask, varMask))) {
        pObj->m_srv.doCmdFilter(varFilter, varMask);
    }

    // get overriden file path
    std::string strPath = pObj->m_pLog->m_prefix + std::string("_path");
    std::string varPath;
    if (VSCP_ERROR_SUCCESS ==
        pObj->m_srv.getRemoteVariableValue(strPath, varPath)) {
        pObj->m_pLog->m_path = varPath;
    }

    std::string strRewrite = pObj->m_pLog->m_prefix + std::string("_rewrite");
    bool bOverwrite;
    if (VSCP_ERROR_SUCCESS ==
        pObj->m_srv.getRemoteVariableBool(strRewrite, &bOverwrite)) {
        if (bOverwrite) {
            pObj->m_pLog->m_flags |= LOG_FILE_OVERWRITE;
        } else {
            pObj->m_pLog->m_flags &= ~LOG_FILE_OVERWRITE;
        }
    }

    bool bVSCPWorksFormat;
    std::string strVscpWorkdFmt =
      pObj->m_pLog->m_prefix + std::string("_vscpworksfmt");
    if (VSCP_ERROR_SUCCESS ==
        pObj->m_srv.getRemoteVariableBool(strVscpWorkdFmt, &bVSCPWorksFormat)) {
        if (bVSCPWorksFormat) {
            pObj->m_pLog->m_flags |= LOG_FILE_VSCP_WORKS;
        } else {
            pObj->m_pLog->m_flags &= ~LOG_FILE_VSCP_WORKS;
        }
    }

    // XML setup 
    std::string str;
    std::string strSetupXML;
    std::string strName = pObj->m_pLog->m_prefix + std::string("_setup");
    if (VSCP_ERROR_SUCCESS ==
        pObj->m_srv.getRemoteVariableValue(strName, strSetupXML, true)) {
        XML_Parser xmlParser = XML_ParserCreate("UTF-8");
        XML_SetUserData(xmlParser, pObj->m_pLog);
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

    // Close server connection
    pObj->m_srv.doCmdClose();

    // Open the file
    if (!pObj->m_pLog->openFile()) return NULL;

    while (!pObj->m_pLog->m_bQuit) {

        // Wait for events
        struct timespec ts;
        ts.tv_sec  = 0;
        ts.tv_nsec = 500000; // 500 ms
        if (ETIMEDOUT == sem_timedwait(&pObj->m_pLog->m_semSendQueue, &ts)) {
            continue;
        }

        if (pObj->m_pLog->m_sendList.size()) {

            pthread_mutex_lock(&pObj->m_pLog->m_mutexSendQueue);
            vscpEvent *pEvent = pObj->m_pLog->m_sendList.front();
            pObj->m_pLog->m_sendList.pop_front();
            pthread_mutex_unlock(&pObj->m_pLog->m_mutexSendQueue);

            if (NULL == pEvent) continue;
            pObj->m_pLog->writeEvent(pEvent);
            vscp_deleteVSCPevent(pEvent);
            pEvent = NULL;

        } // Event received

    } // Receive loop

    // Close the channel
    pObj->m_srv.doCmdClose();

    return NULL;
}
