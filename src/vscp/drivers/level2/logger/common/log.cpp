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

#include <vscp_class.h>
#include <vscp_type.h>
#include <vscpdatetime.h>
#include <vscphelper.h>
#include <vscpremotetcpif.h>

#include "log.h"

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

// Forward declarations
void *threadWorker(void *pData);

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
    m_pLogStream  = NULL;
    m_pthreadWork = NULL;

    sem_init(&m_semSendQueue, 0, 0);
    pthread_mutex_init(&m_mutexSendQueue, NULL)

	openlog("vscpl2drv_logger", LOG_PID, LOG_DAEMON );
}

//////////////////////////////////////////////////////////////////////
// ~CVSCPLog
//

CVSCPLog::~CVSCPLog()
{
    close();

    sem_destroy(&m_semSendQueue);
    pthread_mutex_destroy(&m_mutexSendQueue);

	closelog(void);
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
    bool rv        = true;
    std::string wxstr = std::string(pConfig);

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
	vscp_split( tokens, std::string(pConfig), ";" );

    // Check for path in configuration string
    if (tokens.empty()) {
		syslog( LOG_ERR, "A path must be configured.");	
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
        if (0 == vscp_strcasecmp( str.c_str(),"TRUE")) {
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
        if (0 == vscp_strcasecmp( str.c_str(),"TRUE") ) {
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
        vscp_readFilterFromString(&m_Filter, str);
    }

    // Mask
    if (!tokens.empty()) {
        std::string str;
        str = tokens.front();
		tokens.pop_front();
        vscp_readMaskFromString(&m_Filter, str);
    }

    // start the worker thread
    CLogWrkThreadObj *pObj = new CLogWrkThreadObj();
    if (NULL != pObj) {

		pObj->m_pLog = this;

        if (!pthread_create(&m_pthreadWork, NULL, threadWorker, pObj)) {
            syslog(LOG_CRIT, "Unable to start logger driver worker thread.");
            return false;
        }
    } else {
        syslog(LOG_CRIT, "Unable to allocate thread object.") rv = false;
    }

    return rv;
}

//////////////////////////////////////////////////////////////////////
// close
//

void
CVSCPLog::close(void)
{
    if (m_file.IsOpened() && (m_flags | LOG_FILE_VSCP_WORKS)) {
        m_pLogStream->Write("</vscprxdata>\n", strlen("</vscprxdata>\n"));
    }

    // Close the log-file
    flose(m_file);

    // Do nothing if already terminated
    if (m_bQuit) return;

    m_bQuit = true; // terminate the thread
    sleep(1);     // Give the thread some time to terminate

    // Delete the stream object
    if (NULL != m_pLogStream) delete m_pLogStream;
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

    if (m_flags & LOG_FILE_OVERWRITE) {
        if (m_file.Open(m_path, wxFile::write)) {
            m_pLogStream = new wxFileOutputStream(m_file);
            if (NULL != m_pLogStream) {
                if (m_pLogStream->IsOk()) {
                    if (m_flags & LOG_FILE_VSCP_WORKS) {
                        m_pLogStream->Write(
                          "<?xml version = \"1.0\" encoding = \"UTF-8\" ?>\n",
                          strlen("<?xml version = \"1.0\" encoding = \"UTF-8\" "
                                 "?>\n"));
                        // RX data start
                        m_pLogStream->Write("<vscprxdata>\n",
                                            strlen("<vscprxdata>\n"));
                        return true;
                    } else {
                        return true;
                    }
                } else {
                    return false;
                }
            }
        }
    } else {
        if (m_file.Open(m_path, wxFile::write_append)) {
            m_pLogStream = new wxFileOutputStream(m_file);
            if (NULL != m_pLogStream) {
                if (m_pLogStream->IsOk()) {
                    if (m_flags & LOG_FILE_VSCP_WORKS) {
                        m_pLogStream->Write(
                          "<?xml version = \"1.0\" encoding = \"UTF-8\" ?>\n",
                          strlen("<?xml version = \"1.0\" encoding = \"UTF-8\" "
                                 "?>\n"));
                        // RX data start
                        m_pLogStream->Write("<vscprxdata>\n",
                                            strlen("<vscprxdata>\n"));
                        return true;
                    } else {
                        return true;
                    }
                } else {
                    return false;
                }
            }
        }
    }

    return false;
}

//////////////////////////////////////////////////////////////////////
// addEvent2Queue
//

bool
CVSCPLog::addEvent2SendQueue(const vscpEvent *pEvent)
{
    m_mutexSendQueue.Lock();
    // m_sendQueue.Append((vscpEvent *)pEvent);
    m_sendList.push_back((vscpEvent *)pEvent);
    m_semSendQueue.Post();
    m_mutexSendQueue.Unlock();
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

        // VSCP Works log format

        // Event
        m_pLogStream->Write("<event>\n", strlen("<event>\n"));
        m_pLogStream->Write("rx", strlen("rx"));
        m_pLogStream->Write("</dir>\n", strlen("</dir>\n"));

        m_pLogStream->Write("<time>", strlen("<time>"));
        str = wxDateTime::Now().FormatISODate() + _(" ") +
              wxDateTime::Now().FormatISOTime();
        m_pLogStream->Write(str.mbc_str(), strlen(str.mbc_str()));
        m_pLogStream->Write("</time>\n", strlen("</time>\n"));

        m_pLogStream->Write("<dt>", strlen("<dt>"));
        if (!vscp_getDateStringFromEvent(pEvent, str)) {
            str = _("Failed to get date/time.");
        }
        m_pLogStream->Write(str.mbc_str(), strlen(str.mbc_str()));
        m_pLogStream->Write("</dt>\n", strlen("</dt>\n"));

        m_pLogStream->Write("<head>", strlen("<head>"));
        str.Printf(_("%d"), pEvent->head);
        m_pLogStream->Write(str.mbc_str(), strlen(str.mbc_str()));
        m_pLogStream->Write("</head>\n", strlen("</head>\n"));

        m_pLogStream->Write("<class>", strlen("<class>"));
        str.Printf(_("%d"), pEvent->vscp_class);
        m_pLogStream->Write(str.mbc_str(), strlen(str.mbc_str()));
        m_pLogStream->Write("</class>\n", strlen("</class>\n"));

        m_pLogStream->Write("<type>", strlen("<type>"));
        str.Printf(_("%d"), pEvent->vscp_type);
        m_pLogStream->Write(str.mbc_str(), strlen(str.mbc_str()));
        m_pLogStream->Write("</type>\n", strlen("</type>\n"));

        m_pLogStream->Write("<guid>", strlen("<guid>"));
        vscp_writeGuidToString(pEvent, str);
        m_pLogStream->Write(str.mbc_str(), strlen(str.mbc_str()));
        m_pLogStream->Write("</guid>\n", strlen("</guid>\n"));

        m_pLogStream->Write("<sizedata>",
                            strlen("<sizedata>")); // Not used by read routine
        str.Printf(_("%d"), pEvent->sizeData);
        m_pLogStream->Write(str.mbc_str(), strlen(str.mbc_str()));
        m_pLogStream->Write("</sizedata>\n", strlen("</sizedata>\n"));

        if (0 != pEvent->sizeData) {
            m_pLogStream->Write("<data>", strlen("<data>"));
            vscp_writeVscpDataToString(pEvent, str);
            m_pLogStream->Write(str.mbc_str(), strlen(str.mbc_str()));
            m_pLogStream->Write("</data>\n", strlen("</data>\n"));
        }

        m_pLogStream->Write("<timestamp>", strlen("<timestamp>"));
        str.Printf(_("%d"), pEvent->timestamp);
        m_pLogStream->Write(str.mbc_str(), strlen(str.mbc_str()));
        m_pLogStream->Write("</timestamp>\n", strlen("</timestamp>\n"));

        m_pLogStream->Write("<note>", strlen("<note>"));
        m_pLogStream->Write("</note>\n", strlen("</note>\n"));

        m_pLogStream->Write("</event>\n", strlen("</event>\n"));

    } else {
        // Standard log format
        std::string str;

        str = wxDateTime::Now().FormatISODate() + _(" ") +
              wxDateTime::Now().FormatISOTime() + _(": ");
        m_pLogStream->Write(str.mbc_str(), strlen(str.mbc_str()));

        str.Printf(_("head=%d "), pEvent->head);
        m_pLogStream->Write(str.mbc_str(), strlen(str.mbc_str()));

        str.Printf(_("class=%d "), pEvent->vscp_class);
        m_pLogStream->Write(str.mbc_str(), strlen(str.mbc_str()));

        str.Printf(_("type=%d "), pEvent->vscp_type);
        m_pLogStream->Write(str.mbc_str(), strlen(str.mbc_str()));

        str.Printf(_("GUID="), pEvent->vscp_type);
        m_pLogStream->Write(str.mbc_str(), strlen(str.mbc_str()));

        vscp_writeGuidToString(pEvent, str);
        m_pLogStream->Write(str.mbc_str(), strlen(str.mbc_str()));

        str.Printf(_(" datasize=%d "), pEvent->sizeData);
        m_pLogStream->Write(str.mbc_str(), strlen(str.mbc_str()));

        if (0 != pEvent->sizeData) {
            str.Printf(_("data="), pEvent->vscp_type);
            m_pLogStream->Write(str.mbc_str(), strlen(str.mbc_str()));
            vscp_writeVscpDataToString(pEvent, str);
            m_pLogStream->Write(str.mbc_str(), strlen(str.mbc_str()));
        }

        str.Printf(_(" Timestamp=%d\r\n"), pEvent->timestamp);
        m_pLogStream->Write(str.mbc_str(), strlen(str.mbc_str()));
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




// ----------------------------------------------------------------------------




///////////////////////////////////////////////////////////////////////////////
// 								Worker thread
///////////////////////////////////////////////////////////////////////////////

void *
threadWorker(void *pData)
{
	CLogWrkThreadObj *pObj = (CLogWrkThreadObj *)pData;
	if ( NULL == pObj ) {
		syslog(LOG_CRIT, "No thread object suppied to worker thread. Aborting!");
		return NULL;
	}

    // Check pointers
    if (NULL == pObj->m_pLog) {
		syslog(LOG_CRIT, "No valid logger object suppied to worker thread. Aborting!");
		return NULL;
	}

    // First log on to the host and get configuration
    // variables

    if (VSCP_ERROR_SUCCESS ==
        pObj->m_pLog->m_srv.doCmdOpen(pObj->m_pLog->m_host,
                        pObj->m_pLog->m_username,
                        pObj->m_pLog->m_password) <= 0) {
        return NULL;
    }

    // Find the channel id
    uint32_t ChannelID;
    pObj->m_pLog->m_srv.doCmdGetChannelID(&ChannelID);

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
         pObj->m_pLog->m_srv.getRemoteVariableValue(strFilter, varFilter)) &&
        (VSCP_ERROR_SUCCESS ==
         pObj->m_pLog->m_srv.getRemoteVariableValue(strMask, varMask))) {
        pObj->m_pLog->m_srv.doCmdFilter(varFilter, varMask);
    }

    // get overrided file path
    std::string strPath = pObj->m_pLog->m_prefix + std::string("_path");
    std::string varPath;
    if (VSCP_ERROR_SUCCESS == pObj->m_pLog->m_srv.getRemoteVariableValue(strPath, varPath)) {
        pObj->m_pLog->m_path = varPath;
    }

    std::string strRewrite = pObj->m_pLog->m_prefix + std::string("_rewrite");
    bool bOverwrite;
    if (VSCP_ERROR_SUCCESS ==
        pObj->m_pLog->m_srv.getRemoteVariableBool(strRewrite, &bOverwrite)) {
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
        pObj->m_pLog->m_srv.getRemoteVariableBool(strVscpWorkdFmt, &bVSCPWorksFormat)) {
        if (bVSCPWorksFormat) {
            pObj->m_pLog->m_flags |= LOG_FILE_VSCP_WORKS;
        } else {
            pObj->m_pLog->m_flags &= ~LOG_FILE_VSCP_WORKS;
        }
    }

    // Close server connection
    pObj->m_pLog->m_srv.doCmdClose();

    // Open the file
    if (!pObj->m_pLog->openFile()) return NULL;

    while (!TestDestroy() && !pObj->m_pLog->m_bQuit) {

        // Wait for events
        if (wxSEMA_TIMEOUT == pObj->m_pLog->m_semSendQueue.WaitTimeout(500)) continue;

        if (pObj->m_pLog->m_sendList.size()) {

            pObj->m_pLog->m_mutexSendQueue.Lock();
            vscpEvent *pEvent = pObj->m_pLog->m_sendList.front();
            pObj->m_pLog->m_sendList.pop_front();
            pObj->m_pLog->m_mutexSendQueue.Unlock();

            if (NULL == pEvent) continue;

            pObj->m_pLog->writeEvent(pEvent);

            vscp_deleteVSCPevent(pEvent);
            pEvent = NULL;

        } // Event received

    } // Receive loop

    // Close the channel
    pObj->m_pLog->m_srv.doCmdClose();

    return NULL;
}
