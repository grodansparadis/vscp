// BootControl.cpp: implementation of the CBootControl class.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
// 
// This file is part of the VSCP (http://www.vscp.org) 
//
// Copyright (C) 2000-2010 Ake Hedman, eurosource, <akhe@eurosource.se>
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
// $RCSfile: BootControl.cpp,v $                                       
// $Date: 2005/07/28 21:48:47 $                                  
// $Author: akhe $                                              
// $Revision: 1.9 $ 
//////////////////////////////////////////////////////////////////////

#include <time.h>
#include "BootControl.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBootControl::CBootControl()
{
	m_nClients = 0;				// No clients yet
	m_bHandshake = false;		// No handshake as default
	m_pAddr = 0;
	m_Type = MEM_TYPE_PROGRAM;
	m_pCanalSuperWrapper = NULL;

	// Response timeout
	setResponseTimeout( BOOT_COMMAND_RESPONSE_TIMEOUT );

}

CBootControl::~CBootControl()
{
	if ( NULL != m_pCanalSuperWrapper ) delete m_pCanalSuperWrapper;
}
 

///////////////////////////////////////////////////////////////////////////////
// setCanalDevice
//

void CBootControl::setCanalDevice( devItem * pItem )
{
	if ( NULL != m_pCanalSuperWrapper ) delete m_pCanalSuperWrapper;

	m_pCanalSuperWrapper = new CCanalSuperWrapper( pItem );

	// Open CANAL i/f
	if ( NULL != m_pCanalSuperWrapper ) {
		if ( NULL != pItem ) {
			m_pCanalSuperWrapper->doCmdOpen( pItem->strParameters, pItem->flags );
		}
		else {
			m_pCanalSuperWrapper->doCmdOpen();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// addClient
//

void CBootControl::addClient( unsigned char nicknameID )
{
	int i;

	// First check if the ID is already in the list
	for ( i = 0; i < getClientCnt(); i++ ) {

		if ( nicknameID == m_clientList[ i ].m_nickname ) {
			return;
		}
	}

	if ( m_nClients < 256 ) {
		m_clientList[ getClientCnt() ].m_nickname = nicknameID;
		m_clientList[ getClientCnt() ].m_bootalgorithm = 0;
		m_clientList[ getClientCnt() ].m_pageMSB = 0;
		m_clientList[ getClientCnt() ].m_pageLSB = 0;
		m_clientList[ getClientCnt() ].m_GUID0 = 0;
		m_clientList[ getClientCnt() ].m_GUID3 = 0;
		m_clientList[ getClientCnt() ].m_GUID5 = 0;
		m_clientList[ getClientCnt() ].m_GUID7 = 0;
	}

	// Another client to serve
	m_nClients++;

}

///////////////////////////////////////////////////////////////////////////////
// writeDeviceMemory
//

bool CBootControl::loadIntelHexFile( const char *path )
{
	return m_memImage.load( path );
}



///////////////////////////////////////////////////////////////////////////////
// writeDeviceMemory
//

bool CBootControl::writeDeviceMemory( unsigned long * pchecksum )
{
	bool rv = true;
	canalMsg msg;

	if ( NULL == m_pCanalSuperWrapper ) return FALSE;

	// Send message
	msg.id = ID_PUT_DATA;
	msg.flags = CANAL_IDFLAG_EXTENDED;
	msg.sizeData = 8;

	for ( int i = 0; i < 8; i++ ) {

		switch ( m_Type ) {
		
			case MEM_TYPE_PROGRAM:
				msg.data[ i ] = m_memImage.m_bufPrg[ m_pAddr ];
				*pchecksum += m_memImage.m_bufPrg[ m_pAddr ];
				break;

			case MEM_TYPE_CONFIG:
				msg.data[ i ] = m_memImage.m_bufCfg[ m_pAddr ];
				*pchecksum += m_memImage.m_bufCfg[ m_pAddr ];
				break;

			case MEM_TYPE_EEPROM:
				msg.data[ i ] = m_memImage.m_bufEEPROM[ m_pAddr ]; 
				*pchecksum += m_memImage.m_bufEEPROM[ m_pAddr ];
				break;
		}

		// Update address
		m_pAddr++;

	}


	if ( m_pCanalSuperWrapper->doCmdSend( &msg ) ) {
				
		// Message queued - ( wait for response from client(s) ).
		if ( m_bHandshake ) {

			if ( !checkResponse( ID_RESPONSE_PUT_DATA ) ) {

				// Failure
				rv = false;

			}
		}				
	}
	else {

		// Failure
		rv = false;				

	}

	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// writeDeviceControlRegs
//

bool CBootControl::writeDeviceControlRegs( unsigned long addr,
												unsigned char flags,
												unsigned char cmd,
												unsigned char cmdData0,
												unsigned char cmdData1 )
{
	bool rv = true;	// think positive ;-)
	canalMsg msg;

	if ( NULL == m_pCanalSuperWrapper ) return FALSE;

	// Save the internal addresss
	m_pAddr = addr;

	if ( ( m_pAddr < MEMREG_PRG_END ) && ( m_pAddr < BUFFER_SIZE_PROGRAM ) ) {
							
		// Flash memory
		m_Type = MEM_TYPE_PROGRAM;				 
								
	}
	else if ( ( m_pAddr >= MEMREG_CONFIG_START ) && 
				( ( m_pAddr < MEMREG_CONFIG_START + BUFFER_SIZE_CONFIG ) ) ) {
							
		// Config memory
		m_Type = MEM_TYPE_CONFIG;			 

	}
	else if ( ( m_pAddr >= MEMREG_EEPROM_START ) && 
				( ( m_pAddr <= MEMREG_EEPROM_START + BUFFER_SIZE_EEPROM ) ) ) {

		// EEPROM
		m_Type = MEM_TYPE_EEPROM;	
	}
	else {
		return false;
	}


	msg.data[ 0 ]  = ( unsigned char )( addr & 0xff );
	msg.data[ 1 ]  = ( unsigned char )( ( addr >> 8 ) & 0xff );
	msg.data[ 2 ]  = ( unsigned char )( ( addr >> 16 ) & 0xff );
	msg.data[ 3 ]  = 0;
	msg.data[ 4 ]  = flags;
	msg.data[ 5 ]  = cmd;
	msg.data[ 6 ]  = cmdData0;
	msg.data[ 7 ]  = cmdData1;

	if ( flags & MODE_ACK ) {
		m_bHandshake = true;
	}
	else {
		m_bHandshake = false;
	}

	// Send message
	msg.id = ID_PUT_BASE_INFO;
	msg.flags = CANAL_IDFLAG_EXTENDED;
	msg.sizeData = 8;
	if ( m_pCanalSuperWrapper->doCmdSend( &msg ) ) {
		
		// Message queued - ( wait for response from client(s) ).
		if ( m_bHandshake ) {
			rv = checkResponse( ID_RESPONSE_PUT_BASE_INFO );	
		}

	}
	else {
		rv = false;
	}

	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// checkResponse
//

bool CBootControl::checkResponse( unsigned long id )
{
	int i;
	int nClients;
	canalMsg msg;
	int *pnodeArray;
	time_t tstart, tnow;
	bool rv = false;
	
	if ( NULL == m_pCanalSuperWrapper ) return FALSE;

	// If no nodes we are done.
	if ( 0 == getClientCnt() ) return true;

	// Get system time
	time( &tstart );

	pnodeArray = new int[ m_nClients ];		

	for ( i = 0; i < getClientCnt(); i++ ) {
		pnodeArray[ i ] = m_clientList[ i ].m_nickname; 	
	}

	nClients = getClientCnt();

	bool bRun = true;
	while( bRun ) {
	
		if ( m_pCanalSuperWrapper->doCmdDataAvailable() ) {

			m_pCanalSuperWrapper->doCmdReceive( &msg );

			if ( ( CANAL_IDFLAG_EXTENDED & msg.flags ) &&	// Extended message
				( ( msg.id & 0xffffff00 ) == id ) ) {		// correct id
				
				for ( i = 0; i < m_nClients; i++ ) {
					
					if ( (int)( msg.id & 0xff ) == pnodeArray[ i ] ) {
						
						nClients--;
						pnodeArray[ i ] = -1; // Mark as handled
						if ( !nClients ) {
							// Response received from all - return success
							rv = true;
							bRun = false;
						}

					} // id found

				} // client array

			} // id

		} // received message

		// check for timeout
		if ( m_responseTimeout ) {
			
			time( &tnow );
			if ( (unsigned long)( tnow - tstart ) > m_responseTimeout ) {
				rv = false;
				bRun = false;
			}

		}
		
	}

	if ( NULL != pnodeArray ) delete pnodeArray;

	return rv;
}

///////////////////////////////////////////////////////////////////////////////
// getClientCnt
//

unsigned char CBootControl::getClientCnt( void ) 
{ 
	return m_nClients;  
}


///////////////////////////////////////////////////////////////////////////////
// setDeviceInBootMode
//

bool CBootControl::setDeviceInBootMode( unsigned char idx )
{
	bool bRun;
	canalMsg msg, rcvmsg;
	time_t tstart, tnow;


	// Check for a valid index
	if ( getClientCnt() < ( idx + 1 )  ) return false;


	// First do a test to see if the device is already in boot mode
	// if it is 0x14nn should be returned (nn == nodeid).
	msg.data[ 0 ]  = 0x00;
	msg.data[ 1 ]  = 0x00;
	msg.data[ 2 ]  = 0x00;
	msg.data[ 3 ]  = 0x00;
	msg.data[ 4 ]  = 0x00;
	msg.data[ 5 ]  = 0x00;
	msg.data[ 6 ]  = 0x00;
	msg.data[ 7 ]  = 0x00;
	// Send message
	msg.id = ID_PUT_BASE_INFO;
	msg.flags = CANAL_IDFLAG_EXTENDED;
	msg.sizeData = 8;
	if ( m_pCanalSuperWrapper->doCmdSend( &msg ) ) {


		bRun = true;
		
		// Get start time
		time( &tstart );
		
		while( bRun ) {
	
			time( &tnow );
			if ( (unsigned long)( tnow - tstart ) > 2 ) {
				bRun = false;
			}
			
			if ( m_pCanalSuperWrapper->doCmdDataAvailable() ) {

				m_pCanalSuperWrapper->doCmdReceive( &rcvmsg );
				
				// Is this a read/write reply from the node?
				if ( ( rcvmsg.id & 0xffffff ) == (unsigned long)( ID_RESPONSE_PUT_BASE_INFO + m_clientList[ idx ].m_nickname ) ) {
					// yes already in bootmode - return
					return true;
				}

				// Is this a read/write reply from the node?
				if ( ( rcvmsg.id & 0xffffff ) == (unsigned long)( ID_RESPONSE_PUT_DATA + m_clientList[ idx ].m_nickname ) ) {
					// yes already in bootmode - return
					return true;
				}

			}			
		}
	}
	
	
	

	// Read page register MSB
	if ( !readRegister( m_clientList[ idx ].m_nickname, 
						VSCP_REG_PAGE_SELECT_MSB,
						&m_clientList[ idx ].m_pageMSB ) ) {
		return false;
	}
	
	// Read page register LSB
	if ( !readRegister( m_clientList[ idx ].m_nickname, 
						VSCP_REG_PAGE_SELECT_LSB,
						&m_clientList[ idx ].m_pageLSB ) ) {
		return false;
	}

	// Read page register GUID0
	if ( !readRegister( m_clientList[ idx ].m_nickname, 
						VSCP_REG_GUID0,
						&m_clientList[ idx ].m_GUID0 ) ) {
		return false;
	}

	// Read page register GUID3
	if ( !readRegister( m_clientList[ idx ].m_nickname, 
						VSCP_REG_GUID3,
						&m_clientList[ idx ].m_GUID3 ) ) {
		return false;
	}

	// Read page register GUID5
	if ( !readRegister( m_clientList[ idx ].m_nickname, 
						VSCP_REG_GUID5,
						&m_clientList[ idx ].m_GUID5 ) ) {
		return false;
	}

	// Read page register GUID7
	if ( !readRegister( m_clientList[ idx ].m_nickname, 
						VSCP_REG_GUID7,
						&m_clientList[ idx ].m_GUID7 ) ) {
		return false;
	}

	// Set device in boot mode
	msg.data[ 0 ]  = m_clientList[ idx ].m_nickname;	// Nickname to read register from
	msg.data[ 1 ]  = 1;									// VSCP PIC bootload algorithm	
	msg.data[ 2 ]  = m_clientList[ idx ].m_GUID0;
	msg.data[ 3 ]  = m_clientList[ idx ].m_GUID3;
	msg.data[ 4 ]  = m_clientList[ idx ].m_GUID5;
	msg.data[ 5 ]  = m_clientList[ idx ].m_GUID7;
	msg.data[ 6 ]  = m_clientList[ idx ].m_pageMSB;
	msg.data[ 7 ]  = m_clientList[ idx ].m_pageLSB;

	// Send message
	msg.id = ( VSCP_ENTER_BOOTLODER_MODE << 8 );
	msg.flags = CANAL_IDFLAG_EXTENDED;
	msg.sizeData = 8;
	if ( m_pCanalSuperWrapper->doCmdSend( &msg ) ) {

		bRun = true;
		
		// Get start time
		time( &tstart );
		
		while( bRun ) {
	
			time( &tnow );
			if ( (unsigned long)( tnow - tstart ) > 2 ) {
				bRun = false;
			}
			
			if ( m_pCanalSuperWrapper->doCmdDataAvailable() ) {

				m_pCanalSuperWrapper->doCmdReceive( &rcvmsg );
				
				// Is this a read/write reply from the node?
				if ( ( rcvmsg.id & 0xffffff ) == (unsigned long)( ID_RESPONSE_PUT_BASE_INFO + m_clientList[ idx ].m_nickname ) ) {
					// OK in bootmode - return
					return true;
				}
			}			
		}

	}

	return false;
}


///////////////////////////////////////////////////////////////////////////////
// readRegister
//

bool CBootControl::readRegister( unsigned char node, unsigned char reg, unsigned char *value )
{
	bool bRun;
	canalMsg msg, rcvmsg;
	time_t tstart, tnow;

	msg.data[ 0 ]  = node;	// Nickname to read register from
	msg.data[ 1 ]  = reg;	// register to read	

	// Send message
	msg.id = ( VSCP_READ_REGISTER << 8 );
	msg.flags = CANAL_IDFLAG_EXTENDED;
	msg.sizeData = 2;
	if ( m_pCanalSuperWrapper->doCmdSend( &msg ) ) {
		
		bRun = true;
		
		// Get start time
		time( &tstart );
		
		while( bRun ) {
	
			time( &tnow );
			if ( (unsigned long)( tnow - tstart ) > 2 ) {
				return false;
			}
			
			if ( m_pCanalSuperWrapper->doCmdDataAvailable() ) {

				m_pCanalSuperWrapper->doCmdReceive( &rcvmsg );
				
				// Is this a read/write reply from the node?
				if ( ( rcvmsg.id & 0xffffff ) == (unsigned long)( ( VSCP_RW_RESPONSE << 8 ) + node ) ) {
					if ( rcvmsg.data[ 0 ] == reg ) {
						*value = rcvmsg.data[ 1 ];
						bRun = false;	// return
					}
				}
			}			
		}
	}
	else {
		return false;
	}

	return true;
}