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
#include "lmsensors.h"

// Lists
WX_DEFINE_LIST(VSCPEVENTLIST);
WX_DEFINE_LIST(VSCPSENSORDATALIST);

//////////////////////////////////////////////////////////////////////
// ClmSensorData
//

ClmSensorData::ClmSensorData()
{
	m_path.Empty();
	m_guid.clear(); // Interface GUID
	m_interval = 0; // Disabled
	m_measurement_data_coding = 0;
	m_divideValue = 1;
	m_multiplyValue = 1;
}

//////////////////////////////////////////////////////////////////////
// ~ClmSensorData
//

ClmSensorData::~ClmSensorData()
{

}

//////////////////////////////////////////////////////////////////////
// Clmsensors
//

Clmsensors::Clmsensors()
{
	m_bQuit = false;
	m_pthreadWork = NULL;
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
	//	 _numberofsensors - This is the number of sensors that the driver is supposed
	//	  to read. Sensors are numbered staring with zero. 
	//
	//   _guidn - GUID for sensor n. guid0, guid1, guid2 etc
	//	 _pathn - Path to value for example 
	//				/sys/class/hwmon/hwmon0/temp1_input
	//	 		 which is CPU temp for core 1
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

		wxString strIteration = wxString::FromAscii(itoa(i));
		strIteration.Trim();

		ClmSensorData *pData = new ClmSensorData();
		if (NULL != pData) {

			// Get the path
			wxString strVariableName = m_prefix +
					wxString::FromAscii("_path") + strIteration;
			if (!m_srv.getVariableString(strVariableName, &pData->m_path)) {
				syslog(LOG_ERR,
						"%s prefix=%s i=%d",
						(const char *) "Failed to read variable _path.",
						m_prefix.ToAscii(),
						i);
			}

			// Get GUID
			strVariableName = m_prefix +
					wxString::FromAscii("_path") + strIteration;
			if (!m_srv.getVariableGUID(strVariableName, &pData->m_guid)) {
				syslog(LOG_ERR,
						"%s prefix=%s i=%d",
						(const char *) "Failed to read variable _guid.",
						m_prefix.ToAscii(),
						i);
			}

			// Get sample interval
			strVariableName = m_prefix +
					wxString::FromAscii("_interval") + strIteration;
			if (!m_srv.getVariableInt(strVariableName, &pData->m_interval)) {
				syslog(LOG_ERR,
						"%s prefix=%s i=%d",
						(const char *) "Failed to read variable _interval.",
						m_prefix.ToAscii(),
						i);
			}

			// Get VSCP type
			strVariableName = m_prefix +
					wxString::FromAscii("_vscptype") + strIteration;
			if (!m_srv.getVariableInt(strVariableName,
					&pData->m_vscptype)) {
				syslog(LOG_ERR,
						"%s prefix=%s i=%d",
						(const char *) "Failed to read variable _vscptype.",
						m_prefix.ToAscii(),
						i);
			}
			
			// Get measurement coding (first data byte)
			strVariableName = m_prefix +
					wxString::FromAscii("_coding") + strIteration;
			if (!m_srv.getVariableInt(strVariableName,
					&pData->m_coding)) {
				syslog(LOG_ERR,
						"%s prefix=%s i=%d",
						(const char *) "Failed to read variable _coding.",
						m_prefix.ToAscii(),
						i);
			}

			// Get divide value
			strVariableName = m_prefix +
					wxString::FromAscii("_divide") + strIteration;
			if (!m_srv.getVariableDouble(strVariableName,
					&pData->m_divideValue)) {
				syslog(LOG_ERR,
						"%s prefix=%s i=%d",
						(const char *) "Failed to read variable _divide.",
						m_prefix.ToAscii(),
						i);
			}

			// Get multiply value
			strVariableName = m_prefix +
					wxString::FromAscii("_multiply") + strIteration;
			if (!m_srv.getVariableDouble(strVariableName,
					&pData->m_multiplyValue)) {
				syslog(LOG_ERR,
						"%s prefix=%s i=%d",
						(const char *) "Failed to read variable _multiply.",
						m_prefix.ToAscii(),
						i);
			}
			
			// start the workerthread
			CWrkTread *pthreadWork = new CWrkTread();
			if (NULL != pthreadWork) {
				pthreadWork->m_pObj = this;
				pthreadWork->m_pData = pData;
				pthreadWork->Create();
				pthreadWork->Run();
			} else {
				syslog(LOG_ERR,
						"%s prefix=%s i=%d",
						(const char *) "Failed to start workerthread.",
						m_prefix.ToAscii(),
						i);
				rv = false;
			}

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
//                           Workerthread
//////////////////////////////////////////////////////////////////////

CWrkTread::CWrkTread()
{
	m_pObj = NULL;
}

CWrkTread::~CWrkTread()
{
}


//////////////////////////////////////////////////////////////////////
// Entry
//

void *
CWrkTread::Entry()
{
	// Check pointers
	if (NULL == m_pObj) return NULL;

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
	
	// Open the file
	wxFile file;
	if ( !file.Open(m_pData->m_path)) {
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
		
		memset( 0, buf, sizeof(buf));
		file.Seek(0);
		if ( wxInvalidOffset != file.Read(buf, sizeof(buf)) ) {
			
			wxString str = wxString::FromAscii(buf);
			str.ToLong(&val);
			
			if (m_pData->m_divideValue) {
				val = val/m_pData->m_divideValue;
			}
			
			if (m_pData->m_multiplyValue) {
				val = val*m_pData->m_multiplyValue;
			}
			
			bool bNegative = false;
			if ( val < 0 ) {
				bNegative = true;
				val = abs(val);
			}
			
			vscpEventEx event;
			event.sizeData = 0;
			event.data[0] = m_pData->m_coding;
			event.GUID = m_pData->m_guid;
			event.vscp_class = VSCP_CLASS1_MEASUREMENT;
			event.vscp_type = m_pData->m_vscptype;
			if ( val<0xff ) {
				event.sizeData = 2;
				event.data[1] = val; 
			}
			else if ( val<0xffff ) {
				event.sizeData = 3;
				event.data[1] = (val >> 8) & 0xff;
				event.data[2] = val & 0xff;
			}
			else if ( val<0xffffff ) {
				event.sizeData = 4;
				event.data[1] = (val >> 16) & 0xff;
				event.data[2] = (val >> 8) & 0xff;
				event.data[3] = val & 0xff;
			}
			else {
				event.sizeData = 5;
				event.data[1] = (val >> 24) & 0xff;
				event.data[2] = (val >> 16) & 0xff;
				event.data[3] = (val >> 8) & 0xff;
				event.data[4] = val & 0xff;
			}
			
		}
		
		::wxSleep( m_pData->m_interval ? m_pData->m_interval : 1 );
	}
	
	// Close the file
	file.Close();
}

// Close the channel
m_srv.doCmdClose();

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