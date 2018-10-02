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
#include <pthread.h>
#include <semaphore.h>

#include <expat.h>
#include <pigpio.h>

#include "../../../../common/vscp.h"
#include "../../../../common/vscphelper.h"
#include "../../../../common/vscpremotetcpif.h"
#include "../../../../common/vscp_type.h"
#include "../../../../common/vscp_class.h"
#include "rpigpio.h"

#define XML_BUFF_SIZE   0xffff

// ----------------------------------------------------------------------------

// Forward declarations

void *workerThread( void *data );
static void report_callback0( void * userdata );


//////////////////////////////////////////////////////////////////////
// CRpiCGpioInputGpio
//

CGpioInput::CGpioInput()
{
    m_pin = 0;
    m_pullup = PI_PUD_OFF;
    m_watchdog = 0;
    m_noise_filter_steady = 0;
    m_noise_filter_active = 0;
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
    m_action = ACTION_RPIGPIO_NOOP;
    memset( m_args, 0, sizeof( m_args ) );
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
                else if ( 0 == strcmp( attr[i], "noise_filter_steady") ) {
                    pInputObj->setWatchdog( vscp2_readStringValue( attribute ) );
                }

                // Noice filter active
                else if ( 0 == strcmp( attr[i], "noise_filter_active") ) {
                    pInputObj->setNoiseFilterActive( vscp2_readStringValue( attribute ) );
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

                    // Preparse the action parameters
                    std::deque<std::string> tokens;
                    vscp2_split( tokens, attribute, "," );
                    for ( int idx=0; idx<3; idx++ ) {
                        if ( tokens.size() ) {
                            uint32_t val = vscp2_readStringValue( tokens.front() );
                            pLocalDMObj->setArg( idx, val );
                            tokens.pop_front();
                        }
                    }
                    
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
	//m_pthreadWorker = NULL;
	m_setupXml = _("<?xml version = \"1.0\" encoding = \"UTF-8\" ?><setup><!-- empty --></setup>");
	vscp_clearVSCPFilter( &m_vscpfilter ); // Accept all events
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

    pthread_mutex_init( &m_mutex_SendQueue, NULL );
    pthread_mutex_init( &m_mutex_ReceiveQueue, NULL );

    sem_init( &m_semaphore_SendQueue, 0, 0 );                   
    sem_init( &m_semaphore_ReceiveQueue, 0, 0 );
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

    pthread_mutex_destroy( &m_mutex_SendQueue );
    pthread_mutex_destroy( &m_mutex_ReceiveQueue );
    sem_destroy( &m_semaphore_SendQueue );
    sem_destroy( &m_semaphore_ReceiveQueue );
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
    std::string strConfig = pConfig;
   
	m_username = pUsername;
	m_password = pPassword;
	m_host = pHost;
	m_port = port;
	m_prefix = pPrefix;

	m_setupXml = pPrefix;

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

    std::string str;
	std::string strName = m_prefix + std::string("_setup");
	if ( VSCP_ERROR_SUCCESS != 
            m_srv.getRemoteVariableValue( strName, m_setupXml, true ) ) {
        // OK if not available we use default
    }

    XML_Parser xmlParser = XML_ParserCreate("UTF-8");
    XML_SetUserData( xmlParser, this );
    XML_SetElementHandler( xmlParser,
                            startSetupParser,
                            endSetupParser ) ;

    int bytes_read;
    void *buff = XML_GetBuffer( xmlParser, XML_BUFF_SIZE );

    strncpy( (char *)buff, m_setupXml.c_str(), m_setupXml.length() );

    if ( !XML_ParseBuffer( xmlParser, bytes_read, bytes_read == 0 ) ) {
        syslog( LOG_ERR,
				    "%s %s",
                    (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID,
				    (const char *) "Failed parse XML setup." );
    }

    XML_ParserFree( xmlParser );

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
    for ( it1 = m_inputPinList.begin(); it1 != m_inputPinList.end(); ++it1 ) {
       
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

            // Define noise filter value
            if ( pGpioInput->getNoiseFilterSteady() ) {
                gpioNoiseFilter( pGpioInput->getPin(), 
                                    pGpioInput->getNoiseFilterSteady(),
                                    pGpioInput->getNoiseFilterActive() );
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

    // create and start the workerthread
    if ( 0 != pthread_create( &m_pthreadWorker,  
                                NULL,          
                                workerThread,
                                (void *)this ) ) {
        syslog( LOG_ERR,
				    "%s %s",
                    (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID,
				    (const char *) "Failed to start worker thread." );
        return false;                    
    }

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
	pthread_join( m_pthreadWorker, NULL );

    // Quit gpio library functionality
    gpioTerminate();
}

//////////////////////////////////////////////////////////////////////
// addEvent2SendQueue
//

bool 
CRpiGpio::addEvent2SendQueue( const vscpEvent *pEvent )
{
    pthread_mutex_lock( &m_mutex_SendQueue );
    m_sendList.push_back( (vscpEvent *)pEvent );
	sem_post( &m_semaphore_SendQueue );
	pthread_mutex_unlock( &m_mutex_SendQueue );
    return true;
}

//////////////////////////////////////////////////////////////////////
// sendEvent
//

bool sendEvent( CRpiGpio *pObj, vscpEventEx& eventEx )
{
    vscpEvent *pEvent = new vscpEvent();
    if ( NULL != pEvent ) {
        if ( vscp_convertVSCPfromEx( pEvent, &eventEx ) ) {

            pthread_mutex_lock( &pObj->m_mutex_ReceiveQueue );
            pObj->m_receiveList.push_back( pEvent );
            sem_post( &pObj->m_semaphore_ReceiveQueue );
            pthread_mutex_lock( &pObj->m_mutex_ReceiveQueue );

        }
        else {
            vscp_deleteVSCPevent( pEvent );
            syslog( LOG_ERR,
				    "%s %s",
                    (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID,
				    (const char *) "Failed to convert send event"  );
            return false;
        }

    }

    return true;
}

//////////////////////////////////////////////////////////////////////
//                Workerthread - RpiGpioWorkerTread
//////////////////////////////////////////////////////////////////////

void *workerThread( void *data )
{
    CRpiGpio *pObj = (CRpiGpio *)data;

    while ( pObj->m_bQuit ) {
        
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 500000000;    // 500 ms
        if ( ETIMEDOUT != sem_timedwait( &pObj->m_semaphore_SendQueue, &ts ) ) {
        
            // Check if there is event(s) to handle
            if ( pObj->m_sendList.size() ) {

                // Yes there are an incoming event
                pthread_mutex_lock( &pObj->m_mutex_SendQueue );
                vscpEvent *pEvent = pObj->m_sendList.front();
                pObj->m_sendList.pop_front();
                pthread_mutex_unlock( &pObj->m_mutex_SendQueue );

                if ( NULL == pEvent ) continue;

                // Just to make sure
                if ( 0 == pEvent->sizeData ) {
                    pEvent->pdata = NULL;
                }

                // Feed through matrix - major filter
                if ( vscp_doLevel2Filter( pEvent, &pObj->m_vscpfilter ) ) {

                    std::list<CLocalDM *>::const_iterator it;
                    for ( it = pObj->m_LocalDMList.begin(); 
                            it != pObj->m_LocalDMList.end(); 
                            ++it ) {

                        CLocalDM *pDM = (CLocalDM *)*it;
                        if ( vscp_doLevel2Filter( pEvent, &pDM->getFilter() ) ) {
                            
                            if ( pDM->isIndexCheckEnabled() ) {
                                if ( ( pEvent->sizeData < 1 )  || 
                                        ( pDM->getIndex() != pEvent->pdata[0] ) ) {
                                    continue;
                                } 
                            }

                            if ( pDM->isZoneCheckEnabled() ) {
                                if ( ( pEvent->sizeData < 2 )  || 
                                        ( pDM->getZone() != pEvent->pdata[1] ) ) {
                                    continue;
                                } 
                            }

                            if ( pDM->isSubZoneCheckEnabled() ) {
                                if ( ( pEvent->sizeData )< 3   || 
                                        ( pDM->getSubZone() != pEvent->pdata[2] ) ) {
                                    continue;
                                } 
                            }

                            // OK - Do action
                            switch ( pDM->getAction() ) {

                                case ACTION_RPIGPIO_ON:
                                    {
                                        uint8_t pin = (uint8_t)pDM->getArg( 0 );
                                        if ( pin <= 53 ) {
                                            gpioWrite( pin, 1 );
                                        }
                                        else {
                                            syslog( LOG_ERR,
				                                        "%s %s pin=%d",
                                                        (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID,
				                                        (const char *) "ACTION_RPIGPIO_ON - Invalid pin",
                                                        (int)pin );
                                        }
                                    }
                                    break;

                                case ACTION_RPIGPIO_OFF:
                                    {
                                        uint8_t pin = (uint8_t)pDM->getArg( 0 );
                                        if ( pin <= 53 ) {
                                            gpioWrite( pin, 0 );
                                        }
                                        else {
                                            syslog( LOG_ERR,
				                                        "%s %s pin=%d",
                                                        (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID,
				                                        (const char *) "ACTION_RPIGPIO_OFF - Invalid pin",
                                                        (int)pin );
                                        }
                                    }
                                    break;

                                case ACTION_RPIGPIO_PWM:
                                    {
                                        uint8_t pin = (uint8_t)pDM->getArg( 0 );
                                        uint32_t dutycycle = pDM->getArg( 1 );
                                        
                                        if ( pin <= 31 ) {
                                            gpioPWM( pin, dutycycle );
                                        }
                                        else {
                                            syslog( LOG_ERR,
				                                        "%s %s pin=%d",
                                                        (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID,
				                                        (const char *) "ACTION_RPIGPIO_PWM - Invalid pin",
                                                        (int)pin );
                                        }
                                    }
                                    break;    

                                case ACTION_RPIGPIO_STATUS:
                                    {
                                        vscpEventEx ex;
                                        memset( &ex, 0, sizeof( vscpEventEx) );
                                        uint8_t index = 0;
                                        uint8_t zone = 0;
                                        uint8_t subzone = 0;

                                        uint8_t pin = (uint8_t)pDM->getArg( 0 );
                                        
                                        if ( pin > 53 ) {
                                            syslog( LOG_ERR,
				                                        "%s %s pin=%d",
                                                        (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID,
				                                        (const char *) "ACTION_RPIGPIO_STATUS - Invalid pin",
                                                        (int)pin );
                                        }

                                        int level = gpioRead( pin );
                                        if ( PI_BAD_GPIO == level ) {
                                            syslog( LOG_ERR,
				                                        "%s %s pin=%d",
                                                        (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID,
				                                        (const char *) "ACTION_RPIGPIO_STATUS - Invalid pin (read)",
                                                        (int)pin );
                                        } 

                                        // index
                                        index = (uint8_t)pDM->getArg( 1 );

                                        // zone
                                        zone = (uint8_t)pDM->getArg( 2 );

                                        // subzone
                                        subzone = (uint8_t)pDM->getArg( 3 );

                                        ex.vscp_class = VSCP_CLASS1_INFORMATION;
                                        ex.sizeData = 3;
                                        ex.data[0] = index;
                                        ex.data[1] = zone;
                                        ex.data[2] = subzone;

                                        if ( level ) {
                                            ex.vscp_type = VSCP_TYPE_INFORMATION_ON;
                                        }
                                        else {
                                            ex.vscp_type = VSCP_TYPE_INFORMATION_OFF;
                                        }

                                        sendEvent( pObj, ex );

                                    }
                                    break;
                                    
                                
                                case ACTION_RPIGPIO_SERVO:
                                    {
                                        uint8_t pin = (uint8_t)pDM->getArg( 0 );
                                        uint32_t pulsewidth = pDM->getArg( 0 );
                                        
                                        if ( pin <= 31 ) {
                                            gpioServo( pin, pulsewidth );
                                        }
                                        else {
                                            syslog( LOG_ERR,
				                                        "%s %s pin=%d",
                                                        (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID,
				                                        (const char *) "ACTION_RPIGPIO_PWM - Invalid pin",
                                                        (int)pin );
                                        }
                                    }
                                    break; 

                                case ACTION_RPIGPIO_NOOP:
                                default:
                                    break;


                            }

                        }

                    }

                }

                // We are done with the event
                vscp_deleteVSCPevent_v2( &pEvent );

            } // Event in queue

        }

    } // while

    return NULL;
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

