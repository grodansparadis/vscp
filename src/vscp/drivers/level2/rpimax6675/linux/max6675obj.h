// max6675.h: interface for the gpio driver.
//
// This file is part of the VSCP (http://www.vscp.org) 
//
// The MIT License (MIT)
//
// Copyright (C) 2000-2019 Ake Hedman, Grodans Paradis AB <info@grodansparadis.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#if !defined(____MAX6675OBJ__INCLUDED_)
#define ____MAX6675OBJ__INCLUDED_

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

#define VSCP_RPIMAX6675_SYSLOG_DRIVER_ID        "VSCP MAX6675 driver:"
#define VSCP_LEVEL2_DLL_RPIMAX6675_OBJ_MUTEX    "___VSCP__DLL_L2RPIMAX6675_OBJ_MUTEX____"
#define VSCP_RPIMAX6675_LIST_MAX_MSG		    2048

//#define USE_PIGPIOD           // if define the pigpio daemon i/f is used
                                // Defined in MAkefile normally when if2 version
                                // is built.

// Forward declarations
class VscpRemoteTcpIf;

// Actions
#define ACTION_RPIMAX6675_NOOP             0x00    // No operation
#define ACTION_RPIMAX6675_SYNC             0x01    // Sync

// Future

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

// This class encapsulates one max6675 sensor interface

class CInterface {
public:

    /// Constructor
    CInterface();

    /// Destructor
    virtual ~CInterface();

    void setEnable( bool bEnable= true ) { m_bEnable = bEnable; };
    bool isEnable( void ) { return m_bEnable; };

    void setSpiChannel( uint8_t ch=0 ) { m_spiChannel = ch; };
    uint8_t getSpiChannel( void ) { return m_spiChannel; };

    void setDetectOpenSensor( bool bEnable= true ) { m_bOpenSensor = bEnable; };
    bool isDetectOpenSensor( void ) { return m_bOpenSensor; };

    void setInterval( uint16_t interval=0 ) { m_interval = interval; };
    uint16_t getInterval( void ) { return m_interval; };

    void setUnit( uint8_t unit = 0 ) { m_vscp_unit = unit; };
    uint8_t getUnit( void ) { return m_vscp_unit; };

    bool setVscpClass( uint16_t vscpclass = VSCP_CLASS2_MEASUREMENT_STR );
    uint16_t getVscpClass( void ) { return m_vscp_class; };

    void setIndex( uint8_t index = 0 ) { m_index = index; };
    uint8_t getIndex( void ) { return m_index; };

    void setZone( uint8_t zone = 0 ) { m_zone = zone; };
    uint8_t getZone( void ) { return m_zone; };

    void setSubzone( uint8_t subzone = 0 ) { m_subzone = subzone; };
    uint8_t getSubZone( void ) { return m_subzone; };

    void setLastEvent( uint32_t last ) { m_lastEvent = last; };
    uint32_t getLastEvent( void ) { return m_lastEvent; };

    bool openInterface( void ) 
        { return ( NULL != ( m_max6675 = MAX6675Setup( m_spiChannel ) ) ); };
    void closeInterface( void ) { MAX6675Free( m_max6675 ); };

    max6675_t *getMax6675( void ) { return m_max6675; };

 private:   

    // True if interface is enabled
    bool m_bEnable;

    // SPI channel (0/1)
    uint8_t m_spiChannel;

    // Detect open/missing sensor 
    bool m_bOpenSensor;

    // Interval in seconds between events
    uint16_t m_interval;

    // Unit for temperature Kelvin=0 (default) / Celsius=1 /Fahrenheit=2
    uint8_t m_vscp_unit;

    // Measurement class (default 10)
    uint16_t m_vscp_class;

    // Sensor index
    uint8_t m_index;

    // sensor zone
    uint8_t m_zone;

    // sensor subzone
    uint8_t m_subzone;

    // Time for last event
    uint32_t m_lastEvent;

    // MAX6675 control structure pointer
    max6675_t *m_max6675;
};

// ----------------------------------------------------------------------------


class CRpiMax6675 {
public:

    /// Constructor
    CRpiMax6675();

    /// Destructor
    virtual ~CRpiMax6675();

    /*! 
        Open
        @return True on success.
     */
    bool open( const char *pUsername,
                const char *pPassword,
                const char *pHost,
                short port,
                const char *pPrefix,
                const char *pConfig);

    /*!
        Flush and close the log file
     */
    void close( void );

	/*!
		Add event to send queue 
	 */
	bool addEvent2SendQueue( const vscpEvent *pEvent );

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
	std::list<vscpEvent*> m_sendList;
	std::list<vscpEvent*> m_receiveList;
	
    // Event object to indicate that there is an event in the output queue	
    sem_t m_semaphore_SendQueue;			
	sem_t m_semaphore_ReceiveQueue;	
	
	// Mutex to protect the output queue
    pthread_mutex_t m_mutex_SendQueue;
    pthread_mutex_t m_mutex_ReceiveQueue;

    // Decision matrix
    std::list<CLocalDM*> m_LocalDMList;

    // List with interfaces
    std::list<CInterface*> m_interfaceList;
};





#endif // !defined(VSCPLOG_H__6F5CD90E_ACF7_459A_9ACB_849A57595639__INCLUDED_)
