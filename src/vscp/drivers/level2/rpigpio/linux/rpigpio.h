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

#include <wx/file.h>
#include <wx/wfstream.h>

#include <wiringPi.h>

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
#define VSCP_LEVEL2_DLL_RPIGPIO_OBJ_MUTEX   "___VSCP__DLL_L2GPIO_OBJ_MUTEX____"
#define VSCP_RPIGPIO_LIST_MAX_MSG		    2048
  
// Forward declarations
class RpiGpioWorkerTread;
class VscpRemoteTcpIf;
class wxFile;

// Actions
#define RPIGPIO_ACTION_NOOP     0

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

    bool setPullUp( const wxString& strPullUp );
    uint8_t getPullUp( void );

    bool setMonitor( bool bEnable, 
                        wxString& strEdge, 
                        vscpEventEx& event );
    bool isMonitorEnabled( void );
    uint8_t getMonitorEdge( void );
    vscpEventEx& getMonitorEvent( void );

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

// * * * Monitor input settings

// True if monitoring is enabled
bool m_bEnable_Monitor;

// 0-3 default = INT_EDGE_SETUP
uint8_t m_monitor_edge;

// Event to send when triggered
vscpEventEx m_monitorEvent;

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
    uint8_t getType( void );

    bool setMode( uint8_t mode );
    uint8_t getMode( void );

    bool setRange( uint16_t range );
    uint16_t getRange( void );

    bool setDivisor( uint16_t divisor );
    uint16_t getDivisor( void );

private:

// The GPIO port
uint8_t m_pin;

// Type of PWM (hard (PWM_OUTPUT) or soft (SOFT_PWM_OUTPUT) )
uint8_t m_type;

// Mode balanced or mark & space (default)
uint8_t m_mode;

// PWM range
uint16_t m_range;

// PWM divisor
uint16_t m_divisor;

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

private:

// The GPIO port
uint8_t m_pin;

};

// ----------------------------------------------------------------------------

// The local decision matrix for the driver
class CLocalDM{

public:

    /// Constructor
    CLocalDM() {
            vscp_clearVSCPFilter(&m_vscpfilter);    // Accept all events
            bCompareIndex = false;                  // Don't compare index
            m_index = 0;
            bCompareZone = false;                   // Don't compare zone
            m_zone = 0;
            bCompareSubZone = false;                // Don't compare subzone
            m_subzone = 0;
            m_action = RPIGPIO_ACTION_NOOP;
            m_strActionParam.Empty();               // Looks good (if you feel sick by this)
    };

    /// Destructor
    virtual ~CLocalDM();

private:

    // Filter
    vscpEventFilter m_vscpfilter;

    // Index compare
    bool bCompareIndex;
	uint8_t m_index;

    // Zone compare
    bool bCompareZone;
	uint8_t m_zone;

    // SubZone compare
    bool bCompareSubZone;
	uint8_t m_subzone;

    // Action to execute on match
	uint8_t m_action;

    // Parameter for action
	wxString m_strActionParam;

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
    
     /// VSCP server interface
    VscpRemoteTcpIf m_srv;
		
	std::list<vscpEvent *> m_sendList;
	std::list<vscpEvent *> m_receiveList;
	
	/*!
        Event object to indicate that there is an event in the output queue
     */
    wxSemaphore m_semSendQueue;			
	wxSemaphore m_semReceiveQueue;		
	
	// Mutex to protect the output queue
	wxMutex m_mutexSendQueue;		
	wxMutex m_mutexReceiveQueue;

    // Lists for pin definitions
    std::list<CGpioInput *> m_inputPinList;
    std::list<CGpioOutput *> m_outputPinList;
    std::list<CGpioPwm *> m_pwmPinList;
    std::list<CGpioClock *> m_gpioClockPinList; // Will hold max one entry

    // Decision matrix
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
