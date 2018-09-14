// rpigpio.cpp
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP Project (http://www.vscp.org) 
//
// Copyright (C) 2000-2018 Ake Hedman, 
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
#include <wx/sstream.h>
#include <wx/xml/xml.h>
#include <wx/listimpl.cpp>
#include <wx/thread.h>
#include <wx/tokenzr.h>
#include <wx/datetime.h>

#include "../../../../common/vscp.h"
#include "../../../../common/vscphelper.h"
#include "../../../../common/vscpremotetcpif.h"
#include "../../../../common/vscp_type.h"
#include "../../../../common/vscp_class.h"
#include "rpigpio.h"

// ----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////
// CRpiCGpioInputGpio
//

CGpioInput::CGpioInput()
{
    m_pin = 0;
    m_pullup = PUD_OFF;
    // Monitor
    m_bEnable_Monitor = false;
    m_monitor_edge = INT_EDGE_SETUP;
    m_monitorEvent.obid = 0;
    m_monitorEvent.timestamp = 0;
    m_monitorEvent.year = 0;
    m_monitorEvent.month = 0;
    m_monitorEvent.day = 0;
    m_monitorEvent.hour = 0;
    m_monitorEvent.minute = 0;
    m_monitorEvent.second = 0;
    m_monitorEvent.vscp_class = 0;
    m_monitorEvent.vscp_type = 0;
    m_monitorEvent.head = 0;
    m_monitorEvent.sizeData = 0;
    memset( m_monitorEvent.GUID, 0, 16 );
    // Reports
    m_bEnable_Report = false;
    m_report_period = 1000; // one second
                    
    m_reportEventHigh.obid = 0;
    m_reportEventHigh.timestamp = 0;
    m_reportEventHigh.year = 0;
    m_reportEventHigh.month = 0;
    m_reportEventHigh.day = 0;
    m_reportEventHigh.hour = 0;
    m_reportEventHigh.minute = 0;
    m_reportEventHigh.second = 0;
    m_reportEventHigh.vscp_class = 0;
    m_reportEventHigh.vscp_type = 0;
    m_reportEventHigh.head = 0;
    m_reportEventHigh.sizeData = 0;
    memset( m_reportEventHigh.GUID, 0, 16 );

    m_reportEventLow.obid = 0;
    m_reportEventLow.timestamp = 0;
    m_reportEventLow.year = 0;
    m_reportEventLow.month = 0;
    m_reportEventLow.day = 0;
    m_reportEventLow.hour = 0;
    m_reportEventLow.minute = 0;
    m_reportEventLow.second = 0;
    m_reportEventLow.vscp_class = 0;
    m_reportEventLow.vscp_type = 0;
    m_reportEventLow.head = 0;
    m_reportEventLow.sizeData = 0;
    memset( m_reportEventLow.GUID, 0, 16 );
}

//////////////////////////////////////////////////////////////////////
// ~CGpioInput
//

CGpioInput::~CGpioInput()
{
    ;
}

//////////////////////////////////////////////////////////////////////
// setPin
//

bool CGpioInput::setPin( uint8_t pin ) 
{ 
    if ( pin < 18 ) {
        m_pin = pin; 
        return true;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////
// getPin
//

uint8_t CGpioInput::getPin( void ) 
{
     return m_pin; 
}

//////////////////////////////////////////////////////////////////////
// setPullUp
//

bool CGpioInput::setPullUp( const wxString& strPullUp ) 
{ 
    wxString str = strPullUp.Upper();
    str = str.Trim(false);
    str = str.Trim(true);
    if ( wxNOT_FOUND != str.Find("UP") ) {
        m_pullup = PUD_UP;
    } 
    else if  ( wxNOT_FOUND != str.Find("DOWN") ) {
        m_pullup = PUD_DOWN;
    }
    else if  ( wxNOT_FOUND != str.Find("OFF") ) {
        m_pullup = PUD_OFF;
    }
    else {
        return false;
    }

    return true;
};

//////////////////////////////////////////////////////////////////////
// getPullUp
//

uint8_t CGpioInput::getPullUp( void )
{
    return m_pullup;
}

//////////////////////////////////////////////////////////////////////
// setMonitor
//

bool CGpioInput::setMonitor( bool bEnable, 
                                wxString& strEdge, 
                                vscpEventEx& event )
{
    wxString str = strEdge.Upper();
    str = str.Trim(false);
    str = str.Trim(true);

    if ( wxNOT_FOUND != str.Find("FALLING") ) {
        m_monitor_edge = INT_EDGE_FALLING;
    } 
    else if  ( wxNOT_FOUND != str.Find("RISING") ) {
        m_monitor_edge = INT_EDGE_RISING;
    }
    else if  ( wxNOT_FOUND != str.Find("BOTH") ) {
        m_monitor_edge = INT_EDGE_BOTH;
    }
    else if  ( wxNOT_FOUND != str.Find("SETUP") ) {
        m_monitor_edge = INT_EDGE_SETUP;
    }
    else {
        return false;
    }

    m_bEnable_Monitor = bEnable;
    vscp_copyVSCPEventEx( &m_monitorEvent, &event );

    return true;
}

//////////////////////////////////////////////////////////////////////
// isMonitorEnabled
//

bool CGpioInput::isMonitorEnabled( void )
{
    return m_bEnable_Monitor;
}

//////////////////////////////////////////////////////////////////////
// getMonitorEdge
//

uint8_t CGpioInput::getMonitorEdge( void )
{
    return m_monitor_edge;
}

//////////////////////////////////////////////////////////////////////
// getMonitorEvent
//

vscpEventEx& CGpioInput::getMonitorEvent( void )
{
    return m_monitorEvent;
}

//////////////////////////////////////////////////////////////////////
// setReport
//

bool CGpioInput::setReport( bool bEnable, 
                            long period, 
                            vscpEventEx& eventLow,
                            vscpEventEx& eventHigh )
{
    m_bEnable_Report = bEnable;
    m_report_period = period;
    vscp_copyVSCPEventEx( &m_reportEventLow, &eventLow );
    vscp_copyVSCPEventEx( &m_reportEventHigh, &eventHigh );

    return true;                  
}

//////////////////////////////////////////////////////////////////////
// isReportEnabled
//

bool CGpioInput::isReportEnabled( void )
{
    return m_bEnable_Report;
}

//////////////////////////////////////////////////////////////////////
// getReportPeriod
//

long CGpioInput::getReportPeriod( void )
{
    return m_report_period;
}

//////////////////////////////////////////////////////////////////////
// getReportEventLow
//

vscpEventEx& CGpioInput::getReportEventLow( void )
{
    return m_reportEventLow;
}

//////////////////////////////////////////////////////////////////////
// getReportEventHigh
//

vscpEventEx& CGpioInput::getReportEventHigh( void )
{
    return m_reportEventHigh;
}


// ----------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////
// CGpioOutput
//

CGpioOutput::CGpioOutput() 
{
    m_pin = 0;
    m_state = -1;
}

//////////////////////////////////////////////////////////////////////
// ~CGpioOutput
//

CGpioOutput::~CGpioOutput() 
{
    ;
}

//////////////////////////////////////////////////////////////////////
// setPin
//

bool CGpioOutput::setPin( uint8_t pin ) 
{
    return true;
}

//////////////////////////////////////////////////////////////////////
// getPin
//

uint8_t CGpioOutput::getPin( void )
{
    return m_pin;
}

//////////////////////////////////////////////////////////////////////
// setInitialState
//

void CGpioOutput::setInitialState( int state )
{
    m_state = state;
}

//////////////////////////////////////////////////////////////////////
// getInitialState
//

int CGpioOutput::getInitialState( void )
{
    return m_state;
}

// ----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////
// CGpioPwm
//

CGpioPwm::CGpioPwm() 
{
    m_pin = 18;
    m_type = PWM_OUTPUT;
    m_mode = PWM_MODE_MS;
    m_range = 1024;
    m_divisor = 0;
}

//////////////////////////////////////////////////////////////////////
// ~CGpioPwm
//

CGpioPwm::~CGpioPwm() 
{
    ;
}

//////////////////////////////////////////////////////////////////////
// setPin
//

bool CGpioPwm::setPin( uint8_t pin )
{
    m_pin = pin;
    return true;
}

//////////////////////////////////////////////////////////////////////
// setPin
//

uint8_t CGpioPwm::getPin( void )
{
    return m_pin;
}

//////////////////////////////////////////////////////////////////////
// setType
//

bool CGpioPwm::setType( uint8_t type )
{
    m_type = type;
    return true;
}

//////////////////////////////////////////////////////////////////////
// getType
//

uint8_t CGpioPwm::getType( void )
{
    return m_type;
}

//////////////////////////////////////////////////////////////////////
// setMode
//

bool CGpioPwm::setMode( uint8_t mode )
{
    m_mode = mode;
    return true;
}

//////////////////////////////////////////////////////////////////////
// getMode
//

uint8_t CGpioPwm::getMode( void )
{
    return m_mode;
}

//////////////////////////////////////////////////////////////////////
// setRange
//

bool CGpioPwm::setRange( uint16_t range )
{
    m_range = range;
    return true;
}

//////////////////////////////////////////////////////////////////////
// getRange
//

uint16_t CGpioPwm::getRange( void )
{
    return m_range;
}

//////////////////////////////////////////////////////////////////////
// setDivisor
//

bool CGpioPwm::setDivisor( uint16_t divisor )
{
    m_divisor = divisor;
    return true;
}

//////////////////////////////////////////////////////////////////////
// getDivisor
//

uint16_t CGpioPwm::getDivisor( void )
{
    return m_divisor;
}


// ----------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////
// CGpioClock
//

CGpioClock::CGpioClock() 
{
    m_pin = 7;
}

//////////////////////////////////////////////////////////////////////
// ~CGpioClock
//

CGpioClock::~CGpioClock() 
{
    ;
}

//////////////////////////////////////////////////////////////////////
// setPin
//

bool CGpioClock::setPin( uint8_t pin )
{
    m_pin = pin;
}

//////////////////////////////////////////////////////////////////////
// getPin
//

uint8_t CGpioClock::getPin( void )
{
    return m_pin;
}


// ----------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////
// CLocalDM
//

CLocalDM::CLocalDM() 
{
    vscp_clearVSCPFilter(&m_vscpfilter);    // Accept all events
    m_bCompareIndex = false;                // Don't compare index
    m_index = 0;
    m_bCompareZone = false;                 // Don't compare zone
    m_zone = 0;
    m_bCompareSubZone = false;              // Don't compare subzone
    m_subzone = 0;
    m_action = RPIGPIO_ACTION_NOOP;
    m_strActionParam.Empty();               // Looks good (if you feel sick by this)
}

//////////////////////////////////////////////////////////////////////
// ~CLocalDM
//

CLocalDM::~CLocalDM()
{
    ;
}

//////////////////////////////////////////////////////////////////////
// setIndex
//

bool CLocalDM::setIndex( uint8_t index )
{
    m_bCompareIndex = true;
    m_index = index;
    return true;
}

//////////////////////////////////////////////////////////////////////
// getIndex
//

uint8_t CLocalDM::getIndex( void )
{
    return m_index;
}

//////////////////////////////////////////////////////////////////////
// isIndexCheckEnabled
//

bool CLocalDM::isIndexCheckEnabled( void )
{
    return m_bCompareIndex;
}

//////////////////////////////////////////////////////////////////////
// setZone
//

bool CLocalDM::setZone( uint8_t zone )
{   
    m_bCompareZone = true;
    m_zone = zone;
    return true;
}

//////////////////////////////////////////////////////////////////////
// getZone
//

uint8_t CLocalDM::getZone( void )
{
    return m_zone;
}

//////////////////////////////////////////////////////////////////////
// isZoneCheckEnabled
//

bool CLocalDM::isZoneCheckEnabled( void )
{
    return m_bCompareZone;
}

//////////////////////////////////////////////////////////////////////
// setSubZone
//

bool CLocalDM::setSubZone( uint8_t subzone )
{
    m_bCompareSubZone = true;
    m_subzone = subzone;
    return true;
}

//////////////////////////////////////////////////////////////////////
// getSubZone
//

uint8_t CLocalDM::getSubZone( void )
{
    return m_subzone;
}

//////////////////////////////////////////////////////////////////////
// isSubZoneCheckEnabled
//

bool CLocalDM::isSubZoneCheckEnabled( void )
{
    return m_bCompareSubZone;
}

// ----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////
// CRpiGpio
//

CRpiGpio::CRpiGpio()
{
	m_bQuit = false;
	m_pthreadWorker = NULL;
	m_setupXml = _("<?xml version = \"1.0\" encoding = \"UTF-8\" ?><setup><!-- empty --></setup>");
	vscp_clearVSCPFilter(&m_vscpfilter); // Accept all events
	::wxInitialize();
}

//////////////////////////////////////////////////////////////////////
// ~CRpiGpio
//

CRpiGpio::~CRpiGpio()
{
	close();
	::wxUninitialize();
}


//////////////////////////////////////////////////////////////////////
// open
//
//

bool
CRpiGpio::open( const char *pUsername,
		        const char *pPassword,
		        const char *pHost,
		        short port,
		        const char *pPrefix,
		        const char *pConfig)
{
	bool rv = true;
	wxString wxstr = wxString::FromAscii( pConfig );

	m_username = wxString::FromAscii( pUsername );
	m_password = wxString::FromAscii( pPassword );
	m_host = wxString::FromAscii( pHost );
	m_port = port;
	m_prefix = wxString::FromAscii( pPrefix );

	// Parse the configuration string. It should
	// have the following form
	// path
	
	wxStringTokenizer tkz(wxString::FromAscii(pConfig), _(";\n"));

	// Check for socketcan interface in configuration string
	if ( tkz.HasMoreTokens() ) {
		// Interface
		m_setupXml = tkz.GetNextToken();
	}


	// First log on to the host and get configuration 
	// variables

	if ( VSCP_ERROR_SUCCESS != m_srv.doCmdOpen( m_host,
												m_username,
												m_password ) ) {
		syslog(LOG_ERR,
				"%s %s",
                (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID,
				(const char *)"Unable to connect to VSCP TCP/IP interface. Terminating!");
		
        return false;
	}

	// Find the channel id
	uint32_t ChannelID;
	m_srv.doCmdGetChannelID( &ChannelID );
    
    // Get GUID for channel
	m_srv.doCmdGetGUID( m_ifguid );

	// The server should hold XML configuration data for each
	// driver (se documentation).

    wxString str;
	wxString strName = m_prefix +
			wxString::FromAscii("_setup");
	if ( VSCP_ERROR_SUCCESS != m_srv.getRemoteVariableAsString( strName, m_setupXml ) ) {
        // OK if not available we use default
    }

    wxStringInputStream xmlstream( m_setupXml );
    wxXmlDocument doc;

    if ( !doc.Load( xmlstream ) ) {
        syslog(LOG_ERR,
				"%s %s",
                (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID,
				(const char *) "Unable to parse XML config. Terminating!");
        
        // Close the channel
	    m_srv.doCmdClose();
        
        return false;
    }

    wxString attribute;
    wxXmlNode *child = doc.GetRoot()->GetChildren();
    while ( child ) {
    
        if ( child->GetName() == "setup" ) {
            
            wxXmlNode *subchild = child->GetChildren();
            while ( subchild ) {

                if ( subchild->GetName() == "mask" ) {
                    
                    wxString mask = subchild->GetNodeContent();
                    if ( !vscp_readMaskFromString( &m_vscpfilter, mask ) ) {
                        syslog(LOG_ERR,
				                "%s %s",
                                (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID,
				                (const char *) "Unable to read event receive mask to driver filter.");
                    }

                }
                else if ( subchild->GetName() == "filter" ) {

                    wxString filter = subchild->GetNodeContent();
                    if ( vscp_readFilterFromString( &m_vscpfilter, filter ) ) {
                        syslog(LOG_ERR,
				                "%s %s",
                                (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID,
				                (const char *) "Unable to read event receive filter to driver filter.");
                    }

                }
                // Define one input pin
                else if ( subchild->GetName() == "input" ) {

                    CGpioInput *pInputObj = new CGpioInput;
                    if ( NULL != pInputObj ) {

                        // Get pin
                        attribute =
                            subchild->GetAttribute("pin", "0");
                        uint8_t pin = vscp_readStringValue( attribute );
                        pInputObj->setPin( pin );

                        // Get pullup
                        attribute =
                            subchild->GetAttribute("pullup", "off");
                        pInputObj->setPullUp( attribute ); 
                       

                        // Monitor 

                        // monitor_edge
                        wxString str_monitor_edge =
                            subchild->GetAttribute("monitor_edge", "disable");
                        str_monitor_edge.MakeUpper();
                        if ( wxNOT_FOUND == str_monitor_edge.Find("DISABLE") ) {
                            
                            uint8_t edge = INT_EDGE_SETUP;
                            vscpEventEx event;

                            attribute =
                            subchild->GetAttribute("monitor_event_class", "0");
                            event.vscp_class = vscp_readStringValue( attribute );

                            attribute =
                            subchild->GetAttribute("monitor_event_type", "0");
                            event.vscp_type = vscp_readStringValue( attribute );

                            event.sizeData = 0;
                            attribute =
                            subchild->GetAttribute("monitor_event_data", "0,0,0");
                            vscp_setVscpEventExDataFromString( &event, attribute );

                            attribute =
                            subchild->GetAttribute("monitor_event_index", "0");
                            event.data[0] = vscp_readStringValue( attribute );
                            if ( 0 == event.sizeData ) event.sizeData = 1;

                            attribute =
                            subchild->GetAttribute("monitor_event_zone", "0");
                            event.data[1] = vscp_readStringValue( attribute );
                            if ( event.sizeData < 2 ) event.sizeData = 2;

                            attribute =
                            subchild->GetAttribute("monitor_event_subzone", "0");
                            event.data[2] = vscp_readStringValue( attribute );
                            if ( event.sizeData < 3 ) event.sizeData = 3;

                            if ( !pInputObj->setMonitor( true, 
                                                            str_monitor_edge, 
                                                            event ) ) {
                                syslog(LOG_ERR,
				                        "%s %s %d",
                                        (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID,
				                        (const char *) "Unable toadd input monitor.",
                                        (int)pin );
                            }                            
                        }

                        // Report


                        // Add input to list
                        m_inputPinList.push_back( pInputObj );

                        
                    }

                }
                // Define output pin
                else if ( subchild->GetName() == "output" ) {
                
                }
                // Define PWM pin
                else if ( subchild->GetName() == "pwm" ) {
                
                }
                // Define gpioclock
                else if ( subchild->GetName() == "gpioclock" ) {
                
                }
                // Define Decision Matrix
                else if ( subchild->GetName() == "dm" ) {
                
                }

                subchild = child->GetNext();
            }

            // Process attributes of tag1.
            wxString attrvalue1 =
                child->GetAttribute("attr1", "default-value");
            wxString attrvalue2 =
                child->GetAttribute("attr2", "default-value");
            
        }
        else if (child->GetName() == "tag2") {
            // Process tag2 ...
        }

        child = child->GetNext();
    }
	
	m_srv.doClrInputQueue();

	// start the workerthread
	m_pthreadWorker = new RpiGpioWorkerTread();
	if (NULL != m_pthreadWorker) {
		m_pthreadWorker->m_pObj = this;
		m_pthreadWorker->Create();
		m_pthreadWorker->Run();
	} 
	else {
		rv = false;
	}

	// Close the channel
	m_srv.doCmdClose();

	return rv;
}


//////////////////////////////////////////////////////////////////////
// close
//

void
CRpiGpio::close(void)
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
CRpiGpio::addEvent2SendQueue(const vscpEvent *pEvent)
{
    m_mutexSendQueue.Lock();
	//m_sendQueue.Append((vscpEvent *)pEvent);
    m_sendList.push_back((vscpEvent *)pEvent);
	m_semSendQueue.Post();
	m_mutexSendQueue.Unlock();
    return true;
}



//////////////////////////////////////////////////////////////////////
//                Workerthread - RpiGpioWorkerTread
//////////////////////////////////////////////////////////////////////

RpiGpioWorkerTread::RpiGpioWorkerTread()
{
	m_pObj = NULL;
}

RpiGpioWorkerTread::~RpiGpioWorkerTread()
{
	;
}


//////////////////////////////////////////////////////////////////////
// Entry
//

void *
RpiGpioWorkerTread::Entry()
{
	int sock;
	char devname[IFNAMSIZ + 1];
	fd_set rdfs;
	struct timeval tv;
	struct sockaddr_can addr;
	struct ifreq ifr;
	struct cmsghdr *cmsg;
	struct canfd_frame frame;
	char ctrlmsg[CMSG_SPACE(sizeof(struct timeval)) + CMSG_SPACE(sizeof(__u32))];
	const int canfd_on = 1;

	::wxInitialize();
			
	// Check pointers
	if (NULL == m_pObj) return NULL;

	//strncpy(devname, m_pObj->m_interface.ToAscii(), sizeof(devname) - 1);
#if DEBUG	
	syslog(LOG_ERR, "CWriteSocketCanTread: Interface: %s\n", ifname);
#endif	
	
    while (!TestDestroy() && !m_pObj->m_bQuit) {

        sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
        if (sock < 0) {
            
            if (ENETDOWN == errno) {
                sleep(1);
                continue;
            }
            
            syslog(LOG_ERR,
                    "%s",
                    (const char *) "CReadSocketCanTread: Error while opening socket. Terminating!");
            
            m_pObj->m_bQuit;
            continue;
        }

        strcpy(ifr.ifr_name, devname);
        ioctl(sock, SIOCGIFINDEX, &ifr);

        addr.can_family = AF_CAN;
        addr.can_ifindex = ifr.ifr_ifindex;

#ifdef DEBUG
        printf("using interface name '%s'.\n", ifr.ifr_name);
#endif

        // try to switch the socket into CAN FD mode 
        setsockopt(sock,
                SOL_CAN_RAW,
                CAN_RAW_FD_FRAMES,
                &canfd_on,
                sizeof(canfd_on));

        if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
            syslog(LOG_ERR,
                    "%s",
                    (const char *) "CReadSocketCanTread: Error in socket bind. Terminating!");
            close(sock);
            sleep(2);
            continue;
        }

        bool bInnerLoop = true;
        while (!TestDestroy() && !m_pObj->m_bQuit && bInnerLoop) {

            FD_ZERO(&rdfs);
            FD_SET(sock, &rdfs);

            tv.tv_sec = 0;
            tv.tv_usec = 5000; // 5ms timeout 

            int ret;
            if ( ( ret = select(sock + 1, &rdfs, NULL, NULL, &tv ) ) <  0) {
                // Error
                if (ENETDOWN == errno) {
                    // We try to get contact with the net
                    // again if it goes down
                    bInnerLoop = false;
                }
                else {
                    m_pObj->m_bQuit = true;
                }
                continue;
            }

            if ( ret ) {

                // There is data to read

                ret = read(sock, &frame, sizeof(struct can_frame));
                if (ret < 0) {
                    if (ENETDOWN == errno) {
                        // We try to get contact with the net
                        // again if it goes down
                        bInnerLoop = false;
                        sleep(2);
                    }
                    else {
                        m_pObj->m_bQuit = true;
                    }
                    continue;
                }

                // Must be Extended
                if (!(frame.can_id & CAN_EFF_FLAG)) continue;

                // Mask of control bits
                frame.can_id &= CAN_EFF_MASK;

                vscpEvent *pEvent = new vscpEvent();
                if (NULL != pEvent) {

                    pEvent->pdata = new uint8_t[frame.len];
                    if (NULL == pEvent->pdata) {
                        delete pEvent;
                        continue;
                    }

                    // GUID will be set to GUID of interface
                    // by driver interface with LSB set to nickname
                    memset(pEvent->GUID, 0, 16);
                    pEvent->GUID[VSCP_GUID_LSB] = frame.can_id & 0xff;

                    // Set VSCP class
                    pEvent->vscp_class = vscp_getVSCPclassFromCANALid(frame.can_id);

                    // Set VSCP type
                    pEvent->vscp_type = vscp_getVSCPtypeFromCANALid(frame.can_id);

                    // Copy data if any
                    pEvent->sizeData = frame.len;
                    if (frame.len) {
                        memcpy(pEvent->pdata, frame.data, frame.len);
                    }

                    if (vscp_doLevel2Filter(pEvent, &m_pObj->m_vscpfilter)) {
                        m_pObj->m_mutexReceiveQueue.Lock();
                        //m_pObj->m_receiveQueue.Append(pEvent);
                        m_pObj->m_receiveList.push_back(pEvent);
                        m_pObj->m_semReceiveQueue.Post();
                        m_pObj->m_mutexReceiveQueue.Unlock();
                    } else {
                        vscp_deleteVSCPevent(pEvent);
                    }
                }

            } else {

                // Check if there is event(s) to send
                if (m_pObj->m_sendList.size() /*m_pObj->m_sendQueue.GetCount()*/) {

                    // Yes there are data to send
                    // So send it out on the CAN bus

                    //VSCPEVENTLIST_SEND::compatibility_iterator nodeClient;
                    m_pObj->m_mutexSendQueue.Lock();
                    //nodeClient = m_pObj->m_sendQueue.GetFirst();
                    vscpEvent *pEvent = m_pObj->m_sendList.front();
                    m_pObj->m_sendList.pop_front();
                    m_pObj->m_mutexSendQueue.Unlock();

                    if (NULL == pEvent) continue;

                    // Class must be a Level I class or a Level II
                    // mirror class
                    if (pEvent->vscp_class < 512) {
                        frame.can_id = vscp_getCANALidFromVSCPevent(pEvent);
                        frame.can_id |= CAN_EFF_FLAG; // Always extended
                        if (0 != pEvent->sizeData) {
                            frame.len = (pEvent->sizeData > 8 ? 8 : pEvent->sizeData);
                            memcpy(frame.data, pEvent->pdata, frame.len);
                        }
                    } else if (pEvent->vscp_class < 1024) {
                        pEvent->vscp_class -= 512;
                        frame.can_id = vscp_getCANALidFromVSCPevent(pEvent);
                        frame.can_id |= CAN_EFF_FLAG; // Always extended
                        if (0 != pEvent->sizeData) {
                            frame.len = ((pEvent->sizeData - 16) > 8 ? 8 : pEvent->sizeData - 16);
                            memcpy(frame.data, pEvent->pdata + 16, frame.len);
                        }
                    }

                    // Remove the event
                    m_pObj->m_mutexSendQueue.Lock();
                    //m_pObj->m_sendQueue.DeleteNode(nodeClient);
                    vscp_deleteVSCPevent(pEvent);
                    m_pObj->m_mutexSendQueue.Unlock();


                    // Write the data
                    int nbytes = write(sock, &frame, sizeof(struct can_frame));

                } // event to send
            }  // No data to read
        } // Inner loop

        // Close the socket
        close(sock);

    } // Outer loop

	return NULL;

}

//////////////////////////////////////////////////////////////////////
// OnExit
//

void
RpiGpioWorkerTread::OnExit()
{
	;
}



