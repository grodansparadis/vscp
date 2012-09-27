// FILE: register.h
//
// Copyright (C) 2012 Ake Hedman akhe@grodansparadis.com 
//
// This software is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//

#ifndef _REGISTER_H_
#define _REGISTER_H_

#include "wx/wx.h"
#include "vscp.h"
#include "vscp_class.h"
#include "vscp_type.h"
#include "canal.h"
#include "guid.h"

/*!
    \class CStandardRegisters
    \brief Encapsulates the standard registers of a device
*/
class CStandardRegisters  
{

public:

    /*! 
        Default Constructor
    */
    CStandardRegisters( void );

	/*! 
        Default Destructor
    */
    ~CStandardRegisters( void );

	wxString getMDF( void );

	const uint8_t *getGUID( void ) { return ( m_reg + 0xD0 - 0x80 ); };
	
	void getGUID( cguid *pguid ) { pguid->getFromArray( m_reg + 0xD0 - 0x80 ); };

	uint8_t getBufferSize( void ) { return m_reg[ 0x98 - 0x80 ]; };

	uint8_t getBootloaderAlgorithm( void ) { return m_reg[ 0x97 - 0x80 ]; };

	uint8_t getMajorVersion( void ) { return m_reg[ 0x94 - 0x80 ]; };

	uint8_t getMinorVersion( void ) { return m_reg[ 0x95 - 0x80 ]; };

	uint8_t getSubMinorVersion( void ) { return m_reg[ 0x96 - 0x80 ]; };

	wxString getFirmwareVersionString( void );

	uint16_t getPage( void ) { return ( m_reg[ 0x92 - 0x80 ] * 256 +
									m_reg[ 0x93 - 0x80 ] ); };

	uint8_t getNickname( void ) { return m_reg[ 0x91 - 0x80 ]; };

	uint8_t getAlarm( void ) { return m_reg[ 0x80 - 0x80 ]; };

	uint8_t getConfirmanceVersionMajor( void ) { return m_reg[ 0x81 - 0x80 ]; };

	uint8_t getConfirmanceVersonMinor( void ) { return m_reg[ 0x82 - 0x80 ]; };

	uint8_t getNodeControl( void ) { return m_reg[ 0x83 - 0x80 ]; };

	uint8_t getNumberOfRegisterPages( void ) { return m_reg[ 0x99 - 0x80 ]; };

	uint32_t getManufacturerSubDeviceID( void ) 
								{ return ( ( m_reg[ 0x8D - 0x80 ] << 24 ) +
											( m_reg[ 0x8E - 0x80 ] << 16 ) +
											( m_reg[ 0x8F - 0x80 ] << 8 ) +
											( m_reg[ 0x90 - 0x80 ] ) ); };

	uint32_t getManufacturerDeviceID( void ) 
								{ return ( ( m_reg[ 0x89 - 0x80 ] << 24 ) +
											( m_reg[ 0x8A - 0x80 ] << 16 ) +
											( m_reg[ 0x8B - 0x80 ] << 8 ) +
											( m_reg[ 0x8C - 0x80 ] ) ); };


	uint8_t getStandardReg( uint8_t reg );

	/// Standard register storage
	uint8_t m_reg[ 128 ];

};


#endif
