// peakobj.cpp:  
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2015 
// Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
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
// $RCSfile: peakobj.cpp,v $                                       
// $Date: 2005/09/26 21:57:52 $                                  
// $Author: akhe $                                              
// $Revision: 1.8 $ 

#include "stdio.h"
#include "../../../../common/canal.h"
#include "peakobj.h"
#include "dlldrvobj.h"


// Prototypes
#ifdef WIN32
void workThreadTransmit( void *pObject );
void workThreadReceive( void *pObject );
#else
void *workThreadTransmit( void *pObject );
void *workThreadReceive( void *pObject );
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// CPeakObj
//

CPeakObj::CPeakObj()
{ 
	m_initFlag = 0;

	// Error codes
	m_lastErrorCode = 0;
	m_lastSubErrorCode = 0;
	*m_lastErrorStr = 0;
	
	// No filter mask
	m_filter = 0;
	m_mask = 0;

	m_bRun = false;

	// No DDL loaded
	m_hinst = NULL; 

	// Library not initialized
	m_bInit = false;

	// We are not open
	m_bOpen = false;

#ifdef WIN32
	
	m_hTreadReceive = 0;
	m_hTreadTransmit = 0;

	// Create the device AND LIST access mutexes
	m_peakMutex = CreateMutex( NULL, true, CANAL_DLL_PEAKDRV_OBJ_MUTEX );
	m_receiveMutex = CreateMutex( NULL, true, CANAL_DLL_PEAKDRV_RECEIVE_MUTEX );
	m_transmitMutex = CreateMutex( NULL, true, CANAL_DLL_PEAKDRV_TRANSMIT_MUTEX );

#else
	
	m_flog = NULL;
	pthread_mutex_init( &m_ixxMutex, NULL );
	pthread_mutex_init( &m_receiveMutex, NULL );
	pthread_mutex_init( &m_transmitMutex, NULL );

#endif

	dll_init( &m_transmitList, SORT_NONE );
	dll_init( &m_receiveList, SORT_NONE );
}

//////////////////////////////////////////////////////////////////////
// ~CPeakObj
//

CPeakObj::~CPeakObj()
{		 
	close();
	
	LOCK_MUTEX( m_transmitMutex );
	dll_removeAllNodes( &m_transmitList );
	
	LOCK_MUTEX( m_receiveMutex );
	dll_removeAllNodes( &m_receiveList );


#ifdef WIN32	
	
	if ( NULL != m_peakMutex ) CloseHandle( m_peakMutex );	
	if ( NULL != m_receiveMutex ) CloseHandle( m_receiveMutex );
	if ( NULL != m_transmitMutex ) CloseHandle( m_transmitMutex );

#else

	pthread_mutex_destroy( &m_ixxMutex );
	pthread_mutex_destroy( &m_receiveMutex );
	pthread_mutex_destroy( &m_transmitMutex );

#endif
}


struct __peaksymtbl {
	char symname[ 32 ];	// Symbolic name
	int id;				// index code
	char dllname[ 64 ];	// DLL name
	bool bPnp;			// Plug and Play device 
} peaksymtbl[] = {
		"CANDONGLE",	0,	"pcan_dng.dll", false,
		"CANDONGLEPRO",	1,	"pcan_dnp.dll", false,
		"CANISA",		2,  "pcan_isa.dll", false,
		"CANPCI",		3,  "pcan_pci.dll", true,
		"CANPCI2",		4,  "pcan_2pci.dll", true,
		"CANUSB",		5,	"pcan_usb.dll", true,
		"",				-1, "", false
};

//////////////////////////////////////////////////////////////////////
// setLastError
//

void CPeakObj::setLastError( unsigned long errorcode, 
								unsigned long suberrorcode, 
								char  *perrorstr )
{
	m_lastErrorCode = errorcode;
	m_lastSubErrorCode = suberrorcode;
#ifdef WIN32
	strncpy_s( m_lastErrorStr, sizeof( m_lastErrorStr ), perrorstr, sizeof( m_lastErrorStr ) );
#else
	strncpy( m_lastErrorStr, perrorstr, sizeof( m_lastErrorStr ) );
#endif
}


// PCAN Lite driver only allows use of one
// hardware and one clint on each interface.
static int nDongleDriverUseCnt = 0;
static int nDongleProDriverUseCnt = 0;
static int nIsaDriverUseCnt = 0;
static int nPciDriverUseCnt = 0;
static int nPci2DriverUseCnt = 0;
static int nUSBDriverUseCnt = 0;

//////////////////////////////////////////////////////////////////////
// open
//
//
// filename
//-----------------------------------------------------------------------------
// Parameters for the driver as a string on the following form
//
// For non PnP boards/adapters
//
// "board;bus-speed;hwtype;port;irq;channel;filter;mask"
//
// else
//
// "board;bus-speed;channel;filter;mask"
//
// board
// =====
//
// board can be on a symbolic for or as a number as follows
//
// CANDONGLE      or  0	LPT port adapter
// CANDONGLEPRO		or  1	LPT port adapter (PRO version)
// CANISA			    or  2	for ISA adapter
// CANPCI			    or  3	1 channel PCI adapter
// CANPCI2			  or  4	2 channel PCI adapter
// CANUSB			    or  5	USB adapter
//
// bus-speed
// ========================================================
// One of the predefined bitrates can be set here
// 
//   5 for 5 Kbs
//  10 for 10 Kbs
//  20 for 20 Kbs
//	50 for 50 Kbs
//  100 for 100 Kbs
//  125 for 125 Kbs (default)
//  250 for 250 Kbs
//  500 for 500 Kbs
//  800 for 800 Kbs
//  1000 for 1000 Mbs
//
// hwtype
// ========================================================
// Only valid for non PNP cards
//
// CANDONGLE
// ---------
//		2 - dongle
//		3 - epp
//		5 - sja
//		6 - sja-epp		
//
// CANDONGLEPRO
// ------------
//		7 - dongle pro
//		8 - epp
//
// CANISA
// ------
//		1 - ISA
//		9 - SJA
// 
// port
// ========================================================
// For ISA and parallel port adapters this is the hardware 
// port address
//
//
// irq
// ========================================================
// This is the interrupt to use for non PNP devices.
//
//
// channel
// ========================================================
// Is a value from 0 an up indicating the CAN channel on the 
// selected board.
//
//
// filter
// ========================================================
// Is the hardware dependent filter for this board hardware. 
// Note that this filter may work in a different way then 
// the CANAL filter.
//
//
// mask
// ========================================================
// Is the hardware dependent mask for this board hardware. 
// Note that this filter may work in a different way then 
// the CANAL filter.
//
//
//
//
// 
// flags 
//-----------------------------------------------------------------------------
//
// bit 0
// =====
//  0		11 bit identifier mode
//	1		11/29 bit identifier mode
//
// bit 2
// =====
//
// bit 2
// =====
//
// bit 3
// =====
//
// bit 4
// =====
// 

bool CPeakObj::open( const char *szFileName, unsigned long flags )
{
	const char *p;
	uint32_t busspeed = 125;
	uint16_t btr0btr1 = 0x031C;
	char szDrvParams[ MAX_PATH ];
	int port;
	int irq;
	int hwtype = 0;
	char *next_token = NULL;


	// Save flags
	m_initFlag = flags;

	// save parameter string and convert to upper case
#ifdef WIN32
	strncpy_s( szDrvParams, sizeof( szDrvParams ), szFileName, MAX_PATH );
	_strupr_s( szDrvParams );
#else
	strncpy( szDrvParams, szFileName, MAX_PATH );
	_strupr( szDrvParams );
#endif
	

	// Initiate statistics
	m_stat.cntReceiveData = 0;
	m_stat.cntReceiveFrames = 0;
	m_stat.cntTransmitData = 0;
	m_stat.cntTransmitFrames = 0;

	m_stat.cntBusOff = 0;
	m_stat.cntBusWarnings = 0;
	m_stat.cntOverruns = 0;	

	
	// if open we have noting to do
	if ( m_bRun ) return true;
	
	// Boardtype
#ifdef WIN32
	p = strtok_s( (char * )szDrvParams, ";", &next_token );
#else
	p = strtok( (char * )szDrvParams, ";" );
#endif
	if ( NULL != p ) {
	
		if ( isalpha( *p ) ) {
		
			// Symbolic form
			int symidx = 0;
			while( -1 != peaksymtbl[ symidx ]. id ) {
			
				if ( NULL != strstr( peaksymtbl[ symidx ]. symname, p ) ) {
					m_dwBrdType = peaksymtbl[ symidx ]. id;
					m_bPnp = peaksymtbl[ symidx ].bPnp;
#ifdef WIN32
					strcpy_s( m_dllName, sizeof( m_dllName ), peaksymtbl[ symidx ]. dllname );
#else
					strcpy( m_dllName, peaksymtbl[ symidx ]. dllname );
#endif
					break;
				}

				symidx++;	

			}
				 	
		}
		else {
			
			// Numeric form
			m_dwBrdType = atoi( p );
			m_bPnp = peaksymtbl[ m_dwBrdType ].bPnp;
#ifdef WIN32
			strcpy_s( m_dllName, sizeof( m_dllName ), peaksymtbl[ m_dwBrdType ]. dllname );
#else
			strcpy_s( m_dllName, peaksymtbl[ m_dwBrdType ]. dllname );
#endif

		}

	}

	// Set default values from board-type
	m_channel = 0;
	port = -1;
	irq = -1;


	// Bus-Speed
#ifdef WIN32
	p = strtok_s( NULL, ";", &next_token );
#else
	p = strtok( NULL, ";" );
#endif
	if ( NULL != p ) {		
		if ( ( NULL != strstr( p, "0x" ) ) || ( NULL != strstr( p, "0X" ) )  ) {
#ifdef WIN32
			sscanf_s( p + 2, "%x", &busspeed );
#else
			sscanf( p + 2, "%x", &busspeed );
#endif
		}
		else {
			busspeed = atol( p );
		}
	}


		// Handle busspeed
	switch ( busspeed ) {
	
		case 5:
			btr0btr1 = 0x7F7F;
			break;

		case 10:
			btr0btr1 = 0x672F;
			break;

		case 20:
			btr0btr1 = 0x532F;
			break;

		case 50:
			btr0btr1 = 0x472F;
			break;

		case 100:
			btr0btr1 = 0x432F;
			break;

		case 125:
			btr0btr1 = 0x031C;
			break;

		case 250:
			btr0btr1 = 0x011C;
			break;

		case 500:
			btr0btr1 = 0x001C;
			break;

		case 800:
			btr0btr1 = 0x0016;
			break;

		case 1000:
			btr0btr1 = 0x0014;
			break;

	}


	if ( !m_bPnp ) {

		// hwtype
#ifdef WIN32
		p = strtok_s( NULL, ";", &next_token );
#else
		p = strtok( NULL, ";" );
#endif
		if ( NULL != p ) {		
			if ( ( NULL != strstr( p, "0x" ) ) || ( NULL != strstr( p, "0X" ) )  ) {
#ifdef WIN32
				sscanf( p + 2, "%x", &hwtype );
#else
				sscanf( p + 2, "%x", &hwtype );
#endif
			}
			else {
				hwtype = atoi( p );
			}
		}

		// port
#ifdef WIN32
		p = strtok_s( NULL, ";", &next_token );
#else
		p = strtok( NULL, ";" );
#endif
		if ( NULL != p ) {		
			if ( ( NULL != strstr( p, "0x" ) ) || ( NULL != strstr( p, "0X" ) )  ) {
#ifdef WIN32
				sscanf( p + 2, "%x", &port );
#else
				sscanf( p + 2, "%x", &port );
#endif
			}
			else {
				port = atoi( p );
			}
		}


		// irq
#ifdef WIN32
		p = strtok_s( NULL, ";", &next_token );
#else
		p = strtok( NULL, ";" );
#endif
		if ( NULL != p ) {		
			if ( ( NULL != strstr( p, "0x" ) ) || ( NULL != strstr( p, "0X" ) )  ) {
#ifdef WIN32
				sscanf( p + 2, "%x", &irq );
#else
				sscanf( p + 2, "%x", &irq );
#endif
			}
			else {
				irq = atoi( p );
			}
		}

	}


	// channel 
#ifdef WIN32
	p = strtok_s( NULL, ";", &next_token );
#else
	p = strtok( NULL, ";" );
#endif
	if ( NULL != p ) {		
		if ( ( NULL != strstr( p, "0x" ) ) || ( NULL != strstr( p, "0X" ) )  ) {
#ifdef WIN32
			sscanf( p + 2, "%x", &m_channel );
#else
			sscanf( p + 2, "%x", &m_channel );
#endif
		}
		else {
			m_channel = atoi( p );
		}
	}


	// Filter
#ifdef WIN32
	p = strtok_s( NULL, ";", &next_token );
#else
	p = strtok( NULL, ";" );
#endif
	if ( NULL != p ) {		
		if ( ( NULL != strstr( p, "0x" ) ) || ( NULL != strstr( p, "0X" ) )  ) {
#ifdef WIN32
			sscanf( p + 2, "%x", &m_Peak_filter );
#else
			sscanf( p + 2, "%x", &m_Peak_filter );
#endif
		}
		else {
			m_Peak_filter = atol( p );
		}
	}


	// Mask
#ifdef WIN32
	p = strtok_s( NULL, ";", &next_token );
#else
	p = strtok( NULL, ";" );
#endif
	if ( NULL != p ) {		
		if ( ( NULL != strstr( p, "0x" ) ) || ( NULL != strstr( p, "0X" ) )  ) {
#ifdef WIN32
			sscanf( p + 2, "%x", &m_Peak_mask );
#else
			sscanf( p + 2, "%x", &m_Peak_mask );
#endif
		}
		else {
			m_Peak_mask = atol( p );
		}
	}

	
	switch ( m_dwBrdType ) {
		
		case 0:	// CAN DONGLE
			if ( !nDongleDriverUseCnt ) {
				
				// Initialize Driver DLL
				if ( !initialize( m_dllName, m_bPnp ) ) {
		
					// Failed to initialize
					return false;	
				}
			}
			else if ( 1 == nDongleDriverUseCnt ) {
				// Only one instance allowed
				setLastError( CANAL_ERROR_ONLY_ONE_INSTANCE, 
								GetLastError(),  
								"PEAK CAN DONGLE accept only one instance");
				return false;
			}
			nDongleDriverUseCnt++;
			port = 0x378;		
			irq = 7;
			break;

		case 1:	// CAN DONGLE PRO
			if ( !nDongleProDriverUseCnt ) {
				
				// Initialize Driver DLL
				if ( !initialize( m_dllName, m_bPnp ) ) {
		
					// Failed to initialize
					return false;	
				}
			}
			else if ( 1 == nDongleProDriverUseCnt ) {
				// Only one instance allowed
				setLastError( CANAL_ERROR_ONLY_ONE_INSTANCE, 
								GetLastError(),  
								"PEAK CAN DONGLE PRO accept only one instance");
				return false;
			}
			nDongleProDriverUseCnt++;
			port = 0x378;		
			irq = 7;
			break;

		case 2:	// ISA
			if ( !nIsaDriverUseCnt ) {
				
				// Initialize Driver DLL
				if ( !initialize( m_dllName, m_bPnp ) ) {
		
					// Failed to initialize
					return false;	
				}

			}
			else if ( 1 == nIsaDriverUseCnt ) {
				// Only one instance allowed
				setLastError( CANAL_ERROR_ONLY_ONE_INSTANCE, 
								GetLastError(),  
								"PEAK CAN ISA accept only one instance");
				return false;
			}
			nIsaDriverUseCnt++;
			port = 0x300;		
			irq = 10;
			break;		

		case 3:	// PCI
			if ( !nPciDriverUseCnt ) {
				
				// Initialize Driver DLL
				if ( !initialize( m_dllName, m_bPnp ) ) {
		
					// Failed to initialize
					return false;	
				}
			}
			else if ( 1 == nPciDriverUseCnt ) {
				// Only one instance allowed
				setLastError( CANAL_ERROR_ONLY_ONE_INSTANCE, 
								GetLastError(),  
								"PEAK CAN PCI accept only one instance");
				return false;
			}
			nPciDriverUseCnt++;
			break;

		case 4:	// PCI2	
			if ( !nPci2DriverUseCnt ) {
				
				// Initialize Driver DLL
				if ( !initialize( m_dllName, m_bPnp ) ) {
		
					// Failed to initialize
					return false;	
				}
			}
			else if ( 1 == nPci2DriverUseCnt ) {
				// Only one instance allowed
				setLastError( CANAL_ERROR_ONLY_ONE_INSTANCE, 
								GetLastError(),  
								"PEAK CAN PCI2 accept only one instance");
				return false;
			}
			nPci2DriverUseCnt++;
			break;

		case 5:	// USB
			if ( !nUSBDriverUseCnt ) {
				
				// Initialize Driver DLL
				if ( !initialize( m_dllName, m_bPnp ) ) {
		
					// Failed to initialize
					return false;	
				}
			}
			else if ( 1 == nUSBDriverUseCnt ) {
				// Only one instance allowed
				setLastError( CANAL_ERROR_ONLY_ONE_INSTANCE, 
								GetLastError(),  
								"PEAK CAN USB accept only one instance");
				return false;
			}
			nUSBDriverUseCnt++;
			
			break;
	}

	// Init the device
	unsigned long rv;
	if ( m_bPnp ) {
		rv = m_procInitPnp( btr0btr1, ( m_initFlag & 1 ) );	
		if ( PEAK_CAN_ERR_OK != rv ) {
			setLastError( CANAL_ERROR_INIT_FAIL, 
								rv,  
								"Init error (PCI): suberror is driver failure code");
			return false;
		}
	}
	else {
		rv = m_procInit( btr0btr1, ( m_initFlag & 1 ), hwtype, port, irq );
		if ( PEAK_CAN_ERR_OK != rv ) {
			setLastError( CANAL_ERROR_INIT_FAIL, 
								rv,  
								"Init error: suberror is driver failure code");
			return false;
		}
	}

	// Run run run ..... 
	// (otional (for hard - fellow - rockers) "to the hills..."
	m_bRun = true;

#ifdef WIN32
	
	// Start write thread 
	DWORD threadId;
	if ( NULL == 
			( m_hTreadTransmit = CreateThread(	NULL,
										0,
										(LPTHREAD_START_ROUTINE) workThreadTransmit,
										this,
										0,
										&threadId ) ) ) { 
		// Failure
		setLastError( CANAL_ERROR_INIT_FAIL, 
								GetLastError(),  
								"Init error: Unable to create transmit thread");
		close();
		return false;
	}

	// Start read thread 
	if ( NULL == 
			( m_hTreadReceive = CreateThread(	NULL,
										0,
										(LPTHREAD_START_ROUTINE) workThreadReceive,
										this,
										0,
										&threadId ) ) ) { 
		// Failure
		setLastError( CANAL_ERROR_INIT_FAIL, 
								GetLastError(),  
								"Init error: Unable to create receive thread");
		close();
		return  false;
	}
	
	// Release the mutex
	UNLOCK_MUTEX( m_peakMutex );
	UNLOCK_MUTEX( m_receiveMutex );
	UNLOCK_MUTEX( m_transmitMutex );
	

#else // LINUX


	pthread_attr_t thread_attr;
	pthread_attr_init( &thread_attr );
	
	
	// Create the log write thread.
	if ( pthread_create( 	&m_threadId,
								&thread_attr,
								workThreadTransmit,
								this ) ) {	
							
		syslog( LOG_CRIT, "canallogger: Unable to create peakdrv write thread.");
		setLastError( CANAL_ERROR_INIT_FAIL, 
								GetLastError(),  
								"Init error: Unable to create transmit thread");
		rv = false;
		fclose( m_flog );
	}


	// Create the log write thread.
	if ( pthread_create( 	&m_threadId,
								&thread_attr,
								workThreadReceive,
								this ) ) {	
							
		syslog( LOG_CRIT, "canallogger: Unable to create peakdrv receive thread.");
		setLastError( CANAL_ERROR_INIT_FAIL, 
								GetLastError(),  
								"Init error: Unable to create receive thread");
		rv = false;
		fclose( m_flog );
	}
		

	// Release the mutex
	pthread_mutex_unlock( &m_ixxMutex );

#endif	

	// We are open
	m_bOpen = true;

	return true;
}

 
//////////////////////////////////////////////////////////////////////
// close
//

bool CPeakObj::close( void )
{	
	unsigned long rv;

	// Do nothing if already terminated
	if ( !m_bRun ) return false;
	
	m_bRun = false;

	UNLOCK_MUTEX( m_peakMutex );
	Sleep( 1000 );
	LOCK_MUTEX( m_peakMutex );
	
	// Close the driver
	rv = m_procClose();
	if ( PEAK_CAN_ERR_OK != rv ) {
		setLastError( CANAL_ERROR_INIT_FAIL, 
						rv,  
						"Error on close.");
			return false;
	}
	
	// terminate the worker thread 
#ifdef WIN32	
	
	// Wait for transmit thread to terminate
	while ( true ) {
		GetExitCodeThread( m_hTreadTransmit, &rv );
		if ( STILL_ACTIVE != rv ) break;
	}

	// Wait for receive thread to terminate
	while ( true ) {
		GetExitCodeThread( m_hTreadReceive, &rv );
		if ( STILL_ACTIVE != rv ) break;
	}
	

		
#else
	int *trv;
	pthread_join( m_threadIdReceive, (void **)&trv );
	pthread_join( m_threadIdTransmit, (void **)&trv );
	pthread_mutex_destroy( &m_ixxMutex );
	
#endif

	

	switch ( m_dwBrdType ) {
		
		case 0:			// CAN DONGLE
			nDongleDriverUseCnt--;
			if ( !nDongleDriverUseCnt ) {
				// Free library
				FreeLibrary( m_hinst );
				m_hinst = NULL;
			}
			break;

		case 1:			// CAN DONGLE PRO
			nDongleProDriverUseCnt--;
			if ( !nDongleProDriverUseCnt ) {
				// Free library
				FreeLibrary( m_hinst );
				m_hinst = NULL;
			}
			break;

		case 2:			// ISA
			nIsaDriverUseCnt--;
			if ( !nIsaDriverUseCnt ) {
				// Free library
				FreeLibrary( m_hinst );
				m_hinst = NULL;
			}
			break;		

		case 3:			// PCI
			nPciDriverUseCnt--;
			if ( !nPciDriverUseCnt ) {
				// Free library
				FreeLibrary( m_hinst );
				m_hinst = NULL;
			}
			break;

		case 4:			// PCI2		
			nPci2DriverUseCnt--;
			if ( !nPci2DriverUseCnt ) {
				// Free library
				FreeLibrary( m_hinst );
				m_hinst = NULL;
			}
			break;

		case 5:			// USB
			nUSBDriverUseCnt--;
			if ( !nUSBDriverUseCnt ) {
				// Free library
				FreeLibrary( m_hinst );
				m_hinst = NULL;
			}
			break;
	}
	
	// We are closed
	m_bOpen = false;

	return true;
}


//////////////////////////////////////////////////////////////////////
// doFilter
//

bool CPeakObj::doFilter( canalMsg *pcanalMsg )
{	
	unsigned long msgid = ( pcanalMsg->id & 0x1fffffff);
	if ( !m_mask ) return true;	// fast escape

	// Set bit 32 if extended message
	if ( pcanalMsg->flags | CANAL_IDFLAG_EXTENDED ) {
		msgid &= 0x1fffffff;
		msgid |= 80000000;	
	}
	else {
		// Standard message
		msgid &= 0x000007ff;
	}

	// Set bit 31 if RTR
	if ( pcanalMsg->flags | CANAL_IDFLAG_RTR ) { 
		msgid |= 40000000;	
	}

	return !( ( m_filter ^ msgid ) & m_mask );
}

//////////////////////////////////////////////////////////////////////
// setFilter
//

bool CPeakObj::setFilter( unsigned long filter )
{
	m_filter = filter;
	return true;
}


//////////////////////////////////////////////////////////////////////
// setMask
//

bool CPeakObj::setMask( unsigned long mask )
{
	m_mask = mask;
	return true;
}



//////////////////////////////////////////////////////////////////////
// writeMsg
//

bool CPeakObj::writeMsg( canalMsg *pMsg )
{	
	bool rv = false;
	
	if ( NULL != pMsg ) {

	
		// Must be room for the message
		if ( m_transmitList.nCount < PEAKDRV_MAX_SNDMSG ) {

			dllnode *pNode = new dllnode;
			
			if ( NULL != pNode ) {
			
				canalMsg *pcanalMsg = new canalMsg;

				pNode->pObject = pcanalMsg;
				pNode->pKey = NULL;
				pNode->pstrKey = NULL;

				if ( NULL != pcanalMsg ) {
					memcpy( pcanalMsg, pMsg, sizeof( canalMsg ) );
				}

				LOCK_MUTEX( m_transmitMutex );
				dll_addNode( &m_transmitList, pNode );
				UNLOCK_MUTEX( m_transmitMutex );
 
				rv = true;

			}
			else {

				delete pMsg;

			}
		}
	}
	
	return rv;		
}


//////////////////////////////////////////////////////////////////////
// readMsg
//

bool CPeakObj::readMsg( canalMsg *pMsg )
{
	bool rv = false;
	
	if ( ( NULL != m_receiveList.pHead ) && 
			( NULL != m_receiveList.pHead->pObject ) ) {
		
		memcpy( pMsg, m_receiveList.pHead->pObject, sizeof( canalMsg ) );
		LOCK_MUTEX( m_receiveMutex );
		dll_removeNode( &m_receiveList, m_receiveList.pHead );
		UNLOCK_MUTEX( m_receiveMutex );

		rv = true;
	}

	return rv;
}


///////////////////////////////////////////////////////////////////////////////
//	dataAvailable
//

int CPeakObj::dataAvailable( void )
{
	int cnt;
	
	LOCK_MUTEX( m_receiveMutex );
	cnt = dll_getNodeCount( &m_receiveList );	
	UNLOCK_MUTEX( m_receiveMutex );
 
	return cnt;
}


///////////////////////////////////////////////////////////////////////////////
//	getStatistics
//

bool CPeakObj::getStatistics( PCANALSTATISTICS pCanalStatistics )
{	
	// Must be a valid pointer
	if ( NULL == pCanalStatistics ) return false;

	memcpy( pCanalStatistics, &m_stat, sizeof( canalStatistics ) );

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//	getStatus
//

bool CPeakObj::getStatus( PCANALSTATUS pCanalStatus )
{
	// Must be a valid pointer
	if ( NULL == pCanalStatus ) return false;
	
	// Get return Code
	if ( m_bOpen ) {
		pCanalStatus->channel_status = ( m_procStatus() & 0xffff );
	
		if ( PEAK_CAN_ERR_BUSOFF & pCanalStatus->channel_status ) {
			pCanalStatus->channel_status |= 0x80000000;	
		}

		if ( PEAK_CAN_ERR_BUSHEAVY & pCanalStatus->channel_status ) {
			pCanalStatus->channel_status |= 0x40000000;	
		}

		if ( PEAK_CAN_ERR_BUSLIGHT & pCanalStatus->channel_status ) {
			pCanalStatus->channel_status |= 0x20000000;	
		}
	}

	// Fill in error information
	pCanalStatus->lasterrorcode = m_lastErrorCode;
	pCanalStatus->lasterrorsubcode = m_lastSubErrorCode;
#ifdef WIN32
	strncpy_s( pCanalStatus->lasterrorstr, m_lastErrorStr, sizeof( pCanalStatus->lasterrorstr ) );
#else
	strncpy( pCanalStatus->lasterrorstr, m_lastErrorStr, sizeof( pCanalStatus->lasterrorstr ) );
#endif

	return true;
}



///////////////////////////////////////////////////////////////////////////////
// initialize
//

bool CPeakObj::initialize( char *path, bool bPnp )
{

	// Check that the file exists
	//if ( !( ::wxFileExists( path ) ) ) return false;


	if ( NULL == ( m_hinst = LoadLibrary( path ) ) ) {
		//TRACE("Unable to load canaldll dynamic link library.");
		setLastError( CANAL_ERROR_LIBRARY, GetLastError(), "Unable to load peak dll" );
		return false;
	}

	if ( bPnp ){

		if ( NULL == ( m_procInitPnp = 
						(LPFNDLL_INITPNP)GetProcAddress( m_hinst, "CAN_Init" ) ) ) {
			setLastError( CANAL_ERROR_PROCADDRESS, GetLastError(), "Unable get procaddress for CAN_Init in peak dll" );
			FreeLibrary( m_hinst );
			m_hinst = NULL;
			return false;
		}
	}
	else {
	
		if ( NULL == ( m_procInit = 
						(LPFNDLL_INIT)GetProcAddress( m_hinst, "CAN_Init" ) ) ) {
			setLastError( CANAL_ERROR_PROCADDRESS, GetLastError(), "Unable get procaddress for CAN_Init in peak dll" );
			FreeLibrary( m_hinst );
			m_hinst = NULL;
			return false;
		}

	}

 
	if ( NULL == ( m_procClose = 
						(LPFNDLL_CLOSE)GetProcAddress( m_hinst, "CAN_Close" ) ) ) {
		setLastError( CANAL_ERROR_PROCADDRESS, GetLastError(), "Unable get procaddress for CAN_Close in peak dll" );
		FreeLibrary( m_hinst );
		m_hinst = NULL;
		return false;
	}


	if ( NULL == ( m_procStatus = 
						(LPFNDLL_STATUS)GetProcAddress( m_hinst, "CAN_Status" ) ) ) {
		setLastError( CANAL_ERROR_PROCADDRESS, GetLastError(), "Unable get procaddress for CAN_Status in peak dll" );
		FreeLibrary( m_hinst );
		m_hinst = NULL;
		return false;
	}

	if ( NULL == ( m_procWrite = 
						(LPFNDLL_WRITE)GetProcAddress( m_hinst, "CAN_Write" ) ) ) {
		setLastError( CANAL_ERROR_PROCADDRESS, GetLastError(), "Unable get procaddress for CAN_Write in peak dll" );
		FreeLibrary( m_hinst );
		m_hinst = NULL;
		return false;
	}

	if ( NULL == ( m_procRead = 
						(LPFNDLL_READ)GetProcAddress( m_hinst, "CAN_Read" ) ) ) {
		setLastError( CANAL_ERROR_PROCADDRESS, GetLastError(), "Unable get procaddress for CAN_Read in peak dll" );
		FreeLibrary( m_hinst );
		m_hinst = NULL;
		return false;
	}

	if ( NULL == ( m_procVersionInfo = 
						(LPFNDLL_VERSIONINFO)GetProcAddress( m_hinst, "CAN_VersionInfo" ) ) ) {
		setLastError( CANAL_ERROR_PROCADDRESS, GetLastError(), "Unable get procaddress for CAN_VersionInfo in peak dll" );
		FreeLibrary( m_hinst );
		m_hinst = NULL;
		return false;
	}

	// Mark as initialized
	m_bInit = true;

	return true;
	
}


///////////////////////////////////////////////////////////////////////////////
// workThreadTransmit
//
//

#ifdef WIN32
void workThreadTransmit( void *pObject )
#else
void *workThreadTransmit( void *pObject )
#endif
{
#ifdef WIN32
	DWORD errorCode = 0;
#else
	int rv = 0;
#endif

	CPeakObj * pobj = ( CPeakObj *)pObject;
	if ( NULL == pobj ) {
#ifdef WIN32	
		ExitThread( errorCode ); // Fail
#else
		pthread_exit( &rv );
#endif
	}
	
	while ( pobj->m_bRun ) {

		LOCK_MUTEX( pobj->m_peakMutex );
		
		// Noting to do if we should end...
		if ( !pobj->m_bRun ) continue;

		// Is there something to transmit...
		int ret;
		while ( ( NULL != pobj->m_transmitList.pHead ) && 
				( NULL != pobj->m_transmitList.pHead->pObject ) ) {

			canalMsg msg;
			memcpy( &msg, pobj->m_transmitList.pHead->pObject, sizeof( canalMsg ) ); 
			LOCK_MUTEX( pobj->m_transmitMutex );
			dll_removeNode( &pobj->m_transmitList, pobj->m_transmitList.pHead );
			UNLOCK_MUTEX( pobj->m_transmitMutex );

			PeakCanMsg peakMsg;

			peakMsg.id = msg.id;
			peakMsg.msgType = 0;
			peakMsg.len = msg.sizeData;
			memcpy( peakMsg.data, msg.data, peakMsg.len );

			// Check if RTR
			if ( ( msg.flags & CANAL_IDFLAG_RTR ) ) {
				peakMsg.msgType |= PCAN_MSGTYPE_RTR;
			}

			// Check if extended
			if ( ( msg.flags & CANAL_IDFLAG_EXTENDED ) ) {
				peakMsg.msgType |= PCAN_MSGTYPE_EXTENDED;
			}

			if ( PEAK_CAN_ERR_OK == ( ret = pobj->m_procWrite( &peakMsg ) ) ) {
				
				// Message sent successfully
				// Update statistics
				pobj->m_stat.cntTransmitData += msg.sizeData;
				pobj->m_stat.cntTransmitFrames += 1;


			}
			else {
					
				// Failed - put message back in queue front
				PCANALMSG pMsg	= new canalMsg;
				if ( NULL != pMsg ) {
						
					// Copy in data
					memcpy ( pMsg, &msg, sizeof( canalMsg ) );

					dllnode *pNode = new dllnode; 
					if ( NULL != pNode ) {
																
						pNode->pObject = pMsg;
						LOCK_MUTEX( pobj->m_transmitMutex );
						dll_addNodeHead( &pobj->m_transmitList, pNode );
						UNLOCK_MUTEX( pobj->m_transmitMutex );

					}
					else {

						delete pMsg;
					}

				} // unable to allocate storage

			} // faild to send message							

		} // while data


		// No data to write

		UNLOCK_MUTEX( pobj->m_peakMutex );
		SLEEP( 1 );

		//}	 
	
	} // while 	 


#ifdef WIN32
	ExitThread( errorCode );
#else
	pthread_exit( &rv );
#endif

}


///////////////////////////////////////////////////////////////////////////////
// workThreadReceive
//
//

#ifdef WIN32
void workThreadReceive( void *pObject )
#else
void *workThreadReceive( void *pObject )
#endif
{

#ifdef WIN32
	DWORD errorCode = 0;
#else
	int rv = 0;
#endif

	CPeakObj * pobj = ( CPeakObj *)pObject;
	if ( NULL == pobj ) {
#ifdef WIN32	
		ExitThread( errorCode ); // Fail
#else
		pthread_exit( &rv );
#endif
	}
	
	PeakCanMsg peakMsg;

	while ( pobj->m_bRun ) {
		
		// Noting to do if we should end...
		if ( !pobj->m_bRun ) continue;

		LOCK_MUTEX( pobj->m_peakMutex );
		while ( 0 == ( pobj->m_procRead( &peakMsg ) & PEAK_CAN_ERR_QRCVEMPTY ) ) {

			// Check if this is a status message
			if ( PCAN_MSGTYPE_STATUS & peakMsg.msgType ) {
				
				continue; // TODO

			}			
			
			// Write to the receive buffer
			if (  pobj->m_receiveList.nCount < PEAKDRV_MAX_RCVMSG ) {					
					
				PCANALMSG pMsg	= new canalMsg;
				pMsg->flags = 0;

				if ( NULL != pMsg ) {
						
					dllnode *pNode = new dllnode; 
					if ( NULL != pNode ) {
							
						pMsg->timestamp = GetTickCount() * 1000;
						pMsg->id = peakMsg.id;
						pMsg->sizeData = peakMsg.len;		
						memcpy( pMsg->data, peakMsg.data, pMsg->sizeData ); 
									
						// If extended set extended flag
						if ( PCAN_MSGTYPE_EXTENDED & peakMsg.msgType ) {
							pMsg->flags |= CANAL_IDFLAG_EXTENDED;
						}

						// Check for RTS package
						if ( PCAN_MSGTYPE_RTR & peakMsg.msgType ) {
							pMsg->flags |= CANAL_IDFLAG_RTR;
						}
													
						pNode->pObject = pMsg;
						LOCK_MUTEX( pobj->m_receiveMutex );
						dll_addNode( &pobj->m_receiveList, pNode );
						UNLOCK_MUTEX( pobj->m_receiveMutex );

						// Update statistics
						pobj->m_stat.cntReceiveData += pMsg->sizeData;
						pobj->m_stat.cntReceiveFrames += 1;

					}
					else {

						delete pMsg;

					}
				}				
			} 
			else {
				// Full buffer
				pobj->m_stat.cntOverruns++;	
			}
		} // while rcv msg
				
		
		UNLOCK_MUTEX( pobj->m_peakMutex );
		SLEEP( 1 );
	
	} // while 	 


#ifdef WIN32
	ExitThread( errorCode );
#else
	pthread_exit( &rv );
#endif

}
