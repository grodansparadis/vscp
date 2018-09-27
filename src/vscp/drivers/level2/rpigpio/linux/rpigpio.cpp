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

#include <algorithm>
#include <string>
#include <functional>
#include <cctype>
#include <deque>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <syslog.h>


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

#include <expat.h>
#include <pigpio.h>

#include "../../../../common/vscp.h"
#include "../../../../common/vscphelper.h"
#include "../../../../common/vscpremotetcpif.h"
#include "../../../../common/vscp_type.h"
#include "../../../../common/vscp_class.h"
#include "rpigpio.h"

#define XML_BUFF_SIZE   32000

// ----------------------------------------------------------------------------

static void report_callback0( void * userdata );



//////////////////////////////////////////////////////////////////////
// CRpiCGpioInputGpio
//

CGpioInput::CGpioInput()
{
    m_pin = 0;
    m_pullup = PI_PUD_OFF;
    m_watchdog = 0;
    m_noice_filter_steady = 0;
    m_noice_filter_active = 0;
    m_glitch_filter = 0;

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

bool CGpioInput::setPullUp( const std::string& strPullUp ) 
{ 
    std::string str = strPullUp;
    vscp2_makeUpper( str );
    vscp2_trim( str );
 
    if ( std::string::npos != str.find("UP") ) {
        m_pullup = PI_PUD_OFF;
    } 
    else if  ( std::string::npos != str.find("DOWN") ) {
        m_pullup = PI_PUD_DOWN;
    }
    else if  ( std::string::npos != str.find("OFF") ) {
        m_pullup = PI_PUD_UP;
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
    m_strActionParam.clear();               // Looks good (if you feel sick by this)
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

bool CLocalDM::setActionParameter( const std::string& param )
{
    m_strActionParam = param;
    return true;
}

//////////////////////////////////////////////////////////////////////
// getActionParameter
//

std::string& CLocalDM::getActionParameter( void )
{
    return m_strActionParam;
}


// ----------------------------------------------------------------------------


//////////////////////////////////////////////////////////////////////
// CGpioMonitor
//

CGpioMonitor::CGpioMonitor()
{
    m_pin = 0;

    // Monitor
    m_edge = EITHER_EDGE;

    m_eventFalling.obid = 0;
    m_eventFalling.timestamp = 0;
    m_eventFalling.year = 0;
    m_eventFalling.month = 0;
    m_eventFalling.day = 0;
    m_eventFalling.hour = 0;
    m_eventFalling.minute = 0;
    m_eventFalling.second = 0;
    m_eventFalling.vscp_class = VSCP_CLASS1_INFORMATION;;
    m_eventFalling.vscp_type = VSCP_TYPE_INFORMATION_ON;
    m_eventFalling.head = 0;
    m_eventFalling.sizeData = 3;
    memset( m_eventFalling.data, 0, sizeof( m_eventRising.data ) );
    memset( m_eventFalling.GUID, 0, 16 );
    
    m_eventRising.obid = 0;
    m_eventRising.timestamp = 0;
    m_eventRising.year = 0;
    m_eventRising.month = 0;
    m_eventRising.day = 0;
    m_eventRising.hour = 0;
    m_eventRising.minute = 0;
    m_eventRising.second = 0;
    m_eventRising.vscp_class = VSCP_CLASS1_INFORMATION;
    m_eventRising.vscp_type = VSCP_TYPE_INFORMATION_ON;
    m_eventRising.head = 0;
    m_eventRising.sizeData = 3;
    memset( m_eventRising.data, 0, sizeof( m_eventRising.data ) );
    memset( m_eventRising.GUID, 0, 16 );
}

//////////////////////////////////////////////////////////////////////
// ~CGpioInput
//

CGpioMonitor::~CGpioMonitor()
{
    
}

//////////////////////////////////////////////////////////////////////
// setEdge
//

bool CGpioMonitor::setEdge( const std::string& strEdge )
{
    std::string str = strEdge;
    vscp2_makeUpper( str );
    vscp2_trim( str );
  
    if ( std::string::npos != str.find("FALLING") ) {
        m_edge = FALLING_EDGE;
    } 
    else if ( std::string::npos != str.find("RISING") ) {
        m_edge = RISING_EDGE;
    }
    else if ( std::string::npos != str.find("BOTH") ) {
        m_edge = EITHER_EDGE;
    }
    else if ( std::string::npos != str.find("SETUP") ) {
        m_edge = EITHER_EDGE;
    }
    else {
        return false;
    }
}

//////////////////////////////////////////////////////////////////////
// setMonitorEvents
//

void CGpioMonitor::setMonitorEvents( const vscpEventEx& eventFalling,
                                     const vscpEventEx& eventRising )
{
    vscp_copyVSCPEventEx( &m_eventFalling, &eventFalling );
    vscp_copyVSCPEventEx( &m_eventRising, &eventRising );
}


// ----------------------------------------------------------------------------

int depth_setup_parser;

void
startSetupParser( void *data, const char *name, const char **attr ) 
{
    CRpiGpio *pgpio = (CRpiGpio *)data;
    if ( NULL == pgpio ) return;

    if ( ( 0 == strcmp( name, "setup") ) && ( 0 == depth_setup_parser ) ) {
        for ( int i = 0; attr[i]; i += 2 ) {

            std::string attribute = attr[i+1];
            if ( 0 == strcmp( attr[i], "sample-rate") ) {                
                pgpio->setSampleRate( std::stoi( attribute ) );
            }
            else if ( 0 == strcmp( attr[i], "primary-dma-channel") ) {
                pgpio->setPrimaryDmaChannel( std::stoi( attribute ) );
            }
            else if ( 0 == strcmp( attr[i], "secondary-dma-channel") ) {
                pgpio->setSecondaryDmaChannel( std::stoi( attribute ) );
            }
            else if ( 0 == strcmp( attr[i], "mask") ) {
                if ( !attribute.empty() ) {
                    if ( !vscp2_readMaskFromString( &pgpio->m_vscpfilter, attribute ) ) {
                        syslog( LOG_ERR,
		                            "%s %s ",
                                    (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID,
		                            (const char *) "Unable to read event receive mask to driver filter.");
                    }
                }
            }
            else if ( 0 == strcmp( attr[i], "filter") ) {
                if ( !attribute.empty() ) {
                    if ( !vscp2_readFilterFromString( &pgpio->m_vscpfilter, attribute ) ) {
                        syslog( LOG_ERR,
		                            "%s %s ",
                                    (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID,
		                            (const char *) "Unable to read event receive mask to driver filter.");
                    }
                }
            }
        }
    } 
    else if ( ( 0 == strcmp( name, "input" ) ) && ( 1 == depth_setup_parser ) ) {
        
        CGpioInput *pInputObj = new CGpioInput;
        if ( NULL != pInputObj ) {

            for ( int i = 0; attr[i]; i += 2 ) {
            
                std::string attribute = attr[i+1];

                // Get pin
                if ( 0 == strcmp( attr[i], "pin") ) {
                    uint8_t pin = vscp2_readStringValue( attribute );
                    pInputObj->setPin( pin );
                }    
                
                // Get pullup
                else if ( 0 == strcmp( attr[i], "pullup") ) {
                    pInputObj->setPullUp( attribute );
                }
                
                // Watchdog
                else if ( 0 == strcmp( attr[i], "watchdog") ) {
                    pInputObj->setWatchdog( vscp2_readStringValue( attribute ) );
                }

                // Noice filter steady
                else if ( 0 == strcmp( attr[i], "noice_filter_steady") ) {
                    pInputObj->setWatchdog( vscp2_readStringValue( attribute ) );
                }

                // Noice filter active
                else if ( 0 == strcmp( attr[i], "noice_filter_active") ) {
                    pInputObj->setNoiceFilterActive( vscp2_readStringValue( attribute ) );
                }

                // Glitch filter
                else if ( 0 == strcmp( attr[i], "glitch_filter") ) {
                    pInputObj->setGlitchFilter( vscp2_readStringValue( attribute ) );
                }

            } // for - attributes

            // Add input to list
            pgpio->m_inputPinList.push_back( pInputObj );

        } // input obj
    }
    else if ( ( 0 == strcmp( name, "monitor" ) ) && ( 1 == depth_setup_parser ) ) {
        
        CGpioMonitor *pMonitorObj = new CGpioMonitor;
        if ( NULL != pMonitorObj ) {

            vscpEventEx eventRising;
            vscp2_setVscpEventExDataFromString( &eventRising, "0,0,0" );

            vscpEventEx eventFalling;
            vscp2_setVscpEventExDataFromString( &eventFalling, "0,0,0" );
        
            for ( int i = 0; attr[i]; i += 2 ) {            

                std::string attribute = attr[i+1];

                // Get monitor pin 
                if ( 0 == strcmp( attr[i], "pin") ) {
                    pMonitorObj->setPin( vscp2_readStringValue( attribute ) );
                }

                // monitor_edge
                else if ( 0 == strcmp( attr[i], "edge") ) {
                    pMonitorObj->setEdge( attribute ); 
                }

                // class for Rising event
                else if ( 0 == strcmp( attr[i], "rising_class") ) {
                    eventRising.vscp_class = vscp2_readStringValue( attribute ); 
                }

                // type for Rising event
                else if ( 0 == strcmp( attr[i], "rising_type") ) {
                    eventRising.vscp_type = vscp2_readStringValue( attribute ); 
                }

                // data for Rising event
                else if ( 0 == strcmp( attr[i], "rising_type") ) {
                    vscp2_setVscpEventExDataFromString( &eventRising, attribute ); 
                }

                // index for Rising event
                else if ( 0 == strcmp( attr[i], "rising_index") ) {
                    eventRising.data[0] = vscp2_readStringValue( attribute );
                    if ( 0 == eventRising.sizeData ) eventRising.sizeData = 1; 
                }

                // zone for Rising event
                else if ( 0 == strcmp( attr[i], "rising_zone") ) {
                    eventRising.data[1] = vscp2_readStringValue( attribute );
                    if ( eventRising.sizeData < 2 ) eventRising.sizeData = 2; 
                }

                // subzone for Rising event
                else if ( 0 == strcmp( attr[i], "rising_subzone") ) {
                    eventRising.data[2] = vscp2_readStringValue( attribute );
                    if ( eventRising.sizeData < 3 ) eventRising.sizeData = 3; 
                }

                // class for Falling event
                else if ( 0 == strcmp( attr[i], "falling_class") ) {
                    eventFalling.vscp_class = vscp2_readStringValue( attribute ); 
                }

                // type for Falling event
                else if ( 0 == strcmp( attr[i], "falling_type") ) {
                    eventFalling.vscp_type = vscp2_readStringValue( attribute ); 
                }

                // data for Rising event
                else if ( 0 == strcmp( attr[i], "rising_type") ) {
                    vscp2_setVscpEventExDataFromString( &eventFalling, attribute ); 
                }

                // index for Falling event
                else if ( 0 == strcmp( attr[i], "falling_index") ) {
                    eventFalling.data[0] = vscp2_readStringValue( attribute );
                    if ( 0 == eventFalling.sizeData ) eventFalling.sizeData = 1; 
                }

                // zone for Falling event
                else if ( 0 == strcmp( attr[i], "falling_zone") ) {
                    eventFalling.data[1] = vscp2_readStringValue( attribute );
                    if ( eventFalling.sizeData < 2 ) eventFalling.sizeData = 2; 
                }

                // subzone for Falling event
                else if ( 0 == strcmp( attr[i], "falling_subzone") ) {
                    eventFalling.data[2] = vscp2_readStringValue( attribute );
                    if ( eventFalling.sizeData < 3 ) eventFalling.sizeData = 3; 
                }

            } // for - attributes

            pMonitorObj->setMonitorEvents( eventRising, 
                                            eventFalling );

            // Add monitor to list
            pgpio->m_monitorPinList.push_back( pMonitorObj );                                            
        }
    }
    else if ( ( 0 == strcmp( name, "report" ) ) && ( 1 == depth_setup_parser ) ) {

        uint8_t id = 0;
        for ( int i = 0; attr[i]; i += 2 ) {

            std::string attribute = attr[i+1];

            // Report id 0-9    
            if ( 0 == strcmp( attr[i], "id") ) {
                id = vscp2_readStringValue( attribute );
                if ( id > 9 ) {
                    id = 0;
                    syslog( LOG_ERR,
				            "%s %s %d",
                            (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID,
				            (const char *)"Invalid id. Set to zero.",
                            (int)id );
                } 
            }

            // Pin    
            else if ( 0 == strcmp( attr[i], "pin") ) {
                pgpio->m_reporters[ id ].m_pin = 
                            vscp2_readStringValue( attribute );
            }

            // Period
            else if ( 0 == strcmp( attr[i], "period") ) {
                pgpio->m_reporters[ id ].m_period = 
                            vscp2_readStringValue( attribute );
            }

            // Low class
            else if ( 0 == strcmp( attr[i], "low_class") ) {
                pgpio->m_reporters[ id ].m_reportEventLow.vscp_class = 
                                        vscp2_readStringValue( attribute );
            }

            // Low type
            else if ( 0 == strcmp( attr[i], "low_type") ) {
                pgpio->m_reporters[ id ].m_reportEventLow.vscp_type = 
                                        vscp2_readStringValue( attribute );
            }

            // Low data
            else if ( 0 == strcmp( attr[i], "low_data") ) {
                pgpio->m_reporters[ id ].m_reportEventLow.sizeData = 0;
                vscp_setVscpEventExDataFromString( &pgpio->m_reporters[ id ].m_reportEventLow, 
                                                    attribute );                                    
            }

            // Low index
            else if ( 0 == strcmp( attr[i], "low_index") ) {
                pgpio->m_reporters[ id ].m_reportEventLow.data[0] = 
                                        vscp_readStringValue( attribute );
                if ( 0 == pgpio->m_reporters[ id ].m_reportEventLow.sizeData ) {
                    pgpio->m_reporters[ id ].m_reportEventLow.sizeData = 1;
                }
            }

            // Low zone
            else if ( 0 == strcmp( attr[i], "low_zone") ) {
                pgpio->m_reporters[ id ].m_reportEventLow.data[1] = 
                                        vscp_readStringValue( attribute );
                if ( pgpio->m_reporters[ id ].m_reportEventLow.sizeData < 2 ) {
                    pgpio->m_reporters[ id ].m_reportEventLow.sizeData = 2;
                }
            }

            // Low subzone
            else if ( 0 == strcmp( attr[i], "low_subzone") ) {
                pgpio->m_reporters[ id ].m_reportEventLow.data[2] = 
                                        vscp_readStringValue( attribute );
                if ( pgpio->m_reporters[ id ].m_reportEventLow.sizeData < 3 ) {
                    pgpio->m_reporters[ id ].m_reportEventLow.sizeData = 3;
                }
            }

            // High class
            else if ( 0 == strcmp( attr[i], "high_class") ) {
                pgpio->m_reporters[ id ].m_reportEventHigh.vscp_class = 
                                    vscp2_readStringValue( attribute );
            }

            // High type
            else if ( 0 == strcmp( attr[i], "high_type") ) {
                pgpio->m_reporters[ id ].m_reportEventHigh.vscp_type = 
                                    vscp2_readStringValue( attribute );
            }

            // High data
            else if ( 0 == strcmp( attr[i], "high_data") ) {
                pgpio->m_reporters[ id ].m_reportEventHigh.sizeData = 0;
                vscp_setVscpEventExDataFromString( &pgpio->m_reporters[ id ].m_reportEventHigh, 
                                                    attribute );                                    
            }

            // High index
            else if ( 0 == strcmp( attr[i], "high_index") ) {
                pgpio->m_reporters[ id ].m_reportEventHigh.data[0] = 
                                    vscp_readStringValue( attribute );
                if ( 0 == pgpio->m_reporters[ id ].m_reportEventHigh.sizeData ) {
                    pgpio->m_reporters[ id ].m_reportEventHigh.sizeData = 1;
                }
            }

            // High zone
            else if ( 0 == strcmp( attr[i], "high_zone") ) {
                pgpio->m_reporters[ id ].m_reportEventHigh.data[1] = 
                                    vscp_readStringValue( attribute );
                if ( pgpio->m_reporters[ id ].m_reportEventHigh.sizeData < 2 ) {
                    pgpio->m_reporters[ id ].m_reportEventHigh.sizeData = 2;
                }
            }

            // High subzone
            else if ( 0 == strcmp( attr[i], "high_subzone") ) {
                pgpio->m_reporters[ id ].m_reportEventHigh.data[2] = 
                                        vscp_readStringValue( attribute );
                if ( pgpio->m_reporters[ id ].m_reportEventHigh.sizeData < 3 ) {
                    pgpio->m_reporters[ id ].m_reportEventHigh.sizeData = 3;
                }
            }

        }
    }
    else if ( ( 0 == strcmp( name, "output" ) ) && ( 1 == depth_setup_parser ) ) {

        CGpioOutput *pOutputObj = new CGpioOutput;
        if ( NULL != pOutputObj ) {

            for ( int i = 0; attr[i]; i += 2 ) {
        
                std::string attribute = attr[i+1];

                // Get pin
                if ( 0 == strcmp( attr[i], "pin") ) {
                    pOutputObj->setPin( vscp2_readStringValue( attribute ) );
                }

                // Get initial state
                else if ( 0 == strcmp( attr[i], "initial_state" ) ) {

                    vscp2_makeUpper( attribute );
                    vscp2_trim( attribute );
                    if ( 0 == strcmp( attribute.c_str(), "OFF") ) {
                        pOutputObj->setInitialState( 0 );
                    }
                    else if ( 0 == strcmp( attribute.c_str(), "0") ) {
                        pOutputObj->setInitialState( 0 );
                    }
                    else if ( 0 == strcmp( attribute.c_str(), "ON") ) {
                        pOutputObj->setInitialState( 1 );
                    }
                    else if ( 0 == strcmp( attribute.c_str(), "1") ) {
                        pOutputObj->setInitialState( 1 );
                    }
                }

            }

        }

        // Add input to list
        pgpio->m_outputPinList.push_back( pOutputObj );

    }
    else if ( ( 0 == strcmp( name, "pwm" ) ) && ( 1 == depth_setup_parser ) ) {

        CGpioPwm *pPwmObj = new CGpioPwm;
        if ( NULL != pPwmObj ) {

            for ( int i = 0; attr[i]; i += 2 ) {
        
                std::string attribute = attr[i+1];

                // Get pin
                if ( 0 == strcmp( attr[i], "pin") ) {
                    pPwmObj->setPin( vscp2_readStringValue( attribute ) );
                }

                // Get hardware flag
                else if ( 0 == strcmp( attr[i], "hardware") ) {
                    
                    vscp2_makeUpper( attribute );
                    vscp2_trim( attribute );
                    
                    if ( 0 == strcmp( attribute.c_str(), "TRUE") ) {
                        pPwmObj->enableHardwarePwm();
                    }
                    else if ( 0 == strcmp( attribute.c_str(), "1") ) {
                        pPwmObj->enableHardwarePwm();
                    }
                    else if ( 0 == strcmp( attribute.c_str(), "FALSE") ) {
                        pPwmObj->disableHardwarePwm();
                    }
                    else if ( 0 == strcmp( attribute.c_str(), "0") ) {
                        pPwmObj->disableHardwarePwm();
                    }

                }

                // Get range
                else if ( 0 == strcmp( attr[i], "range") ) {
                    pPwmObj->setRange( vscp2_readStringValue( attribute ) );
                }

                // Get frequency
                else if ( 0 == strcmp( attr[i], "frequency") ) {
                    pPwmObj->setFrequency( vscp2_readStringValue( attribute ) );
                }

            }

        }

        // Add owm pwmoutput to list
        pgpio->m_pwmPinList.push_back( pPwmObj );

    }
    else if ( ( 0 == strcmp( name, "clock" ) ) && ( 1 == depth_setup_parser ) ) {

        CGpioClock *pGpioClockObj = new CGpioClock;
        if ( NULL != pGpioClockObj ) {

            for ( int i = 0; attr[i]; i += 2 ) {

                std::string attribute = attr[i+1];

                if ( 0 == strcmp( attr[i], "pin") ) {
                    pGpioClockObj->setPin( vscp2_readStringValue( attribute ) );
                }
                else if ( 0 == strcmp( attr[i], "frequency") ) {
                    pGpioClockObj->setFrequency( vscp2_readStringValue( attribute ) );
                }
            
            }

            // Add gpio clock output to list
            pgpio->m_gpioClockPinList.push_back( pGpioClockObj );    

        }
    
    }
    else if ( ( 0 == strcmp( name, "dm" ) ) && ( 1 == depth_setup_parser ) ) {

        CLocalDM *pLocalDMObj = new CLocalDM;
        if ( NULL != pLocalDMObj ) {

            // Get filter
            vscpEventFilter filter;

            for ( int i = 0; attr[i]; i += 2 ) {

                std::string attribute = attr[i+1];

                if ( 0 == strcmp( attr[i], "mask_priority") ) {
                    filter.mask_priority = vscp2_readStringValue( attribute );
                }
                else if ( 0 == strcmp( attr[i], "filter_priority") ) {
                    filter.filter_priority = vscp2_readStringValue( attribute );
                }
                else if ( 0 == strcmp( attr[i], "mask_class") ) {
                    filter.mask_class = vscp2_readStringValue( attribute );
                }
                else if ( 0 == strcmp( attr[i], "filter_class") ) {
                    filter.filter_class = vscp2_readStringValue( attribute );
                }
                else if ( 0 == strcmp( attr[i], "mask_type") ) {
                    filter.mask_type = vscp2_readStringValue( attribute );
                }
                else if ( 0 == strcmp( attr[i], "filter_type") ) {
                    filter.filter_type = vscp2_readStringValue( attribute );
                }
                else if ( 0 == strcmp( attr[i], "mask_type") ) {
                    vscp2_getGuidFromStringToArray( filter.mask_GUID,
                                                    attribute );
                }
                else if ( 0 == strcmp( attr[i], "filter_type") ) {
                    vscp2_getGuidFromStringToArray( filter.filter_GUID,
                                                    attribute );
                }     
                else if ( 0 == strcmp( attr[i], "index") ) {
                    pLocalDMObj->setIndex( vscp2_readStringValue( attribute ) );
                }
                else if ( 0 == strcmp( attr[i], "zone") ) {
                    pLocalDMObj->setZone( vscp2_readStringValue( attribute ) );
                }
                else if ( 0 == strcmp( attr[i], "subzone") ) {
                    pLocalDMObj->setSubZone( vscp2_readStringValue( attribute ) );
                }
                else if ( 0 == strcmp( attr[i], "action") ) {
                    pLocalDMObj->setAction( vscp2_readStringValue( attribute ) );
                }
                else if ( 0 == strcmp( attr[i], "action-parameter") ) {
                    pLocalDMObj->setActionParameter( attribute );
                }                 

            } // DM obj.

            pLocalDMObj->setFilter( filter );
            pgpio->m_LocalDMList.push_back( pLocalDMObj );

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

    for ( int i=0; i<10; i++ ) {
        m_reporters[i].m_id = 0;
        m_reporters[i].m_pin = 0;
        m_reporters[i].m_period = 0;
        memset( &m_reporters[i].m_reportEventLow, 0, sizeof( vscpEventEx ) );
        memset( &m_reporters[i].m_reportEventHigh, 0, sizeof( vscpEventEx ) );
    }

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
    

    wxString strConfig = wxString::FromAscii( pConfig );
   
	m_username = wxString::FromAscii( pUsername );
	m_password = wxString::FromAscii( pPassword );
	m_host = wxString::FromAscii( pHost );
	m_port = port;
	m_prefix = wxString::FromAscii( pPrefix );

	// Parse the configuration string. 
	wxStringTokenizer tkz( wxString::FromAscii(pConfig), _(";") );

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
				"%s %s ",
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

    XML_Parser xmlParser = XML_ParserCreate("UTF-8");
    XML_SetUserData( xmlParser, this );
    XML_SetElementHandler( xmlParser,
                            startSetupParser,
                            endSetupParser ) ;

    int bytes_read;
    void *buff = XML_GetBuffer( xmlParser, XML_BUFF_SIZE );

    strncpy( (char *)buff, m_setupXml.mbc_str(), m_setupXml.Length() );

    if ( !XML_ParseBuffer( xmlParser, bytes_read, bytes_read == 0 ) ) {
    
    }

    XML_ParserFree( xmlParser );

    

    wxStringInputStream xmlstream( m_setupXml );
    wxXmlDocument doc;

    if ( !doc.Load( xmlstream ) ) {

        syslog( LOG_ERR,
				    "%s %s ",
                    (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID,
				    (const char *)"Unable to parse XML config. Maybe just not used.");
        
    }

    // start processing the XML file
    if ( !(doc.GetRoot()->GetName() == _("setup") ) ) {

        syslog( LOG_ERR,
				    "%s %s ",
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
    attribute = doc.GetRoot()->GetAttribute("primary-dma-channel", "14");
    m_primary_dma_channel = vscp_readStringValue( attribute );

    // Secondary DMA channel
    attribute = doc.GetRoot()->GetAttribute("secondary-dma-channel", "6");
    m_secondary_dma_channel = vscp_readStringValue( attribute );
    
    // Mask
    attribute = doc.GetRoot()->GetAttribute("mask", "");
    if ( attribute.Length() ) {
        if ( !vscp_readMaskFromString( &m_vscpfilter, attribute ) ) {
            syslog(LOG_ERR,
		            "%s %s ",
                    (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID,
		            (const char *) "Unable to read event receive mask to driver filter.");
        }
    }

    // Filter
    attribute = doc.GetRoot()->GetAttribute("filter", "");
    if ( attribute.Length() ) {
        if ( !vscp_readFilterFromString( &m_vscpfilter, attribute ) ) {
            syslog(LOG_ERR,
		            "%s %s ",
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
                pInputObj->setPullUp( std::string( attribute.mb_str() ) ); 

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

                // Add input to list
                m_inputPinList.push_back( pInputObj );
                        
            } // input obj

        } // input tag

        // Define Monitor
        else if ( child->GetName() == _("monitor") ) {

            CGpioMonitor *pMonitorObj = new CGpioMonitor;
            if ( NULL != pMonitorObj ) {

                // monitor pin    
                attribute = child->GetAttribute("pin", "0");
                pMonitorObj->setPin( vscp_readStringValue( attribute ) );

                // monitor_edge
                //std::string strEdge =
                //        child->GetAttribute("edge", "rising").mbc_str().ToStdString();
                //pMonitorObj->setEdge( strEdge );            

                vscpEventEx eventRising;

                attribute =
                    child->GetAttribute("rising_class", "0");
                eventRising.vscp_class = vscp_readStringValue( attribute );

                attribute =
                    child->GetAttribute("rising_type", "0");
                eventRising.vscp_type = vscp_readStringValue( attribute );

                eventRising.sizeData = 0;
                attribute =
                    child->GetAttribute("rising_data", "0,0,0");
                
                vscp_setVscpEventExDataFromString( &eventRising, attribute );

                attribute =
                    child->GetAttribute("rising_index", "0");
                eventRising.data[0] = vscp_readStringValue( attribute );
                if ( 0 == eventRising.sizeData ) eventRising.sizeData = 1;

                attribute =
                    child->GetAttribute("rising_zone", "0");
                eventRising.data[1] = vscp_readStringValue( attribute );
                if ( eventRising.sizeData < 2 ) eventRising.sizeData = 2;

                attribute =
                    child->GetAttribute("rising_subzone", "0");
                eventRising.data[2] = vscp_readStringValue( attribute );
                if ( eventRising.sizeData < 3 ) eventRising.sizeData = 3;

                vscpEventEx eventFalling;

                attribute =
                    child->GetAttribute("rising_class", "0");
                eventFalling.vscp_class = vscp_readStringValue( attribute );

                attribute =
                    child->GetAttribute("rising_type", "0");
                eventFalling.vscp_type = vscp_readStringValue( attribute );

                eventFalling.sizeData = 0;
                attribute =
                    child->GetAttribute("rising_data", "0,0,0");
                
                vscp_setVscpEventExDataFromString( &eventFalling, attribute );

                attribute =
                    child->GetAttribute("rising_index", "0");
                eventFalling.data[0] = vscp_readStringValue( attribute );
                if ( 0 == eventFalling.sizeData ) eventFalling.sizeData = 1;

                attribute =
                    child->GetAttribute("rising_zone", "0");
                eventFalling.data[1] = vscp_readStringValue( attribute );
                if ( eventFalling.sizeData < 2 ) eventFalling.sizeData = 2;

                attribute =
                    child->GetAttribute("rising_subzone", "0");
                eventFalling.data[2] = vscp_readStringValue( attribute );
                if ( eventFalling.sizeData < 3 ) eventFalling.sizeData = 3;

                pMonitorObj->setMonitorEvents( eventRising, 
                                                eventFalling );

            }

        }

        // Define Report (max 10 )
        else if ( child->GetName() == _("report") ) {
        
            // Report id 0-9    
            attribute = child->GetAttribute("id", "0");
            uint8_t id = vscp_readStringValue( attribute );
            if ( id > 9 ) {

                syslog( LOG_ERR,
				            "%s %s %d",
                            (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID,
				            (const char *)"Invalid id.",
                            (int)id );
                continue;

            }

            // Pin    
            attribute = child->GetAttribute("pin", "0");
            m_reporters[ id ].m_pin = vscp_readStringValue( attribute );

            // Period
            attribute = child->GetAttribute("period", "0");
            m_reporters[ id ].m_period = vscp_readStringValue( attribute );

            attribute =
                child->GetAttribute("rising_class", "0");
            m_reporters[ id ].m_reportEventLow.vscp_class = vscp_readStringValue( attribute );

            attribute =
                child->GetAttribute("rising_type", "0");
           m_reporters[ id ]. m_reportEventLow.vscp_type = vscp_readStringValue( attribute );

            m_reporters[ id ].m_reportEventLow.sizeData = 0;
            attribute =
                child->GetAttribute("rising_data", "0,0,0");
                
            vscp_setVscpEventExDataFromString( &m_reporters[ id ].m_reportEventLow, attribute );

            attribute =
                child->GetAttribute("rising_index", "0");
            m_reporters[ id ].m_reportEventLow.data[0] = vscp_readStringValue( attribute );
            if ( 0 == m_reporters[ id ].m_reportEventLow.sizeData ) m_reporters[ id ].m_reportEventLow.sizeData = 1;

            attribute =
                child->GetAttribute("rising_zone", "0");
            m_reporters[ id ].m_reportEventLow.data[1] = vscp_readStringValue( attribute );
            if ( m_reporters[ id ].m_reportEventLow.sizeData < 2 ) m_reporters[ id ].m_reportEventLow.sizeData = 2;

            attribute =
                child->GetAttribute("rising_subzone", "0");
            m_reporters[ id ].m_reportEventLow.data[2] = vscp_readStringValue( attribute );
            if ( m_reporters[ id ].m_reportEventLow.sizeData < 3 ) m_reporters[ id ].m_reportEventLow.sizeData = 3;


            attribute =
                child->GetAttribute("rising_class", "0");
            m_reporters[ id ].m_reportEventHigh.vscp_class = vscp_readStringValue( attribute );

            attribute =
                child->GetAttribute("rising_type", "0");
            m_reporters[ id ].m_reportEventHigh.vscp_type = vscp_readStringValue( attribute );

            m_reporters[ id ].m_reportEventHigh.sizeData = 0;
            attribute =
                child->GetAttribute("rising_data", "0,0,0");
                
            vscp_setVscpEventExDataFromString( &m_reporters[ id ].m_reportEventHigh, attribute );

            attribute =
                child->GetAttribute("rising_index", "0");
            m_reporters[ id ].m_reportEventHigh.data[0] = vscp_readStringValue( attribute );
            if ( 0 == m_reporters[ id ].m_reportEventHigh.sizeData ) m_reporters[ id ].m_reportEventHigh.sizeData = 1;

            attribute =
                child->GetAttribute("rising_zone", "0");
            m_reporters[ id ].m_reportEventHigh.data[1] = vscp_readStringValue( attribute );
            if ( m_reporters[ id ].m_reportEventHigh.sizeData < 2 ) m_reporters[ id ].m_reportEventHigh.sizeData = 2;

            attribute =
                child->GetAttribute("rising_subzone", "0");
            m_reporters[ id ].m_reportEventHigh.data[2] = vscp_readStringValue( attribute );
            if ( m_reporters[ id ].m_reportEventHigh.sizeData < 3 ) m_reporters[ id ].m_reportEventHigh.sizeData = 3;
        }

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
                //pLocalDMObj->setActionParameter( attribute );    

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

    // If DMA channels is not default change it before initializing
    if ( ( m_primary_dma_channel != 14 ) || 
         ( m_secondary_dma_channel != 6 ) ) {
        gpioCfgDMAchannels( m_primary_dma_channel, 
                            m_secondary_dma_channel ); 
    }

    // Initialize the pigpio lib
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
    std::list<CGpioInput *>::const_iterator it1;
    for (it1 = m_inputPinList.begin(); it1 != m_inputPinList.end(); ++it1) {
       
        CGpioInput *pGpioInput = *it1;
        
        if ( NULL != pGpioInput ) {

            // Set as input
            gpioSetMode( pGpioInput->getPin(), PI_INPUT );

            // Set pullups
            gpioSetPullUpDown( pGpioInput->getPin(), pGpioInput->getPullUp() );

            // Define watchdog value
            if ( pGpioInput->getWatchdog() ) {
                gpioSetWatchdog( pGpioInput->getPin(), 
                                    pGpioInput->getWatchdog() );
            }
            // Define noice filter value
            if ( pGpioInput->getNoiceFilterSteady() ) {
                gpioNoiseFilter( pGpioInput->getPin(), 
                                    pGpioInput->getNoiceFilterSteady(),
                                    pGpioInput->getNoiceFilterActive() );
            }
            // Define glitch filter value
            if ( pGpioInput->getGlitchFilter() ) {
                gpioGlitchFilter( pGpioInput->getPin(), 
                                        pGpioInput->getGlitchFilter() );
            }

        }

    }

    // Init. output pins
    std::list<CGpioOutput *>::const_iterator it2;
    for (it2 = m_outputPinList.begin(); it2 != m_outputPinList.end(); ++it2) {

        CGpioOutput *pGpioOutput = *it2;
        
        if ( NULL != pGpioOutput ) {
            gpioSetMode( pGpioOutput->getPin(), PI_OUTPUT );
            gpioWrite( pGpioOutput->getPin(), 
                        pGpioOutput->getInitialState() ) ;
        }

    }

    // Init. pwm pins
    std::list<CGpioPwm *>::const_iterator it3;
    for ( it3 = m_pwmPinList.begin(); it3 != m_pwmPinList.end(); ++it3) {

        CGpioPwm *pGpioPwm = *it3;

        if ( NULL != pGpioPwm ) {
        
            // Hardware PWM settings
            if ( pGpioPwm->isHardwarePwm() ) {
                gpioHardwarePWM( pGpioPwm->getPin(), 
                                    pGpioPwm->getFrequency(), 
                                    pGpioPwm->getDutyCycle() );            
            }
            else {
                gpioSetPWMfrequency( pGpioPwm->getPin(), 
                                        pGpioPwm->getFrequency() );
                gpioSetPWMrange( pGpioPwm->getPin(), 
                                    pGpioPwm->getRange() ) ;
                gpioSetMode( pGpioPwm->getPin(), PI_OUTPUT );   
                gpioPWM( pGpioPwm->getPin(), pGpioPwm->getDutyCycle() ) ;     
            }

        }

    }

    // Init. clock pins
    std::list<CGpioClock *>::const_iterator it4;
    for ( it4 = m_gpioClockPinList.begin(); it4 != m_gpioClockPinList.end(); ++it4 ) {

        CGpioClock *pGpioClock = *it4;
        
        if ( NULL != pGpioClock ) {
            gpioHardwareClock( pGpioClock->getPin(), pGpioClock->getFrequency() );
        }
        
    }

    reportStruct *preport = new reportStruct;
    preport->id = 0;
    preport->pObj = this;
    gpioSetTimerFuncEx( 0, 1000, report_callback0, preport );

	// start the workerthread
	m_pthreadWorker = new RpiGpioWorkerTread();
	if ( NULL == m_pthreadWorker ) {
        return false;
    }

    // Start the worker thread
	m_pthreadWorker->m_pObj = this;
	m_pthreadWorker->Create();
	m_pthreadWorker->Run();

	return true;
}


//////////////////////////////////////////////////////////////////////
// close
//

void
CRpiGpio::close(void)
{
	// Do nothing if already terminated
	if  ( m_bQuit ) return;

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


//////////////////////////////////////////////////////////////////////
// Report callback (called at timed intervals)
// 

static void report_callback0( void *userdata  ) 
{
    reportStruct *pReport = (reportStruct *)userdata;
    if ( NULL == pReport ) return;

    if ( pReport->id != -1 ) {

    }

    delete pReport;
 }

