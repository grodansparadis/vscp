// socketcan.cpp: implementation of the Csocketcan class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP Project (http://www.vscp.org) 
//
// Copyright (C) 2000-2015 Ake Hedman, 
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

#include <stdio.h>
#include "unistd.h"
#include "stdlib.h"
#include <string.h>
#include "limits.h"
#include "syslog.h"
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

// Different on Kernel 2.6 and cansocket examples
// currently using locally from can-utils
// TODO remove include form makefile when they are in sync
#include <linux/can.h>
#include <linux/can/raw.h>

#include <signal.h>
#include <ctype.h>
#include <libgen.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <net/if.h>
#include <errno.h>

#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"
#include <wx/xml/xml.h>
#include <wx/listimpl.cpp>
#include <wx/thread.h>
#include <wx/tokenzr.h>
#include <wx/datetime.h>

#include <vscp.h>
#include <vscphelper.h>
#include <vscpremotetcpif.h>
#include <vscp_type.h>
#include <vscp_class.h>
#include "wire1.h"


//////////////////////////////////////////////////////////////////////
// CWire1
//

CWire1::CWire1()
{
	m_bQuit = false;
	m_pthreadWork = NULL;
	m_nSensors = 1;	// Default is one sensor
    vscp_clearVSCPFilter(&m_vscpfilter); // Accept all events
	::wxInitialize();
}

//////////////////////////////////////////////////////////////////////
// ~Csocketcan
//

CWire1::~CWire1()
{
	close();
	::wxUninitialize();
}


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
	bool rv = true;
	wxString wxstr = wxString::FromAscii(pConfig);
    wxString strVariableName;

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

	if ( VSCP_ERROR_SUCCESS != m_srv.doCmdOpen( m_host,
                                                    m_username,
                                                    m_password ) ) {
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

	// Get configuration data
	int varNumberOfSensors = 0;

	wxString strNumberOfSensors = m_prefix +
			wxString::FromAscii("_numberofsensors");

	if ( !m_srv.getVariableInt(strNumberOfSensors, &varNumberOfSensors ) ) {	
		// We use default 1 or config parameter supplied
	}

	// Read in the configuration values for each sensor
	for (int i = 0; i < varNumberOfSensors; i++) {

		wxString strIteration;
		strIteration.Printf(_("%d"), i);
		strIteration.Trim();

		CWrkTread *pthreadWork = new CWrkTread();
		if (NULL != pthreadWork) {
			
			// Get sample interval
			strVariableName = m_prefix +
					wxString::FromAscii("_interval") + strIteration;
			if (!m_srv.getVariableInt(strVariableName, &pthreadWork->m_interval)) {
				// Node need to log error as optional
			}
			
			// Disabled if interval is zero
			if ( 0 == pthreadWork->m_interval ) continue;

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
					wxString::FromAscii("_guid") + strIteration;
			if (!m_srv.getVariableGUID(strVariableName, pthreadWork->m_guid)) {
				// Node need to log error as optional
			}

			// Get sample interval
			strVariableName = m_prefix +
					wxString::FromAscii("_interval") + strIteration;
			if (!m_srv.getVariableInt(strVariableName, &pthreadWork->m_interval)) {
				// Node need to log error as optional
			}

			// Get unit
			strVariableName = m_prefix +
					wxString::FromAscii("_unit") + strIteration;
			if (!m_srv.getVariableInt(strVariableName, &pthreadWork->m_unit)) {
				// Node need to log error as optional
			}
			
			// Check read value
			if ( pthreadWork->m_unit > 3 ) pthreadWork->m_unit = 2; // Celsius
			
			// Get index
			strVariableName = m_prefix +
					wxString::FromAscii("_index") + strIteration;
			if (!m_srv.getVariableInt(strVariableName, &pthreadWork->m_unit)) {
				// Node need to log error as optional
			}
            
            // Get coding
			strVariableName = m_prefix +
					wxString::FromAscii("_coding") + strIteration;
			if (!m_srv.getVariableInt( strVariableName, &pthreadWork->m_coding)) {
				// Node need to log error as optional
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
CWire1::close(void)
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
CWire1::addEvent2SendQueue(const vscpEvent *pEvent)
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
    m_index = 0;
	m_unit = DEFAULT_UNIT;
    m_coding = DEFAULT_CODING;
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

	FILE * pFile;
	char line1[80];
	char line2[80];
	unsigned int id[9];
	int temperature;
	
	while (!TestDestroy() && !m_pObj->m_bQuit) {

		// Open the file
		if ( pFile = fopen( m_path , "r") ) {
			syslog(LOG_ERR,
				"%s",
				(const char *) "Workerthread. File to open 1-wire data file. Terminating!");
			// Close the channel
			m_srv.doCmdClose();
			return NULL;
        }
		
        // Read line 1
		if ( !fgets( line1, sizeof( line1 ), pFile ) ) {
			
			// Close the file
			fclose (pFile);
			
			syslog(LOG_ERR,
				"%s",
				(const char *) "Workerthread. Failed to read 1-wire data from file. Terminating!");
			
            // Close the channel
			m_srv.doCmdClose();
			return NULL;
		}
	
		// Read line 2
		if ( !fgets ( line2, sizeof( line2 ) , pFile ) ) {
			
            // Close the file
			fclose (pFile);
			
			syslog(LOG_ERR,
				"%s",
				(const char *) "Workerthread. Failed to read 1-wire data from file. Terminating!");
            
			// Close the channel
			m_srv.doCmdClose();
			return NULL;
		}
	
		// Close the file
		fclose (pFile);
		
		// 08 00 4b 46 ff ff 0d 10 ff : crc=ff YES
		sscanf( line1, "%02x %02x %02x %02x %02x %02x %02x %02x %02x",
				&id[0],&id[1],&id[2],&id[3],&id[4],&id[5],&id[6],&id[7],&id[8] );
				
		if ( NULL == strstr( line1, "YES" ) ) {
            wxSleep( 2 );   // We sleep before trying again.
			continue;       // Try again
		}	
		
		sscanf( line2, "%*s %*s %*s %*s %*s %*s %*s %*s %*s t=%d", &temperature );
				
        double val = temperature;
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
            pEvent->vscp_class = VSCP_CLASS1_MEASUREMENT;
            pEvent->vscp_type = VSCP_TYPE_MEASUREMENT_TEMPERATURE;

            switch ( m_coding ) {
                
                case 0:     // Normalized integer
                {
                    uint8_t buf[8];
                    uint16_t size;
                    if ( vscp_convertFloatToNormalizedEventData( buf,
                                        &size,
                                        val,
                                        m_unit,
                                        m_index ) ) {
                            
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

                case 1:     // String
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

                    } 
                    else {
                       pEvent->sizeData = 0;
                       pEvent->pdata = NULL;
                    }

                    // Set data coding
                    pEvent->pdata[0] = VSCP_DATACODING_NORMALIZED;

                }
                break;

                case 2:
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
                        } 
                        else {
                            memcpy(pEvent->pdata + 1, buf, 6);
                        }

                        // Set data coding
                        pEvent->pdata[0] = VSCP_DATACODING_SINGLE;
                    } 
                    else {
                        pEvent->sizeData = 0;
                        pEvent->pdata = NULL;
                    }
                }
                break;
                
            } // switch
                

            if (vscp_doLevel2Filter(pEvent, &m_pObj->m_vscpfilter)) {
                m_pObj->m_mutexReceiveQueue.Lock();
                m_pObj->m_receiveList.push_back(pEvent);
                m_pObj->m_semReceiveQueue.Post();
                m_pObj->m_mutexReceiveQueue.Unlock();
            }
            else {
                vscp_deleteVSCPevent(pEvent);
            }

		} // event

		::wxSleep(m_interval ? m_interval : 1);
        
	}

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



