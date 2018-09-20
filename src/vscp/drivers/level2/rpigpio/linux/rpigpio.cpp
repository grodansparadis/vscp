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

#include <pigpio.h>

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
    m_watchdog = 0;
    m_noice_filter_steady = 0;
    m_noice_filter_active = 0;
    m_glitch_filter = 0;

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
                                uint8_t edge, 
                                vscpEventEx& event )
{
    if ( edge > INT_EDGE_BOTH ) {
        return false;
    }

    m_bEnable_Monitor = bEnable;
    m_monitor_edge = edge;
    vscp_copyVSCPEventEx( &m_monitorEvent, &event );

    return true;
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
    m_bHardware = false;
    m_range = 1024;
    m_frequency = 0;
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
// setRange
//

bool CGpioPwm::setRange( int range )
{
    m_range = range;
    return true;
}

//////////////////////////////////////////////////////////////////////
// getRange
//

int CGpioPwm::getRange( void )
{
    return m_range;
}

//////////////////////////////////////////////////////////////////////
// setFrequency
//

bool CGpioPwm::setFrequency( int frequency )
{
    m_frequency = frequency;
    return true;
}

//////////////////////////////////////////////////////////////////////
// getFrequency
//

int CGpioPwm::getFrequency( void )
{
    return m_frequency;
}


//////////////////////////////////////////////////////////////////////
// setDutyCycle
//

bool CGpioPwm::setDutyCycle( int dutycycle )
{
    m_dutycycle = dutycycle;
    return true;
}

//////////////////////////////////////////////////////////////////////
// getDutyCycle
//

int CGpioPwm::getDutyCycle( void )
{
    return m_dutycycle;
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

//////////////////////////////////////////////////////////////////////
// setFrequency
//

bool CGpioClock::setFrequency( int frequency )
{
    m_frequency = frequency;
    return true;
}

//////////////////////////////////////////////////////////////////////
// getFrequency
//

int CGpioClock::getFrequency( void )
{
    return m_frequency;
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

//////////////////////////////////////////////////////////////////////
// setFilter
//

bool CLocalDM::setFilter( vscpEventFilter& filter )
{
    vscp_copyVSCPFilter( &m_vscpfilter, &filter );
    return true;
}

//////////////////////////////////////////////////////////////////////
// getFilter
//

vscpEventFilter& CLocalDM::getFilter( void )
{
    return m_vscpfilter;
}

//////////////////////////////////////////////////////////////////////
// setAction
//

bool CLocalDM::setAction( uint8_t action )
{
    m_action = action;
    return true;
}

//////////////////////////////////////////////////////////////////////
// getAction
//

uint8_t CLocalDM::getAction( void )
{
    return m_action;
}

//////////////////////////////////////////////////////////////////////
// setActionParameter
//

bool CLocalDM::setActionParameter( wxString& param )
{
    m_strActionParam = param;
    return true;
}

//////////////////////////////////////////////////////////////////////
// getActionParameter
//

wxString& CLocalDM::getActionParameter( void )
{
    return m_strActionParam;
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
    m_sample_rate = 5;   
    m_primary_dma_channel = 14;
    m_secondary_dma_channel = 6;
	::wxInitialize();
}

//////////////////////////////////////////////////////////////////////
// ~CRpiGpio
//

CRpiGpio::~CRpiGpio()
{
	close();

    // Remove input pin descriptions
    std::list<CGpioInput *>::const_iterator iterator1;
    for (iterator1 = m_inputPinList.begin(); iterator1 != m_inputPinList.end(); ++iterator1) {
        delete *iterator1;
    }

    // Remove output pin descriptions
    std::list<CGpioOutput *>::const_iterator iterator2;
    for (iterator2 = m_outputPinList.begin(); iterator2 != m_outputPinList.end(); ++iterator2) {
        delete *iterator2;
    }

    // Remove pwm pin descriptions
    std::list<CGpioPwm *>::const_iterator iterator3;
    for (iterator3 = m_pwmPinList.begin(); iterator3 != m_pwmPinList.end(); ++iterator3) {
        delete *iterator3;
    }

    // Remove gpio clock pin descriptions
    std::list<CGpioClock *>::const_iterator iterator4;
    for (iterator4 = m_gpioClockPinList.begin(); iterator4 != m_gpioClockPinList.end(); ++iterator4) {
        delete *iterator4;
    }

    // Remove local DM
    std::list<CLocalDM *>::const_iterator iterator5;
    for (iterator5 = m_LocalDMList.begin(); iterator5 != m_LocalDMList.end(); ++iterator5) {
        delete *iterator5;
    }

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
	wxString strConfig = wxString::FromAscii( pConfig );

	m_username = wxString::FromAscii( pUsername );
	m_password = wxString::FromAscii( pPassword );
	m_host = wxString::FromAscii( pHost );
	m_port = port;
	m_prefix = wxString::FromAscii( pPrefix );

	// Parse the configuration string. It should
	
	wxStringTokenizer tkz(wxString::FromAscii(pConfig), _(";\n"));

	// Check for socketcan interface in configuration string
	if ( tkz.HasMoreTokens() ) {
		m_setupXml = tkz.GetNextToken();    // XML for setup
	}

	// First log on to the host and get configuration 
	// variables

	if ( VSCP_ERROR_SUCCESS != m_srv.doCmdOpen( m_host,
                                                    m_port,
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

    m_srv.doClrInputQueue();

	// The server should hold XML configuration data for each
	// driver (se documentation).

    wxString str;
	wxString strName = m_prefix +
			wxString::FromAscii("_setup");
	if ( VSCP_ERROR_SUCCESS != m_srv.getRemoteVariableValue( strName, m_setupXml, true ) ) {
        // OK if not available we use default
    }

    wxStringInputStream xmlstream( m_setupXml );
    wxXmlDocument doc;

    if ( !doc.Load( xmlstream ) ) {

        syslog( LOG_ERR,
				    "%s %s",
                    (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID,
				    (const char *)"Unable to parse XML config. Terminating!");
        
        // Close the channel
	    m_srv.doCmdClose();
        
        return false;

    }

    // start processing the XML file
    if ( !(doc.GetRoot()->GetName() == _("setup") ) ) {

        syslog( LOG_ERR,
				    "%s %s",
                    (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID,
				    (const char *)"Malformed configuration XML (<setup> tag missing). Terminating!");
        
        // Close the channel
	    m_srv.doCmdClose();

        return false;
    }

    wxString attribute;

    // Samplerate
    attribute = doc.GetRoot()->GetAttribute("sample-rate", "5");
    m_sample_rate = vscp_readStringValue( attribute );

    // Primary DMA channel
    attribute = doc.GetRoot()->GetAttribute("primary_dma-channel", "14");
    m_primary_dma_channel = vscp_readStringValue( attribute );

    // Secondary DMA channel
    attribute = doc.GetRoot()->GetAttribute("secondary_dma-channel", "6");
    m_secondary_dma_channel = vscp_readStringValue( attribute );
    
    // Mask
    attribute = doc.GetRoot()->GetAttribute("mask", "");
    if ( attribute.Length() ) {
        if ( !vscp_readMaskFromString( &m_vscpfilter, attribute ) ) {
            syslog(LOG_ERR,
		            "%s %s",
                    (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID,
		            (const char *) "Unable to read event receive mask to driver filter.");
        }
    }

    // Filter
    attribute = doc.GetRoot()->GetAttribute("filter", "");
    if ( attribute.Length() ) {
        if ( !vscp_readFilterFromString( &m_vscpfilter, attribute ) ) {
            syslog(LOG_ERR,
		            "%s %s",
                    (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID,
		            (const char *) "Unable to read event receive filter to driver filter.");
        }
    }

    wxXmlNode *child = doc.GetRoot()->GetChildren();
    while ( child ) {

        if ( child->GetName() == _("input") ) {  

            CGpioInput *pInputObj = new CGpioInput;
            if ( NULL != pInputObj ) {

                // Get pin
                attribute =
                        child->GetAttribute("pin", "0");
                uint8_t pin = vscp_readStringValue( attribute );
                pInputObj->setPin( pin );

                // Get pullup
                attribute =
                        child->GetAttribute("pullup", "off");
                pInputObj->setPullUp( attribute ); 

                // Watchdog
                attribute =
                        child->GetAttribute("watchdog", "0");
                pInputObj->setWatchdog( vscp_readStringValue( attribute ) );

                // Noice filter steady
                attribute =
                        child->GetAttribute("noice_filter_steady", "0");
                pInputObj->setNoiceFilterSteady( vscp_readStringValue( attribute ) );

                // Noice filter active
                attribute =
                        child->GetAttribute("noice_filter_active", "0");
                pInputObj->setNoiceFilterActive( vscp_readStringValue( attribute ) );

                // Glitch filter
                attribute =
                        child->GetAttribute("glitch_filter", "0");
                pInputObj->setGlitchFilter( vscp_readStringValue( attribute ) );
                       
                // * * * Monitor 

                // monitor_edge
                wxString str_monitor_edge =
                        child->GetAttribute("monitor_edge", "disable");
                str_monitor_edge.MakeUpper();
                if ( wxNOT_FOUND == str_monitor_edge.Find("DISABLE") ) {
                            
                    uint8_t edge = INT_EDGE_SETUP;
                    vscpEventEx event;

                    attribute =
                            child->GetAttribute("monitor_event_class", "0");
                    event.vscp_class = vscp_readStringValue( attribute );

                    attribute =
                            child->GetAttribute("monitor_event_type", "0");
                    event.vscp_type = vscp_readStringValue( attribute );

                    event.sizeData = 0;
                    attribute =
                            child->GetAttribute("monitor_event_data", "0,0,0");
                    vscp_setVscpEventExDataFromString( &event, attribute );

                    attribute =
                            child->GetAttribute("monitor_event_index", "0");
                    event.data[0] = vscp_readStringValue( attribute );
                    if ( 0 == event.sizeData ) event.sizeData = 1;

                    attribute =
                            child->GetAttribute("monitor_event_zone", "0");
                    event.data[1] = vscp_readStringValue( attribute );
                    if ( event.sizeData < 2 ) event.sizeData = 2;

                    attribute =
                            child->GetAttribute("monitor_event_subzone", "0");
                    event.data[2] = vscp_readStringValue( attribute );
                    if ( event.sizeData < 3 ) event.sizeData = 3;

                    if ( !pInputObj->setMonitor( true, 
                                                    str_monitor_edge, 
                                                    event ) ) {
                        syslog( LOG_ERR,
				                "%s %s %d",
                                (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID,
				                (const char *) "Unable to add input monitor.",
                                (int)pin );
                    }

                }

                // * * * Report

                attribute =
                    child->GetAttribute("report_period", "");

                if ( attribute.Length() ) {

                    long period = vscp_readStringValue( attribute );

                    // event high
                    vscpEventEx eventHigh;

                    attribute =
                        child->GetAttribute("report_event_high_class", "0");
                    eventHigh.vscp_class = vscp_readStringValue( attribute );

                    attribute =
                        child->GetAttribute("report_event_high_type", "0");
                    eventHigh.vscp_type = vscp_readStringValue( attribute );

                    eventHigh.sizeData = 0;
                    attribute =
                        child->GetAttribute("report_event_high_data", "0,0,0");
                    vscp_setVscpEventExDataFromString( &eventHigh, attribute );

                    attribute =
                        child->GetAttribute("report_event_high_index", "0");
                    eventHigh.data[0] = vscp_readStringValue( attribute );
                    if ( 0 == eventHigh.sizeData ) eventHigh.sizeData = 1;

                    attribute =
                        child->GetAttribute("report_event_high_zone", "0");
                    eventHigh.data[1] = vscp_readStringValue( attribute );
                    if ( eventHigh.sizeData < 2 ) eventHigh.sizeData = 2;

                    attribute =
                        child->GetAttribute("report_event_high_subzone", "0");
                    eventHigh.data[2] = vscp_readStringValue( attribute );
                    if ( eventHigh.sizeData < 3 ) eventHigh.sizeData = 3;

                    // event low
                    vscpEventEx eventLow;

                    attribute =
                        child->GetAttribute("report_event_low_class", "0");
                    eventLow.vscp_class = vscp_readStringValue( attribute );

                    attribute =
                        child->GetAttribute("report_event_low_type", "0");
                    eventLow.vscp_type = vscp_readStringValue( attribute );

                    eventLow.sizeData = 0;
                    attribute =
                        child->GetAttribute("report_event_low_data", "0,0,0");
                    vscp_setVscpEventExDataFromString( &eventLow, attribute );

                    attribute =
                        child->GetAttribute("report_event_low_index", "0");
                    eventLow.data[0] = vscp_readStringValue( attribute );
                    if ( 0 == eventLow.sizeData ) eventLow.sizeData = 1;

                    attribute =
                        child->GetAttribute("report_event_low_zone", "0");
                    eventLow.data[1] = vscp_readStringValue( attribute );
                    if ( eventLow.sizeData < 2 ) eventLow.sizeData = 2;

                    attribute =
                        child->GetAttribute("report_event_low_subzone", "0");
                    eventLow.data[2] = vscp_readStringValue( attribute );
                    if ( eventLow.sizeData < 3 ) eventLow.sizeData = 3;

                    if ( !pInputObj->setReport( true,
                                                period,
                                                eventHigh,
                                                eventLow ) ) {
                        syslog( LOG_ERR,
				                "%s %s %d",
                                (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID,
				                (const char *) "Unable to add input report.",
                                (int)pin );
                    }

                } // report period                          

                // Add input to list
                m_inputPinList.push_back( pInputObj );
                        
            } // input obj

        } // input tag

        // Define output pin
        else if ( child->GetName() == _("output") ) {
                
            CGpioOutput *pOutputObj = new CGpioOutput;
            if ( NULL != pOutputObj ) {

                // Get pin
                attribute =
                    child->GetAttribute("pin", "0");
                uint8_t pin = vscp_readStringValue( attribute );
                pOutputObj->setPin( pin );

                // Get initial state
                attribute =
                    child->GetAttribute("pin", "off");
                attribute.MakeUpper();
                if ( wxNOT_FOUND != attribute.Find("ON") ) {
                    pOutputObj->setInitialState( 1 );
                }
                else {
                    pOutputObj->setInitialState( 0 );
                }

                // Add input to list
                m_outputPinList.push_back( pOutputObj );

            }

        }
        // Define PWM pin
        else if ( child->GetName() == "pwm" ) {
                
            CGpioPwm *pPwmObj = new CGpioPwm;
            if ( NULL != pPwmObj ) {

                // Get pin
                attribute =
                    child->GetAttribute("pin", "18");
                uint8_t pin = vscp_readStringValue( attribute );
                pPwmObj->setPin( pin );

                // Get hardware flag
                attribute =
                    child->GetAttribute("hardware", "false");
                attribute.MakeUpper();
                if ( wxNOT_FOUND != attribute.Find(_("TRUE")) ) {    
                    pPwmObj->enableHardwarePwm();
                }

                // Get rate
                attribute =
                    child->GetAttribute("range", "0");
                uint16_t range = vscp_readStringValue( attribute );    
                pPwmObj->setRange( range );

                // Get divisor
                attribute =
                    child->GetAttribute("range", "0");
                uint16_t frequency = vscp_readStringValue( attribute );    
                pPwmObj->setFrequency( frequency );

                // Add owm pwmoutput to list
                m_pwmPinList.push_back( pPwmObj );

            }

        }
                
        // Define gpioclock
        else if ( child->GetName() == "gpioclock" ) {
                
            CGpioClock *pGpioClockObj = new CGpioClock;
            if ( NULL != pGpioClockObj ) {

                // Get pin
                attribute =
                    child->GetAttribute("pin", "7");
                uint8_t pin = vscp_readStringValue( attribute );
                pGpioClockObj->setPin( pin );

                // Add gpio clock output to list
                m_gpioClockPinList.push_back( pGpioClockObj );

            }

        }
        // Define Decision Matrix
        else if ( child->GetName() == "dm" ) {
                
            CLocalDM *pLocalDMObj = new CLocalDM;
            if ( NULL != pLocalDMObj ) {

                // Get filter
                vscpEventFilter filter;

                attribute =
                    child->GetAttribute("priority-mask", "0");
                filter.filter_priority = vscp_readStringValue( attribute );   

                attribute =
                    child->GetAttribute("priority-filter", "0");
                filter.mask_priority = vscp_readStringValue( attribute );  

                attribute =
                    child->GetAttribute("class-mask", "0");
                filter.mask_class = vscp_readStringValue( attribute );

                attribute =
                    child->GetAttribute("class-filter", "0");
                filter.filter_class = vscp_readStringValue( attribute );

                attribute =
                    child->GetAttribute("type-mask", "0");
                filter.mask_type = vscp_readStringValue( attribute );

                attribute =
                    child->GetAttribute("type-filter", "0"); 
                filter.filter_type = vscp_readStringValue( attribute );   

                attribute =
                    child->GetAttribute("guid-mask", "0");
                vscp_getGuidFromStringToArray( filter.mask_GUID, attribute );  

                attribute =
                    child->GetAttribute("guid-filter", "0");
                vscp_getGuidFromStringToArray( filter.filter_GUID, attribute );     

                pLocalDMObj->setFilter( filter );

                // Get Index
                attribute =
                    child->GetAttribute("index", "0");
                uint8_t index = vscp_readStringValue( attribute ); 
                pLocalDMObj->setIndex( index );   

                // Get Zone
                attribute =
                    child->GetAttribute("zone", "0");  
                uint8_t zone = vscp_readStringValue( attribute );     
                pLocalDMObj->setZone( zone );                             

                // Get SubZone
                attribute =
                    child->GetAttribute("subzone", "0"); 
                uint8_t subzone = vscp_readStringValue( attribute );    
                pLocalDMObj->setSubZone( subzone );

                // Get Action
                attribute =
                    child->GetAttribute("action", "0");                               
                uint8_t action = vscp_readStringValue( attribute ); 
                pLocalDMObj->setAction( action );   

                // Get action parameter
                attribute =
                    child->GetAttribute("action-parameter", "");    
                pLocalDMObj->setActionParameter( attribute );    

                m_LocalDMList.push_back( pLocalDMObj ); 

            } // DM obj.
                    
        } // DM tag

        child = child->GetNext();

    } // while
	
    // Close the channel
	m_srv.doCmdClose();

    // If samplerate is not default change it before initializing
    if ( m_sample_rate != 5 ) {
        gpioCfgClock( m_sample_rate, 1, 0 );
    }

    if ( ( m_primary_dma_channel != 14 ) || 
         ( m_secondary_dma_channel != 6 ) ) {
        gpioCfgDMAchannels( m_primary_dma_channel, 
                            m_secondary_dma_channel ); 
    }

    // Init. the pigpio lib
	if ( gpioInitialise() < 0 ) {
        syslog( LOG_ERR,
				    "%s %s pigpio version: %u, HW rev: %u",
                    (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID,
				    (const char *) "Failed to init. gpio library.",
                    gpioVersion(), gpioHardwareRevision() );
        return false;                    
    }

    // Setup pin functionality

    // Init. input pins
    std::list<CGpioInput *>::const_iterator iterator1;
    for (iterator1 = m_inputPinList.begin(); iterator1 != m_inputPinList.end(); ++iterator1) {
        CGpioInput *pGpioInput = *iterator1;
        gpioSetMode( pGpioInput->getPin(), PI_INPUT );
        gpioSetPullUpDown( pGpioInput->getPin(), pGpioInput->getPullUp() );
        printf("%d\n", pGpioInput->getPin() );
        // Define watchdog value
        if ( pGpioInput->getWatchdog() ) {
            gpioSetWatchdog( pGpioInput->getPin(), pGpioInput->getWatchdog() );
        }
        // Define noice filter value
        if ( pGpioInput->getNoiceFilterSteady() ) {
            gpioNoiseFilter( pGpioInput->getPin(), 
                                pGpioInput->getNoiceFilterSteady(),
                                pGpioInput->getNoiceFilterActive() );
        }
        // Define glitch filter value
        if ( pGpioInput->getGlitchFilter() ) {
            gpioGlitchFilter( pGpioInput->getPin(), pGpioInput->getGlitchFilter() );
        }
    }

    // Init. output pins
    std::list<CGpioOutput *>::const_iterator iterator2;
    for (iterator2 = m_outputPinList.begin(); iterator2 != m_outputPinList.end(); ++iterator2) {
        CGpioOutput *pGpioOutput = *iterator2;
        gpioSetMode( pGpioOutput->getPin(), PI_OUTPUT );
        gpioWrite( pGpioOutput->getPin(), pGpioOutput->getInitialState() ) ;
    }

    // Init. pwm pins
    std::list<CGpioPwm *>::const_iterator iterator3;
    for (iterator3 = m_pwmPinList.begin(); iterator3 != m_pwmPinList.end(); ++iterator3) {
        CGpioPwm *pGpioPwm = *iterator3;
        // Hardware PWM settings
        if ( pGpioPwm->isHardwarePwm() ) {
            gpioHardwarePWM( pGpioPwm->getPin(), pGpioPwm->getFrequency(), pGpioPwm->getDutyCycle() );            
        }
        else {
            gpioSetPWMfrequency( pGpioPwm->getPin(), pGpioPwm->getFrequency() );
            gpioSetPWMrange( pGpioPwm->getPin(), pGpioPwm->getRange() ) ;
            gpioSetMode( pGpioPwm->getPin(), PI_OUTPUT );   
            gpioPWM( pGpioPwm->getPin(), pGpioPwm->getDutyCycle() ) ;     
        }
    }

    // Remove gpio clock pin descriptions
    std::list<CGpioClock *>::const_iterator iterator4;
    for (iterator4 = m_gpioClockPinList.begin(); iterator4 != m_gpioClockPinList.end(); ++iterator4) {
        CGpioClock *pGpioClock = *iterator4;
        gpioHardwareClock( pGpioClock->getPin(), pGpioClock->getFrequency() );
    }



	// start the workerthread
	m_pthreadWorker = new RpiGpioWorkerTread();
	if ( NULL != m_pthreadWorker ) {
		m_pthreadWorker->m_pObj = this;
		m_pthreadWorker->Create();
		m_pthreadWorker->Run();
	} 
	else {
		rv = false;
	}

	return rv;
}


//////////////////////////////////////////////////////////////////////
// close
//

void
CRpiGpio::close(void)
{
	// Do nothing if already terminated
	if  (m_bQuit ) return;

	m_bQuit = true; // terminate the thread
	wxSleep(1);     // Give the worker thread some time to terminate

    // wuit gpio library functionality
    gpioTerminate();
}

//////////////////////////////////////////////////////////////////////
// addEvent2SendQueue
//

bool 
CRpiGpio::addEvent2SendQueue( const vscpEvent *pEvent )
{
    m_mutexSendQueue.Lock();
    m_sendList.push_back( (vscpEvent *)pEvent );
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
	::wxInitialize();
			
	// Check pointers
	if ( NULL == m_pObj ) return NULL;
	
    while ( !TestDestroy() && !m_pObj->m_bQuit ) {
        ;
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



