// tellstickobj.cpp:  
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

#include "stdio.h"
#include "wx/wxprec.h"
#include "wx/wx.h"
#include "wx/defs.h"
#include "wx/app.h"
#include <wx/xml/xml.h>
#include <wx/listimpl.cpp>
#include <wx/thread.h>

#include "tellstickobj.h"
#include "dlldrvobj.h"

// The following define should be uncommented to turn of device A1
// when the driver is opened
//#define DEBUG_NEXA

// Lists
WX_DEFINE_LIST( CANALMSGLIST );
WX_DEFINE_LIST( TELLSTICKDEVICELIST );
WX_DEFINE_LIST( TELLSTICKEVENTLIST );

//#define _CRT_SECURE_NO_WARNINGS   1  // Get rid of string warnings
//#define _CRT_SECURE_NO_DEPRECATE  1

//////////////////////////////////////////////////////////////////////
// CTellStickDevice
//////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////
// Constructor
//

CTellStickDevice::CTellStickDevice()
{
	m_protocol = TELLSTICK_PROTOCOL_NEXA;
	m_ikea_systemcode = 1;
	m_ikea_devicecode = 1;
	m_dimlevel = 255;		// use event dimlevel
	m_bNiceDim = true;
	m_houseCode = 'A';
	m_channelCode = 1;
	m_bTurnOn = false;		// off as default
	m_nodeid = 1;			// Default nodeid = 1
}

//////////////////////////////////////////////////////////////////////
// Destructor
//

CTellStickDevice::~CTellStickDevice()
{
	;
}





//////////////////////////////////////////////////////////////////////
// CTellStickDevice
//////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////
// Constructor
//

CEventTranslation::CEventTranslation()
{
	m_vscp_class = VSCP_CLASS1_CONTROL;
	m_vscp_type = VSCP_TYPE_CONTROL_TURNOFF;
	m_zone = 0;
	m_subzone = 0;
}

//////////////////////////////////////////////////////////////////////
// Destructor
//

CEventTranslation::~CEventTranslation()
{
	// remove all Tellstick devices
	m_deviceList.DeleteContents ( true );
}


//////////////////////////////////////////////////////////////////////
// addNexaDevice
//

void CEventTranslation::addNexaDevice( const char houseCode,
										uint8_t channelCode,
										uint8_t dimlevel,
										bool bTurnOn )
{
	CTellStickDevice *pDevice = new CTellStickDevice;
	if ( NULL != pDevice ) {
		pDevice->m_protocol = TELLSTICK_PROTOCOL_NEXA;
		pDevice->m_houseCode = houseCode;
		pDevice->m_channelCode = channelCode;
		pDevice->m_dimlevel = dimlevel;
		pDevice->m_bTurnOn= bTurnOn;

		m_deviceList.Append( pDevice );
	}
}

//////////////////////////////////////////////////////////////////////
// addWavemanDevice
//

void CEventTranslation::addWavemanDevice( const char houseCode,
											                      uint8_t channelCode,
											                      uint8_t dimlevel,
											                      bool bTurnOn )
{
	CTellStickDevice *pDevice = new CTellStickDevice;
	if ( NULL != pDevice ) {

		pDevice->m_protocol = TELLSTICK_PROTOCOL_WAVEMAN;
		pDevice->m_houseCode = houseCode;
		pDevice->m_channelCode = channelCode;
		pDevice->m_dimlevel = dimlevel;
		pDevice->m_bTurnOn= bTurnOn;

		m_deviceList.Append( pDevice );
	}
}

//////////////////////////////////////////////////////////////////////
// addSartanoDevice
//

void CEventTranslation::addSartanoDevice( uint8_t channelCode,
											bool bTurnOn )
{
	CTellStickDevice *pDevice = new CTellStickDevice;
	if ( NULL != pDevice ) {
		pDevice->m_protocol = TELLSTICK_PROTOCOL_SARTANO;
		pDevice->m_channelCode = channelCode;
		pDevice->m_bTurnOn= bTurnOn;

		m_deviceList.Append( pDevice );
	}
}

//////////////////////////////////////////////////////////////////////
// addIkeaDevice
//

void CEventTranslation::addIkeaDevice( uint8_t systemCode,
										                    uint8_t deviceCode,
										                    uint8_t dimLevel,
										                    bool bNiceDim )
{
	CTellStickDevice *pDevice = new CTellStickDevice;
	if ( NULL != pDevice ) {
		pDevice->m_protocol = TELLSTICK_PROTOCOL_IKEA;
		pDevice->m_ikea_systemcode = systemCode;
		pDevice->m_ikea_devicecode = deviceCode;
		pDevice->m_dimlevel = dimLevel;
		pDevice->m_bNiceDim = bNiceDim;

		m_deviceList.Append( pDevice );
	}
}



//////////////////////////////////////////////////////////////////////
// CTellstickObj
//////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////
// CTellstickObj
//

CTellstickObj::CTellstickObj()
{ 
	m_initFlag = 0;
	
	// No filter mask
	m_filter = 0;
	m_mask = 0;

	// Send replies
	m_bSendReplyEvents = true;

	m_bRun = false;

	dll_init( &m_receiveList, SORT_NONE );
	
}

//////////////////////////////////////////////////////////////////////
// ~CTellstickObj
//

CTellstickObj::~CTellstickObj()
{		 
	close();

	// Delete events in event list
	m_eventList.DeleteContents ( true );

	// Delete events in transmitex list
	m_txMutex.Lock();

	CANALMSGLIST::iterator iter;
	for ( iter = m_txList.begin();
			iter != m_txList.end();
			++iter ) {

		canalMsg *pMsg = *iter;
		if( NULL != pMsg ) {
			delete pMsg;
		}
	}

	m_txList.Clear();

	m_txList.DeleteContents ( true );

	m_receiveMutex.Lock();
	dll_removeAllNodes( &m_receiveList );

}




//////////////////////////////////////////////////////////////////////
// open
//
//
// filename
//-----------------------------------------------------------------------------
// Parameters for the driver as a string on the following form
//		
// "device-id;path-to-config"
//
// device-id
// ======
// Serial number for channel
//
// path-to-config
// ==============
// This is the path to the XML config file. If absent the driver will search for the 
// file tellstick.xml in vscp folder the current users application data folder.
//
// 
// flags 
//-----------------------------------------------------------------------------
// Not used.
//
// 

bool CTellstickObj::open( const char *szDeviceString, unsigned long flags )
{
	const char *p;
	char szDrvParams[ MAX_PATH ];
	m_initFlag = flags;

	// save parameter string and convert to upper case
	strncpy( szDrvParams, szDeviceString, MAX_PATH );
	_strupr( szDrvParams );

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
		
	// serial
	p = strtok( szDrvParams, ";" );
	if ( NULL != p ) {		
		m_SerialNumber = p;	
	}

	// Path to configuration file
	p = strtok( NULL, ";" );
	if ( NULL != p ) {	
		wxString path = wxString::FromAscii( p );
		parseConfiguration( path );
	}

	FT_STATUS ftStatus;
	ftStatus = FT_OpenEx( (PVOID) (const char *)m_SerialNumber.c_str(), 
							FT_OPEN_BY_SERIAL_NUMBER, 
							&m_ftHandle );
	if ( !FT_SUCCESS( ftStatus ) ) return false;
		
	ftStatus = FT_ResetDevice( m_ftHandle );
	ftStatus = FT_Purge( m_ftHandle, FT_PURGE_RX | FT_PURGE_TX );
	ftStatus = FT_SetTimeouts(m_ftHandle, 378, 128);
	ftStatus = FT_SetUSBParameters ( m_ftHandle, 2048, 2048 );
	ftStatus = FT_SetLatencyTimer( m_ftHandle, 3 );
    
#ifdef DEBUG_NEXA	
	// Test Nexa
	char buf[ 512 ];
	createNexaString( "A", "1", false, false, buf );
#endif

	// Run run run ..... 

	m_bRun = true;
	
	// Release the mutex
	m_receiveMutex.Unlock();

	return true;
}

 
//////////////////////////////////////////////////////////////////////
// close
//

bool CTellstickObj::close( void )
{	
	// Do nothing if already terminated
	if ( !m_bRun ) return false;
	
	m_bRun = false;

	FT_Close( m_ftHandle );
 


	return true;
}


//////////////////////////////////////////////////////////////////////
// doFilter
//

bool CTellstickObj::doFilter( canalMsg *pcanalMsg )
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

bool CTellstickObj::setFilter( unsigned long filter )
{
	m_filter = filter;
	return true;
}


//////////////////////////////////////////////////////////////////////
// setMask
//

bool CTellstickObj::setMask( unsigned long mask )
{
	m_mask = mask;
	return true;
}



//////////////////////////////////////////////////////////////////////
// writeMsg
//

bool CTellstickObj::writeMsg( canalMsg *pMsg )
{	
	// Check pointer
	if ( NULL == pMsg ) {
		return false;
	}

	m_txMutex.Lock();
	m_txList.Append( pMsg );
	m_txMutex.Unlock();
	
	CTellstickWrkTread *pThread = new CTellstickWrkTread();
	wxASSERT( pThread );

	// Let the thread access our info
	pThread->m_pCtrlObject = this;

	if ( wxTHREAD_NO_ERROR == pThread->Create() ) {
        //pThread->SetPriority( WXTHREAD_DEFAULT_PRIORITY );
		if ( wxTHREAD_NO_ERROR != pThread->Run() ) {
			return false;
        }
	}
	else {
		return false;
	}

	return true;

}



//////////////////////////////////////////////////////////////////////
// readMsg
//

bool CTellstickObj::readMsg( canalMsg *pMsg )
{
	bool rv = false;
	
	if ( ( NULL != m_receiveList.pHead ) && 
			( NULL != m_receiveList.pHead->pObject ) ) {
		
		memcpy( pMsg, m_receiveList.pHead->pObject, sizeof( canalMsg ) );
		m_receiveMutex.Lock();
		dll_removeNode( &m_receiveList, m_receiveList.pHead );
		m_receiveMutex.Unlock();

		rv = true;
	}

	return rv;
}

//////////////////////////////////////////////////////////////////////
// readMsgBlock
//

bool CTellstickObj::readMsgBlock( canalMsg *pMsg, unsigned long timeout )
{
	bool rv = false;

	// Don't allow block for ever.
	if ( 0 == timeout ) timeout = 100;

	if ( ( NULL == m_receiveList.pHead ) ) {
		if ( wxSEMA_TIMEOUT == m_receiveSemaphore.WaitTimeout( timeout ) ) {
			return false;	
		}
	}

	m_receiveMutex.Lock();
	
	if ( ( NULL != m_receiveList.pHead ) && 
			( NULL != m_receiveList.pHead->pObject ) ) {
		
		memcpy( pMsg, m_receiveList.pHead->pObject, sizeof( canalMsg ) );
		m_receiveMutex.Lock();
		dll_removeNode( &m_receiveList, m_receiveList.pHead );
		m_receiveMutex.Unlock();

		rv = true;
	}

	m_receiveMutex.Unlock();

	return rv;
}


///////////////////////////////////////////////////////////////////////////////
//	dataAvailable
//

int CTellstickObj::dataAvailable( void )
{
	int cnt;
	
	m_receiveMutex.Lock();
	cnt = dll_getNodeCount( &m_receiveList );	
	m_receiveMutex.Unlock();
 
	return cnt;
}


///////////////////////////////////////////////////////////////////////////////
//	getStatistics
//

bool CTellstickObj::getStatistics( PCANALSTATISTICS pCanalStatistics )
{	
	// Must be a valid pointer
	if ( NULL == pCanalStatistics ) return false;

	memcpy( pCanalStatistics, &m_stat, sizeof( canalStatistics ) );

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//	getStatus
//

bool CTellstickObj::getStatus( PCANALSTATUS pCanalStatus )
{
	// Must be a valid pointer
	if ( NULL == pCanalStatus ) return false;

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//	CanalGetDriverInfo
//

char * CTellstickObj::CanalGetDriverInfo( void )
{
	return TELLSTICK_DRIVERINFO;
}


///////////////////////////////////////////////////////////////////////////////
// parseConfiguration
//
//

bool CTellstickObj::parseConfiguration( wxString& path )
{
    wxXmlDocument doc;
    if ( !doc.Load ( path ) )
    {
        return false;
    }

	// start processing the XML file
    if ( doc.GetRoot()->GetName() != wxT ( "tellstick" ) ) {
        return false;
    }

    wxXmlNode *child = doc.GetRoot()->GetChildren();
    while ( child ) {
	
		if ( child->GetName() == wxT( "event" ) ) {

			CEventTranslation *pEvent = new CEventTranslation;
			wxASSERT( NULL != pEvent );

			wxString strType = child->GetAttribute( wxT ( "type" ), wxT ( "off" ) );
			
			strType = strType.MakeUpper();
			if ( wxNOT_FOUND != strType.find( _("ON") ) ) {
				pEvent->m_vscp_class = VSCP_CLASS1_CONTROL;
				pEvent->m_vscp_type = VSCP_TYPE_CONTROL_TURNON;
			}
			else if ( wxNOT_FOUND != strType.find( _("ALL") ) ) {
				pEvent->m_vscp_class = VSCP_CLASS1_CONTROL;
				pEvent->m_vscp_type = VSCP_TYPE_CONTROL_ALL_LAMPS;
			}
			else if ( wxNOT_FOUND != strType.find( _("DIM") ) ) {
				pEvent->m_vscp_class = VSCP_CLASS1_CONTROL;
				pEvent->m_vscp_type = VSCP_TYPE_CONTROL_DIM_LAMPS;
			}
			else { // OFF is also default
				pEvent->m_vscp_class = VSCP_CLASS1_CONTROL;
				pEvent->m_vscp_type = VSCP_TYPE_CONTROL_TURNOFF;
			}

			pEvent->m_zone = vscp_readStringValue( child->GetAttribute ( wxT ( "zone" ), wxT ( "0" ) ) );
			pEvent->m_subzone = vscp_readStringValue( child->GetAttribute ( wxT ( "subzone" ), wxT ( "0" ) ) );

			// Add the event
			m_eventList.Append( pEvent );
		
			wxXmlNode *subchild = child->GetChildren();
			while ( subchild ) {

				if ( subchild->GetName() == wxT( "device" ) ) {

					CTellStickDevice *pDevice = new CTellStickDevice;
					wxASSERT( NULL != pDevice );

					pDevice->m_nodeid = vscp_readStringValue( subchild->GetAttribute( wxT( "usenodeid" ), wxT( "1" ) ) );
					
					wxString strProtocol = subchild->GetAttribute ( wxT( "protocol" ), wxT( "NEXA" ) );
					strProtocol = strProtocol.MakeUpper();
					if ( wxNOT_FOUND != strProtocol.find( _("WAVEMAN") ) ) {
						pDevice->m_protocol = TELLSTICK_PROTOCOL_WAVEMAN;
						pDevice->m_houseCode = subchild->GetAttribute ( wxT( "housecode" ), wxT( "A" ) );
						pDevice->m_channelCode  = vscp_readStringValue( subchild->GetAttribute ( wxT( "channelcode" ), wxT( "1" ) ) );
						pDevice->m_dimlevel = vscp_readStringValue( subchild->GetAttribute ( wxT( "dimlevel" ), wxT( "0" ) ) );
						pDevice->m_bTurnOn = 
							vscp_readStringValue( subchild->GetAttribute( wxT( "state" ), wxT( "0" ) ) ) ? true : false;
					}	
					else if ( wxNOT_FOUND != strProtocol.find( _("SARTANO") ) ) {
						pDevice->m_protocol = TELLSTICK_PROTOCOL_SARTANO;
						pDevice->m_strChannel = subchild->GetAttribute ( wxT( "channelcode" ), wxT( "0" ) );
						pDevice->m_bTurnOn = 
							vscp_readStringValue( subchild->GetAttribute( wxT( "state" ), wxT( "0" ) ) ) ? true : false;
					}
					else if ( wxNOT_FOUND != strProtocol.find( _("IKEA") ) ) {
						pDevice->m_protocol = TELLSTICK_PROTOCOL_IKEA;
						pDevice->m_ikea_systemcode = vscp_readStringValue( subchild->GetAttribute( wxT( "systemcode" ), wxT( "0" ) ) );
						pDevice->m_ikea_devicecode = vscp_readStringValue( subchild->GetAttribute( wxT( "devicecode" ), wxT(  "0" ) ) );
						pDevice->m_dimlevel = vscp_readStringValue( subchild->GetAttribute ( wxT( "dimlevel" ), wxT( "0" ) ) );
						pDevice->m_bNiceDim = 
							( vscp_readStringValue( subchild->GetAttribute ( wxT( "dimstyle" ), wxT( "0" ) ) ) ? true : false );
					}
					else { // NEXA is default
						pDevice->m_protocol = TELLSTICK_PROTOCOL_NEXA;
						pDevice->m_houseCode = subchild->GetAttribute ( wxT( "housecode" ), wxT ( "A" ) );
						pDevice->m_channelCode  = vscp_readStringValue( subchild->GetAttribute ( wxT( "channelcode" ), wxT ( "1" ) ) );
						pDevice->m_dimlevel = vscp_readStringValue( subchild->GetAttribute ( wxT( "dimlevel" ), wxT ( "0" ) ) );
						pDevice->m_bTurnOn = 
							vscp_readStringValue( subchild->GetAttribute ( wxT( "state" ), wxT( "0" ) ) ) ? true : false;
					}

					// Add the device
					pEvent->m_deviceList.Append( pDevice );

				}

				subchild = subchild->GetNext();
			
			}
		}
		else if ( child->GetName() == wxT( "replymsg" ) ) {
			wxString strReplyMsg = child->GetAttribute ( wxT( "send" ), wxT( "true" ) );
			strReplyMsg = strReplyMsg.MakeUpper();
			if ( wxNOT_FOUND != strReplyMsg.find( _("FALSE") ) ) {
				m_bSendReplyEvents = false;
			}
			else {
				m_bSendReplyEvents = true;
			}
		}

		child = child->GetNext();
	}

	return true;
}



///////////////////////////////////////////////////////////////////////////////
//                             Worker thread
///////////////////////////////////////////////////////////////////////////////


CTellstickWrkTread::CTellstickWrkTread()
		: wxThread( wxTHREAD_DETACHED )
{
    m_bQuit = false;
    m_pCtrlObject = NULL;
}


CTellstickWrkTread::~CTellstickWrkTread()
{
	;
}

///////////////////////////////////////////////////////////////////////////////
// Entry
//

void *CTellstickWrkTread::Entry()
{
	canalMsg *pMsg;

	// Must have a valid pointer to the control object
    if ( NULL == m_pCtrlObject ) return NULL;

	m_pCtrlObject->m_tellstickMutex.Lock();

	m_pCtrlObject->m_txMutex.Lock();
	pMsg = m_pCtrlObject->m_txList.GetFirst()->GetData();
	m_pCtrlObject->m_txMutex.Unlock();

	// If no data nothing to do
	if ( NULL == pMsg ) return NULL;

	if ( VSCP_CLASS1_CONTROL == vscp_getVSCPclassFromCANALid( pMsg->id  ) ) {
	
		if ( VSCP_TYPE_CONTROL_TURNON == vscp_getVSCPtypeFromCANALid( pMsg->id ) ) {
			sendDeviceCommands( pMsg, VSCP_TYPE_INFORMATION_ON );
		}
		else if ( VSCP_TYPE_CONTROL_TURNOFF == vscp_getVSCPtypeFromCANALid( pMsg->id ) ) {
			sendDeviceCommands( pMsg, VSCP_TYPE_INFORMATION_OFF );
		}
		else if ( VSCP_TYPE_CONTROL_ALL_LAMPS == vscp_getVSCPtypeFromCANALid( pMsg->id ) ) {
			if ( pMsg->data[ 0 ] ) {
				sendDeviceCommands( pMsg, VSCP_TYPE_INFORMATION_ON );
			}
			else {
				sendDeviceCommands( pMsg, VSCP_TYPE_INFORMATION_OFF );
			}
		}
		else if ( VSCP_TYPE_CONTROL_DIM_LAMPS == vscp_getVSCPtypeFromCANALid( pMsg->id ) ) {
			
		}
	}

	m_pCtrlObject->m_txList.DeleteNode( m_pCtrlObject->m_txList.GetFirst() );

	m_pCtrlObject->m_tellstickMutex.Unlock();

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// OnExit
//

void CTellstickWrkTread::OnExit()
{

}


///////////////////////////////////////////////////////////////////////////////
// createNexaString
//
//

int CTellstickWrkTread::createNexaString( const char *pHouseStr, 
										uint8_t channelCode,
										bool bTurnOn, 
										bool bWaveman,
										char *pTxStr )
{
	*pTxStr = '\0'; // Make sure tx string is empty 
	uint8_t houseCode;
	uint8_t on_offCode;
	int txCode = 0;
	const int unknownCode = 0x6;
	int bit;
	int bitmask = 0x0001;

	houseCode = (int)( ( *pHouseStr ) - 65 );	// House 'A'..'P' 
	//channelCode = atoi( pChannelStr ) - 1;		// Channel 1..16 
	channelCode--;								// Channel 1..16

#ifdef RFCMD_DEBUG
	printf("House: %d, channel: %d, on_off: %d\n", houseCode, channelCode, on_offCode);
#endif

	// check converted parameters for validity 
	if ( ( houseCode < 0) || ( houseCode > 15 ) ||      // House 'A'..'P'
	   ( channelCode < 0) || ( channelCode > 15 ) ) {
		;
	} 
	else {
		
		// b0..b11 txCode where 'X' will be represented by 1 for simplicity.
		//   b0 will be sent first 
		txCode = houseCode;
		txCode |= ( channelCode << 4 );
		
		if ( bWaveman && !bTurnOn ) {
			;
		} 
		else {
			txCode |= ( unknownCode <<  8 );
			txCode |= ((  bTurnOn ? 1 : 0 )  << 11 );
		}

		// convert to send cmd string 
		strcat( pTxStr, "S" );
		for ( bit=0; bit<12; bit++ ) {
			
			if ( ( bitmask & txCode) == 0 ) {
				// bit timing might need further refinement 
				strcat(pTxStr," ` `"); // 320 us high, 960 us low,  320 us high, 960 us low 
				// strcat(pTxStr,"$k$k"); *//* 360 us high, 1070 us low,  360 us high, 1070 us low 
			} 
			else { // add 'X' (floating bit) 
				strcat(pTxStr," `` "); // 320 us high, 960 us low, 960 us high,  320 us low 
				//strcat(pTxStr,"$kk$"); *//* 360 us high, 1070 us low, 1070 us high,  360 us low 
			}

			bitmask = bitmask << 1;

		}

		// add stop/sync bit and command termination char '+'
		strcat( pTxStr, " }+" );
		
		// strcat(pTxStr,"$}+"); 
	}

#ifdef RFCMD_DEBUG
	printf("txCode: %04X\n", txCode);
#endif

#ifdef DEBUG_NEXA
	DWORD len;
	FT_Write( m_ftHandle, pTxStr, strlen( pTxStr ), &len );
#endif

	return strlen( pTxStr );
}

///////////////////////////////////////////////////////////////////////////////
// createSartanoString
//
//

int CTellstickWrkTread::createSartanoString( const char *pChannelStr, 
											bool bTurnOn,
											char *pTxStr )
{
	*pTxStr = '\0';						// Make sure tx string is empty 
	int on_offCode;
	int bit;

	on_offCode =  bTurnOn ? 1 : 0;	// ON/OFF 0..1

#ifdef RFCMD_DEBUG
	printf("Channel: %s, on_off: %d\n", pChannelStr, on_offCode);
#endif

	// check converted parameters for validity 
	if ( ( strlen( pChannelStr ) != 10 ) ||
			( on_offCode < 0 ) || 
			( on_offCode > 1 ) ) {
		;
	} 
	else {
		
		strcat(pTxStr,"S");
		
		for ( bit=0; bit<=9; bit++ ) {

			if ( strncmp( pChannelStr + bit, "1", 1 ) == 0 ) { // If it is a "1"
				strcat( pTxStr, "$k$k" );
			} 
			else {
				strcat(pTxStr,"$kk$");
			}
		}

		if ( on_offCode >= 1 ) {
			strcat( pTxStr, "$k$k$kk$" );	// the "turn on"-code
		}
		else {
			strcat( pTxStr, "$kk$$k$k" );	//the "turn off"-code
		}

		// add stop/sync bit and command termination char '+'
		strcat( pTxStr, "$k+" );
	}

#ifdef RFCMD_DEBUG
	printf("txCode: %04X\n", txCode);
#endif

	return strlen( pTxStr );
}

///////////////////////////////////////////////////////////////////////////////
// createIkeaString
//
//

int CTellstickWrkTread::createIkeaString( int systemCode, 
										int channelCode, 
										int dimLevel, 
										bool bNiceDim, 
										char *pStrReturn )
{
	*pStrReturn = '\0';						// Make sure tx string is empty 

	const char STARTCODE[] = "STTTTTT\\AA";
	const char TT[]  = "TT";
	const char A[]   = "\\AA";
	systemCode--;							// System 1..16
	int DimStyle = bNiceDim ? 1 : 0;
	int intCode      = 0;
	int checksum1    = 0;
	int checksum2    = 0;
	int intFade ;
	int i ;
	int rawChannelCode = 0;

	// check converted parameters for validity 
	if ( ( channelCode <= 0 ) || 
			( channelCode > 10 ) ||
			( systemCode < 0 ) || 
			( systemCode > 15 ) ||
			( dimLevel < 0) || 
			( dimLevel > 10 ) ||
			( DimStyle < 0 ) || 
			( DimStyle > 1 ) ) {
		return 0;
	}

	if ( channelCode == 10 ) {
		channelCode = 0;
	}

	rawChannelCode = ( 1 << ( 9 - channelCode ) );

	strcat( pStrReturn, STARTCODE ) ;		// Startcode, always like this;
	intCode = ( systemCode << 10 ) | rawChannelCode;

	for ( i = 13; i >= 0; --i) {
		if ( ( intCode >> i ) & 1 ) {
			strcat(pStrReturn, TT );
			if ( i % 2 == 0 ) {
				checksum2++;
			} 
			else {
				checksum1++;
			}
		} 
		else {
			strcat( pStrReturn, A );
		}
	}

	if ( ( checksum1 % 2 ) == 0) {
		strcat(pStrReturn, TT );
	} 
	else {
		strcat( pStrReturn, A ) ;			// 1st checksum
	}

	if ( ( checksum2 % 2 ) == 0) {
		strcat( pStrReturn, TT );
	} 
	else {
		strcat( pStrReturn, A ) ;			// 2nd checksum
	}

	if ( DimStyle == 1 ) {
		intFade = 11 << 4; // Smooth
	} 
	else {
		intFade = 1 << 4; // Instant
	}

	switch ( dimLevel )
	{
		case 0 :
			intCode = (10 | intFade) ;		// Concat level and fade
			break;

		case 1 :
			intCode = (1 | intFade) ;		// Concat level and fade
			break;
		
		case 2 :
			intCode = (2 | intFade) ;		// Concat level and fade
			break;
		
		case 3 :
			intCode = (3 | intFade) ;		// Concat level and fade
			break;
		
		case 4 :
			intCode = (4 | intFade) ;		// Concat level and fade
			break;
		
		case 5 :
			intCode = (5 | intFade) ;		// Concat level and fade
			break;
		
		case 6 :
			intCode = (6 | intFade) ;		// Concat level and fade
			break;
		
		case 7 :
			intCode = (7 | intFade) ;		// Concat level and fade
			break;
		
		case 8 :
			intCode = (8 | intFade) ;		// Concat level and fade
			break;
		
		case 9 :
			intCode = (9 | intFade) ;		// Concat level and fade
			break;
		
		case 10 :
		default :
			intCode = (0 | intFade) ;		// Concat level and fade
			break;
	}

	checksum1 = 0;
	checksum2 = 0;

	for ( i = 0; i < 6; ++i ) {
		if ( ( intCode >> i ) & 1 ) {
			strcat( pStrReturn, TT );

			if ( ( i % 2 ) == 0 ) {
				checksum1++;
			} 
			else {
				checksum2++;
			}
		} 
		else {
			strcat( pStrReturn, A );
		}
	}

	if ( ( checksum1 % 2 ) == 0 ) {
		strcat( pStrReturn, TT );
	} 
	else {
		strcat( pStrReturn, A );			// 2nd checksum
	}

	if ( ( checksum2 % 2 ) == 0 ) {
		strcat( pStrReturn, TT );
	} 
	else {
		strcat( pStrReturn, A );			// 2nd checksum
	}

	strcat( pStrReturn, "+" );

	return strlen( pStrReturn );
}


//////////////////////////////////////////////////////////////////////
// sendDeviceCommands
//

void CTellstickWrkTread::sendDeviceCommands( canalMsg *pMsg, 
												uint8_t replyType,
												bool bDim,
												uint8_t dimlevel )
{
	// Go through the event list and look for matching records
	TELLSTICKEVENTLIST::iterator iter;

	for ( iter = m_pCtrlObject->m_eventList.begin();
			iter != m_pCtrlObject->m_eventList.end();
			++iter ) {

		CEventTranslation *pEvent = *iter;
		wxASSERT( NULL != pEvent );

		if ( 0 == pMsg->sizeData ) {
			continue;
		}

		if ( vscp_getVSCPclassFromCANALid( pMsg->id  ) == pEvent->m_vscp_class &&
				vscp_getVSCPtypeFromCANALid( pMsg->id ) == pEvent->m_vscp_type &&
				( ( pMsg->data[1] == pEvent->m_zone ) || ( 255 == pEvent->m_zone ) ) &&
				( ( pMsg->data[1] == pEvent->m_subzone ) || ( 255 == pEvent->m_subzone ) ) ) {
					
			// We have a match - handle the device list
			TELLSTICKDEVICELIST::iterator iterDevice;

			for ( iterDevice = pEvent->m_deviceList.begin(); 
					iterDevice != pEvent->m_deviceList.end(); 
					++iterDevice ) {

				CTellStickDevice *pDevice = *iterDevice;
				wxASSERT( NULL != pDevice );

				char buf[ 128 ];
				DWORD len;

				if ( TELLSTICK_PROTOCOL_NEXA == pDevice->m_protocol ) {

					if ( createNexaString( pDevice->m_houseCode, 
												pDevice->m_channelCode, 
												pDevice->m_bTurnOn, 
												false,	// nexa
												buf ) ) {
					
						if ( FT_OK == FT_Write( m_pCtrlObject->m_ftHandle, buf, strlen( buf ), &len ) ) {
									
							// Success
							if ( m_pCtrlObject->m_bSendReplyEvents ) {
										
								replyEvent( pEvent,
												pDevice,
												VSCP_CLASS1_INFORMATION,
												replyType );
							}
						}
						else {
							if ( m_pCtrlObject->m_bSendReplyEvents ) {

								replyError( pEvent,
												pDevice,
												TELLSTICK_ERROR_WRITE_DEVICE );

							}
						}
					}
					else {
						if ( m_pCtrlObject->m_bSendReplyEvents ) {

							replyError( pEvent,
											pDevice,
											TELLSTICK_ERROR_NEXA_COMMAND );

						}
					} // create device string
				}
				else if ( TELLSTICK_PROTOCOL_WAVEMAN == pDevice->m_protocol ) {
					
					if ( createNexaString( pDevice->m_houseCode, 
							pDevice->m_channelCode, 
							pDevice->m_bTurnOn, 
							true,	// waveman 
							buf ) ) {

						if ( FT_OK == FT_Write( m_pCtrlObject->m_ftHandle, buf, strlen( buf ), &len ) ) {

							// Success
							if ( m_pCtrlObject->m_bSendReplyEvents ) {
										
								replyEvent( pEvent,
												pDevice,
												VSCP_CLASS1_INFORMATION,
												replyType );
							}
						}
						else {
							if ( m_pCtrlObject->m_bSendReplyEvents ) {

								replyError( pEvent,
												pDevice,
												TELLSTICK_ERROR_WRITE_DEVICE );

							}
						}
					}
					else {
						if ( m_pCtrlObject->m_bSendReplyEvents ) {

							replyError( pEvent,
											pDevice,
											TELLSTICK_ERROR_NEXA_COMMAND );

						}
					} // create device string
				}
				else if ( TELLSTICK_PROTOCOL_SARTANO == pDevice->m_protocol ) {

					if ( createSartanoString( pDevice->m_strChannel, 
							pDevice->m_bTurnOn,
							buf ) ) {

						if ( FT_OK == FT_Write( m_pCtrlObject->m_ftHandle, buf, strlen( buf ), &len ) ) {

							// Success
							if ( m_pCtrlObject->m_bSendReplyEvents ) {
										
								replyEvent( pEvent,
												pDevice,
												VSCP_CLASS1_INFORMATION,
												replyType );
							}
						}
						else {
							if ( m_pCtrlObject->m_bSendReplyEvents ) {

								replyError( pEvent,
												pDevice,
												TELLSTICK_ERROR_WRITE_DEVICE );

							}
						}
					}
					else {
						if ( m_pCtrlObject->m_bSendReplyEvents ) {

							replyError( pEvent,
											pDevice,
											TELLSTICK_ERROR_NEXA_COMMAND );

						}
					} // create device string
				}
				else if ( TELLSTICK_PROTOCOL_IKEA == pDevice->m_protocol ) {

					if ( createIkeaString( pDevice->m_ikea_systemcode, 
							pDevice->m_ikea_devicecode, 
							( ( 255 == pMsg->data[ 0 ] ) ? pDevice->m_dimlevel : dimlevel ), 
							pDevice->m_bNiceDim, 
							buf ) ) {

						if ( FT_OK == FT_Write( m_pCtrlObject->m_ftHandle, buf, strlen( buf ), &len ) ) {

							// Success
							if ( m_pCtrlObject->m_bSendReplyEvents ) {
										
								replyEvent( pEvent,
												pDevice,
												VSCP_CLASS1_INFORMATION,
												replyType,
												pMsg->data[ 0 ] );
							}
						}
						else {
							if ( m_pCtrlObject->m_bSendReplyEvents ) {

								replyError( pEvent,
												pDevice,
												TELLSTICK_ERROR_WRITE_DEVICE );

							}
						}
					}
					else {
						if ( m_pCtrlObject->m_bSendReplyEvents ) {

							replyError( pEvent,
											pDevice,
											TELLSTICK_ERROR_NEXA_COMMAND );

						}
					} // create device string
				} // protocol select
			} // iterate through devices
		} // event we should react on
	} // iterate through events
}


//////////////////////////////////////////////////////////////////////
// replyEvent
//

void CTellstickWrkTread::replyEvent( CEventTranslation *pEvent,
										CTellStickDevice *pDevice,
										uint16_t vscp_class,
										uint8_t vscp_type,
										uint8_t dimLevel )
{
	vscpEventEx eventEx;

	eventEx.vscp_class = vscp_class;
	eventEx.vscp_type = vscp_type;
	eventEx.sizeData = 3;
	eventEx.data[ 0 ] = dimLevel;
	eventEx.data[ 1 ] = pEvent->m_zone;
	eventEx.data[ 2 ] = pEvent->m_subzone;

	memset( eventEx.GUID, 0, 16 );
	eventEx.GUID[ 0 ] = pDevice->m_nodeid;

	canalMsg *pcanalMsg = new canalMsg;
	wxASSERT( NULL != pcanalMsg );
	vscp_convertEventExToCanal( pcanalMsg, &eventEx );

	// Add the event to the out queue
	dllnode *pNode = new dllnode;
	wxASSERT( NULL != pNode );

	pNode->pObject = pcanalMsg;
	pNode->pKey = NULL;
	pNode->pstrKey = NULL;
	m_pCtrlObject->m_receiveMutex.Lock();
	dll_addNode( &m_pCtrlObject->m_receiveList, pNode );
	m_pCtrlObject->m_receiveMutex.Unlock();
}

//////////////////////////////////////////////////////////////////////
// replyError
//

void CTellstickWrkTread::replyError( CEventTranslation *pTellstickEvent,
										CTellStickDevice *pDevice,
										uint8_t errorCode )
{
	vscpEventEx eventEx;

	eventEx.vscp_class = VSCP_CLASS1_INFORMATION;
	eventEx.vscp_type = VSCP_TYPE_INFORMATION_ERROR;
	eventEx.sizeData = 3;
	eventEx.data[ 0 ] = TELLSTICK_ERROR_NEXA_COMMAND;
	eventEx.data[ 1 ] = pTellstickEvent->m_zone;
	eventEx.data[ 2 ] = pTellstickEvent->m_subzone;

	memset( eventEx.GUID, 0, 16 );
	eventEx.GUID[ 0 ] = pDevice->m_nodeid;

	canalMsg *pcanalMsg = new canalMsg;
	wxASSERT( NULL != pcanalMsg );

	vscp_convertEventExToCanal( pcanalMsg, &eventEx );

	// Add the event to the out queue
	dllnode *pNode = new dllnode;
	wxASSERT( NULL != pNode );

	pNode->pObject = pcanalMsg;
	pNode->pKey = NULL;
	pNode->pstrKey = NULL;

	m_pCtrlObject->m_receiveMutex.Lock();
	dll_addNode( &m_pCtrlObject->m_receiveList, pNode );
	m_pCtrlObject->m_receiveMutex.Unlock();
}
