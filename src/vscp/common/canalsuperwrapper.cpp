///////////////////////////////////////////////////////////////////////////////
// CanalSuperWrapper.cpp: implementation of the CCanalSuperWrapper class.
//
// This file is part is part of CANAL (CAN Abstraction Layer)
// http://www.vscp.org)
//
// Copyright (C) 2000-2012 Ake Hedman, Grodans Paradis AB, <akhe@grodansparadis.com>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
///////////////////////////////////////////////////////////////////////////////

#include "wx/wx.h"
#include "wx/defs.h"
#include <wx/progdlg.h>

#include "vscp.h"
#include "canalsuperwrapper.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCanalSuperWrapper::CCanalSuperWrapper( void )
{	
	init();

	m_itemDevice.id = USE_TCPIP_INTERFACE;
	m_itemDevice.strName = _("Locahost");
	m_itemDevice.strPath = _("TCPIP");
	m_itemDevice.strParameters = _("admin;secret;localhost;9598");
	m_itemDevice.flags = 0; 
	m_itemDevice.filter = 0;
	m_itemDevice.mask = 0;
	m_devid = 0;
}

CCanalSuperWrapper::CCanalSuperWrapper( devItem *pItem )
{	
	if ( NULL == pItem ) return;

	init();

	if ( pItem->strPath.Length() && pItem->strPath.IsSameAs(_("TCPIP") ) ) {

		// TCP/IP interface
		m_itemDevice.id = USE_TCPIP_INTERFACE;

		m_itemDevice.strName = pItem->strName;
		m_itemDevice.strPath = pItem->strPath;
		m_itemDevice.strParameters = pItem->strParameters;
		m_itemDevice.flags = pItem->flags;
		m_itemDevice.filter = pItem->filter;
		m_itemDevice.mask = pItem->mask;
	}
	else if ( NULL != pItem )  {

		// DLL/DL Interface
		m_itemDevice.id = USE_DLL_INTERFACE;

		if ( pItem->strName.Length() ) {
			m_itemDevice.strName = pItem->strName;
		}

		if ( pItem->strPath.Length() ) {
			m_itemDevice.strPath = pItem->strPath;
		}

		if ( pItem->strParameters.Length() ) {
			m_itemDevice.strParameters = pItem->strParameters;
		}	

		m_itemDevice.flags = pItem->flags;
		m_itemDevice.filter = pItem->filter;
		m_itemDevice.mask = pItem->mask;

		m_canalDll.initialize( m_itemDevice.strPath );

	}
	else {

		// pItem == NULL
		// We do some default

		// TCP/IP interface
		m_itemDevice.id = USE_TCPIP_INTERFACE;
		m_itemDevice.strName = _("Locahost");
		m_itemDevice.strPath = _("TCPIP");
		m_itemDevice.strParameters = _("admin;secret;localhost;9598");
		m_itemDevice.flags = 0;
		m_itemDevice.filter = 0;
		m_itemDevice.mask = 0;

	}

}

CCanalSuperWrapper::~CCanalSuperWrapper()
{
	doCmdClose();	
}

///////////////////////////////////////////////////////////////////////////////
// init
//  

void CCanalSuperWrapper::init( void )
{
	// Init. register read parameters
	m_registerReadErrorTimeout = VSCP_REGISTER_READ_ERROR_TIMEOUT;
	m_registerReadResendTimeout = VSCP_REGISTER_READ_RESEND_TIMEOUT;
	m_registerReadMaxRetries = VSCP_REGISTER_READ_MAX_TRIES;
}

///////////////////////////////////////////////////////////////////////////////
// setInterface
//  TCP/IP version

void CCanalSuperWrapper::setInterface( const wxString& host, 
	const short port,
	const wxString& username,
	const wxString& password )
{
	// TCP/IP interface
	m_itemDevice.id = USE_TCPIP_INTERFACE;

	// No name
	m_itemDevice.strName.Empty();

	// No path
	m_itemDevice.strPath.Empty();

	// Build TCP/IP configuration string
	m_itemDevice.strParameters = username;
	m_itemDevice.strParameters += _(";"); 
	m_itemDevice.strParameters += password;
	m_itemDevice.strParameters += _(";");
	m_itemDevice.strParameters += host;
	m_itemDevice.strParameters += _(";");
	m_itemDevice.strParameters += wxString::Format( _("%d"), port );

	m_itemDevice.flags = 0;
	m_itemDevice.filter = 0;
	m_itemDevice.mask = 0;
}

///////////////////////////////////////////////////////////////////////////////
// setInterface - CANAL version
//  

void CCanalSuperWrapper::setInterface( const wxString& name,
	const wxString& path,
	const wxString& parameters,
	const unsigned long flags,
	const unsigned long filter,
	const unsigned long mask)
{
	// DLL/DL Interface
	m_itemDevice.id = USE_DLL_INTERFACE;

	m_itemDevice.strName = name;
	m_itemDevice.strPath = path;
	m_itemDevice.strParameters = parameters;

	m_itemDevice.flags = flags;
	m_itemDevice.filter = filter;
	m_itemDevice.mask = mask;

	m_canalDll.initialize( m_itemDevice.strPath );
}

///////////////////////////////////////////////////////////////////////////////
// open
//

long CCanalSuperWrapper::doCmdOpen( const wxString& strInterface, unsigned long flags )
{
	long rv = false;

	if ( USE_DLL_INTERFACE == m_itemDevice.id ) {

		// *** Open dll/dl interface ***

		if ( strInterface.Length() ) {
			rv = m_canalDll.doCmdOpen( strInterface, flags );
		}
		else {
			rv = m_canalDll.doCmdOpen( m_itemDevice.strParameters, 
				m_itemDevice.flags );
		}   

	}
	else if ( USE_TCPIP_INTERFACE == m_itemDevice.id) {

		// *** Open remote TCP/IP interface *** 

		if ( strInterface.Length() ) {
			rv = m_vscptcpif.doCmdOpen( strInterface, flags );
		}
		else {
			rv = m_vscptcpif.doCmdOpen( m_itemDevice.strParameters, m_itemDevice.flags );
			if ( CANAL_ERROR_SUCCESS == rv ) {
				// We try to get the interface GUID. If we
				// fail to get it we use the GUID assigned
				// in the constructor
				m_vscptcpif.doCmdGetGUID( (char *)m_GUID.m_id );
			}

		}

	}

	m_devid = rv;

	return rv;
}


///////////////////////////////////////////////////////////////////////////////
// close
//

int CCanalSuperWrapper::doCmdClose( void )
{
	m_devid = 0;

	if ( USE_DLL_INTERFACE == m_itemDevice.id) {
		return m_canalDll.doCmdClose();
	}
	else if ( USE_TCPIP_INTERFACE == m_itemDevice.id) {
		return m_vscptcpif.doCmdClose();
	}

	return CANAL_ERROR_NOT_SUPPORTED;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdNOOP
//

int CCanalSuperWrapper::doCmdNOOP( void )
{
	if ( USE_DLL_INTERFACE == m_itemDevice.id) {
		return m_canalDll.doCmdNOOP();
	}
	else if ( USE_TCPIP_INTERFACE == m_itemDevice.id) {
		return m_vscptcpif.doCmdNOOP();
	}

	return CANAL_ERROR_NOT_SUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdClear
//

int CCanalSuperWrapper::doCmdClear( void )
{
	if ( USE_DLL_INTERFACE == m_itemDevice.id) {
		canalMsg *pMsg = NULL;
		while ( doCmdDataAvailable() ) {
			m_canalDll.doCmdReceive( pMsg );
			delete pMsg;
			pMsg = NULL;
		}
		return CANAL_RESPONSE_SUCCESS;
	}
	else if ( USE_TCPIP_INTERFACE == m_itemDevice.id) {
		return m_vscptcpif.doCmdClear();
	}

	return CANAL_ERROR_NOT_SUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdGetLevel
//

unsigned long CCanalSuperWrapper::doCmdGetLevel( void )
{ 
	if ( USE_DLL_INTERFACE == m_itemDevice.id) {
		return m_canalDll.doCmdGetLevel();
	}
	else if ( USE_TCPIP_INTERFACE == m_itemDevice.id) {
		return CANAL_LEVEL_USES_TCPIP; 
	}

	return CANAL_ERROR_NOT_SUPPORTED;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdSend
//

int CCanalSuperWrapper::doCmdSend( canalMsg *pMsg )
{	
	if ( USE_DLL_INTERFACE == m_itemDevice.id) {
		return m_canalDll.doCmdSend( pMsg );
	}
	else if ( USE_TCPIP_INTERFACE == m_itemDevice.id) {
		return m_vscptcpif.doCmdSendLevel1( pMsg );
	}

	return CANAL_ERROR_NOT_SUPPORTED;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdSend
//

int CCanalSuperWrapper::doCmdSend( const vscpEvent *pEvent )
{	
	canalMsg msg;

	if ( USE_DLL_INTERFACE == m_itemDevice.id ) {
		convertEventToCanal( &msg, pEvent );
		return doCmdSend( &msg );
	}
	else if ( USE_TCPIP_INTERFACE == m_itemDevice.id ) {
		return m_vscptcpif.doCmdSend( pEvent );
	}

	return CANAL_ERROR_NOT_SUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdSend
//

int CCanalSuperWrapper::doCmdSend( const vscpEventEx *pEventEx )
{	
	int rv;

	if ( USE_DLL_INTERFACE == m_itemDevice.id ) {
		vscpEvent *pevent = new vscpEvent;
		convertVSCPfromEx( pevent, pEventEx );
		rv = doCmdSend( pevent );
		deleteVSCPevent( pevent );
		return rv;
	}
	else if ( USE_TCPIP_INTERFACE == m_itemDevice.id ) {
		return m_vscptcpif.doCmdSendEx( pEventEx );
	}

	return CANAL_ERROR_NOT_SUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdReceive
//

int CCanalSuperWrapper::doCmdReceive( canalMsg *pMsg )
{		
	if ( USE_DLL_INTERFACE == m_itemDevice.id) {
		return m_canalDll.doCmdReceive( pMsg );
	}
	else if ( USE_TCPIP_INTERFACE == m_itemDevice.id) {
		return m_vscptcpif.doCmdReceiveLevel1( pMsg );
	}

	return CANAL_ERROR_NOT_SUPPORTED;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdReceive
//

int CCanalSuperWrapper::doCmdReceive( vscpEvent *pEvent )
{	
	return m_vscptcpif.doCmdReceive( pEvent );
}


///////////////////////////////////////////////////////////////////////////////
// doCmdReceive
//

int CCanalSuperWrapper::doCmdReceive( vscpEventEx *pEventEx )
{	
	return m_vscptcpif.doCmdReceiveEx( pEventEx );
}


///////////////////////////////////////////////////////////////////////////////
// doCmdDataAvailable
//

int CCanalSuperWrapper::doCmdDataAvailable( void )
{
	if ( USE_DLL_INTERFACE == m_itemDevice.id ) {
		return m_canalDll.doCmdDataAvailable();
	}
	else if ( USE_TCPIP_INTERFACE == m_itemDevice.id ) {
		return m_vscptcpif.doCmdDataAvailable();
	}

	return CANAL_ERROR_NOT_SUPPORTED;
}


///////////////////////////////////////////////////////////////////////////////
// doCmdState
//

int CCanalSuperWrapper::doCmdStatus( canalStatus *pStatus )
{	
	if ( USE_DLL_INTERFACE == m_itemDevice.id) {
		return m_canalDll.doCmdStatus( pStatus );
	}
	else if ( USE_TCPIP_INTERFACE == m_itemDevice.id) {
		return m_vscptcpif.doCmdStatus( pStatus );
	}

	return CANAL_ERROR_NOT_SUPPORTED;
}



///////////////////////////////////////////////////////////////////////////////
// doCmdStatistics
//

int CCanalSuperWrapper::doCmdStatistics( canalStatistics *pStatistics )
{	
	if ( USE_DLL_INTERFACE == m_itemDevice.id) {
		return m_canalDll.doCmdStatistics( pStatistics );
	}
	else if ( USE_TCPIP_INTERFACE == m_itemDevice.id) {
		return m_vscptcpif.doCmdStatistics( pStatistics );
	}

	return CANAL_ERROR_NOT_SUPPORTED;
}



///////////////////////////////////////////////////////////////////////////////
// doCmdFilter
//

int CCanalSuperWrapper::doCmdFilter( unsigned long filter )
{	
	if ( USE_DLL_INTERFACE == m_itemDevice.id) {
		return m_canalDll.doCmdFilter( filter );
	}
	else if ( USE_TCPIP_INTERFACE == m_itemDevice.id) {
		return CANAL_ERROR_NOT_SUPPORTED;
	}

	return CANAL_ERROR_NOT_SUPPORTED;

}



///////////////////////////////////////////////////////////////////////////////
// doCmdMask
//

int CCanalSuperWrapper::doCmdMask( unsigned long mask )
{	
	if ( USE_DLL_INTERFACE == m_itemDevice.id) {
		return m_canalDll.doCmdMask( mask );
	}
	else if ( USE_TCPIP_INTERFACE == m_itemDevice.id) {
		return CANAL_ERROR_NOT_SUPPORTED;
	}

	return CANAL_ERROR_NOT_SUPPORTED;

}

///////////////////////////////////////////////////////////////////////////////
// doCmdVscpFilter
//

int CCanalSuperWrapper::doCmdVscpFilter( const vscpEventFilter *pFilter )
{
	if ( USE_DLL_INTERFACE == m_itemDevice.id) {
		return CANAL_ERROR_NOT_SUPPORTED;
	}
	else if ( USE_TCPIP_INTERFACE == m_itemDevice.id) {
		return m_vscptcpif.doCmdFilter( pFilter );
	}

	return CANAL_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdBaudrate

int CCanalSuperWrapper::doCmdBaudrate( unsigned long baudrate )
{
	if ( USE_DLL_INTERFACE == m_itemDevice.id) {
		return m_canalDll.doCmdBaudrate( baudrate );
	}
	else if ( USE_TCPIP_INTERFACE == m_itemDevice.id) {
		return CANAL_ERROR_SUCCESS;
	}

	return CANAL_ERROR_NOT_SUPPORTED;
}



///////////////////////////////////////////////////////////////////////////////
// doCmdVersion
//

unsigned long CCanalSuperWrapper::doCmdVersion( void )
{
	if ( USE_DLL_INTERFACE == m_itemDevice.id) {
		return m_canalDll.doCmdVersion();
	}
	else if ( USE_TCPIP_INTERFACE == m_itemDevice.id) {
		return m_vscptcpif.doCmdVersion();
	}

	return 0;
}



///////////////////////////////////////////////////////////////////////////////
// doCmdDLLVersion
//

unsigned long CCanalSuperWrapper::doCmdDLLVersion( void )
{
	if ( USE_DLL_INTERFACE == m_itemDevice.id) {
		return m_canalDll.doCmdDLLVersion();
	}
	else if ( USE_TCPIP_INTERFACE == m_itemDevice.id) {
		return m_vscptcpif.doCmdDLLVersion();
	}

	return 0;
}



///////////////////////////////////////////////////////////////////////////////
// doCmdVendorString
//

const char * CCanalSuperWrapper::doCmdVendorString( void )
{
	if ( USE_DLL_INTERFACE == m_itemDevice.id ) {
		return m_canalDll.doCmdVendorString();
	}
	else if ( USE_TCPIP_INTERFACE == m_itemDevice.id ) {
		return m_vscptcpif.doCmdVendorString();
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdShutDown
//

int CCanalSuperWrapper::doCmdShutDown( void )
{
	if ( USE_TCPIP_INTERFACE == m_itemDevice.id) {
		return m_vscptcpif.doCmdShutDown();
	}

	return CANAL_ERROR_NOT_SUPPORTED;
}

///////////////////////////////////////////////////////////////////////////////
// doCmdGetDriverInfo
//

const char * CCanalSuperWrapper::doCmdGetDriverInfo( void )
{
	if ( USE_DLL_INTERFACE == m_itemDevice.id ) {
		return m_canalDll.doCmdGetDriverInfo();
	}
	else if ( USE_TCPIP_INTERFACE == m_itemDevice.id ) {
		return m_vscptcpif.doCmdGetDriverInfo();
	}

	return NULL;
}




//****************************************************************************
//					   R e g i s t e r  r o u t i n e s
//****************************************************************************


// We don't want the graphcal UI on apps that don't use it 
#if ( wxUSE_GUI != 0 )


//////////////////////////////////////////////////////////////////////////////
// readLevel1Register
//

bool CCanalSuperWrapper::readLevel1Register( uint8_t nodeid, 
	uint8_t reg, 
	uint8_t *pcontent )
{
	bool rv = true;
	uint32_t errors = 0;
	bool bResend;
	wxString strBuf;
	canalMsg canalEvent;

	// Check pointer
	if ( NULL == pcontent ) return false;

	canalEvent.flags = CANAL_IDFLAG_EXTENDED;
	canalEvent.obid = 0;
	canalEvent.id = 0x0900;             // CLASS1.PROTOCOL Read register
	canalEvent.sizeData = 2;
	canalEvent.data[ 0 ] = nodeid;      // Node to read from
	canalEvent.data[ 1 ] = reg;         // Register to read

	bResend = false;
	doCmdSend( &canalEvent );

	//wxDateTime start = wxDateTime::Now();
	wxLongLong startTime = ::wxGetLocalTimeMillis();

	while ( true ) {

		if ( doCmdDataAvailable() ) {									// Message available
			if ( CANAL_ERROR_SUCCESS == doCmdReceive( &canalEvent ) ) {	// Valid event
				if ( (unsigned short)( canalEvent.id & 0xffff ) ==
					( 0x0a00 + nodeid ) ) {                 // Read reply?
						if ( canalEvent.data[ 0 ] == reg ) {// Requested register?

							if ( NULL != pcontent ) {
								*pcontent = canalEvent.data[ 1 ];
							}
							break;

						}	// Check for correct node
				}			// Check for correct reply event 
			}
		}
		else {
			wxMilliSleep( 1 );
		}

		// Check for read error timeout
		if ( ( ::wxGetLocalTimeMillis() - startTime ) > 
			m_registerReadErrorTimeout ) {
				errors++;
		}
		// Should we resend?
		else if ( ( ::wxGetLocalTimeMillis() - startTime ) > 
			m_registerReadResendTimeout ) {
				// Send again
				if ( !bResend) {
					doCmdSend( &canalEvent );
				}
				bResend = true;
		}

		if ( errors > m_registerReadMaxRetries ) {
			rv = false;
			break;
		}

	} // while

	return rv;
}


//////////////////////////////////////////////////////////////////////////////
// writeLevel1Register
//

bool CCanalSuperWrapper::writeLevel1Register( uint8_t nodeid, 
	uint8_t reg, 
	uint8_t *pval )
{
	bool rv = true;
	uint32_t errors = 0;
	bool bResend;
	wxString strBuf;
	canalMsg canalEvent;

	canalEvent.flags = CANAL_IDFLAG_EXTENDED;
	canalEvent.obid = 0;
	canalEvent.id = 0x0B00;             // CLASS1.PROTOCOL Write register
	canalEvent.sizeData = 3;
	canalEvent.data[ 0 ] = nodeid;      // Node to read from
	canalEvent.data[ 1 ] = reg;         // Register to write
	canalEvent.data[ 2 ] = *pval;			// value to write

	bResend = false;
	doCmdSend( &canalEvent );

	wxLongLong startTime = ::wxGetLocalTimeMillis();

	while ( true ) {

		if ( doCmdDataAvailable() ) {									// Message available
			if ( CANAL_ERROR_SUCCESS == doCmdReceive( &canalEvent ) ) { // Valid event
				if ( (unsigned short)( canalEvent.id & 0xffff ) ==
					( 0x0a00 + nodeid ) ) {         // Read reply?
						if ( canalEvent.data[ 0 ] == reg ) {			// Requested register?

							if ( *pval != canalEvent.data[ 1 ] ) rv = false;
							// Save read value
							*pval = canalEvent.data[ 1 ];
							break;
						} // Check for correct node

						// Save read value
						*pval = canalEvent.data[ 1 ];

				} // Check for correct reply event 
			}
		}
		else {
			wxMilliSleep( 1 );
		}

		if ( ( ::wxGetLocalTimeMillis() - startTime ) > 
			m_registerReadErrorTimeout ) {
				errors++;
		}
		else if ( ( ::wxGetLocalTimeMillis() - startTime ) > 
			m_registerReadResendTimeout ) {
				// Send again
				if ( !bResend) {
					doCmdSend( &canalEvent );
				}
				bResend = true;
		}

		if ( errors > m_registerReadMaxRetries ) {
			rv = false;
			break;
		}

	} // while

	return rv;
}

//////////////////////////////////////////////////////////////////////////////
// readLevel2Register
//

bool CCanalSuperWrapper::readLevel2Register( uint8_t *interfaceGUID, 
	uint32_t reg, 
	uint8_t *pcontent,
	uint8_t *pdestGUID,
	bool bLevel2 )
{
	int i;
	bool rv = true;
	bool bInterface = false;  // No specific interface set
	uint32_t errors = 0;
	bool bResend;
	wxString strBuf;
	vscpEventEx event;

	// Check if a specific interface is used
	for ( i=0; i<16; i++ ) {
		if ( interfaceGUID[ i ] ) {
			bInterface= true;
			break;
		}
	}

	if ( bInterface ) {

		// Event should be sent to a specific interface

		event.head = VSCP_PRIORITY_NORMAL;
		event.vscp_class = VSCP_CLASS2_LEVEL1_PROTOCOL;
		event.vscp_type = VSCP_TYPE_PROTOCOL_READ_REGISTER;

		memset( event.GUID, 0, 16 );                // We use GUID for interface 

		event.sizeData = 16 + 2;                    // Interface GUID + nodeid + register to read

		for ( i=0; i<16; i++ ) {
			event.data[ i ] = interfaceGUID[ 15 - i ];	
		}

		event.data[16] = interfaceGUID[0];          // nodeid
		event.data[17] = reg;                       // Register to read

	}

	else {

		// Event should be sent to all interfaces

		// Must have a destination GUID
		if ( NULL == pdestGUID ) return false;

		if ( bLevel2 ) {

			// True real Level II event

			event.head = VSCP_PRIORITY_NORMAL;
			event.vscp_class = VSCP_CLASS2_PROTOCOL;
			event.vscp_type = VSCP2_TYPE_PROTOCOL_READ_REGISTER;

			memset( event.GUID, 0, 16 );		// We use GUID for interface 

			event.sizeData = 22;				// nodeid + register to read

			for ( i=0; i<16; i++ ) {			// Destination GUID
				event.data[ i ] = pdestGUID[ 15 - i ];	
			}	
			event.data[ 16 ] = 0x00;			// Register to read
			event.data[ 17 ] = 0x00;
			event.data[ 18 ] = 0x00;
			event.data[ 19 ] = reg;
			event.data[ 20 ] = 0x00;			// Read one register
			event.data[ 21 ] = 0x01;

		}
		else {

			// Level I over CLASS2 to all interfaces 

			event.head = VSCP_PRIORITY_NORMAL;
			event.vscp_class = VSCP_CLASS2_LEVEL1_PROTOCOL;
			event.vscp_type = VSCP_TYPE_PROTOCOL_READ_REGISTER;

			memset( event.GUID, 0, 16 );				// We use GUID for interface 

			event.sizeData = 16 + 2;					// nodeid + register to read

			for ( i=0; i<16; i++ ) {
				event.data[ i ] = pdestGUID[ 15 - i ];	
			}

			event.data[16] = 0x00;						// nodeid
			event.data[17] = reg;                       // Register to read

		}
	}

	bResend = false;

	// Send the event
	doCmdClear();
	event.timestamp = 0;
	doCmdSend( &event );


	//wxDateTime start = wxDateTime::Now();
	wxLongLong startTime = ::wxGetLocalTimeMillis();

	while ( true ) {

		if ( doCmdDataAvailable() ) {								// Message available
			if ( CANAL_ERROR_SUCCESS == doCmdReceive( &event ) ) {	// Valid event

				// Check for correct reply event

				// Level I Read reply?
				if ( bInterface && ( VSCP_CLASS1_PROTOCOL == event.vscp_class ) && 
					( VSCP_TYPE_PROTOCOL_RW_RESPONSE == event.vscp_type ) ) {   
						if ( event.data[ 0 ] == reg ) {                         // Requested register?
							if ( event.GUID[0] == interfaceGUID[0] ) {          // Correct node?
								if ( NULL != pcontent ) {
									*pcontent = event.data[ 1 ];
									break;
								}
								break;
							}
						} // Check for correct node
				}
				// Level II 512 Read reply?
				else if ( !bInterface && !bLevel2 && 
					( VSCP_CLASS2_LEVEL1_PROTOCOL == event.vscp_class ) && 
					( VSCP_TYPE_PROTOCOL_RW_RESPONSE == event.vscp_type ) ) { 

						if ( isSameGUID( pdestGUID, event.GUID ) ) {
							// Reg we requested?
							if ( event.data[ 0 ] == reg ) {
								// OK get the data
								if ( NULL != pcontent ) {
									*pcontent = event.data[ 18 ];
									break;
								}
							}
						}

				}
				// Level II Read reply?
				else if ( !bInterface && bLevel2 && 
					( VSCP_CLASS2_PROTOCOL == event.vscp_class ) && 
					( VSCP2_TYPE_PROTOCOL_READ_WRITE_RESPONSE == event.vscp_type ) ) { 

						// from us
						if ( isSameGUID( pdestGUID, event.GUID ) ) {	

							uint32_t retreg = ( event.data[ 0 ]  << 24 ) +
								(	event.data[ 1 ]  << 16 ) +
								(	event.data[ 2 ]  << 8 ) +
								event.data[ 3 ];

							// Reg we requested?
							if ( retreg == reg ) {
								// OK get the data
								if ( NULL != pcontent ) {
									*pcontent = event.data[ 18 ];
									break;
								}
							}
						}

				}
			} // valid event
		}
		else {
			//wxMilliSleep( 1 );
		}

		if ( ( ::wxGetLocalTimeMillis() - startTime ) >   
			m_registerReadErrorTimeout ) {
				errors++;
		}
		else if ( ( ::wxGetLocalTimeMillis() - startTime ) > 
			m_registerReadResendTimeout ) {
				// Send again
				if ( !bResend) {
					doCmdClear();
					event.timestamp = 0;
					doCmdSend( &event );
				}
				bResend = true;
		}   

		if ( errors > m_registerReadMaxRetries ) {
			rv = false;
			break;
		}

	} // while

	return rv;
}


//////////////////////////////////////////////////////////////////////////////
// writeLevel2Register
//

bool CCanalSuperWrapper::writeLevel2Register( uint8_t *interfaceGUID, 
	uint32_t reg, 
	uint8_t *pcontent,
	uint8_t *pdestGUID,
	bool bLevel2 )
{
	int i;
	bool rv = true;
	bool bInterface = false;  // No specific interface set
	uint32_t errors = 0;
	bool bResend;
	wxString strBuf;
	vscpEventEx event;

	// Check pointers
	if ( NULL == pcontent ) return false;

	// Check if a specific interface is used
	for ( i=0; i<16; i++ ) {
		if ( interfaceGUID[ i ] ) {
			bInterface= true;
			break;
		}
	}

	if ( bInterface ) {

		event.head = VSCP_PRIORITY_NORMAL;
		event.vscp_class = VSCP_CLASS2_LEVEL1_PROTOCOL;
		event.vscp_type = VSCP_TYPE_PROTOCOL_WRITE_REGISTER;

		memset( event.GUID, 0, 16 );              // We use interface GUID

		event.sizeData = 16 + 3;                  // Interface GUID + nodeid + register to read + valied

		for ( i=0; i<16; i++ ) {
			event.data[ i ] = interfaceGUID[ 15 - i ];	
		}
		event.data[16] = interfaceGUID[ 0 ];      // nodeid
		event.data[17] = reg;                     // Register to write
		event.data[18] = *pcontent;	              // value to write

	}
	else {

		if ( bLevel2 ) {

			// Must have a destination GUID
			if ( NULL == pdestGUID ) return false;

			event.head = VSCP_PRIORITY_NORMAL;
			event.vscp_class = VSCP_CLASS2_PROTOCOL;
			event.vscp_type = VSCP2_TYPE_PROTOCOL_WRITE_REGISTER;

			memset( event.GUID, 0, 16 );		// We use interface GUID

			event.sizeData = 21;				// nodeid + register to read

			for ( i=0; i<16; i++ ) {			// Destination GUID
				event.data[ i ] = pdestGUID[ 15 - i ];	
			}

			event.data[ 16 ] = 0x00;			// Register to write
			event.data[ 17 ] = 0x00;
			event.data[ 18 ] = 0x00;
			event.data[ 19 ] = reg;
			event.data[ 20 ] = *pcontent;		// Data to write

		}
		else {

			// Level I over CLASS2 to all interfaces 

			event.head = VSCP_PRIORITY_NORMAL;
			event.vscp_class = VSCP_CLASS2_LEVEL1_PROTOCOL;
			event.vscp_type = VSCP_TYPE_PROTOCOL_WRITE_REGISTER;

			memset( event.GUID, 0, 16 );			// We use interface GUID

			event.sizeData = 16 + 3;				

			for ( i=0; i<16; i++ ) {		
				event.data[ i ] = interfaceGUID[ 15 - i ];	
			}

			event.data[16] = interfaceGUID[0];        // nodeid
			event.data[17] = reg;                     // Register to write
			event.data[18] = *pcontent;	              // value to write

		}

	}

	bResend = false;
	doCmdSend( &event );

	wxLongLong startTime = ::wxGetLocalTimeMillis();

	while ( true ) {

		if ( doCmdDataAvailable() ) {                                 // Message available
			if ( CANAL_ERROR_SUCCESS == doCmdReceive( &event ) ) {    // Valid event
				if ( ( VSCP_CLASS1_PROTOCOL == event.vscp_class ) && 
					( VSCP_TYPE_PROTOCOL_RW_RESPONSE == event.vscp_type ) ) {   // Read reply?
						if ( event.data[ 0 ] == reg ) {                         // Requested register?
							if ( event.GUID[0] == interfaceGUID[0] ) {          // Correct node?

								// We go a rw reply from the correct node is
								// the written data same as we expect.
								if ( *pcontent != event.data[ 1 ] ) rv = false;
								break;

							}
						}   // Check for correct node
				}       // Check for correct reply event 
			}
		}
		else {
			wxMilliSleep( 1 );
		}

		if ( ( ::wxGetLocalTimeMillis() - startTime ) > 
			m_registerReadErrorTimeout ) {
				errors++;
		}
		else if ( ( ::wxGetLocalTimeMillis() - startTime ) > 
			m_registerReadResendTimeout ) {
				// Send again
				if ( !bResend) {
					doCmdSend( &event );
				}
				bResend = true;
		}

		if ( errors > m_registerReadMaxRetries ) {
			rv = false;
			break;
		}

	} // while

	return rv;
}


//////////////////////////////////////////////////////////////////////////////
// getMDFfromDevice1
//

wxString CCanalSuperWrapper::getMDFfromDevice1( uint8_t id, bool bSilent )
{
	wxString strWrk;
	char url[ 33 ];

	uint8_t *p = (uint8_t *)url;
	for ( int i=0; i<32; i++ ) {
		if ( !readLevel1Register( id, 
			0xE0 + i, 
			p++ ) ) {
				if ( !bSilent ) {												
					::wxMessageBox( _("Unable to read register."), _("VSCP Works"), wxICON_ERROR );
				}
				break;
		}
	}

	strWrk = strWrk.From8BitData( url );
	if ( wxNOT_FOUND == strWrk.Find( _("http://") ) ) {
		wxString str;
		str = _("http://");
		str += strWrk;
		strWrk = str;
	}

	return strWrk;
}

//////////////////////////////////////////////////////////////////////////////
// getMDFfromDevice2
//

wxString CCanalSuperWrapper::getMDFfromDevice2( uint8_t *pguid, 
	bool bLevel2, 
	bool bSilent )
{
	wxString strWrk;
	char url[ 33 ];

	memset( url, 0, sizeof( url ) );

	if ( bLevel2 ) {

		// Level 2 device
		uint8_t *p = (uint8_t *)url;
		for ( int i=0; i<32; i++ ) {
			if ( !readLevel1Register( *pguid, 
				0xE0 + i, 
				p++ ) ) {
					if ( !bSilent ) {												
						::wxMessageBox( _("Unable to read register."), _("VSCP Works"), wxICON_ERROR );
					}
					break;
			}

		}

	}
	else {

		// Level 1 device
		uint8_t *p = (uint8_t *)url;
		for ( int i=0; i<32; i++ ) {
			if ( !readLevel2Register( pguid, 
				0xFFFFFFE0 + i, 
				p++ ) ) {
					if ( !bSilent ) {
						::wxMessageBox( _("Unable to read register."), _("VSCP Works"), wxICON_ERROR );
					}
					break;
			}

		}

	}

	return strWrk;
}

//////////////////////////////////////////////////////////////////////////////
// getLevel1DmInfo
//

bool CCanalSuperWrapper::getLevel1DmInfo( const uint8_t nodeid, 
	uint8_t *pdata )
{
	bool rv = true;
	bool bResend;
	wxString strBuf;
	canalMsg canalEvent;

	// Check pointer
	if ( NULL == pdata ) return false;

	canalEvent.flags = CANAL_IDFLAG_EXTENDED;
	canalEvent.obid = 0;
	canalEvent.id = 0x2000;             // CLASS1.PROTOCOL Get decision matrix info
	canalEvent.sizeData = 1;
	canalEvent.data[ 0 ] = nodeid;      // Node to read from

	bResend = false;
	doCmdSend( &canalEvent );

	wxDateTime start = wxDateTime::Now();

	while ( true ) {

		if ( doCmdDataAvailable() ) {							// Message available
			if ( doCmdReceive( &canalEvent ) ) {				// Valid message
				if ( (unsigned short)( canalEvent.id & 0xffff ) ==
					( 0x2100 + nodeid ) ) {                 // DM info reply?
						// Copy in response data
						memcpy( pdata, canalEvent.data, 8 );
						break;
				}
			}
		}

		if ( (wxDateTime::Now() - start).GetSeconds() > 2 ) {
			rv = false;
			break;
		}
		else if ( (wxDateTime::Now() - start).GetSeconds() > 1 ) {
			// Send again
			if ( !bResend) {
				doCmdSend( &canalEvent );
			}
			bResend = true;
		}
	} // while

	return rv;
}


//////////////////////////////////////////////////////////////////////////////
// getLevel2DmInfo
//

bool CCanalSuperWrapper::getLevel2DmInfo( uint8_t *interfaceGUID, 
	uint8_t *pdata,
	bool bLevel2 )
{
	bool rv = true;
	bool bResend;
	wxString strBuf;
	vscpEventEx event;

	// Check pointer
	if ( NULL == pdata ) return false;

	event.head = 0;
	event.vscp_class = 512;                   // CLASS2.PROTOCOL1
	event.vscp_type = 9;                      // Get decision matrix info
	memset( event.GUID, 0, 16 );              // We use interface GUID
	event.sizeData = 16 + 1;                  // Interface GUID + nodeid 
	memcpy( event.data, interfaceGUID, 16 );  // Address node
	event.data[16] = interfaceGUID[0];        // nodeid

	bResend = false;
	doCmdSend( &event );

	wxDateTime start = wxDateTime::Now();

	while ( true ) {

		if ( doCmdDataAvailable() ) {									// Message available
			if ( CANAL_ERROR_SUCCESS == doCmdReceive( &event ) ) {		// Valid event
				if ( ( 0 == event.vscp_class ) && 
					( 0x21 == event.vscp_type ) ) {					// DM reply?
						memcpy( pdata, event.data, 8 );
						break;
				}
			}
		}

		if ( (wxDateTime::Now() - start).GetSeconds() > 2 ) {
			rv = false;
			break;
		}
		else if ( (wxDateTime::Now() - start).GetSeconds() > 1 ) {
			// Send again
			if ( !bResend) {
				doCmdSend( &event );
			}
			bResend = true;
		}
	} // while

	return rv;
}


//////////////////////////////////////////////////////////////////////////////
// readAllLevel1Registers
//

bool CCanalSuperWrapper::readLevel1Registers( wxWindow *pwnd,
	uint8_t *pregisters,
	uint8_t nodeid,
	uint8_t startreg,
	uint16_t count )
{
	int i;
	unsigned char val;
	bool rv = true;
	int errors = 0;
	wxString strBuf;

	wxProgressDialog progressDlg( _("VSCP Works"),
		_("Reading Registers"),
		256, 
		pwnd,
		wxPD_ELAPSED_TIME | 
		wxPD_AUTO_HIDE | 
		wxPD_APP_MODAL | 
		wxPD_CAN_ABORT );

	progressDlg.Pulse( _("Reading registers!") );


	// *********************
	// Read register content
	// *********************
	for ( i = startreg; i < (startreg + count); i++ ) {

		if ( !progressDlg.Update( i ) ) {
			rv = false;
			break;   // User aborted
		}

		progressDlg.Pulse( wxString::Format(_("Reading register %d"), i) );

		if ( readLevel1Register( nodeid, i, &val ) ) {

			pregisters[ i-startreg ] = val;

		}
		else {
			errors++;
		}

		if ( errors > 2 ) {
			wxMessageBox( _("No node present or problems when communicating with node. Aborting!") );
			rv = false;
			break;
		}

	} // for

	return rv;
}


//////////////////////////////////////////////////////////////////////////////
// readAllLevel2Registers
//

bool CCanalSuperWrapper::readLevel2Registers( wxWindow *pwnd,
	uint8_t *pregisters,
	uint8_t *pinterfaceGUID,
	uint32_t startreg,
	uint32_t count )
{
	uint32_t i;
	unsigned char val;
	bool rv = true;
	int errors = 0;
	wxString strBuf;

	wxProgressDialog progressDlg( _("VSCP Works"),
		_("Reading Registers"),
		256, 
		pwnd,
		wxPD_ELAPSED_TIME | 
		wxPD_AUTO_HIDE | 
		wxPD_APP_MODAL |
		wxPD_CAN_ABORT );

	progressDlg.Pulse( _("Reading registers!") );

	// *********************
	// Read register content
	// *********************
	for ( i = startreg; i < (startreg + count); i++ ) {

		if ( !progressDlg.Update( i ) ) {
			rv = false;
			break;
		}

		progressDlg.Pulse( wxString::Format(_("Reading register %d"), i) );

		if ( readLevel2Register( pinterfaceGUID, i, &val ) ) {

			pregisters[ i - startreg ] = val;

		}
		else {
			errors++;
		}


		if ( errors > 2 ) {
			wxMessageBox( _("No node present or problems when communicating with node. Aborting!") );
			rv = false;
			break;
		}

	} // for


	return rv;
}

//////////////////////////////////////////////////////////////////////////////
// setRegisterPage
//

bool CCanalSuperWrapper::setRegisterPage( uint8_t nodeid, 
	uint16_t page, 
	uint8_t *interfaceGUID )
{
	uint8_t val;

	if ( NULL == interfaceGUID ) {

		val = ( page >> 8 ) & 0xff;
		if ( !writeLevel1Register( nodeid, 
			0x92, 
			&val ) ) {
				return false;
		}

		val = page & 0xff;
		if ( !writeLevel1Register( nodeid, 
			0x93, 
			&val ) ) {
				return false;
		}

	}
	else {

		val = ( page >> 8 ) & 0xff;
		if ( writeLevel2Register( interfaceGUID, 
			0x92, 
			&val ) ) {
				return false;
		}

		val = page & 0xff;
		if ( writeLevel2Register( interfaceGUID, 
			0x93, 
			&val ) ) {
				return false;
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// getRegisterPage
//

uint32_t CCanalSuperWrapper::getRegisterPage( wxWindow *pwnd, uint8_t nodeid, uint8_t *interfaceGUID )
{
	uint32_t page = 0;

	if ( ( NULL == interfaceGUID ) || isGUIDEmpty( interfaceGUID ) ) {

		uint8_t regs[ 2 ];
		if ( readLevel1Registers( pwnd,
			regs,
			nodeid,
			0x92,
			2 ) ) {
				page = ( regs[ 0 ] << 8 ) + regs[ 1 ];
		}
		else {
			return false;
		}

	}
	else {
		uint8_t regs[ 2 ];
		if ( readLevel2Registers( pwnd,
			regs,
			interfaceGUID,
			0xffffff92,
			2 ) ) {
				page = ( regs[ 0 ] << 8 ) + regs[ 1 ];
		}
		else {
			return false;
		}

	}

	return page;

}


///////////////////////////////////////////////////////////////////////////////
//  getDMRow
// 

bool CCanalSuperWrapper::getDMRow( wxWindow *pwnd,
	uint8_t nodeid, 
	CMDF_DecisionMatrix *pdm, 
	uint32_t row, 
	uint8_t *pRow,
	bool bSilent )
{
	// True if the matrix is indexed. That is if it consist
	//	of one row precided by an index into the matrix.
	bool bIndexed = false;

	// Can't load row that is larger then the availabel rows
	if ( row >= pdm->m_nRowCount ) return false;

	// Check if this matrix is indexed
	if ( ( 1 == pdm->m_nLevel ) && ( 120 == pdm->m_nStartOffset ) ) {
		bIndexed = true;
	}	

	uint16_t savepage = getRegisterPage( pwnd, nodeid );
	if ( savepage != pdm->m_nStartPage ) {
		if ( !setRegisterPage( nodeid, pdm->m_nStartPage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to set register page for decision matrix!") );
			return false;
		}
	}

	// Check if this matrix is indexed if so select row
	if ( ( 1 == pdm->m_nLevel ) && ( 120 == pdm->m_nStartOffset ) ) {
		uint8_t val = row;
		if ( !writeLevel1Register( nodeid, 119, &val ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to set register index for decision matrix!") );
			return false;
		}
	}

	// Read row
	if ( !readLevel1Registers( pwnd, pRow, nodeid, 120, pdm->m_nRowSize ) ) {
		if ( !bSilent ) wxMessageBox( _("Unable to read decision matrix row!") );
		return false;
	}

	// Restore page
	if ( savepage != pdm->m_nStartPage ) {
		if ( !setRegisterPage( nodeid, savepage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to restore register page for decision matrix!") );
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//  getAbstractionString
// 

bool CCanalSuperWrapper::getAbstractionString( wxWindow *pwnd,
	uint8_t nodeid,
	CMDF_Abstraction *abstraction,
	wxString& retstr, 
	bool bSilent )
{
	wxString str;

	// Check pointers
	if ( NULL == abstraction) return false;

	uint16_t savepage = getRegisterPage( pwnd, nodeid );
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, abstraction->m_nPage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction string!") );
			return false;
		}
	}

	uint8_t *p;
	p = new uint8_t[ abstraction->m_nWidth + 1 ];
	memset( p, 0, abstraction->m_nWidth + 1 );

	if ( abstraction->m_bIndexed ) {

		for ( uint8_t i=0; i<abstraction->m_nWidth; i++ ) {
			// Write index to string
			uint8_t val = i;
			if ( !writeLevel1Register( nodeid, 
				abstraction->m_nOffset, 
				&val ) ) {
					if ( !bSilent ) wxMessageBox( _("Failed to write abstraction string index!") );
					break;
			}
			// Read value
			if ( !readLevel1Register( nodeid, 
				abstraction->m_nOffset + 1, 
				(p+i) ) ) {
					if ( !bSilent ) wxMessageBox( _("Failed to read indexed abstraction string value!") );
					break;
			}
		}
	}
	else {

		// Read string from linear storage.
		if ( !readLevel1Registers( pwnd, 
			p, 
			nodeid, 
			abstraction->m_nOffset, 
			abstraction->m_nWidth ) ) {
				if ( !bSilent ) wxMessageBox( _("Unable to read abstraction string!") );
		}

	}

	retstr.From8BitData( (const char *)p );
	if ( NULL != p ) delete p;

	// Restore page
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, savepage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction string!") );
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//  writeAbstractionString
//

bool CCanalSuperWrapper::writeAbstractionString( wxWindow *pwnd,
	uint8_t nodeid,
	CMDF_Abstraction *abstraction,
	wxString& strvalue,
	bool bSilent )
{
	// Check pointers
	if ( NULL == abstraction) return false;

	uint8_t *p;
	p = new uint8_t[ abstraction->m_nWidth + 1 ];
	memset( p, 0, abstraction->m_nWidth + 1 );
	strcpy( (char *)p, (const char*)strvalue.mb_str( wxConvUTF8 ) );

	// Save page
	uint16_t savepage = getRegisterPage( pwnd, nodeid );
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, abstraction->m_nPage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction string!") );
			return false;
		}
	}


	if ( abstraction->m_bIndexed ) {
		
		for ( uint8_t i=0; i<abstraction->m_nWidth; i++ ) {
			// Write index
			uint8_t val = i;
			if ( !writeLevel1Register( nodeid, 
										abstraction->m_nOffset, 
										&val ) ) {
				if ( !bSilent ) wxMessageBox( _("Failed to write abstraction string index!") );
				break;
			}
			// Read value
			if ( !writeLevel1Register( nodeid, 
										abstraction->m_nOffset + 1, 
										(p+i) ) ) {
				if ( !bSilent ) wxMessageBox( _("Failed to read indexed abstraction string value!") );
				break;
			}
		}

	}
	else {
		// Write string to linear storage.
		for ( uint8_t i=0; i<abstraction->m_nWidth; i++ ) {

			if ( !writeLevel1Register( nodeid, 
											abstraction->m_nOffset + i, 
											p++ ) ) {
					if ( !bSilent ) wxMessageBox( _("Unable to write abstraction string!") );
					break;
			}
		}
	}

	// Restore page
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, savepage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction string!") );
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//  getAbstractionBitField
// 

bool CCanalSuperWrapper::getAbstractionBitField( wxWindow *pwnd,
	uint8_t nodeid,
	CMDF_Abstraction *abstraction,
	wxString& retstr, 
	bool bSilent )
{
	wxString strvalue;

	// Check pointers
	if ( NULL == abstraction) return false;

	uint16_t savepage = getRegisterPage( pwnd, nodeid );
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, abstraction->m_nPage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction BitField!") );
			return false;
		}
	}

	// Octet width is the number of bytes needed to store the bits
	uint8_t octetwidth = abstraction->m_nWidth/8 + 
							( abstraction->m_nWidth % 8 ) ? 1 : 0; 

	uint8_t *p;
	p = new uint8_t[ octetwidth ];
	memset( p, 0, octetwidth );

	if ( abstraction->m_bIndexed ) {

		for ( uint8_t i=0; i<octetwidth; i++ ) {
			// Write index
			uint8_t val = i;
			if ( !writeLevel1Register( nodeid, 
				abstraction->m_nOffset, 
				&val ) ) {
					if ( !bSilent ) wxMessageBox( _("Failed to write abstraction BitField index!") );
					break;
			}
			// Read value
			if ( !readLevel1Register( nodeid, 
				abstraction->m_nOffset + 1, 
				(p+i) ) ) {
					if ( !bSilent ) wxMessageBox( _("Failed to read indexed abstraction BitField value!") );
					break;
			}
		}
	}
	else {

		// Read string from linear storage.
		if ( !readLevel1Registers( pwnd, 
			p, 
			nodeid, 
			abstraction->m_nOffset, 
			octetwidth ) ) {
				if ( !bSilent ) wxMessageBox( _("Unable to read abstraction BitField!") );
		}

	}

	for ( int i=0; i<abstraction->m_nWidth; i++ ) {
		for ( int j=7; j>0; j-- ) {
			if ( *(p + i) & (1 << j) ) {
				strvalue += _("1");
			}
			else {
				strvalue += _("0");
			}
		}
	}

	if ( NULL != p ) delete p;

	// Restore page
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, savepage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction BitField!") );
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//  writeAbstractionBitField
//

bool CCanalSuperWrapper::writeAbstractionBitField( wxWindow *pwnd,
	uint8_t nodeid,
	CMDF_Abstraction *abstraction,
	wxString& strBitField,
	bool bSilent )
{
	// Check pointers
	if ( NULL == abstraction) return false;

	// Octet width is the number of bytes needed to store the bits
	uint8_t octetwidth = abstraction->m_nWidth/8 + 
							( abstraction->m_nWidth % 8 ) ? 1 : 0; 

	uint8_t *p;
	p = new uint8_t[ octetwidth ];
	memset( p, 0, octetwidth );

	// Build byte array
	wxString str = strBitField;
	for ( int i=0; i<abstraction->m_nWidth; i++ ) {
		for ( int j=7; j>0; j-- ) {
			if ( !str.Length() ) break; // Must be digits left
			if ( _("1") == str.Left( 1 ) ) {
				*(p+1) += (1 << j);
			}
			str = str.Right( str.Length() - 1 );
		}
	}

	// Save page
	uint16_t savepage = getRegisterPage( pwnd, nodeid );
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, abstraction->m_nPage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction BitField!") );
			return false;
		}
	}


	if ( abstraction->m_bIndexed ) {
		
		for ( uint8_t i=0; i<octetwidth; i++ ) {
			// Write index to bitfield
			uint8_t val = i;
			if ( !writeLevel1Register( nodeid, 
										abstraction->m_nOffset, 
										&val ) ) {
				if ( !bSilent ) wxMessageBox( _("Failed to write abstraction BitField index!") );
				break;
			}
			// Read value
			if ( !writeLevel1Register( nodeid, 
										abstraction->m_nOffset + 1, 
										(p+i) ) ) {
				if ( !bSilent ) wxMessageBox( _("Failed to read indexed abstraction BitField value!") );
				break;
			}
		}

	}
	else {
		// Write bitfield to linear storage.
		for ( uint8_t i=0; i<octetwidth; i++ ) {

			if ( !writeLevel1Register( nodeid, 
											abstraction->m_nOffset + i, 
											p++ ) ) {
					if ( !bSilent ) wxMessageBox( _("Unable to write abstraction BitField!") );
					break;
			}
		}
	}

	// Restore page
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, savepage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction BitField!") );
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//  getAbstractionBool
//

bool CCanalSuperWrapper::getAbstractionBool( wxWindow *pwnd,
	uint8_t nodeid,
	CMDF_Abstraction *abstraction,
	bool *bval,
	bool bSilent )
{
	// Check pointers
	if ( NULL == abstraction) return false;

	uint16_t savepage = getRegisterPage( pwnd, nodeid );
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, abstraction->m_nPage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction bool!") );
			return false;
		}
	}

	// Read value
	uint8_t val;
	if ( !readLevel1Register( nodeid, 
		abstraction->m_nOffset, 
		&val ) ) {
			if ( !bSilent ) wxMessageBox( _("Failed to read abstraction boolean value!") );
	}

	*bval = val ? true : false;

	// Restore page
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, savepage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction bool!") );
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//  writeAbstractionBool
//

bool CCanalSuperWrapper::writeAbstractionBool( wxWindow *pwnd,
		uint8_t nodeid,
		CMDF_Abstraction *abstraction,
		bool& bval,
		bool bSilent )
{
	// Check pointers
	if ( NULL == abstraction) return false;

	uint8_t val = bval ? true : false;

	// Save page
	uint16_t savepage = getRegisterPage( pwnd, nodeid );
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, abstraction->m_nPage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction boolean!") );
			return false;
		}
	}

	val = bval ? true : false;
	if ( !writeLevel1Register( nodeid, 
								abstraction->m_nOffset, 
								&val ) ) {
		if ( !bSilent ) wxMessageBox( _("Unable to write abstraction boolean!") );
	}

	// Restore page
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, savepage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction string!") );
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//  getAbstraction8bitinteger
//

bool CCanalSuperWrapper::getAbstraction8bitinteger( wxWindow *pwnd,
	uint8_t nodeid,
	CMDF_Abstraction *abstraction,
	uint8_t *pval,
	bool bSilent )
{
	// Check pointers
	if ( NULL == abstraction) return false;

	uint16_t savepage = getRegisterPage( pwnd, nodeid );
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, abstraction->m_nPage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 8-bit integer!") );
			return false;
		}
	}

	// Read value
	if ( !readLevel1Register( nodeid, 
		abstraction->m_nOffset, 
		pval ) ) {
			if ( !bSilent ) wxMessageBox( _("Failed to read abstraction 8-bit integer value!") );

	}

	// Restore page
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid,savepage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 8-bit integer!") );
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//  writeAbstraction8bitinteger
//

bool CCanalSuperWrapper::writeAbstraction8bitinteger( wxWindow *pwnd,
		uint8_t nodeid,
		CMDF_Abstraction *abstraction,
		uint8_t& val,
		bool bSilent )
{
	// Check pointers
	if ( NULL == abstraction) return false;

	// Save page
	uint16_t savepage = getRegisterPage( pwnd, nodeid );
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, abstraction->m_nPage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 8-bit integer!") );
			return false;
		}
	}

	if ( !writeLevel1Register( nodeid, 
								abstraction->m_nOffset, 
								&val ) ) {
		if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 8-bit integer!") );
	}

	// Restore page
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, savepage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 8-bit integer!") );
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//  getAbstraction16bitinteger
//

bool CCanalSuperWrapper::getAbstraction16bitinteger( wxWindow *pwnd,
	uint8_t nodeid,
	CMDF_Abstraction *abstraction,
	uint16_t *pval,
	bool bSilent )
{
	// Check pointers
	if ( NULL == abstraction) return false;

	uint16_t savepage = getRegisterPage( pwnd, nodeid );
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, abstraction->m_nPage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 16-bit integer!") );
			return false;
		}
	}

	uint8_t *p = 0;
	p = new uint8_t[ 2 ];

	if ( abstraction->m_bIndexed ) {

		for ( uint8_t i=0; i<2; i++ ) {

			// Write index
			uint8_t idx = i;
			if ( !writeLevel1Register( nodeid, 
				abstraction->m_nOffset, 
				&idx ) ) {
					if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
					break;
			}
			// Read value
			if ( !readLevel1Register( nodeid, 
				abstraction->m_nOffset + 1, 
				(p+i) ) ) {
					if ( !bSilent ) wxMessageBox( _("Failed to read indexed abstraction value!") );
					break;
			}
		}
	}
	else {

		// Read string from linear storage.
		if ( !readLevel1Registers( pwnd, 
			p, 
			nodeid, 
			abstraction->m_nOffset, 
			2 ) ) {
				if ( !bSilent ) wxMessageBox( _("Unable to read abstraction string!") );
		}

	}

	*pval = ( p[0] << 8 ) + p[1];
	if ( NULL != p ) delete p;

	// Restore page
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, savepage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 16-bit integer!") );
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//  writeAbstraction16bitinteger
//

bool CCanalSuperWrapper::writeAbstraction16bitinteger( wxWindow *pwnd,
										uint8_t nodeid,
										CMDF_Abstraction *abstraction,
										uint16_t& val16,
										bool bSilent )
{
	uint8_t val;

	// Check pointers
	if ( NULL == abstraction) return false;

	// Save page
	uint16_t savepage = getRegisterPage( pwnd, nodeid );
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, abstraction->m_nPage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 16-bit integer!") );
			return false;
		}
	}

	if ( abstraction->m_bIndexed ) {
	
		// Index = 0
		val = 0;
		if ( !writeLevel1Register( nodeid, 
									abstraction->m_nOffset + 1, 
									&val ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
		}
		
		// Write MSB
		val = ( ( val16 >> 8 ) & 0xff );
		if ( !writeLevel1Register( nodeid, 
									abstraction->m_nOffset + 1, 
									&val ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
		}

		// Index = 1
		val = 1;
		if ( !writeLevel1Register( nodeid, 
									abstraction->m_nOffset + 1, 
									&val ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
		}

		// Write LSB
		val = ( val16 & 0xff );
		if ( !writeLevel1Register( nodeid, 
									abstraction->m_nOffset + 1, 
									&val ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
		}

	}
	else {

		// Write MSB
		val = ( ( val16 >> 8 ) & 0xff );
		if ( !writeLevel1Register( nodeid, 
									abstraction->m_nOffset, 
									&val ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
		}

		// Write LSB
		val = ( val16 & 0xff );
		if ( !writeLevel1Register( nodeid, 
									abstraction->m_nOffset + 1, 
									&val ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
		}
	}

	// Restore page
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, savepage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 16-bit integer!") );
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//  getAbstraction32bitinteger
//

bool CCanalSuperWrapper::getAbstraction32bitinteger( wxWindow *pwnd,
	uint8_t nodeid,
	CMDF_Abstraction *abstraction,
	uint32_t *pval,
	bool bSilent )
{
	// Check pointers
	if ( NULL == abstraction) return false;

	uint16_t savepage = getRegisterPage( pwnd, nodeid );
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, abstraction->m_nPage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 16-bit integer!") );
			return false;
		}
	}

	uint8_t *p;
	p = new uint8_t[ 4 ];

	if ( abstraction->m_bIndexed ) {

		for ( uint8_t i=0; i<4; i++ ) {

			// Write index to string
			uint8_t idx = i;
			if ( !writeLevel1Register( nodeid, 
				abstraction->m_nOffset, 
				&idx ) ) {
					if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
					break;
			}
			// Read value
			if ( !readLevel1Register( nodeid, 
				abstraction->m_nOffset + 1, 
				(p+i) ) ) {
					if ( !bSilent ) wxMessageBox( _("Failed to read indexed abstraction value!") );
					break;
			}
		}
	}
	else {

		// Read string from linear storage.
		if ( !readLevel1Registers( pwnd, 
			p, 
			nodeid, 
			abstraction->m_nOffset, 
			4 ) ) {
				if ( !bSilent ) wxMessageBox( _("Unable to read abstraction string!") );
		}

	}

	*pval = ( p[0] << 24 ) + ( p[1] << 16 ) + ( p[2] << 8 ) + p[3];
	if ( NULL != p ) delete p;

	// Restore page
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, savepage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 16-bit integer!") );
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//  writeAbstraction32bitinteger
//

bool CCanalSuperWrapper::writeAbstraction32bitinteger( wxWindow *pwnd,
										uint8_t nodeid,
										CMDF_Abstraction *abstraction,
										uint32_t& val32,
										bool bSilent )
{
	uint8_t val;

	// Check pointers
	if ( NULL == abstraction) return false;

	// Save page
	uint16_t savepage = getRegisterPage( pwnd, nodeid );
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, abstraction->m_nPage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 16-bit integer!") );
			return false;
		}
	}

	if ( abstraction->m_bIndexed ) {

		for ( int i=0; i<4; i++ ) {
	
			// Index = 0
			val = i;
			if ( !writeLevel1Register( nodeid, 
										abstraction->m_nOffset + 1, 
										&val ) ) {
				if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
			}
		
			// Write data
			val = ( ( val32 >> (8 * (3-i) ) ) & 0xff );
			if ( !writeLevel1Register( nodeid, 
										abstraction->m_nOffset + 1, 
										&val ) ) {
				if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
			}

		}

	}
	else {

		for ( int i=0; i<4; i++ ) {

			// Write data
			val = ( ( val32 >> (8 * (3-i) ) ) & 0xff );
			if ( !writeLevel1Register( nodeid, 
										abstraction->m_nOffset + i, 
										&val ) ) {
				if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
			}

		}
		
	}

	// Restore page
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, savepage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 16-bit integer!") );
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//  getAbstraction64bitinteger
//
bool CCanalSuperWrapper::getAbstraction64bitinteger( wxWindow *pwnd,
	uint8_t nodeid,
	CMDF_Abstraction *abstraction,
	uint64_t *pval,
	bool bSilent )
{
	// Check pointers
	if ( NULL == abstraction) return false;

	uint16_t savepage = getRegisterPage( pwnd, nodeid );
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, abstraction->m_nPage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 16-bit integer!") );
			return false;
		}
	}

	uint8_t *p;
	p = new uint8_t[ 8 ];

	if ( abstraction->m_bIndexed ) {

		for ( uint8_t i=0; i<8; i++ ) {

			// Write index to string
			uint8_t idx = i;
			if ( !writeLevel1Register( nodeid, 
				abstraction->m_nOffset, 
				&idx ) ) {
					if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
					break;
			}
			// Read value
			if ( !readLevel1Register( nodeid, 
				abstraction->m_nOffset + 1, 
				(p+i) ) ) {
					if ( !bSilent ) wxMessageBox( _("Failed to read indexed abstraction value!") );
					break;
			}
		}
	}
	else {

		// Read string from linear storage.
		if ( !readLevel1Registers( pwnd, 
			p, 
			nodeid, 
			abstraction->m_nOffset, 
			8 ) ) {
				if ( !bSilent ) wxMessageBox( _("Unable to read abstraction string!") );
		}

	}

	*pval = *( (uint64_t *)p );
	//( p[0] << 56 ) + ( p[1] << 48 ) + ( p[2] << 40 ) + ( p[3] << 32 ) +
	//		( p[4] << 24 ) + ( p[5] << 16 ) + ( p[6] << 8 ) + p[7];
	if ( NULL != p ) delete p;

	// Restore page
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, savepage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 16-bit integer!") );
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//  writeAbstraction64bitinteger
//

bool CCanalSuperWrapper::writeAbstraction64bitinteger( wxWindow *pwnd,
										uint8_t nodeid,
										CMDF_Abstraction *abstraction,
										uint64_t& val64,
										bool bSilent )
{
	uint8_t val;

	// Check pointers
	if ( NULL == abstraction) return false;

	// Save page
	uint16_t savepage = getRegisterPage( pwnd, nodeid );
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, abstraction->m_nPage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 16-bit integer!") );
			return false;
		}
	}

	if ( abstraction->m_bIndexed ) {

		for ( int i=0; i<8; i++ ) {
	
			// Index = 0
			val = i;
			if ( !writeLevel1Register( nodeid, 
										abstraction->m_nOffset + 1, 
										&val ) ) {
				if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
			}
		
			// Write data
			val = ( ( val64 >> (8 * (3-i) ) ) & 0xff );
			if ( !writeLevel1Register( nodeid, 
										abstraction->m_nOffset + 1, 
										&val ) ) {
				if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
			}

		}

	}
	else {

		for ( int i=0; i<8; i++ ) {

			// Write data
			val = ( ( val64 >> (8 * (3-i) ) ) & 0xff );
			if ( !writeLevel1Register( nodeid, 
										abstraction->m_nOffset + i, 
										&val ) ) {
				if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
			}

		}
		
	}

	// Restore page
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, savepage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 16-bit integer!") );
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//  getAbstractionFloat
//
bool CCanalSuperWrapper::getAbstractionFloat( wxWindow *pwnd,
	uint8_t nodeid,
	CMDF_Abstraction *abstraction,
	float *pval,
	bool bSilent )
{
	// Check pointers
	if ( NULL == abstraction) return false;

	uint16_t savepage = getRegisterPage( pwnd, nodeid );
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, abstraction->m_nPage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 16-bit integer!") );
			return false;
		}
	}

	uint8_t *p;
	p = new uint8_t[ 4 ];

	if ( abstraction->m_bIndexed ) {

		for ( uint8_t i=0; i<4; i++ ) {

			// Write index to string
			uint8_t idx = i;
			if ( !writeLevel1Register( nodeid, 
				abstraction->m_nOffset, 
				&idx ) ) {
					if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
					break;
			}
			// Read value
			if ( !readLevel1Register( nodeid, 
				abstraction->m_nOffset + 1, 
				(p+i) ) ) {
					if ( !bSilent ) wxMessageBox( _("Failed to read indexed abstraction value!") );
					break;
			}
		}
	}
	else {

		// Read string from linear storage.
		if ( !readLevel1Registers( pwnd, 
			p, 
			nodeid, 
			abstraction->m_nOffset, 
			4 ) ) {
				if ( !bSilent ) wxMessageBox( _("Unable to read abstraction string!") );
		}

	}

	*pval = wxINT32_SWAP_ON_LE( *((float *)p) );
	if ( NULL != p ) delete p;

	// Restore page
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, savepage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 16-bit integer!") );
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//  writeAbstractionFloat
//

bool CCanalSuperWrapper::writeAbstractionFloat( wxWindow *pwnd,
		uint8_t nodeid,
		CMDF_Abstraction *abstraction,
		float& valfloat,
		bool bSilent )
{
	uint8_t val;

	// Check pointers
	if ( NULL == abstraction) return false;

	uint8_t *p = (uint8_t *)&valfloat;

	// Save page
	uint16_t savepage = getRegisterPage( pwnd, nodeid );
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, abstraction->m_nPage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 16-bit integer!") );
			return false;
		}
	}

	if ( abstraction->m_bIndexed ) {

		for ( int i=0; i<4; i++ ) {
	
			// Index = 0
			val = i;
			if ( !writeLevel1Register( nodeid, 
										abstraction->m_nOffset + 1, 
										&val ) ) {
				if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
			}
		
			// Write data
			val = *(p+i);
			if ( !writeLevel1Register( nodeid, 
										abstraction->m_nOffset + 1, 
										&val ) ) {
				if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
			}

		}

	}
	else {

		for ( int i=0; i<4; i++ ) {

			// Write data
			val = val = *(p+i);;
			if ( !writeLevel1Register( nodeid, 
										abstraction->m_nOffset + i, 
										&val ) ) {
				if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
			}

		}
		
	}

	// Restore page
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, savepage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 16-bit integer!") );
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//  getAbstractionDouble
//

bool CCanalSuperWrapper::getAbstractionDouble( wxWindow *pwnd,
	uint8_t nodeid,
	CMDF_Abstraction *abstraction,
	double *pval,
	bool bSilent )
{
	// Check pointers
	if ( NULL == abstraction) return false;

	uint16_t savepage = getRegisterPage( pwnd, nodeid );
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, abstraction->m_nPage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 16-bit integer!") );
			return false;
		}
	}

	uint8_t *p;
	p = new uint8_t[ 8 ];

	if ( abstraction->m_bIndexed ) {

		for ( uint8_t i=0; i<8; i++ ) {

			// Write index to string
			uint8_t idx = i;
			if ( !writeLevel1Register( nodeid, 
				abstraction->m_nOffset, 
				&idx ) ) {
					if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
					break;
			}
			// Read value
			if ( !readLevel1Register( nodeid, 
				abstraction->m_nOffset + 1, 
				(p+i) ) ) {
					if ( !bSilent ) wxMessageBox( _("Failed to read indexed abstraction value!") );
					break;
			}
		}
	}
	else {

		// Read string from linear storage.
		if ( !readLevel1Registers( pwnd, 
			p, 
			nodeid, 
			abstraction->m_nOffset, 
			8 ) ) {
				if ( !bSilent ) wxMessageBox( _("Unable to read abstraction string!") );
		}

	}

	*pval = *((double *)p);
	if ( NULL != p ) delete p;

	// Restore page
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, savepage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 16-bit integer!") );
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//  writetAbstractionDouble
//

bool CCanalSuperWrapper::writetAbstractionDouble( wxWindow *pwnd,
		uint8_t nodeid,
		CMDF_Abstraction *abstraction,
		double& valdouble,
		bool bSilent )
{
	uint8_t val;

	// Check pointers
	if ( NULL == abstraction) return false;

	uint8_t *p = (uint8_t *)&valdouble;

	// Save page
	uint16_t savepage = getRegisterPage( pwnd, nodeid );
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, abstraction->m_nPage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 16-bit integer!") );
			return false;
		}
	}

	if ( abstraction->m_bIndexed ) {

		for ( int i=0; i<8; i++ ) {
	
			// Index = 0
			val = i;
			if ( !writeLevel1Register( nodeid, 
										abstraction->m_nOffset + 1, 
										&val ) ) {
				if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
			}
		
			// Write data
			val = *(p+i);
			if ( !writeLevel1Register( nodeid, 
										abstraction->m_nOffset + 1, 
										&val ) ) {
				if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
			}

		}

	}
	else {

		for ( int i=0; i<8; i++ ) {

			// Write data
			val = val = *(p+i);
			if ( !writeLevel1Register( nodeid, 
										abstraction->m_nOffset + i, 
										&val ) ) {
				if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
			}

		}
		
	}

	// Restore page
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, savepage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 16-bit integer!") );
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//  getAbstractionDate
//

bool CCanalSuperWrapper::getAbstractionDate( wxWindow *pwnd,
	uint8_t nodeid,
	CMDF_Abstraction *abstraction,
	wxDateTime *pval,
	bool bSilent )
{
	// Check pointers
	if ( NULL == abstraction) return false;

	uint16_t savepage = getRegisterPage( pwnd, nodeid );
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, abstraction->m_nPage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 16-bit integer!") );
			return false;
		}
	}

	uint8_t *p;
	p = new uint8_t[ 3 ];

	if ( abstraction->m_bIndexed ) {

		for ( uint8_t i=0; i<3; i++ ) {

			// Write index to string
			uint8_t idx = i;
			if ( !writeLevel1Register( nodeid, 
				abstraction->m_nOffset, 
				&idx ) ) {
					if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
					break;
			}
			// Read value
			if ( !readLevel1Register( nodeid, 
				abstraction->m_nOffset + 1, 
				(p+i) ) ) {
					if ( !bSilent ) wxMessageBox( _("Failed to read indexed abstraction value!") );
					break;
			}
		}
	}
	else {

		// Read string from linear storage.
		if ( !readLevel1Registers( pwnd, 
			p, 
			nodeid, 
			abstraction->m_nOffset, 
			3 ) ) {
				if ( !bSilent ) wxMessageBox( _("Unable to read abstraction string!") );
		}

	}

	pval->SetYear( 2000 + p[ 0 ] );
	pval->SetMonth( wxDateTime::Month( p[ 1 ] ) );
	pval->SetDay( p[ 2 ] );
	if ( NULL != p ) delete p;

	// Restore page
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, savepage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 16-bit integer!") );
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//  writeAbstractionDate
//

bool CCanalSuperWrapper::writeAbstractionDate( wxWindow *pwnd,
		uint8_t nodeid,
		CMDF_Abstraction *abstraction,
		wxDateTime& valdate,
		bool bSilent )
{
	uint8_t val;
	uint8_t buf[ 3 ];

	// Check pointers
	if ( NULL == abstraction) return false;

	buf[ 0 ] = valdate.GetYear() - 2000;
	buf[ 1 ] = valdate.GetMonth();
	buf[ 2 ] = valdate.GetDay();

	uint8_t *p = buf;

	// Save page
	uint16_t savepage = getRegisterPage( pwnd, nodeid );
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, abstraction->m_nPage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 16-bit integer!") );
			return false;
		}
	}

	if ( abstraction->m_bIndexed ) {

		for ( int i=0; i<3; i++ ) {
	
			// Index = 0
			val = i;
			if ( !writeLevel1Register( nodeid, 
										abstraction->m_nOffset + 1, 
										&val ) ) {
				if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
			}
		
			// Write data
			val = *(p+i);
			if ( !writeLevel1Register( nodeid, 
										abstraction->m_nOffset + 1, 
										&val ) ) {
				if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
			}

		}

	}
	else {

		for ( int i=0; i<3; i++ ) {

			// Write data
			val = val = *(p+i);
			if ( !writeLevel1Register( nodeid, 
										abstraction->m_nOffset + i, 
										&val ) ) {
				if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
			}

		}
		
	}

	// Restore page
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, savepage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 16-bit integer!") );
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//  getAbstractionTime
//

bool CCanalSuperWrapper::getAbstractionTime( wxWindow *pwnd,
	uint8_t nodeid,
	CMDF_Abstraction *abstraction,
	wxDateTime *pval,
	bool bSilent )
{
	// Check pointers
	if ( NULL == abstraction) return false;

	uint16_t savepage = getRegisterPage( pwnd, nodeid );
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, abstraction->m_nPage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 16-bit integer!") );
			return false;
		}
	}

	uint8_t *p;
	p = new uint8_t[ 3 ];

	if ( abstraction->m_bIndexed ) {

		for ( uint8_t i=0; i<3; i++ ) {

			// Write index to string
			uint8_t idx = i;
			if ( !writeLevel1Register( nodeid, 
				abstraction->m_nOffset, 
				&idx ) ) {
					if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
					break;
			}
			// Read value
			if ( !readLevel1Register( nodeid, 
				abstraction->m_nOffset + 1, 
				(p+i) ) ) {
					if ( !bSilent ) wxMessageBox( _("Failed to read indexed abstraction value!") );
					break;
			}
		}
	}
	else {

		// Read string from linear storage.
		if ( !readLevel1Registers( pwnd, 
			p, 
			nodeid, 
			abstraction->m_nOffset, 
			3 ) ) {
				if ( !bSilent ) wxMessageBox( _("Unable to read abstraction string!") );
		}

	}

	pval->SetHour( p[ 0 ] );
	pval->SetMinute( p[ 1 ] );
	pval->SetSecond( p[ 2 ] );
	if ( NULL != p ) delete p;

	// Restore page
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, savepage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 16-bit integer!") );
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//  writeAbstractionTime
//

bool CCanalSuperWrapper::writeAbstractionTime( wxWindow *pwnd,
		uint8_t nodeid,
		CMDF_Abstraction *abstraction,
		wxDateTime& valtime,
		bool bSilent )
{
	uint8_t val;
	uint8_t buf[ 3 ];

	// Check pointers
	if ( NULL == abstraction) return false;

	buf[ 0 ] = valtime.GetHour();
	buf[ 1 ] = valtime.GetMinute();
	buf[ 2 ] = valtime.GetSecond();

	uint8_t *p = buf;

	// Save page
	uint16_t savepage = getRegisterPage( pwnd, nodeid );
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, abstraction->m_nPage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 16-bit integer!") );
			return false;
		}
	}

	if ( abstraction->m_bIndexed ) {

		for ( int i=0; i<3; i++ ) {
	
			// Index = 0
			val = i;
			if ( !writeLevel1Register( nodeid, 
										abstraction->m_nOffset + 1, 
										&val ) ) {
				if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
			}
		
			// Write data
			val = *(p+i);
			if ( !writeLevel1Register( nodeid, 
										abstraction->m_nOffset + 1, 
										&val ) ) {
				if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
			}

		}

	}
	else {

		for ( int i=0; i<3; i++ ) {

			// Write data
			val = val = *(p+i);
			if ( !writeLevel1Register( nodeid, 
										abstraction->m_nOffset + i, 
										&val ) ) {
				if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
			}

		}
		
	}

	// Restore page
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, savepage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 16-bit integer!") );
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//  getAbstractionGUID
//

bool CCanalSuperWrapper::getAbstractionGUID( wxWindow *pwnd,
	uint8_t nodeid,
	CMDF_Abstraction *abstraction,
	cguid *pval,
	bool bSilent )
{
	// Check pointers
	if ( NULL == abstraction) return false;

	uint16_t savepage = getRegisterPage( pwnd, nodeid );
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, abstraction->m_nPage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 16-bit integer!") );
			return false;
		}
	}

	uint8_t *p;
	p = new uint8_t[ 16 ];

	if ( abstraction->m_bIndexed ) {

		for ( uint8_t i=0; i<16; i++ ) {

			// Write index to string
			uint8_t idx = i;
			if ( !writeLevel1Register( nodeid, 
				abstraction->m_nOffset, 
				&idx ) ) {
					if ( !bSilent ) wxMessageBox( _("Failed to write abstraction index!") );
					break;
			}
			// Read value
			if ( !readLevel1Register( nodeid, 
				abstraction->m_nOffset + 1, 
				(p+i) ) ) {
					if ( !bSilent ) wxMessageBox( _("Failed to read indexed abstraction value!") );
					break;
			}
		}
	}
	else {

		// Read string from linear storage.
		if ( !readLevel1Registers( pwnd, 
			p, 
			nodeid, 
			abstraction->m_nOffset, 
			16 ) ) {
				if ( !bSilent ) wxMessageBox( _("Unable to read abstraction string!") );
		}

	}

	pval->getFromArray( p );
	if ( NULL != p ) delete p;

	// Restore page
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, savepage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 16-bit integer!") );
			return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//  writeAbstractionGUID
//

bool CCanalSuperWrapper::writeAbstractionGUID( wxWindow *pwnd,
		uint8_t nodeid,
		CMDF_Abstraction *abstraction,
		cguid& valguid,
		bool bSilent )
{
	uint8_t val;
	uint8_t buf[ 16 ];

	// Check pointers
	if ( NULL == abstraction) return false;

	memcpy( buf, valguid.getGUID(), 16 );
	uint8_t *p = buf;

	// Save page
	uint16_t savepage = getRegisterPage( pwnd, nodeid );
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, abstraction->m_nPage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to set register page for abstraction 16-bit integer!") );
			return false;
		}
	}

	if ( abstraction->m_bIndexed ) {

		for ( int i=0; i<16; i++ ) {
	
			// Index = 0
			val = i;
			if ( !writeLevel1Register( nodeid, 
										abstraction->m_nOffset + 1, 
										&val ) ) {
				if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
			}
		
			// Write data
			val = *(p+i);
			if ( !writeLevel1Register( nodeid, 
										abstraction->m_nOffset + 1, 
										&val ) ) {
				if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
			}

		}

	}
	else {

		for ( int i=0; i<16; i++ ) {

			// Write data
			val = val = *(p+i);
			if ( !writeLevel1Register( nodeid, 
										abstraction->m_nOffset + i, 
										&val ) ) {
				if ( !bSilent ) wxMessageBox( _("Unable to write abstraction 16-bit integer!") );
			}

		}
		
	}

	// Restore page
	if ( savepage != abstraction->m_nPage ) {
		if ( !setRegisterPage( nodeid, savepage ) ) {
			if ( !bSilent ) wxMessageBox( _("Unable to restore register page for abstraction 16-bit integer!") );
			return false;
		}
	}

	return true;
}

#endif