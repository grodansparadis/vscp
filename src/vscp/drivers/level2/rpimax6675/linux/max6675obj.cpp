// max6675.cpp: implementation of the CRpiMax6675 class.
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

#include "max6675.h"
#include "max6675obj.h"

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
// CInterface
//

CInterface::CInterface() 
{
    m_bEnable = true;
    setSpiChannel();
    setDetectOpenSensor();
    m_interval = 60;    // Every minute
    m_vscp_unit = 0;    // Kelvin
    m_vscp_class = VSCP_CLASS2_MEASUREMENT_STR;
    m_index = 0;
    m_zone = 0;
    m_subzone = 0;
}



CInterface::~CInterface()
{
    ;
}

//////////////////////////////////////////////////////////////////////
// ~CLocalDM
//

bool CInterface::setVscpClass( uint16_t vscpclass ) 
{ 
    switch ( vscpclass ) {
        case VSCP_CLASS1_MEASUREMENT:
        case VSCP_CLASS1_MEASUREMENT32:
        case VSCP_CLASS1_MEASUREMENT64:
        case VSCP_CLASS1_MEASUREZONE:
        case VSCP_CLASS1_SETVALUEZONE:
        case VSCP_CLASS2_MEASUREMENT_FLOAT:
        case VSCP_CLASS2_MEASUREMENT_STR:
            m_vscp_class = vscpclass; 
            break;

        default:
            return false;
    }

    return true;
};

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
    m_action = ACTION_RPIMAX6675_NOOP;
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
        m_action = ACTION_RPIMAX6675_NOOP;
    }
    else if ( std::string::npos != str.find("SYNC") ) {
        m_action = ACTION_RPIMAX6675_SYNC;
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


// ----------------------------------------------------------------------------

int depth_setup_parser = 0;
bool bDM = false;

void
startSetupParser( void *data, const char *name, const char **attr ) 
{
    CRpiMax6675 *pmax6675 = (CRpiMax6675 *)data;
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
                                    (const char *)VSCP_RPIMAX6675_SYSLOG_DRIVER_ID,
		                            (const char *) "Unable to read event receive mask to driver filter.");
                    }
                }
            }
            else if ( 0 == strcmp( attr[i], "filter") ) {
                if ( !attribute.empty() ) {
                    if ( !vscp_readFilterFromString( &pmax6675->m_vscpfilter, attribute ) ) {
                        syslog( LOG_ERR,
		                            "%s %s ",
                                    (const char *)VSCP_RPIMAX6675_SYSLOG_DRIVER_ID,
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
    else if ( ( 0 == strcmp( name, "max6675" ) ) && 
              ( 1 == depth_setup_parser ) ) {
        
        CInterface * pInterface = new CInterface;
        if ( NULL != pInterface ) {

            for ( int i = 0; attr[i]; i += 2 ) {
            
                std::string attribute = attr[i+1];

                // Get enable
                if ( 0 == strcmp( attr[i], "enable") ) {
                    std::string str = attribute;
                    vscp_trim( str );
                    vscp_makeUpper( str );
                    if ( std::string::npos != str.find("TRUE") ) {
                        pInterface->setEnable( true );
                    }
                    else {
                        pInterface->setEnable( false );
                    } 
                }
                // bus
                else if ( 0 == strcmp( attr[i], "spichannel") ) {
                    uint8_t ch = vscp_readStringValue( attribute );
                    pInterface->setSpiChannel( ch );
                }
                // interval
                else if ( 0 == strcmp( attr[i], "interval") ) {
                    uint16_t interval = vscp_readStringValue( attribute );
                    pInterface->setInterval( interval );
                }
                // Check open connection
                if ( 0 == strcmp( attr[i], "detect-open") ) {
                    std::string str = attribute;
                    vscp_trim( str );
                    vscp_makeUpper( str );
                    if ( std::string::npos != str.find("TRUE") ) {
                        pInterface->setDetectOpenSensor( true );
                    }
                    else {
                        pInterface->setDetectOpenSensor( false );
                    } 
                }
                // unit
                else if ( 0 == strcmp( attr[i], "unit") ) {
                    std::string str = attribute;
                    vscp_trim( str );
                    vscp_makeUpper( str );
                    if ( std::string::npos != str.find("K") ) {
                        pInterface->setUnit( 0 );
                    }
                    else if ( std::string::npos != str.find("C") ) {
                        pInterface->setUnit( 1 );
                    }
                    else if ( std::string::npos != str.find("F") ) {
                        pInterface->setUnit( 2 );
                    }
                    else {
                        uint8_t unit = vscp_readStringValue( str );
                        pInterface->setUnit( unit );
                    }
                }
                // VSCP class
                else if ( 0 == strcmp( attr[i], "vscpclass") ) {
                    uint8_t vscpclass = vscp_readStringValue( attribute );
                    if ( !pInterface->setVscpClass( vscpclass ) ) {
                        syslog( LOG_ERR,
				                    "%s %s ",
                                    (const char *)VSCP_RPIMAX6675_SYSLOG_DRIVER_ID,
				                    (const char *)"Invalid VSCP class in configuration.");
		
                    }

                }
                // index
                else if ( 0 == strcmp( attr[i], "index") ) {
                    uint8_t index = vscp_readStringValue( attribute );
                    pInterface->setIndex( index );
                }
                // zone
                else if ( 0 == strcmp( attr[i], "zone") ) {
                    uint8_t zone = vscp_readStringValue( attribute );
                    pInterface->setZone( zone );
                }
                // subzone
                else if ( 0 == strcmp( attr[i], "subzone") ) {
                    uint8_t subzone = vscp_readStringValue( attribute );
                    pInterface->setSubzone( subzone );
                }

            } // for - attributes

            // Add input to list
            pmax6675->m_interfaceList.push_back( pInterface );

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
// CRpiMax6675
//

CRpiMax6675::CRpiMax6675()
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
// ~CRpiMax6675
//

CRpiMax6675::~CRpiMax6675()
{
    // Close if not closed
	if ( !m_bQuit ) close();

    // Remove local DM
    std::list<CLocalDM*>::const_iterator iteratorDM;
    for (iteratorDM = m_LocalDMList.begin(); iteratorDM != m_LocalDMList.end(); ++iteratorDM) {
        delete *iteratorDM;
    }

    // Remove interfaces
    std::list<CInterface*>::const_iterator iteratorIf;
    for (iteratorIf = m_interfaceList.begin(); iteratorIf != m_interfaceList.end(); ++iteratorIf) {
        delete *iteratorIf;
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
CRpiMax6675::open( const char *pUsername,
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
                (const char *)VSCP_RPIMAX6675_SYSLOG_DRIVER_ID,
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
                    (const char *)VSCP_RPIMAX6675_SYSLOG_DRIVER_ID,
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
                    (const char *)VSCP_RPIMAX6675_SYSLOG_DRIVER_ID,
				    (const char *) "Failed to start worker thread." );
        return false;                    
    }

	return true;
}


//////////////////////////////////////////////////////////////////////
// close
//

void
CRpiMax6675::close(void)
{
	m_bQuit = true; // terminate the thread
    pthread_join( m_pthreadWorker, NULL );
}

//////////////////////////////////////////////////////////////////////
// addEvent2SendQueue
//

bool 
CRpiMax6675::addEvent2SendQueue(const vscpEvent *pEvent)
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

bool sendEvent( CRpiMax6675 *pObj, vscpEventEx& eventEx )
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
                    (const char *)VSCP_RPIMAX6675_SYSLOG_DRIVER_ID,
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
    
    CRpiMax6675 *pObj = (CRpiMax6675 *)data;
    
    // Initialize the pigpio lib
#ifdef USE_PIGPIOD
    int pmax6675d_session_id;
    if ( ( pmax6675d_session_id = pigpio_start( (char *)pObj->getPiGpiodHost().c_str(), 
                                                (char *)pObj->getPiGpiodPort().c_str() ) ) < 0 ) {
        syslog( LOG_ERR,
				    "%s %s driver started pigpio version: %ul, HW rev: %ul",
                    (const char *)VSCP_RPIMAX6675_SYSLOG_DRIVER_ID,
				    (const char *) "Failed to init. gpiod library.",
                    get_pigpio_version( pmax6675d_session_id ), 
                    get_hardware_revision( pmax6675d_session_id ) ); 
        return NULL;                                      
    }
#else
	if ( gpioInitialise() < 0 ) {
        syslog( LOG_ERR,
				    "%s %s driver started pigpio version: %ul, HW rev: %ul",
                    (const char *)VSCP_RPIMAX6675_SYSLOG_DRIVER_ID,
				    (const char *) "Failed to init. gpio library.",
                    gpioVersion(), gpioHardwareRevision() );
        return NULL;                    
    }
#endif                    
    
    // Debug
    syslog( LOG_ERR, 
                "%s gpioversion - Version=%d HW version =%d",
                (const char *)VSCP_RPIMAX6675_SYSLOG_DRIVER_ID, 
                gpioVersion(), gpioHardwareRevision() );

    int cntOpenInterfaces = 0;
    std::list<CInterface*>::const_iterator it;
    for ( it = pObj->m_interfaceList.begin(); 
            it != pObj->m_interfaceList.end(); 
            ++it ) {
        CInterface *pInterface = *it;   
        if ( pInterface->isEnable() ) {
            if ( pInterface->openInterface() ) {

                cntOpenInterfaces++;

                pInterface->getMax6675()->m_OpenSensor =
                    pInterface->isDetectOpenSensor() ?
                        1 : 0;
                switch ( pInterface->getUnit() ) {

                    case 0:
                        MAX6675SetScale( pInterface->getMax6675(), 
                                            MAX6675_KELVIN );
                        break;

                    default:
                    case 1:
                        MAX6675SetScale( pInterface->getMax6675(), 
                                            MAX6675_CELSIUS );
                        break;

                    case 2:
                        MAX6675SetScale( pInterface->getMax6675(), 
                                            MAX6675_FAHRENHEIT );
                        break;        
                }
            }
            else {
                syslog( LOG_ERR,
				            "%s %s ",
                            (const char *)VSCP_RPIMAX6675_SYSLOG_DRIVER_ID,
				            (const char *)"Unable to open SPI interface." );
            }
        }
    }

    // If no interface open just quit
    if ( 0 == cntOpenInterfaces ) {
        syslog( LOG_ERR,
				    "%s %s ",
                    (const char *)VSCP_RPIMAX6675_SYSLOG_DRIVER_ID,
				    (const char *)"No SPI interface. Quitting" );
        return NULL;
    }

    while ( !pObj->m_bQuit ) {
        
        if ((-1 == vscp_sem_wait(&pObj->m_semaphore_SendQueue, 100)) &&
         errno == ETIMEDOUT) {
           
            if ( pObj->m_bQuit ) continue;

            // Check if there is event(s) to handle
            if ( pObj->m_sendList.size() ) {

#ifdef RPIGPIO_DEBUG
                syslog( LOG_ERR, 
                            "%s Incoming event.",
                            (const char *)VSCP_RPIMAX6675_SYSLOG_DRIVER_ID  );
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
                            (const char *)VSCP_RPIMAX6675_SYSLOG_DRIVER_ID  );
#endif

                // Just to make sure
                if ( 0 == pEvent->sizeData ) {
                    pEvent->pdata = NULL;
                }

#ifdef RPIGPIO_DEBUG
                syslog( LOG_ERR, 
                            "%s Valid event.",
                            (const char *)VSCP_RPIMAX6675_SYSLOG_DRIVER_ID  );
#endif                

                // Feed through matrix - major filter
                if ( vscp_doLevel2Filter( pEvent, &pObj->m_vscpfilter ) ) {

#ifdef RPIGPIO_DEBUG
                    syslog( LOG_ERR, 
                            "%s After major filter.",
                            (const char *)VSCP_RPIMAX6675_SYSLOG_DRIVER_ID  );
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
                                        (const char *)VSCP_RPIMAX6675_SYSLOG_DRIVER_ID  );
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
                                        (const char *)VSCP_RPIMAX6675_SYSLOG_DRIVER_ID  );
#endif                            

                            // OK - Do action
                            switch ( pDM->getAction() ) {
/*
                                case ACTION_RPIMAX6675_SYNC:
                                    {
#ifdef RPIGPIO_DEBUG                                        
                                        syslog( LOG_ERR, 
                                                    "%s Action = ACTION_RPIGPIO_ON.",
                                                    (const char *)VSCP_RPIMAX6675_SYSLOG_DRIVER_ID  );
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
                                                        (const char *)VSCP_RPIMAX6675_SYSLOG_DRIVER_ID,
				                                        (const char *) "ACTION_RPIGPIO_ON - Invalid pin",
                                                        (int)pin );
                                        }
                                    }
                                    break;
*/
                               

                                case ACTION_RPIMAX6675_NOOP:
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
      
        //std::list<CInterface*>::const_iterator it;
        for ( it = pObj->m_interfaceList.begin(); 
                it != pObj->m_interfaceList.end(); 
                ++it ) {
            CInterface *pInterface = *it;   
            if ( pInterface->isEnable() ) {

                if ( ( now - pInterface->getLastEvent() ) > pInterface->getInterval() ) {

                    // Time to send event

                    float value;
                    uint8_t unit;

                    // Read temperature
                    switch ( pInterface->getUnit() ) {

                        case 0:     // Kelvin
                            unit = 0;
                            value = MAX6675GetTempK( pInterface->getMax6675() );
                            break;

                        case 2:     // Fahrenheit
                            unit = 2;
                            value = MAX6675GetTempF( pInterface->getMax6675() );
                            break;
        
                        default:    // Celsius
                            unit = 1;
                            value = MAX6675GetTempC( pInterface->getMax6675() );
                            break;
                    }

                }

            }
        }


    } // while

    for ( it = pObj->m_interfaceList.begin(); 
            it != pObj->m_interfaceList.end(); 
            ++it ) {
        CInterface *pInterface = *it;   
        if ( pInterface->isEnable() ) {
            pInterface->closeInterface();
        }
    }
    // Quit gpio library functionality
#ifdef USE_PIGPIOD
    // Close lib
    pigpio_stop( pgpiod_session_id );
#else    
    gpioTerminate();
#endif    

    return NULL;
}

