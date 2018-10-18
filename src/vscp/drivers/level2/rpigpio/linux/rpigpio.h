// rpigpio.h: interface for the gpio driver.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
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


#if !defined(____RPIGPIO__INCLUDED_)
#define ____RPIGPIO__INCLUDED_

#include <stdio.h>
#include <string.h>
#include <time.h>

#define _POSIX
#include <unistd.h>
#include <pthread.h>
#include <syslog.h>

#include <list>
#include <string>

#include "../../../../common/canal.h"
#include "../../../../common/vscp.h"
#include "../../../../common/canal_macro.h"
#include "../../../../../common/dllist.h"
#include "../../../../common/vscpremotetcpif.h"
#include "../../../../common/guid.h"

#define VSCP_RPIGPIO_SYSLOG_DRIVER_ID       "VSCP rpigpio driver:"
#define VSCP_LEVEL2_DLL_RPIGPIO_OBJ_MUTEX   "___VSCP__DLL_L2RPIGPIO_OBJ_MUTEX____"
#define VSCP_RPIGPIO_LIST_MAX_MSG		    2048


//#define USE_PIGPIOD           // if define the pigpio daemon i/f is used
                                // Defined in MAkefile normally when if2 version
                                // is built.

#define VSCP_MODE_PWM_SOFT   0
#define VSCP_MODE_PWM_HARD   1  // PWM in hardware

// Forward declarations
class VscpRemoteTcpIf;
class CRpiGpio;


// Actions
#define ACTION_RPIGPIO_NOOP             0x00    // No operation
#define ACTION_RPIGPIO_ON               0x01    // Turn on output
#define ACTION_RPIGPIO_OFF              0x02    // Turn off output
#define ACTION_RPIGPIO_PWM              0x03    // PWM on pwm channel (duty)
#define ACTION_RPIGPIO_FREQUENCY        0x04    // Frequency on pin
#define ACTION_RPIGPIO_STATUS           0x05    // Report status for pin
#define ACTION_RPIGPIO_SERVO            0x06    // Servo

// Future

#define ACTION_RPIGPIO_WAVEFORM         0x07    // Waveform
#define ACTION_RPIGPIO_SHIFTOUT         0x08    // Shift out
#define ACTION_RPIGPIO_SHIFTOUT_EVENT   0x09    // Shift out event data on output pin
#define ACTION_RPIGPIO_SHIFTIN          0x0A    // Shift out
#define ACTION_RPIGPIO_SAMPLE           0x0B    // Sample
#define ACTION_RPIGPIO_SERIAL_OUT       0x0C    // Serial out 
#define ACTION_RPIGPIO_SERIAL_IN        0x0D    // Serial in

// ----------------------------------------------------------------------------

struct reportItem {
    bool bEnable;
    uint8_t pin;
    uint32_t period;
    uint32_t last;
    vscpEventEx eventLow;
    vscpEventEx eventHigh;
};

// ----------------------------------------------------------------------------

struct monitorItem {
    bool bEnable;                   
    uint8_t edge;                 // both/falling/rising  EITHER_EDGE/FALLING_EDGE/RISING_EDGE   
    vscpEventEx eventFalling;     // Event to send when triggered on falling edge or both edges
    vscpEventEx eventRising;      // Event to send when triggered on rising or edge
    vscpEventEx eventWatchdog;    // Event to send when triggered on watchdog
};

// ----------------------------------------------------------------------------

// Define one GPIO input
class CGpioInput{

public:

    /// Constructor
    CGpioInput();

    /// Destructor
    virtual ~CGpioInput();

    // Getters & Setters
    bool setPin( uint8_t pin );
    uint8_t getPin(void);

    bool setPullUp( const std::string& strPullUp );
    uint8_t getPullUp( void );

    bool setWatchdog( uint32_t watchdog ) { m_watchdog = watchdog; return true; };
    uint32_t getWatchdog( void ) { return m_watchdog; };

    bool setNoiseFilter( uint32_t steady, uint32_t active ) 
        { m_noise_filter_steady = steady; m_noise_filter_active = active; return true; };
    bool setNoiseFilterSteady( uint32_t steady ) 
        { m_noise_filter_steady = steady; return true; };
    bool setNoiseFilterActive( uint32_t active ) 
        { m_noise_filter_active = active; return true; };        
    uint32_t getNoiseFilterSteady( void ) { return m_noise_filter_steady; };
    uint32_t getNoiseFilterActive( void ) { return m_noise_filter_active; };

    bool setGlitchFilter( uint32_t glitch ) { m_glitch_filter = glitch; return true; };
    uint32_t getGlitchFilter( void ) { return m_glitch_filter; };

    

    bool setReport( bool bEnable, 
                        long period, 
                        vscpEventEx& eventLow,
                        vscpEventEx& eventHigh );
    bool isReportEnabled( void );
    long getReportPeriod( void );
    vscpEventEx& getReportEventLow( void );
    vscpEventEx& getReportEventHigh( void );

private:

// The GPIO port
uint8_t m_pin;

// The pullup
uint8_t m_pullup;

// Sample watchdog value. 0 = disabled.
uint32_t m_watchdog;

// Noise filter. 
// Level changes on the GPIO are ignored until a level which has been 
// stable for microseconds set here is detected. Level changes on the 
// GPIO are then reported for active microseconds after which the 
// process repeats.0 = disabled.
uint32_t m_noise_filter_steady;
uint32_t m_noise_filter_active;

// Glitch filter. 
// Level changes on the GPIO are not reported unless the level has been 
// stable for at least microseconds set here. 0 = disabled.
uint32_t m_glitch_filter;


};

// ----------------------------------------------------------------------------

// Define one GPIO output
class CGpioOutput{

public:

    /// Constructor
    CGpioOutput();

    /// Destructor
    virtual ~CGpioOutput();

    bool setPin( uint8_t pin );
    uint8_t getPin( void );

    void setInitialState( int state );
    int getInitialState( void );

private:

// The GPIO port
uint8_t m_pin;

// Initial state (-1 (default) don't set)
int m_state;

};

// ----------------------------------------------------------------------------

// Define one PWM output (hardware or software)
class CGpioPwm {

public:

    /// Constructor
    CGpioPwm();

    /// Destructor
    virtual ~CGpioPwm();

    bool setPin( uint8_t pin );
    uint8_t getPin( void );

    bool setType( uint8_t type );
    bool setType( const std::string& strtype );
    uint8_t getType( void );

    bool enableHardwarePwm() { m_bHardware = true; };
    bool disableHardwarePwm() { m_bHardware = false; };
    bool isHardwarePwm( void ) { return m_bHardware; };

    bool setRange( int range );
    int getRange( void );

    bool setFrequency( int frequency );
    int getFrequency( void );

    bool setDutyCycle( int dutycycle );
    int getDutyCycle( void );

private:

// The GPIO port
uint8_t m_pin;

// True if hardware PWM channel should be used.
// If so the GPIO must be one of the following.
//
// 12  PWM channel 0  All models but A and B
// 13  PWM channel 1  All models but A and B
// 18  PWM channel 0  All models
// 19  PWM channel 1  All models but A and B
//
// 40  PWM channel 0  Compute module only
// 41  PWM channel 1  Compute module only
// 45  PWM channel 1  Compute module only
// 52  PWM channel 0  Compute module only
// 53  PWM channel 1  Compute module only
bool m_bHardware;

// Type of PWM (hard (PWM_OUTPUT) or soft (SOFT_PWM_OUTPUT) )
uint8_t m_type;

// PWM range
int m_range;

// PWM frequency
int m_frequency;

// PWM duty cycle
int m_dutycycle;

};

// ----------------------------------------------------------------------------

// Define one GPIO clock pin
class CGpioClock {

public:

    /// Constructor
    CGpioClock();

    /// Destructor
    virtual ~CGpioClock();

    bool setPin( uint8_t pin );
    uint8_t getPin( void );

    bool setFrequency( int frequency );
    int getFrequency( void );

private:

// The GPIO port
uint8_t m_pin;

// Clock frequency
int m_frequency;

};

// ----------------------------------------------------------------------------

#define MAX_DM_ARGS     5

// The local decision matrix for the driver
class CLocalDM{

public:

    /// Constructor
    CLocalDM();

    /// Destructor
    virtual ~CLocalDM();

    void enableRow( void ) { m_bEnable = true; };
    void disableRow( void ) { m_bEnable = false; };
    bool isRowEnabled( void ) { return m_bEnable; };

    bool setFilter( vscpEventFilter& filter );
    vscpEventFilter& getFilter( void );

    bool setIndex( uint8_t index );
    uint8_t getIndex( void );
    bool isIndexCheckEnabled( void );

    bool setZone( uint8_t zone );
    uint8_t getZone( void );
    bool isZoneCheckEnabled( void );

    bool setSubZone( uint8_t subzone );
    uint8_t getSubZone( void );
    bool isSubZoneCheckEnabled( void );

    bool setAction( uint8_t action );
    bool setAction( std::string& str );
    uint8_t getAction( void );

    bool setActionParameter( const std::string& param );
    std::string& getActionParameter( void );

    void setArg( uint8_t idx, uint32_t val ) { if ( idx < MAX_DM_ARGS ) m_args[idx] = val; };
    uint32_t getArg( uint8_t idx ) { if ( idx < MAX_DM_ARGS ) return m_args[idx]; else return 0; };

private:

    // Enable row
    bool m_bEnable;

    // Filter - for DM row trigger
    vscpEventFilter m_vscpfilter;

    // Index compare
    bool m_bCompareIndex;
	uint8_t m_index;

    // Zone compare
    bool m_bCompareZone;
	uint8_t m_zone;

    // SubZone compare
    bool m_bCompareSubZone;
	uint8_t m_subzone;

    // Action to execute on match
	uint8_t m_action;

    // Parameter for action
	std::string m_strActionParam;

    // Parsed action parameter arg. values
    uint32_t m_args[MAX_DM_ARGS];

};

// ----------------------------------------------------------------------------

// Define one GPIO monitor
/*
class CGpioMonitor{

public:

    /// Constructor
    CGpioMonitor();

    /// Destructor
    virtual ~CGpioMonitor();

    void setPin( uint8_t pin ) { m_pin = pin; };
    uint8_t getPin( void ) { return m_pin; };

    void setEdge( uint8_t edge ) { m_edge = edge; };
    bool setEdge( const std::string& strEdge );
    uint8_t getEdge( void ) { return m_edge; };

    void setMonitorEvents( const vscpEventEx& eventFalling,
                            const vscpEventEx& eventRising,
                            const vscpEventEx& eventWatchdog );

    vscpEventEx& getMonitorEventFalling( void ) { return m_eventFalling; };
    vscpEventEx& getMonitorEventRising( void ) { return m_eventRising; };
    vscpEventEx& getMonitorEventWatchdog( void ) { return m_eventWatchdog; };

private:

    // The GPIO port
    uint8_t m_pin;

    // 0-3 default = INT_EDGE_SETUP
    uint8_t m_edge;

    // Event to send when triggered on falling edge or both edges
    vscpEventEx m_eventFalling;

    // Event to send when triggered on rising or edge
    vscpEventEx m_eventRising;

    // Event to send when triggered on watchdog
    vscpEventEx m_eventWatchdog;

};
*/
// ----------------------------------------------------------------------------

class CRpiGpio {

public:

    /// Constructor
    CRpiGpio();

    /// Destructor
    virtual ~CRpiGpio();

    /*! 
        Open
        @return True on success.
     */
    bool open(const char *pUsername,
                const char *pPassword,
                const char *pHost,
                short port,
                const char *pPrefix,
                const char *pConfig);

    /*!
        Flush and close the log file
     */
    void close(void);

	/*!
		Add event to send queue 
	 */
	bool addEvent2SendQueue(const vscpEvent *pEvent);

    // Getter and setter for samplerate
    void setSampleRate( uint8_t rate ) { m_sample_rate = rate; };
    uint8_t getSampleRate( void ) { return m_sample_rate; };

    // Getter and setter for primary DMA channel
    void setPrimaryDmaChannel( uint8_t ch ) { m_primary_dma_channel = ch; };
    uint8_t getPrimaryDmaChannel( void ) { return m_primary_dma_channel; };

    // Getter and setter for secondary DMA channel
    void setSecondaryDmaChannel( uint8_t ch ) { m_secondary_dma_channel = ch; };
    uint8_t getSecondaryDmaChannel( void ) { return m_secondary_dma_channel; };

#ifdef USE_PIGPIOD
    void setPiGpiodHost( const std::string& str ) { m_pigpiod_host = str; };
    std::string getPiGpiodHost( void ) { return m_pigpiod_host; };

    void setPiGpiodPort( const std::string& str ) { m_pigpiod_port = str; };
    std::string getPiGpiodPort( void ) { return m_pigpiod_port; };
#endif    

    // Getter and setter for module index
    void setIndex( uint8_t index ) { m_index = index; };
    uint8_t getIndex( void ) { return m_index; };

    // Getter and setter for module zone
    void setZone( uint8_t zone ) { m_zone = zone; };
    uint8_t getZone( void ) { return m_zone; };

    // Getter and setter for module subzone
    void setSubzone( uint8_t subzone ) { m_subzone = subzone; };
    uint8_t getSubzone( void ) { return m_subzone; };

public:

    /// Run flag
    bool m_bQuit;
	
    /// Server supplied username
    std::string m_username;

    /// Server supplied password
    std::string m_password;

    /// server supplied prefix
    std::string m_prefix;

    /// server supplied host
    std::string m_host;

    /// Server supplied port
    short m_port;
    
    /// XML configuration
    std::string m_setupXml;

#ifdef USE_PIGPIOD
    /// host where pigpid is
    std::string m_pigpiod_host; 

    /// port on host where pigpid is
    std::string m_pigpiod_port;         
#endif     
    
    /// Filter for all trafic to this module
    vscpEventFilter m_vscpfilter;
	
	/// GUID for this module (set to all zero to use interface GUID).
	cguid m_ifguid;

    // Index used for outgoing events from this module
    uint8_t m_index;

    // Zone used for outgoing events from this module
    uint8_t m_zone;

    // Subzone used for outgoing events from this module
    uint8_t m_subzone;

    /// Pointer to worker threads
    pthread_t m_pthreadWorker;
    
    // VSCP server interface
    VscpRemoteTcpIf m_srv;

    // Event lists	
	std::list<vscpEvent *> m_sendList;
	std::list<vscpEvent *> m_receiveList;
	
    // Event object to indicate that there is an event in the output queue	
    sem_t m_semaphore_SendQueue;			
	sem_t m_semaphore_ReceiveQueue;	
	
	// Mutex to protect the output queue
    pthread_mutex_t m_mutex_SendQueue;
    pthread_mutex_t m_mutex_ReceiveQueue;

    // Can be set to 1, 2, 4, 5, 8, 10 always PCM
    uint8_t m_sample_rate;

    // Primary DMA channel 0-14
    uint8_t m_primary_dma_channel;

    // Scondary DMA channel 0-14
    uint8_t m_secondary_dma_channel;

    // Lists for pin definitions
    std::list<CGpioInput *> m_inputPinList;
    //std::list<CGpioMonitor *> m_monitorPinList;
    std::list<CGpioOutput *> m_outputPinList;
    std::list<CGpioPwm *> m_pwmPinList;
    std::list<CGpioClock *> m_gpioClockPinList; // Will hold max one entry

    // Max ten report items
    reportItem m_reporters[10];

    // Monitors each representing a pin from 0-31
    monitorItem m_monitor[32];

    // Decision matrix
    std::list<CLocalDM *> m_LocalDMList; 
};


#endif // !defined(____RPIGPIO__INCLUDED_)
