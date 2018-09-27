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

#include "../../../../common/canal.h"
#include "../../../../common/vscp.h"
#include "../../../../common/canal_macro.h"
#include "../../../../../common/dllist.h"
#include "../../../../common/vscpremotetcpif.h"
#include "../../../../common/guid.h"

#include <list>
#include <string>

using namespace std;

#define VSCP_RPIGPIO_SYSLOG_DRIVER_ID       "VSCP rpigpio driver:"
#define VSCP_LEVEL2_DLL_RPIGPIO_OBJ_MUTEX   "___VSCP__DLL_L2RPIGPIO_OBJ_MUTEX____"
#define VSCP_RPIGPIO_LIST_MAX_MSG		    2048
  
#define VSCP_MODE_PWM_SOFT   0
#define VSCP_MODE_PWM_HARD   1  // PWM in hardware

// Forward declarations
class RpiGpioWorkerTread;
class VscpRemoteTcpIf;
class CRpiGpio;


struct reportStruct {
    int id;
    CRpiGpio * pObj;
};

// Actions
#define RPIGPIO_ACTION_NOOP     0

// ----------------------------------------------------------------------------

struct reportItem {
    uint8_t m_id;
    uint8_t m_pin;
    uint32_t m_period;
    vscpEventEx m_reportEventLow;
    vscpEventEx m_reportEventHigh;
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

    bool setNoiceFilter( uint32_t steady, uint32_t active ) 
        { m_noice_filter_steady = steady; m_noice_filter_active = active; return true; };
    bool setNoiceFilterSteady( uint32_t steady ) 
        { m_noice_filter_steady = steady; return true; };
    bool setNoiceFilterActive( uint32_t active ) 
        { m_noice_filter_active = active; return true; };        
    uint32_t getNoiceFilterSteady( void ) { return m_noice_filter_steady; };
    uint32_t getNoiceFilterActive( void ) { return m_noice_filter_active; };

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

// Noice filter. 
// Level changes on the GPIO are ignored until a level which has been 
// stable for microseconds set here is detected. Level changes on the 
// GPIO are then reported for active microseconds after which the 
// process repeats.0 = disabled.
uint32_t m_noice_filter_steady;
uint32_t m_noice_filter_active;

// Glitch filter. 
// Level changes on the GPIO are not reported unless the level has been 
// stable for at least microseconds set here. 0 = disabled.
uint32_t m_glitch_filter;



// ---

// True if monitoring is enabled
bool m_bEnable_Report;

// Time in milliseconds between reports
long m_report_period;

// Event to send when triggered
vscpEventEx m_reportEventHigh;

// Event to send when triggered
vscpEventEx m_reportEventLow;

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
    bool setType( const wxString& strtype );
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

// The local decision matrix for the driver
class CLocalDM{

public:

    /// Constructor
    CLocalDM();

    /// Destructor
    virtual ~CLocalDM();

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
    uint8_t getAction( void );

    bool setActionParameter( const std::string& param );
    std::string& getActionParameter( void );

private:

    // Filter
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
	std::string
     m_strActionParam;

};

// ----------------------------------------------------------------------------

// Define one GPIO monitor
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
                            const vscpEventEx& eventRising );

    vscpEventEx& getMonitorEventFalling( void ) { return m_eventFalling; };
    vscpEventEx& getMonitorEventRising( void ) { return m_eventFalling; };

private:

    // The GPIO port
    uint8_t m_pin;

    // 0-3 default = INT_EDGE_SETUP
    uint8_t m_edge;

    // Event to send when triggered on falling edge or both edges
    vscpEventEx m_eventFalling;

    // Event to send when triggered on rising or edge
    vscpEventEx m_eventRising;

};

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

public:

    /// Run flag
    bool m_bQuit;
	
    /// Server supplied username
    wxString m_username;

    /// Server supplied password
    wxString m_password;

    /// server supplied prefix
    wxString m_prefix;

    /// server supplied host
    wxString m_host;

    /// Server supplied port
    short m_port;
    
    /// XML configuration
    wxString m_setupXml;
    
    /// Filter
    vscpEventFilter m_vscpfilter;
	
	/// Get GUID for this interface.
	cguid m_ifguid;

    /// Pointer to worker threads
    RpiGpioWorkerTread *m_pthreadWorker;
    
    // VSCP server interface
    VscpRemoteTcpIf m_srv;

    // Event lists	
	std::list<vscpEvent *> m_sendList;
	std::list<vscpEvent *> m_receiveList;
	
    // Event object to indicate that there is an event in the output queue
    wxSemaphore m_semSendQueue;			
	wxSemaphore m_semReceiveQueue;		
	
	// Mutex to protect the output queue
	wxMutex m_mutexSendQueue;		
	wxMutex m_mutexReceiveQueue;

    // Can be set to 1, 2, 4, 5, 8, 10 always PCM
    uint8_t m_sample_rate;

    // Primary DMA channel 0-14
    uint8_t m_primary_dma_channel;

    // Scondary DMA channel 0-14
    uint8_t m_secondary_dma_channel;

    // Lists for pin definitions
    std::list<CGpioInput *> m_inputPinList;
    std::list<CGpioMonitor *> m_monitorPinList;
    std::list<CGpioOutput *> m_outputPinList;
    std::list<CGpioPwm *> m_pwmPinList;
    std::list<CGpioClock *> m_gpioClockPinList; // Will hold max one entry

    // Max ten report items
    reportItem m_reporters[10];

    // Decision matrix
    std::list<CLocalDM *> m_LocalDMList; // Will hold max one entry
};

///////////////////////////////////////////////////////////////////////////////
//				                Worker Treads
///////////////////////////////////////////////////////////////////////////////


class RpiGpioWorkerTread : public wxThread {
public:

    /// Constructor
    RpiGpioWorkerTread();

    /// Destructor
    ~RpiGpioWorkerTread();

    /*!
        Thread code entry point
     */
    virtual void *Entry();

    /*! 
        called when the thread exits - whether it terminates normally or is
        stopped with Delete() (but not when it is Kill()ed!)
     */
    virtual void OnExit();

    /// VSCP server interface
    VscpRemoteTcpIf m_srv;

    /// Sensor object
    CRpiGpio *m_pObj;
    
};



#endif // !defined(____RPIGPIO__INCLUDED_)
