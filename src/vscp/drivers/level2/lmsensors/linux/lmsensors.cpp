// Lmsensors.cpp: implementation of the Clmsensors class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP Project (http://www.vscp.org) 
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
// CPU load
// http://stackoverflow.com/questions/3017162/how-to-get-total-cpu-usage-in-linux-c
//
// Memory usage
// http://stackoverflow.com/questions/669438/how-to-get-memory-usage-at-run-time-in-c
//

#ifdef WIN32

#include <stdio.h>

#else

#include "unistd.h"
#include "stdlib.h"
#include "limits.h"
#include "syslog.h"

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
#include "../../../../common/vscp_type.h"
#include "../../../../common/vscp_class.h"
#include "lmsensors.h"


//////////////////////////////////////////////////////////////////////
// Clmsensors
//

Clmsensors::Clmsensors()
{
	m_bQuit = false;
	m_pthreadWork = NULL;
    clearVSCPFilter(&m_vscpfilter); // Accept all events
	::wxInitialize();
}

//////////////////////////////////////////////////////////////////////
// ~Clmsensors
//

Clmsensors::~Clmsensors()
{
	close();
	::wxUninitialize();
}


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

	bool rv = true;
	wxString wxstr = wxString::FromAscii(pConfig);

	m_username = wxString::FromAscii(pUsername);
	m_password = wxString::FromAscii(pPassword);
	m_host = wxString::FromAscii(pHost);
	m_port = port;
	m_prefix = wxString::FromAscii(pPrefix);

	// Parse the configuration string. It should
	// have the following form
	// path
	// 
	wxStringTokenizer tkz(wxString::FromAscii(pConfig), _(";\n"));

	// Check for # of sensors in configuration string
	if (tkz.HasMoreTokens()) {
		// Path
		m_nSensors = readStringValue(tkz.GetNextToken());
	}

	// First log on to the host and get configuration 
	// variables

	if (m_srv.doCmdOpen(m_host,
			m_port,
			m_username,
			m_password) <= 0) {
		syslog(LOG_ERR,
				"%s",
				(const char *) "Unable to connect to VSCP TCP/IP interface. Terminating!");
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
	int varNumberOfSensors;

	wxString strNumberOfSensors = m_prefix +
			wxString::FromAscii("_numberofsensors");

	if (!m_srv.getVariableInt(strNumberOfSensors, &varNumberOfSensors)) {
		// The variable must be available - terminate
		syslog(LOG_ERR,
				"%s",
				(const char *) "The variable prefix+NumberOfSensors is not available. Terminating!");
		// Close the channel
		m_srv.doCmdClose();
		return false;
	}

	if (0 == varNumberOfSensors) {
		syslog(LOG_ERR,
				"%s",
				(const char *) "NumberOfSensors is zero. Must be at least one sensor. Terminating!");
		// Close the channel
		m_srv.doCmdClose();
		return false;
	}

	// Read in the configuration values for each sensor
	for (int i = 0; i < varNumberOfSensors; i++) {

		wxString strIteration;
		strIteration.Printf(_("%d"), i);
		strIteration.Trim();

		CWrkTread *pthreadWork = new CWrkTread();
		if (NULL != pthreadWork) {

			// Get the path
			wxString strVariableName = m_prefix +
					wxString::FromAscii("_path") + strIteration;
			if (!m_srv.getVariableString(strVariableName, &pthreadWork->m_path)) {
				syslog(LOG_ERR,
						"%s prefix=%s i=%d",
						(const char *) "Failed to read variable _path.",
						(const char *)m_prefix.ToAscii(),
						i);
			}

			// Get GUID
			strVariableName = m_prefix +
					wxString::FromAscii("_path") + strIteration;
			if (!m_srv.getVariableGUID(strVariableName, pthreadWork->m_guid)) {
				syslog(LOG_ERR,
						"%s prefix=%s i=%d",
						(const char *) "Failed to read variable _guid.",
						(const char *)m_prefix.ToAscii(),
						i);
			}

			// Get sample interval
			strVariableName = m_prefix +
					wxString::FromAscii("_interval") + strIteration;
			if (!m_srv.getVariableInt(strVariableName, &pthreadWork->m_interval)) {
				syslog(LOG_ERR,
						"%s prefix=%s i=%d",
						(const char *) "Failed to read variable _interval.",
						(const char *)m_prefix.ToAscii(),
						i);
			}

			// Get VSCP type
			strVariableName = m_prefix +
					wxString::FromAscii("_vscptype") + strIteration;
			if (!m_srv.getVariableInt(strVariableName,
					&pthreadWork->m_vscptype)) {
				syslog(LOG_ERR,
						"%s prefix=%s i=%d",
						(const char *) "Failed to read variable _vscptype.",
						(const char *)m_prefix.ToAscii(),
						i);
			}

			// Get measurement coding (first data byte)
			strVariableName = m_prefix +
					wxString::FromAscii("_coding") + strIteration;
			if (!m_srv.getVariableInt(strVariableName,
					&pthreadWork->m_datacoding)) {
				syslog(LOG_ERR,
						"%s prefix=%s i=%d",
						(const char *) "Failed to read variable _coding.",
						(const char *)m_prefix.ToAscii(),
						i);
			}

			// Get divide value
			strVariableName = m_prefix +
					wxString::FromAscii("_divide") + strIteration;
			if (!m_srv.getVariableDouble(strVariableName,
					&pthreadWork->m_divideValue)) {
				syslog(LOG_ERR,
						"%s prefix=%s i=%d",
						(const char *) "Failed to read variable _divide.",
						(const char *)m_prefix.ToAscii(),
						i);
			}

			// Get multiply value
			strVariableName = m_prefix +
					wxString::FromAscii("_multiply") + strIteration;
			if (!m_srv.getVariableDouble(strVariableName,
					&pthreadWork->m_multiplyValue)) {
				syslog(LOG_ERR,
						"%s prefix=%s i=%d",
						(const char *) "Failed to read variable _multiply.",
						(const char *)m_prefix.ToAscii(),
						i);
			}

			// start the workerthread
			pthreadWork->m_pObj = this;
			pthreadWork->Create();
			pthreadWork->Run();

		}
		else {
			syslog(LOG_ERR,
					"%s prefix=%s i=%d",
					(const char *) "Failed to start workerthread.",
					(const char *)m_prefix.ToAscii(),
					i);
			rv = false;
		}
	}

	// Close the channel
	m_srv.doCmdClose();

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
	wxSleep(1); // Give the thread some time to terminate

}

//////////////////////////////////////////////////////////////////////
// addEvent2SendQueue
//

bool 
Clmsensors::addEvent2SendQueue(const vscpEvent *pEvent)
{
    m_mutexSendQueue.Lock();
	//m_sendQueue.Append((vscpEvent *)pEvent);
    m_sendList.push_back((vscpEvent *)pEvent);
	m_semSendQueue.Post();
	m_mutexSendQueue.Unlock();
    return true;
}

//////////////////////////////////////////////////////////////////////
//                           Workerthread
//////////////////////////////////////////////////////////////////////

CWrkTread::CWrkTread()
{
	m_pObj = NULL;
	m_path.Empty();
	m_guid.clear(); // Interface GUID
	m_interval = DEFAULT_INTERVAL; 
	m_vscpclass = VSCP_CLASS1_MEASUREMENT;
	m_vscptype = 0;
	m_datacoding = VSCP_TYPE_MEASUREMENT_TEMPERATURE;
	m_divideValue = 0;		// Zero means ignore
	m_multiplyValue = 0;	// Zero means ignore
}

CWrkTread::~CWrkTread()
{
	;
}


//////////////////////////////////////////////////////////////////////
// Entry
//

void *
CWrkTread::Entry()
{
	// Check pointers
	if (NULL == m_pObj) return NULL;

/*    
	if (m_srv.doCmdOpen(m_pObj->m_host,
			m_pObj->m_port,
			m_pObj->m_username,
			m_pObj->m_password) <= 0) {
		syslog(LOG_ERR,
				"%s",
				(const char *) "Workerthread. Unable to connect to VSCP TCP/IP interface. Terminating!");
		return NULL;
	}

	// Find the channel id
	uint32_t ChannelID;
	m_srv.doCmdGetChannelID(&ChannelID);
*/
    
	// Open the file
	wxFile file;
	if (!file.Open(m_path)) {
		syslog(LOG_ERR,
				"%s",
				(const char *) "Workerthread. File to open lmsensors file. Terminating!");
		// Close the channel
		m_srv.doCmdClose();
		return NULL;
	}

	char buf[1024];
	long val;
	while (!TestDestroy() && !m_pObj->m_bQuit) {

		memset(buf, 0, sizeof(buf));
		file.Seek(0);
		if (wxInvalidOffset != file.Read(buf, sizeof(buf))) {

			wxString str = wxString::FromAscii(buf);
			str.ToLong(&val);

			if (m_divideValue) {
				val = val / m_divideValue;
			}

			if (m_multiplyValue) {
				val = val * m_multiplyValue;
			}

			bool bNegative = false;
			if (val < 0) {
				bNegative = true;
				val = abs(val);
			}

            vscpEvent *pEvent = new vscpEvent();
            if (NULL != pEvent) {

                pEvent->pdata = NULL;
                pEvent->sizeData = 0;
                
                m_guid.setGUID(pEvent->GUID);
                pEvent->vscp_class = VSCP_CLASS1_MEASUREMENT;
                pEvent->vscp_type = m_vscptype;
                if (val < 0xff) {
                    pEvent->sizeData = 2;
                    pEvent->pdata = new uint8_t[2];
                    if ( NULL != pEvent->pdata ) {
                        pEvent->pdata[1] = val;
                    }
                } else if (val < 0xffff) {
                    pEvent->sizeData = 3;
                    pEvent->pdata = new uint8_t[3];
                    if ( NULL != pEvent->pdata ) {
                        pEvent->pdata[1] = (val >> 8) & 0xff;
                        pEvent->pdata[2] = val & 0xff;
                    }
                } else if (val < 0xffffff) {
                    pEvent->sizeData = 4;
                    pEvent->pdata = new uint8_t[4];
                    if ( NULL != pEvent->pdata ) {
                        pEvent->pdata[1] = (val >> 16) & 0xff;
                        pEvent->pdata[2] = (val >> 8) & 0xff;
                        pEvent->pdata[3] = val & 0xff;
                    }
                } else {
                    pEvent->sizeData = 5;
                    pEvent->pdata = new uint8_t[5];
                    if ( NULL != pEvent->pdata ) {
                        pEvent->pdata[1] = (val >> 24) & 0xff;
                        pEvent->pdata[2] = (val >> 16) & 0xff;
                        pEvent->pdata[3] = (val >> 8) & 0xff;
                        pEvent->pdata[4] = val & 0xff;
                    }
                }
                
                if ( NULL != pEvent->pdata ) {
                    pEvent->pdata[0] = m_datacoding;
                }

                if (doLevel2Filter(pEvent, &m_pObj->m_vscpfilter)) {
                    m_pObj->m_mutexReceiveQueue.Lock();
                    m_pObj->m_receiveList.push_back(pEvent);
                    m_pObj->m_semReceiveQueue.Post();
                    m_pObj->m_mutexReceiveQueue.Unlock();
                }
                else {
                    deleteVSCPevent(pEvent);
                }
            }

		}

		::wxSleep(m_interval ? m_interval : 1);
	}

	// Close the file
	file.Close();

	// Close the channel
	//m_srv.doCmdClose();

	return NULL;

}

//////////////////////////////////////////////////////////////////////
// OnExit
//

void
CWrkTread::OnExit()
{
	;
}
