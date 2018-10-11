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

#include "../../../../common/vscp.h"
#include "../../../../common/vscphelper.h"
#include "../../../../common/vscpremotetcpif.h"
#include "../../../../common/vscp_type.h"
#include "../../../../common/vscp_class.h"
#include "rpigpio.h"
#ifdef USE_PIGPIOD
#include <pigpiod_if2.h>
#else
    #include <pigpio.h>
#endif

// Undef to get extra debug info to syslog
#define RPIGPIO_DEBUG

#define XML_BUFF_SIZE   0xffff

// ----------------------------------------------------------------------------

// Forward declarations

void *workerThread( void *data );

static void monitor_callback( int handle,
                                unsigned gpio,
                                unsigned edge,
                                uint32_t tick,
                                void *userdata );

static void report_callback0( void * userdata );
static void report_callback1( void * userdata );
static void report_callback2( void * userdata );
static void report_callback3( void * userdata );
static void report_callback4( void * userdata );
static void report_callback5( void * userdata );
static void report_callback6( void * userdata );
static void report_callback7( void * userdata );
static void report_callback8( void * userdata );
static void report_callback9( void * userdata );

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
    if ( pin < 32 ) {
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
        m_pullup = PI_PUD_UP;
    } 
    else if  ( std::string::npos != str.find("DOWN") ) {
        m_pullup = PI_PUD_DOWN;
    }
    else if  ( std::string::npos != str.find("OFF") ) {
        m_pullup = PI_PUD_OFF;
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



// ----------------------------------------------------------------------------




//////////////////////////////////////////////////////////////////////
// CGpioOutput
//

CGpioOutput::CGpioOutput() 
{
    m_pin = 0;
    m_state = 255; // Do not set initial state
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
    m_pin = pin;
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
    if ( 0 == state ) {
        m_state = 0;
    }
    else if ( 1 == state ) {
        m_state = 1;
    }
    else {
        m_state = 255; // Don't set initial state
    }
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
    m_bEnable = true;
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
// setAction
//

bool CLocalDM::setAction( std::string& str )
{
    vscp2_makeUpper( str );
    vscp2_trim( str );

    if ( std::string::npos != str.find("NOOP") ) {
        m_action = ACTION_RPIGPIO_NOOP;
    }
    else if ( std::string::npos != str.find("ON") ) {
        m_action = ACTION_RPIGPIO_ON;
    }
    else if ( std::string::npos != str.find("OFF") ) {
        m_action = ACTION_RPIGPIO_OFF;
    }
    else if ( std::string::npos != str.find("PWM") ) {
        m_action = ACTION_RPIGPIO_PWM;
    }
    else if ( std::string::npos != str.find("FREQUENCY") ) {
        m_action = ACTION_RPIGPIO_FREQUENCY;
    }
    else if ( std::string::npos != str.find("STATUS") ) {
        m_action = ACTION_RPIGPIO_STATUS;
    }
    else if ( std::string::npos != str.find("SERVO") ) {
        m_action = ACTION_RPIGPIO_SERVO;
    }
    else if ( std::string::npos != str.find("WAVEFORM") ) {
        m_action = ACTION_RPIGPIO_WAVEFORM;
    }
    else if ( std::string::npos != str.find("SHIFTOUT") ) {
        m_action = ACTION_RPIGPIO_SHIFTOUT;
    }
    else if ( std::string::npos != str.find("SHIFTOUT-EVENT") ) {
        m_action = ACTION_RPIGPIO_SHIFTOUT_EVENT;
    }
    else if ( std::string::npos != str.find("SHIFTIN") ) {
        m_action = ACTION_RPIGPIO_SHIFTIN;
    }
    else if ( std::string::npos != str.find("SAMPLE") ) {
        m_action = ACTION_RPIGPIO_SAMPLE;
    }
    else if ( std::string::npos != str.find("SERIAL-OUT") ) {
        m_action = ACTION_RPIGPIO_SERIAL_OUT;
    }
    else if ( std::string::npos != str.find("SERIAL-IN") ) {
        m_action = ACTION_RPIGPIO_SERIAL_IN;
    }

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
/*
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
    m_eventFalling.vscp_type = VSCP_TYPE_INFORMATION_FALLING;
    m_eventFalling.head = 0;
    m_eventFalling.sizeData = 3;
    memset( m_eventFalling.data, 0, sizeof( m_eventFalling.data ) );
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
    m_eventRising.vscp_type = VSCP_TYPE_INFORMATION_RISING;
    m_eventRising.head = 0;
    m_eventRising.sizeData = 3;
    memset( m_eventRising.data, 0, sizeof( m_eventRising.data ) );
    memset( m_eventRising.GUID, 0, 16 );

    m_eventWatchdog.obid = 0;
    m_eventWatchdog.timestamp = 0;
    m_eventWatchdog.year = 0;
    m_eventWatchdog.month = 0;
    m_eventWatchdog.day = 0;
    m_eventWatchdog.hour = 0;
    m_eventWatchdog.minute = 0;
    m_eventWatchdog.second = 0;
    m_eventWatchdog.vscp_class = VSCP_CLASS1_ALARM;
    m_eventWatchdog.vscp_type = VSCP_TYPE_ALARM_WATCHDOG;
    m_eventWatchdog.head = 0;
    m_eventWatchdog.sizeData = 3;
    memset( m_eventWatchdog.data, 0, sizeof( m_eventWatchdog.data ) );
    memset( m_eventWatchdog.GUID, 0, 16 );
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
                                     const vscpEventEx& eventRising,
                                     const vscpEventEx& eventWatchdog )
{
    vscp_copyVSCPEventEx( &m_eventFalling, &eventFalling );
    vscp_copyVSCPEventEx( &m_eventRising, &eventRising );
    vscp_copyVSCPEventEx( &m_eventWatchdog, &eventWatchdog );
}
*/

// ----------------------------------------------------------------------------

int depth_setup_parser = 0;
bool bDM = false;

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
#ifdef USE_PIGPIOD          
            else if ( 0 == strcmp( attr[i], "pigpiod-host") ) {
                pgpio->setPiGpiodHost( attribute );
            }
            else if ( 0 == strcmp( attr[i], "pigpiod-port") ) {
                pgpio->setPiGpiodPort( attribute );
            }
#endif            
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
            else if ( 0 == strcmp( attr[i], "guid") ) {
                if ( !attribute.empty() ) {
                    pgpio->m_ifguid.getFromString( attribute );
                }
            }
            else if ( 0 == strcmp( attr[i], "index") ) {
                if ( !attribute.empty() ) {
                    pgpio->m_index = (uint8_t)vscp2_readStringValue( attribute );
                }
            }
            else if ( 0 == strcmp( attr[i], "zone") ) {
                if ( !attribute.empty() ) {
                    pgpio->m_zone = (uint8_t)vscp2_readStringValue( attribute );
                }
            }
            else if ( 0 == strcmp( attr[i], "subzone") ) {
                if ( !attribute.empty() ) {
                    pgpio->m_subzone = (uint8_t)vscp2_readStringValue( attribute );
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
                    else if ( 0 == strcmp( attribute.c_str(), "X") ) {
                        pOutputObj->setInitialState( 255 );  // do not set
                    }
                }

            }

        }

        // Add input to list
        pgpio->m_outputPinList.push_back( pOutputObj );

    }
    else if ( ( 0 == strcmp( name, "monitor" ) ) && ( 1 == depth_setup_parser ) ) {
        
        int pin = 0;
        for ( int i = 0; attr[i]; i += 2 ) {            

            std::string attribute = attr[i+1];

            // Get monitor pin 
            if ( 0 == strcmp( attr[i], "pin") ) {
                pin = vscp2_readStringValue( attribute );
                if ( pin > 31 ) pin = 0;
                pgpio->m_monitor[pin].bEnable = true;
            }

            // monitor_edge
            else if ( 0 == strcmp( attr[i], "edge") ) {
                std::string str = attribute;
                vscp2_makeUpper( str );
                vscp2_trim( str );

                if ( std::string::npos != str.find("FALLING") ) {
                    pgpio->m_monitor[pin].edge = FALLING_EDGE;
                } 
                else if ( std::string::npos != str.find("RISING") ) {
                    pgpio->m_monitor[pin].edge = RISING_EDGE;
                }
                else if ( std::string::npos != str.find("BOTH") ) {
                    pgpio->m_monitor[pin].edge = EITHER_EDGE;
                }
                else if ( std::string::npos != str.find("SETUP") ) {
                    pgpio->m_monitor[pin].edge = EITHER_EDGE;
                }
                else {
                    pgpio->m_monitor[pin].edge = EITHER_EDGE;
                } 
            }

            // class for Rising event
            else if ( 0 == strcmp( attr[i], "rising_class") ) {
                pgpio->m_monitor[pin].eventRising.vscp_class = vscp2_readStringValue( attribute ); 
            }

            // type for Rising event
            else if ( 0 == strcmp( attr[i], "rising_type") ) {
                pgpio->m_monitor[pin].eventRising.vscp_type = vscp2_readStringValue( attribute ); 
            }

            // data for Rising event
            else if ( 0 == strcmp( attr[i], "rising_type") ) {
                vscp2_setVscpEventExDataFromString( &pgpio->m_monitor[pin].eventRising, attribute ); 
            }

            // index for Rising event
            else if ( 0 == strcmp( attr[i], "rising_index") ) {
                pgpio->m_monitor[pin].eventRising.data[0] = vscp2_readStringValue( attribute );
                if ( 0 == pgpio->m_monitor[pin].eventRising.sizeData ) pgpio->m_monitor[pin].eventRising.sizeData = 1; 
            }

            // zone for Rising event
            else if ( 0 == strcmp( attr[i], "rising_zone") ) {
                pgpio->m_monitor[pin].eventRising.data[1] = vscp2_readStringValue( attribute );
                if ( pgpio->m_monitor[pin].eventRising.sizeData < 2 ) pgpio->m_monitor[pin].eventRising.sizeData = 2; 
            }

            // subzone for Rising event
            else if ( 0 == strcmp( attr[i], "rising_subzone") ) {
                pgpio->m_monitor[pin].eventRising.data[2] = vscp2_readStringValue( attribute );
                if ( pgpio->m_monitor[pin].eventRising.sizeData < 3 ) pgpio->m_monitor[pin].eventRising.sizeData = 3; 
            }

            // ----------------------------------------------------------------

            // class for Falling event
            else if ( 0 == strcmp( attr[i], "falling_class") ) {
                pgpio->m_monitor[pin].eventFalling.vscp_class = vscp2_readStringValue( attribute ); 
            }

            // type for Falling event
            else if ( 0 == strcmp( attr[i], "falling_type") ) {
                pgpio->m_monitor[pin].eventFalling.vscp_type = vscp2_readStringValue( attribute ); 
            }

            // data for Rising event
            else if ( 0 == strcmp( attr[i], "rising_type") ) {
                vscp2_setVscpEventExDataFromString( &pgpio->m_monitor[pin].eventFalling, attribute ); 
            }

            // index for Falling event
            else if ( 0 == strcmp( attr[i], "falling_index") ) {
                pgpio->m_monitor[pin].eventFalling.data[0] = vscp2_readStringValue( attribute );
                if ( 0 == pgpio->m_monitor[pin].eventFalling.sizeData ) pgpio->m_monitor[pin].eventFalling.sizeData = 1; 
            }

            // zone for Falling event
            else if ( 0 == strcmp( attr[i], "falling_zone") ) {
                pgpio->m_monitor[pin].eventFalling.data[1] = vscp2_readStringValue( attribute );
                if ( pgpio->m_monitor[pin].eventFalling.sizeData < 2 ) pgpio->m_monitor[pin].eventFalling.sizeData = 2; 
            }

            // subzone for Falling event
            else if ( 0 == strcmp( attr[i], "falling_subzone") ) {
                pgpio->m_monitor[pin].eventFalling.data[2] = vscp2_readStringValue( attribute );
                if ( pgpio->m_monitor[pin].eventFalling.sizeData < 3 ) pgpio->m_monitor[pin].eventFalling.sizeData = 3; 
            }

            // ----------------------------------------------------------------

            // class for Watchdog event
            else if ( 0 == strcmp( attr[i], "watchdog_class") ) {
                pgpio->m_monitor[pin].eventWatchdog.vscp_class = vscp2_readStringValue( attribute ); 
            }

            // type for Watchdog event
            else if ( 0 == strcmp( attr[i], "watchdog_type") ) {
                pgpio->m_monitor[pin].eventWatchdog.vscp_type = vscp2_readStringValue( attribute ); 
            }

            // data for Watchdog event
            else if ( 0 == strcmp( attr[i], "watchdog_type") ) {
                vscp2_setVscpEventExDataFromString( &pgpio->m_monitor[pin].eventWatchdog, attribute ); 
            }

            // index for Falling event
            else if ( 0 == strcmp( attr[i], "watchdog_index") ) {
                pgpio->m_monitor[pin].eventWatchdog.data[0] = vscp2_readStringValue( attribute );
                if ( 0 == pgpio->m_monitor[pin].eventWatchdog.sizeData ) pgpio->m_monitor[pin].eventWatchdog.sizeData = 1; 
            }

            // zone for Watchdog event
            else if ( 0 == strcmp( attr[i], "watchdog_zone") ) {
                pgpio->m_monitor[pin].eventWatchdog.data[1] = vscp2_readStringValue( attribute );
                if ( pgpio->m_monitor[pin].eventWatchdog.sizeData < 2 ) pgpio->m_monitor[pin].eventWatchdog.sizeData = 2; 
            }

            // subzone for Watchdog event
            else if ( 0 == strcmp( attr[i], "watchdog_subzone") ) {
                pgpio->m_monitor[pin].eventWatchdog.data[2] = vscp2_readStringValue( attribute );
                if ( pgpio->m_monitor[pin].eventWatchdog.sizeData < 3 ) pgpio->m_monitor[pin].eventWatchdog.sizeData = 3; 
            }

        } // for - attributes

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
                pgpio->m_reporters[ id ].bEnable = true;
            }

            // Pin    
            else if ( 0 == strcmp( attr[i], "pin") ) {
                pgpio->m_reporters[ id ].pin = 
                            vscp2_readStringValue( attribute );
            }

            // Period
            else if ( 0 == strcmp( attr[i], "period") ) {
                pgpio->m_reporters[ id ].period = 
                            vscp2_readStringValue( attribute );
            }

            // Low class
            else if ( 0 == strcmp( attr[i], "low_class") ) {
                pgpio->m_reporters[ id ].eventLow.vscp_class = 
                                        vscp2_readStringValue( attribute );
            }

            // Low type
            else if ( 0 == strcmp( attr[i], "low_type") ) {
                pgpio->m_reporters[ id ].eventLow.vscp_type = 
                                        vscp2_readStringValue( attribute );
            }

            // Low data
            else if ( 0 == strcmp( attr[i], "low_data") ) {
                pgpio->m_reporters[ id ].eventLow.sizeData = 0;
                vscp_setVscpEventExDataFromString( &pgpio->m_reporters[ id ].eventLow, 
                                                    attribute );                                    
            }

            // Low index
            else if ( 0 == strcmp( attr[i], "low_index") ) {
                pgpio->m_reporters[ id ].eventLow.data[0] = 
                                        vscp_readStringValue( attribute );
                if ( 0 == pgpio->m_reporters[ id ].eventLow.sizeData ) {
                    pgpio->m_reporters[ id ].eventLow.sizeData = 1;
                }
            }

            // Low zone
            else if ( 0 == strcmp( attr[i], "low_zone") ) {
                pgpio->m_reporters[ id ].eventLow.data[1] = 
                                        vscp_readStringValue( attribute );
                if ( pgpio->m_reporters[ id ].eventLow.sizeData < 2 ) {
                    pgpio->m_reporters[ id ].eventLow.sizeData = 2;
                }
            }

            // Low subzone
            else if ( 0 == strcmp( attr[i], "low_subzone") ) {
                pgpio->m_reporters[ id ].eventLow.data[2] = 
                                        vscp_readStringValue( attribute );
                if ( pgpio->m_reporters[ id ].eventLow.sizeData < 3 ) {
                    pgpio->m_reporters[ id ].eventLow.sizeData = 3;
                }
            }

            // High class
            else if ( 0 == strcmp( attr[i], "high_class") ) {
                pgpio->m_reporters[ id ].eventHigh.vscp_class = 
                                    vscp2_readStringValue( attribute );
            }

            // High type
            else if ( 0 == strcmp( attr[i], "high_type") ) {
                pgpio->m_reporters[ id ].eventHigh.vscp_type = 
                                    vscp2_readStringValue( attribute );
            }

            // High data
            else if ( 0 == strcmp( attr[i], "high_data") ) {
                pgpio->m_reporters[ id ].eventHigh.sizeData = 0;
                vscp_setVscpEventExDataFromString( &pgpio->m_reporters[ id ].eventHigh, 
                                                    attribute );                                    
            }

            // High index
            else if ( 0 == strcmp( attr[i], "high_index") ) {
                pgpio->m_reporters[ id ].eventHigh.data[0] = 
                                    vscp_readStringValue( attribute );
                if ( 0 == pgpio->m_reporters[ id ].eventHigh.sizeData ) {
                    pgpio->m_reporters[ id ].eventHigh.sizeData = 1;
                }
            }

            // High zone
            else if ( 0 == strcmp( attr[i], "high_zone") ) {
                pgpio->m_reporters[ id ].eventHigh.data[1] = 
                                    vscp_readStringValue( attribute );
                if ( pgpio->m_reporters[ id ].eventHigh.sizeData < 2 ) {
                    pgpio->m_reporters[ id ].eventHigh.sizeData = 2;
                }
            }

            // High subzone
            else if ( 0 == strcmp( attr[i], "high_subzone") ) {
                pgpio->m_reporters[ id ].eventHigh.data[2] = 
                                        vscp_readStringValue( attribute );
                if ( pgpio->m_reporters[ id ].eventHigh.sizeData < 3 ) {
                    pgpio->m_reporters[ id ].eventHigh.sizeData = 3;
                }
            }
        }

        timeval curTime;
        gettimeofday( &curTime, NULL );
        uint32_t now = 1000 * curTime.tv_sec + ( curTime.tv_usec / 1000 );

        pgpio->m_reporters[ id ].last = now;
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
        bDM = true;
    }
    else if ( bDM && 
                ( 0 == strcmp( name, "row" ) ) && 
                ( 2 == depth_setup_parser ) ) {

        CLocalDM *pLocalDMObj = new CLocalDM;
        if ( NULL != pLocalDMObj ) {

            // Get filter
            vscpEventFilter filter;

            for ( int i = 0; attr[i]; i += 2 ) {

                std::string attribute = attr[i+1];

                if ( 0 == strcmp( attr[i], "enable") ) {   
                    
                    vscp2_makeUpper( attribute );
                    vscp2_trim( attribute );
                    
                    if ( 0 == strcmp( attribute.c_str(), "TRUE") ) {
                        pLocalDMObj->enableRow();
                    }
                    else if ( 0 == strcmp( attribute.c_str(), "FALSE") ) {
                        pLocalDMObj->disableRow();
                    }

                } 
                else if ( 0 == strcmp( attr[i], "priority-mask") ) {
                    filter.mask_priority = vscp2_readStringValue( attribute );
                }
                else if ( 0 == strcmp( attr[i], "priority-filter") ) {
                    filter.filter_priority = vscp2_readStringValue( attribute );
                }
                else if ( 0 == strcmp( attr[i], "class-mask") ) {
                    filter.mask_class = vscp2_readStringValue( attribute );
                }
                else if ( 0 == strcmp( attr[i], "class-filter") ) {
                    filter.filter_class = vscp2_readStringValue( attribute );
                }
                else if ( 0 == strcmp( attr[i], "type-mask") ) {
                    filter.mask_type = vscp2_readStringValue( attribute );
                }
                else if ( 0 == strcmp( attr[i], "type-filter") ) {
                    filter.filter_type = vscp2_readStringValue( attribute );
                }
                else if ( 0 == strcmp( attr[i], "guid-mask") ) {
                    vscp2_getGuidFromStringToArray( filter.mask_GUID,
                                                        attribute );
                }
                else if ( 0 == strcmp( attr[i], "guid-filter") ) {
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
                    pLocalDMObj->setAction( attribute );
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

    if ( 0 == strcmp( name, "dm" ) ) bDM = false;
}  

// ----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////
// CRpiGpio
//

CRpiGpio::CRpiGpio()
{
	int i;
    m_bQuit = false;
	m_setupXml = _("<?xml version = \"1.0\" encoding = \"UTF-8\" ?><setup><!-- empty --></setup>");
	vscp_clearVSCPFilter( &m_vscpfilter ); // Accept all events
    m_sample_rate = 5;   
    m_primary_dma_channel = 14;
    m_secondary_dma_channel = 6;
    m_index = 0;
    m_zone = 0;
    m_subzone = 0;

#ifdef USE_PIGPIOD 
    m_pigpiod_host = "127.0.0.1";
    m_pigpiod_port = "8888";
#endif    

    timeval curTime;
    gettimeofday( &curTime, NULL );
    uint32_t now = 1000 * curTime.tv_sec + ( curTime.tv_usec / 1000 );

    for ( i=0; i<10; i++ ) {
        m_reporters[i].bEnable = false;
        m_reporters[i].pin = 0;
        m_reporters[i].period = 0;
        m_reporters[i].last = now;
        memset( &m_reporters[i].eventLow, 0, sizeof( vscpEventEx ) );
        memset( &m_reporters[i].eventHigh, 0, sizeof( vscpEventEx ) );
    }

    // Init monitor
    for ( i=0; i<32; i++ ) {
        m_monitor[i].bEnable = false;
        m_monitor[i].edge = 
        m_monitor[i].edge = EITHER_EDGE;
        
        m_monitor[i].eventFalling.obid = 0;
        m_monitor[i].eventFalling.timestamp = 0;
        m_monitor[i].eventFalling.year = 0;
        m_monitor[i].eventFalling.month = 0;
        m_monitor[i].eventFalling.day = 0;
        m_monitor[i].eventFalling.hour = 0;
        m_monitor[i].eventFalling.minute = 0;
        m_monitor[i].eventFalling.second = 0;
        m_monitor[i].eventFalling.vscp_class = VSCP_CLASS1_INFORMATION;;
        m_monitor[i].eventFalling.vscp_type = VSCP_TYPE_INFORMATION_FALLING;
        m_monitor[i].eventFalling.head = 0;
        m_monitor[i].eventFalling.sizeData = 3;
        memset( m_monitor[i].eventFalling.data, 0, sizeof( m_monitor[i].eventFalling.data ) );
        memset( m_monitor[i].eventFalling.GUID, 0, 16 );
    
        m_monitor[i].eventRising.obid = 0;
        m_monitor[i].eventRising.timestamp = 0;
        m_monitor[i].eventRising.year = 0;
        m_monitor[i].eventRising.month = 0;
        m_monitor[i].eventRising.day = 0;
        m_monitor[i].eventRising.hour = 0;
        m_monitor[i].eventRising.minute = 0;
        m_monitor[i].eventRising.second = 0;
        m_monitor[i].eventRising.vscp_class = VSCP_CLASS1_INFORMATION;
        m_monitor[i].eventRising.vscp_type = VSCP_TYPE_INFORMATION_RISING;
        m_monitor[i].eventRising.head = 0;
        m_monitor[i].eventRising.sizeData = 3;
        memset( m_monitor[i].eventRising.data, 0, VSCP_MAX_DATA );
        memset( m_monitor[i].eventRising.GUID, 0, 16 );

        m_monitor[i].eventWatchdog.obid = 0;
        m_monitor[i].eventWatchdog.timestamp = 0;
        m_monitor[i].eventWatchdog.year = 0;
        m_monitor[i].eventWatchdog.month = 0;
        m_monitor[i].eventWatchdog.day = 0;
        m_monitor[i].eventWatchdog.hour = 0;
        m_monitor[i].eventWatchdog.minute = 0;
        m_monitor[i].eventWatchdog.second = 0;
        m_monitor[i].eventWatchdog.vscp_class = VSCP_CLASS1_ALARM;
        m_monitor[i].eventWatchdog.vscp_type = VSCP_TYPE_ALARM_WATCHDOG;
        m_monitor[i].eventWatchdog.head = 0;
        m_monitor[i].eventWatchdog.sizeData = 3;
        memset( m_monitor[i].eventWatchdog.data, 0, VSCP_MAX_DATA );
        memset( m_monitor[i].eventWatchdog.GUID, 0, 16 );
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
    // Close if not closed
	if ( !m_bQuit ) close();

    // Remove input pin defines
    std::list<CGpioInput *>::const_iterator iterator1;
    for (iterator1 = m_inputPinList.begin(); iterator1 != m_inputPinList.end(); ++iterator1) {
        delete *iterator1;
    }

    // Remove output pin defines
    std::list<CGpioOutput *>::const_iterator iterator2;
    for (iterator2 = m_outputPinList.begin(); iterator2 != m_outputPinList.end(); ++iterator2) {
        delete *iterator2;
    }

    // Remove pwm pin defines
    std::list<CGpioPwm *>::const_iterator iterator3;
    for (iterator3 = m_pwmPinList.begin(); iterator3 != m_pwmPinList.end(); ++iterator3) {
        delete *iterator3;
    }

    // Remove gpio clock pin defines
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

    bytes_read = m_setupXml.length();
    if ( !XML_ParseBuffer( xmlParser, bytes_read, bytes_read == 0 ) ) {
        syslog( LOG_ERR,
				    "%s %s",
                    (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID,
				    (const char *) "Failed parse XML setup." );
    }

    XML_ParserFree( xmlParser );

    // Close the channel
	m_srv.doCmdClose();

    {
        std::list<CGpioOutput *>::const_iterator it;
        for (it = m_outputPinList.begin(); it != m_outputPinList.end(); ++it) {

            CGpioOutput *pGpioOutput = *it;
        
            if ( NULL != pGpioOutput ) {
                uint8_t pin = pGpioOutput->getPin();
                uint8_t state = pGpioOutput->getInitialState();
            }
        }
    }

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
CRpiGpio::close( void )
{
	m_bQuit = true; // terminate the thread
    pthread_join( m_pthreadWorker, NULL );
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
            pthread_mutex_unlock( &pObj->m_mutex_ReceiveQueue );

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

#ifndef USE_PIGPIOD
    // If samplerate is not default change it before initializing
    if ( pObj->m_sample_rate != 5 ) {
        gpioCfgClock( pObj->m_sample_rate, 1, 0 );
    }

    // If DMA channels is not default change it before initializing
    if ( ( pObj->m_primary_dma_channel != 14 ) || 
         ( pObj->m_secondary_dma_channel != 6 ) ) {
        gpioCfgDMAchannels( pObj->m_primary_dma_channel, 
                            pObj->m_secondary_dma_channel ); 
    }
#endif    

    // Initialize the pigpio lib
    //gpioCfgInternals(1<<10, 0);  // Prevent signal 11 error
#ifdef USE_PIGPIOD
    int pgpiod_session_id;
    if ( ( pgpiod_session_id = pigpio_start( (char *)pObj->getPiGpiodHost().c_str(), 
                                                (char *)pObj->getPiGpiodPort().c_str() ) ) < 0 ) {
        syslog( LOG_ERR,
				    "%s %s driver started pigpio version: %ul, HW rev: %ul",
                    (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID,
				    (const char *) "Failed to init. gpiod library.",
                    get_pigpio_version( pgpiod_session_id ), 
                    get_hardware_revision( pgpiod_session_id ) ); 
        return NULL;                                      
    }
#else
	if ( gpioInitialise() < 0 ) {
        syslog( LOG_ERR,
				    "%s %s driver started pigpio version: %ul, HW rev: %ul",
                    (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID,
				    (const char *) "Failed to init. gpio library.",
                    gpioVersion(), gpioHardwareRevision() );
        return NULL;                    
    }
#endif                    
    

    // Debug
    syslog( LOG_ERR, 
                "%s gpioversion - Version=%d HW version =%d",
                (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID, 
                gpioVersion(), gpioHardwareRevision() );

    // Setup pin functionality

    // Init. input pins
    {
        std::list<CGpioInput *>::const_iterator it;
        for ( it = pObj->m_inputPinList.begin(); 
                it != pObj->m_inputPinList.end(); 
                ++it ) {
       
            CGpioInput *pGpioInput = *it;
        
            if ( NULL != pGpioInput ) {

            // Set as input
#ifdef USE_PIGPIOD            
                set_mode( pgpiod_session_id, pGpioInput->getPin(), PI_INPUT );
#else            
                gpioSetMode( pGpioInput->getPin(), PI_INPUT );
#endif            

            // Set pullups
#ifdef USE_PIGPIOD
                set_pull_up_down( pgpiod_session_id, 
                                    pGpioInput->getPin(), 
                                    pGpioInput->getPullUp() );
#else            
                gpioSetPullUpDown( pGpioInput->getPin(), 
                                        pGpioInput->getPullUp() );
#endif            

                // Define watchdog value
                if ( pGpioInput->getWatchdog() ) {
#ifdef USE_PIGPIOD                
                    set_watchdog( pgpiod_session_id, 
                                    pGpioInput->getPin(), 
                                    pGpioInput->getWatchdog() );
#else                                
                    gpioSetWatchdog( pGpioInput->getPin(), 
                                        pGpioInput->getWatchdog() );
#endif                                    
                }

                // Define noise filter value
                if ( pGpioInput->getNoiseFilterSteady() ) {
#ifdef USE_PIGPIOD                 
                    set_noise_filter( pgpiod_session_id,
                                        pGpioInput->getPin(), 
                                        pGpioInput->getNoiseFilterSteady(),
                                        pGpioInput->getNoiseFilterActive() );
#else                                    
                    gpioNoiseFilter( pGpioInput->getPin(), 
                                        pGpioInput->getNoiseFilterSteady(),
                                        pGpioInput->getNoiseFilterActive() );
#endif                                    
                }

                // Define glitch filter value
                if ( pGpioInput->getGlitchFilter() ) {
#ifdef USE_PIGPIOD
                    set_glitch_filter( pgpiod_session_id,
                                        pGpioInput->getPin(), 
                                        pGpioInput->getGlitchFilter() );
#else                
                    gpioGlitchFilter( pGpioInput->getPin(), 
                                            pGpioInput->getGlitchFilter() );
#endif                                        
                }

            }

        }

    }

    // Init. output pins
    {
        std::list<CGpioOutput *>::const_iterator it;
        for ( it = pObj->m_outputPinList.begin(); 
                it != pObj->m_outputPinList.end(); 
                ++it ) {

            CGpioOutput *pGpioOutput = *it;
        
            if ( NULL != pGpioOutput ) {
#ifdef USE_PIGPIOD            
                set_mode( pgpiod_session_id, pGpioOutput->getPin(), PI_OUTPUT );
#else             
                gpioSetMode( pGpioOutput->getPin(), PI_OUTPUT );
#endif            
                if ( ( 0 == pGpioOutput->getInitialState() ) || 
                     ( 1 == pGpioOutput->getInitialState() ) ) {
#ifdef USE_PIGPIOD
                    gpio_write( pgpiod_session_id,
                                pGpioOutput->getPin(), 
                                pGpioOutput->getInitialState() ) ;
#else                     
                    gpioWrite( pGpioOutput->getPin(), 
                                pGpioOutput->getInitialState() ) ;
#endif                            
                }

            }

        }   
    }

    // Init. pwm pins
    {
        std::list<CGpioPwm *>::const_iterator it;
        for ( it = pObj->m_pwmPinList.begin(); 
                it != pObj->m_pwmPinList.end(); 
                ++it) {

            CGpioPwm *pGpioPwm = *it;

            if ( NULL != pGpioPwm ) {
        
                // Hardware PWM settings
                if ( pGpioPwm->isHardwarePwm() ) {
#ifdef USE_PIGPIOD
                    hardware_PWM( pgpiod_session_id, 
                                    pGpioPwm->getPin(), 
                                    pGpioPwm->getFrequency(), 
                                    pGpioPwm->getDutyCycle() );
#else
                    gpioHardwarePWM( pGpioPwm->getPin(), 
                                        pGpioPwm->getFrequency(), 
                                        pGpioPwm->getDutyCycle() );            
#endif                                    
                }
                else {
#ifdef USE_PIGPIOD
                    set_mode( pgpiod_session_id, pGpioPwm->getPin(), PI_OUTPUT );
                    set_PWM_frequency( pgpiod_session_id, pGpioPwm->getPin(), 
                                        pGpioPwm->getFrequency() );
                    set_PWM_range( pgpiod_session_id, pGpioPwm->getPin(), 
                                        pGpioPwm->getRange() ) ;
                   
                    set_PWM_dutycycle( pgpiod_session_id, 
                                        pGpioPwm->getPin(), 
                                        pGpioPwm->getDutyCycle() ) ;
#else                
                    gpioSetMode( pGpioPwm->getPin(), PI_OUTPUT );
                    gpioSetPWMfrequency( pGpioPwm->getPin(), 
                                            pGpioPwm->getFrequency() );
                    gpioSetPWMrange( pGpioPwm->getPin(), 
                                        pGpioPwm->getRange() ) ;
                    gpioPWM( pGpioPwm->getPin(), pGpioPwm->getDutyCycle() ) ;     
#endif                
                }

            }

        }

    }

    // Init. clock pins
    {
        std::list<CGpioClock *>::const_iterator it;
        for ( it = pObj->m_gpioClockPinList.begin(); 
                it != pObj->m_gpioClockPinList.end(); 
                ++it ) {

            CGpioClock *pGpioClock = *it;
        
            if ( NULL != pGpioClock ) {
#ifdef USE_PIGPIOD
                hardware_clock( pgpiod_session_id, 
                                    pGpioClock->getPin(), 
                                    pGpioClock->getFrequency() );
#else            
                gpioHardwareClock( pGpioClock->getPin(), 
                                    pGpioClock->getFrequency() );
#endif            
            }
        
        }
    }

    // Monitor
    /*
    uint32_t monitor_bits = 0;
    {
        std::list<CGpioMonitor *>::const_iterator it;
        for ( it = pObj->m_monitorPinList.begin(); 
                it != pObj->m_monitorPinList.end(); 
                ++it ) {

            CGpioMonitor *pGpioMonitor = *it;
            if ( pGpioMonitor->getPin() <= 31 ) {
                monitor_bits |= ( 1 << pGpioMonitor->getPin() );
            }

        }
    }
    */

#ifdef USE_PIGPIOD
/*
    int pipeNotify;
    if ( PI_NO_HANDLE == ( pipeNotify = notify_open( pgpiod_session_id ) ) ) {
        syslog( LOG_ERR,
				    "%s %s ",
                    (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID,
				    (const char *)"Unable to get notify pipe."  );
#ifdef USE_PIGPIOD
        pigpio_stop( pgpiod_session_id );
#else    
        gpioTerminate();
#endif
        return NULL;
    }

    int fdPipeNotify;
    std::string pipePath = vscp_string_format("/dev/pigpio%d", pipeNotify );
    
    if ( -1 == ( fdPipeNotify = open( pipePath.c_str(), O_NONBLOCK | O_RDONLY ) ) ) {
        syslog( LOG_ERR,
				    "%s %s [%s]",
                    (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID,
				    (const char *)"Unable to open notify pipe.", 
                    pipePath.c_str() );
#ifdef USE_PIGPIOD
        pigpio_stop( pgpiod_session_id );
#else    
        gpioTerminate();
#endif                    
        return NULL;
    }

    notify_begin( pgpiod_session_id, pipeNotify, 1 );
*/
    
    for ( unsigned pin=0; pin<32; pin++ ) {
        if ( pObj->m_monitor[ pin ].bEnable ) {
            callback_ex( pgpiod_session_id, 
                            pin, 
                            pObj->m_monitor[ pin ].edge, 
                            monitor_callback, 
                            (void *)pObj );
        }
    }

#endif    

    /*reportStruct *preport = new reportStruct;
    preport->id = 0;
    preport->pObj = pObj;*/
    //gpioSetTimerFuncEx( 0, 1000, report_callback0, pObj );

    while ( !pObj->m_bQuit ) {
        
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 100000000;    // 100 ms
      
        if ( ETIMEDOUT != sem_timedwait( &pObj->m_semaphore_SendQueue, &ts ) ) {
           
            if ( pObj->m_bQuit ) continue;

            // Check if there is event(s) to handle
            if ( pObj->m_sendList.size() ) {

#ifdef RPIGPIO_DEBUG
                syslog( LOG_ERR, 
                            "%s Incoming event.",
                            (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID  );
#endif
                // Yes there are an incoming event
                pthread_mutex_lock( &pObj->m_mutex_SendQueue );
                vscpEvent *pEvent = pObj->m_sendList.front();
                pObj->m_sendList.pop_front();
                pthread_mutex_unlock( &pObj->m_mutex_SendQueue );

                if ( NULL == pEvent ) continue;

#ifdef RPIGPIO_DEBUG
                syslog( LOG_ERR, 
                            "%s Event removed from queue.",
                            (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID  );
#endif

                // Just to make sure
                if ( 0 == pEvent->sizeData ) {
                    pEvent->pdata = NULL;
                }

#ifdef RPIGPIO_DEBUG
                syslog( LOG_ERR, 
                            "%s Valid event.",
                            (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID  );
#endif                

                // Feed through matrix - major filter
                if ( vscp_doLevel2Filter( pEvent, &pObj->m_vscpfilter ) ) {

#ifdef RPIGPIO_DEBUG
                    syslog( LOG_ERR, 
                            "%s After major filter.",
                            (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID  );
#endif

                    std::list<CLocalDM *>::const_iterator it;
                    for ( it = pObj->m_LocalDMList.begin(); 
                            it != pObj->m_LocalDMList.end(); 
                            ++it ) {

                        CLocalDM *pDM = (CLocalDM *)*it;
                        if ( vscp_doLevel2Filter( pEvent, &pDM->getFilter() ) ) {

#ifdef RPIGPIO_DEBUG
                            syslog( LOG_ERR, 
                                        "%s After DM row filter.",
                                        (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID  );
#endif

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

#ifdef RPIGPIO_DEBUG
                            syslog( LOG_ERR, 
                                        "%s Ready for action.",
                                        (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID  );
#endif                            

                            // OK - Do action
                            switch ( pDM->getAction() ) {

                                case ACTION_RPIGPIO_ON:
                                    {
#ifdef RPIGPIO_DEBUG                                        
                                        syslog( LOG_ERR, 
                                                    "%s Action = ACTION_RPIGPIO_ON.",
                                                    (const char *)VSCP_RPIGPIO_SYSLOG_DRIVER_ID  );
#endif
                                        uint8_t pin = (uint8_t)pDM->getArg( 0 );
                                        if ( pin <= 53 ) { 

#ifdef USE_PIGPIOD
                                            gpio_write( pgpiod_session_id, pin, 1 );
#else
                                            gpioWrite( pin, 1 );
#endif                                            

                                            vscpEventEx ex;
                                            memset( &ex, 0, sizeof( ex ) );
                                            ex.vscp_class = VSCP_CLASS1_INFORMATION;
                                            ex.sizeData = 3;
                                            ex.data[0] = pin;
                                            ex.data[1] = pObj->getZone();
                                            ex.data[2] = pObj->getSubzone();

#ifdef USE_PIGPIOD
                                            if ( gpio_read( pgpiod_session_id, pin ) ) {
#else
                                            if ( gpioRead( pin ) ) {
#endif                                                
                                                ex.vscp_type = VSCP_TYPE_INFORMATION_ON;
                                            }
                                            else {
                                                ex.vscp_type = VSCP_TYPE_INFORMATION_OFF;
                                            }

                                            sendEvent( pObj, ex );

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

#ifdef USE_PIGPIOD
                                            gpio_write( pgpiod_session_id, pin, 0 );
#else
                                            gpioWrite( pin, 0 );
#endif

                                            vscpEventEx ex;
                                            memset( &ex, 0, sizeof( vscpEventEx ) );
                                            ex.vscp_class = VSCP_CLASS1_INFORMATION;
                                            ex.sizeData = 3;
                                            ex.data[0] = pin;
                                            ex.data[1] = pObj->getZone();
                                            ex.data[2] = pObj->getSubzone();

#ifdef USE_PIGPIOD
                                            if ( gpio_read( pgpiod_session_id, pin ) ) {
#else
                                            if ( gpioRead( pin ) ) {
#endif
                                                ex.vscp_type = VSCP_TYPE_INFORMATION_ON;
                                            }
                                            else {
                                                ex.vscp_type = VSCP_TYPE_INFORMATION_OFF;
                                            }

                                            sendEvent( pObj, ex );

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
                                        
                                        vscpEventEx ex;
                                        memset( &ex, 0, sizeof( vscpEventEx ) );
                                        ex.vscp_class = VSCP_CLASS1_INFORMATION;
                                        ex.vscp_type = VSCP_TYPE_INFORMATION_BIG_LEVEL_CHANGED;
                                        ex.sizeData = 3;
                                        ex.data[0] = pin;
                                        ex.data[1] = pObj->getZone();
                                        ex.data[2] = pObj->getSubzone();

                                        int duty_cycle = gpioGetPWMdutycycle( pin );

                                        if ( 0xff >= duty_cycle ) {
                                            ex.sizeData = 4;
                                            ex.data[3] = duty_cycle;
                                        }
                                        else if ( 0xffff >= duty_cycle ) {
                                            ex.sizeData = 5;
                                            ex.data[3] = 0xff & ( duty_cycle >> 8 );
                                            ex.data[4] = 0xff & duty_cycle;
                                        }
                                        else if ( 0xffffff >= duty_cycle ) {
                                            ex.sizeData = 6;
                                            ex.data[3] = 0xff & ( duty_cycle >> 16 );
                                            ex.data[4] = 0xff & ( duty_cycle >> 8 );
                                            ex.data[5] = 0xff & duty_cycle;
                                        }
                                        else if ( 0xffffffff >= duty_cycle ) {
                                            ex.sizeData = 7;
                                            ex.data[3] = 0xff & ( duty_cycle >> 24 );
                                            ex.data[4] = 0xff & ( duty_cycle >> 16 );
                                            ex.data[5] = 0xff & ( duty_cycle >> 8 );
                                            ex.data[6] = 0xff & duty_cycle;
                                        }
                                        /*else if ( 0xffffffffff >= duty_cycle ) {
                                            ex.sizeData = 8;
                                            ex.data[3] = 0xff & ( duty_cycle >> 32 );
                                            ex.data[4] = 0xff & ( duty_cycle >> 24 );
                                            ex.data[5] = 0xff & ( duty_cycle >> 16 );
                                            ex.data[6] = 0xff & ( duty_cycle >> 8 );
                                            ex.data[7] = 0xff & duty_cycle;
                                        }*/

                                        sendEvent( pObj, ex );

                                    }
                                    break;    

                                case ACTION_RPIGPIO_STATUS:
                                    {
                                        vscpEventEx ex;

                                        uint8_t pin = (uint8_t)pDM->getArg( 0 );

                                        memset( &ex, 0, sizeof( vscpEventEx) );
                                        uint8_t index = pin;
                                        uint8_t zone = 0;
                                        uint8_t subzone = 0;

                                        
                                        
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

                                        // zone
                                        zone = (uint8_t)pDM->getArg( 1 );

                                        // subzone
                                        subzone = (uint8_t)pDM->getArg( 2 );

                                        ex.vscp_class = VSCP_CLASS1_INFORMATION;
                                        ex.sizeData = 3;
                                        ex.data[0] = pin;
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

                                        vscpEventEx ex;
                                        memset( &ex, 0, sizeof( vscpEventEx ) );
                                        ex.vscp_class = VSCP_CLASS1_INFORMATION;
                                        ex.vscp_type = VSCP_TYPE_INFORMATION_BIG_LEVEL_CHANGED;
                                        ex.sizeData = 5;
                                        ex.data[0] = pin;
                                        ex.data[1] = pObj->getZone();
                                        ex.data[2] = pObj->getSubzone();

                                        // 0 (off), 500 (most anti-clockwise) to 2500 (most clockwise). 
                                        int pulse_width = gpioGetServoPulsewidth( pin );

                                        if ( 0xff >= pulse_width ) {
                                            ex.sizeData = 4;
                                            ex.data[3] = pulse_width;
                                        }
                                        else if ( 0xffff >= pulse_width ) {
                                            ex.sizeData = 5;
                                            ex.data[3] = 0xff & ( pulse_width >> 8 );
                                            ex.data[4] = 0xff & pulse_width;
                                        }

                                        sendEvent( pObj, ex );

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

        } // if *not* timeout
#ifdef USE_PIGPIOD        
        /*else {
            gpioReport_t report;
            ssize_t nread = read( fdPipeNotify, &report, sizeof( report ) );
            if ( nread == sizeof( report ) ) {
                if ( report.flags | PI_NTFY_FLAGS_WDOG ) {
                    uint8_t pin = PI_NTFY_FLAGS_EVENT & 0x1f;
                    if ( pObj->m_monitor[pin].bEnable ) {
                        pObj->m_monitor[pin].eventWatchdog.data[0] = pin;
                        sendEvent( pObj, pObj->m_monitor[pin].eventWatchdog );
                    }
                }
                else if ( report.flags | PI_NTFY_FLAGS_ALIVE ) {
                    ; // We do nothing
                }
                else if ( report.flags | PI_NTFY_FLAGS_EVENT ) {

                    // Get the pin that generated the even)
                    uint8_t pin = PI_NTFY_FLAGS_EVENT & 0x1f;

                    switch ( PI_NTFY_FLAGS_EVENT & 0x1f ) {
                    
                        case RISING_EDGE:
                            if ( pObj->m_monitor[pin].bEnable ) {
                                pObj->m_monitor[pin].eventRising.data[0] = pin;
                                sendEvent( pObj, pObj->m_monitor[pin].eventRising );
                            }
                            break;
                    
                        case FALLING_EDGE:
                            if ( pObj->m_monitor[pin].bEnable ) {
                                pObj->m_monitor[pin].eventFalling.data[0] = pin;
                                sendEvent( pObj, pObj->m_monitor[pin].eventFalling );
                            }
                            break;  
                    
                        default:
                        case EITHER_EDGE:
                            break;  

                    }

                }
            }
        }*/
#endif     
        timeval curTime;
        gettimeofday( &curTime, NULL );
        uint32_t now = 1000 * curTime.tv_sec + ( curTime.tv_usec / 1000 );

        for ( int i=0; i<10; i++ ) {
            if ( ( pObj->m_reporters[ i ].bEnable ) &&
                 ( (uint32_t)( now - pObj->m_reporters[ i ].last ) > pObj->m_reporters[ i ].period ) ) {
                pObj->m_reporters[ i ].last = now;

#ifdef USE_PIGPIOD
                int pin = gpio_read( pgpiod_session_id, pObj->m_reporters[ i ].pin );
#else
                int pin = gpioRead( pObj->m_reporters[ i ].pin );
#endif                
                if ( pin ) {
                    pObj->m_reporters[ i ].eventHigh.data[0] = pin;
                    sendEvent( pObj, pObj->m_reporters[ i ].eventHigh );
                }
                else {
                    pObj->m_reporters[ i ].eventHigh.data[0] = pin;
                    sendEvent( pObj, pObj->m_reporters[ i ].eventLow );
                }
            }
        }        

    } // while

    // Quit gpio library functionality
#ifdef USE_PIGPIOD
    // Close pipe handling
    /*notify_close( pgpiod_session_id, pipeNotify );
    close( fdPipeNotify );*/
    // Close lib
    pigpio_stop( pgpiod_session_id );
#else    
    gpioTerminate();
#endif    

    return NULL;
}

//////////////////////////////////////////////////////////////////////
// Monitor callback
//
//  typedef void (*CBFuncEx_t)
//      (int handle, unsigned user_gpio, unsigned level, uint32_t tick, void * user);
//

static void monitor_callback( int handle, 
                                    unsigned gpio, 
                                    unsigned edge, 
                                    uint32_t tick, 
                                    void *userdata ) 
{
    CRpiGpio *pObj = (CRpiGpio *)userdata;
    if ( NULL == pObj ) return;

    // Get the pin that generated the even
    uint8_t pin = gpio;
    //printf("Callback %d %d\n", gpio, edge );
    switch ( edge ) {
    
        case 0: // FALLING_EDGE
            if ( pObj->m_monitor[pin].bEnable ) {
                pObj->m_monitor[pin].eventFalling.data[0] = pin;
                sendEvent( pObj, pObj->m_monitor[pin].eventFalling );
                //printf("Falling %d %d\n", pin, edge );
            }
            break;

        case 1: // RISING_EDGE
            if ( pObj->m_monitor[pin].bEnable ) {
                pObj->m_monitor[pin].eventRising.data[0] = pin;
                sendEvent( pObj, pObj->m_monitor[pin].eventRising );
                //printf("Rising %d %d\n", gpio, edge );
            }
            break;
    
        default:
        case 2: // Watchdog
            if ( pObj->m_monitor[pin].bEnable ) {
                pObj->m_monitor[pin].eventWatchdog.data[0] = pin;
                sendEvent( pObj, pObj->m_monitor[pin].eventWatchdog );
                printf("Watchdog %d %d\n", gpio, edge );
            }
            break;

    }

 }

//////////////////////////////////////////////////////////////////////
// Report callbacks (called at timed intervals)
// 

static void report_callback0( void *userdata  ) 
{
    CRpiGpio *pObj = (CRpiGpio *)userdata;
    if ( NULL == pObj ) return;

 }

static void report_callback1( void *userdata  ) 
{
    CRpiGpio *pObj = (CRpiGpio *)userdata;
    if ( NULL == pObj ) return;

 }

 static void report_callback2( void *userdata  ) 
{
    CRpiGpio *pObj = (CRpiGpio *)userdata;
    if ( NULL == pObj ) return;

 }

 static void report_callback3( void *userdata  ) 
{
    CRpiGpio *pObj = (CRpiGpio *)userdata;
    if ( NULL == pObj ) return;

 }

 static void report_callback4( void *userdata  ) 
{
    CRpiGpio *pObj = (CRpiGpio *)userdata;
    if ( NULL == pObj ) return;

 }

 static void report_callback5( void *userdata  ) 
{
    CRpiGpio *pObj = (CRpiGpio *)userdata;
    if ( NULL == pObj ) return;

 }

 static void report_callback6( void *userdata  ) 
{
    CRpiGpio *pObj = (CRpiGpio *)userdata;
    if ( NULL == pObj ) return;

 }

 static void report_callback7( void *userdata  ) 
{
    CRpiGpio *pObj = (CRpiGpio *)userdata;
    if ( NULL == pObj ) return;

 }

 static void report_callback8( void *userdata  ) 
{
    CRpiGpio *pObj = (CRpiGpio *)userdata;
    if ( NULL == pObj ) return;

 }

 static void report_callback9( void *userdata  ) 
{
    CRpiGpio *pObj = (CRpiGpio *)userdata;
    if ( NULL == pObj ) return;

 }