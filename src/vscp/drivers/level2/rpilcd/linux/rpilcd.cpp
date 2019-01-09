// rpilcd.cpp: implementation of the CRpiLCD class.
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

#include "rpilcd.h"

#ifdef USE_PIGPIOD
#include <pigpiod_if2.h>
#else
#include <pigpio.h>
#endif



// Undef to get extra debug info to syslog
#define RPIGPIO_DEBUG

#define XML_BUFF_SIZE   0xffff

// Forward declarations
void *workerThread( void *data );

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
    m_action = ACTION_RPILCD_NOOP;
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
    vscp_makeUpper( str );
    vscp_trim( str );

    if ( std::string::npos != str.find("NOOP") ) {
        m_action = ACTION_RPILCD_NOOP;
    }
    /*else if ( std::string::npos != str.find("ON") ) {
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
    }*/

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


// ----------------------------------------------------------------------------

int depth_setup_parser = 0;
bool bDM = false;

void
startSetupParser( void *data, const char *name, const char **attr ) 
{
    CRpiLCD *pmax6675 = (CRpiLCD *)data;
    if ( NULL == pmax6675 ) return;

    if ( ( 0 == strcmp( name, "setup") ) && 
         ( 0 == depth_setup_parser ) ) {

        for ( int i = 0; attr[i]; i += 2 ) {

            std::string attribute = attr[i+1];
            if ( 0 == strcmp( attr[i], "mask") ) {
                if ( !attribute.empty() ) {
                    if ( !vscp_readMaskFromString( &pmax6675->m_vscpfilter, attribute ) ) {
                        syslog( LOG_ERR,
		                            "%s %s ",
                                    (const char *)VSCP_RPILCD_SYSLOG_DRIVER_ID,
		                            (const char *) "Unable to read event receive mask to driver filter.");
                    }
                }
            }
            else if ( 0 == strcmp( attr[i], "filter") ) {
                if ( !attribute.empty() ) {
                    if ( !vscp_readFilterFromString( &pmax6675->m_vscpfilter, attribute ) ) {
                        syslog( LOG_ERR,
		                            "%s %s ",
                                    (const char *)VSCP_RPILCD_SYSLOG_DRIVER_ID,
		                            (const char *) "Unable to read event receive mask to driver filter.");
                    }
                }
            }
            else if ( 0 == strcmp( attr[i], "guid") ) {
                if ( !attribute.empty() ) {
                    pmax6675->m_ifguid.getFromString( attribute );
                }
            }
#ifdef USE_PIGPIOD          
            else if ( 0 == strcmp( attr[i], "pigpiod-host") ) {
                pmax6675->setPiGpiodHost( attribute );
            }
            else if ( 0 == strcmp( attr[i], "pigpiod-port") ) {
                pmax6675->setPiGpiodPort( attribute );
            }
#endif            
            else if ( 0 == strcmp( attr[i], "index") ) {
                if ( !attribute.empty() ) {
                    pmax6675->m_index = (uint8_t)vscp_readStringValue( attribute );
                }
            }
            else if ( 0 == strcmp( attr[i], "zone") ) {
                if ( !attribute.empty() ) {
                    pmax6675->m_zone = (uint8_t)vscp_readStringValue( attribute );
                }
            }
            else if ( 0 == strcmp( attr[i], "subzone") ) {
                if ( !attribute.empty() ) {
                    pmax6675->m_subzone = (uint8_t)vscp_readStringValue( attribute );
                }
            }
        }
    } 
    else if ( ( 0 == strcmp( name, "interface" ) ) && 
              ( 1 == depth_setup_parser ) ) {
        
        //CGpioInput *pInputObj = new CGpioInput;
        if ( NULL != NULL /*pInputObj*/ ) {

            for ( int i = 0; attr[i]; i += 2 ) {
            
                std::string attribute = attr[i+1];
/*
                // Get pin
                if ( 0 == strcmp( attr[i], "pin") ) {
                    uint8_t pin = vscp_readStringValue( attribute );
                    pInputObj->setPin( pin );
                }    
                
                // Get pullup
                else if ( 0 == strcmp( attr[i], "pullup") ) {
                    pInputObj->setPullUp( attribute );
                }
                
                // Watchdog
                else if ( 0 == strcmp( attr[i], "watchdog") ) {
                    pInputObj->setWatchdog( vscp_readStringValue( attribute ) );
                }

                // Noice filter steady
                else if ( 0 == strcmp( attr[i], "noise_filter_steady") ) {
                    pInputObj->setWatchdog( vscp_readStringValue( attribute ) );
                }

                // Noise filter active
                else if ( 0 == strcmp( attr[i], "noise_filter_active") ) {
                    pInputObj->setNoiseFilterActive( vscp_readStringValue( attribute ) );
                }

                // Glitch filter
                else if ( 0 == strcmp( attr[i], "glitch_filter") ) {
                    pInputObj->setGlitchFilter( vscp_readStringValue( attribute ) );
                }
*/
            } // for - attributes

            // Add input to list
            //pmax6675->m_inputPinList.push_back( pInputObj );

        } // input obj
    }
    
    else if ( ( 0 == strcmp( name, "dm" ) ) && 
              ( 1 == depth_setup_parser ) ) {
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
                    
                    vscp_makeUpper( attribute );
                    vscp_trim( attribute );
                    
                    if ( 0 == strcmp( attribute.c_str(), "TRUE") ) {
                        pLocalDMObj->enableRow();
                    }
                    else if ( 0 == strcmp( attribute.c_str(), "FALSE") ) {
                        pLocalDMObj->disableRow();
                    }

                } 
                else if ( 0 == strcmp( attr[i], "priority-mask") ) {
                    filter.mask_priority = vscp_readStringValue( attribute );
                }
                else if ( 0 == strcmp( attr[i], "priority-filter") ) {
                    filter.filter_priority = vscp_readStringValue( attribute );
                }
                else if ( 0 == strcmp( attr[i], "class-mask") ) {
                    filter.mask_class = vscp_readStringValue( attribute );
                }
                else if ( 0 == strcmp( attr[i], "class-filter") ) {
                    filter.filter_class = vscp_readStringValue( attribute );
                }
                else if ( 0 == strcmp( attr[i], "type-mask") ) {
                    filter.mask_type = vscp_readStringValue( attribute );
                }
                else if ( 0 == strcmp( attr[i], "type-filter") ) {
                    filter.filter_type = vscp_readStringValue( attribute );
                }
                else if ( 0 == strcmp( attr[i], "guid-mask") ) {
                    vscp_getGuidFromStringToArray( filter.mask_GUID,
                                                        attribute );
                }
                else if ( 0 == strcmp( attr[i], "guid-filter") ) {
                    vscp_getGuidFromStringToArray( filter.filter_GUID,
                                                        attribute );
                }     
                else if ( 0 == strcmp( attr[i], "index") ) {
                    pLocalDMObj->setIndex( vscp_readStringValue( attribute ) );
                }
                else if ( 0 == strcmp( attr[i], "zone") ) {
                    pLocalDMObj->setZone( vscp_readStringValue( attribute ) );
                }
                else if ( 0 == strcmp( attr[i], "subzone") ) {
                    pLocalDMObj->setSubZone( vscp_readStringValue( attribute ) );
                }
                else if ( 0 == strcmp( attr[i], "action") ) {
                    pLocalDMObj->setAction( attribute );
                }
                else if ( 0 == strcmp( attr[i], "action-parameter") ) {
                    
                    pLocalDMObj->setActionParameter( attribute );

                    // Preparse the action parameters
                    std::deque<std::string> tokens;
                    vscp_split( tokens, attribute, "," );
                    for ( int idx=0; idx<3; idx++ ) {
                        if ( tokens.size() ) {
                            uint32_t val = vscp_readStringValue( tokens.front() );
                            pLocalDMObj->setArg( idx, val );
                            tokens.pop_front();
                        }
                    }
                    
                }

            } // DM obj.

            pLocalDMObj->setFilter( filter );
            pmax6675->m_LocalDMList.push_back( pLocalDMObj );

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
// CRpiLCD
//

CRpiLCD::CRpiLCD()
{
	int i;
    m_bQuit = false;
	m_setupXml = "<?xml version = \"1.0\" encoding = \"UTF-8\" ?><setup><!-- empty --></setup>";
	vscp_clearVSCPFilter( &m_vscpfilter ); // Accept all events
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



    pthread_mutex_init( &m_mutex_SendQueue, NULL );
    pthread_mutex_init( &m_mutex_ReceiveQueue, NULL );

    sem_init( &m_semaphore_SendQueue, 0, 0 );                   
    sem_init( &m_semaphore_ReceiveQueue, 0, 0 );
}

//////////////////////////////////////////////////////////////////////
// ~CRpiLCD
//

CRpiLCD::~CRpiLCD()
{
    // Close if not closed
	if ( !m_bQuit ) close();

    // Remove local DM
    std::list<CLocalDM*>::const_iterator iterator5;
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
CRpiLCD::open( const char *pUsername,
		            const char *pPassword,
		            const char *pHost,
		            short port,
		            const char *pPrefix,
		            const char *pConfig )
{
    std::string strConfig = pConfig;
   
	m_username = pUsername;
	m_password = pPassword;
	m_host = pHost;
	m_port = port;
	m_prefix = pPrefix;

	vscp_std_decodeBase64IfNeeded( pConfig, m_setupXml );

	// First look on to the host and get the configuration 
	// variable (if there)

	if ( VSCP_ERROR_SUCCESS != m_srv.doCmdOpen( m_host,
                                                    m_port,
												    m_username,
												    m_password ) ) {
		syslog(LOG_ERR,
				"%s %s ",
                (const char *)VSCP_RPILCD_SYSLOG_DRIVER_ID,
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
        // OK if not available we use defaults
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
                    (const char *)VSCP_RPILCD_SYSLOG_DRIVER_ID,
				    (const char *) "Failed parse XML setup." );
    }

    XML_ParserFree( xmlParser );

    // Close the channel
	m_srv.doCmdClose();

    // create and start the workerthread
    if ( 0 != pthread_create( &m_pthreadWorker,  
                                NULL,          
                                workerThread,
                                (void *)this ) ) {
        syslog( LOG_ERR,
				    "%s %s",
                    (const char *)VSCP_RPILCD_SYSLOG_DRIVER_ID,
				    (const char *) "Failed to start worker thread." );
        return false;                    
    }

	return true;
}


//////////////////////////////////////////////////////////////////////
// close
//

void
CRpiLCD::close(void)
{
	m_bQuit = true; // terminate the thread
    pthread_join( m_pthreadWorker, NULL );
}

//////////////////////////////////////////////////////////////////////
// addEvent2SendQueue
//

bool 
CRpiLCD::addEvent2SendQueue(const vscpEvent *pEvent)
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

bool sendEvent( CRpiLCD *pObj, vscpEventEx& eventEx )
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
                    (const char *)VSCP_RPILCD_SYSLOG_DRIVER_ID,
				    (const char *) "Failed to convert send event"  );
            return false;
        }

    }

    return true;
}





//////////////////////////////////////////////////////////////////////
//                Workerthread - RpiMax6675WorkerTread
//////////////////////////////////////////////////////////////////////





void *workerThread( void *data )
{
    
    CRpiLCD *pObj = (CRpiLCD *)data;
    
    // Initialize the pigpio lib
    //gpioCfgInternals(1<<10, 0);  // Prevent signal 11 error
#ifdef USE_PIGPIOD
    int pmax6675d_session_id;
    if ( ( pmax6675d_session_id = pigpio_start( (char *)pObj->getPiGpiodHost().c_str(), 
                                                (char *)pObj->getPiGpiodPort().c_str() ) ) < 0 ) {
        syslog( LOG_ERR,
				    "%s %s driver started pigpio version: %ul, HW rev: %ul",
                    (const char *)VSCP_RPILCD_SYSLOG_DRIVER_ID,
				    (const char *) "Failed to init. gpiod library.",
                    get_pigpio_version( pmax6675d_session_id ), 
                    get_hardware_revision( pmax6675d_session_id ) ); 
        return NULL;                                      
    }
#else
	if ( gpioInitialise() < 0 ) {
        syslog( LOG_ERR,
				    "%s %s driver started pigpio version: %ul, HW rev: %ul",
                    (const char *)VSCP_RPILCD_SYSLOG_DRIVER_ID,
				    (const char *) "Failed to init. gpio library.",
                    gpioVersion(), gpioHardwareRevision() );
        return NULL;                    
    }
#endif                    
    

    // Debug
    syslog( LOG_ERR, 
                "%s gpioversion - Version=%d HW version =%d",
                (const char *)VSCP_RPILCD_SYSLOG_DRIVER_ID, 
                gpioVersion(), gpioHardwareRevision() );


    while ( !pObj->m_bQuit ) {
        
        if ((-1 == vscp_sem_wait(&pObj->m_semaphore_SendQueue, 100)) &&
         errno == ETIMEDOUT) {
           
            if ( pObj->m_bQuit ) continue;

            // Check if there is event(s) to handle
            if ( pObj->m_sendList.size() ) {

#ifdef RPIGPIO_DEBUG
                syslog( LOG_ERR, 
                            "%s Incoming event.",
                            (const char *)VSCP_RPILCD_SYSLOG_DRIVER_ID  );
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
                            (const char *)VSCP_RPILCD_SYSLOG_DRIVER_ID  );
#endif

                // Just to make sure
                if ( 0 == pEvent->sizeData ) {
                    pEvent->pdata = NULL;
                }

#ifdef RPIGPIO_DEBUG
                syslog( LOG_ERR, 
                            "%s Valid event.",
                            (const char *)VSCP_RPILCD_SYSLOG_DRIVER_ID  );
#endif                

                // Feed through matrix - major filter
                if ( vscp_doLevel2Filter( pEvent, &pObj->m_vscpfilter ) ) {

#ifdef RPIGPIO_DEBUG
                    syslog( LOG_ERR, 
                            "%s After major filter.",
                            (const char *)VSCP_RPILCD_SYSLOG_DRIVER_ID  );
#endif

                    std::list<CLocalDM*>::const_iterator it;
                    for ( it = pObj->m_LocalDMList.begin(); 
                            it != pObj->m_LocalDMList.end(); 
                            ++it ) {

                        CLocalDM *pDM = (CLocalDM *)*it;
                        if ( vscp_doLevel2Filter( pEvent, &pDM->getFilter() ) ) {

#ifdef RPIGPIO_DEBUG
                            syslog( LOG_ERR, 
                                        "%s After DM row filter.",
                                        (const char *)VSCP_RPILCD_SYSLOG_DRIVER_ID  );
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
                                        (const char *)VSCP_RPILCD_SYSLOG_DRIVER_ID  );
#endif                            

                            // OK - Do action
                            switch ( pDM->getAction() ) {
/*
                                case ACTION_RPIGPIO_ON:
                                    {
#ifdef RPIGPIO_DEBUG                                        
                                        syslog( LOG_ERR, 
                                                    "%s Action = ACTION_RPIGPIO_ON.",
                                                    (const char *)VSCP_RPILCD_SYSLOG_DRIVER_ID  );
#endif
                                        uint8_t pin = (uint8_t)pDM->getArg( 0 );
                                        if ( pin <= 53 ) { 

#ifdef USE_PIGPIOD
                                            gpio_write( pmax6675d_session_id, pin, 1 );
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
                                                        (const char *)VSCP_RPILCD_SYSLOG_DRIVER_ID,
				                                        (const char *) "ACTION_RPIGPIO_ON - Invalid pin",
                                                        (int)pin );
                                        }
                                    }
                                    break;
*/
                               

                                case ACTION_RPILCD_NOOP:
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
   
        timeval curTime;
        gettimeofday( &curTime, NULL );
        uint32_t now = 1000 * curTime.tv_sec + ( curTime.tv_usec / 1000 );
      

    } // while

    // Quit gpio library functionality
#ifdef USE_PIGPIOD
    // Close lib
    pigpio_stop( pgpiod_session_id );
#else    
    gpioTerminate();
#endif    

    return NULL;
}

