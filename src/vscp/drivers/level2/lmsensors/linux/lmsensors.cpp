// Lmsensors.cpp: implementation of the Clmsensors class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP Project (http://www.vscp.org) 
//
// Copyright (C) 2000-2017 Ake Hedman, 
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
#include <vscphelper.h>
#include <vscpremotetcpif.h>
#include <vscp_type.h>
#include <vscp_class.h>
#include "lmsensors.h"


//////////////////////////////////////////////////////////////////////
// Clmsensors
//

Clmsensors::Clmsensors()
{
	m_bQuit = false;
	m_pthreadWork = NULL;
    vscp_clearVSCPFilter(&m_vscpfilter); // Accept all events
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
		m_nSensors = vscp_readStringValue(tkz.GetNextToken());
	}

	// First log on to the host and get configuration 
	// variables

	if ( VSCP_ERROR_SUCCESS !=  m_srv.doCmdOpen( m_host,
                                                    m_username,
                                                    m_password )) {
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
	int varNumberOfSensors = 0;

	wxString strNumberOfSensors = m_prefix +
			wxString::FromAscii("_numberofsensors");

	if (VSCP_ERROR_SUCCESS != m_srv.getRemoteVariableInt(strNumberOfSensors, &varNumberOfSensors)) {
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
			if (VSCP_ERROR_SUCCESS != m_srv.getRemoteVariableValue(strVariableName, pthreadWork->m_path)) {
				syslog(LOG_ERR,
						"%s prefix=%s i=%d",
						(const char *) "Failed to read variable _path.",
						(const char *)m_prefix.ToAscii(),
						i);
			}

			// Get GUID
			strVariableName = m_prefix +
					wxString::FromAscii("_guid") + strIteration;
			if (VSCP_ERROR_SUCCESS != m_srv.getRemoteVariableGUID(strVariableName, pthreadWork->m_guid)) {
				syslog(LOG_ERR,
						"%s prefix=%s i=%d",
						(const char *) "Failed to read variable _guid.",
						(const char *)m_prefix.ToAscii(),
						i);
			}

			// Get sample interval
			strVariableName = m_prefix +
					wxString::FromAscii("_interval") + strIteration;
			if (VSCP_ERROR_SUCCESS != m_srv.getRemoteVariableInt(strVariableName, &pthreadWork->m_interval)) {
				syslog(LOG_ERR,
						"%s prefix=%s i=%d",
						(const char *) "Failed to read variable _interval.",
						(const char *)m_prefix.ToAscii(),
						i);
			}

			// Get VSCP type
			strVariableName = m_prefix +
					wxString::FromAscii("_vscptype") + strIteration;
			if (VSCP_ERROR_SUCCESS != m_srv.getRemoteVariableInt(strVariableName,
					&pthreadWork->m_vscptype)) {
				syslog(LOG_ERR,
						"%s prefix=%s i=%d",
						(const char *) "Failed to read variable _vscptype.",
						(const char *)m_prefix.ToAscii(),
						i);
			}

			// Get measurement coding (first data byte)
			strVariableName = m_prefix +
					wxString::FromAscii("_datacoding") + strIteration;
			if (VSCP_ERROR_SUCCESS != m_srv.getRemoteVariableInt(strVariableName,
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
			if (VSCP_ERROR_SUCCESS != m_srv.getRemoteVariableDouble(strVariableName,
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
			if (VSCP_ERROR_SUCCESS != m_srv.getRemoteVariableDouble(strVariableName,
					&pthreadWork->m_multiplyValue)) {
				syslog(LOG_ERR,
						"%s prefix=%s i=%d",
						(const char *) "Failed to read variable _multiply.",
						(const char *)m_prefix.ToAscii(),
						i);
			}
            
            // Get read offset
			strVariableName = m_prefix +
					wxString::FromAscii("_readoffset") + strIteration;
			if (VSCP_ERROR_SUCCESS != m_srv.getRemoteVariableInt(strVariableName,
					&pthreadWork->m_readOffset)) {
				syslog(LOG_ERR,
						"%s prefix=%s i=%d",
						(const char *) "Failed to read variable _readoffset.",
						(const char *)m_prefix.ToAscii(),
						i);
			}
            
            // Get index
			strVariableName = m_prefix +
					wxString::FromAscii("_index") + strIteration;
			if (VSCP_ERROR_SUCCESS != m_srv.getRemoteVariableInt(strVariableName,
					&pthreadWork->m_index)) {
				syslog(LOG_ERR,
						"%s prefix=%s i=%d",
						(const char *) "Failed to read variable _index.",
						(const char *)m_prefix.ToAscii(),
						i);
			}
            
            // Get zone
			strVariableName = m_prefix +
					wxString::FromAscii("_zone") + strIteration;
			if (VSCP_ERROR_SUCCESS != m_srv.getRemoteVariableInt(strVariableName,
					&pthreadWork->m_zone)) {
				syslog(LOG_ERR,
						"%s prefix=%s i=%d",
						(const char *) "Failed to read variable _zone.",
						(const char *)m_prefix.ToAscii(),
						i);
			}            
            
            // Get subzone
			strVariableName = m_prefix +
					wxString::FromAscii("_subzone") + strIteration;
			if (VSCP_ERROR_SUCCESS != m_srv.getRemoteVariableInt(strVariableName,
					&pthreadWork->m_subzone)) {
				syslog(LOG_ERR,
						"%s prefix=%s i=%d",
						(const char *) "Failed to read variable _subzone.",
						(const char *)m_prefix.ToAscii(),
						i);
			}
            
            // Get unit
			strVariableName = m_prefix +
					wxString::FromAscii("_unit") + strIteration;
			if (VSCP_ERROR_SUCCESS != m_srv.getRemoteVariableInt(strVariableName,
					&pthreadWork->m_unit)) {
				syslog(LOG_ERR,
						"%s prefix=%s i=%d",
						(const char *) "Failed to read variable _unit.",
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
    m_readOffset = 0;       // Read numerical at offset 0
    m_index = 0;
	m_zone = 0;
	m_subzone = 0;
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
	double val;
	while (!TestDestroy() && !m_pObj->m_bQuit) {

		memset(buf, 0, sizeof(buf));
		file.Seek( m_readOffset );
		if (wxInvalidOffset != file.Read(buf, sizeof(buf))) {

			wxString str = wxString::FromAscii(buf);
            val = atof( buf );

			if (m_divideValue) {
				val = val / m_divideValue;
			}

			if (m_multiplyValue) {
				val = val * m_multiplyValue;
			}

			bool bNegative = false;
			if (val < 0) {
				bNegative = true;
				val = (val< 0) ? -1.0 * val : val;
            }

            vscpEvent *pEvent = new vscpEvent();
            if (NULL != pEvent) {

                pEvent->pdata = NULL;
                pEvent->sizeData = 0;

                m_guid.writeGUID(pEvent->GUID);
                pEvent->vscp_class = m_vscpclass; // VSCP_CLASS1_MEASUREMENT;
                pEvent->vscp_type = m_vscptype;

				pEvent->timestamp = vscp_makeTimeStamp();
            	vscp_setEventDateTimeBlockToNow( pEvent );

                if (VSCP_CLASS1_MEASUREMENT == m_vscpclass) {

                    switch (m_datacoding & VSCP_MASK_DATACODING_TYPE) {

                    case VSCP_DATACODING_BIT:
                    case VSCP_DATACODING_BYTE:
                    case VSCP_DATACODING_INTEGER:
                        if (val < 0xff) {
                            pEvent->sizeData = 2;
                            pEvent->pdata = new uint8_t[2];
                            if (NULL != pEvent->pdata) {
                                pEvent->pdata[1] = val;

                                // Set data coding
                                pEvent->pdata[0] = m_datacoding;
                            } else {
                                pEvent->sizeData = 0;
                                pEvent->pdata = NULL;
                            }
                        }
                        else if (val < 0xffff) {
                            pEvent->sizeData = 3;
                            pEvent->pdata = new uint8_t[3];
                            if (NULL != pEvent->pdata) {
                                long lval = val;
                                pEvent->pdata[1] = (lval >> 8) & 0xff;
                                pEvent->pdata[2] = lval & 0xff;

                                // Set data coding
                                pEvent->pdata[0] = m_datacoding;
                            } else {
                                pEvent->sizeData = 0;
                                pEvent->pdata = NULL;
                            }
                        }
                        else if (val < 0xffffff) {
                            pEvent->sizeData = 4;
                            pEvent->pdata = new uint8_t[4];
                            if (NULL != pEvent->pdata) {
                                long lval = val;
                                pEvent->pdata[1] = (lval >> 16) & 0xff;
                                pEvent->pdata[2] = (lval >> 8) & 0xff;
                                pEvent->pdata[3] = lval & 0xff;

                                // Set data coding
                                pEvent->pdata[0] = m_datacoding;
                            } else {
                                pEvent->sizeData = 0;
                                pEvent->pdata = NULL;
                            }
                        }
                        else {
                            pEvent->sizeData = 5;
                            pEvent->pdata = new uint8_t[5];
                            if (NULL != pEvent->pdata) {
                                long lval = val;
                                pEvent->pdata[1] = (lval >> 24) & 0xff;
                                pEvent->pdata[2] = (lval >> 16) & 0xff;
                                pEvent->pdata[3] = (lval >> 8) & 0xff;
                                pEvent->pdata[4] = lval & 0xff;

                                // Set data coding
                                pEvent->pdata[0] = m_datacoding;
                            } else {
                                pEvent->sizeData = 0;
                                pEvent->pdata = NULL;
                            }
                        }
                        break;

                    case VSCP_DATACODING_NORMALIZED:
                    {
                        uint8_t buf[8];
                        uint16_t size;
                        if ( vscp_convertFloatToNormalizedEventData( buf,
                                &size,
								val,
                                ((m_datacoding >> 3 & 3)),
                                (m_datacoding & 7)) && (0 != size ) ) {
                            
                            pEvent->pdata = new uint8_t[size];
                            if (NULL != pEvent->pdata) {
                                pEvent->sizeData = size;
                                memcpy( pEvent->pdata, buf, size);
                            }
                            else {
                                pEvent->sizeData = 0;
                                pEvent->pdata = NULL;
                            }
                        }
                    }
                        break;

                    case VSCP_DATACODING_STRING:
                    {
                        pEvent->sizeData = 8;
                        pEvent->pdata = new uint8_t[8];

                        if (NULL != pEvent->pdata) {
                            wxString str;
                            str.Printf(_("%lf"), val);
                            if (str.Length() > 7) {
                                str = str.Left(7);
                            }

                            strcpy((char *) (pEvent->pdata + 1), str.ToAscii());

                        } else {
                            pEvent->sizeData = 0;
                            pEvent->pdata = NULL;
                        }

                        // Set data coding
                        pEvent->pdata[0] = m_datacoding;

                    }
                        break;

                    case VSCP_DATACODING_SINGLE:
                    {
                        pEvent->sizeData = 7;
                        pEvent->pdata = new uint8_t[7];

                        if (NULL != pEvent->pdata) {
                            uint8_t buf[6];
                            float f = (float) val;
                            uint8_t *p = (uint8_t *) & f;
                            memcpy(buf, p, 6);
                            // If on a little endian platform we
                            // have to change byte order
                            if (wxIsPlatformLittleEndian()) {
                                for (int i = 0; i < 6; i++) {
                                    pEvent->pdata[1 + i] = buf[5 - i];
                                }
                            } else {
                                memcpy(pEvent->pdata + 1, buf, 6);
                            }

                            // Set data coding
                            pEvent->pdata[0] = m_datacoding;
                        } else {
                            pEvent->sizeData = 0;
                            pEvent->pdata = NULL;
                        }
                    }
                        break;
                    } // switch

                }
                else if ((VSCP_CLASS1_MEASUREZONE == m_vscpclass) ||
                        (VSCP_CLASS1_SETVALUEZONE == m_vscpclass)) {

                    // We pretend we are a standard measurement
                    uint8_t buf[8];
                    uint16_t size;
                    vscp_convertFloatToNormalizedEventData( buf,
                            &size,
							val,
                            ((m_datacoding >> 3 & 3)),
                            (m_datacoding & 7));

                    pEvent->pdata = new uint8_t[size];
                    if (NULL != pEvent->pdata) {
                        if ( size <= 5 ) {
                            pEvent->sizeData = size;
                            memcpy(pEvent->pdata+3, buf, size);
                        }
                        else {
                            delete pEvent->pdata;
                            pEvent->sizeData = 0;
                            pEvent->pdata = NULL;
                        }
                    } 
                    else {
                        pEvent->sizeData = 0;
                        pEvent->pdata = NULL;
                    }
                    
                }
                else if (VSCP_CLASS1_MEASUREMENT64 == m_vscpclass) {
                    uint8_t buf[8];
                    {
                        uint8_t *p = (uint8_t *)&val;
                        memcpy( buf, p, 8 );
                        // If on a little endian platform we
                        // have to change byte order
                        if ( wxIsPlatformLittleEndian() ) {
                            for ( int i=0; i<8; i++ ) {
                                pEvent->pdata[i] = buf[7-i];
                            }
                        }
                        else {
                            memcpy( pEvent->pdata, buf, 8 );
                        }
                    }
                }
                else if (VSCP_CLASS2_MEASUREMENT_STR == m_vscpclass) {
                    wxString str;
                    str.Printf(_("%d,%d,%d,%lf"), 
                                    m_index, 
                                    m_zone, 
                                    m_subzone, 
                                    m_unit,
                                    val );
                    strcpy( (char *)pEvent->pdata, str.ToAscii() );
                }
                
                

                if (vscp_doLevel2Filter(pEvent, &m_pObj->m_vscpfilter)) {
                    m_pObj->m_mutexReceiveQueue.Lock();
                    m_pObj->m_receiveList.push_back(pEvent);
                    m_pObj->m_semReceiveQueue.Post();
                    m_pObj->m_mutexReceiveQueue.Unlock();
                }
                else {
                    vscp_deleteVSCPevent(pEvent);
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
