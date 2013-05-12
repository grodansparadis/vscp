// Log.cpp: implementation of the CVSCPLog class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2013 Ake Hedman, 
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


#ifdef WIN32

#include <stdio.h>

#else

#include "unistd.h"
#include "stdlib.h"
#include "limits.h"

#endif

#ifdef WIN32
#include "winsock.h"
#endif

#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"
#include <wx/xml/xml.h>
#include <wx/listimpl.cpp>
#include <wx/thread.h>
#include <wx/tokenzr.h>
#include <wx/datetime.h>
#include "../../../../common/vscphelper.h"
#include "../../../../common/vscptcpif.h"
#include "log.h"

// Lists
WX_DEFINE_LIST(VSCPEVENTLIST);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CVSCPLog
//

CVSCPLog::CVSCPLog() {
    m_bQuit = false;
    m_pLogStream = NULL;
    m_pthreadWork = NULL;
    ::wxInitialize();
}

//////////////////////////////////////////////////////////////////////
// ~CVSCPLog
//

CVSCPLog::~CVSCPLog() {
    close();
    ::wxUninitialize();
}


//////////////////////////////////////////////////////////////////////
// open
//
// filename
//		the name of the log file
//
// flags 
//      bit 1 = 0 Append.
//      bit 1 = 1 Rewrite (=overwrite)
//      bit 2 - 0 Standard format.
//      bit 2 - 1 VSCP Works XML format.
//

bool CVSCPLog::open(const char *pUsername,
        const char *pPassword,
        const char *pHost,
        short port,
        const char *pPrefix,
        const char *pConfig,
        unsigned long flags) {
    bool rv = true;
    m_flags = flags;
    wxString wxstr = pConfig;

    m_username = pUsername;
    m_password = pPassword;
    m_host = pHost;
    m_port = port;
    m_prefix = pPrefix;

    // Parse the configuration string. It should
    // have the following form
    // username;password;host;prefix;port;filename
    wxStringTokenizer tkz(pConfig, ";\n");

    // Filename
    if (tkz.HasMoreTokens()) {
        m_path = tkz.GetNextToken();
    }

    // start the workerthread
    m_pthreadWork = new CVSCPLogWrkTread();
    if (NULL != m_pthreadWork) {
        m_pthreadWork->m_pLog = this;
        m_pthreadWork->Create();
        m_pthreadWork->Run();
    } else {
        rv = false;
    }

    return rv;
}


//////////////////////////////////////////////////////////////////////
// close
//

void CVSCPLog::close(void) {
    if (m_flags | LOG_FILE_VSCP_WORKS) {
        m_pLogStream->Write("</vscprxdata>\n", strlen("</vscprxdata>\n"));
    }

    // Close the log-file
    m_file.Close();

    // Do nothing if already terminated
    if (m_bQuit) return;

    m_bQuit = true; // terminate the thread
    wxSleep(1); // Give the thread some time to terminate

    // Delete the stream object
    if (NULL != m_pLogStream) delete m_pLogStream;

}


//////////////////////////////////////////////////////////////////////
// doFilter
//

bool CVSCPLog::doFilter(vscpEvent *pEvent) {

    return true;
}


//////////////////////////////////////////////////////////////////////
// setFilter
//

void CVSCPLog::setFilter(vscpEvent *pFilter) {

}


//////////////////////////////////////////////////////////////////////
// setMask
//

void CVSCPLog::setMask(vscpEvent *pMask) {

}


//////////////////////////////////////////////////////////////////////
// openFile
//

bool CVSCPLog::openFile(void) {
    if (m_flags & LOG_FILE_OVERWRITE) {
        if (m_file.Open(m_path, wxFile::write)) {
            m_pLogStream = new wxFileOutputStream(m_file);
            if (NULL != m_pLogStream) {
                if (m_pLogStream->IsOk()) {
                    if (m_flags & LOG_FILE_VSCP_WORKS) {
                        m_pLogStream->Write("<?xml version = \"1.0\" encoding = \"UTF-8\" ?>\n",
                                strlen("<?xml version = \"1.0\" encoding = \"UTF-8\" ?>\n"));
                        // RX data start
                        m_pLogStream->Write("<vscprxdata>\n", strlen("<vscprxdata>\n"));
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
                        m_pLogStream->Write("<?xml version = \"1.0\" encoding = \"UTF-8\" ?>\n",
                                strlen("<?xml version = \"1.0\" encoding = \"UTF-8\" ?>\n"));
                        // RX data start
                        m_pLogStream->Write("<vscprxdata>\n", strlen("<vscprxdata>\n"));
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
// writeEvent
//

bool CVSCPLog::writeEvent(vscpEvent *pEvent) {
    if (m_flags & LOG_FILE_VSCP_WORKS) {

        wxString str;

        // VSCP Works log format

        // Event
        m_pLogStream->Write("<event>\n", strlen("<event>\n"));
        m_pLogStream->Write("rx", strlen("rx"));
        m_pLogStream->Write("</dir>\n", strlen("</dir>\n"));

        m_pLogStream->Write("<time>", strlen("<time>"));
        str = wxDateTime::Now().FormatISODate() + _(" ") + wxDateTime::Now().FormatISOTime();
        m_pLogStream->Write(str.mb_str(), strlen(str.mb_str()));
        m_pLogStream->Write("</time>\n", strlen("</time>\n"));

        m_pLogStream->Write("<head>", strlen("<head>"));
        str.Printf(_("%d"), pEvent->head);
        m_pLogStream->Write(str.mb_str(), strlen(str.mb_str()));
        m_pLogStream->Write("</head>\n", strlen("</head>\n"));

        m_pLogStream->Write("<class>", strlen("<class>"));
        str.Printf(_("%d"), pEvent->vscp_class);
        m_pLogStream->Write(str.mb_str(), strlen(str.mb_str()));
        m_pLogStream->Write("</class>\n", strlen("</class>\n"));

        m_pLogStream->Write("<type>", strlen("<type>"));
        str.Printf(_("%d"), pEvent->vscp_type);
        m_pLogStream->Write(str.mb_str(), strlen(str.mb_str()));
        m_pLogStream->Write("</type>\n", strlen("</type>\n"));

        m_pLogStream->Write("<guid>", strlen("<guid>"));
        writeGuidToString(pEvent, str);
        m_pLogStream->Write(str.mb_str(), strlen(str.mb_str()));
        m_pLogStream->Write("</guid>\n", strlen("</guid>\n"));

        m_pLogStream->Write("<sizedata>", strlen("<sizedata>")); // Not used by read routine	
        str.Printf(_("%d"), pEvent->sizeData);
        m_pLogStream->Write(str.mb_str(), strlen(str.mb_str()));
        m_pLogStream->Write("</sizedata>\n", strlen("</sizedata>\n"));

        if (0 != pEvent->sizeData) {
            m_pLogStream->Write("<data>", strlen("<data>"));
            writeVscpDataToString(pEvent, str);
            m_pLogStream->Write(str.mb_str(), strlen(str.mb_str()));
            m_pLogStream->Write("</data>\n", strlen("</data>\n"));
        }

        m_pLogStream->Write("<timestamp>", strlen("<timestamp>"));
        str.Printf(_("%lu"), pEvent->timestamp);
        m_pLogStream->Write(str.mb_str(), strlen(str.mb_str()));
        m_pLogStream->Write("</timestamp>\n", strlen("</timestamp>\n"));

        m_pLogStream->Write("<note>", strlen("<note>"));
        m_pLogStream->Write("</note>\n", strlen("</note>\n"));

        m_pLogStream->Write("</event>\n", strlen("</event>\n"));

    } else {
        // Standard log format
        wxString str;

        str = wxDateTime::Now().FormatISODate() + _(" ") + wxDateTime::Now().FormatISOTime() + _(": ");
        m_pLogStream->Write(str.mb_str(), strlen(str.mb_str()));

        str.Printf(_("head=%d "), pEvent->head);
        m_pLogStream->Write(str.mb_str(), strlen(str.mb_str()));

        str.Printf(_("class=%d "), pEvent->vscp_class);
        m_pLogStream->Write(str.mb_str(), strlen(str.mb_str()));

        str.Printf(_("type=%d "), pEvent->vscp_type);
        m_pLogStream->Write(str.mb_str(), strlen(str.mb_str()));

        str.Printf(_("GUID="), pEvent->vscp_type);
        m_pLogStream->Write(str.mb_str(), strlen(str.mb_str()));

        writeGuidToString(pEvent, str);
        m_pLogStream->Write(str.mb_str(), strlen(str.mb_str()));

        str.Printf(_("datasize=%d "), pEvent->sizeData);
        m_pLogStream->Write(str.mb_str(), strlen(str.mb_str()));

        if (0 != pEvent->sizeData) {
            str.Printf(_("data="), pEvent->vscp_type);
            m_pLogStream->Write(str.mb_str(), strlen(str.mb_str()));
            writeVscpDataToString(pEvent, str);
            m_pLogStream->Write(str.mb_str(), strlen(str.mb_str()));
        }

        str.Printf(_("Timestamp=%lu\r\n"), pEvent->timestamp);
        m_pLogStream->Write(str.mb_str(), strlen(str.mb_str()));

    }

    return true;
}




//////////////////////////////////////////////////////////////////////
//                           Workerthread
//////////////////////////////////////////////////////////////////////

CVSCPLogWrkTread::CVSCPLogWrkTread() {
    m_pLog = NULL;
}

CVSCPLogWrkTread::~CVSCPLogWrkTread() {

}


//////////////////////////////////////////////////////////////////////
// Entry
//

void *CVSCPLogWrkTread::Entry() {
    // Check pointers
    if (NULL == m_pLog) return NULL;

    // First log on to the host and get configuration 
    // variables

    if (m_srv.doCmdOpen(m_pLog->m_host,
            m_pLog->m_port,
            m_pLog->m_username,
            m_pLog->m_password) <= 0) {
        return NULL;
    }

    // Find the channel id
    uint32_t ChannelID;
    m_srv.doCmdGetChannelID(&ChannelID);

    // It is possible that there is configuration data the server holds 
    // that we need to read in. 
    // We look for 
    //      prefix_filter to find a filter. A string is expected.
    //      prefix_mask to find a mask. A string is expected.
    //      prefix_path to overide the file path for the log file. A string is expected.
    //      prefix_rewrite to override the overwrite flag. A bool is expected.


    // Get filter data
    wxString varFilter;
    wxString varMask;

    if (m_srv.getVariableString(m_pLog->m_prefix + _T("_filter"), &varFilter) &&
            m_srv.getVariableString(m_pLog->m_prefix + _T("_mask"), &varMask)) {
        m_srv.doCmdFilter(varFilter, varMask);
    }

    // get overrided file path
    wxString varPath;
    if (m_srv.getVariableString(m_pLog->m_prefix + _T("_path"), &varPath)) {
        m_pLog->m_path = varPath;
    }

    bool bOverwrite;
    if (m_srv.getVariableBool(m_pLog->m_prefix + _T("_rewrite"), &bOverwrite)) {
        if (bOverwrite) {
            m_pLog->m_flags |= LOG_FILE_OVERWRITE;
        } else {
            m_pLog->m_flags &= ~LOG_FILE_OVERWRITE;
        }
    }

    bool bVSCPWorksFormat;
    if (m_srv.getVariableBool(m_pLog->m_prefix + _T("_vscpworksfmt"), &bVSCPWorksFormat)) {
        if (bVSCPWorksFormat) {
            m_pLog->m_flags |= LOG_FILE_VSCP_WORKS;
        } else {
            m_pLog->m_flags &= ~LOG_FILE_VSCP_WORKS;
        }
    }

    // Open the file
    if (!m_pLog->openFile()) return NULL;

    // Enter receive loop to start to log events
    m_srv.doCmdEnterReceiveLoop();

    int rv;
    vscpEvent event;
    while (!TestDestroy() && !m_pLog->m_bQuit) {

        if (CANAL_ERROR_SUCCESS ==
                (rv = m_srv.doCmdBlockReceive(&event, 1000))) {

            //pRecord->m_time = wxDateTime::Now();
            m_pLog->writeEvent(&event);

            // We are done with the event - remove data if any
            if (NULL != event.pdata) {
                delete [] event.pdata;
                event.pdata = NULL;
            }

        } // Event received
    } // Receive loop




    // Close the channel
    m_srv.doCmdClose();

    return NULL;
}

//////////////////////////////////////////////////////////////////////
// OnExit
//

void CVSCPLogWrkTread::OnExit() {

}